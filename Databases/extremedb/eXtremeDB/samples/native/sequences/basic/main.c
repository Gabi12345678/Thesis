#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mcoseq.h"
#include "timeseries.h"
#include "mcotime.h"
#include "common.h"

const char * dbName = "TimeSeries";

#ifdef DISK_DATABASE
#define DATABASE_SEGMENT_SIZE  64*1024*1024
#else
#define DATABASE_SEGMENT_SIZE  1024*1024*1024
#endif
#define CACHE_SEGMENT_SIZE  1024*1024*1024
#define N_QUOTES       5000000
#define BUF_SIZE       100

#define ASUM(x, y) (((uint8)(x) + (y))*((y) - (x) + 1)/2)
#define FROM 1000000
#define TILL 5000000

static void insert_quotes(mco_db_h db)
{
    mco_trans_h trans;
    Quote quote;
    int i, j;
    float open_buf[BUF_SIZE], close_buf[BUF_SIZE], high_buf[BUF_SIZE], low_buf[BUF_SIZE];
    uint4 timestamp_buf[BUF_SIZE], volume_buf[BUF_SIZE];

    CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &trans));
    CHECK(Quote_new(trans, &quote));
    CHECK(Quote_symbol_put(&quote, "AAA", 3));
    for (i = 0; i < N_QUOTES;) {
        for (j = 0; i < N_QUOTES && j < BUF_SIZE; i++, j++) {
            timestamp_buf[j] = i+1;
            open_buf[j] = 2.0;
            close_buf[j] = 3.0;
            high_buf[j] = 4.0;
            low_buf[j] = 1.0;
            volume_buf[j] = i;
        }
        CHECK(Quote_timestamp_append(&quote, timestamp_buf, j));
        CHECK(Quote_open_append(&quote, open_buf, j));
        CHECK(Quote_close_append(&quote, close_buf, j));
        CHECK(Quote_high_append(&quote, high_buf, j));
        CHECK(Quote_low_append(&quote, low_buf, j));
        CHECK(Quote_volume_append(&quote, volume_buf, j));
    }
    CHECK(mco_trans_commit(trans));
}

typedef struct quote_iterators_t_
{
    mco_seq_iterator_t open;
    mco_seq_iterator_t close;
    mco_seq_iterator_t low;
    mco_seq_iterator_t high;
    mco_seq_iterator_t volume;
    mco_seq_iterator_t timestamp;
} quote_iterators_t;

static void open_iterators(mco_trans_h trans, quote_iterators_t* iterators)
{
    Quote quote;
    CHECK(Quote_by_sym_find(trans, "AAA", 3, &quote)); 
    CHECK(Quote_timestamp_search(&quote, &iterators->timestamp, FROM, MCO_SEQ_BOUNDARY_EXCLUSIVE, TILL, MCO_SEQ_BOUNDARY_INCLUSIVE));
    CHECK(Quote_open_project(&quote, &iterators->open, &iterators->timestamp));
    CHECK(Quote_close_project(&quote, &iterators->close, &iterators->timestamp));
    CHECK(Quote_low_project(&quote, &iterators->low, &iterators->timestamp));
    CHECK(Quote_high_project(&quote, &iterators->high, &iterators->timestamp));
    CHECK(Quote_volume_project(&quote, &iterators->volume, &iterators->timestamp));
}

static void scalar_traverse_quotes(mco_db_h db)
{
    mco_trans_h trans;
    quote_iterators_t iterators;
    double open_sum = 0;
    double close_sum = 0;
    double high_sum = 0;
    double low_sum = 0;
    uint8 timestamp_sum = 0;
    uint8 volume_sum = 0;
    float open, close, low, high;
    mco_time timestamp;
    uint4 volume;
    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    open_iterators(trans, &iterators);
        
    while (mco_seq_next_uint4(&iterators.timestamp, &timestamp) == MCO_S_OK &&
           mco_seq_next_float(&iterators.open, &open) == MCO_S_OK &&
           mco_seq_next_float(&iterators.close, &close) == MCO_S_OK &&
           mco_seq_next_float(&iterators.low, &low) == MCO_S_OK &&
           mco_seq_next_float(&iterators.high, &high) == MCO_S_OK &&
           mco_seq_next_uint4(&iterators.volume, &volume) == MCO_S_OK)
    {
        timestamp_sum += timestamp;
        open_sum += open;
        close_sum += close;
        high_sum += high;
        low_sum += low;
        volume_sum += volume;
    }
    CHECK(mco_trans_commit(trans));
}

static void tile_traverse_quotes(mco_db_h db)
{
    mco_trans_h trans;
    quote_iterators_t iterators;
    double open_sum = 0;
    double close_sum = 0;
    double high_sum = 0;
    double low_sum = 0;
    uint8 timestamp_sum = 0;
    uint8 volume_sum = 0;
    mco_bool is_rle = mco_seq_is_rle();

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    open_iterators(trans, &iterators);

    if (is_rle)
    {
        while (iterators.timestamp.next(&iterators.timestamp) == MCO_S_OK)
        {
            mco_size_t i, tile_size = iterators.timestamp.tile_size;
            for (i = 0; i < tile_size; i++) {
                mco_seq_rle_count_t cnt = mco_seq_rle_count(&iterators.timestamp, i);
                timestamp_sum += iterators.timestamp.tile.u.arr_mco_time[i] * cnt;
            }
        }
        while (iterators.open.next(&iterators.open) == MCO_S_OK)
        {
            mco_size_t i, tile_size = iterators.open.tile_size;
            for (i = 0; i < tile_size; i++) {
                mco_seq_rle_count_t cnt = mco_seq_rle_count(&iterators.open, i);
                open_sum += iterators.open.tile.u.arr_float[i] * cnt;
            }
        }
        while (iterators.close.next(&iterators.close) == MCO_S_OK)
        {
            mco_size_t i, tile_size = iterators.close.tile_size;
            for (i = 0; i < tile_size; i++) {
                mco_seq_rle_count_t cnt = mco_seq_rle_count(&iterators.close, i);
                close_sum += iterators.close.tile.u.arr_float[i] * cnt;
            }
        }
        while (iterators.high.next(&iterators.high) == MCO_S_OK)
        {
            mco_size_t i, tile_size = iterators.high.tile_size;
            for (i = 0; i < tile_size; i++) {
                mco_seq_rle_count_t cnt = mco_seq_rle_count(&iterators.high, i);
                high_sum += iterators.high.tile.u.arr_float[i] * cnt;
            }
        }
        while (iterators.timestamp.next(&iterators.low) == MCO_S_OK)
        {
            mco_size_t i, tile_size = iterators.low.tile_size;
            for (i = 0; i < tile_size; i++) {
                mco_seq_rle_count_t cnt = mco_seq_rle_count(&iterators.low, i);
                low_sum += iterators.low.tile.u.arr_float[i] * cnt;
            }
        }
        while (iterators.timestamp.next(&iterators.volume) == MCO_S_OK)
        {
            mco_size_t i, tile_size = iterators.volume.tile_size;
            for (i = 0; i < tile_size; i++) {
                mco_seq_rle_count_t cnt = mco_seq_rle_count(&iterators.volume, i);
                volume_sum += iterators.volume.tile.u.arr_uint4[i] * cnt;
            }
        }
    }
    else
    {
        while (iterators.timestamp.next(&iterators.timestamp) == MCO_S_OK &&
            iterators.open.next(&iterators.open) == MCO_S_OK &&
            iterators.close.next(&iterators.close) == MCO_S_OK &&
            iterators.low.next(&iterators.low) == MCO_S_OK &&
            iterators.high.next(&iterators.high) == MCO_S_OK &&
            iterators.volume.next(&iterators.volume) == MCO_S_OK)
        {
            mco_size_t i, tile_size = iterators.timestamp.tile_size;
            for (i = 0; i < tile_size; i++) { 
                timestamp_sum += iterators.timestamp.tile.u.arr_mco_time[i];
                open_sum += iterators.open.tile.u.arr_float[i];
                close_sum += iterators.close.tile.u.arr_float[i];
                high_sum += iterators.high.tile.u.arr_float[i];
                low_sum += iterators.low.tile.u.arr_float[i];
                volume_sum += iterators.volume.tile.u.arr_uint4[i];
            }
        }
    }
    CHECK(mco_trans_commit(trans));
}

static void aggregate_quotes(mco_db_h db)
{
    mco_trans_h trans;
    quote_iterators_t iterators;
    quote_iterators_t sum_iterators;
    double open_sum;
    double close_sum;
    double high_sum;
    double low_sum;
    uint8 timestamp_sum;
    uint8 volume_sum;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    open_iterators(trans, &iterators);

    CHECK(mco_seq_agg_sum_uint4(&sum_iterators.timestamp, &iterators.timestamp));
    CHECK(mco_seq_agg_sum_float(&sum_iterators.open, &iterators.open));
    CHECK(mco_seq_agg_sum_float(&sum_iterators.close, &iterators.close));
    CHECK(mco_seq_agg_sum_float(&sum_iterators.high, &iterators.high));
    CHECK(mco_seq_agg_sum_float(&sum_iterators.low, &iterators.low));
    CHECK(mco_seq_agg_sum_uint4(&sum_iterators.volume, &iterators.volume));

    CHECK(mco_seq_next_uint8(&sum_iterators.timestamp, &timestamp_sum));
    CHECK(mco_seq_next_double(&sum_iterators.open, &open_sum));
    CHECK(mco_seq_next_double(&sum_iterators.close, &close_sum));
    CHECK(mco_seq_next_double(&sum_iterators.low, &low_sum));
    CHECK(mco_seq_next_double(&sum_iterators.high, &high_sum));
    CHECK(mco_seq_next_uint8(&sum_iterators.volume, &volume_sum));

    /* aggregated values are accessible here */

    CHECK(mco_trans_commit(trans));
}


static void analyze_quotes(mco_db_h db)
{
    mco_trans_h trans;
    quote_iterators_t iterators;
    mco_seq_iterator_t open_close_iterator;
    mco_seq_iterator_t high_low_iterator;
    mco_seq_iterator_t diff_iterator;
    mco_seq_iterator_t div_iterator;
    mco_seq_iterator_t avg_iterator;
    mco_seq_iterator_t constant_iterator;
    double avg;

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    open_iterators(trans, &iterators);

    CHECK(mco_seq_add_float(&open_close_iterator, &iterators.open, &iterators.close));
    CHECK(mco_seq_add_float(&high_low_iterator, &iterators.high, &iterators.low));
    CHECK(mco_seq_sub_float(&diff_iterator, &open_close_iterator, &high_low_iterator));
    CHECK(mco_seq_const_float(&constant_iterator, 2));
    CHECK(mco_seq_div_float(&div_iterator, &diff_iterator, &constant_iterator));
    CHECK(mco_seq_agg_avg_float(&avg_iterator, &div_iterator));

    CHECK(mco_seq_next_double(&avg_iterator, &avg));

    /* average value is accessible here */

    CHECK(mco_trans_commit(trans));
}

static void remove_quotes(mco_db_h db)
{
    mco_trans_h trans;
    CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &trans));
    CHECK(Quote_delete_all(trans));
    CHECK(mco_trans_commit(trans));
}

void dump_mem(mco_db_h db)
{
    mco_size32_t total_pages;
    mco_size32_t free_pages;
    CHECK(mco_db_free_pages(db, &free_pages));
    CHECK(mco_db_total_pages(db, &total_pages));
    printf("\tUse %d pages\n", (int)(total_pages - free_pages));
}
    
int main(int argc, char *argv[])
{
    mco_runtime_info_t info;
    mco_device_t dev[4];
    mco_db_params_t db_params;
    timer_unit start;
    mco_db_h db;
    int n_devs = 1;

    mco_get_runtime_info(&info);
    CHECK(mco_runtime_start());
    mco_error_set_handler(sample_errhandler);
    mco_db_params_init(&db_params);

    dev[0].type       = MCO_MEMORY_CONV;
    dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;
    dev[0].size       = DATABASE_SEGMENT_SIZE;
    dev[0].dev.conv.ptr = malloc(DATABASE_SEGMENT_SIZE);

#ifdef DISK_DATABASE
    dev[1].type       = MCO_MEMORY_CONV;
    dev[1].assignment = MCO_MEMORY_ASSIGN_CACHE;
    dev[1].size       = CACHE_SEGMENT_SIZE;
    dev[1].dev.conv.ptr = malloc(CACHE_SEGMENT_SIZE);
    
    dev[2].type       = MCO_MEMORY_FILE;
    dev[2].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;
    sprintf( dev[2].dev.file.name, FILE_PREFIX "%s.dbs", dbName );
    dev[2].dev.file.flags = MCO_FILE_OPEN_DEFAULT;

    dev[3].type       = MCO_MEMORY_FILE;
    dev[3].assignment = MCO_MEMORY_ASSIGN_LOG;
    sprintf( dev[3].dev.file.name, FILE_PREFIX "%s.log", dbName );
    dev[3].dev.file.flags = MCO_FILE_OPEN_DEFAULT;
    
    remove(dev[2].dev.file.name);
    remove(dev[3].dev.file.name);

    n_devs += 3;
#endif 

    {
#ifdef DISK_DATABASE
        db_params.mem_page_size = 256;
        db_params.disk_page_size = 4096;
        db_params.db_log_type = UNDO_LOG;
#else
        db_params.mem_page_size = 4096;
        db_params.disk_page_size = 0;
#endif 

        CHECK(mco_db_open_dev(dbName, timeseries_get_dictionary(), dev, n_devs, &db_params));
        CHECK(mco_db_connect(dbName, &db));
       
        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        insert_quotes(db);
        printf("\tElapsed time for insert: %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));
        dump_mem(db);

        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        scalar_traverse_quotes(db);
        printf("\tElapsed time for scalar traverse %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));

        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        tile_traverse_quotes(db);
        printf("\tElapsed time for tile traverse %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));

        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        aggregate_quotes(db);
        printf("\tElapsed time for aggregation %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));

        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        analyze_quotes(db);
        printf("\tElapsed time for analysis %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));

        dump_mem(db);
        
        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        remove_quotes(db);
        printf("\tElapsed time for delete: %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));

        dump_mem(db);

        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        insert_quotes(db);
        printf("\tElapsed time for reinsert: %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));

        dump_mem(db);

        CHECK(mco_db_disconnect(db));
        CHECK(mco_db_close(dbName));
    } 
    return 0;
}

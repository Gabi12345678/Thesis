#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mcoseq.h"
#include "metersdb.h"
#include "mcotime.h"
#include "common.h"

const char * dbName = "metersdb";

#ifdef DISK_DATABASE
#define DATABASE_SEGMENT_SIZE  64*1024*1024
#else
#define DATABASE_SEGMENT_SIZE  1024*1024*1024
#endif
#define CACHE_SEGMENT_SIZE  1024*1024*1024

#define BUF_SIZE       100
#define N_VALUES       5000000

#define FROM 1000000
#define TILL 5000000

static void insert_values(mco_db_h db)
{
    mco_trans_h trans;
    Meter meter;
    int i, j;
    float value_buf[BUF_SIZE];
    mco_datetime timestamp_buf[BUF_SIZE];

    CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &trans));
    CHECK(Meter_new(trans, &meter));
    CHECK(Meter_id_put(&meter, 1));
    for (i = 0; i < N_VALUES;)
    {
        for (j = 0; i < N_VALUES && j < BUF_SIZE; i++, j++)
        {
            timestamp_buf[j] = i+1;
            value_buf[j] = i / BUF_SIZE;
        }
        CHECK(Meter_timestamp_append(&meter, timestamp_buf, j));
        CHECK(Meter_value_append(&meter, value_buf, j));
    }
    CHECK(mco_trans_commit(trans));
}

typedef struct meter_iterators_t_
{
    mco_seq_iterator_t timestamp;
    mco_seq_iterator_t value;
} meter_iterators_t;

static void open_iterators(mco_trans_h trans, meter_iterators_t* iterators)
{
    Meter meter;
    CHECK(Meter_by_id_find(trans, 1, &meter));
    CHECK(Meter_timestamp_search(&meter, &iterators->timestamp, FROM, MCO_SEQ_BOUNDARY_EXCLUSIVE, TILL, MCO_SEQ_BOUNDARY_INCLUSIVE));
    CHECK(Meter_value_project(&meter, &iterators->value, &iterators->timestamp));
}

static void tile_traverse_values(mco_db_h db)
{
    mco_trans_h trans;
    meter_iterators_t iterators;
    double value_sum = 0;
    uint8 timestamp_sum = 0;
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
                timestamp_sum += iterators.timestamp.tile.u.arr_datetime[i] * cnt;
            }
        }
        while (iterators.value.next(&iterators.value) == MCO_S_OK)
        {
            mco_size_t i, tile_size = iterators.value.tile_size;
            for (i = 0; i < tile_size; i++) {
                mco_seq_rle_count_t cnt = mco_seq_rle_count(&iterators.value, i);
                value_sum += iterators.value.tile.u.arr_float[i] * cnt;
            }
        }
    }
    else
    {
        while (iterators.timestamp.next(&iterators.timestamp) == MCO_S_OK &&
            iterators.value.next(&iterators.value) == MCO_S_OK)
        {
            mco_size_t i, tile_size = iterators.timestamp.tile_size;
            for (i = 0; i < tile_size; i++) { 
                timestamp_sum += iterators.timestamp.tile.u.arr_datetime[i];
                value_sum += iterators.value.tile.u.arr_float[i];
            }
        }
    }
    CHECK(mco_trans_commit(trans));
}

static void remove_values(mco_db_h db)
{
    mco_trans_h trans;
    CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &trans));
    CHECK(Meter_delete_all(trans));
    CHECK(mco_trans_commit(trans));
}

static void dump_mem(mco_db_h db)
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

        CHECK(mco_db_open_dev(dbName, metersdb_get_dictionary(), dev, n_devs, &db_params));
        CHECK(mco_db_connect(dbName, &db));

        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        insert_values(db);
        printf("\tElapsed time for insert: %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));
        dump_mem(db);

        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        tile_traverse_values(db);
        printf("\tElapsed time for tile traverse %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));

        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        remove_values(db);
        printf("\tElapsed time for delete: %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));

        dump_mem(db);

        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        insert_values(db);
        printf("\tElapsed time for reinsert: %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));

        dump_mem(db);

        CHECK(mco_db_disconnect(db));
        CHECK(mco_db_close(dbName));
    }

    return 0;
}

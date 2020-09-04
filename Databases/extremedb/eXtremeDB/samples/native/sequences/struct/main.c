#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "mcoseq.h"
#include "events.h"
#include "mcotime.h"
#include "common.h"

const char * dbName = "Events";

#ifdef DISK_DATABASE
#define DATABASE_SEGMENT_SIZE  16*1024*1024
#else
#define DATABASE_SEGMENT_SIZE  32*1024*1024
#endif
#define CACHE_SEGMENT_SIZE  16*1024*1024

#define N_EVENTS        50000
#define BUF_SIZE        100
#define URL_SIZE        10  /* This must be less or equal to MCO_SEQ_MAX_CHAR_SIZE */

void dump_mem(mco_db_h db)
{
    mco_size32_t total_pages;
    mco_size32_t free_pages;

    CHECK(mco_db_free_pages(db, &free_pages));
    CHECK(mco_db_total_pages(db, &total_pages));
    printf("\tUse %d pages\n", (int)(total_pages - free_pages));
}

static void insert_events(mco_db_h db)
{
    mco_trans_h trans;
    Event       evnt;
    int         i, j;
    uint4       day_buf[BUF_SIZE];
    User_fixed  user_buf[BUF_SIZE];
    char        url_buf[BUF_SIZE * URL_SIZE];

    CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &trans));
    CHECK(Event_new(trans, &evnt));
    CHECK(Event_server_put(&evnt, 0));
    for (i = 0; i < N_EVENTS;) {
        for (j = 0; i < N_EVENTS && j < BUF_SIZE; i++, j++) {
            day_buf[j] = i / 100;
            user_buf[j].ip = 1234567890 + i;
            user_buf[j].port = (uint2) (5000 + (i % 100));
            user_buf[j].browser = (uint2) i % 10;
            sprintf(url_buf + j * URL_SIZE, "?s=%d", i % 250);
        }
        CHECK(Event_day_append(&evnt, day_buf, j));
        CHECK(Event_user_append(&evnt, user_buf, j));
        CHECK(Event_url_append(&evnt, url_buf, j));
    }
    CHECK(mco_trans_commit(trans));
}

typedef struct event_iterators_t_
{
    mco_seq_iterator_t day;
    mco_seq_iterator_t user;
    mco_seq_iterator_t url;
} event_iterators_t;

static void open_iterators(mco_trans_h trans, event_iterators_t* iterators)
{
    Event evnt;

    CHECK(Event_by_server_find(trans, 0, &evnt)); 
    CHECK(Event_day_search(&evnt, &iterators->day, N_EVENTS / 100 / 4, MCO_SEQ_BOUNDARY_EXCLUSIVE, N_EVENTS / 100 / 2, MCO_SEQ_BOUNDARY_INCLUSIVE));
    CHECK(Event_user_project(&evnt, &iterators->user, &iterators->day));
    CHECK(Event_url_project(&evnt, &iterators->url, &iterators->day));
}

static uint4 scalar_traverse_events(mco_db_h db)
{
    mco_trans_h         trans;
    event_iterators_t   iterators;
    uint4               day, day_sum = 0;
    User_fixed          user;
    char                url[URL_SIZE];

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    open_iterators(trans, &iterators);

    while (mco_seq_next_uint4(&iterators.day, &day) == MCO_S_OK &&
           mco_seq_next_char(&iterators.user, (char *)&user) == MCO_S_OK &&
           mco_seq_next_char(&iterators.url, url) == MCO_S_OK)
    {
        day_sum += day;
        /* structure 'user' and char buffer 'url' are accessible here */
    }
    CHECK(mco_trans_commit(trans));
    return day_sum;
}

static uint4 tile_traverse_events(mco_db_h db)
{
    mco_trans_h         trans;
    event_iterators_t   iterators;
    uint4               day, day_sum = 0;
    mco_size_t          j;
    User_fixed          *puser;
    char                *purl;
    mco_bool            is_rle = mco_seq_is_rle();

    CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &trans));
    open_iterators(trans, &iterators);

    if (is_rle)
    {
        while (iterators.day.next(&iterators.day) == MCO_S_OK)
        {
            mco_size_t tile_size = iterators.day.tile_size;
            for (j = 0; j < tile_size; j++) { 
                mco_seq_rle_count_t cnt = mco_seq_rle_count(&iterators.day, j);
                day = iterators.day.tile.u.arr_uint4[j];

                day_sum += day * cnt;
            }
        }

        while (iterators.user.next(&iterators.user) == MCO_S_OK)
        {
            mco_size_t tile_size = iterators.user.tile_size;
            for (j = 0; j < tile_size; j++) { 
                mco_seq_rle_count_t cnt = mco_seq_rle_count(&iterators.user, j);
                puser = (User_fixed*)&iterators.user.tile.u.arr_char[j * sizeof(User_fixed)];
                /* pointer to structure 'puser' is accessible here */
            }
        }

        while (iterators.url.next(&iterators.url) == MCO_S_OK)
        {
            mco_size_t tile_size = iterators.url.tile_size;
            for (j = 0; j < tile_size; j++) { 
                mco_seq_rle_count_t cnt = mco_seq_rle_count(&iterators.url, j);
                purl = &iterators.url.tile.u.arr_char[j * URL_SIZE];
                /* pointer to char pointer 'purl' is accessible here */
            }
        }
    }
    else
    {
        while (iterators.day.next(&iterators.day) == MCO_S_OK &&
               iterators.user.next(&iterators.user) == MCO_S_OK &&
               iterators.url.next(&iterators.url) == MCO_S_OK)
        {
            mco_size_t tile_size = iterators.day.tile_size;
            for (j = 0; j < tile_size; j++) { 
                day = iterators.day.tile.u.arr_uint4[j];
                puser = (User_fixed*)&iterators.user.tile.u.arr_char[j * sizeof(User_fixed)];
                purl = &iterators.url.tile.u.arr_char[j * URL_SIZE];

                day_sum += day;
                /* pointer to structure 'puser' and char pointer 'purl' are accessible here */
            }
        }
    }
    CHECK(mco_trans_commit(trans));
    return day_sum;
}

static void remove_events(mco_db_h db)
{
    mco_trans_h trans;
    CHECK(mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &trans));
    CHECK(Event_delete_all(trans));
    CHECK(mco_trans_commit(trans));
}

int main(int argc, char *argv[])
{
    mco_runtime_info_t info;
    mco_device_t dev[4];
    mco_db_params_t db_params;
    timer_unit start;
    mco_db_h db;
    int n_devs = 1;
    uint4 res1, res2;

    CHECK(mco_runtime_start());
    mco_get_runtime_info(&info);
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

        CHECK(mco_db_open_dev(dbName, events_get_dictionary(), dev, n_devs, &db_params));
        CHECK(mco_db_connect(dbName, &db));
       
        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        insert_events(db);
        printf("\tElapsed time for insert: %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));
        dump_mem(db);

        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        res1 = scalar_traverse_events(db);
        printf("\tElapsed time for scalar traverse %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));

        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        res2 = tile_traverse_events(db);
        printf("\tElapsed time for tile traverse %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));
        dump_mem(db);

        start = MCO_SYSTEM_GET_CURRENT_TIME_MSEC();
        remove_events(db);
        printf("\tElapsed time for delete: %d msec\n", (int)(MCO_SYSTEM_GET_CURRENT_TIME_MSEC() - start));
        dump_mem(db);

        CHECK(mco_db_disconnect(db));
        CHECK(mco_db_close(dbName));
    } 
    return (res1 == res2) ? 0 : 1;
}

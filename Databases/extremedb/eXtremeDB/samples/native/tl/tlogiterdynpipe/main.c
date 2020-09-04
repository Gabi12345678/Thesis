/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <tlogiterdynpipedb.h>
#include "mcolog.h"

#define PIPES_COUNT                 10
#define PIPE_SEGMENT_SIZE           1024
#define BUFFER_SEGMENT_SIZE         1024*1024
#define RUN_TIME                    10*1000 /* 10 seconds */

#define IS_ODD(val) ((val)%2 != 0)
#define IS_EVEN(val) ((val)%2 == 0)
#define ASSERT_VAL(func, ret) do { if ((func) != (ret)) { printf("%s:%d: assertion %s == %s failed. Ret code %d\n", __FILE__, __LINE__, #func, #ret, ret); dbg_exit(ret); } } while (0);

char sample_descr[] = {
  "Sample 'tlogiterdynpipe' demonstrates the use of transaction logging iteration functionality\n"
  "using dynamic pipes. Multiple iteration threads are started and each creates pipe device, iterate over it and then detach the device\n"
};
const char *db_name = "tlogiterpipe";

typedef struct {
    int   id;
    int   n_instance_limits;
    int   n_extends;
    int   n_deferred_detachs;
    int   n_immediate_detachs;
    int   n_objects;
    int   n_connects;
    int   n_iterations;
    uint4 last_value;
    mco_bool action_stop_flag;
    mco_bool skip_stop_flag;
    mco_db_h db;
} thread_context_t;

int n_detach_callbacks = 0;

void detach_callback(const char *db_name, mco_device_t *dev, MCO_RET destroy_rc)
{
    printf("*** Detach callback, dbname '%s', ptr %p ***\n", db_name, dev->dev.conv.ptr);
    free(dev->dev.conv.ptr);
    n_detach_callbacks++;
}

void playStopThread( sample_task_t * descriptor )
{
    MCO_RET rc;
    thread_context_t *ctx = (thread_context_t *) descriptor->param;

    while (!ctx->action_stop_flag) {
        sample_sleep(200);
    }
    if (!ctx->skip_stop_flag)
    {
        rc = mco_translog_play_stop(ctx->db);
        if (rc != MCO_E_TL_NOT_STARTED)
        {
            CHECK(rc);
        }
    }
}

/* this function called by mco_translog_iterate as callback to process transaction data passed through a pipe */
MCO_RET iterator_proc (mco_trans_h trans, MCO_Hf* obj, int cid, int obj_state, void* user_ctx)
{
    thread_context_t *ctx = (thread_context_t *)user_ctx;
    if (obj) {
        Record rec;
        uint4 value;

        ctx->n_objects++;
        CHECK(Record_val_get ((Record *)obj, &value));
        if (ctx->last_value) {
            ASSERT_VAL(ctx->last_value + 1, value);
        } else {
            ctx->n_connects++;
        }
        ctx->last_value = value;
    }

    if (--ctx->n_iterations == 0)
    {
        if (IS_ODD(ctx->id)) {
            /* odd threads use mco_translog_pay_stop function to stop reader */
            ctx->action_stop_flag = MCO_YES;
        } else {
            /* even threads use non MCO_S_OK return code to stop reader */
            return (MCO_RET)(2 * 1000 * 1000);
        }
    }
    return MCO_S_OK;
}

/* this thread iterates the transaction */
void IteratorThread( sample_task_t * descriptor )
{
    thread_context_t *ctx = (thread_context_t *) descriptor->param;
    void *buf_ptr = malloc (BUFFER_SEGMENT_SIZE);
    mco_bool stop = MCO_NO;

    if (!buf_ptr)
        dbg_exit(-1);

    while (! stop)
    {
        MCO_RET rc;
        sample_task_t stop_task;
        mco_device_t pipe_dev;
        mco_TL_play_params_t tl_play_params;

        /* declare pipe device */
        pipe_dev.type         = MCO_MEMORY_CONV;
        pipe_dev.assignment   = MCO_MEMORY_ASSIGN_PIPE_BUF;
        pipe_dev.size         = PIPE_SEGMENT_SIZE;
        pipe_dev.dev.conv.ptr = malloc(pipe_dev.size);
        if (!pipe_dev.dev.conv.ptr)
            dbg_exit(-1);

        /* attach pipe device to the database */
        rc = mco_db_extend_dev(db_name, &pipe_dev);
        if (rc == MCO_E_INSTANCES_LIMIT) { /* It's possible due to deferred deletion */
            sample_sleep(100);
            ctx->n_instance_limits++;
            continue;
        }
        ASSERT_VAL(MCO_S_OK, rc);
        ctx->n_extends++;

        /* initialize iterator context */
        ctx->n_iterations = 100 + rand() % 100; /* 100 - 200 iterations */
        ctx->last_value   = 0;
        ctx->action_stop_flag = ctx->skip_stop_flag = MCO_NO;
        ctx->db = descriptor->db_connection;

        /* Prepare parameters and start mco_translog_play_ex() attached to the added pipe device */
        mco_translog_play_params_init(&tl_play_params);
        tl_play_params.pipe_db            = descriptor->db_connection;
        tl_play_params.pipe_device        = &pipe_dev;
        tl_play_params.iteration_proc     = iterator_proc;
        tl_play_params.iterproc_user_ctx  = ctx;
        tl_play_params.dict               = tlogiterdynpipedb_get_dictionary();
        tl_play_params.mem_ptr            = buf_ptr;
        tl_play_params.mem_size           = BUFFER_SEGMENT_SIZE;

        if (IS_ODD(ctx->id))
        {
            /* odd threads use mco_translog_pay_stop function to stop reader */
            sample_start_task(&stop_task, playStopThread, (void *)ctx);
            rc = mco_translog_play_ex(&tl_play_params);
            if (rc != MCO_E_TL_LOG_NOT_OPENED && rc != MCO_E_TL_PIPE_TERM && rc != MCO_S_OK && rc != MCO_E_TL_PLAY_STOPPED)
            {
                printf("Wrong return type of function mco_translog_play_ex %d at line %d\n", rc, __LINE__);
                dbg_exit(rc);
            }
            if (rc != MCO_E_TL_PLAY_STOPPED)
            {
                ctx->skip_stop_flag = MCO_YES;
                ctx->action_stop_flag = MCO_YES;
                stop = MCO_YES;
            }
            sample_join_task (&stop_task);
        }
        else
        {
            /* even threads use non MCO_S_OK return code of iteration_proc to stop reader */
            rc = mco_translog_play_ex(&tl_play_params);
            if (rc != 2 * 1000 * 1000)
            {   /* Return value from the iterator_proc() */
                if (rc != MCO_E_TL_LOG_NOT_OPENED && rc != MCO_E_TL_PIPE_TERM && rc != MCO_S_OK)
                {
                    printf("Wrong return type of function mco_translog_play_ex %d at line %d\n", rc, __LINE__);
                    dbg_exit(rc);
                }
                stop = MCO_YES;
            }
        }

        /* free() will be called in the detach_callback() */
        rc = mco_db_detach_dev(db_name, &pipe_dev);
        if (rc != MCO_S_DEFERRED_DELETE && rc != MCO_S_OK)
        {
            printf("Wrong return type of function mco_db_detach_dev %d\n", rc);
            dbg_exit(rc);
        }
        if (rc == MCO_S_OK) {
            ctx->n_immediate_detachs++;
        } else {
            ctx->n_deferred_detachs++;
        }
    }
    free (buf_ptr);
}

/* create 1 object and update it in the loop */
void use_database( mco_db_h con )
{
    Record  rec;
    mco_trans_h t;
    uint4 value;
    int start_time;

    /* create object with key == 0 */
    CHECK(mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));
    CHECK(Record_new( t, &rec ));
    CHECK(Record_key_put( &rec, 0 ));
    CHECK(Record_val_put( &rec, 100 ));
    CHECK(mco_trans_commit(t));

    /* increase value by one */
    for (start_time = sample_msec(); sample_msec() - start_time < RUN_TIME; ) {
        CHECK(mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));
        CHECK(Record_tkey_find( t, 0, &rec ));
        CHECK(Record_val_get( &rec, &value ));
        CHECK(Record_val_put( &rec, value + 1));
        CHECK(mco_trans_commit(t));
        sample_sleep(1);
    }
}

int main(int argc, char** argv) {
    MCO_RET             rc;
    mco_db_h            db;
    sample_task_t       iter_task[PIPES_COUNT];
    thread_context_t    cnt[PIPES_COUNT];
    int                 i;
    sample_memory_t     dbmem;
    mco_TL_start_data_t tl_parms;

    sample_os_initialize(DEFAULT);

    /* start eXtremeDB runtime */
    mco_runtime_start();

    mco_error_set_handler( &sample_errhandler );

    mco_runtime_setoption(MCO_RT_OPTION_MARK_LAST_OBJ, 1);
    /* Set maximum number of dynamic pipe segments */
    mco_runtime_setoption(MCO_RT_MAX_DYNAMIC_PIPES, PIPES_COUNT); 

    /* Register callback to receive notifications when the pipe segment deleted */
    CHECK(mco_register_detach_dev_callback(detach_callback, MCO_NO));
    mco_translog_init();

    sample_header( sample_descr );

    /* open database instance and connect it */
    CHECK(sample_open_database( db_name, tlogiterdynpipedb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, MAX_DB_CONNECTIONS + PIPES_COUNT, &dbmem ));

    /* connect to the main database */
    CHECK(mco_db_connect( db_name, &db ));

    /* setup and run transaction logging (use dynamic pipes) */
    tl_parms.flags = MCO_TRANSLOG_ITERABLE | MCO_TRANSLOG_PIPE | MCO_TRANSLOG_DYNAMIC_PIPE | MCO_TRANSLOG_SYNC_INSTANTLY;
    tl_parms.disk_page_size = PSTORAGE_PAGE_SIZE;
    CHECK(mco_translog_start( db, 0, &tl_parms ));

    /* Start iterator threads */
    for (i = 0; i < PIPES_COUNT; i++) {
        memset(&cnt[i], 0, sizeof(thread_context_t));
        cnt[i].id = i;
        sample_start_connected_task(&iter_task[i], IteratorThread, db_name, (void *)&cnt[i]);
    }

    /* Make database operations */
    use_database( db );

    /* stop transaction logging */
    CHECK(mco_translog_stop( db ));
    for (i = 0; i < PIPES_COUNT; i++) {
        sample_join_task (&iter_task[i]);
    }
    /* disconnect and close the main database */
    CHECK(mco_db_disconnect( db ));
    CHECK(sample_close_database( db_name, &dbmem ));

    /* Checks */
    {
        int n_instance_limits = 0, n_extends = 0, n_deferred_detachs = 0, n_immediate_detachs = 0;
        for (i = 0; i < PIPES_COUNT; i++) {
            n_instance_limits   += cnt[i].n_instance_limits;
            n_extends           += cnt[i].n_extends;
            n_deferred_detachs  += cnt[i].n_deferred_detachs;
            n_immediate_detachs += cnt[i].n_immediate_detachs;
            printf("Thread #%d iterated %d times, observed %d objects\n", i, cnt[i].n_connects, cnt[i].n_objects);
        }
        ASSERT_VAL(n_extends, n_deferred_detachs + n_immediate_detachs);
        ASSERT_VAL(n_detach_callbacks, n_deferred_detachs + n_immediate_detachs);
        printf("Number of mco_db_extend()/mco_db_detach() : %d\n", n_extends);
        printf("Number of deferred  detachs               : %d\n", n_deferred_detachs);
        printf("Number of immediate detachs               : %d\n", n_immediate_detachs);
    }
    mco_runtime_stop();
    sample_os_shutdown();
    return MCO_S_OK;
}

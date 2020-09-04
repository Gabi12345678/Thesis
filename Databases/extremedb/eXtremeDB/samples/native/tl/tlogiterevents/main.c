/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <tlogitereventsdb.h>
#include "mcolog.h"

#define PIPE_SEGMENT_SIZE       1024
#define BUFFER_SEGMENT_SIZE     1024*1024

char sample_descr[] = {
  "Sample 'tlogitereventsdb' demonstrates the use of \ntransaction logging iteration functionality \nas transport of class changing events.\n"
  "This sample transfers transactions inside one process \nthrough a pipe and reading events during log itaration.\n"
  "The pipe works with blocking (with no extension files).\n"
};
const char *db_name = "tlogitereventsdb";

typedef struct {
    int new_cnt;
    int delete_cnt;
    int delete_all_cnt;
    int class_update_cnt;
    int key_update_cnt;
    int val_update_cnt;
    int val2_update_cnt;
    int str_update_cnt;
} ev_stat_t;

/* Generic open database function */
MCO_RET open_database (const char *db_name, void *mem_ptr, mco_size_t mem_sz, void *pipe_ptr, mco_size_t pipe_sz)
{
  mco_device_t dev[2];
  unsigned int n_dev;
  mco_db_params_t db_params;

  dev[0].type       = MCO_MEMORY_CONV;
  dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;
  dev[0].dev.conv.ptr = mem_ptr;
  dev[0].size       = mem_sz;
  n_dev             = 1;

  if (pipe_sz)
  {
      dev[1].type       = MCO_MEMORY_CONV;
      dev[1].assignment = MCO_MEMORY_ASSIGN_PIPE_BUF;
      dev[1].dev.conv.ptr = pipe_ptr;
      dev[1].size       = pipe_sz;
      n_dev            += 1;
  }

  /* Set default database parameters */
  mco_db_params_init ( &db_params );
  /* Customize the params according to the application */
  db_params.mem_page_size            = MEMORY_PAGE_SIZE;
  db_params.disk_page_size           = 0;            /* Pure in-memory database */
  db_params.db_max_connections       = 10;
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key              = EXTREMEDB_LICENSE_KEY;
#endif

  return mco_db_open_dev (db_name, tlogitereventsdb_get_dictionary (), dev, n_dev, &db_params);
}

MCO_RET my_newEvent_handler( mco_trans_h t, Record * obj, MCO_EVENT_TYPE et, /*INOUT*/ void *param)
{
    ev_stat_t *s = (ev_stat_t *)param;
    printf("my_newEvent_handler %d\n", ++(s->new_cnt));
    return MCO_S_OK;
}

MCO_RET my_deleteEvent_handler( mco_trans_h t, Record * obj, MCO_EVENT_TYPE et, /*INOUT*/ void *param)
{
    ev_stat_t *s = (ev_stat_t *)param;
    printf("my_deleteEvent_handler %d\n", ++(s->delete_cnt));
    return MCO_S_OK;
}

MCO_RET my_classUpdateEvent_handler( mco_trans_h t, Record * obj, int class_code, const char* field_name, mco_dict_type_t field_type,const void * value, /*INOUT*/ void *param)
{
    uint1 updt_map;
    ev_stat_t *s = (ev_stat_t *)param;
    printf("my_classUpdateEvent_handler %d field ", ++(s->class_update_cnt));
    Record_updated_bitmap_get_range ((Record *)obj, 0, Record_updated_bitmap_length, &updt_map);
    if (updt_map & (1 << Record_key_field_index))
    {
        printf("key update %d ", ++(s->key_update_cnt));
    }
    if (updt_map & (1 << Record_val_field_index))
    {
        printf("val update %d ", ++(s->val_update_cnt));
    }
    if (updt_map & (1 << Record_val2_field_index))
    {
        printf("val2 update %d ", ++(s->val2_update_cnt));
    }
    if (updt_map & (1 << Record_str_field_index))
    {
        printf("str update %d ", ++(s->str_update_cnt));
    }
    printf("\n");
    return MCO_S_OK;
}

MCO_RET my_deleteAllEvent_handler( mco_trans_h t, Record * obj, MCO_EVENT_TYPE et, /*INOUT*/ void *param)
{
    ev_stat_t *s = (ev_stat_t *)param;
    printf("my_deleteAllEvent_handler %d\n", ++(s->delete_all_cnt));
    return MCO_S_OK;
}

MCO_RET register_callback(mco_trans_h t, void *param)
{
    MCO_RET rc;

    rc = mco_register_newEvent_handler(t, my_newEvent_handler, param);
    if (rc != MCO_S_OK)
        return rc;
    rc = mco_register_deleteEvent_handler(t, my_deleteEvent_handler, param);
    if (rc != MCO_S_OK)
        return rc;
    rc = mco_register_classUpdateEvent_handler(t, my_classUpdateEvent_handler, param, MCO_AFTER_UPDATE);
    if (rc != MCO_S_OK)
        return rc;
    rc = mco_register_deleteAllEvent_handler(t, my_deleteAllEvent_handler, param);
    return rc;
}

/* This thread iterates the transaction */
void IteratorThread( sample_task_t * descriptor )
{
  MCO_RET rc;
  ev_stat_t stat;
  void *buf_ptr;
  mco_db_h db = (mco_db_h)descriptor->param;

  memset(&stat, 0, sizeof(stat));

  /* Allocate memory for mco_translog_iterate internal buffer */
  buf_ptr = malloc (BUFFER_SEGMENT_SIZE);

  /* Connect to main database for mco_translog_iterate internal purposes */
  CHECK(mco_db_connect (db_name, &db));

  /* Main iteration call */
restart_reading:
  rc = mco_translog_play (0, db,  0 /* Iteration_proc may be omitted */, 0,
                                  tlogitereventsdb_get_dictionary (),
                                  buf_ptr, BUFFER_SEGMENT_SIZE,
                                  register_callback, &stat);

  /* Wait for main thread until it has started transaction logging (create pipe) */
  if (rc == MCO_E_TL_LOG_NOT_OPENED)
  {
    sample_sleep (100);
    goto restart_reading;
  }

  CHECK(mco_db_disconnect (db));

  free (buf_ptr);
}

/* Simulate normal database activity */
MCO_RET use_database( mco_db_h con )
{
    Record  rec;
    mco_trans_h t;
    mco_cursor_t csr;
    uint4 i;
    ev_stat_t stat;

    memset(&stat, 0, sizeof(ev_stat_t));

    /* Add 3 new keys */
    printf("Call trans_start\n");
    CHECK(mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));
    sample_sleep(300);

    for ( i = 0; i < 3; i++ )
    {
        printf("Call Record_new %d\n", ++(stat.new_cnt));
        CHECK(Record_new( t, &rec ));
        sample_sleep(300);

        printf("Call Record_key_put\n");
        CHECK(Record_key_put( &rec, i ));
        sample_sleep(300);
    }

    printf("Call trans_commit\n");
    CHECK(mco_trans_commit( t ));
    sample_sleep(300);

    /* Add 1 more new key */
    printf("Call trans_start\n");
    CHECK(mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));
    sample_sleep(300);

    {
        printf("Call Record_new %d\n", ++(stat.new_cnt));
        CHECK(Record_new( t, &rec ));
        sample_sleep(300);

        printf("Call Record_key_put\n");
        CHECK(Record_key_put( &rec, i++ ));
        sample_sleep(300);
    }

    printf("Call trans_commit\n");
    CHECK(mco_trans_commit( t ));
    sample_sleep(300);

    /* Increase last key value by one */
    printf("Call trans_start\n");
    CHECK(mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));
    sample_sleep(300);

    CHECK(Record_tkey_index_cursor( t, &csr ));
    CHECK(mco_cursor_last( t, &csr ));

    printf("Call Record_from_cursor\n");
    CHECK(Record_from_cursor( t, &csr, &rec ));
    sample_sleep(300);
    {
        uint4 val;

        printf("Call Record_val_get\n");
        CHECK(Record_val_get( &rec, &val ));
        sample_sleep(300);

        printf("Call Record_val_put %d\n", ++(stat.val_update_cnt));
        CHECK(Record_val_put( &rec, ++val ));
        sample_sleep(300);

        printf("Call Record_val2_get\n");
        CHECK(Record_val_get( &rec, &val ));
        sample_sleep(300);

        printf("Call Record_val2_put %d\n", ++(stat.val2_update_cnt));
        CHECK(Record_val_put( &rec, ++val ));
        sample_sleep(300);
    }
    CHECK(mco_cursor_close( t, &csr ));
    sample_sleep(300);

    {
        printf("Call Record_new %d\n", ++(stat.new_cnt));
        CHECK(Record_new( t, &rec ));
        sample_sleep(300);

        printf("Call Record_key_put\n");
        CHECK(Record_key_put( &rec, i ));
        sample_sleep(300);
    }


    printf("Call trans_commit\n");
    CHECK(mco_trans_commit( t ));
    sample_sleep(300);

    /* Increase first 3 key values by one */
    printf("Call trans_start\n");
    CHECK(mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));
    sample_sleep(300);

    CHECK(Record_tkey_index_cursor( t, &csr ));
    CHECK(mco_cursor_first( t, &csr ));

    for ( i = 0; i < 3; i++ )
    {
        uint4 val;

        printf("Call Record_from_cursor\n");
        CHECK(Record_from_cursor( t, &csr, &rec ));
        sample_sleep(300);

        printf("Call Record_val_get\n");
        CHECK(Record_val_get( &rec, &val ));
        sample_sleep(300);

        printf("Call Record_val_put %d\n", ++(stat.val_update_cnt));
        CHECK(Record_val_put( &rec, ++val ));
        sample_sleep(300);

        if ( i != 1) {
            printf("Call Record_val2_get\n");
            CHECK(Record_val2_get( &rec, &val ));
            sample_sleep(300);

            printf("Call Record_val2_put %d\n", ++(stat.val2_update_cnt));
            CHECK(Record_val2_put( &rec, ++val ));
            sample_sleep(300);
        }

        CHECK(mco_cursor_next( t, &csr ));
    }
    CHECK(mco_cursor_close( t, &csr ));
    sample_sleep(300);

    printf("Call trans_commit\n");
    CHECK(mco_trans_commit( t ));
    sample_sleep(300);

    /* Delete first key */
    printf("Call trans_start\n");
    CHECK(mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));
    sample_sleep(300);

    CHECK(Record_tkey_index_cursor( t, &csr ));
    CHECK(mco_cursor_first( t, &csr ));

    printf("Call Record_from_cursor\n");
    CHECK(Record_from_cursor( t, &csr, &rec ));
    sample_sleep(300);

    printf("Call Record_delete %d\n", ++(stat.delete_cnt));
    CHECK(Record_delete( &rec ));
    sample_sleep(300);

    CHECK(mco_cursor_close( t, &csr ));
    sample_sleep(300);

    printf("Call trans_commit\n");
    CHECK(mco_trans_commit( t ));
    sample_sleep(300);

    /* Delete all keys */
    printf("Call trans_start\n");
    CHECK(mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));
    sample_sleep(300);

    printf("Call Record_delete_all %d\n", ++(stat.delete_all_cnt));
    CHECK(Record_delete_all( t ));
    sample_sleep(300);

    printf("Call trans_commit\n");
    CHECK(mco_trans_commit( t ));
    sample_sleep(300);

    return MCO_S_OK;
}

int main(int argc, char** argv) {
  MCO_RET             rc;
  mco_db_h            db;
  void                *mem_ptr, *pipe_ptr;
  sample_task_t       iter_task;

  sample_os_initialize(DEFAULT);

  /* Start eXtremeDB runtime */
  mco_error_set_handler( &sample_errhandler );
  CHECK(mco_runtime_start());
  CHECK(mco_translog_init());

  sample_header( sample_descr );

  sample_show_runtime_info( "The database runtime has the following characteristics:\n" );
  printf( "\nBegin sample execution.\n" );

  /* Allocate memory for the main database and for a pipe */
  mem_ptr = malloc (DATABASE_SIZE);
  pipe_ptr = malloc (PIPE_SEGMENT_SIZE);

  /* Open the main database instance */
  rc = open_database (db_name, mem_ptr, DATABASE_SIZE, pipe_ptr, PIPE_SEGMENT_SIZE);
  sample_rc_check( "\tOpen database", rc );
  if ( MCO_S_OK == rc )
  {
    mco_TL_start_data_t tl_parms;

    /* Connect to the main database */
    rc = mco_db_connect( db_name, &db );
    sample_rc_check( "\tConnect to database", rc );
    if ( MCO_S_OK == rc )
    {
        /* Start the iterator thread */
        sample_start_task(&iter_task, IteratorThread, (void *)db );

        /* Set up and run transaction logging (pipe is created here) */
        tl_parms.flags = MCO_TRANSLOG_ITERABLE | MCO_TRANSLOG_PIPE | MCO_TRANSLOG_SYNC_INSTANTLY | MCO_TRANSLOG_EVENT_MASK;
        tl_parms.disk_page_size = PSTORAGE_PAGE_SIZE;
        /* tl_parms.event_mask = MCO_LOG_MASK_ALL_EVENTS; */
         tl_parms.event_mask = (1 << (MCO_EVENT_newEvent - 1)) | (1 << (MCO_EVENT_deleteEvent - 1))
             | (1 << (MCO_EVENT_deleteAllEvent - 1) | (1 << (MCO_EVENT_classUpdateEvent - 1)));

        rc = mco_translog_start( db, 0, &tl_parms );
        sample_rc_check( "\tStart logging\n", rc );

        /* Wait while iterator thread is alive */
        while(1) {
            mco_TL_current_info_t tl_info;

            CHECK(mco_translog_get_info ( db, &tl_info ));
            if (tl_info.pipe_readers_connected)
                break;
            else
                sample_sleep (100);
        }

        /* This function simulates normal database activity */
        rc = use_database( db );
        sample_rc_check( "\tModify and append database\n", rc );

        /* Stop transaction logging (pipe will be destroyed when the iterator has finished)*/
        rc = mco_translog_stop( db );
        sample_rc_check( "\tStop logging\n", rc );

        sample_join_task (&iter_task);

        /* Disconnect and close the main database */
        rc = mco_db_disconnect( db );
        sample_rc_check( "\tDisconnect database\n", rc );
    }

    rc = mco_db_close( db_name );
    sample_rc_check( "\tClose database", rc );
  }

  free (mem_ptr);
  free (pipe_ptr);

  CHECK(mco_runtime_stop());

  sample_pause_end( "\n\nPress any key to continue . . . " );

  sample_os_shutdown();

  return ( MCO_S_OK == rc ? 0 : 1 );
}

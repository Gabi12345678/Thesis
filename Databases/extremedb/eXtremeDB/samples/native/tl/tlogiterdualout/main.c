/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include "tlogiterdualoutdb.h"
#include "mcolog.h"

#define KEEP_TRANSACTION_INTEGRITY  1
#define PIPE_SEGMENT_SIZE       1024
#define BUFFER_SEGMENT_SIZE     1024*1024

char sample_descr[] = {
  "Sample 'tlogiterdualoutdb' demonstrates the use of \ntransaction logging dual out functionality.\n"
  "This sample transfers transactions between \ntwo databases through a pipe and a file.\n"
  "The pipe works with blocking (with no extension files).\n"
};
const char *db_name  = "tlogiterdualdb";
const char *db_name2 = "tlogiterdualdb2";
const char *log_name = "tlogiterdual.log";

typedef struct {
    mco_db_h    db;
    mco_trans_h trn;
    mco_bool    trn_started;
} db_context_t;

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

  return mco_db_open_dev (db_name, tlogiterdualoutdb_get_dictionary (), dev, n_dev, &db_params);
}

/* This function is called by mco_translog_iterate as callback, to process transaction data passed through a pipe */
MCO_RET iteration_proc (mco_trans_h trans, MCO_Hf* obj, int cid, int obj_state, void* user_ctx)
{
    MCO_RET rc;
    db_context_t *dbc = (db_context_t *)user_ctx;

    /* Handle mco_db_clean call */
    if (trans == 0)
    {
        return mco_db_clean(dbc->db);
    }

#if KEEP_TRANSACTION_INTEGRITY
    if (obj == 0)
    {
        if (dbc->trn_started == MCO_NO)
            return MCO_S_OK;
        printf ("\nCommit transaction to secondary database.\n");
        dbc->trn_started = MCO_NO;
        return mco_trans_commit (dbc->trn);
    }
    else if (dbc->trn_started == MCO_NO)
    {
        printf ("\nStart transaction in secondary database.\n");
        rc = mco_trans_start (dbc->db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &dbc->trn);
        if (rc != MCO_S_OK)
            return rc;
        dbc->trn_started = MCO_YES;
    }
#else
    /* Start a transaction in the secondary database to pass objects to */
    rc = mco_trans_start (dbc->db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &dbc->trn);
    if (rc != MCO_S_OK)
      return rc;
#endif

    if (cid == Record_code)
    {
      uint4 key;
      uint4 val;
      char buff[64*1024];
      uint2 len;
      Record obj1;

      /* Check the kind of object being processed */
      if (obj_state == MCO_TRANS_OBJ_ALL_DELETED)
      {
          Record_delete_all (dbc->trn);
      }
      else if (obj_state == MCO_TRANS_OBJ_DELETED)
      {
          Record_key_get ((Record *)obj, &key);
          Record_tkey_find (dbc->trn, key, &obj1);
          Record_delete (&obj1);
      }
      else if (obj_state == MCO_TRANS_OBJ_CREATED)
      {
          Record_key_get ((Record *)obj, &key);
          Record_val_get ((Record *)obj, &val);
          Record_str_get ((Record *)obj, buff, 0xffff, &len);
          Record_new (dbc->trn, &obj1);
          Record_key_put (&obj1, key);
          Record_val_put (&obj1, val);
          Record_str_put (&obj1, buff, len);
      }
      else
      {
          uint1 updt_map;

          Record_key_get ((Record *)obj, &key);
          Record_tkey_find (dbc->trn, key, &obj1);
          Record_updated_bitmap_get_range ((Record *)obj, 0, 
                    Record_updated_bitmap_length, &updt_map);

          if (updt_map & (1 << Record_val_field_index))
          {
            Record_val_get ((Record *)obj, &val);
            Record_val_put (&obj1, val);
          }
          if (updt_map & (1 << Record_str_field_index))
          {
            Record_str_get ((Record *)obj, buff, 0xffff, &len);
            Record_str_put (&obj1, buff, len);
          }
      }
    }
#if KEEP_TRANSACTION_INTEGRITY
    return MCO_S_OK;
#else
    return mco_trans_commit (dbc->trn);
#endif
}

/* Calculate and display the object count */
void count_database( mco_db_h con )
{
  MCO_RET rc;
  Record  rec;
  mco_trans_h t;
  mco_cursor_t csr;
  uint4 keys = 0;

  rc = mco_trans_start( con, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t );
  if ( MCO_S_OK == rc )  {
    rc = Record_tkey_index_cursor( t, &csr );
    if ( MCO_S_OK == rc )
    {
      rc = mco_cursor_first( t, &csr );
      while (MCO_S_OK == rc)
      {
        keys++;
        rc = mco_cursor_next( t, &csr );
      }
      CHECK(mco_cursor_close( t, &csr ));
    }
    CHECK(mco_trans_rollback( t ));
  }

  printf( "\n\n\tTotal of %d objects in the database 2\n", keys);
  if (keys != 100)
      dbg_exit(1);
}

/* This thread iterates the transaction */
void IteratorThread( sample_task_t * descriptor )
{
  MCO_RET rc;
  void *mem_ptr, *buf_ptr;
  db_context_t dbc;
  mco_db_h db2;
  mco_db_h db = (mco_db_h)descriptor->param;

  /* Allocate memory for secondary database and for mco_translog_iterate internal buffer */
  mem_ptr = malloc (DATABASE_SIZE);
  buf_ptr = malloc (BUFFER_SEGMENT_SIZE);

  /* Create secondary database */
  CHECK(open_database (db_name2, mem_ptr, DATABASE_SIZE, 0, 0));

  /* Connect to secondary database */
  CHECK(mco_db_connect (db_name2, &db2));
  /* Connect to main database for mco_translog_iterate internal purposes */
  CHECK(mco_db_connect (db_name, &db));

  dbc.db = db2;
  dbc.trn = 0;
  dbc.trn_started = MCO_NO;

  /* Main iteration call */
restart_reading:
  rc = mco_translog_iterate (0, db,  iteration_proc, (void *)&dbc,
                                tlogiterdualoutdb_get_dictionary (),
                                buf_ptr, BUFFER_SEGMENT_SIZE);

  /* Wait for main thread until it has started transaction logging (create pipe) */
  if (rc == MCO_E_TL_LOG_NOT_OPENED)
  {
    sample_sleep (100);
    goto restart_reading;
  }

  /* Print objects counted in the secondary database after the transactions are transferred */
  count_database (db2);

  /* Disconnect databases and close the secondary one */
  CHECK(mco_db_disconnect (db2));
  CHECK(mco_db_close (db_name2));
  CHECK(mco_db_disconnect (db));

  free (mem_ptr);
  free (buf_ptr);
}

void ReadLogFile()
{
  MCO_RET rc;
  void *mem_ptr, *buf_ptr;
  db_context_t dbc;
  mco_db_h db2;

  /* Allocate memory for secondary database and for mco_translog_iterate internal buffer */
  mem_ptr = malloc (DATABASE_SIZE);
  buf_ptr = malloc (BUFFER_SEGMENT_SIZE);

  /* Create secondary database */
  CHECK(open_database (db_name2, mem_ptr, DATABASE_SIZE, 0, 0));

  /* Connect to secondary database */
  CHECK(mco_db_connect (db_name2, &db2));

  dbc.db = db2;
  dbc.trn = 0;
  dbc.trn_started = MCO_NO;

  /* Main iteration call */
  CHECK(mco_translog_iterate (log_name, 0,  iteration_proc, (void *)&dbc,
                                tlogiterdualoutdb_get_dictionary (),
                                buf_ptr, BUFFER_SEGMENT_SIZE));

  /* Print objects counted in the secondary database after the transactions are transferred */
  count_database (db2);

  /* Disconnect databases and close the secondary one */
  CHECK(mco_db_disconnect (db2));
  CHECK(mco_db_close (db_name2));

  free (mem_ptr);
  free (buf_ptr);
}

/* Add 100 new objects and then update the last one */
MCO_RET use_database( mco_db_h con )
{
  MCO_RET rc;
  Record  rec;
  mco_trans_h t;
  mco_cursor_t csr;
  uint4 i;

  /* Add 100 new keys */
  rc = mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t );
  if ( MCO_S_OK != rc )
      return rc;

  for ( i = 0; i < 100; i++ ) {
    rc = Record_new( t, &rec );
    if ( MCO_S_OK != rc )
      break;

    rc = Record_key_put( &rec, i );
    if ( MCO_S_OK != rc )
      break;
  }

  if ( MCO_S_OK == rc )
    rc = mco_trans_commit( t );
  else
    mco_trans_rollback( t );

  if ( MCO_S_OK == rc )
  {
    printf( "\n\tAdded %d objects to database", i);
  }

  /* Increase last key value by one */
  rc = mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t );
  if ( MCO_S_OK != rc )
      return rc;

  rc = Record_tkey_index_cursor( t, &csr );
  if ( MCO_S_OK == rc )
  {
    rc = mco_cursor_last( t, &csr );
    if (MCO_S_OK == rc)
    {
      rc = Record_from_cursor( t, &csr, &rec );
      if ( MCO_S_OK == rc )  {
        uint4 val;

        Record_val_get( &rec, &val );
        Record_val_put( &rec, ++val );
      }
    }
    rc = mco_cursor_close( t, &csr );
  }

  if ( MCO_S_OK == rc )
    rc = mco_trans_commit( t );
  else
    mco_trans_rollback( t );

  return rc;
}

int main(int argc, char** argv) {
  MCO_RET             rc;
  mco_db_h            db;
  void                *mem_ptr, *pipe_ptr;
  sample_task_t       iter_task;

  sample_os_initialize(DEFAULT);

  mco_error_set_handler( &sample_errhandler );

#if KEEP_TRANSACTION_INTEGRITY
  /* This option causes an additional call of iteration_proc() at the end of each transaction
     with a null object handle to indicate 'end of transaction'. This enables interation_proc()
     to detect the last object accessed in the current transaction.
  */
  mco_runtime_setoption(MCO_RT_OPTION_MARK_LAST_OBJ, 1);
#endif
  /* Start eXtremeDB runtime */
  mco_runtime_start();
  mco_translog_init();

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

        /* Setup and run transaction logging (pipe is created here) */
        tl_parms.flags = MCO_TRANSLOG_ITERABLE | MCO_TRANSLOG_PIPE | MCO_TRANSLOG_DUAL_OUT;
        tl_parms.disk_page_size = PSTORAGE_PAGE_SIZE;
        tl_parms.dual_log_path = log_name;

        rc = mco_translog_start( db, 0, &tl_parms );
        sample_rc_check( "\tStart logging", rc );

        /* Wait while iterator thread is alive */
        while(1) {
            mco_TL_current_info_t tl_info;

            CHECK(mco_translog_get_info ( db, &tl_info ));
            if (tl_info.pipe_readers_connected)
                break;
            else
                sample_sleep (100);
        }

        printf( "\n\tIterate log via PIPE\n" );

        /* This function simulates normal database activity */
        rc = use_database( db );
        sample_rc_check( "\tModify and append database", rc );

        /* Stop transaction logging (pipe will be destroyed when the iterator has finished)*/
        rc = mco_translog_stop( db );
        sample_rc_check( "\tStop logging", rc );

        sample_join_task (&iter_task);

        /* Disconnect and close the main database */
        rc = mco_db_disconnect( db );
        sample_rc_check( "\tDisconnect database", rc );
    }

    rc = mco_db_close( db_name );
    sample_rc_check( "\tClose database", rc );
  }

  free (mem_ptr);
  free (pipe_ptr);

  printf( "\n\tIterate log file\n");
  ReadLogFile();

  mco_runtime_stop();

  sample_pause_end( "\n\nPress any key to continue . . . " );

  sample_os_shutdown();

  return ( MCO_S_OK == rc ? 0 : 1 );
}

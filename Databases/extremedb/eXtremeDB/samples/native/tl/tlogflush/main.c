/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <tlogflushdb.h>
#include "mcolog.h"

char sample_descr[] = {
  "Sample 'tlogflushdb' demonstrates the use of log flushing options.\n"
};
const char *db_name = "tlogflushdb";
const char *tl_log  = "tlogflush_tl.log";

/* Flush timer procedure */
mco_bool flush_timer( mco_TL_flush_timer_h handle )
{
    if ( handle->flush_time )
    {
        if ( handle->time_elapsed == 0 )
        {
            handle->time_elapsed = (timer_unit) sample_msec();
        }

        if ( ((timer_unit)sample_msec() - handle->time_elapsed) > handle->flush_time )
        {
            handle->time_elapsed = 0;
            printf( "Flush by timer!" );
            return MCO_YES;
        } 
    }
    return MCO_NO;
}

/* Add new objects to the database */
MCO_RET fill_database( mco_db_h con, int count, unsigned int ms )
{
  MCO_RET rc = MCO_S_OK;
  Record  rec;
  mco_trans_h t;
  static uint4 key = 0;
  int i;

  printf ("\n\n");

  for ( i = 0; i < count; i++ )
  {
    rc = mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t );
    if ( MCO_S_OK != rc )
        return rc;

    rc = Record_new( t, &rec );
    if ( MCO_S_OK == rc )
    {
      rc = Record_key_put( &rec, key++ );
    }

    if ( MCO_S_OK == rc )
      rc = mco_trans_commit( t );
    else
      mco_trans_rollback( t );      
   
    if ( MCO_S_OK != rc )
      break;

    printf ("*");
    sample_sleep (ms);
  }
  printf ("\n");
  return rc;
}

int main(int argc, char** argv) {
  MCO_RET             rc;
  sample_memory_t     dbmem;
  mco_db_h            connection;

  sample_os_initialize(DEFAULT);

  /* Start eXtremeDB runtime */
  mco_error_set_handler( &sample_errhandler );
  mco_runtime_start();
  mco_translog_init();

  sample_header( sample_descr );

  sample_show_runtime_info( "The database runtime has the following characteristics:\n" );
  printf( "\nBegin sample execution.\n" );

  /* Open database instance and connect it */
  rc = sample_open_database( db_name, tlogflushdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, MAX_DB_CONNECTIONS, &dbmem );

  sample_rc_check( "\tOpen database", rc );
  if ( MCO_S_OK == rc )
  {    
    mco_TL_start_data_t tl_parms;

    /* Connect to database */
    rc = mco_db_connect( db_name, &connection );
    sample_rc_check( "\tConnect to database", rc );
    if ( MCO_S_OK == rc )
    {
      /* Setup and run transaction logging */
      tl_parms.flags = MCO_TRANSLOG_SYNC_TIMER | MCO_TRANSLOG_SYNC_COUNT;
      tl_parms.disk_page_size = PSTORAGE_PAGE_SIZE;
      tl_parms.flush_depth = 10;  /* Flush data to disk each 10 records */
      tl_parms.flush_time = 1000; /* Or each 1000 msec */
      tl_parms.timer_proc = flush_timer;

      rc = mco_translog_start( connection, tl_log, &tl_parms );
      sample_rc_check( "\tStart logging", rc );

      /* The following two calls to 'fill_database' simulate normal database activity */

      /* With the timeout value between added objects set to 300 ms, flushing 
         by timer will be activated rather than flushing by counter (see the 
         messages displayed by the timer callback proc 'flush_timer'). */
      rc = fill_database( connection, 20, 300 );
      sample_rc_check( "\tAppend database ", rc );

      /* With the timeout value between added objects set to 10 ms, flushing 
         by counter will be activated rather than flushing by timer (note that
         no messages are displayed by the timer callback proc 'flush_timer'). */
      rc = fill_database( connection, 600, 10 );
      sample_rc_check( "\tAppend database ", rc );

      /* Stop transaction logging */
      rc = mco_translog_stop( connection );
      sample_rc_check( "\tStop logging", rc );
      
      rc = mco_db_disconnect( connection );
      sample_rc_check( "\tDisconnect database", rc );
    }

    rc = sample_close_database( db_name, &dbmem );
    sample_rc_check( "\tClose database", rc );
  }

  mco_runtime_stop();

  sample_pause_end( "\n\nPress any key to continue . . . " );

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

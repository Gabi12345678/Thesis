/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <tloglabelsdb.h>
#include "mcolog.h"

char sample_descr[] = {
  "Sample 'tloglabels' demonstrates the use of labeling parts of a log file.\n"
};
const char *db_name = "tloglabelsdb";
const char *tl_log  = "tloglabels_tl.log";

/* Label enumeration procedure */
void label_proc( const char *label, int label_id, void *data )
{
    printf( "%s '%s' id %d", (const char *)data, label, label_id );
}

/* Append 100 objects to database */
MCO_RET append_database( mco_db_h con )
{
  MCO_RET rc;
  Record rec;
  mco_trans_h t;
  uint4 i;
  static uint4 key = 0;

  rc = mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t );
  if ( rc != MCO_S_OK )
      return rc;

  for ( i = 0; i < 100; i++ ) {
    rc = Record_new( t, &rec );
    if ( MCO_S_OK != rc )
      break;

    rc = Record_key_put( &rec, key );
    if ( MCO_S_OK != rc )
      break;
  }

  if ( MCO_S_OK == rc )
    rc = mco_trans_commit( t );
  else
    mco_trans_rollback( t );

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
  rc = sample_open_database( db_name, tloglabelsdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, MAX_DB_CONNECTIONS, &dbmem );

  sample_rc_check( "\tOpen database", rc );
  if ( MCO_S_OK == rc )
  {
    /* Connect to database */
    rc = mco_db_connect( db_name, &connection );
    sample_rc_check( "\tConnect to database", rc );
    if ( MCO_S_OK == rc )
    {
      mco_TL_start_data_t tl_parms;
      mco_TL_current_info_t curr_info;

      /* Set up and run transaction logging */
      tl_parms.flags = 0;
      tl_parms.disk_page_size = PSTORAGE_PAGE_SIZE;

      rc = mco_translog_start( connection, tl_log, &tl_parms );
      sample_rc_check( "\tStart logging", rc );

      /* This function simulates normal database activity */
      rc = append_database( connection );
      sample_rc_check( "\tAppend database ", rc );

      /* Mark log with label */
      rc = mco_translog_label( connection, "label_1", 0 );
      sample_rc_check( "\tSet label to the log ", rc );

      /* This function simulates normal database activity */
      rc = append_database( connection );
      sample_rc_check( "\tAppend database ", rc );

      /* Mark log with label */
      rc = mco_translog_label( connection, "label_2", 0 );
      sample_rc_check( "\tSet label to the log ", rc );

      /* This function simulates normal database activity */
      rc = append_database( connection );
      sample_rc_check( "\tAppend database ", rc );

      /* Mark log with label */
      rc = mco_translog_label( connection, "label_3", 0 );
      sample_rc_check( "\tSet label to the log ", rc );

      /* Get info about current logging process */
      rc = mco_translog_get_info( connection, &curr_info );
      sample_rc_check( "\tGet current logging info", rc );

      printf( "\n\t%d labels written to this log", (int)curr_info.labels_count );

      /* Stop transaction logging */
      rc = mco_translog_stop( connection );
      sample_rc_check( "\tStop logging", rc );

      printf( "\n\tQuery log file labels" );

      /* Query information about the log */
      rc = mco_translog_query_info( connection, tl_log, label_proc,
                                                  (void *)"\n\tFound label", 0 );
      sample_rc_check( "\tQuery log file information", rc );

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

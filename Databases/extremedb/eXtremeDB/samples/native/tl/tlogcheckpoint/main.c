/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <tlogcheckpointdb.h>
#include "mcolog.h"

char sample_descr[] = {
  "Sample 'tlogcheckpointdb' demonstrates the use of function mco_translog_db_save.\n"
};
const char *db_name = "tlogcheckpointdb";
const char *tl_log  = "tlogcheckpoint_tl.log";
const char *db_bak  = "tlogcheckpointdb.bak";

mco_bool g_size_warn_flag = MCO_NO;
int g_size_warn_count = 0;

/* Size warning procedure */
void warn_sz_proc (mco_size_t log_size)
{
    printf ("-Warning! Log size %d-", (int)log_size);
    g_size_warn_flag = MCO_YES;
}

/* Stream writer with prototype mco_stream_write */
mco_size_sig_t file_writer( void *stream_handle /* FILE *  */, const void *from, mco_size_t nbytes )
{
  FILE *f = (FILE *)stream_handle;
  mco_size_sig_t nbs;

  nbs = fwrite( from, 1, nbytes, f );
  return nbs;
}

/* Append database with 100 objects */
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

  sample_os_initialize(DEFAULT|DISK);

  /* Start eXtremeDB runtime */
  mco_error_set_handler( &sample_errhandler );
  mco_runtime_start();
  mco_translog_init();

  sample_header( sample_descr );

  sample_show_runtime_info( "The database runtime has the following characteristics:\n" );
  printf( "\nBegin sample execution.\n" );

  /* Open database instance and connect it */
  rc = sample_open_database( db_name, tlogcheckpointdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
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
      FILE *fbak;

      /* Setup and run transaction logging */
      tl_parms.flags = MCO_TRANSLOG_SIZE_CLBK;
      tl_parms.disk_page_size = PSTORAGE_PAGE_SIZE;
      tl_parms.max_size = 100000; /* Warn if size of log reached 100kB */
      tl_parms.warn_sz_proc = warn_sz_proc;

      rc = mco_translog_start( connection, tl_log, &tl_parms );
      sample_rc_check( "\tStart logging", rc );

      printf( "\n\n" );
      while ( 5 > g_size_warn_count )
      {
        /* This function simulates normal database activity */
        rc = append_database( connection );
        if ( MCO_S_OK == rc )
        {
          printf( "*" );
        }
        else
        {
          sample_rc_check( "\tAppend database ", rc );
        }

        /* Save database checkpoint and truncate log when size of log has exceeded limit */
        if ( g_size_warn_flag == MCO_YES )
        {
          /* Create database checkpoint */
          fbak = fopen( db_bak, "wb" );
          if ( fbak == 0 ) {
            printf( "\n\tCan't open output file for streaming\n");
          } else {
            /* This function acts like mco_db_save and mco_translog_truncate in a single operation */
            rc = mco_translog_db_save( (void *)fbak, file_writer, connection );
            fclose( fbak );
            sample_rc_check( "\tSave database checkpoint", rc );
            printf("\n");
          }

          g_size_warn_flag = MCO_NO;
          g_size_warn_count++;
        }
      }

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

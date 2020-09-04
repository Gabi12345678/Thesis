/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <tlogbasicdb.h>
#include "mcolog.h"

char sample_descr[] = {
  "Sample 'tlogbasicdb' demonstrates the use of basic transaction logging \nfunctionality.\n"
};
const char *db_name = "tlogbasicdb";
const char *tl_log  = "tlogbasic_tl.log";
const char *db_bak  = "tlogbasicdb.bak";

/* Stream writer with prototype mco_stream_write */
mco_size_sig_t file_writer( void *stream_handle /* FILE *  */, const void *from, mco_size_t nbytes )
{
  FILE *f = (FILE *)stream_handle;
  mco_size_sig_t nbs;

  nbs = fwrite( from, 1, nbytes, f );
  return nbs;
}

/* Stream reader with prototype mco_stream_read */
mco_size_sig_t file_reader( void *stream_handle /* FILE *  */,  /* OUT */void *to, mco_size_t max_nbytes )
{
  FILE *f = (FILE *)stream_handle;
  mco_size_sig_t nbs;

  nbs = fread( to, 1, max_nbytes, f );
  return nbs;
}

/* Restore database checkpoint and apply log if it exists */
MCO_RET try_to_restore_db ( mco_db_h *con, sample_memory_t *dbmem, mco_bool *applied )
{
  MCO_RET rc;
  mco_TL_log_info_t info;
  mco_db_params_t db_params;
  FILE *fbak;    

  fbak = fopen( db_bak, "rb" );
  if ( fbak == 0 )
  {
    printf( "\n\tEmpty database opened successfully");
    return MCO_S_OK;
  }

  printf( "\n\tLog file and database checkpoint exist");
  printf( "\n\tRestore database from checkpoint");

  /* Close previously opened database */
  rc = mco_db_disconnect( *con );
  sample_rc_check( "\tDisconnect database", rc );
  if ( MCO_S_OK != rc )
    return rc;

  *con = 0;

  rc = sample_close_database( db_name, dbmem );
  sample_rc_check( "\tClose database", rc );
  if ( MCO_S_OK != rc )
    return rc;

  /* Restore database checkpoint */
  mco_db_params_init( &db_params );
  db_params.mem_page_size      = MEMORY_PAGE_SIZE;      /* Set page size for in-memory part */
  db_params.disk_page_size     = sample_disk_supported() ? PSTORAGE_PAGE_SIZE : 0; /* Set page size for persistent storage */
  db_params.db_max_connections = MAX_DB_CONNECTIONS;    /* Set total number of connections to the database */
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

  if (dbmem->dev[0].type == MCO_MEMORY_CONV)
    dbmem->dev[0].dev.conv.ptr = (void*)malloc( dbmem->dev[0].size );
  if (dbmem->n_dev >= 1 && dbmem->dev[1].type == MCO_MEMORY_CONV)
    dbmem->dev[1].dev.conv.ptr = (void*)malloc( dbmem->dev[1].size );

  rc = mco_db_load( (void *)fbak, file_reader, db_name, tlogbasicdb_get_dictionary(),
                                dbmem->dev, dbmem->n_dev, &db_params );
  fclose( fbak );
  sample_rc_check( "\tLoad database checkpoint image", rc );
  if ( MCO_S_OK != rc )
    return rc;

  /* Connect to database */
  rc = mco_db_connect( db_name, con );
  sample_rc_check( "\tConnect to restored database", rc );
  if ( MCO_S_OK != rc )
    return rc;

  /* Query information about the log */
  rc = mco_translog_query_info( *con, tl_log, 0, 0, &info );
  if ( (MCO_S_OK == rc || MCO_S_TL_INVDATA == rc)
          && info.transaction_apply_compat == MCO_YES )
  {
    /* Apply log */
    rc = mco_translog_apply( *con, tl_log, MCO_TRANSLOG_ALL_LABELS );
    sample_rc_check( "\tApply log to database", rc );

    if (MCO_S_OK == rc || MCO_S_TL_INVDATA == rc)
    {
      *applied = MCO_YES;
    }
  }
  else
  {
    printf( "\n\tDatabase checkpoint was restored but the log can't be applied");
    rc = MCO_E_TL;
  }
  return rc;
}

/* Update database and add 100 new objects */
MCO_RET use_database( mco_db_h con )
{
  MCO_RET rc;
  Record  rec;
  mco_trans_h t;
  mco_cursor_t csr;
  uint4 i, key, keys = 0, last_key = 0;

  /* Increase last key by one */
  rc = mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t );
  if ( MCO_S_OK == rc )  {
    rc = Record_tkey_index_cursor( t, &csr );
    if ( MCO_S_OK == rc )
    {
      rc = mco_cursor_last( t, &csr );
      if (MCO_S_OK == rc)
      {
        rc = Record_from_cursor( t, &csr, &rec );
        if ( MCO_S_OK == rc )  {
          Record_key_get( &rec, &key );
          Record_key_put( &rec, ++key );
          /* Store last key value to add further objects */
          last_key = ++key;
          keys++;
        }
      }
      rc = mco_cursor_close( t, &csr );
    }

    if ( MCO_S_OK == rc )
      rc = mco_trans_commit( t );
    else
      mco_trans_rollback( t );
  }

  if (keys)
      printf( "\n\tUpdated latest object with key %d", last_key - 1);

  /* Add 100 new keys */
  rc = mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t );
  if ( MCO_S_OK != rc )
      return rc;

  for ( i = 0; i < 100; i++ ) {
    rc = Record_new( t, &rec );
    if ( MCO_S_OK != rc )
      break;

    rc = Record_key_put( &rec, last_key++ );
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

  return rc;
}

int main(int argc, char** argv) {
  MCO_RET             rc;
  sample_memory_t     dbmem;
  mco_db_h            connection;
  mco_bool            applied = MCO_NO;
  mco_bool            restore_error = MCO_NO;

  sample_os_initialize(DEFAULT|DISK);

  /* Start eXtremeDB runtime */
  mco_error_set_handler( &sample_errhandler );
  mco_runtime_start();
  mco_translog_init();

  sample_header( sample_descr );

  sample_show_runtime_info( "The database runtime has the following characteristics:\n" );
  printf( "\nBegin sample execution.\n" );

  /* Open database instance and connect it */
  rc = sample_open_database( db_name, tlogbasicdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
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
      /* Restore previous database checkpoint and apply log to it */
      /* Else work with empty database */
      rc = try_to_restore_db( &connection, &dbmem, &applied );
      if ( MCO_S_OK == rc )
      {
        /* Create database checkpoint */
        FILE *fbak = fopen( db_bak, "wb" );
        if ( fbak == 0 ) {
          printf( "\n\tCan't open output file for streaming\n");
        } else {            
          rc = mco_db_save( (void *)fbak, file_writer, connection );
          fclose( fbak );
          sample_rc_check( "\tSave database checkpoint", rc );
        }

        /* Set up and run transaction logging */
        tl_parms.flags = MCO_TRANSLOG_CRC;
        tl_parms.disk_page_size = PSTORAGE_PAGE_SIZE;

        rc = mco_translog_start( connection, tl_log, &tl_parms );
        sample_rc_check( "\tStart logging", rc );

        /* This function simulates normal database activity */
        rc = use_database( connection );
        sample_rc_check( "\tModify and append database", rc );

        /* Stop transaction logging */
        rc = mco_translog_stop( connection );
        sample_rc_check( "\tStop logging", rc );

        /* Now we have a database checkpoint and TL log file */
      }
      else
      {
        restore_error = MCO_YES;
      }

      if ( connection )
      {
        rc = mco_db_disconnect( connection );
        sample_rc_check( "\tDisconnect database", rc );
      }
    }

    rc = sample_close_database( db_name, &dbmem );
    sample_rc_check( "\tClose database", rc );
  }

  mco_runtime_stop();

  if ( MCO_YES == applied || MCO_YES == restore_error )
  {
    printf( "\n\nYou may delete files %s and %s and then run the \nsample again to return to the 'empty"
            " database start' sample workflow.", db_bak, tl_log);
  }
  else
  {
    printf( "\n\nRun the sample again to try the 'continue database usage' sample workflow");
  }

  sample_pause_end( "\n\nPress any key to continue . . . " );

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

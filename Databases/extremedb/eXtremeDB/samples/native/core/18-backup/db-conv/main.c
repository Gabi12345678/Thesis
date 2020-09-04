/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include <backupdb.h>

char sample_descr[] = "Sample 'backup_db' demonstrates saving an image of a database to disk.\n";
const char * db_name = "backupdb";

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

/* Fill database with 100 objects */
int fill_database( mco_db_h con )
{
  MCO_RET rc;
  RecordT rect;
  RecordP recp;
  mco_trans_h t;
  uint4 i;

  rc = mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t );
  if ( rc != MCO_S_OK )
      return 0;

  for ( i = 0; i < 100; i++ ) {
    rc = RecordT_new( t, &rect );
    if ( MCO_S_OK != rc )
      break;

    rc = RecordT_keyT_put( &rect, i );
    if ( MCO_S_OK != rc )
      break;

    rc = RecordP_new( t, &recp );
    if ( MCO_S_OK != rc )
      break;

    rc = RecordP_keyP_put( &recp, i );
    if ( MCO_S_OK != rc )
      break;
  }

  if ( MCO_S_OK == rc )
    rc = mco_trans_commit( t );
  else
    mco_trans_rollback( t );

  return ( MCO_S_OK == rc ) ? i * 2: 0;
}

int  main(int argc, char** argv) 
{
  MCO_RET             rc;
  sample_memory_t     dbmem;
  mco_db_h            connection;
  mco_db_params_t     db_params;
  mco_trans_h         t;
  mco_cursor_t        c;
  FILE                *fbak;
  int                 objs_num, i = 0;

  sample_os_initialize(DEFAULT|DISK);

  mco_error_set_handler( &sample_errhandler );

  sample_header(sample_descr);

  /* Start eXtremeDB runtime */
  mco_runtime_start();

  sample_show_runtime_info("The database runtime has the following characteristics:\n" );
  printf( "\n\nBegin backup/restore . . . " );

  /* Open database instance and connect it */
  rc = sample_open_database( db_name, backupdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, MAX_DB_CONNECTIONS, &dbmem );
  sample_rc_check( "\tOpen database", rc );
  if ( MCO_S_OK == rc ) {

    /* Connect to database */
    rc = mco_db_connect( db_name, &connection );
    sample_rc_check( "\tConnect to database", rc );
    if ( MCO_S_OK == rc )
    {
      /* Clean database content to fill it with fresh data */
      rc = mco_db_clean( connection );
      sample_rc_check( "\tClean database", rc );
      if ( MCO_S_OK == rc ) {
        /* Fill the database */
        objs_num = fill_database( connection );
        printf( "\n\t%d objects were added to the database", objs_num );

        /* Backup database */
        fbak = fopen( "backupdb.bak", "wb" );
        if ( fbak == 0 ) {
          printf( "\n\tCan't open output file for streaming\n");
        } else {            
          rc = mco_db_save( (void *)fbak, file_writer, connection );
          fclose( fbak );
          sample_rc_check( "\tSave database", rc );
        }
      }
      /* Disconnect and close database */
      rc = mco_db_disconnect( connection );
      sample_rc_check( "\tDisconnect database", rc );
    }

    rc = sample_close_database( db_name, &dbmem );
    sample_rc_check( "\tClose database", rc );
  }

  if ( MCO_S_OK == rc ) {

    /* Restore database from backup file */
    mco_db_params_init( &db_params );
    db_params.mem_page_size      = MEMORY_PAGE_SIZE;      /* Set page size for in-memory part */
    db_params.disk_page_size     = sample_disk_supported() ? PSTORAGE_PAGE_SIZE : 0; /* Set page size for persistent storage */
    db_params.db_max_connections = MAX_DB_CONNECTIONS;    /* Set total number of connections to the database */
#ifdef EXTREMEDB_LICENSE_KEY
    db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

    if (dbmem.dev[0].type == MCO_MEMORY_CONV)
      dbmem.dev[0].dev.conv.ptr = (void*)malloc( dbmem.dev[0].size );
    if (dbmem.n_dev >= 1 && dbmem.dev[1].type == MCO_MEMORY_CONV)
      dbmem.dev[1].dev.conv.ptr = (void*)malloc( dbmem.dev[1].size );

    fbak = fopen( "backupdb.bak", "rb" );
    if ( fbak == 0 )
    {
      printf( "\n\tCan't open input file for streaming\n" );
    } else {
      rc = mco_db_load( (void *)fbak, file_reader, db_name, backupdb_get_dictionary(),
                                  dbmem.dev, dbmem.n_dev, &db_params );
      fclose( fbak );
      sample_rc_check( "\tLoad database", rc );
      if ( MCO_S_OK == rc ) {
        /* Connect to database */
        rc = mco_db_connect( db_name, &connection );
        sample_rc_check( "\tConnect to database", rc );
        if ( MCO_S_OK == rc ) {
          /* Count restored database objects */
          rc = mco_trans_start( connection, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t );
          if ( MCO_S_OK == rc ) {                    
            rc = RecordT_tkeyT_index_cursor( t, &c );
            if ( MCO_S_OK == rc ) {
              for ( rc = mco_cursor_first( t, &c ); MCO_S_OK == rc; rc = mco_cursor_next( t, &c ) )
                i++;
            }
            rc = RecordP_tkeyP_index_cursor( t, &c );
            if ( MCO_S_OK == rc ) {
              for ( rc = mco_cursor_first( t, &c ); MCO_S_OK == rc; rc = mco_cursor_next( t, &c ) )
                i++;
            }
            mco_trans_commit( t );
          }

          printf( "\n\t%d objects were found in the database", i );

          /* Disconnect and close database */
          rc = mco_db_disconnect( connection );
          sample_rc_check( "\tDisconnect database", rc );
        }
      }

      rc = sample_close_database( db_name, &dbmem );
      sample_rc_check( "\tClose database", rc );
    }
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include <backupdb.h>

char sample_descr[] = "Sample 'backup_bse' demonstrates binary schema evolution of a database.\n";
const char * db_name = "backupdb";

/* Stream reader with prototype mco_stream_read */
mco_size_sig_t file_reader( void *stream_handle /* FILE *  */,  /* OUT */void *to, mco_size_t max_nbytes )
{
  FILE *f = (FILE *)stream_handle;
  mco_size_sig_t nbs;

  nbs = fread( to, 1, max_nbytes, f );
  return nbs;
}

void init_sample_memory( 
           const char * db_name,  /* Name of the database */
           mco_size_t db_sz,      /* Size of memory segment for in-mem part of the db */
           mco_size_t cache_sz,   /* Size of cache segment for persistent storage */
           sample_memory_t * pdev)
{
    mco_runtime_info_t info;

    pdev->n_dev = 1;

    /* Get runtime info */
    mco_get_runtime_info(&info);
   
    /* Setup memory device as a plain conventional memory region */
    pdev->dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* Assign the device as a main database memory */
    pdev->dev[0].size       = db_sz;                          /* Set the device size */
    if (info.mco_shm_supported) {
        pdev->dev[0].type       = MCO_MEMORY_NAMED;           /* Set the device as a shared named memory device */
        sprintf( pdev->dev[0].dev.named.name, "%s-db", db_name ); /* Set memory name */
        pdev->dev[0].dev.named.flags = 0;                     /* zero flags */
        pdev->dev[0].dev.named.hint  = 0;                     /* Set mapping address or null it */
    } else {
        pdev->dev[0].type       = MCO_MEMORY_CONV;            /* Set the device as a conventional memory device */
        pdev->dev[0].dev.conv.ptr = (void*)malloc( db_sz );   /* Allocate memory and set device pointer */
    }

    if (info.mco_disk_supported) {
        /* Set up memory device as a conventional memory region for cache */
        pdev->dev[1].assignment = MCO_MEMORY_ASSIGN_CACHE;        /* Assign the device as a cache */
        pdev->dev[1].size       = cache_sz;
        if (info.mco_shm_supported) {
            pdev->dev[1].type       = MCO_MEMORY_NAMED;           /* Set the device as a shared named memory device */
            sprintf( pdev->dev[1].dev.named.name, "%s-cache", db_name ); /* Set memory name */
            pdev->dev[1].dev.named.flags = 0;                     /* Zero flags */
            pdev->dev[1].dev.named.hint  = 0;                     /* Set mapping address or null it */
        } else {
            pdev->dev[1].type       = MCO_MEMORY_CONV;            /* Set the device as a conventional memory device */
            pdev->dev[1].dev.conv.ptr = (void*)malloc( cache_sz );/* Allocate memory and set device pointer */
        }
        
        /* Set-up memory device for main database storage */
        pdev->dev[2].type       = MCO_MEMORY_FILE;                /* Set the device as a file device */
        pdev->dev[2].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;   /* Assign the device as a main database persistent storage */
        sprintf( pdev->dev[2].dev.file.name, FILE_PREFIX "%s.dbs", db_name ); /* Name the device */
        pdev->dev[2].dev.file.flags = MCO_FILE_OPEN_DEFAULT;      /* Set the device flags */
    
        /* Set-up memory device for database log */
        pdev->dev[3].type       = MCO_MEMORY_FILE;                /* Set the device as a file device */
        pdev->dev[3].assignment = MCO_MEMORY_ASSIGN_LOG;          /* Assign the device as a log */
        sprintf( pdev->dev[3].dev.file.name, FILE_PREFIX "%s.log", db_name ); /* Name the device */
        pdev->dev[3].dev.file.flags = MCO_FILE_OPEN_DEFAULT;      /* Set the device flags */
    
        //unlink(pdev->dev[2].dev.file.name);
        //unlink(pdev->dev[3].dev.file.name);

        pdev->n_dev            += 3;
    } else {
        pdev->dev[1].assignment = MCO_MEMORY_NULL;
        pdev->dev[2].assignment = MCO_MEMORY_NULL;
        pdev->dev[3].assignment = MCO_MEMORY_NULL;
    }
}

int  main(int argc, char** argv) 
{
  MCO_RET             rc = MCO_S_OK;
  sample_memory_t     dbmem;
  mco_db_h            connection;
  mco_db_params_t     db_params;
  mco_trans_h         t;
  mco_cursor_t        c;
  FILE                *fbak;
  int                 i = 0;

  sample_os_initialize(DEFAULT|DISK);

  /* Start eXtremeDB runtime */
  mco_error_set_handler( &sample_errhandler );
  mco_runtime_start();

  sample_header(sample_descr);

  sample_show_runtime_info("The database runtime has the following characteristics:\n" );
  printf( "\n\nBegin restore and BSE of a previously saved database image . . . " );

  /* Restore database from backup file */
  mco_db_params_init( &db_params );
  db_params.mem_page_size      = MEMORY_PAGE_SIZE;      /* Set page size for in-memory part */
  db_params.disk_page_size     = sample_disk_supported() ? PSTORAGE_PAGE_SIZE : 0; /* Set page size for persistent storage */
  db_params.db_max_connections = MAX_DB_CONNECTIONS;    /* Set total number of connections to the database */
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

  init_sample_memory ( db_name, DATABASE_SIZE, CACHE_SIZE, &dbmem );

  fbak = fopen( "backupdb.bak", "rb" );
  if ( fbak == 0 )
  {
    printf( "\n\nPlease run sample '18-backup-db' in this directory first to save an initial database\n" );
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

        /* Disconnect and close the database */
        rc = mco_db_disconnect( connection );
        sample_rc_check( "\tDisconnect database", rc );
      }
    }

    rc = sample_close_database( db_name, &dbmem );
    sample_rc_check( "\tClose database", rc );
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

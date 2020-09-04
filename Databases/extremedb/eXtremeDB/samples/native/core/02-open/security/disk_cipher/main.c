/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <genericdb.h>

char sample_descr[] = {
  "Sample 'disk_cipher' opens a database and protects it by cipher key.\n"
};
const char * db_name = "disk_cipher";

/* Define relatively small memory segment sizes to facilitate testing */
#define  MAX_DEVICES           10
#define  N_DEVICES             4

/* Define nRecords large enough to cause creation of sufficient data to necessitate 
   use of multifile segments */
const int nRecords = 1000000;
/* Define transaction block size to reduce time for inserts */
const int nInsertsPerTransaction = 10000;

int main(int argc, char* argv[])
{
  MCO_RET            rc;
  mco_db_h db = 0;
  mco_device_t       dev[N_DEVICES];  /* Memory devices for: 0) database, 1) cache, 2) main database storage, 3) transaction log */
  mco_db_params_t    db_params;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  /* Configure first memory device as a plain conventional memory region */
  dev[0].type       = MCO_MEMORY_CONV;
  dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;
  dev[0].size       = DATABASE_SIZE;
  dev[0].dev.conv.ptr = (void*)malloc( DATABASE_SIZE );

  /* Configure conventional memory region for cache */
  dev[1].type       = MCO_MEMORY_CONV;
  dev[1].assignment = MCO_MEMORY_ASSIGN_CACHE;
  dev[1].size       = CACHE_SIZE;
  dev[1].dev.conv.ptr = (void*)malloc( CACHE_SIZE );

  /* Configure FILE memory device for main database storage */
  dev[2].type = MCO_MEMORY_FILE;
  dev[2].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;
  sprintf(dev[2].dev.file.name, FILE_PREFIX "%s.dbs", db_name);
  dev[2].dev.file.flags = MCO_FILE_OPEN_DEFAULT;

  /* Configure FILE memory device for transaction log */
  dev[3].type       = MCO_MEMORY_FILE;
  dev[3].assignment = MCO_MEMORY_ASSIGN_LOG;
  sprintf(dev[3].dev.file.name, FILE_PREFIX "%s.log", db_name);
  dev[3].dev.file.flags = MCO_FILE_OPEN_DEFAULT;

  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );                  /* Initialize the params with default values */
  db_params.mem_page_size      = MEMORY_PAGE_SIZE;    /* Set page size for in-memory part */
  db_params.disk_page_size     = PSTORAGE_PAGE_SIZE;  /* Set page size for persistent storage */
  db_params.db_max_connections = 1;                   /* Set total number of connections to the database */
  db_params.db_log_type        = UNDO_LOG;            /* Set log type */
  db_params.cipher_key         = "my_secure_key";     /* Set cipher key to a database file */
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

  /* Open a database on the configured devices with given params */
  rc = mco_db_open_dev(db_name, genericdb_get_dictionary(), dev, N_DEVICES, &db_params );
  if ( MCO_S_OK == rc ) {

    /* Connect to the database, obtain a database handle */
    mco_db_connect(db_name, &db); /* No recovery connection data */
    if ( MCO_S_OK == rc ) {

      /* Show characteristics of opened database */
      sample_show_device_info("\n\tThe opened database has the following memory devices:", dev, N_DEVICES);

      mco_db_disconnect(db);
    }  

    /* Close the database */
    mco_db_close(db_name);
  }

  /* Stop eXtremeDB runtime */
  mco_runtime_stop();

  /* Free allocated memory */
  free( dev[0].dev.conv.ptr );
  free( dev[1].dev.conv.ptr );
    
  sample_pause_end("\n\nPress any key to continue . . . ");
  sample_os_shutdown();

  return ( MCO_S_OK == rc ? 0 : 1 );
}

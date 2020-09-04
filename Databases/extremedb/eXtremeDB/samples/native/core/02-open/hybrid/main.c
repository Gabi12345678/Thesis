/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <mixeddb.h>

#ifdef MEMORY_PAGE_SIZE
#undef MEMORY_PAGE_SIZE
#endif

#define  MEMORY_PAGE_SIZE      128
const char * db_name             = "hybrid";

int main(int argc, char* argv[])
{
  MCO_RET            rc;
  unsigned int       n_dev = 1;
  mco_runtime_info_t info;
  mco_device_t       dev[4];              /* Reserve memory for 4 device contexts */
  mco_db_params_t    db_params;
  const char *       text = "Sample 'hybrid' opens a hybrid database using 1 or 4 memory device contexts\n"
                            "depending on runtime settings determined from the database schema and\n"
                            "whether the sample is linked with disk-based or in-memory libraries.\n";

  sample_os_initialize(DEFAULT);

  mco_error_set_handler(&sample_errhandler);
  /* Start eXtremeDB runtime */
  mco_runtime_start();

  sample_header(text);

  /* Get runtime info before opening the database */
  mco_get_runtime_info(&info);
   
  /* Set up first memory device as a plain conventional or shared memory region depending on runtime settings */
  dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* Assign the device as a main database memory */
  dev[0].size       = DATABASE_SIZE;          /* Set the device size */
  if (info.mco_shm_supported) {
    dev[0].type       = MCO_MEMORY_NAMED;           /* Set the device as a shared named memory device */
    sprintf( dev[0].dev.named.name, "%s-db", db_name ); /* Set memory name */
    dev[0].dev.named.flags = 0;                     /* Zero flags */
    dev[0].dev.named.hint  = 0;                     /* Set mapping address or null it */
  } else {
    dev[0].type       = MCO_MEMORY_CONV;            /* Set the device as a conventional memory device */
    dev[0].dev.conv.ptr = (void*)malloc( DATABASE_SIZE ); /* Allocate memory and set device pointer */
  }

  /* If disk-based, set up the other 3 memory device contexts depending on runtime settings */
  if (info.mco_disk_supported) {
    /* Set up memory device for cache */
    dev[1].assignment = MCO_MEMORY_ASSIGN_CACHE;      /* Assign the device as cache */
    dev[1].size       = CACHE_SIZE;
    if (info.mco_shm_supported) {
      dev[1].type       = MCO_MEMORY_NAMED;           /* Set the device as shared named memory device */
      sprintf( dev[1].dev.named.name, "%s-cache", db_name ); /* Set memory name */
      dev[1].dev.named.flags = 0;                     /* Zero flags */
      dev[1].dev.named.hint  = 0;                     /* Set mapping address or null it */
    } else {
      dev[1].type       = MCO_MEMORY_CONV;            /* Set the device as a conventional memory device */
      dev[1].dev.conv.ptr = (void*)malloc( CACHE_SIZE ); /* Allocate memory and set device pointer */
    }
    
    /* Set up memory device for main database storage */
    dev[2].type       = MCO_MEMORY_FILE;                /* Set the device as a file device */
    dev[2].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;   /* Assign the device as a main database persistent storage */
    sprintf( dev[2].dev.file.name, FILE_PREFIX "%s.dbs", db_name ); /* Name the device */
    dev[2].dev.file.flags = MCO_FILE_OPEN_DEFAULT;      /* Set the device flags */

    /* Set up memory device for transaction log */
    dev[3].type       = MCO_MEMORY_FILE;                /* Set the device as a file device */
    dev[3].assignment = MCO_MEMORY_ASSIGN_LOG;          /* Assign the device as a log */
    sprintf( dev[3].dev.file.name, FILE_PREFIX "%s.log", db_name ); /* Name the device */
    dev[3].dev.file.flags = MCO_FILE_OPEN_DEFAULT;      /* Set the device flags */

    n_dev            += 3;
  }
    
  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* Initialize the params with default values */

  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* Set page size for in-memory part */
  /* Set page size for persistent storage or zero if not disk-based */
  db_params.disk_page_size     = info.mco_disk_supported ? PSTORAGE_PAGE_SIZE : 0;   
  db_params.db_max_connections = 1;                /* Set total number of connections to the database */
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif
  
  /* Open a database on the device with given params */
  rc = mco_db_open_dev(db_name, mixeddb_get_dictionary(), dev, n_dev, &db_params );
  if ( MCO_S_OK == rc ) {

    /* The database was opened successfully */
    sample_rc_check("\tOpen database", rc );

    /* Show runtime info for opened database */
    sample_show_runtime_info("\n\n\tThe opened database has the following characteristics:\n");

    /* Close the database */
    mco_db_close(db_name);
    sample_rc_check("\tClose database", rc );
      
  } else {
    /* Unable to open the database */
    /* Check the return code for additional informaton */
    sample_rc_check("\tDatabase Open failed", rc );
  }
    
  /* Stop eXtremeDB runtime */
  mco_runtime_stop();

  /* If not shared memory, free allocated memory */
  if (! info.mco_shm_supported) {
      free( dev[0].dev.conv.ptr );
      if (info.mco_disk_supported) {
          free( dev[1].dev.conv.ptr );
      }
  }

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

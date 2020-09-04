/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <shareddb.h>

const char * db_name             = "shareddb";

int main(int argc, char* argv[])
{
  MCO_RET            rc;
  mco_device_t       dev;
  mco_db_params_t    db_params;

#ifdef _INTEGRITY
  mco_sync_first_semaphore_index = 14;
  mco_sync_last_semaphore_index  = 23;
  mco_registry_object_index      = 12; 
#endif

  sample_os_initialize(DEFAULT);

  /* Start eXtremeDB runtime */
  mco_runtime_start();
   
  sample_header("Sample 'shared' opens a database using shared memory.\n" );

  /* Set up the memory device as a shared named memory region */
  dev.assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* Assign the device as a main database memory */
  dev.size       = DATABASE_SIZE;                  /* Set the device size */
#ifdef _INTEGRITY
  dev.type       = MCO_MEMORY_INT_DESC;            /* set the device as a shared named memory device */
  dev.dev.idesc.handle  = 13;                      /* see app_integrate.h */
#else 
  dev.type       = MCO_MEMORY_NAMED;               /* Set the device as a shared named memory device */
  sprintf( dev.dev.named.name, "%s-db", db_name ); /* Set memory name */
  dev.dev.named.flags = 0;                         /* Zero flags */
  dev.dev.named.hint  = 0;                         /* Set mapping address or null it */
#endif

  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* Initialize the params with default values */

  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* Set page size for the in-memory part */
  db_params.disk_page_size     = 0;                /* Set page size to zero to disable disk operations */
  db_params.db_max_connections = 1;                /* Set total number of connections to the database */
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif
    
  /* Open a database on the device with given params */
  rc = mco_db_open_dev(db_name, shareddb_get_dictionary(), &dev, 1, &db_params );
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
 
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();

  return ( MCO_S_OK == rc ? 0 : 1 );
}

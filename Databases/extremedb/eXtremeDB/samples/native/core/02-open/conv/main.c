/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <convdb.h>

#define  DATABASE_SEGMENT_SIZE 300 * 1024
const char * db_name             = "conv";

int main(int argc, char* argv[])
{
  MCO_RET            rc;

  mco_device_t       dev;
  mco_db_params_t    db_params;

  sample_os_initialize(DEFAULT);

  /* Start eXtremeDB runtime */
  rc = mco_runtime_start();

  sample_rc_check("Initialize MCO runtime", rc );
   
  sample_header("\nSample 'conv' opens a database using conventional memory.\n" );

  /* Setup memory device as a plain conventional memory region */
  dev.type       = MCO_MEMORY_CONV;                /* Set the device as a conventional memory device */
  dev.assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* Assign the device as main database memory */
  dev.size       = DATABASE_SEGMENT_SIZE;          /* Set the device size */
  dev.dev.conv.ptr = (void*)malloc( DATABASE_SEGMENT_SIZE ); /* Allocate memory and set device pointer */
  
  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* Initialize the params with default values */

  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* Set page size for the in-memory part */
  db_params.disk_page_size     = 0;                /* Set page size to zero to disable disk operations */
  db_params.db_max_connections = 1;                /* Set total number of connections to the database */
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif
    
  /* Open a database on the device with given params */
  rc = mco_db_open_dev(db_name, convdb_get_dictionary(), &dev, 1, &db_params );
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
    sample_rc_check("\tError opening database", rc );
  }
    
  /* Stop eXtremeDB runtime */
  mco_runtime_stop();
 
  free( dev.dev.conv.ptr );
  
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

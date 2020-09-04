/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <convdb.h>
#include <mcouda.h>
#include <mcohv.h>

#define  DATABASE_SEGMENT_SIZE 300 * 1024
const char * db_name             = "convhv";

int main(int argc, char* argv[])
{
  MCO_RET            rc;

  unsigned int       size;
  mco_device_t       dev;
  mco_db_params_t    db_params;
  mco_metadict_header_t *metadict;
  mcohv_p            hv = 0;

  /* Start eXtremeDB runtime */
  mco_runtime_start();

  mco_error_set_handler(&sample_errhandler);

  /* Initialize MCOHV */
  mcohv_initialize();

  sample_header("Sample 'convhv' opens a database using conventional memory and allows browser \naccess to the database with http database viewer.\n" );

  /* Determine size of meta-dictionary (in bytes) and allocate memory */
  mco_metadict_size(1, &size);
  metadict = (mco_metadict_header_t *) (malloc(size));

  rc = mco_metadict_init (metadict, size, 0);
  sample_rc_check("\tInitialize UDA metadictionary", rc );

  /* Setup memory device as a plain conventional memory region */
  dev.type       = MCO_MEMORY_CONV;                /* Set the device as a conventional memory device */
  dev.assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* Assign the device as main database memory */
  dev.size       = DATABASE_SEGMENT_SIZE;          /* Set the device size */
  dev.dev.conv.ptr = (void*)malloc( DATABASE_SEGMENT_SIZE ); /* Allocate memory and set device pointer */
  
  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* Initialize the params with default values */

  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* Set page size for the in-memory part */
  db_params.disk_page_size     = 0;                /* Set page size to zero to disable disk operations */
  db_params.db_max_connections = 10;               /* Set total number of connections to the database */
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

  mco_metadict_register(metadict, db_name, convdb_get_dictionary(), NULL);

  /* Open a database on the device with given params */
  rc = mco_db_open_dev(db_name, convdb_get_dictionary(), &dev, 1, &db_params );
  if ( MCO_S_OK == rc ) {

    /* The database was opened successfully */
    sample_rc_check("\tOpen database", rc );

    /* Show runtime info for opened database */
    sample_show_runtime_info("\n\n\tThe opened database has the following characteristics:\n");

    rc = mcohv_start(&hv, metadict, 0, 0);
    sample_rc_check("\tStart HTTP database browser", rc );

    /* Simulate the progrm's event- or data-loop */
    sample_pause("\n\n\tVisit page http://localhost:8082/\n\tHit ENTER to stop HTTP server...");

    rc = mcohv_stop(hv);
    sample_rc_check("\tStop HTTP database browser", rc );

    /* Close the database */
    rc = mco_db_close(db_name);
    sample_rc_check("\tClose database", rc );
        
  } else {
    /* Unable to open the database */
    /* Check the return code for additional informaton */
    sample_rc_check("\tError opening database", rc );
  }
    
  /* Stop eXtremeDB runtime */
  mcohv_shutdown();
  mco_runtime_stop();
 
  free( dev.dev.conv.ptr );
  
  sample_pause_end("\n\nPress any key to continue . . . ");

  return ( MCO_S_OK == rc ? 0 : 1 );
}

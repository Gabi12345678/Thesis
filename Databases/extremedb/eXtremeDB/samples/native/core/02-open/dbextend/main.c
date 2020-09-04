/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <extenddb.h>

#define  SEGMENT_SIZE 200 * 1024       /* Size of each memory segment */
#define  MAX_SEGMENTS 10               /* Max number of segments */
const char * db_name = "extenddb";

int main(int argc, char* argv[])
{
  MCO_RET            rc;
  /* For this sample we use an array of device structures on the application stack.  Please note however,
   * that this is not necessary.  It is done here to minimize the code not directly related to the
   * samples' purpose which is to demonstrate how to extend memory.  Since the eXtremeDB runtime copies 
   * the device data to an internal structure, the same mco_device_t structure can be reset and reused 
   * by the application rather than allocating multiple device structures on the stack. 
   */
  mco_device_t       dev[MAX_SEGMENTS];               
  int                n_segments = 0;
  mco_db_params_t    db_params;
  mco_db_h           db;

  sample_os_initialize(DEFAULT);

  /* Set the sample error handler to be called from the eXtremeDB runtime if a fatal error occurs */
  mco_error_set_handler(&sample_errhandler);
  
  /* Start eXtremeDB runtime */
  mco_runtime_start();
   
  sample_header("Sample 'dbextend' extends a database using conventional memory.\n" );

  /* Set up memory device as a plain conventional memory region */
  dev[0].type         = MCO_MEMORY_CONV;               /* Set the device as a conventional memory device */
  dev[0].assignment   = MCO_MEMORY_ASSIGN_DATABASE;    /* Assign the device as main database memory */
  dev[0].size         = SEGMENT_SIZE;                  /* Set the device size */
  dev[0].dev.conv.ptr = (void*)malloc( SEGMENT_SIZE ); /* Allocate memory and set device pointer */
  n_segments++;                                        /* Increment the memory segment counter */

  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* Initialize the params with default values */
  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* Set page size for the in-memory part */
  db_params.disk_page_size     = 0;                /* Set page size to zero to disable disk operations */
  /* Set total number of connections to the database - mco_db_extend_dev() requires an additional connection */
  db_params.db_max_connections = 2;                
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif
      
  /* Open a database on the main device with given params */
  rc = mco_db_open_dev(db_name, extenddb_get_dictionary(), &dev[0], 1, &db_params );
  sample_rc_check("\tOpen database", rc );
  if ( MCO_S_OK == rc ) {

    /* Connect to database */
    rc = mco_db_connect(db_name, &db);
    sample_rc_check("\tConnect to database", rc );   
    if ( MCO_S_OK == rc ) {

      /* Extend the database until out of memory segments */
      while ( MAX_SEGMENTS > n_segments ) {
        dev[n_segments].type         = MCO_MEMORY_CONV;               /* Set the device as a conventional memory device */
        dev[n_segments].assignment   = MCO_MEMORY_ASSIGN_DATABASE;    /* Assign the device as a main database memory */
        dev[n_segments].size         = SEGMENT_SIZE;                  /* Set the device size */
        dev[n_segments].dev.conv.ptr = (void*)malloc( SEGMENT_SIZE ); /* Allocate memory and set device pointer */
        rc = mco_db_extend_dev(db_name, &dev[n_segments]);
        sample_rc_check("\tExtend database", rc );
        if ( MCO_S_OK != rc ) break;  /* If extend failed exit loop */
        n_segments++;
      }

      /* Disconnect from the database */
      rc = mco_db_disconnect(db);
      sample_rc_check("\tDisconnect from database", rc );
      /* Close the database */
      rc = mco_db_close(db_name);
      sample_rc_check("\tClose database", rc );
      /* Free all memory segments */
      while (n_segments) {
        free(dev[--n_segments].dev.conv.ptr);
      }

    } else {

      /* Connection failed: free memory allocated for main database device and close the database without resetting rc */
      free(dev[0].dev.conv.ptr);
      mco_db_close(db_name);
    }
  }   

  /* Stop eXtremeDB runtime */
  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

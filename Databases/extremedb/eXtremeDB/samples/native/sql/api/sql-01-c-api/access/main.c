/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mcoapic.h>
#include "c_apidb.h"
const char * db_name  = "ctestdb";

#define DISK_CACHE_SIZE (4*1024*1024)
#define DISK_PAGE_SIZE  4096

static void extremedb_error_handler(MCO_RET error_code)
{
    fprintf(stderr, "eXtremeDB error %d\n", error_code);
    sample_os_shutdown();
    dbg_exit(1);
}

int main(int argc, char** argv)
{
  MCO_RET           rc;
  status_t          rs;
  mco_db_h          db;
  mco_device_t      dev;
  mco_db_params_t   db_params;
  database_t        engine;

  sample_header("Sample 'access' opens a database using the eXtremeSQL C language API.\n" );
       
  /* set fatal error handler */
  mco_error_set_handler(extremedb_error_handler);

  /* start eXtremeDB runtime */
  mco_runtime_start();
   
  /* setup memory device as a plain conventional memory region */
  dev.type       = MCO_MEMORY_CONV;                /* set the device as a conventional memory device */
  dev.assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* assign the device as main database memory */
  dev.size       = DATABASE_SIZE;                  /* set the device size */
  dev.dev.conv.ptr = (void*)malloc( DATABASE_SIZE );
  if ( !dev.dev.conv.ptr ) {
    extremedb_error_handler(MCO_E_NOMEM);
  }
  
  /* initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* initialize the params with default values */
  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* set page size for the in memory part */
  db_params.disk_page_size     = 0;                /* set page size to zero to disable disk operations */
  db_params.db_max_connections = 1;                /* set total number of connections to the database */
  db_params.db_log_type        = REDO_LOG;         /* set transaction log type */
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif
    
  /* open a database on the device with given params */
  rc = mco_db_open_dev(db_name, c_apidb_get_dictionary(), &dev, 1, &db_params );
  if ( MCO_S_OK == rc ) {

    /* the database was opened successfully */
    sample_rc_check("\tOpen database", rc );

    /* connect to the database by name */   
    rc = mco_db_connect( db_name, &db );
   
    if ( MCO_S_OK == rc ) {

      /* the database is open and a connection successfully established */
      sample_rc_check("\tConnect database", rc );
       
      /* Set SQL memory allocator to C runtime malloc/free */
      rs = mcosql_initialize_memory_manager(&malloc, &free);
      sample_rc_check("\tInitialize SQL memory allocator", (MCO_RET)rs );

      /* Initialize eXtremeDB SQL mapper */
      rs = mcoapi_initialize(db);
      sample_rc_check("\tInitialize SQL runtime", (MCO_RET)rs );

      /* Open SQL engine */
      rs = mcosql_open( &engine );
      sample_rc_check("\tOpen SQL engine", (MCO_RET)rs );

      /* Do normal database processing ... */

      /* Close SQL engine and disconnect from database */
      rs = mcosql_close( engine );
      sample_rc_check("\tClose SQL engine and disconnect", (MCO_RET)rs );
    }

    /* close the database */
    rc = mco_db_close(db_name);
    sample_rc_check("\tClose database", rc );
 
  } else {
    /* unable to open the database */
    /* check the return code for additional informaton */
    sample_rc_check("\tOpen failed", rc );
  }

  /* stop eXtremeDB runtime */
  mco_runtime_stop();
 
  /* free allocated memory */
  free( dev.dev.conv.ptr );
  
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

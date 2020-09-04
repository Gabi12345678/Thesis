/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <simpledb.h>

const char * db_name = "singletask";

int main(int argc, char* argv[]) {
    
  MCO_RET         rc;
  sample_memory_t dbmem;

  sample_os_initialize(DEFAULT);

  /* Start eXtremeDB runtime */
  mco_runtime_start();

  sample_header("Sample 'singletask' opens and connects to a database from a single task.\n" );

  /* Use sample_open_database() to initialize memory device contexts, set database parameters and open the db */
  rc = sample_open_database( db_name, simpledb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &dbmem );                            
  sample_rc_check("\tOpen database", rc );

  if ( MCO_S_OK == rc ) {

    /* The database was opened successfully */
    /* Connect to it by name */   
    mco_db_h connection; /* Connection handle */
    rc = mco_db_connect( db_name, &connection );
   
    if ( MCO_S_OK == rc ) {
      /* The database is open and a connection successfully established */
      sample_rc_check("\tConnect database", rc );
       
       /* Perform desired database operations ... */
       
      /* Do not forget to disconnect it when done */
      rc = mco_db_disconnect( connection );
      sample_rc_check("\tDisconnect database", rc );
    }

    /* Close the database */
    sample_close_database( db_name, &dbmem );
 
  } else {
    /* Unable to open the database */
    /* Check the return code for additional informaton */
    sample_rc_check("\tOpen failed", rc );
  }
    
  /* Stop eXtremeDB runtime */
  mco_runtime_stop();
     
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

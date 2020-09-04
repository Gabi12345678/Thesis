/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <simpledb.h>

const char * db_name = "multiprocess";

int main(int argc, char* argv[]) {
    
  MCO_RET         rc;
  sample_memory_t dbmem;
  mco_db_h        connection; /* connection handle */

  sample_os_initialize(DEFAULT);

#ifdef _INTEGRITY
  mco_sync_first_semaphore_index = 14;
  mco_sync_last_semaphore_index  = 18;
  mco_registry_object_index      = 12; 
#endif

  sample_header("Sample 'multiprocess' opens and connects to a database from multiple processes.\n" );

  /* Start eXtremeDB runtime */

  /* Optionally set access mode, default is 0666 */
  mco_runtime_setoption(MCO_RT_OPTION_UNIX_SHM_MASK, 0600 );

  rc = mco_runtime_start();
  sample_rc_check("\tmco_runtime_start", rc );
  if ( rc == MCO_S_OK ) {

    /* try to connect to database first */
    rc = mco_db_connect( db_name, &connection );
    sample_rc_check("\tConnect", rc );
    
    if ( MCO_E_NOINSTANCE == rc ) {
        
      /* No db found. Open the database and perform tasks of the main process */
      rc = sample_open_database( db_name, simpledb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                                   MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &dbmem );
      sample_rc_check("\tOpen", rc );
                                     
      if ( MCO_S_OK == rc ) {

        /* Connect by name */
        rc = mco_db_connect( db_name, &connection );
        sample_rc_check("\tConnect Main process", rc );

        /* Perform tasks of the main process */
        printf("\n\n\tPerform tasks from main process...\n" );

        /* Disconnect */
        rc = mco_db_disconnect( connection );
        sample_rc_check("\tDisconnect Main process", rc );

        /* Opened */
        sample_pause("\n\n\tStart the program again to connect from a secondary process...\n");

        /* Close the database */
        sample_close_database( db_name, &dbmem );
      }
  
    } else if ( MCO_S_OK == rc ) {
      
      /* Database connection successful, perform tasks of the secondary process */
      printf("\n\n\tConnection successful, perform tasks from secondary process...\n" );

      /* Don't forget to disconnect when done */
      rc = mco_db_disconnect( connection );
      sample_rc_check("\tDisconnect secondary process", rc );
  
    } else {
      /* Handle unexpected error on connect */
    }
        
    /* Stop eXtremeDB runtime */
    mco_runtime_stop();
  }
     
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

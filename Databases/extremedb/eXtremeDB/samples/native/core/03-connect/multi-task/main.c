/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <simpledb.h>

const char * db_name = "multitask";

#define N_TASKS 5

void a_task ( sample_task_t * descriptor ) {
  MCO_RET rc;
  char msg[32];

  /* Every task has to make its own connection */
  mco_db_h connection; /* Connection handle */
  /* Connect to it by name */
  rc = mco_db_connect( db_name, &connection );
  sprintf( msg, "\t\tConnect Task %d", (int) descriptor->param ); 
  sample_rc_check(msg, rc );

  if ( MCO_S_OK == rc ) {
    /* Database successfully connected */

    /* Perform database tasks...*/

    /* Don't forget to disconnect when done */
    rc = mco_db_disconnect( connection );
    sprintf( msg, "\t\tDisconnect Task %d", (int) descriptor->param ); 
    sample_rc_check(msg, rc );
  }
}

int main(int argc, char* argv[]) {
    
  MCO_RET         rc;
  sample_memory_t dbmem;

  sample_os_initialize(DEFAULT);

  /* Start eXtremeDB runtime */
  mco_runtime_start();

  sample_header("Sample 'multitask' opens and connects to a database from multiple tasks.\n" );

  rc = sample_open_database( db_name, simpledb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, N_TASKS, &dbmem );                             
  sample_rc_check("\tOpen database", rc );

  if ( MCO_S_OK == rc ) {

    /* Task descriptors */
    sample_task_t task[N_TASKS];
    unsigned int  i;

    /* Start the tasks */
    for( i=0; i<sizeof(task)/sizeof(task[0]); i++ ) {
        sample_start_task( &task[i], a_task, (void*) i );
        printf("\n\tTask %d started", i );
    }

    /* Wait for completion */
    for( i=0; i<sizeof(task)/sizeof(task[0]); i++ ) {
        sample_join_task( &task[i] );
        printf("\n\tTask %d joined", i );
    }

    /* Close the database */
    sample_close_database( db_name, &dbmem );
    sample_rc_check("\n\tClose database", rc );
  }
  
  /* Stop eXtremeDB runtime */
  mco_runtime_stop();
     
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

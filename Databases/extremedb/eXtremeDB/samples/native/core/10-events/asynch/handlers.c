/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include "evdb.h"

/* Each of the event handler functions increments the visit count.
 * When the object is created, updated or deleted, the eXtremeDB
 * runtime wakes up the event handler thread and the call to
 * mco_async_event_wait() returns with MCO_S_OK.  When the
 * main thread explicitely releases the event, the thread is also
 * awakened, but with the return code MCO_S_EVENT_RELEASED.  This
 * return value breaks the while loop thus terminating the thread.
 */

/* Thread function that handles the <new> event. */
void NewEventHandler( sample_task_t * descriptor ) 
{
  int count = 0;
  /* use the connection created by sample_start_connected_task() */
  mco_db_h db = descriptor->db_connection; 

  while ( MCO_S_OK == mco_async_event_wait(db, MCO_EVENT_newEvent) ) {
    printf("\tNewEventHandler call %d\n", count);
    count++;
  }
  printf("\tExiting NewEventHandler. Number of calls : %d\n", count);
}


/* Thread function that handles the <update> event. */
void UpdateEventHandler( sample_task_t * descriptor ) 
{
  int count = 0;
  /* use the connection created by sample_start_connected_task() */
  mco_db_h db = descriptor->db_connection; 

  while ( MCO_S_OK == mco_async_event_wait(db, MCO_EVENT_updateEvent) ) {
    printf("\tUpdateEventHandler call %d\n", count);
    count++;
  }
  printf("\tExiting UpdateEventHandler. Number of calls : %d\n", count);
}


/* Thread function that handles the <delete> event. */
void DeleteEventHandler( sample_task_t * descriptor ) 
{
  int count = 0;
  /* use the connection created by sample_start_connected_task() */
  mco_db_h db = descriptor->db_connection; 

  while ( MCO_S_OK == mco_async_event_wait(db, MCO_EVENT_deleteEvent) ) {
    printf("\tDeleteEventHandler call %d\n", count);
    count++;
  }
  printf("\tExiting DeleteEventHandler. Number of calls : %d\n", count);
}

/* Thread function that handles the <update> event. */
void CheckpointEventHandler( sample_task_t * descriptor ) 
{
    int count = 0;
    /* use the connection created by sample_start_connected_task() */
    mco_db_h db = descriptor->db_connection; 
    
    while ( MCO_S_OK == mco_async_event_wait(db, MCO_EVENT_checkpointEvent) ) {
        printf("\tCheckpointHandler call %d\n", count);
        count++;
    }
    printf("\tExiting CheckpointEventHandler. Number of calls : %d\n", count);
}


/* Starts all event-handler threads */
void start_handlers(const char *db_name, sample_task_t *tasks)
{
  sample_start_connected_task(&tasks[0], NewEventHandler, db_name, 0);
  sample_start_connected_task(&tasks[1], UpdateEventHandler, db_name, 0);
  sample_start_connected_task(&tasks[2], DeleteEventHandler, db_name, 0);
  sample_start_connected_task(&tasks[3], CheckpointEventHandler, db_name, 0);
}

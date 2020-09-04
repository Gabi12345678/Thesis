/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include "evdb.h"

char sample_descr[] = {
  "Sample 'asynch' demonstrates eXtremeDB asynchronous events.\n" 
};
char explanation[] = {
  "\n\tThe schema declares <new>, <update> and <delete> events\n"
  "\tthat are registred at runtime.  The appropriate event handlers\n"
  "\t(call back functions) are called when an object is created,\n" 
  "\tdeleted or updated.\n"
  "\tThe event handlers execute in a separate thread that is blocked\n"
  "\twaiting on the event or explicit release by mco_async_release().\n\n"
};
const char db_name[]    = "asynch";

#define NOBJECTS 5 /* Number of objects in database */
#define NTHREADS 4 /* Number of handler threads (for new, update,checkpoint and delete events) */

/* Handler thread descriptors */
static sample_task_t tasks[NTHREADS];

/* This wait time allows the thread with the appropriate handler
 * to catch up and execute the handler.  In order to demonstrate
 * how the events are lost, change the "waits". Some of the events
 * could be left unhandled, since the thread had no chance to run!
 */
#define WAIT_TIME      100

/* Starts handler threads. Implemented in handlers.c */
void start_handlers(const char *db_name, sample_task_t *tasks);

/* Creates a new object triggering the <new_object> event */
MCO_RET new_object(mco_db_h db, autoid_t* id)
{
  mco_trans_h t;
  MyClass obj;
  MCO_RET rc;

  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    rc = MyClass_new(t, &obj);
    if ( MCO_S_OK == rc ) {
      MyClass_autoid_get(&obj, id);
      rc = mco_trans_commit(t);
    } else {
      mco_trans_rollback(t);
    }
  }
  return rc;
}

/* Updates an existing object triggering <update> events  */
MCO_RET update_object(mco_db_h db, autoid_t autoid)
{
  mco_trans_h t;
  MyClass obj;
  MCO_RET rc;
  uint4 u4;

  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    rc = MyClass_autoid_find(t, autoid, &obj);
    if ( MCO_S_OK == rc ) {
      MyClass_u4_get(&obj, &u4);
      MyClass_u4_put(&obj, u4 + 1);
      MyClass_checkpoint(&obj);
      rc = mco_trans_commit(t);
    } else {
      mco_trans_rollback(t);
    }
  }
  return rc;
}

MCO_RET del_object(mco_db_h db, autoid_t id)
{
  MCO_RET rc;
  mco_trans_h t;
  MyClass obj;

  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    rc = MyClass_autoid_find(t, id, &obj);
    if ( MCO_S_OK == rc ) {
      rc = MyClass_delete(&obj);
      if ( MCO_S_OK == rc ) {
        rc = mco_trans_commit(t);
      } else {
        mco_trans_rollback(t);
      }
    }
  }
  return rc;
}

int main(int argc, char* argv[])
{
  int i;
  autoid_t autoid;
  MCO_RET rc;
  mco_db_h db = 0;
  mco_runtime_info_t info;
  sample_memory_t dbmem;

  sample_os_initialize(DEFAULT);

  /* Check that events are supported by the runtime */
  mco_get_runtime_info(&info);
  if (!info.mco_events_supported) {
    printf("\nThis sample requires events support runtime");
    sample_pause_end("\n\nPress any key to continue . . . ");
    sample_os_shutdown();
    return 1;
  }

  sample_header(sample_descr);
  printf(explanation);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, evdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {
  
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Create threads and start waiting for the database events */
      start_handlers(db_name, tasks);

      for (i = 0; i < NOBJECTS && MCO_S_OK == rc; i++) {
        /* Allow the handler to execute and handle the event. Events are lost
         * (are unhandled) if they happen or are released when they are not 
         * "waited for" by a thread, or if the waiting thread has no chance 
         * to run. Putting the current thread to sleep will (probably) allow 
         * the handler to execute.
         */
        sample_sleep(WAIT_TIME);

        /* Create a new object, update and delete */
        rc = new_object(db, &autoid);
        if ( MCO_S_OK == rc ) {

          rc = update_object(db, autoid);
          if ( MCO_S_OK == rc ) {

            rc = del_object(db, autoid);
            printf("\n");
          }
        }
      }
      sample_sleep(WAIT_TIME);

      /* Release all the database events. This will cause the appropriate
      wait function to return MCO_S_EVENT_RELEASED */
      mco_async_event_release_all(db);

      /* Wait until all the threads are finished */
      for (i = 0; i < NTHREADS; i++) {
        sample_join_task (&tasks[i]);
      }

      mco_db_disconnect(db);
    }
    
     sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

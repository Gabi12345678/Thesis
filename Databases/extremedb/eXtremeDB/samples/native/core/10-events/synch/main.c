/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include "evdb.h"
#include "handlers.h"

char sample_descr[] = {
  "Sample 'synch' demonstrates eXtremeDB synchronous events.\n" 
};
char explanation[] = {
  "\n\tThe schema declares <new>, <update> and <delete> events\n"
  "\tthat are registered at runtime.  The appropriate event handlers\n"
  "\t(call back functions) are called when a new object is created,\n" 
  "\tdeleted or updated.\n"
  "\tThe event handlers print out data obtained from the object\n"
  "\thandle passed to the call back function by the eXtremeDB\n"
  "\truntime. In this sample the <delete> handler returns a non-zero\n"
  "\tvalue for odd numbers to cause the transaction to be rolled back.\n"
  "\n\tNB: Note that the handlers are executed on the current stack,\n"
  "\tand a database transaction is active for the duration.\n\n"
};
const char * db_name = "synch";
const int NOBJECTS  = 10;

/* Create a new object triggering the <new_object> event */
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

/* Updates an existing object triggering the <update> event */
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
      MyClass_u4_put(&obj, (uint4)autoid+NOBJECTS);
      MyClass_checkpoint(&obj);
      rc = mco_trans_commit(t);
    } else {
      mco_trans_rollback(t);
    }
  }
  return rc;
}

/* This function starts a transaction to delete an object
 * identified by its autoid. Before the transaction is committed,
 * a delete_handler() is called, which in turn returns a non-zero 
 * return code for every other object. This causes the current 
 * "delete" transaction to be aborted at commit time.
 */
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

/* Delete all database entries */
MCO_RET eraseDB(mco_db_h db)
{
  mco_cursor_t csr;
  mco_trans_h t;
  MyClass obj;
  MCO_RET rc;

  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    /* Initialize cursor */
    rc = MyClass_autoid_index_cursor(t, &csr);
    if ( MCO_S_OK == rc ) {

      rc = mco_cursor_first(t, &csr);
      while ( MCO_S_OK == rc ) {
        autoid_t id;
        
        MyClass_from_cursor(t, &csr, &obj);
        MyClass_autoid_get(&obj, &id);
        rc = mco_cursor_next(t, &csr);
        MyClass_delete(&obj);
        printf("\tObject (%d) deleted\n", autoid2int(id));
      }
    }

    rc = mco_trans_commit(t);
  }

  return rc;
}

int main(int argc, char* argv[])
{
  int i;
  autoid_t id;
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

      rc = register_events(db);
      if ( MCO_S_OK == rc ) {

        for (i = 0; i < NOBJECTS && MCO_S_OK == rc; i++)  {
          rc = new_object(db, &id);   
          if ( MCO_S_OK == rc ) {

            rc = update_object(db, id);
            if ( MCO_S_OK == rc ) {

              rc = del_object(db, id);
              printf(( MCO_S_OK == rc ) ? "\n\tTransaction committed\n\n" : "\n\tTransaction rolled back\n\n");
              rc = MCO_S_OK;
            }
          }
        }

        /* Unregister handlers so eraseDB() doesn't fire <delete> events */
        unregister_events(db);
        eraseDB(db);
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

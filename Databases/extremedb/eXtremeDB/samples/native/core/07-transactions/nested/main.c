/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <transdb.h>

char sample_descr[] = "Sample 'nested' demonstrates a nested transaction.\n";
const char * db_name = "nested";

MCO_RET insert_one(mco_db_h db, uint4 from, uint4 to );

/* Insert two Transaction records  */
MCO_RET insert_two(mco_db_h db, uint4 from1, uint4 to1, uint4 from2, uint4 to2)
{
  MCO_RET rc;
  mco_trans_h t;
  Transaction  trans;

  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( rc != MCO_S_OK) return rc;

  rc = Transaction_new(t, &trans);
  if ( MCO_S_OK != rc ) {
    mco_trans_rollback(t);
    return rc;
  }
  /* Put values in first 'new' object */
  Transaction_from_put(&trans, from1);
  Transaction_to_put(&trans, to1);

  /* Call nested transaction in insert_one() to insert second object */
  insert_one(db, from2, to2 );

  /* Now commit the transaction to complete the insert of the first object */
  return mco_trans_commit(t);
}

/* Insert one Transaction record within a read-write transaction */
MCO_RET insert_one(mco_db_h db, uint4 from, uint4 to )
{
  MCO_RET rc;
  mco_trans_h t;
  Transaction  trans;

  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( rc != MCO_S_OK) return rc;
  
  rc = Transaction_new(t, &trans);
  if ( MCO_S_OK != rc ) {
    mco_trans_rollback(t);
    return rc;
  }
  Transaction_from_put(&trans, from);
  Transaction_to_put(&trans, to);
  return mco_trans_commit(t);
}

int main(int argc, char* argv[]) 
{    
  MCO_RET rc;
  mco_db_h db = 0;
  sample_memory_t dbmem;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  /* Open and connect to database */
  rc = sample_open_database( db_name, transdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Perform a simple nested transaction... */
      uint4 from1 = 11, to1 = 16, from2 = 7, to2 = 17;   /* Meaningless values just to provide uint4 arguments */
      rc = insert_two(db, from1, to1, from2, to2);
      sample_rc_check("\tInsert two objects", rc );
  
      mco_db_disconnect(db);
    }

    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

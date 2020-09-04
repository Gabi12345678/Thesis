/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <simpledb.h>

char sample_descr[] = "Sample 'policy' demonstrates the different eXtremeDB Transaction Policies.\n";
const char * db_name = "policy";

#define NUMOBJECTS    1000

mco_db_h db = 0;

MCO_RET do_insert()
{
  MCO_RET rc = MCO_S_OK;
  uint4 i;
  int t1, t2;
  mco_trans_h t;
  A a;

  /* Start the timer, write NUMOBJECTS into the database */
  t1 = sample_msec();

  for (i = 0; i < NUMOBJECTS && MCO_S_OK == rc; i++)
  {
    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    if ( MCO_S_OK == rc ) {
      rc = A_new(t, &a);
      rc = A_ui4_put(&a, i);
      rc = mco_trans_commit(t);
    }
    if ((i % 100) == 0)
    {
        printf(".");
    }
  }

  if ( MCO_S_OK == rc ) {
    /* Flush file system buffers to disk */
    rc = mco_disk_flush(db);
  }

  t2 = sample_msec();

  printf("\n\tTotal %d objects written in %d msec\n", i, (int)(t2 - t1));
  return rc;
}

int main(int argc, char* argv[]) 
{    
  MCO_RET rc;
  sample_memory_t dbmem;
  static mco_log_params_t log_params;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  /* Open and connect to database */
  rc = sample_open_database( db_name, simpledb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Insert objects using the different Transaction Policies */
      printf("\n\tWriting objects using transaction policy - MCO_COMMIT_BUFFERED.\n\t");
      mco_disk_transaction_policy(db, MCO_COMMIT_BUFFERED);
      rc = do_insert();

      if ( MCO_S_OK == rc ) {
        printf("\n\tWriting objects using transaction policy - MCO_COMMIT_NO_SYNC.\n\t");
        mco_disk_transaction_policy(db, MCO_COMMIT_NO_SYNC);
        rc = do_insert();
      }

      if ( MCO_S_OK == rc ) {
        printf("\n\tWriting objects using transaction policy - MCO_COMMIT_SYNC_FLUSH.\n\t");
        mco_disk_transaction_policy(db, MCO_COMMIT_SYNC_FLUSH);
        rc = do_insert();
      }
    
      if ( MCO_S_OK == rc ) {
        log_params.delayed_commit_threshold = CACHE_SIZE/2;
        mco_disk_set_log_params(db, &log_params);
        
        printf("\n\tWriting objects using transaction policy - MCO_COMMIT_DELAYED.\n\t");
        mco_disk_transaction_policy(db, MCO_COMMIT_DELAYED);
        rc = do_insert();
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

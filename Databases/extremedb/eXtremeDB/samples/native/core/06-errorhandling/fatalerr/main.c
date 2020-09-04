/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <errordb.h>

char sample_descr[] = {
  "Sample 'fatalerr' demonstrates calling a user-defined error handler.\n"
};
const char * db_name = "errordb";

/* Implement error handler */
static void errhandler(MCO_RET n)
{
    printf("\neXtremeDB runtime fatal error: %d", n);
    getchar();
    sample_os_shutdown();
    exit( 0 );
}

int main(int argc, char* argv[])
{
  MCO_RET   rc;
  mco_db_h  db;
  sample_memory_t dbmem;
  anObject  rec;
  uint4     data = 1;
  mco_trans_h t;

  sample_header(sample_descr);

  /* Set the error handler to be called from the eXtremeDB runtime if a fatal error occurs */
  mco_error_set_handler(&errhandler);

  /* Start eXtremeDB runtime */
  mco_runtime_start();

  /* Open and connect to database */
  rc = sample_open_database( db_name, errordb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, 0, 1, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    sample_rc_check("\tConnect to database", rc );  
    if ( MCO_S_OK == rc ) {

      /* Start a READ_ONLY transaction to cause anObject_data_put() to fail ... */
      rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
      sample_rc_check("\tStart Transaction", rc );
      if ( MCO_S_OK == rc ) {
        printf("\n\n\tThe following attempt to create a new record\n"
               "\tshould cause the Error handler to be called with Fatal\n"
               "\tError 340049 because it requires a READ_WRITE transaction.\n"
               "\tThe type of transaction started was MCO_READ_ONLY...\n"
               "\t(Note: If linked with a 'no-check' runtime you will get\n"
               "\tan error code instead of a fatal error.)\n");
        /* anObject_new() should fail with error code 340049 = MCO_ERR_TRN + 49 */
        rc = anObject_new(t, &rec);
        sample_rc_check("\tNew record", rc );
        if ( MCO_S_OK == rc ) {
          rc = anObject_data_put(&rec, data);

          /* The following code will not be reached unless the transaction is changed to MCO_READ_WRITE */
          sample_rc_check("\tPut a value to new record", rc );
          if ( MCO_S_OK == rc ) {
            rc = mco_trans_commit(t);
            sample_rc_check("\tCommit Transaction", rc );
          } 
        } else if (rc == MCO_E_ACCESS) {
          printf("\nThe sample was linked with a no-check runtime\n");
          rc = MCO_S_OK;
        }
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

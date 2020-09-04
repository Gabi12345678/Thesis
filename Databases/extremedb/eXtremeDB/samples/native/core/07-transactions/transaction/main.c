/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <simpledb.h>

char sample_descr[] = "Sample 'transaction' demonstrates the different eXtremeDB transaction types.\n";
const char * db_name = "transaction";

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
  rc = sample_open_database( db_name, simpledb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      mco_trans_h t;

      /* Start read-only transaction */
      rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        /* A ReadOnly transaction is intended for lookups and data extraction.
          Mutiple R-O transactions can be started simultaneously with the MURSIW and MVCC transaction managers.
          But only one R-O transaction can be started with the EXCLUSIVE transaction manager.
        */
        printf( "\n\n\tA ReadOnly transaction has been successfully started.\n"
                "\n\t\tR-O transactions are intended for lookups and data extraction."
                "\n\t\tMultiple R-O transactions can be started simultaneously with"
                "\n\t\tthe MURSIW and MVCC transaction managers.  But only one R-O"
                "\n\t\tcan be started with the EXCLUSIVE transaction manager.\n" );

        /* Do lookups and data extraction */    

        /* Commit or rollback - there are no differences for read-only transactions */
        mco_trans_rollback( t ); /* or mco_trans_commit( t ); */

      } else {
        sample_rc_check("\tOpen R-O transaction failed", rc );
      }

      /* Start read-write transaction */
      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        /* R-W transactions are intended for lookups and data extraction, as well as creation and deletion.
          Only one R-W transaction at a time can be started with MURSIW and EXCLUSIVE transaction managers.
          Multiple R-W transactions can be started in parallel with the MVCC transaction manager 
          (refer to the User's Guide regarding isolation levels and resolution of collisions).
        */
        printf( "\n\n\tA ReadWrite transaction has been successfully started.\n"
              "\n\t\tR-W transactions are intended for lookups and data extraction,"
              "\n\t\tas well as data creation and deletion."
              "\n\t\tOnly one R-W transaction at a time can be started with"
              "\n\t\tthe MURSIW and EXCLUSIVE transaction managers."
              "\n\t\tMultiple R-W transactions can be started in parallel with"
              "\n\t\tthe MVCC transaction manager (refer to the Users Guide"
              "\n\t\tregarding isolation levels and resolution of collisions).\n" );

        /* Do lookups, create and destroy objects, modify data */

        /* It is possible to force indexes to be updated during the transaction using mco_trans_checkpoint() */
        mco_trans_checkpoint( t );
        sample_rc_check("\t\tTransaction Checkpoint", rc );
                      
        /* Commit to confirm and store changes or rollback to cancel updates made during this transaction */
        mco_trans_commit( t ); /* Or mco_trans_rollback( t ); */

      } else {
        sample_rc_check("\tOpen R-W transaction failed", rc );
      }

      /* Start read-only transaction and upgrade it to read-write transaction */
      rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {

        MCO_TRANS_TYPE t_type;

        /* Transactions can be upgraded to R-W to update data.
        */
        printf( "\n\n\tTransactions can be upgraded to R-W to update data.\n" );

        /* Get transaction type */
        mco_trans_type( t,  &t_type );
        printf( "\n\t\tTransaction type is %s\n", t_type == MCO_READ_ONLY ? "R-O" : "R-W" );

        /* Upgrade it to R-W */
        rc = mco_trans_upgrade(t);
        sample_rc_check("\t\tUpgrade transaction", rc );

        /* Get transaction type again*/
        mco_trans_type( t,  &t_type );
        printf( "\n\n\t\tNow Transaction type is %s\n", t_type == MCO_READ_ONLY ? "R-O" : "R-W" );

        mco_trans_commit( t );

      } else {
        sample_rc_check("\tOpen R-O transaction failed", rc );
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

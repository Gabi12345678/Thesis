/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <simpledb.h>

const char * db_name = "ttl";

int main(int argc, char* argv[]) {

  MCO_RET         rc;
  sample_memory_t dbmem;

  sample_os_initialize(DEFAULT);

  /* start eXtremeDB runtime */
  mco_runtime_start();

  sample_header("Sample 'ttl' demonstrates TTL policies.\n");
  rc = sample_open_database(db_name, simpledb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE,
                            MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &dbmem);

  sample_rc_check("\tOpen", rc);

  if ( MCO_S_OK == rc ) {

    mco_db_h connection; /* connection handle */

    /* the database was opened succeeded */
    /* connect it by name */

    rc = mco_db_connect(db_name, &connection);
    sample_rc_check("\tConnect", rc);

    if ( MCO_S_OK == rc ) {

      /* the database is connected successfully */
      mco_trans_h t;

      /* create objects of all three classes */
      rc = mco_trans_start(connection, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      sample_rc_check("\tOpen Transaction", rc);
      if ( MCO_S_OK == rc ) {
        A         a;   /* object A handle */
        B         b;   /* object B handle */
        C         c;   /* object C handle */
        int       i;
        const int n_objects = 10;
        
        printf("\n\tCreating %d objects of classes A, B, C\n", n_objects);
        for ( i = 0; i < n_objects; i++ ) {
          A_new(t, &a);
          B_new(t, &b);
          C_new(t, &c);

          A_i_put(&a, i);
          B_s_put(&b, db_name, (uint2)strlen(db_name));
          C_d_put(&c, 3.14f);

          /* sleep 100 ms to demonstrate TTL_clock_time policy */
          sample_sleep(100);
        }

        /* TTL policies are checked before the first stage of the commit */
        /* thus, the objects need to be inserted into indices using the checkpoint */
        rc = mco_trans_checkpoint(t);
        sample_rc_check("\tCheckpoint Transaction", rc);

        rc = mco_trans_commit(t);
        sample_rc_check("\tCommit Transaction", rc);


        printf("\n\tObjects count:\n");

        /* start a read-only transaction to count objects of each class */
        rc = mco_trans_start(connection, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {
          mco_cursor_t csr;
          int count = 0;

          rc = A_list_cursor(t, &csr);
          if ( MCO_S_OK == rc ) {
            rc = mco_cursor_first(t, &csr);
            for ( count = 0; MCO_S_OK == rc; rc = mco_cursor_next(t, &csr) ) {
              count++;
            }
            mco_cursor_close(t, &csr);
          }
          printf("\t\tA: %d\n", count);

          count = 0;
          rc = B_list_cursor(t, &csr);
          if ( MCO_S_OK == rc ) {
            rc = mco_cursor_first(t, &csr);
            for ( count = 0; MCO_S_OK == rc; rc = mco_cursor_next(t, &csr) ) {
              count++;
            }
            mco_cursor_close(t, &csr);
          }
          printf("\t\tB: %d\n", count);

          count = 0;
          rc = C_list_cursor(t, &csr);
          if ( MCO_S_OK == rc ) {
            rc = mco_cursor_first(t, &csr);
            for ( count = 0; MCO_S_OK == rc; rc = mco_cursor_next(t, &csr) ) {
              count++;
            }
            mco_cursor_close(t, &csr);
          }
          printf("\t\tC: %d\n", count);

          rc = mco_trans_commit(t);
        }
      }

      /* don't forget to disconnect when done */
      rc = mco_db_disconnect(connection);
      sample_rc_check("\tDisconnect", rc);
    }

    /* close the database */
    sample_close_database(db_name, &dbmem);
    sample_rc_check("\tClose", rc);
  }

  /* stop eXtremeDB runtime */
  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();

  return ( MCO_S_OK == rc ? 0 : 1 );
}

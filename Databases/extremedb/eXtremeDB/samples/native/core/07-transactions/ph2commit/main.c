/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include "ph2db.h"

char sample_descr[] = "Sample 'ph2commit' demonstrates a two-phase commit transaction to two \nconventional memory databases.\n";
const char * db_name1             = "ph2conv-db1";
const char * db_name2             = "ph2conv-db2";

const unsigned int nRecords = 10;

void init_database(mco_db_h db, uint4 start_key)
{
  uint4 key = start_key, i;
  MCO_RET rc;
  mco_trans_h t;
  Record rec;

  for (i = 0; i < nRecords; ++i, key += 3) {
    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    if ( MCO_S_OK == rc ) {
      rc = Record_new(t, &rec);
      Record_key_put(&rec, key);
      rc = mco_trans_commit(t);
    }    
  }
}

/* Check database instance. Returns 0 if OK, 1 otherwise */
int check_database(mco_db_h db, int instance)
{
  mco_cursor_t csr;
  Record rec;
  MCO_RET rc;
  mco_trans_h t;
  uint4 key = 0, i;

  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    Record_tkey_index_cursor(t, &csr);
    for (rc = mco_cursor_first(t, &csr), i = 0; MCO_S_OK == rc; rc = mco_cursor_next(t, &csr), ++i) {
      Record_from_cursor(t, &csr, &rec);
      Record_key_get(&rec, &key);
      if (key != (i / 2) * 3 + (i % 2) * instance) {
        printf("check database #%d: readed key %d, expected key %d\n", instance, key, (i / 2) * 3 + (i % 2) * instance);        
        mco_trans_rollback(t);  
        return 1;
      }
    }
    mco_trans_rollback(t);

    if (i != nRecords * 2) {
      printf("check database #%d: wrong record num : %d, expected %d\n", instance, i, nRecords * 2);
      return 1;
    }
  } else {
    printf("Can't start transaction : %s\n", mco_ret_string(rc, 0));        
    return 1;
  }
  return 0;
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db1 = 0, db2 = 0;
  mco_trans_h t1, t2;
  Record rec1, rec2;
  uint4 key, i;
  int exit_code = 0;
  sample_memory_t dbmem1, dbmem2;
  char msg[128];
    
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open databases */
  rc = sample_open_database( db_name1, ph2db_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem1 );
  if ( MCO_S_OK == rc ) {

    rc = sample_open_database( db_name2, ph2db_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem2 );
    if ( MCO_S_OK == rc ) {
   
      rc = mco_db_connect(db_name1, &db1);
      if ( MCO_S_OK == rc ) {

        rc = mco_db_connect(db_name2, &db2);
        if ( MCO_S_OK == rc ) {
    
          /* Insert records with keys 3 * N + 1 to database1 */
          init_database(db1, 1);
          printf("\n\tDatabase 1 initialized\n");

          /* Insert records with keys 3 * N + 2 to database2 */
          init_database(db2, 2);
          printf("\n\tDatabase 2 initialized\n");
    
          /* Try to insert records to database1 and database2 simultaneously */
          for (i = 0; i < nRecords * 3; ++i) {
            key = i;
            rc = mco_trans_start(db1, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t1);
            if ( MCO_S_OK == rc ) {
              rc = mco_trans_start(db2, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t2);
              if ( MCO_S_OK == rc ) {
                rc = Record_new(t1, &rec1);
                if ( MCO_S_OK == rc ) {
                  Record_key_put(&rec1, key);
                  rc = Record_new(t2, &rec2);
                  if ( MCO_S_OK == rc ) {
                    Record_key_put(&rec2, key);
                  }
                }
              }
            }
            rc = mco_trans_commit_phase1(t1);
            if ( MCO_S_OK == rc ) {
              rc = mco_trans_commit_phase1(t2);
            }

            /* Only records with key (3 * k) should be added */

            if ( MCO_S_OK != rc ) {
              /* Failed to commit phase 1, rollback both transactions... */
              mco_trans_rollback(t1);
              mco_trans_rollback(t2);
              if (key % 3 == 0) {
                sample_rc_check("\n\t2-Phase commit failed with valid key", rc );
                sample_pause_end("\n\nPress any key to continue . . . ");
              }
            } else {
              /* Committed phase 1, commit phase 2 ... */
              mco_trans_commit_phase2(t1);
              mco_trans_commit_phase2(t2);
              if (key % 3 != 0) {
                sample_rc_check("\n\t2-Phase commit failed with duplicate key", rc );
                sample_pause_end("\n\nPress any key to continue . . . ");
              } else {
                printf("\n\t2-Phase commited successful\n");
              }
            }
          }          

          /* Check databases */
          exit_code = check_database(db1, 1);
          printf("\n\tCheck database 1 : %s\n", exit_code ? "FAILED" : "OK");

          if (exit_code == 0) {
            exit_code = check_database(db2, 2); 
            printf("\n\tCheck database 2 : %s\n", exit_code ? "FAILED" : "OK");
          }
          
          mco_db_disconnect(db2);
        }
        
        mco_db_disconnect(db1);
      }
      
      sample_close_database(db_name2, &dbmem2);
    }
  
    sample_close_database(db_name1, &dbmem1);
  }

  /* Stop runtime and return exit code */
  mco_runtime_stop();
 
  sprintf(msg, "\nTerminating with exit code %d.\n\nPress any key to continue . . . ", exit_code );
  sample_pause_end(msg);

  sample_os_shutdown();
  return exit_code;
}

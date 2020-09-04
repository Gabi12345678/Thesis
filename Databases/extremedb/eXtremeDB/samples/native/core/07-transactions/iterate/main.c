/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include "iterate.h"

char sample_descr[]  = "Sample 'iterate' demonstrates how to iterate through\n"
"committed objects within a two-phase transaction.\n";
const char * db_name = "iterate-db";

const unsigned int nRecords = 4;
volatile int read_deleted_object = 1;

MCO_RET iteration_proc(mco_trans_h trans, MCO_Hf* obj, int cid, int obj_state, void* user_ctx)
{
  MCO_RET rc = MCO_S_NOTFOUND;
  uint4 key;

  if (cid == Record_code)
  {
    if ( MCO_TRANS_OBJ_ALL_DELETED & obj_state )
    {
      if (read_deleted_object)
      {
        rc = Record_key_get((Record *)obj, &key);
        if ( MCO_S_OK == rc ) {
          printf ("%s records starting with key %d\n", (char *)user_ctx, (int)key);
        }
      }
    }
    else if ( MCO_TRANS_OBJ_DELETED & obj_state )
    {
      rc = Record_key_get((Record *)obj, &key);
      if ( MCO_S_OK == rc ) {
        printf ("%s record with key %d\n", (char *)user_ctx, (int)key);
      }
    }
    else if ( MCO_TRANS_OBJ_CREATED & obj_state )
    {
      rc = Record_key_get((Record *)obj, &key);
      if ( MCO_S_OK == rc ) {
        printf ("%s record with key %d\n", (char *)user_ctx, (int)key);
      }
    }
    else
    {
      rc = Record_key_get((Record *)obj, &key);
      if ( MCO_S_OK == rc ) {
        if ( 0 == key ) {
          printf ("\tSimulate an error in 'iteration_proc' for record with key 0\n");
          return MCO_ERR_LAST + 1;
        } else {
          printf ("%s record with key %d\n", (char *)user_ctx, (int)key);
        }
      }
    }
  }

  if ( MCO_S_OK != rc ) {
    printf("Error %d (%s)\n", rc, mco_ret_string(rc, 0));
    return MCO_ERR_LAST + 1;
  } else {
    return MCO_S_OK;
  }
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  mco_trans_h t;
  Record rec;
  uint4 key, i;
  sample_memory_t dbmem;
  mco_runtime_info_t info;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  mco_get_runtime_info(&info);
  /* It is impossible to read any of deleted_all object if DISK + MURSIW configuration used */
  if (info.disk_supported && !info.concurrent_write_transactions)
    read_deleted_object = 0;

  /* Open database */
  rc = sample_open_database( db_name, iterate_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {
      /* Insert records to database */
      for (i = 0; i < nRecords; ++i) 
      {
        key = i;
        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {
          rc = Record_new(t, &rec);
          if ( MCO_S_OK == rc ) {
            rc = Record_key_put(&rec, key);
            if ( MCO_S_OK == rc ) {
              rc = mco_trans_commit_phase1(t);
              if ( MCO_S_OK == rc ) {
                rc = mco_trans_iterate(t, iteration_proc, "\tInsert");
                if ( MCO_S_OK == rc ) {
                  rc = mco_trans_commit_phase2(t);
                } else {
                  printf ("\tIteration proc return code %d. Rollback transaction\n", rc);
                  rc = mco_trans_rollback(t);
                }
              }
            }
          }
        }
      } // End insert transactions
      if ( MCO_S_OK != rc )
      {
        sample_rc_check("Insert", rc );
      }
      else
      {
        /* Update records in database */
        for (i = 0; i < nRecords; ++i) 
        {
          key = i;
          rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
          if ( MCO_S_OK == rc ) {
            rc = Record_hkey_find(t, key, &rec);
            if ( MCO_S_OK == rc ) {
              rc = Record_key_put(&rec, key);
              if ( MCO_S_OK == rc ) {
                rc = mco_trans_commit_phase1(t);
                if ( MCO_S_OK == rc ) {
                  rc = mco_trans_iterate(t, iteration_proc, "\tUpdate");
                  if ( MCO_S_OK == rc ) {
                    rc = mco_trans_commit_phase2(t);
                  } else {
                    printf ("\tIteration proc return code %d. Rollback transaction\n", rc);
                    rc = mco_trans_rollback(t);
                  }
                }
              }
            }
          }
        } // End update transactions
        if ( MCO_S_OK != rc )
        {
          sample_rc_check("Update", rc );
        }
        else
        {

          /* Delete records from database */
          for (i = 0; i < (nRecords / 2); ++i) 
          {
            key = i;
            rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
            if ( MCO_S_OK == rc ) {
              rc = Record_hkey_find(t, key, &rec);
              if ( MCO_S_OK == rc ) {
                rc = Record_delete(&rec);
                if ( MCO_S_OK == rc ) {
                  rc = mco_trans_commit_phase1(t);
                  if ( MCO_S_OK == rc ) {
                    rc = mco_trans_iterate(t, iteration_proc, "\tDelete");
                    if ( MCO_S_OK == rc ) {
                      rc = mco_trans_commit_phase2(t);
                    } else {
                      printf ("\tIteration proc return code %d. Rollback transaction\n", rc);
                      rc = mco_trans_rollback(t);
                    }
                  }
                }
              }
            }
          } // End delete transactions
          if ( MCO_S_OK != rc )
          {
            sample_rc_check("Delete", rc );
          }
          else
          {

            /* Delete_all records of the class */
            rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
            if ( MCO_S_OK == rc ) 
            {
              rc = Record_delete_all(t);
              if ( MCO_S_OK == rc ) {
                rc = mco_trans_commit_phase1(t);
                if ( MCO_S_OK == rc ) {
                  rc = mco_trans_iterate(t, iteration_proc, "\tDelete_all");
                  if ( MCO_S_OK == rc ) {
                    rc = mco_trans_commit_phase2(t);
                  } else {
                    printf ("\tIteration proc return code %d. Rollback transaction\n", rc);
                    rc = mco_trans_rollback(t);
                  }
                }
              }
            } // End delete_all transaction
            if (MCO_S_OK != rc)
            {
              sample_rc_check("Delete_all", rc );
            }

          } // End delete transactions else block
        } // End update transactions else block
      } // End insert transactions else block

      mco_db_disconnect(db);
    }

    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

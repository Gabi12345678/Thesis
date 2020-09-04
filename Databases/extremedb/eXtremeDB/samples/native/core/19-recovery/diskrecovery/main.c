/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include "diskrecoverydb.h"

char sample_descr[] = "Sample 'diskrecovery' demonstrates automatic recovery of a disk based database.\n";
const char * db_name = "diskrecovery";
char explanation[] =  "\tA database is opened and its current contents displayed.\n"
                      "\tThen 10 records are created in separate transactions.  On the\n"
                      "\tthe 11th transaction the program exits to simulate a \n"
                      "\tsystem failure.  The next time the program executes the\n"
                      "\tpreviously inserted 10 records appear showing that the\n"
                      "\ttransaction processing makes a separate recovery\n"
                      "\tprocedure unnecessary.\n";

const unsigned int nRecords = 1000;

int main(int argc, char* argv[])
{
  MCO_RET rc = MCO_S_OK;
  mco_db_h db = 0;
  mco_trans_h t;
  mco_cursor_t c;
  Record rec;
  uint4 i, key;
  sample_memory_t dbmem;
    
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  printf(explanation);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
 
  /* Open databases */
  rc = sample_open_database( db_name, diskrecoverydb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &dbmem );
  if ( MCO_S_OK == rc ) {
    
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {
    
      /* Display db contents */
      printf("\n\n\tThe database contents are:\n");
      key = 0;  /* Set key to 0 in case first pass */
      rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {

        rc = Record_tkey_index_cursor(t, &c);
        if ( MCO_S_CURSOR_EMPTY == rc ) {
          printf("\n\tDatabase is empty");
          rc = MCO_S_OK;
        } else if ( MCO_S_OK != rc ) {
          sample_rc_check("\n\tCursor error", rc );
        } else {

          for (i = 0, rc = mco_cursor_first(t, &c); MCO_S_OK == rc; rc = mco_cursor_next(t, &c), i++)
          {
            rc = Record_from_cursor(t, &c, &rec);
            rc = Record_key_get(&rec, &key);
            printf("\n\tRecord %d: key = %d", i, key);
          }
          if (MCO_S_CURSOR_END != rc)
          {
            sample_rc_check("\n\tEnd of cursor error", rc );
          }
        }
        rc = mco_trans_commit(t);
      }

      /* Insert 10 records */
      printf("\n");
      for (i = 0; i < 10 && MCO_S_OK == rc; i++) {
        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {
          rc = Record_new(t, &rec);
          if ( MCO_S_OK == rc ) {
            key++;    /* Increment key */
            Record_key_put(&rec, key);   
            rc = mco_trans_commit(t);
            printf("\n\tInsert key = %d", key);
          }
        }
      }
      if ( MCO_S_OK == rc ) {
        printf("\n\n\t10 records inserted successfully.  \n\tStart 11th transaction and Exit without committing...\n");
        sample_pause_end("\n\nPress any key to continue . . . ");
        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {
          /* Exit brutally without stopping runtime to simulate a system crash */
          sample_os_shutdown();
          dbg_exit(0);
        } else {
          sample_rc_check("\n\tFailed to start transaction", rc );
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

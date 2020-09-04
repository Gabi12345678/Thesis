/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include "ph2db.h"

char sample_descr[] = "Sample 'ph2disk' demonstrates a two-phase commit to a disk based database.\n";
const char * db_name = "ph2disk";
char explanation[] =  "\tUse command line arguments as follows:\n"
                      "\t  Exit - write a record, commit phase 1 and exit before phase 2 commit\n"
                      "\t  Confirm - complete phase 2 commit on opening the database\n"
                      "\t  Reject - reject the partial transaction\n"
                      "\tIf no argument is specified, the database contents are displayed.\n";

const unsigned int nRecords = 1000;

int main(int argc, char* argv[])
{
  MCO_RET rc = MCO_S_OK;
  mco_db_h db = 0;
  mco_trans_h t;
  mco_cursor_t c;
  Record rec;
  uint4 i, key = 42;
  int exit_code = 2;  /* Cause open and display of db contents if no argument specified */
  sample_memory_t dbmem;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  printf(explanation);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
 
  /* If runtime argument was 'Reject' or 'Confirm' call the appropriate function before opening db;
   * if 'Exit' write a record to the db and exit after phase 1 commit; then display db contents. */
  if ( argc > 1 ) {
    if ( 'C' == argv[1][0] )
      exit_code = 0;
    else if (  'R' == argv[1][0] )
      exit_code = -1;
    else if (  'E' == argv[1][0] )
      exit_code = 1;
    else
      exit_code = 2;
  }

  if ( exit_code < 1 ) {
    if ( 0 == exit_code ) {
      printf("\n\tConfirming any partially completed transactions...");
      rc = mco_disk_confirm_transaction("ph2disk.log", REDO_LOG );
    } else {
      printf("\n\tRejecting any partially completed transactions...");
      rc = mco_disk_reject_transaction("ph2disk.log", REDO_LOG );
    }
  }

  if ( MCO_S_OK == rc ) {

    /* Open databases */
    rc = sample_open_database( db_name, ph2db_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &dbmem );
    if ( MCO_S_OK == rc ) {
    
      rc = mco_db_connect(db_name, &db);
      if ( MCO_S_OK == rc ) {
    
        if ( 1 == exit_code ) {

          /* Insert a record to database and commit phase 1, then exit */
          rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
          if ( MCO_S_OK == rc ) {
            rc = Record_new(t, &rec);
            if ( MCO_S_OK == rc ) {
              Record_key_put(&rec, key);   
              rc = mco_trans_commit_phase1(t);
              sample_rc_check("\n\tCommit Phase 1", rc );
            }
          }
          if ( MCO_S_OK == rc ) {
            printf("\n\tPhase 1 committed successfully.  Exit without committing phase 2 ...");
            sample_pause_end("\n\nPress any key to continue . . . ");
            /* Exit brutally without stopping runtime to simulate a system crash */
            sample_os_shutdown();
            dbg_exit(0);
          }
        }

        /* Exit_code != 1; Confirm or Reject was done, display db contents */
        printf("\n\n\tThe database contents are:\n");
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

        mco_db_disconnect(db);
      }

      sample_close_database(db_name, &dbmem);
    }
  } else {
    sample_rc_check("\n\tFailed to confirm/reject transaction", rc );
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

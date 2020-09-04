/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <math.h>
#include <time.h>
#include <common.h>
#include "hashdb.h"

char sample_descr[] = {
  "Sample 'hash' demonstrates operations on unique and non-unique hash indexes.\n"
};
const char * db_name = "hashdb";

#define SERIES_SIZE 5

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  sample_memory_t dbmem;
  mco_trans_h t;
  mco_cursor_t csr;
  int i;
  Record rec;
  uint4 idx, series;
  int nRecs = 7;
  int findValue = 179;
  
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, hashdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Fill database */
      printf("\n\tInsert 1000 test records ");
      for (i = 0; i < 1000 && MCO_S_OK == rc; i++) {
        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {       
          rc = Record_new(t, &rec);
          if ( MCO_S_OK == rc ) {
            /* Put unique values for index, non-unique values for series */
            idx = i;
            Record_iIdx_put(&rec, idx);
            series = (i / SERIES_SIZE) + 1;
            Record_iSeries_put(&rec, series);
            rc = mco_trans_commit(t);
          } else {
            mco_trans_rollback(t);
          }
          if ( 0 == (i % 100) )  {
            printf(".");
          }
        }
      }

      if ( MCO_S_OK == rc ) {

        /* Find a specific value in unique index */
        printf("\n\n\tFind record with index == %d :", findValue);

        /* Open a read-only transaction */
        rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {
          /* Open the cursor */
          rc = Record_I_Index_index_cursor(t, &csr);
          if ( MCO_S_OK == rc )  {
            /* Get found record - or not */
            rc = Record_I_Index_find(t, findValue, &rec);
            if ( MCO_S_OK == rc )  {
              Record_iIdx_get(&rec, &idx);
              Record_iSeries_get(&rec, &series);
              printf("\n\tIndex %d Series %d", idx, series);
            } else {
              printf("\tnot found");
            }
          }
          /*  Close the transaction */
          mco_trans_rollback(t);
        }

        /* Show all records with specified value in non-unique index */
        printf("\n\n\tSearch for records with iSeries == %d :", findValue);

        /* Open a transaction */
        rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc )  {
          /* Open the cursor */
          rc = Record_I_Series_index_cursor(t, &csr);
          if ( MCO_S_OK ==  rc )  {
            /* Search for records with specified value for iSeries */
            rc = Record_I_Series_search(t, &csr, findValue);
            if ( MCO_S_OK ==  rc )  {
              /* Move to first item */
              rc = mco_cursor_first(t, &csr);
              /* Show all records in cursor */
              for (i = 0; i < nRecs && MCO_S_OK == rc; i++)  {
                /* Get Record from cursor */
                rc = Record_from_cursor(t, &csr, &rec);
                if ( MCO_S_OK == rc )  {
                  Record_iIdx_get(&rec, &idx);
                  Record_iSeries_get(&rec, &series);
                  printf("\n\tIndex %d Series %d", idx, series);
                  /* Move to next item */
                  rc = mco_cursor_next(t, &csr);
                }
              }
              rc = MCO_S_OK;
            } else {
              printf("\tno records found.");
            }
          }
          /*  Close the transaction */
          mco_trans_rollback(t);
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

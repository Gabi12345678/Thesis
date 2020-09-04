/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include "searchdb.h"

char sample_descr[] = { "Sample 'search' demonstrates search operations with tree indexes\n" };
const char * db_name = "search";

uint4 values[] = { 1,2,2,3,3,3,4,4,5,5,5 };
uint4 search_value = 3;

int main(int argc, char* argv[])
{

  MCO_RET rc;
  mco_db_h db = 0;
  mco_trans_h t;
  sample_memory_t dbmem;
  unsigned int i;
  uint4 value;
  mco_cursor_t csr;
  anObject obj;
  
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, searchdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Fill database */
      for (i = 0; i < sizeof(values) / sizeof(values[0]) && MCO_S_OK == rc; ++i) {
        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc )  {
          anObject_new(t, &obj);
          anObject_value_put(&obj, values[i]);
          rc = mco_trans_commit(t);
        }
        if ( MCO_S_OK != rc ) {
          printf("\tUnable insert object #%d: %s\n", i, mco_ret_string(rc, 0));
        }
      }

      if ( MCO_S_OK == rc ) {

        /* Show all objects */
        rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc )  {
          rc = anObject_Idx_index_cursor(t, &csr);    
          if ( MCO_S_OK == rc ) {
            printf("All objects             : ");
            for (rc = mco_cursor_first(t, &csr); rc == MCO_S_OK; rc = mco_cursor_next(t, &csr)) {
              anObject_from_cursor(t, &csr, &obj);
              anObject_value_get(&obj, &value);
              printf("(%d) ", value);
            }
            rc = MCO_S_CURSOR_END == rc ? MCO_S_OK : rc;
          }    
          mco_trans_rollback(t);
        }

        if ( MCO_S_OK == rc ) {

          /* Show objects with value > search_value */
          rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
          if ( MCO_S_OK == rc )  {
            rc = anObject_Idx_index_cursor(t, &csr);    
            if ( MCO_S_OK == rc ) {
              printf("\nObjects with value  > %d : ", search_value);
              for (rc = anObject_Idx_search(t, &csr, MCO_GT, search_value); MCO_S_OK == rc; rc = mco_cursor_next(t, &csr)) {
                anObject_from_cursor(t, &csr, &obj);
                anObject_value_get(&obj, &value);
                printf("(%d) ", value);
              }
              rc = MCO_S_CURSOR_END == rc ? MCO_S_OK : rc;
           }    
            mco_trans_rollback(t);
          }
        }

        if ( MCO_S_OK == rc ) {

          /* Objects with value >= search_value */
          rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
          if ( MCO_S_OK == rc )  {
            rc = anObject_Idx_index_cursor(t, &csr);    
            if ( MCO_S_OK == rc ) {
              printf("\nObjects with value >= %d : ", search_value);
              for (rc = anObject_Idx_search(t, &csr, MCO_GE, search_value); MCO_S_OK == rc; rc = mco_cursor_next(t, &csr)) {
                anObject_from_cursor(t, &csr, &obj);
                anObject_value_get(&obj, &value);
                printf("(%d) ", value);
              }
              rc = MCO_S_CURSOR_END == rc ? MCO_S_OK : rc;
            }    
            mco_trans_rollback(t);
          }
        }

        if ( MCO_S_OK == rc ) {

          /* Objects with value == search_value */
          rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
          if ( MCO_S_OK == rc )  {
            rc = anObject_Idx_index_cursor(t, &csr);    
            if ( MCO_S_OK == rc ) {
              printf("\nObjects with value == %d : ", search_value);
              for (rc = anObject_Idx_search(t, &csr, MCO_EQ, search_value); MCO_S_OK == rc; rc = mco_cursor_next(t, &csr)) {
                int result;

                /* Use _compare() to stop before the object with non-equal value */
                anObject_Idx_compare(t, &csr, search_value, &result);
                if (result) break;
                
                anObject_from_cursor(t, &csr, &obj);
                anObject_value_get(&obj, &value);
                printf("(%d) ", value);
              }
              rc = MCO_S_CURSOR_END == rc ? MCO_S_OK : rc;
            }    
            mco_trans_rollback(t);
          }
        }

        if ( MCO_S_OK == rc ) {

          /* Objects with value <= search_value */
          rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
          if ( MCO_S_OK == rc )  {
            rc = anObject_Idx_index_cursor(t, &csr);    
            if ( MCO_S_OK == rc ) {
              printf("\nObjects with value <= %d : ", search_value);
              for (rc = anObject_Idx_search(t, &csr, MCO_LE, search_value); MCO_S_OK == rc; rc = mco_cursor_prev(t, &csr)) {
                anObject_from_cursor(t, &csr, &obj);
                anObject_value_get(&obj, &value);
                printf("(%d) ", value);
              }
              rc = MCO_S_CURSOR_END == rc ? MCO_S_OK : rc;
            }    
            mco_trans_rollback(t);
          }
        }

        if ( MCO_S_OK == rc ) {

          /* Objects with value < search_value */
          rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
          if ( MCO_S_OK == rc )  {
            rc = anObject_Idx_index_cursor(t, &csr);    
            if ( MCO_S_OK == rc ) {
              printf("\nObjects with value  < %d : ", search_value);
              for (rc = anObject_Idx_search(t, &csr, MCO_LT, search_value); MCO_S_OK == rc; rc = mco_cursor_prev(t, &csr)) {
                anObject_from_cursor(t, &csr, &obj);
                anObject_value_get(&obj, &value);
                printf("(%d) ", value);
              }
              rc = MCO_S_CURSOR_END == rc ? MCO_S_OK : rc;
            }    
            mco_trans_rollback(t);
          }
          printf("\n");
        }

        /* Disconnect and close database, stop runtime */
        mco_db_disconnect(db);
      }

      sample_close_database(db_name, &dbmem);
    }
  }

  mco_runtime_stop();

  sample_pause_end("\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include "udfdb.h"

char sample_descr[] = "Sample 'userdef' demonstrates user-defined compare functions for a tree index.\n";
const char * db_name = "userdef";
char * fruits[] = {
  "banana", "PEAR", "plum", "Peach", "apricot", "Kiwi", "QUINCE", "pineapple", "Lemon", "orange", "apple", 
  "pawpaw", "Fig", "mango", "MANDARIN", "Persimmon", "Grapefruit", 0
};

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  mco_trans_h t;
  int i;
  Record rec;
  mco_cursor_t c;
  uint2 len;
  uint4 value;
  char buf[16];
  char search[16];
  char msg[128];
  sample_memory_t dbmem;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open database */
  rc = sample_open_database( db_name, udfdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    /* Register custom compare & hash functions on database name */
    mco_db_register_udf(db_name, udfdb_get_udfs());

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Insert Records */
      for (i = 0; fruits[i] && MCO_S_OK == rc; ++i)
      {
        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {
          rc = Record_new(t, &rec);
          if ( MCO_S_OK == rc ) {
              Record_value_put(&rec, i);
              Record_name_put(&rec, fruits[i], (uint2)strlen(fruits[i]));
          }
          rc = mco_trans_commit(t);
        }
        if ( MCO_S_OK == rc ) {
          printf("\n\tInserted: %-15s, value = %d", fruits[i], i);
        } else {
          sprintf(msg, "\n\tUnable to insert %-15s", fruits[i] );
          sample_rc_check(msg, rc );
        }
      }

      if ( MCO_S_OK == rc ) {
        printf("\n\n\tStandard (case-sensitive) sequential :");
        rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {
          /* Using ordinary tree index */
          rc = Record_tstd_index_cursor(t, &c);
          if ( MCO_S_OK == rc ) {
            for (rc = mco_cursor_first(t, &c); MCO_S_OK == rc; rc = mco_cursor_next(t, &c))  {
              Record_from_cursor(t, &c, &rec);
              Record_name_get(&rec, buf, 11, &len);
              Record_value_get(&rec, &value);
              printf("\n\t%-15s, value = %d", buf, value);
            }
            rc = mco_trans_commit(t);
          }
        }
      }

      if ( MCO_S_OK == rc ) {
        printf("\n\n\tUser-defined (case-insensitive) sequential :");
        rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
        if (rc == MCO_S_OK) {
          /* Using userdef tree index */
          rc = Record_tudf_index_cursor(t, &c);
          if (rc == MCO_S_OK) {
            for (rc = mco_cursor_first(t, &c); rc == MCO_S_OK; rc = mco_cursor_next(t, &c))  {
              Record_from_cursor(t, &c, &rec);
              Record_name_get(&rec, buf, 11, &len);
              Record_value_get(&rec, &value);
              printf("\n\t%-15s, value = %d", buf, value);
            }
            rc = mco_trans_commit(t);
          }
        }
      }

      if ( MCO_S_OK == rc ) {
        /* Find in indexes */
        strcpy(search, "kiwi"); 
        rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {
          printf("\n\n\tFind by name \"%s\" :", search);
          rc = Record_tudf_find(t, search, (uint2)strlen(search), &rec);
          if ( MCO_S_OK == rc ) {
            Record_name_get(&rec, buf, 11, &len);
            Record_value_get(&rec, &value);
            printf("\n\tUser-defined (case-insensitive): Found '%s', value = %d", buf, value);
          } else {
            printf("\n\tUser-defined (case-insensitive) - '%s' not found", search );
          }

          rc = Record_tstd_find(t, search, (uint2)strlen(search), &rec);
          if ( MCO_S_OK == rc ) {
            Record_name_get(&rec, buf, 11, &len);
            Record_value_get(&rec, &value);
            printf("\n\tStandard (case-sensitive): Found '%s', value = %d", buf, value);
          } else {
            printf("\n\tStandard (case-sensitive) - '%s' not found", search );
          }
        }
        rc = mco_trans_commit(t);
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

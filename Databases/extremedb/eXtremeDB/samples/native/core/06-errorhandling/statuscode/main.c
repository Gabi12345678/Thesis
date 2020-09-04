/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <math.h>
#include <time.h>
#include <common.h>
#include "statuscodedb.h"

char sample_descr[] = {
  "Sample 'statuscode' demonstrates handling of eXtremeDB status codes.\n"
};
const char * db_name = "statuscodedb";

#define N_VALUES 2

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  sample_memory_t dbmem;
  mco_trans_h t;
  mco_cursor_t csr;
  int i;
  Record rec;
  int findValue[N_VALUES] = { 179, 1079 };
  uint4 idx;
  char name[64];
  uint2 size;
   
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, statuscodedb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, 0, 1, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {
      /* Fill database with test records */
      printf("\n\tInsert 1000 test records ");
      for (i = 0; i < 1000 && MCO_S_OK == rc; i++) {
        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {
          rc = Record_new(t, &rec);
          idx = i;
          sprintf(name,"Record %d", i);
          Record_idx_put(&rec, idx);
          Record_name_put(&rec, name, (uint2)strlen(name));
          rc = mco_trans_commit(t);
          if ( 0 == (i % 100) )  {
            printf(".");
          }
        }
      }

      if ( MCO_S_OK == rc ) {

        for ( i=0; i < N_VALUES && MCO_S_OK == rc; i++ ) {

          /* Lookup two values in unique index: one that exists and one that doesn't */
          printf("\n\n\tFind record with index == %d :", findValue[i]);
          rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
          if ( MCO_S_OK == rc ) {
            rc = Record_I_Index_index_cursor(t, &csr);
            if ( MCO_S_OK == rc )  {
              rc = Record_I_Index_find(t, findValue[i], &rec);
              /* Handle the status code returned by the find function... */
              switch ( rc )
              {
                case MCO_S_OK:
                  Record_idx_get(&rec, &idx);
                  Record_name_get(&rec, name, (uint2)strlen(name), &size);
                  printf("\n\t%d: %s", idx, name);
                  break;

                case MCO_S_NOTFOUND:
                  printf("\n\tRecord not found for index %d", findValue[i]);
                  break;

                default:
                  printf("\n\tStatus/Error code is %d", rc);
                  break;
              }
            }
            /*  Close the transaction */
            rc = mco_trans_rollback(t);
          }
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

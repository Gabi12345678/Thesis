/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <math.h>
#include <time.h>
#include <common.h>
#include "treedb.h"

char sample_descr[] = {
  "Sample 'basic' demonstrates basic eXtremeDB cursor operations.\n"
};
const char * db_name = "treedb";

int main(int argc, char* argv[])
{
  MCO_RET rc = MCO_S_OK;
  mco_db_h db = 0;
  sample_memory_t dbmem;
  mco_trans_h t;
  mco_cursor_t csr;
  int i;
  Record rec;
  uint4 key;
  char name[64];
  uint2 size;
  int nRecs = 1001;
  char str[64];
  
  sample_os_initialize(DEFAULT);

  sample_header( sample_descr );

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler( &sample_errhandler );
  mco_runtime_start();
    
  /* Open and connect to database */
  rc = sample_open_database( db_name, treedb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  sample_rc_check( "\tOpen database", rc );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect( db_name, &db );
    sample_rc_check( "\tConnect database", rc );
    if ( MCO_S_OK == rc ) {

      /* Fill database */
      printf( "\n\tInsert 1000 test records " );
      for ( i = 0; i < 1000 && MCO_S_OK == rc; i++ ) {
        rc = mco_trans_start( db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t );
        if ( MCO_S_OK == rc ) {       
          rc = Record_new( t, &rec );
          if ( MCO_S_OK == rc ) {
            Record_key_put( &rec, i );
            sprintf( str, "Record %04d", i );
            Record_name_put( &rec, str, (uint2)strlen(str) );
            rc = mco_trans_commit( t );
          } else {
            mco_trans_rollback( t );
          }
          if ( 0 == (i % 100) )  {
            printf( "." );
          }
        }
      }

      if ( MCO_S_OK == rc ) {
  
        /* Navigate forward from first Record */
        printf( "\n\n\tFirst %d records:", nRecs );

        /* Open a read-only transaction */
        rc = mco_trans_start( db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t );
        if ( MCO_S_OK == rc ) {
          /* Open the cursor */
          rc = Record_autoid_index_cursor( t, &csr );
          if ( MCO_S_OK == rc )  {
            /* Get first record */
            rc = mco_cursor_first( t, &csr );

            /* Show first nRecs */
            for ( i = 0; i < nRecs && MCO_S_OK == rc; i++ ) {
              /* Get Record from cursor */
              rc = Record_from_cursor( t, &csr, &rec );
              if ( MCO_S_OK == rc )  {
                Record_key_get( &rec, &key );
                Record_name_get( &rec, name, (uint2)sizeof(name), &size );
		//                printf( "\n\t%d: %s", key, name );
                /* Move to next item */
                rc = mco_cursor_next( t, &csr );
              }
            }
	    printf ("Cursor done, rc=%d\n", rc);
            /* Close cursor */
            mco_cursor_close( t, &csr );
          }
          /*  Close the transaction */
          mco_trans_rollback( t );
        }

        /* Navigate backwards from last Record */
        printf( "\n\n\tLast %d records:", nRecs );

        /* Open a transaction */
        rc = mco_trans_start( db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t );
        if ( MCO_S_OK == rc )  {
          /* Open the cursor */
          rc = Record_autoid_index_cursor( t, &csr );
          if ( MCO_S_OK ==  rc )  {
            /* Move to last item */
            rc = mco_cursor_last( t, &csr );
            /* Show last nRecs */
            for ( i = 0; i < nRecs && MCO_S_OK == rc; i++ )  {
              /* Get Record from cursor */
              rc = Record_from_cursor( t, &csr, &rec );
              if ( MCO_S_OK == rc )  {
                Record_key_get( &rec, &key );
                Record_name_get( &rec, name, (uint2)sizeof(name), &size );
		//  printf( "\n\t%d: %s", key, name );
              }
              /* Move to prev item */
              rc = mco_cursor_prev( t, &csr );
            }

            /* Close cursor */
            mco_cursor_close( t, &csr );
          }
          /*  Close the transaction */
          mco_trans_rollback( t );
        }
      }

      rc = mco_db_disconnect( db );
      sample_rc_check( "\tDisconnect database", rc );
    }

    rc = sample_close_database( db_name, &dbmem );
    sample_rc_check( "\tClose database", rc );
  }

  mco_runtime_stop();

  sample_pause_end( "\n\nPress any key to continue . . . " );

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

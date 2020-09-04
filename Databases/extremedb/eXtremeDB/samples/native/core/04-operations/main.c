/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <simpledb.h>

const char * db_name = "operations";

int main(int argc, char* argv[]) {
    
  MCO_RET         rc;
  sample_memory_t dbmem;

  sample_os_initialize(DEFAULT);

  /* Start eXtremeDB runtime */
  mco_runtime_start();

  sample_header("Sample 'operations' performs basic insert, read, and update operations.\n" );
  rc = sample_open_database( db_name, simpledb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 1, &dbmem );
                               
  sample_rc_check("\tOpen", rc );

  if ( MCO_S_OK == rc ) {

    mco_db_h connection; /* Connection handle */

    /* The database was opened successfully */
    /* Connect it by name */
    
    rc = mco_db_connect( db_name, &connection );
    sample_rc_check("\tConnect", rc );
    
    if ( MCO_S_OK == rc ) {
        
      /* The database is connected successfully */
      mco_trans_h t;
     
      rc = mco_trans_start(connection, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      sample_rc_check("\tOpen Transaction", rc );
      if ( MCO_S_OK == rc ) {      
        A        a;   /* Object handle */
        A_fixed _a;   /* Fixed size part of class A */
        B        b;   /* Struct handle */
        B_fixed _b;   /* Fixed size part of struct B */
        uint1  ui1;   /* Value place holders */
        uint2  ui2;
        uint4  ui4;
        int1   i1;
        int2   i2;
        int4   i4;
        mco_int8   i8;
        float  f = (float)1.1;
        double d = 2.2;
        char   buf[16]; /* String buffer */
        mco_cursor_t csr; /* Cursor to navigate database contents */
         
        /* Allocate an object */
        A_new ( t, &a );

        /* Put values to scalar fields */
        A_ui1_put ( &a, 1 );
        A_ui2_put ( &a, 2 );
        A_ui4_put ( &a, 3 );
        A_d_put   ( &a, d );
         
        /* Put value to string field using db_name as a value */
        A_s_put   ( &a, db_name, (uint2)strlen( db_name ) );

        /* Put int8 value to numeric field */
        A_num_from_chars( &i8, "1234567");
        A_num_put ( &a, i8 );

        /* Put char array value to decimal field */
        A_dec_to_chars( 987654321, buf, sizeof(buf));
        A_dec_put_chars( &a, buf );

        /* Get struct handle to write it */
        A_b_write_handle ( &a, &b );
         
        /* Put values to the struct fields */
        B_i1_put ( &b, 4 );
        B_i2_put ( &b, 5 );
        B_i4_put ( &b, 6 );
        B_f_put  ( &b, f );
        B_c10_put( &b, db_name, (uint2)strlen( db_name ) );

        rc = mco_trans_commit( t );
        sample_rc_check("\tCommit Transaction", rc );

        /* Open a READ_ONLY transaction, read object A and display its contents */
        rc = mco_trans_start(connection, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) { 
          rc = A_list_cursor(t, &csr);
          if ( MCO_S_OK == rc ) { 
            mco_cursor_first(t, &csr);
            A_from_cursor(t, &csr, &a);
            printf("\n\tContents of first record A: \n");
            /* Get values from the object fields */
            A_ui1_get ( &a, &ui1 );
            A_ui2_get ( &a, &ui2 );
            A_ui4_get ( &a, &ui4 );
            A_d_get   ( &a, &d );
            printf("\t\tui1=%d, ui2=%d, ui4=%ld, d=%f\n", ui1, ui2, ui4, d );
            /* Get values from numeric/decimal fields */
            A_num_get( &a, &i8);
            A_num_get_chars( &a, buf, sizeof(buf));
            printf("\t\tnum=%lld, chars(%s)\n", i8, buf );
            A_dec_get( &a, &i8);
            A_dec_get_chars( &a, buf, sizeof(buf));
            printf("\t\tdec=%lld, chars(%s)\n", i8, buf );
            /* Get string value into the buf and the value length into ui2 */
            /* Note: the _get() will null-terminate the string only if the buffer is long enough */
            A_s_get( &a, buf, sizeof(buf), &ui2);
            printf("\tstring s=(%s), (length = %d)\n", buf, ui2);

            /* Get struct read-handle */
            A_b_read_handle  ( &a, &b );

            /* Get values of the structs fields */
            B_i1_get ( &b, &i1 );
            B_i2_get ( &b, &i2 );
            B_i4_get ( &b, &i4 );
            B_f_get  ( &b, &f );
            B_c10_get( &b, buf, sizeof(buf) );
            printf("\tStruct b: b.i1=%d, b.i2=%d, b.i4=%ld, b.f=%f, b.c10=(%s)\n", i1, i2, i4, f, buf );

            /* Get field values from A_fixed */
            printf("\n\tUsing A_fixed :\n" );
            A_fixed_get(&a, &_a);
            printf("\t\t_a.ui1=%d, _a.ui2=%d, _a.ui4=%ld, _a.d=%f\n", _a.ui1, _a.ui2, _a.ui4, _a.d );
            
            /* Get field values from B_fixed */
            printf("\n\tUsing B_fixed :\n" );
            B_fixed_get(&b, &_b);
            /* Copy and null terminate the character string in _b.c10 */
            strncpy(buf, _b.c10, 10);
            printf("\t\t_b.i1=%d, _b.i2=%d, _b.i4=%ld, _b.f=%f, _b.c10=(%s)\n", _b.i1, _b.i2, _b.i4, _b.f, buf );

          }
        }
        rc = mco_trans_commit( t );
      }
       
      /* Don't forget to disconnect when done */
      rc = mco_db_disconnect( connection );
      sample_rc_check("\tDisconnect", rc );
    }

    /* Close the database */
    sample_close_database( db_name, &dbmem );
    sample_rc_check("\tClose", rc );
  }
  
  /* Stop eXtremeDB runtime */
  mco_runtime_stop();
     
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();

  return ( MCO_S_OK == rc ? 0 : 1 );
}

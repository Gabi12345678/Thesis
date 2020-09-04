/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <overflowdb.h>

char sample_descr[] = {
  "Sample 'overflow_persistent_cache' demonstrates the use of information from\nstructure mco_disk_cache_info_t to prevent the MCO_E_DISK_PAGE_POOL_EXHAUSTED error.\n"
};
const char *db_name = "overflowdb";

int main(int argc, char** argv) {
  MCO_RET             rc, rc2 = MCO_S_OK;
  sample_memory_t     dbmem;
  mco_db_h            connection;

  sample_os_initialize(DEFAULT);

  /* Start eXtremeDB runtime */
  mco_error_set_handler( &sample_errhandler );
  mco_runtime_start();

  sample_header( sample_descr );

  sample_show_runtime_info( "Database runtime has the following characteristics:\n" );
  printf( "\nBegin sample execution.\n" );

  /* Open database instance and connect it */
  rc = sample_open_database( db_name, overflowdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, MAX_DB_CONNECTIONS, &dbmem );
  sample_rc_check( "\tOpen database", rc );
  if ( MCO_S_OK == rc )
  {
    /* Connect to database */
    rc = mco_db_connect( db_name, &connection );
    sample_rc_check( "\tConnect to database", rc );
    if ( MCO_S_OK == rc )
    {
        const mco_disk_cache_info_t *cache_info;
        mco_counter_t pinned_pages_threshold;
        uint4 key_value;
        int commits_count;
        mco_trans_h t;

        /* Clean the database content before filling */
        rc2 = mco_db_clean(connection);
        sample_rc_check("\tClean database", rc2);
        if (MCO_S_OK != rc2)
        {
            goto on_error;
        }

        cache_info = mco_disk_get_cache_info(connection);

        /*
        This sample demonstrates how to predict and prevent the MCO_E_DISK_PAGE_POOL_EXHAUSTED error.
        Usually the pinned pages threshold should be about 2/3 or 3/5 of the total allocated pages count.
        As an uncommitted transaction is placed in the main DB memory device, consequently the transaction 
        size is limited by the size of the main memory device which can cause the MCO_E_NOMEM error. Another
        sample 'samples/native/core/21-overflow/transient' shows how to predict and avoid this error.
        So, for the purpose of demonstration, the pinned pages threshold is set to 10 pages for an x32 application
        not to overflow the main DB memory.
        */
#ifdef MCO_PLATFORM_X64
        pinned_pages_threshold = cache_info->allocated_pages * 2 / 3;
#else
        pinned_pages_threshold = 10;
#endif

        printf( "\n\tFill the database ..." );
        key_value = 0;
        commits_count = 0;

        /* Start first transaction */
        rc2 = mco_trans_start(connection, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if (MCO_S_OK != rc2)
        {
            sample_rc_check("\tStart transaction", rc2);
            goto on_error;
        }

        /* Main loop - simulates normal database activity */
        while ( 1 )
        {
            Record      rec;
            int         i;

            for ( i = 0; i < 100; i++ )
            {
                rc2 = Record_new( t, &rec );
                if ( MCO_S_OK != rc2 )
                {
                    sample_rc_check( "\tNew record", rc2 );
                    goto on_error;
                }

                rc2 = Record_key_put( &rec, key_value );
                if ( MCO_S_OK != rc2 )
                {
                    sample_rc_check( "\tUpdate record", rc2 );
                    goto on_error;
                }
                key_value++;
            }

            /* Commit collected data to get rid of 'page pool exhausted' error caused by lack of unpinned disk cache pages */
            if (cache_info->pinned_pages > pinned_pages_threshold)
            {
                printf("\n\tPinned disk cache pages threshold has been reached.\n");
                rc2 = mco_trans_commit(t);
                if (MCO_S_OK != rc2)
                {
                    sample_rc_check("\tFinish write transaction", rc2);
                    goto on_error;
                }

                /* Do 8 iterations for demo purposes */
                if (8 <= commits_count)
                    break;

                printf("\tRestart the transaction at record %d to get unpinned pages.\n", key_value);
                rc2 = mco_trans_start(connection, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
                if (MCO_S_OK != rc2)
                {
                    sample_rc_check("\tStart the transaction", rc2);
                    goto on_error;
                }
                commits_count++;
            }
        }

        printf("\n\tDone. %d records were added to the database.\n", key_value);

on_error:

        rc = mco_db_disconnect( connection );
        sample_rc_check( "\tDisconnect database", rc );
    }

    rc = sample_close_database( db_name, &dbmem );
    sample_rc_check( "\tClose database", rc );
  }

  mco_runtime_stop();

  sample_pause_end( "\n\nPress any key to continue . . . " );

  sample_os_shutdown();
  return ( (MCO_S_OK == rc && MCO_S_OK == rc2) ? 0 : 1 );
}

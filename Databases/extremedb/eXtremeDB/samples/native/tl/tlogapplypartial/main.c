/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <tlogapplydb.h>
#include "mcolog.h"

#define NLOG_PARTS 5

char sample_descr[] = {
  "Sample 'tlogapplydb' demonstrates the use of the MCO_TRANSLOG_APPLY option.\n"
};
const char *db_name = "tlogapplydb";
const char *tl_log  = "tlogapply_tl";

/* Append 100 objects to database */
void append_database( mco_db_h con )
{
  Record rec;
  mco_trans_h t;
  uint4 i;
  static uint4 key = 0;

  CHECK(mco_trans_start( con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t ));

  for ( i = 0; i < 100; i++ )
  {
    CHECK(Record_new( t, &rec ));
    CHECK(Record_key_put( &rec, key++ ));
  }

  CHECK(mco_trans_commit( t ));
}

uint4 check_database( mco_db_h con )
{
  Record rec;
  mco_trans_h t;
  mco_cursor_t c;
  uint4 key = 0;

  CHECK(mco_trans_start( con, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t ));

  CHECK(Record_tkey_index_cursor(t, &c));
  CHECK(mco_cursor_last(t, &c));
  CHECK(Record_from_cursor(t, &c, &rec));
  CHECK(Record_key_get(&rec, &key));

  CHECK(mco_trans_commit( t ));
  return key;
}

int main(int argc, char** argv)
{
  int i;
  sample_memory_t     dbmem;
  mco_db_h            connection;

  sample_os_initialize(DEFAULT);

  /* Start eXtremeDB runtime */
  mco_error_set_handler( &sample_errhandler );
  mco_runtime_start();
  mco_translog_init();

  sample_header( sample_descr );

  sample_show_runtime_info( "The database runtime has the following characteristics:\n" );
  printf( "\nBegin sample execution.\n" );

  /*
  Log parts Create stage
  */

  /* Open database instance and connect it */
  CHECK(sample_open_database( db_name, tlogapplydb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, MAX_DB_CONNECTIONS, &dbmem ));
  printf("\tOpen database\n");

  /* Connect to database */
  CHECK(mco_db_connect( db_name, &connection ));
  printf( "\tConnect to database\n" );

  for (i = 0; i < NLOG_PARTS; i++)
  {
    char buff[64];
    mco_TL_start_data_t log_parms;

    sprintf(buff, "%s_%d.log", tl_log, i);

    log_parms.flags = (i == 0 ? 0 : MCO_TRANSLOG_RESTART);
    log_parms.disk_page_size = PSTORAGE_PAGE_SIZE;

    CHECK(mco_translog_start( connection, buff, &log_parms ));
    printf( "\tStart logging. Part %d\n", i + 1 );

    /* This function simulates normal database activity */
    append_database( connection );
    printf( "\tAppend database. Part %d\n", i + 1 );
  }

  /* Stop transaction logging */
  CHECK(mco_translog_stop( connection ));
  printf( "\tStop logging\n" );

  CHECK(mco_db_disconnect( connection ));
  printf( "\tDisconnect database\n" );

  CHECK(sample_close_database( db_name, &dbmem ));
  printf( "\tClose database\n" );

  /*
  Log parts Apply stage
  */

  /* Open database instance again and connect it */
  CHECK(sample_open_database( db_name, tlogapplydb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, MAX_DB_CONNECTIONS, &dbmem ));
  printf("\tOpen database\n");

  /* Connect to database */
  CHECK(mco_db_connect( db_name, &connection ));
  printf( "\tConnect to database\n" );

  for (i = 0; i < NLOG_PARTS; i++)
  {
    char buff[64];
    mco_TL_log_info_t info;

    sprintf(buff, "%s_%d.log", tl_log, i);

    CHECK(mco_translog_query_info( connection, buff, 0, 0, &info ));
    printf( "\tQuery info about log part %d\n", i + 1 );

    if (info.transaction_apply_compat != MCO_YES)
    {
      printf( "Log part %d in not compatible with this database!\n", i + 1 );
      dbg_exit(1);
    }

    CHECK(mco_translog_apply( connection, buff, MCO_TRANSLOG_ALL_LABELS ));
    printf( "\tApply database part %d\n", i + 1 );
  }

  if (check_database( connection ) == (100 * NLOG_PARTS))
  {
    printf( "Wrong count of objects in database!\n");
    dbg_exit(1);
  } else
    printf( "\tCheck database content\n" );

  CHECK(mco_db_disconnect( connection ));
  printf( "\tDisconnect database\n" );

  CHECK(sample_close_database( db_name, &dbmem ));
  printf( "\tClose database\n" );

  CHECK(mco_runtime_stop());

  sample_pause_end( "\n\nPress any key to continue . . . " );

  sample_os_shutdown();

  return 0;
}

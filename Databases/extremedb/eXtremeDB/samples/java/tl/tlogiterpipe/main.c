/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <tlogdb.h>
#include "mcolog.h"

#define BUFFER_SEGMENT_SIZE     2*1024*1024

char sample_descr[] = {
  "Sample 'TLogIterPipe_Native' demonstrates the use of transaction logging iteration functionality.\n"
  "This sample read transactions produced by main database via pipe.\n"
  "The pipe works with blocking (with no extension files).\n"
};
const char *db_name = "tlogdb";

/* this function called by mco_translog_iterate as callback to process transaction data passed through a pipe */
MCO_RET iteration_proc (mco_trans_h trans, MCO_Hf* obj, int cid, int obj_state, void* user_ctx)
{
  int *i = (int *)user_ctx;
  (*i)++;

  if (cid == RecordObj_code)
  {
    uint4 key;

    /* check the kind of object being processed */
    if (obj_state == MCO_TRANS_OBJ_ALL_DELETED)
    {
        printf("Delete all objects 'RecordObj'\n");
    }
    else if (obj_state == MCO_TRANS_OBJ_DELETED)
    {
        CHECK(RecordObj_key_get ((RecordObj *)obj, &key));
        printf("Delete object 'RecordObj' with key %d\n", key);
    }
    else if (obj_state == MCO_TRANS_OBJ_CREATED)
    {
        CHECK(RecordObj_key_get ((RecordObj *)obj, &key));
        printf("Create object 'RecordObj' with key %d\n", key);
    }
    else
    {
        CHECK(RecordObj_key_get ((RecordObj *)obj, &key));
        printf("Update object 'RecordObj' with key %d\n", key);
    }
  }
  return MCO_S_OK;
}

int main(int argc, char** argv)
{
  MCO_RET             rc;
  mco_db_h            db;
  int                 count = 0;

  sample_os_initialize(DEFAULT);

  /* start eXtremeDB runtime */
  mco_error_set_handler( &sample_errhandler );
  CHECK(mco_runtime_start());
  mco_translog_init();

  sample_header( sample_descr );

  sample_show_runtime_info( "The database runtime has the following characteristics:\n" );

  /* connect to main database for mco_translog_iterate internal purposes */
  rc = mco_db_connect (db_name, &db);
  if (rc == MCO_E_NOINSTANCE)
  {
    printf ("Database does not exist. Please run main database application first\n");
    dbg_exit(1);
  }
  CHECK(rc);

  printf( "\nBegin sample execution.\n" );

  /* main iteration call */
restart_reading:
  rc = mco_translog_iterate (0, db,  iteration_proc, (void *)&count,
                                tlogdb_get_dictionary (),
                                0, BUFFER_SEGMENT_SIZE);

  /* wait for main thread until it has started transaction logging (create pipe) */
  if (rc == MCO_E_TL_LOG_NOT_OPENED)
  {
    printf ("Waiting for log started\n");
    sample_sleep (1000);
    goto restart_reading;
  }

  CHECK(mco_db_disconnect (db));  
  CHECK(mco_runtime_stop());

  printf ("\n%d operations there transferred via TL pipe\n", count);

  sample_pause_end( "\n\nPress any key to continue . . . " );

  sample_os_shutdown();

  return 0;
}

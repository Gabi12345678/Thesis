/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <AsyncEventDb.h>
#include "handlers.c"

#define NTHREADS 5 /* number of handler threads (for new, update,checkpoint, delete and delete all events) */

const char * db_name             = "AsyncEventDb";

/* Starts handler threads. Implemented in handlers.c */
void start_handlers(const char *db_name, sample_task_t *tasks);

/* handler thread descriptors */
static sample_task_t tasks[NTHREADS];

int main(int argc, char* argv[])
{
    MCO_RET            rc;
    mco_db_h           db  = 0;

    sample_os_initialize(DEFAULT);

    /* start eXtremeDB runtime */
    mco_runtime_start();

    sample_header("Sample 'Async_Native' opens a database using shared memory and receives events.\n" );

    printf ("Opening database device\n");
    /* open a database on the device with given params */
    // rc = mco_db_open_dev(db_name, AsyncEventDb_get_dictionary(), &dev, 1, &db_params );
    // if ( MCO_S_OK == rc ) {

    printf ("Database device opened, connecting\n");
    rc = mco_db_connect(db_name, &db);

    /* the database was opened successfully */
    sample_rc_check("\tConnect database", rc );

    if ( MCO_S_OK == rc )
    {
        /* create threads and start waiting for the database events */
        printf ("\nStarting event handlers");
        start_handlers(db_name, tasks);
        sample_pause ("\nPress any key to close database");

        /* close the database */
        mco_db_disconnect(db);
        sample_rc_check("\tClose database", rc );
    }
    if (MCO_E_NOINSTANCE == rc)
    {
        printf ("\nRun Java application AsyncNative first");
    }

    /* stop eXtremeDB runtime */
    mco_runtime_stop();
    sample_os_shutdown();

    return ( MCO_S_OK == rc ? 0 : 1 );
}

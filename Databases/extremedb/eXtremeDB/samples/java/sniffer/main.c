/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <snifferdb.h>

#ifdef MEMORY_PAGE_SIZE
#undef MEMORY_PAGE_SIZE
#endif

#define  MEMORY_PAGE_SIZE      128

#ifdef  DATABASE_SIZE
#undef DATABASE_SIZE
#endif

#define  DATABASE_SIZE        (1024 * 1024)

#define SNIFFER_INTERRVAL 100

const char * db_name             = "snifferdb";
char explanation[] =  "\n\tStart createdb_sniffer to create a database. Then start\n"
                        "\tan instance of ConnectCtx program. \n"
                        "\tThe second process will cause a segmentation fault.  \n"
                        "\tThe sniffer loop in the first process will detect and report \n"
                        "\tthe 'dead' connection in the second process.\n";

volatile int repair = 0;

sample_task_t sniffer_task;

MCO_RET sniffer_callback(mco_db_h db, void* context, mco_trans_counter_t trans_no)
{
    SAMPLE_OS_TASK_ID pid = *(SAMPLE_OS_TASK_ID *)context;

    if ( sample_os_task_id_check( pid ) == 0 ) {
        return MCO_S_OK;
    }

    printf("Process %d has crashed\n", pid);
    repair++;
    return MCO_S_DEAD_CONNECTION;
}

void sniffer_loop( sample_task_t * descriptor ) 
{
  mco_db_h db;
  SAMPLE_OS_TASK_ID pid = sample_os_task_id_get();
  /* connect using mco_db_connect_ctx() and pass &pid as parameter */
  MCO_RET rc = mco_db_connect_ctx(db_name, &pid, &db);
  
  if ( MCO_S_OK == rc ) {
    /* descriptor->stopped flag is set to 1 by sample_stop_task() in main thread */
    while ( MCO_S_OK == rc && descriptor->stopped == 0 ) {
      rc = mco_db_sniffer(db, sniffer_callback, MCO_SNIFFER_INSPECT_ACTIVE_CONNECTIONS);
      sample_sleep(SNIFFER_INTERRVAL);
    }
    mco_db_disconnect(db);
  } else {
    /* unable to connect to the database */
    sample_rc_check("\tDatabase Connect failed. Stop sniffer thread", rc );
    dbg_exit(1);
  }
}

int main(int argc, char* argv[])
{
  MCO_RET            rc;
  mco_device_t       dev;
  mco_trans_h        t;
  mco_db_params_t    db_params;
  mco_cursor_t       c;
  mco_db_h           db = 0;
  mco_runtime_info_t info;
  long               n = 0;

  sample_os_initialize(DEFAULT);

  /* start eXtremeDB runtime */
  mco_runtime_start();

  /* get runtime info */
  mco_get_runtime_info(&info);

  sample_header("Sample 'createdb_sniffer' creates a database using shared memory and runs a sniffer.\n" );
  puts(explanation);

  /* setup memory device as a shared named memory region */
  dev.assignment = MCO_MEMORY_ASSIGN_DATABASE;  /* assign the device as a main database memory */
  dev.size       = DATABASE_SIZE;               /* set the device size */

  dev.type       = MCO_MEMORY_NAMED;            /* set the device as a shared named memory device */
  sprintf( dev.dev.named.name, "%s", db_name ); /* set memory name */
  dev.dev.named.flags = 0;                      /* zero flags */
  dev.dev.named.hint  = (info.direct_pointers_supported) ? DATABASE_MAP_ADDRESS : 0; /* set mapping address - .NET part should have this memory unallocated */

  /* initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* initialize the params with default values */

  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* set page size for the in memory part */
  db_params.disk_page_size     = 0;                /* set page size to zero to disable disk operations */
  db_params.db_max_connections = 3;                /* set total number of connections to the database */
  db_params.connection_context_size = sizeof(SAMPLE_OS_TASK_ID); /* set context size for the connection. This should match all clients*/
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

  /* open a database on the device with given params */
  rc = mco_db_open_dev(db_name, snifferdb_get_dictionary(), &dev, 1, &db_params );
  if ( MCO_S_OK == rc ) {
    SAMPLE_OS_TASK_ID pid = sample_os_task_id_get();

    /* the database was opened successfully */
    sample_rc_check("\tOpen database", rc );

    /* show runtime info for opened database */
    sample_show_runtime_info("\n\n\tThe opened database has the following characteristics:\n");

    sample_start_task(&sniffer_task, sniffer_loop, 0);

    printf("\nNow start Java or .NET application ConnectCtx to simulate process crashing...\n");

    while (!repair)
        sample_sleep(100);

    /* database now has some records - print them */

    rc = mco_db_connect_ctx(db_name, &pid, &db);

    if (MCO_S_OK == rc)
    {
        CHECK(mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t));

        if (MCO_S_OK == (rc = Obj_i4_index_index_cursor(t, &c)))
        {
            for (rc = mco_cursor_first(t, &c); rc == MCO_S_OK; rc = mco_cursor_next(t, &c))
            {
                Obj obj;
                uint4 ui4;

                rc = Obj_from_cursor(t, &c, &obj);
                if ( MCO_S_OK == rc ) {
                    int4 fld;
                    Obj_i4_get(&obj, &fld);
                    printf ("Object: i4=%d\n", fld);
                }
            }
            if (MCO_S_CURSOR_END != rc)
            {
                sample_rc_check("\tCursor operation failed", rc );
            }
            else
                rc = MCO_S_OK;
        } else {
            sample_rc_check("\tCursor operation failed", rc );
        }
        CHECK(mco_trans_commit(t));

        sample_stop_task(&sniffer_task);
        sample_join_task(&sniffer_task); 

        CHECK(mco_db_disconnect(db));
    } else {
        /* unable to connect to the database */
        sample_rc_check("\tDatabase connect failed", rc );
    }

    /* close the database */
    CHECK(mco_db_close(db_name));
    sample_rc_check("\tClose database", MCO_S_OK );

  } else {
    /* unable to open the database */
    /* check the return code for additional information */
    sample_rc_check("\tDatabase Open failed", rc );
  }
  
  /* stop eXtremeDB runtime */
  mco_runtime_stop();
 
  sample_pause_end("\n\nPress any key to exit . . . ");

  sample_os_shutdown();

  return ( MCO_S_OK == rc ? 0 : 1 );
}

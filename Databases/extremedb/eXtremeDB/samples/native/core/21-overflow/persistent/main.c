/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <overflowdb.h>

char sample_descr[] = {
  "Sample 'overflow_persistent' demonstrates the use of a memory allocation \ncallback.\n"
};
const char *db_name = "overflowdb";
static volatile int gc_flag    = 0;
static volatile int ac_flag    = 0;
static volatile uint4 key_vale = 0;

/* Garbage collector task deletes some objects from database */
void gc_task( sample_task_t *task )
{
  MCO_RET rc;
  mco_disk_info_t info;
  mco_offs_t max_size;

  /* Calculate maximum amount of used space to stop garbage collection */
  max_size = DATABASE_SIZE / 18 - DATABASE_SIZE / 90;

  while ( 1 != task->stopped )
  {
    if ( 0 != gc_flag )
    {
      printf( "\n\tDo garbage collection..." );
      do
      {
        Record       rec;
        mco_trans_h  t;
        mco_cursor_t c;
        int          i;

        /* Start transaction and delete some objects */
        rc = mco_trans_start( task->db_connection, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t );
        if ( MCO_S_OK != rc )
        {
          sample_rc_check( "\tStart transaction", rc );
          task->stopped = 2;
          return;
        }

        rc = Record_tkey_index_cursor( t, &c );
        if ( MCO_S_OK == rc )
        {
          for ( i = 0; i < 1000; i++ )
          {                        
            rc = mco_cursor_first( t, &c );
            if ( MCO_S_OK != rc )
            {
              sample_rc_check( "\tSet cursor at first position", rc );
              break;
            }

            rc = Record_from_cursor( t, &c, &rec );
            if ( MCO_S_OK != rc )
            {
              sample_rc_check( "\tGet record from cursor", rc );
              break;
            }

            rc = Record_delete( &rec );
            if ( MCO_S_OK != rc )
            {
              sample_rc_check( "\tDelete record", rc );
              break;
            }
          }
        }
        else
          sample_rc_check( "\tGet cursor", rc );

        if ( MCO_S_OK == rc )
          rc = mco_trans_commit( t );
        else
          rc = mco_trans_rollback( t );

        if ( MCO_S_OK != rc )
        {
          sample_rc_check( "\tFinish delete transaction", rc );
          task->stopped = 2;
          return;
        }

        /* Get current amount of used space and compare it with maximum value */
        rc = mco_disk_info( task->db_connection , &info );
        if ( MCO_S_OK != rc )
        {
          sample_rc_check( "\tGet disk info", rc );
          task->stopped = 2;
          return;
        }
      } while ( info.used_database_size > max_size );

      printf( "\n\tUsed space is %lld. End of collecting", (long long)info.used_database_size );
      gc_flag = 0;
    }
    sample_sleep( 300 );
  }
  task->stopped = 2;
}

/* Allocation callback registered by mco_register_disk_alloc_callback */
MCO_RET alloc_callback( mco_db_h db, mco_offs_t used )
{
  MCO_RET rc;
  mco_disk_info_t info;

  gc_flag = 1;
  ac_flag++;

  rc = mco_disk_info( db , &info );
  if ( MCO_S_OK != rc )
  {
    sample_rc_check( "\tGet disk info", rc );
  }
  printf( "\n\tAlloc callback is up %d times. Used space is %lld",
      ac_flag, (long long)info.used_database_size);
  return MCO_S_OK;
}

int main(int argc, char** argv) {
  MCO_RET             rc;
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
      /* Clean database content before filling */
      rc = mco_db_clean( connection );
      sample_rc_check( "\tClean database", rc );
      if ( MCO_S_OK == rc )
      {
        sample_task_t task;

        sample_start_connected_task( &task, gc_task, db_name, 0 );

        /* Register alloc callback */
        mco_register_disk_alloc_callback( connection, alloc_callback,
                                    MCO_ALLOC_USED, DATABASE_SIZE / 18 );                

        printf( "\n\tFill database ..." );

        /* Main loop simulates normal database activity */
        while ( 8 > ac_flag && 2 != task.stopped )
        {
          Record      rec;
          mco_trans_h t;
          int         i;

          /* Wait while garbage collector frees excess objects */
          while ( 1 == gc_flag ) sample_sleep( 300 );

          /* Start transaction and add objects to database */
          rc = mco_trans_start( connection, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t );
          if ( MCO_S_OK != rc )
          {
            sample_rc_check( "\tStart transaction", rc );
            break;
          }

          for ( i = 0; i < 100; i++ )
          {
            rc = Record_new( t, &rec );
            if ( MCO_S_OK != rc )
            {
              sample_rc_check( "\tNew record", rc );
              break;
            }

            rc = Record_key_put( &rec, key_vale );
            if ( MCO_S_OK != rc )
            {
              sample_rc_check( "\tUpdate record", rc );
              break;
            }
            key_vale++;
          }

          if ( MCO_S_OK == rc )
            rc = mco_trans_commit( t );
          else
            rc = mco_trans_rollback( t );

          if ( MCO_S_OK != rc )
          {
            sample_rc_check( "\tFinish write transaction", rc );
            break;
          }
        }

        /* Stop garbage collector task */
        sample_stop_task( &task );
        sample_join_task( &task );

        rc = mco_db_disconnect( connection );
        sample_rc_check( "\tDisconnect database", rc );
      }
    }

    rc = sample_close_database( db_name, &dbmem );
    sample_rc_check( "\tClose database", rc );
  }

  mco_runtime_stop();

  sample_pause_end( "\n\nPress any key to continue . . . " );

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

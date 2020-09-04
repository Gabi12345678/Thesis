/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <overflowdb.h>

char sample_descr[] = {
  "Sample 'overflow_transient' demonstrates the use of a memory allocation \ncallback.\n"
};
const char *db_name = "overflowdb";
static volatile int gc_flag    = 0;
static volatile int ac_flag    = 0;
static volatile uint4 key_vale = 0;

/* Garbage collector task deletes some objects from database */
void gc_task( sample_task_t *task )
{
  MCO_RET rc;
  mco_size32_t min_pages, free_pages;

  /*
  Note! If you extend memory space by calling mco_db_extend_dev
  or mco_db_extend during program execution then update information
  about total pages properly.
  */
  rc = mco_db_total_pages( task->db_connection,  &min_pages );
  if ( MCO_S_OK != rc )
  {
    sample_rc_check( "\tGet total pages count", rc );
    task->stopped = 2;
    return;
  }
  printf( "\n\tTotal pages count is %d", min_pages );

  /* Calculate minimum amount of free pages to stop garbage collection */
  min_pages = min_pages / 2 + min_pages / 20;

  while ( 1 != task->stopped )
  {
    if ( gc_flag )
    {
      printf( "\n\tRun garbage collection..." );
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

        /* Get current count of free pages and compare it with minimum value */
        rc = mco_db_free_pages( task->db_connection,  &free_pages );
        if ( MCO_S_OK != rc )
        {
          sample_rc_check( "\tGet free pages count", rc );
          task->stopped = 2;
          return;
        }
      } while ( free_pages < min_pages );

      printf( "\n\tFree pages count is %d. End of collecting", free_pages );
      gc_flag = 0;
    }
    sample_sleep( 300 );
  }
  task->stopped = 2;
}

/* Allocation callback registered by mco_register_alloc_callback */
MCO_RET alloc_callback( mco_db_h db, mco_offs_t used )
{
    MCO_RET rc;
    mco_size32_t free_pages = 0;

    gc_flag = 1;
    ac_flag++;

    rc = mco_db_free_pages( db,  &free_pages );
    if ( MCO_S_OK != rc )
    {
      sample_rc_check( "\tGet free pages count", rc );
    }
    printf( "\n\tAlloc callback is up %d times. Free pages count is %d"
                    , ac_flag, free_pages);
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
      sample_task_t task;

      sample_start_connected_task( &task, gc_task, db_name, 0 );

      /* Register alloc callback */
      mco_register_alloc_callback( connection, alloc_callback,
                                  MCO_ALLOC_USED, DATABASE_SIZE / 2 );            

      printf( "\n\tFill database up with data..." );

      /* Main loop simulates normal database activity */
      while ( ac_flag < 8 && task.stopped != 2)
      {
        Record      rec;
        mco_trans_h t;
        int         i;

        /* Wait while garbage collector frees excess objects */
        while ( gc_flag == 1 ) sample_sleep( 300 );                

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

    rc = sample_close_database( db_name, &dbmem );
    sample_rc_check( "\tClose database", rc );
  }

  mco_runtime_stop();

  sample_pause_end( "\n\nPress any key to continue . . . " );

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

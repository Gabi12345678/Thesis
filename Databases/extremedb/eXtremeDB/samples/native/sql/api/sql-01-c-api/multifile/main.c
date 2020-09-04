/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include <commonSQL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mcoapic.h>
#include "multifiledb.h"
const char * db_name  = "multifiledb";

/* define relatively small memory segment sizes to facilitate testing */
#define  DATABASE_SEGMENT_SIZE 2 * 1024 * 1024 
#define  CACHE_SEGMENT_SIZE    1 * 1024 * 1024 
#define  DISK_PAGE_SIZE        4096
#define  MULTIFILE_SEGMENT_SIZE 1 * 1024 * 1024
#define  MAX_DEVICES           10
#define  N_DEVICES             6
/* Redefine Database size and make sure Cache size is large enough for dirty page map */
#ifdef  DATABASE_SIZE 
  #undef DATABASE_SIZE
#endif
#define  DATABASE_SIZE         512*1024*1024
#define  DIRTY_PAGE_MAP_SIZE   DATABASE_SIZE / DISK_PAGE_SIZE / 8
#ifdef  CACHE_SIZE 
  #undef CACHE_SIZE
#endif
#define  CACHE_SIZE            CACHE_SEGMENT_SIZE + DIRTY_PAGE_MAP_SIZE

/* define nRecords large enough to cause creation of sufficient data to necessitate 
   use of multifile segments */
const unsigned int n_records = 20000;

static void extremedb_error_handler(MCO_RET error_code)
{
    fprintf(stderr, "eXtremeDB error %d\n", error_code);
    sample_os_shutdown();
    dbg_exit(1);
}

int main( int argc, char ** argv )
{
  MCO_RET           rc;
  status_t          rs;
  mco_db_h          db;
  mco_device_t      dev[N_DEVICES];  /* Memory devices for: 0) database, 1) cache, 2) main database storage, 
                                      3) extended database segment 1, 4) extended database segment 2, 5) transaction log */
  mco_db_params_t   db_params;
  database_t        engine;
  long int          i = 0;
  char              str[128];

  sample_os_initialize(DEFAULT);

  sample_header("Sample 'multifile' opens a multifile database using the eXtremeSQL C language API.\n" );
       
  /* set fatal error handler */
  mco_error_set_handler(extremedb_error_handler);

  /* start eXtremeDB runtime */
  mco_runtime_start();
   
  /* configure first memory device as a plain conventional memory region */
  dev[0].type       = MCO_MEMORY_CONV;
  dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;
  dev[0].size       = DATABASE_SEGMENT_SIZE;
  dev[0].dev.conv.ptr = (void*)malloc( DATABASE_SEGMENT_SIZE );

  /* configure convention memory region for cache */
  dev[1].type       = MCO_MEMORY_CONV;
  dev[1].assignment = MCO_MEMORY_ASSIGN_CACHE;
  dev[1].size       = CACHE_SIZE;
  dev[1].dev.conv.ptr = (void*)malloc( CACHE_SIZE );

  /* configure MULTIFILE memory device for main database storage */
  dev[2].type = MCO_MEMORY_MULTIFILE;
  dev[2].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;
  sprintf(dev[2].dev.multifile.name, FILE_PREFIX "%s0.dbs", db_name);
  dev[2].dev.multifile.flags = MCO_FILE_OPEN_DEFAULT;
  dev[2].dev.multifile.segment_size = MULTIFILE_SEGMENT_SIZE;

  /* configure MULTIFILE memory device for extended database segment 1 */
  dev[3].type = MCO_MEMORY_MULTIFILE;
  dev[3].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;
  sprintf(dev[3].dev.multifile.name, FILE_PREFIX "%s1.dbs", db_name);
  dev[3].dev.multifile.flags = MCO_FILE_OPEN_DEFAULT;
  dev[3].dev.multifile.segment_size = MULTIFILE_SEGMENT_SIZE;

  /* configure MULTIFILE memory device for extended database segment 2 */
  dev[4].type = MCO_MEMORY_MULTIFILE;
  dev[4].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;
  sprintf(dev[4].dev.multifile.name, FILE_PREFIX "%s2.dbs", db_name);
  dev[4].dev.multifile.flags = MCO_FILE_OPEN_DEFAULT;
  dev[4].dev.multifile.segment_size = MULTIFILE_SEGMENT_SIZE;

  /* configure FILE memory device for transaction log */
  dev[5].type       = MCO_MEMORY_FILE;
  dev[5].assignment = MCO_MEMORY_ASSIGN_LOG;
  sprintf(dev[5].dev.file.name, FILE_PREFIX "%s.log", db_name);
  dev[5].dev.multifile.flags = MCO_FILE_OPEN_DEFAULT;

  /* initialize and customize the database parameters */
  mco_db_params_init ( &db_params );                  /* initialize the params with default values */
  db_params.mem_page_size      = MEMORY_PAGE_SIZE;    /* set page size for in memory part */
  db_params.disk_page_size     = DISK_PAGE_SIZE;      /* set page size for persistent storage */
  db_params.db_max_connections = MAX_DEVICES;         /* set total number of connections to the database */
  db_params.db_log_type        = REDO_LOG;            /* set log type REDO */
  db_params.disk_max_database_size = DATABASE_SIZE;
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif

  /* open a database on the configured devices with given params */
  rc = mco_db_open_dev(db_name, multifiledb_get_dictionary(), dev, N_DEVICES, &db_params );
  if ( MCO_S_OK == rc ) {

    /* the database was opened successfully */
    sample_rc_check("\tOpen database", rc );

    /* connect to the database by name */   
    rc = mco_db_connect( db_name, &db );
   
    if ( MCO_S_OK == rc ) {

      /* set disk transaction policy to NO_SYNC to speed transactions */
      mco_disk_transaction_policy(db, MCO_COMMIT_NO_SYNC); 

      /* the database is open and a connection successfully established */
      sample_rc_check("\tConnect database", rc );
       
      /* Set SQL memory allocator to C runtime malloc/free */
      rs = mcosql_initialize_memory_manager(&malloc, &free);
      sample_rc_check("\tInitialize SQL memory allocator", (MCO_RET)rs );

      /* Initialize eXtremeDB SQL mapper */
      rs = mcoapi_initialize(db);
      sample_rc_check("\tInitialize SQL runtime", (MCO_RET)rs );

      /* Open SQL engine */
      rs = mcosql_open( &engine );
      sample_rc_check("\tOpen SQL engine", (MCO_RET)rs );

      /* show characteristics of opened database */
      sample_show_device_info("\n\tThe opened database has the following memory devices:", dev, N_DEVICES);

      sample_pause_end("\n\nPress any key to fill the database...");

      /* fill database with enough data to require extension into the multifile segments */
      for (i = 1; i <= n_records && MCO_S_OK == rc; i++) {
        sprintf( str, "Name%ld", i );
        rs = mcosql_execute_statement( engine, NULL, NULL, "insert into Person (name) values (%s)", str );
        if (i % 10000 == 0)
        {
          printf(".");
        }
      }

      sprintf( str, "\t%ld records inserted", i );
      sample_sql_rc_check( str, rc );

      /* Close SQL engine and disconnect from database */
      rs = mcosql_close( engine );
      sample_rc_check("\tClose SQL engine and disconnect", (MCO_RET)rs );
 
    } else {
      /* unable to connect to the database */
      /* check the return code for additional informaton */
      sample_rc_check("\tConnect failed", rc );
    }

    /* close the database */
    rc = mco_db_close(db_name);
    sample_rc_check("\tClose database", rc );
 
  } else {
    /* unable to open the database */
    /* check the return code for additional informaton */
    sample_rc_check("\tOpen failed", rc );
  }

  /* stop eXtremeDB runtime */
  mco_runtime_stop();
 
  /* free allocated memory */
  free( dev[0].dev.conv.ptr );
  free( dev[1].dev.conv.ptr );
  
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <genericdb.h>

char sample_descr[] = {
  "Sample 'disk_multifile' opens a database using MULTIFILE memory devices.\n"
};
const char * db_name = "disk_multifile";

#ifdef MEMORY_PAGE_SIZE
#undef MEMORY_PAGE_SIZE
#endif

#define  MEMORY_PAGE_SIZE      128
#define  DISK_PAGE_SIZE        1024
#define  MULTIFILE_SEGMENT_SIZE 100 * 1024
#define  MAX_DEVICES           10
#define  N_DEVICES             6

/* Define nRecords large enough to cause creation of sufficient data to necessitate 
   use of multifile segments */
const unsigned int nRecords = 10000;
/* Define transaction block size to reduce time for inserts */
const unsigned int nInsertsPerTransaction = 100;

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  mco_device_t       dev[N_DEVICES];  /* Memory devices for: 0) database, 1) cache, 2) main database storage, 
                                      3) extended database segment 1, 4) extended database segment 2, 5) transaction log */
  mco_db_params_t    db_params;
  mco_trans_h t;
  B rec;
  uint4 i, key;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  /* Configure first memory device as a plain conventional memory region */
  dev[0].type       = MCO_MEMORY_CONV;
  dev[0].assignment = MCO_MEMORY_ASSIGN_DATABASE;
  dev[0].size       = DATABASE_SIZE;
  dev[0].dev.conv.ptr = (void*)malloc( DATABASE_SIZE );

  /* Configure conventional memory region for cache */
  dev[1].type       = MCO_MEMORY_CONV;
  dev[1].assignment = MCO_MEMORY_ASSIGN_CACHE;
  dev[1].size       = CACHE_SIZE;
  dev[1].dev.conv.ptr = (void*)malloc( CACHE_SIZE );

  /* Configure MULTIFILE memory device for main database storage */
  dev[2].type = MCO_MEMORY_MULTIFILE;
  dev[2].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;
  sprintf(dev[2].dev.multifile.name, FILE_PREFIX "%s0.dbs", db_name);
  dev[2].dev.multifile.flags = MCO_FILE_OPEN_DEFAULT;
  dev[2].dev.multifile.segment_size = MULTIFILE_SEGMENT_SIZE;

  /* Configure MULTIFILE memory device for extended database segment 1 */
  dev[3].type = MCO_MEMORY_MULTIFILE;
  dev[3].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;
  sprintf(dev[3].dev.multifile.name, FILE_PREFIX "%s1.dbs", db_name);
  dev[3].dev.multifile.flags = MCO_FILE_OPEN_DEFAULT;
  dev[3].dev.multifile.segment_size = MULTIFILE_SEGMENT_SIZE;

  /* Configure MULTIFILE memory device for extended database segment 2 */
  dev[4].type = MCO_MEMORY_MULTIFILE;
  dev[4].assignment = MCO_MEMORY_ASSIGN_PERSISTENT;
  sprintf(dev[4].dev.multifile.name, FILE_PREFIX "%s2.dbs", db_name);
  dev[4].dev.multifile.flags = MCO_FILE_OPEN_DEFAULT;
  dev[4].dev.multifile.segment_size = MULTIFILE_SEGMENT_SIZE;

  /* Configure FILE memory device for transaction log */
  dev[5].type       = MCO_MEMORY_FILE;
  dev[5].assignment = MCO_MEMORY_ASSIGN_LOG;
  sprintf(dev[5].dev.file.name, FILE_PREFIX "%s.log", db_name);
  dev[5].dev.file.flags = MCO_FILE_OPEN_DEFAULT;

  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );                  /* Initialize the params with default values */
  db_params.mem_page_size      = MEMORY_PAGE_SIZE;    /* Set page size for in-memory part */
  db_params.disk_page_size     = DISK_PAGE_SIZE;      /* Set page size for persistent storage */
  db_params.db_max_connections = 1;                   /* Set total number of connections to the database */
  db_params.db_log_type        = UNDO_LOG;            /* Set log type */
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif
  
  /* Open a database on the configured devices with given params */
  rc = mco_db_open_dev(db_name, genericdb_get_dictionary(), dev, N_DEVICES, &db_params );
  if ( MCO_S_OK == rc ) {

    /* Connect to the database, obtain a database handle */
    rc = mco_db_connect(db_name, &db); /* No recovery connection data */
    if ( MCO_S_OK == rc ) {

      /* Show characteristics of opened database */
      sample_show_device_info("\n\tThe opened database has the following memory devices:", dev, N_DEVICES);

      sample_pause_end("\n\nPress any key to fill the database...");

      /* Fill database with enough data to require extension into the multifile segments */
      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      for (i = 1; i <= nRecords && MCO_S_OK == rc; i++) {
        key = i;
        rc = B_new(t, &rec);
        rc = B_ui4_put(&rec, key);

        if (i % nInsertsPerTransaction == 0)
        {
          printf(".");
          rc = mco_trans_commit(t);
          rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        }
      }
      if ( MCO_S_OK == rc ) {
        rc = mco_trans_commit(t);
      }

      mco_db_disconnect(db);
    }

    /* Close the database */
    mco_db_close(db_name);
  }

  /* Stop eXtremeDB runtime */
  mco_runtime_stop();

  /* Free allocated memory */
  free( dev[0].dev.conv.ptr );
  free( dev[1].dev.conv.ptr );
    
  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

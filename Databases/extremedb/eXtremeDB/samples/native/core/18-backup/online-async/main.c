/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <backupdb.h>

#define  DATABASE_SEGMENT_SIZE 1024*1024
const char * db_name = "autobackup";
int nAccounts = 10000;

void UpdateThread( sample_task_t * descriptor)
{
    MCO_RET rc = MCO_S_OK;

    while ( !descriptor->stopped && rc == MCO_S_OK) { 

        mco_trans_h t;

    rc = mco_trans_start( descriptor->db_connection, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);

    if ( rc == MCO_S_OK ) {

        uint4 src, dst;
        uint8 balance;
        A     a;

        src = rand() % nAccounts;
        dst = rand() % nAccounts;

        rc = A_by_id_find( t, src, &a );
        if ( rc == MCO_S_OK ) {
            A_balance_get( &a, &balance );
            A_balance_put( &a, balance-1 );
        }

        if ( rc == MCO_S_OK ) {
            rc = A_by_id_find( t, dst, &a );
        }
        
        if ( rc == MCO_S_OK ) {
            A_balance_get( &a, &balance );
            A_balance_put( &a, balance+1 );
        } 

        if ( rc == MCO_S_OK ) {
            rc = mco_trans_commit( t );
        } else {
            rc = mco_trans_rollback( t );
        }
    }
    }
}


int main(int argc, char* argv[])
{
  MCO_RET            rc;
  mco_db_h           con      = 0;
  mco_trans_h        t        = 0;
  mco_device_t       dev;
  mco_db_params_t    db_params;
  int                nUpdateThreads;
  sample_task_t    * updaters;
  mco_size_t         backup_file_records_count = 0;

  sample_os_initialize(DEFAULT);

  nAccounts      = argc > 1 ? atoi(argv[1]) : 10000;
  nUpdateThreads = argc > 2 ? atoi(argv[2]) : 3;

  updaters = (sample_task_t * )malloc( sizeof(sample_task_t) * nUpdateThreads );

  sample_header("Sample 'online-async' performs database operations while backing up the database in background.\n" );

  /* Start eXtremeDB runtime */
  mco_runtime_start();

  /* Setup memory device as a plain conventional memory region */
  dev.type       = MCO_MEMORY_CONV;                /* Set the device as a conventional memory device */
  dev.assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* Assign the device as main database memory */
  dev.size       = DATABASE_SEGMENT_SIZE;          /* Set the device size */
  dev.dev.conv.ptr = (void*)malloc( dev.size ); /* Allocate memory and set device pointer */
  
  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* Initialize the params with default values */

  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* Set page size for the in-memory part */
  db_params.disk_page_size     = 0;                /* Set page size to zero to disable disk operations */
  db_params.db_max_connections = nUpdateThreads+2; /* Set total number of connections to the database */
  //for a persistent database enable following lines
  //db_params.backup_map_size    = sizeof(int) * DATABASE_SEGMENT_SIZE / PSTORAGE_PAGE_SIZE / 16;
  //sprintf(db_params.backup_map_filename, "%s","Backup_map");
  db_params.backup_max_passes  = 10;
  db_params.backup_min_pages   = 10;
  db_params.mode_mask = MCO_DB_INCREMENTAL_BACKUP;
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif
    
  /* Open a database on the device with given params */
  rc = mco_db_open_dev(db_name, backupdb_get_dictionary(), &dev, 1, &db_params );
  if ( MCO_S_OK == rc ) {
    int i;

    /* The database was opened successfully */
    sample_rc_check("\n\tOpen database", rc );

    /* connect */
    rc = mco_db_connect(db_name, &con);
    sample_rc_check("\n\tConnect to the database", rc );

    if ( rc == MCO_S_OK ) {
    
        /* insert accounts in db */    
        rc = mco_trans_start(con, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        for (i = 0; rc == MCO_S_OK && i < nAccounts; i++) {
            A a;
            rc = A_new(t, &a);
            if ( rc == MCO_S_OK ) {
                A_id_put     (&a, (uint4)(i));
                A_balance_put(&a, 0 );
            }
        }
        rc = mco_trans_commit(t);
        printf("\n\t%u account(s) created", i);

    rc = mco_async_backup_start( con, 500, "file.backup", "label", MCO_BACKUP_TYPE_AUTO, 0, 0 );
        sample_rc_check("\n\tAsync backup started", rc );

        /* Run the database operations */
        for (i = 0; i < nUpdateThreads; ++i) {
            sample_start_connected_task(&updaters[i], UpdateThread, db_name, 0 );
        }

        printf("\n\tRunning the databse...\n" );
        for ( i=0; i<30; i++ ) {

        mco_async_backup_progress_info_t abinfo;

            rc = mco_async_backup_progress_info( con, &abinfo );
            printf("\t%3u. Async. backup info %u %s:\n"
                   "\t\t         Progress: Phase %s, Pass %llu of %llu, step %llu of %llu\n"
                   "\t\t        File name: %s\n"
                   "\t\t            Label: %s\n"
                   "\t\t             Type: %s\n"
                   "\t\tCompression level: %u\n"
                   "\t\t       Error code: %u %s\n"
                   "\t\t    Error message: %s\n\n",
                   i+1,
                   rc, mco_ret_string( rc, 0 ),
                   abinfo.core_info.phase == MCO_BACKUP_PROGRESS_IDLE ? "MCO_BACKUP_PROGRESS_IDLE" : 
                   abinfo.core_info.phase == MCO_BACKUP_PROGRESS_LOOPING ? "MCO_BACKUP_PROGRESS_LOOPING" : 
                   abinfo.core_info.phase == MCO_BACKUP_PROGRESS_FINALIZING ? "MCO_BACKUP_PROGRESS_FINALIZING" : "Invalid phase",
                   (unsigned long long)abinfo.core_info.pass_no, (unsigned long long)abinfo.core_info.max_passes,
                   (unsigned long long)abinfo.core_info.step_no, (unsigned long long)abinfo.core_info.max_steps,
                   abinfo.file_name  ? abinfo.file_name : "N/A",
                   abinfo.last_label ? abinfo.last_label: "N/A" , 
                   abinfo.type == MCO_BACKUP_TYPE_AUTO ? "MCO_BACKUP_TYPE_AUTO" :
                   abinfo.type == MCO_BACKUP_TYPE_SNAPSHOT ? "MCO_BACKUP_TYPE_SNAPSHOT" :
                   abinfo.type == MCO_BACKUP_TYPE_INCREMENTAL ? "MCO_BACKUP_TYPE_INCREMENTAL" : "Invalid type",
                   abinfo.compression_level,
                   abinfo.last_error_code, mco_ret_string(abinfo.last_error_code, 0 ),
                   abinfo.last_error_msg ? abinfo.last_error_msg : "N/A" );

            sample_sleep( 900 );
    
        }

        printf("\n\tStopping the activities\n" );
        for (i = 0; i < nUpdateThreads; ++i) {
            updaters[i].stopped = 1;
            sample_join_task( &updaters[i] );
        }

    rc = mco_async_backup_stop( con, 1 );
        sample_rc_check("\n\tAsync backup stopped", rc );

    /* list backup records */
    {
        mco_size_t          i;
        mco_backup_info_t * backup_file_records = 0;
        char                buf[1024] ="";
        unsigned int        buf_sz = sizeof(buf);
 
        rc = mco_backup_list( con, "file.backup", &backup_file_records_count, &backup_file_records, buf, &buf_sz );
        buf[buf_sz-1] = 0;
        printf("\n\n\tBackup file content ( %u %s %s ):\n", rc, mco_ret_string( rc, 0 ), buf );
        
        printf("\t#   Timestamp  Size       Label\n");
        for ( i=0; i<backup_file_records_count; i++ ) {
            printf("\t%3u %10llu %10llu %s\n", 
                           backup_file_records[i].backup_no, backup_file_records[i].timestamp, backup_file_records[i].size, backup_file_records[i].label );
        }

        mco_sys_free( backup_file_records );
    }

        /* disconnect */
        rc = mco_db_disconnect(con);
        sample_rc_check("\n\tDisconnect from the database", rc );
    }

    /* Close the database */
    mco_db_close(db_name);
    sample_rc_check("\n\tClose database", rc);
  } 
  else 
  {
    /* Unable to open the database */
    /* Check the return code for additional information */
    sample_rc_check("\tError opening database", rc );
  }

  printf("\n" );
  sample_pause_end("\n\nPress any key to continue . . . ");

  mco_runtime_stop();
  free(dev.dev.conv.ptr);
  free(updaters);
  sample_os_shutdown();
  return ( MCO_S_OK == rc && backup_file_records_count  > 0 ? 0 : 1 );
}

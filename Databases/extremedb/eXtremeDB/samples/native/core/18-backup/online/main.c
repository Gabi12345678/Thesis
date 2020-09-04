/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <stdio.h>
#include <common.h>
#include <backupdb.h>

#define  DATABASE_SEGMENT_SIZE 1024*1024
#define  MAX_RECORDS 1000
#define  INCREMENT 10
const char * db_name             = "backup";

int main(int argc, char* argv[])
{
  MCO_RET            rc;
  mco_db_h           db                       = 0;
  mco_trans_h        t                        = 0;
  mco_device_t       dev;
  mco_db_params_t    db_params;
  mco_cursor_t       c;

  /* generate a filename and a label */
  char * filename = "backup.bak";
  char * label = "label", * label2 = "label2", * label3 = "label3";

  char err_buf[512];
  unsigned int err_buf_sz;
  uint4 result = 0;
  int count = 0;

  sample_os_initialize(DEFAULT);

  /* Start eXtremeDB runtime */
  mco_runtime_start();

  /* Please delete old backup file before restarting program */
  sample_header("Sample 'inc_backup' performs 1 full backup and 2 incremental backups.\n" );

  /* Setup memory device as a plain conventional memory region */
  dev.type       = MCO_MEMORY_CONV;                /* Set the device as a conventional memory device */
  dev.assignment = MCO_MEMORY_ASSIGN_DATABASE;     /* Assign the device as main database memory */
  dev.size       = DATABASE_SEGMENT_SIZE;          /* Set the device size */
  dev.dev.conv.ptr = (void*)malloc( dev.size ); /* Allocate memory and set device pointer */
  
  /* Initialize and customize the database parameters */
  mco_db_params_init ( &db_params );               /* Initialize the params with default values */

  db_params.mem_page_size      = MEMORY_PAGE_SIZE; /* Set page size for the in-memory part */
  db_params.disk_page_size     = 0;                /* Set page size to zero to disable disk operations */
  db_params.db_max_connections = 10;               /* Set total number of connections to the database */
  //for a persistent database please set the following
  //db_params.backup_map_size    = sizeof(int) * DATABASE_SEGMENT_SIZE / PSTORAGE_PAGE_SIZE / 16;
  db_params.backup_max_passes  = 10;
  db_params.backup_min_pages   = 10;
  sprintf(db_params.backup_map_filename, "%s","Backup_map");
  db_params.mode_mask = MCO_DB_INCREMENTAL_BACKUP;
#ifdef EXTREMEDB_LICENSE_KEY
  db_params.license_key        = EXTREMEDB_LICENSE_KEY;
#endif
    
  /* Open a database on the device with given params */
  rc = mco_db_open_dev(db_name, backupdb_get_dictionary(), &dev, 1, &db_params );
  if ( MCO_S_OK == rc ) 
  {
    int i;

    /* The database was opened successfully */
    sample_rc_check("\tOpen database", rc );

    /* connect */
    rc = mco_db_connect(db_name, &db);
    
    /* insert 1000 records in db */    
    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    for (i = 0; i < MAX_RECORDS; i++)
    {
      A a;
      A_new(t, &a);
      A_id_put(&a, (uint4)(i));
    }
    rc = mco_trans_commit(t);
    printf("\n\n\t1000 records inserted");

    /* backup the database */
    err_buf_sz = sizeof(err_buf);
    rc = mco_backup_create(db, filename, label, MCO_BACKUP_TYPE_AUTO, 1, 0, err_buf, &err_buf_sz );
    printf("\n\t Full backup created\n");
   
    /* insert 10 more records */
    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    for (i = MAX_RECORDS; i < (MAX_RECORDS + INCREMENT); i++)
    {
      A a;
      A_new(t, &a);
      A_id_put(&a, (uint4)(i));
    }
    rc = mco_trans_commit(t);
    printf("\n\t10 more records inserted");

    /* incremental backup #1 */ 
    err_buf_sz = sizeof(err_buf);
    rc = mco_backup_create(db, filename, label2, MCO_BACKUP_TYPE_INCREMENTAL, 1, 0, err_buf, &err_buf_sz);
    printf("\n\t 1st incremental backup created\n");
  
    /* insert 10 more records */
    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    for (i = MAX_RECORDS + INCREMENT; i < (MAX_RECORDS + (2 * INCREMENT)); i++)
    {
      A a;
      A_new(t, &a);
      A_id_put(&a, (uint4)(i));
    }
    rc = mco_trans_commit(t);
    printf("\n\t10 more records inserted");

    /* incremental backup #2 */ 
    err_buf_sz = sizeof(err_buf);
    rc = mco_backup_create(db, filename, label3, MCO_BACKUP_TYPE_INCREMENTAL, 1, 0, err_buf, &err_buf_sz);
    printf("\n\t 2nd incremental backup created\n");
    
    /* disconnect */
    rc = mco_db_disconnect(db);
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
    
  /* Reopen db on new devices and restore only 1st incremental backup */

  /* Open a database on the device with given params */
  rc = mco_db_open_dev(db_name, backupdb_get_dictionary(), &dev, 1, &db_params);
  if (MCO_S_OK == rc) 
  {
    A a;

    /* The database was opened successfully */
    sample_rc_check("\n\tOpen database", rc);

    /* connect */
    rc = mco_db_connect(db_name, &db);

    /* restore backup according to label number */
    err_buf_sz = sizeof(err_buf);
    rc = mco_backup_restore(db, filename, label2, 0, err_buf, &err_buf_sz);
    sample_rc_check("\tRestore database", rc );

    /* Read from Database */
    printf("\n\n\tDisplaying records of selected incremental backup:\n");
    rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
    if (MCO_S_OK == rc)
    {
      rc = A_by_id_index_cursor(t, &c);
      if (MCO_S_OK == rc)
      {
        rc = mco_cursor_first(t, &c);
        if (MCO_S_OK == rc)
        {
          while (MCO_S_OK == rc)
          {
            A_from_cursor(t, &c, &a);
            A_id_get(&a, &result);
            printf("\tRecord #%i: %u\n", count, result);
            rc = mco_cursor_next(t, &c);
            count++;
          }
        }
        mco_cursor_close(t, &c);
      }
    }
    rc = mco_trans_rollback(t);

    /* disconnect */
    rc = mco_db_disconnect(db);
    /* Close the database */
    mco_db_close(db_name);
    sample_rc_check("\tClose database", rc);

    /* Stop eXtremeDB runtime */
  }
  else
  {
    /* Unable to open the database */
    /* Check the return code for additional information */
    sample_rc_check("\tError opening database", rc);
  }

  sample_pause_end("\n\nPress any key to continue . . . ");

  mco_runtime_stop();
  free(dev.dev.conv.ptr);
  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include <expimpdb.h>
#include <mcoxml.h>

char sample_descr[] = {
  "Sample 'export_import' demonstrates the use of eXtremeDB XML export/import APIs.\n" 
};
const char * db_name = "xmlexpimp";
static char fname[] = FILE_PREFIX "db.xml";
    
#define OBJCOUNT  100

/* File io helpers */
mco_size_sig_t file_writer(void* stream_handle /* FILE*  */, const void* from, mco_size_t nbytes)
{
  return (mco_size_t) fwrite(from, 1, nbytes, (FILE*) stream_handle);
}
mco_size_sig_t file_reader(void* stream_handle /* FILE*  */,  void* to, mco_size_t max_nbytes)
{
  return (mco_size_t) fread(to, 1, max_nbytes, (FILE*) stream_handle);
}

/* Initialize database */
MCO_RET init_database(mco_db_h db)
{
  MCO_RET rc = MCO_S_OK;
  int i;
  mco_trans_h t;
  Record rec;
  char name[64];

  printf("\tInitializing database...");
  for (i = 0; i < OBJCOUNT && !rc; i++)
  {
    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    if ( MCO_S_OK == rc ) {
      rc = Record_new(t, &rec);
      if ( MCO_S_OK == rc ) {
        sprintf(name, "Record %d", i );
        Record_idx_put(&rec, (uint2)i);
        Record_name_put(&rec, name, (uint2)strlen(name));
        rc = mco_trans_commit(t);
      } else {
        mco_trans_rollback(t);
      }
    }
  }
  if ( MCO_S_OK == rc ) {
    printf("\n\tCreated %d objects of class 'Record'\n", OBJCOUNT);
  } 
  return rc;
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  FILE* f;
  mco_trans_h t;
  mco_runtime_info_t info;
  sample_memory_t dbmem;
  char msg[128];

  sample_os_initialize(DEFAULT|DISK);

  /* Check for XML support */
  mco_get_runtime_info(&info);
  if (!info.mco_xml_supported)  {
    printf("This sample requires XML runtime support\n");
    sample_pause_end("\n\nPress any key to continue . . . ");
    sample_os_shutdown();
    return 1;
  }

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  /* Open and connect to database */
  rc = sample_open_database( db_name, expimpdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {
  
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Populate database */
      rc = init_database(db);
      if ( MCO_S_OK == rc ) {

        f = fopen(fname, "wb");
        if ( 0 != f ) {

          /* Export contents of the database in XML format to a file */
          rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
          if ( MCO_S_OK == rc ) {
            rc = mco_db_xml_export(t, f, file_writer);
            sprintf(msg, "\tExporting database to '%s'",  fname );
            sample_rc_check(msg, rc );
            mco_trans_rollback(t);
          }
          fclose(f);
        }
      }

      mco_db_disconnect(db);
    }

    sample_close_database(db_name, &dbmem);
  }
  
  /* Re-open and connect database */
  rc = sample_open_database( db_name, expimpdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {
  
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {
      /* Import contents from XML file */

      f = fopen(fname, "rb");
      if ( 0 != f) {

        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc )  {
          rc = mco_db_xml_import(t, f, file_reader);
          sprintf(msg, "\tImporting database from '%s'",  fname );
          sample_rc_check(msg, rc );
          if ( MCO_S_OK == rc ) {
            rc = mco_trans_commit(t);
          } else {
            mco_trans_rollback(t);
          }
        }
        fclose(f);
      }

      mco_db_disconnect(db);
    }

    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

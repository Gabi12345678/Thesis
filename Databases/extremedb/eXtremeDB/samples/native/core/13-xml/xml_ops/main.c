/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
/* 
 * This sample demonstrates basic XML operations.
 */
#include <stdio.h>
#include <common.h>
#include <mcoxml.h>
#include <xmlopdb.h>

char sample_descr[] = {
  "Sample 'xmlops' demonstrates basic XML operations.\n" 
};
const char * db_name = "xmlops";

/* Helper function to print an XML schema */
mco_size_sig_t file_writer(void* stream_handle /* FILE*  */, const void* from, mco_size_t nbytes)
{
  return (mco_size_t) fwrite(from, 1, nbytes, (FILE*) stream_handle);
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  mco_trans_h t;
  mco_runtime_info_t info;
  sample_memory_t dbmem;
  char xml_templ[] = "<Record><id>%d</id><name>%s</name></Record>"; /* Format string for Record's XML representation */
  char xml[128];
  Record rec; 

  sample_os_initialize(DEFAULT);

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
  rc = sample_open_database( db_name, xmlopdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {
  
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Create a database Record and write its XML definition to the output file */
      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        /* Create Record using the generated function <classname>_xml_create()  */
        sprintf(xml, xml_templ, 111, "Initial Record with Id=111");
        rc = Record_xml_create(t, xml, &rec);
        if ( MCO_S_OK == rc ) {
          printf("\n\tCreate database object...\n\n" );
          /* Print this Record in XML format to the output file */
          rc = Record_xml_get(&rec, stdout, file_writer);
          if ( MCO_S_OK == rc ) {
            mco_trans_commit(t);
          } else {
            mco_trans_rollback(t);
          }
        }
      }

      /* Find the database Record, update it and write its XML definition to the output file  */
      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        /* Find the Record */
        rc = Record_hid_find(t, 111, &rec);
        if ( MCO_S_OK == rc ) {
          /* Update Record from XML */
          sprintf(xml, xml_templ, 222, "Updated Record with Id=222");
          rc = Record_xml_put(&rec, xml);
          if ( MCO_S_OK == rc ) {
            printf("\n\tUpdate database object...\n\n" );
            /* Print out Record in XML form */
            rc = Record_xml_get(&rec, stdout, file_writer);
            if ( MCO_S_OK == rc ) {
              mco_trans_commit(t);
            } else {
              mco_trans_rollback(t);
            }
          }
        }
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

/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
/* 
 * This sample shows how to use <classname>_xml_schema() API.
 */
#include <stdio.h>
#include <common.h>
#include <mcoxml.h>
#include <xmlxsddb.h>

char sample_descr[] = {
  "Sample 'xmlschema' demonstrates use of the <classname>_xml_schema() API.\n" 
};
const char * db_name = "xmlschema";

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
  rc = sample_open_database( db_name, xmlxsddb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {
  
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Print out XML Schema for different classes */
      rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        printf("\n\tXML Schema for Person class...\n");
        Person_xml_schema(t, stdout, file_writer);

        printf("\n\n\tXML Schema for Children class...\n");
        Children_xml_schema(t, stdout, file_writer);

        printf("\n\n\tXML Schema for Dog class...\n");
        Dog_xml_schema(t, stdout, file_writer);

        mco_trans_commit(t);
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

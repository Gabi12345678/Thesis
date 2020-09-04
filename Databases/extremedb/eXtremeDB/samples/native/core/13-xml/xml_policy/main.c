/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
/* 
 * This sample demonstrates the use of mco_xml_get_policy()/mco_xml_set_policy().
 */
#include <stdio.h>
#include <common.h>
#include <mcoxml.h>
#include <xmlpoldb.h>

char sample_descr[] = {
  "Sample 'xmlpolicy' demonstrates the use of the XML Policy API.\n" 
};
const char * db_name = "xmlpolicy";

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
  mco_xml_policy_t policy;
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
  rc = sample_open_database( db_name, xmlpoldb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {
  
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Create a database object */
      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        char buf[] = "Text data";
        Record_new(t, &rec);
        Record_u4_put(&rec, 100);
        Record_u8_put(&rec, 100);
        Record_f_put(&rec, (float)123.456);
        Record_str_put(&rec, buf, sizeof(buf));
        Record_b_put(&rec, buf, sizeof(buf));
        mco_trans_commit(t);
      }

      /* Open READ_WRITE transaction to set XML policy */
      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        rc = Record_hu4_find(t, 100, &rec);
        if ( MCO_S_OK == rc ) {

          /* Get current (default) XML policy */
          mco_xml_get_policy(t, &policy); 
          printf("\n\tUsing Default policy...\n");
          Record_xml_get(&rec, stdout, file_writer);
          
          printf("\n\tUsing FIXED float format...\n");
          policy.float_format = MCO_FLOAT_FIXED;
          mco_xml_set_policy(t, &policy); /* set policy */
          Record_xml_get(&rec, stdout, file_writer);

          printf("\n\tUsing OCTAL integer base...\n");
          policy.int_base = MCO_NUM_OCT;
          mco_xml_set_policy(t, &policy); 
          Record_xml_get(&rec, stdout, file_writer);

          printf("\n\tUsing BINHEX blob coding...\n");
          policy.blob_coding = MCO_TEXT_BINHEX;
          mco_xml_set_policy(t, &policy); 
          Record_xml_get(&rec, stdout, file_writer);

          printf("\n\tUsing XML attributes...\n");
          policy.use_xml_attrs = MCO_YES;
          mco_xml_set_policy(t, &policy); 
          Record_xml_get(&rec, stdout, file_writer);

          printf("\n\tUsing indent OFF...\n");
          policy.indent = MCO_NO;
          mco_xml_set_policy(t, &policy); 
          Record_xml_get(&rec, stdout, file_writer);
          
          printf("\n\tRestore default policy...\n");
          mco_xml_get_default_policy(&policy);
          mco_xml_set_policy(t, &policy); /* set default policy */
          Record_xml_get(&rec, stdout, file_writer);
          
          mco_trans_commit(t);
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

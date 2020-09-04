/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
/* This sample demonstrates the use of the Uniform Data Access 
 * (UDA) API to perform various database operations 
 */
#include <common.h>
#include <mcouda.h>

#include "uda_mcodb.h"

char sample_descr[] = {
  "Sample 'uda_mco' demonstrates converting between UDA and native object handles.\n" 
};
const char* db_name = "uda_mco";
const unsigned int nRecords = 5;
const unsigned int vLength  = 3; /* vector's length */
mco_metadict_header_t *metadict;

/* Helper functions */
MCO_RET init_metadict()
{
  unsigned int size;
  mco_metadict_size(1, &size);
  metadict = (mco_metadict_header_t *) malloc(size);
  return mco_metadict_init (metadict, size, 0);
}

unsigned short get_field_no(mco_uda_object_handle_t *obj, const char *field_name)
{
  mco_dict_field_info_t field_info;
  mco_dict_field_name(metadict, 0, obj->struct_no, field_name, &field_info);
  return field_info.field_no;
}

unsigned short get_struct_no(const char *struct_name)
{
  mco_dict_struct_info_t struct_info;
  mco_dict_struct_name(metadict, 0, struct_name, &struct_info);
  return struct_info.struct_no;
}

unsigned short get_index_no(const char *struct_name, const char *index_name)
{
  mco_dict_index_info_t index_info;
  mco_dict_index_name(metadict, 0, get_struct_no(struct_name), index_name, &index_info);
  return index_info.index_no;
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  mco_trans_h t;
  mco_cursor_t csr;
  unsigned int i;
  mco_uda_object_handle_t rec;
  unsigned short rec_struct_no, rec_class_code;
  mco_uda_value_t value;
  sample_memory_t dbmem;
  char* fruits[] = 
  {
    "banana", "PEAR", "plum", "Peach", "apricot", "Kiwi", "QUINCE", "pineapple", "Lemon", "orange", "apple", 
    "pawpaw", "Fig", "mango", "MANDARIN", "Persimmon", "Grapefruit", 0
  };
    
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
   
  init_metadict();
  mco_metadict_register(metadict, db_name, uda_mcodb_get_dictionary(), 0);

  /* Open and connect to database */
  rc = sample_open_database( db_name, uda_mcodb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, MAX_DB_CONNECTIONS, &dbmem );
  rc = mco_db_connect(db_name, &db);
  if ( MCO_S_OK == rc ) {
  
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      /* Get struct and index id numbers */
      rec_struct_no = get_struct_no("Record");

      /* Populate database */
      for (i = 0; fruits[i] && MCO_S_OK == rc; ++i)
      {
        uda_mcodb_oid oid_struct;
        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {
          oid_struct.id = i;    
          mco_uda_new(t, rec_struct_no, &oid_struct, 0, 0, &rec);

          value.type = MCO_DD_UINT4;
          value.v.u4 = i;
          mco_uda_put(&rec, rec_struct_no, 0, &value);
          
          value.type = MCO_DD_STRING;
          value.v.p.len = (uint2)strlen(fruits[i]);
          value.v.p.p.c = fruits[i];
          mco_uda_put(&rec, get_field_no(&rec, "name"), 0, &value);

          printf("\n\tInserted: %-15s, value = %d", fruits[i], i);
          rc = mco_trans_commit(t);
        }
      }

      /* Get a record to show class code and demonstrate use of uda_to_mco handle functions */
      rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc) {
        Record rec_mco;
        Inner inner_mco;
        mco_uda_object_handle_t rec_uda;
        uint4 u4, inner_data;
        uint2 name_len;
        char name[32];

        CHECK(mco_uda_cursor(t, rec_struct_no, get_index_no("Record", "tu4"), &csr));
        CHECK(mco_cursor_first(t, &csr));
        CHECK(mco_uda_from_cursor(t, &csr, &rec)); /* Get object handle from cursor */
        CHECK(mco_uda_get_class_code(&rec, &rec_class_code));
        printf("\n\n\tThe class code for Record is %u\n", rec_class_code);

        CHECK(mco_uda_to_mco(&rec, (MCO_Hf *)&rec_mco));
        CHECK(Record_u4_get(&rec_mco, &u4));
        CHECK(Record_name_at(&rec_mco, 0, name, sizeof(name), &name_len));
        CHECK(Record_inner_read_handle(&rec_mco, &inner_mco));
        CHECK(Inner_inner_data_get(&inner_mco, &inner_data));
        printf("\tFirst record is: u4 = %d, name = %s, inner_data = %d\n", u4, name, inner_data);

        CHECK(mco_uda_from_mco((MCO_Hf *)&rec_mco, &rec_uda));
        value.type = MCO_DD_STRING;
        value.v.p.size = (uint2)sizeof(name);
        value.v.p.p.c = name;
        CHECK(mco_uda_get(&rec_uda, get_field_no(&rec_uda, "name"), 0, &value));
        printf("\tFirst record 'name' fetched via UDA API is: %s\n", name);

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

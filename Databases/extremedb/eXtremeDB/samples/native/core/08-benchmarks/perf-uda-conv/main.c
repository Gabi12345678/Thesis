/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <mcouda.h>
#include <common.h>
#include "uda_perfdb.h"
char sample_descr[] = "Sample 'perf_uda' demonstrates eXtremeDB performance for basic\n"
                      "operations using the Uniform Data Access (UDA) API.\n"  
                      "First N objects are inserted into a class, a hash\n"
                      "index is created on insertion; then separately a tree index\n"
                      "is built, searches using a tree and a hash table and then a\n"
                      "sequential search are performed.\n"
                      "Finally, the tree index is removed and all objects are deleted\n"
                      "one-by-one.  Each insert and delete is done in a separate\n"
                      "transaction, so that the commit time is included in measurements.\n";
const char * db_name = "uda_perf";

#undef DATABASE_SIZE
/* If you change the number of objects inserted, make sure that you
 * first have enough memory (DATABASE_SIZE), and also declare the hash table
 * size appropriately (hkey[estimated_number_of_entries] in perf.mco
 */
#ifdef _INTEGRITY
#define  DATABASE_SIZE        (2 * 1024 * 1024 * MEMORY_SIZE_X64_MULT + MEMORY_SIZE_WCHAR_ADD) 
unsigned int nRecords = 50000;
#else
#define  DATABASE_SIZE        (8 * 1024 * 1024 * MEMORY_SIZE_X64_MULT + MEMORY_SIZE_WCHAR_ADD)
unsigned int nRecords = 100000;
#endif

mco_metadict_header_t *metadict;

void init_metadict()
{
  unsigned int size;
  mco_metadict_size(1, &size);
  metadict = (mco_metadict_header_t *) malloc(size);
  mco_metadict_init (metadict, size, 0);
}

unsigned short get_field_no(unsigned short struct_no, const char *field_name)
{
  mco_dict_field_info_t field_info;
  if ( MCO_S_OK != mco_dict_field_name(metadict, 0, struct_no, field_name, &field_info) ) 
    return (unsigned short) -1;
  return field_info.field_no;
}

unsigned short get_struct_no(const char *struct_name)
{
  mco_dict_struct_info_t struct_info;
  if ( MCO_S_OK != mco_dict_struct_name(metadict, 0, struct_name, &struct_info) ) 
    return (unsigned short) -1;
  return struct_info.struct_no;
}

unsigned short get_index_no(const char *struct_name, const char *index_name)
{
  mco_dict_index_info_t index_info;
  if ( MCO_S_OK != mco_dict_index_name(metadict, 0, get_struct_no(struct_name), index_name, &index_info) ) 
    return (unsigned short) -1;
  return index_info.index_no;
}

void err_in_trn(uint4 trn_no, MCO_RET rc, char* file, int line)
{
  printf("%s@%d: error in transaction. Trans. #%d, rc==%d\n", file, line, trn_no, rc);
  getchar();
  sample_os_shutdown();
  dbg_exit( - 1);
};

int main(int argc, char* argv[])
{
  time_t start_time;
  MCO_RET rc;
  mco_db_h db = 0;
  sample_memory_t dbmem;
  mco_trans_h t;
  uint4 i;
  long n = 0;
  mco_uda_object_handle_t rec;
  mco_uda_value_t value;
  mco_uda_value_t keys[1];
  mco_cursor_t csr;
  unsigned short Record_struct_no, key_field_no, tkey_index_no, hkey_index_no; /* dictionary numbers */
  mco_uda_object_handle_t obj;
  mco_runtime_info_t rt_info;
  
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  mco_get_runtime_info(&rt_info);
  if (rt_info.disk_supported)
    nRecords = 200;
    
  init_metadict();
  mco_metadict_register(metadict, db_name, uda_perfdb_get_dictionary(), 0);

  /* Open databases */
  rc = sample_open_database( db_name, uda_perfdb_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if (rc) {
    sample_rc_check("\n\tUnable to Open Database", rc );
    sample_pause_end("\n\nPress any key to continue . . . ");
    sample_os_shutdown();
    dbg_exit(1);
  }

  /* Connect to the database, obtain a database handle */
  mco_db_connect(db_name, &db);

  /* Get record and field numbers (from dictionary) into local variables to avoid repeated calls */
  Record_struct_no = get_struct_no("Record");
  key_field_no = get_field_no(Record_struct_no, "key");
  hkey_index_no = get_index_no("Record", "hkey");
  tkey_index_no = get_index_no("Record", "tkey");

  /* Insert Records, don't create the tree index yet */
  printf("\tInsert:\n\t");
  start_time = sample_msec();

  for (i = 0; i < nRecords; i++) {

    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

    rc = mco_uda_new(t, Record_struct_no, 0, 0, 0, &rec);
    if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

    value.type = MCO_DD_UINT4; 
    value.v.u4   = i * 2;       /* Store even values for key */
    rc = mco_uda_put(&rec, key_field_no, 0, &value);
    if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

    if (i % (nRecords / 10) == 0)
    {
      printf(".");
    }

    rc = mco_trans_commit(t);
    if (rc) err_in_trn(i, rc, __FILE__, __LINE__);
  }

  printf("\n\t%d objects: %d milliseconds,(%d microsecs/object)\n", (int)i, 
          (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / i));
    
  /* Hash search */
  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
  if (rc) err_in_trn(0, rc, __FILE__, __LINE__);

  printf("\n\tHash search:\n\t");
  start_time = sample_msec();
  rc = mco_uda_cursor(t, Record_struct_no, hkey_index_no, &csr);
  if (rc) err_in_trn(0, rc, __FILE__, __LINE__);
  keys[0].type = MCO_DD_UINT4;      
  value.type = MCO_DD_UINT4;
  for (i = 0; i < nRecords * 2; i++) {
    keys[0].v.u4 = i;
    rc = mco_uda_lookup(t, Record_struct_no, hkey_index_no, MCO_EQ, keys, 1, &csr);
    if (i % 2 == 0)
    {
      if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

      rc = mco_uda_from_cursor(t, &csr, &obj);
      if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

      rc = mco_uda_get(&obj, key_field_no, 0, &value);
      if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

      if (i != value.v.u4)
      {
        err_in_trn(i, rc, __FILE__, __LINE__);
      }
    }
    else
    {
      if (MCO_S_OK == rc)
      {
        err_in_trn(i, rc, __FILE__, __LINE__);
      }
    }

    if (i % (nRecords / 10) == 0)
    {
      printf(".");
    }
  }
  printf("\n\t%d searches: %d milliseconds (%d microsecs/search)\n", nRecords, 
          (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / nRecords));

  mco_trans_commit(t);

  /* Tree search */
  printf("\n\tTree search:\n\t");
  start_time = sample_msec();

  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
  if (rc) err_in_trn(0, rc, __FILE__, __LINE__);

  rc = mco_uda_cursor(t, Record_struct_no, tkey_index_no, &csr);
  if (rc) err_in_trn(0, rc, __FILE__, __LINE__);
  keys[0].type = MCO_DD_UINT4;
  value.type = MCO_DD_UINT4;
  for (i = 0; i < nRecords * 2; i++)
  {
    keys[0].v.u4 = i;
    rc = mco_uda_lookup(t, Record_struct_no, tkey_index_no, MCO_EQ, keys, 1, &csr);
    if (i % 2 == 0)
    {
      if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

      rc = mco_uda_from_cursor(t, &csr, &obj);
      if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

      rc = mco_uda_get(&obj, key_field_no, 0, &value);
      if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

      if ( i != value.v.u4 )
      {
        err_in_trn(i, rc, __FILE__, __LINE__);
      }

    }
    else
    {
      if (MCO_S_OK == rc)
      {
        err_in_trn(i, rc, __FILE__, __LINE__);
      }
    }

    if (i % (nRecords / 10) == 0)
    {
      printf(".");
    }
  }
  printf("\n\t%d searches: %d milliseconds (%d microsecs/search)\n", nRecords, 
          (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / nRecords));
  mco_trans_commit(t);

  /* Cursor movements */
  printf("\n\tSequential:\n\t");
  start_time = sample_msec();
  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
  if (rc) err_in_trn(0, rc, __FILE__, __LINE__);

  rc = mco_uda_cursor(t, Record_struct_no, tkey_index_no, &csr);
  if (rc) err_in_trn(0, rc, __FILE__, __LINE__);
  value.type = MCO_DD_UINT4;
  for (i = 0, n = 0, rc = mco_cursor_first(t, &csr); MCO_S_OK == rc; rc = mco_cursor_next(t, &csr), i++)
  {
    rc = mco_uda_from_cursor(t, &csr, &obj);
    if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

    rc = mco_uda_get(&obj, key_field_no, 0, &value);
    if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

    if ( i * 2 != value.v.u4 )
    {
      err_in_trn(i, rc, __FILE__, __LINE__);
    }

    if (n % (nRecords / 10) == 0)
    {
      printf(".");
    }
    n++;
  }
  if (MCO_S_CURSOR_END != rc)
  {
    err_in_trn(n, rc, __FILE__, __LINE__);
  }

  printf("\n\t%d searches: %d milliseconds (%d microsecs/search)\n", (int)n, 
          (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / n));
  rc = mco_trans_commit(t);
  if (rc) err_in_trn(0, rc, __FILE__, __LINE__);

  /* Search using hash index and delete all objects  */
  printf("\n\tSearch/delete:\n\t");
  start_time = sample_msec();
  keys[0].type = MCO_DD_UINT4;
  value.type = MCO_DD_UINT4;
  for (i = 0; i < (nRecords * 2); i++)
  {
    keys[0].v.u4 = i;

    rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
    if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

    if ( 0 == i ) {
      rc = mco_uda_cursor(t, Record_struct_no, hkey_index_no, &csr);
      if (rc) err_in_trn(i, rc, __FILE__, __LINE__);
    }

    rc = mco_uda_lookup(t, Record_struct_no, hkey_index_no, MCO_EQ, keys, 1, &csr);
    if (i % 2 == 0)
    {
      if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

      rc = mco_uda_from_cursor(t, &csr, &obj);
      if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

      rc = mco_uda_get(&obj, key_field_no, 0, &value);
      if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

      if (i != value.v.u4)
      {
        err_in_trn(i, rc, __FILE__, __LINE__);
      }

      rc = mco_uda_delete(&obj);
      if (rc) err_in_trn(i, rc, __FILE__, __LINE__);
    }
    else
    {
      if (MCO_S_OK == rc)
      {
        err_in_trn(i, rc, __FILE__, __LINE__);
      }
    };

    rc = mco_trans_commit(t);
    if (rc) err_in_trn(i, rc, __FILE__, __LINE__);

    if (i % (nRecords / 10) == 0)
    {
      printf(".");
    }
  }
  printf("\n\t%d objects: %d milliseconds (%d microsecs/object)\n", (unsigned)i, 
          (int)(sample_msec() - start_time), (int)(((sample_msec() - start_time)* 1000) / i));

  /* Disconnect and close database, stop runtime */
  mco_db_disconnect(db);
  sample_close_database(db_name, &dbmem);
  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return 0;
}

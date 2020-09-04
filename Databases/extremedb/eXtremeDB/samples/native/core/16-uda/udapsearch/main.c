/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#include <common.h>
#include <mcowrap.h>
#include "udapsearch.h"

char sample_descr[] = "Sample 'udapsearch' demonstrates basic eXtremeDB pattern search methods \nusing UDA API.\n";
const char * db_name = "psearch";

/* Default search patterns */
#define PSEARCH_K1 "abc_1*"
#define PSEARCH_K2 "??c_?2?"
#define PSEARCH_K3 1

#define NUMOBJECTS    20

mco_metadict_header_t *metadict;

/* Helper functions */
MCO_RET init_metadict()
{
  unsigned int size;
  mco_metadict_size(1, &size);
  metadict = (mco_metadict_header_t *) malloc(size);
  return mco_metadict_init (metadict, size, 0);
}

unsigned short get_struct_no(const char *struct_name)
{
  mco_dict_struct_info_t struct_info;
  mco_dict_struct_name(metadict, 0, struct_name, &struct_info);
  return struct_info.struct_no;
}

unsigned short get_field_no(unsigned short struct_no, const char *field_name)
{
  mco_dict_field_info_t field_info;
  mco_dict_field_name(metadict, 0, struct_no, field_name, &field_info);
  return field_info.field_no;
}


unsigned short get_index_no(unsigned short struct_no, const char *index_name)
{
  mco_dict_index_info_t index_info;
  mco_dict_index_name(metadict, 0, struct_no, index_name, &index_info);
  return index_info.index_no;
}



/* Pattern search 
 * You can change '?' and '*' symbols to any ascii char
 * or you can ignore fields other than char<n> or string.
 */
MCO_RET psearch(mco_db_h db, char* pattern1, char* pattern2, int p3, mco_bool ignore_other_fields)
{
  MCO_RET rc = MCO_S_OK;
  mco_trans_h trn;
  mco_cursor_t csr;
  mco_uda_object_handle_t test;
  uint2 size1;
  uint4 bsize;
  char key[64], k2[64];
  int4 k3, v1;
  void* buf;
  mco_pattern_policy_t p;
  unsigned short s_no = get_struct_no("TestClass");
  unsigned short i_no = get_index_no(s_no, "i1");
  unsigned short f_no;
  mco_uda_value_t keys[3], value;

  /* Get memory for set of patterns */
  keys[0].type = MCO_DD_STRING;
  keys[0].v.p.p.c = pattern1;
  keys[0].v.p.len = (uint2)strlen(pattern1);

  keys[1].type = MCO_DD_CHAR;
  keys[1].v.p.p.c = pattern2;
  keys[1].v.p.len = (uint2)strlen(pattern2);
  
  keys[2].type = MCO_DD_INT4;
  keys[2].v.i4   = p3;

  mco_uda_pattern_size(metadict, 0, s_no, i_no, keys, 3, &bsize);
  buf = malloc(bsize);

  /* Start read-write transaction to change pattern policy */
  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &trn);
  if ( MCO_S_OK == rc ) {
    mco_get_pattern_policy(trn, &p);
    p.ignore_other_fields = ignore_other_fields; /* Ignore fields other than char<n> or string */
    mco_set_pattern_policy(trn, &p);

    rc = mco_uda_cursor(trn, s_no, i_no, &csr);
    
    for (rc = mco_uda_pattern_search(trn, s_no, i_no, keys, 3, &csr, buf); rc == MCO_S_OK; rc = mco_uda_pattern_next(trn, &csr, buf))
    {
      mco_uda_from_cursor(trn, &csr, &test);

      f_no = get_field_no(s_no, "v1");
      value.type = MCO_DD_INT4;
      mco_uda_get(&test, f_no, 0, &value);
      v1 = value.v.i4;
      
      f_no = get_field_no(s_no, "k3");
      value.type = MCO_DD_INT4;
      mco_uda_get(&test, f_no, 0, &value);
      k3 = value.v.i4;
      
      f_no = get_field_no(s_no, "key");
      value.type = MCO_DD_STRING;
      value.v.p.p.c = key;
      value.v.p.size = sizeof(key);
      mco_uda_get(&test, f_no, 0, &value);
      size1 = value.v.p.len;
      
      f_no = get_field_no(s_no, "k2");
      value.type = MCO_DD_CHAR;
      value.v.p.p.c = k2;
      value.v.p.size = sizeof(k2);
      mco_uda_get(&test, f_no, 0, &value);

      printf("\t Found: v1=%d, key=%s(%d), k2=%s(%d), k3=%d\n", v1, key, size1, k2, (int)strlen(k2), k3);
    }
    rc = MCO_S_OK;
    mco_trans_commit(trn);
  } else {
    sample_rc_check("\tUnable to start transaction", rc );
  }
  free(buf);
  return rc;
}

MCO_RET newobj(mco_db_h db, int id)
{
  MCO_RET rc;
  mco_trans_h t;
  mco_uda_object_handle_t hClass;
  char src1[64], src2[64];
  int4 v1 = (int4)(rand() &0x3FF);
  int4 k3 = (int4)(rand() % 3);
  unsigned short s_no = get_struct_no("TestClass");
  unsigned short f_no;
  mco_uda_value_t value;

  /* Open a read_write transaction with FOREGROUND priority level */
  rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {

    rc = mco_uda_new(t, s_no, 0, 0, 0, &hClass);

    f_no = get_field_no(s_no, "v1");
    value.type = MCO_DD_INT4;
    value.v.i4 = v1;
    mco_uda_put(&hClass, f_no, 0, &value);

    f_no = get_field_no(s_no, "k3");
    value.type = MCO_DD_INT4;
    value.v.i4 = k3;
    mco_uda_put(&hClass, f_no, 0, &value);

    f_no = get_field_no(s_no, "key");
    sprintf(src1, "abc_%03d", rand() & 0xFF);
    value.type = MCO_DD_STRING;
    value.v.p.p.c = src1;
    value.v.p.len = (uint2)strlen(src1);
    mco_uda_put(&hClass, f_no, 0, &value);
    
    f_no = get_field_no(s_no, "k2");
    sprintf(src2, "abc_%03d", rand() & 0x1F);
    value.type = MCO_DD_CHAR;
    value.v.p.p.c = src2;
    value.v.p.len = (uint2)strlen(src2);
    mco_uda_put(&hClass, f_no, 0, &value);

    rc = mco_trans_commit(t);
    if ( MCO_S_OK != rc ) {
      sample_rc_check("Unable to commit transaction", rc );
    }
    printf("\n\t v1=%d, k1=%s, k2=%s k3=%d\tinserted okay", v1, src1, src2, k3);

  } else {
    sample_rc_check("\tUnable to start transaction", rc );
  }
  return rc;
}

int main(int argc, char** argv)
{
  MCO_RET rc;
  mco_db_h db = 0;
  int num = 0;
  sample_memory_t dbmem;
  char * pattern1 = PSEARCH_K1;
  char * pattern2 = PSEARCH_K2;
  int    pattern3 = PSEARCH_K3;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Convert optional command line args to pattern values */
  if ( argc > 1 ) {
    pattern1 = argv[1];
  }
  if ( argc > 2 ) {
    pattern2 = argv[2];
  }
  if ( argc > 3 ) {
    pattern3 = atoi(argv[3]);
  }
 
  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  init_metadict();
  mco_metadict_register(metadict, db_name, udapsearch_get_dictionary(), 0);
    

  /* Open and connect to database */
  rc = sample_open_database( db_name, udapsearch_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {

      srand((unsigned)time(NULL));
      printf("\n\tFilling database with random values:");

      for (num = 0; num < NUMOBJECTS && MCO_S_OK == rc; ++num)  {
        rc = newobj(db, num);
      }
      if (MCO_S_OK == rc) {

        printf("\n\n\tStart search using all patterns and other index components\n");
        rc = psearch(db, pattern1, pattern2, pattern3, MCO_NO);
        if (MCO_S_OK == rc) {
          printf("\n\tStart search using all patterns and ignore other index components\n");
          psearch(db, pattern1, pattern2, pattern3, MCO_YES);
        }
      }

      mco_db_disconnect(db);
    }
  
    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();
  free(metadict);

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

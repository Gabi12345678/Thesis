/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
/*
 * This sample demonstrates custom collation indexes using the UDA API
 */
#include <common.h>
#include <mcouda.h>

#include "udacoll.h"

#ifndef _WIN32
#ifdef _ECOS
    #include <string.h>
#else 
#ifndef _NETOS
    #include <strings.h>
#endif 
#endif 
    #define STR_CMP strcasecmp
#else 
    #include <string.h>
    #define STR_CMP stricmp
#endif 

char sample_descr[] = {
  "Sample 'udacoll' demonstrates custom collations with tree & hash indexes using \nthe UDA API.\n" 
};
const char* db_name = "udacoll";
mco_metadict_header_t *metadict;
unsigned short Record_no, name_no, value_no, tstd_no, tcoll_no, coll_no;

/* Helper functions */
unsigned short get_struct_no(const char *struct_name)
{
  mco_dict_struct_info_t struct_info;
  if (mco_dict_struct_name(metadict, 0, struct_name, &struct_info) != MCO_S_OK) return (unsigned short) -1;
  return struct_info.struct_no;
}

unsigned short get_index_no(const char *struct_name, const char *index_name)
{
  mco_dict_index_info_t index_info;
  if (mco_dict_index_name(metadict, 0, get_struct_no(struct_name), index_name, &index_info) != MCO_S_OK) return (unsigned short) -1;
  return index_info.index_no;
}

unsigned short get_field_no(unsigned short struct_no, const char *field_name)
{
  mco_dict_field_info_t field_info;
  if ( MCO_S_OK != mco_dict_field_name(metadict, 0, struct_no, field_name, &field_info) ) return (unsigned short) -1;
  return field_info.field_no;
}

unsigned short get_collation_no(const char *coll_name)
{
    mco_dict_collation_info_t coll_info;
    if (mco_dict_collation_name(metadict, 0, coll_name, &coll_info) != MCO_S_OK) return (unsigned short) -1;
    return coll_info.collation_no;
}

/* Custom compare & hash functions */
int2 coll_cmp(mco_collate_h c1, uint2 len1, mco_collate_h c2, uint2 len2)
{
  mco_uda_value_t val1, val2;
  char buf1[20], buf2[20];

  /* Get first object's value */
  val1.type = MCO_DD_STRING;
  val1.v.p.size = sizeof(buf1);
  val1.v.p.p.c = buf1;
  mco_uda_collate_get(c1, &val1);

  /* Get second object's value */
  val2.type = MCO_DD_STRING;
  val2.v.p.size = sizeof(buf2);
  val2.v.p.p.c = buf2;
  mco_uda_collate_get(c2, &val2);
  
  /* Compare values */
  return STR_CMP(buf1, buf2);
}


mco_hash_counter_t coll_hash(mco_collate_h c, uint2 len)
{
  mco_uda_value_t val;
  char buf[20];

  /* Get object's value */
  val.type = MCO_DD_STRING;
  val.v.p.size = sizeof(buf);
  val.v.p.p.c = buf;
  mco_uda_collate_get(c, &val);

  /* Hash value */
  return strlen(buf);
}

/* Sequentially read index values */
MCO_RET sequential(mco_db_h db, unsigned short index_no)
{
  MCO_RET rc;
  mco_trans_h t;
  mco_cursor_t c;
  mco_uda_value_t value;
  mco_uda_object_handle_t obj;
  char buf[12];

  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {

    /* Open cursor */
    rc = mco_uda_cursor(t, Record_no, index_no, &c);
    if ( MCO_S_OK == rc ) {

      for (rc = mco_cursor_first(t, &c); MCO_S_OK == rc; rc = mco_cursor_next(t, &c)) {
        mco_uda_from_cursor(t, &c, &obj);
        value.type = MCO_DD_STRING;
        value.v.p.p.c = buf;
        value.v.p.size = 11;
        mco_uda_get(&obj, name_no, 0, &value);

        value.type = MCO_DD_UINT4;
        mco_uda_get(&obj, value_no, 0, &value);
        printf("\n\t%-15s, value = %d", buf, value.v.u4);
      }
      rc = mco_trans_commit(t);
    }
  }

  return rc;
}

MCO_RET find(mco_db_h db, unsigned short index_no, char *search, uint4 *val ) 
{
  MCO_RET rc;
  mco_trans_h t;
  mco_cursor_t c;
  mco_uda_value_t value;
  mco_uda_object_handle_t obj;
  char buf[12];

  rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
  if ( MCO_S_OK == rc ) {
    rc = mco_uda_cursor(t, Record_no, index_no, &c);
    if ( MCO_S_OK == rc ) {
      value.type = MCO_DD_STRING;
      value.v.p.len = strlen(search);
      value.v.p.p.c = search;
      rc = mco_uda_lookup(t, Record_no, index_no, MCO_EQ, &value, 1, &c);
      if ( MCO_S_OK == rc ) {
        mco_uda_from_cursor(t, &c, &obj);
        value.type = MCO_DD_STRING;
        value.v.p.p.c = buf;
        value.v.p.size = 11;
        mco_uda_get(&obj, name_no, 0, &value);

        value.type = MCO_DD_UINT4;
        mco_uda_get(&obj, value_no, 0, &value);
        *val = value.v.u4;
        rc = mco_trans_commit(t);
      } else {
        mco_trans_rollback(t);
      }
    } else {
      mco_trans_rollback(t);
    }
  }
  return rc;
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_db_h db = 0;
  mco_trans_h t;
  int i;
  char search[11];
  mco_uda_object_handle_t obj;
  mco_uda_value_t value;
  unsigned int coll_map_size;
  mco_collation_funcs_h coll_map; 
  unsigned int metadict_size;
  sample_memory_t dbmem;
  uint4 value_u4;
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

  /* Initialize metadict and register dictionary */
  mco_metadict_size(1, &metadict_size);
  metadict = (mco_metadict_header_t *) malloc(metadict_size);
  mco_metadict_init (metadict, metadict_size, 0);
  mco_metadict_register(metadict, db_name, udacoll_get_dictionary(), 0);

  /* Get class/field/index numbers */
  Record_no = get_struct_no("Record"); 
  name_no   = get_field_no(Record_no, "name");
  value_no  = get_field_no(Record_no, "value");
  tstd_no   = get_index_no("Record", "tstd");
  tcoll_no  = get_index_no("Record", "tcoll");
  coll_no   = get_collation_no("C1");

  /* Open database */
  rc = sample_open_database( db_name, udacoll_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, 5, &dbmem );
  if ( MCO_S_OK == rc ) {

    /* Allocate collmap */
    mco_uda_get_collmap_size(metadict, 0, &coll_map_size);
    coll_map = (mco_collation_funcs_h) malloc(coll_map_size);

    /* Register collation compare & hash functions */
    rc = mco_uda_register_collation(metadict, 0, coll_no, coll_map, &coll_cmp, &coll_hash);
    if ( MCO_S_OK == rc ) {

      rc = mco_db_connect(db_name, &db);
      if ( MCO_S_OK == rc ) {

        /* Populate database */
        for (i = 0; fruits[i] && MCO_S_OK == rc; ++i)
        {
          rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
          if ( MCO_S_OK == rc ) {
            mco_uda_new(t, Record_no, 0, 0, 0, &obj);

            value.type = MCO_DD_UINT4;
            value.v.u4 = i;
            mco_uda_put(&obj, value_no, 0, &value);
            
            value.type = MCO_DD_STRING;
            value.v.p.len = (uint2)strlen(fruits[i]);
            value.v.p.p.c = fruits[i];
            mco_uda_put(&obj, name_no, 0, &value);

            rc = mco_trans_commit(t);
            printf("\n\tInserted: %-15s, value = %d", fruits[i], i);
          }
        }

        /* Sequential read */
        printf("\n\n\tStandard (case-sensitive) sequential :");
        sequential(db, tstd_no);
        printf("\n\n\tCustom (case-insensitive) sequential :");
        sequential(db, tcoll_no);

        /* Find */
        strcpy(search, "kiwi");
        printf("\n\n\tFind by name \"%s\" :", search);
        rc = find(db, tstd_no, search, &value_u4);
        if ( MCO_S_OK == rc ) {
          printf("\n\tStandard (case-sensitive): Found '%s', value = %d", search, value_u4);
        } else {
          printf("\n\tStandard (case-sensitive) - '%s' not found", search );
        }

        rc = find(db, tcoll_no, search, &value_u4);
        if ( MCO_S_OK == rc ) {
          printf("\n\tCustom (case-insensitive): Found '%s', value = %d\n", search, value_u4);
        } else {
          printf("\n\tCustom (case-insensitive) - '%s' not found\n", search );
        }

        mco_db_disconnect(db);
      }

      free(coll_map);
    }
      
    sample_close_database(db_name, &dbmem);
  }

  mco_runtime_stop();

  sample_pause_end("\n\nPress any key to continue . . . ");

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

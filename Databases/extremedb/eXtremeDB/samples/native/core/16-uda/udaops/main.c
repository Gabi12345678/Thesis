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

#include "udaops.h"

char sample_descr[] = {
  "Sample 'udaops' demonstrates the UDA API in a variety of database operations.\n" 
};
const char* db_name = "udaops";
const unsigned int n_records = 5;
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
  unsigned short j;
  mco_uda_object_handle_t rec, inner;
  unsigned short rec_struct_no, hu4_index_no;
  mco_uda_value_t value;
  char buf[1024];
  udaops_oid oid;
  unsigned short length;
  sample_memory_t dbmem;
  char msg[128];
    
  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();
   
  init_metadict();
  mco_metadict_register(metadict, db_name, udaops_get_dictionary(), 0);

  /* Open and connect to database */
  rc = sample_open_database( db_name, udaops_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, MAX_DB_CONNECTIONS, &dbmem );
  if ( MCO_S_OK == rc ) {
  
    rc = mco_db_connect(db_name, &db);
    if ( MCO_S_OK == rc ) {
    
      /* Get struct and index id numbers to avoid dictionary lookup overhead in loops */
      rec_struct_no = get_struct_no("Record");
      hu4_index_no = get_index_no("Record", "hu4");

      /* Populate the database */
      printf("\n\tInsert records...\n");
      for (i = 0; i < n_records && MCO_S_OK == rc; ++i) {
        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {
          /* Create a Record object (with oid) */
          oid.id = i;
          mco_uda_new(t, rec_struct_no, &oid, 0, 0, &rec);
          printf("\n\tCreate new Record with OID %d                ", oid.id );

          /* Put a simple value */
          value.type = MCO_DD_UINT4; /* Set field's type */
          value.v.u4   = i * 2;      /* Set appropriate union field */
          mco_uda_put(&rec, get_field_no(&rec, "u4"), 0, &value);
          printf("\n\tPut Record.u4                  = %d          ", value.v.u4 );

          /* Put a BLOB value */
          value.type = MCO_DD_BLOB; /* Set field's type */ 
          value.v.p.p.v = buf;      /* Set pointer to blob data */  
          value.v.p.len = 128;      /* Set blob's length */
          mco_uda_put(&rec, get_field_no(&rec, "b"), 0, &value);
          printf("\n\tPut Record.b value                           " );

          /* Append value to BLOB */
          mco_uda_blob_append(&rec, get_field_no(&rec, "b"), buf, 256);
          printf("\n\tAppend Record.b value                        " );

          /* Put a vector of chars */
          mco_uda_field_alloc(&rec, get_field_no(&rec, "vch"), (uint2)vLength); /* Allocate vector */
          value.type = MCO_DD_CHAR;  /* Set field's type */ 
          value.v.p.p.c = buf;       /* Set pointer to chars */
          value.v.p.len = 20;        /* Set string length */
          for (j = 0; j < vLength; ++j) {
            sprintf(buf, "vch-%d:%d", i,j); /* Put value to buffer */
            mco_uda_put(&rec, get_field_no(&rec, "vch"), j, &value); /* Put value to object */
            printf("\n\tPut Record.vch[%d]              = '%s'  ", j, buf );
          }

          /* Put a STRUCT value and put a simple value to the inner struct */
          value.type = MCO_DD_STRUCT; /* Set struct type */
          mco_uda_put(&rec, get_field_no(&rec, "inner"), 0, &value); /* Initialize inner struct handler */
          inner = value.v.o; /* Get inner struct handler */
          value.type = MCO_DD_UINT4;
          value.v.u4   = i * 2;
          mco_uda_put(&inner, get_field_no(&inner, "inner_data"), 0, &value);
          printf("\n\tPut Record.inner.inner_data    = %d          ", value.v.u4 );

          /* Put a vector of strings to the inner struct */
          mco_uda_field_alloc(&inner, get_field_no(&inner, "inner_vstr"), (uint2)vLength); /* allocate vector */
          value.type = MCO_DD_STRING;
          value.v.p.p.c = buf;
          value.v.p.len = 128;
          for (j = 0; j < vLength; ++j) {
            sprintf(buf, "vstr-%d:%d", i,j);
            mco_uda_put(&inner, get_field_no(&inner, "inner_vstr"), j, &value);
            printf("\n\tPut Record.inner.inner_vstr[%d] = '%s' ", j, buf );
          }        


          /* Boolean fields */
          value.type = MCO_DD_BOOL; /* Set field's type */
          value.v.u1   = i % 2;      /* Set appropriate union field */
          mco_uda_put(&rec, get_field_no(&rec, "bit"), 0, &value);
          printf("\n\tPut Record.bit                 = %d          ", value.v.u1);

          value.type = MCO_DD_BOOL; /* Set field's type */
          value.v.u1   = i % 2;      /* Set appropriate union field */
          mco_uda_put(&rec, get_field_no(&rec, "ev_bit"), 0, &value);
          printf("\n\tPut Record.ev_bit              = %d          ", value.v.u1);

          mco_uda_field_alloc(&rec, get_field_no(&rec, "vbit"), (uint2)vLength); /* Allocate vector */
          value.type = MCO_DD_BOOL; /* Set field's type */
          for (j = 0; j < vLength; ++j) {
            value.v.u1   = (i + j) % 2;      /* Set appropriate union field */
            mco_uda_put(&rec, get_field_no(&rec, "vbit"), j, &value);
            printf("\n\tPut Record.vbit[%d]             = %d ", j, value.v.u1 );
          }        

          mco_uda_length(&rec, get_field_no(&rec, "abit"), &length);
          value.type = MCO_DD_BOOL; /* Set field's type */
          for (j = 0; j < length; ++j) {
            value.v.u1   = (i + j) % 2;      /* Set appropriate union field */
            mco_uda_put(&rec, get_field_no(&rec, "abit"), j, &value);
            printf("\n\tPut Record.abit[%d]             = %d ", j, value.v.u1 );
          }        

          rc = mco_trans_commit(t);
        }
      }

      /* Sequentially read and output records */
      printf("\n\n\r\tSequentially Read records...\n");
      rc = mco_trans_start(db, MCO_READ_ONLY, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        /* Open cursor */
        rc = mco_uda_cursor(t, get_struct_no("Record"), get_index_no("Record", "tu4"), &csr);
        if ( MCO_S_OK == rc ) {
          printf("\n\tGet 'tu4' index cursor...");
          for (rc = mco_cursor_first(t, &csr); rc == MCO_S_OK; rc = mco_cursor_next(t, &csr)) {
            rc = mco_uda_from_cursor(t, &csr, &rec); /* Get object handle from cursor */
            sample_rc_check("\n\tGet object from cursor", rc );
            
            /* Get OID value */
            value.type     = MCO_DD_OID;
            value.v.p.p.v    = &oid;
            value.v.p.size = sizeof(oid);
            mco_uda_get(&rec, get_field_no(&rec, "oid"), 0, &value);
            printf("\n\tGet OID                        : %d", oid.id);
            
            /* Get AUTOOID value */
            value.type     = MCO_DD_AUTOOID;
            mco_uda_get(&rec, get_field_no(&rec, "auto_oid@"), 0, &value);
            printf("\n\tGet AUTOOID                    : %llx", value.v.u8);

            /* Get simple value */
            value.type = MCO_DD_UINT4;
            rc = mco_uda_get(&rec, get_field_no(&rec, "u4"), 0, &value);
            printf("\n\tGet Record.u4                  : %d", value.v.u4);

            /* Get BLOB value */
            value.type = MCO_DD_BLOB; /* Set field's type */ 
            value.v.p.p.v = buf;      /* Set pointer to buffer */  
            value.v.p.size = sizeof(buf); /* Set buffer size */
            mco_uda_get(&rec, get_field_no(&rec, "b"), 0, &value);
            printf("\n\tGet Record.b value             : length %d", value.v.p.len);

            /* Get vector of chars */
            mco_uda_length(&rec, get_field_no(&rec, "vch"), &length); /* Get length of vector */
            printf("\n\tGet Record.vch vector length   : %d", length);

            value.type = MCO_DD_CHAR;     /* Set field's type */ 
            value.v.p.p.c = buf;          /* Set pointer to buffer */
            value.v.p.size = sizeof(buf); /* Set buffer size */
            for (j = 0; j < length; ++j) {
              mco_uda_get(&rec, get_field_no(&rec, "vch"), j, &value); /* Put value to object */
              printf("\n\tGet Record.vch[%d]              : '%s', length %d", j, buf, value.v.p.len);
            }

            /* Get STRUCT value and get simple value from the inner struct */
            value.type = MCO_DD_STRUCT; /* Set struct type */
            mco_uda_get(&rec, get_field_no(&rec, "inner"), 0, &value); /* Initialize inner struct handler */
            inner = value.v.o; /* Get inner struct handler */
            value.type = MCO_DD_UINT4;
            mco_uda_get(&inner, get_field_no(&inner, "inner_data"), 0, &value);
            printf("\n\tGet Record.inner.inner_data    : %d", value.v.u4);

            /* Get vector of string to inner struct */
            mco_uda_length(&inner, get_field_no(&inner, "inner_vstr"), &length); /* Get length of vector */
            value.type = MCO_DD_STRING;
            value.v.p.p.c = buf;
            value.v.p.size = sizeof(buf);
            for (j = 0; j < length; ++j) {
              mco_uda_get(&inner, get_field_no(&inner, "inner_vstr"), j, &value);
              printf("\n\tGet Record.inner.inner_vstr[%d] : '%s', length %d", j, buf, value.v.p.len);
            }
            
            value.type = MCO_DD_BOOL;
            mco_uda_get(&rec, get_field_no(&rec, "bit"), 0, &value);
            printf("\n\tGet Record.bit                 : %d", value.v.u1);
            
            value.type = MCO_DD_BOOL;
            mco_uda_get(&rec, get_field_no(&rec, "ev_bit"), 0, &value);
            printf("\n\tGet Record.ev_bit              : %d", value.v.u1);
            
            
            mco_uda_length(&rec, get_field_no(&rec, "abit"), &length);
            value.type = MCO_DD_BOOL; /* Set field's type */
            for (j = 0; j < length; ++j) {
              mco_uda_get(&rec, get_field_no(&rec, "abit"), j, &value);
              printf("\n\tGet Record.abit[%d]             : %d ", j, value.v.u1 );
            }        

            mco_uda_length(&rec, get_field_no(&rec, "vbit"), &length);
            value.type = MCO_DD_BOOL; /* Set field's type */
            for (j = 0; j < length; ++j) {
              mco_uda_get(&rec, get_field_no(&rec, "vbit"), j, &value);
              printf("\n\tGet Record.vbit[%d]             : %d ", j, value.v.u1 );
            }        
            
            
          }
        }
        mco_trans_commit(t);
      }
    
      /* Find & delete all but the last 2 records  */
      printf("\n\n\tDelete all but 2 records...");
      for (i = 0; i + 2 < n_records; ++i) {
        mco_uda_value_t keys[1];

        rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
        if ( MCO_S_OK == rc ) {

          /* Open cursor */
          rc = mco_uda_cursor(t, rec_struct_no, hu4_index_no, &csr);
          if ( MCO_S_OK == rc ) {

            /* Set keys type and value */
            keys[0].type = MCO_DD_UINT4;
            keys[0].v.u4 = i * 2;

            rc = mco_uda_lookup(t, get_struct_no("Record"), get_index_no("Record", "hu4"), MCO_EQ, keys, 1, &csr);
            sprintf(msg, "\n\tFind Record by u4 = %d         ", keys[0].v.u4 );
            sample_rc_check(msg, rc );

            /* Get object from cursor */
            mco_uda_from_cursor(t, &csr, &rec);

            /* Delete object */
            mco_uda_delete(&rec);
            printf("\n\tDelete Record with u4 = %d     ", keys[0].v.u4 );
            
            rc = mco_trans_commit(t);
            sample_rc_check("\tCommit transaction            ", rc );
          }
        }
      }

      /* Call delete_all to remove the remaining records */
      printf("\n\n\tDelete remaining 2 records...");
      rc = mco_trans_start(db, MCO_READ_WRITE, MCO_TRANS_FOREGROUND, &t);
      if ( MCO_S_OK == rc ) {
        rc = mco_uda_delete_all(t, get_struct_no("Record"));
        rc = mco_trans_commit(t);
        sample_rc_check("\tCommit transaction            ", rc );
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

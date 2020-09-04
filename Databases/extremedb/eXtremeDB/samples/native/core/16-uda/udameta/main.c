/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
/* This sample demonstrates how to work with meta-dictionary - 
 * part of Uniform Database Access (UDA) interface.
 * The sample loads several schemas and prints them out.  
 */
#include <common.h>
#include <mcouda.h>

#include "uda1.h"
#include "uda2.h"
#include "uda3.h"
#include "uda4.h"
#include "uda5.h"

#define METADICT_ENTRY_NUM    20
char sample_descr[] = {
  "Sample 'udameta' demonstrates use of the UDA Metadata API.\n" 
};

/* Print field type */
char *dump_field_type(mco_metadict_header_t *metadict, unsigned short dict_no, mco_dict_field_info_t *field_info)
{
  switch (field_info->type) {
    case MCO_DD_UINT1: return "UINT1";
    case MCO_DD_UINT2: return "UINT2";
    case MCO_DD_UINT4: return "UINT4";
    case MCO_DD_UINT8: return "UINT8";
    case MCO_DD_INT1:  return "INT1"; 
    case MCO_DD_INT2:  return "INT2";
    case MCO_DD_INT4:  return "INT4";
    case MCO_DD_INT8:  return "INT8";
    case MCO_DD_FLOAT: return "FLOAT";
    case MCO_DD_DOUBLE: return "DOUBLE";
    case MCO_DD_DATE:  return "DATE";
    case MCO_DD_TIME:  return "TIME";
    case MCO_DD_CHAR:  return "CHAR";
    case MCO_DD_NCHAR_CHAR: return "NCHAR";
    case MCO_DD_WIDE_CHAR:  return "WCHAR";
    case MCO_DD_STRING: return "STRING";
    case MCO_DD_NCHAR_STRING: return "NSTRING";
    case MCO_DD_WCHAR_STRING: return "WSTRING";
    case MCO_DD_REF: return "REF";
    case MCO_DD_AUTOID: return "AUTOID";
    case MCO_DD_AUTOOID: return "AUTOOID";
    case MCO_DD_STRUCT: {
      mco_dict_struct_info_t struct_info;
      mco_dict_struct(metadict, dict_no, field_info->struct_no, &struct_info);
      return struct_info.name;
    }
    case MCO_DD_BLOB: return "BLOB";
    case MCO_DD_OID: return "OID";
    case MCO_DD_BOOL: return "BOOLEAN";
  }
  return "<UNKNOWN>";
}

/* Print meta-information about index fields of <index_no> in class <struct_no> */
void dump_index_fields(mco_metadict_header_t *metadict, unsigned short dict_no, unsigned short struct_no, unsigned short index_no, unsigned short ifield_count)
{
  MCO_RET rc;
  unsigned short ifield_no;
  
  for (ifield_no = 0; ifield_no < ifield_count; ++ifield_no) {
    mco_dict_ifield_info_t ifield_info;
    mco_dict_field_info_t field_info;
    /* Get index field info by ifield_no and print it out */
    rc = mco_dict_ifield(metadict, dict_no, struct_no, index_no, ifield_no, &ifield_info);
    if ( MCO_S_OK == rc ) {
      mco_dict_field(metadict, dict_no, struct_no, ifield_info.field_no, &field_info);
      printf("            IField : No=%u, FieldNo=%u (%s, %s), Flags=", ifield_info.ifield_no, ifield_info.field_no,
              field_info.name, dump_field_type(metadict, dict_no, &field_info));
      if (ifield_info.flags & MCO_DICT_IFI_DESCENDING) printf("DESC ");
      if (ifield_info.flags & MCO_DICT_IFI_NULLABLE) printf("NULLABLE ");
      printf("\n");
    } else {
      printf("dump_index_fields() : Can't get ifield #%d for index #%d, error %s\n", ifield_no, index_no, mco_ret_string(rc, 0));
    }
  }
}

/* Print meta-information about indexes of class <struct_no> */
void dump_indexes(mco_metadict_header_t *metadict, unsigned short dict_no, unsigned short struct_no, unsigned short index_count)
{
  MCO_RET rc;
  unsigned short index_no;
  
  for (index_no = 0; index_no < index_count; ++index_no) {
    mco_dict_index_info_t index_info;
    /* Get index info by index_no and print it out */
    rc = mco_dict_index(metadict, dict_no, struct_no, index_no, &index_info);
    if ( MCO_S_OK == rc ) {
      printf("        Index %-10s : No=%u, Nfields=%u, Flags=", index_info.name, index_info.index_no, index_info.n_fields);
      if (index_info.flags & MCO_DICT_II_UNIQUE) printf("UNIQUE ");
      if (index_info.flags & MCO_DICT_II_VOLUNTARY) printf("VOLUNTARY ");
      if (index_info.flags & MCO_DICT_II_LIST) printf("LIST ");
      if (index_info.flags & MCO_DICT_II_AUTOID) printf("AUTOID ");
      if (index_info.flags & MCO_DICT_II_TREE) printf("TREE ");
      if (index_info.flags & MCO_DICT_II_HASH) printf("HASH ");
      if (index_info.flags & MCO_DICT_II_USERDEF) printf("USERDEF ");
      if (index_info.flags & MCO_DICT_II_NULLABLE) printf("NULLABLE ");
      printf("\n");
      dump_index_fields(metadict, dict_no, struct_no, index_no, index_info.n_fields);
    } else {
      printf("dump_index() : Can't get index #%d, error %s\n", index_no, mco_ret_string(rc, 0));
    }
  }
}


/* Print meta-information about fields of class <struct_no> */
void dump_fields(mco_metadict_header_t *metadict, unsigned short dict_no, unsigned short struct_no, unsigned short field_count)
{
  MCO_RET rc;
  unsigned short field_no;
  
  for (field_no = 0; field_no < field_count; ++field_no) {
    mco_dict_field_info_t field_info;
    /* Get field info by field_no and print it out */
    rc = mco_dict_field(metadict, dict_no, struct_no, field_no, &field_info);
    if ( MCO_S_OK == rc ) {
      printf("        Field %-10s : No=%u, Type=%s, Dim=%u, Flags=", field_info.name, field_info.field_no, dump_field_type(metadict, dict_no, &field_info), field_info.dimension);
      if (field_info.flags & MCO_DICT_FI_OPTIONAL) printf("OPTIONAL ");
      if (field_info.flags & MCO_DICT_FI_INDEXED) printf("INDEXED ");
      if (field_info.flags & MCO_DICT_FI_VECTOR) printf("VECTOR ");
      if (field_info.flags & MCO_DICT_FI_ARRAY) printf("ARRAY ");
      if (field_info.flags & MCO_DICT_FI_NULLABLE) printf("NULLABLE ");
      if (field_info.flags & MCO_DICT_FI_NULL_INDICATOR) printf("NULL_INDICATOR ");
      if (field_info.flags & MCO_DICT_FI_EVENT_UPD) printf("EV_UPDATE (%d) ", field_info.update_event_no);
      printf("\n");
    } else {
      printf("dump_fields() : Can't get field #%d, error %s\n", field_no, mco_ret_string(rc, 0));
    }
  }
}

/* Print meta-information about events of class <struct_no> */
void dump_events(mco_metadict_header_t *metadict, unsigned short dict_no, unsigned short struct_no, unsigned short event_count)
{
    unsigned short event_no;
    MCO_RET rc;
    
    for (event_no = 0; event_no < event_count; ++event_no) {
        mco_dict_event_info_t event_info;
        mco_dict_field_info_t field_info;

        rc = mco_dict_event(metadict, dict_no, struct_no, event_no, &event_info);
        if (rc == MCO_S_OK) {
            if (event_info.type == MCO_EVENT_UPDATE) {
                rc = mco_dict_field(metadict, dict_no, struct_no, event_info.field_no, &field_info);
                if (rc == MCO_S_OK) {
                    printf("        Event : No=%u, Type=UPDATE, FieldNo=%u (%s %s)", event_info.event_no, event_info.field_no,
                            field_info.name, dump_field_type(metadict, dict_no, &field_info));
                }
            } else {
                printf("        Event : No=%u, Type=", event_info.event_no);
                switch (event_info.type) {
                    case MCO_EVENT_NEW          : printf("NEW"); break;
                    case MCO_EVENT_DELETE       : printf("DELETE"); break;
                    case MCO_EVENT_DELETE_ALL   : printf("DELETE_ALL"); break;
                    case MCO_EVENT_CHECKPOINT   : printf("CHECKPOINT"); break;
                    case MCO_EVENT_CLASS_UPDATE : printf("CLASS_UPDATE"); break;
                    default                     : break;
                }
            }
        }
        printf("\n");
    }
}


/* Print meta-information dictionary */
void dump_dict(mco_metadict_header_t *metadict, unsigned short dict_no)
{
  MCO_RET rc;
  unsigned short struct_count, struct_no;

  /* Get number of structs in the dictionary */
  rc = mco_dict_struct_count(metadict, dict_no, &struct_count);
  if ( MCO_S_OK == rc ) {
    
    for (struct_no = 0; struct_no < struct_count; ++struct_no) {
      mco_dict_struct_info_t struct_info;
      /* Get struct info by struct_no and print it out */
      rc = mco_dict_struct(metadict, dict_no, struct_no, &struct_info);
      if ( MCO_S_OK == rc ) {
        printf("    Struct %-10s : No=%u, Nindex=%u, Nfield=%d, Nevent=%d, Class_code=%d, Flags=", 
            struct_info.name, struct_info.struct_no, struct_info.index_count, struct_info.field_count, 
            struct_info.event_count, struct_info.class_code);

        if (struct_info.flags & MCO_DICT_SI_CLASS) printf("CLASS ");
        if (struct_info.flags & MCO_DICT_SI_OID) printf("OID ");
        if (struct_info.flags & MCO_DICT_SI_AUTOID) printf("AUTOID ");
        if (struct_info.flags & MCO_DICT_SI_LIST) printf("LIST ");
        if (struct_info.flags & MCO_DICT_SI_PERSISTENT) printf("PERSIST ");
        if (struct_info.flags & MCO_DICT_SI_EVENT_NEW) printf("EV_NEW (%d) ", struct_info.new_event_no);
        if (struct_info.flags & MCO_DICT_SI_EVENT_DEL) printf("EV_DEL (%d) ", struct_info.delete_event_no);
        if (struct_info.flags & MCO_DICT_SI_EVENT_UPD) printf("EV_UPD ");
        if (struct_info.flags & MCO_DICT_SI_EVENT_DELALL) printf("EV_DELALL (%d) ", struct_info.delete_all_event_no);
        if (struct_info.flags & MCO_DICT_SI_EVENT_CHECKPOINT) printf("EV_CHECKPOINT (%d) ", struct_info.checkpoint_event_no);
        if (struct_info.flags & MCO_DICT_SI_EVENT_CLASS_UPDATE) printf("EV_CLASSUPDATE (%d) ", struct_info.class_update_event_no);
        printf("\n");
        /* Prints fields and indexes */
        dump_fields(metadict, dict_no, struct_no, struct_info.field_count);
        dump_indexes(metadict, dict_no, struct_no, struct_info.index_count);
        dump_events(metadict, dict_no, struct_no, struct_info.event_count);
        printf("    -----------------------------------------------\n");
      }
    }
  }
}

void dump_metadict(mco_metadict_header_t *metadict)
{
  unsigned short count;
  unsigned short dict_no;
  char msg[128];

  /* Get number of registered dictionaries and print out info for each */
  mco_metadict_count(metadict, &count);        
  for (dict_no = 0; dict_no < count; ++dict_no) {
    MCO_RET rc;
    mco_metadict_entry_t *entry;

    /* Get dictionary by number and check if it is registered */
    rc = mco_metadict_entry(metadict, dict_no, &entry);
    if (rc == MCO_E_UDA_DICT_NOTFOUND)
        continue;

    printf("###############################################\n");

    printf("Entry info : No=%u, Flags=%u, Name=%s\n", entry->dict_no, entry->flags, entry->name);
    dump_dict(metadict, dict_no);
    if ( dict_no == count - 1 )
    sprintf(msg, "    End of meta-data for the last dictionary #%d.\n\nPress any key to continue . . . ", dict_no );
    else
    sprintf(msg, "    End of meta-data for dictionary #%d.\n\nPress any key to continue . . . ", dict_no );
    sample_pause(msg);
  }
}

int main(int argc, char* argv[])
{
  MCO_RET rc;
  mco_metadict_header_t *metadict;  /* Meta-dictionary header. Application must allocate memory for it. */
  unsigned int size;
  sample_memory_t dbmem1, dbmem2;

  sample_os_initialize(DEFAULT);

  sample_header(sample_descr);

  /* Set fatal error handler and start eXtremeDB runtime */
  mco_error_set_handler(&sample_errhandler);
  mco_runtime_start();

  /* Create uda1 and uda2 databases */
  rc = sample_open_database( "uda1", uda1_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                             MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, MAX_DB_CONNECTIONS, &dbmem1 );
  if ( MCO_S_OK == rc ) {
  
    rc = sample_open_database( "uda2", uda2_get_dictionary(), DATABASE_SIZE, CACHE_SIZE, 
                               MEMORY_PAGE_SIZE, PSTORAGE_PAGE_SIZE, MAX_DB_CONNECTIONS, &dbmem2 );
    if ( MCO_S_OK == rc ) {

      /* Determine size of meta-dictionary (in bytes) and allocate memory */
      mco_metadict_size(METADICT_ENTRY_NUM, &size);
      metadict = (mco_metadict_header_t *) (malloc(size));
    
      /* Initialize meta-dictionary. Opened databases ("uda1" and "uda2") will be automatically added to meta-dictionary 
       * Passing MCO_METADICT_DONT_LOAD_EXISTING_DBS flag as last parameter prevents this behavior*/
      rc = mco_metadict_init (metadict, size, 0);
      if ( MCO_S_OK == rc ) {
        mco_db_h db;

        /* Register other dictionaries manually, without opening the database */
        CHECK(mco_metadict_register(metadict, "uda3", uda3_get_dictionary(), 0));
        CHECK(mco_metadict_register(metadict, "uda4", uda4_get_dictionary(), 0));
        CHECK(mco_metadict_register(metadict, "uda5", uda5_get_dictionary(), 0));
          
        printf("Print the complete meta-dictionary\n");
        dump_metadict(metadict);

        /* Un-register some dictionaries */
        CHECK(mco_metadict_unregister(metadict, "uda5"));
        CHECK(mco_metadict_unregister(metadict, "uda1"));

        printf("\n\nPrint reduced meta-dictionary\n");
        dump_metadict(metadict);

        /* Register dictionaries back again. Try function mco_metadict_register_conn */
        CHECK(mco_db_connect("uda1", &db));
        CHECK(mco_metadict_register_conn(metadict, db, 0));
        CHECK(mco_metadict_register(metadict, "uda5", uda5_get_dictionary(), 0));

        printf("\n\nPrint the complete meta-dictionary again\n");
        dump_metadict(metadict);

        CHECK(mco_db_disconnect(db));
        sample_close_database("uda2", &dbmem2);
      }
    }
    sample_close_database("uda1", &dbmem1);
  }

  mco_runtime_stop();

  sample_os_shutdown();
  return ( MCO_S_OK == rc ? 0 : 1 );
}

/*******************************************************************
 *                                                                 *
 *  mcouda.h                                                    *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           * 
 *  All Rights Reserved                                            *
 *                                                                 *
 *  eXtremeDB runtime uniform data access API                      * 
 *                                                                 *
 *******************************************************************/
#ifndef MCO_UNIFORM_DATA_ACCESS_API_H
#define MCO_UNIFORM_DATA_ACCESS_API_H

#ifdef __cplusplus
    extern "C"
    {
#endif 


#include <mco.h>
#ifdef MCO_CFG_WCHAR_SUPPORT
    #include <stdarg.h>
    #ifndef _VXWORKS
        #include <ctype.h>
        #include <wctype.h>
        #ifndef _WIN32
	   #if defined(_QNX) && defined(__cplusplus)
		#undef __cplusplus
		#include <wchar.h>
		#define __cplusplus
	   #else
		#include <wchar.h>
	   #endif
        #endif
    #else 
        #include <stdlib.h>
    #endif 
#endif


#ifndef MCOUDA_API
# ifdef _WIN32
#  ifdef MCOUDA_DLL
#    define MCOUDA_API __declspec(dllexport)
#  else
#    ifdef MCOUDA_USE_DLL
#       define MCOUDA_API __declspec(dllimport)
#    else
#       define MCOUDA_API 
#    endif
#  endif
# else
#  define MCOUDA_API
# endif
#endif

/* Dictionary access API */
typedef struct tag_mco_metadict_entry_t_ {
    unsigned short dict_no;      /* dictionary index */
    unsigned short flags;        /* internal entry flags */
    char           name[16 + 2]; /* MCO_CFG_DB_NAME_MAXSIZE in mcocfg.h */
    void         * dict;         /* pointer to the dictionary */
    void         * user_data;    /* user-specified context */
} mco_metadict_entry_t;

typedef struct tag_mco_metadict_header_t_ {
    unsigned short n_maxentries; /* max number of entries allocated */
    unsigned short n_allocated;  /* number of used entries */
    mco_metadict_entry_t entries[1]; /* meta-dictionary entries */
} mco_metadict_header_t;

/* Get size of memory region enought to fit requires number of entries.
 * In case of statical memory allocation please ensure in properly memory alignment:
 * 
 * char array[(sizeof(mco_metadict_header_t)+sizeof(mco_metadict_entry_t)*n_entries+3) & ~(3)];
 */
MCOUDA_API void         mco_metadict_size       ( unsigned short n_entries, /* out */ unsigned int * size );

/* Initialize the metadict and load initial state from eXdb registry. This function can load dictionaries of existing DBMS
 * if only database is placed in conventional memory. Else if shared memory is used a dictionary should be registered manually
 * by functions mco_metadict_register and mco_metadict_register_conn
 */
#define MCO_METADICT_DONT_LOAD_EXISTING_DBS 	1
MCOUDA_API MCO_RET      mco_metadict_init       ( mco_metadict_header_t * metadict, unsigned int size, unsigned int flags );

/* Put a dict. into the metadict. Useless if DDL operation in database expected because dict is a static dictionary */
MCOUDA_API MCO_RET      mco_metadict_register   ( mco_metadict_header_t * metadict, const char * name, const void * dict, const void * user_data );

/* Put a dictionary referenced by current DB connection into the metadict. Especially useful in case of DDL operation 
 * or then database is placed in shared memory
 */
MCOUDA_API MCO_RET      mco_metadict_register_conn   ( mco_metadict_header_t * metadict, const mco_db_h connection, const void * user_data );

/* Remove a dictionary referenced by name from the metadict. */
MCOUDA_API MCO_RET      mco_metadict_unregister   ( mco_metadict_header_t * metadict, const char * name );

/* Get number of records in the metadict. */
MCOUDA_API MCO_RET      mco_metadict_count      ( const mco_metadict_header_t * metadict, 
                                                                  /* out */ unsigned short * count );

/* Get a record from the metadict. by index */
MCOUDA_API MCO_RET      mco_metadict_entry      ( const mco_metadict_header_t * metadict, unsigned short dict_no, 
                                                                  /* out */ mco_metadict_entry_t ** entry );
/* Get a record from the metadict. by name */
MCOUDA_API MCO_RET      mco_metadict_entry_name ( const mco_metadict_header_t * metadict, const char * name,
                                                                  /* out */ mco_metadict_entry_t ** entry );

/* Get a record from the metadict. by connection */
MCOUDA_API MCO_RET      mco_metadict_entry_conn ( const mco_metadict_header_t * metadict, const mco_db_h connection,
                                                                  /* out */ mco_metadict_entry_t ** entry );
typedef enum tag_mco_dict_type_t_ {
    MCO_DD_UINT1,
    MCO_DD_UINT2,
    MCO_DD_UINT4,
    MCO_DD_UINT8,
    MCO_DD_INT1,
    MCO_DD_INT2,
    MCO_DD_INT4,
    MCO_DD_INT8,
    MCO_DD_FLOAT,
    MCO_DD_DOUBLE,
    MCO_DD_DATE,
    MCO_DD_TIME,
    MCO_DD_DATETIME,
    MCO_DD_CHAR,
    MCO_DD_NCHAR_CHAR,
    MCO_DD_WIDE_CHAR,
    MCO_DD_STRING,
    MCO_DD_NCHAR_STRING,
    MCO_DD_WCHAR_STRING,
    MCO_DD_REF,
    MCO_DD_AUTOID,
    MCO_DD_AUTOOID,
    MCO_DD_STRUCT,
    MCO_DD_BLOB,
    MCO_DD_OID,
    MCO_DD_BOOL,
    MCO_DD_SEQUENCE_UINT1,
    MCO_DD_SEQUENCE_UINT2,
    MCO_DD_SEQUENCE_UINT4,
    MCO_DD_SEQUENCE_UINT8,
    MCO_DD_SEQUENCE_INT1,
    MCO_DD_SEQUENCE_INT2,
    MCO_DD_SEQUENCE_INT4,
    MCO_DD_SEQUENCE_INT8,
    MCO_DD_SEQUENCE_FLOAT,
    MCO_DD_SEQUENCE_DOUBLE,
    MCO_DD_SEQUENCE_CHAR,
    MCO_DD_SEQUENCE_DATETIME,
    MCO_DD_BINARY,
    MCO_DD_VARBINARY
} mco_dict_type_t;

/* structure flags */
#define MCO_DICT_SI_CLASS                 0x1  /* is a class structure */
#define MCO_DICT_SI_OID                   0x2  /* has OID */
#define MCO_DICT_SI_AUTOID                0x4  /* has AutoID */
#define MCO_DICT_SI_LIST                  0x8  /* has list index */
#define MCO_DICT_SI_PERSISTENT           0x10  /* is persistent */
#define MCO_DICT_SI_EVENT_NEW            0x20  /* has new event handler */
#define MCO_DICT_SI_EVENT_DEL            0x40  /* has delete event handler */
#define MCO_DICT_SI_EVENT_UPD            0x80  /* has update event handler for some fields */
#define MCO_DICT_SI_EVENT_DELALL        0x100  /* has delete-all event handler */
#define MCO_DICT_SI_EVENT_CHECKPOINT    0x200  /* has event handler for checkpoint */
#define MCO_DICT_SI_EVENT_CLASS_UPDATE  0x400  /* has update event hamdler for any of the fields */
#define MCO_DICT_SI_COMPACT             0x800  /* is compact */
#define MCO_DICT_SI_PACKED              0x1000 /* is packed */
#define MCO_DICT_SI_DIRECT              0x2000 /* is direct struct */

typedef struct tag_mco_dict_struct_info_t_ {
    unsigned short  struct_no;              /* index of the struct (not dictionary index) */
    char          * name;                   /* name of the struct */
    unsigned int    flags;                  /* flags, see above */
    unsigned short  index_count;            /* number of indexes for the struct */
    unsigned short  field_count;            /* number of fields in the struct */
    unsigned short  event_count;            /* number of events for the struct */
    unsigned short  class_code;             /* dictionary's class code (for classes) */
    unsigned short  new_event_no;           /* index of new event descriptor */
    unsigned short  delete_event_no;        /* index of delete event descriptor */
    unsigned short  delete_all_event_no;    /* index of delete-all event descriptor */
    unsigned short  checkpoint_event_no;    /* index of checkpoint event descriptor */ 
    unsigned short  class_update_event_no;  /* index of class-update event descriptor */

} mco_dict_struct_info_t, * mco_dict_struct_info_p;

/* Number of classes and structs in the dict. */
MCOUDA_API MCO_RET mco_dict_struct_count( const mco_metadict_header_t * metadict, unsigned short dict_no, 
                                                        /* out */ unsigned short * count );
MCOUDA_API MCO_RET mco_dict_struct      ( const mco_metadict_header_t * metadict, unsigned short dict_no, 
                               unsigned short struct_no, 
                                                        /* out */ mco_dict_struct_info_p struct_info );
MCOUDA_API MCO_RET mco_dict_struct_name ( const mco_metadict_header_t * metadict, unsigned short dict_no, 
                               const char * name, 
                                                        /* out */ mco_dict_struct_info_p struct_info );

MCOUDA_API MCO_RET mco_dict_struct_classcode( const mco_metadict_header_t * metadict, unsigned short dict_no,
                                   unsigned short class_code,
                                                            /* out */ mco_dict_struct_info_p struct_info );

/* field flags */
#define MCO_DICT_FI_OPTIONAL             0x1  /* the field is optional */
#define MCO_DICT_FI_INDEXED              0x2  /* the field is indexed */ 
#define MCO_DICT_FI_VECTOR               0x4  /* the field is a vector */
#define MCO_DICT_FI_ARRAY                0x8  /* the field is an array */
#define MCO_DICT_FI_EVENT_UPD       	0x10  /* the field has update event handler */
#define MCO_DICT_FI_HAS_INIT_DATA	    0x20  /* the field has initializator */
#define MCO_DICT_FI_NULLABLE            0x40  /* the field is nullable  */
#define MCO_DICT_FI_NULL_INDICATOR      0x80  /* the field is null indicator */
#define MCO_DICT_FI_ASC_SEQUENCE       0x100  /* the field is an ascending sequence */
#define MCO_DICT_FI_DESC_SEQUENCE      0x200  /* the field is a descending sequence */
#define MCO_DICT_FI_SEQUENCE           0x400  /* the field is a sequence */

typedef struct tag_mco_dict_field_info_t_ {
    unsigned short     field_no;        /* index of the field descriptor (not dictionary index) */
    char             * name;            /* name of the field */
    mco_dict_type_t    type;            /* type of the field */
    unsigned int       flags;           /* flags, see above */
    unsigned short     dimension;       /* dimension for arrays */
    unsigned short     struct_no;       /* used if field is nested struct/optional struct */
    unsigned short     update_event_no; /* index of assiciated update event descriptor */
    unsigned short     precision;       /* for numeric fields */
    unsigned int       size;            /* size of the field */
} mco_dict_field_info_t, * mco_dict_field_info_p;

MCOUDA_API MCO_RET mco_dict_field       ( const mco_metadict_header_t * metadict, unsigned short dict_no,
                               unsigned short struct_no, unsigned short field_no,
                                                        /* out */ mco_dict_field_info_p field_info );
MCOUDA_API MCO_RET mco_dict_field_name  ( const mco_metadict_header_t * metadict, unsigned short dict_no, 
                               unsigned short struct_no, const char * name, 
                                                        /* out */ mco_dict_field_info_p field_info );

/* index flags */
#define MCO_DICT_II_UNIQUE     1     /* index is unique */
#define MCO_DICT_II_VOLUNTARY  2     /* index is voluntary */
#define MCO_DICT_II_LIST       4     /* list-index */
#define MCO_DICT_II_AUTOID     8     /* autoid index */
#define MCO_DICT_II_TREE       0x10  /* b-trie index */
#define MCO_DICT_II_HASH       0x20  /* hash index */
#define MCO_DICT_II_USERDEF    0x40  /* user-defined comparision routines for the index */
#define MCO_DICT_II_KDTREE     0x80  /* kd-trie index */
#define MCO_DICT_II_RTREE      0x100 /* r-trie index */
#define MCO_DICT_II_PATRICIA   0x200 /* patricia index */
#define MCO_DICT_II_NULLABLE   0x400 /* nullable index */
#define MCO_DICT_II_TRIGRAM    0x800 /* tri-gram index */

typedef struct tag_mco_dict_index_info_t_ {
    unsigned short     index_no; /* index of the index descriptor (not dictionary index) */
    char             * name;     /* name of the index */
    unsigned int       flags;    /* flags, see above */
    unsigned short     n_fields; /* number of fields in key */
    unsigned short     index;    /* continuous index numbering in terms of function mco_index_stat_get */
    unsigned int       n_keys_estimate; /* estimated number of keys */
} mco_dict_index_info_t, * mco_dict_index_info_p;

MCOUDA_API MCO_RET mco_dict_index       ( const mco_metadict_header_t * metadict, unsigned short dict_no,
                               unsigned short struct_no, unsigned short index_no,
                                                        /* out */ mco_dict_index_info_p index_info );
MCOUDA_API MCO_RET mco_dict_index_name  ( const mco_metadict_header_t * metadict, unsigned short dict_no, 
                               unsigned short struct_no, const char * name, 
                                                        /* out */ mco_dict_index_info_p index_info );

/* indexed fields flags */
#define MCO_DICT_IFI_DESCENDING 1 /* descending order */
#define MCO_DICT_IFI_NULLABLE   2 /* nullable field */
typedef struct tag_mco_dict_ifield_info_t_ {
    unsigned short     ifield_no; /* index of the descriptor */
    unsigned short     field_no;  /* reference to the field descriptor for name, type, etc */
    unsigned int       flags;     /* flags, see above */
    short              coll_no;   /* index of collation descriptor */
} mco_dict_ifield_info_t, * mco_dict_ifield_info_p;

MCOUDA_API MCO_RET mco_dict_ifield      ( const mco_metadict_header_t * metadict, unsigned short dict_no,
                               unsigned short struct_no, unsigned short index_no, unsigned short ifield_no,
                                                        /* out */ mco_dict_ifield_info_p ifield_info );
MCOUDA_API MCO_RET mco_dict_ifield_name ( const mco_metadict_header_t * metadict, unsigned short dict_no, 
                               unsigned short struct_no, unsigned short index_no, const char * name, 
                                                        /* out */ mco_dict_ifield_info_p ifield_info );

typedef struct tag_mco_dict_nested_ifield_info_t_ {
    unsigned short     ifield_no;
    unsigned short     level;
    unsigned short     struct_no;
    unsigned short     field_no;
    unsigned int       flags;
    short              coll_no;
} mco_dict_nested_ifield_info_t, * mco_dict_nested_ifield_info_p;

MCOUDA_API MCO_RET mco_dict_nested_ifield ( const mco_metadict_header_t * metadict, unsigned short dict_no,
                               unsigned short struct_no, unsigned short index_no, unsigned short ifield_no,
                               unsigned short level, /* out */ mco_dict_nested_ifield_info_p nested_ifield_info );

typedef struct tag_mco_dict_collation_info_t_ {
    unsigned short  collation_no; /* index of the collation descriptor */
    mco_dict_type_t type;         /* field type */
    char          * name;         /* name of the collation */
} mco_dict_collation_info_t, * mco_dict_collation_info_p;

MCOUDA_API MCO_RET mco_dict_collation_count( const mco_metadict_header_t * metadict, unsigned short dict_no, 
                                                        /* out */ unsigned short * count );
MCOUDA_API MCO_RET mco_dict_collation      ( const mco_metadict_header_t * metadict, unsigned short dict_no,
                                  unsigned short collation_no, /* out */ mco_dict_collation_info_p coll_info );
MCOUDA_API MCO_RET mco_dict_collation_name ( const mco_metadict_header_t * metadict, unsigned short dict_no, 
                                  const char * name, /* out */ mco_dict_collation_info_p coll_info );


typedef struct tag_mco_dict_event_info_t_ {
    unsigned short  event_no; /* index of the event descriptor */
    MCO_EVENT_TYPE  type;     /* type of the event */
    unsigned short  field_no; /* associated field descriptor, for update events only */
} mco_dict_event_info_t, * mco_dict_event_info_p;

MCOUDA_API MCO_RET mco_dict_event ( const mco_metadict_header_t *metadict, unsigned short dict_no, 
                         unsigned short struct_no, unsigned short event_no, 
                            /* out */ mco_dict_event_info_p event_info);    

/* Data access API */

typedef struct tag_mco_uda_object_handle_t_ {
    MCO_Hf obj;                 /* eXtremeDB's handle */
    unsigned short struct_no;   /* ref to struct descriptor */
} mco_uda_object_handle_t, * mco_uda_object_handle_p;

typedef struct tag_mco_uda_value_t_ {

    mco_dict_type_t type;             /* type of the value */
    union {
        unsigned char           u1;   /* MCO_DD_UINT1 */
        unsigned short          u2;   /* MCO_DD_UINT2 */
        unsigned int            u4;   /* MCO_DD_UINT4, MCO_DD_DATE, MCO_DD_TIME */
        uint8                   u8;   /* MCO_DD_UINT8, MCO_DD_AUTOID, MCO_DD_AUTOOID, MCO_DD_DATETIME */
        signed char             i1;   /* MCO_DD_INT1 */
        short                   i2;   /* MCO_DD_INT2 */
        int                     i4;   /* MCO_DD_INT4 */
        mco_int8                i8;   /* MCO_DD_INT8 */
        #ifdef MCO_TARGET_FLOAT_SUPPORTED        
            float               f;    /* MCO_DD_FLOAT */
            double              d;    /* MCO_DD_DOUBLE */
        #endif
        mco_uda_object_handle_t o;    /* MCO_DD_STRUCT */
        struct {
            unsigned int        size; /* size of a buffer in bytes */
            unsigned int        len;  /* length of a string in characters */ 
            union {
                char          * c;    /* char strings and binary data: MCO_DD_STRING, MCO_DD_CHAR, MCO_DD_BINARY, MCO_DD_VARBINARY */
                nchar_t       * n;    /* nchar strings: MCO_DD_NCHAR_STRING, MCO_DD_NCHAR_CHAR */
                #ifdef MCO_CFG_WCHAR_SUPPORT 
                    wchar_t   * w;    /* wchar strings: MCO_DD_WCHAR_STRING, MCO_DD_WIDE_CHAR */
                #endif 
                void          * v;    /* general pointer : MCO_DD_BLOB, MCO_DD_REF, MCO_DD_OID */
            } p;                      /* pointer to the buffer */
        } p;                          
    } v;
} mco_uda_value_t, * mco_uda_value_p;

/* Create new object. */
MCOUDA_API MCO_RET mco_uda_new         ( mco_trans_h t, unsigned short struct_no, 
                              const void * oid, 
                              const mco_uda_value_t * initializers, unsigned short initializers_no,
                                                        /* out */ mco_uda_object_handle_t * obj );

MCOUDA_API MCO_RET mco_uda_delete      ( mco_uda_object_handle_t * obj );

/* Delete all objects of the given type */
MCOUDA_API MCO_RET mco_uda_delete_all  ( mco_trans_h t, unsigned short stuct_no);

MCOUDA_API MCO_RET mco_uda_checkpoint  ( mco_uda_object_handle_t * obj );

/* Get object class code */
MCOUDA_API MCO_RET mco_uda_get_class_code( mco_uda_object_handle_t *obj, unsigned short *class_code);

/* Makes MCO_Hf by mco_uda_object_handle_t */
MCOUDA_API MCO_RET mco_uda_to_mco(mco_uda_object_handle_t *udaobj, /* OUT */ MCO_Hf *ret);

/* Makes mco_uda_object_handle_t by MCO_Hf */
MCOUDA_API MCO_RET mco_uda_from_mco(MCO_Hf *mcoobj, /* OUT */ mco_uda_object_handle_t *ret);


/* Open a cursor for object enumeration */
MCOUDA_API MCO_RET mco_uda_cursor      ( mco_trans_h t, unsigned short struct_no, unsigned short index_no,
                                                        /* out */ mco_cursor_t * cursor );

MCOUDA_API MCO_RET mco_uda_lookup      ( mco_trans_h t, unsigned short struct_no, unsigned short index_no, 
                              MCO_OPCODE op, const mco_uda_value_t * keys, unsigned short keys_count,
                                                        /* out */ mco_cursor_t * cursor );

MCOUDA_API MCO_RET mco_uda_from_cursor ( mco_trans_h t, const mco_cursor_t * cursor,
                                                        /* out */ mco_uda_object_handle_t * obj );

/* request cursor bindings */
MCOUDA_API MCO_RET mco_uda_cursor_info ( mco_trans_h t, const mco_metadict_header_t * metadict, const mco_cursor_t * cursor,
                                                        /* out */ unsigned short * dict_no,
                                                        /* out */ unsigned short * struct_no,
                                                        /* out */ unsigned short * index_no ); 

MCOUDA_API MCO_RET mco_uda_locate      ( mco_trans_h t, const mco_uda_object_handle_p obj, unsigned short index_no,
                                                        /* out */ mco_cursor_t * cursor );

MCOUDA_API MCO_RET mco_uda_compare     ( mco_trans_h t, const mco_cursor_t * cursor, 
                              const mco_uda_value_t * keys, unsigned short keys_count,
                                                        /* out */ int * cmp_result );

/* pattern search */
MCOUDA_API MCO_RET  mco_uda_pattern_size(const mco_metadict_header_t *metadict, unsigned short dict_no, unsigned short struct_no, unsigned short index_no,  
                const mco_uda_value_t * keys, unsigned short keys_count, /* out */ uint4 *memsize);

MCOUDA_API MCO_RET  mco_uda_pattern_search( mco_trans_h t, unsigned short struct_no, unsigned short index_no, const mco_uda_value_t * keys, 
                unsigned short keys_count, /* inout */ mco_cursor_h cursor, /* inout */ void *allocated_pattern);

MCOUDA_API MCO_RET  mco_uda_pattern_next(mco_trans_h t, mco_cursor_h cursor, /* inout */ void *allocated_pattern);


/* Get current length of an array or a vector */
MCOUDA_API MCO_RET mco_uda_length      ( const mco_uda_object_handle_p obj, unsigned short field_no, 
                                                        /* out */ unsigned short * dimension );

/* Get value of a scalar field, element of a vector or array, blob or struct.
 * 
 * Application should specify mco_dd_value_t.type field only for any datatypes but strings and blobs.
 * In case of a structured field mco_dd_value_t.o used.
 * The application should use following technique to get value of  string or blob fields:
 * 1. Put datatype code in type field
 * 2. Put zero in mco_dd_value_t.p.p.* field
 * 3. Call mco_dd_get() first time to get size of value (mco_dd_value_t.p.size).
 * 4. Put a pointer to allocated memory region into one of mco_dd_value_t.p.p.*
 * 5. Call mco_dd_get() one more time to get the value into spec. buffer.
 *
 * If the buffer is not enought to contain full value of a string or blob field then only part 
 * of the value will be returned. 
 * If the buffer is larger then the value then mco_dd_get() will put a zero-terminator in the end of the
 * string value.
 */
MCOUDA_API MCO_RET mco_uda_get         ( const mco_uda_object_handle_p obj, unsigned short field_no, unsigned short index,  
                                                        /* out */ mco_uda_value_t * buf );

MCOUDA_API MCO_RET mco_uda_put         ( mco_uda_object_handle_p obj, unsigned short field_no, unsigned short index, 
                             const mco_uda_value_p value );

MCOUDA_API MCO_RET mco_uda_blob_append ( mco_uda_object_handle_p obj, unsigned short field_no,
                             const void *val, uint4 size);

/* Get default value for the field, eihter scalar or array element
 * strings are not supported here 
 */
MCOUDA_API MCO_RET mco_dict_field_init_data(const mco_metadict_header_t * metadict, unsigned short dict_no,
                       unsigned short struct_no, unsigned short field_no, unsigned short index,
                                        /* out */ mco_uda_value_t * buf);

/* vectors and optional structs ops */

/* allocate a vector or optional struct field */
MCOUDA_API MCO_RET mco_uda_field_alloc ( mco_uda_object_handle_p obj, unsigned short field_no, unsigned short length );

/* free a vector element or optional struct field */
MCOUDA_API MCO_RET mco_uda_field_erase  ( mco_uda_object_handle_p obj, unsigned short field_no, unsigned short index );
#define mco_uda_field_free mco_uda_field_erase /* deprecated */


/* Find object by OID */
MCOUDA_API MCO_RET mco_uda_oid_find( mco_trans_h t, const mco_uda_value_t *id, /* out */ mco_uda_object_handle_p obj);


/* userdef indices support */

typedef int2 (*mco_uda_compare_userdef_f)(mco_uda_object_handle_p obj1, unsigned short index1, 
                                        mco_uda_object_handle_p obj2, unsigned short index2, void *user_context);
typedef int2 (*mco_uda_compare_extkey_userdef_f)(mco_uda_object_handle_p obj, unsigned short index, 
                                        mco_uda_value_t *keys, uint2 keys_count, void *user_context);
typedef uint4 (*mco_uda_hash_userdef_f)(mco_uda_object_handle_p obj, unsigned short index, void *user_context);
typedef uint4 (*mco_uda_hash_extkey_userdef_f)(mco_uda_value_t *keys, uint2 keys_count, void *user_context);

typedef struct mco_uda_userdef_funcs_t_ {
    mco_uda_compare_userdef_f         fcomp;
    mco_uda_compare_extkey_userdef_f  fcomp_ext;
    mco_uda_hash_userdef_f            fhash;
    mco_uda_hash_extkey_userdef_f     fhash_ext;
} mco_uda_userdef_funcs_t, *mco_uda_userdef_funcs_h;

MCOUDA_API MCO_RET mco_uda_get_udfmap_size(const mco_metadict_header_t * metadict, unsigned short dict_no, /* out */ unsigned int * size);

MCOUDA_API MCO_RET mco_uda_register_udf(const mco_metadict_header_t * metadict, unsigned short dict_no, 
                                        unsigned short struct_no, unsigned short index_no, mco_userdef_funcs_h udf_map, 
                                        mco_uda_userdef_funcs_h udf_entry, void *user_context);

MCOUDA_API MCO_RET mco_uda_get_collmap_size(const mco_metadict_header_t * metadict, unsigned short dict_no, /* out */ unsigned int * size);
MCOUDA_API MCO_RET mco_uda_register_collation(const mco_metadict_header_t * metadict, unsigned short dict_no, 
                                unsigned short collation_no, mco_collation_funcs_h coll_map, 
                                mco_compare_collation_f fcomp, mco_hash_collation_f fhash);

MCOUDA_API MCO_RET mco_uda_collate_get(mco_collate_h c, /* out */ mco_uda_value_t * val);
MCOUDA_API MCO_RET mco_uda_collate_get_range(mco_collate_h c, uint2 from, /* out */ mco_uda_value_t * val);

MCOUDA_API MCO_RET mco_uda_db_open(const mco_metadict_header_t * metadict, unsigned short dict_no, 
                            mco_device_t * devs, mco_size_t n_devs, mco_db_params_t * params);

MCOUDA_API MCO_RET mco_uda_db_close(const mco_metadict_header_t * metadict, unsigned short dict_no);

/* Events support */
/* Synchronous events */
typedef MCO_RET (*mco_uda_event_handler)( mco_trans_h t, mco_uda_object_handle_p obj, const mco_dict_event_info_t *event_info, /* inout */ void *param );

MCOUDA_API MCO_RET mco_uda_register_event_handler( mco_trans_h t, unsigned short struct_no, unsigned short event_no,
                                        mco_uda_event_handler handler, void * param, MCO_HANDLING_ORDER when);

MCOUDA_API MCO_RET mco_uda_unregister_event_handler( mco_trans_h t, unsigned short struct_no, unsigned short event_no,
                                        mco_uda_event_handler handler);

MCOUDA_API MCO_RET  mco_uda_updated_bitmap_get_fld(mco_uda_object_handle_t *handle, uint2* index, unsigned short *field_no);

/* Asynchronous events */
MCO_RET mco_uda_async_event_wait(mco_db_h dbh, unsigned short struct_no, unsigned short event_no);
MCO_RET mco_uda_async_event_release(mco_db_h dbh, unsigned short struct_no, unsigned short event_no);
MCO_RET mco_uda_async_event_release_all(mco_db_h dbh);


/* ================================= Sequences ========================== */
#include "mcoseq.h"

typedef struct tag_mco_uda_seq_values_t_ {
    mco_size_t      size;
    mco_dict_type_t elem_type;
    uint2           elem_size; /* for MCO_DD_CHAR */
	mco_bitmap_word_t* null_bitmap;
    union {
        mco_uint1       *u1;
        mco_uint2       *u2;
        mco_uint4       *u4;
        mco_uint8       *u8;
        mco_int1        *i1;
        mco_int2        *i2;
        mco_int4        *i4;
        mco_int8        *i8;
		mco_datetime    *dt;
#ifdef MCO_TARGET_FLOAT_SUPPORTED        
        float           *f;
        double          *d;
#endif
        char            *c;
    } v;
} mco_uda_seq_values_t;

MCOUDA_API mco_bool mco_uda_is_sequence(mco_dict_type_t type);

MCOUDA_API MCO_RET mco_uda_seq_first(const mco_uda_object_handle_p obj, unsigned short field_no,/*OUT*/ mco_uda_value_t *value);
MCOUDA_API MCO_RET mco_uda_seq_last(const mco_uda_object_handle_p obj, unsigned short field_no,/*OUT*/ mco_uda_value_t *value);
MCOUDA_API MCO_RET mco_uda_seq_append(mco_uda_object_handle_p obj, unsigned short field_no, mco_uda_seq_values_t *values);
MCOUDA_API MCO_RET mco_uda_seq_insert(mco_uda_object_handle_p obj, unsigned short field_no, mco_seq_no_t pos, mco_uda_seq_values_t *values);
MCOUDA_API MCO_RET mco_uda_seq_delete(mco_uda_object_handle_p obj, unsigned short field_no, mco_seq_no_t from, mco_seq_no_t till);
MCOUDA_API MCO_RET mco_uda_seq_count(const mco_uda_object_handle_p obj, unsigned short field_no, mco_seq_no_t *count);
MCOUDA_API MCO_RET mco_uda_seq_search(const mco_uda_object_handle_p obj, unsigned short field_no, mco_seq_iterator_h iterator, const mco_uda_value_t *low, mco_seq_boundary_kind_t low_boundary, const mco_uda_value_t *high, mco_seq_boundary_kind_t high_boundary);
MCOUDA_API MCO_RET mco_uda_seq_join(mco_uda_object_handle_p obj, unsigned short field_no, mco_seq_iterator_h result, mco_seq_iterator_h join_with);
MCOUDA_API MCO_RET mco_uda_seq_obj_map(mco_uda_object_handle_p obj, unsigned short field_no, mco_seq_iterator_h dst_iterator, mco_seq_iterator_h map_iterator);
MCOUDA_API MCO_RET mco_uda_seq_subseq(mco_uda_object_handle_p obj, unsigned short field_no, mco_seq_iterator_h iterator, mco_seq_no_t from, mco_seq_no_t till);
MCOUDA_API MCO_RET mco_uda_seq_iterator(mco_uda_object_handle_p obj, unsigned short field_no, mco_seq_iterator_h iterator);
MCOUDA_API MCO_RET mco_uda_seq_project(mco_uda_object_handle_p obj, unsigned short field_no, mco_seq_iterator_h dst, mco_seq_iterator_h src);
MCOUDA_API MCO_RET mco_uda_seq_store(mco_uda_object_handle_p obj, unsigned short field_no, mco_seq_iterator_h src);
MCOUDA_API MCO_RET mco_uda_seq_truncate(const mco_uda_object_handle_p udaobj, unsigned short field_no);

typedef void (*mco_uda_seq_func_ptr_t)(mco_uda_value_t *result, mco_uda_value_t *arg, void* ctx);
typedef void (*mco_uda_seq_func2_ptr_t)(mco_uda_value_t *result, mco_uda_value_t *arg1, mco_uda_value_t *arg2, void* ctx);
typedef mco_seq_bool (*mco_uda_seq_cond_ptr_t)(mco_uda_value_t * arg, void* ctx);

MCOUDA_API MCO_RET mco_uda_seq_next(mco_seq_iterator_h iterator, mco_uda_value_t* val);

MCOUDA_API MCO_RET mco_uda_seq_parse(mco_seq_iterator_h result, mco_dict_type_t type, char const* str);
MCOUDA_API MCO_RET mco_uda_seq_const(mco_seq_iterator_h result, const mco_uda_value_t *val);
MCOUDA_API MCO_RET mco_uda_seq_func(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_uda_seq_func_ptr_t func, void* ctx);
MCOUDA_API MCO_RET mco_uda_seq_func2(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right, mco_uda_seq_func2_ptr_t func, void* ctx);
MCOUDA_API MCO_RET mco_uda_seq_cond(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_uda_seq_cond_ptr_t func, void* ctx);
MCOUDA_API MCO_RET mco_uda_seq_map(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h positions);
MCOUDA_API MCO_RET mco_uda_seq_get(mco_seq_iterator_h input, mco_uda_seq_values_t *values);
MCOUDA_API MCO_RET mco_uda_seq_unget(mco_seq_iterator_h result, mco_uda_seq_values_t *buf);
MCOUDA_API MCO_RET mco_uda_seq_metajoin(mco_seq_iterator_h result, mco_seq_meta_iterator_h meta_iterator);
MCOUDA_API MCO_RET mco_uda_seq_union(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right, mco_seq_order_t order);

MCOUDA_API MCO_RET mco_uda_seq_add(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_sub(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_mul(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_div(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_mod(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_max(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_min(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_eq(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_ne(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_gt(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_ge(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_lt(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_le(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_wsum(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_wavg(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_cov(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
MCOUDA_API MCO_RET mco_uda_seq_corr(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);


MCOUDA_API MCO_RET mco_uda_seq_norm(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_thin(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t origin, mco_size_t step);
MCOUDA_API MCO_RET mco_uda_seq_iif(mco_seq_iterator_h result, mco_seq_iterator_h cond, mco_seq_iterator_h then_iter, mco_seq_iterator_h else_iter);
MCOUDA_API MCO_RET mco_uda_seq_if(mco_seq_iterator_h result, mco_seq_iterator_h cond, mco_seq_iterator_h then_iter, mco_seq_iterator_h else_iter);
MCOUDA_API MCO_RET mco_uda_seq_filter(mco_seq_iterator_h result, mco_seq_iterator_h cond, mco_seq_iterator_h input);

MCOUDA_API MCO_RET mco_uda_seq_unique(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_abs(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_neg(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_reverse(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_diff(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_trend(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_agg_max(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_agg_min(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_agg_sum(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_agg_prd(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_agg_avg(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_agg_var(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_agg_dev(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_agg_var_samp(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_agg_dev_samp(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_agg_min_max(mco_seq_iterator_h result, mco_seq_iterator_h input);

MCOUDA_API MCO_RET mco_uda_seq_group_agg_max(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by);
MCOUDA_API MCO_RET mco_uda_seq_group_agg_min(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by);
MCOUDA_API MCO_RET mco_uda_seq_group_agg_sum(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by);
MCOUDA_API MCO_RET mco_uda_seq_group_agg_avg(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by);
MCOUDA_API MCO_RET mco_uda_seq_group_agg_var_samp(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by);
MCOUDA_API MCO_RET mco_uda_seq_group_agg_dev_samp(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by);
MCOUDA_API MCO_RET mco_uda_seq_group_agg_var(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by);
MCOUDA_API MCO_RET mco_uda_seq_group_agg_dev(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by);
MCOUDA_API MCO_RET mco_uda_seq_group_agg_last(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by);
MCOUDA_API MCO_RET mco_uda_seq_group_agg_first(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by);

MCOUDA_API MCO_RET mco_uda_seq_group_agg_wavg(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right, mco_seq_iterator_h group_by);

MCOUDA_API MCO_RET mco_uda_seq_grid_agg_max(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_grid_agg_min(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_grid_agg_sum(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_grid_agg_avg(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_grid_agg_var(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_grid_agg_dev(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_grid_agg_var_samp(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_grid_agg_dev_samp(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_window_agg_max(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_window_agg_min(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_window_agg_sum(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_window_agg_avg(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_window_agg_var(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_window_agg_dev(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_window_agg_var_samp(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_window_agg_dev_samp(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_window_agg_ema(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);
MCOUDA_API MCO_RET mco_uda_seq_window_agg_atr(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval);

MCOUDA_API MCO_RET mco_uda_seq_hash_agg_max(mco_seq_iterator_h result_max, mco_seq_iterator_h result_group_by, mco_seq_iterator_h input, mco_seq_iterator_h group_by, mco_size_t n_groups);
MCOUDA_API MCO_RET mco_uda_seq_hash_agg_min(mco_seq_iterator_h result_min, mco_seq_iterator_h result_group_by, mco_seq_iterator_h input, mco_seq_iterator_h group_by, mco_size_t n_groups);
MCOUDA_API MCO_RET mco_uda_seq_hash_agg_sum(mco_seq_iterator_h result_sum, mco_seq_iterator_h result_group_by, mco_seq_iterator_h input, mco_seq_iterator_h group_by, mco_size_t n_groups);
MCOUDA_API MCO_RET mco_uda_seq_hash_agg_avg(mco_seq_iterator_h result_avg, mco_seq_iterator_h result_group_by, mco_seq_iterator_h input, mco_seq_iterator_h group_by, mco_size_t n_groups);

MCOUDA_API MCO_RET mco_uda_seq_top_max(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t top);
MCOUDA_API MCO_RET mco_uda_seq_top_min(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t top);
MCOUDA_API MCO_RET mco_uda_seq_top_pos_max(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t top);
MCOUDA_API MCO_RET mco_uda_seq_top_pos_min(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t top);

MCOUDA_API MCO_RET mco_uda_seq_cum_agg_avg(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_cum_agg_sum(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_cum_agg_min(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_cum_agg_max(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_cum_agg_prd(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_cum_agg_var(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_cum_agg_dev(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_cum_agg_var_samp(mco_seq_iterator_h result, mco_seq_iterator_h input);
MCOUDA_API MCO_RET mco_uda_seq_cum_agg_dev_samp(mco_seq_iterator_h result, mco_seq_iterator_h input);


MCOUDA_API MCO_RET mco_uda_seq_histogram(mco_seq_iterator_h result, mco_seq_iterator_h input, const mco_uda_value_t *min_value, const mco_uda_value_t *max_value, mco_size_t n_intervals);
MCOUDA_API MCO_RET mco_uda_seq_cross(mco_seq_iterator_h result, mco_seq_iterator_h input, int first_cross_direction);
MCOUDA_API MCO_RET mco_uda_seq_extrema(mco_seq_iterator_h result, mco_seq_iterator_h input, int first_extremum);
MCOUDA_API MCO_RET mco_uda_seq_sort(const mco_uda_seq_values_t *elems, mco_seq_no_t* permutation, mco_seq_order_t order);
MCOUDA_API MCO_RET mco_uda_seq_repeat(mco_seq_iterator_h result, mco_seq_iterator_h input, int n_times);
MCOUDA_API MCO_RET mco_uda_seq_order_by(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_no_t const* permutation, mco_size_t n_elems, mco_uda_seq_values_t *data);

MCOUDA_API MCO_RET mco_uda_seq_stretch(mco_seq_iterator_h result, mco_seq_iterator_h ts1, mco_seq_iterator_h ts2, mco_seq_iterator_h values, const mco_uda_value_t *filler);
MCOUDA_API MCO_RET mco_uda_seq_stretch0(mco_seq_iterator_h result, mco_seq_iterator_h ts1, mco_seq_iterator_h ts2, mco_seq_iterator_h values, const mco_uda_value_t *filler);
MCOUDA_API MCO_RET mco_uda_seq_asof_join(mco_seq_iterator_h result, mco_seq_iterator_h ts1, mco_seq_iterator_h ts2, mco_seq_iterator_h values);
MCOUDA_API MCO_RET mco_uda_seq_cast(mco_seq_iterator_h result, mco_dict_type_t result_type, mco_seq_iterator_h input);

#define mco_uda_seq_print                       mco_seq_print_char
#define mco_uda_seq_match                       mco_seq_match
/* Polymorphic functions */
#define mco_uda_seq_concat                      mco_seq_concat
#define mco_uda_seq_cat                         mco_seq_cat
#define mco_uda_seq_limit                       mco_seq_limit
#define mco_uda_seq_agg_count                   mco_seq_agg_count
#define mco_uda_seq_agg_approxdc                mco_seq_agg_approxdc
#define mco_uda_seq_group_agg_count             mco_seq_group_agg_count
#define mco_uda_seq_group_agg_approxdc          mco_seq_group_agg_approxdc
#define mco_uda_seq_hash_agg_count              mco_seq_hash_agg_count
#define mco_uda_seq_hash_agg_approxdc           mco_seq_hash_agg_approxdc
#define mco_uda_seq_hash_agg_distinct_count     mco_seq_hash_agg_distinct_count
#define mco_uda_seq_hash_agg_dup_count          mco_seq_hash_agg_dup_count
#define mco_uda_seq_release_hash                mco_seq_release_hash
#define mco_uda_seq_free_hash                   mco_seq_free_hash

#define mco_uda_seq_hash                        mco_seq_hash
#define mco_uda_seq_agg_approxdc_hash           mco_seq_agg_approxdc_hash
#define mco_uda_seq_group_agg_approxdc_hash     mco_seq_group_agg_approxdc_hash

#define mco_uda_seq_tee                         mco_seq_tee
#define mco_uda_seq_filter_pos                  mco_seq_filter_pos
#define mco_uda_seq_filter_search               mco_seq_filter_search
#define mco_uda_seq_filter_first_pos            mco_seq_filter_first_pos

#define mco_uda_seq_not                         mco_seq_not
#define mco_uda_seq_and                         mco_seq_and
#define mco_uda_seq_or                          mco_seq_or
#define mco_uda_seq_xor                         mco_seq_xor


#ifdef __cplusplus
    }
#endif 

#endif /* MCO_UNIFORM_DATA_ACCESS_API_H */


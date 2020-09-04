/*******************************************************************
 *                                                                 *
 *  mcoddl.h                                                    *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           * 
 *  All Rights Reserved                                            *
 *                                                                 *
 *  eXtremeDB runtime uniform data access API                      * 
 *                                                                 *
 *******************************************************************/

#ifndef MCO_DDL_H
#define MCO_DDL_H

#include "mcouda.h"

#ifdef __cplusplus
extern "C" 
{
#endif 
    
typedef enum mco_ddl_dictionary_flags_t_ { 
    MCO_DICT_FLAGS_USE_AUTOOID = 1,
    MCO_DICT_FLAGS_PRESERVE_FIELDS_ORDER = 2
} mco_ddl_dictionary_flags_t;

typedef enum mco_ddl_class_flags_t_ { 
    MCO_CLASS_FLAGS_HAS_LIST_INDEX   = 0x0001,
    MCO_CLASS_FLAGS_HAS_AUTOID_INDEX = 0x0040,
    MCO_CLASS_FLAGS_COMPACT          = 0x0008,
    MCO_CLASS_FLAGS_PERSISTENT       = 0x0020,
    MCO_CLASS_FLAGS_LOCAL            = 0x0400,
    MCO_CLASS_FLAGS_DISTRIBUTED      = 0x0800,
    MCO_CLASS_FLAGS_HIDDEN           = 0x1000,
    MCO_CLASS_FLAGS_DROPPED          = 0x4000,
    MCO_CLASS_FLAGS_UPTABLE          = 0x0080,
    MCO_CLASS_FLAGS_DOWNTABLE        = 0x8000
} mco_ddl_class_flags_t;

typedef enum mco_ddl_index_type_t_ { 
    MCO_INDEX_BTREE, 
    MCO_INDEX_RTREE, 
    MCO_INDEX_KDTREE, 
    MCO_INDEX_PATRICIA, 
    MCO_INDEX_HASHTABLE,
    MCO_INDEX_TRIGRAM,
    MCO_INDEX_INCLUSIVE
} mco_ddl_index_type_t;
    
typedef enum mco_ddl_index_flags_t_ { 
    MCO_INDEX_FLAGS_UNIQUE = 0x0004,
    MCO_INDEX_FLAGS_THICK = 0x20000
} mco_ddl_index_flags;

typedef enum mco_ddl_key_flags_t_ { 
    MCO_KEY_FLAGS_DESCENDING = 1
} mco_ddl_key_flags;

typedef enum mco_ddl_field_flags_t_ { 
    MCO_FIELD_FLAGS_OPTIONAL = 0x04,
    MCO_FIELD_FLAGS_HIDDEN   = 0x10,
    MCO_FIELD_FLAGS_NULLABLE = 0x20,
    MCO_FIELD_FLAGS_NUMERIC  = 0x80,
    MCO_FIELD_FLAGS_ASC_SEQUENCE = 0x100,
    MCO_FIELD_FLAGS_DESC_SEQUENCE = 0x200
} mco_ddl_field_flags_t;

typedef struct mco_ddl_field_t_ { 
    char const* name;
    mco_dict_type_t type;
    int n_elements; /* 0 - vector, > 1 - array */
    int size;  /* for char<N> type */
    int flags; /* mco_ddl_field_flags_t */
    int precision;
    struct mco_ddl_struct_t_* structure; /* for structure field */
    void* init_value;
} mco_ddl_field_t;

typedef struct mco_ddl_struct_t_ { 
    char const* name;
    int n_fields;
    mco_ddl_field_t* fields;
} mco_ddl_struct_t;

typedef struct mco_ddl_key_t_ { 
    int field_path_len;
    mco_ddl_field_t** field_path; /* path to the indexed field (descibing all parent structs containing indexed field) */
    int flags; /* mco_ddl_key_flags_t */
} mco_ddl_key_t;

typedef struct mco_ddl_index_t_ { 
    char const* name;
    mco_ddl_index_type_t index_type;
    int n_keys;
    mco_ddl_key_t* keys;
    int flags; /* mco_ddl_index_flags_t */
    mco_size_t init_size;
} mco_ddl_index_t; 

typedef struct mco_ddl_event_t_ {
    MCO_EVENT_TYPE type;
    /* for update events only */
    int field_path_len; 
    mco_ddl_field_t** field_path; /* path to the updated field (descibing all parent structs containing updated field) */
} mco_ddl_event_t;

typedef struct mco_ddl_class_t_ { 
    mco_ddl_struct_t* structure;
    int n_indices;
    mco_ddl_index_t* indices;
    int n_events;
    mco_ddl_event_t* events;
    int flags; /* mco_ddl_class_flags_t */
} mco_ddl_class_t;


typedef struct mco_ddl_dictionary_t_ { 
    int n_classes;
    mco_ddl_class_t* classes;
    int n_structs;
    mco_ddl_struct_t* structs;
    int flags; /* mco_ddl_dictionary_flags_t */
} mco_ddl_dictionary_t;

extern mco_size_t mco_db_get_dictionary_size(mco_db_h dbh);
extern MCO_RET mco_ddl_build_dictionary(mco_db_h dbh, mco_ddl_dictionary_t* dictionary, mco_size_t* dict_size);
extern MCO_RET mco_ddl_get_dictionary(mco_db_h dbh, void* buf, mco_size_t buf_size, mco_size_t* ddl_dict_size);

#ifdef __cplusplus
}
#endif 

#endif

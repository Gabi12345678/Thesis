/*******************************************************************
 *                                                                 *
 *  mcowrap.h                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *  eXtremeDB runtime generated interface definitions              *
 *                                                                 *
 ***************************************************************** */

#ifndef MCO_WRAP__H__
    #define MCO_WRAP__H__

    #ifndef MCO_DICT_CONST
    #define MCO_DICT_CONST
    #endif

    #ifdef __cplusplus
        extern "C"
        {
        #endif 

        #include "mco.h"
        #include "mcouda.h"
        #include "mcoseq.h"

         /* calculator: */

         typedef struct mco_calc_class_t_
         {
             const char *name; /* calculator class name */
             int nobjects;     /* number of objects of the class to create */
             int class_code;
         } mco_calc_class_t;

         typedef struct mco_calculator_t_
         {
             MCO_DICT_CONST mco_calc_class_t *classes; /* an array of classes registered in calculator */
             int nclasses;              /* number of classes in the array */
         } mco_calculator_t;
             

        /* dictionary: */

        /* layout for field/structure in a parent struct/class */
        typedef struct mco_datalayout_t_
        {
            uint2 c_size; /* compact: size not including dynamic part    */
            uint2 c_align; /* compact: align                              */
            uint2 c_offset;
            uint4 u_size; /* notcompact: size not including dynamic part */
            uint4 u_align; /* notcompact: align                           */
            uint4 u_offset;
        } mco_datalayout_t;

        /* field information relative to holding struct/class */
        typedef struct mco_dict_field_t_
        {
            const char* name;
            mco_datalayout_t layout;
            uint1 field_el_type; /* MCO_DB_FLDTYPE_E */
            uint1 flags; /* MCO_DICT_FLDF_   */
            uint2 array_size; /* for array fields */
            int4 struct_num; /* if field is nested struct/optional struct, index of mco_dict_struct_t_ */
            uint4 field_size; /* of the field, 0 when statically unknown  */
            int2 refto_class; /* index of a class inside mco_dict_class_info_t table */
            unsigned int init_index; /* depending on type; array has init values for each el */
            uint2 order_no; /* order of the field in class declaration */
            uint2 no; /* seq. number in a owner struct */
            uint2 event_id;
            uint2 indicator; /* index of indicator field for nullable fields */
            int1  precision; /* precision for numeric type (if greater or equal than zero) */
            int1  seq_order; /* mco_seq_order_t */
            int1  seq_elem_size; /* size of sequence element */
        } mco_dict_field_t;

        typedef struct mco_dict_struct_t_
        {
            const char* name;
            uint2 flags; /* MCO_DICT_STF_...  */
            uint2 n_fields;
            MCO_DICT_CONST mco_dict_field_t* fields; /* [ n_fields ]      */
            uint2 c_size; /* compact: size not including dynamic part    */
            uint2 c_align; /* compact: align                              */
            uint4 u_size; /* notcompact: size not including dynamic part */
            uint4 u_align; /* notcompact: align                           */
        } mco_dict_struct_t;

        typedef struct mco_dict_collation_t_
        {
            const char *name;   /* collation name */
            uint1 type;         /* MCO_DB_FT_... */
            uint1 pad1;
            uint2 pad2;
        } mco_dict_collation_t;


        typedef struct mco_dict_index_field_t_ /* index field description */
        {
            /* for an index compact flag is known statically        */
            mco_offs32_t field_offset; /* for vstruct_based - offset in a structure - vect el  */
            /* For vtype_based: 0                       */
            mco_offs32_sig_t vect_field_offset; /* offset used for v_based indexes.*/
            mco_offs32_t indicator_offset; /* offset of indicator for nullable fields */
            uint4 field_size; /* of the field, 0 when statically unknown  */
            uint1 field_type; /* MCO_DB_FLDTYPE_E */
            uint1 fld_flags; /* MCO_DB_INDFLD_... */
            uint2 fld_no;    /* number of the field in the owner struct */
            int2 collation_id; /* number of custom collation function or -1 */
        } mco_dict_index_field_t;



        typedef struct mco_dict_index_t_
        {
            uint2 class_code;
            uint2 impl_no;  /* index type */
            uint2 numof_fields;
            mco_offs32_sig_t vect_field_offset; /* -1 for not v_based */
            uint4 flags; /* MCO_DB_INDF_...  */
            MCO_DICT_CONST mco_dict_index_field_t* fields;
            mco_hash_counter_t numof_keys_estimate; /* for hash indexes */
            int2 userdef_id; /** number of custom-index or -1 for non-userdef indices */
            int2 reserved;
        } mco_dict_index_t;



        typedef struct mco_dict_class_info_t_ /* indexes specification for a class */
        {
            int2 first_index_num; /* index numbers are sequential for a class */
            int2 last_index_num; /* if < first_index_num => no indexes */

            int2 list_index_num; /* <0 => no list support */
            int2 autoid_index_num; /* <0 => no autoid  */
            uint4 fixedsize; /* 0 for dynamic classes */
            uint2 autoid_offset; /* - for compact or not; 0 if no autoid  */
            int2 history_index_num; /* -1 if no history index */
            uint2 history_length; /* 0 if no history index */
            uint2 history_offset; /* 0 if no history field, for compact or not*/
            int2 first_event_num; /* events numbers are sequential */
            int2 last_event_num; /* if < last_event_num => no events */
            uint2 flags; /* MCO_DB_TYPINFO_...  */
            MCO_DICT_CONST mco_dict_struct_t * struct_ptr; /* layout & members info */
            mco_size_t init_size; /* initial allocation size */
            uint2 auto_oid_offset; /* HA support */
            uint2 reserved; /* 4-align */
        } mco_dict_class_info_t;


        typedef struct mco_dict_event_t_
        {
            uint2 class_code;
            uint2 flags; /* MCO_DB_EVENT_T_... */
            mco_offs32_t field_offset; /* for update event */
            uint4 field_size; /* of the field, 0 when statically unknown  */
            uint1 field_type; /* MCO_DB_FLDTYPE_E */
            uint2 fld_no;
        } mco_dict_event_t;


        typedef struct mco_dictionary_t_
        {
            const char * MCO_DICT_CONST * str_class_names; /* debug mode only */
            const char * MCO_DICT_CONST * str_index_names; /* debug mode only */

            uint2 version_major;
            uint2 version_minor;
            uint2 version_build;
            uint2 magic_number;

            uint4 flags; /**< MCO_DICT_F_ */
            uint2 oid_is_supported;
            uint2 auto_oid_supported; /* HA support */

            /* counters: */
            uint2 n_class_codes; /* class_codes start from 1. n_class_codes == last_classcode */
            uint2 n_list_indexes; /* information only */
            uint2 n_autoid_indexes; /* information only */
            uint2 n_history_indexes; /* information only */
            uint2 n_userdef_indexes; /* total number of user-defined indices */
            uint2 max_numof_indexes_per_obj; /* for rollback segment estimation */
            uint2 n_structs; /* length of v_all_struct */
            uint2 pad;
            mco_counter32_t num_oid_estimation;
            mco_counter32_t num_HA_estimation; /* 0 if not HA mode */
            uint2 n_desc_indexes; /* numbered starting from 0 */
            uint2 n_desc_events;
            #ifdef MCO_CFG_COLLATION_SUPPORT
                uint2 n_desc_colls;  /* number of custom collations */
            #endif /* MCO_CFG_COLLATION_SUPPORT */

            uint2 exact_OID_sizeof;
            uint2 layout_OID_size;

            MCO_DICT_CONST mco_dict_class_info_t* v_class_info;
            MCO_DICT_CONST mco_dict_index_t* v_desc_indexes;
            MCO_DICT_CONST mco_dict_event_t* v_desc_events;
            MCO_DICT_CONST mco_dict_struct_t* v_all_struct;
            #ifdef MCO_CFG_COLLATION_SUPPORT
                MCO_DICT_CONST mco_dict_collation_t* v_desc_colls;
            #endif /* MCO_CFG_COLLATION_SUPPORT */

            /* datatype sizes. eXtremeDB runtime will use sizes below to check runtime, generated schema layout and application compatibilty */
            uint1 sizeof_mco_offs_t[2]; /* size of mco_offs_t data type, index 0 - schema compiler, index 1 - application */
            uint1 sizeof_mco_size_t[2]; /* size of mco_size_t data type, index 0 - schema compiler (not used now), index 1 - application */

            union {
                MCO_DICT_CONST int4         * ptr;
                mco_offs_t     offs;
            } init_i_data;
            uint4 init_i_data_n;
        
            union {
                MCO_DICT_CONST double       * ptr;
                mco_offs_t     offs;
            } init_d_data;
            uint4 init_d_data_n;
            uint4 class_code_origin;
        } mco_dictionary_t, * mco_dict_h;


        #ifdef MCO_NO_FORWARDS

            typedef void* mco_objhandle_h;

        #else 

            struct mco_objhandle_t_;
            typedef struct mco_objhandle_t_* mco_objhandle_h;

        #endif 



        typedef struct mco_external_field_t_
        {
            uint1 field_type; /* MCO_DB_FLDTYPE_E */
            uint2 bit_len; /* field's len in bits, used with patricia index on scalar datatypes */
            mco_value_union_t v;
            const char* ptr;
        }
        mco_external_field_t, * mco_external_field_h;


        /* external key for search, used as mco_abstractkey_external_h */
        typedef mco_external_field_t* mco_external_key_h;

        typedef struct mco_pattern_policy_t_
        {
            char ignore_other_fields;
            char any_char;
            char one_char;
            nchar_t any_nchar;
            nchar_t one_nchar;
            #ifdef MCO_CFG_WCHAR_SUPPORT 
                wchar_t any_wchar;
                wchar_t one_wchar;
            #endif 

        } mco_pattern_policy_t;


        MCO_RET mco_w_new_obj_noid(mco_trans_h t, mco_size_t init_size, uint2 class_code, mco_objhandle_h hret);
        MCO_RET mco_w_new_obj_noid_ev(mco_trans_h t, mco_size_t init_size, uint2 class_code, mco_objhandle_h hret,
                                      uint2 ev_id);
        MCO_RET mco_w_new_obj_oid(mco_trans_h t, mco_size_t init_size, uint2 class_code, const void* oid, uint2
                                  oid_sizeof, mco_objhandle_h ret);
        MCO_RET mco_w_new_obj_oid_ev(mco_trans_h t, mco_size_t init_size, uint2 class_code, const void* oid, uint2
                                     oid_sizeof, mco_objhandle_h ret, uint2 ev_id);
        MCO_RET mco_w_obj_delete(mco_objhandle_h obj);
        MCO_RET mco_w_obj_delete_ev(mco_objhandle_h obj, uint2 ev_id);
        MCO_RET mco_w_obj_delete_all(mco_trans_h t, uint2 class_code);
        MCO_RET mco_w_obj_delete_all_ev(mco_trans_h t, uint2 class_code, uint2 ev_id);
        MCO_RET mco_w_obj_checkpoint(mco_objhandle_h obj);
        MCO_RET mco_w_obj_checkpoint_ev(mco_objhandle_h obj, uint2 ev_id);
        MCO_RET mco_w_oid_get(mco_objhandle_h obj, void* oid, uint2 oid_sizeof);
        MCO_RET mco_w_find_byoid(mco_trans_h t, const void* oid, mco_objhandle_h hret, uint2 class_code); /* class_code
        ignored if 0 */
        MCO_RET mco_w_obj_del_byoid(mco_trans_h t, const void* oid);
        MCO_RET mco_w_classcode_byoid(mco_trans_h t, const void* oid, uint2* ret);

        MCO_RET mco_w_cursor_get_index(mco_cursor_h c, uint2* index);
        MCO_RET mco_w_cursor_locate(mco_trans_h t, uint2 index_code, mco_objhandle_h obj, mco_cursor_h c);
        MCO_RET mco_w_cursor_compare(mco_trans_h t, mco_cursor_h c, mco_external_key_h ek, int* result);
        MCO_RET mco_w_cursor_extract_field(mco_trans_h t, mco_cursor_h c, uint2 key_no, void* buffer, mco_size32_t
                                           buffer_size, mco_size32_t* actual_length);

        MCO_RET mco_w_hash_find_scalar(mco_trans_h t, uint2 index_code, mco_external_key_h ek, mco_objhandle_h ret);
        MCO_RET mco_w_hash_find_vect(mco_trans_h t, uint2 index_code, mco_external_key_h ek, mco_objhandle_h ret,
                                     uint2* element_index);

        MCO_RET mco_w_tree_find_scalar(mco_trans_h t, uint2 index_code, mco_external_key_h ek, mco_objhandle_h ret);
        MCO_RET mco_w_tree_find_vect(mco_trans_h t, uint2 index_code, mco_external_key_h ek, mco_objhandle_h ret,
                                     uint2* element_index);

        MCO_RET mco_w_obj_from_cursor(mco_trans_h t, mco_cursor_h c, uint2 class_code, mco_objhandle_h obj);
        MCO_RET mco_w_list_cursor(mco_trans_h t, uint2 class_code, mco_cursor_h c);
        MCO_RET mco_w_index_cursor(mco_trans_h t, uint2 index_code, mco_cursor_h c);

        MCO_RET mco_w_version_next(mco_objhandle_h h, mco_objhandle_h res, uint2 index_code);
        MCO_RET mco_w_version_prev(mco_objhandle_h h, mco_objhandle_h res, uint2 index_code);

        MCO_RET mco_w_vector_size_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2* result);

        MCO_RET mco_w_v_struct_size_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2 length, int2 indexaware);
        MCO_RET mco_w_v_struct_e_erase2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2 index, int2 indexaware);
        MCO_RET mco_w_v_struct_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no,
                                      mco_objhandle_h ret, uint2 index, mco_offs32_t s_u_sz, mco_offs32_t s_c_sz, mco_offs32_t u_align, mco_offs32_t c_align, int2 indexaware);
        MCO_RET mco_w_v_struct_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, mco_objhandle_h ret, uint2 index);

        MCO_RET mco_w_v_chars_size_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                        length, uint2 elsizei, int2 indexaware);
        MCO_RET mco_w_v_chars_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2 index,
                                     uint2 el_size, char* dest, uint2 destsz);
        MCO_RET mco_w_v_chars_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2 index,
                                     uint2 el_size, const char* src, uint2 srcsz, int2 indexaware);

        MCO_RET mco_w_v_simple_size_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                         length, uint2 elsize, int2 indexaware);
        MCO_RET mco_w_v_simple_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                      index, uint2 elsize, void* dest);
        MCO_RET mco_w_v_simple_e_getrange2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                           startindex, uint2 elsize, uint2 numof_els, void* dest, short is_enum);
        MCO_RET mco_w_v_simple_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                      index, uint2 elsize, int2 indexaware, const void* from);
        MCO_RET mco_w_v_simple_e_putrange2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                           startindex, uint2 elsize, uint2 numof_els, int2 indexaware, const void*
                                           pfrom, short is_enum);

        MCO_RET mco_w_v_bit_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2 index, void* dest);
        MCO_RET mco_w_v_bit_e_getrange2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2 startindex, uint2 numof_els, void* dest0);
        MCO_RET mco_w_v_bit_e_putrange2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2 startindex, uint2 numof_els, short indexaware, const void* pfrom);
        MCO_RET mco_w_v_bit_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2 index, short indexaware, const void* from);
        MCO_RET mco_w_v_bit_size_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no,uint2 length);
        MCO_RET mco_w_v_bit_size_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no,uint2* length);

        MCO_RET mco_w_v_string_size_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                         length, int2 indexaware);
        MCO_RET mco_w_v_string_e_get_len2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                          index, uint2* retlen);
        MCO_RET mco_w_v_string_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                      index, char* dest, uint2 destsz, uint2* retlen);
        MCO_RET mco_w_v_string_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                      index, const char* src, uint2 srcsz, int2 indexaware);

        MCO_RET mco_w_v_nchars_size_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                         length, uint2 elsize, int2 indexaware);
        MCO_RET mco_w_v_nchars_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                      index, uint2 el_size, nchar_t* dest, uint2 destsz);
        MCO_RET mco_w_v_nchars_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                      index, uint2 el_size, const nchar_t* src, uint2 srcsz, int2 indexaware);

        MCO_RET mco_w_v_nstring_size_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                          length, int2 indexaware);
        MCO_RET mco_w_v_nstring_e_get_len2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                           index, uint2* retlen);
        MCO_RET mco_w_v_nstring_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                       index, nchar_t* dest, uint2 destsz, uint2* retlen);
        MCO_RET mco_w_v_nstring_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                       index, const nchar_t* src, uint2 srcsz, int2 indexaware);

#ifdef MCO_CFG_WCHAR_SUPPORT
        MCO_RET mco_w_v_wchars_size_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
            length, uint2 elsize, int2 indexaware);
        MCO_RET mco_w_v_wchars_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
            index, uint2 el_size, wchar_t* dest, uint2 destsz);
        MCO_RET mco_w_v_wchars_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
            index, uint2 el_size, const wchar_t* src, uint2 srcsz, int2 indexaware);
        
        MCO_RET mco_w_v_wstring_size_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
            length, int2 indexaware);
        MCO_RET mco_w_v_wstring_e_get_len2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
            index, uint2* retlen);
        MCO_RET mco_w_v_wstring_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
            index, wchar_t* dest, uint2 destsz, uint2* retlen);
        MCO_RET mco_w_v_wstring_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
            index, const wchar_t* src, uint2 srcsz, int2 indexaware);
#endif

        MCO_RET mco_w_va_struct_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no,
                                       mco_objhandle_h ret, uint2 index, mco_offs32_t s_u_aligned_sz, mco_offs32_t s_c_aligned_sz,
                                       int2 indexaware, uint2 len);
        MCO_RET mco_w_va_struct_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no,
                                       mco_objhandle_h ret, uint2 index, mco_offs32_t s_u_aligned_sz, mco_offs32_t s_c_aligned_sz,
                                       uint2 len);

        MCO_RET mco_w_va_chars_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                      index, uint2 el_size, char* dest, uint2 destsz, uint2 len);
        MCO_RET mco_w_va_chars_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                      index, uint2 el_size, const char* src, uint2 srcsz, int2 indexaware, uint2 len);

        MCO_RET mco_w_va_nchars_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                       index, uint2 el_size, nchar_t* dest, uint2 destsz, uint2 len);
        MCO_RET mco_w_va_nchars_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                       index, uint2 el_size, const nchar_t* src, uint2 srcsz, int2 indexaware, uint2 len);

        MCO_RET mco_w_va_simple_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                       index, uint2 elsize, void* dest, uint2 len);
        MCO_RET mco_w_va_simple_e_getrange2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no,
                                            uint2 startindex, uint2 elsize, uint2 numof_els, void* dest, uint2 len, short is_enum);
        MCO_RET mco_w_va_simple_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                       index, uint2 elsize, int2 indexaware, const void* src, uint2 len);
        MCO_RET mco_w_va_simple_e_putrange2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no,
                                            uint2 startindex, uint2 elsize, uint2 numof_els, int2 indexaware, const
                                            void* pfrom, uint2 len, short is_enum);

        MCO_RET mco_w_va_bit_e_putrange2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2 startindex,
                                         uint2 numof_els, short indexaware, const void* pfrom, uint2 len);
        MCO_RET mco_w_va_bit_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2 index, short
                                    indexaware, const void* from, uint2 len);
        MCO_RET mco_w_va_bit_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset,uint2 fld_no, uint2 index, void* dest,
                                    uint2 len);
        MCO_RET mco_w_va_bit_e_getrange2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2 startindex,
                                         uint2 numof_els, void* dest0, uint2 len);

        MCO_RET mco_w_va_string_e_get_len2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                           index, uint2* retlen, uint2 len);
        MCO_RET mco_w_va_string_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                       index, char* dest, uint2 destsz, uint2* retlen, uint2 len);
        MCO_RET mco_w_va_string_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                       index, const char* src, uint2 srcsz, int2 indexaware, uint2 len);

        MCO_RET mco_w_va_nstring_e_get_len2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no,
                                            uint2 index, uint2* retlen, uint2 len);
        MCO_RET mco_w_va_nstring_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                        index, nchar_t* dest, uint2 destsz, uint2* retlen, uint2 len);
        MCO_RET mco_w_va_nstring_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
                                        index, const nchar_t* src, uint2 srcsz, int2 indexaware, uint2 len);

#ifdef MCO_CFG_WCHAR_SUPPORT
        MCO_RET mco_w_va_wchars_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
            index, uint2 el_size, wchar_t* dest, uint2 destsz, uint2 len);
        MCO_RET mco_w_va_wchars_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
            index, uint2 el_size, const wchar_t* src, uint2 srcsz, int2 indexaware, uint2
            len);

        MCO_RET mco_w_va_wstring_e_get_len2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset,
            uint2 fld_no, uint2 index, uint2* retlen, uint2 len);
        MCO_RET mco_w_va_wstring_e_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
            index, wchar_t* dest, uint2 destsz, uint2* retlen, uint2 len);
        MCO_RET mco_w_va_wstring_e_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2
            index, const wchar_t* src, uint2 srcsz, int2 indexaware, uint2 len);

        MCO_RET mco_w_wchars_put(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, const wchar_t* src, uint2 src_size, uint2
            fldsize, int2 indexaware);
        MCO_RET mco_w_wchars_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const wchar_t*
            src, uint2 src_size, uint2 fldsize, int2 indexaware);
        MCO_RET mco_w_wchars_put_ev(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, const wchar_t* src, uint2 src_size,
            uint2 fldsize, int2 indexaware, uint2 ev_id);
        MCO_RET mco_w_wchars_put_ev2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const
            wchar_t* src, uint2 src_size, uint2 fldsize, int2 indexaware, uint2 ev_id);
        MCO_RET mco_w_wchars_get(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, wchar_t* dest, uint2 dest_size, uint2
            fldsize);
        MCO_RET mco_w_wchars_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, wchar_t* dest,
            uint2 dest_size, uint2 fldsize);
        
        MCO_RET mco_w_wstring_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, wchar_t* dest,
            uint2 dest_size, uint2* retlen);
        MCO_RET mco_w_wstring_len2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2* ret);
        MCO_RET mco_w_wstring_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const
            wchar_t* src, uint2 nbytes, int2 indexaware);
        MCO_RET mco_w_wstring_put_ev2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const
            wchar_t* src, uint2 nbytes, int2 indexaware, uint2 ev_id);
#endif

        MCO_RET mco_w_tree_find_all(mco_trans_h t, mco_cursor_h c, int n_keywords, mco_search_kwd_t* keywords);
        MCO_RET mco_w_tree_find_any(mco_trans_h t, mco_cursor_h c, int n_keywords, mco_search_kwd_t* keywords);
        MCO_RET mco_w_tree_find(mco_trans_h t, mco_cursor_h c, MCO_OPCODE op, mco_external_key_h ek);
        MCO_RET mco_w_tree_find(mco_trans_h t, mco_cursor_h c, MCO_OPCODE op, mco_external_key_h ek);
        MCO_RET mco_w_hash_find(mco_trans_h t, mco_cursor_h c, mco_external_key_h ek);
        MCO_RET mco_w_kdtree_find(mco_trans_h t, mco_cursor_h c, MCO_Hf * range_start, MCO_Hf * range_end );

        MCO_RET mco_w_rect_get(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no,uint2
			elsize, uint2 numof_els, void* dest);
        MCO_RET mco_w_rect_put(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no,uint2
			eltype, uint2 elsize, uint2 numof_els, int2 indexaware, const void* pfrom);

        MCO_RET mco_w_point_get(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no,uint2
			elsize, uint2 numof_els, void* dest);
        MCO_RET mco_w_point_put(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no,uint2
			eltype, uint2 elsize, uint2 numof_els, int2 indexaware, const void* pfrom);

        MCO_RET mco_w_b1_put(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, int2 indexaware, uint1 val);
        MCO_RET mco_w_b1_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, int2 indexaware, uint1 val);
        MCO_RET mco_w_b1_put_ev(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, int2 indexaware, uint1 val, uint2 ev_id);
        MCO_RET mco_w_b1_put_ev2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, int2 indexaware, uint1 val, uint2 ev_id);
        MCO_RET mco_w_b1_get(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, void* p);
        MCO_RET mco_w_b1_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, void* p);

        MCO_RET mco_w_bit_get2(mco_objhandle_h obj,  mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, void* p);
        MCO_RET mco_w_bit_put2(mco_objhandle_h obj,  mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, int2 indexaware, uint1 v);
        MCO_RET mco_w_bit_put_ev2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, int2 indexaware, uint1 val, uint2 ev_id);

        MCO_RET mco_w_b2_put(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, int2 indexaware, uint2 val);
        MCO_RET mco_w_b2_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, int2 indexaware, uint2 val);
        MCO_RET mco_w_b2_put_ev(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, int2 indexaware, uint2 val, uint2 ev_id);
        MCO_RET mco_w_b2_put_ev2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, int2 indexaware, uint2 val, uint2 ev_id);
        MCO_RET mco_w_b2_get(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, void* p);
        MCO_RET mco_w_b2_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, void* p);

        MCO_RET mco_w_b4_put(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, int2 indexaware, uint4 val);
        MCO_RET mco_w_b4_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, int2 indexaware, uint4 val);
        MCO_RET mco_w_b4_put_ev(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, int2 indexaware, uint4 val, uint2 ev_id);
        MCO_RET mco_w_b4_put_ev2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, int2 indexaware, uint4 val, uint2 ev_id);
        MCO_RET mco_w_b4_get(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, void* p);
        MCO_RET mco_w_b4_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, void* p);

        MCO_RET mco_w_b8_put(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, int2 indexaware, const void* p);
        MCO_RET mco_w_b8_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, int2 indexaware, const void* p);
        MCO_RET mco_w_b8_put_ev(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, int2 indexaware, const void* p, uint2 ev_id) ;
        MCO_RET mco_w_b8_put_ev2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, int2 indexaware, const void* p, uint2 ev_id);
        MCO_RET mco_w_b8_get(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, void* p);
        MCO_RET mco_w_b8_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, void* p);

        MCO_RET mco_w_chars_put(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, const char* src, uint2 src_size, uint2 fldsize, int2 indexaware);
        MCO_RET mco_w_chars_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const char* src,
                                 uint2 src_size, uint2 fldsize, int2 indexaware);
        MCO_RET mco_w_chars_put_ev(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, const char* src, uint2 src_size, uint2
                                   fldsize, int2 indexaware, uint2 ev_id);
        MCO_RET mco_w_chars_put_ev2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const char*
                                    src, uint2 src_size, uint2 fldsize, int2 indexaware, uint2 ev_id);
        MCO_RET mco_w_chars_get(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, char* dest, uint2 dest_size, uint2 fldsize);
        MCO_RET mco_w_chars_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, char* dest,
                                 uint2 dest_size, uint2 fldsize);

        MCO_RET mco_w_string_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, char* dest,
                                  uint2 dest_size, uint2* retlen);
        MCO_RET mco_w_string_len2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2* ret);
        MCO_RET mco_w_string_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const char*
                                  src, uint2 nbytes, int2 indexaware);
        MCO_RET mco_w_string_put_ev2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const char*
                                     src, uint2 nbytes, int2 indexaware, uint2 ev_id);

        MCO_RET mco_w_nchars_put(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, const nchar_t* src, uint2 src_size, uint2
                                 fldsize, int2 indexaware);
        MCO_RET mco_w_nchars_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const nchar_t*
                                  src, uint2 src_size, uint2 fldsize, int2 indexaware);
        MCO_RET mco_w_nchars_put_ev(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, const nchar_t* src, uint2 src_size,
                                    uint2 fldsize, int2 indexaware, uint2 ev_id);
        MCO_RET mco_w_nchars_put_ev2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const
                                     nchar_t* src, uint2 src_size, uint2 fldsize, int2 indexaware, uint2 ev_id);
        MCO_RET mco_w_nchars_get(mco_objhandle_h obj, mco_offs32_t fld_offset, uint2 fld_no, nchar_t* dest, uint2 dest_size, uint2 fldsize);
        MCO_RET mco_w_nchars_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, nchar_t* dest,
                                  uint2 dest_size, uint2 fldsize);

        MCO_RET mco_w_nstring_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, nchar_t* dest,
                                   uint2 dest_size, uint2* retlen);
        MCO_RET mco_w_nstring_len2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, uint2* ret);
        MCO_RET mco_w_nstring_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const
                                   nchar_t* src, uint2 nbytes, int2 indexaware);
        MCO_RET mco_w_nstring_put_ev2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const
                                      nchar_t* src, uint2 nbytes, int2 indexaware, uint2 ev_id);

        MCO_RET mco_w_direct_struct_get_ptr(mco_objhandle_h obj, void** ptr, mco_offs32_t size);
        MCO_RET mco_w_struct_get2(mco_objhandle_h obj, mco_objhandle_h ret, mco_offs32_t fld_u_offset, mco_offs32_t
                                  fld_c_offset, uint2 fld_no);
        MCO_RET mco_w_struct_put2(mco_objhandle_h obj, mco_objhandle_h ret, mco_offs32_t fld_u_offset, mco_offs32_t
                                  fld_c_offset, uint2 fld_no, int2 indexaware);

        MCO_RET mco_w_blob_get2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, mco_offs_t
                                startOffset, char* dest, mco_size32_t dest_size, mco_size32_t* retlen);
        MCO_RET mco_w_blob_len2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, mco_size32_t*
                                result);
        MCO_RET mco_w_blob_put2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const char* src,
                                mco_size32_t nbytes);
        MCO_RET mco_w_blob_append2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, const char*
                                   src, mco_size32_t nbytes);

        MCO_RET mco_w_optional_get2(mco_objhandle_h obj, mco_objhandle_h ret, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no);
        MCO_RET mco_w_optional_put2(mco_objhandle_h obj, mco_objhandle_h ret, int struct_num, mco_offs32_t fld_u_offset,
                                    mco_offs32_t fld_c_offset, uint2 fld_no, mco_offs32_t struct_u_size, mco_offs32_t struct_c_size, mco_offs32_t u_align,
                                    mco_offs32_t c_align);
        MCO_RET mco_w_optional_erase2(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no);

        MCO_RET mco_w_voluntary_drop(mco_trans_h t, uint2 index_code);
        MCO_RET mco_w_voluntary_create(mco_trans_h t, uint2 index_code);

        MCO_RET mco_w_compact(mco_objhandle_h obj, uint2 class_code, int struct_num, mco_counter32_t* pages_released);

        typedef MCO_RET(*mco_generic_ev_handler)(mco_trans_h t, mco_objhandle_h obj, MCO_EVENT_TYPE et, void* param);
        typedef MCO_RET(*MCO_update_class_ev_handler)(mco_trans_h t, mco_objhandle_h obj, int class_code, const char* field_name,mco_dict_type_t  field_type,const void * value, void* param);

        MCO_RET  mco_get_fieldname_by_fldno( mco_objhandle_h handle, uint2 fld_no, char * dest, uint2 dest_size,  uint2 * len );

        MCO_RET mco_ev_h_register(mco_trans_h t, mco_generic_ev_handler h, uint2 event_id, void* param, uint2
                                  before_upd);
        MCO_RET mco_ev_h_unregister(mco_trans_h t, mco_generic_ev_handler h, uint2 event_id);

        MCO_RET mco_w_xml_get_object(mco_objhandle_h obj, void* stream_handle, mco_stream_write o_stream, uint2
                                     class_code);
        MCO_RET mco_w_xml_put_object(mco_objhandle_h obj, const char* xml, uint2 class_code);
        MCO_RET mco_w_xml_create_object(mco_trans_h t, const char* xml, mco_objhandle_h obj, uint2 class_code);
        MCO_RET mco_w_xml_schema(mco_trans_h t, void* stream_handle, mco_stream_write o_stream, uint2 class_code);

        MCO_RET mco_w_json_get_object(mco_objhandle_h obj, void* stream_handle, mco_stream_write o_stream, uint2
                                      class_code);

        /* returns MCO_YES if the handles point to the same object */
        mco_bool mco_objects_eq(OBJH1(p1), OBJH1(p2));

        /* Pattern match */

        MCO_RET mco_pattern_size(mco_external_field_h fields, uint4* psize);

        MCO_RET mco_pattern_search(mco_trans_h transaction, mco_cursor_h c, void* pattern_mem, mco_size_t memsize,
                                   mco_external_field_h fields);

        MCO_RET mco_pattern_next(mco_trans_h transaction, mco_cursor_h c, void* pattern_mem, uint2 is_first_search);

        void mco_get_default_pattern_policy(mco_pattern_policy_t* p);
        MCO_RET mco_get_pattern_policy(mco_trans_h t, mco_pattern_policy_t* p);
        MCO_RET mco_set_pattern_policy(mco_trans_h t, const mco_pattern_policy_t* p);
    
        MCO_RET mco_w_normalize_handle(mco_objhandle_h obj);
            
        MCO_RET mco_set_class_allocation_block_size(mco_trans_h t, int class_code, mco_size_t block_size);
        MCO_RET mco_set_class_caching_priority(mco_trans_h t, int class_code, int priority);

        MCO_RET mco_collate_get_char(mco_collate_h c, char *dest, uint2 len);
        MCO_RET mco_collate_get_char_range(mco_collate_h c, char *dest, uint2 from, uint2 len);
        MCO_RET mco_collate_get_nchar(mco_collate_h c, nchar_t *dest, uint2 len);
        MCO_RET mco_collate_get_nchar_range(mco_collate_h c, nchar_t *dest, uint2 from, uint2 len);
#ifdef MCO_CFG_WCHAR_SUPPORT
        MCO_RET mco_collate_get_wchar(mco_collate_h c, wchar_t *dest, uint2 len);
        MCO_RET mco_collate_get_wchar_range(mco_collate_h c, wchar_t *dest, uint2 from, uint2 len);
#endif

        MCO_RET mco_w_numeric_b1_to_string(int1 val, int precision, char* buf, int buf_size);
        MCO_RET mco_w_numeric_b2_to_string(int2 val, int precision, char* buf, int buf_size);
        MCO_RET mco_w_numeric_b4_to_string(int4 val, int precision, char* buf, int buf_size);
        MCO_RET mco_w_numeric_b8_to_string(mco_int8 val, int precision, char* buf, int buf_size);

        MCO_RET mco_w_numeric_b1_from_string(int1* val, int precision, char const* buf);
        MCO_RET mco_w_numeric_b2_from_string(int2* val, int precision, char const* buf);
        MCO_RET mco_w_numeric_b4_from_string(int4* val, int precision, char const* buf);
        MCO_RET mco_w_numeric_b8_from_string(mco_int8* val, int precision, char const* buf);

#define MCO_SEQ_PROTOTYPES(TYPE)                            \
    MCOSEQ_API MCO_RET mco_w_seq_first_##TYPE(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_##TYPE* val);  \
    MCOSEQ_API MCO_RET mco_w_seq_last_##TYPE(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_##TYPE* val);  \
    MCOSEQ_API MCO_RET mco_w_seq_append_##TYPE(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, mco_##TYPE const* vals, mco_size_t n_items, mco_seq_order_t order, mco_bitmap_word_t const* null_bitmap); \
    MCOSEQ_API MCO_RET mco_w_seq_insert_##TYPE(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, mco_seq_no_t pos, mco_##TYPE const* vals, mco_size_t n_items, mco_seq_order_t order, mco_bitmap_word_t const* null_bitmap); \
    MCOSEQ_API MCO_RET mco_w_seq_search_##TYPE(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_seq_iterator_h iterator, mco_##TYPE low, mco_seq_boundary_kind_t low_boundbary, mco_##TYPE high, mco_seq_boundary_kind_t high_boundbary, mco_seq_order_t order) ; \
    MCOSEQ_API MCO_RET mco_w_seq_join_##TYPE(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_seq_iterator_h result, mco_seq_iterator_h join_with, mco_seq_order_t order) ; \
    MCOSEQ_API MCO_RET mco_w_seq_delete_##TYPE(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, mco_seq_no_t from, mco_seq_no_t till, mco_seq_order_t order); \
    MCOSEQ_API MCO_RET mco_w_seq_subseq_##TYPE(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_seq_iterator_h iterator, mco_seq_no_t from, mco_seq_no_t till) ; \
    MCOSEQ_API MCO_RET mco_w_seq_map_##TYPE(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_seq_iterator_h iterator, mco_seq_iterator_h map_iterator) ; \
    MCOSEQ_API MCO_RET mco_w_seq_from_cursor_##TYPE(mco_trans_h trans, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 class_code, mco_seq_iterator_h iterator, mco_cursor_h cursor) ; \
    MCOSEQ_API MCO_RET mco_w_seq_project_##TYPE(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_seq_iterator_h dst, mco_seq_iterator_h src); \
    MCOSEQ_API MCO_RET mco_w_seq_store_##TYPE(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, mco_seq_iterator_h src, mco_seq_order_t order); \
    MCOSEQ_API MCO_RET mco_w_seq_sequence_##TYPE(mco_trans_h trans, uint2 class_code, mco_offs32_t fld_offset, mco_seq_iterator_h seq)

MCOSEQ_API MCO_RET mco_w_seq_count(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_seq_no_t* count);
MCOSEQ_API MCO_RET mco_w_seq_truncate(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no);
 

MCO_SEQ_PROTOTYPES(int1);
MCO_SEQ_PROTOTYPES(int2);
MCO_SEQ_PROTOTYPES(int4);
MCO_SEQ_PROTOTYPES(int8);
MCO_SEQ_PROTOTYPES(uint1);
MCO_SEQ_PROTOTYPES(uint2);
MCO_SEQ_PROTOTYPES(uint4);
MCO_SEQ_PROTOTYPES(uint8);
MCO_SEQ_PROTOTYPES(float);
MCO_SEQ_PROTOTYPES(double);
MCO_SEQ_PROTOTYPES(datetime);

#define mco_w_seq_first_mco_time  mco_w_seq_first_uint4 
#define mco_w_seq_last_mco_time   mco_w_seq_last_uint4 
#define mco_w_seq_append_mco_time mco_w_seq_append_uint4 
#define mco_w_seq_insert_mco_time mco_w_seq_insert_uint4
#define mco_w_seq_delete_mco_time mco_w_seq_delete_uint4
#define mco_w_seq_search_mco_time mco_w_seq_search_uint4
#define mco_w_seq_join_mco_time   mco_w_seq_join_uint4
#define mco_w_seq_subseq_mco_time mco_w_seq_subseq_uint4
#define mco_w_seq_map_mco_time    mco_w_seq_map_uint4
#define mco_w_seq_from_cursor_mco_time mco_w_seq_from_cursor_uint4
#define mco_w_seq_project_mco_time mco_w_seq_project_uint4
#define mco_w_seq_store_mco_time  mco_w_seq_store_uint4

#define mco_w_seq_first_mco_date  mco_w_seq_first_uint4 
#define mco_w_seq_last_mco_date   mco_w_seq_last_uint4 
#define mco_w_seq_append_mco_date mco_w_seq_append_uint4 
#define mco_w_seq_insert_mco_date mco_w_seq_insert_uint4
#define mco_w_seq_delete_mco_date mco_w_seq_delete_uint4
#define mco_w_seq_search_mco_date mco_w_seq_search_uint4
#define mco_w_seq_join_mco_date   mco_w_seq_join_uint4
#define mco_w_seq_subseq_mco_date mco_w_seq_subseq_uint4
#define mco_w_seq_map_mco_date    mco_w_seq_map_uint4
#define mco_w_seq_from_cursor_mco_date mco_w_seq_from_cursor_uint4
#define mco_w_seq_project_mco_date mco_w_seq_project_uint4
#define mco_w_seq_store_mco_date mco_w_seq_store_uint4

#define mco_w_seq_first_mco_int8  mco_w_seq_first_int8 
#define mco_w_seq_last_mco_int8   mco_w_seq_last_int8 
#define mco_w_seq_append_mco_int8 mco_w_seq_append_int8 
#define mco_w_seq_insert_mco_int8 mco_w_seq_insert_int8
#define mco_w_seq_delete_mco_int8 mco_w_seq_delete_int8
#define mco_w_seq_search_mco_int8 mco_w_seq_search_int8
#define mco_w_seq_join_mco_int8   mco_w_seq_join_int8
#define mco_w_seq_subseq_mco_int8 mco_w_seq_subseq_int8
#define mco_w_seq_map_mco_int8    mco_w_seq_map_int8
#define mco_w_seq_from_cursor_mco_int8 mco_w_seq_from_cursor_int8
#define mco_w_seq_project_mco_int8 mco_w_seq_project_int8
#define mco_w_seq_store_mco_int8 mco_w_seq_store_int8

    MCOSEQ_API MCO_RET mco_w_seq_first_char(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, char* val, mco_size_t size);    
    MCOSEQ_API MCO_RET mco_w_seq_last_char(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, char* val, mco_size_t size); 
    MCOSEQ_API MCO_RET mco_w_seq_append_char(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, char const* vals, mco_size_t n_items, mco_seq_order_t order, mco_size_t size, mco_bitmap_word_t const* null_bitmap); 
    MCOSEQ_API MCO_RET mco_w_seq_insert_char(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, mco_seq_no_t pos, char const* vals, mco_size_t n_items, mco_seq_order_t order, mco_size_t size, mco_bitmap_word_t const* null_bitmap); 
    MCOSEQ_API MCO_RET mco_w_seq_search_char(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_seq_iterator_h iterator, char* low, mco_seq_boundary_kind_t low_boundbary, char* high, mco_seq_boundary_kind_t high_boundbary, mco_seq_order_t order, mco_size_t size); 
    MCOSEQ_API MCO_RET mco_w_seq_join_char(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_seq_iterator_h result, mco_seq_iterator_h join_with, mco_seq_order_t order); 
    MCOSEQ_API MCO_RET mco_w_seq_delete_char(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, mco_seq_no_t from, mco_seq_no_t till, mco_seq_order_t order, mco_size_t size); 
    MCOSEQ_API MCO_RET mco_w_seq_subseq_char(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_seq_iterator_h iterator, mco_seq_no_t from, mco_seq_no_t till, mco_size_t size) ; 
    MCOSEQ_API MCO_RET mco_w_seq_map_char(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_seq_iterator_h iterator, mco_seq_iterator_h map_iterator, mco_size_t size) ; 
    MCOSEQ_API MCO_RET mco_w_seq_from_cursor_char(mco_trans_h trans, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 class_code, mco_seq_iterator_h iterator, mco_cursor_h cursor, mco_size_t size) ; 
    MCOSEQ_API MCO_RET mco_w_seq_project_char(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, mco_seq_iterator_h dst, mco_seq_iterator_h src, mco_size_t size); 
    MCOSEQ_API MCO_RET mco_w_seq_store_char(mco_objhandle_h obj, mco_offs32_t fld_u_offset, mco_offs32_t fld_c_offset, uint2 fld_no, mco_seq_iterator_h src, mco_seq_order_t order, mco_size_t size);

        #ifdef __cplusplus
        }
    #endif 

#endif

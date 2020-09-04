/****************************************************************
 *                                                              *
 *  mcoseq.h                                                    *
 *                                                              *
 *  This file is a part of the eXtremeDB source code.           *
 *                                                              *
 *  Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.  *
 *                                                              *
 *  Sequence API                                                *
 *                                                              *
 ****************************************************************/
#ifndef MCO_SEQ_H__
#define MCO_SEQ_H__

#include "mco.h"

#ifdef _WIN32
#  ifdef MCOSEQ_DLL
#    define MCOSEQ_API __declspec(dllexport)
#  else
#    ifdef MCOSEQ_USE_DLL
#       define MCOSEQ_API __declspec(dllimport)
#    else
#       define MCOSEQ_API 
#    endif
#  endif
#else
#  define MCOSEQ_API
#endif

#ifdef _WIN32
#  ifdef MCOSEQMATH_DLL
#    define MCOSEQMATH_API __declspec(dllexport)
#  else
#    ifdef MCOSEQMATH_USE_DLL
#       define MCOSEQMATH_API __declspec(dllimport)
#    else
#       define MCOSEQMATH_API 
#    endif
#  endif
#else
#  define MCOSEQMATH_API
#endif



#ifdef __cplusplus
extern "C"
{
#endif

    typedef uint8 mco_seq_no_t;
    typedef uint1 mco_seq_bool;
    typedef mco_seq_bool mco_mco_seq_bool; /* AIX */

    /**
     * Macros for construction simple date with efficient extraction of components
     */
    #define DMY(_day,_month,_year) (((_year) * 10000) + ((_month) * 100) + (_day))
    #define DAY(_date) ((_date) % 100)
    #define MONTH(_date) (((_date) / 100) % 100)
    #define YEAR(_date) ((_date) / 10000)

    typedef enum mco_seq_order_t_
    {
        MCO_SEQ_NO_ORDER,
        MCO_SEQ_ASC_ORDER,
        MCO_SEQ_DESC_ORDER
    } mco_seq_order_t;

    #define MCO_SEQ_INFINITY ((mco_seq_no_t)-1)

    #ifndef MCO_SEQ_TILE_SIZE
    #define MCO_SEQ_TILE_SIZE 128
    #endif

    #ifndef MCO_SEQ_MAX_CHAR_SIZE
    #define MCO_SEQ_MAX_CHAR_SIZE 32
    #endif
    #define MCO_SEQ_MAX_SCALAR_SIZE 8

    #define MCO_SEQ_TILE_SIZE_BYTES MCO_SEQ_TILE_SIZE*MCO_SEQ_MAX_CHAR_SIZE

	#define MCO_SEQ_NULL_BITMAP_SIZE  ((MCO_SEQ_TILE_SIZE+63)/64)
	#define MCO_SEQ_NULL_BITMAP_BYTES (MCO_SEQ_NULL_BITMAP_SIZE*8)

	typedef mco_uint8 mco_bitmap_word_t;

    typedef enum mco_seq_boundary_kind_t_
    {
        MCO_SEQ_BOUNDARY_OPEN,
        MCO_SEQ_BOUNDARY_INCLUSIVE,
        MCO_SEQ_BOUNDARY_EXCLUSIVE
    } mco_seq_boundary_kind_t;

    typedef union mco_seq_payload_t_
    {
        mco_seq_bool arr_mco_seq_bool[MCO_SEQ_TILE_SIZE_BYTES/sizeof(mco_seq_bool)];
        char arr_char[MCO_SEQ_TILE_SIZE_BYTES];
        int1 arr_int1[MCO_SEQ_TILE_SIZE_BYTES];
        int2 arr_int2[MCO_SEQ_TILE_SIZE_BYTES/sizeof(int2)];
        int4 arr_int4[MCO_SEQ_TILE_SIZE_BYTES/sizeof(int4)];
        mco_int8 arr_int8[MCO_SEQ_TILE_SIZE_BYTES/sizeof(mco_int8)];
        uint1 arr_uint1[MCO_SEQ_TILE_SIZE_BYTES/sizeof(uint1)];
        uint2 arr_uint2[MCO_SEQ_TILE_SIZE_BYTES/sizeof(uint2)];
        uint4 arr_uint4[MCO_SEQ_TILE_SIZE_BYTES/sizeof(uint4)];
        uint8 arr_uint8[MCO_SEQ_TILE_SIZE_BYTES/sizeof(uint8)];
        mco_time arr_mco_time[MCO_SEQ_TILE_SIZE_BYTES/sizeof(mco_time)];
        mco_date arr_mco_date[MCO_SEQ_TILE_SIZE_BYTES/sizeof(mco_date)];
        mco_datetime arr_datetime[MCO_SEQ_TILE_SIZE_BYTES/sizeof(mco_datetime)];
        float arr_float[MCO_SEQ_TILE_SIZE_BYTES/sizeof(float)];
        double arr_double[MCO_SEQ_TILE_SIZE_BYTES/sizeof(double)];
    } mco_seq_payload_t MCO_ALIGN_16;

	typedef struct mco_seq_tile_t_
	{
		mco_bitmap_word_t null_bitmap[MCO_SEQ_NULL_BITMAP_SIZE];
		mco_seq_payload_t u;
	} mco_seq_tile_t MCO_ALIGN_16;

    struct mco_seq_iterator_t_;
    typedef MCO_RET(*mco_seq_iterator_next_t)(struct mco_seq_iterator_t_* iterator);
    typedef MCO_RET(*mco_seq_iterator_reset_t)(struct mco_seq_iterator_t_* iterator);
    typedef MCO_RET(*mco_seq_iterator_prepare_t)(struct mco_seq_iterator_t_* iterator);
    typedef MCO_RET(*mco_seq_iterator_merge_t)(struct mco_seq_iterator_t_* dst, struct mco_seq_iterator_t_* src);

    #define MCO_SEQ_ITERATOR_CTX_SIZE 280

    /*
     * Sequence iterator: it is used not only for iteration but for constructing pipe of different operations with sequence
     */
    typedef struct mco_seq_iterator_t_
    {
        mco_seq_iterator_next_t next; /* method for obtaning next portion of values */
        mco_seq_iterator_reset_t reset; /* start iteration from beginning */
        mco_seq_iterator_prepare_t prepare; /* prepare iterator (used to start parallel processing) */
        mco_seq_iterator_merge_t merge; /* merge two iterators (used to merge results of parallel processing) */
        uint2 elem_size; /* size of element */
        uint2 tile_size; /* number of tile items */
        uint2 tile_offs; /* offset to first not handled tile item */
        uint1 rle_offs;  /* index within same RLE value */
        uint1 prepared;  /* flag set by prepare function */
        mco_seq_no_t first_seq_no; /* first sequence number (inclusive) */
        mco_seq_no_t next_seq_no;  /* sequence number of element returned by sudsequent invocation of next() function */
        mco_seq_no_t last_seq_no;  /* last sequence number (inclusive) */
        struct mco_seq_iterator_t_* opd[3]; /*operands of sequence operator */
        void* buf; /* memory buffer used by operators requiring temporary memory (reverse, order_by) */
        mco_size_t buf_size; /* size of buffer */
        uint1 elem_type; /* result element type MCO_DD_... */
        uint1 bounded; /* iterator is used in expression */
        uint1 createdBySql; /* iterator was created by SQL */
        uint8 context[MCO_SEQ_ITERATOR_CTX_SIZE/8]; /* operator specific context */
        mco_seq_tile_t tile;  /* tile of values */
    } mco_seq_iterator_t, *mco_seq_iterator_h;

    typedef uint1 mco_seq_rle_count_t;
    #define MCO_SEQ_RLE_COUNT(iterator, i) (iterator->tile.u.arr_uint1[sizeof(mco_seq_payload_t) - (i) - 1])
    #define MCO_SEQ_RLE_CURR_COUNT(iterator) (iterator->tile.u.arr_uint1[sizeof(mco_seq_payload_t) - iterator->tile_offs - 1])
    #define MCO_SEQ_RLE_TILE_SIZE(type) (sizeof(mco_seq_payload_t) / (sizeof(mco_##type)+sizeof(mco_seq_rle_count_t)))
    #define MCO_SEQ_RLE_CHAR_TILE_SIZE(size) (sizeof(mco_seq_payload_t) / ((size)+sizeof(mco_seq_rle_count_t)))

    #define MCO_SEQ_RLE_NEXT(iterator, inc)                             \
        if ((iterator->rle_offs += inc) == MCO_SEQ_RLE_CURR_COUNT(iterator)) { \
            iterator->rle_offs = 0;                                     \
            iterator->tile_offs += 1;                                   \
        }

	#define MCO_SEQ_IS_NULL(iterator, i)  ((iterator->tile.null_bitmap[(i) >> 6] & (1LL << ((i) & 63))) != 0)
	#define MCO_SEQ_SET_NULL(iterator, i) iterator->tile.null_bitmap[(i) >> 6] |= (1LL << ((i) & 63))
	#define MCO_SEQ_SET_BIT(dst, dst_bit, src, src_bit)  dst->tile.null_bitmap[(dst_bit) >> 6] |= ((src->tile.null_bitmap[(src_bit) >> 6] >> ((src_bit) & 63)) & 1) << ((dst_bit) & 63)

    /*
     * Iterator through sequence iterators
     */
    struct mco_seq_meta_iterator_t_;
    typedef MCO_RET(*mco_seq_meta_iterator_next_t)(struct mco_seq_meta_iterator_t_* meta_iterator);
    typedef MCO_RET(*mco_seq_meta_iterator_first_t)(struct mco_seq_meta_iterator_t_* meta_iterator);

    typedef struct mco_seq_meta_iterator_t_ {
        mco_seq_meta_iterator_next_t next;
        mco_seq_meta_iterator_first_t first;
        mco_seq_iterator_t iterator; /* iterator inialized by call to next */
        void* user_data; /* user context can be provided either through this field or by creating derived structure */
    } mco_seq_meta_iterator_t, *mco_seq_meta_iterator_h;

    /* Functions defined for all scalar types */
    #define MCO_SEQ_FUNC_DECL(TYPE) \
        typedef mco_##TYPE(*mco_seq_func_ctx_##TYPE##_ptr_t)(mco_##TYPE arg, void* ctx);          \
        typedef mco_##TYPE(*mco_seq_func2_ctx_##TYPE##_ptr_t)(mco_##TYPE arg1, mco_##TYPE arg2, void* ctx); \
        typedef mco_seq_bool(*mco_seq_cond_ctx_##TYPE##_ptr_t)(mco_##TYPE arg, void* ctx); \
        typedef mco_##TYPE(*mco_seq_func_##TYPE##_ptr_t)(mco_##TYPE arg);          \
        typedef mco_##TYPE(*mco_seq_func2_##TYPE##_ptr_t)(mco_##TYPE arg1, mco_##TYPE arg2); \
        typedef mco_seq_bool(*mco_seq_cond_##TYPE##_ptr_t)(mco_##TYPE arg); \
        MCOSEQMATH_API MCO_RET mco_seq_next_##TYPE(mco_seq_iterator_h iterator, mco_##TYPE* val); \
        MCOSEQMATH_API MCO_RET mco_seq_parse_##TYPE(mco_seq_iterator_h result, char const* str); \
        MCOSEQMATH_API MCO_RET mco_seq_const_##TYPE(mco_seq_iterator_h result, mco_##TYPE val); \
        MCOSEQMATH_API MCO_RET mco_seq_func_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_func_##TYPE##_ptr_t func); \
        MCOSEQMATH_API MCO_RET mco_seq_func2_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right, mco_seq_func2_##TYPE##_ptr_t func); \
        MCOSEQMATH_API MCO_RET mco_seq_cond_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_cond_##TYPE##_ptr_t func); \
        MCOSEQMATH_API MCO_RET mco_seq_func_ctx_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_func_ctx_##TYPE##_ptr_t func, void* ctx); \
        MCOSEQMATH_API MCO_RET mco_seq_func2_ctx_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right, mco_seq_func2_ctx_##TYPE##_ptr_t func, void* ctx); \
        MCOSEQMATH_API MCO_RET mco_seq_cond_ctx_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_cond_ctx_##TYPE##_ptr_t func, void* ctx); \
        MCOSEQMATH_API MCO_RET mco_seq_map_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h positions); \
        MCOSEQMATH_API MCO_RET mco_seq_get_##TYPE(mco_seq_iterator_h input, mco_##TYPE* result, mco_size_t* size); \
        MCOSEQMATH_API MCO_RET mco_seq_unget_##TYPE(mco_seq_iterator_h result, mco_##TYPE const* buf, mco_size_t buf_size); \
        MCOSEQMATH_API MCO_RET mco_seq_get_nullable_##TYPE(mco_seq_iterator_h input, mco_##TYPE* result, mco_size_t* size, mco_bitmap_word_t* null_bitmask); \
        MCOSEQMATH_API MCO_RET mco_seq_unget_nullable_##TYPE(mco_seq_iterator_h result, mco_##TYPE const* buf, mco_size_t buf_size, mco_bitmap_word_t const* null_bitmask); \
        MCOSEQMATH_API MCO_RET mco_seq_join_##TYPE(mco_seq_iterator_h result, mco_seq_meta_iterator_h meta_iterator); \
        MCOSEQMATH_API MCO_RET mco_seq_union_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right, mco_seq_order_t order); \
        MCOSEQMATH_API MCO_RET mco_seq_add_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_sub_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_mul_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_div_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_mod_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_max_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_min_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_eq_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_ne_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_gt_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_ge_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_lt_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_le_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_wsum_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_wavg_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_cov_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_corr_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right); \
        MCOSEQMATH_API MCO_RET mco_seq_norm_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_thin_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t origin, mco_size_t step); \
        MCOSEQMATH_API MCO_RET mco_seq_iif_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h cond, mco_seq_iterator_h then_iter, mco_seq_iterator_h else_iter); \
        MCOSEQMATH_API MCO_RET mco_seq_if_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h cond, mco_seq_iterator_h then_iter, mco_seq_iterator_h else_iter); \
        MCOSEQMATH_API MCO_RET mco_seq_filter_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h cond, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_unique_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_abs_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_neg_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_reverse_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_diff_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_trend_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_agg_max_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_agg_min_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_agg_sum_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_agg_prd_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_agg_avg_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_agg_var_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_agg_dev_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_agg_var_samp_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_agg_dev_samp_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_agg_min_max_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_group_agg_max_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by); \
        MCOSEQMATH_API MCO_RET mco_seq_group_agg_min_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by); \
        MCOSEQMATH_API MCO_RET mco_seq_group_agg_sum_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by); \
        MCOSEQMATH_API MCO_RET mco_seq_group_agg_avg_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by); \
        MCOSEQMATH_API MCO_RET mco_seq_group_agg_var_samp_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by); \
        MCOSEQMATH_API MCO_RET mco_seq_group_agg_dev_samp_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by); \
        MCOSEQMATH_API MCO_RET mco_seq_group_agg_var_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by); \
        MCOSEQMATH_API MCO_RET mco_seq_group_agg_dev_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by); \
        MCOSEQMATH_API MCO_RET mco_seq_group_agg_last_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by); \
        MCOSEQMATH_API MCO_RET mco_seq_group_agg_first_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by); \
        MCOSEQMATH_API MCO_RET mco_seq_group_agg_wavg_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right, mco_seq_iterator_h group_by); \
        MCOSEQMATH_API MCO_RET mco_seq_grid_agg_max_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_grid_agg_min_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_grid_agg_sum_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_grid_agg_avg_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_grid_agg_var_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_grid_agg_dev_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_grid_agg_var_samp_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_grid_agg_dev_samp_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_window_agg_max_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_window_agg_min_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_window_agg_sum_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_window_agg_avg_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_window_agg_var_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_window_agg_dev_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_window_agg_var_samp_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_window_agg_dev_samp_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_window_agg_ema_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_window_agg_atr_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t interval); \
        MCOSEQMATH_API MCO_RET mco_seq_hash_agg_max_##TYPE(mco_seq_iterator_h result_max, mco_seq_iterator_h result_group_by, mco_seq_iterator_h input, mco_seq_iterator_h group_by, mco_size_t n_groups); \
        MCOSEQMATH_API MCO_RET mco_seq_hash_agg_min_##TYPE(mco_seq_iterator_h result_min, mco_seq_iterator_h result_group_by, mco_seq_iterator_h input, mco_seq_iterator_h group_by, mco_size_t n_groups); \
        MCOSEQMATH_API MCO_RET mco_seq_hash_agg_sum_##TYPE(mco_seq_iterator_h result_sum, mco_seq_iterator_h result_group_by, mco_seq_iterator_h input, mco_seq_iterator_h group_by, mco_size_t n_groups); \
        MCOSEQMATH_API MCO_RET mco_seq_hash_agg_avg_##TYPE(mco_seq_iterator_h result_avg, mco_seq_iterator_h result_group_by, mco_seq_iterator_h input, mco_seq_iterator_h group_by, mco_size_t n_groups); \
        MCOSEQMATH_API MCO_RET mco_seq_top_max_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t top); \
        MCOSEQMATH_API MCO_RET mco_seq_top_min_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t top); \
        MCOSEQMATH_API MCO_RET mco_seq_top_pos_max_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t top); \
        MCOSEQMATH_API MCO_RET mco_seq_top_pos_min_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t top); \
        MCOSEQMATH_API MCO_RET mco_seq_cum_agg_avg_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_cum_agg_sum_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_cum_agg_min_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_cum_agg_max_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_cum_agg_prd_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_cum_agg_var_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_cum_agg_dev_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_cum_agg_var_samp_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_cum_agg_dev_samp_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_histogram_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_##TYPE min_value, mco_##TYPE max_value, mco_size_t n_intervals); \
        MCOSEQMATH_API MCO_RET mco_seq_cross_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, int first_cross_direction); \
        MCOSEQMATH_API MCO_RET mco_seq_extrema_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, int first_extremum); \
        MCOSEQMATH_API MCO_RET mco_seq_sort_##TYPE(mco_##TYPE const* elems, mco_seq_no_t* permutation, mco_size_t n_elems, mco_seq_order_t order); \
        MCOSEQMATH_API MCO_RET mco_seq_repeat_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, int n_times); \
        MCOSEQMATH_API MCO_RET mco_seq_order_by_##TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_no_t const* permutation, mco_size_t n_elems, mco_##TYPE* data)

    MCO_SEQ_FUNC_DECL(int1);
    MCO_SEQ_FUNC_DECL(int2);
    MCO_SEQ_FUNC_DECL(int4);
    MCO_SEQ_FUNC_DECL(int8);
    MCO_SEQ_FUNC_DECL(uint1);
    MCO_SEQ_FUNC_DECL(uint2);
    MCO_SEQ_FUNC_DECL(uint4);
    MCO_SEQ_FUNC_DECL(uint8);
    MCO_SEQ_FUNC_DECL(float);
    MCO_SEQ_FUNC_DECL(double);
    MCO_SEQ_FUNC_DECL(datetime);

    #define MCO_SEQ_FUNC2_DECL(MNEM, TYPE1, TYPE2)                      \
        MCOSEQMATH_API MCO_RET mco_seq_##MNEM##_##TYPE1##_##TYPE2(mco_seq_iterator_h result, mco_seq_iterator_h ts1, mco_seq_iterator_h ts2, mco_seq_iterator_h values, mco_##TYPE2 filler)

    MCO_SEQ_FUNC2_DECL(stretch, uint4, int1);
    MCO_SEQ_FUNC2_DECL(stretch, uint4, int2);
    MCO_SEQ_FUNC2_DECL(stretch, uint4, int4);
    MCO_SEQ_FUNC2_DECL(stretch, uint4, int8);
    MCO_SEQ_FUNC2_DECL(stretch, uint4, uint1);
    MCO_SEQ_FUNC2_DECL(stretch, uint4, uint2);
    MCO_SEQ_FUNC2_DECL(stretch, uint4, uint4);
    MCO_SEQ_FUNC2_DECL(stretch, uint4, uint8);
    MCO_SEQ_FUNC2_DECL(stretch, uint4, float);
    MCO_SEQ_FUNC2_DECL(stretch, uint4, double);
    MCO_SEQ_FUNC2_DECL(stretch, uint4, datetime);
    MCO_SEQ_FUNC2_DECL(stretch, uint8, int1);
    MCO_SEQ_FUNC2_DECL(stretch, uint8, int2);
    MCO_SEQ_FUNC2_DECL(stretch, uint8, int4);
    MCO_SEQ_FUNC2_DECL(stretch, uint8, int8);
    MCO_SEQ_FUNC2_DECL(stretch, uint8, uint1);
    MCO_SEQ_FUNC2_DECL(stretch, uint8, uint2);
    MCO_SEQ_FUNC2_DECL(stretch, uint8, uint4);
    MCO_SEQ_FUNC2_DECL(stretch, uint8, uint8);
    MCO_SEQ_FUNC2_DECL(stretch, uint8, float);
    MCO_SEQ_FUNC2_DECL(stretch, uint8, double);
    MCO_SEQ_FUNC2_DECL(stretch, uint8, datetime);
    MCO_SEQ_FUNC2_DECL(stretch, datetime, int1);
    MCO_SEQ_FUNC2_DECL(stretch, datetime, int2);
    MCO_SEQ_FUNC2_DECL(stretch, datetime, int4);
    MCO_SEQ_FUNC2_DECL(stretch, datetime, int8);
    MCO_SEQ_FUNC2_DECL(stretch, datetime, uint1);
    MCO_SEQ_FUNC2_DECL(stretch, datetime, uint2);
    MCO_SEQ_FUNC2_DECL(stretch, datetime, uint4);
    MCO_SEQ_FUNC2_DECL(stretch, datetime, uint8);
    MCO_SEQ_FUNC2_DECL(stretch, datetime, datetime);
    MCO_SEQ_FUNC2_DECL(stretch, datetime, float);
    MCO_SEQ_FUNC2_DECL(stretch, datetime, double);


    MCO_SEQ_FUNC2_DECL(stretch0, uint4, int1);
    MCO_SEQ_FUNC2_DECL(stretch0, uint4, int2);
    MCO_SEQ_FUNC2_DECL(stretch0, uint4, int4);
    MCO_SEQ_FUNC2_DECL(stretch0, uint4, int8);
    MCO_SEQ_FUNC2_DECL(stretch0, uint4, uint1);
    MCO_SEQ_FUNC2_DECL(stretch0, uint4, uint2);
    MCO_SEQ_FUNC2_DECL(stretch0, uint4, uint4);
    MCO_SEQ_FUNC2_DECL(stretch0, uint4, uint8);
    MCO_SEQ_FUNC2_DECL(stretch0, uint4, float);
    MCO_SEQ_FUNC2_DECL(stretch0, uint4, double);
    MCO_SEQ_FUNC2_DECL(stretch0, uint4, datetime);
    MCO_SEQ_FUNC2_DECL(stretch0, uint8, int1);
    MCO_SEQ_FUNC2_DECL(stretch0, uint8, int2);
    MCO_SEQ_FUNC2_DECL(stretch0, uint8, int4);
    MCO_SEQ_FUNC2_DECL(stretch0, uint8, int8);
    MCO_SEQ_FUNC2_DECL(stretch0, uint8, uint1);
    MCO_SEQ_FUNC2_DECL(stretch0, uint8, uint2);
    MCO_SEQ_FUNC2_DECL(stretch0, uint8, uint4);
    MCO_SEQ_FUNC2_DECL(stretch0, uint8, uint8);
    MCO_SEQ_FUNC2_DECL(stretch0, uint8, float);
    MCO_SEQ_FUNC2_DECL(stretch0, uint8, double);
    MCO_SEQ_FUNC2_DECL(stretch0, uint8, datetime);
    MCO_SEQ_FUNC2_DECL(stretch0, datetime, int1);
    MCO_SEQ_FUNC2_DECL(stretch0, datetime, int2);
    MCO_SEQ_FUNC2_DECL(stretch0, datetime, int4);
    MCO_SEQ_FUNC2_DECL(stretch0, datetime, int8);
    MCO_SEQ_FUNC2_DECL(stretch0, datetime, uint1);
    MCO_SEQ_FUNC2_DECL(stretch0, datetime, uint2);
    MCO_SEQ_FUNC2_DECL(stretch0, datetime, uint4);
    MCO_SEQ_FUNC2_DECL(stretch0, datetime, uint8);
    MCO_SEQ_FUNC2_DECL(stretch0, datetime, datetime);
    MCO_SEQ_FUNC2_DECL(stretch0, datetime, float);
    MCO_SEQ_FUNC2_DECL(stretch0, datetime, double);


    #define MCO_SEQ_ASOF_DECL(TYPE1, TYPE2)                             \
        MCOSEQMATH_API MCO_RET mco_seq_asof_join_##TYPE1##_##TYPE2(mco_seq_iterator_h result, mco_seq_iterator_h ts1, mco_seq_iterator_h ts2, mco_seq_iterator_h values)

    MCO_SEQ_ASOF_DECL(uint4, int1);
    MCO_SEQ_ASOF_DECL(uint4, int2);
    MCO_SEQ_ASOF_DECL(uint4, int4);
    MCO_SEQ_ASOF_DECL(uint4, int8);
    MCO_SEQ_ASOF_DECL(uint4, uint1);
    MCO_SEQ_ASOF_DECL(uint4, uint2);
    MCO_SEQ_ASOF_DECL(uint4, uint4);
    MCO_SEQ_ASOF_DECL(uint4, uint8);
    MCO_SEQ_ASOF_DECL(uint4, float);
    MCO_SEQ_ASOF_DECL(uint4, double);
    MCO_SEQ_ASOF_DECL(uint4, datetime);
    MCO_SEQ_ASOF_DECL(uint8, int1);
    MCO_SEQ_ASOF_DECL(uint8, int2);
    MCO_SEQ_ASOF_DECL(uint8, int4);
    MCO_SEQ_ASOF_DECL(uint8, int8);
    MCO_SEQ_ASOF_DECL(uint8, uint1);
    MCO_SEQ_ASOF_DECL(uint8, uint2);
    MCO_SEQ_ASOF_DECL(uint8, uint4);
    MCO_SEQ_ASOF_DECL(uint8, uint8);
    MCO_SEQ_ASOF_DECL(uint8, float);
    MCO_SEQ_ASOF_DECL(uint8, double);
    MCO_SEQ_ASOF_DECL(uint8, datetime);
    MCO_SEQ_ASOF_DECL(datetime, int1);
    MCO_SEQ_ASOF_DECL(datetime, int2);
    MCO_SEQ_ASOF_DECL(datetime, int4);
    MCO_SEQ_ASOF_DECL(datetime, int8);
    MCO_SEQ_ASOF_DECL(datetime, uint1);
    MCO_SEQ_ASOF_DECL(datetime, uint2);
    MCO_SEQ_ASOF_DECL(datetime, uint4);
    MCO_SEQ_ASOF_DECL(datetime, uint8);
    MCO_SEQ_ASOF_DECL(datetime, datetime);
    MCO_SEQ_ASOF_DECL(datetime, float);
    MCO_SEQ_ASOF_DECL(datetime, double);

    typedef void(*mco_seq_func_ctx_char_ptr_t)(char* result, char const* arg, mco_size_t elem_size, void* ctx);
    typedef void(*mco_seq_func2_ctx_char_ptr_t)(char* result, char const* arg1, char const* arg2, mco_size_t elem_size, void* ctx);
    typedef mco_seq_bool(*mco_seq_cond_ctx_char_ptr_t)(char const* arg, mco_size_t elem_size, void* ctx);
    typedef void(*mco_seq_func_char_ptr_t)(char* result, char const* arg, mco_size_t elem_size);
    typedef void(*mco_seq_func2_char_ptr_t)(char* result, char const* arg1, char const* arg2, mco_size_t elem_size);
    typedef mco_seq_bool(*mco_seq_cond_char_ptr_t)(char const* arg, mco_size_t elem_size);

    /* Functions for sequence of char */
    MCOSEQMATH_API MCO_RET mco_seq_next_char(mco_seq_iterator_h iterator, void* val);
    MCOSEQMATH_API MCO_RET mco_seq_const_char(mco_seq_iterator_h result, void const* val, mco_size_t size);
    MCOSEQMATH_API MCO_RET mco_seq_iif_char(mco_seq_iterator_h result, mco_seq_iterator_h cond, mco_seq_iterator_h then_iter, mco_seq_iterator_h else_iter);
    MCOSEQMATH_API MCO_RET mco_seq_if_char(mco_seq_iterator_h result, mco_seq_iterator_h cond, mco_seq_iterator_h then_iter, mco_seq_iterator_h else_iter);
    MCOSEQMATH_API MCO_RET mco_seq_filter_char(mco_seq_iterator_h result, mco_seq_iterator_h cond, mco_seq_iterator_h input);
    MCOSEQMATH_API MCO_RET mco_seq_unique_char(mco_seq_iterator_h result, mco_seq_iterator_h input);
    MCOSEQMATH_API MCO_RET mco_seq_thin_char(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t origin, mco_size_t step);
    MCOSEQMATH_API MCO_RET mco_seq_repeat_char(mco_seq_iterator_h result, mco_seq_iterator_h input, int count);
    MCOSEQMATH_API MCO_RET mco_seq_get_char(mco_seq_iterator_h input, void* buf, mco_size_t* size);
    MCOSEQMATH_API MCO_RET mco_seq_unget_char(mco_seq_iterator_h result, void const* buf, mco_size_t buf_size, mco_size_t elem_size);
    MCOSEQMATH_API MCO_RET mco_seq_get_nullable_char(mco_seq_iterator_h input, void* buf, mco_size_t* size, mco_bitmap_word_t* null_bitmap);
    MCOSEQMATH_API MCO_RET mco_seq_unget_nullable_char(mco_seq_iterator_h result, void const* buf, mco_size_t buf_size, mco_size_t elem_size, mco_bitmap_word_t const* null_bitmap);
    MCOSEQMATH_API MCO_RET mco_seq_map_char(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
    MCOSEQMATH_API MCO_RET mco_seq_reverse_char(mco_seq_iterator_h result, mco_seq_iterator_h input);
    MCOSEQMATH_API MCO_RET mco_seq_join_char(mco_seq_iterator_h result, mco_seq_meta_iterator_h meta_iterator, mco_size_t elem_size);
    MCOSEQMATH_API MCO_RET mco_seq_add_char(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
    MCOSEQMATH_API MCO_RET mco_seq_eq_char(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
    MCOSEQMATH_API MCO_RET mco_seq_ne_char(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
    MCOSEQMATH_API MCO_RET mco_seq_gt_char(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
    MCOSEQMATH_API MCO_RET mco_seq_ge_char(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
    MCOSEQMATH_API MCO_RET mco_seq_lt_char(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
    MCOSEQMATH_API MCO_RET mco_seq_le_char(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
    MCOSEQMATH_API MCO_RET mco_seq_func_char(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_func_char_ptr_t func);
    MCOSEQMATH_API MCO_RET mco_seq_func2_char(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right, mco_seq_func2_char_ptr_t func);
    MCOSEQMATH_API MCO_RET mco_seq_cond_char(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_cond_char_ptr_t func);
    MCOSEQMATH_API MCO_RET mco_seq_func_ctx_char(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_func_ctx_char_ptr_t func, void* ctx);
    MCOSEQMATH_API MCO_RET mco_seq_func2_ctx_char(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right, mco_seq_func2_ctx_char_ptr_t func, void* ctx);
    MCOSEQMATH_API MCO_RET mco_seq_cond_ctx_char(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_cond_ctx_char_ptr_t func, void* ctx);
    MCOSEQMATH_API MCO_RET mco_seq_order_by_char(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_no_t const *permutation, mco_size_t n_elems, char* data);
    MCOSEQMATH_API MCO_RET mco_seq_print_char(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_size_t elem_size, char const* format);
    MCOSEQMATH_API MCO_RET mco_seq_match(mco_seq_iterator_h result, mco_seq_iterator_h input, char const* pattern);
	MCOSEQMATH_API MCO_RET mco_seq_skip_nan_float(mco_seq_iterator_h result, mco_seq_iterator_h input); 
	MCOSEQMATH_API MCO_RET mco_seq_skip_nan_double(mco_seq_iterator_h result, mco_seq_iterator_h input); 
	MCOSEQMATH_API MCO_RET mco_seq_parse_char(mco_seq_iterator_h result, char const* str, int width);

    /* Polymorphic functions */
	typedef mco_bool (*mco_parse_function_t)(char const* str, mco_seq_tile_t* tile, int i);
	MCOSEQMATH_API MCO_RET mco_seq_custom_parse(mco_seq_iterator_h result, char const* input, int elem_type, int elem_size, mco_parse_function_t parser);
    MCOSEQMATH_API MCO_RET mco_seq_concat(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
    MCOSEQMATH_API MCO_RET mco_seq_cat(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
    MCOSEQMATH_API MCO_RET mco_seq_limit(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_no_t from, mco_seq_no_t till);
    MCOSEQMATH_API MCO_RET mco_seq_agg_count(mco_seq_iterator_h result, mco_seq_iterator_h input);
    MCOSEQMATH_API MCO_RET mco_seq_agg_count_not_null(mco_seq_iterator_h result, mco_seq_iterator_h input);
    MCOSEQMATH_API MCO_RET mco_seq_agg_approxdc(mco_seq_iterator_h result, mco_seq_iterator_h input);
    MCOSEQMATH_API MCO_RET mco_seq_group_agg_count(mco_seq_iterator_h result, mco_seq_iterator_h group_by);
    MCOSEQMATH_API MCO_RET mco_seq_group_agg_approxdc(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h group_by);
    MCOSEQMATH_API MCO_RET mco_seq_hash_agg_count(mco_seq_iterator_h result_count, mco_seq_iterator_h result_group_by, mco_seq_iterator_h group_by, mco_size_t n_groups);
    MCOSEQMATH_API MCO_RET mco_seq_hash_agg_approxdc(mco_seq_iterator_h result_count, mco_seq_iterator_h result_group_by, mco_seq_iterator_h input, mco_seq_iterator_h group_by, mco_size_t n_groups);
    MCOSEQMATH_API MCO_RET mco_seq_hash_agg_distinct_count(mco_seq_iterator_h result_count, mco_seq_iterator_h result_group_by, mco_seq_iterator_h input, mco_seq_iterator_h group_by, mco_size_t n_groups, mco_size_t n_pairs);
    MCOSEQMATH_API MCO_RET mco_seq_hash_agg_dup_count(mco_seq_iterator_h result_count, mco_seq_iterator_h result_group_by, mco_seq_iterator_h input, mco_seq_iterator_h group_by, mco_size_t n_groups, mco_size_t n_pairs, mco_size_t min_occurrences);
    MCOSEQMATH_API void mco_seq_release_hash(mco_seq_iterator_h iterator);
    MCOSEQMATH_API mco_bool mco_seq_free_hash(mco_seq_iterator_h iterator);

    MCOSEQMATH_API MCO_RET mco_seq_hash(mco_seq_iterator_h result, mco_seq_iterator_h input, mco_seq_iterator_h hash);
    MCOSEQMATH_API MCO_RET mco_seq_agg_approxdc_hash(mco_seq_iterator_h result, mco_seq_iterator_h hash);
    MCOSEQMATH_API MCO_RET mco_seq_group_agg_approxdc_hash(mco_seq_iterator_h result, mco_seq_iterator_h hash, mco_seq_iterator_h group_by);

    MCOSEQMATH_API MCO_RET mco_seq_tee(mco_seq_iterator_h iterator1, mco_seq_iterator_h iterator2, mco_seq_iterator_h input);
    MCOSEQMATH_API MCO_RET mco_seq_filter_pos(mco_seq_iterator_h result, mco_seq_iterator_h cond);
    MCOSEQMATH_API MCO_RET mco_seq_filter_search(mco_seq_iterator_h result, mco_seq_iterator_h cond, mco_seq_iterator_h search_result);
    MCOSEQMATH_API MCO_RET mco_seq_filter_first_pos(mco_seq_iterator_h result, mco_seq_iterator_h cond, mco_size_t n);

    MCOSEQMATH_API MCO_RET mco_seq_not(mco_seq_iterator_h result, mco_seq_iterator_h input);
    MCOSEQMATH_API MCO_RET mco_seq_and(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
    MCOSEQMATH_API MCO_RET mco_seq_or(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);
    MCOSEQMATH_API MCO_RET mco_seq_xor(mco_seq_iterator_h result, mco_seq_iterator_h left, mco_seq_iterator_h right);

    #define MCO_SEQ_CAST_DECL(FROM_TYPE, TO_TYPE)       \
        MCOSEQMATH_API MCO_RET mco_seq_##FROM_TYPE##_to_##TO_TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input); \
        MCOSEQMATH_API MCO_RET mco_seq_##TO_TYPE##_from_##FROM_TYPE(mco_seq_iterator_h result, mco_seq_iterator_h input);

    #define MCO_SEQ_CASTS_DECL(TYPE)            \
        MCO_SEQ_CAST_DECL(TYPE, uint1)          \
        MCO_SEQ_CAST_DECL(TYPE, uint2)          \
        MCO_SEQ_CAST_DECL(TYPE, uint4)          \
        MCO_SEQ_CAST_DECL(TYPE, uint8)          \
        MCO_SEQ_CAST_DECL(TYPE, int1)           \
        MCO_SEQ_CAST_DECL(TYPE, int2)           \
        MCO_SEQ_CAST_DECL(TYPE, int4)           \
        MCO_SEQ_CAST_DECL(TYPE, int8)           \
        MCO_SEQ_CAST_DECL(TYPE, datetime)   \
        MCO_SEQ_CAST_DECL(TYPE, float)          \
        MCO_SEQ_CAST_DECL(TYPE, double)

    MCO_SEQ_CASTS_DECL(uint1)
    MCO_SEQ_CASTS_DECL(uint2)
    MCO_SEQ_CASTS_DECL(uint4)
    MCO_SEQ_CASTS_DECL(uint8)
    MCO_SEQ_CASTS_DECL(int1)
    MCO_SEQ_CASTS_DECL(int2)
    MCO_SEQ_CASTS_DECL(int4)
    MCO_SEQ_CASTS_DECL(int8)
    MCO_SEQ_CASTS_DECL(float)
    MCO_SEQ_CASTS_DECL(double)
    MCO_SEQ_CASTS_DECL(datetime)

	MCOSEQ_API MCO_RET mco_seq_reset_iterator(mco_seq_iterator_h iterator);
    MCOSEQ_API MCO_RET mco_seq_reset_unary_iterator(mco_seq_iterator_h iterator);
    MCOSEQ_API MCO_RET mco_seq_reset_binary_iterator(mco_seq_iterator_h iterator);

    MCOSEQ_API mco_seq_rle_count_t mco_seq_rle_count(mco_seq_iterator_h iterator, int pos);
    MCOSEQ_API mco_bool mco_seq_is_rle(void);
    MCOSEQ_API MCO_RET mco_seq_rle_decode(mco_seq_iterator_h dst, mco_seq_iterator_h src);

    typedef void* (*mco_seq_malloc_t)(mco_seq_iterator_h iterator, mco_size_t size);
    typedef void (*mco_seq_free_t)(mco_seq_iterator_h iterator, void* ptr, mco_size_t size);


    extern mco_seq_malloc_t mco_seq_malloc;
    extern mco_seq_free_t mco_seq_free;
    extern double mco_seq_hash_table_load_factor;
    extern mco_size_t mco_seq_hash_table_init_size;
	extern MCOSEQ_API mco_bool mco_seq_check_bindings;

    MCOSEQ_API void mco_seq_redefine_allocator(mco_seq_malloc_t custom_malloc, mco_seq_free_t custom_free);
    MCOSEQ_API mco_bool mco_seq_enable_check_bindings(mco_bool enabled);
    MCOSEQ_API mco_bool mco_seq_is_hash_aggreate(mco_seq_iterator_h iterator);
    MCOSEQ_API void mco_seq_free_buffer(mco_seq_iterator_h iterator);

    MCOSEQ_API void* mco_seq_sys_malloc(mco_seq_iterator_h iterator, mco_size_t size);
    MCOSEQ_API void mco_seq_sys_free(mco_seq_iterator_h iterator, void* ptr, mco_size_t size);

#ifdef __cplusplus
}
#endif


#endif

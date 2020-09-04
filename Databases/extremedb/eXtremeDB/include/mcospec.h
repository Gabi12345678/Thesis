/*******************************************************************
 *                                                                 *
 *  mcospec.h                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *  eXtremeDB compiler & platform specific configuration           *
 *                                                                 *
 ***************************************************************** */

#ifndef MCO_MCOSPEC_H__
    #define MCO_MCOSPEC_H__

    #include <stddef.h>

    #ifdef __cplusplus
        extern "C"
        {
        #endif

        #include "mcotargt.h"

        #if MCO_CFG_BUILTIN_MEMCPY
        #include <string.h>
        #endif

        /*#define	MCO_CFG_QUAD_STRUCT*/
        #define MCO_CFG_QUAD_SCALAR

        #include "mcoquad.h"

        #define MCO_HA_USE_BIN_SERIALIZATION

         /* typedef int mcoint; */ /* best-int: int or short, signed or unsigned, depending on target; min 2 bytes */
         /* typedef uint2 mcoint; */ /* also must work */

        #ifndef MCO_BASE_TYPES_DEFINED

            /* define MCO_BASE_TYPES_DEFINED if want to use external definitions */

            typedef unsigned char  mco_uint1;
            typedef unsigned short mco_uint2;
            typedef unsigned int   mco_uint4; /* can be 3 or 4 bytes long */
            typedef signed   char  mco_int1;
            typedef short          mco_int2;
            typedef int            mco_int4; /* can be 3 or 4 bytes long */
            typedef unsigned int   mco_date;
            typedef unsigned int   mco_time; /* time_t ok */
            typedef mco_uquad      mco_datetime;
            typedef mco_uquad      mco_uint8;
            typedef mco_iquad      mco_int8;
            typedef mco_iquad      autoid_t;
            typedef unsigned short nchar_t;
            typedef double         mco_double;
            typedef float          mco_float;

            typedef mco_uint1 uint1;
            typedef mco_uint2 uint2;
            typedef mco_uint4 uint4;
            typedef mco_uint8 uint8;
            typedef mco_int1  int1;
            typedef mco_int2  int2;
            typedef mco_int4  int4;
#ifndef _AIX
            typedef mco_int8  int8;
#endif
            typedef mco_uquad      mco_trans_counter_t;

            typedef uint4          mco_size32_t;
            typedef int4           mco_size32_sig_t;

            typedef uint4          mco_offs32_t;
            typedef int4           mco_offs32_sig_t;
            typedef uint4          mco_counter32_t;

            #ifndef MCO_PLATFORM_X64

                typedef uint4 mco_size_t;
                typedef int4  mco_size_sig_t;

            #ifdef MCO_LARGE_DATABASE_SUPPORT
                typedef uint8 mco_offs_t;
                typedef mco_int8  mco_offs_sig_t;
            #else
                typedef uint4 mco_offs_t;
                typedef int4  mco_offs_sig_t;
            #endif

                typedef uint4 mco_counter_t;
                typedef uint4 mco_hash_counter_t;

                typedef uint4 mco_conid_t;
                typedef uint4 mco_addr_t;

            #else

                typedef uint8 mco_size_t;
                typedef mco_int8  mco_size_sig_t;

                typedef uint8 mco_offs_t;
                typedef mco_int8  mco_offs_sig_t;

                typedef uint8 mco_counter_t;
                typedef uint8 mco_hash_counter_t;

                typedef uint4 mco_conid_t;
                typedef uint8 mco_addr_t;
            #endif




            #define MCO_CONID_INVALID ((uint4)-1)

        #endif

        #ifdef MCO_PLATFORM_X64
            #define MCO_MEM_ALIGN  MCO_QUAD_ALIGNMENT

            #define HI_32(val64) (mco_size32_t) (val64>>32)
            #define LO_32(val64) (mco_size32_t)(val64&0xFFFFFFFF)

        #else
            #if defined(MCO_LARGE_DATABASE_SUPPORT) || MCO_CFG_STRICT_ALIGNMENT
            #define MCO_MEM_ALIGN  MCO_QUAD_ALIGNMENT
            #else
            #define MCO_MEM_ALIGN  4
            #endif

            #if defined(_WIN32) ||defined(WIN32)
                #define HI_32(val64) (mco_size32_t) (sizeof(val64)==8?((val64>>31)>>1):0)
            #else
                #define HI_32(val64) (mco_size32_t) ((val64>>31)>>1)
            #endif /*  */
            #define LO_32(val64) (mco_size32_t)(val64&0xFFFFFFFF)

        #endif /*  */

        /* null pointer */
        #define NULLPTR (void*)0

        #define MCO_MAKE_MEM_ALIGN(memAddr) ((((mco_addr_t)memAddr)+(mco_addr_t)(MCO_MEM_ALIGN-1)) & (mco_addr_t)(~(MCO_MEM_ALIGN-1)))

        #ifdef _MSC_VER
            #pragma warning(disable: 4100)
            #pragma warning(disable: 4505)
            #pragma warning(disable: 4206)
        #endif

        /* ZILOG build */
        /* #define MCO_CFG_ZILOG */

        #define MCO_STDCALL /* __stdcall */


        #ifdef MCO_CFG_ZILOG

            #define MCO_NO_SHIFT2
            #define MCO_XCOMP_COPYSTRUCTBUG
            #define MCO_XCOMP_ICASTBUG
            #define MCO_RTL_MEMCPY
            #define MCO_RTL_MEMMOVE
            #define MCO_UINT4_UNAVAIL /* uint4 is really 2 or 3 bytes long */
            #define MCO_NO_EXTRA_HEAP_FN /* disable additional heap functions to reduce code size */
            #undef MCO_HA_USE_BIN_SERIALIZATION

            void* memcpy(void* s1, void* s2, int n);
            int   memcmp(void* s1, void* s2, int n);
            void* memset(void* s, int c, int n);
            void* memmove(void* s1, void* s2, int n);

            #define MCO_NO_FORWARDS /* compiler does not support forward declarations */
            #define MCO_NO_REPORTS  /* just turn off */
            #define MCO_CFG_COMPARE_SUBCALLS
            #define MCO_CFG_SUBCALLS /* use less inlines, as possible */

        #endif


        #if MCO_CFG_BUILTIN_MEMCPY
        #define mco_memcpy  memcpy
        #define mco_memset  memset
        #else
        MCO_INLINE_PREFIX void mco_memcpy(void* dest, const void* src, mco_size_t nbt) MCO_INLINE_SUFFIX;
        MCO_INLINE_PREFIX void mco_memset(void* dest, int fillb, mco_size_t nbt) MCO_INLINE_SUFFIX;
        #endif

        MCO_INLINE_PREFIX void mco_memmove(void* dest, const void* src, mco_size_t nbt) MCO_INLINE_SUFFIX;
        MCO_INLINE_PREFIX int  mco_memcmp(void const* p1, void const* p2, mco_size_t nbt) MCO_INLINE_SUFFIX;
        MCO_INLINE_PREFIX char* mco_strchr(char const* str, char ch) MCO_INLINE_SUFFIX;

        #ifdef MCO_NO_FORWARDS
            #define TRNP1(t) mco_trans_h t__
            #define TRNP2(t) struct mco_trans_t_ * t = (struct mco_trans_t_ *)t__;
            #define OBJH1(h) mco_objhandle_h h__
            #define OBJH2(h) struct mco_objhandle_t_ * h = (struct mco_objhandle_t_ *)h__;
            #define OBJH1_(h) mco_objhandle_h h___
            #define OBJH2_(h) struct mco_objhandle_t_ * h = (struct mco_objhandle_t_ *)h___;
            #define SEMP1(s) mco_sem_h s__
            #define SEMP2(s) struct mco_sem_t_ * s = (struct mco_sem_t_ *)s__;
        #else
            #define TRNP1(t) mco_trans_h t
            #define TRNP2(t) /* nothing */
            #define OBJH1(h) mco_objhandle_h h
            #define OBJH2(h) /* nothing */
            #define OBJH1_(h) mco_objhandle_h h
            #define OBJH2_(h) /* nothing */
            #define SEMP1(s) mco_sem_h s
            #define SEMP2(s) /* nothing */
        #endif

        #ifdef __cplusplus
        }
    #endif

#endif

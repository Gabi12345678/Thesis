/*******************************************************************
 *                                                                 *
 *  mcoquad.h                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           * 
 *  All Rights Reserved                                            *
 *                                                                 *
 *  eXtremeDB runtime 64 bit integers definitions                  *
 *                                                                 *
 ***************************************************************** */

#ifndef MCO_QUAD_H__
    #define MCO_QUAD_H__


    #ifdef __cplusplus
        extern "C"
        {
        #endif 

        /* 64 bit integer arithmetic emulated 
        - replace by native where it is possible.
        Here defined only those methods that are used by DB runtime 
         */

        #ifdef MCO_CFG_QUAD_STRUCT

            typedef struct mco_uquad_t
            {
                unsigned int lo;
                unsigned int hi;
            } mco_uquad;

            typedef struct mco_iquad_t
            {
                unsigned int lo;
                int hi;
            } mco_iquad;

            void mco_uquad_increment(mco_uquad* u8); /* (*u8) ++ */
            short mco_uquad_compare(const mco_uquad* a, const mco_uquad* b); /* a<b ? -1 : (a>b ? 1 : 0) */
            short mco_iquad_compare(const mco_iquad* a, const mco_iquad* b); /* a<b ? -1 : (a>b ? 1 : 0) */

            #define mco_quad_eq(a,b) (((a).lo==(b).lo) && ((a).hi==(b).hi))

            #define MCO_QUAD_ALIGNMENT 4
            #define MCO_PUB_STRUCT_BASE unsigned int

        #else /* MCO_CFG_QUAD_STRUCT */

            #if defined _MSC_VER
                typedef __int64 mco_iquad;
                typedef unsigned __int64 mco_uquad;
            #else 
                #ifdef MCO_USE_STDINT
                #include <stdint.h>
                typedef  int64_t mco_iquad;
                typedef uint64_t mco_uquad;
                #else
                typedef long long mco_iquad;
                typedef unsigned long long mco_uquad;
                #endif
            #endif 

            #define mco_uquad_increment(u8) {(*u8)++;}
            #define mco_uquad_compare(a,b) ((*a)<(*b)?-1:((*a)>(*b)?1:0))
            #define mco_iquad_compare(a,b) ((*a)<(*b)?-1:((*a)>(*b)?1:0))

            #define mco_quad_eq(a,b) (a==b)

            #define MCO_QUAD_ALIGNMENT 8
            #define MCO_PUB_STRUCT_BASE mco_uquad

        #endif /* MCO_CFG_QUAD_STRUCT */

        #ifdef __cplusplus
        }
    #endif 

#endif

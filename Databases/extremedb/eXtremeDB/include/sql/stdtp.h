/*******************************************************************
 *                                                                 *
 *  stdtp.h                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           * 
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifndef __STDTP_H__
    #define __STDTP_H__

    #include "config.h"

    #if !defined(HAS_STDINT)
        #define HAS_STDINT 1
    #endif 

    #if defined(_WIN32) && defined (_MSC_VER)
        #pragma warning(disable:4996)
    #endif 

    #ifdef UNICODE_SUPPORT
        #include <stdarg.h>
        #include <wchar.h>
        #include <wctype.h>
    #endif 

    #if !defined(_WIN32) && HAS_STDINT != 0
        #if defined (_ECOS)
            #include <unistd.h>
        #else
            #include <stdint.h>
        #endif
    #else 

        #if !defined(INT8_T_DEFINED)
            #define INT8_T_DEFINED
            #if defined(_WIN32) && !defined(__MINGW32__)
                typedef __int64 int64_t;
                typedef unsigned __int64 uint64_t;
            #elif defined(SIZEOF_LONG) && SIZEOF_LONG == 8
                typedef unsigned long uint64_t;
                typedef signed long int64_t;
            #elif defined(_LINUX) && !defined (__LYNX)
                #include <sys/types.h>
            #elif defined(_SOLARIS)
                #include <inttypes.h>
            #elif defined(_VXWORKS)
#ifdef _WRS_KERNEL
                #include <types.h>
#else
				#include <types/vxTypesBase.h>
				#include <types/vxTypes.h>                
#endif
            #elif defined(__LYNX)
                #include <stdint.h>
            #else 
                #if !defined(_VXWORKS) && !defined(_HPUX) && !defined(_AIX) && !defined(__BIT_TYPES_DEFINED__) /* _ECOS */
                    typedef unsigned long long uint64_t;
                    typedef signed long long int64_t;
                #endif 
            #endif 
        #endif 

    #endif 

    #ifndef OVERLOAD_LONG
    #ifdef _VXWORKS
    #define OVERLOAD_LONG 1
    #else
    #define OVERLOAD_LONG 0
    #endif
    #endif

    #ifndef INT8_FORMAT
        #if defined(_WIN32) && !defined(__MINGW32__)
            #if defined(__IBMCPP__)
                #define INT8_FORMAT "ll"
                #define L_INT8_FORMAT L"ll"
            #else 
                #define INT8_FORMAT "I64"
                #define L_INT8_FORMAT L"I64"
            #endif 
        #elif defined (SIZEOF_LONG) && SIZEOF_LONG == 8
            #define INT8_FORMAT "l"
            #define L_INT8_FORMAT L"l"
        #else 
            #ifdef __MINGW32__
                #define INT8_FORMAT "I64"
                #define L_INT8_FORMAT L"I64"
            #else 
                #define INT8_FORMAT "ll"
                #define L_INT8_FORMAT L"ll"
            #endif 
        #endif 
    #endif 


   /* default empty */
   #ifdef __GNUC__
   #define MCO_SQL_NORETURN_SUFFIX __attribute__((noreturn))
   #define MCO_VAR_SIZE_ARRAY 0
   #else
   #define MCO_SQL_NORETURN_SUFFIX 
   #define MCO_VAR_SIZE_ARRAY 1
   #endif


    #if MCO_CFG_USE_EXCEPTIONS
    #define MCO_TRY try
    #define MCO_CATCH(x) catch(x)
    #define MCO_THROW throw
    #define MCO_RETURN_ERROR_CODE catch (McoSql::McoSqlException const &x) { return (status_t)x.code; }
    #else
    extern void mco_sql_throw_exeption(char const* file, int line);
    #define MCO_TRY       
    #define MCO_CATCH(x)  
    #define MCO_THROW      mco_sql_throw_exeption(__FILE__, __LINE__), 
    #define MCO_RETURN_ERROR_CODE 
    #endif
 

    #if defined(_VXWORKS) && defined(_RTP)

    #ifndef max
    #define max(a,b)    (((a) > (b)) ? (a) : (b))
    #endif

    #ifndef min
    #define min(a,b)    (((a) < (b)) ? (a) : (b))
    #endif

    #endif

#endif

/*******************************************************************
 *                                                                 *
 *  mcotargt.h                                                     *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *  eXtremeDB runtime target platform definitions                  *
 *                                                                 *
 ***************************************************************** */
/*
 * ++
 *
 * PROJECT:   eXtremeDB
 *
 * SUBSYSTEM: eXtremeDB target platform definitions
 *
 * MODULE:    mcotargt.h
 *
 * ABSTRACT:  common definitions for schema compiler and runtime
 *
 *
 * HISTORY:
 *            1. 2000-2002   TP versions 1-2 created
 * --
 */

#ifndef MCO_TARGET__H__
    #define MCO_TARGET__H__

    #define MCO_TARGET_FLOAT_SUPPORTED   /* base float type supported in C runtime */

    #define MCO_TARGET_DOUBLE_SUPPORTED  /* base double type supported in C runtime */

    /* 32/64-bit autodetection */
    #if defined(__APPLE__)

    #if defined(__x86_64__) || defined(__ppc64__) || defined(__arm64__)
    #define MCO_PLATFORM_X64 1
    #elif defined(__i386__) || defined(__ppc__) || defined(__arm__)
    #undef MCO_PLATFORM_X64
    #else
    #error Target platform was not determined. Please set MCO_PLATFORM_X64 manually.
    #endif

    #endif /* __APPLE__ */


    #if defined(MCO_PLATFORM_X64) && !defined(MCO_LARGE_DATABASE_SUPPORT)
    #define MCO_LARGE_DATABASE_SUPPORT
    #define MCO_LARGE_DATABASE_SUPPORT_X64_AUTOENABLED
    #endif

    #if !defined(MCO_PLATFORM_X64) && defined(MCO_CFG_USE_DIRECT_POINTERS) && defined(MCO_LARGE_DATABASE_SUPPORT)
    #error Large database is not supported for 32-bit direct pointer configuration
    #endif

    #ifdef __sun
    #define RESTRICTED_TEST_ENVIRONMENT 1
    #define MCO_CFG_STRICT_ALIGNMENT 1
    #endif

    #ifndef MCO_CFG_STRICT_ALIGNMENT
    #if defined(__x86_64__) || defined(__i386__) || (defined (_WIN32) && (defined(_M_AMD64) || defined(_M_IX86)))
    #define MCO_CFG_STRICT_ALIGNMENT 0    /* align datatype on size boundary */
    #else
    #define MCO_CFG_STRICT_ALIGNMENT 1    /* align datatype on size boundary */
    #endif
    #endif

    #ifndef MCO_CFG_BUILTIN_MEMCPY
    #if defined(__linux__) || defined(__QNX__)
    #define MCO_CFG_BUILTIN_MEMCPY 1
    #else
    #define MCO_CFG_BUILTIN_MEMCPY 0
    #endif
    #endif


    /* indexes support */
    #define MCO_CFG_HASH_SUPPORT

    #if !defined(MCO_CONFIG_OVERRIDE_KDTREE)   && !defined(MCO_CFG_MINIMAL)
    #define MCO_CFG_KDTREE_SUPPORT
    #endif

    #if !defined(MCO_CONFIG_OVERRIDE_RTREE)    && !defined(MCO_CFG_MINIMAL)
    #define MCO_CFG_RTREE_SUPPORT
    #endif

    #if !defined(MCO_CONFIG_OVERRIDE_PATRICIA) && !defined(MCO_CFG_MINIMAL)
    #define MCO_CFG_PATRICIA_SUPPORT
    #endif

    /* pattern search support */
    #ifndef MCO_CFG_MINIMAL
        #define MCO_CFG_PATTERN_SEARCH
    #endif

    #if !defined(MCO_CONFIG_OVERRIDE_WCHAR) && !defined(__KERNEL__)
        /* #define MCO_CFG_WCHAR_SUPPORT  */
    #endif /* MCO_CONFIG_OVERRIDE_WCHAR */

    #if defined(_WINRT)
        #define MCO_CFG_WCHAR_SUPPORT
    #endif

    /*
     * choose to support custom string collations in indices
     */
    #define MCO_CFG_COLLATION_SUPPORT

    /* inlude keywords definitions, depends on compiler */
    /* default empty */
   #ifdef __GNUC__
   #define MCO_INLINE_PREFIX
   #define MCO_INLINE_SUFFIX
   #define MCO_ALIGN_16 __attribute__((aligned(16)))
/*
   #define MCO_INLINE_PREFIX __inline__
   #define MCO_INLINE_SUFFIX __attribute__((always_inline))
*/
   #define MCO_NORETURN_SUFFIX __attribute__((noreturn))
   #else
   #define MCO_INLINE_PREFIX
   #define MCO_INLINE_SUFFIX
   #define MCO_NORETURN_SUFFIX
   #define MCO_ALIGN_16
   #endif

    #if defined(_AIX) 
    #define MCO_DATABASE_DEFAULT_MAP_ADDRESS (void*)0x30000000
    #elif defined(__APPLE__)
    #define MCO_DATABASE_DEFAULT_MAP_ADDRESS (void*)0x200000000
    #else
    #define MCO_DATABASE_DEFAULT_MAP_ADDRESS (void*)0x20000000
    #endif

    #ifndef MCO_CFG_WITH_MATH_LIB
    #define MCO_CFG_WITH_MATH_LIB 1
    #endif

    #ifndef NO_FLOATING_POINT
    #ifdef _WIN32
      #include <float.h>
      #include <math.h>
      #if _MSC_VER < 1900 /* Visual Studio 2013 and earlier */
      #define isnan _isnan
      #endif
    #else
      #if MCO_CFG_WITH_MATH_LIB
      #include <math.h>
      #endif
      #if !defined(isnan) && (!defined(__GNUC__) || __GNUC__ < 6) && !defined(__USE_MISC)
        #ifdef __cplusplus
          static inline bool isnan(double x) { return x != x; }
        #else
          #define isnan(x) ((x) != (x))
        #endif
      #endif
    #endif
    #endif

    #if defined(_WIN32)
      #if defined(MCO_DLL_API_EXPORTING)
        #define MCO_DLL_API __declspec(dllexport)
      #elif defined(MCO_DLL_API_IMPORTING)
        #define MCO_DLL_API __declspec(dllimport)
      #else
        #define MCO_DLL_API
      #endif
    #else
      #define MCO_DLL_API
    #endif

#define MCO_TRACE_ENABLED 1

#endif

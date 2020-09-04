/*******************************************************************
 *                                                                 *
 *  config.h                                                      *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           *
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifndef __CONFIG_H__
    #define __CONFIG_H__

    #include <stddef.h>
    #include <stdlib.h>
    #include <string.h>
    #include <assert.h>
    #include <stdarg.h>
    #include <stdio.h>
    #include <errno.h>
    #include <time.h>

    #ifndef MCO_CFG_SQL_SUPPORT_STD_STRING
    #define MCO_CFG_SQL_SUPPORT_STD_STRING 1
    #endif

    #ifndef SQL_INTERPRETER_SUPPORT
    #define SQL_INTERPRETER_SUPPORT 1
    #endif

    #ifndef MCO_CFG_CSV_IMPORT_SUPPORT
    #define MCO_CFG_CSV_IMPORT_SUPPORT 1
    #endif

    #ifndef MCO_CFG_USE_EXCEPTIONS
    #define MCO_CFG_USE_EXCEPTIONS 1
    #endif

    #ifndef MCO_CFG_ARRAY_COPY_BUF_SIZE
    #define MCO_CFG_ARRAY_COPY_BUF_SIZE 1024
    #endif

    #ifndef MULTITHREADING_SUPPORT
    #define MULTITHREADING_SUPPORT 1
    #endif
    /*#define HAVE_LOCALTIME_R*/

    /*#define NO_FLOATING_POINT*/

    #ifndef MCO_CFG_XSQL_EXTENSIONS
    #define MCO_CFG_XSQL_EXTENSIONS 1
    #endif

    #ifndef MCO_CONFIG_OVERRIDE_WCHAR
        #define UNICODE_SUPPORT
    #endif /*MCO_CONFIG_OVERRIDE_WCHAR*/

    #ifndef MCO_CFG_SQL_MALLOC_PROFILE
    #define MCO_CFG_SQL_MALLOC_PROFILE 0
    #endif

    #ifndef MCO_CFG_RSQL_TRUNCATE_LARGE_SEQUENCES
    #define MCO_CFG_RSQL_TRUNCATE_LARGE_SEQUENCES 0
    #endif

    #ifndef MCO_CFG_PREALLOCATED_MATERIALIZE_BUFFER_SIZE
    #define MCO_CFG_PREALLOCATED_MATERIALIZE_BUFFER_SIZE (16*1024)
    #endif

    #ifndef MCO_CFG_DEFAULT_PARALLEL_WORKERS
    #define MCO_CFG_DEFAULT_PARALLEL_WORKERS 8
    #endif

    #include "stdtp.h"

#endif

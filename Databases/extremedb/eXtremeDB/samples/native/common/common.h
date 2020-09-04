/****************************************************************
 *                                                              *
 * Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                              *
 ****************************************************************/
#ifndef __MCO_SAMPLES_COMMON_H
#define __MCO_SAMPLES_COMMON_H

#ifdef _WIN32
#pragma warning ( disable:4311)
#pragma warning ( disable:4312)
#endif

#ifdef __cplusplus
extern "C" {
#endif 

#include <stdlib.h>
#include <stdio.h>
#if defined(_QNX) && defined(__cplusplus)
 #undef __cplusplus
 #include <string.h>
 #define __cplusplus
#else 
 #if !defined(_VXWORKS) || defined(_WRS_KERNEL) 
   #include <string.h>
 #else
   #ifdef __cplusplus
     } /* extern "C" */
   #endif 
   #include <string.h>
   #ifdef __cplusplus
     extern "C" {
   #endif 
 #endif
#endif


#include <time.h>

#include <mco.h>
#include "mcosmp.h"
#if defined(_LINUX) || defined(_SOLARIS) || defined(_QNX) || defined (_HPUX) || defined (_AIX)
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#endif
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <process.h>
#endif

#include <initos.h>

/* Define common pefix for database and log file names. This can be filesystem path. Default empty. */
#define FILE_PREFIX ""

/* Memory size adjusters */
/* We need to increase sample memory size for 64-bit configuration due to pointer and long int size multiplication */
#ifdef MCO_PLATFORM_X64
#define MEMORY_SIZE_X64_MULT 3
#define PAGE_SIZE_X64_MULT   2
#else
#define MEMORY_SIZE_X64_MULT 1
#define PAGE_SIZE_X64_MULT   1
#endif
/* We need to increase sample memory size for wchar datatype support */
#if !defined(_INTEGRITY) /* and other OSes with lack of wchar support */
#define MEMORY_SIZE_WCHAR_ADD 900 * 1024
#else
#define MEMORY_SIZE_WCHAR_ADD 0
#endif

/* Common database sizes for the sample. The values can be re-defined if needed. */
#define  DATABASE_SIZE        (600 * 1024 * MEMORY_SIZE_X64_MULT + MEMORY_SIZE_WCHAR_ADD)
#define  CACHE_SIZE           (300 * 1024 * MEMORY_SIZE_X64_MULT )
#define  MEMORY_PAGE_SIZE     (128 * PAGE_SIZE_X64_MULT )
#define  PSTORAGE_PAGE_SIZE   4096
#define  MAX_DB_CONNECTIONS   10  
#define  DATABASE_MAP_ADDRESS MCO_DATABASE_DEFAULT_MAP_ADDRESS


/* Translate error code to a printable message */
const char * mco_ret_string( MCO_RET rc, int * is_error );

typedef struct tag_sample_memory {
    unsigned int       n_dev;
    mco_device_t       dev[4];
} sample_memory_t;


/* Unified database creation routine */
MCO_RET sample_open_database( 
           const char * db_name,  /* name of the database */
           mco_dictionary_h dict, /* pointer to schema */
           mco_size_t db_sz,      /* size of memory segment for in-mem part of the db */
           mco_size_t cache_sz,   /* size of cache segment for persistent storage */
           uint2 mem_pg_sz,       /* size of memory page */
           uint2 pstorage_pg_sz,  /* size of persistent storage page */
           uint2 max_conn_no,     /* max. number of connections */
           sample_memory_t * pdev);

/* Unified database loading routine */
MCO_RET sample_load_database( 
           void* stream_handle,   /* database input stream handle */
           mco_stream_read input_stream_reader, /* database input stream reader */
           const char * db_name,  /* name of the database */
           mco_dictionary_h dict, /* pointer to schema */
           mco_size_t db_sz,      /* size of memory segment for in-mem part of the db */
           mco_size_t cache_sz,   /* size of cache segment for persistent storage */
           uint2 mem_pg_sz,       /* size of memory page */
           uint2 pstorage_pg_sz,  /* size of persistent storage page */
           uint2 max_conn_no,     /* max. number of connections */
           sample_memory_t * pdev);

/* Unified database destruction routine */
MCO_RET sample_close_database( const char * db_name, sample_memory_t * pdev );

/* Pause the sample */
void sample_pause     (const char * prompt);
void sample_pause_end (const char * prompt);

/* Sample task control */
struct tag_sample_task;
typedef void (*sample_task_proc_t) (struct tag_sample_task*);

typedef struct tag_sample_task {
    
    void               * param;
    unsigned int         id;
    volatile int         stopped;
    const char         * db_name;
    mco_db_h             db_connection;
    sample_task_proc_t   proc;
    mco_thread_t       * t;
} sample_task_t;

unsigned int sample_start_task( sample_task_t * descr, sample_task_proc_t task, void * param );
unsigned int sample_start_connected_task( sample_task_t * descr, sample_task_proc_t task, const char * db_name, void * param );
unsigned int sample_stop_task ( sample_task_t * descr );
unsigned int sample_join_task ( sample_task_t * descr );

#define SAMPLE_OS_TASK_ID mco_process_t

SAMPLE_OS_TASK_ID sample_os_task_id_get();
int sample_os_task_id_check( SAMPLE_OS_TASK_ID tid );


int  sample_msec();
void sample_sleep( unsigned int duration_msec );

void sample_errhandler(MCO_RET n);

void sample_header(const char * descr);

void sample_show_runtime_info(const char * lead_line);

int sample_disk_supported();

void sample_show_device_info(const char * lead_line, mco_device_t dev[], int nDev);
void sample_rc_check(const char * msg, MCO_RET rc);

#ifdef _NETOS
int strcasecmp(const char *s1, const char *s2);
#endif

void dbg_exit (int ret);
#define CHECK(func)  { MCO_RET rc_ = func; if (rc_ != MCO_S_OK) { printf ("\nCall \"%s\" at %s:%d failed , code = %d\n\t(%s)\n", #func, __FILE__, __LINE__, rc_, mco_ret_string(rc_, 0)); dbg_exit (1); } }
#define CHECK_CODE(func, code)  { MCO_RET rc_ = func; if (rc_ != (code)) { printf ("\nCall \"" #func "\" at %s:%d failed , code = %d\n\t(%s)\n", __FILE__, __LINE__, rc_, mco_ret_string(rc_, 0)); dbg_exit (1); } }
#define CHECK_VALUE(code1, code2)  { if ((code1) != (code2)) { printf ("\nCheck at %s:%d failed , code1 = %d is not equal to code2 = %d\n", __FILE__, __LINE__, int(code1), int(code2)); dbg_exit (1); } }

#ifdef __cplusplus
} /* extern "C" */
#endif 

#endif /* __MCO_SAMPLES_COMMON_H */


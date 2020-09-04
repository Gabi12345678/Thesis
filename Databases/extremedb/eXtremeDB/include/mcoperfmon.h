/************************************************************************
 *                                                                      *
 *  mcoperfmon.h                                                        *
 *                                                                      *
 *  This file is a part of the eXtremeDB source code                    *
 *                                                                      *
 *  Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.          *
 *                                                                      *
 *  eXtremeDB performance monitoring public definitions and static API  *
 *                                                                      *
 ************************************************************************/

#ifndef _MCO_PERFMON_H_
#define _MCO_PERFMON_H_

#ifndef MCOPERF_API
# ifdef _WIN32
#  ifdef MCOPERF_DLL
#    define MCOPERF_API __declspec(dllexport)
#  else
#    ifdef MCOPERF_USE_DLL
#       define MCOPERF_API __declspec(dllimport)
#    else
#       define MCOPERF_API 
#    endif
#  endif
# else
#  define MCOPERF_API
# endif
#endif

#ifdef __cplusplus
extern "C"
    {
#endif 

#define MCO_PERF_COUNTER_INT           0
#define MCO_PERF_COUNTER_TIMING        1

/* Atomic counters by name */
#define MCO_PERF_ERROR_COUNT               0
#define MCO_PERF_CONNECTION_COUNT          1
#define MCO_PERF_CONNECTS                  2
#define MCO_PERF_DISCONNECTS               3

#define MCO_PERF_HOST_CPU_LOAD             4
#define MCO_PERF_HOST_MEMORY               5

#define MCO_PERF_OPEN_TRANS_COUNT      6
#define MCO_PERF_OPEN_CURSOR_COUNT     7

#define MCO_PERF_LAST_SNAPSHOT_SIZE        8

#define MCO_PERF_MAX_CONNECTIONS           9

#define MCO_PERF_IOT_SEND_BYTES            10
#define MCO_PERF_IOT_RECV_BYTES            11

#define MCO_PERF_TOTAL_COUNTERS            12

/*
    Database counters received by using regular eXDB API
*/

#define MCO_PERF_DB_TOTAL_PAGES               (MCO_PERF_TOTAL_COUNTERS + 0)
#define MCO_PERF_DB_FREE_PAGES                (MCO_PERF_TOTAL_COUNTERS + 1)
#define MCO_PERF_DB_USED_PAGES                (MCO_PERF_TOTAL_COUNTERS + 2)

#define MCO_PERF_DB_FILE_SIZE                 (MCO_PERF_TOTAL_COUNTERS + 3)
#define MCO_PERF_DB_LOG_FILE_SIZE             (MCO_PERF_TOTAL_COUNTERS + 4)
#define MCO_PERF_DB_USED_FILE_SIZE            (MCO_PERF_TOTAL_COUNTERS + 5)

#define MCO_PERF_DB_CACHE_HIT_COUNT           (MCO_PERF_TOTAL_COUNTERS + 6)
#define MCO_PERF_DB_CACHE_MISS_COUNT          (MCO_PERF_TOTAL_COUNTERS + 7)
#define MCO_PERF_DB_CACHE_PAGES_ALLOCATED     (MCO_PERF_TOTAL_COUNTERS + 8)
#define MCO_PERF_DB_CACHE_PAGES_USED          (MCO_PERF_TOTAL_COUNTERS + 9)
#define MCO_PERF_DB_CACHE_PAGES_PINNED        (MCO_PERF_TOTAL_COUNTERS + 10)
#define MCO_PERF_DB_CACHE_PAGES_MODIFIED      (MCO_PERF_TOTAL_COUNTERS + 11)
#define MCO_PERF_DB_CACHE_PAGES_DIRTY         (MCO_PERF_TOTAL_COUNTERS + 12)
#define MCO_PERF_DB_CACHE_PAGES_COPIED        (MCO_PERF_TOTAL_COUNTERS + 13)
#define MCO_PERF_DB_CACHE_PAGES_WRITE_DELAYED (MCO_PERF_TOTAL_COUNTERS + 14)

#define MCO_PERF_TOTAL_DB_COUNTERS			15

#define MCO_PERF_TOTAL_CMN_COUNTERS    1

/* Timing counters by name */

#define MCO_PERF_AVG_OFFS       1
#define MCO_PERF_MIN_OFFS       2
#define MCO_PERF_MAX_OFFS       3
#define MCO_PERF_TOTAL_OFFS     4

#define MCO_PERF_TIMERS_START    100

#define MCO_PERF_DISK_READ_TIME           (MCO_PERF_TIMERS_START+0)
#define MCO_PERF_DISK_WRITE_TIME          (MCO_PERF_TIMERS_START+1)
#define MCO_PERF_LOG_WRITE_TIME          (MCO_PERF_TIMERS_START+2)
#define MCO_PERF_COMMIT_TIME             (MCO_PERF_TIMERS_START+3)
#define MCO_PERF_ROLLBACK_TIME           (MCO_PERF_TIMERS_START+4)
#define MCO_PERF_SNAPSHOT_DURATION		 (MCO_PERF_TIMERS_START+5)

#define MCO_PERF_CLUSTER_NET_SEND_TIME    (MCO_PERF_TIMERS_START+6)
#define MCO_PERF_CLUSTER_NET_RECV_TIME    (MCO_PERF_TIMERS_START+7)

#define MCO_PERF_TOTAL_TIMINGS           8

#define MCO_PERFMON_MEMORY_PAGE_SIZE            128

typedef struct _mco_perf_options_tag
{
    int enabled;
    int monitor_interval_ms;
    int monitor_length_sec;
    int ui_update_rate_ms;
    int store_interval_sec;
} mco_perf_options_t;

typedef struct _mco_perf_db_handle_tag *mco_perf_h;

MCOPERF_API MCO_RET mco_perfmon_init(void *memory, mco_size_t memory_size, mco_db_params_t *db_params);
MCOPERF_API MCO_RET mco_perfmon_close(void);

MCOPERF_API MCO_RET mco_perfmon_attach(const char *dbName);
MCOPERF_API MCO_RET mco_perfmon_detach(const char *dbName);

MCOPERF_API const char *mco_perfmon_get_db_name(void);

MCOPERF_API mco_dictionary_h mco_perfmon_get_dictionary(void);

MCOPERF_API void mco_perfmon_set_options(mco_perf_options_t *options);
MCOPERF_API const mco_perf_options_t* mco_perfmon_get_options(void);

#ifdef __cplusplus
    }
#endif



#endif /* _MCO_PERFMON_H_ */

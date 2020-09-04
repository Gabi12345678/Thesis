#ifndef _MCO_HV_PERF_H_LOADED__
#define _MCO_HV_PERF_H_LOADED__

#ifndef MCOHVPERF_API
# ifdef _WIN32
#  ifdef MCOHVPERF_DLL
#    define MCOHVPERF_API __declspec(dllexport)
#  else
#    ifdef MCOHVPERF_USE_DLL
#       define MCOHVPERF_API __declspec(dllimport)
#    else
#       define MCOHVPERF_API 
#    endif
#  endif
# else
#  define MCOHVPERF_API
# endif
#endif


#ifdef __cplusplus
extern "C" {
#endif

MCOHVPERF_API int mcohv_perf_initialize(void);
MCOHVPERF_API int mcohv_perf_get_monitor_interval_ms(void);
MCOHVPERF_API int mcohv_perf_set_ui_update_rate_ms(int value);


#ifdef __cplusplus
}
#endif

#endif

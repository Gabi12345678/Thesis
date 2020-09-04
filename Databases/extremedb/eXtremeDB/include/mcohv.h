/************************************************************
 *                                                          *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.*
 *                                                          *
 ************************************************************/

#ifndef MCO_HTTPVIEWER
    #define MCO_HTTPVIEWER

#ifdef __cplusplus
extern "C" { 
#endif

#include <mcouda.h>

#ifndef MCOHV_API
# ifdef _WIN32
#  ifdef MCOHV_DLL
#    define MCOHV_API __declspec(dllexport)
#  else
#    ifdef MCOHV_USE_DLL
#       define MCOHV_API __declspec(dllimport)
#    else
#       define MCOHV_API 
#    endif
#  endif
# else
#  define MCOHV_API
# endif
#endif


typedef struct tag_mcohv_interface_def
{
	char* interface_addr;
	unsigned short port;

} mcohv_interface_def_t;

typedef struct tag_hv_query_param
{
	char *param;
	char *value;
	struct tag_hv_query_param *next;
} hv_query_param_t, *hv_query_param_p;

typedef void* mcohv_p;

MCOHV_API int mcohv_initialize(void);
MCOHV_API int mcohv_shutdown(void);
MCOHV_API int mcohv_start(mcohv_p* hv, mco_metadict_header_t *db, mcohv_interface_def_t* intf, unsigned int n_intf);
MCOHV_API int mcohv_start_connect(mcohv_p* hv, char* db_name, mcohv_interface_def_t* intf, unsigned int n_intf);
MCOHV_API int mcohv_stop(mcohv_p hv);

/***************************************************************************
    logger interface
 ***************************************************************************/

typedef enum hvlog_severity_values { HVLOG_DEBUG, HVLOG_INFO, HVLOG_ERROR } hvlog_severity;

typedef void (*loggerfn)(char *prefix, char *msg);

typedef struct _hvlogger_tag
{
  hvlog_severity level;
  loggerfn logger;
} hvlogger_t, *hvlogger_h;

  MCOHV_API void mcohv_set_logger(hvlogger_h logger);

  MCOHV_API MCO_RET mcohv_setoption(const char *option, const char *value);
  MCOHV_API const char *mcohv_getoption(const char *option);

  
#ifdef __cplusplus
}
#endif

#endif

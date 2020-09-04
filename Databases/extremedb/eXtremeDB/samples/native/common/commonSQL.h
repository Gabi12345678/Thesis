#ifndef __MCO_SQL_SAMPLES_COMMON_H
#define __MCO_SQL_SAMPLES_COMMON_H

#ifdef _WIN32
#pragma warning ( disable:4311)
#pragma warning ( disable:4312)
#endif

#ifdef __cplusplus
extern "C" {
#endif 


#include <sqlc.h>
#include <common.h>
/* definemapping address for eXtremeDB database */
#define MAP_ADDRESS DATABASE_MAP_ADDRESS
const char * sql_ret_string( MCO_RET rc );
void sample_sql_rc_check(const char * msg, MCO_RET rc);

#ifdef __cplusplus
} /* extern "C" */
#endif 

#endif /* __MCO_SQL_SAMPLES_COMMON_H */

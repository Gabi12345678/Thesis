/*******************************************************************
 *                                                                 *
 *  mcotime.h                                                          *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           * 
 *  All Rights Reserved                                            *
 *                                                                 *
 ***************************************************************** */
#ifndef MCO_MCOTIME_H__
#define MCO_MCOTIME_H__

#ifdef __cplusplus
extern "C"
{
#endif 

    #include "mcospec.h"

    /**
     * Operating system independent timer prototype to assure that timeouts and stream parameters 
     * are measured in the same units. 
     */
    typedef uint8 timer_unit; 

    typedef  timer_unit (*mco_system_get_current_time_fptr_t)(void);
    timer_unit mco_system_get_current_time(void);
    mco_system_get_current_time_fptr_t set_system_get_current_time_function(mco_system_get_current_time_fptr_t fptr);

    #define MCO_SYSTEM_GET_CURRENT_TIME_MSEC() (mco_system_get_current_time() / 1000)

    #define MCO_TM_INFINITE ((timer_unit) -1)

    typedef uint8 timer_unit_hires;
    timer_unit_hires mco_system_get_hi_res_freq(void);
    timer_unit_hires mco_system_get_hi_res_time(void);

    /* Convert to/from database datetime precision */
    mco_int8 mco_ticks2time(mco_int8 ticks, int scale);
    mco_int8 mco_time2ticks(mco_int8 time,  int scale);
#ifdef __cplusplus
}
#endif 

#endif
    

/****************************************************************
 *                                                              *
 *  mcosmp.h                                                    *
 *                                                              *
 *  This file is a part of the eXtremeDB source code.           *
 *                                                              *
 *  Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.  *
 *                                                              *
 *  Multitasking abstraction API                                * 
 *                                                              *
 ****************************************************************/

#ifndef __MCOSMP_H__
#define __MCOSMP_H__

#include <mco.h>
#include <mcotime.h>

#ifdef __cplusplus
extern "C"
{
#endif 

void sleep_msec(unsigned int duration);
void sleep_usec(timer_unit duration);

typedef void* mco_process_t;

typedef void (*mco_thread_proc_t)(void* arg);

typedef void (*mco_smp_error_handler_t)(char const* file, int line, char const* msg);
extern mco_smp_error_handler_t smp_error_handler;

typedef struct mco_tls_t { 
    void* (*get)(struct mco_tls_t* tls);
    void (*set)(struct mco_tls_t* tls, void* value);
    void (*destroy)(struct mco_tls_t* tls);
} mco_tls_t;

mco_tls_t* mco_create_tls();

typedef struct mco_thread_t { 
    void (*join)(struct mco_thread_t* id);
} mco_thread_t;

extern mco_size_t mco_thread_stack_size;

extern mco_thread_t* mco_create_thread(mco_thread_proc_t proc, void* arg);
extern void mco_join_thread(mco_thread_t* t);

typedef struct mco_mutex_t { 
    void (*lock)(struct mco_mutex_t* m);
    void (*unlock)(struct mco_mutex_t* m);
    void (*destroy)(struct mco_mutex_t* sem);
} mco_mutex_t;

extern mco_mutex_t* mco_create_mutex();
extern void mco_lock_mutex(mco_mutex_t* m);
extern void mco_unlock_mutex(mco_mutex_t* m);
extern void mco_destroy_mutex(mco_mutex_t* m);

typedef struct mco_semaphore_t { 
    mco_bool (*wait)(struct mco_semaphore_t* sem, mco_mutex_t* mutex, int n, timer_unit timeout);
    void (*signal)(struct mco_semaphore_t* sem, int n);
    void (*destroy)(struct mco_semaphore_t* sem);
} mco_semaphore_t;

extern mco_semaphore_t* mco_create_semaphore(int value);
extern mco_bool mco_semaphore_wait(mco_semaphore_t* s, mco_mutex_t* mutex, int n, timer_unit timeout);
extern void mco_semaphore_signal(mco_semaphore_t* s, int inc);
extern void mco_semaphore_destroy(mco_semaphore_t* s);

extern int mco_get_number_of_cpus();
extern mco_process_t mco_get_pid();
extern mco_process_t mco_get_thread_id();
extern mco_bool mco_is_process_alive(mco_process_t proc);

typedef void (*mco_job_t)(int thread_id, int n_threads, void* arg);
typedef void (*mco_job_callback_t)(void* arg, void* result);

typedef struct mco_thread_pool_t { 
    int  (*get_number_of_threads)(struct mco_thread_pool_t* pool);
    void (*execute)(struct mco_thread_pool_t* pool, mco_job_t job, int n_executors, void* arg);
    void (*merge)(struct mco_thread_pool_t* pool, mco_job_callback_t callback, void* result);
    void (*destroy)(struct mco_thread_pool_t* pool);
} mco_thread_pool_t;

extern mco_thread_pool_t* mco_create_thread_pool(int n_threads); /* 0 - choose number of threads automatically based on number of cores */

#ifdef __cplusplus
}
#endif 

#endif

/*****************************************************************
 *                                                               *
 *  mcolog.h                                                     *
 *                                                               *
 *  This file is a part of the eXtremeDB source code             *
 *                                                               *
 *  Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.   *
 *                                                               *
 *  eXtremeDB transaction log public definitions and static API  *
 *                                                               *
 *****************************************************************/

#ifndef _LOG_LOG_H_
    #define _LOG_LOG_H_

    #ifndef MCOLOG_API
    # ifdef _WIN32
    #  ifdef MCOLOG_DLL
    #    define MCOLOG_API __declspec(dllexport)
    #  else
    #    ifdef MCOLOG_USE_DLL
    #       define MCOLOG_API __declspec(dllimport)
    #    else
    #       define MCOLOG_API 
    #    endif
    #  endif
    # else
    #  define MCOLOG_API
    # endif
    #endif

    #ifdef __cplusplus
        extern "C"
        {
        #endif 

        #include "mco.h"
        #include "mcotime.h"

        #define MCO_TRANSLOG_ALL_LABELS     -1
        #define MCO_TRANSLOG_LABEL_MAX_SIZE 255

        /* prototype for the labels enumeration procedure */
        typedef void (*mco_TL_labels_proc)(const char *label, int label_id, void *user_ctx);

        /* information gathered from log file */
        typedef struct mco_TL_log_info_
        {
            mco_trans_counter_t start_trans_counter;        /* transaction counter at start of log  */
            mco_trans_counter_t stop_trans_counter;         /* transaction counter at end of log    */
            mco_trans_counter_t stored_transactions;        /* count of stored transactions         */
            uint2               disk_page_size;             /* size of single disk page             */
            uint2               page_size;                  /* size of in-memory database page      */
            mco_size_t          log_size;                   /* size of log in bytes                 */
            int                 labels_count;               /* count of labels it the log           */
            mco_bool            crc;                        /* if log covered with CRC32            */
            mco_bool            aligned;                    /* if log written with alignment        */
            mco_bool            dictionary_compat;          /* if log compatible with current dict. */
            mco_bool            transaction_apply_compat;   /* if log suitable to be applied        */
            mco_bool            transaction_append_compat;  /* if log suitable to be appended       */
            mco_bool            not_corrupted;              /* if log finished correctly            */
            mco_bool            iteratable;                 /* log is applicable for synchronization*/
                                                            /* with external database (saved with   */
                                                            /* flag MCO_TRANSLOG_ITERABLE)          */
        } mco_TL_log_info_t, *mco_TL_log_info_h;

        /* information about current log */
        typedef struct mco_TL_current_info_
        {
            uint2               flags;                      /* MCO_TL_FLAGS - transaction log flags */
            mco_trans_counter_t start_trans_counter;        /* transaction counter at start of log  */
            mco_trans_counter_t last_trans_counter;         /* current transaction counter          */
            mco_trans_counter_t stored_transactions;        /* count of stored transactions         */
            mco_size_t          log_size;                   /* size of log in bytes                 */
            mco_size_t          pipe_used_size;             /* size of data currently placed in a   */
                                                            /* pipe in bytes                        */
            uint2               pipe_readers_connected;     /* count of currently connected readers */
            int                 labels_count;               /* count of labels it the log           */
            uint4               max_parallel_trans;         /* max logging slots used for parallel  */
                                                            /* transaction commits in MVCC. (Total  */
                                                            /* number of slots is max_connections*2)*/
            uint4               exceeded_trans_slots_count; /* count of times when all of the of    */
                                                            /* logging slots were exceeded          */
            uint4               max_transaction_size;       /* size of largest transaction          */
        } mco_TL_current_info_t, *mco_TL_current_info_h;

        /* logging modes */
        enum MCO_TL_FLAGS
        {
            MCO_TRANSLOG_ALIGNED                = 0x01,   /* start each record with new disk page     */
            MCO_TRANSLOG_APPEND                 = 0x02,   /* append to an existing log file           */
            MCO_TRANSLOG_CRC                    = 0x04,   /* cover transaction data with CRC32        */
            MCO_TRANSLOG_SIZE_CLBK              = 0x08,   /* user call-back function to limit size    */
            MCO_TRANSLOG_SYNC_INSTANTLY         = 0x10,   /* do disk flush on each record             */
            MCO_TRANSLOG_SYNC_TIMER             = 0x20,   /* do disk flush by timer                   */
            MCO_TRANSLOG_SYNC_COUNT             = 0x40,   /* do disk flush by record count            */
            MCO_TRANSLOG_ITERABLE               = 0x80,   /* make log applicable for synchronization  */
                                                          /* with external database by function       */
                                                          /* mco_translog_iterate (will increase size */
                                                          /* of log file)                             */
            MCO_TRANSLOG_RESTART                = 0x100,  /* stop previous log at once                */
            MCO_TRANSLOG_PIPE                   = 0x200,  /* use pipe interface instead of file       */
            MCO_TRANSLOG_DUAL_OUT               = 0x400,  /* duplicate data into a local file if pipe */
                                                          /* interface used                           */
            MCO_TRANSLOG_EVENT_MASK             = 0x800,  /* take event_mask field into account       */
            MCO_TRANSLOG_DYNAMIC_PIPE           = 0x1000, /* use dynamic pipes                        */
            MCO_TRANSLOG_PREREAD_PIPE           = 0x2000  /* pre-read pipe to internal buffer to      */
                                                          /* reduce count of blocking operations      */
        };

        typedef struct mco_TL_flush_timer_
        {
            timer_unit flush_time;
            timer_unit time_elapsed;
        } mco_TL_flush_timer_t, *mco_TL_flush_timer_h;

        /* prototype for the external flush timer procedure:                */
        /*   The external flush timer procedure flushtimer_proc()           */
        /*   MUST signal that the specified amount of time has expired,     */
        /*   then the current contents of LOG file buffer will be flushed.  */
        typedef mco_bool(*mco_TL_timer_proc)(mco_TL_flush_timer_h handle);

        /* prototype for the procedure that warns when the log size limit is reached */
        typedef void (*mco_TL_warn_sz_proc)(mco_size_t log_size);

        typedef uint8 mco_event_mask_t;

        #define MCO_LOG_MASK_ALL_EVENTS  ((mco_event_mask_t)~0)
        #define MCO_LOG_MASK_ALL_CHANGES (0)

        /* logging parameters */
        typedef struct mco_TL_start_data
        {
            uint2               flags;          /* MCO_TL_FLAGS - transaction log flags  */
            uint2               disk_page_size; /* size of single disk page              */
            uint2               flush_depth;    /* maximum count of not flushed records  */ 
            timer_unit          flush_time;     /* time lag between flushed records      */
            mco_TL_timer_proc   timer_proc;     /* user defined external timer procedure.*/ 
                                                /* (MUST signal when the specified amount*/
                                                /* of time has elapsed)                  */
            mco_size_t          max_size;       /* max size of log (warn when reached)   */
            mco_TL_warn_sz_proc warn_sz_proc;   /* user defined procedure that warns     */
                                                /* when maximum size is reached (MUST NOT*/
                                                /* call any eXtremeDB functions)         */
            char const          *dual_log_path; /* path to log file in dual out mode     */
            mco_event_mask_t    event_mask;     /* mask of logged events
                                                 * MCO_LOG_MASK_ALL_CHANGES: all transaction changes will be logged
                                                 * MCO_LOG_MASK_ALL_EVENTS: then only changes cause triggering of any of declared events will be logged
                                                 * otherwise mask of logged event identifiers (MCO_EVENT_*)
                                                 */
        } mco_TL_start_data_t, *mco_TL_start_data_h;

        /*
         * Initialize Transaction Logging subsystem
         */
        MCOLOG_API MCO_RET mco_translog_init ();

        /* 
         * Start logging with user defined parameters
         */
        MCOLOG_API MCO_RET mco_translog_start (mco_db_h db, char const *log_path,
                                                    mco_TL_start_data_h data);

        /*
         * Stop logging and close log file
         */
        MCOLOG_API MCO_RET mco_translog_stop (mco_db_h db);

        /*
         * Terminate logging if reader in pipe mode got malfunctioned
         */
        MCOLOG_API MCO_RET mco_translog_terminate (mco_db_h db);

        /*
         * Mark label on the log at current position (to allow replay of log up to current moment)
         */
        MCOLOG_API MCO_RET mco_translog_label (mco_db_h db, const char *label, int *label_id);

        /*
         * Truncate log and start it again (same as stop/start sequence)
         */
        MCOLOG_API MCO_RET mco_translog_truncate (mco_db_h db);

        /*
         * Create database snapshot together with log truncation. This function guarantee
         * both operations (mco_db_save and mco_translog_truncate) in a single blocking
         * to avoid missed transactions
         */
        MCOLOG_API MCO_RET mco_translog_db_save (void* stream_handle, mco_stream_write output_stream_writer, mco_db_h db);

        /*
         * Flush log immediately
         */
        MCOLOG_API MCO_RET mco_translog_flush (mco_db_h db);

        /*
         * Get information about the currently running log process
         */
        MCOLOG_API MCO_RET mco_translog_get_info (mco_db_h db, mco_TL_current_info_h info);

        /*
         * Apply transactions from the log to the database up to the end of log or to
         * the specified label id
         */
        MCOLOG_API MCO_RET mco_translog_apply (mco_db_h db, char const *file_path, int label_id);

        /*
         * Query labels in the log file through user's call-back function
         */
        MCOLOG_API MCO_RET mco_translog_query_info (mco_db_h db, char const *file_path,
                    mco_TL_labels_proc labels_proc, void *user_ctx, mco_TL_log_info_h info);

        /*
         * Traverse the log file to transfer saved modifications into an external DataBase.
         * This provides an asynchronous restore method to complement the synchronous method 
         * implemented by function mco_trans_iterate and mco_trans_iterator_callback_t callback.
         */
        MCOLOG_API MCO_RET mco_translog_iterate (char const *file_path, mco_db_h pipe_db,
                    mco_trans_iterator_callback_t iteration_proc,
                    void *user_ctx, mco_dictionary_h dict, void *mem_ptr, mco_size_t mem_size);

        typedef MCO_RET (*mco_translog_register_event_handlers_t)(mco_trans_h trans, void *user_ctx);

		MCOLOG_API MCO_RET mco_translog_play (char const *src_file_path, mco_db_h pipe_db,
                                   mco_trans_iterator_callback_t iteration_proc,
                                   void *iterproc_user_ctx, mco_dictionary_h dict,
                                   void *mem_ptr, mco_size_t mem_size,
                                   mco_translog_register_event_handlers_t register_callback,
                                   void *regevent_user_ctx);

        typedef struct mco_TL_play_params_
        {
            char const                             *src_file_path; 
            mco_db_h                                pipe_db; 
            mco_device_t                           *pipe_device;
            mco_trans_iterator_callback_t           iteration_proc;
            void                                   *iterproc_user_ctx;
            mco_dictionary_h                        dict;
            void                                   *mem_ptr; 
            mco_size_t                              mem_size;
            mco_translog_register_event_handlers_t  register_callback;
            void                                   *regevent_user_ctx;
            mco_size_t                              ddl_dict_size;
            int                                     ddl_dict_flags;
            int                                     max_classes;
            int                                     max_indexes;
        } mco_TL_play_params_t, *mco_TL_play_params_h;

        MCOLOG_API MCO_RET mco_translog_play_params_init(mco_TL_play_params_h tl_play_params);

        MCOLOG_API MCO_RET mco_translog_play_ex(mco_TL_play_params_h tl_play_params);

        /*
         * Interrupt reading log (pipe source only) outside of functions mco_translog_iterate,
         * mco_translog_play or mco_translog_play_ex. Database connection handle 'db' must be the handle
         * previously passed into proper log reading function.
         */
        MCOLOG_API MCO_RET mco_translog_play_stop (mco_db_h db);

        #ifdef __cplusplus
        }
    #endif 

#endif /* _LOG_LOG_H_ */

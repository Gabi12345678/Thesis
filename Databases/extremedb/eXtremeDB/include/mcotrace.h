#ifndef __MCO__TRACE_H_LOADED__
#define __MCO__TRACE_H_LOADED__ 1

#include <mco.h>
#include <stdio.h>

#ifdef __cplusplus
    extern "C"
    {
    #endif

#define TRACE_HANDLER_SIZE   3

#ifndef MAX_PATH
 #define MAX_PATH 1024
#endif

#define MCO_TRACE_MAX_FORMAT 256

typedef enum mco_trace_file_appender_options_tag {
    MCO_TRACE_LEAVE_OPENED,     /* Leave file opened till next record */
    MCO_TRACE_KEEP_CLOSED       /* Keep file closed, reopen for each record */
} mco_trace_file_appender_options;

typedef enum mco_trace_rotate_policy_tag {
    MCO_TRACE_NO_ROTATE, MCO_TRACE_ROTATE_SIZE, MCO_TRACE_ROTATE_TIME
} mco_trace_rotate_policy;


MCO_RET mco_trace_init_simple_handler(const char *filename, mco_trace_file_appender_options close_policy, const char *format);
void mco_trace_close_simple_handler(void);

/*
 *
 *   Asynchronous Tracing interface
 */

        /* Tracing message */
        typedef struct mco_trace_message_tag
        {
            /* Message severity */
            mco_severity severity;
            uint8 subsystem;

                /*  time stamp when the event occured, as returned by mco_system_get_current_time(). In mosts systems it has microsecond resolution
                */
            uint8 stamp;

            /* Message text if available, otherwise it's NULL. For sync trace handler it's pointer to caller buffer. For async handler it's allocated dynamically using malloc(), it's hander responsibility to call free() when message is reported.
            */
            char *message;
            /*
                Source file, __FILE__ macro.
            */

            const char *file;
            /*
            Source line, __LINE__ macro
            */
            int line;

        } mco_trace_message;

      typedef void *mco_async_trace_h;
      
      typedef void (*mco_trace_async_start_handler_fn)(mco_async_trace_h hdl, void *arg);
      typedef void (*mco_trace_async_stop_handler_fn)(mco_async_trace_h hdl, void *arg);
      typedef void (*mco_trace_async_message_handler_fn)(mco_async_trace_h hdl, mco_trace_message *msg, void *arg);

typedef struct mco_async_trace_message_hander_desc_tag
{
    mco_severity severity;
    mco_trace_async_start_handler_fn start_hdl;
    mco_trace_async_stop_handler_fn stop_hdl;
    mco_trace_async_message_handler_fn msg_hdl;
    struct mco_async_trace_message_hander_desc_tag *next;
    mco_bool started;
    void *arg;
} mco_async_trace_message_hander_desc_t, *mco_async_trace_message_hander_desc_h;

      
typedef struct mco_trace_head_tag
{
    mco_async_trace_message_hander_desc_h hdl_chain;
} *mco_trace_head_p, *mco_trace_db_handle_h;

/*
 * descriptor for file appender
 */
typedef struct mco_trace_file_appender_tag
{
    /* Filled by mco_trace_file_appender_init, do not touch */
    mco_async_trace_message_hander_desc_t head;
    uint8 create_time;
    size_t file_size;
    FILE *fp;

    /* Below are parameters for appender */

    /* file name to write */
    char fname[MAX_PATH];

    /* Optional format for messages. */
    char *format;

    mco_trace_file_appender_options options;

    mco_trace_rotate_policy policy;
    size_t file_max_size_bytes;
    uint4 file_rotate_period_sec;
} mco_trace_file_appender_t;

MCO_RET mco_trace_file_appender_init(mco_trace_file_appender_t *a);
void mco_trace_file_handler(mco_async_trace_h hdl, mco_trace_message *msg, void *arg);
void mco_trace_file_handler_start(mco_async_trace_h hdl, void *arg);
void mco_trace_file_handler_stop(mco_async_trace_h hdl, void *arg);


MCO_RET mco_trace_init(void);
MCO_RET mco_trace_stop(void);

/*
 *  Add asynchronous trace handler
 */
MCO_RET mco_trace_add_async_handler(mco_async_trace_message_hander_desc_h hdl, mco_severity severity, void *arg);


/* Interface for reading trace file */
MCO_RET mco_trace_simple_file_size(size_t *size);
MCO_RET mco_trace_simple_trace_read(long offset, void *buf, size_t *size);


void mco_int_trace_core_init(void);

#ifdef __cplusplus
}
#endif

#endif

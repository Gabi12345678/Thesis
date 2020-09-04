/****************************************************************
 *                                                              *
 *  mcoiter.h                                                   *
 *                                                              *
 *  This file is a part of the eXtremeDB source code.           *
 *                                                              *
 *  Copyright (c) 2001-2019 McObject LLC. All Rights Reserved.  *
 *                                                              *
 *  eXtremeDB transaction iterator implementations              *
 *                                                              *
 ****************************************************************/
#ifndef MCO_ITERATOR_H__
#define MCO_ITERATOR_H__

#include <mco.h>
#include <mconet.h>

#ifdef _WIN32
#  ifdef MCOITER_DLL
#    define MCOITER_API __declspec(dllexport)
#  else
#    ifdef MCOITER_USE_DLL
#       define MCOITER_API __declspec(dllimport)
#    else
#       define MCOITER_API 
#    endif
#  endif
#else
#  define MCOITER_API
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* Streams */
typedef struct mco_stream_t {
    mco_stream_write writer;
    MCO_RET          (*close) (struct mco_stream_t *self);
    MCO_RET          last_error;
    int              last_errno;
} mco_stream_t, *mco_stream_h;

/* FILE stream */
typedef struct {
    MCO_RET          last_error;
    int              last_errno;
} mco_file_stream_params_t;

MCOITER_API void mco_file_stream_params_init(mco_file_stream_params_t *params);
/* mco_file_t implementation */
MCOITER_API mco_stream_h mco_create_mcofile_stream(const char *filename, mco_file_stream_params_t *params);
/* stdio implementation */
MCOITER_API mco_stream_h mco_create_stdfile_stream(const char *filename, mco_file_stream_params_t *params);

/* Server socket stream */
/* Additional parameters */
typedef struct {
    timer_unit        write_timeout;
    mco_size_t        buffer_size;
    mco_size_t        max_clients;
    const char       *net_interface;
    mco_sock_params_t sock_params;
    MCO_RET           last_error;
    int               last_errno;
} mco_server_socket_stream_params_t;

MCOITER_API void mco_server_socket_stream_params_init(mco_server_socket_stream_params_t *params);
MCOITER_API mco_stream_h mco_create_server_socket_stream(int port, mco_server_socket_stream_params_t *params);

/* Client socket stream */
/* Additional parameters */
typedef struct {
    timer_unit        write_timeout;
    timer_unit        connect_timeout;
    int               connect_attempts;
    timer_unit        connect_interval;
    mco_bool          auto_reconnect;
    mco_size_t        buffer_size;
    mco_sock_params_t sock_params;
    MCO_RET           last_error;
    int               last_errno;
} mco_client_socket_stream_params_t;

MCOITER_API void mco_client_socket_stream_params_init(mco_client_socket_stream_params_t *params);
MCOITER_API mco_stream_h mco_create_client_socket_stream(const char *hostname, int port, mco_client_socket_stream_params_t *params);

/* NULL stream */
MCOITER_API mco_stream_h mco_create_null_stream(void);

/* User-defined stream */
MCOITER_API mco_stream_h mco_create_custom_stream(void* stream_handle, mco_stream_write output_stream_writer, MCO_RET(*close)(void *));

MCOITER_API mco_stream_h mco_create_buffered_stream(mco_stream_h orig, mco_size_t buffer_size, MCO_RET *last_error, int *last_errno);

/* Duplicate stream */
MCOITER_API mco_stream_h mco_create_tee_stream(mco_stream_h stream1, mco_stream_h stream2, mco_bool any_ok);

/* Destroy this stream and all chained streams */
MCOITER_API MCO_RET mco_destroy_stream(mco_stream_h s);

/* Iterators */
typedef struct mco_trans_iterator_t {
    mco_trans_iterator_callback_t callback;
    MCO_RET                       (*close) (struct mco_trans_iterator_t *self);
    MCO_RET                       last_error;
    int                           last_errno;
} mco_trans_iterator_t, *mco_trans_iterator_h;

/* JSON converter */
typedef struct {
    mco_bool         compact;
    mco_bool         ignore_stream_errors;
    MCO_RET          last_error;
    int              last_errno;
} mco_json_converter_params_t;

MCOITER_API void mco_json_converter_params_init(mco_json_converter_params_t *params);
MCOITER_API mco_trans_iterator_h mco_create_json_converter(mco_stream_h stream, mco_json_converter_params_t *params);

/* Destroy iterator and underlying stream(s) */
MCOITER_API MCO_RET mco_destroy_trans_iterator(mco_trans_iterator_h t);

#ifdef __cplusplus
}
#endif
#endif

/************************************************************
 *                                                          *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.*
 *                                                          *
 ************************************************************/

#ifndef _MCO_EWS_CONFIG_H
#define _MCO_EWS_CONFIG_H


/*
 *
 * Debug level
 *
 */
/*#define MCO_TRANSPORT_LAYER_DEBUG*/
/*#define MCO_BUFFERED_TRANSPORT_LAYER_DEBUG*/
/*#define MCO_HTTP_LAYER_DEBUG*/

/*
 *
 * Buffered layer
 *
 */
#define MCO_HTTP_BUFFERED_CONNECTIONS

/*
 *
 * Inprocess CGI buffering strategy
 *
 */
#define MCO_HTTP_CGI_PARTIAL_BUFFERING
/*#define MCO_HTTP_CGI_FULL_BUFFERING*/

/*
 *
 * Runtime behavior
 *
 */
#define MCO_HTTP_TOOLS
#define MCO_HTTP_10
#define MCO_HTTP_SEND_ERRORS

#ifndef MCO_HTTP_OVERRIDE_CONFIG

/*#define MCO_HTTP_SESSION_SUPPORT*/

/*#define MCO_HTTP_NO_BASIC_AUTH*/

#endif /*MCO_HTTP_OVERRIDE_CONFIG*/

/*
 *
 * Runtime limitations
 *
 */
#define MCO_HTTP_CGI_BUFFER_SIZE                 64*1024
#define MCO_HTTP_HEADER_SIZE_LIMIT               1024
#define MCO_HTTP_SESSION_IDENTIFIER_SIZE         16
#define MCO_HTTP_BACKLOG                         SOMAXCONN
#define MCO_TRANSPORT_LAYER_LINGER_TIMEOUT       10
#define MCO_HTTP_SECURED_CONNECTIONS_IO_BUFFER_SIZE 1024*16

#ifndef MCOEWS_API
# ifdef _WIN32
#  ifdef MCOEWS_DLL
#    define MCOEWS_API __declspec(dllexport)
#  else
#    ifdef MCOEWS_USE_DLL
#       define MCOEWS_API __declspec(dllimport)
#    else
#       define MCOEWS_API 
#    endif
#  endif
# else
#  define MCOEWS_API
# endif
#endif

#endif /* _MCO_EWS_CONFIG_H*/

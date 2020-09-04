/************************************************************
 *                                                          *
 * Copyright (c) 2001-2019 McObject LLC. All Right Reserved.*
 *                                                          *
 ************************************************************/

#ifndef _MCO_EWS_HTTP_SUPPORT_H
#define _MCO_EWS_HTTP_SUPPORT_H

#include <mcowscfg.h>
#include <mcowserr.h>
#include <mcotlc.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************************
 *
 * The eXtremeWS's core structures and prototypes
 *
 *****************************************************************************************/

/*
 * The runtime's timestamp structure
 */
#ifdef WIN32
#pragma pack(push)
#pragma pack(1)
#endif /* WIN32 */

typedef struct tag_mcohs_datetime
{
    unsigned char sec:6;    /* seconds after the minute - [0,59] */
    unsigned char min:6;    /* minutes after the hour - [0,59] */
    unsigned char hour:5;   /* hours since midnight - [0,23] */
    unsigned char mday:5;   /* day of the month - [1,31] */
    unsigned char mon:4;    /* months since January - [0,11] */
    unsigned char year:8;   /* years since 1900 */
    unsigned char wday:3;   /* days since Sunday - [0,6] */
} mcohs_datetime_t, *mcohs_datetime_h;
#ifdef WIN32
#pragma pack(pop)
#endif

/*
 * The prototype of an application defined routine for timestamp generating.
 * The eXtrmeWS runtime will call it casually for time-related needs.
 * An application must implement following routine.
 */
MCOEWS_API void hs_app_get_current_datetime(mcohs_datetime_h dt);

/*
 * The prototype of application defined string for the application's customization.
 * The eXtremeWS runtime library will add it's value to the 'Server:'
 * response header field.
 *
 * char hs_app_title[] = "Custom application v1.1";
 */

/*
 * Session stucture
 */
#ifdef MCO_HTTP_SESSION_SUPPORT
#ifdef WIN32
#pragma pack(push)
#pragma pack(1)
#endif
typedef struct tag_mcohs_session
{
    unsigned char       id[MCO_HTTP_SESSION_IDENTIFIER_SIZE];
    mcohs_datetime_t    hit;
} mcohs_session_t, *mcohs_session_h;
#ifdef WIN32
#pragma pack(pop)
#endif
#endif /* MCO_HTTP_SESSION_SUPPORT */

/*
 *
 * The request-context structure
 * This structure will be passed as parameter into http request handler routine.
 *
 */
typedef struct tag_mcohs_request
{
    mcohs_tlc_addr_h    intf_addr;         /* interface address */
    mcohs_tlc_addr_h    peer_addr;         /* peer address */

    char                *method;            /* HTTP request method (GET, POST,...)*/
    char                *URL;
    char                *ver;               /* HTTP protocol version*/
    unsigned int        content_length;    /* length of optional content*/
    char*               attrs;             /* request's attributes. */
                        /* Use mcohs_get_attr(...) and mcohs_get_next_attr(...) */
                        /* routines to deal with this values.*/
    void                *intf_param;        /* 'param' field value of a interface structure*/
    void                *connection_param;  /* 'param' field value of a connection structure*/
    void                *internal;          /* Don't use this field*/
#ifdef MCO_HTTP_SESSION_SUPPORT
    mcohs_session_h     session;            /* application defined session handler*/
#endif /* MCO_HTTP_SESSION_SUPPORT */

    char                *auth_type;         /* authorization type*/
    char                *auth_user_name;    /* user's name */
    char                *auth_user_password;/* user's password*/
} mcohs_request_t, *mcohs_request_h;

/*
 *
 * The response-context structure
 * A http response handler should fill fields of this structure with 
 * appropriate data.
 *
 */

/* cache control flags */
#define MCO_HTTP_CACHABLE_CONTENT   0x1  /* A browser (user agent) may cache the response content*/
#define MCO_HTTP_PRIVATE_CONTENT    0x2  /* A browser shouldn't cache the response content in public store.*/

/* the structure */
typedef struct tag_mcohs_response
{
    unsigned short  status_code;        /* HTTP status code: 200, 404, 5xx, ...*/
    char            *reason_phrase;     /* HTTP reason phrase: "OK", ""File not found", "Internal error" respectively*/
    mcohs_datetime_t modification_date; /* Date and time of the last content's modification*/
    char            *charset;           /* Character set of the content. May be NULL for defaul character set*/
    char            *content_type;      /* MIME type of the content: 'text/html', 'image/gif', ...*/
    char            *content;           /* pointer to a response's content*/
    unsigned int    content_length;     /* length of the content*/
    unsigned char   flags;              /* response cache control flags, see above*/

    char            *extra_header;      /* extra headers ended by CRLF*/

    char            *auth_realm;        /* request for authorization*/
} mcohs_response_t, *mcohs_response_h;

/*
 * The application MUST implement at least one HTTP request handler.
 * The handler should return one of the following completion codes after processing 
 * of every incoming requests.
 */
#define MCO_HTTP_OK                         0
#define MCO_HTTP_SEND_AND_DROP_CONNECTION   1
#define MCO_HTTP_DROP_CONNECTION            2

/*
 * Prototype of a application defined http request handler in 'inprocess CGI' mode.
 *
 * Input : request-context.
 *         The handler should use mcohs_printf() and mcohs_put() routines for
 *         counstucting content.
 * Output: completion code
 *  MCO_HTTP_OK                       - The runtime should send the content and
 *                                      waits for next request
 *  MCO_HTTP_SEND_AND_DROP_CONNECTION - The runtime should send the content and
 *                                      terminate the connection.
 *  MCO_HTTP_DROP_CONNECTION          - The runtime must drop the connection without
 *                                      sending anything to the user agent.
 */
typedef int (*mcohs_http_handler_proc)(mcohs_request_h request);

/*
 *
 * The 'interface' structure
 * This structure defines an network interface for the runtime
 *
 */
#ifdef WIN32
#pragma pack(push)
#pragma pack(1)
#endif

typedef struct tag_mcohs_interface
{
    unsigned int            flags;              /* see below */
    mcohs_tlc_addr_t        addr;               /* network address of the interface */
    mcohs_http_handler_proc request_handler;    /* an application defined routine for */
                                                /* request processing */
    char                    *default_charset;   /* response's default charset. see below */

    mcohs_tlcs_init_h       security;           /* security-related data (keys, certificates) */

    void                    *param;             /* user defined interface parameter */
                                                /* will be passed to a request handler as */
                                                /* 'intf_param' field */

    unsigned int            default_charset_len;
    mco_tlcs_t              ch;

    mco_tlcs_keys_t         keys;
} mcohs_interface_t, *mcohs_interface_h;

/* examples of charsets */
#define CHARSET_ISO_8859_1  "ISO-8859-1"
#define CHARSET_ISO_8859_5  "ISO-8859-5"
#define CHARSET_KOI8R       "KOI8-R"

/* interface flags
 *
 * INTERFACE_CLEAN   - the structure initialized and ready for the runtime
 * INTERFACE_SECURED - the interface should be secured
 */
#define INTERFACE_CLEAN          0
#define INTERFACE_BUFFERED       0x100


typedef struct tag_mcohs_connection
{
    unsigned int        state;         /* state of the connection. see below*/
    long                inactive_time; /* time of 'inactivity' of the connection*/
    void                *param;
                                /* user defined parameter. This parameter*/
                                /* will be passed as 'connection_param' field*/
                                /* to a request handler*/

    mco_tlcs_t          ch;
    mcohs_interface_h   intf;
    unsigned int        eaten;

    unsigned int        cgi_sz;
    char                *cgi;
} mcohs_connection_t, *mcohs_connection_h;

/*
 * Connection's flags
 *
 * CONNECTION_CLEAN - connection structure is clean and ready for using by runtime
 */
#define CONNECTION_CLEAN        0

#ifdef WIN32
#pragma pack(pop)
#endif

/*
 *
 * Some casual strings
 *
 */
extern MCOEWS_API const char * mcohs_text_html;                    /* "text/html"             */
extern MCOEWS_API const char * mcohs_phrase_ok;                    /* "OK"                    */
extern MCOEWS_API const char * mcohs_phrase_internal_server_error; /* "Internal Server Error" */
extern MCOEWS_API const char * mcohs_phrase_bad_request;           /* "Bad request"           */


/*****************************************************************************************
 *
 * eXtremeWS API
 *
 *****************************************************************************************/

/*****************************************************************************************
 * Meaning of the error codes:
 *
 *  HS_OK                     - The operation complete successfully.
 *  HS_CANT_INIT              - The operation couldn't complete during some reason.
 *  HS_EXECUTE                - An incoming connection was detected and registered.
 *                              The applicaton should start connection processor or 
 *                              decline the connection.
 *  HS_TIMEOUT                - The operation was timed out.
 *  HS_FINISHED               - The connection or interface was finished completely.
 *                              The application can stop a processing thread and 
 *                              release any associated resources.
 *  HS_HTTP_HEADER_TOO_BIG    - The HTTP header longer then MCO_HTTP_HEADER_SIZE_LIMIT.
 *  HS_INVALID_REQUEST_LINE   - An invalid HTTP request line was detected.
 *  HS_INVALID_REQUEST_HEADER - An invalid HTTP request header was detected.
 *
 *****************************************************************************************/
/*
 * The main initialization function.
 * This routine must be called before the first call any other eXtremeWS routine.
 *
 * Input:
 *  mco_mem_h mem - handler for memory manager. Zero in common.
 *
 * Output:
 *  none
 *
 * Results:
 *  HS_OK        - success
 *  HS_CANT_INIT - fail
 */
MCOEWS_API int mcohs_runtime_start();

/*
 * The main cleanup function.
 * This routine must be called after the last call of any other eXtremeWS routine.
 *
 * Input:
 *  none
 *
 * Output:
 *  none
 *
 * Results:
 *  HS_OK        - success
 *  HS_CANT_INIT - fail
 */
MCOEWS_API int mcohs_runtime_stop();

/*
 * This routine starts processing of given interface.
 * The parameter must be cleared and properly initialized before call.
 *
 * Input:
 *  mcohs_interface_h intf - pointer to filled interface data structure
 *
 * Output:
 *  none
 *
 * Results:
 *  HS_OK        - success
 *  HS_CANT_INIT - fail
 */
MCOEWS_API int mcohs_interface_start(mcohs_interface_h intf);

/*
 * This routine stops processing of given interface.
 * The interface must be started by mcohs_interface_start() routine.
 *
 * Input:
 *  mcohs_interface_h intf - pointer to initialized interface data structure
 *
 * Output:
 *  none
 *
 * Results:
 *  HS_OK        - success
 *  HS_CANT_INIT - fail
 */
MCOEWS_API int mcohs_interface_stop(mcohs_interface_h intf);

/*
 * This routine stops processing of given interface for forked child.
 *
 * Input:
 *  mcohs_interface_h intf - pointer to initialized interface data structure
 *
 * Output:
 *  none
 *
 * Results:
 *  HS_OK        - success
 *  HS_CANT_INIT - fail
 */
MCOEWS_API int mcohs_interface_detach(mcohs_interface_h intf);

/*
 * The mcohs_connection_check() routine checks the 'intf' interface for incoming network
 * connections and, if one present, initialize 'conn' connection structure
 * The routine blocks the thread's execution no more then 'timeout' seconds.
 *
 * Input:
 *  mcohs_interface_h intf   - a pointer to an initialize interface
 *  mcohs_connection_h conn  - a pointer to clean connection structure
 *  long timeout             - timeout value
 * Output:
 *  mcohs_connection_h conn  - initialized connection structure
 *
 * Results:
 *  HS_EXECUTE   - incoming connection was detected
 *  HS_TIMEOUT   - no incoming connections was detected
 *  HS_CANT_INIT - fail
 *
 */
MCOEWS_API int mcohs_connection_check(mcohs_interface_h intf, mcohs_connection_h conn, long timeout);

/*
 * This routine is 'the heart' of connection processing.
 * This routine should be called in cycle during connection processing thread.
 *
 * Input:
 *  mcohs_connection_h conn  - a pointer to a initialized connection structure
 *  long timeout             - timeout value
 * Output:
 *  none
 *
 * Results:
 *  HS_OK                     - an acton was processed
 *  HS_TIMEOUT                - no action was processed
 *  HS_FINISHED               - the connection was finished completely
 *  HS_HTTP_HEADER_TOO_BIG    - An HTTP header longer then MCO_HTTP_HEADER_SIZE_LIMIT.
 *  HS_INVALID_REQUEST_LINE   - An invalid HTTP request line was detected
 *  HS_INVALID_REQUEST_HEADER - An invalid HTTP request header was detected
 *  HS_CANT_INIT - fail
 */
MCOEWS_API int mcohs_connection_execute(mcohs_connection_h conn, long timeout);


/*
 * This routine puts HTTP response with given code and phrase and cleans the given connection.
 * This routine should be called in case of insufficient server's resources.
 * Note : the connection becomes invalid and clean
 *
 * Input:
 *  mcohs_connection_h conn    - a pointer to a initialized connection structure
 *  unsigned short status_code - an HTTP status code
 *  char *reason_phrase        - respectve status phrase
 * Output:
 *  none
 *
 * Results:
 *  HS_OK        - success
 *  HS_CANT_INIT - fail
 */
MCOEWS_API int mcohs_connection_decline(mcohs_connection_h conn, unsigned short status_code, const char* reason_phrase);

/*
 * The mcohs_connection_clear() routine closes the network connecton and cleans all associated 
 * resources
 *
 * Input:
 *  mcohs_connection_h conn    - a pointer to a initialized connection structure
 * Output:
 *  none
 *
 * Results:
 *  HS_OK        - success
 *  HS_CANT_INIT - fail
 */
MCOEWS_API int mcohs_connection_clear(mcohs_connection_h conn);

/*
 * The mcohs_connection_detach() routine closes the network connecton on parents side of forked program
 * resources
 *
 * Input:
 *  mcohs_connection_h conn    - a pointer to a initialized connection structure
 * Output:
 *  none
 *
 * Results:
 *  HS_OK        - success
 *  HS_CANT_INIT - fail
 */
MCOEWS_API int mcohs_connection_detach(mcohs_connection_h conn);

/*
 * The routine gets specified number of bytes from the request.
 *
 * Input:
 *  mcohs_request_h request - the request context
 *  char **ptr              - pointer to the variable that will point to the requested content.
 *  unsigned int required_sz      - requested number of bytes
 *  unsigned int *got_sz          - recieved number of byte
 *  long timeout                  - timeout value
 *
 * Output: 
 *  char **ptr                    - pointer to the variable that will point to the requested content.
 *  unsigned int *got_sz          - recieved number of byte
 *
 * Results:
 *  HS_OK        - requested number of bytes was recieved completely
 *  HS_TIMEOUT   - The operation was timed out. Partial content was recieved passibly.
 *  HS_CANT_INIT - fail
 */
MCOEWS_API int mcohs_get(mcohs_request_h request, char **ptr, unsigned int required_sz, unsigned int *got_sz, long timeout);

/*
 * This routine drops specified amount of the request's content.
 *
 * Input:
 *  mcohs_request_h request     - the request context
 *  unsigned int required_sz    - number of bytes
 *
 * Output:
 *  none
 *
 * Results:
 *  HS_OK        - requested number of bytes was dropped
 *  HS_CANT_INIT - fail
 */
MCOEWS_API int mcohs_drop(mcohs_request_h request, unsigned int required_sz);

/*
 *
 * Error diagnostic and processing
 *
 */

/*
 * The error diagnostic routine
 *
 * Input:
 *  none
 *
 * Output:
 *  none
 *
 * Results:
 *  OS-depended error code
 */
#ifndef WIN32
#include <errno.h>
#define mcohs_errno() (errno)
#else  /* WIN32 */
#define mcohs_errno() (WSAGetLastError())
#endif /* WIN32 */

#ifdef MCO_HTTP_SESSION_SUPPORT
/*****************************************************************************************
 *
 * Automatic session support
 *
 *****************************************************************************************/
/*
 * The hs_app_make_new_session() routine should allocate memory for a session
 * structure and prepare the structure for a new session.
 *
 * NOTE: the routine should be thread-safe
 */
mcohs_session_h hs_app_get_new_session();

/*
 * The hs_app_find_session() routine should find and return pointer to session
 * structure by specified session identifier.
 */
mcohs_session_h hs_app_find_session(const char *id, unsigned int id_sz);

/*
 * The hs_app_check_dead_sessions() should scan and release timed out session structures
 *
 * NOTE: the routine should be thread-safe
 */
void hs_app_check_dead_sessions();
#endif /* MCO_HTTP_SESSION_SUPPORT */

/*****************************************************************************************
 *
 * Inprocess CGI API
 *
 *****************************************************************************************/

/*
 *
 * The routine sets response status in CGI mode.
 *
 * Input:
 *  mcohs_request_h request    - request context
 *  unsigned short status_code - an HTTP response status code (200, 300, 400, 500, ...)
 *  char *reason_phrase        - the response reason phrase
 *  mcohs_datetime_h mod_date  - time stamp of last modification of a response content
 *  char *charset              - charset of the content. Zero for default.
 *  char *content_type         - type of the content ('text/html', 'image/gif', ...)
 *  unsigned int content_length      - size of the content if present
 *  void *content              - body of the content if present
 *  int cache_control          - cache control flags. zero or OR'ed constants
 *      MCO_HTTP_CACHABLE_CONTENT - a client's browser may cache the response
 *      MCO_HTTP_PRIVATE_CONTENT  - a client's browser shouldn'n place the response
 *                                  to a public cache
 *  char *auth_realm           - request for authorization
 *  char *extra_headers        - addition header fileds ended properly (CRLF)
 *
 *  Output:
 *    none
 *
 *  Results:
 *    none
 */
MCOEWS_API void mcohs_set_status(
    mcohs_request_h request,
    unsigned short status_code, const char *reason_phrase,
    mcohs_datetime_h mod_date,
    const char *charset,
    const char *content_type, unsigned int content_length, const void *content,
    int cache_control,
    const char *auth_realm,
    const char *extra_headers
    );

/*
 * The routine makes a response content in printf manner.
 *
 * Input:
 *  mcohs_request_h request - the request context
 *  char *fmt               - format string
 *  ...                     - addition arguments
 *
 * Output:
 *  none
 *
 * Results:
 *  number of processed bytes
 */
MCOEWS_API unsigned int mcohs_printf(mcohs_request_h request, const char* fmt, ...);

/*
 * The routine puts the given buffer into response content
 *
 * Input:
 *  mcohs_request_h request - the request context
 *  char *buf               - pointer to
 *  unsigned int sz         - and size of the buffer
 *
 * Output:
 *  none
 *
 * Results:
 *  number of processed bytes
 */
MCOEWS_API int mcohs_put(mcohs_request_h request, const char* buf, unsigned int sz);

/*
 * The routine checks if the request's connection is clean (was not written to).
 *
 * Input:
 *  mcohs_request_h request - the request context
 *
 * Output:
 *  none
 *
 * Results:
 *  1 if the request is clean, 0 otherwise
 */
MCOEWS_API int mcohs_is_clean(mcohs_request_h request);

/*****************************************************************************************
 *
 * eXtremeWS's HTTP tools
 *
 *****************************************************************************************/

/*
 * The routine returns a pointer to a value of an requested attribute
 *
 * Input:
 *  mcohs_request_h request - request context
 *  char *name              - name of requested field
 *
 * Output:
 *  none
 *
 * Result:
 *  pointer to the fileds's value or zero if not found
 */
MCOEWS_API const char *mcohs_get_attr(mcohs_request_h request, const char *name);

#ifdef MCO_HTTP_TOOLS
/*
 * This routine enums the request's fields in form 'name: value' one by one.
 *
 * Input:
 *  mcohs_request_h request - request context
 *  char* attr              - the first attr. name
 *
 * Output:
 *  none
 *
 * Result:
 *  pointer to the next field pair
 
 * Example:
 *  const char *p = 0;
 *  while ((p=mcohs_get_next_attr(request, p)) != 0) {
 *      printf("%s\n", p);
 *  }
 *
 */
MCOEWS_API const char *mcohs_get_next_attr(mcohs_request_h request, const char *attr);

/*
 * The routine converts HEX representation of a char to it's value.
 *
 * Input:
 *  unsigned char c - hex ['0','F']
 *
 * Output:
 *  none
 *
 * Result:
 *  unsigned char [0x0,0xF]
 */
MCOEWS_API unsigned char mcohs_hexC2V(unsigned char c);

#endif /*MCO_HTTP_TOOLS*/

#if defined(MCO_HTTP_TOOLS) || defined(MCO_HTTP_SESSION_SUPPORT)
/*
 * The routine converts an unsigned char value to it's HEX representation.
 *
 * Input:
 *  unsigned char c - unsigned char [0x0,0xF]
 *
 * Output:
 *  none
 *
 * Result:
 *  hex ['0','F']
 */
MCOEWS_API unsigned char mcohs_V2hexC(unsigned char c);
#endif /* MCO_HTTP_TOOLS || MCO_HTTP_SESSION_SUPPORT */

/*
 * The routine inplace converts a string from 'HTTP-escaped' representation (%20%FA...)
 * to a plain string
 *
 * Input:
 *  char *str - a escaped string
 *  unsigned int len - length of the string
 *
 * Output:
 *  the unescaped string
 *
 * Result:
 *  number of characters in the unescaped string
 *
 * Note: the routine DOSN'T apply zero-byte to the end of the string
 */
MCOEWS_API unsigned int mcohs_unescape(char *str, unsigned int len);

/*
 * The routine checks equivalence of a string pointed by p and a string pointed by wrd
 * in case-insensitive manner.
 *
 * Input:
 *  char *p   - a zero terminated string
 *  char *wrd - a zero terminated string
 *
 * Output:
 *  none
 *
 * Result:
 *  pointer to first not matching symbol of string p
 */
MCOEWS_API const char *mcohs_strlike(const char *p, const char *wrd);

/*
 * The routine finds a word wrd in a zero-terminated string p in case-insensetive manner.
 *
 * Input:
 *  char *p   - a string
 *  char *wrd - a word
 *
 * Output:
 *  none
 *
 * Result:
 *  pointer to the symbol behind the first occurence of the word or null if the
 *  word was not found.
 */
MCOEWS_API const char *mcohs_findstrlike(const char *p, const char *wrd);

/*
 * This routine converts from mcohs_datetime_t structure to RFC1123 datetime string.
 *
 * Input:
 *  mcohs_datetime_h ptm - pointer to timestamp struct
 *
 * Output:
 *  char *buf           - pointer to buffer at least 40 bytes long
 *
 * Result:
 *  number of characters in the output string
 */
MCOEWS_API unsigned int mcohs_datetime2rfc1123(const mcohs_datetime_h ptm, char *buf);

/*
 * The routine returns difference (in seconds) between two mcohs_datetime_t structures
 *
 * Input:
 *  mcohs_datetime_h dt1 - pointer to the first timestamp structure
 *  mcohs_datetime_h dt2 - pointer to the second timestamp structure
 *
 * Output:
 *  none
 *
 * Result:
 *  difference in seconds.
 *
 * Note: dt2 should be early than dt1
 */
MCOEWS_API unsigned int mcohs_datetime_diff(const mcohs_datetime_h dt1, const mcohs_datetime_h dt2);

/*
 * The routine converts from mcohs_datetime_t to number of seconds since 1 Jan 1900
 *
 * Input:
 *  mcohs_datetime_h dt - pointer to the timestamp structure
 *
 * Output:
 *  none
 *
 * Result:
 *  number of seconds
 *
 */
MCOEWS_API unsigned int mcohs_datetime2uint(const mcohs_datetime_h dt);

/*
 * The routine converts number of seconds to mcohs_datetime_t struct.
 *
 * Input:
 *  unsigned int ts - number of seconde since 1 Jan 1970
 *
 * Output:
 *  mcohs_datetime_h dt - pointer to variable that recives splitted value
 *
 * Result:
 *  none
 *
 */
MCOEWS_API int mcohs_uint2datetime(unsigned int ts, mcohs_datetime_h dt);

/*
 * Converts RFC1123 datetime string to mcohs_datetime_t variable
 *
 * Input:
 *  char *buf - a RFC1123 datetime string
 *
 * Output:
 *  mcohs_datetime_h ptm - pointer to mcohs_datetime_t variable
 *
 * Result:
 *   0  - success
 *  -1  - invalid RFC1123 datetime string
 */
MCOEWS_API int mcohs_rfc11232datetime(const char *buf, mcohs_datetime_h ptm);

#if !defined(MCO_HTTP_NO_BASIC_AUTH) || defined(MCO_HTTP_TOOLS)
/*
 * This routine decodes a string from Base64 encoding to plain zero terminated string.
 *
 * Input:
 *  char *data - a string in Base64 encoding
 *
 * Output:
 *  char *data - a plain zero terminated string
 *
 * Result:
 *  none
 */
MCOEWS_API void mcohs_decode_base64(char *data);
#endif /* !MCO_HTTP_NO_BASIC_AUTH || MCO_HTTP_TOOLS */

/*****************************************************************************************
 *
 * Wrappers
 *
 *****************************************************************************************/

/*
 * The routine returns length of the zero terminated string
 *
 * Input:
 *  char *p - a zero terminated string
 *
 * Output:
 *  none
 *
 * Result:
 *  length of the string in bytes
 */
MCOEWS_API unsigned int mcohs_int_strlen(const char *s);

MCOEWS_API unsigned int mcohs_int_vsprintf_sz(const char* fmt, va_list ap);

/*
 * The routine converts a string to it's numeric representation
 *
 * Input:
 *  char *p - a zero terminated string
 *
 * Output:
 *  none
 *
 * Result:
 *  numeric value of the given string
 *
 */
MCOEWS_API unsigned int mcohs_int_str2szt(const char *p);

/*
 * The routine converts a numeric value to a string.
 * Is assumes the buf is long enought to contain a string representation of the number.
 *
 * Input:
 *  unsigned int num - a number
 *
 * Output:
 *  char *buf - a zero terminated string representation of the given number
 *
 * Result:
 *  number of bytes in resulting string
 *
 * Note: the routine DOESN'T apply zero-byte to the end of the string
 */
MCOEWS_API unsigned int mcohs_int_ui2str(unsigned int num, char* buf);

/*
 * This routine calculates number of digits in the given string.
 *
 * Input:
 *  none
 *
 * Output:
 *  none
 *
 * Result:
 *  number of digist in the string from the begin up to first non-digit symbol
 */
MCOEWS_API int mcohs_check_digits(const char *p);

#ifdef __cplusplus
}
#endif

#endif /* MCO_EWS_HTTP_SUPPORT_H */

#ifndef MCOREST_H_
#define MCOREST_H_

#include "mco.h"


#ifdef __cplusplus
extern "C" {
#endif

#ifndef MCOREST_API
# ifdef _WIN32
#  ifdef MCOREST_DLL
#    define MCOREST_API __declspec(dllexport)
#  else
#    ifdef MCOREST_USE_DLL
#       define MCOREST_API __declspec(dllimport)
#    else
#       define MCOREST_API 
#    endif
#  endif
# else
#  define MCOREST_API
# endif
#endif

#ifndef MCORESTDB_API
# ifdef _WIN32
#  ifdef MCORESTDB_DLL
#    define MCORESTDB_API __declspec(dllexport)
#  else
#    ifdef MCORESTDB_USE_DLL
#       define MCORESTDB_API __declspec(dllimport)
#    else
#       define MCORESTDB_API 
#    endif
#  endif
# else
#  define MCORESTDB_API
# endif
#endif

#ifndef MCORESTPERF_API
# ifdef _WIN32
#  ifdef MCORESTPERF_DLL
#    define MCORESTPERF_API __declspec(dllexport)
#  else
#    ifdef MCORESTPERF_USE_DLL
#       define MCORESTPERF_API __declspec(dllimport)
#    else
#       define MCORESTPERF_API 
#    endif
#  endif
# else
#  define MCORESTPERF_API
# endif
#endif

#ifndef MCORESTSQL_API
# ifdef _WIN32
#  ifdef MCORESTSQL_DLL
#    define MCORESTSQL_API __declspec(dllexport)
#  else
#    ifdef MCORESTSQL_USE_DLL
#       define MCORESTSQL_API __declspec(dllimport)
#    else
#       define MCORESTSQL_API 
#    endif
#  endif
# else
#  define MCORESTSQL_API
# endif
#endif

#ifndef MCORESTTRACE_API
# ifdef _WIN32
#  ifdef MCORESTTRACE_DLL
#    define MCORESTTRACE_API __declspec(dllexport)
#  else
#    ifdef MCORESTTRACE_USE_DLL
#       define MCORESTTRACE_API __declspec(dllimport)
#    else
#       define MCORESTTRACE_API 
#    endif
#  endif
# else
#  define MCORESTTRACE_API
# endif
#endif



typedef void *mcorest_h;  /** MCO REST server handle. */
typedef void *mcorest_interface_h;  /** Network interface handle. */
typedef void *mcorest_conn_h;  /** Network connection handle. */


/**
 * Initializes the MCO REST server runtime.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_initialize(void);

/**
 * Shuts down the MCO REST server runtime.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_shutdown(void);

/**
 * Creates the MCO REST server instance and stores its handle in the rest parameter.
 * @param [in] db_name Name of the database to bind the server to.
 * @param [out] rest Pointer to the server instance handle.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_create(const char *db_name, mcorest_h *rest);

/**
 * Destroys the MCO REST server instance. The server may only be destroyed
 * by the same thread that created it.
 * @param [in] rest Server instance handle.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_destroy(mcorest_h rest);

/**
 * Sets Basic HTTP authentication parameters for the server instance.
 * This method requires the HTTP authentication to be enabled in mcoews as well.
 * All arguments must be non-NULL. Furthermore, the username and the password
 * cannot be empty.
 * Repeated calls to this function will overwrite previously set parameters.
 * @param [in] rest Server instance handle.
 * @param [in] realm Authentication realm, up to 15 characters.
 * @param [in] username Username, up to 15 characters.
 * @param [in] password Password, up to 31 characters.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_set_basic_auth(mcorest_h rest, const char *realm,
    const char *username, const char *password);

/**
 * Resets Basic HTTP authentication parameters and disables the authentication.
 * @param [in] rest Server instance handle.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_reset_basic_auth(mcorest_h rest);

/**
 * Adds a network interface to the MCO REST server instance. This function
 * should only be called from the same thread that created the server.
 * The ssl_params structure is not copied by the server and must be kept valid
 * until the interface is closed.
 * @param [in] rest Server instance handle.
 * @param [in] addr IP address of the interface, as a string.
 * @param [in] port IP port to listen on.
 * @param [in] ssl_params SSL parameters structure (pass NULL to disable SSL).
 * @param [out] intf Pointer to the interface handle.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_add_interface(mcorest_h rest, const char *addr, unsigned short port,
    void *ssl_params, mcorest_interface_h *intf);

/**
 * Initializes the database REST service.
 * @param [in] rest Server instance handle.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCORESTDB_API MCO_RET mcorest_svc_db_init(mcorest_h rest);

/**
 * Initializes the perfmon REST service.
 * @param [in] rest Server instance handle.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCORESTPERF_API MCO_RET mcorest_svc_perf_init(mcorest_h rest);

/**
 * Initializes the SQL REST service.
 * @param [in] rest Server instance handle.
 * @param [in] engine Pointer to an instance of McoMultithreadedSqlEngine.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCORESTSQL_API MCO_RET mcorest_svc_sql_init(mcorest_h rest, void *engine);

/**
* Initializes the trace REST service.
* @param [in] rest Server instance handle.
* @retval MCO_S_OK on success, error code otherwise.
*/
MCORESTTRACE_API MCO_RET mcorest_svc_trace_init(mcorest_h rest);

/**
 * Starts the MCO REST server in threaded mode. No further operations on the
 * server handle and interface handles are allowed until mcorest_stop()
 * is called. This function should only be called from the same thread that
 * created the server.
 * @param [in] rest Server instance handle.
 * @retval MCO_S_OK Success.
 * @retval MCO_E_UNSUPPORTED Threading is disabled.
 * @retval MCO_E_REST_* error code on failure.
 */
MCOREST_API MCO_RET mcorest_start(mcorest_h rest);

/**
 * Stops the MCO REST server in threaded mode after it was started
 * with mcorest_start(). This function should only be called from the same
 * thread that created the server.
 * @param [in] rest Server instance handle.
 * @retval MCO_S_OK Success.
 * @retval MCO_E_UNSUPPORTED Threading is disabled.
 * @retval MCO_E_REST_* error code on failure.
 */
MCOREST_API MCO_RET mcorest_stop(mcorest_h rest);

/**
 * Checks the interface for incoming network connections. Accepts one incoming
 * connection and returns pointer to its handle through the conn parameter.
 * This function may be called by a thread other than the one that created
 * the interface. However, it should not be called with the same interface
 * from different threads.
 * @param [in] intf Interface handle.
 * @param [in] timeout Incoming connection wait timeout.
 * @param [out] conn Pointer to the accepted connection's handle.
 * @retval MCO_S_REST_CONN_ACCEPTED A connection was accepted and its handle
 * is stored in the conn argument.
 * @retval MCO_S_REST_TIMEOUT No connection was accepted.
 * @retval MCO_E_REST_* error code on failure.
 */
MCOREST_API MCO_RET mcorest_interface_check(mcorest_interface_h intf, long timeout, mcorest_conn_h *conn);

/**
 * Executes one or more HTTP requests. This function may be called by a thread
 * other than the one that accepted the connection. However, it should not
 * be called with the same connection from different threads.
 * @param [in] conn Connection handle.
 * @param [in] timeout Incoming data wait timeout.
 * @retval MCO_S_OK A request was executed, the connection is still alive
 * and further requests may be handled.
 * @retval MCO_S_REST_TIMEOUT No request was received, the connection is
 * still alive and requests may be handled.
 * @retval MCO_S_REST_CONN_FINISHED A request was executed and the connection
 * was closed.
 * @retval MCO_E_REST_* error code on failure. The connection was closed.
 */
MCOREST_API MCO_RET mcorest_conn_execute(mcorest_conn_h conn, long timeout);

/**
 * Cancels an active connection. The connection must not be cancelled while
 * it is being used by a different thread.
 * @param [in] conn Connection handle.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_conn_cancel(mcorest_conn_h conn);

/**
 * Shuts down the interface. This function must not be called while the server
 * is running in threaded mode (i.e. after the mcorest_start() call and before
 * the mcorest_stop() call). This function should only be called from the same
 * thread that created the interface. The interface must not be closed while
 * it is being used by a different thread.
 * @param [in] intf Interface handle.
 * @retval MCO_S_OK on success, error code otherwise.
 */
MCOREST_API MCO_RET mcorest_interface_close(mcorest_interface_h intf);


#ifdef __cplusplus
}
#endif


#endif /* MCOREST_H_ */

#ifndef __SQLSRV_C_H__
    #define __SQLSRV_C_H__

    #include "mcoapic.h"

#ifdef __cplusplus
extern "C" { 
#endif

    typedef struct sqlsrv* sqlsrv_t;
    typedef void(*sqlsrv_error_handler_t)(char const* msg);

    typedef enum { s_wait, s_active, s_done, s_canceled } sqlsrv_session_state_t;
    typedef struct {
        sqlsrv_session_state_t state;
        unsigned int peer_addr;
        int peer_port;
    } sqlsrv_session_info_t;
    typedef void(*sqlsrv_session_info_handler_t)(const sqlsrv_session_info_t* info, void* context);

    /**
     * Starts server. This method cause server to open socket at specified port and 
     * accept client connection requests
     * @param server SQL server created by sqlsrv_create
     * @return status of operation as described in error_code enum
     */
    status_t sqlsrv_start(sqlsrv_t server);

    /**
     * Stops server. This method stop accept thread and close server socket.
     * @param server SQL server created by sqlsrv_create
     * @return status of operation as described in error_code enum
     */
    status_t sqlsrv_stop(sqlsrv_t server);

    /**
     * Constructor of SQL server
     * @param server output parameter to get handle of the created SQL server
     * @param engine locql SQL engine
     * @param port server port
     * @param bufferSize size of transfer buffer. Result of client query execution will be placed 
     * in this buffer. If result data source can not fit in buffer then query result will be 
     * delivered to client by parts. In this case transaction lock is kept until all
     * results are sent to the client.
     * @param nThreads optimal number of thread spawned by server. Server will spawn as many threads
     * as client requests arrives. Each thread process requests from the client until 
     * client closes connection. After it thread is return to pool of idle threads or
     * is terminated if number of threads exceeds optimal number of threads.
     * @param listenQueueSize paremeter of socket listen function, meaning maximal number of 
     * connections which can be concurrently accepted by server
     * @param handler procedure for reporting client session errors
     * @return status of operation as described in error_code enum
     */
    status_t sqlsrv_create(sqlsrv_t* server, storage_t engine, int port, size_t bufferSize, size_t nThreads, int
                           listenQueueSize, sqlsrv_error_handler_t handler);

    /**
     * Server destructor
     * @param server SQL server created by sqlsrv_create
     * @return status of operation as described in error_code enum
     */
    status_t sqlsrv_destroy(sqlsrv_t server);

    /**
     * Enumerates sessions in wait state then active sessions.
     * @param server SQL server created by sqlsrv_create
     * @param callback pointer to the function
     * @return status of operation as described in error_code enum
     */
    status_t sqlsrv_sessions_info(sqlsrv_t server, sqlsrv_session_info_handler_t handler, void* context);

    /**
     * Registers (unregister) callback to receive new session start\stop events.
     * @param server SQL server created by sqlsrv_create
     * @param callback pointer to the function
     * @return status of operation as described in error_code enum
     */
    status_t sqlsrv_reg_session_event(sqlsrv_t server, sqlsrv_session_info_handler_t handler, void* context);

#ifdef __cplusplus
}
#endif

#endif

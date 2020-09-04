#ifndef __SQLCLN_C_H__
    #define __SQLCLN_C_H__

    #include "sqlc.h"

#ifdef __cplusplus
extern "C" { 
#endif
    /**
     * Open connection with server
     * @param database handle to the database created by sqlcln_open function
     * @param hostname name of the where server is located
     * @param port server port
     * @param maxConnectAttempts maximal attempts of connection to server,  timeout for
     * each connection attempt is two seconds, so totally connection will be performed
     * during 2*maxConnectionAttempts seconds
     * @return status of operation as described in error_code enum
     */
    status_t sqlcln_open(database_t database, char const* hostname, int port, int maxConnectAttempts);

    /**
     * Close connection with server
     * @param database handle to the database created by sqlcln_open function
     * @return status of operation as described in error_code enum
     */
    status_t sqlcln_close(database_t database);

    /** Constructor of SQL engine remote interface
     * @param database pointer to the location to receive handle of the opened database
     * @param txBufSize size of transmit buffer (used to serialize requests to be sent to * @return status of operation as described in error_code enum
    the server)
     */
    status_t sqlcln_create(database_t* database, size_t txBufSize);

    /**
     * Destructor of SQL engine remote interface
     * @param database handle to the database created by sqlcln_open function
     * @return status of operation as described in error_code enum
     */
    status_t sqlcln_destroy(database_t database);

#ifdef __cplusplus
}
#endif

#endif

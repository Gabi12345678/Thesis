#ifndef __SQLSRV_H__
#define __SQLSRV_H__

#include "mcosql.h"
#include "mconet.h"

namespace McoSql
{
    class Server;

    /**
     * Class performing execution of client SQL requests
     */
	class MCORSQL_API SqlServer
    {
    public:
        typedef void(*error_handler_t)(char const* msg);

        typedef enum { s_wait, s_active, s_done, s_canceled } session_state_t;
        typedef struct {
            session_state_t state;
            mco_inetaddr_t peer_addr;
            int peer_port;
        } session_info_t;
        typedef void(*session_info_handler_t)(const session_info_t* info, void* context);


        typedef enum { q_started, q_active, q_done, q_failed, q_canceled } query_state_t;
        typedef struct {
            McoSqlEngine  *session;
            void          *session_context; //  to store per-session information between callbacks. NULL for new sessions
            uint8          client_id; // ID of the client - the same for all shards of distributed database
            uint4          statementId;
            mco_inetaddr_t peer_addr;
            int            peer_port;
            query_state_t  state;
            const char    *sql;
            Vector<Value> *sql_params;
            const char    *fail_reason;
            QueryStat      stat;
        } query_info_t;
        typedef void(*query_info_handler_t)(query_info_t* qstat, void *context);

        /**
         * Starts server. This method cause server to open socket at specified port and
         * accept client connection requests
         */
        void start();

        /**
         * Stops server. This method stop accept thread and close server socket.
         */
        void stop();

        /**
         * Enumerates sessions in wait state then active sessions.
         */
        void getSessionsInfo(session_info_handler_t handler, void* context = NULL);

        /**
         * Registers (unregister) callback to receive new session start\stop events.
         */
        void regSessionEvent(session_info_handler_t handler, void* context = NULL);

        /**
         * Registers (unregister) callback to receive query statistic events.
         */
        void regQueryInfoEvent(query_info_handler_t handler, void* context = NULL);

        /**
         * Constructor of SQL server
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
         * @param handler error handler
         * @param localDomain server and clients are located at the same host
         * @param handler procedure for reporting client session errors
         * @param sslParameters pointer to a mco_ssl_params_t structure containing SSL connection settings
         * @param authenticationRequired authentication is required for clients
         * @param interruptTimeout period in milliseconds to poll the interrupts from the client(s). If 0, disable interruption
         *
         */
        SqlServer(McoSqlEngine* engine, int port, size_t bufferSize = 64 * 1024,
                  size_t nThreads = 8, int listenQueueSize = 5, error_handler_t handler = NULL, bool localDomain = false,
                  void* sslParameters = NULL, bool authenticationRequired = false, int interruptTimeout = 100, int compressionLevel = 0);


        struct OpenParameters {
            McoSqlEngine   *engine;
            int             port;
            size_t          bufferSize;
            size_t          nThreads;
            int             listenQueueSize;
            error_handler_t handler;
            bool            localDomain;
            void          * sslParameters;
            bool            authenticationRequired;
            int             interruptTimeout;
            const char     *netInterface;
			int             nGcThreads;
			int             gcPeriod;
            int             compressionLevel;

            OpenParameters(McoSqlEngine *eng, int p) : engine(eng), port(p), bufferSize(64 * 1024), nThreads(8), listenQueueSize(5), handler(0), localDomain(false),
                                                       sslParameters(0), authenticationRequired(false), interruptTimeout(100), netInterface(0), nGcThreads(0), gcPeriod(100), compressionLevel(0) {}
        };
        SqlServer(const OpenParameters &params);


        /**
         * Server destructor
         */
        ~SqlServer();

    private:
        Server* server;
    };
}
#endif

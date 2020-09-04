#ifndef __SQLCLN_H__
#define __SQLCLN_H__

#include "sqlcpp.h"
#include "mcotime.h"

#define COLLECT_DSQL_STATS 1

namespace McoSql
{

    class TcpIpSocket;
    class DataSourceStub;

    /**
     * Client interface for sending SQL requests to the server
     */
	class MCORSQL_API RemoteSqlEngine: public SqlEngine
    {
        TcpIpSocket* socket;
        size_t txBufSize;
        char* txBuf;
        DataSourceStub* activeDataSource;
        bool localDomain;
        timer_unit readTimeout;
        QueryStat qstat;
        bool stat_collected;
        uint8 clientId;
        uint4 statementId;

        static void responseReader(DataSourceStub* ds, void* arg);
        static void responseDestructor(DataSourceStub* ds, void* arg);
        String* readResponse(Allocator* allocator);
        void completePreviousQuery();

    public:
		Transaction* beginTransaction(Transaction::Mode mode, int priority, Transaction::IsolationLevel level);
		
		/**
		 * Perform authentication at server
		 */
		bool authenticate(char const* login, char const* password);

        /**
         * Output query statistics if tracing is enabled
         */
        void printQueryStat(FILE *out);
    
        /**
         * Prepared queries are not supported
         */
        void vprepare(PreparedStatement &stmt, char const* sql, va_list* list);

        /**
         * Prepared queries are not supported
         */
        void vprepare(PreparedStatement &stmt, char const* sql, ParamDesc* params);

        /**
         * Remotely execute prepared query at server
         */
        ResultSet* executePrepared(PreparedStatement &stmt, Transaction* trans, int64_t& nRecords);

        /**
         * Remotely execute query at server
         */
        ResultSet* vexecute(Transaction* trans, char const* sql, va_list* list, Value** array, size_t arraySize, int64_t& nRecords);

        /**
         * Open connection with server
         * @param hostname name of the where server is located
         * @param port server port
         * @param maxConnectAttempts maximal attempts of connection to server
         * @param localDomain server and clients are located at the same host
         * @param sslParameters pointer to a mco_ssl_params_t structure
         * containing SSL connection settings
         * @param connectTimeout timeout for each connect attempt in milliseconds, so totally connection can take up to connectTimeout*maxConnectionAttempts milliseconds
         * @param readTimeout timeout for read operations in milliseconds
         * @return <code>true</code> if connection successfully established, <code>false</code> otherwise
         */
        bool open(char const* hostname, int port, int maxConnectAttempts = 10, bool localDomain = false, void* sslParameters = NULL, 
                            timer_unit connectTimeout = 2*1000, timer_unit readTimeout = 1200*1000, int compressionLevel = 0);

        /**
         * Close connection with server
         */
        void close();

        /**
         * Check if engine is local or remote
         */
        virtual bool isRemote();

        /**
          *  Get last query statistics
          *    return:
          *          true in case of stats were collected, in this case ret_stat recieved stats
          *          false if stats were not collected
          */
        bool getServerQueryStats(QueryStat &ret_stat);

        /** Constructor of SQL engine remote interface
         * @param txBufSize size of transmit buffer (used to serialize requests to be sent to the server)
         */
        RemoteSqlEngine(size_t txBufSize = 64 * 1024);

        /**
         * Destructor of SQL engine remote interface
         */
        ~RemoteSqlEngine();
    };


#if COLLECT_DSQL_STATS
    class MCORSQL_API StatsCollector
    {
        public:
            struct OpStats {
                uint64_t beg;
                uint64_t end;
                uint32_t count;
            };

            StatsCollector() : allocator(NULL), shard_stats(NULL), nHosts(0), shards_size(0) { }

            ~StatsCollector() {
                if (allocator != NULL) {
                    allocator->free(shard_stats, shards_size);
                }
            }

            enum Operation { SockWrite = 0, SockRead, ReplicaReconnect, GetResults, MergeResults } ;

            inline void open(Allocator *allocator, int nHosts);

            void reset(void);

            inline void startQuery();

            OpStats  *op_start(Operation op, int node)
            {
                if (op >= GetResults) {
                    return & op_stats[(int)op - 3];
                } else {
                    return shard_stats + node*3 + (int)op;
                }
            }

            void report(FILE *fp);

            Allocator *allocator;
            uint64_t started;
            OpStats op_stats[2];
            OpStats *shard_stats; // Operation * nodes
            int nHosts;
            size_t shards_size;
    };

    class StatsScope
    {
    public:
        StatsScope(StatsCollector &collector, StatsCollector::Operation op, int node = -1);
        ~StatsScope();
    protected:
        StatsCollector::OpStats *op_stats;
    };

    class ReportScope
    {
    public:
        ReportScope(StatsCollector &collector, bool traceEnabled) : _collector(collector), _traceEnabled(traceEnabled)
        {
            _collector.reset();
        }

        ~ReportScope() {
            if (_traceEnabled) {
                _collector.report(stdout);
            }
        }

    private:
        StatsCollector &_collector;
        bool _traceEnabled;
    };

#endif

    /**
     * Client interface for dsitrbuted execution of SQL queries
     */
	class MCORSQL_API DistributedSqlEngine: public SqlEngine
    {
        struct Node {
            TcpIpSocket* socket;
            char* address;
            int port;
            int reconnectCount;
            int reconnectInterval;
            int messageLength;
            QueryStat qstat;
            bool stat_collected;
            String* login;
            String* password;
            void* sslParams;
            int compressionLevel;

            Node();
            ~Node();
        };

        struct Shard {
            bool eof;
            int master;
            int executor;
            int nLiveReplicas;
            DataSourceStub* result;

            Shard() : eof(true), master(0), nLiveReplicas(0), result(NULL) {}
        };

        size_t txBufSize;
        char* txBuf;
        int currDataSource;
        int lastDataSource;
        int nShards;
        int nReplicas;
        bool replication;
        bool localDomain;
		bool initializationCompleted;
        timer_unit connectTimeout;
        timer_unit readTimeout;
        unsigned currShard;
        Node* nodes;
        Shard* shards;
        uint8 clientId;
        uint4 statementId;

        static void responseReader(DataSourceStub* ds, void* arg);
        static void responseDestructor(DataSourceStub* ds, void* arg);
        String* readResponse(Allocator* allocator, size_t& offs, bool throwException = true);
        bool socketRead(int i, void* buf, size_t *bufSize, timer_unit timeout, bool interruptable, int& errorCode);
        void disconnectReplica(int node);
        void reconnectReplica(int node);
        void completePreviousQuery();
		bool authenticateOnNode(int node);
		void broadcastShuffleInfo();

        ResultSet* mergeResults(Runtime* runtime, char const* sql, DataSourceStub* ds);
        size_t copy(ResultSet* ds, char const* sql, int dst);

    public:
		/**
		 * Perform authentication at server
		 */
		bool authenticate(char const* login, char const* password);

        /**
         * Perform authentication at specified node
         */
        bool authenticate(char const* login, char const* password, int node);

        /**
         * Prepared queries are not supported
         */
        void vprepare(PreparedStatement &stmt, char const* sql, va_list* list);

        /**
         * Prepared queries are not supported
         */
        void vprepare(PreparedStatement &stmt, char const* sql, ParamDesc* params);

        /**
         * Remotely execute prepared query at server
         */
        ResultSet* executePrepared(PreparedStatement &stmt, Transaction* trans, int64_t& nRecords);

        /**
         * Remotely execute query at server
         */
        ResultSet* vexecute(Transaction* trans, char const* sql, va_list* list, Value** array, size_t arraySize, int64_t& nRecords);


        typedef enum { SQL_REPLICATION = 0, HA_REPLICATION } ReplicationType;
        /**
         * Open connection with servers
         * @param nodes array with node names and ports ("NAME:PORT")
         * @param nNodes number of nodes
         * @param nReplicas number of replicas, should be diveder of nNodes
         * @param replType replication method
         * @param maxConnectAttempts maximal attempts of connection to server
         * @param bad_node if this parameter is not null and open is failed, then index of not unavailable node is stored at this location 
         * @param localDomain server and clients are located at the same host
         * @param sslParameters pointer to a mco_ssl_params_t structure
         * containing SSL connection settings
         * @param connectTimeout timeout for each connect attempt in milliseconds, so totally connection can take up to connectTimeout*maxConnectionAttempts milliseconds
         * @param readTimeout timeout for read operations in milliseconds
         * @return <code>true</code> if connection successfully established, <code>false</code> otherwise
         */
        bool open(char const* const* nodes, int nNodes, int nReplicas = 1, ReplicationType replType = SQL_REPLICATION,
                        int maxConnectAttempts = 10, int* badNode = NULL, bool localDomain = false, void* sslParameters = NULL,
                        timer_unit connectTimeout = 2*1000, timer_unit readTimeout = 1200*1000, int compressionLevel = 0);



        struct OpenParameters {
            struct NodeParams {
                const char *address;
                void       *sslParameters;
                int         compressionLevel;
                NodeParams() : address(0), sslParameters(0), compressionLevel(0) {}
            };
            NodeParams     *nodes;
            int             nNodes;
            bool            localDomain;
            int             nReplicas;
            ReplicationType replType;
            int             maxConnectAttempts;
            timer_unit      connectTimeout;
            timer_unit      readTimeout;

            OpenParameters(int nNodes) : nodes(new NodeParams[nNodes]), nNodes(nNodes), localDomain(false),
                nReplicas(1), replType(SQL_REPLICATION), maxConnectAttempts(10), connectTimeout(2*1000), readTimeout(1200*1000) {}
            ~OpenParameters() { delete [] nodes; }

        };
        bool open(OpenParameters *params, int *badNode = 0);

        /**
         * Close connection with servers
         */
        void close();

        /**
         * Check if engine is local or remote
         */
        virtual bool isRemote();

        /**
          *  Get last query statistics for the node
          *    return:
          *          true in case of stats were collected, in this case ret_stat recieved stats
          *          false if stats were not collected
          */
        bool getServerQueryStats(int node_no, QueryStat &ret_stat);

        void printQueryStat(FILE *out);

        /**
         * Handler of replica connection failure.
         * Can be redefined in derived class.
         * By default just write message in the log file
         */
        virtual void onReplicaConnectionFailure(int node, int errorCode);

        /**
         * Handler of replica reconect
         * Can be redefined in derived class.
         * By default just write message in the log file
         */
        virtual void onReplicaReconnect(int node);

        /** Constructor of SQL engine remote interface
         * @param txBufSize size of transmit buffer (used to serialize requests to be sent to the server)
         */
        DistributedSqlEngine(size_t txBufSize = 64 * 1024);

        /**
         * Destructor of SQL engine remote interface
         */
        ~DistributedSqlEngine();
        DESTROY(DistributedSqlEngine)


#if COLLECT_DSQL_STATS
        StatsCollector stats_collector;
#endif
    };
}

#endif

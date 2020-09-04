#ifndef ASYNCLN_H
#define ASYNCLN_H


#include <sql/mcosql.h>
#include <sql/sqlcln.h>
#include "mcoapiseq.h"
#include "allocator.h"

#include "sql/value.h"

namespace McoSql {

void _stac_set_error(const char *, const char *file, int line);

#define stac_set_error(str) _stac_set_error((str), __FILE__, __LINE__)
class AsyncConnectionImpl;

#if COLLECT_DSQL_STATS

struct DsqlClientShardStat
{
    uint8 TotalWrite;
    uint8 TotalWriteCount;
    uint8 TotalRead;
    uint8 TotalReadCount;
    uint8 Execute;
    uint8 WriteStart;
    uint8 WriteEnd;
    uint8 ReadStart;
    uint8 ReadEnd;

    DsqlClientShardStat() : TotalWrite(0), TotalWriteCount(0), TotalRead(0), TotalReadCount(0), Execute(0), WriteStart(0), WriteEnd(0), ReadStart(0), ReadEnd(0)
    {}
};

struct DsqlClientStat
{
    DsqlClientShardStat *shards;
    uint4 nshards;
    uint8 GetResults;
    uint8 GetResultsStart;
    uint8 GetResultsEnd;
    uint8 MergeResults;
    uint8 MergeResultsStart;
    uint8 MergeResultsEnd;

    DsqlClientStat() : shards(NULL), nshards(0), GetResults(0), GetResultsStart(0), GetResultsEnd(0), MergeResults(0), MergeResultsStart(0), MergeResultsEnd(0)
    {
    }
};

#endif


class StringList;


struct AsyncQueryParams
{
    McoSql::Value **args;
    size_t nargs;

    AsyncQueryParams(McoSql::Allocator *allocator, size_t size) : nargs(size) {
        args = (McoSql::Value **)allocator->allocate(sizeof(McoSql::Value *) * nargs);
    }
    DESTROY(AsyncQueryParams)
};

struct DQueryStat
{
    int nShards;
    QueryStat *stats;

    DQueryStat(int n) : nShards(n) {
        stats = new McoSql::QueryStat[n];
    }

    ~DQueryStat() {
        delete [] stats;
    }
};


class MCORSQL_API AsyncSqlEngine : public SqlEngine
{
public:

    AsyncSqlEngine(size_t nWorkers = 1, size_t txBufSize = 64 * 1024);

    virtual ~AsyncSqlEngine();

    /**
     * Open connection with servers and synchronously connect all workers using remote engine
     * @param nodes array with node names and ports ("NAME:PORT")
     * @param nNodes number of nodes
     * @param nReplicas number of replicas, should be diveder of nNodes
     * @param replType replication method
     * @param maxConnectAttempts maximal attempts of connection to server
     * @param bad_node if this parameter is not null and open is failed, then index of not unavailable node is stored at this location
     * @param localDomain server and clients are located at the same host
     * @param sslParameters pointer to a mco_ssl_params_t structure
                  containing SSL connection settings
     * @param connectTimeout timeout for each connect attempt in milliseconds, so totally connection can take up to connectTimeout*maxConnectionAttempts milliseconds
     * @param readTimeout timeout for read operations in milliseconds
     * @return <code>true</code> if connection successfully established, <code>false</code> otherwise
     */
    bool open(char const* hostname, int port, int maxConnectAttempts = 10, bool localDomain = false, void* sslParameters = NULL,
              timer_unit connectTimeout = 2*1000, timer_unit readTimeout = 1200*1000, int compressionLevel = 0);


    /**
     * Start connection pool, and synchronously connect all workers using distributed engines
     * @param nodes array with node names and ports ("NAME:PORT")
     * @param nNodes number of nodes
     * @param nReplicas number of replicas, should be diveder of nNodes
     * @param replType replication method
     * @param maxConnectAttempts maximal attempts of connection to server
     * @param bad_node if this parameter is not null and open is failed, then index of not unavailable node is stored at this location
     * @param localDomain server and clients are located at the same host
     * @param sslParameters pointer to a mco_ssl_params_t structure
                 containing SSL connection settings
     * @param connectTimeout timeout for each connect attempt in milliseconds, so totally connection can take up to connectTimeout*maxConnectionAttempts milliseconds
     * @param readTimeout timeout for read operations in milliseconds
     * @param compressionLevel Level of compression 0..9
     * @return <code>true</code> if connection successfully established, <code>false</code> otherwise
     */
    bool open(char const* const* nodes, int nNodes, int nReplicas = 1, DistributedSqlEngine::ReplicationType replType = DistributedSqlEngine::SQL_REPLICATION,
                    int maxConnectAttempts = 10, int* badNode = NULL, bool localDomain = false, void* sslParameters = NULL,
                    timer_unit connectTimeout = 2*1000, timer_unit readTimeout = 1200*1000 , int compressionLevel = 0);



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
        DistributedSqlEngine::ReplicationType replType;
        int             maxConnectAttempts;
        timer_unit      connectTimeout;
        timer_unit      readTimeout;
        size_t          nWorkers;

        OpenParameters(int nNodes) : nodes(new NodeParams[nNodes]), nNodes(nNodes), localDomain(false),
            nReplicas(1), replType(DistributedSqlEngine::SQL_REPLICATION), maxConnectAttempts(10), connectTimeout(2*1000), readTimeout(1200*1000), nWorkers(2) {}
        ~OpenParameters() { delete [] nodes; }

    };
    bool open(OpenParameters *params, int *badNode = 0);

    /**
     * Prepared queries are not supported
     */
    void vprepare(PreparedStatement &stmt, char const* sql, ParamDesc* params)
    {
        throw McoSqlException(McoSqlException::INVALID_OPERATION,
                              String::create(&allocator, "Execution of synchronous query not supported"));
    }


    /**
     * Remotely execute prepared query at server, synchronously using first worker
     */
    ResultSet* executePrepared(PreparedStatement &stmt, Transaction* trans, int64_t& nRecords)
    {
        throw McoSqlException(McoSqlException::INVALID_OPERATION,
                              String::create(&allocator, "Execution of synchronous query not supported"));
    }


    /**
     * Remotely execute query at server is not supported
     */
    ResultSet* vexecute(Transaction* trans, char const* sql, va_list* list, Value** array, size_t arraySize, int64_t& nRecords)
    {
        throw McoSqlException(McoSqlException::INVALID_OPERATION,
                              String::create(&allocator, "Execution of synchronous query not supported"));
    }



    // Close connections to servers and stop workers
    virtual void close();

    /**
     * Check if engine is local or remote
     */
    virtual bool isRemote() {
        return true;
    }

    // Start executing using array of workers.
    void executeMany(Vector<String> *queries, Vector<AsyncQueryParams> *args, Allocator *ret_allocator);

    // Wait for all queries to finish
    // Return results for all queries
    Vector<Table> *waitMany();

    void destroy(Allocator* allocator) {
        this->~AsyncSqlEngine();
        allocator->free(this, sizeof(AsyncSqlEngine));
    }

    size_t getnWorkers();
    size_t getnNodes();

    String *getError();

    void copy_connection_stats(DQueryStat *stats, size_t nstats);

    QueryStat *get_worker_stat(size_t i_wrk);
#if COLLECT_DSQL_STATS
    DsqlClientStat *get_worker_client_stat(size_t i_wrk);
#endif
protected:
    Allocator allocator;
    AsyncConnectionImpl *conn;
    Vector<Table> *ret;    
};

}

#endif // ASYNCLN_H


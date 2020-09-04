package com.mcobject.extremedb;

/**
 * Remote database connection for execution of SQL queries.
 */
public class SqlRemoteConnection implements SqlConnection, java.io.Closeable
{    
	public static class OpenParameters
	{
		public String host = null;		 
		public int port;
		public String[] nodes;
		public int nReplicas;
		public ReplicationType replType;		
		public int maxAttempts;
		public int txBufSize;
		public int connectTimeout;
		public int readTimeout;
		public int compressionLevel;
		
		public Database.SSLParameters sslParameters;
		public String user;
		public String password;
		
		public OpenParameters() {
			host = null;
			port = 0;
			nodes = null;
			nReplicas = 1;
			replType = ReplicationType.SQLReplication;
			maxAttempts = 10;
			txBufSize = 64*1024;
			connectTimeout = 2000;
			readTimeout = 1200*1000;
			compressionLevel = 0;
			sslParameters = null;
			user = null;
			password = null;
		}
	}
	
	public SqlRemoteConnection(OpenParameters params) {
		if (params.host != null) {
			engine = open(params.host, params.port, params.maxAttempts, params.txBufSize, 
					params.connectTimeout, params.readTimeout,  params.compressionLevel,
					params.sslParameters, params.user, params.password );
		} else if (params.nodes != null) {
			engine = openDistributed(params.nodes, params.nReplicas, params.replType.ordinal(), 
					params.maxAttempts, params.txBufSize, params.connectTimeout, params.readTimeout, params.compressionLevel,
					params.sslParameters, params.user, params.password);
		} else {
			throw new IllegalArgumentException("host or nodes should be set in OpenParameters"); 
		}
	}
	
   /**
     * Constructor of the remote database connection.
     * @param host hostname of xsql server
     * @param port port of xsql server
     * @param maxAttempts maximal attempts of connection to server, interval between 
     * each connection attempt is one second, so totally connection will be performed
     * during maxAttempts seconds
     */
    public SqlRemoteConnection(String host, int port, int maxAttempts) { 
        engine = open(host, port, maxAttempts, 64*1024, 2000, 1200*1000, 0, null, null, null);
    }

     /**
     * Constructor of the remote database connection.
     * @param host hostname of xsql server
     * @param port port of xsql server
     * @param maxAttempts maximal attempts of connection to server, interval between 
     * each connection attempt is one second, so totally connection will be performed
     * during maxAttempts seconds
     * @param txBufSize size of internal transmit buffer in bytes. Required to serialize
     * query or statement data
     */
    public SqlRemoteConnection(String host, int port, int maxAttempts, int txBufSize) { 
        engine = open(host, port, maxAttempts, txBufSize, 2000, 1200*1000, 0, null, null, null);
    }

     /**
     * Constructor of the remote database connection.
     * @param host hostname of xsql server
     * @param port port of xsql server
     * @param maxAttempts maximal attempts of connection to server, interval between 
     * each connection attempt is one second, so totally connection will be performed
     * during maxAttempts seconds
     * @param txBufSize size of internal transmit buffer in bytes. Required to serialize
     * query or statement data
     * @param connectTimeout timeout for each connect attempt in milliseconds, so totally 
     * connection can take up to connectTimeout*maxConnectionAttempts milliseconds 
     * @param readTimeout timeout for read operations in milliseconds 
     */
    public SqlRemoteConnection(String host, int port, int maxAttempts, int txBufSize, int connectTimeout, int readTimeout) { 
        engine = open(host, port, maxAttempts, txBufSize, connectTimeout, readTimeout, 0, null, null, null);
    }


    public enum ReplicationType { SQLReplication, HAReplication };
   /**
     * Constructor of the distributed database connection.
     * @param nodes database nodes (each entry should have format "ADDRESS:PORT")
     * @param nReplicas redundancy level
     * @param replType replication method
     * @param maxAttempts maximal attempts of connection to server, interval between 
     * each connection attempt is one second, so totally connection will be performed
     * during maxAttempts seconds
     * @param txBufSize size of internal transmit buffer in bytes. Required to serialize
     * query or statement data
     */
    public SqlRemoteConnection(String[] nodes, int nReplicas, ReplicationType replType, int maxAttempts, int txBufSize) { 
        engine = openDistributed(nodes, nReplicas, replType.ordinal(), maxAttempts, txBufSize, 2000, 1200*1000, 0, null, null, null);
    }

   /**
     * Constructor of the distributed database connection.
     * @param nodes database nodes (each entry should have format "ADDRESS:PORT")
     */
    public SqlRemoteConnection(String[] nodes) { 
        engine = openDistributed(nodes, 1, ReplicationType.SQLReplication.ordinal(), 10, 64*1024, 2000, 1200*1000, 0, null, null, null);
    }

   /**
     * Constructor of the distributed database connection.
     * @param nodes database nodes (each entry should have format "ADDRESS:PORT")
     * @param nReplicas redundancy level
     * @param replType replication method
     * @param maxAttempts maximal attempts of connection to server, interval between 
     * each connection attempt is one second, so totally connection will be performed
     * during maxAttempts seconds
     * @param txBufSize size of internal transmit buffer in bytes. Required to serialize
     * query or statement data
     * @param connectTimeout timeout for each connect attempt in milliseconds, so totally 
     * connection can take up to connectTimeout*maxConnectionAttempts milliseconds 
     * @param readTimeout timeout for read operations in milliseconds 
     */
    public SqlRemoteConnection(String[] nodes, int nReplicas, ReplicationType replType, int maxAttempts, int txBufSize, int connectTimeout, int readTimeout) {  
        engine = openDistributed(nodes, nReplicas, replType.ordinal(), maxAttempts, txBufSize, connectTimeout, readTimeout, 0, null, null, null);
    }

    /**
     * Execute SQL query. 
     * @param query SQL query with '?' used as parameter placeholder.
     * @return cursor through selected tuples
     */
    public SqlResultSet executeQuery(String query, Object... params) {
        if (result != null) { 
            result.close();
        }
        return result = new SqlResultSet(this, executeQuery(engine, query, params));
    }

    /**
     * Execute SQL update statement 
     * @param stmt SQL DML statement with '?' used as parameter placeholder.
     * @return number of affected (updated, inserted, deleted) records
     */
    public int executeStatement(String stmt, Object... params) {
        return executeStatement(engine, stmt, params);
    }

    public void detachResultSet(SqlResultSet res) { 
        result = null;
    }
        

    /**
     * Close this connection.
     */
    public void disconnect() { 
        if (engine != 0) { 
            if (result != null) { 
                result.close();
                result = null;
            }
            close(engine);
            engine = 0;
        }
    }

    /**
     * Close this connection. The same as disconnect.
     */
    public void close() {
        disconnect();
    }

    private SqlResultSet result;
    private long engine;

    private native long open(String host, int port, int maxAttempts, int txBufSize, int connectTimeout, int readTimeout, int compressionLevel, Database.SSLParameters params, String user, String password);
    private native long openDistributed(String[] nodes, int nReplicas, int replType, int maxAttempts, int txBufSize, int connectTimeout, int readTimeout, int compressionLevel, Database.SSLParameters params, String user, String password);
    private native long executeQuery(long engine, String query, Object[] params);
    private native int  executeStatement(long engine, String stmt, Object[] params);
    private native void close(long engine);
}
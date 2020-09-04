package com.mcobject.extremedb;

/**
 * Remote database connection for execution of SQL queries.
 */
public class SqlAggregator implements SqlConnection, java.io.Closeable
{    
   /**
     * Constructor of the SQL aggregating.
     * @param engines a list of engines used to aggregate
     */
     public SqlAggregator(SqlLocalConnection [] engines) {
		init(engines, 64*1024);
     }

     public SqlAggregator(SqlLocalConnection [] engines, long bufferSize) {
         init(engines, bufferSize);
     }

    private void init(SqlLocalConnection [] engines, long bufferSize) 
    {
         long [] _engines = new long[engines.length];
         for (int i = 0; i < engines.length; i++) {
             _engines[i] = engines[i].getEngine();
         }

         agg = open(_engines, bufferSize);        
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
        return result = new SqlResultSet(this, executeQuery(agg, query, params));
    }

    /**
     * Execute SQL update statement 
     * @param stmt SQL DML statement with '?' used as parameter placeholder.
     * @return number of affected (updated, inserted, deleted) records
     */
    public int executeStatement(String stmt, Object... params) {
        return executeStatement(agg, stmt, params);
    }

    public void detachResultSet(SqlResultSet res) { 
        result = null;
    }
        
    private long agg;

    /**
     * Close this connection.
     */
    public void disconnect() { 
	    close(agg);
    }

    /**
     * Close this connection. The same as disconnect.
     */
    public void close() {
        disconnect();
    }

    private SqlResultSet result;    

    private native long open(long [] engines, long bufsize);
    private native long executeQuery(long engine, String query, Object[] params);
    private native int  executeStatement(long engine, String stmt, Object[] params);
    private native void close(long engine);
}

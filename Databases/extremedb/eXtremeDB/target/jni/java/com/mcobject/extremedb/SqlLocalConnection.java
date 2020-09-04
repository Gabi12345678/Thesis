package com.mcobject.extremedb;

import java.util.*;

/**
 * Local database connection for execution of SQL queries.
 * Each thread working with the database should have its own database connection.
 */
public class SqlLocalConnection extends Connection implements SqlConnection
{
    /**
     * Constructor of the local database connection.
     * @param db opened database
     */
    public SqlLocalConnection(Database db) { 
        super(db);        
        engine = open(id);
    }
 
    /**
     * Constructor of Sql connection using existed connections
     * @param con existed database connection
     */
    public SqlLocalConnection(Connection con) { 
        super(con); 
        isCopy = true;
        engine = open(id);
    }
 
    /**
     * Execute SQL query. 
     * @param query SQL query with '?' used as parameter placeholder.
     * @return cursor through selected tuples
     */
    public SqlResultSet executeQuery(String query, Object... params) {
        SqlResultSet result = new SqlResultSet(this, executeQuery(engine, trans, query, params));
        results.add(result);
        return result;
    }

    /**
     * Execute SQL update statement 
     * @param stmt SQL DML statement with '?' used as parameter placeholder.
     * @return number of affected (updated, inserted, deleted) records
     */
    public int executeStatement(String stmt, Object... params) {
        return executeStatement(engine, trans, stmt, params);
    }
    
    public void startTransaction(Database.TransactionType type, Database.TransactionPriority pri, Database.IsolationLevel level) {
        if (nNestedTrans++ == 0) { 
            trans = startSqlTransaction(engine, type.ordinal(), pri.ordinal(), level.ordinal());
        }
    }

    public boolean commitTransaction() {
        if (nNestedTrans > 0) { 
            if (--nNestedTrans == 0) { 
                trans = 0;
                return super.commitTransaction();
            }
            return true;
        } else { 
            return false;
        }
    }

    public void rollbackTransaction() {
        trans = 0;
        nNestedTrans = 0;
        super.rollbackTransaction();
    }

    /**
     * Close this connection.
     */
    public void disconnect() {
        if (engine != 0) {
            Set<SqlResultSet> attachedResults = results;
            results = null;
            for (SqlResultSet result : attachedResults) { 
                result.close();
            }
            close(engine);
            engine = 0;
        }
        if (!isCopy) { 
            super.disconnect();
        }
    }

    /**
     * Close this connection. The same as disconnect.
     */
    public void close() {
        disconnect();
    }

    public void detachResultSet(SqlResultSet result) { 
        if (results != null) { 
            results.remove(result);
        }
    }
    
    public long getEngine() {
    	return engine;
    }
    
    private long engine;
    private long trans;
    private int  nNestedTrans;
    private boolean isCopy;
    private Set<SqlResultSet> results = new HashSet<SqlResultSet>();;

    private native long open(long connection);
    private native long executeQuery(long engine, long trans, String query, Object[] params);
    private native int  executeStatement(long engine, long trans, String stmt, Object[] params);
    private native void close(long engine);
    private native long startSqlTransaction(long engine, int type, int pri, int level);
}


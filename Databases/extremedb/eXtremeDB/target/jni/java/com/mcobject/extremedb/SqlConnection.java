package com.mcobject.extremedb;

import java.util.*;

/**
 * Database connection for execution of SQL queries.
 */
public interface SqlConnection
{
    /**
     * Execute SQL query. 
     * @param query SQL query with '?' used as parameter placeholder.
     * @return cursor through selected tuples
     */
    public SqlResultSet executeQuery(String query, Object... params);

    /**
     * Execute SQL update statement 
     * @param stmt SQL DML statement with '?' used as parameter placeholder.
     * @return number of affected (updated, inserted, deleted) records
     */
    public int executeStatement(String stmt, Object... params);


    /**
     * Close this connection.
     */
    public void disconnect();


    /**
     * This is internal method which is invoked by SqlResultSet and should not be used by application
     */
    public void detachResultSet(SqlResultSet result);
}

    
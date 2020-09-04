package com.mcobject.extremedb;

import java.util.*;

/**
 * Class used to iterate through result of SQL query
 */
public class SqlCursor implements Iterator<SqlTuple>, java.io.Closeable
{
    /**
     * Check if there is a next tuple.
     */
    public boolean hasNext() {
        return hasNext(id);
    }

    /**
     * Get next tuple
     */
    public SqlTuple next() {
        return new SqlTuple(rs, next(id));
    }

    public void remove() {
        throw new UnsupportedOperationException("SqlCursor.remove");
    }

    /**
     * Close the iterator
     */
    public void close() {
        if (id != 0) {
            rs.detachCursor(this);
            close(id);
            id = 0;
        }
    }

    protected void finalize() {
        close();
    }

    SqlCursor(SqlResultSet resultSet, long cursor) {
        rs = resultSet;
        id = cursor;
    }

    private SqlResultSet rs;
    private long id;

    private native boolean hasNext(long id);
    private native long next(long id);
    private native void close(long id);
}

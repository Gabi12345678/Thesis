package com.mcobject.extremedb;

import java.util.*;



/**
 * Result of SQL query
 */
public class SqlResultSet implements Iterable<SqlTuple>, java.io.Closeable
{
    /**
     * Get names of result columns
     */
    public String[] getColumnNames() {
        if (columnNames == null) {
            columnNames = getColumnNames(id);
        }
        return columnNames;
    }

    private static final Class sql2javaTypeMap[] = {
        Object.class,  // tpNull,
        Boolean.class, // tpBool,
        Long.class,    // tpInt1,
        Long.class,    // tpUInt1,
        Long.class,    // tpInt2,
        Long.class,    // tpUInt2,
        Long.class,    // tpInt4,
        Long.class,    // tpUInt4,
        Long.class,    // tpInt8,
        Long.class,    // tpUInt8,
        Double.class,  // tpReal4,
        Double.class,  // tpReal8,
        Date.class,    // tpDateTime,
        java.math.BigDecimal.class, // tpNumeric,
        String.class,  // tpUnicode,
        String.class,  // tpString,
        byte[].class,  // tpRaw,
        Long.class,    // tpReference,
        null,  // tpArray,
        null,  // tpStruct,
        byte[].class,  // tpBlob,
        null,  // tpDataSource,
        null,  // tpList,
        SequenceIterator.class, // tpSequence
        byte[].class, // RSQL + tpSequence = tpArray + element type tpInt1
        byte[].class, // RSQL + tpSequence = tpArray + element type tpUInt1
        short[].class, // RSQL + tpSequence = tpArray + element type tpInt2
        char[].class, // RSQL + tpSequence = tpArray + element type tpUInt2
        int[].class, // RSQL + tpSequence = tpArray + element type tpInt4
        int[].class, // RSQL + tpSequence = tpArray + element type tpUInt4
        long[].class, // RSQL + tpSequence = tpArray + element type tpInt8
        long[].class, // RSQL + tpSequence = tpArray + element type tpUInt8
        float[].class, // RSQL + tpSequence = tpArray + element type tpReal4
        double[].class, // RSQL + tpSequence = tpArray + element type tpReal8
        String[].class, // RSQL + tpSequence = tpArray + element type tpString
        Date[].class    // RSQL + tpSequence = tpArray + element type tpDateTime
    };

    /**
     * Get types of result columns
     */
    public Class[] getColumnTypes() {
        if (columnTypes == null) {
            int[] sqlTypes = getColumnTypes(id);
            columnTypes = new Class[sqlTypes.length];
            for (int i = 0; i < sqlTypes.length; i++) {
                columnTypes[i] = sql2javaTypeMap[sqlTypes[i]];
            }
        }
        return columnTypes;
    }

    /**
     * Get number of result columns
     */
    public int getNumberOfColumns() {
        return getNumberOfColumns(id);
    }

    /**
     * Find column index by name
     * @param column name of the result column
     * @return 0-based index of this column in result set
     */
    public int getColumnNo(String column) {
        String[] columns = getColumnNames();
        for (int i = 0 ; i < columns.length; i++) {
            if (column.equals(columns[i])) {
                return i;
            }
        }
        throw new IllegalArgumentException("No such column " + column);
    }


    /**
     * Close the cursor.
     */
    public void close() {
        if (id != 0) {
            Set<SqlCursor> attachedCursors = cursors;
            cursors = null;
            for (SqlCursor cursor : attachedCursors) {
                cursor.close();
            }
            con.detachResultSet(this);
            close(id);
            id = 0;
        }
    }

    protected void finalize() {
        close();
    }

    /**
     * Get the iterator.
     */
    public Iterator<SqlTuple> iterator() {
        SqlCursor cursor = new SqlCursor(this, getIterator(id));
        cursors.add(cursor);
        return cursor;
    }

    void detachCursor(SqlCursor cursor) {
        if (cursors != null) {
            cursors.remove(cursor);
        }
    }

    SqlResultSet(SqlConnection c, long ds) {
        con = c;
        id = ds;
        cursors = new HashSet<SqlCursor>();
    }

    private long id;
    private SqlConnection con;
    Set<SqlCursor> cursors;
    private String[] columnNames;
    private Class[] columnTypes;

    private native long getIterator(long id);
    private native String[] getColumnNames(long id);
    private native int[] getColumnTypes(long id);
    private native int getNumberOfColumns(long id);
    private native void close(long id);
}

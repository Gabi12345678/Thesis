package com.mcobject.extremedb;

/**
 * Unit of SQL query result
 */
public class SqlTuple { 
    /**
     * Get number of components in tuple
     * @return number of components
     */
    public int size() { 
        return size(id);
    }

    /**
     * Get value of the result column with specified index
     * @param i column index (0-based)
     * @return column's value
     */
    public Object get(int i) { 
        return get(id, i);
    }

    /**
     * Get integer value of the column with specified index
     * @param i column index (0-based)
     * @return column's value
     */
    public long getInt(int i) { 
        return getInt(id, i);
    }

    /**
     * Get floating point value of the column with specified index
     * @param i column index (0-based)
     * @return column's value
     */
    public double getReal(int i) {
        return getReal(id, i);
    }


    /**
     * Get value of the column with specified column name
     * @param column column name
     * @return column's value
     */
    public Object get(String column) { 
        return get(rs.getColumnNo(column));
    }

    /**
     * Get integer value of the column with specified column name
     * @param column column name
     * @return column's value
     */
    public long getInt(String column) { 
        return getInt(rs.getColumnNo(column));
    }

    /**
     * Get floating point value of the column with specified column name
     * @param column column name
     * @return column's value
     */
    public double getReal(String column) {
        return getReal(rs.getColumnNo(column));
    }


    SqlTuple(SqlResultSet resultSet, long record) { 
        rs = resultSet;
        id = record;
    }
        
    private long id;
    private SqlResultSet rs;

    private native int size(long id);
    private native Object get(long id, int i);
    private native long getInt(long id, int i);
    private native double getReal(long id, int i);
}
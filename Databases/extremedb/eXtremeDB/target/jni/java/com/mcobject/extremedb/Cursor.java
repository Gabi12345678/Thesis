package com.mcobject.extremedb;

import java.util.*;
import java.io.UnsupportedEncodingException;

public class Cursor<T> implements Iterable<T>, Iterator<T>, java.io.Closeable
{
    static final int ARRAY_MARKER = 0x80000000;

    /** 
     * Construct a cursor to iterate through instances of the specified class. (inheritance is not considered)
     * @param con a database connection.
     * @param cls the iterated class.
     * @param indexName the name of the index used for iteration and searching.
     */
    public Cursor(Connection con, Class cls, String indexName) {
        init(con, cls);
        indexDesc = classDesc.indicesMap.get(indexName);
        if (indexDesc == null) { 
            throw new IllegalArgumentException("No class index " + indexName + " in class " + cls);
        }
        indexNo = indexDesc.indexNo;
        unique = indexDesc.unique;
        hash = indexDesc.type == Database.IndexType.Hashtable;        
        types = new int[indexDesc.keys.length];
        for (int i = 0; i < types.length; i++) { 
            int type = indexDesc.keys[i].field.type;
            /*
            if ((indexDesc.keys[i].field.flags & (Database.MCO_DICT_FLDF_ARRAY|Database.MCO_DICT_FLDF_VECTOR)) != 0) {
                type |= ARRAY_MARKER;
            } */
            if (indexDesc.type == Database.IndexType.RTree || indexDesc.type == Database.IndexType.RTreeOfPoint) {
                type |= ARRAY_MARKER;
            }
            types[i] = type;
        }
        cursor = openIndexCursor(con.id, indexNo);
    }

    /** 
     * Construct a cursor to iterate through instances of the specified class. (inheritance is not considered.)
     * (The class must have the list or AUTOID index (list() or autoid()) attributes of the Persistent annotation set.)
     * @param con a database connection.
     * @param cls the iterated class.
     */
    public Cursor(Connection con, Class cls) {
        init(con, cls);
        if (classDesc.autoIdIndexNo < 0 && classDesc.listIndexNo < 0) { 
            throw new IllegalArgumentException("Class has no associated AUTOID or list indices");
        }
        if (classDesc.autoIdIndexNo >= 0) { 
            unique = true;
            hash = true;
            types = new int[]{Database.MCO_DB_FT_AUTOID};
            indexNo = classDesc.autoIdIndexNo;
            cursor = openIndexCursor(con.id, indexNo);
        } else { 
            indexNo = -1;
            cursor = openListCursor(con.id, classCode);
        }
    }

    private void init(Connection con, Class cls) {
        this.con = con;
        classDesc = con.db.getClassDescriptor(cls);
        classCode = classDesc.classNo;
        pos = Position.Unknown;
        dir = Direction.Unknown;
    }

    /**
     * Search operations
     */
    public enum Operation { 
        LessThan,
        LessOrEquals,
        Equals,
        GreaterOrEquals,
        GreaterThan,
        Overlaps,
        Contains,
        ExactMatch,
        BestMatch,
        PrefixMatch,
        NextMatch,
        Neighbourhood,
        StrictEquals,
        IsPrefixOf,
        ContainsAll,
        ContainsAny
    };

    private void checkTypes(Object[] values) {
        if (values.length != types.length) { 
            throw new IllegalArgumentException("Number of keys doesn't match with index");
        }
        for (int i = 0; i < values.length; i++) { 
            int type = types[i];
            Object value = values[i];
            if (value == null) { 
                throw new IllegalArgumentException("Key value should not be null");
            }
            if (types[i] == Database.MCO_DB_FT_CHARS || types[i] == Database.MCO_DB_FT_STRING) { 
                if (!(value instanceof String)) { 
                    throw new IllegalArgumentException("String key value expected");
                }
                String encoding = indexDesc.keys[i].field.encoding;
                try { 
                    values[i] = ((String)value).getBytes(encoding);
                } catch (UnsupportedEncodingException x) { 
                    throw new RuntimeException(x);
                }
            }
        }
    }                    
            
    private void checkArrayType(Object[] values) {
        if (types.length != 1 || !indexDesc.thick || (indexDesc.keys[0].field.flags & (Database.MCO_DICT_FLDF_ARRAY|Database.MCO_DICT_FLDF_VECTOR)) == 0) { 
            throw new IllegalArgumentException("ContainsAll/ContainsAny operations are applicable only to thick array indexes");
        }  
        int type = types[0];
        if (type == Database.MCO_DB_FT_CHARS || type == Database.MCO_DB_FT_STRING) { 
            String[] set = (String[])values[0];
            byte[][] chars = new byte[set.length][];
            String encoding = indexDesc.keys[0].field.encoding;
            for (int i = 0; i < set.length; i++) { 
                String value = set[i];
                if (value == null) { 
                    throw new IllegalArgumentException("Key value should not be null");
                }
                try { 
                    chars[i] = value.getBytes(encoding);
                } catch (UnsupportedEncodingException x) { 
                    throw new RuntimeException(x);
                }
            }
            values[0] = chars;
        } 
        else if (type == Database.MCO_DB_FT_WIDE_CHARS || type == Database.MCO_DB_FT_WCHAR_STRING
                 || type == Database.MCO_DB_FT_UNICODE_CHARS || type == Database.MCO_DB_FT_UNICODE_STRING) 
        {
            String[] set = (String[])values[0];            
            for (int i = 0; i < set.length; i++) { 
                if (set[i] == null) { 
                    throw new IllegalArgumentException("Key value should not be null");
                }
            }
        }
    }                    
            

    /**
     * Perform the specified search operation. This method can be used for any type of index; the search operations
     * supported depend on index type.  The table below specifies valid operations for each index type:
     * <table border>
     * <tr><th>Index type</td><td>Supported operations</td></tr>
     * <tr><td>Hashtable</td><td>Equals</td></tr>
     * <tr><td>BTrtee</td><td>LessThan<br>LessOrEquals<br>Equals<br>GreaterThan<br>GreaterOrEquals</td></tr>
     * <tr><td>RTree</td><td>Equals<br>Overlaps<br>Contains<br>Neighbourhood</td></tr>
     * <tr><td>Patricia</td><td>Equals<br>ExactMatch<br>BestMatch<br>PrefixMatch<br>NextMatch</td></tr>
     * <tr><td>Trigram</td><td>Contains</td></tr>
     * </table>
     * This method positions the cursor at the first element matching the specified search condition.
     * Move forward or backward to locate all other elements for which this condition is true. 
	 * For example, in a "less than" search move backward and in a "greater or equals" search move forward. 
     * @param value the search key.
     * @return false if no results are found.
     */
    public boolean search(Operation op, Object... value) {
        if (indexNo < 0) { 
            throw new IllegalArgumentException("Can not search in list cursor");
        }
        if (hash && op != Operation.Equals && op != Operation.StrictEquals) { 
            throw new IllegalArgumentException("Operation is not applicable for hashtable");
        }
        if (op == Operation.ContainsAll || op == Operation.ContainsAny) { 
            checkArrayType(value);
        } else { 
            checkTypes(value);
        }
        reset();
        if (search(con.id, indexNo, cursor, op.ordinal()+1, types, hash, value)) { 
            pos = Position.First;
            return true;
        } else { 
            pos = Position.Empty;
            return false;
        }
    }

    /**
     * Perform the specified search operation for integer key. This method can be used for any type of index; the search operations
     * supported depend on index type.  The table below specifies valid operations for each index type:
     * <table border>
     * <tr><th>Index type</td><td>Supported operations</td></tr>
     * <tr><td>Hashtable</td><td>Equals</td></tr>
     * <tr><td>BTrtee</td><td>LessThan<br>LessOrEquals<br>Equals<br>GreaterThan<br>GreaterOrEquals</td></tr>
     * <tr><td>RTree</td><td>Equals<br>Overlaps<br>Contains<br>Neighbourhood</td></tr>
     * <tr><td>Patricia</td><td>Equals<br>ExactMatch<br>BestMatch<br>PrefixMatch<br>NextMatch</td></tr>
     * <tr><td>Trigram</td><td>Contains</td></tr>
     * </table>
     * This method positions the cursor at the first element matching the specified search condition.
     * Move forward or backward to locate all other elements for which this condition is true. 
	 * For example, in a "less than" search move backward and in a "greater or equals" search move forward. 
     * @param value the search key.
     * @return false if no results are found.
     */
    public boolean search(Operation op, long value) {
        if (indexNo < 0) { 
            throw new IllegalArgumentException("Can not search in list cursor");
        }
        if (hash && op != Operation.Equals && op != Operation.StrictEquals) { 
            throw new IllegalArgumentException("Operation is not applicable for hashtable");
        }
        if (types.length != 1) { 
            throw new IllegalArgumentException("Number of keys doesn't match with index");
        }
        reset();
        if (searchInt(con.id, indexNo, cursor, op.ordinal()+1, types[0], hash, value)) { 
            pos = Position.First;
            return true;
        } else { 
            pos = Position.Empty;
            return false;
        }
    }

    /**
     * Perform the specified search operation for floating point key. This method can be used for any type of index; the search operations
     * supported depend on index type.  The table below specifies valid operations for each index type:
     * <table border>
     * <tr><th>Index type</td><td>Supported operations</td></tr>
     * <tr><td>Hashtable</td><td>Equals</td></tr>
     * <tr><td>BTrtee</td><td>LessThan<br>LessOrEquals<br>Equals<br>GreaterThan<br>GreaterOrEquals</td></tr>
     * <tr><td>RTree</td><td>Equals<br>Overlaps<br>Contains<br>Neighbourhood</td></tr>
     * <tr><td>Patricia</td><td>Equals<br>ExactMatch<br>BestMatch<br>PrefixMatch<br>NextMatch</td></tr>
     * <tr><td>Trigram</td><td>Contains</td></tr>
     * </table>
     * This method positions the cursor at the first element matching the specified search condition.
     * Move forward or backward to locate all other elements for which this condition is true. 
	 * For example, in a "less than" search move backward and in a "greater or equals" search move forward. 
     * @param value the search key.
     * @return false if no results are found.
     */
    public boolean search(Operation op, double value) {
        if (indexNo < 0) { 
            throw new IllegalArgumentException("Can not search in list cursor");
        }
        if (hash && op != Operation.Equals && op != Operation.StrictEquals) { 
            throw new IllegalArgumentException("Operation is not applicable for hashtable");
        }
        if (types.length != 1) { 
            throw new IllegalArgumentException("Number of keys doesn't match with index");
        }
        reset();
        if (searchReal(con.id, indexNo, cursor, op.ordinal()+1, types[0], hash, value)) { 
            pos = Position.First;
            return true;
        } else { 
            pos = Position.Empty;
            return false;
        }
    }

    /**
     * Perform the specified search operation for string key. This method can be used for any type of index; the search operations
     * supported depend on index type.  The table below specifies valid operations for each index type:
     * <table border>
     * <tr><th>Index type</td><td>Supported operations</td></tr>
     * <tr><td>Hashtable</td><td>Equals</td></tr>
     * <tr><td>BTrtee</td><td>LessThan<br>LessOrEquals<br>Equals<br>GreaterThan<br>GreaterOrEquals</td></tr>
     * <tr><td>RTree</td><td>Equals<br>Overlaps<br>Contains<br>Neighbourhood</td></tr>
     * <tr><td>Patricia</td><td>Equals<br>ExactMatch<br>BestMatch<br>PrefixMatch<br>NextMatch</td></tr>
     * <tr><td>Trigram</td><td>Contains</td></tr>
     * </table>
     * This method positions the cursor at the first element matching the specified search condition.
     * Move forward or backward to locate all other elements for which this condition is true. 
	 * For example, in a "less than" search move backward and in a "greater or equals" search move forward. 
     * @param value the search key.
     * @return false if no results are found.
     */
    public boolean search(Operation op, String value) {
        boolean found;
        if (indexNo < 0) { 
            throw new IllegalArgumentException("Can not search in list cursor");
        }
        if (hash && op != Operation.Equals && op != Operation.StrictEquals) { 
            throw new IllegalArgumentException("Operation is not applicable for hashtable");
        }
        if (types.length != 1) { 
            throw new IllegalArgumentException("Number of keys doesn't match with index");
        }
        reset();
        if (types[0] == Database.MCO_DB_FT_CHARS || types[0] == Database.MCO_DB_FT_STRING) { 
            String encoding = indexDesc.keys[0].field.encoding;
            byte[] bytes;
            try { 
                bytes = value.getBytes(encoding);
            } catch (UnsupportedEncodingException x) { 
                throw new RuntimeException(x);
            }
            found = searchBytes(con.id, indexNo, cursor, op.ordinal()+1, types[0], hash, bytes); 
        } else { 
            found = searchStr(con.id, indexNo, cursor, op.ordinal()+1, types[0], hash, value);
        }
        if (found) { 
            pos = Position.First;
            return true;
        } else { 
            pos = Position.Empty;
            return false;
        }
    }

    /**
     * Perform an exact match search on a unique index. This method is equivalent to
     * <pre>
     *     cursor.search(Cursor.Equals, value);
     *     T result = cursor.next();
     * </pre>
     * @param value the search key.
     * @return an object with specified key or null if no object with this key value is found.
     */
    public T find(Object... value) {
        if (!unique) { 
            throw new IllegalArgumentException("Find can be used only for unique indices");
        }
        if (!hash) { 
            return search(Operation.Equals, value) ? next() : null;
        }
        checkTypes(value);
        reset();
        pos = Position.Empty;
        currHandle = find(con.id, indexNo, types, value); 
        if (currHandle != 0) { 
            currObj = (T)con.db.loadObject(currHandle, classDesc);
            return currObj;
        }
        return null;
    }
        
    /**
     * Perform an exact match search of integer key on a unique index. This method is equivalent to
     * <pre>
     *     cursor.search(Cursor.Equals, value);
     *     T result = cursor.next();
     * </pre>
     * @param value the search key.
     * @return an object with specified key or null if no object with this key value is found.
     */
    public T find(long value) {
        if (!unique) { 
            throw new IllegalArgumentException("Find can be used only for unique indices");
        }
        if (!hash) { 
            return search(Operation.Equals, value) ? next() : null;
        }
        if (types.length != 1) { 
            throw new IllegalArgumentException("Number of keys doesn't match with index");
        }
        reset();
        pos = Position.Empty;
        currHandle = findInt(con.id, indexNo, types[0], value); 
        if (currHandle != 0) { 
            currObj = (T)con.db.loadObject(currHandle, classDesc);
            return currObj;
        }
        return null;
    }
        
    /**
     * Perform an exact match search of integer key on a unique index. This method is equivalent to
     * <pre>
     *     cursor.search(Cursor.Equals, value);
     *     T result = cursor.next();
     * </pre>
     * @param value the search key.
     * @return an object with specified key or null if no object with this key value is found.
     */
    public T find(String value) {
        if (!unique) { 
            throw new IllegalArgumentException("Find can be used only for unique indices");
        }
        if (!hash) { 
            return search(Operation.Equals, value) ? next() : null;
        }
        if (types.length != 1) { 
            throw new IllegalArgumentException("Number of keys doesn't match with index");
        }
        reset();
        pos = Position.Empty;
        if (types[0] == Database.MCO_DB_FT_CHARS || types[0] == Database.MCO_DB_FT_STRING) { 
            String encoding = indexDesc.keys[0].field.encoding;
            byte[] bytes;
            try { 
                bytes = value.getBytes(encoding);
            } catch (UnsupportedEncodingException x) { 
                throw new RuntimeException(x);
            }
            currHandle = findBytes(con.id, indexNo, types[0], bytes); 
        } else { 
            currHandle = findStr(con.id, indexNo, types[0], value); 
        }
        if (currHandle != 0) { 
            currObj = (T)con.db.loadObject(currHandle, classDesc);
            return currObj;
        }
        return null;
    }
        
    /**
     * Check if there is a previous result.
     */
    public boolean hasPrev() {
        switch (pos) { 
        case Empty:
            return false;
        case Prev:
        case First:
            return true;
        case Unknown:
            if (moveLast(con.id, cursor)) { 
                pos = Position.First;
                return true;
            }
            pos = Position.Empty;
            return false;
        case Next:
            if (!movePrev(con.id, cursor)) {
                return false;
            }
            pos = Position.Curr;
            // no break            
        case Curr:
            if (movePrev(con.id, cursor)) { 
                pos = Position.Prev;
                return true;
            }
        }
        return false; 
    }

    /**
     * Check if there is a next result.
     */
    public boolean hasNext() {
        switch (pos) { 
        case Empty:
            return false;
        case Next:
        case First:
            return true;
        case Unknown:
            if (moveFirst(con.id, cursor)) { 
                pos = Position.First;
                return true;
            }
            pos = Position.Empty;
            return false;
        case Prev:
            if (!moveNext(con.id, cursor)) {
                return false;
            }
            pos = Position.Curr;
            // no break            
        case Curr:
            if (moveNext(con.id, cursor)) { 
                pos = Position.Next;
                return true;
            }
        }
        return false; 
    }

    /**
     * Move cursor position forward without fetching current object.
     * This method allows traversal of cursor without fetching objects.
     * Desired object can be explicitly fetched using getCurrent() method.
     * @return true if current element is not the last one, false otherwise 
     */
    public boolean moveNext() 
    { 
        dir = Direction.Forward;
        if (!hasNext()) { 
            return false;
        }
        pos = Position.Curr;
        return true;
    }

    /**
     * Move cursor position backward without fetching current object 
     * This method allows traversal of cursor without fetching objects.
     * Desired object can be explicitly fetched using getCurrent() method.
     * @return true if current element is not the first one, false otherwise 
     */
    public boolean movePrev() 
    { 
        dir = Direction.Backward;
        if (!hasPrev()) { 
            return false;
        }
        pos = Position.Curr;
        return true;
    }    

    /**
     * Skip the specified number of results.
     * @param offset if poisitive move forward, if negative move backward. (Zero value is not accepted.)
     * @return the object at the specified position in the cursor or null if this position can not be reached.
     */
    public T skip(int offset) { 
        reset();
        if (offset > 0) { 
            while (hasNext()) {
                pos = Position.Curr;
                if (--offset == 0) { 
                    return getCurrent();
                }
            }
        } else if (offset < 0) { 
            while (hasPrev()) {
                pos = Position.Curr;
                if (++offset == 0) { 
                    return getCurrent();
                }
            }
        } else { 
            throw new IllegalStateException("Skip argument should not be 0");            
        }
        pos = Position.Empty;
        return null;
    }             

    /**
     * Get current object.
     * This method should be used only with moveNext()/movePrev() method as alternative to next()/prev()/
     * @return current object
     */
    public T getCurrent() { 
        pos = Position.Curr;
        currHandle = getCurrentHandle(con.id, cursor, classCode);
        currObj = (T)con.db.loadObject(currHandle, classDesc);
        return currObj;        
    }
        
    /**
     * Move forward. 
     * @return the next object in the result set or null if there are no more objects.
     */
    public T next() {
        dir = Direction.Forward;
        reset();
        if (!hasNext()) { 
            return null;
        }
        return getCurrent();
    }

    /**
     * Move backward. 
     * @return the previous object in the result set or null if there are no more objects.
     */
    public T prev() {
        dir = Direction.Backward;
        reset();
        if (!hasPrev()) { 
            return null;
        }
        return getCurrent();
    }
        
    /**
     * Get the first object in the result set.
     * @return the first object in the result set or null if the result set is empty.
     */
    public T first() {
        dir = Direction.Forward;
        reset();
        if (moveFirst(con.id, cursor)) {            
            return getCurrent();
        } else { 
            pos = Position.Empty;
            return null;
        }
    }

    /**
     * Get the last object in the result set.
     * @return the last object in the result set or null if the result set is empty.
     */
    public T last() {
        dir = Direction.Backward;
        reset();
        if (moveLast(con.id, cursor)) {            
            return getCurrent();
        } else { 
            pos = Position.Empty;
            return null;
        }
    }

    private void moveFromCurrentPosition() {
        if (pos != Position.Empty && pos != Position.Next && pos != Position.Prev) { 
            pos = dir == Direction.Backward
                ? movePrev(con.id, cursor) ? Position.Prev : Position.Empty
                : moveNext(con.id, cursor) ? Position.Next : Position.Empty;
        }
    }

    /**
     * Remove the current object. 
	 * Remove the object previously returned by methods first(), last(), next(), prev() or skip().
     */
    public void remove() { 
        if (currHandle == 0) { 
            throw new IllegalStateException("No current object");
        }
        moveFromCurrentPosition();
        removeObject(currHandle);
        currHandle = 0;
        currObj = null;
    } 

    /**
     * Update the current object. 
	 * Store the object previously returned by methods first(), last(), next(), prev() or skip().
     */
    public void update() {
        if (currHandle == 0) { 
            throw new IllegalStateException("No current object");
        }
        moveFromCurrentPosition();
        con.db.updateObject(currHandle, classDesc, currObj);
    }

    /**
     * Checkpoint the current object. 
	 * Insert indexes for the updated object previously returned by methods first(), last(), next(), prev() or skip().
     * @return false if there is an MVCC conflict.
     */
    public boolean checkpoint() { 
        if (currHandle == 0) { 
            throw new IllegalStateException("No current object");
        }
        return checkpointObject(currHandle);
    }

    /**
     * Get the AUTOID of the current object (if it has one).
     * @return the AUTOID of the object previously returned by methods first(), last(), next(), prev() or skip().
     */
    public long getAutoId() {
        if (currHandle == 0) { 
            throw new IllegalStateException("No current object");
        }
        if (classDesc.autoIdIndexNo < 0) { 
            throw new IllegalStateException("Class has no AUTOID");
        }           
        return con.db.getAutoId(currHandle);
    }

    private void reset() { 
        if (currHandle != 0) { 
            Database.freeHandle(currHandle);
            currHandle = 0;
            currObj = null;
        }
    }

    /**
     * Close the cursor.
     */
    public void close() {
        reset();
        pos = Position.Unknown;
        if (cursor != 0) { 
            if (con.id != 0) { 
                closeCursor(con.id, cursor);
            }
            cursor = 0;
        }
    }

    protected void finalize() { 
        close();
    }

    /**
     * Get the iterator.
     */
    public Iterator<T> iterator() { 
        return this;
    }

    private enum Position {
        Unknown,
        Empty,
        First,
        Curr,
        Next,
        Prev
    };

    private enum Direction {
        Unknown, 
        Forward, 
        Backward
    };
 
    private Position pos;
    private Direction dir;
    private long     currHandle;
    private int      classCode;
    private Database.ClassDescriptor classDesc;
    private int      indexNo;
    private int[]    types;
    private boolean  unique;
    private boolean  hash;    
    private Connection con;
    private long     cursor;
    private T        currObj;
    private Database.IndexDescriptor indexDesc;

    private native boolean moveFirst(long trans, long cursor);
    private native boolean moveLast(long trans, long cursor);
    private native boolean moveNext(long trans, long cursor);
    private native boolean movePrev(long trans, long cursor);
    private native long getCurrentHandle(long trans, long cursor, int classCode);

    private native long openIndexCursor(long con, int indexNo);
    private native long openListCursor(long con, int classCode);
    private native void closeCursor(long trans, long cursor);

    private native boolean checkpointObject(long obj);
    private native void removeObject(long obj);

    private native boolean search(long con, int indexNo, long cursor, int op, int[] types, boolean hash, Object[] values); 
    private native long find(long con, int indexNo, int[] types, Object[] values);        
    private native boolean searchInt(long con, int indexNo, long cursor, int op, int type, boolean hash, long value); 
    private native boolean searchStr(long con, int indexNo, long cursor, int op, int type, boolean hash, String value); 
    private native boolean searchReal(long con, int indexNo, long cursor, int op, int type, boolean hash, double value); 
    private native boolean searchBytes(long con, int indexNo, long cursor, int op, int type, boolean hash, byte[] value); 
    private native long findInt(long con, int indexNo, int type, long value);        
    private native long findStr(long con, int indexNo, int type, String value);        
    private native long findBytes(long con, int indexNo, int type, byte[] value);        
}


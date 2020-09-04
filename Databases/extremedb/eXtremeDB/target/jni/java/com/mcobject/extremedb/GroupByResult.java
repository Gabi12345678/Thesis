package com.mcobject.extremedb;

/**
 * Class reprenting result of group by aggregation operation.
 */
public class GroupByResult implements java.io.Closeable
{
    /**
     * Iterator throw sequence of calculated aggregates for each group
     */
    public SequenceIterator getAggIterator() { 
        return aggIterator;
    }

    /**
     * Iterator throw sequence of group values
     */
    public SequenceIterator getGroupIterator() { 
        return grpIterator;
    }

    /**
     * Release memory used by hash table
     */
    public void close() { 
        if (grpIterator != null) { 
            close(grpIterator.handle);
            grpIterator = null;
            aggIterator = null;
        }
    }

    protected void finalize() { 
        close();
    }
   

    GroupByResult(long[] handles) { 
        aggIterator = new SequenceIterator(handles[0]);        
        grpIterator = new SequenceIterator(handles[1]);
    }    

    private SequenceIterator aggIterator;
    private SequenceIterator grpIterator;
    
    private native void close(long handle);
}
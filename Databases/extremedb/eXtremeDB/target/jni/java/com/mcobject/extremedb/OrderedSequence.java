package com.mcobject.extremedb;

/**
 * Oreded sequence. Elements of this sequnce can be accessed either by position, either by key range.
 * Ussually one or more unordered sequences are associated with ordered sequence,specifuing timestamp.
 * So classical time series can be respresented by ordered sequence with timestamp and unordered sequences with other timeseries 
 * attributes.
 */
public class OrderedSequence extends UnorderedSequence
{
    public enum Boundary { 
        Open,      /* value for this boundary is not specified */
        Inclusive,   
        Exclusive
    };

    /**
     * Perform range search
     * @param low low value (is skipped if lowBoundary=Boundary.Open)
     * @param lowBoundary kind of low boundary
     * @param high high value (is skipped if highBoundary=Boundary.Open)
     * @param highBoundary kind of high boundary
     */
    public SequenceIterator search(long low, Boundary lowBoundary, long high, Boundary highBoundary) { 
        return new SequenceIterator(intSearch(handle, elemType, offset, compactOffset, order, low, lowBoundary.ordinal(), high, highBoundary.ordinal()));
    }
    /**
     * Perform range search
     * @param low low value (is skipped if lowBoundary=Boundary.Open)
     * @param lowBoundary kind of low boundary
     * @param high high value (is skipped if highBoundary=Boundary.Open)
     * @param highBoundary kind of high boundary
     */
    public SequenceIterator search(double low, Boundary lowBoundary, double high, Boundary highBoundary) { 
        return new SequenceIterator(realSearch(handle, elemType, offset, compactOffset, order, low, lowBoundary.ordinal(), high, highBoundary.ordinal()));
    }
    
    OrderedSequence(long hnd, int type, int offsetU, int offsetC, int fieldNo, int order, int elemSize) { 
        super(hnd, type, offsetU, offsetC, fieldNo, order, elemSize);
    }

    private native long intSearch(long hnd, int type, int offsetU, int offsetC, int order, long low, int lowBoundary, long high, int highBoundary);
    private native long realSearch(long hnd, int type, int offsetU, int offsetC, int order, double low, int lowBoundary, double high, int highBoundary);
}
 
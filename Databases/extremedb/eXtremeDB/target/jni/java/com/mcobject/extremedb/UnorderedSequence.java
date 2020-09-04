package com.mcobject.extremedb;

import java.lang.reflect.Array;

/**
 * Unordered sequence. Elements of this sequnce can be accessed only by position.
 * Usually one or more unordered sequences are associated with ordered sequence,specifuing timestamp.
 * So classical time series can be respresented by ordered sequence with timestamp and unordered sequences with other timeseries 
 * attributes.
 */
public class UnorderedSequence
{
    /**
     * Size of tile
     */
    public static final int TILE_SIZE = 100;

    /**
     * Project specified sequence on this one. Usually other projected sequence represents result of timestamp search (locating
     * some interval). Project allows to select this interval from this sequence.
     * @param seq projected sequence
     * @return iterator through result sequence
     */
    public SequenceIterator project(SequenceIterator seq) { 
        return new SequenceIterator(project(handle, elemType, offset, compactOffset, seq.handle, elemSize));
    }

    /**
     * Append items to the sequence.
     * @param items array with items to be added. Type of the array element should match sequence type.
     */
    public void append(Object items) {
        append(handle, elemType, offset, compactOffset, fieldNo, order, items, Array.getLength(items), elemSize);
    }
    
    /**
     * Append items to the sequence.
     * @param items array with items to be added. Type of the array element should match sequence type.
     * @param nItems number of items to append
     */
    public void append(Object items, int nItems) {
        append(handle, elemType, offset, compactOffset, fieldNo, order, items, nItems, elemSize);
    }
    
    /**
     * Insert items to the specified position in the sequence.
     * @param position insert position
     * @param items array with items to be inserted. Type of the array element should match sequence type.
     */
    public void insert(long position, Object items) {
        insert(handle, elemType, offset, compactOffset, fieldNo, order, position, items, Array.getLength(items), elemSize);
    }
    
    /**
     * Insert items to the specified position in the sequence.
     * @param position insert position
     * @param items array with items to be inserted. Type of the array element should match sequence type.
     * @param nItems number of inserted items
    */
    public void insert(long position, Object items, int nItems) {
        insert(handle, elemType, offset, compactOffset, fieldNo, order, position, items, nItems, elemSize);
    }

    /**
     * Delete specified range of items from the sequence
     * @param from first element to be delete (inclusive)
     * @param till last element to be deleted (inclusive)
     */
    public void delete(long from, long till) { 
        delete(handle, elemType, offset, compactOffset, fieldNo, order, from, till, elemSize);
    }
    
    /**
     * Get numer of elements in the sequence
     * @return number of sequence elements
     */
    public long count() {
        return count(handle, offset, compactOffset);
    }

    /** 
     * Get sequence iterator
     * @return sequence iterator
     */
    public SequenceIterator iterator() {
        return subseq(0, -1);
    }

    /**
     * Get subsequence
     * @param from position of first element (inclusive)
     * @param till position of last element (inclusive)
     * @return iterator through result sequence
     */
    public SequenceIterator subseq(long from, long till) { 
        return new SequenceIterator(subseq(handle, elemType, offset, compactOffset, from, till, elemSize));
    }

    /**
     * Extract from second sequence elements with positions specified in the first sequence.
     * Positions are intended to be obtained using SequenceIterator.filterPos or SequenceIterator.topPos methods.
     * Unlike SequenceIterator.map method, this method is able to effciently skip elements.
     * @param positions iterator through sequence with positions of elements
     * @return iterator through result sequence
     */
    public SequenceIterator map(SequenceIterator positions) { 
        return new SequenceIterator(map(handle, elemType, offset, compactOffset, positions.handle, elemSize));
    }

    /**
     * Store items returned by sequence iterator in this sequence
     * @param iter iterator through sourece sequence
     */
    public void store(SequenceIterator iter) { 
        store(handle, elemType, offset, compactOffset, fieldNo, order, iter.handle, elemSize);
    }

    UnorderedSequence(long hnd, int seqType, int offsetU, int offsetC, int no, int ord, int size) { 
        handle = hnd;
        elemType = seqType - Database.MCO_DB_FT_SEQUENCE_UINT1; // map MCO_DB_FT_SEQUENCE_UINT1... -> MCO_DD_UINT1...
        if (seqType == Database.MCO_DB_FT_SEQUENCE_CHAR) {
            elemType += 3; // skip MCO_DD_DATE, MCO_DD_TIME MCO_DD_DATETIME
        } else if (seqType == Database.MCO_DB_FT_SEQUENCE_DATETIME) {
        	elemType = 12; // MCO_DD_DATETIME 
        }
        elemSize = size;
        offset = offsetU;
        compactOffset = offsetC;
        fieldNo = no;
        order = ord;
    }

    private native long project(long hnd, int type, int offsetU, int offsetC, long ts, int elemSize);
    private native void append(long hnd, int type, int offsetU, int offsetC, int fieldNo, int order, Object items, int nItems, int elemSize);
    private native void insert(long hnd, int type, int offsetU, int offsetC, int fieldNo, int order, long position, Object items, int nItems, int elemSize);
    private native void delete(long hnd, int type, int offsetU, int offsetC, int fieldNo, int order, long from, long till, int elemSize);
    private native long count(long hnd, int offsetU, int offsetC);
    private native long map(long hnd, int type, int offsetU, int offsetC, long positions, int elemSize);
    private native long subseq(long hnd, int type, int offsetU, int offsetC, long from, long till, int elemSize);
    private native void store(long hnd, int type, int offsetU, int offsetC, int fieldNo, int order, long src, int elemSize);

    long handle;
    int  offset;
    int  compactOffset;
    int  fieldNo;
    int  order;
    int  elemType;
    int  elemSize;
}
    
    
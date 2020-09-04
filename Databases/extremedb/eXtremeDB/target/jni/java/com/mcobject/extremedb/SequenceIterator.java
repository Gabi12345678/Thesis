package com.mcobject.extremedb;
import java.util.NoSuchElementException;
import java.lang.reflect.Array;
import java.util.Date;

/**
 * Sequence  iterator.
 * Iteration is performed in tile mode: more than one sequence items are fetched at each iteration.
 * All these items (tile) can be obtained using tileItems() method.
 * The main iterator method is nextTile which position iterator on next tile. Size of tile is implementation specific,
 * right now it is 100 items. Certainly at the end of sequence, tile may be shorter.
 * Alternative way of performing iteration is use of get() method. This method extract items into provided array.
 * If you prefer to iterate in scalar mode (item-by-item), then pass array with size 1 to get() method.
 */
public class SequenceIterator implements java.io.Closeable
{
    /**
     * Get items of the current tile
     * @return array with tile items
     */
    public Object tileItems() {
        return tileItems(handle);
    }

    /**
     * Get RLE count for the tile item.
     * For non-RLE implementation of sequences this method always returns 1.
     */
    public int itemRepeatCount(int i) {
        return itemRepeatCount(handle, i);
    }

    /**
     * Decode RLE sequence to non-RLE sequence. For non-RLE implementation this method just copy sequence iterator
     */
    public SequenceIterator rleDecode() {
        return new SequenceIterator(rleDecode(handle));
    }

    /**
     * Check if underlying sequence implementation uses RLE format (in this case repeat counter is associated with each sequence element)
     */
    static native public boolean isRLE();

    /**
     * Get position of first sequence element
     * @return position of element at which this sequence starts (inclusive)
     */
    public long firstPosition() {
        return firstPosition(handle);
    }
    /**
     * Get position of last sequence element
     * @return position of element at which this sequence ends (inclusive)
     */
    public long lastPosition() {
        return lastPosition(handle);
    }
    /**
     * Position of element returned by subsequent invocation of next or nextTile method
     */
    public long nextPosition() {
        return nextPosition(handle);
    }
    /**
     * Advance iterator to next tile
     * @return true if iterator has successfully advanced, false otherwise
     */
    public boolean nextTile() {
        return nextTile(handle);
    }
    /**
     * Type of this sequence
     * @return type of the target sequence
     */
    public Sequence.Type type() {
        int t = type(handle);
        if (t == 40) t = 10;
        else if (t == 12) t = 11;
        return Sequence.Type.class.getEnumConstants()[t];
    }

    /**
     * Size fo element of this sequence
     * @return sequence element size
     */
    public int elemSize() {
        return elemSize(handle);
    }

    /**
     * Extract from second sequence elements with positions specified in the first sequence.
     * Positions should be provided in ascending order (them are intended to be obtained using  SequenceIterator.filterPos or SequenceIterator.topPos methods).
     * @param positions iterator through sequence with positions of elements
     * @return iterator through result sequence
     */
    public SequenceIterator map(SequenceIterator positions) {
        return new SequenceIterator(map(handle, positions.handle));
    }

    /**
     * Fork sequence iterator: create two instances of iterators for the same sequence. These iterators should be traverersed together (used in the same expression)
     * @return array with pair of forked iterator
     */
    public SequenceIterator[] tee() {
        long[] pair = new long[2];
        tee(handle, pair);
        return new SequenceIterator[]{new SequenceIterator(pair[0]), new SequenceIterator(pair[1])};
    }

    /**
     * Add correspondent elements of two sequences
     * @return result sequence iterator
     */
    public SequenceIterator add(SequenceIterator other) {
        return new SequenceIterator(add(handle, other.handle));
    }
    /**
     * Subtract correspondent elements of two sequences
     * @return result sequence iterator
     */
    public SequenceIterator sub(SequenceIterator other) {
        return new SequenceIterator(sub(handle, other.handle));
    }
    /**
     * Divide correspondent elements of two sequneces
     * @return result sequence iterator
     */
    public SequenceIterator div(SequenceIterator other) {
        return new SequenceIterator(div(handle, other.handle));
    }

    /**
     * Multiply correspondent elements of two sequneces
     * @return result sequence iterator
     */
    public SequenceIterator mul(SequenceIterator other) {
        return new SequenceIterator(mul(handle, other.handle));
    }
    /**
     * Divide by modulo correspondent elements of two sequneces
     * @return result sequence iterator
     */
    public SequenceIterator mod(SequenceIterator other) {
        return new SequenceIterator(mod(handle, other.handle));
    }

    /**
     * Check if element of first sequence is equal to correpondent element of second sequenc
     * @return result sequence iterator
     */
    public SequenceIterator eq(SequenceIterator other) {
        return new SequenceIterator(eq(handle, other.handle));
    }
    /**
     * Check if element of first sequence is not equal to correpondent element of second sequence
     * @return result sequence iterator
     */
    public SequenceIterator ne(SequenceIterator other) {
        return new SequenceIterator(ne(handle, other.handle));
    }
    /**
     * Check if element of first sequence is less than to correpondent element of second sequence
     * @return result sequence iterator
     */
    public SequenceIterator lt(SequenceIterator other) {
        return new SequenceIterator(lt(handle, other.handle));
    }
    /**
     * Check if element of first sequence is greater than correpondent element of second sequence
     * @return result sequence iterator
     */
    public SequenceIterator gt(SequenceIterator other) {
        return new SequenceIterator(gt(handle, other.handle));
    }
    /**
     * Check if element of first sequence is less or equal than correpondent element of second sequence
     * @return result sequence iterator
     */
    public SequenceIterator le(SequenceIterator other) {
        return new SequenceIterator(le(handle, other.handle));
    }
    /**
     * Check if element of first sequence is greater or equal than correpondent element of second sequence
     * @return result sequence iterator
     */
    public SequenceIterator ge(SequenceIterator other) {
        return new SequenceIterator(ge(handle, other.handle));
    }

    /**
     * Maximum of two correspondent elements of two sequneces
     * @return result sequence iterator
     */
    public SequenceIterator max(SequenceIterator other) {
        return new SequenceIterator(max(handle, other.handle));
    }
    /**
     * Minimum of two correspondent elements of two sequneces
     * @return result sequence iterator
     */
    public SequenceIterator min(SequenceIterator other) {
        return new SequenceIterator(min(handle, other.handle));
    }

    /**
     * Logical AND of elements of two boolean sequences
     * @return result sequence iterator
     */
    public SequenceIterator and(SequenceIterator other) {
        return new SequenceIterator(and(handle, other.handle));
    }
    /**
     * Logical OR of elements of two boolean sequences
     * @return result sequence iterator
     */
    public SequenceIterator or(SequenceIterator other) {
        return new SequenceIterator(or(handle, other.handle));
    }
    /**
     * Logical XOR of elements of two boolean sequences
     * @return result sequence iterator
     */
    public SequenceIterator xor(SequenceIterator other) {
        return new SequenceIterator(xor(handle, other.handle));
    }

    /**
     * Negate values of sequence elements
     * @return result sequence iterator
     */
    public SequenceIterator neg() {
        return new SequenceIterator(neg(handle));
    }
    /**
     * Logical NOT of elements of boolean sequence
     * @return result sequence iterator
     */
    public SequenceIterator not() {
        return new SequenceIterator(not(handle));
    }

    /**
     * Absolute value of sequence elements
     * @return result sequence iterator
     */
    public SequenceIterator abs() {
        return new SequenceIterator(abs(handle));
    }

    /**
     * Match sequence of string with pattern
     * @param pattern SQL compatible pattern (LIKE clause)
     * @return result sequence iterator
     */
    public SequenceIterator match(String pattern) {
        return new SequenceIterator(match(handle, pattern));
    }


    /**
     * Construct sequence from string literal, i.e. "{1.0, -1.1, 0}"
     * @param val string literal
     * @param type sequence type
     * @return result sequence iterator
     */
    public static SequenceIterator parse(String val, Sequence.Type type) {
        return new SequenceIterator(parseString(val, type.ordinal()));
    }

    /**
     * Construct sequence of double constants
     * @param val constant value
     * @param type sequence type
     * @return result sequence iterator
     */
    public static SequenceIterator constant(double val, Sequence.Type type) {
        return new SequenceIterator(realConstant(val, type.ordinal()));
    }

    /**
     * Construct sequence of integer constants
     * @param val constant value
     * @param type sequence type
     * @return result sequence iterator
     */
    public static SequenceIterator constant(long val, Sequence.Type type) {
        return new SequenceIterator(intConstant(val, type.ordinal()));
    }

    /**
     * Construct sequence of char constants
     * @param val constant value
     * @param type sequence type
     * @return result sequence iterator
     */
    public static SequenceIterator constant(byte[] val) {
        return new SequenceIterator(charConstant(val));
    }

    /**
     * Extract sequence elements in the provided buffer (array)
     * @param buffer array in which sequence elemets will be extracted. This array hould have the same element type as sequence element type.
     * @return actual number of elements extracted (can be smaler than specified array size in case of reaching end of array)
     */
    public int get(Object buffer) {
        return get(handle, buffer);
    }

    /**
     * Get next element of sequence
     * @return object wrapper for next sequence element or null in case of end of sequence
     */
    public Object next()
    {
        switch (type()) {
        case UInt1:
        case Int1:
        {
            byte[] buffer = new byte[1];
            return get(handle, buffer) == 0 ? null : new Byte(buffer[0]);
        }
        case UInt2:
        {
            char[] buffer = new char[1];
            return get(handle, buffer) == 0 ? null : new Character(buffer[0]);
        }
        case Int4:
        case UInt4:
        {
            int[] buffer = new int[1];
            return get(handle, buffer) == 0 ? null : new Integer(buffer[0]);
        }
        case UInt8:
        case Int8:
        {
            long[] buffer = new long[1];
            return get(handle, buffer) == 0 ? null : new Long(buffer[0]);
        }
        case Int2:
        {
            short[] buffer = new short[1];
            return get(handle, buffer) == 0 ? null : new Short(buffer[0]);
        }
        case Float:
        {
            float[] buffer = new float[1];
            return get(handle, buffer) == 0 ? null : new Float(buffer[0]);
        }
        case Double:
        {
            double[] buffer = new double[1];
            return get(handle, buffer) == 0 ? null : new Double(buffer[0]);
        }
        case Char:
        {
            byte[] buffer = new byte[elemSize()];
            return get(handle, buffer) == 0 ? null : buffer;
        }
        case DateTime:
        {
        	Date[] buffer = new Date[1];
        	return get(handle, buffer) == 0 ? null : buffer[0];
        }
        }
        return null;
    }

    /**
     * Get next int element of the sequence
     * @return next sequence element value
     * @throws NoSuchElementException if the iteration has no more elements
     */
    public int nextInt() {
        int[] buffer = new int[1];
        if (get(handle, buffer) == 0) {
            throw new NoSuchElementException();
        }
        return buffer[0];
    }

    /**
     * Get next long element of the sequence
     * @return next sequence element value
     * @throws NoSuchElementException if the iteration has no more elements
     */
    public long nextLong() {
        long[] buffer = new long[1];
        if (get(handle, buffer) == 0) {
            throw new NoSuchElementException();
        }
        return buffer[0];
    }

    /**
     * Get next float element of the sequence
     * @return next sequence element value
     * @throws NoSuchElementException if the iteration has no more elements
     */
    public float nextFloat() {
        float[] buffer = new float[1];
        if (get(handle, buffer) == 0) {
            throw new NoSuchElementException();
        }
        return buffer[0];
    }

    /**
     * Get next double element of the sequence
     * @return next sequence element value
     * @throws NoSuchElementException if the iteration has no more elements
     */
    public double nextDouble() {
        double[] buffer = new double[1];
        if (get(handle, buffer) == 0) {
            throw new NoSuchElementException();
        }
        return buffer[0];
    }

    /**
     * Get next element of the sequence of char
     * @return next sequence element value
     * @throws NoSuchElementException if the iteration has no more elements
     */
    public byte[] nextBytes() {
        byte[] buffer = new byte[elemSize()];
        if (get(handle, buffer) == 0) {
            throw new NoSuchElementException();
        }
        return buffer;
    }

    /**
     * Get next element of the sequence of char
     * @return next sequence element value
     * @throws NoSuchElementException if the iteration has no more elements
     */
    public String nextString() {
        byte[] buffer = new byte[elemSize()];
        if (get(handle, buffer) == 0) {
            throw new NoSuchElementException();
        }
        return (new String(buffer)).trim();
    }

    /**
     * Get next element of the sequence of Date
     * @return next sequence element value
     * @throws NoSuchElementException if the iteration has no more elements
     */
    public Date nextDate() {
        Date[] buffer = new Date[1];
        if (get(handle, buffer) == 0) {
            throw new NoSuchElementException();
        }
        return buffer[0];
    }


    /**
     * Concatenate two sequences: concat({1,2,3}, {4,5}) = {1,2,3,4,5}
     * @return result sequence iterator
     */
    public SequenceIterator concat(SequenceIterator tail) {
        return new SequenceIterator(concat(handle, tail.handle));
    }

    /**
     * Concatenate elements of two sequences: cat({'a','b','c'}, {'x','y','z'}) = {'ax', 'by', 'cz'}
     * @return result sequence iterator
     */
    public SequenceIterator cat(SequenceIterator other) {
        return new SequenceIterator(cat(handle, other.handle));
    }

    /**
     * Weighted sum of two sequences
     * @return result sequence iterator
     */
    public SequenceIterator wsum(SequenceIterator weights) {
        return new SequenceIterator(wsum(handle, weights.handle));
    }
    /**
     * Weighted average of two sequences
     * @return result sequence iterator
     */
    public SequenceIterator wavg(SequenceIterator weights) {
        return new SequenceIterator(wavg(handle, weights.handle));
    }
    /**
     * Covariance of two sequences
     * @return result sequence iterator
     */
    public SequenceIterator cov(SequenceIterator other) {
        return new SequenceIterator(cov(handle, other.handle));
    }
    /**
     * Correlation of two sequences
     * @return result sequence iterator
     */
    public SequenceIterator corr(SequenceIterator other) {
        return new SequenceIterator(corr(handle, other.handle));
    }

    /**
     * Choose element of second or third sequence depending on boolean value of element of first sequence.
     * If lazy param is false, then all three sequences are traversed on the same speed (if element of "then" sequence is used, then element of "else" sequence is skipped and visa versa).
     * If lazy param is true, then position in not used sequence is not changed
     * @return result sequence iterator
     */
    public SequenceIterator iif(SequenceIterator then, SequenceIterator otherwise, boolean lazy) {
        return new SequenceIterator(iif(handle, then.handle, otherwise.handle, lazy));
    }
    /**
     * Leaves only thoses elements of "values" sequence for which boolean elements of this sequence are true
     * @return result sequence iterator
     */
    public SequenceIterator filter(SequenceIterator cond) {
        return new SequenceIterator(filter(cond.handle, handle));
    }
    /**
     * Get positions of true elements of input sequence
     * @return result sequence iterator
     */
    public SequenceIterator filterPos() {
        return new SequenceIterator(filterPos(handle));
    }
    /**
     * Remove duplicates from sorted sequence
     * @return result sequence iterator
     */
    public SequenceIterator unique() {
        return new SequenceIterator(unique(handle));
    }
    /**
     * Normalize sequence: divide each sequence element by square root of sum of squares of all elements
     * @return result sequence iterator
     */
    public SequenceIterator normalize() {
        return new SequenceIterator(normalize(handle));
    }
    /**
     * Select elements of input sequence with specified origin and step
     * @param origin position of first selected element
     * @param step interval between all subsequent elements
     * @return result sequence iterator
     */
    public SequenceIterator thin(long origin, long step) {
        return new SequenceIterator(thin(handle, origin, step));
    }
    /**
     * Get iterator through subsequence [from,till] of input sequence
     * @param from position of first selected element (inclusive)
     * @param till position of last selected element (inclusive)
     * @return result sequence iterator
     */
    public SequenceIterator limit(long from, long till) {
        return new SequenceIterator(limit(handle, from, till));
    }
    /**
     * Reverse elements of this sequences
     * @return result sequence iterator
     */
    public SequenceIterator reverse() {
        return new SequenceIterator(reverse(handle));
    }
    /**
     * Differences between pairs of sequence elements
     * @return result sequence iterator
     */
    public SequenceIterator diff() {
        return new SequenceIterator(diff(handle));
    }
    /**
     * Trend of sequence elements
     * @return result sequence iterator
     */
    public SequenceIterator trend() {
        return new SequenceIterator(trend(handle));
    }

    /**
     * Count sequence elements
     * @return result sequence iterator
     */
    public SequenceIterator aggCount() {
        return new SequenceIterator(aggCount(handle));
    }

    /**
     * Maximum value of sequence elements
     * @return result sequence iterator
     */
    public SequenceIterator aggMax() {
        return new SequenceIterator(aggMax(handle));
    }
    /**
     * Minimum value of sequence elements
     * @return result sequence iterator
     */
    public SequenceIterator aggMin() {
        return new SequenceIterator(aggMin(handle));
    }
    /**
     * Sum of all sequence elements
     * @return result sequence iterator
     */
    public SequenceIterator aggSum() {
        return new SequenceIterator(aggSum(handle));
    }
    /**
     * Product of all sequence elements
     * @return result sequence iterator
     */
    public SequenceIterator aggPrd() {
        return new SequenceIterator(aggPrd(handle));
    }
    /**
     * Average value of sequence elements
     * @return result sequence iterator
     */
    public SequenceIterator aggAvg() {
        return new SequenceIterator(aggAvg(handle));
    }
    /**
     * Population variance of sequence
     * @return result sequence iterator
     */
    public SequenceIterator aggVar() {
        return new SequenceIterator(aggVar(handle));
    }
    /**
     * Population standard deviation of sequence
     * @return result sequence iterator
     */
    public SequenceIterator aggDev() {
        return new SequenceIterator(aggDev(handle));
    }
    /**
     * Sample variance of sequence
     * @return result sequence iterator
     */
    public SequenceIterator aggVarSamp() {
        return new SequenceIterator(aggVarSamp(handle));
    }
    /**
     * Sample standard deviation of sequence
     * @return result sequence iterator
     */
    public SequenceIterator aggDevSamp() {
        return new SequenceIterator(aggDevSamp(handle));
    }
    /**
     * Aproximate distinct values count
     * @return result sequence iterator
     */
    public SequenceIterator aggApproxDC() {
        return new SequenceIterator(aggApproxDC(handle));
    }
    /**
     * Aproximate distinct hash codes count. This method can be used to estimate number of disctinct pairs, triples,...
     * SequenceIterator.hash() method should be used to calculate combined hash code for all affected sequences.
     * @return result sequence iterator
     */
    public SequenceIterator aggApproxHashDC() {
        return new SequenceIterator(aggApproxHashDC(handle));
    }
    /**
     * Calculate both minimum and maximal value of sequence elements. Result sequence contains two elements: first is minimum and second - maximum.
     * @return result sequence iterator
     */
    public SequenceIterator aggMinMax() {
        return new SequenceIterator(aggMinMax(handle));
    }

    /**
     * Weighted average of each group of elements of sequence
     * @param weight iterator through weight sequence
     * @param groupBy iterator through grouping sequence
     * @return result sequence iterator
     */
    public SequenceIterator groupAggWavg(SequenceIterator weight, SequenceIterator groupBy) {
        return new SequenceIterator(groupAggWavg(weight.handle, handle, groupBy.handle));
    }

    /**
     * Maximum of each group of elements of sequence
     * @param groupBy iterator through grouping sequence
     * @return result sequence iterator
     */
    public SequenceIterator groupAggMax(SequenceIterator groupBy) {
        return new SequenceIterator(groupAggMax(handle, groupBy.handle));
    }
    /**
     * Minimum of each group of elements of sequence
     * @param groupBy iterator through grouping sequence
     * @return result sequence iterator
     */
    public SequenceIterator groupAggMin(SequenceIterator groupBy) {
        return new SequenceIterator(groupAggMin(handle, groupBy.handle));
    }
    /**
     * Sum of each group of elements of sequence
     * @param groupBy iterator through grouping sequence
     * @return result sequence iterator
     */
    public SequenceIterator groupAggSum(SequenceIterator groupBy) {
        return new SequenceIterator(groupAggSum(handle, groupBy.handle));
    }
    /**
     * Average of each group of elements of sequence
     * @param groupBy iterator through grouping sequence
     * @return result sequence iterator
     */
    public SequenceIterator groupAggAvg(SequenceIterator groupBy) {
        return new SequenceIterator(groupAggAvg(handle, groupBy.handle));
    }
    /**
     * Population variance of each group of elements of sequence
     * @param groupBy iterator through grouping sequence
     * @return result sequence iterator
     */
    public SequenceIterator groupAggVar(SequenceIterator groupBy) {
        return new SequenceIterator(groupAggVar(handle, groupBy.handle));
    }
    /**
     * Population deviation of each group of elements of sequence
     * @param groupBy iterator through grouping sequence
     * @return result sequence iterator
     */
    public SequenceIterator groupAggDev(SequenceIterator groupBy) {
        return new SequenceIterator(groupAggDev(handle, groupBy.handle));
    }
    /**
     * Sample variance of each group of elements of sequence
     * @param groupBy iterator through grouping sequence
     * @return result sequence iterator
     */
    public SequenceIterator groupAggVarSamp(SequenceIterator groupBy) {
        return new SequenceIterator(groupAggVarSamp(handle, groupBy.handle));
    }
    /**
     * Sample standard deviation of each group of elements of sequence
     * @param groupBy iterator through grouping sequence
     * @return result sequence iterator
     */
    public SequenceIterator groupAggDevSamp(SequenceIterator groupBy) {
        return new SequenceIterator(groupAggDevSamp(handle, groupBy.handle));
    }
    /**
     * First element of the group
     * @param groupBy iterator through grouping sequence
     * @return result sequence iterator
     */
    public SequenceIterator groupAggFirst(SequenceIterator groupBy) {
        return new SequenceIterator(groupAggFirst(handle, groupBy.handle));
    }
    /**
     * Last element of the group
     * @param groupBy iterator through grouping sequence
     * @return result sequence iterator
     */
    public SequenceIterator groupAggLast(SequenceIterator groupBy) {
        return new SequenceIterator(groupAggLast(handle, groupBy.handle));
    }
    /**
     * Count number of elements in each group
     * @return result sequence iterator (Type.UInt8)
     */
    public SequenceIterator groupAggCount() {
        return new SequenceIterator(groupAggCount(handle));
    }
    /**
     * Approximate distinct values count for each group
     * @param groupBy iterator through grouping sequence
     * @return result sequence iterator (Type.UInt4)
     */
    public SequenceIterator groupAggApproxDC(SequenceIterator groupBy) {
        return new SequenceIterator(groupAggApproxDC(handle, groupBy.handle));
    }
    /**
     * Aproximate distinct hash codes count for each group. This method can be used to estimate number of disctinct pairs, triples,...
     * SequenceIterator.hash() method should be used to calculate combined hash code for all affected sequences.
     * @return result sequence iterator
     */
    public SequenceIterator groupAggApproxHashDC(SequenceIterator groupBy) {
        return new SequenceIterator(groupAggApproxHashDC(handle, groupBy.handle));
    }

    /**
     * Maximum for each interval of sequence
     * @param interval grid interval
     * @return result sequence iterator
     */
    public SequenceIterator gridAggMax(int interval) {
        return new SequenceIterator(gridAggMax(handle, interval));
    }
    /**
     * Minimum for each interval of sequence
     * @param interval grid interval
     * @return result sequence iterator
     */
    public SequenceIterator gridAggMin(int interval) {
        return new SequenceIterator(gridAggMin(handle, interval));
    }
    /**
     * Sum of each interval of sequence
     * @param interval grid interval
     * @return result sequence iterator
     */
    public SequenceIterator gridAggSum(int interval) {
        return new SequenceIterator(gridAggSum(handle, interval));
    }
    /**
     * Average of each grid of elements of sequence
     * @param interval grid interval
     * @return result sequence iterator
     */
    public SequenceIterator gridAggAvg(int interval) {
        return new SequenceIterator(gridAggAvg(handle, interval));
    }
    /**
     * Population variance of each interval of elements of sequence
     * @param interval grid interval
     * @return result sequence iterator
     */
    public SequenceIterator gridAggVar(int interval) {
        return new SequenceIterator(gridAggVar(handle, interval));
    }
    /**
     * Population standard deviation of each interval of elements of sequence
     * @param interval grid interval
     * @return result sequence iterator
     */
    public SequenceIterator gridAggDev(int interval) {
        return new SequenceIterator(gridAggDev(handle, interval));
    }
    /**
     * Sample variance of each interval of elements of sequence
     * @param interval grid interval
     * @return result sequence iterator
     */
    public SequenceIterator gridAggVarSamp(int interval) {
        return new SequenceIterator(gridAggVarSamp(handle, interval));
    }
    /**
     * Sample standard deviation of each interval of elements of sequence
     * @param interval grid interval
     * @return result sequence iterator
     */
    public SequenceIterator gridAggDevSamp(int interval) {
        return new SequenceIterator(gridAggDevSamp(handle, interval));
    }

    /**
     * Maximum of window elements
     * @param interval window size
     * @return result sequence iterator
     */
    public SequenceIterator windowAggMax(int interval) {
        return new SequenceIterator(windowAggMax(handle, interval));
    }
    /**
     * Minimum of window elements
     * @param interval window size
     * @return result sequence iterator
     */
    public SequenceIterator windowAggMin(int interval) {
        return new SequenceIterator(windowAggMin(handle, interval));
    }
    /**
     * Sum of window elements
     * @param interval window size
     * @return result sequence iterator
     */
    public SequenceIterator windowAggSum(int interval) {
        return new SequenceIterator(windowAggSum(handle, interval));
    }
    /**
     * Average of window elements
     * @param interval window size
     * @return result sequence iterator
     */
    public SequenceIterator windowAggAvg(int interval) {
        return new SequenceIterator(windowAggAvg(handle, interval));
    }
    /**
     * Population variance of window elements
     * @param interval window size
     * @return result sequence iterator
     */
    public SequenceIterator windowAggVar(int interval) {
        return new SequenceIterator(windowAggVar(handle, interval));
    }
    /**
     * Population standard deviation of window elements
     * @param interval window size
     * @return result sequence iterator
     */
    public SequenceIterator windowAggDev(int interval) {
        return new SequenceIterator(windowAggDev(handle, interval));
    }
    /**
     * Sample variance of window elements
     * @param interval window size
     * @return result sequence iterator
     */
    public SequenceIterator windowAggVarSamp(int interval) {
        return new SequenceIterator(windowAggVarSamp(handle, interval));
    }
    /**
     * Sample standard deviation of window elements
     * @param interval window size
     * @return result sequence iterator
     */
    public SequenceIterator windowAggDevSamp(int interval) {
        return new SequenceIterator(windowAggDevSamp(handle, interval));
    }

    /**
     * Exponential Moving Average (EMA) indicator
     * @param period EMA period in days
     * @return result sequence iterator
     */
    public SequenceIterator windowAggEma(int period) {
        return new SequenceIterator(windowAggEma(handle, period));
    }

    /**
     * Average True Range (ATR) indicator
     * @param period ATR period in days
     * @return result sequence iterator
     */
    public SequenceIterator windowAggAtr(int period) {
        return new SequenceIterator(windowAggAtr(handle, period));
    }

    /**
     * Get top N maximum values
     * @param top N
     * @return result sequence iterator
     */
    public SequenceIterator topMax(int top) {
        return new SequenceIterator(topMax(handle, top));
    }
    /**
     * Get top N minimum values
     * @param top N
     * @return result sequence iterator
     */
    public SequenceIterator topMin(int top) {
        return new SequenceIterator(topMin(handle, top));
    }
    /**
     * Get positions of top N maximum values
     * @param top N
     * @return result sequence iterator
     */
    public SequenceIterator topPosMax(int top) {
        return new SequenceIterator(topPosMax(handle, top));
    }
    /**
     * Get positions of top N minimum values
     * @param top N
     * @return result sequence iterator
     */
    public SequenceIterator topPosMin(int top) {
        return new SequenceIterator(topPosMin(handle, top));
    }

    /**
     * Cumulative maximum (each element of result is maximum of all preceeding elements in input sequence)
     * @return result sequence iterator
     */
    public SequenceIterator cumAggMax() {
        return new SequenceIterator(cumAggMax(handle));
    }
    /**
     * Cumulative minmum (each element of result is minimum of all preceeding elements in input sequence)
     * @return result sequence iterator
     */
    public SequenceIterator cumAggMin() {
        return new SequenceIterator(cumAggMin(handle));
    }
    /**
     * Cumulative sum (each element of result is sum of all preceeding elements in input sequence)
     * @return result sequence iterator
     */
    public SequenceIterator cumAggSum() {
        return new SequenceIterator(cumAggSum(handle));
    }
    /**
     * Cumulative product (each element of result is product of all preceeding elements in input sequence)
     * @return result sequence iterator
     */
    public SequenceIterator cumAggPrd() {
        return new SequenceIterator(cumAggPrd(handle));
    }
    /**
     * Cumulative average (each element of result is average of all preceeding elements in input sequence)
     * @return result sequence iterator
     */
    public SequenceIterator cumAggAvg() {
        return new SequenceIterator(cumAggAvg(handle));
    }
    /**
     * Cumulative population variance (each element of result is variance of all preceeding elements in input sequence)
     * @return result sequence iterator
     */
    public SequenceIterator cumAggVar() {
        return new SequenceIterator(cumAggVar(handle));
    }
    /**
     * Cumulative population standard deviation (each element of result is standard deviation of all preceeding elements in input sequence)
     * @return result sequence iterator
     */
    public SequenceIterator cumAggDev() {
        return new SequenceIterator(cumAggDev(handle));
    }
    /**
     * Cumulative sample variance (each element of result is variance of all preceeding elements in input sequence)
     * @return result sequence iterator
     */
    public SequenceIterator cumAggVarSamp() {
        return new SequenceIterator(cumAggVarSamp(handle));
    }
    /**
     * Cumulative sample standard deviation (each element of result is standard deviation of all preceeding elements in input sequence)
     * @return result sequence iterator
     */
    public SequenceIterator cumAggDevSamp() {
        return new SequenceIterator(cumAggDevSamp(handle));
    }

    /**
     * Build histogram for the input sequence. Minimal (inclusive) and maximal (exclusive) values for input sequence should be specified as well as number of interval (histogram columns). Number of intervals should not be greater than tile size
     * @param low minimal value (inclusive)
     * @param high maximal value (exclusive)
     * @param nIntervals number of columns in histogram
     * @return result sequence iterator
     */
    public SequenceIterator histogram(long low, long high, int nIntervals) {
        return new SequenceIterator(intHistogram(handle, low, high, nIntervals));
    }
    /**
     * Build histogram for the input sequence. Minimal (inclusive) and maximal (exclusive) values for input sequence should be specified as well as number of interval (histogram columns). Number of intervals should not be greater than tile size
     * @param low minimal value (inclusive)
     * @param high maximal value (exclusive)
     * @param nIntervals number of columns in histogram
     * @return result sequence iterator
     */
    public SequenceIterator histogram(double low, double high, int nIntervals) {
        return new SequenceIterator(realHistogram(handle, low, high, nIntervals));
    }
    /**
     * Find positions in input sequence where it crosses zero.
     * @param firstCrossDirection If firstCrossDirection is positive then starts with first cross over, if negative - with cross below, if zero - doesn't matter (first cross can be over or below)
     * @return result sequence iterator
     */
    public SequenceIterator cross(int firstCrossDirection) {
        return new SequenceIterator(cross(handle, firstCrossDirection));
    }
    /**
     * Find positions of extrema (local minimum and maximums) in input sequence
     * @param firstExtremum If firstExtremum is positive then starts with first local maximum, if negative - with local minimum, if zero - doesn't matter
     * @return result sequence iterator
     */
    public SequenceIterator extrema(int firstExtremum) {
        return new SequenceIterator(extrema(handle, firstExtremum));
    }

    /**
     * Repeat each element of sequence specified number of times: {1,2,3}->repeat(3):{1,1,1,2,2,2,3,3,3}
     * @param count repeat count
     * @return result sequence iterator
     */
    public SequenceIterator repeat(int count) {
        return new SequenceIterator(repeat(handle, count));
    }

    /**
     *  Stretches this sequence to the length of first sequence with timestamps. Repeat elements of this sequence with values until correspondent timestamp (sequence ts2) is larger than timestamp from ts1. Assume ts1={1,2,3,4,5}, ts2={2,4}, values={1.1,2.2} and filler=1.0, then result will be {1.1,1.1,2.2,2.2,1.0}
     * @param ts1 longer time series
     * @param ts2 shorter time series (matching elements of this sequence)
     * @param filler value used for tail elements of stretched sequence
     * @return result sequence iterator
     */
    public SequenceIterator stretch(SequenceIterator ts1, SequenceIterator ts2, long filler) {
        return new SequenceIterator(intStretch(handle, ts1.handle, ts2.handle, filler));
    }
    /**
     * Stretches this sequence to the length of first sequence with timestamps. Repeat elements of this sequence with values until correspondent timestamp (sequence ts2) is larger than timestamp from ts1. Assume ts1={1,2,3,4,5}, ts2={2,4}, values={1.1,2.2} and filler=1.0, then result will be {1.1,1.1,2.2,2.2,1.0}
     * @param ts1 longer time series
     * @param ts2 shorter time series (matching elements of this sequence)
     * @param filler value used for tail elements of stretched sequence
     * @return result sequence iterator
     */
    public SequenceIterator stretch(SequenceIterator ts1, SequenceIterator ts2, double filler) {
        return new SequenceIterator(realStretch(handle, ts1.handle, ts2.handle, filler));
    }

    /**
     * Inject missed elements in this sequences so that correspondent timestamp sequences elements are matched.  Assume ts1={1,2,3,5}, ts2={2,3,4}, values={1.1,1.2,1.3} and filler=0.0, then result sequences will be {0.0,1.1,1.2,1.3,1.4,0.0}
     * @param ts1 timestamps of matched sequence
     * @param ts2 timestamps associated with this sequence
     * @param filler fillter element for stretched sequence
     * @return result sequence iterator
     */
    public SequenceIterator stretch0(SequenceIterator ts1, SequenceIterator ts2, long filler) {
        return new SequenceIterator(intStretch0(handle, ts1.handle, ts2.handle, filler));
    }
    /**
     * Inject missed elements in this sequences so that correspondent timestamp sequences elements are matched.  Assume ts1={1,2,3,5}, ts2={2,3,4}, values={1.1,1.2,1.3} and filler=0.0, then result sequences will be {0.0,1.1,1.2,1.3,1.4,0.0}
     * @param ts1 timestamps of matched sequence
     * @param ts2 timestamps associated with this sequence
     * @param filler values used for tail elemented of stretched sequence
     * @return result sequence iterator
     */
    public SequenceIterator stretch0(SequenceIterator ts1, SequenceIterator ts2, double filler) {
        return new SequenceIterator(realStretch0(handle, ts1.handle, ts2.handle, filler));
    }

    /**
     * Get values from this sequence corresponding to timestamp of ts2 closest to the timestamp of ts1.  Assume ts1={4,9}, ts2={1,3,6,10}, values={0.1,0.3,0.6,1.0}, then result sequences will be {0.3,1.0}
     * @param ts1 timestamps of matched sequence
     * @param ts2 timestamps associated with this sequence
     * @param filler values used for tail elemented of stretched sequence
     * @return result sequence iterator
     */
    public SequenceIterator asofJoin(SequenceIterator ts1, SequenceIterator ts2) {
        return new SequenceIterator(asofJoin(handle, ts1.handle, ts2.handle));
    }

    /**
     * Convert elements of this sequence to the specified type
     * @param type result sequence type
     * @return result sequence iterator
     */
    public SequenceIterator cast(Sequence.Type type) {
        return new SequenceIterator(cast(handle, type.ordinal()));
    }


    /**
     * Calculate 32-bit hash code for each value of input sequence.
     * @return result sequence iterator
     */
    public SequenceIterator hash() {
        return new SequenceIterator(hash(handle, 0));
    }

   /**
     * Calculate 32-bit hash code for each value of input sequence and combine it with hash code calculated for other sequence
     * @return result sequence iterator
     */
    public SequenceIterator hash(SequenceIterator other) {
        return new SequenceIterator(hash(handle, other.handle));
    }

    /**
     * Maximum of each group of elements of sequence. Unlike groupAggMax this functions do not require that grouping sequence is ordered.
     * @param groupBy iterator through grouping sequence
     * @param nGroups estimation for number of groups (0 - not available)
     * @return pair of sequences representing aggregation and grouping result
     */
    public GroupByResult hashAggMax(SequenceIterator groupBy, int nGroups) {
        return new GroupByResult(hashAggMax(handle, groupBy.handle, nGroups));
    }

    /**
     * Minimum of each group of elements of sequence. Unlike groupAggMin this functions do not require that grouping sequence is ordered.
     * @param groupBy iterator through grouping sequence
     * @param nGroups estimation for number of groups (0 - not available)
     * @return pair of sequences representing aggregation and grouping result
     */
    public GroupByResult hashAggMin(SequenceIterator groupBy, int nGroups) {
        return new GroupByResult(hashAggMin(handle, groupBy.handle, nGroups));
    }

    /**
     * Sum of each group of elements of sequence. Unlike groupAggSum this functions do not require that grouping sequence is ordered.
     * @param groupBy iterator through grouping sequence
     * @param nGroups estimation for number of groups (0 - not available)
     * @return pair of sequences representing aggregation and grouping result
     */
    public GroupByResult hashAggSum(SequenceIterator groupBy, int nGroups) {
        return new GroupByResult(hashAggSum(handle, groupBy.handle, nGroups));
    }

    /**
     * Average of each group of elements of sequence. Unlike groupAggAvg this functions do not require that grouping sequence is ordered.
     * @param groupBy iterator through grouping sequence
     * @param nGroups estimation for number of groups (0 - not available)
     * @return pair of sequences representing aggregation and grouping result
     */
    public GroupByResult hashAggAvg(SequenceIterator groupBy, int nGroups) {
        return new GroupByResult(hashAggAvg(handle, groupBy.handle, nGroups));
    }

    /**
     * Count number of elements of each group. Unlike groupAggCount this functions do not require that sequence is ordered.
     * @param nGroups estimation for number of groups (0 - not available)
     * @return pair of sequences representing aggregation and grouping result
     */
    public GroupByResult hashAggCount(int nGroups) {
        return new GroupByResult(hashAggCount(handle, nGroups));
    }
    /**
     * Approximate distinct count for each group.
     * @param groupBy iterator through grouping sequence
     * @param nGroups estimation for number of groups (0 - not available)
     */
    public GroupByResult hashAggApproxdc(SequenceIterator groupBy, int nGroups) {
        return new GroupByResult(hashAggApproxdc(handle, groupBy.handle, nGroups));
    }

    /**
     * Count number of distinct elements in each group.
     * @param groupBy iterator through grouping sequence
     * @param nGroups estimation for number of groups (0 - not available)
     * @param nPairs estimation for number of unique pairs (0 - not available)
     * @return pair of sequences representing aggregation and grouping result
     */
    public GroupByResult hashAggDistinctCount(SequenceIterator groupBy, int nGroups, int nPairs) {
        return hashAggDupCount(groupBy, nGroups, nPairs, 1);
    }

    /**
     * Count number of duplicates in each group.
     * @param groupBy iterator through grouping sequence
     * @param nGroups estimation for number of groups (0 - not available)
     * @param nPairs estimation for number of unique pairs (0 - not available)
     * @param minOccurrences minimal number for occurrences of a particular value to be counted.
     * With minOccurrences=1 this functions is equivalent to hashAggDistinctCount, with minOccurrences=2 it counts items encountered more than once...
     * @return pair of sequences representing aggregation and grouping result
     */
    public GroupByResult hashAggDupCount(SequenceIterator groupBy, int nGroups, int nPairs, int minOccurrences) {
        return new GroupByResult(hashAggDupCount(handle, groupBy.handle, nGroups, nPairs, minOccurrences));
    }

   /**
     * Sort sequence elements (previously extracted using SequenceIterator.get method) and construct permutation array which can be used to access elements of other sequences (also extracted to arrays)
     * @param arr immutable array with sequence elements
     * @param permutation array which will be filled by this function. Number of sorted elements is equal to size of this array.
     * @param order sort order
     */
    public void sort(Object arr, long[] permutation, Sequence.Order order) {
        if (Array.getLength(arr) < permutation.length) {
            throw new IllegalArgumentException("Size of data array should not be smaller than size of permutation array");
        }
        sort(handle, arr, permutation, order.ordinal());
    }

    /**
     * Reset iterator to first position
     */
    public void reset() {
        reset(handle);
    }

    /**
     * Close iterator and release all resources
     */
    public void close() {
        if (handle != 0) {
            close(handle);
            handle = 0;
        }
    }

    protected void finalize() {
        close();
    }

    SequenceIterator(long hnd) {
        handle = hnd;
    }
    private native Object tileItems(long hnd);
    private native int itemRepeatCount(long hnd, int i);
    private native long rleDecode(long hnd);
    private native long firstPosition(long hnd);
    private native long lastPosition(long hnd);
    private native long nextPosition(long hnd);
    private native boolean nextTile(long hnd);
    private native int type(long hnd);
    private native int elemSize(long hnd);

    private native long add(long left, long right);
    private native long sub(long left, long right);
    private native long div(long left, long right);

    private native long mul(long left, long right);
    private native long mod(long left, long right);

    private native long eq(long left, long right);
    private native long ne(long left, long right);
    private native long lt(long left, long right);
    private native long gt(long left, long right);
    private native long le(long left, long right);
    private native long ge(long left, long right);

    private native long max(long left, long right);
    private native long min(long left, long right);

    private native long and(long left, long right);
    private native long or(long left, long right);
    private native long xor(long left, long right);

    private native long neg(long hnd);
    private native long not(long hnd);

    private native long abs(long hnd);

    private native long match(long hnd, String pattern);


    private static native long intConstant(long val, int type);
    private static native long realConstant(double val, int type);
    private static native long charConstant(byte[] val);
    private static native long parseString(String val, int type);

    private native int get(long hnd, Object buffer);

    private native long concat(long hnd, long tail);
    private native long cat(long hnd, long tail);

    private native long wsum(long hnd, long weights);
    private native long wavg(long hnd, long weights);
    private native long cov(long left, long right);
    private native long corr(long left, long right);

    private native long iif(long cond, long then, long otherwise, boolean lazy);
    private native long filter(long cond, long values);
    private native long filterPos(long cond);
    private native long unique(long hnd);
    private native long normalize(long hnd);
    private native long thin(long hnd, long origin, long step);
    private native long limit(long hnd, long from, long till);
    private native long reverse(long hnd);
    private native long diff(long hnd);
    private native long trend(long hnd);
    private native long aggCount(long hnd);

    private native long aggMax(long hnd);
    private native long aggMin(long hnd);
    private native long aggSum(long hnd);
    private native long aggPrd(long hnd);
    private native long aggAvg(long hnd);
    private native long aggVar(long hnd);
    private native long aggDev(long hnd);
    private native long aggVarSamp(long hnd);
    private native long aggDevSamp(long hnd);
    private native long aggApproxDC(long hnd);
    private native long aggApproxHashDC(long hnd);
    private native long aggMinMax(long hnd);

    private native long groupAggWavg(long left, long right, long groupBy);
    private native long groupAggMax(long hnd, long groupBy);
    private native long groupAggMin(long hnd, long groupBy);
    private native long groupAggSum(long hnd, long groupBy);
    private native long groupAggAvg(long hnd, long groupBy);
    private native long groupAggVar(long hnd, long groupBy);
    private native long groupAggDev(long hnd, long groupBy);
    private native long groupAggVarSamp(long hnd, long groupBy);
    private native long groupAggDevSamp(long hnd, long groupBy);
    private native long groupAggFirst(long hnd, long groupBy);
    private native long groupAggLast(long hnd, long groupBy);
    private native long groupAggCount(long groupBy);
    private native long groupAggApproxDC(long hnd, long groupBy);
    private native long groupAggApproxHashDC(long hnd, long groupBy);

    private native long gridAggMax(long hnd, int interval);
    private native long gridAggMin(long hnd, int interval);
    private native long gridAggSum(long hnd, int interval);
    private native long gridAggAvg(long hnd, int interval);
    private native long gridAggVar(long hnd, int interval);
    private native long gridAggDev(long hnd, int interval);
    private native long gridAggVarSamp(long hnd, int interval);
    private native long gridAggDevSamp(long hnd, int interval);

    private native long windowAggMax(long hnd, int interval);
    private native long windowAggMin(long hnd, int interval);
    private native long windowAggSum(long hnd, int interval);
    private native long windowAggAvg(long hnd, int interval);
    private native long windowAggVar(long hnd, int interval);
    private native long windowAggDev(long hnd, int interval);
    private native long windowAggVarSamp(long hnd, int interval);
    private native long windowAggDevSamp(long hnd, int interval);
    private native long windowAggEma(long hnd, int interval);
    private native long windowAggAtr(long hnd, int interval);

    private native long topMax(long hnd, int top);
    private native long topMin(long hnd, int top);
    private native long topPosMax(long hnd, int top);
    private native long topPosMin(long hnd, int top);

    private native long cumAggMax(long hnd);
    private native long cumAggMin(long hnd);
    private native long cumAggSum(long hnd);
    private native long cumAggPrd(long hnd);
    private native long cumAggAvg(long hnd);
    private native long cumAggVar(long hnd);
    private native long cumAggDev(long hnd);
    private native long cumAggVarSamp(long hnd);
    private native long cumAggDevSamp(long hnd);

    private native long intHistogram(long hnd, long low, long high, int nIntervals);
    private native long realHistogram(long hnd, double low, double high, int nIntervals);
    private native long cross(long hnd, int firstCrossDirection);
    private native long extrema(long hnd, int firstExtremum);
    private native long repeat(long hnd, int count);
    private native long intStretch(long hnd, long ts1, long ts2, long filler);
    private native long realStretch(long hnd, long ts1, long ts2, double filler);
    private native long intStretch0(long hnd, long ts1, long ts2, long filler);
    private native long realStretch0(long hnd, long ts1, long ts2, double filler);
    private native long asofJoin(long hnd, long ts1, long ts2);

    private native long cast(long hnd, int type);
    private native void tee(long hnd, long[] pair);
    private native long map(long hnd, long positions);

    private native long   hash(long hnd, long hash);

    private native long[] hashAggMax(long hnd, long groupBy, int nGroups);
    private native long[] hashAggMin(long hnd, long groupBy, int nGroups);
    private native long[] hashAggSum(long hnd, long groupBy, int nGroups);
    private native long[] hashAggAvg(long hnd, long groupBy, int nGroups);
    private native long[] hashAggApproxdc(long hnd, long groupBy, int nGroups);
    private native long[] hashAggCount(long hnd, int nGroups);
    private native long[] hashAggDupCount(long hnd, long groupBy, int nGroups, int nPairs, int minOccurrences);

    private native void sort(long hnd, Object arr, long[] permutation, int order);
    private native void reset(long hnd);

    private native void close(long hnd);

    long handle;
}

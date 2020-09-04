import com.mcobject.extremedb.*;
import java.util.Arrays;

public class Query
{
    static final int BUF_SIZE = 256;
    
    private static int printIntSequence(Quote quote, SequenceIterator iterator)
    {
        System.out.print(quote.symbol + ": {");
        Object elem;
        int i;
        for (i = 0; (elem = iterator.next()) != null; i++) { 
            if (i != 0) System.out.print(",");
            System.out.print(elem);
        }
        System.out.println("}");
        return i;
    }

    private static int printFloatSequence(Quote quote, SequenceIterator iterator)
    {
        System.out.print(quote.symbol + ": {");
        Object elem;
        int i;
        for (i = 0; (elem = iterator.next()) != null; i++) { 
            if (i != 0) System.out.print(",");
            System.out.format("%.3f", elem);
        }
        System.out.println("}");
        return i;
    }

    /**
     * Simple iteration through two related sequences.
     * Sequences belong to the same timeseries and therefore have the same number of elements
     */
    public static void iterate(Connection con)
    {
        int[] dayBuf = new int[BUF_SIZE];
        float[] highBuf = new float[BUF_SIZE];
        byte[] dayStrBuf = new byte[15*BUF_SIZE];
        int len;

        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- ITERATE -------------------");

        /* Iterate through all objects */
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Prepare iterators
            SequenceIterator dayIterator = quote.day.iterator();
            SequenceIterator highIterator = quote.high.iterator();
            SequenceIterator dayStrIterator = quote.day_str.iterator();

            // Execute query
            while ((len = dayIterator.get(dayBuf)) > 0) {
                int len2 = highIterator.get(highBuf);
                int len3 = dayStrIterator.get(dayStrBuf);
                assert(len == len2);
                assert(len == len3);
                for (int i = 0; i < len; i++) { 
                    String s = new String(Arrays.copyOfRange(dayStrBuf, i*15, i*15+12));
                    System.out.format("%s[%d(%s)]: %f\n", quote.symbol, dayBuf[i], s, highBuf[i]);
                }
            }
        }
        con.commitTransaction();
    }

    /**
     * Extract data belonging to specified interval.
     * Ordered sequence (day) is used to select interval and then this interval is projected to other (unordered) sequences
     */
    public static void interval(Connection con)
    {
        int[] dayBuf = new int[BUF_SIZE];
        float[] openBuf = new float[BUF_SIZE];
        float[] closeBuf = new float[BUF_SIZE];
        int len;

        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- INTERVAL ------------------");

        // Iterate through all objects
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Select interval
            SequenceIterator dayIterator = quote.day.search(20130101, OrderedSequence.Boundary.Inclusive, 20130401, OrderedSequence.Boundary.Inclusive);
            // Prepare iterators
            SequenceIterator openIterator = quote.open.project(dayIterator);
            SequenceIterator closeIterator = quote.close.project(dayIterator);

            // Execute query
            while ((len = dayIterator.get(dayBuf)) > 0) {
                int len2 = openIterator.get(openBuf);
                assert(len == len2);
                len2 = closeIterator.get(closeBuf);
                assert(len == len2);
                for (int i = 0; i < len; i++) { 
                    System.out.format("%s[%d]: %f..%f\n", quote.symbol, dayBuf[i], openBuf[i], closeBuf[i]);
                }
            }
        }
        con.commitTransaction();
    }


    /**
     * Evaluate expression for sequences. This example calculates average of high and low prices.
     */
    public static void apply(Connection con)
    {
        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- APPLY ---------------------");

        // Iterate through all objects
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Execute query
            printFloatSequence(quote, quote.high.iterator().add(quote.low.iterator()).div(SequenceIterator.constant(2.0, Sequence.Type.Float)));
        }
        con.commitTransaction();
    }

    /**
     * Filter elements of sequence. Get days of last month when close > open.
     */
    public static void filter(Connection con)
    {
        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- FILTER --------------------");

        // Iterate through all objects
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Select interval
            long count = quote.day.count();
            int last = quote.day.subseq(count-1, count-1).nextInt();
            SequenceIterator dayIterator = quote.day.search(IntDate.construct(1, IntDate.month(last), IntDate.year(last)), OrderedSequence.Boundary.Inclusive, last, OrderedSequence.Boundary.Inclusive);

            // Execute query
            printIntSequence(quote, dayIterator.filter(quote.close.project(dayIterator).gt(quote.open.project(dayIterator))));
        }
        con.commitTransaction();
    }

    /**
     * Find correlation between two sequences
     */
    public static void correlation(Connection con)
    {
        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- CORRELATION ---------------");

        // Iterate through all objects
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Execute query
            printFloatSequence(quote, quote.low.iterator().corr(quote.high.iterator()));
        }
        con.commitTransaction();
    }


    /**
     * Calculate grand (sequence-wide) aggregates. Get maximal close price for each symbol.
     */
    public static void grandAggregate(Connection con)
    {
        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- GRAND AGGREGATE -----------");

        // Iterate through all objects
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Execute query
            printFloatSequence(quote, quote.close.iterator().aggMax());
        }
        con.commitTransaction();
    }

    /**
     * Calculate window (moving) aggregates. Find dates where the 20-day moving average crosses over the 5-day moving average.
     */
    public static void windowAggregate(Connection con)
    {
        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- WINDOW AGGREGATE ----------");

        // Iterate through all objects
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Execute query
            printIntSequence(quote, quote.day.map(quote.close.iterator().windowAggAvg(20).sub(quote.close.iterator().windowAggAvg(5)).cross(1)));
        }
        con.commitTransaction();
    }

    /**
     * Calculate grid aggregates. Determine maximal close price for each week 
     */
    public static void gridAggregate(Connection con)
    {
        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- GRID AGGREGATE ------------");

        // Iterate through all objects
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Execute query
            printFloatSequence(quote, quote.close.iterator().gridAggMax(7));
        }
        con.commitTransaction();
    }

    /**
     * Calculate aggregate for each group. A group is determined by ordered sequence elements with the same value.
     * Find out total volume for each month of this year.
     */
    public static void groupByAggregate(Connection con)
    {
        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- GROUP BY AGGREGATE --------");

        // Iterate through all objects
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Select interval
            SequenceIterator dayIterator = quote.day.search(20130101, OrderedSequence.Boundary.Inclusive, 20140101, OrderedSequence.Boundary.Exclusive);
            // Execute query
            printIntSequence(quote, quote.volume.project(dayIterator).groupAggSum(dayIterator.div(SequenceIterator.constant(100, Sequence.Type.UInt4))));
        }
        con.commitTransaction();
    }

    /**
     * Calculate aggregate for each group. Groups are determined by arbitrary (not necessarily sorted) sequence 
     * using a hash table.
     * Find average price for volume groups 0..10, 11..20, 21..30...
     */
    public static void hashAggregate(Connection con)
    {
        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- HASH AGGREGATE ------------");

        // Iterate through all objects
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Execute query
            GroupByResult result = quote.close.iterator().hashAggAvg(quote.volume.iterator().div(SequenceIterator.constant(10, Sequence.Type.UInt4)), 0);
            printFloatSequence(quote, result.getAggIterator());
            result.close();
        }
        con.commitTransaction();
    }

    /**
     * Sort sequence elements. This example outputs close prices and volumes in volume descending order
     */
    public static void sort(Connection con)
    {
        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- SORT ----------------------");

        // Iterate through all objects
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Extract data into array 
            int count = (int)quote.volume.count();
            int[] volumes = new int[count];
            float[] prices = new float[count];
            long permutation[] = new long[count];
            int len = quote.volume.iterator().get(volumes);
            assert(len == count);
            len = quote.close.iterator().get(prices);
            assert(len == count);
            
            // Execute query
            quote.volume.iterator().sort(volumes, permutation, Sequence.Order.Descending); 
            for (int i = 0; i < count; i++) { 
                int j = (int)permutation[i];
                System.out.format("%s: volume=%d, price=%.3f\n", quote.symbol, volumes[j], prices[j]);
            }
        }
        con.commitTransaction();
    }

    /**
     * Get top-N sequence elements. This example calculates top-10 close prices and correspondent volumes
     */    
    public static void top(Connection con)
    {
        int[] volumeBuf = new int[BUF_SIZE];
        float[] closeBuf = new float[BUF_SIZE];
        int len;

        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- TOP -----------------------");

        // Iterate through all objects
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Prepare iterators
            SequenceIterator topCloseIterator = quote.close.iterator().topMax(10);
            SequenceIterator topVolumeIterator = quote.volume.map(quote.close.iterator().topPosMax(10));

            // Execute query
            while ((len = topVolumeIterator.get(volumeBuf)) > 0) {
                int len2 = topCloseIterator.get(closeBuf);
                assert(len == len2);
                for (int i = 0; i < len; i++) { 
                    System.out.format("%s: price=%.3f, volume=%d\n", quote.symbol, closeBuf[i], volumeBuf[i]);
                }
            }
        }
        con.commitTransaction();
    }
    
    /**
     * Volume weighted average prices for last week: vwap = sum(close*volume)/sum(volume)
     */
    public static void VWAP(Connection con)
    {
        con.startTransaction(Database.TransactionType.ReadOnly);  
        System.out.println("--- VWAP ----------------------");

        // Iterate through all objects
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        for (Quote quote : cursor) { 
            // Select interval
            long count = quote.day.count();
            int last = quote.day.subseq(count-1, count-1).nextInt();
            SequenceIterator slice = quote.day.search(last-7, OrderedSequence.Boundary.Exclusive, last, OrderedSequence.Boundary.Inclusive);

            // Prepare iterators
            SequenceIterator vwap = quote.volume.project(slice).cast(Sequence.Type.Float).mul(quote.close.project(slice)).aggSum().div(quote.volume.project(slice).aggSum().cast(Sequence.Type.Double));

            // Execute query
            System.out.format("%s: %f\n", quote.symbol, vwap.nextDouble());
        }
        con.commitTransaction();
    }
}

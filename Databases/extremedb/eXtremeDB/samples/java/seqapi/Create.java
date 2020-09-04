import com.mcobject.extremedb.*;
import com.mcobject.extremedb.Database.Device.Kind;

public class Create
{
    static final int PAGE_SIZE = 4096;
    static final long DATABASE_SIZE = 64L*1024*1024;
    static long seed = 2013;
    static int dayOfYear;

    public static Database database(String[] args)
    {
        int config = Database.MCO_CFG_SEQUENCE_SUPPORT;

        Database.Device[] devs;
        Database.Parameters params = new Database.Parameters();
        params.memPageSize = PAGE_SIZE; // Memory page size
        params.classes = new Class[] { Quote.class }; // List of classes which should be stored in eXtremeDB database. The classes must use @Persistent annotation

        // Get config parameters from the command line
        for (String s : args)
        {
            if ("disk".equals(s))  config |= Database.MCO_CFG_DISK_SUPPORT;
            if ("debug".equals(s)) config |= Database.MCO_CFG_DEBUG_LIBRARY;
            if ("mvcc".equals(s))  config |= Database.MCO_CFG_MVCC_TRANSACTION_MANAGER;
            if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        if ((config & Database.MCO_CFG_DISK_SUPPORT) != 0)
        {
            java.io.File file = new java.io.File("stockdb.dbs");
            file.delete();
            file = new java.io.File("stockdb.log");
            file.delete();

            params.diskPageSize = 32768;
            params.diskClassesByDefault = true;
            devs = new Database.Device[] {
                new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE),
                new Database.PrivateMemoryDevice(Database.Device.Kind.DiskCache, DATABASE_SIZE),
                new Database.FileDevice(Kind.Data, "stockdb.dbs"),
                new Database.FileDevice(Kind.TransactionLog, "stockdb.log")
            };
        } else { // in-mem only
            devs = new Database.Device[] {
                new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE)
            };
        }

        Database db = new Database(config);
        db.open("stockdb", params, devs);
        return db;
    }

    static int randomInteger(int mod)
    {
        return (int)(seed = (seed * 3141592621L + 2718281829L) % 1000000007L) % mod;
    }

    static float randomFloat() 
    { 
        return (float)randomInteger(10000) / 100;
    }

    static void insertRandomQuote(Connection con, int n_symbols) { 
        float temp;
        String symbol = "SYM" + randomInteger(n_symbols);
        float low  = randomFloat();
        float high = randomFloat();
        float open = randomFloat();
        float close = randomFloat();
        if (high < low) { temp = high; high = low; low = temp; } 
        if (high < open) { temp = high; high = open; open = temp; } 
        if (high < close) { temp = high; high = close; close = temp; } 
        if (low > open) { temp = low; low = open; open = temp; } 
        if (low > close) { temp = low; low = close; close = temp; } 
        int volume = randomInteger(1000);
        /* Date is represented by an integer in YYYYMMDD format, for example 20130830 */
        int day = IntDate.construct(1 + dayOfYear%30, 1 + dayOfYear/30%12, 2013 + dayOfYear/30/12);
        dayOfYear += 1;

        con.startTransaction(Database.TransactionType.ReadWrite);        
        Quote quote;
        Cursor<Quote> cursor = new Cursor<Quote>(con, Quote.class, "symbol");
        while ((quote = cursor.find(symbol)) == null) {
            quote = new Quote();
            quote.symbol = symbol;
            con.insert(quote);
            con.checkpointTransaction();
        }
        quote.day.append(new int[]{day});
        quote.low.append(new float[]{low});
        quote.high.append(new float[]{high});
        quote.open.append(new float[]{open});
        quote.close.append(new float[]{close});
        quote.volume.append(new int[]{volume});
        String tid = new String("Day:" + Long.toString(day));
        quote.day_str.append(new String[]{tid});
        /*
        // There is another way to pass a string to sequence just as a type byte array
        byte[] tid_bytes = new byte[15]; // Array must be divisible by element size
        byte[] tb = tid.getBytes();
        System.arraycopy(tb, 0, tid_bytes, 0, tb.length);
        quote.day_str.append(tid_bytes);
        */
        con.commitTransaction();
    }

    public static void randomData(Connection con, int n_symbols, int n_quotes)
    {
        for (int i = 0; i < n_quotes; i++) { 
            insertRandomQuote(con, n_symbols);
        }
    }
}

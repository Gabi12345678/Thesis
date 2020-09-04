import com.mcobject.extremedb.*;

import java.io.Console;
import java.util.Date;

@Persistent // Class will be stored in eXtremeDB database
class Quotes
{
    @Indexable(unique=true)
    @Dimension(4)
    String Symbol;

    @Sequence(type=Sequence.Type.Int8, order=Sequence.Order.Ascending)
    OrderedSequence qTime;

    @Sequence(type=Sequence.Type.Float)
    UnorderedSequence Volume;

    @Sequence(type=Sequence.Type.Char, elemSize=8)
    UnorderedSequence Str;

    @Sequence(type=Sequence.Type.DateTime)
    UnorderedSequence qDt;
};

public class SqlSeq
{
    static final int PAGE_SIZE = 128;
    static final int DISK_PAGE_SIZE = 4096;
    static final int DISK_CACHE_SIZE = 8*1024*1024;
    static final int DATABASE_SIZE = 16*1024*1024;
    static final String [] symbols = {"sbl", "dbl", "tpl", "apl"};
    static final int N_QUOTES = 10;

    public static void main(String[] args)
    { 
        Database db;
        int config = Database.MCO_CFG_SQL_SUPPORT | Database.MCO_CFG_SEQUENCE_SUPPORT;
        Database.Device devs[];
        Database.Parameters params = new Database.Parameters();
        String host = "localhost";
        int port = 5001;
        int maxAttempts = 10;

        // Get config parameters from the command line
        for (String s : args) {
            if ("disk".equals(s))  config |= Database.MCO_CFG_DISK_SUPPORT;
            if ("shm".equals(s))   config |= Database.MCO_CFG_SHARED_MEMORY;
            if ("debug".equals(s)) config |= Database.MCO_CFG_DEBUG_LIBRARY;
            if ("mvcc".equals(s))  config |= Database.MCO_CFG_MVCC_TRANSACTION_MANAGER;
            if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        // Create Database object
        db = new Database(config);
        Database.RuntimeInfo ri = db.getRunTimeInfo();
        long hint = ri.direct_pointers_supported ? 0x20000000 : 0;

        params.memPageSize = PAGE_SIZE; // Memory page size
        params.classes = new Class[] { Quotes.class }; // List of classes which should be stored in eXtremeDB database. The classes must use @Persistent annotation
        params.maxConnections = 10;
        
        if ((config & Database.MCO_CFG_DISK_SUPPORT) != 0) { // on-disk configuration

            java.io.File file = new java.io.File("opendb.dbs");
            file.delete();
            file = new java.io.File("opendb.log");
            file.delete();

            params.diskPageSize = DISK_PAGE_SIZE; // Disk page size
            params.diskClassesByDefault = true; // Mark @Persistent classes as on-disk classes by default

            // Declare memory devices
            devs = new Database.Device[4];

            if ((config & Database.MCO_CFG_SHARED_MEMORY) != 0) {
                devs[0] = new Database.SharedMemoryDevice(Database.Device.Kind.Data, "opendb-db", 0, DATABASE_SIZE);
                devs[1] = new Database.SharedMemoryDevice(Database.Device.Kind.DiskCache, "opendb-cache", 0, DISK_CACHE_SIZE);
            } else {
                devs[0] = new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE);
                devs[1] = new Database.PrivateMemoryDevice(Database.Device.Kind.DiskCache, DISK_CACHE_SIZE);
            }
                
            devs[2] = new Database.FileDevice(Database.Device.Kind.Data, "opendb.dbs");
            devs[3] = new Database.FileDevice(Database.Device.Kind.TransactionLog, "opendb.log");

        } else { // in-memory configuration

            devs = new Database.Device[1];

            if ((config & Database.MCO_CFG_SHARED_MEMORY) != 0) {
                devs[0] = new Database.SharedMemoryDevice(Database.Device.Kind.Data, "opendb-db", hint, DATABASE_SIZE);
            } else {
                devs[0] = new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE);
            }
        }

        // Create Database object
        db = new Database(config);
        db.open("sqldb", params, devs); // open database. If the database is already opened a DatabaseError exception is thrown
            
        System.out.println("Open " + 
                           (((config & Database.MCO_CFG_DISK_SUPPORT) != 0) ? "on-disk" : "in-memory") + " database on " + 
                           (((config & Database.MCO_CFG_SHARED_MEMORY) != 0) ? "shared" : "conv") + " memory. Use " + 
                           (((config & Database.MCO_CFG_MVCC_TRANSACTION_MANAGER) != 0) ? "MVCC" : "MURSIW") + " transaction manager");

        SqlLocalConnection con = db.connectSql();

        SqlServer server = new SqlServer(con, port);
        server.start();

        SqlConnection rcon = new SqlRemoteConnection(host, port, maxAttempts);

        insertData(con);

        System.out.println("Query data using local connection");
        queryData(con);

        System.out.println("Query data using remote connection");
        queryData(rcon);

        rcon.disconnect();

        server.stop();

        con.disconnect();

        System.out.println("Close database");
        db.close();
    }

    private static void insertData(SqlConnection conn) {

        for (int i = 0; i < symbols.length; i++) {
            long [] times = new long[N_QUOTES];
            float[] volumes = new float[N_QUOTES];
            String[] strings = new String[N_QUOTES];
            Date[] dtt = new Date[N_QUOTES];
            long base_time = (i + 1) * 100000;
            float base_volume = (i + 1) * 1000;
            for (int j = 0; j < N_QUOTES; j++) {
                times[j] = base_time + 100 + j;
                volumes[j] = base_volume + 1 + j;
                strings[j] = new String("str" + i + "_" + j);
                dtt[j] = new Date(2017 - 1900, 6, j + 1, 13, 35, 17);
            }
            conn.executeStatement("INSERT OR UPDATE INTO Quotes (Symbol, qTime, Volume, Str, qDt) VALUES (?,?,?,?,?)",
                symbols[i], times, volumes, strings, dtt);
        }
    }

    private static void queryData(SqlConnection conn) {
        for (int i = 0; i < symbols.length; i++) {
            SqlResultSet result = conn.executeQuery("SELECT Symbol, qTime, Volume, Str, qDt FROM Quotes WHERE Symbol=?", symbols[i]);

            System.out.println("SELECT Symbol, qTime, Volume, Str, qDt FROM Quotes WHERE Symbol=" + symbols[i]);

            String[] columns = result.getColumnNames();
            Class[] classes = result.getColumnTypes();
            for (int j = 0; j < columns.length; j++) {
                System.out.println("Column '" + columns[j] + "' has type '" + classes[j].getName() + "'");
            }

            for (SqlTuple tuple: result) {
                for (int j = 0; j < columns.length; j++) {
                    if (tuple.get(j).getClass().getName() == "com.mcobject.extremedb.SequenceIterator") {
                        SequenceIterator seq = (SequenceIterator)tuple.get(j);
                        Sequence.Type type = seq.type();
                        long[] longBuf = new long[1];
                        float[] floatBuf = new float[1];
                        byte[] bytesBuf = new byte[8]; // This is to contain one element of sequence Str; its size (8) 
                                                       // must correspond to the Sequence(ElemSize) property on line 18
                        Date[] dtBuf = new Date[1];
                        int len;
                        System.out.print("{");
                        if (type == Sequence.Type.Int8) {
                            int k = 0;
                            while ((len = seq.get(longBuf)) > 0) {
                                System.out.print(longBuf[0] + " ");
                                k++;
                            }
                        } else if (type == Sequence.Type.Float) {
                            int k = 0;
                            while ((len = seq.get(floatBuf)) > 0) {
                                System.out.print(floatBuf[0] + " ");
                                k++;
                            }
                        } else if (type == Sequence.Type.DateTime) {
                            int k = 0;
                            while ((len = seq.get(dtBuf)) > 0) {
                                System.out.print(dtBuf[0] + " ");
                                k++;
                            }
                        } else if (type == Sequence.Type.Char) {
                            int k = 0;
                            while ((len = seq.get(bytesBuf)) > 0) {
                                String s = new String(bytesBuf);
                                System.out.print(s + " ");
                                k++;
                            }
                        }
                        System.out.print("}");
                    } else if (tuple.get(j).getClass().getName() == "[J") {
                        System.out.print("{");
                        long[] longArray = (long[])tuple.get(j);
                        for (int k = 0; k < longArray.length; k++) {
                            System.out.print(longArray[k] + " ");
                        }
                        System.out.print("}");
                    } else if (tuple.get(j).getClass().getName() == "[F") {
                        System.out.print("{");
                        float[] floatArray = (float[])tuple.get(j);
                        for (int k = 0; k < floatArray.length; k++) {
                            System.out.print(floatArray[k] + " ");
                        }
                        System.out.print("}");
                    } else if (tuple.get(j).getClass().getName() == "[Ljava.lang.String;") {
                        System.out.print("{");
                        String[] stringArray = (String[])tuple.get(j);
                        for (int k = 0; k < stringArray.length; k++) {
                            System.out.print(stringArray[k] + " ");
                        }
                        System.out.print("}");
                    } else if (tuple.get(j).getClass().getName() == "[Ljava.util.Date;") {
                        System.out.print("{");
                        Date[] dateArray = (Date[])tuple.get(j);
                        for (int k = 0; k < dateArray.length; k++) {
                            System.out.print(dateArray[k] + " ");
                        }
                        System.out.print("}");
                    } else if (tuple.get(j).getClass().getName() == "java.lang.String") {
                        String symb = (String)tuple.get(j);
                        System.out.print(symb);
                    }
                    if (j != columns.length - 1 )
                        System.out.print(", ");
                }
                System.out.println();
            }
            System.out.println();
        }
    }
}

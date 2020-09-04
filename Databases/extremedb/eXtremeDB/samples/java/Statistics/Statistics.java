import com.mcobject.extremedb.*;
import com.mcobject.extremedb.Database.Device.Kind;
import com.mcobject.extremedb.Database.TransactionType;
import com.mcobject.extremedb.Statistic.ClassStat;
import com.mcobject.extremedb.Statistic.IndexStat;
import java.io.Console;

@Persistent
class HashedObj {
    @Indexable(unique = true, type = Database.IndexType.Hashtable, initSize = 1000)
    public int key;
}

@Persistent
class TriedObj {
    @Indexable(unique = true, type = Database.IndexType.BTree)
    public int key;
}

@Persistent(list = true)
class ListedObj {
    public int key;
}

@Persistent(autoid = true)
class AutoidedObj {
    public int key;
}

@Persistent
class OIDedObj {
    public int key;
}

@Persistent(list = true, autoid = true)
class ComplexObj {
    @Indexable(unique = true, type = Database.IndexType.Hashtable, initSize = 1000)
    // [Indexable(Unique=true, Type = Database.IndexType.BTree)]
    public int key;
}

public class Statistics {
    static final int PAGE_SIZE = 128;
    static final int DISK_PAGE_SIZE = 4096;
    static final int DATABASE_SIZE = 16 * 1024 * 1024;
    static final int DISK_CACHE_SIZE = 8 * 1024 * 1024;

    public static void main(String[] args) throws Exception {
        Database db;
        int config = 0;

        Database.Parameters params = new Database.Parameters();
        params.memPageSize = PAGE_SIZE;
        //
        // Get config parameters from the command line
        for (String s : args)
        {
            if ("disk".equals(s))  config |= Database.MCO_CFG_DISK_SUPPORT;
            if ("shm".equals(s))   config |= Database.MCO_CFG_SHARED_MEMORY;
            if ("debug".equals(s)) config |= Database.MCO_CFG_DEBUG_LIBRARY;
            if ("mvcc".equals(s))  config |= Database.MCO_CFG_MVCC_TRANSACTION_MANAGER;
            if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        params.classes = new Class[] {
            HashedObj.class,
            TriedObj.class,
            ListedObj.class,
            AutoidedObj.class,
            OIDedObj.class,
            ComplexObj.class
        };

        Database.Device[] devs;
        if ((config & Database.MCO_CFG_DISK_SUPPORT) != 0)
        {
            java.io.File file = new java.io.File("jstatistics.dbs");
            file.delete();
            file = new java.io.File("jstatistics.log");
            file.delete();

            params.diskPageSize = DISK_PAGE_SIZE;
            params.diskClassesByDefault = true;
            devs = new Database.Device[] {
                new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE),
                new Database.PrivateMemoryDevice(Database.Device.Kind.DiskCache, DISK_CACHE_SIZE),
                new Database.FileDevice(Kind.Data, "jstatistics.dbs"),
                new Database.FileDevice(Kind.TransactionLog, "jstatistics.log")
            };
        } else { // in-mem only
            devs = new Database.Device[] {
                new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE)
            };
        }

        try
        {
            System.out.println("Sample 'Statistics' demonstrates statistics retrieval from" +
                                    (((config & Database.MCO_CFG_DISK_SUPPORT) != 0)? " disk" : " in-memory") + " database");

            db = new Database(config);
            // Open and connect to database
            db.open("Statistics", params, devs);

            db.generateMcoFile("jstatistics.mco"); // Generate database schema file

            Connection con = new Connection(db);

            if ((config & Database.MCO_CFG_DISK_SUPPORT) != 0)
            {
                con.setCommitPolicy(Database.CommitPolicy.NoSync);
            }

            LoadData(con);

            System.out.println();

            System.out.format("Total pages:    %d", con.getTotalPages());
            System.out.println();
            System.out.format("Free pages:     %d", con.getFreePages());
            System.out.println();
            System.out.format("Page size:      %d", con.getDbPageSize());
            System.out.println();
            
            con.startTransaction(TransactionType.ReadOnly);
            for (int cls = 1; cls <= params.classes.length; cls++) {
                ClassStat stat = con.getClassStat(cls);

                System.out.format("Class %d: %d objects, %d pages, %d blob pages",
                        cls, stat.ObjectsCount, stat.CorePageCount, stat.BlobPages);
                System.out.println();
            }
            con.rollbackTransaction();

            System.out.println();

            con.startTransaction(TransactionType.ReadOnly);

            short idx_num = con.getIndexStatCount();

            for (short idx = 0; idx < idx_num; idx++) {
                Statistic.IndexStat stat = con.getIndexStat(idx);

                System.out.format("%s: %d key(s) %d page(s) ", stat.Label, stat.KeysCount, stat.PagesCount);
                System.out.println();
            }

            con.rollbackTransaction();

            if ((config & Database.MCO_CFG_DISK_SUPPORT) != 0)
            {
                Statistic.DiskInfo di = con.getDiskInfo();
                System.out.format("Disk file size: %d, Used: %d, Transaction log file size: %d",
                                di.DataFileSize, di.LogFileSize, di.LogFileSize);
                System.out.println();
            }

            // Disconnect and close database, stop runtime
            con.disconnect();
            db.close();
        } catch (DatabaseError dbe) {
            if (dbe.errorCode == 13)
                System.out.println("Delete database files jstatistics.dbs and jstatistics.log before start");
            else
                System.out.println("Error: " + dbe);
        }
        System.out.println("Press enter to finish");
        if (System.console() != null) {
            System.console().readLine();
        }
    }

    private static void LoadData(Connection con) {
        System.out.println("Statistics.LoadData()");
        for (int i = 0; i < 123; i++) {
            con.startTransaction(TransactionType.ReadWrite);
            HashedObj o = new HashedObj();
            o.key = i;
            con.insert(o);
            con.commitTransaction();
        }

        for (int i = 0; i < 255; i++) {
            con.startTransaction(TransactionType.ReadWrite);
            TriedObj o = new TriedObj();
            o.key = i;
            con.insert(o);
            con.commitTransaction();
        }

        for (int i = 0; i < 76; i++) {
            con.startTransaction(TransactionType.ReadWrite);
            ListedObj o = new ListedObj();
            o.key = i;
            con.insert(o);
            con.commitTransaction();
        }

        for (int i = 0; i < 333; i++) {
            con.startTransaction(TransactionType.ReadWrite);
            AutoidedObj o = new AutoidedObj();
            o.key = i;
            con.insert(o);
            con.commitTransaction();
        }

        for (int i = 0; i < 26; i++) {
            con.startTransaction(TransactionType.ReadWrite);
            OIDedObj o = new OIDedObj();
            o.key = i;
            con.insert(o);
            con.commitTransaction();
        }

        for (int i = 0; i < 192; i++) {
            con.startTransaction(TransactionType.ReadWrite);
            ComplexObj o = new ComplexObj();
            o.key = i;
            con.insert(o);
            con.commitTransaction();
        }
    }
}

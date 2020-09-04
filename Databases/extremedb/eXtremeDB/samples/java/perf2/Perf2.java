import com.mcobject.extremedb.*;

// Sample 'Perf2' demonstrates eXtremeDB JNI performance for basic operations
// and how to open multiple databases.
// First N objects are inserted into a class, a hash index is created
// on insertion, then used for searches.  Then a tree index is built
// and searches are performed by key then sequentially.
// Finally, the tree index is removed and all objects are deleted
// one-by-one.  Each insert and delete is done in a separate
// transaction, so that the commit time is included in measurements.


public class Perf2
{
    @Persistent
    static class Record
    {        
        @Indexable(unique=true, type=Database.IndexType.Hashtable, initSize=100000)
        int hkey;

        @Indexable(unique=true, type=Database.IndexType.BTree)
        int tkey;
    }


    static final int PAGE_SIZE = 128;
    static final int DISK_PAGE_SIZE = 4096;
    static final int DISK_CACHE_SIZE = 8*1024*1024;
    static final int DATABASE_SIZE = 16*1024*1024;

    static final int N_RECORDS = 100000;

    public static void main(String[] args) throws Exception
    { 
        int i;
        Database db, db2;
        boolean onDisk = false;
        Database.Parameters params = new Database.Parameters();
        for (String s : args) {
            if ("disk".equals(s))  onDisk = true;
            if (s.startsWith("license:")) params.license_key = s.substring(8);
        }
        params.memPageSize = PAGE_SIZE;
        params.classes = new Class[]{Record.class};
        if (onDisk) {
            db = new Database(Database.MCO_CFG_DISK_SUPPORT);
            params.diskPageSize = DISK_PAGE_SIZE;
            params.diskClassesByDefault = true;
            db.open("perf", params, new Database.Device[] {
                new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE),
                new Database.PrivateMemoryDevice(Database.Device.Kind.DiskCache, DISK_CACHE_SIZE),
                new Database.FileDevice(Database.Device.Kind.Data, "perf.dbs"),
                new Database.FileDevice(Database.Device.Kind.TransactionLog, "perf.log")});

            db2 = new Database(Database.MCO_CFG_DISK_SUPPORT);
            params.diskPageSize = DISK_PAGE_SIZE;
            params.diskClassesByDefault = true;
            db2.open("perf2", params, new Database.Device[] {
                new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE),
                new Database.PrivateMemoryDevice(Database.Device.Kind.DiskCache, DISK_CACHE_SIZE),
                new Database.FileDevice(Database.Device.Kind.Data, "perf2.dbs"),
                new Database.FileDevice(Database.Device.Kind.TransactionLog, "perf2.log")});
        } else {
            db = new Database();
            db.open("perf", params, DATABASE_SIZE);

            db2 = new Database();
            db2.open("perf2", params, DATABASE_SIZE);
        }
        Connection con = new Connection(db);
        Connection con2 = new Connection(db2);

        if (onDisk)
        {
            con.setCommitPolicy(Database.CommitPolicy.NoSync);
            con2.setCommitPolicy(Database.CommitPolicy.NoSync);
        }

        ////////////////////////////////////////////////////// 
        // Insert some data in the database
        ////////////////////////////////////////////////////// 
        long start = System.currentTimeMillis();
        for (i = 0; i < N_RECORDS; i++) {
            con.startTransaction(Database.TransactionType.ReadWrite);
            Record rec = new Record();
            rec.hkey = rec.tkey = i*2;
            con.insert(rec);
            con.commitTransaction();
        }
        System.out.println("Elapsed time for inserting " + N_RECORDS + " records: " + (System.currentTimeMillis() - start) + " milliseconds");

        start = System.currentTimeMillis();
        for (i = 0; i < N_RECORDS; i++) {
            con2.startTransaction(Database.TransactionType.ReadWrite);
            Record rec = new Record();
            rec.hkey = rec.tkey = i*2;
            con2.insert(rec);
            con2.commitTransaction();
        }
        System.out.println("Elapsed time for inserting " + N_RECORDS + " records: " + (System.currentTimeMillis() - start) + " milliseconds");

        ////////////////////////////////////////////////////// 
        // Hash search
        ////////////////////////////////////////////////////// 
        start = System.currentTimeMillis();
        con.startTransaction(Database.TransactionType.ReadOnly);
        Cursor<Record> hc = new Cursor<Record>(con, Record.class, "hkey");
        Cursor<Record> tc = new Cursor<Record>(con, Record.class, "tkey");
        for (i = 0; i < N_RECORDS*2; i++) {
            Record rec = hc.find(i);
            if (i % 2 == 0) { 
                assert(rec != null && rec.hkey == i && rec.tkey == i);
            } else { 
                assert(rec == null);
            }
        }
        con.commitTransaction();
        System.out.println("Elapsed time for " + N_RECORDS*2 + " hash searches: " + (System.currentTimeMillis() - start) + " milliseconds");

        ////////////////////////////////////////////////////// 
        // Tree search
        ////////////////////////////////////////////////////// 
        start = System.currentTimeMillis();
        con.startTransaction(Database.TransactionType.ReadOnly);
        for (i = 0; i < N_RECORDS*2; i++) {
            Record rec = tc.find(i);
            if (i % 2 == 0) { 
                assert(rec != null && rec.hkey == i && rec.tkey == i);
            } else { 
                assert(rec == null);
            }
        }
        con.commitTransaction();
        System.out.println("Elapsed time for " + N_RECORDS*2 + " tree searches: " + (System.currentTimeMillis() - start) + " milliseconds");

        ////////////////////////////////////////////////////// 
        // Cursor movements 
        ////////////////////////////////////////////////////// 
        start = System.currentTimeMillis();
        con.startTransaction(Database.TransactionType.ReadOnly);        
        i = 0;
        for (Record rec = tc.first(); rec != null; rec = tc.next()) {
            assert(rec.hkey == i*2 && rec.tkey == i*2);
            i += 1;
        }
        assert(i == N_RECORDS);
        con.commitTransaction();
        System.out.println("Elapsed time for traversal " + N_RECORDS + " records: " + (System.currentTimeMillis() - start) + " milliseconds");

         ////////////////////////////////////////////////////// 
        // Search & Delete operation
        ////////////////////////////////////////////////////// 
        start = System.currentTimeMillis();
        for (i = 0; i < N_RECORDS*2; i++) {
            con.startTransaction(Database.TransactionType.ReadWrite);
            Record rec = hc.find(i);
            if (i % 2 == 0) { 
                assert(rec != null && rec.hkey == i && rec.tkey == i);
                hc.remove();
            } else { 
                assert(rec == null);
            }
            con.commitTransaction();
        }
        System.out.println("Elapsed time for removing " + N_RECORDS + " records: " + (System.currentTimeMillis() - start) + " milliseconds");
        hc.close();
        tc.close();

        start = System.currentTimeMillis();
        for (i = 0; i < N_RECORDS*2; i++) {
            con2.startTransaction(Database.TransactionType.ReadWrite);
            Cursor<Record> hc2 = new Cursor<Record>(con2, Record.class, "hkey");
            Record rec = hc2.find(i);
            if (i % 2 == 0) { 
                assert(rec != null && rec.hkey == i && rec.tkey == i);
                hc2.remove();
            } else { 
                assert(rec == null);
            }
            con2.commitTransaction();
            hc2.close();
        }
        System.out.println("Elapsed time for removing " + N_RECORDS + " records: " + (System.currentTimeMillis() - start) + " milliseconds");

        con2.disconnect();
        db2.close();

        con.disconnect();
        db.close();
    }
}

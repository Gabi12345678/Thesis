import com.mcobject.extremedb.*;

@Persistent // Class will be stored in eXtremeDB database
class RecordObj
{
    @Indexable(type = Database.IndexType.BTree, unique = true)
    public int key;
};

public class TLogCheckpoint
{
    static final int PAGE_SIZE = 128;
    static final int DISK_PAGE_SIZE = 4096;
    static final int DISK_CACHE_SIZE = 8*1024*1024;
    static final int DATABASE_SIZE = 16*1024*1024;
    static final String TL_LOG_FILE = "tlogsnapshot_tl_cs.log";
    static final String DBIMAGE_FILE = "tlogsnapshotdb_cs.bak";

    static void UseDatabase(Connection con) throws DatabaseError
    {
        int i, keys = 0, last_key = 0;

        // Increase last key by one
        con.startTransaction(Database.TransactionType.ReadWrite);
        Cursor<RecordObj> cursor = new Cursor<RecordObj>(con, RecordObj.class, "key");

        RecordObj rec = null;

        try
        {
            rec = cursor.last();
        }
        catch (DatabaseError dbe)
        {
            if (dbe.errorCode != 10)
            {   // Code 10 means empty database.
                throw dbe;
            }
        }

        if (rec != null)
        {
            last_key = rec.key + 1;
            // Store last key value to add further objects
            rec.key = last_key;
            cursor.update();
            keys = 1;
        }

        cursor.close(); // Close cursor
        con.commitTransaction();

        if (keys != 0)
            System.out.println("Updated latest object with key " + (last_key - 1));

        // Add 100 new keys
        con.startTransaction(Database.TransactionType.ReadWrite);
        for (i = 0; i < 100; i++)
        {
            rec = new RecordObj();
            rec.key = ++last_key;
            con.insert(rec);
        }
        con.commitTransaction();
        System.out.println("Added " + i + " objects to database");
    }

    public static void main(String[] args) throws DatabaseError, java.io.IOException, java.lang.InterruptedException
    { 
        Database db;
        boolean generateSchema = false;
        Database.Device[] devs;
        Database.Parameters parameters = new Database.Parameters();

        parameters.memPageSize = PAGE_SIZE; // Memory page size
        parameters.classes = new Class[] { RecordObj.class }; // List of classes which should be stored in eXtremeDB database. The classes must use Persistent attribute
        parameters.maxConnections = 10;

        // Get config parameters from the command line
        for (String s : args) {
            if ("schema".equals(s)) generateSchema = true;
            if (s.startsWith("license:")) parameters.license_key = s.substring(8);
        }

        System.out.println("Create database");

        int mode = Database.MCO_CFG_DEBUG_LIBRARY | Database.MCO_CFG_TL_SUPPORT;
        // Create Database object
        db = new Database(mode);

        try
        {
            devs = new Database.Device[1];
            devs[0] = new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE);
            db.open("tlogdb", parameters, devs);
        }
        catch (DatabaseError dbe)
        {
            System.out.format("Connect database error %d\n", dbe.errorCode);
            return;
        }

        if (generateSchema)
        {
            System.out.println("Generate database schema file for native iterator");
            db.generateMcoFile("schema.mco");

            System.out.println("Close database");
            db.close();
            return;
        }

        // Connect to the database
        Connection con = new Connection(db);
        System.out.println("Connect to the database");

        // Initialize and run logging
        LogWriter logw;
        try
        {
            logw = new LogWriter(con, TL_LOG_FILE);
            LogWriter.LogParams lparms = new LogWriter.LogParams();
            lparms.diskPageSize = DISK_PAGE_SIZE;
            lparms.flags = LogWriter.LF_CRC;
            logw.start(lparms);
            System.out.println("Transactions logging started.");
        }
        catch (DatabaseError dbe)
        {
            System.out.format("Error. Can't start logging %d\n", dbe.errorCode);
            return;
        }

        System.out.println("Start working with the database");

        // Log some database activity
        UseDatabase(con);

        // Store database snapshot together with log truncation
        logw.saveSnapshot(DBIMAGE_FILE);
        System.out.println("Database snapshot image was saved and transaction log has been truncated");

        // Log more transactions over saved snapshot
        UseDatabase(con);

        logw.stop();
        System.out.println("Transactions logging stopped");

        con.disconnect();
        System.out.println("Disconnect from the database");

        db.close();
        System.out.println("Close database");

        System.out.println("Press enter to finish");
        if (System.console() != null) {
            System.console().readLine();
        }
    }
}

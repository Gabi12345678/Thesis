import com.mcobject.extremedb.*;

@Persistent // Class will be stored in eXtremeDB database
class RecordObj
{
    @Indexable(type = Database.IndexType.BTree, unique = true)
    public int key;
};

public class TLogDBSource
{
    static final int PAGE_SIZE = 128;
    static final int PIPE_PAGE_SIZE = 4096;
    static final int DATABASE_SIZE = 16*1024*1024;
    static final int PIPE_SIZE = 1*1024*1024;
    static final int WAIT_TIME = 100;

    static void UseDatabase(Connection con) throws DatabaseError
    {
        int i, key;

        // Add 100 new keys
        con.startTransaction(Database.TransactionType.ReadWrite);
        for (i = 0, key = 0; i < 100; i++, key++)
        {
            RecordObj rec = new RecordObj();
            rec.key = i;
            con.insert(rec);
        }
        con.commitTransaction();
        System.out.println("Added " + i + " objects to database");

        // Remove last 10 keys
        con.startTransaction(Database.TransactionType.ReadWrite);        
        for (i = 0, key = 99; i < 10; i++, key--)
        {
            Cursor<RecordObj> cursor = new Cursor<RecordObj>(con, RecordObj.class, "key");
            RecordObj rec = cursor.find(key);
            if (rec != null)
                cursor.remove();
            else
                System.out.println("Onject with key " + key + " was not found");
            cursor.close();
        }
        con.commitTransaction();
        System.out.println("Remove " + i + " objects from database");
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
        parameters.dictionaryNoSort = true;

        // Get config parameters from the command line
        for (String s : args) {
            if ("schema".equals(s)) generateSchema = true;
            if (s.startsWith("license:")) parameters.license_key = s.substring(8);
        }

        System.out.println("Create database");

        int mode = Database.MCO_CFG_SHARED_MEMORY | Database.MCO_CFG_DEBUG_LIBRARY | Database.MCO_CFG_TL_SUPPORT;
        // Create Database object
        db = new Database(mode);

        try
        {
            devs = new Database.Device[2];
            devs[0] = new Database.SharedMemoryDevice(Database.Device.Kind.Data, "tlogdb-db", 0, DATABASE_SIZE);
            devs[1] = new Database.SharedMemoryDevice(Database.Device.Kind.PipeBuffer, "tlogdb-pipe", 0, PIPE_SIZE);
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
            logw = new LogWriter(con, "");
            LogWriter.LogParams lparms = new LogWriter.LogParams();
            lparms.diskPageSize = PIPE_PAGE_SIZE;
            lparms.flags = LogWriter.LF_ITERABLE | LogWriter.LF_PIPE;
            logw.start(lparms);
            System.out.println("Transactions logging started. Waiting for reader side...");
        }
        catch (DatabaseError dbe)
        {
            System.out.format("Error. Can't start logging %d\n", dbe.errorCode);
            return;
        }

        LogWriter.LogInfo inf;
        do {
            inf = logw.getInfo();
            Thread.sleep(WAIT_TIME);
        } while (inf.pipeUsedSize != 0);

        System.out.println("Start working with the database");

        UseDatabase(con);

        logw.stop();
        System.out.println("Transactions logging stopped");

        con.disconnect();
        System.out.println("Disconnect from the database");

        boolean stop_flag = false;
        
        do {
            try 
            {
                db.close();
                stop_flag = true;
            }
            catch (DatabaseError dbe)
            {
                if (dbe.errorCode == 69 /* MCO_E_OPENED_SESSIONS */) {
                    // Wait while all connections to the database are closed
                    Thread.sleep(WAIT_TIME);
                } else {
                    System.out.format("Error. Can't start logging %d\n", dbe.errorCode);
                    return;
                }
            }
        } while (!stop_flag);
        
        System.out.println("Close database");

        System.out.println("Press enter to finish");
        if (System.console() != null) {
            System.console().readLine();
        }
    }
}

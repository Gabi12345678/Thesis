import java.io.IOException;

import com.mcobject.extremedb.*;

@Persistent // Class will be stored in eXtremeDB database
class SniffObjCtx
{
    @Indexable(type = Database.IndexType.BTree, unique = true)
    public int i4;
};

public class ConnectCtx
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 1024*1024;

    public static void main(String[] args) throws DatabaseError
    { 
        // Display program description
        System.out.println("Sample 'ConnectCtx' connects to the database opened by Java application\n" +
                           "'CreateDBSniffer' or by native application 'createdb_sniffer' then it throws " +
                           "an exception to simulate process crashing.\n");

        Database db;
        Database.Parameters parameters = new Database.Parameters();
        Database.Device devs[];

        int mode = Database.MCO_CFG_SHARED_MEMORY | Database.MCO_CFG_MVCC_TRANSACTION_MANAGER;

        // Get config parameters from the command line
        for (String s : args) {
            if (s.startsWith("license:")) parameters.license_key = s.substring(8);
            if ("debug".equals(s)) mode |= Database.MCO_CFG_DEBUG_LIBRARY;
        }
        parameters.memPageSize = PAGE_SIZE; // Memory page size
        parameters.classes = new Class[] { SniffObjCtx.class }; // List of classes which should be stored in eXtremeDB database. The classes must use Persistent attribute
        parameters.maxConnections = 10;
        parameters.dictionaryNoSort = true;

        // Create Database object
        db = new Database(mode);
        parameters.connectionContextSize = Sniffer.getProcessIDSize(db);
        try
        {
            devs = new Database.Device[1];
            devs[0] = new Database.SharedMemoryDevice(Database.Device.Kind.Data, "opendb-db", 0, DATABASE_SIZE);

            // Open database using shared memory. No device description should be passed to runtime.
            System.out.println("Open database");
            db.open("snifferdb", parameters, devs);

            db.generateMcoFile("snifferdb_ctx.mco");

            System.out.println("New database instance created");
        }
        catch (DatabaseError dbe)
        {
            if (dbe.errorCode == 66) { // Code 66 means duplicate instance. Valid case for SHM configuration
                System.out.println("Database opened");
            } else {
                if (dbe.errorCode == 62) {
                    System.out.println("eXtremeDB assembly is not compatible with option 'shm'. Please replace reference to assembly with shared memory functionality");
                } else if (dbe.errorCode == 620000) {
                    System.out.println("eXtremeDB assembly is not compatible with option 'disk'. Please replace reference to assembly with disk manager functionality");
                }

                System.out.println(dbe);
                return;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }


        int processID = Sniffer.getCurrentProcessID();

        System.out.format("My PID=%d\nConnect to a DB\n", processID);

        Connection con = db.connect(Sniffer.processIDtoBytes(db, processID)); // connect to the database
        System.out.println("Connected");

        for (int i = 0; i < 10; i++)
        {
            con.startTransaction(Database.TransactionType.ReadWrite);
            SniffObjCtx rec = new SniffObjCtx();
            rec.i4 = i * 3;
            con.insert(rec);
            con.commitTransaction();
        }

        System.out.println("Insert complete");

        System.out.println("Simulating program crash by using unhandled exception");

        throw new DatabaseError(0, "");

        /* This code is never reached, database disconnect skipped */
    }
}

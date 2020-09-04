import java.io.IOException;

import com.mcobject.extremedb.*;

@Persistent // Class will be stored in eXtremeDB database
class SniffObj
{
    @Indexable(type = Database.IndexType.BTree, unique = true)
    public int i4;
};

public class CreateDBSniffer
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 1024*1024;
    static final int SNIFFER_INTERRVAL = 100;

    Database db;
    int processID;

    public static void main(String[] args) throws Exception
    {
        new CreateDBSniffer(args); 
    }

    CreateDBSniffer(String[] args) throws Exception
    { 
        // Display program description
        System.out.println("Sample 'CreateDBSniffer' creates a database using shared memory and runs a sniffer functionality.");
        System.out.println("\n\tStart CreateDBSniffer to create a database Then start\n" +
                        "\tan instance of ConnectCtx program.\n" +
                        "\tThe second process will cause a segmentation fault.  \n" +
                        "\tThe sniffer loop in the first process will detect and report \n" +
                        "\tthe 'dead' connection in the second process.");

        Database.Parameters parameters = new Database.Parameters();
        Database.Device devs[];

        int mode = Database.MCO_CFG_SHARED_MEMORY | Database.MCO_CFG_MVCC_TRANSACTION_MANAGER;

        // Get config parameters from the command line
        for (String s : args) {
            if (s.startsWith("license:")) parameters.license_key = s.substring(8);
            if ("debug".equals(s)) mode |= Database.MCO_CFG_DEBUG_LIBRARY;
        }
        parameters.memPageSize = PAGE_SIZE; // Memory page size
        parameters.classes = new Class[] { SniffObj.class }; // List of classes which should be stored in eXtremeDB database. The classes must use Persistent attribute
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
            System.out.println("Database opened");
        }
        catch (DatabaseError dbe)
        {
            if (dbe.errorCode == 66) { // Code 66 means duplicate instance.
                System.out.println("Database already exists");
            } else if (dbe.errorCode == 62) {
                System.out.println("eXtremeDB assembly is not compatible with option 'shm'. Please replace reference to assembly with shared memory functionality");
            } else if (dbe.errorCode == 620000) {
                System.out.println("eXtremeDB assembly is not compatible with option 'disk'. Please replace reference to assembly with disk manager functionality");
            }
            System.out.println(dbe);
            return;
        }

        processID = Sniffer.getCurrentProcessID();

        Connection con = db.connect(Sniffer.processIDtoBytes(db, processID)); // connect to the database
        System.out.println("Connected to the database.");

        System.out.println("\nNow start Java application ConnectCtx to simulate process crashing...");

        SnifferThread snifferThread = new SnifferThread(SNIFFER_INTERRVAL);
        snifferThread.start();

        while (snifferThread.getRepairCount() == 0) {
            try { Thread.sleep(100); } catch (Exception e) {};
        }

        // database now has some records - print them
        con.startTransaction(Database.TransactionType.ReadOnly);
        Cursor<SniffObj> cursor = new Cursor<SniffObj>(con, SniffObj.class, "i4");
        System.out.println("All objects             : ");
        for (SniffObj o : cursor) {
            System.out.println("Object: i4=" + o.i4);
        }
        con.commitTransaction();

        snifferThread.setStopFlag();
        snifferThread.join();

        con.disconnect();
        System.out.println("Disconnect from the database");

        db.close();
        System.out.println("Close database");
    }

    class SnifferThread extends Thread implements Sniffer.ConnIterator {
        int interval;
        int repair;
        boolean stop;
        Sniffer sniffer;

        SnifferThread(int interval) {
            this.interval = interval;
            stop = false;
        }

        public void run() {
            Connection con = new Connection(db, Sniffer.processIDtoBytes(db, processID));
            sniffer = new Sniffer(con);

            while (! stop) {
                sniffer.iterateConnections(this, Sniffer.MCO_SNIFFER_INSPECT_ACTIVE_CONNECTIONS);
                try { Thread.sleep(interval); } catch (Exception e) {};
            }

            con.disconnect();
        }

        public void setStopFlag() {
            stop = true;
        }

        public int getRepairCount()
        {
            return repair;
        }

        public boolean isConnectionAlive(byte[] context, long transNo)
        {
            int pid = sniffer.bytesToProcessID(context);
            boolean isAlive = Sniffer.isProcessAlive(pid);
            if ( !isAlive ) {
                System.out.println("Process " + pid + " has crashed");
                repair++;
            }
            return isAlive;
        }
    }
}
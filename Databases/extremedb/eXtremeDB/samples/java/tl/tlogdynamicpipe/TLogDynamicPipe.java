import java.util.Random;
import com.mcobject.extremedb.*;

@Persistent // class will be stored in eXtremeDB database
class Record
{
    @Indexable(type = Database.IndexType.Hashtable, unique = true)
    public int key;
    public int val;
    public String str;
};

public class TLogDynamicPipe
{
    static final int PAGE_SIZE = 128;
    static final int DATABASE_SIZE = 16*1024*1024;

    static final int N_ITERATOR_THREADS   = 4; // number of readers
    static final int PIPE_SEGMENT_SIZE    = 1024;
    static final int PIPE_SEGMENT_ALIGNMENT = 1024*1024;
    static final int BUFFER_SEGMENT_SIZE  = 1024*1024;
    static final int MAX_RUN_TIME         = 10*1000; // 10 seconds 

    Database db;
    int config;
    int processID;

    public static void main(String[] args) throws Exception
    {
        new TLogDynamicPipe(args); 
    }

    TLogDynamicPipe(String[] args) throws Exception
    {
        boolean primaryDatabase = true;
        Database.Parameters parameters = new Database.Parameters();

        // get config parameters from the command line
        for (String s : args) {
            if ("shm".equals(s))   config |= Database.MCO_CFG_SHARED_MEMORY;
            if ("debug".equals(s)) config |= Database.MCO_CFG_DEBUG_LIBRARY;
            if ("mvcc".equals(s))  config |= Database.MCO_CFG_MVCC_TRANSACTION_MANAGER;
            if (s.startsWith("license:")) parameters.license_key = s.substring(8);
        }

        parameters.memPageSize = PAGE_SIZE; // memory page size
        parameters.classes = new Class[] { Record.class }; // list of classes which should be stored in eXremeDB database. The classes must use Persistent attribute
        parameters.maxConnections = 16 + N_ITERATOR_THREADS * 3; // up to 3 processes in the shared-memory configuration

        // Create Database object
        db = new Database(config | Database.MCO_CFG_TL_SUPPORT); // initialize runtime with TL support
        Database.RuntimeInfo ri = db.getRunTimeInfo();
        long hint = ri.direct_pointers_supported ? 0x20000000 : 0;

        // example of how to apply connection context if sniffer is used
        // Sniffer.getProcessIDSize is the same as Database.RuntimeInfo.mco_process_t
        if ((config & Database.MCO_CFG_SHARED_MEMORY) != 0)
            parameters.connectionContextSize = Sniffer.getProcessIDSize(db);

        db.setRuntimeOption(Database.RT_MAX_DYNAMIC_PIPES, N_ITERATOR_THREADS * 3); // for 3 processes in the shared-memory configuration
        db.setRuntimeOption(Database.RT_OPTION_MARK_LAST_OBJ, 1); // enable invocation of LogReader.LogIterator.endOfTransaction marker

        Database.Device devs[] = new Database.Device[1];
        if ((config & Database.MCO_CFG_SHARED_MEMORY) != 0) {
            devs[0] = new Database.SharedMemoryDevice(Database.Device.Kind.Data, "tldynpipe-db", hint, DATABASE_SIZE);
        } else {
            devs[0] = new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE);
        }

        try {
            db.open("tlogdb", parameters, devs);
            System.out.println("Database created");
        } catch (DatabaseError dbe) {
            if (dbe.errorCode != 66) { // code 66 means duplicate instance. Valid case for SHM configuration
                throw dbe;
            }
            System.out.println("Database already exists");
            primaryDatabase = false;
        }

        // connection context useful if sniffer functionality is used
        processID = Sniffer.getCurrentProcessID();

        // connect to the database
        Connection con;

        if ((config & Database.MCO_CFG_SHARED_MEMORY) == 0)
            con = new Connection(db);
        else
            con = new Connection(db, Sniffer.processIDtoBytes(db, processID)); // example of how to apply connection context if sniffer is used    

        System.out.println("Connect to the database");

        LogWriter logw = null;
        if (primaryDatabase) {
            // initialize and run logging
            logw = new LogWriter(con, null);
            LogWriter.LogParams lparms = new LogWriter.LogParams();
            lparms.diskPageSize = 4096;
            lparms.flags = LogWriter.LF_PIPE | LogWriter.LF_DYNAMIC_PIPE | LogWriter.LF_ITERABLE | LogWriter.LF_SYNC_INSTANTLY;
            logw.start(lparms);
            System.out.println("Transactions logging started.");
        }

        // start iterator threads
        iterThreads = new IteratorThread[N_ITERATOR_THREADS];
        for (int i = 0; i < N_ITERATOR_THREADS; ++i) {
            iterThreads[i] = new IteratorThread(i, hint != 0 ? hint + DATABASE_SIZE + i * (PIPE_SEGMENT_ALIGNMENT + BUFFER_SEGMENT_SIZE) : 0);
            iterThreads[i].start();
        }

        if (primaryDatabase) {
            // some database activity
            UseDatabase(con);
            // stop logging
            logw.stop();
        }

        // wait for iterator threads to complete
        int n_extends = 0, n_deferred_detachs = 0, n_immediate_detachs = 0;
        for (int i = 0; i < N_ITERATOR_THREADS; ++i) {
            iterThreads[i].join();
            n_extends           += iterThreads[i].n_extends;
            n_deferred_detachs  += iterThreads[i].n_deferred_detachs;
            n_immediate_detachs += iterThreads[i].n_immediate_detachs;
            System.out.println("Thread #" + i + " iterated " + iterThreads[i].n_connects + " times, observed " + iterThreads[i].n_objects + " objects");
        }
        System.out.println("Number of mco_db_extend()/mco_db_detach() : " + n_extends);
        System.out.println("Number of deferred  detaches              : " + n_deferred_detachs);
        System.out.println("Number of immediate detaches              : " + n_immediate_detachs);

        con.disconnect();
        if (primaryDatabase) {
            try { Thread.sleep(500); } catch (Exception e) {}; // Wait for secondary processes to disconnect
            db.close();
        }
    }

    static void UseDatabase(Connection con) throws DatabaseError
    {
        Record rec;
        // create object with key == 0
        con.startTransaction(Database.TransactionType.ReadWrite);
        rec = new Record();
        rec.key = 0;
        rec.val = 100;
        con.insert(rec);
        con.commitTransaction();

        // increase rec.val by 1
        for (long startTime = System.currentTimeMillis(); System.currentTimeMillis() - startTime < MAX_RUN_TIME; ) {
            con.startTransaction(Database.TransactionType.ReadWrite);
            Cursor<Record> cursor = new Cursor<Record>(con, Record.class, "key");
            rec = cursor.find(0);
            if (rec != null) {
                rec.val++;
                cursor.update();
            }
            cursor.close();
            con.commitTransaction();
            try { Thread.sleep(1); } catch (Exception e) {};
            if (rec.val % 100 == 0) System.out.print(".");
        }
        System.out.println("\nNumber of updates : " + (rec.val - 100) + "\n");
    }

    class StopperThread extends Thread {
        LogReader logr;
        boolean action_flag;
        boolean skip_flag;

        StopperThread(LogReader logr)
        {
            this.logr = logr;
            action_flag = false;
            skip_flag = false;
        }

        public void run() {
            while (!action_flag) {
                try {
                    Thread.sleep(200);
                } catch (Exception e) {};
            }
            if (!skip_flag)
            {
                try
                {
                    logr.stopPipeIteration();
                }
                catch (DatabaseError de)
                {
                    if (de.errorCode != 603) // skip expected error code MCO_E_TL_NOT_STARTED
                        throw de;
                }
            }
        }

        public void doAction() {
            action_flag = true;
        }

        public void skipAction() {
            skip_flag = true;
            action_flag = true;
        }
    }

    class IteratorThread extends Thread implements LogReader.LogIterator {
        int  id;
        long hint;
        int  n_instance_limits;
        int  n_extends;
        int  n_deferred_detachs;
        int  n_immediate_detachs;
        int  n_objects;
        int  n_connects;
        int  last_value;
        int  n_iterations;
        int  n_passes;
        StopperThread stop_thread;

        IteratorThread(int id, long hint) {
            this.id = id;
            this.hint = hint;
        }

        public void run() {
            boolean stop = false;
            Connection con;

            if ((config & Database.MCO_CFG_SHARED_MEMORY) == 0)
                con = new Connection(db);
            else
                con = new Connection(db, Sniffer.processIDtoBytes(db, processID)); // example of how to apply connection context if sniffer is used

            Random rnd = new Random();
            while (! stop) {
                // create new pipe device
                Database.Device pipe;
                if ((config & Database.MCO_CFG_SHARED_MEMORY) != 0) {
                    String segment_name = java.util.UUID.randomUUID().toString();
                    segment_name += id;
                    pipe = new Database.SharedMemoryDevice(Database.Device.Kind.PipeBuffer, 
                                    segment_name, hint, PIPE_SEGMENT_SIZE);
                } else {
                    pipe = new Database.PrivateMemoryDevice(Database.Device.Kind.PipeBuffer, PIPE_SEGMENT_SIZE);
                }
                // attach the pipe to the database
                try {
                    if ((config & Database.MCO_CFG_SHARED_MEMORY) == 0)
                        db.extend(pipe);
                    else
                        db.extend(pipe, Sniffer.processIDtoBytes(db, processID)); // example of how to apply connection context if sniffer is used
                } catch (DatabaseError de) {
                    if (de.errorCode == 83) { // MCO_E_INSTANCE_LIMIT
                        try { Thread.sleep(100); } catch (Exception e) {};
                        n_instance_limits++;
                        continue;
                    }
                    throw de;
                }
                n_extends++;

                n_iterations = 100 + rnd.nextInt(100);
                last_value   = 0;

                if (id % 2 != 0)
                {
                    // odd threads use mco_translog_play_stop function to stop reader
                    LogReader logr = new LogReader(con, null);

                    stop_thread = new StopperThread(logr);
                    stop_thread.start();

                    try {
                        logr.iteratePipe(this, BUFFER_SEGMENT_SIZE, hint != 0 ? hint + PIPE_SEGMENT_ALIGNMENT : 0, pipe);
                        // writer has stopped logging
                        stop_thread.skipAction();
                        stop = true;
                    } catch (DatabaseError de) {
                        if (de.errorCode != 617) { // expected error code MCO_E_TL_PLAY_STOPPED caused by call 'stopPipeIteration'
                            if (de.errorCode == 604 || de.errorCode == 612) {
                                // writer has stopped logging (MCO_E_TL_LOG_NOT_OPENED | MCO_E_TL_PIPE_TERM)
                                stop_thread.skipAction();
                                stop = true;
                            } else
                                throw de; // unexpected error
                        }
                    }

                    try { stop_thread.join(); } catch (Exception e) {};
                } else {
                    // even threads use non MCO_S_OK return code to stop reader
                    // iterate over the attached pipe
                    LogReader logr = new LogReader(con, null);
                    try {
                        logr.iteratePipe(this, BUFFER_SEGMENT_SIZE, hint != 0 ? hint + PIPE_SEGMENT_ALIGNMENT : 0, pipe);
                        // writer has stopped logging
                        stop = true;
                    } catch (DatabaseError de) {
                        if (de.errorCode != 2 * 1000 * 1000) { // expected error code thrown from method 'update'
                            if (de.errorCode == 604 || de.errorCode == 612)
                                // writer has stopped logging (MCO_E_TL_LOG_NOT_OPENED | MCO_E_TL_PIPE_TERM)
                                stop = true;
                            else
                                throw de; // unexpected error
                        }
                    }
                }
                // detach the pipe 
                boolean rc;
                if ((config & Database.MCO_CFG_SHARED_MEMORY) == 0)
                    rc = db.detach(pipe);
                else
                    rc = db.detach(pipe, Sniffer.processIDtoBytes(db, processID)); // example of how to apply connection context if sniffer is used

                if (rc) {
                    n_immediate_detachs++;
                } else {
                    n_deferred_detachs++;
                }
                try { Thread.sleep(500); } catch (Exception e) {};
                n_passes++;
            }
            con.disconnect();
        }

        // LogIterator interface implementation

        public void update(Object obj) {
            Record rec = (Record)obj;
            n_objects++;
            if (last_value != 0) {
                assert(last_value + 1 == rec.val);
            } else {
                n_connects++;
            }
            last_value = rec.val;

            if (--n_iterations == 0) 
            {
                if (id % 2 != 0) {
                    // odd threads use mco_translog_play_stop function to stop reader
                    stop_thread.doAction();
                } else {
                    // even threads use non MCO_S_OK return code to stop reader
                    throw new DatabaseError(2 * 1000 * 1000, null);
                }
            }
        }

        public void create(Object obj) {}
        public void delete(Object obj) {}
        public void deleteAll(Class type) {}
        public void endOfTransaction() {}
    }
    IteratorThread iterThreads[];
}

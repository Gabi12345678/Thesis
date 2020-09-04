import com.mcobject.extremedb.*;

import java.io.Console;

@Persistent
class Quote
{
    @Indexable
    long ikey;
    
    @Indexable
    @Dimension(8)
    String symbol;
    
    int stamp;
    float low;
    float high;
    float open;
    float close;
    int volume;
};

public class sqlserver
{
    static final int PAGE_SIZE = 128;
    static final int DISK_PAGE_SIZE = 4096;
    static final int DISK_CACHE_SIZE = 8*1024*1024;
    static final int DATABASE_SIZE = 16*1024*1024;
    static final int N_QUOTES = 10;
    static final int port = 5001;

    public static void main(String[] args)
    {
        Database db;
        int config = Database.MCO_CFG_SQL_SUPPORT;
        Database.Device devs[];
        Database.Parameters params = new Database.Parameters();

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

        params.memPageSize = PAGE_SIZE; // memory page size
        params.classes = new Class[] { Quote.class }; // list of classes which should be stored in eXremeDB database. The classes must use @Persistent annotation
        params.maxConnections = 10;

        if ((config & Database.MCO_CFG_DISK_SUPPORT) != 0) { // on-disk configuration

            java.io.File file = new java.io.File("opendb.dbs");
            file.delete();
            file = new java.io.File("opendb.log");
            file.delete();

            params.diskPageSize = DISK_PAGE_SIZE; // disk page size
            params.diskClassesByDefault = true; // mark @Persistent classes as on-disk classes by default

            // declare memory devices
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

        // create Database object
        db = new Database(config);

        db.open("sqldb", params, devs); // open database. If database already opened DatabaseError exception thrown

        System.out.println("Open " + 
                           (((config & Database.MCO_CFG_DISK_SUPPORT) != 0) ? "on-disk" : "in-memory") + " database on " + 
                           (((config & Database.MCO_CFG_SHARED_MEMORY) != 0) ? "shared" : "conv") + " memory. Use " + 
                           (((config & Database.MCO_CFG_MVCC_TRANSACTION_MANAGER) != 0) ? "MVCC" : "MURSIW") + " transaction manager");

        SqlLocalConnection con = db.connectSql();
        
        SqlServer server = new SqlServer(con, port);
        
        System.out.println("starting SQL server..");
        server.start();
        
        System.out.println("server started, connecting.");
        SqlRemoteConnection conn = new SqlRemoteConnection("localhost", port, 1);
        
        insert_quotes(conn);
        traverse_quotes(conn);
        delete_quotes(conn);

        System.out.println("Client sessions:");
        SqlServer.SessionInfo[] sessions = server.getSessionsInfo();
        for (SqlServer.SessionInfo sess : sessions) {
            System.out.format("[%s]:%d - %s\n", sess.peerAddr, sess.peerPort, sess.peerState);
        }
        System.out.println("Disconnecting..");
        conn.disconnect();

        server.stop();
        System.out.println("Server stopped.");
    }

    private static void delete_quotes(SqlRemoteConnection conn) {
        System.out.println("delete_quotes");
        
        String sql = "DELETE FROM Quote WHERE ikey = ?";
        conn.executeStatement(sql, 3);
        
    }

    private static void traverse_quotes(SqlRemoteConnection conn) {
        System.out.println("traverse_quotes"); 
        String sql = "SELECT ikey, symbol, stamp, low, high, open, close, volume FROM Quote ORDER BY ikey";
        
        SqlResultSet result = conn.executeQuery(sql);
        for (SqlTuple tuple: result) {
            System.out.println(tuple.getInt(0) + ", " + tuple.get("symbol") + ", " + tuple.get("stamp"));
        }
    }

    private static void insert_quotes(SqlRemoteConnection conn) {

        System.out.println("insert_quotes");
        
        String sql = "INSERT INTO Quote(ikey, symbol, stamp, low, high, open, close, volume) VALUES (?,?,?,?,?,?,?,?)";

        for (int i = 0; i < N_QUOTES; i++) { 
            conn.executeStatement(sql, i, "AA" + i, i+1, 1.0, 4.0, 2.0, 3.0, i*1000);
        }
        System.out.println("Insertion done");
    }
}

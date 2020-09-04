import com.mcobject.extremedb.*;

import java.io.Console;

@Persistent // Class will be stored in eXtremeDB database
class MyTable
{        
    int pk;
    String value;
};

public class Sql
{
    static final int PAGE_SIZE = 128;
    static final int DISK_PAGE_SIZE = 4096;
    static final int DISK_CACHE_SIZE = 8*1024*1024;
    static final int DATABASE_SIZE = 16*1024*1024;

    public static void main(String[] args)
    { 
        Database db;
        int config = Database.MCO_CFG_SQL_SUPPORT;
        Database.Device devs[];
        Database.Parameters params = new Database.Parameters();

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
        params.classes = new Class[] { MyTable.class }; // List of classes which should be stored in eXtremeDB database. The classes must use @Persistent annotation
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
        db.open("sqldb", params, devs); // open database. If database already opened DatabaseError exception thrown
            
        System.out.println("Open " + 
                           (((config & Database.MCO_CFG_DISK_SUPPORT) != 0) ? "on-disk" : "in-memory") + " database on " + 
                           (((config & Database.MCO_CFG_SHARED_MEMORY) != 0) ? "shared" : "conv") + " memory. Use " + 
                           (((config & Database.MCO_CFG_MVCC_TRANSACTION_MANAGER) != 0) ? "MVCC" : "MURSIW") + " transaction manager");

        SqlLocalConnection con = db.connectSql();
        con.startTransaction(Database.TransactionType.ReadWrite);  
        con.executeStatement("insert into MyTable (pk,value) values (?,?)", 2012, "Good bye");
        con.executeStatement("insert into MyTable (pk,value) values (?,?)", 2013, "Hello");
        con.commitTransaction();

        con.startTransaction(Database.TransactionType.ReadOnly);  
        SqlResultSet result = con.executeQuery("select pk,value from MyTable where pk>=?", 2013);
        for (String column : result.getColumnNames()) { 
            System.out.print(column + ", ");
        }
        System.out.println();
        for (SqlTuple tuple : result) {
            System.out.println(tuple.getInt(0) + ", " + tuple.get("value"));
        }
        con.commitTransaction();
        con.disconnect();
        System.out.println("Disconnect from the database");
        db.close();
        System.out.println("Close database");
    }
}

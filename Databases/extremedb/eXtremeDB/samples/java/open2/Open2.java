import com.mcobject.extremedb.*;
import java.io.Console;

@Persistent // Class will be stored in eXtremeDB database
class OpenObj
{        
    int i4;
};

public class Open2
{
    static final int PAGE_SIZE = 128;
    static final int DISK_PAGE_SIZE = 4096;
    static final int DISK_CACHE_SIZE = 8*1024*1024;
    static final int DATABASE_SIZE = 16*1024*1024;

    public static void ShowRuntimeInfo(Database db)
    {
        Database.RuntimeInfo ri = db.getRunTimeInfo();
        System.out.println("Database opened with parameters:");
        System.out.println("mco_version_major = " + ri.mco_version_major);
        System.out.println("mco_version_minor = " + ri.mco_version_minor);
        System.out.println("mco_build_number = " + ri.mco_build_number);

        System.out.println("mco_size_t = " + ri.mco_size_t);
        System.out.println("mco_offs_t = " + ri.mco_offs_t);
        System.out.println("uint4_supported = " + ri.uint4_supported);
        System.out.println("float_supported = " + ri.float_supported);
        System.out.println("mco_checklevel = " + ri.mco_checklevel);
        System.out.println("evaluation_version = " + ri.evaluation_version);
        System.out.println("large_database_supported = " + ri.large_database_supported);
        System.out.println("collation_supported = " + ri.collation_supported);
        System.out.println("heap31_supported = " + ri.heap31_supported);
        System.out.println("bin_serialization_supported = " + ri.bin_serialization_supported);
        System.out.println("fixedrec_supported = " + ri.fixedrec_supported);
        System.out.println("statistics_supported = " + ri.statistics_supported);
        System.out.println("events_supported = " + ri.events_supported);
        System.out.println("save_load_supported = " + ri.save_load_supported);
        System.out.println("object_initialization_supported = " + ri.object_initialization_supported);
        System.out.println("direct_index_field_access_supported = " + ri.direct_index_field_access_supported);
        System.out.println("multiprocess_access_supported = " + ri.multiprocess_access_supported);
        System.out.println("object_repack_supported = " + ri.object_repack_supported);
        System.out.println("transaction_logging_supported = " + ri.transaction_logging_supported);
        System.out.println("cluster_supported = " + ri.cluster_supported);
        System.out.println("high_availability_supported = " + ri.high_availability_supported);
        System.out.println("ha_multicast_supported = " + ri.ha_multicast_supported);
        System.out.println("ha_incremental_replication_supported = " + ri.ha_incremental_replication_supported);
        System.out.println("binary_schema_evalution_supported = " + ri.binary_schema_evalution_supported);
        System.out.println("unicode_supported = " + ri.unicode_supported);
        System.out.println("wchar_supported = " + ri.wchar_supported);
        System.out.println("recovery_supported = " + ri.recovery_supported);
        System.out.println("disk_supported = " + ri.disk_supported);
        System.out.println("direct_pointers_supported = " + ri.direct_pointers_supported);
        System.out.println("persistent_object_supported = " + ri.persistent_object_supported);
        System.out.println("xml_import_export_supported = " + ri.xml_import_export_supported);
        System.out.println("user_defined_index_supported = " + ri.user_defined_index_supported);
        System.out.println("multifile_supported = " + ri.multifile_supported);
        System.out.println("multifile_descriptor_supported = " + ri.multifile_descriptor_supported);
        System.out.println("two_phase_commit_supported = " + ri.two_phase_commit_supported);
        System.out.println("rtree_supported = " + ri.rtree_supported);
        System.out.println("tree_based_hash = " + ri.tree_based_hash);
        System.out.println("tmgr_mvcc_async_cleanup = " + ri.tmgr_mvcc_async_cleanup);
        System.out.println("concurent_disk_btree = " + ri.concurent_disk_btree);
        System.out.println("open_cursor_goto_first = " + ri.open_cursor_goto_first);
        System.out.println("smart_index_insert = " + ri.smart_index_insert);
        System.out.println("btree_leaf_lock = " + ri.btree_leaf_lock);
        System.out.println("null_statistics = " + ri.null_statistics);
        System.out.println("implicit_runtime_start = " + ri.implicit_runtime_start);
        System.out.println("bufferized_sync_iostream = " + ri.bufferized_sync_iostream);
        System.out.println("async_replication = " + ri.async_replication);
        System.out.println("fast_transaction_list = " + ri.fast_transaction_list);
        System.out.println("extendable_dirty_page_bitmap = " + ri.extendable_dirty_page_bitmap);
        System.out.println("mursiw_policy = " + ri.mursiw_policy);
        System.out.println("sync_capabilities = " + ri.sync_capabilities);
        System.out.println("char_comparison_policy = " + ri.char_comparison_policy);
        System.out.println("stream_buffer_size = " + ri.stream_buffer_size);
        System.out.println("max_db_instances = " + ri.max_db_instances);
        System.out.println("max_db_name_length = " + ri.max_db_name_length);
        System.out.println("max_extends = " + ri.max_extends);
        System.out.println("tl_page_buffer_size = " + ri.tl_page_buffer_size);
        System.out.println("ha_max_replicas = " + ri.ha_max_replicas);
        System.out.println("ha_transmit_buffer_size = " + ri.ha_transmit_buffer_size);
        System.out.println("ha_syncronization_buffer_size = " + ri.ha_syncronization_buffer_size);
        System.out.println("default_redo_log_limit = " + ri.default_redo_log_limit);
        System.out.println("mvcc_critical_sections = " + ri.mvcc_critical_sections);
        System.out.println("mvcc_per_index_locks = " + ri.mvcc_per_index_locks);
        System.out.println("con_disk_page_cache_size = " + ri.con_disk_page_cache_size);
        System.out.println("small_con_cache_threshold = " + ri.small_con_cache_threshold);
        System.out.println("extendable_dirty_page_bitmap_limit = " + ri.extendable_dirty_page_bitmap_limit);
        System.out.println("max_vista_sessions = " + ri.max_vista_sessions);
        System.out.println("concurrent_write_transactions = " + ri.concurrent_write_transactions);
        System.out.println("encryption_support = " + ri.encryption_support);
        System.out.println("backup_support = " + ri.backup_support);
        System.out.println("mco_revision = " + ri.mco_revision);
        System.out.println("mco_process_t = " + ri.mco_process_t);
    }

    public static void main(String[] args)
    { 
        Database db1;
        int config = 0;
        boolean closeDB = false;
        boolean disk = false;
        boolean shared = false;
        Database.Device devs[];
        Database.Parameters params = new Database.Parameters();

        // Get config parameters from the command line
        for (String s : args) {
            if ("disk".equals(s))  { disk = true; config |= Database.MCO_CFG_DISK_SUPPORT; }
            if ("shm".equals(s))   { shared = true; config |= Database.MCO_CFG_SHARED_MEMORY; }
            if ("debug".equals(s)) config |= Database.MCO_CFG_DEBUG_LIBRARY;
            if ("mvcc".equals(s))  config |= Database.MCO_CFG_MVCC_TRANSACTION_MANAGER;
            if (s.startsWith("license:")) params.license_key = s.substring(8);
        }

        // Create Database object
        db1 = new Database(config);
        Database.RuntimeInfo ri = db1.getRunTimeInfo();
        long hint = ri.direct_pointers_supported ? 0x20000000 : 0;

        params.memPageSize = PAGE_SIZE; // Memory page size
        params.classes = new Class[] { OpenObj.class }; // List of classes which should be stored in eXremeDB database. The classes must use @Persistent annotation
        params.maxConnections = 10;

        if (disk) { // on-disk configuration
            java.io.File file = new java.io.File("opendb2.dbs");
            file.delete();
            file = new java.io.File("opendb2.log");
            file.delete();

            params.diskPageSize = DISK_PAGE_SIZE; // Disk page size
            params.diskClassesByDefault = true; // Mark @Persistent classes as on-disk classes by default

            // Declare memory devices
            devs = new Database.Device[4];

            if (shared) {
                devs[0] = new Database.SharedMemoryDevice(Database.Device.Kind.Data, "opendb2-db", 0, DATABASE_SIZE);
                devs[1] = new Database.SharedMemoryDevice(Database.Device.Kind.DiskCache, "opendb2-cache", 0, DISK_CACHE_SIZE);
            } else {
                devs[0] = new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE);
                devs[1] = new Database.PrivateMemoryDevice(Database.Device.Kind.DiskCache, DISK_CACHE_SIZE);
            }
                
            devs[2] = new Database.FileDevice(Database.Device.Kind.Data, "opendb2.dbs");
            devs[3] = new Database.FileDevice(Database.Device.Kind.TransactionLog, "opendb2.log");

        } else { // in-memory configuration

            devs = new Database.Device[1];

            if (shared) {
                devs[0] = new Database.SharedMemoryDevice(Database.Device.Kind.Data, "opendb2-db", hint, DATABASE_SIZE);
            } else {
                devs[0] = new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE);
            }
        }

        try {
            db1.open("opendb", params, devs); // Open database. If database already opened DatabaseError exception thrown
            closeDB = true;
            System.out.println("Sample 'Open2' demonstrates opening and connecting \nan " + 
                (disk ? "on-disk" : "in-memory") + " database on " + 
                (shared ? "shared" : "conv") + " memory. \nUsing " + 
                (((config & Database.MCO_CFG_MVCC_TRANSACTION_MANAGER) != 0) ? "MVCC" : "MURSIW") + " transaction manager.\n");

            ShowRuntimeInfo(db1);

        } catch (DatabaseError dbe) {
            if (dbe.errorCode != 66) { // Code 66 means duplicate instance. Valid case for SHM configuration
                throw dbe;
            }
        }

        Connection con1 = new Connection(db1); // Connect to the database
        System.out.println("Connected to the " + (disk ? "Persistent" : "first In-Memory") + " database. \n\nPress Enter to continue\n");
        if (System.console() != null) {
            System.console().readLine();
        }

        Database db2 = new Database(db1);

        devs = new Database.Device[1];

        if (shared) {
            devs[0] = new Database.SharedMemoryDevice(Database.Device.Kind.Data, "opendb2-db2", hint != 0 ? hint + DATABASE_SIZE : 0, DATABASE_SIZE);
        } else {
            devs[0] = new Database.PrivateMemoryDevice(Database.Device.Kind.Data, DATABASE_SIZE);
        }

        Database.Parameters params2 = new Database.Parameters();
        params2.memPageSize = PAGE_SIZE; // Memory page size
        params2.classes = new Class[] { OpenObj.class }; // List of classes which should be stored in eXremeDB database. The classes must use @Persistent annotation
        params2.maxConnections = 10;

        try {
            db2.open("opendb2", params2, devs); // Open database. If database already opened DatabaseError exception thrown
            closeDB = true;
            System.out.println("Open in-memory database on " + 
                (shared ? "shared" : "conv") + " memory. Use " + 
                (((config & Database.MCO_CFG_MVCC_TRANSACTION_MANAGER) != 0) ? "MVCC" : "MURSIW") + " transaction manager");
            ShowRuntimeInfo(db2);
        } catch (DatabaseError dbe) {
            if (dbe.errorCode != 66) { // Code 66 means duplicate instance. Valid case for SHM configuration
                throw dbe;
            }
        }

        Connection con2 = new Connection(db2); // Connect to the database
        System.out.println("Connect to the In-Memory database. \n\nPress Enter to continue");
        if (System.console() != null) {
            System.console().readLine();
        }

        con2.disconnect();
        System.out.println("Disconnected from the In-Memory database");

        db2.close();
        System.out.println("In-Memory database closed");

        con1.disconnect();
        System.out.println("Disconnected from the " + (disk ? "Persistent" : "first In-Memory") + " database");

        if (closeDB) {
            db1.close();
            System.out.println((disk ? "Persistent" : "First In-Memory") + " database closed");
        }

        if (System.console() != null) {
            System.console().readLine();
        }
    }
}

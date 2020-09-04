Command-line options.
---------------------
Command-line options have precedence on config file parameters.

./xsql [LOCAL_DB_NAME | {@HOSTNAME[:PORT]}] {OPTIONS}

if LOCAL_DB_NAME specified, connects to shared-memory database.
if @HOSTNAME[:PORT] specified, connects to remove SQL server.
Otherwise, creates database and optionally start SQL server.

OPTIONS :

-c <config_file> : use specified configuration file;
-f <sql_script>  : SQL script to execute aftr connecting or creating database
-shared          : force to use shared-memory runtime;
-conv            : force to use convential memory runtime;
-mvcc            : force to use MVCC transaction manager;
-mursiw          : force to use MURSIW transaction manager;
-memory          : force to use in-memory only runtime;
-disk            : force to use disk runtime;
-debug           : force to use debug runtime version;
-release         : force to use release runtime version;
-i               : force to use interactive mode;
-b               : force to use batch mode (exit after execution SQL scripts);
-time            : measure execution time;

Parameters for created (new) database:

-dbname <name>   : database name;
-size <dbsize>   : size of in-memory database part;
-page <size>     : size of in-memory page in bytes;
-cache <csize>   : size of disk cache;
-path <dbfile>   : database file name;
-log  <logfile>  : log file name;
-image <filename>: database image file name;
-recovery        : run sniffer on startup to cleanup shared memory database;
-trace           : switch on SQL engine trace;
-p [<iface>:]<port> : start SQL server on specified port/interface;

Configuration file.
-------------------
It uses JSON format with extensions:

1. Allows hexadecimal (0x[0-9A-F]), octal(0[0-7]) and decimal numbers
2. Numbers allow suffixes 'k', 'K' (kilo), 'm', 'M' (mega) and 'g', 'G' (giga). For example, "100m" means 100*1024*1024
3. Quotes are not mandatory for strings without whitespaces and special symbols.
4. Symbols after '#' are comment (up to end of line)

By default, xsql will find 'xsql.cfg' file in the working directory. Can be overrided by '-c' command-line option
If some parameter's value is a combination of flags, it can be defined as array or single flag value.
For example,
a) "flags : value1"  means "flags = value1"
b) "flags : [value1, value2, value3]" means "flags = value1 | value2 | value3"
c) "flags : []" means "flags = 0"

{
    # Runtime configuration (set of libraries)
    runtime_configuration : {
        # Use debug libraries, boolean. Default value 'false'
        debug_library : false,

        # Disk database support, boolean. Default value 'false'
        disk_support : true,

        # Use shared memory, boolean. default value 'false'
        shared_memory : false,

        # Transaction manager, 'mursiw' or 'mvcc'. Default value 'mursiw'
        transaction_manager : mvcc,

        # Synchronization library, e.g. 'mcoslnx'. 
        # Default values are: 'mcoslnx' for Linux with shared-memory; 'mcoslnxp' for Linux with conv. memory;
        #                     'mcossun' for Solaris; 'mcoshpuxp' for HP-UX
        sync_library : mcoslnx,

        # Memory device library, default 'mcomipc' for shared-memory, 'mcomconv' for conventional
        mem_library : mcomipc,

        # File system wrapper library, default 'mcofu98'
        fs_library : mcofu98,
        
        # Use RLE sequences implementation, boolean. Default value 'false'
        rle_sequences : false,
    },

    # Global eXtremeDB runtime options (for mco_runtime_setoption())
    runtime_options : {

        # MCO_RT_WINDOWS_SHM_OPT
        windows_shm_opt : {
            # One of "global" (MCO_RT_WINDOWS_SHM_PREFIX_GLOBAL), "local" (MCO_RT_WINDOWS_SHM_PREFIX_LOCAL),
            # "sessions" (MCO_RT_WINDOWS_SHM_PREFIX_SESSIONS) or  "none" (MCO_RT_WINDOWS_SHM_PREFIX_NONE)
            prefix : none,

            # One of "empty" (MCO_RT_WINDOWS_SHM_SEC_DESCR_EMPTY), "null" (MCO_RT_WINDOWS_SHM_SEC_DESCR_NULL)
            # or "sameuser" (MCO_RT_WINDOWS_SHM_SEC_DESCR_SAMEUSER)
            sec_descr : empty,
        },

        # MCO_RT_OPTION_MARK_LAST_OBJ, boolean
        mark_last_obj : true,

        # MCO_RT_OPTION_UNIX_SHM_MASK, unsigned
        unix_shm_mask : 0666,

        # MCO_RT_POSIX_SHM_OPT, combination of "anonymous" (MCO_RT_POSIX_SHM_ANONYMOUS) and "shared" (MCO_RT_POSIX_SHM_SHARED)
        posix_shm_opt : [anonymous, shared],

        # MCO_RT_CRC_ALGORITHM, one of "none" (MCO_RT_CRC32_NONE), "crc32" (MCO_RT_CRC32), "crc32_fast" (MCO_RT_CRC32_FAST),
        # "crc32_old" (MCO_RT_CRC32_OLD) or "crc32c" (MCO_RT_CRC32C)
        crc_algorithm: none,

        # MCO_RT_MAX_PATRICIA_DEPTH, unsigned
        max_patricia_depth : 256,

        # MCO_RT_MAX_DYNAMIC_PIPES, unsigned
        max_dynamic_pipes : 10,

        # MCO_RT_OPTION_CLASS_LOAD_MERGE, boolean
        class_load_merge : false,


        # Resolution of datetime type (DateTime::precision) : 1 - in seconds, 1000 - in milliseconds, 1000000 - in microseconds. 
        # Unsigned, default value is 1 (seconds)
        datetime_precision : 1000,

    },

    # Database schema definition. Only one of 'schema' or 'schema_file' allowed.
    schema : "declare database test;", # inlined mco-schema, string
    schema_file : "schema.mco",        # name of schema file, string

    # mcocomp parameters for compiling the schema
    mcocomp_params : {
        nonatomic      : true,            # make unspecified classes nonatomic, boolean. Default value is false
        persistent     : false,           # make unspecified classes persistent, boolean. Default value is false
        compact        : true,            # make all classes compact, boolean. Default value is false
        nosort         : true,            # do not change order of fields, boolean. Default value is true
        include_dir    : "/home/user/"    # specifies the include directory
        wchar_size     : 4,               # size of native wchar_t type (1, 2 or 4)
    },

    # Database name, string
    database_name : "mydbname",

    # Shortcats for database size, cache size, file and log names. Conflicts with 'devices' (see below):
    database_size    :  100m,      # database size in bytes, unsigned
    cache_size       :  200m,      # disk cache size in bytes, unsigned
    database_filename: "test.dbs", # database file name, string
    log_filename     : "test.log", # log file name, string

    # Database image filename. If specified, load database content from it
    saved_image : "db.img"

    # Array of device's descriptions. Corresponds to mco_device_t structures
    # Conflicts with 'database_size', 'cache_size', 'database_filename', 'log_filename'
    devices : [
        {
            # Device type (mco_device_t::type), one of 'memory' (MCO_MEMORY_CONV or MCO_MEMORY_NAMED depending on 'runtime_configuration.shared_memory'),
            # 'file' (MCO_MEMORY_FILE), 'multifile' (MCO_MEMORY_MULTIFILE), 'raid' (MCO_MEMORY_RAID) or 'cyclic_file_buf' (MCO_MEMORY_CYCLIC_FILE_BUF)
            # Default value is 'memory'
            type : memory,

            # mco_device_t::assignment, one of 'database' (MCO_MEMORY_ASSIGN_DATABASE), 'cache' (MCO_MEMORY_ASSIGN_CACHE),
            # 'persistent' (MCO_MEMORY_ASSIGN_PERSISTENT) or 'log' (MCO_MEMORY_ASSIGN_LOG)
            # Default value is 'database'
            assignment : database,

            # Size of device (mco_device_t::size), unsigned
            size : 100m,

            # Device name (mco_device_t::dev.<...>.name). Ignored for MCO_MEMORY_CONV. string
            name : "device-name",

            # mco_device_t::dev.named.hint for MCO_MEMORY_NAMED device, unsigned
            shared_hint : 0x2000000,

            # mco_device_t::dev.multifile.segment_size for MCO_MEMORY_MULTIFILE device type, unsigned
            segment_size : 10m,

            # mco_device_t::dev.raid.offset for MCO_MEMORY_RAID device type, unsigned
            offset : 0,

            # mco_device_t::dev.raid.level for MCO_MEMORY_RAID device type, unsigned
            level : 0,


            # Flag set (mco_device_t::dev.XXX.flags), depends on device type.
            # For MCO_MEMORY_NAMED - combination of "anonymous" (MCO_RT_POSIX_SHM_ANONYMOUS) and "shared" (MCO_RT_POSIX_SHM_SHARED)
            # For MCO_MEMORY_FILE, MCO_MEMORY_CYCLIC_FILE_BUF, MCO_MEMORY_MULTIFILE, MCO_MEMORY_RAID - combination of
            #    "default" (MCO_FILE_OPEN_DEFAULT), "read_only" (MCO_FILE_OPEN_READ_ONLY), "truncate" (MCO_FILE_OPEN_TRUNCATE),"no_buffering" (MCO_FILE_OPEN_NO_BUFFERING),
            #    "existing" (MCO_FILE_OPEN_EXISTING), "temporary" (MCO_FILE_OPEN_TEMPORARY), "fsync_fix" (MCO_FILE_OPEN_FSYNC_FIX), "subpartition" (MCO_FILE_OPEN_SUBPARTITION),
            #    "fsync_aio_barrier" (MCO_FILE_OPEN_FSYNC_AIO_BARRIER), "compressed" (MCO_FILE_OPEN_COMPRESSED)
            #    "lock" (MCO_FILE_OPEN_LOCK), "no_read_buffering" (MCO_FILE_OPEN_NO_READ_BUFFERING) and "no_write_buffering" (MCO_FILE_OPEN_NO_WRITE_BUFFERING)

            flags : default,
        },
    ],


    # Database parameters. Corresponds to mco_db_params_t structure
    db_params: {
        # In-memory page size (mco_db_params_t::mem_page_size), unsigned.
        mem_page_size : 256,

        # Disk page size (mco_db_params_t::disk_page_size), unsigned
        disk_page_size : 4096,

        # Max number of connections (mco_db_params_t::db_max_connections), unsigned
        db_max_connections : 100,

        # Max size of persistent database (mco_db_params_t::disk_max_database_size), unsigned
        disk_max_database_size : 0,

        # Quantum of increasing size of database file (mco_db_params_t::file_extension_quantum), unsigned
        file_extension_quantum : 0,

        # Database logging method (mco_db_params_t::db_log_type), one of "no" (NO_LOG), "redo" (REDO_LOG), "undo" (UNDO_LOG)
        db_log_type : redo,

        # Criteria of hash reallocation in percent of used items (mco_db_params_t::hash_load_factor), unsigned
        hash_load_factor : 100,

        # Maximum number of active write transactions when optimistic locking of B-Tree indexes can be performed
        index_optimistic_lock_threshold : 100,

        # Log settings (mco_db_params_t::log_params)
        log_params: {
            # mco_log_params_t::default_commit_policy, one of 'sync_flush' (MCO_COMMIT_SYNC_FLUSH), 'buffered' (MCO_COMMIT_BUFFERED),
            # 'delayed' (MCO_COMMIT_DELAYED) or 'no_sync', (MCO_COMMIT_NO_SYNC)
            default_commit_policy : buffered,

            # mco_log_params_t::redo_log_limit, unsigned
            redo_log_limit : 16m,

            # mco_log_params_t::delayed_commit_threshold, unsigned
            delayed_commit_threshold : 0,

            # mco_log_params_t::max_delayed_transactions, unsigned
            max_delayed_transactions : 100,

            # mco_log_params_t::max_commit_delay, unsigned
            max_commit_delay : 1000
        },

        # Open mode mask (mco_db_params_t::mode_mask), combination of 'mvcc_auto_vacuum' (MCO_DB_MODE_MVCC_AUTO_VACUUM),
        # 'smart_index_insert (MCO_DB_MODE_SMART_INDEX_INSERT), 'open_existing' (MCO_DB_OPEN_EXISTING), 'use_crc_check' (MCO_DB_USE_CRC_CHECK),
        # 'transient' (MCO_DB_TRANSIENT), 'lazy_mem_initialization' (MCO_DB_LAZY_MEM_INITIALIZATION),
        # 'mursiw_disk_commit_optimization' (MCO_DB_MURSIW_DISK_COMMIT_OPTIMIZATION), 'bulk_write_modified_pages' (MCO_DB_BULK_WRITE_MODIFIED_PAGES),
        # 'index_preload' (MCO_DB_INDEX_PRELOAD),
        # 'disable_nested_transactions' (MCO_DB_DISABLE_NESTED_TRANSACTIONS), 'disable_implicit_rollback' (MCO_DB_DISABLE_IMPLICIT_ROLLBACK),
        # 'inmemory_protection' (MCO_DB_INMEMORY_PROTECTION), 'inclusive_btree' (MCO_DB_INCLUSIVE_BTREE),
        # 'inmemory_compression (MCO_DB_INMEMORY_COMPRESSION), 'separate_bitmap' (MCO_DB_SEPARATE_BITMAP),
        # 'disable_btree_rebalance_on_delete' (MCO_DB_DISABLE_BTREE_REBALANCE_ON_DELETE),
        # 'auto_rollback_first_phase' (MCO_DB_AUTO_ROLLBACK_FIRST_PHASE), 'mvcc_compatibility_mode' (MCO_DB_MVCC_COMPATIBILITY_MODE),
        # 'disable_page_pool_reserve' (MCO_DB_DISABLE_PAGE_POOL_RESERVE), 'redo_log_optimization' (MCO_DB_REDO_LOG_OPTIMIZATION),
        # 'disable_hot_updates' (MCO_DB_DISABLE_HOT_UPDATES), 'sql_autocheckpoint' (MCO_DB_SQL_AUTOCHECKPOINT),
        # 'read_only' (MCO_DB_MODE_READ_ONLY), 'use_aio' (MCO_DB_USE_AIO), 'incremental_backup' (MCO_DB_INCREMENTAL_BACKUP) and
        # 'mvcc_table_level_locking' (MCO_DB_MVCC_TABLE_LEVEL_LOCKING):

        mode_mask : [],

        # Minimal number of pages in per-connection allocator (mco_db_params_t::min_conn_local_pages), unsigned
        min_conn_local_pages : 256,

        # Maximal number of pages in per-connection allocator (mco_db_params_t::max_conn_local_pages), unsigned
        max_conn_local_pages : 1024,

        # Priority of allocator bitmap pages in LRU page pool (mco_db_params_t::allocation_bitmap_caching_priority), integer
        allocation_bitmap_caching_priority : 0,

        # Priority of indices pages in LRU page pool (mco_db_params_t::index_caching_priority), integer
        index_caching_priority : 0,

        # Priority of objects (non-blob) pages in LRU page pool (mco_db_params_t::object_caching_priority), integer
        object_caching_priority : 0,

        # Database encryption key (mco_db_params_t::cipher_key), string
        cipher_key : "secret",

        # Dynamically extend hash table when number of items exceeds load factor (mco_db_params_t::dynamic_hash), boolean
        dynamic_hash : true,

        # eXtremeDB license key (mco_db_params_t::license_key) ,string
        license_key : "ABCABCABCABC",

        # Used to reserve space for classes in database header to allow dynamic table creation (mco_db_params_t::max_classes), unsigned
        max_classes : 100,

        # Used to reserve space for indexes in database header to allow dynamic table creation (mco_db_params_t::max_indexes), unsigned
        max_indexes : 1000,

        # Used to reserve space for the dictionary in the database header to allow dynamic table creation (mco_db_params_t::ddl_dict_size), unsigned
        ddl_dict_size : 128k,

        # Size of object after reaching which mco_w_compact is called during transaciton commit (mco_db_params_t::autocompact_threshold), unsigned
        autocompact_threshold : 100k,

        # Scheduling policy for transactions with the same priority (mco_db_params_t::trans_sched_policy).
        # One of 'fifo' (MCO_SCHED_FIFO), 'reader_favor' (MCO_SCHED_READER_FAVOR) or 'writer_favor' (MCO_SCHED_WRITER_FAVOR)
        trans_sched_policy : fifo,

        # Maximal number of pages accessed by connection (mco_db_params_t::max_active_pages), unsigned
        max_active_pages: 32,

        # Number of bundles in encryption/protection/compression page hash (mco_db_params_t::page_hash_bundles), unsigned
        page_hash_bundles: 8,

        # compression level (mco_db_params_t::compression_level), 0..9, 0 - no compression
        compression_level: 0,

        # bitmap of page kinds which should be compressed, (mco_db_params_t::compression_mask), unsigned
        compression_mask: 0x1,

        # use to allocate page map: virtual database space can not be larger than physical size * compression ratio, 
        # (mco_db_params_t::expected_compression_ratio), unsigned
        expected_compression_ratio: 10,

        # number of keys taken from leaf B-Tree page at each access, can not be larger than MCO_CFG_CURSOR_BUF_SIZE,
        # (mco_db_params_t::btree_cursor_read_ahead_size), unsigned
        btree_cursor_read_ahead_size : 3,

        # MVCC bitmap size, should be power of two, unsigned
        mvcc_bitmap_size : 0,

        # heap memory used by various internal DB functionalities, mostly for buffering during
        # XML export/import and database save/load operations, unsigned
        additional_heap_size : 10k,

        # Size of copy-on-write MVCC pagemap (in bytes, one byte per disk page), unsigned
        cow_pagemap_size : 1m,

        # Size of backup counters array, bytes, power of two, ignored if disk_max_database_size set.
        # Set to zero do disable backup feature
        backup_map_size : 64k,

        # Number of pages for last exclusive pass of backup procedure, set to zero to disable treshold
        backup_min_pages : 1024,

        # Max number of passes before exclusive pass of backup procedure
        backup_max_passes : 10,

        # Name of a file will be used to store backup temporary data on mco_db_close() call 
        backup_map_filename : "mydb.bm",
    },

    # Run sniffer to cleanup shared memory database, boolean
    recovery_enabled : true,

    # Parameter for setlocale()
    locale : "C",

    # Max. queue length for asynchronous I/O driver. '0' means default values , unsigned
    aio_queue_length : 10000,

    #switch on SQL engine trace, boolean
    sql_trace : false,

    # Number of shards for SqlAggregator client
    sql_n_shards : 4,

    # Shortcut for sql_servers[] array. Conflicts with 'sql_servers'
    # Listen port/interface for SQL server in the form [<iface>:]<port>. For example, "5000" or  "[::1]:10000"
    sql_port : 5001,

    # Use Unix domain sockets for client-server interaction
    sql_local_sockets : true

    # Compression level for client-server interaction, 0..9. Default value 0 (no compression)
    sql_compression_level : 1,

    # Redundancy level, unsgined.
    sql_n_replicas : 1,

    # Maximal number of attempts to connect to the server, unsigned. Default value 10
    sql_connect_attempts : 10,
    
    # Timeout for each connect attempt in milliseconds, unsigned. Default value 2000 (2 seconds)
    sql_connect_timeout  : 2000,
    
    # Timeout for read operations in milliseconds, unsigned. Default value 1200000 (20 minutes)
    sql_read_timeout : 120000,

    # Replication method ('sql' or 'ha') :
    sql_replication_type : sql,

    # Size of SQL (server or client) transfer buffer in bytes, unsigned. Default 64k
    sql_comm_buffer : 256k,

    # Number of SQL server threads
    sql_server_threads : 8,

    # Number of SQL server GC threads
    sql_server_gc_threads : 2,

    # Maximum amount of memory SQL can use for internal allocators. Default 0 (no limit)
    sql_workspace_limit : 1g,

    # Enable SQL parallel sequence execution
    sql_use_connection_pool : false,

    # Require authentication for clients, boolean. Default value 'false'
    sql_auth_required : false,

    # Period in milliseconds to poll the interrupts from the client(s). If 0, disable interruption. Default value 100
    sql_interrupt_timeout : 500,

    # User name to connect to xSQL server, string
    sql_login : 'username',
    
    # Password to connect to xSQL server, string
    sql_password : 'secure_passwd',

    # Tune parameters for SQL optimizer
    sql_optimizer_params : {
        # Size of buffer used for batch insert, unsigned, default 1024
        batch_insert_buffer_size : 1024,
    },

    # Array of SQL server's descriptions. The xsql will start the specified number of servers
    # Conflicts with 'sql_port'
    sql_servers : [
        {
            # Listen port/interface for SQL server instance in the form [<iface>:]<port>. For example, "5000" or  "[::1]:10000"
            port           : 5000,

            # Size of SQL server transfer buffer in bytes, unsigned. Default is 'sql_comm_buffer'
            comm_buffer    : 64k,

            # Number of SQL server threads. Default is 'sql_server_threads'
            server_threads : 8,

            # Use Unix domain sockets for client-server interaction. Default is sql_local_sockets'
            local_socket   : true,

            # Require authentication for clients, boolean. Default value 'false'
            auth_required : false,

            # Period in milliseconds to poll the interrupts from the client(s). If 0, disable interruption. Default value 100
            interrupt_timeout : 500,

            # Number of SQL server threads. Default is 'sql_server_gc_threads'
            gc_threads : 2,

            # Period of activating GC threads in milliseconds. Default value is 100 msec
            gc_period : 100,

            # SSL settings specific for this server. See the global ssl_params section
            # for the description of the parameters and their default values.
            # These parameters override the global settings.
            # Global SSL settings are inherited if this section is empty.
            # SSL is only enabled if this section is specified explicitly.
            ssl_params : {
                cipher_list : "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4",
                max_cert_list : 102400,
                verify_mode : [verify_peer, verify_fail_if_no_cert],
                verify_depth : 100,
                cert_file_pem : "sqlservercert.pem",
                pkey_file_pem : "sqlserverkey.pem"
            }

            # Compression level for client-server interaction, 0..9. Default value is 'sql_compression_level'
            compression_level : 1,
        },
    ]


    # Threshold to abort the current transaction if amount of free memory becomes lower. Unsigned, default 1Mb
    free_memory_threshold : 4m,

    # !!! DEPRECATED !!! Use 'database_name' and 'open_database : false' instead
    # Shared-memory database name to connect, string
    local_client : "dbname",

    # Open new database (true) or connect to existing one (false). Default value is 'true'
    open_database : true,

    # Array of remote nodes to connect in form <IP/hostname>:<port>
    remote_client : [ "192.168.1.100:5001", "192.168.1.102:5001" ],

    # Array of scripts (filenames) to execute
    sql_scripts : [ script_1.sql, script_2.sql ],

    # In-place SQL script
    sql_statements: "create table A (i integer);",

    # Name of event log file or 'stderr'
    event_log : xsql.log,

    # Cluster parameters, corresponds to mco_cluster_params_t structure
    cluster_params : {
        # List of cluster nodes (mco_cluster_params_t::nodes)
        nodes : [
            {
                # Node address in form <IP/hostname>:<port> (mco_cluster_node_params_t::addr)
                addr : "127.0.0.1:5000",

                # Quorum rank (mco_cluster_node_params_t::qrank), unsigned
                qrank : 0
            },
        ],

        # Current node ID  (mco_cluster_params_t::node_id), unsigned
        node_id : 0,

        # Size of connection pool in percents of db_max_connections (mco_cluster_params_t::conn_pool_factor)
        conn_pool_factor: 50,

        # Max number of objects per message during synchronization (mco_cluster_params_t::sync_msg_objects), unsigned
        sync_msg_objects : 100,

        # Max size of message in bytes during synchronization (mco_cluster_params_t::sync_msg_size), unsigned
        sync_msg_size : 100k,

        # Transaction window parameters (mco_cluster_params_t::window)
        window : {
            # Size of window buffer in bytes (mco_cluster_window_t::bsize), unsigned
            bsize : 100k,

            # Number of transactions (mco_cluster_window_t::length), unsigned
            length : 4,

            # Window timeout in milliseconds (mco_cluster_window_t::timeout), unsigned
            timeout : 10,
        },

        # Internal send buffer size in bytes (mco_cluster_params_t::cl_sndbuf), unsigned
        cl_sndbuf : 2k,

        # Internal receive buffer size in bytes (mco_cluster_params_t::cl_rcvbuf), unsigned
        cl_rcvbuf : 2k,

        # Cluster mode mask (mco_cluster_params_t::mode_mask), combination of 'early_data_send' (MCO_CLUSTER_MODE_EARLY_DATA_SEND) 
        # and 'binary_evolution' (MCO_CLUSTER_MODE_BINARY_EVOLUTION)
        mode_mask : binary_evolution,

        # Custom quorum policies. Possible values are :
        # 'qrank_based' - default policy based on node's qrank;
        # 'always_writable' - allows WRITE transactions on the node regardless of state of other nodes
        # 'all_nodes_required' - WRITE transactions are allowed only if all other nodes are alive and connected to the current node
        quorum_policy : qrank_based,

        # Detach node from cluster on exit, boolean. Default value is 'false'
        auto_detach : true,

        # Cluster-specific SSL settings. See the global ssl_params section
        # for the description of the parameters and their default values.
        # These parameters override the global settings.
        # Global SSL settings are used if this section is empty.
        # SSL is only enabled if this section is specified explicitly.
        ssl_params : {
            cipher_list : "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4",
            max_cert_list : 102400,
            verify_mode : [verify_peer, verify_fail_if_no_cert],
            verify_depth : 100,
            cert_file_pem : "clustercert.pem",
            pkey_file_pem : "clusterkey.pem"
        },

        # TCP/IP parameters (mco_cluster_params_t::nw::tcp)
        nw_tcp : {
            # Maximum socket send buffer in bytes (mco_clnw_tcp_params_t::so_sndbuf), unsigned
            so_sndbuf : 16k,

            # Maximum socket receive buffer in bytes (mco_clnw_tcp_params_t::so_rcvbuf), unsigned
            so_rcvbuf : 16k,

            # Timeout for connecting nodes to each other in milliseconds (mco_clnw_tcp_params_t::connect_timeout), unsigned
            connect_timeout : 5000,

            # Time between attempts to connect to other nodes in milliseconds (mco_clnw_tcp_params_t::connect_interval), unsigned
            connect_interval : 500,

            # Number of milliseconds between keep-alive messages in milliseconds (mco_clnw_tcp_params_t::keepalive_time), unsigned
            keepalive_time : 1000,

            # Number of keep-alive probes to send before node disconnects (mco_clnw_tcp_params_t::keepalive_probes), unsigned
            keepalive_probes : 5,

            # mco_clnw_tcp_params_t::socket_domain, one of 'inet' (MCO_SOCK_INET_DOMAIN), 'inetv4' (MCO_SOCK_INETV4_DOMAIN),
            # 'inetv6' (MCO_SOCK_INETV6_DOMAIN), 'local' (MCO_SOCK_LOCAL_DOMAIN) or 'sdp' (MCO_SOCK_SDP_DOMAIN)
            socket_domain : inet,

            # compression level (mco_clnw_tcp_params_t::compression_level), 0..9, 0 - no compression
            compression_level: 0,
        },

        # MPI parameters (mco_cluster_params_t::nw::tcp)
        nw_mpi : {
            # MPI mode flags (mco_clnw_mpi_params_t::flags). combination of 'busywait' (MCO_CLUSTER_MPI_BUSYWAIT) and 'serialized_send' (MCO_CLUSTER_MPI_SERIALIZED_SEND)
            flags : busywait
        }
    },


    # HA parameters
    ha_params: {
        # Address(es) of master(s). Transport-dependend, ":tcp:5000" (or just "5000"), ":udp:5000" or ":pipe:/tmp/test00"
        # See also mco_HA_attach_replica() description
        master_ports : [ ":tcp:6100", ":udp:6100" ],

        # Master parameters. Corresponds to mco_HA_master_params_t structure
        master_params : {
            # Master modes & options flags (mco_HA_master_params_t::mode_flags). Combination of
            # 'mcast_reliable' (MCO_HAMODE_MCAST_RELIABLE), 'hotsync' (MCO_HAMODE_HOTSYNCH) and 'binevolution' (MCO_HAMODE_BINEVOLUTION)
            # Other modes are enabled automatically (see below)
            mode_flags : hotsync,

            # Maximum number of replicas that can connect to master (mco_HA_master_params_t::max_number_of_replicas), unsigned
            max_number_of_replicas : 10,

            # Size of buffer for asynchronous replication in bytes (mco_HA_master_params_t::async_databuf::size), unsigned
            # If not 0, enables async replication mode (MCO_HAMODE_ASYNC)
            async_databuf_size : 100m,

            # Maximum number of transactions stored on master side. (mco_HA_master_params_t::trans_log_length), unsigned
            # If not 0, enables stateful replication (MCO_HAMODE_STATEFUL_REPLICATION)
            trans_log_length : 100,

            # Timeout to send transaction in milliseconds (mco_HA_master_params_t::commit_timeout), unsigned
            commit_timeout : 2000,

            # Initial synchronization timeout in milliseconds (mco_HA_master_params_t::initial_timeout), unsigned
            initial_timeout : 60000,

            # Timeout for waiting replica's ACK in milliseconds (mco_HA_master_params_t::synch_timeout), unsigned
            synch_timeout : 2000,

            # Timeout for detaching replica in milliseconds (mco_HA_master_params_t::detach_timeout), unsigned
            detach_timeout : 1000,

            # Multicast address in form "<IP/hostname>:<port>" or "<port>" (mco_HA_master_params_t::mcast_addr and mco_HA_master_params_t::mcast_port), string
            # Enables multicast replication mode (MCO_HAMODE_MCAST)
            mcast_address : "224.0.0.1:12345",

            # Number of objects to send at once during hot-synchronization (mco_HA_master_params_t::hotsync_msg_objects), unsigned
            # Makes sense only if 'hotsync' mode
            hotsync_msg_objects : 100,

            # Max message size during hot-synchronization in bytes (mco_HA_master_params_t::hotsync_msg_size), unsigned
            # Makes sense only if 'hotsync' mode
            hotsync_msg_size : 100k,

            # Delay between hot-synchronization messages in milliseconds (mco_HA_master_params_t::hotsync_delay), unsigned. Default value is 0
            # Makes sense only if 'hotsync' mode
            hotsync_delay : 10,

            # Minimum number of connected replicas in order to the master database to accept write transactions (mco_HA_master_params_t::quorum), unsigned. Default value is 0
            quorum : 1,

            # Compression level (mco_HA_master_params_t::compression_level), 0..9, 0 - no compression
            compression_level: 0,

            # HA Master-specific SSL settings. See the global ssl_params section
            # for the description of the parameters and their default values.
            # These parameters override the global settings.
            # Global SSL settings are used if this section is empty.
            # SSL is only enabled if this section is specified explicitly.
            ssl_params : {
                cipher_list : "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4",
                max_cert_list : 102400,
                verify_mode : [verify_peer, verify_fail_if_no_cert],
                verify_depth : 100,
                cert_file_pem : "hamastercert.pem",
                pkey_file_pem : "hamasterkey.pem"
            }
        },

        # Interval between keep-alive messages in milliseconds.
        keep_alive_interval : 1000,

        # Transaction window size. See mco_HA_set_trans_window_size() description
        trans_window_size : 10,

        # Connection timeout period in milliseconds, used on both master and replica sides. See description of mco_HA_attach_master() and mco_HA_attach_replica()
        connect_timeout : 2000,

        # Connection string(s). Transport-dependend, ":tcp:192.168.1.100:5000" (or just "192.168.1.100:5000"), ":udp:192.168.1.100:5000" or ":pipe:/tmp/test00"
        # Xsql will try to connect to addresses in round-robin manner
        connection_strings: [ "192.168.1.100:5000", "192.168.1.102:5000" ],

        # Interval between connection attempts in milliseconds, unsigned
        connect_interval : 1000,

        # Maximum number of connect rounds, unsigned
        connect_attempts : 5,

        # Number of port used for cancel socket. If 0, cancellation will be disabled. Default value is 9990
        cancel_port : 4321,

        # Replica parameters. Corresponds to mco_HA_replica_params_t structure
        replica_params : {
            # Master modes & options flags (mco_HA_replica_params_t::mode_flags). Combination of
            # 'force_master' (MCO_HAMODE_FORCE_MASTER) and 'force_sync' (MCO_HAMODE_FORCE_SYNC)
            mode_flags : [ force_sync ],

            # Initial synchronization timeout in milliseconds (mco_HA_replica_params_t::initial_timeout), unsigned
            initial_timeout : 60000,

            # Timeout to receive transaction data from master in milliseconds (mco_HA_replica_params_t::commit_timeout), unsigned
            commit_timeout : 2000,

            # Max time between two consecutive transactions in milliseconds (mco_HA_replica_params_t::wait_data_timeout), unsigned
            wait_data_timeout : 5000,

            # Max number of attempts to repeat commit for reliable multicast (mco_HA_replica_params_t::repeat_counter), unsigned
            repeat_counter : 3,

            # Multicast address in form "<IP/hostname>:<port>" or "<port>" (mco_HA_replica_params_t::mcast_addr and mco_HA_replica_params_t::mcast_port), string
            mcast_address : "224.0.0.1:12345",

            # Disk commit policy during initial synchronization (mco_HA_replica_params_t::initial_commit_policy)
            # One of 'sync_flush' (MCO_COMMIT_SYNC_FLUSH), 'buffered' (MCO_COMMIT_BUFFERED),
            # 'delayed' (MCO_COMMIT_DELAYED) or 'no_sync', (MCO_COMMIT_NO_SYNC)
            initial_commit_policy : no_sync,

            # Number of objects per transaction during initial synchronization (mco_HA_replica_params_t::initial_objs_in_trans), unsigned
            initial_objs_in_trans : 100,

            # Maximum number of transactions that can be merged on the replica side (mco_HA_replica_params_t::batch_commit_length), unsigned. Default value is 0 (no limit)
            batch_commit_length : 10,

            # Maximum total size of transactions that can be merged on the replica side (mco_HA_replica_params_t::batch_commit_bsize), unsigned. Default value is 0 (no limit)
            batch_commit_bsize : 100k,

            # Maximum period of time that the commit on the replica can be delayed, in milliseconds (mco_HA_replica_params_t::batch_commit_period), unsigned. Default value is 1000 (1 second)
            batch_commit_period : 5,

            # Compression level (mco_HA_replica_params_t::compression_level), 0..9, 0 - no compression
            compression_level: 0,

            # HA Replica-specific SSL settings. See the global ssl_params section
            # for the description of the parameters and their default values.
            # These parameters override the global settings.
            # Global SSL settings are used if this section is empty.
            # SSL is only enabled if this section is specified explicitly.
            ssl_params : {
                cipher_list : "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4",
                max_cert_list : 102400,
                verify_mode : [verify_peer, verify_fail_if_no_cert],
                verify_depth : 100,
                cert_file_pem : "hareplicacert.pem",
                pkey_file_pem : "hareplicakey.pem"
            },
        }

        # Initial HA switch mode. Possible values are:
        # 'auto' (default) - start as a replica, if connection to master fails, switch to master mode
        # 'master'         - run only as a master
        # 'replica'        - run only as a replica
        # 'standalone'     - don't start HA
        switch_mode : "auto",

    }

    #PERFMON params

    # Use perfmon in xsql
    perfmon : {
        # Use mcohv
        hv : true,

        #IM database size for perfmon
        database_size : 64m,

	#Set listening address and port for embedded HTTP server:
	hvaddr : "0.0.0.0",  # Default value - listen on all interfaces
	hvport : 8082, #Default value
	show_browser : true   # Show database browser in addition to performance monitor
    },

    # REST WebAPI server parameters.
    rest : {
        # Interface address to use. Listen on all interfaces (0.0.0.0) by default.
        addr : "0.0.0.0",
        # Port to use. Default value is 8083.
        port : 8000,

        # HTTP basic authentication parameters. Authentication is disabled
        # if these parameters are not set.
        http_realm: "WebAPI",
        http_username: "user",
        http_password: "pass",

        # REST-specific SSL settings. See the global ssl_params section
        # for the description of the parameters and their default values.
        # These parameters override the global settings.
        # Global SSL settings are used if this section is empty.
        # SSL is only enabled if this section is specified explicitly.
        ssl_params : {
            cipher_list : "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4",
            max_cert_list : 102400,
            verify_mode : [verify_none],
            verify_depth : 100,
            cert_file_pem : "restcert.pem",
            pkey_file_pem : "restkey.pem"
        },
    },

    # Transaction Logging (TL) parameters.
    tl_params : {

        # Transaction log flags (mco_TL_start_data::flags). Combination of 'crc' (MCO_TRANSLOG_CRC) and 'iterable' (MCO_TRANSLOG_ITERABLE)
        # MCO_TRANSLOG_APPEND, MCO_TRANSLOG_SIZE_CLBK, MCO_TRANSLOG_SYNC_INSTANTLY and MCO_TRANSLOG_SYNC_COUNT are enabled automatically (see below)
        # Default value 0
        flags : [crc, iterable],

        # Path to logfile. Required, no default value.
        log_path : "my.log",

        # Path to saved image used by mco_translog_db_save(). Not required, but if not set, the mco_translog_db_save() can't be called
        snapshot_path : "my.img",

        # Size of single disk page in bytes (mco_TL_start_data::disk_page_size). Default value 4096
        disk_page_size : 4096,

        # Maximum count of not flushed records (mco_TL_start_data::flush_depth). 
        # If set to 0, don't flush
        # If set to 1, the MCO_TRANSLOG_SYNC_INSTANTLY flag is automatically set
        # If > 1, MCO_TRANSLOG_SYNC_COUNT flag is automatically set
        # Default value 0 (no flushing)
        flush_depth : 10,

        # Max size of log file in bytes. When the limit is reached the mco_translog_db_save() is automatically called. 
        # It saves database image and truncates the log file. Parameter 'snapshot_path' is required when max_size > 0
        # Default value 0 (no limit)
        max_size : 1m,

        # Period in millisecond to automatically call mco_translog_db_save().
        # This function saves database image and truncates the log file. Parameter 'snapshot_path' is required when snapshot_period > 0
        # Can be used together with 'max_size'
        # Default value 0 (don't snapshot periodically)
        snapshot_period : 5000,

        # Whether to apply saved TL files when the xsql is started. Possible values are:
        # 'yes'  - do apply ( mco_db_load(snapshot_path) + mco_translog_apply(log_path) ). If files are not exists or other error occure, don't start xsql
        # 'no'   - don't apply TL, just create the database with specified parameters
        # 'auto' - apply TL if snapshot_path and log_path files are exists
        # Default value 'auto'
        apply : auto,

        # Boolean parameter, if 'true' then TL is not started automatically (but is configured)
        # Default value 'false'
        stopped : false,
    },
    
    
    # Statistic & logging parameters
    xlog_params : {
        # Indicates turning the collection of the statistics on and off, boolean. The default value is 'true'.
        collect_stat : true, 
        
        # Indicates the size of the circular buffer for each client: how many statements are kept from the client are kept in the log, integer. The default value is 10.
        ringbuf_size : 25, 
        
        # Indicates the time in milliseconds to keep the logs after the client has been disconnected, integer. The default value is 60000 (one minute).
        max_inactive_time : 1200000, 
        
        # Indicates the maximum number of disconnected clients (even if the time specified in by the max_inactive_client has not passed yet), integer. Default value -1 (no limit)
        max_inactive_sessions : 100,
        
        
        # Indicates turning the server restarting on/off, boolean. The default value is 'true'.
        restart_server : true,
        
        # Indicates the maximum number of server restarts if the current server is terminated “too quickly “, integer. The default value is 3.
        restart_n_quick_fail : 3,
        
        # Indicates the time in milliseconds that is considered “too small”, integer. The default value is 10000 (10 seconds)
        # Thus if the server is up for less then specified time, it is restarted 'restart_n_quick_fail' times before the parent process is terminated
        restart_quick_fail_time : 10000, 

        # Size of signal stack context (Linux only), unsigned. Default is 8Mb
        alt_sig_stack_size : 16m,

        # Time interval (in milliseconds) to dump log files in case of fatal error, unsigned. Default value is 3*1000
        dump_wait_time : 5000,

    },

    # Global SSL parameters. These settings (except ca_file and ca_path) can be overridden
    # for the Cluster, HA and SQL Server(s) in their respective ssl_params subsections.
    ssl_params : {
        # List of ciphers to be used, as a string in OpenSSL format.
        # The default value is "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4".
        cipher_list : "HIGH:!aNULL:!kRSA:!PSK:!SRP:!MD5:!RC4",

        # Maximum size allowed for the peer's certificate chain, in bytes. The default value is 100 KiB.
        max_cert_list : 102400,

        # Peer verification mode flags.
        # The default value is [verify_peer, verify_fail_if_no_cert].
        # verify_none:
        #   server mode: client certificate won't be requested;
        #   client mode: when a non-anonymous cipher is used, the server certificate will be requested and verified, but the handshake will be continued regardless of the result.
        # verify_peer:
        #   server mode: client certificate will be requested and verified if provided; the handshake will proceed if the client does not provide its certificate;
        #   client mode: when a non-anonymous cipher is used, the server certificate will be requested and verified; the handshake will be aborted in case of verification failure.
        # verify_fail_if_no_cert (can only be used together with verify_peer):
        #   server mode: the handshake will be terminated if the client doesn't send its certificate;
        #   client mode: ignored.
        # verify_client_once (can only be used together with verify_peer):
        #   server mode: the client certificate will only be requested on initial TLS handshake (but not in case of renegotiation);
        #   client mode: ignored.
        verify_mode : [verify_peer, verify_fail_if_no_cert],

        # Maximum depth for the certificate chain verification. The default value is 100.
        verify_depth : 100,

        # Local certificate file in PEM format. The default value is none.
        cert_file_pem : "mycert.pem",

        # Local private key in PEM format. The default value is none.
        pkey_file_pem : "mykey.pem",

        # CA certificate chain file in PEM format. The default value is none.
        ca_file : "ca.pem",

        # Path to the CA certificate storage. The default value is none.
        # This directory should be set up as described on the OpenSSL SSL_CTX_load_verify_locations() man page.
        ca_path : "/home/user/ca",
    },

    # Tracing parameters
    trace_params : {
        # Specifies filename where tracing will go. It's possible to specify "stderr" to pass all the messages to standard error stream
        file : "xsql.log",

        # Format for the messages. Format specifiers are:
        #  %s
        #  %e
        format: "%s:  %e",

        # Maximum severity for messages apperared in trace file
        # Values are: debug, info, notice, warning, error, fatal
        severity: info

        # Options which controls if the file will be closed and reopened for each written record.
        # Values are: leave_opened, keep_closed
        options: keep_closed

        # File rotation policy. Possible values: no (default), size, time
        rotate_policy: no

        # File size when file is rotated when size rotation policy is used, bytes.
        # File is cut be lines, so this is a soft limit, exact file size will be slightly different.
        size_rotate_bytes: 1m

        # How often file rotation occurs when time rotation policy is used, bytes.
        # Actual rotation occurs when next recor is to be written to the file.
        time_rotate_period: 86400 # 1 day

    }

    # IoT (active replication) parameters
    iot_params : {
        # ID of this device/server, must be greater than 0 and unique for all agents. Also can be specified in MCO schema
        agent_id : 10024,

        # Maximum length of the listen socket queue. Default value is 0 (means system-defined maximum)
        listen_queue_size : 5,

        # Number of threads to process incoming request. Default value is 8
        n_callback_threads : 16,

        # Timeout for recv() operation, in milliseconds. Default value is 10*1000 (10 seconds)
        recv_timeout : 2000,

        # Timeout for send() operation, in milliseconds. Default value is 10*1000 (10 seconds)
        send_timeout : 2000,

        # Port number for service port. Default value 0 (means system-defined port number)
        wakeup_port : 9900,

        # Max number of cached database connections. Default value is (n_callback_threads + 2)
        conn_pool_size : 16,

        # Type of automatic synchronization (see 'sync_on_connect' and 'sync_period' below)
        # One of 'push' (send outgoing data), 'pull' (request for incoming data) or 'sync' (both push and pull). Default value is 'sync'
        sync_type : "pull",

        # Whether to remove outgoing data which were sended to and acked by recepient. Default value is false
        clear_on_ack : false,

        # Whether to perform synchronization immediately after the connection is established.
        # 'sync_type' defines the direction of replication. Default value is false.
        sync_on_connect : true,

        # Period in millisecond to automatically synchronize data. Default value is 0 (disable automatic replication)
        # 'sync_type' defines the direction of replication.
        sync_period : 120000,

        # Default compression level, 0..9, 0 - no compression
        compression_level: 0,

        # Array of connectors
        connections : [
            {
                # Type of connection, 'listen' (default) or 'connect'
                type : "listen",

                # Network address to connect or listen on
                address : "127.0.0.1:5000",

                # Timeout for connect() operation in milliseconds (has sense only for 'type' == "connect").
                # Default value is 2000 (2 seconds)
                connect_timeout : 2000,

                # Interval between connect() operations in milliseconds (has sense only for 'type' == "connect").
                # Default value is 60*1000 (1 minute)
                connect_interval : 10000,

                # Use local (UNIX) socket domain. Default value 'false'
                local_sockets : false,

                # Per-connection compression level, 0..9, 0 - no compression
                compression_level: 0,

                # SSL parameters. If this section is not specified, SSL is not used for the connection
                ssl_params : {   }
            },
        ],
    }
}

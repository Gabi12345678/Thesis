package com.mcobject.extremedb;

import java.util.*;
import java.io.*;
import java.math.BigDecimal;
import java.math.BigInteger;
import java.lang.reflect.*;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * eXtremeDB database wrapper.
 */
public class Database implements java.io.Closeable
{

    /**
     * Transaction types.
     */
    public enum TransactionType
    {
        ReadOnly,
        Update,
        ReadWrite,
        Exclusive
    };

    /**
     * Transaction priorities.
     */
    public enum TransactionPriority
    {
        Idle,
        Background,
        Foreground,
        AboveNormal,
        Highest
    };

    /**
     * Transaction isolation levels.
     */
    public enum IsolationLevel {
        Default,
        ReadCommitted,
        RepeatableRead,
        Serializable
    };

    /**
     * Transaction log types.
     */
    public enum LogType {
        NoLog,
        RedoLog,
        UndoLog
    };

    /**
     * Index kind
     */
    public enum IndexType {
        Hashtable,
        BTree,
        Patricia,
        RTree,
        RTreeOfPoint,
        Trigram
    }


    static final String[] indexTypeName = {"hash", "tree", "patricia", "rtree", "rtree", "trigram"};

    /**
     * Event types
     */
    public enum EventType {
        OnNew,
        OnFieldUpdate,
        OnDelete,
        OnDeleteAll,
        OnCheckpoint,
        OnUpdate
    }

    static final String[] eventTypeName = {"new", "update", "delete", "delete_all", "checkpoint", "update"};

    /**
     * Transaction commit policies (used only for on-disk databases).
     */
    public enum CommitPolicy {
        /**
         * Wait until all changes are saved on disk. (the default policy)
         */
        SyncFlush,

        /**
         * Do not flush changes to disk.
         */
        Buffered,   /* runtime buffered transactions */

        /**
         * Delay flushing changes to disk until the size of the changes or number of delayed transactions exceeds
		 * the specified threshold value.
         */
        Delayed,

        /**
         * Flush changes to disk asynchronously.
         */
        NoSync
    };

    /**
     * Scheduling policy for transactions with the same priority
     */
    public enum TransSchedPolicy
    {
        /**
         * Honest policy: first-in first-out
         */
         Fifo,

         /**
          * Place readers in queue before writers
          */
         ReaderFavor,

         /**
          * Place writers in queue  before readers
          */
         WriterFavor
    };

    public static final int Compression_ObjHead     = 0x01;
    public static final int Compression_ObjNode     = 0x02;
    public static final int Compression_BlobHead    = 0x40;
    public static final int Compression_BlobTail    = 0x80;
    public static final int Compression_FixedrecSet = 0x1000;
    public static final int Compression_All         = Compression_ObjHead | Compression_ObjNode | Compression_FixedrecSet
                                                            | Compression_BlobHead | Compression_BlobTail;

    /**
     * Automatically perform cleanup of a disk database after a crash. (remove all working copies.)
     * (This is appropriate only if using the MVCC transaction manager.)
     */
    public static final int DB_MODE_MVCC_AUTO_REPAIR = 0x01;

    /**
     * Sort objects by key before inserting into disk indexes.
     */
    public static final int DB_MODE_SMART_INDEX_INSERT = 0x2;

    /**
     * Open existing NVRAM database. By default INSTANCE_DUPLICATE is reported if opened database already exists.
     * With this flag it is not an error to have the database already registered.
     */
    public static final int DB_OPEN_EXISTING              = 0x04;

    /**
     * Calculate CRC of disk database pages
     */
    public static final int DB_USE_CRC_CHECK              = 0x08;

    /**
     * Mark class transient (not saved in disk database)
     */
    public static final int DB_TRANSIENT                  = 0x10;

    /**
     * Initialize memory pages on demand
     */
    public static final int DB_LAZY_MEM_INITIALIZATION    = 0x20;

    /**
     * Release MURSIW write lock before flushing changes to the disk
     */
    public static final int DB_MURSIW_DISK_COMMIT_OPTIMIZATION = 0x40;

    /**
     * Write all dirty unpinned pages to the disk at once (instead of writing individual page when it is thrown away from cache)
     */
    public static final int DB_BULK_WRITE_MODIFIED_PAGES = 0x80;

    /**
     * Calculate CRC only for first and last word of transaction body
     */
    public static final int DB_TRANS_LOG_FAST_CRC = 0x0100;

    public static final int DB_INDEX_PRELOAD = 0x0200;
    public static final int DB_DISABLE_NESTED_TRANSACTIONS = 0x0400;
    public static final int DB_DISABLE_IMPLICIT_ROLLBACK = 0x0800;
    public static final int DB_INMEMORY_PROTECTION = 0x1000;
    public static final int DB_INCLUSIVE_BTREE = 0x2000;
    public static final int DB_INMEMORY_COMPRESSION = 0x4000;
    public static final int DB_SEPARATE_BITMAP = 0x8000;
    public static final int DB_DISABLE_BTREE_REBALANCE_ON_DELETE = 0x10000;
    public static final int DB_AUTO_ROLLBACK_FIRST_PHASE = 0x20000;
    public static final int DB_MVCC_COMPATIBILITY_MODE = 0x40000;
    public static final int DB_DISABLE_PAGE_POOL_RESERVE = 0x80000;
    public static final int DB_REDO_LOG_OPTIMIZATION = 0x100000;
    public static final int DB_DISABLE_HOT_UPDATES = 0x200000;
    public static final int DB_SQL_AUTOCHECKPOINT = 0x400000;
    public static final int DB_MODE_READ_ONLY = 0x800000;
    public static final int DB_USE_AIO = 0x1000000;
    public static final int DB_INCREMENTAL_BACKUP = 0x2000000;

    /**
     * Multibyte string encoding. (This can be used in Parameters.stringEncoding and Encoding annotation.)
     */
    public static final String MULTIBYTE_ENCODING = "UTF-8";

    /**
     * Wide character string encoding. (This can be used in Parameters.stringEncoding and Encoding annotation.)
     */
    public static final String WIDE_CHARACTER_ENCODING = "UTF-16";

	/*
	 *  Actual loaded runtime information
	 */
	public static class RuntimeInfo
	{
        public byte    mco_version_major;
        public byte    mco_version_minor;
        public short   mco_build_number;

        public byte    mco_size_t;
        public byte    mco_offs_t;
        public boolean uint4_supported;
        public boolean float_supported;
        public byte    mco_checklevel;
        public boolean evaluation_version;
        public boolean large_database_supported;
        public boolean collation_supported;
        public boolean heap31_supported;
        public boolean bin_serialization_supported;
        public boolean fixedrec_supported;
        public boolean statistics_supported;
        public boolean events_supported;
        public boolean save_load_supported;
        public boolean object_initialization_supported;
        public boolean direct_index_field_access_supported;
        public boolean multiprocess_access_supported;
        public boolean object_repack_supported;
        public boolean transaction_logging_supported;
        public boolean cluster_supported;
        public boolean high_availability_supported;
        public boolean ha_multicast_supported;
        public boolean ha_incremental_replication_supported;
        public boolean binary_schema_evalution_supported;
        public boolean unicode_supported;
        public boolean wchar_supported;
        public boolean recovery_supported;
        public boolean disk_supported;
        public boolean direct_pointers_supported;
        public boolean persistent_object_supported;
        public boolean xml_import_export_supported;
        public boolean user_defined_index_supported;
        public boolean multifile_supported;
        public boolean multifile_descriptor_supported;
        public boolean two_phase_commit_supported;
        public boolean rtree_supported;
        public boolean tree_based_hash;
        public boolean tmgr_mvcc_async_cleanup;
        public boolean concurent_disk_btree;
        public boolean open_cursor_goto_first;
        public boolean smart_index_insert;
        public boolean btree_leaf_lock;
        public boolean null_statistics;
        public boolean implicit_runtime_start;
        public boolean bufferized_sync_iostream;
        public boolean async_replication;
        public boolean fast_transaction_list;
        public boolean extendable_dirty_page_bitmap;
        public byte    mursiw_policy;
        public byte    sync_capabilities;
        public byte    char_comparison_policy;
        public long    stream_buffer_size;
        public byte    max_db_instances;
        public byte    max_db_name_length;
        public byte    max_extends;
        public long    tl_page_buffer_size;
        public short   ha_max_replicas;
        public long    ha_transmit_buffer_size;
        public long    ha_syncronization_buffer_size;
        public long    default_redo_log_limit;
        public byte    mvcc_critical_sections;
        public byte    mvcc_per_index_locks;
        public short   con_disk_page_cache_size;
        public byte    small_con_cache_threshold;
        public long    extendable_dirty_page_bitmap_limit;
        public byte    max_vista_sessions;
        public boolean concurrent_write_transactions;
        public byte    encryption_support;
        public boolean backup_support;
        public String  mco_revision;
        public byte    mco_process_t;
	};

	/*
	 * Database time functions
	 */
	public interface GetCurrentTimeCallback {
		public int getCurrentTime();
	}

	public static void setCurrentTimeCallback(GetCurrentTimeCallback callback)
	{
		setCurrentTimeCallbackInt(callback);
	}



    /*
     * Runtime options
     */
    public static final int RT_OPTION_EXTHEAPS           = 0;
    public static final int RT_WINDOWS_SHM_OPT           = 1;
    public static final int RT_OPTION_MARK_LAST_OBJ      = 2;
    public static final int RT_OPTION_UNIX_SHM_MASK      = 3;
    public static final int RT_POSIX_SHM_OPT             = 4;
    public static final int RT_CRC_ALGORITHM             = 5;
    public static final int RT_MAX_PATRICIA_DEPTH        = 6;
    public static final int RT_MAX_DYNAMIC_PIPES         = 7;
    public static final int RT_OPTION_CLASS_LOAD_MERGE   = 8;
    public static final int RT_OPTION_DATETIME_PRECISION = 9;

    public static void setRuntimeOption(int option, int value)
    {
        setRuntimeOptionInt(option, value);
    }

    public static int  getRuntimeOption(int option)
    {
        return getRuntimeOptionInt(option);
    }


    public static class HAChannelInfo
    {
        public String  protocolType;
        public long    bytesSent;
        public long    bytesRcvd;
        public boolean supportUnreliable;
        public boolean asyncMode;

        public HAChannelInfo(String protocolType, long bytesSent, long bytesRcvd, boolean supportUnreliable, boolean asyncMode)
        {
            this.protocolType = protocolType;
            this.bytesSent = bytesSent;
            this.bytesRcvd = bytesRcvd;
            this.supportUnreliable = supportUnreliable;
            this.asyncMode = asyncMode;
        }

    };

    /** Cluster parameters */
    public static class ClusterNodeParams
    {
        public String addr;
        public int    qrank;

        public ClusterNodeParams(String addr, int qrank) {
            this.addr  = addr;
            this.qrank = qrank;
        };

        public ClusterNodeParams(String addr) {
            this(addr, 1);
        }
    };

    public static class ClusterNodeInfo
    {
        public String addr;
        public int qrank;
        public int nodeId;

        public ClusterNodeInfo(String addr, int qrank, int nodeId) {
            this.addr = addr;
            this.qrank = qrank;
            this.nodeId = nodeId;
        }
    }

    public static class ClusterWindow
    {
        public int                  length; /* in transactions */
        public int                  bsize;  /* in bytes */
        public int                  timeout;

        public ClusterWindow(int length, int bsize, int timeout) {
            this.length  = length;
            this.bsize   = bsize;
            this.timeout = timeout;
        };

        public ClusterWindow(int length) {
            this(length, 0, 1);
        };
    }

    public interface ClusterQuorumCallback {
        public boolean checkQuorum(int neighborIds[]);
    }

    public interface ClusterNotifying {
        public static final int NODE_CONNECT    = 0;
        public static final int NODE_DISCONNECT = 1;

        public void onNotify(int code, ClusterNodeInfo nodeInfo);
    }

    public abstract static class ClusterNWParams { };

    public static final int SOCK_INET_DOMAIN  = 0;
    public static final int SOCK_LOCAL_DOMAIN = 1;
    public static final int SOCK_SDP_DOMAIN   = 2;

    public static class ClusterTCPParams extends ClusterNWParams
    {
        public int                   socketSendBuf;
        public int                   socketRecvBuf;
        public int                   connectTimeout;
        public int                   connectInterval;
        public int                   socketDomain;
        public int                   keepAliveTime;
        public int                   keepAliveProbes;
        public int                   compressionLevel;
        public Database.SSLParameters sslParameters;

        public ClusterTCPParams() {
            socketSendBuf   = 0;
            socketRecvBuf   = 0;
            connectTimeout  = 5 * 1000;
            connectInterval = 200;
            socketDomain    = SOCK_INET_DOMAIN;
            keepAliveTime   = 1000;
            keepAliveProbes = 10;
            compressionLevel = 0;
            sslParameters = null;
        }
    }

    public static final int CLUSTER_MPI_BUSYWAIT         = 0x1;
    public static final int CLUSTER_MPI_SERIALIZED_SEND  = 0x2;

    public static class ClusterMPIParams extends ClusterNWParams
    {
        public int flags;
        public ClusterMPIParams() {
            flags = 0;
        }
    }

    public static final int CLUSTER_MODE_DEBUG_OUTPUT     = 0x1;
    public static final int CLUSTER_MODE_EARLY_DATA_SEND  = 0x2;
    public static final int CLUSTER_MODE_BINARY_EVOLUTION = 0x4;

    public static class ClusterParams
    {
        public ClusterNodeParams     nodes[];
        public int                   nodeId;
        public int                   connPoolFactor;
        public int                   syncMsgObjects;
        public int                   syncMsgSize;
        public int                   clusterSendBuf;
        public int                   clusterRecvBuf;
        public int                   mode;
        public ClusterWindow         window;
        public ClusterQuorumCallback quorumCallback;
        public ClusterNotifying      notifyCallback;
        public ClusterNWParams       nwParams;

        public ClusterParams(ClusterNodeParams nodes[], int nodeId) {
            this.nodes      = nodes;
            this.nodeId     = nodeId;
            connPoolFactor  = 50;
            syncMsgObjects  = 100;
            syncMsgSize     = 0;
            clusterSendBuf  = 0;
            clusterRecvBuf  = 0;
            mode            = 0;
            nwParams        = null;
        };
    };

    public static class ClusterInfo
    {
        public int  rTransCommit;
        public int  rTransRollback;
        public long bytesSent;
        public long bytesRecv;
        public int  nActiveNodes;
        public int  nodeId;

        ClusterInfo(int rTransCommit, int rTransRollback, long bytesSent, long bytesRecv, int nActiveNodes, int nodeId) {
            this.rTransCommit     = rTransCommit;
            this.rTransRollback   = rTransRollback;
            this.bytesSent        = bytesSent;
            this.bytesRecv        = bytesRecv;
            this.nActiveNodes     = nActiveNodes;
            this.nodeId           = nodeId;
        }
    };


    public enum BackupType {
        Auto, Snapshot, Incremental
    };

    public static final int BACKUP_FLAG_COMPRESSED = 1;
    public static final int BACKUP_FLAG_ENCRYPTED  = 2;

    public static class BackupInfo {
        public int        protocolVersion;
        public BackupType type;
        public int        flags;
        public int        backupNo;
        public long       timestamp;
        public long       size;
        public long       offset;
        public int        crc;
        public long       transNo;
        public int        memPageSize;
        public int        diskPageSize;
        public long       nPagesTotal;
        public String     dbName;
        public String     label;

        public BackupInfo(int protocolVersion, BackupType type, int flags, int backupNo, long timestamp, long size, long offset, int crc,
                            long transNo, int memPageSize, int diskPageSize, long nPagesTotal, String dbName, String label)
        {
            this.protocolVersion     = protocolVersion;
            this.type                = type;
            this.flags               = flags;
            this.backupNo            = backupNo;
            this.timestamp           = timestamp;
            this.size                = size;
            this.offset              = offset;
            this.crc                 = crc;
            this.transNo             = transNo;
            this.memPageSize         = memPageSize;
            this.diskPageSize        = diskPageSize;
            this.nPagesTotal         = nPagesTotal;
            this.dbName              = dbName;
            this.label               = label;
        }
    };


    /**
     * Parameters for the database open method.
     *
     */
    public static class Parameters
    {
        /**
         * Memory page size (default: 256).
         * When using a disk database, this should be a power of two and at least 8 times smaller than disk page size.
         */
        public int         memPageSize;

        /**
         * Disk page size (default: 0). Typically 4096 bytes; zero for all-in-memory databases.
         */
        public int         diskPageSize;

        /**
         * The maximum number of database connections (default: 100).
         */
        public int         maxConnections;

        /**
         * Recovery connection context (default: 0 - no recovery connection context)
         */
        public int         connectionContextSize;

        /**
         * The maximum size for a disk database (default: 0 - unlimited).
         */
        public long        maxDiskDatabaseSize;

        /**
         * Quantum of increasing size of database file (can help to reduce file fragmentation)
         */
        public long        diskFileExtensionQuantum;

        /**
         * The REDO transaction log threshold.  After reaching this a checkpoint is performed and the log is truncated.
         * Note that the checkpoint can be performed only after a transaction commit, so the size of the log file can become
         * larger than the specified limit. A zero value means to use the implementation specific default value.
         */
        public long        redoLogLimit;

        /**
         * The maximum size of uncommitted changes (delayed transaction commits) when using the CommitPolicy.Delayed policy
         * (default: 0 - 1/3 of the available disk cache).
         */
        public long        delayedCommitThreshold;

        /**
         * The maximum number of delayed transaction commits when using the CommitPolicy.Delayed policy (default: 0 - unlimited).
         */
        public int         maxDelayedTransactions;

        /**
         * Default commit policy for new connections (default : CommitPolicy.SyncFlush)
         */
        public CommitPolicy defaultCommitPolicy;

        /**
         * The transaction log type (default: LogType.RedoLog).
         */
        public LogType     logType;

        /**
         * The hash table load factor (default: 100%).  This is a criterion for hash table extension.
         */
        public int         hashLoadFactor;

        /**
         * The maximum number of active write transactions when using optimistic locking of B-Tree indexes (default: 0 - unlimited).
         */
        public int         indexOptimisticLockThreshold;

        /**
         * A combination of DB_MODE_* bits (default: 0).
         */
        public int         mode;

		/**
		 * The path to the file from which a database snapshot will be loaded at open time (default: null - no snapshot to load).
		 */
		public String      databaseSnapshotFilePath;

        /**
         * The default string encoding (string encoding can be sepcified using the Encoding annotation for each field separately)
         * (default: UTF-8).
         */
        public String      stringEncoding;

        /**
         * The classes to be stored in the database.
         * Only instances of the classes listed can be stored in the database.
         * This parameter has no default value and must be explicitly specified by the application.
         */
        public Class[]     classes;

        /**
         * Whether the classes specified in <code>classes</code> will be stored on disk.  Also disk classes may be marked
		 * explicitly using the disk() attribute of the Persistent annotation.
         */
        public boolean     diskClassesByDefault;

        /**
         * Whether classes specified in <code>classes</code> will be implicitly marked as compact. Also compact classes may be marked
		 * explicitly using the compact() attribute of Persistent annotation.
         */
        public boolean     compactClassesByDefault;

        /**
         * Whether classes specified in <code>classes</code> will be implicitly marked as nonatomic. Also nonatomic classes may be marked
		 * explicitly using the compact() attribute of Persistent annotation.
         */
        public boolean     nonatomicClassesByDefault;

        /**
         * Make dictionary compatible with mcocomp utility called with -nosort option
         */
        public boolean     dictionaryNoSort;

        /**
         * Maximal number of classes in the database: is needed to support dynamic table creation
         */
        public int         maxClasses;

        /**
         * Maximal number of indexes in the database: is needed to support dynamic table creation
         */
        public int         maxIndexes;

        /**
         * Maximal size of database dictionary: is needed to support dynamic table creation
         */
        public int         maxDictionarySize;

        /**
         * Cluster configuration parameters
         */
        public ClusterParams clusterParams;

        /**
         * eXtremeDB license key (if required)
         */
        public String      license_key;

        /**
         * Sheduling policy for transactions with the same priority
         */
        public TransSchedPolicy schedPolicy;

        /**
         * SQL workspace quota. Default is 0 - unlimited
         */
        public long        sqlWorkspaceLimit;

        /**
         * Compression level: 0..9, 0 - no compression, -1: default compression level
         */
        public int         compressionLevel;

        /**
         * Bitmap of page kinds which should be compressed
         */
        public int         compressionMask;

        /**
         * Use to allocate page map: virtual database space can not be larger than physical size compression ratio
         */
        public int         expectedCompressionRatio;

        /**
         * Database encryption key
         */
        public String      cipherKey;

        /**
        *  Size of backup counters array, bytes, power of two, ignored if disk_max_database_size set. Default is 0 - disable backup feature
        */
        public long        backupMapSize;

        /**
        *  Number of pages for last exclusive pass of backup procedure, set to zero to disable treshold. Default is 0
        */
        public int         backupMinPages;

        /**
        * Max number of passes before exclusive pass of backup procedure. Default is 10
        */
        public int         backupMaxPasses;


        /**
        * Name of a file will be used to store backup temporary data on mco_db_close() call.
        * OptionalStruct, set to null for default "<persistent-storage>.bmap" file locate at the same location as persistent storage file
        */
        public String      backupMapFile;


        /**
		*  Delay in milliseconds between writing backup blocks to minimize backup impact on performance
        */
        public int         fileBackupDelay;

        /**
         * Constructor with default values
         */
        public Parameters() {
            memPageSize = 256;
            diskPageSize = 4096;
            maxConnections = 100;
            connectionContextSize = 0;
            maxDiskDatabaseSize = 0;
            diskFileExtensionQuantum = 0;
            redoLogLimit = 16*1024*1024;
            delayedCommitThreshold = 0;
            maxDelayedTransactions = 0;
            defaultCommitPolicy = CommitPolicy.SyncFlush;
            logType = LogType.RedoLog;
            hashLoadFactor = 100;
            indexOptimisticLockThreshold = 100;
            stringEncoding = "UTF-8";
            schedPolicy = TransSchedPolicy.Fifo;
            sqlWorkspaceLimit = 0;
            compressionLevel = -1;
            compressionMask = Compression_All;
            expectedCompressionRatio = 10;
            backupMaxPasses = 10;
            backupMinPages  = 0;
            backupMapSize   = 0;
            backupMapFile   = null;
        }
    }

    /*
        Parameters for setting up SSL connections
     */
    public static class SSLParameters {
        public String cipherList;
        public long maxCertList;
        public long options;
        public int verifyMode;
        public int verifyDepth;
        public String certificateFile;
        public String privateKeyFile;

        public static final long OPT_NO_SSLV2 = 0x01;
        public static final long OPT_NO_SSLV3 = 0x02;
        public static final long OPT_NO_TLSV1_0 = 0x04;
        public static final long OPT_NO_TLSV1_1 = 0x08;
        public static final long OPT_NO_TLSV1_2 = 0x10;
        public static final long OPT_NO_COMPRESSION = 0x20;
        public static final long OPT_SINGLE_DH_USE = 0x40;

        public static final int VERIFY_NONE = 0x0;
        public static final int VERIFY_PEER = 0x01;
        public static final int VERIFY_FAIL_IF_NO_PEER_CERT = 0x02;
        public static final int VERIFY_CLIENT_ONCE = 0x04;

        public  SSLParameters() {
            cipherList = null;
            maxCertList = 102400;
            options = OPT_NO_SSLV2 | OPT_NO_SSLV3 | OPT_NO_COMPRESSION | OPT_SINGLE_DH_USE;
            verifyMode = VERIFY_PEER | VERIFY_FAIL_IF_NO_PEER_CERT;
            verifyDepth = 100;
            certificateFile = null;
            privateKeyFile = null;
        }
    }

    /**
     * An abstract database device (a volatile or non-volatile memory resource).
     */
    public abstract static class Device {
        /**
         * The assignment of this device.
         */
        public enum Kind {
            /**
             * This device is used for database data (disk or memory).
             */
            Data,

            /**
             * This device is used for the disk cache.
             */
            DiskCache,

            /**
             * This device is used for the transaction log.
             */
            TransactionLog,

            /**
             * This device is used as async buffer for HA.
             */
            AsyncBuffer,

            /**
             * This device is used as pipe buffer for TL.
             */
            PipeBuffer
        };

        protected Device(Kind kind) {
            this.kind = kind.ordinal();
        }

        int kind;
    }

    /**
     * An in-process private memory segment (allocated using malloc).
     */
    public static class PrivateMemoryDevice extends Device {
        /**
         * The private in-process memory device constructor.
         * @param kind the device assgnment.
         * @param size the region size.
         */
         public PrivateMemoryDevice(Kind kind, long size) {
            super(kind);
            this.size = size;
        }

        long  size;
        long  allocatedMemory;
    }

    /**
     * A shared memory region (allocated using mmap or a similar system call).
     */
    public static class SharedMemoryDevice extends Device {
        /**
         * The shared memory device constructor.
         * @param kind the device assgnment.
         * @param name the segment name.
         * @param hint the proposed region address.
         * @param size the region size.
         */
        public SharedMemoryDevice(Kind kind, String name, long hint, long size) {
            super(kind);
            this.name = name;
            this.hint = hint;
            this.size = size;
        }

        String name;
        long   hint;
        long   size;
    }

    /**
     * An Operating Systm file device.
     */
    public static class FileDevice extends Device {
        /**
         * The file device constructor.
         * @param kind the device assignment.
         * @param path the file path.
         */
        public FileDevice(Kind kind, String path) {
            super(kind);
            this.path = path;
        }

        public String path;
    }

    /**
     * A multifile segment.  A multifile consists of several physical files.  The size of each segment except the last is fixed,
	 * the last segment can be extended indefinitely.
     */
    public static class MultiFileDevice extends FileDevice {
        /**
         * The multifile device constructor.
         * @param kind the device assgnment.
         * @param path the file path.
         * @param size the multifile segment size (ignored for last segment).
         */
        public MultiFileDevice(Kind kind, String path, long size) {
            super(kind, path);
            this.size = size;
        }
        long   size;
    }

    /**
     * A RAID segment.
     */
    public static class RaidDevice extends FileDevice {
        /**
         * The RAID device constructor.
         * @param kind the device assgnment.
         * @param path the file path.
         * @param raidLevel 0 - (RAID-0: striping) or 1 (RAID-1: mirroring) .
         */
        public RaidDevice(Kind kind, String path, int raidLevel) {
            super(kind, path);
            this.raidLevel = raidLevel;
        }
        int raidLevel;
    }

    /**
     * The open database method.
     * @param name the database name (used to identify the database instance in the system).
     * @param params the open parameters.
     * @param devices the list of database devices.
     */
    public void open(String name, Parameters params, Device[] devices) {
        this.name = name;
        defaultEncoding = params.stringEncoding;
        buildDictionary(params);
        open(name, structsList.toArray(), classes, hasInmemDistribClasses, hasDiskDistribClasses, totalFields, totalIndices, totalKeys, events.size(), devices, params);
    }

    /**
     * Open an all-in-memory database .
     * @param name the database name (used to identify database instance in the system).
     * @param params the open parameters.
     * @param size the size of the database segment in memory.
     */
    public void open(String name, Parameters params, long size) {
        open(name, params, new Device[]{new PrivateMemoryDevice(Device.Kind.Data, size)});
    }

    /*
     *
     */
    public RuntimeInfo getRunTimeInfo() {
    	RuntimeInfo ret = getRuntimeInfo();
		return ret;
	}

    private static String mcoStructName(String name) {
        return name.substring(name.lastIndexOf('.')+1);
    }

    private void printFieldDeclaration(PrintStream out, FieldDescriptor f) {
        out.print("    ");
        if ((f.flags & MCO_DICT_FLDF_NULLABLE) != 0) {
            out.print("nullable ");
        }
        if ((f.flags & MCO_DICT_FLDF_VECTOR) != 0) {
            if ((f.flags &  MCO_DICT_FLDF_NUMERIC) != 0) {
                out.print("vector< numeric<" + f.width + "," + f.precision + "> > " + f.name);
            } else {
                switch (f.type) {
                case MCO_DB_FT_UINT2:
                    out.print("vector<jchar> " + f.name);
                    break;
                case MCO_DB_FT_INT1:
                    out.print("vector<jbyte> " + f.name);
                    break;
                case MCO_DB_FT_INT2:
                    out.print("vector<jshort> " + f.name);
                    break;
                case MCO_DB_FT_INT4:
                    out.print("vector<jint> " + f.name);
                    break;
                case MCO_DB_FT_INT8:
                    out.print("vector<jlong> " + f.name);
                    break;
                case MCO_DB_FT_BOOL:
                    out.print("vector<boolean> " + f.name);
                    break;
                case MCO_DB_FT_FLOAT:
                    out.print("vector<float> " + f.name);
                    break;
                case MCO_DB_FT_DOUBLE:
                    out.print("vector<double> " + f.name);
                    break;
                case MCO_DB_FT_DATETIME:
                    out.print("vector<datetime> " + f.name);
                    break;
                case MCO_DB_FT_CHARS:
                    out.print("vector<char<" + f.size + ">> " + f.name);
                    break;
                case MCO_DB_FT_STRING:
                    out.print("vector<string> " + f.name);
                    break;
                case MCO_DB_FT_UNICODE_CHARS:
                    out.print("vector<nchar<" + (f.size/2) + ">> " + f.name);
                    break;
                case MCO_DB_FT_UNICODE_STRING:
                    out.print("vector<nstring> " + f.name);
                    break;
                case MCO_DB_FT_STRUCT:
                    Class c = f.components.cls;
                    out.print("vector<" + mcoStructName(c.getName()) + "> " + f.name);
                    break;
                default:
                    assert(false);
                }
            }
        } else {
            if ((f.flags &  MCO_DICT_FLDF_NUMERIC) != 0) {
                out.print("numeric<" + f.width + "," + f.precision + "> " + f.name);
            } else {
                switch (f.type) {
                case MCO_DB_FT_SEQUENCE_INT1:
                    out.print("sequence<jbyte> " + f.name);
                    break;
                case MCO_DB_FT_SEQUENCE_UINT1:
                    out.print("sequence<ubyte> " + f.name);
                    break;
                case MCO_DB_FT_SEQUENCE_UINT2:
                    out.print("sequence<jchar> " + f.name);
                    break;
                case MCO_DB_FT_SEQUENCE_INT2:
                    out.print("sequence<jshort> " + f.name);
                    break;
                case MCO_DB_FT_SEQUENCE_UINT4:
                    out.print("sequence<uint> " + f.name);
                    break;
                case MCO_DB_FT_SEQUENCE_INT4:
                    out.print("sequence<jint> " + f.name);
                    break;
                case MCO_DB_FT_SEQUENCE_UINT8:
                    out.print("sequence<ulong> " + f.name);
                    break;
                case MCO_DB_FT_SEQUENCE_INT8:
                    out.print("sequence<jlong> " + f.name);
                    break;
                case MCO_DB_FT_SEQUENCE_FLOAT:
                    out.print("sequence<float> " + f.name);
                    break;
                case MCO_DB_FT_SEQUENCE_DOUBLE:
                    out.print("sequence<double> " + f.name);
                    break;
                case MCO_DB_FT_SEQUENCE_CHAR:
                    out.print("sequence<char<" + f.seqElemSize + ">> " + f.name);
                    break;
                case MCO_DB_FT_SEQUENCE_DATETIME:
                	out.print("sequence<datetime> " + f.name);
                	break;
                case MCO_DB_FT_UINT2:
                    out.print("jchar " + f.name);
                    break;
                case MCO_DB_FT_INT1:
                    out.print("jbyte " + f.name);
                    break;
                case MCO_DB_FT_INT2:
                    out.print("jshort " + f.name);
                    break;
                case MCO_DB_FT_INT4:
                    out.print("jint " + f.name);
                    break;
                case MCO_DB_FT_INT8:
                    if (f.refCls < 0) {
                        out.print("jlong " + f.name);
                    } else {
                        out.print("autoid_t<" + classes[f.refCls-1].name + "> " + f.name);
                    }
                    break;
                case MCO_DB_FT_BOOL:
                    out.print("boolean " + f.name);
                    break;
                case MCO_DB_FT_FLOAT:
                    out.print("float " + f.name);
                    break;
                case MCO_DB_FT_DOUBLE:
                    out.print("double " + f.name);
                    break;
                case MCO_DB_FT_DATETIME:
                    out.print("datetime " + f.name);
                    break;
                case MCO_DB_FT_CHARS:
                    out.print("char<" + f.size + "> " + f.name);
                    break;
                case MCO_DB_FT_STRING:
                    out.print("string " + f.name);
                    break;
                case MCO_DB_FT_UNICODE_CHARS:
                    out.print("nchar<" + (f.size/2) + "> " + f.name);
                    break;
                case MCO_DB_FT_UNICODE_STRING:
                    out.print("nstring " + f.name);
                    break;
                case MCO_DB_FT_STRUCT:
                    if ((f.flags & MCO_DICT_FLDF_OPTIONAL) != 0) {
                        out.print("optional ");
                    }
                    Class c = f.fld.getType();
                    if (f.arraySize != 0) {
                        c = c.getComponentType();
                    }
                    out.print(mcoStructName(c.getName()) + " " + f.name);
                    break;
                case MCO_DB_FT_BLOB:
                    out.print("blob " + f.name);
                    break;
                default:
                    assert(false);
                }
            }
            if (f.arraySize != 0) {
                out.print("[" + f.arraySize + "]");
            }
        }
        out.println(";");
    }


    /**
     * Generate an eXtremeDB MCO file based on the Java class definitions.
     */
    public void generateMcoFile(String path) throws IOException
    {
        PrintStream out = new PrintStream(new FileOutputStream(path));
        out.println("#define jbyte  signed<1>");
        out.println("#define jshort signed<2>");
        out.println("#define jint   signed<4>");
        out.println("#define jlong  signed<8>");
        out.println("#define jchar  unsigned<2>");
        out.println("#define ubyte  unsigned<1>");
        out.println("#define uint   unsigned<4>");
        out.println("#define ulong  unsigned<8>");
        out.println("#define HASH_SIZE 10000");
        out.println("declare database " + name + ";");
        if ((config & (MCO_CFG_HA_SUPPORT|MCO_CFG_CLUSTER_SUPPORT|MCO_CFG_MPI_CLUSTER_SUPPORT|MCO_CFG_TL_SUPPORT)) != 0)
        {
            out.println("declare auto_oid[HASH_SIZE];");
        }
        for (StructDescriptor s : structsList) {
            ClassDescriptor c = null;
            if (s instanceof ClassDescriptor) {
                c = (ClassDescriptor)s;
                if ((s.flags & MCO_DB_TYPINFO_PERSISTENT) != 0) {
                    out.println("persistent ");
                }
                if ((s.flags & MCO_DB_TYPINFO_NONATOMIC) != 0) {
                    out.println("nonatomic ");
                }
                if ((s.flags & MCO_DB_TYPINFO_LOCAL) != 0) {
                    out.println("local ");
                }
                if ((s.flags & MCO_DB_TYPINFO_DISTRIBUTED) != 0) {
                    out.println("distributed ");
                }
                out.println("class " + s.name + " {");
                if (c.autoIdIndexNo >= 0) {
                    out.println("    autoid[HASH_SIZE];");
                }
            } else {
                out.println("struct " + s.name + " {");
            }
            for (FieldDescriptor f : s.fields) {
                printFieldDeclaration(out, f);
            }
            if (c != null) {
                for (EventDescriptor e : c.events) {
                    out.print("    event<");
                    if (e.type == EventType.OnFieldUpdate) {
                        out.print(e.fieldName);
                        out.print(" update> ");
                    } else {
                        out.print(eventTypeName[e.type.ordinal()]);
                        out.print("> ");
                    }
                    out.print(e.name);
                    out.println(";");
                }
                if (c.listIndexNo >= 0) {
                    out.println("    list;");
                }
                for (IndexDescriptor idx : c.indices) {
                    out.print("    ");
                    if (idx.unique) {
                        out.print("unique ");
                    } else if (idx.thick) {
                        out.print("thick ");
                    }
                    out.print(indexTypeName[idx.type.ordinal()]);
                    String sep = "<";
                    for (int k = 0; k < idx.keys.length; k++) {
                        KeyDescriptor key = idx.keys[k];
                        out.print(sep);
                        out.print(key.name);
                        if (key.descending) {
                            out.print(" desc");
                        }
                        sep = ", ";
                    }
                    out.print("> " + idx.name.replace('.', '_') + "_index");
                    if (idx.type == IndexType.Hashtable) {
                        out.print("[" + idx.size + "]");
                    }
                    out.println(";");
                }
                if (c.ttlCount > 0) {
                    out.print("    TTL_count ");
                    out.print(c.ttlCount);
                    out.println(";");
                }
                if (c.ttlClockTime > 0) {
                    out.print("    TTL_clock_time ");
                    out.print(c.ttlClockTime);
                    out.println(" us;");
                }
            }
            out.println("};");
        }
        out.close();
    }


    /**
     * Close the database.
     */
    public void close() {
        if (name != null) {
            String n = name;
            name = null; // To prevent double-closing if mco_db_close failed
            close(n);
        }
    }

    protected void finalize() {
        close();
    }


    /**
     * Extend the database
     */
    public void extend(Device device) {
        if (name == null) {
            throw new IllegalArgumentException("Database is not opened");
        }
        extend(name, device, null);
    }


    /**
     * Extend the database with context
     */
    public void extend(Device device, byte [] context) {
        if (name == null) {
            throw new IllegalArgumentException("Database is not opened");
        }
        extend(name, device, context);
    }


    /**
     * Detach device from the database
     */
    public boolean detach(Device device) {
        if (name == null) {
            throw new IllegalArgumentException("Database is not opened");
        }
        return detach(name, device, null);
    }

    /**
     * Detach device from the database with context
     */
    public boolean detach(Device device, byte [] context) {
        if (name == null) {
            throw new IllegalArgumentException("Database is not opened");
        }
        return detach(name, device, context);
    }

    /**
     * Create a new database connection.
     * Each thread should use its own database connection.
     * @return the new connection.
     */
    public Connection connect() {
        return new Connection(this);
    }

    /**
     * Create a new database connection.
     * Each thread should use its own database connection.
     * @param context context for opening database
     * @return the new connection.
     */
    public Connection connect(byte [] context) {
        return new Connection(this, context);
    }

    /**
     * Create a new database connection for already created database.
     *
     * @param name the existed database name.
     * @return the new connection.
     */
    public Connection connect(String name) {
        this.name = name;
        Connection con = new Connection(this);
        this.name = null; // avoid closing database in close() method
        return con;
    }

    /**
     * Create a new database connection for already created database.
     *
     * @param name the existed database name.
     * @param context context for opening database
     * @return the new connection.
     */
    public Connection connect(String name, byte[] context) {
        this.name = name;
        Connection con = new Connection(this, context);
        this.name = null; // avoid closing database in close() method
        return con;
    }
    /**
     * Create a new Sql local database connection.
     * Each thread should use its own database connection.
     * @return the new connection.
     */
    public SqlLocalConnection connectSql() {
        return new SqlLocalConnection(this);
    }

    public static final int MCO_CFG_DEBUG_LIBRARY = 1;
    public static final int MCO_CFG_MVCC_TRANSACTION_MANAGER = 2;
    public static final int MCO_CFG_SHARED_MEMORY = 4;
    public static final int MCO_CFG_DISK_SUPPORT = 8;
    public static final int MCO_CFG_HA_SUPPORT = 16;
    public static final int MCO_CFG_CLUSTER_SUPPORT = 32;
    public static final int MCO_CFG_MPI_CLUSTER_SUPPORT = 64;
    public static final int MCO_CFG_TL_SUPPORT = 128;
    public static final int MCO_CFG_SQL_SUPPORT = 256;
    public static final int MCO_CFG_SEQUENCE_SUPPORT = 512;
    public static final int MCO_CFG_LOAD_ASSEMBLY = 1024;
    public static final int MCO_CFG_USE_POSIX_LIBRARIES = 2048;
    public static final int MCO_CFG_DISK_COMPRESSION = 4096;

    private static boolean forceAssembly() {
        return (System.getProperty("os.name").toLowerCase().indexOf("aix") >= 0 &&
                 System.getProperty("java.vm.name").toLowerCase().indexOf("ibm") >= 0);
    }

    private static AtomicInteger rtStop;

    /**
     * The database constructor.
     * @param config the combination of MCO_CFG_* bits.
     */
    public Database(int config) {
        Runtime rt = Runtime.getRuntime();
        StringBuffer buf = new StringBuffer();
        buf.append("extremedbjni");

        if ((config & MCO_CFG_MPI_CLUSTER_SUPPORT) != 0) {
            config |= MCO_CFG_CLUSTER_SUPPORT;
        }
        /* Force using MVCC for cluster configuration */
        if ((config & MCO_CFG_CLUSTER_SUPPORT) != 0) {
            config |= MCO_CFG_MVCC_TRANSACTION_MANAGER;
        }
        if (forceAssembly()) {
            config |= MCO_CFG_LOAD_ASSEMBLY;
        }
        this.config = config;
        String vm = System.getProperty("java.vm.name");
        String os = System.getProperty("os.arch");
        String osname = System.getProperty("os.name");
        String javaPath = System.getProperty("java.library.path");
        Map<String, String> env = System.getenv();
        String lib_path = (!osname.toLowerCase().contains("windows") && env.containsKey("MCO_LIBRARY_PATH"))
            ? env.get("MCO_LIBRARY_PATH") + "/" : "";
        String lib_extension = (osname.toLowerCase().contains("mac") || osname.toLowerCase().contains("darwin")) ? ".jnilib" : ".so";
        String disk_err_msg = new String();
        if ((config & MCO_CFG_DISK_SUPPORT) != 0) {
            disk_err_msg += "\nFailure to load the disk manager library. Note that only Fusion and Financial Edition installations of "
                            + "eXtremeDB support persistent storage using the disk manager.";
        }

        if ((config & MCO_CFG_LOAD_ASSEMBLY) != 0) {
            if ((config & MCO_CFG_DISK_SUPPORT) != 0) {
                buf.append("_disk");
            }
            buf.append((config & MCO_CFG_MVCC_TRANSACTION_MANAGER) != 0 ? "_mvcc" : "_mursiw");
            if ((config & MCO_CFG_SHARED_MEMORY) != 0) {
                buf.append("_shm");
            }
            if ((config & MCO_CFG_MPI_CLUSTER_SUPPORT) != 0) {
                buf.append("_clmpi");
            }
            if ((config & MCO_CFG_DEBUG_LIBRARY) != 0) {
                buf.append("_debug");
            }
            String library = buf.toString();
            try {
                //System.out.println("Try to load library " + buf);
                if (lib_path.isEmpty())
                    rt.loadLibrary(library);
                else {
                    File f = new File(lib_path + "lib" + library + lib_extension);
                    rt.load(f.getAbsolutePath());
                }
            } catch (Throwable x) {
                if (lib_path.isEmpty())
                    throw new LinkageError(vm + " at " + os + " failed to load library '" + library + "' using java.library.path='" + javaPath + "': " + x.getMessage() + disk_err_msg);
                else
                    throw new LinkageError(vm + " at " + os + " failed to load library '" + lib_path + "lib" + library + lib_extension + "' using MCO_LIBRARY_PATH='" + lib_path + "': " + x.getMessage() + disk_err_msg);
            }
        } else {
            String library = ((config & MCO_CFG_DEBUG_LIBRARY) != 0 ? "extremedbjni_debug" : "extremedbjni");
            try {
                if (lib_path.isEmpty())
                    rt.loadLibrary(library);
                else {
                    File f = new File(lib_path + "lib" + library + lib_extension);
                    rt.load(f.getAbsolutePath());
                }
                loadDependentLibraries(config);
            } catch (RuntimeException x) {
                if (lib_path.isEmpty())
                    throw new LinkageError(vm + " at " + os + " failed to load library '" + library + "' using java.library.path='" + javaPath + "': " + x.getMessage() + disk_err_msg);
                else
                    throw new LinkageError(vm + " at " + os + " failed to load library '" + lib_path + "lib" + library + lib_extension + "' using MCO_LIBRARY_PATH='" + lib_path + "': " + x.getMessage() + disk_err_msg);
            }
        }
        runtimeStart(config);
        rtStop = new AtomicInteger(0);
        rt.addShutdownHook(new Thread(new Runnable() { public void run() { syncRuntimeStop(); }}));
    }

    static void syncRuntimeStop()
    {
        if (rtStop.incrementAndGet() == 1)
            runtimeStop();
    }

    static {
        try {
            Class.forName("sun.misc.Unsafe");
            String cls = "com.mcobject.extremedb.sun14.Sun14ReflectionProvider";
            reflectionProvider = (ReflectionProvider)Class.forName(cls).newInstance();
        } catch (Throwable x) {
            //x.printStackTrace();
            reflectionProvider = new StandardReflectionProvider();
        }
    }

    /**
     * The database constructor with the default library configuration.
     */
    public Database() {
        this(0);
    }

    /**
     * The database constructor for secondary databases opening.
     * It inherits library configuration from a previously opened database.
     */
    public Database(Database db) {
        this.config = db.config;
    }

    static class EventDescriptor {
        int id;
        String name;
        Database.EventType type;
        ClassDescriptor scope;
        FieldDescriptor field;
        String fieldName;

        EventDescriptor(String name, Database.EventType type, ClassDescriptor scope, int id) {
            this.name = name;
            this.type = type;
            this.scope = scope;
            this.id = id;
        }
    }

    static class StructDescriptor {
        Class cls;
        String name;
        int structNo;
        FieldDescriptor[] fields;
        int size;
        int alignment;
        int compactSize;
        int compactAlignment;
        int flags;
        Constructor loadConstructor;

        void setColumnNo(int colNo) {
            for (int i = 0; i < fields.length; i++) {
                FieldDescriptor fld = fields[i];
                fld.colNo = colNo;
                if (fld.components != null) {
                    fld.components.setColumnNo(colNo);
                }
            }
        }

        void calculateCompactClassOffsets(int classOffset) {
            for (int i = 0; i < fields.length; i++) {
                FieldDescriptor fld = fields[i];
                fld.classOffset = classOffset + fld.compactOffset;
                if (fld.components != null) {
                    fld.components.calculateCompactClassOffsets(fld.classOffset);
                }
            }
        }

        void calculateClassOffsets(int classOffset) {
            for (int i = 0; i < fields.length; i++) {
                FieldDescriptor fld = fields[i];
                fld.classOffset = classOffset + fld.offset;
                if (fld.components != null) {
                    fld.components.calculateClassOffsets(fld.classOffset);
                }
            }
        }

        StructDescriptor(Class c) {
            cls = c;
            name = mcoStructName(c.getName());
            try {
                loadConstructor = reflectionProvider.getDefaultConstructor(c);
            } catch (Exception x) {
                throw new RuntimeException(x);
            }
            try {
                loadConstructor.setAccessible(true);
            } catch (Exception x) {}
        }
    }

    static class ClassDescriptor extends StructDescriptor {
        HashMap<String, IndexDescriptor> indicesMap;
        IndexDescriptor[] indices;
        int classNo;
        int listIndexNo;
        int autoIdIndexNo;
        int initSize;
        EventDescriptor[] events;
        long ttlCount;
        long ttlClockTime;

        void setColumnNo() {
            for (int i = 0; i < fields.length; i++) {
                FieldDescriptor fld = fields[i];
                fld.colNo = fld.fieldNo;
                if (fld.components != null) {
                    fld.components.setColumnNo(fld.colNo);
                }
            }
        }

        ClassDescriptor(Class c) {
            super(c);
        }
    }

    static class FieldDescriptor implements Comparable<FieldDescriptor>
    {
        StructDescriptor owner;
        String name;
        Field fld;
        int refCls;
        int fieldNo;
        int colNo;
        int type;
        int flags;
        int arraySize;
        int fixedSize;
        int size;
        int offset;
        int alignment;
        int compactSize;
        int compactOffset;
        int compactAlignment;
        int classOffset;
        int indicatorNo;
        int indicatorOffset;
        int indicatorCompactOffset;
        boolean isList;
        StructDescriptor components;
        String encoding;
        EventDescriptor onUpdateEvent;
        int width;
        int precision;
        int seqOrder;
        int seqElemSize;
        Class enumClass;

        public long scale(BigDecimal dec) {
            long val = dec.unscaledValue().longValue();
            int prec = dec.scale();
            while (prec < precision) {
                val *= 10;
                prec += 1;
            }
            while (prec > precision) {
                val /= 10;
                prec -= 1;
            }
            return val;
        }

        public int compareTo(FieldDescriptor fd) {
            return alignment > fd.alignment ? -1 : alignment < fd.alignment ? 1 : name.compareTo(fd.name);
        }

    }

    static class KeyDescriptor {
        String name;
        FieldDescriptor field;
        boolean descending;

        KeyDescriptor(String name, FieldDescriptor field, boolean descending) {
            this.name = name;
            this.field = field;
            this.descending = descending;
        }
    }

    static class IndexDescriptor {
        String name;
        KeyDescriptor[] keys;
        int indexNo;
        int flags;
        boolean unique;
        boolean thick;
        IndexType type;
        long size;
    }

    final static int MCO_DB_FT_NONE =       0;
    final static int MCO_DB_FT_UINT1 =      1;
    final static int MCO_DB_FT_UINT2 =      2;
    final static int MCO_DB_FT_UINT4 =      3;
    final static int MCO_DB_FT_INT1 =       4;
    final static int MCO_DB_FT_INT2 =       5;
    final static int MCO_DB_FT_INT4 =       6;
    final static int MCO_DB_FT_CHARS =      7;
    final static int MCO_DB_FT_STRING =     8;
    final static int MCO_DB_FT_REF =        9;
    final static int MCO_DB_FT_FLOAT =      10;
    final static int MCO_DB_FT_DOUBLE =     11;
    final static int MCO_DB_FT_UINT8 =      12;
    final static int MCO_DB_FT_INT8 =       13;
    final static int MCO_DB_FT_AUTOID =   14 /* 8 byte =  */;
    final static int MCO_DB_FT_OBJVERS =  15 /* 2 byte =  */;
    final static int MCO_DB_FT_DATE =       16 ;
    final static int MCO_DB_FT_TIME =       17 ;
    final static int MCO_DB_FT_AUTOOID =    18 ;
    final static int MCO_DB_FT_UNICODE_CHARS =      19;
    final static int MCO_DB_FT_UNICODE_STRING =     20;
    final static int MCO_DB_FT_WIDE_CHARS =         21;
    final static int MCO_DB_FT_WCHAR_STRING =       22;
    final static int MCO_DB_FT_BOOL =   23;
    final static int MCO_DB_FT_DATETIME =   24;

    final static int MCO_DB_FT_SEQUENCE_UINT1 = 30;
    final static int MCO_DB_FT_SEQUENCE_UINT2 = 31;
    final static int MCO_DB_FT_SEQUENCE_UINT4 = 32;
    final static int MCO_DB_FT_SEQUENCE_UINT8 = 33;
    final static int MCO_DB_FT_SEQUENCE_INT1  = 34;
    final static int MCO_DB_FT_SEQUENCE_INT2  = 35;
    final static int MCO_DB_FT_SEQUENCE_INT4  = 36;
    final static int MCO_DB_FT_SEQUENCE_INT8  = 37;
    final static int MCO_DB_FT_SEQUENCE_FLOAT = 38;
    final static int MCO_DB_FT_SEQUENCE_DOUBLE = 39;
    final static int MCO_DB_FT_SEQUENCE_CHAR = 40;
    final static int MCO_DB_FT_SEQUENCE_DATETIME = 41;

    final static int MCO_DB_FT_STRUCT =     50;
    final static int MCO_DB_FT_BLOB =       51;

    final static int MCO_DICT_FLDF_VECTOR =       0x01;
    final static int MCO_DICT_FLDF_ARRAY =        0x02;
    final static int MCO_DICT_FLDF_OPTIONAL =     0x04;
    final static int MCO_DICT_FLDF_INDEXED =      0x08;
    final static int MCO_DICT_FLDF_HIDDEN   =     0x10;
    final static int MCO_DICT_FLDF_NULLABLE =     0x20;
    final static int MCO_DICT_FLDF_NULL_INDICATOR = 0x40;
    final static int MCO_DICT_FLDF_NUMERIC        = 0x80;

    final static int MCO_DB_TYPINFO_HAS_LIST =      0x0001;
    final static int MCO_DB_TYPINFO_HAS_OID =       0x0002;
    final static int MCO_DB_TYPINFO_HAS_BLOBS =     0x0004;
    final static int MCO_DB_TYPINFO_COMPACT =       0x0008;
    final static int MCO_DB_TYPINFO_FIXEDREC =      0x0010;
    final static int MCO_DB_TYPINFO_PERSISTENT =    0x0020;
    final static int MCO_DB_TYPINFO_HAS_AUTOID =    0x0040;
    final static int MCO_DB_TYPINFO_HAS_HISTORY =   0x0080;
    final static int MCO_DB_TYPINFO_HAS_EVENTS =    0x0100;
    final static int MCO_DB_TYPINFO_HAS_SEQUENCES = 0x0200;
    final static int MCO_DB_TYPINFO_LOCAL =         0x0400;
    final static int MCO_DB_TYPINFO_DISTRIBUTED =   0x0800;
    final static int MCO_DB_TYPINFO_HIDDEN =        0x1000;
    final static int MCO_DB_TYPINFO_NONATOMIC =     0x2000;

    final static int MCO_DB_INDF_UNIQUE =           0x0004;
    final static int MCO_DB_INDF_VSTRUCT_BASED =    0x0008;
    final static int MCO_DB_INDF_VTYPE_BASED =      0x0010;
    final static int MCO_DB_INDF_V_BASED =        (MCO_DB_INDF_VSTRUCT_BASED | MCO_DB_INDF_VTYPE_BASED);
    final static int MCO_DB_INDF_PERSISTENT =       0x0020;
    final static int MCO_DB_INDF_VOLUNTARY =        0x0040;
    final static int MCO_DB_INDF_ASTRUCT_BASED =    0x0080;
    final static int MCO_DB_INDF_ATYPE_BASED =      0x0100;
    final static int MCO_DB_INDF_VOLUNTARY_SAVED =  0x0200;

    final static int MCO_DB_INDF_T_LIST =           0x0400;
    final static int MCO_DB_INDF_T_REGULAR =        0x0800;
    final static int MCO_DB_INDF_T_AUTOID =         0x1000;
    final static int MCO_DB_INDF_T_HISTORY =        0x2000;
    final static int MCO_DB_INDF_NULLABLE =        0x10000;
    final static int MCO_DB_INDF_THICK    =        0x20000;
    final static int MCO_DB_INDF_COMPACT  =        0x40000;
    final static int MCO_DB_INDF_POINT    =        0x80000;
    final static int MCO_DB_INDF_TRIGRAM  =       0x100000;

    final static int scalarSeqElemSize[] = {1,2,4,8,1,2,4,8,4,8,0,8};


    int buildFieldsList(StructDescriptor struct, ClassDescriptor clazz, String path) {
        ArrayList<FieldDescriptor> fieldList = new ArrayList<FieldDescriptor>();
        struct.flags |= MCO_DB_TYPINFO_FIXEDREC;
        Class scope = struct.cls;
        int nIndices = 0;
        do {
            for (Field f : scope.getDeclaredFields()) {
                if (!f.isSynthetic() && (f.getModifiers() & (Modifier.TRANSIENT|Modifier.STATIC)) == 0) {
                    try {
                        f.setAccessible(true);
                    } catch (Exception x) {}
                    FieldDescriptor desc = new FieldDescriptor();
                    desc.fld = f;
                    desc.name = f.getName();
                    desc.owner = struct;
                    desc.refCls = -1;
                    Class c = f.getType();
                    Indexable idx = (Indexable)f.getAnnotation(Indexable.class);
                    Dimension dim = (Dimension)f.getAnnotation(Dimension.class);
                    Event evt = (Event)f.getAnnotation(Event.class);

                    if (f.getAnnotation(Nullable.class) != null) {
                        desc.flags |= MCO_DICT_FLDF_NULLABLE;
                    }
                    if (c.isEnum()) {
                        desc.enumClass = c;
                        desc.type = MCO_DB_FT_INT4;
                        desc.fixedSize = desc.size = desc.compactSize = 4;
                        desc.alignment = desc.compactAlignment = 4;
                    } else if (c == int.class) {
                        desc.type = MCO_DB_FT_INT4;
                        desc.fixedSize = desc.size = desc.compactSize = 4;
                        desc.alignment = desc.compactAlignment = 4;
                    } else if (c == boolean.class) {
                        desc.type = MCO_DB_FT_BOOL;
                        desc.fixedSize = desc.size = desc.compactSize = 1;
                        desc.alignment = desc.compactAlignment = 1;
                    } else if (c == short.class) {
                        desc.type = MCO_DB_FT_INT2;
                        desc.fixedSize = desc.size = desc.compactSize = 2;
                        desc.alignment = desc.compactAlignment = 2;
                    } else if (c == char.class) {
                        desc.type = MCO_DB_FT_UINT2;
                        desc.fixedSize = desc.size = desc.compactSize = 2;
                        desc.alignment = desc.compactAlignment = 2;
                    } else if (c == long.class) {
                        References refs = (References)f.getAnnotation(References.class);
                        if (refs != null) {
                            Class refCls = refs.value();
                            for (int i = 0; i < params.classes.length; i++) {
                                if (params.classes[i] == refCls) {
                                    desc.refCls = i+1;
                                    break;
                                }
                            }
                            if (desc.refCls < 0) {
                                throw new IllegalArgumentException("Class " + refCls + " is not included in list of database classes");
                            }
                        }
                        desc.type = MCO_DB_FT_INT8;
                        desc.fixedSize = desc.size = desc.compactSize = 8;
                        desc.alignment = desc.compactAlignment = 8;
                    } else if (c == byte.class) {
                        desc.type = MCO_DB_FT_INT1;
                        desc.fixedSize = desc.size = desc.compactSize = 1;
                        desc.alignment = desc.compactAlignment = 1;
                    } else if (c == float.class) {
                        desc.type = MCO_DB_FT_FLOAT;
                        desc.fixedSize = desc.size = desc.compactSize = 4;
                        desc.alignment = desc.compactAlignment = 4;
                    } else if (c == double.class) {
                        desc.type = MCO_DB_FT_DOUBLE;
                        desc.fixedSize = desc.size = desc.compactSize = 8;
                        desc.alignment = desc.compactAlignment = 8;
                    } else if (c == Date.class) {
                        desc.type = MCO_DB_FT_DATETIME;
                        desc.fixedSize = desc.size = desc.compactSize = 8;
                        desc.alignment = desc.compactAlignment = 8;
                    } else if (c == Integer.class) {
                        desc.type = MCO_DB_FT_INT4;
                        desc.fixedSize = desc.size = desc.compactSize = 4;
                        desc.alignment = desc.compactAlignment = 4;
                        desc.flags |= MCO_DICT_FLDF_NULLABLE;
                    } else if (c == Boolean.class) {
                        desc.type = MCO_DB_FT_BOOL;
                        desc.fixedSize = desc.size = desc.compactSize = 1;
                        desc.alignment = desc.compactAlignment = 1;
                        desc.flags |= MCO_DICT_FLDF_NULLABLE;
                    } else if (c == Short.class) {
                        desc.type = MCO_DB_FT_INT2;
                        desc.fixedSize = desc.size = desc.compactSize = 2;
                        desc.alignment = desc.compactAlignment = 2;
                        desc.flags |= MCO_DICT_FLDF_NULLABLE;
                    } else if (c == Character.class) {
                        desc.type = MCO_DB_FT_UINT2;
                        desc.fixedSize = desc.size = desc.compactSize = 2;
                        desc.alignment = desc.compactAlignment = 2;
                        desc.flags |= MCO_DICT_FLDF_NULLABLE;
                    } else if (c == Long.class) {
                        desc.type = MCO_DB_FT_INT8;
                        desc.fixedSize = desc.size = desc.compactSize = 8;
                        desc.alignment = desc.compactAlignment = 8;
                        desc.flags |= MCO_DICT_FLDF_NULLABLE;
                    } else if (c == Byte.class) {
                        desc.type = MCO_DB_FT_INT1;
                        desc.fixedSize = desc.size = desc.compactSize = 1;
                        desc.alignment = desc.compactAlignment = 1;
                        desc.flags |= MCO_DICT_FLDF_NULLABLE;
                    } else if (c == Float.class) {
                        desc.type = MCO_DB_FT_FLOAT;
                        desc.fixedSize = desc.size = desc.compactSize = 4;
                        desc.alignment = desc.compactAlignment = 4;
                        desc.flags |= MCO_DICT_FLDF_NULLABLE;
                    } else if (c == Double.class) {
                        desc.type = MCO_DB_FT_DOUBLE;
                        desc.fixedSize = desc.size = desc.compactSize = 8;
                        desc.alignment = desc.compactAlignment = 8;
                        desc.flags |= MCO_DICT_FLDF_NULLABLE;
                    } else if (c == BigDecimal.class) {
                        Numeric num = (Numeric)f.getAnnotation(Numeric.class);
                        desc.width = 19;
                        if (num != null) {
                            desc.width = num.width();
                            desc.precision = num.precision();
                        }
                        if (desc.width <= 2) {
                            desc.type = MCO_DB_FT_INT1;
                            desc.fixedSize = desc.size = desc.compactSize = 1;
                            desc.alignment = desc.compactAlignment = 1;
                        } else if (desc.width <= 4) {
                            desc.type = MCO_DB_FT_INT2;
                            desc.fixedSize = desc.size = desc.compactSize = 2;
                            desc.alignment = desc.compactAlignment = 2;
                        } else if (desc.width <= 9) {
                            desc.type = MCO_DB_FT_INT4;
                            desc.fixedSize = desc.size = desc.compactSize = 4;
                            desc.alignment = desc.compactAlignment = 4;
                        } else if (desc.width <= 19) {
                            desc.type = MCO_DB_FT_INT8;
                            desc.fixedSize = desc.size = desc.compactSize = 8;
                            desc.alignment = desc.compactAlignment = 8;
                        } else {
                            throw new IllegalArgumentException("Width should be less than 20");
                        }
                        desc.flags |= MCO_DICT_FLDF_NUMERIC;
                    } else if (c == String.class) {
                        Encoding e = (Encoding)f.getAnnotation(Encoding.class);
                        desc.encoding = defaultEncoding;
                        if (e != null) {
                            desc.encoding = e.value();
                        }
                        if (dim != null) {
                            desc.arraySize = 0;
                            if (desc.encoding.equals(WIDE_CHARACTER_ENCODING)) {
                                desc.type = MCO_DB_FT_UNICODE_CHARS;
                                desc.fixedSize = desc.compactSize = desc.size = dim.value()*2;
                                desc.alignment = desc.compactAlignment = 2;
                            } else {
                                desc.type = MCO_DB_FT_CHARS;
                                desc.fixedSize = desc.compactSize = desc.size = dim.value();
                                desc.alignment = desc.compactAlignment = 1;
                            }
                        } else {
                            desc.fixedSize = 0;
                            desc.type = desc.encoding.equals(WIDE_CHARACTER_ENCODING) ? MCO_DB_FT_UNICODE_STRING : MCO_DB_FT_STRING;
                            desc.size = 4;
                            desc.compactSize = 2;
                            desc.alignment = 4;
                            desc.compactAlignment = 2;
                            struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                        }
                    } else if (c == UnorderedSequence.class || c == OrderedSequence.class) {
                        Sequence seq = (Sequence)f.getAnnotation(Sequence.class);
                        if (seq == null) {
                            throw new IllegalArgumentException("Missed sequence annotation");
                        }
                        desc.type = seq.type() == Sequence.Type.Char ? MCO_DB_FT_SEQUENCE_CHAR : MCO_DB_FT_SEQUENCE_UINT1 + seq.type().ordinal();
                        struct.flags |= MCO_DB_TYPINFO_HAS_SEQUENCES;
                        int ptrSize = getPointerSize();
                        desc.seqOrder = ((c == UnorderedSequence.class) ? Sequence.Order.None
										 : (seq.order() == Sequence.Order.None) ? Sequence.Order.Ascending : seq.order()).ordinal();
                        desc.seqElemSize = seq.type() == Sequence.Type.Char ? seq.elemSize() : scalarSeqElemSize[seq.type().ordinal()];
                        if (desc.seqElemSize == 0) {
                            throw new IllegalArgumentException("Sequence of char element size is not specified");
                        }
                        desc.fixedSize = 0;
                        desc.size = desc.compactSize = ptrSize;
                        desc.alignment = desc.compactAlignment = ptrSize;
                    } else if (c == byte[].class) {
                        Blob blob = (Blob)f.getAnnotation(Blob.class);
                        if (blob != null) {
                            desc.type = MCO_DB_FT_BLOB;
                            struct.flags |= MCO_DB_TYPINFO_HAS_BLOBS;
                            struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                            desc.fixedSize = 0;
                            int ptrSize = getPointerSize();
                            desc.size = desc.compactSize = ptrSize + 4;
                            desc.alignment = desc.compactAlignment = ptrSize;
                        } else {
                            Sequence seq = (Sequence)f.getAnnotation(Sequence.class);
                            if (seq != null) {
                                if (seq.type() != Sequence.Type.Int1 && seq.type() != Sequence.Type.UInt1) {
                                    throw new IllegalArgumentException("Sequence type is not compatible with array element type");
                                }
                                desc.type = MCO_DB_FT_SEQUENCE_UINT1 + seq.type().ordinal();
                                struct.flags |= MCO_DB_TYPINFO_HAS_SEQUENCES;
                                int ptrSize = getPointerSize();
                                desc.seqOrder = seq.order().ordinal();
                                desc.seqElemSize = 1;
                                desc.fixedSize = 0;
                                desc.size = desc.compactSize = ptrSize;
                                desc.alignment = desc.compactAlignment = ptrSize;
                            } else {
                                desc.type = MCO_DB_FT_INT1;
                                desc.fixedSize = 1;
                                if (dim != null) {
                                    desc.flags |= MCO_DICT_FLDF_ARRAY;
                                    desc.arraySize = dim.value();
                                    desc.size = desc.compactSize = desc.arraySize;
                                    desc.alignment = desc.compactAlignment = 1;
                                } else {
                                    desc.flags |= MCO_DICT_FLDF_VECTOR;
                                    desc.size = 6;
                                    desc.compactSize = 4;
                                    desc.alignment = 4;
                                    desc.compactAlignment = 2;
                                    struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                                }
                            }
                        }
                    } else if (c == boolean[].class) {
                        desc.type = MCO_DB_FT_BOOL;
                        desc.fixedSize = 1;
                        if (dim != null) {
                            desc.flags |= MCO_DICT_FLDF_ARRAY;
                            desc.arraySize = dim.value();
                            desc.size = desc.compactSize = (desc.arraySize + 7)/8;
                            desc.alignment = desc.compactAlignment = 1;
                        } else {
                            desc.flags |= MCO_DICT_FLDF_VECTOR;
                            desc.size = 6;
                            desc.compactSize = 4;
                            desc.alignment = 4;
                            desc.compactAlignment = 2;
                            struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                        }
                    } else if (c == int[].class) {
                        Sequence seq = (Sequence)f.getAnnotation(Sequence.class);
                        if (seq != null) {
                            if (seq.type() != Sequence.Type.Int4 && seq.type() != Sequence.Type.UInt4) {
                                throw new IllegalArgumentException("Sequence type is not compatible with array element type");
                            }
                            desc.type = MCO_DB_FT_SEQUENCE_UINT1 + seq.type().ordinal();
                            struct.flags |= MCO_DB_TYPINFO_HAS_SEQUENCES;
                            int ptrSize = getPointerSize();
                            desc.seqOrder = seq.order().ordinal();
                            desc.seqElemSize = 4;
                            desc.fixedSize = 0;
                            desc.size = desc.compactSize = ptrSize;
                            desc.alignment = desc.compactAlignment = ptrSize;
                        } else {
                            desc.type = MCO_DB_FT_INT4;
                            desc.fixedSize = 4;
                            if (dim != null) {
                                desc.flags |= MCO_DICT_FLDF_ARRAY;
                                desc.arraySize = dim.value();
                                desc.size = desc.compactSize = desc.arraySize*4;
                                desc.alignment = desc.compactAlignment = 4;
                            } else {
                                desc.flags |= MCO_DICT_FLDF_VECTOR;
                                desc.size = 6;
                                desc.compactSize = 4;
                                desc.alignment = 4;
                                desc.compactAlignment = 2;
                                struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                            }
                        }
                    } else if (c == short[].class) {
                        Sequence seq = (Sequence)f.getAnnotation(Sequence.class);
                        if (seq != null) {
                            if (seq.type() != Sequence.Type.Int2 && seq.type() != Sequence.Type.UInt2) {
                                throw new IllegalArgumentException("Sequence type is not compatible with array element type");
                            }
                            desc.type = MCO_DB_FT_SEQUENCE_UINT1 + seq.type().ordinal();
                            struct.flags |= MCO_DB_TYPINFO_HAS_SEQUENCES;
                            int ptrSize = getPointerSize();
                            desc.seqOrder = seq.order().ordinal();
                            desc.seqElemSize = 2;
                            desc.fixedSize = 0;
                            desc.size = desc.compactSize = ptrSize;
                            desc.alignment = desc.compactAlignment = ptrSize;
                        } else {
                            desc.type = MCO_DB_FT_INT2;
                            desc.fixedSize = 2;
                            if (dim != null) {
                                desc.flags |= MCO_DICT_FLDF_ARRAY;
                                desc.arraySize = dim.value();
                                desc.size = desc.compactSize = desc.arraySize*2;
                                desc.alignment = desc.compactAlignment = 2;
                            } else {
                                desc.flags |= MCO_DICT_FLDF_VECTOR;
                                desc.size = 6;
                                desc.compactSize = 4;
                                desc.alignment = 4;
                                desc.compactAlignment = 2;
                                struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                            }
                        }
                    } else if (c == char[].class) {
                        Sequence seq = (Sequence)f.getAnnotation(Sequence.class);
                        if (seq != null) {
                            if (seq.type() != Sequence.Type.Int2 && seq.type() != Sequence.Type.UInt2) {
                                throw new IllegalArgumentException("Sequence type is not compatible with array element type");
                            }
                            desc.type = MCO_DB_FT_SEQUENCE_UINT1 + seq.type().ordinal();
                            struct.flags |= MCO_DB_TYPINFO_HAS_SEQUENCES;
                            int ptrSize = getPointerSize();
                            desc.seqOrder = seq.order().ordinal();
                            desc.seqElemSize = 2;
                            desc.fixedSize = 0;
                            desc.size = desc.compactSize = ptrSize;
                            desc.alignment = desc.compactAlignment = ptrSize;
                        } else {
                            desc.type = MCO_DB_FT_UINT2;
                            desc.fixedSize = 2;
                            if (dim != null) {
                                desc.flags |= MCO_DICT_FLDF_ARRAY;
                                desc.arraySize = dim.value();
                                desc.size = desc.compactSize = desc.arraySize*2;
                                desc.alignment = desc.compactAlignment = 2;
                            } else {
                                desc.flags |= MCO_DICT_FLDF_VECTOR;
                                desc.size = 6;
                                desc.compactSize = 4;
                                desc.alignment = 4;
                                desc.compactAlignment = 2;
                                struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                            }
                        }
                    } else if (c == long[].class) {
                        Sequence seq = (Sequence)f.getAnnotation(Sequence.class);
                        if (seq != null) {
                            if (seq.type() != Sequence.Type.Int8 && seq.type() != Sequence.Type.UInt8) {
                                throw new IllegalArgumentException("Sequence type is not compatible with array element type");
                            }
                            desc.type = MCO_DB_FT_SEQUENCE_UINT1 + seq.type().ordinal();
                            struct.flags |= MCO_DB_TYPINFO_HAS_SEQUENCES;
                            int ptrSize = getPointerSize();
                            desc.seqOrder = seq.order().ordinal();
                            desc.seqElemSize = 8;
                            desc.fixedSize = 0;
                            desc.size = desc.compactSize = ptrSize;
                            desc.alignment = desc.compactAlignment = ptrSize;
                        } else {
                            desc.type = MCO_DB_FT_INT8;
                            desc.fixedSize = 8;
                            if (dim != null) {
                                desc.flags |= MCO_DICT_FLDF_ARRAY;
                                desc.arraySize = dim.value();
                                desc.size = desc.compactSize = desc.arraySize*8;
                                desc.alignment = desc.compactAlignment = 8;
                            } else {
                                desc.flags |= MCO_DICT_FLDF_VECTOR;
                                desc.size = 6;
                                desc.compactSize = 4;
                                desc.alignment = 4;
                                desc.compactAlignment = 2;
                                struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                            }
                        }
                    } else if (c == Date[].class) {
                    	Sequence seq = (Sequence)f.getAnnotation(Sequence.class);
                    	if (seq != null) {
                            if (seq.type() != Sequence.Type.DateTime) {
                                throw new IllegalArgumentException("Sequence type is not compatible with array element type");
                            }

                    		desc.type = MCO_DB_FT_SEQUENCE_DATETIME;
                            struct.flags |= MCO_DB_TYPINFO_HAS_SEQUENCES;
                            int ptrSize = getPointerSize();
                            desc.seqOrder = seq.order().ordinal();
                            desc.seqElemSize = 8;
                            desc.fixedSize = 0;
                            desc.size = desc.compactSize = ptrSize;
                            desc.alignment = desc.compactAlignment = ptrSize;
                    	} else {
                    		desc.type = MCO_DB_FT_DATETIME;
                    		desc.fixedSize = 8;
                    		if (dim != null) {
                    			desc.flags |= MCO_DICT_FLDF_ARRAY;
                    			desc.arraySize = dim.value();
                    			desc.size = desc.compactSize = desc.arraySize*8;
                    			desc.alignment = desc.compactAlignment = 8;
                    		} else {
                    			desc.flags |= MCO_DICT_FLDF_VECTOR;
                    			desc.size = 6;
                    			desc.compactSize = 4;
                    			desc.alignment = 4;
                    			desc.compactAlignment = 2;
                    			struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                    		}
                    	}
                    } else if (c == BigDecimal[].class) {
                        Numeric num = (Numeric)f.getAnnotation(Numeric.class);
                        desc.width = 19;
                        if (num != null) {
                            desc.width = num.width();
                            desc.precision = num.precision();
                        }
                        if (desc.width <= 2) {
                            desc.type = MCO_DB_FT_INT1;
                            desc.fixedSize = desc.size = desc.compactSize = 1;
                            desc.alignment = desc.compactAlignment = 1;
                        } else if (desc.width <= 4) {
                            desc.type = MCO_DB_FT_INT2;
                            desc.fixedSize = desc.size = desc.compactSize = 2;
                            desc.alignment = desc.compactAlignment = 2;
                        } else if (desc.width <= 9) {
                            desc.type = MCO_DB_FT_INT4;
                            desc.fixedSize = desc.size = desc.compactSize = 4;
                            desc.alignment = desc.compactAlignment = 4;
                        } else if (desc.width <= 19) {
                            desc.type = MCO_DB_FT_INT8;
                            desc.fixedSize = desc.size = desc.compactSize = 8;
                            desc.alignment = desc.compactAlignment = 8;
                        } else {
                            throw new IllegalArgumentException("Width should be less than 20");
                        }
                        if (dim != null) {
                            desc.flags |= MCO_DICT_FLDF_ARRAY;
                            desc.arraySize = dim.value();
                            desc.size = desc.compactSize = desc.arraySize*desc.size;
                        } else {
                            desc.flags |= MCO_DICT_FLDF_VECTOR;
                            desc.size = 6;
                            desc.compactSize = 4;
                            desc.alignment = 4;
                            desc.compactAlignment = 2;
                            struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                        }
                        desc.flags |= MCO_DICT_FLDF_NUMERIC;
                    } else if (c == float[].class) {
                        Sequence seq = (Sequence)f.getAnnotation(Sequence.class);
                        if (seq != null) {
                            if (seq.type() != Sequence.Type.Float) {
                                throw new IllegalArgumentException("Sequence type is not compatible with array element type");
                            }
                            desc.type = MCO_DB_FT_SEQUENCE_FLOAT;
                            struct.flags |= MCO_DB_TYPINFO_HAS_SEQUENCES;
                            int ptrSize = getPointerSize();
                            desc.seqOrder = seq.order().ordinal();
                            desc.seqElemSize = 4;
                            desc.fixedSize = 0;
                            desc.size = desc.compactSize = ptrSize;
                            desc.alignment = desc.compactAlignment = ptrSize;
                        } else {
                            desc.type = MCO_DB_FT_FLOAT;
                            desc.fixedSize = 4;
                            if (dim != null) {
                                desc.flags |= MCO_DICT_FLDF_ARRAY;
                                desc.arraySize = dim.value();
                                desc.size = desc.compactSize = desc.arraySize*4;
                                desc.alignment = desc.compactAlignment = 4;
                            } else {
                                desc.flags |= MCO_DICT_FLDF_VECTOR;
                                desc.size = 6;
                                desc.compactSize = 4;
                                desc.alignment = 4;
                                desc.compactAlignment = 2;
                                struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                            }
                        }
                    } else if (c == double[].class) {
                        Sequence seq = (Sequence)f.getAnnotation(Sequence.class);
                        if (seq != null) {
                            if (seq.type() != Sequence.Type.Double) {
                                throw new IllegalArgumentException("Sequence type is not compatible with array element type");
                            }
                            desc.type = MCO_DB_FT_SEQUENCE_DOUBLE;
                            struct.flags |= MCO_DB_TYPINFO_HAS_SEQUENCES;
                            int ptrSize = getPointerSize();
                            desc.seqOrder = seq.order().ordinal();
                            desc.seqElemSize = 8;
                            desc.fixedSize = 0;
                            desc.size = desc.compactSize = ptrSize;
                            desc.alignment = desc.compactAlignment = ptrSize;
                        } else {
                            desc.type = MCO_DB_FT_DOUBLE;
                            desc.fixedSize = 8;
                            if (dim != null) {
                                desc.flags |= MCO_DICT_FLDF_ARRAY;
                                desc.arraySize = dim.value();
                                desc.size = desc.compactSize = desc.arraySize*8;
                                desc.alignment = desc.compactAlignment = 8;
                            } else {
                                desc.flags |= MCO_DICT_FLDF_VECTOR;
                                desc.size = 6;
                                desc.compactSize = 4;
                                desc.alignment = 4;
                                desc.compactAlignment = 2;
                                struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                            }
                        }
                    } else if (c == String[].class) {
                        Encoding e = (Encoding)f.getAnnotation(Encoding.class);
                        desc.encoding = defaultEncoding;
                        if (e != null) {
                            desc.encoding = e.value();
                        }
                        desc.type = desc.encoding.equals(WIDE_CHARACTER_ENCODING) ? MCO_DB_FT_UNICODE_STRING : MCO_DB_FT_STRING;
                        desc.fixedSize = 0;
                        struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                        if (dim != null) {
                            desc.flags |= MCO_DICT_FLDF_ARRAY;
                            desc.arraySize = dim.value();
                            desc.size = desc.arraySize*4;
                            desc.compactSize = desc.arraySize*2;
                            desc.alignment = 4;
                            desc.compactAlignment = 2;
                        } else {
                            desc.flags |= MCO_DICT_FLDF_VECTOR;
                            desc.fixedSize = 0;
                            desc.size = 6;
                            desc.compactSize = 4;
                            desc.alignment = 4;
                            desc.compactAlignment = 2;
                        }
                    } else if (c.isArray()) {
                        // An array of optional structs.
                        desc.type = MCO_DB_FT_STRUCT;
                        c = c.getComponentType();
                        if (c.isEnum()) {
						    throw new IllegalArgumentException("Arrays of enum type are not supported");
                        }
                        StructDescriptor s = structsMap.get(c);
                        StructDescriptor news = new StructDescriptor(c);
                        if (s == null) {
                            structsMap.put(c, news);
                        }
                        int saveTotalFields = totalFields;
                        buildFieldsList(news, clazz, null);
                        if (s != null) {
                            totalFields = saveTotalFields;
                        } else {
                            s = news;
                            structsList.add(s);
                        }
                        if (dim != null) {
                            desc.flags |= MCO_DICT_FLDF_ARRAY;
                            desc.arraySize = dim.value();
                            desc.size = desc.arraySize*((s.size + s.alignment - 1) & ~(s.alignment - 1));
                            desc.compactSize = desc.arraySize*((s.compactSize + s.compactAlignment-1) & ~(s.compactAlignment-1));
                            desc.alignment = s.alignment;
                            desc.compactAlignment = s.compactAlignment;
                            struct.flags &= s.flags | ~MCO_DB_TYPINFO_FIXEDREC;
                        } else {
                            desc.fixedSize = 0;
                            desc.flags |= MCO_DICT_FLDF_VECTOR;
                            desc.size = 6;
                            desc.compactSize = 4;
                            desc.alignment = 4;
                            desc.compactAlignment = 2;
                            struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                        }
                        struct.flags |= s.flags & (MCO_DB_TYPINFO_HAS_BLOBS|MCO_DB_TYPINFO_HAS_SEQUENCES);
                        desc.components = news;
                    } else if (List.class.isAssignableFrom(c) && f.getGenericType() instanceof ParameterizedType) {
                        // An array of optional structs.
                        desc.type = MCO_DB_FT_STRUCT;
                        c = (Class)((ParameterizedType)f.getGenericType()).getActualTypeArguments()[0];
                        if (c.isEnum()) {
						    throw new IllegalArgumentException("Lists of enum type are not supported");
                        }
                        StructDescriptor s = structsMap.get(c);
                        StructDescriptor news = new StructDescriptor(c);
                        if (s == null) {
                            structsMap.put(c, news);
                        }
                        int saveTotalFields = totalFields;
                        buildFieldsList(news, clazz, null);
                        if (s != null) {
                            totalFields = saveTotalFields;
                        } else {
                            s = news;
                            structsList.add(s);
                        }
                        desc.fixedSize = 0;
                        desc.flags |= MCO_DICT_FLDF_VECTOR;
                        desc.size = 6;
                        desc.compactSize = 4;
                        desc.alignment = 4;
                        desc.compactAlignment = 2;
                        struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                        struct.flags |= s.flags & (MCO_DB_TYPINFO_HAS_BLOBS|MCO_DB_TYPINFO_HAS_SEQUENCES);
                        desc.components = news;
                        desc.isList = true;
                    } else {
                        OptionalStruct optional = (OptionalStruct)f.getAnnotation(OptionalStruct.class);
                        // optional structure
                        desc.type = MCO_DB_FT_STRUCT;
                        if (optional != null) {
                            desc.flags |= MCO_DICT_FLDF_OPTIONAL;
                            struct.flags &= ~MCO_DB_TYPINFO_FIXEDREC;
                        }
                        StructDescriptor s = structsMap.get(c);
                        StructDescriptor news = new StructDescriptor(c);
                        if (s == null) {
                            structsMap.put(c, news);
                        }
                        int saveTotalFields = totalFields;
                        int nStructIndices = buildFieldsList(news, clazz, path != null && optional == null ? path + f.getName() + '.' : null);
                        if (s != null) {
                            totalFields = saveTotalFields;
                        } else {
                            s = news;
                            structsList.add(s);
                        }
                        if (nStructIndices != 0) {
                            nIndices += nStructIndices;
                            desc.flags |= MCO_DICT_FLDF_INDEXED;
                        }
                        struct.flags &= s.flags | ~MCO_DB_TYPINFO_FIXEDREC;
                        struct.flags |= s.flags & (MCO_DB_TYPINFO_HAS_BLOBS|MCO_DB_TYPINFO_HAS_SEQUENCES);
                        desc.components = news;
                        desc.fixedSize = 0;
                        if (optional != null) {
                            desc.size = 4;
                            desc.compactSize = 2;
                            desc.alignment = 4;
                            desc.compactAlignment = 2;
                        } else {
                            desc.size = s.size;
                            desc.compactSize = s.compactSize;
                            desc.alignment = s.alignment;
                            desc.compactAlignment = s.compactAlignment;
                        }
                    }
                    if (evt != null) {
                        String name = evt.name();
                        EventDescriptor e = events.get(name);
                        if (e != null) {
						    throw new IllegalArgumentException("Event " + name + " is already defined in class " + e.scope.name);
                        }
                        EventType type = evt.type();
                        if (type != EventType.OnFieldUpdate) {
					  	    throw new IllegalArgumentException("Event " + name + " should have OnFieldUpdate type");
                        }
                        if (path == null) {
					  	    throw new IllegalArgumentException("Update of array component " + name + " can not be triggered");
                        }
                        clazz.flags |= MCO_DB_TYPINFO_HAS_EVENTS;
                        e = new EventDescriptor(name, type, clazz, events.size());
                        events.put(name, e);
                        e.field = desc;
                        e.fieldName = path + f.getName();
                        desc.onUpdateEvent = e;
                        EventDescriptor[] newEvents = new EventDescriptor[clazz.events.length+1];
                        System.arraycopy(clazz.events, 0, newEvents, 0, clazz.events.length);
                        newEvents[newEvents.length-1] = e;
                        clazz.events = newEvents;
                    }
                    if (idx != null) {
                        desc.flags |= MCO_DICT_FLDF_INDEXED;
                        nIndices += 1;
                        if (path != null) {
                            IndexDescriptor idxDesc = new IndexDescriptor();
                            idxDesc.name = path + f.getName();
                            if (desc.type == MCO_DB_FT_STRUCT)
                            {
                                throw new IllegalArgumentException("Struct " + desc.name + " can not be indexed");
                            }
                            idxDesc.keys = new KeyDescriptor[]{new KeyDescriptor(idxDesc.name, desc, idx.descending())};
                            totalKeys += 1;
                            idxDesc.type = idx.type();
                            if (idxDesc.type == IndexType.Hashtable) {
                                idxDesc.size = idx.initSize();
                            }
                            idxDesc.flags = MCO_DB_INDF_T_REGULAR | (clazz.flags & MCO_DB_TYPINFO_PERSISTENT);
                            if (idxDesc.type == IndexType.RTree) {
                                if ((desc.flags & MCO_DICT_FLDF_ARRAY) == 0) {
                                    throw new IllegalArgumentException("R-Tree index " + desc.name + " can be defined only for array");
                                }
                                if ((desc.arraySize & 1) != 0) {
                                    throw new IllegalArgumentException("R-Tree index " + desc.name + " can be defined only for array with even length");
                                }
                            } else if (idxDesc.type == IndexType.RTreeOfPoint) {
                                if ((desc.flags & MCO_DICT_FLDF_ARRAY) == 0) {
                                    throw new IllegalArgumentException("R-Tree index " + desc.name + " can be defined only for array");
                                }
                                idxDesc.flags |= MCO_DB_INDF_POINT;
                            } else if (idxDesc.type == IndexType.Trigram) {
                                if (c != String.class) {
                                    throw new IllegalArgumentException("Trigram index " + desc.name + " can be defined only for strings");
                                }
                                idxDesc.flags |= MCO_DB_INDF_THICK | MCO_DB_INDF_TRIGRAM;
                            }
                            if ((desc.flags & MCO_DICT_FLDF_ARRAY) != 0) {
                                idxDesc.flags |= MCO_DB_INDF_ATYPE_BASED;
                            }
                            if ((desc.flags & MCO_DICT_FLDF_VECTOR) != 0) {
                                idxDesc.flags |= MCO_DB_INDF_VTYPE_BASED;
                            }
                            if (idx.unique()) {
                                idxDesc.unique = true;
                                idxDesc.flags |= MCO_DB_INDF_UNIQUE;
                            } else if (idx.thick()) {
                                idxDesc.thick = true;
                                idxDesc.flags |= MCO_DB_INDF_THICK;
                            }
                            clazz.indicesMap.put(idxDesc.name, idxDesc);
                        }
                    }
                    fieldList.add(desc);
                }
            }
        } while ((scope = scope.getSuperclass()) != null);
        FieldDescriptor[] fields = (FieldDescriptor[])fieldList.toArray(new FieldDescriptor[fieldList.size()]);
        if (!params.dictionaryNoSort) {
            Arrays.sort(fields);
        }
        int offset = 0;
        int fieldNo = 0;
        struct.alignment = 1;
        struct.compactAlignment = 1;
        if (struct == clazz) {
            if ((config & (MCO_CFG_HA_SUPPORT|MCO_CFG_CLUSTER_SUPPORT|MCO_CFG_MPI_CLUSTER_SUPPORT|MCO_CFG_TL_SUPPORT)) != 0) {
                offset += 8;
                fieldNo += 1;
                struct.alignment = 8;
                struct.compactAlignment = 8;
            }
            if ((clazz.flags & MCO_DB_TYPINFO_HAS_AUTOID) != 0) {
                offset += 8;
                fieldNo += 1;
                struct.alignment = 8;
                struct.compactAlignment = 8;
            }
            if ((clazz.flags & MCO_DB_TYPINFO_DISTRIBUTED) != 0) {
                offset += 8;
                fieldNo += 1;
                struct.alignment = 8;
                struct.compactAlignment = 8;
            }
            if (clazz.ttlCount > 0) {
                offset += 8;
                fieldNo += 1;
                struct.alignment = 8;
                struct.compactAlignment = 8;
            }
            if (clazz.ttlClockTime > 0) {
                offset += 8;
                fieldNo += 1;
                struct.alignment = 8;
                struct.compactAlignment = 8;
            }
        }
        int compactOffset = offset;
        for (int i = 0; i < fields.length; i++) {
            FieldDescriptor desc = fields[i];
            desc.fieldNo = fieldNo++;
            offset = (offset + desc.alignment - 1) & ~(desc.alignment - 1);
            compactOffset = (compactOffset + desc.compactAlignment - 1) & ~(desc.compactAlignment - 1);
            desc.offset = offset;
            desc.compactOffset = compactOffset;
            offset += desc.size;
            compactOffset += desc.compactSize;
        }
        for (int i = 0; i < fields.length; i++) {
            FieldDescriptor desc = fields[i];
            if ((desc.flags & MCO_DICT_FLDF_NULLABLE) != 0) {
                desc.indicatorNo = fieldNo++;
                desc.indicatorOffset = offset++;
                desc.indicatorCompactOffset = compactOffset++;
            }
        }
        struct.fields = fields;
        totalFields += fieldNo;
        if (fields.length != 0) {
            if (fields[0].alignment > struct.alignment) {
                struct.alignment = fields[0].alignment;
            }
            if (fields[0].compactAlignment > struct.compactAlignment) {
                struct.compactAlignment = fields[0].compactAlignment;
            }
        } else {
            struct.alignment = 1;
            struct.compactAlignment = 1;
        }
        struct.size = offset;
        struct.compactSize = compactOffset;
        return nIndices;
    }

    void resolveStructComponents(StructDescriptor desc) {
        for (int i = 0; i < desc.fields.length; i++) {
            if (desc.fields[i].components != null) {
                desc.fields[i].components = structsMap.get(desc.fields[i].components.cls);
            }
        }
    }

    FieldDescriptor locateField(StructDescriptor structure, String keyName) {
        int dot = keyName.indexOf('.');
        String fieldName = keyName;
        String componentName = null;
        if (dot > 0) {
            fieldName = keyName.substring(0, dot);
            componentName = keyName.substring(dot+1);
        }
        for (FieldDescriptor field : structure.fields) {
            if (field.name.equals(fieldName)) {
                if (componentName != null) {
                    if (field.components == null) {
                        throw new IllegalArgumentException("Field " + fieldName + " type has no component " + componentName);
                    }
                    return locateField(field.components, componentName);
                }
                return field;
            }
        }
        throw new IllegalArgumentException("Field " + fieldName + " not found in class " + structure.name);
    }

    void addIndex(ClassDescriptor desc, Index idx) {
        IndexDescriptor idxDesc = new IndexDescriptor();
        idxDesc.name = idx.name();
        Key[] keys = idx.keys();
        idxDesc.keys = new KeyDescriptor[keys.length];
        idxDesc.type = idx.type();
        if (idxDesc.type == IndexType.Hashtable) {
            idxDesc.size = idx.initSize();
        }
        totalKeys += keys.length;
        idxDesc.flags = MCO_DB_INDF_T_REGULAR | (desc.flags & MCO_DB_TYPINFO_PERSISTENT);
        for (int k = 0; k < keys.length; k++) {
            String keyName = keys[k].value();
            Database.FieldDescriptor keyDesc = locateField(desc, keyName);
            if (keyDesc.type == MCO_DB_FT_STRUCT) {
                throw new IllegalArgumentException("Struct " + keyName + " can not be indexed");
            }
            idxDesc.keys[k] = new KeyDescriptor(keyName, keyDesc, keys[k].descending());
            keyDesc.flags |= MCO_DICT_FLDF_INDEXED;
            if (idxDesc.type == IndexType.RTree) {
                if (k != 0) {
                    throw new IllegalArgumentException("R-Tree index " + idxDesc.name + " can be defined only for single field array field");
                }
                if ((keyDesc.flags & MCO_DICT_FLDF_ARRAY) == 0) {
                    throw new IllegalArgumentException("R-Tree index " + idxDesc.name + " can be defined only for array");
                }
                if ((keyDesc.arraySize & 1) != 0) {
                    throw new IllegalArgumentException("R-Tree index " + idxDesc.name + " can be defined only for array with even length");
                }
            } else if (idxDesc.type == IndexType.RTreeOfPoint) {
                if (k != 0) {
                    throw new IllegalArgumentException("R-Tree index " + idxDesc.name + " can be defined only for single field array field");
                }
                if ((keyDesc.flags & MCO_DICT_FLDF_ARRAY) == 0) {
                    throw new IllegalArgumentException("R-Tree index " + idxDesc.name + " can be defined only for array");
                }
                idxDesc.flags |= MCO_DB_INDF_POINT;
            } else if (idxDesc.type == IndexType.Trigram) {
                if (k != 0 || keyDesc.fld.getType() != String.class) {
                    throw new IllegalArgumentException("Trigram index " + idxDesc.name + " can be defined only for strings");
                }
                idxDesc.flags |= MCO_DB_INDF_THICK | MCO_DB_INDF_TRIGRAM;
            }
        }
        if (idx.unique()) {
            idxDesc.unique = true;
            idxDesc.flags |= MCO_DB_INDF_UNIQUE;
        } else if (idx.thick()) {
            idxDesc.thick = true;
            idxDesc.flags |= MCO_DB_INDF_THICK;
        }
        desc.indicesMap.put(idxDesc.name, idxDesc);
    }

    private void buildDictionary(Parameters params) {
        structsMap = new HashMap<Class, StructDescriptor>();
        structsList = new ArrayList<StructDescriptor>();
        classes = new ClassDescriptor[params.classes.length];
        events = new HashMap<String, EventDescriptor>();
        this.params = params;
        totalFields = 0;
        totalKeys = 0;
        hasInmemDistribClasses = hasDiskDistribClasses = false;
        int indexNo = 0;
        for (int i = 0; i < params.classes.length; i++) {
            Class c = params.classes[i];
            ClassDescriptor desc = new ClassDescriptor(c);
            desc.classNo = i+1;
            structsMap.put(c, desc);
            desc.indicesMap = new HashMap<String, IndexDescriptor>();
            this.classes[i] = desc;
            if (params.diskClassesByDefault) {
                desc.flags |= MCO_DB_TYPINFO_PERSISTENT;
            }
            if (params.compactClassesByDefault) {
                desc.flags |= MCO_DB_TYPINFO_COMPACT;
            }
            if (params.nonatomicClassesByDefault) {
                desc.flags |= MCO_DB_TYPINFO_NONATOMIC;
            }
            desc.listIndexNo = -1;
            desc.autoIdIndexNo = -1;
            int nClassIndices = 0;

            Event[] eventList = null;
            Trigger trigger = (Trigger)c.getAnnotation(Trigger.class);
            if (trigger != null) {
                eventList = trigger.value();
            } else {
                Event e = (Event)c.getAnnotation(Event.class);
                if (e != null) {
                    eventList = new Event[1];
                    eventList[0] = e;
                }
            }
            if (eventList != null) {
                desc.events = new EventDescriptor[eventList.length];
                for (int j = 0; j < eventList.length; j++) {
                    String name = eventList[j].name();
                    EventDescriptor e = events.get(name);
                    if (e != null) {
						throw new IllegalArgumentException("Event " + name + " is already defined in class " + e.scope.name);
                    }
                    EventType type = eventList[j].type();
                    if (type == EventType.OnFieldUpdate) {
						throw new IllegalArgumentException("Event " + name + " has to be be defined for the particular field");
                    }
                    desc.flags |= MCO_DB_TYPINFO_HAS_EVENTS;
                    events.put(name, desc.events[j] = new EventDescriptor(name, type, desc, events.size()));
                }
            } else {
                desc.events = new EventDescriptor[0];
            }
            Persistent p = (Persistent)c.getAnnotation(Persistent.class);
            if (p != null) {
                if (p.disk()) {
                    desc.flags |= MCO_DB_TYPINFO_PERSISTENT;
                } else if (p.inmemory()) {
                    desc.flags &= ~MCO_DB_TYPINFO_PERSISTENT;
                }
                if (p.nonatomic()) {
                    desc.flags |= MCO_DB_TYPINFO_NONATOMIC;
                }
                if (p.atomic()) {
                    desc.flags &= ~MCO_DB_TYPINFO_NONATOMIC;
                }
                if (p.large()) {
                    desc.flags &= ~MCO_DB_TYPINFO_COMPACT;
                } else if (p.compact()) {
                    desc.flags |= MCO_DB_TYPINFO_COMPACT;
                }
                if (p.list()) {
                    desc.flags |= MCO_DB_TYPINFO_HAS_LIST;
                    desc.listIndexNo = indexNo++;
                    nClassIndices += 1;
                }
                if (p.autoid()) {
                    desc.flags |= MCO_DB_TYPINFO_HAS_AUTOID;
                    desc.autoIdIndexNo = indexNo++;
                    nClassIndices += 1;
                    totalKeys += 1;
                }
                if (p.local()) {
                    if (p.distributed()) throw new IllegalArgumentException("'local' contradicts with 'distributed' in class " + c.getName());
                    desc.flags |= MCO_DB_TYPINFO_LOCAL;
                }
                if (p.distributed()) {
                    if (p.local()) throw new IllegalArgumentException("'local' contradicts with 'distributed' in class " + c.getName());
                    desc.flags |= MCO_DB_TYPINFO_DISTRIBUTED;
                    nClassIndices += 1;
                    totalKeys += 1;
                    indexNo++;
                    if ((desc.flags & MCO_DB_TYPINFO_PERSISTENT) == 0) {
                        hasInmemDistribClasses = true;
                    } else {
                        hasDiskDistribClasses = true;
                    }
                }
            }
            TTL ttl = (TTL)c.getAnnotation(TTL.class);
            if (ttl != null) {
                desc.ttlCount = ttl.maxCount();
                desc.ttlClockTime = ttl.maxTime();
                if (desc.ttlCount > 0) {
                    nClassIndices += 1;
                    totalKeys += 1;
                    indexNo++;
                }
                if (desc.ttlClockTime > 0) {
                    nClassIndices += 1;
                    totalKeys += 1;
                    indexNo++;
                }
            }

            buildFieldsList(desc, desc, "");
            if ((desc.flags & MCO_DB_TYPINFO_COMPACT) != 0) {
                desc.calculateCompactClassOffsets(0);
            } else {
                desc.calculateClassOffsets(0);
            }
            desc.setColumnNo();
            resolveStructComponents(desc);
            structsList.add(desc);

            Index idx = (Index)c.getAnnotation(Index.class);
            if (idx != null) {
                addIndex(desc, idx);
            }
            Indexes indices = (Indexes)c.getAnnotation(Indexes.class);
            if (indices != null) {
                for (Index ind : indices.value()) {
                    addIndex(desc, ind);
                }
            }
            desc.indices = (IndexDescriptor[])desc.indicesMap.values().toArray(new IndexDescriptor[desc.indicesMap.size()]);
            for (IndexDescriptor idxDesc : desc.indices) {
                idxDesc.indexNo = indexNo++;
                nClassIndices += 1;
            }
            if (nClassIndices == 0) {
                desc.flags |= MCO_DB_TYPINFO_HAS_LIST;
                desc.listIndexNo = indexNo++;
                nClassIndices += 1;
            }
            desc.initSize = (desc.flags & MCO_DB_TYPINFO_COMPACT) == 0
                ? ((desc.size + desc.alignment - 1) & ~(desc.alignment - 1))
                : ((desc.compactSize + desc.compactAlignment - 1) & ~(desc.compactAlignment - 1));
        }
        totalIndices = indexNo;
        int structNo = 0;
        for (StructDescriptor struct : structsList) {
            struct.structNo = structNo++;
        }
    }

    static final String truncateFixLenString(String str) {
        int zero = str.indexOf('\0');
        return zero >= 0 ? str.substring(0, zero) : str;
    }

    Object loadObject(long handle, StructDescriptor desc) {
        ReflectionProvider provider = reflectionProvider;
        try {
            Object obj = desc.loadConstructor.newInstance(); // desc.cls.newInstance();
            for (FieldDescriptor f : desc.fields) {
                StructDescriptor struct = f.components;
                if ((f.flags & MCO_DICT_FLDF_NULLABLE) != 0
                    && getIntField(handle, MCO_DB_FT_BOOL, f.indicatorOffset, f.indicatorCompactOffset, f.indicatorNo) == 0)
                {
                    continue;
                }
                if ((f.flags & MCO_DICT_FLDF_ARRAY) != 0) {
                    if (struct != null) {
                        int len = f.arraySize;
                        Object arr = Array.newInstance(struct.cls, len);
                        for (int i = 0; i < len; i++) {
                            long h = getArrayOfObjectFieldElement(handle, f.offset, f.compactOffset,
                                                                  ((struct.size + struct.alignment - 1) & ~(struct.alignment - 1)),
                                                                  ((struct.compactSize + struct.compactAlignment - 1) & ~(struct.compactAlignment - 1)),
                                                                  f.fieldNo, i, len);
                            if (h != 0) {
                                Array.set(arr, i, loadObject(h, struct));
                                freeHandle(h);
                            }
                        }
                        provider.set(f.fld, obj, arr);
                    } else if (f.type == MCO_DB_FT_STRING) {
                        String[] arr = new String[f.arraySize];
                        for (int i = 0; i < arr.length; i++) {
                            arr[i] = new String(getArrayOfStringFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, f.arraySize), f.encoding);
                        }
                        provider.set(f.fld, obj, arr);
                    } else if (f.type == MCO_DB_FT_UNICODE_STRING) {
                        String[] arr = new String[f.arraySize];
                        for (int i = 0; i < arr.length; i++) {
                            arr[i] = getArrayOfWStringFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, f.arraySize);
                        }
                        provider.set(f.fld, obj, arr);
                    } else {
                        provider.set(f.fld, obj, getArrayOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.arraySize));
                    }
                } else if ((f.flags & MCO_DICT_FLDF_VECTOR) != 0) {
                    if (struct != null) {
                        int len = getVectorOfObjectFieldLength(handle, f.offset, f.compactOffset, f.fieldNo);
                        Object col;
                        if (f.isList) {
                            ArrayList list = new ArrayList(len);
                            for (int i = 0; i < len; i++) {
                                long h = getVectorOfObjectFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i);
                                if (h != 0) {
                                    list.add(loadObject(h, struct));
                                    freeHandle(h);
                                } else {
                                    list.add(null);
                                }
                            }
                            col = list;
                        } else {
                            col = Array.newInstance(f.components.cls, len);
                            for (int i = 0; i < len; i++) {
                                long h = getVectorOfObjectFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i);
                                if (h != 0) {
                                    Array.set(col, i, loadObject(h, struct));
                                    freeHandle(h);
                                }
                            }
                        }
                        provider.set(f.fld, obj, col);
                    } else if (f.type == MCO_DB_FT_STRING) {
                        int len = getVectorOfObjectFieldLength(handle, f.offset, f.compactOffset, f.fieldNo);
                        String[] arr = new String[len];
                        for (int i = 0; i < len; i++) {
                            arr[i] = new String(getVectorOfStringFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i), f.encoding);
                        }
                        provider.set(f.fld, obj, arr);
                    } else if (f.type == MCO_DB_FT_UNICODE_STRING) {
                        int len = getVectorOfObjectFieldLength(handle, f.offset, f.compactOffset, f.fieldNo);
                        String[] arr = new String[len];
                        for (int i = 0; i < len; i++) {
                            arr[i] = getVectorOfWStringFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i);
                        }
                        provider.set(f.fld, obj, arr);
                    } else {
                        provider.set(f.fld, obj, getVectorOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo));
                    }
                } else {
                    if ((f.flags & MCO_DICT_FLDF_NULLABLE) != 0) {
                        switch (f.type) {
                        case MCO_DB_FT_UINT2:
                            provider.set(f.fld, obj, new Character((char)getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)));
                            continue;
                        case MCO_DB_FT_INT1:
                            if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                                provider.set(f.fld, obj, new BigDecimal(BigInteger.valueOf(getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)), f.precision));
                            } else {
                                provider.set(f.fld, obj, new Byte((byte)getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)));
                            }
                            continue;
                        case MCO_DB_FT_INT2:
                            if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                                provider.set(f.fld, obj, new BigDecimal(BigInteger.valueOf(getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)), f.precision));
                            } else {
                                provider.set(f.fld, obj, new Short((short)getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)));
                            }
                            continue;
                        case MCO_DB_FT_INT4:
                            if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                                provider.set(f.fld, obj, new BigDecimal(BigInteger.valueOf(getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)), f.precision));
                            } else {
                                provider.set(f.fld, obj, new Integer((int)getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)));
                            }
                            continue;
                        case MCO_DB_FT_FLOAT:
                            provider.set(f.fld, obj, new Float((float)getRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)));
                            continue;
                        case MCO_DB_FT_DOUBLE:
                            provider.set(f.fld, obj, new Double(getRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)));
                            continue;
                        case MCO_DB_FT_DATETIME:
                            provider.set(f.fld, obj, new Date(getDateTimeField(handle, f.offset, f.compactOffset, f.fieldNo)));
                            continue;
                        case MCO_DB_FT_INT8:
                        case MCO_DB_FT_AUTOID:
                            if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                                provider.set(f.fld, obj, new BigDecimal(BigInteger.valueOf(getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)), f.precision));
                            } else {
                                provider.set(f.fld, obj, new Long(getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)));
                            }
                            continue;
                        case MCO_DB_FT_BOOL:
                            provider.set(f.fld, obj, new Boolean(getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo) != 0));
                            continue;
                        }
                    }
                    switch (f.type) {
                    case MCO_DB_FT_UINT2:
                        provider.setChar(f.fld, obj, (char)getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo));
                        break;
                    case MCO_DB_FT_INT1:
                        if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                            provider.set(f.fld, obj, new BigDecimal(BigInteger.valueOf(getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)), f.precision));
                        } else {
                            provider.setByte(f.fld, obj, (byte)getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo));
                        }
                        break;
                    case MCO_DB_FT_INT2:
                        if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                            provider.set(f.fld, obj, new BigDecimal(BigInteger.valueOf(getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)), f.precision));
                        } else {
                            provider.setShort(f.fld, obj, (short)getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo));
                        }
                        break;
                    case MCO_DB_FT_INT4:
                        if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                            provider.set(f.fld, obj, new BigDecimal(BigInteger.valueOf(getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)), f.precision));
                        } else if (f.enumClass != null) {
                            provider.set(f.fld, obj, f.enumClass.getEnumConstants()[(int)getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)]);
                        } else {
                            provider.setInt(f.fld, obj, (int)getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo));
                        }
                        break;
                    case MCO_DB_FT_CHARS:
                        provider.set(f.fld, obj, truncateFixLenString(new String((byte[])getArrayOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.size), f.encoding)));
                        break;
                    case MCO_DB_FT_UNICODE_CHARS:
                        provider.set(f.fld, obj, truncateFixLenString(new String((char[])getArrayOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.size/2))));
                        break;
                    case MCO_DB_FT_STRING:
                        provider.set(f.fld, obj, new String(getStringField(handle, f.offset, f.compactOffset, f.fieldNo), f.encoding));
                        break;
                    case MCO_DB_FT_UNICODE_STRING:
                        provider.set(f.fld, obj, getWStringField(handle, f.offset, f.compactOffset, f.fieldNo));
                        break;
                    case MCO_DB_FT_FLOAT:
                        provider.setFloat(f.fld, obj, (float)getRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo));
                        break;
                    case MCO_DB_FT_DOUBLE:
                        provider.setDouble(f.fld, obj, getRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo));
                        break;
                    case MCO_DB_FT_DATETIME:
                        provider.set(f.fld, obj, new Date(getDateTimeField(handle, f.offset, f.compactOffset, f.fieldNo)));
                        break;
                    case MCO_DB_FT_INT8:
                    case MCO_DB_FT_AUTOID:
                        if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                            provider.set(f.fld, obj, new BigDecimal(BigInteger.valueOf(getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)), f.precision));
                        } else {
                            provider.setLong(f.fld, obj, getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo));
                        }
                        break;
                    case MCO_DB_FT_BOOL:
                        provider.setBoolean(f.fld, obj, getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo) != 0);
                        break;
                    case MCO_DB_FT_STRUCT:
                    {
                        long h = (f.flags & MCO_DICT_FLDF_OPTIONAL) != 0
                            ? getOptStructField(handle, f.offset, f.compactOffset, f.fieldNo)
                            : getStructField(handle, f.offset, f.compactOffset, f.fieldNo);
                        if (h != 0) {
                            provider.set(f.fld, obj, loadObject(h, f.components));
                            freeHandle(h);
                        }
                        break;
                    }
                    case MCO_DB_FT_BLOB:
                        provider.set(f.fld, obj, getBlobField(handle, f.offset, f.compactOffset, f.fieldNo));
                        break;
                    case MCO_DB_FT_SEQUENCE_INT1:
                    case MCO_DB_FT_SEQUENCE_INT2:
                    case MCO_DB_FT_SEQUENCE_INT4:
                    case MCO_DB_FT_SEQUENCE_INT8:
                    case MCO_DB_FT_SEQUENCE_UINT1:
                    case MCO_DB_FT_SEQUENCE_UINT2:
                    case MCO_DB_FT_SEQUENCE_UINT4:
                    case MCO_DB_FT_SEQUENCE_UINT8:
                    case MCO_DB_FT_SEQUENCE_FLOAT:
                    case MCO_DB_FT_SEQUENCE_DOUBLE:
                    case MCO_DB_FT_SEQUENCE_CHAR:
                    case MCO_DB_FT_SEQUENCE_DATETIME:
                        if (f.fld.getType().isArray()) {
                            provider.set(f.fld, obj, getSequenceField(handle, f.type, f.offset, f.compactOffset, f.seqElemSize));
                        } else {
                            provider.set(f.fld, obj, f.seqOrder == 0
                                         ? new UnorderedSequence(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.seqOrder, f.seqElemSize)
                                         : new OrderedSequence(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.seqOrder, f.seqElemSize));
                        }
                        break;
                    }
                }
            }
            if (obj instanceof Initializable) {
                ((Initializable)obj).initialize();
            }
            return obj;
        } catch (Exception x) {
            throw new RuntimeException(x);
        }
    }

    private static String getString(Object s) {
        return s == null ? "" : (String)s;
    }

    void storeObject(long handle, StructDescriptor desc, Object obj) {
        try {
            for (FieldDescriptor f : desc.fields) {
                boolean indexAware = (f.flags & MCO_DICT_FLDF_INDEXED) != 0;
                if ((f.flags & MCO_DICT_FLDF_NULLABLE) != 0) {
                    Object val = f.fld.get(obj);
                    if (val != null) {
                        setIntField(handle, MCO_DB_FT_BOOL, f.indicatorOffset, f.indicatorCompactOffset, f.indicatorNo, (byte)1, indexAware);
                    } else {
                        continue;
                    }
                }
                if ((f.flags & MCO_DICT_FLDF_ARRAY) != 0) {
                    Object arr = f.fld.get(obj);
                    if (arr != null) {
                        StructDescriptor struct = f.components;
                        if (struct != null) {
                            int len = f.arraySize;
                            for (int i = 0; i < len; i++) {
                                Object elem = Array.get(arr, i);
                                if (elem != null) {
                                    long h = setArrayOfObjectFieldElement(handle, f.offset, f.compactOffset,
                                                                          ((struct.size + struct.alignment - 1) & ~(struct.alignment - 1)),
                                                                          ((struct.compactSize + struct.compactAlignment - 1) & ~(struct.compactAlignment - 1)),
                                                                          f.fieldNo, i, len, indexAware);
                                    storeObject(h, struct, elem);
                                    freeHandle(h);
                                }
                            }
                        } else if (f.type == MCO_DB_FT_STRING) {
                            String[] sa = (String[])arr;
                            int len = f.arraySize;
                            for (int i = 0; i < len; i++) {
                                setArrayOfStringFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, len, indexAware, getString(sa[i]).getBytes(f.encoding));
                            }
                        } else if (f.type == MCO_DB_FT_UNICODE_STRING) {
                            String[] sa = (String[])arr;
                            int len = f.arraySize;
                            for (int i = 0; i < len; i++) {
                                setArrayOfWStringFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, len, indexAware, getString(sa[i]));
                            }
                         } else {
                            setArrayOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, arr, f.arraySize, indexAware);
                        }
                    }
                } else if ((f.flags & MCO_DICT_FLDF_VECTOR) != 0) {
                    Object arr = f.fld.get(obj);
                    if (arr != null) {
                        StructDescriptor struct = f.components;
                        if (struct != null) {
                            if (f.isList) {
                                List list = (List)arr;
                                int len = list.size();;
                                setVectorOfObjectFieldSize(handle, f.offset, f.compactOffset, f.fieldNo, len, indexAware);
                                for (int i = 0; i < len; i++) {
                                    Object elem = list.get(i);
                                    if (elem != null) {
                                        long h = setVectorOfObjectFieldElement(handle, f.offset, f.compactOffset,
                                                                               struct.size, struct.alignment, struct.compactSize, struct.compactAlignment,
                                                                               f.fieldNo, i, indexAware);
                                        storeObject(h, struct, elem);
                                        freeHandle(h);
                                    } else {
                                        eraseVectorOfObjectFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, indexAware);
                                    }
                                }
                            } else {
                                int len = Array.getLength(arr);
                                setVectorOfObjectFieldSize(handle, f.offset, f.compactOffset, f.fieldNo, len, indexAware);
                                for (int i = 0; i < len; i++) {
                                    Object elem = Array.get(arr, i);
                                    if (elem != null) {
                                        long h = setVectorOfObjectFieldElement(handle, f.offset, f.compactOffset,
                                                                               struct.size, struct.alignment, struct.compactSize, struct.compactAlignment,
                                                                               f.fieldNo, i, indexAware);
                                        storeObject(h, struct, elem);
                                        freeHandle(h);
                                    } else {
                                        eraseVectorOfObjectFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, indexAware);
                                    }
                                }
                            }
                        } else if (f.type == MCO_DB_FT_STRING) {
                            String[] sa = (String[])arr;
                            int len = sa != null ? sa.length : 0;
                            setVectorOfStringFieldSize(handle, f.offset, f.compactOffset, f.fieldNo, len, indexAware);
                            for (int i = 0; i < len; i++) {
                                setVectorOfStringFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, indexAware, getString(sa[i]).getBytes(f.encoding));
                            }
                        } else if (f.type == MCO_DB_FT_UNICODE_STRING) {
                            String[] sa = (String[])arr;
                            int len = sa != null ? sa.length : 0;
                            setVectorOfWStringFieldSize(handle, f.offset, f.compactOffset, f.fieldNo, len, indexAware);
                            for (int i = 0; i < len; i++) {
                                setVectorOfWStringFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, indexAware, getString(sa[i]));
                            }
                        } else {
                            setVectorOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, arr, indexAware);
                        }
                    }
                } else {
                    if ((f.flags & MCO_DICT_FLDF_NULLABLE) != 0) {
                        Object val = f.fld.get(obj);
                        if (val != null) {
                            switch (f.type) {
                            case MCO_DB_FT_UINT2:
                                setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ((Character)val).charValue(), indexAware);
                                continue;
                            case MCO_DB_FT_INT1:
                            case MCO_DB_FT_INT2:
                            case MCO_DB_FT_INT4:
                            case MCO_DB_FT_INT8:
                            case MCO_DB_FT_AUTOID:
                                if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                                    setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.scale((BigDecimal)val), indexAware);
                                } else if (f.enumClass != null) {
                                    setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ((Enum)val).ordinal(), indexAware);
                                } else {
                                    setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ((Number)val).longValue(), indexAware);
                                }
                                continue;
                            case MCO_DB_FT_DATETIME:
                                setDateTimeField(handle, f.offset, f.compactOffset, f.fieldNo, ((Date)val).getTime(), indexAware);
                                break;
                            case MCO_DB_FT_FLOAT:
                            case MCO_DB_FT_DOUBLE:
                                setRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ((Number)val).doubleValue(), indexAware);
                                continue;
                            case MCO_DB_FT_BOOL:
                                setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ((Boolean)val).booleanValue() ? 1 : 0, indexAware);
                                continue;
                            }
                        } else {
                            continue;
                        }
                    }
                    if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                        setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.scale((BigDecimal)f.fld.get(obj)), indexAware);
                        continue;
                    }
                    switch (f.type) {
                    case MCO_DB_FT_UINT2:
                        setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.fld.getChar(obj), indexAware);
                        break;
                    case MCO_DB_FT_INT1:
                        setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.fld.getByte(obj), indexAware);
                        break;
                    case MCO_DB_FT_INT2:
                        setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.fld.getShort(obj), indexAware);
                        break;
                    case MCO_DB_FT_INT4:
                        if (f.enumClass != null) {
                            setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ((Enum)f.fld.get(obj)).ordinal(), indexAware);
                        } else {
                            setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.fld.getInt(obj), indexAware);
                        }
                        break;
                    case MCO_DB_FT_CHARS:
                        setArrayOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo,
                                              getString(f.fld.get(obj)).getBytes(f.encoding), f.size, indexAware);
                        break;
                    case MCO_DB_FT_UNICODE_CHARS:
                        setArrayOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo,
                                              getString(f.fld.get(obj)).toCharArray(), f.size/2, indexAware);
                        break;
                    case MCO_DB_FT_STRING:
                        setStringField(handle, f.offset, f.compactOffset, f.fieldNo, getString(f.fld.get(obj)).getBytes(f.encoding), indexAware);
                        break;
                    case MCO_DB_FT_UNICODE_STRING:
                        setWStringField(handle, f.offset, f.compactOffset, f.fieldNo, getString(f.fld.get(obj)), indexAware);
                        break;
                    case MCO_DB_FT_FLOAT:
                        setRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.fld.getFloat(obj), indexAware);
                        break;
                    case MCO_DB_FT_DOUBLE:
                        setRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.fld.getDouble(obj), indexAware);
                        break;
                    case MCO_DB_FT_DATETIME:
                        setDateTimeField(handle, f.offset, f.compactOffset, f.fieldNo, ((Date)f.fld.get(obj)).getTime(), indexAware);
                        break;
                    case MCO_DB_FT_INT8:
                    case MCO_DB_FT_AUTOID:
                        setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.fld.getLong(obj), indexAware);
                        break;
                    case MCO_DB_FT_BOOL:
                        setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.fld.getBoolean(obj) ? 1 : 0, indexAware);
                        break;
                    case MCO_DB_FT_STRUCT:
                    {
                        Object s = f.fld.get(obj);
                        if (s != null) {
                            StructDescriptor struct = f.components;
                            long h = (f.flags & MCO_DICT_FLDF_OPTIONAL) != 0
                                ? setOptStructField(handle, struct.structNo, f.offset, f.compactOffset, struct.size, struct.alignment, struct.compactSize, struct.compactAlignment, f.fieldNo)
                                : setStructField(handle, f.offset, f.compactOffset, f.fieldNo, indexAware);
                            storeObject(h, struct, s);
                            freeHandle(h);
                        } else if ((f.flags & MCO_DICT_FLDF_OPTIONAL) != 0) {
                            eraseStructField(handle, f.offset, f.compactOffset, f.fieldNo);
                        } else {
                            throw new IllegalArgumentException("Non-optional field " + f.name + " of class " + f.owner.name + " is assigned null value");
                        }
                        break;
                    }
                    case MCO_DB_FT_BLOB:
                    {
                        byte[] blob = (byte[])f.fld.get(obj);
                        if (blob == null) {
                            blob = new byte[0];
                        }
                        setBlobField(handle, f.offset, f.compactOffset, f.fieldNo, blob);
                        break;
                    }
                    case MCO_DB_FT_SEQUENCE_INT1:
                    case MCO_DB_FT_SEQUENCE_INT2:
                    case MCO_DB_FT_SEQUENCE_INT4:
                    case MCO_DB_FT_SEQUENCE_INT8:
                    case MCO_DB_FT_SEQUENCE_UINT1:
                    case MCO_DB_FT_SEQUENCE_UINT2:
                    case MCO_DB_FT_SEQUENCE_UINT4:
                    case MCO_DB_FT_SEQUENCE_UINT8:
                    case MCO_DB_FT_SEQUENCE_FLOAT:
                    case MCO_DB_FT_SEQUENCE_DOUBLE:
                    case MCO_DB_FT_SEQUENCE_CHAR:
                    case MCO_DB_FT_SEQUENCE_DATETIME:
                    {
                        Object seq = f.fld.get(obj);
                        if (seq != null && !(seq instanceof UnorderedSequence)) {
                            setSequenceField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, seq, f.seqOrder, f.seqElemSize);
                        }
                    }
                    }
                }
            }
        } catch (Exception x) {
            throw new RuntimeException(x);
        }
    }

    void updateObject(long handle, StructDescriptor desc, Object obj) {
        try {
            for (FieldDescriptor f : desc.fields) {
                boolean indexAware = (f.flags & MCO_DICT_FLDF_INDEXED) != 0;
                boolean updateEventFired = false;
                if ((f.flags & MCO_DICT_FLDF_NULLABLE) != 0) {
                    Object val = f.fld.get(obj);
                    if (f.onUpdateEvent != null)  {
                        long prevIndValue = getIntField(handle, MCO_DB_FT_BOOL, f.indicatorOffset, f.indicatorCompactOffset, f.indicatorNo);
                         if ((prevIndValue != 0) ^ (val != null)) {
                             updateField(handle, f.onUpdateEvent.id);
                             updateEventFired = true;
                         }
                     }
                     if (val != null) {
                         setIntField(handle, MCO_DB_FT_BOOL, f.indicatorOffset, f.indicatorCompactOffset,
                                     f.indicatorNo, (byte)1, indexAware);
                     } else {
                         setIntField(handle, MCO_DB_FT_BOOL, f.indicatorOffset, f.indicatorCompactOffset,
                                     f.indicatorNo, (byte)0, indexAware);
                         continue;
                     }
                }
                if ((f.flags & MCO_DICT_FLDF_ARRAY) != 0) {
                    Object arr = f.fld.get(obj);
                    if (arr != null) {
                        StructDescriptor struct = f.components;
                        if (struct != null) {
                            int len = f.arraySize;
                            for (int i = 0; i < len; i++) {
                                Object elem = Array.get(arr, i);
                                if (elem != null) {
                                    long h = setArrayOfObjectFieldElement(handle, f.offset, f.compactOffset,
                                                                          ((struct.size + struct.alignment - 1) & ~(struct.alignment - 1)),
                                                                          ((struct.compactSize + struct.compactAlignment - 1) & ~(struct.compactAlignment - 1)),
                                                                          f.fieldNo, i, len, indexAware);
                                    storeObject(h, struct, elem);
                                    freeHandle(h);
                                }
                            }
                        } else if (f.type == MCO_DB_FT_STRING) {
                            String[] sa = (String[])arr;
                            int len = f.arraySize;
                            for (int i = 0; i < len; i++) {
                                setArrayOfStringFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, len, indexAware, getString(sa[i]).getBytes(f.encoding));
                            }
                        } else if (f.type == MCO_DB_FT_UNICODE_STRING) {
                            String[] sa = (String[])arr;
                            int len = f.arraySize;
                            for (int i = 0; i < len; i++) {
                                setArrayOfWStringFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, len, indexAware, getString(sa[i]));
                            }
                         } else {
                            setArrayOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, arr, f.arraySize, indexAware);
                        }
                    }
                } else if ((f.flags & MCO_DICT_FLDF_VECTOR) != 0) {
                    Object arr = f.fld.get(obj);
                    if (arr != null) {
                        StructDescriptor struct = f.components;
                        if (struct != null) {
                            if (f.isList) {
                                List list = (List)arr;
                                int len = list.size();
                                setVectorOfObjectFieldSize(handle, f.offset, f.compactOffset, f.fieldNo, len, indexAware);
                                for (int i = 0; i < len; i++) {
                                    Object elem = list.get(i);
                                    if (elem != null) {
                                        long h = setVectorOfObjectFieldElement(handle, f.offset, f.compactOffset,
                                                                               struct.size, struct.alignment, struct.compactSize, struct.compactAlignment,
                                                                               f.fieldNo, i, indexAware);
                                        storeObject(h, struct, elem);
                                        freeHandle(h);
                                    } else {
                                        eraseVectorOfObjectFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, indexAware);
                                    }
                                }
                            } else {
                                int len = Array.getLength(arr);
                                setVectorOfObjectFieldSize(handle, f.offset, f.compactOffset, f.fieldNo, len, indexAware);
                                for (int i = 0; i < len; i++) {
                                    Object elem = Array.get(arr, i);
                                    if (elem != null) {
                                        long h = setVectorOfObjectFieldElement(handle, f.offset, f.compactOffset,
                                                                               struct.size, struct.alignment, struct.compactSize, struct.compactAlignment,
                                                                               f.fieldNo, i, indexAware);
                                        storeObject(h, struct, elem);
                                        freeHandle(h);
                                    } else {
                                        eraseVectorOfObjectFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, indexAware);
                                    }
                                }
                            }
                        } else if (f.type == MCO_DB_FT_STRING) {
                            String[] sa = (String[])arr;
                            int len = sa != null ? sa.length : 0;
                            setVectorOfStringFieldSize(handle, f.offset, f.compactOffset, f.fieldNo, len, indexAware);
                            for (int i = 0; i < len; i++) {
                                setVectorOfStringFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, indexAware, getString(sa[i]).getBytes(f.encoding));
                            }
                        } else if (f.type == MCO_DB_FT_UNICODE_STRING) {
                            String[] sa = (String[])arr;
                            int len = sa != null ? sa.length : 0;
                            setVectorOfWStringFieldSize(handle, f.offset, f.compactOffset, f.fieldNo, len, indexAware);
                            for (int i = 0; i < len; i++) {
                                setVectorOfWStringFieldElement(handle, f.offset, f.compactOffset, f.fieldNo, i, indexAware, getString(sa[i]));
                            }
                        } else {
                            setVectorOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, arr, indexAware);
                        }
                    }
                } else {
                    long ivalue;
                    double rvalue;
                    String svalue;
                    if ((f.flags & MCO_DICT_FLDF_NULLABLE) != 0) {
                        Object val = f.fld.get(obj);
                        if (val != null) {
                            switch (f.type) {
                            case MCO_DB_FT_UINT2:
                                ivalue = ((Character)val).charValue();
                                if (ivalue !=  getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)) {
                                    setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ivalue, indexAware);
                                    if (f.onUpdateEvent != null)  {
                                        updateField(handle, f.onUpdateEvent.id);
                                    }
                                }
                                continue;
                            case MCO_DB_FT_INT1:
                            case MCO_DB_FT_INT2:
                            case MCO_DB_FT_INT4:
                            case MCO_DB_FT_INT8:
                            case MCO_DB_FT_AUTOID:
                                if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                                    ivalue = f.scale((BigDecimal)val);
                                } else {
                                    ivalue = ((Number)val).longValue();
                                }
                                if (ivalue != getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)) {
                                    setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ivalue, indexAware);
                                    if (f.onUpdateEvent != null)  {
                                        updateField(handle, f.onUpdateEvent.id);
                                    }
                                }
                                continue;
                            case MCO_DB_FT_FLOAT:
                            case MCO_DB_FT_DOUBLE:
                                rvalue = ((Number)val).doubleValue();
                                if (rvalue != getRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)) {
                                    setRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, rvalue, indexAware);
                                    if (f.onUpdateEvent != null)  {
                                        updateField(handle, f.onUpdateEvent.id);
                                    }
                                }
                                continue;
                            case MCO_DB_FT_DATETIME:
                                ivalue = ((Date)val).getTime();
                                if (ivalue != getDateTimeField(handle, f.offset, f.compactOffset, f.fieldNo)) {
                                    setDateTimeField(handle, f.offset, f.compactOffset, f.fieldNo, ivalue, indexAware);
                                    if (f.onUpdateEvent != null)  {
                                        updateField(handle, f.onUpdateEvent.id);
                                    }
                                }
                                continue;
                            case MCO_DB_FT_BOOL:
                                ivalue = ((Boolean)val).booleanValue() ? 1 : 0;
                                if (ivalue != getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)) {
                                    setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ivalue, indexAware);
                                    if (f.onUpdateEvent != null)  {
                                        updateField(handle, f.onUpdateEvent.id);
                                    }
                                }
                                continue;
                            }
                        } else {
                            continue;
                        }
                    }
                    switch (f.type) {
                    case MCO_DB_FT_UINT2:
                        ivalue = f.fld.getChar(obj);
                        if (ivalue != getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)) {
                            setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ivalue, indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_INT1:
                        if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                            ivalue = f.scale((BigDecimal)f.fld.get(obj));
                        } else {
                            ivalue = f.fld.getByte(obj);
                        }
                        if (ivalue != getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)) {
                            setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ivalue, indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_INT2:
                        if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                            ivalue = f.scale((BigDecimal)f.fld.get(obj));
                        } else {
                            ivalue = f.fld.getShort(obj);
                        }
                        if (ivalue != getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)) {
                            setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ivalue, indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_INT4:
                        if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                            ivalue = f.scale((BigDecimal)f.fld.get(obj));
                        } else if (f.enumClass != null) {
                            ivalue = ((Enum)f.fld.get(obj)).ordinal();
                        } else {
                            ivalue = f.fld.getInt(obj);
                        }
                        if (ivalue != getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)) {
                            setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ivalue, indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_CHARS:
                        svalue = getString(f.fld.get(obj));
                        if (!svalue.equals(truncateFixLenString(new String((byte[])getArrayOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.size), f.encoding)))) {
                            setArrayOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo,
                                                  svalue.getBytes(f.encoding), f.size, indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_UNICODE_CHARS:
                        svalue = getString(f.fld.get(obj));
                        if (!svalue.equals(truncateFixLenString(new String((char[])getArrayOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, f.size/2))))) {
                            setArrayOfScalarField(handle, f.type, f.offset, f.compactOffset, f.fieldNo,
                                                  svalue.toCharArray(), f.size/2, indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_STRING:
                        svalue = getString(f.fld.get(obj));
                        if (!svalue.equals(new String(getStringField(handle, f.offset, f.compactOffset, f.fieldNo), f.encoding))) {
                            setStringField(handle, f.offset, f.compactOffset, f.fieldNo, svalue.getBytes(f.encoding), indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_UNICODE_STRING:
                        svalue = getString(f.fld.get(obj));
                        if (!svalue.equals(getWStringField(handle, f.offset, f.compactOffset, f.fieldNo))) {
                            setWStringField(handle, f.offset, f.compactOffset, f.fieldNo, getString(f.fld.get(obj)), indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_FLOAT:
                        rvalue = f.fld.getFloat(obj);
                        if (rvalue != getRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)) {
                            setRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, rvalue, indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_DOUBLE:
                        rvalue = f.fld.getDouble(obj);
                        if (rvalue != getRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)) {
                            setRealField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, rvalue, indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_DATETIME:
                        ivalue = ((Date)f.fld.get(obj)).getTime();
                        if (ivalue != getDateTimeField(handle, f.offset, f.compactOffset, f.fieldNo)) {
                            setDateTimeField(handle, f.offset, f.compactOffset, f.fieldNo, ivalue, indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_INT8:
                    case MCO_DB_FT_AUTOID:
                        if ((f.flags & MCO_DICT_FLDF_NUMERIC) != 0) {
                            ivalue = f.scale((BigDecimal)f.fld.get(obj));
                        } else {
                            ivalue = f.fld.getLong(obj);
                        }
                        if (ivalue != getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)) {
                            setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ivalue, indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_BOOL:
                        ivalue = f.fld.getBoolean(obj) ? 1 : 0;
                        if (ivalue != getIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo)) {
                            setIntField(handle, f.type, f.offset, f.compactOffset, f.fieldNo, ivalue, indexAware);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    case MCO_DB_FT_STRUCT:
                    {
                        Object s = f.fld.get(obj);
                        if (s != null) {
                            StructDescriptor struct = f.components;
                            long h = (f.flags & MCO_DICT_FLDF_OPTIONAL) != 0
                                ? setOptStructField(handle, struct.structNo, f.offset, f.compactOffset, struct.size, struct.alignment, struct.compactSize, struct.compactAlignment, f.fieldNo)
                                : setStructField(handle, f.offset, f.compactOffset, f.fieldNo, indexAware);
                            updateObject(h, struct, s);
                            freeHandle(h);
                        } else {
                            eraseStructField(handle, f.offset, f.compactOffset, f.fieldNo);
                        }
                        break;
                    }
                    case MCO_DB_FT_BLOB:
                    {
                        byte[] blob = (byte[])f.fld.get(obj);
                        if (blob == null) {
                            blob = new byte[0];
                        }
                        if (!Arrays.equals(blob, getBlobField(handle, f.offset, f.compactOffset, f.fieldNo))) {
                            setBlobField(handle, f.offset, f.compactOffset, f.fieldNo, blob);
                        } else {
                            continue; // do not trigger update event on this fields
                        }
                        break;
                    }
                    default: // ignore sequences
                    }
                }
                if (!updateEventFired && f.onUpdateEvent != null)  {
                    updateField(handle, f.onUpdateEvent.id);
                }
            }
            updateObject(handle);
        } catch (Exception x) {
            throw new RuntimeException(x);
        }
    }

    int findEvent(String name) {
        EventDescriptor desc = events.get(name);
        if (desc == null) {
            throw new IllegalArgumentException("No such event " + name);
        }
        return desc.id+1;
    }

    long getAutoId(long handle) {
        int autoIdOffs = ((config & (MCO_CFG_HA_SUPPORT|MCO_CFG_CLUSTER_SUPPORT|MCO_CFG_MPI_CLUSTER_SUPPORT|MCO_CFG_TL_SUPPORT)) != 0) ? 8 : 0;
        return getIntField(handle, MCO_DB_FT_AUTOID, autoIdOffs, autoIdOffs, 0);
    }

    ClassDescriptor getClassDescriptor(Class cls) {
        StructDescriptor desc = structsMap.get(cls);
        if (desc instanceof ClassDescriptor) {
            return (ClassDescriptor)desc;
        } else {
            throw new IllegalArgumentException("No class descriptor for class " + cls);
        }
    }


    private native void   open(String name, Object[] structs, ClassDescriptor[] classes, boolean hasInmemDistribClasses, boolean hasDiskDistribClasses, int totalFields, int totalIndices, int totalKeys, int totalEvents, Device[] devices, Parameters params);
    private native Database.RuntimeInfo getRuntimeInfo();
    private native void   updateObject(long handle);
    private native void   updateField(long handle, int eventId);
    private native long   getIntField(long handle, int type, int offset, int compactOffset, int fieldNo);
    private native long   getDateTimeField(long handle, int offset, int compactOffset, int fieldNo);
    private native double getRealField(long handle, int type, int offset, int compactOffset, int fieldNo);
    private native byte[] getStringField(long handle, int offset, int compactOffset, int fieldNo);
    private native String getWStringField(long handle, int offset, int compactOffset, int fieldNo);
    private native Object getArrayOfScalarField(long handle, int type, int offset, int compactOffset, int fieldNo, int length);
    private native Object getVectorOfScalarField(long handle, int type, int offset, int compactOffset, int fieldNo);
    private native long   getOptStructField(long handle, int offset, int compactOffset, int fieldNo);
    private native long   getStructField(long handle, int offset, int compactOffset, int fieldNo);
    private native long   getArrayOfObjectFieldElement(long handle, int offset, int compactOffset, int structSize, int compactStructSize, int fieldNo, int index, int length);
    private native int    getVectorOfObjectFieldLength(long handle, int offset, int compactOffset, int fieldNo);
    private native long   getVectorOfObjectFieldElement(long handle, int offset, int compactOffset, int fieldNo, int index);
    private native byte[] getArrayOfStringFieldElement(long handle, int offset, int compactOffset, int fieldNo, int index, int length);
    private native byte[] getVectorOfStringFieldElement(long handle, int offset, int compactOffset, int fieldNo, int index);
    private native String getArrayOfWStringFieldElement(long handle, int offset, int compactOffset, int fieldNo, int index, int length);
    private native String getVectorOfWStringFieldElement(long handle, int offset, int compactOffset, int fieldNo, int index);
    private native byte[] getBlobField(long handle, int offset, int compactOffset, int fieldNo);
    private native Object getSequenceField(long handle, int type, int offset, int compactOffset, int seqElemSize);

    private native void   setIntField(long handle, int type, int offset, int compactOffset, int fieldNo, long value, boolean indexAware);
    private native void   setDateTimeField(long handle, int offset, int compactOffset, int fieldNo, long value, boolean indexAware);
    private native void   setRealField(long handle, int type, int offset, int compactOffset, int fieldNo, double value, boolean indexAware);
    private native void   setStringField(long handle, int offset, int compactOffset, int fieldNo, byte[] value, boolean indexAware);
    private native void   setWStringField(long handle, int offset, int compactOffset, int fieldNo, String value, boolean indexAware);
    private native void   setArrayOfScalarField(long handle, int type, int offset, int compactOffset, int fieldNo, Object value, int length, boolean indexAware);
    private native void   setVectorOfScalarField(long handle, int type, int offset, int compactOffset, int fieldNo, Object value, boolean indexAware);
    private native long   setOptStructField(long handle, int structNo, int offset, int compactOffset, int size, int alignment,
                                         int compactSize, int compactAlignment, int fieldNo);
    private native long   setStructField(long handle, int offset, int compactOffset, int fieldNo, boolean indexAware);
    private native void   eraseStructField(long handle, int offset, int compactOffset, int fieldNo);
    private native void   eraseVectorOfObjectFieldElement(long handle, int offset, int compactOffset, int fieldNo, int index, boolean indexAare);
    private native void   setVectorOfObjectFieldSize(long handle, int offset, int compactOffset, int fieldNo, int size, boolean indexAware);
    private native long   setVectorOfObjectFieldElement(long handle, int offset, int compactOffset, int size, int alignment,
                                                        int compactSize, int compactAlignment, int fieldNo, int index, boolean indexAware);
    private native long   setArrayOfObjectFieldElement(long handle, int offset, int compactOffset, int size,
                                                       int compactSize, int fieldNo, int index, int length, boolean indexAware);
    private native void   setVectorOfStringFieldSize(long handle, int offset, int compactOffset, int fieldNo, int size, boolean indexAware);
    private native void   setVectorOfStringFieldElement(long handle, int offset, int compactOffset, int fieldNo, int index, boolean indexAware, byte[] value);
    private native void   setArrayOfStringFieldElement(long handle, int offset, int compactOffset, int fieldNo, int index, int length, boolean indexAware, byte[] value);
    private native void   setVectorOfWStringFieldSize(long handle, int offset, int compactOffset, int fieldNo, int size, boolean indexAware);
    private native void   setVectorOfWStringFieldElement(long handle, int offset, int compactOffset, int fieldNo, int index, boolean indexAware, String value);
    private native void   setArrayOfWStringFieldElement(long handle, int offset, int compactOffset, int fieldNo, int index, int length, boolean indexAware, String value);
    private native void   setBlobField(long handle, int offset, int compactOffset, int fieldNo, byte[] value);
    private native void   setSequenceField(long handle, int type, int offset, int compactOffset, int fieldNo, Object value, int seqOrder, int elemSize);
    private native void   close(String name);
    private native void   extend(String name, Device device, byte [] context);
    private native boolean detach(String name, Device device, byte [] context);
    static  native void   freeHandle(long handle);
    static  native int    getPointerSize();
    static  native void   runtimeStart(int config);
    static  native void   runtimeStop();
    static  native void   loadDependentLibraries(int config);
    static  native void   setCurrentTimeCallbackInt(GetCurrentTimeCallback callback);
    public  static native int getCurrentTime();
    static  native void   setRuntimeOptionInt(int option, int value);
    static  native int    getRuntimeOptionInt(int option);

    public static native void sslInit();
    public static native void sslLoadVerifyLocations(String caFile, String caPath);

    public static native void aioStart(long max_queue_length, long n_workers);
    public static native void aioStop();

    ClassDescriptor[] classes;
    HashMap<Class, StructDescriptor> structsMap;
    ArrayList<StructDescriptor> structsList;
    Parameters params;

    int totalIndices;
    int totalFields;
    int totalKeys;
    int config;
    String name;
    long quorumCallbackMemory;
    long notifyCallbackMemory;
    long SSLParamsMemory;
    long haNotifyCallbackMemory;
    long haErrHandlerMemory;
    String defaultEncoding;
    boolean hasInmemDistribClasses;
    boolean hasDiskDistribClasses;
    static ReflectionProvider reflectionProvider;
    HashMap<String, EventDescriptor> events;
};

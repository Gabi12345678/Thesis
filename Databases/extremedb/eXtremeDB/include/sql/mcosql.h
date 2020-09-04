/*******************************************************************
 *                                                                 *
 *  mcosql.h                                                       *
 *                                                                 *
 *  This file is a part of the eXtremeDB source code               *
 *  Copyright (c) 2001-2019 McObject LLC                           * 
 *  All Rights Reserved                                            *
 *                                                                 *
 *******************************************************************/
#ifndef __MCOSQL_H__
#define __MCOSQL_H__


#include "sqlcpp.h"
#include "dbapi.h"
#include "mcoapi.h"
#include "mcosmp.h"

#ifdef _WIN32
#  define MCOSQL_UDF __declspec(dllexport)
#else
#  define MCOSQL_UDF
#endif

class MCOSQL_API McoSqlOpenParameters
{
public:
    enum OpenFlags
    {
        ALLOCATE_MEMORY = 1,  
        /* Allocate memory buffer of specified size for eXtremeDB database*/
        /* using malloc function (value of mapAddress parameter is ignored).*/
        /* If this flag is not set, specified value of mapAddress parameter is used.*/
        /* This flag is ignored for shared memory database, in which case mapAddress*/
        /* always specifies mapping address. */
        SET_ERROR_HANDLER = 2,  /* Set McoSql specific error handler. */
        START_MCO_RUNTIME = 4,  /* Start MCO runtime.*/
        INITIALIZE_DATABASE = 16,  /* Initialize new database instance (call mco_db_open).*/
        PRESERVE_SHARED_DATABASE = 32, /* Do not close database, keeping it in shared memory*/
        FOREIGN_CONNECTION = 64, /* Connection was established outside SqlEngine: do not perform disconnect and database close */
        DEFAULT_OPEN_FLAGS = ALLOCATE_MEMORY | SET_ERROR_HANDLER | START_MCO_RUNTIME | INITIALIZE_DATABASE 
    };

    char const* databaseName;
    mco_dictionary_h dictionary;
    size_t mainMemoryDatabaseSize;
    void* mainMemoryDatabaseAddress;
    size_t maxConnections;
    size_t maxTransactionSize;
    size_t mainMemoryPageSize;
    uint2 hashLoadFactor;
    int flags;
    int modeMask;

    char* savedImage;

    char* diskDatabaseFile;
    char* diskDatabaseLogFile;
    mco_offs_t diskDatabaseMaxSize;
    void* diskCacheMemoryAddress;
    size_t diskCacheSize;
    int diskPageSize;
    MCO_LOG_TYPE logType;
    mco_offs_t redoLogLimit;
	int maxDelayedTransactions;
    MCO_COMMIT_POLICY defaultCommitPolicy;
    int  compressionLevel;
    MCO_TRANS_SCHED_POLICY transSchedPolicy;
    mco_stream_read streamReader;
    void* stream;
    McoSql::SqlEngine* remoteEngine;

    uint8 maxTransTime; /* maximal transaction time, 0 to disable */
	int maxActivePages;

    mco_device_t* devices;
    size_t        n_devices;

    McoSql::Table** tables;
    size_t          n_tables;

    McoSql::SqlOptimizerParameters optimizerParams;

    size_t maxClasses;
    size_t maxIndexes;
    size_t maxDictionarySize;
    int    dictionaryFlags;

    size_t           backupMapSize;
    unsigned int     backupMinPages;
    unsigned int     backupMaxPasses;
    char            *backupMapFile;

	uint4            fileBackupDelay;

    void setDefaultValues();

    McoSqlOpenParameters()
    {
        setDefaultValues();
    }

    McoSqlOpenParameters(char const* name, mco_dictionary_h dictionary, size_t size, size_t pageSize = 128,
                         void* mapAddress = MCO_DATABASE_DEFAULT_MAP_ADDRESS,
                         size_t maxTransSize = 0, 
                         int flags = DEFAULT_OPEN_FLAGS, 
                         char const* savedImage = NULL);
};

struct MCOSQL_API WaitEventProfiler
{
	bool running;
	uint64_t waitEvents[MCO_WAIT_EVENT_LAST];
	timer_unit rate;
	mco_thread_t* thread;
	uint64_t probes;
};

class MCOSQL_API McoSqlEngine: public McoSql::SqlEngine
{
    friend class McoSqlSession;
protected:
    mco_db_h dbh;
    McoDatabase db;
    void* addr;
    size_t pageSize;
    bool diskDatabase;
    int flags;
    char * license_key;
    size_t free_memory_threshold;
    McoSqlEngine(McoSqlEngine* engine);
    virtual void init();
    size_t threadId;
	WaitEventProfiler profiler;

	static void profileProc(void* arg);
	void profile(void);

  public:
    McoSqlEngine(); /* default constructor*/

    /**
     * Open database: open eXtremeDB database, eXtremeDB SQL mapper and McoSql engine.
     * @param openParams parameters of database open
     * @throws RuntimeException in case of error
     */
    void open(McoSqlOpenParameters const &params);

    /**
     * Open database: open eXtremeDB SQL mapper and McoSql engine given eXtremeDB database handle
     * @param dbh eXtremeDB database handle
     * @throws RuntimeException in case of error
     */
    void open(mco_db_h dbh);

    /**
     * Open database: open eXtremeDB database, eXtremeDB SQL mapper and McoSql engine.
     * @param name database name 
     * @param dictionary database scheme definition
     * @param size eXtremeDB database size (bytes)
     * @param pageSize eXtremeDB page size (bytes)
     * @param mapAddress eXtremeDB map address (for shared memory database)
     * @param maxTransSize eXtremeDB limit for transaction size (object)
     * If value of this parameter is 0, then default transaction size limit is used.
     * @param flags optional open flags (see OpenFlags enum)
     * @param databaseFile database file to be loaded 
     * @param maxClasses limit for number of classes
     * @param maxIndexes limit for number of indexes
     * @param maxDictionarySize limit for total dictionary size
     * @throws RuntimeException in case of error
     */
    void open(char const* name, mco_dictionary_h dictionary, size_t size, 
              size_t pageSize = 128,
              void* mapAddress = MCO_DATABASE_DEFAULT_MAP_ADDRESS, 
              size_t maxTransSize = 0, 
              int flags = McoSqlOpenParameters::DEFAULT_OPEN_FLAGS, 
              char const* databaseFile = NULL,
              size_t maxClasses = 100, size_t maxIndexes = 100, size_t maxDictionarySize = 16*1024);


    /**
     * Create connection pool needed for parallel query execution.
     */
    void createConnectionPool();

    /**
     * Destroy connection pool created by  createConnectionPool()
     */
    void destroyConnectionPool();

	size_t connectionPoolSize();

    void registerExternalTables(McoSql::Table** tables,  size_t n_tables);

    /**
     * Save database to the specified file.
     * @param databaseFile path to the file where database has to be stored
     */
    void save(char const *databaseFile);
    void save(void *stream, mco_stream_write streamWriter);

    /**
     * Save table content to the specified file.
     * @param filePath path to the file where table has to be stored
     * @param className name of the table to save
     */
    void saveClass(char const *filePath, char const *className);

    /**
     * Load table content from the specified file.
     * @param filePath path to the file from where table has to be loaded
     * @param className name of the table to load or NULL
     */
    void loadClass(char const *filePath, char const *className = 0);

    /**
     * Save database metadata compatible with xsql utility to the specified file.
     * @param metadataFile path to the file where database metadata has to be stored
     */
    void saveMetadata(char const *metadataFile);
    void saveMetadata(void *stream, mco_stream_write streamWriter);

    /**
     * Save database dictionary to the specified file.
     * @param dictionaryFile path to the file where database dictionary has to be stored
     */
    void saveDictionary(char const *dictionaryFile);
    void saveDictionary(void *stream, mco_stream_write streamWriter);

    /**
     * Online backup of disk database
     * @param dbFile path to database backup file
     * @param logFile path to transaction log backup file
     * @param nThreads number of concurrent threads
     */
    void fileBackup(char const *dbFile, char const* logFile, int nThreads = 1);
        
    void setFreeMemoryThreshold(size_t threshold);
    size_t getFreeMemoryThreshold();

    /**
     * Close database.
     */
    virtual void close();

    /**
     * Disconnect database.
     */
    void disconnect();

    /**
     * Get database handle which can be used by eXtremeDB API functions.
     */
    mco_db_h getHandle()
    {
        return dbh;
    }

    /**
     * Get autoid of last allocated objects 
     */
    int64_t getLastAutoid();
            
    /**
     * Change transaction commit policy for the specified database connections
     * @param policy one of MCO_COMMIT_ASYNC_FLUSH, MCO_COMMIT_SYNC_FLUSH or MCO_COMMIT_NO_FLUSH
     * @return previous policy
     */
    MCO_COMMIT_POLICY setTransactionPolicy(MCO_COMMIT_POLICY policy);

    /**
     * Flush all changes done by committed transactions to the disk.
     * This method can be used to save to the disk changes when MCO_COMMIT_NO_FLUSH transaction policy is used
     */
    void flush();

    /**
     * Get database name
     */
    char* getName();

    /** 
     * Do not close eXtremeDB database keeping it in shared memory
     */
    void detach();

    /** 
     * Returns true if detach() was called
     */
    bool isDetached();

    /**
     * Get eXtremeDB page size.
     */
    size_t getPageSize();

    /**
     * Get number of free in-memory pages.
     */
    size_t getFreePages();

    /**
     * Get total number of in-memory pages.
     */
    size_t getTotalPages();

    /**
     * Get information about disk usage.
     * @return false if disk is not supported
     */
    bool getDiskInfo(mco_disk_info_t& info);

    /**
     * Get information about disk allocator fragmentation.
     * @return false if disk is not supported
     */
    bool getDiskAllocInfo(mco_disk_alloc_info_t& unaligned, mco_disk_alloc_info_t& aligned);

    /**
     * Set license key (for license-protected packages only)
     * The license key have to be set before call of any of open() method
     */
    void license( char * key ) {
        license_key = key;
    }
	
    void setInterrupted(bool enable);

	void backupCreate(McoSql::String* file, McoSql::String* label, McoSql::BackupKind kind, int compressionLevel, McoSql::String* cipherKey);
	void backupRestore(McoSql::String* file, McoSql::String* label, McoSql::String* cipherKey);
	void backupVerify(McoSql::String* file, McoSql::String* label, McoSql::String* cipherKey);
	void backupInfo(McoSql::String* file, McoSql::SyntheticTable* backupInfo, McoSql::Allocator* allocator);

	bool copyFile(mco_file_h dst, mco_file_h src, uint4 delay);

	virtual void startQuery(char const* sql);
	virtual void endQuery();

	virtual void updateConnectionInfo(McoSql::SyntheticTable* table, McoSql::Runtime* runtime);

	void startProfile(timer_unit rate);
	McoSql::DataSource* stopProfile(McoSql::Runtime* runtime);

	virtual SqlEngine* createParallelEngine();

    virtual ~McoSqlEngine();
    DESTROY(McoSqlEngine);
};

/**
 * This class must be used to share a single database connection between multiple threads. 
 * Multithreaded applications can have their own SQL engines for each connected thread (i.e. create
 * their own instance of McoSqlEngine). It is, however, not convenient and not very
 * efficient. In many cases the application does not start threads by itself, rather the thread pool is
 * maintained by some external component and the user code is executed in the context of
 * one of those threads. A typical example is Apache servlets.<p>
 * McoSqlSession allows multiple threads to share a single McoMultithreadedSqlEngine instance.
 * The constructor of this class uses the <code>mco_db_connect</code> function to establish a connection for 
 * a particular thread.
 * The destructor of this class releases the connection by calling the <code>mco_db_disconnect</code> function
 */
class MCOSQL_API McoSqlSession: public McoSqlEngine
{
public:
	McoSqlEngine* const engine;

    /**
     * Establish a connection for the thread via the <code>mco_db_connect</code> function 
     * @param engine shared instance of the McoSqlEngine (it is assumed
     * to be the McoMultithreadedSqlEngine) 
     */
    McoSqlSession(McoSqlEngine* engine);

    /**
     * Release the connection using the <code>mco_db_disconnect</code> function
     */
    ~McoSqlSession();
};

/**
 * Special session used to execute asynchronous commit.
 * All other session should use MCO_BUFFERED_COMMIT policy and call McoAsyncCommit::listen(session) method
 * Instance of this class should be created by create by application in separate thread.
 * Then start() method initiate infinite loop which perform commits whit specified commit interval or
 * when some other session performed commit and notified about it.
 */
class MCOSQL_API McoAsyncCommitSession : public McoSqlSession
{
	McoSql::Mutex*     mutex;
	McoSql::Semaphore* go;
	McoSql::Semaphore* done;
	int        nPendingCommits;   // number of reported buffered commits
	int        nWaitingSessions;  // number of sessiosn blocks because of disk cache overflow
	int        maxCacheUsage;
	bool       waitingForCommits; // asynchronous commit thread is waiting for some work 
	bool       running;
	timer_unit interval;

  public:
	/*
	 * Force thread to process commit
	 */
	void commit();

	/*
	 * Start asynchronous commit loop
	 */
	void start();

	/*
	 * Start asynchronous commit loop
	 */
	void stop();

	/*
	 * Receive commit notifications from this session
	 */
	void listen(McoSqlSession* session);

	/*
	 * Constructor of asyncronous commit session.
	 * @param engine main database engine (shared by all sessions)
	 * @param commitInterval maximal interval between commits (flushinig changes to the disk)
	 * if not notified about performed commit by some other listened session.
	 * @param maxCacheUsagePercent maximal percent of disk cached used by pinned pages: in REDO mode,
	 * modified pages are pinned inn memory until been flushed to the transation log. Too large number
	 * of pending (buffered) commits may cause disk cache overflow.
	 * To prevent it, session perfmed commit will be blocked until cache usage falls down specified threashold.
	 * Zero value disables this check.
	 */
	McoAsyncCommitSession(McoSqlEngine* engine, timer_unit commitInterval = MCO_TM_INFINITE, int maxCacheUsagePercent = 70);
	~McoAsyncCommitSession();
};

class MCOSQL_API QuotaSysAllocator : public McoSql::StdAllocator
{
    mco_size_t const limit;
    mco_size_t total;

    struct Segment { 
        mco_size_t    size;
        mco_process_t tid;
    };

  public:
    size_t allocated();
    size_t quota();

    void* allocate(size_t size);
    void free(void* ptr);
    
    QuotaSysAllocator(size_t quota);
    QuotaSysAllocator(size_t quota, malloc_t malloc, free_t free);
};

typedef McoSqlEngine McoMultithreadedSqlEngine;

#endif

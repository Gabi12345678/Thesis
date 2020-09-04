package com.mcobject.extremedb;

/**
 *  Transaction log writer. 
 */
public class LogWriter
{
    /**
     *  Start each record with new disk page 
     */
    public static final int LF_ALIGNED                = 0x01;
    /**
     *  Append to an existing log file           
     */
    public static final int LF_APPEND                 = 0x02; 
    /**
     *  Cover transaction data with CRC32        
     */
    public static final int LF_CRC                    = 0x04; 
    /**
     *  User call-back function to limit size  (not supported in .Net API)
     */
    public static final int LF_SIZE_CALLBACK          = 0x08; 
    /**
     *  Do disk flush on each record             *
     */
    public static final int LF_SYNC_INSTANTLY         = 0x10; 
    /**
     *  Do disk flush by timer                   
     */
    public static final int LF_SYNC_TIMER             = 0x20; 
    /**
     *  Do disk flush by record count            
     */
    public static final int LF_SYNC_COUNT             = 0x40; 
    /**
     *  Make log applicable for LogReader.LogIterate method
     */
    public static final int LF_ITERABLE               = 0x80; 
    /**
     *  Stop previous log at once                
     */
    public static final int LF_RESTART                = 0x100;
    /**
     *  Use pipe interface instead of file       
     */
    public static final int LF_PIPE                   = 0x200;
    /**
     *  Support for dynamic pipes
     */
    public static final int LF_DYNAMIC_PIPE           = 0x1000;
    
    /**
     *  Transaction log writer constructor
     *  @param con Database connection
     *  @param filePath Path to the written transaction log file
     */
    public LogWriter(Connection con, String filePath)
    {
        this.con = con;
        this.filePath = filePath;
    }

    /**
     *  Logging parameters
     */
    public static class LogParams
    {
        /**
         *  Transaction log flags (bitmask of LF_* constants)
         */
        public int flags;        
        /**
         *  Size of single disk page              
         */
        public int diskPageSize; 
        /**
         *  Maximum count of unflushed records    
         */
        public int flushDepth;    
        /**
         *  Time lag between flushed records (milliseconds)
         */
        public long flushTime;     
    };
    
    /**
     *  Start transaction logging. Database should be created with Database.Mode.TransactionLoggtingSupport flag.
     *  @param logParams logginf parameters
     */
    public void start(LogParams logParams)
    {
        startLogging(con.id, filePath, logParams);
    }

    /**
     *  Stop logging and close log file
     */
    public void stop()
    {
        stopLogging(con.id);
    }
    
    /**
     *  Terminate logging if reader in pipe mode get defunctioned
     */
    public void terminate()
    {
        terminateLogging(con.id);
    }
    
    /**
     *  Mark label on the log at current position (to allow replay of log up to current moment)
     *  @param label label name
     *  @return label id
     */
    public int setLabel(String label)
    {
        return setLogLabel(con.id, label);
    }
    
    /**
     *  Truncate log and start it again (same as stop/start sequence)
     */
    public void truncate()
    {
        truncateLog(con.id);
    }

    /**
     * Save a database snapshot to the specified file together with log truncation.
     * This snaphot can later be loaded by the Database.open method if the corresponding
     * file path is specified in Database.Parameters.databaseSnapshotFilePath.
     * This function guarantee both operations (Connection::saveSnapshot and
     * TransactionLog::truncate) in a single blocking to avoid missed transactions.
     * @param databaseSnapshotFilePath the path to the file where the snapshot will be saved. 
     * (A file is always created or overwritten if it exists).
     * @return true if the snapshot was sucessfully saved and log was truncated, false if the specified file can not be opened or log truncation failed.
     */
    public boolean saveSnapshot(String databaseSnapshotFilePath) {
        return saveDBLog(con.id, databaseSnapshotFilePath);
    }

    /**
     *  Flush log immediately
     */
    public void flush()
    {
        flushLog(con.id);
    }

    /**
     *  Information about current log
     */        
    public static class LogInfo 
    {
        /**
         *  Transaction log flags (bitmask of LF_* constants)
         */
        public int      flags;                     
        /**
         *  Transaction counter at start of log  
         */
        public long     startTransCounter;         
        /**
         *  Current transaction counter          
         */
        public long     lastTransCounter;          
        /**
         *  Count of stored transactions         
         */
        public long     storedTransactions;        
        /**
         *  Size of log in bytes                 
         */
        public long     logSize;
        /**
         *  Used size of pipe in bytes                 
         */
        public long     pipeUsedSize;
        /**
         *  Count of labels it the log           
         */
        public int      labelsCount;               
        /**
         *  Max logging slots used for parallel transaction commits in mvcc. (Total number of slots is max_connections*2)
         */
        public int      maxParallelTrans;          
        /**
         *  Count of times when all of the of logging slots were exceeded 
         */
        public int      exceededTransSlotsCount;                                                         
        /**
         *  Size of largest transaction
         */
        public int      maxTransactionSize;
    }  
      
    /**
     *  Get information about the current log
     */                
    public LogInfo getInfo()
    {
        return getLogWriterInfo(con.id);
    }

    private native void startLogging(long con, String filePath, LogWriter.LogParams logParams);

    private native void stopLogging(long con);

    private native void terminateLogging(long con);

    private native int setLogLabel(long con, String label);

    private native void truncateLog(long con);

    private native boolean saveDBLog(long con, String name);

    private native void flushLog(long con);

    private native LogWriter.LogInfo getLogWriterInfo(long con);

    Connection con;
    String filePath;
}

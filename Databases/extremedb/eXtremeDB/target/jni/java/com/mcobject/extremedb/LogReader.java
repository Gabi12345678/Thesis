package com.mcobject.extremedb;

    
/**
 *  Transaction log reader. 
 */
public class LogReader
{
    /**
     *  Transaction log reader constructor. 
     *  @param con Connection to the pipe database or to the database where log is replayed
     *  @param filePath Path to the transaction log file
     */
    public LogReader(Connection con, String filePath)
    {
        this.con = con;
        this.filePath = filePath;
    }
    
    /**
     *  Apply transactions from the log to the database up to the specified label id
     *  @param labelId Label ID
     */
    public void apply(int labelId)
    {
        applyLog(con.id, filePath, labelId);
    }
    
    /**
     *  Apply transactions from the log to the database up to the end of log 
     */
    public void apply()
    {
        applyLog(con.id, filePath, -1);
    }

    /**
     *  Information gathered from log file
     */
    public static class LogInfo
    {
        /**
         *  Transaction counter at start of log  
         */
        public long    startTransCounter;        
        /**
         *  Transaction counter at end of log    
         */
        public long    stopTransCounter;         
        /**
         *  Count of stored transactions         
         */
        public long    storedTransactions;       
        /**
         *  Size of single disk page             
         */
        public int     diskPageSize;             
        /**
         *  Size of in-memory database page      
         */
        public int     pageSize;                 
        /**
         *  Size of log in bytes                 
         */
        public long    logSize;                  
        /**
         *  List of labels it the log           
         */
        public String[] labels;                   
        /**
         *  If log covered with CRC32            
         */
        public boolean crc;                      
        /**
         *  If log written with alignment        
         */
        public boolean aligned;                  
        /**
         *  If log compatible with current dict. 
         */
        public boolean dictionaryCompat;         
        /**
         *  If log suitable to be applied        
         */
        public boolean transactionApplyCompat;   
        /**
         *  If log suitable to be appended       
         */
        public boolean transactionAppendCompat;  
        /**
         *  If log finished correctly            
         */
        public boolean notCorrupted;             
        /**
         *  Log is applicable for synchronisation with external database (saved with flag MCO_TRANSLOG_ITERABLE)
         */
        public boolean iteratable;               
    }
    
    /**
     *  Get information about the log file
     */
    public LogInfo getInfo() 
    {
        return getLogReaderInfo(con.id, filePath);
    }
    
    /**
     *  Interface for iterator through transaction log objects
     */
    public interface LogIterator
    {
        /**       
         *  Callback on object creation
         *  @param obj Created object
         */
        void create(Object obj);

        /**       
         *  Callback on object update
         *  @param obj Updated object
         */
        void update(Object obj);

        /**       
         *  Callback on object delete
         *  @param obj Deleted object
         */
        void delete(Object obj);

        /**       
         *  Callback on delete all class instances
         *  @param type Type of the deleted objects            
         */
        void deleteAll(Class type);
        
        /**
         *  Callback on end of transaction. Is called if option RT_OPTION_MARK_LAST_OBJ set
         */
        void endOfTransaction();
    }
    
    /**       
     *  Iterate through transaction log file entries
     *  @param iterator Iterator class which method will be invoked for all log entries
     *  @param memSize Size of temporary database used to fetch objects (should be large enough to fit the largest objects)
     */        
    public void iterateFile(LogIterator iterator, long memSize, long hint) 
    {
        Database db = con.db;
        iterateLog(0, filePath, db.structsList.toArray(), db.classes, db.hasInmemDistribClasses, db.hasDiskDistribClasses, 
                                    db.totalFields, db.totalIndices, db.totalKeys, db.events.size(), db.config, iterator, memSize, hint, null);
    }

    /**       
     *  Iterate through transaction log entries fetched from pipe
     *  @param iterator Iterator class which method will be invoked for all log entries
     *  @param memSize Size of temporary database used to fetch objects (should be large enough to fit the largest objects)
     */        
    public void iteratePipe(LogIterator iterator, long memSize, long hint) 
    {
        Database db = con.db;
        iterateLog(con.id, filePath, db.structsList.toArray(), db.classes, db.hasInmemDistribClasses, db.hasDiskDistribClasses, 
                                        db.totalFields, db.totalIndices, db.totalKeys, db.events.size(), db.config, iterator, memSize, hint, null);
    }

    public void iteratePipe(LogIterator iterator, long memSize, long hint, Database.Device pipe) 
    {
        Database db = con.db;
        iterateLog(con.id, filePath, db.structsList.toArray(), db.classes, db.hasInmemDistribClasses, db.hasDiskDistribClasses, 
                                        db.totalFields, db.totalIndices, db.totalKeys, db.events.size(), db.config, iterator, memSize, hint, pipe);
    }

    /**
     *  Stop method iteratePipe specified by connection of current LogReader
     */
    public void stopPipeIteration()
    {
        stopIteration(con.id);
    }

    Connection con;
    String filePath;

    /**       
     *  Internal method, should not be called by application
     */        
    public int invokeCallback(LogIterator iterator, int operation, int classId, long handle)
    {
        Database db = con.db;
        try {
            if (handle == 0) {
                iterator.endOfTransaction();
            } else {
                switch (operation) 
                {
                case 1: // object created 
                    iterator.create(db.loadObject(handle, db.classes[classId-1]));
                    break;
                case 2: // object deleted 
                    iterator.delete(db.loadObject(handle, db.classes[classId-1]));
                    break;
                case 3: // object created and deleted: do nothing?
                    break;
                case 0: // object updated?
                    iterator.update(db.loadObject(handle, db.classes[classId-1]));
                    break;
                case 4: // delete all
                    iterator.deleteAll(db.classes[classId-1].cls);
                    break;
                default:
                    break;
                }
            }
        } catch (DatabaseError de) {
            return de.errorCode;
        }
        return 0;
    }
    private native void applyLog(long con, String filePath, int labelId);

    private native LogReader.LogInfo getLogReaderInfo(long con, String filePath);

    private native void iterateLog(long con, String filePath, Object[] structs,
                                   Database.ClassDescriptor[] classes, boolean hasInmemDistribClasses, boolean hasDiskDistribClasses, 
                                   int totalFields, int totalIndices, int totalKeys, int totalEvents, int config, 
                                   LogIterator iterator, long memSize, long hint, Database.Device pipe);

    private native void stopIteration(long con);
}

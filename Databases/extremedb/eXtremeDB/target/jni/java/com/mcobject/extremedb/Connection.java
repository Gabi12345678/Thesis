package com.mcobject.extremedb;

/**
 * Database connection.
 * Each thread working with the database should have its own database connection.
 */
public class Connection implements java.io.Closeable
{
    /**
     * Constructor of the database connection.
     * @param db opened database
     */
    public Connection(Database db) { 
        this.db = db;
        id = connect(db.name, null);
    }
    
    /**
     * Constructor of the database connection with context.
     * @param db		opened database
     * @param context	
     */
    public Connection(Database db, byte [] context) { 
        this.db = db;
        id = connect(db.name, context);
    }
    
    protected Connection(Connection con) { 
        db = con.db;
        id = con.id;
    }

    /**
     * Start a transaction with the default priority and isolation level.
     * (Nested transactions are supported.)
     */
    public void startTransaction(Database.TransactionType type) {
        startTransaction(type, Database.TransactionPriority.Foreground, Database.IsolationLevel.Default);
    }

    /**
     * Start a transaction with the specified priority and isolation level.
     * (Nested transactions are supported.)
     */
    public void startTransaction(Database.TransactionType type, Database.TransactionPriority pri, Database.IsolationLevel level) {
        startTransaction(id, type.ordinal(), pri.ordinal(), level.ordinal());
    }

    /**
     * Commit a transaction.
     * @return false if there is an MVCC conflict.
     */
    public boolean commitTransaction() {
        return commitTransaction(id);
    }

    /**
     * Complete one of two possible transaction commit phases.
     * @param phase 1 or 2.
     * @return false in case of MVCC conflict.
     */
    public boolean commitTransaction(int phase) {
        if (phase != 1 && phase != 2) { 
            throw new IllegalArgumentException("Invalid phase: " + phase);
        }
        return commitTransactionPhase(id, phase);
    }

    /**
     * Set the transaction commit policy for this connection.
     */
    public void setCommitPolicy(Database.CommitPolicy policy) {
        setCommitPolicy(id, policy.ordinal());
    }

    /**
     * Flush to the disk all changes made by committed transactions.
     * This method can be used to flush changes when the MCO_COMMIT_BUFFERED transaction policy is used.
     */
    public void diskFlush() {
        diskFlush(id);
    }

    /**
     * Rollback a transaction. Note that any nested transactions are aborted.
     */
    public void rollbackTransaction() {
        rollbackTransaction(id);
    }

    /**
     * Checkpoint current transaction: insert all object updated by current transaction in indexes
     * @return false if there is an MVCC conflict.
     */
    public boolean checkpointTransaction() { 
        return checkpointTransaction(id);
    }

    /**
     * Insert a new object in the database.
     * @param obj an object of one of the classes enumerated in the Database.Parameters.classes list.
     * @return the AUTOID of the created object if the autoid attribute is set in the Persistent annotation for this class.
     * or 0 if class has not associated AUTOID index 
     */
    public long insert(Object obj) {
        Database.ClassDescriptor cls = db.getClassDescriptor(obj.getClass());
        long h = createObject(id, cls.classNo, cls.initSize);
        db.storeObject(h, cls, obj);        
        long autoid = cls.autoIdIndexNo >= 0 ? db.getAutoId(h) : 0;
        Database.freeHandle(h);
        return autoid;
    }

    /**
     * Return the number of instances of this class stored in the database.
     */
    public long count(Class cls) {
        return count(id, db.getClassDescriptor(cls).classNo);
    }

    /**
     * Remove all instances of the specified class. (Class inheritance is not considered.)
     * @param cls the class from which the instances will be removed.
     */
    public void removeAll(Class cls) {
        removeAll(id, db.getClassDescriptor(cls).classNo);
    }

    /**
     * Close this connection.
     */
    public void disconnect() {
        if (id != 0) { 
            disconnect(id);
            id = 0;
        }
    }

    /**
     * Close this connection. The same as disconnect.
     */
    public void close() {
        disconnect();
    }

    /**
     * Save a database snapshot to the specified file.  This snapshot can later be loaded by the Database.open method
     * if the corresponding file path is specified in Database.Parameters.databaseSnapshotFilePath.
     * @param databaseSnapshotFilePath the path to the file where the snapshot will be saved. 
     * (A file is always created or overwritten if it exists).
     * @return true if the snapshot was successfully saved, false if the specified file can not be opened.
     */
    public boolean saveSnapshot(String databaseSnapshotFilePath) {
        return save(id, databaseSnapshotFilePath);
    }

    /**
     * Save a database metadata to the specified file.  This metadata can later be loaded as a xsql
     * configuration file.
     * @param databaseMetasataFilePath the path to the file where the snapshot will be saved. 
     * (A file is always created or overwritten if it exists).
     */
    public boolean saveMetadata(String databaseMetadataFilePath, boolean saveDefaults) {
        return saveMetadata(id, databaseMetadataFilePath, saveDefaults);
    }

    public boolean saveMetadata(String databaseMetadataFilePath) {
        return saveMetadata(id, databaseMetadataFilePath, true);
    }

    /**
     * Save a database dictionary to the specified file.
     * @param databaseDictionaryFilePath the path to the file where the dictionary will be saved. 
     * (A file is always created or overwritten if it exists).
     */
    public boolean saveDictionary(String databaseDictionaryFilePath) {
        return saveDictionary(id, databaseDictionaryFilePath);
    }
    

    /**
     * Save table content to the specified file.  This snapshot can later be loaded by the Database.open or Connection.loadClass methods
     * @param filePath the path to the file where the table content will be saved. 
     * (A file is always created or overwritten if it exists).
     * @param cls the class which instances will be saved.
     * @return true if the snapshot was successfully saved, false if the specified file can not be opened.
     */
    public boolean saveClass(String filePath, Class cls) {
        return saveClass(id, filePath, db.getClassDescriptor(cls).classNo);
    }

    /**
     * Load table content from the file previously created by Connection.saveClass or Connection.saveSnapshot methods
     * @param filePath the path to the file from where the table content will be loaded. 
     * @param cls the class which instances will be loaded. If image file was created by Connection.saveClass this parameter can be null
     * @param doMerge if False the content of the table is cleared before loading from image file. If True the loaded objects are "added" to the table
     * @return true if the objects were successfully loaded, false if the specified file can not be opened.
     */
    public boolean loadClass(String filePath, Class cls, boolean doMerge) {
        return loadClass(id, filePath, (cls != null) ? db.getClassDescriptor(cls).classNo : 0, doMerge);
    }

    public boolean loadClass(String filePath) {
        return loadClass(id, filePath, 0, false);
    }

    /**
     * Waiting signaling of particular asycnhronous event.
     * This method should be called inseparate thread and is blocked until event is released
     * @param name event name
     */       
    public void waitEvent(String name) {
        waitEvent(id, db.findEvent(name));
    }

    /**
     * Signaling particular asycnhronous event.
     * @param name event name
     */
    public void releaseEvent(String name) {
        releaseEvent(id, db.findEvent(name));
    }
        
    /**
     * Signaling all asycnhronous events.
     */                 
    public void releaseAllEvents() {
        releaseAllEvents(id);
    }

    /**
     * Statistics function returns the total number of free pages
     * @return number of free pages
     */
    public int getFreePages() {
		return getFreePages(id);
    }
        
    /**
     * Statistics function returns the total number of available (originally allocated) pages. 
     * @return  number of available (originally allocated) pages
     */
    public int getTotalPages() {
    	return getTotalPages(id);
    }
    
    /**
     * Statistics function returns the current pagesize for a database connection
     * @return database page size
     */
    public short getDbPageSize() {
		return getDbPageSize(id);
    	
    }
    
    /**
     * Statistics function allows collecting of statistics on the database at runtime.
     * @param classCode a class code
     * @return statistics data for the class
     */
    public Statistic.ClassStat getClassStat(int classCode) {
		return getClassStat(id, classCode);    	
    }
    
    /**
     * Statistics property returns the number of indexes in the database.  It must be called
     * in the context of a read-only transaction and is often used in conjunction with the
     * getIndexStat() to obtain index statistics at runtime
     * @return number of indexes in the database
     */
    public short getIndexStatCount()
    {
    	return getIndexStatCount(id);
    }

    /**
     * Statistics function obtains index statistics at runtime
     * @param index zero-based index number. Total count of indexes can be obtained by IndexStatCount property
     * @return index statistics
     */
    public Statistic.IndexStat getIndexStat(short index)
    {
    	Object o1 = com.mcobject.extremedb.Statistic.IndexStat.class.getClassLoader().getResource("com.mcobject.extremedb.Statistic.IndexStat.class");
    	Object o2 = com.mcobject.extremedb.Statistic.IndexStat.class.getClassLoader().getResource("com.mcobject.extremedb.Statistic$IndexStat.class");
    	return getIndexStat(id, index);
    }
    
     /**
      *  function returns information about the current state of the database
      *  and log file: the size of the log file in bytes, the size of the database file in bytes
      *  and the amount of space that is actually used in the database file.
     * @return current state of the database
     */
    public Statistic.DiskInfo getDiskInfo() {
    	return getDiskInfo(id);		
	}


    
    public void createBackup(String fileName, String label, Database.BackupType type) {
        createBackup(id, fileName, label, type.ordinal(), 0, null);
    }

    public void createBackup(String fileName, String label, Database.BackupType type, int compressionLevel, String cipher) {
        createBackup(id, fileName, label, type.ordinal(), compressionLevel, cipher);
    }

    public void restoreBackup(String fileName, String label) {
        restoreBackup(id, fileName, label, null);
    }

    public void restoreBackup(String fileName, String label, String cipher) {
        restoreBackup(id, fileName, label, cipher);
    }

    public Database.BackupInfo [] listBackup(String fileName) {
        return listBackup(id, fileName);
    }

    protected void finalize() { 
        disconnect();
    }

    private native void startTransaction(long con, int type, int pri, int level);
    private native boolean commitTransactionPhase(long con, int phase);
    private native boolean commitTransaction(long con);
    private native boolean checkpointTransaction(long con);
    private native void setCommitPolicy(long con, int policy);
    private native void diskFlush(long con);
    private native void rollbackTransaction(long con);
    private native void removeAll(long con, int classId);
    private native void disconnect(long con);
    private native long createObject(long con, int classId, int initSize);
    private native long connect(String name, byte [] context);
    private native boolean save(long con, String name);
    private native boolean saveMetadata(long con, String name, boolean saveDefaults);
    private native boolean saveDictionary(long con, String name);
    private native boolean saveClass(long con, String name, int classId);
    private native boolean loadClass(long con, String name, int classId, boolean doMerge);
    private native long count(long con, int classId);
    private native void waitEvent(long con, int id);
    private native void releaseEvent(long con, int id);
    private native void releaseAllEvents(long con);
    private native int getFreePages(long con);
    private native int getTotalPages(long con);
    private native short getDbPageSize(long con);
    private native Statistic.ClassStat getClassStat(long con, int class_code); 
    private native short getIndexStatCount(long con);
    private native Statistic.IndexStat getIndexStat(long con, int idx_num);
    private native Statistic.DiskInfo getDiskInfo(long con);
    private native void createBackup(long con, String fileName, String label, int type, int compressionLevel, String cipher);
    private native void restoreBackup(long con, String fileName, String label, String cipher);
    private native Database.BackupInfo [] listBackup(long con, String fileName);

    Database db;
    long id;
}

package com.mcobject.extremedb;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 *  Database recovery API. 
 */
public class Sniffer
{
    /**
     *  Sniffer policy. Inspect all active connections.
     */
    public static final int MCO_SNIFFER_INSPECT_ACTIVE_CONNECTIONS      = 0x00;

    /**
     *  Sniffer policy. Inspect all connections with active transactions.
     */
    public static final int MCO_SNIFFER_INSPECT_ACTIVE_TRANSACTIONS     = 0x01;

    /**
     *  Sniffer policy. Inspect connections with active transactions whose
     *  transaction number has not changed since the previous invocation of mco_db_sniffer()
     */
    public static final int MCO_SNIFFER_INSPECT_HANGED_TRANSACTIONS     = 0x02;
    
    /**
     *  Sniffer constructor
     *  @param con Database connection
     */
    public Sniffer(Connection con)
    {
        this.con = con;
    }

    /**
     *  Interface for traversing through database connections
     */
    public interface ConnIterator
    {
        /**       
         *  Callback on each of existing connection
         *  @param procID ID of a process established this connection
         *  @param transNo current transaction number
         */
        boolean isConnectionAlive(byte[] context, long transNo);
    }

    /**       
     *  Iterate through database connections
     *  @param iterator Iterator class which method will be invoked for all connections
     *  @param policy one of connection traversal strategies MCO_SNIFFER_INSPECT_ACTIVE_CONNECTIONS,
     *  MCO_SNIFFER_INSPECT_ACTIVE_TRANSACTIONS or MCO_SNIFFER_INSPECT_HANGED_TRANSACTIONS
     */        
    public void iterateConnections(ConnIterator iterator, int policy)
    {
        iterateConnectionsInt(con.id, iterator, policy);
    }

    /**       
     *  Check if a process with given ID exists
     *  @param procID identifier of a process
     */        
    public static boolean isProcessAlive(int procID)
    {
        return isProcessAliveInt(procID);
    }

    /**       
     *  Get identifier of current process
     */        
    public static int getCurrentProcessID()
    {
        return getCurrentProcessIDInt();
    }

    /**
     *  Convert process identifier to a bytes buffer according to current platform properties
     *  as bytes order and size of mco_process_t data type.
     *  Note that originally connection context is a set of bytes of size specified by 
     *  Database.Parameters.connectionContextSize value. There is an agreement that Connection
     *  context is a value of type mco_process_t which holds a process identifier. Different
     *  components like SQL engine and xsql utility adhere to this agreement.
     */
    public byte[] processIDtoBytes(int procID)
    {
        return processIDtoBytes(con.db, procID);
    }

    public static byte[] processIDtoBytes(Connection con, int procID)
    {
        return processIDtoBytes(con.db, procID);
    }

    public static byte[] processIDtoBytes(Database db, int procID)
    {
        Database.RuntimeInfo rtInfo = db.getRunTimeInfo();
        ByteBuffer buff = ByteBuffer.allocate(rtInfo.mco_process_t).order(ByteOrder.nativeOrder());
        if (rtInfo.mco_process_t == 4) {
            buff.putInt(procID);
        } else {
            buff.putLong(procID);
        }
        return buff.array();
    }

    /**
     *  Convert bytes buffer to a process identifier according to current platform properties
     *  as bytes order and size of mco_process_t data type.
     */
    public int bytesToProcessID(byte[] connectionContext)
    {
        return bytesToProcessID(con.db, connectionContext);
    }

    public static int bytesToProcessID(Connection con, byte[] connectionContext)
    {
        return bytesToProcessID(con.db, connectionContext);
    }

    public static int bytesToProcessID(Database db, byte[] connectionContext)
    {
        Database.RuntimeInfo rtInfo = db.getRunTimeInfo();
        if (rtInfo.mco_process_t != connectionContext.length) {
            throw new IllegalArgumentException("Invalid context size. Expected " + rtInfo.mco_process_t
                                                + ", provided " + connectionContext.length);
        }
        ByteBuffer buff = ByteBuffer.allocate(rtInfo.mco_process_t).order(ByteOrder.nativeOrder());
        buff.put(connectionContext);
        buff.flip();
        if (rtInfo.mco_process_t == 4) {
            return buff.getInt();
        } else {
            return (int)buff.getLong();
        }
    }

    /**
     *  Size of mco_process_t data type which usually represents process ID which is a
     *  sniffer connection context in turn. Put this size to Database.Parameters.connectionContextSize
     *  field if process ID supposed to be a connection context. This is common behavior of SQL
     *  engine and xsql utility.
     */
    public int getProcessIDSize()
    {
        return getProcessIDSize(con.db);
    }

    public static int getProcessIDSize(Connection con)
    {
        return getProcessIDSize(con.db);
    }

    public static int getProcessIDSize(Database db)
    {
        Database.RuntimeInfo rtInfo = db.getRunTimeInfo();
        return rtInfo.mco_process_t;
    }

    /**       
     *  Internal method, should not be called by application
     */        
    public boolean invokeCallback(ConnIterator iterator, byte[] context, long transNo)
    {
        return iterator.isConnectionAlive(context, transNo);
    }

    Connection con;

    private native void iterateConnectionsInt(long con, ConnIterator iterator, int policy);
    private static native boolean isProcessAliveInt(int procID);
    private static native int getCurrentProcessIDInt();
}

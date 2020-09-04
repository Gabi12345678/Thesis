package com.mcobject.extremedb;

import java.util.*;

/**
 * Class used to iterate through result of SQL query
 */
public class SqlServer
{
    public static class OpenParameters
    {
        public SqlLocalConnection conn;
        public int port;
        public int bufsize;
        public int nThreads;
        public int listenQueueSize;
        public boolean localDomain;
        public Database.SSLParameters sslParameters;
        public boolean authenticationRequired;
        public int interruptTimeout;
        public int compressionLevel;
        public String netInterface;

        public OpenParameters(SqlLocalConnection conn, int port) {
            this.conn = conn;
            this.port = port;
            bufsize = 65536;
            nThreads = 4;
            listenQueueSize = 5;
            localDomain = false;
            sslParameters = null;
            authenticationRequired = false;
            interruptTimeout = 100;
            compressionLevel = 0;
            netInterface = null;
        }
    }
    /**
     * SQL server constructor
     * 
     */
    public SqlServer(SqlLocalConnection conn, int port)
    {
    	engine = conn.getEngine();
    	OpenParameters op = new OpenParameters(conn, port);
    	init(op);
    }

    public SqlServer(SqlLocalConnection conn, int port, int bufsize)
    {
    	engine = conn.getEngine();
    	OpenParameters op = new OpenParameters(conn, port);
    	op.bufsize = bufsize;
        init(op);
    }

    public SqlServer(OpenParameters op)
    {
        engine = op.conn.getEngine();
        init(op);
    }

    private void init(SqlServer.OpenParameters op) {
        server = serverInit(engine, op.port, op.bufsize, op.nThreads, op.listenQueueSize, op.localDomain,
                op.sslParameters, op.authenticationRequired, op.interruptTimeout, op.netInterface, op.compressionLevel);
    }

    /**
     * start SQL server
     * 
     */
    public void start()
    {
    	serverStart(server);
    }

    /**
     * stop SQL server
     * 
     */
    public void stop()
    {
    	serverStop(server);
    }
  
    public enum SessionState {
        Wait, Active, Done, Canceled;
        private static final SessionState values[] = values();
        public static final SessionState from_value(int v) { return values[v];}
    };
    
    public static class SessionInfo
    {
        public String       peerAddr;
        public int          peerPort;
        public SessionState peerState;

        public SessionInfo(String addr, int port, SessionState state) {
            peerAddr = addr;
            peerPort = port;
            peerState = state;
        }
    }

    /**
     * Get SQL server sessions info
     * 
     */
    public SessionInfo[] getSessionsInfo()
    {
    	return serverGetSessionsInfo(server);
    }
      
    private long engine;
    private long server;
    
    private native long serverInit(long con, int port, int bufsize, int nThreads, int listenQueueSize,
                                   boolean localDomain, Database.SSLParameters sslParameters,
                                   boolean authenticationRequired, int interruptTimeout, String netInterface, int compressionLevel);

    private native void serverStart(long server);
    private native void serverStop(long server);
       
    private native SessionInfo[] serverGetSessionsInfo(long server);
}

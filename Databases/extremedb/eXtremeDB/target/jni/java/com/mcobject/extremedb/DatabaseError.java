package com.mcobject.extremedb;

/**
 * An exception thrown by eXtremeDB.
 */
public class DatabaseError extends RuntimeException
{
    /**
     * The eXtremeDB error code.
     */
    public final int errorCode;

    /**
     * The eXtremeDB error code description.
     */
    public final String errorCodeDescription;
    
    /**
     * The Constructor of the exception.
     * @param err the eXtremeDB error code.
     */
    public DatabaseError(int err, String description) { 
        errorCode = err;
        errorCodeDescription = description;
    }

    public String getMessage() {
        return "eXtremeDB runtime error " + errorCode + ": " + errorCodeDescription;
    }
}

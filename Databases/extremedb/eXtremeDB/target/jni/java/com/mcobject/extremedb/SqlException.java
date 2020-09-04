package com.mcobject.extremedb;

/**
 * An exception thrown by eXtremeDB SQL engine.
 */
public class SqlException extends RuntimeException
{
    /**
     * The eXtremeDB error code.
     */
    public final String message;
    
    /**
     * The Constructor of the exception.
     * @param msg error message
     */
    public SqlException(String msg) { 
        message = msg;
    }

    public String getMessage() {
        return message;
    }
}
package com.mcobject.extremedb;

/**
 * The interface that must be implemented by classes that need to initialize a transient state.
 */
public interface Initializable 
{
    /**
     * This method will be called by eXtremeDB when the object is loaded from the database.
     */
    public void initialize();
}
package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * The description of eXtremeDB event.
 */
@Target({ElementType.TYPE, ElementType.FIELD})
@Retention(RetentionPolicy.RUNTIME)
public @interface Event
{
    /**
     * Name of the event
     */
    String name();

    /**
     * Type of event
     */
    Database.EventType type();
}
    

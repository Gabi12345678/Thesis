package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * List of events associated with this class. Java doesn't allow  duplicate annotations so if you have to define more than one 
 * event, then you should wrap the in Trigger annotation:
 * <pre>
 *     @Trigger(@Event(name="OnMyClassInsert", type=Database.EventType.OnNew),
 *              @Event(name="OnMyClassDelete", type=Database.EventType.OnDelte))
 *     class MyClass {
 *         ...
 *     }
 * </pre>
 */
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
public @interface Trigger
{
    Event[] value();
}
    

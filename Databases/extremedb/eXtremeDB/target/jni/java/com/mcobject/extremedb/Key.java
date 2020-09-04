package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * The description of a component od compound (multifield) index. 
 */
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
public @interface Key
{
    /**
     * Name of the indexed field
     */
    String value();

    /**
     * The objects in this index are stored in descending order.
     */
    boolean descending() default false;
}

package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * Store a array field in the database as a sequence.
 */
@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
public @interface Sequence {
    public enum Order { 
        None, 
        Ascending,
        Descending
    }    
    Order order() default Order.None;
    
    public enum Type {
        UInt1,
        UInt2,
        UInt4,
        UInt8,
        Int1,
        Int2,
        Int4,
        Int8,
        Float,
        Double,
        Char,
        DateTime
    }
    Type type();

    /**
     * Size of sequence element: needed only for sequence of char
     */
    int elemSize() default 0;
};

package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * Create an index for this field.
 */
@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
public @interface Indexable {
    /**
     * The index is unique.
     */
    boolean unique() default false;

    /**
     * The index optimized for large number of duplicates
     */
    boolean thick() default false;

    /**
     * The objects in this index are stored in descending order.
     */
    boolean descending() default false;

    /**
     * The initial size of a hash table.
     */
    int     initSize() default 1000;

    /**
     * The index type (Hashtable, BTree, RTree, Particia)
     */
    Database.IndexType type() default Database.IndexType.BTree;
};

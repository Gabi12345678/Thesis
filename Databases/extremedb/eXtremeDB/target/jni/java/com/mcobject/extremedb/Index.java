package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * The description of a compound (multifield) index. Although this class can be used to create
 * indices for a single field, it is more invenient to do it using @Indexable annotation to the particular field.
 */
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
public @interface Index 
{
    /**
     * Name of the index
     */
    String name();

    /**
     * The index is unique.
     */
    boolean unique() default false;

    /**
     * The index optimized for large number of duplicates
     */
    boolean thick() default false;

    /**
     * The initial size of a hash table.
     */
    int     initSize() default 1000;

    /**
     * The index type (Hashtable, BTree, RTree, Particia)
     */
    Database.IndexType type() default Database.IndexType.BTree;

    /**
     * List of index keys
     */
    Key[] keys();
}
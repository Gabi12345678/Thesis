package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * The description of a class that is stored in an eXremeDB database.
 */
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
public @interface Persistent {
    /**
     * This class is stored on disk.
     */
    boolean disk() default false;
    /**
     * This class is in-memory only.
     */
    boolean inmemory() default false;
    
    /**
     * Create a list index for this class.
     */
    boolean list() default false;
    
    /**
     * Assign an AUTOID to the instances of this class.
     */
    boolean autoid() default false;

    /**
     * Use the compact layout for this class (the size of an instance of this class can not be larger than 64Kb).
     */
    boolean compact() default false;
    /**
     * Use the standard layout for this class (the size of instance of this class can be larger than 64Kb).
     */
    boolean large() default false;
    /**
     * Not replicated in HA and Cluster environment
     */
    boolean local() default false;
    /**
     * Manual replication (scatter/gather) in cluster environment
     */
    boolean distributed() default false;
    /**
     * Enforce atomic semantic for this class
     */
    boolean atomic() default false;
    /**
     * Do not enforce atomic semantic for this class
     */
    boolean nonatomic() default false;
};

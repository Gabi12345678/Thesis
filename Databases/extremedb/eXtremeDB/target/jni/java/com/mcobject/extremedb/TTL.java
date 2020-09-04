package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * The description of a TTL policy.
 */
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
public @interface TTL {
    /**
     * Object count threshold.
     */
    long maxCount() default 0;

    /**
     * Object age threshold.
     */
    long maxTime() default 0;
};

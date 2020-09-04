package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * The specification of array dimensions (if not set, then arrays will be stored in the eXtremeDB as vectors).
 */
@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
public @interface Dimension {
    int     value() default 0;
};

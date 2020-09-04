package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * The specification of string field encoding.
 */
@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
public @interface Encoding {
    String value();
};

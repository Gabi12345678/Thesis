package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * Store a byte array field in the database as a BLOB.
 */
@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
public @interface Blob {
};
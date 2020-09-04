package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * The annotation for an optional field, ie. a field that can be null. (This corresponds to eXtremeDB 'optional' DDL declaration).
 */
@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
public @interface OptionalStruct {
};

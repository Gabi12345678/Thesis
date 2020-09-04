package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * The annotation for an nullable field, ie. a field that can be null. (This corresponds to eXtremeDB 'nullable' DDL declaration).
 */
@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
public @interface Nullable {
};

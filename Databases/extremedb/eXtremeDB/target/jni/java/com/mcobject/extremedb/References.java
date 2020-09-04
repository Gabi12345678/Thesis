package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * The specification of references constraint
 */
@Target(ElementType.FIELD)
@Retention(RetentionPolicy.RUNTIME)
public @interface References
{
    Class value();
}
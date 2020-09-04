package com.mcobject.extremedb;

import java.lang.annotation.*;

/**
 * List of class indices. Java doesn't allow  duplicate annotations so if you have to define more than one index, 
 * then you should wrap the in Indexes annotation:
 * <pre>
 *     @Indexes(@Index(name="index1", keys={"a", "b"}),
 *              @Index(name="index2", keys={"x", "&lt;y", z}))
 *     class MyClass {
 *         ...
 *     }
 * </pre>
 */
@Target(ElementType.TYPE)
@Retention(RetentionPolicy.RUNTIME)
public @interface Indexes
{
    Index[] value();
}
    
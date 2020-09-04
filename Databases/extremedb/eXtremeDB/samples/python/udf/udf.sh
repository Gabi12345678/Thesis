#!/bin/bash

MCO_ROOT=../../..
export LD_LIBRARY_PATH=$MCO_ROOT/target/bin.so
export PYTHONPATH=$MCO_ROOT/target/bin/python

export LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libpython2.7.so

ls $LD_LIBRARY_PATH/libmcopython*

$MCO_ROOT/target/bin/xsql -i -f udf.sql -debug


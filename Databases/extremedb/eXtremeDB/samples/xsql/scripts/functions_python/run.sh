#!/usr/bin/env bash

export MCO_ROOT=`(cd ../../../.. && pwd)`
export PYTHONPATH=$MCO_ROOT/target/bin/python

$MCO_ROOT/target/bin/xsql -c xsql.cfg -b -f $1.sql

#!/usr/bin/env bash

export MCO_ROOT=`(cd ../../../.. && pwd)`

$MCO_ROOT/target/bin/xsql -c xsql.cfg -b -f $1.sql

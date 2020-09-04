#!/bin/bash

# Run XSQL with create table and UDF, import some data and perform query

../../../../../../target/bin/xsql -c xsql.cfg -f create.sql -f import.sql -f query.sql -b

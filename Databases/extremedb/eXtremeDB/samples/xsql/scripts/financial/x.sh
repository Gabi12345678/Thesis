#!/usr/bin/env bash
../../../../target/bin/xsql -b -size 100m -f IBM-q1-2013.sql -f x$1.sql

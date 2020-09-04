#!/usr/bin/env bash
../../../../target/bin/xsql -b -size 100m -f simple.sql -f f$1.sql

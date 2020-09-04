#!/usr/bin/env bash
../../../../target/bin/xsql -b -size 100m -f quote.sql -f insert.sql -f g$1.sql

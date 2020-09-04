#!/usr/bin/env bash

MCO_ROOT=../../..
cd `dirname $0`

LD_LIBRARY_PATH=$MCO_ROOT/target/bin.so/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
if [ "$(uname)" == "Darwin" ]; then
    MCO_LIBRARY_PATH=$MCO_ROOT/target/bin.so
    export MCO_LIBRARY_PATH
fi
java -ea -classpath $MCO_ROOT/target/bin/extremedb.jar:. `basename $0 .sh` $1 $2 $3 $4

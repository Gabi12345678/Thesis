#!/usr/bin/env bash

MCO_ROOT=../../../..
cd `dirname $0`

LD_LIBRARY_PATH=$MCO_ROOT/target/bin.so/:$LD_LIBRARY_PATH
export LD_LIBRARY_PATH
if [ "$(uname)" == "Darwin" ]; then
    MCO_LIBRARY_PATH=$MCO_ROOT/target/bin.so
    export MCO_LIBRARY_PATH
fi

PYTHONPATH=$MCO_ROOT/target/bin/python/
export PYTHONPATH

if [[ -z "$PYTHONBIN" ]]
then
	export PYTHONBIN=python
fi

echo "PYTHONPATH=$PYTHONPATH"

$PYTHONBIN sqlperf.py

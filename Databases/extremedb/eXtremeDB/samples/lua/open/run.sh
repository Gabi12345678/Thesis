#!/bin/bash

export LUA_PATH=../../../target/lua/exdb.lua
export LD_LIBRARY_PATH=../../../target/bin.so

../../../target/bin/luajit open.lua

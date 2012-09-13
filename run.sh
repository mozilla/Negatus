#!/bin/sh

rm agent
make -f Makefile.droid
$ADB_PATH shell "rm /data/local/agent"
$ADB_PATH shell "rm -rf /data/local/tests"
$ADB_PATH shell "rm -rf /data/local/tmp*"
$ADB_PATH push agent /data/local
$ADB_PATH push libs/libnspr4.so /data/local
$ADB_PATH push libs/libplc4.so /data/local
$ADB_PATH push libs/libplds4.so /data/local
$ADB_PATH shell 'export NSPR_LOG_MODULES="NegatusLOG:5, timestamp" \
  && LD_LIBRARY_PATH=/data/local /data/local/agent'

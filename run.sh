#!/bin/sh

rm agent
make -f Makefile.droid
$ADB shell "rm /data/local/agent"
$ADB shell "rm -rf /data/local/tests"
$ADB shell "rm -rf /data/local/tmp*"
$ADB push agent /data/local
$ADB push libs/libnspr4.so /data/local
$ADB push libs/libplc4.so /data/local
$ADB push libs/libplds4.so /data/local
$ADB shell 'export NSPR_LOG_MODULES="NegatusLOG:5, timestamp" \
  && LD_LIBRARY_PATH=/data/local /data/local/agent'

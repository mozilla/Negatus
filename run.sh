#!/bin/sh

$ADB shell "rm /data/local/agent"
$ADB shell "rm -rf /data/local/tests"
$ADB shell "rm -rf /data/local/tmp*"
$ADB push agent /data/local
$ADB push nspr/libnspr4.so /data/local
$ADB push nspr/libplc4.so /data/local
$ADB push nspr/libplds4.so /data/local
$ADB shell "cd /data/local; LD_LIBRARY_PATH=. ./agent"

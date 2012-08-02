#!/bin/sh

rm agent
make agent
$ADB shell "rm /data/local/agent"
$ADB shell "rm -rf /data/local/tests"
$ADB shell "rm -rf /data/local/tmp*"
$ADB push agent /data/local
$ADB shell "/data/local/agent"

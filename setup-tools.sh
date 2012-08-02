#!/bin/sh

wget http://busybox.net/downloads/binaries/1.19.0/busybox-armv6l
$ADB remount
$ADB push busybox-armv6l /system/bin/busybox

$ADB shell 'cd /system/bin; chmod 555 busybox; for x in `./busybox --list`; do ln -s ./busybox $x; done'

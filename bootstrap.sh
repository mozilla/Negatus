#!/bin/sh

export TOOLCH=toolchain
export PATH=$TOOLCH/bin:$PATH

if [ ! -d $TOOLCH ]
then
    $NDKPATH/build/tools/make-standalone-toolchain.sh --platform=android-9 --install-dir=$TOOLCH
fi

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.

# CFLAGS := \
#   -fPIC -mandroid -DANDROID \
#   -DOS_ANDROID -fno-short-enums -fno-exceptions -lstdc++
CFLAGS := \
  --sysroot=$(TOOLCH)/sysroot\
  -I$(TOOLCH)/lib/gcc/arm-linux-androideabi/4.6.x-google/include\
  -I$(TOOLCH)/lib/gcc/arm-linux-androideabi/4.6.x-google/include-fixed\
  -I$(TOOLCH)/arm-linux-androideabi/include/c++/4.6\
  -I$(TOOLCH)/arm-linux-androideabi/include/c++/4.6/arm-linux-androideabi\
  -I$(TOOLCH)/sysroot/usr/include\
  $(shell nspr-config --cflags)\
  -Llibs\
  -lstdc++ -lnspr4
FILES := \
  src/SUTAgent.cpp src/misc.cpp\
  src/SessionEventHandler.cpp\
  src/CommandEventHandler.cpp
CC=arm-linux-androideabi-g++

agent:
	$(CC) $(CFLAGS) $(FILES) -o agent

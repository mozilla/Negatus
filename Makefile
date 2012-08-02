# CFLAGS := \
#   -fPIC -mandroid -DANDROID \
#   -DOS_ANDROID -fno-short-enums -fno-exceptions -lstdc++
CFLAGS := \
  --sysroot=$(TOOLCH)/sysroot\
  -I$(TOOLCH)/lib/gcc/arm-linux-androideabi/4.6.x-google/include\
  -I$(TOOLCH)/lib/gcc/arm-linux-androideabi/4.6.x-google/include-fixed\
  -I$(TOOLCH)/arm-linux-androideabi/include/c++/4.6\
  -I$(TOOLCH)/arm-linux-androideabi/include/c++/4.6/arm-linux-androideabi\
  -I$(TOOLCH)/sysroot/usr/include
FILES := SUTAgent.cpp
CC=arm-linux-androideabi-g++

agent:
	$(CC) $(CFLAGS) $(FILES) -o agent

# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.

SRCS=\
	src/Buffer.cpp \
	src/BufferedSocket.cpp \
	src/CommandEventHandler.cpp \
	src/EventHandler.cpp \
	src/Hash.cpp \
	src/HeartbeatEventHandler.cpp \
	src/Logger.cpp \
	src/Logging.cpp \
	src/PullFileEventHandler.cpp \
	src/PushFileEventHandler.cpp \
	src/Reactor.cpp \
	src/Registration.cpp \
	src/Shell.cpp \
	src/SocketAcceptor.cpp \
	src/Strings.cpp \
	src/SUTAgent.cpp \
	src/Subprocess.cpp \
	src/SubprocessEventHandler.cpp \
	src/Version.cpp

OBJS=$(subst .cpp,.o,$(SRCS))

all: agent

agent: $(OBJS)
	$(LD) -o agent $(OBJS) $(LDFLAGS) $(LDLIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

clean:
	rm -f src/*.o agent

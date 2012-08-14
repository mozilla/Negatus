SRCS=\
	BufferedSocket.cpp \
	Logging.cpp \
	Reactor.cpp \
	SUTAgent.cpp \
	SessionEventHandler.cpp \
	SocketAcceptor.cpp

OBJS=$(subst .cpp,.o,$(SRCS))

all: agent

agent: $(OBJS)
	$(LD) -o agent $(OBJS) $(LDFLAGS) $(LDLIBS) 

%.o: %.c 
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $<

clean:
	rm -f *.o agent

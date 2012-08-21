LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	src/Buffer.cpp \
	src/BufferedSocket.cpp \
	src/CommandEventHandler.cpp \
	src/Logging.cpp \
	src/Reactor.cpp \
	src/SessionEventHandler.cpp \
	src/SocketAcceptor.cpp \
	src/Strings.cpp \
	src/SUTAgent.cpp

LOCAL_SHARED_LIBRARIES:= libnspr4 libplc4 libplds4

LOCAL_MODULE_TAGS:= eng

LOCAL_MODULE:= agent
LOCAL_MODULE_PATH:= $(TARGET_OUT_OPTIONAL_EXECUTABLES)
include $(BUILD_EXECUTABLE)


LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
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

LOCAL_LDFLAGS += -L$(LOCAL_PATH)/libs -lplds4 -lplc4 -ldl -lnspr4
LOCAL_SHARED_LIBRARIES := libstlport
LOCAL_C_INCLUDES:= \
       $(shell nspr-config --includedir) \
       external/stlport/stlport \
       bionic

LOCAL_CFLAGS += -fpermissive

LOCAL_MODULE_TAGS:= eng

LOCAL_MODULE:= sutagent
LOCAL_MODULE_PATH:= $(TARGET_OUT_OPTIONAL_EXECUTABLES)

include $(BUILD_EXECUTABLE)


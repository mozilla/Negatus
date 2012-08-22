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

#LOCAL_LDFLAGS += -L/Users/mbalaur/localCode/Mozilla/phoneB2G/system/sutagent/libs -lplds4 -lplc4 -ldl -lnspr4
LOCAL_LDFLAGS += -L$(LOCAL_PATH)/libs -lplds4 -lplc4 -ldl -lnspr4
LOCAL_SHARED_LIBRARIES := libstlport #libnspr4 libplc4-prebuilt libplds4-prebuilt
LOCAL_C_INCLUDES:= \
       $(shell nspr-config --includedir) \
       external/stlport/stlport \
       bionic

LOCAL_CFLAGS += -fpermissive

LOCAL_MODULE_TAGS:= eng

LOCAL_MODULE:= sutagent
LOCAL_MODULE_PATH:= $(TARGET_OUT_OPTIONAL_EXECUTABLES)

include $(BUILD_EXECUTABLE)


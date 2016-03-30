LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= test.cpp

LOCAL_CFLAGS	:= -std=gnu++11 -fpermissive

LOCAL_LDLIBS    := -llog

LOCAL_MODULE:= patch

include $(BUILD_SHARED_LIBRARY)

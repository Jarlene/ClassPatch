LOCAL_PATH:= $(call my-dir)
subdirs := $(addprefix $(LOCAL_PATH)/,$(addsuffix /Android.mk, \
		hook \
		test \
	))



LOCAL_SRC_FILES:= base/base64.cpp \
                  base/thread.cpp \
                  base/NativeException.cpp
CXX11_FLAGS := -std=c++11
LOCAL_CFLAGS += $(CXX11_FLAGS)
include $(subdirs)

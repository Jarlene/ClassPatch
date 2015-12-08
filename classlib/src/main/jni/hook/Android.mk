LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE:= substrate-dvm
LOCAL_SRC_FILES := libsubstrate-dvm.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE:= substrate
LOCAL_SRC_FILES := libsubstrate.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= Hook.cpp\
                  hookDalvik/HookDalvikMethod.cpp \
                  hookArt/HookArtMethod.cpp \
                  hookNative/HookNativeMethod.cpp \
                  hookNative/NativeHook.cpp \
                  hookNative/NativeElfHook.cpp


LOCAL_CFLAGS	:= -std=gnu++11 -fpermissive -DDEBUG -O0
#LOCAL_CFLAGS    += -std=c99 -Wall

LOCAL_LDLIBS    := -llog
LOCAL_LDLIBS    += -L$(LOCAL_PATH) -lsubstrate-dvm -lsubstrate

LOCAL_MODULE:= commonHook

include $(BUILD_SHARED_LIBRARY)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= Hook.cpp \
                  hookDalvik/HookDalvikMethod.cpp \
                  hookArt/HookArtMethod.cpp \
                  hookNative/relocate.cpp \
                  hookNative/InlineHook.cpp \
                  hookNative/HookNativeMethod.cpp \
                  hookNative/NativeHook.cpp \
                  hookNative/NativeElfHook.cpp \
                  hookNative/NativeInlineHook.cpp \
                  hookNative/InlineUtils.cpp



LOCAL_CFLAGS	:= -std=gnu++11 -fpermissive -DDEBUG -O0
#LOCAL_CFLAGS    += -std=c99 -Wall

LOCAL_LDLIBS    := -llog
#LOCAL_LDLIBS    += -L$(LOCAL_PATH) -lsubstrate-dvm -lsubstrate -lTKHooklib

LOCAL_MODULE:= commonHook

include $(BUILD_SHARED_LIBRARY)

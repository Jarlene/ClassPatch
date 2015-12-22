//
// Created by Jarlene on 2015/9/30.
//
#include "../../base/log.h"
#include "NativeElfHook.h"
#include "NativeHook.h"

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <jni.h>

#define LOG_TAG "HookNativeMethod"




static void* dvmDlsym(void *hand, const char *name) {
    void* ret = dlsym(hand, name);
    char msg[1024] = { 0 };
    snprintf(msg, sizeof(msg) - 1, "0x%x", ret);
    LOGD("%s = %s\n", name, msg);
    return ret;
}



extern void __attribute__ ((visibility ("hidden"))) HookNativeMethod(JNIEnv* env, jstring oldSoName, jstring newSoName, jstring oldSymbol, jstring newSymbol) {
    jboolean isCopy = JNI_TRUE;
    char* old_so_name = env->GetStringUTFChars(oldSoName, &isCopy);
    char* new_so_name = env->GetStringUTFChars(newSoName, &isCopy);
    char* old_symbol = env->GetStringUTFChars(oldSymbol, &isCopy);
    char* new_symbol = env->GetStringUTFChars(newSymbol, &isCopy);

    void* newHandle   = dlopen(new_so_name, RTLD_NOW);
    if (newHandle) {
        void* pluginNativeMethod = dvmDlsym(newHandle, new_symbol);
        LOGD("the new so method addr is %p", pluginNativeMethod);
        addElfHook(old_so_name, old_symbol, pluginNativeMethod);
        void* oldHandle = elfLoadLibrary(old_so_name);
        elfHookSymbol(oldHandle, old_symbol, (void**)&pluginNativeMethod);
    }

}




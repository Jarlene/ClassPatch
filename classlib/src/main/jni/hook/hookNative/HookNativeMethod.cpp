//
// Created by Jarlene on 2015/9/30.
//
#include "../../base/log.h"
#include "NativeElfHook.h"
#include "NativeHook.h"
#include "TKHooklib.h"
#include "InlineHook.h"
#include "NativeInlineHook.h"

#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
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
    if (old_so_name == NULL) {
        return;
    }
    char* new_so_name = env->GetStringUTFChars(newSoName, &isCopy);
    if (new_so_name == NULL) {
        return;
    }
    char* old_symbol = env->GetStringUTFChars(oldSymbol, &isCopy);
    if (old_symbol == NULL) {
        return;
    }
    char* new_symbol = env->GetStringUTFChars(newSymbol, &isCopy);
    if (new_symbol == NULL) {
        return;
    }


    void* oldHandle = dlopen(old_so_name, RTLD_NOW);
    if (oldHandle == NULL) {
        return;
    }
    void* newHandle = dlopen(new_so_name, RTLD_NOW);
    if (newHandle == NULL) {
        return;
    }


    void* oldMethod = dvmDlsym(oldHandle, old_symbol);
    if (oldMethod == NULL) {
        return;
    }
    void* pluginNativeMethod = dvmDlsym(newHandle, new_symbol);
    if (pluginNativeMethod == NULL) {
        return;
    }

    void* OldFunc = NULL;

    if (registerInlineHook(oldMethod, pluginNativeMethod, (uint32_t **)&OldFunc) == INLINE_HOOK_OK) {
        LOGD("registerInlineHook  Ok");
    }
    if (inlineHook(oldMethod) == INLINE_HOOK_OK ) {
        LOGD("inlineHook Ok");
    }

    //TK_InlineHookFunction(oldMethod, pluginNativeMethod, &OldFunc);


//    if (newHandle) {
//        void* pluginNativeMethod = dvmDlsym(newHandle, new_symbol);
//        LOGD("the new so method addr is %p", pluginNativeMethod);
//        addElfHook(old_so_name, old_symbol, pluginNativeMethod);
//        void* oldHandle = elfLoadLibrary(old_so_name);
//        elfHookSymbol(oldHandle, old_symbol, (void**)&pluginNativeMethod);
//    }

}




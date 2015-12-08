//
// Created by Jarlene on 2015/9/29.
//

#include <stdio.h>
#include <assert.h>
#include <android_runtime/AndroidRuntime.h>
#include "jni.h"
#include "../base/log.h"


#define LOG_TAG "Hook"
#define HOOK_CLASS "com/baidu/music/classlib/jni/HookBridge"

extern jboolean ArtModelInit(JNIEnv* env, int apiLevel);
extern void HookArtMethod(JNIEnv* env, jobject srcMethod, jobject targetMethod);

extern jboolean DalvikModelInit(JNIEnv* env, jclass mainClass, int apiLevel);
extern void HookDalvikMethod(JNIEnv* env, jobject srcMethod, jobject targetMethod);

extern void HookNativeMethod(JNIEnv* env, jstring oldSoName, jstring newSoName, jstring oldSymbol, jstring newSymbol);

extern void DalvikFilterClass(JNIEnv* env, jstring className);

static bool isArtModel;
static jclass mainClass;




extern "C" void testString(JNIEnv* env) {
    jclass clazz = env->FindClass("com/baidu/music/classpatch/MainActivity");
    if (clazz == NULL) {
        return;
    }

    jobject obj = env->NewGlobalRef(clazz);
    jmethodID method = env->GetStaticMethodID(clazz, "toastSoString", "(Ljava/lang/String;)V");
    if (method == NULL) {
        return;
    }
    jstring str = env->NewStringUTF("come from libcommonHook.so ---testString");
    env->CallStaticVoidMethod(obj, method, str);
    env->DeleteGlobalRef(obj);
}

static bool initHookEnv(JNIEnv* env, jclass clazz, jboolean isArt, jint apiLevel) {
    isArtModel = isArt;
    LOGV("the runtime api level is %d, android's model is ", (int)apiLevel, (isArt ? "art" : "dalvik"));
    if (isArt) {
        ArtModelInit(env, (int) apiLevel);
    } else {
        DalvikModelInit(env, mainClass, (int) apiLevel);
    }
    return true;
}

static void HookJavaMethod(JNIEnv* env, jclass clazz, jobject srcMethod, jobject targetMethod) {
    LOGV("HookMethod android's model is %s", (isArtModel ? "art" : "dalvik"));
    if (isArtModel) {
        HookArtMethod(env, srcMethod, targetMethod);
    } else {
        HookDalvikMethod(env, srcMethod, targetMethod);
    }
}

static void classesFilter(JNIEnv* env, jclass clazz, jobjectArray classArray) {
    if (!isArtModel) {
        int size = env->GetArrayLength(classArray);
        for (int i = 0; i < size; ++i) {
            jstring clazzName = (jstring) env->GetObjectArrayElement(classArray, i);
            DalvikFilterClass(env, clazzName);
        }
    }
}

static void HookNativeMethodSo(JNIEnv* env, jclass clazz, jstring oldSoName, jstring newSoName, jstring oldSymbol, jstring newSymbol) {
    HookNativeMethod(env, oldSoName, newSoName, oldSymbol, newSymbol);
    testString(env);
}


static int registerNativeMethods(JNIEnv* env, const char* className,
                                 JNINativeMethod* gMethods, int numMethods) {
    jclass clazz= env->FindClass(className);
    mainClass = clazz;
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

/*
 * native方法映射
 */
static JNINativeMethod nativeMethods[] = {
        { "initHookEnv", "(ZI)Z", (void*)initHookEnv },
        { "replaceJavaMethod", "(Ljava/lang/reflect/Member;Ljava/lang/reflect/Member;)V", (void*)HookJavaMethod },
        { "replaceNativeMethod", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V", (void*)HookNativeMethodSo },
        { "classesResolvedFilter", "([Ljava/lang/String;)V", (void*) classesFilter}
};





/**
 * 注册native方法
 */
static int registerNatives(JNIEnv* env) {
    if (!registerNativeMethods(env, HOOK_CLASS, nativeMethods,
                               sizeof(nativeMethods) / sizeof(nativeMethods[0])))
        return JNI_FALSE;

    return JNI_TRUE;
}

/*
 * Set some test stuff up.
 *
 * Returns the JNI version on success, -1 on failure.
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env = NULL;
    jint result = -1;

    if (vm->GetEnv((void**) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    assert(env != NULL);
    if (!registerNatives(env)) { //注册
        return -1;
    }
    /* success -- return valid version number */
    result = JNI_VERSION_1_6;

    return result;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM* vm, void* reserved) {
    LOGI("JNI_OnUnload");
    JNIEnv *env;
    int nJNIVersionOK = vm->GetEnv((void **)&env, JNI_VERSION_1_6) ;
    if (nJNIVersionOK == JNI_OK) {
        if(mainClass) {
            env->DeleteGlobalRef(mainClass);
        }
    }
}

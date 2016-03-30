//
// Created by Administrator on 2016/3/29.
//

#include <stdio.h>
#include <assert.h>
#include <android_runtime/AndroidRuntime.h>
#include "jni.h"


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
    jstring str = env->NewStringUTF("come from test.so ---testString");
    env->CallStaticVoidMethod(obj, method, str);
    env->DeleteGlobalRef(obj);
}
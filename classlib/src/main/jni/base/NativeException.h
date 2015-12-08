//
// Created by Jarlene on 2015/10/15.
//

#ifndef COMMONDEMO_NATIVEEXCEPTION_H
#define COMMONDEMO_NATIVEEXCEPTION_H

#include <jni.h>
#include <stddef.h>
#include <stdio.h>

#define MSG_SIZE 1024


class NativeException {

public:
    static jint throwNoClassDefError(JNIEnv* pEnv, const char* szFmt, ...);

    static jint throwRuntimeException(JNIEnv* pEnv, const char* szFmt, ...);

    static jint throwIllegalArgumentException(JNIEnv* pEnv, const char* szFmt, ...);

    static jint throwIllegalStateException(JNIEnv* pEnv, const char* szFmt, ...);

    static jint throwIOException(JNIEnv* pEnv, const char* szFmt, ...);

    static jint throwAssertionError(JNIEnv* pEnv, const char* szFmt, ...);

    static jint throwOutOfMemoryError(JNIEnv* pEnv, const char* szFmt, ...);

    static jint throwNullPointerException(JNIEnv* pEnv, const char* szFmt, ...);

private:
    static jint throwException(JNIEnv* pEnv, const char* szClassName, const char* szFmt, va_list va_args);
};


#endif //COMMONDEMO_NATIVEEXCEPTION_H

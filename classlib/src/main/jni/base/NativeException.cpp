//
// Created by Jarlene on 2015/10/15.
//

#include "NativeException.h"

static jint NativeException::throwException(JNIEnv* pEnv, const char* szClassName, const char* szFmt, va_list va_args) {
    char szMsg[MSG_SIZE];
    vsnprintf(szMsg, MSG_SIZE, szFmt, va_args);
    jclass exClass = pEnv->FindClass(szClassName);
    return pEnv->ThrowNew(exClass, szMsg);
}

static jint NativeException::throwNoClassDefError(JNIEnv* pEnv, const char* szFmt, ...) {
    va_list va_args;
    va_start(va_args, szFmt);
    jint ret = throwException(pEnv, "java/lang/NoClassDefFoundError", szFmt, va_args);
    va_end(va_args);
    return ret;
}

static jint NativeException::throwRuntimeException(JNIEnv* pEnv, const char* szFmt, ...) {
    va_list va_args;
    va_start(va_args, szFmt);
    jint ret = throwException(pEnv, "java/lang/RuntimeException", szFmt, va_args);
    va_end(va_args);
    return ret;
}

static jint NativeException::throwIllegalArgumentException(JNIEnv* pEnv, const char* szFmt, ...) {
    va_list va_args;
    va_start(va_args, szFmt);
    jint ret = throwException(pEnv, "java/lang/IllegalArgumentException", szFmt, va_args);
    va_end(va_args);
    return ret;
}

static jint NativeException::throwIllegalStateException(JNIEnv* pEnv, const char* szFmt, ...) {
    va_list va_args;
    va_start(va_args, szFmt);
    jint ret = throwException(pEnv, "java/lang/IllegalStateException", szFmt, va_args);
    va_end(va_args);
    return ret;
}

static jint NativeException::throwOutOfMemoryError(JNIEnv* pEnv, const char* szFmt, ...) {
    va_list va_args;
    va_start(va_args, szFmt);
    jint ret = throwException(pEnv, "java/lang/OutOfMemoryError", szFmt, va_args);
    va_end(va_args);
    return ret;
}

static jint NativeException::throwAssertionError(JNIEnv* pEnv, const char* szFmt, ...) {
    va_list va_args;
    va_start(va_args, szFmt);
    jint ret = throwException(pEnv, "java/lang/AssertionError", szFmt, va_args);
    va_end(va_args);
    return ret;
}

static jint NativeException::throwIOException(JNIEnv* pEnv, const char* szFmt, ...) {
    va_list va_args;
    va_start(va_args, szFmt);
    jint ret = throwException(pEnv, "java/io/IOException", szFmt, va_args);
    va_end(va_args);
    return ret;
}

static jint NativeException::throwNullPointerException(JNIEnv* pEnv, const char* szFmt, ...) {
    va_list va_args;
    va_start(va_args, szFmt);
    jint ret = throwException(pEnv, "java/lang/NullPointerException", szFmt, va_args);
    va_end(va_args);
    return ret;
}

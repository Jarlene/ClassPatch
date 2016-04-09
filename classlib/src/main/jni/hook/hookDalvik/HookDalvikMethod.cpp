//
// Created by Jarlene on 2015/9/29.
//
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>

#include <stdbool.h>
#include <fcntl.h>
#include <dlfcn.h>

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cassert>
#include <algorithm>

#include "DexFile.h"
#include "dalvik.h"
#include "substrate.h"
#include "../hookNative/NativeHook.h"
#include "../hookNative/NativeInlineHook.h"



#define LOG_TAG "HookDalvikMethod"

static jmethodID jInvokeMethod;
static jmethodID jClassMethod;
static ClassObject* mainClassObj;
static jboolean isOpen = true;

std::vector<char*> FilterClassNamesVector;

ClassObject* (*dvmResolveClass_Proxy)(ClassObject* referrer, u4 classIdx, bool fromUnverifiedConstant);

ClassObject* proxyDvmResolveClass(ClassObject* referrer, u4 classIdx, bool fromUnverifiedConstant) {
    DvmDex* pDvmDex = referrer->pDvmDex;
    ClassObject* resClass = NULL;
    char* className;
    resClass = dvmDexGetResolvedClass(pDvmDex, classIdx);
    if (resClass != NULL && isOpen) { // 如果需要立即生效，则需要将替换的class做判断，让其不进行返回。
        return resClass;
    }

    className = dexStringByTypeIdx(pDvmDex->pDexFile, classIdx);
    if (className[0] != '\0' && className[1] == '\0') {
        resClass = dvmFindPrimitiveClass_fnPtr(className[0]);
    } else {
        resClass = dvmFindClassNoInit_fnPtr(className, referrer->classLoader);
    }
    if (resClass != NULL) {
        dvmDexSetResolvedClass(pDvmDex, classIdx, resClass);
    } else {
        resClass = dvmResolveClass_Proxy(referrer, classIdx, true);
    }
    return resClass;
}



//指明要hook的lib ：
//MSConfig(MSFilterLibrary,"/system/lib/libdvm.so")
//
//MSInitialize {
//    LOGD("Cydia Init");
//    MSImageRef image;
//    //载入lib
//    image = MSGetImageByName("/system/lib/libdvm.so");
//    if (image != NULL) {
//        LOGD("image is not null");
//        void * dexload=MSFindSymbol(image,"dvmResolveClass");
//
//        if(dexload != NULL) {
//            LOGD("dexload is not null addr is %p", dexload);
//            MSHookFunction(dexload, (void*)proxyDvmResolveClass, (void**)&dvmResolveClass_Proxy);
//        } else {
//            LOGD("error find dvmResolveClass");
//        }
//    }
//}
//



static void DalvikHandler(const u4* args, JValue* pResult, const Method* method, struct Thread* self) {
    ClassObject* returnType;
    JValue result;
    ArrayObject* argArray;
    LOGV("DalvikHandler source method: %s %s", method->name, method->shorty);
    Method* targetMethod = (Method*)method->insns;

    targetMethod->accessFlags = targetMethod->accessFlags | ACC_PUBLIC; // 访问权限设置成public。
    LOGV("DalvikHandler target method: %s %s", targetMethod->name, targetMethod->shorty);
    returnType = dvmGetBoxedReturnType_fnPtr(method);
    if (returnType == NULL) {
        LOGE("return type is null, goto fail");
        assert(dvmGetBoxedReturnType_fnPtr(self));
        goto Bail;
    }
    LOGV("call Dalvik Hanlder");
    if (!dvmIsStaticMethod(targetMethod)) {
        Object* thisObj = (Object*) args[0];
        ClassObject* tmp = thisObj->clazz;
        thisObj->clazz = targetMethod->clazz;
        argArray = dvmBoxMethodArgs(targetMethod, args + 1);
        if (dvmCheckException_fnPtr(self)) {
            LOGE("non static check exception true");
            goto Bail;
        }
        dvmCallMethod_fnPtr(self, (Method*) jInvokeMethod,
                            dvmCreateReflectMethodObject_fnPtr(targetMethod), &result, thisObj,
                            argArray);
        LOGV("call dvmCallMethod_fnPtr ");

        thisObj->clazz = tmp;
    } else {
        argArray = dvmBoxMethodArgs(targetMethod, args);
        if (dvmCheckException_fnPtr(self)) {
            goto Bail;
        }
        dvmCallMethod_fnPtr(self, (Method*) jInvokeMethod,
                            dvmCreateReflectMethodObject_fnPtr(targetMethod), &result, NULL, argArray);
    }
    if (dvmCheckException_fnPtr(self)) {
        goto Bail;
    }
    if (returnType->primitiveType == PRIM_VOID) {
        LOGV("Method return type is void ");
    } else if (result.l == NULL) {
        if (dvmIsPrimitiveClass(returnType)) {
            LOGE("non-static check exception true");
            goto Bail;
        }
        pResult->l = NULL;
    } else {
        if (!dvmUnboxPrimitive_fnPtr(result.l, returnType, pResult)) {
            char msg[1024] = { 0 };
            snprintf(msg, sizeof(msg) - 1, "%s!=%s\0",
                     ((Object*) result.l)->clazz->descriptor,
                     returnType->descriptor);
            goto Bail;
        }
    }
    Bail: dvmReleaseTrackedAlloc_fnPtr((Object*) argArray, self);
}

static void* dvmDlsym(void *hand, const char *name) {
    void* ret = dlsym(hand, name);
    char msg[1024] = { 0 };
    snprintf(msg, sizeof(msg) - 1, "0x%x", ret);
    LOGD("%s = %s\n", name, msg);
    return ret;
}

static ArrayObject* dvmBoxMethodArgs(const Method* method, const u4* args){
    const char* desc = &method->shorty[1]; // [0] is the return type.

    /* count args */
    size_t argCount = dexProtoGetParameterCount_fnPtr(&method->prototype);

    static ClassObject* classJavaLangObjectArray = dvmFindArrayClass_fnPtr("[Ljava/lang/Object;", NULL);

    /* allocate storage */
    ArrayObject* argArray = dvmAllocArrayByClass_fnPtr(classJavaLangObjectArray, argCount, ALLOC_DEFAULT);
    if (argArray == NULL)
        return NULL;

    Object** argObjects = (Object**) (void*) argArray->contents;

    /*
     * Fill in the array.
     */
    size_t srcIndex = 0;
    size_t dstIndex = 0;
    while (*desc != '\0') {
        char descChar = *(desc++);
        JValue value;

        switch (descChar) {
            case 'Z':
            case 'C':
            case 'F':
            case 'B':
            case 'S':
            case 'I':
                value.i = args[srcIndex++];
                argObjects[dstIndex] = (Object*) dvmBoxPrimitive_fnPtr(value,
                                                                       dvmFindPrimitiveClass_fnPtr(descChar));
                /* argObjects is tracked, don't need to hold this too */
                dvmReleaseTrackedAlloc_fnPtr(argObjects[dstIndex], NULL);
                dstIndex++;
                break;
            case 'D':
            case 'J':
                value.j = dvmGetArgLong(args, srcIndex);
                srcIndex += 2;
                argObjects[dstIndex] = (Object*) dvmBoxPrimitive_fnPtr(value, dvmFindPrimitiveClass_fnPtr(descChar));
                dvmReleaseTrackedAlloc_fnPtr(argObjects[dstIndex], NULL);
                dstIndex++;
                break;
            case '[':
            case 'L':
                argObjects[dstIndex++] = (Object*) args[srcIndex++];
                LOGD("boxMethodArgs object: index = %d", dstIndex - 1);
                break;
        }
    }

    return argArray;
}

static s8 dvmGetArgLong(const u4* args, int elem) {
    s8 val;
    memcpy(&val, &args[elem], sizeof(val));
    return val;
}

static bool dvmIsPrimitiveClass(const ClassObject* clazz) {
    return clazz->primitiveType != PRIM_NOT;
}

extern jboolean  __attribute__((visibility ("hidden"))) DalvikModelInit(JNIEnv* env, jclass mainClass, int apiLevel) {

    // 处理android 3.0系统以下的特殊地方。具体方法的Symbol可以用IDA工具查看
    void* dvmHand = dlopen("/system/lib/libdvm.so", RTLD_GLOBAL);
//    elfHookSymbol(dvmHand, "dvmResolveClass", (void*)proxyDvmResolveClass);

    if (dvmHand) {
        dvmComputeMethodArgsSize_fnPtr = dvmDlsym(dvmHand,
                                                   apiLevel > 10 ?
                                                   "_Z24dvmComputeMethodArgsSizePK6Method" :
                                                   "dvmComputeMethodArgsSize");
        if (!dvmComputeMethodArgsSize_fnPtr) {
            LOGE("dvmComputeMethodArgsSize_fnPtr error");
            return JNI_FALSE;
        }
        dvmCallMethod_fnPtr = dvmDlsym(dvmHand,
                                        apiLevel > 10 ?
                                        "_Z13dvmCallMethodP6ThreadPK6MethodP6ObjectP6JValuez" :
                                        "dvmCallMethod");
        if (!dvmCallMethod_fnPtr) {
            LOGE("dvmCallMethod_fnPtr error");
            return JNI_FALSE;
        }
        dexProtoGetParameterCount_fnPtr = dvmDlsym(dvmHand,
                                                    apiLevel > 10 ?
                                                    "_Z25dexProtoGetParameterCountPK8DexProto" :
                                                    "dexProtoGetParameterCount");
        if (!dexProtoGetParameterCount_fnPtr) {
            LOGE("dexProtoGetParameterCount_fnPtr error");
            return JNI_FALSE;
        }

        dvmAllocArrayByClass_fnPtr = dvmDlsym(dvmHand,
                                               "dvmAllocArrayByClass");
        if (!dvmAllocArrayByClass_fnPtr) {
            LOGE("dvmAllocArrayByClass_fnPtr error");
            return JNI_FALSE;
        }
        dvmBoxPrimitive_fnPtr = dvmDlsym(dvmHand,
                                          apiLevel > 10 ?
                                          "_Z15dvmBoxPrimitive6JValueP11ClassObject" :
                                          "dvmWrapPrimitive");
        if (!dvmBoxPrimitive_fnPtr) {
            LOGE("dvmBoxPrimitive_fnPtr error");
            return JNI_FALSE;
        }
        dvmFindPrimitiveClass_fnPtr = dvmDlsym(dvmHand,
                                                apiLevel > 10 ?
                                                "_Z21dvmFindPrimitiveClassc" :
                                                "dvmFindPrimitiveClass");
        if (!dvmFindPrimitiveClass_fnPtr) {
            LOGE("dvmFindPrimitiveClass_fnPtr error");
            return JNI_FALSE;
        }

        dvmFindClassNoInit_fnPtr = dvmDlsym(dvmHand,
                                            apiLevel > 10 ?
                                            "_Z18dvmFindClassNoInitPKcP6Object" :
                                            "dvmFindClassNoInit");
        if (!dvmFindClassNoInit_fnPtr) {
            LOGE("dvmFindClassNoInit_fnPtr error");
            return JNI_FALSE;
        }

        dvmReleaseTrackedAlloc_fnPtr = dvmDlsym(dvmHand,
                                                 "dvmReleaseTrackedAlloc");
        if (!dvmReleaseTrackedAlloc_fnPtr) {
            LOGE("dvmReleaseTrackedAlloc_fnPtr error");
            return JNI_FALSE;
        }
        dvmCheckException_fnPtr = dvmDlsym(dvmHand,
                                            apiLevel > 10 ?
                                            "_Z17dvmCheckExceptionP6Thread" : "dvmCheckException");
        if (!dvmCheckException_fnPtr) {
            LOGE("dvmCheckException_fnPtr error");
            return JNI_FALSE;
        }

        dvmGetException_fnPtr = dvmDlsym(dvmHand,
                                          apiLevel > 10 ?
                                          "_Z15dvmGetExceptionP6Thread" : "dvmGetException");
        if (!dvmGetException_fnPtr) {
            LOGE("dvmGetException_fnPtr error");
            return JNI_FALSE;
        }
        dvmFindArrayClass_fnPtr = dvmDlsym(dvmHand,
                                            apiLevel > 10 ?
                                            "_Z17dvmFindArrayClassPKcP6Object" :
                                            "dvmFindArrayClass");
        if (!dvmFindArrayClass_fnPtr) {
            LOGE("dvmFindArrayClass_fnPtr error");
            return JNI_FALSE;
        }
        dvmCreateReflectMethodObject_fnPtr = dvmDlsym(dvmHand,
                                                       apiLevel > 10 ?
                                                       "_Z28dvmCreateReflectMethodObjectPK6Method" :
                                                       "dvmCreateReflectMethodObject");
        if (!dvmCreateReflectMethodObject_fnPtr) {
            LOGE("dvmCreateReflectMethodObject_fnPtr error");
            return JNI_FALSE;
        }

        dvmGetBoxedReturnType_fnPtr = dvmDlsym(dvmHand,
                                                apiLevel > 10 ?
                                                "_Z21dvmGetBoxedReturnTypePK6Method" :
                                                "dvmGetBoxedReturnType");
        if (!dvmGetBoxedReturnType_fnPtr) {
            LOGE("dvmGetBoxedReturnType_fnPtr error");
            return JNI_FALSE;
        }
        dvmUnboxPrimitive_fnPtr = dvmDlsym(dvmHand,
                                            apiLevel > 10 ?
                                            "_Z17dvmUnboxPrimitiveP6ObjectP11ClassObjectP6JValue" :
                                            "dvmUnwrapPrimitive");
        if (!dvmUnboxPrimitive_fnPtr) {
            LOGE("dvmUnboxPrimitive_fnPtr error");
            return JNI_FALSE;
        }
        dvmDecodeIndirectRef_fnPtr = dvmDlsym(dvmHand,
                                               apiLevel > 10 ?
                                               "_Z20dvmDecodeIndirectRefP6ThreadP8_jobject" :
                                               "dvmDecodeIndirectRef");
        if (!dvmDecodeIndirectRef_fnPtr) {
            LOGE("dvmDecodeIndirectRef_fnPtr error");
            return JNI_FALSE;
        }
        dvmThreadSelf_fnPtr = dvmDlsym(dvmHand,
                                        apiLevel > 10 ?
                                        "_Z13dvmThreadSelfv" :
                                        "dvmThreadSelf");
        if (!dvmThreadSelf_fnPtr) {
            LOGE("dvmThreadSelf_fnPtr error");
            return JNI_FALSE;
        }
        dvmVerifyClass_fnPtr = dvmDlsym(dvmHand,
                                        apiLevel > 10 ?
                                        "_Z14dvmVerifyClassP11ClassObject":
                                        "dvmVerifyClass");
        if (!dvmFindArrayClass_fnPtr) {
            LOGE("dvmFindArrayClass_fnPtr error");
            return JNI_FALSE;
        }

        dvmResolveClass_fnPtr = dvmDlsym(dvmHand, "dvmResolveClass");
        if (!dvmResolveClass_fnPtr) {
            LOGE("dvmResolveClass_fnPtr error");
            return JNI_FALSE;
        }
        if (registerInlineHook(dvmResolveClass_fnPtr, proxyDvmResolveClass, (uint32_t **)&dvmResolveClass_Proxy) == INLINE_HOOK_OK) {
            LOGD("registerInlineHook  Ok");
        }
        if (inlineHook(dvmResolveClass_fnPtr) == INLINE_HOOK_OK ) {
            LOGD("inlineHook Ok");
        }

//        Method* dvmResolveClassMethod = (Method*)dvmResolveClass_fnPtr;
//        dvmResolveClassMethod->accessFlags = ACC_NATIVE | ACC_PUBLIC;
//        dvmResolveClassMethod->nativeFunc = proxyDvmResolveClass;


        mainClassObj = (ClassObject*) dvmDecodeIndirectRef_fnPtr(dvmThreadSelf_fnPtr(), mainClass);
        if (mainClassObj == NULL) {
            LOGD("main class is null");
        }

        jclass clazz = env->FindClass("java/lang/reflect/Method");
        jInvokeMethod = env->GetMethodID(clazz, "invoke",
                                         "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;");
        jClassMethod = env->GetMethodID(clazz, "getDeclaringClass",
                                        "()Ljava/lang/Class;");

        return JNI_TRUE;
    } else {
        return JNI_FALSE;
    }
}

extern void __attribute__ ((visibility ("hidden"))) HookDalvikMethod(JNIEnv* env, jobject src, jobject target) {
    jobject clazz = env->CallObjectMethod(src, jClassMethod);
    ClassObject* clz = (ClassObject*) dvmDecodeIndirectRef_fnPtr(dvmThreadSelf_fnPtr(), clazz);
    clz->status = CLASS_INITIALIZED;

    Method* srcMethod = (Method*) env->FromReflectedMethod(src);
    Method* targetMethod = (Method*) env->FromReflectedMethod(target);
    LOGD("dalvikMethod: %s", srcMethod->name);
    srcMethod->jniArgInfo = 0x80000000;
    srcMethod->accessFlags |= ACC_NATIVE;

    int argsSize = dvmComputeMethodArgsSize_fnPtr(srcMethod);
    if (!dvmIsStaticMethod(srcMethod)) {
        argsSize++;
    }

    srcMethod->registersSize = srcMethod->insSize = argsSize;
    srcMethod->insns = (void*) targetMethod;

    srcMethod->nativeFunc = DalvikHandler;
}

extern void __attribute__ ((visibility ("hidden"))) DalvikFilterClass(JNIEnv* env, jstring className) {
    jboolean isCopy = JNI_TRUE;
    char* clazz = env->GetStringUTFChars(className, &isCopy);
    LOGD("filte class name  is %s", clazz);
    FilterClassNamesVector.push_back(clazz);
}

extern void __attribute__ ((visibility ("hidden"))) resolveOpenHot(JNIEnv* env, jboolean open) {
    isOpen = open;
}

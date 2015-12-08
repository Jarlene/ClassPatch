//
// Created by Jarlene on 2015/9/29.
//
#include <time.h>
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

#include "art.h"

#define LOG_TAG "hookArtMethod"
using namespace art;
using namespace mirror;


static int ArtApiLevel;


static void* artDlsym(void *hand, const char *name) {
    void* ret = dlsym(hand, name);
    char msg[1024] = { 0 };
    snprintf(msg, sizeof(msg) - 1, "0x%x", ret);
    LOGD("%s = %s\n", name, msg);
    return ret;
}

extern jboolean  __attribute__((visibility ("hidden"))) ArtModelInit(JNIEnv* env, int apiLevel) {
    ArtApiLevel = apiLevel;
    LOGV("init Art Model Method");
    void* artHand = dlopen("libart.so", RTLD_NOW);
    if (artHand) {
        char* verifyClass = "_ZN3art11ClassLinker23VerifyClassUsingOatFileERKNS_7DexFileEPNS_6mirror5ClassERNS5_6StatusE";
        artVerifyClass_fnPtr = artDlsym(artHand, verifyClass);
        if (!artVerifyClass_fnPtr) {
            LOGE("artVerifyClass_fnPtr error");
            return JNI_FALSE;
        }
    }
    return JNI_TRUE;
}

extern void  __attribute__((visibility ("hidden"))) HookArtMethod(JNIEnv* env, jobject srcMethod, jobject targetMethod) {
    ArtMethod* srcMeth = (ArtMethod*) env->FromReflectedMethod(srcMethod);
    ArtMethod* targetMeth = (ArtMethod*) env->FromReflectedMethod(targetMethod);

    targetMeth->declaring_class_->class_loader_ = srcMeth->declaring_class_->class_loader_; // ClassLoader
    targetMeth->declaring_class_->clinit_thread_id_ = srcMeth->declaring_class_->clinit_thread_id_;
    targetMeth->declaring_class_->status_ = srcMeth->declaring_class_->status_;

    srcMeth->declaring_class_ = targetMeth->declaring_class_;
    if (ArtApiLevel > 22) {
        srcMeth->dex_cache_resolved_types_ = targetMeth->dex_cache_resolved_types_;
        srcMeth->access_flags_ = targetMeth->access_flags_;
        srcMeth->dex_cache_resolved_methods_ = targetMeth->dex_cache_resolved_methods_;
        srcMeth->dex_code_item_offset_ = targetMeth->dex_code_item_offset_;
        srcMeth->method_index_ = targetMeth->method_index_;
        srcMeth->dex_method_index_ = targetMeth->dex_method_index_;

        srcMeth->ptr_sized_fields_.entry_point_from_interpreter_ = targetMeth->ptr_sized_fields_.entry_point_from_interpreter_;
        srcMeth->ptr_sized_fields_.entry_point_from_jni_ = targetMeth->ptr_sized_fields_.entry_point_from_jni_;
        srcMeth->ptr_sized_fields_.entry_point_from_quick_compiled_code_ = targetMeth->ptr_sized_fields_.entry_point_from_quick_compiled_code_;
        LOGV("this is running on android 6.0 device");
    } else if (ArtApiLevel > 21) {
        srcMeth->dex_cache_resolved_types_ = targetMeth->dex_cache_resolved_types_;
        srcMeth->access_flags_ = targetMeth->access_flags_;
        srcMeth->dex_cache_resolved_methods_ = targetMeth->dex_cache_resolved_methods_;
        srcMeth->dex_code_item_offset_ = targetMeth->dex_code_item_offset_;
        srcMeth->method_index_ = targetMeth->method_index_;
        srcMeth->dex_method_index_ = targetMeth->dex_method_index_;

        srcMeth->ptr_sized_fields_.entry_point_from_interpreter_ = targetMeth->ptr_sized_fields_.entry_point_from_interpreter_;
        srcMeth->ptr_sized_fields_.entry_point_from_jni_ = targetMeth->ptr_sized_fields_.entry_point_from_jni_;
        srcMeth->ptr_sized_fields_.entry_point_from_quick_compiled_code_ = targetMeth->ptr_sized_fields_.entry_point_from_quick_compiled_code_;
        LOGV("this is running on android 5.1 device");
    } else {
        srcMeth->access_flags_ = targetMeth->access_flags_;
        srcMeth->frame_size_in_bytes_ = targetMeth->frame_size_in_bytes_;
        srcMeth->dex_cache_initialized_static_storage_ = targetMeth->dex_cache_initialized_static_storage_;
        srcMeth->dex_cache_resolved_types_ = targetMeth->dex_cache_resolved_types_;
        srcMeth->dex_cache_resolved_methods_ = targetMeth->dex_cache_resolved_methods_;
        srcMeth->vmap_table_ = targetMeth->vmap_table_;
        srcMeth->core_spill_mask_ = targetMeth->core_spill_mask_;
        srcMeth->fp_spill_mask_ = targetMeth->fp_spill_mask_;
        srcMeth->mapping_table_ = targetMeth->mapping_table_;
        srcMeth->code_item_offset_ = targetMeth->code_item_offset_;
        srcMeth->entry_point_from_compiled_code_ = targetMeth->entry_point_from_compiled_code_;

        srcMeth->entry_point_from_interpreter_ = targetMeth->entry_point_from_interpreter_;
        srcMeth->native_method_ = targetMeth->native_method_;
        srcMeth->method_index_ = targetMeth->method_index_;
        srcMeth->method_dex_index_ = targetMeth->method_dex_index_;
        LOGV("this is running on android 5.0 device");
    }

}

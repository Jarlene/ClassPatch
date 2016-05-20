#include <string.h>
#include <jni.h>
#include <android/log.h>
#include "elfhook.h"

#define  LOG_TAG    "HookTest"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

int hook_memcpy(const void *s1, const void *s2, int count) {
	LOGE("***%s: p1=%p, p2=%p\n", __FUNCTION__, s1, s2);
	int nRet = memcmp(s1, s2, count);
	return nRet;
}

jint Java_com_example_myndk_MainActivity_hookTest(JNIEnv* env, jobject thiz) {
	char * soName = "libhello-jni.so";
	char * funName = "memcpy";
	LOGE("hook_memcpy addr: %p", hook_memcpy);
	elfHook(soName, funName, (unsigned) hook_memcpy);
	return -1;
}


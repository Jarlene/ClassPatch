//
// Created by Administrator on 2016/5/4.
//

#ifndef CLASSPATCH_NATIVEINLINEHOOK_H
#define CLASSPATCH_NATIVEINLINEHOOK_H

#include "../../base/log.h"

#define LOG_TAG "NativeInlineHook"

struct hook_t {
	unsigned int jump[3];
	unsigned int store[3];
	unsigned char jumpt[20];
	unsigned char storet[20];
	unsigned int orig;
	unsigned int patch;
	unsigned char thumb;
	unsigned char name[128];
	void *data;
};

void hook_cacheflush(unsigned int begin, unsigned int end);
void hook_precall(struct hook_t *h);
void hook_postcall(struct hook_t *h);
int hook(struct hook_t *h, int pid, char *libname, char *funcname, void *hook_arm, void *hook_thumb);
void unhook(struct hook_t *h);

#endif //CLASSPATCH_NATIVEINLINEHOOK_H

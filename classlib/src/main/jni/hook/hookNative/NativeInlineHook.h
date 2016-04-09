//
// Created by Jarlene on 2016/4/9.
//
#include <stdio.h>

#ifndef CLASSPATCH_NATIVEINLINEHOOK_H
#define CLASSPATCH_NATIVEINLINEHOOK_H

#ifdef __cplusplus
extern "C" {
#endif

enum InlineHook_Status {
	INLINE_HOOK_ERROR_UNKNOWN = -1,
	INLINE_HOOK_OK = 0,
	INLINE_HOOK_ERROR_NOT_INITIALIZED,
	INLINE_HOOK_ERROR_NOT_EXECUTABLE,
	INLINE_HOOK_ERROR_NOT_REGISTERED,
	INLINE_HOOK_ERROR_NOT_HOOKED,
	INLINE_HOOK_ERROR_ALREADY_REGISTERED,
	INLINE_HOOK_ERROR_ALREADY_HOOKED,
	INLINE_HOOK_ERROR_SO_NOT_FOUND,
	INLINE_HOOK_ERROR_FUNCTION_NOT_FOUND
};

enum hook_status {
	REGISTERED,
	HOOKED,
};

enum InlineHook_Status registerInlineHook(uint32_t target_addr, uint32_t new_addr, uint32_t **proto_addr);
enum InlineHook_Status inlineUnHook(uint32_t target_addr);
void inlineUnHookAll();
enum InlineHook_Status inlineHook(uint32_t target_addr);
void inlineHookAll();

#ifdef __cplusplus
};
#endif

#endif //CLASSPATCH_NATIVEINLINEHOOK_H

//
// Created by Jarlene on 16/5/16.
//

#ifndef CLASSPATCH_INLINEHOOK_H
#define CLASSPATCH_INLINEHOOK_H

#include <stdio.h>

enum inline_hook_status {
	INLINE_ERROR_UNKNOWN = -1,
	INLINE_OK = 0,
	INLINE_ERROR_NOT_INITIALIZED,
	INLINE_ERROR_NOT_EXECUTABLE,
	INLINE_ERROR_NOT_REGISTERED,
	INLINE_ERROR_NOT_HOOKED,
	INLINE_ERROR_ALREADY_REGISTERED,
	INLINE_ERROR_ALREADY_HOOKED,
	INLINE_ERROR_SO_NOT_FOUND,
	INLINE_ERROR_FUNCTION_NOT_FOUND
};

enum inline_hook_status registerInlineHook(uint32_t target_addr, uint32_t new_addr, uint32_t **proto_addr);
enum inline_hook_status inlineUnHook(uint32_t target_addr);
void inlineUnHookAll();
enum inline_hook_status inlineHook(uint32_t target_addr);
void inlineHookAll();



#endif //CLASSPATCH_INLINEHOOK_H

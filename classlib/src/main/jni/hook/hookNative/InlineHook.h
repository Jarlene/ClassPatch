//
// Created by Administrator on 2016/5/20.
//

#ifndef CLASSPATCH_INLINEHOOK_H
#define CLASSPATCH_INLINEHOOK_H

#include <stdio.h>

enum inline_status {
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

enum inline_status registerInlineHook(uint32_t target_addr, uint32_t new_addr, uint32_t **proto_addr);
enum inline_status inlineUnHook(uint32_t target_addr);
void inlineUnHookAll();
enum inline_status inlineHook(uint32_t target_addr);
void inlineHookAll();


#endif //CLASSPATCH_INLINEHOOK_H

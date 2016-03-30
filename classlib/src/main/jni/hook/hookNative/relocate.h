#ifndef _RELOCATE_H
#define _RELOCATE_H

#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
void relocateInstruction(uint32_t target_addr, void *orig_instructions, int length, void *trampoline_instructions, int *orig_boundaries, int *trampoline_boundaries, int *count);
#ifdef __cplusplus
};
#endif
#endif
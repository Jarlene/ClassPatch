//
// Created by Jarlene on 2015/11/24.
//

#ifndef CLASSPATCH_NATIVEHOOK_H
#define CLASSPATCH_NATIVEHOOK_H

#include <stdio.h>
#include <stdlib.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <android/log.h>
#include <sys/syscall.h>

#define LOG_TAG "NativeHook"

#define PAGE_START(addr, size) ~((size) - 1) & (addr)
#define SOINFO_NAME_LEN 128


typedef void (*linker_function_t)();

typedef struct link_map_t {
    uintptr_t l_addr;
    char*  l_name;
    uintptr_t l_ld;
    struct link_map_t* l_next;
    struct link_map_t* l_prev;
} link_map_t;

typedef struct soinfo {
    char name[SOINFO_NAME_LEN];
    const Elf_Phdr* phdr;
    size_t phnum;
    Elf_Addr entry;
    Elf_Addr base;
    unsigned size;

    uint32_t unused1;  // DO NOT USE, maintained for compatibility.

    Elf_Dyn* dynamic;

    uint32_t unused2; // DO NOT USE, maintained for compatibility
    uint32_t unused3; // DO NOT USE, maintained for compatibility

    struct soinfo* next;

    unsigned flags;

    const char* strtab;
    Elf_Sym* symtab;
    size_t nbucket;
    size_t nchain;
    unsigned* bucket;
    unsigned* chain;

    //------------------

    // This is only used by 32-bit MIPS, but needs to be here for
    // all 32-bit architectures to preserve binary compatibility.
    unsigned* plt_got;

    Elf_Rel* plt_rel;
    size_t plt_rel_count;

    Elf_Rel* rel;
    size_t rel_count;

    linker_function_t* preinit_array;
    size_t preinit_array_count;

    linker_function_t* init_array;
    size_t init_array_count;
    linker_function_t* fini_array;
    size_t fini_array_count;

    linker_function_t init_func;
    linker_function_t fini_func;

    // ARM EABI section used for stack unwinding.
    unsigned* ARM_exidx;
    size_t ARM_exidx_count;

    size_t ref_count;
    link_map_t link_map;

    int constructors_called;

    // When you read a virtual address from the ELF file, add this
    // value to get the corresponding address in the process' address space.
    Elf_Addr load_bias;

} soinfo;



void *elfLoadLibrary(const char *libname);
void *elfHookSymbol(void * libhandle, const char *symbolname, void *hookfunc);



#endif //CLASSPATCH_NATIVEHOOK_H

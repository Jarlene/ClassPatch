//
// Created by Jarlene on 2015/11/24.
//

#include "NativeHook.h"

#include <dlfcn.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <elf.h>
#include "../../base/log.h"


static Elf_Sym* lookup_symbol(soinfo *si, const char *symbolname) {
    Elf_Sym* symtab = si->symtab;
    const char *strtab = si->strtab;

    for(unsigned int i=1; i < (si->nchain); i++) {
        Elf_Sym *s = symtab + i;
        const char *str = strtab + s->st_name;
        LOGD("the symbol name  is %s", str);
        if (!strcmp(str, symbolname)) {
            LOGD("Found symbol %s @ index %d with offset %x\n", str, i, s->st_value);
            return s;
        }
    }
    return NULL;
}

void *elfLoadLibrary(const char *libname) {
    void *handle = dlopen(libname, RTLD_GLOBAL);
    if (!handle) {
        LOGD("Failed to load lib %s\n", libname);
    }

    return handle;
}

void *elfHookSymbol(void * libhandle, const char *symbolname, void *hookfunc) {
    soinfo *si = (soinfo *)libhandle;
    if (!si) {
        LOGD("Invalid handle\n");
        return NULL;
    }
    Elf_Sym* symbol = lookup_symbol(si, symbolname);
    if (!symbol) {
        LOGD("Failed to find symbol %s\n", symbolname);
        return NULL;
    }
    LOGD("so bias addr is %p, symbol off is %p",si->load_bias,symbol->st_value);
    Elf32_Word originalfunc = si->load_bias + symbol->st_value ;
    LOGD("the original func addr is %p", originalfunc);

    if (originalfunc == (Elf32_Word)hookfunc) {
        LOGD("Symbol %s already patched\n", symbolname);
        return NULL;
    }

    if (mprotect((void *)si->base, si->size, PROT_READ|PROT_WRITE|PROT_EXEC)) {
        LOGD("Failed to make symbol table writable: %s\n", strerror(errno));
    }
    LOGD("hookfunc addr is %p",hookfunc);
    Elf32_Word hookoffset = (uint32_t)hookfunc - si->load_bias;
    symbol->st_value = hookoffset;
    mprotect((void *)si->base, si->size, PROT_READ);
    return (void *)originalfunc;

}
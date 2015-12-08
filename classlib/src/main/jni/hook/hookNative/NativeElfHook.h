// 针对Native Method进行Elf Hook。
// Created by Jarlene on 2015/11/27.
//

#ifndef CLASSPATCH_NATIVEELFHOOK_H
#define CLASSPATCH_NATIVEELFHOOK_H

#include <sys/types.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "Linker.h"
#include "../../base/log.h"


#define ORIGINAL( TYPENAME, ... ) \
    ((TYPENAME ## _t)find_original( #TYPENAME ))( __VA_ARGS__ )

#define DEFINEHOOK( RET_TYPE, NAME, ARGS ) \
    typedef RET_TYPE (* NAME ## _t)ARGS; \
    RET_TYPE hook_ ## NAME ARGS

#define ADDHOOK( NAME ) \
    { #NAME, 0, (uintptr_t)&hook_ ## NAME }

typedef struct LdModule {
    uintptr_t   address;
    std::string name;
    LdModule( uintptr_t a, const std::string& n ) : address(a), name(n) {
    }
} elfLDModule;

typedef std::vector<elfLDModule> elfLDModuleMap;

unsigned addElfHook(const char* soName, const char* symbol, void* replaceFunc);

#endif //CLASSPATCH_NATIVEELFHOOK_H

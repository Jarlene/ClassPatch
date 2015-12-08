//
// Created by Jarlene on 2015/11/27.
//

#include "NativeElfHook.h"
#include <sys/mman.h>

#define LOG_TAG "NativeElfHook"


static unsigned elfHash(const char *_name) {
    const unsigned char *name = (const unsigned char *) _name;
    unsigned h = 0, g;
    while(*name) {
        h = (h << 4) + *name++;
        g = h & 0xf0000000;
        h ^= g;
        h ^= g >> 24;
    }
    return h;
}

static Elf32_Sym *  soInfoElfLookUp(struct SoInfo *si, unsigned hash, const char *name) {
    Elf32_Sym *symtab = si->symtab;
    const char *strtab = si->strtab;
    unsigned n;

    for( n = si->bucket[hash % si->nbucket]; n != 0; n = si->chain[n] ) {
        Elf32_Sym *s = symtab + n;
        const char *methodName = strtab + s->st_name;
        LOGD("symbol name is %s", methodName);
        if( strcmp(methodName, name) == 0 ) {
            return s;
        }
    }
    return NULL;
}

//elfLDModule getHookedMap() {
//    elfLDModule modules;
//    char buffer[1024] = {0};
//    uintptr_t address;
//    std::string name;
//
//    FILE *fp = fopen( "/proc/self/maps", "rt" );
//    if( fp == NULL ){
//        perror("fopen");
//        goto done;
//    }
//
//    while( fgets( buffer, sizeof(buffer), fp ) ) {
//        if( strstr( buffer, "r-xp" ) ){
//            address = (uintptr_t)strtoul( buffer, NULL, 16 );
//            name    = strrchr( buffer, ' ' ) + 1;
//            name.resize( name.size() - 1 );
//
//            modules.push_back( ld_module_t( address, name ) );
//        }
//    }
//
//    done:
//
//    if(fp){
//        fclose(fp);
//    }
//
//    return modules;
//}

unsigned patchAddress( unsigned addr, unsigned newval ) {
    unsigned original = -1;
    size_t pagesize = sysconf(_SC_PAGESIZE);
    const void *aligned_pointer = (const void*)(addr & ~(pagesize - 1));

    mprotect(aligned_pointer, pagesize, PROT_WRITE | PROT_READ | PROT_EXEC);

    original = *(unsigned *)addr;
    *((unsigned*)addr) = newval;

    mprotect(aligned_pointer, pagesize, PROT_READ);

    return original;
}

unsigned addElfHook(const char* soName, const char* symbol, void* replaceFunc) {
    struct SoInfo *si = NULL;
    Elf32_Rel *rel = NULL;
    Elf32_Sym *s = NULL;
    unsigned int sym_offset = 0;
    size_t i;

    // since we know the module is already loaded and mostly
    // we DO NOT want its constructors to be called again,
    // ise RTLD_NOLOAD to just get its soinfo address.
    si = (struct SoInfo *)dlopen( soName, RTLD_GLOBAL/* RTLD_NOLOAD */ );
    if( !si ){
        LOGE( "dlopen error: %s.", dlerror() );
        return 0;
    }
    s = soInfoElfLookUp( si, elfHash(symbol), symbol );
    if( !s ){
        return 0;
    }
    sym_offset = s - si->symtab;
    LOGD("the sym offset is %d", sym_offset);

    // loop reloc table to find the symbol by index
    for( i = 0, rel = si->plt_rel; i < si->plt_rel_count; ++i, ++rel ) {
        unsigned type  = ELF32_R_TYPE(rel->r_info);
        unsigned sym   = ELF32_R_SYM(rel->r_info);
        unsigned reloc = (unsigned)(rel->r_offset + si->base);

        if( sym_offset == sym ) {
            switch(type) {
                case R_ARM_JUMP_SLOT:
                    return patchAddress( reloc, replaceFunc );
                default:
                    LOGE( "Expected R_ARM_JUMP_SLOT, found 0x%X", type );
            }
        }
    }

    unsigned original = 0;

    // loop dyn reloc table
    for( i = 0, rel = si->rel; i < si->rel_count; ++i, ++rel ) {
        unsigned type  = ELF32_R_TYPE(rel->r_info);
        unsigned sym   = ELF32_R_SYM(rel->r_info);
        unsigned reloc = (unsigned)(rel->r_offset + si->base);
        if( sym_offset == sym ) {
            switch(type) {
                case R_ARM_ABS32:
                case R_ARM_GLOB_DAT:
                    original = patchAddress( reloc, replaceFunc );
                default:
                    LOGE( "Expected R_ARM_ABS32 or R_ARM_GLOB_DAT, found 0x%X", type );
            }
        }
    }

    if( original == 0 ){
        LOGE( "Unable to find symbol in the reloc tables ( plt_rel_count=%u - rel_count=%u ).", si->plt_rel_count, si->rel_count );
    }

    return original;
}
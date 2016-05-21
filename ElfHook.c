#include <string.h>
#include <jni.h>
#include <android/log.h>
#include <stdint.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <dirent.h>
#include <elf.h>
#include <errno.h>
#include <sys/ioctl.h>

/**
 * elf关键信息，地址均为绝对地址
 */
typedef struct {
//	const ElfHandle *handle;
	/**
	 * 在内存中的基地址
	 */
	uint8_t *elf_base;
	/**
	 * 头信息
	 */
	Elf32_Ehdr *ehdr;
	/**
	 * 程序头信息
	 */
	Elf32_Phdr *phdr;
	/**
	 * 段表
	 */
	Elf32_Shdr *shdr;
	/***
	 * .dynamic表地址
	 */
	Elf32_Dyn *dyn;
	/***
	 * .dynamic表大小
	 */
	Elf32_Word dynsz;
	/**
	 * 符号表
	 */
	Elf32_Sym *sym;
	/**
	 * 符号表大小
	 */
	Elf32_Word symsz;

	/**
	 * 函数重定位表
	 */
	Elf32_Rel *relplt;
	/**
	 * 函数重定位表大小
	 */
	Elf32_Word relpltsz;
	/**
	 * 数据重定位表
	 */
	Elf32_Rel *reldyn;
	/**
	 * 数据重定位表大小
	 */
	Elf32_Word reldynsz;

	uint32_t nbucket;
	uint32_t nchain;

	uint32_t *bucket;
	uint32_t *chain;

	const char *shstr;
	/**
	 * 字符串表地址
	 */
	const char *symstr;
} ElfInfo;

#define  LOG_TAG    "Hook"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

/**
 * 查找soname的基址，如果为NULL，则为当前进程基址
 */
void *findLibBase(const char *soname) {
	FILE *fd = fopen("/proc/self/maps", "r");
	char line[256];
	void *base = 0;

	while (fgets(line, sizeof(line), fd) != NULL) {
		if (soname == NULL || strstr(line, soname)) {
			line[8] = '\0';
			base = (void *) strtoul(line, NULL, 16);
			break;
		}
	}

	fclose(fd);
	return base;
}

void printfSymbols(Elf32_Sym *sym, char * strtab, int symsz) {
//	Elf32_Sym *sym = info.sym;
	LOGI("symbol table info:\n");
	int i;
	for (i = 0; i < symsz; i++) {
		LOGI("[%2d] %-6p %-20s\n", i, sym[i].st_value, sym[i].st_name + strtab);
	}
}

void findSym(Elf32_Sym *sym, char * strtab, int symsz, char *sysName,
		int *sysIdx) {
	int i;
	for (i = 0; i < symsz; i++) {
//		LOGI("[%2d] %-6p %-20s\n", i, sym[i].st_value, sym[i].st_name + strtab);
		if (!strcmp(sym[i].st_name + strtab, sysName)) {
			LOGE("sys found!!!! Idx: %d", i);
			*sysIdx = i;
			return;
		}
	}
	*sysIdx = -1;

}

int findRel(Elf32_Rel*reldyn, Elf32_Rel* relplt, int reldynsz, int relpltsz,
		Elf32_Sym *sym, char * strtab, char * fun_name) {
	int j;
	for (j = 0; j < relpltsz; j++) {
		const char *name = sym[ELF32_R_SYM(relplt[j].r_info)].st_name + strtab;
		if (!strcmp(name, fun_name)) {
			LOGE("fun_name found!!!! Idx: %d", j);
			LOGE("fun_name found!!!! offset: %p", relplt[j].r_offset);
			LOGE(
					"fun_name found!!!! Idx in strtab: %d", ELF32_R_SYM(relplt[j].r_info));
			return j;

//					*sysIdx = i;
//			break;
		}
//		LOGI(
//				"[%.2d-%.4d] 0x%-.8x 0x%-.8x %-10s\n", i, j, rel[j].r_offset, rel[j].r_info, name);
	}
	return -1;

}

void printfRelInfo(Elf32_Rel*reldyn, Elf32_Rel* relplt, int reldynsz,
		int relpltsz, Elf32_Sym *sym, char * strtab) {
	Elf32_Rel* rels[] = { reldyn, relplt };
	Elf32_Word resszs[] = { reldynsz, relpltsz };

	LOGI("rel section info:\n");
	int i, j;
	for (i = 0; i < sizeof(rels) / sizeof(rels[0]); i++) {
		Elf32_Rel *rel = rels[i];
		Elf32_Word relsz = resszs[i];

		for (j = 0; j < relsz; j++) {
			const char *name = sym[ELF32_R_SYM(rel[j].r_info)].st_name + strtab;
			LOGI(
					"[%.2d-%.4d] 0x%-.8x 0x%-.8x %-10s\n", i, j, rel[j].r_offset, rel[j].r_info, name);
		}
	}
}

void printSymInfoFromElfInfo(ElfInfo *elfInfo) {
	/**
	 * symtab首地址
	 */
	Elf32_Sym *symtab = elfInfo->sym;
	/**
	 * symtab大小
	 */
	int symtabsz = elfInfo->symsz;
	/**
	 * strtab首地址
	 */
	char *strtab = elfInfo->symstr;
	printfSymbols(symtab, strtab, symtabsz);
}

void printRelInfoFromElfInfo(ElfInfo *elfInfo) {
	/**
	 * symtab首地址
	 */
	Elf32_Sym *symtab = elfInfo->sym;
	/**
	 * symtab大小
	 */
	int symtabsz = elfInfo->symsz;
	/**
	 * strtab首地址
	 */
	char *strtab = elfInfo->symstr;
	Elf32_Rel * relplt = elfInfo->relplt;
	Elf32_Rel * reldyn = elfInfo->reldyn;
	int relpltsz = elfInfo->relpltsz;
	int reldynsz = elfInfo->reldynsz;

	printfRelInfo(reldyn, relplt, reldynsz, relpltsz, symtab, strtab);
}

unsigned getSymAddr(void * module_base, char * sym_name) {
	Elf32_Ehdr *si = (Elf32_Ehdr *) malloc(sizeof(Elf32_Ehdr));
	//Section Header
	Elf32_Shdr *shdr = (Elf32_Shdr *) malloc(sizeof(Elf32_Shdr));
	Elf32_Phdr *phdr;
//	Elf32_Shdr *shdr;
	Elf32_Rel *rel = NULL;
	unsigned int sym_offset = 0;
	int i;

	si = (Elf32_Ehdr*) module_base;
	LOGI("si addr: %p", si);
	LOGI("module Base: %p", module_base);
	if (!si) {
		LOGE("can not get so info!!!");
		return;
	}
	int shnum = si->e_shnum;
	/**
	 * segment数量
	 */
	int phnum = si->e_phnum;
	LOGI("e_shnum: %d", shnum);
	LOGI("p_shnum: %d", phnum);
	unsigned long shdr_addr = si->e_shoff;

	LOGD("e_shoff: %p", shdr_addr);
//	LOGD("e_type: %s", si->e_type);
	LOGD("e_enty: %p", si->e_entry);
	LOGD("e_phoff: %p", si->e_phoff);
	/**
	 * segment描述符大小
	 */
	int phentsize = si->e_phentsize;
	LOGI("phentsize: %d", phentsize);
	//程序头地址
	phdr = (Elf32_Phdr *) (si->e_phoff + module_base);
//	void* phaddr = (si->e_phoff + module_base);
	/**
	 * dynamic的长度
	 */
	int dynsz;
	for (i = 0; i < phnum; i++) {
//		phdr = (Elf32_Phdr *) phaddr;
//		LOGI("*******************************************");
		if (phdr->p_type == PT_DYNAMIC) {
			LOGI("find dynamic!!!");
//			LOGD("dynamic Segment%d内存地址: %p", i+1, phdr);
			LOGD("dynamic Segment%d内存地址: %p", i+1, phdr);
			LOGD("dynamic Segment%d类型: %d", i+1, phdr->p_type);
			LOGD("dynamic Segment%d文件偏移地址: %p", i+1, phdr->p_offset);
			LOGD("dynamic Segment%d虚拟偏移地址: %p", i+1, phdr->p_vaddr);
			LOGD("dynamic Segment%d物理偏移地址: %p", i+1, phdr->p_paddr);
			LOGD("dynamic Segment%d虚拟地址大小: %p", i+1, phdr->p_memsz);
			dynsz = phdr->p_memsz / sizeof(Elf32_Dyn);
			LOGD("dynamic size: %d", dynsz);
			break;
		}
		phdr++;
//		phaddr = phaddr + phentsize;
	}
	/**
	 * 得到dynamic首地址
	 */
	Elf32_Dyn *dyn = (Elf32_Dyn *) (phdr->p_vaddr + module_base);
	/**
	 * symtab首地址
	 */
	Elf32_Sym *symtab;
	/**
	 * symtab大小
	 */
	int symtabsz;
	/**
	 * strtab首地址
	 */
	char *strtab;
	Elf32_Rel * relplt;
	Elf32_Rel * reldyn;
	int relpltsz;
	int reldynsz;
	for (i = 0; i < dynsz; i++, dyn++) {
		switch (dyn->d_tag) {
		case DT_SYMTAB:
			LOGI("DT_SYMTAB found, addr: %p", module_base+ dyn->d_un.d_ptr);
			symtab = (Elf32_Sym*) (module_base + dyn->d_un.d_ptr);
			break;
		case DT_STRTAB:
			LOGI("DT_STRTAB found, addr: %p", module_base+ dyn->d_un.d_ptr);
			strtab = (char *) (module_base + dyn->d_un.d_ptr);
			break;
		case DT_REL:
			LOGI("DT_REL found, addr: %p", module_base+ dyn->d_un.d_ptr);
			reldyn = (Elf32_Rel *) (module_base + dyn->d_un.d_ptr);
			break;

		case DT_RELSZ:
			LOGI("DT_RELSZ found, addr: %p", module_base+ dyn->d_un.d_ptr);
			reldynsz = dyn->d_un.d_val / sizeof(Elf32_Rel);
			LOGI("reldyn size: %d", reldynsz);
			break;

		case DT_JMPREL:
			LOGI("DT_JMPREL found, addr: %p", module_base+ dyn->d_un.d_ptr);
			relplt = (Elf32_Rel *) (module_base + dyn->d_un.d_ptr);
			break;

		case DT_PLTRELSZ:
			LOGI("DT_PLTRELSZ found, addr: %p", module_base+ dyn->d_un.d_ptr);
			relpltsz = dyn->d_un.d_val / sizeof(Elf32_Rel);
			LOGI("relplt size: %d", relpltsz);
			break;

		case DT_HASH:
			LOGI("DT_HASH found, addr: %p", module_base+ dyn->d_un.d_ptr);
			uint32_t *rawdata = (uint32_t *) (module_base + dyn->d_un.d_ptr);
//			info.nbucket = rawdata[0];
//			info.nchain = rawdata[1];
			symtabsz = rawdata[1];
			LOGI("systab size: %d", rawdata[1]);

//			info.bucket = rawdata + 2;
//			info.chain = info.bucket + info.nbucket;

//			info.symsz = info.nchain;
			break;
		}
	}

	printfSymbols(symtab, strtab, symtabsz);

//	printfRelInfo(reldyn, relplt, reldynsz, relpltsz, symtab, strtab);
//	char *fun1 = "Java_com_example_myndk_MainActivity_stringFromJNI";
//	char *fun2 = "Java_com_example_myndk_MainActivity_stringFromJNI2";
//	char *fun3 = "memcpy";
//	findRel(reldyn, relplt, reldynsz, relpltsz, symtab, strtab, fun3);

//********************************************************
	int fun1_Idx;
//	int fun2_Idx;
//
	findSym(symtab, strtab, symtabsz, sym_name, &fun1_Idx);
//	findSym(symtab, strtab, symtabsz, fun2, &fun2_Idx);
//
	LOGE("sym1: %p", symtab+fun1_Idx);
	LOGE("sym1 addr: %p", (symtab+fun1_Idx)->st_value);

	return (symtab + fun1_Idx)->st_value + module_base;

}

void * getFunAddr(void * module_base, char * fun_name) {
	Elf32_Ehdr *si = (Elf32_Ehdr *) malloc(sizeof(Elf32_Ehdr));
	//Section Header
	Elf32_Shdr *shdr = (Elf32_Shdr *) malloc(sizeof(Elf32_Shdr));
	Elf32_Phdr *phdr;
//	Elf32_Shdr *shdr;
	Elf32_Rel *rel = NULL;
	unsigned int sym_offset = 0;
	int i;

	si = (Elf32_Ehdr*) module_base;
	LOGI("si addr: %p", si);
	LOGI("module Base: %p", module_base);
	if (!si) {
		LOGE("can not get so info!!!");
		return;
	}
	int shnum = si->e_shnum;
	/**
	 * segment数量
	 */
	int phnum = si->e_phnum;
	LOGI("e_shnum: %d", shnum);
	LOGI("p_shnum: %d", phnum);
	unsigned long shdr_addr = si->e_shoff;

	LOGD("e_shoff: %p", shdr_addr);
//	LOGD("e_type: %s", si->e_type);
	LOGD("e_enty: %p", si->e_entry);
	LOGD("e_phoff: %p", si->e_phoff);
	/**
	 * segment描述符大小
	 */
	int phentsize = si->e_phentsize;
	LOGI("phentsize: %d", phentsize);
	//程序头地址
	phdr = (Elf32_Phdr *) (si->e_phoff + module_base);
//	void* phaddr = (si->e_phoff + module_base);
	/**
	 * dynamic的长度
	 */
	int dynsz;
	for (i = 0; i < phnum; i++) {
//		phdr = (Elf32_Phdr *) phaddr;
//		LOGI("*******************************************");
		if (phdr->p_type == PT_DYNAMIC) {
			LOGI("find dynamic!!!");
//			LOGD("dynamic Segment%d内存地址: %p", i+1, phdr);
			LOGD("dynamic Segment%d内存地址: %p", i+1, phdr);
			LOGD("dynamic Segment%d类型: %d", i+1, phdr->p_type);
			LOGD("dynamic Segment%d文件偏移地址: %p", i+1, phdr->p_offset);
			LOGD("dynamic Segment%d虚拟偏移地址: %p", i+1, phdr->p_vaddr);
			LOGD("dynamic Segment%d物理偏移地址: %p", i+1, phdr->p_paddr);
			LOGD("dynamic Segment%d虚拟地址大小: %p", i+1, phdr->p_memsz);
			dynsz = phdr->p_memsz / sizeof(Elf32_Dyn);
			LOGD("dynamic size: %d", dynsz);
			break;
		}
		phdr++;
//		phaddr = phaddr + phentsize;
	}
	/**
	 * 得到dynamic首地址
	 */
	Elf32_Dyn *dyn = (Elf32_Dyn *) (phdr->p_vaddr + module_base);
	/**
	 * symtab首地址
	 */
	Elf32_Sym *symtab;
	/**
	 * symtab大小
	 */
	int symtabsz;
	/**
	 * strtab首地址
	 */
	char *strtab;
	Elf32_Rel * relplt;
	Elf32_Rel * reldyn;
	int relpltsz;
	int reldynsz;
	for (i = 0; i < dynsz; i++, dyn++) {
		switch (dyn->d_tag) {
		case DT_SYMTAB:
			LOGI("DT_SYMTAB found, addr: %p", module_base+ dyn->d_un.d_ptr);
			symtab = (Elf32_Sym*) (module_base + dyn->d_un.d_ptr);
			break;
		case DT_STRTAB:
			LOGI("DT_STRTAB found, addr: %p", module_base+ dyn->d_un.d_ptr);
			strtab = (char *) (module_base + dyn->d_un.d_ptr);
			break;
		case DT_REL:
			LOGI("DT_REL found, addr: %p", module_base+ dyn->d_un.d_ptr);
			reldyn = (Elf32_Rel *) (module_base + dyn->d_un.d_ptr);
			break;

		case DT_RELSZ:
			LOGI("DT_RELSZ found, addr: %p", module_base+ dyn->d_un.d_ptr);
			reldynsz = dyn->d_un.d_val / sizeof(Elf32_Rel);
			LOGI("reldyn size: %d", reldynsz);
			break;

		case DT_JMPREL:
			LOGI("DT_JMPREL found, addr: %p", module_base+ dyn->d_un.d_ptr);
			relplt = (Elf32_Rel *) (module_base + dyn->d_un.d_ptr);
			break;

		case DT_PLTRELSZ:
			LOGI("DT_PLTRELSZ found, addr: %p", module_base+ dyn->d_un.d_ptr);
			relpltsz = dyn->d_un.d_val / sizeof(Elf32_Rel);
			LOGI("relplt size: %d", relpltsz);
			break;

		case DT_HASH:
			LOGI("DT_HASH found, addr: %p", module_base+ dyn->d_un.d_ptr);
			uint32_t *rawdata = (uint32_t *) (module_base + dyn->d_un.d_ptr);
//			info.nbucket = rawdata[0];
//			info.nchain = rawdata[1];
			symtabsz = rawdata[1];
			LOGI("systab size: %d", rawdata[1]);

//			info.bucket = rawdata + 2;
//			info.chain = info.bucket + info.nbucket;

//			info.symsz = info.nchain;
			break;
		}
	}

//	printfSymbols(symtab, strtab, symtabsz);

	printfRelInfo(reldyn, relplt, reldynsz, relpltsz, symtab, strtab);
	LOGD("-------------------------------------------------------------");
	//绝对地址
	int idx = findRel(reldyn, relplt, reldynsz, relpltsz, symtab, strtab,
			fun_name);
	LOGD("-------------------------------------------------------------");

	LOGI("relplt_addr: %p", relplt[idx]);
	LOGI("relplt_addr offset: %p", relplt[idx].r_offset);

	void * relplt_addr;

	//跳转地址
	relplt_addr = relplt[idx].r_offset + module_base;
	LOGI("relplt_addr: %p", relplt_addr);
	return relplt_addr;

}

/**
 * 将so1中的fun1替换为so2的fun2
 */
void getElfInfo(ElfInfo *elfInfo) {
	void *module_base = elfInfo->elf_base;
	Elf32_Ehdr *si = (Elf32_Ehdr *) malloc(sizeof(Elf32_Ehdr));
	//Section Header
	Elf32_Shdr *shdr = (Elf32_Shdr *) malloc(sizeof(Elf32_Shdr));
	Elf32_Phdr *phdr;
	Elf32_Rel *rel = NULL;
	unsigned int sym_offset = 0;
	int i;

	si = (Elf32_Ehdr*) module_base;
	elfInfo->ehdr = si;
	LOGI("si addr: %p", si);
	LOGI("module Base: %p", module_base);
	if (!si) {
		LOGE("can not get so info!!!");
		return;
	}
	int shnum = si->e_shnum;
	/**
	 * segment数量
	 */
	int phnum = si->e_phnum;
	LOGI("e_shnum: %d", shnum);
	LOGI("p_shnum: %d", phnum);
	unsigned long shdr_addr = si->e_shoff;
	elfInfo->shdr = (Elf32_Shdr *) (shdr_addr + module_base);
	LOGD("e_shoff: %p", shdr_addr);
//	LOGD("e_type: %s", si->e_type);
	LOGD("e_enty: %p", si->e_entry);
	LOGD("e_phoff: %p", si->e_phoff);
	/**
	 * segment描述符大小
	 */
	int phentsize = si->e_phentsize;
	LOGI("phentsize: %d", phentsize);
	//程序头地址
	phdr = (Elf32_Phdr *) (si->e_phoff + module_base);
	elfInfo->phdr = phdr;
//	void* phaddr = (si->e_phoff + module_base);
	/**
	 * dynamic的长度
	 */
	int dynsz;
	for (i = 0; i < phnum; i++) {
//		phdr = (Elf32_Phdr *) phaddr;
//		LOGI("*******************************************");
		if (phdr->p_type == PT_DYNAMIC) {
			LOGI("find dynamic!!!");
//			LOGD("dynamic Segment%d内存地址: %p", i+1, phdr);
			LOGD("dynamic Segment%d内存地址: %p", i+1, phdr);
			LOGD("dynamic Segment%d类型: %d", i+1, phdr->p_type);
			LOGD("dynamic Segment%d文件偏移地址: %p", i+1, phdr->p_offset);
			LOGD("dynamic Segment%d虚拟偏移地址: %p", i+1, phdr->p_vaddr);
			LOGD("dynamic Segment%d物理偏移地址: %p", i+1, phdr->p_paddr);
			LOGD("dynamic Segment%d虚拟地址大小: %p", i+1, phdr->p_memsz);
			dynsz = phdr->p_memsz / sizeof(Elf32_Dyn);
			LOGD("dynamic size: %d", dynsz);
			break;
		}
		phdr++;
//		phaddr = phaddr + phentsize;
	}
	/**
	 * 得到dynamic首地址
	 */
	Elf32_Dyn *dyn = (Elf32_Dyn *) (phdr->p_vaddr + module_base);
	elfInfo->dyn = dyn;
	elfInfo->dynsz = dynsz;
	/**
	 * symtab首地址
	 */
	Elf32_Sym *symtab;
	/**
	 * symtab大小
	 */
	int symtabsz;
	/**
	 * 字符串表strtab首地址
	 */
	char *strtab;
	Elf32_Rel * relplt;
	Elf32_Rel * reldyn;
	int relpltsz;
	int reldynsz;
	for (i = 0; i < dynsz; i++, dyn++) {
		switch (dyn->d_tag) {
		case DT_SYMTAB:
			LOGI("DT_SYMTAB found, addr: %p", module_base+ dyn->d_un.d_ptr);
			symtab = (Elf32_Sym*) (module_base + dyn->d_un.d_ptr);
			elfInfo->sym = symtab;
			break;
		case DT_STRTAB:
			LOGI("DT_STRTAB found, addr: %p", module_base+ dyn->d_un.d_ptr);
			strtab = (char *) (module_base + dyn->d_un.d_ptr);
			elfInfo->symstr = strtab;
			break;
		case DT_REL:
			LOGI("DT_REL found, addr: %p", module_base+ dyn->d_un.d_ptr);
			reldyn = (Elf32_Rel *) (module_base + dyn->d_un.d_ptr);
			elfInfo->reldyn = reldyn;
			break;

		case DT_RELSZ:
			LOGI("DT_RELSZ found, addr: %p", module_base+ dyn->d_un.d_ptr);
			reldynsz = dyn->d_un.d_val / sizeof(Elf32_Rel);
			LOGI("reldyn size: %d", reldynsz);
			elfInfo->reldynsz = reldynsz;
			break;

		case DT_JMPREL:
			LOGI("DT_JMPREL found, addr: %p", module_base+ dyn->d_un.d_ptr);
			relplt = (Elf32_Rel *) (module_base + dyn->d_un.d_ptr);
			elfInfo->relplt = relplt;
			break;

		case DT_PLTRELSZ:
			LOGI("DT_PLTRELSZ found, addr: %p", module_base+ dyn->d_un.d_ptr);
			relpltsz = dyn->d_un.d_val / sizeof(Elf32_Rel);
			LOGI("relplt size: %d", relpltsz);
			elfInfo->relpltsz = relpltsz;
			break;

		case DT_HASH:
			LOGI("DT_HASH found, addr: %p", module_base+ dyn->d_un.d_ptr);
			uint32_t *rawdata = (uint32_t *) (module_base + dyn->d_un.d_ptr);
//			info.nbucket = rawdata[0];
//			info.nchain = rawdata[1];
			symtabsz = rawdata[1];
			elfInfo->symsz = symtabsz;
			LOGI("systab size: %d", rawdata[1]);

//			info.bucket = rawdata + 2;
//			info.chain = info.bucket + info.nbucket;

//			info.symsz = info.nchain;
			break;
		}
	}

}

unsigned getSymAddrFromElfInfo(ElfInfo *elfInfo, char *symName) {
	void * module_base = elfInfo->elf_base;
	/**
	 * symtab首地址
	 */
	Elf32_Sym *symtab = elfInfo->sym;
	/**
	 * symtab大小
	 */
	int symtabsz = elfInfo->symsz;
	/**
	 * strtab首地址
	 */
	char *strtab = elfInfo->symstr;
	printfSymbols(symtab, strtab, symtabsz);
//********************************************************
	int fun1_Idx = -1;
//	int fun2_Idx;
//
	findSym(symtab, strtab, symtabsz, symName, &fun1_Idx);
	if (fun1_Idx == -1) {
		LOGE("can not get sym idx");
		return 0;
	}
	LOGE("sym1: %p", symtab+fun1_Idx);
	LOGE("sym1 addr: %p", (symtab+fun1_Idx)->st_value);

	return (symtab + fun1_Idx)->st_value + module_base;

}

unsigned getFunAddrFromElfInfo(ElfInfo *elfInfo, char *funName) {
	void * module_base = elfInfo->elf_base;
	/**
	 * symtab首地址
	 */
	Elf32_Sym *symtab = elfInfo->sym;
	/**
	 * symtab大小
	 */
	int symtabsz = elfInfo->symsz;
	/**
	 * strtab首地址
	 */
	char *strtab = elfInfo->symstr;
	Elf32_Rel * relplt = elfInfo->relplt;
	Elf32_Rel * reldyn = elfInfo->reldyn;
	int relpltsz = elfInfo->relpltsz;
	int reldynsz = elfInfo->reldynsz;

	printfRelInfo(reldyn, relplt, reldynsz, relpltsz, symtab, strtab);
	LOGD("-------------------------------------------------------------");
	//绝对地址
	int idx = findRel(reldyn, relplt, reldynsz, relpltsz, symtab, strtab,
			funName);
	if (idx == -1) {
		LOGE("can not find fun idx!");
		return 0;
	}
	LOGD("-------------------------------------------------------------");
	LOGI("relplt_addr: %p", relplt[idx]);
	LOGI("relplt_addr offset: %p", relplt[idx].r_offset);

	void * relplt_addr;

	//跳转地址
	relplt_addr = relplt[idx].r_offset + module_base;
	LOGI("relplt_addr: %p", relplt_addr);
	return relplt_addr;

}

jint replaceFun(unsigned sym_addr, size_t * fun_addr) {
	int pagesize = sysconf(_SC_PAGE_SIZE);
	if (pagesize == -1)
		LOGE("sysconf get pagesize error!!!!!!!!!!");
	LOGD("pagesize: %d", pagesize);

	//成功返回值为0
	if (!mprotect(
			(void *) (((size_t) fun_addr) & (((size_t) - 1) ^ (pagesize - 1))),
			pagesize, PROT_READ | PROT_WRITE)) {
		LOGE("mptotect success!!");
	} else {
		LOGE("mptotect failed!!");
		return -1;
	}

	fun_addr = (size_t *) fun_addr;
	//替换地址
	*fun_addr = (size_t) sym_addr;

	if (!mprotect(
			(void *) (((size_t) fun_addr) & (((size_t) - 1) ^ (pagesize - 1))),
			pagesize, PROT_READ | PROT_EXEC)) {
		LOGE("mptotect success!!");
	} else {
		LOGE("mptotect failed!!");
		return -1;
	}
	return 1;
}

/**
 * so1_name, 目标so名称
 * so2_name，替换的so名称
 * fun1_name，目标函数名称，在so1_name中定义
 * fun2_name，替换的函数名称，在so2_name中定义
 */
int hookcall(char* so1_name, char* so2_name, char* fun1_name, char* fun2_name) {
	void * module1_base;
	void * module2_base;
	module1_base = findLibBase(so1_name);
	module2_base = findLibBase(so2_name);
	LOGE("module1 Base: %p", module1_base);
	LOGE("module2 Base: %p", module2_base);

	unsigned sym_addr = getSymAddr(module2_base, fun2_name);
	size_t * fun_addr = (size_t *) getFunAddr(module1_base, fun1_name);

	int pagesize = sysconf(_SC_PAGE_SIZE);
	if (pagesize == -1)
		LOGE("sysconf get pagesize error!!!!!!!!!!");
	LOGD("pagesize: %d", pagesize);
	//
	//成功返回值为0
	if (!mprotect(
			(void *) (((size_t) fun_addr) & (((size_t) - 1) ^ (pagesize - 1))),
			pagesize, PROT_READ | PROT_WRITE)) {
		LOGE("mptotect success!!");
	} else {
		LOGE("mptotect failed!!");
		return -1;
	}

	fun_addr = (size_t *) fun_addr;
	//替换地址
	*fun_addr = (size_t) sym_addr;

	if (!mprotect(
			(void *) (((size_t) fun_addr) & (((size_t) - 1) ^ (pagesize - 1))),
			pagesize, PROT_READ | PROT_EXEC)) {
		LOGE("mptotect success!!");
		return 1;
	} else {
		LOGE("mptotect failed!!");
		return -1;
	}
}

/**
 *targetSoName, 目标so名称
 *targetFunName, 目标函数名称
 *replaceFunAddr, 替换函数地址
 */
int elfHook(char *targetSoName, char * targetFunName, unsigned replaceFunAddr) {
	ElfInfo *targetElfInfo;
	ElfInfo *myElfInfo;
	targetElfInfo = (ElfInfo *) malloc(sizeof(ElfInfo));
	myElfInfo = (ElfInfo *) malloc(sizeof(ElfInfo));
	char * so2_Name = "libelfhook.so";

	LOGI("soName: %s", targetSoName);
	targetElfInfo->elf_base = findLibBase(targetSoName);
	if (targetElfInfo->elf_base == 0) {
		LOGE("can not get target So %s base addr", targetSoName);
		return -1;
	}
	LOGD("target so base addr: %p", targetElfInfo->elf_base);
	getElfInfo(targetElfInfo);

	size_t * fun_addr = (size_t *) getFunAddrFromElfInfo(targetElfInfo,
			targetFunName);
	if (fun_addr == 0) {
		LOGE("can not get fun %s addr!", targetFunName);
		return -1;
	}

	LOGE("replaceFunAddr: %p", replaceFunAddr);

	if (1 != replaceFun(replaceFunAddr, fun_addr)) {
		LOGE("replace fun failed!");
		return -1;
	}
	return 1;
}

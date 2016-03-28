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
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/ptrace.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <elf.h>
#include <android/log.h>
#include <errno.h>
//#include "hook.h"

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
#define TARGET_PATH "/system/bin/servicemanager"
#define LIB "/data/app-lib/com.example.myndk-2/libhello-jni.so"

jint Hook(JNIEnv* env, jobject thiz, jstring jstr, jobjectArray methodArray);
static JNINativeMethod gMethods[] = { { "Hook",
		"(Ljava/lang/String;[Ljava/lang/String;)I", (void*) Hook }, };

char *libPath;
const char *pkgName = "com.example.myndk";
const char *libName = "libhello-jni.so";

const char *libc_path = "/system/lib/libc.so";

int pid;

int my_memcpy(const void *s1, const void *s2, int count) {
	LOGE("***%s: p1=%p, p2=%p\n", __FUNCTION__, s1, s2);
	int nRet = memcmp(s1, s2, count);
	return nRet;
}

FILE * my_fopen(const char * path, const char * mode) {
	LOGE("***%s: path=%p, mode=%p\n", __FUNCTION__, path, mode);
	LOGE("***%s: path=%s, mode=%s\n", __FUNCTION__, path, mode);
	return fopen(path, mode);
}

int my_strlen(char * s) {
	LOGE("strlen hooked!!!");
	return strlen(s);
}

jint my_GetEnv(JavaVM *vm, void** env, jint version) {
	LOGE("GetEnv has been hooked!!!");
	return (*vm)->GetEnv(vm, env, version);
}
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

//获取目标进程pid中module所在内存位置
void* get_module_base(pid_t pid, const char* module_name) {

	LOGI("get_module_base agrs: Pid: %d;module: %s", pid, module_name);
	FILE *fp;
	long addr = 0;
	char *pch;
	char filename[32];
	char line[1024];
	/**
	 * 直接对proc文件进行读取，就可以得到虚拟内存映射
	 */
	if (pid < 0) {
		/* self process */
		snprintf(filename, sizeof(filename), "/proc/self/maps", pid);
	} else {
		snprintf(filename, sizeof(filename), "/proc/%d/maps", pid);
	}

	fp = fopen(filename, "r");

	if (fp != NULL) {
		while (fgets(line, sizeof(line), fp)) {
			if (strstr(line, pkgName)) {
//				LOGI("%s", line);
				if (strstr(line, module_name)) {
					LOGI("%s", line);
					//得到libPath
					pch = strstr(line, "/data");

					LOGI("%s", pch);
					LOGI("pch len: %d", strlen(pch));

					libPath = (char*) malloc(sizeof(char) * strlen(pch));
					strcpy(libPath, pch);
//					libPath = pch;
					LOGI("set libPath: %s", libPath);
					LOGI("module libPath len: %d", strlen(libPath));

					//分割字符串
					pch = strtok(line, "-");
					//字符串转变为数字
					addr = strtoul(pch, NULL, 16);

					if (addr == 0x8000)
						addr = 0;
					break;
				}

			}
			continue;
			//判断module_name是否是其子串
			if (strstr(line, module_name)) {

				//分割字符串
				pch = strtok(line, "-");
				//字符串转变为数字
				addr = strtoul(pch, NULL, 16);

				if (addr == 0x8000)
					addr = 0;

				break;
			}
		}

		fclose(fp);
	} else {
		LOGD("can't read /proc/pid/maps");
	}

	return (void *) addr;
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

//for (int i = 0; i < info.relpltsz; i++) {
//	Elf32_Rel& rel = info.relplt[i];
//	if (ELF32_R_SYM(rel.r_info) == symidx && ELF32_R_TYPE(rel.r_info) == R_ARM_JUMP_SLOT) {
//
//		void *addr = (void *) (info.elf_base + rel.r_offset);
//		if (replaceFunc(addr, replace_func, old_func))
//			goto fails;
//
//		//only once
//		break;
//	}
//}

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
//	LOGE("sym2: %p", symtab+fun2_Idx);
//
////	memcpy(symtab[fun1_Idx].st_value, symtab[fun2_Idx].st_value, sizeof(Elf32_Addr));
//
//	LOGI("ptrace pid: %d", pid);
//	void *fun1_addr = (void *) (symtab + fun1_Idx);
//	void *fun2_addr = (void *) (symtab + fun2_Idx);
//	fun1_addr = fun1_addr + sizeof(Elf32_Word);
//	fun2_addr = fun2_addr + sizeof(Elf32_Word);
//	LOGI("fun1 addr: %p", fun1_addr);
//	LOGI("fun2 addr: %p", fun2_addr);
//
//	unsigned * p1 = (unsigned *) fun1_addr;
//	unsigned * p2 = (unsigned *) fun2_addr;
//
////	Elf32_Addr addr1 =
//
////	*fun1_addr = *fun2_addr;
//
////	*fun1_addr = *(void **)fun2_addr;
//
//	int pagesize = sysconf(_SC_PAGE_SIZE);
//	if (pagesize == -1)
//		LOGE("sysconf get pagesize error!!!!!!!!!!");
//
//	void *name_address = NULL;
//	name_address = (size_t *) fun1_addr;
//
//	int result = mprotect(
//			(void *) (((size_t) name_address)
//					& (((size_t) - 1) ^ (pagesize - 1))), pagesize,
//			PROT_READ | PROT_WRITE);
//	LOGD("result: %d", result);
//	*p1 = *p2;
//
//	result = mprotect(
//			(void *) (((size_t) name_address)
//					& (((size_t) - 1) ^ (pagesize - 1))), pagesize,
//			PROT_READ | PROT_EXEC);
//	LOGD("result: %d", result);
	//****************************************************
//	*name_address = (size_t)fun1_addr;

//	if (mprotect((void *)fun1_addr& ((size_t)-1) ^ (pagesize - 1)), pagesize, PROT_READ | PROT_WRITE) == -1)
//	LOGE("error happend at mprotect: %d - %s", errno, strerror(errno));
//
//	fun1_addr = symtab[fun2_Idx].st_value;
//	if (mprotect(fun1_addr, pagesize, PROT_READ | PROT_EXEC) == -1)
//		LOGE("error happend at mprotect: %d - %s", errno, strerror(errno));

//	printfSymbols(symtab, strtab, symtabsz);
//	if (ptrace(PTRACE_POKETEXT, pid, symtab + fun1_Idx + sizeof(Elf32_Word),
//			symtab[fun2_Idx].st_value) == -1) {
//		LOGE("error happend at lash: %d - %s", errno, strerror(errno));
//	}

//	symtab[fun1_Idx].st_value = symtab[fun2_Idx].st_value;

	/**
	 * 以下是section的信息，存在问题。
	 */
//	shdr = (Elf32_Shdr *) (shdr_addr + module_base);
//	LOGI("section1 Addr: %p", shdr);
//	LOGI("section1 Desc size: %d", sizeof(*shdr));
//	LOGI("sizeof char: %d", sizeof(char));
//
//	LOGI("section1 Size: %d", shdr->sh_size);
//
//	LOGD("section1偏移地址: %p", shdr->sh_offset);
//
//	//Section Header区表每项的大小，一般等于sizeof(ELF32_Shdr)
//	int shent_size = si->e_shentsize;
//	LOGI("shent_size: %d", shent_size);
//	LOGI("Elf32_Shdr_size: %d", sizeof(Elf32_Shdr));
//	//读取Seciton Header中关于段表字符串表项的位置
//	unsigned long shstridx = si->e_shstrndx;
//	LOGI("shstridx: %d", shstridx);
//
//	void *shstrtab_add = shdr_addr + shstridx * shent_size + module_base;
//	LOGI("shstrtab_add: %p", shstrtab_add);
////迄今为止，正确
//	shdr = (Elf32_Shdr *) shstrtab_add;
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

//	relplt_addr = (Elf32_Addr *) relplt_addr;
//	LOGI("%d",sizeof(relplt_addr));
//	LOGI("%d",sizeof(Elf32_Rel));
//	LOGI("%d",sizeof(Elf32_Addr));
//	LOGI("%d",sizeof(Elf32_Word));
	LOGI("relplt_addr: %p", relplt[idx]);
	LOGI("relplt_addr offset: %p", relplt[idx].r_offset);

	void * relplt_addr;

	//跳转地址
	relplt_addr = relplt[idx].r_offset + module_base;
	LOGI("relplt_addr: %p", relplt_addr);
//	relplt_addr = (unsigned *) relplt_addr;
//	LOGI("*relplt_addr: %d", *relplt_addr);
	return relplt_addr;

}

/**
 * 将so1中的fun1替换为so2的fun2
 */
void hookcall(char* so1_name, char* so2_name, char* fun1_name, char* fun2_name) {
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
	//	void *name_address = NULL;
	//	name_address = (size_t *) fun1_addr;
	//
	//成功返回值为0
	if (!mprotect(
			(void *) (((size_t) fun_addr) & (((size_t) - 1) ^ (pagesize - 1))),
			pagesize, PROT_READ | PROT_WRITE)) {
		LOGE("mptotect success!!");
	} else {
		LOGE("mptotect failed!!");
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
	}
}
//测试，将memcpy替换为my_memcpy
void Java_com_example_myndk_MainActivity_testLib(JNIEnv* env, jobject thiz,
		jstring jstr, jint jpid) {

	char * s1 = "wangfabo";
	LOGE("s len: %d", my_strlen(s1));
	pid = jpid;
//	LOGI("Lib Test");
	const char *s = (*env)->GetStringUTFChars(env, jstr, NULL);
//	LOGI("%s", s);
//	LOGI("Pid: %d", pid);

	char * so1_Name = "libhello-jni.so";
	char * so2_Name = "libhook.so";
	void * module1_base;
	void * module2_base;
	char * fun_name = "memcpy";
	char * rel_fun_name = "my_memcpy";
	module1_base = get_module_base(pid, so1_Name);
	module2_base = get_module_base(pid, so2_Name);
	LOGE("module1 Base: %p", module1_base);
	LOGE("module2 Base: %p", module2_base);

	unsigned sym_addr = getSymAddr(module2_base, rel_fun_name);
	size_t * fun_addr = (size_t *) getFunAddr(module1_base, fun_name);

	LOGI("replace addr: %p", sym_addr);
	LOGI("func addr: %p", fun_addr);

	int pagesize = sysconf(_SC_PAGE_SIZE);
	if (pagesize == -1)
		LOGE("sysconf get pagesize error!!!!!!!!!!");
	LOGD("pagesize: %d", pagesize);
	//
	//	void *name_address = NULL;
	//	name_address = (size_t *) fun1_addr;
	//
	//成功返回值为0
	if (!mprotect(
			(void *) (((size_t) fun_addr) & (((size_t) - 1) ^ (pagesize - 1))),
			pagesize, PROT_READ | PROT_WRITE)) {
		LOGE("mptotect error!!");
	}
//	LOGD("result: %d", result);

//	if(modifyMemAccess((void *)fun_addr, PROT_EXEC|PROT_READ|PROT_WRITE,pagesize)){
//		LOGE("[-] modifymemAccess fails, error %s.", strerror(errno));
////		res = 1;
////		goto fails;
//	}
//#define PAGE_START(addr,pagesize) (~(pagesize - 1) & (addr))

//	void *page_start_addr = (void *) (~(pagesize - 1) & (fun_addr));
//	mprotect(page_start_addr, pagesize, PROT_READ | PROT_WRITE);

	fun_addr = (size_t *) fun_addr;
	*fun_addr = (size_t) sym_addr;

	if (!mprotect(
			(void *) (((size_t) fun_addr) & (((size_t) - 1) ^ (pagesize - 1))),
			pagesize, PROT_READ | PROT_EXEC)) {
		LOGE("mptotect error!!");
	}

//	result = mprotect(
//			(void *) (((size_t) fun_addr) & (((size_t) - 1) ^ (pagesize - 1))),
//			pagesize, PROT_READ | PROT_EXEC);
//	LOGD("result: %d", result);

//	sym_addr = getSymAddr(module2_base, rel_fun_name);
//	fun_addr = getFunAddr(module1_base, fun_name);

	LOGD("hooked replace addr: %p", sym_addr);
	LOGD("hooked func addr: %p", fun_addr);
	LOGD("hooked func addr: %p", *fun_addr);
//	*fun_addr = (size_t ) sym_addr;

}
//测试，将memcpy替换为my_memcpy
void Java_com_example_myndk_MainActivity_readLib(JNIEnv* env, jobject thiz) {
	char * so1_Name = "libhello-jni.so";
	char * so2_Name = "libhook.so";
//	void * module1_base;
//	void * module2_base;
	char * fun_name = "memcpy";
	char * rel_fun_name = "my_memcpy";

	hookcall(so1_Name, so2_Name, fun_name, rel_fun_name);

}

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
	//
	//	void *name_address = NULL;
	//	name_address = (size_t *) fun1_addr;
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
	} else {
		LOGE("mptotect failed!!");
		return -1;
	}
	return 1;
}
/**
 * jstr Hook的so名字
 * methodArray Hook的方法数组
 */
jint Hook(JNIEnv* env, jobject thiz, jstring jstr, jobjectArray methodArray) {
	/**
	 * 本共享库的名字
	 */
	char * so2_Name = "libhook.so";
	char * fun_name = "memcpy";
	char * rel_fun_name = "my_memcpy";

	ElfInfo *targetElfInfo;
	ElfInfo *myElfInfo;
	targetElfInfo = (ElfInfo *) malloc(sizeof(ElfInfo));
	myElfInfo = (ElfInfo *) malloc(sizeof(ElfInfo));
	//------得到目标so文件的信息
	const char *soName = (*env)->GetStringUTFChars(env, jstr, NULL);
	LOGI("soName: %s", soName);
	targetElfInfo->elf_base = findLibBase(soName);
	if (targetElfInfo->elf_base == 0) {
		LOGE("can not get target So %s base addr", soName);
		return -1;
	}
	LOGD("target so base addr: %p", targetElfInfo->elf_base);
	getElfInfo(targetElfInfo);
	//-----------------------------

	//------得到libhook.so文件的信息
	myElfInfo->elf_base = findLibBase(so2_Name);
	if (myElfInfo->elf_base == 0) {
		LOGE("can not get my so base addr");
		return -1;
	}
	LOGD("my so base addr: %p", myElfInfo->elf_base);
	getElfInfo(myElfInfo);
	//-----------------------------

//	unsigned sym_addr = getSymAddrFromElfInfo(myElfInfo, rel_fun_name);
//	if (sym_addr == 0) {
//		LOGE("can not get sym addr!");
//	}
//	size_t * fun_addr = (size_t *) getFunAddrFromElfInfo(targetElfInfo,
//			fun_name);
//	if (fun_addr == 0) {
//		LOGE("can not get fun addr!");
//	}
//
//	if (1 != replaceFun(sym_addr, fun_addr)) {
//		LOGE("replace fun failed!");
//	}

	jint length = (*env)->GetArrayLength(env, methodArray);
	int idx = 0;
	char *method;
	char *sufix = "my_";
	char *sym;
	for (idx = 0; idx < length; idx++) {
		jstring element = (*env)->GetObjectArrayElement(env, methodArray, idx);
		method = (*env)->GetStringUTFChars(env, element, NULL);
//		sym = method;
		sym = (char*) malloc(strlen(sufix) + strlen(method) + 1);
		strcpy(sym, sufix);
		strcat(sym, method);
		LOGI("method: %s", method);
		LOGI("sym: %s", sym);
		unsigned sym_addr = getSymAddrFromElfInfo(myElfInfo, sym);
		if (sym_addr == 0) {
			LOGE("can not get sym %s addr!", sym);
			continue;
		}
		size_t * fun_addr = (size_t *) getFunAddrFromElfInfo(targetElfInfo,
				method);
		if (fun_addr == 0) {
			LOGE("can not get fun %s addr!", method);
			continue;
		}

		if (1 != replaceFun(sym_addr, fun_addr)) {
			LOGE("replace fun failed!");
			continue;
		}

	}

	return 1;
}

//当动态库被加载时这个函数被系统调用
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
	JNIEnv* env = NULL;
	jint result = -1;

	//获取JNI版本
	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("GetEnv failed!");
		return result;
	}

	jclass cls = (*env)->FindClass(env, "com/example/myndk/MainActivity");
	jint nRes = (*env)->RegisterNatives(env, cls, gMethods,
			sizeof(gMethods) / sizeof(gMethods[0]));
	if (nRes < 0) {
		return JNI_ERR;
	}

	return JNI_VERSION_1_4;
}


void Java_com_example_myndk_MainActivity_testLibDvm(JNIEnv* env, jobject thiz) {

	ElfInfo *targetElfInfo;
	targetElfInfo = (ElfInfo *) malloc(sizeof(ElfInfo));
	const char *soName = "libart.so";
	LOGI("soName: %s", soName);
	targetElfInfo->elf_base = findLibBase(soName);
	if (targetElfInfo->elf_base == 0) {
		LOGE("can not get target So %s base addr", soName);
		return -1;
	}
	LOGD("target so base addr: %p", targetElfInfo->elf_base);
	getElfInfo(targetElfInfo);

	printRelInfoFromElfInfo(targetElfInfo);
	printSymInfoFromElfInfo(targetElfInfo);

//	char * so1_Name = "libdvm.so";
//	char * so2_Name = "libhook.so";
////	void * module1_base;
////	void * module2_base;
//	char * fun_name = "GetEnv";
//	char * rel_fun_name = "my_GetEnv";
//
//	hookcall(so1_Name, so2_Name, fun_name, rel_fun_name);
	//函数原型为
	// ClassObject* dvmResolveClass(const ClassObject* referrer, u4 classIdx,bool fromUnverifiedConstant)
//classobject 在object.h定义，u4在common.h定义
}


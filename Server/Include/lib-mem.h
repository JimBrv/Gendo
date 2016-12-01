/*
 * Header : lib-mem.h
 * Copyright : All right reserved by SecWin, 2010
 * Simple memory management and tracker wrappers
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */

#ifndef LIB_MEM_H
#define LIB_MEM_H

#include <stdio.h>
#include "lib-base.h"
#include "lib-types.h"
#include "lib-list.h"

#define mem_malloc(size)	lib_mem_malloc(size, __FILE__, __LINE__)
#define mem_free(ptr)		lib_mem_free(ptr, __FILE__, __LINE__)
#define mem_check(ptr)	        lib_mem_check(ptr)
#define mem_show()		lib_mem_show(stdout)

/* cause code of error process */
#define MEM_MALLOC_FAILED	1
#define MEM_FREE_INVALID1	2
#define MEM_FREE_INVALID2	3

/* default compile path */
#define MAX_FILE_COMPILE_PATH     256


/* mem hook function definition */
typedef int (*pfn_mem_hook_t)(int code, void *m);	/* a hook function for error process */

/* mem tracker infomation */
struct mem_struct {
	struct lib_list node;
	int    size;		/* for clear memory before free */
	int    module;          /* module id, not use now */
	char   file[MAX_FILE_COMPILE_PATH];  /* file name */
	int    line;		/* line number */
	int    magic;		/* detect a invalid memory block */
};


extern void *lib_mem_malloc(int size, char *file, int line);
extern void  lib_mem_free(void *ptr, char *file, int line);
extern int   lib_mem_check(void *ptr);
extern void  lib_mem_show(FILE *stream);
extern void  mem_init(pfn_mem_hook_t hook, int on);


#endif /* !LIB_MEM_H */

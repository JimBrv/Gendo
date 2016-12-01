/*
 * Filename : lib_hash.c
 * Copyright : All right reserved by SecWin, 2010
 * memory management and tracker simple wrappers
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */

#include "lib-base.h"
#include "lib-debug.h"
#include "lib-list.h"
#include "lib-mem.h"
#include "lib-thread.h"


#define MEM_MAGIC	          0xABCDE987	/* a arbitrary number for debug */

pfn_mem_hook_t mem_hook = 0;

int  mem_on = 1;		                        /* switch for memory tool */


THD_LOCK_T mem_lock;                        /* multi-thread support */
#define MEM_LOCK_INIT() thd_lock_init(&(mem_lock), NULL)
#define MEM_LOCK_FINL() thd_lock_finl(&(mem_lock))
#define MEM_LOCK()      thd_lock(&(mem_lock))
#define MEM_UNLOCK()    thd_unlock(&(mem_lock))


LIB_LIST_HEAD(mem_list);	/* head node for memory list of malloc */

/* 
 * Default function for error process while user module donn't provide.
 * Parameters in: 
 * Returns: OK
 */
int mem_default_hook(int code, void *arg)
{
	struct mem_struct *m = (struct mem_struct *)(arg);

	lib_error(MODULE_UTL, 
		  "MEM error: code[%d], module[%d] on [%s:%d]", 
		  code,
		  m->module, 
		  m->file, 
		  m->line);
	return OK;
}

/* 
 * Allocate memory, The memory have cleared.
 * Parameters in : size - block size to allocated memory 
 *		   test - a number provided by user module for debug
 * Returns : a pointer to the allocated memory. 
 */
void *lib_mem_malloc(int size, char *file, int line)
{
	int len = 0, *magicp = NULL;
	void *ptr = NULL;
	struct mem_struct *m = NULL;

	if (!mem_on) {
		ptr = malloc(size);
		if (!ptr) {
			return NULL;
		}

		memset(ptr, 0, size);
		return ptr;
	}

    MEM_LOCK();

	/* mem format: mem_struct|mem|magic */
	len = sizeof(struct mem_struct) + size + sizeof (int);
	m   = (struct mem_struct *)malloc(len);
	if (!m) {
	    mem_hook(MEM_MALLOC_FAILED, 0);
        goto out;
	}

	memset(m, 0, len);

	ptr     = (void *)(m + 1);
	magicp  = (int *)(ptr + size);
	m->size = len;
	m->line = line;
	strncpy(m->file, file, MAX_FILE_COMPILE_PATH);
	m->module= 0;
	m->magic = MEM_MAGIC;
	*magicp  = MEM_MAGIC;

	lib_list_add_tail(&(m->node), &mem_list);
out:
	MEM_UNLOCK();

	return ptr;
}

/* Free memory, The memory haved cleared before free.
 * Parameters in : ptr  - pointer of memory block 
 *		   test - a number provided by user module for debug
 * Returns : none
 */
void lib_mem_free(void *ptr, char *file, int line)
{
	int len = 0, *magicp = NULL;
	struct mem_struct *m = NULL;

	if (!mem_on) {
		if (ptr) 
			free(ptr);
		else
			lib_error(MODULE_UTL, "free NULL pointer [%s:%d]", file, line);
		return;
	}

    if (!ptr) return;

	MEM_LOCK();
    
    m = (struct mem_struct *)((char *)ptr - sizeof(struct mem_struct));

	if (m->magic != MEM_MAGIC) {
		mem_hook(MEM_FREE_INVALID1, m);
		goto out;
	}

	len = m->size;
	strncpy(m->file, file, MAX_FILE_COMPILE_PATH);
	m->line = line;
	magicp = (int *)((char *)m + len - sizeof(int));
	if ((*magicp) != MEM_MAGIC) {
		mem_hook(MEM_FREE_INVALID2, m);
		goto out;
	}
	lib_list_del(&(m->node));
    free(m);
    
out:
	MEM_UNLOCK();
}

int lib_mem_check(void *ptr)
{
	int len, *magicp;
	struct mem_struct *m;

	if (!mem_on) {
		return OK;
	}

	m = (struct mem_struct *)((char *)ptr - sizeof(struct mem_struct));

	if (m->magic != MEM_MAGIC) {
		mem_hook(MEM_FREE_INVALID1, m);
		return ERROR;
	}

	len = m->size;
	magicp = (int *) ((char *) m + len - sizeof (int));
	if ((*magicp) != MEM_MAGIC) {
		mem_hook(MEM_FREE_INVALID2, m);
		return ERROR;
	}

	return OK;
}

/* Show memory list of malloc.
 * Parameters in : stream - stream for output information
 * Returns : none
 */
void lib_mem_show(FILE * stream)
{
	lib_list_node *node;
	struct mem_struct *m;

	lib_info("memory list: module,  size,  address,  file:line\n");

	MEM_LOCK();
	lib_list_for_each(node, &mem_list) {
		m = lib_list_entry(node, struct mem_struct, node);
		lib_info("\t%s \t%d \t%p, \t%s:%d \t%s\n", 
			 m->module,				 
			 m->size - sizeof (struct mem_struct), 
			 m + 1,
			 m->file, 
			 m->line,
			 m->magic != MEM_MAGIC ? "(ERR)":"(OK)");
	}
	MEM_UNLOCK();
}

/* Initial function of memory tool module, (MUST be invoked in xxx_init of every module).
 * Parameters in : hook - a hook function for error process, (can be empty).
 * 		   on   - switch for memory tool module.
 * Returns : none
 */
void mem_init(pfn_mem_hook_t hook, int on)
{
	if (hook)
		mem_hook = hook;
	else
		mem_hook = mem_default_hook;

	mem_on = on;
	MEM_LOCK_INIT();
}



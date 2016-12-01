/*
 * Header : lib-index-cache.h
 * Copyright : All right reserved by SecWin, 2010
 * Cache simple implementation with indexing by id.
 * The user list cache should use the id as an index.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
 
#ifndef LIB_INDEX_CACHE_H
#define LIB_INDEX_CACHE_H

#include "lib-base.h"
#include "lib-types.h"
#include "lib-list.h"
#include "lib-thread.h"
#include "lib-msg.h"

#define IDX_CACHE_MAX  10000000 // max 10 million index, 40M memory consume 
#define IDX_CACHE_MID  1000000  // 1 million index
#define IDX_CACHE_SML  100000   // 100 k index

typedef void (*pfn_idx_cache_on_delete)(void *p1, void *p2);

typedef struct _idx_cache_data_s {
	lib_list_node node;
	void         *data;
    int           ref;    // reference count
}idx_cache_data_t, *pidx_cache_data_t;

#define idx_cache_data_ref_inc(ITEM)  ((ITEM)->ref++)
#define idx_cache_data_ref_dec(ITEM)  ((ITEM)->ref--)
#define is_idx_cache_data_refed(ITEM) ((ITEM)->ref > 0)

typedef struct _idx_cache_s {
	char  name[MAX_NAME_LEN];
	int   max;
	int   cur;
	pfn_idx_cache_on_delete fun_on_delete;
	void *p1;
	lib_list_node list;      // list for all items, facilitate for walking ops
	THD_LOCK_T lock;
	idx_cache_data_t *ary;  // ary for item pointers
}idx_cache_t, *pidx_cache_t;


idx_cache_t *idx_cache_init(char *name, int max, pfn_idx_cache_on_delete del_fun, void *p);
void idx_cache_finl(idx_cache_t *ic);
void idx_cache_delete(idx_cache_t *ic, UInt32 id);
void idx_cache_delete_internal(idx_cache_t *ic, UInt32 id);



/* 
  * Walk comparing function.
  * Return 1 for found items
  */
typedef int (*pfn_idx_cache_walk_cmp)(void *item, void *caller);
typedef int (*pfn_idx_cache_walk_job)(void *item, void *caller);


/*  
  * Walk function with cmp, then do job.
  * Once will control the walk process.
  */
void idx_cache_walk(idx_cache_t *ic, 
                          pfn_idx_cache_walk_cmp walk_cmp, 
                          pfn_idx_cache_walk_job walk_job,
                          void *caller,
                          int   once);
                    

void *idx_cache_get(idx_cache_t *ic, UInt32 id);
int idx_cache_update(idx_cache_t *ic, UInt32 id, void* data_updated);
int   idx_cache_put(idx_cache_t *ic, UInt32 id, void *param);
int   idx_cache_get_capacity(idx_cache_t *ic);
int   idx_cache_get_count(idx_cache_t *ic);

void *idx_cache_get_next(idx_cache_t *ic);

void  idx_cache_clear(idx_cache_t *ic, UInt32 id);
void idx_cache_flush(idx_cache_t *ic);


#endif

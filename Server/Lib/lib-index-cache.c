/*
 * Filename : lib-index-cache.c
 * Copyright : All right reserved by SecWin, 2010
 * Simple cache with uniq index id. It's useful for user database
 * for the id is never overwrite.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */

#include "lib-index-cache.h"
#include "lib-mem.h"
#include "lib-debug.h"

idx_cache_t *idx_cache_init(char *name, int max, pfn_idx_cache_on_delete del_fun, void *p)
{
	idx_cache_t *ic = NULL;

	if (max > IDX_CACHE_MAX) {
		lib_error(MODULE_UTL, "idx cache : new(%d) > max(%d)\n", max, IDX_CACHE_MAX);
		goto out;
	}
	ic = mem_malloc(sizeof(idx_cache_t));
	if (!ic) {
		lib_error(MODULE_UTL, "idx cache : malloc(%d) failed\n", max);
		goto out;
	}
	strncpy(ic->name, name, MAX_NAME_LEN);
	ic->max = max;
	ic->cur = 0;
	ic->fun_on_delete = del_fun;
	ic->p1 = p;
    INIT_LIB_LIST(&ic->list);
	
	ic->ary = (idx_cache_data_t *)mem_malloc(sizeof(idx_cache_data_t) * max);
	if (!ic->ary) {
		mem_free(ic);
		ic = NULL;
		lib_error(MODULE_UTL, "idx cache : malloc(%d) failed\n", max);
		goto out;		
	}
	thd_lock_init(&ic->lock, NULL);
	
out:
	return ic;
}


void idx_cache_finl(idx_cache_t *ic)
{
	int i = 0;
	void *p = NULL;
	
	if (!ic) {
		lib_error(MODULE_UTL, "idx cache : delete NULL, bug found\n");
		return;
	}
	for (i = 0; i < ic->max; i++) {
		p = ic->ary[i % ic->max].data;
		if (p && ic->fun_on_delete) {
			ic->fun_on_delete(ic->p1, p);
	    }
	}
	thd_lock_finl(&ic->lock);
	mem_free(ic->ary);
	mem_free(ic);	
}

inline void idx_cache_delete(idx_cache_t *ic, UInt32 id)
{	
	void *p = NULL;
    if (id >= ic->max) return;
	
	thd_lock(&ic->lock);
	p = ic->ary[(id % ic->max)].data;
	ic->ary[(id % ic->max)].data = NULL;
	lib_list_del(&(ic->ary[(id % ic->max)].node));
	ic->cur--;
	thd_unlock(&ic->lock);

	/* call back item delete function */
	if (p && ic->fun_on_delete) 
		ic->fun_on_delete(ic->p1, p);
}

/* like idx_cache_delete(), but assumes locked outside */
inline void idx_cache_delete_internal(idx_cache_t *ic, UInt32 id)
{	
	void *p = NULL;
    if (id >= ic->max) return;
	
	p = ic->ary[(id % ic->max)].data;
	ic->ary[(id % ic->max)].data = NULL;
	lib_list_del(&(ic->ary[(id % ic->max)].node));
	ic->cur--;

	/* call back item delete function */
	if (p && ic->fun_on_delete) 
		ic->fun_on_delete(ic->p1, p);
}


inline void *idx_cache_get(idx_cache_t *ic, UInt32 id)
{
	void *p = NULL;
    if (id >= ic->max) return NULL;
	
	thd_lock(&ic->lock);
	p = ic->ary[(id % ic->max)].data;
	thd_unlock(&ic->lock);
	return p;
}

inline int idx_cache_update(idx_cache_t *ic, UInt32 id, void* data_updated)
{
    if (id >= ic->max) return NULL;
	
	thd_lock(&ic->lock);
	ic->ary[(id % ic->max)].data = data_updated;
	thd_unlock(&ic->lock);
	return OK;
}

/* walk cache until the end */
void *idx_cache_get_next(idx_cache_t *ic)
{
	void *p = NULL;	
	return p;
}

void idx_cache_walk(idx_cache_t *ic, 
                          pfn_idx_cache_walk_cmp walk_cmp, 
                          pfn_idx_cache_walk_job walk_job,
                          void *caller,
                          int   once)
{    
	int found = 1;    
	lib_list_node *pnode = NULL;
    idx_cache_data_t *item = NULL;
	
	thd_lock(&ic->lock);

    lib_list_for_each(pnode, &ic->list) {
        item = lib_list_entry(pnode, idx_cache_data_t, node);
        if (walk_cmp) {
            /* 
                       * if walk_cmp set, the found indicates the search result,
                       * if not, the walk all list to do the job
                       */
            found = walk_cmp((void*)item->data, caller);
        }

        if (found && walk_job) {
            walk_job((void*)item->data, caller);
        }
        
        if (found && once) break;
    }	

    thd_unlock(&ic->lock);
    return;
}


inline int  idx_cache_get_count(idx_cache_t *ic)
{
	return ic->cur;
}


inline int idx_cache_get_capacity(idx_cache_t *ic)
{
	return ic->max;
}
inline int  idx_cache_put(idx_cache_t *ic, UInt32 id, void *param)
{
	void *p = NULL;
    int  ret= OK;

    if (id >= ic->max) return ERROR;
	
	thd_lock(&ic->lock);
	if ((p = ic->ary[(id % ic->max)].data) != NULL) {
		lib_error(MODULE_UTL, "idx cache : the slot has old item=[%p] when put new item=[%p], bug found\n",
			      p, param);
		//lib_assert(false);
		ret = ERROR;
	}else{
		ic->ary[(id % ic->max)].data = param;
		lib_list_add_tail(&(ic->ary[(id % ic->max)].node), &ic->list);
		ic->cur++;
		lib_assert(ic->cur < ic->max);
	}
	thd_unlock(&ic->lock);
	return ret;
}

inline void  idx_cache_clear(idx_cache_t *ic, UInt32 id)
{
	void *p = NULL;
	
	thd_lock(&ic->lock);
	if ((p = ic->ary[(id % ic->max)].data) != NULL) {
		ic->ary[(id % ic->max)].data = NULL;
	       lib_list_del(&(ic->ary[(id % ic->max)].node));		
		ic->cur--;		
	}
	thd_unlock(&ic->lock);
	return;
}
inline void idx_cache_flush(idx_cache_t *ic)
{
	int i = 0;
	void *p = NULL;
	
	if (!ic) {
		lib_error(MODULE_UTL, "idx cache : delete NULL, bug found\n");
		return;
	}
	thd_lock(&ic->lock);
	for (i = 0; i < ic->max; i++) {
		if ((p = ic->ary[(i % ic->max)].data) != NULL) {
			ic->ary[(i % ic->max)].data = NULL;
		   	lib_list_del(&(ic->ary[(i % ic->max)].node));		
			ic->cur--;
		}
		/* call back item delete function */
		if (p && ic->fun_on_delete) 
			ic->fun_on_delete(ic->p1, p);
		}
	thd_unlock(&ic->lock);
	return;
}

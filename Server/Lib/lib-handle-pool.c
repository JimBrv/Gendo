/*
 * Filename : lib-handle-pool.c
 * Copyright : All right reserved by SecWin, 2010
 * General handle pool support
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2011.6     initial version        
 */
#include <unistd.h>
#include "lib-debug.h"
#include "lib-handle-pool.h"

static handle_pool_t handle_pool;
static bool binit = false;
#define NO_HANDLE_SLEEP_USEC 500*1000      // 500 ms
#define NO_HANDLE_SLEEP_SEC  1             // 1 sec

#define is_hd_ok(HD)  ((HD) && HANDLE_IDLE == (HD)->state && (HD)->handle != NULL)


handle_t *_hp_get_handle(char *file, int line)
{
	lib_list_node *pNode, *pNodeTp;
	handle_t *handle = NULL;
	bool found = false;
    //int  try_cnt = 0;
	
	if (handle_pool.count <= 0) {
		hp_error("handle pool's count == 0!!!");
		return NULL;
	}
	
	//sem_wait(&handle_pool.sem);
//again:

	thd_lock(&handle_pool.lock);
    lib_list_for_each_safe(pNode, pNodeTp, &(handle_pool.handle_list)) {
		handle = lib_list_entry(pNode, handle_t, node);
		if (is_hd_ok(handle)) {
			handle->state = HANDLE_BUSY;
			strncpy(handle->file, file, MAX_FILE_COMPILE_PATH);
			handle->line = line;
			found = true;		
			break;
		}
    }
	thd_unlock(&handle_pool.lock);

#if 0
    if (!found) {
        if ((try_cnt++) < 4) {
            /* wait for some release */
            sleep(NO_HANDLE_SLEEP_SEC);
            goto again;
        }
    }
#endif

#if 0
    /* try to re-init one handle */
    if (!found) {
        thd_lock(&handle_pool.lock);
        lib_list_for_each_safe(pNode, pNodeTp, &(handle_pool.handle_list)) {
            handle = lib_list_entry(pNode, handle_t, node);
            if (HANDLE_NONE == handle->state) {
                if (OK == hp_reinit_handle(handle)) {
                    found = false;       
                }
                break;
            }
        }
        thd_unlock(&handle_pool.lock);
    }        
#endif

	return (found ? handle : NULL);
}


void hp_put_handle(handle_t *handle)
{
    thd_lock(&handle_pool.lock);
	if (handle) {
        if (NULL != handle->handle && HANDLE_BUSY == handle->state) {
		    handle->state = HANDLE_IDLE;
        }else{
            hp_info("handle inactive: idx=[%d]\n", handle->idx);
            handle->state = HANDLE_NONE;
        }
		//memset(handle->file, 0, MAX_FILE_COMPILE_PATH);
		handle->line  = 0;
	}
	thd_unlock(&handle_pool.lock);
	
	return;
}

int  hp_reinit_handle(handle_t *handle)
{
    assert(handle != NULL);
	int ret = 0;
	if (handle->pfn_close) 
        ret = handle->pfn_close(handle);
	if (handle->pfn_open)
        ret = handle->pfn_open(handle);

	if (ret) {
        hp_error("handle, idx=[%d] re-init open failed!!!\n", handle->idx);
        handle->state = HANDLE_NONE;
	}else{
        hp_info("handle, idx=[%d] re-init open ok.\n", handle->idx);
        handle->state = HANDLE_IDLE;
	}
	
    return ret;
}

int hp_new_handle(pfn_handle_open pfn_open, pfn_handle_open pfn_close, void *ctx)
{
	handle_t *handle = NULL;
	int ret  = 0;
	
	handle = mem_malloc(sizeof(handle_t));
	if (!handle) {
		hp_error("malloc handle memory failed\n");
		goto err;
	}
	handle->ctx      = ctx;
	handle->pfn_open = pfn_open;
	handle->pfn_close= pfn_close;
	handle->state    = HANDLE_IDLE;
  
	ret = pfn_open(handle);
	if (ret) {
		hp_error("open handle failed\n");
		goto err;
	}
	
	thd_lock(&handle_pool.lock);	
	lib_list_add(&(handle->node), &(handle_pool.handle_list));	
    handle_pool.count++;
    handle->idx = handle_pool.count;
	thd_unlock(&handle_pool.lock);	
	return OK;
	
err:
	if (handle) mem_free(handle);
	return ERROR;
		
}

int hp_get_count(void)
{
   return handle_pool.count;
}


/*
 * just init lock 
 */
int  hp_init(int count)
{
    if (binit) {
		hp_error("pool has been inited, bug found\n");
		return ERROR;
    }
	
	INIT_LIB_LIST(&(handle_pool.handle_list));

	if (thd_lock_init(&(handle_pool.lock), NULL)) {
		hp_error("init thread lock failed\n");
		goto err;					
	}
	if (sem_init(&(handle_pool.sem), 0, count)) {
		hp_error("init sem failed\n");
		goto err;					
	}
	
	handle_pool.count = 0;
	binit = true;
	return OK;	
err:
	INIT_LIB_LIST(&(handle_pool.handle_list));
	return ERROR;		    
}


int  hp_finl(void)
{
    lib_list_node *pNode, *pNodeTp;
	handle_t *handle;

	thd_lock(&handle_pool.lock);
	lib_list_for_each_safe(pNode, pNodeTp, &(handle_pool.handle_list)) {
		handle = lib_list_entry(pNode, handle_t, node);
		lib_list_del(&(handle->node));
		if (handle->pfn_close) handle->pfn_close(handle);
		mem_free(handle);
	}
    thd_unlock(&handle_pool.lock);	
	
	INIT_LIB_LIST(&handle_pool.handle_list);
	thd_lock_finl(&handle_pool.lock);
	sem_destroy(&handle_pool.sem);
	
	handle_pool.count = 0;
	binit = false;
	return OK;
}


/* 
 * Walk all HP to check its keeplive 
 */
int hp_keeplive_walk(void *param)
{
	lib_list_node *pNode, *pNodeTp;
	handle_t *handle = NULL;
    int ret = 0;
	
	if (handle_pool.count <= 0) {
		hp_error("HP-Walk: handle pool's count == 0!!!");
		return OK;
	}
	
	thd_lock(&handle_pool.lock);
    lib_list_for_each_safe(pNode, pNodeTp, &(handle_pool.handle_list)) {
        handle = lib_list_entry(pNode, handle_t, node);
        if (is_hp_none(handle)) {
            hp_info("HP-Walk: re-init handle idx=%d, fd=%d, state=%d...\n", 
                    handle->idx,
                    (int)handle->handle,
                    handle->state);
            ret = hp_reinit_handle(handle);
            if (OK == ret) {
                sleep(5);
            }
        }        
    }
    thd_unlock(&handle_pool.lock);
    return OK;
}


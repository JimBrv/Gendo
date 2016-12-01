/*
 * Filename : lib_socket.c
 * Copyright : All right reserved by SecWin, 2010
 * thread and pool management
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
#include "lib-base.h"
#include "lib-socket.h"
#include "lib-debug.h"
#include "lib-mem.h"
#include "lib-thread.h"

#define IS_NO_IDLE_AND_EXCEED_LIMIT(POOL)				\
    ((((POOL)->idle_threads)<=0 )&&(((POOL)->threads_num)>=((POOL)->max_threads)))

#define IS_NO_IDLE_AND_BELOW_LIMIT(POOL)				\
    ((((POOL)->idle_threads)<=0 )&&(((POOL)->threads_num)<((POOL)->max_threads)))

#define IS_IDLE(POOL) ((POOL)->idle_threads > 0)
	

static thread_pool_t global_thread_pool;

static inline thread_pool_t *get_thread_pool(void)
{
    return &global_thread_pool;
}

inline void thread_insert_new(thread_pool_t *pool, thread_item_t *thread)
{
    lib_list_add(&(thread->node), &(pool->idle_list));
    pool->threads_num++;
}

inline void thread_remove_new(thread_pool_t *pool, thread_item_t *thread)
{
    lib_list_del(&(thread->node));
    pool->threads_num--;
}


/* assume the pool global lock is hold 
 * just return the input thread to faclitate some environment
 */
inline thread_item_t *thread_put_idle(thread_item_t *thread)
{
#if 0
    thread_pool_t *pool = thread->pool;
    lib_list_add(&(thread->node), &(pool->idle_list));
    pool->idle_threads++;
    return thread;
#endif
    set_thread_idle(thread);
    thread_pool_t *pool = thread->pool;
    pool->idle_threads++;
    return thread;
}

/* assume the pool global lock is hold */
thread_item_t *thread_get_idle(thread_pool_t *tp)
{
#if 0
    thread_pool_t *pool   = tp;
    thread_item_t *thread = NULL;
    lib_list_node     *pNode  = NULL;
    if (lib_list_empty(&(pool->idle_list)) && pool->idle_threads == 0) {
	return NULL;
    }else{
	pNode = (&(pool->idle_list))->next;
	thread = lib_list_entry(pNode, thread_item_t, node);
	lib_list_del(&(thread->node));
	pool->idle_threads--;
	return thread;
    }
#endif
    /* Simple state to make it stable */
    bool found          = false;
    lib_list_node *node = NULL;
    thread_item_t *thd  = NULL;
    thread_pool_t *pool = tp;

    lib_list_for_each(node, &(pool->idle_list)) {
        thd = lib_list_entry(node, thread_item_t, node);
        if (thd && is_thread_idle(thd)) {
            set_thread_busy(thd);
          	pool->idle_threads--;
            found = true;
            break;
        }    
    }

    return (found ? thd : NULL); 
}

inline thread_item_t *thread_put_busy(thread_item_t *thread)
{
#if 0
    thread_pool_t *pool = thread->pool;
    lib_list_add(&(thread->node), &(pool->busy_list));
    return thread;
#endif
    set_thread_busy(thread);
    return thread;
    
}

/* pop thread from busy list, 
 * pay attention to that there is no thread_get_busy() for no reason to get an 
 * thread executing.
 * this function pop an unused 
 */
inline thread_item_t *thread_pop_busy(thread_item_t *thread)
{
#if 0
    lib_list_del(&(thread->node));
    return thread;
#endif
    return thread;
}


/*
 * reclaim thread to pool, when the thread number blows the max limit
 * just put it into pool. And if it exceeds the max limit, means the thread
 * should not be reclaimed. 
 * return : 0  reclaim ok
 *             -1 reclaim failed for the thread child exceeds the pool max limitation
 */
static int reclaim_thread(thread_pool_t *thread_pool,thread_item_t *thread)
{
    int ret = ERROR;
    
    thd_lock(&thread_pool->threads_lock);
    if (thread_pool->idle_threads < thread_pool->max_threads) {
        thread_put_idle(thread);	
        ret = OK;
        if (thread_pool->idle_threads >= thread_pool->threads_num) {
            thd_cond_signal(&thread_pool->mFullCond);
        }
    }
    thd_unlock(&thread_pool->threads_lock);
    thd_cond_signal(&thread_pool->mIdleCond);       
    return ret;
}

static void* thread_wrapper(void* arg)
{
    thread_item_t *thread = (thread_item_t *)arg;
    thread_pool_t *pool   = thread->pool;
	
    while(!pool->is_shutdown) {

	    /* do the job first */
        thread->pfunc(thread->arg1, thread->arg2);
        
	    /* job done, reclaim thread to pool */
        thd_lock(&thread->thread_lock);
	    //thread_pop_busy(thread);
		
        if (reclaim_thread(pool, thread) == OK){
	        /* reclaimed ok, just wait self condition for new assinged job */
            thd_cond_wait(&thread->thread_cond,&thread->thread_lock);
            thd_unlock(&thread->thread_lock);
        }else{
            thd_unlock(&thread->thread_lock);
            thd_cond_finl(&thread->thread_cond);
            thd_lock_finl(&thread->thread_lock);
            thread_remove_new(thread->pool, thread);
            mem_free(thread);
            thread = NULL;
            break;
        }
    }

    if(thread != NULL) {
	    /* reclaim ok, notify pool with empty condition, force those wait job to exec */
        thd_lock(&thread->thread_lock);
        if(pool->threads_num <= 0) {
            thd_cond_signal(&pool->mEmptyCond);
        }
        thd_unlock(&thread->thread_lock);
    }
	
    return 0;
}


int _thread_dispatch(pfn_dispatch pfun, void* arg1, void* arg2, char *file, int line)
{
	int            ret = 0;
	size_t         old_size = 0;
	THD_ATTR_T     attr;
	thread_item_t *thread = NULL;
	thread_pool_t *thread_pool = get_thread_pool();

	thd_lock(&thread_pool->threads_lock);

	/* wait one thread to idle */
	while(IS_NO_IDLE_AND_EXCEED_LIMIT(thread_pool) == true){
	    thd_cond_wait(&thread_pool->mIdleCond, &thread_pool->threads_lock);	
	}

	if (IS_NO_IDLE_AND_BELOW_LIMIT(thread_pool) == true) {
		/* all threads are busy, and the pool blows max limit, so create new one */
		thread = (thread_item_t *)mem_malloc(sizeof(thread_item_t));
		memset(&thread->tid, 0, sizeof(THREAD_T));
		thd_lock_init(&thread->thread_lock,NULL);
		thd_cond_init(&thread->thread_cond,NULL);
		thread->arg1   = arg1;
		thread->arg2   = arg2;
		thread->pfunc  = pfun;
		strncpy(thread->file, file, MAX_FILE_COMPILE_PATH);
		thread->line   = line;
		INIT_LIB_LIST(&thread->node);

		thread->pool   = thread_pool;
			
		thd_attr_init(&attr);
		thd_attr_set_detach(&attr, PTHREAD_CREATE_DETACHED);

		if (thd_attr_get_stack(&attr, &old_size) != 0) {
			lib_error(MODULE_UTL, "thread pool get stack size failed, exit");
			goto out;
		}

		if (old_size > MIN_THREAD_STACK_SIZE) {
			if (thd_attr_set_stack(&attr, MIN_THREAD_STACK_SIZE) != 0) {
				lib_error(MODULE_UTL, "thread pool set stack size failed, exit");
				goto out;
			}
		}       

		/* create new thread to exec function */
		//thread_put_busy(thread);
		set_thread_busy(thread);
		thread_insert_new(thread_pool, thread);
		if (thd_create(&thread->tid, &attr, thread_wrapper, thread) == 0) {
			/* new thread working now */
			lib_info("thread pool => create new thread ok: pool=%p, max=%d, cur=%d, idle=%d, tid=%d\n", 
			thread_pool, thread_pool->max_threads, thread_pool->threads_num, thread_pool->idle_threads, thread->tid);            
		}
		else{
			ret = -1;
			lib_error(MODULE_UTL, "can't create thread.\n");
			thd_lock_finl(&thread->thread_lock);
			thd_cond_finl(&thread->thread_cond);
			//thread_pop_busy(thread);
			thread_remove_new(thread_pool, thread);
			mem_free(thread);
		}
	       thd_attr_finl(&attr);
	}
	else 
	if (IS_IDLE(thread_pool)) {
		/* thread pool has idle thread */
		thread = thread_get_idle(thread_pool);
		if (!thread) {
			lib_error(MODULE_UTL, "pool has idle thread, but return NULL [%s:%d]", __FILE__, __LINE__);
			goto out;
		}
		/* trigger the assigned thread to work */
		thd_lock(&thread->thread_lock);

		thread->pfunc  = pfun;
		thread->arg1   = arg1;
		thread->arg2   = arg2;
		strncpy(thread->file, file, MAX_FILE_COMPILE_PATH);
		thread->line   = line;
		thread->pool   = thread_pool;

		//thread_put_busy(thread);
		thd_unlock(&thread->thread_lock);        
		thd_cond_signal(&thread->thread_cond);
	}
	else {
		lib_error(MODULE_UTL, "thread pool: no such condition [%s:%d]", __FILE__, __LINE__);
	}

out:	
	thd_unlock(&thread_pool->threads_lock);
	return ret;
}



int thread_dispatch_with_tp(pfn_dispatch pfun, void* arg1, void* arg2, void *tp, char *file, int line)
{
    int            ret = 0;
    size_t         old_size = 0;
    THD_ATTR_T     attr;
    thread_item_t *thread = NULL;
    thread_pool_t *thread_pool = (thread_pool_t*)tp;
    assert(thread_pool != NULL);

    thd_lock(&thread_pool->threads_lock);
    
    /* wait one thread to idle */
    while(IS_NO_IDLE_AND_EXCEED_LIMIT(thread_pool) == true)
	{
	    thd_cond_wait(&thread_pool->mIdleCond, &thread_pool->threads_lock);	
	}

    if (IS_NO_IDLE_AND_BELOW_LIMIT(thread_pool) == true) {
	    /* all threads are busy, and the pool blows max limit, so create new one */
        
	    thread = (thread_item_t *)mem_malloc(sizeof(thread_item_t));
        memset(&thread->tid, 0, sizeof(THREAD_T));
        thd_lock_init(&thread->thread_lock,NULL);
        thd_cond_init(&thread->thread_cond,NULL);
        thread->arg1   = arg1;
		thread->arg2   = arg2;
        thread->pfunc  = pfun;
	    strncpy(thread->file, file, MAX_FILE_COMPILE_PATH);
	    thread->line   = line;
	    INIT_LIB_LIST(&thread->node);
		
        thread->pool   = thread_pool;
			
        thd_attr_init(&attr);
        thd_attr_set_detach(&attr, PTHREAD_CREATE_DETACHED);
        if (thd_attr_get_stack(&attr, &old_size) != 0) {
	        lib_error(MODULE_UTL, "thread pool get stack size failed, exit");
            goto out;
        }
        
        if (old_size > MIN_THREAD_STACK_SIZE) {
	        if (thd_attr_set_stack(&attr, MIN_THREAD_STACK_SIZE) != 0) {
		        lib_error(MODULE_UTL, "thread pool set stack size failed, exit");
		        goto out;
	        }
        }       
	
	    /* create new thread to exec function */
	    //thread_put_busy(thread);
        set_thread_busy(thread);
        thread_insert_new(thread_pool, thread);            
        if (thd_create(&thread->tid, &attr, thread_wrapper, thread) == 0) {            
            /* new thread working now */
            lib_info("thread pool => create new thread ok: pool=%p, max=%d, cur=%d, tid=%d\n", 
                      thread_pool, thread_pool->max_threads, thread_pool->threads_num, thread->tid);
            
        }else{
            ret = -1;
            lib_error(MODULE_UTL, "can't create thread.\n");
            thd_lock_finl(&thread->thread_lock);
            thd_cond_finl(&thread->thread_cond);
	        //thread_pop_busy(thread);
            thread_remove_new(thread_pool, thread);
            mem_free(thread);
        }
        thd_attr_finl(&attr);
    }else if (IS_IDLE(thread_pool)) {
	    /* thread pool has idle thread */
	    thread = thread_get_idle(thread_pool);
	    if (!thread) {
	        lib_error(MODULE_UTL, "pool has idle thread, but return NULL [%s:%d]", __FILE__, __LINE__);
	        goto out;
	    }
        /* trigger the assigned thread to work */
        thd_lock(&thread->thread_lock);
		
        thread->pfunc  = pfun;
        thread->arg1   = arg1;
		thread->arg2   = arg2;
	    strncpy(thread->file, file, MAX_FILE_COMPILE_PATH);
	    thread->line   = line;
        thread->pool   = thread_pool;
	    //thread_put_busy(thread);
        thd_unlock(&thread->thread_lock);
        
        thd_cond_signal(&thread->thread_cond);		
    }else {
    	lib_error(MODULE_UTL, "thread pool: no such condition [%s:%d]", __FILE__, __LINE__);
    }

out:	
    thd_unlock(&thread_pool->threads_lock);
    return ret;
}


static int create_thread_pool(thread_pool_t *thread_pool,int max_thread)
{
    if (!thread_pool) return ERROR; 
    if (thread_pool->init) {
	lib_error(MODULE_UTL, "do you want to overwrite an initialized thread pool?");
	return ERROR;
    }

    if(max_thread <= 0) {
        thread_pool->max_threads = 2;
    }
    thread_pool->max_threads = max_thread;
    
    thd_lock_init(&thread_pool->threads_lock,NULL);
    thd_cond_init(&thread_pool->mEmptyCond,NULL);
    thd_cond_init(&thread_pool->mFullCond,NULL);
    thd_cond_init(&thread_pool->mIdleCond,NULL);

    thread_pool->idle_threads= 0;
    thread_pool->threads_num = 0;
    thread_pool->is_shutdown = 0;

    INIT_LIB_LIST(&(thread_pool->idle_list));
    INIT_LIB_LIST(&(thread_pool->busy_list));
	
    thread_pool->init = true;
	
    return OK;
}

static void free_thread_idle_list(lib_list_node *list)
{
    thread_item_t *thread = NULL;
    if (!list) return;
	
    thread = lib_list_entry(list, thread_item_t, node);
    thd_lock_finl(&thread->thread_lock);
    thd_cond_finl(&thread->thread_cond);
    mem_free(thread);
}
static void free_thread_pool(thread_pool_t *thread_pool)
{
    lib_list_node *pNode   = NULL;
    lib_list_node *pNodeTp = NULL;

    if (!thread_pool || thread_pool->init == false) return;
    lib_info("shutdown thread pool\n");
    
    thd_lock(&thread_pool->threads_lock);
    
    lib_info("thread pool: free pool. idle=%d, all=%d, max=%d\n",
	     thread_pool->idle_threads, 
	     thread_pool->threads_num,
	     thread_pool->max_threads);

    thread_pool->is_shutdown = 1;

    if (thread_pool->idle_threads < thread_pool->threads_num) {
        lib_info("wait working threads=%d to exit\n", thread_pool->threads_num - thread_pool->idle_threads);
        thd_cond_wait(&thread_pool->mFullCond, &thread_pool->threads_lock);
    }

    /* trigger all wait idle thread to exit */
    lib_list_for_each_safe(pNode, pNodeTp, &(thread_pool->idle_list)) {
	thread_item_t *thread = lib_list_entry(pNode, thread_item_t, node);
        if(thread != NULL) {
            thd_lock(&thread->thread_lock);
            thd_cond_signal(&thread->thread_cond);
            thd_unlock(&thread->thread_lock);
        }
    }
    
    lib_info("thread pool: wait all threads exit and empty condition\n");
    if(thread_pool->threads_num > 0) {
        lib_info("Thread pool wait empty condition, left thread=%d to exit\n",thread_pool->threads_num );
        thd_cond_wait(&thread_pool->mEmptyCond,&thread_pool->threads_lock);
    }

    lib_info("thread pool: destroy resource\n");
    lib_list_free(&(thread_pool->idle_list), free_thread_idle_list);
    thread_pool->idle_threads = 0;
    thd_unlock(&thread_pool->threads_lock);
	
    thd_lock_finl(&thread_pool->threads_lock);
    thd_cond_finl(&thread_pool->mIdleCond);
    thd_cond_finl(&thread_pool->mEmptyCond);
    thd_cond_finl(&thread_pool->mFullCond);

    thread_pool->init = false;

    lib_info("thread pool: exit\n");
	
    return;
}

inline int get_max_threads(void)
{
    thread_pool_t *pool = get_thread_pool();
    return pool->max_threads;
}

void thread_pool_dump(void)
{
    thread_pool_t *pool   = get_thread_pool();
    thread_item_t *thread = NULL;
    lib_list_node     *pNode, *pNodeTp;
    int i = 0;

    thd_lock(&pool->threads_lock);
    lib_info("\t dump thread pool info start, idle=%d, all=%d, max=%d\n", pool->idle_threads, pool->threads_num, pool->max_threads);

    lib_list_for_each_safe(pNode, pNodeTp, &(pool)->idle_list) {
	i++;
	thread = lib_list_entry(pNode, thread_item_t, node);
        if(thread != NULL) {			
	    thd_lock(&thread->thread_lock);
	    lib_info("\t thread=[%d]: state=[%s], tid=[%x], code=[%s:%d]\n", i, "idle", thread->tid, thread->file, thread->line);
	    thd_unlock(&thread->thread_lock);
        }
    }

    lib_list_for_each_safe(pNode, pNodeTp, &(pool)->busy_list) {
	i++;
	thread = lib_list_entry(pNode, thread_item_t, node);
	if(thread != NULL) {			
	    thd_lock(&thread->thread_lock);
	    lib_info("\t thread=[%d]: state=[%s], tid=[%x], code=[%s:%d]\n", i, "busy", thread->tid, thread->file, thread->line);
	    thd_unlock(&thread->thread_lock);
	}
    }	
    
    thd_unlock(&pool->threads_lock);
    lib_info("\t dump thread pool info over\n");    
}

int thread_pool_init(int max_thread)
{
    return create_thread_pool(get_thread_pool(), max_thread);
}


thread_pool_t *thread_pool_new(int max_thread)
{
    thread_pool_t *tp = NULL;
    int ret = OK;
    tp = mem_malloc(sizeof(thread_pool_t));
    assert(tp != NULL);
    ret = create_thread_pool(tp, max_thread);
    if (ret) return NULL;
    return (void*)tp;
}

int thread_pool_finl(void)
{    
    free_thread_pool(get_thread_pool());
    return OK;
}

void monitor_threads_num( void )
{
    thread_pool_t *pool = get_thread_pool();
    lib_info("glb_threads_num[%d] .\n", pool->threads_num);
}


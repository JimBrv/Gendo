/*
 * Header : lib-thread.h
 * Copyright : All right reserved by SecWin, 2010
 * Linux thread and pool management
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
#ifndef LIB_THREAD_H
#define LIB_THREAD_H

#include <pthread.h>
#include "lib-types.h"
#include "lib-mem.h"
#include "lib-list.h"
#include "lib-bit.h"

#ifndef win32
#define THD_LOCK_T   pthread_mutex_t
#define THD_COND_T   pthread_cond_t
#define THD_ATTR_T   pthread_attr_t
#define THREAD_T     pthread_t


#define thd_create(TID, ATTR, FUN, PARAM) pthread_create((TID), (ATTR), (FUN), (PARAM))

#define thd_lock(LOCK)             pthread_mutex_lock((LOCK))
#define thd_unlock(LOCK)           pthread_mutex_unlock((LOCK))
#define thd_cond_wait(COND, LOCK)  pthread_cond_wait((COND), (LOCK)) 
#define thd_cond_signal(COND)      pthread_cond_signal((COND))

#define thd_lock_init(LOCK, P1)    pthread_mutex_init((LOCK), (P1))
#define thd_lock_finl(LOCK)        pthread_mutex_destroy((LOCK))
#define thd_cond_init(COND, P1)    pthread_cond_init((COND), (P1))
#define thd_cond_finl(COND)        pthread_cond_destroy((COND))
#define thd_attr_init(ATTR)        pthread_attr_init((ATTR))
#define thd_attr_finl(ATTR)        pthread_attr_destroy((ATTR))

#define thd_attr_set_detach(ATTR, OPT)    pthread_attr_setdetachstate((ATTR), (OPT))
#define thd_attr_set_stack(ATTR, OPT)     pthread_attr_setstacksize((ATTR), (OPT))
#define thd_attr_get_stack(ATTR, OPT)     pthread_attr_getstacksize((ATTR), (OPT))

#else  
/* windows thread lock goes here */

#endif 


#define MAX_THREAD (1024)                  // default max threads in one process
#define MIN_THREAD_STACK_SIZE  1<<17       // 128k stack

typedef void (*pfn_dispatch)(void *, void *);    // thread pool dispatch task for idle thread

struct _thread_pool_s;

enum {
    THD_STATE_BUSY = 1,
};

#define is_thread_busy(THREAD)    is_bit_set((THREAD)->state, THD_STATE_BUSY)
#define is_thread_idle(THREAD)    (!is_bit_set((THREAD)->state, THD_STATE_BUSY))
#define set_thread_idle(THREAD)   unset_bit((THREAD)->state, THD_STATE_BUSY)
#define set_thread_busy(THREAD)   set_bit((THREAD)->state, THD_STATE_BUSY)


typedef struct _thread_s{
    lib_list_node     node;
    THREAD_T      tid;
    THD_LOCK_T    thread_lock;
    THD_COND_T    thread_cond;
    pfn_dispatch  pfunc;
    void*         arg1;
    void*         arg2;	
    char          file[MAX_FILE_COMPILE_PATH];
    int           line;
    int           state;
    struct _thread_pool_s *pool;    
}thread_item_t;

typedef struct _thread_pool_s{
    THD_LOCK_T threads_lock;
    int        threads_num;

    int        max_threads;
    int        idle_threads;
    int        is_shutdown;

    THD_COND_T mIdleCond;
    THD_COND_T mFullCond;
    THD_COND_T mEmptyCond;

    bool         init;
    lib_list_node  idle_list;
    lib_list_node  busy_list;
}thread_pool_t;


#define  thread_dispatch(PFUN, ARG)  _thread_dispatch((PFUN), (ARG), NULL, __FILE__, __LINE__)
#define  thread_dispatch_new(PFUN, ARG1, ARG2) _thread_dispatch((PFUN), (ARG1), (ARG2), __FILE__, __LINE__)   

int thread_dispatch_with_tp(pfn_dispatch pfun, void* arg1, void* arg2, void *tp, char *file, int line);


extern int _thread_dispatch(pfn_dispatch pfun, void* arg1, void* arg2, char *file, int line);

extern void thread_pool_dump(void);

extern int  thread_pool_init(int max_thread);

extern int  thread_pool_finl(void);

extern void monitor_threads_num(void);

extern thread_pool_t *thread_pool_new(int max_thread);

void thread_insert_new(thread_pool_t *pool, thread_item_t *thread);
void thread_remove_new(thread_pool_t *pool, thread_item_t *thread);


#endif


/*
  * Header : lib-handle-pool.h
  * Copyright : All right reserved by SecWin, 2010
  * General handle pool.Like DB handle pool...
  * 
  * Ver   Author   Date         Desc
  * 0.1	wy      2011.6     initial version        
  */
  
#ifndef LIB_HANDLE_POOL_H
#define LIB_HANDLE_POOL_H

#define MAX_HANDLE_POOL_CNT     32
#define HANDLE_NONE             2                  // not ready
#define HANDLE_BUSY             1                  // in use
#define HANDLE_IDLE             0                  // can use
#define MAX_HANDLE_CTX_LEN      1024

#include <unistd.h>
#include <semaphore.h>
#include "lib-base.h"
#include "lib-thread.h"
#include "lib-list.h"
#include "lib-time.h"
#include "lib-debug.h"

typedef enum {
    HANDLE_RET_OK = 0,
    HANDLE_RET_NEED_MORE_DATA,
    HANDLE_RET_SERVER_UNREACHEABLE,
    HANDLE_RET_QUERY_ERROR,
}handle_err_e;

#define hp_info(...)  {lib_info(__VA_ARGS__);lib_syslog(MODULE_UTL, __VA_ARGS__);}
#define hp_error(...) {lib_error(MODULE_UTL,__VA_ARGS__);lib_syslog(MODULE_UTL, __VA_ARGS__);}

/* handle call back */
typedef void (*pfn_handle_cb)(void *handle_param, void *cb_param);

/* handle open, close */
typedef int (*pfn_handle_open)(void *handle);
typedef int (*pfn_handle_close)(void *handle);


/*database pool*/
typedef struct _handle_s {
    lib_list_node    node;
    int              idx;
    int              state;
	void            *handle;
	void            *ctx;
	char             file[MAX_FILE_COMPILE_PATH];
	int              line;
	pfn_handle_open  pfn_open;
	pfn_handle_close pfn_close;	
}handle_t, *phandle_t;

typedef struct _handle_pool_s {
	int         count;	
	THD_LOCK_T  lock;
	sem_t       sem;
	lib_list_node   handle_list;
}handle_pool_t, *phandle_pool_t;

#define hp_get_handle() _hp_get_handle(__FILE__, __LINE__)

#define hp_set_none(HD) ((HD)->state = HANDLE_NONE)
#define hp_set_idle(HD) ((HD)->state = HANDLE_IDLE)
#define hp_set_busy(HD) ((HD)->state = HANDLE_BUSY)
#define is_hp_none(HD)  ((HD)->state == HANDLE_NONE)


/* handle pool managment interface */
handle_t *_hp_get_handle(char *file, int line);

void hp_put_handle(handle_t *handle);
int  hp_reinit_handle(handle_t *handle);
int  hp_init(int count);
int  hp_finl(void);

int hp_new_handle(pfn_handle_open pfn_open, pfn_handle_close pfn_close, void *ctx);

int hp_keeplive_walk(void *param);




#endif


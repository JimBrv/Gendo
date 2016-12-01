/*
  * Header : lib-timer.h
  * Copyright : All right reserved by SecWin, 2010
  * list based timer support. Maybe use min-heap based timer for performance.
  * 
  * Ver   Author   Date         Desc
  * 0.1	wy      2010.8     initial version        
  */

#ifndef LIB_TIMER_H
#define LIB_TIMER_H

#include "lib-base.h"
#include "lib-types.h"
#include "lib-time.h"
#include "lib-poll.h"

typedef void (*pfn_timeout) (void *);
typedef void (*pfn_delete) (void *);

typedef struct _timer_s {
    lib_list_node   node;                   
	UInt32      expires;            // expire time in second
	pfn_timeout timeout_fun;        // timeout callback
	void       *timeout_param;          
	pfn_delete  delete_fun;         // delete callback
	void       *delete_param;
}lib_timer_t, *plib_timer_t;


int timer_init(lib_timer_t *tp, pfn_timeout t_fun, void *t_p, pfn_delete d_fun, void *d_p);

int has_timer(lib_timer_t *tp);

int timer_start(lib_timer_t *tp, unsigned long time);

int timer_stop(lib_timer_t *tp);

int timer_reset(lib_timer_t *tp, unsigned long time);

int timer_timeout_callback(void *p);

int timer_delete_callback(void *p);             

void show_timer(FILE * stream, char *tile);

#endif


/*
 * Filename : lib_socket.c
 * Copyright : All right reserved by SecWin, 2010
 * Simple list timer support.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
#include "lib-debug.h"
#include "lib-timer.h"


LIB_LIST_HEAD_STATIC(timer_list);

int timer_init(lib_timer_t *tp, pfn_timeout t_fun, void *t_p, pfn_delete d_fun, void *d_p)
{
	INIT_LIB_LIST(&(tp->node));
	tp->expires       = 0;
	tp->timeout_fun   = t_fun;
	tp->timeout_param = t_p;
	tp->delete_fun    = d_fun;
	tp->delete_param  = d_p;
	return OK;
}

int has_timer(lib_timer_t *tp)
{
	struct lib_list *ln;
	lib_timer_t     *tp1;

	lib_list_for_each(ln, &timer_list) {
		tp1 = lib_list_entry(ln, lib_timer_t, node);
		if (tp1 == tp) {
			return 1;
		}
	}

	return 0;
}

int timer_start(lib_timer_t *tp, unsigned long time)
{
	if (has_timer(tp)) {
		lib_error(MODULE_UTL, "invalid timer_start\n");
		return ERROR;
	}

	if (time == 0)		
		tp->expires = (unsigned long) 0 - 1;
	else 
		tp->expires = time;
	
	lib_list_add(&(tp->node), &timer_list);

	return OK;
}

int timer_stop(lib_timer_t *tp)
{
	if (!has_timer(tp)) {
		lib_error(MODULE_UTL, "invalid timer_stop\n");
		return ERROR;
	}

	lib_list_del(&(tp->node));
	tp->expires = 0;

	return OK;
}

int timer_reset(lib_timer_t *tp, unsigned long time)
{
	timer_stop(tp);
	timer_start(tp, time);
	return OK;
}

/* call back on timeout */
int timer_timeout_callback(void *p)
{
	unsigned long time = (unsigned long)p;
	struct lib_list *ln1, *ln2;
	lib_timer_t *tp;

	lib_list_for_each_safe(ln1, ln2, &timer_list) {
		tp = lib_list_entry(ln1, lib_timer_t, node);
		if (tp->expires == ((unsigned long) 0 - 1))
			continue;
		if (tp->expires > time) {
			tp->expires -= time;
			continue;
		}

		lib_list_del(&(tp->node));
		tp->expires = 0;
		if (tp->timeout_fun) 
			(tp->timeout_fun)(tp->timeout_param);
	}

	return OK;
}



/* call back on deleting from timer list */
int timer_delete_callback(void *p)
{
	unsigned long time = (unsigned long) p;
	struct lib_list *ln1, *ln2;
	lib_timer_t *tp;

	lib_list_for_each_safe(ln1, ln2, &timer_list) {
		tp = lib_list_entry(ln1, lib_timer_t, node);
		if (tp->expires == ((unsigned long) 0 - 1))
			continue;
		if (tp->expires > time) {
			tp->expires -= time;
			continue;
		}

		lib_list_del(&(tp->node));
		tp->expires = 0;
		if (tp->delete_fun) 
			(tp->delete_fun)(tp->delete_param);
	}

	return OK;
}


void show_timer(FILE * stream, char *tile)
{
	struct lib_list *ln;
	lib_timer_t *tp;

	fprintf(stream, "%s\n", tile);
	lib_list_for_each(ln, &timer_list) {
		tp = lib_list_entry(ln, lib_timer_t, node);
		fprintf(stream, "Timer %p: %u\n", tp, tp->expires);
	}
	fprintf(stream, "\n");
}


void Sleep(long wait)
{
	usleep(wait*1000); 
}
/*
 * Header : lib-msg-consumer.h
 * Copyright : All right reserved by SecWin, 2010
 * Consumer-Producer pattern with msg pool
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */

#ifndef LIB_MSG_CONSUMER_H
#define LIB_MSG_CONSUMER_H

#include "lib-msgpool.h"
#include "lib-thread.h"


/* consumer, producer interface */
typedef int (*pfn_msg_consumer)(void *param, msg_item_t *pmsg);
typedef int (*pfn_msg_producer)(void *param, msg_item_t *pmsg);

typedef struct _msg_consumer_s {
	char             name[MAX_NAME_LEN];
	int              run;
	int              init;
	pfn_msg_consumer consumer_fun;     /* consumer callback function */
	pfn_msg_producer producer_fun;     /* producer callback function */
	void            *consumer_param;   /* consumer param */
	void            *producer_param;   /*producer param */
	msg_pool_t       pool_cache;       /* pool for get/put msg */
	msg_pool_t       pool_consumer;    /* pool for consumer/producer trigger lock */
	int              is_multi_thread;  /* support multi-thread consumer worker */
}msg_consumer_t, *pmsg_consumer_t;



/* New mc with thread pool worker support */
int mc_multi_thread_init(msg_consumer_t  *mc, 
	                           char            *name,
	                           pfn_msg_consumer consumer_fun, 
	                           void            *consumer_param,
	                           pfn_msg_producer producer_fun,
	                           void            *producer_param,
	                           int              is_multi_thread);

/* make old mc programs happy */
#define mc_init(...)   mc_multi_thread_init(__VA_ARGS__, 0)

/* New mc with msg count, msg length specific */
int mc_init_with_msglen(msg_consumer_t  *mc,
                               char            *name,
                               pfn_msg_consumer consumer_fun, 
                               void            *consumer_param,
                               pfn_msg_producer producer_fun,
                               void            *producer_param,
                               int              count,
                               int              len,
                               int              is_multi_thread);
                               


/* Consumer interface */
int mc_consumer_run(msg_consumer_t *mc);
int mc_consumer_run_with_tp(msg_consumer_t *mc);



/* Producer intraface */
/* Return producer a msg */
msg_item_t *mc_producer_get_msg(msg_consumer_t *mc);
int         mc_producer_run(msg_consumer_t *mc, msg_item_t *pmsg);
int         mc_producer_get_msg_len(msg_consumer_t *mc);

void mc_put_msg_to_cache(msg_consumer_t *mc, msg_item_t *pmsg);

void mc_start(msg_consumer_t *mc);

void mc_stop(msg_consumer_t *mc);

void mc_finl(msg_consumer_t *mc);

#endif

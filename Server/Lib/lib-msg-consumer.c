/*
 * Filename : lib-msg-consumer.c
 * Copyright : All right reserved by SecWin, 2010
 * Classic consumer-producer pattern implementation
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
#include "lib-debug.h"
#include "lib-msg-consumer.h"

#define IS_MC_RUN(MC)  ((MC) && (MC)->init && (MC)->run)
#define mc_error(...) lib_error(MODULE_UTL, __VA_ARGS__)
#define mc_info(...)  lib_info(__VA_ARGS__)


/* wont block the caller */
static inline msg_item_t *mc_get_msg_from_cache(msg_consumer_t *mc)
{
    return  msg_pool_get(&mc->pool_cache, mc->pool_cache.msg_payload_len);
}

/* will block the caller if there is no msg in consumer pool */
static inline msg_item_t *mc_get_msg_from_consumer(msg_consumer_t *mc)
{
    return msg_pool_consumer(&mc->pool_consumer);
}
inline void mc_put_msg_to_cache(msg_consumer_t *mc, msg_item_t *pmsg)
{
    return msg_pool_put(&mc->pool_cache, pmsg);
}
static inline void mc_put_msg_to_consumer(msg_consumer_t *mc, msg_item_t *pmsg)
{
    return msg_pool_producer(&mc->pool_consumer, pmsg);
}

int mc_multi_thread_init(msg_consumer_t  *mc,
                               char            *name,
                               pfn_msg_consumer consumer_fun, 
                               void            *consumer_param,
                               pfn_msg_producer producer_fun,
                               void            *producer_param,
                               int              is_multi_thread)
{
    int ret = ERROR;
    if (!mc) goto out;
    if (msg_pool_init(&mc->pool_cache, MSG_POOL_TYPE_CACHE, mc->pool_cache.count, mc->pool_cache.msg_payload_len)) {
        mc_error("msg consumer : init pool cache failed!\n");
        goto out;
    }
    
    if (msg_pool_init(&mc->pool_consumer, MSG_POOL_TYPE_CONSUMER_PRODUCER, 0, 0)) {
        mc_error("msg consumer : init pool consumer failed!\n");
        goto out;
    }
    
    mc->consumer_fun   = consumer_fun;
    mc->consumer_param = consumer_param;
    mc->producer_fun   = producer_fun;
    mc->producer_param = producer_param;
    mc->is_multi_thread= is_multi_thread;
    strncpy(mc->name, name, MAX_NAME_LEN);
    mc->init = 1;   
    ret = OK;   
  out:
    return ret;
}


/* New MC initiator 
 * To specify the msg pool's capacity and msg length.
 * Especially for some envrionment which needs large msg. 
 * For exmp, UI/Shell etc.
 */
int mc_init_with_msglen(msg_consumer_t  *mc,
                               char            *name,
                               pfn_msg_consumer consumer_fun, 
                               void            *consumer_param,
                               pfn_msg_producer producer_fun,
                               void            *producer_param,
                               int              count,
                               int              len,
                               int              is_multi_thread)
{
    int ret = ERROR;
    if (!mc) goto out;
    if (msg_pool_init(&mc->pool_cache, MSG_POOL_TYPE_CACHE, count, len)) {
        mc_error("msg consumer : init pool cache failed!\n");
        goto out;
    }

    if (msg_pool_init(&mc->pool_consumer, MSG_POOL_TYPE_CONSUMER_PRODUCER, count, len)) {
        mc_error("msg consumer : init pool consumer failed!\n");
        goto out;
    }

    mc->consumer_fun   = consumer_fun;
    mc->consumer_param = consumer_param;
    mc->producer_fun   = producer_fun;
    mc->producer_param = producer_param;
    mc->is_multi_thread= is_multi_thread;
    strncpy(mc->name, name, MAX_NAME_LEN);
    mc->init = 1;   
    ret = OK;   
out:
    return ret;
}


void mc_stop(msg_consumer_t *mc)
{
    if (mc) mc->run = 0;
}

void mc_start(msg_consumer_t *mc)
{
    if (mc) mc->run = 1;
}

void mc_finl(msg_consumer_t *mc)
{
    if (IS_MC_RUN(mc)) {
    mc_info("msg consumer : running, can't final it. stop it first.\n");
    mc_stop(mc);
    }
    mc->run = 0;    
    mc->init = 0;
    mc->consumer_fun   = NULL;
    mc->consumer_param = NULL;
    mc->producer_fun   = NULL;
    mc->producer_param = NULL;
    msg_pool_finl(&mc->pool_cache);
    msg_pool_finl(&mc->pool_consumer);
}


/* multi thread function wrapper makes consumer function 
 *  fit thread pool function 
 */
void mc_consumer_thread_wrapper(void *arg1, void *arg2)
{
    msg_consumer_t *mc  = (msg_consumer_t*)arg1;
    msg_item_t     *pmsg= (msg_item_t*)arg2;
    mc->consumer_fun(mc->consumer_param, pmsg);

    /* reclaim msg to cache pool */
    mc_put_msg_to_cache(mc, pmsg);
}


/* consumer will run loop */
int mc_consumer_run(msg_consumer_t *mc)
{
    msg_item_t *pmsg = NULL;

    while(IS_MC_RUN(mc)) {
        pmsg = mc_get_msg_from_consumer(mc);
        if (!pmsg) {
            mc_error("msg consumer : consumer pool return NULL, bug found!!\n");
            break;
        }
        if (mc->consumer_fun && mc->is_multi_thread == 0) {
            /* single thread dispatcher */
            mc->consumer_fun(mc->consumer_param, pmsg);
            /* reclaim msg to cache */
            mc_put_msg_to_cache(mc, pmsg);
        }else if (mc->consumer_fun && mc->is_multi_thread) {
            /* multi thread dispatcher */
            thread_dispatch_new(mc_consumer_thread_wrapper, (void*)mc, (void*)pmsg);
            //lib_info("TP consumer: msg(0x%x) => thd(%s), tp(0x%p)\n", pmsg->msg.msg, mc->name, mc->consumer_param);
            //thread_dispatch_with_tp(mc_consumer_thread_wrapper, (void*)mc, (void*)pmsg, mc->consumer_param, __FILE__, __LINE__);
        }

    }
    return OK;
}

int mc_consumer_run_with_tp(msg_consumer_t *mc)
{
    msg_item_t *pmsg = NULL;

    while(IS_MC_RUN(mc)) {
        pmsg = mc_get_msg_from_consumer(mc);
        if (!pmsg) {
            mc_error("msg consumer : consumer pool return NULL, bug found!!\n");
            break;
        }
        if (mc->consumer_fun && mc->is_multi_thread == 0) {
            /* single thread dispatcher */
            mc->consumer_fun(mc->consumer_param, pmsg);
            /* reclaim msg to cache */
            mc_put_msg_to_cache(mc, pmsg);
        }else if (mc->consumer_fun && mc->is_multi_thread) {
            /* multi thread dispatcher */
            //thread_dispatch_new(mc_consumer_thread_wrapper, (void*)mc, (void*)pmsg);
            //lib_info("TP consumer: msg(0x%x) => thd(%s), tp(0x%p)\n", pmsg->msg.msg, mc->name, mc->consumer_param);
            thread_dispatch_with_tp(mc_consumer_thread_wrapper, (void*)mc, (void*)pmsg, mc->consumer_param, __FILE__, __LINE__);
        }

    }
    return OK;
}


inline int mc_producer_get_msg_len(msg_consumer_t *mc)
{
    return (mc ? (mc->pool_cache.msg_payload_len) : 0);
}



/* Return producer a msg */
inline msg_item_t *mc_producer_get_msg(msg_consumer_t *mc)
{
    return mc_get_msg_from_cache(mc);
}



/* producer inserts msg to consumer list, just runs once */
int mc_producer_run(msg_consumer_t *mc, msg_item_t *pmsg)
{
    if (!pmsg) {
        mc_error("msg consumer : producer recv an NULL msg!!!\n");
        return ERROR;
    }
            
    if (mc->producer_fun) {
        mc->producer_fun(mc->producer_param, pmsg);
    }
    
    mc_put_msg_to_consumer(mc, pmsg);
    return OK;
}


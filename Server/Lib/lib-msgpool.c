/*
 * Filename : lib-msgpool.c
 * Copyright : All right reserved by SecWin, 2010
 * msg pool list header, acts as consumer-productor pattern, with 
 * multi-thread safety.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
#include "lib-debug.h"
#include "lib-msgpool.h"

#define mp_error(...) lib_error(MODULE_UTL, __VA_ARGS__)
#define mp_info(...)  lib_info(__VA_ARGS__)

#define CHECK_POOL_CACHE(MP) ((MP) && ((MP)->type == MSG_POOL_TYPE_CACHE)) 
#define CHECK_POOL_CONSUMER(MP) ((MP) && ((MP)->type == MSG_POOL_TYPE_CONSUMER_PRODUCER)) 
#define CHECK_POOL(MP) \
	((MP) && ((MP)->type == MSG_POOL_TYPE_CACHE || (MP)->type == MSG_POOL_TYPE_CONSUMER_PRODUCER))

int msg_pool_init(msg_pool_t *mp, MSG_POOL_TYPE_E type, UInt32 count, UInt32 len)
{
    int i = 0;
    lib_list_node *pNode, *pNodeTp;
    msg_item_t *pmsg;

    if (!mp) { 
	    mp_error("msg pool init is NULL!!!\n");
	    goto err_out;
    }

    if (type == MSG_POOL_TYPE_CACHE && count == 0) {
	    count = MSG_POOL_DEFAULT_COUNT;
    }
	
    if (type == MSG_POOL_TYPE_CACHE && len == 0) {
	    len = MSG_POOL_DEFAULT_PAYLOAD_LEN;
    }	
	
    if (count > MSG_POOL_MAX_COUNT) { 
	    mp_error("msg pool count=%d exceeds max=%d!\n", count, MSG_POOL_MAX_COUNT);
	    goto err_out;
    }
    INIT_LIB_LIST(&mp->msg_list);

    for (i = 0; i < count; i++) {
    	msg_item_t *pmsg = (msg_item_t *)mem_malloc(sizeof(msg_item_t) + len);
    	if (!pmsg) goto err_out;
    	lib_list_add(&pmsg->node, &mp->msg_list);	
    }

    if (thd_lock_init(&mp->lock, NULL)) {
    	mp_error("msg pool init lock failed\n");
    	goto err_out;
    }

    if (thd_cond_init(&mp->cond, NULL)) {
    	mp_error("msg pool init cond failed!\n");
    	goto err_cond;
    }
	
    mp->type  = type;
    mp->count = count;
	mp->limit = count;
    mp->msg_payload_len = len;

    mp_info("msg pool init ok, type=%d, count=%d, len=%d\n", 
    	    mp->type, mp->count, mp->msg_payload_len);
    return OK;
	
  err_cond:
    thd_lock_finl(&mp->lock);
	
  err_out:

    lib_list_for_each_safe(pNode, pNodeTp, &mp->msg_list) {
    	pmsg = lib_list_entry(pNode, msg_item_t, node);
    	lib_list_del(&pmsg->node);
    	mem_free(pmsg);		
    }
    mp->count = 0;
	
    return ERROR;	
}

void  msg_pool_finl(msg_pool_t *mp)
{
    lib_list_node *pNode, *pNodeTp;
    msg_item_t *pmsg;

    if (!mp && mp->count != 0) { 
	mp_error("msg pool is NULL or count != 0!!!\n");
	return;
    }
	
    thd_lock(&mp->lock);
    if (mp->count == 0) {
		thd_cond_signal(&mp->cond);
    }	
    thd_unlock(&mp->lock);
	

    lib_list_for_each_safe(pNode, pNodeTp, &mp->msg_list) {
	pmsg = lib_list_entry(pNode, msg_item_t, node);
	lib_list_del(&pmsg->node);
	mem_free(pmsg);		
    }
    mp->count = mp->limit = 0;
    mp->msg_payload_len = 0;
    thd_lock_finl(&mp->lock);
    thd_cond_finl(&mp->cond);
    mp_info("msg pool final ok\n");	
}

#define MSG_POOL_CHECK(MP) ((MP)&&((MP)->count > 0))

/*
 * Consumer function will block until one msg has being putted in list
 * must be used with msg_pool_productor() in pairs.
 *
 */
msg_item_t *msg_pool_consumer(msg_pool_t *mp)
{
    msg_item_t *pmsg     = NULL;
    lib_list_node *pNode = NULL;
	
    thd_lock(&mp->lock);
    if (!CHECK_POOL_CONSUMER(mp)) {
	mp_error("pool consumer, error pool type\n");
	goto out;
    }
	
    /* wait until productor put msg */
    if (mp->count == 0) {
	thd_cond_wait(&mp->cond, &mp->lock);
    }

    pNode = (&(mp->msg_list))->next;
    if (!pNode) {
	mp_error("pool consumer had item, but list is NULL, bug found!!!!\n");
	goto out;
    }
    pmsg  = lib_list_entry(pNode, msg_item_t, node);
    lib_list_del(&(pmsg->node));
    mp->count--;
  out:
    thd_unlock(&mp->lock);
    return pmsg;
}

void msg_pool_producer(msg_pool_t *mp, msg_item_t *pmsg)
{
    if (!mp || !pmsg) return;
	
    thd_lock(&mp->lock);
    if (!CHECK_POOL_CONSUMER(mp)) {
	mp_error("pool producer, error pool type\n");
	goto out;
    }

    /* put msg to tail */
    lib_list_add_tail(&(pmsg->node), &(mp->msg_list));
    mp->count++;

    /* notify waiting consumer */
    if (mp->count == 1) {
	thd_cond_signal(&mp->cond);
    }
  out:		
    thd_unlock(&mp->lock);
}


/*
 * The pool acts as a msg cache. To facinitate the getting/putting
 * msg operations.
 *
 * Distinguish these pool cache funtion  pairs with the 
 * consumer/productor function pairs
 *
 * The caller won't be blocked by this pool cache interface
 */
msg_item_t *msg_pool_get(msg_pool_t *mp, int msg_payload_len)
{
    msg_item_t *pmsg = NULL;
    lib_list_node *pNode;
	
    thd_lock(&mp->lock);
	
    if (!CHECK_POOL_CACHE(mp)) goto out;
	 
    if (mp->msg_payload_len >= msg_payload_len && mp->count > 0) {
		pNode = (&(mp->msg_list))->next;
		if (!pNode) {
	    	mp_error("pool cache get item, but list is NULL, bug found!!!!\n");
	    	goto out;
		}
		pmsg  = lib_list_entry(pNode, msg_item_t, node);
		lib_list_del(&(pmsg->node));
		/* not reinitiate the msg:) */
		mp->count--;
    }else{
    	if (mp->limit >= MSG_POOL_MAX_COUNT) {		
	    	mp_error("pool cache exceeds the MAX_LIMIT (%d),  !!!!!!\n", MSG_POOL_MAX_COUNT);
	    	goto out;
    	}
	
		pmsg = mem_malloc(sizeof(msg_item_t) + msg_payload_len);
		if (!pmsg) {			
	    	mp_error("pool cache get item, malloc failed!\n");
	    	goto out;
		}
		mp->limit++;
    }
  out:
    thd_unlock(&mp->lock);
    return pmsg;
			
}
void msg_pool_put(msg_pool_t *mp, msg_item_t *pmsg)
{
    thd_lock(&mp->lock);
    if (!CHECK_POOL_CACHE(mp)) goto out;

    /* Need reclaim the memory on some environment ?? */
	
    /* Add to head, make cpu cache happy */
    lib_list_add(&pmsg->node, &mp->msg_list);
    mp->count++;
	
  out:
    thd_unlock(&mp->lock);
}

void msg_pool_dump(msg_pool_t *mp)
{
    lib_list_node *pNode = NULL;
    msg_item_t    *pmsg  = NULL;
	
    thd_lock(&mp->lock);

    mp_info("msg pool dump now!!! pool info: pool_type=[%d], msg_cnt=[%d], payload_len=[%d]\n", 
	    mp->type, mp->count, mp->msg_payload_len);
    lib_list_for_each(pNode, &mp->msg_list) {
	pmsg = lib_list_entry(pNode, msg_item_t, node);
	mp_info("msg info: peer=[0x%x], msg=[0x%x], payload_len=[%d]\n", 
		pmsg->peer, pmsg->msg.msg, pmsg->msg.len);
    }
	
    thd_unlock(&mp->lock);
}

void msg_pool_purge(msg_pool_t *mp)
{
    lib_list_node *pNode = NULL, *pNodeTp = NULL;
    msg_item_t *pmsg = NULL;
		
    thd_lock(&mp->lock);

    if (CHECK_POOL(mp)) goto out;
		
    lib_list_for_each_safe(pNode, pNodeTp, &mp->msg_list) {
    	pmsg = lib_list_entry(pNode, msg_item_t, node);
    	lib_list_del(&pmsg->node);
    	mem_free(pmsg);	
    }
	
    mp->count = 0;
    INIT_LIB_LIST(&mp->msg_list);
  out:
    thd_unlock(&mp->lock);

}

/* 
 * No length check here,should be garanteed by caller.
 * And no link list to copy :)
 */
inline int  msg_item_copy(msg_item_t *pmsg_dst, msg_item_t *pmsg_src)
{
    if (!pmsg_dst || !pmsg_src) return ERROR;
    INIT_LIB_LIST(&pmsg_dst->node);
    pmsg_dst->peer    = pmsg_src->peer;
    pmsg_dst->msg.msg = pmsg_src->msg.msg;
    pmsg_dst->msg.len = pmsg_src->msg.len;
    memcpy(pmsg_dst->msg.payload, pmsg_src->msg.payload, pmsg_src->msg.len);
    return OK;
}


inline int msg_copy(msg_head_t *pmsg_dst, msg_head_t *pmsg_src)
{
    if (!pmsg_dst || !pmsg_src) return ERROR;
	lib_assert(pmsg_src->len <= MAX_MSG_LEN);
	memcpy(pmsg_dst, pmsg_src, sizeof(msg_head_t));
	memcpy(pmsg_dst->payload, pmsg_src->payload, pmsg_src->len);
	return OK;
}

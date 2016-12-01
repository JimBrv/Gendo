/*
 * Header : lib-msgpool.h
 * Copyright : All right reserved by SecWin, 2010
 * msg pool list header, acts as consumer-productor pattern, with 
 * multi-thread safety.
 * 
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */

#ifndef LIB_MSGPOOL_H
#define LIB_MSGPOOL_H

#include "lib-base.h"
#include "lib-types.h"
#include "lib-msg.h"
#include "lib-list.h"
#include "lib-thread.h"


#define MSG_POOL_DEFAULT_COUNT       10000           /* default pool items for cache */
#define MSG_POOL_MAX_COUNT           100000          /* max pool items for cache, 100000 */
#define MSG_POOL_DEFAULT_PAYLOAD_LEN 1024            /* default msg payload for cache */


typedef struct _msg_item_s {
	lib_list_node node;
	void         *peer;           /* the IO for msg sent back */
	msg_head_t    msg;            /* contains msg_head and payload*/
}msg_item_t, *pmsg_item_t;

/* pool's type */
typedef enum {
	MSG_POOL_TYPE_CONSUMER_PRODUCER = 1,
	MSG_POOL_TYPE_CACHE,
}MSG_POOL_TYPE_E;

typedef struct _msg_pool_s {
	int           type;                /* pool type */
	UInt32        limit;               /* pool max item limit */
	UInt32        count;               /* pool current items */
	UInt32        msg_payload_len;     /* pool default msg payload */
	THD_LOCK_T    lock; 
	THD_COND_T    cond;
	lib_list_node msg_list;	
}msg_pool_t, *pmsg_pool_t;

#define msg_pool_consumer_productor_init(MP) \
		msg_pool_init((MP), MSG_POOL_TYPE_CONSUMER_PRODUCER, 0, 0)
		
#define msg_pool_cache_init(MP, CNT, LEN)  \
		msg_pool_init((MP), MSG_POOL_TYPE_CACHE, (CNT), (LEN))


int   msg_pool_init(msg_pool_t *mp, MSG_POOL_TYPE_E type, UInt32 count, UInt32 len);
void  msg_pool_finl(msg_pool_t *mp);

msg_item_t *msg_pool_consumer(msg_pool_t *mp);
void        msg_pool_producer(msg_pool_t *mp, msg_item_t *pmsg);


msg_item_t *msg_pool_get(msg_pool_t *mp, int msg_payload_len);
void        msg_pool_put(msg_pool_t *mp, msg_item_t *pmsg);


void msg_pool_dump(msg_pool_t *mp);

int  msg_item_copy(msg_item_t *pmsg_dst, msg_item_t *pmsg_src);

int  msg_copy(msg_head_t *pmsg_dst, msg_head_t *pmsg_src);


#endif

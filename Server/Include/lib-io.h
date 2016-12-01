/*
 * Header : lib-io.h
 * Copyright : All right reserved by SecWin, Extelecom, 2011
 * IO abstract class for socket epoll duplexer.
 * lib-poll sometimes needs to duplexes IO data 
 * from one io side to another io side, or vise else.
 * 
 * So the IO class has pointer to peer.
 * 
 * Ver   Author   Date         Desc
 * 0.1  wy      2011.3      initial version        
 */
 
#ifndef LIB_IO_H
#define LIB_IO_H

#include "lib-base.h"
#include "lib-types.h"
#include "lib-list.h"
#include "lib-bit.h"
#include "lib-msg.h"
#include "lib-hash.h"

typedef int (*pfn_io_encode)(void *io, msg_head_t *msg, char *buf);
typedef int (*pfn_io_decode)(void *io, msg_head_t *msg, char *buf);

typedef struct _lib_io_s_ {
    HASH_NODE_T   node;                  // global list
    void         *peer;                  // to peer IO
    int           state;                 // current IO, peer's state
    pfn_io_encode pfun_encode;
    pfn_io_decode pfun_decode;
    int           fd;                    
    char          ip[MAX_NAME_LEN];
    int           port;
	unsigned int  idx;                   // global idx for self
    unsigned int  svccount;              // svc duplux counter
    int           kp;                    // keeplive failed count
    void         *param1;
    void         *param2;
    long          last_time;             // last busy time
    unsigned int  txtotal;               // send total byte
    unsigned int  rxtotal;               // recv total byte
    unsigned int  txbyte;                // send byte
    unsigned int  rxbyte;                // recv byte
    unsigned int  flag;                  // io flags 
    char          txbuf[MID_MSG_LEN];    // send buf
    char          rxbuf[MID_MSG_LEN];    // recv buf
}lib_io_t, *plib_io_t;

typedef struct _lib_io_set_ {
    HASH_TABLE_T  *ht;
    int            cnt;
}lib_io_set_t, *plib_io_set_t;

#define io_get_peer(SIO)       ((lib_io_t*)((SIO)->peer))
#define io_set_peer(SIO,PIO)   (((SIO)->peer)=(void*)(PIO))

#define io_get_param1(SIO)     ((SIO)->param1)
#define io_set_param1(SIO, P1) (((SIO)->param1)=(void*)(P1))
#define io_get_param2(SIO)     ((SIO)->param2)
#define io_set_param2(SIO, P2) (((SIO)->param2)=(void*)(P2))

typedef enum {
    IO_STATE_SELF_BIT    = 1,
    IO_STATE_PEER_BIT    = 2,
    IO_STATE_OUTSIDE_BIT = 4,
    IO_STATE_OK_BIT      = 8,
}IO_STATE_BIT;

#define io_set_self_ready(SIO)      set_bit((SIO)->state, IO_STATE_SELF_BIT)
#define io_unset_self_ready(SIO)    unset_bit((SIO)->state, IO_STATE_SELF_BIT)
#define is_io_self_ready(SIO)       is_bit_set((SIO)->state, IO_STATE_SELF_BIT)
#define io_set_peer_ready(SIO)      set_bit((SIO)->state, IO_STATE_PEER_BIT)
#define io_unset_peer_ready(SIO)    unset_bit((SIO)->state, IO_STATE_PEER_BIT)
#define is_io_peer_ready(SIO)       is_bit_set((SIO)->state, IO_STATE_PEER_BIT)

/* IO outside/inside type */
#define io_set_outside(SIO)         set_bit((SIO)->state, IO_STATE_OUTSIDE_BIT)
#define io_set_inside(SIO)          unset_bit((SIO)->state, IO_STATE_OUTSIDE_BIT)
#define is_io_outside(SIO)          is_bit_set((SIO)->state, IO_STATE_OUTSIDE_BIT) 
#define is_io_inside(SIO)           (!is_io_outside((SIO)))

/* IO working state */
#define io_set_ok(SIO)         set_bit((SIO)->state, IO_STATE_OK_BIT)
#define io_set_stop(SIO)       unset_bit((SIO)->state, IO_STATE_OK_BIT)
#define is_io_ok(SIO)          is_bit_set((SIO)->state, IO_STATE_OK_BIT) 
#define is_io_stop(SIO)        (!is_io_ok((SIO)))


#define io_init(SIO) do { \
    memset((SIO), 0, sizeof(lib_io_t)); \
    hash_init(&(SIO)->node); \
}while(0)


/* IO base operation */
lib_io_t *io_get_by_fd(lib_io_set_t *pset, const int fd);
int       io_add(lib_io_set_t *pset, lib_io_t *pio);
int       io_del(lib_io_set_t *pset, lib_io_t *pio);
int       io_del_by_fd(lib_io_set_t *pset, const int fd);
lib_io_set_t *io_set_new();
lib_io_t     *io_new();




#endif

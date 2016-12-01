/*
  * Header : lib-libevent.h
  * Copyright : All right reserved by SecWin, 2010
  * Libevent wrapper, for older lib-poll.c has some uncertain bug,
  * use the powerful libevent-2.0.x as the new event engine.
  * 
  * Ver   Author   Date         Desc
  * 0.1	wy      2011.8      initial version        
  */

#ifndef LIB_LIBEVENT_H
#define LIB_LIBEVENT_H

#include "lib-debug.h"
#include "lib-thread.h"
#include "lib-socket.h"
#include "lib-poll.h"
#include "lib-msg.h"
#include "lib-timer.h"
#include "lib-poll.h"

#include <event2/event-config.h>
#include <event.h>
#include <event2/listener.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>


/* 
  * new event engine with libevent-2.0, 
  * No socket, signal, timer differences, No fd <=> callback function map.
  * all things will be done well by libevent. 
  * simplly and efficiently than older pol
  */
typedef struct _libevt_pool_s_ {
    int                max;                  // max limit
    int                cur;                  // current items
    THD_LOCK_T         lock;                 // maybe used someday
    struct event_base *evt_base;             // libevent base
    lib_list_node      node;                 // libevt_t list items
    void              *ctx;                  // callback param
}libevt_pool_t, *plibevt_pool_t;


/* libevent item, for each connection */
typedef struct _libevt_s_ {
    lib_list_node           node;                 // linked into evt_pool
    struct event            evt;                  // libevent needs it
    int                     fd;                   // 
    struct timeval          last_time;            // last active time 
    int                     idle_timeout;         // is idle timeout
    libevt_pool_t          *evt_pool;             // to libevt_pool root
    event_callback_fn       new_cb;               // callback on new
    event_callback_fn       delete_cb;            // callback on delete
    event_callback_fn       event_cb;             // callback on event
    void                   *ctx;                  // some param
}libevt_t, *plibevt_t;

#define libevt_pool_get_evt_base(EVTPOOL) ((EVTPOOL)->evt_base)
#define libevt_pool_get_ctx(EVTPOOL)      ((EVTPOOL)->ctx)

#define libevt_get_pool(EVT)              ((EVT)->evt_pool)
#define libevt_get_evt(EVT)               (&((EVT)->evt))
#define libevt_get_ctx(EVT)               ((EVT)->ctx)


/*
  * To make the server easy change from poll to libevt
  * import acceptor, connector in new event engine
  *
  */
typedef struct _libevt_acceptor_s {
    int                   flag;                // acceptor for new fd param: non-blocking?
    char                  ip[MAX_NAME_LEN];
    int                   port;
	int                   accept_fd;
	int                   socket_timeout;      // new fd timeout. blocking socket useful
	int                   idle_timeout;        // for idle socket timeout
	void                 *evt_pool;            // evt_pool root
	void                 *listener;            // listener of libevent
	void                 *ctx;                 // context, should be msg_consumer_t
	event_callback_fn     accept_cb;           // on event callback
    void                 *accept_cb_param;     // event param
	event_callback_fn     event_cb;            // on event callback
    void                 *event_cb_param;      // event param
}libevt_acceptor_t, *plibevt_acceptor_t;

typedef struct _libevt_connector_s {
    int                   flag;                // fd flag: non-blocking, blocking?
    char                  ip[MAX_NAME_LEN];
    int                   port;    
	int                   connect_fd;
	int                   socket_timeout;      // fd timeout. blocking socket useful
	void                 *evt_pool;            // evt_pool root
	void                 *ctx;                 // context
	event_callback_fn     event_cb;            // on event callback
    void                 *event_cb_param;      // event param
}libevt_connector_t, *plibevt_connector_t;


/* libevt_pool operation */
libevt_pool_t *libevt_pool_new(int max, void *ctx);
void           libevt_pool_set_ctx(libevt_pool_t *evt_pool, void *ctx);

/* libevt operation */
libevt_t *libevt_new(libevt_pool_t      *evt_pool, 
                       int                 fd, 
                       event_callback_fn   new_cb,
                       event_callback_fn   del_cb,
                       event_callback_fn   event_cb,
                       void               *ctx);
int       libevt_del(libevt_t *evt);
int       libevt_clean(libevt_t *evt);


/*
 * wrapper for accept, connect
 */
int libevt_acceptor(libevt_pool_t *evt_pool,   libevt_acceptor_t *acceptor);
int libevt_connector(libevt_pool_t *evt_pool, libevt_connector_t *connector);

/* new libevent interface for timer, signal handler */
int libevt_timer(libevt_pool_t *evt_pool, int time, event_callback_fn timer_cb, void *param);

int libevt_signal(libevt_pool_t *evt_pool, int sig, event_callback_fn signal_cb, void *param);


int libevt_run(libevt_pool_t *evt_pool);

#endif

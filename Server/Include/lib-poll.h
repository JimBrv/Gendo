/*
  * Header : lib-poll.h
  * Copyright : All right reserved by SecWin, 2010
  * epoll wrapper & IO reactor and dupulexer
  * 
  * Ver   Author   Date         Desc
  * 0.1	wy      2010.8     initial version        
  */

#ifndef LIB_POLL_H
#define LIB_POLL_H

#include <signal.h>
#include <sys/epoll.h>
#include "lib-base.h"
#include "lib-list.h"
#include "lib-hash.h"

#define POLL_MAX_EVENT_NUM   50000                  // max poll fd num
#define POLL_HASH_SIZE       50000        
#define POLL_MAGIC           0x12345678
#define POLL_MAX_CLEAN_IDLE_FD_TIME 60*20            // cleanup idle fd per 20 min    

#define POLL_TIMEOUT_FOREVER  -1                      
#define POLL_TIMEOUT_DEFAULT  POLL_TIMEOUT_FOREVER  // default timeout is forever

enum poll_type{
	POLL_EVENT_SERVER,
	POLL_EVENT_TIMER,
	POLL_EVENT_SIGNAL
};

enum poll_event_type {
	POLL_EVENT_IN  = 1,
	POLL_EVENT_OUT = 2,
	POLL_EVENT_ERR = 4
};

enum poll_func_action {
	POLL_FUNC_IDLE,
	POLL_FUNC_DEL
};

enum poll_conn_type {
	POLL_CONN_SHORT,
	POLL_CONN_LONG
};

enum poll_socket_type {
	POLL_TCP,
	POLL_UDP,
	POLL_FILE
};

typedef int (*pfn_poll_svr) (int fd, int type, void *param);
typedef int (*pfn_poll_tmr) (void *param);
typedef int (*pfn_poll_itr) (void *param);
typedef int (*pfn_poll_sig) (int sig_no, void *param);
typedef int (*pfn_poll_del) (int fd, void *psvr, void *param);


typedef struct _poll_poll_s {
	int magic;
    HASH_TABLE_T    *server_list;
	struct lib_list  timer_list;
	struct lib_list  signal_list;
	pfn_poll_itr     pFun_itr;
	void            *pItr_param;
	void            *pSig_srv;
	int              last_act_srv;
	int              last_act_tmr;
	int              last_act_sig;
	int              last_act_fd;
	int              ep_fd;
	int              max_event;
    int              max_conn;
	int              pipe_fd[2];
	int              min_timeout;
	int              pid;
	int              run;
}poll_poll_t, *ppoll_poll_t;

typedef struct _poll_server_s {
	HASH_NODE_T        node;
	struct epoll_event ep_event;
	poll_poll_t       *pPoll;
	int                server_fd;
	pfn_poll_svr       pFun_svr;
	void              *pParam;
	pfn_poll_del       pFun_del;
	void              *pParam_del;
    long               last_time;
    int                idle_timeout;
}poll_server_t, *ppoll_server_t;

typedef struct _poll_signal_s {
	lib_list_node     node;
	poll_poll_t  *pPoll;
	int           sig_no;
	pfn_poll_sig  pFun_sig;
	void         *pParam;
	int           notify;
	struct sigaction old_sig;
}poll_signal_t, *ppoll_signal_t;

typedef struct _poll_timer_s {
	lib_list_node     node;
	poll_poll_t  *pPoll;
	pfn_poll_tmr  pFun_tmr;
	void         *pParam;
	int           timeout;
	int           times;
	int           notify_times;
	long          last_notify;
}poll_timer_t, *ppoll_timer_t;

/* Acceptor pattern */
typedef struct _poll_acceptor_s {
	void                 *pPoll;
	int                   accept_fd;
	int                   socket_timeout;
	enum poll_socket_type socket_type;
	enum poll_conn_type   conn_type;
	pfn_poll_svr          pFun_Accept;
	void                 *param;
}poll_acceptor_t, *ppoll_acceptor_t;


/* Connector pattern */
typedef struct _poll_connector_s {
	void                 *pPoll;
	int                   connect_fd;
	int                   socket_timeout;
	enum poll_socket_type socket_type;
	enum poll_conn_type   conn_type;
	pfn_poll_svr          pFun_Connect;
	void                 *param;
}poll_connector_t, *ppoll_connector_t;


/* Acceptor + IO duplexer pattern */
typedef struct _poll_duplexer_s {
	void                 *pPoll;
	int                   accept_fd;
	int                   socket_timeout;
	enum poll_socket_type socket_type;
	enum poll_conn_type   conn_type;
	pfn_poll_svr          pfun_peer;
	pfn_poll_svr          pfun_accept;
    pfn_poll_del          pfun_del;
	void                 *param_peer;
	void                 *param_accept;
    void                 *param_del;
}poll_duplexer_t, *ppoll_duplexer_t;


/* UDP recvor pattern */
typedef struct _poll_recvor_s {
	void                 *pPoll;
	int                   recv_fd;
	int                   socket_timeout;
	enum poll_socket_type socket_type;
	enum poll_conn_type   conn_type;
	pfn_poll_svr          pFun_Recv;
	void                 *param;
}poll_recvor_t, *ppoll_recvor_t;
		
/* poll reactor initialization & finalization */
extern void  poll_init();
extern int   poll_stop(void *pPollNode);
extern void *poll_new(int max_event);
extern void  poll_free(void *pPollNode);


#if 0
extern void *poll_add_server(void *pPollNode, 
                                  int fd, 
                                  pfn_poll_svr pFunSrv,
				                  void *pParam);
#endif

#define poll_add_server(...) poll_add_server_with_idle_timeout(__VA_ARGS__, 0) 
/* poll reactor operations */
extern void *poll_add_server_with_idle_timeout(void *pPollNode, 
                                                          int   fd, 
                                                          pfn_poll_svr pFunSrv,
                        				                  void *pParam,
                        				                  int   idle_timeout);

extern void *poll_get_server(void *pPoll, const int fd);
extern int   poll_del_server(void *pSrvNode);
extern int   poll_del_server_by_fd(void *pPollNode, int fd);


extern int   poll_set(void *pSrvNode, int event);
extern int   poll_unset(void *pSrvNode, int event);
extern void *poll_add_timer(void *pPollNode, long timeout, long times, pfn_poll_tmr pFunTmr, void *pParam);
extern int   poll_del_timer(void *pTmrNode);
extern void *poll_add_signal(void *pPollNode, int sigNum, pfn_poll_sig pFunSig, void *pParam);
extern int   poll_del_signal(void *pSigNode);
extern int   poll_wait(void *pPollNode, int timeout);
extern int   poll_set_interrupt(void *pPollNode, pfn_poll_itr pFunItr, void *pParm);
extern int   poll_unset_interrupt(void *pPollNode);
extern void  poll_dump(void *pPollNode);

/* Wrapper listen, accept operation serials */
extern void *poll_acceptor(poll_acceptor_t * pAccept);
extern void *poll_connector(poll_connector_t *pConnect);
extern void *poll_io_duplexer(poll_duplexer_t * pIoDuplex);

extern void *poll_recvor(poll_recvor_t *pRecvor);

/* Cleanup idle fd in timer */
extern int poll_timer_clean_idle_fd(void *pTmrNode);



#endif

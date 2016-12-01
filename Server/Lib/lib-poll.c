/*
 * Filename : lib-poll.c
 * Copyright : All right reserved by SecWin, 2010
 * Epoll wrapper & IO reactor and dupulexer, refrence LibEvent, ACE reactor pattern 
 * for IO du
 * 
 * Ver   Author   Date         Desc
 * 0.1  wy      2010.8     initial version        
 */

#include "lib-base.h"
#include "lib-debug.h"
#include "lib-socket.h"
#include "lib-thread.h"
#include "lib-hash.h"
#include "lib-list.h"
#include "lib-time.h"
#include "lib-mem.h"

#include "lib-poll.h"

typedef struct _poll_base_s {
    struct lib_list     poll_list;
    pthread_mutex_t lock;
} poll_base_t;

typedef struct _poll_base_node_s {
    lib_list_node    node;
    poll_poll_t *pPoll;
} poll_base_node_t;

poll_base_t *pBase = NULL;

static int poll_server_safe(poll_poll_t *pPoll, int fd) {
    if ((pPoll->last_act_srv != POLL_FUNC_DEL)
        || ((pPoll->last_act_srv == POLL_FUNC_DEL) 
        && (pPoll->last_act_fd != fd))) {
        return OK;
    }else {
        return ERROR;
    }
}


int poll_call_server(poll_server_t *pSrv, int type)
{
    int ret = ERROR;
    pSrv->pPoll->last_act_srv = POLL_FUNC_IDLE;
    pSrv->pPoll->last_act_fd = -1;
    if (pSrv->pFun_svr != NULL) {
        ret = pSrv->pFun_svr(pSrv->server_fd, type, pSrv->pParam);
    }
    return ret;
}

int poll_call_timer(poll_timer_t *pTmr)
{
    int ret = 0;
    pTmr->pPoll->last_act_tmr = POLL_FUNC_IDLE;
    pTmr->pPoll->last_act_fd = -1;
    if (pTmr->pFun_tmr != NULL) {
        ret = pTmr->pFun_tmr(pTmr->pParam);
    }
    return ret;
}

int poll_call_signal(poll_signal_t *pSig)
{
    int ret = 0;
    pSig->pPoll->last_act_sig = POLL_FUNC_IDLE;
    pSig->pPoll->last_act_fd = -1;
    if (pSig->pFun_sig != NULL) {
        ret = pSig->pFun_sig(pSig->sig_no, pSig->pParam);
    }
    return ret;
}

static int poll_set_pipe(int fd)
{
    int flags = 0;

    flags = fcntl(fd, F_GETFL);
    if (flags < 0) {
        lib_error(MODULE_UTL, "get pipe error [errno:%d]\n", errno);
        return ERROR;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        lib_error(MODULE_UTL, "set pipe error [errno:%d]\n", errno);
        return ERROR;
    }
    return OK;
}

static void poll_sig(poll_poll_t * pPoll)
{
    lib_list_node *pNode = NULL;
    lib_list_node *pNodeTp = NULL;
    poll_signal_t *pSig = NULL;

    lib_list_for_each_safe(pNode, pNodeTp, &(pPoll->signal_list)) {
        pSig = lib_list_entry(pNode, poll_signal_t, node);
        if (pSig->notify > 0) {
            pSig->notify = 0;
            poll_call_signal(pSig);
        }
    }
}

static poll_signal_t *poll_get_sig_by_no(poll_poll_t * pPoll, const int sig_no)
{
    lib_list_node *pNode = NULL;
    poll_signal_t* pSig = NULL;

    lib_list_for_each(pNode, &(pPoll->signal_list)) {
        pSig = lib_list_entry(pNode, poll_signal_t, node);
        if (pSig->sig_no == sig_no) {
            return pSig;
        }
    }
    return NULL;
}

static void poll_signal_handle(int signum)
{
    int errnoTmp = 0;
    poll_poll_t *pPoll = NULL;
    poll_signal_t* pSig = NULL;
    poll_base_node_t *pBaseNode = NULL;
    lib_list_node *pNode = NULL;
    lib_list_node *pNodeTp = NULL;

    errnoTmp = errno;

    if (pBase == NULL) {
        errno = errnoTmp;
        return;
    }

    pthread_mutex_lock(&(pBase->lock));
    lib_list_for_each_safe(pNode, pNodeTp, &(pBase->poll_list)) {
        pBaseNode = lib_list_entry(pNode, poll_base_node_t, node);
        if(pBaseNode == NULL) {
            break;
        }
        pPoll = pBaseNode->pPoll;
        if(pPoll == NULL) {
            break;
        }
        if (pPoll->magic != POLL_MAGIC) {
            continue;
        }
        if (pPoll->pid != getpid()) {
            continue;
        }
        pSig = poll_get_sig_by_no(pPoll, signum);
        if (pSig == NULL) {
            continue;
        }
        if (pPoll->pipe_fd[1] > 0) {
            pSig->notify = 1;
            write(pPoll->pipe_fd[1], "1", 1);
        }
    }
    pthread_mutex_unlock(&(pBase->lock));

    errno = errnoTmp;
}

static inline int poll_hash_fd(int fd)
{
    return fd % POLL_HASH_SIZE;
}

static int poll_hash_creat_key(HASH_NODE_T *pNode, void *p)
{
    poll_server_t *pSrv = NULL;

    pSrv = hash_entry(pNode, poll_server_t, node);
    return poll_hash_fd(pSrv->server_fd);
}

static int poll_compare_fd(HASH_NODE_T *pNode, void *p)
{
    int fd = *((int *) p);
    poll_server_t *pServer = hash_entry(pNode, poll_server_t, node);

    if (fd == pServer->server_fd) {
        return ERROR;
    }
    return OK;
}

static poll_server_t *poll_get_server_by_fd(poll_poll_t *pPoll, const int fd)
{
    HASH_NODE_T *pNode = NULL;
    int key = poll_hash_fd(fd);
    poll_server_t *pSrv = NULL;

    pNode = hash_walk(pPoll->server_list, key, poll_compare_fd, (void *) &fd);
    if (pNode == NULL) {
        return NULL;
    }
    pSrv = hash_entry(pNode, poll_server_t, node);

    return pSrv;
}

static void poll_del_fd(poll_poll_t *pPoll, int fd)
{
    struct epoll_event clientEvent;

    memset(&clientEvent, 0, sizeof (clientEvent));
    epoll_ctl(pPoll->ep_fd, EPOLL_CTL_DEL, fd, &clientEvent);
}

static void poll_free_srv_list(HASH_NODE_T * pSrvNode)
{
    poll_server_t *pSrv = NULL;

    if (pSrvNode == NULL) return;
    
    pSrv = hash_entry(pSrvNode, poll_server_t, node);
    poll_del_fd(pSrv->pPoll, pSrv->server_fd);

    mem_free(pSrv);
}

static void poll_free_sig_list(lib_list_node * pSigNode)
{
    poll_signal_t* pSig = NULL;

    if (pSigNode == NULL) return;
    
    pSig = lib_list_entry(pSigNode, poll_signal_t, node);
    mem_free(pSig);
}

static void poll_free_tmr_list(lib_list_node * pTmrNode)
{
    poll_timer_t *pTmr = NULL;

    if (pTmrNode == NULL) return;
    
    pTmr = lib_list_entry(pTmrNode, poll_timer_t, node);
    mem_free(pTmr);
}

static int poll_self_fun_signal(int fd, int type, void *param)
{
    poll_poll_t *pPoll = (poll_poll_t *)param;
    char rev;

    if (pPoll == NULL) {
        return ERROR;
    }
    if (type == POLL_EVENT_IN) {
        read(fd, &rev, 1);
        poll_sig(pPoll);
    }
    return OK;
}

static void poll_print_server(HASH_NODE_T * node)
{
    poll_server_t *pSrv = hash_entry(node, poll_server_t, node);

    lib_info("\tfd[%d],pFun[%p],pParam[%p]", pSrv->server_fd, pSrv->pFun_svr, pSrv->pParam);
}

static void poll_init_timer(poll_poll_t *pPoll, int default_timeout)
{
    poll_timer_t *pTmr = NULL;
    lib_list_node *pNode = NULL;
    long now_time = lib_time_get();
    int min_timeout = default_timeout;

    lib_list_for_each(pNode, &(pPoll->timer_list)) {
        pTmr = lib_list_entry(pNode, poll_timer_t, node);
        pTmr->last_notify = now_time;
        if (pTmr->timeout == POLL_TIMEOUT_DEFAULT) {
            pTmr->timeout = default_timeout;
        }
        if (pTmr->timeout < min_timeout) {
            min_timeout = pTmr->timeout;
        }
    }
    pPoll->min_timeout = min_timeout;
}

static void poll_check_timer(poll_poll_t *pPoll, long now_time)
{
    poll_timer_t *pTmr = NULL;
    lib_list_node *pNode = NULL;
    lib_list_node *pNodeTp = NULL;

    lib_list_for_each_safe(pNode, pNodeTp, &(pPoll->timer_list)) {
        pTmr = lib_list_entry(pNode, poll_timer_t, node);
        if ((now_time - pTmr->last_notify) >= pTmr->timeout) {
            if ((pTmr->notify_times >= pTmr->times) && (pTmr->times != 0)) {
                poll_del_timer((void *) pTmr);
            } else {
                pTmr->notify_times++;
                pTmr->last_notify = now_time;
                poll_call_timer(pTmr);
            }
        }
    }
}

static poll_base_node_t *poll_base_get(poll_poll_t *pPoll)
{
    lib_list_node *pNode = NULL;
    poll_base_node_t *pBaseNode = NULL;
    bool   bFound = false;

    if (pBase == NULL) {
        return NULL;
    }
    
    pthread_mutex_lock(&(pBase->lock));
    lib_list_for_each(pNode, &(pBase->poll_list)) {
        pBaseNode = lib_list_entry(pNode, poll_base_node_t, node);
        if (pBaseNode->pPoll == pPoll) {
            bFound = true;
            break;
        }
    }
    pthread_mutex_unlock(&(pBase->lock));
    return bFound ? pBaseNode : NULL;
}

static int poll_base_add(poll_poll_t *pPoll)
{
    poll_base_node_t *pBaseNode = NULL;

    if (pBase == NULL) {
        pBase = mem_malloc(sizeof(poll_base_t));
        if (pBase == NULL) {
            return ERROR;
        }
        INIT_LIB_LIST(&(pBase->poll_list));
        pthread_mutex_init(&(pBase->lock), NULL);
    }

    if (poll_base_get(pPoll) != NULL) {
        return OK;
    }
    
    pBaseNode = (poll_base_node_t *)mem_malloc(sizeof(poll_base_node_t));
    if (pBaseNode == NULL) {
        return ERROR;
    }
    pBaseNode->pPoll = pPoll;
    
    pthread_mutex_lock(&(pBase->lock));
    lib_list_add(&(pBaseNode->node), &(pBase->poll_list));
    pthread_mutex_unlock(&(pBase->lock));

    return OK;
}

static int poll_base_del(poll_poll_t *pPoll)
{
    poll_base_node_t *pBaseNode = NULL;

    if (pBase == NULL) {
        return OK;
    }
    pBaseNode = poll_base_get(pPoll);
    if (pBaseNode != NULL) {
        pthread_mutex_lock(&(pBase->lock));
        lib_list_del(&(pBaseNode->node));
        pthread_mutex_unlock(&(pBase->lock));
        mem_free(pBaseNode);
    }

    return OK;
}

inline void *poll_get_server(void *pPollNode, const int fd)
{
    return (void *)poll_get_server_by_fd((poll_poll_t*)pPollNode, fd);
}

void poll_init()
{
    pBase = NULL;
}

void *poll_new(int max_event)
{
    poll_poll_t *pPoll = NULL;

    pPoll = (poll_poll_t *)mem_malloc(sizeof (poll_poll_t));
    if (!pPoll) {
        lib_error(MODULE_UTL,"_malloc poll_poll_t failed\n");
        return NULL;
    }

    pPoll->ep_fd = epoll_create(max_event);
    if (pPoll->ep_fd < 0) {
        lib_error(MODULE_UTL,"epoll_create failed [errno:%d]\n", errno);
        mem_free(pPoll);
        return NULL;
    }

    pPoll->magic = POLL_MAGIC;
    pPoll->server_list = hash_create(POLL_HASH_SIZE, poll_hash_creat_key, NULL);
    if (pPoll->server_list == NULL) {
        lib_error(MODULE_UTL,"hash_create failed\n");
        mem_free(pPoll);
        return NULL;
    }

    INIT_LIB_LIST(&(pPoll->timer_list));
    INIT_LIB_LIST(&(pPoll->signal_list));
    pPoll->max_event  = max_event;
    pPoll->max_conn   = 0;
    pPoll->pid        = getpid();
    pPoll->pFun_itr   = NULL;
    pPoll->pItr_param = NULL;

    return (void *) pPoll;
}

void poll_free(void *pPollNode)
{
    poll_poll_t *pPoll = (poll_poll_t *) pPollNode;

    if (pPoll == NULL) {
        lib_error(MODULE_UTL, "poll_free entry NULL\n");
        return;
    }

    if (pPoll->magic != POLL_MAGIC) {
        lib_error(MODULE_UTL, "poll_free epoll magic error\n");
        return;
    }

    poll_stop(pPollNode);
    hash_free_all(pPoll->server_list, poll_free_srv_list);
    lib_list_free(&(pPoll->signal_list), poll_free_sig_list);
    lib_list_free(&(pPoll->timer_list), poll_free_tmr_list);

    lib_close(pPoll->ep_fd);
    if(pPoll->pipe_fd[0] > 0) {
        lib_close(pPoll->pipe_fd[0]);
    }
    if(pPoll->pipe_fd[1] > 0) {
        lib_close(pPoll->pipe_fd[1]);
    }
    poll_base_del(pPoll);
    mem_free(pPollNode);
}

void *poll_add_server_with_idle_timeout(void *pPollNode, 
                                                          int   fd, 
                                                          pfn_poll_svr pFunSrv,
                        				                  void *pParam,
                        				                  int   idle_timeout)
{
    int ret = 0;
    poll_server_t *pSrv = NULL;
    poll_poll_t *pPoll = (poll_poll_t *) pPollNode;

    if ((pPoll == NULL) || (pFunSrv == NULL) || (fd < 0)) {
        return NULL;
    }

    pSrv = (poll_server_t *)mem_malloc(sizeof(poll_server_t));
    if (!pSrv) {
        return NULL;
    }

    pSrv->server_fd = fd;
    pSrv->pFun_svr  = pFunSrv;
    pSrv->pParam    = pParam;
    pSrv->pPoll     = pPoll;
    pSrv->idle_timeout=idle_timeout;

    pSrv->ep_event.data.fd = fd;
    pSrv->ep_event.events  = EPOLLIN|EPOLLERR|EPOLLHUP;

    ret = epoll_ctl(pPoll->ep_fd, 
                    EPOLL_CTL_ADD, 
                    pSrv->server_fd, 
                    &(pSrv->ep_event));
    
    if (ret < 0) {
        lib_error(MODULE_UTL, "fd[%d] EPOLL_CTL_ADD faild [ep_fd:%d],[errno:%d]\n",
                 pSrv->server_fd, pPoll->ep_fd, errno);
        mem_free(pSrv);
        return NULL;
    }
    pPoll->max_conn++;
    hash_add(&(pSrv->node), pPoll->server_list);
    return (void *)pSrv;

}


#if 0
void *poll_add_server(void *pPollNode, int fd, pfn_poll_svr pFunSrv, void *pParam)
{
    int ret = 0;
    poll_server_t *pSrv = NULL;
    poll_poll_t *pPoll = (poll_poll_t *) pPollNode;

    if ((pPoll == NULL) || (pFunSrv == NULL) || (fd < 0)) {
        return NULL;
    }

    pSrv = (poll_server_t *)mem_malloc(sizeof(poll_server_t));
    if (!pSrv) {
        return NULL;
    }

    pSrv->server_fd = fd;
    pSrv->pFun_svr  = pFunSrv;
    pSrv->pParam    = pParam;
    pSrv->pPoll     = pPoll;

    pSrv->ep_event.data.fd = fd;
    pSrv->ep_event.events  = EPOLLIN|EPOLLERR|EPOLLHUP;

    ret = epoll_ctl(pPoll->ep_fd, 
                    EPOLL_CTL_ADD, 
                    pSrv->server_fd, 
                    &(pSrv->ep_event));
    
    if (ret < 0) {
        lib_error(MODULE_UTL, "fd[%d] EPOLL_CTL_ADD faild [ep_fd:%d],[errno:%d]\n",
                 pSrv->server_fd, pPoll->ep_fd, errno);
        mem_free(pSrv);
        return NULL;
    }
    pPoll->max_conn++;
    hash_add(&(pSrv->node), pPoll->server_list);
    return (void *)pSrv;
}
#endif 

int poll_del_server(void *pSrvNode)
{
    poll_server_t *pSrv = (poll_server_t *) pSrvNode;
    poll_poll_t *pPoll = NULL;

    if (pSrv == NULL) {
        return ERROR;
    }

    pPoll = pSrv->pPoll;
    if (pPoll == NULL) {
        return ERROR;
    }
    
    if (pSrv->server_fd <= 0 || 
        pSrv->server_fd >= POLL_MAX_EVENT_NUM) {
        lib_error(MODULE_UTL, "del server=0x%p, fd=%d illegal!\n", pSrv, pSrv->server_fd);
        return ERROR;
    }

    pPoll->last_act_srv = POLL_FUNC_DEL;
    pPoll->last_act_fd  = pSrv->server_fd;
    pPoll->max_conn--;
    poll_del_fd(pPoll, pSrv->server_fd);
    hash_del(&(pSrv->node), pPoll->server_list);
    
    if (pSrv->server_fd > 0) {
        lib_close(pSrv->server_fd);
        if (pSrv->pFun_del) {
            pSrv->pFun_del(pSrv->server_fd, (void*)pSrv, (void*)pSrv->pParam_del);
        }        
        pSrv->server_fd = 0;
    }
    mem_free(pSrv);
    
    return OK;
}


int poll_del_server_by_fd(void *pPollNode, int fd)
{
    poll_server_t *pSrv = NULL;
    poll_poll_t *pPoll = (poll_poll_t *) pPollNode;

    pSrv = poll_get_server_by_fd(pPoll, fd);
    if (pPoll == NULL) {
        return ERROR;
    }

    return poll_del_server((void *) pSrv);
}

int poll_set(void *pSrvNode, int event)
{
    int ret = 0;
    poll_server_t *pSrv = NULL;
    poll_poll_t *pPoll = NULL;

    if (pSrvNode == NULL) {
        return ERROR;
    }
    
    pSrv = (poll_server_t *) pSrvNode;
    pPoll = pSrv->pPoll;

    if (pPoll == NULL) {
        return ERROR;
    }

    if (event & POLL_EVENT_IN)
        pSrv->ep_event.events |= EPOLLIN;
    if (event & POLL_EVENT_OUT)
        pSrv->ep_event.events |= EPOLLOUT;
    if (event & POLL_EVENT_ERR)
        pSrv->ep_event.events |= EPOLLERR;

    ret = epoll_ctl(pPoll->ep_fd, 
                    EPOLL_CTL_MOD, 
                    pSrv->server_fd,
                    &(pSrv->ep_event));
    
    if (ret < 0) {
        lib_error(MODULE_UTL,
                 "fd[%d] SET EPOLL_CTL_MOD faild [ep_fd:%d],[errno:%d]\n",
                pSrv->server_fd, pPoll->ep_fd, errno);
        return ERROR;
    }

    return OK;
}

int poll_unset(void *pSrvNode, int event)
{
    int ret = 0;
    poll_server_t *pSrv = NULL;
    poll_poll_t *pPoll = NULL;

    if (pSrvNode == NULL) {
        return ERROR;
    }
    
    pSrv = (poll_server_t *)pSrvNode;
    pPoll = pSrv->pPoll;

    if (pPoll == NULL) {
        return ERROR;
    }

    if (event & POLL_EVENT_IN)
        pSrv->ep_event.events &= ~EPOLLIN;
    if (event & POLL_EVENT_OUT)
        pSrv->ep_event.events &= ~EPOLLOUT;
    if (event & POLL_EVENT_ERR)
        pSrv->ep_event.events &= ~EPOLLERR;

    ret = epoll_ctl(pPoll->ep_fd, 
                    EPOLL_CTL_MOD, 
                    pSrv->server_fd, 
                    &(pSrv->ep_event));
    
    if (ret < 0) {
        lib_error(MODULE_UTL,
                 "fd[%d] UNSET EPOLL_CTL_MOD faild [ep_fd:%d],[%s:%d],[errno:%d]\n",
                 pSrv->server_fd, pPoll->ep_fd, errno);
        return ERROR;
    }

    return OK;
}

int poll_set_interrupt(void *pPollNode, pfn_poll_itr pFunItr, void *pParm)
{
    poll_poll_t *pPoll = (poll_poll_t *) pPollNode;

    if ((pPoll == NULL) || (pFunItr == NULL)) {
        return ERROR;
    }

    pPoll->pFun_itr = pFunItr;
    pPoll->pItr_param = pParm;

    return OK;
}

int poll_unset_interrupt(void *pPollNode)
{
    poll_poll_t *pPoll = (poll_poll_t *) pPollNode;

    if (pPoll == NULL) {
        return ERROR;
    }

    pPoll->pFun_itr = NULL;
    pPoll->pItr_param = NULL;

    return OK;
}

void *poll_add_timer(void *pPollNode, long timeout, long times,
                         pfn_poll_tmr pFunTmr, void *pParam)
{
    poll_timer_t *pTmr = NULL;
    poll_poll_t *pPoll = (poll_poll_t *) pPollNode;

    if ((pPoll == NULL)) {
        return NULL;
    }
    if ((timeout <= 0) 
        && (timeout != POLL_TIMEOUT_DEFAULT)) {
        return NULL;
    }

    pTmr = (poll_timer_t *)mem_malloc(sizeof (poll_timer_t));
    if (!pTmr) {
        return NULL;
    }

    pTmr->last_notify = 0;
    pTmr->pFun_tmr = pFunTmr;
    pTmr->pParam   = pParam;
    pTmr->pPoll    = pPoll;
    pTmr->timeout  = timeout;
    pTmr->times    = times;

    lib_list_add(&(pTmr->node), &(pPoll->timer_list));

    return (void *)pTmr;
}

int poll_del_timer(void *pTmrNode)
{
    poll_timer_t *pTmr = NULL;
    poll_poll_t *pPoll = NULL;
    lib_list_node *pNode = NULL;

    if (pTmrNode == NULL) {
        return ERROR;
    }

    pTmr = (poll_timer_t *)pTmrNode;
    pPoll = pTmr->pPoll;

    if (pPoll == NULL) {
        return ERROR;
    }

    pPoll->last_act_tmr = POLL_FUNC_DEL;
    lib_list_del(&(pTmr->node));
    mem_free(pTmr);
    pPoll->min_timeout = 0;
    lib_list_for_each(pNode, &(pPoll->timer_list)) {
        pTmr = lib_list_entry(pNode, poll_timer_t, node);
        if (pPoll->min_timeout == 0) {
            pPoll->min_timeout = pTmr->timeout;
        }
        if (pTmr->timeout < pPoll->min_timeout) {
            pPoll->min_timeout = pTmr->timeout;
        }
    }
    return OK;
}

void *poll_add_signal(void *pPollNode, int sigNum, pfn_poll_sig pFunSig, void *pParam)
{
    poll_signal_t *pSig  = NULL;
    poll_poll_t   *pPoll = (poll_poll_t *) pPollNode;
    struct sigaction sig;

    if ((pPoll == NULL)) {
        return NULL;
    }

    if (poll_base_add(pPoll) < 0) {
        lib_error(MODULE_UTL, "init sig base error\n", errno);
        return NULL;
    }

    pSig = (poll_signal_t *)mem_malloc(sizeof(poll_signal_t));
    if (!pSig) {
        return NULL;
    }
    if (lib_list_empty(&(pPoll->signal_list))) {
        if (pipe(pPoll->pipe_fd) < 0) {
            lib_error(MODULE_UTL, "pipe error [errno:%d]\n",
                     errno);
            pPoll->pipe_fd[0] = 0;
            pPoll->pipe_fd[1] = 0;
            mem_free(pSig);
            return NULL;
        }

        if (poll_set_pipe(pPoll->pipe_fd[0]) < 0) {
            lib_close(pPoll->pipe_fd[0]);
            lib_close(pPoll->pipe_fd[1]);
            pPoll->pipe_fd[0] = 0;
            pPoll->pipe_fd[1] = 0;
            mem_free(pSig);
            return NULL;
        }
        if (poll_set_pipe(pPoll->pipe_fd[1]) < 0) {
            lib_close(pPoll->pipe_fd[0]);
            lib_close(pPoll->pipe_fd[1]);
            pPoll->pipe_fd[0] = 0;
            pPoll->pipe_fd[1] = 0;
            mem_free(pSig);
            return NULL;
        }

        pPoll->pSig_srv = poll_add_server(pPollNode, 
                                          pPoll->pipe_fd[0], 
                                          poll_self_fun_signal, 
                                          (void *)pPoll);
        if (pPoll->pSig_srv == NULL) {
            lib_error(MODULE_UTL, "add signal fun error\n");
            lib_close(pPoll->pipe_fd[0]);
            lib_close(pPoll->pipe_fd[1]);
            pPoll->pipe_fd[0] = 0;
            pPoll->pipe_fd[1] = 0;
            mem_free(pSig);
            return NULL;
        }
        poll_set(pPoll->pSig_srv, POLL_EVENT_IN|POLL_EVENT_ERR);
    }

    pSig->pFun_sig = pFunSig;
    pSig->pParam = pParam;
    pSig->pPoll = pPoll;
    pSig->sig_no = sigNum;
    
    lib_list_add(&(pSig->node), &(pPoll->signal_list));
    

    memset(&sig, 0, sizeof(struct sigaction));
    memset(&(pSig->old_sig), 0, sizeof(struct sigaction));
    sig.sa_handler = poll_signal_handle;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;
    if (sigaction(sigNum, &sig, &(pSig->old_sig)) < 0) {
        lib_error(MODULE_UTL, "add signal error\n");
        lib_list_del(&(pSig->node));
        mem_free(pSig);
        return NULL;
    }

    return (void *) pSig;
}

int
poll_del_signal(void *pSigNode)
{
    poll_signal_t *pSig = NULL;
    poll_poll_t   *pPoll = NULL;
    struct sigaction old_sig;

    if (pSigNode == NULL) {
        return ERROR;
    }

    pSig  = (poll_signal_t*)pSigNode;
    pPoll = pSig->pPoll;

    if (pPoll == NULL) {
        return ERROR;
    }
    
    sigaction(pSig->sig_no, &(pSig->old_sig), &old_sig);

    pPoll->last_act_sig = POLL_FUNC_DEL;
    lib_list_del(&(pSig->node));
    mem_free(pSig);
    if (lib_list_empty(&(pPoll->signal_list))) {
        poll_del_server(pPoll->pSig_srv);
        lib_close(pPoll->pipe_fd[0]);
        lib_close(pPoll->pipe_fd[1]);
        pPoll->pipe_fd[0] = 0;
        pPoll->pipe_fd[1] = 0;
    }
    return OK;
}

static int poll_set_fd_timeout(int fd, int timeout)
{
    struct timeval tv;
    int retIn = 0;

    if (timeout <= 0) {
        return OK;
    }
    memset(&tv, 0, sizeof (tv));
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    retIn = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *) (&tv),
               sizeof (struct timeval));
    if (retIn < 0) {
        lib_error(MODULE_UTL, "setsockopt err fd[%d]\n", fd);
        return ERROR;
    }
    return OK;
}



/* acceptor's real function to accept a new fd, 
  * and put the new fd to reactor
  */
static int poll_accept(int fd, int type, void *param)
{
    poll_acceptor_t *pAccept = (poll_acceptor_t *)param;
    int acceptFd = 0;
    int retIn = 0;
    void *pSrv = NULL;

    if(pAccept == NULL) {
        return ERROR;
    }
    
    if (type == POLL_EVENT_IN) {
            //while(true) {
            //acceptFd = lib_accept_nonblock(fd); 
            poll_poll_t *ppoll = (poll_poll_t *)pAccept->pPoll;
            if (ppoll->max_conn >= (ppoll->max_event - 10)) {
                lib_error(MODULE_UTL, "poll fd(%d) exceeds max(%d), refuse connect!\n", ppoll->max_conn,ppoll->max_event);
                acceptFd = lib_accept(fd);
                if (acceptFd < 0) {
                    lib_error(MODULE_UTL, "accept fd[%d] error\n", fd);
                    return ERROR;
                }else{
                    lib_close(acceptFd);
                    return OK;
                }
            }        

            acceptFd = lib_accept(fd);
            if (acceptFd < 0) {
                lib_error(MODULE_UTL, "accept fd[%d] error\n", fd);
                return ERROR;
            }
            /*
                      if (acceptFd == OK) {
                          return OK;
                      }
                      */
            
            retIn = poll_set_fd_timeout(acceptFd, pAccept->socket_timeout);
            if (retIn < 0) {
                lib_error(MODULE_UTL, "set fd[%d] timeout error\n", fd);
                lib_close(acceptFd);
                return ERROR;
            }

            /* Add new fd to reactor */
            #if 0
            pSrv = poll_add_server(pAccept->pPoll, 
                                   acceptFd,
                                   pAccept->pFun_Accept, 
                                   pAccept->param);
            #endif
            pSrv = poll_add_server_with_idle_timeout(pAccept->pPoll, 
                                   acceptFd,
                                   pAccept->pFun_Accept, 
                                   pAccept->param,
                                   1);
            
            if (pSrv == NULL) {
                lib_error(MODULE_UTL, "add fd[%d] control error\n", fd);
                lib_close(acceptFd);
                return ERROR;
            }         

        //}
        //poll_set(pSrv, POLL_EVENT_IN|POLL_EVENT_ERR);
    } else {
        pAccept->pFun_Accept(fd, POLL_EVENT_ERR, pAccept->param);
        return ERROR;
    }
    return OK;
}

/* Acceptor for server fd, especially for TCP */
void *poll_acceptor(poll_acceptor_t *pAccept)
{
    void *pSrv = NULL;

    if (pAccept == NULL) {
        return NULL;
    }
    if ((pAccept->accept_fd < 0)
        || (pAccept->pPoll == NULL)
        || (pAccept->pFun_Accept == NULL)) {
        return NULL;
    }

    pSrv = poll_add_server(pAccept->pPoll, 
                   pAccept->accept_fd,
                   poll_accept, 
                   (void *)pAccept);
    if (pSrv == NULL) {
        return NULL;
    }
    //poll_set(pSrv, POLL_EVENT_IN|POLL_EVENT_ERR);
    return pSrv;
}

/* Connector for non-server fd */
void *poll_connector(poll_connector_t *pConnect)
{
    void *pSrv = NULL;
    int    ret = 0;

    if (pConnect == NULL) {
        return NULL;
    }
    if ((pConnect->connect_fd < 0)
        || (pConnect->pPoll == NULL)
        || (pConnect->pFun_Connect == NULL)) {
        return NULL;
    }
    
    ret = poll_set_fd_timeout(pConnect->connect_fd, pConnect->socket_timeout);
    if (ret < 0) {
        lib_error(MODULE_UTL, "set fd[%d] timeout error\n", pConnect->connect_fd);
        return ERROR;
    }
    
    pSrv = poll_add_server(pConnect->pPoll, 
                   pConnect->connect_fd,
                   pConnect->pFun_Connect, 
                   (void *)pConnect->param);
    if (pSrv == NULL) {
        return NULL;
    }
    //poll_set(pSrv, POLL_EVENT_IN|POLL_EVENT_ERR);
    return pSrv;
}


/* recvor callback wrapper */
static int poll_recvfrom(int fd, int type, void *param)
{
    poll_recvor_t *pRecv = (poll_recvor_t *)param;

    if(pRecv == NULL) {
        return ERROR;
    }
    
    if (type == POLL_EVENT_IN) {
        pRecv->pFun_Recv(fd, POLL_EVENT_IN, pRecv->param);
    } else {
        pRecv->pFun_Recv(fd, POLL_EVENT_ERR, pRecv->param);
        return ERROR;
    }
    return OK;
}


/* Recvor for UDP fd */
void *poll_recvor(poll_recvor_t *pRecvor)
{
    void *pSrv = NULL;

    if (pRecvor == NULL) {
        return NULL;
    }
    if ((pRecvor->recv_fd < 0)
        || (pRecvor->pPoll == NULL)
        || (pRecvor->pFun_Recv == NULL)) {
        return NULL;
    }

    pSrv = poll_add_server(pRecvor->pPoll, 
                           pRecvor->recv_fd,
                           poll_recvfrom, 
                           (void *)pRecvor);
    if (pSrv == NULL) {
        return NULL;
    }
    
    //poll_set(pSrv, POLL_EVENT_IN|POLL_EVENT_ERR);
    return pSrv;
}


void poll_dump(void *pPollNode)
{
    poll_poll_t   *pPoll = (poll_poll_t *) pPollNode;
    poll_signal_t *pSig = NULL;
    poll_timer_t  *pTmr = NULL;
    poll_base_node_t *pBaseNode = NULL;
    lib_list_node *pNode = NULL;

    lib_info("\n---------------------------------------------------------------\n");
    lib_info("pBase[%p],pid[%d]\n", pBase, getpid());
    if (pBase != NULL) {
        lib_list_for_each(pNode, &(pBase->poll_list)) {
            pBaseNode = lib_list_entry(pNode, poll_base_node_t, node);
            lib_info("\tpPoll[%p]\n", pBaseNode->pPoll);
        }
    }
    lib_info("poll[%p],epoll_fd[%d],max_event[%d],min_timeout[%d],pipe_fd[%d-%d]\n",
            pPoll, pPoll->ep_fd, pPoll->max_event, pPoll->min_timeout,
            pPoll->pipe_fd[0], pPoll->pipe_fd[1]);
    lib_info("\nsignal monitor:\n");
    lib_list_for_each(pNode, &(pPoll->signal_list)) {
        pSig = lib_list_entry(pNode, poll_signal_t, node);
        lib_info("\tsignal[%d],pFun[%p],pParam[%p]\n", pSig->sig_no, pSig->pFun_sig, pSig->pParam);
    }
    lib_info("\ntimer monitor:\n");
    lib_list_for_each(pNode, &(pPoll->timer_list)) {
        pTmr = lib_list_entry(pNode, poll_timer_t, node);
        lib_info("\ttimeout[%d],times[%d],notify_times[%d],last_notify[%ld],pFun[%p],pParam[%p]\n",
                pTmr->timeout, pTmr->times, pTmr->notify_times,
                pTmr->last_notify, pTmr->pFun_tmr, pTmr->pParam);
    }
    lib_info("\nfd monitor:\n");
    hash_walk_all(pPoll->server_list, poll_print_server);
    lib_info("---------------------------------------------------------------\n");
}

int poll_stop(void *pPollNode)
{
    poll_poll_t *pPoll = (poll_poll_t *) pPollNode;

    if(pPoll == NULL) {
        return ERROR;
    }

    pPoll->run = 0;
    return OK;
}

static struct epoll_event epoll_ev[POLL_MAX_EVENT_NUM];

int poll_wait(void *pPollNode, int timeout)
{
    int i = 0;
    int nfds = 0;
    int tOut = 0;
    long lasttime = 0;
    long nowtime = 0;
    int run_timeout = timeout;
    poll_poll_t *pPoll = (poll_poll_t *) pPollNode;

    if (pPoll == NULL) {
        return ERROR;
    }

    if ((hash_empty(pPoll->server_list))
        &&(lib_list_empty(&(pPoll->timer_list)))
        &&(lib_list_empty(&(pPoll->signal_list))))
    {
        lib_error(MODULE_UTL, "poll is empty");
        return OK;
    }

    if ((timeout < -1)
        ||((timeout == -1)
        &&(!lib_list_empty(&(pPoll->timer_list))))) 
    {
        lib_error(MODULE_UTL, "timeout invalid"); 
        return ERROR;
    }

    memset(&epoll_ev, 0, sizeof(struct epoll_event)*POLL_MAX_EVENT_NUM);
    poll_init_timer(pPoll, timeout);

    pPoll->run = 1;
    lasttime = lib_time_get() + run_timeout;
    while (pPoll->run) {
        if (pPoll->magic != POLL_MAGIC) {
            lib_error(MODULE_UTL, "epoll_wait epoll magic error\n");
            return ERROR;
        }
        
        if (timeout >= 0) {
            nowtime = lib_time_get();
            poll_check_timer(pPoll, nowtime);
            if (nowtime >= lasttime) {
                lasttime = nowtime + run_timeout;
            }
            tOut = (lasttime - nowtime) * 1000;
        }else {
            tOut = -1;
        }

        if(pPoll->pFun_itr != NULL) {
            if(pPoll->pFun_itr(pPoll->pItr_param) != OK) {
                break;
            }
        }
        
        if(pPoll->run != 1) {
            break;
        }
                
        nfds = epoll_wait(pPoll->ep_fd, epoll_ev, pPoll->max_event, tOut);
        if (nfds < 0) {
            if (errno == EINTR) {
                if(pPoll->pFun_itr != NULL) {
                    if(pPoll->pFun_itr(pPoll->pItr_param) != OK) {
                        break;
                    }
                }
                continue;
            }
            lib_error(MODULE_UTL, "epoll_wait error, errno[%d]", errno);
            break;
        }

        /* walk epoll IO reactor and call back function */
        for (i = 0; ((i < nfds) && (pPoll->run)); ++i) {
            poll_server_t *pEventValue = NULL;
            int socketFd = epoll_ev[i].data.fd;
            int ret = 0;
            
            if (socketFd < 0) {
                lib_error(MODULE_UTL, "socket Fd < 0");
                continue;
            }
            pEventValue = poll_get_server_by_fd(pPoll, socketFd);
            if (pEventValue == NULL) {
                /* the event node might be deleted manually  nothing todo here */
                continue;
            }
            if (epoll_ev[i].events & EPOLLIN) {
                ret = poll_call_server(pEventValue, POLL_EVENT_IN);
            } else if (epoll_ev[i].events & EPOLLOUT) {
                ret = poll_call_server(pEventValue, POLL_EVENT_OUT);
            } else {
                /* 
                              * When error occurs, call back first, then delete IO  
                              * So, keep remembering that should not close the IO 
                              * in any call back function, and it will be closed by the reactor
                              * outside automatically. 
                              * Add the IO to poll reactor, then don't touch it !!!
                              */
                  
                ret = poll_call_server(pEventValue, POLL_EVENT_ERR);            
            }

            /* update fd time, for cleanup idle fd later  */
            pEventValue->last_time = nowtime;  

            /* error happens, close IO and delete it from poll */
            if (ret) { 
                poll_del_server(pEventValue);   
            }
        }
    }
    return OK;
}


void *poll_add_server_with_delete_callback(void *pPollNode, 
                                                       int fd, 
                                                       pfn_poll_svr pFunSrv, 
                                                       void *pParam,
                                                       pfn_poll_del pFunDelCb,
                                                       void *pParamDel)
{
    int ret = 0;
    poll_server_t *pSrv = NULL;
    poll_poll_t *pPoll = (poll_poll_t *) pPollNode;

    if ((pPoll == NULL) || (pFunSrv == NULL) || (fd < 0)) {
        return NULL;
    }

    pSrv = (poll_server_t *)mem_malloc(sizeof(poll_server_t));
    if (!pSrv) {
        return NULL;
    }

    pSrv->server_fd = fd;
    pSrv->pFun_svr  = pFunSrv;
    pSrv->pParam    = pParam;
    pSrv->pFun_del  = pFunDelCb;
    pSrv->pParam_del= pParamDel;
    pSrv->pPoll     = pPoll;

    pSrv->ep_event.data.fd = fd;
    pSrv->ep_event.events  = EPOLLIN|EPOLLERR|EPOLLHUP;

    ret = epoll_ctl(pPoll->ep_fd, 
                    EPOLL_CTL_ADD, 
                    pSrv->server_fd, 
                    &(pSrv->ep_event));
    
    if (ret < 0) {
        lib_error(MODULE_UTL, "fd[%d] EPOLL_CTL_ADD faild [ep_fd:%d],[errno:%d]\n",
                 pSrv->server_fd, pPoll->ep_fd, errno);
        mem_free(pSrv);
        return NULL;
    }
    pPoll->max_conn++;
    hash_add(&(pSrv->node), pPoll->server_list);
    return (void *)pSrv;
}


#include "lib-io.h"
/* 
  * Poll Duplux IO accept wrapper.
  * Duplexer Working flow:  IO-1  <=======> Duplexer <=======> IO-2
  * 
  */
static int poll_io_duplex(int fd, int type, void *param)
{
    poll_duplexer_t *pDuplex = (poll_duplexer_t *)param;
    int acceptFd = 0;
    int retIn = 0;
    void *pSrv = NULL;

    if(pDuplex == NULL) {
        return ERROR;
    }
    
    if (type == POLL_EVENT_IN) {
            poll_poll_t *ppoll = (poll_poll_t *)pDuplex->pPoll;
            if (ppoll->max_conn >= (ppoll->max_event - 10)) {
                lib_error(MODULE_UTL, "IoDuplexer poll fd(%d) exceeds max(%d), refuse connect!\n", ppoll->max_conn,ppoll->max_event);
                acceptFd = lib_accept(fd);
                if (acceptFd < 0) {
                    lib_error(MODULE_UTL, "IoDuplexer accept fd[%d] error\n", fd);
                    return ERROR;
                }else{
                    lib_close(acceptFd);
                    return OK;
                }
            }        

            acceptFd = lib_accept(fd);
            if (acceptFd < 0) {
                lib_error(MODULE_UTL, "IoDuplexer accept fd[%d] error\n", fd);
                return ERROR;
            }
            
            retIn = poll_set_fd_timeout(acceptFd, pDuplex->socket_timeout);
            if (retIn < 0) {
                lib_error(MODULE_UTL, "IoDuplexer set fd[%d] timeout error\n", fd);
                lib_close(acceptFd);
                return ERROR;
            }

            /* Init self IO, outside */
            lib_io_t *pio = io_new();
            assert(pio != NULL);
            io_set_self_ready(pio);
            io_set_outside(pio);
            pio->fd = acceptFd;

            /* Init Peer IO, inside */
            if (pDuplex->pfun_peer) { 
                retIn = (pDuplex->pfun_peer)((int)pio, 0, pDuplex);
            }else{
                lib_error(MODULE_UTL, "IoDuplexer init peer function is NULL!\n");
                lib_close(acceptFd);
                mem_free(pio);
                return ERROR;               
            }
            
            if (retIn) {
                lib_error(MODULE_UTL, "IoDuplexer init peer IO got error!\n");
                lib_close(acceptFd);
                mem_free(pio);
                return OK;
            }           
            
            /*
                     * Add self IO (outside) fd to reactor, 
                     * Note, peer IO (inside) fd has been put into reactor 
                     * in ** pfun_peer() **.
                     * 
                     */
             #if 0
            pSrv = poll_add_server(pDuplex->pPoll, 
                                   pio->fd,
                                   pDuplex->pfun_accept, 
                                   pDuplex->param_accpet);
            #endif
            pSrv = poll_add_server_with_delete_callback(pDuplex->pPoll, 
                                                        pio->fd,
                                                        pDuplex->pfun_accept, 
                                                        pDuplex->param_accept,
                                                        pDuplex->pfun_del,
                                                        pDuplex->param_del);  
            if (pSrv == NULL) {
                lib_error(MODULE_UTL, "add fd[%d] control error\n", fd);
                lib_close(acceptFd);
                return ERROR;
            }         
    } else {
        pDuplex->pfun_accept(fd, POLL_EVENT_ERR, pDuplex->param_accept);
        return ERROR;
    }
    return OK;
}


void *poll_io_duplexer(poll_duplexer_t *pIoDuplex)
{
    void *pSrv = NULL;

    if (pIoDuplex == NULL) {
        return NULL;
    }
    if ((pIoDuplex->accept_fd < 0)
        || (pIoDuplex->pPoll == NULL)
        || (pIoDuplex->pfun_accept == NULL)) {
        return NULL;
    }

    pSrv = poll_add_server(pIoDuplex->pPoll, 
                           pIoDuplex->accept_fd,
                           poll_io_duplex, 
                           (void *)pIoDuplex);
    if (pSrv == NULL) {
        return NULL;
    }
    return pSrv;
}


int poll_timer_clean_idle_fd(void *pTmrNode)
{
    poll_poll_t  *pPoll = NULL;
	long          now_time = 0;

    if (pTmrNode == NULL) {
        return ERROR;
    }

    pPoll     = (poll_poll_t *)pTmrNode;       
    now_time  = lib_time_get();

    if (pPoll == NULL) {
        return ERROR;
    }

    int i = 0;
    HASH_NODE_T *node = NULL;
    struct lib_list *node1 = NULL, *node2 = NULL;

    HASH_TABLE_T *table = pPoll->server_list;

    if (table == NULL) {
	    lib_error(MODULE_UTL, "Hash Table for FD_SERVER is NULL\n");
	    return ERROR;
    }

    lib_info("Cleanup idle fd...\n");
    for (i = 0; i < table->size; i++) {
    	lib_list_for_each_safe(node1, node2, &((&(table->head[i]))->node)) {
    	    if (node1 == NULL) {
    		    continue;
    	    }else{
    		    node = lib_list_entry(node1, HASH_NODE_T, node);
                poll_server_t *pSrv = hash_entry(node, poll_server_t, node);
                if (pSrv->idle_timeout && (now_time - pSrv->last_time) > POLL_MAX_CLEAN_IDLE_FD_TIME) {
                    lib_info("\t Clean idle fd=%d, last_time=%u\n", pSrv->server_fd, pSrv->last_time);
                    poll_del_server((void*)pSrv);
                }
                
    	    }
	    }
    } 
    lib_info("Cleanup idle fd ok!\n");
    return OK;
}


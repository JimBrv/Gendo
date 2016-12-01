/*
 * Change lib-poll to libevent-2.0
 * 
 */
#include "lib-libevt.h"


int libevt_set_recv_timeout(int fd, int timeout)
{
    struct timeval tv;
    int ret = 0;

    if (timeout <= 0) {
        return OK;
    }
    memset(&tv, 0, sizeof (tv));
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    ret = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *)(&tv),
                     sizeof(struct timeval));
    if (ret < 0) {
        lib_error(MODULE_UTL, "setsockopt err fd[%d]\n", fd);
        return ERROR;
    }
    return OK;
}



libevt_pool_t *libevt_pool_new(int max, void *ctx)
{
    libevt_pool_t *evt_pool = NULL;
    struct event_base *base = NULL;
    if (max <= 0) {
        lib_error(MODULE_UTL, "libevt_pool_new() => max invalid\n");
        return NULL;
    }
    
    evt_pool = mem_malloc(sizeof(libevt_pool_t));
    if (!evt_pool) return NULL;
    
    base = event_base_new();
    if (!base) {
        lib_error(MODULE_UTL, "couldn't create event base!\n");
        mem_free(evt_pool);
        return NULL;
    }
    
    evt_pool->evt_base = base;    
    evt_pool->max = max;
    evt_pool->cur = 0;
    evt_pool->ctx = ctx;
    thd_lock_init(&evt_pool->lock, NULL);
    INIT_LIB_LIST(&evt_pool->node);
    return evt_pool;
}


void libevt_pool_set_ctx(libevt_pool_t *evt_pool, void *ctx)
{
    if (evt_pool) 
        evt_pool->ctx = ctx;
    return;
}


libevt_t *libevt_new(libevt_pool_t *evt_pool, 
                       int            fd, 
                       event_callback_fn   new_cb,
                       event_callback_fn   del_cb,
                       event_callback_fn   event_cb,
                       void          *ctx)
{
    libevt_t *evt = NULL;
    if (!evt_pool) {
        lib_error(MODULE_UTL, "libevt_new() => evt pool or fd invalid\n");
        return NULL;
    }

    evt = mem_malloc(sizeof(libevt_t));
    if (!evt) {
        lib_error(MODULE_UTL, "libevt_new() => new evt failed\n");
        return NULL;        
    }

    evt->fd       = fd;
    evt->evt_pool = evt_pool;
    evt->ctx      = ctx;
    evt->new_cb   = new_cb;
    evt->delete_cb= del_cb;
    evt->event_cb = event_cb;
    evt->idle_timeout = 1;
    evutil_gettimeofday(&evt->last_time, NULL);
    
    INIT_LIB_LIST(&evt->node);
    lib_list_add(&evt->node, &evt_pool->node);
    evt_pool->cur++;
    return evt;    
}


int libevt_del(libevt_t *evt)
{
    libevt_pool_t *evt_pool = NULL;
    if (!evt) {
        lib_error(MODULE_UTL, "libevt_del() => evt NULL\n");
        return ERROR;
    }
    evt_pool = libevt_get_pool(evt);
    if (!evt_pool) {
        lib_error(MODULE_UTL, "libevt_del() => evt_pool NULL\n");
        return ERROR;        
    }
    
    lib_list_del(&evt->node);
    evt_pool->cur--;
    mem_free(evt);
    return OK;
}


int libevt_clean(libevt_t *evt)
{
    int ret = 0;
    if (!evt) return ERROR;
    ret = event_del(&evt->evt);
    lib_close(evt->fd);             
    libevt_del(evt);  
    return OK;
}

/* 
  * acceptor callback for libevent, 
  * after it accpeted new fd, it called by libevent,
  * and we set a read callback for the new fd.
  */
static void
libevt_accept_conn_cb(struct evconnlistener *listener,
                    evutil_socket_t        fd, 
                    struct sockaddr       *address, 
                    int                    socklen,
                    void                  *ctx)
{
        /* We got a new connection! */
        int ret = 0;
        struct event_base *base     = evconnlistener_get_base(listener);
        libevt_acceptor_t *acceptor = (libevt_acceptor_t *)ctx;
        libevt_pool_t     *evt_pool = NULL;
        struct timeval     tm;

        if (!base || !acceptor) {
            lib_error(MODULE_UTL, "base or acceptor is null\n");
            return;
        }
        evt_pool = (libevt_pool_t *)acceptor->evt_pool;
        if (!evt_pool || base != evt_pool->evt_base) {
            lib_error(MODULE_UTL, "evt pool is null or base != evtpool->base\n");
            return;            
        }

        if (acceptor->socket_timeout > 0) {
            ret = libevt_set_recv_timeout(fd, acceptor->socket_timeout);
            if (ret) {
                lib_error(MODULE_UTL, "set blocking fd=%d recv timeout failed\n", fd);
                return;                            
            }
        }
        

        libevt_t *evt = libevt_new(evt_pool, fd, NULL, NULL, NULL, acceptor->ctx);
        if (!evt) {
            lib_error(MODULE_UTL, "libevt_new() got error\n");
            return;            
        }        

        /* add new fd to libevent */
        ret = event_assign(&evt->evt, 
                           base,
                           fd, 
                           EV_READ|EV_PERSIST,  
                           acceptor->event_cb,
                           (void*)evt);
        if (ret) {
            lib_error(MODULE_UTL, "event_assign error!\n");
            libevt_del(evt);
            lib_close(fd);
            return;
        }

        if (acceptor->idle_timeout > 0) {
            tm.tv_sec  = acceptor->idle_timeout;
            tm.tv_usec = 0;
            evt->idle_timeout = acceptor->idle_timeout;
            ret = event_add(&evt->evt, &tm);
        }else{
            ret = event_add(&evt->evt, NULL);
        }
        
        if (ret) {
            lib_error(MODULE_UTL, "event_add error!\n");
            libevt_del(evt);   
            lib_close(fd);
            return;
        }
        
        return;
}

static void
libevt_accept_error_cb(struct evconnlistener *listener, void *ctx)
{
        struct event_base *base = evconnlistener_get_base(listener);
        int err = EVUTIL_SOCKET_ERROR();
        lib_error(MODULE_UTL, "Got an error %d (%s) on the listener.",
                  err, evutil_socket_error_to_string(err));
        lib_syslog(MODULE_UTL, "Got an error %d (%s) on the listener.",
                  err, evutil_socket_error_to_string(err));
        //event_base_loopexit(base, NULL);
}


/*
 * Note: default set accepted new fd as blocking
 *
 */
int libevt_acceptor(libevt_pool_t *evt_pool, libevt_acceptor_t *acceptor)
{
    struct evconnlistener *listener = NULL;
    struct event_base     *base = NULL;
    struct sockaddr_in     sin;
    
    if (!evt_pool || !acceptor) {
        lib_error(MODULE_UTL, "libevt_acceptor() => null!\n");
        return ERROR;
    }
    
    base = evt_pool->evt_base;
    if (!base) {
        lib_error(MODULE_UTL, "libevt_acceptor() => base is null!\n");
        return ERROR;
    }

    acceptor->evt_pool = evt_pool;

    /* This is an INET address */
    memset(&sin, 0, sizeof(sin));
    sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = inet_addr(acceptor->ip);
    sin.sin_port        = htons(acceptor->port);

    /* create libevent listener */
    listener = evconnlistener_new_bind(base, 
                                       libevt_accept_conn_cb, 
                                       (void*)acceptor,
                                       LEV_OPT_LEAVE_SOCKETS_BLOCKING|LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE|LEV_OPT_THREADSAFE,
                                       -1,
                                       (struct sockaddr*)&sin, 
                                       sizeof(sin));
    if (!listener) {
        lib_error(MODULE_UTL, "Couldn't create listener");
        return ERROR;
    }
    evconnlistener_set_error_cb(listener, libevt_accept_error_cb);
    return OK;   
}

int libevt_connector(libevt_pool_t *evt_pool, libevt_connector_t *connector)
{
    int ret = 0;
    struct event_base     *base = NULL;
    
    if (!evt_pool || !connector) {
        lib_error(MODULE_UTL, "libevt_connector() => nul!\n");
        return ERROR;
    }

    if (!evt_pool->evt_base) {
        base = event_base_new();
        if (!base) {
            lib_error(MODULE_UTL, "couldn't open event base!\n");
            return ERROR;
        }
        evt_pool->evt_base = base;
    }else{
        base = evt_pool->evt_base;
    }

    connector->evt_pool = evt_pool;
    int fd = lib_client(connector->ip, connector->port, SOCK_STREAM);
    if (fd <= 0) {
        lib_error(MODULE_UTL, "create connect fd error!\n");
        return ERROR;   
    }


    connector->connect_fd = fd;
    libevt_t *evt = libevt_new(evt_pool, fd, NULL, NULL, NULL, (void*)connector->ctx);    
    if (!evt) {
        lib_error(MODULE_UTL, "libevt_new() got error\n");
        return ERROR;            
    }        
    
    ret = event_assign(&evt->evt, 
                       base,
                       fd, 
                       EV_READ|EV_PERSIST,  
                       connector->event_cb,
                       evt);
    if (ret) {
        lib_error(MODULE_UTL, "event_assign error!\n");
        libevt_del(evt);
        lib_close(fd);
        return ERROR;
    }
    
    ret = event_add(&evt->evt, NULL);
    if (ret) {
        lib_error(MODULE_UTL, "event_add error!\n");
        libevt_del(evt);   
        lib_close(fd);
        return ERROR;
    }    
    return OK;
}

/* new libevent interface for timer, signal handler */
int libevt_timer(libevt_pool_t *evt_pool, int time, event_callback_fn timer_cb, void *param)
{
    libevt_t *evt = NULL;
    struct event_base *base = NULL; 
    struct timeval tm;

    if (!evt_pool || !evt_pool->evt_base) {
        lib_error(MODULE_UTL, "evt_pool or base invalid!\n");
        return ERROR;
    }
    base = evt_pool->evt_base;
    
    evt = libevt_new(evt_pool, -1, NULL, NULL, NULL, param);
    if (!evt) {
        lib_error(MODULE_UTL, "libevt_new() error!\n");
        return ERROR;
    }
    
    if (event_assign(&evt->evt, base, -1, EV_PERSIST, timer_cb, param)) {
        lib_error(MODULE_UTL, "event_assign() event error!\n");
        libevt_del(evt);
        return ERROR;        
    }
    
    tm.tv_sec  = time;
    tm.tv_usec = 0;
    if (evtimer_add(&evt->evt, &tm)< 0) {
        lib_error(MODULE_UTL,"evttimer_add() event error!\n");
        libevt_del(evt);
        return ERROR;
    }
    
    return OK;
}


int libevt_signal(libevt_pool_t *evt_pool, int sig, event_callback_fn signal_cb, void *param)
{
    libevt_t *evt = NULL;
    struct event_base *base = NULL; 

    if (!evt_pool || !evt_pool->evt_base) {
        lib_error(MODULE_UTL, "evt_pool or base invalid!\n");
        return ERROR;
    }
    base = evt_pool->evt_base;
    
    evt = libevt_new(evt_pool, -1, NULL, NULL, NULL, param);
    if (!evt) {
        lib_error(MODULE_UTL, "libevt_new() error!\n");
        return ERROR;
    }
    
    if (evsignal_assign(&evt->evt, base, sig, signal_cb, param)) {
        lib_error(MODULE_UTL, "evsigal_assign() event error!\n");
        libevt_del(evt);
        return ERROR;        
    }
    
    if (evsignal_add(&evt->evt, NULL)< 0) {
        lib_error(MODULE_UTL, "evsignal_add() event error!\n");
        libevt_del(evt);
        return ERROR;
    }
    
    return OK;

}

int libevt_run(libevt_pool_t *evt_pool)
{
    struct event_base *base = NULL;
    if (!evt_pool || !evt_pool->evt_base) 
        return ERROR;
    base = evt_pool->evt_base;
    return event_base_dispatch(base);   
}


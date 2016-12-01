/*
 * Filename : gas-main.c
 * Copyright : All right reserved gendo team.
 *
 * gendo, 2013.3
 */

#include "gas.h"
#include "gas-user.h"

#define RAS_BASE_THREAD          11
#define RAS_HIGH_PRIORITY_THREAD 8

//#define RAS_MSG_PROCESS_THREAD  sys_cpu_get()
      

/* libevent engine */
libevt_pool_t      *evt_pool;
libevt_acceptor_t   evt_user_acceptor;


/* thread pool + msg pool */
lvl_mp_t  ras_lvl_msg_pool;


/* msg consumer-producer pool */
msg_consumer_t *user_msg_pool;        /* for user highest msg */

/* config params, from configure file */
ras_config_t ras_cf;


void ras_signal_cb(int sig, short events, void *user_data)
{
    lib_info("Caught an interrupt signal, continue run!\n");
}

static int ras_config_init(void)
{
    char buf[MAX_NAME_LEN] = {0};

    if (config_get(GAS_IP, ras_config_get(ras_ip))) goto err;
    if (config_get(GAS_PORT, buf)) goto err;
    ras_config_get(ras_port) = atoi(buf);    
    if (config_get(GAS_DEBUG_PORT, buf)) goto err;
    ras_config_get(ras_debug_port) = atoi(buf);  

    /* Max connection */
    if (config_get(GAS_MAX_CONN, buf)) goto err;
    ras_config_get(ras_max_conn) = atoi(buf);      
    if (config_get(GAS_MAX_THREAD, buf)) goto err;
    ras_config_get(ras_max_thread) = atoi(buf);
    if (config_get(GAS_ID, buf)) goto err;
    ras_config_get(ras_id) = atoi(buf);

    /* DB setting */
    if (config_get(DBS_IP, ras_config_get(dbs_ip))) goto err;
    if (config_get(DBS_PORT, buf)) goto err;
    ras_config_get(dbs_port) = atoi(buf);

    if (config_get(DBS_DB_CONNECT, buf)) goto err;
    ras_config_get(dbs_connect) = atoi(buf);

    if (config_get(DBS_DB_NAME, ras_config_get(db_name))) goto err;
    if (config_get(DBS_DB_USER, ras_config_get(db_user))) goto err;
    if (config_get(DBS_DB_PWD,  ras_config_get(db_pwd))) goto err;

    /* LOG setting */
    if (config_get(LGS_IP, ras_config_get(lgs_ip))) goto err;
    if (config_get(LGS_PORT, buf)) goto err;
    ras_config_get(lgs_port) = atoi(buf);
        
    return OK;
        
  err:
    return ERROR;
}

int ras_mem_hook(int code, void *arg)
{
    struct mem_struct *m = (struct mem_struct *)(arg);

    ras_error("MEM error: code[%d], module[%d] on [%s:%d]", 
              code,
              m->module, 
              m->file, 
              m->line);
    return OK;
}

/* init ras global resource */
int ras_init(void)
{
    system("ulimit -n 10240");
    int ret = OK;
    mem_init(ras_mem_hook, 1);

    if (ras_config_init()) {
        ras_error("init config got error\n");
        return ERROR;
    }        

   
    if (db_pool_init(ras_config_get(dbs_ip), 
                     ras_config_get(dbs_port), 
                     ras_config_get(db_user),
                     ras_config_get(db_pwd),
                     ras_config_get(db_name),
                     ras_config_get(ras_max_thread)))
    {
        ras_error("create db pool error\n");
        return ERROR;       
    }
    
    db_set_instant_server(ras_config_get(dbs_ip),
                          ras_config_get(dbs_port),
                          ras_config_get(db_user),
                          ras_config_get(db_pwd),
                          ras_config_get(db_name));

   	lib_log_init(ras_config_get(lgs_ip), ras_config_get(lgs_port), LOG_DISP_ALL);
    
    return ret;
}


static int ras_signal_init(void)
{
    int ret = 0;    
    
    ret = libevt_signal(evt_pool, SIGPIPE, ras_signal_cb, (void*)evt_pool);
    if (ret) {
        lib_info("error on libevt_signal\n");
        return 1;
    }
    ret = libevt_signal(evt_pool, SIGHUP, ras_signal_cb, (void*)evt_pool);
    if (ret) {
        lib_info("error on libevt_signal\n");
        return 1;
    }

    return ret;
}


/* Libevent engine here */
int ras_poll_init_with_libevt(void)
{
    int  ret = 0;
    
    evt_pool = libevt_pool_new(ras_config_get(ras_max_conn)+1000, NULL);   
    if (!evt_pool) {
        ras_error("init evt pool failed\n");
        return ERROR;           
    }
        
    evt_user_acceptor.accept_cb       = NULL;
    evt_user_acceptor.accept_cb_param = NULL;
    evt_user_acceptor.event_cb        = ras_user_msg_recv;
    evt_user_acceptor.event_cb_param  = NULL;
    evt_user_acceptor.ctx             = (void*)&ras_lvl_msg_pool;
    evt_user_acceptor.evt_pool        = NULL;
    evt_user_acceptor.flag            = 0;
    evt_user_acceptor.listener        = NULL;
    evt_user_acceptor.socket_timeout  = 1;
    evt_user_acceptor.idle_timeout    = 60*10;
    evt_user_acceptor.port            = ras_config_get(ras_port);
    strcpy(evt_user_acceptor.ip, "0.0.0.0");
    
    ret = libevt_acceptor(evt_pool, &evt_user_acceptor);
    if (ret) {
        lib_info("error on libevt_acceptor\n");
        return 1;
    }

    /* Must catch sigpipe incase exist for broken pipe */
    ret = ras_signal_init();
    if (ret) {
        ras_error("init signal handler fail\n");
        return 1;
    }


    /* Manage state timer*/
    //ret = libevt_timer(evt_pool, 
    //                   60,
    //                   ras_min_timer, 
    //                   (void*)evt_pool);
    if (ret) {
        lib_info("error on libevt_timer Manage state\n");
        return 1;
    }

    /* End timer */
    
    return OK;
}


static void ras_user_high_priority_thread(void *param1, void *param2)
{
    lvl_mp_t       *lvl = (lvl_mp_t*)param1;
    msg_consumer_t *mc = (msg_consumer_t *)lvl->pmc_msg_high;
    strcpy(mc->name, "ras-high-thread");
    mc->pool_cache.count           = 100;
    mc->pool_cache.msg_payload_len = MID_MSG_LEN;

    ras_info("Init high priority thread = multi(%d), max_connection=%d\n", lvl->thd_high, ras_config_get(ras_max_conn));
    mc_multi_thread_init(mc, 
                         "ras-user-low-thread", 
                         ras_user_msg_handle,
                         (void*)lvl->ptp_high,
                         NULL, 
                         NULL,
                         1);

    mc_start(mc);        
    /* msg consumer starting work */
    mc_consumer_run_with_tp(mc);
}


int ras_msg_pool_init(void)
{
    user_msg_pool = mem_malloc(sizeof(msg_consumer_t));
	
    if (!user_msg_pool) {
        ras_error("create msg pool error");
        return ERROR;
    }
    ras_lvl_msg_pool.pmc_msg_high = user_msg_pool;
    ras_lvl_msg_pool.pmc_msg_mid  = NULL;
    ras_lvl_msg_pool.pmc_msg_low  = NULL;

    return OK;
}


static void ras_debug_thread(void *param1, void *param2)
{
    int debug_fd = 0, svr_fd = 0;
    char debug_buf[1024] = {0};
    msg_head_t *pmsg = (msg_head_t *)debug_buf;
    int n = 0;
        
    svr_fd = lib_server("127.0.0.1", ras_config_get(ras_debug_port));
    if (svr_fd <= 0 ) {
        ras_error("server : create debug socket error\n");
        return;               
    }
    while(true) {
        debug_fd = lib_accept(svr_fd);
        ras_info("recving debug shell connect, enter debug mode...\n");
                
        while(true) {
            memset(debug_buf, 0, 1024);
            if ((n = lib_recv_msg(debug_fd, debug_buf, 1024)) <= 0) {
                lib_close(debug_fd);
                break;
            }
            switch(pmsg->msg) {
            case 0:
                break;
            default:
                ras_info("unknown cmd=[0x%x]\n", pmsg->msg);
                break;
            }
        }
    }
}


int ras_thread_init(void)
{
    thread_pool_t *tp = NULL;

    /* create thread to deal BASE operation */
    if (thread_pool_init(4)) {
        ras_error("create thread pool error");
        return ERROR;
    }
    
    tp = thread_pool_new(RAS_HIGH_PRIORITY_THREAD);
    if (!tp) {
        ras_error("create high thread pool(%d) error\n", RAS_HIGH_PRIORITY_THREAD);
        return ERROR;        
    }
    ras_lvl_msg_pool.ptp_high = tp;
    ras_lvl_msg_pool.thd_high = RAS_HIGH_PRIORITY_THREAD;    

    ras_info("TP-high=0x%p,count=%d\n",
             ras_lvl_msg_pool.ptp_high,
             ras_lvl_msg_pool.thd_high);
        
    thread_dispatch(ras_user_high_priority_thread, (void*)&ras_lvl_msg_pool);    
    thread_dispatch(ras_debug_thread, NULL);

    return OK;
}

#define RAS_USAGE "GAS usage: gas Port\n" 


int main(int argc, char **argv)
{
    int ret = ERROR;
    int port = 0;
    
    if (argc < 2) {
        printf(RAS_USAGE);
        return OK;
    }
    if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help")) {
        printf(RAS_USAGE);
        return OK;
    }    
  
    
    /* Check RAS IP:Port */
    port = atoi(argv[1]);
    
    if (port <= 0 || port > 65535) {
        ras_error("Bad Port params. input correct Port (range: 1-65535).\n");
        return -1;
    }
    
    ras_config_get(ras_port) = port;
    
    ret = ras_init();
    if (ret) {
        ras_error("init failed\n");
        goto out;
    }

    ret = ras_msg_pool_init();
    if (ret) {
        ras_error("msg pool init failed\n");
        goto out;
    }
        
    ret = ras_poll_init_with_libevt();
    if (ret) {
        ras_error("module init failed\n");
        goto out;
    }
    
    ret = ras_thread_init();
    if (ret) {
        ras_error("thread pool init failed\n");
        goto out;
    }

    server_list_t svr;
    gas_dump_server_info(TYPE_ALL, &svr);
    
    
    ret = libevt_run(evt_pool);

    while(true) {
        ras_error("RAS libevt loopback break, bug found...\n");
        sleep(10);
    }
        
  out:
        
    thread_pool_finl();
    if (user_msg_pool) {
        mc_stop(user_msg_pool);
        mc_finl(user_msg_pool);
    }
    
    return OK;
}

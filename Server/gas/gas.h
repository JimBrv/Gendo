/*
 * Header : ras.h
 * Copyright : All right reserved by Gendo team. 2013
 * 
 * gendo, 2013.3
 */


#ifndef GAS_H
#define GAS_H

#include "lib-base.h"
#include "lib-poll.h"
#include "lib-thread.h"
#include "lib-debug.h"
#include "lib-ticket.h"
#include "lib-config.h"
#include "lib-db.h"
#include "lib-file.h"
#include "lib-mem.h"
#include "lib-list.h"
#include "lib-msg.h"
#include "lib-msgpool.h"
#include "lib-msg-consumer.h"
#include "lib-socket.h"
#include "lib-time.h"
#include "lib-timer.h"
#include "lib-index-cache.h"
#include "lib-log.h"
#include "lib-string.h"
#include "lib-bit.h"
#include "lib-sys.h"
#include "lib-libevt.h"

#define ras_error(...)    {lib_error(MODULE_GAS, __VA_ARGS__);lib_syslog(MODULE_GAS, __VA_ARGS__);}
#define ras_info(...)     {lib_debug(MODULE_GAS, __VA_ARGS__);lib_syslog(MODULE_GAS, __VA_ARGS__);}
#define ras_log(...)      lib_log(MODULE_GAS, __VA_ARGS__)

typedef struct _ras_config_s {
    /* ras listen port */
    char ras_ip[MAX_NAME_LEN];
    int  ras_port;
    int  ras_debug_port;
    int  ras_max_thread;
    int  ras_max_conn;
    /* self id */
    int  ras_id;  

    /* db server info */
    char db_user[MAX_NAME_LEN];
    char db_pwd[MAX_NAME_LEN];
    char db_name[MAX_NAME_LEN];
    char dbs_ip[MAX_NAME_LEN];
    int  dbs_port;
    int  dbs_connect;


    char lgs_ip[MAX_NAME_LEN];
    int  lgs_port;
}ras_config_t, *pras_config_t;

extern ras_config_t ras_cf;
#define ras_config_get(ITEM) (ras_cf.ITEM)

typedef struct _ras_usr_leverage_msg_pool_s {
    msg_consumer_t *pmc_msg_high;             // for user kp msg, high priority
    msg_consumer_t *pmc_msg_mid;              // for user apply, cancel msg, midium priority
    msg_consumer_t *pmc_msg_low;              // for user lower priority msg
    thread_pool_t  *ptp_high;                 // thread pool for each priority
    thread_pool_t  *ptp_mid;
    thread_pool_t  *ptp_low;
    int             thd_high;                 // thread count
    int             thd_mid;
    int             thd_low;
}lvl_mp_t, *plvl_mp_t;

#define MAX_RAS_DB_CONNECTION 8

void ras_user_msg_recv(int fd, short type, void *param);
int  ras_user_msg_handle(void *param, msg_item_t *pmsg);

#endif


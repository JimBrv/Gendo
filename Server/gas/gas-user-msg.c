
#include "gas.h"
#include "gas-user.h"

static char user_msg_buf[MAX_MSG_LEN] = {0};


#define GAS_USER_MSG_CHECK(MSGIT) \
    ((((MSGIT)->msg.msg) > MSG_GAS_USER_START) && (((MSGIT)->msg.msg) < MSG_GAS_USER_END) \
   &&(((MSGIT)->msg.len) <= MID_MSG_LEN))

/* msg map handler */
#define BEGIN_MSG_MAP(MSG) \
switch((MSG)) {

#define ON_MSG_RET(MSG, PROC, PARAM1, PARAM2, RET) \
case (MSG): { \
    (RET)=PROC(PARAM1, PARAM2); \
} \
break;

#define END_MSG_MAP(MSG)   \
default:  \
    ras_error("msg=0x%x doesn't map to handler!", (MSG)); \
    break; \
}


int gas_user_login(void *param, msg_item_t *pmsg)
{
    int ret = 0;
    puser_login_t login   = msg_container_of(pmsg->msg.payload, user_login_t);
    puser_login_ack_t ack = msg_container_of(pmsg->msg.payload, user_login_ack_t);;
    char name[MAX_NAME_LEN] = {0};
    char pwd[MAX_NAME_LEN]  = {0};

    memcpy(name, login->name, MAX_NAME_LEN);
    memcpy(pwd,  login->pwd,  MAX_NAME_LEN);
    name[MAX_NAME_LEN - 1] = 0;
    pwd[MAX_NAME_LEN - 1] = 0;
    
    if (lib_name_sanity(name)) {
        ret = ERR_GAS_USER_LOGIN_NAME_SANITY;
        ras_error("login user=[%s], name sanity check fail!\n", name);
        goto out;
    }
    
    if (gas_db_user_auth(name, pwd) == false) {
        ret = ERR_GAS_USER_LOGIN_AUTH_FAIL; 
        ras_error("login user=[%s], pwd=[%s] auth fail!\n", name, pwd);
        goto out;
    }else{
        ras_info("login user=[%s], pwd=[%s] auth OK.\n", name, pwd);
    }

    ret = gas_db_user_info(name, &ack->user);
    if (ret) {
        ret = ERR_GAS_USER_LOGIN_AUTH_FAIL;     
        ras_error("login user=[%s], get user info fail\n", name);
        goto out;
    }

    gas_db_user_update_lastlogin(name);
out:
    pmsg->msg.len = sizeof(user_login_ack_t);
    ack->ret      = ret;
    if (ret) {
        memset(&ack->user, 0, sizeof(user_t));
    }
    ret = lib_send_msg((int)pmsg->peer, &pmsg->msg);
    if (ret <= 0) {
        ras_error("login user=[%s], send msg fail\n", name);
    }
    return OK;
}


int gas_server_list(void *param, msg_item_t *pmsg)
{
    int ret = 0;
    pserver_list_t svr_list = msg_container_of(pmsg->msg.payload, server_list_t);;
    
    ras_info("get svr_list\n");

    if (gas_db_server_info(TYPE_ALL, svr_list)) {
        ret = ERR_GAS_SERVER_LIST_FAIL; 
        ras_error("get svr_list fail!\n");
        goto out;
    }
out:
    pmsg->msg.len = sizeof(server_list_t);
    if (ret) {
        memset(svr_list, 0, sizeof(server_list_t));
    }
    ret = lib_send_msg((int)pmsg->peer, &pmsg->msg);
    if (ret <= 0) {
        ras_error("get svr_list, send msg fail\n");
    }
    return OK;

}




/* 
  * user msg handle.
  * It will be called by msg consumer  thread.
  */
int ras_user_msg_handle(void *param, msg_item_t *pmsg)
{
    int ret = ERROR;
        
    if (!GAS_USER_MSG_CHECK(pmsg)) {
        ras_error("user msg : msg sanity error, msg.peer=[%d], msg=[0x%x], len=[%d]\n",
                  (int)pmsg->peer, pmsg->msg.msg, pmsg->msg.len);
        return ERROR;                           
    }
        
    BEGIN_MSG_MAP(pmsg->msg.msg) 
        ON_MSG_RET(MSG_GAS_USER_LOGIN,  gas_user_login, param, pmsg, ret)
        ON_MSG_RET(MSG_GAS_SERVER_LIST, gas_server_list,param, pmsg, ret)
    END_MSG_MAP(pmsg->msg.msg)
        
    return ret;
}

void ras_user_msg_recv(int fd, short type, void *param)
{
    int ret              = 0;
    msg_item_t     *pmsg = NULL;
    msg_head_t     *pmh  = NULL;        
    msg_consumer_t *pmc  = NULL;
    libevt_t       *pevt = (libevt_t *)param;
    lib_assert(pevt != NULL);
    lvl_mp_t       *ptp  = (lvl_mp_t *)pevt->ctx;
    lib_assert(ptp != NULL);
    lib_assert(fd == pevt->fd);
    
    if (EV_TIMEOUT == type) {
        /* timeout, should close idle socket */
        ras_error("user msg : socket(%d) idle timeout\n", fd);        
        goto err_out;
    }else if (EV_READ != type) {
        ras_error("user msg : libevt not READ|TIMEOUT, should colse socket\n");
        goto err_out;      
    }

    ret = lib_recv_msg(fd, user_msg_buf, MAX_MSG_LEN);
    if (ret <= 0) goto err_out;

    pmh = (msg_head_t*)user_msg_buf;

    pmc = (msg_consumer_t *)ptp->pmc_msg_high;
    assert(pmc != NULL);
    
    pmsg = mc_producer_get_msg(pmc);
    if (!pmsg) {
        /* recv msg, but discards it silently */
        ras_error("user msg : msg pool cannot alloc msg=0x%x, the incoming msg was discarded\n", pmh->msg);
        goto full_out;
    }
    
    msg_copy(&pmsg->msg, pmh);      
    pmsg->peer = (void*)fd;
         
    mc_producer_run(pmc, pmsg);
    return;

err_out:
    /* cleanup libevent */
    libevt_clean(pevt);
    return;
full_out:
    /* msg pool is full, dont close the socket for client will relogin automatically */
    return;
}


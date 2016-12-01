/*
 * Header : lib-msg.h
 * Copyright : All right reserved Gendo
 * 
 * 
 * gendo, 2013,3
 */

#ifndef LIB_MSG_H
#define LIB_MSG_H
#ifdef WIN32
#pragma warning (disable:4200)
#endif

#include "lib-types.h"

/* base name len */
#define MAX_NAME_LEN 32
#define MAX_PWD_LEN  48
#define MAX_DATE_LEN 20
#define TICKET_LEN   32   


#ifdef __cplusplus
extern "C" {
#endif


#define MID_MSG_LEN  1024*50    // midium 50k 
#define MAX_MSG_LEN  1024*1024  // max 1M 
#define SML_MSG_LEN  1024       // small 1k 


/* TLV format msg in rop */
typedef struct _msg_head_s {
    UInt32 msg;        // msg type
    UInt32  len;        // payload's length
    char payload[0]; 
}msg_head_t, *pmsg_head_t;

#define MSG_HEAD_LEN sizeof(msg_head_t)

#define MSG_SANITY(MSG) ((MSG)->msg != 0 && (MSG)->len <= MAX_MSG_LEN)

/* force converts memory to structure  */
#define msg_container_of(PTR, TYPE) ({(TYPE *)((char *)(PTR));})     

#define SIZEOF(ARY) (sizeof((ARY))/sizeof((ARY)[0]))



/* msg macro, reference it both client & server */
/* msg base, and offset */
#define MSG_BASE                       0x00010000               
#define MSG_OK_MASK                    0x7fffffff             // highest bit 0-OK, 1-ERR
#define set_msg_ok(MG)                 
#define set_msg_err(MG)                ((MG)|= ~MSG_OK_MASK) 
#define is_msg_ok(MG)                 !((MG)&(~MSG_OK_MASK))

#define MSG_INTERNAL                   (MSG_BASE + 0xF00000)  /* for internal & reserve */

#define MSG_GAS                        (MSG_BASE + 0x00000)     /* for Gas server */
#define MSG_LGS                        (MSG_BASE + 0x10000)     /* for Log server */

#define ERR_GAS                        (MSG_BASE + 0xE0000)     /* for Log server */



#define MSG_MODULE_INTERNAL_SPLIT       0x01000  /* split the msg within a module */


/* debug msg for all modules */
typedef enum {
    MSG_DEBUG_SHOW_THREAD = MSG_INTERNAL,
    MSG_DEBUG_SHOW_MSG_POOL,
    MSG_DEBUG_SHOW_MEM,
    MSG_DEBUG_MODIFY_MAX_LOGIN_NUM,
    MSG_DEBUG_DISABLE_COS,
    MSG_DEBUG_ENABLE_COS,
    MSG_DEBUG_ALL,
    MSG_DEBUG_COS_KICK_USER,
    
}MSG_DEBUG_E;


/* lbs msg goes here */
#define MSG_TO_HANDLER_MASK    0x0fff        /* last 4k mask used for msg handler index */

typedef enum {
    /* client <=> GAS */
    MSG_GAS_USER_START = (MSG_GAS + 0), 
    MSG_GAS_USER_LOGIN,
    MSG_GAS_USER_LOGOUT,
    MSG_GAS_USER_CHECK_VERSION,
    MSG_GAS_USER_LOGIN_REFUSE,
    MSG_GAS_SERVER_LIST,
    MSG_GAS_USER_END,
}MSG_GAS_E;

typedef enum {
    /* ERROR define */
    ERR_GAS_USER_LOGIN_AUTH_FAIL = ERR_GAS, 
    ERR_GAS_USER_LOGIN_NAME_SANITY,
    ERR_GAS_SERVER_LIST_FAIL,
    ERR_GAS_USER_LOGIN_REFUSE,
    ERR_GAS_SERVER_LIST_REFUSE,
}ERR_GAS_E;



/* lgs msg */
typedef enum {
    /* lgs log msg */
    MSG_LGS_START = (MSG_LGS + 0),
    MSG_LGS_LOG,
    MSG_LGS_END,
}MSG_LGS_E;

/* user class type */
typedef enum {
    TYPE_C2O_WEB      = 1,    // China2Outside web browser, online mv,etc
    TYPE_C2O_GAME   = 2,    // China2Outside game
    TYPE_O2C_WEB      = 4,    // Outside2China web browser,download,mv
    TYPE_O2C_GAME   = 8,    // Outside2China game 
    TYPE_ALL                 = TYPE_O2C_GAME|TYPE_O2C_WEB|TYPE_C2O_WEB|TYPE_C2O_GAME,
}VPN_TYPE_E;


/* Gendo user structure, according DB */
typedef struct _user_s {
    /* same as db_user_t, base information */
    int        id;
    char    name[MAX_NAME_LEN];
    char    pwd[MAX_NAME_LEN];
    int       active;
    char    reg_time[MAX_NAME_LEN];
    char    nick[MAX_NAME_LEN];
    char    email[MAX_NAME_LEN];
    char    note[MAX_NAME_LEN*4];
    int       quota_cycle;
    Int64   quota_bytes;
    Int64   quota_used;
    char    quota_name[MAX_NAME_LEN*4];
    char    quota_expire[MAX_NAME_LEN];
    Int64   history_bytes;    
    int       enable;
    int       level;
    char    quota_start[MAX_NAME_LEN];
    int       hobby;
    int       login_cnt;
    int       type;	                        // see VPN_TYPE_E
}user_t, *puser_t;



/* user login from client to GAS */
typedef struct _user_login_s {
    char name[MAX_NAME_LEN];
    char pwd[MAX_PWD_LEN];
    char os[MAX_NAME_LEN/2];
}user_login_t, *puser_login_t;

/* user login ack from GAS to client */     
typedef struct _user_login_ack_s {
    int           ret; 
    user_t        user;
}user_login_ack_t, *puser_login_ack_t;

/* VPN protocol  */
typedef enum {
    PROTO_PPTP        = 1,
    PROTO_L2TP        = 2,
    PROTO_OPENVPN_UDP = 4,
    PROTO_OPENVPN_TCP = 8,
}protocol_e;

/* VPN state  */
typedef enum {
    VPN_STATE_IDLE    = 0,
    VPN_STATE_NORMAL  = 1,
    VPN_STATE_BUSY    = 2,
    VPN_STATE_DOWN    = 3,
}vpn_state_e;


/* server information */
typedef struct _server_s {
    int            id;
    char         ip[MAX_NAME_LEN];
    char         name[MAX_NAME_LEN];
    char         desc[MAX_NAME_LEN*2];    
    protocol_e   protocol;                              // pptp, l2tp, openvpn-udp, openvpn-tcp, etc
    int               latency;                                // ms latency of ping
    int              cur_user;                               // current load
    int              max_user;                             // max load
    vpn_state_e  state;                                  // status: Idle, Normal, Busy
    char         ssl_port[MAX_NAME_LEN*4]; // openvpn port, like 'tcp=443,udp=443'
    int            type;                                        // see VPN_TYPE_E
    char         location[MAX_NAME_LEN/2]; 
}server_t, *pserver_t;


#define MAX_SERVER_LIST  64

/* server list info from GAS to client */
typedef struct _server_list_s {
    int      svr_cnt;                        
    server_t svr_ary[MAX_SERVER_LIST];
}server_list_t, *pserver_list_t;

/* version check */
typedef struct _user_version_s {
    unsigned int  v_m;       // main version id
    unsigned int  v_s;       // sub version id
    char          v_time[MAX_NAME_LEN];    // version dispatch time
    char          v_name[MAX_NAME_LEN];    // version name
}user_version_t, *puser_version_t;

    
#ifdef __cplusplus
}
#endif


#endif

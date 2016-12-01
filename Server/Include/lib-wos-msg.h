/*
 * Header : lib-msg.h
 * Copyright : All right reserved by SecWin, 2010
 * ROP msg define, include it both windows client and Linux server!
 * The default method to encape/dencape the msg is the " Memory Force" method.
 * 
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */

#ifndef LIB_WOS_MSG_H
#define LIB_WOS_MSG_H
#ifdef WIN32
#pragma warning (disable:4200)
#endif
#include "lib-msg.h"

#ifdef __cplusplus
extern "C" {
#endif

#define KEEP_KEY_BIT_SAVE					28		//0b000011100
#define CLEAN_UP_OTHER_BIT					226		//0b011100010
#define CLEAN_UP_STATUES_BIT				511		//0b111111111

typedef enum {
	WOS_USER_STATE_NORMAL =				1,		//0b000000001,
	WOS_USER_STATE_SPEEDUP =                               2,		//0b000000010,
	WOS_USER_STATE_PLAY_GAME=			4,		//0b000000100,
	WOS_USER_STATE_DOWNLOAD=			8,		//0b000001000,
	WOS_USER_STATE_WATCH_MOVIE=		16,		//0b000010000,
	WOS_USER_STATE_PR_LOGOUT=			32,		//0b000100000,
	WOS_USER_STATE_TIMEOUT=				64,		//0b001000000,
	WOS_USER_STATE_TIME_NOT_ENOUGH =	128,	//0b010000000,
	WOS_USER_STATE_PRETEND_SPEEDUP    =	256,	//0b100000000,
}USER_STATE;


typedef enum {
    /* lwos <--> user msg */
    MSG_LWOS_USER_START = (MSG_LWOS + 0),//851968 
    MSG_LWOS_USER_LOGIN,
    MSG_LWOS_USER_LOGOUT,
    MSG_LWOS_USER_CHECK_VERSION,
    MSG_LWOS_USER_LOGIN_REFUSE,
    
    MSG_LWOS_USER_LOGIN_ACK,
    MSG_LWOS_USER_CHECK_VERSION_ACK,
    MSG_LWOS_USER_LOGIN_REFUSE_ACK,
    MSG_LWOS_USER_END,
    

    /*lwos<-->wos msg*/
    MSG_LWOS_WOS_START=(MSG_LWOS+MSG_MODULE_INTERNAL_SPLIT),//856064
    MSG_LWOS_WOS_KEEPLIVE,
    MSG_LWOS_WOS_END,
    
    MSG_LWOS_END,
}MSG_LWOS_E;

/* debug msg for all modules */
typedef enum {
    MSG_DEBUG_SHOW_WOS_THREAD = MSG_INTERNAL,
    MSG_DEBUG_SHOW_WOS_MSG_POOL,
    MSG_DEBUG_SHOW_WOS_MEM,
    MSG_DEBUG_DISABLE_WOS,
    MSG_DEBUG_ENABLE_WOS,
    MSG_DEBUG_WOS_ALL,
    MSG_DEBUG_MODIFY_MAX_WOS_LOGIN_NUM,
    MSG_DEBUG_WOS_KICK_USER,
    MSG_DEBUG_WOS_DISABLE_NODE,
    MSG_DEBUG_WOS_ENABLE_NODE,
    MSG_DEBUG_WOS_RESET_MODE_ON,
    MSG_DEBUG_WOS_RESET_MODE_OFF,
    MSG_DEBUG_WOS_KEEPLIVE_KICK_MODE_ON,
    MSG_DEBUG_WOS_KEEPLIVE_KICK_MODE_OFF,
}MSG_WOS_DEBUG_E;

/* wos to lwos */
typedef enum {
    WOS_STATE_ACTIVE = 0,
    WOS_STATE_OVERLOAD,
    WOS_STATE_STANDBY,
    WOS_STATE_DOWN,
    WOS_STATE_STOPING,               // should stop WOS in the feature
    WOS_STATE_STOPPED,               // WOS stopped
}wos_state_e;


typedef enum{
	/*wos<-->user msg*/
	MSG_WOS_USER_START = (MSG_WOS),//917504
	MSG_WOS_USER_LOGIN,
	MSG_WOS_USER_OTHERWAYLOGIN,
	MSG_WOS_USER_LOGOUT,
	MSG_WOS_USER_SPEEDUP,
	MSG_WOS_USER_UNSPEEDUP,
	MSG_WOS_USER_GET_INFO,
	MSG_WOS_USER_GET_POPWINDOWSINFO,
	MSG_WOS_USER_GET_ONLINETIME,
	MSG_WOS_USER_SET_USERSTATE,
	MSG_WOS_USER_KEEPLIVE,
	MSG_WOS_USER_CHECK_STATE,
	MSG_WOS_USER_PRETEND_SPEEDUP,
	MSG_WOS_USER_REFRESH_SPEEDUP_TIME,
	MSG_WOS_USER_RESPEEDUP,
	MSG_WOS_USER_END,

	/*wos<-->lwos msg*/
	MSG_WOS_LWOS_START=(MSG_WOS+MSG_MODULE_INTERNAL_SPLIT),
	MSG_WOS_LWOS_KEEPLIVE,
	MSG_WOS_LWOS_END,

	MSG_WOS_CLT_START=(MSG_WOS+MSG_MODULE_INTERNAL_SPLIT*2),
	MSG_WOS_USER_LOGIN_ACK,
	MSG_WOS_USER_OTHERWAYLOGIN_ACK,
	MSG_WOS_USER_LOGOUT_ACK,
	MSG_WOS_USER_SPEEDUP_ACK,
	MSG_WOS_USER_UNSPEEDUP_ACK,
	MSG_WOS_USER_GET_INFO_ACK,
	MSG_WOS_USER_GET_POPWINDOWSINFO_ACK,
	MSG_WOS_USER_GET_ONLINETIME_ACK,
	MSG_WOS_USER_SET_USERSTATE_ACK,
	MSG_WOS_USER_KEEPLIVE_ACK,
	MSG_WOS_USER_CHECK_STATE_ACK,
	MSG_WOS_USER_PRETEND_SPEEDUP_ACK,
	MSG_WOS_USER_REFRESH_SPEEDUP_TIME_ACK,
	MSG_WOS_USER_RESPEEDUP_ACK,
	
	MSG_WOS_USER_LOGIN_BUSY_REFUSE,         /* WOS refuse user login for busy */
	MSG_WOS_USER_SYSTEM_STOP_REFUSE,        /* WOS refuse user for stopping */
	MSG_WOS_USER_SYSTEM_STOP_FORCE_ONLINE_KICK,
	
	MSG_WOS_CLT_END,
}MSG_WOS_MSG;



/* user's state */
typedef enum{
    //BASE RESULT STATE OK
    RESULT_OK=0,
    RESULT_RELOGIN_OK,
    
    ERROR_MSG_INVALID,
    ERROR_USER_INVALID ,
    ERROR_SYSTEM_REFUSE,
    ERROR_TICKET_INVALID,
    ERROR_TICKET_TIMEOUT,
    ERROR_STATE_NOT_CORRECT,
    ERROR_SYSTEM_ERROR,
    ERROR_PWD_INVALID,
    ERROR_NAME_INVALID,
    ERROR_NO_FREE_ADSL ,
    ERROR_SPEEDUP_TIME_NOT_ENOUGH,
    ERROR_VIP_LEVEL_INVALID,
    ERROR_STILL_SPEEDUP,
    ERROR_REFRESH_SPEEDUP_TIME_FAILD,
}ERROR_E;



typedef struct _wos_user_login_ack_s {
    unsigned int  user_id;    //user id
    char          ticket[TICKET_LEN*2+1];       //random key
    int           result; //user_login_state_e
    unsigned int  last_login;   // last login time
    char          wos_ip[MAX_NAME_LEN];   // wos server
    unsigned int  wos_port;
}user_lwos_login_ack_t, *puser_lwos_login_ack_t;


/* user login to lbs info */
typedef struct _user_login_lwos_s {
    char name[MAX_NAME_LEN];
    char pwd[MAX_PWD_LEN];
}user_login_lwos_t, *puser_login_lwos_t;



/*BEGIN WOS2*/

#define WOS_KEY_LEN	MAX_NAME_LEN+1
#define MAX_SPEEDUP_LEVEL		4
#define SPEEDUP_TIME_DETAIL		2
#define WOS_SVR_LEN	MAX_NAME_LEN*2+1
#define MSG_IS_USED 		1
#define MSG_NOT_USED		0
#define KEEP_LIVE_TIMES			3


typedef struct _wos_user_info_s {
	int 		userid;//user db id
	char 	username[MAX_NAME_LEN];//bandwidth name
	char		dftbw[MAX_NAME_LEN];//defualt bandwidth
	char		dstbw[MAX_NAME_LEN];//dest bandwidth
	char		bwname[MAX_NAME_LEN];//bandwidth name for co version
	char		bwpwd[MAX_NAME_LEN*2+1];//bandwidth name for co viersion
	char		key[WOS_SVR_LEN];// used by web
	char		loginip[MAX_NAME_LEN];
	int		userstate;//user state
	int		speedlevel;
	int		speeduplefttime[MAX_SPEEDUP_LEVEL];
	int		keeplive;
	char		timeremaining[MAX_NAME_LEN*4];//剩余提速时间
	float	accountbalance;//余额
	float	userpoints;
	int		accountlevel;
	UInt32 	logintime;
	UInt32	speedup_time;
	UInt32	speedupusetime;
	UInt32	endonlinetime;
	UInt32	endspeeduptime;
	UInt32     prevkeeplivetime;
}user_info, *puser_info;


typedef struct _wos_online_user_{
	UInt32  id;
	UInt32  uid;
	char name[MAX_NAME_LEN];
	int   state;
	int 	netmode;
	int apply_cnt;
	char login_time[MAX_NAME_LEN];
	char apply_time[MAX_NAME_LEN];
	char withdraw_time[MAX_NAME_LEN];
	char login_ip[MAX_NAME_LEN];
	int wos_id;
	char wos_ip[MAX_NAME_LEN];
	int wos_port;
	int vip_level;
	char adsl[MAX_NAME_LEN];
	char key[MAX_NAME_LEN*2+1];
}wos_online_user,*pwos_online_user;


typedef struct _adsl_account_info_{
	int id;
	int speedup_level;
	char adsl_name[MAX_NAME_LEN];
	char adsl_pwd[MAX_NAME_LEN*2];
}adsl_account_info,*padsl_account_info;

typedef enum{
	BW_ACCOUNT_NOT_USED=0,
	BW_ACCOUNT_USED,
	BW_ACCOUNT_RELOGIN,
}BW_STATUS;

struct user_login_msg
{
	char username[MAX_NAME_LEN];	
	char pwd[MAX_NAME_LEN+1];
	char key[WOS_SVR_LEN];	
};
struct user_login_result_msg
{
	int id;	
	int result;	
	char reserve1[MAX_NAME_LEN];	
	char reserve2[MAX_NAME_LEN];	
};

struct ticket_msg
{
	int id;	
	char username[MAX_NAME_LEN];	
	char key[WOS_SVR_LEN];	
	char reserve1[MAX_NAME_LEN];	
	char reserve2[MAX_NAME_LEN];	
};

struct set_user_state_msg{
	struct ticket_msg ticket;
	int state;
};

struct other_way_login_msg
{
	char openid[WOS_KEY_LEN];	
	char key[WOS_SVR_LEN];	
	char reserve1[MAX_NAME_LEN];	
	char reserve2[MAX_NAME_LEN];	
};
struct  keep_live_result_msg
{
	int result;	
	int userstate;
	char reserve1[MAX_NAME_LEN];		
	char reserve2[MAX_NAME_LEN];		
};

struct clt_pretend_speedup_result_msg{
	int result;
	int userstate;
	char reserve1[MAX_NAME_LEN];		
	char reserve2[MAX_NAME_LEN];	
};

struct user_refresh_speedup_time{
	char username[MAX_NAME_LEN];
	char key[WOS_SVR_LEN];
	char reserve1[MAX_NAME_LEN];		
	char reserve2[MAX_NAME_LEN]; 
};

struct user_refresh_speedup_time_result{
	int result;
};

struct  get_user_info_result_msg
{
	int result;	
	char username[MAX_NAME_LEN];	
	int defbw;	
	char bwpwd[MAX_NAME_LEN];		
	int destbw;	
	char timeremaining[MAX_NAME_LEN*4];		
	char accountbalance[MAX_NAME_LEN];	
	float userpoints;	
	int accountlevel;	
	char reserve1[MAX_NAME_LEN];		
	char reserve2[MAX_NAME_LEN];		
};
struct  get_pop_window_result_msg
{
	int  msgid;	
	char title[MAX_NAME_LEN*4];	
	char content[MAX_NAME_LEN*32];	
	char contentclickurl[MAX_NAME_LEN*4];	
	char imageurl[MAX_NAME_LEN*4];	
	char imageclickurl[MAX_NAME_LEN*4];	
	char reserve1[MAX_NAME_LEN];	
	char reserve2[MAX_NAME_LEN];	
};
struct  get_user_time_result_msg
{
	int result;	
	float onlinetime;	
	float sumonlinetime;	
};
struct  ret_adsl_account_msg
{
	int result;	
	char adslname[WOS_KEY_LEN];	
	char adslpwd[WOS_SVR_LEN];	
};

struct  check_state_result_msg
{
	int result;	
	char reserve1[MAX_NAME_LEN];	
	char reserve2[MAX_NAME_LEN];	
};

/*END WOS2*/

/* user login WOS */
typedef struct _user_wos_login_s {
    char   ticket[TICKET_LEN];
    char   name[MAX_NAME_LEN];   
    char   pwd[MAX_PWD_LEN];  
    int      net_mode;
}user_wos_login_t, *puser_wos_login_t, user_wos_first_login_t, *puser_wos_first_login_t;

typedef struct _user_wos_logout_s {
    int      user_id;
    char   name[MAX_NAME_LEN];   
}user_wos_logout_t, *puser_wos_logout_t;

typedef struct _user_wos_keeplive_s {
    int  id;
    int  req;
    char name[MAX_NAME_LEN];
}user_wos_keeplive_t, *puser_wos_keeplive_t;

/* user apply bandwidth */
typedef struct _user_wos_apply_s {
    int  id;              // user id
    int  bw_grp;          // target bandwidth group
    char name[MAX_NAME_LEN];
	int  dest_speed;
}user_wos_apply_t, *puser_wos_apply_t;

typedef struct _user_wos_keeplive_ack_s {
    int state;
}user_wos_keeplive_ack_t, *puser_wos_keeplive_ack_t;


/* wos to lwos notify */
typedef struct _wos_lwos_notify_s {
    int  wos_state; // wos_stat_e
    char wos_ip[MAX_NAME_LEN];
    int  wos_port;      
}wos_lwos_notify_t, *pwos_lwos_notify_t;

/* wos to lwos apply bussiness */
typedef struct _wos_lwos_apply_bsns_s {
    unsigned int group_id ; 
    unsigned int wos_id ; 
    unsigned int bsns_type; 
    unsigned int wos_bsns_port;
    char   wos_ip[MAX_NAME_LEN];
}wos_lwos_apply_bsns_t, *pwos_lwos_apply_bsns_t;


/* wos keeplive to lwos */
typedef struct _wos_lwos_keeplive_s {
    unsigned int  group_id ;         // wos group
    unsigned int  wos_id ;           // id in wos group
    int     wos_state;         // current state 
    unsigned int  refuse_accept;     // 0 : accept , 1: refuse 
    unsigned int  cur_user;
    unsigned int  max_user;
    unsigned int  offline_user;      // user state      
    unsigned int  normal_user;
    unsigned int  speedup_user;
    char    wos_ip[MAX_NAME_LEN];
    unsigned int  wos_port;
}wos_lwos_keeplive_t, *pwos_lwos_keeplive_t;


/* lwos keeplive to wos */
typedef struct _lwos_wos_keeplive_s {
    int    lwos_state ;    
    unsigned int wos_max_load; 
    char   lwos_ip[MAX_NAME_LEN];    
    unsigned int lwos_port;
}lwos_wos_keeplive_t, *plwos_wos_keeplive_t;

/* Important structure  */
typedef struct _user_wos_login_ack_s{    
	int           state;       // user_login_state_e
}user_wos_login_ack_t, *puser_wos_login_ack_t;

typedef struct _user_speedup_bill_t{
	int		id;
	char 	name[MAX_NAME_LEN];
	char		begin_time[MAX_NAME_LEN];
	char 	end_time[MAX_NAME_LEN];
	int		speedup_time;
	char 	adsl[MAX_NAME_LEN];
}user_speedup_bill,*puser_speedup_bill;

#ifdef __cplusplus
}
#endif


#endif

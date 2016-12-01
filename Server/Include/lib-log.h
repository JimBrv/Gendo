/*
  * Header : lib-log.h
  * Copyright : All right reserved by SecWin, 2010
  * Log msg implemetation
  * 
  * Ver   Author   Date         Desc
  * 0.1	wy      2010.8     initial version        
  */

#ifndef LIB_LOG_H
#define LIB_LOG_H


#include <syslog.h>
#include "lib-base.h"
#include "lib-types.h"
#include "lib-msg.h"
#include "lib-debug.h"
#include "lib-socket.h"

typedef enum {		
	LOG_LWOS_LOGIN_OK=0,
	LOG_LWOS_LOGIN_FAILD,
	LOG_LWOS_RELOGIN_OK,
	LOG_LWOS_RELOGIN_FAILD,
	LOG_LWOS_WOS_LOGIN_OK,
	LOG_LWOS_WOS_LOGIN_FAILD,
	LOG_LWOS_WOS_DOWN,
	
	LOG_WOS_LOGIN_OK,
	LOG_WOS_LOGIN_ALREADY_ONLINE,
	LOG_WOS_LOGIN_FIALD,
	LOG_WOS_RELOGIN_OK,
	LOG_WOS_RELOGIN_FAILD,
	LOG_WOS_ASSIGNED_OK,
	LOG_WOS_ASSIGNED_FAILD,
	LOG_WOS_LOGOUT_OK,
	LOG_WOS_LOGOUT_FAILD,
	LOG_WOS_KEEPLIVE_TIMEOUT,
	LOG_WOS_TAKEBACK_ADSL_OK,
	LOG_WOS_TAKEBACK_ADSL_FAILD,
	LOG_WOS_TIMEOUT_DEAL_FAILD,
	LOG_WOS_CLT_PRETEND_SPEEDUP_OK,
	LOG_WOS_CLT_PRETEND_SPEEDUP_FAILD,
	LOG_WOS_CANCLE_PRETEND_SPEEDUP_OK,
	LOG_WOS_CANCLE_PRETEND_SPEEDUP_FAILD,
	LOG_WOS_REFRESH_SPEEDUP_TIME_OK,
	LOG_WOS_REFRESH_SPEEDUP_TIME_FAILD,
	LOG_WOS_ADD_USER_SPEEDUP_BILL_FAILD,
}log_msg_e;

typedef struct _log_info_s {
	int   log;
	int   module;
	char *name;
}log_info_t, *plog_info_t;

typedef enum {
	LOG_DISP_DB     = 0x1,
	LOG_DISP_SYSLOG = 0x2,
	LOG_DISP_ALL    = LOG_DISP_DB | LOG_DISP_SYSLOG,
}log_disp_e;

#define MAX_LOG_ARGV 8       // max to 8 params

/* log client build this and send to server */
typedef struct _log_head_s {
	int  module;
	int  id;
	int  level;
	int  time;
	int  dispatch;
	char param_fmt[MAX_NAME_LEN*4];
	int  argc;
	char argv[MAX_LOG_ARGV][MAX_NAME_LEN*8];
}log_head_t, *plog_head_t;


/* log server will convert the client's log_head to concret log */
typedef struct _log_s {
	log_head_t log_head;      // 
	char      *log_format;    // contains the log msg format
	char       buf[0];
}log_t, *plog_t;


typedef struct _log_speedup_bill_s{
    char name[MAX_NAME_LEN];//user name
    char sbuf[MAX_NAME_LEN];//start speedup time buffer
    char ebuf[MAX_NAME_LEN];//end speedup time buffer
    int  speeduptime;
    int  speedup_bw;
}log_speedup_bill,*plog_speedup_bill;


/* client interface for all module */
int lib_log(int module, int id, ...);
int lib_log_cgs(int msgid, char* name, char* msg);

int lib_log_init(char *svrip, int svrport, int dispatch);
int lib_cgs_log_init(char *svrip, int svrport, int dispatch);

#endif

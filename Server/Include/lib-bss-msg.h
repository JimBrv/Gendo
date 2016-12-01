#ifndef BSS_MSG_H
#define BSS_MSG_H

#include "lib-msg.h"
#include "lib-types.h"

//user info len define
#define  BSS_COUNT_LEN                 4
#define  BSS_SEQUENCEID_LEN            4
#define  BSS_MESSEGEID_LEN             18
#define  BSS_SUBSCRBSTAT_LEN           5
#define  BSS_OPTRAREAID_LEN            8
#define  BSS_OPTRID_LEN                20
#define  BSS_ACPTSITEID_LEN            20
#define  BSS_CHANGETIME_LEN            14
#define  BSS_MDN_LEN                   21
#define  BSS_IMSI_LEN                  15
#define  BSS_OLD_MDN_LEN               21
#define  BSS_OLD_SERVICE_ID_LEN        5
#define  BSS_SERVICE_ID_LEN            5
#define  BSS_USERTYPE_LEN              1
#define  BSS_USERBRAND_LEN             5
#define  BSS_USERSCPTYPE_LEN           1
#define  BSS_USERPREPAIDID_LEN         16
#define  BSS_USERSERVICETYPE_LEN       1
#define  BSS_RESERVE1_LEN              40
#define  BSS_RESERVE2_LEN              40
#define  BSS_STATE_FLAG_LEN            1
#define  BSS_ERROR_REASON_LEN          3


//bind info len define
#define  BSS_LOGIN_NAME_LEN            16
#define  BSS_PASSWORD_LEN              16
#define  BSS_RESERVE_LEN               8
#define  BSS_BIND_RESULT_LEN           1

//msg head len define
#define  BSS_MESSAGE_LENGTH_LEN        4
#define  BSS_COMMAND_ID_LEN            4
#define  BSS_SEQUENCENUMBER            4

//for db operation 
#define  BSS_MSG_FORM_INVALID          1

enum{
    BSS_CMD_BIND = 0x1,
    BSS_CMD_UNBIND,
    BSS_CMD_USER_INFO,
    BSS_CMD_KEEPALIVE,

    BSS_CMD_BIND_RESP = 0x80000001,
    BSS_CMD_UNBIND_RESP,
    BSS_CMD_USER_INFO_RESP,
    BSS_CMD_KEEPALIVE_RESP,
}bss_cmd_id;

/*
 * MSG HEAD FOR ROP <---> CRM
 */
enum {
    BSS_US_START = 91000,
    BSS_US_NEW_USER,
    BSS_US_UPDATE_USER,
    BSS_US_CANCEL_USER,
    BSS_US_PCANCEL_USER,
    BSS_US_CCANCEL_USER,
    BSS_US_START_USER = 91009,
    BSS_US_YY_STOP,
    BSS_US_OWING_STOP,
    BSS_US_GH = 91020,
    BSS_US_UPDATE_SPEED = 91030,
    BSS_US_END= 91090,
}bss_user_msg;

//for msg head
typedef struct _bss_msg_head_{
    unsigned int msg_len;
    unsigned int cmd;
    unsigned int sequence_number;
}bss_msg_head,*pbss_msg_head;


#define RECORD_SEQUNCE_NUMBER_ERROR      101
#define SUBSCRBSTAT_ERROR                102
#define USERNAME_ERROR                   103
#define SERVICE_ID_ERROR                 104
#define OLDSERVICE_ID_ERROR              105
#define USERNAME_NOT_EXIST               106
#define NEW_USERNAME_ERROR               107
#define SYSTEM_ERROR                     108


//for msg user_info
typedef struct _bss_user_info_{
    unsigned int count;
    unsigned int sequenceid;
    char messageid[BSS_MESSEGEID_LEN];
    char subscrbstat[BSS_SUBSCRBSTAT_LEN];
    char optrareaid[BSS_OPTRAREAID_LEN];
    char optrid[BSS_OPTRID_LEN];
    char acptsiteid[BSS_ACPTSITEID_LEN];
    char changetime[BSS_CHANGETIME_LEN];
    char mdn[BSS_MDN_LEN];
    char imsi[BSS_IMSI_LEN];
    char oldmdn[BSS_OLD_MDN_LEN];
    char oldserviceid[BSS_OLD_SERVICE_ID_LEN];
    char serviceid[BSS_SERVICE_ID_LEN];
    char usertype;
    char userbrand[BSS_USERBRAND_LEN];
    char userscptype[BSS_USERTYPE_LEN];
    char userprepaidid[BSS_USERPREPAIDID_LEN];
    char userservicetype[BSS_USERSERVICETYPE_LEN];
    char reserve1[BSS_RESERVE1_LEN];
    char reserve2[BSS_RESERVE2_LEN];
}bss_user_info,*pbss_user_info;

typedef struct _bss_user_info_seq_{
    unsigned int sequencenumber;
    bss_user_info     bui;
}bss_user_info_seq,*pbss_user_info_seq;

typedef struct _bss_new_account_s{
    unsigned int sequencenumber;
    unsigned int sequenceid; 
    char name[MAX_NAME_LEN];
    char speed[MAX_NAME_LEN*2];
    int  areacode;
    char time[MAX_NAME_LEN];
}bss_new_account,*pbss_new_account;


typedef struct _bss_cancel_account_s{
    unsigned int sequencenumber;
    unsigned int sequenceid; 
    char name[MAX_NAME_LEN];
}bss_cancel_account,*pbss_cancel_account;

typedef struct _bss_start_account_s{
    unsigned int sequencenumber;
    unsigned int sequenceid;
    char name[MAX_NAME_LEN];
}bss_start_account,*pbss_start_account;

typedef struct _bss_stop_account_s{
    unsigned int sequencenumber;
    unsigned int sequenceid;
    char name[MAX_NAME_LEN];
}bss_stop_account,*pbss_stop_account;

typedef struct _bss_update_username_s{
    int sequencenumber;
    int sequenceid;
    char name[MAX_NAME_LEN];
    char newname[MAX_NAME_LEN];
}bss_update_username,*pbss_update_username;

typedef struct _bss_update_speed_s{
    unsigned int sequencenumber;
    unsigned int sequenceid;
    char name[MAX_NAME_LEN];
    char speed[MAX_NAME_LEN*2];
}bss_update_speed,*pbss_update_speed;

typedef struct _bss_operation_resp_s{
    bss_msg_head bmh;
    unsigned int count;
    unsigned int sequenceid;
    char stateflag;
    char errorreason[BSS_ERROR_REASON_LEN];
    char reserve[BSS_RESERVE_LEN];
}bss_operation_resp,*pbss_operation_resp;

#endif


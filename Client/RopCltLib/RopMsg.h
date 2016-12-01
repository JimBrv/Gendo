/* 
 * Define the client msg between client all modules.
 */
#ifndef ROP_CLT_MSG_H
#define ROP_CLT_MSG_H

#include "stdafx.h"
#include "lib-msg.h"
#include "map"
#include "set"
#include "string"
#pragma once
#pragma warning(disable:4995)  // Traffic Control was deprecated in Vista timeframe
#pragma warning(disable:4127)  // conditional expression is constant
#pragma warning(disable:4201)  // nameless struct/union
#pragma warning(disable:4503)
#include <stdlib.h>
#include <stdio.h>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <ntddndis.h>
#include <traffic.h>

using namespace std;

#define MSG_CLT_UI         (MSG_CLT)
#define MSG_CLT_USER_CTRL  (MSG_CLT + 0X1000)
#define MSG_CLT_ETM_CTRL   (MSG_CLT + 0X2000)
#define MSG_CLT_WEB_CTRL   (MSG_CLT + 0X3000)
#define MSG_CLT_ERR        (MSG_CLT + 0XF000)

#define MaxLength 32



/* Msg from UI to Ctrl */
enum {
    MSG_CTL_UI_USER_LOGIN = MSG_CLT_UI,
    MSG_CTL_UI_USER_APPLY_BW,
    MSG_CTL_UI_USER_WITHDRAW_BW,
    MSG_CTL_UI_USER_LOGOUT,    
    MSG_CTL_UI_NET_STATE,                // Network connect or disconn
    MSG_CTL_UI_SYS_STATUS,
	MSG_CLT_UI_GET_USER_STATE,
};


/* Msg for ETM */
enum {
	MSG_CTL_ETM_CTRL_START = MSG_CLT_ETM_CTRL,
	MSG_CTL_ETM_CTRL_STOP,

	/* ETM Policy->Group->Flow->Filter arches msg */
	MSG_CTL_ETM_CTRL_POLICY_GET, 
	MSG_CTL_ETM_CTRL_POLICY_SET,
	MSG_CTL_ETM_CTRL_POLICY_MODIFY,
	MSG_CTL_ETM_CTRL_GROUP_GET,
	MSG_CTL_ETM_CTRL_GROUP_SET,
	MSG_CTL_ETM_CTRL_GROUP_MODIFY,
	MSG_CTL_ETM_CTRL_FLOW_GET,
	MSG_CTL_ETM_CTRL_FLOW_SET,
	MSG_CTL_ETM_CTRL_FLOW_MODIFY,
	MSG_CTL_ETM_CTRL_RULE_GET,
	MSG_CTL_ETM_CTRL_RULE_SET,
	MSG_CTL_ETM_CTRL_RULE_MODIFY,

	/* ETM Ctrl<->Filter communication msg */
	MSG_CTL_ETM_FILTER_IP_CONNECT,            // ETM filter notify IP connection 
	MSG_CTL_ETM_FILTER_IP_DISCONNECT,         // ETM filter nofity IP disconnected
};


/* Client error msg */
enum {
    MSG_CLT_ERR_CONNECT_LBS_FAIL = MSG_CLT_ERR,   // Connect to LBS server error
    MSG_CLT_ERR_CONNECT_RAS_FAIL,                 // Connect to RAS server error
    MSG_CLT_ERR_APPLY_REFUSE,                     // Apply request refuse by server
    MSG_CLT_ERR_WITHDRAW_REFUSE,                  // Withdraw request refuse by server
    MSG_CLT_ERR_USER_INVALID,                     // User is invalid
    MSG_CLT_ERR_REDIAL,                           // Redial PPPoE fail
    MSG_CLT_ERR_UI_DISCONNECT,                    // UI disconnect with service
    MSG_CLT_ERROR_AAA_UNREACHABLE,                // AAA system check user failed
    MSG_CLT_ERROR_SERVER_CONNECTION,              // AAA system check user failed
    MSG_CLT_ERROR_USER_CREDIT_INSUFFICIENT,       // Credit insuf
    MSG_CLT_ERROR_USER_CREDIT_ZERO_NOTIFY,         // Credit is Zero. nofity
    MSG_CLT_ERROR_USER_CREDIT_ZERO_WITHDRAW_FORCE, // Credit is Zero. forth to withdraw
    MSG_CLT_ERROR_USER_CREDIT_ZERO_APPLY_REFUSE,   // Credit is Zero. apply refuse
    MSG_CLT_ERROR_USER_HAS_LOGINED,                // User relogin, force logout
    MSG_CLT_ERROR_USER_CREDIT_CHARGE_OK,           // User charge credit
    MSG_CLT_ERROR_USER_CREDIT_CHARGE_FAIL_REFUSE,  // Server Refuse credit charge
	ROP_CLT_ERROR_PPPOE_DIAL_USER_INVALID,
	ROP_CLT_ERROR_PPPOE_DIAL_TIMEOUT,
	ROP_CLT_ERROR_PPPOE_DIAL_FAIL,
    ROP_CLT_ERROR_USER_RELOGIN_FAIL,
};

enum
{
   ETM_PROTO_TCP=1,
   ETM_PROTO_UDP,
   ETM_PROTO_IP,
};

/* Base error info for all modules */
typedef struct _err_s {
    int sModule;
    int dModule;
    int err;
    int len;
    char payload[0];
}ModErr, *PModErr;


#define NET_STATE_DISCONNECT 0
#define NET_STATE_CONNECTED  1

#define FLOW_GROUP_HTTP "http"
#define FLOW_GROUP_FTP "ftp"
#define FLOW_GROUP_P2P "p2p"
#define FLOW_GROUP_OTHER "other"
#define FLOW_GROUP_BEST "Best"
#define FLOW_NAME_HTTP "httpflow"
#define FLOW_NAME_FTP "ftpflow"
#define FLOW_NAME_P2P "p2pflow"
#define FLOW_NAME_OTHERTCP "othertcpflow"
#define FLOW_NAME_BEST "bestflow"

/* MSG_CTL_UI_NET_STATE msg content */
typedef struct _net_state_s {
    int iState;                // 0-disconnect, 1-connected
    int iType;                 // 1-ethernet, 2-pppoe
    unsigned int uRx;          // recv bytes
    unsigned int uTx;          // sent bytes
}MsgNetState, *PMsgNetState;

/* MSG_CTL_UI_NET_STATE msg content */
typedef struct _sys_status_s {
    double uNetRxRate;          // recv rate
    double uNetTxRate;          // sent rate
    double uCpuRate;            // cpu rate
}MsgSysStatus, *PMsgSysStatus;


/* MSG_CTL_UI_NET_STATE msg content */
typedef struct _req_ack_s {
    int iState;                // 0-OK, else Error code
}MsgReqAck, *PMsgReqAck;

/*STRUCT FILTER*/
typedef struct _filter_s{
	HANDLE hFlow;//filter所属的flow句柄
	ULONG Address;
	ULONG port;
	ULONG protocolId;
	ULONG hself;//自身句柄
	ULONG procid;
}FilterStruct,*PFilterStruct;

/*STRUCT FLOW*/
typedef struct _flow_s{
	HANDLE hInterface;//flow所属的网卡句柄
	HANDLE hself;
	int Bw;//每一个flow的带宽大小
	int ActualBw;//实际带宽大小
	int DSCPValue;
    int OnePValue;
	int ThrottleRate;
	int procid;
}FlowStruct,*PFlowStruct;

/*FLOW HANDLE*/
typedef HANDLE FlowHandler;

/*FILTER HANDLE*/
typedef HANDLE FilterHandler;

typedef HANDLE InfcHandler;

/*PROCESS ID*/
typedef ULONG Pid;

typedef unsigned long etm_ip;

/*FILTER LIST:use ip as filter list key*/
typedef map<etm_ip,FilterStruct> Filter_list;

typedef map<InfcHandler,Filter_list> FilterList_pInf;

typedef map<Pid,FilterList_pInf> FilterlistMain;

typedef string namestr;

/*FLOW LIST*/
typedef map<namestr,FlowStruct>Flow_list;

//Group
typedef struct _flow_list_s{
 int bw;//带宽
 ULONG used;
 Flow_list flowlist;
}FlowListStr,*PFlowListStr;

/*Etm_Grop LIST*/
typedef map<namestr,FlowListStr> Etm_Group;

typedef set<ULONG,less<ULONG>> BestIpList;

typedef map<string,HANDLE> HBestFilter;
//typedef struct _etm_policy_s{
//	bool isused;//是否是使用中的策略
//	float bw;//带宽
//	float usedbw;//已用带宽
//	Etm_Group etmgroup;
//}EtmPolicyStr,*PEtmPolicyStr;

/*Policy LIST:use Etm_Policy name as key*/
//typedef map<namestr,EtmGroupStr> Etm_Policy;

//typedef struct _etm_policy_s{
//	int bw;//带宽
//	Etm_Policy etmpolicy;
//}EtmPolicyStr,*PEtmPolicyStr;

/*Etm main list content all the map*/
//typedef map<namestr,EtmPolicyStr>Etm_MainList;


/*msg:tdi send ip and port to tc */
typedef struct _tdi_tc_conect_info_s{
	ULONG pid;
	ULONG ip;
	ULONG port;
	ULONG protocolType;
}STTCIS,*PSTTCIS;


/*msg:tc send start command to tdi*/
typedef struct _tc_tdi_start_s{
	int port;
}TTSS,*PTTSS;

#define NOT_SPECIFIED 0xFFFF
typedef struct ifc_info
{
	HANDLE hIfc;//网卡句柄
	HANDLE hFlow;//流句柄
	HANDLE hFilter;  //规则
} IFC_INFO, *PIFC_INFO;

typedef struct ifc_list
{
	ULONG IfcCount;
	PIFC_INFO pIfcInfo;
} IFC_LIST, *PIFC_LIST;

/* network order etm address, from client modules */
typedef struct EtmBestIPs_s {
	int          cnt;
	unsigned int ip[RAS_ETM_BEST_IP_CNT];
}EtmBestIPs, *PEtmBestIPs;

typedef struct userctrl_userState
{
	int  iState;
}UserctrlUserState, *pUserctrlUserState;

#endif
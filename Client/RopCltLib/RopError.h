#ifndef ROP_ROPAPIERR_H
#define ROP_ROPAPIERR_H

#include "RopUtil.h"
#include "RopSyncSocket.h"
#define ErrorFile    L"ErrInfo"
#define DLL_VERSION  "HNUnicom-V1R2-20120630"

class RopError
{
public:
	RopError();
	~RopError();
	INT    SetUserName(CHAR* pUserName);
	INT    SetInfo(CHAR* pInfo);
	INT    WriteInfo(short ErrNum);
	VOID   SetNetMode(int imode){m_net_mode = imode;}; // 0 PPPOE,  1 route
	INT    SendErr();
	VOID    SetLbsAddr(CHAR* pUrl){strcpy_s(m_Lbs_IP,MAX_NAME_LEN,pUrl);};
private:
	int   GetClientInfo(HANDLE fileHandle,pclient_error& pErrorInfo);
private:
	CHAR		m_user_name[MAX_NAME_LEN];
	CHAR		m_client_version[MAX_NAME_LEN];
	CHAR		m_os_version[MAX_NAME_LEN];
	SHORT		m_net_mode;
	CHAR		m_safe_soft[MAX_NAME_LEN];
	CHAR		m_resave[MAX_NAME_LEN];
	SHORT		m_error_cnt;
	CHAR		m_time[MAX_NAME_LEN];
	SHORT		m_error_id;
	SHORT		m_repeat;
	CHAR		m_info[MAX_NAME_LEN];

	WCHAR		m_wErrFilePath[MAX_PATH];
	CHAR		m_Lbs_IP[MAX_NAME_LEN];
	DWORD		m_port;	
};


#endif
/*
 * L2TP VPN support 
 * GendoVPN, 2013
 */

#ifndef ROP_L2TP_H
#define ROP_L2TP_H
#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <Ras.h>
#include <RasError.h>
#include "RopBase.h"

#define ROP_L2TP_DEVICE_NAME  TEXT("Gendo WAN Miniport (L2TP)")
#define ROP_L2TP_DEVICE_TYPE  TEXT("RASDT_Vpn")
#define ROP_L2TP_ENTRY_NAME   TEXT("gendo-l2tp")
#define ROP_L2TP_PBK_PATH     TEXT("gendo-l2tp.pbk")

typedef void (WINAPI *pDialCB)(UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwErro);

class RopL2TP
{
public:
	RopL2TP();
	~RopL2TP() {/*HangUp();*/};
	void SetIpUserPwd(TCHAR *sIp, TCHAR *sUserName, TCHAR *sPwd)
	{
	    wcscpy(m_sUserName, (const wchar_t*)sUserName);
		wcscpy(m_sUserPwd, (const wchar_t*)sPwd);
		wcscpy(m_sIp, (const wchar_t*)sIp);
		wcscpy(m_sEntryName, (const wchar_t*)ROP_L2TP_ENTRY_NAME);
		wcscpy(m_sDeviceName, (const wchar_t*)ROP_L2TP_DEVICE_NAME);
		wcscpy(m_sDeviceType, (const wchar_t*)ROP_L2TP_DEVICE_TYPE);
	}

	int  Dial(pDialCB pFunc);
	int  DialLoop();
	void HangUp();
	void HangUpAll();
	int  ReDial(DWORD dwSleep);
	BOOL IsConnected() {return m_bConn;}
	BOOL IsL2TPLink();
    int  CreateL2TPEntry(TCHAR *sName);

	static void WINAPI RasDialFunc(UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwError);
    
protected:
	TCHAR m_sEntryName[MAX_NAME_LEN];
	TCHAR m_sDeviceType[MAX_NAME_LEN];
	TCHAR m_sDeviceName[MAX_NAME_LEN];
	TCHAR m_sUserName[MAX_NAME_LEN];
	TCHAR m_sUserPwd[MAX_PWD_LEN];
	TCHAR m_sIp[MAX_NAME_LEN];

private:
	HRASCONN m_hRasConn; 
	BOOL     m_bConn;
	BOOL     m_bOpen;
};


#endif
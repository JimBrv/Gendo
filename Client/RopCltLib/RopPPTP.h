/*
 * PPTP VPN support 
 * All right reserved by Extelecom, 2012
 */

#ifndef ROP_PPPTP_H
#define ROP_PPPTP_H
#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <Ras.h>
#include <RasError.h>


#include "RopBase.h"

#define ROP_PPTP_DEVICE_NAME  TEXT("Gendo WAN Miniport (PPTP)")
#define ROP_PPTP_DEVICE_TYPE  TEXT("RASDT_Vpn")
#define ROP_PPTP_ENTRY_NAME   TEXT("gendo")
#define ROP_PPTP_PBK_PATH     TEXT("gendo.pbk")

typedef void (WINAPI *pDialCB)(UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwErro);

class RopPPTP
{
public:
	RopPPTP();
	~RopPPTP() {/*HangUp();*/};
	void SetIpUserPwd(TCHAR *sIp, TCHAR *sUserName, TCHAR *sPwd)
	{
	    wcscpy(m_sUserName, (const wchar_t*)sUserName);
		wcscpy(m_sUserPwd, (const wchar_t*)sPwd);
		wcscpy(m_sIp, (const wchar_t*)sIp);
		wcscpy(m_sEntryName, (const wchar_t*)ROP_PPTP_ENTRY_NAME);
		wcscpy(m_sDeviceName, (const wchar_t*)ROP_PPTP_DEVICE_NAME);
		wcscpy(m_sDeviceType, (const wchar_t*)ROP_PPTP_DEVICE_TYPE);
	}

	int  Dial();
	int  DialLoop();
	void HangUp();
	void HangUpWithout3G();
	void HangUpAll();
	int  SelectivityHangUp(int* ret);
	int  ReDial(DWORD dwSleep);
	BOOL IsConnected() {return m_bConn;}
	BOOL IsRopLink();
	int  WaitForHangupAndDial(DWORD dwSleep);
	int  EnumRasEntry();
	BOOL IsPPTPRas(LPRASENTRYNAME rasEntry);
	int  CreateRopLinkEntry(LPRASENTRYNAME rasEntry);
	static void WINAPI RasDialFunc(UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwError);

    int  CreatePPTPEntry(TCHAR *sName);
    int  DialPPTP(pDialCB pFunc);

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
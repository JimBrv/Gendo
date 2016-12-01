#ifndef ROP_PPPOE_H
#define ROP_PPPOE_H
#include "stdafx.h"
#include "RopBase.h"
#include <winsock2.h>
#include <windows.h>
#include <Ras.h>
#include <RasError.h>
#include "lib-msg.h"

#define ROP_PPPOE_DEVICE_NAME  TEXT("Rop WAN Miniport (PPPOE)")
#define ROP_PPPOE_DEVICE_TYPE  TEXT("RASDT_PPPoE")
#define ROP_PPPOE_ENTRY_NAME   TEXT("RopLink")
#define ROP_PPPOE_PBK_PATH     TEXT("ROP.pbk")

class RopPPPoE : public RopBase
{
public:
    RopPPPoE();
    ~RopPPPoE() {/*HangUp();*/};
    void SetUserPwd(TCHAR *sUserName, TCHAR *sPwd) {wcscpy(m_sUserName, (const wchar_t*)sUserName);wcscpy(m_sUserPwd, (const wchar_t*)sPwd);}
    int  Open(TCHAR *sEntryName = ROP_PPPOE_ENTRY_NAME, 
              TCHAR *sDevName = ROP_PPPOE_DEVICE_NAME, 
              TCHAR *sDevType = ROP_PPPOE_DEVICE_TYPE);
    int  Dial();
    int  DialLoop();
    void HangUp();
	void HangUpWithout3G();
    void HangUpAll();
	int SelectivityHangUp(int* ret);
    int  ReDial(DWORD dwSleep);
    BOOL IsConnected() {return m_bConn;}
    BOOL IsRopLink();
	int  WaitForHangupAndDial(DWORD dwSleep);
	int  EnumRasEntry();
	BOOL IsPPPoERas(LPRASENTRYNAME rasEntry);
	int  CreateRopLinkEntry(LPRASENTRYNAME rasEntry);
    static void WINAPI RasDialFunc(UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwError);

protected:
    TCHAR m_sEntryName[MAX_NAME_LEN];
    TCHAR m_sDeviceType[MAX_NAME_LEN];
    TCHAR m_sDeviceName[MAX_NAME_LEN];
    TCHAR m_sUserName[MAX_NAME_LEN];
    TCHAR m_sUserPwd[MAX_PWD_LEN];

private:
    HRASCONN m_hRasConn; 
    BOOL     m_bConn;
    BOOL     m_bOpen;
};


#endif
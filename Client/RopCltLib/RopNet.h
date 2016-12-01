#ifndef ROP_NET_H
#define ROP_NET_H
#include "stdafx.h"
#include <Windows.h>
#include <string.h>
#include <Iphlpapi.h>
#include "RopBase.h"
#include "RopUtil.h"
#include "RopProcess.h"
#include "RopNetState.h"


char* GetFileStr(TCHAR* FilePath);
BOOL  RegIP( char*  lpszAdapterName, char* pIPAddress, char* pNetMask, char* pNetGate,  char* pDNSServer1,  char* pDNSServer2 ) ;
BOOL  RegName( char* lpszAdapterName, char* Name);
BOOL  SetIp(LPCSTR lpstrAdapter,LPCSTR lpstrDestAddr, LPCSTR lpstrMask, LPCSTR lpstrGateway);
BOOL  SetIpDHCP(LPCSTR lpstrAdapter);
BOOL  SetDNS(LPCSTR lpstrAdapter,LPCSTR lpstrDNS1,LPCSTR lpstrDNS2);
BOOL  SetDNSDHCP(LPCSTR lpstrAdapter);
BOOL  SetMetric(LPCSTR lpstrAdapter,DWORD dwMetric);
BOOL  DHCPRenew(char *NicName);


#endif


#ifndef ROP_EKP_H
#define ROP_EKP_H
#include "stdafx.h"
#include "stdio.h"
#include "windows.h"
//#include <shlwapi.h>
#include <winbase.h>
#include <Psapi.h>
#include <string>

BOOL RopStopProcess(const WCHAR* strName);
BOOL RopStopProcessA(CHAR *strName);

BOOL RopCreateProcess(TCHAR *strName, BOOL wait = TRUE);
BOOL RopCreateProcessA(CHAR *strName, BOOL wait = TRUE);
/* result back */
BOOL RopCreateProcessR(TCHAR *strName, char* result);

BOOL EnhancePriv(VOID);
BOOL RopOpenURL(TCHAR *Url, HANDLE Hwnd);
std::string RopHttpRequest(TCHAR *lpHostName, short sPort, TCHAR *lpUrl, TCHAR *lpMethod, TCHAR *lpPostData, INT nPostDataLen);

#endif ROP_EKP_H
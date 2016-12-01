#include "stdafx.h"
#include "RopUtil.h"
#include "RopProcess.h"
#include <Shellapi.h>
#include <Objbase.h>
#include <wininet.h>
#include <string>

using namespace std;

BOOL EnhancePriv(VOID) 
{
    HANDLE   hToken; 

    BOOL bRet = OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&hToken);
    if (!bRet) {
        LDebug(DEBUG_L_ERROR, "OpenProcessToken error=%d\n", GetLastError());
        return bRet;
    }
    
    TOKEN_PRIVILEGES   tp;    
    tp.PrivilegeCount=1;    
    LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&tp.Privileges[0].Luid);    
    tp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;    
    AdjustTokenPrivileges(hToken,FALSE,&tp,sizeof(tp),NULL,NULL);    
    CloseHandle(hToken);
    return TRUE;    
}


BOOL RopStopProcess(const WCHAR *strName)
{
	BOOL b = FALSE;
	unsigned long aProc[1024];
	unsigned long cbN = 0;
    if (!EnhancePriv()) {
        LDebug(DEBUG_L_ERROR, "Enhance Priv fail, cant kill\n");
        return FALSE;
    }
	if (EnumProcesses(&aProc[0],sizeof(aProc),&cbN)) {
		int c = cbN/sizeof(long);
		int i;
		for (i = 0;i < c;i++){
			HANDLE hP;
			hP = OpenProcess(PROCESS_ALL_ACCESS,false,aProc[i]);
			if (hP) {
				HMODULE hMod;
				if(EnumProcessModules(hP,&hMod,sizeof(hMod),&cbN)){
					WCHAR pName[MAX_PATH];
					WCHAR pBaseName[MAX_PATH];
					pName[0] = pBaseName[0] = 0;
					if (GetModuleFileNameEx(hP, hMod, pName, sizeof(pName))) {
						if (GetModuleBaseName(hP, hMod, pBaseName, sizeof(pBaseName))) {
							if( (0 == wcscmp(pName, strName))||(0 == wcscmp(pBaseName,strName)) ){
                                LDebug(DEBUG_L_NOTICE, "Find process: %s(%s) killed\n", pName, pBaseName);
								b = TerminateProcess(hP,0);
							}
						}
					}
				}else{
                    LDebug(DEBUG_L_ERROR, "EnumProcFail error=%d\n", GetLastError());
                }
			}else{
                LDebug(DEBUG_L_ERROR, "OpenProcess error=%d\n", GetLastError());
            }
		}
	}
	return b;
}

BOOL RopStopProcessA(CHAR *strName)
{
    TCHAR cmdLine[256] = {0};
    INT   len = 256;
    mb2wc(strName, strlen(strName), cmdLine, &len);
    return RopStopProcess(cmdLine);
}



BOOL RopCreateProcess(TCHAR *strName, BOOL wait)
{
    STARTUPINFO m_StartupInfo;
    PROCESS_INFORMATION m_ProcessInfo;
    GetStartupInfo (&m_StartupInfo);
    m_StartupInfo.wShowWindow = SW_HIDE;
    TCHAR cmdLine[256] = {0};
    lstrcpy(cmdLine, strName);

    
    if (!EnhancePriv()) {
        LDebug(DEBUG_L_ERROR, "Enhance Priv fail, cant kill\n");
        return FALSE;
    }

    BOOL l_Result = CreateProcess(
        NULL,
        (LPWSTR)cmdLine,
        NULL,
        NULL,
        FALSE,
        CREATE_SEPARATE_WOW_VDM | CREATE_NO_WINDOW | CREATE_DEFAULT_ERROR_MODE | IDLE_PRIORITY_CLASS,
        //CREATE_DEFAULT_ERROR_MODE | IDLE_PRIORITY_CLASS,
        NULL,
        NULL,
        &m_StartupInfo,
        &m_ProcessInfo);

    LDebug(DEBUG_L_INFO,"Command: %S\n", strName);

    if (l_Result) {
        DWORD l_State = 0;
        int iHelloCount = 0;

        if (wait) {
            while (GetExitCodeProcess(m_ProcessInfo.hProcess, &l_State) && l_State == STILL_ACTIVE) {
                Sleep(500);
                LDebug(DEBUG_L_INFO, "Waiting command...\n");
            }
        }

        CloseHandle(m_ProcessInfo.hProcess);
        CloseHandle(m_ProcessInfo.hThread);
        return TRUE;
    } else {
        LDebug(DEBUG_L_ERROR,"Failed to start [%s]\n", strName);
        return FALSE;
    }
    return TRUE;
}

BOOL RopCreateProcessA(CHAR *strName, BOOL wait)
{
    TCHAR cmdLine[256] = {0};
    INT   len = 256;
    mb2wc(strName, strlen(strName), cmdLine, &len);
    return RopCreateProcess(cmdLine, wait);
}

BOOL RopOpenURL(TCHAR *Url, HANDLE Hwnd)
{
    TCHAR cmdLine[256] = {0};
    INT   len = 256;
    BOOL  ret = TRUE;
    if (!EnhancePriv()) {
        LDebug(DEBUG_L_ERROR, "Enhance Priv fail, cant kill\n");
        return FALSE;
    }
    SHELLEXECUTEINFO sei;
    ZeroMemory(&sei, sizeof(SHELLEXECUTEINFO));
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    // Shit Microsoft Win8, root cannot get HTTP associated with Browser, fuck
    //HINSTANCE hIns = ShellExecute((HWND)Hwnd, _T("open"), _T("rundll32.exe"), _T("url.dll,FileProtocolHandler http://www.google.com"),NULL,SW_SHOWNORMAL);
    //HINSTANCE hIns = ShellExecute(NULL, _T("open"), Url, NULL, NULL, SW_SHOWNORMAL);
    //if ((INT)hIns <= 32) {
    //    LDebug(DEBUG_L_ERROR, "Open URL failed=%d\n", GetLastError());
    //    ret = FALSE;
    //}

    sei.cbSize = sizeof(SHELLEXECUTEINFO);
    sei.lpVerb = _T("open");
    sei.lpFile = Url;
   // lstrcpy(sei.lpFile, Url); 
    
    if(!ShellExecuteEx(&sei)) {
        DWORD le = GetLastError();
    }

    CoUninitialize();
    
    return ret;
}

BOOL RopCreateProcessR(TCHAR *strName, char* result)
{
    memset(result, 0 , sizeof(result));
    STARTUPINFO m_StartupInfo;
    PROCESS_INFORMATION m_ProcessInfo;
    TCHAR cmdName[256] = {0};
    lstrcpy(cmdName, strName);

    SECURITY_ATTRIBUTES sa;
    HANDLE hRead,hWrite;
    sa.nLength=sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor=NULL;
    sa.bInheritHandle=TRUE;
    if (!CreatePipe(&hRead,&hWrite,&sa,0)) {
        return -1;
    }

    m_StartupInfo.cb=sizeof(STARTUPINFO);
    GetStartupInfo (&m_StartupInfo);
    m_StartupInfo.hStdError=hWrite;
    m_StartupInfo.hStdOutput=hWrite;
    m_StartupInfo.wShowWindow = SW_HIDE;
    m_StartupInfo.dwFlags=STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    BOOL l_Result = CreateProcess(
        NULL,
        (LPWSTR)cmdName,
        NULL,
        NULL,
        TRUE,
        0,//CREATE_SEPARATE_WOW_VDM | CREATE_NO_WINDOW | CREATE_DEFAULT_ERROR_MODE | IDLE_PRIORITY_CLASS,
        NULL,
        NULL,
        &m_StartupInfo,
        &m_ProcessInfo);

    CloseHandle(hWrite);
    LDebug(DEBUG_L_INFO,"Command: %S\n", strName);

    DWORD bytesRead;
    char buffer[128] = {0};
    while (ReadFile(hRead,(LPVOID)&buffer,127,&bytesRead,NULL)){
        strcat(result,buffer);
        memset(&buffer,0,128);
    }

    if (l_Result) {
        DWORD l_State;
        int iHelloCount = 0;
        while (GetExitCodeProcess(m_ProcessInfo.hProcess, &l_State) && l_State == STILL_ACTIVE) {
            LDebug(DEBUG_L_ERROR,"Waiting cmd...\n");
            Sleep(200);
        }
        CloseHandle(m_ProcessInfo.hProcess);
        CloseHandle(m_ProcessInfo.hThread);
        CloseHandle(hRead);
        return l_State;
    } else {
        LDebug(DEBUG_L_ERROR,"Failed to start [%s]\n", strName);
        CloseHandle(hRead);
        return -1;
    }
    CloseHandle(hRead);
    LDebug(DEBUG_L_ERROR,"Exist cmd...\n");
    return 0;
}

string RopHttpRequest(TCHAR *lpHostName, short sPort, TCHAR *lpUrl, TCHAR *lpMethod, TCHAR *lpPostData, INT nPostDataLen)
{
    HINTERNET hInternet,hConnect,hRequest;
    BOOL bRet;

    string strResponse;

    hInternet = (HINSTANCE)InternetOpen(_T("User-Agent"), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
    if(!hInternet)
        goto out;

    hConnect = (HINSTANCE)InternetConnect(hInternet, lpHostName, sPort, NULL, _T("HTTP/1.1"), INTERNET_SERVICE_HTTP, 0, 0);
    if(!hConnect)
        goto out;

    hRequest = (HINSTANCE)HttpOpenRequest(hConnect,lpMethod,lpUrl, _T("HTTP/1.1"),NULL,NULL,INTERNET_FLAG_RELOAD,0);
    if(!hRequest)
        goto out;

    //bRet = HttpAddRequestHeaders(hRequest,"Content-Type: application/x-www-form-urlencoded",Len(FORMHEADERS),HTTP_ADDREQ_FLAG_REPLACE | HTTP_ADDREQ_FLAG_ADD);
    //if(!bRet)
    //goto out;

    bRet = HttpSendRequest(hRequest,NULL,0,lpPostData,nPostDataLen);
    while(TRUE)
    {
        char cReadBuffer[4096];
        unsigned long lNumberOfBytesRead;
        bRet = InternetReadFile(hRequest,cReadBuffer,sizeof(cReadBuffer) - 1,&lNumberOfBytesRead);
        if(!bRet || !lNumberOfBytesRead)
            break;
        cReadBuffer[lNumberOfBytesRead] = 0;
        strResponse = strResponse + cReadBuffer;
    }

out:
    if(hRequest)
        InternetCloseHandle(hRequest);
    if(hConnect)
        InternetCloseHandle(hConnect);
    if(hInternet)
        InternetCloseHandle(hInternet);
    return strResponse;
}
/*
 * Gendo L2TP VPN connection support
 *                           gendo,2013
 */
#include "stdafx.h"
#include <string>
#include "RopL2TP.h"
#include "RopUtil.h"

using namespace std;

RopL2TP::RopL2TP(void)
{
    ZeroMemory(m_sDeviceName, sizeof(m_sDeviceName));
    ZeroMemory(m_sDeviceType, sizeof(m_sDeviceType));
    ZeroMemory(m_sEntryName, sizeof(m_sEntryName));
    ZeroMemory(m_sUserName, sizeof(m_sUserName));
    ZeroMemory(m_sUserPwd, sizeof(m_sUserPwd));
    m_hRasConn = NULL;
    m_bConn = false;
    m_bOpen = true;
}

#define IsSelfEntry(pRasCon)  !lstrcmpi((pRasCon)->szEntryName, ROP_L2TP_ENTRY_NAME)

void WINAPI RopL2TP::RasDialFunc(UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwError)
{
    TCHAR *str = NULL;
    switch(rasconnstate)
    {
    case RASCS_OpenPort:
        str= TEXT("Opening Port...");
        break;
    case RASCS_PortOpened:
        str= TEXT("Port opened successful!");
        break;
    case RASCS_ConnectDevice:
        str= TEXT("Connecting Device...");
        break;
    case RASCS_DeviceConnected:
        str= TEXT("Device connected successful!");
        break;
    case RASCS_AllDevicesConnected:
        str= TEXT("All Device connected successful!");
        break;
    case RASCS_Authenticate:
        str= TEXT("Authorizing User/Pwd...");
        break;
    case RASCS_AuthNotify:
        if (dwError == 0) {
            str= TEXT("Authorizing User/Pwd processing...!");
        }else{
            str= TEXT("Authorizing User/Pwd failed...!");
        }
        break;
    case RASCS_AuthRetry:
        str= TEXT("Re-Auth User/Pwd...");
        break;
    case RASCS_AuthCallback:
        str= TEXT("Auth callback!");
        break;
    case RASCS_AuthChangePassword:
        str= TEXT("Auth Change Pwd req...");
        break;
    case RASCS_AuthProject:
        str= TEXT("Auth procedure starting...");
        break;
    case RASCS_AuthLinkSpeed:
        str= TEXT("Auth Link Speed...");
        break;
    case RASCS_AuthAck:
        str= TEXT("Authentication Acknowledged");
        break;
    case RASCS_ReAuthenticate:
        str= TEXT("Reauthenticatation Started");
        break;
    case RASCS_Authenticated:
        str= TEXT("Authenticated");
        break;
    case RASCS_PrepareForCallback:
        str= TEXT("Preparation For Callback");
        break;
    case RASCS_WaitForModemReset:
        str= TEXT("Waitin for modem reset...");
        break;
    case RASCS_WaitForCallback:
        str= TEXT("Waiting For Callback");
        break;
    case RASCS_Interactive:
        str= TEXT("Interactive");
        break;
    case RASCS_RetryAuthentication:
        str= TEXT("Retry Authentication!");
        break;
    case RASCS_CallbackSetByCaller:
        str= TEXT("Callback Set By Caller");
        break;
    case RASCS_PasswordExpired:
        str= TEXT("Password is Expired!");
        break;
    case RASCS_Connected:
        str= TEXT("Connection is OK!");
        break;
    case RASCS_Disconnected:
        str= TEXT("Connection is disconnected!!!");
        break;
    default:
        str= TEXT("Undefine!");
        break;
    }

    LDebug(DEBUG_L_ERROR, "L2TP Dial state: %S\n", str);
    /* Notify UI */
}

/* Sync loop for while dialing */
int RopL2TP::DialLoop()
{
    LDebug(DEBUG_L_ERROR, "L2TP: DialLoop...\n");
	if (IsL2TPLink()) {
		LDebug(DEBUG_L_ERROR, "Has L2TP already\n");
		return 0;
	}

	HRASCONN m_hRasConn = NULL;
	RASDIALPARAMS rdParams; 
	ZeroMemory(&rdParams, sizeof(RASDIALPARAMS));


	rdParams.dwSize = sizeof(RASDIALPARAMS);
	lstrcpy(rdParams.szEntryName,m_sEntryName);
	lstrcpy(rdParams.szUserName, m_sUserName);
	lstrcpy(rdParams.szPassword, m_sUserPwd);


	WCHAR PbkPATH[MAX_PATH] = {0};
	WCHAR wDirPathFile[MAX_PATH] = {0};
	char szPathAnsci[MAX_PATH] = {0};
	char drive[_MAX_DRIVE] = {0};
	char dir[_MAX_DIR] = {0};
	char Path[MAX_PATH] = {0};
	size_t Len = MAX_PATH;
	GetModuleFileName(NULL,wDirPathFile,MAX_PATH);
	wc2mb(wDirPathFile,wcslen(wDirPathFile),szPathAnsci,(int*)&Len);
	_splitpath(szPathAnsci, drive, dir, NULL,NULL);
	sprintf(Path,"%s%s",drive,dir);
	mb2wc(Path,strlen(Path),PbkPATH,(int*)&Len);
	lstrcat(PbkPATH,ROP_L2TP_PBK_PATH);
	//LDebug(DEBUG_L_ERROR, "PBKPATH = %S\n", PbkPATH);

	DWORD dwRet = RasDial(NULL, PbkPATH, &rdParams, 0L, NULL, &m_hRasConn);
	if(dwRet != ERROR_SUCCESS) {
		TCHAR szBuf[256] = {0};
		DWORD dwRetVal = ERROR_SUCCESS;
		dwRetVal = RasGetErrorString((UINT)dwRet,szBuf,256);
		if(dwRetVal == ERROR_SUCCESS) {
			LDebug(DEBUG_L_ERROR, "Error Code %d\n",dwRet);
		}else{
			LDebug(DEBUG_L_ERROR, "RasGetErrorString Failed,LastErr = %ld,RetVal = %d\n", GetLastError(),dwRetVal);
		}
		if (m_hRasConn) {
			RasHangUp(m_hRasConn);
			m_hRasConn = NULL;
		}
		return dwRet;
	}

	LDebug(DEBUG_L_ERROR, "L2TP: DialLoop OK£¡\n");
	m_bConn = TRUE;
	return 0;   
}


/* 
 * Check current link is ROPLink or not
 */
BOOL RopL2TP::IsL2TPLink()
{
    DWORD dwCb = 0, i = 0;
    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwConnections = 0;
    LPRASCONN lpRasConn = NULL;
    RASCONN   RasConnAry[10];
    lpRasConn = RasConnAry;
    BOOL  bRet = FALSE;

    // The first RASCONN structure in the array must contain the RASCONN structure size
    RasConnAry[0].dwSize = 0x53c;   // XP size
    dwCb = sizeof(RASCONN)*10;
    dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);

    if (dwRet == ERROR_SUCCESS) {
        if (dwConnections == 0) {
            LDebug(DEBUG_L_DEBUG, "System No RASConn.\n");
            bRet = FALSE;
            goto out;
        }else{
            LDebug(DEBUG_L_DEBUG, "System using '%d' RASConns.\n", dwConnections);
        }
        for (i = 0; i < dwConnections; i++) {
            LDebug(DEBUG_L_DEBUG, "Checking RASConn='%S'...\n", lpRasConn[i].szEntryName);
            if (IsSelfEntry(&lpRasConn[i])) {
                LDebug(DEBUG_L_DEBUG, "Current Link is 'gendo-l2tp'\n");
                bRet = TRUE;
                goto out;
            }
        }
    }else if (dwRet == ERROR_BUFFER_TOO_SMALL){
        lpRasConn = (LPRASCONN) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
        if (lpRasConn == NULL){
            bRet = FALSE;
            goto out;
        }

        lpRasConn[0].dwSize = 0x53c;
        dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);

        // If successful, print the names of the active connections.
        if (ERROR_SUCCESS == dwRet){
            for (i = 0; i < dwConnections; i++){
                LDebug(DEBUG_L_DEBUG, "Checking RASConn='%S'...\n", lpRasConn[i].szEntryName);
                if (IsSelfEntry(&lpRasConn[i])) {
                    LDebug(DEBUG_L_DEBUG, "Current Link is 'ROP'\n");
                    bRet = TRUE;
                    goto out;
                }
            }
        }
        HeapFree(GetProcessHeap(), 0, lpRasConn);
        lpRasConn = NULL;
    }else{
        LDebug(DEBUG_L_ERROR, "RasEnumConnections fail=%d!", dwRet);
        bRet = FALSE;
    }

out:
    return bRet;
}


int RopL2TP::ReDial(DWORD dwSleep)
{
	LDebug(DEBUG_L_ERROR, "L2TP:ReDial.....\n");
    int iRet = 0;
    HangUpAll(); 

    if (dwSleep != 0) 
        Sleep(dwSleep);
    iRet = DialLoop();
    if (iRet) {
        LDebug(DEBUG_L_ERROR, "Dial failed\n");
        return -1;
    }
    
    return 0;
}

/*
 * return L2TP entry number
 */
int L2TPEnumEntry(void)
{
	DWORD dwCb = 0, i = 0;
	DWORD dwRet = ERROR_SUCCESS;
	DWORD dwConnections = 0;
	LPRASCONN lpRasConn = NULL;
	RASCONN   RasConnAry[10];
	lpRasConn = RasConnAry;

	// The first RASCONN structure in the array must contain the RASCONN structure size
	RasConnAry[0].dwSize = 0x53c;   // XP size
	dwCb = sizeof(RASCONN)*10;
	dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);

	if (dwRet == ERROR_SUCCESS) {
		if (dwConnections == 0) {
			LDebug(DEBUG_L_DEBUG, "L2TPEnumEntry => System No RASConn.\n");
			return 0;
		}
		LDebug(DEBUG_L_DEBUG, "L2TPEnumEntry => System using '%d' RASConns:\n", dwConnections);
		for (i = 0; i < dwConnections; i++){
			LDebug(DEBUG_L_DEBUG, "L2TPEnumEntry => RASConn[%d]='%S'\n", i, lpRasConn[i].szEntryName);
		}
	    return dwConnections;
	}else if (dwRet == ERROR_BUFFER_TOO_SMALL){
		lpRasConn = (LPRASCONN) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
		if (lpRasConn == NULL){
			LDebug(DEBUG_L_ERROR, "HeapAlloc failed!\n");
			return -1;
		}

		lpRasConn[0].dwSize = 0x53c;
		dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);

		if (ERROR_SUCCESS == dwRet){
			for (i = 0; i < dwConnections; i++){
				LDebug(DEBUG_L_DEBUG, "L2TPEnumEntry => RASConn[%d]='%S'...\n", i, lpRasConn[i].szEntryName);
			}
		}
		HeapFree(GetProcessHeap(), 0, lpRasConn);
		return dwConnections;
	}else{
		LDebug(DEBUG_L_ERROR, "L2TPEnumEntry => RasEnumConnections fail=%d!", dwRet);
	}
	return dwConnections;
}

int RopL2TP::CreateL2TPEntry(TCHAR *sName)
{
    DWORD EntryInfoSize = 0;
    DWORD DeviceInfoSize = 0;
    DWORD Ret;
    LPRASENTRY lpRasEntry;
    LPBYTE lpDeviceInfo;

    // Get buffer sizing information for a default phonebook entry
    if ((Ret = RasGetEntryProperties(NULL, sName, NULL, &EntryInfoSize, NULL, &DeviceInfoSize)) != 0)
    {
        if (Ret != ERROR_BUFFER_TOO_SMALL)
        {
            LDebug(DEBUG_L_DEBUG, "RasGetEntryProperties sizing failed with error %d\n", Ret);
            return -1;
        }
    }

    lpRasEntry = (LPRASENTRY) GlobalAlloc(GPTR, EntryInfoSize);

    if (DeviceInfoSize == 0)
        lpDeviceInfo = NULL;
    else
        lpDeviceInfo = (LPBYTE) GlobalAlloc(GPTR, DeviceInfoSize);

    // Get default phonebook entry
    //lpRasEntry->dwSize = sizeof(RASENTRY);
    lpRasEntry->dwSize = EntryInfoSize;

    if ((Ret = RasGetEntryProperties(NULL, sName, lpRasEntry, &EntryInfoSize, lpDeviceInfo, &DeviceInfoSize)) != 0)
    {
        LDebug(DEBUG_L_DEBUG, "RasGetEntryProperties failed with error %d, lasterr=%d\n", Ret, GetLastError());
        return -1;
    }

    // Install a new phonebook entry, "Testentry", using default properties
    if ((Ret = RasSetEntryProperties(ROP_L2TP_PBK_PATH, _T("gendo"), lpRasEntry, EntryInfoSize, lpDeviceInfo, DeviceInfoSize)) != 0)
    {
        LDebug(DEBUG_L_DEBUG,"RasSetEntryProperties failed with error %d\n", Ret);
        return -1;
    }
    return 0;
}

int RopL2TP::Dial(pDialCB pFunc)
{
    DWORD EntryInfoSize = 0;
    DWORD DeviceInfoSize = 0;
    DWORD Ret;
    LPRASENTRY lpRasEntry;
    RASDIALPARAMS RasParam;
    LDebug(DEBUG_L_ERROR, "L2TP: Dial...\n");

    if ((Ret = RasGetEntryProperties(ROP_L2TP_PBK_PATH, m_sEntryName, NULL, &EntryInfoSize, NULL, &DeviceInfoSize)) != 0)
    {
        if (Ret != ERROR_BUFFER_TOO_SMALL)
        {
            LDebug(DEBUG_L_DEBUG, "RasGetEntryProperties sizing failed with error %d\n", Ret);
            return -1;
        }
    }

    lpRasEntry = (LPRASENTRY) GlobalAlloc(GPTR, EntryInfoSize);
    if (!lpRasEntry) return -1;

    lpRasEntry->dwSize = EntryInfoSize;

    if ((Ret = RasGetEntryProperties(ROP_L2TP_PBK_PATH, m_sEntryName, lpRasEntry, &EntryInfoSize, NULL, &DeviceInfoSize)) != 0)
    {
        if (Ret != ERROR_BUFFER_TOO_SMALL)
        {
            LDebug(DEBUG_L_DEBUG, "RasGetEntryProperties sizing failed with error %d\n", Ret);
            return -1;
        }
    }
    /* Set VPN address */
    lstrcpy(lpRasEntry->szLocalPhoneNumber, m_sIp);

    /* Set*/
    memset(&RasParam, 0, sizeof(RASDIALPARAMS));
    RasParam.dwSize = sizeof(RASDIALPARAMS);

    lstrcpy(RasParam.szEntryName,      m_sEntryName);
    lstrcpy(RasParam.szPhoneNumber,    m_sIp);
    lstrcpy(RasParam.szCallbackNumber, _T(""));
    lstrcpy(RasParam.szUserName,       m_sUserName);
    lstrcpy(RasParam.szPassword,       m_sUserPwd);
    lstrcpy(RasParam.szDomain,         _T(""));

    Ret = RasDial(NULL, ROP_L2TP_PBK_PATH, &RasParam, NULL, (LPVOID)pFunc, &m_hRasConn);
    if (Ret != ERROR_SUCCESS) {
        TCHAR sErr[1024] = {0};
        LDebug(DEBUG_L_DEBUG,"RasDial failed with error %d\n", Ret);
        RasGetErrorString(Ret, sErr, 1024);
        Ret = -1;
    }

    if (lpRasEntry) GlobalFree(lpRasEntry);

    return Ret;
}

void RopL2TP::HangUp()
{
    LDebug(DEBUG_L_ERROR, "L2TP: HangUp...\n");
    if (m_hRasConn) {
        RasHangUp(m_hRasConn);
        RASCONNSTATUS RasStatus;
        RasStatus.dwSize = sizeof(RASCONNSTATUS);
        DWORD dwRet = RasGetConnectStatus(m_hRasConn, &RasStatus);
        while(dwRet == ERROR_SUCCESS && RasStatus.dwError != ERROR_INVALID_HANDLE) {
            Sleep(100);
            RasStatus.dwSize = sizeof(RASCONNSTATUS);
            dwRet = RasGetConnectStatus(m_hRasConn, &RasStatus);
        }

        m_hRasConn = NULL;
        m_bConn = false;
        LDebug(DEBUG_L_ERROR, "L2TP: HangUp ROP OK.\n");
    }
}


/* Hangup All RAS connections */
void RopL2TP::HangUpAll()
{
    DWORD dwCb = 0, i = 0;
    DWORD dwRet = ERROR_SUCCESS;
    DWORD dwConnections = 0;
    LPRASCONN lpRasConn = NULL;
    RASCONN   RasConnAry[10];
    lpRasConn = RasConnAry;

    // The first RASCONN structure in the array must contain the RASCONN structure size
    RasConnAry[0].dwSize = 0x53c;   // XP size
    dwCb = sizeof(RASCONN)*10;
    LDebug(DEBUG_L_ERROR, "L2TP: HangUp All\n");
    dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);

    if (dwRet == ERROR_SUCCESS) {
        if (dwConnections == 0) {
            LDebug(DEBUG_L_DEBUG, "System No RASConn.\n");
            return;
        }else{
            LDebug(DEBUG_L_DEBUG, "System using '%d' RASConns.\n", dwConnections);
        }
        for (i = 0; i < dwConnections; i++){
            if (/*FALSE*/!IsSelfEntry(&lpRasConn[i])) {
                LDebug(DEBUG_L_DEBUG, "Skip '%S'...", lpRasConn[i].szEntryName);
                continue;
            }
            LDebug(DEBUG_L_DEBUG, "Hangup GendoL2TP='%S'...\n", lpRasConn[i].szEntryName);
            DWORD  dwRasHangUpRet = RasHangUp(lpRasConn[i].hrasconn); 
            if (dwRasHangUpRet != ERROR_SUCCESS)
            {
                LDebug(DEBUG_L_DEBUG, "RasHangUp Failed ret = %d\n",dwRasHangUpRet);
            }

            RASCONNSTATUS RasStatus;
            RasStatus.dwSize = sizeof(RASCONNSTATUS);
            dwRet = RasGetConnectStatus(lpRasConn[i].hrasconn, &RasStatus);
            while(dwRet == ERROR_SUCCESS && RasStatus.dwError != ERROR_INVALID_HANDLE) {
                Sleep(100);
                RasStatus.dwSize = sizeof(RASCONNSTATUS);
                dwRet = RasGetConnectStatus(lpRasConn[i].hrasconn, &RasStatus);
            }

            LDebug(DEBUG_L_DEBUG, "Hangup RASConn='%S'...OK\n", lpRasConn[i].szEntryName);
        }
    }else if (dwRet == ERROR_BUFFER_TOO_SMALL){
        lpRasConn = (LPRASCONN) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
        if (lpRasConn == NULL){
            LDebug(DEBUG_L_ERROR, "HeapAlloc failed!\n");
            return;
        }

        lpRasConn[0].dwSize = 0x53c;

        dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);

        // If successful, print the names of the active connections.
        if (ERROR_SUCCESS == dwRet){
            for (i = 0; i < dwConnections; i++){
                if (!IsSelfEntry(&lpRasConn[i])) {
                    LDebug(DEBUG_L_DEBUG, "Skip '%S'...", lpRasConn[i].szEntryName);
                    continue;
                }
                LDebug(DEBUG_L_DEBUG, "Hangup RASConn='%S'...\n", lpRasConn[i].szEntryName);
                RasHangUp(lpRasConn[i].hrasconn);
            }
        }
        //Deallocate memory for the connection buffer
        HeapFree(GetProcessHeap(), 0, lpRasConn);
        lpRasConn = NULL;
    }else{
        LDebug(DEBUG_L_ERROR, "RasEnumConnections fail=%d!", dwRet);
    }
}
/*
 * Windows RAS PPTP function wrapper class
 * All right reserved by extelecom, 2012
 * 
 */
#include "stdafx.h"
#include <string>
#include "RopPPTP.h"
#include "RopUtil.h"

using namespace std;

RopPPTP::RopPPTP(void)
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

#define IsSelfEntry(pRasCon)  !lstrcmpi((pRasCon)->szEntryName, ROP_PPTP_ENTRY_NAME)

void WINAPI RopPPTP::RasDialFunc(UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwError)
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

    LDebug(DEBUG_L_ERROR, "PPTP Dial state: %S\n", str);
    /* Notify UI */
}

/* Sync loop for while dialing */
int RopPPTP::DialLoop()
{
    LDebug(DEBUG_L_ERROR, "PPTP: DialLoop...\n");
	if (IsRopLink()) {
		LDebug(DEBUG_L_ERROR, "Has PPTP already\n");
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
	lstrcat(PbkPATH,ROP_PPTP_PBK_PATH);
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

	LDebug(DEBUG_L_ERROR, "PPTP: DialLoop OK£¡\n");
	m_bConn = TRUE;
	return 0;   
}

int RopPPTP::Dial()
{
	LDebug(DEBUG_L_ERROR, "PPTP: Dial......\n");
	if (IsRopLink()) {
		LDebug(DEBUG_L_ERROR, "Has PPTP already\n");
		return 0;
	}

    RASDIALPARAMS rdParams; 
    ZeroMemory(&rdParams, sizeof(RASDIALPARAMS));
    rdParams.dwSize = sizeof(RASDIALPARAMS);
    wcscpy(rdParams.szEntryName, m_sEntryName);
    wcscpy(rdParams.szUserName, m_sUserName);
    wcscpy(rdParams.szPassword, m_sUserPwd);
    m_hRasConn = NULL;

    DWORD dwRet = RasDial(NULL, ROP_PPTP_PBK_PATH, &rdParams, 0L, (RASDIALFUNC)RasDialFunc, &m_hRasConn);
    if(dwRet) {
        TCHAR szBuf[256] = {0};
        if(RasGetErrorString((UINT)dwRet,szBuf,256)==0) {
            LDebug(DEBUG_L_ERROR, "RAS Dial Error=%ld, Info=%S\n", dwRet, szBuf);
        }else{
            LDebug(DEBUG_L_ERROR, "RAS Dial Error=%ld, Info=NULL\n", GetLastError());
        }
        if (m_hRasConn) {
            RasHangUp(m_hRasConn);
            m_hRasConn = NULL;
        }
        return -1;
    }
    /////////////todo:µÈ´ý¡£¡£///////////////////
    Sleep(4000);
    /////////////////////////////
    m_bConn = true;
    return 0;
}

void RopPPTP::HangUp()
{
	LDebug(DEBUG_L_ERROR, "PPTP: HangUp ROP...\n");
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
            LDebug(DEBUG_L_ERROR, "PPTP: HangUp ROP OK.\n");
    }
}


/* 
 * Check current link is ROPLink or not
 */
BOOL RopPPTP::IsRopLink()
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
                LDebug(DEBUG_L_DEBUG, "Current Link is 'ROP'\n");
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

/* Hangup All RAS connections */
void RopPPTP::HangUpAll()
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
    LDebug(DEBUG_L_ERROR, "PPTP: HangUp All\n");
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
            LDebug(DEBUG_L_DEBUG, "Hangup PKVPN='%S'...\n", lpRasConn[i].szEntryName);
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


/*
  re value:
 -1:faild
  0:no PPTP link exist;
  1:RopLink,nothing to do.
  2:3G link,nothing to do.
  3:more than 1 link,nothing to do.
  4:Ok
  return 
  -1:failed
   0:OK
  other:PPTP Error Code
*/
int RopPPTP::SelectivityHangUp(int* ret)
{
	try
	{
		DWORD dwCb = 0, i = 0;
		DWORD dwRet = ERROR_SUCCESS;
		DWORD dwConnections = 0;
		LPRASCONN lpRasConn = NULL;
		RASCONN   RasConnAry[10];
		lpRasConn = RasConnAry;
		RasConnAry[0].dwSize = 0x53c;   // XP size
		dwCb = sizeof(RASCONN)*10;
		dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);
		if (dwRet != ERROR_SUCCESS) {
			LDebug(DEBUG_L_ERROR, "RasEnumConnections fail=%d!", dwRet);
			*ret = 0;
			return -1; 
		}
		if (1 != dwConnections) {
			LDebug(DEBUG_L_ERROR, "RasEnumConnections PPP Cnt=%d!\n", dwConnections);
			*ret = 0;
			return -1; 
		}

		/*if (!lstrcmp(lpRasConn[0].szEntryName, ROP_PPTP_ENTRY_NAME)) {
			LDebug(DEBUG_L_DEBUG, "Current Link is 'ROP'\n");
			*ret = 0; 
			return;
		}*/

		int mlen = 129;
		char tempBuffer[129] = {0};
		wc2mb(lpRasConn[i].szDeviceName,wcslen(lpRasConn[i].szDeviceName),tempBuffer,&mlen);  
		string str(tempBuffer);
		int bi=str.find_last_of("3G");
		int li=str.find_last_of("3g");

		if(bi != -1 || li != -1) {
			LDebug(DEBUG_L_DEBUG, "Current Link is '3G'\n");
			*ret = 0; 
			return -1; 
		}
       
		LDebug(DEBUG_L_ERROR, "Hangup '%S' PPP\n", lpRasConn[0].szEntryName);
		dwRet = RasHangUp(lpRasConn[0].hrasconn);
		if (ERROR_SUCCESS != dwRet) {
            LDebug(DEBUG_L_ERROR, "Hangup '%S' PPP fail=%d\n", lpRasConn[0].szEntryName, dwRet);
		}
		
		DWORD dwDialRet = DialLoop();
		if(dwDialRet == 0) {
			*ret = 0;
			return 0;
		}else {
			*ret = -1;
			return dwDialRet;
		}
	}

	catch (...) {
		LDebug(DEBUG_L_DEBUG, "SelectivityHangUp got exception\n");
		*ret = -1;
		return -1;
	}
}

int RopPPTP::ReDial(DWORD dwSleep)
{
	LDebug(DEBUG_L_ERROR, "PPTP:ReDial.....\n");
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
 * return PPTP entry number
 */
int PPTPEnumEntry(void)
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
			LDebug(DEBUG_L_DEBUG, "PPTPEnumEntry => System No RASConn.\n");
			return 0;
		}
		LDebug(DEBUG_L_DEBUG, "PPTPEnumEntry => System using '%d' RASConns:\n", dwConnections);
		for (i = 0; i < dwConnections; i++){
			LDebug(DEBUG_L_DEBUG, "PPTPEnumEntry => RASConn[%d]='%S'\n", i, lpRasConn[i].szEntryName);
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
				LDebug(DEBUG_L_DEBUG, "PPTPEnumEntry => RASConn[%d]='%S'...\n", i, lpRasConn[i].szEntryName);
			}
		}
		HeapFree(GetProcessHeap(), 0, lpRasConn);
		return dwConnections;
	}else{
		LDebug(DEBUG_L_ERROR, "PPTPEnumEntry => RasEnumConnections fail=%d!", dwRet);
	}
	return dwConnections;
}


int RopPPTP::WaitForHangupAndDial(DWORD dwSleep)
{
	DWORD dwOut = GetTickCount();
	DWORD dwRas = 1;
	int   iRet  = 0;
	while((GetTickCount() - dwOut) < dwSleep && dwRas != 0) {
		LDebug(DEBUG_L_DEBUG, "WaitForHangupAndDial => waiting all PPP hangup...\n");
		dwRas = PPTPEnumEntry();
		if (dwRas == 0) {
			LDebug(DEBUG_L_DEBUG, "WaitForHangupAndDial => No PPP\n");
			break;
		}else{
			LDebug(DEBUG_L_DEBUG, "WaitForHangupAndDial => PPP Conn=%d, wait for 2 seconds...\n", dwRas);
			Sleep(2000);
			continue;
		}		
	}
	if (dwRas != 0) {
		LDebug(DEBUG_L_DEBUG, "WaitForHangupAndDial => timeout, still has PPP Conn=%d, no dialing, user not on the link!!!\n",dwRas);
		return 0;
	}
	Sleep(25000);
	iRet = DialLoop();
	if (iRet) {
		LDebug(DEBUG_L_ERROR, "WaitForHangupAndDial => Dial failed\n");
		return iRet;
	}
	return 0;
}

void RopPPTP::HangUpWithout3G()
{
	try
	{
		DWORD dwCb = 0, i = 0;
		DWORD dwRet = ERROR_SUCCESS;
		DWORD dwConnections = 0;
		LPRASCONN lpRasConn = NULL;
		RASCONN   RasConnAry[10];
		lpRasConn = RasConnAry;
		RasConnAry[0].dwSize = 0x53c;   // XP size
		dwCb = sizeof(RASCONN)*10;
		dwRet = RasEnumConnections(lpRasConn, &dwCb, &dwConnections);
		if (dwRet != ERROR_SUCCESS) {
			LDebug(DEBUG_L_ERROR, "HangUpWithout3G RasEnumConnections fail=%d!", dwRet);
			return; 
		}
		if (1 != dwConnections) {
			LDebug(DEBUG_L_ERROR, "HangUpWithout3G RasEnumConnections PPP Cnt=%d!\n", dwConnections);
			return; 
		}

		int mlen = 129;
		char tempBuffer[129] = {0};
		wc2mb(lpRasConn[i].szDeviceName,wcslen(lpRasConn[i].szDeviceName),tempBuffer,&mlen);  
		string str(tempBuffer);
		int bi=str.find_last_of("3G");
		int li=str.find_last_of("3g");

		if(bi != -1 || li != -1) {
			LDebug(DEBUG_L_DEBUG, "Current Link is '3G'\n");
			return;
		}
       
		HangUpAll();
	}

	catch (...) {
		LDebug(DEBUG_L_DEBUG, "HangUpWithout3G got exception\n");
	}
}

int RopPPTP::EnumRasEntry()
{
	DWORD dwCb = 0;
	DWORD dwRet = ERROR_SUCCESS;
	DWORD dwEntries = 0;
	LPRASENTRYNAME lpRasEntryName = NULL;

	// Call RasEnumEntries with lpRasEntryName = NULL. dwCb is returned with the required buffer size and 
	// a return code of ERROR_BUFFER_TOO_SMALL
	dwRet = RasEnumEntries(NULL, NULL, lpRasEntryName, &dwCb, &dwEntries);

	if (dwRet == ERROR_BUFFER_TOO_SMALL){
		// Allocate the memory needed for the array of RAS entry names.
		lpRasEntryName = (LPRASENTRYNAME) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwCb);
		if (lpRasEntryName == NULL){
			LDebug(DEBUG_L_DEBUG, "HeapAlloc failed!\n");
			return 0;
		}
		// The first RASENTRYNAME structure in the array must contain the structure size
		lpRasEntryName[0].dwSize = sizeof(RASENTRYNAME);

		// Call RasEnumEntries to enumerate all RAS entry names
		dwRet = RasEnumEntries(NULL, NULL, lpRasEntryName, &dwCb, &dwEntries);

		// If successful, print the RAS entry names 
		if (ERROR_SUCCESS == dwRet){
			for (DWORD i = 0; i < dwEntries; i++) {
				if (IsPPTPRas(&lpRasEntryName[i])) {
					CreateRopLinkEntry(&lpRasEntryName[i]);
				}
			}
		}

		//Deallocate memory for the connection buffer
		HeapFree(GetProcessHeap(), 0, lpRasEntryName);
		lpRasEntryName = NULL;
		return 0;
	}

	// There was either a problem with RAS or there are RAS entry names to enumerate    
	if(dwEntries >= 1){
		LDebug(DEBUG_L_DEBUG, "The operation failed to acquire the buffer size.\n");
	}else{
		LDebug(DEBUG_L_DEBUG, "There were no RAS entry names found:.\n");
	}

	return 0;
}

BOOL RopPPTP::IsPPTPRas( LPRASENTRYNAME rasEntry )
{
	DWORD EntryInfoSize = 0;
	DWORD DeviceInfoSize = 0;
	DWORD Ret;
	LPRASENTRY lpRasEntry;
	LPBYTE lpDeviceInfo;
	string temp;

	// Get buffer sizing information for a default phonebook entry
	if ((Ret = RasGetEntryProperties(rasEntry->szPhonebookPath, rasEntry->szEntryName, NULL, &EntryInfoSize, NULL, &DeviceInfoSize)) != 0)
	{
		if (Ret != ERROR_BUFFER_TOO_SMALL)
		{
			LDebug(DEBUG_L_DEBUG, "RasGetEntryProperties sizing failed with error %d\n");
			return FALSE;
		}
	}

	lpRasEntry = (LPRASENTRY)GlobalAlloc(GPTR, EntryInfoSize);

	if (DeviceInfoSize == 0)
		lpDeviceInfo = NULL;
	else
		lpDeviceInfo = (LPBYTE)GlobalAlloc(GPTR, DeviceInfoSize);

	lpRasEntry->dwSize = sizeof(RASENTRY);

	if ((Ret = RasGetEntryProperties(rasEntry->szPhonebookPath, rasEntry->szEntryName, lpRasEntry, &EntryInfoSize, lpDeviceInfo, &DeviceInfoSize)) != 0)
	{
		LDebug(DEBUG_L_DEBUG, "RasGetEntryProperties failed with error %d\n", Ret);

		Ret = FALSE;
		goto out;
	}

	LDebug(DEBUG_L_DEBUG,"Enum Entry=%s, DevType=%s, DwType=%d, PhoneNumber=%s\n", rasEntry->szEntryName, lpRasEntry->szDeviceType, lpRasEntry->dwType, lpRasEntry->szLocalPhoneNumber);	
	//temp.Format(_T("Enum Entry=%s, DevType=%s, DwType=%d, PhoneNumber=%s"), rasEntry->szEntryName, lpRasEntry->szDeviceType, lpRasEntry->dwType, lpRasEntry->szLocalPhoneNumber);

	if (!lstrcmpi(lpRasEntry->szDeviceType, RASDT_Vpn) && !lstrcmpi(lpRasEntry->szLocalPhoneNumber, m_sIp)) {
		Ret = TRUE;
	}

out:
	if (lpRasEntry)   GlobalFree(lpRasEntry);
	if (lpDeviceInfo) GlobalFree(lpDeviceInfo);

	return Ret;
}

int RopPPTP::CreateRopLinkEntry( LPRASENTRYNAME rasEntry )
{
	DWORD EntryInfoSize = 0;
	DWORD DeviceInfoSize = 0;
	DWORD Ret;
	LPRASENTRY lpRasEntry;
	LPBYTE lpDeviceInfo;

	// Get buffer sizing information for a default phonebook entry
	if ((Ret = RasGetEntryProperties(rasEntry->szPhonebookPath, rasEntry->szEntryName, NULL, &EntryInfoSize, NULL, &DeviceInfoSize)) != 0)
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
	lpRasEntry->dwSize = sizeof(RASENTRY);

	if ((Ret = RasGetEntryProperties(rasEntry->szPhonebookPath, rasEntry->szEntryName, lpRasEntry, &EntryInfoSize, lpDeviceInfo, &DeviceInfoSize)) != 0)
	{
		LDebug(DEBUG_L_DEBUG, "RasGetEntryProperties failed with error %d\n", Ret);
		return -1;
	}

	// Validate new phonebook name "Testentry"
	if ((Ret = RasValidateEntryName(ROP_PPTP_PBK_PATH, _T("RopPPTP"))) != ERROR_SUCCESS)
	{
		LDebug(DEBUG_L_DEBUG,"RasValidateEntryName failed with error %d\n", Ret);
	}

	// Install a new phonebook entry, "Testentry", using default properties
	if ((Ret = RasSetEntryProperties(ROP_PPTP_PBK_PATH, _T("RopPPTP"), lpRasEntry, EntryInfoSize, lpDeviceInfo, DeviceInfoSize)) != 0)
	{
		LDebug(DEBUG_L_DEBUG,"RasSetEntryProperties failed with error %d\n", Ret);
		return -1;
	}
	return 0;
}


int RopPPTP::CreatePPTPEntry(TCHAR *sName)
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
    if ((Ret = RasSetEntryProperties(ROP_PPTP_PBK_PATH, _T("gendo"), lpRasEntry, EntryInfoSize, lpDeviceInfo, DeviceInfoSize)) != 0)
    {
        LDebug(DEBUG_L_DEBUG,"RasSetEntryProperties failed with error %d\n", Ret);
        return -1;
    }
    return 0;
}

int RopPPTP::DialPPTP(pDialCB pFunc)
{
    DWORD EntryInfoSize = 0;
    DWORD DeviceInfoSize = 0;
    DWORD Ret;
    LPRASENTRY lpRasEntry;
    RASDIALPARAMS RasParam;

    if ((Ret = RasGetEntryProperties(ROP_PPTP_PBK_PATH, m_sEntryName, NULL, &EntryInfoSize, NULL, &DeviceInfoSize)) != 0)
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

    if ((Ret = RasGetEntryProperties(ROP_PPTP_PBK_PATH, m_sEntryName, lpRasEntry, &EntryInfoSize, NULL, &DeviceInfoSize)) != 0)
    {
        if (Ret != ERROR_BUFFER_TOO_SMALL)
        {
            LDebug(DEBUG_L_DEBUG, "RasGetEntryProperties sizing failed with error %d\n", Ret);
            return -1;
        }
    }
    /* Set VPN address */
    lstrcpy(lpRasEntry->szLocalPhoneNumber, m_sIp);

    memset(&RasParam, 0, sizeof(RASDIALPARAMS));
    RasParam.dwSize = sizeof(RASDIALPARAMS);

    lstrcpy(RasParam.szEntryName,      m_sEntryName);
    lstrcpy(RasParam.szPhoneNumber,    m_sIp);
    lstrcpy(RasParam.szCallbackNumber, _T(""));
    lstrcpy(RasParam.szUserName,       m_sUserName);
    lstrcpy(RasParam.szPassword,       m_sUserPwd);
    lstrcpy(RasParam.szDomain,         _T(""));

    Ret = RasDial(NULL, ROP_PPTP_PBK_PATH, &RasParam, NULL, (LPVOID)pFunc, &m_hRasConn);
    if (Ret != ERROR_SUCCESS) {
        TCHAR sErr[1024] = {0};
        LDebug(DEBUG_L_DEBUG,"RasDial failed with error %d\n", Ret);
        RasGetErrorString(Ret, sErr, 1024);
        Ret = -1;
    }

    if (lpRasEntry) GlobalFree(lpRasEntry);

    return Ret;
}
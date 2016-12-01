#include "stdafx.h"
#include <stdio.h>
#include <io.h>
#include <WinInet.h>
#include <Urlmon.h>
#include "RopUtil.h"
#include "RopPing.h"
#include "lib-msg.h"
#include "RopFileUtil.h"
#include "RopDownloadCB.h"

int RopSetFileSecurity(unsigned char* szFileName)
{
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    PSID pInteractiveSid = NULL;
    PSID pAdministratorsSid = NULL;
	PSID pUserSid = NULL;
    SECURITY_DESCRIPTOR sd;
    PACL pDacl = NULL;
    DWORD dwAclSize;

    LONG lRetCode;
    BOOL bSuccess = FALSE;

    if (!AllocateAndInitializeSid(&sia,1, SECURITY_INTERACTIVE_RID,
        0, 0, 0, 0, 0, 0, 0, &pInteractiveSid )) {
            goto cleanup;
    }

    if (!AllocateAndInitializeSid( &sia, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS,0, 0, 0, 0, 0, 0, &pAdministratorsSid)) {
            goto cleanup;
    }

	if (!AllocateAndInitializeSid( &sia, 2, SECURITY_IUSER_RID,
        DOMAIN_GROUP_RID_USERS,0, 0, 0, 0, 0, 0, &pUserSid)) {
            goto cleanup;
    }
	
    dwAclSize = sizeof(ACL) +
        2 * ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
        GetLengthSid(pInteractiveSid) +
        GetLengthSid(pAdministratorsSid) ;

    pDacl = (PACL)HeapAlloc(GetProcessHeap(), 0, dwAclSize);
    if (pDacl == NULL) goto cleanup;

    if (!InitializeAcl(pDacl, dwAclSize, ACL_REVISION)) {
        goto cleanup;
    }

#define ALL_ACCESS 0xffffffff
    if (!AddAccessAllowedAce( pDacl, ACL_REVISION, ALL_ACCESS, pInteractiveSid )) {
        goto cleanup;
    }

    if (!AddAccessAllowedAce( pDacl, ACL_REVISION, ALL_ACCESS, pAdministratorsSid )) {
        goto cleanup;
    }
	if (!AddAccessAllowedAce( pDacl, ACL_REVISION, ALL_ACCESS, pUserSid )) {
        goto cleanup;
    }

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        goto cleanup;
    }

    if (!SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE)) {
        goto cleanup;
    }

    lRetCode = SetFileSecurityA((char*)szFileName, (SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, &sd);

    if (lRetCode != ERROR_SUCCESS) {

        goto cleanup;
    }

    bSuccess = TRUE; // indicate success

cleanup:
    if (pDacl != NULL)
        HeapFree(GetProcessHeap(), 0, pDacl);

    if (pInteractiveSid != NULL)
        FreeSid(pInteractiveSid);

    if (pAdministratorsSid != NULL)
        FreeSid(pAdministratorsSid);

	if (pUserSid != NULL)
        FreeSid(pUserSid);

    if (bSuccess) {
        return 0;
    } else {
        //LDebug(DEBUG_L_ERROR,"ERROR updating performance data security\n");
        return -1;
    }
}

BOOL IsAdmin()
{
    HANDLE hAccessToken;
    PTOKEN_GROUPS ptgGroups;
    DWORD dwInfoBufferSize;
    PSID psidAdministrators;
    int x;
    BOOL bSuccess;
    BOOL Result = FALSE;

    bSuccess = OpenThreadToken(GetCurrentThread(),TOKEN_QUERY,TRUE,&hAccessToken);
    if (!bSuccess) {
        if (GetLastError() == ERROR_NO_TOKEN) {
            bSuccess = OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&hAccessToken);
        }
    }
    if (bSuccess) {
        char* pTemp = new char[1024];
        ptgGroups = (PTOKEN_GROUPS)pTemp;
        bSuccess = GetTokenInformation(hAccessToken,TokenGroups,ptgGroups,1024,&dwInfoBufferSize);
        CloseHandle(hAccessToken);
        if (bSuccess) {
            SID_IDENTIFIER_AUTHORITY Authority = {SECURITY_NT_AUTHORITY};
            AllocateAndInitializeSid(\
                                    &Authority,\
                                    2, \
                                    SECURITY_BUILTIN_DOMAIN_RID,   \
                                    DOMAIN_ALIAS_RID_ADMINS,  \
                                    0, \
                                    0,  \
                                    0,   \
                                    0,   \
                                    0,   \
                                    0,   \
                                    &psidAdministrators
                                    );

            for (x=0;x<=(int)ptgGroups->GroupCount-1;x++) {
                if (EqualSid(psidAdministrators,ptgGroups->Groups[x].Sid)) {
                    Result=TRUE;
                    break;
                }
            }
            FreeSid(psidAdministrators);
        }
        delete []pTemp;
    }
    return Result;
}



BOOL GetFileVersion(char* FileName,char* Version)
{
    DWORD dwDummyHandle;
    int len = GetFileVersionInfoSizeA(FileName, &dwDummyHandle);
    if (len <= 0)
        return FALSE;

    unsigned char* pVersionInfo = new unsigned char[len];
    if (!GetFileVersionInfoA(FileName, 0, len, pVersionInfo)) {
        if (pVersionInfo)delete []pVersionInfo;
        return FALSE;
    }

    LPVOID lpvi;
    UINT iLen;
    if (!VerQueryValue(pVersionInfo, TEXT("\\"), &lpvi, &iLen)) {
        if (pVersionInfo)delete []pVersionInfo;
        return FALSE;
    }
    *(VS_FIXEDFILEINFO*)pVersionInfo = *(VS_FIXEDFILEINFO*)lpvi;

    VS_FIXEDFILEINFO* pInfo = (VS_FIXEDFILEINFO*)pVersionInfo;
    if (pInfo) {

        int n1 = ((pInfo->dwFileVersionMS >> 16)&0xffff);
        int n2 = ((pInfo->dwFileVersionMS )&0xffff);

        int n3 = ((pInfo->dwFileVersionLS >> 16)&0xffff);
        int n4 = ((pInfo->dwFileVersionLS )&0xffff);

        sprintf(Version,"%d.%d.%d.%d",n1,n2,n3,n4);
        if (pVersionInfo)delete []pVersionInfo;
        return TRUE;
    }

    if (pVersionInfo)delete []pVersionInfo;
    return FALSE;
}


int  HttpDownloadFile(char *sIP, int iPort, char *sSrcFile, char *sDstFile, PVOID pCtx, char *sShowName)
{
    char sAddr[MAX_NAME_LEN] = {0};
    int  iLen = MAX_NAME_LEN, ret = 0;
    char URL[4096] = {0};
    char URL_DEL_OLD[4096] = {0};
    char URL_DEL[4096] = {0};
    TCHAR sFile[64] = {0};
    
    
    //GetHostByName((char*)sIP, (char*)sAddr);  // No DNS resolving
    strncpy(sAddr, sIP, MAX_NAME_LEN);
    sprintf(URL,"http://%s:%d/%s",sAddr,iPort,sSrcFile);
    sprintf(URL_DEL_OLD,"http://%s:%d/%s",sAddr,iPort,sSrcFile);
    sprintf(URL_DEL,"http://%s:%d/%s",sAddr,iPort,sSrcFile);


    DeleteUrlCacheEntryA(URL_DEL_OLD);
    DeleteUrlCacheEntryA(URL_DEL);

	/* Load UrlDownloadToFile dll and address to 
	 * Make Avira happy. It complaints about Downloader trojoy
	 */
	HANDLE hUrl = INVALID_HANDLE_VALUE;
	hUrl = LoadLibraryA("urlmon.dll");
	if (!hUrl) {
		LDebug(DEBUG_L_ERROR,"Cannot Loadlib urlmon\n");
		return -3;
	}
	typedef HRESULT (_stdcall*pfnUrlDwToFile)(LPUNKNOWN,LPCSTR,LPCSTR,DWORD,LPBINDSTATUSCALLBACK);

	pfnUrlDwToFile pfnUrl = NULL;
	pfnUrl = (pfnUrlDwToFile)GetProcAddress((HMODULE)hUrl, "URLDownloadToFileA");
	if (!pfnUrl) {
		LDebug(DEBUG_L_ERROR,"Cannot get UrlDwToFileA pointer!\n");
		return -3;
	}
	iLen = 64;
    mb2wc(sShowName, strlen(sShowName), sFile, &iLen);

	CDownloadCallback callback(sFile, pCtx);
	//LDebug(DEBUG_L_ERROR,"DownLoading '%s' to '%s'...", URL, sDstFile);
	if (S_OK == pfnUrl(NULL,URL,sDstFile,0,&callback)) {
    //if (S_OK == URLDownloadToFileA(NULL,URL,sDstFile,0,&callback)) {
		//LDebug(DEBUG_L_ERROR,"DownLoad '%s' to '%s' OK", URL, sDstFile);
        long length = 0;
        FILE* fp=fopen(sDstFile,"rb");

		if (fp) {
            fseek(fp,SEEK_END,0);
            length=ftell(fp);
            fclose(fp);
        }
        //RopSetFileSecurity((unsigned char*)sDstFile);
        DeleteUrlCacheEntryA(URL_DEL_OLD);
        DeleteUrlCacheEntryA(URL_DEL);
        ret = length;
    } else {
        ret = -2;
        LDebug(DEBUG_L_ERROR,"download %s by URLDownloadToFile Fail!",URL);
    }

	FreeLibrary((HMODULE)hUrl);
    return ret;
}
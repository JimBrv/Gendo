// RopUpdate.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "RopUpdate.h"
#include "RopPing.h"
#include "RopUtil.h"
#include "RopCfgFile.h"
#include "RopFileUtil.h"
#include <ctime>


// 这是导出变量的一个示例
ROPUPDATE_API int nRopUpdate=0;

// 这是导出函数的一个示例。
ROPUPDATE_API int fnRopUpdate(void)
{
	return 42;
}

static RopCfgFile *pCfgCurrent = NULL;
static RopCfgFile *pCfgNew = NULL;

static PVOID  pOnUIProgressCtx = NULL;

ROPUPDATE_API BOOL IsServerAvaliable(void)
{
    char sAddr[MAX_NAME_LEN] = {0};
    int  iLen = MAX_NAME_LEN;
    int  iConn = -1;
    WCHAR wcAddr[MAX_NAME_LEN] = {0};
    if (pCfgCurrent) {
        pCfgCurrent->GetServerAddr(sAddr, iLen);
        mb2wc(sAddr, strlen(sAddr), wcAddr, &iLen);
        iConn = 0/*Ping(wcAddr)*/;
    }
    return (iConn==0 ? true:false);
}

ROPUPDATE_API BOOL IsNeedUpdate()
{
    BOOL bFlag = false;
    char sAddr[MAX_NAME_LEN] = {0};
    char sLocalFile[MAX_NAME_LEN]  = {0};
    char sRemoteFile[MAX_NAME_LEN] = {0};
    int  iLen = MAX_NAME_LEN;
    int  iPort = -1;
    int  iRet = 0;
    if (!pCfgCurrent) {
        UpErr("Current version is NULL!\n");
        return false;
    }
    pCfgCurrent->GetServerDownloadAddr(sAddr, iLen);
    iPort = pCfgCurrent->GetServerDownloadPort();
    
    iLen = MAX_NAME_LEN;
    pCfgCurrent->GetServerDownloadPath(sRemoteFile, iLen);
    UpDbg("Update by %s\n", sAddr);
    strcat(sRemoteFile, CFG_NAME);
    sprintf(sLocalFile, "%s", LOCAL_CFG_PATH);
    strcat(sLocalFile, TEMP_CFG_NAME);
	UpDbg("RemoteFile:%s\r\nsLocalFile:%s\r\n",sRemoteFile,sLocalFile);
    if (pCfgCurrent->m_pUiCB) {
        pCfgCurrent->m_pUiCB(0, PHASE_DOWNLOAD_COMPONENT, "Download configure file...");
    }

	srand(unsigned(time(0)));
	sprintf(sRemoteFile,"%s?id=%d",sRemoteFile,rand());
    iRet = HttpDownloadFile(sAddr, iPort, sRemoteFile, sLocalFile, NULL, CFG_NAME);
    if (iRet <= 0) {
        UpErr("Current New version error!\n");
        return false;  
    }

    pCfgNew = new RopCfgFile(LOCAL_CFG_PATH, TEMP_CFG_NAME, pCfgCurrent->m_pUiCB, pOnUIProgressCtx);
    if (!pCfgNew) {
        UpErr("Create New version error!\n");
        return false;  
    }

    if (false == pCfgNew->OpenFile()) {
        UpErr("Open New version file fail\n");
        return false;
    }
    
    pCfgNew->m_pUiCB = pCfgCurrent->m_pUiCB;
    iRet = pCfgNew->ReadAndInit();
    if (iRet) {
        UpErr("Parse New version file got error!\n");
        return false;
    }
    
    //pCfgCurrent->CalcMD5();
    if (pCfgCurrent->m_pUiCB) {
        pCfgCurrent->m_pUiCB(0, PHASE_CHECK_VERSION, "Check Client Version now...");
    }

    bFlag = pCfgNew->CompareFile(pCfgCurrent);

    return bFlag;
}

ROPUPDATE_API BOOL Update()
{
    int iRet = 0;
    if (!pCfgNew) {
        UpErr("Update() => New CFG is NULL!\n");
        return false;  
    }
    if (pCfgNew->m_pUiCB) {
        pCfgNew->m_pUiCB(0, PHASE_CHECK_VERSION, "Updating new version now...");
    }

    iRet = pCfgNew->Update();
    if (pCfgNew->m_pUiCB) {
        pCfgNew->m_pUiCB(0, PHASE_CHECK_VERSION, "Updating new version OK...");
    }
    /* Update CFG file */
    if (pCfgCurrent) {
        delete pCfgCurrent;
        pCfgCurrent = NULL;
    }
    if (pCfgNew) {
        delete pCfgNew;
        pCfgNew = NULL;
    }
	if (!iRet){
		UpErr("Replace CFG NOW!\n");
		::DeleteFileA(CFG_NAME);
		if (!MoveFileExA(TEMP_CFG_NAME, CFG_NAME, MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED)) {
			if (ERROR_ACCESS_DENIED == GetLastError()) {
				char DestFile_[128] = {0};
				SYSTEMTIME t;
				GetSystemTime(&t);
				sprintf(DestFile_,"%s_%d%d%d%d%d",CFG_NAME, t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
				MoveFileExA(CFG_NAME,DestFile_,MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED);
				MoveFileExA(DestFile_,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);

				if (!MoveFileExA(TEMP_CFG_NAME, CFG_NAME, MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED)) {
					LDebug(DEBUG_L_ERROR,"%s need reboot %d",CFG_NAME, GetLastError());
					MoveFileExA(TEMP_CFG_NAME,CFG_NAME,MOVEFILE_REPLACE_EXISTING|MOVEFILE_DELAY_UNTIL_REBOOT);
				}
			}
		}
    }

    return iRet ? false: true;
}


/* Must call this after determined need update */
ROPUPDATE_API INT UpdateNotice(TCHAR *pMsg, INT row, INT col_size)
{
    int len2 = col_size;
    if (!pCfgNew) return -1;
    if (strlen(pCfgNew->m_UpdateMsg1) > 0) {
        mb2wc(pCfgNew->m_UpdateMsg1, strlen(pCfgNew->m_UpdateMsg1), pMsg, &len2);
    }
    if (strlen(pCfgNew->m_UpdateMsg2) > 0) {
        len2 = col_size;
        mb2wc(pCfgNew->m_UpdateMsg2, strlen(pCfgNew->m_UpdateMsg2), pMsg+col_size*1, &len2);
    }
    
    if (strlen(pCfgNew->m_UpdateMsg3) > 0) {
        len2 = col_size;
        mb2wc(pCfgNew->m_UpdateMsg3, strlen(pCfgNew->m_UpdateMsg3), pMsg+col_size*2, &len2);
    }
    if (strlen(pCfgNew->m_UpdateMsg4) > 0) {
        len2 = col_size;
        mb2wc(pCfgNew->m_UpdateMsg4, strlen(pCfgNew->m_UpdateMsg4), pMsg+col_size*3, &len2);
    }
    if (strlen(pCfgNew->m_UpdateMsg5) > 0) {
        len2 = col_size;
        mb2wc(pCfgNew->m_UpdateMsg5, strlen(pCfgNew->m_UpdateMsg5), pMsg+col_size*4, &len2);
    }
    return 0;
}

ROPUPDATE_API int Init(PUiPhaseStepCB pUiStepCB, PVOID pOnProgressCtx)
{
    int iRet = 0;
    try {
        WORD wVersionRequested;
        WSADATA wsaData;
        wVersionRequested = MAKEWORD(2, 2);

        if (WSAStartup( wVersionRequested, &wsaData )) {
            UpErr("RopExecuter Couldn't startup Winsock\n");
            return -1;
        }
        pOnUIProgressCtx = pOnProgressCtx;
        pCfgCurrent = new RopCfgFile("", CFG_NAME, pUiStepCB, pOnProgressCtx);
        if (false == pCfgCurrent->OpenFile()) {
            UpErr("Init open CFG file fail\n");
            return -1;
        }
        iRet = pCfgCurrent->ReadAndInit();
        if (iRet) {
            UpErr("Read/Parse CFG file got error!\n");
            return -2;
        }
        UpDbg("Init with CFG file OK!\n");
        return 0;
    }catch(...) {
        UpErr("Init got error\n");
    }
    return 0;
}

ROPUPDATE_API void Finl(void)
{
    if (pCfgCurrent) {
        delete pCfgCurrent;
        pCfgCurrent = NULL;
    }
    if (pCfgNew) {
        delete pCfgNew;
        pCfgNew = NULL;
    }
    return;
}

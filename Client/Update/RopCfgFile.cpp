/*
 * Update procedure:  Read Cfg File->Get Server IP->Get New Cfg from Server->
 *                    Parse Cfg->Check 2 Cfg items->Update each item
 * if update is true, each component(item) will be downloaded.
 * Simple 
 */
#include "stdafx.h"
#include <stdio.h>
#include <Shlwapi.h>
#include <assert.h>
#include "RopUtil.h"
#include "RopRegistry.h"
#include "RopAsyncSocket.h"

#include "RopCfgFile.h"
#include "RopCfgItem.h"

BOOL DownLoadCfgFile(char *szAddr, int iPort, char *szToLocalFile)
{
    char sLocalPath[128] = {0};
    char sRemotePath[128] = {0};
    int  iRet = 0;
    sprintf(sLocalPath, ".\\%s", szToLocalFile);
    sprintf(sRemotePath, "%s%s", SERVER_CFG_PATH, CFG_NAME);
    iRet = HttpDownloadFile(szAddr, iPort, sRemotePath, sLocalPath, NULL, CFG_NAME);
    if (iRet <= 0) {
        UpErr("Download New CFG file fail, update stopped!\n");
        return FALSE;
    }
    return TRUE;
}


RopCfgFile::RopCfgFile(char *szFilePath, char *szFileName, PUiPhaseStepCB pCB, PVOID pOnUpdateCtx)
{
    strcpy(m_FileName, szFileName);
    strcpy(m_LocalPath, szFilePath);
    ZeroMemory(m_OEMCompany, sizeof(m_OEMCompany));
    ZeroMemory(m_OEMProduct, sizeof(m_OEMProduct));
    ZeroMemory(m_OEMProgmenu, sizeof(m_OEMProgmenu));
    ZeroMemory(m_ServerAddr, sizeof(m_ServerAddr));
    ZeroMemory(m_ServerDownloadPath, sizeof(m_ServerDownloadPath));
    ZeroMemory(m_UpdateMsg1, sizeof(m_UpdateMsg1));
    ZeroMemory(m_UpdateMsg2, sizeof(m_UpdateMsg2));
    ZeroMemory(m_UpdateMsg3, sizeof(m_UpdateMsg3));
    ZeroMemory(m_UpdateMsg4, sizeof(m_UpdateMsg4));
    ZeroMemory(m_UpdateMsg5, sizeof(m_UpdateMsg5));
    m_ServerDownloadPort = 0;
    m_ServerServicePort = 0;
    m_pUiCB = pCB;
    m_bNeedUpdate = false;
    m_pOnUpdateCtx= pOnUpdateCtx;
}

RopCfgFile::~RopCfgFile(void)
{
    vector<RopCfgItem*>::iterator iter;
    for (iter = m_vItem.begin(); iter != m_vItem.end(); iter++) {
        RopCfgItem* pItem = *iter;
        if (pItem) {
            delete pItem;
        }
    } 
    m_vItem.erase(m_vItem.begin(), m_vItem.end());
    if (m_fp) fclose(m_fp);
}

BOOL RopCfgFile::OpenFile()
{
    char sCfgFile[128] = {0};
    sprintf(sCfgFile, "%s%s", m_LocalPath, m_FileName);
    m_fp = fopen(sCfgFile, "r");
    if (!m_fp) {
        UpErr("Open file '%s' fail!\n", sCfgFile);
        return false;
    }
    return true;
}

int RopCfgFile::Update()
{
    int iRet = 0;
	bool bUpdateSucc = true;
	vector<RopCfgItem*> updateItems;
    vector<RopCfgItem*>::iterator iter;
    for (iter = m_vItem.begin(); iter != m_vItem.end(); iter++) {
        RopCfgItem* pItem = *iter;
        if (pItem) {
            if (pItem->IsNeedUpdate()) {
				updateItems.push_back(pItem);
                iRet = pItem->Update(); 
                if (iRet) {
					bUpdateSucc = false;
                    LDebug(DEBUG_L_ERROR, "update item='%s' fail!!!\n", pItem->m_Name);                        
                    break;
                }
            }
        }
    }
	if (bUpdateSucc){
		vector<RopCfgItem*>::iterator  updateIter;
		for (updateIter = updateItems.begin();updateIter != updateItems.end();updateIter++){
			RopCfgItem* pIterm = * updateIter;
			pIterm->CopyFile();
		}
		updateItems.clear();
	}

    return iRet;
}

int  RopCfgFile::SetParam(char *szName,char *szValue)
{
    if (!szName)return -1;
    if (!szValue)return -1;

    StrTrimA(szName,("\t\r\n"));
    StrTrimA(szValue,("\t\r\n"));

    if (StrCmpNIA(szName,"OEMCompany",strlen(szName))==0) {
        StrCpyNA(m_OEMCompany, szValue, MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"OEMProduct",strlen(szName))==0) {
        StrCpyNA(m_OEMProduct,szValue,MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"OEMProgmenu",strlen(szName))==0) {
        StrCpyNA(m_OEMProgmenu,szValue,MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"Downloadmask",strlen(szName))==0) {
        m_Downloadmask = atoi(szValue);
    } else  if (StrCmpNIA(szName,"ClientVersion",strlen(szName))==0) {
        StrCpyNA(m_ClientVersion,szValue,MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"HttpServerAddr",strlen(szName))==0) {
        StrCpyNA(m_HttpServerAddr,szValue,MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"HttpServerDownloadPort",strlen(szName))==0) {
        m_ServerDownloadPort = atoi(szValue);
    }else if (StrCmpNIA(szName,"ServerAddr",strlen(szName))==0) {
        StrCpyNA(m_ServerAddr,szValue,MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"ServerServicePort",strlen(szName))==0) {
        m_ServerServicePort = atoi(szValue);
    } else if (StrCmpNIA(szName,"ServerDownloadPath",strlen(szName))==0) {
        StrCpyNA(m_ServerDownloadPath,szValue,MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"SleepBeforeDial",strlen(szName))==0) {
        m_dwDialSleep = atoi(szValue);
    } else if (StrCmpNIA(szName, "UpdateMsg1", strlen(szName))==0) {
        StrCpyNA(m_UpdateMsg1, szValue, MAX_NAME_LEN*10);
    } else if (StrCmpNIA(szName, "UpdateMsg2", strlen(szName))==0) {
        StrCpyNA(m_UpdateMsg2, szValue, MAX_NAME_LEN*10);
    } else if (StrCmpNIA(szName, "UpdateMsg3", strlen(szName))==0) {
        StrCpyNA(m_UpdateMsg3, szValue, MAX_NAME_LEN*10);
    } else if (StrCmpNIA(szName, "UpdateMsg4", strlen(szName))==0) {
        StrCpyNA(m_UpdateMsg4, szValue, MAX_NAME_LEN*10);
    } else if (StrCmpNIA(szName, "UpdateMsg5", strlen(szName))==0) {
        StrCpyNA(m_UpdateMsg5, szValue, MAX_NAME_LEN*10);
    } else {
        return -1;
    }
    return 0;
}

int  RopCfgFile::ReadAndInit()
{
    char Line[512] = {0};
    char Name[128];
    char Value[128];

    while (ReadLine(Line,512)) {
        int iLineType = ParseLine(Line,Name,Value);
        switch (iLineType) {
            case 1: 
                {
                    //NULL line,ignore it
                }
                break;
            case 2: 
                {    
                    //end
                    //LDebug(DEBUG_L_ERROR,"end\n");
                    return 0;
                }
                break;
            case 3: 
                {
                    //global parameters.
                    SetParam(Name,Value);
                }
                break;
            case 4: 
                // Item parameters
                {
                    if (StrCmpNIA(Value, CFG_ITEM_KEY,strlen(Value))==0) {
                        RopCfgItem* pItem = new RopCfgItem(m_fp, Name, this, m_pOnUpdateCtx);
                        if (pItem) {
                            pItem->SetPath(m_OEMCompany,m_OEMProduct,m_LocalPath,m_OEMProgmenu);
                            pItem->ReadAndInit();
                            m_vItem.push_back(pItem);
                        }
                    }
                }
                break;
            default:
                return -2;
        }
    }
    return 0;
}

int  RopCfgFile::CalcTime(int iOpt)
{
    int iTime = 0;
    vector<RopCfgItem *>::iterator iter;
    for (iter = m_vItem.begin(); iter != m_vItem.end(); iter++) {
        RopCfgItem* pItem = *iter;
        if (pItem) {
            iTime += pItem->CalcTime(iOpt);
        }
    }
    return iTime;
}

void RopCfgFile::CalcMD5()
{
    vector<RopCfgItem *>::iterator iter;
    for (iter = m_vItem.begin(); iter != m_vItem.end(); iter++) {
        RopCfgItem* pItem = *iter;
        if (pItem) {   
            pItem->CalcMD5();
        }
    }
}

void RopCfgFile::SaveMD5()
{
    vector<RopCfgItem *>::iterator iter;
    for (iter = m_vItem.begin(); iter != m_vItem.end(); iter++) {
        RopCfgItem* pItem = *iter;
        if (pItem) {
            pItem->SaveMD5();
        }
    }
    FlushFile();
}

void RopCfgFile::GenerateFile()
{
    SetPosHead();
    int iCur = 0;
    static char buf[1000][512];   // 1k lines max
    ZeroMemory(buf, 1000*512);

    /* Read all lines out */
    while(ReadLine(buf[iCur], 512)) {
        //strcat(buf[iCur],"\n");
        iCur++;
    }
    
    vector<RopCfgItem *>::iterator iter;
    int i = 0;
    char sName[256]  = {0};
    char sValue[256] = {0};
    BOOL bFlag = FALSE;
    for (iter = m_vItem.begin(); iter != m_vItem.end(); iter++) {
        RopCfgItem* pItem = *iter;
        bFlag = FALSE;
        for (i = 0; i < iCur; i++) {
            ParseLine(buf[i], sName, sValue);
            if (!StrCmpNIA(pItem->m_Name, sName, strlen(sName))) {
                bFlag = TRUE;
            }
            if (bFlag == TRUE && !StrCmpNIA(sName, "Md5", strlen(sName))) {
                sprintf(buf[i], "%s = %s", "Md5", pItem->Md5);
                break;
            }
        }
    }
        
    FILE *hNew = fopen("CFG_New", "w+");
    if (!hNew) {
        /* error */
        return;
    }
    
    for(i = 0; i < iCur; i++) {
        fputs(buf[i], hNew);
        fputs("\n", hNew);
    }
    fflush(hNew);
    fclose(hNew);
}


/* 
 * Make sure the pitem in the vector by calling
 * FindItem before Comparing it. If it doesnt exit
 * the sync is true.
 */
BOOL RopCfgFile::IsItemSync(RopCfgItem *pItem)
{
    BOOL bEqu = true;
    vector<RopCfgItem *>::iterator iter;
    for (iter = m_vItem.begin(); iter != m_vItem.end(); iter++) {
        RopCfgItem *pTmpItem = *iter;
        if (StrCmpNIA(pItem->m_Name, pTmpItem->m_Name, strlen(pTmpItem->m_Name)) == 0 &&
            strlen(pItem->m_Name) == strlen(pTmpItem->m_Name)) {
            if (!memcmp(pItem->Md5, pTmpItem->Md5, MD5_DIGEST_LENGTH*2)) {
                bEqu = false;
                break;
            }
        }
    }
    return bEqu;
}

BOOL RopCfgFile::FindItem(RopCfgItem *pItem)
{
    BOOL bFnd = false;
    vector<RopCfgItem *>::iterator iter;
    for (iter = m_vItem.begin(); iter != m_vItem.end(); iter++) {
        RopCfgItem *pTmpItem = *iter;
        if (StrCmpNIA(pItem->m_Name, pTmpItem->m_Name, strlen(pItem->m_Name)) == 0 &&
            strlen(pItem->m_Name) == strlen(pTmpItem->m_Name)) {
            bFnd = true;
            break;
        }
    }
    return bFnd;
}


/*
 * This comparing function should be called by
 * the 'New' config parser. 
 * The input config file *pOld should always be
 * the local file. 
 */
BOOL RopCfgFile::CompareFile(RopCfgFile *pOld)
{
    assert(pOld != NULL);
    vector<RopCfgItem *>::iterator iter1, iter2;

    for (iter1 = m_vItem.begin(); iter1 != m_vItem.end(); iter1++) {
        RopCfgItem *pNewItem = *iter1;
        if (pOld->FindItem(pNewItem)) {
            if (pOld->IsItemSync(pNewItem)) {
                pNewItem->m_bNeedUpdate = true;   // set new item be updated
                m_bNeedUpdate = true;
            }
        }else{
           pNewItem->m_bNeedUpdate = true;
           m_bNeedUpdate = true;
        }
    }
    return m_bNeedUpdate;
}



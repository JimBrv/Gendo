/* 
 * Base config file for client component, exe file line.
 * Client config file should be ansi, so this module 
 * uses char while dealing file.
 * 
 * All right reserved by secwing.com, 2010
 */
#ifndef ROP_CFG_FILE_H
#define ROP_CFG_FILE_H

#include "stdafx.h"
#include <stdio.h>
#include <vector>
#include "lib-msg.h"
#include "RopFileUtil.h"
#include "RopUpdate.h"
#include "RopCfgItem.h"
using namespace std;

#define TEMP_CFG_NAME    "_version_new_"
#define CFG_NAME         "version"

#define CFG_ITEM_KEY     "CfgItem"

BOOL DownLoadCfgFile(char *szAddr, int iPort, char *szToLocalFile = TEMP_CFG_NAME);

class RopCfgFile : public RopCfgBase
{
public:
    RopCfgFile(char *szFilePath, char *szFileName, PUiPhaseStepCB pCB, PVOID pOnUpdateCtx);
    ~RopCfgFile();

    BOOL IsNeedUpdate() {return m_bNeedUpdate;}
    BOOL Update();
    BOOL DownLoadCfgFile(char *szAddr, int iPort, char *szToLocalFile = TEMP_CFG_NAME);
    BOOL IsItemSync(RopCfgItem *pItem);
    BOOL FindItem(RopCfgItem *pItem);

    BOOL CompareFile(RopCfgFile *pOld);
    BOOL OpenFile();
    
    int  SetParam(char *szName,char *szValue);
    int  ReadAndInit();
    int  CalcTime(int iOpt);
    void CalcMD5();
    void SaveMD5();
    void GenerateFile();

    void GetServerDownloadAddr(char *sAddr, int &iLen) {strncpy(sAddr, m_HttpServerAddr, iLen);}
    int  GetServerDownloadPort(void) {return m_ServerDownloadPort;}
    void GetServerDownloadPath(char *sPath, int &iLen) {strncpy(sPath, m_ServerDownloadPath, iLen);}
    void GetServerAddr(char *sAddr, int &iLen) {strncpy(sAddr, m_ServerAddr, iLen);}
    int  GetServerServicePort(void) {return m_ServerServicePort;}

    DWORD GetDialSleep(void) {return m_dwDialSleep;}

public:
    vector <RopCfgItem*> m_vItem;   // vetor pool for component
    PUiPhaseStepCB       m_pUiCB;   // Callback function to notify UI
    PVOID                m_pOnUpdateCtx;   // Context for UI Notify

public:
    char m_LocalPath[MAX_NAME_LEN];
    char m_FileName[MAX_NAME_LEN];
    char m_OEMCompany[MAX_NAME_LEN];
    char m_OEMProduct[MAX_NAME_LEN];
    char m_OEMProgmenu[MAX_NAME_LEN];
    BOOL m_bNeedUpdate;
    int  m_Downloadmask;
    char m_ClientVersion[MAX_NAME_LEN];
    char m_HttpServerAddr[MAX_NAME_LEN];       // Http download server
    int  m_ServerDownloadPort;                 // Http server download port
    char m_ServerAddr[MAX_NAME_LEN];           // Service server address
    int  m_ServerServicePort;                  // Service Port
    char m_ServerDownloadPath[MAX_NAME_LEN];   // Path on server
    DWORD m_dwDialSleep;                       // Sleep count before PPPoE dialing

    char m_UpdateMsg1[MAX_NAME_LEN*10];         // update msg showing
    char m_UpdateMsg2[MAX_NAME_LEN*10];         
    char m_UpdateMsg3[MAX_NAME_LEN*10];         
    char m_UpdateMsg4[MAX_NAME_LEN*10];         
    char m_UpdateMsg5[MAX_NAME_LEN*10];        

};



#endif
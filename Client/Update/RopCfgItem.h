/* 
 * Base config file for client component, exe file line.
 * Client config file should be ansi, so this module 
 * uses char while dealing file.
 * All right reserved by secwing.com, 2010
 */
#ifndef ROP_CFG_ITEM_H
#define ROP_CFG_ITEM_H

#include "stdafx.h"
#include <stdio.h>
#include <vector>
#include "RopUpdate.h"
#include "lib-msg.h"
#include "RopUtil.h"
#include "RopFileUtil.h"
using namespace std;

#define FILETYPE_COMMON  1
#define FILETYPE_SERVICE 2
#define FILETYPE_EXE     3
#define FILETYPE_DRIVER  4

#define LOCAL_CFG_PATH         ".\\"
#define SERVER_CFG_PATH        "/download/"  
#define SERVER_CFG_HTTP_GET    "http://%s%s%s"             /* http://x.x.x.x/download/cfg */

class RopCfgBase
{
public:
    RopCfgBase(FILE* fp) {m_fp = fp;};
    RopCfgBase() {m_fp = NULL;};
    virtual ~RopCfgBase() {};

    void   SetFp(FILE *fp) {m_fp = fp;}
    char  *ReadLine(LPSTR lpLine,int iLen);
    int    WriteLine(LPSTR lpLine);
    int    ParseLine(LPSTR lpLine, LPSTR lpName, LPSTR lpValue);
    int    SetPosHead();
    void   FlushFile();
protected:
    FILE   *m_fp;
};

class RopCfgFile;

class RopCfgItem : public RopCfgBase
{
public:
    RopCfgItem(FILE *fp, char *szName, RopCfgFile *pCfgFile, PVOID pOnUpdateCtx);
    ~RopCfgItem();
    
    int   DownloadFile(char *szAddr, unsigned int port);
    int   CopyFile();
    BOOL  DeleteFile();
    BOOL  IsNeedUpdate() {return m_bNeedUpdate;}

    int  CalcTime(int iOpt);
    VOID CalcMD5();
    VOID SaveMD5();
    int  Update();
    int  SetParam(char *szName, char *szValue);
    int  SetPath(char *szOEMCompany,char *szOEMProduct,char *szTmpPath,char *szOEMProgmenu);
    int  ReadAndInit();

public:
    BOOL m_bNeedUpdate;
    char m_Name[MAX_NAME_LEN];
    char m_TmpPath[MAX_NAME_LEN];
    RopCfgFile *m_pCfgFile;
    PVOID m_pCallCtx;

public:	
    char Md5[MD5_DIGEST_LENGTH*2+1];
    char NameOnServer[MAX_NAME_LEN];
    char ExtOnServer[MAX_NAME_LEN];
    char NameOnLocal[MAX_NAME_LEN];
    char ExtOnLocal[MAX_NAME_LEN];
    char LocalPath[MAX_NAME_LEN*16];
	char ServerPath[MAX_NAME_LEN];
    char VerOnServer[MAX_NAME_LEN];
    char orgLocalPath[MAX_NAME_LEN];
    char TmpSuffix[MAX_NAME_LEN];
    char Guid[MAX_NAME_LEN]; 
    int  iFileType;                        
    int  iNeedTime;                  
    unsigned int iSupportOS;
    unsigned int SupportOS[32];		
    unsigned int DownloadMask;	
    unsigned int SizeOnServer;
};

#endif
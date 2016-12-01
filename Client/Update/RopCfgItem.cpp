#include "stdafx.h"
#include <io.h>
#include <Shlwapi.h>
#include "RopUtil.h"
#include "RopCfgItem.h"
#include "RopFileUtil.h"
#include "RopCfgFile.h"
#include <ctime>

char ProgramPath[512]={0};

char* RopCfgBase::ReadLine(LPSTR lpLine,int iLen)
{
    if (!m_fp)return NULL;
    return fgets(lpLine,iLen,m_fp);
}

int   RopCfgBase::WriteLine(LPSTR lpLine)
{
    if (!m_fp)return 0;
    return fputs(lpLine,m_fp);
}


int   RopCfgBase::SetPosHead()
{
    if (!m_fp) return -1;
    fpos_t pos = 0;
    return fsetpos(m_fp, &pos);
}

void RopCfgBase::FlushFile()
{
    if (!m_fp) return;
    fflush(m_fp);
}

int   RopCfgBase::ParseLine(LPSTR lpLine, LPSTR lpName, LPSTR lpValue)
{
    if (!lpLine)return -1;
    if (!lpName)return -1;
    if (!lpValue)return -1;

    StrTrimA(lpLine, (" \t\r\n"));
    if (strlen(lpLine)<=0) return 1;            //empty line
    if (StrCmpNIA(lpLine,"end",3)==0) return 2;  //end
    char* pPos = NULL;

    pPos = StrChrIA(lpLine,'=');                 //  delem '='
    if (pPos) {
        StrCpyNA(lpName,lpLine,pPos-lpLine+1);
        lpName[pPos-lpLine+1] = '\0';

        StrCpyNA(lpValue,pPos+1,strlen(lpLine)-(pPos-lpLine));
        lpValue[strlen(lpLine)-(pPos-lpLine)] = '\0';

        StrTrimA(lpName,(" \t\r\n"));
        StrTrimA(lpValue,(" \t\r\n"));
        return 3;            
    }

    pPos = NULL;
    pPos = StrChrIA(lpLine,':');                 //  delem ':'
    if (pPos) {
        StrCpyNA(lpName,lpLine,pPos-lpLine+1);
        lpName[pPos-lpLine+1] = '\0';

        StrCpyNA(lpValue,pPos+1,strlen(lpLine)-(pPos-lpLine));
        lpValue[strlen(lpLine)-(pPos-lpLine)+1] = '\0';

        StrTrimA(lpName,(" \t\r\n"));
        StrTrimA(lpValue,(" \t\r\n"));
        return 4;
    }
    return -2;    // bad
}


/*
 * Config item parser and container implementation
 */
RopCfgItem::RopCfgItem(FILE *fp, char *szName, RopCfgFile *pCfgFile, PVOID pCtx):RopCfgBase(fp)
{
    m_fp = fp;
    m_pCfgFile = pCfgFile;
    StrCpyNA(m_Name,szName,MAX_NAME_LEN);
    m_bNeedUpdate = FALSE;
    iSupportOS = 0;
    ZeroMemory(NameOnServer, sizeof(NameOnServer));
    ZeroMemory(ExtOnServer, sizeof(ExtOnServer));
    ZeroMemory(NameOnLocal, sizeof(NameOnLocal));
    ZeroMemory(ExtOnLocal, sizeof(ExtOnLocal));
    ZeroMemory(LocalPath, sizeof(LocalPath));
    ZeroMemory(VerOnServer, sizeof(VerOnServer));
	ZeroMemory(ServerPath,sizeof(ServerPath));
    strcpy(TmpSuffix, "tmp");
    m_pCallCtx = pCtx;
}

RopCfgItem::~RopCfgItem()
{
    
}

inline void streplace(char *source,char och,char nch){
	char *tmp = source;
	while(*tmp!='\0'){
		if(*tmp == och){
			*tmp = nch;
		}
		tmp++;
	}
	return;
}

int   RopCfgItem::DownloadFile(char *szAddr, unsigned int port)
{    
	char SourceFile[MAX_NAME_LEN*16]={0};
	char DestFile[MAX_NAME_LEN*16]={0};

    if (strlen(ExtOnServer) > 0) {
        sprintf(SourceFile,"%s%s/%s.%s", m_pCfgFile->m_ServerDownloadPath,ServerPath,NameOnServer,ExtOnServer);
    }else{
        sprintf(SourceFile,"%s%s/%s", m_pCfgFile->m_ServerDownloadPath,ServerPath, NameOnServer);
    }
    if (strlen(ExtOnLocal) > 0) {
        sprintf(DestFile,"%s\\%s.%s.%s",LocalPath, NameOnLocal, ExtOnLocal, TmpSuffix);
    }else{
        sprintf(DestFile,"%s\\%s.%s",LocalPath, NameOnLocal, TmpSuffix);
    }
	LDebug(DEBUG_L_DEBUG,"CfgItem download src %s.\n",SourceFile);
    ::DeleteFileA(DestFile);
	LDebug(DEBUG_L_DEBUG,"CfgItem download dst %s.\n",DestFile);

	srand(unsigned(time(0)));
	sprintf(SourceFile,"%s?id=%d",SourceFile,rand());
    int iRet = HttpDownloadFile(szAddr, port, SourceFile, DestFile, m_pCallCtx, NameOnLocal);  
    return iRet;
}

int RopCfgItem::CopyFile()
{
    char SourceFile[MAX_NAME_LEN*16];
    char DestFile[MAX_NAME_LEN*16];
    char DestFile_[MAX_NAME_LEN*16];
    char ver[256];
	if (strlen(ExtOnLocal) == 0){
		sprintf(SourceFile,"%s\\%s.%s", LocalPath, NameOnLocal, TmpSuffix);
		sprintf(DestFile,"%s\\%s",LocalPath, NameOnLocal);
	}else{
		sprintf(SourceFile,"%s\\%s.%s.%s", LocalPath, NameOnLocal, ExtOnLocal, TmpSuffix);
		sprintf(DestFile,"%s\\%s.%s",LocalPath, NameOnLocal, ExtOnLocal);
	}
    ::DeleteFileA(DestFile);
    if (!MoveFileExA(SourceFile,DestFile,MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED)) {
        if (ERROR_ACCESS_DENIED == GetLastError()) {
            SYSTEMTIME t;
            GetSystemTime(&t);
            sprintf(DestFile_,"%s\\%s.%s_%d%d%d%d%d",m_TmpPath,NameOnLocal,ExtOnLocal,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
            MoveFileExA(DestFile,DestFile_,MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED);
            MoveFileExA(DestFile_,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);

            if (!MoveFileExA(SourceFile,DestFile,MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED)) {
                LDebug(DEBUG_L_ERROR,"%s need reboot %d\n",DestFile,GetLastError());
                MoveFileExA(SourceFile,DestFile,MOVEFILE_REPLACE_EXISTING|MOVEFILE_DELAY_UNTIL_REBOOT);
            }
        }
    }
    GetFileVersion(DestFile,ver);
    RopSetFileSecurity((unsigned char*)DestFile);

    return 0;
}

BOOL  RopCfgItem::DeleteFile()
{
    char SourceFile[MAX_NAME_LEN];
    char DestFile[MAX_NAME_LEN];
    char DestFile_[MAX_NAME_LEN];
    sprintf(SourceFile,"%s\\%s",m_TmpPath,NameOnServer);
    sprintf(DestFile,"%s\\%s.%s",LocalPath,NameOnLocal,ExtOnLocal);
    if (!::DeleteFileA(DestFile)) {
        if (ERROR_ACCESS_DENIED == GetLastError()) {
            LDebug(DEBUG_L_ERROR,"In using,Can't delete %s error(%d)\n",DestFile,GetLastError());
            SYSTEMTIME t;
            GetSystemTime(&t);
            sprintf(DestFile_,"%s\\%s.%s_%d%d%d%d%d",LocalPath,NameOnLocal,ExtOnLocal,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
            MoveFileExA(DestFile,DestFile_,MOVEFILE_REPLACE_EXISTING|MOVEFILE_COPY_ALLOWED);
            MoveFileExA(DestFile_,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
        } else {
            //LDebug(DEBUG_L_ERROR,"DeleteFile fail,%s(%d)",DestFile,GetLastError());
        }
    }
    return TRUE;
}

int  RopCfgItem::CalcTime(int iOpt)
{
    switch (iFileType) {
        case FILETYPE_COMMON:
            { //common file, just copy or replace
                return 5;
            }
            break;
        case FILETYPE_DRIVER: 
            { //driver, need kml support
                return 10;
            }
            break;
        case FILETYPE_SERVICE:
            { // service, more time needs
                return 20;
            }
            break;
        default:
            break;
    }
    return 0;
}
int  RopCfgItem::Update()
{   
    if (m_pCfgFile->m_pUiCB) {
        m_pCfgFile->m_pUiCB(iNeedTime, PHASE_UPDATE_COMPONENT, NameOnLocal);
    } 
    int iRet = DownloadFile(m_pCfgFile->m_HttpServerAddr, m_pCfgFile->m_ServerDownloadPort);
    if (!iRet) {
        /* error while download file */
        return -1;
    }else{
		return 0;
	}
}

int  RopCfgItem::SetParam(char *szName, char *szValue)
{
	if (!szName)return -1;
    if (!szValue)return -1;
    StrTrimA(szName,("\t\r\n"));
    StrTrimA(szValue,("\t\r\n"));
    if (StrCmpNIA(szName,"NameOnServer",strlen(szName))==0) {
        StrCpyNA(NameOnServer,szValue,MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"ExtOnServer",strlen(szName))==0) {
        StrCpyNA(ExtOnServer,szValue,MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"NameOnLocal",strlen(szName))==0) {
        StrCpyNA(NameOnLocal,szValue,MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"ExtOnLocal",strlen(szName))==0) {
        StrCpyNA(ExtOnLocal,szValue,MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"LocalPath",strlen(szName))==0) {
	StrCpyNA(orgLocalPath,szValue,MAX_NAME_LEN);    
	char prgpath[257]={0},Tmp_szValue[MAX_NAME_LEN];
	int szValueLen = strlen(szValue);
	if(szValueLen<=2){
		sprintf(LocalPath,"%s",ProgramPath);
	}else{
		memcpy(prgpath,szValue+1,szValueLen-2);
		memcpy(ServerPath,prgpath,strlen(prgpath));
		streplace(prgpath,'/','\\');
		memcpy(Tmp_szValue,prgpath,MAX_NAME_LEN);
		char *strtmp = strtok(Tmp_szValue,"\\");
		if(!strtmp){
			sprintf(LocalPath,"%s\\%s",ProgramPath, Tmp_szValue);
			if ( (_access( LocalPath, 0 )) == -1 ) {
				CreateDirectoryA(LocalPath,NULL);
				LDebug(DEBUG_L_DEBUG,"create %s dir\r\n",LocalPath);
			}
		}else{
			sprintf(LocalPath,"%s\\%s",ProgramPath, strtmp);
			if ( (_access( LocalPath, 0 )) == -1 ) {
				CreateDirectoryA(LocalPath,NULL);
				LDebug(DEBUG_L_DEBUG,"create %s dir\r\n",LocalPath);
			}
			while((strtmp = strtok(NULL,"\\"))!=NULL){
				strcat(LocalPath,"\\");
				strcat(LocalPath,strtmp);
				if ( (_access( LocalPath, 0 )) == -1 ) {
					CreateDirectoryA(LocalPath,NULL);
					LDebug(DEBUG_L_DEBUG,"create %s dir\r\n",LocalPath);
				}
			}
		}
	}
	extern int RopSetFileSecurity(unsigned char* szFileName);
	RopSetFileSecurity((unsigned char*)LocalPath);
    } else if (StrCmpNIA(szName,"Guid",strlen(szName))==0) {
        StrCpyNA(Guid,szValue,MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"VerOnServer",strlen(szName))==0) {
        StrCpyNA(VerOnServer,szValue,MAX_NAME_LEN);
    } else if (StrCmpNIA(szName,"FileType",strlen(szName))==0) {
        iFileType = atoi(szValue);
    } else if (StrCmpNIA(szName,"SupportOS",strlen(szName))==0) {
        SupportOS[iSupportOS] = atoi(szValue);
        iSupportOS ++;
    } else if (StrCmpNIA(szName,"DownloadMask",strlen(szName))==0) {
        DownloadMask = atoi(szValue);
    } else if (StrCmpNIA(szName,"SizeOnServer",strlen(szName))==0) {
        SizeOnServer = atoi(szValue);
    } else if (StrCmpNIA(szName,"MD5",strlen(szName))==0) {
        StrCpyNA(Md5, szValue, MD5_DIGEST_LENGTH*2+1);        
    } 
    else {
        return -1;
    }
    return 0;
}

int  RopCfgItem::SetPath(char *szOEMCompany,char *szOEMProduct,char *szTmpPath,char *szOEMProgmenu)
{
/*
    StrCpyNA(m_OEMCompany,szOEMCompany,MAX_NAME_LEN);
    StrCpyNA(m_OEMProduct,szOEMProduct,MAX_NAME_LEN);
    StrCpyNA(m_OEMProgmenu,szOEMProgmenu,MAX_NAME_LEN);*/
    StrCpyNA(m_TmpPath, szTmpPath, MAX_NAME_LEN);
    return 0;
}

int  RopCfgItem::ReadAndInit()
{
    char Line[1024];
    char Name[1024];
    char Value[1024];
	char tmppath[512]={0};
	if(!GetModuleFileNameA(NULL,tmppath,512))  
	{  
		LDebug(DEBUG_L_DEBUG,"GetModuleFileName failed (%d)\n", GetLastError());  
		return -1;  
	} 
	char *chptr = strrchr((char*)tmppath,'\\');
	memcpy(ProgramPath,tmppath,chptr-tmppath);
    while (ReadLine(Line, 1023)) {
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
                    return 0;
                }
                break;
            case 3: 
                {
                    //valid parameters.
                    SetParam(Name,Value);
                    break;
                }
                break;
            case 4: 
                {
                    //: line readed,error
                    LDebug(DEBUG_L_ERROR,"CfgItem,:line read error.\n");
                    return -1;
                }
                break;
            default:
                return -2;
        }
    }
    return 0;
}

VOID RopCfgItem::CalcMD5()
{
    char sName[128] = {0};
    char sMD5[128] = {0};
    sprintf(sName, "%s.%s", NameOnLocal, ExtOnLocal);
    if (GetFileMD5(sName, sMD5)) {
        memcpy(Md5, sMD5, MD5_DIGEST_LENGTH*2);
        Md5[MD5_DIGEST_LENGTH*2] = 0;
    }
}

VOID RopCfgItem::SaveMD5()
{
    char Line[1024];
    char Name[1024];
    char Value[1024];
    BOOL bFlag = false;

    SetPosHead();
    while (ReadLine(Line, 1023)) {
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
                    //return;
                }
                break;
            case 3: 
                {
                    /* Item MD5 followed */
                    if (bFlag && !StrCmpNIA(Name, "MD5", strlen("MD5"))) {
                        char sNewLine[1024] = {0};
                        sprintf(sNewLine, "%s = %s", "MD5", "01234578901234578901234578912");
                        WriteLine(sNewLine);
                        return;
                    }
                }
                break;
            case 4: 
                {
                    /* Item start */
                    if (!StrCmpNIA(Name, m_Name, strlen(Name))) {
                        bFlag = TRUE;
                    }
                }
                break;
            default:
                return;
        }
    }
    return;
}
#include "stdafx.h"
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "windows.h"
#include "RopUtil.h"
#include "RopRegistry.h"

#if 0
void Dbg(char *p_Format, ...)
{
    static char l_Buffer [4096];
    va_list l_ArgumentList;
    va_start(l_ArgumentList, p_Format);
    vsprintf(l_Buffer, p_Format, l_ArgumentList);
    va_end(l_ArgumentList);

#if defined(USE_DBGPRINT)
    OutputDebugStringA(l_Buffer);
#else
    printf(l_Buffer);
#endif
}
#endif

unsigned int DebugLevel = DEBUG_L_DEBUG;
unsigned int DebugType  = DEBUG_OUT_VIEW|DEBUG_OUT_FILE;     

#define DEBUG_INFO_LEN 10240
void LDebug(unsigned int iLevel,char *p_Format, ...)
{
    DWORD iUseit = 0;
    DWORD iLevelReg = 0;
    DWORD iTypeReg = 0;
    RopRegistry key;
    key.Open(HKLM, DEBUG_SWITCH);
    //key.SetKeySecurity(HKLM, DEBUG_SWITCH);
    key.Read(TEXT("DebugSettingOn"),iUseit);
    key.Read(TEXT("DebugLevel"),iLevelReg);
    key.Read(TEXT("DebugOut"),iTypeReg);
    key.Close();

    if(iUseit)
    {		
        DebugLevel = iLevelReg;
        DebugType = iTypeReg;
    }	

    if (iLevel>=DebugLevel) {
        char a_Buffer [DEBUG_INFO_LEN+512];
        char l_Buffer [DEBUG_INFO_LEN+256];
        memset(a_Buffer,0,sizeof(a_Buffer));
        memset(l_Buffer,0,sizeof(l_Buffer));
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		char  tmStr[32] ;
		sprintf_s(tmStr,32," %d-%d-%d %d:%d:%d",tm.wYear,tm.wMonth,tm.wDay,tm.wHour,tm.wMinute,tm.wSecond);

        _snprintf(a_Buffer, DEBUG_INFO_LEN+256, "[%s%s]:", "Gen",tmStr);
        va_list l_ArgumentList;
        va_start (l_ArgumentList, p_Format);
        _vsnprintf(l_Buffer, DEBUG_INFO_LEN,p_Format, l_ArgumentList);
        va_end (l_ArgumentList);
        strncat(a_Buffer,l_Buffer,DEBUG_INFO_LEN+256);



        if (DebugType & DEBUG_OUT_VIEW) {
            OutputDebugStringA(a_Buffer);
        } 

        if (DebugType & DEBUG_OUT_FILE) {
            DWORD dwWritten = 0;
			
			WCHAR wDirPath[MAX_PATH]={0};
			WCHAR wLogPath[MAX_PATH]={0};
		
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
			mb2wc(Path,strlen(Path),wDirPath,(int*)&Len);
			wsprintf(wLogPath,TEXT("%s\\%s"),wDirPath,LogName);

            HANDLE h = CreateFile(wLogPath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
            if (SetFilePointer ( h, 0, NULL, FILE_END ) > 2*1024*1024) {
                SetFilePointer ( h, 0, NULL, FILE_BEGIN );
                SetEndOfFile(h);
            }
            WriteFile(h, a_Buffer, strlen(a_Buffer), &dwWritten, NULL );
            CloseHandle(h );
        }
    }
}

static ModInfo ModAry[] = {
    {UI_USER_MODULE,   UI_USER_MODULE_NAME,   0, 0},
    {UI_ETM__MODULE,   UI_ETM_MODULE_NAME,    0, 0},
    {CTRL_USER_MODULE, CTRL_USER_MODULE_NAME, 0, 0},
    {CTRL_ETM_MODULE , CTRL_ETM_MODULE_NAME,  0, 0},
    {CTRL_UPDATE_MODULE, CTRL_UPDATE_NAME,    0, 0},
    {MSG_SVC_MODULE ,  MSG_SVC_MODULE_NAME,   0, 0},
    {MON_SVC_MODULE ,  MON_SVC_MODULE_NAME,   0, 0},
};

static TCHAR *Mid2Mname(int id)
{
    for (int iTmp = 0; iTmp < (sizeof(ModAry)/sizeof(ModInfo)); iTmp++) {
        if (ModAry[iTmp].ID == id) return ModAry[iTmp].Name;
    }
    return NULL;
}

/* Print with module name */
void ModDbg(unsigned int iLevel, unsigned int iMod, char *p_Format, ...)
{
    DWORD iUseit = 0;
    DWORD iLevelReg = 0;
    DWORD iTypeReg = 0;
    RopRegistry key;
    key.Open(HKLM, DEBUG_SWITCH);
    key.Read(TEXT("DebugSettingOn"),iUseit);
    key.Read(TEXT("DebugLevel"),iLevelReg);
    key.Read(TEXT("DebugOut"),iTypeReg);
    key.Close();

    if(iUseit)
    {		
        DebugLevel = iLevelReg;
        DebugType = iTypeReg;
    }	

    if (iLevel>=DebugLevel) {
        char a_Buffer [DEBUG_INFO_LEN+512];
        char l_Buffer [DEBUG_INFO_LEN+256];
        char szName[36] = {0};
        int  iLen = 36;
        TCHAR *pwName = NULL;

        memset(a_Buffer,0,sizeof(a_Buffer));
        memset(l_Buffer,0,sizeof(l_Buffer));
        
        pwName = Mid2Mname(iMod);
        assert(pwName != NULL);
        wc2mb(pwName, wcslen(pwName), szName, &iLen);
        
		SYSTEMTIME tm;
		GetLocalTime(&tm);
		char  tmStr[32] ;
		sprintf_s(tmStr,32," %d-%d-%d %d:%d:%d",tm.wYear,tm.wMonth,tm.wDay,tm.wHour,tm.wMinute,tm.wSecond);

        _snprintf(a_Buffer, DEBUG_INFO_LEN + 256, "[%s%s]:", szName,tmStr);
        va_list l_ArgumentList;
        va_start (l_ArgumentList, p_Format);
        _vsnprintf(l_Buffer, DEBUG_INFO_LEN, p_Format, l_ArgumentList);
        va_end (l_ArgumentList);
        strncat(a_Buffer, l_Buffer, DEBUG_INFO_LEN + 256);

        if (DebugType & DEBUG_OUT_VIEW) {
            OutputDebugStringA(a_Buffer);
        } 

        if (DebugType& DEBUG_OUT_FILE) {
            DWORD dwWritten = 0;

			WCHAR wDirPath[MAX_PATH]={0};
			WCHAR wLogPath[MAX_PATH]={0};

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
			mb2wc(Path,strlen(Path),wDirPath,(int*)&Len);

		    wsprintf(wLogPath,TEXT("%s\\%s"),wDirPath,LogName);

            HANDLE h = CreateFile(wLogPath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
            if (SetFilePointer ( h, 0, NULL, FILE_END ) > 2*1024*1024) {
                SetFilePointer ( h, 0, NULL, FILE_BEGIN );
                SetEndOfFile(h);
            }
            WriteFile(h, a_Buffer, strlen(a_Buffer), &dwWritten, NULL );
            CloseHandle(h );
        }
    }
}

int utf82wc(wchar_t *pu8, int utf8Len, TCHAR *pwc, int pwcLen)
{
    int nRtn = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)pu8, -1, pwc, pwcLen);  
    //LDebug(DEBUG_L_INFO, "%S", pwc);
    return nRtn;
}



int  wc2mb(const wchar_t *wc, int wlen, char *mb, int *mlen)
{
    assert(wc != NULL && mb != NULL);
    size_t wl = wcslen(wc) + 1;
    size_t cl = *mlen;
    size_t converted = 0;
    if (wl > *mlen) return -1;
    //wcstombs_s(&converted, mb, wl, wc, _TRUNCATE);
    converted = WideCharToMultiByte(CP_ACP,0,wc,wl,mb,cl,NULL,NULL);
    *mlen = converted;

    return 0;
}

int  mb2wc(const char *mb, int mlen, wchar_t *wc, int *wlen)
{
    assert(wc != NULL && mb != NULL);
    size_t ml = strlen(mb) + 1;
    size_t converted = 0;
    if (ml > *wlen) return -1;
    //mbstowcs_s(&converted, wc, ml, mb, _TRUNCATE);
    converted = MultiByteToWideChar(CP_ACP,0,mb,-1,wc,*wlen);
    *wlen = converted;
    return 0;    
}

PVOID MemAlloc (ULONG p_Size, BOOLEAN zero)
{
    PVOID l_Return = NULL;
    if (p_Size) {
        __try {
            if (zero) {
                l_Return = LocalAlloc(LPTR,p_Size);
            } else {
                l_Return = LocalAlloc(LMEM_FIXED,p_Size);
            }

        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            l_Return = NULL;
        }
    }
    return l_Return;
}

VOID MemFree (PVOID p_Addr, ULONG p_Size)
{
    if (p_Addr && p_Size) {
        __try {
            LocalFree(p_Addr);
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
        }
    }
}

#include <io.h>
#include "md5.h"

#define MD5_FILE_BUFSIZE 16*1024
bool GetFileMD5(const char *sFileName,char *sMd5)
{
    bool bRet = true;
    if (!sMd5 || !sFileName)return false;
    FILE* fp=fopen(sFileName,"rb");
    if (fp) {
        MD5_CTX c;
        unsigned char md[MD5_DIGEST_LENGTH];
        int fd;
        int i;
        static unsigned char buf[MD5_FILE_BUFSIZE];

        fd=fileno(fp);
        MD5_Init(&c);
        for (;;) {
            i=read(fd,buf,MD5_FILE_BUFSIZE);
            if (i <= 0) break;
            MD5_Update(&c,buf,(unsigned long)i);
        }
        MD5_Final(&(md[0]),&c);

        //strncpy(strMd5,(const char*)md,MD5_DIGEST_LENGTH);
        sMd5[0] = 0;
        for (i=0;i<MD5_DIGEST_LENGTH;i++) {
            char strSub[16];
            sprintf(strSub,"%02x",md[i]);
            strcat(sMd5,strSub);
        }

        fclose(fp);
        return true;
    } else {
        return false;
    }
}


/* Get current program/dll location directory */
static char  cSelfPath[256] = {0};
char *GetExeName()
{
	WCHAR sPath[256] = {0};
	DWORD dwRet = GetModuleFileName(NULL, sPath, 255);
	DWORD dwLen = 256;
	if (dwRet == 0) {
		Dbg("GetModuleFileName fail=%d", GetLastError());
		return NULL;
	}
	wc2mb(sPath, dwRet, cSelfPath, (int*)&dwLen);	
	return cSelfPath;
}


/* make sure each dst ary item has enough memory  */
int StringSplit(char *src, char split, char **dst, int row_len,  int *n)
{
    int i = 0;
    char *start = NULL, *end = NULL, *mid = NULL;

    if (!src || !split || !dst || !n) return -1;
    start = src;
    end   = src + strlen(src);

    while(*start != 0 && (mid = strchr(start, split)) != NULL) {
        if (*n < i) return -1;
        char *pCur = (char *)((dst) + i*row_len/4);
        memcpy(pCur, start, (int)(mid - start));
        *(pCur+(mid - start)) = 0;
        i++;
        start = ++mid;
    }

    if (*start != 0) {
        char *pCur = (char *)((dst) + i*row_len/4);
        memcpy(pCur, start, (int)(end - start));
        *(pCur+(end - start)) = 0;
        i++;
    }

    *n = i;
    return 0;
}

void Write2File(char* file_name,char *p_Format, ...)
{
    if (!file_name || !strlen(file_name)){
        return;
    }

    SYSTEMTIME tm;
    GetLocalTime(&tm);
    char  tmStr[32] ;
    sprintf_s(tmStr,32,"%d-%d-%d %d:%d:%d",tm.wYear,tm.wMonth,tm.wDay,tm.wHour,tm.wMinute,tm.wSecond);

    char a_Buffer [DEBUG_INFO_LEN+512];
    char l_Buffer [DEBUG_INFO_LEN+256];
    va_list l_ArgumentList;
    va_start (l_ArgumentList, p_Format);
    _vsnprintf(l_Buffer, DEBUG_INFO_LEN,p_Format, l_ArgumentList);
    va_end (l_ArgumentList);
    sprintf(a_Buffer,"\n%s  ", tmStr);
    strncat(a_Buffer,l_Buffer,DEBUG_INFO_LEN+256);

    DWORD dwWritten = 0;
    WCHAR wDirPath[MAX_PATH]={0};
    WCHAR wLogPath[MAX_PATH]={0};
    WCHAR wDirPathFile[MAX_PATH] = {0};
    char szPathAnsci[MAX_PATH] = {0};
    char drive[_MAX_DRIVE] = {0};
    char dir[_MAX_DIR] = {0};
    char Path[MAX_PATH] = {0};
    size_t Len = MAX_PATH;
    GetModuleFileName(NULL,wDirPathFile,2048);
    wc2mb(wDirPathFile,wcslen(wDirPathFile),szPathAnsci,(int*)&Len);
    _splitpath(szPathAnsci, drive, dir, NULL,NULL);
    sprintf(Path,"%s%s%s",drive,dir,file_name);
    mb2wc(Path,strlen(Path),wDirPath,(int*)&Len);

    HANDLE h = CreateFile(wDirPath, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
    if (SetFilePointer ( h, 0, NULL, FILE_END ) > 2*1024*1024) {
        SetFilePointer ( h, 0, NULL, FILE_BEGIN );
        SetEndOfFile(h);
    }

    WriteFile(h, a_Buffer, strlen(a_Buffer), &dwWritten, NULL );
    CloseHandle(h );

}

BOOL IsOs64bit() 
{ 
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL); 
    LPFN_ISWOW64PROCESS fnIsWow64Process; 
    BOOL bIsWow64 = FALSE; 
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("kernel32")), "IsWow64Process"); 
    if (NULL != fnIsWow64Process) { 
        fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
    } 
    return bIsWow64; 
} 

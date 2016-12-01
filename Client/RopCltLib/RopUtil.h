#ifndef ROP_UTILS
#define ROP_UTILS
#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include "RopModule.h"
#define USE_DBGPRINT


#define DEBUG_L_ALL      0
#define DEBUG_L_DEBUG    1
#define DEBUG_L_INFO     2
#define DEBUG_L_NOTICE   3
#define DEBUG_L_WARNING  4
#define DEBUG_L_ERROR    5
#define DEBUG_L_CRITICAL 6
#define DEBUG_L_ALERT    7
#define DEBUG_L_EMERG    8

#define DEBUG_OUT_VIEW   1
#define DEBUG_OUT_FILE   2

#define DEBUG_SWITCH TEXT("software\\gendo\\debug")
#define LogName TEXT("log.txt")

#define SIZEOF_ARRAY(X)   (sizeof((X))/(sizeof(X[0])))


/* Debug function */
void LDebug(unsigned int iLevel,char *p_Format, ...);
#define Dbg(...) LDebug(DEBUG_L_DEBUG, ##__VA_ARGS__)
void ModDbg(unsigned int iLevel, unsigned int iMod, char *p_Format, ...);


/* wchar/char convertor */
int  wc2mb(const wchar_t *wc, int wlen, char *mb, int *mlen);
int  mb2wc(const char *mb, int mlen, wchar_t *wc, int *wlen);

/* utf8/wchar convertor */
int utf82wc(wchar_t *pu8, int utf8Len, TCHAR *pwc, int pwcLen);


/* memory wrapper */
PVOID MemAlloc (ULONG p_Size, BOOLEAN zero);
VOID  MemFree (PVOID p_Addr, ULONG p_Size);

/* MD5 wrapper */
#include "md5.h"
bool GetFileMD5(const char *sFileName, char *sMd5);

INT StringSplit(char *src, char split, char **dst, int row_len,  int *n);

void Write2File(char* file_name,char *p_Format, ...);

BOOL IsOs64bit();

#endif
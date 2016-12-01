#ifndef ROP_FILEUTIL_H
#define ROP_FILEUTIL_H
#include "stdafx.h"


int  RopSetFileSecurity(unsigned char* szFileName);
BOOL IsAdmin();
BOOL GetFileVersion(char* FileName,char* Version);

/* Downlaod file from internet by http */
int  HttpDownloadFile(char *sIP, int iPort, char *sSrcFile, char *sDstFile, PVOID pCtx, char *sShowName);


#endif
#ifndef ROP_TAP_H
#define ROP_TAP_H
#include "stdafx.h"
#include <Windows.h>
#include "RopBase.h"
#include "RopUtil.h"
#include "RopProcess.h"
#include "RopNetState.h"
#include "RopNet.h"

#define  TAPNAME_32 "TAP-Win32 Adapter V9"
#define  TAPNAME_64 "TAP-Windows Adapter V9"

class RopTap : public RopBase 
{
public:
    RopTap()  {}
    ~RopTap() {}
public:
    INT  TapInstall();
    INT  TapUnstall();
    INT  TapStatus();
    INT  TapRestore();
    INT  TapSetIpDns(char *ip, char *mask, char *gw, char *dns1, char *dns2);
    INT  TapSetName(CHAR *name);
    BOOL IsTapNameOK(CHAR *name);
};

#endif
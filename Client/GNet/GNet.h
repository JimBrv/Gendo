// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 GNET_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// GNET_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef GNET_EXPORTS
#define GNET_API __declspec(dllexport)
#else
#define GNET_API __declspec(dllimport)
#endif
#include "stdafx.h"
#include <Windows.h>
#include "RopBase.h"
#include "RopUtil.h"
#include "RopProcess.h"
#include "RopNetState.h"

#define  TAPNAME "TAP-Win32 Adapter V9"
#define  TAPIP   "10.8.18.8"

extern "C"
{
    GNET_API INT  TapInstall();
    GNET_API INT  TapUnstall();
    GNET_API INT  TapStatus();
    GNET_API INT  TapRestore();
    GNET_API INT  TapSetIpDns(char *ip, char *mask, char *gw, char *dns1, char *dns2);
};


typedef INT  (*pfnTapInstall)();
typedef INT  (*pfnTapUnstall)();
typedef INT  (*pfnTapStatus)();
typedef INT  (*pfnTapRestore)();
typedef INT  (*pfnTapSetIpDns)(char *, char *, char *, char *, char *);

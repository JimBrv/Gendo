#ifndef ROP_MODULE_H
#define ROP_MODULE_H
#include "stdafx.h"
#include <windows.h>
#include <tchar.h>

#define NAME_LEN  128

#define ALL_MODULE         (0xffffffff)
#define CTRL_USER_MODULE   (0x01<<1)
#define UI_USER_MODULE     (0x01<<2)
#define CTRL_ETM_MODULE    (0x01<<3)
#define UI_ETM__MODULE     (0x01<<4)
#define CTRL_UPDATE_MODULE (0x01<<5)

#define MSG_SVC_MODULE     (0x01<<8)
#define MON_SVC_MODULE     (0x01<<9)

#define CTRL_USER_MODULE_NAME   TEXT("UserCtrl")
#define UI_USER_MODULE_NAME     TEXT("UserUI")
#define CTRL_ETM_MODULE_NAME    TEXT("EtmCtrl")
#define UI_ETM_MODULE_NAME      TEXT("EtmUI")
#define CTRL_UPDATE_NAME        TEXT("Update")

#define MSG_SVC_MODULE_NAME     TEXT("MsgSvc")
#define MON_SVC_MODULE_NAME     TEXT("MonSvc")

#define MAX_MODULEID (0x01<<10)

#define IS_TO_MODULE(MS, MD) ((MS) & (MD)) 

#define MAGIC_TAG 0x09182736

/* TLV, modules format msg betweens in Rop Client Modules, dispatches by MsgBus */
typedef struct _Msg_Module_s_
{
    unsigned int tag;     // Magic
    unsigned int sModule; // Src module
    unsigned int dModule; // Dst module
    unsigned int len;     // Length
    char payload[0];      // Content container, should contains msg_head_t
}ModMsg, *PModMsg;


class RopAsyncDll;  /* Make inter include happy */

typedef struct _MODULE_INFO_
{
    int          ID;
    TCHAR        Name[NAME_LEN];
    RopAsyncDll *Handle;
    PVOID        reserve;
}ModInfo, *PModInfo;

#endif
#ifndef ROP_EVENT_H
#define ROP_EVENT_H
#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>

#include "RopBuf.h"
#include "RopAsyncIO.h"
#include "RopUtil.h"

typedef void (*PfnEventCallBack)(PVOID pContext,PVOID pSSocket,unsigned char* pBuf,DWORD dwLen,DWORD dwEventType);

/* Base event dispatcher */
class RopEventDispatcher
{
public:
    PfnEventCallBack m_pCallBack;
public:
    RopEventDispatcher(){m_pCallBack = NULL;};
    virtual void OnEvent(RopAsyncIO *pSrcIO, RopAsyncIO *pDstIO,ROP_BUF* pBuf,DWORD dwEventType);
};

#endif

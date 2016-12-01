
#ifndef ROP_ASYNCDLL_H
#define ROP_ASYNCDLL_H

#include "stdafx.h"
#include "windows.h"
#include "RopQueue.h"
#include "RopBuf.h"
#include "RopModule.h"
#include "RopUtil.h"
#include "RopAsyncIO.h"
#include "RopEventDispatcher.h"

#define MODULE_NAME_LEN 256
#define MAX_QUEUE_ITEM  10240

/* The export function in Dll module */
typedef DWORD (*DllMsgCB)(DWORD dwContext,PVOID pInMsg,DWORD dwInLen);
typedef DWORD (*DllInitialize)();
typedef DWORD (*DllFinalize)();
typedef DWORD (*DllInfo)(ModInfo *pInfo);
typedef DWORD (*DllMsgHandler)(DWORD dwContext,PVOID pInMsg,DWORD dwInLen);
typedef DWORD (*DllSetMsgCB)(DllMsgCB pMsgCb, DWORD dwContext);

class RopAsyncDllException
{
};

class RopAsyncDll: public virtual RopAsyncIO
{
public: 
    RopAsyncDll();
    virtual ~RopAsyncDll();	
public:  
    virtual BOOL CheckAndDoShutdown();

    //virtual int Open(LPCSTR szResource, DWORD dwAttr);
    virtual int  Open(LPCTSTR szFilePath, DWORD dwDesiredAccess);

    virtual int  Send (ROP_BUF& Buf);
    virtual int  Send (char* buf,DWORD len);
    virtual void Receive();
    virtual int  OnReceive();
    virtual int  ReceiveAndWait(char* buf,DWORD len);
    virtual void TimeoutEvent();


protected:
    DWORD NotifyMsg(PVOID pInMsg,DWORD dwInLen);

    static DWORD MsgCallback(DWORD dwContext,PVOID pInMsg,DWORD dwInLen);

    DllInitialize Initialize;
    DllMsgHandler MsgHandler;
    DllSetMsgCB   SetMsgCB;

    DWORD     PeerContext;
    RopQueue* MsgQ;

public:
    DllInfo       Info;
    DllFinalize   Finalize;	

private:
    static DWORD WINAPI DllSendThdProc(LPVOID lpParameter);

    int OnSend();
    RopQueue* SendQ;
public:

public:
    bool   m_bStopped;
    HANDLE m_SendEvent;
    HANDLE m_StopEvent;
    DWORD OldTickCount;
    DWORD OldTickCount_OBJ;
};

#endif

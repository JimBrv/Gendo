#include "stdafx.h"
#include "RopAsyncDll.h"

RopAsyncDll::RopAsyncDll()
{
    m_Handle = (HANDLE)INVALID_SOCKET;
    Initialize=NULL;
    Finalize=NULL;
    Info=NULL;
    MsgHandler=NULL;
    SetMsgCB=NULL;
	m_bStopped = false;
	m_SendEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	OldTickCount = 0;
	OldTickCount_OBJ = 0;
}

RopAsyncDll::~RopAsyncDll()
{
    if (m_Handle) {
        if (Finalize)Finalize();
        FreeLibrary((HINSTANCE)m_Handle);
        m_Handle = (HANDLE)NULL;
    }
	
    if (MsgQ) {
        ROP_BUF* pTempMsg = NULL;
        while (pTempMsg = (ROP_BUF*)MsgQ->Pop()) {
            MemFree(pTempMsg,1);
        }
        delete MsgQ;
        MsgQ = NULL;
    }
	
	if (SendQ) {
        ROP_BUF* pTempMsg = NULL;
        while (pTempMsg = (ROP_BUF*)SendQ->Pop()) {
            MemFree(pTempMsg,1);
        }
        delete SendQ;
        SendQ = NULL;
    }
	
	if(m_SendEvent)CloseHandle(m_SendEvent);
	if(m_StopEvent)CloseHandle(m_StopEvent);
	
}

DWORD WINAPI RopAsyncDll::DllSendThdProc(LPVOID lpParameter)
{
    RopAsyncDll *pDll = (RopAsyncDll *)lpParameter;
    if(!pDll)
    {
        LDebug(DEBUG_L_EMERG,"SendThreadProc, Param error");
        return 0;
    }

    HANDLE hTable[2];
    hTable[0] = pDll->m_StopEvent;
    hTable[1] = pDll->m_SendEvent;

    LDebug(DEBUG_L_INFO,"module %S create data send thread", pDll->GetName());
    while(!pDll->m_bStopped)
    {
        try
        {			
            DWORD dwRet = WaitForMultipleObjects(2, hTable, FALSE, 2000);

            DWORD NewTickCount = 0;		
            NewTickCount = GetTickCount();
            if(NewTickCount - pDll->OldTickCount >= 15*1000)
            {
                //LDebug(DEBUG_L_INFO,"Module %S send thread running", pDll->GetName());
                pDll->OldTickCount = NewTickCount;

            }

            switch(dwRet)
            {
            case WAIT_TIMEOUT:
                {

                }
                break;
            case WAIT_OBJECT_0: //stop
                {
                    LDebug(DEBUG_L_EMERG,"Module(%s):STOP event received",pDll->GetName());
                    return 0;
                }
                break;
            case WAIT_OBJECT_0+1://data
                {
                    try
                    {
                        pDll->OnSend();
                    }
                    catch(...)
                    {
                        LDebug(DEBUG_L_WARNING,"Module(%s):Send Queue Exception",pDll->GetName());
                    }
                }
                break;
            default:
                break;
            }
        }catch(...)
        {
            LDebug(DEBUG_L_EMERG,"Module(%s):Send THREAD Exception",pDll->GetName());
        }
    }
    return 0;
}

int RopAsyncDll::Open(LPCTSTR  szFilePath, DWORD dwDesiredAccess)
{
    int dwRes = 0;
    if (m_Handle != (HANDLE)INVALID_SOCKET) {
        //throw RopAsyncDllException();
        LDebug(DEBUG_L_WARNING,"RopAsyncDll::Open,m_Handle Has value.");
    }
	
    MsgQ = new RopQueue(MAX_QUEUE_ITEM);
    if (!MsgQ) {
        return -1;
    }
	
	SendQ = new RopQueue(MAX_QUEUE_ITEM);
    if (!SendQ) {
        return -2;
    }	
	
    if (!(m_Handle = LoadLibrary(szFilePath))) {
        dwRes = -2;
        return dwRes;
    }
	
    Initialize = (DllInitialize)GetProcAddress((HMODULE )m_Handle,"Initialize");
    Finalize   = (DllFinalize)GetProcAddress((HMODULE )m_Handle,"Finalize");
    Info       = (DllInfo)GetProcAddress((HMODULE )m_Handle,"Info");
    MsgHandler = (DllMsgHandler)GetProcAddress((HMODULE )m_Handle,"MsgHandler");
    SetMsgCB   = (DllSetMsgCB)GetProcAddress((HMODULE )m_Handle, "SetMsgCB");
	
    if ((!Initialize) || (!Finalize) || (!Info) ||(!MsgHandler) ||(!SetMsgCB)) {
        dwRes = -3;
        return dwRes;
    }
    Initialize();
    PeerContext = SetMsgCB(RopAsyncDll::MsgCallback,(DWORD)this);
	LDebug(DEBUG_L_INFO,"Open Module (%S),Create thread .",szFilePath);
	CreateThread(NULL,0,DllSendThdProc,this,0,NULL);
	
    return dwRes;
}

void RopAsyncDll::Receive()
{
    try {
        if (!CheckAndDoShutdown()
			&&  (m_Handle != (HANDLE)INVALID_SOCKET)) {
            if (MsgQ->Count()>0) {
                SetEvent(EventHandle());
            }
        }
    } catch (...) {
        LDebug(DEBUG_L_EMERG,"dll %s receive exception\r\n",GetName());
    }
}

int RopAsyncDll::OnReceive()
{
    try {
        unsigned long Status = 0;
        EventReset();
        ROP_BUF* pTempMsg = (ROP_BUF*)MsgQ->Pop();		
		
        while (pTempMsg) {
            if (m_EventDispatcher) {
                m_EventDispatcher->OnEvent(this,GetPeer(),pTempMsg,EVENT_TYPE_DLLDATA_RECVED);
            }
            MemFree(pTempMsg,1);
            pTempMsg = (ROP_BUF*)MsgQ->Pop();
        }
        m_Buf.datal = 0;
    } catch (...) {
        LDebug(DEBUG_L_EMERG,"dll %s OnReceive exception\r\n",GetName());
    }
	
    return 1;
}

int RopAsyncDll::OnSend()
{
    try {
        unsigned long Status = 0;
        
        ROP_BUF* pTempMsg = (ROP_BUF*)SendQ->Pop();
		
		if (pTempMsg) {
			MsgHandler(PeerContext,pTempMsg->datap, pTempMsg->datal);
            MemFree(pTempMsg,1);            
        }
		
		if(SendQ->Count()<=0)
			ResetEvent(m_SendEvent);
        
    } catch (...) {
        LDebug(DEBUG_L_EMERG,"dll %s OnSend exception\r\n",GetName());
    }
	
    return 1;
}

int RopAsyncDll::Send (ROP_BUF& Buf)
{
    try {
        return Send((char*)Buf.datap,Buf.datal);
    } catch (...) {
        LDebug(DEBUG_L_EMERG,"dll %s Send exception\r\n",GetName());
    }
    return 0;
	
}

int RopAsyncDll::Send (char* buf,DWORD len)
{
    try {
        //MsgHandler(PeerContext,buf,len);
		if (!buf || len<=0)return -1;
        ROP_BUF* pTempMsg = (ROP_BUF*)MemAlloc(sizeof(ROP_BUF)+len,FALSE);
        if (!pTempMsg) {
            return -2;
        }
        pTempMsg->datal = len;
        pTempMsg->hdrp = pTempMsg->datap = pTempMsg->data = (unsigned char*)pTempMsg + sizeof(ROP_BUF);
        pTempMsg->hdrl = 0;
        pTempMsg->resved = 0;
        pTempMsg->size = len;
        pTempMsg->waitl  = len;
        memcpy(pTempMsg->datap,buf,len);
        if (!SendQ->Push(pTempMsg)) {
            try {
                MemFree(pTempMsg,1);
            } catch (...) {
            }
        }
        SetEvent(m_SendEvent);
		
    } catch (...) {
        LDebug(DEBUG_L_EMERG,"dll %s Send exception\r\n",GetName());
    }
    return len;
}
int RopAsyncDll::ReceiveAndWait(char* buf,DWORD len)
{
    return -1;
}

BOOL RopAsyncDll::CheckAndDoShutdown()
{
    BOOL bRet = FALSE;
    if (m_ShutdownIndicated
		&&(m_Handle != (HANDLE)NULL)) {
        try {
			SetEvent(m_StopEvent);
            Finalize();//howto wait it to end??
            if (m_Handle) {
                FreeLibrary((HINSTANCE)m_Handle);
                m_Handle = (HANDLE)NULL;
            }
            if (MsgQ) {
                ROP_BUF* pTempMsg = NULL;
                while (pTempMsg = (ROP_BUF*)MsgQ->Pop()) {
                    MemFree(pTempMsg,1);
                }
                delete MsgQ;
                MsgQ = NULL;
            }
			if (SendQ) {
                ROP_BUF* pTempMsg = NULL;
                while (pTempMsg = (ROP_BUF*)SendQ->Pop()) {
                    MemFree(pTempMsg,1);
                }
                delete SendQ;
                SendQ = NULL;
            }
			
            bRet = TRUE;
        } catch (...) {
        }
    }
    RopAsyncIO::CheckAndDoShutdown();
    return bRet;
}

DWORD RopAsyncDll::NotifyMsg(PVOID pInMsg, DWORD dwInLen)
{
    try {
        if (!pInMsg || dwInLen<=0)return -1;
        ROP_BUF* pTempMsg = (ROP_BUF*)MemAlloc(sizeof(ROP_BUF)+dwInLen,FALSE);
        if (!pTempMsg) {
            return -2;
        }
        pTempMsg->datal = dwInLen;
        pTempMsg->hdrp = pTempMsg->datap = pTempMsg->data = (unsigned char*)pTempMsg + sizeof(ROP_BUF);
        pTempMsg->hdrl = 0;
        pTempMsg->resved = 0;
        pTempMsg->size = dwInLen;
        pTempMsg->waitl  = dwInLen;
        memcpy(pTempMsg->datap,pInMsg,dwInLen);
        if (!MsgQ->Push(pTempMsg)) {
            try {
                MemFree(pTempMsg,1);
            } catch (...) {
            }
        }
        SetEvent(EventHandle());
    } catch (...) {
        LDebug(DEBUG_L_EMERG,"dll %s IndicateMsg exception\r\n",GetName());
    }
    return 0;
}


DWORD RopAsyncDll::MsgCallback(DWORD dwContext,PVOID pInMsg,DWORD dwInLen)
{
    ModMsg* pMsg = (ModMsg* )pInMsg;
    RopAsyncDll* pThis = (RopAsyncDll*)dwContext;
    /* dwContext is this* pointer which pass while dll calling setcb() */
    /* For static function hasn't this* pointer*/
	
    if (!pThis) {
        return -1;
    }
	
    try {
        return pThis->NotifyMsg(pInMsg,dwInLen);
    } catch (...) {
        LDebug(DEBUG_L_EMERG,"dll %s MsgCallback exception\r\n",pThis->GetName());
    }
    return 0;
}

void RopAsyncDll::TimeoutEvent()
{
    try {
		DWORD NewTickCount = 0;		
		NewTickCount = GetTickCount();
		if(NewTickCount - OldTickCount_OBJ >= 30*1000)
		{
			OldTickCount_OBJ = NewTickCount;
			//LDebug(DEBUG_L_INFO,"module %S timeout & running", GetName());
		}
    } catch (...) {
        LDebug(DEBUG_L_EMERG,"dll %s TimeoutEvent exception\r\n",GetName());
    }
}
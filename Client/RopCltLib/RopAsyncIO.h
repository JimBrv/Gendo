#ifndef ROP_ASYNCIO_H
#define ROP_ASYNCIO_H

#include "stdafx.h"
#include "winsock2.h"

#include "windows.h"


#include "RopBase.h"
#include "RopBuf.h"
#pragma warning(disable:4290 4200)

#define  EVENT_TYPE_SOCKETDATA_RECVED  0x1
#define  EVENT_TYPE_DEVICEDATA_RECVED  0x2
#define  EVENT_TYPE_DLLDATA_RECVED     0x11
#define  EVENT_TYPE_SOCKET_ERROR       0x3
#define  EVENT_TYPE_SOCKET_ACCEPTED    0x4
#define  EVENT_TYPE_LSNSOCKET_ERROR    0x5
#define  EVENT_TYPE_SOCKET_CLOSED      0x6
#define  EVENT_TYPE_DEVICE_ERROR       0x7
#define  EVENT_TYPE_TIMER_RECVED       0x8

#define  EVENT_TYPE_DLLDATA_ERROR      0x22

#define EVENT_TYPE_MONDATA_RECVED     0x31

typedef unsigned char MACADDR[6];


class RopEventDispatcher;

class RopAsyncIOException
{
};

class RopAsyncIO : public RopBase
{
protected:  
	int     m_Status;
	DWORD   m_Flags;
	MACADDR m_MacAddr;
    TCHAR   m_Name[24];  
public:
	unsigned char* GetMac()	{return m_MacAddr;};
    const TCHAR  *GetName() {return m_Name;}
    void    SetName(TCHAR *sName) {lstrcpyn(m_Name, sName, 24);}

public: 
	WSAOVERLAPPED m_OlRead, m_OlWrite;//设置叠成异步套接字用到。
	__int64 m_TxErr, m_RxErr, m_Tx, m_Rx;
	BOOL m_ShutdownIndicated;
	RopAsyncIO* m_Peer;
	int m_Sync;
	ROP_BUF m_Buf;
    BOOL m_bIsOpened;
	
public:    
	BOOL m_bBlocked;
	PVOID	m_pContext;	
	RopEventDispatcher* m_EventDispatcher;
	HANDLE m_Handle;
	RopAsyncIO() throw (RopAsyncIOException);
	virtual ~RopAsyncIO() ;

	virtual int Open(LPCTSTR szResource, DWORD dwAttr)= 0;
	
public:    
	virtual __int64 TxErr()  {return m_TxErr;}
	virtual __int64 RxErr()  {return m_RxErr;}
	virtual __int64 Tx()     {return m_Tx;}
	virtual __int64 Rx()     {return m_Rx;}
	
public:    
	virtual BOOL CancelPendingIo (){return CancelIo(m_Handle);};
	virtual int Send (ROP_BUF& Buf){return 0;};
	virtual int Send (char* buf,DWORD len){return 0;};

	virtual int SendEx (ROP_BUF& Buf,DWORD dwTimeOut = INFINITE){return 0;};
	virtual int SendEx (char* buf,DWORD len,DWORD dwTimeOut = INFINITE){return 0;};

	
public:    
	virtual RopAsyncIO* Accept(){return NULL;};
	virtual void Receive() {};
	virtual int OnReceive(){return 0;};
	virtual int ReceiveAndWaitEx(char* buf,DWORD len,DWORD dwTimeOut = INFINITE){return 0;};
	virtual int ReceiveAndWait(char* buf,DWORD len){return 0;};
	virtual BOOL CheckAndDoShutdown();
	virtual void TimeoutEvent(){}
	virtual void Shutdown();
    virtual BOOL ShutdownSilent() {m_bIsOpened = FALSE;return false;};
	
public:    
	void EventReset();
	inline void SetPeer(RopAsyncIO* Peer){m_Peer = Peer;}
	inline RopAsyncIO* GetPeer(){return m_Peer;}
	inline void SetEventDispatcher(RopEventDispatcher* Dispatcher){m_EventDispatcher = Dispatcher;}
	
public:   
	HANDLE &EventHandle()       {return m_OlRead.hEvent;}
	WSAOVERLAPPED &Overlapped() {return m_OlRead;}
};

#endif
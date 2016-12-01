#include "stdafx.h"
#include <stdlib.h>
#include <string.h>

#include "RopUtil.h"
#include "RopAsyncIO.h"
#include "RopAsyncSocket.h"
#include "RopEventDispatcher.h"
#include "RopProxy.h"
#include "lib-msg.h"
//异步套接字的构造。
RopAsyncSocket::RopAsyncSocket ()
{
    m_Handle = (HANDLE)INVALID_SOCKET;
    m_ProxyParam.Type = PROXY_TYPE_NOPROXY;
    m_ProxyParam.Host[0]=0;
    m_ProxyParam.Port = 1080;
    m_ProxyParam.User[0] = 0;
    m_ProxyParam.Pass[0] = 0;
    m_ProxyParam.NeedPass = 0;

}
//设置代理，以填充m_ProxyParam结构体。
void RopAsyncSocket::SetProxy(int Type,char* Host,int Port,int NeedPass=0,char* User=NULL,char* Pass=NULL)
{

	char ipaddr[PROXY_NAME_LEN];
	ipaddr[0] = 0;
	strncpy(ipaddr,Host,PROXY_NAME_LEN);	
	
	if(INADDR_NONE == inet_addr(ipaddr))//is valid ip address?
	{		
		struct in_addr in;
		hostent* pHost = gethostbyname(Host);//do dns resolvtion
		if(!pHost)
		{		
			Dbg("RopAsyncSocket::SetProxy,DNS fail,gethostbyname\n");
			return ;
		}
		int iaddr = *(int*)pHost->h_addr_list[0];
		
		in.S_un.S_addr = (iaddr);
		char * pAddr = inet_ntoa(in);//convert ip to string
		if(pAddr)
		{				
			strncpy(ipaddr,pAddr,PROXY_NAME_LEN);			
		}
		else 
		{	
			Dbg("RopAsyncSocket::SetProxy,DNS fail,inet_ntoa\n");
			return;
		}
	}	

	Dbg("RopAsyncSocket::SetProxy %s(%s):%d\n",Host,ipaddr,Port);
    m_ProxyParam.Type = Type;
    strncpy(m_ProxyParam.Host,ipaddr,PROXY_NAME_LEN);
    m_ProxyParam.Port = Port;
    m_ProxyParam.NeedPass = NeedPass;
    if (NeedPass) {
        if (User)strncpy(m_ProxyParam.User,User,PROXY_NAME_LEN);
        if (Pass)strncpy(m_ProxyParam.Pass,Pass,PROXY_NAME_LEN);
    }
}
//连接代理服务。
int RopAsyncSocket::OpenNoProxy(LPCTSTR szServer, DWORD dwPort)
{
    int dwRes = 0;
    IN_ADDR     RemoteIpAddress;
    SOCKADDR_IN m_remoteAddr;
    CHAR        szServerIP[128] = {0};
    int         iLen = 128;

    wc2mb((LPWSTR)szServer, wcslen(szServer), szServerIP, &iLen);

    if(INADDR_NONE == inet_addr(szServerIP))//is valid ip address?
    {		
        struct in_addr in;
        hostent* pHost = gethostbyname(szServerIP);//do dns resolvtion
        if(!pHost)
        {		
            Dbg("DNS fail,gethostbyname\n");
            return -1;
        }
        int iaddr = *(int*)pHost->h_addr_list[0];

        in.S_un.S_addr = (iaddr);
        char *pAddr = inet_ntoa(in);//convert ip to string
        if(pAddr)
        {				
            strncpy(szServerIP, pAddr, PROXY_NAME_LEN);			
        }
        else 
        {	
            Dbg("DNS fail,inet_ntoa\n");
            return -1;
        }
    }

    ZeroMemory(&m_remoteAddr, sizeof (m_remoteAddr));
    m_remoteAddr.sin_family = AF_INET;
    m_remoteAddr.sin_port = htons((WORD)dwPort);
    RemoteIpAddress.s_addr = inet_addr(szServerIP);
    m_remoteAddr.sin_addr = RemoteIpAddress;
    if (connect((SOCKET)m_Handle, (PSOCKADDR)&m_remoteAddr, sizeof(SOCKADDR_IN)) == 0 ) {
        BOOL opt = TRUE;
        if (setsockopt((SOCKET)m_Handle,IPPROTO_TCP,TCP_NODELAY,(char*)&opt,sizeof(opt))!=0) {
            Dbg("setsockopt fail\n");
        }
        dwRes = 0;
    } else {
        Dbg("OpenNoProxy %s:%d fail,%d\n",szServerIP,dwPort,WSAGetLastError());
        closesocket((SOCKET)m_Handle);
        m_Handle = (HANDLE)INVALID_SOCKET;
        dwRes = -1;
    }
    return dwRes;
}

int RopAsyncSocket::OpenSocks4(LPCTSTR szServer, DWORD dwPort)
{
    int dwRes = 0;
    IN_ADDR RemoteIpAddress;
    SOCKADDR_IN m_remoteAddr;
    int  iLen = 128;
    CHAR szServerIP[128] = {0};

    wc2mb((LPWSTR)szServer, wcslen(szServer), szServerIP, &iLen);

    if(INADDR_NONE == inet_addr(szServerIP))//is valid ip address?
    {		
        struct in_addr in;
        hostent* pHost = gethostbyname(szServerIP);//do dns resolvtion
        if(!pHost)
        {		
            Dbg("DNS fail,gethostbyname\n");
            return -1;
        }
        int iaddr = *(int*)pHost->h_addr_list[0];

        in.S_un.S_addr = (iaddr);
        char *pAddr = inet_ntoa(in);//convert ip to string
        if(pAddr)
        {				
            strncpy(szServerIP, pAddr, PROXY_NAME_LEN);			
        }
        else 
        {	
            Dbg("DNS fail,inet_ntoa\n");
            return -1;
        }
    }

    ZeroMemory(&m_remoteAddr, sizeof (m_remoteAddr));
    m_remoteAddr.sin_family = AF_INET;
    m_remoteAddr.sin_port = htons((WORD)m_ProxyParam.Port);
    RemoteIpAddress.s_addr = inet_addr(m_ProxyParam.Host);
    m_remoteAddr.sin_addr = RemoteIpAddress;
    if (connect((SOCKET)m_Handle, (PSOCKADDR)&m_remoteAddr, sizeof(SOCKADDR_IN)) == 0 ) {
        BOOL opt = TRUE;
        if (setsockopt((SOCKET)m_Handle,IPPROTO_TCP,TCP_NODELAY,(char*)&opt,sizeof(opt))!=0) {
            Dbg("setsockopt fail\n");
        }
        dwRes = doSock4Connect((SOCKET)m_Handle, &m_ProxyParam, szServerIP, dwPort);
        if (dwRes < 0) {
            closesocket((SOCKET) m_Handle);
            m_Handle = (HANDLE)INVALID_SOCKET;
        }

    } else {
        closesocket((SOCKET)m_Handle);
        m_Handle = (HANDLE)INVALID_SOCKET;
        dwRes = -1;
    }
    return dwRes;
}

int RopAsyncSocket::OpenSocks5(LPCTSTR szServer, DWORD dwPort)
{
    int dwRes = 0;
    IN_ADDR RemoteIpAddress;
    SOCKADDR_IN m_remoteAddr;
    CHAR szServer2[128] = {0};
    int  iLen = 128;
    wc2mb((LPWSTR)szServer, wcslen(szServer), szServer2, &iLen);

    ZeroMemory(&m_remoteAddr, sizeof (m_remoteAddr));
    m_remoteAddr.sin_family = AF_INET;
    m_remoteAddr.sin_port = htons((WORD)m_ProxyParam.Port);
    RemoteIpAddress.s_addr = inet_addr(m_ProxyParam.Host);
    m_remoteAddr.sin_addr = RemoteIpAddress;
    if (connect((SOCKET)m_Handle, (PSOCKADDR)&m_remoteAddr, sizeof(SOCKADDR_IN)) == 0 ) {
        BOOL opt = TRUE;
        if (setsockopt((SOCKET)m_Handle,IPPROTO_TCP,TCP_NODELAY,(char*)&opt,sizeof(opt))!=0) {
            Dbg("setsockopt fail\n");
        }

        dwRes = doSock5Connect((SOCKET)m_Handle, &m_ProxyParam, szServer2, dwPort);
        if (dwRes < 0) {
            closesocket((SOCKET) m_Handle);
            m_Handle = (HANDLE)INVALID_SOCKET;
        }

    } else {
        closesocket((SOCKET)m_Handle);
        m_Handle = (HANDLE)INVALID_SOCKET;
        dwRes = -1;
    }
    return dwRes;
}

int RopAsyncSocket::OpenHttp11(LPCTSTR szServer, DWORD dwPort)
{
    int dwRes = 0;
    IN_ADDR RemoteIpAddress;
    SOCKADDR_IN m_remoteAddr;
    CHAR szServer2[128] = {0};
    int  iLen = 128;
    wc2mb((LPWSTR)szServer, wcslen(szServer), szServer2, &iLen);
    ZeroMemory(&m_remoteAddr, sizeof (m_remoteAddr));
    m_remoteAddr.sin_family = AF_INET;
    m_remoteAddr.sin_port = htons((WORD)m_ProxyParam.Port);
    RemoteIpAddress.s_addr = inet_addr(m_ProxyParam.Host);
    m_remoteAddr.sin_addr = RemoteIpAddress;
    if (connect((SOCKET)m_Handle, (PSOCKADDR)&m_remoteAddr, sizeof(SOCKADDR_IN)) == 0 ) {
        BOOL opt = TRUE;
        if (setsockopt((SOCKET)m_Handle,IPPROTO_TCP,TCP_NODELAY,(char*)&opt,sizeof(opt))!=0) {
            Dbg("setsockopt fail\n");
        }
        dwRes = doHttp11Connect((SOCKET)m_Handle, &m_ProxyParam, szServer2, dwPort);
        if (dwRes < 0) {
            closesocket((SOCKET) m_Handle);
            m_Handle = (HANDLE)INVALID_SOCKET;
        }

    } else {
        closesocket((SOCKET)m_Handle);
        m_Handle = (HANDLE)INVALID_SOCKET;
        dwRes = -1;
    }
    return dwRes;
}

RopAsyncSocket::~RopAsyncSocket()
{
    try {
        closesocket((SOCKET)m_Handle);
        m_Handle = (HANDLE)INVALID_SOCKET;
    } catch (...) {
    }
}

int RopAsyncSocket::Open(LPCTSTR szServer, DWORD dwPort)
{
    int dwRes = 0;
    if (m_Handle != (HANDLE)INVALID_SOCKET) {
        throw RopAsyncSocketException();
    }
    if (dwRes == 0) {
        SOCKET s = INVALID_SOCKET;
        if (dwRes == 0) {
            GROUP g = 0;
            s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, g, WSA_FLAG_OVERLAPPED);
            if (s == INVALID_SOCKET) {
                dwRes = (DWORD)WSAGetLastError();
                throw RopAsyncSocketException();
            }
        }
        
        if (dwRes == 0) {
            m_Handle = (HANDLE)s;
            switch (m_ProxyParam.Type) {
                case PROXY_TYPE_NOPROXY:
                    dwRes = OpenNoProxy(szServer,dwPort);
                    break;
                case PROXY_TYPE_SOCKS4:
                case PROXY_TYPE_SOCKS4A:
                    dwRes = OpenSocks4(szServer,dwPort);
                    break;
                case PROXY_TYPE_SOCKS5:
                    dwRes = OpenSocks5(szServer,dwPort);
                    break;
                case PROXY_TYPE_HTTP11:
                    dwRes = OpenHttp11(szServer,dwPort);
                    break;
                default:
                    dwRes = OpenNoProxy(szServer,dwPort);
                    break;
            }

        }
    }
    m_bIsOpened = true;
    return dwRes;
}
void RopAsyncSocket::Receive()
{
    if (! CheckAndDoShutdown()
        && (m_Handle != (HANDLE)INVALID_SOCKET)) {
        DWORD m_Flags = 0;
        DWORD dwReceived = 0;
        WSABUF wsabuf;
        m_Status = 0;

        wsabuf.buf = (char *) (m_Buf.datap)+m_Buf.datal;
        wsabuf.len = m_Buf.size-m_Buf.datal;

        if (m_Sync = (WSARecv((SOCKET)m_Handle, &wsabuf, 1, &dwReceived, &m_Flags, &m_OlRead, NULL) != SOCKET_ERROR)) {

            m_Buf.datal+=dwReceived;
            SetEvent(m_OlRead.hEvent);
        }

        if ((!m_Sync) && (WSAGetLastError() != WSA_IO_PENDING)) {
            m_Status = WSAGetLastError ();
            Dbg ("[SOCKET]Failed on pending read attempt, error=%d\n",m_Status);
            SetEvent(m_OlRead.hEvent);
        }
    }

}

int RopAsyncSocket::OnReceive()
{
    unsigned long Status = 0;
    if (!m_Sync) {
        if (m_Status) { //error;
            if (m_EventDispatcher)
			{
				Dbg ("[SOCKET]OnReceive failed(1)\n");
                m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf,EVENT_TYPE_SOCKET_ERROR);
			}
            EventReset();
            return -1;
        } else {
            DWORD Transfered = 0;
            if (!WSAGetOverlappedResult((SOCKET)m_Handle,&m_OlRead,&Transfered,FALSE,&m_Flags)) {
                m_Status = WSAGetLastError();

                if (m_EventDispatcher)
				{
					Dbg ("[SOCKET]OnReceive failed(2),%d\n",m_Status);
                    m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf,EVENT_TYPE_SOCKET_ERROR);
				}
                EventReset();
                return -1;
            }

            if (Transfered==0) {
                if (m_EventDispatcher) {
					//Dbg ("[SOCKET]OnReceive failed(3)\n");
                    m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf,EVENT_TYPE_SOCKET_ERROR);
                }
                EventReset();
                return 0;
            }
        }

        m_Buf.datal += m_OlRead.InternalHigh;
        Status = m_OlRead.Internal;
    }

    if (m_Buf.datal <=0) {
        if (m_EventDispatcher)
		{
            m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf,EVENT_TYPE_SOCKET_ERROR);
		}

        EventReset();
        m_Rx+=m_Buf.datal;
        m_Buf.datal = 0;

        return 0;
    } else {
        if (m_EventDispatcher) {
            m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf,EVENT_TYPE_SOCKETDATA_RECVED);
        }
    }
    EventReset();
    m_Rx+=m_Buf.datal;
    m_Buf.datal = 0;
    return 1;
}

int RopAsyncSocket::Send(ROP_BUF& Buf)
{
	/*
    int Written = 0, Flags = 0;
    WSABUF wsabuf;

    if ((Buf.datal>0)
            &&  (m_Handle != (HANDLE)INVALID_SOCKET)) {
        wsabuf.buf = (char*)Buf.datap;
        wsabuf.len = Buf.datal;
        if (WSASend ((SOCKET)m_Handle, &wsabuf, 1, (unsigned long*)&Written, 0, &m_OlWrite, NULL) != SOCKET_ERROR)
            ;
        else if (WSAGetLastError() == WSA_IO_PENDING)
            WSAGetOverlappedResult ((SOCKET)m_Handle, &m_OlWrite, (unsigned long*)&Written, TRUE, (unsigned long*)&Flags);
        else {
            Written = -1;
            Dbg ("[SOCKET]RopAsyncSocket failed on write attempt\n");
        }

        ResetEvent (m_OlWrite.hEvent);
    }
    m_Tx+=Written;
    return Written;
	*/
	return Send((char*)Buf.datap,Buf.datal);

}
int RopAsyncSocket::Send(char* buf,DWORD len)
{

    int Written = 0, Flags = 0;
    WSABUF wsabuf;

    if (buf && (len>0) && (m_Handle != (HANDLE)INVALID_SOCKET)) {
        wsabuf.buf = (char*)buf;
        wsabuf.len = len;
        if (WSASend ((SOCKET)m_Handle, &wsabuf, 1,(unsigned long*)&Written, 0, &m_OlWrite, NULL) != SOCKET_ERROR)
            ;
        else if (WSAGetLastError() == WSA_IO_PENDING)
            WSAGetOverlappedResult ((SOCKET)m_Handle, &m_OlWrite, (unsigned long*)&Written, TRUE, (unsigned long*)&Flags);
        else {
            Written = -1;
            Dbg ("[SOCKET]RopAsyncSocket failed on write attempt\n");
        }

        ResetEvent (m_OlWrite.hEvent);
    }
    if(Written>0)
		m_Tx+=Written;
    return Written;

}

int RopAsyncSocket::SendEx(ROP_BUF& Buf,DWORD dwTimeOut)
{
	return SendEx((char*)Buf.datap,Buf.datal,dwTimeOut);
}
int RopAsyncSocket::SendEx (char* buf,DWORD len,DWORD dwTimeOut)
{
    int Written = 0, Flags = 0;
    WSABUF wsabuf;
    if (buf && (len > 0) && (m_Handle != (HANDLE)INVALID_SOCKET)) {
        wsabuf.buf = (char*)buf;
        wsabuf.len = len;
		PModMsg pMod = (PModMsg)buf;
		pmsg_head_t pMsg = (pmsg_head_t)pMod->payload;
		//Dbg("AsyncSocket => Send, byte=%d, (%x->%x), tag=%x, msg=%x, len=%d\n", 
		//	len, pMod->sModule, pMod->dModule, pMod->tag, pMsg->msg, pMsg->len);

        if (WSASend ((SOCKET)m_Handle, &wsabuf, 1,(unsigned long*)&Written, 0, &m_OlWrite, NULL) != SOCKET_ERROR) {
            
        } else if (WSAGetLastError() == WSA_IO_PENDING) {
			DWORD dwWait = WaitForSingleObject(m_OlWrite.hEvent,dwTimeOut);
			switch(dwWait)
			{
			case WAIT_OBJECT_0:
				{
					if(!WSAGetOverlappedResult ((SOCKET)m_Handle, &m_OlWrite, (unsigned long*)&Written, FALSE, (unsigned long*)&Flags))
					{
						Written = (-1)*WSAGetLastError();
					}
				}
				break;
			case WAIT_TIMEOUT:
				{
					Written = (-1)*WAIT_TIMEOUT;
					m_bBlocked = TRUE;
				}
				break;
			default:
				break;
			}            
		} else {
            Written = -1;
            Dbg ("[SOCKET]RopAsyncSocket failed on write attempt\n");
        }

        ResetEvent (m_OlWrite.hEvent);
    }
	if(Written>0)
		m_Tx+=Written;
    return Written;

}

int RopAsyncSocket::ReceiveAndWaitEx(char* buf,DWORD len,DWORD dwTimeOut)
{
    int ret = 0;
    if (! CheckAndDoShutdown()
            &&  (m_Handle != (HANDLE)INVALID_SOCKET)) {
        DWORD Flags = 0;
        WSABUF wsabuf;
        wsabuf.buf = (char *)buf;
        wsabuf.len = len;

        if (WSARecv((SOCKET)m_Handle, &wsabuf, 1, (unsigned long*)&ret, (unsigned long*)&Flags, &m_OlRead, NULL) != SOCKET_ERROR)
            ;
        else if (WSAGetLastError() == WSA_IO_PENDING)//will block
		{
			DWORD dwWait = WaitForSingleObject(m_OlRead.hEvent,dwTimeOut);
			switch(dwWait)
			{
			case WAIT_OBJECT_0:
				{
					if(!WSAGetOverlappedResult ((SOCKET)m_Handle, &m_OlRead, (unsigned long*)&ret, FALSE, (unsigned long*)&Flags))
					{
						ret = (-1)*WSAGetLastError();
					}
				}
				break;
			case WAIT_TIMEOUT:
				ret = (-1)*WAIT_TIMEOUT;
				break;
			default:
				break;
			}
		}
        else {
            ret = -1;
            Dbg ("[SOCKET]failed on read attempt\n");
        }

        ResetEvent (m_OlRead.hEvent);
    }
    return ret;
}

int RopAsyncSocket::ReceiveAndWait(char* buf,DWORD len)
{
    int ret = 0;
    if (! CheckAndDoShutdown()
            &&  (m_Handle != (HANDLE)INVALID_SOCKET)) {
        DWORD Flags = 0;
        WSABUF wsabuf;
        wsabuf.buf = (char *)buf;
        wsabuf.len = len;

        if (WSARecv((SOCKET)m_Handle, &wsabuf, 1, (unsigned long*)&ret, (unsigned long*)&Flags, &m_OlRead, NULL) != SOCKET_ERROR)
            ;
        else if (WSAGetLastError() == WSA_IO_PENDING)//will block
            WSAGetOverlappedResult ((SOCKET)m_Handle, &m_OlRead, (unsigned long*)&ret, TRUE, (unsigned long*)&Flags);
        else {
            ret = -1;
            Dbg ("[SOCKET]failed on read attempt\n");
        }

        ResetEvent (m_OlRead.hEvent);
    }
    return ret;
}

BOOL RopAsyncSocket::CheckAndDoShutdown()
{
    BOOL bRet = FALSE;
    if (m_ShutdownIndicated
            &&(m_Handle != (HANDLE)INVALID_SOCKET)) {
        shutdown((SOCKET)m_Handle,SD_BOTH );
        closesocket((SOCKET)m_Handle);
        m_Handle = (HANDLE)INVALID_SOCKET;
        m_bIsOpened = FALSE;
        bRet = TRUE;
    }
    RopAsyncIO::CheckAndDoShutdown();
    return bRet;
}

BOOL RopAsyncSocket::ShutdownSilent()
{
    BOOL bRet = FALSE;
    if (/*m_ShutdownIndicated && */(m_Handle != (HANDLE)INVALID_SOCKET)) {
            shutdown((SOCKET)m_Handle,SD_BOTH );
            closesocket((SOCKET)m_Handle);
            m_Handle = (HANDLE)INVALID_SOCKET;
            bRet = TRUE;
            m_bIsOpened = FALSE;
    }
    return bRet;
}

//base64 encode
int RopAsyncSocket::to64frombits(unsigned char *out, const unsigned char *in, int inlen)
{
    const char base64digits[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (; inlen >= 3; inlen -= 3) {
        *out++ = base64digits[in[0] >> 2];
        *out++ = base64digits[((in[0] << 4) & 0x30)|(in[1] >> 4)];
        *out++ = base64digits[((in[1] << 2) & 0x3c)|(in[2] >> 6)];
        *out++ = base64digits[in[2] & 0x3f];
        in += 3;
    }
    if (inlen > 0) {
        unsigned char fragment;
        *out++ = base64digits[in[0] >> 2];
        fragment = (in[0] << 4) & 0x30;
        if (inlen > 1)
            fragment |= in[1] >> 4;
        *out++ = base64digits[fragment];
        *out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
        *out++ = '=';
    }
    *out = NULL;
    return 0;
}



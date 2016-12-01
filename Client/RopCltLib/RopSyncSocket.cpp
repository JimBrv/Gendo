/*
 * Synchoronize socket.
 * Note: Don't use it in Executer-Dispatcher pattern!!!!!
 *       It will block the whole process for it's blocking.
 */
#include "stdafx.h"
#include "RopSyncSocket.h"
#include <winsock.h>
#include "RopUtil.h"
#include "RopPing.h"

RopSyncSocket::RopSyncSocket()
{
    m_Handle = (HANDLE)INVALID_SOCKET;
    
}

RopSyncSocket::~RopSyncSocket()
{
    if ((SOCKET)m_Handle != INVALID_SOCKET) closesocket((SOCKET)m_Handle);
}

void RopSyncSocket::Receive()
{
    return;
}

int  RopSyncSocket::OnReceive()
{
    LDebug(DEBUG_L_WARNING, "Sync-Blocking socket doesnt support OnReceive() operation...!!!\n");
    return -1;
}

int  RopSyncSocket::ReceiveAndWait(char* buf,DWORD len)
{
    int ret = -1;
    if (buf && len > 0 && (SOCKET)m_Handle != INVALID_SOCKET) {
        ret = ::recv((SOCKET)m_Handle, buf, len, 0);
        if (ret == 0) {
            LDebug(DEBUG_L_WARNING, "Sync socket recving, peer closed.\n");  
            return 0;
        }
        if(ret == SOCKET_ERROR) {  
			if(WSAETIMEDOUT == ::WSAGetLastError()) {  
                //LDebug(DEBUG_L_WARNING, "Sync socket recv timout\n");  
                return (-1)*WSAETIMEDOUT;
			}else if(WSAEINTR == ::WSAGetLastError()){
				//LDebug(DEBUG_L_WARNING, "Sync socket recv get error = %d\n", ::WSAGetLastError());
				return (-1)*WSAEINTR;
			}else if (WSAECONNRESET == ::WSAGetLastError()){
				LDebug(DEBUG_L_WARNING, "Sync socket recv get error = %d\n", ::WSAGetLastError());
				return (-1)*WSAECONNRESET;
			}else{
                LDebug(DEBUG_L_WARNING, "Sync socket recv get error = %d\n", ::WSAGetLastError());  
                return SOCKET_ERROR;
            } 
        }
        m_Rx += ret;
    }    
    return ret;
}
int  RopSyncSocket::ReceiveAndWaitEx(char* buf,DWORD len,DWORD dwTimeOut)
{
    int ret = -1;
    if (buf && len > 0 && (SOCKET)m_Handle != INVALID_SOCKET) {
        if (dwTimeOut != INFINITE) {
            SetTimeout((SOCKET)m_Handle, dwTimeOut, TRUE);
        }        
        ret = ::recv((SOCKET)m_Handle, buf, len, 0);        
        if (ret == 0) {
            LDebug(DEBUG_L_WARNING, "Sync socket recving, peer closed.\n");  
            return 0;
        }
        if(ret == SOCKET_ERROR) {  
            if(::WSAGetLastError() == WSAETIMEDOUT) {  
                //LDebug(DEBUG_L_WARNING, "Sync socket recv time (%d) out\n", dwTimeOut);  
                return (-1)*WSAETIMEDOUT;
            }else{
                LDebug(DEBUG_L_WARNING, "Sync socket recv get error=%d\n", ::WSAGetLastError());  
                return SOCKET_ERROR;
            } 
        }
        m_Rx += ret;
    }    
    return ret;
}

BOOL RopSyncSocket::CheckAndDoShutdown()
{   
    BOOL bRet = FALSE;
    if (m_ShutdownIndicated &&(m_Handle != (HANDLE)INVALID_SOCKET)) {
            shutdown((SOCKET)m_Handle,SD_BOTH );
            closesocket((SOCKET)m_Handle);
            m_Handle = (HANDLE)INVALID_SOCKET;
            m_bIsOpened = FALSE;
            bRet = TRUE;
    }
    RopAsyncIO::CheckAndDoShutdown();
    return bRet;
}

BOOL RopSyncSocket::ShutdownSilent()
{
    if ((SOCKET)m_Handle != INVALID_SOCKET) {
        shutdown((SOCKET)m_Handle,SD_BOTH );
        closesocket((SOCKET)m_Handle);
        m_Handle = (HANDLE)INVALID_SOCKET;
        m_bIsOpened = FALSE;
    }
    return true;
}
/* Sync socket open connect */
int  RopSyncSocket::Open(LPCTSTR szServer, DWORD dwPort)
{
    int dwRes = 0;

    if (m_Handle != (HANDLE)INVALID_SOCKET) {
        throw RopSyncSocketException();
    }

    /* Sync, Blocking socket as the title */
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == s) {
        LDebug(DEBUG_L_ERROR, "cannot create new socket, error=%d!", WSAGetLastError());
        return -1;
    }
    m_Handle = (HANDLE)s;

    IN_ADDR     RemoteIpAddress;
    SOCKADDR_IN m_remoteAddr;
    CHAR szServerIP[128] = {0};
    int  iLen = 128;

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
            Dbg("DNS Resolve:'%s => %s'\n", szServerIP, pAddr);
            strncpy(szServerIP, pAddr, 64);
            
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
        m_bIsOpened = true;
    } else {
        Dbg("Connect %s:%d fail,%d\n", szServerIP, dwPort, WSAGetLastError());
        closesocket((SOCKET)m_Handle);
        m_Handle = (HANDLE)INVALID_SOCKET;
        dwRes = -1;
    }
   
    return dwRes;
}

#if 0
/* Sync socket open connect */
int  RopSyncSocket::OpenAnsi(char *szServer, DWORD dwPort)
{
    int dwRes = 0;

    if (m_Handle != (HANDLE)INVALID_SOCKET) {
        throw RopSyncSocketException();
    }

    /* Sync, Blocking socket as the title */
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (INVALID_SOCKET == s) {
        LDebug(DEBUG_L_ERROR, "cant new socket, error=%d!", WSAGetLastError());
        return -1;
    }
    m_Handle = (HANDLE)s;

    IN_ADDR RemoteIpAddress;
    SOCKADDR_IN m_remoteAddr;

    ZeroMemory(&m_remoteAddr, sizeof (m_remoteAddr));
    m_remoteAddr.sin_family = AF_INET;
    m_remoteAddr.sin_port = htons((WORD)dwPort);
    RemoteIpAddress.s_addr = inet_addr(szServer);
    m_remoteAddr.sin_addr = RemoteIpAddress;

    if (connect((SOCKET)m_Handle, (PSOCKADDR)&m_remoteAddr, sizeof(SOCKADDR_IN)) == 0 ) {
        BOOL opt = TRUE;
        if (setsockopt((SOCKET)m_Handle,IPPROTO_TCP,TCP_NODELAY,(char*)&opt,sizeof(opt))!=0) {
            Dbg("setsockopt fail\n");
        }
        dwRes = 0;
        m_bIsOpened = true;
    } else {
        Dbg("Connect %s:%d fail,%d\n", szServer, dwPort, WSAGetLastError());
        closesocket((SOCKET)m_Handle);
        m_Handle = (HANDLE)INVALID_SOCKET;
        dwRes = -1;
    }
    return dwRes;
}

#endif

int  RopSyncSocket::Send (ROP_BUF& Buf)
{
    return Send((char *)Buf.datap, Buf.datal);
}
int  RopSyncSocket::Send (char* buf, DWORD len)
{
	int ret = 0;
    int Written = 0;
    if (buf && (len>0) && (m_Handle != (HANDLE)INVALID_SOCKET)) {
		Written = ::send((SOCKET)m_Handle, buf, len, 0);
        if (Written == SOCKET_ERROR) {
           ret = -1;
           LDebug(DEBUG_L_ERROR, "[SOCKET] Sync Socket failed on send, error=%d\n",WSAGetLastError());
        }
        //ResetEvent (m_OlWrite.hEvent);
		LDebug(DEBUG_L_INFO,"Send Count = %d\n",Written);
    }
    if(Written > 0)
        m_Tx+=Written;
    return ret;
}

int  RopSyncSocket::SendEx (ROP_BUF& Buf,DWORD dwTimeOut)
{
    return SendEx((char*)Buf.datap, Buf.datal, dwTimeOut);
}
int  RopSyncSocket::SendEx (char* buf,DWORD len,DWORD dwTimeOut)
{
    int ret = -1;
    if (buf && len > 0 && (SOCKET)m_Handle != INVALID_SOCKET) {
        if (dwTimeOut == INFINITE) {
            SetTimeout((SOCKET)m_Handle, dwTimeOut, FALSE);
        }        
        ret = ::send((SOCKET)m_Handle, buf, len, 0);

        if(ret == SOCKET_ERROR) {  
            if(::WSAGetLastError() == WSAETIMEDOUT) {  
                LDebug(DEBUG_L_WARNING, "Sync socket send time(%d) out\n", dwTimeOut);  
                return (-1)*WSAETIMEDOUT;
            }else{
                LDebug(DEBUG_L_WARNING, "Sync socket send get error=%d\n", ::WSAGetLastError());  
                return SOCKET_ERROR;
            } 
        }
    }    
    return ret;
}

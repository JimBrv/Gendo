#include "stdafx.h"
#include "RopAsyncAcceptor.h"


RopAsyncAcceptor::RopAsyncAcceptor()
{
    m_Handle = (HANDLE)INVALID_SOCKET;
    m_pClient = NULL;
    m_MaxClient = 32;
    m_CurClient = 0;
    ZeroMemory(m_sPortRegPath, sizeof(m_sPortRegPath));
    ZeroMemory(m_sPortRegName, sizeof(m_sPortRegName));
}

RopAsyncAcceptor::RopAsyncAcceptor(LPCTSTR sPortRegPath, LPCTSTR sPortRegName)
{
    m_Handle = (HANDLE)INVALID_SOCKET;
    m_pClient = NULL;
    m_MaxClient = 32;
    m_CurClient = 0;
    lstrcpy(m_sPortRegPath, sPortRegPath);
    lstrcpy(m_sPortRegName, sPortRegName);
}


RopAsyncAcceptor::~RopAsyncAcceptor()
{
    if (m_Handle != (HANDLE)INVALID_SOCKET) {
        closesocket((SOCKET)m_Handle);
        m_Handle = (HANDLE)INVALID_SOCKET;
    }
    RopRegistry key;
    key.Open(HKLM, m_sPortRegPath);
    key.Write(m_sPortRegName, DWORD(0));
    key.Close();
}

int RopAsyncAcceptor::Open(LPCTSTR szServer, DWORD dwPort)
{
    int dwRes = 0;

    if (m_Handle != (HANDLE)INVALID_SOCKET) {
        throw RopAsyncAcceptorException();
    }
    SOCKET s = INVALID_SOCKET;
    GROUP g = 0;
    s = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, g, WSA_FLAG_OVERLAPPED);
    if (s == INVALID_SOCKET) {
        throw RopAsyncAcceptorException();
    }
    char szLocalServer[128] = {0};
    int  iLen = 128;
    wc2mb((LPWSTR)szServer, wcslen(szServer), szLocalServer, &iLen);

    if (dwRes == 0) {
        IN_ADDR ListenIpAddress;
        SOCKADDR_IN m_listenAddr;
        ZeroMemory(&m_listenAddr, sizeof(m_listenAddr));
        
        m_listenAddr.sin_family = AF_INET;
        m_listenAddr.sin_port = htons((WORD)dwPort);
        ListenIpAddress.s_addr = inet_addr(szLocalServer);
        m_listenAddr.sin_addr = ListenIpAddress;

        if (bind(s, (SOCKADDR *)&m_listenAddr, sizeof(m_listenAddr))==SOCKET_ERROR) {
            closesocket((SOCKET)m_Handle);
			int a = WSAGetLastError();
            m_Handle = (HANDLE)INVALID_SOCKET;
            throw RopAsyncAcceptorException();
        }

        int namelen = sizeof(m_listenAddr);
        if (getsockname(s, (SOCKADDR *)&m_listenAddr, &namelen)==SOCKET_ERROR ) {
            closesocket((SOCKET)m_Handle);
            m_Handle = (HANDLE)INVALID_SOCKET;
            throw RopAsyncAcceptorException();
        }
        WSAEventSelect(s, EventHandle(), FD_ACCEPT|FD_CLOSE);
        dwRes = 0;
        m_Handle = (HANDLE)s;
        if (listen(s, 200)==SOCKET_ERROR) {
            closesocket((SOCKET)m_Handle);
            m_Handle = (HANDLE)INVALID_SOCKET;
            throw RopAsyncAcceptorException();
        }
        return ntohs(m_listenAddr.sin_port);
    }
    return -1;
}

void RopAsyncAcceptor::Receive()
{
    try {
        m_Status = 0;
        if (!CheckAndDoShutdown() && (m_Handle != (HANDLE)INVALID_SOCKET)) {
            // Nothing to do 
        }
    } catch (...) {
        LDebug(DEBUG_L_EMERG,"listensocket receive exception\r\n");
    }
    return;
}

int RopAsyncAcceptor::OnReceive()
{
    try {
        WSANETWORKEVENTS NetworkEvents;
        WSAEnumNetworkEvents((SOCKET)m_Handle, EventHandle(), &NetworkEvents);

        if (NetworkEvents.lNetworkEvents & FD_ACCEPT) {
            if (NetworkEvents.iErrorCode[FD_ACCEPT_BIT] != 0) {
                if (m_EventDispatcher) {
                    m_EventDispatcher->OnEvent(this, NULL, &m_Buf, EVENT_TYPE_LSNSOCKET_ERROR);
                }
                EventReset();
                return -1;
            }

            EventReset();
            if (m_EventDispatcher) {
                m_EventDispatcher->OnEvent(this, NULL, &m_Buf, EVENT_TYPE_SOCKET_ACCEPTED);
            }
        } else if (NetworkEvents.lNetworkEvents & FD_CLOSE) {
            Shutdown();
            EventReset();
            return -5;
        }
    } catch (...) {
        LDebug(DEBUG_L_EMERG,"listensocket onreceive exception\r\n");
    }
    return 1;
}
BOOL RopAsyncAcceptor::CheckAndDoShutdown()
{
    BOOL bRet = FALSE;
    if (m_ShutdownIndicated &&(m_Handle != (HANDLE)INVALID_SOCKET)) {
        closesocket((SOCKET)m_Handle);
        m_Handle = (HANDLE)INVALID_SOCKET;
        bRet = TRUE;
    }
    RopAsyncIO::CheckAndDoShutdown();
    return bRet;
}

RopAsyncIO* RopAsyncAcceptor::Accept()
{
    try {
        m_pClient = new RopAsyncSocket;

        if (!m_pClient) {
            return NULL;
        }

    } catch (...) {
        LDebug(DEBUG_L_EMERG,"listensocket accept exception\r\n");
        return NULL;
    }

    SOCKET s = accept((SOCKET)m_Handle,	NULL, NULL);
    m_pClient->m_Handle = (HANDLE)s;
    WSAEventSelect((SOCKET)(m_pClient->m_Handle), m_pClient->EventHandle(),0);

    int iBlock = 0;
    DWORD iRet;
    if (0!=WSAIoctl((SOCKET)(m_pClient->m_Handle),FIONBIO,&iBlock,sizeof(iBlock),NULL,0,&iRet,NULL,NULL)) {
        int iErr = WSAGetLastError();
        LDebug(DEBUG_L_EMERG,"listensocket ioctl error:%d\r\n",iErr);
        delete m_pClient;
        m_pClient = NULL;
        return NULL;
    }

    if (m_pClient->m_Handle == (HANDLE)INVALID_SOCKET) {
        return NULL;
    }
    return m_pClient;
}

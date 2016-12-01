
#include "stdafx.h"
#include "RopUtil.h"
#include "RopAsyncIO.h"
#include "RopEventDispatcher.h"

RopAsyncIO::RopAsyncIO() throw (RopAsyncIOException) : m_ShutdownIndicated (FALSE)
{
    m_bBlocked = FALSE;
    m_TxErr = m_RxErr = m_Tx = m_Rx = 0;
    m_OlRead.OffsetHigh = m_OlRead.Offset = 0;
    m_OlWrite.OffsetHigh = m_OlWrite.Offset = 0;
    m_Sync = 0;
    m_Peer = NULL;
    m_EventDispatcher = NULL;
    m_ShutdownIndicated = FALSE;
    m_pContext = NULL;
    m_bIsOpened = false;

    if ((m_OlRead.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL)) == NULL) {
        Dbg("[IO]Create Read Event Error :%d\n",GetLastError());
        throw RopAsyncIOException();
    }else if ((m_OlWrite.hEvent = CreateEvent (NULL, TRUE, FALSE, NULL)) == NULL) {
        Dbg("[IO]Create Write Event Error :%d\n",GetLastError());
        throw RopAsyncIOException();
    }

    m_Buf.data = new unsigned char [PACKET_LEN];
    if (!m_Buf.data) {
        Dbg("[IO]Create Buffer Error :%d\n",GetLastError());
        throw RopAsyncIOException();
    }

    m_Buf.datal = 0;
    m_Buf.datap = m_Buf.data;
    m_Buf.hdrl = 0;
    m_Buf.hdrp = m_Buf.data;
    m_Buf.resved = 0;
    m_Buf.size = PACKET_LEN;
    m_Buf.waitl = PACKET_LEN;

    m_Status = 0;

    for (int i=0;i<sizeof(MACADDR);i++)m_MacAddr[i]=0xff;
    ZeroMemory(m_Name, sizeof(m_Name));

    EventReset();
}


RopAsyncIO::~RopAsyncIO()
{
    try {
        if (m_Buf.data) {
            delete []m_Buf.data;
            m_Buf.data = NULL;
            m_Buf.datap = NULL;
        }
        if (m_OlRead.hEvent) CloseHandle (m_OlRead.hEvent);
        if (m_OlWrite.hEvent) CloseHandle (m_OlWrite.hEvent);
    } catch (...) {
    }
}

void RopAsyncIO::EventReset()
{
    ResetEvent (m_OlRead.hEvent);
}

void RopAsyncIO::Shutdown()
{
    m_ShutdownIndicated = TRUE;
    m_bIsOpened = false;
}

BOOL RopAsyncIO::CheckAndDoShutdown()
{
    BOOL bRet = FALSE;
    if (m_ShutdownIndicated) {
        if (m_OlRead.hEvent) CloseHandle (m_OlRead.hEvent);
        if (m_OlWrite.hEvent) CloseHandle (m_OlWrite.hEvent);
        m_OlWrite.hEvent = m_OlRead.hEvent = NULL;
        if (m_EventDispatcher)
            m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf,EVENT_TYPE_SOCKET_CLOSED);
        bRet = TRUE;
    }

    return bRet;
}

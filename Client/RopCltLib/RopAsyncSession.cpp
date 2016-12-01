/* 
 * In async socket environment, the msg recved by async socket maybe
 * contains only part of msg data, So it's unsafe to cast the recved data
 * to the msg, and it's unconvinient to force recv() function until
 * the data all recved in Async socket.
 * The session will put async socket data into buffer and try to use peek-msg
 * to get an "untacked" msg.
 * Version    Data       Author
 * 1.0     2010-11-16    wy
 * 
 * All rights reserved by Secwing, Extelecom company.
 */
#include "stdafx.h"
#include "RopAsyncSession.h"


RopSession::RopSession()
{
    m_pData = new unsigned char[MAX_BUFLEN];
    m_BufLen = MAX_BUFLEN;
    m_TailPos = m_HeadPos = 0;
    if (!m_pData) {
        Dbg("RopSession alloc memory error.\n");
        m_BufLen = 0;
        throw RopSessionException();
    }
}

RopSession::RopSession(unsigned int iBufLen)
{
    m_pData = new unsigned char[iBufLen];
    m_BufLen = iBufLen;
    m_TailPos = m_HeadPos = 0;
    if (!m_pData) {
        Dbg("RopSession alloc memory error.\n");
        m_BufLen = 0;
        throw RopSessionException();
    }
}

RopSession::~RopSession()
{
    if (m_pData) {
        delete []m_pData;
        m_pData = NULL;
    }
    m_TailPos = m_HeadPos = 0;

}

unsigned int RopSession::InputData(unsigned char *buf, unsigned int len)
{
    unsigned int iUsed = m_TailPos-m_HeadPos;
    unsigned int iAvailable = m_BufLen - iUsed;

    if (len > iAvailable) { //expand or just return error?
        return 0;
    }
    memmove(m_pData + m_TailPos,buf,len);
    m_TailPos += len;
    
    return len;

}

PModMsg RopSession::PeekMsg()
{
    unsigned int iUsed = m_TailPos - m_HeadPos;
    if (iUsed < sizeof(ModMsg)) {
        return NULL;
    }
    PModMsg pMsg = (PModMsg)(m_pData + m_HeadPos);
    if (iUsed < (pMsg->len + sizeof(ModMsg))) {
        return NULL;
    }
    if (pMsg->tag != MAGIC_TAG) {
        Reset();
        return NULL;
    }
    
    return pMsg;//need to valid message here
}

unsigned int RopSession::DiscardData(unsigned int len)
{
    unsigned int iUsed = m_TailPos-m_HeadPos;
    if (iUsed < len) {
        return 0;
    }
    m_TailPos -= len;
    iUsed = m_TailPos - m_HeadPos;

    /* move data to head */
    if (iUsed > 0)
        memmove(m_pData+m_HeadPos,m_pData+m_HeadPos+len,iUsed);
    
    return len;
}

unsigned int RopSession::GetDataCount()
{
    unsigned int iUsed = m_TailPos - m_HeadPos;
    return iUsed;
}

void RopSession::Reset()
{
    m_TailPos = m_HeadPos = 0;   
}

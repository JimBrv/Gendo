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

#ifndef ROP_ASYNC_SESSION_H
#define ROP_ASYNC_SESSION_H

#include "stdafx.h"
#include <windows.h>
#include "RopBase.h"
#include "RopBuf.h"
#include "RopUtil.h"
#include "RopModule.h"

#define MAX_BUFLEN 1*1024*1024   // 3M max for async socket data buf

class RopSessionException
{
};
class RopSession : public RopBase  
{
public:
    void Reset();
    unsigned int GetDataCount();
    unsigned int DiscardData(unsigned int len);
    PModMsg PeekMsg();
    unsigned int InputData(unsigned char* buf, unsigned int len);
    RopSession();
    RopSession(unsigned int iBufLen);
    virtual ~RopSession();

protected:
    unsigned long  m_BufLen;
    unsigned long  m_TailPos;  // Buffer index tail to put data
    unsigned long  m_HeadPos;  // Buffer index head to get data
    unsigned char* m_pData;   // Buffer start point
};

#endif
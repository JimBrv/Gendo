/************************************************************************/
/*  Sync socket interface for module_msg/msg_head_t/xxx_msg etc. */
/************************************************************************/

#ifndef ROP_SYNCMSG_H
#define ROP_SYNCMSG_H

#include "stdafx.h"
#include "RopBase.h"
#include "lib-msg.h"
#include "RopModule.h"
#include "RopAsyncIO.h"
#include "RopSyncSocket.h"
#include "RopUtil.h"

class RopSyncMsg:public RopBase
{
public:
    RopSyncMsg(RopSyncSocket *pSocket) {m_pSocket = pSocket;};
    ~RopSyncMsg() {};

    int RecvAndWaitMsg(PModMsg buf, int len, DWORD dwTimeout = INFINITE);
    int RecvAndWaitMsg(pmsg_head_t buf, int len, DWORD dwTimeout = INFINITE);

    int SendMsg(PModMsg buf, int len, DWORD dwTimeout = INFINITE);
    int SendMsg(pmsg_head_t buf, int len, DWORD dwTimeout = INFINITE);
public:
    RopSyncSocket *m_pSocket;
};

#endif
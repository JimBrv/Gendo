#include "stdafx.h"
#include "RopSyncMsg.h"

#define MSG_HEAD_SANITY(Mx) (((Mx)->len<MAX_MSG_LEN) && (Mx)->msg!=0)
#define MOD_HEAD_SANITY(Mx) (((Mx)->len<MAX_MSG_LEN) && (Mx)->tag == MAGIC_TAG)

int RopSyncMsg::RecvAndWaitMsg(PModMsg buf, int len, DWORD dwTimeout)
{
    PModMsg pMsg = (PModMsg)buf;
    char   *pBuf = (char*)buf;
    if (!m_pSocket) return -1;
    int iTotal = sizeof(ModMsg);
    int iCur = 0, iRet = 0;

    while (iCur < iTotal) {
        iRet = m_pSocket->ReceiveAndWaitEx(pBuf+iCur, iTotal-iCur, dwTimeout);
        if (iRet == (-1)*WSAETIMEDOUT) {
            //LDebug(DEBUG_L_ERROR, "recv mod_head timeout!");
            return iRet;
        }else if (iRet <= 0) {
			LDebug(DEBUG_L_ERROR, "recv mod_head error=%d!", iRet);
			return iRet;
		}
        iCur += iRet;
    }

    if (!MOD_HEAD_SANITY(pMsg)) {
        LDebug(DEBUG_L_ERROR, "recv mod_head(%d) disorder:%d->%d, tag=0x%x, len=%d!", 
               iCur, pMsg->sModule, pMsg->dModule, pMsg->tag, pMsg->len);
        return -1;
    }

    if (pMsg->len > (len-sizeof(ModMsg))) {
        LDebug(DEBUG_L_ERROR, "recv mod_head larger than buf len=%d, %d->%d, tag=0x%x, len=%d!", 
               len, pMsg->sModule, pMsg->dModule, pMsg->tag, pMsg->len);
        return -1;
    }

    iTotal = pMsg->len;
    pBuf = (char*)pMsg->payload;
    iCur = 0;
    while(iCur < iTotal) {
        iRet = m_pSocket->ReceiveAndWaitEx(pBuf+iCur, iTotal-iCur, dwTimeout);
        if (iRet==(-1)*WSAETIMEDOUT) {
            LDebug(DEBUG_L_ERROR, "recv mod_payload timeout!");
            return iRet;
		}else if (iRet <= 0) {
			LDebug(DEBUG_L_ERROR, "recv mod_payload error=%d!", iRet);
			return iRet;
		}
        iCur += iRet;
    }
    return sizeof(ModMsg)+pMsg->len;  
}

int RopSyncMsg::RecvAndWaitMsg(pmsg_head_t buf, int len, DWORD dwTimeout)
{
    pmsg_head_t pMsg = (pmsg_head_t)buf;
    char       *pBuf = (char*)buf;
    if (!m_pSocket) return -1;
    int iTotal = sizeof(msg_head_t);
    int iCur = 0, iRet = 0;

    while (iCur < iTotal) {
        iRet = m_pSocket->ReceiveAndWaitEx(pBuf+iCur, iTotal-iCur, dwTimeout);
        if (iRet <= 0 || iRet==(-1)*WSAETIMEDOUT) {
            //LDebug(DEBUG_L_ERROR, "recv msg_head error!");
            return iRet;
        }
        iCur += iRet;
    }


    if (!MSG_HEAD_SANITY(pMsg)) {
        LDebug(DEBUG_L_ERROR, "recv msg_head disorder:msg=0x%x, len=%d!", pMsg->msg, pMsg->len);
        return -1;
    }

    if (pMsg->len > (len-sizeof(msg_head_t))) {
        LDebug(DEBUG_L_ERROR, "recv msg_head large than buf:msg=0x%x, len=%d!", pMsg->msg, pMsg->len);
        return -1;
    }

    iTotal = pMsg->len;
    pBuf = (char*)pMsg->payload;
    iCur = 0;
    while(iCur < iTotal) {
        iRet = m_pSocket->ReceiveAndWaitEx(pBuf+iCur, iTotal-iCur, dwTimeout);
        if (iRet <= 0 || iRet==(-1)*WSAETIMEDOUT) {
            //LDebug(DEBUG_L_ERROR, "recv msg_payload error!");
            return iRet;
        }
        iCur += iRet;
    }
    return sizeof(msg_head_t)+pMsg->len;
}

int RopSyncMsg::SendMsg(PModMsg buf, int len, DWORD dwTimeout)
{
    PModMsg pMsg = (PModMsg)buf;
    char   *pBuf = (char*)buf;
    if (!m_pSocket) return -1;
    int iTotal = sizeof(ModMsg);
    int iCur = 0, iRet = 0;

    while (iCur < iTotal) {
        iRet = m_pSocket->SendEx(pBuf+iCur, iTotal-iCur, dwTimeout);
        if (iRet <= 0 || iRet==(-1)*WSAETIMEDOUT) {
            //LDebug(DEBUG_L_ERROR, "send mod_head error!");
            return iRet;
        }
        iCur += iRet;
    }

    iTotal = pMsg->len;
    pBuf = (char*)pMsg->payload;
    iCur = 0;
    while(iCur < iTotal) {
        iRet = m_pSocket->SendEx(pBuf+iCur, iTotal-iCur, dwTimeout);
        if (iRet <= 0 || iRet==(-1)*WSAETIMEDOUT) {
            //LDebug(DEBUG_L_ERROR, "send mod_payload error!");
            return iRet;
        }
        iCur += iRet;
    }
    return sizeof(ModMsg)+pMsg->len;  
}
int RopSyncMsg::SendMsg(pmsg_head_t buf, int len, DWORD dwTimeout)
{
    pmsg_head_t pMsg = (pmsg_head_t)buf;
    char       *pBuf = (char*)buf;
    if (!m_pSocket) return -1;
    int iTotal = sizeof(msg_head_t);
    int iCur = 0, iRet = 0;

    while (iCur < iTotal) {
        iRet = m_pSocket->SendEx(pBuf+iCur, iTotal-iCur, dwTimeout);
        if (iRet <= 0 || iRet==(-1)*WSAETIMEDOUT) {
            //LDebug(DEBUG_L_ERROR, "send msg_head error!");
            return iRet;
        }
        iCur += iRet;
    }

    iTotal = pMsg->len;
    pBuf = (char*)pMsg->payload;
    iCur = 0;
    while(iCur < iTotal) {
        iRet = m_pSocket->SendEx(pBuf+iCur, iTotal-iCur, dwTimeout);
        if (iRet <= 0 || iRet==(-1)*WSAETIMEDOUT) {
            //LDebug(DEBUG_L_ERROR, "send msg_payload error!");
            return iRet;
        }
        iCur += iRet;
    }
    return sizeof(msg_head_t)+pMsg->len;
}


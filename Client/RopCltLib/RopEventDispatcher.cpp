#include "stdafx.h"
#include "RopEventDispatcher.h"

void RopEventDispatcher::OnEvent(RopAsyncIO *pSrcIO, RopAsyncIO *pDstIO,ROP_BUF* pBuf,DWORD dwEventType)
{
    try {
        switch(dwEventType) {
            case EVENT_TYPE_SOCKET_CLOSED: 
                if (m_pCallBack) 
                    m_pCallBack(pSrcIO->m_pContext, pSrcIO, (unsigned char*)pBuf->datap, pBuf->datal, dwEventType);
        }
    }
    catch(...)
    {
        LDebug(DEBUG_L_ERROR, "Event parent dispatch msg got error!\n");
    }
}

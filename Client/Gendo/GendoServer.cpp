#include "StdAfx.h"
#include "RopUtil.h"
#include "GendoServer.h"


INT CGendoSvrList::Insert(INT       index,
    server_t                *pSvr,
    CListUI                 *pList, 
    CListContainerElementUI *pListItem,
    CHorizontalLayoutUI     *pHLayout,
    CLabelUI                *pLbName,
    CLabelUI                *pLbDesc,
    CLabelUI                *pLbProto,
    CLabelUI                *pLbPing,
    CLabelUI                *pLbLoad,
    CLabelUI                *pLbState)
{
    GendoSvr *pGdSvr = new GendoSvr;
    memset(pGdSvr, 0, sizeof(GendoSvr));
    memcpy(&pGdSvr->SvrInfo, pSvr, sizeof(server_t));
    pGdSvr->pList     = pList;
    pGdSvr->pListItem = pListItem;
    pGdSvr->pHLayout  = pHLayout;
    pGdSvr->pLbName   = pLbName;
    pGdSvr->pLbDesc   = pLbDesc;
    pGdSvr->pLbProto  = pLbProto;
    pGdSvr->pLbPing   = pLbPing;
    pGdSvr->pLbLoad   = pLbLoad;
    pGdSvr->pLbState  = pLbState;
    pGdSvr->pWnd      = GetDuiMainWnd();

    INT ret = ParseServerIpPort(pGdSvr);

    m_pSvrList.push_back(pGdSvr);
    if (m_pDuiList == NULL) m_pDuiList = pList;
    return 0;
}


INT CGendoSvrList::Remove(INT index)
{
    int i = 0;
    if (index < 0 || index >= m_pSvrList.size()) return 0;

    vector<GendoSvr*>::iterator iter;
    for (iter = m_pSvrList.begin();iter != m_pSvrList.end() && i != index - 1; iter++,i++)
    
    if (iter != m_pSvrList.end()) {
        GendoSvr *pSvr = (*iter);
        iter = m_pSvrList.erase(iter);
        delete pSvr;
    }
    
    return 0;
}

INT CGendoSvrList::RemoveAll(VOID)
{
    vector<GendoSvr*>::iterator iter;
    for (iter = m_pSvrList.begin();iter != m_pSvrList.end();) { 
        GendoSvr *pSvr = (*iter);
        iter = m_pSvrList.erase(iter);
        delete pSvr;
    }
    return 0;
}

PGendoSvr CGendoSvrList::Get(INT   index)
{
    if (index < 0 || index >= m_pSvrList.size()) return NULL;
    PGendoSvr pSvr = (PGendoSvr)m_pSvrList[index];
    return pSvr;
}


/*
 * Parse SSL port to port array
 */
INT CGendoSvrList::ParseServerIpPort(GendoSvr *pSvr)
{
    CHAR PortAry[10][MAX_NAME_LEN];
    CHAR IP[MAX_NAME_LEN];
    INT  Cnt = 10, i = 0, j = 0, k = 0;

    if (!pSvr) return -1;

    if (StringSplit(pSvr->SvrInfo.ssl_port, ',', (char **)PortAry, MAX_NAME_LEN, &Cnt)) {
        return -1;
    }

    if (Cnt <= 0 || Cnt >= 10) {
        return -1;
    }
    
    for (i = 0; i < Cnt; i++) {
        CHAR *pTmp = strchr(PortAry[i], '=');
        if (!pTmp) continue;
        pTmp++;
        INT Port = atoi(pTmp);
        if (Port <= 0 || Port >= 65535) continue;

        if (strstr(PortAry[i], "tcp")) {
            pSvr->ssl_tcp[j] = Port;
            j++;
        }else if (strstr(PortAry[i], "udp")) {
            pSvr->ssl_udp[k] = Port;
            k++;
        }
    }
    pSvr->ssl_tcpport_cnt = j;
    pSvr->ssl_udpport_cnt = k;
    i = MAX_NAME_LEN;
    mb2wc(pSvr->SvrInfo.ip, strlen(pSvr->SvrInfo.ip), (wchar_t*)pSvr->ip, &i);

    return 0;
}



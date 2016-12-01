#pragma once
#include "StdAfx.h"
#include <vector>
#include "lib-msg.h"

using namespace std;

typedef struct _GendoSvr_ 
{
    VOID                    *pWnd;
    server_t                 SvrInfo;
    CListUI                 *pList;
    CListContainerElementUI *pListItem;
    CHorizontalLayoutUI     *pHLayout;
    CLabelUI                *pLbName;
    CLabelUI                *pLbDesc;
    CLabelUI                *pLbProto;
    CLabelUI                *pLbPing;
    CLabelUI                *pLbLoad;
    CLabelUI                *pLbState;
    TCHAR                    ip[MAX_NAME_LEN];
    INT                      iCurProto;
    TCHAR                    sCurProtoName[MAX_NAME_LEN];
    INT                      ssl_tcpport_cnt;               // TCP/UDP port array
    INT                      ssl_tcp[32];
    INT                      ssl_udpport_cnt;              
    INT                      ssl_udp[32];
}GendoSvr, *PGendoSvr;


class CGendoSvrList
{
public:
    CGendoSvrList() {m_pDuiList = NULL;}
    ~CGendoSvrList(){};

    INT Insert(INT       index,
               server_t                *pSvr,
               CListUI                 *pList, 
               CListContainerElementUI *pListItem,
               CHorizontalLayoutUI     *pHLayout,
               CLabelUI                *pLbName,
               CLabelUI                *pLbDesc,
               CLabelUI                *pLbProto,
               CLabelUI                *pLbPing,
               CLabelUI                *pLbLoad,
               CLabelUI                *pLbState);
    INT Remove(INT index);
    INT RemoveAll(VOID);

    PGendoSvr Get(INT   index);
    INT       Size(VOID) {return m_pSvrList.size();}
    VOID      SetDuiList(CListUI *pList) {m_pDuiList = pList;}
    VOID     *GetDuiList()               {return m_pDuiList;}
    VOID      SetDuiMainWnd(VOID *pWnd)  {m_pDuiWnd = pWnd;}
    VOID     *GetDuiMainWnd()            {return m_pDuiWnd;}

    INT        ParseServerIpPort(GendoSvr *pSvr);

private:
    VOID              *m_pDuiWnd;
    CListUI           *m_pDuiList;
    vector<GendoSvr *> m_pSvrList;

};
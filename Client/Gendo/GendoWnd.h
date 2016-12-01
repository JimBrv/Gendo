#pragma once
#include "StdAfx.h"
#include "Resource.h"
#include "RopPPTP.h"
#include "RopOpenVpn.h"
#include "RopL2TP.h"
#include "RopTap.h"
#include "GendoServer.h"

typedef struct _SplitDuiControlOpt_ 
{
    INT   iOperation;
    PVOID pDuiWnd;
    PVOID pParam;
}SplitDuiControl, *PSplitDuiControl;



//////////////////////////////////////////////////////////////////////////
///

class CGendoWnd : public WindowImplBase, public IListCallbackUI
{
public:
	CGendoWnd(void);
	~CGendoWnd(void);

	virtual void OnFinalMessage( HWND );
	virtual CDuiString GetSkinFolder();
	virtual CDuiString GetSkinFile();
	virtual LPCTSTR GetWindowClassName( void ) const;
	virtual void Notify( TNotifyUI &msg );
	virtual LRESULT OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual LRESULT OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	virtual void InitWindow();
	virtual LRESULT OnMouseHover( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    //virtual LRESULT OnMouseMove( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	virtual LRESULT OnChar( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
	virtual UILIB_RESOURCETYPE GetResourceType() const;
	virtual CDuiString GetZIPFileName() const;


    void ShowMsg(LPCTSTR str);
    VOID ShowNotice(LPCTSTR str);
    VOID ShowNotice(CHAR *str);

	DUI_DECLARE_MESSAGE_MAP()
	virtual void OnClick(TNotifyUI& msg);
	virtual void OnSelectChanged( TNotifyUI &msg );
	virtual void OnItemClick( TNotifyUI &msg );
    void ShowLoginDlg(TNotifyUI &msg);

    void ShowUserInfo(void);

    virtual LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem);


    LRESULT OnUserProg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnUserPrint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnCopyData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


public:
    VOID      SetInstance(HINSTANCE hIntance) {m_hInstance = hIntance;}
    HINSTANCE GetInstance() {return m_hInstance;}
    void      AddTray();
    void      DelTray();
    /* 气球提示 */
    BOOL      SetBubbleTips(LPCTSTR lpszTips, LPCTSTR lpszTitle);
    /* 鼠标提示 */
    BOOL SetTrayTips(LPCTSTR lpszTips);
    /* 托盘提示 */
    VOID SetTrayConnectStatus(INT state, LPCTSTR svr, LPCTSTR proto);

public:
    CListUI *GetDownloadList(VOID) {return m_pListUI;}
    CListUI *GetHistoryList(VOID)  {return m_pListHistory;}

public:
    virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
    CLabelUI    *GetDialMsgCtrl(VOID) {return m_pLabelProtoInfo;};
    CProgressUI *GetDialProgressCtrl(VOID) {return NULL;};


public:
    INT CreateSvrUI(pserver_t pSvr, int index);
    INT GetServerList(TCHAR *ip, INT port);
    INT RefreshServerList(void);
    INT PingServerList(void);

    /* Get server info by list index */
    INT GetServerInfoByIndex(int index, pserver_t pSvr);

public:
    int Connect();
    int ConnectSSL(int protocol);
    int Disconnect();
    int GetRandomSSLPort(INT *port_ary, INT cnt);

public:
    VOID CreateUpdateExe(VOID);

    /* OpenVPN status */
    INT ShowOpenVPNStatus(VOID);

    VOID SetConnectProgreass(INT status);

public:
	CButtonUI* m_pCloseBtn;
	CButtonUI* m_pMaxBtn;
	CButtonUI* m_pRestoreBtn;
	CButtonUI* m_pMinBtn;
	CListUI   *m_pListUI;
    CListUI   *m_pListHistory;

    CEditUI*    m_pUser;
    CEditUI*    m_pUserPwd;
    CComboUI*   m_pProtocol;
    CLabelUI*   m_pLabelProtoInfo;

    CProgressUI* m_pUserInfoTrafficPrg;
    COptionUI*  m_pOptPPTP;
    COptionUI*  m_pOptL2TP;
    COptionUI*  m_pOptSSLTCP;
    COptionUI*  m_pOptSSLUDP;

    CProgressUI* m_pConnectPrg;

    CButtonUI *m_pConnectBtn;
    CButtonUI *m_pDisConnectBtn;

public:
	CListUI             *m_pMsgList;
    CRITICAL_SECTION     m_Lock;
    vector<CDuiString *> m_MsgPool;
    server_list_t        m_SvrList;
    server_t             m_SvrOnChoise;
    server_t             m_SvrOnUse;
    BOOL                 m_bSvrChoise;

public:
    BOOL                m_bServerListGot;   /* Server list got yet */
    BOOL                m_bLockSvr;         /* Should lock server list after connecting OK */
    class CGendoSvrList m_ServerList;       /* Server list container */
    GendoSvr            m_ServerCur;        /* Current server selected */

    HANDLE              m_hServerSync;      /* Sync lock for server refresh */
    

public:
    RopPPTP    m_pptp;
    RopOpenVpn m_ovpn;
    RopL2TP    m_l2tp;
    RopTap     m_tap;


public:
    CRITICAL_SECTION m_ListLock;
    int       m_curTab; // current active tab

public:
    HINSTANCE m_hInstance;
};


void WINAPI OnRasDialCallback(UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwError);


/* Thread make main frame UI unblock */
DWORD WINAPI ThdOpvnVpnConnect(LPVOID lpParam);
DWORD WINAPI ThdShowSvrList(LPVOID lpParam);
DWORD WINAPI ThdRefreshSvrList(LPVOID lpParam);

DWORD WINAPI ThdPingSvr2(LPVOID lpParam);
DWORD WINAPI ThdPingSvr(LPVOID lpParam);

/* Make user info tab sportish */
DWORD WINAPI ThdRefreshUserInfo(LPVOID lpParam);

DWORD WINAPI ThdDisconnectVPN(LPVOID lpParam);

DWORD WINAPI PingSvrByHttp(LPVOID lpParam);
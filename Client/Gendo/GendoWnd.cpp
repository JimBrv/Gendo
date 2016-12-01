#include "StdAfx.h"
#include <stdlib.h>
#include <time.h>
#include <ShellAPI.h>
#include "GendoWnd.h"
#include "RopUtil.h"
#include "GendoLoginWnd.h"
#include "GendoUser.h"
#include "RopPPTP.h"
#include "RopPing.h"
#include "RopProcess.h"
#include "RopFileUtil.h"
using namespace std;

CGendoUser User;
user_t     UserInfo;
extern CGendoWnd *pMainWnd;

NOTIFYICONDATA nid;

BOOL Debug = TRUE;

DWORD WINAPI ThreadShowMsg(LPVOID lpParam)
{
    CGendoWnd *pWnd = (CGendoWnd*)(lpParam);
    ASSERT(pWnd != NULL);
    Sleep(5000);
    int j = 0;
    while(true) {
        int i = 0;
        EnterCriticalSection(&pWnd->m_Lock);
        for (i = 0; i < pWnd->m_MsgPool.size(); i++) {
            j++;
            CDuiString *pMsg = pWnd->m_MsgPool[i];
            ASSERT(pMsg != NULL);
            if (Debug) {
                CListContainerElementUI *pListItem =  new CListContainerElementUI;
                CTextUI *pText = new CTextUI;
                pListItem->ApplyAttributeList(_T(" width=\"600\" height=\"20\""));
                pText->ApplyAttributeList(_T(" width=\"600\" height=\"20\""));
                pText->SetText(pMsg->GetData());
                if (pWnd->m_pMsgList != NULL) {
                    pListItem->Add(pText);
                    pWnd->m_pMsgList->Add(pListItem);
                    pWnd->m_pMsgList->EndDown();
                }
            }
            delete pMsg;
        }
        if (pWnd->m_pMsgList->GetCount() > 200)
            pWnd->m_pMsgList->RemoveAll();
        pWnd->m_MsgPool.clear();
        LeaveCriticalSection(&pWnd->m_Lock);
        Sleep(100);
    }
}

//////////////////////////////////////////////////////////////////////////
///

DUI_BEGIN_MESSAGE_MAP(CGendoWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_SELECTCHANGED,OnSelectChanged)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_ITEMCLICK,OnItemClick)
DUI_END_MESSAGE_MAP()


CGendoWnd::CGendoWnd(void)
{
    InitializeCriticalSection(&m_Lock);
    InitializeCriticalSection(&m_ListLock);
    
    m_pCloseBtn = NULL;
    m_pMaxBtn = NULL;
    m_pRestoreBtn  = NULL;
    m_pMinBtn = NULL;
    m_pListUI = NULL;
    m_pMsgList = NULL;
    
    m_pUser = NULL;
    m_pUserPwd = NULL;
    m_pProtocol = NULL;
    m_pLabelProtoInfo = NULL;
    m_bLockSvr = FALSE;
    m_bSvrChoise = FALSE;
    m_bServerListGot = FALSE;
    m_pUserInfoTrafficPrg = NULL;
    m_curTab = 0;
}

CGendoWnd::~CGendoWnd(void)
{
}

void CGendoWnd::OnFinalMessage( HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

DuiLib::CDuiString CGendoWnd::GetSkinFolder()
{
	return _T("gendo-skin");	
}

DuiLib::CDuiString CGendoWnd::GetSkinFile()
{
	return _T("gendo.xml");
}

UILIB_RESOURCETYPE CGendoWnd::GetResourceType() const
{
#ifdef _DEBUG
	return UILIB_FILE;
#else
    //return UILIB_FILE;
	return UILIB_ZIP;
#endif
}

DuiLib::CDuiString CGendoWnd::GetZIPFileName() const
{
	return _T("gendo-skin0725.zip");
}

LPCTSTR CGendoWnd::GetWindowClassName( void ) const
{
	return _T("GendoWnd");
}

int CGendoWnd::ConnectSSL(int protocol)
{
    int ret = 0;
    int ssl_port = 0;

    if (m_tap.TapStatus()) {
        /* Install TAP */
        ShowNotice(_T("SSL协议需要安装新的软件模块，正在安装中，请稍后..."));
        ret = m_tap.TapInstall();
        if (ret) {
            ShowNotice(_T("SSL协议安装失败，请关闭客户端重试！！！"));
            return 1;
        }
        ShowNotice(_T("SSL协议模块安装成功，正在配置网络环境..."));
        ret = m_tap.TapSetName("gendovpn");

        if (ret) {
            ShowNotice(_T("SSL协议安装成功，但配置网络环境失败，请重启客户端自动修复！"));
            return 1;
        }

        ShowNotice(_T("SSL协议模块安装成功，正在为您接入SSL..."));
    }else{
        if (!m_tap.IsTapNameOK("gendovpn")) {
            LDebug(DEBUG_L_CRITICAL, "Tap installed, but name not gendovpn, force change!!!");
            m_tap.TapSetName("gendovpn");
        }
        ShowNotice(_T("SSL协议驱动运行正常，正在为您接入SSL......"));
    }

    if (PROTO_OPENVPN_TCP == protocol) {
        m_ovpn.SetAuthTyp(0,0);
        ssl_port = GetRandomSSLPort(m_ServerCur.ssl_tcp, m_ServerCur.ssl_tcpport_cnt);
        m_ovpn.SetParams(User.m_sUser, User.m_sPwd, m_ServerCur.ip, 1, ssl_port, (LPCTSTR)m_ServerCur.pLbName->GetText());
    }else{
        m_ovpn.SetAuthTyp(0,0);
        ssl_port = GetRandomSSLPort(m_ServerCur.ssl_udp, m_ServerCur.ssl_udpport_cnt);
        m_ovpn.SetParams(User.m_sUser, User.m_sPwd, m_ServerCur.ip, 0, ssl_port, (LPCTSTR)m_ServerCur.pLbName->GetText());
    }

    HANDLE hthread = CreateThread(NULL,0, ThdOpvnVpnConnect, this, NULL, NULL);
    CloseHandle(hthread);
       
    return 0;
}


int CGendoWnd::Connect()
{
    TCHAR SvrIP[32]  = {0};
    TCHAR SvrPort[8] = {0};
    INT   SvrProto   = 0;
    INT   len = 32, ret = 0;    

    m_pConnectPrg->SetVisible(true);
    m_pConnectPrg->SetValue(0);

    if (!m_bSvrChoise) {
        ShowNotice(_T("您还未选择服务器，请单击选中服务器!"));
        return -1;
    } 

    if (!User.m_bLogin) {
        ShowNotice(_T("您尚未登陆，请点击右上角提示登陆!"));
        return -1;
    }

    if (m_pOptPPTP->IsSelected()) {
        SvrProto = m_ServerCur.iCurProto = PROTO_PPTP;
    }else if (m_pOptL2TP->IsSelected()) {
        SvrProto = m_ServerCur.iCurProto = PROTO_L2TP;
    }else if (m_pOptSSLUDP->IsSelected()) {
        SvrProto = m_ServerCur.iCurProto = PROTO_OPENVPN_UDP;
    }else{
        ShowNotice(_T("未选择协议，请先选择协议！"));
        return -1;   
    }
    

    //m_pptp.CreatePPTPEntry(_T("jimpass"));
    if (SvrProto == PROTO_PPTP) {
        m_pptp.SetIpUserPwd(m_ServerCur.ip, User.m_sUser, User.m_sPwd);
        ret = m_pptp.DialPPTP(OnRasDialCallback);
    }else if (SvrProto == PROTO_L2TP) {
        //ShowNotice(_T("L2TP服务目前暂时关闭，请重新选择服务器！"));
        m_l2tp.SetIpUserPwd(m_ServerCur.ip, User.m_sUser, User.m_sPwd);
        ret = m_l2tp.Dial(OnRasDialCallback);
    }else if (SvrProto== PROTO_OPENVPN_TCP) {
        /* TODO: for random port */
        ret = ConnectSSL(PROTO_OPENVPN_TCP);
        if (ret) {
            LDebug(DEBUG_L_CRITICAL, "Install SSL Tap first");
        }
        ret = 0;
    }else if (SvrProto == PROTO_OPENVPN_UDP) {
        /* TODO: for random port */
        ret = ConnectSSL(PROTO_OPENVPN_UDP);
        if (ret) {
            LDebug(DEBUG_L_CRITICAL, "Install SSL Tap first");
        }
        ret = 0;
        /* SSL is async workding thread */
    }

    if (ret) {
        Disconnect();
        ShowNotice(_T("连接服务器失败，请重试!"));
        return -1;
    }
    
    if (SvrProto == PROTO_L2TP || SvrProto == PROTO_PPTP) {
        SetConnectProgreass(100);
        ShowNotice(TEXT("连接成功! VPN已经建立，祝您愉快畅游网络!"));
        SetTrayConnectStatus(0, (LPCTSTR)m_ServerCur.pLbName->GetText(), (SvrProto == PROTO_PPTP) ? _T("PPTP"):_T("L2TP"));
    }

    m_bLockSvr = TRUE;
    CButtonUI *pBtn = (CButtonUI*)m_PaintManager.FindControl(_T("connect"));
    assert(pBtn != NULL);
    //pBtn->ApplyAttributeList(_T("disabledtextcolor=\"#FFA7A6AA\"  normalimage=\"file='toolbar\\start2.png' dest='3,3,18,18'\" hotimage=\"file='toolbar\\start2.png' dest='0,0,18,18'\""));
    pBtn->SetEnabled(FALSE);

    pBtn = (CButtonUI*)m_PaintManager.FindControl(_T("disconnect"));
    assert(pBtn != NULL);
    pBtn->SetEnabled(TRUE);

    return 0;
}

int CGendoWnd::Disconnect()
{
    CLabelUI *pInfo = GetDialMsgCtrl();
    CDuiString str = _T("正在断开VPN连接......");
    pInfo->SetText(str.GetData());
    Sleep(1*1000);
    /* close PPTP */
    m_pptp.HangUp();

    /* close L2TP */
    m_l2tp.HangUp();

    /* close ssl */
    m_ovpn.DisConnect();

    Sleep(1*1000);
    str = _T("VPN连接已断开!");
    pInfo->SetText(str.GetData());

    m_bLockSvr = FALSE;
    CButtonUI *pBtn = (CButtonUI*)m_PaintManager.FindControl(_T("connect"));
    assert(pBtn != NULL);
    //pBtn->ApplyAttributeList(_T("disabledtextcolor=\"#FFA7A6AA\"  normalimage=\"file='toolbar\\start2.png' dest='3,3,18,18'\" hotimage=\"file='toolbar\\start2.png' dest='0,0,18,18'\""));
    pBtn->SetEnabled(TRUE);

    pBtn = (CButtonUI*)m_PaintManager.FindControl(_T("disconnect"));
    assert(pBtn != NULL);
    pBtn->SetEnabled(FALSE);
    SetTrayConnectStatus(1, NULL, NULL);
    return 0;
}

void CGendoWnd::AddTray()
{
    nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
    nid.hWnd   = m_hWnd;    
    nid.uID    = IDR_MAINFRAME;
    nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP ;
    nid.uCallbackMessage = WM_NOTIFYICON;  //自定义的消息名称
    //nid.hIcon = LoadIcon(GetWindowInstance(NULL), _T("IDI_SMALL"));
    //nid.hIcon = LoadIcon(GetInstance(), _T("IDI_SMALL"));
    nid.hIcon = LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_SMALL));
    lstrcpy(nid.szTip, _T("跟斗云，随行而至！\r\n状态： 未连接\r\n"));
    ::Shell_NotifyIcon(NIM_ADD, &nid);      //在托盘区添加图标
}

void CGendoWnd::DelTray()
{
    //nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
    //nid.hWnd   = m_hWnd;    
    //nid.uID    = IDR_MAINFRAME;
    //nid.uFlags = NIF_ICON|NIF_MESSAGE|NIF_TIP ;
    //nid.uCallbackMessage = WM_NOTIFYICON;  //自定义的消息名称
    ////nid.hIcon = LoadIcon(GetWindowInstance(NULL), _T("IDI_SMALL"));
    ////nid.hIcon = LoadIcon(GetInstance(), _T("IDI_SMALL"));
    //nid.hIcon = LoadIcon(GetInstance(), MAKEINTRESOURCE(IDI_SMALL));
    //lstrcpy(nid.szTip, _T("跟斗云已在后台运行"));
    ::Shell_NotifyIcon(NIM_DELETE, &nid);      //在托盘区添加图标
}

/* 托盘气球提示 */
BOOL CGendoWnd::SetBubbleTips(LPCTSTR lpszTips, LPCTSTR lpszTitle)
{
    nid.uFlags = NIF_INFO;
    lstrcpy(nid.szInfoTitle, lpszTitle);
    lstrcpy(nid.szInfo, lpszTips);
    nid.dwInfoFlags = NIIF_USER;
    nid.uTimeout    = 1000;
    return ::Shell_NotifyIcon(NIM_MODIFY, &nid);
}

BOOL CGendoWnd::SetTrayTips(LPCTSTR lpszTips)
{
    nid.uFlags = NIF_TIP;
    wsprintf(nid.szTip, _T("%s\r\n%s\r\n"), _T("跟斗云，随行而至！"), lpszTips);
    return ::Shell_NotifyIcon(NIM_MODIFY, &nid);    
}


VOID CGendoWnd::SetTrayConnectStatus(INT state, LPCTSTR svr, LPCTSTR proto)
{
    TCHAR status[128] = {0};
    if (state == 0) {
        /* Show connection */
        wsprintf(status, _T("%s: %s\r\n%s--%s\r\n"), _T("状态"), _T("连接成功"), svr, proto);
    }else{
        /* Show idle info */
        wsprintf(status, _T("%s: %s\r\n"), _T("状态"), _T("未连接"));
    }
    SetTrayTips(status);
}

void CGendoWnd::OnClick( TNotifyUI &msg )
{
	if( msg.pSender == m_pCloseBtn ) 
	{ 
		//PostQuitMessage(0); // 因为activex的原因，使用close可能会出现错误 
        ::ShowWindow(m_hWnd, HIDE_WINDOW);
        SetBubbleTips(_T("跟斗云已后台运行"), _T("提示"));
        return;
	}else if( msg.pSender == m_pMinBtn ) 
	{ 
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0); 
		return; 
	}else if( msg.pSender == m_pMaxBtn ) 
	{ 
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE, 0); 
		return; 
	}else if( msg.pSender == m_pRestoreBtn ) 
	{
		SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0);
		return;
	}
	else if( msg.pSender->GetName() == _T("quitbtn") ) 
	{
		PostQuitMessage(0); // 因为activex的原因，使用close可能会出现错误
	}else if(msg.pSender->GetName() == _T("connect")) {
        Connect();
	}else if(msg.pSender->GetName() == _T("disconnect")) {
        //Disconnect();
        /* Create new thread to handle hangup operation, keep main windows happy */
        HANDLE hthread = CreateThread(NULL,0, ThdDisconnectVPN, this, NULL, NULL);
        CloseHandle(hthread);
	}else if (msg.pSender->GetName() == _T("down_start")) {
		
    }else if (msg.pSender->GetName() == _T("welcome-name")) {
        if (!User.m_bLogin) {
            ShowLoginDlg(msg);
        }else{
            RopOpenURL(_T("http://www.gendocloud.com"), this->m_hWnd);
        }
    }else if (msg.pSender->GetName() == _T("refresh")) {
        RefreshServerList();
    }else if (msg.pSender->GetName() == _T("ping")) {
        PingServerList();
    }else if (msg.pSender->GetName() == _T("twitter-follow")) {
        RopOpenURL(_T("https://twitter.com/GendoCloud"), this->m_hWnd);
    }else if (msg.pSender->GetName() == _T("pptp")) {
        if (m_bLockSvr) return;
        CDuiString info;        
        info.Append(_T("PPTP为首选，无法连接时请尝试L2TP或SSL!"));
        m_ServerCur.iCurProto = PROTO_PPTP;
        wsprintf(m_ServerCur.sCurProtoName, _T("PPTP"));
        ShowNotice(info.GetData());
    }else if (msg.pSender->GetName() == _T("l2tp")) {
        if (m_bLockSvr) return;
        //m_pOptL2TP->ApplyAttributeList(_T("textcolor=\"#ff1493\""));
        CDuiString info;        
        info.Append(_T("L2TP共享密钥为\"gendovpn\"！"));
        m_ServerCur.iCurProto = PROTO_L2TP;
        wsprintf(m_ServerCur.sCurProtoName, _T("L2TP"));
        ShowNotice(info.GetData());
    }else if (msg.pSender->GetName() == _T("ssl-udp")) {
        if (m_bLockSvr) return;
        CDuiString info;        
        info.Append(_T("SSL-UDP速度最快，建议选用！"));
        m_ServerCur.iCurProto = PROTO_OPENVPN_UDP;
        wsprintf(m_ServerCur.sCurProtoName, _T("SSL-UDP"));
        ShowNotice(info.GetData());
    }else if (msg.pSender->GetName() == _T("ssl-tcp")) {
        if (m_bLockSvr) return;
        CDuiString info;        
        info.Append(_T("SSL-TCP连接速度较SSL-UDP慢，UDP连接失败时请选择TCP！"));
        m_ServerCur.iCurProto = PROTO_OPENVPN_TCP;
        wsprintf(m_ServerCur.sCurProtoName, _T("SSL-TCP"));
        ShowNotice(info.GetData());
    }
}



LRESULT CGendoWnd::OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	CControlUI* pHover = m_PaintManager.FindControl(pt);
	if( pHover == NULL ) return 0;
	/*演示悬停在下载列表的图标上时，动态变换下载图标状态显示*/
	if(pHover->GetName() == _T("down_ico"))
	{
		//MessageBox(NULL, _T("鼠标在某控件例如按钮上悬停后，对目标控件操作，这里改变了状态图标大小"), _T("DUILIB DEMO"), MB_OK);
		CButtonUI *pBtn = (CButtonUI *)pHover;
		if (!lstrcmp(pBtn->GetNormalImage(),_T("downlist_pause.png"))) {
			pBtn->ApplyAttributeList(_T("hotimage=\"file='downlist_run.png' dest='10,10,32,26'\" "));
		}else{
			pBtn->ApplyAttributeList(_T("hotimage=\"file='downlist_pause.png' dest='10,10,32,26'\" "));
		}
		
	}else if (pHover->GetName() == _T("welcome-name")) {
        if (!User.m_bLogin) {
            pHover->ApplyAttributeList(_T("textcolor=\"#FFFFFF00\" tooltip=\"点击弹出登录框\""));
        }
    }

    bHandled = FALSE;
	return 0;
}


void CGendoWnd::ShowUserInfo(void)
{
    CLabelUI *m_pUser       = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("name_text")));
    CLabelUI *m_pNick       = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("nick_text")));
    CLabelUI *m_pRegister   = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("register_text")));
    CLabelUI *m_pLevel      = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("level_text")));
    CLabelUI *m_pHobby      = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("hobby_text")));
    CLabelUI *m_pMonTraffic = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("qbyte_ratio")));
    CLabelUI *m_pMonTNotice = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("qbyte_text")));
    //CProgressUI *m_pMonPrg = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("qbyte_progress")));
    CLabelUI *m_pAllTraffic = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("hbyte_text")));
    CLabelUI *m_pAllTNotice = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("hbyte_title")));
    CLabelUI *m_pState      = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("state_text")));
    
    

    if (!m_pUser || !m_pNick || !m_pRegister || !m_pLevel || !m_pMonTraffic ||
        !m_pMonTNotice || !m_pAllTraffic || !m_pAllTNotice  || !m_pState || !m_pHobby) {
            return;
    }

    TCHAR tmp[128] = {0};

    m_pUser->SetText(User.m_sUser);
    m_pNick->SetText(User.m_sNick);
    m_pRegister->SetText(User.m_sRegister);

    if (User.m_iLevel == 0) {
        wsprintf(tmp, _T("%s -- %s"), _T("免费用户"), User.m_sQuotaName);
    }else{
        wsprintf(tmp, _T("%s -- %s"), _T("VIP用户"), User.m_sQuotaName);
    }
    m_pLevel->SetText(tmp);

    if (User.m_iHobby & 1 || User.m_iHobby == 0) {
        wsprintf(tmp, _T("%s"), _T("国外网络加速 "));
    }
    if (User.m_iHobby & 2) {
        wsprintf(tmp, _T("%s"), _T("国内网络加速 "));
    }
    if (User.m_iHobby & 4) {
        wsprintf(tmp, _T("%s"), _T("游戏加速 "));
    }
    if (User.m_iHobby & 8) {
        wsprintf(tmp, _T("%s"), _T("视频/下载加速 "));
    }
    m_pHobby->SetText(tmp);

    Int64 u1 = UserInfo.quota_bytes/1024/1024;
    INT   u2 = u1/1024;
    u1       = UserInfo.quota_used/1024/1024;
    INT   u3 = u1;
    u1       = UserInfo.history_bytes/1024/1024;
    INT   u4 = u1;

    wsprintf(tmp, _T("%dM/%dG"), u3, u2);
    m_pMonTraffic->SetText(tmp);

    if (u3 > u2*1024*0.3) {
        m_pMonTNotice->SetText(_T("流量快耗尽，升级为VIP畅享无限流量，多线路VIP专享服务!"));
        m_pMonTNotice->ApplyAttributeList(_T("textcolor=\"#ff1493\""));
    }else{
        m_pMonTNotice->SetText(_T("升级为VIP，享受无限流量/多线路/多协议，VIP专属服务!"));
        m_pMonTNotice->ApplyAttributeList(_T("textcolor=\"#ff1493\""));
    }

    wsprintf(tmp, _T("%dM"), u4);
    m_pAllTraffic->SetText(tmp);

    m_pAllTNotice->SetText(_T("勋章：小乌龟，下一等级：10G"));

    m_pState->SetText(_T("活力四射"));


    HANDLE hthread = CreateThread(NULL,0, ThdRefreshUserInfo, this, NULL, NULL);
    CloseHandle(hthread);
}

void CGendoWnd::OnSelectChanged( TNotifyUI &msg )
{
	if(msg.pSender->GetName() == _T("opt_server"))
	{
		static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tab_main")))->SelectItem(0);
        m_curTab = 0;
	}else if(msg.pSender->GetName() == _T("opt_user"))
	{
		static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tab_main")))->SelectItem(1);
        m_curTab = 1;
        ShowUserInfo();
	}
	else if(msg.pSender->GetName() == _T("opt_history"))
	{
		static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tab_main")))->SelectItem(2);
        m_curTab = 2;
	}
}

void CGendoWnd::ShowLoginDlg(TNotifyUI &msg)
{
    CGendoLoginWnd* pLoginFrame = new CGendoLoginWnd();
    if( pLoginFrame == NULL ) { Close(); return; }
    pLoginFrame->Create(m_hWnd, _T("Login"), UI_WNDSTYLE_DIALOG, 0, 0, 0, 0, 0, NULL);
    pLoginFrame->CenterWindow();
    pLoginFrame->ShowModal();
    if (User.m_bLogin) {
        CButtonUI *pLabel = static_cast<CButtonUI *>(m_PaintManager.FindControl(_T("welcome-name")));
        if (pLabel) {
            TCHAR welcome[128] = {0};
            wsprintf(welcome, _T("text=\"%s\" textcolor=\"#FFFFFF00\" tooltip=\"%s, 套餐: %s, 等级: %d\""), 
                     User.m_sUser,
                     User.m_iLevel?_T("VIP"):_T("免费用户"),
                     User.m_sQuotaName,
                     User.m_iLevel);
            pLabel->ApplyAttributeList(welcome);
        }
    }else{
        CLabelUI *pLabel = static_cast<CLabelUI *>(m_PaintManager.FindControl(_T("welcome-name")));
        if (pLabel) {
            TCHAR welcome[128] = {0};
            wsprintf(welcome, _T("text=\"%s\" textcolor=\"#FFFFFF00\" showhtml=\"true\""), _T("尚未登录!"));
            pLabel->ApplyAttributeList(welcome);
        }
    }
    RefreshServerList();
}

void CGendoWnd::Notify( TNotifyUI &msg )
{

    if (msg.sType == _T("windowinit")) {
        ShowLoginDlg(msg);
    }else if(msg.sType == _T("itemselect")) {
  
    }else{
        return WindowImplBase::Notify(msg);
    }
        /*
    else if (msg.sType == _T("itemselect")) {
    
        if (msg.pSender->GetName() == _T("protocol")) {
            if (m_bLockSvr) return;
            CDuiString info = m_pProtocol->GetText();
            if (info == _T("PPTP")) {
                info.Append(_T(" 首选，无法连接时请尝试L2TP或SSL!"));
                m_ServerCur.iCurProto = PROTO_PPTP;
                wsprintf(m_ServerCur.sCurProtoName, _T("PPTP"));
            }else if (info == _T("L2TP")) {
                info.Append(_T(" 共享密钥为\"gendovpn\"！"));
                m_ServerCur.iCurProto = PROTO_L2TP;
                wsprintf(m_ServerCur.sCurProtoName, _T("L2TP"));
            }else if (info == _T("SSL-UDP")) {
                info.Append(_T(" UDP速度最快，建议选用！"));
                m_ServerCur.iCurProto = PROTO_OPENVPN_UDP;
                wsprintf(m_ServerCur.sCurProtoName, _T("SSL-UDP"));
            }else if (info == _T("SSL-TCP")) {
                info.Append(_T(" TCP连接速度较SSL-UDP慢，UDP连接失败时请选择TCP！"));
                m_ServerCur.iCurProto = PROTO_OPENVPN_TCP;
                wsprintf(m_ServerCur.sCurProtoName, _T("SSL-TCP"));
            }
            ShowNotice(info.GetData());
        }*/

}

VOID CGendoWnd::ShowNotice(LPCTSTR str)
{
    CLabelUI *pInfo = GetDialMsgCtrl();
    assert(pInfo != NULL);
    CDuiString sNotice = str;
    pInfo->SetText(sNotice.GetData());
}


VOID CGendoWnd::ShowNotice(CHAR *str)
{
    TCHAR msg[128] = {0};
    INT len = 128;
    mb2wc(str, strlen(str), msg, &len);
    CLabelUI *pInfo = GetDialMsgCtrl();
    assert(pInfo != NULL);
    CDuiString sNotice = msg;
    pInfo->SetText(sNotice.GetData());
}

void CGendoWnd::ShowMsg(LPCTSTR str)
{
    CDuiString*  pStr = new CDuiString(str);

    EnterCriticalSection(&m_Lock);
    m_MsgPool.push_back(pStr);
    LeaveCriticalSection(&m_Lock);
}

LRESULT CGendoWnd::OnMouseWheel( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	// 解决ie控件收不到滚动消息的问题
	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	::ScreenToClient(m_PaintManager.GetPaintWindow(), &pt);
	CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("ie")));
	if( pControl && pControl->IsVisible() ) {
		RECT rc = pControl->GetPos();
		if( ::PtInRect(&rc, pt) ) {
			return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
		}
	}

	bHandled = FALSE;
	return 0;
}

//消息循环
LRESULT CGendoWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lRes = 0;
    switch( uMsg )
    {
    //case WM_USER_PROG:        lRes = OnUserProg(uMsg, wParam, lParam, bHandled); bHandled = TRUE; break;
    //case WM_USER_TEXTPRINT:   lRes = OnUserPrint(uMsg, wParam, lParam, bHandled);bHandled = TRUE; break;
    //case WM_COPYDATA:         lRes = OnCopyData(uMsg, wParam, lParam, bHandled); bHandled = TRUE; break;
        case WM_NOTIFYICON:
        {
            if (lParam == WM_LBUTTONDBLCLK){
                ::ShowWindow(m_hWnd,SW_SHOW);
                bHandled = TRUE;
            } 
            if (lParam == WM_RBUTTONDOWN){	
                HMENU hMenu1=CreatePopupMenu();
                AppendMenu(hMenu1,MF_STRING,IDM_SHOW,_T("&显示跟斗云"));
                AppendMenu(hMenu1,MF_STRING,IDM_EXIT,_T("&退出跟斗云"));
                POINT point;
                ::GetCursorPos(&point);
                SetForegroundWindow(m_hWnd);
                TrackPopupMenu(hMenu1,TPM_RIGHTBUTTON,point.x,point.y,0,m_hWnd,NULL);
                ::PostMessage(m_hWnd,WM_NULL,0,0);
                bHandled = TRUE;
            }
        }
        break;
        case WM_COMMAND:
        {
            if (IDM_SHOW == LOWORD(wParam)) {
                ::ShowWindow(m_hWnd,SW_SHOW);
                bHandled = TRUE;
            } else if (IDM_EXIT == LOWORD(wParam)) {
                SetBubbleTips(_T("跟斗云正在退出..."), _T("提示"));
                Sleep(3000);
                ::PostQuitMessage(0);
                DelTray();
                bHandled = TRUE;
            }
        }
        break;

        default:
        break;
    }

    if( bHandled ) return lRes;
    return WindowImplBase::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
}

LRESULT CGendoWnd::OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	// 有时会在收到WM_NCDESTROY后收到wParam为SC_CLOSE的WM_SYSCOMMAND
	if( wParam == SC_CLOSE ) {
		::PostQuitMessage(0L);
		bHandled = TRUE;
		return 0;
	}
	BOOL bZoomed = ::IsZoomed(*this);
	LRESULT lRes = CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	if( ::IsZoomed(*this) != bZoomed ) {
		if( !bZoomed ) {
			CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("maxbtn")));
			if( pControl ) pControl->SetVisible(false);
			pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("restorebtn")));
			if( pControl ) pControl->SetVisible(true);
		}
		else {
			CControlUI* pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("maxbtn")));
			if( pControl ) pControl->SetVisible(true);
			pControl = static_cast<CControlUI*>(m_PaintManager.FindControl(_T("restorebtn")));
			if( pControl ) pControl->SetVisible(false);
		}
	}
	return lRes;
}

void CGendoWnd::InitWindow()
{
	m_pCloseBtn   = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("closebtn")));
	m_pMaxBtn     = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("maxbtn")));
	m_pRestoreBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("restorebtn")));
	m_pMinBtn     = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("minbtn")));
	m_pListUI     = static_cast<CListUI*>(m_PaintManager.FindControl(_T("server_list")));
	m_pMsgList    = static_cast<CListUI*>(m_PaintManager.FindControl(_T("userinfo_list")));
    m_pUser       = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("user")));
    m_pUserPwd    = static_cast<CEditUI*>(m_PaintManager.FindControl(_T("password")));
    m_pLabelProtoInfo = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("protocol-info")));
    m_pUserInfoTrafficPrg = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("qbyte_progress")));
    m_pMsgList->SetItemTextColor(RGB(255,0,0));
    m_pConnectPrg = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("connect_progress")));
    m_pConnectPrg->SetMaxValue(100);
    m_pConnectBtn    = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("connect")));
    m_pDisConnectBtn = static_cast<CButtonUI*>(m_PaintManager.FindControl(_T("disconnect")));

    m_pOptPPTP    = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("pptp")));
    m_pOptL2TP    = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("l2tp")));
    //m_pOptSSLTCP  = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("ssl-tcp")));
    m_pOptSSLTCP  = NULL;
    m_pOptSSLUDP  = static_cast<COptionUI*>(m_PaintManager.FindControl(_T("ssl-udp")));

    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);

    if (WSAStartup( wVersionRequested, &wsaData )) {
        Dbg("Couldn't startup Winsock\n");
    }
    m_ServerList.SetDuiMainWnd((VOID*)this);

    CButtonUI *pBtn = (CButtonUI*)m_PaintManager.FindControl(_T("disconnect"));
    assert(pBtn != NULL);
    pBtn->SetEnabled(FALSE);

    m_hServerSync = CreateEvent(NULL, TRUE, FALSE, NULL);


    /*
    if (IsNeedUpdate()) {
        CreateUpdateExe();
    }*/
}

LPCTSTR CGendoWnd::GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
{
    TCHAR *sTxt = _T("Hello");
    pControl->SetUserData((LPCTSTR)sTxt);
    return pControl->GetUserData();
}

LRESULT CGendoWnd::OnChar( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	/*演示键盘消息的处理*/
	TCHAR press_char = (TCHAR)wParam;
	if(press_char == VK_BACK)
	{
		//MessageBox(NULL, _T("按下了回退键"), _T("DUILIB DEMO"), MB_OK);
	}
	else
	{
		bHandled = FALSE;
	}


	if (!bHandled)	
		return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
	else
	    return 0;
}


void WINAPI OnRasDialCallback(UINT unMsg, RASCONNSTATE rasconnstate, DWORD dwError)
{
    CDuiString str;
    assert(pMainWnd != NULL);
    switch(rasconnstate)
    {
    case RASCS_OpenPort:
        str= TEXT("正在打开端口...");
        pMainWnd->SetConnectProgreass(10);
        break;
    case RASCS_PortOpened:
        str= TEXT("端口打开正常...");
        pMainWnd->SetConnectProgreass(15);
        break;
    case RASCS_ConnectDevice:
        str= TEXT("正在连接设备...");
        pMainWnd->SetConnectProgreass(20);
        break;
    case RASCS_DeviceConnected:
        str= TEXT("设备连接正常...");
        pMainWnd->SetConnectProgreass(25);
        break;
    case RASCS_AllDevicesConnected:
        str= TEXT("所有设备连接正常...");
        break;
    case RASCS_Authenticate:
        str= TEXT("正在验证用户名/密码......");
        pMainWnd->SetConnectProgreass(40);
        break;
    case RASCS_AuthNotify:
        if (dwError == 0) {
            str= TEXT("服务器响应验证请求......");
            pMainWnd->SetConnectProgreass(50);
        }else{
            str= TEXT("服务器验证用户名/密码失败!!!");
            pMainWnd->SetConnectProgreass(50);
        }
        break;
    case RASCS_AuthRetry:
        str= TEXT("服务器要求重新验证用户名/密码...");
        break;
    case RASCS_AuthCallback:
        str= TEXT("验证请求返回...");
        pMainWnd->SetConnectProgreass(60);
        break;
    case RASCS_AuthChangePassword:
        str= TEXT("服务器请求改变密码...");
        break;
    case RASCS_AuthProject:
        str= TEXT("正在应用VPN接入网络设置，请等待...");
        pMainWnd->SetConnectProgreass(70);
        break;
    case RASCS_AuthLinkSpeed:
        str= TEXT("开始认证链路速率...");
        break;
    case RASCS_AuthAck:
        str= TEXT("认证结果确认中...");
        break;
    case RASCS_ReAuthenticate:
        str= TEXT("即将开始重新认证过程...");
        break;
    case RASCS_Authenticated:
        str= TEXT("认证成功!");
        pMainWnd->SetConnectProgreass(90);
        break;
    case RASCS_PrepareForCallback:
        str= TEXT("准备回调过程...");
        break;
    case RASCS_WaitForModemReset:
        str= TEXT("正在等待Modem设备重启...");
        break;
    case RASCS_WaitForCallback:
        str= TEXT("正在等待回调过程中...");
        break;
    case RASCS_Interactive:
        str= TEXT("正在交互处理中...");
        break;
    case RASCS_RetryAuthentication:
        str= TEXT("重新开始认证过程中...");
        break;
    case RASCS_CallbackSetByCaller:
        str= TEXT("回调过程设置中...");
        break;
    case RASCS_PasswordExpired:
        str= TEXT("对不起，用户密码过期!!!");
        break;
    case RASCS_Connected:
        str= TEXT("连接成功! VPN已经建立，祝您愉快畅游网络!");
        pMainWnd->SetConnectProgreass(100);
        Sleep(1000);
        pMainWnd->m_pConnectPrg->SetVisible(false);
        break;
    case RASCS_Disconnected:
        str= TEXT("连接已断开!!!");
        pMainWnd->SetConnectProgreass(0);
        break;
    default:
        str= TEXT("");
        break;
    }
    CLabelUI *pMsgCtrl = pMainWnd->GetDialMsgCtrl();

    assert(pMsgCtrl != NULL);
    pMsgCtrl->SetText(str.GetData());
}


INT CGendoWnd::CreateSvrUI(pserver_t pSvr, int index)
{
    int ret = 0;
    TCHAR SvrName[32] = {0};
    TCHAR SvrDesc[128]= {0};
    TCHAR SvrCountry[32] = {0};
    TCHAR SvrProto[64]= {0};
    TCHAR SvrPing[32] = {0};
    TCHAR SvrLoad[32] = {0};
    TCHAR SvrState[32]= {0};
    TCHAR SvrIP[32]   = {0};
    TCHAR SvrPort[32] = {0};

    TCHAR SvrNameCtrl[32] = {0};
    TCHAR SvrProtoCtrl[64]= {0};
    TCHAR SvrPingCtrl[32]= {0};
    TCHAR SvrLoadCtrl[32] = {0};
    TCHAR SvrStateCtrl[32]= {0};
    TCHAR SvrIPCtrl[32]   = {0};
    TCHAR SvrPortCtrl[32] = {0};

    ret = utf82wc((wchar_t*)pSvr->name, -1, SvrName, 32);
    ret = utf82wc((wchar_t*)pSvr->desc, -1, SvrDesc, 128);
    ret = utf82wc((wchar_t*)pSvr->location, -1, SvrCountry, 32);

    CListContainerElementUI *pListItem  = new CListContainerElementUI;
    CHorizontalLayoutUI     *pHLayout   = new CHorizontalLayoutUI;
    CLabelUI                *pLbName    = new CLabelUI;
    CLabelUI                *pLbDesc    = new CLabelUI;
    CLabelUI                *pLbProto   = new CLabelUI;
    CLabelUI                *pLbPing    = new CLabelUI;
    CLabelUI                *pLbLoad    = new CLabelUI;
    CLabelUI                *pLbState   = new CLabelUI;
    CLabelUI                *pLbCountry = new CLabelUI;

    wsprintf(SvrNameCtrl,  _T("name=\"name-%d\""),  index);
    wsprintf(SvrProtoCtrl, _T("name=\"proto-%d\""), index);
    wsprintf(SvrLoadCtrl,  _T("name=\"load-%d\""),  index);
    wsprintf(SvrStateCtrl, _T("name=\"state-%d\""), index);
    wsprintf(SvrIPCtrl,    _T("name=\"ip-%d\""),    index);
    wsprintf(SvrPortCtrl,  _T("name=\"port-%d\""),  index);


    pLbName->SetText(SvrName);
    pLbName->ApplyAttributeList(_T("width=\"220\" textcolor=\"#FF000000\" disabledtextcolor=\"#FFA7A6AA\" showhtml=\"true\" align=\"center\""));
    pLbName->ApplyAttributeList(SvrNameCtrl);


    pLbDesc->SetText(SvrDesc);
    pLbDesc->ApplyAttributeList(_T("width=\"160\" textcolor=\"#FFAAAAAA\" disabledtextcolor=\"#FFA7A6AA\" showhtml=\"true\" align=\"center\""));


    if (!lstrcmp(SvrCountry, _T("美国"))) {
       pLbCountry->SetBkImage(_T("country_us.jpg"));
       //RECT rct = {5,3,0,0};
       //pLbCountry->SetPadding(rct);
        pLbCountry->ApplyAttributeList(_T("width=\"21\" height=\"14\" padding=\"10,4,0,0\" align=\"center\" "));
    }else if (!lstrcmp(SvrCountry, _T("德国"))) {
        pLbCountry->SetBkImage(_T("country_germany.jpg"));
        pLbCountry->ApplyAttributeList(_T("width=\"21\" height=\"14\" padding=\"10,4,0,0\" align=\"center\""));
    }else if (!lstrcmp(SvrCountry, _T("日本"))) {
        pLbCountry->SetBkImage(_T("country_japan.jpg"));
        pLbCountry->ApplyAttributeList(_T("width=\"21\" height=\"14\" padding=\"10,4,0,0\" align=\"center\""));
    }else if (!lstrcmp(SvrCountry, _T("中国"))) {
        pLbCountry->SetBkImage(_T("country_china.jpg"));
        pLbCountry->ApplyAttributeList(_T("width=\"21\" height=\"14\" padding=\"10,4,0,0\" align=\"center\""));
    }

    if (pSvr->protocol & PROTO_PPTP) {
        lstrcat(SvrProto, _T("PPTP/"));
    }
    if (pSvr->protocol & PROTO_L2TP) {
        lstrcat(SvrProto, _T("L2TP/"));
    }
    if (pSvr->protocol & PROTO_OPENVPN_TCP || pSvr->protocol & PROTO_OPENVPN_UDP) {
        lstrcat(SvrProto, _T("SSL"));
    }
    pLbProto->SetText(SvrProto);
    pLbProto->ApplyAttributeList(_T("width=\"150\" textcolor=\"#FFAAAAAA\" disabledtextcolor=\"#FFA7A6AA\" showhtml=\"true\" align=\"center\""));


    //pLbPing->SetText(_T("测速中..."));
    pLbPing->SetBkImage(_T("speed.GIF"));
    pLbPing->ApplyAttributeList(_T("width=\"110\" height=\"20\" textcolor=\"#FFAAAAAA\" disabledtextcolor=\"#FFA7A6AA\" showhtml=\"true\" align=\"center\" "));

    wsprintf(SvrLoad, _T("%d/%d"), pSvr->cur_user, pSvr->max_user);
    pLbLoad->SetText(SvrLoad);
    pLbLoad->ApplyAttributeList(_T("width=\"70\" textcolor=\"#FFAAAAAA\" disabledtextcolor=\"#FFA7A6AA\" showhtml=\"true\" align=\"center\""));


    if (pSvr->state == VPN_STATE_IDLE) {
        lstrcpy(SvrState, _T("空闲"));
        pLbState->ApplyAttributeList(_T("width=\"70\" textcolor=\"#FFAAAAAA\" disabledtextcolor=\"#FFA7A6AA\" showhtml=\"true\" align=\"center\""));
    }else if (pSvr->state == VPN_STATE_NORMAL) {
        lstrcpy(SvrState, _T("正常"));
        pLbState->ApplyAttributeList(_T("width=\"70\" textcolor=\"#FFAAAAAA\" disabledtextcolor=\"#FFA7A6AA\" showhtml=\"true\" align=\"center\""));
    }else if (pSvr->state == VPN_STATE_BUSY) {
        lstrcpy(SvrState, _T("繁忙"));
        pLbState->ApplyAttributeList(_T("width=\"70\" textcolor=\"#FF6A6A\" disabledtextcolor=\"#FFA7A6AA\" showhtml=\"true\" align=\"center\""));
    }else if (pSvr->state == VPN_STATE_DOWN) {
        lstrcpy(SvrState, _T("维护中"));
        pLbState->ApplyAttributeList(_T("width=\"70\" textcolor=\"#FF6A6A\" disabledtextcolor=\"#FFA7A6AA\" showhtml=\"true\" align=\"center\""));
    }else{
        lstrcpy(SvrState, _T("正常"));
        pLbState->ApplyAttributeList(_T("width=\"70\" textcolor=\"#FFAAAAAA\" disabledtextcolor=\"#FFA7A6AA\" showhtml=\"true\" align=\"center\""));
    }
    pLbState->SetText(SvrState);

    /* Control to element */
    pHLayout->Add(pLbName);
    pHLayout->Add(pLbDesc);
    pHLayout->Add(pLbCountry);
    pHLayout->Add(pLbProto);
    pHLayout->Add(pLbPing);
    pHLayout->Add(pLbLoad);
    pHLayout->Add(pLbState);

    /* List item */
    pListItem->Add(pHLayout);

    /* List */
    EnterCriticalSection(&m_ListLock);
    m_pListUI->Add(pListItem);
    LeaveCriticalSection(&m_ListLock);

    /* List attribute in last for take effect */
    pHLayout->ApplyAttributeList(_T(" height=\"30\" bkcolor=\"#006B93B2\" inset=\"3,5,3,5\""));
    pListItem->ApplyAttributeList(_T("height=\"30\""));

    m_ServerList.Insert(index, pSvr, m_pListUI, pListItem,
                        pHLayout, pLbName, pLbDesc, pLbProto, pLbPing, pLbLoad, pLbState);

    return 0;
}






INT CGendoWnd::GetServerList(TCHAR *ip, INT port)
{
    int ret = -1, len = 0, i = 0;
    char buf[1024*100] = {0};
    RopSyncSocket Connection;
    if (Connection.Open(ip, port)) {
        LDebug(DEBUG_L_ALERT, "connect GAS fail=%d\n", WSAGetLastError());
        return -1;
    }

    RopSyncMsg SyncMsg(&Connection);

    pmsg_head_t    pMsg = (pmsg_head_t)buf;
    puser_login_t pUser = (puser_login_t)pMsg->payload;
    pMsg->msg = MSG_GAS_SERVER_LIST;
    pMsg->len = 0;

    ret = SyncMsg.SendMsg(pMsg, pMsg->len+sizeof(msg_head_t));
    if (ret <= 0) {
        LDebug(DEBUG_L_ALERT, "send server msg GAS fail!");
        return -1;
    }
    ret = SyncMsg.RecvAndWaitMsg(pMsg, 1024*90, 10*1000);
    if (ret <= 0) {
        LDebug(DEBUG_L_ALERT, "recv server msg GAS fail!");
        return -2;
    }

    pserver_list_t pAck = (pserver_list_t)pMsg->payload;

    LDebug(DEBUG_L_INFO, "VPN server list [%d]\n", pAck->svr_cnt);
    for (i = 0; i < pAck->svr_cnt; i++) {
        /*LDebug(DEBUG_L_INFO, "svr[%d]: ip='%s', name='%s', proto=%d, cap='%d/%d', ssl='%s', state='%d'\n",
              i,
              pAck->svr_ary[i].ip,
              pAck->svr_ary[i].name,
              pAck->svr_ary[i].protocol,
              pAck->svr_ary[i].cur_user,
              pAck->svr_ary[i].max_user,
              pAck->svr_ary[i].ssl_port,
              pAck->svr_ary[i].state);*/
    }
    
    memcpy(&m_SvrList, pAck, sizeof(server_list_t));

    HANDLE hthread = CreateThread(NULL,0, ThdShowSvrList, this, NULL, NULL);
    CloseHandle(hthread);
    return 0;
}


INT CGendoWnd::RefreshServerList(void)
{
    /* BugFix: List item operation MUST
     * done in main thread, otherwise it crashes
     * gendo, 2013-07
     */
    EnterCriticalSection(&m_ListLock);
    m_pListUI->RemoveAll();
    LeaveCriticalSection(&m_ListLock);
    Sleep(500);

    HANDLE hthread = CreateThread(NULL,0, ThdRefreshSvrList, this, NULL, NULL);
    CloseHandle(hthread);  
    return 0;
}


INT CGendoWnd::GetServerInfoByIndex(int index, pserver_t pSvr)
{
    int i = 0;
    if (index < 0 || index >=m_SvrList.svr_cnt) {
        Dbg("get server info array overflowed!");
        return -1;
    }

    if (0 == m_SvrList.svr_cnt) {
        Dbg("get server info array empty!");
        return -1;
    }

    memcpy(pSvr, &m_SvrList.svr_ary[index], sizeof(server_t));
    return 0;
}

void CGendoWnd::OnItemClick( TNotifyUI &msg )
{
    if (m_curTab != 0) return;

    int   index = ((CListContainerElementUI *)msg.pSender)->GetIndex();          
    TCHAR SvrNameCtrl[32] = {0};
    server_t svr;
    memset(&svr, 0, sizeof(server_t));

    if (m_bLockSvr) return;

    PGendoSvr pSvr = m_ServerList.Get(index);
    assert(pSvr != NULL);
    memcpy(&svr, &pSvr->SvrInfo, sizeof(server_t));
    memcpy(&m_ServerCur, pSvr, sizeof(GendoSvr)); 
    m_bSvrChoise = true;

    //wsprintf(SvrNameCtrl, _T("name-%d"), index);
    //CControlUI *pCurSvr =m_PaintManager.FindSubControlByName(msg.pSender, SvrNameCtrl);
    //if (!pCurSvr) return;
    CControlUI *pCurSvr = (CControlUI*)pSvr->pLbName;

    /* Update according server info */
    CControlUI *pInfo = m_PaintManager.FindControl(_T("cur-svr"));
    pInfo->SetText(pCurSvr->GetText());
    pInfo->ApplyAttributeList(_T("textcolor=\"#FFFF0000\""));

    CStdPtrArray* opw=m_PaintManager.GetOptionGroup(_T("proto"));
    if (NULL != opw) {
        int size=opw->GetSize();//组的大小
        for(int i=0;i < size;i++) {
           
        }
    }

    m_pOptPPTP->Selected(FALSE);
    m_pOptL2TP->Selected(FALSE);
    m_pOptSSLUDP->Selected(FALSE);
    //m_pOptSSLTCP->Selected(FALSE);

    m_pOptPPTP->SetEnabled(FALSE);
    m_pOptL2TP->SetEnabled(FALSE);
    m_pOptSSLUDP->SetEnabled(FALSE);
   // m_pOptSSLTCP->SetEnabled(FALSE);


    if (svr.protocol & PROTO_PPTP) {
        m_pOptPPTP->SetEnabled(TRUE);
    }
    if (svr.protocol & PROTO_L2TP) {
        m_pOptL2TP->SetEnabled(TRUE);
    }
    if (svr.protocol & PROTO_OPENVPN_UDP) {
        m_pOptSSLUDP->SetEnabled(TRUE);
    }
    if (svr.protocol & PROTO_OPENVPN_TCP) {
        //m_pOptSSLTCP->SetEnabled(TRUE);
    }

    /* Set default proto to PPTP, else SSL */
    if (svr.protocol & PROTO_OPENVPN_UDP) {
        m_pOptSSLUDP->Selected(TRUE);
    }else{
        m_pOptPPTP->Selected(TRUE);
    }

    return;
}


#if 0
void CGendoWnd::OnItemClick( TNotifyUI &msg )
{
    if (m_curTab != 0) return;

    int   index = ((CListContainerElementUI *)msg.pSender)->GetIndex();          
    TCHAR SvrNameCtrl[32] = {0};
    server_t svr;
    memset(&svr, 0, sizeof(server_t));

    if (m_bLockSvr) return;
    
    PGendoSvr pSvr = m_ServerList.Get(index);
    assert(pSvr != NULL);
    memcpy(&svr, &pSvr->SvrInfo, sizeof(server_t));
    memcpy(&m_ServerCur, pSvr, sizeof(GendoSvr)); 
    m_bSvrChoise = true;

    //wsprintf(SvrNameCtrl, _T("name-%d"), index);
    //CControlUI *pCurSvr =m_PaintManager.FindSubControlByName(msg.pSender, SvrNameCtrl);
    //if (!pCurSvr) return;
    CControlUI *pCurSvr = (CControlUI*)pSvr->pLbName;
    
    /* Update according server info */
    CControlUI *pInfo = m_PaintManager.FindControl(_T("cur-svr"));
    pInfo->SetText(pCurSvr->GetText());
    pInfo->ApplyAttributeList(_T("textcolor=\"#FFFF0000\""));

    CComboUI *pProto = (CComboUI*)m_PaintManager.FindControl(_T("protocol"));
    pProto->RemoveAll();

    CListLabelElementUI *pProtoItem = NULL;

    if (svr.protocol & PROTO_PPTP) {
        pProtoItem = new CListLabelElementUI;
        pProto->Add(pProtoItem);
        pProtoItem->ApplyAttributeList(_T("name=\"pptp\" text=\"PPTP\""));
    }
    if (svr.protocol & PROTO_L2TP) {
        pProtoItem = new CListLabelElementUI;
        pProto->Add(pProtoItem);
        pProtoItem->ApplyAttributeList(_T("name=\"l2tp\" text=\"L2TP\""));
    }
    if (svr.protocol & PROTO_OPENVPN_UDP) {
        pProtoItem = new CListLabelElementUI;
        pProto->Add(pProtoItem);
        pProtoItem->ApplyAttributeList(_T("name=\"ssl-udp\" text=\"SSL-UDP\""));
    }
    if (svr.protocol & PROTO_OPENVPN_TCP) {
        pProtoItem = new CListLabelElementUI;
        pProto->Add(pProtoItem);
        pProtoItem->ApplyAttributeList(_T("name=\"ssl-tcp\" text=\"SSL-TCP\""));
    }

    pProto->SelectItem(0);
    return;
}
#endif


INT CGendoWnd::PingServerList(void)
{
    int i = 0;

    /*
    for (i = 0; i < m_ServerList.Size(); i++) {
        GendoSvr *pSvr = m_ServerList.Get(i);
        HANDLE hthread = CreateThread(NULL,0, ThdPingSvr, pSvr, NULL, NULL);
        CloseHandle(hthread);   
    }*/


    HANDLE hthread = CreateThread(NULL,0, ThdPingSvr2, this, NULL, NULL);
    //HANDLE hthread = CreateThread(NULL,0, PingSvrByHttp, this, NULL, NULL);
    CloseHandle(hthread);

    return 0;
}

VOID CGendoWnd::CreateUpdateExe(VOID)
{

}


/* 
 * while openvpn connecting, we grasp its log
 * to nofity user the procedure
 */
INT CGendoWnd::ShowOpenVPNStatus(VOID)
{
    BOOL  bRet = FALSE;
    DWORD dwStart = GetCurrentTime();
    INT   ret = 1;
    while(!bRet) {
        TCHAR  sMsg[128] = {0};

        ret = m_ovpn.GetStatus(sMsg, OPENVPN_LOG_FILE);
        ShowNotice(sMsg);
        /* Make progress active */
        if (ret > 0) {
            m_pConnectPrg->SetValue(ret*16);
        }

        Sleep(1000);

        if (GetCurrentTime() - dwStart > 30*1000 || ret == 0) {
            bRet = TRUE;
        }

        if (ret == -1) {
            /* should return immediatly */
            bRet = TRUE;
        }
    }
    return ret;
};

/* Get random SSL port to survive GFW block */
int CGendoWnd::GetRandomSSLPort(INT *port_ary, INT cnt)
{
    int i = 0;
    srand(time(NULL));
    i = rand()%(cnt);
    ASSERT(i <= cnt - 1);
    int port = port_ary[i];
    return port;
}

/* Set connect progreass bar */
VOID CGendoWnd::SetConnectProgreass(INT status)
{
    m_pConnectPrg->SetValue(status);
}

DWORD WINAPI ThdOpvnVpnConnect(LPVOID lpParam)
{
    CGendoWnd *pWnd = (CGendoWnd*)lpParam;
    int ret = 0;
    ret = pWnd->m_ovpn.Connect();
    pWnd->m_ovpn.Cleanup();
    if (ret) {
        pWnd->ShowNotice(_T("SSL 连接过程中出现问题，请重新选择服务器，谢谢！"));
        return -1;
    }
    ret = pWnd->ShowOpenVPNStatus();

    if (ret == -1) {
        /* Permission deny, need admin right */
        pWnd->ShowNotice(_T("SSL 创建失败！权限不足，请以管理员方式运行客户端！"));
        Sleep(1000);
        pWnd->Disconnect();
    }else if (ret == 6) {
        /* Failed in 15 second, kill openvpn */
        pWnd->ShowNotice(_T("SSL 创建失败！正在修复Win8环境，请重试！"));
        pWnd->m_ovpn.FixWin8Up(OPENVPN_TAP_NAME);
        Sleep(1000);
        pWnd->Disconnect();
    }else if (ret) {
        /* Failed in 15 second, kill openvpn */
        pWnd->ShowNotice(_T("SSL 创建失败！正在恢复环境，请重试或使用其它协议。"));
        Sleep(2000);
        pWnd->Disconnect();
    }else{
        TCHAR ping[128] ={0};
        wsprintf(ping, _T("ping %s -n 10"), pWnd->m_ovpn.m_sIP);
        RopCreateProcess(ping, FALSE);
        pWnd->ShowNotice(_T("SSL 创建成功，祝您畅游跟斗云！"));
    }
    pWnd->m_pConnectPrg->SetVisible(false);
    pWnd->SetTrayConnectStatus(0, pWnd->m_ovpn.m_sSvrName, pWnd->m_ovpn.m_sProto);
    return ret;
}

DWORD WINAPI ThdShowSvrList(LPVOID lpParam)
{
    int i = 0;
    CGendoWnd *pWnd = (CGendoWnd *)lpParam;
    assert(pWnd != NULL);
    pserver_t pSvr = NULL;

    CLabelUI *pInfo = pWnd->GetDialMsgCtrl();
    CDuiString str = _T("正在刷新服务器列表......");
    pInfo->SetText(str.GetData());

    for (i = 0; i < pWnd->m_SvrList.svr_cnt; i++) {
        pSvr = &(pWnd->m_SvrList.svr_ary[i]);
        assert(pSvr != NULL);
        pWnd->CreateSvrUI(pSvr, i);
        Sleep(100);
    }

    str = _T("服务器列表刷新完成.");
    pInfo->SetText(str.GetData());
    pWnd->m_bServerListGot = TRUE;
    SetEvent(pWnd->m_hServerSync);

    return 0;
}

DWORD WINAPI ThdRefreshSvrList(LPVOID lpParam)
{
    int i = 0;
    CGendoWnd *pWnd = (CGendoWnd *)lpParam;
    assert(pWnd != NULL);

    pWnd->m_ServerList.RemoveAll();
    pWnd->m_bServerListGot = FALSE;
    ResetEvent(pWnd->m_hServerSync);
    if (pWnd->GetServerList(GENDO_GAS_IP, GENDO_GAS_PORT)) {
        Dbg("get server list fail\n");
        return -1;
    }

    DWORD dwRet = WaitForSingleObject(pWnd->m_hServerSync, 5000);
    pWnd->PingServerList();

    return 0;
}

DWORD WINAPI ThdPingSvr(LPVOID lpParam)
{
    int i = 0, iLatency = 0;
    TCHAR sTmp[MAX_NAME_LEN] = {0};
    GendoSvr *pSvr = (GendoSvr *)lpParam;
    assert(pSvr != NULL);
    CGendoWnd *pWnd = (CGendoWnd *)pSvr->pWnd;
    assert(pWnd != NULL);

    pSvr->pLbPing->SetText(_T("测速中..."));

    Sleep(1000);
    iLatency = PingLatency(pSvr->ip);
    wsprintf(sTmp, _T("%d ms"), iLatency);
    Sleep(1000);
    pSvr->pLbPing->SetText(sTmp);

    return 0;
}

typedef struct _svr_ping_s {
    void *svr;             // this 
    int   start;           // array start
    int   end;             // array end
}svr_ping_t;

#define PING_EACH_THREAD 3
#define HTTP_GET_LATENCY_FILE  _T("speed.html")

DWORD WINAPI ThdPingSvr3(LPVOID lpParam)
{
    int i = 0, iLatency = 0;
    TCHAR sTmp[MAX_NAME_LEN] = {0};
    svr_ping_t *pParam = (svr_ping_t *)lpParam;
    CGendoWnd *pWnd    = (CGendoWnd *)pParam->svr;
    int start = pParam->start;
    int end   = pParam->end;
    assert(pWnd != NULL);
    GendoSvr *pSvr = NULL;

    /* Last one */
    if (start == end) {
        pSvr = pWnd->m_ServerList.Get(start);
        pSvr->pLbPing->SetText(_T("测速中..."));
        pSvr->pLbPing->ApplyAttributeList(_T("textcolor=\"#3A5FCD\""));
        string url;
        DWORD diff1 = GetCurrentTime();
        url = RopHttpRequest(pSvr->ip, 80, HTTP_GET_LATENCY_FILE, _T("GET"), NULL, 0);
        iLatency = (GetCurrentTime() - diff1)/2;
        Dbg("IP='%S' => '%s'", pSvr->ip, url.c_str());
        wsprintf(sTmp, _T("%dms"), iLatency);
        pSvr->pLbPing->SetText(sTmp);
        if (iLatency <= 100) {
            pSvr->pLbPing->ApplyAttributeList(_T("textcolor=\"#458B74\""));
        }else if (iLatency <= 200) {
            pSvr->pLbPing->ApplyAttributeList(_T("textcolor=\"#2E8B57\""));
        }else{
            pSvr->pLbPing->ApplyAttributeList(_T("textcolor=\"#FF6A6A\""));
        }
    }

    for (i = start;i <= end && i < pWnd->m_ServerList.Size(); i++) {
        pSvr = pWnd->m_ServerList.Get(i);
        pSvr->pLbPing->SetText(_T("测速中..."));
        pSvr->pLbPing->ApplyAttributeList(_T("textcolor=\"#3A5FCD\""));
        string url;
        DWORD diff1 = GetCurrentTime();
        url = RopHttpRequest(pSvr->ip, 80, HTTP_GET_LATENCY_FILE, _T("GET"), NULL, 0);
        iLatency = (GetCurrentTime() - diff1)/2;
        //Dbg("IP='%S' => '%s'", pSvr->ip, url.c_str());
        wsprintf(sTmp, _T("%dms"), iLatency);
        pSvr->pLbPing->SetText(sTmp);
        if (iLatency <= 100) {
            pSvr->pLbPing->ApplyAttributeList(_T("textcolor=\"#458B74\""));
        }else if (iLatency <= 200) {
            pSvr->pLbPing->ApplyAttributeList(_T("textcolor=\"#2E8B57\""));
        }else{
            pSvr->pLbPing->ApplyAttributeList(_T("textcolor=\"#FF6A6A\""));
        }
    }

    CLabelUI *pInfo = pWnd->GetDialMsgCtrl();
    CDuiString str = _T("服务器延迟测试已完成......");
    pInfo->SetText(str.GetData());

    free(lpParam);
    return 0;
}


DWORD WINAPI ThdPingSvr2(LPVOID lpParam)
{
    int i = 0, iLatency = 0;
    TCHAR sTmp[MAX_NAME_LEN] = {0};
    CGendoWnd *pWnd = (CGendoWnd *)lpParam;
    assert(pWnd != NULL);
    GendoSvr *pSvr = NULL;

    CLabelUI *pInfo = pWnd->GetDialMsgCtrl();
    CDuiString str = _T("正在测试服务器延时，请稍后......");
    pInfo->SetText(str.GetData());

    int cnt1 = 0, cnt2 = 0;
    cnt1 = pWnd->m_ServerList.Size()/PING_EACH_THREAD;    // each thread ping 3 server
    cnt2 = pWnd->m_ServerList.Size()%PING_EACH_THREAD;

    for (i = 0; i < (cnt1+cnt2); i++) {
        svr_ping_t *ping = (svr_ping_t *)malloc(sizeof(svr_ping_t));
        ASSERT(ping != NULL);
        ping->svr   = lpParam;
        ping->start = i*PING_EACH_THREAD;
        ping->end   = (i+1)*PING_EACH_THREAD - 1;
        if (ping->end >= pWnd->m_ServerList.Size()) {
            ping->end = pWnd->m_ServerList.Size() - 1;
        }
        HANDLE hthread = CreateThread(NULL,0, ThdPingSvr3, ping, NULL, NULL);
        CloseHandle(hthread);
    }

    pInfo->SetText(str.GetData());

    return 0;
}

DWORD WINAPI ThdRefreshUserInfo(LPVOID lpParam)
{
    int i = 0, iLatency = 0;
    TCHAR sTmp[MAX_NAME_LEN] = {0};
    CGendoWnd *pWnd = (CGendoWnd *)lpParam;
    assert(pWnd != NULL);
    GendoSvr *pSvr = NULL;
    INT count = 0;
    INT step  = 10;

    CProgressUI *pPrg = pWnd->m_pUserInfoTrafficPrg;

    ASSERT(pPrg != NULL);
    UINT32 traffic = UserInfo.quota_used/1024;
    UINT32 val = traffic/step;
    UINT32 cur = val;
    UINT32 max = UserInfo.quota_bytes/1024;

    pPrg->SetMaxValue(max);

    while(count < step) {
        pPrg->SetValue(cur);
        Sleep(100);
        count++;
        cur += val;
    }

    return 0;
}



DWORD WINAPI ThdDisconnectVPN(LPVOID lpParam)
{
    CGendoWnd *pWnd = (CGendoWnd *)lpParam;
    ASSERT(pWnd!=NULL);
    CLabelUI *pInfo = pWnd->GetDialMsgCtrl();
    pWnd->m_pConnectPrg->SetValue(0);
    pWnd->m_pConnectPrg->SetVisible(true);
    CDuiString str = _T("正在断开VPN连接......");
    pWnd->m_pConnectPrg->SetValue(10);
    pInfo->SetText(str.GetData());
    Sleep(1*1000);
    pWnd->m_pConnectPrg->SetValue(20);
    /* close PPTP */
    pWnd->m_pptp.HangUp();
    pWnd->m_pConnectPrg->SetValue(50);
    /* close L2TP */
    pWnd->m_l2tp.HangUp();
    pWnd->m_pConnectPrg->SetValue(60);
    /* close ssl */
    pWnd->m_ovpn.DisConnect();
    Sleep(300);
    pWnd->m_pConnectPrg->SetValue(80);
    Sleep(200);
    pWnd->m_pConnectPrg->SetValue(90);
    Sleep(300);
    pWnd->m_pConnectPrg->SetValue(95);

    Sleep(100);
    str = _T("VPN连接已断开!");
    pInfo->SetText(str.GetData());
    pWnd->m_pConnectPrg->SetValue(100);

    pWnd->m_bLockSvr = FALSE;
    //pBtn->ApplyAttributeList(_T("disabledtextcolor=\"#FFA7A6AA\"  normalimage=\"file='toolbar\\start2.png' dest='3,3,18,18'\" hotimage=\"file='toolbar\\start2.png' dest='0,0,18,18'\""));
    pWnd->m_pConnectBtn->SetEnabled(TRUE);
    pWnd->m_pDisConnectBtn->SetEnabled(FALSE);
    pWnd->SetTrayConnectStatus(1, NULL, NULL);
    pWnd->m_pConnectPrg->SetVisible(false);
    return 0;
}


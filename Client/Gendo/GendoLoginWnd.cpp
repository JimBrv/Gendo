#include "StdAfx.h"
#include <Shellapi.h>
#include "GendoLoginWnd.h"
#include "RopUtil.h"
#include "RopProcess.h"
#include "RopAES.h"


DuiLib::CDuiString CGendoLoginWnd::GetSkinFolder()
{
    return _T("gendo-skin\\");	
}

DuiLib::CDuiString CGendoLoginWnd::GetSkinFile()
{
    return _T("login.xml");
}

LPCTSTR CGendoLoginWnd::GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
{
    TCHAR *sTxt = _T("Hello");
    pControl->SetUserData((LPCTSTR)sTxt);
    return pControl->GetUserData();
}

void CGendoLoginWnd::InitWindow()
{
    CComboUI* m_pAccountCombo = static_cast<CComboUI*>(m_pm.FindControl(_T("accountcombo")));
    CEditUI*  m_pAccountEdit  = static_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
    CEditUI*  m_pPwdEdit      = static_cast<CEditUI*>(m_pm.FindControl(_T("pwdedit")));
    CLabelUI* m_pLoginInfo    = static_cast<CLabelUI*>(m_pm.FindControl(_T("login_info")));
    COptionUI *m_pSaveme = static_cast<COptionUI*>(m_pm.FindControl(_T("saveme")));

    if (IsUserSaved()) {
        TCHAR name[32] = {0};
        TCHAR pwd[32]  = {0};

        GetUserPwdSaved(name, pwd);
        m_pAccountEdit->SetText(name);
        m_pPwdEdit->SetText(pwd);
        m_pSaveme->Selected(TRUE);
    }else{
        if( m_pAccountCombo && m_pAccountEdit ) m_pAccountEdit->SetText(m_pAccountCombo->GetText());
    }

    m_pAccountEdit->SetFocus();
}

void CGendoLoginWnd::OnFinalMessage(HWND /*hWnd*/) 
{ 
    m_pm.RemovePreMessageFilter(this);
    delete this; 
}


int CGendoLoginWnd::LoginGas(TCHAR *name, TCHAR *pwd)
{
    int ret = -1, len = 0;
    char UserName[MAX_NAME_LEN*2] = {0};
    char UserPwd[MAX_NAME_LEN*2]  = {0};
    char buf[1024] = {0};
    len = MAX_NAME_LEN;
    wc2mb(name, lstrlen(name), UserName, &len);
    len = MAX_NAME_LEN;
    wc2mb(pwd, lstrlen(pwd), UserPwd, &len);

    RopSyncSocket Connection;
    if (Connection.Open(GENDO_GAS_IP, GENDO_GAS_PORT)) {
        LDebug(DEBUG_L_ALERT, "connect GAS fail=%d\n", WSAGetLastError());
        return -1;
    }
    RopSyncMsg SyncMsg(&Connection);
    
    pmsg_head_t    pMsg = (pmsg_head_t)buf;
    puser_login_t pUser = (puser_login_t)pMsg->payload;
    pMsg->msg = MSG_GAS_USER_LOGIN;
    pMsg->len = sizeof(user_login_t);
    strcpy(pUser->name, UserName);
    strcpy(pUser->pwd,  UserPwd);
    strcpy(pUser->os,   User.GetOS());

    LDebug(DEBUG_L_INFO, "login user=[%s], os=[%s]\n", UserName, pUser->os);

    ret = SyncMsg.SendMsg(pMsg, pMsg->len+sizeof(msg_head_t));
    if (ret <= 0) {
        LDebug(DEBUG_L_ALERT, "send login msg GAS fail!");
        return -1;
    }
    ret = SyncMsg.RecvAndWaitMsg(pMsg, 1024);
    if (ret <= 0) {
        LDebug(DEBUG_L_ALERT, "send login msg GAS fail!");
        return -1;
    }

    puser_login_ack_t pAck = (puser_login_ack_t)pMsg->payload;
    if (pAck->ret) {
        LDebug(DEBUG_L_INFO, "auth user fail=0x%x!", pAck->ret);
        m_pLoginInfo = static_cast<CLabelUI*>(m_pm.FindControl(_T("login_info")));
        m_pLoginInfo->ApplyAttributeList(_T("bkcolor=\"#FFA7A6AA\""));
        m_pLoginInfo->SetText(_T("账号密码认证失败，请输入正确信息！"));
        return -1;
    }


    LDebug(DEBUG_L_INFO, "auth user ok: name='%s', active=%d, reg='%s', quota_name='%s', quota_cycle=%d, quota_use=%I64d, quota_bytes=%I64d, level=%d, type=%d\n",
           pAck->user.name, pAck->user.active, pAck->user.reg_time, pAck->user.quota_name,
           pAck->user.quota_cycle, pAck->user.quota_used, pAck->user.quota_bytes,
           pAck->user.level, pAck->user.type);

    TCHAR QtName[128] = {0};
    ret = utf82wc((wchar_t*)pAck->user.quota_name, wcslen((wchar_t*)pAck->user.quota_name), QtName, 128);
    if (ret <= 0) {
        LDebug(DEBUG_L_ERROR, "convert quota name fail!\n");
        lstrcpy(User.m_sQuotaName, _T("未获取成功!"));
    }else{
        lstrcpy(User.m_sQuotaName, QtName);
    }

    ret = utf82wc((wchar_t*)pAck->user.nick, wcslen((wchar_t*)pAck->user.nick), QtName, 128);
    if (ret <= 0) {
        LDebug(DEBUG_L_ERROR, "convert quota name fail!\n");
        lstrcpy(User.m_sNick, _T("未获取成功!"));
    }else{
        lstrcpy(User.m_sNick, QtName);
    }


    User.m_iQuotaTraffic     = pAck->user.quota_bytes;
    User.m_iQuotaUsedTraffic = pAck->user.quota_used;
    User.m_iHistoryTraffic   = pAck->user.history_bytes;
    User.m_iLevel            = pAck->user.level;
    User.m_iState            = pAck->user.active;
    User.m_iHobby            = pAck->user.hobby;
    User.m_iLoginCnt         = pAck->user.login_cnt;
     
    len = 32;
    TCHAR expire[32] = {0};
    mb2wc(pAck->user.quota_expire, strlen(pAck->user.quota_expire), expire, &len);
    lstrcpy(User.m_sQuotaExpire, expire);
    len = 32;
    mb2wc(pAck->user.reg_time, strlen(pAck->user.reg_time), expire, &len);
    lstrcpy(User.m_sRegister, expire);

    lstrcpy(User.m_sUser,   name);
    lstrcpy(User.m_sPwd,    pwd);
    m_sUser = name;
    m_sPwd  = pwd;

    extern user_t UserInfo;
    memcpy(&UserInfo, &pAck->user, sizeof(user_t));

    User.m_bLogin = TRUE;


    return 0;
}


INT CGendoLoginWnd::Login(CDuiString &sUser, CDuiString &sPwd)
{
    INT ret = 0;
    ret = LoginGas((LPTSTR)sUser.GetData(), (LPTSTR)sPwd.GetData());
   
    COptionUI *m_pSaveme = static_cast<COptionUI*>(m_pm.FindControl(_T("saveme")));

    if (m_pSaveme->IsSelected()) {
        SaveUserPwd((TCHAR *)sUser.GetData(), (TCHAR*)sPwd.GetData());
    }
    return ret;
}


void CGendoLoginWnd::Notify(TNotifyUI& msg)
{
    if( msg.sType == _T("click") ) {
        if( msg.pSender->GetName() == _T("closebtn")) { Close(); return; }
        else if(msg.pSender->GetName() == _T("loginBtn")) { 
            CEditUI* pAccountEdit   = static_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
            CEditUI* pPwdEdit       = static_cast<CEditUI*>(m_pm.FindControl(_T("pwdedit")));
            int ret = Login(pAccountEdit->GetText(), pPwdEdit->GetText());
            if (ret) {
                return;
            }
            Close();
            return;
        }else if(msg.pSender->GetName() == _T("cancel")) { 
            Close();
            return;
        }else if (msg.pSender->GetName() == _T("register")) {
            // register new user
            /*
            CActiveXUI* pActiveXUI = static_cast<CActiveXUI*>(m_pm.FindControl(_T("ie")));
            if( pActiveXUI ) {
                IWebBrowser2* pWebBrowser = NULL;
                pActiveXUI->GetControl(IID_IWebBrowser2, (void**)&pWebBrowser);
                if( pWebBrowser != NULL ) {
                    pWebBrowser->Navigate(L"http://www.gendocloud.com/register",NULL,NULL,NULL,NULL); 
                    pWebBrowser->Release();
                }
            }*/

            /*
            CWebBrowserUI  * pWebBrowserUI = static_cast<CWebBrowserUI *>(m_pm.FindControl(_T("ie")));
            pWebBrowserUI ->SetWebBrowserEventHandler(m_pWebBrowserEventHandler);
            if(  pWebBrowserUI != NULL )  {
            pWebBrowserUI ->Navigate2(_T("http://www.duilib.com")); 
            }
            */
            RopOpenURL(_T("http://www.gendocloud.com/register"), this->m_hWnd);
        }
    }
    else if( msg.sType == _T("itemselect") ) {
        if( msg.pSender->GetName() == _T("accountcombo") ) {
            CEditUI* pAccountEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
            if( pAccountEdit ) pAccountEdit->SetText(msg.pSender->GetText());
        }
    }
}

LRESULT CGendoLoginWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
    styleValue &= ~WS_CAPTION;
    ::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

    m_pm.Init(m_hWnd);
    m_pm.AddPreMessageFilter(this);
    CDialogBuilder builder;
    CControlUI* pRoot = builder.Create(_T("login.xml"), (UINT)0, NULL, &m_pm);
    ASSERT(pRoot && "Failed to parse XML");
    m_pm.AttachDialog(pRoot);
    m_pm.AddNotifier(this);
    InitWindow();
    return 0;
}

LRESULT CGendoLoginWnd::OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if( ::IsIconic(*this) ) bHandled = FALSE;
    return (wParam == 0) ? TRUE : FALSE;
}

LRESULT CGendoLoginWnd::OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0;
}

LRESULT CGendoLoginWnd::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    return 0;
}

LRESULT CGendoLoginWnd::OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    POINT pt; pt.x = GET_X_LPARAM(lParam); pt.y = GET_Y_LPARAM(lParam);
    ::ScreenToClient(*this, &pt);

    RECT rcClient;
    ::GetClientRect(*this, &rcClient);

    RECT rcCaption = m_pm.GetCaptionRect();
    if( pt.x >= rcClient.left + rcCaption.left && pt.x < rcClient.right - rcCaption.right \
        && pt.y >= rcCaption.top && pt.y < rcCaption.bottom ) {
            CControlUI* pControl = static_cast<CControlUI*>(m_pm.FindControl(pt));
            if( pControl && _tcscmp(pControl->GetClass(), _T("ButtonUI")) != 0 )
                return HTCAPTION;
    }

    return HTCLIENT;
}

LRESULT CGendoLoginWnd::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    SIZE szRoundCorner = m_pm.GetRoundCorner();
    if( !::IsIconic(*this) && (szRoundCorner.cx != 0 || szRoundCorner.cy != 0) ) {
        CDuiRect rcWnd;
        ::GetWindowRect(*this, &rcWnd);
        rcWnd.Offset(-rcWnd.left, -rcWnd.top);
        rcWnd.right++; rcWnd.bottom++;
        HRGN hRgn = ::CreateRoundRectRgn(rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, szRoundCorner.cx, szRoundCorner.cy);
        ::SetWindowRgn(*this, hRgn, TRUE);
        ::DeleteObject(hRgn);
    }

    bHandled = FALSE;
    return 0;
}

LRESULT CGendoLoginWnd::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT lRes = 0;
    BOOL bHandled = TRUE;
    switch( uMsg ) {
    case WM_CREATE:        lRes = OnCreate(uMsg, wParam, lParam, bHandled); break;
    case WM_NCACTIVATE:    lRes = OnNcActivate(uMsg, wParam, lParam, bHandled); break;
    case WM_NCCALCSIZE:    lRes = OnNcCalcSize(uMsg, wParam, lParam, bHandled); break;
    case WM_NCPAINT:       lRes = OnNcPaint(uMsg, wParam, lParam, bHandled); break;
    case WM_NCHITTEST:     lRes = OnNcHitTest(uMsg, wParam, lParam, bHandled); break;
    case WM_SIZE:          lRes = OnSize(uMsg, wParam, lParam, bHandled); break;
    default:
        bHandled = FALSE;
    }
    if( bHandled ) return lRes;
    if( m_pm.MessageHandler(uMsg, wParam, lParam, lRes) ) return lRes;
    return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
}

LRESULT CGendoLoginWnd::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
    if( uMsg == WM_KEYDOWN ) {
        if( wParam == VK_RETURN ) {
            CEditUI* pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("accountedit")));
            if( pEdit->GetText().IsEmpty() ) pEdit->SetFocus();
            else {
                pEdit = static_cast<CEditUI*>(m_pm.FindControl(_T("pwdedit")));
                if( pEdit->GetText().IsEmpty() ) pEdit->SetFocus();
                else Close();
            }
            return true;
        }
        else if( wParam == VK_ESCAPE ) {
            PostQuitMessage(0);
            return true;
        }

    }
    return false;
}

BOOL CGendoLoginWnd::IsUserSaved()
{
    CHAR upw[2][32] = {0};
    INT  row = 2;

    RopAES aes;
    INT ret = aes.DecryptFileToBufAry(GENDO_LOGIN_SAVE_PATH, GENDO_LOGIN_SAVE_FILE, (char *)upw, &row, 32);

    return ret==0 ? TRUE:FALSE;
}

VOID CGendoLoginWnd::SaveUserPwd(TCHAR *sUser, TCHAR *sPwd)
{
    CHAR upw[2][32] = {0};
    INT  len = 32;
    wc2mb(sUser, lstrlen(sUser), upw[0], &len);
    len = 32;
    wc2mb(sPwd, lstrlen(sPwd), upw[1], &len);

    RopAES aes;
    INT ret = aes.EncryptFileByBufAry(GENDO_LOGIN_SAVE_PATH, GENDO_LOGIN_SAVE_FILE, (char *)upw, 2, 32);
    if (ret) {
        LDebug(DEBUG_L_ALERT, "save user failed\n");
    }else{
        LDebug(DEBUG_L_ALERT, "save user info OK\n");
    }
    return;
}

INT CGendoLoginWnd::GetUserPwdSaved(TCHAR *user, TCHAR *pwd)
{
    RopAES aes;
    CHAR   upw[2][32] = {0};
    INT    row = 2;
    INT ret = aes.DecryptFileToBufAry(GENDO_LOGIN_SAVE_PATH, GENDO_LOGIN_SAVE_FILE, (char *)upw, &row, 32);
    if (ret) {
        LDebug(DEBUG_L_ALERT, "get saved user failed\n");
        return -1;
    }
    row = 32;
    mb2wc(upw[0], strlen(upw[0]), user, &row);
    row = 32;
    mb2wc(upw[1], strlen(upw[1]), pwd, &row);

    return 0;
}
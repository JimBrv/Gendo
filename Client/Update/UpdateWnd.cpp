#include "StdAfx.h"
#include <WinSock2.h>
#include "RopUtil.h"
#include "UpdateWnd.h"
#include "RopUpdate.h"
#include "RopProcess.h"
#include <Shellapi.h>

//////////////////////////////////////////////////////////////////////////
///

DUI_BEGIN_MESSAGE_MAP(CUpdateWnd, WindowImplBase)
    DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
DUI_END_MESSAGE_MAP()

CUpdateWnd::CUpdateWnd(void)
{

}

CUpdateWnd::~CUpdateWnd(void)
{
}

void CUpdateWnd::OnFinalMessage( HWND hWnd)
{
    __super::OnFinalMessage(hWnd);
    delete this;
}

DuiLib::CDuiString CUpdateWnd::GetSkinFolder()
{
    return _T("gendo-skin\\");
}

DuiLib::CDuiString CUpdateWnd::GetSkinFile()
{
    return _T("update.xml");
}

UILIB_RESOURCETYPE CUpdateWnd::GetResourceType() const
{
#ifdef _DEBUG
    return UILIB_FILE;
#else
    return UILIB_ZIP;
#endif
}

DuiLib::CDuiString CUpdateWnd::GetZIPFileName() const
{
    return _T("update-skin.zip");
}

LPCTSTR CUpdateWnd::GetWindowClassName( void ) const
{
    return _T("UpdateWnd");
}

LPCTSTR CUpdateWnd::GetItemText(CControlUI* pControl, int iIndex, int iSubItem)
{
    TCHAR *sTxt = _T("Hello");
    pControl->SetUserData((LPCTSTR)sTxt);
    return pControl->GetUserData();
}


LRESULT CUpdateWnd::OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
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

LRESULT CUpdateWnd::OnDownloadProg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    bHandled = TRUE;
    return 0;
}


LRESULT CUpdateWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    LRESULT lRes = 0;
    switch( uMsg )
    {
        //case WM_USER_PROG:        lRes = OnUserProg(uMsg, wParam, lParam, bHandled); bHandled = TRUE; break;
        //case WM_USER_TEXTPRINT:   lRes = OnUserPrint(uMsg, wParam, lParam, bHandled);bHandled = TRUE; break;
        //case WM_COPYDATA:         lRes = OnCopyData(uMsg, wParam, lParam, bHandled); bHandled = TRUE; break;
    default:
        bHandled = FALSE;
    }
    if( bHandled ) return lRes;
    return WindowImplBase::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
}



void FakeUpdateCB(unsigned int iNeedTime,  unsigned int iSubPhaseType, char *sName)
{
    return;
}




DWORD WINAPI ThdRefreshUpdateProgress(LPVOID lpParam)
{
    int ret = 0;
    CUpdateWnd *pWnd = (CUpdateWnd *)lpParam;
    assert(pWnd != NULL);

    Sleep(500);

    ret = Init(FakeUpdateCB, pWnd);
    if (ret) {
        goto out;
    }

    if (!IsNeedUpdate()) {
        goto out;
    }
    pWnd->m_pLbInfo->SetText(_T("正在更新"));
    
    pWnd->m_pLbUpdateNotice->SetText(_T("新版本改进："));
    TCHAR sMsg[5*512] = {0};
    UpdateNotice(sMsg, 5, 512);

    if (sMsg[0] != NULL) {
        pWnd->m_pLbUpdateMsg1->SetText(sMsg);
    }
    if (sMsg[1*512] != NULL) {
        pWnd->m_pLbUpdateMsg2->SetText(sMsg+512);
    }
    if (sMsg[2*512] != NULL) {
        pWnd->m_pLbUpdateMsg3->SetText(sMsg+2*512);
    }
    if (sMsg[3*512] != NULL) {
        pWnd->m_pLbUpdateMsg4->SetText(sMsg+3*512);
    }
    if (sMsg[4*512] != NULL) {
        pWnd->m_pLbUpdateMsg5->SetText(sMsg+4*512);
    }



    Update();

    Finl();

    pWnd->m_pLbSpeed->SetText(_T("更新完成，自动重启中..."));
    Sleep(3000);
out:
#ifdef _DEBUG
    //RopCreateProcess(_T("GendoUI_d.exe"), FALSE);
    UpErr("ShellEx!\n");
    HINSTANCE ht = ShellExecute(NULL, _T("open"), _T("GendoUI_d.exe"), NULL, NULL, SW_SHOWNORMAL);
    if ((unsigned int)ht <= 32) {
        UpErr("Create GendoUI.exe got error=%d!\n", ht);
    }
#else
    //RopCreateProcess(_T("GendoUI.exe"), FALSE);
    UpErr("ShellEx!\n");
    HINSTANCE ht = ShellExecute(NULL, _T("open"), _T("GendoUI.exe"), NULL, NULL, SW_SHOWNORMAL);
    if ((unsigned int)ht <= 32) {
        UpErr("Create GendoUI.exe got error=%d!\n", ht);
    }
#endif
    exit(0);

    return 0;
}





void CUpdateWnd::InitWindow()
{
    m_pProgress = static_cast<CProgressUI*>(m_PaintManager.FindControl(_T("progress")));
    m_pLbInfo   = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("update-space")));
    m_pLbItem   = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("update-item")));
    m_pLbSpeed  = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("update-speed")));
    m_pLbRatio  = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("update-ratio")));
    m_pLbUpdateNotice = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("update-notice")));
    m_pLbUpdateMsg1    = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("update-msg1")));
    m_pLbUpdateMsg2    = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("update-msg2")));
    m_pLbUpdateMsg3    = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("update-msg3")));
    m_pLbUpdateMsg4    = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("update-msg4")));
    m_pLbUpdateMsg5    = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("update-msg5")));

    ASSERT(m_pProgress != NULL && m_pLbItem != NULL && m_pLbSpeed != NULL);
     
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);

    if (WSAStartup( wVersionRequested, &wsaData )) {
        Dbg("Couldn't startup Winsock\n");
    }



    HANDLE hthread = CreateThread(NULL,0, ThdRefreshUpdateProgress, this, NULL, NULL);
    CloseHandle(hthread);  
}



void CUpdateWnd::OnClick( TNotifyUI &msg )
{
    /*
    if( msg.pSender == m_pCloseBtn ) 
    { 
        PostQuitMessage(0); // 因为activex的原因，使用close可能会出现错误
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
    }*/
}


void CUpdateWnd::Notify( TNotifyUI &msg )
{
    if (msg.sType == _T("windowinit")) {
       
    }else if (msg.sType == _T("itemselect")) {
        if (msg.pSender->GetName() == _T("protocol")) {
        }
    }else{
        return WindowImplBase::Notify(msg);
    }
}
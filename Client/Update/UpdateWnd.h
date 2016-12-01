#pragma once
#include "StdAfx.h"


//////////////////////////////////////////////////////////////////////////
///

class CUpdateWnd : public WindowImplBase, public IListCallbackUI
{
public:
    CUpdateWnd(void);
    ~CUpdateWnd(void);

    virtual void OnFinalMessage( HWND );
    virtual CDuiString GetSkinFolder();
    virtual CDuiString GetSkinFile();
    virtual LPCTSTR GetWindowClassName( void ) const;
    virtual void Notify( TNotifyUI &msg );
    virtual LRESULT OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
    virtual void    InitWindow();
    virtual UILIB_RESOURCETYPE GetResourceType() const;
    virtual CDuiString GetZIPFileName() const;
    virtual LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem);

    DUI_DECLARE_MESSAGE_MAP()
    virtual void OnClick(TNotifyUI& msg);

    LRESULT OnDownloadProg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

public:
    virtual LRESULT HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


public:
    VOID UpdateTest(VOID);


public:
   
    CLabelUI*    m_pLbItem;
    CLabelUI*    m_pLbSpeed;
    CLabelUI*    m_pLbRatio;
    CLabelUI*    m_pLbInfo;
    CProgressUI* m_pProgress;
    CLabelUI*    m_pLbUpdateNotice;
    CLabelUI*    m_pLbUpdateMsg1;
    CLabelUI*    m_pLbUpdateMsg2;
    CLabelUI*    m_pLbUpdateMsg3;
    CLabelUI*    m_pLbUpdateMsg4;
    CLabelUI*    m_pLbUpdateMsg5;
};

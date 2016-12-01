#pragma once
#include "StdAfx.h"
#include "GendoUser.h"
#include "RopSyncSocket.h"
#include "RopSyncMsg.h"

extern CGendoUser User;

#define GENDO_GAS_IP   _T("gendocloud.com")
#define GENDO_GAS_PORT 8082
#define GENDO_LOGIN_SAVE_FILE "gendo.save"
#define GENDO_LOGIN_SAVE_PATH "."

//class CGendoLoginWnd : public CWindowWnd, public INotifyUI, public IMessageFilterUI
class CGendoLoginWnd : public WindowImplBase, public IListCallbackUI
{
public:
    CGendoLoginWnd() {};
    LPCTSTR GetWindowClassName() const { return _T("UILoginFrame"); }
    UINT    GetClassStyle() const { return UI_CLASSSTYLE_DIALOG; }
    void    OnFinalMessage(HWND /*hWnd*/);

    virtual CDuiString GetSkinFolder();
    virtual CDuiString GetSkinFile();
    virtual LPCTSTR GetItemText(CControlUI* pControl, int iIndex, int iSubItem);

    virtual void InitWindow();

    INT Login(CDuiString &sUser, CDuiString &sPwd);

    void Notify(TNotifyUI& msg);

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled);

    /* GAS authentication */
    int LoginGas(TCHAR *name, TCHAR *pwd);

    /* save user information */
    BOOL IsUserSaved();
    VOID SaveUserPwd(TCHAR *sUser, TCHAR *sPwd);
    INT  GetUserPwdSaved(TCHAR *user, TCHAR *pwd);

public:
    CPaintManagerUI m_pm;
    CDuiString      m_sUser;
    CDuiString      m_sPwd;

private:
    CComboUI* m_pAccountCombo;
    CEditUI*  m_pAccountEdit;
    CEditUI*  m_pPwdEdit;
    CLabelUI* m_pLoginInfo;
    BOOL      m_bRember;
};

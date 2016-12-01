// Gendo update 
//

#include "stdafx.h"
#include <exdisp.h>
#include <comdef.h>
#include "UpdateWnd.h"

CUpdateWnd *pMainWnd;

#define MAINWNDTITLE _T("¸ú¶·ÔÆ")

BOOL SetForegroundWnd(LPCTSTR lpWindowName)
{
    HWND hWnd = FindWindow(NULL,lpWindowName);
    if (hWnd != NULL){
        if (IsIconic(hWnd)){
            ShowWindow(hWnd,SW_RESTORE);
        }
        ShowWindow(hWnd,SW_SHOW);
        SetForegroundWindow(hWnd);
        SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);
        SetWindowPos(hWnd,HWND_NOTOPMOST,0,0,0,0,SWP_NOSIZE | SWP_NOMOVE);
        return TRUE;
    }
    return FALSE;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, int nCmdShow)
{
    HANDLE hMutex = CreateMutex(NULL,TRUE, MAINWNDTITLE);
    if(GetLastError() == ERROR_ALREADY_EXISTS){
        CloseHandle(hMutex);
        SetForegroundWnd(MAINWNDTITLE);
        return 0;
    }

    CPaintManagerUI::SetInstance(hInstance);
    HRESULT Hr = ::CoInitialize(NULL);
    if( FAILED(Hr) ) return 0;

    CUpdateWnd* pFrame = new CUpdateWnd();
    if( pFrame == NULL ) return 0;
    pFrame->Create(NULL, _T("Gendo VPN-Auto update"), UI_WNDSTYLE_FRAME, 0L, 0, 0, 680, 450);
    pFrame->CenterWindow();

    //pFrame->LoadIcon(IDI_UPDATE);
    //pFrame->SetIcon(TRUE);
    pMainWnd = pFrame;
    ::ShowWindow(*pFrame, SW_SHOW);

    CPaintManagerUI::MessageLoop();

    ::CoUninitialize();
    return 0;
}

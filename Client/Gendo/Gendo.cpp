// RichListDemo.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include <exdisp.h>
#include <comdef.h>
#include "Resource.h"
#include "GendoWnd.h"

CGendoWnd *pMainWnd;

#define MAINWNDTITLE _T("跟斗云")

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

	CGendoWnd* pFrame = new CGendoWnd();
	if( pFrame == NULL ) return 0;
	pFrame->Create(NULL,  MAINWNDTITLE, UI_WNDSTYLE_FRAME, 0L, 0, 0, 850, 600);
	pFrame->CenterWindow();
    pFrame->SetInstance(hInstance);
    pFrame->SetIcon(IDI_RICHLISTDEMO);
    
    pMainWnd = pFrame;
	::ShowWindow(*pFrame, SW_SHOW);

    pFrame->AddTray();
	CPaintManagerUI::MessageLoop();

	::CoUninitialize();
	return 0;
}

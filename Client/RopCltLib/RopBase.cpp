#include "stdafx.h"
#include "windows.h"
#include "RopBase.h"

int RopBase::OSVersion()
{
    OSVERSIONINFO Vi;
    int RetVal = -1;

    Vi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    GetVersionEx(&Vi);

    switch(Vi.dwMajorVersion) {
    case 4:
        RetVal = WIN_2000;
        strcpy(m_os, "Win2000");
    break;
    case 5:
        switch(Vi.dwMinorVersion) {
        case 0:
            RetVal = WIN_2000;
            strcpy(m_os, "Win2000");
            break;
        case 1:
            RetVal = WIN_XP;
            strcpy(m_os, "WinXP");
            break;
        default:
            RetVal = WIN_2003;
            strcpy(m_os, "Win2003");
            break;
        }
    break;
    case 6:
        switch(Vi.dwMinorVersion) {
        case 0:
            RetVal = WIN_VISTA;
            strcpy(m_os, "WinVista");
        break;
        case 1:
            RetVal = WIN_7;
            strcpy(m_os, "Win7");
        break;
        case 2:
            RetVal = WIN_8;
            strcpy(m_os, "Win8");
        break;
        default:
            RetVal = WIN_8;
            strcpy(m_os, "Win8");
        break;
        }
    break;
    default:
        RetVal = WIN_2000;
        strcpy(m_os, "Win2000");
    break;
    }

    return RetVal;
}

char *RopBase::OSName()
{
    OSVersion();
    return m_os;
}

bool RopBase::IsOS64bit() 
{ 
    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL); 
    LPFN_ISWOW64PROCESS fnIsWow64Process; 
    BOOL bIsWow64 = FALSE; 
    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(_T("kernel32")), "IsWow64Process"); 
    if (NULL != fnIsWow64Process) { 
        fnIsWow64Process(GetCurrentProcess(),&bIsWow64);
    } 
    return bIsWow64; 
} 



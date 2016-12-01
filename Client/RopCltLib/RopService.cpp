
#include "stdafx.h"
#include "RopService.h"


SERVICE_STATUS_HANDLE RopService::m_ServiceStatusHandle = 0;
SERVICE_STATUS RopService::m_ServiceStatus;
SC_HANDLE RopService::m_ManagerHandle;
SC_HANDLE RopService::m_ServiceHandle;
HDEVNOTIFY RopService::m_DevNotifyHandle;
DEV_BROADCAST_DEVICEINTERFACE RopService::m_DevNotifyFilter;

TCHAR RopService::m_sSvcName[128];
TCHAR RopService::m_sDispName[128];
TCHAR RopService::m_sExeName[128];

void RopService::OpenSCM()
{
    if (! (m_ManagerHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))) {
       Dbg("OpenSCManager failed\n");
    }
}

void RopService::CloseSCM()
{
    CloseServiceHandle(m_ManagerHandle);
}


void RopService::OpenService(LPCTSTR SvcName)
{
    OpenSCM();
    if (! (m_ServiceHandle = ::OpenService(m_ManagerHandle, SvcName, SERVICE_ALL_ACCESS))) {
        CloseSCM();
        throw RopServiceException();
    }
    lstrcpy(m_sSvcName, SvcName);
}

void RopService::CloseService()
{
    CloseServiceHandle (m_ServiceHandle);
}

void RopService::GetSvcInfo(SERVICE_STATUS &Status)
{
    if(!QueryServiceStatus(m_ServiceHandle, &Status)) {
        Status.dwCurrentState = SERVICE_STOPPED;
    }
}

unsigned long RopService::GetSvcState(LPCTSTR SvcName)
{
    SERVICE_STATUS Status;

    try {
        Status.dwCurrentState = SERVICE_STOPPED;
        OpenService(SvcName);
        GetSvcInfo(Status);
        CloseService();
    } catch(...) {
    }

    return Status.dwCurrentState;
}

BOOL RopService::SvcExists(LPCTSTR SvcName)
{
    BOOL bReturn = FALSE;

    try {
        OpenService(SvcName);
        CloseService();
        bReturn = TRUE;
    } catch(...) {
    }

    return bReturn;
}

BOOL RopService::InstallSvc(LPCTSTR SvcName, LPCTSTR ExeName, LPCTSTR DispName)
{
    BOOL bReturn =  FALSE;

    if (!(bReturn = SvcExists(SvcName))) {
        try {
            if(SvcExists(SvcName)) {
                Dbg("Service [%s] already exists,failed to install\n", SvcName);
                throw RopServiceException();
            }

            OpenSCM();

            m_ServiceHandle = CreateService(m_ManagerHandle,
                                            SvcName,
                                            DispName,
                                            SERVICE_ALL_ACCESS,
                                            SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS ,
                                            SERVICE_AUTO_START,
                                            SERVICE_ERROR_NORMAL,
                                            ExeName,
                                            TEXT("NDIS"),
                                            NULL,
                                            NULL,
                                            NULL,
                                            NULL);

            if (m_ServiceHandle) {
                Dbg("service [%s] was successfully installed\n", SvcName);
                CloseService();
                bReturn = TRUE;
            }

            CloseSCM();
        } catch(...) {
        }
    }

    return bReturn;
}

BOOL RopService::RemoveSvc(LPCTSTR SvcName)
{
    BOOL bReturn = FALSE;

    if (StopSvc(SvcName)) {
        try {
            OpenService(SvcName);

            if (bReturn = DeleteService(m_ServiceHandle))
                Dbg("Delete service [%s] OK.\n", SvcName);
            else
                Dbg("Remove service [%s] Failed\n", SvcName);

            CloseService();
        } catch(...) {
        }
    }

    return bReturn;
}

BOOL RopService::StartSvc(LPCTSTR SvcName)
{
    BOOL bReturn = FALSE;

    if (!SvcExists(SvcName))
        Dbg("Service [%s] does not exist\n", SvcName);
    else if (GetSvcState(SvcName) == SERVICE_RUNNING)
        Dbg("Service [%s] has already been started\n", SvcName), bReturn = TRUE;
    else {
        try {
            OpenService(SvcName);
            Dbg("Starting service [%s]\n", SvcName);
            StartService(m_ServiceHandle, 0, NULL);
            CloseService();
        } catch (...) {
        }

        if (bReturn = WaitSvc(SvcName, SVC_STATE_STARTED)) {
            Dbg("Service [%s] has been started\n", SvcName);
        }
    }

    return bReturn;
}

BOOL RopService::StopSvc(LPCTSTR SvcName)
{
    BOOL bReturn = FALSE;

    if (!SvcExists(SvcName))
        Dbg("Service [%S] does not exist\n", SvcName);
    else if (GetSvcState(SvcName) == SERVICE_STOPPED)
        Dbg("Service [%s] has already been stopped\n", SvcName), bReturn = TRUE;
    else {
        SERVICE_STATUS Status;

        try {
            OpenService(SvcName);
            Dbg("Stopping service [%s]\n", SvcName);
            ControlService(m_ServiceHandle, SERVICE_CONTROL_STOP, &Status);
            Sleep(1000);
            CloseService();
        } catch (...) {
        }

        if (bReturn = WaitSvc(SvcName, SVC_STATE_STOPPED)) {
            Dbg("Service [%s] has been stopped\n", SvcName);
        }
    }

    return bReturn;
}

BOOL RopService::WaitSvc(LPCTSTR SvcName, SVC_STATE_QUERY iState)
{
    BOOL bReturn = FALSE;

    if (SvcExists(SvcName)) {
        for (int bRetry = 16; bRetry > 0 && ! bReturn; --bRetry) {
            switch (GetSvcState(SvcName)) 
            {
                    case SERVICE_RUNNING: {
                        if (iState == SVC_STATE_STARTED) bReturn = TRUE;
                        break;
                    }

                    case SERVICE_STOPPED: {
                        if (iState == SVC_STATE_STOPPED) bReturn = TRUE;
                        break;
                    }

                    case SERVICE_START_PENDING:
                    case SERVICE_STOP_PENDING:
                    default: {
                        break;
                    }
            }
            Sleep(1000);
        }
    }

    return bReturn;
}
BOOL RopService::InstallRopService()
{
    TCHAR sBuf[MAX_PATH + 1] = {0};
    GetSystemDirectory(sBuf, sizeof(sBuf));
    lstrcat(sBuf, L"\\");
	lstrcat(sBuf, m_sExeName);
    return InstallSvc(m_sSvcName, sBuf, m_sDispName);
}

BOOL RopService::RemoveRopService()
{
    return RemoveSvc(m_sSvcName);
}



BOOL RopService::NotifyUSBEvent(int iEvent)
{
#if 0
    char tmp[128] = {0};
    PModMsg pMsg   = (PModMsg)tmp;
    pmsg_head_t pMsgHead = (pmsg_head_t)pMsg->payload;
    int iTotalLen = 0;

    pMsg->sModule = MSG_SWITCH_MODULE;
    pMsg->dModule = CTRL_USER_MODULE;
    pMsg->tag     = MAGIC_TAG;
    pMsg->len     = sizeof(msg_head_t);
    pMsgHead->msg      = iEvent;
    pMsgHead->len      = 0;

    iTotalLen = sizeof(ModMsg) + pMsg->len;

    for (int i=0;i < ModuleNum;i++) {
        if ((pMsg->dModule & AllModules[i].ID) == AllModules[i].ID) {
            try {
                if (AllModules[i].Handle) {
                    Dbg( "Send USB event to SMNG!");
                    AllModules[i].Handle->Send((char*)pMsg, iTotalLen);
                }
            } catch (...) {
                Dbg("Send USB event to SMNG got exception!!!!!!!!\n");
                return FALSE;
            }
        }
    }
#endif
    return TRUE;

}

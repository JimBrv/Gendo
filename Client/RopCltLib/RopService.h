#ifndef ROP_SERVIE_H
#define ROP_SERVIE_H

#include "stdafx.h"
#include <windows.h>
#include <winsvc.h>
#include <dbt.h>
#include <tchar.h>

#include "RopBase.h"
#include "RopUtil.h"
#include "RopModule.h"


typedef enum SVC_STATE_QUERY
{
    SVC_STATE_STARTED,
    SVC_STATE_STOPPED
};

typedef enum {
    ROP_SERVICE_STOPPED,
    ROP_SERVICE_NOT_INSTALLED,
    ROP_SERVICE_PENDING,
    ROP_SERVICE_STARTED,
    ROP_SERVICE_FAILED
}RopServiceLevel;


class RopServiceException
{
};

class RopService : public RopBase
{
protected: static SERVICE_STATUS_HANDLE m_ServiceStatusHandle;
           static SERVICE_STATUS m_ServiceStatus;
           static SC_HANDLE m_ManagerHandle;
           static SC_HANDLE m_ServiceHandle;
           static HDEVNOTIFY m_DevNotifyHandle;      
           static DEV_BROADCAST_DEVICEINTERFACE m_DevNotifyFilter;

public:    static void OpenService (LPCTSTR SvcName);
           static void CloseService();

public:    static void OpenSCM();
           static void CloseSCM();

public:    static void GetSvcInfo (SERVICE_STATUS &Status);
           static unsigned long GetSvcState (LPCTSTR SvcName);

public:    static BOOL InstallSvc (LPCTSTR SvcName, LPCTSTR ExeName, LPCTSTR DispName);
           static BOOL WaitSvc (LPCTSTR SvcName, SVC_STATE_QUERY iState = SVC_STATE_STARTED);
           static BOOL StartSvc (LPCTSTR SvcName);
           static BOOL StopSvc (LPCTSTR SvcName);
           static BOOL RemoveSvc (LPCTSTR SvcName);
           static BOOL SvcExists (LPCTSTR SvcName);

public:    static BOOL StartRopService(LPCTSTR SvcName)      {WaitSvc(SvcName); return StartSvc(SvcName);}
           static BOOL StopRopService(LPCTSTR SvcName)       {return StopSvc(SvcName);}

           static BOOL RopServiceExists(LPCTSTR SvcName)     {return SvcExists(SvcName);}
           static unsigned long GetRopState(LPCTSTR SvcName) {return GetSvcState(SvcName);}

public:    static BOOL InstallRopService();
           static BOOL RemoveRopService();

public:    
    static void SetServiceInfo(LPCTSTR SvcName, LPCTSTR DispName, LPCTSTR ExeName) 
           {
               lstrcpy(m_sSvcName, SvcName);
               lstrcpy(m_sDispName, DispName);
               lstrcpy(m_sExeName, ExeName);
           }

public:
           static BOOL NotifyUSBEvent(int iEvent);    

private:
    static TCHAR m_sSvcName[128];
    static TCHAR m_sDispName[128];
    static TCHAR m_sExeName[128];
};
#endif

#include "stdafx.h"
#include "RopUtil.h"
#include "RopRouteTable.h"
#include "RopProcess.h"

RouteTable::RouteTable(void)
{
}


RouteTable::~RouteTable(void)
{
}


INT RouteTable::GetIfIdByName(TCHAR *interface)
{
    return -1;
}

BOOL RouteTable::IsRouteExist(TCHAR *dest, TCHAR *mask, TCHAR *gateway)
{
    bool bFound = false;
    PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
    PMIB_IPFORWARDROW pRow = NULL;
    DWORD dwSize = 0;
    BOOL  bOrder = FALSE;
    DWORD dwStatus = 0;
    CHAR  cNet[32] = {0};
    CHAR  cNetMask[32] = {0};
    CHAR  cGateWay[32] = {0};
    INT   iLen = 32;
    wc2mb(dest, lstrlen(dest), cNet, &iLen);
    iLen = 32;
    wc2mb(mask, lstrlen(mask), cNetMask, &iLen);
    iLen = 32;
    wc2mb(gateway, lstrlen(gateway), cGateWay, &iLen);

    pRow = (PMIB_IPFORWARDROW)malloc(sizeof(MIB_IPFORWARDROW));
    if (!pRow) {
        return bFound;
    }

    dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, bOrder);
    if (dwStatus == ERROR_INSUFFICIENT_BUFFER) {
        if (!(pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize))) {
            return bFound;
        }
        dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, bOrder);
    }

    if (dwStatus != ERROR_SUCCESS) {
        if (pIpForwardTable)
            free(pIpForwardTable);
        return bFound;
    }

    for (unsigned int j=0; j < pIpForwardTable->dwNumEntries; j++) {
        if (pIpForwardTable->table[j].dwForwardDest == inet_addr(cNet) && 
            pIpForwardTable->table[j].dwForwardMask == inet_addr(cNetMask) &&
            pIpForwardTable->table[j].dwForwardNextHop == inet_addr(cGateWay)) {
                bFound = true;
                break;
        }
    }        

    if (pIpForwardTable)
        free(pIpForwardTable);
    if (pRow)
        free(pRow);
    return bFound;
}

INT RouteTable::AddRoute(TCHAR *dest, TCHAR *mask, TCHAR *gateway, INT metric, TCHAR *interface)
{
    TCHAR command[1024] = {0};

    if (IsRouteExist(dest, mask, gateway)) {
        LDebug(DEBUG_L_INFO, "Route %S/%S/%S exist\n", dest, mask, gateway);
        return 0;
    }

    if (lstrlen(interface) == 0) {
        wsprintf(command, _T("route add %s MASK %s %s METRIC %d"), dest, mask, gateway, metric);
    } else {
        int id = GetIfIdByName(interface);
        wsprintf(command, _T("route add %s MASK %s %s METRIC %d IF %d"), dest, mask, gateway, metric, id);
    }

    if (RopCreateProcess(command)!=0) {
        LDebug(DEBUG_L_ERROR,"FAILED::%s",command);
        return -1;
    }

    return 0;
}

INT RouteTable::DelRoute(TCHAR *dest, TCHAR *mask, TCHAR *gateway)
{
    TCHAR command[1024] = {0};
    wsprintf(command, _T("route delete %s MASK %s %s"), dest, mask, gateway);

    if (!IsRouteExist(dest, mask, gateway)) {
        LDebug(DEBUG_L_INFO, "Route %S/%S/%S Not-exist\n", dest, mask, gateway);
        return 0;
    }

    if (RopCreateProcess(command)!=0) {
        LDebug(DEBUG_L_ERROR,"FAILED::%s",command);
        return -1;
    }

    return 0;
}


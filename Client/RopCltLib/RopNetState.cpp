#include "stdafx.h"
#include <shlwapi.h>
#include "RopNetState.h"
#include "stdio.h"
#include "Iphlpapi.h"
#include "RopUtil.h"
#include "RopRegistry.h"


NetState::NetState()
{
    m_bInit = FALSE;
    ZeroMemory(&m_NdisAry, sizeof(m_NdisAry));
}

NetState::~NetState()
{

}

#define IsSkipNdis(ND_NAME) (StrStrIA(ND_NAME, "vmware") || StrStrIA(ND_NAME, "loop") || \
    StrStrIA(ND_NAME,"virtual")   || StrStrIA(ND_NAME, "TAP") || \
    StrStrIA(ND_NAME, "wireless") || StrStrIA(ND_NAME, "3G")  || \
    StrStrIA(ND_NAME, "wifi") || StrStrIA(ND_NAME, "VPN"))

int NetState::GetAllNdis()
{
    DWORD dwResult = 0;
    bool  bTrue    = true;
    ULONG uSize    = 0;
    ULONG uOutNum  = 0;
    PIP_ADAPTER_INFO padp = NULL;
    memset(&m_NdisAry,0,sizeof(m_NdisAry));
    try {
        /* Get All adapter */
        dwResult = GetAdaptersInfo(padp, &uSize);
        if (ERROR_BUFFER_OVERFLOW == dwResult) {
            padp = (PIP_ADAPTER_INFO)malloc(uSize);
            if (!padp) {
                LDebug(DEBUG_L_ERROR, "Malloc adapter list failed,byte=%d!\n", uSize);
                return -1;
            }
            dwResult = GetAdaptersInfo(padp, &uSize);
            if (ERROR_SUCCESS == dwResult) {
                PIP_ADAPTER_INFO padp_cur = padp;
                while (padp_cur && uOutNum < MAX_NDIS_ITEMS) {
                    PNdisItem pNdis = NULL;
                    char sDesc[256] = {0};
                    char sType[32]  = {0};
                    pNdis = &m_NdisAry.Items[m_NdisAry.iNum];
                    switch(padp_cur->Type) {
                        case MIB_IF_TYPE_PPP:
                            sprintf(sType, "PPPoE");
                            strcpy(pNdis->sName, padp_cur->Description);
                            strcpy(pNdis->sIP, padp_cur->IpAddressList.IpAddress.String);
                            if (strstr(pNdis->sName, "Rop")) {
                                pNdis->iType  = NDIS_TYPE_ROP_PPPOE;
                            }else{
                                pNdis->iType  = NDIS_TYPE_OTHER_PPPOE;
                            }
                            pNdis->iState = NDIS_STATE_INACTIVE;
                            m_NdisAry.iNum++;
                            break;
                        case MIB_IF_TYPE_ETHERNET:
                            sprintf(sType, "Ethernet");
                            strcpy(pNdis->sName, padp_cur->Description);
                            strcpy(pNdis->sIP, padp_cur->IpAddressList.IpAddress.String);
                            pNdis->iType  = NDIS_TYPE_NETCARD;
                            pNdis->iState = NDIS_STATE_INACTIVE;
                            m_NdisAry.iNum++;
                            break;
                        case MIB_IF_TYPE_LOOPBACK:
                            sprintf(sType, "Loopback");
                            break;
                        default:
                            sprintf(sType, "Other");
                            break;
                    }
                    sprintf(sDesc, "Adapter Info: Name='%s', type='%s', IP='%s'\n",
                            padp_cur->Description, 
                            sType,
                            padp_cur->IpAddressList.IpAddress.String);
                    LDebug(DEBUG_L_ALL, sDesc);
                    padp_cur = padp_cur->Next;
                }
                bTrue = true;
            }else{
                LDebug(DEBUG_L_ERROR, "GetAdapterInfo() failed=%d!\n", GetLastError());
                return -1;
            }
        }
        if (padp) free(padp);
        
        m_bInit = TRUE;
        return 0;
    } catch(...) {
        LDebug(DEBUG_L_ERROR, "DetectNet got exception\n");
        if (padp) free(padp);
        return -1;
    }
}


void NetState::DumpAllNdis()
{
    if (!m_bInit) GetAllNdis();
    int i = 0;
    for (i = 0; i < m_NdisAry.iNum;i++) {
        char sDesc[256] = {0};
        char sType[32]  = {0};
        PNdisItem pNdis = &m_NdisAry.Items[i];
        switch(pNdis->iType) {
            case NDIS_TYPE_NETCARD:
                sprintf(sType, "Ethernet");
                break;
            case NDIS_TYPE_ROP_PPPOE:
            case NDIS_TYPE_OTHER_PPPOE:
                sprintf(sType, "PPPoE");
                break;
            case NDIS_TYPE_WLAN:
                sprintf(sType, "Ethernet WLAN");
                break;
            case NDIS_TYPE_VMWARE:
                sprintf(sType, "Ethernet VMware");
                break;
            case NDIS_TYPE_VPN:
                sprintf(sType, "Ethernet VPN");
                break;
            default:
                sprintf(sType, "Other");
                break;
        }
        sprintf(sDesc, "Adapters: Name='%s', type='%s', IP='%s'\n",
                pNdis->sName, 
                sType,
                pNdis->sIP);
        LDebug(DEBUG_L_DEBUG, sDesc);
    }
}

/*
 * Note: Don't use it, for wireless NIC type is uncertained in xp, win7, win8
 */
int NetState::GetEthNic()
{
    DWORD dwResult = 0;
    bool  bTrue    = true;
    ULONG uSize    = 0;
    ULONG uOutNum  = 0;
    PIP_ADAPTER_INFO padp = NULL;
    memset(&m_NdisAry,0,sizeof(m_NdisAry));
    try {
        /* Get All adapter */
        dwResult = GetAdaptersInfo(padp, &uSize);
        if (ERROR_BUFFER_OVERFLOW == dwResult) {
            padp = (PIP_ADAPTER_INFO)malloc(uSize);
            if (!padp) {
                LDebug(DEBUG_L_ERROR, "Malloc adapter list failed,byte=%d!\n", uSize);
                return -1;
            }
            dwResult = GetAdaptersInfo(padp, &uSize);
            if (ERROR_SUCCESS == dwResult) {
                PIP_ADAPTER_INFO padp_cur = padp;
                while (padp_cur && uOutNum < MAX_NDIS_ITEMS) {
                    PNdisItem pNdis = NULL;
                    char sDesc[256] = {0};
                    char sType[32]  = {0};
                    pNdis = &m_NdisAry.Items[m_NdisAry.iNum];
                    switch(padp_cur->Type) {
                    case MIB_IF_TYPE_ETHERNET:
                        sprintf(sType, "Ethernet");
                        strcpy(pNdis->sName, padp_cur->Description);
                        strcpy(pNdis->sIP, padp_cur->IpAddressList.IpAddress.String);
                        pNdis->iType  = NDIS_TYPE_NETCARD;
                        m_NdisAry.iNum++;
                        break;
                    default:
                        sprintf(sType, "Other");
                        break;
                    }
                    sprintf(sDesc, "Adapter Info: Name='%s', type='%s', IP='%s'\n",
                        padp_cur->Description, 
                        sType,
                        padp_cur->IpAddressList.IpAddress.String);
                    LDebug(DEBUG_L_ALL, sDesc);
                    padp_cur = padp_cur->Next;
                }
                bTrue = true;
            }else{
                LDebug(DEBUG_L_ERROR, "GetAdapterInfo() failed=%d!\n", GetLastError());
                return -1;
            }
        }
        if (padp) free(padp);

        m_bInit = TRUE;
        return 0;
    } catch(...) {
        LDebug(DEBUG_L_ERROR, "DetectNet got exception\n");
        if (padp) free(padp);
        return -1;
    }
}

void NetState::DumpEthNdis()
{
    if (!m_bInit) GetEthNic();
    int i = 0;
    for (i = 0; i < m_NdisAry.iNum;i++) {
        char sDesc[256] = {0};
        char sType[32]  = {0};
        PNdisItem pNdis = &m_NdisAry.Items[i];
        switch(pNdis->iType) {
        case NDIS_TYPE_NETCARD:
            sprintf(sType, "Ethernet");
            break;
        case NDIS_TYPE_ROP_PPPOE:
        case NDIS_TYPE_OTHER_PPPOE:
            sprintf(sType, "PPPoE");
            break;
        case NDIS_TYPE_WLAN:
            sprintf(sType, "Ethernet WLAN");
            break;
        case NDIS_TYPE_VMWARE:
            sprintf(sType, "Ethernet VMware");
            break;
        case NDIS_TYPE_VPN:
            sprintf(sType, "Ethernet VPN");
            break;
        default:
            sprintf(sType, "Other");
            break;
        }
        sprintf(sDesc, "Adapters: Name='%s', type='%s', IP='%s'\n",
            pNdis->sName, 
            sType,
            pNdis->sIP);
        LDebug(DEBUG_L_DEBUG, sDesc);
    }
}



BOOL NetState::GetNicFromDesc(TCHAR *desc, IP_ADAPTER_INFO *pAdp)
{
    DWORD dwResult = 0;
    BOOL  bTrue    = FALSE;
    ULONG uSize    = 0;
    ULONG uOutNum  = 0;
    PIP_ADAPTER_INFO padp = NULL;
    CHAR  sDesc[128] = {0};
    CHAR  sInfo[512] = {0};
    int   len = 128;
    wc2mb(desc, lstrlen(desc), sDesc, &len);
    memset(pAdp, 0, sizeof(IP_ADAPTER_INFO));

    try {
        /* Get All adapter */
        dwResult = GetAdaptersInfo(padp, &uSize);
        if (ERROR_BUFFER_OVERFLOW == dwResult) {
            padp = (PIP_ADAPTER_INFO)malloc(uSize);
            if (!padp) {
                LDebug(DEBUG_L_ERROR, "Malloc adapter list failed,byte=%d!\n", uSize);
                return FALSE;
            }
            dwResult = GetAdaptersInfo(padp, &uSize);
            if (ERROR_SUCCESS == dwResult) {
                PIP_ADAPTER_INFO padp_cur = padp;
                while (padp_cur && uOutNum < MAX_NDIS_ITEMS) {
                    sprintf(sInfo, "NicDump: Desc='%s', len=(%d, %d), type='%d', IP='%s', Guid='%s'\n",
                        padp_cur->Description, 
                        strlen(padp_cur->Description),
                        strlen(sDesc),
                        padp_cur->Type, 
                        padp_cur->IpAddressList.IpAddress.String,
                        padp_cur->AdapterName);
                    LDebug(DEBUG_L_DEBUG, sInfo);

                    if (!strncmp(sDesc, padp_cur->Description, strlen(padp_cur->Description))) {
                        sprintf(sInfo, "Find it: Desc='%s', len=(%d, %d), type='%d', IP='%s', Guid='%s'\n",
                            padp_cur->Description, 
                            strlen(padp_cur->Description),
                            strlen(sDesc),
                            padp_cur->Type, 
                            padp_cur->IpAddressList.IpAddress.String,
                            padp_cur->AdapterName);
                        LDebug(DEBUG_L_DEBUG, sInfo);
                        memcpy(pAdp, padp_cur, sizeof(IP_ADAPTER_INFO));
                        bTrue = TRUE;
                        break;
                    }
                    padp_cur = padp_cur->Next;
                }
            }else{
                LDebug(DEBUG_L_ERROR, "GetAdapterInfo() failed=%d!\n", GetLastError());
                return -1;
            }
        }
        if (padp) free(padp);
        return bTrue;
    } catch(...) {
        LDebug(DEBUG_L_ERROR, "DetectNet got exception\n");
        if (padp) free(padp);
        return -1;
    }
    return 0;
}

#include "wlanapi.h"
#pragma comment(lib, "wlanapi.lib")

int NetState::GetWirelessNicCount()
{
    DWORD pdwNegotiatedVersion;
    HANDLE phClientHandle;
    PWLAN_INTERFACE_INFO_LIST wiiList;
    DWORD dwRet = 0;
    INT cnt = 0, i = 0;

    dwRet = WlanOpenHandle(1,NULL,&pdwNegotiatedVersion,&phClientHandle);
    if (dwRet != ERROR_SUCCESS) {
        LDebug(DEBUG_L_DEBUG, "GetWirelessNicCount failed=%d\n", dwRet);
        return -1;
    }

    dwRet = WlanEnumInterfaces(phClientHandle,NULL,&wiiList);
    if (dwRet != ERROR_SUCCESS) {
        LDebug(DEBUG_L_DEBUG, "GetWirelessNicCount Enum failed=%d\n", dwRet);
        return -1;
    }
    
    cnt = wiiList->dwNumberOfItems;
    for (i = 0; i < cnt; i++) {
       // LDebug(DEBUG_L_DEBUG, "Wireless NIC[%d]: name=%S, connected=%s\n", i,
       //                        wiiList->InterfaceInfo[i].strInterfaceDescription,
       //                        wiiList->InterfaceInfo[i].isState == wlan_interface_state_connected ? "true":"false");
    }

    WlanFreeMemory(wiiList);
    WlanCloseHandle(phClientHandle, NULL);
    return cnt;
}

int NetState::GetWirelessNicState()
{
    DWORD pdwNegotiatedVersion;
    HANDLE phClientHandle;
    PWLAN_INTERFACE_INFO_LIST wiiList;
    DWORD dwRet = 0;
    INT cnt = 0;

    dwRet = WlanOpenHandle(1,NULL,&pdwNegotiatedVersion,&phClientHandle);
    if (dwRet != ERROR_SUCCESS) {
        LDebug(DEBUG_L_DEBUG, "GetWirelessNicState failed=%d\n", dwRet);
        return 0;
    }

    dwRet = WlanEnumInterfaces(phClientHandle,NULL,&wiiList);
    if (dwRet != ERROR_SUCCESS) {
        LDebug(DEBUG_L_DEBUG, "GetWirelessNicState Enum failed=%d\n", dwRet);
        return 0;
    }

    cnt = wiiList->dwNumberOfItems;
    if (cnt == 0) return 0;
    cnt = wiiList->InterfaceInfo[0].isState;
    WlanFreeMemory(wiiList);
    WlanCloseHandle(phClientHandle, NULL);

    return (cnt == wlan_interface_state_connected) ? 1 : 0;
}

BOOL NetState::IsWirelessNic()
{
    if (GetWirelessNicCount() > 0 && GetWirelessNicState() == 1) {
        /* Only exist Wireless NIC and Connected */
        return TRUE;
    }
    return FALSE;
}

int NetState::GetWirelessNic(DsgNic *pNic)
{
    DWORD pdwNegotiatedVersion;
    HANDLE phClientHandle;
    PWLAN_INTERFACE_INFO_LIST wiiList;
    IP_ADAPTER_INFO adp;
    DWORD dwRet = 0;
    TCHAR name[128] = {0};
    TCHAR desc[128] = {0};
    INT cnt = 0;
    BOOL found = FALSE;
    
    memset(pNic, 0, sizeof(DsgNic));
    memset(&adp, 0, sizeof(IP_ADAPTER_INFO));

    dwRet = WlanOpenHandle(1,NULL,&pdwNegotiatedVersion,&phClientHandle);
    if (dwRet != ERROR_SUCCESS) {
        LDebug(DEBUG_L_DEBUG, "GetWirelessNic failed=%d\n", GetLastError());
        return -1;
    }

    dwRet = WlanEnumInterfaces(phClientHandle,NULL,&wiiList);
    if (dwRet != ERROR_SUCCESS) {
        LDebug(DEBUG_L_DEBUG, "GetWirelessNic Enum failed=%d\n", GetLastError());
        return -1;
    }

    cnt = wiiList->dwNumberOfItems;
    lstrcpy(desc, wiiList->InterfaceInfo[0].strInterfaceDescription);
    WlanFreeMemory(wiiList);
    WlanCloseHandle(phClientHandle, NULL);

    found = GetNicFromDesc(desc, &adp);
    if (!found) {
        LDebug(DEBUG_L_DEBUG, "GetWirelessNic=> GetNicFrom desc='%A' failed\n", desc);
        return -1;
    }               

    CHAR mac[MAX_NAME_LEN]    = {0};
    CHAR guid[MAX_NAME_LEN*4] = {0};
    int  len = MAX_NAME_LEN;

    /* get IP */
    strcpy(pNic->sIP, adp.IpAddressList.IpAddress.String);

    /* get mac */
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", adp.Address[0], adp.Address[1], adp.Address[2], adp.Address[3],
           adp.Address[4], adp.Address[5]);
    strcpy(pNic->sMac, mac);
    memcpy((void*)pNic->iMac, (void*)adp.Address, 8);

    /* get Name */
    GetNicNameFromGuid(adp.AdapterName, pNic->sName);

    /* get guid */
    strcpy(pNic->sGuid, adp.AdapterName);
        
    /* wireless */
    pNic->iType = 1;

    /* get descrption */
    strcpy(pNic->sDesc, adp.Description);

    LDebug(DEBUG_L_DEBUG, "Wireless NIC: name='%s', desc='%s', ip='%s', mac='%s', type=%d",
           pNic->sName, pNic->sDesc, pNic->sIP, pNic->sMac, pNic->iType);
    
    return 0;
}

TCHAR RegNicLocation[] = _T("System\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\");
TCHAR RegNicDesc[]     = _T("\\Connection");

int NetState::GetNicNameFromGuid(CHAR *guid, CHAR *name)
{
    LONG  ret = 0;
    int len = 128;
    TCHAR KeyValue[1024] = {0};
    TCHAR Name[128] = {0};
    TCHAR Guid[128] = {0};
    TCHAR PnpInstanceID[128] = {0};
    DWORD MediaSubType = {0};

    mb2wc(guid, strlen(guid), Guid, &len);
    wsprintf(KeyValue, _T("%s%s%s"), RegNicLocation, Guid, RegNicDesc);

    RopRegistry Key;
    ret = Key.Open(HKEY_LOCAL_MACHINE,KeyValue);
    Key.Read(_T("PnpInstanceID"),PnpInstanceID);
    Key.Read(_T("Name"),Name);
    Key.Read(_T("MediaSubType"),MediaSubType);
    Key.Close();

    len = 128;
    wc2mb(Name, lstrlen(Name), name, &len);
    return 0;
}


int NetState::GetNormalNic(DsgNic *pNic)
{
    DWORD dwResult = 0;
    ULONG uSize    = 0;
    ULONG uOutNum  = 0;
    PIP_ADAPTER_INFO padp = NULL;
    IP_ADAPTER_INFO adp;
    TCHAR desc[128] = {0};
    BOOL found = FALSE;
    INT  Cnt = 0, len = 0;
    CHAR mac[MAX_NAME_LEN] = {0};

    /* Get All adapter */
    dwResult = GetAdaptersInfo(padp, &uSize);
    if (ERROR_BUFFER_OVERFLOW == dwResult) {
        padp = (PIP_ADAPTER_INFO)malloc(uSize);
        if (!padp) {
            LDebug(DEBUG_L_ERROR, "Malloc adapter list failed,byte=%d!\n", uSize);
            return -1;
        }
        dwResult = GetAdaptersInfo(padp, &uSize);
        if (ERROR_SUCCESS == dwResult) {
            PIP_ADAPTER_INFO padp_cur = padp;
            while (padp_cur && uOutNum < MAX_NDIS_ITEMS) {
                PNdisItem pNdis = NULL;
                char sDesc[256] = {0};

                switch(padp_cur->Type) {
                case MIB_IF_TYPE_ETHERNET:
                    if (IsSkipNdis(padp_cur->Description)) {
                        sprintf(sDesc, "Skip Adapter: Desc='%s', IP='%s'\n",
                                padp_cur->Description, 
                                padp_cur->IpAddressList.IpAddress.String);
                        LDebug(DEBUG_L_DEBUG, sDesc);
                    }else{
                        sprintf(sDesc, "Real Adapter: Desc='%s', IP='%s'\n",
                                padp_cur->Description, 
                                padp_cur->IpAddressList.IpAddress.String);
                        LDebug(DEBUG_L_DEBUG, sDesc);
                        memcpy(&adp, padp_cur, sizeof(IP_ADAPTER_INFO));
                        Cnt++;
                        found = TRUE;
                    }
                    break;
                default:
                    break;
                }
                padp_cur = padp_cur->Next;
            }
        }else{
            LDebug(DEBUG_L_ERROR, "GetAdapterInfo() failed=%d!\n", GetLastError());
            return -1;
        }
    }
    free(padp);
    
    if (!found) {
        LDebug(DEBUG_L_ERROR, "No normal NIC found!\n", GetLastError());
        return -1;
    }

    /* get IP */
    strcpy(pNic->sIP, adp.IpAddressList.IpAddress.String);

    /* get mac */
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", adp.Address[0], adp.Address[1], adp.Address[2], adp.Address[3],
        adp.Address[4], adp.Address[5]);
    strcpy(pNic->sMac, mac);
    memcpy((void*)pNic->iMac, (void*)adp.Address, 8);

    /* get Name */
    GetNicNameFromGuid(adp.AdapterName, pNic->sName);

    /* get guid */
    strcpy(pNic->sGuid, adp.AdapterName);

    /* normal */
    pNic->iType = 0;

    /* get descrption */
    strcpy(pNic->sDesc, adp.Description);

    LDebug(DEBUG_L_DEBUG, "Normal NIC: name='%s', desc='%s', ip='%s', mac='%s', type=%d",
        pNic->sName, pNic->sDesc, pNic->sIP, pNic->sMac, pNic->iType);

    return 0;
}

int NetState::GetExtelecomTapNic(DsgNic *pNic)
{
    DWORD dwResult = 0;
    ULONG uSize    = 0;
    ULONG uOutNum  = 0;
    PIP_ADAPTER_INFO padp = NULL;
    IP_ADAPTER_INFO adp;
    TCHAR desc[128] = {0};
    BOOL found = FALSE;
    INT  Cnt = 0, len = 0;
    CHAR mac[MAX_NAME_LEN] = {0};

    /* Get All adapter */
    dwResult = GetAdaptersInfo(padp, &uSize);
    if (ERROR_BUFFER_OVERFLOW == dwResult) {
        padp = (PIP_ADAPTER_INFO)malloc(uSize);
        if (!padp) {
            LDebug(DEBUG_L_ERROR, "Malloc adapter list failed,byte=%d!\n", uSize);
            return -1;
        }
        dwResult = GetAdaptersInfo(padp, &uSize);
        if (ERROR_SUCCESS == dwResult) {
            PIP_ADAPTER_INFO padp_cur = padp;
            while (padp_cur && uOutNum < MAX_NDIS_ITEMS) {
                PNdisItem pNdis = NULL;
                char sDesc[256] = {0};

                switch(padp_cur->Type) {
                case MIB_IF_TYPE_ETHERNET:
					if (!strncmp(padp_cur->Description, pNic->sDesc, strlen(pNic->sDesc))) {
                        sprintf(sDesc, "Tap Adapter: Desc='%s', IP='%s'\n",
                                padp_cur->Description, 
                                padp_cur->IpAddressList.IpAddress.String);
                        LDebug(DEBUG_L_DEBUG, sDesc);
                        memcpy(&adp, padp_cur, sizeof(IP_ADAPTER_INFO));
                        Cnt++;
                        found = TRUE;
                    }
                    break;
                default:
                    break;
                }
                padp_cur = padp_cur->Next;
            }
        }else{
            LDebug(DEBUG_L_ERROR, "GetAdapterInfo() failed=%d!\n", GetLastError());
            return -1;
        }
    }
    free(padp);
    
    if (!found) {
        LDebug(DEBUG_L_ERROR, "No normal NIC found!\n", GetLastError());
        return -1;
    }

    /* get IP */
    strcpy(pNic->sIP, adp.IpAddressList.IpAddress.String);

    /* get mac */
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", adp.Address[0], adp.Address[1], adp.Address[2], adp.Address[3],
        adp.Address[4], adp.Address[5]);
    strcpy(pNic->sMac, mac);
    memcpy((void*)pNic->iMac, (void*)adp.Address, 8);

	pNic->iIndex = adp.Index;
    /* get Name */
    GetNicNameFromGuid(adp.AdapterName, pNic->sName);

    /* get guid */
    strcpy(pNic->sGuid, adp.AdapterName);

    /* normal */
    pNic->iType = 0;

    /* get descrption */
    strcpy(pNic->sDesc, adp.Description);
    LDebug(DEBUG_L_DEBUG, "Normal NIC: name='%s', desc='%s', ip='%s', mac='%s', type=%d",
        pNic->sName, pNic->sDesc, pNic->sIP, pNic->sMac, pNic->iType);

    return 0;
}



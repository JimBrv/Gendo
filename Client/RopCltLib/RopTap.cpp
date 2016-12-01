/*
 * Gendo Tap NIC control VPN connection support
 *                             gendo,2013
 */
#include "stdafx.h"
#include "RopTap.h"

static TCHAR TAP32_INSTALL_CMD[]    = _T(".\\driver_x86\\tapinstall.exe install .\\driver_x86\\OemWin2k.inf tap0901");
static TCHAR TAP32_UNINSTALL_CMD[]  = _T(".\\driver_x86\\tapinstall.exe remove tap0901");
static TCHAR TAP32_STATUS[]         = _T(".\\driver_x86\\tapinstall.exe status tap0901");

static TCHAR TAP64_INSTALL_CMD[]    = _T(".\\driver_x64\\tapinstall.exe install .\\driver_x64\\OemWin2k.inf tap0901");
static TCHAR TAP64_UNINSTALL_CMD[]  = _T(".\\driver_x64\\tapinstall.exe remove tap0901");
static TCHAR TAP64_STATUS[]         = _T(".\\driver_x64\\tapinstall.exe status tap0901");

static TCHAR TAP_NAME_CHANGE_CMD[]  = _T("netsh interface set interface name=\"%s\" newname=\"%s\"");


INT RopTap::TapInstall()
{
    if(!TapStatus()) return 0;
    char result[2048] = {0};
    BOOL bRet = FALSE;
    if (IsOs64bit()) {
        bRet = RopCreateProcessR(TAP64_INSTALL_CMD,result);
    }else{
        bRet = RopCreateProcessR(TAP32_INSTALL_CMD,result);
    }
    Write2File("install.log", "%s", result);
    if (bRet) {
        LDebug(DEBUG_L_DEBUG, "Install TAP device failed!\n");
        return -1;
    }
    return TapStatus();
}


INT RopTap::TapUnstall()
{
    if(TapStatus()) return 0;
    char result[2048] = {0};
    BOOL bRet = FALSE;

    if (IsOs64bit()) {
        bRet = RopCreateProcessR(TAP64_UNINSTALL_CMD,result);
    }else{
        bRet = RopCreateProcessR(TAP32_UNINSTALL_CMD,result);
    }
    Write2File("install.log", "%s", result);
    if (bRet) {
        LDebug(DEBUG_L_DEBUG, "Remove TAP device failed!\n");
        return -1;
    }
    if(!TapStatus()) return -1;
    return 0;
}



INT RopTap::TapStatus()
{
    DsgNic nic;
    NetState net;
    memset(&nic, 0, sizeof(nic));
    if (!IsOs64bit()) {
        strcpy(nic.sDesc, TAPNAME_32);
    }else{
        strcpy(nic.sDesc, TAPNAME_64);
    }
    int ret = net.GetExtelecomTapNic(&nic);
    if(ret){
        LDebug(DEBUG_L_DEBUG, "no TAP device found!\n");
        return -1;
    }
    return 0;
}

INT RopTap::TapRestore()
{
    int ret = 0;
    ret = TapUnstall();
    if(ret){
        LDebug(DEBUG_L_DEBUG, "Restore TAP device faileded at remove progress!\n");
        return -1;
    }
    ret = TapInstall();
    if(ret){
        LDebug(DEBUG_L_DEBUG, "Restore TAP device faileded at install progress!\n");
        return -1;
    }
    return 0;
}

INT RopTap::TapSetIpDns(char *ip, char *mask, char *gw, char *dns1, char *dns2)
{
    DsgNic nic;
    NetState net;
    memset(&nic, 0, sizeof(nic));
    if (!IsOs64bit()) {
        strcpy(nic.sDesc, TAPNAME_32);
    }else{
        strcpy(nic.sDesc, TAPNAME_64);
    }
    int ret = net.GetExtelecomTapNic(&nic);
    if(ret){
        LDebug(DEBUG_L_DEBUG, "no TAP device found!\n");
        return -1;
    }
    BOOL bRet1, bRet2, bRet3;
    bRet1 = SetMetric(nic.sName,1);

    bRet2 = SetIp(nic.sName,  ip, mask, gw);
    bRet3 = SetDNS(nic.sName, dns1, dns2);
    if(!bRet2 || !bRet3){
        LDebug(DEBUG_L_DEBUG, "Set TAP ip dhcp!\n");
        bRet2 = SetIpDHCP(nic.sName);
        bRet3 = SetDNSDHCP(nic.sName);
        if(!bRet2 || !bRet3) return -1;
    }

    return 0;
}

INT  RopTap::TapSetName(CHAR *name)
{
    int   ret = 0;
    TCHAR cmd[128] = {0};
    TCHAR nic_old[64] = {0};
    TCHAR nic_new[64] = {0};
    DsgNic   nic;
    NetState net;
    memset(&nic, 0, sizeof(nic));
    if (!IsOs64bit()) {
        strcpy(nic.sDesc, TAPNAME_32);
    }else{
        strcpy(nic.sDesc, TAPNAME_64);
    }
    ret = net.GetExtelecomTapNic(&nic);

    if (ret) {
        LDebug(DEBUG_L_CRITICAL, "No TAP nic found!");
        return 1;
    }
    ret = 64;
    mb2wc(name, strlen(name), nic_new, &ret);
    ret = 64;
    mb2wc(nic.sName, strlen(nic.sName), nic_old, &ret);

    wsprintf(cmd, TAP_NAME_CHANGE_CMD, nic_old, nic_new);

    if(RopCreateProcess(cmd)) {
        return 0;
    }else{
        return 2;
    }
}

BOOL  RopTap::IsTapNameOK(CHAR *name)
{
    int   ret = 0;
    TCHAR cmd[128] = {0};
    DsgNic   nic;
    NetState net;
    memset(&nic, 0, sizeof(nic));
    if (!IsOs64bit()) {
        strcpy(nic.sDesc, TAPNAME_32);
    }else{
        strcpy(nic.sDesc, TAPNAME_64);
    }
    ret = net.GetExtelecomTapNic(&nic);

    if (ret) {
        LDebug(DEBUG_L_CRITICAL, "No TAP nic found!");
        return FALSE;
    }
    
    if (!strcmp(nic.sName, name)) {
        LDebug(DEBUG_L_CRITICAL, "TAP nic name match!");
        return TRUE;
    }
    LDebug(DEBUG_L_CRITICAL, "TAP nic name not-match!");
    return FALSE;
}

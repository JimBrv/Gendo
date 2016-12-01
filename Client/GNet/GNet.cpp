#include "StdAfx.h"
#include <Windows.h>
#include <string.h>
#include <Iphlpapi.h>
#include "GNet.h"

char* GetFileStr(TCHAR* FilePath);
BOOL  RegIP( char*  lpszAdapterName, char* pIPAddress, char* pNetMask, char* pNetGate,  char* pDNSServer1,  char* pDNSServer2 ) ;
BOOL  RegName( char* lpszAdapterName, char* Name);
BOOL  SetIp(LPCSTR lpstrAdapter,LPCSTR lpstrDestAddr, LPCSTR lpstrMask, LPCSTR lpstrGateway);
BOOL  SetIpDHCP(LPCSTR lpstrAdapter);
BOOL  SetDNS(LPCSTR lpstrAdapter,LPCSTR lpstrDNS1,LPCSTR lpstrDNS2);
BOOL  SetDNSDHCP(LPCSTR lpstrAdapter);
BOOL  SetMetric(LPCSTR lpstrAdapter,DWORD dwMetric);


static TCHAR TAP32_INSTALL_CMD[]    = _T(".\\driver_x86\\tapinstall.exe install .\\OemWin2k.inf tap0901");
static TCHAR TAP32_UNINSTALL_CMD[]  = _T(".\\driver_x86\\tapinstall.exe remove tap0901");
static TCHAR TAP32_STATUS[]         = _T(".\\driver_x86\\tapinstall.exe status tap0901");

static TCHAR TAP64_INSTALL_CMD[]    = _T(".\\driver_x64\\tapinstall.exe install .\\OemWin2k.inf tap0901");
static TCHAR TAP64_UNINSTALL_CMD[]  = _T(".\\driver_x64\\tapinstall.exe remove tap0901");
static TCHAR TAP64_STATUS[]         = _T(".\\driver_x64\\tapinstall.exe status tap0901");


GNET_API INT TapInstall()
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


GNET_API INT TapUnstall()
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



GNET_API INT TapStatus()
{
    DsgNic nic;
    NetState net;
    memset(&nic, 0, sizeof(nic));
    strcpy(nic.sDesc, TAPNAME);
    int ret = net.GetExtelecomTapNic(&nic);
    if(ret){
        LDebug(DEBUG_L_DEBUG, "no TAP device found!\n");
        return -1;
    }
    return 0;
}

GNET_API INT TapRestore()
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


char* GetFileStr(TCHAR* FilePath)
{
    FILE* pFile; 
    pFile = fopen ("tap.log", "rb" );  
    if (!pFile){  
        LDebug(DEBUG_L_DEBUG, "check TAP device failed!\n");
        return NULL;
    }
    int size = 0;
    fseek (pFile , 0 , SEEK_END);  
    size = ftell (pFile);  
    rewind (pFile); 
    char* buffer = (char*) malloc (sizeof(char) * size);  
    if (!buffer){  
        LDebug(DEBUG_L_DEBUG, "malloc failed!\n");
        fclose(pFile);
        return NULL;
    }
    if((fread (buffer,1,size,pFile))!= size){
        LDebug(DEBUG_L_DEBUG, "read log file error!\n");
        fclose(pFile);
        free(buffer);
        return NULL;
    }
    fclose (pFile);
    return buffer;
}

GNET_API int TapSetIpDns(char *ip, char *mask, char *gw, char *dns1, char *dns2)
{
    DsgNic nic;
    NetState net;
    memset(&nic, 0, sizeof(nic));
    strcpy(nic.sDesc, TAPNAME);
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

BOOL RegIP( char* lpszAdapterName, char* pIPAddress, char* pNetMask, char* pNetGate,  char* pDNSServer1,  char* pDNSServer2 ) 
{ 
    HKEY hKey; 
    char strKeyName[512] = {0};
    sprintf(strKeyName,"SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\%s", lpszAdapterName); 
    if( ::RegOpenKeyExA( HKEY_LOCAL_MACHINE, strKeyName, 0, KEY_WRITE, &hKey ) != ERROR_SUCCESS ) 
        return FALSE; 

    char DNSServer[64] = {0};
    sprintf(DNSServer, "%s,%s", pDNSServer1, pDNSServer2); 
    int n = 0;
    ::RegSetValueExA( hKey, "IPAddress", 0, REG_MULTI_SZ, (unsigned char*)pIPAddress, strlen(pIPAddress)+2 ); 
    ::RegSetValueExA( hKey, "SubnetMask", 0, REG_MULTI_SZ, (unsigned char*)pNetMask, strlen(pNetMask)+2 ); 
    ::RegSetValueExA( hKey, "DefaultGateway", 0, REG_MULTI_SZ, (unsigned char*)pNetGate, strlen(pNetGate)+2 ); 
    ::RegSetValueExA( hKey, "NameServer", 0, REG_SZ, (unsigned char*)DNSServer, strlen(DNSServer)+2 ); 
    ::RegSetValueExA( hKey, "EnableDHCP", 0, REG_DWORD, (PBYTE)&n, sizeof(DWORD)); 
    //char tmp[10] = {0};
    //::RegSetValueExA( hKey, "DefaultGatewayMetric", 0, REG_MULTI_SZ, (unsigned char*)tmp, strlen(tmp)+2); 
    //n = 1;
    //::RegSetValueExA( hKey, "InterfaceMetric", 0, REG_DWORD, (const BYTE *)&n, 4); 
    ::RegCloseKey(hKey); 

    return TRUE; 
} 


BOOL RegName( char* lpszAdapterName, char* Name) 
{ 
    HKEY hKey; 
    char strKeyName[512] = {0};
    sprintf(strKeyName,"SYSTEM\\ControlSet001\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\%s\\Connection", lpszAdapterName); 
    if( ::RegOpenKeyExA( HKEY_LOCAL_MACHINE, strKeyName, 0, KEY_WRITE, &hKey ) != ERROR_SUCCESS ) 
        return FALSE; 

    int n = 0;
    ::RegSetValueExA( hKey, "Name", 0, REG_SZ, (unsigned char*)Name, strlen(Name)+2); 

    ::RegCloseKey(hKey); 

    return TRUE; 
}

BOOL SetIpDHCP(LPCSTR lpstrAdapter)
{
    if(!lpstrAdapter){
        LDebug(DEBUG_L_DEBUG, "param error!\n");
        return FALSE;
    }
    BOOL bRet;
    TCHAR wCmd[256]={0};
    TCHAR wAdapter[128] = {0};
    INT   len =128;
    mb2wc(lpstrAdapter, strlen(lpstrAdapter), wAdapter, &len);

    wsprintf(wCmd,L"netsh interface ip set address \"%s\" dhcp",wAdapter);
    bRet =RopCreateProcess(wCmd);
    if (bRet) {
        LDebug(DEBUG_L_DEBUG, "Set TAP ip addr dhcp failed!\n");
        return FALSE;
    }
    return TRUE;
}

BOOL SetIp(LPCSTR lpstrAdapter,LPCSTR lpstrDestAddr, LPCSTR lpstrMask, LPCSTR lpstrGateway)
{
    if((!lpstrDestAddr)||(!lpstrMask)||(!lpstrGateway)){
        LDebug(DEBUG_L_DEBUG, "param error!\n");
        return FALSE;
    }

    BOOL bRet;
    TCHAR wCmd[256]={0};
    TCHAR wAdapter[128] = {0};
    TCHAR wDestAddr[128] = {0};
    TCHAR wMask[128] = {0};
    TCHAR wGateway[128] = {0};
    INT   len1 =128;
    INT   len2 =128;
    INT   len3 =128;
    INT   len4 =128;
    mb2wc(lpstrAdapter, strlen(lpstrAdapter), wAdapter, &len1);
    mb2wc(lpstrDestAddr, strlen(lpstrDestAddr), wDestAddr, &len2);
    mb2wc(lpstrMask, strlen(lpstrMask), wMask, &len3);
    mb2wc(lpstrGateway, strlen(lpstrGateway), wGateway, &len4);

    wsprintf(wCmd,L"netsh interface ip set address \"%s\" static %s %s %s",wAdapter, wDestAddr, wMask, wGateway);
    bRet =RopCreateProcess(wCmd);
    if (bRet) {
        LDebug(DEBUG_L_DEBUG, "Set TAP ip addr static failed!\n");
        return FALSE;
    }

    return TRUE;
}
BOOL SetDNSDHCP(LPCSTR lpstrAdapter)
{
    if(!lpstrAdapter){
        LDebug(DEBUG_L_DEBUG, "param error!\n");
        return FALSE;
    }
    BOOL bRet;
    TCHAR wCmd[256]={0};
    TCHAR wAdapter[128] = {0};
    INT   len =128;
    mb2wc(lpstrAdapter, strlen(lpstrAdapter), wAdapter, &len);

    wsprintf(wCmd,L"netsh interface ip set dns \"%s\" dhcp",wAdapter);
    bRet =RopCreateProcess(wCmd);
    if (bRet) {
        LDebug(DEBUG_L_DEBUG, "Set TAP ip dns dhcp failed!\n");
        return FALSE;
    }
    return TRUE;
}
BOOL SetDNS(LPCSTR lpstrAdapter,LPCSTR lpstrDNS1,LPCSTR lpstrDNS2)
{
    if((!lpstrAdapter)||(!lpstrDNS1)||(!lpstrDNS2)){
        LDebug(DEBUG_L_DEBUG, "param error!\n");
        return FALSE;
    }

    BOOL bRet;
    TCHAR wCmd[256]={0};
    TCHAR wAdapter[128] = {0};
    TCHAR wDNS1[128] = {0};
    TCHAR wDNS2[128] = {0};
    INT   len1 =128;
    INT   len2 =128;
    INT   len3 =128;
    mb2wc(lpstrAdapter, strlen(lpstrAdapter), wAdapter, &len1);
    mb2wc(lpstrDNS1, strlen(lpstrDNS1), wDNS1, &len2);
    mb2wc(lpstrDNS2, strlen(lpstrDNS2), wDNS2, &len3);

    wsprintf(wCmd,L"netsh interface ip set dns \"%s\" static %s primary yes",wAdapter, wDNS1);
    bRet =RopCreateProcess(wCmd);
    if (bRet) {
        LDebug(DEBUG_L_DEBUG, "Set TAP main dns static failed!\n");
        return FALSE;
    }

    wsprintf(wCmd,L"netsh interface ip add dns \"%s\" %s",wAdapter, wDNS2);
    bRet =RopCreateProcess(wCmd);
    if (bRet) {
        LDebug(DEBUG_L_DEBUG, "Set TAP sub dns static failed!\n");
        //return FALSE;
    }

    return TRUE;
}


BOOL SetMetric(LPCSTR lpstrAdapter,DWORD dwMetric)
{
    if((!lpstrAdapter) ||dwMetric <= 0){
        LDebug(DEBUG_L_DEBUG, "param error!\n");
        return FALSE;
    }
    BOOL bRet;
    RopBase OS;
    DWORD dwVersion = OS.OSVersion();
    if(dwVersion > 5){
        TCHAR wCmd[256]={0};
        TCHAR wAdapter[128] = {0};
        INT   len =128;
        mb2wc(lpstrAdapter, strlen(lpstrAdapter), wAdapter, &len);

        wsprintf(wCmd,L"netsh interface ip set interface \"%s\" metric=%d",wAdapter, dwMetric);
        bRet =RopCreateProcess(wCmd);
        if (bRet) {
            LDebug(DEBUG_L_DEBUG, "Set TAP metric failed [vista+]!\n");
            return FALSE;
        }
    }
    else{
        bRet =RopCreateProcess(L"wscript.exe SetMetricOne.vbs");
        if (bRet) {
            LDebug(DEBUG_L_DEBUG, "Set TAP metric faileded [xp]!\n");
            return FALSE;
        }
    }
    return TRUE;
}

GNET_API BOOL DHCPRenew()
{
    DsgNic nic;
    NetState net;
    memset(&nic, 0, sizeof(nic));
    strcpy(nic.sDesc, TAPNAME);
    int ret = net.GetExtelecomTapNic(&nic);
    if(ret){
        LDebug(DEBUG_L_DEBUG, "no TAP device found!\n");
        return -1;
    }

    TCHAR wCmd[256]={0};
    TCHAR wAdapter[128] = {0};
    INT   len =128;
    mb2wc(nic.sName, strlen(nic.sName), wAdapter, &len);

    wsprintf(wCmd,L"ipconfig /renew \"%s\"",wAdapter);

    BOOL bRet =RopCreateProcess(wCmd);
    if (bRet) {
        LDebug(DEBUG_L_DEBUG, "renew TAP dhcp faileded!\n");
        return FALSE;
    }
    return TRUE;
}

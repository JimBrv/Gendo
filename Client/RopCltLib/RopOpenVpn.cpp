#include "StdAfx.h"
#include <fstream>
#include <vector>
#include "RopUtil.h"
#include "RopProcess.h"
#include "RopOpenVpn.h"

using namespace std;


#define OPENVPN_CONF_CONTENT \
    "client\n"\
    "dev tun\n"\
    "resolv-retry infinite\n"\
    "nobind\n"\
    "persist-key\n"\
    "persist-tun\n"\
    "ca ca.crt\n"\
    "auth-user-pass\n"\
    "ns-cert-type server\n"\
    ";tls-auth ta.key 1\n"\
    ";cipher x\n"\
    "comp-lzo\n"\
    "verb 3\n"\
    "route-delay\n"\
    "obfs-salt Hello\n"\
    "obfs-padlen 16\n"\
    "mssfix 1386\n"\
    ";mute 20\n"



VOID RopOpenVpn::SetParams(TCHAR *sUser, TCHAR *sPwd, TCHAR *sIP, INT iProto, INT iPort, LPCTSTR sSvrName)
{
    lstrcpy(m_sUser, sUser);
    lstrcpy(m_sPwd,  sPwd);
    lstrcpy(m_sIP,   sIP);
    lstrcpy(m_sSvrName, sSvrName);
    m_iPort  = iPort;
    m_iProto = iProto;

    if (0 == m_iProto) {
        // PROTO_OPENVPN_UDP
        lstrcpy(m_sProto, _T("SSL-UDP"));        
    }else{
        lstrcpy(m_sProto, _T("SSL-TCP"));
    }

    INT ret = CreateConfFile();
    if (ret) {
        LDebug(DEBUG_L_ERROR, "cannot create gendo.conf\n");
    }

    ret = CreatePwdFile();
    if (ret) {
        LDebug(DEBUG_L_ERROR, "cannot create pwd file\n");
    }
}


VOID RopOpenVpn::SetAuthTyp(INT iType, INT iSslAlg)
{
    m_iAuthType = iType;
    m_iSslAlgs  = iSslAlg;

}

INT  RopOpenVpn::CreatePwdFile(VOID)
{
    CHAR user[32] = {0};
    CHAR pwd[32] = {0};
    INT  len = 0;
    ofstream conf;

    conf.open(OPENVPN_CONFIG_USERPWD, ios_base::trunc);
    if (!conf.is_open()) {
        LDebug(DEBUG_L_ERROR, "cannot open gendo.conf\n");
        return -1;
    }
    len = 32;
    wc2mb(m_sUser, lstrlen(m_sUser), user, &len);
    len = 32;
    wc2mb(m_sPwd, lstrlen(m_sPwd), pwd, &len);
    
    conf << user << endl;
    conf << pwd  << endl;
    conf.close();
    return 0;
}


VOID  RopOpenVpn::Cleanup(VOID)
{
    ofstream conf;

    conf.open(OPENVPN_CONFIG_USERPWD, ios_base::trunc);
    if (!conf.is_open()) {
        LDebug(DEBUG_L_ERROR, "cannot open pwd\n");
    }
    conf.close();

    conf.open(OPENVPN_CONFIG_FILE, ios_base::trunc);
    if (!conf.is_open()) {
        LDebug(DEBUG_L_ERROR, "cannot open conf\n");
        return;
    }
    conf.close();
    return;
}

INT  RopOpenVpn::CreateConfFile(VOID)
{
    CHAR dev[64]   = {0};
    CHAR proto[10] = {0};
    CHAR remote[256]   = {0};
    CHAR ip[128] = {0};
    INT  len = 0;
    ofstream conf;

    sprintf(dev, "dev-node %s", OPENVPN_TAP_NAME);
    
    if (m_iProto == 0) {
        sprintf(proto, "proto %s", "udp");
    }else{
        sprintf(proto, "proto %s", "tcp");
    }
    
    len = 256;
    wc2mb(m_sIP, lstrlen(m_sIP), ip, &len);
    sprintf(remote, "remote %s %d", ip, m_iPort);

    conf.open(OPENVPN_CONFIG_FILE, ios_base::trunc);
    if (!conf.is_open()) {
        LDebug(DEBUG_L_ERROR, "cannot open gendo.conf\n");
        return -1;
    }

    conf << OPENVPN_CONF_CONTENT << endl;
    conf << dev   << endl;
    conf << proto << endl;
    conf << remote<< endl;

    conf.close();
    return 0;
}


INT  RopOpenVpn::Connect()
{
    BOOL bRet = RopCreateProcessA(OPENVPN_CMD_AUTH_USERPWD, FALSE);
    Sleep(1000);
    return bRet == TRUE ? 0 : -1;
}
INT  RopOpenVpn::DisConnect()
{
    BOOL bRet = RestoreRoute();
    bRet = RopStopProcessA(OPENVPN_PROCESS_NAME);
    return 0;
}

INT  RopOpenVpn::ReConnect()
{
    DisConnect();
    return Connect();
}


#define OPENVPN_STATUS_MSG_CONNECT "Connecting OpenVPN server... (Gendo SSL VPN Version 1.0.0)!"

/*
 * Parse openvpn log file to get current state
 * Maybe I need use openvpnUI notification, but this also is a simple 
 * way to determine Openvpn.exe process status:)
 *
 */
INT RopOpenVpn::GetStatus(TCHAR *msg, CHAR *logfile)
{
    int ret = 1;
    ifstream  log;
    vector<string> content;
    vector<string>::reverse_iterator iter;
    char   buf[512] = {0};
    string line;

    log.open(logfile);
    if (!log.is_open()) {
        LDebug(DEBUG_L_ERROR, "cannot open openvpn log='%s'\n", logfile);
        return -1;
    }
    while(!log.eof()) {
        log.getline(buf, 512);
        line = buf;
        content.push_back(line);
    }
    log.close();

    bool FixedWin8 = FALSE;

    /* Revert parse log file */
    for (iter = content.rbegin(); iter != content.rend(); iter++) {
        line = *iter;
        if (line.find(OPENVP_STATUS_INIT) != string::npos) {
            lstrcpy(msg, _T("OpenVpn 正在初始化连接..."));
            ret = 1;
            break;
        }
        if (line.find(OPENVP_STATUS_HANDSHAKE) != string::npos) {
            lstrcpy(msg, _T("OpenVpn 正在执行SSL握手协议..."));
            ret = 2;
            break;
        }
        if (line.find(OPENVP_STATUS_AUTH_OK) != string::npos) {
            lstrcpy(msg, _T("OpenVpn 正在认证用户信息..."));
            ret = 4;
            break;
        }
        if (line.find(OPENVP_STATUS_HANDSHAKE_OK) != string::npos) {
            lstrcpy(msg, _T("OpenVpn SSL握手协议协商成功..."));
            ret = 5;
            break;
        }
        if (line.find(OPENVP_STATUS_DHCP) != string::npos) {
            lstrcpy(msg, _T("OpenVpn 正在为本地计算机分配接入地址..."));
            ret = 6;
            break;
        }

        if (line.find(OPENVP_STATUS_PERMISSION_DENY) != string::npos) {
            lstrcpy(msg, _T("OpenVpn 权限不足，请以管理员权限启动客户端！（右键单击->管理员方式启动）"));
            ret = -1;
            break;
        }

        if (line.find(OPENVP_STATUS_NEED_TOUCH) != string::npos) {
            lstrcpy(msg, _T("OpenVpn 正在修复Windows兼容性问题..."));
            //m_iFixWin8Timer++;
           // if (m_iFixWin8Timer >= 5) {
            //    m_iFixWin8Timer = 0;
            //    FixWin8Up(OPENVPN_TAP_NAME);
            //}
            FixWin8TapEnable(OPENVPN_TAP_NAME); 
            m_bFixedWin8 = FALSE;
            ret = 6;
            break;
        }


        if (line.find(OPENVP_STATUS_ALL_DONE) != string::npos) {
            lstrcpy(msg, _T("OpenVpn 接入就绪..."));
            ret = 0;
            break;
        }
    }        

    return ret;
}

INT RopOpenVpn::FixWin8Up(char *TapName)
{
    char cmd[128] = {0};
    sprintf(cmd, OPENVPN_CMD_FIX_DOWN, TapName);
    BOOL bRet = RopCreateProcessA(cmd);
    Sleep(2000);
    sprintf(cmd, OPENVPN_CMD_FIX_UP, TapName);
    bRet = RopCreateProcessA(cmd);
    return bRet == TRUE ? 0 : -1;
}

INT RopOpenVpn::FixWin8TapEnable(char *TapName)
{
    char cmd[128] = {0};
    BOOL bRet = FALSE;
    Sleep(2000);
    sprintf(cmd, OPENVPN_CMD_FIX_UP, TapName);
    bRet = RopCreateProcessA(cmd);
    return bRet == TRUE ? 0 : -1;
}

VOID RopOpenVpn::DelDefaultRoute()
{
    TCHAR command[1024] = {0};
    wsprintf(command, _T("route delete %s MASK %s %s"), _T("0.0.0.0"), _T("128.0.0.0"), _T("-p"));

    LDebug(DEBUG_L_INFO, "Restore route1: %S\n", command);

    if (RopCreateProcess(command)!=0) {
        LDebug(DEBUG_L_ERROR,"FAILED::%s",command);
    }

    wsprintf(command, _T("route delete %s MASK %s %s"), _T("128.0.0.0"), _T("128.0.0.0"), _T("-p"));
    LDebug(DEBUG_L_INFO, "Restore route2: %S\n", command);

    if (RopCreateProcess(command)!=0) {
        LDebug(DEBUG_L_ERROR,"FAILED::%s",command);
    }
}

VOID RopOpenVpn::DelNicRoute()
{    
    PMIB_IPFORWARDTABLE pIpForwardTable = NULL;
    DWORD dwSize = 0;
    BOOL  bOrder = FALSE;
    DWORD dwStatus = 0;
    CHAR  cNet[32] = {0};
    CHAR  cNetMask[32] = {0};
    CHAR  cGateWay[32] = {0};
    CHAR  command[512] = {0};

    dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, bOrder);
    if (dwStatus == ERROR_INSUFFICIENT_BUFFER) {
        if (!(pIpForwardTable = (PMIB_IPFORWARDTABLE)malloc(dwSize))) {
            return;
        }
        dwStatus = GetIpForwardTable(pIpForwardTable, &dwSize, bOrder);
    }

    if (dwStatus != ERROR_SUCCESS) {
        if (pIpForwardTable) free(pIpForwardTable);
        return;
    }

    for (unsigned int j=0; j < pIpForwardTable->dwNumEntries; j++) {
        if (IsGendoOvpnNet(pIpForwardTable->table[j].dwForwardDest)) {
            /* Delete Nic ip range route */
            in_addr sin;
            sin.S_un.S_addr = pIpForwardTable->table[j].dwForwardDest;
            strcpy(cNet,     inet_ntoa(sin));
            sin.S_un.S_addr = pIpForwardTable->table[j].dwForwardMask;
            strcpy(cNetMask, inet_ntoa(sin));
            sin.S_un.S_addr = pIpForwardTable->table[j].dwForwardNextHop;
            strcpy(cGateWay, inet_ntoa(sin));
            sprintf(command, "route delete %s MASK %s %s", cNet, cNetMask, cGateWay);
            LDebug(DEBUG_L_INFO, "Restore NicRoute: %s\n", command);

            if (RopCreateProcessA(command)!=0) {
                LDebug(DEBUG_L_ERROR,"FAILED::%s",command);
            }

        }
    }

    if (pIpForwardTable) free(pIpForwardTable);
    return;
}

/* Delete HOST route of ovpn */
VOID RopOpenVpn::DelHostIPRoute()
{
    TCHAR command[1024] = {0};
    wsprintf(command, _T("route delete %s %s"), m_sIP, _T("-p"));

    LDebug(DEBUG_L_INFO, "Restore route3: delete server host ip route\n");

    if (RopCreateProcess(command)!=0) {
        LDebug(DEBUG_L_ERROR,"FAILED::%s",command);
    }
}

/*
 * Openvpn.exe cannot easy stopped with safely auto-remove its route.
 * Just remove these route By gendo self.
 * Need remove routes: 
 *   0.0.0.0/128.0.0.0         172.18.x.x -- defualt route
 *  172.18.x.0/255.255.255.0   172.18.x.x -- NIC route
 *  172.18.x.x/255.255.255.255 172.18.x.x -- NIC route
 *  172.18.x.x/255.255.255.252 172.18.x.x -- NIC route
 *  128.0.0.0/128.0.0.0 172.18.x.x -- dont know why it comes out:)
 */

INT RopOpenVpn::RestoreRoute()
{
    DelDefaultRoute();
    DelNicRoute();
    DelHostIPRoute();
    return 0;
}
/*
 * OpenVPN SSL support 
 * All right reserved by Gendo, 2013
 * Gendo.Jim 
 */

#ifndef ROP_OPENVPN_H
#define ROP_OPENVPN_H
#include "StdAfx.h"
#include <windows.h>
#include "RopBase.h"
#include <IPHlpApi.h>


#define OPENVPN_TAP_NAME          "gendovpn"
#define OPENVPN_CONFIG_TEMPLATE   "gendo-tmpl.conf"
#define OPENVPN_CONFIG_FILE       "gendo.conf"
#define OPENVPN_CONFIG_USERPWD    "gendo-pwd"
#define OPENVPN_LOG_FILE          "gendo.log"
#define MAX_PORT_NUM               12
/************************************************************************/
/* OpenVPN commcand format, make sure delete pwd file:)                 */
/* openvpn.exe --config client.ovpn --auth-user-pass pwd.txt            */
/************************************************************************/
#define OPENVPN_PROCESS_NAME      "openvpn.exe"
#define OPENVPN_CMD_AUTH_USERPWD  "openvpn.exe --config gendo.conf --auth-user-pass gendo-pwd  --log gendo.log --dhcp-pre-release --ip-win32 netsh"
#define OPENVPN_CMD_AUTH_CERT     "openvpn.exe --config gendo.conf --cert %s"
#define OPENVPN_CMD_FIX_UP        "netsh interface set interface \"%s\" ENABLED"
#define OPENVPN_CMD_FIX_DOWN      "netsh interface set interface \"%s\" DISABLED"


#define OPENVP_STATUS_INIT            "Data Channel MTU parms"
#define OPENVP_STATUS_HANDSHAKE       "TLS: Initial"
#define OPENVP_STATUS_AUTH_OK         "VERIFY OK: nsCertType=SERVER"
#define OPENVP_STATUS_HANDSHAKE_OK    "[server] Peer Connection Initiated with "
#define OPENVP_STATUS_DHCP            "TAP-Win32"

/* Need we touch it in Win8, shit win8 */
#define OPENVP_STATUS_NEED_TOUCH      "Waiting for TUN/TAP interface to come up"

#define OPENVP_STATUS_PERMISSION_DENY "FlushIpNetTable failed on interface"

#define OPENVP_STATUS_ALL_DONE        "Initialization Sequence Completed"

/* Gendo alloc IP/mask range */
#define GENDO_OVPN_IP_NET  "172.18.0.0"
#define GENDO_OVPN_IP_MASK "255.255.0.0"

#define IsGendoOvpnNet(IPADDR) (((IPADDR)&inet_addr(GENDO_OVPN_IP_MASK))==\
    (inet_addr(GENDO_OVPN_IP_NET)&inet_addr(GENDO_OVPN_IP_MASK)))


class RopOpenVpn 
{
public:
    RopOpenVpn() 
    {
        m_sUser[0] = 0; 
        m_sPwd[0] = 0; 
        m_bUdpFirst = TRUE; 
        m_iPort = m_iProto = m_iPortSize = m_iAuthType = m_iSslAlgs = 0;
        m_bFixedWin8 = FALSE;
    };
    ~RopOpenVpn(){};

    VOID SetParams(TCHAR *sUser, TCHAR *sPwd, TCHAR *sIP, INT iProto, INT iPort, LPCTSTR sSvrName);
    VOID SetAuthTyp(INT iType, INT iSslAlg);
    INT  CreateConfFile(VOID);
    INT  CreatePwdFile(VOID);
    INT  Connect();
    INT  DisConnect();
    INT  ReConnect();

    /* Must restore route before calling DisConnect */
    INT  RestoreRoute();
    VOID DelDefaultRoute();
    VOID DelNicRoute();
    VOID DelHostIPRoute();
    
    VOID  Cleanup(VOID);

    /* Get current status, return 0 means OK, else put status to msg */
    INT GetStatus(TCHAR *msg, CHAR *logfile);

    INT FixWin8Up(char *TapName);
    INT FixWin8TapEnable(char *TapName);

public:

public:
    TCHAR m_sUser[MAX_NAME_LEN];
    TCHAR m_sPwd[MAX_NAME_LEN];
    TCHAR m_sIP[MAX_NAME_LEN];
    TCHAR m_sSvrName[MAX_NAME_LEN];
    TCHAR m_sProto[MAX_NAME_LEN];

    BOOL  m_bUdpFirst;               // indicate udp -> tcp 
    INT   m_iPortSize;               // how many port size, udp/tcp should be equal
    INT   m_iPort;                   // udp,tcp port array
    INT   m_iProto;  
    INT   m_iSslAlgs;                // 0 - defualt enc/decrypt algs
    INT   m_iAuthType;               // 0 - user/pwd, 1 - user cert

    BOOL  m_bFixedWin8;            // Win8 fix counter delay 5 timer

};

#endif
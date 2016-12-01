#ifndef ROP_NET_STATE_H
#define ROP_NET_STATE_H
#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include "RopBase.h"
#include "lib-msg.h"
#include <iphlpapi.h>

typedef enum {
    NET_STATE_DISCONNECTED = 0,
    NET_STATE_CONNECTED_NETCARD,
    NET_STATE_CONNECTED_WLAN,
    NET_STATE_CONNECTED_PPPOE,
}NetStateE;


typedef enum {
    NDIS_STATE_INACTIVE = 0,
    NDIS_STATE_ACTIVE,
}NdisStateE;

typedef enum {
    NDIS_TYPE_NETCARD = 1,      // ethernet
    NDIS_TYPE_WLAN,             // WLAN ethernet
    NDIS_TYPE_OTHER_PPPOE,      // Other PPPoE
    NDIS_TYPE_ROP_PPPOE,        // My PPPoE
    NDIS_TYPE_VMWARE,
    NDIS_TYPE_VPN,
}NdisTypeE;


#define MAX_NDIS_ITEMS 64
#define MAX_NDIS_NAME  128

typedef struct _Ndis_Item_s_ {
    int    iType;      // Ndis typeE
    int    iState;     // Ndis StateE
    char   sName[MAX_NDIS_NAME];
    char   sIP[MAX_NAME_LEN];  
    unsigned int iRx;        // recv bytes
    unsigned int iTx;        // transmit bytes
}NdisItem, *PNdisItem;

typedef struct _Ndis_Ary_s_ {
    int     iNum;
    NdisItem Items[MAX_NDIS_ITEMS];
}NdisAry, *PNdisAry;


typedef struct _DsgNic_ {
    CHAR  sName[MAX_NAME_LEN*4];
    CHAR  sDesc[MAX_NAME_LEN*4];
    CHAR  sIP[MAX_NAME_LEN];
    CHAR  sGateWay[MAX_NAME_LEN];
    CHAR  sMac[MAX_NAME_LEN];
    BYTE  iMac[8];
    CHAR  sGuid[MAX_NAME_LEN*4];
    INT   iType;      // 0: 有线， 1: 无线
    DWORD iIndex;
}DsgNic, *PDsgNic;


class NetState : public RopBase
{
public:
    NetState();
    ~NetState();

public:
    int  GetAllNdis(void);
    int  GetEthNic();
    void DumpAllNdis(void);
    void DumpEthNdis();
    BOOL GetNicFromDesc(TCHAR *desc, IP_ADAPTER_INFO *pAdp);

    int  GetWirelessNicCount();
    int  GetWirelessNicState();

    BOOL IsWirelessNic();

    int  GetWirelessNic(DsgNic *pNic);
    int  GetNormalNic(DsgNic *pNic);
    int  GetExtelecomTapNic(DsgNic *pNic);

    int GetNicNameFromGuid(CHAR *guid, CHAR *name);

protected:
    BOOL    m_bInit;
    NdisAry m_NdisAry; 
};

#endif
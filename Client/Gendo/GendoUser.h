#pragma once
#include "StdAfx.h"
#include "RopBase.h"

class CGendoUser: public RopBase
{
public:
    CGendoUser() {
        m_bLogin = FALSE;
    };
    ~CGendoUser() {};

    CHAR *GetOS(VOID) {return OSName();}

public:
    BOOL  m_bLogin;
    TCHAR m_sUser[32];
    TCHAR m_sNick[32];
    TCHAR m_sPwd[32];
    INT   m_iType;          // user type : 0-Free, 1-VIP
    INT   m_iState;         // user state: 0-Inactive, 1-Active, 
    INT   m_iLevel;         // user level: 0-Free, 1-N: VIP 
    INT   m_iHobby;         // user hobby
    INT   m_iServiceType;   // user service type, according traffic bytes.
    
    TCHAR m_sQuotaName[32];   // Quota service information
    INT   m_iQuotaCycle;      
    Int64 m_iQuotaTraffic;
    Int64 m_iQuotaUsedTraffic;
    TCHAR m_sQuotaExpire[32];   // next expire date
    Int64 m_iHistoryTraffic;   
    TCHAR m_sRegister[32];      // register time
    TCHAR m_iLoginCnt;
};
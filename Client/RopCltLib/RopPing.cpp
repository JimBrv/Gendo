#include "stdafx.h"
#include <stdio.h>
#include <ctime>
#include "RopPing.h"
#include "RopUtil.h"
#include "RopProcess.h"


USHORT checksum(USHORT* buff, int size)  
{  
    unsigned long cksum = 0;  
    while(size>1)  
    {  
        cksum += *buff++;  
        size -= sizeof(USHORT);  
    }  
    if(size)  
    {  
        cksum += *(UCHAR*)buff;  
    }  
    // 将32位的chsum高16位和低16位相加，然后取反  
    cksum = (cksum >> 16) + (cksum & 0xffff);  
    cksum += (cksum >> 16);             
    return (USHORT)(~cksum);  
}  

#include <ws2ipdef.h>  // for IP_TTL
BOOL SetTTL(SOCKET s, int nValue)  
{  
    int ret = ::setsockopt(s, IPPROTO_IP, IP_TTL, (char*)&nValue, sizeof(nValue));  
    return ret != SOCKET_ERROR;  
}  

BOOL SetTimeout(SOCKET s, int nTime, BOOL bRecv)  
{  
    int ret = ::setsockopt(s, SOL_SOCKET, bRecv ? SO_RCVTIMEO : SO_SNDTIMEO, (char*)&nTime, sizeof(nTime));  
    return ret != SOCKET_ERROR;  
}  


BOOL GetHostByName(char *sName, char *sIP)
{
    char *pAddr = NULL;

    if(INADDR_NONE == inet_addr(sName)) {
        struct in_addr in;
        hostent *pHost = gethostbyname(sName);
        if(!pHost) {
            LDebug(DEBUG_L_ERROR, "GetHostByName(): cant resolve name='%s'\n", sName);
            return false;
        }else{
            int iaddr = *(int*)pHost->h_addr_list[0];
            in.S_un.S_addr = (iaddr);
            pAddr = inet_ntoa(in);
            if(!pAddr) {
                LDebug(DEBUG_L_ERROR, "GetHostByName(): inet_ntoa fail\n");
                return false;
            }
        }
    }else{
        pAddr = sName;
    }

    strcpy(sIP, pAddr);
    return TRUE;
}


BOOL GetHostByName(TCHAR *sName, char *sIP)
{
    char szDestIp[32] = {0};
    char *pAddr = NULL;
    int  iLen = 32;
    wc2mb(sName, wcslen(sName), szDestIp, &iLen);

    return GetHostByName(szDestIp, sIP);
}


/* 
 * Ping will block for some second(default 3 sec)
 */
INT Ping(TCHAR *sIP, int iSecond)
{
    char szDestIp[128] = {0};
    int  iLen = 128;
    SOCKET sRaw = ::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);  
    SetTimeout(sRaw, 1000, TRUE);  

    if (!GetHostByName(sIP, szDestIp)) {
        LDebug(DEBUG_L_ERROR, "Ping(): cant resolve name='%S' address!\n", sIP);
        return -1;
    }

    SOCKADDR_IN dest;  
    dest.sin_family = AF_INET;  
    dest.sin_port = htons(0);  
    dest.sin_addr.S_un.S_addr = inet_addr(szDestIp);  

    char buff[sizeof(ICMP_HDR) + 32];  
    ICMP_HDR* pIcmp = (ICMP_HDR*)buff;  

    // 填写ICMP封包数据，请求一个ICMP回显  
    pIcmp->icmp_type = 8;      
    pIcmp->icmp_code = 0;  
    pIcmp->icmp_id = (USHORT)::GetCurrentProcessId();  
    pIcmp->icmp_checksum = 0;  
    pIcmp->icmp_sequence = 0;  

    // 填充数据部分，可以为任意  
    memset(&buff[sizeof(ICMP_HDR)], 'E', 32);  

    // 开始发送和接收ICMP封包  
    USHORT  nSeq = 0;  
    char recvBuf[1024];  
    SOCKADDR_IN from;  
    int nLen = sizeof(from);
    BOOL bReached = FALSE;
    DWORD dwCur = ::GetTickCount();

    while((::GetTickCount() - dwCur) < (iSecond*1000))  
    {  
        int nRet;  
        pIcmp->icmp_checksum = 0;  
        pIcmp->icmp_timestamp = ::GetTickCount();  
        pIcmp->icmp_sequence = nSeq++;  
        pIcmp->icmp_checksum = checksum((USHORT*)buff, sizeof(ICMP_HDR) + 32);  
        nRet = ::sendto(sRaw, buff, sizeof(ICMP_HDR) + 32, 0, (SOCKADDR *)&dest, sizeof(dest));  
        if(nRet == SOCKET_ERROR)  
        {  
            LDebug(DEBUG_L_WARNING, "ping: sendto(%s) fail=%d\n", szDestIp, ::WSAGetLastError());  
            return -1;  
        }  
        nRet = ::recvfrom(sRaw, recvBuf, 1024, 0, (sockaddr*)&from, &nLen);  
        if(nRet == SOCKET_ERROR)  
        {  
            if(::WSAGetLastError() == WSAETIMEDOUT)  
            {  
                LDebug(DEBUG_L_WARNING, "ping: sendto(%s) %d second timed out\n", szDestIp, iSecond);  
                continue;  
            }  
            LDebug(DEBUG_L_WARNING, "ping: recvfrom(%s) failed: %d\n", szDestIp, ::WSAGetLastError());  
            return -1;  
        }  

        int nTick = ::GetTickCount();  
        if(nRet < sizeof(IPHeader) + sizeof(ICMP_HDR))  
        {  
            LDebug(DEBUG_L_WARNING, "ping: Too few bytes from %s \n", ::inet_ntoa(from.sin_addr));  
        }  

        // 接收到的数据中包含IP头，IP头大小为20个字节，所以加20得到ICMP头  
        ICMP_HDR* pRecvIcmp = (ICMP_HDR*)(recvBuf + 20);   
        if(pRecvIcmp->icmp_type != 0)   
        {  
            LDebug(DEBUG_L_WARNING, "ping: nonecho type %d recvd, from '%s' \n", pRecvIcmp->icmp_type, inet_ntoa(from.sin_addr));  
            return -1;  
        }  

        if(pRecvIcmp->icmp_id != ::GetCurrentProcessId())  
        {  
            LDebug(DEBUG_L_WARNING, "ping: '%s' someone else's packet! \n", szDestIp);  
            //return -1;  
        }  

        char sTmp[256] = {0};
        int n = 0;
        IPHeader *iph = (IPHeader *)recvBuf;
        in_addr sdr;
        sdr.S_un.S_addr = iph->ipSource;
        n += sprintf(sTmp, "From '%s' return %d bytes: ", inet_ntoa(sdr), nRet);  
        n += sprintf(sTmp + n, "packet serial = %d.\t", pRecvIcmp->icmp_sequence);  
        n += sprintf(sTmp + n, "laterncy = %d ms...\n", nTick - pRecvIcmp->icmp_timestamp);  
        LDebug(DEBUG_L_DEBUG, "%s", sTmp);  
        bReached = TRUE;

        ::Sleep(500);  
    } 
    return (bReached ? 0 : -1);
}


/* 
 * Return latency ms(default 3 sec)
 */
INT PingLatency(TCHAR *sIP, int iSecond)
{
    char szDestIp[128] = {0};
    int  iLen = 128;
    int  iLatency = 0;
    int  iCnt     = 0;
    int  ret      = 500;

    if (!EnhancePriv()) {
        LDebug(DEBUG_L_ERROR, "Enhance Priv fail, cant kill\n");
        return 181;
    }
    //SOCKET sRaw = ::socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    SOCKET sRaw = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (sRaw == INVALID_SOCKET) {
        LDebug(DEBUG_L_ERROR, "Ping(): cannot create socket, error=%d\n", ::WSAGetLastError());
        return 181;
    }

    SetTimeout(sRaw, 1000, TRUE);  

    if (!GetHostByName(sIP, szDestIp)) {
        LDebug(DEBUG_L_ERROR, "Ping(): cant resolve name='%S' address!\n", sIP);
        return 182;
    }

    SOCKADDR_IN dest;  
    dest.sin_family = AF_INET;  
    dest.sin_port = htons(0);  
    dest.sin_addr.S_un.S_addr = inet_addr(szDestIp);  

    char buff[sizeof(ICMP_HDR) + 32] = {0};  
    ICMP_HDR* pIcmp = (ICMP_HDR*)buff;

    // 填充数据部分，可以为任意  
    memset(&buff[sizeof(ICMP_HDR)], 'a', 20);  

    // 开始发送和接收ICMP封包  
    USHORT  nSeq = 0;  
    char recvBuf[1024] = {0};  
    SOCKADDR_IN from;  
    int nLen = sizeof(from);
    BOOL bReached = FALSE;
    DWORD dwCur = ::GetTickCount();

    while((::GetTickCount() - dwCur) < (iSecond*1000))  
    {  
        int nRet;
        srand(time(NULL));
        USHORT rd = rand();
        pIcmp->icmp_type = 8;
        pIcmp->icmp_code = 0;
        pIcmp->icmp_checksum = 0;
        pIcmp->icmp_timestamp = ::GetTickCount();  
        pIcmp->icmp_sequence = nSeq++;
        pIcmp->icmp_id       = rd;
        pIcmp->icmp_checksum = checksum((USHORT*)buff, sizeof(ICMP_HDR) + 32);
        LDebug(DEBUG_L_WARNING, "Ping-Req '%s', seq=%d, id=%d, time=%u\n", szDestIp, pIcmp->icmp_sequence, pIcmp->icmp_id, pIcmp->icmp_timestamp);

        nRet = ::sendto(sRaw, buff, sizeof(ICMP_HDR) + 32, 0, (SOCKADDR *)&dest, sizeof(dest));  
        if(nRet == SOCKET_ERROR)  
        {  
            LDebug(DEBUG_L_WARNING, "ping: sendto(%s) fail=%d\n", szDestIp, ::WSAGetLastError());  
            ret = 403;
            goto out;
        }
        Sleep(50);
        nRet = ::recvfrom(sRaw, recvBuf, 1024, 0, (sockaddr*)&from, &nLen);  
        if(nRet == SOCKET_ERROR)  
        {  
            if(::WSAGetLastError() == WSAETIMEDOUT)  
            {  
                LDebug(DEBUG_L_WARNING, "ping: sendto(%s) %d second timed out\n", szDestIp, iSecond);  
                continue;  
            }  
            LDebug(DEBUG_L_WARNING, "ping: recvfrom(%s) failed: %d\n", szDestIp, ::WSAGetLastError());  
            ret = 404;
            goto out;
        }  

        int nTick = ::GetTickCount();  
        if(nRet < sizeof(IPHeader) + sizeof(ICMP_HDR))  
        {  
            LDebug(DEBUG_L_WARNING, "ping-reply: Too few bytes from %s \n", ::inet_ntoa(from.sin_addr));  
        }  

        // 接收到的数据中包含IP头，IP头大小为20个字节，所以加20得到ICMP头  
        ICMP_HDR* pRecvIcmp = (ICMP_HDR*)(recvBuf + 20);   
        if(pRecvIcmp->icmp_type != 0)   
        {  
            LDebug(DEBUG_L_WARNING, "ping-reply: nonecho type %d recvd, from='%s', time=%u\n", 
                pRecvIcmp->icmp_type, inet_ntoa(from.sin_addr), pRecvIcmp->icmp_timestamp);
            //return 405;  
        }  

        if(pRecvIcmp->icmp_id != rd )  
        {  
            LDebug(DEBUG_L_WARNING, "ping-reply: '%s' someone else's packet! \n", szDestIp);  
            //return -1;  
        }  

        char sTmp[256] = {0};
        int n = 0;
        int wait = 0;
        wait =  nTick - pRecvIcmp->icmp_timestamp;
        bReached = TRUE;
        iLatency += wait;
        iCnt++;
        IPHeader *iph = (IPHeader *)recvBuf;
        in_addr sdr, ddr;
        sdr.S_un.S_addr = iph->ipSource;
        ddr.S_un.S_addr = iph->ipDestination;
        n += sprintf(sTmp, "ping-reply: from raw='%s'->'%s', return %d bytes: ", inet_ntoa(sdr), inet_ntoa(ddr), nRet);  
        n += sprintf(sTmp + n, "packet seq = %d, id=%d\t", pRecvIcmp->icmp_sequence, pRecvIcmp->icmp_id);  
        n += sprintf(sTmp + n, "laterncy = %d ms, latency=%d/%d...\n", wait, iLatency, iCnt);  
        LDebug(DEBUG_L_DEBUG, "%s", sTmp);  
        ::Sleep(800);
    }

out:
    closesocket(sRaw);

    if (iCnt > 0 && iLatency > 0) {
        return iLatency/(iCnt)*3/3; // :)
    }else{
        return ret; //:)
    }
}
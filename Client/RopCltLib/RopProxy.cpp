#include "stdafx.h"
#include <stdlib.h>
#include "winsock2.h"
#include "RopUtil.h"
#include "RopProxy.h"

int to64frombits(unsigned char *out, const unsigned char *in, int inlen)
{
    const char base64digits[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (; inlen >= 3; inlen -= 3)
    {
        *out++ = base64digits[in[0] >> 2];
        *out++ = base64digits[((in[0] << 4) & 0x30)|(in[1] >> 4)];
        *out++ = base64digits[((in[1] << 2) & 0x3c)|(in[2] >> 6)];
        *out++ = base64digits[in[2] & 0x3f];
        in += 3;
    }
    if (inlen > 0)
    {
        unsigned char fragment;
        *out++ = base64digits[in[0] >> 2];
        fragment = (in[0] << 4) & 0x30;
        if (inlen > 1)
            fragment |= in[1] >> 4;
        *out++ = base64digits[fragment];
        *out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
        *out++ = '=';
    }
    *out = NULL;
    return 0;
}

DWORD doHttp11Connect(SOCKET s, PPROXY_PARAM proxy, LPCSTR targetSvr, int targetPort)
{
    BYTE  pbMessage[2000]; 
    pbMessage[0] = 0;
    DWORD cbMessage;	       
    DWORD dwRes = 0;

    if(proxy->NeedPass==0) //no need auth
    {
        strcpy((char*)pbMessage, "CONNECT ");
        strcat((char*)pbMessage, targetSvr);
        strcat((char*)pbMessage, ":");
        _itoa(targetPort, (char*)pbMessage + strlen((char*)pbMessage), 10);
        strcat((char*)pbMessage, " HTTP/1.0\r\nUser-Agent: sslvpn\r\n\r\n");
        cbMessage = (DWORD)strlen((char*)pbMessage);
    }
    else //need auth
    {
        //Proxy-Authorization:   Basic     
        char   szAuth[1024+1] = {""};   
        char   szAuthT[1024+1] = {""};   
        sprintf(szAuthT, "%s:%s", proxy->User, proxy->Pass);   
        to64frombits((unsigned char*)szAuth,(unsigned char*)szAuthT,strlen(szAuthT));   
        sprintf((char *)pbMessage, "CONNECT %s:%d HTTP/1.0\r\nProxy-Authorization: Basic %s\r\n\r\n",targetSvr,targetPort,szAuth);   
        cbMessage = (DWORD)strlen((char*)pbMessage);
    }


    if(send(s, (char*)pbMessage, cbMessage, 0) == SOCKET_ERROR)
    {
        Dbg("[SOCKET]OpenHttp11,Error %d sending message to proxy!\n", WSAGetLastError());
        dwRes = -1;
        return dwRes;
    }

    cbMessage = recv(s, (char*)pbMessage, 200, 0);
    if(cbMessage == SOCKET_ERROR)
    {
        Dbg("[SOCKET]OpenHttp11,Error %d receiving message from proxy\n", WSAGetLastError());
        dwRes = -2;
        return dwRes;
    }
    if(!strstr((char*)pbMessage,"200"))
    {
        Dbg("[SOCKET]OpenHttp11,Error:%s", pbMessage);
        dwRes = -3;
        return dwRes;
    }

    return dwRes;
}

DWORD doSock4Connect(SOCKET s, PPROXY_PARAM proxy, LPCSTR targetSvr, int targetPort)
{
    BYTE  pbMessage[2000]; 
    pbMessage[0] = 0;
    DWORD cbMessage;	     
    DWORD dwRes = 0;

    pbMessage[0] = 4;
    pbMessage[1] = 1;
    unsigned short uPort = htons(targetPort);
    memcpy(pbMessage+2, &uPort, 2);
    long v = inet_addr(targetSvr);

    pbMessage[4] = ((unsigned char *)&(v))[0];//10;
    pbMessage[5] = ((unsigned char *)&(v))[1];//23;
    pbMessage[6] = ((unsigned char *)&(v))[2];//7;
    pbMessage[7] = ((unsigned char *)&(v))[3];//197;


    cbMessage = 8;
    if(send(s, (char*)pbMessage, cbMessage, 0) == SOCKET_ERROR)
    {
        Dbg("[SOCKET]OpenSocks4,Error %d sending message to proxy!\n", WSAGetLastError());
        dwRes = -1;
        return dwRes;
    }

    cbMessage = recv(s, (char*)pbMessage, 200, 0);
    if(cbMessage == SOCKET_ERROR)
    {
        Dbg("[SOCKET]OpenSocks4,Error %d receiving message from proxy\n", WSAGetLastError());
        dwRes = -2;
        return dwRes;
    }

    if((int)pbMessage[0] != 0 || (int)pbMessage[1] != 90)
    {
        Dbg("[SOCKET]OpenSocks4,Error:%s", pbMessage);
        dwRes = -3;
        return dwRes;
    }
    return dwRes;
}

DWORD doSock5Connect(SOCKET s, PPROXY_PARAM proxy, LPCSTR targetSvr, int targetPort)
{
    BYTE  pbMessage[2000]; 
    pbMessage[0] = 0;
    DWORD cbMessage;	        
    DWORD dwRes = 0;

    pbMessage[0] = 5;
    pbMessage[1] = 2;
    pbMessage[2] = 0;
    pbMessage[3] = 2;
    cbMessage = 4;
    if(send(s, (char*)pbMessage, cbMessage, 0) == SOCKET_ERROR)
    {
        Dbg("[SOCKET]OpenSocks5,Error %d sending message to proxy!\n", WSAGetLastError());
        dwRes = -1;
        return dwRes;
    }

    cbMessage = recv(s, (char*)pbMessage, 200, 0);
    if(cbMessage == SOCKET_ERROR)
    {
        Dbg("[SOCKET]OpenSocks5,Error %d receiving message from proxy\n", WSAGetLastError());
        dwRes = -2;
        return dwRes;
    }

    if((int)pbMessage[0] == 5)//need auth
    {
        // 			if(proxy->NeedPass == 0)
        // 			{
        // 				Dbg("[SOCKET]OpenHttp11,Error:Auth need.");
        // 				dwRes = -3;
        // 				return dwRes;
        // 			}
        if((int)pbMessage[1] == 2)//usr/psw auth need
        {
            pbMessage[0] = 1;
            cbMessage = strlen(proxy->User);
            pbMessage[1] = (BYTE)cbMessage;
            strncpy((char *)pbMessage+2, proxy->User,strlen(proxy->User));
            cbMessage += 2;
            pbMessage[cbMessage] = strlen(proxy->Pass);
            strcpy((char *)pbMessage+cbMessage+1, proxy->Pass);
            cbMessage = cbMessage + 1 + strlen(proxy->Pass);

            if(send(s, (char*)pbMessage, cbMessage, 0) == SOCKET_ERROR)
            {
                Dbg("[SOCKET]OpenSocks5,Error %d sending message to proxy!\n", WSAGetLastError());
                dwRes = -1;
                return dwRes;
            }

            cbMessage = recv(s, (char*)pbMessage, 200, 0);
            if(cbMessage == SOCKET_ERROR)
            {
                Dbg("[SOCKET]OpenSocks5,Error %d receiving message from proxy\n", WSAGetLastError());
                dwRes = -2;
                return dwRes;
            }

            if((int)pbMessage[1]!=0)
            {
                Dbg("[SOCKET]OpenSocks5,Auth Error:%s", pbMessage);
                dwRes = -3;
                return dwRes;
            }

        }
    }

    //translate DestAddr
    pbMessage[0] = 5;
    pbMessage[1] = 1;
    pbMessage[2] = 0;
    long v = inet_addr(targetSvr);

    if (INADDR_NONE != v) {
        pbMessage[3] = 1;//addr

        pbMessage[4] = ((unsigned char *)&(v))[0];//10;
        pbMessage[5] = ((unsigned char *)&(v))[1];//23;
        pbMessage[6] = ((unsigned char *)&(v))[2];//7;
        pbMessage[7] = ((unsigned char *)&(v))[3];//197;
        cbMessage = 8;
    }else{
        pbMessage[3] = 3;//name
        cbMessage = strlen(targetSvr);    
        pbMessage[4] = (BYTE)cbMessage;

        strcpy((char *)pbMessage+5, targetSvr);
        cbMessage += 5;
    }

    unsigned short uPort = htons(targetPort);
    memcpy(pbMessage+cbMessage, &uPort, 2);
    cbMessage += 2;
    if(send(s, (char*)pbMessage, cbMessage, 0) == SOCKET_ERROR)
    {
        Dbg("[SOCKET]OpenSocks5,Error %d sending message to proxy!\n", WSAGetLastError());
        dwRes = -1;
        return dwRes;
    }

    cbMessage = recv(s, (char*)pbMessage, 200, 0);
    if(cbMessage == SOCKET_ERROR)
    {
        Dbg("[SOCKET]OpenSocks5,Error %d receiving message from proxy\n", WSAGetLastError());
        dwRes = -2;
        return dwRes;
    }

    if(pbMessage[0] != 5 || pbMessage[1] != 0)
    {
        return -3;
    }
    return dwRes;

}

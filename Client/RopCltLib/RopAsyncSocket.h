#ifndef ROP_ASYNCSOCK_H
#define ROP_ASYNCSOCK_H

#include "stdafx.h"

#include "RopAsyncIO.h"
#include "windows.h"

#define PROXY_NAME_LEN 128

#define PROXY_TYPE_NOPROXY 0
#define PROXY_TYPE_SOCKS4  1
#define PROXY_TYPE_SOCKS4A 2
#define PROXY_TYPE_SOCKS5  3
#define PROXY_TYPE_HTTP11  4

typedef struct _PROXY_PARAM
{
    int  Type;
    char Host[PROXY_NAME_LEN];
    int  Port;
    int  NeedPass;
    char User[PROXY_NAME_LEN];
    char Pass[PROXY_NAME_LEN];	
} PROXY_PARAM,*PPROXY_PARAM; 

class RopAsyncSocketException
{
};

class RopAsyncSocket : public virtual RopAsyncIO
{
private:   
    PROXY_PARAM m_ProxyParam;
public:

    int OpenNoProxy(LPCTSTR szServer, DWORD dwPort);
    int OpenSocks4(LPCTSTR szServer, DWORD dwPort);
    int OpenSocks5(LPCTSTR szServer, DWORD dwPort);
    int OpenHttp11(LPCTSTR szServer, DWORD dwPort);
    int to64frombits(unsigned char *out, const unsigned char *in, int inlen);
public: 	
    RopAsyncSocket ();
    virtual ~RopAsyncSocket();	
public:  
    virtual void SetProxy(int Type,char* Host,int Port,int NeedPass,char* User,char* Pass);
    virtual BOOL CheckAndDoShutdown();
    virtual BOOL ShutdownSilent();
    virtual int Open(LPCTSTR szServer, DWORD dwPort);
    virtual int Send (ROP_BUF& Buf);
    virtual int Send (char* buf,DWORD len);
    virtual int SendEx (ROP_BUF& Buf,DWORD dwTimeOut = INFINITE);
    virtual int SendEx (char* buf,DWORD len,DWORD dwTimeOut = INFINITE);


    virtual void Receive();
    virtual int OnReceive();
    virtual int ReceiveAndWait(char* buf,DWORD len);
    virtual int ReceiveAndWaitEx(char* buf,DWORD len,DWORD dwTimeOut = INFINITE);
};
#endif

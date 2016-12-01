#ifndef ROP_SYNCSOCK_H
#define ROP_SYNCSOCK_H

#include "stdafx.h"
#include "lib-msg.h"
#include "RopAsyncIO.h"

class RopSyncSocketException
{

};

/* Synchoronized socket */
class RopSyncSocket : virtual public RopAsyncIO
{
private:   
    //PROXY_PARAM m_ProxyParam;
public:

    /* No proxy support now...
    int OpenNoProxy(LPCTSTR szServer, DWORD dwPort);
    int OpenSocks4(LPCTSTR szServer, DWORD dwPort);
    int OpenSocks5(LPCTSTR szServer, DWORD dwPort);
    int OpenHttp11(LPCTSTR szServer, DWORD dwPort);
    int to64frombits(unsigned char *out, const unsigned char *in, int inlen);
    */
public: 	
    RopSyncSocket ();
    virtual ~RopSyncSocket();	
public:  
    //virtual void SetProxy(int Type,char* Host,int Port,int NeedPass,char* User,char* Pass);
    virtual BOOL CheckAndDoShutdown();
    virtual BOOL ShutdownSilent();
    virtual int  Open(LPCTSTR szServer, DWORD dwPort);
    //virtual int  OpenAnsi(char *szServer, DWORD dwPort);  // for ansi buffer easy
    virtual int  Send (ROP_BUF& Buf);
    virtual int  Send (char* buf,DWORD len);
    virtual int  SendEx (ROP_BUF& Buf,DWORD dwTimeOut = INFINITE);
    virtual int  SendEx (char* buf,DWORD len,DWORD dwTimeOut = INFINITE);


    virtual void Receive();
    virtual int  OnReceive();
    virtual int  ReceiveAndWait(char* buf,DWORD len);
    virtual int  ReceiveAndWaitEx(char* buf,DWORD len,DWORD dwTimeOut = INFINITE);

};


#endif
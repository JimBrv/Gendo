#ifndef ROP_ASYNC_ACCEPTOR_H
#define ROP_ASYNC_ACCEPTOR_H
#include "stdafx.h"
#include <winsock2.h>
#include <windows.h>
#include "RopCltLib.h"


class RopAsyncAcceptorException
{
};

class RopAsyncAcceptor : public virtual RopAsyncIO
{
public: 
    RopAsyncAcceptor ();
    RopAsyncAcceptor(LPCTSTR sPortRegPath, LPCTSTR sPortRegName);
    virtual ~RopAsyncAcceptor();

protected:
    RopAsyncSocket *m_pClient;
    int			    m_MaxClient;
    int			    m_CurClient;
    /* listen port information in registry */
    TCHAR           m_sPortRegPath[128];
    TCHAR           m_sPortRegName[128];

public: 
    virtual RopAsyncIO *Accept();

    virtual BOOL CheckAndDoShutdown();
    virtual int  Open(LPCTSTR szServer, DWORD dwPort);

    virtual void Receive();
    virtual int  OnReceive();
};
#endif
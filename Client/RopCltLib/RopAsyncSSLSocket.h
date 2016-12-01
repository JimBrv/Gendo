#ifndef ROP_ASYNCSSLSOCK_H
#define ROP_ASYNCSSLSOCK_H

#include "stdafx.h"
#include <windows.h>
#include <sspi.h>
#include <schannel.h>

#include "wincrypt.h"
#include "RopUtil.h"
#include "RopAsyncIO.h"

#include <map>
#include <string>
#define SSL_SEND_TIMEOUT 10000
#define SSL_RECV_TIMEOUT 60000
#define IO_BUFFER_SIZE   0x10000
#define DLL_NAME TEXT("Secur32.dll")
#define NT4_DLL_NAME TEXT("Security.dll")


class RopAsyncSecureSocketException
{
};

class RopAsyncSecureSocket : public RopAsyncSocket
{
public:
    PCCERT_CONTEXT m_pRemoteCertContext;	
    int GetCertHash(BYTE* pbElement, DWORD* pcbElement);

public: 
    int m_bSSLEnable;
    RopAsyncSecureSocket ();
    virtual ~RopAsyncSecureSocket();	

public:  
    virtual BOOL CheckAndDoShutdown();
    virtual int Open(LPCTSTR szServer, DWORD dwPort);
    virtual int Send (ROP_BUF& Buf);
    virtual int Send (char* buf,DWORD len);
    virtual int SendEx (ROP_BUF& Buf,DWORD dwTimeOut = INFINITE);
    virtual int SendEx (char* buf,DWORD len,DWORD dwTimeOut = INFINITE);

    virtual void Receive();
    virtual int OnReceive();
    virtual int ReceiveAndWait(char* buf,DWORD len);
    virtual int ReceiveAndWaitEx(char* buf,DWORD len,DWORD dwTimeOut = INFINITE);


protected:
    enum State
    {
        STATE_NONE,
        STATE_INITIAL_BLOB_SENT,
        STATE_HANDSHAKE_DATA_RECEIVED,
        STATE_HANDSHAKE_DATA_SENT,
        STATE_CLOSE_DATA_SENT,
        STATE_DATA_SENT,
        STATE_CLOSE_ACKNOWLEDGE_DATA_SENT,
        STATE_DATA_RECEIVED,
    };

    LPTSTR   pszServerName;
    INT     iPortNumber;
    BOOL    fVerbose;
    LPTSTR   pszUserName;
    DWORD   dwProtocol;
    ALG_ID  aiKeyExch;

    CredHandle hClientCreds;
    CtxtHandle hContext;
    BOOL fCredsInitialized ;
    BOOL fContextInitialized;

    HCERTSTORE      hMyCertStore ;
    SCHANNEL_CRED   SchannelCred;	

    BOOL  bHaveReadValidData ;

    DWORD           m_dwState;
    SECURITY_STATUS m_ss;   
    SecBuffer       m_SecBuf1[1], m_SecBuf2[4];
    ROP_BUF			m_Buf2;
    PBYTE           m_pReadBuffer;    
    DWORD           m_dwReadBufLen;   
    DWORD           m_dwBytesReceived;
    SecBuffer*      m_pExtraBuffer; 
    SecBuffer*		m_pDataBuffer;
    BOOL m_bExtra;

    BOOL	m_bSessionReuse;
    BOOL	m_bSendFlag;
    DWORD   m_dwAlgNum;
#define MAX_ALG_NUM 32
    ALG_ID  m_AlgID[32];

public:
    void SetAlgID(int num,ALG_ID* pAlgID);
    void SetProto(DWORD dwProto);
    static DWORD gAlgNum;
    static DWORD gProto ;
    static CRITICAL_SECTION  csBug;
    static BOOL m_bHavingBug;
    static BOOL HavingBugInDLL();
    static HMODULE g_hSecurity ;	

    static CredHandle hReusableClientCreds;
    static CredHandle hReusableClientCredsSSL2;
    static BOOL bReusableCredInited;

    static PSecurityFunctionTable g_pSSPI;
    static BOOL LoadSecurityLibrary(void);
    static void UnloadSecurityLibrary(void);
    static SECURITY_STATUS CreateReusableCredentials(PCredHandle phCreds,int dwAlgNum,ALG_ID* AlgID,DWORD dwProto);


protected:
    //helper functions used by ssl	
    SECURITY_STATUS CreateCredentials(LPTSTR pszUserName,PCredHandle phCreds);	
    SECURITY_STATUS	PerformClientHandshake(
        SOCKET          Socket,
        PCredHandle     phCreds,
        LPTSTR          pszServerName,
        CtxtHandle *    phContext,
        SecBuffer *     pExtraData);


    SECURITY_STATUS
        ClientHandshakeLoop(
        SOCKET          Socket,
        PCredHandle     phCreds,
        CtxtHandle *    phContext,
        BOOL            fDoInitialRead,
        SecBuffer *     pExtraData);


    void
        DisplayCertChain(
        PCCERT_CONTEXT  pServerCert,
        BOOL            fLocal);


    DWORD
        VerifyServerCertificate(
        PCCERT_CONTEXT  pServerCert,
        PSTR            pszServerName,
        DWORD           dwCertFlags);


    LONG
        DisconnectFromServer(
        PCredHandle     phCreds,
        CtxtHandle *    phContext);

    void
        DisplayConnectionInfo(
        CtxtHandle *phContext);

    void
        GetNewClientCredentials(
        CredHandle *phCreds,
        CtxtHandle *phContext);

    DWORD ReceiveLoop();

};


#endif

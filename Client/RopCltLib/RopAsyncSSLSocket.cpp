/*
 * Windows native SSPI for ssl socket.
 */

#include "stdafx.h"
#include <stdlib.h>

#include "RopUtil.h"
#include "RopAsyncSocket.h"
#include "RopAsyncSSLSocket.h"
#include "RopEventDispatcher.h"

#define SEC_E_CONTEXT_EXPIRED_OLD _HRESULT_TYPEDEF_(0x00090317L)

HMODULE RopAsyncSecureSocket::g_hSecurity = NULL;
PSecurityFunctionTable RopAsyncSecureSocket::g_pSSPI = {0};
CRITICAL_SECTION  RopAsyncSecureSocket::csBug;
BOOL RopAsyncSecureSocket::m_bHavingBug = FALSE;
CredHandle RopAsyncSecureSocket::hReusableClientCredsSSL2 = {0};
CredHandle RopAsyncSecureSocket::hReusableClientCreds = {0};
BOOL RopAsyncSecureSocket::bReusableCredInited=FALSE;
DWORD RopAsyncSecureSocket::gAlgNum = 0;
DWORD RopAsyncSecureSocket::gProto = 0;

RopAsyncSecureSocket::RopAsyncSecureSocket()
{
    m_Handle = (HANDLE)INVALID_SOCKET;
    m_dwAlgNum = 0;

    pszServerName   = NULL;
    iPortNumber     = 443;
    fVerbose        = FALSE;
    pszUserName     = NULL;
    dwProtocol      = 0;//SP_PROT_SSL3;
    aiKeyExch       = CALG_RSA_KEYX;
    hMyCertStore = NULL;
    fCredsInitialized = FALSE;
    fContextInitialized = FALSE;
    m_pReadBuffer = NULL;
    m_dwReadBufLen = 0;

    m_Buf2.datal = 0;
    m_Buf2.datap =  NULL;
    m_Buf2.hdrl = 0;
    m_Buf2.hdrp =  NULL;
    m_Buf2.resved = 0;
    m_Buf2.size = 0;
    m_Buf2.waitl = 0;

    m_pExtraBuffer = NULL;
    m_pDataBuffer = NULL;
    m_bExtra = FALSE;
    m_pRemoteCertContext = NULL;

    m_bSSLEnable = 1;
    m_bSessionReuse = FALSE;
    m_bSendFlag = FALSE;
}

RopAsyncSecureSocket::~RopAsyncSecureSocket()
{

    if (m_ShutdownIndicated
            &&(m_Handle != (HANDLE)INVALID_SOCKET)) {
        Dbg("[SCONNECT]%p Enter RopAsyncSecureSocket without close.",this);
    }

    try {
        if (m_Handle != (HANDLE)INVALID_SOCKET)
            DisconnectFromServer(&hClientCreds,&hContext);
        if (fContextInitialized) {
            g_pSSPI->DeleteSecurityContext(&hContext);
            fContextInitialized = FALSE;
        }

        if (hMyCertStore) {
            CertCloseStore(hMyCertStore, 0);
        }

        if (m_Handle != (HANDLE)INVALID_SOCKET) { 
            shutdown((SOCKET)m_Handle,SD_BOTH );
            closesocket((SOCKET)m_Handle);
            m_Handle = (HANDLE)INVALID_SOCKET;
        }
        if (m_pReadBuffer) {
            delete m_pReadBuffer;
            m_dwReadBufLen = 0;
            m_pReadBuffer = NULL;
        }

        if (m_pRemoteCertContext) {
            CertFreeCertificateContext(m_pRemoteCertContext);
            m_pRemoteCertContext = NULL;
        }
    } catch (...) {
    }

}

int RopAsyncSecureSocket::Open(LPCTSTR szServer, DWORD dwPort)
{
    int dwRes = 0;
    unsigned char flag = 0xff;
    SECURITY_STATUS Status;
    SecBuffer  ExtraData;
    PCCERT_CONTEXT pRemoteCertContext = NULL;

    if (!g_hSecurity) {
        return -1;
    }

    if (!m_bSessionReuse) { //NC
        //Dbg ("[SSL]:create not-reuseable session\n");
        if (CreateCredentials(pszUserName, &hClientCreds)) {
            Dbg ("[SSL]:Error creating Client credentials(NC)\n");
            return -2;
        }
        fCredsInitialized = TRUE;
    } else { //session reuse used by proxy
        EnterCriticalSection(&csBug);
        if (!bReusableCredInited) {
            //Dbg ("[SSL]:create reuseable session<%d-%d>\n",m_dwAlgNum,dwProtocol);

            if (CreateReusableCredentials( &hReusableClientCreds,m_dwAlgNum,m_AlgID,dwProtocol)) {
                Dbg ("[SSL]:Error creating reuseable session(SSLv3/TLSv1)\n");
                LeaveCriticalSection(&csBug);
                return -2;
            }
            if (CreateReusableCredentials( &hReusableClientCredsSSL2,m_dwAlgNum,m_AlgID,dwProtocol)) {
                Dbg ("[SSL]:Error creating reuseable session(SSL2)\n");
                LeaveCriticalSection(&csBug);
                return -2;
            }

            if (dwProtocol==SP_PROT_SSL2) {
                hClientCreds = hReusableClientCredsSSL2;
            } else {
                hClientCreds = hReusableClientCreds;
            }
            bReusableCredInited = TRUE;
        } else {
            if (dwProtocol==SP_PROT_SSL2) {
                Dbg ("[SSL]:reuse session(SSLv2)\n");
                hClientCreds = hReusableClientCredsSSL2;
            } else {
                Dbg ("[SSL]:reuse session(SSLv3/TLSv1)\n");
                hClientCreds = hReusableClientCreds;
            }
        }
        fCredsInitialized = TRUE;
        LeaveCriticalSection(&csBug);
    }

    RopAsyncSocket::Open(szServer,dwPort);
    if (m_Handle == (HANDLE)INVALID_SOCKET) {
        dwRes = -3;
        Dbg ("[SSL]:Connect Failed %d\n",GetLastError());
        goto cleanup;
    }

    if (m_bSendFlag) {
        RopAsyncSocket::Send((char*)&flag,1);
    }
    if (PerformClientHandshake((SOCKET)m_Handle,&hClientCreds,pszServerName,&hContext,&ExtraData)) {
        if (bReusableCredInited) {
            g_pSSPI->FreeCredentialsHandle(&hReusableClientCreds);
            g_pSSPI->FreeCredentialsHandle(&hReusableClientCredsSSL2);
            bReusableCredInited = FALSE;
        }

        Dbg("[SSL]:Error performing handshake\n");
        dwRes = -4;
        goto cleanup;
    }
    fContextInitialized = TRUE;

    Status = g_pSSPI->QueryContextAttributes(&hContext,SECPKG_ATTR_REMOTE_CERT_CONTEXT,(PVOID)&pRemoteCertContext);
    if (Status != SEC_E_OK) {
        dwRes = -5;
        Dbg("[SSL]:Error 0x%x querying remote certificate\n", Status);
        goto cleanup;
    }
    m_pRemoteCertContext = pRemoteCertContext;

#if 0
    Status = VerifyServerCertificate(pRemoteCertContext,pszServerName,0);
    if (Status) {
        printf("Error 0x%x authenticating server credentials!\n", Status);
    }
#endif

    //DisplayConnectionInfo(&hContext);

    SecPkgContext_StreamSizes Sizes;
    Status = g_pSSPI->QueryContextAttributes(&hContext,SECPKG_ATTR_STREAM_SIZES,&Sizes);
    if (Status == SEC_E_OK) {
        m_dwReadBufLen = Sizes.cbHeader + Sizes.cbMaximumMessage + Sizes.cbTrailer;
        m_pReadBuffer = new BYTE[m_dwReadBufLen];
        if (m_pReadBuffer == NULL) {
            Dbg("[SSL]:Allocate Buffer Error(m_pReadBuffer)\n");
            dwRes = -6;
            goto cleanup;
        }
    } else {
        dwRes = -7;
        goto cleanup;
    }

cleanup:
    return dwRes;

}
void RopAsyncSecureSocket::Receive()
{

    if (!m_bSSLEnable) {
        RopAsyncSocket::Receive();
        return;
    }
    if (! CheckAndDoShutdown()
            &&  (m_Handle != (HANDLE)INVALID_SOCKET)) {
        DWORD Flags = 0;
        DWORD dwRes = 0;

        WSABUF wsabuf;
        wsabuf.buf = (char *) m_Buf.datap;
        wsabuf.len = m_Buf.size;
        bHaveReadValidData = FALSE;

        if (m_Buf2.datal > 0) {
            //there are decrypted data needed to be copyed to client.
            //(previous receive did not fetch all data because of small buffer)
            Dbg("[SSL]:Privious Received data Available\n");
        } else {
            if (m_pExtraBuffer) {
                MoveMemory(m_Buf.datap, m_pExtraBuffer->pvBuffer, m_pExtraBuffer->cbBuffer);
                m_Buf.datal = m_pExtraBuffer->cbBuffer;
                m_dwBytesReceived = m_pExtraBuffer->cbBuffer;
                m_SecBuf2[0].pvBuffer = m_Buf.datap;
                m_SecBuf2[0].cbBuffer = m_Buf.datal;
                m_bExtra = TRUE;
            } else {
                m_dwBytesReceived = 0;
                m_ss = SEC_E_INCOMPLETE_MESSAGE;
                m_SecBuf2[0].pvBuffer = m_Buf.datap;
                m_SecBuf2[0].cbBuffer = 0;
            }

            m_dwState = STATE_NONE;
            dwRes = ReceiveLoop();
            if (m_bExtra) {
                SetEvent(m_OlRead.hEvent);
            }
        }
    }
}

//<=0 for error
int RopAsyncSecureSocket::OnReceive()
{
    if (!m_bSSLEnable) {
        return RopAsyncSocket::OnReceive();
    }

    unsigned long Status = 0;
    if (m_bExtra) {
        m_bExtra = FALSE;
        EventReset();
        return 1;
    }
    if (!m_Sync) {
        if (m_Status) { //error;
            if (m_EventDispatcher) {
                m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf2,EVENT_TYPE_SOCKET_ERROR);
            }
            EventReset();
            return -1;
        } else {
            DWORD Transfered = 0;
            if (!WSAGetOverlappedResult((SOCKET)m_Handle,&m_OlRead,&Transfered,FALSE,&m_Flags)) {
                m_Status = WSAGetLastError ();
                if (m_EventDispatcher) {
                    m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf2,EVENT_TYPE_SOCKET_ERROR);
                }
                EventReset();
//				Dbg ("[SSL]:Socket Error:%d.\n",m_Status);
                return -1;
            }
            if (Transfered==0) {
//				Dbg ("[SSL]:Socket Closed by Peer\n");
                if (m_EventDispatcher) {
                    m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf2,EVENT_TYPE_SOCKET_ERROR);
                }
                EventReset();
                return 0;
            }
        }

        m_Buf.datal += m_OlRead.InternalHigh;
        Status = m_OlRead.Internal;
    } else {
        if (m_Buf.datal == 0) {
//			Dbg ("[SSL]:Socket Closed by Peer\n");
            if (m_EventDispatcher) {
                m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf2,EVENT_TYPE_SOCKET_ERROR);
            }
            EventReset();
            return 0;
        }
    }

    if (m_dwState == STATE_DATA_RECEIVED) {
        ReceiveLoop();
    }

    EventReset();
    return 1;

}

DWORD RopAsyncSecureSocket::ReceiveLoop()
{
    DWORD dwRes = 0;
    DWORD dwSize = 0;
    m_pDataBuffer = m_pExtraBuffer = NULL;

    do {
        switch (m_dwState) {
            case STATE_NONE:
                if (m_dwBytesReceived == 0 || m_ss == SEC_E_INCOMPLETE_MESSAGE) {
                    m_dwState = STATE_DATA_RECEIVED;
                    RopAsyncSocket::Receive();
                    //need to process error?
                    goto exitloop;
                }
            case STATE_DATA_RECEIVED: {
                SecBufferDesc SecBuffDesc = {0};

                m_SecBuf2[0].BufferType = SECBUFFER_DATA;
                m_SecBuf2[0].cbBuffer   = m_Buf.datal;
                m_SecBuf2[1].BufferType = SECBUFFER_EMPTY;
                m_SecBuf2[2].BufferType = SECBUFFER_EMPTY;
                m_SecBuf2[3].BufferType = SECBUFFER_EMPTY;
                SecBuffDesc.ulVersion = SECBUFFER_VERSION;
                SecBuffDesc.cBuffers  = 4;
                SecBuffDesc.pBuffers  = m_SecBuf2;

                m_ss = g_pSSPI->DecryptMessage(&hContext, &SecBuffDesc, 0, NULL);
                if (m_ss == SEC_E_INCOMPLETE_MESSAGE) {
                    EventReset();
                    m_dwState = STATE_NONE;
                    if (m_bExtra) {
                        m_pExtraBuffer = NULL;
                    }
                    goto exitloop;
                }

                if (m_ss == SEC_E_CONTEXT_EXPIRED || m_ss == SEC_E_CONTEXT_EXPIRED_OLD) {
                    //need more processing?
                    Dbg ("[SSL]:SEC_E_CONTEXT_EXPIRED or SEC_E_CONTEXT_EXPIRED_OLD\n");
                    goto exitloop;
                }

                if (m_ss != SEC_E_OK && m_ss != SEC_I_RENEGOTIATE) {
                    Dbg ("[SSL]:Decrypt Error\n");
                    dwRes = m_ss;
                    goto exitloop;
                }

                for (int i = 1; i < 4; i++) {
                    if (m_pDataBuffer == NULL && m_SecBuf2[i].BufferType == SECBUFFER_DATA)
                        m_pDataBuffer = m_SecBuf2 + i;
                    if (m_pExtraBuffer == NULL && m_SecBuf2[i].BufferType == SECBUFFER_EXTRA)
                        m_pExtraBuffer = m_SecBuf2 + i;
                }

                if (m_pDataBuffer) {
                    dwSize = m_pDataBuffer->cbBuffer;
                    if (dwSize > m_Buf.size) {
                        //this will never happen.
                        dwSize = m_Buf.size;
                        Dbg ("[SSL]:Receive Buffer Overflow.\n");
                    } else {
                    }

                    m_Buf2.datap = m_pDataBuffer->pvBuffer;
                    m_Buf2.datal = dwSize;

                }

                if (m_ss == SEC_I_RENEGOTIATE) {
                    Dbg ("[SSL]:RENEGOTIATE.\n");
                    dwRes = ClientHandshakeLoop((SOCKET)m_Handle,&hClientCreds,&hContext,FALSE, m_pExtraBuffer);
                    if (dwRes != SEC_E_OK) {
                        goto exitloop;
                    }
                }

                if (m_pDataBuffer) {
                    bHaveReadValidData = TRUE;
                }
                m_dwState = STATE_NONE;
            }
        }
    } while (m_ss == SEC_E_INCOMPLETE_MESSAGE);

exitloop:

    if (dwRes) {
    } else if (bHaveReadValidData) {
        EventReset();
        if (m_EventDispatcher) {
            m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf2,EVENT_TYPE_SOCKETDATA_RECVED);
        }
        m_Rx+=m_Buf2.datal;
        m_Buf2.datal = 0;
        m_Buf.datal = 0;
    }
    return dwRes;
}

int RopAsyncSecureSocket::Send (ROP_BUF& Buf)
{
    if (!m_bSSLEnable) {
        return RopAsyncSocket::Send(Buf);
    }

    return Send((char*)Buf.datap,Buf.datal);
}
//发送指定长度的buffer
int RopAsyncSecureSocket::Send (char* buf,DWORD len)
{
    if (!m_bSSLEnable) {
        //Dbg("[SSL]: Send with No SSL\n");
        return RopAsyncSocket::Send(buf,len);
    }

    if (len == 0 || buf == NULL) {
        Dbg("[SSL]: Send buffer Error\n");
        return -1;
    }

    SECURITY_STATUS scRet = 0;
    SecBuffer SecBuf[4];

    SecPkgContext_StreamSizes Sizes;
    scRet = g_pSSPI->QueryContextAttributes(
                &hContext,
                SECPKG_ATTR_STREAM_SIZES,
                &Sizes);

    if (scRet == SEC_E_OK) {
        if (len > Sizes.cbMaximumMessage) {
            Dbg("[SSL]: Send Length Error\n");
            return -1;
        }
        DWORD dwIoBufferLength = Sizes.cbHeader + Sizes.cbMaximumMessage + Sizes.cbTrailer;
        if (m_dwReadBufLen < dwIoBufferLength) {
            Dbg("[SSL]: Send Length Error\n");
            return -1;
        }

        SecBuf[0].pvBuffer     = m_pReadBuffer;
        SecBuf[0].cbBuffer     = Sizes.cbHeader;
        SecBuf[0].BufferType   = SECBUFFER_STREAM_HEADER;

        SecBuf[1].pvBuffer     = ((PBYTE)SecBuf[0].pvBuffer) + SecBuf[0].cbBuffer;
        SecBuf[1].cbBuffer     = len;
        SecBuf[1].BufferType   = SECBUFFER_DATA;

        CopyMemory(SecBuf[1].pvBuffer, buf, len);

        SecBuf[2].pvBuffer     = ((PBYTE)SecBuf[1].pvBuffer) + SecBuf[1].cbBuffer;
        SecBuf[2].cbBuffer     = Sizes.cbTrailer;
        SecBuf[2].BufferType   = SECBUFFER_STREAM_TRAILER;

        SecBuf[3].BufferType   = SECBUFFER_EMPTY;

        SecBufferDesc Message;
        Message.ulVersion       = SECBUFFER_VERSION;
        Message.cBuffers        = 4;
        Message.pBuffers        = SecBuf;

        scRet = g_pSSPI->EncryptMessage(&hContext, 0, &Message, 0);
        if (SUCCEEDED(scRet)) {
            int dwRes = 0;
            DWORD dwCountToSend = SecBuf[0].cbBuffer + SecBuf[1].cbBuffer + SecBuf[2].cbBuffer;
            dwRes = RopAsyncSocket::Send((char*)SecBuf[0].pvBuffer, dwCountToSend);
            if (dwRes == -1) { //error processing
                Dbg("[SSL]: Send Error<%d>\n",dwRes);
            }
            //Dbg("[SSL]: Send %d Bytes by Socket %p\n",dwRes,this);
            return dwRes;
        } else {
            Dbg("[SSL]:Send Encrypt Error\n");
        }
    }
    return -1;
}
//设置超时发送。
int RopAsyncSecureSocket::SendEx (ROP_BUF& Buf,DWORD dwTimeOut)
{
    if (!m_bSSLEnable) {
        return RopAsyncSocket::SendEx(Buf,dwTimeOut);
    }

    return SendEx((char*)Buf.datap,Buf.datal,dwTimeOut);
}
//设置超时发送长度
int RopAsyncSecureSocket::SendEx (char* buf,DWORD len,DWORD dwTimeOut)
{
    if (!m_bSSLEnable) {
        //Dbg("[SSL]: Send with No SSL\n");
        return RopAsyncSocket::SendEx(buf,len,dwTimeOut);
    }

    if (len == 0 || buf == NULL) {
        Dbg("[SSL]: Send buffer Error\n");
        return -1;
    }

    SECURITY_STATUS scRet = 0;
    SecBuffer SecBuf[4];

    SecPkgContext_StreamSizes Sizes;
    scRet = g_pSSPI->QueryContextAttributes(
                &hContext,
                SECPKG_ATTR_STREAM_SIZES,
                &Sizes);

    if (scRet == SEC_E_OK) {
        if (len > Sizes.cbMaximumMessage) {
            Dbg("[SSL]: Send Length Error\n");
            return -1;
        }
        DWORD dwIoBufferLength = Sizes.cbHeader + Sizes.cbMaximumMessage + Sizes.cbTrailer;
        if (m_dwReadBufLen < dwIoBufferLength) {
            Dbg("[SSL]: Send Length Error\n");
            return -1;
        }

        SecBuf[0].pvBuffer     = m_pReadBuffer;
        SecBuf[0].cbBuffer     = Sizes.cbHeader;
        SecBuf[0].BufferType   = SECBUFFER_STREAM_HEADER;

        SecBuf[1].pvBuffer     = ((PBYTE)SecBuf[0].pvBuffer) + SecBuf[0].cbBuffer;
        SecBuf[1].cbBuffer     = len;
        SecBuf[1].BufferType   = SECBUFFER_DATA;

        CopyMemory(SecBuf[1].pvBuffer, buf, len);

        SecBuf[2].pvBuffer     = ((PBYTE)SecBuf[1].pvBuffer) + SecBuf[1].cbBuffer;
        SecBuf[2].cbBuffer     = Sizes.cbTrailer;
        SecBuf[2].BufferType   = SECBUFFER_STREAM_TRAILER;

        SecBuf[3].BufferType   = SECBUFFER_EMPTY;

        SecBufferDesc Message;
        Message.ulVersion       = SECBUFFER_VERSION;
        Message.cBuffers        = 4;
        Message.pBuffers        = SecBuf;

        scRet = g_pSSPI->EncryptMessage(&hContext, 0, &Message, 0);
        if (SUCCEEDED(scRet)) {
            int dwRes = 0;
            DWORD dwCountToSend = SecBuf[0].cbBuffer + SecBuf[1].cbBuffer + SecBuf[2].cbBuffer;
            dwRes = RopAsyncSocket::SendEx((char*)SecBuf[0].pvBuffer, dwCountToSend,dwTimeOut);
            if (dwRes == -1) { //error processing
                Dbg("[SSL]: Send Error<%d>\n",dwRes);
            }

			if (dwRes == (-1)*WAIT_TIMEOUT) { //error processing
                Dbg("[SSL]: Send Timeout(%d)\n",dwTimeOut);
            }
            //Dbg("[SSL]: Send %d Bytes by Socket %p\n",dwRes,this);
            return dwRes;
        } else {
            Dbg("[SSL]:Send Encrypt Error\n");
        }
    }
    return -1;
}
//异步处理接受并等待，设置超时等待时间。
int RopAsyncSecureSocket::ReceiveAndWaitEx(char* buf,DWORD len,DWORD dwTimeOut)
{
    if (!m_bSSLEnable) {
        return RopAsyncSocket::ReceiveAndWaitEx(buf,len,dwTimeOut);
    }

    if (buf==NULL || len<=0)return -1;
    int dwRes = 0;
    DWORD cbData= 0;
    DWORD cbIoBuffer = 0;
    SECURITY_STATUS scRet ;

    if (m_pExtraBuffer) {
        MoveMemory(m_pReadBuffer, m_pExtraBuffer->pvBuffer, m_pExtraBuffer->cbBuffer);
        cbIoBuffer = m_pExtraBuffer->cbBuffer;
        scRet = SEC_E_INCOMPLETE_MESSAGE;
        m_pExtraBuffer = NULL;
    }

    while (TRUE) {
        if (0 == cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE) {
            cbData = RopAsyncSocket::ReceiveAndWaitEx((char*)m_pReadBuffer+cbIoBuffer,min(len,m_dwReadBufLen-cbIoBuffer),dwTimeOut);
			if(cbData == (-1)*WAIT_TIMEOUT)
			{
				Dbg ("[SSL]:Receive TimeOut\n");
				return (-1)*WAIT_TIMEOUT;
			} else if (cbData == SOCKET_ERROR) {
                Dbg ("[SSL]:Error %d reading data from server\n", WSAGetLastError());
                scRet = SEC_E_INTERNAL_ERROR;
                break;
            } else if (cbData <= 0) {
                return -1;
            } else {
                cbIoBuffer += cbData;
            }
        }

        m_SecBuf2[0].pvBuffer     = m_pReadBuffer;
        m_SecBuf2[0].cbBuffer     = cbIoBuffer;
        m_SecBuf2[0].BufferType   = SECBUFFER_DATA;

        m_SecBuf2[1].BufferType   = SECBUFFER_EMPTY;
        m_SecBuf2[2].BufferType   = SECBUFFER_EMPTY;
        m_SecBuf2[3].BufferType   = SECBUFFER_EMPTY;

        SecBufferDesc Message;
        Message.ulVersion       = SECBUFFER_VERSION;
        Message.cBuffers        = 4;
        Message.pBuffers        = m_SecBuf2;

        scRet = g_pSSPI->DecryptMessage(&hContext, &Message, 0, NULL);
        if (scRet == SEC_E_INCOMPLETE_MESSAGE) {
            continue;
        }
        if (scRet == SEC_I_CONTEXT_EXPIRED)
            break;

        if ( scRet != SEC_E_OK &&
                scRet != SEC_I_RENEGOTIATE &&
                scRet != SEC_I_CONTEXT_EXPIRED) {
            Dbg ("[SSL]:Error 0x%x returned by DecryptMessage\n", scRet);
            return -1;
        }

        SecBuffer* pDataBuffer  = NULL;
        SecBuffer* pExtraBuffer = NULL;
        for (int i = 1; i < 4; i++) {
            if (pDataBuffer == NULL && m_SecBuf2[i].BufferType == SECBUFFER_DATA) {
                pDataBuffer = &m_SecBuf2[i];
            }
            if (pExtraBuffer == NULL && m_SecBuf2[i].BufferType == SECBUFFER_EXTRA) {
                m_pExtraBuffer = pExtraBuffer = &m_SecBuf2[i];
            }
        }

        if (pDataBuffer) {
            CopyMemory(buf,pDataBuffer->pvBuffer, min(len,pDataBuffer->cbBuffer));
            dwRes = pDataBuffer->cbBuffer;
            return dwRes;
        }

        if (pExtraBuffer) {
        } else {
            cbIoBuffer = 0;
        }
        //to be processed
        if (scRet == SEC_I_RENEGOTIATE) {
        }
    }
    return dwRes;
}
//接受并等待
int RopAsyncSecureSocket::ReceiveAndWait(char* buf,DWORD len)
{
    if (!m_bSSLEnable) {
        return RopAsyncSocket::ReceiveAndWait(buf,len);
    }

    if (buf==NULL || len<=0)return -1;
    int dwRes = 0;
    DWORD cbData= 0;
    DWORD cbIoBuffer = 0;
    SECURITY_STATUS scRet ;

    if (m_pExtraBuffer) {
        MoveMemory(m_pReadBuffer, m_pExtraBuffer->pvBuffer, m_pExtraBuffer->cbBuffer);
        cbIoBuffer = m_pExtraBuffer->cbBuffer;
        scRet = SEC_E_INCOMPLETE_MESSAGE;
        m_pExtraBuffer = NULL;
    }

    while (TRUE) {
        if (0 == cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE) {
            cbData = RopAsyncSocket::ReceiveAndWait((char*)m_pReadBuffer+cbIoBuffer,min(len,m_dwReadBufLen-cbIoBuffer));
            if (cbData == SOCKET_ERROR) {
                Dbg ("[SSL]:Error %d reading data from server\n", WSAGetLastError());
                scRet = SEC_E_INTERNAL_ERROR;
                break;
            } else if (cbData <= 0) {
                return -1;
            } else {
                cbIoBuffer += cbData;
            }
        }

        m_SecBuf2[0].pvBuffer     = m_pReadBuffer;
        m_SecBuf2[0].cbBuffer     = cbIoBuffer;
        m_SecBuf2[0].BufferType   = SECBUFFER_DATA;

        m_SecBuf2[1].BufferType   = SECBUFFER_EMPTY;
        m_SecBuf2[2].BufferType   = SECBUFFER_EMPTY;
        m_SecBuf2[3].BufferType   = SECBUFFER_EMPTY;

        SecBufferDesc Message;
        Message.ulVersion       = SECBUFFER_VERSION;
        Message.cBuffers        = 4;
        Message.pBuffers        = m_SecBuf2;

        scRet = g_pSSPI->DecryptMessage(&hContext, &Message, 0, NULL);
        if (scRet == SEC_E_INCOMPLETE_MESSAGE) {
            continue;
        }
        if (scRet == SEC_I_CONTEXT_EXPIRED)
            break;

        if ( scRet != SEC_E_OK &&
                scRet != SEC_I_RENEGOTIATE &&
                scRet != SEC_I_CONTEXT_EXPIRED) {
            Dbg ("[SSL]:Error 0x%x returned by DecryptMessage\n", scRet);
            return -1;
        }

        SecBuffer* pDataBuffer  = NULL;
        SecBuffer* pExtraBuffer = NULL;
        for (int i = 1; i < 4; i++) {
            if (pDataBuffer == NULL && m_SecBuf2[i].BufferType == SECBUFFER_DATA) {
                pDataBuffer = &m_SecBuf2[i];
            }
            if (pExtraBuffer == NULL && m_SecBuf2[i].BufferType == SECBUFFER_EXTRA) {
                m_pExtraBuffer = pExtraBuffer = &m_SecBuf2[i];
            }
        }

        if (pDataBuffer) {
            CopyMemory(buf,pDataBuffer->pvBuffer, min(len,pDataBuffer->cbBuffer));
            dwRes = pDataBuffer->cbBuffer;
            return dwRes;
        }

        if (pExtraBuffer) {
        } else {
            cbIoBuffer = 0;
        }
        //to be processed
        if (scRet == SEC_I_RENEGOTIATE) {
        }
    }
    return dwRes;
}
//加载安全库
BOOL RopAsyncSecureSocket::LoadSecurityLibrary(void)
{
    INIT_SECURITY_INTERFACE         pInitSecurityInterface;
    OSVERSIONINFO VerInfo;
    WCHAR lpszDLL[MAX_PATH];

    InitializeCriticalSection(&csBug);
    if (HavingBugInDLL()) {
        m_bHavingBug = TRUE;
        Dbg("[SSL]:This Platform having a bug in sconnect.dll.\n");
    }

    VerInfo.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
    if (!GetVersionEx (&VerInfo)) {
        return FALSE;
    }

    if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT
            && VerInfo.dwMajorVersion == 4) {
        wcscpy(lpszDLL, NT4_DLL_NAME);
    } else if (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS ||
               VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT ) {
        wcscpy(lpszDLL, DLL_NAME );
    } else {
        return FALSE;
    }

    g_hSecurity = LoadLibrary(lpszDLL);
    if (g_hSecurity == NULL) {
        Dbg("[SSL]:Error 0x%x loading %s.\n", GetLastError(), (char*)lpszDLL);
        return FALSE;
    }

    pInitSecurityInterface = (INIT_SECURITY_INTERFACE)GetProcAddress(
                                 g_hSecurity,
                                 "InitSecurityInterfaceA");

    if (pInitSecurityInterface == NULL) {
        printf("Error 0x%x reading InitSecurityInterface entry point.\n",
               GetLastError());
        return FALSE;
    }

    g_pSSPI = pInitSecurityInterface();

    if (g_pSSPI == NULL) {
        Dbg("[SSL]:Error 0x%x reading security interface.\n",
                 GetLastError());
        return FALSE;
    }

    return TRUE;
}
//卸载安全库
void
RopAsyncSecureSocket::UnloadSecurityLibrary(void)
{
    FreeLibrary(g_hSecurity);
    g_hSecurity = NULL;
    DeleteCriticalSection(&csBug);
    if (HavingBugInDLL()) {

    }
    if (bReusableCredInited) {
        g_pSSPI->FreeCredentialsHandle(&hReusableClientCreds);
        g_pSSPI->FreeCredentialsHandle(&hReusableClientCredsSSL2);
        bReusableCredInited = FALSE;
    }
}

SECURITY_STATUS
RopAsyncSecureSocket::CreateReusableCredentials(PCredHandle phCreds,int dwAlgNum,ALG_ID* AlgID,DWORD dwProto)// out
{
    TimeStamp       tsExpiry;
    SECURITY_STATUS Status;

    DWORD           cSupportedAlgs = 0;
    ALG_ID          rgbSupportedAlgs[MAX_ALG_NUM];

    PCCERT_CONTEXT  pCertContext = NULL;

    SCHANNEL_CRED lSchannelCred;

    ZeroMemory(&rgbSupportedAlgs, sizeof(rgbSupportedAlgs));
    ZeroMemory(&lSchannelCred, sizeof(lSchannelCred));
    lSchannelCred.dwVersion  = SCHANNEL_CRED_VERSION;
    lSchannelCred.grbitEnabledProtocols = dwProto;
    if (dwAlgNum) {
        rgbSupportedAlgs[cSupportedAlgs++] = AlgID[cSupportedAlgs++];
    }
    if (cSupportedAlgs) {
        lSchannelCred.cSupportedAlgs    = cSupportedAlgs;
        lSchannelCred.palgSupportedAlgs = rgbSupportedAlgs;
    }

    lSchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS;
    lSchannelCred.dwFlags |= SCH_CRED_MANUAL_CRED_VALIDATION;

    Status = g_pSSPI->AcquireCredentialsHandle(
                 NULL,                   // Name of principal
                 UNISP_NAME,           // Name of package
                 SECPKG_CRED_OUTBOUND,   // Flags indicating use
                 NULL,                   // Pointer to logon ID
                 &lSchannelCred,          // Package specific data
                 NULL,                   // Pointer to GetKey() func
                 NULL,                   // Value to pass to GetKey()
                 phCreds,                // (out) Cred Handle
                 &tsExpiry);             // (out) Lifetime (optional)

    if (Status != SEC_E_OK) {
        Dbg("[SSL]:Error 0x%x returned by AcquireCredentialsHandle\n", Status);
        goto cleanup;
    }

cleanup:
    return Status;
}

SECURITY_STATUS
RopAsyncSecureSocket::CreateCredentials(
    LPTSTR     pszUserName,              // in
    PCredHandle phCreds)            // out
{
    TimeStamp       tsExpiry;
    SECURITY_STATUS Status;

    DWORD           cSupportedAlgs = 0;
    ALG_ID          rgbSupportedAlgs[MAX_ALG_NUM];

    PCCERT_CONTEXT  pCertContext = NULL;

    if (hMyCertStore == NULL) {
        hMyCertStore = CertOpenSystemStore(0, TEXT("MY"));
        if (!hMyCertStore) {
            Dbg("[SSL]:Error 0x%x returned by CertOpenSystemStore\n", GetLastError());
            return SEC_E_NO_CREDENTIALS;
        }
    }
    if (pszUserName) {
        pCertContext = CertFindCertificateInStore(hMyCertStore,
                       X509_ASN_ENCODING,
                       0,
                       CERT_FIND_SUBJECT_STR_A,
                       pszUserName,
                       NULL);
        if (pCertContext == NULL) {
            Dbg("[SSL]:Error 0x%x returned by CertFindCertificateInStore\n",GetLastError());
            return SEC_E_NO_CREDENTIALS;
        }
    }
    ZeroMemory(&SchannelCred, sizeof(SchannelCred));
    SchannelCred.dwVersion  = SCHANNEL_CRED_VERSION;
    if (pCertContext) {
        SchannelCred.cCreds = 1;
        SchannelCred.paCred = &pCertContext;
    }
    SchannelCred.grbitEnabledProtocols = dwProtocol;
    /*
    if(aiKeyExch)
    {
    	rgbSupportedAlgs[cSupportedAlgs++] = aiKeyExch;
    }
    */
    if (m_dwAlgNum) {
        rgbSupportedAlgs[cSupportedAlgs++] = m_AlgID[cSupportedAlgs++];
    }

    if (cSupportedAlgs) {
        SchannelCred.cSupportedAlgs    = cSupportedAlgs;
        SchannelCred.palgSupportedAlgs = rgbSupportedAlgs;
    }

    SchannelCred.dwFlags |= SCH_CRED_NO_DEFAULT_CREDS;
    SchannelCred.dwFlags |= SCH_CRED_MANUAL_CRED_VALIDATION;

    Status = g_pSSPI->AcquireCredentialsHandle(
                 NULL,                   // Name of principal
                 UNISP_NAME,           // Name of package
                 SECPKG_CRED_OUTBOUND,   // Flags indicating use
                 NULL,                   // Pointer to logon ID
                 &SchannelCred,          // Package specific data
                 NULL,                   // Pointer to GetKey() func
                 NULL,                   // Value to pass to GetKey()
                 phCreds,                // (out) Cred Handle
                 &tsExpiry);             // (out) Lifetime (optional)
    if (Status != SEC_E_OK) {
        Dbg("[SSL]:Error 0x%x returned by AcquireCredentialsHandle\n", Status);
        goto cleanup;
    }

cleanup:
    if (pCertContext) {
        CertFreeCertificateContext(pCertContext);
    }
    return Status;
}


LONG
RopAsyncSecureSocket::DisconnectFromServer(
    PCredHandle     phCreds,
    CtxtHandle *    phContext)
{
    DWORD           dwType;
    PBYTE           pbMessage;
    DWORD           cbMessage;
    DWORD           cbData;

    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIFlags;
    DWORD           dwSSPIOutFlags;
    TimeStamp       tsExpiry;
    DWORD           Status;

    dwType = SCHANNEL_SHUTDOWN;
    OutBuffers[0].pvBuffer   = &dwType;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = sizeof(dwType);

    OutBuffer.cBuffers  = 1;
    OutBuffer.pBuffers  = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = g_pSSPI->ApplyControlToken(phContext, &OutBuffer);

    if (FAILED(Status)) {
        Dbg("[SSL]:Error 0x%x returned by ApplyControlToken\n", Status);
        goto cleanup;
    }

    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
                  ISC_REQ_REPLAY_DETECT     |
                  ISC_REQ_CONFIDENTIALITY   |
                  ISC_RET_EXTENDED_ERROR    |
                  ISC_REQ_ALLOCATE_MEMORY   |
                  ISC_REQ_STREAM;

    OutBuffers[0].pvBuffer   = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = 0;

    OutBuffer.cBuffers  = 1;
    OutBuffer.pBuffers  = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    Status = g_pSSPI->InitializeSecurityContext(
                 phCreds,
                 phContext,
                 NULL,
                 dwSSPIFlags,
                 0,
                 SECURITY_NATIVE_DREP,
                 NULL,
                 0,
                 phContext,
                 &OutBuffer,
                 &dwSSPIOutFlags,
                 &tsExpiry);

    if (FAILED(Status)) {
        Dbg("[SSL]:Error 0x%x returned by InitializeSecurityContext\n", Status);
        goto cleanup;
    }

    pbMessage = (unsigned char*)OutBuffers[0].pvBuffer;
    cbMessage = OutBuffers[0].cbBuffer;

    if (pbMessage != NULL && cbMessage != 0 && (!m_bBlocked)) {
		
        cbData = RopAsyncSocket::SendEx((char*)pbMessage,cbMessage,SSL_SEND_TIMEOUT);
        if (cbData == SOCKET_ERROR || cbData <= 0) {
            Status = WSAGetLastError();
            Dbg("[SSL]:Error %d sending close notify(%d)\n", Status,cbData);
            goto cleanup;
        }

        g_pSSPI->FreeContextBuffer(pbMessage);
    }
cleanup:
    return Status;
}

SECURITY_STATUS
RopAsyncSecureSocket::
PerformClientHandshake(
    SOCKET          Socket,         // in
    PCredHandle     phCreds,        // in
    LPTSTR          pszServerName,  // in
    CtxtHandle *    phContext,      // out
    SecBuffer *     pExtraData)     // out
{
    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIFlags;
    DWORD           dwSSPIOutFlags;
    TimeStamp       tsExpiry;
    SECURITY_STATUS scRet;
    DWORD           cbData;

    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
                  ISC_REQ_REPLAY_DETECT     |
                  ISC_REQ_CONFIDENTIALITY   |
                  ISC_RET_EXTENDED_ERROR    |
                  ISC_REQ_ALLOCATE_MEMORY   |
                  ISC_REQ_STREAM;

    OutBuffers[0].pvBuffer   = NULL;
    OutBuffers[0].BufferType = SECBUFFER_TOKEN;
    OutBuffers[0].cbBuffer   = 0;

    OutBuffer.cBuffers = 1;
    OutBuffer.pBuffers = OutBuffers;
    OutBuffer.ulVersion = SECBUFFER_VERSION;

    scRet = g_pSSPI->InitializeSecurityContext(
                phCreds,
                NULL,
                pszServerName,
                dwSSPIFlags,
                0,
                SECURITY_NATIVE_DREP,
                NULL,
                0,
                phContext,
                &OutBuffer,
                &dwSSPIOutFlags,
                &tsExpiry);

    if (scRet != SEC_I_CONTINUE_NEEDED) {
        Dbg("[SSL]:Error %x returned by InitializeSecurityContext (1)\n", scRet);
        return scRet;
    }

    if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL) {
        //cbData = send(Socket,(char*)OutBuffers[0].pvBuffer,	OutBuffers[0].cbBuffer,0);
		cbData = RopAsyncSocket::SendEx((char*)OutBuffers[0].pvBuffer,OutBuffers[0].cbBuffer,SSL_SEND_TIMEOUT);
        if (cbData == SOCKET_ERROR || cbData <= 0) {
            Dbg("[SSL]:Error %d sending data to server 1(%d)\n", WSAGetLastError(),cbData);
            g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);
            g_pSSPI->DeleteSecurityContext(phContext);
            return SEC_E_INTERNAL_ERROR;
        }
        g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);
        OutBuffers[0].pvBuffer = NULL;
    }
    return ClientHandshakeLoop(Socket, phCreds, phContext, TRUE, pExtraData);
}

/*****************************************************************************/
SECURITY_STATUS
RopAsyncSecureSocket::ClientHandshakeLoop(
    SOCKET          Socket,         // in
    PCredHandle     phCreds,        // in
    CtxtHandle *    phContext,      // in, out
    BOOL            fDoInitialRead, // in
    SecBuffer *     pExtraData)     // out
{
    SecBufferDesc   InBuffer;
    SecBuffer       InBuffers[2];
    SecBufferDesc   OutBuffer;
    SecBuffer       OutBuffers[1];
    DWORD           dwSSPIFlags;
    DWORD           dwSSPIOutFlags;
    TimeStamp       tsExpiry;
    SECURITY_STATUS scRet;
    DWORD           cbData;

    PUCHAR          IoBuffer;
    DWORD           cbIoBuffer;
    BOOL            fDoRead;

    dwSSPIFlags = ISC_REQ_SEQUENCE_DETECT   |
                  ISC_REQ_REPLAY_DETECT     |
                  ISC_REQ_CONFIDENTIALITY   |
                  ISC_RET_EXTENDED_ERROR    |
                  ISC_REQ_ALLOCATE_MEMORY   |
                  ISC_REQ_STREAM;

    IoBuffer = (unsigned char*)LocalAlloc(LMEM_FIXED, IO_BUFFER_SIZE);
    if (IoBuffer == NULL) {
        Dbg("[SSL]:Out of memory (1)\n");
        return SEC_E_INTERNAL_ERROR;
    }
    cbIoBuffer = 0;

    fDoRead = fDoInitialRead;
    scRet = SEC_I_CONTINUE_NEEDED;

    while (scRet == SEC_I_CONTINUE_NEEDED        ||
            scRet == SEC_E_INCOMPLETE_MESSAGE     ||
            scRet == SEC_I_INCOMPLETE_CREDENTIALS) {

        if (0 == cbIoBuffer || scRet == SEC_E_INCOMPLETE_MESSAGE) {
            if (fDoRead) {
                //cbData = recv(Socket,(char*)IoBuffer + cbIoBuffer, IO_BUFFER_SIZE - cbIoBuffer, 0);
				cbData = RopAsyncSocket::ReceiveAndWaitEx((char*)IoBuffer + cbIoBuffer, IO_BUFFER_SIZE - cbIoBuffer,SSL_RECV_TIMEOUT);
                if (cbData < 0) {
                    Dbg("[SSL]:Error %d reading data from server(%d)\n", WSAGetLastError(),cbData);
                    scRet = SEC_E_INTERNAL_ERROR;
                    break;
                } else if (cbData == 0) {
                    Dbg("[SSL]:Server unexpectedly disconnected\n");
                    scRet = SEC_E_INTERNAL_ERROR;
                    break;
                }
                cbIoBuffer += cbData;
            } else {
                fDoRead = TRUE;
            }
        }

        InBuffers[0].pvBuffer   = IoBuffer;
        InBuffers[0].cbBuffer   = cbIoBuffer;
        InBuffers[0].BufferType = SECBUFFER_TOKEN;

        InBuffers[1].pvBuffer   = NULL;
        InBuffers[1].cbBuffer   = 0;
        InBuffers[1].BufferType = SECBUFFER_EMPTY;

        InBuffer.cBuffers       = 2;
        InBuffer.pBuffers       = InBuffers;
        InBuffer.ulVersion      = SECBUFFER_VERSION;

        OutBuffers[0].pvBuffer  = NULL;
        OutBuffers[0].BufferType= SECBUFFER_TOKEN;
        OutBuffers[0].cbBuffer  = 0;

        OutBuffer.cBuffers      = 1;
        OutBuffer.pBuffers      = OutBuffers;
        OutBuffer.ulVersion     = SECBUFFER_VERSION;

        if (m_bHavingBug) {
            EnterCriticalSection(&csBug);
            try {
                scRet = g_pSSPI->InitializeSecurityContext(phCreds,
                        phContext,
                        NULL,
                        dwSSPIFlags,
                        0,
                        SECURITY_NATIVE_DREP,
                        &InBuffer,
                        0,
                        NULL,
                        &OutBuffer,
                        &dwSSPIOutFlags,
                        &tsExpiry);
            } catch (...) {
                LeaveCriticalSection(&csBug);
            }
            LeaveCriticalSection(&csBug);
        } else {
            scRet = g_pSSPI->InitializeSecurityContext(phCreds,
                    phContext,
                    NULL,
                    dwSSPIFlags,
                    0,
                    SECURITY_NATIVE_DREP,
                    &InBuffer,
                    0,
                    NULL,
                    &OutBuffer,
                    &dwSSPIOutFlags,
                    &tsExpiry);
        }

        if (scRet == SEC_E_OK
                ||scRet == SEC_I_CONTINUE_NEEDED
                ||FAILED(scRet) && (dwSSPIOutFlags & ISC_RET_EXTENDED_ERROR)) {
            if (OutBuffers[0].cbBuffer != 0 && OutBuffers[0].pvBuffer != NULL) {
                //cbData = send(Socket,(char*)OutBuffers[0].pvBuffer,	OutBuffers[0].cbBuffer,0);
				cbData = RopAsyncSocket::SendEx((char*)OutBuffers[0].pvBuffer,	OutBuffers[0].cbBuffer,SSL_SEND_TIMEOUT);
                if (cbData == SOCKET_ERROR || cbData <= 0) {
                    Dbg("[SSL]:Error %d sending data to server 2(%d)\n",	WSAGetLastError(),cbData);
                    g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);
                    g_pSSPI->DeleteSecurityContext(phContext);
                    return SEC_E_INTERNAL_ERROR;
                }
                g_pSSPI->FreeContextBuffer(OutBuffers[0].pvBuffer);
                OutBuffers[0].pvBuffer = NULL;
            }
        }
        if (scRet == SEC_E_INCOMPLETE_MESSAGE) {
            continue;
        }
        if (scRet == SEC_E_OK) {
            if (InBuffers[1].BufferType == SECBUFFER_EXTRA) {
                pExtraData->pvBuffer = LocalAlloc(LMEM_FIXED, InBuffers[1].cbBuffer);
                if (pExtraData->pvBuffer == NULL) {
                    Dbg("[SSL]:Out of memory (2)\n");
                    return SEC_E_INTERNAL_ERROR;
                }

                MoveMemory(pExtraData->pvBuffer,IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),InBuffers[1].cbBuffer);
                pExtraData->cbBuffer   = InBuffers[1].cbBuffer;
                pExtraData->BufferType = SECBUFFER_TOKEN;
            } else {
                pExtraData->pvBuffer   = NULL;
                pExtraData->cbBuffer   = 0;
                pExtraData->BufferType = SECBUFFER_EMPTY;
            }
            break;
        }
        if (FAILED(scRet)) {
            Dbg("[SSL]:Error 0x%x returned by InitializeSecurityContext (2)\n", scRet);
            break;
        }
        if (scRet == SEC_I_INCOMPLETE_CREDENTIALS) {
            GetNewClientCredentials(phCreds, phContext);
            fDoRead = FALSE;
            scRet = SEC_I_CONTINUE_NEEDED;
            continue;
        }
        if (InBuffers[1].BufferType == SECBUFFER_EXTRA ) {
            MoveMemory(IoBuffer,IoBuffer + (cbIoBuffer - InBuffers[1].cbBuffer),InBuffers[1].cbBuffer);
            cbIoBuffer = InBuffers[1].cbBuffer;
        } else {
            cbIoBuffer = 0;
        }
    }

    if (FAILED(scRet)) {
        g_pSSPI->DeleteSecurityContext(phContext);
    }
    LocalFree(IoBuffer);
    return scRet;
}

void
RopAsyncSecureSocket::GetNewClientCredentials(
    CredHandle *phCreds,
    CtxtHandle *phContext)
{
    CredHandle hCreds;
    SecPkgContext_IssuerListInfoEx IssuerListInfo;
    PCCERT_CHAIN_CONTEXT pChainContext;
    CERT_CHAIN_FIND_BY_ISSUER_PARA FindByIssuerPara;
    PCCERT_CONTEXT  pCertContext;
    TimeStamp       tsExpiry;
    SECURITY_STATUS Status;
    Status = g_pSSPI->QueryContextAttributes(phContext,
             SECPKG_ATTR_ISSUER_LIST_EX,
             (PVOID)&IssuerListInfo);
    if (Status != SEC_E_OK) {
        Dbg("[SSL]:Error 0x%x querying issuer list info\n", Status);
        return;
    }

    ZeroMemory(&FindByIssuerPara, sizeof(FindByIssuerPara));
    FindByIssuerPara.cbSize = sizeof(FindByIssuerPara);
    FindByIssuerPara.pszUsageIdentifier = szOID_PKIX_KP_CLIENT_AUTH;
    FindByIssuerPara.dwKeySpec = 0;
    FindByIssuerPara.cIssuer   = IssuerListInfo.cIssuers;
    FindByIssuerPara.rgIssuer  = IssuerListInfo.aIssuers;

    pChainContext = NULL;

    while (TRUE) {
        pChainContext = CertFindChainInStore(hMyCertStore,
                                             X509_ASN_ENCODING,
                                             0,
                                             CERT_CHAIN_FIND_BY_ISSUER,
                                             &FindByIssuerPara,
                                             pChainContext);
        if (pChainContext == NULL) {
            Dbg("[SSL]:Error 0x%x finding cert chain\n", GetLastError());
            break;
        }
        pCertContext = pChainContext->rgpChain[0]->rgpElement[0]->pCertContext;

        // Create schannel credential.
        SchannelCred.dwVersion = SCHANNEL_CRED_VERSION;
        SchannelCred.cCreds = 1;
        SchannelCred.paCred = &pCertContext;

        Status = g_pSSPI->AcquireCredentialsHandle(
                     NULL,                   // Name of principal
                     UNISP_NAME,           // Name of package
                     SECPKG_CRED_OUTBOUND,   // Flags indicating use
                     NULL,                   // Pointer to logon ID
                     &SchannelCred,          // Package specific data
                     NULL,                   // Pointer to GetKey() func
                     NULL,                   // Value to pass to GetKey()
                     &hCreds,                // (out) Cred Handle
                     &tsExpiry);             // (out) Lifetime (optional)
        if (Status != SEC_E_OK) {
            Dbg("[SSL]:Error 0x%x returned by AcquireCredentialsHandle\n", Status);
            continue;
        }
        Dbg("[SSL]:\nnew schannel credential created\n");
        g_pSSPI->FreeCredentialsHandle(phCreds);

        *phCreds = hCreds;
        break;
    }
}

BOOL RopAsyncSecureSocket::CheckAndDoShutdown()
{
    BOOL bRet = FALSE;
    if (m_ShutdownIndicated
            &&(m_Handle != (HANDLE)INVALID_SOCKET))

    {
        DisconnectFromServer(&hClientCreds,&hContext);
        if (fContextInitialized) {
            g_pSSPI->DeleteSecurityContext(&hContext);
            fContextInitialized = FALSE;
        }

        if (!m_bSessionReuse) { //not reuse session,free it now.
            if (fCredsInitialized) {
                g_pSSPI->FreeCredentialsHandle(&hClientCreds);
                fCredsInitialized = FALSE;
            }
        }

        if (m_pReadBuffer) {
            delete m_pReadBuffer;
            m_dwReadBufLen = 0;
            m_pReadBuffer = NULL;
        }
        bRet = TRUE;
    }
    RopAsyncSocket::CheckAndDoShutdown();

    return bRet;
}

int RopAsyncSecureSocket::GetCertHash(BYTE* pbElement, DWORD* pcbElement)
{
    HCERTSTORE         hCertStore = NULL;

    if (!pcbElement || !pcbElement)
        return -1;

    if (CertGetCertificateContextProperty(
                m_pRemoteCertContext,
                CERT_SHA1_HASH_PROP_ID,
                pbElement,
                pcbElement
            )) {
        return 0;
    } else {
        Dbg("[SSL]:CertGetCertificateContextProperty failed. \n");
        return -2;
    }

}


////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
//code unuse now
DWORD
RopAsyncSecureSocket::VerifyServerCertificate(
    PCCERT_CONTEXT  pServerCert,
    PSTR            pszServerName,
    DWORD           dwCertFlags)
{
    return 0;
#if 0
    HTTPSPolicyCallbackData  polHttps;
    CERT_CHAIN_POLICY_PARA   PolicyPara;
    CERT_CHAIN_POLICY_STATUS PolicyStatus;
    CERT_CHAIN_PARA          ChainPara;
    PCCERT_CHAIN_CONTEXT     pChainContext = NULL;

    LPSTR rgszUsages[] = {  szOID_PKIX_KP_SERVER_AUTH,
                            szOID_SERVER_GATED_CRYPTO,
                            szOID_SGC_NETSCAPE
                         };
    DWORD cUsages = sizeof(rgszUsages) / sizeof(LPSTR);

    PWSTR   pwszServerName = NULL;
    DWORD   cchServerName;
    DWORD   Status;

    if (pServerCert == NULL) {
        Status = SEC_E_WRONG_PRINCIPAL;
        goto cleanup;
    }


    //
    // Convert server name to unicode.
    //

    if (pszServerName == NULL || strlen(pszServerName) == 0) {
        Status = SEC_E_WRONG_PRINCIPAL;
        goto cleanup;
    }

    cchServerName = MultiByteToWideChar(CP_ACP, 0, pszServerName, -1, NULL, 0);
    pwszServerName = (unsigned short*)LocalAlloc(LMEM_FIXED, cchServerName * sizeof(WCHAR));
    if (pwszServerName == NULL) {
        Status = SEC_E_INSUFFICIENT_MEMORY;
        goto cleanup;
    }
    cchServerName = MultiByteToWideChar(CP_ACP, 0, pszServerName, -1, pwszServerName, cchServerName);
    if (cchServerName == 0) {
        Status = SEC_E_WRONG_PRINCIPAL;
        goto cleanup;
    }
    //
    // Build certificate chain.
    //

    ZeroMemory(&ChainPara, sizeof(ChainPara));
    ChainPara.cbSize = sizeof(ChainPara);
    ChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_OR;
    ChainPara.RequestedUsage.Usage.cUsageIdentifier     = cUsages;
    ChainPara.RequestedUsage.Usage.rgpszUsageIdentifier = rgszUsages;

    if (!CertGetCertificateChain(
                NULL,
                pServerCert,
                NULL,
                pServerCert->hCertStore,
                &ChainPara,
                0,
                NULL,
                &pChainContext)) {
        Status = GetLastError();
        Dbg("[SSL]:Error 0x%x returned by CertGetCertificateChain!\n", Status);
        goto cleanup;
    }


    //
    // Validate certificate chain.
    //

    ZeroMemory(&polHttps, sizeof(HTTPSPolicyCallbackData));
    polHttps.cbStruct           = sizeof(HTTPSPolicyCallbackData);
    polHttps.dwAuthType         = AUTHTYPE_SERVER;
    polHttps.fdwChecks          = dwCertFlags;
    polHttps.pwszServerName     = pwszServerName;

    memset(&PolicyPara, 0, sizeof(PolicyPara));
    PolicyPara.cbSize            = sizeof(PolicyPara);
    PolicyPara.pvExtraPolicyPara = &polHttps;

    memset(&PolicyStatus, 0, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);

    if (!CertVerifyCertificateChainPolicy(
                CERT_CHAIN_POLICY_SSL,
                pChainContext,
                &PolicyPara,
                &PolicyStatus)) {
        Status = GetLastError();
        Dbg("[SSL]:Error 0x%x returned by CertVerifyCertificateChainPolicy!\n", Status);
        goto cleanup;
    }

    if (PolicyStatus.dwError) {
        Status = PolicyStatus.dwError;
        //DisplayWinVerifyTrustError(Status);
        goto cleanup;
    }


    Status = SEC_E_OK;

cleanup:

    if (pChainContext) {
        CertFreeCertificateChain(pChainContext);
    }

    if (pwszServerName) {
        LocalFree(pwszServerName);
    }

    return Status;
#endif
}

void
RopAsyncSecureSocket::DisplayCertChain(PCCERT_CONTEXT  pServerCert,BOOL fLocal)
{
#if 0
    CHAR szName[1000];
    PCCERT_CONTEXT pCurrentCert;
    PCCERT_CONTEXT pIssuerCert;
    DWORD dwVerificationFlags;

    // display leaf name
    if (!CertNameToStr(pServerCert->dwCertEncodingType,
                       &pServerCert->pCertInfo->Subject,
                       CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                       szName, sizeof(szName))) {
        printf("Error 0x%x building subject name\n", GetLastError());
    }
    if (fLocal) {
        printf("Client subject: %s\n", szName);
    } else {
        printf("Server subject: %s\n", szName);
    }
    if (!CertNameToStr(pServerCert->dwCertEncodingType,
                       &pServerCert->pCertInfo->Issuer,
                       CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                       szName, sizeof(szName))) {
        printf("Error 0x%x building issuer name\n", GetLastError());
    }
    if (fLocal) {
        printf("Client issuer: %s\n", szName);
    } else {
        printf("Server issuer: %s\n\n", szName);
    }


    // display certificate chain
    pCurrentCert = pServerCert;
    while (pCurrentCert != NULL) {
        dwVerificationFlags = 0;
        pIssuerCert = CertGetIssuerCertificateFromStore(pServerCert->hCertStore,
                      pCurrentCert,
                      NULL,
                      &dwVerificationFlags);
        if (pIssuerCert == NULL) {
            if (pCurrentCert != pServerCert) {
                CertFreeCertificateContext(pCurrentCert);
            }
            break;
        }

        if (!CertNameToStr(pIssuerCert->dwCertEncodingType,
                           &pIssuerCert->pCertInfo->Subject,
                           CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                           szName, sizeof(szName))) {
            printf("Error 0x%x building subject name\n", GetLastError());
        }
        printf("CA subject: %s\n", szName);
        if (!CertNameToStr(pIssuerCert->dwCertEncodingType,
                           &pIssuerCert->pCertInfo->Issuer,
                           CERT_X500_NAME_STR | CERT_NAME_STR_NO_PLUS_FLAG,
                           szName, sizeof(szName))) {
            printf("Error 0x%x building issuer name\n", GetLastError());
        }
        printf("CA issuer: %s\n\n", szName);

        if (pCurrentCert != pServerCert) {
            CertFreeCertificateContext(pCurrentCert);
        }
        pCurrentCert = pIssuerCert;
        pIssuerCert = NULL;
    }
#endif
}

void
RopAsyncSecureSocket::DisplayConnectionInfo(CtxtHandle *phContext)
{
    SECURITY_STATUS Status;
    SecPkgContext_ConnectionInfo ConnectionInfo;

    Status = g_pSSPI->QueryContextAttributes(phContext,
             SECPKG_ATTR_CONNECTION_INFO,
             (PVOID)&ConnectionInfo);
    if (Status != SEC_E_OK) {
        Dbg ("[SSL]:Error 0x%x querying connection info\n", Status);
        return;
    }

    printf("\n");

    switch (ConnectionInfo.dwProtocol) {
        case SP_PROT_TLS1_CLIENT:

            Dbg ("[SSL]:Protocol: TLS1\n");
            break;

        case SP_PROT_SSL3_CLIENT:
            Dbg ("[SSL]:Protocol: SSL3\n");
            break;

        case SP_PROT_PCT1_CLIENT:
            Dbg ("[SSL]:Protocol: PCT\n");
            break;

        case SP_PROT_SSL2_CLIENT:
            Dbg ("[SSL]:Protocol: SSL2\n");
            break;

        default:
            Dbg ("[SSL]:Protocol: 0x%x\n", ConnectionInfo.dwProtocol);

    }

    switch (ConnectionInfo.aiCipher) {
        case CALG_RC4:
            Dbg ("[SSL]:Cipher: RC4\n");
            break;

        case CALG_3DES:
            Dbg ("[SSL]:Cipher: 3DES\n");
            break;

        case CALG_RC2:
            Dbg ("[SSL]:Cipher: RC2\n");
            break;

        case CALG_DES:
        case CALG_CYLINK_MEK:
            Dbg ("[SSL]:Cipher: DES\n");
            break;

        case CALG_SKIPJACK:
            Dbg ("[SSL]:Cipher: Skipjack\n");
            break;

        default:
            Dbg ("[SSL]:Cipher: 0x%x\n", ConnectionInfo.aiCipher);
    }
    Dbg ("[SSL]:Cipher strength: %d\n", ConnectionInfo.dwCipherStrength);
    switch (ConnectionInfo.aiHash) {
        case CALG_MD5:
            Dbg ("[SSL]:Hash: MD5\n");
            break;

        case CALG_SHA:
            Dbg ("[SSL]:Hash: SHA\n");
            break;

        default:
            Dbg ("[SSL]:Hash: 0x%x\n", ConnectionInfo.aiHash);
    }
    Dbg ("[SSL]:Hash strength: %d\n", ConnectionInfo.dwHashStrength);

    switch (ConnectionInfo.aiExch) {
        case CALG_RSA_KEYX:
        case CALG_RSA_SIGN:
            Dbg ("[SSL]:Key exchange: RSA\n");
            break;

        case CALG_KEA_KEYX:
            Dbg ("[SSL]:Key exchange: KEA\n");
            break;

        case CALG_DH_EPHEM:
            Dbg ("[SSL]:Key exchange:  DH Ephemeral\n");
            break;

        default:
            Dbg ("[SSL]:Key exchange: 0x%x\n", ConnectionInfo.aiExch);
    }
    Dbg ("[SSL]:Key exchange strength: %d\n", ConnectionInfo.dwExchStrength);
}

BOOL RopAsyncSecureSocket::HavingBugInDLL()
{
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    if (si.dwNumberOfProcessors == 1) {
        return FALSE;
    }
    OSVERSIONINFOEX vi = {0};
    vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (!GetVersionEx((OSVERSIONINFO*)&vi)) {
        vi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        GetVersionEx((OSVERSIONINFO*)&vi);
        //an OS privious to windows 2000? >>>> unsopported in succendo.
        return TRUE;
    } else if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT && vi.dwMajorVersion >= 5) {
        if (vi.dwMajorVersion < 5
                || vi.dwMajorVersion == 5 &&  vi.dwMinorVersion == 0 && vi.wServicePackMajor < 2) {
            return TRUE;
        }
    }
    return FALSE;
}

void RopAsyncSecureSocket::SetAlgID(int num, ALG_ID *pAlgID)
{
    /*
    	m_dwAlgNum = num;
    	if(m_dwAlgNum<0)m_dwAlgNum=0;
    	if(m_dwAlgNum>MAX_ALG_NUM)m_dwAlgNum=MAX_ALG_NUM;
    	memcpy(m_AlgID,pAlgID,sizeof(ALG_ID)*m_dwAlgNum);
    	m_bSessionReuse = TRUE;
    	if(gAlgNum!=m_dwAlgNum)
    	{
    		if(bReusableCredInited)
    		{
    			g_pSSPI->FreeCredentialsHandle(&hReusableClientCreds);
    			g_pSSPI->FreeCredentialsHandle(&hReusableClientCredsSSL2);
    			bReusableCredInited = FALSE;
    		}
    		Dbg ("[SSL]:ALG changed,destroy sessions: %d-%d\n", gAlgNum ,m_dwAlgNum);
    		gAlgNum = m_dwAlgNum;
    	}

    	if(gAlgNum>0)
    		m_bSendFlag = TRUE;
    	else
    		m_bSendFlag = FALSE;
    */
    m_bSendFlag = TRUE;
    return;
}

void RopAsyncSecureSocket::SetProto(DWORD dwProto)
{
    /*
    Dbg ("[SSL]:Set Protocol: %x\n",dwProto);
    dwProtocol = dwProto;
    m_bSessionReuse = TRUE;

    if(gProto!=dwProtocol)
    {
    	if(bReusableCredInited)
    	{
    		g_pSSPI->FreeCredentialsHandle(&hReusableClientCreds);
    		g_pSSPI->FreeCredentialsHandle(&hReusableClientCredsSSL2);
    		bReusableCredInited = FALSE;
    	}
    	Dbg ("[SSL]:Proto changed,destroy sessions: %d-%d\n", gProto ,dwProtocol);
    	gProto = dwProtocol;
    }
    */
    return;
}

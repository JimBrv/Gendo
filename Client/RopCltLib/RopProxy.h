#ifndef _ROP_PROXY_H__
#define _ROP_PROXY_H__

#include "stdio.h"
#include "winsock2.h"
#include "RopAsyncSocket.h"

INT   to64frombits(unsigned char *out, const unsigned char *in, int inlen);
DWORD doHttp11Connect(SOCKET s, PPROXY_PARAM proxy, LPCSTR targetSvr, int targetPort);
DWORD doSock4Connect(SOCKET s, PPROXY_PARAM proxy, LPCSTR targetSvr, int targetPort);
DWORD doSock5Connect(SOCKET s, PPROXY_PARAM proxy, LPCSTR targetSvr, int targetPort);

#endif

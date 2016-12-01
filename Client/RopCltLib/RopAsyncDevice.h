/*
 * Async device controller for Windows kernel driver
 * For example, ETM-Filter
 * All right reserved by Extelecom Inc.
 * W.Y 2011-3
 */

#ifndef ROP_ASYNCDEVICE_H
#define ROP_ASYNCDEVICE_H

#include "stdafx.h"
#include "windows.h"
#include "winioctl.h"
#include "RopBuf.h"
#include "RopModule.h"
#include "RopUtil.h"
#include "RopAsyncIO.h"
#include "RopEventDispatcher.h"

#define METHOD_BUFFERED			0
#define METHOD_IN_DIRECT		1
#define METHOD_OUT_DIRECT		2
#define METHOD_NEITHER			3
#define FILE_ANY_ACCESS			0
#define CTL_CODE(DeviceType, Function, Method, Access)  \
	( \
	((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
	)

#define	IOCTL_ETM_START	      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1111, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_ETM_STOP        CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1112, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_ETM_SET_STATUS  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1113, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define	IOCTL_ETM_GET_STATUS  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x1114, METHOD_BUFFERED, FILE_ANY_ACCESS)



class RopAsyncDeviceException
{
};

class RopAsyncDevice: public virtual RopAsyncIO
{
public: 
	RopAsyncDevice();
	virtual ~RopAsyncDevice();
public:
	virtual BOOL CheckAndDoShutdown();
	virtual int  Open(LPCTSTR szFilePath, DWORD dwDesiredAccess);
	virtual int  Send(ROP_BUF &Buf);
	virtual int  Send(char *buf, DWORD len);

	virtual void Receive();
	virtual int  OnReceive();
	virtual int  ReceiveAndWait(char* buf,DWORD len);
	//virtual void TimeoutEvent();
public:
	BOOL RopAsyncDevice::SetStatus(int State);
};

#endif
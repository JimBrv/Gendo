/*
* Asynchoronised Device for Kernel Device Driver
*/
#include "stdafx.h"
#include "RopAsyncDevice.h"

RopAsyncDevice::RopAsyncDevice ()
{
    m_Handle = INVALID_HANDLE_VALUE;
}

RopAsyncDevice::~RopAsyncDevice()
{
    if (m_Handle != INVALID_HANDLE_VALUE)
        CloseHandle(m_Handle);
}

int RopAsyncDevice::Open(LPCTSTR szFilePath, DWORD dwDesiredAccess)
{
    m_Handle = CreateFile(szFilePath,
						  dwDesiredAccess,
					      FILE_SHARE_READ,
						  0,
						  OPEN_EXISTING,
						  FILE_ATTRIBUTE_SYSTEM | FILE_FLAG_OVERLAPPED,
						  0);

    if (m_Handle == INVALID_HANDLE_VALUE) {
        LDebug(DEBUG_L_ERROR, "Cannot Open EtmFilter Device!!!");
        return -1;
    }

    DWORD len = 0;
	int   ptr = 0;
	BOOL  ret = DeviceIoControl(m_Handle,
								IOCTL_ETM_START,
								NULL,
								0,
								&ptr,
								sizeof(int),
								&len,
								0);
	if (FALSE == ret) {
        LDebug(DEBUG_L_ERROR, "Cannot Send ETM_START IoCtl to EtmFilter!");
		 return -1;
    }     
	LDebug(DEBUG_L_ERROR,"IOCTL_ETM_START=%u\n",IOCTL_ETM_START);
    return 0;
}

BOOL RopAsyncDevice::SetStatus(int State)
{
    if (m_Handle == INVALID_HANDLE_VALUE)
        return FALSE;
    int lState = State;
    DWORD len = 0;
    if (!DeviceIoControl(m_Handle,
                        IOCTL_ETM_SET_STATUS,
                        &lState,
                        sizeof (State),
                        NULL,
                        0,
                        &len,
                        0))
        return FALSE;
    return TRUE;
}

void RopAsyncDevice::Receive()
{
    if (!CheckAndDoShutdown()
        &&  (m_Handle != (HANDLE)INVALID_SOCKET)) {
            try {
                DWORD dwReceived = 0;
                if (m_Sync = ReadFile(m_Handle, m_Buf.datap, PACKET_LEN, &dwReceived, &m_OlRead)) {
                    m_Buf.datal+=dwReceived;
                    SetEvent(m_OlRead.hEvent);
                }
                if ((!m_Sync) && (GetLastError() != ERROR_IO_PENDING)) {
                    m_Status = GetLastError ();
                    SetEvent(m_OlRead.hEvent);
                }

            } catch (...) {
                m_Sync = FALSE;
                m_Status = 1;
                SetEvent(m_OlRead.hEvent);
                LDebug(DEBUG_L_ERROR,"EtmFilter Read Exception\n");
            }
    }
}

int RopAsyncDevice::ReceiveAndWait(char* buf,DWORD len)
{
    int ret = 0;
    if (!CheckAndDoShutdown()) {
        if (ReadFile (m_Handle, buf,len, (unsigned long*)&ret, &m_OlRead))
            ;
        else if (WSAGetLastError() == WSA_IO_PENDING)
		{
			 LDebug(DEBUG_L_ERROR,"WSA_IO_PENDING\n");
            GetOverlappedResult (m_Handle, &m_OlRead, (unsigned long*)&ret, TRUE);
		}
        else {
            ret = -1;
            LDebug(DEBUG_L_ERROR,"EtmFilter Read Fail\n");
        }
        ResetEvent (m_OlRead.hEvent);
    }
    return ret;
}

int RopAsyncDevice::OnReceive()
{
    unsigned long Status = 0;
    if (!m_Sync) {
        if (m_Status) { 
            if (m_EventDispatcher)
			{
				LDebug(DEBUG_L_DEBUG,"m_Status\n");
                m_EventDispatcher->OnEvent(this,GetPeer(),& m_Buf, EVENT_TYPE_DEVICE_ERROR);
			}
            EventReset();
            return -1;
			
        } else {
            DWORD Transfered = 0;
            if (!GetOverlappedResult(m_Handle, &m_OlRead, &Transfered, FALSE) ) {
                m_Status = GetLastError ();
                if (m_EventDispatcher)
				{
					LDebug(DEBUG_L_DEBUG,"AsyncDevice: GetOverlappedResult Error\n");
                    m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf,EVENT_TYPE_DEVICE_ERROR);
				}
                EventReset();
                return -1;
				
            }
        }
        m_Buf.datal += m_OlRead.InternalHigh;
        Status = m_OlRead.Internal;
    }
    if (m_Buf.datal <= 0) {
        if (m_EventDispatcher)
		{
			LDebug(DEBUG_L_DEBUG,"AsyncDevice: m_Buf.datal <= 0!\n");
            m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf,EVENT_TYPE_DEVICE_ERROR);
		}
        return -1;
    } else {
        try {
            if (m_EventDispatcher) {
                m_EventDispatcher->OnEvent(this,GetPeer(),&m_Buf,EVENT_TYPE_DEVICEDATA_RECVED);
            }
        } catch (...) {
            LDebug(DEBUG_L_ERROR, "EtmFilter OnEvent Exception!");
        }
    }
    try {
        EventReset();
    } catch (...) {
        LDebug(DEBUG_L_ERROR, "EtmFilter EventReset Exception.");
    }

    m_Buf.datal = 0;
    return 1;
}

int RopAsyncDevice::Send(ROP_BUF &Buf)
{
    int Written = 0;
    if (Buf.datal > 0) {
        if (WriteFile (m_Handle, Buf.datap, Buf.datal, (unsigned long*)&Written, &m_OlWrite));
        else if (WSAGetLastError() == WSA_IO_PENDING)
            GetOverlappedResult (m_Handle, &m_OlWrite, (unsigned long*)&Written, TRUE);
        else {
            Written = -1;
            LDebug(DEBUG_L_ERROR,"EtmFilter Write Fail\n");
        }
        ResetEvent (m_OlWrite.hEvent);
    }
    return Written;
}

int RopAsyncDevice::Send(char* buf, DWORD len)
{
    int Written = 0;
    if (buf && (len > 0)) {
        if (WriteFile (m_Handle, buf, len, (unsigned long*)&Written, &m_OlWrite));
        else if (WSAGetLastError() == WSA_IO_PENDING) {
            GetOverlappedResult (m_Handle, &m_OlWrite, (unsigned long*)&Written, TRUE);
        }else{
            Written = -1;
            LDebug(DEBUG_L_ERROR,"EtmFilter Write Fail\n");
        }
        ResetEvent (m_OlWrite.hEvent);
    }
    return Written;
}

BOOL RopAsyncDevice::CheckAndDoShutdown()
{
    BOOL bRet = FALSE;
    if (m_ShutdownIndicated
        &&(m_Handle != INVALID_HANDLE_VALUE)) {
			DWORD len = 0;
			int   ptr = 0;
			//CancelPendingIo();
			if (!DeviceIoControl(m_Handle,
				IOCTL_ETM_STOP,
				NULL,
				0,
				&ptr,
				sizeof(int),
				&len,
				0))
			{
				LDebug(DEBUG_L_ERROR, "Cannot Send ETM_STOP IoCtrl to EtmFilter!");
			}
			Sleep(1000);
            CloseHandle(m_Handle);
            m_Handle = INVALID_HANDLE_VALUE;
            bRet = TRUE;
    }
    RopAsyncIO::CheckAndDoShutdown();
    return bRet;
}
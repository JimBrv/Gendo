#include "stdafx.h"
#include "RopUtil.h"
#include "RopExecuter.h"

RopExecuter::RopExecuter() throw (RopExecuterException) : m_State (STATE_STOPPED)
{
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);

    if (WSAStartup( wVersionRequested, &wsaData )) {
        Dbg("RopExecuter Couldn't startup Winsock\n");
        throw RopExecuterException();
    }
}

RopExecuter::~RopExecuter()
{
    Purge();
    WSACleanup();
}

void RopExecuter::Add(RopAsyncIO* pio)
{
    if (pio)
        push_back(pio);
}

void RopExecuter::Del(RopAsyncIO* pio)
{
    if (empty()) {
        return;
    }
    iterator it;

    for (it = begin(); it != end();it++) {
        if (*it == pio) {
            erase(it);
            return;
        }
    }
}

void RopExecuter::Purge()
{
    for (; size(); pop_back())
        delete (back());
}

void RopExecuter::EventLoop()
{
    m_State = STATE_RUNNING;

    BatchAsyncReceive();

    while (HaveValidHandles()) 
        switch (int Index = EventSelect())
        {
            case WAIT_TIMEOUT: 
            {
                    switch (m_State) 
                    {
                        case STATE_PENDING: 
                        {
                            m_State = STATE_FORCE;
                        }
                        break;
                    case STATE_FORCE: 
                        {
                            m_State = STATE_STOPPED;
                            Purge();
                        }
                        break;
                    case STATE_RUNNING: 
                        {
						    static int iTimeoutCount = 0;
						    if(iTimeoutCount++ >= 3) {
							    iTimeoutCount = 0;
							    //Dbg("Main Thread is running...");
						    }
                            for (unsigned long Idx = size(); Idx; --Idx)
                                try {
                                    (*this) [Idx - 1].TimeoutEvent();
                                } catch (...) {
                                    
                                }                            
                        }
                        break;
                    }
                    break;   /* End state */
            }             /* End Timeout */
            break;
            
            /* Call event dispatcher callback */
            default: 
            {
                for (unsigned long Size = size(); Index < Size; ++Index) {
                    try {
                        HANDLE &Handle = (*this) [Index].EventHandle();

                        if (Handle && WaitForSingleObject(Handle, 0) == WAIT_OBJECT_0) {
                            (*this) [Index].OnReceive();
                            (*this) [Index].Receive();
                        }
                    } catch (...) {

                    }
                }
                break; 
            }  /* End default */
        }      /* End while */
}

BOOL RopExecuter::HaveValidHandles()
{
    unsigned long Size = size();
    BOOL Found = FALSE;

    iterator it;
    for (it = begin(); it != end() ;)
        try {
            (*it)->CheckAndDoShutdown();/////add for bug
            if ((*it)->EventHandle()) {
                Found = TRUE;
                it++;
            } else {
                delete *it;
                *it = NULL;
                it = erase(it);
            }
        } catch (...) {
        }

    return (Found);
}

void RopExecuter::BatchAsyncReceive()
{
    for (unsigned long Index = size(); Index; --Index)
        (*this) [Index - 1].Receive();
}

int RopExecuter::EventSelect()
{
    int ItemCount = 0, Result = WAIT_TIMEOUT, Size = size();

    for (unsigned long Index = 0; Index < Size; ++Index) {
        try {
            if (m_EventTable [ItemCount] = (*this) [Index].EventHandle())
                ++ItemCount;
        } catch (...) {
        }
    }

    if (ItemCount && (Result = WaitForMultipleObjects (ItemCount, m_EventTable, FALSE, EVENT_SELECT_TIMEOUT)) != WAIT_TIMEOUT) {
        Result -= WAIT_OBJECT_0;
    }

    return Result;
}

void RopExecuter::Shutdown()
{
    if (m_State == STATE_RUNNING) {
        unsigned long Size = size();
        for (int Index = 0; Index < Size; ++Index)
            if ((*this) [Index].EventHandle()) {
                (*this) [Index].Shutdown();
                (*this) [Index].CancelPendingIo();
            }
		 m_State = STATE_PENDING;
		 Sleep(2000);
    }
}

RopAsyncIO &RopExecuter::operator[] (unsigned long Subscript) throw (RopIndexException)
{
    if (Subscript >= size()) {
        throw RopIndexException();
    }

    try {
        return ((RopAsyncIO &) *((vector <RopAsyncIO *> *) this)->operator[] (Subscript));
    } catch (...) {
    }

    throw RopIndexException();
}


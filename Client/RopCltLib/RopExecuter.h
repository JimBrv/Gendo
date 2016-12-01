#ifndef ROP_EXECUTER_H
#define ROP_EXECUTER_H

#include "stdafx.h"
#include <vector>
#include "RopBase.h"
#include "RopAsyncIO.h"

using namespace std;

#ifndef MAX_DEVICE_NUM
#define MAX_DEVICE_NUM 64
#endif

#ifndef EVENT_SELECT_TIMEOUT
#define EVENT_SELECT_TIMEOUT 1000 
#endif

typedef enum
{
    STATE_RUNNING,
    STATE_PENDING,
    STATE_FORCE,
    STATE_STOPPED
}ShutdownState;

class RopExecuterException
{
};
class RopIndexException
{
};

class RopExecuter : public RopBase, public vector<RopAsyncIO *>
{
private:   ShutdownState m_State;
           HANDLE m_EventTable[MAX_DEVICE_NUM];

public:    RopExecuter() throw (RopExecuterException);
           ~RopExecuter();
           void Add(RopAsyncIO* pio);
           void Del(RopAsyncIO* pio);


public:    void EventLoop() throw (RopExecuterException);
           void Shutdown();
           void Purge();

protected: BOOL HaveValidHandles();
           void BatchAsyncReceive();
           int  EventSelect();

public:    RopAsyncIO &operator[] (unsigned long Subscript) throw (RopIndexException);
};

#endif
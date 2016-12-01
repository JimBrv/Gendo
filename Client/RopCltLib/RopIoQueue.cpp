
#include "stdafx.h"
#include "RopUtil.h"
#include "RopIoQueue.h"


RopIOQueue::RopIOQueue()
{
}

RopIOQueue::~RopIOQueue()
{
    Purge();
}

void RopIOQueue::Add(RopAsyncIO* pio)
{
    if (pio)
        push_back(pio);
}

void RopIOQueue::Del(RopAsyncIO* pio)
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

void RopIOQueue::Purge()
{
    for (; size(); pop_back())
        delete (back());
}

RopAsyncIO &RopIOQueue::operator[] (unsigned long Subscript) throw (RopIOQueueException)
{
    if (Subscript >= size()) {
        throw RopIOQueueException();
    }

    try {
        return ((RopAsyncIO &) *((vector <RopAsyncIO *> *) this)->operator[] (Subscript));
    } catch (...) {
    }

    throw RopIOQueueException();
}

int RopIOQueue::GetSize()
{
    return size();
}

void RopIOQueue::BatchSend(char* buf,int len)
{
    unsigned long Size = size();

    for (int Index = 0; Index < Size; ++Index)
        if ((*this) [Index].EventHandle()) {
            try {
                if((*this) [Index].m_bBlocked)continue;
                int sendl = 0;
                while (sendl < len) {
                    int forsend = len-sendl;
                    int tmp = (*this) [Index].SendEx(buf+sendl,forsend,10000);
                    //Dbg("Batch send %d.\n",tmp);

                    if (tmp <0) {
                        break;
                    }
                    sendl+=tmp;
                }
            } catch (...) {
                Dbg("Batch send error.UI socket num:%d",Size);
            }
        }

}
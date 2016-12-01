#ifndef ROP_IOQUEUE_H
#define ROP_IOQUEUE_H

#include "stdafx.h"
#include <vector>
#include "RopBase.h"
#include "RopAsyncIO.h"

using namespace std;

class RopIOQueueException 
{

};

class RopIOQueue : public RopBase, public vector<RopAsyncIO *>
{
public:    RopIOQueue();
           ~RopIOQueue();
           int GetSize();
           void Add(RopAsyncIO* pio);
           void Del(RopAsyncIO* pio);
           void Purge();
           void BatchSend(char* buf,int len);

public:    RopAsyncIO &operator[] (unsigned long Subscript) throw (RopIOQueueException);
};

#endif
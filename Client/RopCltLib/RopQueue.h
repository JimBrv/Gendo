#ifndef ROP_QUEUE
#define ROP_QUEUE
#include "stdafx.h"
#include <windows.h>
#include "RopBase.h"


typedef struct _queue_s_
{
    unsigned long base;
    unsigned long size;
    unsigned long capacity;
    unsigned long max_size;
    void* data[];
} Queue;

class RopQueue : public RopBase  
{
public:
    DWORD Count();
    PVOID Extract(PVOID item);
    PVOID Pop();
    PVOID Push(PVOID item);
    RopQueue(DWORD capacity);
    virtual ~RopQueue();
private:

    Queue* q;
    CRITICAL_SECTION cs;
};

#endif
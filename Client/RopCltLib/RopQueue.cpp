
#include "stdafx.h"
#include "RopUtil.h"
#include "RopQueue.h"

#define QUEUE_BYTE_ALLOCATION(size) \
    (sizeof (Queue) + (size * sizeof (PVOID)))

#define QUEUE_ADD_INDEX(var, inc) \
do{ \
    var += inc; \
    if (var >= q->capacity) \
    var -= q->capacity; 	\
}while(0);

#define QUEUE_SANITY_CHECK() \
    ASSERT (q != NULL && q->base < q->capacity && q->size <= q->capacity)

#define QueueCount(q) (q->size)

#define UPDATE_MAX_SIZE() \
do{ \
    if (q->size > q->max_size) \
        q->max_size = q->size; \
}while(0);


RopQueue::RopQueue(DWORD capacity)
{
    q= NULL;
    InitializeCriticalSection(&cs);
    if (capacity < 0 ) {
    } else {
        q = (Queue *) MemAlloc (QUEUE_BYTE_ALLOCATION (capacity), TRUE);
        if (!q)
            return ;

        q->base = q->size = 0;
        q->capacity = capacity;
        q->max_size = 0;
    }


}

RopQueue::~RopQueue()
{
    DeleteCriticalSection(&cs);
    if (q) {
        //QUEUE_SANITY_CHECK ();
        MemFree (q, QUEUE_BYTE_ALLOCATION (q->capacity));
        q= NULL;
    }

}

PVOID RopQueue::Push(PVOID item)
{
    ULONG dest;
    //QUEUE_SANITY_CHECK ();
    if (q->size == q->capacity) {
        Dbg("RopQueue Queue Full.");
        return NULL;
    }
    EnterCriticalSection(&cs);
    try
    {
        dest = q->base;
        QUEUE_ADD_INDEX (dest, q->size);
        q->data[dest] = item;
        ++q->size;
        UPDATE_MAX_SIZE();
    }
    catch(...)
    {
    }
    LeaveCriticalSection(&cs);
    return item;
}

PVOID RopQueue::Pop()
{
    ULONG oldbase;
    //QUEUE_SANITY_CHECK ();
    if (!q->size)
        return NULL;
    EnterCriticalSection(&cs);
    try
    {
        oldbase = q->base;
        QUEUE_ADD_INDEX (q->base, 1);
        --q->size;
        UPDATE_MAX_SIZE();
    }
    catch(...)
    {

    }
    LeaveCriticalSection(&cs);
    return q->data[oldbase];

}

PVOID RopQueue::Extract(PVOID item)
{
    ULONG src, dest, count, n;
    //QUEUE_SANITY_CHECK ();
    EnterCriticalSection(&cs);
    try
    {
        n = 0;
        src = dest = q->base;
        count = q->size;
        while (count--) {
            if (item == q->data[src]) {
                ++n;
                --q->size;
            } else {
                q->data[dest] = q->data[src];
                QUEUE_ADD_INDEX (dest, 1);
            }
            QUEUE_ADD_INDEX (src, 1);
        }
    }
    catch(...)
    {
    }
    LeaveCriticalSection(&cs);
    if (n)
        return item;
    else
        return NULL;

}

DWORD RopQueue::Count()
{
    return QueueCount(q);
}

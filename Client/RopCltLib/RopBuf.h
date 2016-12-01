#ifndef ROP_BUF_H
#define ROP_BUF_H

#include "stdafx.h"

#define PACKET_LEN 100*1024

typedef struct _ROP_BUF_
{
    unsigned long size;//full size of data	
    unsigned long resved;//unused;

    void* hdrp;//hdr pointer
    unsigned long hdrl;//hdr length

    void* datap;//data pointer	
    unsigned long datal;//data length
    unsigned long waitl;//bytes wait for sending or receiving

    //	unsigned char data[PACKET_LEN];//buffer allocated;
    unsigned char* data;//buffer allocated;
}ROP_BUF, *PROP_BUF;

#endif
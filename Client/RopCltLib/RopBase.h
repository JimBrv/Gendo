#ifndef ROP_BASE_H
#define ROP_BASE_H
#include "stdafx.h"
#include <tchar.h>
#include "lib-msg.h"

#define WIN_2000   1
#define WIN_XP     2
#define WIN_2003   3
#define WIN_VISTA  4
#define WIN_7      7
#define WIN_8      8

class RopBase
{
public:   
    int   OSVersion();
    char *OSName();
    bool  IsOS64bit();

public:
    char m_os[MAX_NAME_LEN];
};


#endif
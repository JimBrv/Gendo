#ifndef ROP_ROUTETABLE_H
#define ROP_ROUTETABLE_H
#include "stdafx.h"
#include <IPHlpApi.h>
#include "RopBase.h"


class RouteTable
{
public:
	RouteTable(void);
	~RouteTable(void);

//    ULONG GetIFIndex( const char *ip);
//    BOOL GetTapAdapterInfo();
//    BOOL GetTapIpExist(CHAR *tap);
//    BOOL IsActiveTap();
    INT   GetIfIdByName(TCHAR *interface);
    BOOL  IsRouteExist(TCHAR *dest, TCHAR *mask, TCHAR *gateway);
    INT   AddRoute(TCHAR *dest, TCHAR *mask, TCHAR *gateway, INT metric, TCHAR *interface);
    INT   DelRoute(TCHAR *dest, TCHAR *mask, TCHAR *gateway);
    
};

#endif

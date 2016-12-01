#pragma once
#include "pdh.h"
#include <vector>
using namespace std;
#define  MAX_NUM       10
//计数器的全路径。<AddObject中使用>
//#define  PDH_STRPATH_NETWORK             "\\Network Interface(Realtek RTL8139_810x Family Fast Ethernet NIC)\\Bytes Received/sec"
#define  PDH_STRPATH_CPUCAP                 "\\Processor(_Total)\\% Processor Time" //CPU占用率
#define  PDH_STRPATH_RAS                         "\\RAS Total\\Bytes Transmitted"  
#define  PDH_STRPATH_CACHE                   "\\cache\\Dirty Pages"
#define  PDH_STRPATH_NBT                        "\\NBT Connection(Total)\\Bytes Total/sec"
#define  PDH_STRPATH_RASTRANSMIT     "\\RAS Total\\Bytes Transmitted/sec"
#define  PDH_STRPATH_RASRECEIVED        "\\RAS Total\\Bytes Received"

//计数器的局部路径。<GetNetworkSpeed中使用>
#define  PDH_STRDOWNWIDTH	                 ")\\Bytes Received/sec"
#define  PDH_STRTOTALWIDTH                   ")\\Bytes Total/sec"
#define  PDH_STRUPWIDTH                           ")\\Bytes Sent/sec"
#define  PDH_STRPACHETERROR                  ")\\Packets Received Errors"


struct PDHNODE
{
    HQUERY   hQuery;
	HCOUNTER  hCounter;
	double   df;
	TCHAR   str[MAX_PATH];
};
class CRopPDH : public vector<PDHNODE>
{
public:
	CRopPDH(void);
	~CRopPDH(void);
	BOOL PushObject(char * str);
	BOOL PopObject(char *str);
	BOOL UpdateInfo(char * str = NULL);
	double   GetDownSpeed();
	double   GetUpSpeed();
	double   GetCPUSpeed();
	
private:
	double GetLoopValue(char * str);
	int  GetDownPath(char *ch);
	int  GetUpPath(char *ch);

private:
	double  m_bandWidth;
    PDHNODE  m_NetworkInfo[MAX_NUM];
	PDHNODE   m_NetUpInfo[MAX_NUM];
	int           m_nNetCard;

};

#include "StdAfx.h"
#include "RopUtil.h"
#include "RopPDH.h"
#include <iostream>

using namespace  std;
#pragma  comment(lib,"pdh.lib")

CRopPDH::CRopPDH(void)
{
	this->clear();
	m_bandWidth = 0.0;
    m_nNetCard = 0;
	ZeroMemory(m_NetworkInfo,sizeof(m_NetworkInfo));
}

CRopPDH::~CRopPDH(void)
{
	if(!empty())
	{
		vector<PDHNODE>::iterator it = begin();
		for(it ; it != end(); ++it)
		{
			PdhCloseQuery(it->hQuery);
		}
	}
}

BOOL CRopPDH::PushObject(char * ch)
{

	TCHAR local[MAX_PATH]  = {0};
	size_t szcontext = 0;
	size_t  len = 260;
	//mbstowcs_s(&szcontext,local,len,ch,_TRUNCATE);
    mb2wc(ch, strlen(ch), local, (int*)&len);
	if(!empty())
	{
		vector<PDHNODE>::iterator it = begin();
		for( it ;it != end() ; ++it)
		{
			if(wcscmp(local,it->str) == 0)
			{
				Dbg("find this path!");
				return FALSE;
			}
		}
	}
	HQUERY          hQuery;
	PDH_STATUS      pdhStatus;
	HCOUNTER        hCounter;
	pdhStatus = PdhOpenQuery(0,0,&hQuery);

	if(pdhStatus  != ERROR_SUCCESS)
	{
		Dbg("open error");
		PdhCloseQuery(hQuery);
		return  FALSE;
	}

    char  temp[MAX_PATH] = {0};
	strcpy_s(temp,ch);
    TCHAR szFullPathBuffer[MAX_PATH] = {0};
	//size_t context;
	size_t szlen = sizeof(temp);
	//mbstowcs_s(&context,szFullPathBuffer,szlen,temp,_TRUNCATE);
    mb2wc(temp, strlen(temp), szFullPathBuffer, (int*)&szlen);
	//wcscpy(szFullPathBuffer,temp);
	DWORD dwSize = sizeof(szFullPathBuffer);  

	pdhStatus  =  PdhAddCounter(hQuery,szFullPathBuffer,0,&hCounter);

	if(pdhStatus != ERROR_SUCCESS)
	{
		Dbg("add counter error!");
		PdhCloseQuery(hQuery);
		return  FALSE ;
	}

	pdhStatus = PdhCollectQueryData(hQuery);
	if(pdhStatus != ERROR_SUCCESS)
	{
		Dbg("collect query data error!");
		PdhCloseQuery(hQuery);
		return  FALSE;
	}

	pdhStatus = PdhCollectQueryData(hQuery);
	if(pdhStatus != ERROR_SUCCESS)
	{
		Dbg("collect query data error!");
		PdhCloseQuery(hQuery);
		return  FALSE;
	}

	PDH_FMT_COUNTERVALUE pfc;  
	DWORD dwOpt;  
	pdhStatus = PdhGetFormattedCounterValue(hCounter,PDH_FMT_DOUBLE ,&dwOpt,&pfc);
	if(	pdhStatus != ERROR_SUCCESS)
	{
		Dbg("get value  error!");
		PdhCloseQuery(hQuery);
		return FALSE ;
	}

	PDHNODE  newNode;
	newNode.hCounter = hCounter;
	newNode.hQuery     = hQuery;
	wcscpy_s(newNode.str,szFullPathBuffer);
	newNode.df = pfc.doubleValue;
	push_back(newNode);

	return TRUE ;

}

BOOL CRopPDH::UpdateInfo(char *ch)
{
	Sleep(1000);
	PDH_FMT_COUNTERVALUE pfc;  
	DWORD dwOpt; 
	vector<PDHNODE>::iterator it = begin();
	if(ch == NULL)
	{
		for(it;it != end() ;  ++it )
		{
			int ret = PdhCollectQueryData(it->hQuery);
			if(ret  != ERROR_SUCCESS)
		 {
			 Dbg("update error! ");
			 return FALSE;
		 }

			ret = PdhGetFormattedCounterValue(it->hCounter,PDH_FMT_DOUBLE,&dwOpt,&pfc);
			if(ret != ERROR_SUCCESS)
		 {
			 Dbg("Update Get VALUE error! ");
			 return FALSE;
		 }
			it->df  = pfc.doubleValue;//修改df的值便于查找更新结果。
		}
	}
	else 
	{
		TCHAR max[MAX_PATH];
		size_t len = 260;
		//mbstowcs_s(&context,max,len,ch,_TRUNCATE);
        mb2wc(ch, strlen(ch), max, (int*)&len);
		for( it = begin() ; it != end(); ++it)
		{
			if(wcscmp(it->str,max) == 0)
			{
				int ret = PdhCollectQueryData(it->hQuery);
				if(ret != ERROR_SUCCESS)
				{
					Dbg("collect error");
					return FALSE;
				}
				ret = PdhGetFormattedCounterValue(it->hCounter,PDH_FMT_DOUBLE,&dwOpt,&pfc);
				if(ret != ERROR_SUCCESS)
				{
					Dbg("get value error! ");
					return FALSE;
				}
				it->df = pfc.doubleValue;
				m_bandWidth = it->df;
				return TRUE;
			}
		}
		return FALSE;
	}
	return  TRUE;
}

BOOL CRopPDH::PopObject(char *str)
{
	TCHAR  temp[MAX_PATH];
	char hot[MAX_PATH];
	strcpy_s(hot,str);
	size_t len = sizeof(hot);
	//mbstowcs_s(&context,temp,len,hot,_TRUNCATE);
    mb2wc(hot, strlen(hot), temp, (int*)&len);
	vector<PDHNODE>::iterator it = begin() ;
	for(it;it != end() ; ++it)
	{
		if(wcscmp(temp,it->str) == 0)
		{
			erase(it);
			return  TRUE ;
		}
	}
	return  FALSE;
}
double   CRopPDH::GetDownSpeed()
{

	PDH_STATUS      pdhStatus;
	int i = 0;
	static int ret = 0;
	if(ret == 0)
	GetDownPath(PDH_STRDOWNWIDTH);
	ret ++;
    double total = 0;

	for(;i<m_nNetCard;++i)
	{
		pdhStatus = PdhCollectQueryData(m_NetworkInfo[i].hQuery);
		if(pdhStatus != ERROR_SUCCESS)
		{
			Dbg("collect error!");
			return 0;
		}
	//	Sleep(500);
		DWORD dwOpt;
		PDH_FMT_COUNTERVALUE pfc;  
		pdhStatus = PdhGetFormattedCounterValue(m_NetworkInfo[i].hCounter,PDH_FMT_DOUBLE,&dwOpt,&pfc);
		if(pdhStatus != ERROR_SUCCESS)
		{
			Dbg("get value error!");
			return 0;
		}
		total += pfc.doubleValue;
		
	}

    return  total/1024;
}


double   CRopPDH::GetCPUSpeed()
{
	static int i = 0;
	i ++;
	if( i ==1)
	{
		PushObject(PDH_STRPATH_CPUCAP);
	}
	UpdateInfo(PDH_STRPATH_CPUCAP);
	return m_bandWidth;
}
int  CRopPDH::GetDownPath(char *ch)
{
	PDH_STATUS status = ERROR_SUCCESS;
	LPWSTR pwsCounterListBuffer = NULL;
	DWORD dwCounterListSize = 0;
	LPWSTR pwsInstanceListBuffer = NULL;
	DWORD dwInstanceListSize = 0;
	LPWSTR pTemp = NULL;
	char cTotalPath[MAX_PATH] = {0};
	char cLocPath[MAX_PATH] =  {0};
	double retDou = 0.0;

	status = PdhEnumObjectItems(
		NULL,
		NULL,
		L"Network Interface",
		pwsCounterListBuffer,   // pass NULL and 0
		&dwCounterListSize,     // to get required buffer size
		pwsInstanceListBuffer, 
		&dwInstanceListSize, 
		PERF_DETAIL_WIZARD,     // counter detail level
		0); 
	if(status != ERROR_SUCCESS)
	{
		pwsCounterListBuffer = (LPWSTR)malloc(dwCounterListSize * sizeof(WCHAR));
		pwsInstanceListBuffer = (LPWSTR)malloc(dwInstanceListSize * sizeof(WCHAR));
		if((pwsInstanceListBuffer != NULL) && (pwsCounterListBuffer != NULL))
		{
			status = PdhEnumObjectItems(
				NULL,
				NULL,
				L"Network Interface",
				pwsCounterListBuffer,
				&dwCounterListSize,
				pwsInstanceListBuffer,
				&dwInstanceListSize,
				PERF_DETAIL_WIZARD,
				0);

			if(status == ERROR_SUCCESS)
			{
				int  nNet = 0;
				size_t  len;
				for (pTemp = pwsInstanceListBuffer; *pTemp != 0; pTemp += wcslen(pTemp) + 1) 
				{
					//wcstombs_s(&context,cLocPath,len,pTemp,_TRUNCATE);
                    len = MAX_PATH;
                    wc2mb(pTemp, wcslen(pTemp), cLocPath, (int*)&len);
					strcpy_s(cTotalPath , "\\Network Interface(");
					strcat_s(cTotalPath , cLocPath);
					strcat_s(cTotalPath ,  ch);
					//len = sizeof(cTotalPath);
					//mbstowcs_s(&context,m_NetworkInfo[nNet].str,len,cTotalPath,_TRUNCATE);
                    len = MAX_PATH;
                    mb2wc(cTotalPath, strlen(cTotalPath), m_NetworkInfo[nNet].str, (int*)&len);

					HQUERY          hQuery;
					PDH_STATUS      pdhStatus;
					HCOUNTER        hCounter;

					pdhStatus = PdhOpenQuery(0,0,&hQuery);
					if(pdhStatus != ERROR_SUCCESS)
					{
						Dbg("open error ! ");
						return -1;
					}

					pdhStatus = PdhAddCounter(hQuery,m_NetworkInfo[nNet].str,0,&hCounter);
					if(pdhStatus != ERROR_SUCCESS)
					{
						Dbg("add error");
						PdhCloseQuery(hQuery);
						return -1;
					}

					pdhStatus = PdhCollectQueryData(hQuery);
					if(pdhStatus != ERROR_SUCCESS)
					{
						Dbg("query error! ");
						PdhCloseQuery(hQuery);
						return -1;
					}

					pdhStatus = PdhCollectQueryData(hQuery);
					if(pdhStatus != ERROR_SUCCESS)
					{
						Dbg("query error! ");
						PdhCloseQuery(hQuery);
						return -1;
					}

					PDH_FMT_COUNTERVALUE pfc;  
					DWORD dwOpt; 
					pdhStatus = PdhGetFormattedCounterValue(hCounter,PDH_FMT_DOUBLE,&dwOpt,&pfc);
					if(pdhStatus != ERROR_SUCCESS)
					{
						Dbg("get valuse error ");
						PdhCloseQuery(hQuery);
						return -1;
					}  
					m_NetworkInfo[nNet].hCounter = hCounter;
					m_NetworkInfo[nNet].hQuery = hQuery;
					nNet ++;
					m_nNetCard = nNet;
				}
			}
		}
		else
		{
			Dbg("memory error!");
			return -1;
		}
	}
	return 0;
}

double   CRopPDH::GetUpSpeed()
{
	PDH_STATUS      pdhStatus;
	int i = 0;
	static int ret = 0;
	if(ret == 0)
		GetUpPath(PDH_STRUPWIDTH);
	ret ++;
	double total = 0;

	for(;i<m_nNetCard;++i)
	{
		pdhStatus = PdhCollectQueryData(m_NetUpInfo[i].hQuery);
		if(pdhStatus != ERROR_SUCCESS)
		{
			Dbg("collect error!");
			return 0;
		}
		//	Sleep(500);
		DWORD dwOpt;
		PDH_FMT_COUNTERVALUE pfc;  
		pdhStatus = PdhGetFormattedCounterValue(m_NetUpInfo[i].hCounter,PDH_FMT_DOUBLE,&dwOpt,&pfc);
		if(pdhStatus != ERROR_SUCCESS)
		{
			Dbg("get value error!");
			return 0;
		}
		total += pfc.doubleValue;

	}

	return  total/1024;

}

int  CRopPDH::GetUpPath(char *ch)
{

	PDH_STATUS status = ERROR_SUCCESS;
	LPWSTR pwsCounterListBuffer = NULL;
	DWORD dwCounterListSize = 0;
	LPWSTR pwsInstanceListBuffer = NULL;
	DWORD dwInstanceListSize = 0;
	LPWSTR pTemp = NULL;
	char cTotalPath[MAX_PATH] = {0};
	char cLocPath[MAX_PATH] =  {0};
	double retDou = 0.0;

	status = PdhEnumObjectItems(
		NULL,
		NULL,
		L"Network Interface",
		pwsCounterListBuffer,   // pass NULL and 0
		&dwCounterListSize,     // to get required buffer size
		pwsInstanceListBuffer, 
		&dwInstanceListSize, 
		PERF_DETAIL_WIZARD,     // counter detail level
		0); 
	if(status != ERROR_SUCCESS)
	{
		pwsCounterListBuffer = (LPWSTR)malloc(dwCounterListSize * sizeof(WCHAR));
		pwsInstanceListBuffer = (LPWSTR)malloc(dwInstanceListSize * sizeof(WCHAR));
		if((pwsInstanceListBuffer != NULL) && (pwsCounterListBuffer != NULL))
		{
			status = PdhEnumObjectItems(
				NULL,
				NULL,
				L"Network Interface",
				pwsCounterListBuffer,
				&dwCounterListSize,
				pwsInstanceListBuffer,
				&dwInstanceListSize,
				PERF_DETAIL_WIZARD,
				0);

			if(status == ERROR_SUCCESS)
			{
				int  nNet = 0;
				size_t  len;
				for (pTemp = pwsInstanceListBuffer; *pTemp != 0; pTemp += wcslen(pTemp) + 1) 
				{
					//len = wcslen(pTemp) + 1;
					//wcstombs_s(&context,cLocPath,len,pTemp,_TRUNCATE);
                    len = MAX_PATH;
                    wc2mb(pTemp, wcslen(pTemp), cLocPath, (int*)&len);
					strcpy_s(cTotalPath , "\\Network Interface(");
					strcat_s(cTotalPath , cLocPath);
					strcat_s(cTotalPath ,  ch);
					//len = sizeof(cTotalPath);
					//mbstowcs_s(&context,m_NetUpInfo[nNet].str,len,cTotalPath,_TRUNCATE);
                    len = MAX_PATH;
                    mb2wc(cTotalPath, strlen(cTotalPath), m_NetUpInfo[nNet].str, (int*)&len);

					HQUERY          hQuery;
					PDH_STATUS      pdhStatus;
					HCOUNTER        hCounter;

					pdhStatus = PdhOpenQuery(0,0,&hQuery);
					if(pdhStatus != ERROR_SUCCESS)
					{
						Dbg("open error ! ");
						return -1;
					}

					pdhStatus = PdhAddCounter(hQuery,m_NetUpInfo[nNet].str,0,&hCounter);
					if(pdhStatus != ERROR_SUCCESS)
					{
						Dbg("add error");
						PdhCloseQuery(hQuery);
						return -1;
					}

					pdhStatus = PdhCollectQueryData(hQuery);
					if(pdhStatus != ERROR_SUCCESS)
					{
						Dbg("query error! ");
						PdhCloseQuery(hQuery);
						return -1;
					}

					pdhStatus = PdhCollectQueryData(hQuery);
					if(pdhStatus != ERROR_SUCCESS)
					{
						Dbg("query error! ");
						PdhCloseQuery(hQuery);
						return -1;
					}

					PDH_FMT_COUNTERVALUE pfc;  
					DWORD dwOpt; 
					pdhStatus = PdhGetFormattedCounterValue(hCounter,PDH_FMT_DOUBLE,&dwOpt,&pfc);
					if(pdhStatus != ERROR_SUCCESS)
					{
						Dbg("get valuse error ");
						PdhCloseQuery(hQuery);
						return -1;
					}  
					m_NetUpInfo[nNet].hCounter = hCounter;
					m_NetUpInfo[nNet].hQuery = hQuery;
					nNet ++;
					m_nNetCard = nNet;
				}
			}
		}
		else
		{
			Dbg("memory error!");
			return -1;
		}
	}
	return 0;
}
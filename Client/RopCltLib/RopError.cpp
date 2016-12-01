#include "stdafx.h"
#include "RopError.h"
#include <vector>
#include <string.h>
#include <map>
using namespace std;

void GetSystemVersion(char* version)
{
	OSVERSIONINFOEX    info; 
	ZeroMemory(&info,sizeof(OSVERSIONINFOEX));
	info.dwOSVersionInfoSize   =   sizeof(info); 
	GetVersionEx((OSVERSIONINFO *)&info); 
	SYSTEM_INFO sysinfo;
	ZeroMemory(&sysinfo,sizeof(SYSTEM_INFO));
	GetSystemInfo(&sysinfo);
	if(VER_PLATFORM_WIN32_NT!=info.dwPlatformId)
	{
		strcpy_s(version,MAX_NAME_LEN,"WINDOWS_UNKNOWN_VERSION");
		return;
	}
	if   (   info.dwMajorVersion   ==   5   ) 
	{ 
		if   (   info.dwMinorVersion   ==   0   ) 
		{
			strcpy_s(version,MAX_NAME_LEN,"WINDOWS_2000");
			return;
		}
		else   if   ( info.dwMinorVersion == 1) 
		{
			strcpy_s(version,MAX_NAME_LEN,"WINDOWS_XP");
			return; 
		}
		else   if (info.dwMinorVersion == 2) 
		{
			if(sysinfo.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
			{ 
				strcpy_s(version,MAX_NAME_LEN,"WINDOWS_SERVER_2003_X64");
				return;
			}
			else
			{
				strcpy_s(version,MAX_NAME_LEN,"WINDOWS_SERVER_2003");
				return;
			}
		}
		else 
		{
			strcpy_s(version,MAX_NAME_LEN,"WINDOWS_UNKNOWN_VERSION");
			return; 
		}
	}
	else if( info.dwMajorVersion   ==   6)
	{
		if(info.dwMinorVersion==0)
		{
			if(sysinfo.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
			{ 
				strcpy_s(version,MAX_NAME_LEN,"WINDOWS_VISTA_X64");
				return;
			}
			strcpy_s(version,MAX_NAME_LEN,"WINDOWS_VISTA");
			return;
		}
		else if(info.dwMinorVersion==1)
		{
			if(sysinfo.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
			{ 
				strcpy_s(version,MAX_NAME_LEN,"WINDOWS_7_X64");
				return;
			}
			strcpy_s(version,MAX_NAME_LEN,"WINDOWS_7");
			return;
		}  
	}
	else
	{
		strcpy_s(version,MAX_NAME_LEN,"WINDOWS_UNKNOWN_VERSION");
		return;
	}
}



RopError::RopError()
{
	//init variable
	strcpy_s(m_user_name,MAX_NAME_LEN,"UNKONW USERNAME");
	strcpy_s(m_client_version,MAX_NAME_LEN,DLL_VERSION);
	GetSystemVersion(m_os_version);
	m_net_mode = 0;
	memset(m_safe_soft,0,MAX_NAME_LEN);
	memset(m_resave,0,MAX_NAME_LEN);
	memset(m_time,0,MAX_NAME_LEN);
	memset(m_Lbs_IP,0,MAX_NAME_LEN);
	memset(m_wErrFilePath,0,MAX_NAME_LEN);
	m_error_cnt = 0;
	m_error_id = 0;
	m_repeat = 0;
	m_port = 10019;  //err Server Port

	WCHAR wDirPath[MAX_PATH] = {0};
	WCHAR wDirPathFile[MAX_PATH] = {0};
	char szPathAnsci[MAX_PATH] = {0};
	char drive[_MAX_DRIVE] = {0};
	char dir[_MAX_DIR] = {0};
	char Path[MAX_PATH] = {0};
	size_t Len = MAX_PATH;

	if (GetModuleFileName(NULL,wDirPathFile,MAX_PATH) == 0)
	{
		LDebug(DEBUG_L_ERROR,"RopError ictor GetModuleFileName  Failed!");		
	}
	wc2mb(wDirPathFile,wcslen(wDirPathFile),szPathAnsci,(int*)&Len);
	_splitpath(szPathAnsci, drive, dir, NULL,NULL);
	sprintf(Path,"%s%s",drive,dir);
	mb2wc(Path,strlen(Path),wDirPath,(int*)&Len);

	wsprintf(m_wErrFilePath,TEXT("%s\\%s"),wDirPath,ErrorFile);
	
}

void Trim(char* pStr,int len)
{
	string tmpStr = pStr;
	if (tmpStr.empty()){
		return;
	}
	tmpStr.erase(0, tmpStr.find_first_not_of(' '));       //prefixing spaces
	tmpStr.erase(tmpStr.find_last_not_of(' ')+1);         //surfixing spaces
	
	tmpStr.erase(0, tmpStr.find_first_not_of("\r\n"));       //prefixing spaces
	tmpStr.erase(tmpStr.find_last_not_of("\r\n")+1);         //surfixing spaces
	
	strcpy_s(pStr,len,tmpStr.c_str());
}




void SpiltStr(const char* srcStr,const char* token,vector<string>& vec)
{
	size_t len = strlen(srcStr);
	if (len == 0){
		return ;
	}
	char* pbuf = (char*)malloc(len+1);
	char* tmp = NULL;
	strcpy_s(pbuf,len+1,srcStr);
	Trim(pbuf,len+1);
	char* pstr = strtok_s(pbuf,token,&tmp);
	while (pstr != NULL){
		Trim(pstr,strlen(pstr)+1);
		vec.push_back(pstr);
		pstr = strtok_s(NULL,token,&tmp);
	}
	free(pbuf);

}

int RopError::GetClientInfo(HANDLE fileHandle,pclient_error& pErr)
{
	try{
		char buf[1024*10] = {0};
		DWORD dwReaded = 0;
		if(!ReadFile(fileHandle,buf,1024*10,&dwReaded,NULL)){
			return -1;
		}
	
		Trim(buf,1024*10);

		vector<string> vecRows;
		char* pRowStoken = ".";
		//LDebug(DEBUG_L_ERROR,"buf = %s",buf);
		SpiltStr(buf,pRowStoken,vecRows);
		if (vecRows.size() == 0){
			LDebug(DEBUG_L_ERROR,"ErrFile have nothing Infomation!\n");
			return -1;
		}else if (vecRows.size()>10){
			vecRows.erase(vecRows.begin(),vecRows.begin()+vecRows.size()-10);
		}

		size_t count = vecRows.size();

		char* pStoken = ",";

		map<int,int> numMap;
		map<int,string> NumAndTimeMap;
		int itemNum = 0;
		vector<string> vecTime;
		for (int i = count-1; i >= 0; i--){
			vector<string> vecInfo;
			SpiltStr(vecRows[i].c_str(),pStoken,vecInfo);
			if (vecInfo.size()<6){
				continue;
			}
			if (vecInfo.size() == 7)
			{
				strcpy_s(pErr->safe_soft,MAX_NAME_LEN,vecInfo[6].c_str());
			}
			int ErrNum = atoi(vecInfo[5].c_str());
			if (!numMap.count(ErrNum)){
				numMap[ErrNum] = 1;
				NumAndTimeMap[ErrNum] = vecInfo[4];
				itemNum++;
			}else{
				++numMap[ErrNum];
			}
			if (i == count-1){
				strcpy_s(pErr->user_name,MAX_NAME_LEN,vecInfo[0].c_str());
				strcpy_s(pErr->client_version,MAX_NAME_LEN,vecInfo[1].c_str());
				strcpy_s(pErr->os_version,MAX_NAME_LEN,vecInfo[2].c_str());
				pErr->net_mode = atoi(vecInfo[3].c_str());
			}
		}

		pErr->error_cnt = numMap.size();
		map<int,int>::const_iterator map_it = numMap.begin();
		itemNum = 0;
		while(map_it != numMap.end()){
			pErr->error_item[itemNum].error_id = map_it->first;
			pErr->error_item[itemNum].repeat = map_it->second;
			strcpy_s(pErr->error_item[itemNum].time,MAX_NAME_LEN,NumAndTimeMap[map_it->first].c_str());
			itemNum++;
			++map_it;
		}
		
	}catch(...){
		LDebug(DEBUG_L_ERROR,"Read ErrFile Failed!");
		//clearup file
		SetFilePointer(fileHandle,0,NULL,FILE_BEGIN);
		SetEndOfFile(fileHandle);
		return -1;
	}
	return 0;
}

RopError::~RopError()
{

}

int RopError::SetUserName( CHAR* pUserName )
{
	if (strlen(pUserName) >= 32 ){
		LDebug(DEBUG_L_ERROR,"UserName >= 32 is too long!\n");
		return -1;
	}
	strcpy_s(m_user_name,MAX_NAME_LEN,pUserName);
	return 0;
}

int RopError::SetInfo(CHAR* pInfo)
{
	if (strlen(pInfo) >= 32 ){
		LDebug(DEBUG_L_ERROR,"pInfo >= 32 is too long!\n");
		return -1;
	}
	strcpy_s(m_info,MAX_NAME_LEN,pInfo);
	return 0;
}

int RopError::SendErr()
{
	HANDLE fileHandle = CreateFile(m_wErrFilePath,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if (fileHandle == INVALID_HANDLE_VALUE){
		LDebug(DEBUG_L_ERROR,"SendErr Error => File Handle is invalid!\n");
		return -1;
	}

	char buf[1024] =  {0};
	pmsg_head_t pmsg = (pmsg_head_t)buf;
	pmsg->msg = MSG_ERR_CS_CONTENT;
	pmsg->len = sizeof(client_error);
	pclient_error pErrorInfo = (pclient_error)pmsg->payload;	
	if(GetClientInfo(fileHandle,pErrorInfo)){
		CloseHandle(fileHandle);
		return 0;
	}
	DWORD len = sizeof(msg_head_t) + pmsg->len;

	/* Get LBS Address */
	//RopCfgFile CfgFile(".\\", CFG_NAME, NULL);
	//if (FALSE == CfgFile.OpenFile()) {
	//	LDebug(DEBUG_L_ERROR,"Can't open configure file, can't get LBS IP-Port, init failed!!!\n");
	//	return -1;
	//}
	//if (CfgFile.ReadAndInit()) {
	//	LDebug(DEBUG_L_ERROR,"ReadAndInit configure file got error, init failed!!!\n");
	//	return -1;
	//}    

	//CfgFile.GetServerAddr(m_Lbs_IP, ipLen);
	
	int ipLen = 32;	
	RopSyncSocket errSocket;
	WCHAR LBS_IP[MAX_NAME_LEN] = {0};

	mb2wc(m_Lbs_IP,MAX_NAME_LEN,LBS_IP,&ipLen);

	if(errSocket.Open(LBS_IP,m_port)){
		LDebug(DEBUG_L_ERROR,"RopError Can not Connect ErrServer!\n");
		CloseHandle(fileHandle);
		return 0;
	}
	
	if(!errSocket.Send(buf,len)){
		//clearup file
		SetFilePointer(fileHandle,0,NULL,FILE_BEGIN);
		SetEndOfFile(fileHandle);
		DWORD dwWritten = 0;
		char tmpBuf[32] = {0};
		WriteFile(fileHandle, tmpBuf, strlen(tmpBuf), &dwWritten, NULL );
	}
	
	LDebug(DEBUG_L_ERROR,"Send ErrorInfo OK!\n");
	errSocket.ShutdownSilent();
	CloseHandle(fileHandle);
	return 0;
}

int RopError::WriteInfo(SHORT ErrNum)
{
	LDebug(DEBUG_L_ERROR,"RopError Write LastErr=%d\n",ErrNum);
	HANDLE fileHandle = CreateFile(m_wErrFilePath,GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,0);
	if (fileHandle == INVALID_HANDLE_VALUE){
		LDebug(DEBUG_L_ERROR,"Open recorde ErrFile Failed!\n");
		CloseHandle(fileHandle);
		return -1;
	}

	SYSTEMTIME tm;
	GetLocalTime(&tm);
	sprintf_s(m_time,32,"%d-%d-%d %d:%d:%d",tm.wYear,tm.wMonth,tm.wDay,tm.wHour,tm.wMinute,tm.wSecond);
	m_error_id = ErrNum;

	char buf[1024] = {0};
	strcpy_s(m_safe_soft,MAX_NAME_LEN,m_info);
	sprintf_s(buf,1024,"%s,%s,%s,%d,%s,%d,%s,%s.\n",m_user_name,m_client_version,m_os_version,m_net_mode,
		m_time,m_error_id,m_safe_soft,m_resave);
	DWORD dwWritten = 0;

	SetEndOfFile(fileHandle);
	if(SetFilePointer(fileHandle,0,NULL,FILE_END) >= 10240){
		SetFilePointer (fileHandle, 0, NULL, FILE_BEGIN);
		SetEndOfFile(fileHandle);
	}
	if(!WriteFile(fileHandle,buf,strlen(buf),&dwWritten,NULL)){
		LDebug(DEBUG_L_ERROR,"RopError Write Failed!LastError = %d \n",GetLastError());
	}else{
		//LDebug(DEBUG_L_ERROR,"RopError Write the Error info OK!\n");
	}

	CloseHandle(fileHandle);
	memset(m_info,0,MAX_NAME_LEN);
	return 0;
}

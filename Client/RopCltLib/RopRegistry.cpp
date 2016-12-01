#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include "RopRegistry.h"

RopRegistry::RopRegistry()
{
    m_hKey=NULL;
}

RopRegistry::~RopRegistry()
{
    Close();
}
//判定指定的键值是否存在。
BOOL RopRegistry::IsKeyExist(HKEY hKeyRoot, LPCTSTR pszPath)
{
    BOOL bRet = FALSE;
    HKEY hKey;
    LONG lRet = RegOpenKeyEx(hKeyRoot, pszPath, 0, KEY_READ, &hKey);
    if (ERROR_SUCCESS == lRet) {
        RegCloseKey(hKey);
        bRet = TRUE;
    }
    return bRet;
}



BOOL RopRegistry::IsKeyExistEx(HKEY hKeyRoot, LPCTSTR pszPath,LPCTSTR Key)
{
	BOOL bRet = FALSE;
	HKEY hKey;
	LONG lRet = RegOpenKeyEx(hKeyRoot, pszPath, 0, KEY_READ, &hKey);
	if (ERROR_SUCCESS == lRet) {
		LPBYTE owner_Get=new BYTE[80];
		DWORD type_1=REG_SZ;
		DWORD cbData_1=80;
		LONG ret1=::RegQueryValueEx(hKey,Key,NULL,&type_1,owner_Get,&cbData_1);
		if(ret1 ==ERROR_SUCCESS){
			bRet = TRUE;
		}
	}
	RegCloseKey(hKey);
	return bRet;
}


//打开键值。
LONG RopRegistry::Open(HKEY hKeyRoot, LPCTSTR pszPath)
{
    DWORD dw;
    LONG  lRet;

    lRet = RegCreateKeyEx(hKeyRoot, pszPath, 0L, NULL,
                          REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS|KEY_CREATE_SUB_KEY,NULL,
                          &m_hKey,&dw);
    return lRet;
}
//在注册表中删除指定的键值。
LONG RopRegistry::Delete(HKEY hKeyRoot, LPCTSTR pszPath)
{
    return RegDeleteKey(hKeyRoot, pszPath);
}
//关闭注册表
void RopRegistry::Close()
{
    if (m_hKey) {
        RegCloseKey(m_hKey);
        m_hKey = NULL;
    }
}
//写入注册表的键值。（以不同的方式。）
LONG RopRegistry::Write(LPCTSTR pszKey, DWORD dwVal)
{
    return RegSetValueEx(m_hKey, pszKey, 0L, REG_DWORD,
                         (CONST BYTE*)&dwVal, sizeof(DWORD));
}

LONG RopRegistry::Write(LPCTSTR pszKey, LPCTSTR pszData)
{
    return RegSetValueEx(m_hKey, pszKey, 0L, REG_SZ,
                         (CONST BYTE*)pszData, sizeof(TCHAR)*( _tcslen(pszData)+1));
}

LONG RopRegistry::WriteMultiSz(LPCTSTR pszKey, LPCTSTR pszData)
{
    TCHAR buf[1024];
    int len = MakeMultiSz(buf,pszData);
    if (len>0) {
        return RegSetValueEx(m_hKey, pszKey, 0L, REG_MULTI_SZ ,
                             (unsigned char*)buf, len);
    }
    return 0;
}


LONG RopRegistry::Write(LPCTSTR pszKey, const BYTE* pData, DWORD dwLength)
{
    return RegSetValueEx(m_hKey, pszKey, 0L, REG_BINARY,
                         pData, dwLength);
}
//读注册表中的内容。
LONG RopRegistry::Read(LPCTSTR pszKey, DWORD& dwVal)
{
    DWORD dwType;
    DWORD dwSize=sizeof(DWORD);
    DWORD dwDest;

    LONG lRet = RegQueryValueEx(m_hKey, pszKey, NULL,
                                &dwType, (BYTE*)&dwDest, &dwSize);

    if (lRet==ERROR_SUCCESS) {
        dwVal = dwDest;
        return lRet;
    }

    /* create value first, and return -1 for value */
    if (lRet == ERROR_FILE_NOT_FOUND) {
        dwType = 0;
        dwSize = sizeof(DWORD);
        lRet = RegSetValueEx(m_hKey, pszKey, 0L, REG_DWORD, (BYTE*)&dwType, dwSize);
        dwDest = 0;  
    }
    return lRet;
}

LONG RopRegistry::Read(LPCTSTR pszKey, const TCHAR *sVal)
{
    try {
    DWORD dwType;
    DWORD dwSize = MAX_REG_LEN + 1;

    LONG lRet = RegQueryValueEx(m_hKey, pszKey, NULL,
                                &dwType, (BYTE*)sVal, &dwSize);

    /* create value first, and return -1 for value */
    if (lRet == ERROR_FILE_NOT_FOUND) {
        dwType = 0;
        dwSize = MAX_REG_LEN + 1;
        lRet = RegSetValueEx(m_hKey, pszKey, 0L, REG_SZ, (BYTE*)&dwType, dwSize);
        *(char *)sVal = 0;  
    }

    return lRet;
    }
    catch(...){

    }
}

LONG RopRegistry::Read(LPCTSTR pszKey, BYTE *sVal, DWORD &dwLen)
{
    DWORD dwType;
    DWORD dwSize = dwLen;

    LONG lRet = RegQueryValueEx(m_hKey, pszKey, NULL, &dwType, sVal, &dwSize);

    if (lRet == ERROR_SUCCESS) {
        dwLen = dwSize;
        return lRet;
    }

    /* create value first, and return -1 for value */
    if (lRet == ERROR_FILE_NOT_FOUND) {
        dwType = 0;
        dwSize = dwLen;
        lRet = RegSetValueEx(m_hKey, pszKey, 0L, REG_SZ, (BYTE*)&dwType, dwSize);
        *sVal = 0; 
        dwLen = 0;
    }
    return lRet;
}

LONG RopRegistry::ReadMultiSz(LPCTSTR pszKey, const char *sVal,DWORD& dwLen)
{
    DWORD dwType;
    DWORD dwSize=200;

    LONG lRet = RegQueryValueEx(m_hKey, pszKey, NULL,
                                &dwType, (BYTE*)sVal, &dwSize);

    if (lRet==ERROR_SUCCESS) {
        dwLen = dwSize;
        return lRet;
    }

    /* create value first, and return -1 for value */
    if (lRet == ERROR_FILE_NOT_FOUND) {
        dwType = 0;
        dwSize = 200;
        lRet = RegSetValueEx(m_hKey, pszKey, 0L, REG_MULTI_SZ, (BYTE*)&dwType, dwSize);
        *(char *)sVal = 0; 
        dwLen = 0;
    }

    return lRet;
}




int RopRegistry::MakeMultiSz(LPTSTR lpBuf, LPCTSTR lpData)
{
    LPTSTR lpB;
    LPCTSTR lpD;
    lpB = lpBuf;
    lpD = lpData;
    while (*lpD  != _T('\0')) {
        if (*lpD == _T('\r')) {
            *lpB++ = _T('\0');
        } else
            if (!_istcntrl(*lpD)) {
                *lpB++ = *lpD;
            }
        lpD++;
    }
    *lpB++ = _T('\0');
    *lpB++ = _T('\0');
    return ((LPBYTE)lpB - (LPBYTE)lpBuf);
}
//设置注册表键值的安全属性。
int RopRegistry::SetKeySecurity(HKEY hRootKey, LPCTSTR lpszSubKey)
{
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    PSID pInteractiveSid = NULL;
    PSID pAdministratorsSid = NULL;
    SECURITY_DESCRIPTOR sd;
    PACL pDacl = NULL;
    DWORD dwAclSize;
    HKEY hKey;
    LONG lRetCode;
    BOOL bSuccess = FALSE; // assume this function fails
// open the performance key for WRITE_DAC access
    lRetCode = RegOpenKeyEx( hRootKey, lpszSubKey, 0, WRITE_DAC, &hKey );

    if (lRetCode != ERROR_SUCCESS) {
        return RTN_ERROR;
    }

// prepare a Sid representing any Interactively logged-on user
    if (!AllocateAndInitializeSid(&sia,1, SECURITY_INTERACTIVE_RID,
                                  0, 0, 0, 0, 0, 0, 0, &pInteractiveSid )) {
        goto cleanup;
    }

// preprate a Sid representing the well-known admin group
    if (!AllocateAndInitializeSid( &sia, 2, SECURITY_BUILTIN_DOMAIN_RID,
                                   DOMAIN_ALIAS_RID_ADMINS,0, 0, 0, 0, 0, 0, &pAdministratorsSid)) {
        goto cleanup;
    }

// compute size of new acl
    dwAclSize = sizeof(ACL) +
                2 * ( sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) ) +
                GetLengthSid(pInteractiveSid) +
                GetLengthSid(pAdministratorsSid) ;

// allocate storage for Acl
    pDacl = (PACL)HeapAlloc(GetProcessHeap(), 0, dwAclSize);
    if (pDacl == NULL) goto cleanup;

    if (!InitializeAcl(pDacl, dwAclSize, ACL_REVISION)) {
        goto cleanup;
    }

// grant the Interactive Sid KEY_READ access to the perf key
    if (!AddAccessAllowedAce( pDacl, ACL_REVISION, KEY_ALL_ACCESS, pInteractiveSid )) {
        goto cleanup;
    }

// grant the Administrators Sid KEY_ALL_ACCESS access to the perf key
    if (!AddAccessAllowedAce( pDacl, ACL_REVISION, KEY_ALL_ACCESS, pAdministratorsSid )) {
        goto cleanup;
    }

    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        goto cleanup;
    }

    if (!SetSecurityDescriptorDacl(&sd, TRUE, pDacl, FALSE)) {
        goto cleanup;
    }

// apply the security descriptor to the registry key
    lRetCode = RegSetKeySecurity(hKey, (SECURITY_INFORMATION)DACL_SECURITY_INFORMATION, &sd);

    if (lRetCode != ERROR_SUCCESS) {
        goto cleanup;
    }

    bSuccess = TRUE; // indicate success

cleanup:

    RegCloseKey(hKey);
    RegCloseKey(HKEY_LOCAL_MACHINE);

// free allocated resources
    if (pDacl != NULL)
        HeapFree(GetProcessHeap(), 0, pDacl);

    if (pInteractiveSid != NULL)
        FreeSid(pInteractiveSid);

    if (pAdministratorsSid != NULL)
        FreeSid(pAdministratorsSid);

    if (bSuccess) {
//       printf("SUCCESS updating performance data security\n");
        return RTN_OK;
    } else {
//       printf("ERROR updating performance data security\n");
        return RTN_ERROR;
    }
}


LONG RopRegistry::EnumKey(DWORD dwIndex, LPTSTR lpName)
{
    return RegEnumKey(m_hKey,dwIndex,lpName,MAX_PATH + 1);

}

LONG RopRegistry::EnumValue(DWORD dwIndex, LPTSTR lpName,DWORD* NameLen, LPTSTR lpValue, DWORD* ValueLen)
{
    DWORD type;

    return  RegEnumValue(m_hKey,              // handle to key to query
                         dwIndex,          // index of value to query
                         lpName,     // address of buffer for value string
                         NameLen,  // address for size of value buffer
                         NULL,     // reserved
                         &type,         // address of buffer for type code
                         (unsigned char*)lpValue,          // address of buffer for value data
                         ValueLen        // address for size of data buffer
                         );
}

DWORD RopRegistry::DeleteKeyNT(HKEY hStartKey, LPTSTR pKeyName)
{
    DWORD   dwRtn, dwSubKeyLength;
    LPTSTR  pSubKey = NULL;
    TCHAR   szSubKey[256]; // (256) this should be dynamic.
    HKEY    hKey;

    if ( pKeyName &&  lstrlen(pKeyName)) {
        RopRegistry temp;
        temp.SetKeySecurity(hStartKey,pKeyName);
        if ( (dwRtn=RegOpenKeyEx(hStartKey,pKeyName,
                                 0, KEY_ENUMERATE_SUB_KEYS | DELETE, &hKey )) == ERROR_SUCCESS) {
            while (dwRtn == ERROR_SUCCESS ) {
                dwSubKeyLength = 256;
                dwRtn=RegEnumKeyEx(
                                  hKey,
                                  0,       // always index zero
                                  szSubKey,
                                  &dwSubKeyLength,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL);

                if (dwRtn == ERROR_NO_MORE_ITEMS) {
                    RopRegistry temp;
                    temp.SetKeySecurity(hStartKey,pKeyName);
                    dwRtn = RegDeleteKey(hStartKey, pKeyName);
                    break;
                } else if (dwRtn == ERROR_SUCCESS) {
                    dwRtn=DeleteKeyNT(hKey, szSubKey);
                }
            }
            RegCloseKey(hKey);
            // Do not save return code because error
            // has already occurred
        }
    } else
        dwRtn = ERROR_BADKEY;

    return dwRtn;

}

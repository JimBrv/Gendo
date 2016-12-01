#pragma once

#include "stdafx.h"
#include "HttpDownload.h"
#include <vector>

using std::vector;

//////////////////////////////////////////////////////////////////////////

#define WM_USER_PROG			WM_USER+100
#define WM_USER_TEXTPRINT		WM_USER+101
#define WM_USER_DELETE_ITEM		WM_USER+102


#define	Thread_OutCode_ProgRess	0x110
#define	Thread_OutCode_Print	0x120
#define Thread_OutCode_Sleep	0x130


class _Nv_Download_Ex;



//任务发布器传送的结构体
struct tagDownloadAttriBute
{
    __int64		int64FileSize;
    CHAR		strFileName[1024];
    CHAR		strURL[4096];
    CHAR		strFilePath[1024];
    CHAR		strCollie[4096];
    BOOL		bIsOpen;
    INT			nNameType;

    tagDownloadAttriBute()
    {
        int64FileSize = 0;
        memset(strFilePath, 0, sizeof(strFilePath));
        memset(strURL, 0, sizeof(strURL));
        memset(strFileName, 0, sizeof(strFileName));
        memset(strCollie, 0, sizeof(strCollie));
        bIsOpen	= FALSE;
        nNameType = -1;
    }

    ~tagDownloadAttriBute()
    {

    }
};


//记录下载文件进度的结构体
struct tag_Donwload_File_Ini_Data 
{
    string strUrl;
    string strPath;
    string strFileName;
    string strDataPrint;
    string strCoolie;
    __int64 n64ProgRessPos;
    __int64 n64FileSize;
    BOOL	bOpenFile;

    tag_Donwload_File_Ini_Data()
    {
        n64ProgRessPos = 0;
        bOpenFile = FALSE;
    }
};


//线程任务数据结构体
struct tag_Download_Data 
{
    CProgressUI		*pProgress;
    _Nv_Download	*Nv_Download_;
    CTextUI			*pTextPrint;
    INT				 nPos;
    CListUI			*pList;
    _Nv_Download_Ex *pNv_Download_Ex;
    BOOL		     bMode;

    tag_Download_Data()
    {
        pProgress = NULL;
        Nv_Download_ = NULL;
        pTextPrint = NULL;
        nPos = 0;
        pList = NULL;
        pNv_Download_Ex = NULL;
        bMode = FALSE;
    }
};




class _Nv_Download_Ex
{
public:
    //初始化
    _Nv_Download_Ex()
    { 
        m_ListItemPos=-1;
        m_Handle_ProgRess = NULL;
        m_Handle_Print = NULL;
        m_bClose       = FALSE;
        Nv_Download_   = NULL;
        m_bOpenFile    = NULL;
    };
    ~_Nv_Download_Ex(){};

    //////////////////////////////////////////////////////////////////////////
    //list item的所有指针
    CListUI						*m_pList;
    CListContainerElementUI		*m_pListElement;
    CVerticalLayoutUI			*m_pVerticalLayout;
    CButtonUI					*m_pBtnIco;
    CTextUI						*m_pTextFile;
    CTextUI						*m_pTextPrint;
    CTextUI						*m_pTextJdtBk;
    CProgressUI					*m_pProgRess;
    CButtonUI					*m_pBtnStop;
    CButtonUI					*m_pBtnGo;
    CButtonUI					*m_pBtnOK;
    CButtonUI					*m_pBtnClose;

    //DUI 父窗口指针
    VOID                       *m_pCtrlMng;

    //下载工作类
    _Nv_Download				*Nv_Download_;

    string						m_strUrl;
    string						m_strFileSavePath;
    string						m_strCoolie;

    //当前list的pos
    INT							m_ListItemPos;

    //线程退出代码
    HANDLE						m_Handle_ProgRess;
    HANDLE						m_Handle_Print;

    //是否销毁list
    BOOL						m_bClose;
    //是否直接打开文件
    BOOL						m_bOpenFile;

    //获取父窗口指针
    BOOL SetCtrlMng(VOID *pdlg)
    {
        if (pdlg != NULL)
        {
            m_pCtrlMng = pdlg;
            return TRUE;
        }
        return FALSE;
    }
};

class DownloadDuiControlMng 
{
public:
    DownloadDuiControlMng(VOID) 
    {
        m_dwLinkCount = m_nTaskCount = 0;
        m_pDsgWnd = NULL;
    }
    ~DownloadDuiControlMng() {}

    //创建list_item与DUI item
    BOOL create_ListItem(CHAR *strUrl, CHAR *strFilePath, char *strFileName, BOOL bOpenMode);
    //添加一个list item
    BOOL add_ListItem(CHAR *strUrl, CHAR *strFilePath, BOOL bOpenMode, _Nv_Download_Ex *pdlg);
    //获取list pos
    INT  get_DownloadPtrPos(CControlUI* cContrCmp);
    //获取当前应用程序路径
    CHAR *get_ProcessPath(VOID);

    //添加记录文件
    BOOL AddDownloadRecord(CHAR *strUrl,
                           CHAR *strSaveFilePath,
                           CHAR *strFileName,
                           __in __int64 n64FileSize,
                           CHAR *strCoolie,
                           __in __int64 n64ProgRessPos,
                           CHAR *strPrint,
                           __in bool bOpenFile);

    //修改记录文件
    BOOL ModDownloadRecord(CHAR *strUrl,
                           CHAR *strSaveFilePath,
                           CHAR *strFileName,
                           CHAR *strCoolie,
                           __in __int64 n64FileSize,
                           __in __int64 n64ProgRessPos,
                           CHAR *strPrint,
                           __in bool bOpenFile);

    /* 断点续传文件初始化*/
    BOOL ReInitDownloadRecord(VOID);
    /* 刷新记录文件 */
    BOOL FlushDownloadRecord(VOID);


    BOOL ModRecordFile(tag_Donwload_File_Ini_Data *pData);

    VOID set_StrSign(__in CHAR *strData);

public:
    //数据区域
    //CPaintManagerUI m_pm;
    VOID  *m_pDsgWnd;

public:

    //连接数累计
    DWORD			m_dwLinkCount;
    //任务引用计数
    INT				m_nTaskCount;
    //记录配置文件容器
    vector<tag_Donwload_File_Ini_Data>	m_vtDownLoadFileData;

    //任务追踪容器
    vector<_Nv_Download_Ex*>	        m_vt_Nv_Download;

public:
    _Nv_Download_Ex*  GetCurrentTaskByOptBtn(CControlUI* pBtn);
};
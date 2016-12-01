
#include "StdAfx.h"
#include "HttpDownloadMng.h"
#include "GendoWnd.h"
#include "RopUtil.h"



BOOL DownloadDuiControlMng::add_ListItem(char *strUrl, char *strFilePath, BOOL bOpenMode, _Nv_Download_Ex *pdlg)
{
    //获取list指针
    CListUI *cList_Item = static_cast<CListUI*>(((CGendoWnd*)m_pDsgWnd)->GetDownloadList());
    if(NULL == cList_Item)
    {
        return FALSE;
    }
    pdlg->m_pList = cList_Item;
    //创建list item节点指针
    CListContainerElementUI *new_node = new CListContainerElementUI;
    new_node->ApplyAttributeList(_T("height=\"60\""));
    pdlg->m_pListElement = new_node;

    //创建一个界面布局器 横向布局 高度 60
    CVerticalLayoutUI *new_v_lay = new CVerticalLayoutUI;
    new_v_lay->ApplyAttributeList(_T("height=\"60\""));
    pdlg->m_pVerticalLayout = new_v_lay;

    //////////////////////////////////////////////////////////////////////////
    //创建各种控件
    CButtonUI *cBtn_Ico = new CButtonUI;
    cBtn_Ico->ApplyAttributeList(
        _T("name=\"ico\" float=\"true\" pos=\"5,10,0,0\" width=\"48\" height=\"38\" tooltip=\"图标..\" bkimage=\"file='exe_file.png'\"")
        );
    pdlg->m_pBtnIco = cBtn_Ico;

    CTextUI *cText_file = new CTextUI;
    cText_file->ApplyAttributeList(
        _T("name=\"file\" float=\"true\" pos=\"55,15,0,0\" width=\"300\" height=\"20\" tooltip=\"文件名..\" font=\"0\" textcolor=\"#ffffffff\"")
        );
    pdlg->m_pTextFile = cText_file;

    CTextUI *cText_Print = new CTextUI;
    cText_Print->ApplyAttributeList(
        _T("name=\"print\" float=\"true\" pos=\"55,35,0,0\" width=\"300\" height=\"20\" text=\"正在准备下载..\" tooltip=\"进度提示..\"  font=\"0\" textcolor=\"#ffddd333\"")
        );
    pdlg->m_pTextPrint = cText_Print;

    CTextUI *cText_jdtbk = new CTextUI;
    cText_jdtbk->ApplyAttributeList(
        _T("name=\"jdtbk\" float=\"true\" pos=\"300,15,0,0\" width=\"210\" height=\"7\" bkimage=\"file='progress_back.png'\"")
        );
    pdlg->m_pTextJdtBk = cText_jdtbk;

    CProgressUI *cProgress = new CProgressUI;
    cProgress->ApplyAttributeList(
        _T("name=\"jdt\" float=\"true\" pos=\"300,15,0,0\" width=\"210\" height=\"7\" foreimage=\"file='progress_fore.png'\"")
        );
    pdlg->m_pProgRess = cProgress;
    cProgress->SetMaxValue(100);
    cProgress->SetMinValue(0);

    CButtonUI *cBtn_Stop = new CButtonUI;
    cBtn_Stop->ApplyAttributeList(
        _T("name=\"stop\" float=\"true\" pos=\"400,25,0,0\" width=\"30\" height=\"30\" tooltip=\"暂停下载..\" normalimage=\"file='stop.png'\"")
        );
    pdlg->m_pBtnStop = cBtn_Stop;

    CButtonUI *cBtn_Down = new CButtonUI;
    cBtn_Down->ApplyAttributeList(
        _T("name=\"down\" float=\"true\" pos=\"440,25,0,0\" width=\"30\" height=\"30\" tooltip=\"开始下载..\" normalimage=\"file='go.png'\"")
        );
    pdlg->m_pBtnGo = cBtn_Down;

    CButtonUI *cBtn_Open = new CButtonUI;
    cBtn_Open->ApplyAttributeList(
        _T("name=\"open\" float=\"true\" pos=\"480,25,0,0\" width=\"30\" height=\"30\" tooltip=\"打开文件..\" normalimage=\"file='ok.png'\"")
        );
    pdlg->m_pBtnOK = cBtn_Open;

    CButtonUI *cBtn_Dele = new CButtonUI;
    cBtn_Dele->ApplyAttributeList(
        _T("name=\"dele\" float=\"true\" pos=\"520,5,0,0\" width=\"20\" height=\"20\" tooltip=\"删除任务..\" normalimage=\"file='close.png'\"")
        );
    pdlg->m_pBtnClose = cBtn_Dele;
    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    //将创建好的控件添加到界面布局器中
    new_v_lay->Add(cBtn_Ico);
    new_v_lay->Add(cText_file);
    new_v_lay->Add(cText_Print);
    new_v_lay->Add(cText_jdtbk);
    new_v_lay->Add(cProgress);
    new_v_lay->Add(cBtn_Stop);
    new_v_lay->Add(cBtn_Down);
    new_v_lay->Add(cBtn_Open);
    new_v_lay->Add(cBtn_Dele);

    //将界面布局器添加到list item节点中
    new_node->Add(new_v_lay);
    //最后把list节点插入到list中
    cList_Item->AddAt(new_node,0);

    return TRUE;
}


//创建list item
BOOL DownloadDuiControlMng::create_ListItem(CHAR *strUrl, CHAR *strFilePath, CHAR *strFileName, BOOL bOpenMode)
{
    tag_Donwload_File_Ini_Data tag_File_Data;
    _Nv_Download_Ex	*Nv_Task_ = new _Nv_Download_Ex;
    CDuiString cDuiStr;
    WCHAR wUrl[512] = {0};
    WCHAR wFilePath[512] = {0};
    WCHAR wFileName[512] = {0};
    INT   iLen = 512;

    MultiByteToWideChar(CP_ACP, 0, strUrl, 20, wUrl, iLen);
    iLen = 512;
    MultiByteToWideChar(CP_ACP, 0, strFilePath, -1, wFilePath, iLen);
    iLen = 512;
    MultiByteToWideChar(CP_ACP, 0, strFileName, -1, wFileName, iLen);

    string sFilePathName = strFilePath;
    sFilePathName       += strFileName;


    tag_File_Data.strUrl         = strUrl;
    tag_File_Data.strFileName    = strFileName;
    tag_File_Data.strPath        = strFilePath;
    tag_File_Data.strDataPrint   = sFilePathName;
    tag_File_Data.strCoolie      = "";
    tag_File_Data.n64ProgRessPos = 5;
    tag_File_Data.n64FileSize    = 100;
    tag_File_Data.bOpenFile      = TRUE;

    Nv_Task_->m_strUrl          = strUrl;
    Nv_Task_->m_strFileSavePath = sFilePathName;
    Nv_Task_->m_bOpenFile       = bOpenMode;
    Nv_Task_->m_strCoolie       = "";
    

    //获取list指针
    CListUI *cList_Item = static_cast<CListUI*>(((CGendoWnd*)m_pDsgWnd)->GetDownloadList());
    if(NULL == cList_Item)
    {
        return FALSE;
    }

    //创建list item节点指针
    CListContainerElementUI *new_node = new CListContainerElementUI;
    new_node->ApplyAttributeList(_T("height=\"45\""));


    //创建一个界面布局器 横向布局 高度 60
    CVerticalLayoutUI *new_v_lay = new CVerticalLayoutUI;
    new_v_lay->ApplyAttributeList(_T("height=\"45\""));


    //////////////////////////////////////////////////////////////////////////
    //创建各种控件
    CButtonUI *cBtn_Ico = new CButtonUI;
    cBtn_Ico->ApplyAttributeList(
        _T("name=\"ico\" float=\"true\" pos=\"5,5,0,0\" width=\"45\" height=\"38\" tooltip=\"图标..\" bkimage=\"file='exe_file.png'\"")
        );


    CTextUI *cText_file = new CTextUI;
    cText_file->ApplyAttributeList(
        _T("name=\"file\" float=\"true\" pos=\"55,8,0,0\" width=\"300\" height=\"20\" tooltip=\"文件名..\" font=\"0\" textcolor=\"#FFAAAAAA\"")
        );

    cText_file->SetText(wUrl);


    CTextUI *cText_Print = new CTextUI;
    cText_Print->ApplyAttributeList(
        _T("name=\"print\" float=\"true\" pos=\"55,25,0,0\" width=\"300\" height=\"20\" text=\"正在准备下载..\" tooltip=\"进度提示..\"  font=\"0\" textcolor=\"#0070cc\"")
        );

    cText_Print->SetText(_T("腾讯QQ 把妹神器！"));

    CTextUI *cText_jdtbk = new CTextUI;
    cText_jdtbk->ApplyAttributeList(
        _T("name=\"jdtbk\" float=\"true\" pos=\"300,10,0,0\" width=\"220\" height=\"10\" bkimage=\"file='progress_back.png'\"")
        );


    CProgressUI *cProgress = new CProgressUI;
    cProgress->ApplyAttributeList(
        _T("name=\"jdt\" float=\"true\" pos=\"300,10,0,0\" width=\"220\" height=\"10\" foreimage=\"file='progress_fore.png'\"")
        );
    cProgress->SetMaxValue((int)(tag_File_Data.n64FileSize));
    cProgress->SetMinValue(0);
    cProgress->SetValue((int)(tag_File_Data.n64ProgRessPos));

    CButtonUI *cBtn_Stop = new CButtonUI;
    cBtn_Stop->ApplyAttributeList(
        _T("name=\"stop\" float=\"true\" pos=\"450,25,0,0\" width=\"14\" height=\"14\" tooltip=\"暂停下载..\" normalimage=\"file='toolbar\\pause2.png'\"")
        );


    CButtonUI *cBtn_Down = new CButtonUI;
    cBtn_Down->ApplyAttributeList(
        _T("name=\"down\" float=\"true\" pos=\"475,25,0,0\" width=\"14\" height=\"14\" tooltip=\"开始下载..\" normalimage=\"file='toolbar\\start.png'\"")
        );


    CButtonUI *cBtn_Open = new CButtonUI;
    cBtn_Open->ApplyAttributeList(
        _T("name=\"open\" float=\"true\" pos=\"500,25,0,0\" width=\"14\" height=\"14\" tooltip=\"打开文件..\" normalimage=\"file='toolbar\\open.png'\"")
        );


    CButtonUI *cBtn_Dele = new CButtonUI;
    cBtn_Dele->ApplyAttributeList(
        _T("name=\"dele\" float=\"true\" pos=\"540,15,0,0\" width=\"20\" height=\"20\" tooltip=\"删除任务..\" normalimage=\"file='close.png'\"")
        );

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////
    //将创建好的控件添加到界面布局器中
    new_v_lay->Add(cBtn_Ico);
    new_v_lay->Add(cText_file);
    new_v_lay->Add(cText_Print);
    new_v_lay->Add(cText_jdtbk);
    new_v_lay->Add(cProgress);
    new_v_lay->Add(cBtn_Stop);
    new_v_lay->Add(cBtn_Down);
    new_v_lay->Add(cBtn_Open);
    new_v_lay->Add(cBtn_Dele);

    //将界面布局器添加到list item节点中
    new_node->Add(new_v_lay);
    //最后把list节点插入到list中
    cList_Item->Add(new_node);


    Nv_Task_->m_pList = cList_Item;
    Nv_Task_->m_pListElement = new_node;
    Nv_Task_->m_pVerticalLayout = new_v_lay;
    Nv_Task_->m_pBtnIco = cBtn_Ico;
    Nv_Task_->m_pTextFile = cText_file;
    Nv_Task_->m_pTextPrint = cText_Print;
    Nv_Task_->m_pTextJdtBk = cText_jdtbk;
    Nv_Task_->m_pProgRess = cProgress;
    Nv_Task_->m_pBtnStop = cBtn_Stop;
    Nv_Task_->m_pBtnGo = cBtn_Down;
    Nv_Task_->m_pBtnOK = cBtn_Open;
    Nv_Task_->m_pBtnClose = cBtn_Dele;

    m_vt_Nv_Download.push_back(Nv_Task_);

    Nv_Task_->SetCtrlMng(this);
    return TRUE;
}


int DownloadDuiControlMng::get_DownloadPtrPos(CControlUI* cContrCmp)
{
    if (cContrCmp == NULL)
    {
        return -1;
    }

    DWORD dwSize = m_vt_Nv_Download.size();
    for (int i=0; i<(int)dwSize; ++i)
    {
        if (m_vt_Nv_Download[i]->m_pListElement == cContrCmp)
        {
            return i;
        }
    }

    return -1;
}


/* 
 * Get task element by button clicked
 */
_Nv_Download_Ex *DownloadDuiControlMng::GetCurrentTaskByOptBtn(CControlUI* pBtn)
{
    if (pBtn == NULL) return NULL;

    CControlUI *pBtnPapa  = pBtn->GetParent();
    if (pBtnPapa == NULL) return NULL;

    CControlUI *pListElmt = pBtnPapa->GetParent();
    if (pListElmt == NULL) return NULL;

    DWORD dwSize = m_vt_Nv_Download.size();

    for (int i = 0; i < (int)dwSize; i++) {
        if (m_vt_Nv_Download[i]->m_pListElement == pListElmt) {
            return m_vt_Nv_Download[i];
        }
    }

    return NULL;
}



char *DownloadDuiControlMng::get_ProcessPath(void)
{
    static CHAR szOutCpyFile[512] = {0};
    TCHAR  cPath[512] = {0};
    int    iSize = 512;
    DWORD dw = GetModuleFileName(NULL, (LPWSTR)(cPath), 512);
    DWORD dwSize = lstrlen(cPath);

    for (int i=0; i<(int)dwSize; ++i) {
        if(cPath[dwSize-i-1] == _T('\\')) {
            cPath[dwSize-i] = _T('\0');
            break;
        }
    }
    szOutCpyFile[0] = '\0';
    wc2mb(cPath, dwSize, szOutCpyFile, &iSize);
    
    return szOutCpyFile;
}



//添加一个配置文件记录
BOOL DownloadDuiControlMng::AddDownloadRecord(
    CHAR *strUrl,
    CHAR *strSaveFilePath,
    CHAR *strFileName,
    __in __int64 n64FileSize,
    CHAR *strCoolie,
    __in __int64 n64ProgRessPos,
    CHAR *strPrint,
    __in bool bOpenFile
    )
{
    if (strCoolie == NULL)
    {
        strCoolie = "strCoolie";
    }
    //拷贝数据
    tag_Donwload_File_Ini_Data  tag_File_Data;
    tag_File_Data.strUrl = strUrl;
    tag_File_Data.strPath = strSaveFilePath;
    tag_File_Data.strFileName = strFileName;
    tag_File_Data.strDataPrint = strPrint;
    tag_File_Data.strCoolie = strCoolie;
    tag_File_Data.n64ProgRessPos = n64ProgRessPos;
    tag_File_Data.bOpenFile = bOpenFile;
    tag_File_Data.n64FileSize = n64FileSize;

    vector<tag_Donwload_File_Ini_Data>::iterator it;
    it = m_vtDownLoadFileData.begin();
    m_vtDownLoadFileData.insert(it, tag_File_Data);

    string strPath = string(get_ProcessPath()) + "downloadfile.ini"; 
    FILE *fp = fopen(strPath.c_str(), "w+");
    if (fp != NULL)
    {
        //写入
        DWORD dwSize = m_vtDownLoadFileData.size();
        fprintf(fp, "%d\n", dwSize);
        for (int i=0; i<(int)dwSize; ++i)
        {
            fprintf(fp, "%s\n%s\n%s\n%s\n%s\n%lld\n%lld\n%d\n",
                m_vtDownLoadFileData[i].strUrl.c_str(),
                m_vtDownLoadFileData[i].strPath.c_str(),
                m_vtDownLoadFileData[i].strFileName.c_str(),
                m_vtDownLoadFileData[i].strCoolie.c_str(),
                m_vtDownLoadFileData[i].strDataPrint.c_str(),
                m_vtDownLoadFileData[i].n64FileSize,
                m_vtDownLoadFileData[i].n64ProgRessPos,
                m_vtDownLoadFileData[i].bOpenFile
                );
        }

        fclose(fp);
    }

    return TRUE;
}

BOOL DownloadDuiControlMng::ModRecordFile(tag_Donwload_File_Ini_Data *pData)
{
    int dwSize = m_vtDownLoadFileData.size();
    int iCur = -1, i = 0;
    for (i = 0; i < dwSize; i++) {
        if (m_vtDownLoadFileData[i].strUrl == pData->strUrl &&
            m_vtDownLoadFileData[i].strFileName == pData->strFileName)
        {
            iCur = i;
            break;
        }
    }

    if (iCur == -1) {
        return FALSE;
    }

    m_vtDownLoadFileData[iCur] = *pData;

    /* force all download record flushed */
    string strPath = string(get_ProcessPath()) + "downloadfile.ini"; 
    FILE *fp = fopen(strPath.c_str(), "w+");
    if (fp != NULL)
    {
        //写入
        dwSize = m_vtDownLoadFileData.size();
        fprintf(fp, "%d\n", dwSize);
        for (int i=0; i<(int)dwSize; ++i)
        {
            fprintf(fp, "%s\n%s\n%s\n%s\n%s\n%lld\n%lld\n%d\n",
                    m_vtDownLoadFileData[i].strUrl.c_str(),
                    m_vtDownLoadFileData[i].strPath.c_str(),
                    m_vtDownLoadFileData[i].strFileName.c_str(),
                    m_vtDownLoadFileData[i].strCoolie.c_str(),
                    m_vtDownLoadFileData[i].strDataPrint.c_str(),
                    m_vtDownLoadFileData[i].n64FileSize,
                    m_vtDownLoadFileData[i].n64ProgRessPos,
                    m_vtDownLoadFileData[i].bOpenFile);
            }
        fclose(fp);
    }
    return true;
}


//修改一个配置文件记录
BOOL DownloadDuiControlMng::ModDownloadRecord(
                                            CHAR *strUrl,
                                            CHAR *strSaveFilePath,
                                            CHAR *strFileName,
                                            CHAR *strCoolie,
                                            __in __int64 n64FileSize,
                                            __in __int64 n64ProgRessPos,
                                            CHAR *strPrint,
                                            __in bool bOpenFile)
{
    if (strCoolie == NULL)
    {
        strCoolie = "strCoolie";
    }
    //拷贝数据
    tag_Donwload_File_Ini_Data  tag_File_Data;
    tag_File_Data.strUrl = strUrl;
    tag_File_Data.strPath = strSaveFilePath;
    tag_File_Data.strFileName = strFileName;
    tag_File_Data.strDataPrint = strPrint;
    tag_File_Data.strCoolie = strCoolie;
    tag_File_Data.n64FileSize    = n64FileSize;
    tag_File_Data.n64ProgRessPos = n64ProgRessPos;
    tag_File_Data.bOpenFile      = bOpenFile;

    ModRecordFile(&tag_File_Data);

    return TRUE;
}


//读取配置文件
BOOL DownloadDuiControlMng::ReInitDownloadRecord(VOID)
{

    string strPath = string(get_ProcessPath()) + "downloadfile.ini"; 
    FILE *fp = fopen(strPath.c_str(), "r");
    WCHAR wBuf[1024] = {0};
    CHAR  cBuf[1024] = {0};
    int   wLen = 1024, cLen = 1024;

    if (fp != NULL)
    {
        //拷贝数据
        tag_Donwload_File_Ini_Data  tag_File_Data;
        //写入
        DWORD dwSize = 0;
        fscanf(fp, "%d\n", &dwSize);
        for (int i=0; i<(int)dwSize; ++i)
        {
            fgets(cBuf, 1024, fp);
            set_StrSign(cBuf);
            tag_File_Data.strUrl = cBuf;

            fgets(cBuf, 1024, fp);
            set_StrSign(cBuf);
            tag_File_Data.strPath = cBuf;

            fgets(cBuf, 1024, fp);
            set_StrSign(cBuf);
            tag_File_Data.strFileName = cBuf;

            fgets(cBuf, 1024, fp);
            set_StrSign(cBuf);
            tag_File_Data.strCoolie = cBuf;

            fgets(cBuf, 1024, fp);
            set_StrSign(cBuf);
            tag_File_Data.strDataPrint = cBuf;

            fscanf(fp, "%lld\n%lld\n%d\n", &tag_File_Data.n64FileSize, &tag_File_Data.n64ProgRessPos, &tag_File_Data.bOpenFile);

            m_vtDownLoadFileData.push_back(tag_File_Data);

            create_ListItem((char*)tag_File_Data.strUrl.c_str(), (char*)tag_File_Data.strPath.c_str(), (char*)tag_File_Data.strFileName.c_str(), tag_File_Data.bOpenFile);

        }

        fclose(fp);
    }

    return TRUE;
}


BOOL DownloadDuiControlMng::FlushDownloadRecord(VOID)
{
    string strPath = string(get_ProcessPath()) + "downloadfile.ini"; 
    FILE *fp = fopen(strPath.c_str(), "w+");
    if (fp != NULL)
    {
        //写入
        DWORD dwSize = m_vtDownLoadFileData.size();
        fprintf(fp, "%d\n", dwSize);
        for (int i=0; i<(int)dwSize; ++i)
        {
            fprintf(fp, "%s\n%s\n%s\n%s\n%s\n%lld\n%lld\n%d\n",
                m_vtDownLoadFileData[i].strUrl.c_str(),
                m_vtDownLoadFileData[i].strPath.c_str(),
                m_vtDownLoadFileData[i].strFileName.c_str(),
                m_vtDownLoadFileData[i].strCoolie.c_str(),
                m_vtDownLoadFileData[i].strDataPrint.c_str(),
                m_vtDownLoadFileData[i].n64FileSize,
                m_vtDownLoadFileData[i].n64ProgRessPos,
                m_vtDownLoadFileData[i].bOpenFile
                );
        }

        fclose(fp);
        return TRUE;
    }

    return FALSE;
}


//剔除字符串\n
void DownloadDuiControlMng::set_StrSign(__in char *strData)
{
    int nLen = 0;

    while (strData[nLen] != '\0')
    {
        if (strData[nLen] == '\n')
        {
            strData[nLen] = '\0';
        }
        nLen++;
    }
}
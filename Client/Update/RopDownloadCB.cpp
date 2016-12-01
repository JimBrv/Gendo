#include "stdafx.h"
#include "RopUtil.h"
#include "RopDownloadCB.h"
#include "UpdateWnd.h"

static DWORD dwDownloaded = 0;
static DWORD dwTotal = 0;

int UpdateDownload(PVOID pCtx, TCHAR *sUpdateItem, DWORD iDownload)
{
    CUpdateWnd *pWnd = (CUpdateWnd*)pCtx;
    dwDownloaded += iDownload;
#if 0
    LDebug(DEBUG_L_WARNING, "Download '%S' bytes => %d, complete => %d, total => %d\n",
        sUpdateItem, 
        iDownload, 
        dwDownloaded, 
        dwTotal);
#endif

    if (pWnd != NULL && dwTotal > 0) {
        /* Notify UI progress bar */
        pWnd->m_pProgress->SetValue(dwDownloaded);
        TCHAR sSpeed[64] = {0};
        TCHAR sRatio[64] = {0};
        FLOAT fDownloaded = (FLOAT)dwDownloaded;
        FLOAT fTotal = (FLOAT)dwTotal;
        FLOAT fRatio = (fDownloaded/fTotal);
        swprintf(sSpeed, _T("%dK/%dK\t (%2.0f%%)"), dwDownloaded/1024, dwTotal/1024, fRatio*100);
        pWnd->m_pLbSpeed->SetText(sSpeed);
        //pWnd->m_pLbRatio->SetText(sRatio);
    }


    return 0;
}


CDownloadCallback::CDownloadCallback()
{
    m_sInfo[0] = 0;
    m_pCtx = NULL;
    iDownloaded = 0;
    dwDownloaded = 0;
    dwTotal = 0;
    m_bSetMax = FALSE;
}

CDownloadCallback::CDownloadCallback(TCHAR *sInfo, PVOID pCtx)
{
    lstrcpyn(m_sInfo, sInfo, 64);
    iDownloaded = 0;
    m_pCtx = pCtx;
    dwDownloaded = 0;
    dwTotal = 0;
    m_bSetMax = FALSE;
}

CDownloadCallback::~CDownloadCallback()
{

}

VOID CDownloadCallback::SetCtx(PVOID pCtx)
{
    m_pCtx = pCtx;
}

HRESULT CDownloadCallback::OnProgress(ULONG ulProgress,   ULONG ulProgressMax,
                                      ULONG ulStatusCode, LPCWSTR wszStatusText )
{
    UNREFERENCED_PARAMETER(ulStatusCode);
    if (!m_bSetMax && m_pCtx != NULL && ulProgressMax > 0) {
            CUpdateWnd *pWnd = (CUpdateWnd*)m_pCtx;
            pWnd->m_pProgress->SetMaxValue(ulProgressMax);
            pWnd->m_pLbItem->SetText(m_sInfo);
            m_bSetMax = TRUE;
    }
    DWORD iDelta = 0;
    iDelta      = ulProgress - iDownloaded ;
    iDownloaded = ulProgress;
    dwTotal     = ulProgressMax;
    UpdateDownload(m_pCtx, m_sInfo, iDelta);
    return S_OK;
}
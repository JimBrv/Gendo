#ifndef ROP_UPDATE_H
#define ROP_UPDATE_H
// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 ROPUPDATE_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// ROPUPDATE_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifndef  ROPUPDATE_EXPORTS
#define ROPUPDATE_EXPORTS
#endif

#ifdef ROPUPDATE_EXPORTS
#define ROPUPDATE_API __declspec(dllexport)
#else
#define ROPUPDATE_API __declspec(dllimport)
#endif


typedef enum {
    PHASE_CONNECT_SERVER = 1,
    PHASE_CHECK_VERSION,
    PHASE_DOWNLOAD_COMPONENT,
    PHASE_UPDATE_COMPONENT,
}UPDATE_PHASE_E;


/* UI phase step call back function while updateing */
typedef void (*PUiPhaseStepCB)(unsigned int iNeedTime,  unsigned int iSubPhaseType, char *sName);    // sub phase steps

typedef BOOL (*PIsServerAvaliable)();
typedef BOOL (*PIsNeedUpdate)();
typedef BOOL (*PUpdate)();
typedef int  (*PInit)(PUiPhaseStepCB pUiStepCB);
typedef void (*PFinl)();


extern "C" {
    ROPUPDATE_API BOOL IsServerAvaliable(void);
    ROPUPDATE_API BOOL IsNeedUpdate(void);
    ROPUPDATE_API BOOL Update(void);
    ROPUPDATE_API INT  UpdateNotice(TCHAR *pMsg, INT row, INT col_size);
    ROPUPDATE_API int  Init(PUiPhaseStepCB pUiStepCB, PVOID pOnProgressCtx);
    ROPUPDATE_API void Finl(void);
};



#include "RopUtil.h"
#define UpDbg(...) ModDbg(DEBUG_L_DEBUG, CTRL_UPDATE_MODULE, ##__VA_ARGS__)
#define UpErr(...) ModDbg(DEBUG_L_CRITICAL, CTRL_UPDATE_MODULE, ##__VA_ARGS__)

#endif
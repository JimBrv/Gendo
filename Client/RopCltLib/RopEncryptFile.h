#pragma  once
#include "StdAfx.h"
#include <vector>
#include <string>
#include <assert.h>
#include "RopBase.h"
#include "RopAES.h"
#pragma warning( disable:4996)

class RopEncryptFile 
{
public:
    RopEncryptFile();
    ~RopEncryptFile();

    int  OpenEncryptFile(char *path, char *file);
    int  CloseEncryptFile();
    int  FindLine(int  line_num, char *output, int len);
    int  FindLine(char *pattern,char *output, int len);
    void DumpFile();

private:
    FILE *m_fp;
    char **m_buf;
    int   m_row;
    int   m_size;
    char  m_pathfile[256];
    bool  m_opened;
};
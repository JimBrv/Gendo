
#include "StdAfx.h"
#include <io.h>
#include "RopEncryptFile.h"

#define MAX_TEXT_FILE_ROW  512
#define MAX_TEXT_FILE_COL  512    // 512*512 text file size

RopEncryptFile::RopEncryptFile()
{
    m_fp  = NULL;
    m_buf = NULL;
    m_size = 0;
    m_row = 0;
    m_opened =false;
}

RopEncryptFile::~RopEncryptFile()
{
    CloseEncryptFile();
}

int  RopEncryptFile::OpenEncryptFile(char *path, char *file)
{
    char fpath[256] = {0};
    char line[1024*4] = {0};
    RopAES  aes;
    sprintf(fpath, "%s\\%s", path, file);

    m_fp = fopen(fpath, "r");
    if (m_fp == NULL) return -1;
    int size = filelength(fileno(m_fp));
    if (size > MAX_TEXT_FILE_COL*MAX_TEXT_FILE_ROW) {
        /* too large for memory file */
        fclose(m_fp);
        m_fp = NULL;
        return -2;
    }


    m_buf = (char **)new char[MAX_TEXT_FILE_ROW][MAX_TEXT_FILE_COL];
    assert(m_buf != NULL);
    m_size = MAX_TEXT_FILE_ROW*MAX_TEXT_FILE_COL;

    int cur = 0;
    char dec_line[1024*4] = {0};
    while (fgets(line, 1024*4, m_fp) != NULL) {
        int len1 = strlen(line);
        len1 -= 1;
        line[len1] = 0; // Clean the Addtional '\n'
        int len2 = aes.DecryptWithHex(line, len1, dec_line, 4*1024);
        if (len2 <= 0) {
            printf("error on decrypt file=%s\n", fpath);
            goto ERROR;
        }
        len1 = strlen(dec_line);
        memcpy(m_buf[cur], dec_line, len1);
        cur++;
        memset(line, 0, sizeof(line));
        memset(dec_line, 0, sizeof(dec_line));
    }

    strcpy(m_pathfile, fpath);
    m_opened = true;
    m_row = cur;
    fclose(m_fp);
    m_fp = NULL;
    return 0;

ERROR:
    fclose(m_fp);
    m_fp = NULL;
    delete m_buf;
    m_buf = NULL;
    return -1;
}


int  RopEncryptFile::CloseEncryptFile()
{
    if (m_fp) {
        fclose(m_fp);
        m_fp = NULL;
    }
    if (m_buf) {
        delete m_buf;
        m_buf = NULL;
        m_size = 0;
    }
    m_opened = false;
    return 0;
}

int  RopEncryptFile::FindLine(int line_num, char *output, int len)
{
    if (!m_opened) return -1;
    if (line_num < 1 || line_num > m_row || m_buf == NULL) return -2;
    strcpy(output, m_buf[line_num - 1]);
    return 0;
}

int  RopEncryptFile::FindLine(char *pattern,char *output, int len)
{
    if (!m_opened) return -1;
    if (!m_buf) return -2;
    int cur = 0;
    bool found = false;
    while (cur < m_row) {
        if (strstr(pattern, m_buf[cur])) {
            strcpy(output, m_buf[cur]);
            found = true;
            break;
        }
        cur++;
    }
    return found ? 0 : -3;
}

void RopEncryptFile::DumpFile()
{
    if (!m_opened || !m_buf) return;
    printf("file: %s, size=%d\n", m_pathfile, m_size);
    printf("%s", m_buf);
}
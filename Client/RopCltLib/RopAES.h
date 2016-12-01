
#pragma  once
#include "StdAfx.h"
#include <openssl/aes.h>
#include <openssl/sha.h>

#pragma warning( disable:4996)

#define ROP_AES_KEY_SIZE      16
#define ROP_AES_KEY_DEFAULT   "GendoCloudVPNKey"
#define ROP_FILE_MAX_MEM_SIZE 1024*1024*10   // Max open 10M file in mem


class RopAES  
{
public:
    RopAES(void);
    RopAES(unsigned char *key, int enc_type);
    virtual ~RopAES() {return;};
    /* return encryped len in output, <= 0 means output buffer overflow */
    int Encrypt(char *input, int len1, char *output, int len2);
    int Decrypt(char *input, int len1, char *output, int len2);

    /* Encrypt/Decrypt the raw buffer with the 0-9,A-E HEX chars */
    int EncryptWithHex(char *input, int len1, char *output, int len2);
    int DecryptWithHex(char *input, int len1, char *output, int len2);

    /* En/Decrypt for text file */
    int EncryptTextFile(char *in_path, char *in_file, char *out_path, char *out_file);
    int DecryptTextFile(char *in_path, char *in_file, char *out_path, char *out_file);


    /* En/Decrypt for binary file */
    int EncryptBinary(char *in_path, char *in_file, char *out_path, char *out_file);
    int DecryptBinary(char *in_path, char *in_file, char *out_path, char *out_file);

    /* En/Decrypt for buffer array */
    int EncryptFileByBufAry(char *out_path, char *out_file, char *buf, int  row, int col_size);
    int DecryptFileToBufAry(char *out_path, char *out_file, char *buf, int *row, int col_size);


    int Hex2Mem(char *input, int len1, unsigned char *output, int len2);
    int Mem2Hex(char *input, int len1, char *output, int len2);
    void DumpMem(void *buf, int len);

    short A2I(char a);


    /* SHA hash */
    void SHA256(char *input, char *output);
private:
    unsigned char m_key[ROP_AES_KEY_SIZE + 1];   // must be 16 bytes
    int      m_keylen;    // must be 16 bytes, or fill 0
    int      m_enctype;   //128, 256 AES alg
    AES_KEY  m_enckey;
    AES_KEY  m_deckey;

};
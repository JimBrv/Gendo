
#include "StdAfx.h"
#include "string.h"
#include "stdio.h"
#include "assert.h"
#include <io.h>
#include <openssl/sha.h>
#include "RopAES.h"


RopAES::RopAES(unsigned char *key, int enc_type) 
{
    int ret = 0;
    memset(m_key, 0,   ROP_AES_KEY_SIZE);
    memcpy(m_key, key, ROP_AES_KEY_SIZE);
    m_enctype = enc_type;
    ret = AES_set_encrypt_key(m_key, enc_type, &m_enckey);
    if (ret) {
        printf("AES set encrypt ret = %d\n", ret);
        return;
    }
    ret = AES_set_decrypt_key(m_key, enc_type, &m_deckey);
    if (ret) {
        printf("AES set decrypt ret = %d\n", ret);
        return;
    }
}

RopAES::RopAES(void) 
{
    int ret = 0;
    memset(m_key, 0,   ROP_AES_KEY_SIZE);
    memcpy(m_key, ROP_AES_KEY_DEFAULT, ROP_AES_KEY_SIZE);
    m_enctype = 128;
    ret = AES_set_encrypt_key(m_key, m_enctype, &m_enckey);
    if (ret) {
        printf("AES set encrypt ret = %d\n", ret);
        return;
    }
    ret = AES_set_decrypt_key(m_key, m_enctype, &m_deckey);
    if (ret) {
        printf("AES set decrypt ret = %d\n", ret);
        return;
    }
}

int RopAES::Encrypt(char *input, int len1, char *output, int len2)
{
    int i = 0; 
    int len = len1;
    if (len2 < len1) {
        return -1;
    }

    while(i < len) {
        AES_encrypt((unsigned char *) input + i,
            (unsigned char *) output + i, 
            &m_enckey);
        i += 16;
    }

    return i;
}


int RopAES::Decrypt(char *input, int len1, char *output, int len2)
{
    int i = 0; 
    int len = len1;
    if (len2 < len1) {
        return -1;
    }

    while(i < len) {
        AES_decrypt((unsigned char *) input + i,
            (unsigned char *) output + i,  
            &m_deckey);
        i += 16;
    }
    return i;
}

short RopAES::A2I(char a)
{
    short i = 0;
    switch (a) {
    case '0':
        i = 0;
        break;
    case '1':
        i = 1;
        break;
    case '2':
        i = 2;
        break;
    case '3':
        i = 3;
        break;
    case '4':
        i = 4;
        break;
    case '5':
        i = 5;
        break;
    case '6':
        i = 6;
        break;
    case '7':
        i = 7;
        break;
    case '8':
        i = 8;
        break;
    case '9':
        i = 9;
        break;
    case 'a':
    case 'A':
        i = 10;
        break;
    case 'b':
    case 'B':
        i = 11;
        break;
    case 'c':
    case 'C':
        i = 12;
        break;
    case 'd':
    case 'D':
        i = 13;
        break;
    case 'e':
    case 'E':
        i = 14;
        break;
    case 'f':
    case 'F':
        i = 15;
        break;
    default:
        i = -1;
        break;
    }
    return i;
}

int RopAES::Hex2Mem(char *input, int len1, unsigned char *output, int len2)
{
    int i = 0, j = 0;
    assert(len2 >= (len1/2));

    for(; i < len1; i+=2, j++) {
        short high = A2I(input[i]);
        if (high < 0) return -1;
        short low  = A2I(input[i+1]);
        if (low < 0) return -1;
        short all = (high << 4) + low;
        *(output+j) = (unsigned char)all;
    }
    return len1/2;
}


int RopAES::Mem2Hex(char *input, int len1, char *output, int len2)
{
    int i = 0, j = 0;
    assert(len2 >= 2*len1);

    for(;i < len1; i++, j+=2) {
        sprintf(output + j,     "%X", ((input[i] >> 4)&0x0F));
        sprintf(output + j + 1, "%X", (input[i] & 0x0F));
    }
    return len1*2;
}

/* Encrypt/Decrypt the raw buffer with the 0-9,A-E HEX chars */
int RopAES::EncryptWithHex(char *input, int len1, char *output, int len2)
{
    char *buf = NULL;
    buf = new char[len1 + 16];
    assert(buf != NULL);

    int len = Encrypt(input, len1, buf, len1+16);
    if (len <= 0) {
        // error;
        delete buf;
        return -1;
    }

    len = Mem2Hex(buf, len, output, len2);
    delete buf;
    return len;
}

int RopAES::DecryptWithHex(char *input, int len1, char *output, int len2)
{
    unsigned char *buf = new unsigned char[len1 + 16];
    memset(buf, 0, len1+16);
    assert(buf != NULL);

    int len = Hex2Mem(input, len1, buf, len1+16);
    if (len < 0) {
        printf("Hex file corrupt! Decrypt failed\n");
        delete buf;
        return -1;
    }
    len = Decrypt((char *)buf, len, output, len2);
    delete buf;
    return len;
}

void RopAES::DumpMem(void *buf, int len)
{
    char *start = (char *)buf;
    printf("\n Dump: ");
    for (int i = 0; i < len; i++) {
        printf("%X%X ", (start[i]>>4)&0x0F, start[i]&0x0F);
    }
    printf("\n End Dump\n");
}

#define MAX_FILE_TEXT_LINE_SIZE 1024*4
#define MAX_FILE_PATH_SIZE      256

int RopAES::EncryptTextFile(char *in_path, char *in_file, char *out_path, char *out_file)
{
    char in[MAX_FILE_PATH_SIZE]        = {0};
    char out[MAX_FILE_PATH_SIZE]       = {0};
    char line[MAX_FILE_TEXT_LINE_SIZE] = {0};
    char enc_line[MAX_FILE_TEXT_LINE_SIZE] = {0};
    
    FILE *fp_in = NULL, *fp_out = NULL;
    sprintf(in, "%s\\%s", in_path, in_file);
    sprintf(out, "%s\\%s", out_path, out_file);

    if ((fp_in = fopen(in, "r")) == NULL) {
        return -1;
    }
    
    if ((fp_out = fopen(out, "w+")) == NULL) {
        fclose(fp_in);
        return -1;
    }

    while (fgets(line, MAX_FILE_TEXT_LINE_SIZE, fp_in) != NULL) {
        int len = strlen(line);
        memset(enc_line, 0, MAX_FILE_TEXT_LINE_SIZE);
        len = EncryptWithHex(line, len, enc_line, MAX_FILE_TEXT_LINE_SIZE);
        fputs(enc_line, fp_out);        
        fputs("\n", fp_out);    // Will add additional '\n', remove while decrypt
    }

    if (fp_in) fclose(fp_in);
    if (fp_out) fclose(fp_out);
    return 0;
}

int RopAES::DecryptTextFile(char *in_path, char *in_file, char *out_path, char *out_file)
{
    char in[MAX_FILE_PATH_SIZE]        = {0};
    char out[MAX_FILE_PATH_SIZE]       = {0};
    char line[MAX_FILE_TEXT_LINE_SIZE] = {0};
    char enc_line[MAX_FILE_TEXT_LINE_SIZE] = {0};
    int ret = 0;

    FILE *fp_in = NULL, *fp_out = NULL;
    sprintf(in, "%s\\%s", in_path, in_file);
    sprintf(out, "%s\\%s", out_path, out_file);

    if ((fp_in = fopen(in, "r")) == NULL) {
        return -1;
    }

    if ((fp_out = fopen(out, "w+")) == NULL) {
        fclose(fp_out);
        return -1;
    }

    while (fgets(line, MAX_FILE_TEXT_LINE_SIZE, fp_in) != NULL) {
        int len = strlen(line);
        len -= 1;
        line[len] = 0; // Clean the Addtional '\n'
        memset(enc_line, 0, MAX_FILE_TEXT_LINE_SIZE);
        len = DecryptWithHex(line, len, enc_line, MAX_FILE_TEXT_LINE_SIZE);
        if (len < 0) {
            printf("Decrypt file failed\n");
            ret = -1;
            break;
        }
        fputs(enc_line, fp_out);
    }

    if (fp_in) fclose(fp_in);
    if (fp_out) fclose(fp_out);
    return ret;
}


int RopAES::EncryptBinary(char *in_path, char *in_file, char *out_path, char *out_file)
{
    char in[MAX_FILE_PATH_SIZE]        = {0};
    char out[MAX_FILE_PATH_SIZE]       = {0};
    char line[MAX_FILE_TEXT_LINE_SIZE] = {0};
    char enc_line[MAX_FILE_TEXT_LINE_SIZE + 128] = {0};

    FILE *fp_in = NULL, *fp_out = NULL;
    sprintf(in, "%s\\%s", in_path, in_file);
    sprintf(out, "%s\\%s", out_path, out_file);

    if ((fp_in = fopen(in, "rb")) == NULL) {
        return -1;
    }

    if ((fp_out = fopen(out, "wb+")) == NULL) {
        fclose(fp_in);
        return -1;
    }

    while (!feof(fp_in)) {
        int len = fread(line, sizeof(char), MAX_FILE_TEXT_LINE_SIZE/2, fp_in);
        memset(enc_line, 0, MAX_FILE_TEXT_LINE_SIZE);

        len = EncryptWithHex(line, len, enc_line, MAX_FILE_TEXT_LINE_SIZE);
        fwrite(enc_line, sizeof(char), len, fp_out);
    }

    if (fp_in) fclose(fp_in);
    if (fp_out) fclose(fp_out);
    return 0;
}

int RopAES::DecryptBinary(char *in_path, char *in_file, char *out_path, char *out_file)
{
    char in[MAX_FILE_PATH_SIZE]        = {0};
    char out[MAX_FILE_PATH_SIZE]       = {0};
    char line[MAX_FILE_TEXT_LINE_SIZE] = {0};
    char enc_line[MAX_FILE_TEXT_LINE_SIZE] = {0};

    FILE *fp_in = NULL, *fp_out = NULL;
    sprintf(in, "%s\\%s", in_path, in_file);
    sprintf(out, "%s\\%s", out_path, out_file);

    if ((fp_in = fopen(in, "rb")) == NULL) {
        return -1;
    }

    if ((fp_out = fopen(out, "wb+")) == NULL) {
        fclose(fp_in);
        return -1;
    }

    while (!feof(fp_in)) {
        int len = fread(line, sizeof(char), MAX_FILE_TEXT_LINE_SIZE/2, fp_in);
        memset(enc_line, 0, MAX_FILE_TEXT_LINE_SIZE);

        len = DecryptWithHex(line, len, enc_line, MAX_FILE_TEXT_LINE_SIZE);
        fwrite(enc_line, sizeof(char), len, fp_out);
    }

    if (fp_in) fclose(fp_in);
    if (fp_out) fclose(fp_out);
    return 0;
}


void RopAES::SHA256(char *input, char *output)
{
    int i = 0;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, strlen(input));
    SHA256_Final(hash, &sha256);

    for(i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sprintf(output + (i * 2), "%02x", hash[i]);
    }
}


/* 
 * Create Encrypt file by buffer array:)
 */
int RopAES::EncryptFileByBufAry(char *out_path, char *out_file, char *buf, int row, int col_size)
{
    int  cur = 0;
    char out[MAX_FILE_PATH_SIZE]           = {0};
    char line[MAX_FILE_TEXT_LINE_SIZE]     = {0};
    char enc_line[MAX_FILE_TEXT_LINE_SIZE] = {0};

    FILE *fp_out = NULL;
    sprintf(out, "%s\\%s", out_path, out_file);

    if ((fp_out = fopen(out, "w+")) == NULL) {
        return -1;
    }

    while (cur < row) {
        strcpy(line, buf+(cur*col_size));
        int len = strlen(line);
        memset(enc_line, 0, MAX_FILE_TEXT_LINE_SIZE);
        len = EncryptWithHex(line, len, enc_line, MAX_FILE_TEXT_LINE_SIZE);
        fputs(enc_line, fp_out);        
        fputs("\n", fp_out);    // Will add additional '\n', remove while decrypt
        cur++;
    }

    if (fp_out) fclose(fp_out);
    return 0;
}

/* 
 * Create Encrypt file by buffer array:)
 */
int RopAES::DecryptFileToBufAry(char *out_path, char *out_file, char *buf, int *row, int col_size)
{
    int  cur = 0;
    char in[MAX_FILE_PATH_SIZE]            = {0};
    char line[MAX_FILE_TEXT_LINE_SIZE]     = {0};
    char dec_line[MAX_FILE_TEXT_LINE_SIZE] = {0};

    FILE *fp_in = NULL;
    sprintf(in, "%s\\%s", out_path, out_file);

    if ((fp_in = fopen(in, "r")) == NULL) {
        return -1;
    }

    while (fgets(line, MAX_FILE_TEXT_LINE_SIZE, fp_in) != NULL) {
        int len = strlen(line);
        len -= 1;
        line[len] = 0; // Clean the Addtional '\n'
        memset(dec_line, 0, MAX_FILE_TEXT_LINE_SIZE);
        len = DecryptWithHex(line, len, dec_line, MAX_FILE_TEXT_LINE_SIZE);
        if (len < 0) {
            printf("Decrypt file failed\n");
            return -2;
        }
        strcpy(buf+(cur*col_size), dec_line);
        if (cur >= (*row)) {
            /* will over flow, skip left */
            break;
        }
        cur++;
    }

    *row = cur;
    if (fp_in) fclose(fp_in);
    return 0;
}
/*
 * Filename : lbs-ticket.c
 * Copyright : All right reserved by SecWin, 2010
 * User ticket generator, verifying wrapper.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
#include "lib-debug.h"
#include "lib-config.h"
#include "lib-ticket.h"

static AES_KEY       lib_encrypt_key;
static AES_KEY       lib_decrypt_key;
static unsigned char lib_cryptstr[AES_BLOCK_SIZE];

#define HEX_TAG						0b00001111
char HEX_BUF[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
MD5_CTX 		ctx;


void lib_crypt_set_AEScryptstr(unsigned char *cryptstr)
{
	memcpy(lib_cryptstr, cryptstr, AES_BLOCK_SIZE);
}

void lib_crypt_get_AEScryptstr(unsigned char *cryptstr) 
{
	 memcpy(cryptstr, lib_cryptstr, AES_BLOCK_SIZE);
}

/* init LBS cryption key */
void lib_crypt_init()
{   
    struct timeval tv;
	unsigned char cryptstr[AES_BLOCK_SIZE+1] = {0};

	gettimeofday(&tv, NULL);
	srandom(tv.tv_sec);

	if (config_get(AES_CRYPT_STRING, (char*)cryptstr)) {
		lib_error(MODULE_UTL, "can't get AES crypt string, use default\n");
		memcpy(cryptstr, TICKET_AES_CRYPT_STRING, AES_BLOCK_SIZE);
	}
	memcpy(lib_cryptstr, cryptstr, AES_BLOCK_SIZE);
	AES_set_encrypt_key(lib_cryptstr, 128, &lib_encrypt_key);
	AES_set_decrypt_key(lib_cryptstr, 128, &lib_decrypt_key);
	
	/*
	 * AES shared within LBS, COS.
	 
	for (i = 0; i < AES_BLOCK_SIZE; i++) {
		lib_cryptstr[i] = (random()%254 + 1);    // random to 1-254
	}
	
	AES_set_encrypt_key(lib_cryptstr, 128, &lib_encrypt_key);
	AES_set_decrypt_key(lib_cryptstr, 128, &lib_decrypt_key);
	*/
}

static inline void lib_ticket_hash(char *buf, int len, char *hash)
{
	SHA1((unsigned char *)buf, len, (unsigned char *)hash);
}

static void lib_ticket_encrypt(char *ticket)
{
	int i = 0; 
    int len = USER_TICKET_LEN;
	char buf[USER_TICKET_LEN] = {0};

	while (i < len) {
		AES_encrypt((unsigned char *) ticket + i,
			        (unsigned char *) buf + i, 
			        &lib_encrypt_key);
		i += 16;
	}

	memcpy(ticket, buf, len);
}

static int lib_ticket_unhash(char *buf, int len, char *hash)
{
	char result[20];

	SHA1((unsigned char *)buf, len, (unsigned char *)result);
	return (!memcmp(hash, result, 20));
}



/* decrypt ticket, decrypted msg put in source buf */
static void lib_ticket_decrypt(char *ticket)
{
	int i = 0, len = USER_TICKET_LEN;
	char buf[USER_TICKET_LEN];

	while (i < len) {
		AES_decrypt((unsigned char *) ticket + i,
			    (unsigned char *) buf + i, &lib_decrypt_key);
		i += 16;
	}

	memcpy(ticket, buf, len);
}

int lib_ticket_generate(char *ticket, user_ticket_t *ut)
{
	char *p = (char *)ut;
	
	/* hash userid, reserve, random */
	lib_ticket_hash(p, 12, ut->hash);
	memcpy(ticket, p, USER_TICKET_LEN);
	lib_ticket_encrypt(ticket);
	return OK;
}

void lib_ticket_encrypt_pwd(char *pwd){
	lib_ticket_encrypt(pwd);
}


static int  cstoi(char ch)
{
  switch(ch)
  {
	    case '0'  :  return 0;
	    case '1'  :  return 1;
	    case '2'  :  return 2;
	    case '3'  :  return 3;
	    case '4'  :  return 4;
	    case '5'  :  return 5;
	    case '6'  :  return 6;
	    case '7'  :  return 7;
	    case '8'  :  return 8;
	    case '9'  :  return 9;
	    case 'A'  :  return 10;
	    case 'B'  :  return 11;
	    case 'C'  :  return 12;
	    case 'D'  :  return 13;
	    case 'E'  :  return 14;
	    case 'F'  :  return 15;
	    default   :  return -1;
  }

}


int lib_ticket_decode_pwd(char *opwd,char *npwd){
	if(!opwd||!npwd){
		return ERROR;
	}
	char buftp[USER_TICKET_LEN]={0};
	char *top = opwd;
	char *b = buftp;
	char  hh=0,hl =0;
	unsigned char temp=0;
	int n  = 0;
	while(n < 32){
		hh = cstoi((*top));
		top+=1;
		hl = cstoi((*top));
		temp = hh << 4;
		*b = ( temp+ hl);
		b+=1;
		top+=1;
		n++;
	}
	lib_ticket_decrypt_pwd(buftp);
	memcpy(npwd,buftp,strlen(buftp));
	return OK;
}

int lib_ticket_decode_str(char *opwd,char *npwd){
	if(!opwd||!npwd){
		return ERROR;
	}
	char *top = opwd;
	char *b = npwd;
	char  hh=0,hl =0;
	int n = 0;
	unsigned char temp=0;
	while(n<32){
		hh = cstoi((*top));
		top+=1;
		hl = cstoi((*top));
		temp = hh << 4;
		*b = ( temp+ hl);
		b+=1;
		top+=1;
		n++;
	}
	return OK;
}



void lib_ticket_decrypt_pwd(char *pwd){
	lib_ticket_decrypt(pwd);
}


int lib_ticket_verify(char *ticket)
{
	lib_ticket_decrypt(ticket);
	if (!lib_ticket_unhash(ticket, 12, ticket + 12)) {
		return ERROR;
	}
	return OK;
}

void lib_ticket_dump(char *ticket)
{
	int i = 0, n = 0;
	char buf[USER_TICKET_LEN*6 + 1] = {0};
	
	n += sprintf(buf, "%s", "ticket = [0X ");
	for (i = 0; i < USER_TICKET_LEN; i++) {
		n += sprintf(buf + n, " %02x", (unsigned char)ticket[i]);
	}
	n += sprintf(buf + n, "%s", "]\n");
	lib_info(buf);
}

int lib_encode_str(char* src,char* dst){
	int ret = -1,n = 0;
	unsigned char hx=0;
	unsigned char hh=0,hl = 0;
	char *top = src;
	char *temp = dst;
	if(!src){
		return ret;
	}
	while(n < 32){
		hx = *top;
		hl =  hx&HEX_TAG;
		hh = (hx>>4)&HEX_TAG;
		*temp = HEX_BUF[hh];
		temp++;
		*temp = HEX_BUF[hl];
		temp++;
		top++;
		n++;
	}
	return 0;
}

void lib_cookie_init(){
	MD5_Init(&ctx); 
}

int lib_cookie_create(char *cookie,int id,int random){
       if(!cookie){
	    return -1;
       }
       char buffer[33]={0};
       char tmp[3]={0};
       sprintf(buffer,"%d%d",id,random);
       unsigned char md[16];
       MD5_Update(&ctx,buffer,strlen(buffer));
       MD5_Final(md,&ctx);
       int i =0;
       for( i=0; i<16; i++ ){
        sprintf(tmp,"%02X",md[i]);
        strcat(cookie,tmp);
      }
       return 0;
}



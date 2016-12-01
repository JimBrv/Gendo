/*
  * Header : lib-ticket.h
  * Copyright : All right reserved by SecWin, 2010
  * User ticket
  * 
  * Ver   Author   Date         Desc
  * 0.1	wy      2010.8     initial version        
  */
  
#ifndef LIB_TICKET_H
#define LIB_TICKET_H

#include <openssl/aes.h>
#include <openssl/sha.h>
#include <openssl/md5.h>

#include "lib-base.h"
#include "lib-types.h"

#define USER_TICKET_LEN         32
#define USER_TICKET_HASH_LEN    20

typedef struct user_ticket_s {
	UInt32 userid;
	UInt32 lastlogin;
	UInt32 random;
	char   hash[USER_TICKET_HASH_LEN];
}user_ticket_t, *puser_ticket_t;

#define is_same_ticket(OLD,NEW) (!memcmp((OLD), (NEW), USER_TICKET_LEN))
#define TICKET_AES_CRYPT_STRING "1234567890-=][po"

void lib_crypt_init();
void lib_cookie_init();
int lib_cookie_create(char *cookie,int id,int random);
int lib_encode_str(char* src,char* dst);


int  lib_ticket_generate(char *ticket, user_ticket_t *ut);

int  lib_ticket_verify(char *ticket);

void lib_ticket_dump(char *ticket);

void lib_ticket_encrypt_pwd(char *pwd);

int  lib_ticket_decode_pwd(char *opwd,char *npwd);
int lib_ticket_decode_str(char *opwd,char *npwd);


#endif

/*
 * Header : lib-bit.h
 * Copyright : All right reserved by SecWin, 2010
 * bits operation
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
 
#ifndef LIB_BIT_H
#define LIB_BIT_H

#include "lib-base.h"

typedef enum {
	USER_FLAG_BIT_ATTR   =          0b00001,
	USER_FLAG_BIT_ONLINE =          0b00010,
	USER_FLAG_BIT_RESTORE=          0b00100,
	USER_FLAG_BIT_START_ACCOUNT=    0b01000,//start up account and shut down account
	USER_FLAG_BIT_CANCEL_ACCOUNT=   0b10000,//cancel account
}user_flag_e;



#define set_bit(FLAG, BIT)    ((FLAG)|=(BIT))
#define unset_bit(FLAG, BIT)  ((FLAG)&= ~(BIT))

#define is_bit_set(FLAG, BIT)  ((FLAG)&(BIT))

#endif

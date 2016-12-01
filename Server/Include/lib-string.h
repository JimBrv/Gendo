/*
 * Header : lib-string.h
 * Copyright : All right reserved by SecWin, 2010
 * C string interface.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */

#ifndef	LIB_STRING_H_
#define	LIB_STRING_H_

#include "lib-base.h"
#include <ctype.h>

int lib_string_split(char *src, char split, char **dst, int *n);
int lib_name_sanity(char *name);


#endif

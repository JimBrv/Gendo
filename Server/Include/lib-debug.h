/*
  * Header : lib-debug.h
  * Copyright : All right reserved by SecWin, 2010
  * Printf, fprintf error debug wrappers
  * 
  * Ver   Author   Date         Desc
  * 0.1	wy      2010.8     initial version        
  */

#ifndef LIB_DEBUG_H
#define LIB_DEBUG_H


#include <syslog.h>
#include "lib-base.h"
#include "lib-types.h"

typedef enum {
	DEBUG = 0,
	WARNING,
	ERR,
	CRITICAL,
}PRINT_LEVEL_E;

void lib_error(MODULEID id, char *format, ...);
void lib_debug(MODULEID id, char *format, ...);
void lib_syslog(MODULEID id, char *format, ...);
void lib_debug_set_level(PRINT_LEVEL_E lv);
void lib_set_debug(int on);

void lib_printf(char *format, ...);


#define lib_info(...) lib_debug(MODULE_NONE, __VA_ARGS__)

#define lib_assert(COND) assert((COND)) 

#endif

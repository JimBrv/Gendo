/*
 * Filename : lib-debug.c
 * Copyright : All right reserved by SecWin, 2010
 * printf, fprintf error debug msg implementation
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <errno.h>

#include "lib-base.h"
#include "lib-debug.h"

static int level;
static int debug_on = 1;

void lib_set_debug(int on) 
{
	debug_on = on;
}


static char *gendo_module_name[] = {
    "GAS",    // Gas auth server
    "DBS",
    "LGS",    // Log server
    "UTL",
    "",
}; 

void lib_error(MODULEID id, char *format, ...)
{
    static char error_buf[1024];
    va_list args;
    time_t clock;
    struct tm tm;
    int errnoTp = 0;

	
    errnoTp = errno;
    time(&clock);
    localtime_r(&clock, &tm);
    strftime(error_buf, 100, "%Y/%m/%d %H:%M:%S", &tm);
    va_start(args, format);
    printf("%s-bug(%s): ", gendo_module_name[(int) id], error_buf);
    vprintf(format, args);
    va_end(args);
    errno = errnoTp;
}

void lib_debug(MODULEID id, char *format, ...)
{
    static char debug_buf[1024];
    va_list args;
    time_t clock;
    struct tm tm;
    int errnoTp = 0;

	if (!debug_on) return;
	
    errnoTp = errno;
    time(&clock);
    localtime_r(&clock, &tm);
    strftime(debug_buf, 100, "%Y/%m/%d %H:%M:%S", &tm);
    va_start(args, format);
    printf("%s(%s): ", gendo_module_name[(int) id], debug_buf);
    vprintf(format, args);
    va_end(args);
    errno = errnoTp;
}

/* print without time prefix */
void lib_printf(char *format, ...)
{
    //static char prt_buf[1024];
    va_list args;
    int errnoTp = 0;
	
    errnoTp = errno;

    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    errno = errnoTp;
}

void lib_syslog(MODULEID id, char *format, ...)
{     
    va_list args;
    char buf[512] = {0};

    va_start(args, format);
    vsprintf(buf, format, args);
    syslog(LOG_NOTICE|LOG_USER, "%s", buf);   
    va_end(args);    
}

void lib_debug_set_level(PRINT_LEVEL_E lv)
{
	level = lv;
}

int lib_init_debug(int dest)
{
	level = 0;
    return 0;
}


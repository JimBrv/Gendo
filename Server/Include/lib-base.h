/*
  * Header : lib-base.h
  * Copyright : All right reserved by SecWin, 2010
  * Base library utils header collection and system headers.
  * 
  * Ver   Author   Date         Desc
  * 0.1	wy      2010.8     initial version        
  */

#ifndef LIB_BASE_H
#define LIB_BASE_H

/* std header */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/file.h>
#include <signal.h>
#include <sys/time.h>
#include <assert.h>


/* network header */
#include <resolv.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>


typedef enum {
    MODULE_GAS = 0,
    MODULE_DBS,
    MODULE_LGS,
    MODULE_UTL,
    MODULE_NONE,
}MODULEID;

#endif


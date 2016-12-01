/*
  * Header : lib-time.h
  * Copyright : All right reserved by SecWin, 2010
  * Time wrappers
  * 
  * Ver   Author   Date         Desc
  * 0.1	wy      2010.8     initial version        
  */

#ifndef LIB_TIME_H
#define LIB_TIME_H

#include <time.h>
#include <sys/sysinfo.h>
#include "lib-base.h"

#define TIME_STAMP_ZERO   "0000-00-00 00:00:00"
typedef struct time{
    int hour;
    int min;
    int sec;
}nowtime;


long lib_time_get();

void lib_time_int2string(int sec, char *buf);
void lib_time_int2string_nodelm(int sec, char *buf);

int  lib_time_range_compare(char *start, char *stop, char *current);
int  lib_time_diff(char *start, char *stop);
void lib_get_time(char *buf,nowtime *timer);
void lib_time_init2string_delay_hour(int sec,char *buf);
void Sleep( long wait );

#define lib_time_current_stamp(BUF) lib_time_int2string(0, (BUF))

#endif

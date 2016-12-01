/*
 * Filename : lib_time.c
 * Copyright : All right reserved by SecWin, 2010
 * Time base function.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
#include "lib-timer.h"

inline long lib_time_get()
{
	struct sysinfo info;
	sysinfo(&info);
	return info.uptime;
}

void lib_time_int2string(int sec, char *buf)
{
	time_t tv;
	struct tm *tm = NULL;
		
	if (sec == 0) {
		time(&tv);
	}else{
		tv = (time_t)sec;
	}
	tm = localtime(&tv);
	sprintf(buf, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
	return;
}

void lib_time_int2string_nodelm(int sec, char *buf)
{
	time_t tv;
	struct tm *tm = NULL;
		
	if (sec == 0) {
		time(&tv);
	}else{
		tv = (time_t)sec;
	}
	tm = localtime(&tv);
	sprintf(buf, "%.4d%.2d%.2d%.2d%.2d%.2d",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
	return;
}



/* 
 * compare time range and current time.
 * @return:
 *         0 :  current time in start-stop time range
 *        -1 :  current < start
 *         1  :  current > stop
 * @input:
 *         all in date-time format "yyyy-mm-dd hh:mm:ss"
 */
int  lib_time_range_compare(char *start, char *stop, char *current)
{
    return 0;
}


/* 
 * Diff time and return the seconds 
 * @return:
 *             0   : equal
 *            -xx : start > stop
 *            +xx :start < stop
 * @input:
 *         all in date-time format "yyyy-mm-dd hh:mm:ss"
 */
int  lib_time_diff(char *start, char *stop)
{
    return 0;
}




int lib_is_time_zero(char *time)
{
    return !strncmp(time, TIME_STAMP_ZERO, strlen(TIME_STAMP_ZERO));
}

void lib_get_time(char *buf,nowtime *timer)
{
    struct tm *tm = NULL; 
    time_t tv;
    tv = time(NULL);
    tm = localtime(&tv);
    timer->hour = tm->tm_hour;
    timer->min = tm->tm_min;
    timer->sec = tm->tm_sec;
    sprintf(buf,"%.2d:%.2d:%.2d",tm->tm_hour, tm->tm_min, tm->tm_sec);
    
}

void lib_time_init2string_delay_hour(int sec,char *buf)
{
	time_t tv;
	struct tm *tm = NULL;
		
	if (sec == 0) {
		time(&tv);
	}else{
		tv = (time_t)sec;
	}
	tv=tv-3600;
	tm = localtime(&tv);
	sprintf(buf, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d",
			tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
			tm->tm_hour, tm->tm_min, tm->tm_sec);
	return;
}

UInt32 lib_time_string2int(char *timestr){
	struct tm tm_time;
	strptime(timestr, "%Y-%m-%d %H:%M:%S", &tm_time); 
	return	mktime(&tm_time);
}


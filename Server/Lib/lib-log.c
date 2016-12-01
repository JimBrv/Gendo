/*
 * Filename : lib-log.c
 * Copyright : All right reserved by SecWin, 2010
 * Log client interface.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
#include "lib-log.h"

static int disp = LOG_DISP_DB;
char ip[MAX_NAME_LEN];
int  port = 0;

static int disp_cgs = LOG_DISP_DB;
char ip_cgs[MAX_NAME_LEN];
int  port_cgs = 0;

typedef struct _log_id_fmt {
	int   module;
	int   id;
	int   level;
	int   param_cnt;
	char *param_fmt;
	char *param_info;
}log_id_fmt_t, *plog_id_fmt_t;

log_id_fmt_t log_param_fmt_ary[] = {
	//{MODULE_WOS,		LOG_WOS_ADD_USER_SPEEDUP_BILL_FAILD,	WARNING,	5, "%s %s %s %d %s",	"add user[user]  speedup bill faild,begin time[btime] end time[etime] speeduptime[speeduptime] reason[reason]"}, 
};  


int lib_log_init(char *svrip, int svrport, int dispatch)
{
	disp = dispatch;
	strcpy(ip, svrip);
	port = svrport;
	
	return OK;
}


int lib_cgs_log_init(char *svrip, int svrport, int dispatch)
{
	disp_cgs = dispatch;
	strcpy(ip_cgs, svrip);
	port_cgs = svrport;
	return OK;
}


void lib_log_finl(void)
{
	return;
}

#define LOG_ID_SANITY(ID)   ((ID) < SIZEOF(log_param_fmt_ary))
#define LOG_PARAM_SANITY(CNT, ID) ((CNT) <= MAX_LOG_ARGV && (CNT) == (log_param_fmt_ary[(ID)].param_cnt))

static inline log_id_fmt_t *get_fmt_from_id(int id)
{
	return &log_param_fmt_ary[id];
}


/* log interface for client */
int lib_log(int module, int id, ...)
{
	char log_buf[10240] = {0};
	msg_head_t *msg = (msg_head_t*)log_buf;
	log_head_t *lh = (log_head_t *)msg->payload;
	int ret = 0;
	int fd  = 0, intValue = 0;
	log_id_fmt_t *fmt = NULL;
	char *pfmt = NULL;
	char *pValue = NULL;
	va_list ap;

	/*
	memset(&lh, 0, sizeof(lh));
	lh->argv = (char **)&lh->argv;
	for(i = 0; i < MAX_LOG_ARGV; i++){
		lh->argv[i] = &lh->argv + i*MAX_NAME_LEN;
	}*/

	if (!LOG_ID_SANITY(id)) {
		return ERROR;
	}
	
	fmt = get_fmt_from_id(id);

	/* fill params apply format */
	pfmt = fmt->param_fmt;
	
	va_start(ap, id);
	while (*pfmt) {
		if (*pfmt != '%') {
			pfmt++;
			continue;
		}
		
		switch (*++pfmt) {
		case 's':
			pValue = (char *)va_arg(ap, const char *);
			strncpy(lh->argv[lh->argc++], pValue, 2*MAX_NAME_LEN);
			break;

		case 'd':
		case 'P':
			intValue = va_arg(ap, int);
			snprintf(lh->argv[lh->argc++], MAX_NAME_LEN, "%d",
				 intValue);
			break;
		default:
			break;
		}
		pfmt++;
	}
	va_end(ap);

	if (!LOG_PARAM_SANITY(lh->argc, id)) {
		lib_error(MODULE_UTL, "log param count is wrong, id=%d bug\n", lh->argc);
		return ERROR;
	}

	lh->module = fmt->module;
	lh->id     = fmt->id;
	lh->level  = fmt->level;
	lh->time   = time(NULL);
	lh->dispatch = disp;

	msg->msg  = MSG_LGS_LOG;
	msg->len  = sizeof(log_head_t);
	
	fd = lib_client(ip, port, SOCK_DGRAM);
	if (fd <= 0) {
		lib_error(MODULE_UTL, "create socket failed\n");
		return ERROR;
	}
	
	ret = lib_sendto_msg(fd, msg);
	if (ret <= 0) {
		lib_error(MODULE_UTL, "send log=%d to LGS=%s:%d failed\n", id, ip, port);
	}
	lib_close(fd);
	
	return OK;
}



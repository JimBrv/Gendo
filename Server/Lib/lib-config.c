/*
 * Filename : lib-config.c
 * Copyright : All right reserved by SecWin, 2010
 * config file support.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
 

#include "lib-debug.h"
#include "lib-config.h"


#define conf_lock_read(lock_fd)         lock_read(lock_fd, 1, 2)
#define conf_lock_write(lock_fd)        lock_write(lock_fd, 1, 2)
#define conf_unlock(lock_fd)            do { unlock(lock_fd, 1); close(lock_fd); } while (0)

static int conf_init_lock(char *file)
{
	return init_lock(file);
}

static struct config_pacos cfg_pacos[] = {
	/*lbs server IP:Port */
	{GAS_IP,            "gas_ip",             "127.0.0.1"},
	{GAS_PORT,          "gas_port",           "8888"},
    {GAS_DEBUG_PORT,    "gas_debug_port",     "8889"},
    {GAS_MAX_THREAD,    "gas_max_thread",     "8"},
    {GAS_MAX_CONN,      "gas_max_connect",    "1024"},
    {GAS_ID,            "gas_id",             "1"},   
	/* DB server info */
	{DBS_IP,            "db_server_ip",         "127.0.0.1"},
	{DBS_PORT,          "db_server_port",       "3306"},
	{DBS_DB_NAME,       "db_server_dbname",     "openvpn"},
	{DBS_DB_USER,		"db_server_user",    	"root"},
	{DBS_DB_PWD,		"db_server_pwd", 	    "root"},
	{DBS_DB_CONNECT,	"db_server_connect",	"20"},

	/* log server info */
	{LGS_IP,            "log_server_ip",        "127.0.0.1"},
	{LGS_PORT,          "log_server_port",      "8890"},

	/* AES key */
	{AES_CRYPT_STRING,  "AES_crypt_string",     "1234567890-=][po"},
};

int config_get(int id, char *result)
{
	int count, i, line;
	char buf[LINE_BUF_LEN], *p;
	FILE *fp;
	int lock_fd;

	lock_fd = conf_init_lock(CONFIG_LOCK_FILE);
	if (lock_fd < 0) {
		return ERROR;
	}

	conf_lock_write(lock_fd);

	count = sizeof (cfg_pacos) / sizeof (struct config_pacos);
	for (i = 0; i < count; i++) {
		if (cfg_pacos[i].id == id) {
			line = IsInFileField(CONFIG_FILE, cfg_pacos[i].name, 1);
			if (line) {
				fp = fopen(CONFIG_FILE, "r");
				if (!fp) {
					conf_unlock(lock_fd);
					return ERROR;
				}
				FileToLine(fp, line);
				p = GetLine(fp);
				fclose(fp);
				bzero(buf, sizeof (buf));
				vgetw(p, NULL, 0, buf, sizeof (buf));
				strcpy(result, buf);
				conf_unlock(lock_fd);
				return OK;
			} else {
				strcpy(result, cfg_pacos[i].default_val);
				conf_unlock(lock_fd);
				return OK;
			}
		}
	}
	conf_unlock(lock_fd);
	return ERROR;
}

int config_set(int id, char *fmt, ...)
{
	va_list args;
	int count, i, line, ret = ERROR;
	char vbuf[LINE_BUF_LEN];
	char buf[LINE_BUF_LEN];
	int lock_fd;

	lock_fd = conf_init_lock(CONFIG_LOCK_FILE);
	if (lock_fd < 0) {
		return ERROR;
	}

	conf_lock_write(lock_fd);

	va_start(args, fmt);
	vsprintf(vbuf, fmt, args);
	va_end(args);

	count = sizeof (cfg_pacos) / sizeof (struct config_pacos);
	for (i = 0; i < count; i++) {
		if (cfg_pacos[i].id == id) {
#if 0
			if (strchr(vbuf, ' '))
				sprintf(buf, "%s \"%s\"", cfg_pacos[i].name,
					vbuf);
			else
#endif
				sprintf(buf, "%s %s", cfg_pacos[i].name, vbuf);
			line = IsInFileField(CONFIG_FILE, cfg_pacos[i].name, 1);
			if (line)
				ret = ReplaceLine(CONFIG_FILE, buf, line);
			else
				ret = AppendLine(CONFIG_FILE, buf);
		}
	}
	if (ret == true)
		ret = OK;
	conf_unlock(lock_fd);
	return ret;
}

int config_remove(int id)
{
	int count, i, line, ret = ERROR;
	int lock_fd;

	lock_fd = conf_init_lock(CONFIG_LOCK_FILE);
	if (lock_fd < 0) {
		return ERROR;
	}

	conf_lock_write(lock_fd);

	count = sizeof (cfg_pacos) / sizeof (struct config_pacos);
	for (i = 0; i < count; i++) {
		if (cfg_pacos[i].id == id) {
			line = IsInFileField(CONFIG_FILE, cfg_pacos[i].name, 1);
			if (line) {
				ret = DeleteLine(CONFIG_FILE, line);
				break;
			}
		}
	}
	conf_unlock(lock_fd);
	if (ret == true)
		ret = OK;
	return ret;
}


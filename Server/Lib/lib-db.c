/*
 * Filename : lib-db.c
 * Copyright : All right reserved by SecWin, 2010
 * Simple list timer support.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
#include <unistd.h>
#include "lib-debug.h"
#include "lib-db.h"
#include "lib-msg.h"

static db_pool_t db_pool;
#define DB_NO_HANDLE_SLEEP_USEC 100*1000


/* 
  * Pay attention: it will block client if no handle is idle
  */
db_handle_t *_db_get_handle(char *file, int line)
{
	lib_list_node *pNode, *pNodeTp;
	db_handle_t *handle = NULL;
	bool found = false;
    int  try_cnt = 0;
	
	if (db_pool.count <= 0) {
		db_error(0, "db pool's count == 0!!!");
		return NULL;
	}
	
	sem_wait(&db_pool.sem);

//again:
	thd_lock(&db_pool.lock);
    lib_list_for_each_safe(pNode, pNodeTp, &(db_pool.handle_list)) {
		handle = lib_list_entry(pNode, db_handle_t, node);
		if (handle->state == DB_HANDLE_IDLE) {
			handle->state = DB_HANDLE_BUSY;
			strncpy(handle->file, file, MAX_FILE_COMPILE_PATH);
			handle->line = line;
			found = true;		
			break;
		}
    }
	thd_unlock(&db_pool.lock);
#if 0
    if (!found) {
        if ((try_cnt++) < 10) {
            /* wait for some release */
            usleep(DB_NO_HANDLE_SLEEP_USEC); // 100ms
            goto again;
        }
    }
#endif
	
	return (found ? handle : NULL);
}


void db_put_handle(db_handle_t *handle)
{
    thd_lock(&db_pool.lock);
	if (handle) {
		handle->state = DB_HANDLE_IDLE;
		memset(handle->file, 0, MAX_FILE_COMPILE_PATH);
		handle->line  = 0;
	}
	thd_unlock(&db_pool.lock);
	sem_post(&db_pool.sem);
	
	return;
}

static int db_connect_init(char *db_server, int db_port, char *db_user, char *db_pwd, char *db_name, MYSQL **mysql)
{
	char *set1 = "SET NAMES 'utf8'";
	char *set2 = "SET CHARACTER SET utf8";
	my_bool reconnect = 1;

	/* initialize connection handler */
	*mysql = mysql_init(NULL);
	if (*mysql == NULL) {
		db_error(0, "mysql_init() failed (probably out of memory)\n");
		return ERROR;
	}

	mysql_options(*mysql, MYSQL_OPT_RECONNECT, (char *)&reconnect);

	/* connect to server */
	if (mysql_real_connect(*mysql, db_server, db_user, db_pwd, db_name, db_port, NULL, 0) == NULL) 
	{
		db_error(*mysql, "mysql_real_connect() failed ");
		mysql_close(*mysql);
		return ERROR;
	}
	mysql_options(*mysql, MYSQL_OPT_RECONNECT, (const char *)&reconnect);

	mysql_real_query(*mysql, set1, strlen(set1));
	mysql_real_query(*mysql, set2, strlen(set2));
	return OK;
}


void db_connect_destroy(MYSQL **mysql)
{
	if (*mysql) mysql_close(*mysql);
}


void db_pool_dump(void)
{
	lib_list_node *pNode, *pNodeTp;
	db_handle_t *handle = NULL;
	int i = 0;
	
	thd_lock(&db_pool.lock);
	db_info("dump db pool now...\n");
	lib_list_for_each_safe(pNode, pNodeTp, &(db_pool.handle_list)) {
		handle = lib_list_entry(pNode, db_handle_t, node);
		db_info("db handle=[%d], state=[%s], caller=[%s:%d]\n",
			     i,
			     handle->state == DB_HANDLE_BUSY ? "busy" : "idle",
				 handle->file,
				 handle->line);
		i++;
	}
	db_info("db pool handle count=%d\n", db_pool.count);
	thd_unlock(&db_pool.lock);
}

int db_pool_init(char *db_server, int db_port, char *db_user, char *db_pwd, char *db_name, int count) 
{
	lib_list_node *pNode, *pNodeTp;
	db_handle_t *handle = NULL;
	int i;
	
	INIT_LIB_LIST(&(db_pool.handle_list));

	for (i = 0; i < count; i++) {
		handle = mem_malloc(sizeof(db_handle_t));
		if (!handle) {
			db_error(0, "malloc handle memory failed\n");
			goto err;
		}
		if (db_connect_init(db_server, db_port, db_user, db_pwd, db_name, &(handle->mysql))) {
			db_error(0, "create db connect failed\n");
			goto err;			
		}
		handle->state = DB_HANDLE_IDLE;
		lib_list_add(&(handle->node), &(db_pool.handle_list));
	}
	if (thd_lock_init(&(db_pool.lock), NULL)) {
		db_error(0, "init thread lock failed\n");
		goto err;					
	}
	if (sem_init(&(db_pool.sem), 0, count)) {
		db_error(0, "init sem failed\n");
		goto err;					
	}
	
	db_pool.count = count;
	return OK;
	
err:
	lib_list_for_each_safe(pNode, pNodeTp, &(db_pool.handle_list)) {
		handle = lib_list_entry(pNode, db_handle_t, node);
		lib_list_del(&(handle->node));
		if (handle->mysql) db_connect_destroy(&(handle->mysql));
		mem_free(handle);
	}
	INIT_LIB_LIST(&(db_pool.handle_list));
	return ERROR;		
}

int db_pool_finl(void)
{
    lib_list_node *pNode, *pNodeTp;
	db_handle_t *handle;

	thd_lock(&db_pool.lock);
	lib_list_for_each_safe(pNode, pNodeTp, &(db_pool.handle_list)) {
		handle = lib_list_entry(pNode, db_handle_t, node);
		if (handle->state == DB_HANDLE_IDLE) {
			lib_list_del(&(handle->node));
			if (handle->mysql) db_connect_destroy(&(handle->mysql));
			mem_free(handle);
		}else{
			lib_error(MODULE_UTL, "a db handle hold by [%s:%d], pool can't destoy it!", handle->file, handle->line);
			thd_unlock(&db_pool.lock);
			return ERROR;
		}
	}
    thd_unlock(&db_pool.lock);	
	
	INIT_LIB_LIST(&db_pool.handle_list);
	thd_lock_finl(&db_pool.lock);
	sem_destroy(&db_pool.sem);
	
	db_pool.count = 0;
	return OK;
}

#define HANDLE_CHECK(H)  ((H) && (H)->state == DB_HANDLE_BUSY && (H)->mysql)

/* db SQL query interface */
inline int db_query(db_handle_t *handle, char *sql, int len, pfn_db_query_cb cb, void *cb_param)
{
	int ret            = OK;
	MYSQL_RES *res_set = NULL;
	
	if (!HANDLE_CHECK(handle)) {
		db_error(0, "handle check failed!");
		return ERROR;
	}
	ret = mysql_real_query(handle->mysql, sql, len);
	if (ret) {
		db_error(handle->mysql, "mysql_real_query() failed:");
		db_error(0, sql);
		return ERROR;
	}
	res_set = mysql_store_result(handle->mysql);
	if (!res_set) {
		if (mysql_errno(handle->mysql) != 0) {
			db_error(handle->mysql, "mysql_real_query() failed:");
			db_error(0, sql);		
			return ERROR;
		}
	    /* query ok, and without result, like update, insert, create table, etc */
	}else{
	    /* query ok, with result, for select  etc */
		MYSQL_ROW row;
		int j = 0;
		while ((row = mysql_fetch_row(res_set)) != NULL) {
			/* get row content here */
			void *param = NULL;
			
		  	if (cb) cb (param, cb_param);
		    j++;
		}
		mysql_free_result(res_set);
	}
	
	return ret;
}


/* 
 * No handle pool support for DB
 *
 */
typedef struct _db_svr_info_ {
    char ip[MAX_NAME_LEN];
    int  port;
    char user[MAX_NAME_LEN];
    char pwd[MAX_NAME_LEN];
    char db[MAX_NAME_LEN];        
}db_instant_svr, pdb_instant_svr;

db_instant_svr instant_svr;

void db_set_instant_server(char *db_svr, int db_port, char *db_user, char *db_pwd, char *db_name)
{
    strcpy(instant_svr.ip, db_svr);
    strcpy(instant_svr.user, db_user);
    strcpy(instant_svr.pwd, db_pwd);
    strcpy(instant_svr.db, db_name);
    instant_svr.port = db_port;
}

db_handle_t *db_get_instant_handle(void)
{
    char *set1 = "SET NAMES 'utf8'";
    db_handle_t *handle = mem_malloc(sizeof(db_handle_t));
    assert(handle!=NULL);
    
    handle->mysql = mysql_init(NULL);
    if (handle->mysql == NULL) {
        db_error(0, "mysql_init() failed (probably out of memory)\n");
        mem_free(handle);
        return NULL;
    }

    if (mysql_real_connect(handle->mysql, 
        instant_svr.ip, 
        instant_svr.user, 
        instant_svr.pwd, 
        instant_svr.db, 
        instant_svr.port, 
        NULL,
        0) == NULL) 
    {
        db_error(handle->mysql, "mysql_real_connect() failed ");
        mysql_close(handle->mysql);
        mem_free(handle);
        return NULL;
    }

	mysql_real_query(handle->mysql, set1, strlen(set1));
    
    return handle;
}


void db_put_instant_handle(db_handle_t *handle)
{
    if (!handle && !handle->mysql) return;
    mysql_close(handle->mysql);
    mem_free(handle);
    return;
}


void db_set_handle_utf8(db_handle_t *handle)
{
    char *set1 = "SET NAMES 'utf8'";
    mysql_real_query(handle->mysql, set1, strlen(set1));
    return;
}
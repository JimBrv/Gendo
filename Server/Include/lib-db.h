/*
  * Header : lib-db.h
  * Copyright : All right reserved by SecWin, 2010
  * database handle and pool.
  * 
  * Ver   Author   Date         Desc
  * 0.1	wy      2010.8     initial version        
  */
  
#ifndef LIB_DB_H
#define LIB_DB_H

#define MAX_DB_HANDLE           32
#define DB_HANDLE_BUSY          1
#define DB_HANDLE_IDLE          0
#define MAX_DB_SQL_LEN          2048

#include <mysql/mysql.h>
#include <unistd.h>
#include <semaphore.h>

#include "lib-base.h"
#include "lib-thread.h"
#include "lib-list.h"
#include "lib-time.h"
#include "lib-debug.h"

#define MYSQL_DEFAULT_SERVER_PORT  3306
#define MYSQL_DEFAULT_USER         "root"
#define MYSQL_DEFAULT_USER_PWD     "root"         

typedef enum {
    DB_RET_OK = 0,
    DB_RET_NEED_MORE_DATA,
    DB_RET_SERVER_UNREACHEABLE,
    DB_RET_QUERY_ERROR,
}db_err_e;

#ifdef DBDEBUG
#define SQL_print(str)	printf("SQL: %s, %d\n",(str),strlen(str));
#else
#define SQL_print(str)
#endif

#define db_info(...)  lib_info(__VA_ARGS__)

#define db_error(sql_sock, msg) do { \
	fprintf(stderr, "DB Error : (%s:%d) %s\n", __FILE__, __LINE__, (msg)); \
	if ((sql_sock) != NULL) { \
		fprintf(stderr, "MySQL Error : %u (%s)\n", mysql_errno((sql_sock)), \
			mysql_error((sql_sock))); \
        } \
}while(0)

/* db query call back */
typedef void (*pfn_db_query_cb)(void *db_param, void *cb_param);


/*database pool*/
typedef struct _db_handle_s {
    lib_list_node node;
    int          state;
	MYSQL       *mysql;
	char         sql[MAX_DB_SQL_LEN];
	char         file[MAX_FILE_COMPILE_PATH];
	int          line;
}db_handle_t, *pdb_handle_t;

typedef struct _db_pool_s {
	int         count;	
	THD_LOCK_T  lock;
	sem_t       sem;
	lib_list_node   handle_list;
}db_pool_t, *pdb_pool_t;

#define db_get_handle() _db_get_handle(__FILE__, __LINE__)


/* db pool managment interface */
db_handle_t *_db_get_handle(char *file, int line);

void db_put_handle(db_handle_t *handle);

int db_pool_init(char *db_server, int db_port, char *db_user, char *db_pwd, char *db_name, int count);

int db_pool_finl(void);

/* db SQL query interface */
int db_query(db_handle_t *handle, char *sql, int len, pfn_db_query_cb cb, void *cb_param);

/* db INSTANT handler */
db_handle_t *db_get_instant_handle(void);
void         db_put_instant_handle(db_handle_t *handle);
void         db_set_instant_server(char *db_svr, int db_port, char *db_user, char *db_pwd, char *db_name);


/* set mysql db connection to utf8, for reconnecting will lose the utf8 setting */
void db_set_handle_utf8(db_handle_t *handle);


#endif

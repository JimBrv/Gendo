/* 
 * opertation with DB interface
 * 
 * gendo, 2013.3, all right reserved
 */

#include "lib-db.h"
#include "gas-user.h"


bool gas_db_user_auth(char *name, char *pwd)
{
    pdb_handle_t handle = db_get_instant_handle();
    if (!handle) return false;
   	MYSQL_RES *res_set = NULL;
    int   ret = 0, len = 0, count = 0;
    char  sql[512] = {0};
    char *fmt = "select count(*) from user where username='%s' and password='%s'";
    len = sprintf(sql, fmt, name, pwd);

    db_set_handle_utf8(handle);
    
	ret = mysql_real_query(handle->mysql, sql, len);
	if (ret) {
		db_error(handle->mysql, "mysql_real_query() failed:");
		db_error(0, sql);
        goto out;
	}
	res_set = mysql_store_result(handle->mysql);
	if (!res_set) {
		if (mysql_errno(handle->mysql) != 0) {
			db_error(handle->mysql, "mysql_real_query() failed:");
			db_error(0, sql);		
			goto out;
		}
	    /* query ok, and without result, like update, insert, create table, etc */
	}else{
	    /* query ok, with result, for select  etc */
		MYSQL_ROW row = mysql_fetch_row(res_set);
        if (row) {
            count = atoi(row[0]);
		}
		mysql_free_result(res_set);
	}
out:
    if (handle) db_put_instant_handle(handle);
    return (count == 1 ? true : false);
}



/*
 *
 * For mysql charset issue, why does the handle pool
 * cannot get utf8 chinese correct.
 * And it works with the upper function.
 * Its really a shit bug.
 * Ubuntu mysql-5.5 fixed it.
 * Debian still has problem
 *
 */
int gas_db_user_info(char *name, puser_t user)
{
    pdb_handle_t handle = db_get_handle();
    if (!handle) return false;
    MYSQL_RES *res_set = NULL;
    int   ret = 0, len = 0;
    char  sql[512] = {0};
    char *fmt = "select id,username, password,active,creation, name,email, \
		       note, quota_cycle,quota_bytes,quota_used,quota_name,\
		       quota_expire,history_bytes,enabled,level, quota_start, hobby,login_cnt, type from user where username='%s'";
    len = sprintf(sql, fmt, name); 

    db_set_handle_utf8(handle);
    ret = mysql_real_query(handle->mysql, sql, strlen(sql));
    if (ret) {
        db_error(handle->mysql, "mysql_real_query() failed:");
        db_error(0, sql);
        ret = ERROR;
        goto out;
    }
    res_set = mysql_store_result(handle->mysql);
    if (!res_set) {
        if (mysql_errno(handle->mysql) != 0) {
            db_error(handle->mysql, "mysql_real_query() failed:");
            db_error(0, sql);
            ret = ERROR;
            goto out;
        }
    }else{
        /* query ok, with result, for select  etc */
        MYSQL_ROW row = mysql_fetch_row(res_set);
        assert(row != NULL);
        user->id = atoi(row[0]);
        snprintf(user->name, MAX_NAME_LEN, "%s", row[1]);
        snprintf(user->pwd, MAX_NAME_LEN,  "%s",row[2]);
        user->active = atoi(row[3]);
        strcpy(user->reg_time, row[4]);
        snprintf(user->nick, MAX_NAME_LEN,  "%s",row[5]);
        strcpy(user->email, row[6]);
        snprintf(user->note, MAX_NAME_LEN*4, "%s", row[7]);
        user->quota_cycle = atoi(row[8]);
        user->quota_bytes = atoll(row[9]);
        user->quota_used  = atoll(row[10]);
        snprintf(user->quota_name, MAX_NAME_LEN*4,  "%s",row[11]);  
       
        strcpy(user->quota_expire, row[12]);
        user->history_bytes = atoll(row[13]);
        user->enable = atoi(row[14]);
        user->level  = atoi(row[15]);

       strcpy(user->quota_start, row[16]);
       user->hobby     = atoi(row[17]);
       user->login_cnt = atoi(row[18]);
       user->type        = atoi(row[19]);
        
        mysql_free_result(res_set);
    }
    
out:
    if (handle) db_put_handle(handle);
    return (ret ? ERROR : OK);
}


void gas_db_user_update_lastlogin(char *name)
{
    pdb_handle_t handle = db_get_instant_handle();
    if (!handle) return;
    int   ret = 0, len = 0;
    char  sql[512] = {0};
    char *fmt = "update user set login_cnt=login_cnt+1, login_latest=CURRENT_TIMESTAMP where username='%s';";
    len = sprintf(sql, fmt, name);

    db_set_handle_utf8(handle);
    
	ret = mysql_real_query(handle->mysql, sql, len);
	if (ret) {
		db_error(handle->mysql, "mysql_real_query() failed:");
		db_error(0, sql);
        goto out;
	}
out:
    if (handle) db_put_instant_handle(handle);
    return;
}


int gas_db_server_info(int type, pserver_list_t svrlist)
{
    pdb_handle_t handle = db_get_handle();
    if (!handle) return false;
    MYSQL_RES *res_set = NULL;
    int   ret = 0, len = 0;
    char  sql[512] = {0};
    char *fmt = "select * from server where type=%d limit 0,30";   // 30 node is happy for me :)

    if (type == TYPE_ALL) {
        len = sprintf(sql, "%s", "select * from server limit 0,30");
    }else{
        len = sprintf(sql, fmt, type);
    }
    

    db_set_handle_utf8(handle);

    ret = mysql_real_query(handle->mysql, sql, len);
    if (ret) {
        db_error(handle->mysql, "mysql_real_query() failed:");
        db_error(0, sql);
        ret = ERROR;
        goto out;
    }
    res_set = mysql_store_result(handle->mysql);
    if (!res_set) {
        if (mysql_errno(handle->mysql) != 0) {
            db_error(handle->mysql, "mysql_real_query() failed:");
            db_error(0, sql);
            ret = ERROR;
            goto out;
        }
    }else{
        /* query ok, with result, for select  etc */
        MYSQL_ROW row;
        int i = 0;
        while ((row = mysql_fetch_row(res_set)) != NULL) {
            pserver_t svr = &svrlist->svr_ary[i];
            svr->id = atoi(row[0]);
            strcpy(svr->ip, row[1]);
            snprintf(svr->name, MAX_NAME_LEN,   "%s", row[2]);
            snprintf(svr->desc, MAX_NAME_LEN*2, "%s", row[3]);
            svr->protocol = atoi(row[4]);
            svr->latency  = atoi(row[5]);
            svr->cur_user = atoi(row[6]);
            svr->max_user = atoi(row[7]);
            svr->state    = atoi(row[8]);
            strcpy(svr->ssl_port, row[9]);
            svr->type     = atoi(row[10]);
            snprintf(svr->location, MAX_NAME_LEN/2, "%s", row[11]);
            i++;
        }
        svrlist->svr_cnt = i;
        mysql_free_result(res_set);
    }
    
out:
    if (handle) db_put_handle(handle);
    return (ret ? ERROR : OK);
}



void gas_dump_server_info(int type, pserver_list_t svrlist)
{
    int i = 0;
    int ret = gas_db_server_info(type, svrlist);
    if (ret) {
        ras_error("Dump server error!\n");
        return;
    }
    ras_info("Dump server list:\n");
    for (i = 0; i < svrlist->svr_cnt; i++) {
        pserver_t svr = &svrlist->svr_ary[i];
        ras_info("svr[%d]: ip='%s', name='%s', desc='%s', typ='%d', location='%s', capacity='%d/%d', ssl-port='%s'\n",
                 svr->id, svr->ip, svr->name, svr->desc, svr->type, svr->location, svr->cur_user,
                 svr->max_user, svr->ssl_port);
    }
    ras_info("Dump server over!\n");
}
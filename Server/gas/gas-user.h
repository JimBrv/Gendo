/*
 * Header : gas-user.h
 * Copyright : All right reserved by Gendo team
 *
 * wy, 2013.3
 */


#ifndef GAS_USER_H
#define GAS_USER_H

#include "lib-msg.h"
#include "gas.h"

bool gas_db_user_auth(char *name, char *pwd);
int  gas_db_user_info(char *name, puser_t user);
int  gas_db_server_info(int type, pserver_list_t svrlist);
void gas_dump_server_info(int type, pserver_list_t svrlist);
void gas_db_user_update_lastlogin(char *name);


#endif

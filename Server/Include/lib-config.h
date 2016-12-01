/*
 * Header : lib-config.h
 * Copyright : All right reserved by SecWin, 2010
 * config file header.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */


#ifndef LIB_CONFIG_H
#define LIB_CONFIG_H

#include "lib-base.h"
#include "lib-types.h"
#include "lib-file.h"

#define INVALID_VALUE		          "---"
#define DEFAULT_USER_NUMBER_STR	      "5"
#define DEFAULT_INTERFACE_NUMBER_STR  "4"
#define DEFAULT_CPU_NUMBER_STR	      "1"
#define DEFAULT_ACC_TYPE_STR	      "None"
#define DEFAULT_LICENSE_EXPIRE_STR	  "-1"
#define DEFAULT_LICENSE_FUNCS_STR	  "FFFFFFFF"

#define CONFIG_FILE		    "/gendo/conf/config.sys"

#if 0  /* Use one config file in current version */
#define LBS_CONFIG_FILE	    "/rop/conf/config.lbs"
#define COS_CONFIG_FILE	    "/rop/conf/config.cos"
#define LGS_CONFIG_FILE		"/rop/conf/config.lgs"
#define CLT_CONFIG_FILE	    "/rop/conf/config.clt"
#define SYSLOG_CONFIG_FILE	"/rop/conf/config.syslog"
#endif

#define CONFIG_LOCK_FILE	    "/tmp/config.lock"


#define IP_ADDRESS_MANUAL	1
#define IP_ADDRESS_DHCP		2

#define INTF_INTERNAL		1
#define INTF_EXTERNAL		2
#define INTF_HA			3

#define ROUTE_ADD	1
#define ROUTE_DEL	2

#define SERVICE_SSH	1
#define SERVICE_FORWARD	2
#define SERVICE_SNMPD	3

#define SERVICE_START	1
#define SERVICE_STOP	2

/* for config file */
struct config_pacos {
	int   id;
	char *name;
	char *default_val;
};

enum config_type {
	/* gas server config */
    GAS_IP,
    GAS_PORT,
    GAS_DEBUG_PORT,
    GAS_MAX_THREAD,
    GAS_MAX_CONN,
    GAS_ID,
    
	/* db server config */
	DBS_IP,    
	DBS_PORT,
	DBS_DB_NAME,
	DBS_DB_USER,
	DBS_DB_PWD,
	DBS_DB_CONNECT,
	
	/* log server config */
	LGS_IP,    
	LGS_PORT,

	/* ticket en/decrypt key */
	AES_CRYPT_STRING,
};

int config_set(int id, char *fmt, ...);
int config_get(int id, char *result);
int config_remove(int id);


#endif

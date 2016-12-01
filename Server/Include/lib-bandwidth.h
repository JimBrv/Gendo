/*
 * Header : lib-bandwidth.h
 * Copyright : All right reserved by SecWin, 2010
 * bits operation
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
 
#ifndef LIB_BANDWIDTH_H
#define LIB_BANDWIDTH_H

//#include "lib-base.h"
#include "lib-msg.h"


#define BW_SANITY(BW) ((BW) == BANDWIDTH_GROUP_DEFAULT          \
                       || (BW) == BANDWIDTH_GROUP_GAME          \
                       || (BW) == BANDWIDTH_GROUP_DOWNLOAD      \
                       || (BW) == BANDWIDTH_GROUP_MOVIE)

/* 
  * BW map for 1M/2M to desc and private value
  */
typedef struct _bw_map_s {
    int    bw;     
    char  *desc;
    char  *value;
}bw_map_t, *pbw_map_t;


typedef enum {
    USER_BW_STATE_DEFAULT = 0,
    USER_BW_STATE_DOWNLOAD,
    USER_BW_STATE_GAME,
    USER_BW_STATE_MOVIE,   
}USER_BW_STATE_INDEX_E;


/* convert bandwidth to cms user state */
#define bw_grp_to_state(GRP, ST) \
do { \
        switch(GRP) { \
        case BANDWIDTH_GROUP_DEFAULT: \
            (ST) = USER_BW_STATE_DEFAULT; \
            break; \
        case BANDWIDTH_GROUP_GAME:\
            (ST)  = USER_BW_STATE_GAME;\
            break; \
        case BANDWIDTH_GROUP_DOWNLOAD: \
            (ST)  = USER_BW_STATE_DOWNLOAD; \
            break; \
        case BANDWIDTH_GROUP_MOVIE: \
            (ST)  = USER_BW_STATE_MOVIE; \
            break; \
        default: \
            (ST) = USER_BW_STATE_DEFAULT; \
            break; \
    }   \
}while(0)


/* helper function */
char* lib_bw_get_name(bandwidth_group_e bw, char *bw_value);

int   lib_bw_get_credit(bandwidth_group_e bw);
int   lib_bw_get_group(char *name);
int   lib_bw_get_dft_grp_desc(bandwidth_group_e bw, char *bw_value, char *desc);
char* lib_bw_get_speedup_grp_desc(bandwidth_group_e bw);

int   lib_bw_get_speed_desc(int speed, char *desc);
int   lib_bw_get_default_desc(int speed, char *desc);

int   lib_bw_get_speed_by_value(char * vlaue);

bw_map_t *lib_bw_get_by_speed(int speed);
char *lib_get_serviceid_value(char* svr_id);

int lib_get_tag_index(char* value, int num);

char *lib_bw_get_value_by_desc(char *desc);


bw_map_t *lib_bw_get_info_by_value(char *value);

int lib_bw_byte2mega(int speed_byte);
char* lib_bw_mega2value(int mega);


/* Get 1M plus according base */
int       lib_bw_1M_plus(int base_speed);
bw_map_t *lib_bw_get_1M_plus(int base_speed);
bw_map_t *lib_bw_get_4M();
int       lib_bw_auto_plus(int base_speed);
bw_map_t *lib_bw_get_auto_plus(int base_speed);

/* 2-4, 4-5, 5-6M plus */
bw_map_t *lib_gw_get_2to4_4to5(int base_speed);


/* CQ unicom bw value */
#if 0
#define BW_1K_VALUE      "grp_id=4497,treat_level=3560,down_speed=1024"
#define BW_256K_VALUE    "grp_id=4500,treat_level=3562,down_speed=262144"
#define BW_512K_VALUE    "grp_id=4495,treat_level=3558,down_speed=524288"
#define BW_1M_VALUE      "grp_id=4490,treat_level=3553,down_speed=1048576"
#define BW_1_5M_VALUE    "grp_id=4501,treat_level=3563,down_speed=1572864"
#define BW_2M_VALUE      "grp_id=4491,treat_level=3554,down_speed=2097152"
#define BW_2_5M_VALUE    "grp_id=4502,treat_level=3564,down_speed=2621440"
#define BW_3M_VALUE      "grp_id=4492,treat_level=3555,down_speed=3145728"
#define BW_3_5M_VALUE    "grp_id=4506,treat_level=3567,down_speed=3670016"
#define BW_4M_VALUE      "grp_id=4493,treat_level=3556,down_speed=4194304"
#define BW_5M_VALUE      "grp_id=4503,treat_level=3565,down_speed=5242880"
#define BW_6M_VALUE      "grp_id=0,treat_level=0,down_speed=0"                  // not support
#define BW_7M_VALUE      "grp_id=0,treat_level=0,down_speed=0"
#define BW_8M_VALUE      "grp_id=4494,treat_level=3557,down_speed=8388608"
#define BW_9M_VALUE      "grp_id=0,treat_level=0,down_speed=0"
#define BW_10M_VALUE     "grp_id=4504,treat_level=3566,down_speed=10485760"
#endif
#if 0
/* HuNan Unicom bw value */
#define BW_1K_VALUE      "svr_id=128"
#define BW_256K_VALUE    "svr_id=256"
#define BW_512K_VALUE    "svr_id=512"
#define BW_1M_VALUE      "svr_id=1024"
#define BW_1_5M_VALUE    "svr_id=1536"
#define BW_2M_VALUE      "svr_id=2048"
#define BW_2_5M_VALUE    "svr_id=2560"
#define BW_3M_VALUE      "svr_id=3072"
#define BW_3_5M_VALUE    "svr_id=3584"
#define BW_4M_VALUE      "svr_id=4096"
#define BW_5M_VALUE      "svr_id=5120"
#define BW_6M_VALUE      "svr_id=6144"                 // not support
#define BW_7M_VALUE      "svr_id=7168"
#define BW_8M_VALUE      "svr_id=8192"
#define BW_9M_VALUE      "svr_id=9216"
#define BW_10M_VALUE     "svr_id=1000"

#define BW_12M_VALUE     "svr_id=1200"
#define BW_20M_VALUE     "svr_id=2000"
#define BW_50M_VALUE     "svr_id=5000"
#define BW_100M_VALUE    "svr_id=6000"
#define BW_1000M_VALUE   "svr_id=9999"
#endif
/* SiChuan aipu bw value */
#define BW_1K_VALUE      "bw_value=128"
#define BW_256K_VALUE    "bw_value=256"
#define BW_512K_VALUE    "bw_value=512"
#define BW_1M_VALUE      "bw_value=1"
#define BW_1_5M_VALUE    "bw_value=1536"
#define BW_2M_VALUE      "bw_value=2"
#define BW_2_5M_VALUE    "bw_value=2560"
#define BW_3M_VALUE      "bw_value=3"
#define BW_3_5M_VALUE    "bw_value=3584"
#define BW_4M_VALUE      "bw_value=4"
#define BW_5M_VALUE      "bw_value=5"
#define BW_6M_VALUE      "bw_value=6"
#define BW_7M_VALUE      "bw_value=7"
#define BW_8M_VALUE      "bw_value=8"
#define BW_9M_VALUE      "bw_value=9"
#define BW_10M_VALUE     "bw_value=10"
#define BW_12M_VALUE     "bw_value=12"
#define BW_20M_VALUE     "bw_value=20"
#define BW_50M_VALUE     "bw_value=50"
#define BW_100M_VALUE    "bw_value=100"
#define BW_1000M_VALUE   "bw_value=1000"

#endif

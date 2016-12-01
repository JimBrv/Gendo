/*
 * Header : lib-reference.h
 * Copyright : All right reserved by SecWin, 2010
 * 
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */
 
#ifndef LIB_REFERENCE_H
#define LIB_REFERENCE_H

#include "lib-base.h"

typedef struct _lib_ref_s_ {
    int  ref;
    int  ref_history;
    char ref_file[MAX_NAME_LEN*3];
    int  ref_line;
}lib_ref_t, *plib_ref_t;

#define lib_ref_inc(REF)         {(REF)->ref++;snprintf((REF)->ref_file, MAX_NAME_LEN*3 - 1, "%s", __FILE__);(REF)->ref_line = __LINE__;}
#define lib_ref_dec(REF)         {(REF)->ref--;/*(REF)->ref_file[0]='\0';(REF)->ref_line=0;*/}
#define lib_is_refed(REF)        (((REF)->ref) > 0)
#define lib_ref_history_inc(REF) ((REF)->ref_history++)
#define lib_ref_inif(REF)        (memset((REF), 0, sizeof(lib_ref_t))

#endif


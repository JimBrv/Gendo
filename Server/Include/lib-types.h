/*
 * Header : lib-types.h
 * Copyright : All right reserved by SecWin, 2010
 * Base type definition
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */


#ifndef LIB_TYPES_H
#define LIB_TYPES_H

#include <sys/types.h>


#ifdef __cplusplus
extern "C" {
#endif
//#define WIN32
#ifdef WIN32

#pragma pack(4)    // Make structure consistent with server
/* windows types */
typedef unsigned char    UChar; 
typedef __int8  Int8,  *pInt8;
typedef __int16 Int16, *pInt16;
typedef __int32 Int32, *pInt32;
typedef __int64 Int64, *pInt64;

typedef unsigned __int8  UInt8,  *pUInt8;
typedef unsigned __int16 UInt16, *pUInt16;
typedef unsigned __int32 UInt32, *pUInt32;
typedef unsigned __int64 UInt64, *pUInt64;

#define false (0)
#define true  (1)

#else
// other OS types

#define OK     0
#define ERROR -1

#include <stdbool.h>

typedef unsigned char   UChar; 
typedef char    Int8,  *pInt8;
typedef int16_t Int16, *pInt16;
typedef int32_t Int32, *pInt32;
typedef int64_t Int64, *pInt64;

typedef unsigned char  UInt8, *pUInt8;
typedef u_int16_t UInt16, *pUInt16;
typedef u_int32_t UInt32, *pUInt32;
typedef u_int64_t UInt64, *pUInt64;

//typedef int bool;

#endif



#ifdef __cplusplus
}
#endif

#endif

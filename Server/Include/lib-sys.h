/*
 * Header : lib-sys.h
 * Copyright : All right reserved by SecWin, 2010
 * Linux system support interfaces.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.10     initial version        
 */
#ifndef LIB_SYS_H
#define LIB_SYS_H

#define _GNU_SOURCE

#include "lib-types.h"
#include "lib-debug.h"

#include <sched.h>



/* get cpu cores */
inline int sys_cpu_get(void);

/* bind process to cpu */
inline int sys_cpu_bind(int cpu, int process);

/* unbind process to cpu */
inline int sys_cpu_unbind(int cpu, int process);

#endif

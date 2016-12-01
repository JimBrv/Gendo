/*
 * Filename : lib-sys.c
 * Copyright : All right reserved by SecWin, 2010
 * Linux system misc interface support
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.10     initial version        
 */
#include "lib-sys.h"


inline int sys_cpu_get(void)
{
    return sysconf(_SC_NPROCESSORS_CONF);
}

/* bind process to cpu */
inline int sys_cpu_bind(int cpu, int process)
{
    cpu_set_t mask;
    CPU_ZERO(&mask);
    CPU_SET(cpu, &mask);
    if (sched_setaffinity(process, sizeof(mask), &mask) == -1) {
        lib_error(MODULE_UTL, "can't bind process=%d to cpu=%d\n",
                  process, cpu);
        return ERROR;
    }
    return OK;
}

/* unbind process to cpu */
inline int sys_cpu_unbind(int cpu, int process)
{
    cpu_set_t mask;
    sched_getaffinity(process, sizeof(mask), &mask);
    if (CPU_ISSET(cpu, &mask)) {
        CPU_CLR(cpu, &mask);
    }
    sched_setaffinity(process, sizeof(mask), &mask);
    return OK;
}


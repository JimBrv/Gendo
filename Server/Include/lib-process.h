/*
  * Header : lib-process.h
  * Copyright : All right reserved by Extelecom, 2011
  * list based timer support. Maybe use min-heap based timer for performance.
  *      
  */

#ifndef LIB_PROCESS_H
#define LIB_PROCESS_H

#include "lib-base.h"
#include "lib-types.h"

int lib_create_process(char *path, char *argv);


#endif

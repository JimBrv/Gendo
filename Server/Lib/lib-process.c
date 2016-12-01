/*
 * Filename : lib_time.c
 * Copyright : All right reserved by Extelecom, 2011
 * Time base function.    
 */
#include "lib-process.h"
#include "lib-msg.h"
#include "lib-debug.h"

int lib_exec_process(char *process, char *param)
{
    char *argv[3];
	char  process_path[MAX_NAME_LEN] = {0};
	char  process_param[MAX_NAME_LEN]= {0};
    if (!process) {
        lib_error(MODULE_UTL, "exec process is NULL!\n");
        return ERROR;
    }

	lib_info("Starting process '%s'...\n", process);

	sprintf(process_path,  "%s", process);
    if (param) {
	    sprintf(process_param, "%s", param);
    }
    
	argv[0] = process_path;
	argv[1] = process_param;
	argv[2] = NULL;
	execv((char *const)process_path, (char **const)argv);
	lib_error(MODULE_UTL, "failed to execv '%s'\n", process_path); 
    exit(0);
    return ERROR;
}

int lib_create_process(char *process, char *param)
{
    /* ignore child incase <defunc> */
    signal(SIGCHLD, SIG_IGN);  

    pid_t pid;
    pid = fork();
    
	if (pid < 0) {
		lib_error(MODULE_UTL, "fork() failed\n");
        return ERROR;
	} else if (pid == 0) { 
	    /* child */
		return lib_exec_process(process, param);
	} else {   
	    /* self process, parent */
        lib_info("create child process pid=%d OK!\n", pid);
        return OK;
	}
}



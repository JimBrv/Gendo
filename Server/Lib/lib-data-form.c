#include "lib-data-form.h"

int lib_data_form_verify(char *pattern, char* value){
    int ret = ERROR;
    if(pattern == NULL || value == NULL){
        return ERROR;
    }
    char tpm[100]={0},ebuf[1024]={0}; 
    regex_t reg;
    regmatch_t pmatch[1];
    int result=0;
    if (regcomp(&reg, pattern,0)){
        lib_error(MODULE_UTL,"parameter format is not correct\n");
        return ERROR;
    }
    if (result = regexec(&reg,  value, 1, pmatch, 0)!= 0){
        regerror(result, &reg, ebuf, sizeof(ebuf));
        lib_error(MODULE_UTL,"parameter is not correct:%s,value is[%s]\n",ebuf,value);
        goto out;
    }
    
    ret = OK;
out:
    regfree(&reg);
    return ret;
}

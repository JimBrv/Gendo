#ifndef 	LIB_REGEX_H_
#define	LIB_REGEX_H_
#include <stdio.h>
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <string.h>

#define ERRBUF 128

char err[ERRBUF];


int lib_regcomp_regex(regex_t *reg,char *pattern);

int lib_result_regex(regex_t *reg,size_t  nmatch,regmatch_t *match,char *str);

int lib_free_regex(regex_t *reg);

int lib_get_attribut_value(char *dst,char *src,char *parttern,int startindex);


#endif

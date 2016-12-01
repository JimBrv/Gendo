#include "lib-regex.h"


int lib_regcomp_regex(regex_t *reg,char *pattern){
	int  resulta = -1;
	if(!pattern||!pattern){
		 return -1;
	}
         resulta = regcomp(reg, pattern, REG_EXTENDED | REG_NEWLINE);
        if(resulta != 0)
        {
                regerror(resulta, reg, err, sizeof(err));
	      return -1;
        }
out:
	return resulta;
}

int lib_result_regex(regex_t *reg,size_t  nmatch,regmatch_t *match,char *str){
	 int   resultb = -1;
	if(!reg||!match||!str){
		return -1;
	}
          resultb = regexec(reg, str, nmatch, match, 0);
        if(resultb != 0)
        {
                 regerror(resultb, reg, err, sizeof(err));
	        resultb = -1;
        }
        return resultb;
}


int lib_free_regex(regex_t *reg){
	if(reg){
		regfree(reg);
	}
	return 0;
}

int  lib_get_attribut_value(char *dst,char *src,char *parttern,int startindex){
	    regex_t reg ;
	    regmatch_t match[1];	
	    int ret  = 0;
	    if(lib_regcomp_regex(&reg,parttern)){
		ret = -1;
		goto out;
	   }
	    if(lib_result_regex(&reg,1,match,src)){
		ret = -1;
		goto out;
	   }
	    if(match[0].rm_eo-match[0].rm_so-startindex-1<=0){
		goto out;
	   }
	    if(match[0].rm_eo-match[0].rm_so>0){
	  	  memcpy(dst,src+match[0].rm_so+startindex,match[0].rm_eo-match[0].rm_so-startindex-1);
	   }
out:
	  regfree(&reg);
            return ret;
}



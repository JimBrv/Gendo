
#include <iconv.h>
#include "lib-string.h"
#include "lib-msg.h"

/* make sure each dst ary item has enough memory:)  */
int lib_string_split(char *src, char split, char **dst, int *n)
{
	int i = 0;
	char *start = NULL, *end = NULL, *mid = NULL;

	if (!src || !split || !dst || !n) return -1;
	start = src;
	end   = src + strlen(src);
	
	while(*start != 0 && (mid = strchr(start, split)) != NULL) {
		if (*n < i) return -1;
		memcpy(dst[i++], start, (int)(mid - start));
		start = ++mid;
	}
	
	if (*start != 0) {
		memcpy(dst[i++], start, (int)(end - start));
	}
	
	*n = i;
	return 0;
}

int lib_name_sanity(char *name)
{
    int len = 0, cur = 0;
	if (!name) return -1;
	len = strlen(name);
	if (len <= 0 || len >= MAX_NAME_LEN)
		return -1;
	while(cur < len) {
		if (!isalnum(name[cur])) {
			return -1;
		}
		cur++;
	}
	return 0;    
}


int lib_code_convert(char *from_charset,char *to_charset,char *inbuf,int inlen,char *outbuf,int outlen)
{
    iconv_t cd;
    int rc;
    char **pin = &inbuf;
    char **pout = &outbuf;

    cd = iconv_open(to_charset,from_charset);
    if (cd == 0) return -1;
    memset(outbuf,0,outlen);
    size_t in_buf  = inlen;
    size_t out_buf = outlen;
    rc = iconv(cd,pin,&in_buf,pout,&out_buf);
    if (rc == -1) {
        switch(errno) {
        case E2BIG:
            lib_error(MODULE_UTL, "iconv E2BIG\n");
        break;
        case EILSEQ:
            lib_error(MODULE_UTL, "iconv EILSEQ\n");
        break;
        case EINVAL:            
            lib_error(MODULE_UTL, "iconv EINVAL\n");
        break;
        }
        lib_error(MODULE_UTL, "iconv errno=%d\n", errno);
    }
    iconv_close(cd);
    return rc > 0 ? 0 : -1;
}

int lib_u2g(char *inbuf,int inlen,char *outbuf,int outlen)
{
    return lib_code_convert((char *)"utf-8",(char *)"gb2312",inbuf,inlen,outbuf,outlen);
}
int lib_g2u(char *inbuf,size_t inlen,char *outbuf,size_t outlen)
{
    return lib_code_convert((char *)"gb2312",(char *)"utf-8",inbuf,inlen,outbuf,outlen);
}

int lib_a2u(char *inbuf,int inlen,char *outbuf,int outlen)
{
    return lib_code_convert((char *)"ascii",(char *)"utf-8",inbuf,inlen,outbuf,outlen);
}



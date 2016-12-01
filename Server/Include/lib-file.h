/*
 * Header : lib-file.h
 * Copyright : All right reserved by SecWin, 2010
 * file operation wrappers.
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */


#ifndef LIB_FILE_H
#define LIB_FILE_H

#include "lib-base.h"
#include "lib-types.h"

#ifndef BOOL
#	define BOOL	int
#endif

#ifndef true
#	define true 	1
#	define false 	0
#endif

#ifndef TRUE
#	define TRUE 	1
#	define FALSE	0
#endif

#ifndef PATH_MAX
#	define PATH_MAX	256
#endif

#define LINE_BUF_LEN		8192
#define LINE_LIMIT_LEN		8000

#define max(x, y)	((x) > (y) ? (x) : (y))
#define min(x, y)	((x) < (y) ? (x) : (y))
#define get_field	GetField
#define DELETEENTER(s) while(s[strlen(s)-1] == '\n' || s[strlen(s)-1] == '\r') s[strlen(s)-1] = 0;

BOOL IsSpace(int c);
BOOL IsEOL(int c);
BOOL IsNull(char *str);
BOOL IsNoteLine(char *line);
BOOL FileExist(char *fname);

/*
BOOL LockToRead(int fd);
BOOL Unlock(int fd);
BOOL LockToWrite(int fd);
*/

/* define for timeout */
#define	WAIT_FOREVER	-1
#define	NO_WAIT		0
/* about file lock, timeout is seconds */
int init_lock(char *lock_file);

int lock_byte(int fd, int type, uint offset, int timeout);
#define	lock_read(fd, offset, timeout)	lock_byte(fd, F_RDLCK, offset, timeout)
#define	lock_write(fd, offset, timeout)	lock_byte(fd, F_WRLCK, offset, timeout)
#define	unlock(fd, offset)		lock_byte(fd, F_UNLCK, offset, NO_WAIT)

/*
* move file point to line
*/
BOOL FileToLine(FILE * fp, int line);

/*
* default seperator char is ' ' and '\t'
*/
char *GetField(char *line, char *spchar, int f);
char *vgetw(char *line, char *spchar, int f, char *filed, int len);
char *GetLine(FILE * fp);

/*
*	return in which line, if not found, return 0;
*/

int IsInFileField(char *fname, char *str, int f);
int IsInFileField2(char *fname, char *str, char *sp, int f);
char *get_word_of_line(char *filename, char *spchar, int ln, int f);
int *get_line_of_words(char *filename, char *spc, int argc, char *argv[]);
int spilt_line2fields(char *line, char *spc, int argc, char **argv[]);
BOOL InsertLine(char *fname, char *line, int ln);
BOOL AppendLine(char *fname, char *line);
BOOL DeleteLine(char *fname, int ln);
BOOL DeleteSomeLine(char *fname, int ln, int num);
BOOL ReplaceLine(char *fname, char *line, int ln);
BOOL get_word(char *line, char *spchar, char *filed, int f);
char *safecpy(char *to, char *from, int len);
char *safecat(char *to, char *from, int len);
BOOL atob(char *str);
BOOL IsNum(char *str);
BOOL IsHexNum(char *str);
BOOL IsGoodIp(char *str);
BOOL IsGoodMask(char *str);
BOOL IsGoodCmd(char *str);
BOOL IsGoodName(char *str);
BOOL IsGoodDNS(char *str);
BOOL IsGoodMac(char *str);
int CountLine(char *fname);
int hextoi(char *s);
char *get_network(char *ip, char *netmask);
int delete_lines_of_words(char *filename, char *spc, int argc, char *argv[]);

#endif


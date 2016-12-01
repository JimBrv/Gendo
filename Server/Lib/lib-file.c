/*
 * Filename : lib-file.c
 * Copyright : All right reserved by SecWin, 2010
 * file operation 
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */

#include "lib-file.h"

#define u_long unsigned long int

BOOL
IsSpace(int c)
{
	if (c == ' ' || c == '\t')
		return true;
	return false;
}

BOOL
IsEOL(int c)
{
	if (!c || c == '\n' || c == '\r')
		return true;
	return false;
}

BOOL
IsNoteLine(char *line)
{
	while (*line && *line == ' ')
		line++;
	if (*line == '#')
		return true;
	return false;
}

BOOL
FileExist(char *fname)
{
	struct stat buf;
	return stat(fname, &buf) ? false : true;
}

BOOL
FileCP(char *src, char *dst)
{
	int sfd, dfd, ret = true;
	size_t len;
	char buf[LINE_BUF_LEN];

	if (FileExist(src) == false) {
		sfd =
		    open(src, O_WRONLY | O_TRUNC | O_CREAT | O_SYNC,
			 S_IRWXU | S_IRWXG | S_IRWXO);
		close(sfd);
		dfd =
		    open(dst, O_WRONLY | O_TRUNC | O_CREAT | O_SYNC,
			 S_IRWXU | S_IRWXG | S_IRWXO);
		close(dfd);
		return true;
	}
	if ((sfd = open(src, O_CREAT | O_RDONLY)) < 0) {
		return false;
	}

	if ((dfd =
	     open(dst, O_WRONLY | O_TRUNC | O_CREAT | O_SYNC,
		  S_IRWXU | S_IRWXG | S_IRWXO)) < 0) {
		close(sfd);
		return false;
	}

	while (1) {
		len = read(sfd, buf, LINE_BUF_LEN);
		if (len == 0)
			break;
		else if (len < 0) {
			ret = false;
			break;
		}
		if (write(dfd, buf, len) != len) {
			ret = false;
			break;
		}
	}
	close(sfd);
	close(dfd);
	return ret;
}

/*
* move file point to line
*/
BOOL
FileToLine(FILE * fp, int line)
{
	char buf[LINE_BUF_LEN];
	int i;
	rewind(fp);
	for (i = 1; i < line; i++) {
		if (fgets(buf, LINE_BUF_LEN, fp) == NULL)
			return false;
	}
	return true;
}

/*
* default seperator char is ' ' and '\t'
*/
char *
GetField(char *line, char *spchar, int f)
{
	static char field[LINE_BUF_LEN];
	int i = 0;
	char spc[] = " \t\n\r", *sp = spc, *p1 = NULL, *p2 = NULL;

	if (spchar && *spchar)
		sp = spchar;

	if (f == -1) {		/* the last feild */
		p2 = &line[strlen(line)] - 1;
		while (strchr(sp, *p2) != 0 && p2 >= line)
			p2--;	/* the last sep */
		p1 = p2;
		while (strchr(sp, *p1) == 0 && p1 >= line)
			p1--;
		p2++;
		p1++;
		i = -1;
	} else {
		for (; i < f && *line; i++) {
			while (*line && strchr(sp, *line) != 0)
				line++;	/* the head sep */
			p1 = line;

			while (*line && strchr(sp, *line) == 0)
				line++;
			p2 = line;
			if (*line)
				line++;
		}
	}
	if (i != f || (p2 - p1) <= 0)
		return NULL;	/* not enought feild */
	strncpy(field, p1, min(p2 - p1, sizeof (field) - 1));
	field[min(p2 - p1, sizeof (field) - 1)] = '\0';

	return field;
}

char *
GetLine(FILE * fp)
{
	static char line[LINE_BUF_LEN];
	int i;
	char *p;

	while (fgets(line, sizeof (line), fp) != NULL) {
		if (IsNoteLine(line))
			continue;
		if (IsEOL(*line))
			continue;
		p = line;
		while (!IsEOL(*p))
			p++;
		/* delete last space */
		while (p >= line && (IsSpace(*p) || IsEOL(*p)))
			p--;
		*(p + 1) = 0;

		/* delete first space */
		p = line;
		while (*p && IsSpace(*p))
			p++;
		/* move */
		i = 0;
		while (*p)
			line[i++] = *p++;
		line[i] = 0;
		if (*line == 0)
			continue;
		return line;
	}
	return NULL;
}

/*
*	return in which line, if not found, return 0;
*/

int
IsInFileField(char *fname, char *str, int f)
{
	char buf[LINE_BUF_LEN], *p, *r;
	FILE *fp;
	int i = 0;
	long place;
	int found = 0;

	if ((fp = fopen(fname, "r")) == NULL)
		return 0;
	while ((p = GetLine(fp)) != NULL) {
		if (f == -2) {	/* compare one line */
			if (strcmp(p, str) == 0)
				found = 1;
		} else {
			if ((r = GetField(p, NULL, f)) != NULL
			    && strcmp(r, str) == 0)
				found = 1;
		}
		if (found) {
			/* which line */
			place = ftell(fp);
			rewind(fp);
			while (fgets(buf, sizeof (buf), fp) != NULL) {
				i++;
				if (ftell(fp) == place)
					break;
			}
			fclose(fp);
			return i;
		}
	}
	fclose(fp);
	return 0;
}

int
IsInFileField2(char *fname, char *str, char *sp, int f)
{
	char buf[LINE_BUF_LEN], *p, *r;
	FILE *fp;
	int i = 0;
	long place;

	if ((fp = fopen(fname, "r")) == NULL)
		return 0;
	while ((p = GetLine(fp)) != NULL) {
		if ((r = GetField(p, sp, f)) != NULL && strcmp(r, str) == 0) {
			/* which line */
			place = ftell(fp);
			rewind(fp);
			while (fgets(buf, sizeof (buf), fp) != NULL) {
				i++;
				if (ftell(fp) == place)
					break;
			}
			fclose(fp);
			return i;
		}
	}
	fclose(fp);
	return 0;
}

BOOL
InsertLine(char *fname, char *line, int ln)
{
	FILE *fp, *tmp;
	char tmpname[PATH_MAX];
	char buf[LINE_BUF_LEN];

	sprintf(tmpname, "%s%s", fname, "tmp");
	if (FileCP(fname, tmpname) != true)
		return false;
	if ((tmp = fopen(tmpname, "r+")) == NULL) {
		return false;
	}
	unlink(tmpname);
	FileToLine(tmp, ln);
	truncate(fname, ftell(tmp));

	if ((fp = fopen(fname, "a")) == NULL) {
		fclose(tmp);
		return false;
	}
	fseek(fp, 0, 2);	/* to last place */
	fprintf(fp, "%s\n", line);
	while (fgets(buf, sizeof (buf), tmp) != NULL)
		fprintf(fp, "%s", buf);
	fclose(fp);
	fclose(tmp);
	return true;
}

BOOL
AppendLine(char *fname, char *line)
{
	FILE *fp;
	int c;

	if (!(fname))
		fp = fopen(fname, "w+");
	else
		fp = fopen(fname, "r+");
	if (fp == NULL)
		return false;
	fseek(fp, -1, 2);	/* to last place */
	c = fgetc(fp);
	if (!IsEOL(c))
		fprintf(fp, "\n");
	fseek(fp, 0, 2);
	fprintf(fp, "%s\n", line);
	fclose(fp);
	return true;
}

BOOL
DeleteLine(char *fname, int ln)
{
	return DeleteSomeLine(fname, ln, 1);
}

BOOL
DeleteSomeLine(char *fname, int ln, int num)
{
	FILE *fp, *tmp;
	char tmpname[PATH_MAX];
	char buf[LINE_BUF_LEN];

	sprintf(tmpname, "%s%s", fname, "tmp");
	if (FileCP(fname, tmpname) != true)
		return false;
	if ((tmp = fopen(tmpname, "r+")) == NULL) {
		return false;
	}
	unlink(tmpname);
	FileToLine(tmp, ln);
	truncate(fname, ftell(tmp));	/* delete -1 ??????????? */
	if (num < 0)		/* means to last */
		return true;

	if ((fp = fopen(fname, "a")) == NULL) {
		fclose(tmp);
		return false;
	}
	fseek(fp, 0, 2);	/* to last place */
	for (; num; num--)
		fgets(buf, sizeof (buf), tmp);	/* to skip curent line */

	while (fgets(buf, sizeof (buf), tmp) != NULL)
		fprintf(fp, "%s", buf);
	fclose(fp);
	fclose(tmp);
	return true;
}

BOOL
ReplaceLine(char *fname, char *line, int ln)
{
	FILE *fp, *tmp;
	char tmpname[PATH_MAX];
	char buf[LINE_BUF_LEN];

	sprintf(tmpname, "%s%s", fname, "tmp");
	if (FileCP(fname, tmpname) != true)
		return false;
	if ((tmp = fopen(tmpname, "r+")) == NULL)
		return false;
	unlink(tmpname);
	FileToLine(tmp, ln);
	truncate(fname, ftell(tmp));	/* delete -1 ??????? */

	if ((fp = fopen(fname, "a")) == NULL) {
		fclose(tmp);
		return false;
	}
	fseek(fp, 0, 2);	/* to last place */
	fgets(buf, LINE_BUF_LEN, tmp);	/* to skip curent line */

	fprintf(fp, "%s\n", line);
	while (fgets(buf, LINE_BUF_LEN, tmp) != NULL)
		fprintf(fp, "%s", buf);
	fclose(fp);
	fclose(tmp);
	return true;
}

char *
vgetw(char *line, char *spchar, int f, char *field, int len)
{
	int i = 0;
	char spc[] = " \t\n\r", *sp = spc, *p1 = NULL, *p2 = NULL;

	if (spchar && *spchar)
		sp = spchar;
	bzero(field, len);

	if (f == 0) {
		/* until space */
		while (*line && strchr(sp, *line) == NULL)
			line++;

		/* skip space between name and value */
		while (IsSpace(*line)) 
			line++;

		/*
		if (*line)
			line++;
		else
			return NULL;
		*/
		strncpy(field, line, min(len - 1, strlen(line)));
		return field;
	} else if (f == -1) {	/* the last feild */
		p2 = &line[strlen(line)] - 1;
		while (strchr(sp, *p2) != NULL && p2 >= line)
			p2--;	/* the last sep */
		p1 = p2;
		while (strchr(sp, *p1) == NULL && p1 >= line)
			p1--;
		p2++;
		p1++;
		i = -1;
	} else {
		for (; i < f && *line; i++) {
			while (*line && strchr(sp, *line) != NULL)
				line++;	/* the head sep */
			p1 = line;

			while (*line && strchr(sp, *line) == NULL)
				line++;
			p2 = line;
			if (*line)
				line++;
		}
	}
	if (i != f || (p2 - p1) <= 0)
		return NULL;	/* not enought feild */
	strncpy(field, p1, min(len - 1, p2 - p1));

	return field;
}

BOOL
get_word(char *line, char *spchar, char *field, int f)
{
	int i = 0;
	char spc[] = " \t\n\r", *sp = spc, *p1 = NULL, *p2 = NULL;

	if (spchar && *spchar)
		sp = spchar;

	if (f == -1) {		/* the last feild */
		p2 = &line[strlen(line)] - 1;
		while (strchr(sp, *p2) != NULL && p2 >= line)
			p2--;	/* the last sep */
		p1 = p2;
		while (strchr(sp, *p1) == NULL && p1 >= line)
			p1--;
		p2++;
		p1++;
		i = -1;
	} else {
		for (; i < f && *line; i++) {
			while (*line && strchr(sp, *line) != NULL)
				line++;	/* the head sep */
			p1 = line;

			while (*line && strchr(sp, *line) == NULL)
				line++;
			p2 = line;
			if (*line)
				line++;
		}
	}
	if (i != f || (p2 - p1) <= 0)
		return false;	/* not enought feild */
	strncpy(field, p1, p2 - p1);
	field[p2 - p1] = '\0';

	return true;
}

BOOL
LockToRead(int fd)
{
	if (flock(fd, LOCK_SH) < 0)
		return FALSE;
	else
		return TRUE;
}

BOOL
Unlock(int fd)
{
	if (flock(fd, LOCK_UN) < 0)
		return FALSE;
	else
		return TRUE;
}

BOOL
LockToWrite(int fd)
{
	if (flock(fd, LOCK_EX) < 0)
		return FALSE;
	else
		return TRUE;
}

char *
safecpy(char *to, char *from, int len)
{
	if (to == NULL || from == NULL)
		return NULL;
	bzero(to, len);
	strncpy(to, from, min(len - 1, strlen(from)));
	return to;
}

char *
safecat(char *to, char *from, int len)
{
	if (to == NULL || from == NULL)
		return NULL;
	if (len - 1 - strlen(to) < strlen(from)) {
		return NULL;
	} else {
		strcat(to, from);
		return to;
	}
}

BOOL
atob(char *val)
{
	if (val == NULL || *val == '\0')
		return FALSE;
	return atoi(val) != 0 ? TRUE : FALSE;
}

BOOL
IsNum(char *s)
{
	char *e;

	if (s == NULL)
		return FALSE;
	s += strspn(s, " 0\t");
	strtol(s, &e, 0);
	return *e == '\0';
}

BOOL
IsHexNum(char *s)
{
	char *e;

	if (s == NULL || s[0] == '\0')
		return FALSE;
	strtol(s, &e, 16);
	return *e == '\0';
}

int
hextoi(char *s)
{
	return strtol(s, NULL, 16);
}

/* may like x.x.x.x, x.x.x.x/yy, x.x.x.x/y.y.y.y */
BOOL
IsGoodIp(char *ip)
{
	char *f;
	unsigned long addr;
	int i;
	char mask[256];

	if (ip == NULL)
		return FALSE;
	f = GetField(ip, " \t/\n\r", 1);
	if (f == NULL)
		return FALSE;
	addr = inet_addr(f);
	if (addr == INADDR_NONE)
		return FALSE;
	for (i = 1; i <= 4; i++) {
		f = GetField(ip, ".", i);
		if (IsNull(f) || atoi(f) < 0 || atoi(f) > 255)
			return FALSE;
	}

	if ((f = vgetw(ip, " \t/\n\r", 2, mask, sizeof (mask))) == NULL)
		return TRUE;
	if (strchr(mask, '.') == NULL) {
		if (!IsNum(mask))
			return FALSE;
		if (atoi(mask) < 0 || atoi(mask) > 32)
			return FALSE;
	} else {
		addr = inet_addr(mask);
		if (addr == INADDR_NONE && strcmp(mask, "255.255.255.255") != 0)
			return FALSE;
		for (i = 1; i <= 4; i++) {
			f = GetField(mask, ".", i);
			if (IsNull(f) || atoi(f) < 0 || atoi(f) > 255)
				return FALSE;
		}
		if (!IsGoodMask(mask))
			return FALSE;
	}

	return TRUE;
}

BOOL
IsGoodMask(char *str)
{
	unsigned long addr, swap;
	int i;

	if (str == NULL)
		return FALSE;
	addr = inet_addr(str);
	if (addr == INADDR_NONE && strcmp(str, "255.255.255.255") != 0)
		return FALSE;
	swap = ((addr & 0xff000000) >> 24)
	    + ((addr & 0xff0000) >> 8)
	    + ((addr & 0xff00) << 8)
	    + ((addr & 0xff) << 24);
	for (i = 0; i <= 32; i++) {
		if (swap != 0 && (swap & 0x80000000) == 0)
			return FALSE;
		if (swap == 0)
			break;
		swap <<= 1;
	}
	return TRUE;
}

BOOL
IsGoodMac(char *str)
{
	int n, hex;
	char buf[LINE_BUF_LEN];

	for (n = 1; n <= 6; n++) {
		if (vgetw(str, ":\t ", n, buf, sizeof (buf)) == NULL)
			return FALSE;
		hex = hextoi(buf);
		if (!IsHexNum(buf) || hex < 0 || hex > 0xff)
			return FALSE;
	}
	if (vgetw(str, ":\t ", n, buf, sizeof (buf)))	/* more than 6 fields */
		return FALSE;
	return TRUE;
}

int
CountLine(char *fn)
{
	int n;
	FILE *fp;
	char line[LINE_BUF_LEN];

	fp = fopen(fn, "r");
	if (fp == NULL)
		return -1;
	n = 0;
	while (fgets(line, LINE_BUF_LEN, fp))
		n++;
	fclose(fp);
	return n;
}

BOOL
IsNull(char *str)
{
	if (str == NULL)
		return TRUE;
	if (str[0] == '\0')
		return TRUE;
	return FALSE;
}

BOOL
IsGoodCmd(char *str)
{
	if (strstr(str, ";") != NULL)
		return FALSE;
	return TRUE;
}

BOOL
IsGoodName(char *str)
{
	char *ext = "_";

	if (!isalpha(*str) && (unsigned char) (*str) < 128)
		return FALSE;
	str++;
	for (; *str; str++)
		if (!isalnum(*str) && strchr(ext, *str) == NULL
		    && (unsigned char) (*str) < 128)
			return FALSE;
	return TRUE;
}

BOOL
IsGoodDNS(char *str)
{
	char *ext = "_.";

	if (!isalpha(*str) && (unsigned char) (*str) < 128)
		return FALSE;
	str++;
	for (; *str; str++) {
		if (!isalnum(*str) && strchr(ext, *str) == NULL
		    && (unsigned char) (*str) < 128)
			return FALSE;
	}
	return TRUE;
}

static char *
inetaddr(u_int32_t ip)
{
	struct in_addr in;
	in.s_addr = ip;
	return inet_ntoa(in);
}

char *
get_network(char *ip, char *netmask)
{
	u_long addr, mask;

	addr = inet_addr(ip);
	mask = inet_addr(netmask);
	addr &= mask;
	return inetaddr(addr);
}

char *
get_word_of_line(char *filename, char *spchar, int ln, int f)
{
	static char buf[LINE_BUF_LEN], *p;
	FILE *fp;
	int i = 1;

	if ((fp = fopen(filename, "r")) == NULL)
		return NULL;
	while (i <= ln)
		if ((p = fgets(buf, sizeof (buf), fp)) == NULL) {
			fclose(fp);
			return NULL;
		} else
			i++;
	if (p)
		p = GetField(p, spchar, f);
	fclose(fp);
	return p;

}

int *
get_line_of_words(char *filename, char *spc, int argc, char *argv[])
{
	static char buf[LINE_BUF_LEN];
	static int ln[LINE_BUF_LEN];
	FILE *fp;
	int i, j, line_num;
	char *p, *line;
	char *sp = " \t\n\r";

	if ((fp = fopen(filename, "r")) == NULL)
		return NULL;
	if (spc)
		sp = spc;
	bzero(ln, sizeof (ln));
	line_num = j = 0;
	while (j < 1023 && (line = fgets(buf, sizeof (buf), fp)) != NULL) {
		int match = 0;
		line_num++;
		for (i = 0; i < argc; i++) {
			while (*line && strchr(sp, *line) != 0)
				line++;	/* the head sep */
			p = line;
			while (*line && strchr(sp, *line) == 0)
				line++;
			if (*line) {
				*line = 0;
				line++;
			}
			if (argv[i] != NULL && strcmp(argv[i], p) != 0)
				break;
			if (i == argc - 1)
				match = 1;
		}
		if (match) {
			ln[j] = line_num;
			j++;
		}
	}
	fclose(fp);
	return ln;
}

int
delete_lines_of_words(char *filename, char *spc, int argc, char *argv[])
{
	static char buf[LINE_BUF_LEN];
	char buf2[LINE_BUF_LEN];
	char tmpname[PATH_MAX];
	FILE *fp, *fp_out;
	int i;
	char *p, *line;
	char *sp = " \t\n\r";

	sprintf(tmpname, "%s%s", filename, "tmp");
	if ((fp = fopen(filename, "r")) == NULL
	    || (fp_out = fopen(tmpname, "a")) == NULL) {
		if (fp)
			fclose(fp);
		return 1;
	}
	if (spc)
		sp = spc;
	while ((line = fgets(buf, sizeof (buf), fp)) != NULL) {
		int match = 0;
		strcpy(buf2, buf);
		for (i = 0; i < argc; i++) {
			while (*line && strchr(sp, *line) != 0)
				line++;	/* the head sep */
			p = line;
			while (*line && strchr(sp, *line) == 0)
				line++;
			if (*line) {
				*line = 0;
				line++;
			}
			if (argv[i] != NULL && strcmp(argv[i], p) != 0)
				break;
			if (i == argc - 1)
				match = 1;
		}
		if (!match) {
			fprintf(fp_out, "%s", buf2);
		}
	}
	fclose(fp);
	fclose(fp_out);
	if (rename(tmpname, filename))
		return 1;
	return 0;
}

int
spilt_line2fields(char *line, char *spc, int argc, char **argv[])
{
	char *sp;
	int i = 0;
	char default_spc[] = " \t\n\r";

	if (!line || argc == 0)
		return 0;
	sp = (spc ? spc : default_spc);

	for (i = 0; i < argc; i++) {
		while (*line && strchr(sp, *line) != 0)
			line++;	/* the head sep */
		if (*line == 0)
			return i;
		if (argv[i])
			*argv[i] = line;

		while (*line && strchr(sp, *line) == 0)
			line++;
		if (*line) {
			*line = 0;
			line++;
		}
	}
	return argc;

}

int
init_lock(char *lock_file)
{
	int lock_fd;

	lock_fd = open(lock_file, O_RDWR | O_TRUNC | O_CREAT, 0666);
	if (lock_fd < 0) {
		printf("open lock file '%s' failed", lock_file);
		return -1;
	}
	return lock_fd;
}

typedef void (*sighandler_t) (int);

static sighandler_t
sig_set(int signum, void (*signal_hand) (int))
{
	struct sigaction act, old;

	memset(&act, 0, sizeof (act));
	memset(&old, 0, sizeof (act));

	act.sa_handler = signal_hand;
	sigemptyset(&act.sa_mask);
	sigaddset(&act.sa_mask, signum);
	act.sa_flags = /* SA_RESTART | */ SA_NOCLDSTOP;

	sigaction(signum, &act, &old);

	return old.sa_handler;
}

static sighandler_t old_hdl = NULL;

static void
sig_alarm(int sig)
{
	return;
}

int
lock_byte(int fd, int type, uint offset, int timeout)
{
	int ret;
	time_t die_time, t;
	struct flock lock;
	char buf[64];

	lock.l_type = type;
	lock.l_start = offset;
	lock.l_whence = SEEK_SET;
	lock.l_len = 0;

	if (timeout == NO_WAIT)
		return fcntl(fd, F_SETLK, &lock);

	time(&t);
	die_time = t + timeout;
	if (timeout != WAIT_FOREVER) {
		old_hdl = sig_set(SIGALRM, sig_alarm);
		alarm(timeout);
	}

	while (TRUE) {
		ret = fcntl(fd, F_SETLKW, &lock);
		if (ret < 0) {
			if (errno == EINTR) {
				/* catch signal, check timeout */
				if (timeout == WAIT_FOREVER)
					continue;
				time(&t);
				if (t < die_time) {
					alarm(die_time - t);
					continue;
				}
				printf("pid=%d lock timeout %ds\n", getpid(),
				       timeout);
			} else {
				printf
				    ("lock_byte, fd=%d, type=%d, offset=%d, timeout=%d",
				     fd, type, offset, timeout);
			}
		} else {
			sprintf(buf, "pid=%d\n", getpid());
			write(fd, buf, strlen(buf));
		}
		break;
	}

	if (timeout != WAIT_FOREVER) {
		alarm(0);
		sig_set(SIGALRM, old_hdl);
	}

	return ret;
}





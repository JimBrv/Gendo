/*
 * Filename : lib_socket.c
 * Copyright : All right reserved by SecWin, 2010
 * Base socket wrapper
 * 
 * Ver   Author   Date         Desc
 * 0.1  wy      2010.8     initial version        
 */
//#include <linux/tcp.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include "lib-base.h"
#include "lib-socket.h"
#include "lib-debug.h"
#include "lib-msg.h"


#define IS_SOCKET_SHOULD_CONTINUE(ERR) \
    ((ERR) == EINTR /* || (ERR) == EWOULDBLOCK || (ERR) == EAGAIN*/)


int lib_socket_tcp_nodelay(int fd)
{
	int ret = 0;
	int flag = 1;

	if (fd <=0 ) return -1;
	ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(flag));
	if (ret == -1) {
	    lib_error(MODULE_UTL, "couldn't setsockopt(TCP_NODELAY)\n");
	    return -1;
	}
	return 0;
}

int lib_socket(int domain, int type, int protocol)
{
	/* Disable the Nagle (TCP No Delay) algorithm */
    int fd = 0;

	fd = socket(domain, type, protocol);
	if (fd <= 0) {
	    lib_error(MODULE_UTL, "socket() error!\n");		
		return -1;
	}
	if (type == SOCK_STREAM) {
	    lib_socket_tcp_nodelay(fd);
	}
	return fd;
}

int lib_socketpair(int domain, int type, int protocol, int sockvec[2])
{
    return socketpair(domain, type, protocol, sockvec);
}

int lib_bind(int fd, struct sockaddr_in *addr)
{
    int addrlen = sizeof(struct sockaddr_in);
    return bind(fd, (struct sockaddr *) addr, addrlen);
}

int lib_connect_nonblock(int sockfd, const struct sockaddr *serv_addr,
             socklen_t addrlen)
{
    int ret;

    while (1) {
        ret = connect(sockfd, serv_addr, addrlen);
        if ((ret < 0) && IS_SOCKET_SHOULD_CONTINUE(errno)) {
            continue;
        }
        return ret;
    }
}

int lib_connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
    int ret;

    while (1) {
    ret = connect(sockfd, serv_addr, addrlen);
    if (ret < 0) {
        if (IS_SOCKET_SHOULD_CONTINUE(errno)) {
            continue;
        }

        lib_error(MODULE_UTL, "connect() errno(%d)\n", errno);
        return ret;
    }

    return ret;
    }
}


int lib_accept(int fd)
{
    int ret;

    while (1) {
        ret = accept(fd, NULL, NULL);
        if (ret < 0) {
            if (IS_SOCKET_SHOULD_CONTINUE(errno)) {
                continue;
            }

            lib_error(MODULE_UTL, "accept() errno(%d)\n", errno);
            return ret;
        }
		lib_socket_tcp_nodelay(ret);
        return ret;
    }
}

#define IS_SOCKET_ACCEPT_NO_NEW(ERR) \
    ((ERR) == EWOULDBLOCK || (ERR) == EAGAIN)

int lib_accept_nonblock(int fd)
{
    int ret = 0;

    while (1) {
        ret = accept(fd, NULL, NULL);
        if (ret < 0) {
            if (IS_SOCKET_ACCEPT_NO_NEW(errno)) {
                return OK;
            }

            lib_error(MODULE_UTL, "accept() errno(%d)\n", errno);
            return ret;
        }

        return ret;
    }
}


int lib_recv(int fd, char *buf, int len, int flags)
{
    int ret;

    while (1) {
        ret = recv(fd, buf, len, flags);
        if (ret < 0) {
            if (IS_SOCKET_SHOULD_CONTINUE(errno)) {
                continue;
            }

            lib_error(MODULE_UTL, "recv(%d) errno(%d)\n", fd, errno);
            return ret;
        }
        if (ret == 0) {
            lib_error(MODULE_UTL, "recv() peer closed\n");
            return ret;
        }

        return ret;
    }
}
    
int lib_recvn(int fd, char *buf, int len, int flags)
{
    int ret =0;
    int nleft = 0;
    int nread = 0;
    int trycnt = 0;
    int i = 0;
    char *ptr = NULL;

    ptr   = buf;
    nleft = len;

#if 0
    ioctl(fd, FIONREAD, &i);

    if (i == 0) {
        /* No data in buffer, should close soon */
        lib_error(MODULE_UTL,"recvn(%d) no buf data\n", fd);
        return -1;
    }
#endif
    
    while (nleft > 0) {
        if ((nread = recv(fd, ptr, nleft, flags)) < 0) {
            if (IS_SOCKET_SHOULD_CONTINUE(errno)) {
                nread = 0;
                trycnt++;
                if (trycnt >= 2) { 
                    return -1;
                }
                lib_info("recvn(%d) should continue\n", fd);
            } else {
                lib_error(MODULE_UTL, "recvn(%d) errno(%d)\n", fd, errno);
                return -1;
            }
        } else if (nread == 0) {
            /* peer closed socket */
            lib_error(MODULE_UTL, "recvn(%d) peer closed\n", fd);
	     return 0;
            
        }

        nleft -= nread;
        ptr   += nread;
    }
    return (len - nleft);
}

/* Block socket recving with timeout */
int lib_recv_timeout(int fd, char *buf, int len, int second)
{
    fd_set rset;
    struct timeval time;
    time.tv_sec = second;
    time.tv_usec= 0;
    int ret = 0;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    if (second != 0) {
        ret = select(fd+1, &rset, NULL, NULL, &time);
    }else{
        ret = select(fd+1, &rset, NULL, NULL, NULL);
    }

    if(ret < 0) {
        /* error */
        lib_error(MODULE_UTL, "recv_timeout:select(%d) got error=%d\n",fd, errno);
        return ret;
    }else if(ret == 0) {
        /* timeout */
        lib_info("recv_timeout:select(%d) timeout\n",fd);
        return ret;
    }  
    
    /* recv will not blocked now */
    if (FD_ISSET(fd, &rset)){
        return lib_recv(fd, buf, len, 0);
    }        
    return ret;  
}

/* Block socket recving with timeout */
int lib_recvn_timeout(int fd, char *buf, int len, int second)
{
    fd_set rset;
    struct timeval time;
    time.tv_sec = second;
    time.tv_usec= 0;
    int ret = 0;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    if (second != 0) {
        ret = select(fd+1, &rset, NULL, NULL, &time);
    }else{
        ret = select(fd+1, &rset, NULL, NULL, NULL);
    }

    if(ret < 0) {
        /* error */
        lib_error(MODULE_UTL, "recvn_timeout:select(%d) got error=%d\n",fd, errno);
        return ret;
    }else if(ret == 0) {
        /* timeout */
        lib_info("recvn_timeout:select(%d) timeout\n",fd);
        return ret;
    }  
    
    /* recv will not blocked now */
    if (FD_ISSET(fd, &rset)){
        return lib_recvn(fd, buf, len, 0);
    }        
    return ret;  
}


int lib_send(int fd, char *buf, int len, int flags)
{
    int ret;

    while (1) {
    ret = send(fd, buf, len, flags);
    if (ret < 0) {
        if (IS_SOCKET_SHOULD_CONTINUE(errno)) {
            continue;
        }

        lib_error(MODULE_UTL, "send() errno(%d)\n", errno);
        return ret;
    }

    return ret;
    }
}

int lib_sendn(int fd, char *buf, int len, int flags)
{
    int nleft = 0;
    int nwritten = 0;
    char *ptr = NULL;

    ptr = buf;
    nleft = len;
    while (nleft > 0) {
    if ((nwritten = send(fd, ptr, nleft, flags)) <= 0) {
        if (IS_SOCKET_SHOULD_CONTINUE(errno))
            nwritten = 0;
        else {
            lib_error(MODULE_UTL, "send2(%d, %p, %d) errno(%d)\n",fd,buf,len,errno);
            return -1;
        }
    }
    nleft -= nwritten;
    ptr += nwritten;
    }
    return len;
}

int lib_recvfrom(int fd, char *buf, int len, struct sockaddr_in *addr)
{
    int ret;
    socklen_t addrlen = sizeof (struct sockaddr_in);

    while (1) {
    if (addr) {
        ret = recvfrom(fd, buf, len, 0, (struct sockaddr *)addr, &addrlen);
    } else {
        ret = recvfrom(fd, buf, len, 0, NULL, NULL);
    }
    if (ret < 0) {
        if (IS_SOCKET_SHOULD_CONTINUE(errno)) {
        continue;
        }

        lib_error(MODULE_UTL, "recvfrom() errno(%d)\n",
              errno);
        return ret;
    }

    return ret;
    }
}

int lib_sendto(int fd, char *buf, int len, SA *addr, int tolen)
{
    int ret;

    while (1) {
    ret = sendto(fd, buf, len, 0, (struct sockaddr *)addr, tolen);
    if (ret < 0) {
        if (IS_SOCKET_SHOULD_CONTINUE(errno)) {
            continue;
        }

        lib_error(MODULE_UTL, "sendto() errno(%d)\n", errno);
        return ret;
    }

    return ret;
    }
}

int lib_close(int fd)
{
    if (fd > 0) 
        close(fd); 
    return OK;
}

int lib_set_fd_timeout(int fd, int timeout)
{
    struct timeval tv;
    int retIn = 0;

    if (timeout <= 0) {
        return OK;
    }
    memset(&tv, 0, sizeof (tv));
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    retIn = setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (char *) (&tv),
		     sizeof (struct timeval));
    if (retIn < 0) {
        lib_error(MODULE_UTL, "setsockopt err fd[%d]\n", fd);
        return ERROR;
    }
    return OK;
}

int lib_select(int maxfds, fd_set * rd, fd_set * wr, fd_set * ec, unsigned long seconds)
{
    struct timeval time;

    time.tv_sec = seconds;
    time.tv_usec = 0;

    if (seconds == 0) {
        return select(maxfds, rd, wr, ec, NULL);
    } else {
        return select(maxfds, rd, wr, ec, &time);
    }
}

char *lib_inet_ntoa(long ipaddr)
{
    struct in_addr addr;

    addr.s_addr = htonl(ipaddr);
    return inet_ntoa(addr);
}

char *inet_ntoa_r(struct in_addr ina, char *buf)
{
    unsigned char *ucp = (unsigned char *) &ina;
    sprintf(buf, "%d.%d.%d.%d", ucp[0] & 0xff, ucp[1] & 0xff, ucp[2] & 0xff, ucp[3] & 0xff);
    return buf;
}

char *lib_inet_ntoa_r(long ipaddr, char *buf)
{
    struct in_addr addr;

    addr.s_addr = htonl(ipaddr);
    return inet_ntoa_r(addr, buf);
}

long lib_inet_aton(char *ipstr)
{
    struct in_addr addr;

    inet_aton(ipstr, &addr);
    return ntohl(addr.s_addr);
}

int lib_inet_addr(char *ipstr)
{
    in_addr_t ip;

    ip = inet_addr(ipstr);
    if ((ip == INADDR_NONE) || (ip == 0))
        return ERROR;
    return OK;
}

int lib_gethostbyname(char *name, char *ip)
{
    struct hostent *ht = NULL;
    lib_assert(name && ip);
    ht = gethostbyname(name);
    if (!ht) return ERROR;
    strcpy(ip, ht->h_addr_list[0]);
    return OK;
}

int lib_server_unix(char *sun_path, int type)
{
    int fd = 0;
    int ret = 0;
    int on = 0;
    struct sockaddr_un servaddr;

    if (sun_path == NULL) {
        lib_error(MODULE_UTL, "o2_server_unix sun_path NULL\n");
        return ERROR;
    }
    fd = lib_socket(AF_LOCAL, type, 0);
    if (fd <= 0) {
        lib_error(MODULE_UTL, "socket(%s)\n", sun_path);
        return ERROR;
    }

    on = 1;
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
    if (ret != OK) {
        close(fd);
        lib_error(MODULE_UTL, "setsockopt(%s)\n", sun_path);
        return ERROR;
    }

    unlink(sun_path);
    bzero(&servaddr, sizeof (servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, sun_path);
    ret = bind(fd, (struct sockaddr *) &servaddr, sizeof (servaddr));
    if (ret != OK) {
        close(fd);
        lib_error(MODULE_UTL, "bind(%s)\n", sun_path);
        return ERROR;
    }

    chmod(sun_path, 0777);

    if (type == SOCK_STREAM) {
        ret = listen(fd, 256);
        if (ret != OK) {
            close(fd);
            lib_error(MODULE_UTL, "listen(%s)\n", sun_path);
            return ERROR;
        }
    }

    return fd;
}

int lib_client_unix(char *sun_path, int type)
{
    int fd = 0;
    int ret = 0;
    struct sockaddr_un servaddr;

    if (sun_path == NULL) {
    lib_error(MODULE_UTL, "o2_client_unix sun_path NULL\n");
    return ERROR;
    }

    fd = lib_socket(AF_LOCAL, type, 0);
    if (fd <= 0) {
    lib_error(MODULE_UTL, "socket\n");
    return ERROR;
    }

    bzero(&servaddr, sizeof (servaddr));
    servaddr.sun_family = AF_LOCAL;
    strcpy(servaddr.sun_path, sun_path);
    ret = connect(fd, (struct sockaddr *) &servaddr, sizeof (servaddr));
    if (ret < 0) {
    close(fd);
    lib_error(MODULE_UTL, "connect\n");
    return ERROR;
    }

    return fd;
}

int lib_server_udp(char *ipstr, int port)
{
    int fd;
    int ret, on;
    struct sockaddr_in addr;

    fd = lib_socket(AF_INET, SOCK_DGRAM, 0);
    if (fd <= 0) {
        lib_error(MODULE_UTL, "socket(%d)\n", port);
        return ERROR;
    }

    on = 1;
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
    if (ret != OK) {
        close(fd);
        lib_error(MODULE_UTL, "setsockopt(%d)\n", port);
        return ERROR;
    }

    memset(&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    if (ipstr) {
        addr.sin_addr.s_addr = inet_addr(ipstr);
    }
    addr.sin_port = htons(port);

    ret = lib_bind(fd, &addr);
    if (ret != OK) {
        close(fd);
        lib_error(MODULE_UTL, "bind(%d)\n", port);
        return ERROR;
    }
    
    return fd;
}

int lib_server_nonblock(char *ipstr, int port)
{
    int fd;
    int ret, on;
    int flag = 0;
    struct sockaddr_in addr;

    fd = lib_socket(AF_INET, SOCK_STREAM, 0);
    if (fd <= 0) {
        lib_error(MODULE_UTL, "socket(%d)\n", port);
        return ERROR;
    }

    on = 1;
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
    if (ret != OK) {
        close(fd);
        lib_error(MODULE_UTL, "setsockopt(%d)\n", port);
        return ERROR;
    }

    /* set non-block */
    flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag|O_NONBLOCK);

    memset(&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    if (ipstr) {
        addr.sin_addr.s_addr = inet_addr(ipstr);
    }
    addr.sin_port = htons(port);

    ret = lib_bind(fd, &addr);
    if (ret != OK) {
        close(fd);
        lib_error(MODULE_UTL, "bind(%d)\n", port);
        return ERROR;
    }

    /* Notice: should be get tcp_max_syn_backlog by sysctl */
    ret = listen(fd, 256);
    if (ret != OK) {
        close(fd);
        lib_error(MODULE_UTL, "listen(%d)\n", port);
        return ERROR;
    }

    return fd;
}


int lib_server(char *ipstr, int port)
{
    int fd;
    int ret, on;
    struct sockaddr_in addr;

    fd = lib_socket(AF_INET, SOCK_STREAM, 0);
    if (fd <= 0) {
        lib_error(MODULE_UTL, "socket(%d)\n", port);
        return ERROR;
    }

    on = 1;
    ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on));
    if (ret != OK) {
        close(fd);
        lib_error(MODULE_UTL, "setsockopt(%d)\n", port);
        return ERROR;
    }

    memset(&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    if (ipstr) {
        addr.sin_addr.s_addr = inet_addr(ipstr);
    }
    addr.sin_port = htons(port);

    ret = lib_bind(fd, &addr);
    if (ret != OK) {
        close(fd);
        lib_error(MODULE_UTL, "bind(%d)\n", port);
        return ERROR;
    }

    /* Notice: should be get tcp_max_syn_backlog by sysctl */
    ret = listen(fd, 256);
    if (ret != OK) {
        close(fd);
        lib_error(MODULE_UTL, "listen(%d)\n", port);
        return ERROR;
    }

    return fd;
}

int _lib_client(char *ipstr, int port, int type, char *file, int line)
{
    int fd, ret;
    struct sockaddr_in addr;
    struct hostent h;
    struct hostent *hp;
    char buf[MAX_HOSTBUF_LEN];
    int h_errno;

    if (ipstr) {
        res_init();
        ret = gethostbyname_r(ipstr, &h, buf, sizeof (buf), &hp, &h_errno);
        if (ret || !hp) {
            lib_error(MODULE_UTL, "gethostbyname,ip=%s:%d, [%s:%d]\n",ipstr, port, file, line);
            return ERROR;
        }
    }

    fd = lib_socket(AF_INET, type, 0);
    if (fd <= 0) {
        lib_error(MODULE_UTL, "socket,[%s:%d]\n", file, line);
        return ERROR;
    }

    memset(&addr, 0, sizeof (struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(LIB_NET_LOCAL);
    if (ipstr) {
        memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
    }

    ret = lib_connect(fd, (struct sockaddr *) &addr, sizeof (addr));
    if (ret < 0) {
        close(fd);
        lib_error(MODULE_UTL, "connect,[%s:%d],[%s:%d]\n", 
        ipstr, port, file, line);
        return ERROR;
    }

    return fd;
}

int _lib_client2(char *ipstr, int localip, int port, int type, char *file, int line)
{
    int fd;
    int ret;
    struct sockaddr_in addr;
    struct hostent h;
    struct hostent *hp;
    char buf[MAX_HOSTBUF_LEN];
    int h_errno;

    if (ipstr) {
        res_init();
        ret = gethostbyname_r(ipstr, &h, buf, sizeof (buf), &hp, &h_errno);
        if (ret || !hp) {
            lib_error(MODULE_UTL, "gethostbyname,[%s:%d]\n", file, line);
            return ERROR;
        }
    }

    fd = lib_socket(AF_INET, type, 0);
    if (fd <= 0) {
        lib_error(MODULE_UTL, "socket,[%s:%d]\n", file, line);
        return ERROR;
    }

    if (localip) {
        memset(&addr, 0, sizeof (struct sockaddr_in));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(localip);
        ret = bind(fd, (struct sockaddr *) &addr, sizeof (addr));
        if (ret < 0) {
            close(fd);
            lib_error(MODULE_UTL, "bind,[%s:%d]\n", file, line);
            return ERROR;
        }
    }

    memset(&addr, 0, sizeof (struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(LIB_NET_LOCAL);
    if (ipstr) {
        memcpy(&addr.sin_addr, hp->h_addr, hp->h_length);
    }

    ret = lib_connect(fd, (struct sockaddr *) &addr, sizeof (addr));
    if (ret < 0) {
        close(fd);
        lib_error(MODULE_UTL, "connect,[%s:%d],[%s:%d]\n", 
                  ipstr, port, file, line);
        return ERROR;
    }

    return fd;
}

int lib_client_timeout(char *ipstr, int port, int type, int s)
{
    int fd, ret, flags;
    int error;
    socklen_t len;
    struct sockaddr_in addr;
    fd_set rset, wset;
    struct timeval t;

    fd = lib_socket(AF_INET, SOCK_STREAM, 0);
    if (fd <= 0) {
    return ERROR;
    }
    
    memset(&addr, 0, sizeof (struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ipstr);

    flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    ret = lib_connect_nonblock(fd, (struct sockaddr *) &addr, sizeof (addr));
    if (ret < 0) {
    if (errno != EINPROGRESS) {
        goto failed_connect;
    }

    FD_ZERO(&rset);
    FD_SET(fd, &rset);
    FD_ZERO(&wset);
    FD_SET(fd, &wset);

    t.tv_sec = s;
    t.tv_usec = 0;

    ret = select(fd + 1, &rset, &wset, NULL, &t);
    if (ret <= 0) {
        goto failed_connect;
    }

    if ((!FD_ISSET(fd, &rset)) && (!FD_ISSET(fd, &wset))) {
        goto failed_connect;
    }

    error = 0;
    len = sizeof (error);
    ret = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
    if (ret < 0) {
        goto failed_connect;
    }

    if (error) {
        goto failed_connect;
    }
    }

    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    return fd;

  failed_connect:
    fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
    lib_close(fd);
    return ERROR;
}

int lib_send_msg(int fd, pmsg_head_t pmsg)
{
    if (!pmsg || fd <= 0) return -1;
    int i = 0;
    i = lib_sendn(fd, (char *)pmsg, sizeof(msg_head_t) + pmsg->len, 0);
    if (i != (sizeof(msg_head_t) + pmsg->len)) {
        lib_error(MODULE_UTL, "send msg failed [%s:%d]\n", __FILE__, __LINE__);
        return -1;
    }
    return pmsg->len + sizeof(msg_head_t);
}

int lib_send_msg_htonl(int fd,pmsg_head_t pmsg){
    if (!pmsg || fd <= 0) return -1;
    int i = 0;
    UInt32 len = pmsg->len;
    pmsg->msg = htonl(pmsg->msg);
    pmsg->len = htonl(pmsg->len);
    i = lib_sendn(fd, (char *)pmsg, sizeof(msg_head_t) + len, 0);
    if (i != (sizeof(msg_head_t) + len)) {
        lib_error(MODULE_UTL, "send msg failed [%s:%d]\n", __FILE__, __LINE__);
        return -1;
    }
    return len + sizeof(msg_head_t);
}

int lib_recv_msg(int fd, char *buf, int len)
{
    pmsg_head_t pmsg = (pmsg_head_t)buf;
    int i = 0;
    
    if (!pmsg || fd <= 0) return -1;
    i = lib_recvn(fd, (char *)pmsg, sizeof(msg_head_t), 0);
    if (i != sizeof(msg_head_t)) {
        return -1;
    }
    if (pmsg->len > MAX_MSG_LEN || pmsg->len > (len - sizeof(msg_head_t))) {
        lib_error(MODULE_UTL, "recv msg payload len too large=%d\n", pmsg->len);
        return -1;
    }
    
    i = lib_recvn(fd, (char *)pmsg->payload, pmsg->len, 0);
    if (i != pmsg->len) {
        lib_error(MODULE_UTL, "recv msg payload failed [%s:%d]\n", __FILE__, __LINE__);
        return -1;
    }
    return pmsg->len + sizeof(msg_head_t);

}

int lib_recv_msg_ntohl(int fd,char * buf,int len){
	pmsg_head_t pmsg = (pmsg_head_t)buf;
	int i = 0;
	 
	 if (!pmsg || fd <= 0) return -1;
	 i = lib_recvn(fd, (char *)pmsg, sizeof(msg_head_t), 0);
	 if (i != sizeof(msg_head_t)) {
		 return -1;
	 }
	 pmsg->msg =ntohl(pmsg->msg);
	 pmsg->len=   ntohl(pmsg->len);
	 if (pmsg->len > MAX_MSG_LEN || pmsg->len > (len - sizeof(msg_head_t))) {
		 lib_error(MODULE_UTL, "recv msg payload len too large=%d\n", pmsg->len);
		 return -1;
	 }
	 i = lib_recvn(fd, (char *)pmsg->payload, pmsg->len, 0);
	 if (i != pmsg->len) {
		 lib_error(MODULE_UTL, "recv msg payload failed [%s:%d]\n", __FILE__, __LINE__);
		 return -1;
	 }
	 return pmsg->len + sizeof(msg_head_t);
}

int lib_recv_msg_timeout(int fd, char *buf, int len, int second)
{
    pmsg_head_t pmsg = (pmsg_head_t)buf;
    int i = 0;
    
    if (!pmsg || fd <= 0) return -1;
    if ((pmsg->len + sizeof(msg_head_t)) > len) return -1;

	/* Recving msg_head with timeout */
    i = lib_recvn_timeout(fd, (char *)pmsg, sizeof(msg_head_t), second);
    if (i != sizeof(msg_head_t)) {
        //lib_debug(MODULE_UTL, "recv msg head failed [%s:%d]\n", __FILE__, __LINE__);
        return i;
    }

    if (pmsg->len > MAX_MSG_LEN || pmsg->len > (len - sizeof(msg_head_t))) {
        lib_error(MODULE_UTL, "recv msg payload len too large=%d\n", pmsg->len);
        return -1;
    }

	/* 
	 * Recbing payload withou timeout, payload must follows msg_head 
	 * Guaranteed by sender.
	 */
    i = lib_recvn(fd, (char *)pmsg->payload, pmsg->len, 0);
    if (i != pmsg->len) {
        lib_error(MODULE_UTL, "recv msg payload failed [%s:%d]\n", __FILE__, __LINE__);
        return -1;
    }
    return pmsg->len + sizeof(msg_head_t);	
}


/* udp msg recv/send */
int lib_sendto_msg(int fd, pmsg_head_t pmsg)
{
    int i = 0;
    if (!pmsg || fd <= 0) return -1;
    
    i = lib_send(fd, (char *)pmsg, sizeof(msg_head_t) + pmsg->len, 0);
    if (i <= 0) {
        lib_error(MODULE_UTL, "sendto_msg failed\n");
        return -1;
    }
    
    return i;
}

int lib_recvfrom_msg(int fd, char *buf, int len)
{
    int i = 0;
    
    if (fd <= 0 || !buf || len <= 0) return -1;
    
    i = lib_recvfrom(fd, buf, len, NULL);
    if (i <= 0) {
        lib_error(MODULE_UTL, "recvfrom_msg failed\n");
        return i;
    }
    return i;
}

int lib_peerip(int fd, char *buf)
{
    int ret = OK;
    int len = sizeof(struct sockaddr_in);
    struct sockaddr_in addr;

    ret = getpeername(fd ,(struct sockaddr *)&addr, (socklen_t *)&len);
    
    if(ret < 0) {
        lib_error(MODULE_UTL,"getpeername error(%d)\n", errno);
        return ERROR;
    }

    inet_ntoa_r(addr.sin_addr, buf);
    return OK;
}




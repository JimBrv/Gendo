/*
 * Header : lib-socket.h
 * Copyright : All right reserved by SecWin, 2010
 * Socket wrapper header
 * 
 * Ver   Author   Date         Desc
 * 0.1	wy      2010.8     initial version        
 */

#ifndef LIB_SOCKET_H
#define LIB_SOCKET_H

#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "lib-types.h"
#include "lib-msg.h"

#define LIB_NET_LOCAL   	"127.0.0.1"
#define LIB_NET_ALL	        NULL
#define MAX_HOSTBUF_LEN		512
#define	SA	struct sockaddr

#define HIPQUAD(addr) \
	((unsigned char *)&(addr))[3], \
	((unsigned char *)&(addr))[2], \
	((unsigned char *)&(addr))[1], \
	((unsigned char *)&(addr))[0]

#define NIPQUAD(addr) \
	((unsigned char *)&(addr))[0], \
	((unsigned char *)&(addr))[1], \
	((unsigned char *)&(addr))[2], \
	((unsigned char *)&(addr))[3]

/* input: netmask (host order) */
#define MASK2NUM(netmask) \
	__extension__ \
({ \
 	unsigned int _netmask = netmask; \
 	int num = 0; \
 	while (_netmask) { \
 		_netmask = _netmask << 1; \
 		num++; \
 	} \
 	num; \
 })

/* return value is host order */
#define NUM2MASK(masklen) ((masklen)? htonl(0xFFFFFFFF << (32-(masklen))) : 0)

#ifdef __cplusplus
#define extern "C" {
#endif
/* socket interfaces */
extern int lib_socket_tcp_nodelay(int fd);

extern int lib_socket(int domain, int type, int protocol);
extern int lib_socketpair(int domain, int type, int protocol, int sockvec[2]);
extern int lib_bind(int fd, struct sockaddr_in *addr);
extern int lib_connect(int sockfd, const struct sockaddr *serv_addr,
		      socklen_t addrlen);

extern int lib_set_fd_timeout(int fd, int timeout);
		      
extern int lib_accept(int fd);
extern int lib_accept_nonblock(int fd);

extern int lib_recv(int fd, char *buf, int len, int flags);
extern int lib_recvn(int fd, char *buf, int len, int flags);
extern int lib_recvn_timeout(int fd, char *buf, int len, int second);
extern int lib_recv_timeout(int fd, char *buf, int len, int second);

extern int lib_send(int fd, char *buf, int len, int flags);
extern int lib_sendn(int fd, char *buf, int len, int flags);
extern int lib_recvfrom(int fd, char *buf, int len, struct sockaddr_in *addr);
extern int lib_sendto(int fd, char *buf, int len, SA *addr, int tolen);
extern int lib_close(int fd);
extern int lib_select(int maxfds, 
	                    fd_set *rd, 
	                    fd_set *wr, 
	                    fd_set *ec,
		                unsigned long seconds);
extern char *lib_inet_ntoa(long ipaddr);
extern char *inet_ntoa_r(struct in_addr ina, char *buf);
extern char *lib_inet_ntoa_r(long ipaddr, char *buf);
extern long lib_inet_aton(char *ipstr);
extern int  lib_inet_addr(char *ipstr);

extern int  lib_server_unix(char *sun_path, int type);
extern int  lib_client_unix(char *sun_path, int type);
extern int  lib_server(char *ipstr, int port);
extern int  lib_server_udp(char *ipstr, int port);

#define lib_client(ipstr, port, type) \
	_lib_client(ipstr, port, type, __FILE__, __LINE__)
#define lib_client2(ipstr, ipaddr, port, type) \
	_lib_client2(ipstr, ipaddr, port, type, __FILE__, __LINE__)
extern int _lib_client(char *ipstr, int port, int type, char *file, int line);
extern int _lib_client2(char *ipstr, int ipaddr, int port, int type, char *file, int line);
extern int lib_client_timeout(char *ipstr, int port, int type, int s);

extern int lib_recv_msg(int fd, char *buf, int len);
extern int lib_recv_msg_timeout(int fd, char *buf, int len, int timeout);

extern int lib_recv_msg_ntohl(int fd,char *buf,int len);

extern int lib_send_msg(int fd, pmsg_head_t pmsg);

extern int lib_send_msg_htonl(int fd, pmsg_head_t pmsg);

extern int lib_recvfrom_msg(int fd, char *buf, int len);

extern int lib_sendto_msg(int fd, pmsg_head_t pmsg);

extern int lib_peerip(int fd, char *buf);

extern int lib_gethostbyname(char *name, char *ip);

#ifdef __cplusplus
}
#endif


#endif

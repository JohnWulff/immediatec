static const char tcpc_h[] =
"@(#)$Id: tcpc.h,v 1.4 2001/01/31 17:53:10 jw Exp $";
/********************************************************************
 *
 *	TCP/IC communication support
 *
 *	Common includes and definitions for UDP, TCP and T/TCP
 *	clients and servers
 *
 *	W. R. Stevens, TCP/IP Illustrated, Vol 3, p 5
 *
 *******************************************************************/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define REQUEST	400		/* max size of request, in bytes */
#define REPLY	400		/* max size of reply, in bytes */

#define UDP_SERV_PORT	7777	/* UDP server's wel-known port */
#define TCP_SERV_PORT	8888	/* TCP server's wel-known port */
#define TTCP_SERV_PORT	9999	/* TTCP server's wel-known port */

    /* Following shortens all the type casts of pointer arguments */
#define SA		struct sockaddr *

#ifdef STEVENS
void	err_quit(const char *, ...);
void	err_sys(const char *, ...);
void	read_stream(int, char *, int);
#endif

extern const char *	hostNM;		/* 127.0.0.1 */
extern const char *	portNM;		/* pplc service */
extern const char *	pplcNM;		/* pplc name */
extern float		timeout;	/* in seconds */

extern int		connect_to_server(const char* host, const char* port,
					  const char* pplc, float delay);
extern int		wait_for_next_event(int maxFN);
extern int		rcvd_msg_from_server(int sock, char* buf, int maxLen);
extern void		send_msg_to_server(int sock, const char* msg);

extern fd_set		rdfds;
extern struct timeval	timeoutCounter;
extern struct timeval	timeoutValue;

extern int		sockFN;			/* TCPIP socket file number */

extern int		micro;
extern void		microPrint(const char * str, int mask);
extern void		microReset(int mask);

static const char tcpc_h[] =
"@(#)$Id: tcpc.h,v 1.7 2001/03/11 15:10:19 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2001  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <john@je-wulff.de>
 *
 *	tcpc.h
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
extern const char *	portNM;		/* icc service */
extern const char *	iccNM;		/* icc name */
extern float		timeout;	/* in seconds */

extern int		connect_to_server(const char* host, const char* port,
					  const char* icc, float delay);
extern int		wait_for_next_event(int maxFN);
extern int		rcvd_msg_from_server(int sock, char* buf, int maxLen);
extern void		send_msg_to_server(int sock, const char* msg);

extern fd_set		rdfds;
extern fd_set		infds;
extern struct timeval	timeoutCounter;
extern struct timeval	timeoutValue;

extern int		sockFN;			/* TCPIP socket file number */

extern int		micro;
extern void		microPrint(const char * str, int mask);
extern void		microReset(int mask);

/********************************************************************
 *
 *	Copyright (C) 1985-2005  John E. Wulff
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

#ifndef TCPC_H
#define TCPC_H
static const char tcpc_h[] =
"@(#)$Id: tcpc.h,v 1.13 2005/07/14 09:27:22 jw Exp $";

/* INT_MAX is set to the system value in sys/socket.h via bits/socket.h via limits.h */
#if INT_MAX == 32767
#define MY_INT_MAX
#endif

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* restore the command line definition of INT_MAX if it was 32767 for simulated 16 bit */
#ifdef MY_INT_MAX
#undef INT_MAX
#define INT_MAX 32767
#endif

#define REQUEST	1400			/* max size of request, in bytes */
#define REPLY	1400			/* max size of reply, in bytes */

#define UDP_SERV_PORT	7777		/* UDP server's wel-known port */
#define TCP_SERV_PORT	8888		/* TCP server's wel-known port */
#define TTCP_SERV_PORT	9999		/* TTCP server's wel-known port */

    /* Following shortens all the type casts of pointer arguments */
#define SA		struct sockaddr *

#ifdef STEVENS
void	err_quit(const char *, ...);
void	err_sys(const char *, ...);
void	read_stream(int, char *, int);
#endif

extern const char *	iC_hostNM;	/* 127.0.0.1 */
extern const char *	iC_portNM;	/* immcc service */
extern char *		iC_iccNM;	/* immcc name qualified with instance */
extern char *		iC_iidNM;	/* instance ID */
extern float		iC_timeout;	/* in seconds */

extern int		iC_connect_to_server(const char* host, const char* port, float delay);
extern int		iC_wait_for_next_event(int maxFN);
extern int		iC_rcvd_msg_from_server(int sock, char* buf, int maxLen);
extern void		iC_send_msg_to_server(int sock, const char* msg);

extern fd_set		iC_rdfds;
extern fd_set		iC_infds;
extern struct timeval	iC_timeoutCounter;
extern struct timeval	iC_timeoutValue;

extern int		iC_micro;
extern void		iC_microPrint(const char * str, int mask);
extern void		iC_microReset(int mask);
#endif	/* TCPC_H */

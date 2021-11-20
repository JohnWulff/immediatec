/********************************************************************
 *
 *	Copyright (C) 1985-2009  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
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
"@(#)$Id: tcpc.h 1.25 $";

/* INT_MAX is set to the system value in sys/socket.h via bits/socket.h via limits.h */
#if INT_MAX == 32767
#define MY_INT_MAX
#endif

#include	<sys/types.h>
#ifdef	_WIN32
#include	<winsock2.h>
#include	<windows.h>
#else	/* ! _WIN32 Linux */
#include	<sys/select.h>
#include	<unistd.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#define SOCKET   int
#endif	/* _WIN32 */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

/* restore the command line definition of INT_MAX if it was 32767 for simulated 16 bit */
#ifdef MY_INT_MAX
#undef INT_MAX
#define INT_MAX 32767
#endif

#define REQUEST	1400-4			/* max size of request, in bytes - leave room for 4 byte length header */
#define REPLY	1400-4			/* max size of reply, in bytes - leave room for 4 byte length header */

#define LOCALHOST	"localhost"
#define LOCALHOST1	"127.0.0.1"
#define iC_PORT		"8778"

    /* Following shortens all the type casts of pointer arguments */
#define SA		struct sockaddr *

extern const char *	iC_hostNM;	/* LOCALHOST */
extern const char *	iC_portNM;	/* iC_PORT */
extern char *		iC_iccNM;	/* immcc name qualified with instance */
extern char *		iC_iidNM;	/* instance ID */

extern int		iC_maxFN;
extern fd_set		iC_rdfds;
extern fd_set		iC_exfds;
extern SOCKET		iC_sockFN;	/* TCP/IP socket file number */

extern SOCKET		iC_connect_to_server(const char* host, const char* port);
extern int		iC_wait_for_next_event(fd_set * infdsp, fd_set * ixfdsp, struct timeval * ptv);
extern int		iC_rcvd_msg_from_server(SOCKET sock, char* buf, int maxLen);
extern void		iC_send_msg_to_server(SOCKET sock, const char* msg);

extern int		iC_Xflag;	/* 1 if this process started iCserver */
extern char *		iC_vcd;
#if YYDEBUG && !defined(_WINDOWS)
extern int		iC_micro;
extern void		iC_microPrint(const char * str, int mask);
extern void		iC_microReset(int mask);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#endif	/* TCPC_H */

static const char RCS_Id[] =
"@(#)$Id: tcpc.c,v 1.18 2007/08/09 14:05:00 jw Exp $";
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
 *	tcpc.c
 *	TCP/IC communication support
 *
 *	M. J. Bach, The Design of the UNIX Operating System
 *	11.4 Sockets, p. 388
 *
 *	W. R. Stevens, TCP/IP Illustrated, Vol 3, p 10
 *
 *******************************************************************/

#include	<stdio.h>
#ifdef	WIN32
#include	<Time.h>
#else	/* WIN32 */
#include	<netdb.h>
#include	<sys/time.h>
#endif	/* WIN32 */
#include	<ctype.h>
#include	<errno.h>
#include	"tcpc.h"
#include	"icc.h"

const char *	iC_hostNM = "localhost";	/* 127.0.0.1 */
const char *	iC_portNM = "8778";		/* iC service */
char *		iC_iccNM  = "stdin";		/* immcc name qualified with instance */
char *		iC_iidNM  = "";			/* instance ID */
double		iC_timeout = 0.05;		/* default 50 ms on 50 ms off */

fd_set		iC_rdfds;
fd_set		iC_infds;
struct timeval	iC_timeoutCounter;
struct timeval	iC_timeoutValue;

/********************************************************************
 *
 *	private structures and variables
 *
 *******************************************************************/

typedef struct NetBuffer {
#if INT_MAX == 32767 && defined (LONG16)
    long	length;
#else
    int		length;
#endif
    char	buffer[REPLY];
} NetBuffer;

static struct timeval *	ptv = NULL;
#ifdef	WIN32
static int	freqFlag;
static double	frequency;
static LARGE_INTEGER freq, start, end;
#else	/* WIN32 */
static struct timeval	mt0;
static struct timeval	mt1;
#endif	/* WIN32 */

/********************************************************************
 *
 *	Reset microsecend timer at program start
 *	Print microseconds since last call to microPrint
 *
 *******************************************************************/

void
iC_microReset(int mask)
{
#ifdef	WIN32
    QueryPerformanceCounter(&start);
#else	/* WIN32 */
    gettimeofday(&mt0, 0);			/* reset for next measurement */
#endif	/* WIN32 */
    iC_micro |= mask;
} /* iC_microReset */

void
iC_microPrint(const char * str, int mask)
{
#ifdef	WIN32
    double fi;

    if (freqFlag == 0 && QueryPerformanceFrequency(&freq)) {
	freqFlag = 1;
	frequency = freq.HighPart * 4294967296.0 + freq.LowPart;
        // printf("frequency is %f\n", frequency);
    }
    if (freqFlag && QueryPerformanceCounter(&end)) {
	// printf("start %d.%u\n", start.HighPart, start.LowPart);
	// printf("end   %d.%u\n", end.HighPart, end.LowPart);
	fi = ((double)(end.LowPart - start.LowPart) +
	     (double)(end.HighPart - start.HighPart) * 4294967296.0) / frequency;
	printf("%10.6f: %s\n", fi, str);
        QueryPerformanceCounter(&start);	/* start of next measurement without print time */
    } else {
        printf("micro: %s\n", str);
    }
#else	/* WIN32 */
    long	sec;
    long	usec;

    if (gettimeofday(&mt1, 0) == 0) {
	sec = mt1.tv_sec - mt0.tv_sec;
	usec = mt1.tv_usec - mt0.tv_usec;
	if (usec < 0) {
	    sec--;
	    usec += 1000000;
	}
	printf("%3ld.%03ld,%03ld: %s\n", sec, usec/1000, usec%1000, str);
    }
    gettimeofday(&mt0, 0);			/* start of next measurement without print time */
#endif	/* WIN32 */
    iC_micro &= ~mask;
} /* iC_microPrint */

/********************************************************************
 *
 *	Connect to server 'host' and 'port' as a client
 *	Use 'iccNM' for messages (do not register with server)
 *	Set timeout value with 'delay' 
 *	Return:	socket file number
 *
 *******************************************************************/

SOCKET
iC_connect_to_server(const char *	host,
		     const char *	port,
		     double		delay)
{
    SOCKET		sock;
    struct in_addr	sin_addr;
    unsigned short int	sin_port;
    struct sockaddr_in	server;
#ifdef	WIN32
    WORD		wVersionRequested;
    WSADATA		wsaData;
    int			err;

//    wVersionRequested = MAKEWORD( 1, 0 );
    wVersionRequested = MAKEWORD( 2, 2 );

    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
	/* Tell the user that we could not find a usable */
	/* WinSock DLL.                                  */
	fprintf(iC_errFP, "WSAStartup failed: %d\n", err);
	iC_quit(1);
    }
/*    fprintf(iC_errFP, "WSAStartup: requested %d.%d, version %d.%d, high version %d.%d\n",
	HIBYTE( wVersionRequested ), LOBYTE( wVersionRequested ),
	HIBYTE( wsaData.wVersion ), LOBYTE( wsaData.wVersion ),
	HIBYTE( wsaData.wHighVersion ), LOBYTE( wsaData.wHighVersion ));
*/

    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested. (works with 1.0 to 2.2 - JW 051107)    */

    if ( LOBYTE( wsaData.wVersion ) != LOBYTE( wVersionRequested ) ||
	    HIBYTE( wsaData.wVersion ) != HIBYTE( wVersionRequested ) ) {
	/* Tell the user that we could not find a usable */
	/* WinSock DLL.                                  */
	WSACleanup( );
	fprintf(iC_errFP, "WSAStartup failed: requested %d.%d, got %d.%d\n",
	    HIBYTE( wVersionRequested ), LOBYTE( wVersionRequested ),
	    HIBYTE( wsaData.wVersion ), LOBYTE( wsaData.wVersion ));
	iC_quit(1);
    }

    /* The WinSock DLL is acceptable. Proceed. */
#endif	/* WIN32 */

    if (isdigit(*host)) {
#ifdef	WIN32
	/* inet_addr() obselete under Linux because of 255.255.255.255 error return */
	if ((sin_addr.S_un.S_addr = inet_addr(host)) == INADDR_NONE)
#else	/* WIN32 */
	if (inet_aton(host, &sin_addr) == 0)
#endif	/* WIN32 */
	{
	    fprintf(iC_errFP, "inet_aton with '%s' failed\n", host);
	    iC_quit(1);
	}
    } else {
	struct hostent *	pHost;
	if ((pHost = gethostbyname(host)) == NULL) {
	    fprintf(iC_errFP, "gethostbyname with '%s' failed\n", host);
	    iC_quit(1);
	}
	/* gethostbyname returns h_addr in network byte order */
	sin_addr.s_addr = *((unsigned int*)pHost->h_addr);
    }

    if (isdigit(*port)) {
	sin_port = htons(atoi(port));
    } else {
	struct servent *	pServ;
	if ((pServ = getservbyname(port, "tcp")) == NULL) {
	    fprintf(iC_errFP, "getservbyname with '%s/tcp' failed\n", port);
	    iC_quit(1);
	}
	/* getservbyname returns s_port in network byte order */
	sin_port = pServ->s_port;
    }

#ifdef	WIN32
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
	fprintf(iC_errFP, "socket failed: %d\n", WSAGetLastError());
#else	/* WIN32 */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
	perror("socket failed");
#endif	/* WIN32 */
	iC_quit(1);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr = sin_addr;
    server.sin_port = sin_port;

    if (connect(sock, (SA)&server, sizeof(server)) < 0) {
	fprintf(iC_errFP, "ERROR in %s: client could not be connected to server '%s:%d'\n",
	    iC_iccNM, inet_ntoa(server.sin_addr), ntohs(server.sin_port));
	perror("connect failed");
	iC_quit(1);
    }

    fprintf(iC_outFP, "'%s' connected to server at '%s:%d'\n",
	iC_iccNM, inet_ntoa(server.sin_addr), ntohs(server.sin_port));

    FD_ZERO(&iC_infds);			/* should be done centrally if more than 1 connect */
#ifndef	WIN32
    FD_SET(0, &iC_infds);		/* watch stdin for inputs - FD_CLR on EOF */
    /* can only use sockets, not file descriptors under WINDOWS - use kbhit() */
#endif	/* WIN32 */
    FD_SET(sock, &iC_infds);		/* watch sock for inputs */

    iC_timeoutValue.tv_sec = (int)delay;
    iC_timeoutValue.tv_usec = (int)((delay - iC_timeoutValue.tv_sec) * 1e6);
    if (iC_timeoutValue.tv_sec != 0 || iC_timeoutValue.tv_usec != 0) {
	iC_timeoutCounter = iC_timeoutValue;
	ptv = &iC_timeoutCounter;
    }
    return sock;
} /* iC_connect_to_server */

/********************************************************************
 *
 *	Wait for next selected event
 *
 *******************************************************************/

int
iC_wait_for_next_event(int maxFN)
{
    int	retval;

    do {
	iC_rdfds = iC_infds;
    } while ((retval = select(maxFN + 1, &iC_rdfds, 0, 0, ptv)) == -1 && errno == EINTR);
#ifdef	WIN32
    if (retval == -1) {
	fprintf(iC_errFP, "ERROR: select failed: %d\n", WSAGetLastError());
	iC_quit(1);
    }
#endif	/* WIN32 */

    return retval;
} /* iC_wait_for_next_event */

/********************************************************************
 *
 *	Receive known length buffer from server
 *
 *******************************************************************/

static int
rcvd_buffer_from_server(SOCKET sock, char * buf, int length)
{
    int		len;

    if ((len = recv(sock, buf, length, 0)) > 0) {
	if (len != length) {
	    int i;
	    fprintf(iC_errFP, "recv: length %d != expected length %d\n", len, length);
	    for (i = 0; i < len; i++) {
		fprintf(iC_errFP, " %02x", ((unsigned char*)&buf)[i]);
	    }
	}
    } else if (len < 0) {
	perror("recv failed");
	iC_quit(1);
    }
    return len;
} /* rcvd_buffer_from_server */

/********************************************************************
 *
 *	Receive message from server
 *
 *******************************************************************/

int
iC_rcvd_msg_from_server(SOCKET sock, char * buf, int maxLen)
{
    NetBuffer	netBuf;
    int		len;

    if ((len = rcvd_buffer_from_server(sock, (char*)&netBuf.length, sizeof netBuf.length))
	    == sizeof netBuf.length) {		/* get 4 bytes of length to start with */
	len = ntohl(netBuf.length);		/* length in host byte order */
	if (len >= maxLen) {
	    fprintf(iC_errFP, "ERROR in %s: received message is too long: %d >= %d\n", iC_iccNM, len, maxLen);
	    len = maxLen - 1;
	}
	maxLen = len;
	if ((len = rcvd_buffer_from_server(sock, netBuf.buffer, maxLen)) == maxLen) {
	    memcpy(buf, netBuf.buffer, len);
	    buf[len] = '\0';
#if YYDEBUG
	    if (iC_debug & 02) {
		fprintf(iC_outFP, "%s < '%s'\n", iC_iccNM, buf);	/* trace recv buffer */
	    }
#endif
	}
    }
    return len;
} /* iC_rcvd_msg_from_server */

/********************************************************************
 *
 *	Send message to server
 *	(handles short messages only)
 *
 *******************************************************************/

void
iC_send_msg_to_server(SOCKET sock, const char * msg)
{
    NetBuffer	netBuf;
    size_t	len = strlen(msg);

    if (len >= sizeof netBuf.buffer) {
	fprintf(iC_errFP, "ERROR in %s: message to send is too long: %d >= %d\n", iC_iccNM, len, sizeof netBuf.length);
	len = sizeof netBuf.buffer - 1;
    } else
#if YYDEBUG
    if (iC_debug & 01) {
	fprintf(iC_outFP, "%s > '%s'\n", iC_iccNM, msg);	/* trace send buffer */
    }
#endif
    memcpy(netBuf.buffer, msg, len + 1);
    netBuf.length = htonl(len);
    len += sizeof netBuf.length;
    if (send(sock, (char*)&netBuf, len, 0) != len) {
	perror("send failed");
	iC_quit(1);
    }
} /* iC_send_msg_to_server */

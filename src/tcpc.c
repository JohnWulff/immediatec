static const char RCS_Id[] =
"@(#)$Id: tcpc.c,v 1.15 2005/01/26 15:49:43 jw Exp $";
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
#include	<netdb.h>
#include	<ctype.h>
#include	<sys/time.h>
#include	<errno.h>
#include	"tcpc.h"
#include	"icc.h"

const char *	iC_hostNM = "localhost";	/* 127.0.0.1 */
const char *	iC_portNM = "8778";		/* iC service */
char *		iC_iccNM  = "C0";		/* icc name qualified with instance */
char *		iC_iidNM  = "";			/* instance id */
float		iC_timeout = 0.05;		/* default 50 ms on 50 ms off */

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
    long	length;
    char	buffer[REPLY];
} NetBuffer;

static struct timeval *	ptv = NULL;
static struct timeval	mt0;
static struct timeval	mt1;

/********************************************************************
 *
 *	Reset microsecend timer at program start
 *	Print microseconds since last call to microPrint
 *
 *******************************************************************/

void
iC_microReset(int mask)
{
    gettimeofday(&mt0, 0);			/* reset for next measurement */
    iC_micro |= mask;
} /* microReset */

void
iC_microPrint(const char * str, int mask)
{
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
    iC_micro &= ~mask;
} /* microPrint */

/********************************************************************
 *
 *	Connect to server 'host' and 'port' as a client
 *	Use 'iccNM' for messages (do not register with server)
 *	Set timeout value with 'delay' 
 *	Return:	socket file number
 *
 *******************************************************************/

int
iC_connect_to_server(const char *	host,
		  const char *	port,
		  float		delay)
{
    int			sock;
    struct in_addr	sin_addr;
    unsigned short int	sin_port;
    struct sockaddr_in	server;

    if (isdigit(*host)) {
	if (inet_aton(host, &sin_addr) == 0) {
	    fprintf(stderr, "inet_aton with '%s' failed\n", host);
	    iC_quit(1);
	}
    } else {
	struct hostent *	pHost;
	if ((pHost = gethostbyname(host)) == NULL) {
	    fprintf(stderr, "gethostbyname with '%s' failed\n", host);
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
	    fprintf(stderr, "getservbyname with '%s/tcp' failed\n", port);
	    iC_quit(1);
	}
	/* getservbyname returns s_port in network byte order */
	sin_port = pServ->s_port;
    }

    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
	perror("socket failed");
	iC_quit(1);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr = sin_addr;
    server.sin_port = sin_port;

    if (connect(sock, (SA)&server, sizeof(server)) < 0) {
	fprintf(stderr, "ERROR in %s: client could not be connected to server '%s:%d'\n",
	    iC_iccNM, inet_ntoa(server.sin_addr), ntohs(server.sin_port));
	perror("connect failed");
	iC_quit(1);
    }

    printf("'%s' connected to server at '%s:%d'\n",
	iC_iccNM, inet_ntoa(server.sin_addr), ntohs(server.sin_port));

    FD_ZERO(&iC_infds);			/* should be done centrally if more than 1 connect */
    FD_SET(0, &iC_infds);		/* watch stdin for inputs - FD_CLR on EOF */
    FD_SET(sock, &iC_infds);		/* watch sock for inputs */

    iC_timeoutValue.tv_sec = (int)delay;
    iC_timeoutValue.tv_usec = (int)((delay - iC_timeoutValue.tv_sec) * 1e6);
    if (iC_timeoutValue.tv_sec != 0 || iC_timeoutValue.tv_usec != 0) {
	iC_timeoutCounter = iC_timeoutValue;
	ptv = &iC_timeoutCounter;
    }
    return sock;
} /* connect_to_server */

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

    return retval;
} /* wait_for_next_event */

/********************************************************************
 *
 *	Receive known length buffer from server
 *
 *******************************************************************/

static int
rcvd_buffer_from_server(int sock, char * buf, int length)
{
    int		len;

    if ((len = recv(sock, buf, length, 0)) > 0) {
	if (len != length) {
	    int i;
	    printf("recv: length %d != expected length %d\n", len, length);
	    for (i = 0; i < len; i++) {
		printf(" %02x", ((unsigned char*)&buf)[i]);
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
iC_rcvd_msg_from_server(int sock, char * buf, int maxLen)
{
    NetBuffer	netBuf;
    int		len;

    if ((len = rcvd_buffer_from_server(sock, (char*)&netBuf.length, sizeof netBuf.length))
	    == sizeof netBuf.length) {		/* get 4 bytes of length to start with */
	len = ntohl(netBuf.length);		/* length in host byte order */
	if (len >= maxLen) {
	    fprintf(stderr, "ERROR in %s: received message is too long: %d >= %d\n", iC_iccNM, len, maxLen);
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
} /* rcvd_msg_from_server */

/********************************************************************
 *
 *	Send message to server
 *	(handles short messages only)
 *
 *******************************************************************/

void
iC_send_msg_to_server(int sock, const char * msg)
{
    NetBuffer	netBuf;
    size_t	len = strlen(msg);

    if (len >= sizeof netBuf.buffer) {
	fprintf(stderr, "ERROR in %s: message to send is too long: %d >= %d\n", iC_iccNM, len, sizeof netBuf.length);
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
} /* send_msg_to_server */

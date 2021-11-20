static const char RCS_Id[] =
"@(#)$Id: tcpc.c 1.33 $";
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
 *	tcpc.c
 *	TCP/IC communication support
 *
 *	M. J. Bach, The Design of the UNIX Operating System
 *	11.4 Sockets, p. 388
 *
 *	W. R. Stevens, TCP/IP Illustrated, Vol 3, p 10
 *
 *   Modification John Wulff 1.23 2012.11.06 - disable Nagle's algorithm
 *	From <en.wikipedia.org/wiki/Nagle's_algorithm>
 *	Applications that expect real time response can react poorly to
 *	Nagle's algorithm. The algorithm purposefully delays transmission,
 *	increasing bandwidth efficiency at the expense of latency.
 *	For this reason applications with low bandwidth time sensitive
 *	transmissions typically use TCP_NODELAY to bypass the Nagle delay.
 *
 *	This very much applies to network traffic in the immediate C system.
 *	Messages are typically 5 to 10 bytes long staggered at intervals
 *	which are usually 10's of milliseconds to seconds apart, but can
 *	involve a burst of several messages as a result of the same trigger.
 *	Nagle's algorithm sends the first if these bursts and delays the
 *	rest for up to 500 ms or when an unrelated return message comes,
 *	which is much too late.	(Also changed for Perl code in iCmsg.pm)
 *
 *   Modification John Wulff 1.24 2014.03.31 - change from select() to poll().
 *	This change is required for Raspberry Pi GPIO interrupts, because
 *	the interrupts generated are classed as out-of-band input, which
 *	is signalled by POLLPRI instead od POLLIN using poll(). See:
 *	https://developer.ridgerun.com/wiki/index.php/How_to_use_GPIO_signals
 *
 *   Modification John Wulff 1.25 2015.02.08 - change again poll() to select().
 *	Using the 3rd exceptiom parameter using select() works for GPIO
 *	out-of-band interrupts. Since select() maintains correct timing,
 *	select() is preferable to poll() and was made the default for iC.
 *
 *   Modification John Wulff 1.25 2015.03.05 - when starting iCserver
 *	extend retries from 10 to 50 - wait 10 seconds max - will break
 *	break within 200 ms of connecting.
 *
 *   Modification John Wulff 1.26 2015.08.20 - when starting iCserver
 *	while running process is SUID root, execute iCserver at uid priveleges.
 *	Not doing this causes iCbox to throw an error message:
 *		Insecure $ENV{PATH} while running setuid at
 *		/usr/local/bin/iCserver line 254.
 *
 *   Modification John Wulff 1.29 2018.06.04 - when starting iCserver
 *	do not use -a (autovivify) if stdin is not a terminal.
 *	This allows starting an auxiliary iC app with piped input.
 *
 *******************************************************************/

#include	<stdio.h>
#include	<signal.h>
#include	<unistd.h>
#include	<netinet/tcp.h>
#ifdef	_WIN32
#include	<Time.h>
#else	/* ! _WIN32 Linux */
#include	<netdb.h>
#include	<sys/time.h>
#endif	/* _WIN32 */
#include	<ctype.h>
#include	<errno.h>
#include	<assert.h>
#include	<string.h>
#include	<time.h>
#include	"tcpc.h"
#include	"icc.h"

const char *	iC_hostNM = LOCALHOST1;		/* localhost or 127.0.0.1 */
const char *	iC_portNM = iC_PORT;		/* 8778 iC well known port */
char *		iC_iccNM  = "stdin";		/* immcc name qualified with instance */
char *		iC_iidNM  = "";			/* instance ID */
SOCKET		iC_sockFN = -1;			/* TCP/IP socket file number */
int		iC_Xflag  = 0;			/* 1 if this process started iCserver */

int		iC_maxFN = 0;
fd_set		iC_rdfds;
fd_set		iC_exfds;

/********************************************************************
 *
 *	private structures and variables
 *
 *	NetBuffer matches buffer used in Perl module msg.pm
 *
 *******************************************************************/

typedef struct NetBuffer {
#if INT_MAX == 32767 && defined (LONG16)
    long	length;
#else
    int		length;
#endif
    char	buffer[REPLY+4];
} NetBuffer;

#ifdef	_WIN32
static int	freqFlag;
static double	frequency;
static LARGE_INTEGER freq, start, end;
#else	/* not  _WIN32 */
static struct timeval	mt0;
static struct timeval	mt1;
static struct timespec	ms200 = { 0, 200000000, };
#endif	/* _WIN32 */
#if YYDEBUG && !defined(_WINDOWS)

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
} /* iC_microReset */

void
iC_microPrint(const char * str, int mask)
{
    if (iC_debug & 0100) fprintf(iC_outFP, "\n");
    long	sec;
    long	usec;

    if (gettimeofday(&mt1, 0) == 0) {
	sec = mt1.tv_sec - mt0.tv_sec;
	usec = mt1.tv_usec - mt0.tv_usec;
	if (usec < 0) {
	    sec--;
	    usec += 1000000;
	}
	fprintf(iC_outFP, "%3ld.%03ld,%03ld: %s\n", sec, usec/1000, usec%1000, str);
    }
    fflush(iC_outFP);
    gettimeofday(&mt0, 0);			/* start of next measurement without print time */
    iC_micro &= ~mask;
} /* iC_microPrint */
#endif	/* YYDEBUG && !defined(_WINDOWS) */

/********************************************************************
 *
 *	Connect to server 'host' and 'port' as a client
 *	Use 'iccNM' for messages (do not register with server)
 *	Return:	socket file number
 *
 *******************************************************************/

#define TLEN	100

SOCKET
iC_connect_to_server(const char *	host,
		     const char *	port)
{
    SOCKET		sock;
    struct in_addr	sin_addr;
    unsigned short int	sin_port;
    struct sockaddr_in	server;
    int			flag = 1;		// support setting TCP_NODELAY
    int			r;
    int			len;
    char 		cp[TLEN];
#ifdef	_WIN32
    WORD		wVersionRequested;
    WSADATA		wsaData;
    int			err;

    wVersionRequested = MAKEWORD( 2, 2 );	// alternative MAKEWORD( 1, 0 );

    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
	/* Tell the user that we could not find a usable */
	/* WinSock DLL.                                  */
	fprintf(iC_errFP, "WSAStartup failed: %d\n", err);
	iC_quit(SIGUSR1);
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
	iC_quit(SIGUSR1);
    }

    /* The WinSock DLL is acceptable. Proceed. */
#endif	/* _WIN32 */

    if (isdigit(*host)) {
#ifdef	_WIN32
	/* inet_addr() obselete under Linux because of 255.255.255.255 error return */
	if ((sin_addr.S_un.S_addr = inet_addr(host)) == INADDR_NONE)
#else	/* ! _WIN32 Linux */
	if (inet_aton(host, &sin_addr) == 0)
#endif	/* _WIN32 */
	{
	    fprintf(iC_errFP, "inet_aton with '%s' failed\n", host);
	    iC_quit(SIGUSR1);
	}
    } else {
	struct hostent *	pHost;
	if ((pHost = gethostbyname(host)) == NULL) {
	    fprintf(iC_errFP, "gethostbyname with '%s' failed\n", host);
	    iC_quit(SIGUSR1);
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
	    iC_quit(SIGUSR1);
	}
	/* getservbyname returns s_port in network byte order */
	sin_port = pServ->s_port;
    }

#ifdef	_WIN32
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
	fprintf(iC_errFP, "socket failed: %d\n", WSAGetLastError());
	iC_quit(SIGUSR1);
    }
#else	/* ! _WIN32 Linux */
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
	perror("socket failed");
	iC_quit(SIGUSR1);
    }
#endif	/* _WIN32 */

    if (setsockopt(sock,		/* SOCKET affected */
		   IPPROTO_TCP,		/* at tcp level */
		   TCP_NODELAY,		/* disable Nagles algorithm */
		   (char*)&flag,	/* historical crud */
		   sizeof(int))) {	/* length of option value */
	perror("setsockopt failed");
	iC_quit(SIGUSR1);
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr = sin_addr;
    server.sin_port = sin_port;

    for (r = 0; connect(sock, (SA)&server, sizeof(server)) < 0; r++) {
	if (r < 50 ||  errno != ECONNREFUSED) {	/* wait 10 seconds max - will break within 200 ms of connecting */
	    if (r == 0) {
		if (strcmp(inet_ntoa(server.sin_addr), "127.0.0.1") == 0) {
		    len = snprintf(cp, TLEN, "iCserver -kz%s%s",
		        (isatty(fileno(stdin))) ? "a" : "",	/* [autovivify] if stdin is a terminal (not pipe) */
			(iC_debug & DQ)         ? "q" : "");	/* [quiet] */
		    assert(len < sizeof cp);		/* unlikely since message is fixed size */
		    iC_fork_and_exec(iC_string2argv(cp, 2));	/* fork iCserver -kz[a][q] block STDIN */
		    iC_Xflag = 1;		/* this process started iCserver */
		} else {
		    fprintf(iC_errFP, "ERROR: %s: '%s:iCserver -p %s -akz' cannot be started here - start it on '%s' to connect\n",
			iC_iccNM, host, port, host);
		    iC_quit(SIGUSR1);
		}
	    }
#ifdef	_WIN32
	    Sleep(200);				/* 200 ms in ms */
#else	/* ! _WIN32 Linux */
	    nanosleep(&ms200, NULL);
#endif	/* _WIN32 */
	} else {
	    fprintf(iC_errFP, "ERROR in %s: client could not be connected to server '%s:%d'\n",
		iC_iccNM, inet_ntoa(server.sin_addr), ntohs(server.sin_port));
	    perror("connect failed");
	    iC_quit(SIGUSR1);
	}
    }

    if (iC_osc_lim != 0 && (iC_debug & DQ) == 0) { /* suppress connection info for unlimited oscillations */
	fprintf(iC_errFP, "'%s' connected to server at '%s:%d'\n",
	    iC_iccNM, inet_ntoa(server.sin_addr), ntohs(server.sin_port));
    }

    return sock;
} /* iC_connect_to_server */

/********************************************************************
 *
 *	Wait for next selected input, optional extra input or timer event
 *
 *******************************************************************/

int
iC_wait_for_next_event(fd_set * infdsp, fd_set * ixfdsp, struct timeval * ptv)
{
    fd_set *	exfdsp;
    int		retval;

    do {				/* repeat for caught signal */
	iC_rdfds = *infdsp;
	if (ixfdsp) {
	    exfdsp = &iC_exfds;
	    iC_exfds = *ixfdsp;
	} else {
	    exfdsp = 0;
	}
    } while ((retval = select(iC_maxFN + 1, &iC_rdfds, 0, exfdsp, ptv)) == -1 && errno == EINTR);

#ifdef	_WIN32
    if (retval == -1) {
	fprintf(iC_errFP, "ERROR: select failed: %d\n", WSAGetLastError());
	iC_quit(SIGUSR1);
    }
#endif	/* _WIN32 */

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
	iC_quit(SIGUSR1);
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
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 02) {
		fprintf(iC_outFP, "%s < '%s'\n", iC_iccNM, buf);	/* trace recv buffer */
		fflush(iC_outFP);
	    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	}
#if YYDEBUG && !defined(_WINDOWS)
    } else if (iC_debug & 02) {
	fprintf(iC_outFP, "%s < received zero length message\n", iC_iccNM);
	fflush(iC_outFP);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
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

    assert(len < sizeof netBuf.buffer);		/* check when sending - should not trunctate message */
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 01) {
	fprintf(iC_outFP, "%s > '%s'\n", iC_iccNM, msg);	/* trace send buffer */
	fflush(iC_outFP);
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    netBuf.length = htonl(len);
    memcpy(netBuf.buffer, msg, len + 1);
    len += sizeof netBuf.length;
    if (send(sock, (char*)&netBuf, len, 0) != len) {
	perror("send failed");
	iC_quit(SIGUSR1);
    }
} /* iC_send_msg_to_server */

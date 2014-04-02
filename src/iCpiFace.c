/********************************************************************
 *
 *	Copyright (C) 2014  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	iCpiFace
 *	I/O client for iCserver handling real 8 bit inputs and 8 bit outputs
 *	from one or more  PiFace controllers attached to a Raspberry Pi
 *
 *	Up to 8 PiFace controllers can be handled - 4 on one PiRack
 *
 *******************************************************************/

#include	"tcpc.h"
#include	<signal.h>
#include	<assert.h>
#include	<errno.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#ifndef	SELECT 
#include	<poll.h>
#endif	/* SELECT */

#if !defined(RASPBERRYPI) || !defined(TCP) || defined(_WINDOWS)
#error - must be compiled with RASPBERRYPI and TCP defined and not _WINDOWS
#else	/* defined(RASPBERRYPI) && defined(TCP) && !defined(_WINDOWS) */

#include	"icc.h"
#include	"wiringPi.h"

#define QUIT_TERMINAL	(SIGRTMAX+3)
#define QUIT_DEBUGGER	(SIGRTMAX+4)
#define QUIT_SERVER	(SIGRTMAX+5)

#define IOCHANNELS	30		/* initial Channels[] size */
#define ENTRYSZ		16		/* entry "SIX0000-000\0" max length 12 */
#define	INSTSIZE	3		/* limit instance to 3 characters */
#define MAX_PIFACE_AD	8		/* maximum number of PiFace's */
#define BS		64		/* buffer size for I/O strings */

#undef	max
#define	max(x, y) ((x) > (y) ? (x) : (y))

extern void		iC_initIO(void);
static void		storeChannel(unsigned short channel, int pa);
extern void		doEdge(int pin);
extern void		doUnexport(int pin);
static int		gpio_fd_open(unsigned int gpio);

static const char *	usage =
"Usage:\n"
" %s [-tmh][ -s <server>][ -p <port>][ -n <name>][ -i <inst>]\n"
"	  [ -d <deb>][ [<ID>[-<ID>]]|[<ID>[-<ID>]-<inst>] ...]\n"
"	-s host      host ID of server       (default '%s')\n"
"	-p port      service port of server  (default '%s')\n"
"	-n name      of this client          (default X0)\n"
"	-i inst      instance of this client (default '%s'; 1 to %d digits)\n"
"	ID	X0 X1 D X7\n"
"		For the PiFace only pairs of 8 bit digital input and output\n"
"		with the same index for each PiFace unit can be generated.\n"
"		The number n of argument Xn is the IEC1131 ID. The position\n"
"		of the Xn argument is the piFace address selected by jumpers.\n"
"		For testing, dummy ID D may be before or between real ID's Xn.\n"
"		So for the above list X0 X1 D X7 the following is generated:\n"
"		    inputs IX0.0-7 and outputs QX0.0-7 for PiFace address 0\n"
"		    inputs IX1.0-7 and outputs QX1.0-7 for PiFace address 1\n"
"		    no inputs and outputs are generated for address 2\n"
"		    inputs IX7.0-7 and outputs QX7.0-7 for PiFace address 3\n"
"		Default ID is X0 generating inputs IX0.0-7 and outputs QX0.0-7\n"
"		A positive range of ID's can be specified with - eg X0-X3\n"
"	ID-inst	Each individual ID or range can be followed by -<inst>, where\n"
"		<inst> consists of up to 3 numeric digits (usually 1 digit).\n"
"		Such an appended instance code takes precedence over the\n"
"		general instance specified with the -i <inst> option above.\n"
"					TRACE options\n"
"	-t	trace all I/O activity\n"
"       -m	display microsecond timing info\n"
"	-d deb	+1   trace send   action\n"
"		+2   trace rcv    action\n"
"		+10  trace input  action\n"
"		+20  trace output action\n"
"		+100 show arguments	DEBUG options\n"
"		+200 show debugging details\n"
"	-h	help, ouput this Usage text only\n"
"Copyright (C) 2014 John E. Wulff     <immediateC@gmail.com>\n"
"Version	$Id: iCpiFace.c,v 1.2 2014/04/02 05:16:10 pi Exp $\n"
;

char *		iC_progname;		/* name of this executable */
static char *	iC_path;
static char *	iC_fullProgname;
short		iC_debug = 0;
int		iC_micro = 0;
unsigned short	iC_osc_lim = 1;

int		piFaceAd = 0;

/********************************************************************
 *
 *	piFaceIO array pf[piFaceAd][iotype] is organised as follows:
 *	  [piFaceAd] is the address selected by jumpers (range 0 - 7)
 *	  [iotype]   is the either [0] for input or [1] for output
 *	  each element of the array is a struct piFaceIO
 *
 *******************************************************************/

typedef struct	piFaceIO {
    char *		iecName;	/* iec name IXn, QXn, IXn-i  or QXn-i */
    unsigned short	channel;	/* the channel number supplied by iCserver */
} piFaceIO;

piFaceIO	pf[MAX_PIFACE_AD][2];	/* piFace iec names and channels */
int *		Channels = NULL;	/* dynamic array to store piFace input addresses */
int		ioChannels = 0;		/* allocated size of Channels[] */
static unsigned short	topChannel = 0;
static char	regBuf[REPLY];		/* data sent to iCserver */
static char	rpyBuf[REPLY];		/* data received from iCserver */
static char	spiBuf[REPLY];		/* data received from SPI read */

char		inpBuf[BS];
char		outBuf[BS];

FILE *		iC_outFP;		/* listing file pointer */
FILE *		iC_errFP;		/* error file pointer */
short		errorFlag;

static SOCKET	sockFN = -1;		/* TCP/IP socket file number */
static int	gpioFN = -1;		/* /sys/class/gpio SPI file number */
static int	spidFN = -1;		/* /dev/spidev0.0 SPI file number */

/********************************************************************
 *
 *	Main program
 *
 *******************************************************************/

int
main(
    int		argc,
    char **	argv)
{
    int		n;
    int		pa;
    int		len;
    int		regBufLen = 0;
    char *	cp;
    unsigned short	channel;
    int		retval;
#ifndef	SELECT
    int		soc;			/* TCP/IP socket pollfd array index */
    int		spi;			/* SPI out-of-band pollfd array index */
    int		sti;			/* STDIN pollfd array index */
#endif	/* SELECT */

    iC_outFP = stdout;			/* listing file pointer */
    iC_errFP = stderr;			/* error file pointer */
    iC_initIO();			/* catch memory access signal */

    /********************************************************************
     *
     *	Determine program name
     *
     *******************************************************************/

    iC_path = argv[0];			/* in case there is a leading path/ */
    len = strlen(iC_path);			/* strlen(outBuf) == len */
    iC_fullProgname = iC_emalloc(len+1);
    strcpy(iC_fullProgname, iC_path);
    if ((iC_progname = strrchr(iC_path, '/')) == NULL &&
        (iC_progname = strrchr(iC_path, '\\')) == NULL
    ) {
	iC_progname = iC_path;		/* no leading path */
	iC_path     = "";		/* default path */
    } else {
	*iC_progname++ = '\0';		/* path has been stripped and isolated */
    }
    iC_iccNM = NULL;			/* generate our own name - not 'stdin' from tcpc.c */

    if (geteuid() != 0) {
	fprintf(iC_errFP, "ERROR: %s: Must be root to run. Program should be suid root\n", iC_progname) ;
	exit(1);
    }

    /********************************************************************
     *
     *	Process program arguments
     *
     *******************************************************************/

    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
		    int		debi;
		    int		slen;
		case 's':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (strlen(*argv)) iC_hostNM = *argv;
		    goto break2;
		case 'p':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (strlen(*argv)) iC_portNM = *argv;
		    goto break2;
		case 'n':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (strlen(*argv)) iC_iccNM = *argv;
		    goto break2;
		case 'i':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if ((slen = strlen(*argv)) > INSTSIZE ||
			slen != strspn(*argv, "0123456789")) {
			fprintf(iC_errFP, "ERROR '-i %s' is non numeric or longer than %d characters\n",
			    *argv, INSTSIZE);
			goto error;
		    }
		    iC_iidNM = *argv;
		    goto break2;
		case 'm':
		    iC_micro++;		/* microsecond info */
		    break;
		case 'd':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if ((slen = strlen(*argv)) != strspn(*argv, "0123") ||	/* octal digits only */
			sscanf(*argv, "%o", &debi) != 1 ||
			debi & ~0333) {
			fprintf(iC_errFP, "ERROR '-d %s' is not a well formed octal string or exceeds range 333\n", *argv);
			goto error;
		    }
		    iC_debug |= debi;	/* short */
		    goto break2;
		case 't':
		    iC_debug |= 033;	/* trace all activity */
		    break;
		default:
		    fprintf(iC_errFP,
			"%s: unknown command line switch '%s'\n", iC_progname, *argv);
		case 'h':
		case '?':
	      error:
		    fprintf(iC_errFP, usage, iC_progname, iC_hostNM, iC_portNM, iC_iidNM, INSTSIZE);
		    exit(1);
		}
	    } while (*++*argv);
	  break2: ;
	} else {
	    /* analyse ID argument(s) */
	    int	iecStart = 0;
	    int	iecEnd   = 0;

	    if (piFaceAd >= MAX_PIFACE_AD) {
	      tooManyArgs:
		fprintf(iC_errFP, "ERROR 'Too many ID arguments (%d max)\n", MAX_PIFACE_AD);
		goto error;
	    }
	    if (strcmp(*argv, "D") == 0) {
		if (piFaceAd >= MAX_PIFACE_AD - 1) goto tooManyArgs;
		pf[piFaceAd][0].iecName = 0;				/* Dummy place holder ID D */
		pf[piFaceAd][1].iecName = 0;
	    	piFaceAd++;
	    } else {
		char	tail[32];
		int	iid = -1;

		if ((n = sscanf(*argv, "X%5d%31s", &iecStart, tail)) == 1) {
		    iecEnd = iecStart;				/* only a single Xn ID */
		} else if (n == 2) {
		    if ((n = sscanf(tail, "-X%5d%31s", &iecEnd, tail)) == 0) {
			iecEnd = iecStart;			/* a single Xn-i ID with instance ID */
		    }
		    if (iC_debug & 0200) fprintf(iC_outFP, "n = %d	iecStart = %d	iecEnd = %d	tail = %s\n", n, iecStart, iecEnd, tail);
		    if (n != 1 && (n = sscanf(tail, "-%3d%31s", &iid, tail)) != 1) {
			goto illFormed;
		    }
		    if (iC_debug & 0200) fprintf(iC_outFP, "n = %d	iecStart = %d	iecEnd = %d	tail = %s\n", n, iecStart, iecEnd, tail);
		} else if (n == 0) {
		  illFormed:
		    fprintf(iC_errFP, "ERROR '%s' is not a well formed ID\n", *argv);
		    goto error;
		}
		if (iecEnd < iecStart) {
		    fprintf(iC_errFP, "ERROR '%s' is not a positive ID range\n", *argv);
		    goto error;
		}
		for (n = iecStart; n < iecEnd+1; n++) {
		    if (iid >= 0) {
			snprintf(inpBuf, BS, "IX%d-%d", n, iid);/* individual instance has precedence */
			snprintf(outBuf, BS, "QX%d-%d", n, iid);
		    } else if (strlen(iC_iidNM) > 0) {
			snprintf(inpBuf, BS, "IX%d-%s", n, iC_iidNM);
			snprintf(outBuf, BS, "QX%d-%s", n, iC_iidNM);
		    } else {
			snprintf(inpBuf, BS, "IX%d", n);
			snprintf(outBuf, BS, "QX%d", n);
		    }
		    for (pa = 0; pa < piFaceAd; pa++) {
			if ((cp = pf[pa][0].iecName) && strcmp(inpBuf, cp) == 0) {
			    fprintf(iC_errFP, "WARNING attempt to define %s and %s more than once - ignore\n", inpBuf, outBuf);
			    errorFlag++;
			}
		    }
		    if (errorFlag) {
			errorFlag = 0;
			continue;
		    }
		    len = strlen(inpBuf);			/* strlen(outBuf) == len */
		    pf[piFaceAd][0].iecName = iC_emalloc(len+1);
		    strcpy(pf[piFaceAd][0].iecName, inpBuf);
		    pf[piFaceAd][1].iecName = iC_emalloc(len+1);
		    strcpy(pf[piFaceAd][1].iecName, outBuf);
		    piFaceAd++;
		    regBufLen += len + len + 4;			/* ready to copy iecId's into regBuf */
		}
	    }
	}
    }
    if (iC_debug & 0200)  fprintf(iC_outFP, "fullPath = '%s' path = '%s' progname = '%s'\n", iC_fullProgname, iC_path, iC_progname);
    if (regBufLen == 0) {
	if (strlen(iC_iidNM) > 0) {				/* build default IX0 and QX0 */
	    snprintf(inpBuf, BS, "IX0-%s", iC_iidNM);
	    snprintf(outBuf, BS, "QX0-%s", iC_iidNM);
	} else {
	    snprintf(inpBuf, BS, "IX0");
	    snprintf(outBuf, BS, "QX0");
	}
	len = strlen(inpBuf);					/* strlen(outBuf) == len */
	pf[piFaceAd][0].iecName = iC_emalloc(len+1);
	strcpy(pf[piFaceAd][0].iecName, inpBuf);
	pf[piFaceAd][1].iecName = iC_emalloc(len+1);
	strcpy(pf[piFaceAd][1].iecName, outBuf);
	piFaceAd++;
	regBufLen += len + len + 4;				/* ready to copy iecId's into regBuf */
    }
    if (iC_iccNM == NULL) {
	strncpy(inpBuf, "PIFACE", BS);				/* generate a meaningful name */
	for (pa = 0; pa < piFaceAd; pa++) {
	    if (pf[pa][0].iecName) {
		len = strlen(inpBuf);
		cp = inpBuf + len;
		snprintf(cp, BS-len, "_%d", pa);
		if (iC_debug & 0100) fprintf(iC_outFP, "%22s	%s	%s\n", inpBuf, pf[pa][0].iecName, pf[pa][1].iecName);
	    }
	}
	len = strlen(inpBuf);	
	iC_iccNM = iC_emalloc(len+1);
	strcpy(iC_iccNM, inpBuf);
    }
    if (iC_micro) iC_microReset(0);	/* start timing */
    if (iC_debug & 0100)  fprintf(iC_outFP, "host = %s, port = %s, name = %s, instance = '%s'\n", iC_hostNM, iC_portNM, iC_iccNM, iC_iidNM);

    /********************************************************************
     *
     *	All arguments have been taken care of
     *
     *	Start TCP/IP communication, build registration string and register
     *
     *******************************************************************/

    sockFN = iC_connect_to_server(iC_hostNM, iC_portNM);

    regBufLen += strlen(iC_iccNM) + 4 + 4;			/* really ready to copy iecId's into regBuf  Nname...,Z\n */
    assert(regBufLen <= REQUEST);				/* should fit easily with name and 16 I/Os */
    if (strlen(iC_iidNM) == 0) {
	len = snprintf(regBuf, regBufLen, "N%s", iC_iccNM);			/* name header */
    } else {
	len = snprintf(regBuf, regBufLen, "N%s-%s", iC_iccNM, iC_iidNM);	/* name-instance header */
    }
    cp = regBuf + len;
    regBufLen -= len;
    for (pa = 0; pa < piFaceAd; pa++) {
	if (pf[pa][0].iecName) {
	    len = snprintf(cp, regBufLen, ",S%s,R%s", pf[pa][0].iecName, pf[pa][1].iecName);
	    cp += len;						/* send and receive I/O */
	    regBufLen -= len;
	    assert(regBufLen > 2);
	}
    }
    if (iC_debug & 0200)  fprintf(iC_outFP, "regBufLen = %d\n", regBufLen);
    snprintf(cp, regBufLen, ",Z");				/* Z terminator */
    if (iC_debug & 0100)  fprintf(iC_outFP, "register:%s\n", regBuf);
    if (iC_micro) iC_microPrint("send registration", 0);
    iC_send_msg_to_server(sockFN, regBuf);			/* register IOs with iCserver */
    if (iC_rcvd_msg_from_server(sockFN, rpyBuf, REPLY)) {	/* busy wait for reply */
	if (iC_micro) iC_microPrint("ack received", 0);
	if (iC_debug & 0100) fprintf(iC_outFP, "reply:%s\n", rpyBuf);
	/* distribute channels returned in iCserver acknowledgment */
	cp = rpyBuf - 1;
	for (pa = 0; pa < piFaceAd; pa++) {
	    char *	np;

	    if ((np = pf[pa][0].iecName) != NULL) {
		for (n = 0; n < 2; n++) {
		    assert(cp);
		    channel = atoi(++cp);
		    assert(channel > 0);
		    if (channel > topChannel) {
			topChannel = channel;
		    }
		    if (n == 0) {
			assert(*np == 'I');	
			if (iC_debug & 0100) fprintf(iC_outFP, "piFace '%d' input  on channel %hu (%s)\n",
			    pa, channel, pf[pa][0].iecName);
			pf[pa][0].channel = channel;		/* link output channel to piFace input address */
			storeChannel(channel, pa);		/* link piFace input address to output channel */
		    } else {
			assert(n == 1 && *(pf[pa][1].iecName) == 'Q');	
			if (iC_debug & 0100) fprintf(iC_outFP, "piFace '%d' output on channel %hu (%s)\n",
			    pa, channel, pf[pa][1].iecName);
			pf[pa][1].channel = channel;		/* link input channel to piFace output address */
			storeChannel(channel, pa+010);		/* link piFace output address to input channel */
		    }
		    cp = strchr(cp, ',');
		}
	    }
	}
	assert(cp == NULL);					/* Ack string matches Registration */
	if (iC_debug & 0100) fprintf(iC_outFP, "reply: top channel = %hu\n", topChannel);
    } else {
	iC_quit(QUIT_SERVER);					/* quit normally with 0 length message */
    }

    /********************************************************************
     *
     *	Setup PiFace
     *
     *	Initialisation of /dev/spidev0.0 for SPI modes and read/writeBytes
     *
     *  Initialisation using the /sys/class/gpio interface to the GPIO
     *	systems - slightly slower, but always usable as a non-root user
     *	export GPIO pin 25, which is INTB of the MCP23S17 device.
     *
     *	Write to all relevant MCP23S17 registers to setup PIFACE
     *
     *	Wait for Interrupts on GPIO pin 25 which has been exported and then
     *	opening /sys/class/gpio/gpio25/value
     *
     *	This is actually done via the /sys/class/gpio interface regardless of
     *	the wiringPi access mode in-use. Maybe sometime it might get a better
     *	way for a bit more efficiency.	(comment by Gordon Henderson)
     *
     *******************************************************************/

    doEdge(25);				/* TODO allow for 2 pins */
    if ((spidFN = wiringPiSetupPiFace()) < 0) {
	fprintf(iC_errFP, "ERROR: '%s' PiFace setup: %s\n", iC_iccNM, strerror(errno));
    }
    if ((gpioFN = gpio_fd_open(25)) < 0) {
	fprintf(iC_errFP, "ERROR: '%s' PiFace open gpio 25: %s\n", iC_iccNM, strerror(errno));
    }

    /********************************************************************
     *
     *	Set all bits to wait for interrupts
     *
     *******************************************************************/

#ifdef	SELECT 
    FD_ZERO(&iC_infds);			/* should be done centrally if more than 1 connect */
    FD_ZERO(&iC_ixfds);			/* should be done centrally if more than 1 connect */
    FD_SET(sockFN, &iC_infds);		/* watch sock for inputs */
    FD_SET(gpioFN, &iC_ixfds);		/* watch SPI for out-of-band input - do after iC_connect_to_server() */
#ifndef	WIN32
    FD_SET(0, &iC_infds);		/* watch stdin for inputs - FD_CLR on EOF */
    /* can only use sockets, not file descriptors under WINDOWS - use kbhit() */
#endif	/* WIN32 */
#else	/* ! SELECT */
    soc = pollSet(sockFN, POLLIN);	/* watch TCP/IP socket for inputs */
    spi = pollSet(gpioFN, POLLPRI);	/* watch SPI for out-of-band input - do after iC_connect_to_server() */
#ifndef	WIN32
    /* can only use sockets, not file descriptors under WINDOWS - use kbhit() */
    sti = pollSet(0, POLLIN);		/* watch stdin for inputs - pollClr(sti) on EOF */
#endif	/* WIN32 */
#endif	/* SELECT */

    if (iC_debug & 0200) fprintf(iC_outFP, "sockFN = %d	gpioFN = %d spidFN = %d\n", sockFN, gpioFN, spidFN);

    /********************************************************************
     *
     *  External input (TCP/IP via socket and SIO from PiFace)
     *  Wait for input in a poll/select statement most of the time
     *
     *******************************************************************/

    for (;;) {
#ifdef	SELECT 
	if ((retval = iC_wait_for_next_event(max(sockFN, gpioFN), NULL)) > 0)	/* no timer events */
#else	/* ! SELECT */
	if ((retval = iC_wait_for_next_event(-1)) > 0)		/* no timer events */
#endif	/* SELECT */
	{
	    int x;
	    int		val;

#ifdef	SELECT 
	    if (FD_ISSET(sockFN, &iC_rdfds))
#else	/* ! SELECT */
	    if (pollEvent(soc, POLLIN))
#endif	/* SELECT */
	    {
		/********************************************************************
		 *  TCP/IP input
		 *******************************************************************/
		if (iC_rcvd_msg_from_server(sockFN, rpyBuf, REPLY)) {
		    if (iC_micro) iC_microPrint("TCP input received", 0);
		    assert(Channels);
		    cp = rpyBuf - 1;
		    do {
			if ((n = sscanf(++cp, "%hu:%d", &channel, &val)) == 2 &&
			    channel > 0	&&
			    channel <= topChannel &&
			    (Channels[channel] & ~07) == 010
			) {
			    assert(Channels[channel] == 010);	/* TODO this assumes we received for PiFace 0 */
			    writeBytePiFaceA(val);	/* write data to PiFace A output */
			} else {
			    fprintf(iC_errFP, "ERROR: n = %d, channel = %hu, topChannel = %hu, val = %d, '%s'\n",
				n, channel, topChannel, val, cp);
			    break;
			}
		    } while ((cp = strchr(cp, ',')) != NULL);
		} else {
		    iC_quit(QUIT_SERVER);	/* quit normally with 0 length message from iCserver */
		}
	    }
#ifdef	SELECT 
	    if (FD_ISSET(gpioFN, &iC_exfds))	/* watch for out-of-band GPIO input */
#else	/* ! SELECT */
	    if (pollEvent(spi, POLLPRI))	/* watch for out-of-band GPIO input */
#endif	/* SELECT */
	    {
		/********************************************************************
		 *  SPI input
		 *******************************************************************/
		/********************************************************************
		 *  Do a dummy read to clear the interrupt on /dev/class/gpio25/value
		 *  A one character read appars to be enough (according to Gordon Henderson)
		 *******************************************************************/
		read (gpioFN, &x, 1);		/* dummy read to clear interrupt on /dev/class/gpio25/value */
		if (iC_micro) iC_microPrint("SPI input received", 0);
		val = readBytePiFaceIntCapB();	/* read of PiFace B at the time of the INTB interrupt */
		/* TODO this assumes we received from PiFace 0 */
		snprintf(regBuf, REPLY, "%hu:%d", pf[0][0].channel, val);	/* data telegram */
		iC_send_msg_to_server(sockFN, regBuf);	/* send data value to iCserver */
	    }
#ifdef	SELECT 
	    if (FD_ISSET(0, &iC_rdfds))
#else	/* ! SELECT */
	    if (pollEvent(sti, POLLIN))
#endif	/* SELECT */
	    {
		/********************************************************************
		 *  STDIN
		 *******************************************************************/
		if ((n = getchar()) == EOF) {
		    fprintf(iC_errFP, "EOF received in stdin\n");
#ifdef	SELECT 
		    FD_CLR(0, &iC_infds);		/* ignore EOF - happens in bg */
#else	/* ! SELECT */
		    pollClr(sti);			/* ignore EOF - happens in bg */
#endif	/* SELECT */
		} else if (n == 'q') {
		    iC_quit(QUIT_TERMINAL);		/* quit normally */
		} else if (n == 't') {
		    iC_debug ^= 0100;			/* toggle -t flag */
		} else if (n == 'm') {
		    iC_micro ^= 1;			/* toggle micro */
		} else if (n == 'a') {
		    					/* dummy SPI interrupt */
		    x = digitalReadPiFace(0);
		    fprintf(stderr, "SPI 0 = %d\n", x);
		}
	    }
	} else if (retval == 0) {
	    fprintf(iC_errFP, "ERROR: timer event from select ???\n");
	    iC_quit(SIGUSR1);			/* should definitely not happen */
	} else {
	    perror("ERROR: select failed");	/* retval -1 */
	    iC_quit(SIGUSR1);
	}
    }
} /* main */

/********************************************************************
 *
 *	Initialize IO
 *
 *******************************************************************/

void
iC_initIO(void)
{
    signal(SIGSEGV, iC_quit);			/* catch memory access signal */
} /* iC_initIO */

/********************************************************************
 *
 *	Initalise and expand dynamic array Channels[] as necessary
 *	Store input piFace addresses in Channels[]
 *
 *******************************************************************/

static void
storeChannel(unsigned short channel, int pa)
{
    while (channel >= ioChannels) {
	Channels = (int *)realloc(Channels,	/* initially NULL */
	    (ioChannels + IOCHANNELS) * sizeof(int));
	assert(Channels);
	memset(&Channels[ioChannels], '\0', IOCHANNELS * sizeof(int));
	ioChannels += IOCHANNELS;	/* increase the size of the array */
	if (iC_debug & 0200) fprintf(iC_outFP, "storeChannel: Channels[%d] increase\n", ioChannels);
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "storeChannel: Channels[%d] <= %d\n", channel, pa);
    Channels[channel] = pa;		/* store piFace input address */
} /* storeChannel */

/********************************************************************
 *
 *	Quit program with 'q' or ctrlC or Break via signal SIGINT
 *	or program abort on detected bugs.
 *
 *******************************************************************/

void
iC_quit(int sig)
{
    if (spidFN > 0) {
	close(spidFN);				/* close connection to /dev/spidev0.0 */
    }
    if (gpioFN > 0) {
	close(gpioFN);				/* close connection to /sys/class/gpio/value */
    }
    doUnexport(25);				/* TODO allow for 2 pins */
    if (sockFN > 0) {
	close(sockFN);				/* close connection to iCserver */
    }
    if (sig == QUIT_TERMINAL) {
	fprintf(iC_errFP, "\n'%s' stopped from terminal\n", iC_iccNM);
    } else if (sig == QUIT_DEBUGGER) {
	fprintf(iC_errFP, "\n'%s' stopped by debugger\n", iC_iccNM);
    } else if (sig == QUIT_SERVER) {
	fprintf(iC_errFP, "\n'%s' disconnected by server\n", iC_iccNM);
    } else if (sig == SIGINT) {
	fprintf(iC_errFP, "\n'%s' stopped by interrupt from terminal\n", iC_iccNM);
    } else if (sig == SIGSEGV) {
	fprintf(iC_errFP, "\n'%s' stopped by 'Invalid memory reference'\n", iC_iccNM);
    } else if (sig == SIGUSR1) {
	fprintf(iC_errFP, "\n'%s' stopped by 'non-recoverable run-time error'\n", iC_iccNM);
    }
    if (iC_outFP != stdout) {
	fflush(iC_outFP);
	fclose(iC_outFP);
    }
    if (iC_errFP != stderr) {
	fflush(iC_errFP);
	fclose(iC_errFP);
    }
    exit(sig < QUIT_TERMINAL ? sig : 0);	/* really quit */
} /* iC_quit */

/****************************************************************
 * gpio_fd_open
 ****************************************************************/

static int
gpio_fd_open(unsigned int gpio)
{
    char	buf[32];

    snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", gpio);
    return open(buf, O_RDONLY | O_NONBLOCK );
} /* gpio_fd_open */

/********************************************************************
 *
 *   The following routines were extracted from gpio.c
 *   Copyright (c) 2012 Gordon Henderson
 *
 *   It seems tidier to use these routines directly to access the
 *   /sys/class/gpio device interface rather than execute 'gpio' from C
 *
 *   This version has been modified  by John E. Wulff	2014/03/25
 *   for the immediate C system to interface with a PiFace in
 *   accordance with the terms of the GNU Lesser General Public License
 *
 *******************************************************************/

/********************************************************************
 *
 *	changeOwner:
 *
 *	Change the ownership of the file to the real userId of the calling
 *	program so we can access it.
 *
 *******************************************************************/

static void
changeOwner(char *cmd, char *file)
{
    uid_t uid = getuid();
    uid_t gid = getgid();

    if (chown(file, uid, gid) != 0) {
	if (errno == ENOENT) {			/* Warn that it's not there */
	    fprintf(iC_errFP, "%s: Warning: File not present: %s\n", cmd, file);
	} else {
	    fprintf(iC_errFP, "%s: Unable to change ownership of %s: %s\n", cmd, file, strerror(errno));
	    iC_quit(SIGUSR1);
	}
    }
} /* changeOwner */

/********************************************************************
 *
 *	doEdge:
 *
 *	To enable interrupts from PiFace 0-3 inputs export GPIO pin 25
 *	To enable interrupts from PiFace 4-7 inputs export GPIO pin 24
 *	(the latter can be selected on a PiRack)
 *	in both cases set edge to falling
 *
 *	Easy access to changing the edge trigger on a GPIO pin
 *	This uses the /sys/class/gpio device interface.
 *
 *******************************************************************/

void
doEdge(int pin)
{
    FILE *	fd;
    char	fName[128];
    char	pName[128];

    assert(pin >= 0 && pin < 64);

    /* Export the pin and set direction to input */

    if ((fd = fopen("/sys/class/gpio/export", "w")) == NULL) {
	fprintf(iC_errFP, "%s: Unable to open GPIO export interface: %s\n", iC_progname, strerror(errno));
	iC_quit(SIGUSR1);
    }

    fprintf(fd, "%d\n", pin);
    fclose(fd);
    sprintf(fName, "/sys/class/gpio/gpio%d/direction", pin);
    if ((fd = fopen(fName, "w")) == NULL) {
	fprintf(iC_errFP, "%s: Unable to open GPIO direction interface for pin %d: %s\n", iC_progname, pin,strerror(errno));
	iC_quit(SIGUSR1);
    }

    fprintf(fd, "in\n");
    fclose(fd);

    /* Set the edge for interrupts to falling */

    sprintf(fName, "/sys/class/gpio/gpio%d/edge", pin);
    if ((fd = fopen(fName, "w")) == NULL) {
	fprintf(iC_errFP, "%s: Unable to open GPIO edge interface for pin %d: %s\n", iC_progname, pin,strerror(errno));
	iC_quit(SIGUSR1);
    }

    fprintf(fd, "falling\n");
    fclose(fd);

    /* Change ownership of the value and edge files, so the current user can actually use it! */

    sprintf(fName, "/sys/class/gpio/gpio%d/value", pin);
    changeOwner(iC_fullProgname, fName);

    sprintf(fName, "/sys/class/gpio/gpio%d/edge", pin);
    changeOwner(iC_fullProgname, fName);
} /* doEdge */

/********************************************************************
 *
 *	doUnexport:
 *
 *	This uses the /sys/class/gpio device interface.
 *
 *******************************************************************/

void
doUnexport(int pin)
{
    FILE *	fd;

    if ((fd = fopen("/sys/class/gpio/unexport", "w")) == NULL) {
	fprintf(iC_errFP, "%s: Unable to open GPIO unexport interface: %s\n", iC_progname, strerror(errno));
	iC_quit(SIGUSR1);
    }

    fprintf(fd, "%d\n", pin);
    fclose(fd);
} /* doUnexport */
#endif	/* defined(RASPBERRYPI) && defined(TCP) && !defined(_WINDOWS) */

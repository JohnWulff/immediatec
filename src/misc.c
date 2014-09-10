static const char misc_c[] =
"@(#)$Id: misc.c,v 1.13 2014/08/11 05:40:18 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2011  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	misc.c
 *	miscellanious functions shared by all modules
 *
 *******************************************************************/

#define		_GNU_SOURCE
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#ifdef	_MSDOS_
#include	<dos.h>
#include	<conio.h>
#else	/* ! _MSDOS_ Linux */
#include	<termios.h>
#include	<unistd.h>
#endif	/* ! _MSDOS_ Linux */
#include	<signal.h>
#include	<assert.h>
#ifdef	TCP
#include	"tcpc.h"
#endif	/* TCP */
#include	"icc.h"
#ifdef	RASPBERRYPI
#include	"rpi_gpio.h"
#include	"mcp23s17.h"
#ifdef	BCM2835
#include	"bcm2835.h"
#endif	/* BCM2835 */
#endif	/* RASPBERRYPI */

unsigned char	iC_bitMask[]    = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,	/* 0 1 2 3 4 5 6 7 */
};

#ifdef	TCP

#ifndef	EFENCE
char		regBuf[REQUEST];
char		rpyBuf[REPLY];		/* Buffer in which iCserver input is read to */
#else	/* EFENCE */
char *		regBuf;
char *		rpyBuf;
#endif	/* EFENCE */

#ifdef	RASPBERRYPI
int		iC_opt_P;
int		iC_opt_B;
int		iC_opt_E;
int		iC_opt_L;
int		npf;
int		gpio23FN = -1;		/* /sys/class/gpio/gpio23/value LIRC file number */
int		gpio25FN = -1;		/* /sys/class/gpio/gpio25/value SPI file number */
int		spidFN[2] = { -1, -1 };	/* /dev/spidev0.0, /dev/spidev0.1 SPI file numbers */
#else	/* ! RASPBERRYPI */
#ifndef	WIN32
#include	<time.h>
static struct timespec	ms200 = { 0, 200000000, };
#endif	/* ! WIN32 */
#endif	/* ! RASPBERRYPI */

#endif	/* TCP */

#ifndef	_MSDOS_
int		ttyparmFlag = 0;
struct termios	ttyparms;
#endif	/* ! _MSDOS_ Linux */

#ifdef	WIN32
#include	<io.h>
#include	<fcntl.h>
#include	<sys/stat.h>

/********************************************************************
 *
 * version of mkstemp() for Windows
 *
 *******************************************************************/

int
mkstemp(char * templ)
{
    /* must use _S_IWRITE, otherwise newly created file will be read only when closed */
    return open(mktemp(templ), _O_CREAT | _O_RDWR | _O_TRUNC, _S_IWRITE);
}
#endif	/* WIN32 */

/********************************************************************
 *
 * version of malloc() which checks return value and works under Windows
 *
 *******************************************************************/

void *
iC_emalloc(unsigned	nbytes)		/* check return from malloc */
{
    void *	bp;
#ifdef _WINDOWS
#if defined(_LARGE_) || defined(_HUGE_)
    GLOBALHANDLE		hglobal;

    if ((hglobal = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, nbytes)) == 0) {
	execerror("out of memory", NS, __FILE__, __LINE__);
    }
    bp = GlobalLock(hglobal);		/* actual pointer */
#else
    LOCALHANDLE		hlocal;

    if ((hlocal = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, nbytes)) == 0) {
	execerror("out of memory", NS, __FILE__, __LINE__);
    }
    bp = LocalLock(hlocal);		/* actual pointer */
#endif
#else

    if ((bp = malloc(nbytes)) == NULL) {
    }
#endif
    memset(bp, 0, nbytes);		/* when free() is used memory can be non zero */
    return bp;
} /* iC_emalloc */
#ifdef _WINDOWS

/********************************************************************
 *
 * if free under windows gives trouble, make this a null function
 *
 *******************************************************************/

void
iC_efree(void *	p)
{
#if defined(_LARGE_) || defined(_HUGE_)
    GLOBALHANDLE		hglobal;

    hglobal = GlobalHandle(p);		/* retrieve the handle */
    GlobalUnlock(hglobal);
    GlobalFree(hglobal);		/* big deal */
#else
    LOCALHANDLE		hlocal;

    hlocal = LocalHandle(p);		/* retrieve the handle */
    LocalUnlock(hlocal);
    LocalFree(hlocal);			/* big deal */
#endif
} /* iC_efree */
#endif
#if defined(RUN) || defined (TCP) || defined(LOAD)

/********************************************************************
 *
 *	Compare gt_ids in two Gates support of qsort()
 *
 *	change the collating position of '_' before digits ('.' '/')
 *	change all '_' to '/' to get correct ordering (/ never occurs in C variable)
 *	use strverscmp(), which puts sequences of digits in number order.
 *
 *******************************************************************/

#define		ABSIZE	256
static char *	aBuf = NULL;
static int	aSiz = 0;	/* dynamic size adjusted with realloc */
static char *	bBuf = NULL;
static int	bSiz = 0;	/* dynamic size adjusted with realloc */

int
iC_cmp_gt_ids( const Gate ** a, const Gate ** b)
{
    char *	ap;
    char *	bp;
    char *	cp;

    if ((cp = strchr(ap = (*a)->gt_ids, '_')) != 0) {
	while (strlen(ap) >= aSiz) {
	    aBuf = (char *)realloc(aBuf, (aSiz + ABSIZE) * sizeof(char));
	    assert(aBuf);
	    memset(&aBuf[aSiz], '\0', ABSIZE * sizeof(char));
	    aSiz += ABSIZE;
	}
	cp += aBuf - ap;
	ap = strncpy(aBuf, ap, aSiz);
	do {
	    *cp++ = '/';		/* change all '_' to '/' */
	} while ((cp = strchr(cp, '_')) != 0);
    }
    if ((cp = strchr(bp = (*b)->gt_ids, '_')) != 0) {
	while (strlen(bp) >= bSiz) {
	    bBuf = (char *)realloc(bBuf, (bSiz + ABSIZE) * sizeof(char));
	    assert(bBuf);
	    memset(&bBuf[bSiz], '\0', ABSIZE * sizeof(char));
	    bSiz += ABSIZE;
	}
	cp += bBuf - bp;
	bp = strncpy(bBuf, bp, bSiz);
	do {
	    *cp++ = '/';		/* change all '_' to '/' */
	} while ((cp = strchr(cp, '_')) != 0);
    }
    return( strverscmp(ap, bp) );
} /* iC_cmp_gt_ids */

/********************************************************************
 *
 *	Initialize IO
 *
 *******************************************************************/

void
iC_initIO(void)
{
#ifdef	TCP
#ifdef	EFENCE
    regBuf = iC_emalloc(REQUEST);
    rpyBuf = iC_emalloc(REPLY);
#endif	/* EFENCE */
#endif	/* TCP */
    signal(SIGSEGV, iC_quit);			/* catch memory access signal */
} /* iC_initIO */

/********************************************************************
 *
 *	Quit program with 'q' or ctrlC or Break via signal SIGINT
 *	or program abort on detected bugs.
 *
 *******************************************************************/

#ifndef TCP
#define iC_iccNM	iC_progname		/* for icr */
#endif	/* TCP */

void
iC_quit(int sig)
{
#ifdef	RASPBERRYPI
    if (iC_opt_P) {
	int		un;
	int		pfce;

#ifdef	BCM2835
	/********************************************************************
	 *	Disable the pullup resistor on gpio25/INTB
	 *	Close the BCM2835 direct memory access
	 *******************************************************************/
	bcm2835_gpio_set_pud(25, BCM2835_GPIO_PUD_OFF);	/* Disable Pull Up */
	bcm2835_close();
#endif	/* BCM2835 */
	/********************************************************************
	 *	Close PiFaceCad
	 *	Shutdown all slected PiFace Units leaving interrupts off and open drain
	 *******************************************************************/
	if (iC_debug & 0200) fprintf(iC_outFP, "### Shutdown %d unit(s)\n", npf);
	for (un = 0; un < npf; un++) {
	    if (setupMCP23S17(pfi[un].spiFN, pfi[un].pfa, IOCON_ODR, 0x00, 0) < 0) {
		fprintf(iC_errFP, "ERROR: %s: PiFace %d not found after succesful test ???\n", iC_progname, pfi[un].pfa);
	    }
	}
	/********************************************************************
	 *	Close selected spidev devices
	 *******************************************************************/
	for (pfce = 0; pfce < 2; pfce++) {
	    if (spidFN[pfce] > 0) {
		close(spidFN[pfce]);		/* close connection to /dev/spidev0.0, /dev/spidev0.1 */
	    }
	}
	/********************************************************************
	 *	Close GPIO25/INTB/INTA value 
	 *******************************************************************/
	if (gpio25FN > 0) {
	    close(gpio25FN);			/* close connection to /sys/class/gpio/gpio25/value */
	}
	/********************************************************************
	 *	free up the sysfs for gpio 25 unless used by another program (SIGUSR2)
	 *******************************************************************/
	if (sig != SIGUSR2 && (un = doUnexport(25)) != 0) {
	    sig = un;				/* unable to unexport gpio 25 */
	}
    } /* iC_opt_P */
#endif	/* RASPBERRYPI */
    /********************************************************************
     *  The following termination function is an empty function
     *  in the libict.a support library.
     *		int iCend(void) { return 0; }
     *  It may be implemented in a literal block of an iC source, in
     *  which case that function will be linked in preference.
     *  User implementations of iCend() should return 1, to activate
     *  the debug message "iend complete ======".
     *
     *  It can be used to free allocated memory, etc.
     *
     *  If the iCbegin() function contains a fork() call, iCend() may have
     *  a matching wait() call.
     *******************************************************************/
    if ((sig >= QUIT_TERMINAL || sig == SIGINT)
	&& iCend()
    ) {						/* termination function */
#if	YYDEBUG
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\niCend complete ======\n");
	}
#endif	/* YYDEBUG */
    }
#ifdef TCP
    if (iC_sockFN > 0) {
#ifndef RASPBERRYPI
	if (C_channel) {
	    /* disconnect iClive - follow with '0' for iCserver */
	    snprintf(regBuf, REQUEST, "%hu:5,%hu:0", C_channel, C_channel);
	    iC_send_msg_to_server(iC_sockFN, regBuf);
#ifdef	WIN32
	    Sleep(200);				/* 200 ms in ms */
#else	/* WIN32 */
	    nanosleep(&ms200, NULL);
#endif	/* WIN32 */
	    if (iC_micro) iC_microPrint("Disconnected", 0);
	}
#endif /* ! RASPBERRYPI */
	if (iC_Xflag) {			/* stop iCserver if this process started it */
	    snprintf(regBuf, REQUEST, "X%s", iC_iccNM);
	    iC_send_msg_to_server(iC_sockFN, regBuf);
	}
	close(iC_sockFN);			/* close connection to iCserver */
    }
#endif /* TCP */
    /********************************************************************
     *	Normal quit
     *******************************************************************/
    fflush(iC_outFP);				/* in case dangling debug messages without CR */
    if (iC_outFP != stdout) {
	fclose(iC_outFP);
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
#if defined(TCP) && ! defined(RASPBERRYPI)
    if (iC_savFP) {
	fflush(iC_savFP);
	fclose(iC_savFP);
    }
    if (iC_vcdFP) {
	fflush(iC_vcdFP);
	fclose(iC_vcdFP);
    }
#endif /* defined(TCP) && ! defined(RASPBERRYPI) */
#ifdef	_MSDOS_
    if (oldhandler && iC_debug & 03000) {
	/* reset the old interrupt handler */
#ifdef	MSC
	_dos_setvect(INTR, oldhandler);
#else	/* MSC */
	setvect(INTR, oldhandler);
#endif	/* MSC */
    }
#else	/* ! _MSDOS_ Linux */
    if (ttyparmFlag) {
	if (tcsetattr(0, TCSAFLUSH, &ttyparms) == -1) exit(-1);
    }
#endif	/* ! _MSDOS_ Linux */
    if (iC_errFP != stderr) {
	fflush(iC_errFP);
	fclose(iC_errFP);
    }
#ifdef	TCP
#ifdef	EFENCE
    free(rpyBuf);
    free(regBuf);
#endif	/* EFENCE */
#ifndef	RASPBERRYPI
#ifdef	EFENCE
    free(msgBuf);
    free(iC_outBuf);
#endif	/* EFENCE */
#endif	/* !RASPBERRYPI */
#endif	/* TCP */
    exit(sig < QUIT_TERMINAL ? sig : 0);	/* really quit */
} /* iC_quit */
#endif /* defined(RUN) || defined (TCP) || defined(LOAD) */

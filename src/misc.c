static const char misc_c[] =
"@(#)$Id: misc.c 1.18 $";
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
#include	<err.h>
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
#ifndef	PWM
#ifdef	RASPBERRYPI
#include	"rpi_gpio.h"
#include	"mcp23s17.h"
#include	"pifacecad.h"
#include	"bcm2835.h"
#endif	/* RASPBERRYPI */
#endif	/* PWM */

/********************************************************************
 *
 *	iC_bitIndex[] is a 256 byte array, whose values represent the
 *	bit position 0 - 7 of the rightmost 1 bit of the array index
 *
 *******************************************************************/

unsigned char	iC_bitIndex[]   = {
 0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,	/* index 0 has no 1's - points to 0 */
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
};

/********************************************************************
 *
 *	iC_bitMask[] is an 8 byte array, whose values are the mask for
 *	bit position 0 - 7
 *
 *******************************************************************/

unsigned char	iC_bitMask[]    = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,	/* 0 1 2 3 4 5 6 7 */
};

#ifdef	TCP
#ifndef	EFENCE
char		regBuf[REQUEST];	/* build registration string */
char		rpyBuf[REPLY];		/* ack string - also use to collect -e equivalences */
#else	/* EFENCE */
char *		regBuf;
char *		rpyBuf;
#endif	/* EFENCE */

#ifndef	PWM
int		iC_opt_l;
#ifdef	RASPBERRYPI
int		iC_opt_P;
int		iC_opt_G;
int		iC_opt_B;
int		iC_opt_E;
int		iC_opt_L;
int		iC_npf;
int		gpio23FN = -1;		/* /sys/class/gpio/gpio23/value LIRC file number */
int		gpio25FN = -1;		/* /sys/class/gpio/gpio25/value SPI file number */
int		spidFN[2] = { -1, -1 };	/* /dev/spidev0.0, /dev/spidev0.1 SPI file numbers */
piFaceIO	iC_pfL[MAXPF];		/* piFace names/gates and channels */
gpioIO *	iC_gpL[2];		/* GPIO names/gates and channels */
#endif	/* RASPBERRYPI */
#if	defined(LOAD) && ! defined(WIN32)
#include	<time.h>
static struct timespec	ms200 = { 0, 200000000, };
#endif	/* defined(LOAD) && ! defined(WIN32) */

/********************************************************************
 *
 *	Special code to support PiFace Control and Display.
 *	Display a string either directly on a PiFaceCAD
 *	or send it via TCP/IP on PFCAD4 to be displayed
 *	  displayString	formatted for display on a PiFaceCAD
 *	  channel	0 = direct display or > 0 && < 0xfff0 iCserver channel
 *
 *******************************************************************/

void
writePiFaceCAD(const char * displayString, unsigned short channel)
{
    char *	cp;
    char	buf[101];

    if (channel == 0) {
#ifdef	RASPBERRYPI
	pifacecad_lcd_clear();
	pifacecad_lcd_write(displayString);	/* write direct to PiFaceCAD */
#else	/* RASPBERRYPI */
	fprintf(stderr, "ERROR: cannot write directly to PiFaceCAD\n");
	iC_quit(SIGUSR1);
#endif	/* RASPBERRYPI */
    } else
    if (channel < 0xfff0) {
	cp = buf;
	if (snprintf(buf, 100, "%hu:%s", channel, displayString) > 100) {
	    buf[100] = '\0';			/* terminate in case of overflow (unlikely) */
	}
#if	YYDEBUG
	if (iC_debug & 0200) {
	    fprintf(iC_outFP, "writePiFaceCAD: '%s'\n", buf);	/* terminate with a CR in case last line has none */
	}
#endif	/* YYDEBUG */
	while ((cp = strchr(cp+1 , ',')) != NULL) {	/* no commas in channel: */
	    *cp = '\036';			/* replace every comma by ASCII RS */
	}
	assert(iC_sockFN > 0);
	iC_send_msg_to_server(iC_sockFN, buf);
    } else {
	fprintf(stderr, "ERROR: unable to register as PFCAD4 sender - is PiFaceCAD input correct ?\n");
	iC_quit(SIGUSR1);
    }
} /* writePiFaceCAD */
#ifdef	RASPBERRYPI

/********************************************************************
 *
 *  Execute the SUID root progran iCgpioPUD(gpio, pud) to set pull-up/down
 *      gpio	GPIO number in the range 0 - 27 (higher GPIO's are in the next block)
 *      pud	0	BCM2835_GPIO_PUD_OFF
 *      pud	1	BCM2835_GPIO_PUD_DOWN
 *      pud	2	BCM2835_GPIO_PUD_UP
 *
 *******************************************************************/

void
iC_gpio_pud(int gpio, int pud)
{
    int		r;
    char	buf[100];

    snprintf(buf, 100, "iCgpioPUD -g %d -p %d", gpio, pud);	/* execute iCgpioPUD as a separate process */
#if	YYDEBUG
	if (iC_debug & 0200) { fprintf(iC_outFP, "*** %s: '%s'\n", iC_progname, buf); fflush(iC_outFP); }
#endif	/* YYDEBUG */
    if ((r = system(buf)) != 0) {
	perror("iCgpioPUD");
	fprintf(iC_errFP, "WARNING: %s: system(\"%s\") could not be executed $? = %d - ignore\n",
	    iC_progname, buf, r);
    }
} /* iC_gpio_pud */
#endif	/* RASPBERRYPI */
#endif	/* PWM */
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
 * sets allocated memory to '\0'
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
	err(1, "%s", iC_progname);	/* hard ERROR */
    }
    bp = GlobalLock(hglobal);		/* actual pointer */
#else
    LOCALHANDLE		hlocal;

    if ((hlocal = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, nbytes)) == 0) {
	err(1, "%s", iC_progname);	/* hard ERROR */
    }
    bp = LocalLock(hlocal);		/* actual pointer */
#endif
#else
    if ((bp = malloc(nbytes)) == NULL) {
	err(1, "%s", iC_progname);	/* hard ERROR */
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
 *	Convert string of call + arguments to an argv call array
 *	    callString	single space separated call + argument string
 *	    argc	the number of call + arguments in callString
 *
 *	There must be at least one token string (the call) in callString
 *	and argc must be the exact number of space separated strings in argv
 *	otherwise a hard error will occurr (assert(0))
 *
 *	This routine can be called in iCbegin() to support iC_fork_and_exec()
 *
 *******************************************************************/

char **
iC_string2argv(char * callString, int argc)
{
    int	i;
    char *	copy;			/* copy of call string to break into tokens */
    char **	argv;			/* array of individual pointers to return */

    copy = (char *)iC_emalloc(strlen(callString)+1);	/* +1 for '\0' */
    argv = (char **)iC_emalloc((argc + 1) * sizeof(char*));
    strcpy(copy, callString);
    /* retrieve first token from copy, separated using " " */
    if ((argv[i = 0] = strtok(copy, " ")) == NULL) assert(0);
    /* continue to retrieve tokens until NULL is returned */
    while (i <= argc && (argv[++i] = strtok(NULL, " ")) != NULL);
    assert(i <= argc);			/*  programmer error */
    return argv;
} /* iC_string2argv */

/********************************************************************
 *
 *	Fork and execute a new process
 *	    argv	null terminated array of call + arguments
 *
 *	This routine can be called in iCbegin() to run a parallel process
 *
 *	Note: to allow splicing in of "-s" "host" and/or "-p" port" after
 *	      the initial program to be executed, that program must
 *	      be a direct call: eg "iCbox" and not "perl" "-S" "iCbox".
 *	      The execvp() used can handle this type of call.
 *
 *******************************************************************/

void
iC_fork_and_exec(char ** argv)
{
    pid_t	pid;
    uid_t	uid;
    uid_t	euid;
    char *	cp;
    char **	cpp = argv;
#ifdef	TCP
    char **	epp;
    int		extraArgs = 0;
    char *	hostp = NULL;
    char *	portp = NULL;
    if (strcmp(iC_hostNM, LOCALHOST) &&
	strcmp(iC_hostNM, LOCALHOST1)) {
	extraArgs += 2;
	hostp = (char *)iC_hostNM;
    }
    if (strcmp(iC_portNM, iC_PORT)) {
	extraArgs += 2;
	portp = (char *)iC_portNM;
    }
    if (extraArgs) {
	assert(*cpp);			/* must have at least one argument - the program to execute */
	while (*cpp++) {
	    extraArgs++;		/* add original number of arguments */
	}
	epp = cpp = (char **)iC_emalloc((extraArgs + 1) * sizeof(char*));
	extraArgs--;			/* count arguments for final assertion check */
	*epp++ = *argv++;		/* copy pointer to program first */
	if (hostp) {
	    extraArgs -= 2;
	    *epp++ = "-s";
	    *epp++ = hostp;		/* followed by optional -s hostp */
	}
	if (portp) {
	    extraArgs -= 2;
	    *epp++ = "-p";
	    *epp++ = portp;		/* followed by optional -s portp */
	}
	while ((cp = *argv++) != NULL) {
	    extraArgs--;
	    *epp++ = cp;		/* copy rest of original argument pointers */
	}
	assert(extraArgs == 0);		/* final assertion check */
	*epp = NULL;			/* argv terminator in extra element allowed for in malloc */
	argv = cpp;			/* new extended arguments */
    }
#endif	/* TCP */
    uid = getuid();
    euid = geteuid();
    if (iC_debug & 04) fprintf(iC_errFP, "uid = %d euid = %d\n", (int)uid, (int)euid);
    /********************************************************************
     *	Display the call + arguments to keep track of what is happening
     *  unless -q quiet operation
     *      $ call arg1 arg2 ... argn
     *******************************************************************/
    if ((iC_debug & DQ) == 0) {
	fprintf(iC_errFP, "$");
	while ((cp = *cpp++) != NULL) {
	    fprintf(iC_errFP, " %s", cp);
	}
	fprintf(iC_errFP, " &\n");
    }
    /********************************************************************
     *	Fork now
     *******************************************************************/
    if ((pid = fork()) == 0) {
	/* child process */
	if (euid != uid) {
	    if (seteuid(uid) != 0) {	/* execute child process at uid privileges */
		perror("seteuid failed");	/* hard ERROR */
		exit(SIGUSR1);
	    }
	}
	execvp(*argv, argv);		/* execute call in parallel child process */
	/* only get here if exec fails */
	fprintf(iC_errFP, "Could not execute %s - ", *argv); perror("");	/* hard ERROR */
	exit(SIGUSR1);			/* do not use iC_quit to avoid termQuit() for parent stuff */
    } else if (pid < 0) {
	perror("fork failed");		/* hard ERROR */
	iC_quit(SIGUSR1);		/* still parent */
    }
    /* continue parent process with extended privileges */
} /* iC_fork_and_exec */

/********************************************************************
 *
 *  Compare gt_ids in two Gates support of qsort()
 *
 *  change the collating position of '_' before digits ('.' '/')
 *  change all '_' to '/' to get correct ordering (/ never occurs in C variable)
 *  use strverscmp(), which puts sequences of digits in number order.
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
 *  Quit program with 'q' or ctrlC or Break via signal SIGINT
 *  or program abort on detected bugs.
 *
 *******************************************************************/

#ifndef TCP
#define iC_iccNM	iC_progname		/* for icr */
#endif	/* TCP */

void
iC_quit(int sig)
{
#ifdef	RASPBERRYPI
    sig = (*iC_term)(sig);			/* clear and unexport RASPBERRYPI stuff */
#endif	/* RASPBERRYPI */
    /********************************************************************
     *  The following termination function is an empty function
     *  in the libict.a support library.
     *  	int iCend(void) { return -1; }
     *  It may be implemented in a literal block of an iC source, in
     *  which case that function will be linked in preference.
     *  User implementations of iCend() should return 0, to activate
     *  the debug message "== iCend complete ======".
     *
     *  It can be used to free allocated memory, etc.
     *
     *  If the iCbegin() function contains a fork() call, iCend() may have
     *  a matching wait() call.
     *******************************************************************/
    if ((sig >= QUIT_TERMINAL || sig == SIGINT)
	&& iCend() != -1			/* iC termination function */
    ) {
#if	YYDEBUG
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\n== iCend complete ======\n");
	}
#endif	/* YYDEBUG */
    }
#ifdef TCP
    if (iC_sockFN > 0) {
	if (sig < SIGUSR1 || sig == QUIT_TERMINAL || sig == QUIT_DEBUGGER) {	/* but not QUIT_SERVER */
#ifdef LOAD
	    if (C_channel) {
		/* disconnect iClive - follow with '0' for iCserver */
		snprintf(regBuf, REQUEST, "%hu:5,%hu:0", C_channel, C_channel);
		iC_send_msg_to_server(iC_sockFN, regBuf);
#ifdef	WIN32
		Sleep(200);			/* 200 ms in ms */
#else	/* WIN32 */
		nanosleep(&ms200, NULL);
#endif	/* WIN32 */
		if (iC_micro) iC_microPrint("Disconnected", 0);
	    }
#endif /* LOAD */
	    if (iC_Xflag) {			/* stop iCserver if this process started it */
		snprintf(regBuf, REQUEST, "X%s", iC_iccNM);
		iC_send_msg_to_server(iC_sockFN, regBuf);
	    }
	    iC_send_msg_to_server(iC_sockFN, "");	/* 0 length message to disconnect from iCserver */
	}
	close(iC_sockFN);			/* close connection to iCserver */
    }
#endif /* TCP */
    /********************************************************************
     *  Normal quit
     *******************************************************************/
    fflush(iC_outFP);				/* in case dangling debug messages without CR */
    if (iC_outFP != stdout) {
	fclose(iC_outFP);
    }
    if ((iC_debug & DQ) == 0) {
	if (sig == QUIT_TERMINAL) {
	    fprintf(iC_errFP, "\n'%s' stopped from terminal\n", iC_iccNM);
	} else if (sig == QUIT_DEBUGGER) {
	    fprintf(iC_errFP, "\n'%s' stopped by debugger\n", iC_iccNM);
	} else if (sig == QUIT_SERVER) {
	    fprintf(iC_errFP, "\n'%s' disconnected by server\n", iC_iccNM);
	}
    }
    if (sig == SIGINT) {
	fprintf(iC_errFP, "\n'%s' stopped by interrupt from terminal\n", iC_iccNM);
    } else if (sig == SIGSEGV) {
	fprintf(iC_errFP, "\n'%s' stopped by 'Invalid memory reference'\n", iC_iccNM);
    } else if (sig == SIGUSR1) {
	fprintf(iC_errFP, "\n'%s' stopped by 'non-recoverable run-time error'\n", iC_iccNM);
    }
#if defined(TCP) && defined(LOAD)
    if (iC_savFP) {
	fflush(iC_savFP);
	fclose(iC_savFP);
    }
    if (iC_vcdFP) {
	fflush(iC_vcdFP);
	fclose(iC_vcdFP);
    }
#endif /* defined(TCP) && defined(LOAD) */
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
#if defined(TCP) && defined(EFENCE)
    free(rpyBuf);
    free(regBuf);
#ifdef	LOAD
    free(msgBuf);
    free(iC_outBuf);
#endif	/* LOAD */
#endif /* defined(TCP) && defined(EFENCE) */
    exit(sig < QUIT_TERMINAL ? sig : 0);	/* really quit */
} /* iC_quit */
#endif /* defined(RUN) || defined (TCP) || defined(LOAD) */

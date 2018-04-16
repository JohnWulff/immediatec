static const char ict_c[] =
"@(#)$Id: ict.c 1.75 $";
/********************************************************************
 *
 *	Copyright (C) 1985-2017  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	ict.c
 *	runtime execution of iC network with TCP/IP I/O to iCserver
 *
 *	Standalone run-time version for TCP/IP communication.
 *	Alternatively to produce compiled applications
 *	linked with libict.a produced with LOAD option
 *
 *	branched from "icc.c,v 1.6 1999/08/06"
 *
 *******************************************************************/

/* J.E. Wulff	3-Mar-85 */

#ifndef TCP
#error - must be compiled with TCP defined to make a linkable library
#else	/* TCP */
#define		_GNU_SOURCE		/* for memrchr() in string.h */
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<errno.h>
#include	<sys/types.h>
#ifdef	WIN32
#include	<Time.h>
#else	/* WIN32 */
#include	<sys/time.h>
#endif	/* WIN32 */
#include	<signal.h>
#include	<ctype.h>
#include	<assert.h>
#include	"tcpc.h"
#include	"icc.h"
#ifdef	RASPBERRYPI
#include	"rpi_gpio.h"
#include	"mcp23s17.h"
#include	"pifacecad.h"
#endif	/* RASPBERRYPI */

#define IOCHANNELS	30		/* initial Channels[] size */
#define	OSIZ	20
#define	TSIZ	10

static unsigned short	liveFlag;
static unsigned short	stepFlag;
static unsigned short	stepMask;
static unsigned short	debugFlag;	/* set by iClive in 'd' message, cleared in DEBUG message */
static unsigned short	debugMask;	/* set by iClive in 's' 'n' message, cleared in 'c' message */
static unsigned short	debugStop;	/* cleared by iClive in CONT message */

char			iC_stdinBuf[REPLY];	/* store a line of STDIN - notified by TX0.1 */
static int		stdinFlag = 0;
static fd_set		infds;			/* initialised file descriptor set for normal iC_wait_for_next_event() */
static fd_set		ixfds;			/* initialised extra descriptor set for normal iC_wait_for_next_event() */
static fd_set		idfds;			/* initialised file descriptor set for debug iC_wait_for_next_event() */
struct timeval		iC_timeOut = { 0, 49850 }; /* 50 mS select timeout - 5 ms if TX0.3 programmed */
static struct timeval	toCnt  = { 0, 4850 };	/* fast timeout counter that select uses for 5 ms */
static int		t5msFlag = 0;		/* set if 5 ms timer TX0.3 is programmed */
#ifdef	RASPBERRYPI
static struct timeval *	toCoff = NULL;		/* select() timeout off when NULl pointer for PiFaceCad and debug timeout */
static piFaceIO *	pfCADpfp = NULL;	/* for PiFaceCAD string via PFCAD4 */
static unsigned short	pfCADchannel = 0;
#endif	/* RASPBERRYPI */
static Gate		pfCADgate = { 1, -iC_ARNC, iC_ARITH, 0, "PFCAD4", {0}, {0}, 0, S_WIDTH };	/* iC string */
static struct timeval *	toCntp = NULL;		/* select timeout switched off when NULl pointer */

static void	regAck(Gate ** oStart, Gate ** oEnd);
static void	sendOutput(void);
static void	debugWait(void);
static void	receiveActiveSymbols(char * cp1);
static void	receiveWatchOrRestore(char * cp1);
static void	storeChannel(unsigned short channel, Gate * gp
#ifdef	RASPBERRYPI
			    , piFaceIO * pfp, gpioIO * gep, int iq
#endif	/* RASPBERRYPI */
			    );
static char *
#if	INT_MAX == 32767 && defined (LONG16)
convert2binary(long value, int bitFlag);
#else	/* INT_MAX == 32767 && defined (LONG16) */
convert2binary(int value, int bitFlag);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
static char		binBuf[33];		/* allows 32 bits */
static iC_Functp *	iC_i_lists[] = { I_LISTS };

/********************************************************************
 *  initialise all lists with their name to aid symbolic debugging
 *  do this here because at least "flist" is needed in load() for
 *  ini output, which runs before iC_icc().
 *******************************************************************/
/* these lists are toggled (initialised dynamically) */
static Gate	alist0 = { 0, 0, 0, 0, "alist0", };
static Gate	alist1 = { 0, 0, 0, 0, "alist1", };
Gate *		iC_a_list;		/* arithmetic output action list */
static Gate	olist0 = { 0, 0, 0, 0, "olist0", };
static Gate	olist1 = { 0, 0, 0, 0, "olist1", };
Gate *		iC_o_list;		/* logic output action list */
/* these lists are not toggled (static initialisation here) */
Gate *		iC_c_list;		/* main clock list "iClock" */
static Gate	flist = { 0, 0, 0, 0, "flist", };
Gate *		iC_f_list = &flist;	/* deferred function action list (init in load) */
static Gate	slist = { 0, 0, 0, 0, "slist", };
Gate *		iC_s_list = &slist;	/* send bit and byte outputs */

short		iC_error_flag;

unsigned	iC_linked;		/* link Flag for iC_liveData() */
unsigned	iC_scan_cnt;		/* count scan operations */
unsigned	iC_link_cnt;		/* count link operations */
#if	YYDEBUG && (!defined(_WINDOWS) || defined(LOAD))
unsigned	iC_glit_cnt;		/* count glitches */
unsigned long	iC_glit_nxt;		/* count glitch scan */
#endif	/* YYDEBUG && (!defined(_WINDOWS) || defined(LOAD)) */

static Gate *	timNull[] = { 0, 0, 0, 0, 0, 0, 0, 0, }; /* speeds up tim[] lookup */
static Gate **	tim = timNull;		/* point to array of 8 null pointers */
static unsigned short	topChannel = 0;
static char	buffer[BS];
#ifdef	RASPBERRYPI

static int	fireInput(piFaceIO * pfp, gpioIO * gep, int val);
static void	writePiFace(piFaceIO * pfp, int val, int iq);
static int	tcnt_750 = 15;		/* 750 ms timer for PiFaceCAD shift */
static int	slr = 0;
static int	cbc = 0x03;		/* first input will switch to cursor/blink off */
static channelSel * Channels = NULL;	/* dynamic array to store input gate and PiFace pointers */
#else	/* ! RASPBERRYPI */
static Gate **	Channels = NULL;	/* dynamic array to store input gate addresses */
#endif	/* ! RASPBERRYPI */

static int	ioChannels = 0;		/* allocated size of Channels[] */
unsigned short	C_channel;		/* channel for sending messages to Debug in ict.c */
static unsigned short D_channel;	/* channel for receiving messages from Debug in ict.c */
static Gate	D_gate;			/* Gate on channel for receiving messages from Debug */
static int	liveOffset;		/* for live header */
static int	regOffset;		/* for register send */
#if	YYDEBUG
static unsigned int waitCount;		/* for WAIT display */
#endif	/* YYDEBUG */
#ifndef	EFENCE
char		msgBuf[REQUEST];	/* Buffer in which live data is collected */
char		iC_outBuf[REQUEST];	/* Buffer in which output is collected in iC_output() */
#else	/* EFENCE */
char *		msgBuf;
char *		iC_outBuf;
#endif	/* EFENCE */
static char *	outPtr;
static int	outBufLen = REQUEST;
static int	msgOffset;		/* for message send */
extern const char	iC_ID[];
static char *	iC_sav;
static long	virtualTime = 0;
FILE *		iC_vcdFP = NULL;
FILE *		iC_savFP = NULL;
static char *	vcd_ftype[]	= { VCD_FTYPE };
static char *	sav_ftype[]	= { SAV_FTYPE };

/********************************************************************
 *
 *	Procedure iC_icc
 *
 *******************************************************************/

void
iC_icc(void)				/* Gate ** sTable, Gate ** sTend are global */
{
    int			i;
    int			c;
    int			cnt;
    short		pass;
    short		typ;
    unsigned short	channel = 0;
    Gate **		opp;
    char *		cp;
    Gate *		gp;
    int			len;
    iC_Functp		init_fa;
    int			t50m  = 1;
    int			t500m = 1;
    int			t5s   = 1;
    int			t30s  = 1;
    int			retval;
    int			mask;
#if	INT_MAX == 32767 && defined (LONG16)
    long		val;
#else	/* INT_MAX == 32767 && defined (LONG16) */
    int			val;
#endif	/* INT_MAX == 32767 && defined (LONG16) */
    unsigned short	index;
    unsigned short	iClock_index;
    unsigned short	debugBlock;
    FILE *		vcdFlag;
#ifdef	RASPBERRYPI
    piFaceIO *		pfp;
    iqDetails *		pfq;
    int			iq;
    unsigned short	marking;
    const char		RSin[2][4]  = { "RxxS", "RSxS" };		// [opt_E][marking] */
    const char		RSout[2][4] = { "SxxR", "SSxR" };		// [opt_E][marking] */
    char *		op;
    int			ol;
    int			m;
    int			m1;
    unsigned short	bit;
    unsigned short	gpio;
    gpioIO *		gep;
    int			b;
    int			pqSel;
#endif	/* RASPBERRYPI */

#ifdef	EFENCE
    msgBuf = iC_emalloc(REQUEST);
    iC_outBuf = iC_emalloc(REQUEST);
#endif	/* EFENCE */
    signal(SIGSEGV, iC_quit);		/* catch memory access signal */

    if (iC_outFP && iC_outFP != stdout) {
	fclose(iC_outFP);
#ifndef LOAD
	if (iFlag) {
	    iC_inversionCorrection();	/* only applies to compiler listing */
	    iFlag = 0;
	}
#endif	/* LOAD */
    }
    iC_outFP = stdout;			/* standard output from here */
    if (iC_errFP && iC_errFP != stderr) {
	fclose(iC_errFP);
    }
    iC_errFP = stderr;			/* standard error from here */
    if (iC_micro) iC_microReset(0);	/* start timing */

    if ((gp = iC_TX0p) != 0) {		/* are EOI etc or TX0 timers programmed */
	tim = gp->gt_list;		/* TX0.0 - TX0.7, TX0.0 is EOI - end of initialisation */
	assert(tim);			/* TX0.1 is used to report receiving a new line on STDIN */
					/* TX0.2 is used as a fixed bit LO, ~TX0.2 as fixed bit HI */
	for (i = 3; i < 8; i++) {	/* TX0.2 is used as a fixed bit LO, ~TX0.2 as fixed bit HI */
	    if (tim[i] != NULL) {	/* any of the 5 timers 5ms ms - 30 seconds programmed ? */
		if (i == 3) {
		    t5msFlag = 1;	/* 5 ms timer is programmed */
		    iC_timeOut = toCnt;	/* { 0, 5000 } initially */
		}
#ifdef	RASPBERRYPI
		toCoff =		/* permanent select timeout for PiFaceCAD shift and debug timeout */
#endif	/* RASPBERRYPI */
		toCntp = &toCnt;	/* activate select timeout */
		break;			/* could optimise by varying timeout value */
	    }
	}
    }
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 04) fprintf(iC_outFP, "t5msFlag  %d, iC_timeout  %d:%d\n",
	t5msFlag, (int)iC_timeOut.tv_sec, (int)iC_timeOut.tv_usec);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    /********************************************************************
     *  Clear and then set all bits to wait for interrupts
     *******************************************************************/
    FD_ZERO(&infds);			/* should be done centrally if more than 1 connect */
    FD_ZERO(&idfds);
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 04) fprintf(iC_outFP, "*** all normal interrupts have been cleared\n");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    FD_ZERO(&ixfds);
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 04) fprintf(iC_outFP, "*** all extra interrupts have been cleared\n");
#endif	/* YYDEBUG && !defined(_WINDOWS) */

    if ((iC_debug & 0400) == 0 && iC_argh <= 0) {
	char *		tbp;		/* points to next entry in regBuf */
	int		tbc;		/* length of filled entries */
	char *		tbt;		/* temp pointer */
	Gate **		sopp;
	char		entry[ENTRYSZ];	/* entry "SIX0000-000\0" max length 12 */
	char *		ep;		/* points to next point in entry */
	int		el;		/* entry length */
	char *		qp;		/* points to next entry in msgBuf for equivalence inputs */
	int		qc;		/* length of filled entries in msgBuf */
	int		ql;		/* length of individual entry in msgBuf */
	int		qFlag;
	const char *	sr[] = { "N", ",SC", ",RD", };	/* name, controller, debugger */
#ifdef	RASPBERRYPI
	int		en;		/* number of registration items */
	if (iC_opt_B) {
	    /********************************************************************
	     *  Start an iCbox -d for display of Internal in and ExtOut in/out
	     *  variables before registering any of the I/O variables so that
	     *  iCserver -a does not catch them.
	     *  Internal out variables will be displayed on an autovivified iCbox
	     *  if -B. If these were to be displayed on the extra iCbox -d, that
	     *  receiver would start another autovivified sender iCbox, which would
	     *  clash with the sender in this application and cause failure.
	     *******************************************************************/
	    len = snprintf(buffer, BS, "iCbox -dz%s -n %s-DI", (iC_debug & DQ) ? "q" : "", iC_iccNM);
	    b = 4;				/* initial number of tokens in buffer */
	    op = buffer + len;			/* optional host and port appended in iC_fork_and_exec() */
	    ol = BS - len;
	    /********************************************************************
	     *  PiFace arguments
	     *******************************************************************/
	    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
		for (iq = 0; iq < 3; iq++) {
		    pfq = &(pfp->s[iq]);
		    if ((gp = pfq->i.gate) != NULL &&
			(((marking = gp->gt_live) == Internal && gp->gt_fni == TRAB) ||
			marking == ExtOut)) {
			assert(ol > 10);
			len = snprintf(op, ol, " %s", gp->gt_ids);	/* add I/O name */
			b++;						/* count tokens in buffer */
			op += len;
			ol -= len;
			if (marking == Internal && *iC_iidNM) {
			    len = snprintf(op, ol, "-%s", iC_iidNM);	/* append instance ID */
			    op += len;
			    ol -= len;
			}
			if ((mask = pfq->bmask) != 0xff) {
			    len = snprintf(op, ol, ",%d", mask);	/* mask out any PiFace bits not used */
			    op += len;
			    ol -= len;
			}
		    }
		}
	    }
	    /********************************************************************
	     *  GPIO arguments
	     *******************************************************************/
	    for (iq = 0; iq < 2; iq++) {
		for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
		    if ((gp = gep->s.i.gate) != NULL &&
			(((marking = gp->gt_live) == Internal && gp->gt_fni == TRAB) ||
			marking == ExtOut)) {
			assert(ol > 14);
			len = snprintf(op, ol, " %s", gp->gt_ids);	/* add I/O name */
			b++;						/* count tokens in buffer */
			op += len;
			ol -= len;
			if (marking == Internal && *iC_iidNM) {
			    len = snprintf(op, ol, "-%s", iC_iidNM);	/* append instance ID */
			    op += len;
			    ol -= len;
			}
			if ((mask = gep->Gbmask) != 0xff) {
			    len = snprintf(op, ol, ",%d", mask);	/* mask out any GPIO bits not used */
			    op += len;
			    ol -= len;
			}
		    }
		}
	    }
	    if (b > 4) {	/* any items for iCbox -d to show (without items IX0 QX0 would be shown which causes error) */
		iC_fork_and_exec(iC_string2argv(buffer, b));	/* fork iCbox -d */
	    }
	}
	en = 0;			/* no registration items yet */
#endif	/* RASPBERRYPI */
	/********************************************************************
	 *  Do registration with iCserver, by sending a registration string in regAck()
	 *  consisting of N<progname>,SC<progname>,RD<progname>[,RIXx...][,SQXx...],Z
	 *  for every input and output gate in the application (see iCserver spec).
	 *  iCserver acknowledges the registration string by returning a comma
	 *  separated list of channel numbers - one for each gate registered.
	 *  These real channel numbers are used to transmit I/O data from senders
	 *  to receivers via iCserver.
	 *******************************************************************/
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 04) fprintf(iC_outFP, "%s: I/O registration objects\n", iC_iccNM);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	tbp = regBuf;
	tbc = REQUEST;
	el = 0;
	qp = msgBuf;		/* prepare to collect comma separated list of inputs */
	qc = REQUEST;		/* for doing all inputs as equivalences with '-e I' command */
	ql = qFlag = 0;
	for (i = 0; i < 3; i++) {
	    tbp += el;
	    tbc -= el;
	    el = snprintf(tbp, tbc, "%s%s", sr[i], iC_iccNM);	/* directly into regBuf */
	}
	/* use last string to initialise name "D<progname>[-<instance>]" for D_gate */
	D_gate.gt_ids = iC_emalloc(el-1);			/* including '\0' */
	strncpy(D_gate.gt_ids, tbp+2, el-1);		/* +2 leave out ",R" */
	D_gate.gt_new = -2;	/* changes - not equal 0 - 5 for debug messages */
	D_gate.gt_old = -1;	/* never changes - not equal 0 - 5 for debug messages */
#ifdef	RASPBERRYPI
	if (iC_opt_P && !iC_opt_G) {
	    /********************************************************************
	     *  Initialise active PiFaceCad
	     *******************************************************************/
	    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {	/* scan for PiFaceCAD */
		if (pfp->pfa == 4 && pfp->intf == INTFA && (gp = pfp->Igate)) {
		    if (iC_debug & 0200) fprintf(iC_outFP, "== Initialise active PiFaceCAD\n");
		    pifacecad_lcd_clear();		/* active PiFaceCAD */
		    pifacecad_lcd_display_on();		/* default is on */
		    pifacecad_lcd_set_backlight(1);	/* default is on */
		    /********************************************************************
		     *  At this stage marking is still stored in gp->gt_live
		     *  Register PFCAD4 as receiver if PiFaceCAD is called for ExtOut input
		     *******************************************************************/
		    if (gp->gt_live == ExtOut) {	/* current marking */
			tbp += el;
			tbc -= el;
			el = snprintf(tbp, tbc, ",R%s", pfCADgate.gt_ids);	/* receiver on PFCAD4 */
			pfCADpfp = pfp;			/* use in regAck() */
			P_channel = 0xfffd;		/* in case direct combined with ExtOut */
		    } else if (P_channel == 0xfffe) {
			P_channel = 0;			/* direct with writePiFaceCAD() */
		    }
		}
	    }
	}
#endif	/* RASPBERRYPI */
	/********************************************************************
	 *  Check if app wants to send strings to PiFaceCAD (can be non RASPBERRTPI)
	 *  Register PFCAD4 as sender unless PiFaceCAD is called for Intern input
	 *	P_channel	0xffff	this app is not interested in PiFaceCAD (default)
	 *	P_channel	0xfffe	this app wants to write to PiFaceCAD
	 *	P_channel	0xfffd	this app wants to write to PiFaceCAD - error ExtOut
	 *	P_channel	0	this app has an active PiFaceCAD not ExtOut input
	 *******************************************************************/
	if (P_channel == 0xfffe) {
	    tbp += el;
	    tbc -= el;
	    el = snprintf(tbp, tbc, ",S%s", pfCADgate.gt_ids);	/* receiver on PFCAD4 */
	}
	/********************************************************************
	 *  Registration of normal immediate variables in the app
	 *******************************************************************/
	for (opp = sopp = sTable; opp < sTend; opp++) {
	    gp = *opp;
	    tbp += el;
	    tbc -= el;
	    ep = entry;
	    el = 0;
	    mask = gp->gt_mark;
#ifdef	RASPBERRYPI

	    /********************************************************************
	     *
	     *  Marking values in gt_live to influence registration of I/Os:
	     *
	     *	0   External	// default  IXx <=RI= iCserver
	     *			//		      iCserver <=SQ= QXx
	     *
	     *	1   Internal	//          IXx <==== SIO/GPIO
	     *			//		      SIO/GPIO <==== QXx
	     *			// if opt_E    also   iCserver <=SI= IXx (display only)
	     *			//             and    iCserver <=SQ= QXx
	     *
	     *	2   Dummy	// ignore -    output WARNING  *****
	     *
	     *	3   ExtOut	// like iCpiFace      iCserver <=SI= SIO/GPIO (IXx)
	     *			//		      SIO/GPIO <=RQ= iCserver (QXx)
	     *
	     *  Signal flows for an arrangement of 2 apps, app1 and app2
	     *  serviced by 2 PiFaces, PiFace0 and PiFace1 called as follows:
	     *
	     *    app1 -P IX0 QX0 IX2 QX2 IX1 QX1 IX3 QX3;
	     *       uses IX0 QX0 IX1 QX1
	     *    app2;
	     *       uses IX2 QX2
	     *            IX3 QX3 not used by anybody
	     *
	     *    PiFace0 provides IX0 QX0 Internal  1 to app1 by SIO
	     *       also channels  3   4  if opt_E
	     *    PiFace1 provides IX2 QX2 ExtOut    3 by SIO, TCP/IP
	     *         on channels  7   8
	     *                     IX1 QX1 External  0 default TCP/IP
	     *         on channels  5   6
	     *                     IX3 QX3 Dummy     2 go nowhere
	     *
	     * **********************************                 +---SI-<- IX1 * External
	     * *         *  PiFace0  *  PiFace1 *                 | +-RQ->- QX1 * External
	     * *         *  IX0 QX0  *  IX2 QX2 *                 | |
	     * *         ****|***|*******|***|***   ************* | | ***********
	     * * app1 -P     v   ^  -E   v   ^  *   *  iCserver * v ^ * iCbox -d*
	     * *             |   |       |   |  *   *           * | | *         *
	     * * IX0 -<----1-+---|-1E-->-|---|-SI---3->--IX0-->-3-|-|-RI->- IX0d* Internal with opt_E
	     * * QX0 ->--------1-+--1E->-|---|-SQ---4->--QX0-->-4-|-|-RQ->- QX0 * Internal with opt_E
	     * * IX1 -<--------------0---|---|-RI---5-<--IX1--<-5-+-|-RI-<- IX1d* External display
	     * * QX1 ->---------------0--|---|-SQ---6->--QX1-->-6---+-RQ->- QX1 * External display
	     * *                         +-3-|-SI---7->--IX2-->-7-+---RI->- IX2d* ExtOut display
	     * *                            3+-RQ---8-<--QX2--<-8-|-+-RQ->- QX2 * ExtOut display
	     * *      (IX3) -----<---2---       *   *           * | | *         * Dummy
	     * *      (QX3) ----->----2--       *   *           * | | *         * Dummy
	     * **********************************   ************* | | ***********
	     *              \              /        \           / v ^
	     *               \  markings  /          \ channels/  | | ***********
	     *                                                    | | *  app2   *
	     *                                                    | | *         *
	     *                                                    +-|-RI->- IX2 * ExtOut
	     *                                                      +-SQ-<- QX2 * ExtOut
	     *                                                        *         *
	     *                                                        ***********
	     *
	     * **********************************     Same without opt_E and without opt_B (display)
	     * *         *  PiFace0  *  PiFace1 *     and no External
	     * *         *  IX0 QX0  *  IX2 QX2 *
	     * *         ****|***|*******|***|***   *************
	     * * app1 -P     v   ^       v   ^  *   *  iCserver *
	     * *             |   |       |   |  *   *           *     ***********
	     * * IX0 -<----1-+   |       |   |  *   *           *     *  app2   * Internal
	     * * QX0 ->--------1-+       |      *   *           R     *         * Internal
	     * *                         +-3-|-SI---7->--IX2-->-7-----RI->- IX2 * ExtOut
	     * *                            3+-RQ---8-<--QX2--<-8-----SQ-<- QX2 * ExtOut
	     * *                                *   *           *     *         *
	     * **********************************   *************     ***********
	     *              \              /        \           /
	     *               \  markings  /          \ channels/
	     *
	     *
	     *  Registration
	     *
	     *  After registration gt_live is used for other purposes, so the
	     *  gt_live markings must be transferred.
	     *
	     *  Use the channelSel Channels[] with .g for Gate * .p for piFaceIO *
	     *	For			IXx (-INTW)	QXx (OUTW)	flag interpretation
	     *				.g	.p	.g	.p	.g	.p
	     *		0 External	gp	NULL	gp f	NULL	1	0
	     *		1 Internal	gp	pfp E	gp	pfp E	1	1
	     *		2 Dummy		 -       -       -       -	0	0
	     *		3 ExtOut	NULL	pfp	NULL	pfp	0	1
	     *
	     *  Input interrupt and output handling
	     *
	     *  TCP/IP interrupt provides "channel:val" for every changed byte
	     *                  markings        External Internal Dummy    ExtOut
	     *                                     0        1        2        3
	     *    gp  = Channels[channel].g     // gp       (gp)     -        NULL
	     *    pfp = Channels[channel].p     // NULL     (pfp)    -        pfp
	     *    if (!gp && !pfp) Warning();	//	never called for Internal
	     *    if (gp) traMb(gp, 0);         // RI External
	     *    if (pfp)) writePF(pfp ...);   // RQ ExtOut
	     *
	     *  SIO interrupt provides pfp1 and val
	     *    channel = pfp1->Ichannel;
	     *    gp  = Channels[channel].g     // (gp)     gp       -        NULL
	     *    pfp = Channels[channel].p     // (NULL)   pfp E    -        pfp
	     *    assert(!gp || pfp);		//	never called for External
	     *    if (gp) traMb(gp, 0);         // I  Internal
	     *    if (pfp && (!gp || opt_E)) {
	     *        assert(pfp1 == pfp);
	     *        send_msg(channel ...);    // SI Internal & opt_E or ExtOut
	     *    }
	     *
	     *  GPIO interrupt provides pfp1 and val	TODO
	     *    channel = pfp1->Ichannel;
	     *    gp  = Channels[channel].g     // (gp)     gp       -        NULL
	     *    pfp = Channels[channel].p     // (NULL)   pfp E    -        pfp
	     *    assert(!gp || pfp);		//	never called for External
	     *    if (gp) traMb(gp, 0);         // I  Internal
	     *    if (pfp && (!gp || opt_E)) {
	     *        assert(pfp1 == pfp);
	     *        send_msg(channel ...);    // SI Internal & opt_E or ExtOut
	     *    }
	     *
	     *  iC_output(val, channel)
	     *    gp  = Channels[channel].g     // gp       (gp)     -        (NULL)
	     *    pfp = Channels[channel].p     // NULL     pfp E    -        (pfp)
	     *    if (pfp) writePF(pfp ...);    // Q  Internal
	     *    assert(gp);			//	never called for ExtOut
	     *    if (gp && (!pfp || iC_opt_E)) {
	     *        send_msg(channel ...);    // SQ External or Internal & opt_E
	     *    }
	     *
	     *******************************************************************/

	  if ((marking = gp->gt_live) != Dummy && *gp->gt_ids != 'T') {	/* TX0 and Dummy arguments are not registered */
	    assert(marking <= ExtOut);
	    if (gp->gt_ini == -INPW)		/* IXx IBx ... */
#else	/* ! RASPBERRYPI */
	    if (gp->gt_ini == -INPW && *gp->gt_ids != 'T')	/* IXx IBx ... */
#endif	/* ! RASPBERRYPI */
	    {
#ifdef	RASPBERRYPI
		if ((c = RSin[iC_opt_E][marking]) != 'x') {
		    el = snprintf(ep, ENTRYSZ, ",%c%s", c, gp->gt_ids);	/* register R S x S input/ExtOut send */
		    if (c == 'R') {
			ql = snprintf(qp, qc, ",%s", gp->gt_ids);	/* input directly into msgBuf */
		    }
		} else {
		    el = ql = 0;		/* input Internal !opt_E - do not register this input */
		    gp->gt_mark |= X_FLAG;	/* flag non-registration in regAck() - allocate virtual channel */
		}
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 04) fprintf(iC_outFP,
		    "%s	iq=1 opt_E=%d marking= %hu	gt_mark=0x%04x	\"%s\"	RSin[%d] = '%s'\n",
		    gp->gt_ids, iC_opt_E, marking, gp->gt_mark, el ? ep : "", iC_opt_E, RSin[iC_opt_E]);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#else	/* ! RASPBERRYPI */
	    	el = snprintf(ep, ENTRYSZ, ",R%s", gp->gt_ids);	/* register R input */
		ql = snprintf(qp, qc, ",%s", gp->gt_ids);	/* input directly into msgBuf */
#endif	/* ! RASPBERRYPI */
		qc -= ql;
		if (qc > 0) {
		    qp += ql;			/* ready for next entry */
		} else {
		    qc += ql;			/* msgBuf overflowed - ignore for equivalences */
		    *qp = '\0';			/* delete latest entry - this works in the test case */
		    if (qFlag == 0) {
			fprintf(iC_errFP, "WARNING: %s: too many -e equivalences - ignore '%s' and later\n",
			    iC_progname, gp->gt_ids);
			qFlag = 1;		/* unlikely to overflow, but code has been tested */
		    }				/* with small msgBuf[13] trying to fit 14 bytes */
		}
		if (gp->gt_fni != TRAB) {
		    mask = 0x100;		/* block output of used bits for analog I/O */
		}
	    } else
	    if (gp->gt_fni == OUTW) {		/* QXx */
#ifdef	RASPBERRYPI
		if ((c = RSout[iC_opt_E][marking]) != 'x') {
		    el = snprintf(ep, ENTRYSZ, ",%c%s", c, gp->gt_ids);	/* register S S x R output/ExtOut read */
		} else {
		    el = 0;			/* output Internal !opt_E - do not register this output */
		    gp->gt_mark |= X_FLAG;	/* flag non-registration in regAck() - allocate virtual channel */
		}
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 04) fprintf(iC_outFP,
		    "%s	iq=0 opt_E=%d marking= %hu	gt_mark=0x%04x	\"%s\"	RSout[%d] = '%s'\n",
			gp->gt_ids, iC_opt_E, marking, gp->gt_mark, el ? ep : "", iC_opt_E, RSout[iC_opt_E]);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#else	/* ! RASPBERRYPI */
		el = snprintf(ep, ENTRYSZ, ",S%s", gp->gt_ids);	/* register S output */
#endif	/* ! RASPBERRYPI */
		if (el > 0 && gp->gt_ini == -ARN) {
		    if ((tbt = strrchr(ep, '_')) != NULL &&
			*(tbt+1) == '0' &&
			*(tbt+2) == '\0') {
			*tbt = '\0';	/* strip trailing "_0" from internal output name */
			el -= 2;
			mask = 0x100;	/* block output of used bits for analog I/O */
			assert(el > 0);
		    } else {
			fprintf(iC_errFP, "\n%s: ill formed output name '%s'\n", iC_iccNM, ep + 1);
			iC_quit(SIGUSR1);
		    }
		}
	    }
	    if (el > 0) {
		if (*iC_iidNM) {
		    el += snprintf(ep + el, ENTRYSZ - el, "-%s", iC_iidNM);	/* append instance ID */
		}
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 04) fprintf(iC_outFP,
		    "%s%s%s	%d	%d	(%d)	{%hu}\n",
		    gp->gt_ids, (*iC_iidNM) ? "-" : "", iC_iidNM,
		    (int)gp->gt_ini, (int)gp->gt_fni, mask, gp->gt_live);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		if (mask && mask < X_MASK) {
		    el += snprintf(ep + el, ENTRYSZ - el, "(%d)", mask);	/* mask of used bits */
		}
		if (tbc - el <= (opp+1 == sTend ? 2 : 0)) {	/* leave room for ",Z" at end */
		    /********************************************************************
		     * buffer is about to overflow
		     *******************************************************************/
		    if (iC_micro & 06) iC_microPrint("partial registration", 0);
		    regAck(sopp, opp);		/* send/receive partial registration */
		    sopp = opp;
		    tbp = regBuf;		/* start new string */
		    tbc = REQUEST;		/* length of filled entries */
		    assert(*ep == ',' && (*(ep+1) == 'S' || *(ep+1) == 'R'));
		    ep++;			/* chop initial ',' */
		    el--;
		}
		strncpy(tbp, ep, tbc);		/* place entry in regBuf */
#ifdef	RASPBERRYPI
		en++;				/* count registration item */
#endif	/* RASPBERRYPI */
	    }
#ifdef	RASPBERRYPI
	  }
#endif	/* RASPBERRYPI */
	}
#ifdef	RASPBERRYPI
      if (en || iC_opt_L) {			/* Do TCP/IP I/O only if needed */
#endif	/* RASPBERRYPI */
	strncpy(tbp + el, ",Z", tbc - el);	/* place termination in regBuf - 2 bytes are free */
	if (iC_micro & 06) iC_microPrint("last registration", 0);
	assert(opp == sTend);
	regAck(sopp, opp);			/* send/receive last registration */
	snprintf(regBuf, REQUEST, "%hu:2;%s", C_channel, iC_iccNM);
	if (iC_micro & 06) iC_microPrint("Send application name", 0);
	iC_send_msg_to_server(iC_sockFN, regBuf);		/* Application Name */
	/********************************************************************
	 * Fork iClive
	 *******************************************************************/
	if (iC_opt_l) {
	    int	ii;
	    if ((ii = strlen(iC_iidNM)) == 0) {
		snprintf(buffer, BS, "iClive -z%s %s.ic", (iC_debug & DQ) ? "q" : "", iC_iccNM);
	    } else {
		ii = strlen(iC_iccNM) - 1 - ii;			/* remove -instance qualification */
		snprintf(buffer, BS, "iClive -z%si%s %*.*s.ic",
		    (iC_debug & DQ) ? "q" : "", iC_iidNM, ii, ii, iC_iccNM);
	    }
	    iC_fork_and_exec(iC_string2argv(buffer, 3));	/* fork iClive -zq app.ic */
	}
#ifdef	RASPBERRYPI
      }
	if (iC_opt_P) {
	    /********************************************************************
	     *  Allocate virtual channels for PiFace and GPIO Internal Gates, which
	     *  do not involve iCserver - they select iC_pfL[] or iC_gpL[] entries only.
	     *  Internal Gates with opt_E have already been allocated to a real
	     *  channel in regAck() as well as External and ExtOut Gates.
	     *******************************************************************/
	    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
		for (iq = 0; iq < 3; iq++) {
		    pfq = &(pfp->s[iq]);
		    if ((gp = pfq->i.gate) != NULL &&
			(gp->gt_mark & X_FLAG)) {		/* no X_FLAG set if !opt_L */
			assert(gp->gt_mark & P_FLAG);		/* P_FLAG || PB_FLAG */
			gp->gt_mark &= ~(X_FLAG | PG_MASK);
			channel = ++topChannel;			/* extra virtual channel */
			pfq->channel = channel;			/* receive or send input */
			if ((marking = gp->gt_live) == Internal) {	/* transfer markings */
			    storeChannel(channel, gp, pfp, NULL, iq);	/* Internal */
			} else if (marking == ExtOut) {
			    storeChannel(channel, NULL, pfp, NULL, iq);	/* ExtOut */
			} else {
			    assert(0);				/* never External, Dummy or other!!! */
			}
			if (gp->gt_ini != -INPW && gp->gt_fni == OUTW) {
			    gp->gt_channel = channel;		/* <== Output */
#if YYDEBUG && !defined(_WINDOWS)
			    if (iC_debug & 04) fprintf(iC_outFP, "store output: channel:  %s <== %hu\n",
				gp->gt_ids, channel);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			}

		    }
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 04) fprintf(iC_outFP, "iC_pfL[%d] %s:%d,%s:%d\n",
			pfp - iC_pfL,
			pfp->Igate ? pfp->Igate->gt_ids : NULL, pfp->Ichannel,
			pfp->Qgate ? pfp->Qgate->gt_ids : NULL, pfp->Qchannel);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		}
	    }
	    /********************************************************************
	     *  Allocate virtual channels for Internal GPIO Gates
	     *******************************************************************/
	    for (iq = 0; iq < 2; iq++) {
		for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
		    if ((gp = gep->Ggate) != NULL &&
			(gp->gt_mark & X_FLAG)) {		/* no X_FLAG set if !opt_L */
			assert((gp->gt_mark & PG_MASK) == G_FLAG);
			gp->gt_mark &= ~(X_FLAG | PG_MASK);
			channel = ++topChannel;			/* extra virtual channel */
			gep->Gchannel = channel;			/* receive or send input */
			if ((marking = gp->gt_live) == Internal) {	/* transfer markings */
			    storeChannel(channel, gp, NULL, gep, 0);	/* Internal */
			} else if (marking == ExtOut) {
			    storeChannel(channel, NULL, NULL, gep, 0);	/* ExtOut */
			} else {
			    assert(0);				/* never External, Dummy or other!!! */
			}
			if (gp->gt_ini != -INPW && gp->gt_fni == OUTW) {
			    gp->gt_channel = channel;		/* <== Output */
#if YYDEBUG && !defined(_WINDOWS)
			    if (iC_debug & 24) fprintf(iC_outFP, "store output: channel:  %s <== %hu\n",
				gp->gt_ids, channel);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			}
			if (iC_debug & 0100) {
			    fprintf(iC_outFP, "GPIO");
			    for (bit = 0; bit <= 7; bit++) {
				if ((gpio = gep->gpioNr[bit]) != 0xffff) {
				    fprintf(iC_outFP, ",%hu", gpio);
				} else {
				    fprintf(iC_outFP, ",d");
				}
			    }
			    fprintf(iC_outFP, "	%s  on channel %hu\n", gp->gt_ids, channel);
			}
		    }
		}
	    }
	    /********************************************************************
	     *  Report results of argument analysis
	     *  Port A outputs are inverted by hardware
	     *  Port B inputs and outputs are direct
	     *******************************************************************/
	    if (iC_debug & 0300) {
		char *	iidPtr;
		char *	iidSep;
		if (iC_npf) {
		    fprintf(iC_outFP, "Allocation for %d PiFace unit%s, global instance = \"%s\"\n",
			iC_npf, iC_npf == 1 ? "" : "s", iC_iidNM);
		    fprintf(iC_outFP, "	IEC-out	IEC-in	mask-i	IEC+out	mask+o	ch-out	ch-in	ch+out	pfa unit\n\n");
		    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
			if (pfp->Qgate != NULL) {
			    assert(pfp->Qbmask == 0xff);		/* all 8 bits always output */
			    if (*iC_iidNM == '\0' || strchr(pfp->Qgate->gt_ids, '-') != NULL) {
				iidSep = iidPtr = "";		/* null instance or local instance */
			    } else {
				iidSep = "-";
				iidPtr = iC_iidNM;			/* global instance */
			    }
			    fprintf(iC_outFP, "	%c%s%s%s",
				pfp->Qinv ? ' ' : '~', pfp->Qgate->gt_ids, iidSep, iidPtr);	/* double inversion */
			} else {
			    fprintf(iC_outFP, "	Dummy");
			}
			if (pfp->Igate != NULL) {
			    if (*iC_iidNM == '\0' || strchr(pfp->Igate->gt_ids, '-') != NULL) {
				iidSep = iidPtr = "";		/* null instance or local instance */
			    } else {
				iidSep = "-";
				iidPtr = iC_iidNM;			/* global instance */
			    }
			    if (pfp->Ibmask == 0xff) {
				fprintf(iC_outFP, "	%c%s%s%s	",
				    pfp->Iinv ? '~' : ' ', pfp->Igate->gt_ids, iidSep, iidPtr);
			    } else {
				fprintf(iC_outFP, "	%c%s%s%s	0x%02x",
				    pfp->Iinv ? '~' : ' ', pfp->Igate->gt_ids, iidSep, iidPtr, pfp->Ibmask);
			    }
			} else {
			    fprintf(iC_outFP, (pfp->QPgate != NULL) ? "		" : "	Dummy	");
			}
			if (pfp->QPgate != NULL) {
			    if (*iC_iidNM == '\0' || strchr(pfp->QPgate->gt_ids, '-') != NULL) {
				iidSep = iidPtr = "";		/* null instance or local instance */
			    } else {
				iidSep = "-";
				iidPtr = iC_iidNM;			/* global instance */
			    }
			    if (pfp->QPbmask == 0xff) {
				fprintf(iC_outFP, "	%c%s%s%s	",
				    pfp->QPinv ? '~' : ' ', pfp->QPgate->gt_ids, iidSep, iidPtr);
			    } else {
				fprintf(iC_outFP, "	%c%s%s%s	0x%02x",
				    pfp->QPinv ? '~' : ' ', pfp->QPgate->gt_ids, iidSep, iidPtr, pfp->QPbmask);
			    }
			} else {
			    fprintf(iC_outFP, "		");
			}
			fprintf(iC_outFP, "	%3hu	%3hu	%3hu	%2hu  %2d\n",
			    pfp->Qchannel, pfp->Ichannel, pfp->QPchannel, pfp->pfa, pfp - iC_pfL);
			if (pfp == pfCADpfp) {
			    fprintf(iC_outFP, "	 %s					%3hu			%2hu  %2d\n",
				pfCADgate.gt_ids, pfCADchannel, pfp->pfa, pfp - iC_pfL);
			}
		    }
		    fprintf(iC_outFP, "\n");
		}
		if (iC_gpL[0] || iC_gpL[1]) {
		    fprintf(iC_outFP, "Allocation for GPIO elements, global instance = \"%s\"\n", iC_iidNM);
		    fprintf(iC_outFP, "	Bit IEC	inst	gpio	channel\n\n");
		    for (iq = 0; iq < 2; iq++) {
			for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
			    strcpy(buffer, gep->Ggate->gt_ids);		/* retrieve name[-instance] */
			    iidSep = "-";
			    if ((iidPtr = strchr(buffer, '-')) != NULL) {
				*iidPtr++ = '\0';		/* separate name and instance */
			    } else if (*iC_iidNM) {
				iidPtr = iC_iidNM;		/* global instance */
			    } else {
				iidSep = iidPtr = "";		/* null instance */
			    }
			    for (bit = 0; bit <= 7; bit++) {
				if ((gpio = gep->gpioNr[bit]) != 0xffff) {	/* saved gpio number for this bit */
				    fprintf(iC_outFP, "	%c%s.%hu	%s%s	%3hu	%3hu\n",
					gep->Ginv & iC_bitMask[bit] ? '~' : ' ',
					buffer, bit, iidSep, iidPtr, gpio, gep->Gchannel);
				}
			    }
			}
			fprintf(iC_outFP, "\n");
		    }
		}
	    }
	}
	/********************************************************************
	 *	Set bits to wait for SIO interrupts on GPIO25
	 *******************************************************************/
	if (gpio25FN > 0) {
	    FD_SET(gpio25FN, &ixfds);		/* watch GPIO25 for out-of-band input - do after iC_connect_to_server() */
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 04) fprintf(iC_outFP, "*** SIO interrupt has been primed\n");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	}
	/********************************************************************
	 *  Set all GPIO IXn input bits for interrupts
	 *******************************************************************/
	for (gep = iC_gpL[1]; gep; gep = gep->nextIO) {	/* IXn inputs only */
	    for (bit = 0; bit <= 7; bit++) {
		if ((gpio = gep->gpioNr[bit]) != 0xffff) {
		    assert(gep->gpioFN[bit] > 0);		/* make sure it has been opened */
		    FD_SET(gep->gpioFN[bit], &ixfds);		/* watch GPIO N for out-of-band input */
		}
	    }
	}
#endif	/* RASPBERRYPI */
#ifndef	WIN32
	if ((iC_debug & DZ) == 0) {
	    FD_SET(0, &infds);			/* watch stdin for inputs unless - FD_CLR on EOF */
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 04) fprintf(iC_outFP, "*** stdin interrupt has been primed\n");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	}	/* can only use sockets, not file descriptors under WINDOWS - use kbhit() */
#endif	/* WIN32 */
    }

/********************************************************************
 *
 *	Initialise the work lists to empty lists
 *
 *******************************************************************/

    iC_error_flag = 0;
    alist0.gt_rptr = iC_a_list = &alist1;	/* initialise alternate */
    Out_init(iC_a_list);
    alist1.gt_rptr = iC_a_list = &alist0;	/* start with alist0 */
    Out_init(iC_a_list);
    olist0.gt_rptr = iC_o_list = &olist1;	/* initialise alternate */
    Out_init(iC_o_list);
    olist1.gt_rptr = iC_o_list = &olist0;	/* start with olist0 */
    Out_init(iC_o_list);
#ifdef	LOAD
    iC_c_list = &iClock;				/* system clock list */
    Out_init(iC_c_list);
#endif	/* LOAD */
    Out_init(iC_f_list);
    Out_init(iC_s_list);
#if	YYDEBUG
    if (iC_debug & 0100) {
	if (iC_debug & DQ) {
	    fprintf(iC_outFP, "== Init scan ===========");
	} else {
	    fprintf(iC_outFP, "\nINITIALISATION\n");
	}
    }
#endif	/* YYDEBUG */

    /* if (iC_debug & 0400) == 0 then no live bits are set in gt_live | 0x8000 */
    /* header for live data */
    /********************************************************************
     *  Initialise msgBuf for live data collection before initialisation
     *  liveOffset may also be used with messages in regBuf because they
     *  use the same channel number and all relevant messages have only
     *  one command digit.
     *  Message in msgBuf is ignored of iC_debug&0400 stops process after init
     *******************************************************************/
    msgOffset = liveOffset = snprintf(msgBuf, REQUEST, "%hu:3", C_channel);
    /********************************************************************
     *  Carry out 4 Passes to initialise all Gates
     *******************************************************************/
    for (pass = 0; pass < 4; pass++) {
#if	YYDEBUG
	if ((iC_debug & (DQ|0100)) == 0100) fprintf(iC_outFP, "\n== Pass %d:", pass + 1);
#endif	/* YYDEBUG */
	for (opp = sTable; opp < sTend; opp++) {
	    gp = *opp;
	    typ = gp->gt_ini > 0 ? AND : -gp->gt_ini;
	    if (typ < MAX_OP) {
		init_fa = iC_i_lists[typ][pass];
		(*init_fa)(gp, typ);	/* initialise for this pass */
	    }
	}
    }
    iC_osc_max = iC_osc_lim;		/* during Init oscillations were not checked */

#if	YYDEBUG
    if (iC_debug & 0100) {
	fprintf(iC_outFP, "\n== Init complete =======\n");
    }
#endif	/* YYDEBUG */

    if (iC_error_flag) {
	if (iC_error_flag >= 2) {
	    fprintf(iC_outFP, "\n*** Fatal Errors ***\n");
	    iC_quit(SIGUSR1);
	}
	fprintf(iC_outFP, "\n*** Warnings ***\n");
    }
    if (iC_debug & 0400) {
	iC_quit(0);			/* terminate - no inputs */
    }
    signal(SIGINT, iC_quit);		/* catch ctrlC and Break */

#ifdef	SIGTTIN
    /********************************************************************
     *  The following behaviour was observed on Linux kernel 2.2
     *  When this process is running in the background, and a command
     *  (even a command which does not exist) is run from the bash from
     *  the same terminal on which this process was started, then this
     *  process is sent signal SIGTTIN. If the signal SIGTTIN is ignored,
     *  then stdin receives EOF (a few thousand times) every time a
     *  character is entered at the terminal.
     *
     *  To prevent this process from hanging when run in the background,
     *  SIGTTIN is ignored, and when the first EOF is received on stdin,
     *  FD_CLR(0, &infds) is executed to stop selects on stdin from then on.
     *  This means, that stdin can still be used normally for the q, t, x, d
     *  and m command when the process is run in the foreground, and behaves
     *  correctly (does not STOP mysteriously) when run in the background.
     *
     *  This means that such a process cannot be stopped with q, only with
     *  ctrl-D, when it has been brought to the foreground with fg.
     *******************************************************************/
    signal(SIGTTIN, SIG_IGN);		/* ignore tty input signal in bg */
#endif	/* SIGTTIN */

    /********************************************************************
     *  Prepare index entries first to allow ALIAS back-references
     *  in VCD file generation and for sending Symbol Table to iClive
     *  No variable has index 0 - used as a flag in BREAK logic
     *******************************************************************/
    index        = 0			/* start with index 1 after initial increment */;
    iClock_index = 0;
    debugBlock   = 0x1fff;		/* mask debug and live bits */
    for (opp = sTable; opp < sTend; opp++) {
	if (++index == 0x2000) {
	    debugBlock = 0x7fff;	/* mask live bit only - cannot fit debug bits */
	} else if (index == 0x8000) {
	    debugBlock = 0xffff;	/* mask no bits - cannot even fit live bit */
	}
	gp = *opp;
	if (iC_vcd && iC_argh <= 0) {
	    if (
#ifdef	LOAD
		gp == &iClock
#else	/* LOAD */
		strcmp(gp->gt_ids, "iClock") == 0
#endif	/* LOAD */
	    ) {
		iClock_index = index;	/* must be set before next loop in case first symbol comes before iClock */
	    }
	}
	if (debugBlock != 0xffff) {	/* prevent setting the live bit */
	    gp->gt_live = index;	/* index and initial live bit 0 */
#if	YYDEBUG
	    if (iC_debug & 040) fprintf(iC_outFP, "%d	%s\n", index, gp->gt_ids);
#endif	/* YYDEBUG */
	}
    }
    /********************************************************************
     *  Generate a VCD file (Value Change Dump), an industry standard
     *  file format specified by IEEE-1364 (initially developed for Verilog).
     *  It is used to display storage oscilloscope type traces of bit
     *  and int signals from a running iC application with GTKWAVE.
     *  Also generate a SAV file, which has information about the traces
     *  to display initially. Inverted ALIASES are handled correctly.
     *******************************************************************/
    if (iC_vcd && iC_argh <= 0) {
	time_t walltime;
	char *	savCode = "";
	int	extended = 0;
	int	extFlag = 0;
	char *	dumpvars;
	char *	dumpPtr;
	char *	dumpEnd;
	char	modName[ESIZE];
	char	modPrev[ESIZE] = "";

	if (index >= 10000) {
	    fprintf(iC_errFP, "\n%s: cannot handle %hu variables in file '%s' - limit 10,000 - not generated\n",
		iC_iccNM, index, iC_vcd);
	} else {
	    time(&walltime);
	    len = 0;
	    if ((iC_vcdFP = fopen(iC_vcd, "w")) == NULL) {
		fprintf(iC_errFP, "\n%s: cannot open vcd file '%s'\n", iC_iccNM, iC_vcd);
		perror("fopen");
		iC_quit(SIGUSR1);
	    }
	    /********************************************************************
	     *  Generate a SAV file if the VCD file has the extension '.vcd'.
	     *  Output standard headers for both the SAV and the VCD file.
	     *******************************************************************/
	    if ((cp = strrchr(iC_vcd, '.')) != NULL && strcmp(++cp, "vcd") == 0) {
		iC_sav = iC_emalloc(strlen(iC_vcd)+1);	/* +1 for '\0' */
		strcpy(iC_sav, iC_vcd);
		cp = strrchr(iC_sav, '.') + 1;
		strcpy(cp, "sav");		/* generate xxx.sav only if xxx.vcd */
		if ((iC_savFP = fopen(iC_sav, "w")) == NULL) {
		    fprintf(iC_errFP, "\n%s: cannot open sav file '%s'\n", iC_iccNM, iC_sav);
		    perror("fopen");
		    iC_quit(SIGUSR1);
		}
		/********************************************************************
		 *  initial SAV header
		 *******************************************************************/
		fprintf(iC_savFP,
		    "[*]\n"
		    "[*] immediate C %s\n"
		    "[*] %s"
		    "[*]\n"
		    "[dumpfile] \"%s\"\n"
		    "[size] 1024 700\n"
		    "[pos] -1 -1\n"
		    "*-4.343955 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1 -1\n"
		, iC_ID, asctime(gmtime(&walltime)), iC_iccNM);	/* zoom, tims.marker, 26 named_markers */
		savCode = sav_ftype[GATE];	/* "wire" instead of "event" for iClock */
		fprintf(iC_savFP,
		    "@%s\n"
		    "%s.iClock\n"		/* display iClock as the first trace */
		, savCode, iC_iccNM);
	    }
	    /********************************************************************
	     *  initial VCD header
	     *******************************************************************/
	    fprintf(iC_vcdFP,
		"$date\n"
		"    %s"
		"$end\n"
		"$version\n"
		"    immediate C %s\n"
		"$end\n"
		"$timescale\n"
		"     1 us\n"
		"$end\n"
		"\n"
		"$scope module %s $end\n"
	    , asctime(gmtime(&walltime)), iC_ID, iC_iccNM);
	    /* index entries have been entered above to allow ALIAS back-references */
	    dumpPtr = dumpvars = iC_emalloc(index * 8);	/* enough for all integer variables with 4 byte codes */
	    dumpEnd = dumpvars + (index * 7);		/* will be free'd in this routine */
	    /********************************************************************
	     *  to maintain index correlation handle all symbols
	     *******************************************************************/
	    for (opp = sTable; opp < sTend; opp++) {
		int		fni;
		Gate *	gm;
		char *	ids;
		char *	idsa;
		char *	code;
		int		size;
		char *	prefix;
		char	iqt[2];		/* single char buffer - space for 0 terminator */
		char	xbwl[2];	/* single char buffer - space for 0 terminator */
		int		byte;
		int		bit;
		char	tail[15];	/* compiler generated suffix .123456_123456 max */
		int		inverse = 0;

		gp = *opp;
		if (gp->gt_ini != -NCONST) {		/* do not trace numbers */
		    idsa = ids = gp->gt_ids;
		    fni = gp->gt_fni;
		    if (fni == UDFA && (gp->gt_ini == -ARNC || gp->gt_ini == -LOGC)) {
			/********************************************************************
			 *  Resolve immC array - the array references are like ALIAS'es.
			 *  Treat the array like a module, whose members are all the array
			 *  reference names recorded in the VCD file. They are not mentioned
			 *  in the SAV file. But they can be selected for display later.
			 *******************************************************************/
			if (extFlag) {
			    fprintf(iC_vcdFP, "$upscope $end\n");
			    extFlag = 0;
			}
			fprintf(iC_vcdFP, "$scope module %s $end\n", ids);	/* use array identifier as module name */
			for (i = 0; i < gp->gt_old; i++) {
			    gm = gp->gt_rlist[i];	/* immC member - cannot be an ALIAS itself */
			    fni = MAX_FTY + gm->gt_fni;	/* match iClive */
			    if ((code = vcd_ftype[fni]) != NULL) {
				size = *code == 'i' ? 32 : 1;
				fprintf(iC_vcdFP, "$var %s %d %d %s[%d] $end\n", code, size, gm->gt_live, ids, i);
			    }
			}
			fprintf(iC_vcdFP, "$upscope $end\n");
		    } else {
			/********************************************************************
			 *  Resolve ALIAS'es
			 *******************************************************************/
			while (gp->gt_ini == -ALIAS) {
			    if (gp->gt_fni == GATE || gp->gt_fni == GATEX) {
				inverse ^= gp->gt_mark;	/* holds ALIAS inversion flag */
			    }
			    gp = gp->gt_rptr;		/* resolve ALIAS */
			    idsa = gp->gt_ids;		/* alias name */
			    fni = MAX_FTY + gp->gt_fni + (inverse << 1);	/* match iClive */
			}
			/********************************************************************
			 *  Ignore variables whose name finishes with _0. They are compiler
			 *  generated output variables, which always follow their base variable.
			 *  Determine extended variables, whose name finish with _1 _2 etc.
			 *  These are generated auxiliary variables, which are not displayed
			 *  initially. They are grouped in modules, which have the base name
			 *  of these variables and which may be selected for display later.
			 *******************************************************************/
			extended = 0;
			if ((cp = strrchr(ids, '_')) != NULL) {
			    len = cp - ids;		/* length of base for generating module */
			    if (len >= ESIZE) len = ESIZE-1;
			    if (strcmp(++cp, "0") == 0) continue;	/* ignore Qnn_0 */
			    extended = 1;		/* possibly an extended iC variable xxx_1 */
			    while (*cp) {
				if (! isdigit(*cp++)) {
				    extended = 0;	/* not an extended iC variable xxx_a */
				    break;
				}
			    }
			}
			/********************************************************************
			 *  Generate a VCD $var line for each displayable variable, which
			 *  associates the name of the variable with its display type, size
			 *  and ID code. Although the ID code can be 1 to 4 bytes of any
			 *  printable ASCII character, in this immdiate C implementation
			 *  the ID codes chosen  are made up of 1 to 4 decimal digits only
			 *  and are in fact the index offset of the internal symbol table
			 *  stored in gt_live of each gate node for the live display in iClive.
			 *  Extended variables are bracketed by a $sope module and a $upscope
			 *  line.
			 *******************************************************************/
			if ((code = vcd_ftype[fni]) != NULL) {
			    if (gp->gt_live == iClock_index) {
				code = vcd_ftype[GATE];	/* "wire" instead of "event" for iClock */
			    }
			    if (extended) {
				strncpy(modName, ids, len);
				*(modName + len) = '\0';	/* generate substring without extension */
				if ((cp = strrchr(modName, '.')) != 0) {
				    *cp = '_';		/* modify QX0.0 to QX0_0 for module name */
				}
				if (strcmp(modPrev, modName) != 0) {
				    if (extFlag) {
					fprintf(iC_vcdFP, "$upscope $end\n");	/* change in module name */
				    }
				    fprintf(iC_vcdFP, "$scope module %s $end\n", modName);
				    extFlag = 1;
				    strncpy(modPrev, modName, ESIZE);
				}
			    } else if (extFlag) {
				fprintf(iC_vcdFP, "$upscope $end\n");
				extFlag = 0;
			    }
			    prefix = "";
			    if (sscanf(ids, "%1[IQT]%1[XBWL]%5d.%5d%7s", iqt, xbwl, &byte, &bit, tail) == 4) {
				size = 1;
				prefix = "\\";		/* variables containing '.' must be prefixed with '\' in .sav file */
			    } else
			    if (sscanf(idsa, "%1[IQT]%1[BWL]%5d%7s", iqt, xbwl, &byte, tail) >= 3 &&
				(! extended || strcmp(tail, "_0") == 0)) {
				switch (xbwl[0]) {
				case 'B':
				    size = 8;		/* reduce size of QBn although only QBn_0 output is reduced for output */
				    break;		/* NOTE: ALIAS QBn and QWN has size of aliased variable - usually 32 */
				case 'W':
				    size = 16;		/* reduce size of QWn although only QWn_0 output is reduced for output */
				    break;
#if INT_MAX != 32767 || defined (LONG16)
				case 'L':
				    size = 32;
				    break;
#endif	/* INT_MAX != 32767 || defined (LONG16) */
				default:
				    assert(0);		/* no valid output word definition */
				}
			    } else {
				size = *code == 'i' ? 32 : 1;
			    }
			    fprintf(iC_vcdFP, "$var %s %d %d %s $end\n", code, size, gp->gt_live, ids);
			    /********************************************************************
			     *  Generate $dumpvar initialisation string from values produced in
			     *  Pass 1 - Pass 4 above. This starts traces of inverted logic and
			     *  initial arithmetic values correctly.
			     *******************************************************************/
			    if (fni < MAX_FTY) {			/* no initial $dumpvar for ALIAS */
				switch (*code) {
				case 'e':				/* event - this still shows as a '1' tick despite value '0' */
				case 'w':				/* wire */
				    assert(dumpPtr < dumpEnd - 5);	/* enough for one more entry */
				    dumpPtr += sprintf(dumpPtr, " %d%hu", gp->gt_val < 0 ? 1 : 0, gp->gt_live);
				    break;
				case 'i':				/* integer */
				    assert(dumpPtr < dumpEnd - 38);	/* enough for one more entry */
				    dumpPtr += sprintf(dumpPtr, " b%s %hu",
					convert2binary(gp->gt_new, 0),	/* convert to 32 bit binary string */
					gp->gt_live);
				    break;
				default:				/* hard error */
				    assert(0);
				    break;
				}
			    }
			    /********************************************************************
			     *  For the SAV file generate a line for each variable to be displayed
			     *  initially. Precede each group of variables of the same type by the
			     *  SAV types @24 for a decimal integer, @ 28 and @ 68 for normal and
			     *  inverted bit types as well as @28 for clock and timer events.
			     *******************************************************************/
			    if (iC_savFP && ! extended && (code = sav_ftype[fni]) != NULL && gp->gt_live != iClock_index) {
				if (strcmp(savCode, code) != 0) {
				    fprintf(iC_savFP, "@%s\n", code);
				    savCode = code;
				}
				if (size == 1) {
				    fprintf(iC_savFP, "%s.%s%s\n", iC_iccNM, prefix, ids);
				} else {
				    fprintf(iC_savFP, "%s.%s[%d:0]\n", iC_iccNM, ids, size - 1);
				}
			    }
			}
		    }
		}
	    }
	    if (extFlag) {
		fprintf(iC_vcdFP, "$upscope $end\n");
	    }
	    fprintf(iC_vcdFP,
		"$upscope $end\n"					/* finalise initial $scope */
		"$enddefinitions $end\n"
		"\n"
	    );
	    fprintf(iC_vcdFP, "$dumpvars%s $end\n", dumpvars);	/* finally print the $dumpvars string */
	    free(dumpvars);
	}
    } /* end of VCD SAV initialisation */

    /********************************************************************
     *  if TX0.0 - EOI - "End of Initialisation" is used in the program
     *******************************************************************/
    if ((gp = tim[0]) != NULL) {
	/********************************************************************
	 *  0 Carry out one clock scan of all master gates with inverted inputs
	 *    before EOI so that inverted timer delays can be masked with EOI
	 *******************************************************************/
	if (iC_c_list != iC_c_list->gt_next) {
	    if (++iC_mark_stamp == 0) {	/* next generation for oscillator check */
		iC_mark_stamp++;		/* leave out zero */
	    }
	    if (iC_vcdFP) {
		fprintf(iC_vcdFP, "#%ld\n1%hu\n", ++virtualTime, iClock_index);	/* mark active iClock in VCD output */
		vcdFlag = NULL;		/* no 2nd empty clock scan vcd output */
	    }
	    iC_scan_clk(iC_c_list);		/* ignore f_list entries can only occurr here */
	    if (iC_vcdFP) {
		fprintf(iC_vcdFP, "#%ld\n0%hu\n", ++virtualTime, iClock_index);	/* end active iClock in VCD output */
	    }
	    if (iC_f_list != iC_f_list->gt_next) { iC_scan_clk(iC_f_list); }	/* execute f_list entries */
	}
	/********************************************************************
	 *  1 Set and fire "End Of Initialisation"
	 *******************************************************************/
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, "\n== end of initialisation\nEOI:\t%s  1 ==>", gp->gt_ids);
#endif	/* YYDEBUG */
	gp->gt_val = -1;		/* set EOI once as first action */
	iC_link_ol(gp, iC_o_list);	/* fire EOI Input Gate */
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, " -1\n");
#endif	/* YYDEBUG */
	iC_liveData(gp, 1);		/* VCD and/or iClive */
    }

    /********************************************************************
     *  The following initialisation function is an empty function
     *  in the libict.a support library.
     *		int iCbegin(int argc, char** argv) { return -1; }
     *  It may be implemented in a literal block of an iC source, in
     *  which case that function will be linked in preference.
     *  User implementations of iCbegin() should not return -1, (usually 0)
     *  to activate the debug message "== iCbegin complete ====".
     *
     *  It can be used to initialise immC variables etc. For this reason
     *  it should be executed once after EOI, but before normal processing.
     *  It can use the initial values of all immediate variables.
     *
     *  If the iCbegin() function contains a fork() call, a child process is
     *  spawned, which will run in parallel with immediate processing.
     *******************************************************************/
    if (iCbegin(iC_argc, iC_argv) != -1) {	/* initialisation function */
#if	YYDEBUG
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\n== iCbegin complete ====\n");
	}
#endif	/* YYDEBUG */
    }
    outPtr = iC_outBuf;			/* used in folowing initialisation and operational loop only */
    outBufLen = REQUEST;

#ifdef	RASPBERRYPI
    if (iC_opt_P) {
	/********************************************************************
	 *  Generate PiFace initialisation inputs and outputs for active PiFaces
	 *  Send possible TCP/IP after GPIO done
	 *******************************************************************/
	if (!iC_opt_G) {
	    assert(gpio25FN > 0);
	    if (iC_debug & 0200) fprintf(iC_outFP, "== Initialise active PiFace units\n");
	    gpio_read(gpio25FN);		/* dummy read to clear interrupt on /dev/class/gpio25/value */
	    if (iC_micro) iC_microPrint("SPI initialise", 0);
	    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
		if (pfp->Qgate) {
		    val = 0;				/* initialise with logical 0 */
		    pfp->Qval = pfp->Qinv ^ 0xff;	/* inverted to force all output bits */
		    iC_output(val, pfp->Qchannel);	/* write output Gates directly and/or via TCP/IP */
		}
		if (pfp->Igate) {
		    val = readByte(pfp->spiFN, pfp->pfa, pfp->inpr);
		    pfp->Ival = val ^ 0xff;		/* inverted to force distributing input bits */
		    fireInput(pfp, NULL, val);		/* ini input Gates directly and/or via TCP/IP */
		}
		if (pfp->QPgate) {
		    val = 0;				/* initialise with logical 0 */
		    pfp->QPval = pfp->QPinv ^ 0xff;	/* inverted to force all output bits */
		    iC_output(val, pfp->QPchannel);	/* write output Gates directly and/or via TCP/IP */
		}
	    }
	}
	/********************************************************************
	 *  Extend with GPIO IXn initialisation inputs
	 *  There may be no GPIOs or only GPIO outputs - nothing to initialise
	 *******************************************************************/
	for (gep = iC_gpL[1]; gep; gep = gep->nextIO) {	/* IXn inputs only */
	    val = 0;
	    for (bit = 0; bit <= 7; bit++) {
		if ((gpio = gep->gpioNr[bit]) != 0xffff) {
		    if ((b = gpio_read(gep->gpioFN[bit])) != -1) {
			if (b) val |= iC_bitMask[bit];
		    } else {
			fprintf(iC_errFP, "WARNING: %s: GPIO %hu returns invalid value -1 (not 0 or 1 !!)\n",
			    iC_progname, gpio);	/* should not happen */
		    }
		}
	    }
	    gep->Gval = val ^ 0xff;		/* inverted to force distributing input bits */
	    fireInput(NULL, gep, val);		/* ini input Gates directly and/or via TCP/IP */
	}
	/********************************************************************
	 *  Send IXn inputs to iCsever to initialise receivers
	 *******************************************************************/
	sendOutput();				/* Send 1 block of initialisation data (if any) */
	/********************************************************************
	 *  Write GPIO QXn initialisation outputs
	 *******************************************************************/
	for (gep = iC_gpL[0]; gep; gep = gep->nextIO) {	/* QXn outputs only */
	    val = 0;				/* initialise with logical 0 */
	    gep->Gval = gep->Ginv ^ 0xff;	/* inverted to force all output bits */
	    iC_output(val, gep->Gchannel);	/* write output Gates directly and/or via TCP/IP */
	}
    }

#endif	/* RASPBERRYPI */
    /********************************************************************
     *  Operational loop
     *******************************************************************/
    regOffset = 0;				/* regBuf is now clean for debugWait() re-transmits */
#if	YYDEBUG
    waitCount = 0;
#endif	/* YYDEBUG */
    for (;;) {
	/********************************************************************
	 *  Sequencing of different action lists and New I/O handling
	 *
	 ****** INITIALISATION *****
	 * -1          build ini string
	 *  0          scan c_list for inverted ini inputs unless c_list empty if TX0.0
	 *  1          put EOI on o_list                                       if TX0.0
	 *    Loop:
	 *  2          ++mark_stamp to control oscillations
	 ****** COMBINATORIAL PHASE *****
	 *  3          scan a_list unless a_list empty
	 *                 INPW ARITH expr results to a_list
	 *                 comparisons, &&, || to o_list
	 *                 clocked actions to c_list via own clock list
	 *  4        { scan o_list; goto Loop } unless o_list empty
	 *                 bit actions to o_list
	 *                 bits used in arithmetic to a_list (less common)
	 *                 clocked actions to c_list via own clock list
	 ****** CLOCK PHASE *******
	 * 5         { ++mark_stamp to control oscillations
	 *             scan c_list; DO 5; goto Loop } unless c_list empty
	 *                 transfer ARITH master values as slave values to a_list
	 *                 transfer GATE master values as slave values to o_list
	 *                 (does not use any combinatorial ARITH or GATE values)
	 *                 transfer master entries on slave clock lists to c_list
	 *                 (continue scanning c_list until all these have been handled)
	 *                 defer 'if else switch' slave C actions to f_list
	 ****** COMBINATORIAL PHASE *****
	 *  6        { scan f_list; goto Loop } unless f_list empty
	 *                 C actions can use and generate combinatotrial ARITH and
	 *                 GATE values, which is start of a new combinatorial scan
	 ****** SEND PHASE *****
	 *  7        { scan s_list; } unless s_list empty # only one scan is required
	 *                 do OUTW and OUTX Gates building send string from channels
	 *  8          send output string with final outputs only (also ini string)
	 *  9          switch to alternate a_list and o_list
	 ****** IDLE PHASE *****
	 * 10          wait for next input or timer
	 ****** INPUT PHASE *****
	 * 11          read new input and link INPW Gates directly to a_list
	 *             or via traMb to o_list
	 *    goto Loop
	 *******************************************************************/
	if (++iC_mark_stamp == 0) {	/* next generation for oscillator check */
	    iC_mark_stamp++;		/* leave out zero */
	}
	vcdFlag = iC_vcdFP;
	for (;;) {
	    if (iC_a_list != iC_a_list->gt_next) { iC_scan_ar (iC_a_list);           }
	    if (iC_o_list != iC_o_list->gt_next) { iC_scan    (iC_o_list); continue; }
	    if (iC_c_list != iC_c_list->gt_next) {
		if (++iC_mark_stamp == 0) {	/* next generation for oscillator check */
		    iC_mark_stamp++;		/* leave out zero */
		}
		if (iC_vcdFP) {
		    fprintf(iC_vcdFP, "#%ld\n1%hu\n", ++virtualTime, iClock_index);	/* mark active iClock in VCD output */
		    vcdFlag = NULL;		/* no 2nd empty clock scan vcd output */
		}
		iC_scan_clk(iC_c_list);		/* new f_list entries can only occurr here */
		if (iC_vcdFP) {
		    fprintf(iC_vcdFP, "#%ld\n0%hu\n", ++virtualTime, iClock_index);	/* end active iClock in VCD output */
		}
		if (iC_f_list != iC_f_list->gt_next) { iC_scan_clk(iC_f_list); }	/* execute f_list entries */
		continue;
	    } else if (vcdFlag) {
		fprintf(iC_vcdFP, "#%ld\n1%hu\n", ++virtualTime, iClock_index);	/* mark non active iClock in VCD output */
		fprintf(iC_vcdFP, "#%ld\n0%hu\n", ++virtualTime, iClock_index);	/* end non active iClock in VCD output */
	    }
	    if (iC_s_list != iC_s_list->gt_next) { iC_scan_snd(iC_s_list);           }
	    break;
	}
	sendOutput();				/* Send last (usually only) block of output data */

	/********************************************************************
	 *  Switch to alternate lists
	 *
	 *  alternate list contains all those gates which were marked in
	 *  the previous scan and which were active more than
	 *  MARKMAX times. These are oscillators which wil be
	 *  scanned again in the next cycle.
	 *******************************************************************/
	iC_a_list = iC_a_list->gt_rptr;	/* alternate arithmetic list */
	iC_o_list = iC_o_list->gt_rptr;	/* alternate logic list */

	/********************************************************************
	 *  Send live data collected in msgBuf during initialisation
	 *  and previous loop to iCserver.
	 *  Initialise msgBuf for live data collection during next loop.
	 *******************************************************************/
	if (msgOffset > liveOffset) {
	    iC_send_msg_to_server(iC_sockFN, msgBuf);
	    msgOffset = liveOffset;			/* msg = "C_channel:3" */
	}
	if (iC_micro) iC_microPrint("Scan complete", 0);

#if	YYDEBUG
	if (iC_scan_cnt || iC_link_cnt) {
	    if (iC_debug & 0100) fprintf(iC_outFP, "\n");
	    if (iC_debug & 02000) {
		fprintf(iC_outFP, "scan = %5u  link = %5u",
		    iC_scan_cnt, iC_link_cnt);
		if (iC_glit_cnt) {
		    fprintf(iC_outFP, "  glitch = %5u, %lu", iC_glit_cnt, iC_glit_nxt);
		}
		fprintf(iC_outFP, "\n");
	    }
	    iC_glit_cnt = iC_glit_nxt =
	    iC_scan_cnt = iC_link_cnt = 0;
	}
#endif	/* YYDEBUG */
	if (iC_osc_gp) {
	    fprintf(iC_outFP,
		"*** Warning: %s has oscillated %d times - check iC program!!!\n",
		iC_osc_gp->gt_ids, iC_osc_gp->gt_mcnt);
	    iC_osc_gp = NULL;
	}

#if	YYDEBUG
	waitCount++;
	if (iC_debug & 0300) {		/* osc or detailed info */
	    if ((iC_debug & 0200) &&
		(iC_a_list->gt_next != iC_a_list || iC_o_list->gt_next != iC_o_list)) {
		fprintf(iC_outFP, "OSC =");
		for (gp = iC_a_list->gt_next; gp != iC_a_list; gp = gp->gt_next) {
		    fprintf(iC_outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
		}
		for (gp = iC_o_list->gt_next; gp != iC_o_list; gp = gp->gt_next) {
		    fprintf(iC_outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
		}
		fprintf(iC_outFP, "\n");
		goto outWAIT;		/* output WAIT - do not test 0100 */
	    }
	}
	if (iC_debug & 0100) {		/* end of cycle */
	  outWAIT:
	    fprintf(iC_outFP, "== WAIT %5u ==========\n", waitCount);
	    fflush(iC_outFP);
#ifdef	RASPBERRYPI
	    if ((slr &= ~0x07) == 0) {	/* stop 350 ms timeout - no 2nd wait message needed */
		toCntp = toCoff;	/* slr also holds shift direction - not touched */
	    }
#endif	/* RASPBERRYPI */
	}
#endif	/* YYDEBUG */

	/********************************************************************
	 *  Any input I.. and other data collected during a break in debugWait()
	 *  is retransmitted to iCserver now to be received again on own channels.
	 *******************************************************************/

	if (regOffset) {
	    iC_send_msg_to_server(iC_sockFN, regBuf);
	    regOffset = 0;
	}

	/********************************************************************
	 *  Input from external input modules and time input (if used)
	 *******************************************************************/
	if (iC_vcdFP) {
	    virtualTime += 11;
	    virtualTime -= (virtualTime%10 + 1);	/* next input is on 10's boundary */
	}
	for (cnt = 0; cnt == 0; ) {		/* stay in input loop if nothing linked */
	    /********************************************************************
	     *  Turn TX0.1 lo in a cycle by itself before waiting for further I/O
	     *******************************************************************/
	    if (stdinFlag) {
		gp = tim[1];			/* TX0.1 */
		assert(gp && gp->gt_val == -1);	/* must be programmed if stdinFlag was set */
#if	YYDEBUG
		if (iC_debug & 0100) fprintf(iC_outFP, "\n%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
		gp->gt_val = 1;			/* set  TX0.1 lo */
		iC_link_ol(gp, iC_o_list);	/* fire TX0.1 lo to terminate notification */
#if	YYDEBUG
		if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
#endif	/* YYDEBUG */
		iC_liveData(gp, 0);		/* VCD and/or iClive */
		stdinFlag = 0;			/* ready for next STDIN */
		break;				/* do a scan - no need to increment cnt by using break */
	    }
	    /********************************************************************
	     *  Wait for input or timer interrupts in a select() statement
	     *  most of the time
	     *******************************************************************/
	    retval = iC_wait_for_next_event(&infds, &ixfds, iC_osc_flag ? &toCnt : toCntp);
	    if (iC_osc_flag) {
		cnt++;				/* gates have been linked to alternate list - do a scan */
		iC_osc_flag = 0;		/* normal timer operation again */
	    }
	    if (retval == 0) {
		/********************************************************************
		 *  Timer interrupt
		 *******************************************************************/
		if (toCnt.tv_sec == 0 && toCnt.tv_usec == 0) {
		    toCnt = iC_timeOut;		/* transfer timeout value */
		    if (t5msFlag == 0) goto T50;		/* if no 10 ms timer iC_timeOut is 50ms */
		    if ((gp = tim[3]) != 0) {			/* 10 millisecond timer */
#if	YYDEBUG
			if (iC_debug & 01000) {
			    Gate **	lp;
			    Gate *	tp;
			    int	cn1 = 2;
			    lp = gp->gt_list;			/* test if any clock or timer is active */
			    do {				/* for normal and inverted */
				while ((tp = *lp++) != 0) {
				    if (tp->gt_fni == CLCK || tp->gt_fni == TIMR) {
					tp = tp->gt_funct;
					if (tp->gt_next != tp) {
					    goto linkT3;	/* found an active clock or timer */
					}
				    } else {
					goto linkT3;	/* found a link to non clock or timer */
				    }
				}
			    } while (--cn1);
			    goto skipT3;			/* excuse spaghetti - faster without flag */
			}
		    linkT3:
			if (iC_micro && !cnt) iC_microPrint("Timer TX0.3 received", 0);
			if (iC_debug & 0100) fprintf(iC_outFP, "\n%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
			gp->gt_val = - gp->gt_val;		/* complement input */
			iC_link_ol(gp, iC_o_list);		/* 5 ms on, 5 ms off is 10ms */
#if	YYDEBUG
			if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
#endif	/* YYDEBUG */
			iC_liveData(gp, gp->gt_val < 0 ? 1 : 0);	/* VCD and/or iClive */
			cnt++;
#if	YYDEBUG
		    skipT3: ;
#endif	/* YYDEBUG */
		    }
		    if (--t50m <= 0) {
			t50m = 10;
		      T50:
#ifdef	RASPBERRYPI
			if (slr) {			/* most of the time slr == 0, test only once */
#if YYDEBUG && !defined(_WINDOWS)
			    /********************************************************************
			     *  Handle P: debug message timeout of 350 ms
			     *******************************************************************/
			    if ((slr & 0x07) != 0) {
				if ((--slr & 0x07) == 0) {	/* 350 ms timer (7*50ms) */
				    fprintf(iC_outFP, "\n======== WAIT ==========\n");
				    fflush(iC_outFP);
				    if (slr == 0) {		/* slr holds shift direction */
					toCntp = toCoff;	/* stop 350 ms timeout unless iC TX0 uses it */
				    }
				}
			    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			    /********************************************************************
			     *  Handle PiFaceCAD shift every 750 ms
			     *******************************************************************/
			    if ((slr & 0x18) && --tcnt_750 <= 0) {
				tcnt_750 = 15;			/* 750 ms timer (15*50ms) */
				if (slr & 0x08) {
				    pifacecad_lcd_move_left();	/* move text left every 750 ms */
				} else {
				    pifacecad_lcd_move_right();	/* move text right every 750 ms */
				}
			    }
			}
#endif	/* RASPBERRYPI */
			/********************************************************************
			 *  TIMERS here every 100 milliseconds
			 *
			 *  The iC_debug facility -d1000 stops linking the 10 ms to 60 second
			 *  TIMERS tim[3] to tim[7] when they are connected directly to
			 *  the slave input of a clock or timer Gate and that clock or timer
			 *  has no Gates on the clock list it controls - ie nothing will happen.
			 *  This stops continuous output when tracing logic (-d100 or -t) when
			 *  these internal TIMERS produce no change.
			 *  It also reduces the data traffic to iClive and reduces flashing of
			 *  clocks and timers controlled by internal TIMERS.
			 *******************************************************************/
			if ((gp = tim[4]) != 0) {		/* 100 millisecond timer */
#if	YYDEBUG
			    if (iC_debug & 01000) {
				Gate **	lp;
				Gate *	tp;
				int	cn1 = 2;
				lp = gp->gt_list;		/* test if any clock or timer is active */
				do {				/* for normal and inverted */
				    while ((tp = *lp++) != 0) {
					if (tp->gt_fni == CLCK || tp->gt_fni == TIMR) {
					    tp = tp->gt_funct;
					    if (tp->gt_next != tp) {
						goto linkT4;	/* found an active clock or timer */
					    }
					} else {
					    goto linkT4;	/* found a link to non clock or timer */
					}
				    }
				} while (--cn1);
				goto skipT4;			/* excuse spaghetti - faster without flag */
			    }
			linkT4:
			    if (iC_micro && !cnt) iC_microPrint("Timer TX0.4 received", 0);
			    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
			    gp->gt_val = - gp->gt_val;		/* complement input */
			    iC_link_ol(gp, iC_o_list);		/* 50 ms on, 50 ms off is 100ms */
#if	YYDEBUG
			    if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
#endif	/* YYDEBUG */
			    iC_liveData(gp, gp->gt_val < 0 ? 1 : 0);	/* VCD and/or iClive */
			    cnt++;
#if	YYDEBUG
			skipT4: ;
#endif	/* YYDEBUG */
			}
			if (--t500m <= 0) {
			    t500m = 10;
			    if ((gp = tim[5]) != 0) {		/* 1 second timer */
#if	YYDEBUG
				if (iC_debug & 01000) {
				    Gate **	lp;
				    Gate *	tp;
				    int	cn1 = 2;
				    lp = gp->gt_list;		/* test if any clock or timer is active */
				    do {			/* for normal and inverted */
					while ((tp = *lp++) != 0) {
					    if (tp->gt_fni == CLCK || tp->gt_fni == TIMR) {
						tp = tp->gt_funct;
						if (tp->gt_next != tp) {
						    goto linkT5;	/* found an active clock or timer */
						}
					    } else {
						goto linkT5;	/* found a link to non clock or timer */
					    }
					}
				    } while (--cn1);
				    goto skipT5;		/* excuse spaghetti - faster without flag */
				}
			    linkT5:
				if (iC_micro && !cnt) iC_microPrint("Timer TX0.5 received", 0);
				if (iC_debug & 0100) fprintf(iC_outFP, "\n%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
				gp->gt_val = - gp->gt_val;	/* complement input */
				iC_link_ol(gp, iC_o_list);	/* 500 ms on, 500 ms off is 1 second */
#if	YYDEBUG
				if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
#endif	/* YYDEBUG */
				iC_liveData(gp, gp->gt_val < 0 ? 1 : 0);	/* VCD and/or iClive */
				cnt++;
#if	YYDEBUG
			    skipT5: ;
#endif	/* YYDEBUG */
			    }
			    if (--t5s <= 0) {
				t5s = 10;
				if ((gp = tim[6]) != 0) {	/* 10 second timer */
#if	YYDEBUG
				    if (iC_debug & 01000) {
					Gate **	lp;
					Gate *	tp;
					int	cn1 = 2;
					lp = gp->gt_list;	/* test if any clock or timer is active */
					do {			/* for normal and inverted */
					    while ((tp = *lp++) != 0) {
						if (tp->gt_fni == CLCK || tp->gt_fni == TIMR) {
						    tp = tp->gt_funct;
						    if (tp->gt_next != tp) {
							goto linkT6;/* found an active clock or timer */
						    }
						} else {
						    goto linkT6;/* found a link to non clock or timer */
						}
					    }
					} while (--cn1);
					goto skipT6;		/* excuse spaghetti - faster without flag */
				    }
				linkT6:
				    if (iC_micro && !cnt) iC_microPrint("Timer TX0.6 received", 0);
				    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
				    gp->gt_val = - gp->gt_val;	/* complement input */
				    iC_link_ol(gp, iC_o_list);	/* 5 sec on, 5 sec off is 10 sec */
#if	YYDEBUG
				    if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
#endif	/* YYDEBUG */
				    iC_liveData(gp, gp->gt_val < 0 ? 1 : 0);	/* VCD and/or iClive */
				    cnt++;
#if	YYDEBUG
				skipT6: ;
#endif	/* YYDEBUG */
				}
				if (--t30s <= 0) {
				    t30s = 6;
				    if ((gp = tim[7]) != 0) {	/* 60 second timer */
#if	YYDEBUG
					if (iC_debug & 01000) {
					    Gate **	lp;
					    Gate *	tp;
					    int	cn1 = 2;
					    lp = gp->gt_list;	/* test if any clock or timer is active */
					    do {		/* for normal and inverted */
						while ((tp = *lp++) != 0) {
						    if (tp->gt_fni == CLCK || tp->gt_fni == TIMR) {
							tp = tp->gt_funct;
							if (tp->gt_next != tp) {
							    goto linkT7;/* found an active clock or timer */
							}
						    } else {
							goto linkT7;/* found a link to non clock or timer */
						    }
						}
					    } while (--cn1);
					    goto skipT7;	/* excuse spaghetti - faster without flag */
					}
				    linkT7:
					if (iC_micro && !cnt) iC_microPrint("Timer TX0.7 received", 0);
					if (iC_debug & 0100) fprintf(iC_outFP, "\n%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
					gp->gt_val = - gp->gt_val;	/* complement input */
					iC_link_ol(gp, iC_o_list);	/* 30 sec on, 30 sec off is 60 sec */
#if	YYDEBUG
					if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
#endif	/* YYDEBUG */
					iC_liveData(gp, gp->gt_val < 0 ? 1 : 0);	/* VCD and/or iClive */
					cnt++;
#if	YYDEBUG
				    skipT7: ;
#endif	/* YYDEBUG */
				    }
				}
			    }
			}
		    }
		}
	    } else if (retval > 0) {
		/********************************************************************
		 *  TCP/IP input from iCserver
		 *******************************************************************/
		if (iC_sockFN > 0 && FD_ISSET(iC_sockFN, &iC_rdfds)) {
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 04) fprintf(iC_outFP, "*ML TCP interrupt");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    if (iC_rcvd_msg_from_server(iC_sockFN, rpyBuf, REPLY) != 0) {
#if YYDEBUG && !defined(_WINDOWS)
			if (iC_debug & 04) { fprintf(iC_outFP, " << %s\n", rpyBuf); fflush(iC_outFP); }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			if (iC_micro && !cnt) iC_microPrint("Input received", 0);
			cp = rpyBuf - 1;	/* increment to first character in rpyBuf in first use of cp */
			if (isdigit(rpyBuf[0])) {
			    char *	cpe;
			    char *	cps;

			    assert(Channels);
			    do {
				if ((cpe = strchr(++cp, ',')) != NULL) { /* find next comma in input */
				    *cpe = '\0';	/* split off leading comma separated token */
				}
//    fprintf(iC_outFP, " TT %s TT", cp); fflush(iC_outFP);
				if (
				    (cps = strchr(cp, ':')) != NULL &&	/* strip only first ':' separating channel and data */
				    (channel = (unsigned short)atoi(cp)) > 0 &&
				    channel <= topChannel &&
#ifndef	RASPBERRYPI
				    (gp = Channels[channel]) != NULL
#else	/* RASPBERRYPI */
				    ((gp = Channels[channel].g) != NULL ||
				    (pqSel = Channels[channel].pqs) != 0)
#endif	/* RASPBERRYPI */
				) {
#if	INT_MAX == 32767 && defined (LONG16)
				    val = atol(++cps);			/* val required for RQ ExtOut */
#else	/* INT_MAX == 32767 && defined (LONG16) */
				    val = atoi(++cps);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#ifdef	RASPBERRYPI
				  if (gp) {				/* RI External */
				    if (gp == &pfCADgate) {
					cp = cps - 1;			/* on : before display string */
					while ((cp = strchr(cp+1 , '\036')) != NULL) {	/* ASCII RS */
					    *cp = ',';			/* replace every RS by a comma */
					}
					writePiFaceCAD(cps, 0);		/* display on local PiFaceCAD */
				    } else
#endif	/* RASPBERRYPI */
				    if (val != gp->gt_new &&		/* first change or glitch */
				    ((gp->gt_new = val) != gp->gt_old) ^ (gp->gt_next != 0)) {
					/* arithmetic master action */
					if (gp->gt_fni == TRAB) {
#if	YYDEBUG
#if	INT_MAX == 32767 && defined (LONG16)
					    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s<\t%hu:%ld\t0x%02lx ==>> 0x%02lx",
						gp->gt_ids, channel, gp->gt_new, gp->gt_old, gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
					    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s<\t%hu:%d\t0x%02x ==>> 0x%02x",
						gp->gt_ids, channel, gp->gt_new, gp->gt_old, gp->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
					    else if (iC_debug & 020) fprintf(iC_outFP, "%s\t%2hu:%s\n",
						gp->gt_ids, channel, convert2binary(gp->gt_new, 1));
#endif	/* YYDEBUG */
					    cnt += iC_traMb(gp, 0);	/* distribute bits directly */
					} else
					if (gp->gt_ini == -INPW) {
#if	YYDEBUG
					    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s[\t", gp->gt_ids);
#if	INT_MAX == 32767 && defined (LONG16)
					    if (iC_debug & 0100) fprintf(iC_outFP, "%hu:%ld\t%ld ==>", channel, gp->gt_new, gp->gt_old);
#else	/* INT_MAX == 32767 && defined (LONG16) */
					    if (iC_debug & 0100) fprintf(iC_outFP, "%hu:%d\t%d ==>", channel, gp->gt_new, gp->gt_old);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#endif	/* YYDEBUG */
					    iC_link_ol(gp, iC_a_list);	/* no actions */
#if	YYDEBUG
#if	INT_MAX == 32767 && defined (LONG16)
					    if (iC_debug & 0100) fprintf(iC_outFP, " %ld", gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
					    if (iC_debug & 0100) fprintf(iC_outFP, " %d", gp->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#endif	/* YYDEBUG */
					    cnt++;
					} else
					if (gp == &D_gate) {		/* D_gate */
					    char *	cp1;

					    assert(gp->gt_fni == UDFA);	/* D_gate initialised type */
					    gp->gt_new = 0;		/* allow repeated 1-6 commands */
					    switch (val) {
					    case 0:			/* IGNORE */
						break;

					    case 1:			/* GET_SYMBOL_TABLE */
#if	YYDEBUG
						if (iC_debug & 0140) fprintf(iC_outFP, "Symbol Table requested by '%s'\n", iC_iccNM);
#endif	/* YYDEBUG */
						/* index entries have been entered to allow ALIAS back-references */
						regOffset = snprintf(regBuf, REQUEST, "%hu:1", C_channel);
						assert(regOffset == liveOffset);
						/* to maintain index correlation send all symbols */
						for (opp = sTable; opp < sTend; opp++) {
						    int		len;
						    int		rest;
						    int		fni;
						    int		i;
						    int		inverse = 0;
						    char *	ids;
						    Gate *	gm;

						    len = 0;
						    gp = *opp;
						    ids = gp->gt_ids;
						    fni = gp->gt_fni;
						    if (fni == UDFA && (gp->gt_ini == -ARNC || gp->gt_ini == -LOGC)) {
							for (i = 0; i < gp->gt_old; i++) {	/* immC array */
							    gm = gp->gt_rlist[i];
							    assert(gm->gt_fni < MAX_FTY);	/* immC member - cannot be an ALIAS itself */
							    fni = MAX_FTY + gm->gt_fni;	/* match iClive */
							    while (rest = REQUEST - regOffset, (len =
								snprintf(&regBuf[regOffset], rest, ";%s[%d] %d %d",
								    ids, i, fni, gm->gt_live & debugBlock)
								) < 0 || len >= rest) {
								regBuf[regOffset] = '\0';	/* terminate */
								if (iC_micro & 06) iC_microPrint("Send Symbols intermediate", 0);
								iC_send_msg_to_server(iC_sockFN, regBuf);
								regOffset = liveOffset;
							    }
							    regOffset += len;
							}
						    }
						    fni = gp->gt_fni;
						    while (gp->gt_ini == -ALIAS) {
							if (gp->gt_fni == GATE || gp->gt_fni == GATEX) {
							    inverse ^= gp->gt_mark;	/* holds ALIAS inversion flag */
							}
							gp = gp->gt_rptr;		/* resolve ALIAS */
							fni = MAX_FTY + gp->gt_fni + (inverse << 1);	/* match iClive */
						    }
						    while (rest = REQUEST - regOffset, (len = fni >= MAX_FTY ?
							snprintf(&regBuf[regOffset], rest, ";%s %d %d",
							    ids, fni, gp->gt_live & debugBlock)              :
							snprintf(&regBuf[regOffset], rest, ";%s %d", ids, fni)
							) < 0 || len >= rest) {
							regBuf[regOffset] = '\0';	/* terminate */
							if (iC_micro & 06) iC_microPrint("Send Symbols intermediate", 0);
							iC_send_msg_to_server(iC_sockFN, regBuf);
							regOffset = liveOffset;
						    }
						    regOffset += len;
						}
						if (regOffset > liveOffset) {
						    if (iC_micro & 06) iC_microPrint("Send Symbols", 0);
						    iC_send_msg_to_server(iC_sockFN, regBuf);
						}
						regOffset = 0;				/* clear for debugWait() */
						/* end of symbol table - execute scan - follow with '0' to leave in iCserver */
						snprintf(regBuf, REQUEST, "%hu:4,%hu:0", C_channel, C_channel);
						if (iC_micro & 06) iC_microPrint("Send Scan Command", 0);
						iC_send_msg_to_server(iC_sockFN, regBuf);
						liveFlag = 1;				/* live inhibit bits are set */
						stepFlag = 0;				/* requires new step next bits */
						break;

					    case 2:			/* iClive poll */
						/********************************************************************
						 *  Receive when re-registering - makes sure, that application is seen
						 *******************************************************************/
						if (liveFlag) {
						    for (opp = sTable; opp < sTend; opp++) {
							(*opp)->gt_live &= 0x7fff;	/* clear live active */
						    }
						    liveFlag = 0;
						}
						/* poll iClive with 'ch:2;<name>' */
						snprintf(regBuf, REQUEST, "%hu:2;%s", C_channel, iC_iccNM);
						if (iC_micro & 06) iC_microPrint("Send application name", 0);
						iC_send_msg_to_server(iC_sockFN, regBuf);
						break;

					    case 3:			/* RECEIVE_ACTIVE_SYMBOLS */
					    case 4:			/* LAST_ACTIVE_SYMBOLS */
						receiveActiveSymbols(rpyBuf);
						if (val == 4) {
						    liveFlag = 1;	/* live inhibit bits are correct */
						}
						break;

					    case 5:			/* GET_END */
						/********************************************************************
						 *  Receive from iClive when Symbol Table is no longer required
						 *******************************************************************/
						if (liveFlag) {
						    for (opp = sTable; opp < sTend; opp++) {
							(*opp)->gt_live &= 0x7fff;	/* clear live active */
						    }
						    liveFlag = 0;
						}
						snprintf(regBuf, REQUEST, "%hu:0", C_channel);
						iC_send_msg_to_server(iC_sockFN, regBuf);
						break;

					    case 6:			/* STOP PROGRAM */
						iC_quit(QUIT_DEBUGGER);	/* quit normally */
						break;

					    case 7:			/* DEBUGGER STOPPED */
#if	YYDEBUG
						if (iC_debug & 0100) fprintf(iC_outFP, "Debugger has stopped for '%s'\n", iC_iccNM);
#endif	/* YYDEBUG */
						snprintf(regBuf, REQUEST, "%hu:0", C_channel);
						iC_send_msg_to_server(iC_sockFN, regBuf);
						break;

					    case 8:			/* RECEIVE_STEP_SYMBOLS */
					    case 9:			/* LAST_STEP_SYMBOLS */
						stepMask = 0x2000;
						goto StepOrNext;
					    case 10:			/* RECEIVE_NEXT_SYMBOLS */
					    case 11:			/* LAST_NEXT_SYMBOLS */
						stepMask = 0x4000;
					      StepOrNext:
						if (debugBlock != 0x1fff) break;	/* cannot set step and next debug */
						if (stepFlag & stepMask) {
						    for (opp = sTable; opp < sTend; opp++) {
							(*opp)->gt_live &= ~stepMask;	/* clear step or next active */
						    }
						    stepFlag &= ~stepMask;		/* do not set again until case 9 or 11 received */
						}
						cp1 = rpyBuf;
						while ((cp1 = strchr(cp1, ';')) != NULL) {
						    index = atoi(++cp1);
						    assert(index <= sTend - sTable);	/* check index is in range */
						    gp = sTable[index-1];		/* index in iClive starts at 1 */
						    gp->gt_live |= stepMask;		/* set step or next active */
#if	YYDEBUG
						    if (iC_debug & 040) fprintf(iC_outFP, "T %3hu	%s	%u %u %3hu\n",
							index, gp->gt_ids, gp->gt_live >> 15, (gp->gt_live >> 13) & 0x3, gp->gt_live & 0x1fff); fflush(iC_outFP);
#endif	/* YYDEBUG */
						}
						if (val == 9 || val == 11) {
						    stepFlag |= stepMask;		/* step or next bits are correct */
						}
						break;

					    case 12:			/* RECEIVE_WATCH_OR_RESTORE */
						if (debugBlock != 0x1fff) break;	/* cannot set step and next debug */
						receiveWatchOrRestore(rpyBuf);
						break;

						/********************************************************************
						 *  DEBUG commands
						 *******************************************************************/
					    case 'd':
						if (debugFlag == 0 && stepFlag == 0x6000) {
						    debugMask = 0x6000;	/* continue till next breakpoint */
						    debugStop = 0;	/* TODO should not be set */
						    debugFlag = 1;	/* start DEBUG mode */
						}
						break;
					    case 's':
						if (debugFlag != 0) {
						    debugMask = 0x2000;	/* single step */
						    debugStop = 0;
						}
						break;
					    case 'n':
						if (debugFlag != 0) {
						    debugMask = 0x4000;	/* next IO */
						    debugStop = 0;
						}
						break;
					    case 'c':
						if (debugFlag != 0) {
						    debugMask = 0x6000;	/* continue till next breakpoint */
						    debugStop = 0;
						}
						break;
					    case 'D':
						if (debugFlag != 0) {
						    debugMask = 0x0000;	/* continue */
						    debugStop = 0;
						    debugFlag = 0;	/* end DEBUG mode */
						}
						break;
					    case 't':
						iC_debug |= 01100;	/* block inactive timers - set -t flag */
						break;
					    case 'S':			/* cannot use 'T' - already used below */
						iC_debug &= ~0100;	/* reset -t flag */
						break;
					    case 'm':
						iC_micro = 1;		/* set micro */
						break;
					    case 'M':
						iC_micro = 0;		/* reset micro */
						break;
					    case 'T':
						iC_send_msg_to_server(iC_sockFN, "T");	/* print iCserver tables */
						break;
					    default:
						goto RcvWarning;	/* unknown D_channel:? case */
					    }
					} else goto RcvWarning;
				    }
				    if (gp->gt_ini == -INPW) {
					if (iC_linked++ == 0) {		/* 1st change linked, further changes not linked again */
#if	YYDEBUG
					    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s[\t", gp->gt_ids);
#if	INT_MAX == 32767 && defined (LONG16)
					    if (iC_debug & 0100) fprintf(iC_outFP, "%hu:%ld\t%ld ==> %ld", channel, gp->gt_new, gp->gt_old, gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
					    if (iC_debug & 0100) fprintf(iC_outFP, "%hu:%d\t%d ==> %d", channel, gp->gt_new, gp->gt_old, gp->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#endif	/* YYDEBUG */
					}
					iC_liveData(gp, gp->gt_new);	/* VCD and/or iClive */
				    }
#ifdef	RASPBERRYPI
				  } else {
				    assert(pqSel);
				    iC_output(val & X_MASK, channel);	/* RQ ExtOut */
				  }
#endif	/* RASPBERRYPI */
				} else {
				    fprintf(iC_errFP, "channel = %hu, topChannel = %hu, gp = %s\n", channel, topChannel, gp ? gp->gt_ids : "null");
				    goto RcvWarning;
				}
//    fprintf(iC_outFP, " EE %s EE", cp); fflush(iC_outFP);
			    } while ((cp = cpe) != NULL);		/* next token if any */
			}
			else {
			  RcvWarning:
			    fprintf(iC_errFP, "WARNING: %s: received '%s' from iCserver ???\n", iC_iccNM, rpyBuf);
			}
		    } else {
			iC_quit(QUIT_SERVER);		/* quit normally with 0 length message from iCserver */
		    }
		}   /*  end of TCP/IP interrupt */
#ifdef	RASPBERRYPI
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) {
		    op = buffer;
		    ol = BS;
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		/********************************************************************
		 *  GPIO25 interrupt means SIO input from a PiFace
		 *******************************************************************/
		if (gpio25FN > 0 && FD_ISSET(gpio25FN, &iC_exfds)) {	/* watch for out-of-band GPIO25 input */
		    int		val;
#if YYDEBUG && !defined(_WINDOWS)
		    int		m1 = 0;
		    int		m;
		    if (iC_debug & 04) fprintf(iC_outFP, "*** SIO interrupt has occurred\n");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    if (iC_micro) iC_microPrint("SPI input received", 0);
		    assert(outPtr == iC_outBuf);	/* was cleared after send loop */
		    do {
			/********************************************************************
			 *  Scan PiFace units for input interrupts (even those not used)
			 *  More interrupts can arrive for PiFace's already scanned, especially
			 *  with bouncing mechanical contacts - repeat the scan until GPIO25 == 1
			 *******************************************************************/
#if YYDEBUG && !defined(_WINDOWS)
			m = 0;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
			    if (readByte(pfp->spiFN, pfp->pfa, pfp->intf)) {	/* interrupt flag on this unit ? */
				cnt += fireInput(
				    pfp,		/* fire input Gates directly and/or via TCP/IP */
				    NULL,
				    readByte(
					pfp->spiFN,
					pfp->pfa,	/* read interrupting PiFace */
					pfp->inpr	/* from B for PiFace, A for PiFaceCAD */
				    )
				);
#if YYDEBUG && !defined(_WINDOWS)
				if (iC_debug & 0100) {
				    len = snprintf(op, ol, " P%d %s", pfp->pfa, pfp->Igate->gt_ids); /* source name */
				    op += len;
				    ol -= len;
				}
				m++;			/* count INTF interrupts found */
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			    }
			}
#if YYDEBUG && !defined(_WINDOWS)
			m1++;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			if ((val = gpio_read(gpio25FN)) == -1) {
			    perror("GPIO25 read");
			    fprintf(iC_errFP, "ERROR: %s: GPIO25 read failed\n", iC_progname);
			    break;
			}
		    } while (val != 1);			/* catch interrupts which came in during for loop */
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 0200) {
			if (m1 > 1){
			    fprintf(iC_errFP, "WARNING: %s: GPIO25 interrupt %d loops %d changes \"%s\"\n",
				iC_progname, m1, m, iC_outBuf+1);
			} else if (m == 0) {	/* for some reason this happens occasionaly - no inputs are missed though */
			    fprintf(iC_errFP, "WARNING: %s: GPIO25 interrupt and no INTF set on PiFaces\n", iC_progname);
			}
		    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		}   /*  end of GPIO25 interrupt */
		/********************************************************************
		 *  GPIO N interrupt means GPIO n input
		 *******************************************************************/
		m1 = 0;
		for (gep = iC_gpL[1]; gep; gep = gep->nextIO) {	/* IXn inputs only */
		    m = 0;
		    val = gep->Gval;					/* old input value */
		    for (bit = 0; bit <= 7; bit++) {
			if ((gpio = gep->gpioNr[bit]) != 0xffff) {
			    if (FD_ISSET(gep->gpioFN[bit], &iC_exfds))	/* any out-of-band GPIO N input */
			    {
				if ((b = gpio_read(gep->gpioFN[bit])) == 0) {
				    val &= ~(1 << bit);
				} else if (b == 1) {
				    val |= 1 << bit;
				} else {
				    fprintf(iC_errFP, "WARNING: %s: GPIO %hu returns invalid value %d (not 0 or 1 !!)\n",
					iC_progname, gpio, b);		/* should not happen */
				}
				m++;
				if (m1++ == 0 && iC_micro) iC_microPrint("GPIO input received", 0);
			    }
			}
		    }
		    if (m && val != gep->Gval) {
			cnt += fireInput(NULL, gep, val);	/* fire input Gates directly and/or via TCP/IP */
#if YYDEBUG && !defined(_WINDOWS)
			if (iC_debug & 0100) {
			    len = snprintf(op, ol, " G %s", gep->Ggate->gt_ids); /* source name */
			    op += len;
			    ol -= len;
			}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    }
		}   /*  end of GPIO N interrupt */
#if YYDEBUG && !defined(_WINDOWS)
		if ((iC_debug & 0100) && outPtr > iC_outBuf) {		/* any data for iCserver? */
		    fprintf(iC_outFP, "\nP: %s:	%s	<%s", iC_iccNM, iC_outBuf+1, buffer);
		    toCntp = &toCnt; slr |= 0x07;	/* activate 350 ms debug message timeout */
		}					/* must output debug message before sendOutput() */
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		sendOutput();				/* send possible TCP/IP data now - may not effect loop scan */
#endif	/* RASPBERRYPI */
		/********************************************************************
		 *  STDIN interrupt
		 *******************************************************************/
		if (FD_ISSET(0, &iC_rdfds)) {
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 04) fprintf(iC_outFP, "*** stdin interrupt has occurred\n");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    if (fgets(iC_stdinBuf, REPLY, stdin) == NULL) {	/* get input before TX0.1 notification */
			FD_CLR(0, &infds);		/* ignore EOF - happens in bg or file - turn off interrupts */
			iC_stdinBuf[0] = '\0';		/* notify EOF to iC application by zero length iC_stdinBuf */
		    }
		    if ((gp = tim[1]) != NULL) {	/* TX0.1 is used to notify receiving a new line or EOF on STDIN */
			assert(stdinFlag == 0 && gp->gt_val == 1);	/* TX0.1 lo */
#if	YYDEBUG
			if (iC_debug & 0100) fprintf(iC_outFP, "*** %s\n%s %+d ^=>",
			    iC_stdinBuf, gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
			gp->gt_val = -1;		/* set  TX0.1 hi */
			iC_link_ol(gp, iC_o_list);	/* fire TX0.1 hi to notify that a new line is available in iC_stdinBuf */
#if	YYDEBUG
			if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
#endif	/* YYDEBUG */
			iC_liveData(gp, 1);		/* VCD and/or iClive */
			cnt++;
			stdinFlag = 1;			/* arm cycle for turning TX0.1 lo */
		    } else if ((c = iC_stdinBuf[0]) == 'q' || c == '\0') {
			iC_quit(QUIT_TERMINAL);		/* quit normally with 'q' or ctrl+D */
#if	YYDEBUG
		    } else if (c == 't') {
			iC_debug |= 01000;		/* block inactive timers */
			iC_debug ^= 0100;		/* toggle -t flag */
		    } else if (c == 'm') {
			iC_micro++;			/* toggle more micro */
			if (iC_micro >= 3) iC_micro = 0;
		    } else if (c == 'T') {
			iC_send_msg_to_server(iC_sockFN, "T");	/* print iCserver tables */
		    } else if (c != '\n') {
			fprintf(iC_errFP, "no action coded for '%c' - try t, m, T, or q followed by ENTER\n", c);
#endif	/* YYDEBUG */
		    }					/* ignore the rest of STDIN */
		}   /*  end of STDIN interrupt */
	    } else {				/* retval -1 */
		perror("ERROR: select failed");
		iC_quit(SIGUSR1);
	    }
	    /* if many inputs change simultaneously increase oscillator limit */
	    iC_osc_lim = (cnt << 1) + 1;	/* (cnt * 2) + 1 */
	    if (iC_osc_lim < iC_osc_max) {
		iC_osc_lim = iC_osc_max;	/* cnt = 1, osc_lim = 4 default */
	    }
	}
    } /* for (;;) */
} /* iC_icc */

/********************************************************************
 *
 *	Output data directly to either PiFace or PiFaceCAD or GPIO
 *	and/or as a message to iCserver
 *	pqSel	pfp	gep
 *	  0	NULL	NULL	// External output to iCserver only
 *	  1	0xcc	NULL	// PiFace output
 *	  2	NULL	0xcc	// GPIO output
 *
 *******************************************************************/

void
#if	INT_MAX == 32767 && defined (LONG16)
iC_output(long val, unsigned short channel)
#else	/* INT_MAX == 32767 && defined (LONG16) */
iC_output(int val, unsigned short channel)
#endif	/* INT_MAX == 32767 && defined (LONG16) */
{
    int			len = 0;
#ifdef	RASPBERRYPI
    piFaceIO*		pfp = NULL;
    gpioIO*		gep = NULL;
    channelSel *	channelSp;
    int			pqSel;

    channelSp = &Channels[channel];
    if ((pqSel = channelSp->pqs) == 1) {
	/********************************************************************
	 *  direct PiFace output
	 *******************************************************************/
	pfp = channelSp->p;
	assert(pfp && pfp->Qgate && pfp->Qgate->gt_ids && *pfp->Qgate->gt_ids == 'Q');	/* make sure this is really a PiFace output */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\nP: %s:	%hu:%d P%d	> %s",
		iC_iccNM, channel, (int)val, pfp->pfa, pfp->Qgate->gt_ids);
	    toCntp = &toCnt; slr |= 0x07;		/* activate 350 ms debug message timeout */
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	writePiFace(pfp, val & X_MASK, channelSp->iqs);	/* Q Internal */
    } else if (pqSel == 2) {
	/********************************************************************
	 *  direct GPIO output
	 *******************************************************************/
	int			val0;
	int			diff;
	int			mask;
	int			fd;
	unsigned short	bit;
	gep = channelSp->q;
	assert(gep && gep->Ggate && gep->Ggate->gt_ids && *gep->Ggate->gt_ids == 'Q');	/* make sure this is really a GPIO output */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\nP: %s:	%hu:%d G	> %s",
		iC_iccNM, channel, (int)val, gep->Ggate->gt_ids);
	    toCntp = &toCnt; slr |= 0x07;		/* activate 350 ms debug message timeout */
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	val0 = val ^ gep->Ginv; /* normally write non-inverted data to GPIO output - invert data for -I */
	diff = val0 ^ gep->Gval;				/* bits which are going to change */
	if (iC_debug & 04) fprintf(iC_outFP, "*** val0 = 0x%02x	Gval = 0x%02x	diff = 0x%02x	Ginv = 0x%02x\n",
	    (int)val0, gep->Gval, diff, gep->Ginv);
	assert ((diff & ~0xff) == 0);			/* may receive a message which involves no change */
	while (diff) {
	    bit = iC_bitIndex[diff];			/* returns 0 - 7 for values 1 - 255 (avoid 0) */
	    mask  = iC_bitMask[bit];			/* returns hex 01 02 04 08 10 20 40 80 */
	    if ((fd = gep->gpioFN[bit]) != -1) {		/* is GPIO pin open ? */
		gpio_write(fd, val0 & mask);		/* yes - write to GPIO (does not need to be a bit) */
	    } else if ((iC_debug & 0300) == 0300) {
		fprintf(iC_errFP, "WARNING: %s: no GPIO associated with %s.%hu\n",
		    iC_progname, gep->Ggate->gt_ids, bit);
	    }
	    diff &= ~mask;				/* clear the bit just processed */
	    if (iC_debug & 04) fprintf(iC_outFP, "***   bit = %hu	mask = 0x%02x	val0&mask = 0x%02x	diff = 0x%02x\n",
		bit, mask, val0&mask, diff);
	}
	gep->Gval = val0;				/* ready for next output */
    }
  if (channelSp->g && (pqSel == 0 || iC_opt_E)) { /* SQ External (default) or Q Internal && opt_E */
#endif	/* RASPBERRYPI */
    /********************************************************************
     *  TCP/IP output
     *******************************************************************/
    len = snprintf(outPtr, outBufLen,
#if	INT_MAX == 32767 && defined (LONG16)
	",%hu:%ld",					/* signed long to output for QXx QBx QWx QLx */
#else	/* INT_MAX == 32767 && defined (LONG16) */
	",%hu:%d",					/* signed int to output for QXx QBx QWx QLx */
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	channel, val);
    if (len) {
	outPtr += len;
	outBufLen -= len;
	if (outBufLen < OSIZ) {				/* holds 65535:-1879048193\0 MAX 18 characters */
	    assert(outBufLen >= 0);			/* only fails if this len > OSIZ */
	    sendOutput();				/* Send output data before it overflows */
	}
    }
#ifdef	RASPBERRYPI
  }
#endif	/* RASPBERRYPI */
} /* iC_output */

/********************************************************************
 *
 *	Send output data collected in iC_outBuf to iCserver
 *
 *	Send TCP/IP output data collected in iC_outBuf to iCserver.
 *	Can send more than one block sequentially to iCserver.
 *	The output data will be distributed correctly.  If there is more than
 *	1 block, early blocks are sent directly in iC_output(), which executes
 *	in iC_scan_snd(iC_s_list) directly before the break in the Operational
 *	loop just before this point.
 *
 *	More than one block is very unlikely except for deliberate strange
 *	output strategies like modifying more than 250 OUTW Gates by the
 *	same input:
 *	QX0.0 = ~IX0.0; QX1.0 = ~IX0.0; QX2.0 = ~IX0.0; ... QX299.0 = ~IX0.0;
 *	Because of defensive programming in iC_output() this now works.
 *
 *******************************************************************/

static void
sendOutput(void)
{
    if (outPtr > iC_outBuf) {				/* any data for iCserver? */
	assert(iC_outBuf[0] == ',');			/* skip first ',' */
#ifdef	RASPBERRYPI
	if (iC_sockFN > 0)				/* may be called when no iCserver for direct output */
#endif	/* RASPBERRYPI */
	iC_send_msg_to_server(iC_sockFN, iC_outBuf+1);	/* send block - usually the only one */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 04) {
	    fprintf(iC_outFP, "iC_outBuf[%d] \"%s\"\n", (int)(outPtr - iC_outBuf-1), iC_outBuf+1);
	    *(iC_outBuf+1) = '\0';			/* clean debug output next time */
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	outPtr = iC_outBuf;				/* ready for next Operational loop */
	outBufLen = REQUEST;
    }
} /* sendOutput */
#ifdef	RASPBERRYPI

/********************************************************************
 *
 *	Output values from SIO or GPIO input interrupts or initialisation
 *	directly by firing input Gates as was well as a possible message
 *	via iCserver if opt_E or ExtOut
 *
 *******************************************************************/

static int
fireInput(piFaceIO * pfp, gpioIO * gep, int val)
{
    Gate *		gp;
    int			cnt = 0;
    int			len;
    unsigned short	channel = 0;		/* stop warning */
    int			invMask = 0;		/* stop warning */
    int			valOld = 0;		/* stop warning */
    char		pgBuf[4];		/* store " P0" to " P7" " D" or " G" */

    if (pfp) {
	if (pfp->Igate) {
	    assert(pfp->Igate->gt_ids);
	    channel = pfp->Ichannel;
	    assert(channel && channel < ioChannels);	/* should not happen now with early test for pfp->Igate */
	    assert(pfp == Channels[channel].p);		/* ExtOut or Intern with correct pfp */
	    val &= pfp->Ibmask;			/* mask out any bits on Port B which are output */
	    invMask = pfp->Iinv;		/* any masked bits are never inverted */
	    valOld = pfp->Ival;
	    pfp->Ival = val;			/* store change for comparison */
	    snprintf(pgBuf, 4, " P%d", pfp->pfa);
	} else if (iC_debug & 0300) {
	    snprintf(pgBuf, 4, " D");
	    if (iC_debug & 0100) fprintf(iC_outFP, "\n");
	    fprintf(iC_outFP, "Dummy PiFace %d:%hu input %s\n",
		pfp - iC_pfL, pfp->pfa, convert2binary(val, 1));
	}
    } else if (gep) {
	assert(gep->Ggate);
	channel = gep->Gchannel;
	assert(channel && channel < ioChannels);	/* should not happen now with early assert */
	assert(gep == Channels[channel].q);		/* ExtOut or Intern with correct gep */
	invMask = gep->Ginv;			/* any unused GPIOs are never inverted */
	valOld = gep->Gval;
	gep->Gval = val;			/* store change for comparison */
	snprintf(pgBuf, 4, " G");
    } else {
	assert(0);				/* wrong call = must be either pfp or gep */
    }
    if (val != valOld) {
	val ^= invMask;				/* by default do not invert PiFace and GPIO inputs - they are inverted with -I */
	if ((gp = Channels[channel].g) != NULL) {	/* I Internal */
	    assert(gp->gt_fni == TRAB);
	    gp->gt_new = val;
#if	YYDEBUG
#if	INT_MAX == 32767 && defined (LONG16)
	    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s<\t%hu:%ld%s\t0x%02lx ==>> 0x%02lx",
		gp->gt_ids, channel, gp->gt_new, pgBuf, gp->gt_old, gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
	    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s<\t%hu:%d%s\t0x%02x ==>> 0x%02x",
		gp->gt_ids, channel, gp->gt_new, pgBuf, gp->gt_old, gp->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	    else if (iC_debug & 020) fprintf(iC_outFP, "\n%s\t%2hu:%s%s",
		gp->gt_ids, channel, convert2binary(gp->gt_new, 1), pgBuf);
#endif	/* YYDEBUG */
	    cnt = iC_traMb(gp, 0);		/* distribute bits directly */
	}
	if (gp == NULL || iC_opt_E) {		/* SI ExtOut or Internal && opt_E */
	    assert(outBufLen > 11);		/* fits largest data telegram */
	    len = snprintf(outPtr, outBufLen, ",%hu:%d", channel, val);
	    if (len) {
		outPtr += len;
		outBufLen -= len;
		assert(outBufLen >= 0);		/* unlikely to fail with PiFace I/O only */
	    }
	}
    }
    return cnt;
} /* fireInput */

/********************************************************************
 *
 *	Write bit data as a single byte to PiFace or PiFaceCAD or GPIO
 *
 *******************************************************************/

static void
writePiFace(piFaceIO * pfp, int val, int iq)
{
    iqDetails *	pfq;
    int		pfa;
    int		m;
    int		v18;

    if ((pfa = pfp->pfa) != 4 || pfp->intf != INTFA) {	/* PiFace */
	/********************************************************************
	 *  Direct output to a PiFace
	 *******************************************************************/
	pfq = &(pfp->s[iq]);
	writeByte(pfp->spiFN, pfa,
		    iq ? OLATB : OLATA,
		    (val & pfq->bmask) ^ pfq->inv
		 );	/* normally write inverted data to PiFace A output */
    } else if ((m = val ^ pfp->Qval) != 0) {		/* PiFaceCAD */
	if (m & 0x07) {
	    /* TODO */
	}
	if (m & 0x18) {					/* change shift left or right */
	    slr &= ~0x18;				/* clear old shift value */
	    if ((v18 = val & 0x18) != 0) {
		slr |= v18;				/* set shift left 0x08 or shift right 0x10 */
		tcnt_750 = 1;				/* first character shift immediately */
		toCntp = &toCnt;			/* start 750 ms timeout */
	    } else if (slr == 0) {			/* slr also holds debug message timeout counter */
		toCntp = toCoff;			/* stop 750 ms timeout unless iC TX0 uses it */
	    }						/* for both off */
	}
	if ((m & 0x20) && (val & 0x20)) {
	    switch (cbc = (cbc + 1) & 0x03) {		/* cbc holds cursor and blink state */
	    case 0x01:
		pifacecad_lcd_cursor_on();
		pifacecad_lcd_blink_off();
		break;
	    case 0x02:
		pifacecad_lcd_cursor_off();
		pifacecad_lcd_blink_on();
		break;
	    case 0x03:
		pifacecad_lcd_cursor_on();
		pifacecad_lcd_blink_on();
		break;
	    case 0x00:
	    default:
		pifacecad_lcd_cursor_off();
		pifacecad_lcd_blink_off();		/* default is both off */
		break;
	    }
	}
	if (m & 0x40) {
	    if (val & 0x40) {
		pifacecad_lcd_display_off();
	    } else {
		pifacecad_lcd_display_on();		/* default is on */
	    }
	}
	if (m & 0x80) {
	    pifacecad_lcd_set_backlight(!(val & 0x80));	/* default is on */
	}
	pfp->Qval = val;		/* store change for comparison */
    }
} /* writePiFace */
#endif	/* RASPBERRYPI */

/********************************************************************
 *
 *	Connect and send registration and receive acknowledgement.
 *	If RASPBERRYPI this is only done if there is actually some
 ^	TCP/IP to register - there might be only direct I/O
 *
 *******************************************************************/

static void
regAck(Gate ** oStart, Gate ** oEnd)
{
    Gate **		opp;
    Gate *		gp;
    char *		cp;
    char *		ip;
    int			len;
    int			bc;
#ifdef	RASPBERRYPI
    piFaceIO *		pfp;
    gpioIO *		gep;
    int			iq;
    unsigned short	marking;
    unsigned short	pgFlag;
#endif	/* RASPBERRYPI */
    unsigned short	channel;
    if (iC_sockFN <= 0) {
	/********************************************************************
	 *  Start TCP/IP communication before any inputs are generated => outputs
	 *******************************************************************/
	if (iC_debug & 04) fprintf(iC_outFP, "*** before iC_connect_to_server\n");
	iC_sockFN = iC_connect_to_server(iC_hostNM, iC_portNM);
	if (iC_debug & 04) fprintf(iC_outFP, "*** after  iC_connect_to_server\n");
	if (iC_sockFN > iC_maxFN) {
	    iC_maxFN = iC_sockFN;
	}
	/********************************************************************
	 *  Set all bits to wait for TCP/IP interrupts here
	 *******************************************************************/
	FD_SET(iC_sockFN, &infds);		/* watch sock for inputs in normal wait */
	FD_SET(iC_sockFN, &idfds);		/* watch sock for inputs in debugWait() */
	/********************************************************************
	 *  Send optional equivalence strings from the command line to iCserver
	 *  to extend equivalences before registration proper. This is only
	 *  done once in the first call to this function regAck() before
	 *  iC_socFN was set by iC_connect_to_server().
	 *
	 *  The equivalence string from the command line can be a single IEC
	 *  name eg -eIX0-1 or simply -eIX0 or a comma separated list of
	 *  IEC names -eIX0-1,IX1-1 or -eIX0,IX1 or the special equivalence
	 *  command -eI which equivalences all IEC inputs which are going to
	 *  be registered by the app.
	 *
	 *  Plain IEC strings eg IX0 are equivalenced to IEC=IEC-<instance> from
	 *  the command line option -i<instance>, if that is set eg:
	 *      -eIX0,IX1 -i1 produces 'IX0=IX0-1 IX1=IX1-1'
	 *  If instance is not set the IEC string remains unchanged.
	 *
	 *  The total input equivalence list is limited to 1399 bytes before
	 *  expansion (200 - 250 plain equivalences), which should be plenty.
	 *
	 *  If the expanded equivalence list exceeds 1399 bytes, the list is
	 *  sent to iCserver in sections.
	 *******************************************************************/
	if (rpyBuf[0] == ',') {
	    if (strcmp(rpyBuf, ",I") != 0) {
		strncpy(msgBuf, rpyBuf, REQUEST);	/* not  -I use command line -e options in rpyBuf */
	    }						/* else -I use all IEC inputs collected in msgBuf */
	    strcpy(rpyBuf, "E");			/* 'E' is equivalence command for iCserver */
	    cp = rpyBuf + 1;				/* bypass initial 'E' */
	    bc = REPLY  - 1;
	    for (ip = strtok(msgBuf, ", "); ip; ip = strtok(NULL, ", ")) { /* delimit with ',' or ' ' */
		if (strcmp(ip, "I") == 0) {
		    fprintf(iC_errFP, "WARNING: %s: '-e I' ignored in favour of other -e arguments\n",
			iC_progname);
		    continue;
		}
		do {
		    if (bc <= 0) {
			*cp = '\0';			/* buffer oveflowed - remove last entry */
#if YYDEBUG && !defined(_WINDOWS)
			if (iC_debug & 0224) fprintf(iC_outFP, "equivalence:%s\n", rpyBuf);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			iC_send_msg_to_server(iC_sockFN, rpyBuf); /* send partial equivalence string */
			cp = rpyBuf + 1;		/* bypass initial 'E' */
			bc = REPLY  - 1;
			assert(len < bc);		/* bombs out if single entry len >= REPLY-1 */
		    }					/* else infinite loop */
		    if (strchr(ip, '=') || strlen(iC_iidNM) == 0) {
			len = snprintf(cp, bc, " %s", ip);
			/* do not modify if equivalence contains '=' or no instance was set */
		    } else {
			len = snprintf(cp, bc, " %s=%s-%s", ip, ip, iC_iidNM);	/* append instance */
		    }
		    bc -= len;
		} while (bc <= 0);
		cp += len;				/* room for this entry and '\0' terminator */
	    }
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0224) fprintf(iC_outFP, "equivalence:%s\n", rpyBuf);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    iC_send_msg_to_server(iC_sockFN, rpyBuf);	/* send final equivalence string */
	}
    }
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 04) fprintf(iC_outFP, "*** TCP interrupt has been primed\n");
#endif	/* YYDEBUG && !defined(_WINDOWS) */

#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0224) fprintf(iC_outFP, "register:%s\n", regBuf);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    iC_send_msg_to_server(iC_sockFN, regBuf);	/* register controller and IOs */
    if (iC_rcvd_msg_from_server(iC_sockFN, rpyBuf, REPLY) != 0) {	/* busy wait for acknowledgment reply */
	if (iC_micro & 06) iC_microPrint("reply from server", 0);
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0224) fprintf(iC_outFP, "reply:%s\n", rpyBuf);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	cp = rpyBuf - 1;	/* increment to first character in rpyBuf in first use of cp */
	if (Channels == NULL) {
	    // C channel - messages from controller to debugger
	    C_channel = atoi(++cp);
	    assert(C_channel > 0);
	    if (C_channel > topChannel) {
		topChannel = C_channel;
	    }
	    cp = strchr(cp, ',');
	    // D channel - messages from debugger to controller
	    assert(cp);
	    D_channel = atoi(++cp);
	    assert(D_channel > 0);
	    if (D_channel > topChannel) {
		topChannel = D_channel;
	    }
	    storeChannel(D_channel, &D_gate	/* ==> Debug input */
#ifdef	RASPBERRYPI
					, NULL, NULL, 0
#endif	/* RASPBERRYPI */
	    );
	    cp = strchr(cp, ',');
#ifdef	RASPBERRYPI
	    if (pfCADpfp) {
		// P channel - receivee string messages for PiFaceCAD via PFCAD4
		assert(cp);
		channel = atoi(++cp);
		assert(channel > 0);
		if (channel > topChannel) {
		    topChannel = channel;
		}
		storeChannel(channel, &pfCADgate, pfCADpfp, NULL, 0);
		pfCADchannel = channel;
		cp = strchr(cp, ',');
	    }
#endif	/* RASPBERRYPI */
	    /********************************************************************
	     *  Check if app wants to send strings to PiFaceCAD (can be non RASPBERRTPI)
	     *	P_channel	0xfffe	this app wants to write to PiFaceCAD
	     *******************************************************************/
	    if (P_channel == 0xfffe) {
		// P channel - send string messages for PiFaceCAD via PFCAD4
		assert(cp);
		channel = atoi(++cp);
		assert(channel > 0);
		if (channel > topChannel) {
		    topChannel = channel;
		}
		storeChannel(channel, &pfCADgate
#ifdef	RASPBERRYPI
						, NULL, NULL, 0
#endif	/* RASPBERRYPI */
		);
		P_channel = channel;
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0300) fprintf(iC_outFP, "	 %s			%3hu\n",
		    pfCADgate.gt_ids, P_channel);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		cp = strchr(cp, ',');
	    }
	}
	// remaining input and output channel indices matching registrations
	for (opp = oStart; opp < oEnd; opp++) {		/* sTable to sTend */
	    gp = *opp;
#ifdef	RASPBERRYPI
	  if ((marking = gp->gt_live) != Dummy) {	/* extra I/O Dummy arguments are not registered */
#endif	/* RASPBERRYPI */
	    if ((gp->gt_ini == -INPW && *gp->gt_ids != 'T') ||	/* suppress TX0 */
		gp->gt_fni == OUTW) {
#ifdef	RASPBERRYPI
		if (!(gp->gt_mark & X_FLAG)) {			/* do Internal/ExtOut later */
		    assert(cp);
		    channel = atoi(++cp);
		    assert(channel > 0);
		    if (channel > topChannel) {
			topChannel = channel;
		    }
		    if (marking == External) {			/* transfer markings */
			storeChannel(channel, gp, NULL, NULL, 0);
		    } else if ((pgFlag = gp->gt_mark & PG_MASK) == P_FLAG) {
			pfp = gp->gt_pfp;			/* stored temporarily in load.c */
			assert(pfp);				/* over written by gt_channel */
			pfp->s[iq = gp->gt_ini == -INPW].channel = channel;	/* receive or send input */
			if (marking == Internal) {
			    storeChannel(channel, gp, pfp, NULL, iq);
			} else if (marking == ExtOut) {
			    storeChannel(channel, NULL, pfp, NULL, iq);
			}
		    } else if (pgFlag == PB_FLAG) {
			pfp = gp->gt_pfp;			/* stored temporarily in load.c */
			assert(pfp);				/* over written by gt_channel */
			assert(gp->gt_ini != -INPW);		/* Relay+ Port B output only */
			pfp->s[2].channel = channel;		/* send input */
			if (marking == Internal) {
			    storeChannel(channel, gp, pfp, NULL, 2);
			} else if (marking == ExtOut) {
			    storeChannel(channel, NULL, pfp, NULL, 2);
			}
		    } else if (pgFlag == G_FLAG) {
			gep = gp->gt_gep;			/* stored temporarily in load.c */
			assert(gep);				/* over written by gt_channel */
			gep->Gchannel = channel;		/* receive or send input */
			if (marking == Internal) {
			    storeChannel(channel, gp, NULL, gep, 0);
			} else if (marking == ExtOut) {
			    storeChannel(channel, NULL, NULL, gep, 0);
			}
		    } else {
			assert(0);	/* Internal or ExtOut must have P_FLAG, PB_FLAG or G_FLAG */
		    }
		    if (gp->gt_ini != -INPW && gp->gt_fni == OUTW) {
			gp->gt_channel = channel;		/* <== Output */
#if YYDEBUG && !defined(_WINDOWS)
			if (iC_debug & 0224) fprintf(iC_outFP, "store output: channel:  %s <== %hu\n",
			    gp->gt_ids, channel);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    }
		    cp = strchr(cp, ',');
		}
#else	/* ! RASPBERRYPI */
		assert(cp);
		channel = atoi(++cp);
		assert(channel > 0);
		if (channel > topChannel) {
		    topChannel = channel;
		}
		if (gp->gt_ini == -INPW) {
		    storeChannel(channel, gp);			/* ==> Input */
		} else
		if (gp->gt_fni == OUTW) {
		    gp->gt_channel = channel;			/* <== Output */
		}
		cp = strchr(cp, ',');
#endif	/* ! RASPBERRYPI */
	    }
#ifdef	RASPBERRYPI
	  }
#endif	/* RASPBERRYPI */
	}
	assert(cp == NULL);			/* Ack string matches Registration */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0224) fprintf(iC_outFP, "reply: Channels %hu\n", topChannel);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    } else {
	iC_quit(QUIT_SERVER);			/* quit normally */
    }
} /* regAck */

/********************************************************************
 *
 *	Initalise and expand dynamic array Channels[] as necessary
 *	Store input Gate address in Channels[]
 *	For RASPBERRYPI store piFaceIO/gpioIO pointer in Channels[]
 *	as well as pqs to select 0 External 1 piFaceIO*, 2 gpioIO*
 *	and iqs to select Port A output, Port B/A input or Port B output
 *
 *******************************************************************/

static void
storeChannel(unsigned short channel, Gate * gp
#ifdef	RASPBERRYPI
	    , piFaceIO * pfp, gpioIO * gep, int iq
#endif	/* RASPBERRYPI */
	)
{
#ifdef	RASPBERRYPI
    channelSel *	channelSp;
#endif	/* ! RASPBERRYPI */
    while (channel >= ioChannels) {
#ifdef	RASPBERRYPI
	Channels = (channelSel *)realloc(Channels,	/* initially NULL */
	    (ioChannels + IOCHANNELS) * sizeof(channelSel));
	assert(Channels);
	memset(&Channels[ioChannels], '\0', IOCHANNELS * sizeof(channelSel));
#else	/* ! RASPBERRYPI */
	Channels = (Gate **)realloc(Channels,	/* initially NULL */
	    (ioChannels + IOCHANNELS) * sizeof(Gate *));
	assert(Channels);
	memset(&Channels[ioChannels], '\0', IOCHANNELS * sizeof(Gate *));
#endif	/* ! RASPBERRYPI */
	ioChannels += IOCHANNELS;	/* increase the size of the array */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 04) fprintf(iC_outFP, "storeChannel: Channels[%d] increase\n", ioChannels);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    }
#ifdef	RASPBERRYPI
    channelSp = &Channels[channel];
    channelSp->g = gp;		/* store Gate pointer */
    channelSp->pqs = 0;		/* assume no direct I/O - External only */
    channelSp->p = pfp;		/* store piFace pointer or NULL if External */
    channelSp->iqs = iq;	/* PiFace Port A or Port B I/O selection */
    if (pfp) {
	channelSp->pqs = 1;	/* PiFace */
    } else if (gep) {
	channelSp->pqs = 2;	/* GPIO */
	channelSp->q = gep;	/* store gpioIO pointer */
    }
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 24) {
	if (pfp) {
	    fprintf(iC_outFP, "storeChannel: Channels[%d].g <== %s	.p <== %s,%s\n",
	    channel,
	    gp ?  gp->gt_ids : NULL,
	    pfp ? (pfp->Igate ? pfp->Igate->gt_ids : NULL) : NULL,
	    pfp ? (pfp->Qgate ? pfp->Qgate->gt_ids : NULL) : NULL);
	} else if (gep) {
	    fprintf(iC_outFP, "storeChannel: Channels[%d].g <== %s	.q <== %s\n",
	    channel,
	    gp ?  gp->gt_ids : NULL,
	    gep ? (gep->Ggate ? gep->Ggate->gt_ids : NULL) : NULL);
	}
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#else	/* ! RASPBERRYPI */
    Channels[channel] = gp;		/* store input Gate pointer */
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 04) fprintf(iC_outFP, "storeChannel: Channels[%d] <== %s\n", channel, gp->gt_ids);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#endif	/* ! RASPBERRYPI */
} /* storeChannel */

/********************************************************************
 *
 *	Wait for TCP events only in DEBUG mode
 *
 *  iClive DEBUG system messages		Keybord Accelerator	at app
 *
 *  8:	activate DEBUG mode				'd'	set debugFlag
 *		un-grey INTERRUPT/STEP/NEXT/CONTINUE			send symbol as it changes
 *		turn search entry window pink				(app is executing)
 *		make all break point symbols break symbols		stops on next break point (if any)
 *
 *  9:	STEP						's'	start execution
 *		make all active symbols break symbols			stops on next break symbol
 *  10:	NEXT						'n'	start execution
 *		make only output symbols break symbols			stops after next output
 *  11:	CONTINUE					'c'	start execution
 *		make only break point symbols break symbols		stops on next break point (if any)
 *
 *  12:	de-activate DEBUG mode				'D'	clear debugFlag
 *		grey INTERRUPT/STEP/NEXT/CONTINUE			buffer changed symbols
 *		turn search entry window clear				start execution
 *		clear all break symbols (effectively de-activates break points)
 *
 *	BREAK execution when changed symbol received		stop execution
 *		if DEBUG mode and current symbol is a break symbol
 *		show break symbol in pink search entry window
 *		start search for the active symbol (moves text)
 *
 *  The following are independent of DEBUG mode
 *
 *  13:	activate/de-activate debug output in app	't'	toggle -t flag
 *
 *  14:	step 3 state microsecond output in app		'm'	toggle more micro
 *
 *******************************************************************/

static void
debugWait(void)
{
    char *		cp;
    int			val;
    int			retval;
    unsigned short	channel;
    char		debugBuf[REPLY];	/* needs local buffer */

    /********************************************************************
     *  For debug messages wait for TCP inputs only
     *  Ignore stdin, extra and timer inerrupts
     *******************************************************************/
    retval = iC_wait_for_next_event(&idfds, 0, 0);
    if (retval > 0) {
	/********************************************************************
	 *  TCP/IP input from iCserver
	 *******************************************************************/
	if (iC_sockFN > 0 && FD_ISSET(iC_sockFN, &iC_rdfds)) {
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 04) fprintf(iC_outFP, "*DW TCP interrupt");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    if (iC_rcvd_msg_from_server(iC_sockFN, debugBuf, REPLY) != 0) {
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 04) { fprintf(iC_outFP, " << %s\n", debugBuf); fflush(iC_outFP); }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		cp = debugBuf - 1;	/* increment to first character in debugBuf in first use of cp */
		if (isdigit(debugBuf[0])) {
		    char *	cpe;
		    char *	cps;

		    do {
			if ((cpe = strchr(++cp, ',')) != NULL) { /* find next comma in input */
			    *cpe = '\0';	/* split off leading comma separated token */
			}
			channel = (unsigned short)atoi(cp);
			if (channel == D_channel) {
			    if ((cps = strchr(cp, ':')) == NULL) goto RcvWarning;
			    val = atoi(++cps);
			    switch (val) {
			    case 0:			/* IGNORE */
			    case 5:			/* GET_END */
				break;
			    case 1:			/* GET_SYMBOL_TABLE */
			    case 2:			/* iClive poll */
			    case 7:			/* DEBUGGER STOPPED */
				goto Retransmit;	/* handle in main loop */
			    case 3:			/* RECEIVE_ACTIVE_SYMBOLS */
			    case 4:			/* LAST_ACTIVE_SYMBOLS */
				debugMask = 0x0000;
				receiveActiveSymbols(debugBuf);
				if (val == 4) {
				    liveFlag = 1;	/* live inhibit bits are correct */
				}
				break;
			    case 6:			/* STOP PROGRAM */
				iC_quit(QUIT_DEBUGGER);	/* quit normally */
				break;
			    case 12:			/* RECEIVE_WATCH_OR_RESTORE */
				receiveWatchOrRestore(debugBuf);
				break;
				/********************************************************************
				 *  DEBUG commands
				 *******************************************************************/
			    case 's':
				if (debugFlag != 0) {
				    debugMask = 0x2000;	/* single step */
				    debugStop = 0;
				}
			    case 'd':			/* already in debug mode - ignore */
				break;
			    case 'n':
				if (debugFlag != 0) {
				    debugMask = 0x4000;	/* next IO */
				    debugStop = 0;
				}
				break;
			    case 'c':
				if (debugFlag != 0) {
				    debugMask = 0x6000;	/* continue till next breakpoint */
				    debugStop = 0;
				}
				break;
			    case 'D':
				if (debugFlag != 0) {
				    debugMask = 0x0000;	/* continue */
				    debugStop = 0;
				    debugFlag = 0;	/* end DEBUG mode */
				}
				break;
			    case 't':
				iC_debug |= 01100;	/* block inactive timers - set -t flag */
				break;
			    case 'S':			/* cannot use 'T' - already used below */
				iC_debug &= ~0100;	/* reset -t flag */
				break;
			    case 'm':
				iC_micro = 1;		/* set micro */
				break;
			    case 'M':
				iC_micro = 0;		/* reset micro */
				break;
			    case 'T':
				iC_send_msg_to_server(iC_sockFN, "T");	/* print iCserver tables */
				break;
			    default:
				goto RcvWarning;	/* unknown D_channel:? case */
			    }
			} else {
			    /********************************************************************
			     *  Any I.. inputs and other control messages are collected while
			     *  stopped at a break and transmitted to iCserver just before waiting
			     *  for new input in the main loop. This means these messages will be
			     *  received as the next input in the main loop to be processed by
			     *  the next iC scan. This scheme has the advantage, that there is
			     *  a minimum of interference with the main loop due to debugging.
			     *  Timer TX0.x and STDIN interrupts are ignored completely while at
			     *  a break and also act when waiting for new input in the main loop.
			     *  The same holds for GPIO interrupts from a RASPBERRYPI.
			     *******************************************************************/
			  Retransmit:
			    if (regOffset == 0) {
				regOffset = snprintf(regBuf, REQUEST, "%s", cp);
			    } else {
				regOffset += snprintf(&regBuf[regOffset], REQUEST - regOffset, ",%s", cp);
			    }
//    fprintf(iC_outFP, " $$ %s $$", cp); fflush(iC_outFP);
			}
		    } while ((cp = cpe) != NULL);		/* next token if any */
//    fprintf(iC_outFP, " @"); fflush(iC_outFP);
		}
		else {
		  RcvWarning:
		    fprintf(iC_errFP, "WARNING: %s: received '%s' in debugWait from iCserver ???\n", iC_iccNM, debugBuf);
		}
	    } else {
		iC_quit(QUIT_SERVER);		/* quit normally with 0 length message from iCserver */
	    }
	}   /*  end of TCP/IP interrupt */
    } else if (retval == 0) {
	fprintf(iC_errFP, "ERROR: select in debugWait should not interrupt from a timer\n");
	iC_quit(SIGUSR1);
    } else {				/* retval -1 */
	perror("ERROR: select in debugWait failed");
	iC_quit(SIGUSR1);
    }
} /* debugWait */

/********************************************************************
 *
 *	Output VCD data and/or a live data message during scans
 *	Handle live data and breaks in execution in DEBUG mode
 *
 *	debugMask	set by iClive in CONT message, cleared in DEBUG message
 *	debugStop	cleared by iClive in CONT message
 *
 *******************************************************************/

void
#if	INT_MAX == 32767 && defined (LONG16)
iC_liveData(Gate * gp, long value)
#else	/* INT_MAX == 32767 && defined (LONG16) */
iC_liveData(Gate * gp, int value)
#endif	/* INT_MAX == 32767 && defined (LONG16) */
{
    int			len;
    int			rest;
    char *		code;
    unsigned short	index;
    unsigned short	brk;
    char		tempBuf[TSIZ];

//    fprintf(iC_outFP, " ## %s %d (%d) ##", gp->gt_ids, value, iC_linked); fflush(iC_outFP);
    if (iC_linked) {
	index = gp->gt_live & 0x1fff;
	if (iC_vcdFP && (code = vcd_ftype[gp->gt_fni]) != NULL) {
	    fprintf(iC_vcdFP, "#%ld\n", ++virtualTime);
	    switch (*code) {
	    case 'e':					/* event */
	    case 'w':					/* wire */
#if	INT_MAX == 32767 && defined (LONG16)
		fprintf(iC_vcdFP, "%1.1ld%hu\n", value, index);
#else	/* INT_MAX == 32767 && defined (LONG16) */
		fprintf(iC_vcdFP, "%1.1d%hu\n", value, index);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		break;
	    case 'i':					/* integer */
		fprintf(iC_vcdFP, "b%s %hu\n",
		    convert2binary(value, 0),		/* convert to 32 bit binary string */
		    index);
		break;
	    default:					/* hard error */
		assert(0);
		break;
	    }
	}
	if (debugMask == 0x0000) {
	    /********************************************************************
	     *  Handle live data in NORMAL mode at maximum speed
	     *******************************************************************/
	  noBreak:
	    if (gp->gt_live & 0x8000) {			/* is variable in window for iClive */
		while ((len =
#if	INT_MAX == 32767 && defined (LONG16)
			snprintf(&msgBuf[msgOffset], rest = REQUEST - msgOffset,
			    ";%hu %ld", index, value)
#else	/* INT_MAX == 32767 && defined (LONG16) */
			snprintf(&msgBuf[msgOffset], rest = REQUEST - msgOffset,
			    ";%hu %d", index, value)
#endif	/* INT_MAX == 32767 && defined (LONG16) */
			) < 0 || len >= rest
		) {
		    /********************************************************************
		     *  Send live data collected in msgBuf before it overflows to iCserver
		     *******************************************************************/
		    msgBuf[msgOffset] = '\0';		/* terminate */
		    iC_send_msg_to_server(iC_sockFN, msgBuf);
		    msgOffset = liveOffset;		/* msg = "C_channel:3" */
		}
		msgOffset += len;
	    }
	} else {
	    /********************************************************************
	     *  Debug break information is stored in gt_live with index and Live flag:
	     *    index            1 to 0x1fff
	     *    IGNORE	00	0x0000	not in current source or ignored (TX0.x)
	     *	  ACTIVE	01	0x2000	non OUTPUT variable in current source
	     *	  OUTPUT	10	0x4000	OUTPUT variable Q..._0 independent of source
	     *	  BREAKPOINT	11	0x6000	breakpoint details handled in iClive
	     *    Live flag             0x8000
	     *
	     *    \   00 |  01    11 |  10   debugMask
	     *  brk+-----+-----+-----+-----+
	     *  00 |  X =|     |     |     |	X  if debugMask == 0x0000 (00)   
	     *   --+-----+-----+-----+-----+
	     *  01 |  X  |  B =|     |     |	B if brk == debugMask
	     *     +-----+-----+-----+-----+
	     *  11 |  X  |  B  |  BP=|  B  |	  or brk == 0x6000 (11)
	     *   --+-----+-----+-----+-----+
	     *  10 |  X  |     |     |  B =|	BP if B and debugMask == 0x6000 (11)   
	     *     +-----+-----+-----+-----+
	     *
	     *******************************************************************/
	    if ((brk = gp->gt_live & 0x6000) != debugMask &&
		brk != 0x6000) {			/* not a break point */
		goto noBreak;
	    }
	    /********************************************************************
	     *  B  Break now either unconditionally sending "C_channel:8"
	     *      debugMask	0x2000	for every non OUTPUT variable or breakpoint      step
	     *      debugMask	0x4000	for every OUTPUT variable Q..._0 or breakpoint   next
	     *  BP or break for unconditional or conditional breakpoint sending "C_channel:9"
	     *      debugMask	0x6000	for every BREAKPOINT only                        continue
	     *******************************************************************/
	    if (iC_debug & 0100) fflush(iC_outFP);	/* in case dangling text debug messages without CR */
	    while ((len =				/* repeat code for speed */
#if	INT_MAX == 32767 && defined (LONG16)
		    snprintf(&msgBuf[msgOffset], rest = REQUEST - msgOffset,
			";%hu %ld", index, value)
#else	/* INT_MAX == 32767 && defined (LONG16) */
		    snprintf(&msgBuf[msgOffset], rest = REQUEST - msgOffset,
			";%hu %d", index, value)
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		    ) < 0 || len >= rest
	    ) {
		/********************************************************************
		 *  Send live data collected in msgBuf before it overflows to iCserver
		 *******************************************************************/
		msgBuf[msgOffset] = '\0';		/* terminate */
		iC_send_msg_to_server(iC_sockFN, msgBuf);
		msgOffset = liveOffset;			/* msg = "C_channel:3" */
	    }
	    msgOffset += len;
	    /********************************************************************
	     *  Send changes now
	     *******************************************************************/
	    msgBuf[msgOffset] = '\0';			/* terminate */
	    iC_send_msg_to_server(iC_sockFN, msgBuf);
	    msgOffset = liveOffset;			/* msg = "C_channel:3" */
	    if (debugStop == 0) {
		snprintf(tempBuf, TSIZ, "%hu:%d",		/* BREAK message */
		    C_channel, debugMask == 0x6000 ? 9 : 8);	/* BP : B */
		iC_send_msg_to_server(iC_sockFN, tempBuf);
		debugStop = 1;
	    }
	    while (debugStop != 0) {
		debugWait();				/* this by-passes iC scan */
		/********************************************************************
		 *  debugWait() has returned --- which has
		 *	either received 't' 'm' 'T' or an I.. input to be re-transmitted
		 *	    debugStop != 0         --- so wait here again
		 *	or received 'd' 's' 'n' 'c' or 'D'
		 *	    debugStop == 0         --- return to continue scan
		 *******************************************************************/
	    }
	}
	iC_linked = 0;
    }
} /* iC_liveData */

/********************************************************************
 *
 *	Receive active symbols to set up Live bits and send initial data
 *
 *******************************************************************/

static void
receiveActiveSymbols(char * cp1)
{
    Gate **		opp;
    unsigned short	debugMaskSave;
    Gate *		gp;
    int			index;
    long		value;
    int			fni;

    if (liveFlag) {
	for (opp = sTable; opp < sTend; opp++) {
	    (*opp)->gt_live &= 0x7fff;	/* clear live active */
	}
	liveFlag = 0;			/* do not set again until case 4 received */
    }
    /********************************************************************
     *  Send live data just in case there was also an input in this loop
     *  (should go straight to scan())
     *******************************************************************/
    if (msgOffset > liveOffset) {
	iC_send_msg_to_server(iC_sockFN, msgBuf);
    }
    msgOffset = liveOffset;		/* msg = "C_channel:3" */
    debugMaskSave = debugMask;
    debugMask = 0x0000;
    while ((cp1 = strchr(cp1, ';')) != NULL) {
	index = atoi(++cp1);
	assert(index <= sTend - sTable);/* check index is in range */
	gp = sTable[index-1];		/* index in iClive starts at 1 */
	gp->gt_live |= 0x8000;		/* set live active */
	value = (
	    (fni = gp->gt_fni) == ARITH ||
	    fni == D_SH ||
	    fni == F_SW ||
	    fni == TRAB ||
	    fni == CH_AR)   ? gp->gt_new
			    : fni == OUTW   ? gp->gt_out
					    : gp->gt_val < 0  ? 1
							      : 0;
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 04) fprintf(iC_outFP, "%4d %-15s %ld\n",
	    index, gp->gt_ids, value);	/* only INT_MAX != 32767 */
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	if (value) {
	    iC_linked++;			/* pretend it was linked */
	    iC_liveData(gp, value);		/* initial active live values */
	}
    }
    debugMask = debugMaskSave;
    /********************************************************************
     *  Send live data collected in msgBuf during RECEIVE_ACTIVE_SYMBOLS
     *  because scan() is only executed when another input occurs.
     *  a scan may occurr between received blocks which
     *  also sends a live C_channel:3 block
     *******************************************************************/
    if (msgOffset > liveOffset) {
	iC_send_msg_to_server(iC_sockFN, msgBuf);
    }
    msgOffset = liveOffset;			/* msg = "C_channel:3" */
} /* receiveActiveSymbols */

/********************************************************************
 *
 *	Receive Watch or restore values
 *	Format is <D_channel:12;<index> <value>;...
 *
 *******************************************************************/

static void
receiveWatchOrRestore(char * cp1)
{
    Gate *	gp;
    int		index;
    int		value;

    while ((cp1 = strchr(cp1, ';')) != NULL) {
	index = atoi(++cp1);
	assert(index <= sTend - sTable);	/* check index is in range */
	cp1 = strchr(cp1, ' ');
	assert(cp1 != NULL);
	value = atoi(++cp1);
	assert(value <= 3);
	gp = sTable[index-1];			/* index in iClive starts at 1 */
	gp->gt_live &= ~0x6000;			/* clear step/next or watch/ignore bits */
	gp->gt_live |= value << 13;		/* set watch/ignore or restore step/next */
#if	YYDEBUG
	if (iC_debug & 040) fprintf(iC_outFP, "W %3d	%s	%u %u %3hu\n",
	    index, gp->gt_ids, gp->gt_live >> 15, (gp->gt_live >> 13) & 0x3, gp->gt_live & 0x1fff);
	fflush(iC_outFP);
#endif	/* YYDEBUG */
    }
} /* receiveWatchOrRestore */

/********************************************************************
 *
 *	Convert an integer to a binary string
 *	binBuf	is a buffer which must be at least 33 characters long
 *	value	is a 32 bit integer
 *	binFlag is either 0, in which up to 32 bits are converted to
 *		a binary string without leading 0's
 *		or it is  1, in which case 8 bits with leading 0's are produced
 *
 *******************************************************************/

static char *
#if	INT_MAX == 32767 && defined (LONG16)
convert2binary(long value, int binFlag)
#else	/* INT_MAX == 32767 && defined (LONG16) */
convert2binary(int value, int binFlag)
#endif	/* INT_MAX == 32767 && defined (LONG16) */
{
    char *		binPtr;
#if	INT_MAX == 32767 && defined (LONG16)
    unsigned long	binMask;
#else	/* INT_MAX == 32767 && defined (LONG16) */
    unsigned int	binMask;
#endif	/* INT_MAX == 32767 && defined (LONG16) */
    if (binFlag == 0 && value == 0) {	/* convert to binary string */
	strcpy(binBuf, "0");		/* '0' is the only digit */
    } else {
	binPtr = binBuf;
	binMask = binFlag ? 1 << 7 : 1 << 31;
	do {
	    if (binMask & value) {
		*binPtr++ = '1';	/* start with the left-most '1' digit */
		binFlag = 1;
	    } else if (binFlag) {
		*binPtr++ = '0';
	    }
	} while ((binMask >>= 1) != 0);
	*binPtr++ = '\0';		/* there is at least 1 '1' digit */
    }
    return binBuf;
} /* convert2binary */
#endif	/* TCP */

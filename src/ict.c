static const char ict_c[] =
"@(#)$Id: ict.c,v 1.43 2005/01/17 14:55:23 jw Exp $";
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
 *	ict.c
 *	parallel plc - runtime execution with TCP/IP I/O to iCserver
 *
 *	Standalone run-time version for TCP/IP communication.
 *	Alternatively to produce compiled applications
 *	linked with libict.a produced with LOAD option
 *
 *	branched from "icc.c,v 1.6 1999/08/06"
 *
 *******************************************************************/

/* J.E. Wulff	3-Mar-85 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<time.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/time.h>
#include	<signal.h>
#include	<ctype.h>
#include	<assert.h>
#include	"tcpc.h"
#include	"icg.h"
#include	"icc.h"

#define MAX_IO	8
#define MAX_W	2
#define INTR 0x1c    /* The clock tick interrupt */

#if	YYDEBUG
static void	display(void);
#endif	/* YYDEBUG */

#define D10	10			/* 1/10 second select timeout under Linux */
#define ENTER	'\n'

Functp	*i_lists[] = { I_LISTS };

static Gate	alist0;			/* these lists are toggled */
static Gate	alist1;
Gate *		a_list;			/* arithmetic output action list */
static Gate	olist0;			/* these lists are toggled */
static Gate	olist1;
Gate *		o_list;			/* logic output action list */
Gate *		c_list;			/* main clock list "iClock" */
static Gate	flist;
Gate *		f_list;			/* auxiliary function clock list */
static Gate	slist;
Gate *		s_list;			/* send bit and byte outputs */
static Gate *	timNull[] = { 0, 0, 0, 0, 0, 0, 0, 0, }; /* speeds up tim[] lookup */

unsigned char	QM_[IXD/8];		/* Output slot mask per cage */
unsigned char	QMM;			/* Output cage mask for 1 rack */

unsigned char	pdata[IXD];		/* input differences */

short		dis_cnt;
short		error_flag;

unsigned	scan_cnt;		/* count scan operations */
unsigned	link_cnt;		/* count link operations */
#if	YYDEBUG && (!defined(_WINDOWS) || defined(LOAD))
unsigned	glit_cnt;		/* count glitches */
unsigned long	glit_nxt;		/* count glitch scan */
#endif	/* YYDEBUG && (!defined(_WINDOWS) || defined(LOAD)) */

static int	sockFN = 0;		/* TCP/IP socket file number */

static int	msgOffset = 0;		/* for message send */
static int	regOffset = 0;		/* for register send */
static int	liveOffset = 0;		/* for live header */
#ifndef EFENCE 
static char	msgBuf[REQUEST];
static char	regBuf[REQUEST];
char		outBuf[REQUEST];
char		rpyBuf[REPLY];
int		outOffset;
#else	/* EFENCE */
static char *	msgBuf;
static char *	regBuf;
char *		outBuf;
static char *	rpyBuf;
#endif	/* EFENCE */
static int	maxChannels = 0;
static int	C_channel = 0;		/* channel for sending messages to Debug */
static Gate	D_gate;			/* Gate on channel for receiving messages from Debug */

/********************************************************************
 *
 *	Procedure icc
 *
 *******************************************************************/

void
icc(
    Gate *	g_lists,
    unsigned	gate_count[])
{
    int		i;
    short	pass;
    int		c;
    short	typ;
    int		cnt;
    int		index;
#ifdef	LOAD
    Gate **	opp;
    char *	cp;
    Gate **	Channels;
#else	/* LOAD */
    unsigned *	gcp;
    Functp * *	ilp;
#endif	/* LOAD */
    Gate *	gp;
    Gate **	tim = timNull;	/* point to array of 8 null pointers */
    Functp	init_fa;
    int		tcnt = 1;
    float	delay = 0.0;	/* timer processing stopped */
    int		retval;
#ifdef	EFENCE
    msgBuf = emalloc(REQUEST);
    regBuf = emalloc(REQUEST);
    outBuf = emalloc(REQUEST);
    rpyBuf = emalloc(REPLY);
#endif	/* EFENCE */
    initIO();				/* catch memory access signal */	
    if (outFP != stdout) {
	fclose(outFP);
#ifndef LOAD
	if (iFlag) {
	    inversionCorrection();
	    iFlag = 0;
	}
#endif	/* LOAD */
	outFP = stdout;			/* standard output from here */
    }
    if (errFP != stderr) {
	fclose(errFP);
	errFP = stderr;			/* standard error from here */
    }

    if ((gp = TX_[0]) != 0) {		/* are EOP or TX0 timers programmed */
	tim = gp->gt_list;		/* TX0.0 - TX0.7 */
	assert(tim);
	for (cnt = 1; cnt < 8; cnt++) {
	    if (tim[cnt] != NULL) {	/* any of the 7 timers programmed ? */
		delay = timeout;	/* yes - setup timer processing */
		break;			/* could optimise by varying delay */
	    }
	}
    }

    if ((debug & 0400) == 0) {
	/* Start TCP/IP communication before any inputs are generated => outputs */
	if (micro) microReset(04);
	sockFN = connect_to_server(hostNM, portNM, delay);
#ifndef	LOAD
	snprintf(regBuf, REQUEST, "%s %d", "C0", IXD);
	send_msg_to_server(sockFN, regBuf);	/* register C0 with IXD as maxIOs */
	if (micro) microPrint("connect to server", 0);
#else	/* LOAD */
	char *		tbp = regBuf;
	int		tbc = REQUEST;
	int		cn  = 0;
	char *		tbt;
	int		i;
	const char *	sr[] = { "N", ",SC", ",RD", };	/* name, controller, debugger */

	if (debug & 04) fprintf(outFP, "%s: I/O registration objects\n", iccNM);
	for (i = 0; i < 3; i++) {
	    tbp += cn;
	    tbc -= cn;
	    cn = snprintf(tbp, tbc, "%s%s", sr[i], iccNM);
	}
	/* use last string to initialise name "D<progname>[-<instance>]" for D_gate */
	D_gate.gt_ids = emalloc(cn-1);			/* including '\0' */
	strncpy(D_gate.gt_ids, tbp+2, cn-1);		/* +2 leave out ",R" */
	D_gate.gt_new = -2;	/* changes - not equal 0 - 5 for debug messages */
	D_gate.gt_old = -1;	/* never changes - not equal 0 - 5 for debug messages */
//#####if	YYDEBUG
//####	if (debug & 0100) fprintf(outFP, "Initialise D_gate: '%s', gt_new = %d, gt_old = %d\n",
//####			    D_gate.gt_ids, D_gate.gt_new, D_gate.gt_old);
//#####endif	/* YYDEBUG */

	for (opp = sTable; opp < sTend; opp++) {
	    gp = *opp;
	    tbp += cn;
	    tbc -= cn;
	    cn = 0;
	    if (gp->gt_ini == -INPW && *gp->gt_ids != 'T') {	/* suppress TX0 */
		cn = snprintf(tbp, tbc, ",R%s", gp->gt_ids);	/* read input at controller */
	    } else
	    if (gp->gt_fni == OUTW) {
		cn = snprintf(tbp, tbc, ",S%s", gp->gt_ids);	/* send output at controller */
		if (gp->gt_ini == -ARN) {
		    if ((tbt = strrchr(tbp, '_')) != NULL && *(tbt+1) == '0') {
			*tbt = '\0';	/* strip trailing "_0" from internal output name */
			cn -= 2;
			assert(cn > 0);
		    } else {
			fprintf(errFP, "\n%s: illformed output name '%s'\n", iccNM, tbp + 1);
			quit(3);
		    }
		}
	    }
	    if (cn > 0) {
		if (debug & 04) fprintf(outFP,
		    "%s	%d	%d\n", gp->gt_ids, (int)gp->gt_ini, (int)gp->gt_fni);
		if (strlen(iidNM) > 0) {
		    cn += snprintf(tbp + cn, tbc - cn, "-%s", iidNM);	/* append instance id */
		}
	    }
	    if (cn >= tbc) {
		fprintf(errFP, "\n%s: buffer '%s' has overflowed %d %d\n", iccNM, regBuf, cn, tbc);
		quit(4);
	    }
	}
	if (debug & 04) fprintf(outFP, "register:%s\n", regBuf);
	send_msg_to_server(sockFN, regBuf);		/* register controller and IOs */
	if (micro) microPrint("connect to server", 0);
	/* receive registration reply */
	if (rcvd_msg_from_server(sockFN, rpyBuf, REPLY)) {
	    if (micro) microPrint("reply from server", 0);
	    if (debug & 04) fprintf(outFP, "reply:%s\n", rpyBuf);
	    cp = rpyBuf - 1;
	    do {
		index = atoi(++cp);
		if (index > maxChannels) {
		    maxChannels = index;
		}
	    } while ((cp = strchr(cp, ',')) != NULL);
	    maxChannels++;	/* size of array Channels[] determined from received msg */
	    Channels = (Gate **)calloc(maxChannels, sizeof(Gate *));
	    // C channel index - messages from controller to debugger
	    C_channel = atoi(rpyBuf);
	    assert(C_channel > 0 && C_channel < maxChannels);
	    // D channel index - messages from debugger to controller
	    cp = strchr(rpyBuf, ',');
	    assert(cp);
	    index = atoi(++cp);
	    assert(index > 0 && index < maxChannels);
	    Channels[index] = &D_gate;				/* ==> Debug input */
	    // remaining input and output channel indices matching registrations
	    cp = strchr(cp, ',');
	    for (opp = sTable; opp < sTend; opp++) {
		gp = *opp;
		if (gp->gt_ini == -INPW && *gp->gt_ids != 'T' ||	/* suppress TX0 */
		    gp->gt_fni == OUTW) {
		    assert(cp);
		    index = atoi(++cp);
		    assert(index > 0 && index < maxChannels);
		    if (gp->gt_ini == -INPW) {
			Channels[index] = gp;				/* ==> Input */
		    } else
		    if (gp->gt_fni == OUTW) {
			// TODO gp->gt_list = (Gate **)index;
			gp->gt_channel = index;				/* <== Output */
		    }
		    cp = strchr(cp, ',');
		}
	    }
	    assert(cp == NULL);
	    if (debug & 04) fprintf(outFP, "reply: Channels %d\n", maxChannels);
	    regOffset = snprintf(regBuf, REQUEST, "%d:2;%s", C_channel, iccNM);
	    if (micro) microPrint("Send application name", 0);
	    send_msg_to_server(sockFN, regBuf);				/* Application Name */
	} else {
	    fprintf(errFP, "\n'%s' disconnected by server\n", iccNM);
	    quit(0);			/* quit normally */
	}
#endif	/* LOAD */
    }

/********************************************************************
 *
 *	Initialise the work lists to empty lists
 *
 *******************************************************************/

    error_flag = 0;
    alist0.gt_rlist = (Gate **)(a_list = &alist1);	/* initialise alternate */
    Out_init(a_list);
    alist1.gt_rlist = (Gate **)(a_list = &alist0);	/* start with alist0 */
    Out_init(a_list);
    olist0.gt_rlist = (Gate **)(o_list = &olist1);	/* initialise alternate */
    Out_init(o_list);
    olist1.gt_rlist = (Gate **)(o_list = &olist0);	/* start with olist0 */
    Out_init(o_list);
#ifdef	LOAD
    c_list = &iClock;			/* system clock list */
    /* TODO check c_list is a propoer list even if DEQ (also other places) */
#endif	/* LOAD */
    f_list = &flist;			/* function clock list */
    Out_init(f_list);
    s_list = &slist;			/* send outputs */
    Out_init(s_list);
    /********************************************************************
     *  Carry out 4 Passes to initialise all Gates
     *******************************************************************/
#if	YYDEBUG
    if (debug & 0100) fprintf(outFP, "\nINITIALISATION\n");
#endif	/* YYDEBUG */

#ifdef	LOAD 
    /* if (debug & 0400) == 0 then no live bits are set in gt_live | 0x8000 */
    /* header for live data */
    /********************************************************************
     *  Initialise msgBuf for live data collection before initialisation
     *  liveOffset may also be used with messages in regBuf because they
     *  use the same channel number and all relevant messages have only
     *  one command digit.
     *  Message in msgBuf is ignored of debug &0400 stops process after init
     *******************************************************************/
    msgOffset = liveOffset = snprintf(msgBuf, REQUEST, "%d:3", C_channel);
#endif	/* LOAD */

    for (i = 0; i < IXD; i++) {		/* clear output array used to hold */
	QX_[i] = 0;			/* output size X, B or W during compilation */
    }
    for (pass = 0; pass < 4; pass++) {
#if	YYDEBUG
	if (debug & 0100) fprintf(outFP, "\nPass %d:", pass + 1);
#endif	/* YYDEBUG */
#ifdef	LOAD
	for (opp = sTable; opp < sTend; opp++) {
	    gp = *opp;
	    typ = gp->gt_ini > 0 ? AND : -gp->gt_ini;
	    if (typ < MAX_OP) {
		init_fa = i_lists[typ][pass];
		(*init_fa)(gp, typ);	/* initialise for this pass */
	    }
	}
#else	/* LOAD */
	gp = g_lists;
	gcp = gate_count;
	ilp = i_lists;
	for (typ = 0; typ < MAX_OP; typ++) {
	    init_fa = (*ilp++)[pass];
	    for (cnt = *gcp++; cnt; cnt--) {
		(*init_fa)(gp++, typ);	/* initialise for this pass */
	    }
	}
#endif	/* LOAD */
    }

#if	YYDEBUG
    if (debug & 0100) {
	fprintf(outFP, "\nInit complete =======\n");
    }
#endif	/* YYDEBUG */

    if (error_flag) {
	if (error_flag == 1) {
	    fprintf(outFP, "\n*** Fatal Errors ***\n");
	    quit(1);
	}
	fprintf(outFP, "\n*** Warnings ***\n");
    }
    if (debug & 0400) {
	quit(0);			/* terminate - no inputs */
    }
    signal(SIGINT, quit);		/* catch ctrlC and Break */	

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
     *  ctrl-C, when it has been brought to the foregroung with fg.
     *******************************************************************/
    signal(SIGTTIN, SIG_IGN);		/* ignore tty input signal in bg */
#endif	/* SIGTTIN */

    if ((gp = tim[0]) != NULL) {
#if	YYDEBUG
	if (debug & 0100) fprintf(outFP, "\nEOP:\t%s  1 ==>", gp->gt_ids);
#endif	/* YYDEBUG */
	gp->gt_val = -1;			/* set EOP once as first action */
	link_ol(gp, o_list);			/* fire EOP Input Gate */
#if	YYDEBUG
	if (debug & 0100) fprintf(outFP, " -1");
#endif	/* YYDEBUG */
    }

    dis_cnt = DIS_MAX;
    /********************************************************************
     *  Operational loop
     *******************************************************************/
    for ( ; ; ) {
	if (micro & 06) microPrint("Input", 0);
	if (++mark_stamp == 0) {	/* next generation for check */
	    mark_stamp++;		/* leave out zero */
	}

	/********************************************************************
	 *  New I/O handling and the sequencing of different action lists
	 *
	 *  1   initialisation - put EOP on o_list
	 *      # actions after an idle period:
	 *  2   Loop:   scan a_list unless empty
	 *                 INPW ARITH to a_list
	 *                 comparisons to o_list
	 *                 clocked actions to c_list via own clock list
	 *  3          scan o_list; goto Loop unless empty
	 *                 bits used in arithmetic to a_list (less common)
	 *                 clocked actions to c_list via own clock list
	 *  4          scan c_list; goto Loop unless empty
	 *                 put ARITH and GATE actions on a_list and o_list
	 *                 defer 'if else switch' C actions to f_list
	 *  5          scan f_list; goto Loop unless empty
	 *                 C actions can generate now ARITH and GATE actions
	 *  6   scan s_list			# only one scan is required
	 *          do OUTW Gates building send string
	 *      send output string with final outputs only
	 *      switch to alternate a_list and o_list
	 *      idle - wait for next input
	 *      read new input and link INPW Gates directly to a_list
	 *      or via traMb to o_list
	 *  Loop algorithms with for or do while - continue are all incorrect
	 *******************************************************************/
      Loop:
	if (a_list != a_list->gt_next) { scan_ar (a_list);            }
	if (o_list != o_list->gt_next) { scan    (o_list); goto Loop; }
	if (c_list != c_list->gt_next) { scan_clk(c_list); goto Loop; }
	if (f_list != f_list->gt_next) { scan_clk(f_list); goto Loop; }
	if (s_list != s_list->gt_next) { scan_snd(s_list);            }

#ifdef	LOAD 
	/********************************************************************
	 *  Send live data collected in msgBuf during initialisation
	 *  and previous loop to iCserver
	 *******************************************************************/
	if (msgOffset > liveOffset) {
	    send_msg_to_server(sockFN, msgBuf);
	    if (micro) microReset(0);
	}
	/********************************************************************
	 *  Initialise msgBuf for live data collection during next loop
	 *******************************************************************/
	msgOffset = liveOffset;				/* msg = "C_channel:3" */
#endif	/* LOAD */

#if	YYDEBUG
	if (scan_cnt || link_cnt) {
	    if (debug & 0100) fprintf(outFP, "\n");
	    if (debug & 02000) {
		fprintf(outFP, "scan = %5u  link = %5u",
		    scan_cnt, link_cnt);
		if (glit_cnt) {
		    fprintf(outFP, "  glitch = %5u, %lu", glit_cnt, glit_nxt);
		}
		fprintf(outFP, "\n");
	    }
	    glit_cnt = glit_nxt =
	    scan_cnt = link_cnt = 0;
	}
#endif	/* YYDEBUG */
	/********************************************************************
	 *  Switch to alternate lists
	 *
	 *  alternate list contains all those gates which were marked in
	 *  the previous scan and which were active more than
	 *  MARKMAX times. These are oscillators which wil be
	 *  scanned again in the next cycle.
	 *******************************************************************/
	a_list = (Gate*)a_list->gt_rlist;	/* alternate arithmetic list */
	o_list = (Gate*)o_list->gt_rlist;	/* alternate logic list */

	if (osc_gp) {
	    fprintf(outFP,
		"*** Warning: %s has oscillated %d times - check iC program!!!\n",
		osc_gp->gt_ids, osc_gp->gt_mcnt);
	    osc_gp = NULL;
	}

#if	YYDEBUG
	if (debug & 0300) {			/* osc or detailed info */
	    if ((debug & 0200) &&
		(o_list->gt_next != o_list || a_list->gt_next != a_list)) {
		fprintf(outFP, "OSC =");
		for (gp = a_list->gt_next; gp != a_list; gp = gp->gt_next) {
		    fprintf(outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
		}
		for (gp = o_list->gt_next; gp != o_list; gp = gp->gt_next) {
		    fprintf(outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
		}
		fprintf(outFP, "\n");
	    }
	    display();				/* inputs and outputs */
	}
#endif	/* YYDEBUG */
#ifndef	LOAD

/********************************************************************
 *
 *	Output to external output modules
 *
 *	QMM and all QM_[] bytes are initially clear (global variables)
 *	bits are set in QMM and QM_[] in preceding scans by outMw and outMx
 *	to mark cages and slots
 *	QMM and all QM_[] are left clear after the following loops
 *
 *	Use 8 bit masks for fast conversion of mask to index via bitIndex[]
 *
 *******************************************************************/

	msgOffset = 0;
	while (QMM) {			/* rack with bit set for every cage */
	    int len;
	    int rest;
	    int mask = QMM & -QMM;		/* rightmost cage set in QMM */
	    int cage = bitIndex[mask];		/* mask has only 1 bit set */
	    int slots = QM_[cage];
	    int cageOffset = cage << 3;		/* cage has 8 slots */
	    while (slots) {
		int mask = slots & -slots;	/* rightmost slot set in slots */
		int slot = cageOffset + bitIndex[mask];
#if INT_MAX == 32767 && defined (LONG16)
		long val;
#else
		int val;
#endif
		char Qtype = QT_[slot];
		assert(Qtype);			/* make sure slot is programmed */
		val = Qtype == 'L' ? *(long*)&QX_[slot] :
		      Qtype == 'W' ? *(short*)&QX_[slot] : QX_[slot];
		while ((len =
#if INT_MAX == 32767 && defined (LONG16)
			snprintf(&msgBuf[msgOffset], rest = REPLY - msgOffset,
			    "%c%d.%ld,", Qtype, slot, val)
#else
			snprintf(&msgBuf[msgOffset], rest = REPLY - msgOffset,
			    "%c%d.%d,", Qtype, slot, val)
#endif
			) < 0 || len > rest	/* use > because of -1 truncation */
		    ) {
		    msgBuf[msgOffset - 1] = '\0';	/* clear last ',' */
		    if (micro) microPrint("Send intermediate", 0);
		    send_msg_to_server(sockFN, msgBuf);
		    if (micro) microReset(0);
		    msgOffset = 0;
		}
		msgOffset += len;
		slots &= ~mask;			/* clear rightmost slot in slots */
	    }
	    QM_[cage] = slots;			/* clear QM_[cage] */
	    QMM &= ~mask;		/* clear rightmost cage in QMM */
	}
	if (msgOffset > 0) {
	    msgBuf[msgOffset - 1] = '\0';	/* clear last ',' */
	    if (micro) microPrint("Send", 0);
	    send_msg_to_server(sockFN, msgBuf);
	    if (micro) microReset(0);
	}
#else	/* LOAD */

	/********************************************************************
	 *  Send output data collected in outBuf to iCserver
	 *******************************************************************/
	if (outOffset > 1) {		/* if any item it is at least 2 long 'x,' */
	    outBuf[outOffset - 1] = '\0';	/* clear last ',' */
	    if (micro) microPrint("New Send", 0);
	    send_msg_to_server(sockFN, outBuf);
	    memset(outBuf, 0, REQUEST);		/* clear for next data from outMw() */
	    outOffset = 0;
	    if (micro) microReset(0);
	}
#endif	/* LOAD */

	/********************************************************************
	 *  Input from external input modules and time input if used
	 *  Wait for input in a select statement most of the time
	 *
	 *  The debug facility -d1000 stops linking the 100 msec and 1 second
	 *  TIMERS tim[4] and tim[5] when they are connected directly to
	 *  the slave input of a clock or timer Gate and that clock or timer
	 *  has no Gates on the clock list it controls - ie nothing will happen.
	 *  This stops continuous output when tracing logic (-d100 or -t) when
	 *  these internal TIMERS produce no change.
	 *  It also reduces the data traffic to iClive and reduces flashing of
	 *  clocks and timers controlled by internal TIMERS.
	 *******************************************************************/
	for (cnt = 0; cnt == 0; ) {	/* stay in input loop if nothing linked */
	    retval = wait_for_next_event(sockFN);	/* wait for inputs or timer */
	    if (micro & 02) microReset(0);

	    if (retval == 0) {
		timeoutCounter = timeoutValue;	/* will repeat if timeout with input */
		/********************************************************************
		 *  TIMERS here every 100 milliseconds
		 *******************************************************************/
		if ((gp = tim[4]) != 0) {		/* 100 millisecond timer */
#if	YYDEBUG
		    if (debug & 01000) {
			Gate **	lp;
			Gate *	tp;
			int	cn1 = 2;
			lp = gp->gt_list;	/* test if any clock or timer is active */
			do {				/* for normal and inverted */
			    while ((tp = *lp++) != 0) {
				if (tp->gt_fni == CLCK || tp->gt_fni == TIMR) {
				    tp = *((Gate **)tp->gt_list);
				    if (tp->gt_next != tp) {
					goto linkT4;	/* found an active clock or timer */
				    }
				} else {
				    goto linkT4;	/* found a link to non clock or timer */
				}
			    }
			} while (--cn1);
			goto skipT4;		/* excuse spaghetti - faster without flag */
		    }
		linkT4:
		    if (debug & 0100) fprintf(outFP, " %s ", gp->gt_ids);
#endif	/* YYDEBUG */
		    gp->gt_val = - gp->gt_val;	/* complement input */
		    link_ol(gp, o_list);
		    cnt++;
#if	YYDEBUG
		    if (debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', outFP);
		skipT4: ;
#endif	/* YYDEBUG */
		}
		if (--tcnt <= 0) {
		    tcnt = D10;
		    if ((gp = tim[5]) != 0) {	/* 1 second timer */
#if	YYDEBUG
			if (debug & 01000) {
			    Gate **	lp;
			    Gate *	tp;
			    int	cn1 = 2;
			    lp = gp->gt_list;	/* test if any clock or timer is active */
			    do {			/* for normal and inverted */
				while ((tp = *lp++) != 0) {
				    if (tp->gt_fni == CLCK || tp->gt_fni == TIMR) {
					tp = *((Gate **)tp->gt_list);
					if (tp->gt_next != tp) {
					    goto linkT5; /* found an active clock or timer */
					}
				    } else {
					goto linkT5;	/* found a link to non clock or timer */
				    }
				}
			    } while (--cn1);
			    goto skipT5;	/* excuse spaghetti - faster without flag */
			}
		    linkT5:
			if (debug & 0100) fprintf(outFP, " %s ", gp->gt_ids);
#endif	/* YYDEBUG */
			gp->gt_val = - gp->gt_val;	/* complement input */
			link_ol(gp, o_list);
			cnt++;
#if	YYDEBUG
			if (debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', outFP);
		    skipT5: ;
#endif	/* YYDEBUG */
		    }
		}
	    } else if (retval > 0) {
		if (FD_ISSET(sockFN, &rdfds)) {
		    /********************************************************************
		     *  TCP/IP input
		     *******************************************************************/
		    if (micro) microReset(04);
		    if (rcvd_msg_from_server(sockFN, rpyBuf, REPLY)) {
			int		slot;
#if	INT_MAX == 32767 && defined (LONG16)
			long		val;
#else	/* INT_MAX == 32767 && defined (LONG16) */
			int		val;
#endif	/* INT_MAX == 32767 && defined (LONG16) */
			int		mask;
			int		diff;
			int		count;
//#####if	YYDEBUG
//####			if (debug & 0100) fprintf(outFP, "%s:rpyBuf'%s'\n", iccNM, rpyBuf);
//#####endif	/* YYDEBUG */
#ifndef	LOAD
			char		utype[4];		/* need 2 with NUL */

// ########## OLD Input
			if (
#if	INT_MAX == 32767 && defined (LONG16)
				sscanf(rpyBuf, "%1[XBWLD]%d.%ld", utype, &slot, &val)
#else	/* INT_MAX == 32767 && defined (LONG16) */
				sscanf(rpyBuf, "%1[XBWLD]%d.%d", utype, &slot, &val)
#endif	/* INT_MAX == 32767 && defined (LONG16) */
				== 3 && slot < IXD
			    ) {
			    switch (*utype) {
			    case 'X':				/* bit input */
				val &= 0xff;			/* safety measure */
				diff = val ^ pdata[slot];
				pdata[slot] = val;		/* ready for next scan */
				slot <<= 3;			/* convert to bit index */
				while (diff) {		/* age old algorithm from CSR days */
				    mask = diff & -diff;	/* rightmost bit set in diff */
				    index = bitIndex[mask];	/* mask has only 1 bit set */
				    /* ignore Gate if not programmed  or no change in bit */
				    if ((gp = IX_[slot+index]) != 0 &&
					((gp->gt_val & 0x80) ^ (val & mask ? 0x80 : 0x00))) {
					/* relies on input initialized to +1 !!!!!! */
					/* and no other function modifies gp_gt_val */
					gp->gt_val = - gp->gt_val;	/* complement input */
#if YYDEBUG
					if (debug & 0100) fprintf(outFP, " %s ", gp->gt_ids);
#endif
					link_ol(gp, o_list);	/* fire Input Gate */
#if YYDEBUG
					if (debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', outFP);
#endif
					cnt++;
					/* o_list will be scanned before next input */
				    }
				    diff &= ~mask;		/* clear rightmost bit in diff */
				}				/* loops only once for every 1 in diff */
				break;

			    case 'B':				/* 8 bit byte input */
				if ((gp = IB_[slot]) != NULL) {
				    val &= 0xff;		/* reduce to byte */
				    goto wordIn;
				}
				break;

			    case 'W':				/* 16 bit word input */
				if ((gp = IW_[slot]) != NULL) {
				    val = (short)val;		/* reduce to signed word */
#if	INT_MAX != 32767 || defined (LONG16)
				    goto wordIn;
				}
				break;

			    case 'L':				/* 32 bit long input */
				if ((gp = IL_[slot]) != NULL) {
#endif	/* INT_MAX == 32767 && defined (LONG16) */
			    wordIn:
				    if (val != gp->gt_new &&	/* first change or glitch */
				    ((gp->gt_new = val) != gp->gt_old) ^ (gp->gt_next != 0)) {
					/* arithmetic master action */
#if	YYDEBUG
					if (debug & 0100) fprintf(outFP, "%s[\t", gp->gt_ids);
#endif	/* YYDEBUG */
					link_ol(gp, a_list);	/* no actions */
#if	YYDEBUG
#if	INT_MAX == 32767 && defined (LONG16)
					/* TODO - format for byte, word or long */
					if (debug & 0100) fprintf(outFP, "%ld", gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
					if (debug & 0100) fprintf(outFP, "%d", gp->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#endif	/* YYDEBUG */
					cnt++;
				    }
				}
				break;

			    default:
				goto RcvError;			/* unkikely if sscanf %1[XBWLD] OK */
			    }
			}
// ########## NEW Input
#else	/* LOAD */
			static int	liveFlag;

			if (isdigit(rpyBuf[0])) {
			    cp = rpyBuf - 1;
			    do {
				int n;
				if (
#if	INT_MAX == 32767 && defined (LONG16)
				    (n = sscanf(++cp, "%d:%ld", &slot, &val))
#else	/* INT_MAX == 32767 && defined (LONG16) */
				    (n = sscanf(++cp, "%d:%d", &slot, &val))
#endif	/* INT_MAX == 32767 && defined (LONG16) */
				    == 2 &&
				    slot > 0	&&
				    slot < maxChannels &&
				    (gp = Channels[slot]) != NULL
				) {
				    if (debug & 04) fprintf(outFP, "%4d %-15s val=%3d new=%3d old=%3d next=%p\n",
					slot, gp->gt_ids, val, gp->gt_new, gp->gt_old, gp->gt_next);	/* only INT_MAX != 32767 */
				    if (val != gp->gt_new &&	/* first change or glitch */
				    ((gp->gt_new = val) != gp->gt_old) ^ (gp->gt_next != 0)) {
					/* arithmetic master action */
//#####if	YYDEBUG
//#### 					if (debug & 0100) fprintf(outFP, "INPUT: %s, ini=%d fni=%d\n", gp ? gp->gt_ids : "null", gp->gt_ini, gp->gt_fni);
//#####endif	/* YYDEBUG */
					if (gp->gt_fni == TRAB) {
#if	YYDEBUG
					    if (debug & 0100) fprintf(outFP, "%s<\t", gp->gt_ids);
#endif	/* YYDEBUG */
					    count = traMb(gp, 0);	/* distribute bits directly */
#if	YYDEBUG
					    if (debug & 0100) fprintf(outFP, "%d%s",
							    gp->gt_new, count ? "" : "\n");
#endif	/* YYDEBUG */
					    cnt += count;	/* extra number of fired gates */
					} else
					if (gp->gt_ini == -INPW) {
#if	YYDEBUG
					    if (debug & 0100) fprintf(outFP, "%s[\t", gp->gt_ids);
#endif	/* YYDEBUG */
					    link_ol(gp, a_list);	/* no actions */
#if	YYDEBUG
#if	INT_MAX == 32767 && defined (LONG16)
					    /* TODO - format for byte, word or long */
					    if (debug & 0100) fprintf(outFP, "%ld", gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
					    if (debug & 0100) fprintf(outFP, "%d", gp->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#endif	/* YYDEBUG */
					    cnt++;
					} else
					if (gp->gt_fni == UDFA) {	/* D_gate initialised type */
//#####if	YYDEBUG
//####					    if (debug & 0100) fprintf(outFP, "D_gate: n = %d, channel = %d, val = %d, gp = %s, ini = %d, fni = %d\n",
//####						n, slot, (int)val, gp ? gp->gt_ids : "null", gp->gt_ini, gp->gt_fni);
//#####endif	/* YYDEBUG */
					    gp->gt_new = 0;	/* allow repeated 1-6 commands */
					    switch (val) {
					    case 0:		/* IGNORE */
						break;

					    case 1:		/* GET_SYMBOL_TABLE */
						printf("Symbol Table requested by '%s'\n", iccNM);
						/* prepare index entries first to allow ALIAS back-references */
						index = 0;
						for (opp = sTable; opp < sTend; opp++) {
						    (*opp)->gt_live = index++;	/* index and live inhibit */
						    index &= 0x7fff;		/* rolls over if > 32768 Symbols */
						}
						regOffset = snprintf(regBuf, REQUEST, "%d:1", C_channel);
						/* to maintain index correlation send all symbols */
						for (opp = sTable; opp < sTend; opp++) {
						    int		len;
						    int		rest;
						    int		fni;
						    int		inverse = 0;
						    char *	ids;

						    gp = *opp;
						    ids = gp->gt_ids;
						    fni = gp->gt_fni;
						    while (gp->gt_ini == -ALIAS) {
							if (gp->gt_fni == GATE) {
							    inverse ^= gp->gt_mark;	/* holds ALIAS inversion flag */
							}
							gp = (Gate*)gp->gt_rlist;	/* resolve ALIAS */
							fni = MAX_FTY + gp->gt_fni + inverse;
						    }
						    if (fni == CH_BIT && gp->gt_ini >= 0) {
							fni = RI_BIT;		/* AND - LATCH, display as logic signal */
						    }
						    while (rest = REQUEST - regOffset, (len = fni >= MAX_FTY ?
							snprintf(&regBuf[regOffset], rest, ";%s %d %d", ids, fni, gp->gt_live) :
							snprintf(&regBuf[regOffset], rest, ";%s %d", ids, fni)
							) < 0 || len >= rest) {
							regBuf[regOffset] = '\0';		/* terminate */
							if (micro) microPrint("Send Symbols intermediate", 0);
							send_msg_to_server(sockFN, regBuf);
							if (micro) microReset(0);
							regOffset = liveOffset;
						    }
						    regOffset += len;
						}
						if (regOffset > liveOffset) {
						    if (micro) microPrint("Send Symbols", 0);
						    send_msg_to_server(sockFN, regBuf);
						    if (micro) microReset(0);
						}
						/* end of symbol table - execute scan - follow with '0' to leave in iCserver */
						regOffset = snprintf(regBuf, REQUEST, "%d:4,%d:0", C_channel, C_channel);
						if (micro) microPrint("Send Scan Command", 0);
						send_msg_to_server(sockFN, regBuf);
						liveFlag = 1;		/* live inhibit bits are set */
						if (micro) microReset(0);
						break;

					    case 3:		/* RECEIVE_ACTIVE_SYMBOLS */
					    case 4:		/* LAST_ACTIVE_SYMBOLS */
						if (liveFlag) {
						    for (opp = sTable; opp < sTend; opp++) {
							(*opp)->gt_live &= 0x7fff;	/* clear live active */
						    }
						    liveFlag = 0;	/* do not set again until case 4 received */
						}
						/********************************************************************
						 *  Send live data just in case there was also an input in this loop
						 *  (should go straight to scan())
						 *******************************************************************/
						if (msgOffset > liveOffset) {
						    send_msg_to_server(sockFN, msgBuf);
						    if (micro) microReset(0);
						}
						msgOffset = liveOffset;		/* msg = "C_channel:3" */
						char * cp1 = rpyBuf;
						while ((cp1 = strchr(cp1, ';')) != NULL) {
						    long		value;
						    int		fni;
						    index = atoi(++cp1);
						    assert(index < sTend - sTable);	/* check index is in range */
						    gp = sTable[index];
						    gp->gt_live |= 0x8000;	/* set live active */
						    value = (
							(fni = gp->gt_fni) == ARITH ||
							fni == D_SH ||
							fni == F_SW ||
							fni == TRAB ||
							fni == OUTW ||
							(fni == CH_BIT && gp->gt_ini == -ARN)) ? gp->gt_new
							/* CH_BIT is int if ARN else bit */    : gp->gt_val < 0 ? 1
														: 0;
						    if (debug & 04) fprintf(outFP, "%4d %-15s %d\n",
							index, gp->gt_ids, value);	/* only INT_MAX != 32767 */
						    if (value) {
							liveData(gp->gt_live, value);	/* initial active live values */
						    }
						}
						/********************************************************************
						 *  Send live data collected in msgBuf during RECEIVE_ACTIVE_TELEGRAMS
						 *  because scan() is only executed when another input occurs.
						 *  a scan may occurr between received blocks which
						 *  also sends a live C_channel:3 block
						 *******************************************************************/
						if (msgOffset > liveOffset) {
						    send_msg_to_server(sockFN, msgBuf);
						    if (micro) microReset(0);
						}
						msgOffset = liveOffset;		/* msg = "C_channel:3" */
						if (val == 4) {
						    liveFlag = 1;		/* live inhibit bits are correct */
						}
						break;

					    case 5:		/* GET_END */
						for (opp = sTable; opp < sTend; opp++) {
						    (*opp)->gt_live &= 0x7fff;	/* clear live active */
						}
						liveFlag = 0;
						printf("Symbol Table no longer required by '%s'\n", iccNM);
						/* leave '0' for iCserver */
						regOffset = snprintf(regBuf, REQUEST, "%d:0", C_channel);
						send_msg_to_server(sockFN, regBuf);
						if (micro) microReset(0);
						break;

					    case 6:		/* STOP PROGRAM */
						fprintf(errFP, "\n'%s' stopped by debugger\n", iccNM);
						quit(0);			/* quit normally */
						break;

					    case 7:		/* DEBUGGER STOPPED */
						fprintf(errFP, "\nDebugger has stopped\n", iccNM);
						regOffset = snprintf(regBuf, REQUEST, "%d:2;%s", C_channel, iccNM);
						if (micro) microPrint("Send application name", 0);
						send_msg_to_server(sockFN, regBuf);	/* Application Name for next debugger start */
						break;

					    default:
						goto RcvError;		/* unknown C_channel:? case */
					    }
					} else goto RcvError;
				    }
				} else {
				    fprintf(errFP, "n = %d, channel = %d, maxChannels = %d, gp = %s\n", n, slot, maxChannels, gp ? gp->gt_ids : "null");
				    goto RcvError;
				}
			    } while ((cp = strchr(cp, ',')) != NULL);
			}
#endif	/* LOAD */
			else {
			  RcvError:
			    fprintf(errFP, "ERROR: '%s' rcvd at '%s' ???\n", rpyBuf, iccNM);
			}
		    } else {
			fprintf(errFP, "\n'%s' disconnected by server\n", iccNM);
			quit(0);			/* quit normally */
		    }
		} else if (FD_ISSET(0, &rdfds)) {	/* unlikely to have both */
		    /********************************************************************
		     *  STDIN
		     *******************************************************************/
		    if ((c = getchar()) == EOF) {
			FD_CLR(0, &infds);		/* ignore EOF - happens in bg */
		    } else if (c == 'q') {
			fprintf(errFP, "\n'%s' stopped from terminal\n", iccNM);
			quit(0);			/* quit normally */
#if	YYDEBUG
		    } else if (c == 't') {
			debug ^= 0100;			/* toggle -t flag */
		    } else if (c == 'm') {
			micro++;			/* toggle more micro */
			if (micro >= 3) micro = 0;
		    } else if (debug & 0300) {		/* osc or detailed info */
			unsigned short flag = xflag;
			if (c == 'x') {
			    flag = 1;			/* hexadecimal output */
			} else if (c == 'd') {
			    flag = 0;			/* decimal output */
			}
			if (flag != xflag) {
			    xflag = flag;
			    display();			/* inputs and outputs */
			}
#endif	/* YYDEBUG */
		    }
		    /* ignore the rest for now */
		}
	    } else {				/* retval -1 */
		perror("ERROR: select failed");
		quit(1);
	    }
	    /* if many inputs change simultaneously increase oscillator limit */
	    osc_lim = (cnt << 1) + 1;			/* (cnt * 2) + 1 */
	    if (osc_lim < osc_max) {
		osc_lim = osc_max;			/* cnt = 1, osc_lim = 3 default */
	    }
	}
    } /* for ( ; ; ) */
} /* icc */

/********************************************************************
 *
 *	Build a live data message during scans
 *
 *******************************************************************/

void
#if	INT_MAX == 32767 && defined (LONG16)
liveData(unsigned short index, long value)
#else	/* INT_MAX == 32767 && defined (LONG16) */
liveData(unsigned short index, int value)
#endif	/* INT_MAX == 32767 && defined (LONG16) */
{
    int len;
    int rest;
    while ((len =
#if	INT_MAX == 32767 && defined (LONG16)
	    snprintf(&msgBuf[msgOffset], rest = REQUEST - msgOffset,
		";%hu %ld", index & 0x7fff, value)
#else	/* INT_MAX == 32767 && defined (LONG16) */
	    snprintf(&msgBuf[msgOffset], rest = REQUEST - msgOffset,
		";%hu %d", index & 0x7fff, value)
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	    ) < 0 || len >= rest
	) {
	/********************************************************************
	 *  Send live data collected in msgBuf before it overflows to iCserver
	 *******************************************************************/
	msgBuf[msgOffset] = '\0';		/* terminate */
	send_msg_to_server(sockFN, msgBuf);
	msgOffset = liveOffset;				/* msg = "C_channel:3" */
    }
    msgOffset += len;
} /* liveData */
#if	YYDEBUG

/********************************************************************
 *
 *	Display inputs & outputs
 *
 *******************************************************************/

static void
display(void)
{
    int			n;
    Gate *		gp;
    unsigned short	data;

    if (dis_cnt++ >= DIS_MAX) {		/* display header line */
	dis_cnt = 1;
	if ((gp = IX_[0]) != 0) {
	    for (n = 0; n < MAX_IO; n++) {
		fprintf(outFP, " I%d", n);
	    }
	}
	if (IB_[1] != 0) fprintf(outFP, "  IB1");
	if (IW_[2] != 0) fprintf(outFP, "    IW2");
#if	INT_MAX != 32767 || defined (LONG16)
	if (IL_[4] != 0) fprintf(outFP, "      IL4");
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	fprintf(outFP, "   ");
	for (n = 0; n < MAX_IO; n++) {
	    fprintf(outFP, " Q%d", n);
	}
	fprintf(outFP, "  QB1    QW2");
#if	INT_MAX != 32767 || defined (LONG16)
	fprintf(outFP, "      QL4");
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	fprintf(outFP, "\n");
    }
    if ((gp = IX_[0]) != 0) {
	data = gp->gt_new;
	for (n = 0; n < MAX_IO; n++) {
	    fprintf(outFP, "  %c", (data & 0x0001) ? '1' : '0');
	    data >>= 1;				/* scan input bits */
	}
    }
    /* display IB1, IW2 and IL4 if active */
    if (!xflag) {
	if ((gp = IB_[1]) != 0) fprintf(outFP, " %4d", gp->gt_new & 0xff);
	if ((gp = IW_[2]) != 0) fprintf(outFP, " %6hd", (short)gp->gt_new);
#if	INT_MAX == 32767
#if	defined (LONG16)
	if ((gp = IL_[4]) != 0) fprintf(outFP, " %8ld", gp->gt_new);
#endif	/* defined (LONG16) */
#else	/* INT_MAX == 32767 */
	if ((gp = IL_[4]) != 0) fprintf(outFP, " %8d", gp->gt_new);
#endif	/* INT_MAX == 32767 */
    } else {
	if ((gp = IB_[1]) != 0) fprintf(outFP, "   %02x", gp->gt_new & 0xff);
	if ((gp = IW_[2]) != 0) fprintf(outFP, "   %04x", gp->gt_new & 0xffff);
#if	INT_MAX == 32767
#if	defined (LONG16)
	if ((gp = IL_[4]) != 0) fprintf(outFP, " %08lx", gp->gt_new);
#endif	/* defined (LONG16) */
#else	/* INT_MAX == 32767 */
	if ((gp = IL_[4]) != 0) fprintf(outFP, " %08x", gp->gt_new);
#endif	/* INT_MAX == 32767 */
    }
    fprintf(outFP, "   ");
    data = *(unsigned short*)QX_;
    for (n = 0; n < MAX_IO; n++) {
	fprintf(outFP, "  %c", (data & 0x0001) ? '1' : '0');
	data >>= 1;				/* scan output bits */
    }
    /* display QB1, QW2 and QL4 */
    if (!xflag) {
#if	INT_MAX == 32767
#if	defined (LONG16)
	fprintf(outFP, " %4d %6hd %8ld\n", QX_[1], *(short*)&QX_[2], *(long*)&QX_[4]);
#else	/* defined (LONG16) */
	fprintf(outFP, " %4d %6hd\n", QX_[1], *(short*)&QX_[2]);	/* no QL4 */
#endif	/* defined (LONG16) */
#else	/* INT_MAX == 32767 */
	fprintf(outFP, " %4d %6hd %8d\n", QX_[1], *(short*)&QX_[2], *(int*)&QX_[4]);
#endif	/* INT_MAX == 32767 */
    } else {
#if	INT_MAX == 32767
#if	defined (LONG16)
	fprintf(outFP, "   %02x   %04hx %08lx\n", QX_[1], *(short*)&QX_[2], *(long*)&QX_[4]);
#else	/* defined (LONG16) */
	fprintf(outFP, "   %02x   %04hx\n", QX_[1], *(short*)&QX_[2]);	/* no QL4 */
#endif	/* defined (LONG16) */
#else	/* INT_MAX == 32767 */
	fprintf(outFP, "   %02x   %04hx %08x\n", QX_[1], *(short*)&QX_[2], *(int*)&QX_[4]);
#endif	/* INT_MAX == 32767 */
    }
    fflush(outFP);
} /* display */
#endif	/* YYDEBUG */

/********************************************************************
 *
 *	Initialize IO
 *
 *******************************************************************/

void initIO(void)
{
    signal(SIGSEGV, quit);		/* catch memory access signal */	
}

/********************************************************************
 *
 *	Quit program with 'q' or ctrlC or Break via signal SIGINT
 *	or program abort on detected bugs.
 *
 *******************************************************************/

void quit(int sig)
{
    struct timespec ms200 = { 0, 200000000, };

    if (sockFN) {
	if (C_channel) {
	    /* disconnect iClive - follow with '0' for iCserver */
	    regOffset = snprintf(regBuf, REQUEST, "%d:5,%d:0", C_channel, C_channel);
	    send_msg_to_server(sockFN, regBuf);
	    if (micro) microReset(0);
	    nanosleep(&ms200, NULL);
	}
	close(sockFN);			/* close connection to iCserver */
    }
    fflush(outFP);
    if (sig == SIGINT) {
	fprintf(errFP, "\n'%s' stopped by interrupt from terminal\n", iccNM);
    } else if (sig == SIGSEGV) {
	fprintf(errFP, "\n'%s' stopped by 'Invalid memory reference'\n", iccNM);
    }
    if (errFP != stderr) {
	fflush(errFP);
	fclose(errFP);
    }
#ifdef	EFENCE
    free(rpyBuf);
    free(outBuf);
    free(regBuf);
    free(msgBuf);
#endif	/* EFENCE */
    exit(sig);			/* really quit */
} /* quit */

static const char ict_c[] =
"@(#)$Id: ict.c,v 1.44 2005/01/26 15:16:30 jw Exp $";
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
#include	"icc.h"

#define MAX_W	2
#define INTR 0x1c    /* The clock tick interrupt */

#define D10	10			/* 1/10 second select timeout under Linux */
#define ENTER	'\n'

iC_Functp	*iC_i_lists[] = { I_LISTS };

static Gate	alist0;			/* these lists are toggled */
static Gate	alist1;
Gate *		iC_a_list;		/* arithmetic output action list */
static Gate	olist0;			/* these lists are toggled */
static Gate	olist1;
Gate *		iC_o_list;		/* logic output action list */
Gate *		iC_c_list;		/* main clock list "iClock" */
static Gate	flist;
Gate *		iC_f_list;		/* auxiliary function clock list */
static Gate	slist;
Gate *		iC_s_list;		/* send bit and byte outputs */

#if	YYDEBUG
static int	dis_cnt;
#endif	/* YYDEBUG */
short		iC_error_flag;

unsigned	iC_scan_cnt;		/* count scan operations */
unsigned	iC_link_cnt;		/* count link operations */
#if	YYDEBUG && (!defined(_WINDOWS) || defined(LOAD))
unsigned	iC_glit_cnt;		/* count glitches */
unsigned long	glit_nxt;		/* count glitch scan */
#endif	/* YYDEBUG && (!defined(_WINDOWS) || defined(LOAD)) */

static int	sockFN = 0;		/* TCP/IP socket file number */

#ifndef	LOAD	// OLD I/O
unsigned char	iC_QM_[IXD/8];		/* Output slot mask per cage */
unsigned char	iC_QMM;			/* Output cage mask for 1 rack */
unsigned char	iC_pdata[IXD];		/* input differences */
#else		// NEW I/O
static Gate *	timNull[] = { 0, 0, 0, 0, 0, 0, 0, 0, }; /* speeds up tim[] lookup */
static int	maxChannels = 0;
static int	C_channel = 0;		/* channel for sending messages to Debug */
static Gate	D_gate;			/* Gate on channel for receiving messages from Debug */
static int	regOffset = 0;		/* for register send */
static int	liveOffset = 0;		/* for live header */
#ifndef	EFENCE
static char	regBuf[REQUEST];
#else	/* EFENCE */
static char *	regBuf;
#endif	/* EFENCE */
#endif		// END NEW I/O

static int	msgOffset = 0;		/* for message send */
#ifndef	EFENCE
static char	msgBuf[REQUEST];
char		iC_outBuf[REQUEST];	/* Buffer in which output is collected in rsff.c */
char		rpyBuf[REPLY];		/* Buffer in which iCserver input is read to */
#else	/* EFENCE */
static char *	msgBuf;
char *		iC_outBuf;
static char *	rpyBuf;
#endif	/* EFENCE */
int		iC_outOffset;

/********************************************************************
 *
 *	Procedure iC_icc
 *
 *******************************************************************/

void
iC_icc(
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
    Gate **	tim = timNull;		/* point to array of 8 null pointers */
#else	/* LOAD */
    unsigned *	gcp;
    iC_Functp * *	ilp;
    Gate **	tim = &iC_TX_[0];	/* pointers to 8 system gates TX[0] to TX[7] */
#endif	/* LOAD */
    Gate *	gp;
    iC_Functp	init_fa;
    int		tcnt = 1;
    float	delay = 0.0;		/* timer processing stopped */
    int		retval;
#ifdef	EFENCE
    msgBuf = iC_emalloc(REQUEST);
#ifdef	LOAD
    regBuf = iC_emalloc(REQUEST);
#endif	/* LOAD */
    iC_outBuf = iC_emalloc(REQUEST);
    rpyBuf = iC_emalloc(REPLY);
#endif	/* EFENCE */
    iC_initIO();			/* catch memory access signal */
    if (iC_outFP != stdout) {
	fclose(iC_outFP);
#ifndef LOAD
	if (iFlag) {
	    iC_inversionCorrection();
	    iFlag = 0;
	}
#endif	/* LOAD */
	iC_outFP = stdout;		/* standard output from here */
    }
    if (iC_errFP != stderr) {
	fclose(iC_errFP);
	iC_errFP = stderr;		/* standard error from here */
    }

#ifdef	LOAD
    if ((gp = iC_TX0p) != 0) {		/* are EOP or TX0 timers programmed */
	tim = gp->gt_list;		/* TX0.0 - TX0.7 */
#endif	/* LOAD */
	assert(tim);
	for (cnt = 1; cnt < 8; cnt++) {
	    if (tim[cnt] != NULL) {	/* any of the 7 timers programmed ? */
		delay = iC_timeout;	/* yes - setup timer processing */
		break;			/* could optimise by varying delay */
	    }
	}
#ifdef	LOAD
    }
#endif	/* LOAD */

    if ((iC_debug & 0400) == 0) {
	/* Start TCP/IP communication before any inputs are generated => outputs */
	if (iC_micro) iC_microReset(04);
	sockFN = iC_connect_to_server(iC_hostNM, iC_portNM, delay);
#ifndef	LOAD
	snprintf(msgBuf, REQUEST, "%s %d", "C0", IXD);
	iC_send_msg_to_server(sockFN, msgBuf);	/* register C0 with IXD as maxIOs */
	if (iC_micro) iC_microPrint("connect to server", 0);
#else	/* LOAD */
	char *		tbp = regBuf;
	int		tbc = REQUEST;
	int		cn  = 0;
	char *		tbt;
	const char *	sr[] = { "N", ",SC", ",RD", };	/* name, controller, debugger */

	if (iC_debug & 04) fprintf(iC_outFP, "%s: I/O registration objects\n", iC_iccNM);
	for (i = 0; i < 3; i++) {
	    tbp += cn;
	    tbc -= cn;
	    cn = snprintf(tbp, tbc, "%s%s", sr[i], iC_iccNM);
	}
	/* use last string to initialise name "D<progname>[-<instance>]" for D_gate */
	D_gate.gt_ids = iC_emalloc(cn-1);			/* including '\0' */
	strncpy(D_gate.gt_ids, tbp+2, cn-1);		/* +2 leave out ",R" */
	D_gate.gt_new = -2;	/* changes - not equal 0 - 5 for debug messages */
	D_gate.gt_old = -1;	/* never changes - not equal 0 - 5 for debug messages */

	for (opp = iC_sTable; opp < iC_sTend; opp++) {
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
			fprintf(iC_errFP, "\n%s: illformed output name '%s'\n", iC_iccNM, tbp + 1);
			iC_quit(3);
		    }
		}
	    }
	    if (cn > 0) {
		if (iC_debug & 04) fprintf(iC_outFP,
		    "%s	%d	%d\n", gp->gt_ids, (int)gp->gt_ini, (int)gp->gt_fni);
		if (strlen(iC_iidNM) > 0) {
		    cn += snprintf(tbp + cn, tbc - cn, "-%s", iC_iidNM); /* append instance id */
		}
	    }
	    if (cn >= tbc) {
		fprintf(iC_errFP, "\n%s: buffer '%s' has overflowed %d %d\n", iC_iccNM, regBuf, cn, tbc);
		iC_quit(4);
	    }
	}
	if (iC_debug & 04) fprintf(iC_outFP, "register:%s\n", regBuf);
	iC_send_msg_to_server(sockFN, regBuf);		/* register controller and IOs */
	if (iC_micro) iC_microPrint("connect to server", 0);
	/* receive registration reply */
	if (iC_rcvd_msg_from_server(sockFN, rpyBuf, REPLY)) {
	    if (iC_micro) iC_microPrint("reply from server", 0);
	    if (iC_debug & 04) fprintf(iC_outFP, "reply:%s\n", rpyBuf);
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
	    for (opp = iC_sTable; opp < iC_sTend; opp++) {
		gp = *opp;
		if ((gp->gt_ini == -INPW && *gp->gt_ids != 'T') ||	/* suppress TX0 */
		    gp->gt_fni == OUTW) {
		    assert(cp);
		    index = atoi(++cp);
		    assert(index > 0 && index < maxChannels);
		    if (gp->gt_ini == -INPW) {
			Channels[index] = gp;				/* ==> Input */
		    } else
		    if (gp->gt_fni == OUTW) {
			gp->gt_list = (Gate **)index;			/* <== Output */
		    }
		    cp = strchr(cp, ',');
		}
	    }
	    assert(cp == NULL);
	    if (iC_debug & 04) fprintf(iC_outFP, "reply: Channels %d\n", maxChannels);
	    regOffset = snprintf(regBuf, REQUEST, "%d:2;%s", C_channel, iC_iccNM);
	    if (iC_micro) iC_microPrint("Send application name", 0);
	    iC_send_msg_to_server(sockFN, regBuf);			/* Application Name */
	} else {
	    fprintf(iC_errFP, "\n'%s' disconnected by server\n", iC_iccNM);
	    iC_quit(0);			/* quit normally */
	}
#endif	/* LOAD */
    }

/********************************************************************
 *
 *	Initialise the work lists to empty lists
 *
 *******************************************************************/

    iC_error_flag = 0;
    alist0.gt_rlist = (Gate **)(iC_a_list = &alist1);	/* initialise alternate */
    Out_init(iC_a_list);
    alist1.gt_rlist = (Gate **)(iC_a_list = &alist0);	/* start with alist0 */
    Out_init(iC_a_list);
    olist0.gt_rlist = (Gate **)(iC_o_list = &olist1);	/* initialise alternate */
    Out_init(iC_o_list);
    olist1.gt_rlist = (Gate **)(iC_o_list = &olist0);	/* start with olist0 */
    Out_init(iC_o_list);
#ifdef	LOAD
    iC_c_list = &iClock;				/* system clock list */
    /* TODO check c_list is a propoer list even if DEQ (also other places) */
#endif	/* LOAD */
    iC_f_list = &flist;					/* function clock list */
    Out_init(iC_f_list);
    iC_s_list = &slist;					/* send outputs */
    Out_init(iC_s_list);
    /********************************************************************
     *  Carry out 4 Passes to initialise all Gates
     *******************************************************************/
#if	YYDEBUG
    if (iC_debug & 0100) fprintf(iC_outFP, "\nINITIALISATION\n");
#endif	/* YYDEBUG */

#ifdef	LOAD 
    /* if (iC_debug & 0400) == 0 then no live bits are set in gt_live | 0x8000 */
    /* header for live data */
    /********************************************************************
     *  Initialise msgBuf for live data collection before initialisation
     *  liveOffset may also be used with messages in regBuf because they
     *  use the same channel number and all relevant messages have only
     *  one command digit.
     *  Message in msgBuf is ignored of iC_debug&0400 stops process after init
     *******************************************************************/
    msgOffset = liveOffset = snprintf(msgBuf, REQUEST, "%d:3", C_channel);
#endif	/* LOAD */

#ifndef	LOAD
    for (i = 0; i < IXD; i++) {		/* clear output array used to hold */
	iC_QX_[i] = 0;			/* output size X, B or W during compilation */
    }
#endif	/* LOAD */
    for (pass = 0; pass < 4; pass++) {
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, "\nPass %d:", pass + 1);
#endif	/* YYDEBUG */
#ifdef	LOAD
	for (opp = iC_sTable; opp < iC_sTend; opp++) {
	    gp = *opp;
	    typ = gp->gt_ini > 0 ? AND : -gp->gt_ini;
	    if (typ < MAX_OP) {
		init_fa = iC_i_lists[typ][pass];
		(*init_fa)(gp, typ);	/* initialise for this pass */
	    }
	}
#else	/* LOAD */
	gp = g_lists;
	gcp = gate_count;
	ilp = iC_i_lists;
	for (typ = 0; typ < MAX_OP; typ++) {
	    init_fa = (*ilp++)[pass];
	    for (cnt = *gcp++; cnt; cnt--) {
		(*init_fa)(gp++, typ);	/* initialise for this pass */
	    }
	}
#endif	/* LOAD */
    }

#if	YYDEBUG
    if (iC_debug & 0100) {
	fprintf(iC_outFP, "\nInit complete =======\n");
    }
#endif	/* YYDEBUG */

    if (iC_error_flag) {
	if (iC_error_flag == 1) {
	    fprintf(iC_outFP, "\n*** Fatal Errors ***\n");
	    iC_quit(1);
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
     *  ctrl-C, when it has been brought to the foregroung with fg.
     *******************************************************************/
    signal(SIGTTIN, SIG_IGN);		/* ignore tty input signal in bg */
#endif	/* SIGTTIN */

    if ((gp = tim[0]) != NULL) {
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, "\nEOP:\t%s  1 ==>", gp->gt_ids);
#endif	/* YYDEBUG */
	gp->gt_val = -1;		/* set EOP once as first action */
	iC_link_ol(gp, iC_o_list);			/* fire EOP Input Gate */
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, " -1");
#endif	/* YYDEBUG */
    }

#if	YYDEBUG
    dis_cnt = DIS_MAX;
#endif	/* YYDEBUG */
    /********************************************************************
     *  Operational loop
     *******************************************************************/
    for ( ; ; ) {
	if (iC_micro & 06) iC_microPrint("Input", 0);
	if (++iC_mark_stamp == 0) {	/* next generation for check */
	    iC_mark_stamp++;		/* leave out zero */
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
	if (iC_a_list != iC_a_list->gt_next) { iC_scan_ar (iC_a_list);            }
	if (iC_o_list != iC_o_list->gt_next) { iC_scan    (iC_o_list); goto Loop; }
	if (iC_c_list != iC_c_list->gt_next) { iC_scan_clk(iC_c_list); goto Loop; }
	if (iC_f_list != iC_f_list->gt_next) { iC_scan_clk(iC_f_list); goto Loop; }
	if (iC_s_list != iC_s_list->gt_next) { iC_scan_snd(iC_s_list);            }

#ifdef	LOAD  // ########## NEW Input
	/********************************************************************
	 *  Send live data collected in msgBuf during initialisation
	 *  and previous loop to iCserver
	 *******************************************************************/
	if (msgOffset > liveOffset) {
	    iC_send_msg_to_server(sockFN, msgBuf);
	    if (iC_micro) iC_microReset(0);
	}
	/********************************************************************
	 *  Initialise msgBuf for live data collection during next loop
	 *******************************************************************/
	msgOffset = liveOffset;				/* msg = "C_channel:3" */
#endif	/* LOAD */ // ########## END NEW Input

#if	YYDEBUG
	if (iC_scan_cnt || iC_link_cnt) {
	    if (iC_debug & 0100) fprintf(iC_outFP, "\n");
	    if (iC_debug & 02000) {
		fprintf(iC_outFP, "scan = %5u  link = %5u",
		    iC_scan_cnt, iC_link_cnt);
		if (iC_glit_cnt) {
		    fprintf(iC_outFP, "  glitch = %5u, %lu", iC_glit_cnt, glit_nxt);
		}
		fprintf(iC_outFP, "\n");
	    }
	    iC_glit_cnt = glit_nxt =
	    iC_scan_cnt = iC_link_cnt = 0;
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
	iC_a_list = (Gate*)iC_a_list->gt_rlist;	/* alternate arithmetic list */
	iC_o_list = (Gate*)iC_o_list->gt_rlist;	/* alternate logic list */

	if (iC_osc_gp) {
	    fprintf(iC_outFP,
		"*** Warning: %s has oscillated %d times - check iC program!!!\n",
		iC_osc_gp->gt_ids, iC_osc_gp->gt_mcnt);
	    iC_osc_gp = NULL;
	}

#if	YYDEBUG
	if (iC_debug & 0300) {		/* osc or detailed info */
	    if ((iC_debug & 0200) &&
		(iC_o_list->gt_next != iC_o_list || iC_a_list->gt_next != iC_a_list)) {
		fprintf(iC_outFP, "OSC =");
		for (gp = iC_a_list->gt_next; gp != iC_a_list; gp = gp->gt_next) {
		    fprintf(iC_outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
		}
		for (gp = iC_o_list->gt_next; gp != iC_o_list; gp = gp->gt_next) {
		    fprintf(iC_outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
		}
		fprintf(iC_outFP, "\n");
	    }
	    iC_display(&dis_cnt, DIS_MAX);	/* inputs and outputs */
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
	while (iC_QMM) {			/* rack with bit set for every cage */
	    int len;
	    int rest;
	    int mask = iC_QMM & -iC_QMM;	/* rightmost cage set in QMM */
	    int cage = iC_bitIndex[mask];	/* mask has only 1 bit set */
	    int slots = iC_QM_[cage];
	    int cageOffset = cage << 3;		/* cage has 8 slots */
	    while (slots) {
		int mask = slots & -slots;	/* rightmost slot set in slots */
		int slot = cageOffset + iC_bitIndex[mask];
#if INT_MAX == 32767 && defined (LONG16)
		long val;
#else
		int val;
#endif
		char Qtype = iC_QT_[slot];
		assert(Qtype);			/* make sure slot is programmed */
		val = Qtype == 'L' ? *(long*)&iC_QX_[slot] :
		      Qtype == 'W' ? *(short*)&iC_QX_[slot] : iC_QX_[slot];
		while ((len =
#if INT_MAX == 32767 && defined (LONG16)
			snprintf(&msgBuf[msgOffset], rest = REQUEST - msgOffset,
			    "%c%d.%ld,", Qtype, slot, val)
#else
			snprintf(&msgBuf[msgOffset], rest = REQUEST - msgOffset,
			    "%c%d.%d,", Qtype, slot, val)
#endif
			) < 0 || len > rest	/* use > because of -1 truncation */
		    ) {
		    msgBuf[msgOffset - 1] = '\0';	/* clear last ',' */
		    if (iC_micro) iC_microPrint("Send intermediate", 0);
		    iC_send_msg_to_server(sockFN, msgBuf);
		    if (iC_micro) iC_microReset(0);
		    msgOffset = 0;
		}
		msgOffset += len;
		slots &= ~mask;			/* clear rightmost slot in slots */
	    }
	    iC_QM_[cage] = slots;		/* clear QM_[cage] */
	    iC_QMM &= ~mask;			/* clear rightmost cage in QMM */
	}
	if (msgOffset > 0) {
	    msgBuf[msgOffset - 1] = '\0';	/* clear last ',' */
	    if (iC_micro) iC_microPrint("Send", 0);
	    iC_send_msg_to_server(sockFN, msgBuf);
	    if (iC_micro) iC_microReset(0);
	}
#else	/* LOAD */

	/********************************************************************
	 *  Send output data collected in outBuf to iCserver
	 *******************************************************************/
	if (iC_outOffset > 1) {		/* if any item it is at least 2 long 'x,' */
	    iC_outBuf[iC_outOffset - 1] = '\0';	/* clear last ',' */
	    if (iC_micro) iC_microPrint("New Send", 0);
	    iC_send_msg_to_server(sockFN, iC_outBuf);
	    memset(iC_outBuf, 0, REQUEST);	/* clear for next data from outMw() */
	    iC_outOffset = 0;
	    if (iC_micro) iC_microReset(0);
	}
#endif	/* LOAD */ // ########## END NEW Input

	/********************************************************************
	 *  Input from external input modules and time input if used
	 *  Wait for input in a select statement most of the time
	 *
	 *  The iC_debug facility -d1000 stops linking the 100 msec and 1 second
	 *  TIMERS tim[4] and tim[5] when they are connected directly to
	 *  the slave input of a clock or timer Gate and that clock or timer
	 *  has no Gates on the clock list it controls - ie nothing will happen.
	 *  This stops continuous output when tracing logic (-d100 or -t) when
	 *  these internal TIMERS produce no change.
	 *  It also reduces the data traffic to iClive and reduces flashing of
	 *  clocks and timers controlled by internal TIMERS.
	 *******************************************************************/
	for (cnt = 0; cnt == 0; ) {		/* stay in input loop if nothing linked */
	    retval = iC_wait_for_next_event(sockFN);	/* wait for inputs or timer */
	    if (iC_micro & 02) iC_microReset(0);

	    if (retval == 0) {
		iC_timeoutCounter = iC_timeoutValue;	/* will repeat if timeout with input */
		/********************************************************************
		 *  TIMERS here every 100 milliseconds
		 *******************************************************************/
		if ((gp = tim[4]) != 0) {	/* 100 millisecond timer */
#if	YYDEBUG
		    if (iC_debug & 01000) {
			Gate **	lp;
			Gate *	tp;
			int	cn1 = 2;
			lp = gp->gt_list;	/* test if any clock or timer is active */
			do {			/* for normal and inverted */
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
		    if (iC_debug & 0100) fprintf(iC_outFP, " %s ", gp->gt_ids);
#endif	/* YYDEBUG */
		    gp->gt_val = - gp->gt_val;	/* complement input */
		    iC_link_ol(gp, iC_o_list);
		    cnt++;
#if	YYDEBUG
		    if (iC_debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', iC_outFP);
		skipT4: ;
#endif	/* YYDEBUG */
		}
		if (--tcnt <= 0) {
		    tcnt = D10;
		    if ((gp = tim[5]) != 0) {	/* 1 second timer */
#if	YYDEBUG
			if (iC_debug & 01000) {
			    Gate **	lp;
			    Gate *	tp;
			    int	cn1 = 2;
			    lp = gp->gt_list;	/* test if any clock or timer is active */
			    do {		/* for normal and inverted */
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
			if (iC_debug & 0100) fprintf(iC_outFP, " %s ", gp->gt_ids);
#endif	/* YYDEBUG */
			gp->gt_val = - gp->gt_val;	/* complement input */
			iC_link_ol(gp, iC_o_list);
			cnt++;
#if	YYDEBUG
			if (iC_debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', iC_outFP);
		    skipT5: ;
#endif	/* YYDEBUG */
		    }
		}
	    } else if (retval > 0) {
		if (FD_ISSET(sockFN, &iC_rdfds)) {
		    /********************************************************************
		     *  TCP/IP input
		     *******************************************************************/
		    if (iC_micro) iC_microReset(04);
		    if (iC_rcvd_msg_from_server(sockFN, rpyBuf, REPLY)) {
#if	INT_MAX == 32767 && defined (LONG16)
			long		val;
#else	/* INT_MAX == 32767 && defined (LONG16) */
			int		val;
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#ifndef	LOAD	// ########## OLD Input
			int		mask;
			int		diff;
			int		slot;
			char		utype[4];	/* need 2 with NUL */

			if (
#if	INT_MAX == 32767 && defined (LONG16)
				sscanf(rpyBuf, "%1[XBWLD]%d.%ld", utype, &slot, &val)
#else	/* INT_MAX == 32767 && defined (LONG16) */
				sscanf(rpyBuf, "%1[XBWLD]%d.%d", utype, &slot, &val)
#endif	/* INT_MAX == 32767 && defined (LONG16) */
				== 3 && slot < IXD
			    ) {
			    switch (*utype) {
			    case 'X':			/* bit input */
				val &= 0xff;		/* safety measure */
				diff = val ^ iC_pdata[slot];
				iC_pdata[slot] = val;	/* ready for next scan */
				slot <<= 3;		/* convert to bit index */
				while (diff) {		/* age old algorithm from CSR days */
				    mask = diff & -diff;	/* rightmost bit set in diff */
				    index = iC_bitIndex[mask];	/* mask has only 1 bit set */
				    /* ignore Gate if not programmed  or no change in bit */
				    if ((gp = iC_IX_[slot+index]) != 0 &&
					((gp->gt_val & 0x80) ^ (val & mask ? 0x80 : 0x00))) {
					/* relies on input initialized to +1 !!!!!! */
					/* and no other function modifies gp_gt_val */
					gp->gt_val = - gp->gt_val;	/* complement input */
#if YYDEBUG
					if (iC_debug & 0100) fprintf(iC_outFP, " %s ", gp->gt_ids);
#endif
					iC_link_ol(gp, iC_o_list);	/* fire Input Gate */
#if YYDEBUG
					if (iC_debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', iC_outFP);
#endif
					cnt++;
					/* o_list will be scanned before next input */
				    }
				    diff &= ~mask;	/* clear rightmost bit in diff */
				}			/* loops only once for every 1 in diff */
				break;

			    case 'B':			/* 8 bit byte input */
				if ((gp = iC_IB_[slot]) != NULL) {
				    val &= 0xff;	/* reduce to byte */
				    goto wordIn;
				}
				break;

			    case 'W':			/* 16 bit word input */
				if ((gp = iC_IW_[slot]) != NULL) {
				    val = (short)val;	/* reduce to signed word */
#if	INT_MAX != 32767 || defined (LONG16)
				    goto wordIn;
				}
				break;

			    case 'L':			/* 32 bit long input */
				if ((gp = iC_IL_[slot]) != NULL) {
#endif	/* INT_MAX == 32767 && defined (LONG16) */
			    wordIn:
				    if (val != gp->gt_new &&	/* first change or glitch */
				    ((gp->gt_new = val) != gp->gt_old) ^ (gp->gt_next != 0)) {
					/* arithmetic master action */
#if	YYDEBUG
					if (iC_debug & 0100) fprintf(iC_outFP, "%s[\t", gp->gt_ids);
#endif	/* YYDEBUG */
					iC_link_ol(gp, iC_a_list);	/* no actions */
#if	YYDEBUG
#if	INT_MAX == 32767 && defined (LONG16)
					/* TODO - format for byte, word or long */
					if (iC_debug & 0100) fprintf(iC_outFP, "%ld", gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
					if (iC_debug & 0100) fprintf(iC_outFP, "%d", gp->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#endif	/* YYDEBUG */
					cnt++;
				    }
				}
				break;

			    default:
				goto RcvError;		/* unkikely if sscanf %1[XBWLD] OK */
			    }
			}
#else	/* LOAD */ // ########## NEW Input
			int		count;
			int		channel;
			static int	liveFlag;

			if (isdigit(rpyBuf[0])) {
			    cp = rpyBuf - 1;
			    do {
				int n;
				if (
#if	INT_MAX == 32767 && defined (LONG16)
				    (n = sscanf(++cp, "%d:%ld", &channel, &val))
#else	/* INT_MAX == 32767 && defined (LONG16) */
				    (n = sscanf(++cp, "%d:%d", &channel, &val))
#endif	/* INT_MAX == 32767 && defined (LONG16) */
				    == 2 &&
				    channel > 0	&&
				    channel < maxChannels &&
				    (gp = Channels[channel]) != NULL
				) {
				    if (val != gp->gt_new &&	/* first change or glitch */
				    ((gp->gt_new = val) != gp->gt_old) ^ (gp->gt_next != 0)) {
					/* arithmetic master action */
					if (gp->gt_fni == TRAB) {
#if	YYDEBUG
					    if (iC_debug & 0100) fprintf(iC_outFP, "%s<\t", gp->gt_ids);
#endif	/* YYDEBUG */
					    count = iC_traMb(gp, 0);	/* distribute bits directly */
#if	YYDEBUG
					    if (iC_debug & 0100) fprintf(iC_outFP, "%d%s",
							    gp->gt_new, count ? "" : "\n");
#endif	/* YYDEBUG */
					    cnt += count;	/* extra number of fired gates */
					} else
					if (gp->gt_ini == -INPW) {
#if	YYDEBUG
					    if (iC_debug & 0100) fprintf(iC_outFP, "%s[\t", gp->gt_ids);
#endif	/* YYDEBUG */
					    iC_link_ol(gp, iC_a_list);	/* no actions */
#if	YYDEBUG
#if	INT_MAX == 32767 && defined (LONG16)
					    /* TODO - format for byte, word or long */
					    if (iC_debug & 0100) fprintf(iC_outFP, "%ld", gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
					    if (iC_debug & 0100) fprintf(iC_outFP, "%d", gp->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#endif	/* YYDEBUG */
					    cnt++;
					} else
					if (gp->gt_fni == UDFA) {	/* D_gate initialised type */
					    gp->gt_new = 0;	/* allow repeated 1-6 commands */
					    switch (val) {
					    case 0:		/* IGNORE */
						break;

					    case 1:		/* GET_SYMBOL_TABLE */
						printf("Symbol Table requested by '%s'\n", iC_iccNM);
						/* prepare index entries first to allow ALIAS back-references */
						index = 0;
						for (opp = iC_sTable; opp < iC_sTend; opp++) {
						    (*opp)->gt_live = index++;	/* index and live inhibit */
						    index &= 0x7fff;		/* rolls over if > 32768 Symbols */
						}
						regOffset = snprintf(regBuf, REQUEST, "%d:1", C_channel);
						/* to maintain index correlation send all symbols */
						for (opp = iC_sTable; opp < iC_sTend; opp++) {
						    int		len;
						    int		rest;
						    int		fni;
						    int		inverse = 0;
						    char *	ids;

						    gp = *opp;
						    ids = gp->gt_ids;
						    fni = gp->gt_fni;
						    while (gp->gt_ini == -ALIAS) {
							if (gp->gt_fni == GATE || gp->gt_fni == GATEX) {
							    inverse ^= gp->gt_mark;	/* holds ALIAS inversion flag */
							}
							gp = (Gate*)gp->gt_rlist;	/* resolve ALIAS */
							fni = MAX_FTY + gp->gt_fni + (inverse << 1);	/* match iClive */
						    }
						    if (fni == CH_BIT && gp->gt_ini >= 0) {
							fni = RI_BIT;	/* AND - LATCH XOR, display as logic signal */
						    }
						    while (rest = REQUEST - regOffset, (len = fni >= MAX_FTY ?
							snprintf(&regBuf[regOffset], rest, ";%s %d %d", ids, fni, gp->gt_live) :
							snprintf(&regBuf[regOffset], rest, ";%s %d", ids, fni)
							) < 0 || len >= rest) {
							regBuf[regOffset] = '\0';	/* terminate */
							if (iC_micro) iC_microPrint("Send Symbols intermediate", 0);
							iC_send_msg_to_server(sockFN, regBuf);
							if (iC_micro) iC_microReset(0);
							regOffset = liveOffset;
						    }
						    regOffset += len;
						}
						if (regOffset > liveOffset) {
						    if (iC_micro) iC_microPrint("Send Symbols", 0);
						    iC_send_msg_to_server(sockFN, regBuf);
						    if (iC_micro) iC_microReset(0);
						}
						/* end of symbol table - execute scan - follow with '0' to leave in iCserver */
						regOffset = snprintf(regBuf, REQUEST, "%d:4,%d:0", C_channel, C_channel);
						if (iC_micro) iC_microPrint("Send Scan Command", 0);
						iC_send_msg_to_server(sockFN, regBuf);
						liveFlag = 1;		/* live inhibit bits are set */
						if (iC_micro) iC_microReset(0);
						break;

					    case 3:		/* RECEIVE_ACTIVE_SYMBOLS */
					    case 4:		/* LAST_ACTIVE_SYMBOLS */
						if (liveFlag) {
						    for (opp = iC_sTable; opp < iC_sTend; opp++) {
							(*opp)->gt_live &= 0x7fff;	/* clear live active */
						    }
						    liveFlag = 0;	/* do not set again until case 4 received */
						}
						/********************************************************************
						 *  Send live data just in case there was also an input in this loop
						 *  (should go straight to scan())
						 *******************************************************************/
						if (msgOffset > liveOffset) {
						    iC_send_msg_to_server(sockFN, msgBuf);
						    if (iC_micro) iC_microReset(0);
						}
						msgOffset = liveOffset;		/* msg = "C_channel:3" */
						char * cp1 = rpyBuf;
						while ((cp1 = strchr(cp1, ';')) != NULL) {
						    long	value;
						    int		fni;
						    index = atoi(++cp1);
						    assert(index < iC_sTend - iC_sTable);	/* check index is in range */
						    gp = iC_sTable[index];
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
						    if (iC_debug & 04) fprintf(iC_outFP, "%4d %-15s %ld\n",
							index, gp->gt_ids, value);	/* only INT_MAX != 32767 */
						    if (value) {
							iC_liveData(gp->gt_live, value);	/* initial active live values */
						    }
						}
						/********************************************************************
						 *  Send live data collected in msgBuf during RECEIVE_ACTIVE_TELEGRAMS
						 *  because scan() is only executed when another input occurs.
						 *  a scan may occurr between received blocks which
						 *  also sends a live C_channel:3 block
						 *******************************************************************/
						if (msgOffset > liveOffset) {
						    iC_send_msg_to_server(sockFN, msgBuf);
						    if (iC_micro) iC_microReset(0);
						}
						msgOffset = liveOffset;		/* msg = "C_channel:3" */
						if (val == 4) {
						    liveFlag = 1;		/* live inhibit bits are correct */
						}
						break;

					    case 5:		/* GET_END */
						for (opp = iC_sTable; opp < iC_sTend; opp++) {
						    (*opp)->gt_live &= 0x7fff;	/* clear live active */
						}
						liveFlag = 0;
						printf("Symbol Table no longer required by '%s'\n", iC_iccNM);
						/* leave '0' for iCserver */
						regOffset = snprintf(regBuf, REQUEST, "%d:0", C_channel);
						iC_send_msg_to_server(sockFN, regBuf);
						if (iC_micro) iC_microReset(0);
						break;

					    case 6:		/* STOP PROGRAM */
						fprintf(iC_errFP, "\n'%s' stopped by debugger\n", iC_iccNM);
						iC_quit(0);			/* quit normally */
						break;

					    case 7:		/* DEBUGGER STOPPED */
						fprintf(iC_errFP, "\nDebugger has stopped\n");
						regOffset = snprintf(regBuf, REQUEST, "%d:2;%s", C_channel, iC_iccNM);
						if (iC_micro) iC_microPrint("Send application name", 0);
						iC_send_msg_to_server(sockFN, regBuf);	/* Application Name for next debugger start */
						break;

					    default:
						goto RcvError;		/* unknown C_channel:? case */
					    }
					} else goto RcvError;
				    }
				} else {
				    fprintf(iC_errFP, "n = %d, channel = %d, maxChannels = %d, gp = %s\n", n, channel, maxChannels, gp ? gp->gt_ids : "null");
				    goto RcvError;
				}
			    } while ((cp = strchr(cp, ',')) != NULL);
			}
#endif	/* LOAD */ // ########## END NEW Input
			else {
			  RcvError:
			    fprintf(iC_errFP, "ERROR: '%s' rcvd at '%s' ???\n", rpyBuf, iC_iccNM);
			}
		    } else {
			fprintf(iC_errFP, "\n'%s' disconnected by server\n", iC_iccNM);
			iC_quit(0);			/* quit normally */
		    }
		} else if (FD_ISSET(0, &iC_rdfds)) {	/* unlikely to have both */
		    /********************************************************************
		     *  STDIN
		     *******************************************************************/
		    if ((c = getchar()) == EOF) {
			FD_CLR(0, &iC_infds);		/* ignore EOF - happens in bg */
		    } else if (c == 'q') {
			fprintf(iC_errFP, "\n'%s' stopped from terminal\n", iC_iccNM);
			iC_quit(0);			/* quit normally */
#if	YYDEBUG
		    } else if (c == 't') {
			iC_debug ^= 0100;		/* toggle -t flag */
		    } else if (c == 'm') {
			iC_micro++;			/* toggle more micro */
			if (iC_micro >= 3) iC_micro = 0;
		    } else if (iC_debug & 0300) {	/* osc or detailed info */
			unsigned short flag = iC_xflag;
			if (c == 'x') {
			    flag = 1;			/* hexadecimal output */
			} else if (c == 'd') {
			    flag = 0;			/* decimal output */
			}
			if (flag != iC_xflag) {
			    iC_xflag = flag;
			    iC_display(&dis_cnt, DIS_MAX);	/* inputs and outputs */
			}
#endif	/* YYDEBUG */
		    }
		    /* ignore the rest for now */
		}
	    } else {				/* retval -1 */
		perror("ERROR: select failed");
		iC_quit(1);
	    }
	    /* if many inputs change simultaneously increase oscillator limit */
	    iC_osc_lim = (cnt << 1) + 1;	/* (cnt * 2) + 1 */
	    if (iC_osc_lim < iC_osc_max) {
		iC_osc_lim = iC_osc_max;	/* cnt = 1, osc_lim = 3 default */
	    }
	}
    } /* for ( ; ; ) */
} /* iC_icc */
#ifdef	LOAD

/********************************************************************
 *
 *	Build a live data message during scans
 *
 *******************************************************************/

void
#if	INT_MAX == 32767 && defined (LONG16)
iC_liveData(unsigned short index, long value)
#else	/* INT_MAX == 32767 && defined (LONG16) */
iC_liveData(unsigned short index, int value)
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
	iC_send_msg_to_server(sockFN, msgBuf);
	msgOffset = liveOffset;			/* msg = "C_channel:3" */
    }
    msgOffset += len;
} /* liveData */
#endif	/* LOAD */

/********************************************************************
 *
 *	Initialize IO
 *
 *******************************************************************/

void iC_initIO(void)
{
    signal(SIGSEGV, iC_quit);			/* catch memory access signal */
}

/********************************************************************
 *
 *	Quit program with 'q' or ctrlC or Break via signal SIGINT
 *	or program abort on detected bugs.
 *
 *******************************************************************/

void iC_quit(int sig)
{
    if (sockFN) {
#ifdef	LOAD
	if (C_channel) {
	    struct timespec ms200 = { 0, 200000000, };
	    /* disconnect iClive - follow with '0' for iCserver */
	    regOffset = snprintf(regBuf, REQUEST, "%d:5,%d:0", C_channel, C_channel);
	    iC_send_msg_to_server(sockFN, regBuf);
	    if (iC_micro) iC_microReset(0);
	    nanosleep(&ms200, NULL);
	}
#endif	/* LOAD */
	close(sockFN);				/* close connection to iCserver */
    }
    fflush(iC_outFP);
    if (sig == SIGINT) {
	fprintf(iC_errFP, "\n'%s' stopped by interrupt from terminal\n", iC_iccNM);
    } else if (sig == SIGSEGV) {
	fprintf(iC_errFP, "\n'%s' stopped by 'Invalid memory reference'\n", iC_iccNM);
    }
    if (iC_errFP != stderr) {
	fflush(iC_errFP);
	fclose(iC_errFP);
    }
#ifdef	EFENCE
    free(rpyBuf);
    free(iC_outBuf);
#ifdef	LOAD
    free(regBuf);
#endif	/* LOAD */
    free(msgBuf);
#endif	/* EFENCE */
    exit(sig);					/* really quit */
} /* iC_quit */

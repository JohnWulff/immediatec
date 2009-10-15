static const char icr_c[] =
"@(#)$Id: icr.c,v 1.38 2009/10/13 12:03:23 jw Exp $";
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
 *	icr.c
 *	parallel plc - runtime execution with stdio test input only
 *
 *******************************************************************/

/* J.E. Wulff	3-Mar-85 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<string.h>
#ifdef	_MSDOS_
#include	<dos.h>
#include	<conio.h>
#else	/* Linux */
#define getch() getchar()
#define ungetch(x) ungetc(x, stdin)
#include	<sys/types.h>
#include	<sys/time.h>
#include	<termios.h>
#include	<unistd.h>
#endif	/* Linux */
#include	<signal.h>
#include	<ctype.h>
#include	"icc.h"
#ifdef	LOAD
#error - must be compiled without LOAD defined to make an executable compiler
#endif	/* LOAD */

/***NOTE: This is an interrupt service
 routine. You cannot compile this program
 with Test Stack Overflow turned on and
 get an executable file that operates
 correctly. */

#define INTR	0x1c			/* The clock tick interrupt */
#define YSIZE	13			/* - and 10 dec digits or 12 oct digits */
#define TLIMIT	4			/* TX0.4 is fastest timer */

#ifndef SIGRTMAX
#define SIGRTMAX	32		/* for non-POSIX systems (Win32) */
#endif
#define QUIT_TERMINAL	(SIGRTMAX+1)

static unsigned	time_cnt;		/* count time in ticks */
static short	flag1C;

#ifdef	_MSDOS_
#ifdef	MSC
void (interrupt far *oldhandler)();
void interrupt far handler1C(void);
#else	/* MSC */
void interrupt (*oldhandler)(void);
void interrupt handler1C(void);
#endif	/* MSC */
#define D10	3			/* 1/3 second interrupts under MSDOS */
#define ENTER	'\r'

#else	/* Linux */

#define D10	10			/* 1/10 second select timeout under Linux */
#define ENTER	'\n'
static fd_set	selectinfds;		/* Set of fd for input (stdin, sync) */
static fd_set	readfds;		/* Read mask for select system call */
static int	maxfd;			/* Highest fd in selectinfds */
static struct timeval	timeOut = { 0, 50000 };	/* 50 mS */
static struct timeval	to;
static struct timeval *	top;

static int		ttyparmFlag = 0;
static struct termios	ttyparms;
static struct termios	ttyparmh;

# define max(x, y) ((x) > (y) ? (x) : (y))

static int
kbhit(void)
{
    fd_set	iC_infds = selectinfds;
    int		stat;

    /* Wait until stdin is ready */
    do {
	readfds = iC_infds;
	if (top && top->tv_sec == 0 && top->tv_usec == 0) {
	    *top = timeOut;
	}
    } while ((stat = select (maxfd + 1, &readfds, 0, 0, top)) == -1
	   && errno == EINTR);
    if (stat == -1) {
	fprintf(stderr, "error in select\n");
    } else if (stat == 0) {
	time_cnt++;			/* count time in ticks */
	/* increase the global counter */
	flag1C = 1;			/* 50 ms on, 50 ms off */
    }
    return stat;			/* can only be 0 or 1 */
} /* kbhit */
#endif	/* Linux */

static iC_Functp * iC_i_lists[] = { I_LISTS };

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

unsigned char	iC_QM_[IXD/8];		/* Output slot mask per cage */
unsigned char	iC_QMM;			/* Output cage mask for 1 rack */

static int	dis_cnt;
short		iC_error_flag;

unsigned	iC_scan_cnt;		/* count scan operations */
unsigned	iC_link_cnt;		/* count link operations */
#if	YYDEBUG && !defined(_WINDOWS)
unsigned	iC_glit_cnt;		/* count glitches */
unsigned long	glit_nxt;		/* count glitch scan */
#endif	/* YYDEBUG && !defined(_WINDOWS) */

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
    int		cn;
    int		cnt;
    Gate **	opp;
    Gate **	tim = &iC_TX_[0];	/* pointers to 8 system gates TX[0] to TX[7] */
    Gate *	gp;
    iC_Functp	init_fa;
    int		tcnt1  = 1;
    int		tcnt10 = 1;
    int		tcnt60 = 1;
#if	INT_MAX == 32767 && defined (LONG16)
    long	val;
#else	/* INT_MAX == 32767 && defined (LONG16) */
    int	val;
#endif	/* INT_MAX == 32767 && defined (LONG16) */
    char *	format;			/* number format */
    char	ybuf[YSIZE];		/* buffer for number */
    char *	yp;

    iC_initIO();			/* catch memory access signal */

    if (iC_outFP && iC_outFP != stdout) {
	fclose(iC_outFP);
	if (iFlag) {
	    iC_inversionCorrection();	/* only applies to compiler listing */
	    iFlag = 0;
	}
    }
    iC_outFP = stdout;			/* standard output from here */
    if (iC_errFP && iC_errFP != stderr) {
	fclose(iC_errFP);
    }
    iC_errFP = stderr;			/* standard error from here */

    for (cnt = 1; cnt < 8; cnt++) {
	if (tim[cnt] != NULL) {		/* any of the 7 timers programmed ? */
	    if (cnt < TLIMIT) {
		fprintf(iC_errFP, "\n%s: Timer TX0.%d is not supported\n", iC_progname, cnt);
		iC_quit(6);
	    }
	    top = &to;			/* activate select timeout */
	    break;			/* could optimise by varying delay */
	}
    }

#ifdef	_MSDOS_
#ifdef	MSC
    oldhandler = _dos_getvect(INTR);	/* save the old interrupt vector */
#else	/* MSC */
    oldhandler = getvect(INTR);		/* save the old interrupt vector */
#endif	/* MSC */
#else	/* Linux */
    /* Setup fd sets */
    FD_ZERO (&selectinfds);
    maxfd = 0;
    FD_SET (fileno (stdin), &selectinfds);
    maxfd = max(maxfd, (int)fileno(stdin));
#endif	/* Linux */

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
    Out_init(iC_f_list);
    Out_init(iC_s_list);
#if	YYDEBUG
    if (iC_debug & 0100) fprintf(iC_outFP, "\nINITIALISATION\n");
#endif	/* YYDEBUG */
    for (i = 0; i < IXD; i++) {		/* clear output array used to hold */
	iC_QX_[i] = 0;			/* output size X, B or W during compilation */
    }
    /********************************************************************
     *  Carry out 4 Passes to initialise all Gates
     *******************************************************************/
    for (pass = 0; pass < 4; pass++) {
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, "\nPass %d:", pass + 1);
#endif	/* YYDEBUG */
	for (opp = iC_sTable; opp < iC_sTend; opp++) {
	    gp = *opp;
	    typ = gp->gt_ini > 0 ? AND : -gp->gt_ini;
	    if (typ < MAX_OP) {
		init_fa = iC_i_lists[typ][pass];
		(*init_fa)(gp, typ);	/* initialise for this pass */
	    }
	}
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

    if (iC_debug & 03000) {
#ifdef	_MSDOS_
#ifdef	MSC
	_dos_setvect(INTR, handler1C);	/* install the new interrupt handler */
#else
	setvect(INTR, handler1C);	/* install the new interrupt handler */
#endif	/* MSC */
#endif	/* _MSDOS_ */
    }
#ifndef	_MSDOS_
    if (tcgetattr(0, &ttyparms) == -1)   {
	fprintf(stderr, "Cannot get termio from stdin\n");
	exit(-5);
    }
    memcpy((void *) &ttyparmh, (void *) &ttyparms, sizeof(struct termios));
    ttyparmh.c_lflag &= ~(ECHO | ICANON | ECHOE);
    if (tcsetattr(0, TCSAFLUSH, &ttyparmh) == -1) exit(-6);
    ttyparmFlag = 1;
#endif	/* _MSDOS_ */

    if ((gp = tim[0]) != NULL) {
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, "EOI:\t%s  1 ==>", gp->gt_ids);
#endif	/* YYDEBUG */
	gp->gt_val = -1;		/* set EOI once as first action */
	iC_link_ol(gp, iC_o_list);	/* fire EOI Input Gate */
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, " -1\n");
#endif	/* YYDEBUG */
    }

    dis_cnt = DIS_MAX;
    /********************************************************************
     *  The following initialisation function is an empty function
     *  in the libict.a support library.
     *		int iCbegin(void) { return 0; }
     *  It may be implemented in a literal block of an iC source, in
     *  which case that function will be linked in preference.
     *  User implementations of iCbegin() should return 1, to activate
     *  the debug message "iCbegin complete ====".
     *
     *  It can be used to initialise immC variables etc. For this reason
     *  it should be executed once after EOI, but before normal processing.
     *  It can use the initial values of all immediate variables.
     *
     *  If the iCbegin() function contains a fork() call, a child process is
     *  spawned, which will run in parallel with immediate processing.
     *******************************************************************/
    if (iCbegin()) {			/* initialisation function */
#if	YYDEBUG
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\niCbegin complete ====\n");
	}
#endif	/* YYDEBUG */
    }
    /********************************************************************
     *  Operational loop
     *******************************************************************/
    for (;;) {
	if (++iC_mark_stamp == 0) {	/* next generation for check */
	    iC_mark_stamp++;		/* leave out zero */
	}

	time_cnt = 0;			/* clear time count */

	/********************************************************************
	 *  Sequencing of different action lists and New I/O handling
	 *
	 *  1   initialisation - put EOI on o_list
	 *      # actions after an idle period:
	 *  2   Loop:  scan a_list unless a_list empty
	 *                 INPW ARITH expr results to a_list
	 *                 comparisons, &&, || to o_list
	 *                 clocked actions to c_list via own clock list
	 *  3        { scan o_list; goto Loop } unless o_list empty
	 *                 bit actions to o_list
	 *                 bits used in arithmetic to a_list (less common)
	 *                 clocked actions to c_list via own clock list
	 ****** CLOCK PHASE *******
	 *  4        { scan c_list; DO 5; goto Loop } unless c_list empty
	 *                 transfer ARITH master values as slave values to a_list
	 *                 transfer GATE master values as slave values to o_list
	 *                 (does not use any combinatorial ARITH or GATE values)
	 *                 transfer master entries on slave clock lists to c_list
	 *                 (continue scanning c_list until all these have been handled)
	 *                 defer 'if else switch' slave C actions to f_list
	 ****** COMBINATORIAL *****
	 *  5        { scan f_list; } unless f_list empty
	 *                 C actions can use and generate combinatotrial ARITH and
	 *                 GATE values, which is start of a new combinatorial scan
	 *  6   scan s_list			# only one scan is required
	 *          do OUTW Gates building send string
	 *  7   send output string with final outputs only
	 *  8   switch to alternate a_list and o_list
	 *  9   IDLE - wait for next input
	 * 10   read new input and link INPW Gates directly to a_list
	 *      or via traMb to o_list; goto Loop
	 *******************************************************************/
	for (;;) {
	    if (iC_a_list != iC_a_list->gt_next) { iC_scan_ar (iC_a_list);           }
	    if (iC_o_list != iC_o_list->gt_next) { iC_scan    (iC_o_list); continue; }
	    if (iC_c_list != iC_c_list->gt_next) {
		iC_scan_clk(iC_c_list);		/* new flist entries can only occurr here */
		if (iC_f_list != iC_f_list->gt_next) { iC_scan_clk(iC_f_list); }
		continue;
	    }
	    if (iC_s_list != iC_s_list->gt_next) { iC_scan_snd(iC_s_list);           }
	    break;
	}

	if (iC_scan_cnt || iC_link_cnt) {
	    fprintf(iC_outFP, "\n");
#if	YYDEBUG
	    if (iC_debug & 02000) {
		fprintf(iC_outFP, "scan = %5u  link = %5u  time = %5u ms",
		    iC_scan_cnt, iC_link_cnt, time_cnt);
		if (iC_glit_cnt) {
		    fprintf(iC_outFP, "  glitch = %5u, %lu", iC_glit_cnt, glit_nxt);
		}
		fprintf(iC_outFP, "\n");
	    }
	    iC_glit_cnt = glit_nxt =
#endif	/* YYDEBUG */
	    iC_scan_cnt = iC_link_cnt = 0;
	}
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
	}
#endif	/* YYDEBUG */
	iC_display(&dis_cnt, DIS_MAX);		/* inputs and outputs */
	/********************************************************************
	 *  Input from keyboard and time input if used
	 *******************************************************************/
	cnt = 0;
    TestInput:
	while (!kbhit() && !flag1C);		/* check inputs */
	cn = 1;
	if (flag1C) {				/* 1/D10 second timer */
	    flag1C = 0;
	    /********************************************************************
	     *  TIMERS here every 50 milliseconds - ~54 ms for MSDOS
	     *
	     *  The iC_debug facility -d1000 stops linking the 100 ms to 60 second
	     *  TIMERS tim[4] to tim[7] when they are connected directly to
	     *  the slave input of a clock or timer Gate and that clock or timer
	     *  has no Gates on the clock list it controls - ie nothing will happen.
	     *  This stops continuous output when tracing logic (-d100 or -t) when
	     *  these internal TIMERS produce no change.
	     *  It also reduces the data traffic to iClive and reduces flashing of
	     *  clocks and timers controlled by internal TIMERS.
	     *******************************************************************/
	    if ((gp = tim[4]) != 0) {			/* 100 millisecond timer */
#if	YYDEBUG
		if (iC_debug & 01000) {
		    Gate **	lp;
		    Gate *	tp;
		    int	cn1 = 2;
		    lp = gp->gt_list;		/* test if any clock or timer is active */
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
		gp->gt_val = - gp->gt_val;		/* complement input */
		iC_link_ol(gp, iC_o_list);
		cn = 0;					/* TX0.4 changed */
		cnt++;
#if	YYDEBUG
		if (iC_debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', iC_outFP);
	    skipT4: ;
#endif	/* YYDEBUG */
	    }
	    if (--tcnt1 <= 0) {
		tcnt1 = D10;			/* 10 under Linux, 9 under MSDOS */
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
				    tp = *((Gate **)tp->gt_list);
				    if (tp->gt_next != tp) {
					goto linkT5;	/* found an active clock or timer */
				    }
				} else {
				    goto linkT5;	/* found a link to non clock or timer */
				}
			    }
			} while (--cn1);
			goto skipT5;			/* excuse spaghetti - faster without flag */
		    }
		linkT5:
		    if (iC_debug & 0100) fprintf(iC_outFP, " %s ", gp->gt_ids);
#endif	/* YYDEBUG */
		    gp->gt_val = - gp->gt_val;		/* complement input */
		    iC_link_ol(gp, iC_o_list);
		    cn = 0;				/* TX0.5 changed */
		    cnt++;
#if	YYDEBUG
		    if (iC_debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', iC_outFP);
		skipT5: ;
#endif	/* YYDEBUG */
		}
		if (--tcnt10 <= 0) {
		    tcnt10 = 10;
		    if ((gp = tim[6]) != 0) {		/* 10 second timer */
#if	YYDEBUG
			if (iC_debug & 01000) {
			    Gate **	lp;
			    Gate *	tp;
			    int	cn1 = 2;
			    lp = gp->gt_list;		/* test if any clock or timer is active */
			    do {			/* for normal and inverted */
				while ((tp = *lp++) != 0) {
				    if (tp->gt_fni == CLCK || tp->gt_fni == TIMR) {
					tp = *((Gate **)tp->gt_list);
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
			if (iC_debug & 0100) fprintf(iC_outFP, " %s ", gp->gt_ids);
#endif	/* YYDEBUG */
			gp->gt_val = - gp->gt_val;	/* complement input */
			iC_link_ol(gp, iC_o_list);
			cn = 0;				/* TX0.6 changed */
			cnt++;
#if	YYDEBUG
			if (iC_debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', iC_outFP);
		    skipT6: ;
#endif	/* YYDEBUG */
		    }
		    if (--tcnt60 <= 0) {
			tcnt60 = 6;
			if ((gp = tim[7]) != 0) {	/* 60 second timer */
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
						goto linkT7;/* found an active clock or timer */
					    }
					} else {
					    goto linkT7;/* found a link to non clock or timer */
					}
				    }
				} while (--cn1);
				goto skipT7;		/* excuse spaghetti - faster without flag */
			    }
			linkT7:
			    if (iC_debug & 0100) fprintf(iC_outFP, " %s ", gp->gt_ids);
#endif	/* YYDEBUG */
			    gp->gt_val = - gp->gt_val;	/* complement input */
			    iC_link_ol(gp, iC_o_list);
			    cn = 0;			/* TX0.7 changed */
			    cnt++;
#if	YYDEBUG
			    if (iC_debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', iC_outFP);
			skipT7: ;
#endif	/* YYDEBUG */
			}
		    }
		}
	    }
	    if (cn) {
		goto TestInput;			/* no timer fired */
	    }
	} else {
	    while (cn) {
		if ((c = getch()) == 'q') {
		    iC_quit(QUIT_TERMINAL);	/* quit normally */
		}
		if (c == EOF) {
		    iC_quit(0);			/* quit at end of file */
		}
		if (c != ENTER) {
		    putc(c, iC_outFP);		/* echo */
		}
		if (c >= '0' && c < '0' + MAX_IO) {
		    if ((gp = iC_IX_[c - '0']) != 0) {
			putc('+', iC_outFP);	/* acknowledge input */
			gp->gt_val = -gp->gt_val; /* complement input */
			iC_link_ol(gp, iC_o_list);
#if	YYDEBUG
			if (iC_debug & 0100) {
			    putc(gp->gt_val < 0 ? '1' : '0', iC_outFP);
			}
#endif	/* YYDEBUG */
			cn--;			/* apply input ? */
			cnt++;			/* count inputs */
		    } else {
			putc('?', iC_outFP);	/* input not configured */
		    }
		} else if (c == '+') {
		    cn++;			/* 1 extra input */
		} else if (c == '-') {
		    if (--cn <= 0) cn = 1;	/* at least 1 more in */
		}
		if (c == 'b') {
		    if ((gp = iC_IB_[1]) != 0) {	/* b byte input to IB1 */
			goto wordIn;
		    }
		    goto wordEr;		/* input not configured */
		}
		if (c == 'w') {
		    if ((gp = iC_IW_[2]) != 0) {	/* w word input to IW2 */
#if	INT_MAX != 32767 || defined (LONG16)
			goto wordIn;
		    }
		    goto wordEr;		/* input not configured */
		}
		if (c == 'l') {
		    if ((gp = iC_IL_[4]) != 0) {	/* l long input to IL4 */
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		    wordIn:
			yp = ybuf;
			if ((c = getch()) == '0') {	/* initial 0 is oct or hex */
			    putc(c, iC_outFP);		/* echo */
			    if ((c = getch()) == 'x' || c == 'X') {
				putc(c, iC_outFP);	/* echo */
#if	INT_MAX == 32767 && defined (LONG16)
				format = "%lx%s";	/* hexadecimal */
#else	/* INT_MAX == 32767 && defined (LONG16) */
				format = "%x%s";	/* hexadecimal */
#endif	/* INT_MAX == 32767 && defined (LONG16) */
			    } else {
#if	INT_MAX == 32767 && defined (LONG16)
				format = "%lo%s";	/* octal */
#else	/* INT_MAX == 32767 && defined (LONG16) */
				format = "%o%s";	/* octal */
#endif	/* INT_MAX == 32767 && defined (LONG16) */
				*yp++ = '0';		/* may be a single 0 */
				ungetch(c);
			    }
			} else {
			    if (c == '-') {		/* optional dec minus sign */
				putc(c, iC_outFP);	/* echo */
				*yp++ = c;
				c = getch();
			    }
#if	INT_MAX == 32767 && defined (LONG16)
			    format = "%ld%s";		/* decimal */
#else	/* INT_MAX == 32767 && defined (LONG16) */
			    format = "%d%s";		/* decimal */
#endif	/* INT_MAX == 32767 && defined (LONG16) */
			    ungetch(c);
			}
			while ((c = getch()) != EOF && (
			    (isprint(c) && yp < &ybuf[YSIZE-1]) ||
			    c == '\b' ||
			    c == '\177')
			) {
			    if (c == '\b' || c == '\177') {
				if (yp > ybuf) {	/* backspace or del */
				    fprintf(iC_outFP, "\b \b");
				    yp--;
				} else {
				    break;
				}
			    } else {
				putc(c, iC_outFP);	/* echo */
				*yp++ = c;
			    }
			}
			*yp = 0;			/* string terminator */
			if (sscanf(ybuf, format, &val, ybuf) != 1) goto wordEr;
			putc('+', iC_outFP);		/* acknowledge input */
			if (gp == iC_IB_[1]) {
			    val &= 0xff;		/* reduce to byte */
			} else if (gp == iC_IW_[2]) {
			    val = (short)val;		/* reduce to signed word */
			}
			if (val != gp->gt_new && /* first change or glitch */
			((gp->gt_new = val) != gp->gt_old) ^ (gp->gt_next != 0)) {
			    /* arithmetic master action */
			    iC_link_ol(gp, iC_a_list);	/* no actions */
			}
			cn--;
			cnt++;				/* count inputs */
		    } else {
		    wordEr:
			putc('?', iC_outFP);		/* input not configured */
		    }
		} else if (c == 'x') {
		    iC_xflag = 1;			/* hexadecimal output */
		    cn = 0;
		} else if (c == 'd') {
		    iC_xflag = 0;			/* decimal output */
		    cn = 0;
		} else if (c == ENTER) {
		    cn = 0;
		} else {
		    putc('?', iC_outFP);		/* not a valid input character */
		}
	    }
	}
	/* if many inputs change simultaneously increase oscillator limit */
	iC_osc_lim = (cnt << 1) + 1;			/* (cnt * 2) + 1 */
	if (iC_osc_lim < iC_osc_max) {
	    iC_osc_lim = iC_osc_max;			/* cnt = 1, osc_lim = 3 default */
	}
    } /* for (;;) */
} /* iC_icc */
#ifdef	_MSDOS_

/********************************************************************
 *
 *	Clock tick interrupt handler (54 ms. ?)
 *
 *******************************************************************/

#ifdef	MSC
void interrupt far handler1C(void)
#else
void interrupt handler1C(void)
#endif	/* MSC */
{
#ifndef	MSC
    /* disable interrupts during the handling of the interrupt */
    disable();
#endif	/* MSC */
    time_cnt++;						/* count time in ticks */
    /* increase the global counter */
    if (iC_debug & 01000) {
	flag1C = 1;
    }
#ifndef	MSC
    /* reenable interrupts at the end of the handler */
    enable();
#endif	/* MSC */
    /* call the old routine */
    oldhandler();
} /* handler1C */
#endif	/* _MSDOS_ */

/********************************************************************
 *
 *	Initialize IO
 *
 *******************************************************************/

void iC_initIO(void)
{
    signal(SIGSEGV, iC_quit);			/* catch memory access signal */
#ifdef	_MSDOS_
    oldhandler = NULL;
#endif	/* _MSDOS_ */
}

/********************************************************************
 *
 *	Quit program with 'q' or ctrlC or Break via signal SIGINT
 *	or program abort on detected bugs.
 *
 *******************************************************************/

void iC_quit(int sig)
{
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
    if ((sig >= QUIT_TERMINAL || sig == SIGINT) &&
	iCend()) {				/* termination function */
#if	YYDEBUG
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\niCend complete ======\n");
	}
#endif	/* YYDEBUG */
    }
    if (sig == QUIT_TERMINAL) {
	fprintf(iC_errFP, "\n'%s' stopped from terminal\n", iC_progname);
    } else if (sig == SIGINT) {
	fprintf(iC_errFP, "\n'%s' stopped by interrupt from terminal\n", iC_progname);
    } else if (sig == SIGSEGV) {
	fprintf(iC_errFP, "\n'%s' stopped by 'Invalid memory reference'\n", iC_progname);
    }
    if (iC_outFP != stdout) {
	fflush(iC_outFP);
	fclose(iC_outFP);
    }
#ifdef	_MSDOS_
    if (oldhandler && iC_debug & 03000) {
	/* reset the old interrupt handler */
#ifdef	MSC
	_dos_setvect(INTR, oldhandler);
#else	/* MSC */
	setvect(INTR, oldhandler);
#endif	/* MSC */
    }
#else	/* Linux */
    if (ttyparmFlag) {
	if (tcsetattr(0, TCSAFLUSH, &ttyparms) == -1) exit(-1);
    }
#endif	/* Linux */
    if (iC_errFP != stderr) {
	fflush(iC_errFP);
	fclose(iC_errFP);
    }
    exit(sig < QUIT_TERMINAL ? sig : 0);	/* really quit */
} /* iC_quit */

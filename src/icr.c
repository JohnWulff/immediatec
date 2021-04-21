static const char icr_c[] =
"@(#)$Id: icr.c 1.48 $";
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
 *	icr.c
 *	runtime execution with stdio test input only
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
#else	/* ! _MSDOS_ Linux */
#define getch() getchar()
#define ungetch(x) ungetc(x, stdin)
#include	<sys/select.h>
#include	<sys/time.h>
#include	<termios.h>
#include	<unistd.h>
#endif	/* ! _MSDOS_ Linux */
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
#define	MAX_IO	8
#define	DIS_MAX	5			/* diplay heading after this many */

static unsigned	time_cnt;		/* count time in ticks */
static short	flag1C;

#ifdef	_MSDOS_
#ifdef	MSC
void interrupt far handler1C(void);
#else	/* MSC */
void interrupt handler1C(void);
#endif	/* MSC */
#define D10	3			/* 1/3 second interrupts under MSDOS */
#define ENTER	'\r'

#else	/* ! _MSDOS_ Linux */

#define D10	10			/* 1/10 second select timeout under Linux */
#define ENTER	'\n'
static fd_set	selectinfds;		/* Set of fd for input (stdin, sync) */
static fd_set	readfds;		/* Read mask for select system call */
static int	maxfd;			/* Highest fd in selectinfds */
struct timeval	iC_timeOut = { 0, 50000 };	/* 50 mS select timeout - may be modified in iCbegin() */
static struct timeval	toCnt =   { 0, 50000 };	/* actual timeout counter that select uses */
static struct timeval *	toCntp = NULL;		/* select timeout switched off when NULl pointer */
static struct termios	ttyparmh;

# define max(x, y) ((x) > (y) ? (x) : (y))

static void display(int * dis_cntp, int dis_max);

static int
kbhit(void)
{
    fd_set	infds = selectinfds;
    int		stat;

    /* Wait until stdin is ready */
    do {
	readfds = infds;
	if (toCnt.tv_sec == 0 && toCnt.tv_usec == 0) {
	    toCnt = iC_timeOut;		/* transfer timeout value */
	}
    } while ((stat = select (maxfd + 1, &readfds, 0, 0, iC_osc_flag ? &toCnt : toCntp))
	== -1 && errno == EINTR);
    if (stat == -1) {
	fprintf(stderr, "error in select\n");
    } else if (stat == 0) {
	time_cnt++;			/* count time in ticks */
	/* increase the global counter */
	flag1C = 1;			/* 50 ms on, 50 ms off */
    }
    return stat;			/* can only be 0 or 1 */
} /* kbhit */
#endif	/* ! _MSDOS_ Linux */

static iC_Functp * iC_i_lists[] = { I_LISTS };

/********************************************************************
 *  initialise all lists with their name to aid symbolic debugging
 *  do this here because at least "flist" is needed in load() for
 *  ini output, which runs before iC_icc().
 *******************************************************************/
/* these lists are toggled (initialised dynamically) */
static Gate	alist0 = { 0, 0, 0, 0, "alist0", };
static Gate	alist1 = { 0, 0, 0, 0, "alist1", };
Gate *		iC_aList;		/* arithmetic output action list */
static Gate	olist0 = { 0, 0, 0, 0, "olist0", };
static Gate	olist1 = { 0, 0, 0, 0, "olist1", };
Gate *		iC_oList;		/* logic output action list */
/* these lists are not toggled (static initialisation here) */
Gate *		iC_cList;		/* main clock list "iClock" */
static Gate	flist = { 0, 0, 0, 0, "flist", };
Gate *		iC_fList = &flist;	/* deferred function action list (init in load) */
static Gate	slist = { 0, 0, 0, 0, "slist", };
Gate *		iC_sList = &slist;	/* send bit and byte outputs */

unsigned char	iC_QM_[IXD/8];		/* Output slot mask per cage */
unsigned char	iC_QMM;			/* Output cage mask for 1 rack */

static int	dis_cnt;
short		iC_error_flag;

unsigned	iC_scan_cnt;		/* count scan operations */
unsigned	iC_link_cnt;		/* count link operations */
#if	YYDEBUG && !defined(_WINDOWS)
unsigned	iC_glit_cnt;		/* count glitches */
unsigned long	iC_glit_nxt;		/* count glitch scan */
#endif	/* YYDEBUG && !defined(_WINDOWS) */

/********************************************************************
 *
 *	Procedure iC_icc
 *
 *******************************************************************/

void
iC_icc(void)				/* Gate ** sTable, Gate ** sTend are global */
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

    signal(SIGSEGV, iC_quit);			/* catch memory access signal */
#ifdef	_MSDOS_
    oldhandler = NULL;
#endif	/* _MSDOS_ */

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
		iC_quit(SIGUSR1);
	    }
	    toCntp = &toCnt;		/* activate select timeout */
	    break;			/* could optimise by varying timeout value */
	}
    }

#ifdef	_MSDOS_
#ifdef	MSC
    oldhandler = _dos_getvect(INTR);	/* save the old interrupt vector */
#else	/* MSC */
    oldhandler = getvect(INTR);		/* save the old interrupt vector */
#endif	/* MSC */
#else	/* ! _MSDOS_ Linux */
    /* Setup fd sets */
    FD_ZERO (&selectinfds);
    maxfd = 0;
    FD_SET (fileno (stdin), &selectinfds);
    maxfd = max(maxfd, (int)fileno(stdin));
#endif	/* ! _MSDOS_ Linux */

/********************************************************************
 *
 *	Initialise the work lists to empty lists
 *
 *******************************************************************/

    iC_error_flag = 0;
    alist0.gt_rptr = iC_aList = &alist1;	/* initialise alternate */
    Out_init(iC_aList);
    alist1.gt_rptr = iC_aList = &alist0;	/* start with alist0 */
    Out_init(iC_aList);
    olist0.gt_rptr = iC_oList = &olist1;	/* initialise alternate */
    Out_init(iC_oList);
    olist1.gt_rptr = iC_oList = &olist0;	/* start with olist0 */
    Out_init(iC_oList);
    Out_init(iC_fList);
    Out_init(iC_sList);
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
	if (iC_debug & 0100) fprintf(iC_outFP, "\n== Pass %d:", pass + 1);
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
	iC_link_ol(gp, iC_oList);	/* fire EOI Input Gate */
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, " -1\n");
#endif	/* YYDEBUG */
    }

    dis_cnt = DIS_MAX;
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
    /********************************************************************
     *  Operational loop
     *******************************************************************/
    for (;;) {
	/********************************************************************
	 *  Sequencing of different action lists and Old I/O handling
	 *
	 *  1   initialisation - put EOI on iC_oList
	 *      # actions after an idle period:
	 *  2   Loop:  ++mark_stamp to control oscillations
	 ****** COMBINATORIAL PHASE *****
	 *             scan iC_aList unless iC_aList empty
	 *                 INPW ARITH expr results to iC_aList
	 *                 comparisons, &&, || to iC_oList
	 *                 clocked actions to iC_cList via own clock list
	 *  3        { scan iC_oList; goto Loop } unless iC_oList empty
	 *                 bit actions to iC_oList
	 *                 bits used in arithmetic to iC_aList (less common)
	 *                 clocked actions to iC_cList via own clock list
	 ****** CLOCK PHASE *******
	 *  4        { ++mark_stamp to control oscillations
	 *             scan iC_cList; DO 5; goto Loop } unless iC_cList empty
	 *                 transfer ARITH master values as slave values to iC_aList
	 *                 transfer GATE master values as slave values to iC_oList
	 *                 (does not use any combinatorial ARITH or GATE values)
	 *                 transfer master entries on slave clock lists to iC_cList
	 *                 (continue scanning iC_cList until all these have been handled)
	 *                 defer 'if else switch' slave C actions to iC_fList
	 ****** COMBINATORIAL PHASE *****
	 *  5        { scan iC_fList; } unless iC_fList empty
	 *                 C actions can use and generate combinatotrial ARITH and
	 *                 GATE values, which is start of a new combinatorial scan
	 *  6   scan iC_sList			# only one scan is required
	 *          do OUTW and OUTX Gates via slot and cage
	 *  7   distribute slots
	 *  8   switch to alternate iC_aList and iC_oList
	 *  9   IDLE - wait for next input
	 * 10   read new input and link INPW Gates directly to iC_aList
	 *      link INPX Gates directly to iC_oList; goto Loop
	 *******************************************************************/
	if (++iC_mark_stamp == 0) {	/* next generation for oscillator check */
	    iC_mark_stamp++;		/* leave out zero */
	}
	time_cnt = 0;			/* clear time count */
	for (;;) {
	    if (iC_aList != iC_aList->gt_next) { iC_scan_ar (iC_aList);           }
	    if (iC_oList != iC_oList->gt_next) { iC_scan    (iC_oList); continue; }
	    if (iC_cList != iC_cList->gt_next) {
		if (++iC_mark_stamp == 0) {	/* next generation for oscillator check */
		    iC_mark_stamp++;		/* leave out zero */
		}
		iC_scan_clk(iC_cList);		/* new flist entries can only occur here */
		if (iC_fList != iC_fList->gt_next) { iC_scan_clk(iC_fList); }
		continue;
	    }
	    if (iC_sList != iC_sList->gt_next) { iC_scan_snd(iC_sList);           }
	    break;
	}

	if (iC_scan_cnt || iC_link_cnt) {
	    fprintf(iC_outFP, "\n");
#if	YYDEBUG
	    if (iC_debug & 02000) {
		fprintf(iC_outFP, "scan = %5u  link = %5u  time = %5u ms",
		    iC_scan_cnt, iC_link_cnt, time_cnt);
		if (iC_glit_cnt) {
		    fprintf(iC_outFP, "  glitch = %5u, %lu", iC_glit_cnt, iC_glit_nxt);
		}
		fprintf(iC_outFP, "\n");
	    }
	    iC_glit_cnt = iC_glit_nxt =
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
	iC_aList = iC_aList->gt_rptr;	/* alternate arithmetic list */
	iC_oList = iC_oList->gt_rptr;	/* alternate logic list */

	if (iC_osc_gp) {
	    fprintf(iC_outFP,
		"*** Warning: %s has oscillated %d times - check iC program!!!\n",
		iC_osc_gp->gt_ids, iC_osc_gp->gt_mcnt);
	    iC_osc_gp = NULL;
	}

#if	YYDEBUG
	if ((iC_debug & 0200) &&		/* osc info */
	    (iC_aList->gt_next != iC_aList || iC_oList->gt_next != iC_oList)) {
	    fprintf(iC_outFP, "OSC =");
	    for (gp = iC_aList->gt_next; gp != iC_aList; gp = gp->gt_next) {
		fprintf(iC_outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
	    }
	    for (gp = iC_oList->gt_next; gp != iC_oList; gp = gp->gt_next) {
		fprintf(iC_outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
	    }
	    fprintf(iC_outFP, "\n");
	}
#endif	/* YYDEBUG */
	/********************************************************************
	 *  Display inputs and outputs
	 *******************************************************************/
	display(&dis_cnt, DIS_MAX);		/* inputs and outputs */
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
				tp = tp->gt_funct;
				if (tp->gt_next != tp) {
				    goto linkT4;/* found an active clock or timer */
				}
			    } else {
				goto linkT4;	/* found a link to non clock or timer */
			    }
			}
		    } while (--cn1);
		    goto skipT4;		/* excuse spaghetti - faster without flag */
		}
	    linkT4:
		if (iC_debug & 0100) fprintf(iC_outFP, "%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
		gp->gt_val = - gp->gt_val;		/* complement input */
		iC_link_ol(gp, iC_oList);
		cn = 0;					/* TX0.4 changed */
		cnt++;
#if	YYDEBUG
		if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
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
				    tp = tp->gt_funct;
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
		    if (iC_debug & 0100) fprintf(iC_outFP, "%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
		    gp->gt_val = - gp->gt_val;		/* complement input */
		    iC_link_ol(gp, iC_oList);
		    cn = 0;				/* TX0.5 changed */
		    cnt++;
#if	YYDEBUG
		    if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
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
			if (iC_debug & 0100) fprintf(iC_outFP, "%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
			gp->gt_val = - gp->gt_val;	/* complement input */
			iC_link_ol(gp, iC_oList);
			cn = 0;				/* TX0.6 changed */
			cnt++;
#if	YYDEBUG
			if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
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
					    tp = tp->gt_funct;
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
			    if (iC_debug & 0100) fprintf(iC_outFP, "%s %+d ^=>", gp->gt_ids, gp->gt_val);
#endif	/* YYDEBUG */
			    gp->gt_val = - gp->gt_val;	/* complement input */
			    iC_link_ol(gp, iC_oList);
			    cn = 0;			/* TX0.7 changed */
			    cnt++;
#if	YYDEBUG
			    if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gp->gt_val);
			skipT7: ;
#endif	/* YYDEBUG */
			}
		    }
		}
	    }
	    if (iC_osc_flag) {
		cnt++;				/* gates have been linked to alternate list */
		iC_osc_flag = 0;		/* normal timer operation again */
	    } else
	    if (cn) {
		goto TestInput;			/* no timer fired */
	    }
	} else {
	    while (cn) {
		if ((c = getch()) == 'q' || c == 4) {	/* q or ctrl+D */
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
			iC_link_ol(gp, iC_oList);
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
		} else if (c == 'b') {
		    if ((gp = iC_IB_[1]) != 0) {	/* b byte input to IB1 */
			goto wordIn;
		    }
		    goto wordEr;		/* input not configured */
		} else if (c == 'w') {
		    if ((gp = iC_IW_[2]) != 0) {	/* w word input to IW2 */
#if	INT_MAX != 32767 || defined (LONG16)
			goto wordIn;
		    }
		    goto wordEr;		/* input not configured */
		} else if (c == 'l') {
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
			    iC_link_ol(gp, iC_aList);	/* no actions */
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
		} else if (c == 'h') {
		    fprintf(iC_outFP, "\n"
"      Entering 0 to 7 toggles simulated inputs IX0.0 to IX0.7\n"
"      Normally 0 to 7 acts immediately. Preceding it with +\n"
"      allows more simultaneous logic inputs - one for each +\n"
"      Entering b<num>CR alters simulated inputs IB1 if coded\n"
"      Entering w<num>CR alters simulated inputs IW2 if coded\n"
"      Entering l<num>CR alters simulated inputs IL4 if coded\n"
"                <num> may be decimal 255, octal 0177 or hexadecimal 0xff\n"
"      Programmed outputs QX0.0 to QX0.7, QB1, QB2 and QL4 are displayed\n"
"      Entering d outputs QB1, QB2 and QL4 in decimal (default)\n"
"      Entering x outputs QB1, QB2 and QL4 in hexadecimal\n"
"      Entering q or ctrl-C quits the program\n"
"      Entering any other character echos ?\n");
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
	    iC_osc_lim = iC_osc_max;			/* cnt = 1, osc_lim = 4 default */
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
 *	Display inputs & outputs
 *
 *******************************************************************/

static void
display(int * dis_cntp, int dis_max)
{
    int			n;
    Gate *		gp;
    unsigned char	x0;
    int			b1;
    int			w2;
#if  INT_MAX != 32767 || defined (LONG16)
    long		l4;
#endif /* INT_MAX != 32767 || defined (LONG16) */

    if ((*dis_cntp)++ >= dis_max) {	/* display header line */
	*dis_cntp = 1;
	for (n = 0; n < MAX_IO; n++) {
	    if ((gp = iC_IX_[n]) != 0) fprintf(iC_outFP, " I%d", n);
	}
	if (iC_IB_[1] != 0) fprintf(iC_outFP, "  IB1");
	if (iC_IW_[2] != 0) fprintf(iC_outFP, "    IW2");
#if	INT_MAX != 32767 || defined (LONG16)
	if (iC_IL_[4] != 0) fprintf(iC_outFP, "      IL4");
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	fprintf(iC_outFP, "   ");
	for (n = 0; n < MAX_IO; n++) {
	    fprintf(iC_outFP, " Q%d", n);
	}
	fprintf(iC_outFP, "  QB1    QW2");
#if	INT_MAX != 32767 || defined (LONG16)
	fprintf(iC_outFP, "      QL4");
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	fprintf(iC_outFP, "\n");
    }
    /* display IX0 .. IX7 - any that are active */
    for (n = 0; n < MAX_IO; n++) {
	if ((gp = iC_IX_[n]) != 0) {
	    fprintf(iC_outFP, "  %c", gp->gt_val < 0 ? '1' : '0');
	}
    }
    /* display IB1, IW2 and IL4 if active */
    if (!iC_xflag) {
#if INT_MAX == 32767 && defined (LONG16)
	if ((gp = iC_IB_[1]) != 0) fprintf(iC_outFP, " %4ld", gp->gt_new);
	if ((gp = iC_IW_[2]) != 0) fprintf(iC_outFP, " %6ld", gp->gt_new);
	if ((gp = iC_IL_[4]) != 0) fprintf(iC_outFP, " %8ld", gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
	if ((gp = iC_IB_[1]) != 0) fprintf(iC_outFP, " %4d", gp->gt_new);
	if ((gp = iC_IW_[2]) != 0) fprintf(iC_outFP, " %6d", gp->gt_new);
#if	INT_MAX != 32767
	if ((gp = iC_IL_[4]) != 0) fprintf(iC_outFP, " %8d", gp->gt_new);
#endif	/* INT_MAX != 32767 */
#endif	/* INT_MAX == 32767 && defined (LONG16) */
    } else {
#if INT_MAX == 32767 && defined (LONG16)
	if ((gp = iC_IB_[1]) != 0) fprintf(iC_outFP, " 0x%02lx", gp->gt_new);
	if ((gp = iC_IW_[2]) != 0) fprintf(iC_outFP, " 0x%04lx", gp->gt_new);
	if ((gp = iC_IL_[4]) != 0) fprintf(iC_outFP, " 0x%08lx", gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
	if ((gp = iC_IB_[1]) != 0) fprintf(iC_outFP, " 0x%02x", gp->gt_new);
	if ((gp = iC_IW_[2]) != 0) fprintf(iC_outFP, " 0x%04x", gp->gt_new);
#if	INT_MAX != 32767
	if ((gp = iC_IL_[4]) != 0) fprintf(iC_outFP, " 0x%08x", gp->gt_new);
#endif	/* INT_MAX != 32767 */
#endif	/* INT_MAX == 32767 && defined (LONG16) */
    }
    fprintf(iC_outFP, "   ");

    x0 = iC_QX_[0];
    b1 = iC_QX_[1];
    w2 = *(short*)&iC_QX_[2];
#if	INT_MAX == 32767
    l4 = *(long*)&iC_QX_[4];
#else	/* INT_MAX == 32767 */
    l4 = *(int*)&iC_QX_[4];
#endif	/* INT_MAX == 32767 */
    /* display QX0 .. QX7 */
    for (n = 0; n < MAX_IO; n++) {
	fprintf(iC_outFP, "  %c", (x0 & 0x0001) ? '1' : '0');
	x0 >>= 1;			/* scan output bits */
    }
    /* display QB1, QW2 and QL4 */
    if (!iC_xflag) {
#if	INT_MAX != 32767 || defined (LONG16)
	fprintf(iC_outFP, " %4d %6hd %8ld\n", (int)b1, w2, l4);
#else	/* INT_MAX != 32767 || defined (LONG16) */
	fprintf(iC_outFP, " %4d %6hd\n", (int)b1, w2);	/* no QL4 */
#endif	/* INT_MAX != 32767 || defined (LONG16) */
    } else {
#if	INT_MAX != 32767 || defined (LONG16)
	fprintf(iC_outFP, "   0x%02x   0x%04hx 0x%08lx\n", (int)b1, w2, l4);
#else	/* INT_MAX != 32767 || defined (LONG16) */
	fprintf(iC_outFP, "   0x%02x   0x%04hx\n", (int)b1, w2);	/* no QL4 */
#endif	/* INT_MAX != 32767 || defined (LONG16) */
    }
    fflush(iC_outFP);
} /* display */

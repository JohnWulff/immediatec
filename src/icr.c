static const char icc_c[] =
"@(#)$Id: icr.c,v 1.30 2005/01/26 15:16:22 jw Exp $";
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
 *	icc.c
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
#include	<termio.h>
#endif
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

#define MAX_W	2
#define INTR	0x1c			/* The clock tick interrupt */
#define YSIZE	13			/* - and 10 dec digits or 12 oct digits */

static unsigned	time_cnt;		/* count time in ticks */
static short	flag1C;

#ifdef	_MSDOS_
#ifdef	MSC
void (interrupt far *oldhandler)();
void interrupt far handler1C(void);
#else
void interrupt (*oldhandler)(void);
void interrupt handler1C(void);
#endif
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
static struct termio	ttyparms;
static struct termio	ttyparmh;

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
	flag1C = 1;			/* 1/20 second on, 1/20 second off */
    }
    return stat;			/* can only be 0 or 1 */
} /* kbhit */
#endif

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

unsigned char	iC_QM_[IXD/8];		/* Output slot mask per cage */
unsigned char	iC_QMM;			/* Output cage mask for 1 rack */

static int	dis_cnt;
short		iC_error_flag;

unsigned	iC_scan_cnt;		/* count scan operations */
unsigned	iC_link_cnt;		/* count link operations */
#if	YYDEBUG && !defined(_WINDOWS)
unsigned	iC_glit_cnt;		/* count glitches */
unsigned long	glit_nxt;		/* count glitch scan */
#endif

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
    short	c;
    short	typ;
    int		cn;
    int		cnt;
    Gate **	tim = &iC_TX_[0];	/* pointers to 8 system gates TX[0] to TX[7] */
    unsigned *	gcp;
    iC_Functp * *	ilp;
    Gate *	gp;
    iC_Functp	init_fa;
    int		tcnt = 1;
#if	INT_MAX == 32767 && defined (LONG16)
    long	val;
#else
    int	val;
#endif
    char *	format;			/* number format */
    char	ybuf[YSIZE];		/* buffer for number */
    char *	yp;

    iC_initIO();			/* catch memory access signal */
    iC_error_flag = 0;
    alist0.gt_rlist = (Gate **)(iC_a_list = &alist1);	/* initialise alternate */
    Out_init(iC_a_list);
    alist1.gt_rlist = (Gate **)(iC_a_list = &alist0);	/* start with alist0 */
    Out_init(iC_a_list);
    olist0.gt_rlist = (Gate **)(iC_o_list = &olist1);	/* initialise alternate */
    Out_init(iC_o_list);
    olist1.gt_rlist = (Gate **)(iC_o_list = &olist0);	/* start with olist0 */
    Out_init(iC_o_list);
    iC_f_list = &flist;			/* function clock list */
    Out_init(iC_f_list);
    iC_s_list = &slist;			/* send outputs */
    Out_init(iC_s_list);

    if (iC_outFP != stdout) {
	fclose(iC_outFP);
	if (iFlag) {
	    iC_inversionCorrection();
	    iFlag = 0;
	}
	iC_outFP = stdout;		/* standard output from here */
    }
    if (iC_errFP != stderr) {
	fclose(iC_errFP);
	iC_errFP = stderr;		/* standard error from here */
    }
    for (cnt = 1; cnt < 8; cnt++) {
	if (tim[cnt] != NULL) {		/* any of the 7 timers programmed ? */
	    top = &to;			/* activate select timeout */
	    break;			/* could optimise by varying delay */
	}
    }

#ifdef	_MSDOS_
#ifdef	MSC
    oldhandler = _dos_getvect(INTR);	/* save the old interrupt vector */
#else
    oldhandler = getvect(INTR);		/* save the old interrupt vector */
#endif
#else	/* Linux */
    /* Setup fd sets */
    FD_ZERO (&selectinfds);
    maxfd = 0;
    FD_SET (fileno (stdin), &selectinfds);
    maxfd = max(maxfd, (int)fileno(stdin));
#endif

#if	YYDEBUG
    if (iC_debug & 0100) fprintf(iC_outFP, "\nINITIALISATION\n");
#endif
    for (i = 0; i < IXD; i++) {		/* clear output array used to hold */
	iC_QX_[i] = 0;			/* output size X, B or W during compilation */
    }
    for (pass = 0; pass < 4; pass++) {
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, "\nPass %d:", pass + 1);
#endif
	gp = g_lists;
	gcp = gate_count;
	ilp = iC_i_lists;
	for (typ = 0; typ < MAX_OP; typ++) {
	    init_fa = (*ilp++)[pass];
	    for (cnt = *gcp++; cnt; cnt--) {
		(*init_fa)(gp++, typ);	/* initialise for this pass */
	    }
	}
    }

#if	YYDEBUG
    if (iC_debug & 0100) {
	fprintf(iC_outFP, "\nInit complete =======\n");
    }
#endif

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
    if (ioctl(0, TCGETA, &ttyparms) == -1)   {
	fprintf(stderr, "Cannot get termio from stdin\n");
	exit(-5);
    }
    ttyparmFlag = 1;
    memcpy((void *) &ttyparmh, (void *) &ttyparms, sizeof(struct termio));
    ttyparmh.c_lflag &= ~(ECHO | ICANON);
    if (ioctl(0, TCSETA, &ttyparmh) == -1) exit(-6);
#endif	/* _MSDOS_ */

    if ((gp = tim[0]) != 0) {
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, "\nEOP:\t%s  1 ==>", gp->gt_ids);
#endif
	gp->gt_val = -1;		/* set EOP once as first action */
	iC_link_ol(gp, iC_o_list);	/* fire EOP Input Gate */
#if	YYDEBUG
	if (iC_debug & 0100) fprintf(iC_outFP, " -1");
#endif
    }

    dis_cnt = DIS_MAX;
    for ( ; ; ) {
	if (++iC_mark_stamp == 0) {	/* next generation for check */
	    iC_mark_stamp++;		/* leave out zero */
	}

	time_cnt = 0;			/* clear time count */

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
#endif
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
#endif
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
	     *  TIMERS here every 100 milliseconds
	     *******************************************************************/
	    if ((gp = tim[4]) != 0) {		/* 100 millisecond timer */
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
		gp->gt_val = - gp->gt_val;	/* complement input */
		iC_link_ol(gp, iC_o_list);
		cn = 0;				/* TX0.4 changed */
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
			goto skipT5;		/* excuse spaghetti - faster without flag */
		    }
		linkT5:
		    if (iC_debug & 0100) fprintf(iC_outFP, " %s ", gp->gt_ids);
#endif	/* YYDEBUG */
		    gp->gt_val = - gp->gt_val;	/* complement input */
		    iC_link_ol(gp, iC_o_list);
		    cn = 0;			/* TX0.5 changed */
		    cnt++;
#if	YYDEBUG
		    if (iC_debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', iC_outFP);
		skipT5: ;
#endif	/* YYDEBUG */
		}
	    }
	    if (cn) {
		goto TestInput;			/* neither TX.1 nor TX.2 fired */
	    }
	} else {
	    while (cn) {
		if ((c = getch()) == 'q' || c == EOF) {
		    fprintf(iC_errFP, "\n'%s' stopped from terminal\n", iC_progname);
		    iC_quit(0);			/* quit normally */
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
#endif
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
#endif
		    wordIn:
			yp = ybuf;
			if ((c = getch()) == '0') {	/* initial 0 is oct or hex */
			    putc(c, iC_outFP);		/* echo */
			    if ((c = getch()) == 'x' || c == 'X') {
				putc(c, iC_outFP);	/* echo */
#if	INT_MAX == 32767 && defined (LONG16)
				format = "%lx%s";	/* hexadecimal */
#else
				format = "%x%s";	/* hexadecimal */
#endif
			    } else {
#if	INT_MAX == 32767 && defined (LONG16)
				format = "%lo%s";	/* octal */
#else
				format = "%o%s";	/* octal */
#endif
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
#else
			    format = "%d%s";		/* decimal */
#endif
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
    }
} /* iC_icc */
#ifdef	_MSDOS_

/********************************************************************
 *
 *	Clock tick interrupt handler (54 ms. ?)
 *
 *******************************************************************/

int count1C = 0;

#ifdef	MSC
void interrupt far handler1C(void)
#else
void interrupt handler1C(void)
#endif
{
#ifndef	MSC
    /* disable interrupts during the handling of the interrupt */
    disable();
#endif
    time_cnt++;						/* count time in ticks */
    /* increase the global counter */
    if (iC_debug & 01000 &&
	count1C++ >= 6) {			/* roughly 1/3 second on, 1/3 second off */
	count1C = 0;
	flag1C = 1;
    }
#ifndef	MSC
    /* reenable interrupts at the end of the handler */
    enable();
#endif
    /* call the old routine */
    oldhandler();
} /* handler1C */
#endif

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
#endif
}

/********************************************************************
 *
 *	Quit program with 'q' or ctrlC or Break via signal SIGINT
 *	or program abort on detected bugs.
 *
 *******************************************************************/

void iC_quit(int sig)
{
    fflush(iC_outFP);
#ifdef	_MSDOS_
    if (oldhandler && iC_debug & 03000) {
	/* reset the old interrupt handler */
#ifdef	MSC
	_dos_setvect(INTR, oldhandler);
#else
	setvect(INTR, oldhandler);
#endif
    }
#else	/* Linux */
    if (ttyparmFlag) {
	if (ioctl(0, TCSETA, &ttyparms) == -1) exit(-1);
    }
#endif
    if (sig == SIGINT) {
	fprintf(iC_errFP, "\n'%s' stopped by interrupt from terminal\n", iC_progname);
    } else if (sig == SIGSEGV) {
	fprintf(iC_errFP, "\n'%s' stopped by 'Invalid memory reference'\n", iC_progname);
    }
    if (iC_errFP != stderr) {
	fflush(iC_errFP);
	fclose(iC_errFP);
    }
    exit(sig);					/* really quit */
} /* iC_quit */

static const char icc_c[] =
"@(#)$Id: icc.c,v 1.25 2004/01/04 20:36:45 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2001  John E. Wulff
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
#ifdef _MSDOS_
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
#include	"icg.h"
#include	"icc.h"

/***NOTE: This is an interrupt service
 routine. You cannot compile this program
 with Test Stack Overflow turned on and
 get an executable file that operates
 correctly. */

#define MAX_IO	8
#define MAX_W	2
#define INTR	0x1c			/* The clock tick interrupt */
#define YSIZE	13			/* - and 10 dec digits or 12 oct digits */

static void	display(void);
static unsigned	time_cnt;		/* count time in ticks */
static short	flag1C;

#ifdef _MSDOS_
#ifdef MSC
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
    fd_set	infds = selectinfds;
    int		stat;

    /* Wait until stdin is ready */
    do {
	readfds = infds;
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
	if (debug & 01000) {		/* 1/20 second on, 1/20 second off */
	    flag1C = 1;
	}
    }
    return stat;			/* can only be 0 or 1 */
} /* kbhit */
#endif

Functp	*i_lists[] = { I_LISTS };
#ifdef LOAD
#endif

static Gate	alist0;			/* these lists are toggled */
static Gate	alist1;
Gate *		a_list;			/* arithmetic output action list */
static Gate	olist0;			/* these lists are toggled */
static Gate	olist1;
Gate *		o_list;			/* logic output action list */
Gate *		c_list;			/* main clock list "iClock" */
static Gate	flist;
Gate *		f_list;			/* auxiliary function clock list */

unsigned char	QM_[IXD/8];		/* Output slot mask per cage */
unsigned char	QMM;			/* Output cage mask for 1 rack */

short		dis_cnt;
short		error_flag;

unsigned	scan_cnt;			/* count scan operations */
unsigned	link_cnt;			/* count link operations */
#if YYDEBUG && (!defined(_WINDOWS) || defined(LOAD))
unsigned	glit_cnt;			/* count glitches */
unsigned long	glit_nxt;			/* count glitch scan */
#endif

/********************************************************************
 *
 *	Procedure icc
 *
 *******************************************************************/

void
icc(
    Gate *	g_lists,
    unsigned *	gate_count)
{
    int		i;
    short	pass;
    short	c;
    short	typ;
    int		cnt;
#ifdef LOAD
    Gate **	opp;
#else
    unsigned *	gcp;
    Functp * *	ilp;
#endif
    Gate *	gp;
    Functp	init_fa;
    int		tcnt = 3;
#if INT_MAX == 32767 && defined (LONG16)
    long	val;
#else
    int	val;
#endif
    char *	format;		/* number format */
    char	ybuf[YSIZE];	/* buffer for number */
    char *	yp;

    initIO();				/* catch memory access signal */	
    error_flag = 0;
    alist0.gt_rlist = (Gate **)(a_list = &alist1);	/* initialise alternate */
    Out_init(a_list);
    alist1.gt_rlist = (Gate **)(a_list = &alist0);	/* start with alist0 */
    Out_init(a_list);
    olist0.gt_rlist = (Gate **)(o_list = &olist1);	/* initialise alternate */
    Out_init(o_list);
    olist1.gt_rlist = (Gate **)(o_list = &olist0);	/* start with olist0 */
    Out_init(o_list);
#ifdef LOAD
    c_list = &iClock;			/* system clock list */
#endif
    f_list = &flist;			/* function clock list */
    Out_init(f_list);

    if (outFP != stdout) {
	fclose(outFP);
	if (iFlag) {
	    inversionCorrection();
	    iFlag = 0;
	}
	outFP = stdout;			/* standard output from here */
    }
    if (errFP != stderr) {
	fclose(errFP);
	errFP = stderr;			/* standard error from here */
    }

#ifdef _MSDOS_
#ifdef MSC 
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

#if YYDEBUG
    if (debug & 0100) fprintf(outFP, "\nINITIALISATION\n");
#endif
    for (i = 0; i < IXD; i++) {		/* clear output array used to hold */
	QX_[i] = 0;			/* output size X, B or W during compilation */
    }
    for (pass = 0; pass < 4; pass++) {
#if YYDEBUG
	if (debug & 0100) fprintf(outFP, "\nPass %d:", pass + 1);
#endif
#ifdef LOAD
	for (opp = sTable; opp < sTend; opp++) {
	    gp = *opp;
	    typ = gp->gt_ini > 0 ? AND : -gp->gt_ini;
	    if (typ < MAX_OP) {
		init_fa = i_lists[typ][pass];
		(*init_fa)(gp, typ);	/* initialise for this pass */
	    }
	}
#else
	gp = g_lists;
	gcp = gate_count;
	ilp = i_lists;
	for (typ = 0; typ < MAX_OP; typ++) {
	    init_fa = (*ilp++)[pass];
	    for (cnt = *gcp++; cnt; cnt--) {
		(*init_fa)(gp++, typ);	/* initialise for this pass */
	    }
	}
#endif
    }

#if YYDEBUG
    if (debug & 0100) {
	fprintf(outFP, "\nInit complete =======\n");
    }
#endif

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

    if (debug & 03000) {
#ifdef _MSDOS_
#ifdef MSC 
	_dos_setvect(INTR, handler1C);	/* install the new interrupt handler */
#else
	setvect(INTR, handler1C);	/* install the new interrupt handler */
#endif
#else	/* Linux */
	top = &to;				/* activate select timeout */
#endif
    }
#ifndef _MSDOS_
    if (ioctl(0, TCGETA, &ttyparms) == -1)   {
	fprintf(stderr, "Cannot get termio from stdin\n");
	exit(-5);
    }
    ttyparmFlag = 1;
    memcpy((void *) &ttyparmh, (void *) &ttyparms, sizeof(struct termio));
    ttyparmh.c_lflag &= ~(ECHO | ICANON);
    if (ioctl(0, TCSETA, &ttyparmh) == -1) exit(-6);
#endif

    if ((gp = TX_[0]) != 0) {
#if YYDEBUG
	if (debug & 0100) fprintf(outFP, "\nEOP:\t%s  1 ==>", gp->gt_ids);
#endif
	gp->gt_val = -1;			/* set EOP initially */
	link_ol(gp, o_list);			/* fire Input Gate */
#if YYDEBUG
	if (debug & 0100) fprintf(outFP, " -1");
#endif
    }

    dis_cnt = DIS_MAX;
    for ( ; ; ) {
	if (++mark_stamp == 0) {	/* next generation for check */
	    mark_stamp++;		/* leave out zero */
	}

	time_cnt = 0;			/* clear time count */

	do {
	    /* scan arithmetic and logic output lists until empty */
	    while (scan_ar(a_list) || scan(o_list));
	    /* then scan clock list once and repeat until clock list empty */
	    /* only then scan f_list, which holds if-else-switch events */
	} while (scan_clk(c_list) || scan_clk(f_list));

	if (scan_cnt || link_cnt) {
	    fprintf(outFP, "\n");
#if YYDEBUG
	    if (debug & 02000) {
		fprintf(outFP, "scan = %5u  link = %5u  time = %5u ms",
		    scan_cnt, link_cnt, time_cnt);
		if (glit_cnt) {
		    fprintf(outFP, "  glitch = %5u, %lu", glit_cnt, glit_nxt);
		}
		fprintf(outFP, "\n");
	    }
	    glit_cnt = glit_nxt =
#endif
	    scan_cnt = link_cnt = 0;
	}

/********************************************************************
 *
 *	Switch to alternate lists
 *
 *	alternate list contains all those gates which were marked in
 *	the previous scan and which were active more than
 *	MARKMAX times. These are oscillators which wil be
 *	scanned again in the next cycle.
 *
 *******************************************************************/

	a_list = (Gate*)a_list->gt_rlist;	/* alternate arithmetic list */
	o_list = (Gate*)o_list->gt_rlist;	/* alternate logic list */

	if (osc_gp) {
	    fprintf(outFP,
		"*** Warning: %s has oscillated %d times - check iC program!!!\n",
		osc_gp->gt_ids, osc_gp->gt_mcnt);
	    osc_gp = NULL;
	}

#if YYDEBUG
	if ((debug & 0200) &&
	    (a_list->gt_next != a_list || o_list->gt_next != o_list)) {
	    fprintf(outFP, "OSC =");
	    for (gp = a_list->gt_next; gp != a_list; gp = gp->gt_next) {
		fprintf(outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
	    }
	    for (gp = o_list->gt_next; gp != o_list; gp = gp->gt_next) {
		fprintf(outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
	    }
	    fprintf(outFP, "\n");
	}
#endif
	display();				/* inputs and outputs */

/********************************************************************
 *
 *	Input from keyboard and time input if used
 *
 *******************************************************************/

    TestInput:
	while (!kbhit() && !flag1C);		/* check inputs */
	cnt = 1;
	if (flag1C) {				/* 1/D10 second timer */
	    flag1C = 0;
	    if ((gp = TX_[1]) != 0) {		/* 100 millisecond timer */
		fprintf(outFP, " %s ", gp->gt_ids);
		gp->gt_val = - gp->gt_val;	/* complement input */
		link_ol(gp, o_list);
		putc(gp->gt_val < 0 ? '1' : '0', outFP);
		cnt = 0;			/* TX.1 changed */
	    }
	    if (--tcnt == 0) {
		tcnt = D10;
		if ((gp = TX_[2]) != 0) {	/* 1 second timer */
		    fprintf(outFP, " %s ", gp->gt_ids);
		    gp->gt_val = - gp->gt_val;	/* complement input */
		    link_ol(gp, o_list);
		    putc(gp->gt_val < 0 ? '1' : '0', outFP);
		    cnt = 0;			/* TX.2 changed */
		}
	    }
	    if (cnt) {
		goto TestInput;			/* neither TX.1 nor TX.2 */
	    }
	} else {
	    while (cnt) {
		if ((c = getch()) == 'q' || c == EOF) {
		    fprintf(errFP, "\n%s stopped from terminal\n", progname);
		    quit(0);			/* quit normally */
		}
		if (c != ENTER) {
		    putc(c, outFP);		/* echo */
		}
		if (c >= '0' && c < '0' + MAX_IO) {
		    if ((gp = IX_[c - '0']) != 0) {
			putc('+', outFP);	/* acknowledge input */
			gp->gt_val = -gp->gt_val; /* complement input */
			link_ol(gp, o_list);
#if YYDEBUG
			if (debug & 0100) {
			    putc(gp->gt_val < 0 ? '1' : '0', outFP);
			}
#endif
			cnt--;			/* apply input ? */
		    } else {
			putc('?', outFP);	/* input not configured */
		    }
		} else if (c == '+') {
		    cnt++;			/* 1 extra input */
		} else if (c == '-') {
		    if (--cnt <= 0) cnt = 1;	/* at least 1 more in */
		} else if (c == 'b') {
		    if ((gp = IB_[1]) != 0) {	/* b byte input to IB1 */
			goto wordIn;
		    }
		    goto wordEr;		/* input not configured */
		} else if (c == 'w') {
		    if ((gp = IW_[2]) != 0) {	/* w word input to IW2 */
#if INT_MAX != 32767 || defined (LONG16)
			goto wordIn;
		    }
		    goto wordEr;		/* input not configured */
		} else if (c == 'l') {
		    if ((gp = IL_[4]) != 0) {	/* l long input to IL4 */
#endif
		    wordIn:
			yp = ybuf;
			if ((c = getch()) == '0') {	/* initial 0 is oct or hex */
			    putc(c, outFP);		/* echo */
			    if ((c = getch()) == 'x' || c == 'X') {
				putc(c, outFP);		/* echo */
#if INT_MAX == 32767 && defined (LONG16)
				format = "%lx%s";	/* hexadecimal */
#else
				format = "%x%s";	/* hexadecimal */
#endif
			    } else {
#if INT_MAX == 32767 && defined (LONG16)
				format = "%lo%s";	/* octal */
#else
				format = "%o%s";	/* octal */
#endif
				*yp++ = '0';		/* may be a single 0 */
				ungetch(c);
			    }
			} else {
			    if (c == '-') {	/* optional dec minus sign */
				putc(c, outFP);		/* echo */
				*yp++ = c;
				c = getch();
			    }
#if INT_MAX == 32767 && defined (LONG16)
			    format = "%ld%s";		/* decimal */
#else
			    format = "%d%s";		/* decimal */
#endif
			    ungetch(c);
			}
			while ((c = getch()) != EOF &&
			    (isprint(c) && yp < &ybuf[YSIZE-1] || c == '\b' || c == '\177')) {
			    if (c == '\b' || c == '\177') {
				if (yp > ybuf) {	/* backspace or del */
				    fprintf(outFP, "\b \b");
				    yp--;
				} else {
				    break;
				}
			    } else {
				putc(c, outFP);		/* echo */
				*yp++ = c;
			    }
			}
			*yp = 0;		/* string terminator */
			if (sscanf(ybuf, format, &val, ybuf) != 1) goto wordEr;
			putc('+', outFP);	/* acknowledge input */
			if (gp == IB_[1]) {
			    val &= 0xff;	/* reduce to byte */
			} else if (gp == IW_[2]) {
			    val = (short)val;	/* reduce to signed word */
			}
			if (val != gp->gt_new && /* first change or glitch */
			((gp->gt_new = val) != gp->gt_old) ^ (gp->gt_next != 0)) {
			    /* arithmetic master action */
			    link_ol(gp, a_list);	/* no actions */
			}
			cnt--;
		    } else {
		    wordEr:
			putc('?', outFP);	/* input not configured */
		    }
		} else if (c == 'x') {
		    xflag = 1;			/* hexadecimal output */
		    cnt = 0;
		} else if (c == 'd') {
		    xflag = 0;			/* decimal output */
		    cnt = 0;
		} else if (c == ENTER) {
		    cnt = 0;
		} else {
		    putc('?', outFP);		/* not a valid input character */
		}
	    }
	}
    }
} /* icc */

/********************************************************************
 *
 *	display inputs & outputs
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
	for (n = 0; n < 10; n++) {
	    if ((gp = IX_[n]) != 0) fprintf(outFP, " I%d", n);
	}
	if (IB_[1] != 0) fprintf(outFP, "  IB1");
	if (IW_[2] != 0) fprintf(outFP, "    IW2");
#if INT_MAX != 32767 || defined (LONG16)
	if (IL_[4] != 0) fprintf(outFP, "      IL4");
#endif
	fprintf(outFP, "   ");
	for (n = 0; n < MAX_IO; n++) {
	    fprintf(outFP, " Q%d", n);
	}
	fprintf(outFP, "  QB1    QW2");
#if INT_MAX != 32767 || defined (LONG16)
	fprintf(outFP, "      QL4");
#endif
	fprintf(outFP, "\n");
    }
    for (n = 0; n < 10; n++) {
	if ((gp = IX_[n]) != 0) {
	    fprintf(outFP, "  %c", gp->gt_val < 0 ? '1' : '0');
	}
    }
    /* display IB1, IW2 and IL4 if active */
    if (!xflag) {
	if ((gp = IB_[1]) != 0) fprintf(outFP, " %4d", gp->gt_new & 0xff);
	if ((gp = IW_[2]) != 0) fprintf(outFP, " %6hd", (short)gp->gt_new);
#if INT_MAX == 32767
#if defined (LONG16)
	if ((gp = IL_[4]) != 0) fprintf(outFP, " %8ld", gp->gt_new);
#endif
#else
	if ((gp = IL_[4]) != 0) fprintf(outFP, " %8d", gp->gt_new);
#endif
    } else {
	if ((gp = IB_[1]) != 0) fprintf(outFP, "   %02x", gp->gt_new & 0xff);
	if ((gp = IW_[2]) != 0) fprintf(outFP, "   %04x", gp->gt_new & 0xffff);
#if INT_MAX == 32767
#if defined (LONG16)
	if ((gp = IL_[4]) != 0) fprintf(outFP, " %08lx", gp->gt_new);
#endif
#else
	if ((gp = IL_[4]) != 0) fprintf(outFP, " %08x", gp->gt_new);
#endif
    }
    fprintf(outFP, "   ");
    data = *(unsigned short*)QX_;
    for (n = 0; n < MAX_IO; n++) {
	fprintf(outFP, "  %c", (data & 0x0001) ? '1' : '0');
	data >>= 1;				/* scan output bits */
    }
    /* display QB1, QW2 and QL4 */
    if (!xflag) {
#if INT_MAX == 32767
#if defined (LONG16)
	fprintf(outFP, " %4d %6hd %8ld\n", QX_[1], *(short*)&QX_[2], *(long*)&QX_[4]);
#else
	fprintf(outFP, " %4d %6hd\n", QX_[1], *(short*)&QX_[2]);	/* no QL4 */
#endif
#else
	fprintf(outFP, " %4d %6hd %8d\n", QX_[1], *(short*)&QX_[2], *(int*)&QX_[4]);
#endif
    } else {
#if INT_MAX == 32767
#if defined (LONG16)
	fprintf(outFP, "   %02x   %04hx %08lx\n", QX_[1], *(short*)&QX_[2], *(long*)&QX_[4]);
#else
	fprintf(outFP, "   %02x   %04hx\n", QX_[1], *(short*)&QX_[2]);	/* no QL4 */
#endif
#else
	fprintf(outFP, "   %02x   %04hx %08x\n", QX_[1], *(short*)&QX_[2], *(int*)&QX_[4]);
#endif
    }
    fflush(outFP);
} /* display */
#ifdef _MSDOS_

/********************************************************************
 *
 *	Clock tick interrupt handler (54 ms. ?)
 *
 *******************************************************************/

int count1C = 0;

#ifdef MSC 
void interrupt far handler1C(void)
#else
void interrupt handler1C(void)
#endif
{
#ifndef MSC 
    /* disable interrupts during the handling of the interrupt */
    disable();
#endif
    time_cnt++;				/* count time in ticks */
    /* increase the global counter */
    if (debug & 01000 &&
	count1C++ >= 6) {	/* roughly 1/3 second on, 1/3 second off */
	count1C = 0;
	flag1C = 1;
    }
#ifndef MSC 
    /* reenable interrupts at the end of the handler */
    enable();
#endif
    /* call the old routine */
    oldhandler();
} /* handler1C */
#endif

/********************************************************************
 *
 *	initialize IO
 *
 *******************************************************************/

void initIO(void)
{
    signal(SIGSEGV, quit);		/* catch memory access signal */	
#ifdef _MSDOS_
    oldhandler = NULL;
#endif
}

/********************************************************************
 *
 *	Quit program with 'q' or ctrlC or Break via signal SIGINT
 *	or program abort on detected bugs.
 *
 *******************************************************************/

void quit(int sig)
{
    fflush(outFP);
#ifdef _MSDOS_
    if (oldhandler && debug & 03000) {
	/* reset the old interrupt handler */
#ifdef MSC 
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
	fprintf(errFP, "\n%s stopped by interrupt from terminal\n", progname);
    } else if (sig == SIGSEGV) {
	fprintf(errFP, "\n%s stopped by 'Invalid memory reference'\n", progname);
    }
    if (errFP != stderr) {
	fflush(errFP);
	fclose(errFP);
    }
    exit(sig);			/* really quit */
} /* quit */

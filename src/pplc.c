static const char pplc_c[] =
"@(#)$Id: pplc.c,v 1.5 1999/08/06 15:43:37 jw Exp $";
/********************************************************************
 *
 *	parallel plc - procedure
 *
 *	FOR Microsoft C
 *
 *	"pplc.c	3.33	95/02/06"
 *
 *******************************************************************/

/* J.E. Wulff	3-Mar-85 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifdef _MSDOS_
#include <dos.h>
#include <conio.h>
#else	/* Linux */
#define getch() getchar()
#define ungetch(x) ungetc(x, stdin)
#include <sys/types.h>
#include <sys/time.h>
#include <termio.h>
#endif
#include <signal.h>
#include <ctype.h>
#include "pplc.h"

/***NOTE: This is an interrupt service
 routine. You cannot compile this program
 with Test Stack Overflow turned on and
 get an executable file that operates
 correctly. */

#define MAX_IO	8
#define MAX_W	2
#define INTR 0x1c    /* The clock tick interrupt */
#define YSIZE	10

static void	display(void);
static unsigned	time_cnt;			/* count time in ticks */
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

Gate		alist0;
Gate		alist1;
Gate *		a_list;			/* these lists are toggled */
Gate		olist0;
Gate		olist1;
Gate *		o_list;			/* these lists are toggled */
Gate *		c_list;			/* "iClock" */

Gate *		IX_[IXD*8];		/* pointers to Bit Input Gates */
Gate *		IB_[IXD];		/* pointers to Byte Input Gates */
Gate *		IW_[IXD];		/* pointers to Word Input Gates */
Gate *		TX_[TXD*8];		/* pointers to System Bit Gates */
unsigned char	QX_[IXD];		/* Output bit field */

short		dis_cnt;
short		error_flag;

unsigned	scan_cnt;			/* count scan operations */
unsigned	link_cnt;			/* count link operations */
unsigned	glit_cnt;			/* count glitches */
unsigned long	glit_nxt;			/* count glitch scan */

/********************************************************************
 *
 *	Procedure pplc
 *
 *******************************************************************/

void
pplc(
    Gate *	g_lists,
    unsigned *	gate_count)
{
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
    int		val;
    char *	format;		/* number format */
    char	ybuf[YSIZE];	/* buffer for number */
    char *	yp;

    error_flag = 0;
    (Gate*)alist0.gt_rlist = a_list = &alist1;	/* initialise alternate */
    Out_init(a_list);
    (Gate*)alist1.gt_rlist = a_list = &alist0;	/* start with alist0 */
    Out_init(a_list);
    (Gate*)olist0.gt_rlist = o_list = &olist1;	/* initialise alternate */
    Out_init(o_list);
    (Gate*)olist1.gt_rlist = o_list = &olist0;	/* start with olist0 */
    Out_init(o_list);
#ifdef LOAD
    c_list = &iClock;				/* system clock list */
#endif

    if (outFP != stdout) {
	fclose(outFP);
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
    maxfd = max (maxfd, fileno (stdin));
    if ( ioctl(0, TCGETA, &ttyparms) == -1 )   {
	fprintf(stderr, "Cannot get termio from stdin\n");
	quit(-5);
    }
#endif

    if (debug & 0100) fprintf(outFP, "\nINITIALISATION\n");
    for (pass = 0; pass < 4; pass++) {
	if (debug & 0100) fprintf(outFP, "\nPass %d:", pass + 1);
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
    if (error_flag) {
	fprintf(outFP, "\n*** Fatal Error ***\n");
	exit(1);
    }
    if (debug & 0100) fprintf(outFP, "\nPass 5:");
    if (o_list->gt_next == o_list) {			/* empty */
	scan_clk(c_list);				/* clock list */
    }
    if (debug & 0100) {
	fprintf(outFP, "\nInit complete =======\n");
    }
    if ((gp = TX_[0]) != 0) {
	if (debug & 0100) fprintf(outFP, "\t%s  1 ==>", gp->gt_ids);
	gp->gt_val = -1;			/* set EOP initially */
	link_ol(gp, o_list);			/* fire Input Gate */
	if (debug & 0100) fprintf(outFP, " -1\n");
    }

    if (debug & 0400) {
	exit(0);				/* terminate - no inputs */
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
    memcpy((void *) &ttyparmh, (void *) &ttyparms, sizeof(struct termio));
    ttyparmh.c_lflag &= ~(ECHO | ICANON);
    if (ioctl(0, TCSETA, &ttyparmh) == -1) quit(-6);
#endif

    dis_cnt = DIS_MAX;
    for ( ; ; ) {
	if (++mark_stamp == 0) {	/* next generation for check */
	    mark_stamp++;		/* leave out zero */
	}

	time_cnt = 0;			/* clear time count */

	do {
	    c = ENTER;
	    /* scan arithmetic and logic output lists until empty */
	    while (scan_ar(a_list) || scan(o_list)) {
		if (debug & 0300) {	/* osc or detailed info */
		    display();		/* inputs and outputs */
		    c = 0;
		}
	    }
	} while (scan_clk(c_list));	/* then scan clock list until empty */

	/*
	 *	alternate list contains all those gates which were marked in
	 *	the previous scan and which were active more than
	 *	MARKMAX times. These are oscillators which wil be
	 *	scanned again in the next cycle.
	 */

	if ((scan_cnt || link_cnt) && (debug & 02000)) {
	    fprintf(outFP, "\nscan = %5d  link = %5d  time = %5d ms  ",
		scan_cnt, link_cnt, time_cnt);
	    if (glit_cnt) {
		fprintf(outFP, "glitch = %5d, %ld  ",
		glit_cnt, glit_nxt);
	    }
	    scan_cnt = link_cnt = glit_cnt = glit_nxt = 0;
	}

	a_list = (Gate*)a_list->gt_rlist;	/* alternate arithmetic list */
	o_list = (Gate*)o_list->gt_rlist;	/* alternate logic list */
	if ((debug & 0200) &&
	    (a_list->gt_next != a_list || o_list->gt_next != o_list)) {
	    fprintf(outFP, "\nOSC =");
	    for (gp = a_list->gt_next; gp != a_list; gp = gp->gt_next) {
		fprintf(outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
	    }
	    for (gp = o_list->gt_next; gp != o_list; gp = gp->gt_next) {
		fprintf(outFP, " %s(#%d),", gp->gt_ids, gp->gt_mcnt);
	    }
	}

	if (c == ENTER) {
	    display();				/* inputs and outputs */
	}

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
		    quit(0);			/* quit normally */
		}
		if (c != ENTER) {
		    putc(c, outFP);		/* echo */
		}
		if (c >= '0' && c < '0' + MAX_IO) {
		    if ((gp = IX_[c - '0']) != 0) {
			gp->gt_val = -gp->gt_val; /* complement input */
			link_ol(gp, o_list);
			if (debug & 0100) {
			    putc(gp->gt_val < 0 ? '1' : '0', outFP);
			}
			cnt--;			/* apply input ? */
		    }
		} else if (c == '+') {
		    cnt++;			/* 1 extra input */
		} else if (c == '-') {
		    if (--cnt <= 0) cnt = 1;	/* at least 1 more in */
		} else if (c == 'b') {
		    if ((gp = IB_[1]) != 0) {
			goto wordIn;
		    } else {
			goto wordEr;		/* input not configured */
		    }
		} else if (c == 'w') {
		    if ((gp = IW_[2]) != 0) {
		    wordIn:
			yp = ybuf;
			if ((c = getch()) == '-') {
			    putc(c, outFP);		/* echo */
			    *yp++ = c;
			    c = getch();
			}
			if (c == '0') {			/* oct or hex or 0 */
			    putc(c, outFP);		/* echo */
			    if ((c = getch()) == 'x' || c == 'X') {
				putc(c, outFP);		/* echo */
				format = "%x%s";	/* hexadecimal */
			    } else {
				format = "%o%s";	/* octal */
				*yp++ = '0';		/* may be a single 0 */
				ungetch(c);
			    }
			} else {
			    format = "%d%s";		/* decimal */
			    ungetch(c);
			}
			while ((c = getch()) != EOF && yp < &ybuf[YSIZE-1] &&
			    (isxdigit(c) || c == '\b')) {
			    if (c == '\b') {
				if (yp > ybuf) {
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
			if (c != ENTER && c != EOF) ungetch(c);
			putc(c, outFP);		/* echo */
			if (gp == IB_[1]) {
			    val &= 0xff;	/* reduce to byte */
			}

			if (val != gp->gt_new && /* first change or glitch */
			((gp->gt_new = val) != gp->gt_old) ^ (gp->gt_next != 0)) {
			    /* arithmetic master action */
			    link_ol(gp, a_list);	/* no ACTIONs */
			}
			cnt--;
		    } else {
		    wordEr:
			putc('?', outFP);	/* input not configured */
			c = ENTER;
			cnt = 0;
		    }
		} else if (c == 'x') {
		    aaflag = 0;			/* hexadecimal output */
		    c = ENTER;
		    cnt = 0;
		} else if (c == 'd') {
		    aaflag = 1;			/* decimal output */
		    c = ENTER;
		    cnt = 0;
		} else if (c == ENTER) {
		    cnt = 0;			/* terminate and scan */
		}
	    }
	}
    }
} /* pplc */

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
	fprintf(outFP, "\n");
	for (n = 0; n < 10; n++) {
	    if ((gp = IX_[n]) != 0) fprintf(outFP, " I%d", n);
	}
	if (IB_[1] != 0) printf("  IB1");
	if (IW_[2] != 0) printf("    IW2");
	fprintf(outFP, "   ");
	for (n = 0; n < MAX_IO; n++) {
	    fprintf(outFP, " Q%d", n);
	}
	fprintf(outFP, "  QB1    QW2");
    }
    fprintf(outFP, "\n");			/* display outputs */
    for (n = 0; n < 10; n++) {
	if ((gp = IX_[n]) != 0) {
	    fprintf(outFP, "  %c", gp->gt_val < 0 ? '1' : '0');
	}
    }
    /* display IB1 and IW2 if active */
    if (aaflag) {
	if ((gp = IB_[1]) != 0) printf(" %4d", gp->gt_old & 0xff);
	if ((gp = IW_[2]) != 0) printf(" %6d", gp->gt_old);
    } else {
	if ((gp = IB_[1]) != 0) printf("   %02x", gp->gt_old & 0xff);
	if ((gp = IW_[2]) != 0) printf("   %04x", gp->gt_old);
    }
    fprintf(outFP, "   ");
    data = *(unsigned short*)QX_;
    for (n = 0; n < MAX_IO; n++) {
	fprintf(outFP, "  %c", (data & 0x0001) ? '1' : '0');
	data >>= 1;				/* scan output bits */
    }
    /* display QB1 and QW2 */
    if (aaflag) {
	fprintf(outFP, " %4d %6d   : ",
	    QX_[1], *(unsigned short*)&QX_[2]);
    } else {
	fprintf(outFP, "   %02x   %04x   : ",
	    QX_[1], *(unsigned short*)&QX_[2]);
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
 *	Quit program with 'q' or ctrlC or Break via signal SIGINT
 *	or program abort on detected bugs.
 *
 *******************************************************************/

void quit(int sig)
{
    if (debug & 03000) {
#ifdef _MSDOS_
	/* reset the old interrupt handler */
#ifdef MSC 
	_dos_setvect(INTR, oldhandler);
#else
	setvect(INTR, oldhandler);
#endif
#endif
    }
#ifndef _MSDOS_
    if (ioctl(0, TCSETA, &ttyparms) == -1) exit(-1);
#endif
    fprintf(errFP, "\n");
    if (sig) {
	fprintf(errFP, "Quit with sig = %d\n", sig);
    }
    exit(sig);			/* really quit */
} /* quit */

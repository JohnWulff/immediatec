static const char pptc_c[] =
"@(#)$Id: pptc.c,v 1.7 2001/01/14 22:37:40 jw Exp $";
/********************************************************************
 *
 *	parallel plc - procedure
 *
 *	Standalone run-time version for TCP/IP DemoBoxes.
 *	Alternatively to produce compiled applications
 *	linked with load.a produced with with LOAD option
 *
 *	branched from "pplc.c,v 1.6 1999/08/06"
 *
 *******************************************************************/

/* J.E. Wulff	3-Mar-85 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#define getch() getchar()
#define ungetch(x) ungetc(x, stdin)
#include <sys/types.h>
#include <sys/time.h>
#include <termio.h>
#include <signal.h>
#include <ctype.h>
#include "pplc.h"
#include "tcpc.h"

#define MAX_IO	8
#define MAX_W	2
#define INTR 0x1c    /* The clock tick interrupt */
#define YSIZE	10

static void	display(void);
static unsigned	time_cnt;			/* count time in ticks */

#define D10	10			/* 1/10 second select timeout under Linux */
#define ENTER	'\n'

static struct termio	ttyparms;
static struct termio	ttyparmh;

# define max(x, y) ((x) > (y) ? (x) : (y))

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

int		sockFN;			/* TCP/IP socket file number */

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
    int		tcnt = D10;
    int		val;
    char *	format;		/* number format */
    char	ybuf[YSIZE];	/* buffer for number */
    char *	yp;
    float	delay = 0.0;	/* timer processing stopped */
    int		retval;

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
#ifndef LOAD
	if (iFlag) {
	    inversionCorrection();
	    iFlag = 0;
	}
#endif
	outFP = stdout;			/* standard output from here */
    }
    if (errFP != stderr) {
	fclose(errFP);
	errFP = stderr;			/* standard error from here */
    }

    if ( ioctl(0, TCGETA, &ttyparms) == -1 )   {
	fprintf(stderr, "Cannot get termio from stdin\n");
	quit(-5);
    }

    for (cnt = 1; cnt < 8; cnt++) {
	if (TX_[cnt] != NULL) {		/* any of the 7 timers programmed ? */
	    delay = timeout;		/* yes - setup timer processing */
	}				/* could optimise by varying delay */
    }

    if ((debug & 0400) == 0) {
	/* Start TCP/IP communication before any inputs are generated => outputs */
	sockFN = connect_to_server(hostNM, portNM, pplcNM, delay);
    }

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
	if (error_flag == 1) {
	    fprintf(outFP, "\n*** Fatal Errors ***\n");
	    exit(1);
	}
	fprintf(outFP, "\n*** Warnings ***\n");
    }
    if (debug & 0100) fprintf(outFP, "\nPass 5:");
    if (o_list->gt_next == o_list) {			/* empty */
	scan_clk(c_list);				/* clock list */
    }
    if (debug & 0100) {
	fprintf(outFP, "\nInit complete =======\n");
    }

    if (debug & 0400) {
	quit(0);				/* terminate - no inputs */
    }

    signal(SIGINT, quit);		/* catch ctrlC and Break */	
    memcpy((void *) &ttyparmh, (void *) &ttyparms, sizeof(struct termio));
    ttyparmh.c_lflag &= ~(ECHO | ICANON);
    if (ioctl(0, TCSETA, &ttyparmh) == -1) quit(-6);

    if ((gp = TX_[0]) != NULL) {
	if (debug & 0100) fprintf(outFP, "\nEOP:\t%s  1 ==>", gp->gt_ids);
	gp->gt_val = -1;			/* set EOP initially */
	link_ol(gp, o_list);			/* fire Input Gate */
	if (debug & 0100) fprintf(outFP, " -1");
    }

    dis_cnt = DIS_MAX;
    if (debug & 0300) {			/* osc or detailed info */
	display();			/* inputs and outputs */
    }

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

	if ((debug & 0300) && c == ENTER) {	/* osc or detailed info */
	    display();				/* inputs and outputs */
	}

/********************************************************************
 *
 *	handle inputs
 *
 *******************************************************************/

	for (cnt = 0; cnt == 0; ) {
	    retval = wait_for_next_event(sockFN);	/* inputs or timer */

	    if (retval == 0) {
		timeoutCounter = timeoutValue;	/* will repeat if timeout with input */
		/*
		 *	TIMERS here every 100 milliseconds
		 */
		if ((gp = TX_[4]) != 0) {		/* 100 millisecond timer */
		    if (debug & 0100) fprintf(outFP, " %s ", gp->gt_ids);
		    gp->gt_val = - gp->gt_val;	/* complement input */
		    link_ol(gp, o_list);
		    if (debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', outFP);
		    cnt++;
		}
		if (--tcnt == 0) {
		    tcnt = D10;
		    if ((gp = TX_[5]) != 0) {	/* 1 second timer */
			if (debug & 0100) fprintf(outFP, " %s ", gp->gt_ids);
			gp->gt_val = - gp->gt_val;	/* complement input */
			link_ol(gp, o_list);
			if (debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', outFP);
			cnt++;
		    }
		}
	    } else if (retval > 0) {
		if (FD_ISSET(sockFN, &rdfds)) {
		    char	rBuf[16];
		    /*
		     *	TCP/IP input
		     */
		    if (rcvd_msg_from_server(sockFN, rBuf, sizeof rBuf)) {
			unsigned int	unit;
			unsigned int	index;
			int		val;

			if (debug & 0200) fprintf(outFP, "%s rcvd %s\n", rBuf, pplcNM);
			if (sscanf(rBuf, "X%d,%d,%d", &unit, &index, &val) == 3) {
			    if (unit < IXD && index < 8 && (gp = IX_[unit*8+index]) != NULL) {
				val = val ? -1 : 1;
				if (val != gp->gt_val) {
				    gp->gt_val = val;		 /* changed input */
				    if (debug & 0100) fprintf(outFP, " %s ", gp->gt_ids);
				    link_ol(gp, o_list);
				    if (debug & 0100) putc(gp->gt_val < 0 ? '1' : '0', outFP);
				    cnt++;
				}
			    }
			} else if (sscanf(rBuf, "B%d,%d", &unit, &val) == 2) {
			    if (unit < IXD && (gp = IB_[unit]) != NULL) {
				val &= 0xff;		/* limit to byte value */
				goto wordIn;
			    }
			} else if (sscanf(rBuf, "W%d,%d", &unit, &val) == 2) {
			    if (unit < IXD && (gp = IW_[unit]) != NULL) {
			    wordIn:
				if (val != gp->gt_new &&	/* first change or glitch */
				((gp->gt_new = val) != gp->gt_old) ^ (gp->gt_next != 0)) {
				    /* arithmetic master action */
				    if (debug & 0100) fprintf(outFP, " %s ", gp->gt_ids);
				    link_ol(gp, a_list);	/* no ACTIONs */
				    if (debug & 0100) fprintf(outFP, "%d", gp->gt_new);
				    cnt++;
				}
			    }
			} else {
			    fprintf(errFP, "ERROR: %s rcvd %s ???\n", rBuf, pplcNM);
			}
		    } else {
			close(sockFN);
			fprintf(errFP, "\n%s disconnected by server\n", pplcNM);
			quit(0);			/* quit normally */
		    }
		} else if (FD_ISSET(0, &rdfds)) {	/* unlikely to have both */
		    /*
		     *	STDIN
		     */
		    if ((c = getch()) == 'q' || c == EOF) {
			quit(0);			/* quit normally */
		    }
		    if (debug & 0300) {			/* osc or detailed info */
			unsigned short flag;
			if (c == 'x') {
			    flag = 1;			/* hexadecimal output */
			} else if (c == 'd') {
			    flag = 0;			/* decimal output */
			}
			if (flag != xflag) {
			    putc(c, outFP);
			    xflag = flag;
			    display();			/* inputs and outputs */
			}
		    }
		    /* ignore the rest for now */
		}
	    } else {				/* retval -1 */
		perror("ERROR: select failed");
		quit(1);				/* ZZZ change number */
	    }
	}
    } /* for ( ; ; ) */
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
	if (IB_[1] != 0) fprintf(outFP, "  IB1");
	if (IW_[2] != 0) fprintf(outFP, "    IW2");
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
    if (!xflag) {
	if ((gp = IB_[1]) != 0) fprintf(outFP, " %4d", gp->gt_old & 0xff);
	if ((gp = IW_[2]) != 0) fprintf(outFP, " %6d", gp->gt_old);
    } else {
	if ((gp = IB_[1]) != 0) fprintf(outFP, "   %02x", gp->gt_old & 0xff);
	if ((gp = IW_[2]) != 0) fprintf(outFP, "   %04x", gp->gt_old);
    }
    fprintf(outFP, "   ");
    data = *(unsigned short*)QX_;
    for (n = 0; n < MAX_IO; n++) {
	fprintf(outFP, "  %c", (data & 0x0001) ? '1' : '0');
	data >>= 1;				/* scan output bits */
    }
    /* display QB1 and QW2 */
    if (!xflag) {
	fprintf(outFP, " %4d %6d   : ",
	    QX_[1], *(unsigned short*)&QX_[2]);
    } else {
	fprintf(outFP, "   %02x   %04x   : ",
	    QX_[1], *(unsigned short*)&QX_[2]);
    }
    fflush(outFP);
} /* display */

/********************************************************************
 *
 *	Quit program with 'q' or ctrlC or Break via signal SIGINT
 *	or program abort on detected bugs.
 *
 *******************************************************************/

void quit(int sig)
{
    fflush(outFP);
    if (ioctl(0, TCSETA, &ttyparms) == -1) exit(-1);
    if ((debug & 0400) == 0) {
	fprintf(errFP, "\n");
    }
    if (sig) {
	fprintf(errFP, "Quit with sig = %d\n", sig);
    }
    if (errFP != stderr) {
	fflush(errFP);
	fclose(errFP);
    }
    exit(sig);			/* really quit */
} /* quit */

static const char pplt_c[] =
"@(#)$Id: pplt.c,v 1.3 2001/01/03 10:49:24 jw Exp $";
/********************************************************************
 *
 *	parallel plc - procedure
 *
 *	Standalone run-time version (LOAD) and compiler version for
 *	Feldbus card.
 *	
 *	J.E. Wulff	3-Mar-85
 *
 *	"pplt.c	1.27	95/02/05"
 *
 *******************************************************************/

#ifdef _WINDOWS
#include <windows.h>
#endif
#include <stdio.h>
#ifndef _WINDOWS
#include <stdlib.h>
#endif
#include <signal.h>
#include "pplc.h"
#include "piot.h"

#ifdef MSC
#define getvect(x)	_dos_getvect(x)
#define setvect(x,y)	_dos_setvect(x,y)
#define inportb(x)	_inp(x)
#define outportb(x,y)	_outp(x,y)
#define INTERRUPT	interrupt far
#else
#define INTERRUPT	interrupt
#endif

Functp	*i_lists[] = { I_LISTS };

#define INTR 0x0f				/* nummer des interrupts */
#define CM 20

Gate		alist0;
Gate		alist1;
Gate *		a_list;			/* these lists are toggled */
Gate		olist0;
Gate		olist1;
Gate *		o_list;			/* these lists are toggled */
Gate *		c_list;			/* "iClock" */

void init_int(void);
int		error_pplt;		/* ZZZ implement proper check */
unsigned char	inout(unsigned char data);
unsigned char	tick;
Gate *		IX_[IXD*8];		/* pointers to Bit Input Gates */
Gate *		IB_[IXD];		/* pointers to Byte Input Gates */
Gate *		IW_[IXD];		/* pointers to Word Input Gates */
Gate *		TX_[TXD*8];		/* pointers to System Bit Gates */
unsigned char	idata[IXD];		/* Input bit field */
unsigned char	QX_[IXD];		/* Output bit field */
unsigned char	pdata[IXD];		/* rest used only locally */
unsigned short	sdata[(IXD-1)/(sizeof(unsigned short)*8)+1];
unsigned short	cdata[(IXD-1)/(sizeof(unsigned short)*8)+1];
unsigned char	conf[CM];
unsigned char	slots;
unsigned char	m50;
unsigned char	m500;

#ifndef _WINDOWS
void INTERRUPT isrtimer(void);	/* isr für 8253-interrupt */
void (INTERRUPT *oldint15)();	/* save old vector */
unsigned	scan_cnt;			/* count scan operations */
unsigned	link_cnt;			/* count link operations */
unsigned	glit_cnt;			/* count glitches */
unsigned long	glit_nxt;			/* count glitch scan */
#else
WORD FAR PASCAL isrtimer(HWND hwnd, UINT message,
                         register WPARAM wParam, LPARAM lParam);
#endif

short		error_flag;

void
pplc(
    Gate *	g_lists,
    unsigned *	gate_count)
{
    short	pass;
    short	typ;
    unsigned	cnt;
    char	val;
#ifdef LOAD
    Gate **	opp;
#else
    unsigned *	gcp;
    Functp * *	ilp;
#endif
    Gate *	gp;
    Functp	init_fa;
    unsigned	i;

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

#ifndef _WINDOWS
    if (debug & 0100) fprintf(outFP, "\nINITIALISATION\n");
#endif
    for (pass = 0; pass < 4; pass++) {
#ifndef _WINDOWS
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
    if (error_flag) {
	if (error_flag == 1) {
#ifndef _WINDOWS
	    fprintf(outFP, "\n*** Fatal Errors ***\n");
#endif
	    return;
	}
#ifndef _WINDOWS
	fprintf(outFP, "\n*** Warnings ***\n");
#endif
    }
#ifndef _WINDOWS
    if (debug & 0100) fprintf(outFP, "\nPass 5:");
#endif
    if (o_list->gt_next == o_list) {			/* empty */
	scan_clk(c_list);				/* clock list */
    }
#ifndef _WINDOWS
    if (debug & 0100) {
	fprintf(outFP, "\nInit complete =======\n");
    }
#endif
    if ((gp = TX_[0]) != 0) {
#ifndef _WINDOWS
	if (debug & 0100) fprintf(outFP, "\nEOP:\t%s  1 ==>", gp->gt_ids);
#endif
	gp->gt_val = -1;			/* set EOP initially */
	link_ol(gp, o_list);			/* fire Input Gate */
#ifndef _WINDOWS
	if (debug & 0100) fprintf(outFP, " -1");
#endif
    }

#ifndef _WINDOWS
    if (debug & 0400) {
	fprintf(outFP, "\n");
	quit(0);			/* terminate - no inputs */
    }
    debug = 0;				/* no further dos I/O */
#endif

/********************************************************************
 *
 *	Serielle Ein/Ausgabe
 *
 *******************************************************************/

#ifndef _WINDOWS
    signal(SIGINT, quit);		/* catch ctrlC and Break */	
    oldint15 = getvect(INTR);		/* alten vektor retten */
    setvect(INTR, isrtimer);		/* pointer verbiegen */
#endif
    init_int();				/* grundinitialisierung */
    outportb(PIO_PC, 0x2);		/* RCK=0, MODE=1, EOP=0 */
    for (i = 0; i < 50000; i++);	/* 40ms warten   */
    outportb(PIO_PC, 0x6);		/* RCK=1, MODE=1, EOP=0 */
    for (tick = 0; tick < CM; tick++) {
	conf[tick] = inout(tick);	/* ersten interrupt freigeben */
    }
    outportb(PIO_PC, 0x1);		/* RCK=0, MODE=0, EOP=1 */
    slots = conf[0];			/* output 3..0 for 8, 1 for 2 */
    if (slots > IXD) slots = IXD;	/* ZZZ just current hardware */
    cdata[0] = slots != 2 ? 0xf0 : 0x01;/* input 7..4 for 8, 0 for 2 */
    for (i = 0; i < 50000; i++);	/* 40ms warten   */
#ifndef _WINDOWS 
    outportb(0x20, 0x20);		/* EOI des 8259A setzen, freigabe */
					/* des nächsten int */
    outportb(0x21,inportb(0x21) & 0x7f);/* int-maske IRQ7 des 8259A enable */
    while (kbhit() == 0);		/* keep it running */
    quit(0);				/* terminate correctly */
#endif
} /* pplc */

/********************************************************************
 *
 *	subroutine zur ein-/ausgabe über den seriellen bus
 *
 *******************************************************************/

unsigned char
inout(unsigned char data)
{
    outportb (PIO_PA, data);		/* datenausgabe */
    while (!(inportb(PIO_PC) & 0x20));	/* noch keine daten verfügbar */
    return (inportb(PIO_PA));		/* gültige daten holen und tschüß */
} /* inout */

/********************************************************************
 *
 *	subroutine zur grundinitialisierung der buskarte
 *
 *******************************************************************/

void
init_int(void)
{

/* 8255A */
/* mode 2, port b -> input */
    outportb(PIO_CMD, PIOMODE2);

#ifndef _WINDOWS 
/* 8253 */
/* zähler 1: */
    outportb(CT_CMD, COUNTER1+SQWAVE);	/* squarewave für interrupt */
    outportb(CT_1, VALUE1_LO);
    outportb(CT_1, VALUE1_HI);
#endif

/* zähler 2: */
    outportb(CT_CMD, COUNTER2+HTSTROBE);/* datenübernahme nach 9tem takt */
    outportb(CT_2, VALUE2_LO);
    outportb(CT_2, VALUE2_HI);

/* zähler 3: */
    outportb(CT_CMD, COUNTER3+ONESHOT); /* tor für 8 takte */
    outportb(CT_3, VALUE3_LO);
    outportb(CT_3, VALUE3_HI);

} /* init_int */

/********************************************************************
 *
 *	interruptserviceroutine
 *
 *******************************************************************/
#ifndef _WINDOWS
void INTERRUPT
isrtimer(void)
#else
WORD FAR PASCAL isrtimer(HWND hwnd, UINT message,
                         register WPARAM wParam, LPARAM lParam)
#endif
{
    register Gate *	gp;
    unsigned char*	idp;
    unsigned char*	odp;
    unsigned char*	pdp;
    unsigned short*	sdp;
    unsigned short*	cdp;
    Gate **		igp;
    Gate **		iBp;
    Gate **		iWp;
    unsigned short	slotMask;
    unsigned char	iData;
    unsigned char	pData;
    int			n;

    /********************************************************************
     *
     *	scan inputs ==> idata, QX_ ==> out
     *
     *	only this part is specific to Kbus hardware
     *
     *******************************************************************/

    outportb(PIO_PC, 0x5);	/* RCK=0, MODE=0, EOP=1 */
    outportb(PIO_PC, 0x1);	/* RCK=0, MODE=0, EOP=1 */

    odp = QX_;			/* Output data to be transmitted */
    idp = idata;		/* Input data from this scan */
    pdp = pdata;		/* input data from Previous scan */
    sdp = sdata;		/* modified input Slots mask */
    *sdp = 0;			/* clear bit field for slots */
    slotMask = 1;		/* start with bit 0 */
    iData = inout(0);		/* start output with a dummy byte */
    for (n = 0; n < slots; n++) {
	if (iData != *pdp++) {
	    *sdp |= slotMask;	/* mark bit for every slot with change */
	}
	if ((slotMask <<= 1) == 0) {
	   *++sdp = 0;		/* next bit field for slots */
	    slotMask = 1;	/* start again with bit 0 */
	}
	*idp++ = iData;
	iData = inout(*odp++);	/* input receives dummy byte at end */
    }
    if (iData != 0) {
	error_pplt++;		/* ZZZ implement proper check */
    }

    /********************************************************************
     *
     *	Efficient algorithm to fire Input Gates from changes in idata.
     *	First groups of 16 slots are checked and ignored if no change.
     *	  This takes care of the many cycles where absolutely no change
     *	  occurs. This way it is OK to scan at high frequency giving
     *	  good resolution.
     *	Next groups of 8 bits per slot are checked.
     *	Only those bits where a change took place fire an Input Gate.
     *	Care has been taken that all pointers stay in array bounds
     *
     *******************************************************************/

    igp = IX_;			/* pointers to Input bit Gates */
    iBp = IB_;			/* pointers to Input byte Gates */
    iWp = IW_;			/* pointers to Input word Gates */
    idp = idata;		/* Input data from this scan */
    pdp = pdata;		/* input data from Previous scan */
    sdp = sdata;		/* modified input Slots mask */
    cdp = cdata;		/* input Configuration mask */
    while (sdp < &sdata[(IXD-1)/(sizeof(unsigned short)*8)+1]) {
	Gate ** isp = igp;
	Gate ** ibp = iBp;
	Gate ** iwp = iWp;
	igp += sizeof slotMask * 8 * 8;
	iBp += sizeof slotMask * 8;
	iWp += sizeof slotMask * 8;
	slotMask = *sdp++ & *cdp++;	/* only configured input slots */
	while (slotMask != 0) {
	    if (slotMask & 0x0001) {	/* ignore if no change in slot */
		register Gate ** ip = isp;
		pData = (iData = *idp++) ^ *pdp;
		*pdp++ = iData;		/* ready for next scan */
		/* cdata COULD weed out slots not programmed initially */
		if ((gp = *ibp) != 0 && /* is slot programmed ? */
		    ((gp->gt_new = iData) != gp->gt_old) ^ (gp->gt_next != 0)) {
		    /* arithmetic master action */
		    link_ol(gp, a_list);	/* COULD use a clock here */
		}
		do {			/* assert pData != 0 */
		    /* ignore Gate if no change or not programmed */
		    if ((pData & 0x80) && (gp = *ip) != 0) {
			if ((iData ^ gp->gt_val) & 0x80) {
			    gp->gt_val = - gp->gt_val; /* complement input */
			    link_ol(gp, o_list);	/* fire Input Gate */
			    /* o_list will be scanned before next input */
			}
		    }
		    ip++;
		    iData <<= 1;
		} while (pData <<= 1);
	    } else {
		idp++;		/* next slot input byte */
		pdp++;		/* next input difference byte */
	    }
	    isp += 8;		/* next group of 8 IX_[] vectors */
	    ibp++;		/* next single IB_[] vector */
	    iwp++;		/* next single IW_[] vector */
	    slotMask >>= 1;	/* next bit in slot mask */
	}
	/* ZZZ adjust idp, pdp like isp */
    }

    if (m50-- == 0) {
#ifndef _WINDOWS 
	m50 = 50-1;				/* 50 mS on 50 mS off */
#else
	m50 = 1-1;				/* 55 mS on 55 mS off */
#endif
	if ((gp = TX_[1]) != 0) {		/* 100 millisecond timer */
	    gp->gt_val = - gp->gt_val;		/* complement input */
	    link_ol(gp, o_list);
	}
	if (m500-- == 0) {
#ifndef _WINDOWS 
	    m500 = 10-1;			/* 500 mS on 500 mS off */
#else
	    m500 = 9-1;				/* 500 mS on 500 mS off */
#endif
	    if ((gp = TX_[2]) != 0) {		/* 1 second timer */
		gp->gt_val = - gp->gt_val;	/* complement input */
		link_ol(gp, o_list);
	    }
	}
    }
    /********************************************************************
     *
     *	pplc scan
     *
     *******************************************************************/

    if (++mark_stamp == 0) {	/* next generation for check */
	mark_stamp++;		/* leave out zero */
    }

    do {
	/* scan arithmetic and logic output lists until empty */
	while (scan_ar(a_list) || scan(o_list));
    } while (scan_clk(c_list));	/* then scan clock list until empty */

    /*
     *	alternate list contains all those gates which were marked in
     *	the previous scan and which were active more than
     *	MARKMAX times. These are oscillators which wil be
     *	scanned again in the next cycle.
     */

#ifndef _WINDOWS
    if ((scan_cnt || link_cnt) && (debug & 02000)) {
	fprintf(outFP, "\nscan = %5d  link = %5d  ",
	    scan_cnt, link_cnt);
	if (glit_cnt) {
	    fprintf(outFP, "glitch = %5d, %ld  ",
	    glit_cnt, glit_nxt);
	}
	scan_cnt = link_cnt = glit_cnt = glit_nxt = 0;
    }
#endif

    a_list = (Gate*)a_list->gt_rlist;	/* alternate arithmetic list */
    o_list = (Gate*)o_list->gt_rlist;	/* alternate logic list */
#ifndef _WINDOWS
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

    outportb(0x20, 0x20);	/* EOI des 8259A setzen, notwendig für */
#else
    return 0;
#endif
} /* isrtimer */

/********************************************************************
 *
 *	Quit program with 'q' or ctrlC or Break via signal SIGINT
 *	or program abort on detected bugs.
 *
 *******************************************************************/

void quit(int sig)
{
#ifndef _WINDOWS 
    if (oldint15) {
	setvect(INTR, oldint15);	/* alten vektor restaurieren */
	oldint15 = 0;
    }
    fprintf(errFP, "\nQuit with sig = %d\n", sig);
    exit(sig);			/* really quit */
#endif
} /* quit */

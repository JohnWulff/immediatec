static const char pcop_c[] =
"@(#)$Id: pcop.c,v 1.3 2001/03/02 12:56:32 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2001  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <john.wulff@inka.de>
 *
 *	pcop.c
 *	parallel plc - runtime execution with Phoenix Contact COP card
 *		       connected to InterBus-S field bus I/O system
 *
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <dos.h>
#include <signal.h>

#define P_8086		/* fuer Include-Dateien notwendig */
#define C_TURBO_C	/* Syntax fuer stdtypes.h, s.S. 4-45 Handbuch */

#include <ibs_dos.h>
#include <ddi_lib.h>
#include "pplc.h"
#include "ibsd.h"
#include "time.h"

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
unsigned char	slots;
unsigned char	m50;
unsigned char	m500;
unsigned short	txw;
unsigned short	txp;

void INTERRUPT isrtimer(void);	/* isr fuer 8253-interrupt */

#ifdef LOAD
unsigned	scan_cnt;			/* count scan operations */
unsigned	link_cnt;			/* count link operations */
unsigned	glit_cnt;			/* count glitches */
unsigned long	glit_nxt;			/* count glitch scan */
unsigned	time_cnt;			/* count time in ms */
#endif

short		error_flag;

/********************************************************************
 *
 *	pplc starter
 *
 *******************************************************************/

void
pplc(
    Gate *	g_lists,
    unsigned *	gate_count)
{
    short	pass;
    short	typ;
#ifdef LOAD
    Gate **	opp;
#else
    unsigned	cnt;
    unsigned *	gcp;
    Functp * *	ilp;
#endif
    Gate *	gp;
    Functp	init_fa;

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
	    return;
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
    if ((gp = TX_[0]) != 0) {
	if (debug & 0100) fprintf(outFP, "\t%s  1 ==>", gp->gt_ids);
	gp->gt_val = -1;			/* set EOP initially */
	link_ol(gp, o_list);			/* fire Input Gate */
	if (debug & 0100) fprintf(outFP, " -1\n");
    }

    if (debug & 0400) {
	fprintf(outFP, "\n");
	quit(-3);			/* terminate - no inputs */
    }

    signal(SIGINT, quit);		/* catch ctrlC and Break */	
    slots = 16;
    cdata[0] = 0xffff;

    if (InitDataLink() == FALSE) quit(-2);
    if (StartIBS() == FALSE) {	/* im Fehlerfall muessen die geoeffneten */
				/* Datenkanaele wieder geschlossen werden */
	quit(-1);			/* CloseDataLink() only */
    }
    tinit();				/* Timer0 mit 1 ms starten */

/********************************************************************
 *
 *	**** do logic control in scanIn() ****
 *	and commumicate with possible supervisory PC
 *
 *******************************************************************/

    for ( ; ; ) {
	register char *	cp;
	unsigned long	l;
	int		c;
	int		af;

	do {
	    if (kbhit()) quit(0);	/* terminate deliberately */
	    scanIn();
	    scanLiveCmd();		/* generate live data */
	} while (!recvMsg());

    newMsg:
	printf("msg = %02x%02x\n", inMsg->base[0], inMsg->base[1]);
	switch (IB_GetMsgCode(inMsg->base)) {

	case GET_SYMBOL_TABLE:
	    IB_SetCmdCode(outMsg->base, ACKNOWLEDGE);	/* confirm */
	    putMsg(c = sTend - sTable);	/* lo byte of sTlength */
	    putMsg(c >> 8);		/* hi byte of sTlength */
	    putMsg(l = sTstrLen);	/* lo byte of sTstrLen */
	    putMsg(l >>= 8);		/* hi1 byte of sTstrLen */
	    putMsg(l >>= 8);		/* hi2 byte of sTstrLen */
	    putMsg(l >> 8);		/* hi3 byte of sTstrLen */
	    printf("sTlength = %d sTstrLen = %ld\n",
		(unsigned)(sTend - sTable), sTstrLen);
	    for (opp = sTable; opp < sTend; opp++) {
		gp = *opp;
		putMsg(gp->gt_ini);	/* -type or ini */
		cp = gp->gt_ids;
		do {
		    if (putMsg(c = *cp++) == 0) goto newMsg; /* may flush */
		} while (c != 0);
	    }
	    if (flushMsg(outMsg) == 0) goto newMsg;
	    break;

	case GET_END:
	    printf("GET_END reveived\n");
	    /* this telegram gets system back to kbhit() correctly */
	    /* otherwise pplc loops scanIn() in fillMsg() for ever */
	    /* ZZZ can implement mechanism to check correct block end */
	    break;			/* no ACKNOWLEDGE */

	case PUT_LIVE_CMD:
	    recvLiveCmd();
	    break;			/* no ACKNOWLEDGE here */

	default:
	    IB_SetCmdCode(outMsg->base, MSG_ERROR);	/* error message */
	    printf("MSG_ERROR\n");
	    sendMsg(2);
	}
    }
    /* never exits here */
} /* pplc */

/********************************************************************
 *
 *	Quit program with 'q' or ctrlC or Break via signal SIGINT
 *	or program abort on detected bugs.
 *
 *******************************************************************/

void
quit(int sig)
{
    if (sig >= 0) {
	trestore();		/* Timer 0 default values, restore vector */
	StopIBS();
    }
    if (sig >= -1) {
	CloseDataLink();	/* closes channels, restore INT vector */
    }
    fprintf(outFP, "\nQuit with sig = %d\n", sig);
    exit(sig);			/* really quit */
			/* ZZZ quit under windows not correct */
} /* quit */

/********************************************************************
 *
 *	pplc input scan and execution
 *
 *******************************************************************/

void
scanIn(void)
{
    register Gate *	gp;
    unsigned char*	DIp;
    unsigned char*	DOp;
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
     *******************************************************************/

    if (IBSsendData() != TRUE) quit(3);	/* error exit 3 */
    if (IBSrecvData() != TRUE) quit(4);	/* error exit 4 */

    DIp = (unsigned char*)DataIn;	/* IBS binaere Eingaben */
    DOp = (unsigned char*)DataOut;	/* IBS binaere Ausgaben */
    odp = QX_;			/* Output data to be transmitted */
    idp = idata;		/* Input data from this scan */
    pdp = pdata;		/* input data from Previous scan */
    sdp = sdata;		/* modified input Slots mask */
    *sdp = 0;			/* clear bit field for slots */
    slotMask = 1;		/* start with bit 0 */
    for (n = 0; n < slots; n++) {
	iData = *DIp++;
	if (iData != *pdp++) {
	    *sdp |= slotMask;	/* mark bit for every slot with change */
	}
	if ((slotMask <<= 1) == 0) {
	   *++sdp = 0;		/* next bit field for slots */
	    slotMask = 1;	/* start again with bit 0 */
	}
	*idp++ = iData;
	*DOp++ = *odp++;
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
		    if ((pData & 1) && (gp = *ip) != 0 &&
			(iData ^ (unsigned char)gp->gt_val >> 7)) {
			/* last test is for security - should be true */
			gp->gt_val = - gp->gt_val; /* complement input */
			link_ol(gp, o_list);	/* fire Input Gate */
			/* o_list will be scanned before next input */
		    }
		    ip++;
		    iData >>= 1;
		} while (pData >>= 1);
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

    txp ^= txw;				/* timer changes */
    if ((txp & 1 << 1) &&		/* test bit 1 */
	(gp = TX_[1]) != 0 &&		/* 100 millisecond timer */
	((txw << 7 - 1 ^ gp->gt_val) & 1 << 7)) {
	gp->gt_val = - gp->gt_val;	/* complement input */
	link_ol(gp, o_list);
    }
    if ((txp & 1 << 2) &&		/* test bit 2 */
	(gp = TX_[2]) != 0 &&		/* 1 second timer */
	((txw << 7 - 2 ^ gp->gt_val) & 1 << 7)) {
	gp->gt_val = - gp->gt_val;	/* complement input */
	link_ol(gp, o_list);
    }
    txp = txw;				/* update compare register */

    /********************************************************************
     *
     *	pplc scan
     *
     *******************************************************************/

    if (++mark_stamp == 0) {	/* next generation for check */
	mark_stamp++;		/* leave out zero */
    }

#ifdef LOAD
    time_cnt = 0;			/* clear time count */
#endif

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

#ifdef LOAD
    if ((scan_cnt || link_cnt) && (debug & 02000)) {
	fprintf(outFP, "\nscan = %5d  link = %5d  time = %5d ms  ",
	    scan_cnt, link_cnt, time_cnt);
	if (glit_cnt) {
	    fprintf(outFP, "glitch = %5d, %ld  ",
	    glit_cnt, glit_nxt);
	}
	scan_cnt = link_cnt = glit_cnt = glit_nxt = 0;
    }
#endif

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
} /* scanIn */

/********************************************************************
 *
 *	timer interrupt service routine
 *
 *******************************************************************/

void INTERRUPT
isrtimer(void)
{
    register Gate *	gp;

#ifdef LOAD
    time_cnt++;				/* count time in ms */
#endif
    if (m50-- == 0) {
	m50 = 50-1;				/* 50 mS on 50 mS off */
	txw ^= 1 << 1;				/* 100 ms timer toggle bit 1 */
	if (m500-- == 0) {
	    m500 = 10-1;			/* 500 mS on 500 mS off */
	    txw ^= 1 << 2;			/* 1 sec timer toggle bit 2 */
	}
    }
    outportb(0x20, 0x20);			/* EOI des 1. 8259A setzen */
    outportb(0x21, inportb(0x21) & 0xfe);	/* Int-maske IRQ0 enable */
} /* isrtimer */

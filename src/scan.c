static const char scan_c[] =
"@(#)$Id: scan.c,v 1.35 2008/06/25 21:48:28 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2008  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <ic@je-wulff.de>
 *
 *	scan.c
 *	parallel plc - runtime execution - scan action lists
 *		     - do gate function, master and slave actions
 *
 *******************************************************************/

/* J.E. Wulff	3-Mar-85 */

#include	<stdio.h>
#include	"icc.h"

static void	link_c(Gate * gp, Gate * out_list);

/********************************************************************
 *
 *	The following 5 arrays are indexed by gt_fni (is ftype)
 *
 *	UDFA	ARITH	GATE	GATEX	RI_BIT	S_SH	R_SH	D_SH
 *	F_SW	S_FF	R_FF	D_FF	CH_BIT	F_CF	F_CE	CLCK	TIMR
 *	TRAB	OUTW	OUTX	CLCKL	TIMRL
 *
 *******************************************************************/

static iC_Functp2 initAct[] = {			/* called in pass4 */
		    INIT_ACT
		};

static iC_Functp2 masterAct[] = {		/* called in scan, scan_ar and pass4 */
		    MASTER_ACT
		};

static iC_Functp2 slaveAct[] = {		/* called in scan_clk */
		    SLAVE_ACT
		};

static iC_Functp init2[] = {			/* called in pass2 */
		    INIT2_ACT
		};

unsigned int	iC_bit2[] = {			/* used in i_ff2() and i_ff3() */
		    BIT2_LST
		};

Gate *		iC_gx;	/* used to point to action Gate in chMbit riMbit */
#if YYDEBUG && !defined(_WINDOWS)
short		iC_dc;	/* debug display counter in scan and rsff */
#endif	/* YYDEBUG && !defined(_WINDOWS) */

/********************************************************************
 *
 *	Scan of nodes on an arithmetic action list
 *
 *	The nodes op on the a_list are all of ftype ARITH and the target
 *	nodes gp are all of type ARN + which have a cexe function.
 *
 *	The target nodes gp can trigger both arithmetic and logical actions.
 *
 *	The actions ARITH, D_SH, F_SW and CH_BIT require arithmetic processing.
 *	CH_BIT has a logical value which is triggered for every change in
 *	numerical value. The change is stored in CH_BIT when slave is clocked.
 *
 *	The remaining actions are first converted from int to bit
 *	and require logical processing only.
 *
 *******************************************************************/

void
iC_scan_ar(Gate *	out_list)
{
#if INT_MAX == 32767 && defined (LONG16)
    long		val;
#else	/* INT_MAX == 32767 && defined (LONG16) */
    int			val;
#endif	/* INT_MAX == 32767 && defined (LONG16) */
    Gate *		gp;
    Gate **		lp;
    Gate *		op;

#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
	fprintf(iC_outFP, "\narith scan ==========");
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    while ((op = out_list->gt_next) != out_list) {	/* scan outputs */
#ifndef DEQ
	out_list->gt_next = op->gt_next;		/* unlink from */
	op->gt_next = 0;				/* output list */
	if (op == (Gate *)out_list->gt_list) {		/* last entry ? */
	    out_list->gt_list = (Gate **)out_list;	/* fix pointer */
	}
#else	/* DEQ */
	out_list->gt_next = gp = op->gt_next;	/* list ==> next */
	gp->gt_prev = out_list;			/* list <== next */
	op->gt_next = op->gt_prev = 0;		/* unlink Gate */
#endif	/* DEQ */
	/********************************************************************
	 * up to this point changes in gt_new back to gt_old could have
	 * caused this Gate to be unlinked (glitch), resulting in the
	 * following update executions of arithmetic functions not
	 * being done. They are then no longer necessary, since no
	 * change in the function results would occurr.
	 *******************************************************************/
	op->gt_old = op->gt_new;		/* now new value is fixed */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
#if INT_MAX == 32767 && defined (LONG16)
	    fprintf(iC_outFP, "\n%s: %ld", op->gt_ids, op->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
	    fprintf(iC_outFP, "\n%s: %d", op->gt_ids, op->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	    iC_dc = 0;
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
	if (op->gt_live & 0x8000) {
	    iC_liveData(op->gt_live, op->gt_new);	/* live is active */
	}
#endif	/* defined(TCP) || defined(LOAD) */
	lp = op->gt_list;
	while ((gp = *lp++) != 0) {
#ifdef LOAD
	    iC_CFunctp exec;
#endif	/* LOAD */
	    iC_scan_cnt++;			/* count scan operations */
#if YYDEBUG && !defined(_WINDOWS)
	    iC_gx = gp;				/* save old gp in iC_gx */
	    if (iC_debug & 0100) {
		if (iC_dc++ >= 4) {
		    iC_dc = 1;
		    putc('\n', iC_outFP);
		}
#if INT_MAX == 32767 && defined (LONG16)
		fprintf(iC_outFP, "\t%s %ld ==>", gp->gt_ids,
		    gp->gt_fni == ARITH || gp->gt_fni == D_SH || gp->gt_fni == F_SW ||
		    gp->gt_fni == CH_BIT || gp->gt_fni == OUTW ?
		    gp->gt_new : (long)gp->gt_val);
#else	/* INT_MAX == 32767 && defined (LONG16) */
		fprintf(iC_outFP, "\t%s %d ==>", gp->gt_ids,
		    gp->gt_fni == ARITH || gp->gt_fni == D_SH || gp->gt_fni == F_SW ||
		    gp->gt_fni == CH_BIT || gp->gt_fni == OUTW ?
		    gp->gt_new : gp->gt_val);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#ifdef LOAD
	    if ((exec = (iC_CFunctp)*(gp->gt_rlist)) != 0) {
		val = exec(gp);			/* compute arith expression */
	    }
#else	/* LOAD */
	    if (gp->gt_rlist) {
		val = iC_exec(gp->gt_rfunctn, gp);	/* must pass both -/+ */
	    }
#endif	/* LOAD */
	    else {
		val = op->gt_new;		/* pass value to master output */
	    }
	    /************************************************************
	     *
	     *	For any C_expressions, the computed value is returned.
	     *
	     ***********************************************************/

	    if (gp->gt_fni == ARITH || gp->gt_fni == D_SH || gp->gt_fni == F_SW ||
		gp->gt_fni == CH_BIT || gp->gt_fni == OUTW) {
		if (val != gp->gt_new) {
		    gp->gt_new = val;
		    (*masterAct[gp->gt_fni])(gp, iC_a_list); /* arithmetic master action */
		}
	    } else if ((val = val ? -1 : 1) != gp->gt_val) {
		gp->gt_val = val;		/* convert val to logic value */
		/* logic master action */
		(*masterAct[gp->gt_fni])(gp, iC_o_list);
	    }
#if YYDEBUG && !defined(_WINDOWS)
	    /* global iC_gx is modified in arithmetic chMbit() master action */
#if INT_MAX == 32767 && defined (LONG16)
	    if (iC_debug & 0100) fprintf(iC_outFP, " %ld",
		    iC_gx->gt_fni == ARITH || iC_gx->gt_fni == D_SH || iC_gx->gt_fni == F_SW ||
		    iC_gx->gt_fni == CH_BIT || iC_gx->gt_fni == OUTW ?
		    iC_gx->gt_new : (long)iC_gx->gt_val);
#else	/* INT_MAX == 32767 && defined (LONG16) */
	    if (iC_debug & 0100) fprintf(iC_outFP, " %d",
		    iC_gx->gt_fni == ARITH || iC_gx->gt_fni == D_SH || iC_gx->gt_fni == F_SW ||
		    iC_gx->gt_fni == CH_BIT || iC_gx->gt_fni == OUTW ?
		    iC_gx->gt_new : iC_gx->gt_val);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	}
    }
} /* scan_ar */

/********************************************************************
 *
 *	Scan of nodes on a logic action list
 *
 *	The nodes op on the o_list are all of ftype GATE and the target
 *	nodes gp are all of types AND, OR, LATCH or XOR.
 *
 *	The target nodes gp can trigger both arithmetic and logical actions.
 *
 *	The target actions for ARITH and D_SH cause logic to arithmetic
 *	conversion in the masterAct functions if called from o_list
 *	This is faster for the bulk of the plain logic actions which
 *	would be slowed down by an extra test in the scan.
 *
 *	The target action for GATE is to simply call iC_link_ol().
 *
 *******************************************************************/

void
iC_scan(Gate *	out_list)
{
    int			val;
    Gate *		gp;
    Gate **		lp;
    Gate *		op;

#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
	fprintf(iC_outFP, "\nlogic scan ==========");
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    while ((op = out_list->gt_next) != out_list) {	/* scan outputs */
#ifndef DEQ
	out_list->gt_next = op->gt_next;		/* unlink from */
	op->gt_next = 0;				/* output list */
	if (op == (Gate *)out_list->gt_list) {		/* last entry ? */
	    out_list->gt_list = (Gate **)out_list;	/* fix pointer */
	}
#else	/* DEQ */
	out_list->gt_next = gp = op->gt_next;	/* list ==> next */
	gp->gt_prev = out_list;			/* list <== next */
	op->gt_next = op->gt_prev = 0;		/* unlink Gate */
#endif	/* DEQ */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\n%s:", op->gt_ids);
	    iC_dc = 0;
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
	if (op->gt_live & 0x8000) {
	    iC_liveData(op->gt_live, op->gt_val < 0 ? 1 : 0);	/* live is active */
	}
#endif	/* defined(TCP) || defined(LOAD) */
	/************************************************************
	 * In the following code all proccessing for normal and
	 * inverted outputs ar coded twice (identically) to speed up
	 * exeution a little.
	 ***********************************************************/
	lp = op->gt_list;
	/************************************************************
	 * GATE logical types AND OR LATCH (LOGC never occurrs)
	 * op->gt_val is either -1 or +1 when a gate fires and it is
	 * linked to the tail of the action list.
	 * By the time the gate has reached the head of the action
	 * list, it may have been modified either + or -.
	 ***********************************************************/
	val = (op->gt_val < 0) ? -1 : 1;
#ifdef LOAD
	if (op->gt_fni == GATE) {
	    /********************************************************************
	     * ftype == GATE - no targets will be XOR
	     *******************************************************************/
	    /* do twice: once with val, then whith -val */
	    while ((gp = *lp++) != 0) { /* scan non-inverted outputs */
		iC_scan_cnt++;			/* count scan operations */
#if YYDEBUG && !defined(_WINDOWS)
		iC_gx = gp;			/* save old gp in iC_gx */
		if (iC_debug & 0100) {
		    if (iC_dc++ >= 4) {
			iC_dc = 1;
			putc('\n', iC_outFP);
		    }
		    fprintf(iC_outFP, "\t%s %2d ==>", gp->gt_ids, gp->gt_val);
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		/************************************************************
		 * GATE logical types AND OR LATCH for normal outputs
		 ***********************************************************/
		if ((gp->gt_val += val) == 0) {	/* gate function */
		    gp->gt_val = val;		/* val is logic value */
		    (*masterAct[gp->gt_fni])(gp, iC_o_list);/* master action */
		}
#if YYDEBUG && !defined(_WINDOWS)
		/* global iC_gx is modified in riMbit() and chMbit() master action */
		if (iC_debug & 0100) fprintf(iC_outFP, " %d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    }
	    /************************************************************
	     * GATE logical types AND OR LATCH for inverted outputs
	     ***********************************************************/
	    val = -val;				/* invert logic value */
	    while ((gp = *lp++) != 0) { /* scan inverted outputs */
		iC_scan_cnt++;			/* count scan operations */
#if YYDEBUG && !defined(_WINDOWS)
		iC_gx = gp;			/* save old gp in iC_gx */
		if (iC_debug & 0100) {
		    if (iC_dc++ >= 4) {
			iC_dc = 1;
			putc('\n', iC_outFP);
		    }
		    fprintf(iC_outFP, "\t%s %2d ==>", gp->gt_ids, gp->gt_val);
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		/************************************************************
		 * GATE logical types AND OR LATCH for inverted outputs
		 ***********************************************************/
		if ((gp->gt_val += val) == 0) {	/* gate function */
		    gp->gt_val = val;		/* val is inverted value */
		    (*masterAct[gp->gt_fni])(gp, iC_o_list);/* master action */
		}
#if YYDEBUG && !defined(_WINDOWS)
		/* global iC_gx is modified in riMbit() and chMbit() master action */
		if (iC_debug & 0100) fprintf(iC_outFP, " %d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    }
	} else {
#endif	/* LOAD */
	    /********************************************************************
	     * ftype == GATEX - some targets may be XOR - set at load time
	     * or direct execution, in which case ftype GATE handles XOR AND ...
	     *******************************************************************/
	    /* do twice: once with val, then whith -val */
	    while ((gp = *lp++) != 0) { /* scan non-inverted outputs */
		iC_scan_cnt++;			/* count scan operations */
#if YYDEBUG && !defined(_WINDOWS)
		iC_gx = gp;			/* save old gp in iC_gx */
		if (iC_debug & 0100) {
		    if (iC_dc++ >= 4) {
			iC_dc = 1;
			putc('\n', iC_outFP);
		    }
		    fprintf(iC_outFP, "\t%s %2d ==>", gp->gt_ids, gp->gt_val);
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		if (gp->gt_ini != 0) {		/* trigger for XOR (UDF) */
		    /************************************************************
		     * GATE logical types AND OR LATCH for normal outputs
		     ***********************************************************/
		    if ((gp->gt_val += val) == 0) {	/* gate function */
			gp->gt_val = val;		/* val is logic value */
			(*masterAct[gp->gt_fni])(gp, iC_o_list);/* master action */
		    }
		} else {
		    /************************************************************
		     * XOR processing for normal outputs
		     * xor gates are initialised to 0 for 2, 3 or more 0 inputs.
		     * Every time one of the inputs changes from a 0 to a 1 or
		     * from a 1 to a 0, the output is toggled.
		     * This is independent of the value of the current input.
		     ***********************************************************/
		    gp->gt_val ^= 0x80;		/* xor independent of change */
		    (*masterAct[gp->gt_fni])(gp, iC_o_list);/* master action */
		}
#if YYDEBUG && !defined(_WINDOWS)
		/* global iC_gx is modified in riMbit() and chMbit() master action */
		if (iC_debug & 0100) fprintf(iC_outFP, " %d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    }
	    /************************************************************
	     * GATE logical types AND OR LATCH for inverted outputs
	     ***********************************************************/
	    val = -val;				/* invert logic value */
	    while ((gp = *lp++) != 0) { /* scan inverted outputs */
		iC_scan_cnt++;			/* count scan operations */
#if YYDEBUG && !defined(_WINDOWS)
		iC_gx = gp;			/* save old gp in iC_gx */
		if (iC_debug & 0100) {
		    if (iC_dc++ >= 4) {
			iC_dc = 1;
			putc('\n', iC_outFP);
		    }
		    fprintf(iC_outFP, "\t%s %2d ==>", gp->gt_ids, gp->gt_val);
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		if (gp->gt_ini != 0) {		/* trigger for XOR (UDF) */
		    /************************************************************
		     * GATE logical types AND OR LATCH for inverted outputs
		     ***********************************************************/
		    if ((gp->gt_val += val) == 0) {	/* gate function */
			gp->gt_val = val;		/* val is inverted value */
			(*masterAct[gp->gt_fni])(gp, iC_o_list);/* master action */
		    }
		} else {
		    /************************************************************
		     * XOR processing for inverted outputs
		     ***********************************************************/
		    gp->gt_val ^= 0x80;		/* xor independent of change */
		    (*masterAct[gp->gt_fni])(gp, iC_o_list);/* master action */
		}
#if YYDEBUG && !defined(_WINDOWS)
		/* global iC_gx is modified in riMbit() and chMbit() master action */
		if (iC_debug & 0100) fprintf(iC_outFP, " %d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    }
#ifdef LOAD
	}
#endif	/* LOAD */
    }
} /* iC_scan */

/********************************************************************
 *
 *	Scan of nodes on a clock action list
 *
 *	The nodes gp on the c_list are all actions of ftype D_SH - F_CF
 *	or CLCKL or TIMRL.
 *
 *	The appropriate slaveAct function is executed in this scan.
 *	When the slave action is a CLCKL or TIMRL action, the action
 *	nodes on their lists will be linked to c_list extending the
 *	clock scan immediately.
 *
 *	The execution of F_SW, F_CF and F_CE (IF ELSE or SWITCH) are
 *	deferred until all other slave actions have been executed, by
 *	linking them to f_list, which is scanned when c_list is empty.
 *
 *	All other slave actions act on GATE or ARITH (by D_SH only)
 *	nodes which are linked to o_list or a_list in the slaveAct.
 *
 *	Finally f_list is scanned when all other actions are completed and
 *	all slave outputs have their final value for this clock scan.
 *	F_SW, F_CF and F_CE execute C fragments as slaveAct when they
 *	are clocked. (IF ELSE or SWITCH). No output is linked directly
 *	to any action lists. The C fragments may assign to immediate
 *	gates of type LOGC or ARNC which are linked to the o_list or
 *	a_list respectively in the assignment if they change.
 *	The execution of C fragments during the scan of f_list, is then
 *	the start of a new run of combinatorial changes if immediate
 *	variables are altered, apart from the side effects of the C code.
 *
 *******************************************************************/

void
iC_scan_clk(Gate *	out_list)		/* scan a clock list */
{
    Gate *	op;
#ifdef DEQ
    Gate *	gp;
#endif	/* DEQ */

#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
	fprintf(iC_outFP, out_list == iC_c_list ? "\nclock scan =========="
					  : "\nfunct scan ==========");
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    while ((op = out_list->gt_next) != out_list) {	/* scan outputs */
#ifndef DEQ
	out_list->gt_next = op->gt_next;		/* unlink from */
	op->gt_next = 0;				/* output list */
	if (op == (Gate *)out_list->gt_list) {		/* last entry ? */
	    out_list->gt_list = (Gate **)out_list;	/* fix pointer */
	}
#else	/* DEQ */
	out_list->gt_next = gp = op->gt_next;	/* list ==> next */
	gp->gt_prev = out_list;			/* list <== next */
	op->gt_next = op->gt_prev = 0;		/* unlink Gate */
#endif	/* DEQ */
	iC_scan_cnt++;				/* count scan operations */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\n%s:", op->gt_ids);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	/* only fScf() and fSsw() require out_list to switch to f_list */
	(*slaveAct[op->gt_fni])(op, out_list);	/* execute slave action */
    }
} /* scan_clk */

/********************************************************************
 *
 *	scan_snd
 *
 *******************************************************************/

void
iC_scan_snd(Gate *	out_list)
{
    Gate *	op;
#ifdef DEQ
    Gate *	gp;
#endif	/* DEQ */

#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
	fprintf(iC_outFP, "\nsend scan ==========");
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    while ((op = out_list->gt_next) != out_list) {
#ifndef DEQ
	out_list->gt_next = op->gt_next;		/* unlink from */
	op->gt_next = 0;				/* output list */
	if (op == (Gate *)out_list->gt_list) {		/* last entry ? */
	    out_list->gt_list = (Gate **)out_list;	/* fix pointer */
	}
#else	/* DEQ */
	out_list->gt_next = gp = op->gt_next;	/* list ==> next */
	gp->gt_prev = out_list;			/* list <== next */
	op->gt_next = op->gt_prev = 0;		/* unlink Gate */
#endif	/* DEQ */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\n%s:\t", op->gt_ids);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	iC_outMw(op, out_list);			/* Master action is always iC_outMw() */
	iC_scan_cnt++;				/* count scan operations */
    }
} /* iC_scan_snd */

/********************************************************************
 *
 *	Pass 1 initialisation for all nodes.
 *
 *	Clear the logic value to zero. Used to count links.
 *
 *******************************************************************/

void
iC_pass1(Gate * op, int typ)			/* Pass1 init on gates */
{
    /* op->gt_mcnt is compiled or generated with 0, */
    /* except for _f0_1, where it is and should be 1 */
#ifndef DEQ
    op->gt_next = 0;				/* clear link */
#else	/* DEQ */
    op->gt_next = op->gt_prev = 0;		/* unlink Gate */
#endif	/* DEQ */
} /* pass1 */

/********************************************************************
 *
 *	Pass 2 initialisation for all nodes.
 *
 *	Call a routine which depends on the ftype of this node.
 *	For Gates this is gate2().
 *
 *******************************************************************/

void
iC_pass2(Gate * op, int typ)			/* Pass2 init on gates */
{
    (*init2[op->gt_fni])(op, typ);		/* call pass2 function init */
} /* pass2 */

/********************************************************************
 *
 *	Pass 2 initialisation for Gates via gt_fni == GATE in pass2().
 *	or gt_fni == ARITH.
 *
 *	Count the connections in each Gate to which this Gate is
 *	connected. Limit the maximum to PPGATESIZE (127) for GATE.
 *	In Pass 3 every simple Gate will contain the count of how
 *	many Gates operate on that Gate.
 *
 *******************************************************************/

void
iC_gate2(Gate * op, int typ)			/* pass2 function init gates */
{
    int	cnt;
    Gate **	lp;
    Gate *	gp;

    if ((lp = op->gt_list) != 0) {
	cnt = (op->gt_fni < MAX_AR) ? 1 : 2;
	do {					/* for normal and inverted */
	    while ((gp = *lp++) != 0) {		/* init connected gates */
		if (gp->gt_mcnt++ >= PPGATESIZE) {
		    fprintf(iC_outFP, "\nError:\ttoo many inputs on gate: %s",
			gp->gt_ids);
		    iC_error_flag = 1;		/* cannot execute with this error */
		}				/* error message only once */
	    }
	} while (--cnt);
    } else {
	fprintf(iC_outFP,
	    "\nWarning:	gate %s has no output list", op->gt_ids);
	    iC_error_flag = 2;			/* can execute with this warning */
    }
} /* gate2 */

/********************************************************************
 *
 *	Pass 3 initialisation for Gates. Called directly for types.
 *
 *	The connection count is reported and a Warning issued if zero
 *	for types other than LOGC or ARNC.
 *	For AND the initial value for gt_val is this count. gt_val is
 *	set as follows:
 *	gt_ini	gt_val
 *	   0	   0	for UDF (because number of inputs is 0 !!)
 *	   0	   1	for XOR
 *	   1	   1	for OR ARN ARNC LOGC
 *	   n	   n	for AND where n is number of inputs
 *	(n+1)/2	(n+1)/2	for LATCH
 *	Multi input LATCHes are handled correctly (not supported yet).
 *	In all cases this corresponds to the state when all inputs
 *	are LO. Since gt_val is set to a positive count in every
 *	instance, this assumption is true for all Gates at the end
 *	of pass 3.
 *
 *	For the reverse logic check the initial value of gt_val is
 *	saved in gt_ini for UDF AND OR and LATCH. (XOR requires 0 ??)
 *	Really only required for AND and LATCH, all others assume 1.
 *
 *******************************************************************/

void
gate3(Gate * gp, int typ)			/* Pass3 init on gates */
{
    unsigned char opt = iC_os[typ];
    if (gp->gt_mcnt == 0 && typ != LOGC && typ != ARNC) {
	fprintf(iC_outFP,
	    "\nWarning:    %c	%s\thas no input connections",
	    opt, gp->gt_ids);
	    iC_error_flag = 2;			/* can execute with this warning */
    } else {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\n	    %c	%s:\t%d inputs",
		opt, gp->gt_ids, gp->gt_mcnt);
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	switch (typ) {
	case ARNC:
	case ARN:
	    gp->gt_new = gp->gt_old = 0;
	    /* fall through */
	case LOGC:
	    gp->gt_val = 1;
	    break;
	case XOR:
	    gp->gt_ini = 0;			/* set XOR gates to 0/1 */
	    gp->gt_val = 1;
	    break;
	case UDF:				/* number of inputs is 0 !! */
	case AND:
	    gp->gt_ini = gp->gt_val = gp->gt_mcnt;	/* AND set to number of inputs */
	    break;
	case OR:
	    gp->gt_ini = gp->gt_val = 1;	/* set OR gates to +1 */
	    break;
	case LATCH:
	    gp->gt_ini = gp->gt_val = (int)(gp->gt_mcnt + 1) >> 1;	/* set LATCH gates */
	    break;
	default:
	    gp->gt_val = 0;			/* should not happen */
	    break;
	}
	gp->gt_mcnt = 0;			/* clear gt_mcnt */
	gp->gt_live = 0;			/* clear live flag */
    }
} /* gate3 */

iC_Functp		iC_gate_i[] = {iC_pass1, iC_pass2, gate3, iC_pass4, };

/********************************************************************
 *
 *	Pass 4 initialisation for all logic and arithmetic Gates.
 *
 *	Do a special logic scan on all inverted connections only. At
 *	the end of pass 3 all logic Gates have been initialised to LO.
 *	Each non-inverted connection to other Gates acts as a LO
 *	signal on that Gate which corresponds to the original
 *	assumption. Thus no Gate action needs to be carried out for
 *	non-inverted connections.
 *
 *	Each inverted connection to other Gates acts as a HI signal
 *	which means that a Gate action must be carried out for all
 *	inverted connections. This applies to XOR gates also. Outputs
 *	which change are linked (or unlinked in case of a glitch). All
 *	clocked and timer actions are linked to c_list immediately,
 *	rather than to some other clock or timer. This is done with
 *	the assumptions that the clock or timer action took place
 *	well before turn on. This process simulates what happens in
 *	logic hardware when power is applied.
 *
 *	Each arithmetic output is initialised to 0. Arithmetic
 *	outputs only act on gates of type ARN (+), but these have
 *	functions which may produce results other than 0. Thus they
 *	must be evaluated in this pass and those which do change
 *	are linked, and those which change back are unlinked - Same
 *	as in scan_ar().
 *
 *	Clocked and timer arithmetic actions are processed with their
 *	arithmetic masterAct and thus go on their own clock or timer
 *	list. This is needed particularly for a D_SH action, whose
 *	slave may be simultaneously acted on by S_SH or R_SH, which
 *	should come first on start up.
 *
 *	Clocked 'switch' (F_SW) and 'else' actions (F_CE) require special
 *	treatment. They must be executed once on startup, even if the
 *	input is 0, because the switch or else C fragment may assign
 *	values, even when the input is 0. Clocked 'if' (F_CF) does not
 *	need this.
 *
 *******************************************************************/

void
iC_pass4(Gate * op, int typ)			/* Pass4 init on gates */
{
    Gate **	lp;
    Gate *	gp;
#if INT_MAX == 32767 && defined (LONG16)
    long	val;
#else	/* INT_MAX == 32767 && defined (LONG16) */
    int		val;
#endif	/* INT_MAX == 32767 && defined (LONG16) */

    if (op->gt_fni < MIN_ACT) {			/* UDFA, ARITH, GATE */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\ninit %s:", op->gt_ids);
	    iC_dc = 0;
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	lp = op->gt_list;
	if (op->gt_fni == ARITH) {
	    /************************************************************
	     * Scan arithmetic outputs
	     ***********************************************************/
	    while ((gp = *lp++) != 0) {		/* do arithmetic outputs */
#ifdef LOAD
		iC_CFunctp exec;
#endif	/* LOAD */
		iC_scan_cnt++;			/* count scan operations */
#if YYDEBUG && !defined(_WINDOWS)
		iC_gx = gp;			/* save old gp in iC_gx */
		if (iC_debug & 0100) {
		    if (iC_dc++ >= 4) {
			iC_dc = 1;
			putc('\n', iC_outFP);
		    }
#if INT_MAX == 32767 && defined (LONG16)
		    fprintf(iC_outFP, "\t%s %ld ==>", gp->gt_ids,
			gp->gt_fni == ARITH || gp->gt_fni == D_SH || gp->gt_fni == F_SW ||
			gp->gt_fni == CH_BIT || gp->gt_fni == OUTW ?
			gp->gt_new : (long)gp->gt_val);
#else	/* INT_MAX == 32767 && defined (LONG16) */
		    fprintf(iC_outFP, "\t%s %d ==>", gp->gt_ids,
			gp->gt_fni == ARITH || gp->gt_fni == D_SH || gp->gt_fni == F_SW ||
			gp->gt_fni == CH_BIT || gp->gt_fni == OUTW ?
			gp->gt_new : gp->gt_val);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#ifdef LOAD
		if ((exec = (iC_CFunctp)*(gp->gt_rlist)) != 0) {
		    val = exec(gp);		/* compute arith expression */
		}
#else	/* LOAD */
		if (gp->gt_rlist) {
		    val = iC_exec(gp->gt_rfunctn, gp);	/* must pass both -/+ */
		}
#endif	/* LOAD */
		else {
		    val = op->gt_new;		/* pass value to master output */
		}

		if (gp->gt_fni == ARITH || gp->gt_fni == D_SH ||
		    gp->gt_fni == CH_BIT || gp->gt_fni == OUTW) {
		    if (val != gp->gt_new) {
			gp->gt_new = val;
			(*initAct[gp->gt_fni])(gp, iC_a_list);
		    }
		} else if (gp->gt_fni == F_SW) {
		    gp->gt_new = val;		/* store new value, even if unchanged */
		    if (gp->gt_next == 0) {	/* execute unconditionally unless linked */
			(*initAct[gp->gt_fni])(gp, iC_a_list);	/* link_c() */
		    }
		} else if (gp->gt_fni == F_CE) {
		    gp->gt_val = val ? -1 : 1;	/* convert val to logic value */
		    if (gp->gt_next == 0) {	/* execute unconditionally unless linked */
			(*initAct[gp->gt_fni])(gp, iC_a_list);	/* link_c() */
		    }
		} else if ((val = val ? -1 : 1) != gp->gt_val) {
		    gp->gt_val = val;		/* convert val to logic value */
		    (*initAct[gp->gt_fni])(gp, iC_o_list);		/* init action */
		}
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
		if (iC_debug & 0100) fprintf(iC_outFP, " %ld",
		    iC_gx->gt_fni == ARITH || iC_gx->gt_fni == D_SH || gp->gt_fni == F_SW ||
		    iC_gx->gt_fni == CH_BIT || iC_gx->gt_fni == OUTW ?
		    iC_gx->gt_new : (long)iC_gx->gt_val);
#else	/* INT_MAX == 32767 && defined (LONG16) */
		if (iC_debug & 0100) fprintf(iC_outFP, " %d",
		    iC_gx->gt_fni == ARITH || iC_gx->gt_fni == D_SH || gp->gt_fni == F_SW ||
		    iC_gx->gt_fni == CH_BIT || iC_gx->gt_fni == OUTW ?
		    iC_gx->gt_new : iC_gx->gt_val);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    }
	} else if (op->gt_fni == GATE || op->gt_fni == GATEX) {
	    /************************************************************
	     * Scan normal logical outputs
	     ***********************************************************/
	    while ((gp = *lp++) != 0) {		/* ignore direct outputs except F_SW F_CE */
		if (gp->gt_fni == F_SW || gp->gt_fni == F_CE) {
		    /************************************************************
		     * Special initial treatment for ftypes F_SW F_CE normal outputs
		     ***********************************************************/
		    iC_scan_cnt++;		/* count scan operations */
#if YYDEBUG && !defined(_WINDOWS)
		    iC_gx = gp;			/* save old gp in iC_gx */
		    if (iC_debug & 0100) {
			if (iC_dc++ >= 4) {
			    iC_dc = 1;
			    putc('\n', iC_outFP);
			}
			fprintf(iC_outFP, "\t%s %2d ==>", gp->gt_ids, gp->gt_val);
		    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    if (gp->gt_next == 0) {	/* execute unconditionally unless linked */
			(*initAct[gp->gt_fni])(gp, iC_o_list);	/* link_c() */
		    }
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 0100) fprintf(iC_outFP, " %d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		}
	    }
	    /************************************************************
	     * Scan inverted logical outputs
	     ***********************************************************/
	    while ((gp = *lp++) != 0) {		/* do inverted outputs */
		/************************************************************
		 * no need to optimize for GATEX in this initialization
		 * - always test for XOR
		 ***********************************************************/
		iC_scan_cnt++;			/* count scan operations */
#if YYDEBUG && !defined(_WINDOWS)
		iC_gx = gp;			/* save old gp in iC_gx */
		if (iC_debug & 0100) {
		    if (iC_dc++ >= 4) {
			iC_dc = 1;
			putc('\n', iC_outFP);
		    }
		    fprintf(iC_outFP, "\t%s %2d ==>", gp->gt_ids, gp->gt_val);
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		if (gp->gt_fni == F_SW || gp->gt_fni == F_CE) {
		    /************************************************************
		     * Special initial treatment for ftypes F_SW F_CE inverted outputs
		     ***********************************************************/
		    if (gp->gt_ini != 0) {	/* logic not including XOR */
			/************************************************************
			 * GATE logical types AND OR LATCH for inverted outputs
			 ***********************************************************/
			if (--gp->gt_val == 0) {	/* gate function */
			    --gp->gt_val;
			}
		    } else {
			/************************************************************
			 * XOR for inverted outputs
			 ***********************************************************/
			gp->gt_val ^= 0x80;		/* xor independent of change */
		    }
		    if (gp->gt_next == 0) {	/* execute unconditionally unless linked */
			(*initAct[gp->gt_fni])(gp, iC_o_list);	/* link_c() */
		    }
		} else {
		    /************************************************************
		     * Initial treatment for inverted outputs
		     ***********************************************************/
		    if (gp->gt_ini != 0) {		/* logic not including XOR */
			/************************************************************
			 * GATE logical types AND OR LATCH for inverted outputs
			 ***********************************************************/
			if (--gp->gt_val == 0) {	/* gate function */
			    --gp->gt_val;
			    (*initAct[gp->gt_fni])(gp, iC_o_list);	/* init action */
			}
		    } else {
			/************************************************************
			 * XOR for inverted outputs
			 ***********************************************************/
			gp->gt_val ^= 0x80;		/* xor independent of change */
			(*initAct[gp->gt_fni])(gp, iC_o_list);	/* init action every time */
		    }
		}
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) fprintf(iC_outFP, " %d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    }
#if YYDEBUG && !defined(_WINDOWS)
	} else {
	    if (iC_debug & 0100) fprintf(iC_outFP, " ftype UDFA ??? %d", op->gt_fni);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	}
    }
} /* pass4 */

/********************************************************************
 *
 *	During pass 4 all logic Gates with clocked actions are put
 *	directly on the c_list, to simulate that all timers and
 *	clocks have completed before initialisation.
 *
 *	These Gates, which have an inverted input, will all appear
 *	in the first clock scan after initialisation.
 *
 *******************************************************************/

static void
link_c(Gate * gp, Gate * out_list)
{
    iC_link_ol(gp, iC_c_list);			/* link clocked Gate to c_list */
} /* link_c */

/********************************************************************
 *
 *	Error function for indirect execution when action is illegal.
 *	Could only happen due to compiler errors.
 *
 *******************************************************************/

void
iC_err_fn(					/* error - no master or slave function */
    Gate *	gp,
    Gate *	out_list)
{
#if !defined(_WINDOWS) || defined(LOAD)
    fprintf(iC_errFP,
    "\n%s: line %d: Gate %s, action %d not implemented",
    __FILE__, __LINE__, gp->gt_ids, gp->gt_fni);
    iC_quit(-1);
#endif	/* !defined(_WINDOWS) || defined(LOAD) */
} /* err_fn */

/********************************************************************
 *
 *	Null function for indirect execution when no action is
 *	necessary.
 *
 *******************************************************************/

void iC_null(void)	{}			/* null function for function lists */

/********************************************************************
 *
 *	Arithmetic Master action, when fired from an arithmetic or
 *	logic expression
 *
 *	NOTE: This action does not act on a clocked slave object.
 *
 *******************************************************************/

void
iC_arithMa(					/* ARITH master action */
    Gate *	gp,				/* NOTE: there is no slave action */
    Gate *	out_list)
{
    if (out_list == iC_o_list) {
	/* called from logic scan - convert d to a */
#if INT_MAX == 32767 && defined (LONG16)
	gp->gt_new = (long)((unsigned char)gp->gt_val >> 7);
#else	/* INT_MAX == 32767 && defined (LONG16) */
	gp->gt_new = ((unsigned char)gp->gt_val >> 7);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	/********************************************************************
	 * since new is only modified here and since gt_val has changed,
	 * new must differ from old (no need to check).
	 * old will be modified during arithmetic scan, although that
	 * would not be necessary for this action.
	 *******************************************************************/
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
	if (iC_debug & 0100) fprintf(iC_outFP, "%ld", gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
	if (iC_debug & 0100) fprintf(iC_outFP, "%d", gp->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    }
    if ((gp->gt_new != gp->gt_old) ^ (gp->gt_next != 0)) {
	iC_link_ol(gp, iC_a_list);		/* link to arithmetic list */
    }
} /* arithMa */

static const char scan_c[] =
"@(#)$Id: scan.c 1.47 $";
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
 *	scan.c
 *	runtime execution - scan action lists
 *	- do gate function, master and slave actions
 *
 *******************************************************************/

/* J.E. Wulff	3-Mar-85 */

#include	<stdio.h>
#include	<string.h>
#include	<signal.h>
#include	"icc.h"

/********************************************************************
 *
 *	The following 5 arrays are indexed by gt_fni (is ftype)
 *
 *	UDFA	ARITH	GATE	GATEX	RI_BIT	S_SH	R_SH	D_SH	CH_BIT
 *	S_FF	R_FF	D_FF	CH_AR	F_SW	F_CF	F_CE	CLCK	TIMR
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
 *	The nodes op on the iC_aList are all of ftype ARITH and the target
 *	nodes gp are all of type ARN + which have a cexe function.
 *
 *	The target nodes gp can trigger both arithmetic and logical actions.
 *
 *	The actions ARITH, D_SH, F_SW and CH_AR require arithmetic processing.
 *	CH_AR has a logical value which is triggered for every change in
 *	numerical value. The change is stored in CH_AR when slave is clocked.
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
	fprintf(iC_outFP, "\n== arith scan ==========");
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    while ((op = out_list->gt_next) != out_list) {	/* scan outputs */
#ifndef DEQ
	out_list->gt_next = op->gt_next;		/* unlink from */
	op->gt_next = 0;				/* output list */
	if (op == out_list->gt_ptr) {			/* last entry ? */
	    out_list->gt_ptr = out_list;		/* yes - fix pointer */
	}
#else	/* DEQ */
	out_list->gt_next = gp = op->gt_next;		/* list ==> next */
	gp->gt_prev = out_list;				/* list <== next */
	op->gt_next = op->gt_prev = 0;			/* unlink Gate */
#endif	/* DEQ */
	/********************************************************************
	 * up to this point changes in gt_new back to gt_old could have
	 * caused this Gate to be unlinked (glitch), resulting in the
	 * following update executions of arithmetic functions not
	 * being done. They are then no longer necessary, since no
	 * change in the function results would occur.
	 *******************************************************************/
	op->gt_old = op->gt_new;			/* now new value is fixed */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
#if INT_MAX == 32767 && defined (LONG16)
	    fprintf(iC_outFP, "\n%s:	%ld", op->gt_ids, op->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
	    fprintf(iC_outFP, "\n%s:	%d", op->gt_ids, op->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	    iC_dc = 0;
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	lp = op->gt_list;
	while ((gp = *lp++) != 0) {
#ifdef LOAD
	    iC_CFunctp exec;
#endif	/* LOAD */
	    iC_scan_cnt++;				/* count scan operations */
	    iC_gx = gp;					/* save old gp in iC_gx */
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) {
		if (iC_dc++ >= 4) {
		    iC_dc = 1;
		    fprintf(iC_outFP, "\n\t");
		}
		if (gp->gt_fni == ARITH  ||
		    gp->gt_fni == D_SH   ||
		    gp->gt_fni == F_SW   ||
		    gp->gt_fni == CH_AR ||
		    gp->gt_fni == OUTW) {
#if INT_MAX == 32767 && defined (LONG16)
		    fprintf(iC_outFP, "\t%s %ld ==>", gp->gt_ids, gp->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
		    fprintf(iC_outFP, "\t%s %d ==>", gp->gt_ids, gp->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		} else {
		    fprintf(iC_outFP, "\t%s %+d ==>", gp->gt_ids, gp->gt_val);
		}
	    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#ifdef LOAD
	    if (gp->gt_fni != OUTW && (exec = (iC_CFunctp)*(gp->gt_rlist)) != 0) {
		val = exec(gp);				/* compute arith expression */
	    }
#else	/* LOAD */
	    if (gp->gt_fni != OUTW && gp->gt_rlist) {
		val = iC_exec(gp->gt_rfunctn, gp);	/* must pass both -/+ */
	    }
#endif	/* LOAD */
	    else {
		val = op->gt_new;			/* pass value to master output */
	    }
	    /************************************************************
	     *
	     *	For any C_expressions, the computed value is returned.
	     *
	     ***********************************************************/

	    if (gp->gt_fni == ARITH || gp->gt_fni == D_SH || gp->gt_fni == CH_AR ||
		gp->gt_fni == F_SW || gp->gt_fni == OUTW) {
		if (val != gp->gt_new) {
		    gp->gt_new = val;
		    (*masterAct[gp->gt_fni])(gp, iC_aList); /* arithmetic master action */
		}
	    } else if ((val = val ? -1 : 1) != gp->gt_val) {
		gp->gt_val = val;			/* convert val to logic value */
		(*masterAct[gp->gt_fni])(gp, iC_oList);/* logic master action */
	    }
	    /* global iC_gx is modified in arithmetic chMbit() master action */
	    if (iC_gx->gt_fni == ARITH  ||
		iC_gx->gt_fni == D_SH   ||
		iC_gx->gt_fni == CH_AR ||
		iC_gx->gt_fni == F_SW   ||
		iC_gx->gt_fni == OUTW) {
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) {
#if INT_MAX == 32767 && defined (LONG16)
		    fprintf(iC_outFP, " %ld", iC_gx->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
		    fprintf(iC_outFP, " %d", iC_gx->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
		iC_liveData(iC_gx, iC_gx->gt_new);	/* VCD and/or iClive */
#endif
	    } else {
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) {
		    fprintf(iC_outFP, " %+d", iC_gx->gt_val);
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
		iC_liveData(iC_gx, iC_gx->gt_val < 0 ? 1 : 0); /* VCD and/or iClive */
#endif
	    }
	}
    }
} /* iC_scan_ar */

/********************************************************************
 *
 *	Scan of nodes on a logic action list
 *
 *	The nodes op on the iC_oList are all of ftype GATE and the target
 *	nodes gp are all of types AND, OR, LATCH or XOR.
 *
 *	The target nodes gp can trigger both arithmetic and logical actions.
 *
 *	The target actions for ARITH and D_SH cause logic to arithmetic
 *	conversion in the masterAct functions if called from iC_oList
 *	This is faster for the bulk of the plain logic actions which
 *	would be slowed down by an extra test in the scan.
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
	fprintf(iC_outFP, "\n== logic scan ==========");
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    while ((op = out_list->gt_next) != out_list) {	/* scan outputs */
#ifndef DEQ
	out_list->gt_next = op->gt_next;		/* unlink from */
	op->gt_next = 0;				/* output list */
	if (op == out_list->gt_ptr) {			/* last entry ? */
	    out_list->gt_ptr = out_list;		/* yes - fix pointer */
	}
#else	/* DEQ */
	out_list->gt_next = gp = op->gt_next;		/* list ==> next */
	gp->gt_prev = out_list;				/* list <== next */
	op->gt_next = op->gt_prev = 0;			/* unlink Gate */
#endif	/* DEQ */
	/************************************************************
	 * In the following code all proccessing for normal and
	 * inverted targets are coded twice (identically) to speed up
	 * exeution a little.
	 *
	 * GATE logical types AND OR LATCH (LOGC never occurs)
	 * op->gt_val is either -1 or +1 when a gate fires and it is
	 * linked to the tail of the action list.
	 * By the time the gate has reached the head of the action
	 * list, it may have been modified either + or -.
	 ***********************************************************/
	val = (op->gt_val < 0) ? -1 : 1;		/* normalise logic value */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\n%s:	%+d", op->gt_ids, val);
	    iC_dc = 0;
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	lp = op->gt_list;				/* point to 1st target list */
#ifdef LOAD
	if (op->gt_fni == GATE) {
	    /********************************************************************
	     * ftype == GATE - no targets will be XOR
	     *******************************************************************/
	    while ((gp = *lp++) != 0) {			/* scan non-inverted targets */
		iC_scan_cnt++;				/* count scan operations */
		iC_gx = gp;				/* save old gp in iC_gx */
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) {
		    if (iC_dc++ >= 4) {
			iC_dc = 1;
			fprintf(iC_outFP, "\n\t");
		    }
		    fprintf(iC_outFP, "\t%s %+d +=>", gp->gt_ids, gp->gt_val);
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		/************************************************************
		 * GATE logical types AND OR LATCH for normal targets
		 ***********************************************************/
		if ((gp->gt_val += val) == 0) {		/* normal logic */
		    gp->gt_val = val;			/* step past 0 */
		    (*masterAct[gp->gt_fni])(gp, iC_oList);/* master action */
		}
#if YYDEBUG && !defined(_WINDOWS)
		/* global iC_gx is modified in riMbit() and chMbit() master action */
		if (iC_debug & 0100) fprintf(iC_outFP, " %+d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
		iC_liveData(iC_gx, iC_gx->gt_val < 0 ? 1 : 0); /* VCD and/or iClive */
#endif
	    }						/* point to 2nd target list */
	    /************************************************************
	     * GATE logical types AND OR LATCH for inverted targets
	     ***********************************************************/
	    while ((gp = *lp++) != 0) {			/* scan inverted targets */
		iC_scan_cnt++;				/* count scan operations */
		iC_gx = gp;				/* save old gp in iC_gx */
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) {
		    if (iC_dc++ >= 4) {
			iC_dc = 1;
			fprintf(iC_outFP, "\n\t");
		    }
		    fprintf(iC_outFP, "\t%s %+d -=>", gp->gt_ids, gp->gt_val);
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		/************************************************************
		 * GATE logical types AND OR LATCH for inverted targets
		 ***********************************************************/
		if ((gp->gt_val -= val) == 0) {		/* inverted logic */
		    gp->gt_val = -val;			/* step past 0 */
		    (*masterAct[gp->gt_fni])(gp, iC_oList);/* master action */
		}
#if YYDEBUG && !defined(_WINDOWS)
		/* global iC_gx is modified in riMbit() and chMbit() master action */
		if (iC_debug & 0100) fprintf(iC_outFP, " %+d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
		iC_liveData(iC_gx, iC_gx->gt_val < 0 ? 1 : 0); /* VCD and/or iClive */
#endif
	    }
	} else {
#endif	/* LOAD */
	    /********************************************************************
	     * ftype == GATEX - some targets may be XOR - set at load time
	     * or direct execution, in which case ftype GATE handles XOR AND ...
	     *******************************************************************/
	    while ((gp = *lp++) != 0) {			/* scan non-inverted targets */
		iC_scan_cnt++;				/* count scan operations */
		iC_gx = gp;				/* save old gp in iC_gx */
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) {
		    if (iC_dc++ >= 4) {
			iC_dc = 1;
			fprintf(iC_outFP, "\n\t");
		    }
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		if (gp->gt_ini != 0) {			/* trigger for XOR (UDF) */
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 0100) {
			fprintf(iC_outFP, "\t%s %+d +=>", gp->gt_ids, gp->gt_val);
		    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    /************************************************************
		     * GATE logical types AND OR LATCH for normal targets
		     ***********************************************************/
		    if ((gp->gt_val += val) == 0) {	/* normal logic */
			gp->gt_val = val;		/* step past 0 */
			(*masterAct[gp->gt_fni])(gp, iC_oList);/* master action */
		    }
		} else {
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 0100) {
			fprintf(iC_outFP, "\t%s %+d ^=>", gp->gt_ids, gp->gt_val);
		    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    /************************************************************
		     * XOR processing for normal targets
		     * xor gates are initialised to 0 for 2, 3 or more 0 inputs.
		     * Every time one of the inputs changes from a 0 to a 1 or
		     * from a 1 to a 0, the output is toggled by negating the target.
		     * This is independent of the value of the current input.
		     ***********************************************************/
		    gp->gt_val = -gp->gt_val;		/* xor independent of change */
		    (*masterAct[gp->gt_fni])(gp, iC_oList);/* master action */
		}
#if YYDEBUG && !defined(_WINDOWS)
		/* global iC_gx is modified in riMbit() and chMbit() master action */
		if (iC_debug & 0100) fprintf(iC_outFP, " %+d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
		iC_liveData(iC_gx, iC_gx->gt_val < 0 ? 1 : 0); /* VCD and/or iClive */
#endif
	    }						/* point to 2nd target list */
	    /************************************************************
	     * GATE logical types AND OR LATCH for inverted targets
	     ***********************************************************/
	    while ((gp = *lp++) != 0) {			/* scan inverted targets */
		iC_scan_cnt++;				/* count scan operations */
		iC_gx = gp;				/* save old gp in iC_gx */
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) {
		    if (iC_dc++ >= 4) {
			iC_dc = 1;
			fprintf(iC_outFP, "\n\t");
		    }
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		if (gp->gt_ini != 0) {			/* trigger for XOR (UDF) */
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 0100) {
			fprintf(iC_outFP, "\t%s %+d -=>", gp->gt_ids, gp->gt_val);
		    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    /************************************************************
		     * GATE logical types AND OR LATCH for inverted targets
		     ***********************************************************/
		    if ((gp->gt_val -= val) == 0) {	/* inverted logic */
			gp->gt_val = -val;		/* step past 0 */
			(*masterAct[gp->gt_fni])(gp, iC_oList);/* master action */
		    }
		} else {
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 0100) {
			fprintf(iC_outFP, "\t%s %+d ^=>", gp->gt_ids, gp->gt_val);
		    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    /************************************************************
		     * XOR processing for inverted targets
		     ***********************************************************/
		    gp->gt_val = -gp->gt_val;		/* xor independent of change */
		    (*masterAct[gp->gt_fni])(gp, iC_oList);/* master action */
		}
#if YYDEBUG && !defined(_WINDOWS)
		/* global iC_gx is modified in riMbit() and chMbit() master action */
		if (iC_debug & 0100) fprintf(iC_outFP, " %+d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
		iC_liveData(iC_gx, iC_gx->gt_val < 0 ? 1 : 0); /* VCD and/or iClive */
#endif
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
 *	The nodes gp on the iC_cList are all actions of ftype D_SH - F_CF
 *	or CLCK or TIMR.
 *
 *	The appropriate slaveAct function is executed in this scan.
 *	When the slave action is a CLCK or TIMR action, the action
 *	nodes on their lists will be linked to iC_cList extending the
 *	clock scan immediately.
 *
 *	The execution of F_SW, F_CF and F_CE (IF ELSE or SWITCH) are
 *	deferred until all other slave actions have been executed, by
 *	linking them to iC_fList, which is scanned when iC_cList is empty.
 *
 *	All other slave actions act on GATE or ARITH (by D_SH only)
 *	nodes which are linked to iC_oList or iC_aList in the slaveAct.
 *
 *	Finally iC_fList is scanned when all other actions are completed and
 *	all slave outputs have their final value for this clock scan.
 *	F_SW, F_CF and F_CE execute C fragments as slaveAct when they
 *	are clocked. (IF ELSE or SWITCH). No output is linked directly
 *	to any action lists. The C fragments may assign to immediate
 *	gates of type LOGC or ARNC which are linked to the iC_oList or
 *	iC_aList respectively in the assignment if they change.
 *	The execution of C fragments during the scan of iC_fList, is then
 *	the start of a new run of combinatorial changes if immediate
 *	variables are altered, apart from the side effects of the C code.
 *
 *******************************************************************/

void
iC_scan_clk(Gate *	out_list)			/* scan a clock list */
{
    Gate *	op;
#ifdef DEQ
    Gate *	gp;
#endif	/* DEQ */

#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
	fprintf(iC_outFP, out_list == iC_cList ? "\n== clock scan =========="
						: "\n== funct scan ==========");
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    while ((op = out_list->gt_next) != out_list) {	/* scan outputs */
#ifndef DEQ
	out_list->gt_next = op->gt_next;		/* unlink from */
	op->gt_next = 0;				/* output list */
	if (op == out_list->gt_ptr) {			/* last entry ? */
	    out_list->gt_ptr = out_list;		/* yes - fix pointer */
	}
#else	/* DEQ */
	out_list->gt_next = gp = op->gt_next;		/* list ==> next */
	gp->gt_prev = out_list;				/* list <== next */
	op->gt_next = op->gt_prev = 0;			/* unlink Gate */
#endif	/* DEQ */
	iC_scan_cnt++;					/* count scan operations */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\n%s:", op->gt_ids);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	/* only fScf() and fSsw() require out_list to switch to iC_fList */
	(*slaveAct[op->gt_fni])(op, out_list);		/* execute slave action */
    }
} /* iC_scan_clk */

/********************************************************************
 *
 *	Scan of nodes on a send action list
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
	fprintf(iC_outFP, "\n== send scan  ==========");
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    while ((op = out_list->gt_next) != out_list) {
#ifndef DEQ
	out_list->gt_next = op->gt_next;		/* unlink from */
	op->gt_next = 0;				/* output list */
	if (op == out_list->gt_ptr) {			/* last entry ? */
	    out_list->gt_ptr = out_list;		/* yes - fix pointer */
	}
#else	/* DEQ */
	out_list->gt_next = gp = op->gt_next;		/* list ==> next */
	gp->gt_prev = out_list;				/* list <== next */
	op->gt_next = op->gt_prev = 0;			/* unlink Gate */
#endif	/* DEQ */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\n%s:\t", op->gt_ids);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	iC_outMw(op, out_list);				/* Master action is always iC_outMw() */
	iC_scan_cnt++;					/* count scan operations */
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
iC_pass1(Gate * op, int typ)				/* Pass1 init on gates */
{
    /* op->gt_mcnt is compiled or generated with 0, */
    /* except for _f0_1, where it is and should be 1 */
#ifndef DEQ
    op->gt_next = 0;					/* clear link */
#else	/* DEQ */
    op->gt_next = op->gt_prev = 0;			/* unlink Gate */
#endif	/* DEQ */
} /* iC_pass1 */

/********************************************************************
 *
 *	Pass 2 initialisation for all nodes.
 *
 *	Call a routine which depends on the ftype of this node.
 *	For Gates this is gate2().
 *
 *******************************************************************/

void
iC_pass2(Gate * op, int typ)				/* Pass2 init on gates */
{
    (*init2[op->gt_fni])(op, typ);			/* call pass2 function init */
} /* iC_pass2 */

/********************************************************************
 *
 *	Pass 2 initialisation for Gates via gt_fni == GATE in pass2().
 *	or gt_fni == ARITH.
 *
 *	Count the connections in each Gate to which this Gate is
 *	connected. Limit the maximum to PPGATESIZE (127) for GATE
 *	AND OR or LATCH only. XOR and ARN can have any number of inputs.
 *	In Pass 3 every simple Gate will contain the count of how
 *	many Gates operate on that Gate. XOR does not need a correct value.
 *	In fact only AND needs a correct value <= 127 which is now guaranteed
 *	with changes in AND OR LATCH gate generation splitting large gates
 *	as a final operation before reporting NET TOPOLOGY. (jw 2012.09.19)
 *
 *******************************************************************/

void
iC_gate2(Gate * op, int typ)				/* pass2 function init gates */
{
    int	cnt;
    Gate **	lp;
    Gate *	gp;

    if ((lp = op->gt_list) != 0) {
	cnt = (op->gt_fni < MAX_AR) ? 1 : 2;
	do {						/* for normal and inverted */
	    while ((gp = *lp++) != 0) {			/* init connected gates */
		if (gp->gt_mcnt < PPGATESIZE) {
		    gp->gt_mcnt++;
		} else
		if (gp->gt_ini != -ARN && gp->gt_ini != -XOR) {
			fprintf(iC_outFP, "\nError:\ttoo many inputs on gate: %s", gp->gt_ids);
			iC_error_flag |= 2;		/* cannot execute with this error */
		}					/* error message only once */
	    }
	} while (--cnt);
    } else {
	fprintf(iC_outFP,
	    "\nWarning:	gate %s has no output list", op->gt_ids);
	    iC_error_flag |= 1;				/* can execute with this warning */
    }
} /* iC_gate2 */

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

static void
gate3(Gate * gp, int typ)				/* Pass3 init on gates */
{
    unsigned char opt = iC_os[typ];
    if (gp->gt_mcnt == 0 && typ != LOGC && typ != ARNC && strcmp(gp->gt_ids, "LO") != 0) {
	fprintf(iC_outFP,
	    "\nWarning:    %c	%s\thas no input connections",
	    opt, gp->gt_ids);
	    iC_error_flag |= 1;				/* can execute with this warning */
    } else {
#if YYDEBUG && !defined(_WINDOWS)
	if ((iC_debug & (DQ|0100)) == 0100) {
	    fprintf(iC_outFP, "\n	    %c	%s:\t%d inputs",
		opt, gp->gt_ids, typ == ARNC || typ == LOGC ? gp->gt_mark : gp->gt_mcnt);
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	switch (typ) {
	case ARNC:
	    if (gp->gt_fni == UDFA) {
		gp->gt_old = gp->gt_mark;		/* immC array size */
	    } else {
		gp->gt_old = gp->gt_new;		/* immC int may be initialised to non-zero */
	    }
	    gp->gt_val = 1;
	    break;
	case LOGC:
	    if ((gp->gt_new & ~0x1) != 0) {
		fprintf(iC_outFP,
		    "\nWarning:    %d	%s\timmC bit initialiser should be 1 or 0 (default)",
		    gp->gt_new, gp->gt_ids);
		    iC_error_flag |= 1;			/* can execute with this warning */
	    }
	    gp->gt_val = gp->gt_new ? -1 : 1;		/* initialise immC bit to 1 or 0 */
	    gp->gt_new = 0;
	    gp->gt_old = gp->gt_mark;			/* immC array size if array else 0 */
	    break;
	case ARN:
	    gp->gt_new = gp->gt_old = 0;
	    gp->gt_val = 1;
	    break;
	case XOR:
	    gp->gt_val = 1;				/* XOR gates set to +1 */
	    gp->gt_ini = 0;				/* gt_ini 0 marks XOR */
	    break;
	case UDF:					/* number of inputs is 0 !! */
	case AND:
	    gp->gt_ini = gp->gt_val = gp->gt_mcnt;	/* AND set to number of inputs */
	    break;
	case OR:
	    gp->gt_ini = gp->gt_val = 1;		/* OR gates set to +1 */
	    break;
	case LATCH:
	    gp->gt_ini = gp->gt_val = (int)(gp->gt_mcnt + 1) >> 1;/* set LATCH gates */
	    break;
	default:
	    gp->gt_val = 0;				/* should not happen */
	    break;
	}
	gp->gt_mcnt = 0;				/* clear gt_mcnt */
	gp->gt_live = 0;				/* clear live flag */
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
 *	clocked and timer actions are linked to iC_cList immediately,
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
iC_pass4(Gate * op, int typ)				/* Pass4 init on gates */
{
    Gate **	lp;
    Gate *	gp;
#if INT_MAX == 32767 && defined (LONG16)
    long	val;
#else	/* INT_MAX == 32767 && defined (LONG16) */
    int		val;
#endif	/* INT_MAX == 32767 && defined (LONG16) */

    if (op->gt_fni < MIN_ACT) {				/* UDFA, ARITH, GATE */
	lp = op->gt_list;
	if (op->gt_fni == UDFA && (op->gt_ini == -ARNC || op->gt_ini == -LOGC)) {
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) {
		fprintf(iC_outFP, "\n%s:	[%d]", op->gt_ids, (int)op->gt_old);	/* immC array */
	    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	} else
	if (op->gt_fni == ARITH) {
#if YYDEBUG && !defined(_WINDOWS)
	    if ((iC_debug & 0100) && (!(iC_debug & DQ) || *lp)) {
#if INT_MAX == 32767 && defined (LONG16)
		fprintf(iC_outFP, "\n%s:	%ld", op->gt_ids, op->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
		fprintf(iC_outFP, "\n%s:	%d", op->gt_ids, op->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		iC_dc = (iC_debug & DQ) ? 0 : 1;
	    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    /************************************************************
	     * Scan arithmetic outputs
	     ***********************************************************/
	    while ((gp = *lp++) != 0) {			/* do arithmetic outputs */
#ifdef LOAD
		iC_CFunctp exec;
#endif	/* LOAD */
		iC_scan_cnt++;				/* count scan operations */
		iC_gx = gp;				/* save old gp in iC_gx */
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) {
		    if (iC_dc++ >= 4) {
			iC_dc = 1;
			fprintf(iC_outFP, "\n\t");
		    }
		    if (gp->gt_fni == ARITH  ||
			gp->gt_fni == D_SH   ||
			gp->gt_fni == F_SW   ||
			gp->gt_fni == CH_BIT ||
			gp->gt_fni == OUTW) {
    #if INT_MAX == 32767 && defined (LONG16)
			fprintf(iC_outFP, "\t%s %ld ==>", gp->gt_ids, gp->gt_new);
    #else	/* INT_MAX == 32767 && defined (LONG16) */
			fprintf(iC_outFP, "\t%s %d ==>", gp->gt_ids, gp->gt_new);
    #endif	/* INT_MAX == 32767 && defined (LONG16) */
		    } else {
			fprintf(iC_outFP, "\t%s %+d ==>", gp->gt_ids, gp->gt_val);
		    }
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#ifdef LOAD
		if (gp->gt_fni != OUTW && (exec = (iC_CFunctp)*(gp->gt_rlist)) != 0) {
		    val = exec(gp);			/* compute arith expression */
		}
#else	/* LOAD */
		if (gp->gt_fni != OUTW && gp->gt_rlist) {
		    val = iC_exec(gp->gt_rfunctn, gp);	/* must pass both -/+ */
		}
#endif	/* LOAD */
		else {
		    val = op->gt_new;			/* pass value to master output */
		}

		if (gp->gt_fni == ARITH || gp->gt_fni == D_SH ||
		    gp->gt_fni == CH_BIT || gp->gt_fni == OUTW) {
		    if (val != gp->gt_new) {
			gp->gt_new = val;
			(*initAct[gp->gt_fni])(gp, iC_aList);
		    }
		} else if (gp->gt_fni == F_SW) {
		    gp->gt_new = val;			/* store new value, even if unchanged */
		    if (gp->gt_next == 0) {		/* execute unconditionally unless linked */
			(*initAct[gp->gt_fni])(gp, iC_aList);	/* link_cl() */
		    }
		} else if (gp->gt_fni == F_CE) {
		    gp->gt_val = val ? -1 : 1;		/* convert val to logic value */
		    if (gp->gt_next == 0) {		/* execute unconditionally unless linked */
			(*initAct[gp->gt_fni])(gp, iC_aList);	/* link_cl() */
		    }
		} else if ((val = val ? -1 : 1) != gp->gt_val) {
		    gp->gt_val = val;			/* convert val to logic value */
		    (*initAct[gp->gt_fni])(gp, iC_oList);/* init action */
		}
		if (iC_gx->gt_fni == ARITH  ||
		    iC_gx->gt_fni == D_SH   ||
		    iC_gx->gt_fni == F_SW   ||
		    iC_gx->gt_fni == CH_BIT ||
		    iC_gx->gt_fni == OUTW) {
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 0100) {
#if INT_MAX == 32767 && defined (LONG16)
			fprintf(iC_outFP, " %ld", iC_gx->gt_new);
#else	/* INT_MAX == 32767 && defined (LONG16) */
			fprintf(iC_outFP, " %d", iC_gx->gt_new);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
		    iC_liveData(iC_gx, iC_gx->gt_new);	/* VCD and/or iClive */
#endif
		} else {
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 0100) {
			fprintf(iC_outFP, " %+d", iC_gx->gt_val);
		    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
		    iC_liveData(iC_gx, iC_gx->gt_val < 0 ? 1 : 0); /* VCD and/or iClive */
#endif
		}
	    }
	} else if (op->gt_fni == GATE || op->gt_fni == GATEX) {
#if YYDEBUG && !defined(_WINDOWS)
	    if ((iC_debug & (DQ|0100)) == 0100) {
		fprintf(iC_outFP, "\n%s:	%+d", op->gt_ids, op->gt_val);
	    }
	    iC_dc = (iC_debug & DQ) ? 0 : 1;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    /************************************************************
	     * Scan normal logical outputs
	     ***********************************************************/
	    while ((gp = *lp++) != 0) {			/* ignore direct outputs except F_SW F_CE and pre-initialised immC bits */
		/************************************************************
		 * no need to optimize for GATEX in this initialization
		 * - always test for XOR
		 ***********************************************************/
		if (gp->gt_fni == F_SW || gp->gt_fni == F_CE) {
		    iC_scan_cnt++;			/* count scan operations */
		    iC_gx = gp;				/* save old gp in iC_gx */
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 0100) {
			if (iC_dc == 0) {
			    fprintf(iC_outFP, "\n%s:	%+d", op->gt_ids, op->gt_val);
			}
			if (iC_dc++ >= 4) {
			    iC_dc = 1;
			    fprintf(iC_outFP, "\n\t");
			}
		    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    /************************************************************
		     * Special initial treatment for ftypes F_SW F_CE normal 1 outputs
		     ***********************************************************/
		    if (op->gt_ini == -LOGC && op->gt_val < 0) {	/* only initialised immC bit */
			if (gp->gt_ini != 0) {		/* logic not including XOR */
#if YYDEBUG && !defined(_WINDOWS)
			    if (iC_debug & 0100) {
				fprintf(iC_outFP, "\t%s %+d +=>", gp->gt_ids, gp->gt_val);
			    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			    /************************************************************
			     * GATE logical types AND OR LATCH for normal 1 outputs
			     ***********************************************************/
			    if (--gp->gt_val == 0) {	/* gate function */
				--gp->gt_val;
			    }
			} else {
#if YYDEBUG && !defined(_WINDOWS)
			    if (iC_debug & 0100) {
				fprintf(iC_outFP, "\t%s %+d ^=>", gp->gt_ids, gp->gt_val);
			    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			    /************************************************************
			     * XOR for normal 1 outputs
			     ***********************************************************/
			    gp->gt_val = -gp->gt_val;	/* xor independent of change */
			}
		    }
#if YYDEBUG && !defined(_WINDOWS)
		    else if (iC_debug & 0100) {
			fprintf(iC_outFP, "\t%s %+d ==>", gp->gt_ids, gp->gt_val);
		    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    if (gp->gt_next == 0) {		/* execute unconditionally unless linked */
			(*initAct[gp->gt_fni])(gp, iC_oList);	/* link_cl() */
		    }
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 0100) fprintf(iC_outFP, " %+d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
		    iC_liveData(iC_gx, iC_gx->gt_val < 0 ? 1 : 0); /* VCD and/or iClive */
#endif
		} else if (op->gt_ini == -LOGC && op->gt_val < 0) {	/* only initialised immC bit */
		    /************************************************************
		     * Initial treatment for normal 1 outputs
		     ***********************************************************/
		    if (gp->gt_ini != 0) {		/* logic not including XOR */
			iC_scan_cnt++;			/* count scan operations */
			iC_gx = gp;			/* save old gp in iC_gx */
#if YYDEBUG && !defined(_WINDOWS)
			if (iC_debug & 0100) {
			    if (iC_dc == 0) {
				fprintf(iC_outFP, "\n%s:	%+d", op->gt_ids, op->gt_val);
			    }
			    if (iC_dc++ >= 4) {
				iC_dc = 1;
				fprintf(iC_outFP, "\n\t");
			    }
			    fprintf(iC_outFP, "\t%s %+d -=>", gp->gt_ids, gp->gt_val);
			}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			/************************************************************
			 * GATE logical types AND OR LATCH for normal 1 outputs
			 ***********************************************************/
			if (--gp->gt_val == 0) {	/* gate function */
			    --gp->gt_val;
			    (*initAct[gp->gt_fni])(gp, iC_oList);	/* init action */
			}
		    } else {
#if YYDEBUG && !defined(_WINDOWS)
			if (iC_debug & 0100) {
			    fprintf(iC_outFP, "\t%s %+d ^=>", gp->gt_ids, gp->gt_val);
			}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			/************************************************************
			 * XOR for normal 1 outputs
			 ***********************************************************/
			gp->gt_val = -gp->gt_val;	/* xor independent of change */
			(*initAct[gp->gt_fni])(gp, iC_oList);	/* init action every time */
		    }
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_debug & 0100) fprintf(iC_outFP, " %+d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
		    iC_liveData(iC_gx, iC_gx->gt_val < 0 ? 1 : 0); /* VCD and/or iClive */
#endif
		}
	    }
	    /************************************************************
	     * Scan inverted logical outputs
	     ***********************************************************/
	    while ((gp = *lp++) != 0) {			/* do inverted outputs */
		/************************************************************
		 * no need to optimize for GATEX in this initialization
		 * - always test for XOR
		 ***********************************************************/
		iC_scan_cnt++;				/* count scan operations */
		iC_gx = gp;				/* save old gp in iC_gx */
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) {
		    if (iC_dc == 0) {
			fprintf(iC_outFP, "\n%s:	%+d", op->gt_ids, op->gt_val);
		    }
		    if (iC_dc++ >= 4) {
			iC_dc = 1;
			fprintf(iC_outFP, "\n\t");
		    }
		}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		if (gp->gt_fni == F_SW || gp->gt_fni == F_CE) {
		    if (op->gt_ini != -LOGC || op->gt_val > 0) {	/* except initialised immC bit */
			/************************************************************
			 * Special initial treatment for ftypes F_SW F_CE inverted 0 outputs
			 ***********************************************************/
			if (gp->gt_ini != 0) {		/* logic not including XOR */
#if YYDEBUG && !defined(_WINDOWS)
			    if (iC_debug & 0100) {
				fprintf(iC_outFP, "\t%s %+d -=>", gp->gt_ids, gp->gt_val);
			    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			    /************************************************************
			     * GATE logical types AND OR LATCH for inverted 0 outputs
			     ***********************************************************/
			    if (--gp->gt_val == 0) {	/* gate function */
				--gp->gt_val;
			    }
			} else {
#if YYDEBUG && !defined(_WINDOWS)
			    if (iC_debug & 0100) {
				fprintf(iC_outFP, "\t%s %+d ^=>", gp->gt_ids, gp->gt_val);
			    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			    /************************************************************
			     * XOR for inverted 0 outputs
			     ***********************************************************/
			    gp->gt_val = -gp->gt_val;	/* xor independent of change */
			}
		    }
		    if (gp->gt_next == 0) {		/* execute unconditionally unless linked */
			(*initAct[gp->gt_fni])(gp, iC_oList);	/* link_cl() */
		    }
		} else if (op->gt_ini != -LOGC || op->gt_val > 0) {	/* except initialised immC bit */
		    /************************************************************
		     * Initial treatment for inverted 0 outputs (exclude inverted 1 immC bits)
		     ***********************************************************/
		    if (gp->gt_ini != 0) {		/* logic not including XOR */
#if YYDEBUG && !defined(_WINDOWS)
			if (iC_debug & 0100) {
			    fprintf(iC_outFP, "\t%s %+d -=>", gp->gt_ids, gp->gt_val);
			}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			/************************************************************
			 * GATE logical types AND OR LATCH for inverted 0 outputs
			 ***********************************************************/
			if (--gp->gt_val == 0) {	/* gate function */
			    --gp->gt_val;
			    (*initAct[gp->gt_fni])(gp, iC_oList);	/* init action */
			}
		    } else {
#if YYDEBUG && !defined(_WINDOWS)
			if (iC_debug & 0100) {
			    fprintf(iC_outFP, "\t%s %+d ^=>", gp->gt_ids, gp->gt_val);
			}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			/************************************************************
			 * XOR for inverted 0 outputs
			 ***********************************************************/
			gp->gt_val = -gp->gt_val;	/* xor independent of change */
			(*initAct[gp->gt_fni])(gp, iC_oList);	/* init action every time */
		    }
		}
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) fprintf(iC_outFP, " %+d", iC_gx->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
		iC_liveData(iC_gx, iC_gx->gt_val < 0 ? 1 : 0); /* VCD and/or iClive */
#endif
	    }
#if YYDEBUG && !defined(_WINDOWS)
	} else {
	    if (iC_debug & 0100) fprintf(iC_outFP, "\n%s:	ftype UDFA ??? %d", op->gt_ids, op->gt_fni);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	}
    }
} /* iC_pass4 */

/********************************************************************
 *
 *	During pass 4 all logic Gates with clocked actions are put
 *	directly on the iC_cList, to simulate that all timers and
 *	clocks have completed before initialisation.
 *
 *	These Gates, which have an inverted input, will all appear
 *	in the first clock scan after initialisation.
 *
 *******************************************************************/

void
iC_link_cl(Gate * gp, Gate * out_list)
{
    iC_link_ol(gp, iC_cList);				/* link clocked Gate to iC_cList */
} /* iC_link_cl */

/********************************************************************
 *
 *	Error function for indirect execution when action is illegal.
 *	Could only happen due to compiler errors.
 *
 *******************************************************************/

void
iC_err_fn(						/* error - no master or slave function */
    Gate *	gp,
    Gate *	out_list)
{
#if !defined(_WINDOWS) || defined(LOAD)
    fprintf(iC_errFP,
    "\n%s: line %d: Gate %s, action %d not implemented",
    __FILE__, __LINE__, gp->gt_ids, gp->gt_fni);
    iC_quit(SIGUSR1);
#endif	/* !defined(_WINDOWS) || defined(LOAD) */
} /* iC_err_fn */

/********************************************************************
 *
 *	Null function for indirect execution when no action is
 *	necessary.
 *
 *******************************************************************/

void iC_null(void)	{}				/* null function for function lists */

/********************************************************************
 *
 *	Arithmetic Master action, when fired from an arithmetic or
 *	logic expression
 *
 *	NOTE: This action does not act on a clocked slave object.
 *
 *******************************************************************/

void
iC_arithMa(						/* ARITH master action */
    Gate *	gp,					/* NOTE: there is no slave action */
    Gate *	out_list)
{
    if (out_list == iC_oList) {
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
	iC_link_ol(gp, iC_aList);			/* link to arithmetic list */
    }
} /* iC_arithMa */
#if defined(TCP) || defined(LOAD)

/********************************************************************
 *
 *	Logic Master action
 *
 *	NOTE: This action does not act on a clocked slave object.
 *
 *******************************************************************/

void
iC_gateMa(						/* GATE master action */
    Gate *	gp,					/* NOTE: there is no slave action */
    Gate *	out_list)
{
    iC_link_ol(gp, iC_oList);				/* link to logic list */
} /* iC_gateMa */
#endif	/* else define iCgateMa iC_link_ol endif defined(TCP) || defined(LOAD) */

static const char rsff_c[] =
"@(#)$Id: rsff.c 1.67 $";
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
 *	rsff.c
 *	SR flip flop and other clocked functions
 *
 *******************************************************************/

/* J.E. Wulff	8-Mar-85 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<signal.h>
#include	<assert.h>
#include	"icc.h"

#ifndef	_WIN32
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif	/* _WIN32 */

/********************************************************************
 *
 *	Clocked Flip Flop.
 *
 *	Can be set and reset by the S_FF and R_FF actions to produce
 *	an SR Flip Flop.
 *		f1 = SR(I1, I2, clock);	// clock has been defined
 *		f2 = SR(I3, I4);	// iClock is default
 *
 *	Can also be acted on by one D_FF action to produce a D Flip
 *	Flop.
 *		f3 = D(I5, clock);	// clock has been defined
 *		f4 = D(I6);		// iClock is default
 *
 *	All three actions can act on one Flip Flop, although at least
 *	the D_FF action or alternatively both S_FF and R_FF must be
 *	present. This is checked in i_ff2() and i_ff3(), which are the
 *	pass 2 and pass 3 routines for all clocked functions.
 *
 *	All clocked functions work by the Master/Slave principle. Each
 *	such function consist of a Master and a Slave. The Master
 *	is an action Gate which is fired during the arithmetic or
 *	logic scan whenever its logic value changes. Upon firing,
 *	the Master action Gate is linked to the clock list specified
 *	in the call, if the function requires it. When the clock fires,
 *	which is during the clock scan, the Master Gate executes the
 *	Slave action, which normally modifies the Slave Object.
 *
 *	Most actions are initiated by the LO to HI transition of the
 *	action Gate, qualified by the clock. Exceptions are the D_FF
 *	action for D Flip Flops and the CH_BIT action described below,
 *	which initiate actions on both the rising and the falling edge.
 *
 *	A consistent naming convention for Gate pointers has been used
 *	for all routines:
 *		gm	Gate Master	pointer to master gate (1st arg)
 *		gs	Gate Slave	pointer to associated slave
 *		gx	special slave	pointer globally shared with scan.c
 *		ma	Master Actions	pointer to Gate array, pointing to
 *					associated slave, clock and time
 *		out_list 2nd argument	used in D_SH, CH_BIT, F_SW and OUTW
 *					to select logic ar arithmetic input
 *		out_list 2nd argument	used in IF ELSE and SWITCH to select
 *					deferred action iC_fList
 *
 *******************************************************************/

/********************************************************************
 *
 *	Master set action S_FF on flip/flop FF
 *
 *******************************************************************/

void
iC_sMff(					/* S_FF master action on FF */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate **	ma;

    if (
	(
	    (ma = gm->gt_list)[FL_GATE]->gt_val > 0 &&
	    gm->gt_val < 0			/* anything to set */
	) ||
	gm->gt_next				/* or glitch */
    ) {
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
    } else {
#if defined(TCP) || defined(LOAD)
	iC_linked++;				/* show change even when no action */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_sMff */

/********************************************************************
 *
 *	Slave set action S_FF on flip/flop FF
 *
 *******************************************************************/

void
iC_sSff(					/* S_FF slave action on FF */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate *	gs;

    assert(gm->gt_val < 0);			/* S FF receives -1 ==> 1 ??? */
    gs = gm->gt_funct;
    if (gs->gt_val > 0) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\t-1\t%s %+d S=>", gs->gt_ids, gs->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	gs->gt_val = -1;			/* set slave output */
	iC_link_ol(gs, iC_oList);
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, " -1");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
	iC_liveData(gs, 1);			/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_sSff */

/********************************************************************
 *
 *	Master reset action R_FF on flip/flop FF
 *
 *******************************************************************/

void
iC_rMff(					/* R_FF master action on FF */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate **	ma;

    if (
	(
	    (ma = gm->gt_list)[FL_GATE]->gt_val < 0 &&
	    gm->gt_val < 0			/* anything to reset */
	) ||
	gm->gt_next				/* or glitch */
    ) {
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
    } else {
#if defined(TCP) || defined(LOAD)
	iC_linked++;				/* show change even when no action */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_rMff */

/********************************************************************
 *
 *	Slave reset action R_FF on flip/flop FF
 *
 *******************************************************************/

void
iC_rSff(					/* R_FF slave action on FF */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate *	gs;

    assert(gm->gt_val < 0);			/* R FF receives -1 ==> 1 ??? */
    gs = gm->gt_funct;
    if (gs->gt_val < 0) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\t-1\t%s %+d R=>", gs->gt_ids, gs->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	gs->gt_val = 1;				/* reset slave output */
	iC_link_ol(gs, iC_oList);
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, " +1");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
	iC_liveData(gs, 0);			/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_rSff */

/********************************************************************
 *
 *	Master delay action D_FF on flip/flop FF
 *
 *******************************************************************/

void
iC_dMff(					/* D_FF master action on FF */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate **	ma;

    if (
	((ma = gm->gt_list)[FL_GATE]->gt_val < 0)
	^ (gm->gt_val < 0)			/* any change */
	^ (gm->gt_next != 0)			/* xor glitch */
    ) {
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
    } else {
#if defined(TCP) || defined(LOAD)
	iC_linked++;				/* show change even when no action */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_dMff */

/********************************************************************
 *
 *	Slave delay action D_FF on flip/flop FF
 *
 *******************************************************************/

void
iC_dSff(					/* D_FF slave action on FF */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate *	gs;
    signed char	val;

    gs = gm->gt_funct;
    if ((val = (gm->gt_val < 0) ? -1 : 1) != gs->gt_val) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\t%+d\t%s %+d D=>", val, gs->gt_ids, gs->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	gs->gt_val = val;			/* transfer val to slave */
	iC_link_ol(gs, iC_oList);
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gs->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
	iC_liveData(gs, val < 0 ? 1 : 0);	/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_dSff */

/********************************************************************
 *
 *	Sample/hold function with SH(arithmetic_expr)
 *				  SHR(arithmetic_expr, reset)
 *				  SHSR(arithmetic_expr, set, reset)
 *
 *	Sample/hold is the arithmetic analog of a flip/flop
 *	The delay function transfers the arithmetic input at the
 *	master to the slave on the next clock.
 *	Set and reset set the slave to -1 (all 1s) or 0 (all 0s)
 *	on the next clock. Set and reset have priority over delay.
 *
 *	As of version 1.37 (icc_1.104) SH with reset or with set/reset
 *	were implemented as system defined functions in init.c.
 *	This change means that no locking between data, set and reset
 *	inputs is necessary at this level. This change was prompted by
 *	similar changes in the D flip/flop with reset or set/reset,
 *	which did nor really work correctly, because it had no locking
 *	code in this file.
 *
 *******************************************************************/

/********************************************************************
 *
 *	Master delay D_SH on sample/hold SH
 *
 *******************************************************************/

void
iC_dMsh(					/* D_SH master action on SH */
    Gate *	gm,
    Gate *	out_list)
{
    Gate *	gs;

    if (out_list == iC_oList) {
	/* called from logic scan - convert d to a */
	gm->gt_new = gm->gt_val < 0 ? 1 : 0;
	/********************************************************************
	 * since new is only modified here and since gt_val has changed,
	 * new must differ from old (no need to check).
	 * old will be modified during arithmetic scan, although that
	 * would not be necessary for this action.
	 *******************************************************************/
    }
    /********************************************************************
     * SH master action is only called if gt_new has changed first time
     * versus gt_old or changed back, which is a glitch. This is already
     * tested in scan_ar() unlike D master action which has to do it here.
     * Nevertheles the action must only be linked, if the new value will
     * change the master, since that can be manipulated by set or reset.
     *******************************************************************/
    gs = gm->gt_funct;
    if (
	(gm->gt_new != gs->gt_new)		/* any change */
	^ (gm->gt_next != 0)			/* xor glitch */
    ) {
	iC_link_ol(gm, gm->gt_clk);		/* master action */
    } else {
#if defined(TCP) || defined(LOAD)
	iC_linked++;				/* show change even when no action */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_dMsh */

/********************************************************************
 *
 *	Slave delay action D_SH on sample/hold SH
 *
 *******************************************************************/

void
iC_dSsh(					/* D_SH slave action on SH */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate *	gs;

    gs = gm->gt_funct;
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
	if (iC_debug & 0100) fprintf(iC_outFP, "\t%ld\t%s %ld SH=>", gm->gt_new, gs->gt_ids, gs->gt_new);
#else
	if (iC_debug & 0100) fprintf(iC_outFP, "\t%d\t%s %d SH=>", gm->gt_new, gs->gt_ids, gs->gt_new);
#endif
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    /********************************************************************
     * since gm is still on clock list gt_new must differ from gt_old
     *******************************************************************/
    gs->gt_new = gm->gt_old = gm->gt_new;	/* transfer value to slave */
    iC_link_ol(gs, iC_aList);			/* fire new arithmetic action */

#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
    if (iC_debug & 0100) fprintf(iC_outFP, " %ld", gs->gt_new);
#else
    if (iC_debug & 0100) fprintf(iC_outFP, " %d", gs->gt_new);
#endif
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
    iC_liveData(gs, gs->gt_new);		/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
} /* iC_dSsh */

/********************************************************************
 *
 *	Master set action S_SH on sample/hold SH
 *
 *******************************************************************/

void
iC_sMsh(					/* S_SH master action on SH */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate **	ma;

    if (
	(
	    (ma = gm->gt_list)[FL_GATE]->gt_new != -1 &&
	    gm->gt_val < 0			/* anything to set */
	) ||
	gm->gt_next				/* or glitch */
    ) {
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
    } else {
#if defined(TCP) || defined(LOAD)
	iC_linked++;				/* show change even when no action */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_sMsh */

/********************************************************************
 *
 *	Slave set action S_SH on sample/hold SH
 *
 *******************************************************************/

void
iC_sSsh(					/* S_SH slave action on SH */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate *	gs;

    assert(gm->gt_val < 0);			/* S SH receives -1 ==> 1 ??? */
    gs = gm->gt_funct;
    if (gs->gt_new != -1) {
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
	if (iC_debug & 0100) fprintf(iC_outFP, "\t-1\t%s %2ld S SH=>", gs->gt_ids, gs->gt_new);
#else
	if (iC_debug & 0100) fprintf(iC_outFP, "\t-1\t%s %2d S SH=>", gs->gt_ids, gs->gt_new);
#endif
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	gs->gt_new = -1;			/* set SH slave output */
	iC_link_ol(gs, iC_aList);
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, " -1");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
	iC_liveData(gs, -1);			/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_sSsh */

/********************************************************************
 *
 *	Master reset action R_SH on sample/hold SH
 *
 *******************************************************************/

void
iC_rMsh(					/* R_SH master action on SH */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate **	ma;

    if (
	(
	    (ma = gm->gt_list)[FL_GATE]->gt_new != 0 &&
	    gm->gt_val < 0			/* anything to reset */
	) ||
	gm->gt_next				/* or glitch */
    ) {
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
    } else {
#if defined(TCP) || defined(LOAD)
	iC_linked++;				/* show change even when no action */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_rMsh */

/********************************************************************
 *
 *	Slave reset action R_SH on sample/hold SH
 *
 *******************************************************************/

void
iC_rSsh(					/* R_SH slave action on SH */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate *	gs;

    assert(gm->gt_val < 0);			/* R SH receives -1 ==> 1 ??? */
    gs = gm->gt_funct;
    if (gs->gt_new != 0) {
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
	if (iC_debug & 0100) fprintf(iC_outFP, "\t-1\t%s %2ld R SH=>", gs->gt_ids, gs->gt_new);
#else
	if (iC_debug & 0100) fprintf(iC_outFP, "\t-1\t%s %2d R SH=>", gs->gt_ids, gs->gt_new);
#endif
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	gs->gt_new = 0;				/* reset SH slave output */
	iC_link_ol(gs, iC_aList);
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, " 0");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
	iC_liveData(gs, 0);			/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_rSsh */

/********************************************************************
 *
 *	Pass 2 initialisation for clocked Gates via ftype S_FF - TIMR
 *	except F_SW, F_CF and F_CE in pass2().
 *
 *	Each action Gate ors a bit reserved for its action in gt_mcnt
 *	of the function Gate on which it acts. If ONCE_M is set, that
 *	action may only occurr once.
 *
 *	In Pass 3 every function Gate will contain a bit for every
 *	action which acts on that function.
 *
 *******************************************************************/

void
iC_i_ff2(Gate * gm, int typ)			/* called via output lists */
{
    Gate *		gs;
    unsigned int	mask;			/* action bit mask */

    if ((gs = gm->gt_funct) != 0) {
	mask = iC_bit2[gm->gt_fni];
	/* with this test D actions must occurr before S and R actions */
	if ((mask & ONCE_M) && gs->gt_mcnt == (mask &= ~ONCE_M)) {
	    fprintf(iC_outFP,
	"\nError:    %c\t%s\thas %c input action more than once (%.4x:%.4x)",
		iC_os[iC_types[gm->gt_fni]], gs->gt_ids,
		iC_fos[gm->gt_fni], gs->gt_mcnt, mask);
	    iC_error_flag |= 2;			/* cannot execute with this error */
	}
	gs->gt_mcnt |= mask;			/* allow other multiple inputs */
    }
} /* i_ff2 */

/********************************************************************
 *
 *	Pass 3 initialisation for function Gates. Called directly for
 *	function types.
 *
 *	These routines check that only actions compatible with the
 *	slave node are present. (checks that compilers are correct).
 *
 *	This is done by comparing the action bits applied in pass 2
 *	with the bit mask required for that function. This detects if
 *	there are too many or too few actions for the function.
 *
 *******************************************************************/

static void
i_ff3(Gate * gm, int typ)			/* Pass3 init on FF etc. */
{
    unsigned int	mask;			/* action bit mask */
    char		opt;

    if (gm != iC_cList) {			/* iClock has no inputs */
	mask = iC_bit2[iC_ftypes[typ]] & ~ONCE_M;/* action bit mask required */
	opt = iC_os[typ];
	/* accept only the right number of compatible inputs */
	if (gm->gt_mcnt != mask) {
	    fprintf(iC_outFP,
	    "\nError:    %c	%s\thas %s inputs (%.4x:%.4x)",
		opt, gm->gt_ids, gm->gt_mcnt ? "incompatible" : "no",
		gm->gt_mcnt, mask);
	    iC_error_flag |= 2;			/* cannot execute with this error */
#if YYDEBUG && !defined(_WINDOWS)
	} else if ((iC_debug & (DQ|0100)) == 0100) {
	    fprintf(iC_outFP, "\n	    %c	%s:\t%.4x inputs",
		opt, gm->gt_ids, gm->gt_mcnt);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	}
    }
    if (typ < MAX_LV) {
	gm->gt_val = 1;				/* set fblk gates to +1 anyway */
	if (typ == SH || typ == INPW) {
	    gm->gt_new = gm->gt_old = 0;	/* clear arithmetic */
	} else if (typ == NCONST) {
	    char *	ep;
	    /* convert constant 18 from dec, 077 from oct 0x1c from hex */
	    gm->gt_new = gm->gt_old = strtol(gm->gt_ids, &ep, 0); /* long to int or long */
	    if (*ep != '\0') {
		fprintf(iC_outFP,
		"\nError:    constant '%s' cannot be converted to a number", gm->gt_ids);
		iC_error_flag |= 2;		/* cannot execute with this error */
	    }
	}
    } else {
	gm->gt_val = 0;				/* used in visualization */
	Out_init(gm);				/* link as clock or timer list */
	if (gm != iC_cList) {
	    gm->gt_rptr = iC_cList;		/* iC_cList is alternate for link */
	}					/* except iC_cList which has 0 */
    }
    if (gm->gt_fni != OUTW && gm->gt_fni != OUTX) {
	gm->gt_mark = 0;			/* clear for stamp and diff in actions */
    }
} /* i_ff3 */

iC_Functp	iC_ff_i[] = {iC_pass1, iC_pass2, i_ff3, iC_pass4};

/********************************************************************
 *
 *	Master function to detect RISE in logical input.
 *		O0 = RISE(IX0.1);		// iClock is default
 *		O1 = RISE(IX0.0,clock);		// clock has been defined
 *	This function goes HI when its input goes from LO to HI (rising edge).
 *	The function goes LO with the next clock in the slave action.
 *
 *	NOTE: global variable 'iC_gx' is shared with scan.c (declared in icc.h)
 *	Gate *		iC_gx;	// used to point to action Gate in riMbit
 *
 *******************************************************************/

void
iC_riMbit(					/* RI_BIT master action on EF */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate **	ma;

    if (gm->gt_val < 0 || gm->gt_next) {
	iC_gx = (ma = gm->gt_list)[FL_GATE];	/* gm->gt_funct */
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, " %+d", gm->gt_val);
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
	iC_liveData(gm, gm->gt_val < 0 ? 1 : 0); /* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    if (iC_dc++ >= 4) {
		iC_dc = 1;
		fprintf(iC_outFP, "\n\t");
	    }
	    fprintf(iC_outFP, "\t%s %+d E=>",	/* slave */
		iC_gx->gt_ids, iC_gx->gt_val);
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	iC_gx->gt_val = gm->gt_val;		/* immediate (glitch resets here if linked) */
	iC_link_ol(iC_gx, iC_oList);		/* set (reset) slave output now - reset when clocked*/
    } else {
#if defined(TCP) || defined(LOAD)
	iC_linked++;				/* show change even when no action */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_riMbit */

/********************************************************************
 *
 *	Slave function for RISE in logical input.
 *	The function goes LO again here
 *
 *******************************************************************/

void
iC_riSbit(					/* RI_BIT slave action */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate *	gs;

    gs = gm->gt_funct;
    if (gs->gt_val < 0) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\t-1\t%s %+d E=>", gs->gt_ids, gs->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	gs->gt_val = 1;				/* reset slave output to LO */
	iC_link_ol(gs, iC_oList);		/* on startup a reset action comes when EF is LO */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, " +1");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
	iC_liveData(gs, 0);			/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_riSbit */

/********************************************************************
 *
 *	Master function for CHANGE in logical input.
 *		O2 = CHANGE(IX0.2);		// iClock is default
 *		O3 = CHANGE(IX0.3,clock);	// clock has been defined
 *	This function goes HI when its logical input changes from LO to HI
 *	or from HI to LO.
 *	The function goes LO with the next clock in the slave action.
 *
 *	NOTE: global variable 'iC_gx' is shared with scan.c (declared in icc.h)
 *	Gate *		iC_gx;	// used to point to action Gate in chMbit
 *
 *******************************************************************/

void
iC_chMbit(					/* CH_BIT master action on VF */
    Gate *	gm,
    Gate *	out_list)
{
    Gate **	ma;

    assert (out_list == iC_oList);		/* logic change */
    iC_gx = (ma = gm->gt_list)[FL_GATE];	/* gm->gt_funct */
    iC_link_ol(gm, ma[FL_CLK]);			/* ignore input - master action */
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
	fprintf(iC_outFP, " %+d", gm->gt_val);
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
    iC_liveData(gm, gm->gt_val < 0 ? 1 : 0);	/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
	if (iC_dc++ >= 4) {
	    iC_dc = 1;
	    fprintf(iC_outFP, "\n\t");
	}
	fprintf(iC_outFP, "\t%s %+d V=>",	/* slave */
	    iC_gx->gt_ids, iC_gx->gt_val);
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    iC_gx->gt_val = -iC_gx->gt_val;		/* immediate (glitch resets here if linked) */
    iC_link_ol(iC_gx, iC_oList);		/* set (glitch reset) slave output now - reset when clocked*/
} /* iC_chMbit */

/********************************************************************
 *
 *	Slave function for CHANGE in logical input.
 *	The function goes LO again here
 *
 *******************************************************************/

void
iC_chSbit(					/* CH_BIT slave action */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate *	gs;

    gs = gm->gt_funct;
    if (gs->gt_val < 0) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\t%+d\t%s %+d V=>", gm->gt_val, gs->gt_ids, gs->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	gs->gt_val = 1;				/* reset slave output to LO */
	iC_link_ol(gs, iC_oList);		/* on startup a reset action comes when VF is LO */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, " +1");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
	iC_liveData(gs, 0);			/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_chSbit */

/********************************************************************
 *
 *	Master function for CHANGE in arithmetic input.
 *		O4 = CHANGE(IB4);	// iClock is default
 *		O5 = CHANGE(IB5,clock);	// clock has been defined
 *	This function goes HI when its arithmetic input changes.
 *	The function goes LO with the next clock in the slave action.
 *
 *	NOTE: global variable 'iC_gx' is shared with scan.c (declared in icc.h)
 *	Gate *		iC_gx;	// used to point to action Gate in chMbit
 *
 *******************************************************************/

void
iC_chMar(					/* CH_AR master action on VF */
    Gate *	gm,
    Gate *	out_list)
{
    Gate **	ma;

    assert (out_list == iC_aList);		/* arithmetic change */
    if ((gm->gt_new != gm->gt_old) ^ (gm->gt_next != 0)) {
	iC_gx = (ma = gm->gt_list)[FL_GATE];	/* gm->gt_funct */
	iC_link_ol(gm, ma[FL_CLK]);		/* ignore input - master action */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
#if INT_MAX == 32767 && defined (LONG16)
	    fprintf(iC_outFP, " %ld", gm->gt_new);
#else
	    fprintf(iC_outFP, " %d", gm->gt_new);
#endif
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
	iC_liveData(gm, gm->gt_new);		/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    if (iC_dc++ >= 4) {
		iC_dc = 1;
		fprintf(iC_outFP, "\n\t");
	    }
	    fprintf(iC_outFP, "\t%s %+d v=>",	/* slave */
		iC_gx->gt_ids, iC_gx->gt_val);
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	iC_gx->gt_val = -iC_gx->gt_val;		/* immediate (glitch resets here if linked) */
	iC_link_ol(iC_gx, iC_oList);		/* set (reset) slave output now - reset when clocked*/
    }
} /* iC_chMar */

/********************************************************************
 *
 *	Slave function for CHANGE in arithmetic input.
 *	The function goes LO again here
 *
 *	Arithmetic change nodes have storage for new and old.
 *	Up to this point changes in new back to old could have
 *	caused this Gate to be unlinked (glitch), which is no change.
 *
 *******************************************************************/

void
iC_chSar(					/* CH_AR slave action */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate *	gs;

    gm->gt_old = gm->gt_new;			/* now new value is fixed */
    gs = gm->gt_funct;
    if (gs->gt_val < 0) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
#if INT_MAX == 32767 && defined (LONG16)
	    fprintf(iC_outFP, "\t%ld\t%s %+d v=>", gm->gt_new, gs->gt_ids, gs->gt_val);
#else
	    fprintf(iC_outFP, "\t%d\t%s %+d v=>", gm->gt_new, gs->gt_ids, gs->gt_val);
#endif
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	gs->gt_val = 1;				/* reset slave output to LO */
	iC_link_ol(gs, iC_oList);		/* on startup a reset action comes when VF is LO */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, " +1");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
	iC_liveData(gs, 0);			/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_chSar */

/********************************************************************
 *
 *	Master function for SWITCH().
 *
 *******************************************************************/

void
iC_fMsw(					/* F_SW master action */
    Gate *	gm,
    Gate *	out_list)
{
    if (out_list == iC_oList) {
	/* called from logic scan - convert d to a */
	gm->gt_new = gm->gt_val < 0 ? 1 : 0;
    }
    if ((gm->gt_new != gm->gt_old) ^ (gm->gt_next != 0)) {
	iC_link_ol(gm, gm->gt_clk);		/* master action */
    } else {
#if defined(TCP) || defined(LOAD)
	iC_linked++;				/* show change even when no action */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_fMsw */

/********************************************************************
 *
 *	Slave function for SWITCH().
 *
 *	Execute a C statement or C statement block when triggered
 *	by a clocked logical or arithmetic change.
 *		switch (expr) { case 1: i = 1; break; ... }
 *
 *******************************************************************/

void
iC_fSsw(					/* F_SW slave action on SW */
    Gate *	gm,
    Gate *	out_list)
{
    if (out_list == iC_cList) {
	/* defer execution until iC_fList is scanned */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\tdefer () ");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	iC_link_ol(gm, iC_fList);
    } else {
	assert(out_list == iC_fList);
	gm->gt_old = gm->gt_new;		/* now new value is fixed */
	/* execute C function as action procedure with side effects */
#ifdef LOAD
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
	if (iC_debug & 0100) fprintf(iC_outFP, "\t%2ld SW0x%lx{\n", gm->gt_new, (long)gm->gt_funct);
#else
	if (iC_debug & 0100) fprintf(iC_outFP, "\t%2d SW0x%lx{\n", gm->gt_new, (long)gm->gt_funct);
#endif
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	((iC_CFunctp)(gm->gt_funct))(gm);
#else	/* ! LOAD */
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
	if (iC_debug & 0100) fprintf(iC_outFP, "\t%2ld SW%d{\n", gm->gt_new, gm->gt_functn);
#else
	if (iC_debug & 0100) fprintf(iC_outFP, "\t%2d SW%d{\n", gm->gt_new, gm->gt_functn);
#endif
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	iC_exec(gm->gt_functn, gm);		/* must pass both -/+ */
#endif	/* ! LOAD */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "}");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    }
} /* iC_fSsw */

/********************************************************************
 *
 *	Master function for IF()
 *
 *	if (expr) { ; }			triggers slave on rising edge only
 *
 *******************************************************************/

void
iC_fMcf(					/* F_CF master action */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    if (
	gm->gt_val < 0  ||			/* rising edge */
	gm->gt_next				/* or glitch */
    ) {
	iC_link_ol(gm, gm->gt_clk);		/* master action */
    } else {
#if defined(TCP) || defined(LOAD)
	iC_linked++;				/* show change even when no action */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_fMcf */

/********************************************************************
 *
 *	Master function for IF() ELSE
 *
 *	if (expr) { ; } else { ; }	triggers slave on both edges
 *
 *	The same C code fragment is executed on bothe edges. The code
 *	fragment has an if else test to decide which code to execute.
 *
 *******************************************************************/

void
iC_fMce(					/* F_CE master action */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    iC_link_ol(gm, gm->gt_clk);			/* master action */
} /* iC_fMce */

/********************************************************************
 *
 *	Slave function for IF() IF() ELSE.
 *
 *	Execute a C statement or C statement block when triggered
 *	by a clocked logical or arithmetic change.
 *		if (expr) { x = y * cfn(z); }	// iClock is default
 *		if (expr,clock) { for (i = 0; i < 10; i++) cfn(i); }
 *		if (expr) { i++; cfn(i) } else { i--; cfn(i) }
 *	Note: braces { } around C statements are mandatory
 *
 *******************************************************************/

void
iC_fScf(					/* F_CF and F_CE slave action on CF */
    Gate *	gm,
    Gate *	out_list)
{
    if (out_list == iC_cList) {
	/* defer execution until iC_fList is scanned */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\tdefer {} ");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	iC_link_ol(gm, iC_fList);
    } else {
	assert(out_list == iC_fList);
	/* execute C function as action procedure with side effects */
#ifdef LOAD
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\t%d IF0x%lx{\n", gm->gt_val ? 1 : 0, (long)gm->gt_funct);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	((iC_CFunctp)(gm->gt_funct))(gm);
#else	/* ! LOAD */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\t%d IF%d{\n", gm->gt_val ? 1 : 0, gm->gt_functn);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	iC_exec(gm->gt_functn, gm);		/* must pass both -/+ */
#endif	/* ! LOAD */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "}");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    }
} /* iC_fScf */
#if defined(TCP) || defined(LOAD)

/********************************************************************
 *
 * NEW
 *	Distribute received logical input byte to correct
 *	logical input bit Gate
 *
 *******************************************************************/

int
iC_traMb(					/* TRAB master action */
    Gate *	gm,				/* NOTE: there is no slave action */
    Gate *	out_list)			/* not used */
{
    int		diff;
    int		index;
    int		mask;
    int		val;
    int		count;
    Gate *	gs;

    count = 0;
    diff = gm->gt_new ^ gm->gt_old;
    assert (!(diff & ~0xff));			/* diff may be 0 in rare cases when called with gt_new == gt_old */
    while (diff) {
	index = iC_bitIndex[diff];		/* returns 0 - 7 for values 1 - 255 (avoid 0) */
	mask  = iC_bitMask[index];		/* returns hex 01 02 04 08 10 20 40 80 */
	if ((gs = gm->gt_list[index]) != 0) {	/* is bit Gate allocated ? */
	    val   = (gm->gt_new & mask) ? -1 : 1;	/* yes */
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) {
		if (iC_dc++ >= 4) {
		    iC_dc = 1;
		    fprintf(iC_outFP, "\n\t");
		}
		fprintf(iC_outFP, "\t%s %+d ==>", gs->gt_ids, gs->gt_val);
	    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    if (gs->gt_val != val) {
		gs->gt_val = val;
		iC_link_ol(gs, iC_oList);	/* fire input bit Gate */
		count++;			/* count Gates linked */
	    }
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, " %+d", gs->gt_val);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    iC_liveData(gs, gs->gt_val < 0 ? 1 : 0);	/* VCD and/or iClive */
	}
	diff &= ~mask;				/* clear the bit just processed */
    }
    gm->gt_old = gm->gt_new;
    if (count) {
	iC_linked++;
	iC_liveData(gm, gm->gt_new);		/* iClive only */
    }
    return count;				/* used to adjust YYDEBUG output */
} /* iC_traMb */

/********************************************************************
 *
 * NEW
 *	Output to a word or byte whose Channel index is in gt_channel,
 *	which is in union with gt_list.
 *		gt_mark == B_WIDTH means 1 byte is output
 *		gt_mark == W_WIDTH means 2 bytes or 1 word is output
 *		gt_mark == L_WIDTH means 4 bytes or 1 long is output
 *
 *	For initialisation purposes this ftype OUTW node is acted on
 *	by exactly one ARITH node defined in an output assignment.
 *	This must line up with OUTP_M (1). This is checked in i_ff3().
 *	    QB1_0 <== QB1 = b1; // output b1 to byte on registered channel
 *	    QW2_0 <== QW2 = w2; // output w2 to word on registered channel
 *	    QL4_0 <== QL4 = l4; // output l4 to word on registered channel
 *	QB1, QW2  or QL4 can be used as arithmetic values (ftype is ARITH)
 *	b1, w2  or l4 are expressions or alias's (ftype is ARITH)
 *	they may be any logical or arithmetic expression including INPUT.
 *
 *	An ftype OUTW node QXx is also acted on by all bit outputs QXx.y
 *	These execute outMx(), which links its output directly to iC_sList.
 *	These extra nodes are generated at load time - not compiled.
 *
 *	NOTE: This action does not act on a clocked slave object.
 *	      It was linked to iC_sList, which is scanned only once when
 *	      all other lists have been scanned. This ensures each output
 *	      is only sent once to the external I/O's per cycle when it
 *	      really changes. Glitches do not appear in the output.
 *
 *		out_list 2nd argument	used to defer action to iC_sList
 *
 *******************************************************************/

void
iC_outMw(					/* NEW OUTW master action */
    Gate *		gm,			/* NOTE: there is no slave action */
    Gate *		out_list)
{
#if INT_MAX == 32767 && defined (LONG16)
    long		val;
#else
    int			val;
#endif
#if YYDEBUG && !defined(_WINDOWS)
    int			mask;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    unsigned short	channel;

    if (out_list == iC_aList) {
	/* defer execution until iC_sList is scanned */
	if (
	    (gm->gt_old != gm->gt_new)		/* any change */
	    ^ (gm->gt_next != 0)		/* xor glitch */
	) {
	    iC_link_ol(gm, iC_sList);		/* master action to send list */
	}
    } else {
	/********************************************************************
	 * Enter here from iC_scan_snd() directly for QBx_0, QWx_0, QLx_0
	 * or from iC_outMx() for QXx, which stores bits QXx.0 - QXx.7
	 * Since we are now at the head of iC_sList no changes due to glitches
	 * will occur any more - so gt_new and gt_old can be manipulated.
	 * Signed char and signed short values are sign extended in val for
	 * byte and word outputs. This value is stored in gt_out, which is in
	 * union with gt_rlist which is last used in Pass 6 of the initialisation,
	 * where it is also initialised to 0. No initial assignment for value 0
	 * occurs here. Care is taken in iC_scan_ar() and iC_pass4() to not use
	 * gt_list for testing for the existence of an exec() if gt_ini == OUTW.
	 * Symbol Table updates for iClive take their value from gt_out.
	 * Changes in value are transmitted to iC_live at the end of iC_outMw(),
	 * where val has the correct signed value for byte and word.
	 *******************************************************************/
	assert(out_list == iC_sList);
	assert(gm->gt_new != gm->gt_old);
	assert(gm->gt_mark & (W_MASK | X_MASK));

	channel = gm->gt_channel;		/* set up during registration */
	assert(channel > 0);			/* -1 is error, 0 is not registered */
	val = gm->gt_new;			/* modified value to output */

#if YYDEBUG && !defined(_WINDOWS)
	if ((mask = gm->gt_mark) & W_MASK) {	/* Word output Gate (ignore word length select bits in gt_mark) */
#if	INT_MAX == 32767 && defined (LONG16)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%hu:%ld\t%ld ==>> %ld", channel, val, gm->gt_old, val);
#else	/* INT_MAX == 32767 && defined (LONG16) */
	    if (iC_debug & 0100) fprintf(iC_outFP, "%hu:%d\t%d ==>> %d", channel, val, gm->gt_old, val);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	} else
	if (mask & X_MASK) {			/* Gate that output bits were xferred to in iC_outMx */
	    assert(!((int)val & ~X_MASK));
	    if (iC_debug & 0100) fprintf(iC_outFP, "%hu:%d\t0x%02x ==>> 0x%02x", channel, (int)val, (int)gm->gt_old, (int)val);
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */

	gm->gt_old = gm->gt_out = val;		/* update gt_old now - gt_out for window scrolling */
	iC_output(val, channel);		/* Output int/long data as a message to iCserver or directly */
	iC_linked++;
    }
} /* iC_outMw */

/********************************************************************
 *
 * NEW
 *	Transfer this output bit to the associated byte output Gate
 *		gt_mark contains the bit position as an 8 bit mask.
 *
 *******************************************************************/

void
iC_outMx(					/* NEW OUTX master action */
    Gate *	gm,				/* NOTE: there is no slave action */
    Gate *	out_list)			/* not used */
{
    int		mask;
    int		val;
    Gate *	gs;

    gs = (gm->gt_ptr);				/* OUTW Gate */
    mask = gm->gt_mark;
    val = gs->gt_new;
    if (gm->gt_val < 0) {			/* output action */
	val |= mask;				/* set bit in byte Gate */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) putc('1', iC_outFP);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    } else {
	val &= ~mask;				/* clear bit in byte gate */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) putc('0', iC_outFP);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    }
    assert(val != gs->gt_new);			/* should only fire when there is a change */
    gs->gt_new = val;
    if (
	(gs->gt_old != gs->gt_new)		/* any change - maybe further change in same byte */
	^ (gs->gt_next != 0)			/* in which case node is not linked again - xor glitch */
    ) {
	iC_link_ol(gs, iC_sList);		/* master action to send list */
    }
} /* iC_outMx */
#else /* defined(TCP) || defined(LOAD) */

/********************************************************************
 *
 * OLD - from 1.37
 *	Output to a word or byte whose slot index is in gt_slot.
 *		gt_mark == B_WIDTH means 1 byte is output
 *		gt_mark == W_WIDTH means 2 bytes or 1 word is output
 *		gt_mark == L_WIDTH means 4 bytes or 1 long is output
 *
 *	For initialisation purposes this ftype OUTW node is acted on
 *	by exactly one ARITH node defined in an output assignment.
 *	This must line up with OUTP_M (1). This is checked in i_ff3().
 *		QB1 = b1;	// output b1 to byte at QX_[1]
 *		QW2 = w2;	// output w2 to word at QX_[2]
 *		QL4 = l4;	// output l4 to word at QX_[4]
 *	b1, w2  or l4 can be used as arithmetic values (ftype is ARITH)
 *	and may be any logical or arithmetic function, including INPUT.
 *
 *	NOTE: for word output the I/O address (slot) must be even.
 *	      for long output the I/O address must be on a 4 byte boundary.
 *	      The I/O address (slot) must be < IXD (max 64).
 *	      These are checked by the compiler in iClex() and by
 *	      iCbox and iCserver since rev 1.68
 *
 *	NOTE: This action does not act on a clocked slave object.
 *	      and the node is not linked to any action or clock list
 *
 *******************************************************************/

void
iC_outMw(					/* OLD OUTW master action */
    Gate *	gm,				/* NOTE: there is no slave action */
    Gate *	out_list)
{
#if INT_MAX == 32767 && defined (LONG16)
    long	val;
#else
    int		val;
#endif
    int		slot;
    int		cage;
    int		mask;

    if (gm->gt_new != gm->gt_old) {
	slot = gm->gt_slot;
	mask = gm->gt_mark;
	assert(slot < min(IXD, 64) && mask);	/* IXD must be <= 64 for this scheme */
	cage = slot >> 3;			/* test here because of cage algorithm */
#ifdef MIXED
	if (out_list == iC_oList) {
	    /* called from logic scan - convert d to a */
	    /* MIXED mode is currently not compiled - ERROR */
	    gm->gt_new = gm->gt_val < 0 ? 1 : 0;
	}
#endif

	val = gm->gt_old = gm->gt_new;		/* update gt_old since no iC_link_ol */
	if (mask == B_WIDTH) {
#ifndef _WINDOWS
	    val &= 0xff;			/* for display only */
#endif
	    iC_QX_[slot] = val;			/* output byte to slot */
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%hd", (short)val & 0xff);	/* byte */
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	} else if (mask == W_WIDTH) {
#ifndef _WINDOWS
	    val &= 0xffff;			/* for display only */
#endif
	    assert((slot & 0x01) == 0);		/* even I/O word address */
	    *(short*)&iC_QX_[slot] = val;	/* output word to slot and slot+1 */
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%hd", (short)val);	/* word */
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if INT_MAX != 32767 || defined (LONG16)
	} else if (mask == L_WIDTH) {
	    assert((slot & 0x03) == 0);		/* 4 byte I/O long address */
#if INT_MAX == 32767
	    *(long*)&iC_QX_[slot] = val;	/* output long from slot to slot+3 */
	#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%ld", val);	/* long */
	#endif	/* YYDEBUG && !defined(_WINDOWS) */
#else
	    *(int*)&iC_QX_[slot] = val;		/* output long from slot to slot+3 */
	#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%d", val);	/* long */
	#endif	/* YYDEBUG && !defined(_WINDOWS) */
#endif
#endif
#ifndef _WINDOWS
	} else {
	    val = 0;				/* error - no output */
#endif
	}
	iC_QM_[cage] |= iC_bitMask[slot & 0x7];	/* mark the cage */
	iC_QMM |= iC_bitMask[cage & 0x7];	/* mark the rack */
    }
} /* iC_outMw */

/********************************************************************
 *
 * OLD
 *	Output a bit to a bit field whose slot index is in gt_slot.
 *		gt_mark contains the bit position as an 8 bit mask.
 *
 *	For initialisation purposes this ftype OUTX Gate is acted on
 *	by exactly one GATE node defined in an output assignment.
 *	This must line up with OUTP_M (1). This is checked in i_ff3().
 *		QX0.0 = O0;	// output O0 to bit 0.0
 *	O0 can be used as a logical value (ftype is GATE) and may be
 *	any logical or arithmetic function, including INPUT.
 *
 *	NOTE: This action does not act on a clocked slave object.
 *	      and the node is not linked to any action or clock list
 *
 *******************************************************************/

void
iC_outMx(					/* OLD OUTX master action */
    Gate *	gm,				/* NOTE: there is no slave action */
    Gate *	out_list)
{
    int			slot;
    int			cage;
    unsigned char	mask;

    slot = gm->gt_slot;
    mask = (unsigned char)gm->gt_mark;
    assert(slot < min(IXD, 64) && mask);/* IXD must be <= 64 for this scheme */
    cage = slot >> 3;				/* test here because of cage algorithm */
#ifdef MIXED
    if (out_list == iC_aList) {
	/* called from arithmetic scan - convert a to d */
	/* MIXED mode is currently not compiled - ERROR */
	gm->gt_val = gm->gt_new ? -1 : 1;
    }
#endif
    if (gm->gt_val < 0) {			/* output action */
	iC_QX_[slot] |= mask;			/* set bit at slot,mask */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) putc('1', iC_outFP);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    } else {
	iC_QX_[slot] &= ~mask;			/* clear bit at slot,mask */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) putc('0', iC_outFP);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    }
    iC_QM_[cage] |= iC_bitMask[slot & 0x7];	/* mark the cage */
    iC_QMM |= iC_bitMask[cage & 0x7];		/* mark the rack */
} /* iC_outMx */
#endif /* defined(TCP) || defined(LOAD) */

/********************************************************************
 *
 *	Master function for CLOCK() and TIMER().
 *
 *******************************************************************/

void
iC_fMfn(					/* CLCK TIMR master action */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    if ( gm->gt_val < 0 || gm->gt_next) {
	iC_link_ol(gm, gm->gt_clk);		/* master action */
    } else {
#if defined(TCP) || defined(LOAD)
	iC_linked++;				/* show change even when no action */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_fMfn */

/********************************************************************
 *
 *	Clock driver function.
 *
 *	The output of this function is a clock, which can be used
 *	to synchronise all clocked functions. The clock driver can
 *	itself be synchronised by another clock. Such a clock output
 *	will be active in the same clock period as the synchronising
 *	clock. The default clock for all synchronised functions is
 *	iClock, which starts a clock scan after every terminating
 *	arithmetic and logic scan. The iClock scan is loosely
 *	equivalent to Step 0 in a conventional PLC.
 *
 *	Clock outputs are not logic signals but events of zero duration
 *	as far as logic signals are concerned. During these zero
 *	duration clock events, all slave actions dependent on a particular
 *	clock are executed, resulting in possible changes to outputs.
 *	It is of major importance that all outputs which do change as
 *	a result of clocked actions have all changed simultaneously
 *	during one clock event, as far as the next arithmetic and logic
 *	scan is concerned.
 *
 *		clk1 = CLOCK(I6);		// iClock is default
 *		clk2 = CLOCK(I7,clk1);
 *
 *******************************************************************/

void
iC_clockSfn(					/* Clock function */
    Gate *	gm,
    Gate *	out_list)			/* not used */
{
    Gate *	gs;
#if YYDEBUG && !defined(_WINDOWS) || defined(DEQ)
    Gate *	tp;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#ifdef DEQ
    Gate *	np;
#endif
    /********************************************************************
     * the clock list is controlled by a control block which is the slave
     * Gate structure which goes with this function
     *******************************************************************/
    assert(gm->gt_val < 0);			/* C clock receives -1 ==> 1 ??? */
    gs = gm->gt_funct;
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
	fprintf(iC_outFP, "\t-1\t%s C:", gs->gt_ids);
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    gs->gt_val = -1;				/* set for visualization only */
    if (gs->gt_next != gs) {			/* skip if this clock list empty */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    iC_dc = 0;				/* functions which are clocked */
	    for (tp = gs->gt_next; tp != gs; tp = tp->gt_next) {
		if (iC_dc++ >= 8) {
		    iC_dc = 1;
		    fprintf(iC_outFP, "\n\t");
		}
		fprintf(iC_outFP, "\t%s", tp->gt_ids);
	    }
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#ifndef DEQ
	iC_cList->gt_ptr->gt_next = gs->gt_next;
	iC_cList->gt_ptr = gs->gt_ptr; 	/* link gs */
	iC_cList->gt_ptr->gt_next = iC_cList;	/* to iC_cList */
#else	/* DEQ */
	/* link chain of Gates on clock list gs to end of iC_cList */
	tp = iC_cList->gt_prev;		/* save iC_cList.previous */
	tp->gt_next = np = gs->gt_next;		/* iC_cList.last ==> new */
	np->gt_prev = tp;			/* iC_cList.last <== new */
	tp = gs->gt_prev;			/* save gs.previous */
	tp->gt_next = iC_cList;		/* gs.last ==> iC_cList */
	iC_cList->gt_prev = tp;		/* gs.last <== iC_cList */
#endif	/* DEQ */
#if defined(TCP) || defined(LOAD)
    iC_linked++;
    iC_liveData(gs, 1);				/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
	Out_init(gs);				/* relink empty gs */
    }
} /* iC_clockSfn */

iC_Functp	iC_clock_i[] = {iC_pass1, iC_null1, i_ff3, iC_null1};	/* no output lists */

/********************************************************************
 *
 *	Timer driver function.
 *
 *	The output of this function can be used wherever a clock
 *	may be used. For details see manual.
 *
 *		tim1 = TIMER(I8);		// iClock is default
 *		tim2 = TIMER(I9,tim1,3);
 *
 *******************************************************************/

void
iC_timerSfn(					/* Timer function */
    Gate *		gm,
    Gate *		out_list)		/* not used */
{
    Gate *		gs;
    Gate *		tp;			/* functions which are timed */
    Gate *		np;
    unsigned short	tc;			/* first entry timer count */
    /********************************************************************
     * the timer list is controlled by a control block which is the slave
     * Gate structure which goes with this function
     *******************************************************************/
    assert(gm->gt_val < 0);			/* T timer receives -1 ==> 1 ??? */
    gs = gm->gt_funct;
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
	fprintf(iC_outFP, "\t-1\t%s T", gs->gt_ids);
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    gs->gt_val = -1;				/* set for visualization only */
    if ((np = gs->gt_next) != gs) {		/* skip if this clock list empty */
	tc = --(np->gt_mark);			/* count down first entry */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    iC_dc = 1;				/* allow for (time) */
	    fprintf(iC_outFP, "!(%hu)\t%s", tc, np->gt_ids);
	}
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	if (tc == 0) {
#ifdef DEQ
	    /* DEQ link head of timer chain gs to end of iC_cList */
	    tp = iC_cList->gt_prev;		/* save iC_cList.previous */
	    tp->gt_next = np;			/* iC_cList.last ==> new */
	    np->gt_prev = tp;			/* iC_cList.last <== new */
#endif
	    do {
		tp = np;
		if ((np = tp->gt_next) == gs) {
		    break;			/* end of list found */
#if YYDEBUG && !defined(_WINDOWS)
		} else if (iC_debug & 0100) {
		    if (iC_dc++ >= 8) {
			iC_dc = 1;
			fprintf(iC_outFP, "\n\t");
		    }
		    fprintf(iC_outFP, "\t!(%hu)\t%s", np->gt_mark, np->gt_ids);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		}
	    } while (np->gt_mark == 0);		/* unlink sequence of 0 time entries */
#ifndef DEQ
	    iC_cList->gt_ptr->gt_next = gs->gt_next;/* => new */
	    gs->gt_next = np;			/* timer => rest */
	    iC_cList->gt_ptr = tp;		/* clock last => new last */
	    tp->gt_next = iC_cList;    	/* new last => iC_cList */
	    if (np == gs) {			/* last entry ? */
		gs->gt_ptr = np;		/* yes - fix timer last */
	    }
#else	/* DEQ */
	    /* DEQ link tail of timer chain which is due to iC_cList */
	    tp->gt_next = iC_cList;		/* gs.last ==> iC_cList */
	    iC_cList->gt_prev = tp;		/* gs.last <== iC_cList */
	    /* the re-linking of gs works correctly also if np == gs */
	    gs->gt_next = np;			/* gs ==> new new */
	    np->gt_prev = gs;			/* gs <== new new */
#endif	/* DEQ */
	}
#if defined(TCP) || defined(LOAD)
	iC_linked++;
	iC_liveData(gs, tc);			/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
    }
} /* iC_timerSfn */

/********************************************************************
 *
 *	Assign to an immC int node (type ARNC) with a choice of straight
 *	assignment as well as pre/post increment/decrement operators
 *	and all the assignment operators.
 *
 *	The node is linked to iC_aList when rv != new && new == old.
 *	If a second assignment occurs while node is still linked to
 *	iC_aList (new != old), if rv != old new is simply updated
 *	else node is unlinked and new updated, making new == old.
 *	When rv == new, there is no change at all.
 *
 *	This new implementation (jw 120722) is passed glv once and
 *	does not need to be called twice for ++/-- or += which solves
 *	the problem of side effects from calling macros with indexed
 *	operators supplying glv twice. Also the C generator in gram.y
 *	is much cleaner and works with immC variables inside index
 *	expressions.
 *
 *	If glv->gt_fni == UDFA, an out of range Warning was issued on
 *	the console and no assignment occurs here. This is done so that
 *	iC programs keep running despite an attempted out of range array
 *	reference.
 *
 *	Return value is the value assigned or 0 if index was out of range.
 *
 *	NOTE: the switch cases for assignment operators must line up
 *	with the ppi numbers defined in gram.y for 'assignment_operator'
 *	and immediate INC_OP/DEC_OP expressions.
 *
 *******************************************************************/
#if YYDEBUG && !defined(_WINDOWS)

static char *	ppiS[] = {
    "=",
    "=",
    "*=",
    "/=",
    "%=",
    "+=",
    "-=",
    "&=",
    "^=",
    "|=",
    "<<=",
    ">>=",
    "++",
    "--",
    "++",
    "--",
};
#endif	/* YYDEBUG && !defined(_WINDOWS) */

#if INT_MAX == 32767 && defined (LONG16)
long
iC_assignA(Gate * glv, int ppi, long rv) {	/* } */
    long nv;
#if YYDEBUG && !defined(_WINDOWS)
    long iv = rv;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#else
int
iC_assignA(Gate * glv, int ppi, int rv) {
    int nv;
#if YYDEBUG && !defined(_WINDOWS)
    int iv = rv;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#endif
    if (glv->gt_fni == UDFA) return 0;		/* index out of range */
    assert(glv->gt_ini == -ARNC);
    nv = glv->gt_new;
    switch (ppi) {
    case 2:
	rv = nv *= rv;
	break;
    case 3:
	rv = nv /= rv;
	break;
    case 4:
	rv = nv %= rv;
	break;
    case 5:
	rv = nv += rv;
	break;
    case 6:
	rv = nv -= rv;
	break;
    case 7:
	rv = nv &= rv;
	break;
    case 8:
	rv = nv ^= rv;
	break;
    case 9:
	rv = nv |= rv;
	break;
    case 10:
	rv = nv <<= rv;
	break;
    case 11:
	rv = nv >>= rv;
	break;
    case 12:
	rv = ++nv;
	break;
    case 13:
	rv = --nv;
	break;
    case 14:
	rv = nv++;
	break;
    case 15:
	rv = nv--;
	break;
    case 0:
    case 1:
    default:
	nv = rv;
    }
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
#if INT_MAX == 32767 && defined (LONG16)
	if (ppi < 12) {
	    fprintf(iC_outFP, "\tAA %ld %s %s %ld >", glv->gt_new, glv->gt_ids, ppiS[ppi], iv);
	} else if (ppi < 14) {
	    fprintf(iC_outFP, "\tAA %ld %s%s   >", glv->gt_new, ppiS[ppi], glv->gt_ids);
	} else {
	    fprintf(iC_outFP, "\tAA %ld   %s%s >", glv->gt_new, glv->gt_ids, ppiS[ppi]);
	}
#else
	if (ppi < 12) {
	    fprintf(iC_outFP, "\tAA %d %s %s %d >", glv->gt_new, glv->gt_ids, ppiS[ppi], iv);
	} else if (ppi < 14) {
	    fprintf(iC_outFP, "\tAA %d %s%s   >", glv->gt_new, ppiS[ppi], glv->gt_ids);
	} else {
	    fprintf(iC_outFP, "\tAA %d   %s%s >", glv->gt_new, glv->gt_ids, ppiS[ppi]);
	}
#endif
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    if (nv != glv->gt_new) {
	if (glv->gt_new == glv->gt_old ||	/* first arithmetic change */
	    nv == glv->gt_old) {		/* or glitch to old value */
	    iC_link_ol(glv, iC_aList);
	}
	glv->gt_new = nv;			/* first or later change */
    }
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
    if (iC_debug & 0100) fprintf(iC_outFP, " %ld\n", nv);
#else
    if (iC_debug & 0100) fprintf(iC_outFP, " %d\n", nv);
#endif
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
    iC_liveData(glv, nv);			/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
    return rv;
} /* iC_assignA */

/********************************************************************
 *
 *	Assign to an immC bit node (type LOGC) with a choice of straight
 *	assignment as well as pre/post increment/decrement operators
 *	and all the assignment operators.
 *
 *	Straight assignment makes a lot of sense for bit variables.
 *	It also makes sense to interpret the complement operator '~'
 *	as applying to the input rv interpreted as a bit with values
 *	0 and 1 whose complement ~rv is 1 and 0. This means the
 *	    value assigned is 0 if rv == 0 or rv == ~1 (-2)
 *	    value assigned is 1 if rv == 1 or rv == ~0 (-1) or any
 *	    value except 0 or -2
 *	This fudge is now only relevant if the value rv to be assigned
 *	is a numerical 0 or 1 for lo or hi or an imm or immc int variable,
 *	because the output of all complement operations with the ~ operator
 *	on imm or immC bit values in C code is now carried out by leaving
 *	out the '~' operator and supplying an extra parameter inv to all
 *	bit manipulation macros and this function, which causes the output
 *	to be the proper bit complement if inv == 1.
 *
 *	Other assignment operators are of doubtful utility for immC bits
 *	but are included anyway for completeness. In this case input rv
 *	is treated normally as in C for the "!" operator
 *	    value assigned is 0 if rv == 0
 *	    value assigned is 1 if rv != 0
 *
 *	The node is linked to iC_oList when rv, interpreted as above
 *	changes from its previous value.
 *
 *	If glv->gt_fni == UDFA, an out of range Warning was issued on
 *	the console and no assignment occurs here. This is done so that
 *	iC programs keep running despite an attempted out of range array
 *	reference.
 *
 *	Return is the assigned value converted to the bit value 0 or 1
 *	or the complemented bit value 1 or 0 if the input parameter inv is 1
 *	or 0 if index was out of range.
 *
 *******************************************************************/

#if INT_MAX == 32767 && defined (LONG16)
long
iC_assignL(Gate * glv, int inv, int ppi, long rv) {	/* } */
    long nv;
#if YYDEBUG && !defined(_WINDOWS)
    long iv = rv;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#else
int
iC_assignL(Gate * glv, int inv, int ppi, int rv) {
    int nv;
#if YYDEBUG && !defined(_WINDOWS)
    int iv = rv;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#endif
    signed char val;
    if (glv->gt_fni == UDFA) return 0;	/* index out of range */
    assert(glv->gt_ini == -LOGC);
    nv = glv->gt_val < 0 ? 1 : 0;
    switch (ppi) {
    case 2:
	rv = nv *= rv;
	break;
    case 3:
	rv = nv /= rv;
	break;
    case 4:
	rv = nv %= rv;
	break;
    case 5:
	rv = nv += rv;
	break;
    case 6:
	rv = nv -= rv;
	break;
    case 7:
	rv = nv &= rv;
	break;
    case 8:
	rv = nv ^= rv;
	break;
    case 9:
	rv = nv |= rv;
	break;
    case 10:
	rv = nv <<= rv;
	break;
    case 11:
	rv = nv >>= rv;
	break;
    case 12:
	rv = ++nv;
	break;
    case 13:
	rv = --nv;
	break;
    case 14:
	rv = nv++;
	break;
    case 15:
	rv = nv--;
	break;
    case 0:
    case 1:
    default:
	if (rv == ~1) rv = 0;		/* bit 0 for straight assignment of ~1 */
	nv = rv;
    }
    val = nv ? -1 : 1;
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
	if (ppi < 12) {
#if INT_MAX == 32767 && defined (LONG16)
	    fprintf(iC_outFP, "\tLA %+d %s %s %ld >", glv->gt_val, glv->gt_ids, ppiS[ppi], iv);
#else
	    fprintf(iC_outFP, "\tLA %+d %s %s %d >", glv->gt_val, glv->gt_ids, ppiS[ppi], iv);
#endif
	} else if (ppi < 14) {
	    fprintf(iC_outFP, "\tLA %+d %s%s   >", glv->gt_val, ppiS[ppi], glv->gt_ids);
	} else {
	    fprintf(iC_outFP, "\tLA %+d   %s%s >", glv->gt_val, glv->gt_ids, ppiS[ppi]);
	}
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    if (glv->gt_val != val) {
	glv->gt_val = val;
	iC_link_ol(glv, iC_oList);	/* logic change or glitch */
    }
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
	fprintf(iC_outFP, " %+d\n", val);
    }
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#if defined(TCP) || defined(LOAD)
    iC_liveData(glv, (val < 0) ? 1 : 0);	/* VCD and/or iClive */
#endif /* defined(TCP) || defined(LOAD) */
    return (rv != 0) ^ inv;	/* change to logic value 0 or 1 or its complement */
} /* iC_assignL */

/********************************************************************
 *
 *	Evaluate an indexed reference to an immC array
 *
 *	The compiler stores the size of the initialised immC array in gt_old.
 *	An index within range returns the indexed member of the immC array.
 *
 *	If there is a range error a Warning is issued on the console and the
 *	array itself is returned.  The arithmetic or logical value returned
 *	if this is accessed is 0.  It is also checked by iC_assignA() and
 *	iC_assignL(), which return arithmetic or logic 0 if assignment to an
 *	out of range array member is attempted. No array member is modified.

 *	This is done so that iC programs keep running - it would be
 *	drastic to quit a running control application for a range error.
 *
 *******************************************************************/

Gate *
iC_index(Gate * gm, int index)
{
    assert((gm->gt_ini == -ARNC || gm->gt_ini == -LOGC) && gm->gt_fni == UDFA);
    if (index >= gm->gt_old || index < 0) {
	fprintf(iC_errFP, "\nWarning: %s: immC array reference %s[%d] is out of bounds (size = %d) - no action\n",
	    iC_progname, gm->gt_ids, index, (int)gm->gt_old);
	assert(gm->gt_new == 0 && gm->gt_val >= 0);
	return gm;			/* array itself - returns 0 if accessed - stops assignment */
    }
    return gm->gt_rlist[index];		/* immC member of array gm[index] */
} /* iC_index */

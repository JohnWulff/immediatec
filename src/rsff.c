static const char rsff_c[] =
"@(#)$Id: rsff.c,v 1.44 2007/03/07 13:21:44 jw Exp $";
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
 *	rsff.c
 *	parallel plc - SR flip flop and other clocked functions
 *
 *******************************************************************/

/* J.E. Wulff	8-Mar-85 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<assert.h>
#include	"icc.h"
#ifdef TCP
#include	"tcpc.h"
#endif /* TCP */

#ifndef	WIN32
#define min(x,y) ((x) < (y) ? (x) : (y))
#endif	/* WIN32 */

Gate		iConst = { 1, -NCONST, ARITH, 0, "iConst", 0, 0, 0 };

/********************************************************************
 *
 *	bitIndex[] is a 256 byte array, whose values represent the
 *	bit position 0 - 7 of the rightmost 1 bit of the array index
 *
 *******************************************************************/

unsigned char	iC_bitIndex[]   = {
 0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
};

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
 *					deferred action f_list
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
    Gate *	out_list)
{
    Gate **	ma;

#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {
	iC_liveData(gm->gt_live, gm->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    if (
	(
	    (ma = gm->gt_list)[FL_GATE]->gt_val > 0 &&
	    gm->gt_val < 0			/* anything to set */
	) ||
	gm->gt_next				/* or glitch */
    ) {
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
    }
} /* sMff */

/********************************************************************
 *
 *	Slave set action S_FF on flip/flop FF
 *
 *******************************************************************/

void
iC_sSff(					/* S_FF slave action on FF */
    Gate *	gm,
    Gate *	out_list)
{
    Gate *	gs;

    assert(gm->gt_val < 0);			/* S FF receives -1 ==> 1 ??? */
    gs = gm->gt_funct;
    if (gs->gt_val > 0) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\tS %s %2d ==>", gs->gt_ids, gs->gt_val);
	}
#endif
	gs->gt_val = -1;			/* set slave output */
	iC_link_ol(gs, iC_o_list);
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, " %d", gs->gt_val);
	}
#endif
    }
} /* sSff */

/********************************************************************
 *
 *	Master reset action R_FF on flip/flop FF
 *
 *******************************************************************/

void
iC_rMff(					/* R_FF master action on FF */
    Gate *	gm,
    Gate *	out_list)
{
    Gate **	ma;

#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {
	iC_liveData(gm->gt_live, gm->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    if (
	(
	    (ma = gm->gt_list)[FL_GATE]->gt_val < 0 &&
	    gm->gt_val < 0			/* anything to reset */
	) ||
	gm->gt_next				/* or glitch */
    ) {
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
    }
} /* rMff */

/********************************************************************
 *
 *	Slave reset action R_FF on flip/flop FF
 *
 *******************************************************************/

void
iC_rSff(					/* R_FF slave action on FF */
    Gate *	gm,
    Gate *	out_list)
{
    Gate *	gs;

    assert(gm->gt_val < 0);			/* R FF receives -1 ==> 1 ??? */
    gs = gm->gt_funct;
    if (gs->gt_val < 0) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\tR %s %2d ==>", gs->gt_ids, gs->gt_val);
	}
#endif
	gs->gt_val = 1;				/* reset slave output */
	iC_link_ol(gs, iC_o_list);
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, " %d", gs->gt_val);
	}
#endif
    }
} /* rSff */

/********************************************************************
 *
 *	Master delay action D_FF on flip/flop FF
 *
 *******************************************************************/

void
iC_dMff(					/* D_FF master action on FF */
    Gate *	gm,
    Gate *	out_list)
{
    Gate **	ma;

#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {
	iC_liveData(gm->gt_live, gm->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    if (
	((ma = gm->gt_list)[FL_GATE]->gt_val < 0)
	^ (gm->gt_val < 0)			/* any change */
	^ (gm->gt_next != 0)			/* xor glitch */
    ) {
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
    }
} /* dMff */

/********************************************************************
 *
 *	Slave delay action D_FF on flip/flop FF
 *
 *******************************************************************/

void
iC_dSff(					/* D_FF slave action on FF */
    Gate *	gm,
    Gate *	out_list)
{
    Gate *	gs;
    char	val;

    gs = gm->gt_funct;
    if ((val = (gm->gt_val < 0) ? -1 : 1) != gs->gt_val) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\tD %s %2d ==>", gs->gt_ids, gs->gt_val);
	}
#endif
	gs->gt_val = val;			/* transfer val to slave */
	iC_link_ol(gs, iC_o_list);
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, " %d", gs->gt_val);
	}
#endif
    }
} /* dSff */

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

    if (out_list == iC_o_list) {
	/* called from logic scan - convert d to a */
	gm->gt_new = gm->gt_val < 0 ? 1 : 0;
	/********************************************************************
	 * since new is only modified here and since gt_val has changed,
	 * new must differ from old (no need to check).
	 * old will be modified during arithmetic scan, although that
	 * would not be necessary for this action.
	 *******************************************************************/
    }
#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {			/* misses all but first change */
	iC_liveData(gm->gt_live, gm->gt_new);	/* and return to old value */
    }
#endif
    /********************************************************************
     * SH master action is only called if gt_new has changed first time
     * versus gt_old or changed back, which is a glitch. This is already
     * tested in scan_ar() unlike D master action which has to do it here.
     * Nevertheles the action must only be linked, if the new value will
     * changes the master, since that can be manipulated by set or reset.
     *******************************************************************/
    gs = gm->gt_funct;
    if (
	(gm->gt_new != gs->gt_new)		/* any change */
	^ (gm->gt_next != 0)			/* xor glitch */
    ) {
	iC_link_ol(gm, gm->gt_clk);		/* master action */
    }
} /* dMsh */

/********************************************************************
 *
 *	Slave delay action D_SH on sample/hold SH
 *
 *******************************************************************/

void
iC_dSsh(					/* D_SH slave action on SH */
    Gate *	gm,
    Gate *	out_list)
{
    Gate *	gs;

#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {			/* value can change more than once */
	iC_liveData(gm->gt_live, gm->gt_new);	/* output final value here */
    }
#endif
    gs = gm->gt_funct;
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
	if (iC_debug & 0100) fprintf(iC_outFP, "\tSH %s %ld ==>", gs->gt_ids, gs->gt_new);
#else
	if (iC_debug & 0100) fprintf(iC_outFP, "\tSH %s %d ==>", gs->gt_ids, gs->gt_new);
#endif
#endif
    /********************************************************************
     * since gm is still on clock list gt_new must differ from gt_old
     *******************************************************************/
    gs->gt_new = gm->gt_old = gm->gt_new;	/* transfer value to slave */
    iC_link_ol(gs, iC_a_list);			/* fire new arithmetic action */

#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
    if (iC_debug & 0100) fprintf(iC_outFP, " %ld", gs->gt_new);
#else
    if (iC_debug & 0100) fprintf(iC_outFP, " %d", gs->gt_new);
#endif
#endif
} /* dSsh */

/********************************************************************
 *
 *	Master set action S_SH on sample/hold SH
 *
 *******************************************************************/

void
iC_sMsh(					/* S_SH master action on SH */
    Gate *	gm,
    Gate *	out_list)
{
    Gate **	ma;

#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {
	iC_liveData(gm->gt_live, gm->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    if (
	(
	    (ma = gm->gt_list)[FL_GATE]->gt_new != -1 &&
	    gm->gt_val < 0			/* anything to set */
	) ||
	gm->gt_next				/* or glitch */
    ) {
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
    }
} /* sMsh */

/********************************************************************
 *
 *	Slave set action S_SH on sample/hold SH
 *
 *******************************************************************/

void
iC_sSsh(					/* S_SH slave action on SH */
    Gate *	gm,
    Gate *	out_list)
{
    Gate *	gs;

    assert(gm->gt_val < 0);			/* S SH receives -1 ==> 1 ??? */
    gs = gm->gt_funct;
    if (gs->gt_new != -1) {
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
	if (iC_debug & 0100) fprintf(iC_outFP, "\tS SH %s %2ld ==>", gs->gt_ids, gs->gt_new);
#else
	if (iC_debug & 0100) fprintf(iC_outFP, "\tS SH %s %2d ==>", gs->gt_ids, gs->gt_new);
#endif
#endif
	gs->gt_new = -1;			/* set SH slave output */
	iC_link_ol(gs, iC_a_list);
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
	if (iC_debug & 0100) fprintf(iC_outFP, " %ld", gs->gt_new);
#else
	if (iC_debug & 0100) fprintf(iC_outFP, " %d", gs->gt_new);
#endif
#endif
    }
} /* sSsh */

/********************************************************************
 *
 *	Master reset action R_SH on sample/hold SH
 *
 *******************************************************************/

void
iC_rMsh(					/* R_SH master action on SH */
    Gate *	gm,
    Gate *	out_list)
{
    Gate **	ma;

#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {
	iC_liveData(gm->gt_live, gm->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    if (
	(
	    (ma = gm->gt_list)[FL_GATE]->gt_new != 0 &&
	    gm->gt_val < 0			/* anything to reset */
	) ||
	gm->gt_next				/* or glitch */
    ) {
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
    }
} /* rMsh */

/********************************************************************
 *
 *	Slave reset action R_SH on sample/hold SH
 *
 *******************************************************************/

void
iC_rSsh(					/* R_SH slave action on SH */
    Gate *	gm,
    Gate *	out_list)
{
    Gate *	gs;

    assert(gm->gt_val < 0);			/* S SH receives -1 ==> 1 ??? */
    gs = gm->gt_funct;
    if (gs->gt_new != 0) {
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
	if (iC_debug & 0100) fprintf(iC_outFP, "\tS SH %s %2ld ==>", gs->gt_ids, gs->gt_new);
#else
	if (iC_debug & 0100) fprintf(iC_outFP, "\tS SH %s %2d ==>", gs->gt_ids, gs->gt_new);
#endif
#endif
	gs->gt_new = 0;				/* reset SH slave output */
	iC_link_ol(gs, iC_a_list);
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
	if (iC_debug & 0100) fprintf(iC_outFP, " %ld", gs->gt_new);
#else
	if (iC_debug & 0100) fprintf(iC_outFP, " %d", gs->gt_new);
#endif
#endif
    }
} /* rSsh */

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
	    iC_error_flag = 1;			/* cannot execute with this error */
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

void
i_ff3(Gate * gm, int typ)			/* Pass3 init on FF etc. */
{
    unsigned int	mask;			/* action bit mask */
    char		opt;

    if (gm != iC_c_list) {			/* iClock has no inputs */
	mask = iC_bit2[iC_ftypes[typ]] & ~ONCE_M;/* action bit mask required */
	opt = iC_os[typ];
	/* accept only the right number of compatible inputs */
	if (gm->gt_mcnt != mask) {
	    fprintf(iC_outFP,
	    "\nError:    %c	%s\thas %s inputs (%.4x:%.4x)",
		opt, gm->gt_ids, gm->gt_mcnt ? "incompatible" : "no",
		gm->gt_mcnt, mask);
	    iC_error_flag = 1;			/* cannot execute with this error */
#if YYDEBUG && !defined(_WINDOWS)
	} else if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\n	    %c	%s:\t%.4x inputs",
		opt, gm->gt_ids, gm->gt_mcnt);
#endif
	}
    }
    if (typ < MAX_LV) {
	gm->gt_val = 1;				/* set fblk gates to +1 anyway */
	if (typ == SH || typ == INPW) {
	    gm->gt_new = gm->gt_old = 0;	/* clear arithmetic */
	} else if (typ == NCONST && strcmp(gm->gt_ids, "iConst") != 0) {
	    char *	ep;
	    /* convert constant 18 from dec, 077 from oct 0x1c from hex */
	    gm->gt_new = gm->gt_old = strtol(gm->gt_ids, &ep, 0); /* long to int or long */
	    if (*ep != '\0') {
		fprintf(iC_outFP,
		"\nError:    constant '%s' cannot be converted to a number", gm->gt_ids);
		iC_error_flag = 1;		/* cannot execute with this error */
	    }
	}
    } else {
	gm->gt_val = 0;				/* used in visualization */
	Out_init(gm);				/* link as clock or timer list */
	if (gm != iC_c_list) {
	    gm->gt_rlist = (Gate**)iC_c_list;	/* c_list is alternate for link */
	}					/* except c_list which has 0 */
    }
    if (gm->gt_fni != OUTW && gm->gt_fni != OUTX) {
	gm->gt_mark = 0;			/* clear for stamp and diff in actions */
    }
} /* i_ff3 */

iC_Functp	iC_ff_i[] = {iC_pass1, iC_pass2, i_ff3, iC_pass4};

/********************************************************************
 *
 *	Master function to detect RISE in logical input.
 *		O1 = RISE(I1,clock);	// clock has been defined
 *		O2 = RISE(I2);		// iClock is default
 *	This function goes HI when its input goes HI (rising edge).
 *	The function goes LO with the next clock in the slave action.
 *
 *	NOTE: global variable 'iC_gx' is shared with scan.c (declared in icc.h)
 *	Gate *		iC_gx;	// used to point to action Gate in riMbit
 *
 *******************************************************************/

void
iC_riMbit(					/* RI_BIT master action on EF */
    Gate *	gm,
    Gate *	out_list)
{
    Gate **	ma;

#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {
	iC_liveData(gm->gt_live, gm->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    iC_gx = (ma = gm->gt_list)[FL_GATE];	/* gm->gt_funct */
    if (gm->gt_val < 0 || gm->gt_next) {
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, " %d", gm->gt_val);
	    if (iC_dc++ >= 4) {
		iC_dc = 1;
		putc('\n', iC_outFP);
	    }
	    fprintf(iC_outFP, "\t%s %2d E=>",
		iC_gx->gt_ids, iC_gx->gt_val);
	}
#endif
	iC_gx->gt_val = -iC_gx->gt_val;		/* immediate (glitch could reset here) */
	iC_link_ol(iC_gx, iC_o_list);		/* set or reset slave output */
    }
} /* riMbit */

/********************************************************************
 *
 *	Slave function for RISE in logical input.
 *	The function goes LO again here
 *
 *******************************************************************/

void
iC_riSbit(					/* RI_BIT slave action */
    Gate *	gm,
    Gate *	out_list)
{
    Gate *	gs;

    gs = gm->gt_funct;
    if (gs->gt_val < 0) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\tE %s %2d ==>", gs->gt_ids, gs->gt_val);
	}
#endif
	gs->gt_val = 1;				/* reset slave output to LO */
	iC_link_ol(gs, iC_o_list);
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, " %d", gs->gt_val);
	}
	/* on startup a reset action comes when EF is LO */
#endif
    }
} /* riSbit */

/********************************************************************
 *
 *	Master function for CHANGE in logical or arithmetic input.
 *		O3 = CHANGE(I4,clock);	// clock has been defined
 *		O4 = CHANGE(I4);	// iClock is default
 *	This function goes HI when its logical input changes from
 *	HI to LO or from LO to HI or its arithmetic input changes.
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

#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {			/* display all changes */
	iC_liveData(gm->gt_live, gm->gt_ini < 0 ? gm->gt_new
					     : gm->gt_val < 0 ? 1 : 0);
    }
#endif
    if (out_list == iC_o_list ||		/* logic or arithmetic change */
	((gm->gt_new != gm->gt_old) ^ (gm->gt_next != 0))) {
	iC_gx = (ma = gm->gt_list)[FL_GATE];	/* ignore input */
	iC_link_ol(gm, ma[FL_CLK]);		/* master action */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
#if INT_MAX == 32767 && defined (LONG16)
	    fprintf(iC_outFP, " %ld", out_list != iC_o_list? gm->gt_new : (long)gm->gt_val);
#else
	    fprintf(iC_outFP, " %d", out_list != iC_o_list? gm->gt_new : gm->gt_val);
#endif
	    if (iC_dc++ >= 4) {
		iC_dc = 1;
		putc('\n', iC_outFP);
	    }
	    fprintf(iC_outFP, "\t%s %2d V=>",
		iC_gx->gt_ids, iC_gx->gt_val);
	}
#endif
	iC_gx->gt_val = -iC_gx->gt_val;		/* immediate (glitch could reset here) */
	iC_link_ol(iC_gx, iC_o_list);		/* set or reset slave output */
    }
} /* chMbit */

/********************************************************************
 *
 *	Slave function for CHANGE in logical or arithmetic input.
 *	The function goes LO again here
 *
 *	All change nodes have storage for new and old, although only
 *	those fired from ARN type really need it.
 *	Up to this point changes in new back to old could have
 *	caused this Gate to be unlinked (glitch), which is no change.
 *
 *******************************************************************/

void
iC_chSbit(					/* CH_BIT slave action */
    Gate *	gm,
    Gate *	out_list)
{
    Gate *	gs;

    gm->gt_old = gm->gt_new;			/* now new value is fixed */
#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {			/* value can change more than once */
	iC_liveData(gm->gt_live, gm->gt_ini < 0 ?	/* output final value here */
	gm->gt_new : gm->gt_val < 0 ? 1 : 0);
    }
#endif
    gs = gm->gt_funct;
    if (gs->gt_val < 0) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\tV %s %2d ==>", gs->gt_ids, gs->gt_val);
	}
#endif
	gs->gt_val = 1;				/* reset slave output to LO */
	iC_link_ol(gs, iC_o_list);
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, " %d", gs->gt_val);
	}
	/* on startup a reset action comes when VF is LO */
#endif
    }
} /* chSbit */

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
    if (out_list == iC_o_list) {
	/* called from logic scan - convert d to a */
	gm->gt_new = gm->gt_val < 0 ? 1 : 0;
    }
#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {
	iC_liveData(gm->gt_live, gm->gt_new);
    }
#endif
    if ((gm->gt_new != gm->gt_old) ^ (gm->gt_next != 0)) {
	iC_link_ol(gm, gm->gt_clk);		/* master action */
    }
} /* fMsw */

/********************************************************************
 *
 *	Slave function for SWITCH().
 *
 *	Execute a C statement or C statement block when triggered
 *	by a clocked logical or arithmetic change.
 *		SWITCH (expr) { case 1: i = 1; break; ... }
 *
 *******************************************************************/

void
iC_fSsw(					/* F_SW slave action on SW */
    Gate *	gm,
    Gate *	out_list)
{
    if (out_list == iC_c_list) {
	/* defer execution until f_list is scanned */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\tdefer () ");
#endif
	iC_link_ol(gm, iC_f_list);
    } else {
	assert(out_list == iC_f_list);
	gm->gt_old = gm->gt_new;		/* now new value is fixed */
	/* execute C function as action procedure with side effects */
#ifdef LOAD
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\tF%p(\n", (iC_CFunctp)gm->gt_funct);
#endif
	((iC_CFunctp)(gm->gt_funct))(gm);
#else
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\tF%d(\n", (int)gm->gt_funct);
#endif
	iC_exec((int)gm->gt_funct, gm);	/* must pass both -/+ */
#endif
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, ")");
#endif
    }
} /* fSsw */

/********************************************************************
 *
 *	Master function for IF()
 *
 *	IF (expr) { ; }			triggers slave on rising edge only
 *
 *******************************************************************/

void
iC_fMcf(					/* F_CF master action */
    Gate *	gm,
    Gate *	out_list)
{
#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {
	iC_liveData(gm->gt_live, gm->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    if (
	gm->gt_val < 0  ||			/* rising edge */
	gm->gt_next				/* or glitch */
    ) {
	iC_link_ol(gm, gm->gt_clk);		/* master action */
    }
} /* fMcf */

/********************************************************************
 *
 *	Master function for IF() ELSE
 *
 *	IF (expr) { ; } ELSE { ; }	triggers slave on both edges
 *
 *	The same C code fragment is executed on bothe edges. The code
 *	fragment has an if else test to decide which code to execute.
 *
 *******************************************************************/

void
iC_fMce(					/* F_CE master action */
    Gate *	gm,
    Gate *	out_list)
{
#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {
	iC_liveData(gm->gt_live, gm->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    iC_link_ol(gm, gm->gt_clk);			/* master action */
} /* fMce */

/********************************************************************
 *
 *	Slave function for IF() IF() ELSE.
 *
 *	Execute a C statement or C statement block when triggered
 *	by a clocked logical or arithmetic change.
 *		IF (expr,clock) x = y * cfn(z);	// iClock is default
 *		IF (expr) { for (i = 0; i < 10; i++) cfn(z); z++; }
 *		IF (expr) { i++; } ELSE { j--; }
 *
 *******************************************************************/

void
iC_fScf(					/* F_CF and F_CE slave action on CF */
    Gate *	gm,
    Gate *	out_list)
{
    if (out_list == iC_c_list) {
	/* defer execution until f_list is scanned */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\tdefer {} ");
#endif
	iC_link_ol(gm, iC_f_list);
    } else {
	assert(out_list == iC_f_list);
	/* execute C function as action procedure with side effects */
#ifdef LOAD
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\tF%p{\n", (iC_CFunctp)gm->gt_funct);
#endif
	((iC_CFunctp)(gm->gt_funct))(gm);
#else
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "\tF%d{\n", (int)gm->gt_funct);
#endif
	iC_exec((int)gm->gt_funct, gm);	/* must pass both -/+ */
#endif
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "}");
#endif
    }
} /* fScf */
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
    Gate *	out_list)
{
    int		diff;
    int		index;
    int		mask;
    int		val;
    int		count;
    Gate *	gs;

    count = 0;
    diff = gm->gt_new ^ gm->gt_old;
    assert (diff && !(diff & ~0xff));
    while (diff) {
	index = iC_bitIndex[diff];		/* returns 0 - 7 for values 1 - 255 */
	assert(index < 8);			/* ZZZ can be removed after initial testing */
	mask  = iC_bitMask[index];		/* returns hex 01 02 04 08 10 20 40 80 */
	if ((gs = gm->gt_list[index]) != 0) {	/* is bit Gate allocated ? */
	    val   = (gm->gt_new & mask) ? -1 : 1;	/* yes */
	    if (gs->gt_val != val) {
		gs->gt_val = val;
		iC_link_ol(gs, iC_o_list);	/* fire input bit Gate */
		count++;			/* count Gates linked */
	    }
	}
	diff &= ~mask;				/* clear the bit just processed */
    }
    gm->gt_old = gm->gt_new;
    if (gm->gt_live & 0x8000) {
	iC_liveData(gm->gt_live, gm->gt_old);	/* live is active */
    }
    return count;				/* used to adjust YYDEBUG output */
} /* traMb */

/********************************************************************
 *
 * NEW
 *	Output to a word or byte whose Channel index is in gt_list.
 *		gt_mark == B_WIDTH means 1 byte is output
 *		gt_mark == W_WIDTH means 2 bytes or 1 word is output
 *		gt_mark == L_WIDTH means 4 bytes or 1 long is output
 *
 *	For initialisation purposes this ftype OUTW node is acted on
 *	by exactly one ARITH node defined in an output assignment.
 *	This must line up with OUTP_M (1). This is checked in i_ff3().
 *		QB1 = b1;	// output b1 to byte on registered channel
 *		QW2 = w2;	// output w2 to word on registered channel
 *		QL4 = l4;	// output l4 to word on registered channel
 *	b1, w2  or l4 can be used as arithmetic values (ftype is ARITH)
 *	and may be any logical or arithmetic function, including INPUT.
 *
 *	An ftype OUTW node QXn is also acted on by all bit outputs QXn.m.
 *	These execute outMx(), which links its output directly to s_list.
 *	These extra nodes are generated at load time - not compiled.
 *
 *	NOTE: This action does not act on a clocked slave object.
 *	      It was linked to s_list, which is scanned only once when
 *	      all other lists have been scanned. This ensures each output
 *	      is only sent once to the external I/O's per cycle when it
 *	      really changes. Glitches do not appear in the output.
 *
 *		out_list 2nd argument	used to defer action to s_list
 *
 *******************************************************************/

void
iC_outMw(					/* NEW OUTW master action */
    Gate *	gm,				/* NOTE: there is no slave action */
    Gate *	out_list)
{
#if INT_MAX == 32767 && defined (LONG16)
    long	val;
#else
    int		val;
#endif
    int		mask;
    int		channel;
    int		rest;
    int		len;

    if (out_list == iC_a_list) {
	/* defer execution until s_list is scanned */
	if (
	    (gm->gt_old != gm->gt_new)		/* any change */
	    ^ (gm->gt_next != 0)		/* xor glitch */
	) {
	    iC_link_ol(gm, iC_s_list);		/* master action to send list */
	}
    } else {
	assert(out_list == iC_s_list);
	assert(gm->gt_new != gm->gt_old);

	channel = (int)gm->gt_list;		/* set up during registration */
	assert(channel > 0);			/* -1 is error, 0 is not registered */
	mask = gm->gt_mark;
	val = gm->gt_old = gm->gt_new;		/* update gt_old since no iC_link_ol */
	if (gm->gt_live & 0x8000) {
	    iC_liveData(gm->gt_live, val);	/* live is active */
	}
	if (mask == B_WIDTH) {
#ifndef _WINDOWS 
	    val &= 0xff;			/* for display only */
#endif
	    len = snprintf(&iC_outBuf[iC_outOffset], rest = REQUEST - iC_outOffset,
		"%d:%hhd,", channel, (char)val);	/* TODO overflow */
	    iC_outOffset += len;
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%d", val & 0xff);	/* byte */
#endif
	} else
	if (mask == W_WIDTH) {
#ifndef _WINDOWS 
	    val &= 0xffff;			/* for display only */
#endif
	    len = snprintf(&iC_outBuf[iC_outOffset], rest = REQUEST - iC_outOffset,
		"%d:%hd,", channel, (short)val);	/* TODO overflow */
	    iC_outOffset += len;
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%hd", (short)val);	/* word */
#endif
#if INT_MAX != 32767 || defined (LONG16)
	} else
	if (mask == L_WIDTH) {
#if INT_MAX == 32767
	    len = snprintf(&iC_outBuf[iC_outOffset], rest = REQUEST - iC_outOffset,
		"%d:%ld,", channel, (long)val);	/* TODO overflow */
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%ld", val);	/* long */
#endif
#else
	    len = snprintf(&iC_outBuf[iC_outOffset], rest = REQUEST - iC_outOffset,
		"%d:%d,", channel, val);	/* TODO overflow */
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%d", val);	/* long */
#endif
#endif
	    iC_outOffset += len;
#endif
	} else
	if (mask & X_MASK) {
#ifndef _WINDOWS 
	    val &= 0xff;			/* for display only */
#endif
	    len = snprintf(&iC_outBuf[iC_outOffset], rest = REQUEST - iC_outOffset,
		"%d:%d,", channel, val);	/* TODO overflow */
	    iC_outOffset += len;
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%d", val & 0xff);	/* byte */
#endif
#ifndef _WINDOWS
	} else {
	    val = 0;				/* error - no output */
#endif
	}
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
    Gate *	out_list)
{
    int		mask;
    int		val;
    Gate *	gs;

    gs = (Gate*)(gm->gt_list);			/* OUTW Gate */
    mask = gm->gt_mark;
    if (gm->gt_live & 0x8000) {
	iC_liveData(gm->gt_live, gm->gt_val < 0 ? 1 : 0);	/* live is active */
    }
    val = gs->gt_new;
    if (gm->gt_val < 0) {			/* output action */
	val |= mask;				/* set bit in byte Gate */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) putc('1', iC_outFP);
#endif
    } else {
	val &= ~mask;				/* clear bit in byte gate */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) putc('0', iC_outFP);
#endif
    }
    assert(val != gs->gt_new);			/* should only fire when there is a change */
    gs->gt_new = val;
    if (
	(gs->gt_old != gs->gt_new)		/* any change */
	^ (gs->gt_next != 0)			/* xor glitch */
    ) {
	iC_link_ol(gs, iC_s_list);		/* master action to send list */
    }
} /* outMx */
#else /* defined(TCP) || defined(LOAD) */

/********************************************************************
 *
 * OLD - from 1.37
 *	Output to a word or byte whose slot index is in gt_list.
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
	slot = (int)gm->gt_list;
	mask = gm->gt_mark;
	assert(slot < min(IXD, 64) && mask);	/* IXD must be <= 64 for this scheme */
	cage = slot >> 3;			/* test here because of cage algorithm */
#ifdef MIXED
	if (out_list == iC_o_list) {
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
	    if (iC_debug & 0100) fprintf(iC_outFP, "%d", val & 0xff);	/* byte */
#endif
	} else if (mask == W_WIDTH) {
#ifndef _WINDOWS
	    val &= 0xffff;			/* for display only */
#endif
	    assert((slot & 0x01) == 0);		/* even I/O word address */
	    *(short*)&iC_QX_[slot] = val;	/* output word to slot and slot+1 */
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%hd", (short)val);	/* word */
#endif
#if INT_MAX != 32767 || defined (LONG16)
	} else if (mask == L_WIDTH) {
	    assert((slot & 0x03) == 0);		/* 4 byte I/O long address */
#if INT_MAX == 32767
	    *(long*)&iC_QX_[slot] = val;	/* output long from slot to slot+3 */
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%ld", val);	/* long */
#endif
#else
	    *(int*)&iC_QX_[slot] = val;		/* output long from slot to slot+3 */
#if YYDEBUG && !defined(_WINDOWS)
	    if (iC_debug & 0100) fprintf(iC_outFP, "%d", val);	/* long */
#endif
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
 *	Output a bit to a bit field whose slot index is in gt_list.
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

    slot = (int)gm->gt_list;
    mask = (unsigned char)gm->gt_mark;
    assert(slot < min(IXD, 64) && mask);/* IXD must be <= 64 for this scheme */
    cage = slot >> 3;				/* test here because of cage algorithm */
#ifdef MIXED
    if (out_list == iC_a_list) {
	/* called from arithmetic scan - convert a to d */
	/* MIXED mode is currently not compiled - ERROR */
	gm->gt_val = gm->gt_new ? -1 : 1;
    }
#endif
    if (gm->gt_val < 0) {			/* output action */
	iC_QX_[slot] |= mask;			/* set bit at slot,mask */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) putc('1', iC_outFP);
#endif
    } else {
	iC_QX_[slot] &= ~mask;			/* clear bit at slot,mask */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) putc('0', iC_outFP);
#endif
    }
    iC_QM_[cage] |= iC_bitMask[slot & 0x7];	/* mark the cage */
    iC_QMM |= iC_bitMask[cage & 0x7];		/* mark the rack */
} /* outMx */
#endif /* defined(TCP) || defined(LOAD) */

/********************************************************************
 *
 *	Master function for CLOCK() and TIMER().
 *
 *******************************************************************/

void
iC_fMfn(					/* CLCK TIMR master action */
    Gate *	gm,
    Gate *	out_list)
{
#if defined(TCP) || defined(LOAD)
    if (gm->gt_live & 0x8000) {
	iC_liveData(gm->gt_live, gm->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    if ( gm->gt_val < 0 || gm->gt_next) {
	iC_link_ol(gm, gm->gt_clk);		/* master action */
    }
} /* fMfn */

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
 *	duration clock events, all actions dependent on a particular
 *	clock are executed, resulting in possible changes to outputs.
 *	It is of major importance that all outputs which do change as
 *	a result of clocked actions have all changed simultaneously
 *	during one clock event, as far as the next arithmetic and logic
 *	scan is concerned.
 *
 *		clock = CLOCK(I6);	// iClock is default
 *		clk2 =  C(I7,clock);	// C is synonym for CLOCK
 *
 *******************************************************************/

void
iC_clockSfn(					/* Clock function */
    Gate *	gm,
    Gate *	out_list)
{
    Gate *	gs;
#if YYDEBUG && !defined(_WINDOWS) || defined(DEQ)
    Gate *	tp;
#endif
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
	    fprintf(iC_outFP, "\tC %s", gs->gt_ids);
	}
#endif
    gs->gt_val = -1;				/* set for visualization only */
#if defined(TCP) || defined(LOAD)
    if (gs->gt_live & 0x8000) {
	iC_liveData(gs->gt_live, 1L);		/* live is active */
    }
#endif
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
#endif
#ifndef DEQ
	((Gate *)iC_c_list->gt_list)->gt_next = gs->gt_next;
	iC_c_list->gt_list = gs->gt_list; 	/* link gs */
	((Gate *)iC_c_list->gt_list)->gt_next = iC_c_list;    /* to c_list */
#else
	/* link chain of Gates on clock list gs to end of c_list */
	tp = iC_c_list->gt_prev;		/* save c_list.previous */
	tp->gt_next = np = gs->gt_next;		/* c_list.last ==> new */
	np->gt_prev = tp;			/* c_list.last <== new */
	tp = gs->gt_prev;			/* save gs.previous */
	tp->gt_next = iC_c_list;		/* gs.last ==> c_list */
	iC_c_list->gt_prev = tp;		/* gs.last <== c_list */
#endif
	Out_init(gs);				/* relink empty gs */
    }
} /* clockSfn */

iC_Functp	iC_clock_i[] = {iC_pass1, iC_null1, i_ff3, iC_null1};	/* no output lists */

/********************************************************************
 *
 *	Timer driver function.
 *
 *	The output of this function can be used wherever a clock
 *	may be used. For details see manual.
 *
 *		timer = TIMER(I8);	// iClock is default
 *		tim2 =  T(I9,clock);	// T is synonym for TIMER
 *
 *******************************************************************/

void
iC_timerSfn(					/* Timer function */
    Gate *	gm,
    Gate *	out_list)
{
    Gate *	gs;
    Gate *	tp;				/* functions which are timed */
    Gate *	np;
    /********************************************************************
     * the timer list is controlled by a control block which is the slave
     * Gate structure which goes with this function
     *******************************************************************/
    assert(gm->gt_val < 0);			/* T timer receives -1 ==> 1 ??? */
    gs = gm->gt_funct;
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_debug & 0100) {
	fprintf(iC_outFP, "\tT %s", gs->gt_ids);
    }
#endif
    gs->gt_val = -1;				/* set for visualization only */
#if defined(TCP) || defined(LOAD)
    if (gs->gt_live & 0x8000) {
	iC_liveData(gs->gt_live, 1L);		/* live is active */
    }
#endif
    if ((np = gs->gt_next) != gs) {		/* skip if this clock list empty */
	np->gt_mark--;				/* count down first element */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    iC_dc = 1;				/* allow for (time) */
	    fprintf(iC_outFP, "\t(%d)", np->gt_mark);
	}
#endif
	if (np->gt_mark == 0) {
#ifdef DEQ
	    /* link head of timer chain gs to end of c_list */
	    tp = iC_c_list->gt_prev;		/* save c_list.previous */
	    tp->gt_next = np;			/* c_list.last ==> new */
	    np->gt_prev = tp;			/* c_list.last <== new */
#endif
	    do {
		tp = np;
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) {
		    if (iC_dc++ >= 8) {
			iC_dc = 1;
			fprintf(iC_outFP, "\n\t");
		    }
		    fprintf(iC_outFP, "\t%s", tp->gt_ids);
		}
#endif
	    } while ((np = tp->gt_next) != gs && np->gt_mark == 0);
#ifndef DEQ
	    ((Gate *)iC_c_list->gt_list)->gt_next = gs->gt_next;/* => new */
	    gs->gt_next = np;			/* timer => rest */
	    iC_c_list->gt_list = (Gate **)tp;	/* clock last => new last */
	    tp->gt_next = iC_c_list;    	/* new last => c_list */
	    if (np == gs) {			/* last entry ? */
		gs->gt_list = (Gate **)np;	/*  yes, fix timer last */
	    }
#else
	    /* link tail of timer chain which is due to c_list */
	    tp->gt_next = iC_c_list;		/* gs.last ==> c_list */
	    iC_c_list->gt_prev = tp;		/* gs.last <== c_list */
	    /* the re-linking of gs works correctly also if np == gs */
	    gs->gt_next = np;			/* gs ==> new new */
	    np->gt_prev = gs;			/* gs <== new new */
#endif
	}
    }
} /* timerSfn */

/********************************************************************
 *
 *	Assign to an imm int node of type ARNC
 *
 *	The node is linked to a_list when rv != new && new == old.
 *	If a second assignment occurs while node is still linked to
 *	a_list (new != old), if rv != old new is simply updated
 *	else node is unlinked and new updated, making new == old.
 *	When rv == new, there is no change at all.
 *
 *******************************************************************/

#if INT_MAX == 32767 && defined (LONG16)
long
iC_assignA(Gate * gm, long rv)
#else
int
iC_assignA(Gate * gm, int rv)
#endif
{
    assert(gm->gt_ini == -ARNC);
    if (rv != gm->gt_new) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
#if INT_MAX == 32767 && defined (LONG16)
	    fprintf(iC_outFP, "\tAA %s %ld ==>", gm->gt_ids, gm->gt_new);
#else
	    fprintf(iC_outFP, "\tAA %s %d ==>", gm->gt_ids, gm->gt_new);
#endif
	}
#endif
	if (gm->gt_new == gm->gt_old || rv == gm->gt_old) {
	    iC_link_ol(gm, iC_a_list);	/* arithmetic change or glitch */
	}
	gm->gt_new = rv;			/* first or later change */
#if YYDEBUG && !defined(_WINDOWS)
#if INT_MAX == 32767 && defined (LONG16)
	if (iC_debug & 0100) fprintf(iC_outFP, " %ld\n", gm->gt_new);
#else
	if (iC_debug & 0100) fprintf(iC_outFP, " %d\n", gm->gt_new);
#endif
#endif
    }
    return rv;
} /* iC_assignA */

/********************************************************************
 *
 *	Assign to an imm bit node of type LOGC
 *
 *	The node is linked to o_list when rv, interpreted as a logic
 *	value (0 is rv == 0, 1 is rv != 0) changes from its previous
 *	value.
 *
 *******************************************************************/

#if INT_MAX == 32767 && defined (LONG16)
long
iC_assignL(Gate * gm, long rv)
#else
int
iC_assignL(Gate * gm, int rv)
#endif
{
    char val;

    assert(gm->gt_ini == -LOGC);
    val = rv ? -1 : 1;
    if (gm->gt_val != val) {
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, "\tLA %s %2d ==>", gm->gt_ids, gm->gt_val);
	}
#endif
	gm->gt_val = val;
	iC_link_ol(gm, iC_o_list);		/* logic change or glitch */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) {
	    fprintf(iC_outFP, " %d\n", gm->gt_val);
	}
#endif
    }
    return rv != 0;				/* change to logic value 0 or 1 */
} /* iC_assignL */

static const char rsff_c[] =
"@(#)$Id: rsff.c,v 1.23 2001/04/04 18:20:55 jw Exp $";
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
 *	rsff.c
 *	parallel plc - SR flip flop and other clocked functions
 *
 *******************************************************************/

/* J.E. Wulff	8-Mar-85 */

#include	<stdio.h>
#include	<stdlib.h>
#include	<assert.h>
#include	"icc.h"

char *		full_type[]  = { FULL_TYPE };
char *		full_ftype[] = { FULL_FTYPE };
unsigned char	types[]      = { TYPES };
unsigned char	ftypes[]     = { FTYPES };
char		os[]         = OPS;
char		fos[]        = FOPS;

unsigned char	bitMask[]    = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,	/* 0 1 2 3 4 5 6 7 */
};

unsigned char	bitIndex[]   = {
    0, 0, 1, 0, 2, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0,	/* 0x01 0x02 0x04 0x08 */
    4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,	/* 0x10 */
    5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 	/* 0x20 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
    6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 	/* 0x40 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 	/* 0x80 */
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
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
 *******************************************************************/

void
sMff(					/* S_FF master action on FF */
    Gate *	gp,
    Gate *	out_list)
{
    Gate **	fa;

#if defined(TCP) && defined(LOAD)
    if (gp->gt_live & 0x8000) {
	liveData(gp->gt_live, gp->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    if (
	(fa = gp->gt_list)[FL_GATE]->gt_val > 0
	&& gp->gt_val < 0		/* anything to set */
	|| gp->gt_next
    ) {
	link_ol(gp, fa[FL_CLK]);	/* master action */
    }
} /* sMff */

void
sSff(					/* S_FF slave action on FF */
    Gate *	gf,
    Gate *	out_list)
{
    Gate *	gp;

    if (gf->gt_val < 0) {
	gp = gf->gt_funct;
	if (gp->gt_val > 0) {
#ifndef _WINDOWS 
	    if (debug & 0100) {
		fprintf(outFP, "\tS %s %2d ==>", gp->gt_ids, gp->gt_val);
	    }
#endif
	    gp->gt_val = -1;		/* set slave output */
	    link_ol(gp, o_list);
#ifndef _WINDOWS 
	    if (debug & 0100) {
		fprintf(outFP, " %d", gp->gt_val);
	    }
#endif
	}
#if !defined(_WINDOWS) || defined(LOAD)
    } else {
	fprintf(errFP,
	    "\n%s: line %d: S FF %s receives -1 ==> 1 ???\n",
	    __FILE__, __LINE__, gp->gt_ids);
	quit(-1);
#endif
    }
} /* sSff */

void
rMff(					/* R_FF master action on FF */
    Gate *	gp,
    Gate *	out_list)
{
    Gate **	fa;

#if defined(TCP) && defined(LOAD)
    if (gp->gt_live & 0x8000) {
	liveData(gp->gt_live, gp->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    if (
	(fa = gp->gt_list)[FL_GATE]->gt_val < 0
	&& gp->gt_val < 0		/* anything to reset */
	|| gp->gt_next
    ) {
	link_ol(gp, fa[FL_CLK]);	/* master action */
    }
} /* rMff */

void
rSff(					/* R_FF slave action on FF */
    Gate *	gf,
    Gate *	out_list)
{
    Gate *	gp;

    if (gf->gt_val < 0) {
	gp = gf->gt_funct;
	if (gp->gt_val < 0) {
#ifndef _WINDOWS 
	    if (debug & 0100) {
		fprintf(outFP, "\tR %s %2d ==>", gp->gt_ids, gp->gt_val);
	    }
#endif
	    gp->gt_val = 1;		/* reset slave output */
	    link_ol(gp, o_list);
#ifndef _WINDOWS 
	    if (debug & 0100) {
		fprintf(outFP, " %d", gp->gt_val);
	    }
#endif
	}
#if !defined(_WINDOWS) || defined(LOAD)
    } else {
	fprintf(errFP,
	    "\n%s: line %d: R FF %s receives -1 ==> 1 ???\n",
	    __FILE__, __LINE__, gp->gt_ids);
	quit(-1);
#endif
    }
} /* rSff */

void
dMff(					/* D_FF master action on FF */
    Gate *	gp,
    Gate *	out_list)
{
    Gate **	fa;

#if defined(TCP) && defined(LOAD)
    if (gp->gt_live & 0x8000) {
	liveData(gp->gt_live, gp->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    if (
	((fa = gp->gt_list)[FL_GATE]->gt_val < 0)
	^ (gp->gt_val < 0)		/* any change */
	^ (gp->gt_next != 0)
    ) {
	link_ol(gp, fa[FL_CLK]);	/* master action */
    }
} /* dMff */

void
dSff(					/* D_FF slave action on FF */
    Gate *	gf,
    Gate *	out_list)
{
    Gate *	gp;
    short	val;

    gp = gf->gt_funct;
    if ((val = (gf->gt_val < 0) ? -1 : 1) != gp->gt_val) {
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, "\tD %s %2d ==>", gp->gt_ids, gp->gt_val);
	}
#endif
	gp->gt_val = val;		/* transfer val to slave */
	link_ol(gp, o_list);
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, " %d", gp->gt_val);
	}
#endif
    }
} /* dSff */

/********************************************************************
 *
 *	Sample/hold function with D(arithmetic expr)
 *
 *******************************************************************/

void
dMsh(					/* D_SH master action on SH */
    Gate *	gp,
    Gate *	out_list)
{
    if (out_list == o_list) {
	/* called from logic scan - convert d to a */
	gp->gt_new = gp->gt_val < 0 ? 1 : 0;
	/*
	 * since new is only modified here and since gt_val has changed,
	 * new must differ from old (no need to check).
	 * old will be modified during arithmetic scan, although that
	 * would not be necessary for this action.
	 */
    }
#if defined(TCP) && defined(LOAD)
    if (gp->gt_live & 0x8000) {
	liveData(gp->gt_live, gp->gt_new);	/* live is active */
    }
#endif
    link_ol(gp, gp->gt_clk);		/* master action */
} /* dMsh */

void
dSsh(					/* D_SH slave action on SH */
    Gate *	gf,
    Gate *	out_list)
{
    Gate *	gp;
    short	val;

    gp = gf->gt_funct;
#ifndef _WINDOWS 
    if (debug & 0100) {
	fprintf(outFP, "\tSH %s %d ==>", gp->gt_ids, gp->gt_new);
    }
#endif
    /* since gf is still on clock list gt_new must differ from gt_old */
    gp->gt_new = gf->gt_old = gf->gt_new; /* transfer value to slave */
    link_ol(gp, a_list);	/* fire new arithmetic action */
#ifndef _WINDOWS 
    if (debug & 0100) fprintf(outFP, " %d", gp->gt_new);
#endif
} /* dSsh */

/********************************************************************
 *
 *	Pass 2 initialisation for clocked Gates via ftype S_FF - TIMR
 *	except F_SW and F_CF in pass2().
 *
 *	Each action Gate ors a bit reserved for its action in gt_val
 *	of the function Gate on which it acts. If ONCE_M is set, that
 *	action may only occurr once.
 *
 *	In Pass 3 every function Gate will contain a bit for every
 *	action which acts on that function.
 *
 *******************************************************************/

void
i_ff2(Gate * op, int typ)		/* called via output lists */
{
    Gate *		gp;
    unsigned char	mask;		/* action bit mask */

    if ((gp = op->gt_funct) != 0) {
	if (gp->gt_mcnt & ONCE_M & (mask = bit2[op->gt_fni])) {
	    fprintf(outFP,
	"\nError:    %c\t%s\thas %c input action more than once (%.4x:%.4x)",
		os[types[op->gt_fni]], gp->gt_ids,
		fos[op->gt_fni], gp->gt_mcnt, mask);
	    error_flag = 1;		/* cannot execute with this error */
	}
	gp->gt_mcnt |= mask;		/* allow other multiple inputs */
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
i_ff3(Gate * gp, int typ)		/* Pass3 init on FF etc. */
{
    unsigned char	mask;		/* action bit mask */
    char		opt;

    if (gp != c_list) {			/* iClock has no inputs */
	mask = bit2[ftypes[typ]] & ~ONCE_M;/* action bit mask required */
	opt = os[typ];
	/* accept only the right number of compatible inputs */
	if ((gp->gt_mcnt & ~ONCE_M) != mask) {
	    fprintf(outFP,
	    "\nError:    %c	%s\thas %s inputs (%.4x:%.4x)",
		opt, gp->gt_ids, gp->gt_mcnt ? "incompatible" : "no",
		gp->gt_mcnt, mask);
	    error_flag = 1;		/* cannot execute with this error */
#ifndef _WINDOWS 
	} else if (debug & 0100) {
	    fprintf(outFP, "\n	    %c	%s:\t%.4x inputs",
		opt, gp->gt_ids, gp->gt_mcnt);
#endif
	}
    }
    if (typ < MAX_LV) {
	gp->gt_val = 1;		/* set fblk gates to +1 anyway */
	if (typ == SH || typ == INPW) {
	    gp->gt_new = gp->gt_old = 0;	/* clear arithmetic */
	} else if (typ == NCONST) {
	    gp->gt_new = gp->gt_old = atoi(gp->gt_ids);	/* constant */
	}
    } else {
	gp->gt_val = 0;		/* used in visualization */
	Out_init(gp);		/* link as clock or timer list */
	if (gp != c_list) {
	    gp->gt_rlist = (Gate**)c_list;	/* c_list is alternate for link */
	}				/* except c_list which has 0 */
    }
    if (gp->gt_fni != OUTW && gp->gt_fni != OUTX) {
	gp->gt_mark = 0;	/* clear for stamp and diff in actions */
    }
} /* i_ff3 */

Functp	ff_i[] = {pass1, pass2, i_ff3, pass4};

/********************************************************************
 *
 *	Master function to detect RISE in logical input.
 *		O1 = RISE(I1,clock);	// clock has been defined
 *		O2 = RISE(I2);		// iClock is default
 *	This function goes HI when its input goes HI (rising edge).
 *	The function goes LO with the next clock in the slave action.
 *
 *******************************************************************/

void
riMbit(					/* RI_BIT master action on EF */
    Gate *	gp,
    Gate *	out_list)
{
    Gate **	fa;

#if defined(TCP) && defined(LOAD)
    if (gp->gt_live & 0x8000) {
	liveData(gp->gt_live, gp->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    gx = (fa = gp->gt_list)[FL_GATE];
    if (gp->gt_val < 0 || gp->gt_next) {
	link_ol(gp, fa[FL_CLK]);	/* master action */
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, " %d", gp->gt_val);
	    if (dc++ >= 4) {
		dc = 1;
		putc('\n', outFP);
	    }
	    fprintf(outFP, "\t%s %2d E=>",
		gx->gt_ids, gx->gt_val);
	}
#endif
	gx->gt_val = -gx->gt_val;	/* immediate (glitch could reset here) */
	link_ol(gx, o_list);		/* set or reset slave output */
    }
} /* riMbit */

/********************************************************************
 *
 *	Slave function for RISE in logical input.
 *	The function goes LO again here
 *
 *******************************************************************/

void
riSbit(					/* RI_BIT slave action */
    Gate *	gf,
    Gate *	out_list)
{
    Gate *	gp;

    gp = gf->gt_funct;
    if (gp->gt_val < 0) {
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, "\tE %s %2d ==>", gp->gt_ids, gp->gt_val);
	}
#endif
	gp->gt_val = 1;			/* reset slave output to LO */
	link_ol(gp, o_list);
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, " %d", gp->gt_val);
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
 *******************************************************************/

void
chMbit(					/* CH_BIT master action on VF */
    Gate *	gp,
    Gate *	out_list)
{
    Gate **	fa;

#if defined(TCP) && defined(LOAD)
    if (gp->gt_live & 0x8000) {
	liveData(gp->gt_live,(out_list != o_list) ?
	gp->gt_new : gp->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    gx = (fa = gp->gt_list)[FL_GATE];	/* ignore input */
    link_ol(gp, fa[FL_CLK]);		/* master action */
#ifndef _WINDOWS 
    if (debug & 0100) {
	fprintf(outFP, " %d", out_list != o_list? gp->gt_new : gp->gt_val);
	if (dc++ >= 4) {
	    dc = 1;
	    putc('\n', outFP);
	}
	fprintf(outFP, "\t%s %2d V=>",
	    gx->gt_ids, gx->gt_val);
    }
#endif
    gx->gt_val = -gx->gt_val;		/* immediate (glitch could reset here) */
    link_ol(gx, o_list);		/* set or reset slave output */
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
chSbit(					/* CH_BIT slave action */
    Gate *	gf,
    Gate *	out_list)
{
    Gate *	gp;

    gf->gt_old = gf->gt_new;		/* now new value is fixed */
    gp = gf->gt_funct;
    if (gp->gt_val < 0) {
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, "\tV %s %2d ==>", gp->gt_ids, gp->gt_val);
	}
#endif
	gp->gt_val = 1;			/* reset slave output to LO */
	link_ol(gp, o_list);
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, " %d", gp->gt_val);
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
fMsw(					/* F_SW master action */
    Gate *	gp,
    Gate *	out_list)
{
    if (out_list == o_list) {
	/* called from logic scan - convert d to a */
	gp->gt_new = gp->gt_val < 0 ? 1 : 0;
    }
#if defined(TCP) && defined(LOAD)
    if (gp->gt_live & 0x8000) {
	liveData(gp->gt_live, gp->gt_new);	/* live is active */
    }
#endif
    link_ol(gp, gp->gt_clk);		/* master action */
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
fSsw(					/* F_SW slave action on SW */
    Gate *	gf,
    Gate *	out_list)
{
    gf->gt_old = gf->gt_new;		/* now new value is fixed */
    /* execute C function as action procedure with side effects */
#ifdef LOAD
#ifndef _WINDOWS 
    if (debug & 0100) fprintf(outFP, "\tF%p(", (CFunctp)gf->gt_funct);
#endif
    ((CFunctp)(gf->gt_funct))(gf);
#else
#ifndef _WINDOWS 
    if (debug & 0100) fprintf(outFP, "\tF%d(", (int)gf->gt_funct);
#endif
    c_exec((int)gf->gt_funct, gf);	/* must pass both -/+ */
#endif
#ifndef _WINDOWS 
    if (debug & 0100) fprintf(outFP, ")\n");
#endif
} /* fSsw */

/********************************************************************
 *
 *	Master function for IF() IF() ELSE.
 *
 *******************************************************************/

void
fMcf(					/* F_CF master action */
    Gate *	gp,
    Gate *	out_list)
{
#if defined(TCP) && defined(LOAD)
    if (gp->gt_live & 0x8000) {
	liveData(gp->gt_live, gp->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    link_ol(gp, gp->gt_clk);		/* master action */
} /* fMcf */

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
fScf(					/* F_CF slave action on CF */
    Gate *	gf,
    Gate *	out_list)
{
    /* execute C function as action procedure with side effects */
#ifdef LOAD
#ifndef _WINDOWS 
    if (debug & 0100) fprintf(outFP, "\tF%p{", (CFunctp)gf->gt_funct);
#endif
    ((CFunctp)(gf->gt_funct))(gf);
#else
#ifndef _WINDOWS 
    if (debug & 0100) fprintf(outFP, "\tF%d{", (int)gf->gt_funct);
#endif
    c_exec((int)gf->gt_funct, gf);	/* must pass both -/+ */
#endif
#ifndef _WINDOWS 
    if (debug & 0100) fprintf(outFP, "}\n");
#endif
} /* fScf */

/********************************************************************
 *
 *	Output to a word or byte whose slot index is in gt_list.
 *		gt_mark == B_WIDTH means 1 byte is output
 *		gt_mark == W_WIDTH means 2 bytes or 1 word is output
 *
 *	For initialisation purposes this ftype OUTW node is acted on
 *	by exactly one ARITH node defined in a output assignment.
 *	This must line up with OUTP_M (1). This is checked in i_ff3().
 *		QB1 = b1;	// output b1 to byte at QX_[1]
 *		QW2 = w2;	// output w2 to word at QX_[2]
 *	b1  or w2 can be used as arithmetic values (ftype is ARITH)
 *	and may be any logical or arithmetic function, including INPUT.
 *
 *	NOTE: This action does not act on a clocked slave object.
 *	      and the node is not linked to any action or clock list
 *
 *******************************************************************/

void
outMw(					/* OUTW master action */
    Gate *	gp,		/* NOTE: there is no slave action */
    Gate *	out_list)
{
    int		val;
    int		slot;
    int		cage;
    int		mask;

    slot = (int)gp->gt_list;
    mask = gp->gt_mark;
    assert(slot < IXD && mask);
    cage = slot >> 3;			/* IXD must be <= 64 for this scheme */
#ifdef MIXED
    if (out_list == o_list) {
	/* called from logic scan - convert d to a */
	/* MIXED mode is currently not compiled - ERROR */
	gp->gt_new = gp->gt_val < 0 ? 1 : 0;
    }
#endif

    val = gp->gt_old = gp->gt_new;	/* update gt_old since no link_ol */
#if defined(TCP) && defined(LOAD)
    if (gp->gt_live & 0x8000) {
	liveData(gp->gt_live, val);	/* live is active */
    }
#endif
    if (mask == B_WIDTH) {
#ifndef _WINDOWS
	val &= 0xff;			/* for display only */
#endif
	QX_[slot] = val;		/* output byte to slot */
	QM_[cage] |= bitMask[slot & 0x7];	/* mark the cage */
	QMM |= bitMask[cage & 0x7];		/* mark the rack */
    } else if (mask == W_WIDTH) {
	*(short*)&QX_[slot] = val;	/* output word to slot and slot+1 */
	QM_[cage] |= bitMask[slot & 0x7];	/* mark the cage */
	QMM |= bitMask[cage & 0x7];		/* mark the rack */
#ifndef _WINDOWS
    } else {
	val = 0;			/* error - no output */
#endif
    }
#ifndef _WINDOWS 
	if (debug & 0100) fprintf(outFP, "%d", val);	/* byte or word */
#endif
} /* outMw */

/********************************************************************
 *
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
outMx(					/* OUTX master action */
    Gate *	gp,		/* NOTE: there is no slave action */
    Gate *	out_list)
{
    int			slot;
    int			cage;
    unsigned char	mask;

    slot = (int)gp->gt_list;
    mask = (unsigned char)gp->gt_mark;
    assert(slot < IXD && mask);
    cage = slot >> 3;			/* IXD must be <= 64 for this scheme */
#ifdef MIXED
    if (out_list == a_list) {
	/* called from arithmetic scan - convert a to d */
	/* MIXED mode is currently not compiled - ERROR */
	gp->gt_val = gp->gt_new ? -1 : 1;
    }
#endif
#if defined(TCP) && defined(LOAD)
    if (gp->gt_live & 0x8000) {
	liveData(gp->gt_live, gp->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    if (gp->gt_val < 0) {		/* output action */
	QX_[slot] |= mask;		/* set bit at slot,mask */
#ifndef _WINDOWS 
	if (debug & 0100) putc('1', outFP);
#endif
    } else {
	QX_[slot] &= ~mask;		/* clear bit at slot,mask */
#ifndef _WINDOWS 
	if (debug & 0100) putc('0', outFP);
#endif
    }
    QM_[cage] |= bitMask[slot & 0x7];	/* mark the cage */
    QMM |= bitMask[cage & 0x7];		/* mark the rack */
} /* outMx */

/********************************************************************
 *
 *	Master function for CLOCK() and TIMER().
 *
 *******************************************************************/

void
fMfn(					/* CLCK TIMR master action */
    Gate *	gp,
    Gate *	out_list)
{
#if defined(TCP) && defined(LOAD)
    if (gp->gt_live & 0x8000) {
	liveData(gp->gt_live, gp->gt_val < 0 ? 1 : 0);	/* live is active */
    }
#endif
    if ( gp->gt_val < 0 || gp->gt_next) {
	link_ol(gp, gp->gt_clk);	/* master action */
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
clockSfn(				/* Clock function */
    Gate *	gf,
    Gate *	out_list)
{
    Gate *	gp;
    /*
     * the clock list is controlled by a control block which is the
     * Gate structure which goes with this function
     */
    if (gf->gt_val < 0) {
	Gate *	tp;
	Gate *	np;

	gp = gf->gt_funct;
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, "\tC %s", gp->gt_ids);
	}
#endif
	gp->gt_val = -1;		/* set for visualization only */
#if defined(TCP) && defined(LOAD)
	if (gp->gt_live & 0x8000) {
	    liveData(gp->gt_live, 1);	/* live is active */
	}
#endif
	if (gp->gt_next != gp) {
#ifndef _WINDOWS 
	    if (debug & 0100) {
		int	dc = 0;		/* functions which are clocked */
		for (tp = gp->gt_next; tp != gp; tp = tp->gt_next) {
		    if (dc++ >= 8) {
			dc = 1;
			fprintf(outFP, "\n\t");
		    }
		    fprintf(outFP, "\t%s", tp->gt_ids);
		}
	    }
#endif
#ifndef DEQ
	    ((Gate *)c_list->gt_list)->gt_next = gp->gt_next;
	    c_list->gt_list = gp->gt_list;  	/* link gp */
	    ((Gate *)c_list->gt_list)->gt_next = c_list;    /* to c_list */
#else
	    /* link chain of Gates on clock list gp to end of c_list */
	    tp = c_list->gt_prev;		/* save c_list.previous */
	    tp->gt_next = np = gp->gt_next;	/* c_list.last ==> new */
	    np->gt_prev = tp;			/* c_list.last <== new */
	    tp = gp->gt_prev;			/* save gp.previous */
	    tp->gt_next = c_list;		/* gp.last ==> c_list */
	    c_list->gt_prev = tp;		/* gp.last <== c_list */
#endif
	    Out_init(gp);			/* relink empty gp */
	}
#if !defined(_WINDOWS) || defined(LOAD)
#ifdef LEVEL
	if ((gp = gf->gt_clk) != c_list) {
	    link_ol(gf, gp);
	}
#endif
    } else {
	fprintf(errFP,
	    "\n%s: line %d: C clock %s receives -1 ==> 1 ???\n",
	    __FILE__, __LINE__, gf->gt_funct->gt_ids);
	quit(-1);
#endif
    }
} /* clockSfn */

Functp	clock_i[] = {pass1, null1, i_ff3, null1};	/* no output lists */

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
timerSfn(				/* Timer function */
    Gate *	gf,
    Gate *	out_list)
{
    Gate *	gp;
    /*
     * the timer list is controlled by a control block which is the
     * Gate structure which goes with this function
     */
    if (gf->gt_val < 0) {
	Gate *	tp;			/* functions which are timed */
	Gate *	np;

	gp = gf->gt_funct;
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, "\tT %s", gp->gt_ids);
	}
#endif
	gp->gt_val = -1;		/* set for visualization only */
#if defined(TCP) && defined(LOAD)
	if (gp->gt_live & 0x8000) {
	    liveData(gp->gt_live, 1);	/* live is active */
	}
#endif
	if ((np = gp->gt_next) != gp) {
	    int	dc = 1;	/* allow for (time) */

	    np->gt_mark--;		/* count down first element */
#ifndef _WINDOWS 
	    if (debug & 0100) {
		fprintf(outFP, "\t(%d)", np->gt_mark);
	    }
#endif
	    if (np->gt_mark == 0) {
#ifdef DEQ
		/* link head of timer chain gp to end of c_list */
		tp = c_list->gt_prev;		/* save c_list.previous */
		tp->gt_next = np;		/* c_list.last ==> new */
		np->gt_prev = tp;		/* c_list.last <== new */
#endif
		do {
		    tp = np;
#ifndef _WINDOWS 
		    if (debug & 0100) {
			if (dc++ >= 8) {
			    dc = 1;
			    fprintf(outFP, "\n\t");
			}
			fprintf(outFP, "\t%s", tp->gt_ids);
		    }
#endif
		} while ((np = tp->gt_next) != gp && np->gt_mark == 0);
#ifndef DEQ
		((Gate *)c_list->gt_list)->gt_next = gp->gt_next;/* => new */
		gp->gt_next = np;		/* timer => rest */
		c_list->gt_list = (Gate **)tp;	/* clock last => new last */
		tp->gt_next = c_list;    	/* new last => c_list */
		if (np == gp) {			/* last entry ? */
		    gp->gt_list = (Gate **)np;	/*  yes, fix timer last */
		}
#else
		/* link tail of timer chain which is due to c_list */
		tp->gt_next = c_list;		/* gp.last ==> c_list */
		c_list->gt_prev = tp;		/* gp.last <== c_list */
		/* the re-linking of gp works correctly also if np == gp */
		gp->gt_next = np;		/* gp ==> new new */
		np->gt_prev = gp;		/* gp <== new new */
#endif
	    }
	}
#if !defined(_WINDOWS) || defined(LOAD)
    } else {
	fprintf(errFP,
	    "\n%s: line %d: T timer %s receives -1 ==> 1 ???\n",
	    __FILE__, __LINE__, gf->gt_funct->gt_ids);
	quit(-1);
#endif
    }
} /* timerSfn */

/********************************************************************
 *
 *	Assign to an imm int node
 *
 *	The node is linked to a_list when rv != new && new == old.
 *	If a second assignment occurs while node is still linked to
 *	a_list (new != old), if rv != old new is simply updated
 *	else node is unlinked and new updated, making new == old.
 *	When rv == new, there is no change at all.
 *
 *******************************************************************/

int
assign(Gate * lv, int rv)
{
    if (rv != lv->gt_new) {
	if (lv->gt_new == lv->gt_old || rv == lv->gt_old) {
	    link_ol(lv, a_list);	/* first change or glitch */
	}
	lv->gt_new = rv;		/* first or later change */
    }
    return rv;
} /* assign */

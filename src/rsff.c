static const char rsff_c[] =
"@(#)$Id: rsff.c,v 1.3 1999/08/02 21:26:19 jw Exp $";
/* RS flip flop function */

/* J.E. Wulff	8-Mar-85 */

/* "rsff.c	3.32	95/02/11" */

#include	<stdio.h>
#include	"pplc.h"

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
    register Gate *	gp,
    Gate *		out_list)
{
    register Gate **	fa;

    if (
	(fa = gp->gt_list)[FL_GATE]->gt_val > 0
	&& gp->gt_val < 0		/* anything to set */
	|| gp->gt_next
    ) {
	link_ol(gp, fa[FL_CLK]);	/* master action */
    }
} /* sMff */

void
s_ff(					/* S_FF slave action on FF */
    register Gate *	gf,
    Gate *		out_list)
{
    register Gate *	gp;

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
} /* s_ff */

void
rMff(					/* R_FF master action on FF */
    register Gate *	gp,
    Gate *		out_list)
{
    register Gate **	fa;

    if (
	(fa = gp->gt_list)[FL_GATE]->gt_val < 0
	&& gp->gt_val < 0		/* anything to reset */
	|| gp->gt_next
    ) {
	link_ol(gp, fa[FL_CLK]);	/* master action */
    }
} /* rMff */

void
r_ff(					/* R_FF slave action on FF */
    register Gate *	gf,
    Gate *		out_list)
{
    register Gate *	gp;

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
} /* r_ff */

void
dMff(					/* D_FF master action on FF */
    register Gate *	gp,
    Gate *		out_list)
{
    register Gate **	fa;

    if (
	((fa = gp->gt_list)[FL_GATE]->gt_val < 0)
	^ (gp->gt_val < 0)		/* any change */
	^ (gp->gt_next != 0)
    ) {
	link_ol(gp, fa[FL_CLK]);	/* master action */
    }
} /* dMff */

void
d_ff(					/* D_FF slave action on FF */
    register Gate *	gf,
    Gate *		out_list)
{
    register Gate *	gp;
    register short	val;

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
} /* d_ff */

/********************************************************************
 *
 *	Sample/hold function with D(arithmetic expr)
 *
 *******************************************************************/

void
dMsh(					/* D_SH master action on SH */
    register Gate *	gp,
    Gate *		out_list)
{
    if (out_list == o_list) {
	/* called from logic scan - convert d to a */
	gp->gt_new = (int)((unsigned char)gp->gt_val >> 7);
	/*
	 * since new is only modified here and since gt_val has changed,
	 * new must differ from old (no need to check).
	 * old will be modified during arithmetic scan, although that
	 * would not be necessary for this action.
	 */
    }
    link_ol(gp, gp->gt_clk);	/* master action */
} /* dMsh */

void
d_sh(					/* D_SH slave action on SH */
    register Gate *	gf,
    Gate *		out_list)
{
    register Gate *	gp;
    register short	val;

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
} /* d_sh */

/********************************************************************
 *
 *	Pass 2 initialisation for clocked Gates via ftype S_FF - TIMR
 *	except F_CF in pass2().
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
i_ff2(register Gate * op, int typ)	/* called via output lists */
{
    register Gate *	gp;
    uchar		mask;		/* action bit mask */

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
i_ff3(register Gate * gp, int typ)	/* Pass3 init on FF etc. */
{
    uchar		mask;		/* action bit mask */
    char		opt;

    if (gp != c_list) {
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
    if (typ < MAX_CK) {
	gp->gt_val = 1;		/* set fblk gates to +1 anyway */
	if (typ == SH || typ == INPW) {
	    gp->gt_new = gp->gt_old = 0;	/* clear arithmetic */
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
 *	Special function to detect a rising edge in the logic input
 *		O1 = RISE(I1,clock);	// clock has been defined
 *		O2 = RISE(I2);		// iClock is default
 *	This function goes HI when its input goes HI and goes LO
 *	again with the next clock.
 *
 *******************************************************************/

void
riMbit(					/* RI_BIT master action on EF */
    register Gate *	gp,
    Gate *		out_list)
{
    register Gate **	fa;

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
	gx->gt_val = -gx->gt_val;	/* immediate */
	link_ol(gx, o_list);		/* set slave output */
    }
} /* riMbit */

void
ri_bit(					/* RI_BIT slave action */
    register Gate *	gf,
    register Gate *	out_list)
{
    register Gate *	gp;

    gp = gf->gt_funct;
    if (gp->gt_val < 0) {
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, "\tE %s %2d ==>", gp->gt_ids, gp->gt_val);
	}
#endif
	gp->gt_val = 1;			/* reset slave output */
	link_ol(gp, o_list);
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, " %d", gp->gt_val);
	}
	/* on startup a reset action comes when EF is LO */
#endif
    }
} /* ri_bit */

/********************************************************************
 *
 *	Special function to detect a change in the logic input
 *		O3 = CHANGE(I4,clock);	// clock has been defined
 *		O4 = CHANGE(I4);	// iClock is default
 *	This function goes HI when its input changes from HI to LO or
 *	from LO to HI. The function goes LO again with the next clock.
 *
 *******************************************************************/

void
chMbit(					/* CH_BIT master action on VF */
    register Gate *	gp,
    Gate *		out_list)
{
    register Gate **	fa;

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
    gx->gt_val = -gx->gt_val;		/* immediate */
    link_ol(gx, o_list);		/* set slave output */
} /* chMbit */

void
ch_bit(					/* CH_BIT slave action */
    register Gate *	gf,
    register Gate *	out_list)
{
    register Gate *	gp;

    /*
     * all change nodes have storage for new and old, although only
     * those fired from ARN type really need it.
     * up to this point changes in new back to old could have
     * caused this Gate to be unlinked (glitch), which is no change.
     */
    gf->gt_old = gf->gt_new;	/* now new value is fixed */
    gp = gf->gt_funct;
    if (gp->gt_val < 0) {
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, "\tV %s %2d ==>", gp->gt_ids, gp->gt_val);
	}
#endif
	gp->gt_val = 1;			/* reset slave output */
	link_ol(gp, o_list);
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, " %d", gp->gt_val);
	}
	/* on startup a reset action comes when VF is LO */
#endif
    }
} /* ch_bit */

/********************************************************************
 *
 *	Master function for on() CLOCK() and TIMER().
 *
 *******************************************************************/

void
fMfn(					/* F_CF CLCK TIMR master action */
    register Gate *	gp,
    Gate *		out_list)
{
    if ( gp->gt_val < 0 || gp->gt_next) {
	link_ol(gp, gp->gt_clk);	/* master action */
    }
} /* fMfn */

/********************************************************************
 *
 *	Execute a C statement or C statement block when triggered
 *	by a clocked logical condition.
 *		on (expr,clock) x = y * cfn(z);	// iClock is default
 *		on (expr) { for (i = 0; i < 10; i++) cfn(z); z++; }
 *
 *******************************************************************/

void
f_cf(					/* F_CF slave action on CF */
    register Gate *	gf,
    register Gate *	out_list)
{
    register Gate *	gp;
    char		val;

    if (gf->gt_val < 0) {
#ifndef _WINDOWS 
	if (debug & 0100) fprintf(outFP, "\tF%d{", (int)gf->gt_funct);
#endif
	/* execute C function as action procedure with side effects */
#ifdef LOAD
	((CFunctp)(gf->gt_funct))();
#else
	c_exec((int)gf->gt_funct);
#endif
#ifndef _WINDOWS 
	if (debug & 0100) putc('}', outFP);
#endif
#if !defined(_WINDOWS) || defined(LOAD)
    } else {
	fprintf(errFP,
	    "\n%s: line %d: F%d cFunction %s receives -1 ==> 1 ???\n",
	    __FILE__, __LINE__, (int)gf->gt_funct, gf->gt_ids);
	quit(-1);
#endif
    }
} /* f_cf */

/********************************************************************
 *
 *	Output to a word or byte whose byte address is in gt_list.
 *		gt_mark == 1 means 1 byte is output
 *		gt_mark == 2 means 2 bytes or 1 word is output
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
    register Gate *	gp,		/* NOTE: there is no slave action */
    Gate *		out_list)
{
    int	val;

#ifdef MIXED
    if (out_list == o_list) {
	/* called from logic scan - convert d to a */
	/* MIXED mode is currently not compiled - ERROR */
	gp->gt_new = (int)((unsigned char)gp->gt_val >> 7);
    }
#endif
    val = gp->gt_old = gp->gt_new;	/* update gt_old since no link_ol */
    if (gp->gt_mark == 1) {
#ifndef _WINDOWS
	val &= 0xff;			/* for display only */
#endif
	*(char*)gp->gt_list = val;	/* output byte */
    } else if (gp->gt_mark == 2) {
	*(int*)gp->gt_list = val;	/* output word */
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
 *	Output a bit to a bit field whose byte address is in gt_list.
 *		gt_mark contains the bit position as a bit mask.
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
    register Gate *	gp,		/* NOTE: there is no slave action */
    Gate *		out_list)
{
#ifdef MIXED
    if (out_list == a_list) {
	/* called from arithmetic scan - convert a to d */
	/* MIXED mode is currently not compiled - ERROR */
	gp->gt_val = gp->gt_old ? -1 : 1;
    }
#endif
    if (gp->gt_val & 0x80) {		/* output action */
	*(uchar*)gp->gt_list |=
	(uchar)gp->gt_mark;		/* set bit */
#ifndef _WINDOWS 
	if (debug & 0100) putc('1', outFP);
#endif
    } else {
	*(uchar*)gp->gt_list &=
	~(uchar)gp->gt_mark;		/* clear bit */
#ifndef _WINDOWS 
	if (debug & 0100) putc('0', outFP);
#endif
    }
} /* outMx */

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
clock_fn(				/* Clock function */
    register Gate *	gf,
    register Gate *	out_list)
{
    register Gate *	sp;
    /*
     * the clock list is controlled by a control block which is the
     * Gate structure which goes with this function
     */
    if (gf->gt_val < 0) {
	register Gate *	tp;
	register Gate *	np;

	sp = gf->gt_funct;
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, "\tC %s", sp->gt_ids);
	}
#endif
	sp->gt_val = -1;		/* set for visualization only */
	if (sp->gt_next != sp) {
#ifndef _WINDOWS 
	    if (debug & 0100) {
		register int	dc = 0;	/* functions which are clocked */
		for (tp = sp->gt_next; tp != sp; tp = tp->gt_next) {
		    if (dc++ >= 8) {
			dc = 1;
			fprintf(outFP, "\n\t");
		    }
		    fprintf(outFP, "\t%s", tp->gt_ids);
		}
	    }
#endif
#ifndef DEQ
	    ((Gate *)c_list->gt_list)->gt_next = sp->gt_next;
	    (Gate *)c_list->gt_list = (Gate *)sp->gt_list;  /* link sp */
	    ((Gate *)c_list->gt_list)->gt_next = c_list;    /* to c_list */
#else
	    /* link chain of Gates on clock list sp to end of c_list */
	    tp = c_list->gt_prev;		/* save c_list.previous */
	    tp->gt_next = np = sp->gt_next;	/* c_list.last ==> new */
	    np->gt_prev = tp;			/* c_list.last <== new */
	    tp = sp->gt_prev;			/* save sp.previous */
	    tp->gt_next = c_list;		/* sp.last ==> c_list */
	    c_list->gt_prev = tp;		/* sp.last <== c_list */
#endif
	    Out_init(sp);			/* relink empty sp */
	}
#if !defined(_WINDOWS) || defined(LOAD)
#ifdef LEVEL
	if ((sp = gf->gt_clk) != c_list) {
	    link_ol(gf, sp);
	}
#endif
    } else {
	fprintf(errFP,
	    "\n%s: line %d: C clock %s receives -1 ==> 1 ???\n",
	    __FILE__, __LINE__, gf->gt_funct->gt_ids);
	quit(-1);
#endif
    }
} /* clock_fn */

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
timer_fn(				/* Timer function */
    register Gate *	gf,
    register Gate *	out_list)
{
    register Gate *	sp;
    /*
     * the timer list is controlled by a control block which is the
     * Gate structure which goes with this function
     */
    if (gf->gt_val < 0) {
	register Gate *	tp;	/* functions which are timed */
	register Gate *	np;

	sp = gf->gt_funct;
#ifndef _WINDOWS 
	if (debug & 0100) {
	    fprintf(outFP, "\tT %s", sp->gt_ids);
	}
#endif
	sp->gt_val = -1;		/* set for visualization only */
	if ((np = sp->gt_next) != sp) {
	    register int	dc = 1;	/* allow for (time) */

	    np->gt_mark--;		/* count down first element */
#ifndef _WINDOWS 
	    if (debug & 0100) {
		fprintf(outFP, "\t(%d)", np->gt_mark);
	    }
#endif
	    if (np->gt_mark == 0) {
#ifdef DEQ
		/* link head of timer chain sp to end of c_list */
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
		} while ((np = tp->gt_next) != sp && np->gt_mark == 0);
#ifndef DEQ
		((Gate *)c_list->gt_list)->gt_next = sp->gt_next;/* => new */
		sp->gt_next = np;		/* timer => rest */
		(Gate *)c_list->gt_list = tp;	/* clock last => new last */
		tp->gt_next = c_list;    	/* new last => c_list */
		if (np == sp) {			/* last entry ? */
		    (Gate *)sp->gt_list = np;	/*  yes, fix timer last */
		}
#else
		/* link tail of timer chain which is due to c_list */
		tp->gt_next = c_list;		/* sp.last ==> c_list */
		c_list->gt_prev = tp;		/* sp.last <== c_list */
		/* the re-linking of sp works correctly also if np == sp */
		sp->gt_next = np;		/* sp ==> new new */
		np->gt_prev = sp;		/* sp <== new new */
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
} /* timer_fn */

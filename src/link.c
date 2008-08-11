static const char link_c[] =
"@(#)$Id: link.c,v 1.27 2008/06/25 21:46:06 jw Exp $";
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
 *	link.c
 *	linked list routine
 *
 *	4-Aug-93	New mark algorithm using extra byte in Gate
 *	24-Jan-95	Faster algorithm for finding alternate list
 *
 *******************************************************************/

/* J.E. Wulff	3-Mar-85 */

#include	<stdio.h>
#include	"icc.h"

unsigned short	iC_mark_stamp = 1;		/* incremented every scan */
Gate *		iC_osc_gp = NULL;		/* report oscillations */
static short	warn_cnt = OSC_WARN_CNT;/* limit the number of oscillator warnings */

/* link a gate block into the output list */

void
iC_link_ol(
    Gate *		gp,
    Gate *		out_list)
{
    Gate *		tp;
    Gate *		np;
#ifndef DEQ
    Gate *		ap;
#endif	/* DEQ */
    unsigned short	diff;
    int			time;

    if (gp->gt_next) {
#if YYDEBUG && (!defined(_WINDOWS) || defined(LOAD))
	iC_glit_cnt++;				/* count glitches */
#endif	/* YYDEBUG && (!defined(_WINDOWS) || defined(LOAD)) */
#ifndef DEQ
	ap = tp = out_list;			/* glitch */
	diff = 0;				/* save time remaining */
	while (tp->gt_next != gp) {		/* find previous entry */
#if YYDEBUG && (!defined(_WINDOWS) || defined(LOAD))
	    glit_nxt++;				/* count glitch scan */
#endif	/* YYDEBUG && (!defined(_WINDOWS) || defined(LOAD)) */
	    diff += tp->gt_mark;		/* makes sense for TIMRL */
	    if ((tp = tp->gt_next) == ap) {	/* end of one list */
		/********************************************************************
		 * find glitch in alternate list - rare
		 *
		 * the list heads for o_list and a_list and their alternate
		 * list heads have pointers to their respective alternate
		 * list heads in gt_rlist.
		 *
		 * all clock or timer nodes are list heads which have *c_list as
		 * their alternate in gt_rlist.
		 *
		 * *clist (iClock) has 0 in gt_rlist for termination
		 *******************************************************************/
		if ((tp = (Gate*)tp->gt_rlist) == out_list || tp == 0) {
#if !defined(_WINDOWS) || defined(LOAD)
		    fprintf(iC_errFP,
    "\n%s: line %d: cannot find '%s' entry in '%s' or '%s' after glitch\n",
    __FILE__, __LINE__, gp->gt_ids, out_list->gt_ids, ap->gt_ids);
#endif	/* !defined(_WINDOWS) || defined(LOAD) */
		    iC_quit(-1);
		}
		ap =  tp;
#if YYDEBUG && !defined(_WINDOWS)
		if ((iC_debug & 0300) == 0300) putc('@', iC_outFP); /* alternate found */
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    }
	}
	np = tp->gt_next = gp->gt_next;		/* unlink from */
	gp->gt_next = 0;			/* activity list */
	if (np == ap) {				/* last entry ? */
	    ap->gt_list = (Gate **)tp;		/* adjust pointer */
	} else if (ap->gt_fni == TIMRL) {	/* correct timer list ? */
	    np->gt_mark += gp->gt_mark;		/* adjust diff prev to old */
	    gp->gt_mark += diff;		/* time remaining */
	}
#else	/* DEQ */
	tp = gp->gt_prev;			/* glitch - previous */
	tp->gt_next = np = gp->gt_next;		/* previous ==> next */
	np->gt_prev = tp;			/* previous <== next */
	gp->gt_next = gp->gt_prev = 0;		/* unlink Gate */
	if (out_list->gt_fni == TIMRL) {	/* correct timer list ? */
	    np->gt_mark += gp->gt_mark;		/* adjust diff prev to next */
	    /* ignore time remaining in gp */
	}
#endif	/* DEQ */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) fprintf(iC_outFP, "g<");
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    } else {
	iC_link_cnt++;				/* count link operations */
	if (gp->gt_fni < MIN_ACT) {		/* ARITH & GATE may oscillate */
	    if (gp->gt_mark != iC_mark_stamp) {	/* first link this cycle */
		gp->gt_mark = iC_mark_stamp;	/* yes, stamp the gate */
		gp->gt_mcnt = 0;		/*      clear mark count */
	    } else {
		if (++gp->gt_mcnt >= iC_osc_lim) {	/* new alg: cnt 1 larger */
#if YYDEBUG && !defined(_WINDOWS)
		    if ((iC_debug & 0300) == 0300) putc('#', iC_outFP);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    out_list = (Gate*)out_list->gt_rlist; /* link gate next cycle */
		    if (warn_cnt > 0
#if YYDEBUG && !defined(_WINDOWS)
				    && !(iC_debug & 0200)
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    ) {
			warn_cnt--;		/* limit the number of warning messages */
			iC_osc_gp = gp;		/* report oscillations */
		    }
		}
#if YYDEBUG && !defined(_WINDOWS)
//####		if ((iC_debug & 0300) == 0300) fprintf(iC_outFP, "%d", gp->gt_mcnt);
		if ((iC_debug & 0300) == 0300) fprintf(iC_outFP, "%d,%d", gp->gt_mcnt,iC_osc_lim);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	    }
	} else if (out_list->gt_fni == TIMRL) {	/* rest are actions */
	    /********************************************************************
	     * except OUTW or OUTX which is never linked to TIMRL - ignore
	     *
	     * action D_SH is always timed, even when arithmetic
	     * value is 0 (ignore gt_val). This implements sample/hold
	     *
	     * the same applies to F_SW which is the 'switch' action and
	     * to CH_BIT which is the CHANGE action. CHANGE times on both
	     * edges, both for arithmetic as well as for logical input.
	     *******************************************************************/
	    if (
		(
		    (
			gp->gt_val > 0 &&		/* 'LO' action gate and not */
			gp->gt_fni != CH_BIT &&
			gp->gt_fni != D_SH &&
			gp->gt_fni != F_SW
		    ) ||
		    (time = gp->gt_time->gt_old) <= 0	/* or required time is 0 or -ve */
		) &&
		(time = out_list->gt_old) <= 0		/* and preset off time is 0 */
	    ) {
		out_list = iC_c_list;			/* put on 'clock' list imme */
	    } else {
		/********************************************************************
		 * 'HI' action gate clocked by timer/counter (time >= 1)
		 * or alternate 'LO' action if preset time is 1 (TIMER1)
		 * which is equivalent to normal clocking.
		 * Link Gate gp into list sorted by time order.
		 * Negative times are treated like 0 times.
		 *******************************************************************/
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_debug & 0100) fprintf(iC_outFP, "(%d)!", time);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		tp = out_list;
		diff = 0;
		while ((np = tp->gt_next) != out_list &&
		    (int)(diff += np->gt_mark) <= time) {
		    tp = np;				/* scan along time sorted list */
		}
#ifndef DEQ
		tp->gt_next = gp;			/* old => new */
		gp->gt_next = np;			/* new => next */
		diff -= np->gt_mark;			/* full time to previous */
		gp->gt_mark = time - diff;		/* diff previous to new */
		if (np != out_list) {
		    np->gt_mark -= gp->gt_mark;		/* adjust diff new to old */
		} else {
		    out_list->gt_list = (Gate **)gp;	/* list => new */
#ifndef NOCHECK
		    /* check that algorithm is correct */
		    if (out_list->gt_mark != 0) {
			fprintf(iC_errFP,
		    "\n%s: line %d: TIMER %s gt_mark is %d, should be 0)\n",
			__FILE__, __LINE__, out_list->gt_ids, out_list->gt_mark);
			iC_quit(-1);
		    }
#endif	/* NOCHECK */
		}
#else	/* DEQ */
		np->gt_prev = tp->gt_next = gp;		/* next, previous ==> new */
		gp->gt_next = np;			/* new ==> next */
		gp->gt_prev = tp;			/* previous <== newious */
		if (np != out_list) {
		    diff -= np->gt_mark;		/* full time to previous */
		}
		gp->gt_mark = time - diff;		/* diff previous to new */
		np->gt_mark -= gp->gt_mark;		/* adjust diff new to old */
		/* if np == out_list, out_list->gt_mark gets -diff */
#endif	/* DEQ */
		return;					/* sorted link action complete */
	    }
	}
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_debug & 0100) putc('>', iC_outFP);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
#ifndef DEQ
	((Gate *)out_list->gt_list)->gt_next = gp;	/* old => new */
	gp->gt_next = out_list;				/* new => list */
	out_list->gt_list = (Gate **)gp;		/* list => new */
#else	/* DEQ */
	tp = out_list->gt_prev;				/* save previous */
	out_list->gt_prev = tp->gt_next = gp;		/* list, previous ==> new */
	gp->gt_next = out_list;				/* new ==> list */
	gp->gt_prev = tp;				/* previous <== new */
#endif	/* DEQ */
    }
} /* iC_link_ol */

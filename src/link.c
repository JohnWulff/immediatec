static const char link_c[] =
"@(#)$Id: link.c,v 1.15 2001/04/15 09:03:37 jw Exp $";
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

unsigned short	mark_stamp = 1;		/* incremented every scan */

/* link a gate block into the output list */

void
link_ol(
    Gate *		gp,
    Gate *		out_list)
{
    Gate *		tp;
    Gate *		np;
    Gate *		ap;
    unsigned short	diff;
    int			time;
#ifndef _WINDOWS 
    char *		format;
#endif

    if (gp->gt_next) {
#if !defined(_WINDOWS) || defined(LOAD)
	glit_cnt++;				/* count glitches */
#endif
#ifndef DEQ
	ap = tp = out_list;			/* glitch */
	diff = 0;				/* save time remaining */
	while (tp->gt_next != gp) {		/* find previous entry */
#if !defined(_WINDOWS) || defined(LOAD)
	    glit_nxt++;				/* count glitch scan */
#endif
	    diff += tp->gt_mark;		/* makes sense for TIMRL */
	    if ((tp = tp->gt_next) == ap) {	/* end of one list */
		/**********************************************************
		 *
		 *	find glitch in alternate list - rare
		 *
		 *	the list heads for o_list and a_list and their
		 *	alternate list heads have pointers to their
		 *	respective alternate list heads in gt_rlist.
		 *
		 *	all clock or timer nodes are list heads which
		 *	have *c_list as their alternate in gt_rlist.
		 *
		 *	*clist (iClock) has 0 in gt_rlist for termination
		 *
		 *********************************************************/
		if ((tp = (Gate*)tp->gt_rlist) == out_list || tp == 0) {
#if !defined(_WINDOWS) || defined(LOAD)
		    fprintf(errFP,
    "\n%s: line %d: cannot find '%s' entry in '%s' or '%s' after glitch\n",
    __FILE__, __LINE__, gp->gt_ids, out_list->gt_ids, ap->gt_ids);
#endif
		    quit(-1);
		}
		ap =  tp;
#ifndef _WINDOWS 
		if (debug & 0200) putc('@', outFP); /* alternate found */
#endif
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
#else
	tp = gp->gt_prev;			/* glitch - previous */
	tp->gt_next = np = gp->gt_next;		/* previous ==> next */
	np->gt_prev = tp;			/* previous <== next */
	gp->gt_next = gp->gt_prev = 0;		/* unlink Gate */
	if (out_list->gt_fni == TIMRL) {	/* correct timer list ? */
	    np->gt_mark += gp->gt_mark;		/* adjust diff prev to next */
	    /* ignore time remaining in gp */
	}
#endif
#ifndef _WINDOWS 
	if (debug & 0100) fprintf(outFP, "g<");
#endif
    } else {
#if !defined(_WINDOWS) || defined(LOAD)
	link_cnt++;				/* count link operations */
#endif
	if (gp->gt_fni < MIN_ACT) {	/* ARITH & GATE may oscillate */
	    if (gp->gt_mark != mark_stamp) {	/* first link this cycle */
		gp->gt_mark = mark_stamp;	/* yes, stamp the gate */
		gp->gt_mcnt = 0;		/*      clear mark count */
	    } else {
		if (++gp->gt_mcnt >= osc_max) {	/* new alg: cnt 1 larger */
#ifndef _WINDOWS 
		    if (debug & 0200) putc('#', outFP);
#endif
		    out_list = (Gate*)out_list->gt_rlist; /* link gate next cycle */
		}
#ifndef _WINDOWS 
		if (debug & 0200) fprintf(outFP, "%d", gp->gt_mcnt);
#endif
	    }
	} else if (out_list->gt_fni == TIMRL) {	/* rest are actions */
	    /*
	     * except OUTW or OUTX which is never linked to TIMRL - ignore
	     *
	     * action D_SH is always timed, even when arithmetic
	     * value is 0 (ignore gt_val). This implements sample/hold
	     *
	     * the same applies to F_SW which is the 'switch' action and
	     * to CH_BIT which is the CHANGE action. CHANGE times on both
	     * edges, both for arithmetic as well as for logical input.
	     */
	    if ((gp->gt_val > 0 &&			/* 'LO' action gate and not */
		(gp->gt_fni < D_SH || gp->gt_fni > CH_BIT) ||	/* D_SH .. CH_BIT */
		(time = gp->gt_time->gt_old) <= 0) &&	/* or required time is 0 or -ve */
		(time = out_list->gt_old) <= 0) {	/* and preset off time is 0 */
		out_list = c_list;			/* put on 'clock' list imme */
	    } else {
		/*
		 * 'HI' action gate clocked by timer/counter (time >= 1)
		 * or alternate 'LO' action if preset time is 1 (TIMER1)
		 * which is equivalent to normal clocking.
		 * Link Gate gp into list sorted by time order.
		 * Negative times are treated like 0 times.
		 */
#ifndef _WINDOWS 
		if (debug & 0100) fprintf(outFP, "(%d)!", time);
#endif
		tp = out_list;
		diff = 0;
		while ((np = tp->gt_next) != out_list &&
		    (int)(diff += np->gt_mark) <= time) {
		    tp = np;		/* scan along time sorted list */
		}
#ifndef DEQ
		tp->gt_next = gp;			/* old => new */
		gp->gt_next = np;			/* new => next */
		diff -= np->gt_mark;		/* full time to previous */
		gp->gt_mark = time - diff;		/* diff previous to new */
		if (np != out_list) {
		    np->gt_mark -= gp->gt_mark;	/* adjust diff new to old */
		} else {
		    out_list->gt_list = (Gate **)gp;	/* list => new */
#ifndef NOCHECK
		    /* check that algorithm is correct */
		    if (out_list->gt_mark != 0) {
			fprintf(errFP,
		    "\n%s: line %d: TIMER %s gt_mark is %d, should be 0)\n",
			__FILE__, __LINE__, out_list->gt_ids, out_list->gt_mark);
			quit(-1);
		    }
#endif
		}
#else
		np->gt_prev = tp->gt_next = gp;	/* next, previous ==> new */
		gp->gt_next = np;			/* new ==> next */
		gp->gt_prev = tp;			/* previous <== newious */
		if (np != out_list) {
		    diff -= np->gt_mark;		/* full time to previous */
		}
		gp->gt_mark = time - diff;		/* diff previous to new */
		np->gt_mark -= gp->gt_mark;		/* adjust diff new to old */
		/* if np == out_list, out_list->gt_mark gets -diff */
#endif
		return;			/* sorted link action complete */
	    }
	}
#ifndef _WINDOWS 
	if (debug & 0100) putc('>', outFP);
#endif
#ifndef DEQ
	((Gate *)out_list->gt_list)->gt_next = gp;	/* old => new */
	gp->gt_next = out_list;				/* new => list */
	out_list->gt_list = (Gate **)gp;		/* list => new */
#else
	tp = out_list->gt_prev;			/* save previous */
	out_list->gt_prev = tp->gt_next = gp;	/* list, previous ==> new */
	gp->gt_next = out_list;			/* new ==> list */
	gp->gt_prev = tp;			/* previous <== new */
#endif
    }
} /* link_ol */

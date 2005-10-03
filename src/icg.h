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
 *	icg.h
 *	immediate C - Gate structure
 *
 *******************************************************************/

/* J.E. Wulff	3-Mar-85 */

#ifndef ICG_H
#define ICG_H
static const char icg_h[] =
"@(#)$Id: icg.h,v 1.8 2005/08/18 12:05:13 jw Exp $";

#ifndef INT_MAX
#include	<limits.h>
#endif

/*	Function types 'op' 'Symbol.type' and display */

#define	iC_UDF		0	/* represents undefined type */
			    /* arithmetic functions */
#define	iC_ARNC		1	/* arithmetic node in a C fragment */
#define	iC_ARN		2	/* arithmetic node */
			    /* logical functions */
#define	iC_LOGC		3	/* logical node in a C fragment */
#define	iC_XOR		4	/* logical XOR gate */
#define	iC_AND		5	/* logical AND gate */
#define	iC_OR		6	/* logical OR gate */
#define	iC_LATCH	7	/* single node LATCH element */
			    /* functions driven by action gates */
#define	iC_SH		8	/* sample and hold clocked action */
#define	iC_FF		9	/* flip flop clocked action */
#define	iC_EF		10	/* RISE clocked action */
#define	iC_VF		11	/* CHANGE clocked action */
#define	iC_SW		12	/* SWITCH clocked action NOTE: no output */
#define	iC_CF		13	/* IF clocked action NOTE: no output */
			    /* values from extermal sources */
#define	iC_NCONST	14	/* constant number */
#define	iC_INPB		15	/* byte input from bit output OUTX */
#define	iC_INPW		16	/* arithmetic input */
#define	iC_INPX		17	 /* logical input (must be 1 higher than INPW) */
			    /* clock functions controlling clock lists */
#define	iC_CLK		18	/* CLOCK clocked action */
#define	iC_TIM		19	/* TIMER clocked action */
			    /* non executable functions */
#define	iC_ALIAS	20	/* non executable functions */
#define	iC_ERR		21	/* mark node which had error during generation */
			    /* for compilation only */
#define	iC_KEYW		22	/* hold yacc token, used for compilation only */
		    /* for C-compilation only */
#define	iC_CTYPE	23	/* C Types */
#define	iC_CWORD	24	/* C tokens used temporarily */

#define	iC_IFUNCT	25	/* immediate function type */

/*	Output types: Symbol.ftype and iC_Gt.gt_fni */

#define	iC_UDFA		0	/* indices into action function arrays */
			    /* unclocked immediate outputs */
#define	iC_ARITH	1	/* arithmetic action */
#define	iC_GATE		2	/* optimized logical action - handles target AND OR LATCH only */
#define	iC_GATEX	3	/* full logical action - handles target AND OR LATCH as well as XOR */
			    /* clocked actions used internally */
#define	iC_RI_BIT	4	/* RISE logical action */
#define	iC_S_SH		5	/* sample and hold set action */
#define	iC_R_SH		6	/* sample and hold reset action */
#define	iC_D_SH		7	/* sample and hold arithmetic action */
#define	iC_CH_BIT	8	/* CHANGE logical or arithmetic action (iClive uses CH_BIT-RI_BIT) */
#define	iC_S_FF		9	/* FF set logical action   requires 1001 at genr.c 233 */
#define	iC_R_FF		10	/* FF reset logical action requires 1010 */
#define	iC_D_FF		11	/* FF D logical action     requires 1011 */
#define	iC_F_SW		12	/* SWITCH arithmetic action */
#define	iC_F_CF		13	/* IF logical action */
#define	iC_F_CE		14	/* IF ELSE logical action */
#define	iC_CLCK		15	/* CLOCK logical action */
#define	iC_TIMR		16	/* TIMER logical action */
			    /* outputs to external sinks */
#define	iC_TRAB		17	/* logical input byte transferred to INPX */
#define	iC_OUTW		18	/* arithmetic output */
#define	iC_OUTX		19	/* logical output */
			    /* clock outputs */
#define	iC_CLCKL	20	/* clock action */
#define	iC_TIMRL	21	/* timer action */

typedef struct Gate {			/* Gate */
	char		gt_val;		/* forward logic value */
	char		gt_ini;		/* initial logic value */
	unsigned char	gt_fni;		/* function index */
	unsigned char	gt_mcnt;	/* mark counter */
	char *		gt_ids;		/* id string */
	struct Gate **	gt_list;	/* forward logic list */
	struct Gate **	gt_rlist;	/* reverse logic list */
	struct Gate *	gt_next;	/* forward link */
	unsigned short	gt_mark;	/* mark for stamping gate */
	unsigned short	gt_live;	/* live flag and index */
#ifdef DEQ
	struct Gate *	gt_prev;	/* previous link */
#endif
#if INT_MAX == 32767 && defined (LONG16)
	long		gt_new;		/* new value for arithhmetic */
	long		gt_old;		/* old value for arithhmetic */
#else
	int		gt_new;		/* new value for arithhmetic */
	int		gt_old;		/* old value for arithhmetic */
#endif
} iC_Gt;

extern iC_Gt		iClock;		/* System clock */

#if INT_MAX == 32767 && defined (LONG16)
extern long		iC_assignA(iC_Gt * lv, long rv);
extern long		iC_assignL(iC_Gt * lv, long rv);
#else
extern int		iC_assignA(iC_Gt * lv, int rv);
extern int		iC_assignL(iC_Gt * lv, int rv);
#endif
#endif	/* ICG_H */

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
"@(#)$Id: icg.h,v 1.6 2004/12/22 16:56:27 jw Exp $";

#ifndef INT_MAX
#include	<limits.h>
#endif

/*	Function types 'op' 'Symbol.type' and display */

#define	UDF	0	/* represents undefined type */
		    /* arithmetic functions */
#define	ARNC	1	/* arithmetic node in a C fragment */
#define	ARN	2	/* arithmetic node */
		    /* logical functions */
#define	LOGC	3	/* logical node in a C fragment */
#define	AND	4	/* logical AND gate */
#define	OR	5	/* logical OR gate */
#define	LATCH	6	/* single node LATCH element */
		    /* functions driven by action gates */
#define	SH	7	/* sample and hold clocked action */
#define	FF	8	/* flip flop clocked action */
#define	VF	9	/* CHANGE clocked action */
#define	EF	10	/* RISE clocked action */
#define	SW	11	/* SWITCH clocked action NOTE: no output */
#define	CF	12	/* IF clocked action NOTE: no output */
		    /* values from extermal sources */
#define	NCONST	13	/* constant number */
#define	INPB	14	/* byte input from bit output OUTX */
#define	INPW	15	/* arithmetic input */
#define	INPX	(INPW+1) /* logical input */
		    /* clock functions controlling clock lists */
#define	CLK	17	/* CLOCK clocked action */
#define	TIM	18	/* TIMER clocked action */
		    /* non executable functions */
#define	ALIAS	19	/* non executable functions */
#define	ERR	20	/* mark node which had error during generation */
		    /* for compilation only */
#define	KEYW	21	/* hold yacc token, used for compilation only */
		    /* for C-compilation only */
#define	CTYPE	22	/* C Types */
#define	CWORD	23	/* C tokens used temporarily */

#define	IFUNCT	24	/* immediate function type */

#define	MAX_GT	SH	/* types < MAX_GT are driven by a value */
#define	MAX_LV	CLK	/* types < MAX_LV return a logical or arith value */
#define	MAX_OP	ALIAS	/* types < MAX_OP are executable */
#define	MAX_LS	KEYW	/* types < MAX_LS are generated */

			/* mark nodes declared as extern - stop assignment */
#define	TM	0x1f	/* mask for type from EXT_TYPES */
#define	EM	0x20	/* mask for external type from EXT_TYPES */
#define	FM	0x40	/* mask for terminal function type from FUN_TYPES */
			/* intermediate Symbols are marked by ->name == 0 */
#define EXT_ARN	(ARN | EM)
#define EXT_AND	(AND | EM)
#define EXT_OR	(OR  | EM)
#define EXT_CLK	(CLK | EM)
#define EXT_TIM	(TIM | EM)

/*	Output types: Symbol.ftype and Gate.gt_fni */

#define	UDFA	0	/* indices into action function arrays */
		    /* unclocked immediate outputs */
#define	ARITH	1	/* arithmetic action */
#define	GATE	(ARITH+1) /* logical action */
		    /* clocked actions used internally */
#define	RI_BIT	3	/* RISE logical action */
#define	CH_BIT	4	/* CHANGE logical or arithmetic action */
#define	S_SH	5	/* sample and hold set action */
#define	R_SH	6	/* sample and hold reset action */
#define	D_SH	7	/* sample and hold arithmetic action */
#define	F_SW	8	/* SWITCH arithmetic action */
#define	S_FF	9	/* FF set logical action   not needed 1001 */
#define	R_FF	10	/* FF reset logical action not needed 1010 */
#define	D_FF	11	/* FF D logical action     not needed 1011 */
#define	F_CF	12	/* IF logical action */
#define	F_CE	13	/* IF ELSE logical action */
#define	CLCK	14	/* CLOCK logical action */
#define	TIMR	15	/* TIMER logical action */
		    /* outputs to external sinks */
#define	TRAB	16	/* logical input byte transferred to INPX */
#define	OUTW	17	/* arithmetic output */
#define	OUTX	18	/* logical output */
		    /* clock outputs */
#define	CLCKL	19	/* clock action */
#define	TIMRL	20	/* timer action */

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
	int		gt_channel;	/* TODO use gt_list to store channel */
} Gate;

extern Gate		iClock;		/* System clock */
extern Gate *		IX_[];		/* pointers to bit Input Gates */
extern Gate *		IB_[];		/* pointers to byte Input Gates */
extern Gate *		IW_[];		/* pointers to word Input Gates */
#if INT_MAX != 32767 || defined (LONG16)
extern Gate *		IL_[];		/* pointers to long Input Gates */
#endif
extern Gate *		TX_[];		/* pointers to bit System Gates */

#define aAssign		assign
#define lAssign		assign
#if INT_MAX == 32767 && defined (LONG16)
extern long		assign(Gate * lv, long rv);
#else
extern int		assign(Gate * lv, int rv);
#endif
#endif	/* ICG_H */

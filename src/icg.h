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
 *	icg.h
 *	immediate C - Gate structure
 *
 *******************************************************************/

/* J.E. Wulff	3-Mar-85 */

#ifndef ICG_H
#define ICG_H
static const char icg_h[] =
"@(#)$Id: icg.h,v 1.1 2002/08/26 19:25:27 jw Exp $";

#define	NOT	1	/* used in List_e.le_val */

/* operations used in 'op' 'Symbol.type' and display */

#define	UDF	0	/* represents undefined type */
#define	ARNC	1	/* arithmetic node in a C fragment */
#define	ARN	2	/* arithmetic node */

#define	LOGC	3	/* logical node in a C fragment */
#define	AND	4
#define	OR	5
#define	LATCH	6

#define	SH	7	/* functions driven by action gates */
#define	FF	8
#define	VF	9
#define	EF	10
#define	SW	11	/* NOTE: no output */
#define	CF	12	/* NOTE: no output */
#define	NCONST	13	/* constant number */
#define	INPW	14	/* arithmetic input */
#define	INPX	15	/* logical input */

#define	CLK	16	/* functions which controll clock lists */
#define	TIM	17

#define	ALIAS	18	/* non executable functions */
#define	ERR	19	/* mark node which had error during generation */

#define	KEYW	20	/* hold yacc token, used for compilation only */

#define	CTYPE	21	/* used for C-compilation only */
#define	CWORD	22	/* used for C-compilation only */

#define	MAX_GT	SH	/* types < MAX_GT are driven by a value */
#define	MAX_LV	CLK	/* types < MAX_LV return a logical or arith value */
#define	MAX_OP	ALIAS	/* types < MAX_OP are executable */
#define	MAX_LS	KEYW	/* types < MAX_LS are generated */

			/* mark nodes declared as extern - stop assignment */
#define	TM	31	/* 0x1f mask for type from EXT_TYPES */
#define EXT_ARN	ARN+TM+1
#define EXT_AND	AND+TM+1
#define EXT_OR	OR+TM+1
#define EXT_CLK	CLK+TM+1
#define EXT_TIM	TIM+TM+1

/*	action function types Symbol.ftype and Gate.gt_fni */

#define	UDFA	0	/* indices into action function arrays */
#define	ARITH	1	/* arithmetic gate */
#define	GATE	2	/* logical gate */
#define	RI_BIT	3	/* logical action */
#define	CH_BIT	4	/* arithmetic action */
#define	S_SH	5	/* arithmetic action */
#define	R_SH	6	/* arithmetic action */
#define	D_SH	7	/* arithmetic action */
#define	F_SW	8	/* arithmetic action */
#define	S_FF	9	/* logical action not needed 1001 */
#define	R_FF	10	/* logical action not needed 1010 */
#define	D_FF	11	/* logical action not needed 1011 */
#define	F_CF	12	/* logical action */
#define	F_CE	13	/* logical action */
#define	CLCK	14	/* logical action */
#define	TIMR	15	/* logical action */
#define	OUTW	16	/* arithmetic output */
#define	OUTX	17	/* logical output */
#define	CLCKL	18	/* clock action */
#define	TIMRL	19	/* timer action */

#define	MAX_AR	GATE	/* ftypes >= MAX_AR never cause simple arithmetic */
#define	MIN_ACT	RI_BIT	/* ftypes >= MIN_ACT cause an action */
#define	MAX_ACT	OUTW	/* ftypes >= MAX_ACT never cause an action */
#define	MAX_FTY	TIMRL+1	/* ftypes >= MAX_FTY are ALIAS ftypes for live list */

#define	ONCE_M	0x100	/* actions containing this bit only once */
#define	S_FF_M	0x01	/* masks in array bit2 for pass2 */
#define	R_FF_M	0x02
#define	D_FF_M	(S_FF_M | R_FF_M | ONCE_M)
#define	CH_B_M	0x04
#define	RI_B_M	0x08
#define	S_SH_M	0x10
#define	R_SH_M	0x20
#define	D_SH_M	(S_SH_M | R_SH_M | ONCE_M)
#define	CLCK_M	0x40
#define	TIMR_M	0x80
#define	F_CF_M	0	/* has no slave node */
#define	F_CW_M	0	/* has no slave node */
#define	INPT_M	0	/* only used for check so far */
#define	OUTP_M	1	/* used to check that 1 input */

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
	int		gt_new;		/* new value for arithhmetic */
	int		gt_old;		/* old value for arithhmetic */
} Gate;

#define	FL_GATE	0
#define	FL_CLK	1
#define	FL_TIME	2
			/* action gate output or C function pointer */
#define	gt_funct	gt_list[FL_GATE]
			/* clock list pointer */
#define	gt_clk		gt_list[FL_CLK]
			/* gate holding time value (ARN or NCONST) */
#define	gt_time		gt_list[FL_TIME]
	/* this order is required for initialisation */

extern Gate		iClock;		/* System clock */
extern Gate *		IX_[];		/* pointers to Bit Input Gates */
extern Gate *		IB_[];		/* pointers to Byte Input Gates */
extern Gate *		IW_[];		/* pointers to Word Input Gates */
extern Gate *		TX_[];		/* pointers to System Bit Gates */
#endif	/* ICG_H */

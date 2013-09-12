/********************************************************************
 *
 *	Copyright (C) 1985-2011  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	icc.h
 *	immediate C - runtime execution header
 *
 *******************************************************************/

#ifndef ICC_H
#define ICC_H
static const char icc_h[] =
"@(#)$Id: icc.h,v 1.73 2013/09/10 08:31:47 jw Exp $";

/* STARTFILE "icg.h" */
/********************************************************************
 *
 *	Copyright (C) 1985-2011  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	icg.h	// extracted from icc.h by perl script icg.pl
 *	immediate C - Gate structure
 *
 *******************************************************************/

#ifndef ICG_H
#define ICG_H

#ifndef INT_MAX
#include	<limits.h>
#endif

/*	Function types 'op' 'Symbol.type' and display */

#define	iC_UDF		0	/* represents undefined type */
			    /* arithmetic functions */
#define	iC_ARNC		1	/* arithmetic node in a C fragment */
#define	iC_ARNF		2	/* arithmetic node - function block return */
#define	iC_ARN		3	/* arithmetic node */
			    /* logical functions */
#define	iC_LOGC		4	/* logical node in a C fragment */
#define	iC_XOR		5	/* logical XOR gate */
#define	iC_AND		6	/* logical AND gate */
#define	iC_OR		7	/* logical OR gate */
#define	iC_LATCH	8	/* single node LATCH element */
			    /* functions driven by action gates */
#define	iC_SH		9	/* sample and hold clocked action */
#define	iC_FF		10	/* flip flop clocked action */
#define	iC_EF		11	/* RISE clocked action */
#define	iC_VF		12	/* CHANGE clocked action */
#define	iC_SW		13	/* SWITCH clocked action NOTE: no output */
#define	iC_CF		14	/* IF clocked action NOTE: no output */
			    /* values from extermal sources */
#define	iC_NCONST	15	/* constant number */
#define	iC_INPB		16	/* byte input from bit output OUTX */
#define	iC_INPW		17	/* arithmetic input */
#define	iC_INPX		18	 /* logical input (must be 1 higher than INPW) */
			    /* clock functions controlling clock lists */
#define	iC_CLK		19	/* CLOCK clocked action */
#define	iC_TIM		20	/* TIMER clocked action */
			    /* non executable functions */
#define	iC_ALIAS	21	/* non executable functions */
#define	iC_ERR		22	/* mark node which had error during generation */
			    /* for compilation only */
#define	iC_KEYW		23	/* hold yacc token, used for compilation only */
		    /* for C-compilation only */
#define	iC_CTYPE	24	/* C Types */
#define	iC_CWORD	25	/* C tokens used temporarily */

#define	iC_IFUNCT	26	/* immediate function type */

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
#define	iC_CH_BIT	8	/* CHANGE logical action */
#define	iC_S_FF		9	/* FF set logical action   requires 1001 at genr.c 233 */
#define	iC_R_FF		10	/* FF reset logical action requires 1010 */
#define	iC_D_FF		11	/* FF D logical action     requires 1011 */
#define	iC_CH_AR	12	/* CHANGE arithmetic action */
#define	iC_F_SW		13	/* SWITCH arithmetic action */
#define	iC_F_CF		14	/* IF logical action */
#define	iC_F_CE		15	/* IF ELSE logical action */
#define	iC_CLCK		16	/* CLOCK logical action */
#define	iC_TIMR		17	/* TIMER logical action */
			    /* outputs to external sinks */
#define	iC_TRAB		18	/* logical input byte transferred to INPX */
#define	iC_OUTW		19	/* arithmetic output */
#define	iC_OUTX		20	/* logical output */
			    /* clock outputs */
#define	iC_CLCKL	21	/* clock action */
#define	iC_TIMRL	22	/* timer action */
#define	iC_F_ERR	23	/* error action */

typedef union GppIpI {
    struct Gate **	gpp;		/* Gate pointer array */
    struct Gate *	gp;		/* Gate pointer */
    int		*	ip;		/* int array */
#ifdef TCP
    unsigned short	channel;	/* output channel */
#if INT_MAX == 32767 && defined (LONG16)
    long		out;		/* out value for arithhmetic */
#else
    int			out;
#endif
#else	/* TCP */
    int			slot;		/* out slot index */
#endif	/* TCP */
} GppIpI;				/* auxiliary union to avoid casts */

typedef struct Gate {			/* Gate */
	signed char	gt_val;		/* forward logic value */
	signed char	gt_ini;		/* initial logic value */
	unsigned char	gt_fni;		/* function index */
	unsigned char	gt_mcnt;	/* mark counter */
	char *		gt_ids;		/* id string */
	union  GppIpI	gt_ll;		/* forward logic list */
	union  GppIpI	gt_rll;		/* reverse logic list */
	struct Gate *	gt_next;	/* forward link */
	unsigned short	gt_mark;	/* mark for stamping gate */
	unsigned short	gt_live;	/* live flag and index */
#ifdef DEQ
	struct Gate *	gt_prev;	/* back link for double ended queue */
#endif
#if INT_MAX == 32767 && defined (LONG16)
	long		gt_new;		/* new value for arithhmetic */
	long		gt_old;		/* old value for arithhmetic */
#else
	int		gt_new;		/* new value for arithhmetic */
	int		gt_old;		/* old value for arithhmetic */
#endif
} iC_Gt;

#define	FL_GATE	0
#define	FL_CLK	1
#define	FL_TIME	2
				/* normal gate pointer pointer */
#define	gt_list		gt_ll.gpp
				/* normal gate pointer */
#define	gt_ptr		gt_ll.gp
#ifdef TCP
				/* OUT channel for OUTW gates */
#define	gt_channel	gt_ll.channel
#else	/* TCP */
				/* OUT slot index for OUTW and OUTX gates */
#define	gt_slot		gt_ll.slot
#endif	/* TCP */
				/* action C function pointer */
#define	gt_funct	gt_ll.gpp[FL_GATE]
				/* action C function number in int array */
#define	gt_functn	gt_ll.ip[FL_GATE]
				/* clock list pointer */
#define	gt_clk		gt_ll.gpp[FL_CLK]
				/* gate holding time value (ARN or NCONST) */
#define	gt_time		gt_ll.gpp[FL_TIME]
				/* this order is required for initialisation */

				/* reverse gate pointer pointer */
#define	gt_rlist	gt_rll.gpp
				/* reverse gate pointer */
#define	gt_rptr		gt_rll.gp
#ifdef TCP
				/* OUT value for OUTW gates */
#define	gt_out		gt_rll.out
#endif	/* TCP */
				/* reverse C function number in int array */
#define	gt_rfunctn	gt_rll.ip[FL_GATE]

extern iC_Gt		iClock;		/* System clock */

#if INT_MAX == 32767 && defined (LONG16)
extern long		iC_assignA(iC_Gt * glv, int ppi, long rv);
extern long		iC_assignL(iC_Gt * glv, int ppi, long rv);
#else
extern int		iC_assignA(iC_Gt * glv, int ppi, int rv);
extern int		iC_assignL(iC_Gt * glv, int ppi, int rv);
#endif
extern iC_Gt *		iC_index(iC_Gt * gm, int index);
extern struct timeval	iC_timeOut;	/* 50 mS select timeout - may be modified in iCbegin() */
extern int		iCbegin(void);	/* initialisation function */
extern int		iCend(void);	/* termination function */
#endif	/* ICG_H */
/* ENDFILE "icg.h" */

#ifdef _WINDOWS
#define	strlen(a)	lstrlen(a)
#define	strcpy(a,b)	lstrcpy(a,b)
#define	strcmp(a,b)	lstrcmp(a,b)
#define	sprintf		wsprintf
#define	calloc(a,b)	iC_emalloc((a)*(b))
#define	free(a)		iC_efree(a)
extern void		iC_efree(void *);
#endif

#ifndef PPGATESIZE
#define	PPGATESIZE 127		/* natural gate size for char gt_val */
#endif
#define	INSTSIZE 3		/* limit instance to 3 characters */

#define	IXD	64		/* number of slots in immcc (<= 64) */
#define	TXD	1		/* number of system slots (timers etc) */
#define ESIZE	512

/*	Function types 'op' 'Symbol.type' and display */

#define	UDF	iC_UDF		/* represents undefined type */
		    	/* arithmetic functions */
#define	ARNC	iC_ARNC		/* arithmetic node in a C fragment */
#define	ARN	iC_ARN		/* arithmetic node */
#define	ARNF	iC_ARNF		/* arithmetic node - function block return */
		    	/* logical functions */
#define	LOGC	iC_LOGC		/* logical node in a C fragment */
#define	XOR	iC_XOR		/* logical XOR gate */
#define	AND	iC_AND		/* logical AND gate */
#define	OR	iC_OR		/* logical OR gate */
#define	LATCH	iC_LATCH	/* single node LATCH element */
		    	/* functions driven by action gates */
#define	SH	iC_SH		/* sample and hold clocked action */
#define	FF	iC_FF		/* flip flop clocked action */
#define	EF	iC_EF		/* RISE clocked action */
#define	VF	iC_VF		/* CHANGE clocked action */
#define	SW	iC_SW		/* SWITCH clocked action NOTE: no output */
#define	CF	iC_CF		/* IF clocked action NOTE: no output */
		    	/* values from extermal sources */
#define	NCONST	iC_NCONST	/* constant number */
#define	INPB	iC_INPB		/* byte input from bit output OUTX */
#define	INPW	iC_INPW		/* arithmetic input */
#define	INPX	iC_INPX		 /* logical input */
		    	/* clock functions controlling clock lists */
#define	CLK	iC_CLK		/* CLOCK clocked action */
#define	TIM	iC_TIM		/* TIMER clocked action */
		    	/* non executable functions */
#define	ALIAS	iC_ALIAS	/* non executable functions */
#define	ERR	iC_ERR		/* mark node which had error during generation */
		   	 /* for compilation only */
#define	KEYW	iC_KEYW		/* hold yacc token, used for compilation only */
		  	  /* for C-compilation only */
#define	CTYPE	iC_CTYPE	/* C Types */
#define	CWORD	iC_CWORD	/* C tokens used temporarily */

#define	IFUNCT	iC_IFUNCT	/* immediate function type */

/*	Output types: Symbol.ftype and iC_Gt.gt_fni */

#define	UDFA	iC_UDFA		/* indices into action function arrays */
			    /* unclocked immediate outputs */
#define	ARITH	iC_ARITH	/* arithmetic action */
#define	GATE	iC_GATE		/* optimized logical action - handles target AND OR LATCH only */
#define	GATEX	iC_GATEX	/* full logical action - handles target AND OR LATCH as well as XOR */
			    /* clocked actions used internally */
#define	RI_BIT	iC_RI_BIT	/* RISE logical action */
#define	S_SH	iC_S_SH		/* sample and hold set action */
#define	R_SH	iC_R_SH		/* sample and hold reset action */
#define	D_SH	iC_D_SH		/* sample and hold arithmetic action */
#define	CH_BIT	iC_CH_BIT	/* CHANGE logical action */
#define	S_FF	iC_S_FF		/* FF set logical action   requires 1001 at genr.c 233 */
#define	R_FF	iC_R_FF		/* FF reset logical action requires 1010 */
#define	D_FF	iC_D_FF		/* FF D logical action     requires 1011 */
#define	CH_AR	iC_CH_AR	/* CHANGE arithmetic action */
#define	F_SW	iC_F_SW		/* SWITCH arithmetic action */
#define	F_CF	iC_F_CF		/* IF logical action */
#define	F_CE	iC_F_CE		/* IF ELSE logical action */
#define	CLCK	iC_CLCK		/* CLOCK logical action */
#define	TIMR	iC_TIMR		/* TIMER logical action */
			    /* outputs to external sinks */
#define	TRAB	iC_TRAB		/* logical input byte transferred to INPX */
#define	OUTW	iC_OUTW		/* arithmetic output */
#define	OUTX	iC_OUTX		/* logical output */
			    /* clock outputs */
#define	CLCKL	iC_CLCKL	/* clock action */
#define	TIMRL	iC_TIMRL	/* timer action */
#define	F_ERR	iC_F_ERR	/* error action */

typedef struct Gate	Gate;	/* iC_Gt equivalent to Gate */

#define	MIN_GT	(LOGC+1)	/* types < MIN_GT are UDF, Arith and LOGC */
#define	MAX_GT	(LATCH+1)	/* types < MAX_GT are driven by a value */
#define	MAX_LV	(INPX+1)	/* types < MAX_LV return a logical or arith value */
#define	MAX_OP	(TIM+1)		/* types < MAX_OP are executable */
#define	MAX_LS	(ERR+1)		/* types < MAX_LS are generated */

#define	MAX_AR	(ARITH+1)	/* ftypes >= MAX_AR never cause simple arithmetic */
#define	MIN_ACT	RI_BIT		/* ftypes >= MIN_ACT cause an action */
#define	MAX_ACT	(TIMR+1)	/* ftypes >= MAX_ACT never cause an action */
#define	MAX_FTY	(TIMRL+1)	/* ftypes >= MAX_FTY are ALIAS ftypes for iClive list */

#define	ONCE_M	0x100		/* actions containing this bit only once */
#define	S_FF_M	0x01		/* masks in array iC_bit2 for pass2 */
#define	R_FF_M	0x02
#define	D_FF_M	(S_FF_M | R_FF_M | ONCE_M)
#define	CH_B_M	0x04
#define	RI_B_M	0x08
#define	S_SH_M	0x10
#define	R_SH_M	0x20
#define	D_SH_M	(S_SH_M | R_SH_M | ONCE_M)
#define	CLCK_M	0x40
#define	TIMR_M	0x80
#define	F_CF_M	0		/* has no slave node */
#define	F_CW_M	0		/* has no slave node */
#define	INPT_M	0		/* only used for check so far */
#define	OUTP_M	1		/* used to check that 1 input */

#define	Sizeof(x)	((sizeof x) / (sizeof x[0]))
#ifndef DEQ
#define	Out_init(ol)	(ol->gt_next = ol->gt_ptr  = ol)
#else
#define	Out_init(ol)	(ol->gt_next = ol->gt_prev = ol)
#endif
#if defined(TCP) || defined(LOAD)
extern void	iC_gateMa(Gate *, Gate *);	/* GATE master action */
#else	/* defined(TCP) || defined(LOAD) */
#define		iC_gateMa	iC_link_ol
#endif	/* defined(TCP) || defined(LOAD) */

/********************************************************************
 *
 *	The following lists are indexed by type
 *
 *	UDF	ARNC	ARNF	ARN	LOGC	XOR	AND	OR	LATCH
 *	SH	FF	EF	VF	SW	CF	NCONST	INPB	INPW	INPX
 *	CLK	TIM	ALIAS	ERR	KEYW	CTYPE	CWORD	IFUNCT
 *
 *******************************************************************/

/* list of types */
#define	FULL_TYPE \
	"UDF","ARNC","ARNF","ARN","LOGC","XOR","AND","OR","LATCH",\
	"SH","FF","EF","VF","SW","CF","NCONST","INPB","INPW","INPX",\
	"CLK","TIM","ALIAS","ERR","KEYW","CTYPE","CWORD","IFUNCT",

/* list of iC_ extended types in icg.h */
#define	iC_EXT_TYPE \
	"iC_UDF","iC_ARNC","iC_ARNF","iC_ARN","iC_LOGC","iC_XOR","iC_AND","iC_OR","iC_LATCH",\
	"iC_SH","iC_FF","iC_EF","iC_VF","iC_SW","iC_CF","iC_NCONST","iC_INPB","iC_INPW","iC_INPX",\
	"iC_CLK","iC_TIM","iC_ALIAS","iC_ERR","iC_KEYW","iC_CTYPE","iC_CWORD","iC_IFUNCT",

#define	OPS	".-++\"^&|%*#/>({=][<:!@?ktwi"	/* DEBUG display of types - line up with pplstfix */

/* ftypes corresponding to types */
#define	FTYPES \
	UDFA, ARITH, ARITH, ARITH, GATE, GATE, GATE, GATE, GATE,\
	D_SH, D_FF, RI_BIT, CH_BIT, F_SW, F_CF, ARITH, OUTX, ARITH, TRAB,\
	CLCK, TIMR, GATE, F_ERR, 0, 0, 0, 0,

/* compiler tokens corresponding to type */
#define	DEF_TYP \
	0, AVARC, 0, 0, LVARC, 0, 0, 0, 0,\
	0, 0, 0, 0, 0, 0, NUMBER, 0, 0, 0,\
	0, 0, 0, 0,  0, 0, 0, 0,

/* initialisation tables for different types (must have MAX_LS entries) */
#define	I_LISTS \
	iC_gate_i, iC_gate_i, iC_gate_i, iC_gate_i, iC_gate_i, iC_gate_i, iC_gate_i, iC_gate_i, iC_gate_i,\
	iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i,\
	iC_clock_i, iC_clock_i, iC_clock_i, iC_clock_i,

/********************************************************************
 *
 *	The following lists are indexed by gt_fni (is ftype)
 *
 *	UDFA	ARITH	GATE	GATEX	RI_BIT	S_SH	R_SH	D_SH
 *	CH_BIT	S_FF	R_FF	D_FF	CH_AR	F_SW	F_CF	F_CE	CLCK	TIMR
 *	TRAB	OUTW	OUTX	CLCKL	TIMRL	F_ERR
 *
 *******************************************************************/

/* list of ftypes */
#define	FULL_FTYPE \
	"UDFA","ARITH","GATE","GATEX","RI_BIT","S_SH","R_SH","D_SH",\
	"CH_BIT","S_FF","R_FF","D_FF","CH_AR","F_SW","F_CF","F_CE","CLCK","TIMR",\
	"TRAB","OUTW","OUTX","CLCKL","TIMRL","F_ERR",

/* list of iC_ extended ftypes in icg.h */
#define	iC_EXT_FTYPE \
	"iC_UDFA","iC_ARITH","iC_GATE","iC_GATEX","iC_RI_BIT","iC_S_SH","iC_R_SH","iC_D_SH",\
	"iC_CH_BIT","iC_S_FF","iC_R_FF","iC_D_FF","iC_CH_AR","iC_F_SW","iC_F_CF","iC_F_CE","iC_CLCK","iC_TIMR",\
	"iC_TRAB","iC_OUTW","iC_OUTX","iC_CLCKL","iC_TIMRL","iC_F_ERR",

/* list of VCD types for GTKWAVE - use first characters 'i' 'w' or 'e' for identification in some cases*/
/* see gtkwave-3.3.43/src/vcd_keywords.c - wordlist[] */
#define	VCD_FTYPE \
	NULL,"integer","wire","wire","wire","wire","wire","integer",\
	"wire","wire","wire","wire","integer","integer","wire","wire","wire","wire",\
	NULL,NULL,NULL,"event","event",\
	NULL,"integer","wire","wire","wire","wire",NULL,NULL,	/* ALIAS ARITH GATE GATEX INV_GATE INV_GATEX */\
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,\
	NULL,NULL,NULL,"event","event",				/* ALIAS CLCKL TIMRL */

/* list of SAV types for GTKWAVE - 24 is decimal integer, 28 is bit 68 is inverted bit (for inverting ALIAS)*/
/* see gtkwave-3.3.43/src/analyzer.h - TR_INVERT=0x40 TR_JUSTIFY=0x20 TR_BIN=0x08 TR_DEC=0x04 TR_HEX=0x02 */
#define	SAV_FTYPE \
	NULL,"24","28","28","28","28","28","24",\
	"28","28","28","28","24","24","28","28","28","28",\
	NULL,NULL,NULL,"28","28",\
	NULL,"24","28","28","68","68",NULL,NULL,		/* ALIAS ARITH GATE GATEX INV_GATE INV_GATEX */\
	NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,\
	NULL,NULL,NULL,"28","28",				/* ALIAS CLCKL TIMRL */

/* iC_Functp2		initAct[] = {		// called in pass4 */
#define INIT_ACT \
	iC_err_fn, iC_arithMa, iC_gateMa, iC_gateMa, iC_link_cl, iC_link_cl, iC_link_cl, iC_dMsh, \
	iC_chMbit, iC_link_cl, iC_link_cl, iC_link_cl, iC_chMar, iC_link_cl, iC_link_cl, iC_link_cl, iC_link_cl, iC_link_cl, \
	iC_err_fn, iC_outMw, iC_outMx, iC_err_fn, iC_err_fn, iC_err_fn,

/* iC_Functp2		masterAct[] = {		// called in scan, scan_ar and pass4 */
#define MASTER_ACT \
	iC_err_fn, iC_arithMa, iC_gateMa, iC_gateMa, iC_riMbit, iC_sMsh, iC_rMsh, iC_dMsh, \
	iC_chMbit, iC_sMff, iC_rMff, iC_dMff, iC_chMar, iC_fMsw, iC_fMcf, iC_fMce, iC_fMfn, iC_fMfn, \
	iC_err_fn, iC_outMw, iC_outMx, iC_err_fn, iC_err_fn, iC_err_fn,

/* iC_Functp2		slaveAct[] = {		// called in scan_clk */
#define SLAVE_ACT \
	iC_err_fn, iC_err_fn, iC_err_fn, iC_err_fn, iC_riSbit, iC_sSsh, iC_rSsh, iC_dSsh, \
	iC_chSbit, iC_sSff, iC_rSff, iC_dSff, iC_chSar, iC_fSsw, iC_fScf, iC_fScf, iC_clockSfn, iC_timerSfn, \
	iC_err_fn, iC_err_fn, iC_err_fn, iC_err_fn, iC_err_fn, iC_err_fn,

/* iC_Functp		init2[] = {		// called in pass2 */
#define INIT2_ACT \
	iC_null1, iC_gate2, iC_gate2, iC_gate2, iC_i_ff2, iC_i_ff2, iC_i_ff2, iC_i_ff2, \
	iC_i_ff2, iC_i_ff2, iC_i_ff2, iC_i_ff2, iC_i_ff2, iC_null1, iC_null1, iC_null1, iC_i_ff2, iC_i_ff2, \
	iC_null1, iC_null1, iC_null1, iC_null1, iC_null1, iC_null1,

/* unsigned int	iC_bit2[] = {			// used in i_ff2() and i_ff3() */
#define BIT2_LST \
	0, INPT_M, INPT_M, INPT_M, RI_B_M, S_SH_M, R_SH_M, D_SH_M, \
	CH_B_M, S_FF_M, R_FF_M, D_FF_M, CH_B_M, F_CW_M, F_CF_M, F_CF_M, CLCK_M, TIMR_M, \
	0, OUTP_M, 0, 0, 0, 0,	/* TRAB and OUTX ==> 0 jw 040417 */

#define	FOPS	"UA _EsrHVSRDvIFGCTBWX:!e"	/* DEBUG display of ftypes - line up with pplstfix and iClive */

/* types corresponding to ftypes */
#define	TYPES \
	UDF, ARN, OR, OR, EF, SH, SH, SH, \
	VF, FF, FF, FF, VF, SW, CF, CF, CLK, TIM, \
	INPX, ARN, AND, ERR, ERR, ERR,

/* C types corresponding to ftypes */
#define	CTYPES \
	UDF, ARNC, LOGC, LOGC, ERR, ERR, ERR, ERR, ERR,\
	ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, \
	ERR, ARNC, LOGC,ERR, ERR, ERR,

/* compiler tokens corresponding to ftype */
#define	DEF_ACT \
	UNDEF, AVAR, LVAR, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, AOUT, LOUT, CVAR, TVAR, YYERRCODE,

#if INT_MAX == 32767
#define VAR_USE		0x8000		/* largest value generating unsigned short */
#define VAR_ASSIGN	0x4000		/* limits small model to 31 C functions !!! */
#define USE_MASK	0xc000		/* (VAR_USE|VAR_ASSIGN) */
#define VAR_MASK	0x3fff		/* (VAR_ASSIGN-1) is a positive value */
#define USE_OFFSET	14
#define VAL_MASK	0x1ff		/* index of arithmetic values (511 max) */
#define FUN_OFFSET	9
#define FUN_MAX		0x1f		/* (VAR_MASK>>FUN_OFFET) = 31 */
#else	/* INT_MAX != 32767 */
#define VAR_USE		0x40000000	/* largest value generating signed int */
#define VAR_ASSIGN	0x20000000	/* 8191 C functions !!! */
#define USE_MASK	0x60000000	/* (VAR_USE|VAR_ASSIGN) */
#define VAR_MASK	0x1fffffff	/* (VAR_ASSIGN-1) is a positive value */
#define USE_OFFSET	29
#define VAL_MASK	0xffff		/* index of arithmetic values (65535 max) */
#define FUN_OFFSET	16
#define FUN_MAX		0x1fff		/* (VAR_MASK>>FUN_OFFET) = 8191 */
#endif	/* INT_MAX == 32767 */
#define USE_COUNT	(sizeof(unsigned int)*4)
#define USE_TEXT	"??","="," v","=v",	/* must be 4 entries */

#define YYERRCODE	256		/* defined in comp.c but not exported */
					/* constant "iConst" Symbol is installed in S.T. */
extern Gate		iConst;		/* defined in rsff.c */

/* cMacro names generated by gram.y for ARITH and GATE values and assignments */
#define CMACRO_NAMES \
/* 0  ERR  */	"iC_ERR(",\
/* 1 ARITH */	"iC_AV(",\
/* 2 GATE  */	"iC_LV(",\
/* 3  asgn */	"iC_AA(",\
/* 4       */	"iC_LA(",\
/* 5 array */	"iC_AVI(",\
/* 6       */	"iC_LVI(",\
/* 7       */	"iC_AAI(",\
/* 8       */	"iC_LAI(",\
/* 9 size  */	"iC_SIZ(",\
/* 10 literal */"iC_AVL(",\
/* 11      */	"iC_LVL(",\
/* 12      */	"iC_AAL(",\
/* 13      */	"iC_LAL(",\
/* 14      */	"iC_AVIL(",\
/* 15      */	"iC_LVIL(",\
/* 16      */	"iC_AAIL(",\
/* 17      */	"iC_LAIL(",\
/* 18 size */	"iC_SIZL(",
#define CMACRO_ASSIGN	2
#define CMACRO_INDEX	4
#define CMACRO_LITERAL	9
#define CMACRO_SIZE	9

extern char *		iC_full_type[];	/* { FULL_TYPE } */
extern char *		iC_full_ftype[];/* { FULL_FTYPE } */
extern unsigned char	iC_types[];	/* { TYPES } */
extern unsigned char	iC_ctypes[];	/* { CTYPES } */
extern unsigned char	iC_ftypes[];	/* { FTYPES } */
extern char		iC_os[];	/* OPS */
extern char		iC_fos[];	/* FOPS */
extern char *		iC_useText[4];	/* USE_TEXT */
extern FILE *		iC_outFP;	/* output file pointer */
extern FILE *		iC_errFP;	/* error file pointer */
extern short		iC_debug;	/* from -do argument in call to main */
extern void *		iC_emalloc(unsigned);	/* check return from malloc */

typedef void		(*iC_Functp2)(Gate *, Gate *);
typedef void		(*iC_Functp)(Gate *, int);
#if INT_MAX == 32767 && defined (LONG16)
typedef long		(*iC_CFunctp)(Gate *);	/* external C functions */
#else
typedef int		(*iC_CFunctp)(Gate *);	/* external C functions */
#endif

extern void	iC_link_ol(		/* link a gate block into */
		Gate * gp, Gate * out_list);	/* an output or clock list */

extern void	iC_link_cl(		/* link clocked Gate directly to c_list */
		Gate * gp, Gate * out_list);	/* during pass4 initialisation */


extern void	iC_icc(
		Gate ** sTable, Gate ** sTend);	/* initialise and execute */

extern unsigned short	iC_osc_max;
extern unsigned short	iC_osc_lim;
extern unsigned short	iC_osc_flag;

#define USE_ALIAS	01
#define USE_STRICT	02
#define MAXUSETYPE	2
#define USESTACKSZ	32		/* nested includes - should do */

extern unsigned int	iC_uses;	/* 01=alias 02=strict as bit field */
extern unsigned int	iC_useStack[USESTACKSZ];
extern unsigned int	iC_useStackIndex;

#define iC_Aflag (iC_uses & USE_ALIAS)	/* -A alias - generate ARITH alias nodes */
#define iC_Sflag (iC_uses & USE_STRICT)	/* -S strict - all imm variables must be declared */

extern unsigned short	iC_aflag;	/* -a flag signals append mode */
extern unsigned short	iC_lflag;	/* -a build new aux files */

					/*   icc.c   */
extern char *		iC_progname;	/* name of this executable */
extern unsigned short	iC_xflag;	/* -x flag signals hexadecimal output */
extern unsigned short	iFlag;		/* inversion correction needed */
extern int		iC_inversionCorrection(void);
extern int		iC_maxIO;	/* I/O index limited to 64 or -1 no limit */
extern unsigned char	iC_QX_[];	/* Output bit field slots */
extern unsigned short	iC_optimise;	/* optimisation levels 0 - 7 */
#if ! defined(TCP) && ! defined(LOAD)	/* OLD I/O */
extern Gate *		iC_TX_[];	/* pointers to bit System Gates */
extern Gate *		iC_IX_[];	/* pointers to bit Input Gates */
extern Gate *		iC_IB_[];	/* pointers to byte Input Gates */
extern Gate *		iC_IW_[];	/* pointers to word Input Gates */
#if INT_MAX != 32767 || defined (LONG16)
extern Gate *		iC_IL_[];	/* pointers to long Input Gates */
#endif	/* INT_MAX != 32767 || defined (LONG16) */
extern unsigned char	iC_QM_[];	/* Output slot mask per cage */
extern unsigned char	iC_QMM;		/* Output cage mask for 1 rack */
extern unsigned char	iC_pdata[];	/* rest used only locally */
#else					/* NEW I/O */
extern Gate *		iC_TX0p;	/* pointer to bit System Gates */
#endif					/* END NEW I/O */
#ifndef	EFENCE
extern char		iC_outBuf[];
#else	/* EFENCE */
extern char *		iC_outBuf;
#endif	/* EFENCE */
extern int		iC_outOffset;

extern Gate *		iC_a_list;
extern Gate *		iC_o_list;
extern Gate *		iC_c_list;
extern Gate *		iC_f_list;
extern Gate *		iC_s_list;	/* send bit and byte outputs */

extern unsigned int	iC_bit2[];
extern Gate *		iC_gx;		/* points to action Gate in chMbit and riMbit */
#if YYDEBUG && !defined(_WINDOWS)
extern short		iC_dc;		/* debug display counter in scan and rsff */
#endif
extern unsigned char	iC_bitMask[];
#define	X_MASK		0xff		/* mask to detect used bits in bit I/O byte */
#define	B_WIDTH		257		/* marks output as byte width signed */
#define	W_WIDTH		258		/* marks output as word width signed */
#if INT_MAX != 32767 || defined (LONG16)
#define	L_WIDTH		259		/* marks output as long width signed */
#define	H_WIDTH		260		/* marks output as long long width signed */
#endif

extern void	iC_arithMa(Gate *, Gate *);	/* ARITH master action */
extern void	iC_sMff(Gate *, Gate *);	/* S_FF master action on FF */
extern void	iC_rMff(Gate *, Gate *);	/* R_FF master action on FF */
extern void	iC_dMff(Gate *, Gate *);	/* D_FF master action on FF */
extern void	iC_sMsh(Gate *, Gate *);	/* S_SH master action on SH */
extern void	iC_rMsh(Gate *, Gate *);	/* R_SH master action on SH */
extern void	iC_dMsh(Gate *, Gate *);	/* D_SH master action on SH */
extern void	iC_riMbit(Gate *, Gate *);	/* RI_BIT master action on EF */
extern void	iC_chMbit(Gate *, Gate *);	/* CH_BIT master action on VF */
extern void	iC_chMar(Gate *, Gate *);	/* CH_AR master action on VF */
extern void	iC_fMsw(Gate *, Gate *);	/* F_SW master action */
extern void	iC_fMcf(Gate *, Gate *);	/* F_CF master action */
extern void	iC_fMce(Gate *, Gate *);	/* F_CE master action */
extern void	iC_fMfn(Gate *, Gate *);	/* CLCK TIMR master action */
extern int	iC_traMb(Gate *, Gate *);	/* TRAB master action */
extern void	iC_outMw(Gate *, Gate *);	/* OUTW master action */
extern void	iC_outMx(Gate *, Gate *);	/* OUTX master action */

extern void	iC_sSff(Gate *, Gate *);	/* S_FF slave action on FF */
extern void	iC_rSff(Gate *, Gate *);	/* R_FF slave action on FF */
extern void	iC_dSff(Gate *, Gate *);	/* D_FF slave action on FF */
extern void	iC_sSsh(Gate *, Gate *);	/* S_SH slave action on SH */
extern void	iC_rSsh(Gate *, Gate *);	/* R_SH slave action on SH */
extern void	iC_dSsh(Gate *, Gate *);	/* D_SH slave action on SH */
extern void	iC_riSbit(Gate *, Gate *);	/* RI_BIT slave action on EF */
extern void	iC_chSbit(Gate *, Gate *);	/* CH_BIT slave action on VF */
extern void	iC_chSar(Gate *, Gate *);	/* CH_AR slave action on VF */
extern void	iC_fSsw(Gate *, Gate *);	/* F_SW slave action on SW */
extern void	iC_fScf(Gate *, Gate *);	/* F_CF F_CE slave action on CF */
extern void	iC_clockSfn(Gate *, Gate *);	/* CLCK slave action on CLK */
extern void	iC_timerSfn(Gate *, Gate *);	/* TIMR slave action on TIM */
extern void	iC_err_fn(Gate *, Gate *);	/* no master or slave function */

extern void	iC_null(void);		/* null function */
#define	iC_null2	(iC_Functp2)iC_null	/* null function for funct lists */

extern short	iC_error_flag;

extern void	iC_scan(Gate *);		/* scan logic action list */
extern void	iC_scan_ar(Gate *);		/* scan arithmetic action list */
extern void	iC_scan_clk(Gate *);		/* scan a clock list */
extern void	iC_scan_snd(Gate *)		/* scan send list */;
extern void	iC_pass1(Gate *, int);		/* Pass1 init on gates */
extern void	iC_pass2(Gate *, int);		/* Pass2 init on gates */
extern void	iC_gate2(Gate *, int);		/* pass2 function init gates */
extern void	iC_i_ff2(Gate *, int);		/* called via output lists */
extern void	iC_pass4(Gate *, int);		/* Pass4 init on gates */
#define	iC_null1 (iC_Functp)iC_null		/* null function for init lists */

extern iC_Functp iC_gate_i[];
extern iC_Functp iC_ff_i[];
extern iC_Functp iC_clock_i[];

#define	MARKMAX 4				/* number of oscillations allowed */
#define	OSC_WARN_CNT 10				/* number of oscillation warnings */

extern unsigned short	iC_mark_stamp;		/* incremented every scan */
extern Gate *		iC_osc_gp;		/* report oscillations */

extern unsigned	iC_scan_cnt;			/* count scan operations */
extern unsigned	iC_link_cnt;			/* count link operations */
#if YYDEBUG && (!defined(_WINDOWS) || defined(LOAD))
extern unsigned	iC_glit_cnt;			/* count glitches */
extern unsigned	long glit_nxt;			/* count glitch scan */
#endif

extern void	iC_initIO(void);		/* init signal and correct interrupt vectors */
extern void	iC_quit(int sig);		/* quit with correct interrupt vectors */

#if defined(RUN) || defined(TCP) && ! defined(LOAD)
extern Gate *		iC_pf0_1;		/* pointer to _f0_1 if generated in buildNet() */
#endif /* defined(RUN) || defined(TCP) && ! defined(LOAD) */

#if defined(RUN) || defined(TCP) || defined(LOAD)
extern Gate **		iC_list[];		/* used to load several modules */
extern int		iC_cmp_gt_ids( const Gate ** a, const Gate ** b);
typedef int (*iC_fptr)(const void*, const void*);
#endif /* RUN or TCP or LOAD */
#ifndef LOAD
#if INT_MAX == 32767 && defined (LONG16)
extern long		iC_exec(int iC_indx, Gate * gp);
#else
extern int		iC_exec(int iC_indx, Gate * gp);
#endif
#endif /* LOAD */

#if INT_MAX == 32767 && defined (LONG16)
extern void		iC_liveData(Gate * gp, long value);	/* VCD and/or iClive */
#else
extern void		iC_liveData(Gate * gp, int value);	/* VCD and/or iClive */
#endif
					/*   misc.c  */
#ifdef	WIN32
#define snprintf    _snprintf
extern int mkstemp(char * template);
#endif	/* WIN32 */

#endif	/* ICC_H */

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
 *	icc.h
 *	immediate C - runtime execution header
 *
 *******************************************************************/

#ifndef ICC_H
#define ICC_H
static const char icc_h[] =
"@(#)$Id: icc.h,v 1.58 2005/04/16 17:15:06 jw Exp $";

#include	"icg.h"

#ifdef _WINDOWS
#define	strlen(a)	lstrlen(a)
#define	strcpy(a,b)	lstrcpy(a,b)
#define	strcmp(a,b)	lstrcmp(a,b)
#define	sprintf		wsprintf
#define	calloc(a,b)	iC_emalloc((a)*(b))
#define	free(a)		iC_efree(a)
extern void		iC_efree(void *);
#endif
#if defined(RUN) || ((defined (LOAD) || defined(TCP)) && defined (YYDEBUG))
#define			MAX_IO	8
extern void		iC_display(int * dis_cntp, int dis_max);
#endif	/* RUN or ((LOAD orTCP) and YYDEBUG) */

#ifndef PPGATESIZE
#define	PPGATESIZE 127		/* natural gate size for char gt_val */
#endif
#define	INSTSIZE 3		/* limit instance to 3 characters */

#define	DIS_MAX	5		/* diplay heading after this many */
#define	IXD	64		/* number of slots in icc (<= 64) */
#define	TXD	1		/* number of system slots (timers etc) */

/*	Function types 'op' 'Symbol.type' and display */

#define	UDF	iC_UDF		/* represents undefined type */
		    	/* arithmetic functions */
#define	ARNC	iC_ARNC		/* arithmetic node in a C fragment */
#define	ARN	iC_ARN		/* arithmetic node */
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
#define	CH_BIT	iC_CH_BIT	/* CHANGE logical or arithmetic action (iClive uses CH_BIT-RI_BIT) */
#define	S_FF	iC_S_FF		/* FF set logical action   requires 1001 at genr.c 233 */
#define	R_FF	iC_R_FF		/* FF reset logical action requires 1010 */
#define	D_FF	iC_D_FF		/* FF D logical action     requires 1011 */
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

typedef struct Gate	Gate;	/* iC_Gt equivalent to Gate */

#define	MIN_GT	(LOGC+1)	/* types < MIN_GT are UDF, Arith and LOGC */
#define	MAX_GT	(LATCH+1)	/* types < MAX_GT are driven by a value */
#define	MAX_LV	(INPX+1)	/* types < MAX_LV return a logical or arith value */
#define	MAX_OP	(TIM+1)		/* types < MAX_OP are executable */
#define	MAX_LS	(ERR+1)		/* types < MAX_LS are generated */

				/* mark nodes declared as extern - stop assignment */
#define	TM	0x1f		/* mask for type from EXT_TYPES */
#define	EM	0x20		/* mask for external type from EXT_TYPES */
#define	FM	0x40		/* mask for terminal function type from FUN_TYPES */
				/* intermediate Symbols are marked by ->name == 0 */
#define EXT_ARN	(ARN | EM)
#define EXT_XOR	(XOR | EM)
#define EXT_AND	(AND | EM)
#define EXT_OR	(OR  | EM)
#define EXT_CLK	(CLK | EM)
#define EXT_TIM	(TIM | EM)

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

#define	Sizeof(x)	((sizeof x) / (sizeof x[0]))
#ifndef DEQ
#define	Out_init(ol)	(ol->gt_list = (Gate **)(ol->gt_next = ol))
#else
#define	Out_init(ol)	(ol->gt_next = ol->gt_prev = ol)
#endif

/********************************************************************
 *
 *	The following lists are indexed by type
 *
 *	UDF	ARNC	ARN	LOGC	XOR	AND	OR	LATCH
 *	SH	FF	EF	VF	SW	CF	NCONST	INPB	INPW	INPX
 *	CLK	TIM	ALIAS	ERR	KEYW	CTYPE	CWORD	IFUNCT
 *
 *******************************************************************/

/* list of types */
#define	FULL_TYPE \
	"UDF","ARNC","ARN","LOGC","XOR","AND","OR","LATCH",\
	"SH","FF","EF","VF","SW","CF","NCONST","INPB","INPW","INPX",\
	"CLK","TIM","ALIAS","ERR","KEYW","CTYPE","CWORD","IFUNCT",

/* list of iC_ extended types in icg.h */
#define	iC_EXT_TYPE \
	"iC_UDF","iC_ARNC","iC_ARN","iC_LOGC","iC_XOR","iC_AND","iC_OR","iC_LATCH",\
	"iC_SH","iC_FF","iC_EF","iC_VF","iC_SW","iC_CF","iC_NCONST","iC_INPB","iC_INPW","iC_INPX",\
	"iC_CLK","iC_TIM","iC_ALIAS","iC_ERR","iC_KEYW","iC_CTYPE","iC_CWORD","iC_IFUNCT",

#define	OPS	".-+\"^&|%*#/>({=][<:!@?ktwi"	/* DEBUG display of types */

/* ftypes corresponding to types */
#define	FTYPES \
	UDFA, ARITH, ARITH, GATE, GATE, GATE, GATE, GATE,\
	D_SH, D_FF, RI_BIT, CH_BIT, F_SW, F_CF, ARITH, OUTX, ARITH, TRAB,\
	CLCK, TIMR, GATE, GATE, 0, 0, 0, 0,

/* compiler tokens corresponding to type */
#define	DEF_TYP \
	YYERRCODE, AVARC, YYERRCODE, LVARC, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, \
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,

/* initialisation tables for different types (must have MAX_LS entries) */
#define	I_LISTS \
	iC_gate_i, iC_gate_i, iC_gate_i, iC_gate_i, iC_gate_i, iC_gate_i, iC_gate_i, iC_gate_i,\
	iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i, iC_ff_i,\
	iC_clock_i, iC_clock_i, iC_clock_i, iC_clock_i,

/********************************************************************
 *
 *	The following lists are indexed by gt_fni (is ftype)
 *
 *	UDFA	ARITH	GATE	GATEX	RI_BIT	S_SH	R_SH	D_SH
 *	CH_BIT	S_FF	R_FF	D_FF	F_SW	F_CF	F_CE	CLCK	TIMR
 *	TRAB	OUTW	OUTX	CLCKL	TIMRL
 *
 *******************************************************************/

/* list of ftypes */
#define	FULL_FTYPE \
	"UDFA","ARITH","GATE","GATEX","RI_BIT","S_SH","R_SH","D_SH",\
	"CH_BIT","S_FF","R_FF","D_FF","F_SW","F_CF","F_CE","CLCK","TIMR",\
	"TRAB","OUTW","OUTX","CLCKL","TIMRL",

/* list of iC_ extended ftypes in icg.h */
#define	iC_EXT_FTYPE \
	"iC_UDFA","iC_ARITH","iC_GATE","iC_GATEX","iC_RI_BIT","iC_S_SH","iC_R_SH","iC_D_SH",\
	"iC_CH_BIT","iC_S_FF","iC_R_FF","iC_D_FF","iC_F_SW","iC_F_CF","iC_F_CE","iC_CLCK","iC_TIMR",\
	"iC_TRAB","iC_OUTW","iC_OUTX","iC_CLCKL","iC_TIMRL",

/* iC_Functp2		initAct[] = {		// called in pass4 */
#define INIT_ACT \
	iC_err_fn, iC_arithMa, iC_link_ol, iC_link_ol, link_c, link_c, link_c, iC_dMsh, \
	iC_chMbit, link_c, link_c, link_c, link_c, link_c, link_c, link_c, link_c, \
	iC_err_fn, iC_outMw, iC_outMx, iC_err_fn, iC_err_fn,

/* iC_Functp2		masterAct[] = {		// called in scan, scan_ar and pass4 */
#define MASTER_ACT \
	iC_err_fn, iC_arithMa, iC_link_ol, iC_link_ol, iC_riMbit, iC_sMsh, iC_rMsh, iC_dMsh, \
	iC_chMbit, iC_sMff, iC_rMff, iC_dMff, iC_fMsw, iC_fMcf, iC_fMce, iC_fMfn, iC_fMfn, \
	iC_err_fn, iC_outMw, iC_outMx, iC_err_fn, iC_err_fn,

/* iC_Functp2		slaveAct[] = {		// called in scan_clk */
#define SLAVE_ACT \
	iC_err_fn, iC_err_fn, iC_err_fn, iC_err_fn, iC_riSbit, iC_sSsh, iC_rSsh, iC_dSsh, \
	iC_chSbit, iC_sSff, iC_rSff, iC_dSff, iC_fSsw, iC_fScf, iC_fScf, iC_clockSfn, iC_timerSfn, \
	iC_err_fn, iC_err_fn, iC_err_fn, iC_err_fn, iC_err_fn,

/* iC_Functp		init2[] = {		// called in pass2 */
#define INIT2_ACT \
	iC_null1, iC_gate2, iC_gate2, iC_gate2, iC_i_ff2, iC_i_ff2, iC_i_ff2, iC_i_ff2, \
	iC_i_ff2, iC_i_ff2, iC_i_ff2, iC_i_ff2, iC_null1, iC_null1, iC_null1, iC_i_ff2, iC_i_ff2, \
	iC_null1, iC_null1, iC_null1, iC_null1, iC_null1,

/* unsigned int	iC_bit2[] = {			// used in i_ff2() and i_ff3() */
#define BIT2_LST \
	0, INPT_M, INPT_M, INPT_M, RI_B_M, S_SH_M, R_SH_M, D_SH_M, \
	CH_B_M, S_FF_M, R_FF_M, D_FF_M, F_CW_M, F_CF_M, F_CF_M, CLCK_M, TIMR_M, \
	0, OUTP_M, 0, 0, 0,	/* TRAB and OUTX ==> 0 jw 040417 */

#define	FOPS	"UA _EsrHVSRDIFGCTBWX:!"	/* DEBUG display of ftypes */

/* types corresponding to ftypes */
#define	TYPES \
	UDF, ARN, OR, OR, EF, SH, SH, SH, \
	VF, FF, FF, FF, SW, CF, CF, CLK, TIM, \
	INPX, ARN, INPB, ERR, ERR

/* C types corresponding to ftypes */
#define	CTYPES \
	UDF, ARNC, LOGC, LOGC, ERR, ERR, ERR, ERR, ERR,\
	ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR, \
	ERR, ARNC, LOGC,ERR, ERR

/* compiler tokens corresponding to ftype */
#define	DEF_ACT \
	UNDEF, AVAR, LVAR, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, AOUT, LOUT, CVAR, TVAR

#define YYERRCODE	256	/* defined in comp.c but not exported */

/* name for constant 'iConst' Symbol (installed in S.T.) */
#define ICONST	"iConst"

/* macro names generated by gram.y for ARITH and GATE values and assignments */
#define MACRO_NAMES "#error \"iC generate error\" (",\
	"iC_AV(","iC_LV(","iC_AA(","iC_LA(","iC_AVL(","iC_LVL(","iC_AAL(","iC_LAL(",
#define MACRO_OFFSET	2
#define MACRO_LITERAL	4

extern char *		iC_full_type[];	/* { FULL_TYPE } */
extern char *		iC_full_ftype[];/* { FULL_FTYPE } */
extern unsigned char	iC_types[];	/* { TYPES } */
extern unsigned char	iC_ctypes[];	/* { CTYPES } */
extern unsigned char	iC_ftypes[];	/* { FTYPES } */
extern char		iC_os[];	/* OPS */
extern char		iC_fos[];	/* FOPS */
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
		Gate *, Gate *);	/* an output or clock list */

extern void	iC_icc(			/* initialise and execute */
		Gate * g_lists, unsigned gate_count[]);

extern unsigned short	iC_osc_max;
extern unsigned short	iC_osc_lim;
extern unsigned short	iC_Aflag;	/* -A flag signals ARITH alias */
extern unsigned short	iC_Sflag;	/* strict - all imm variables must be declared */
extern unsigned short *	iC_useTypes[];
#define USETYPEFLAGS	&iC_Aflag, &iC_Sflag
#define MAXUSETYPE	2
extern unsigned short	iC_aflag;	/* -a flag signals append mode */
extern unsigned short	iC_lflag;	/* -l flag signals append mode */
extern unsigned short	iC_Tflag;	/* define iC_tVar */

					/*   icc.c   */
extern char *		iC_progname;	/* name of this executable */
extern unsigned short	iC_xflag;	/* -x flag signals hexadecimal output */
extern unsigned short	iFlag;		/* inversion correction needed */
extern int		iC_inversionCorrection(void);

#ifndef	LOAD	// OLD I/O
extern int		iC_maxIO;	/* I/O index limited to 64 or -1 no limit */
extern Gate *		iC_TX_[];	/* pointers to bit System Gates */
extern Gate *		iC_IX_[];	/* pointers to bit Input Gates */
extern Gate *		iC_IB_[];	/* pointers to byte Input Gates */
extern Gate *		iC_IW_[];	/* pointers to word Input Gates */
#if INT_MAX != 32767 || defined (LONG16)
extern Gate *		iC_IL_[];	/* pointers to long Input Gates */
#endif	/* INT_MAX != 32767 || defined (LONG16) */
extern unsigned char	iC_QX_[];	/* Output bit field slots */
extern char		iC_QT_[];	/* Output type of slots */
extern unsigned char	iC_QM_[];	/* Output slot mask per cage */
extern unsigned char	iC_QMM;		/* Output cage mask for 1 rack */
extern unsigned char	iC_idata[];	/* Input bit field */
extern unsigned char	iC_pdata[];	/* rest used only locally */
extern iC_Functp *	iC_i_lists[];
#else		// NEW I/O
extern Gate *		iC_TX0p;	/* pointer to bit System Gates */
extern Gate *		iC_IX0p;	/* pointer to Input Gate for bit iC_display */
extern Gate *		iC_QX0p;	/* pointer to Output Gate for bit iC_display */
extern Gate *		iC_IB1p;	/* pointer to Input Gate for byte iC_display */
extern Gate *		iC_QB1p;	/* pointer to Output Gate for byte iC_display */
extern Gate *		iC_IW2p;	/* pointer to Input Gate for word iC_display */
extern Gate *		iC_QW2p;	/* pointer to Output Gate for word iC_display */
#if	INT_MAX != 32767 || defined (LONG16)
extern Gate *		iC_IL4p;	/* pointer to Input Gate for long iC_display */
extern Gate *		iC_QL4p;	/* pointer to Output Gate for long iC_display */
#endif	/* INT_MAX != 32767 || defined (LONG16) */
#ifndef	EFENCE
extern char		iC_outBuf[];
#else	/* EFENCE */
extern char *		iC_outBuf;
#endif	/* EFENCE */
extern int		iC_outOffset;
#endif		// END NEW I/O

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
extern unsigned char	iC_bitIndex[];
#define	X_MASK		0xff		/* mask to detect used bits in bit I/O byte */
#define	B_WIDTH		257		/* marks output as byte width signed */
#define	W_WIDTH		258		/* marks output as word width signed */
#if INT_MAX != 32767 || defined (LONG16)
#define	L_WIDTH		259		/* marks output as long width signed */
#define	H_WIDTH		260		/* marks output as long long width signed */
#endif

extern void	iC_sMff(Gate *, Gate *);	/* S_FF master action on FF */
extern void	iC_rMff(Gate *, Gate *);	/* R_FF master action on FF */
extern void	iC_dMff(Gate *, Gate *);	/* D_FF master action on FF */
extern void	iC_sMsh(Gate *, Gate *);	/* S_SH master action on SH */
extern void	iC_rMsh(Gate *, Gate *);	/* R_SH master action on SH */
extern void	iC_dMsh(Gate *, Gate *);	/* D_SH master action on SH */
extern void	iC_chMbit(Gate *, Gate *);	/* CH_BIT master action on VF */
extern void	iC_riMbit(Gate *, Gate *);	/* RI_BIT master action on EF */
extern void	iC_fMsw(Gate *, Gate *);	/* F_SW master action */
extern void	iC_fMcf(Gate *, Gate *);	/* F_CF master action */
extern void	iC_fMce(Gate *, Gate *);	/* F_CE master action */
extern void	iC_fMfn(Gate *, Gate *);	/* CLCK TIMR master action */
extern int	iC_traMb(Gate *, Gate *);	/* TRAB master action */
extern void	iC_outMw(Gate *, Gate *);	/* OUTW master action */
extern void	iC_outMx(Gate *, Gate *);	/* OUTX master action */
extern void	iC_arithMa(Gate *, Gate *);	/* ARITH master action */

extern void	iC_sSff(Gate *, Gate *);	/* S_FF slave action on FF */
extern void	iC_rSff(Gate *, Gate *);	/* R_FF slave action on FF */
extern void	iC_dSff(Gate *, Gate *);	/* D_FF slave action on FF */
extern void	iC_sSsh(Gate *, Gate *);	/* S_SH slave action on SH */
extern void	iC_rSsh(Gate *, Gate *);	/* R_SH slave action on SH */
extern void	iC_dSsh(Gate *, Gate *);	/* D_SH slave action on SH */
extern void	iC_chSbit(Gate *, Gate *);	/* CH_BIT slave action on VF */
extern void	iC_riSbit(Gate *, Gate *);	/* RI_BIT slave action on EF */
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

#define	MARKMAX 3				/* number of oscillations allowed */
						/* should be odd so that a transition */
						/* is seen in first cycle */
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

#ifdef LOAD
extern Gate **		iC_sTable;		/* pointer to dynamic array */
extern Gate **		iC_sTend;		/* end of dynamic array */
extern unsigned long	iC_sTstrLen;		/* length of symbol strings */
extern Gate **		iC_list[];		/* used to load several modules */
#else
#if INT_MAX == 32767 && defined (LONG16)
extern long		iC_exec(int iC_index, Gate * gp);
#else
extern int		iC_exec(int iC_index, Gate * gp);
#endif
#endif

#if INT_MAX == 32767 && defined (LONG16)
extern void		iC_liveData(unsigned short index, long value);
#else
extern void		iC_liveData(unsigned short index, int value);
#endif
#endif	/* ICC_H */

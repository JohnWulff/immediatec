static const char icc_h[] =
"@(#)$Id: icc.h,v 1.39 2002/07/29 11:07:45 jw Exp $";
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
 *	icc.h
 *	parallel plc - runtime execution header
 *
 *******************************************************************/

/* J.E. Wulff	3-Mar-85 */

#ifdef _WINDOWS
#define	strlen(a)	lstrlen(a)
#define	strcpy(a,b)	lstrcpy(a,b)
#define	strcmp(a,b)	lstrcmp(a,b)
#define	sprintf		wsprintf
#define	calloc(a,b)	emalloc((a)*(b))
#define	free(a)		efree(a)
extern void	efree(void *);
#endif

#ifndef PPGATESIZE
#define	PPGATESIZE 127		/* natural gate size for char val */
#endif

#define	DIS_MAX	5		/* diplay heading after this many */
#define	IXD	64		/* number of slots in icc (<= 64) */
#define	TXD	1		/* number of system slots (timers etc) */

#define	Sizeof(x)	((sizeof x) / (sizeof x[0]))
#ifndef DEQ
#define	Out_init(ol)	(ol->gt_list = (Gate **)(ol->gt_next = ol))
#else
#define	Out_init(ol)	(ol->gt_next = ol->gt_prev = ol)
#endif

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

/*	action function types symbol.ftype and Gate.gt_fni */

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

/* list of types */
#define	FULL_TYPE "UDF","ARNC","ARN","LOGC","AND","OR","LATCH",\
	"SH","FF","VF","EF","SW","CF","NCONST","INPW","INPX",\
	"CLK","TIM","ALIAS","ERR","KEYW","CTYPE","CWORD"

#define	OPS	".-+\"&|%*#^/({=[<:!@?;tw"	/* DEBUG display of types */

/* ftypes corresponding to types */
#define	FTYPES	UDFA, ARITH, ARITH, GATE, GATE, GATE, GATE,\
	D_SH, D_FF, CH_BIT, RI_BIT, F_SW, F_CF, ARITH, ARITH, GATE,\
	CLCK, TIMR, GATE, GATE, 0, 0, 0

/* compiler tokens corresponding to type */
#define	DEF_TYP	YYERRCODE, AVARC, YYERRCODE, LVARC, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, NVAR,\
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE,

/* initialisation tables for different types (must have MAX_LS entries) */
#define	I_LISTS	gate_i, gate_i, gate_i, gate_i, gate_i, gate_i, gate_i,\
	ff_i, ff_i, ff_i, ff_i, ff_i, ff_i, ff_i, ff_i, ff_i,\
	clock_i, clock_i, clock_i, clock_i,

/* list of ftypes */
#define	FULL_FTYPE "UDFA","ARITH","GATE","RI_BIT","CH_BIT","S_SH","R_SH","D_SH",\
	"F_SW","S_FF","R_FF","D_FF","F_CF","F_CE","CLCK","TIMR","OUTW","OUTX",\
	"CLCKL","TIMRL"

#define	FOPS	"UA EVsrHISRDFGCTWX:!"	/* DEBUG display of ftypes */

/* types corresponding to ftypes */
#define	TYPES	UDF, ARN, OR, EF, VF, SH, SH, SH, SW,\
	FF, FF, FF, CF, CF, CLK, TIM, ARN, AND,\
	ERR, ERR

/* compiler tokens corresponding to ftype */
#define	DEF_ACT	UNDEF, AVAR, LVAR, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, AOUT, LOUT,\
	CVAR, TVAR

extern char *		full_type[];
extern char *		full_ftype[];
extern unsigned char	types[];	/*   comp.y   */
extern unsigned char	ftypes[];
extern char		os[];
extern char		fos[];
extern FILE *		inFP;		/* input file pointer */
extern FILE *		outFP;		/* output file pointer */
extern FILE *		errFP;		/* error file pointer */
extern short		debug;		/* from -do argument in call to main */

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

typedef void		(*Functp2)(Gate *, Gate *);
typedef void		(*Functp)(Gate *, int);
typedef int		(*CFunctp)(Gate *);	/* external C functions */

extern void	link_ol(		/* link a gate block into */
		Gate *, Gate *);	/* an output or clock list */

extern void	icc(			/* initialise and execute */
		Gate *, unsigned *);	/*   g_lists, gate_count   */

extern unsigned short	osc_max;
extern unsigned short	Aflag;		/* -A flag signals ARITH alias */
extern unsigned short	aflag;		/* -a flag signals append mode */

					/*   main.c   */
extern const char *	progname;	/* name of this executable */
extern unsigned short	xflag;		/* -x flag signals hexadecimal output */
extern unsigned short	iFlag;		/* inversion correction needed */
extern int		inversionCorrection(void);

extern Gate		iClock;		/* System clock */
extern Gate *		IX_[];		/* pointers to Bit Input Gates */
extern Gate *		IB_[];		/* pointers to Byte Input Gates */
extern Gate *		IW_[];		/* pointers to Word Input Gates */
extern Gate *		TX_[];		/* pointers to System Bit Gates */
extern unsigned char	QX_[];		/* Output bit field slots */
extern char		QT_[IXD];	/* Output type of slots */
extern unsigned char	QM_[];		/* Output slot mask per cage */
extern unsigned char	QMM;		/* Output cage mask for 1 rack */
extern unsigned char	idata[];	/* Input bit field */
extern unsigned char	pdata[];	/* rest used only locally */
extern Functp *		i_lists[];

extern Gate *		a_list;
extern Gate *		o_list;
extern Gate *		c_list;

extern unsigned int	bit2[];
extern Gate *		gx;	/* points to action Gate in chMbit and riMbit */
extern short		dc;	/* debug display counter in scan and rsff */
extern unsigned char	bitMask[];
extern unsigned char	bitIndex[];
#define	B_WIDTH		257		/* marks output as Byte width */
#define	W_WIDTH		258		/* marks output as Word width */

extern void	sMff(Gate *, Gate *);	/* S_FF master action on FF */
extern void	rMff(Gate *, Gate *);	/* R_FF master action on FF */
extern void	dMff(Gate *, Gate *);	/* D_FF master action on FF */
extern void	sMsh(Gate *, Gate *);	/* S_SH master action on SH */
extern void	rMsh(Gate *, Gate *);	/* R_SH master action on SH */
extern void	dMsh(Gate *, Gate *);	/* D_SH master action on SH */
extern void	chMbit(Gate *, Gate *);	/* CH_BIT master action on VF */
extern void	riMbit(Gate *, Gate *);	/* RI_BIT master action on EF */
extern void	fMsw(Gate *, Gate *);	/* F_SW master action */
extern void	fMcf(Gate *, Gate *);	/* F_CF master action */
extern void	fMce(Gate *, Gate *);	/* F_CE master action */
extern void	fMfn(Gate *, Gate *);	/* CLCK TIMR master action */
extern void	outMw(Gate *, Gate *);	/* OUTW master action */
extern void	outMx(Gate *, Gate *);	/* OUTX master action */
extern void	arithMa(Gate *, Gate *);/* ARITH master action */

extern void	sSff(Gate *, Gate *);	/* S_FF slave action on FF */
extern void	rSff(Gate *, Gate *);	/* R_FF slave action on FF */
extern void	dSff(Gate *, Gate *);	/* D_FF slave action on FF */
extern void	sSsh(Gate *, Gate *);	/* S_SH slave action on SH */
extern void	rSsh(Gate *, Gate *);	/* R_SH slave action on SH */
extern void	dSsh(Gate *, Gate *);	/* D_SH slave action on SH */
extern void	chSbit(Gate *, Gate *);	/* CH_BIT slave action on VF */
extern void	riSbit(Gate *, Gate *);	/* RI_BIT slave action on EF */
extern void	fSsw(Gate *, Gate *);	/* F_SW slave action on SW */
extern void	fScf(Gate *, Gate *);	/* F_CF F_CE slave action on CF */
extern void	clockSfn(Gate *, Gate *);/* CLCK slave action on CLK */
extern void	timerSfn(Gate *, Gate *);/* TIMR slave action on TIM */
extern void	err_fn(Gate *, Gate *);	/* no master or slave function */

extern void	null(void);		/* null function */
#define	null2	(Functp2)null		/* null function for funct lists */

extern short	error_flag;

extern int	scan(Gate *);		/* scan logic action list */
extern int	scan_ar(Gate *);	/* scan arithmetic action list */
extern int	scan_clk(Gate *);	/* scan a clock list */
extern void	pass1(Gate *, int);	/* Pass1 init on gates */
extern void	pass2(Gate *, int);	/* Pass2 init on gates */
extern void	gate2(Gate *, int);	/* pass2 function init gates */
extern void	i_ff2(Gate *, int);	/* called via output lists */
extern void	pass4(Gate *, int);	/* Pass4 init on gates */
#define	null1	(Functp)null		/* null function for init lists */

extern	Functp	gate_i[];
extern	Functp	ff_i[];
extern	Functp	clock_i[];

#define	MARKMAX 3		/* number of oscillations allowed */
				/* should be odd so that a transition */
				/* is seen in first cycle */

/* parallel plc */
/* parallel plc */
extern unsigned short	mark_stamp;	/* incremented every scan */

#if !defined(_WINDOWS) || defined(LOAD)
extern unsigned	scan_cnt;			/* count scan operations */
extern unsigned	link_cnt;			/* count link operations */
extern unsigned	glit_cnt;			/* count glitches */
extern unsigned	long glit_nxt;			/* count glitch scan */
#endif

#ifndef _WINDOWS 
extern void	quit(int sig);	/* quit with correct interrupt vectors */
#endif

#ifdef LOAD
extern Gate **		sTable;			/* pointer to dynamic array */
extern Gate **		sTend;			/* end of dynamic array */
extern unsigned long	sTstrLen;		/* length of symbol strings */
extern Gate **		i_list[];		/* used to load several modules */
#else
extern int		c_exec(int pp_index, Gate * gp);
#endif
extern int		assign(Gate * lv, int rv);

extern void		liveData(unsigned short index, int value);

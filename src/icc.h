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
 *	immediate C - runtime execution header
 *
 *******************************************************************/

#ifndef ICC_H
#define ICC_H
static const char icc_h[] =
"@(#)$Id: icc.h,v 1.53 2004/03/15 16:00:14 jw Exp $";

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
#define	PPGATESIZE 127		/* natural gate size for char gt_val */
#endif

#define	DIS_MAX	5		/* diplay heading after this many */
#define	IXD	64		/* number of slots in icc (<= 64) */
#define	TXD	1		/* number of system slots (timers etc) */

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

/* list of types */
#define	FULL_TYPE "UDF","ARNC","ARN","LOGC","AND","OR","LATCH",\
	"SH","FF","VF","EF","SW","CF","NCONST","INPW","INPX",\
	"CLK","TIM","ALIAS","ERR","KEYW","CTYPE","CWORD","IFUNCT",

#define	OPS	".-+\"&|%*#^/({=[<:!@?;twi"	/* DEBUG display of types */

/* ftypes corresponding to types */
#define	FTYPES	UDFA, ARITH, ARITH, GATE, GATE, GATE, GATE,\
	D_SH, D_FF, CH_BIT, RI_BIT, F_SW, F_CF, ARITH, ARITH, GATE,\
	CLCK, TIMR, GATE, GATE, 0, 0, 0, 0, 0,

/* compiler tokens corresponding to type */
#define	DEF_TYP	YYERRCODE, AVARC, YYERRCODE, LVARC, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,

/* initialisation tables for different types (must have MAX_LS entries) */
#define	I_LISTS	gate_i, gate_i, gate_i, gate_i, gate_i, gate_i, gate_i,\
	ff_i, ff_i, ff_i, ff_i, ff_i, ff_i, ff_i, ff_i, ff_i,\
	clock_i, clock_i, clock_i, clock_i,

/* list of ftypes */
#define	FULL_FTYPE "UDFA","ARITH","GATE","RI_BIT","CH_BIT","S_SH","R_SH","D_SH",\
	"F_SW","S_FF","R_FF","D_FF","F_CF","F_CE","CLCK","TIMR","OUTW","OUTX",\
	"CLCKL","TIMRL",

/* macro names generated by gram.y for ARITH and GATE values and assignments */
#define MACRO_NAMES "#error \"iC generate error\" (",\
	"_AV(","_LV(","_AA(","_LA(","_AVL(","_LVL(","_AAL(","_LAL(",
#define MACRO_OFFSET	2
#define MACRO_LITERAL	4

#define	FOPS	"UA EVsrHISRDFGCTWX:!"		/* DEBUG display of ftypes */

/* types corresponding to ftypes */
#define	TYPES	UDF, ARN, OR, EF, VF, SH, SH, SH, SW,\
	FF, FF, FF, CF, CF, CLK, TIM, ARN, AND,\
	ERR, ERR

/* C types corresponding to ftypes */
#define	CTYPES	UDF, ARNC, LOGC, ERR, ERR, ERR, ERR, ERR, ERR,\
	ERR, ERR, ERR, ERR, ERR, ERR, ERR, ARNC, LOGC,\
	ERR, ERR

/* compiler tokens corresponding to ftype */
#define	DEF_ACT	UNDEF, AVAR, LVAR, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, AOUT, LOUT,\
	CVAR, TVAR

#define YYERRCODE	256		/* defined in comp.c but not exported */

extern char *		full_type[];
extern char *		full_ftype[];
extern unsigned char	types[];
extern unsigned char	ctypes[];
extern unsigned char	ftypes[];
extern char		os[];
extern char		fos[];
extern FILE *		outFP;		/* output file pointer */
extern FILE *		errFP;		/* error file pointer */
extern short		debug;		/* from -do argument in call to main */
extern void *	emalloc(unsigned);	/* check return from malloc */

typedef void		(*Functp2)(Gate *, Gate *);
typedef void		(*Functp)(Gate *, int);
#if INT_MAX == 32767 && defined (LONG16)
typedef long		(*CFunctp)(Gate *);	/* external C functions */
#else
typedef int		(*CFunctp)(Gate *);	/* external C functions */
#endif

extern void	link_ol(		/* link a gate block into */
		Gate *, Gate *);	/* an output or clock list */

extern void	icc(			/* initialise and execute */
		Gate *, unsigned *);	/*   g_lists, gate_count   */

extern unsigned short	osc_max;
extern unsigned short	Aflag;		/* -A flag signals ARITH alias */
extern unsigned short	aflag;		/* -a flag signals append mode */
extern unsigned short	Tflag;		/* define _tVar in _list_.c _list1.h */

					/*   main.c   */
extern const char *	progname;	/* name of this executable */
extern unsigned short	xflag;		/* -x flag signals hexadecimal output */
extern unsigned short	iFlag;		/* inversion correction needed */
extern int		inversionCorrection(void);

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
extern Gate *		f_list;

extern unsigned int	bit2[];
extern Gate *		gx;	/* points to action Gate in chMbit and riMbit */
#if YYDEBUG && !defined(_WINDOWS)
extern short		dc;	/* debug display counter in scan and rsff */
#endif
extern unsigned char	bitMask[];
extern unsigned char	bitIndex[];
#define	B_WIDTH		257		/* marks output as byte width */
#define	W_WIDTH		258		/* marks output as word width */
#if INT_MAX != 32767 || defined (LONG16)
#define	L_WIDTH		259		/* marks output as long width */
#endif

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
#define	OSC_WARN_CNT 10		/* number of oscillation warnings */

extern unsigned short	mark_stamp;		/* incremented every scan */
extern Gate *		osc_gp;			/* report oscillations */

extern unsigned	scan_cnt;			/* count scan operations */
extern unsigned	link_cnt;			/* count link operations */
#if YYDEBUG && (!defined(_WINDOWS) || defined(LOAD))
extern unsigned	glit_cnt;			/* count glitches */
extern unsigned	long glit_nxt;			/* count glitch scan */
#endif

extern void	initIO(void);	/* init signal and correct interrupt vectors */
extern void	quit(int sig);	/* quit with correct interrupt vectors */

#ifdef LOAD
extern Gate **		sTable;			/* pointer to dynamic array */
extern Gate **		sTend;			/* end of dynamic array */
extern unsigned long	sTstrLen;		/* length of symbol strings */
extern Gate **		i_list[];		/* used to load several modules */
#else
#if INT_MAX == 32767 && defined (LONG16)
extern long		c_exec(int pp_index, Gate * gp);
#else
extern int		c_exec(int pp_index, Gate * gp);
#endif
#endif

#if INT_MAX == 32767 && defined (LONG16)
extern void		liveData(unsigned short index, long value);
#else
extern void		liveData(unsigned short index, int value);
#endif
#endif	/* ICC_H */

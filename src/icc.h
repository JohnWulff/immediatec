/********************************************************************
 *
 *	Copyright (C) 1985-2017  John E. Wulff
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
"@(#)$Id: icc.h 1.88 $";

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

#include	<signal.h>
#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */
#include	<linux/gpio.h>
#include	<sys/ioctl.h>
#include	<stdlib.h>
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
#ifndef INT_MAX
#include	<limits.h>
#endif	/* INT_MAX */

#ifndef	max
#define	max(x, y) ((x) > (y) ? (x) : (y))
#endif	/* max */

#ifndef	min
#define	min(x, y) ((x) < (y) ? (x) : (y))
#endif	/* min */

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
#ifdef	RASPBERRYPI
    struct piFaceIO *	pfp;		/* PiFace pointer */
    struct gpioIO *	gep;		/* GPIO pointer */
#endif	/* RASPBERRYPI */
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
				/* OUT value for OUTW gates after registration */
#define	gt_out		gt_rll.out
#ifdef	RASPBERRYPI
				/* save pfFaceIO * or gpioIO * until registration*/
#define	gt_pfp		gt_rll.pfp
#define	gt_gep		gt_rll.gep
#endif	/* RASPBERRYPI */
#endif	/* TCP */
				/* reverse C function number in int array */
#define	gt_rfunctn	gt_rll.ip[FL_GATE]

extern iC_Gt		iClock;		/* System clock */

#if INT_MAX == 32767 && defined (LONG16)
extern long		iC_assignA(iC_Gt * glv, int ppi, long rv);
extern long		iC_assignL(iC_Gt * glv, int inv, int ppi, long rv);
#else
extern int		iC_assignA(iC_Gt * glv, int ppi, int rv);
extern int		iC_assignL(iC_Gt * glv, int inv, int ppi, int rv);
#endif
extern iC_Gt *		iC_index(iC_Gt * gm, int index);
extern struct timeval	iC_timeOut;	/* 50 mS select timeout - may be modified in iCbegin() */
extern unsigned short	P_channel;	/* PiFaceCAD channel associated with iCbegin() */
extern int		iCbegin(int argc, char** argv);	/* initialisation function */
extern int		iCend(void);			/* termination function */
extern char **		iC_string2argv(char * callString, int argc);	/* can be called in iCbegin() */
extern void		iC_fork_and_exec(char ** argv);			/* can be called in iCbegin() */
extern char		iC_stdinBuf[];	/* store a line of STDIN - reported by TX0.1 */
extern void		iC_quit(int sig);	/* quit with correct interrupt vectors */
extern short		iC_debug;	/* from -do argument in call to main + misc flag bits */
#define	DZ		0100000		/* misc flag in iC_debug to block STDIN */
#define	DQ		0040000		/* quiet operation on startup and shutdown of apps and iCserver */

#ifndef SIGRTMAX
#define SIGRTMAX	32			/* for non-POSIX systems (Win32) */
#endif
#define QUIT_TERMINAL	(SIGRTMAX+3)
#define QUIT_DEBUGGER	(SIGRTMAX+4)
#define QUIT_SERVER	(SIGRTMAX+5)

/* Special code to support PiFace Control and Display on a RapberryPi from any networked computer */
#define PIFACECAD_KEY	"iC PiFaceCAD I/O"	/* generate and test same key (TODO test currently not implemented)*/
extern void		writePiFaceCAD(		/* write display string either direct or via TCP/IP */
			const char * displayString,
			unsigned short channel);/* 0 = direct display or > 0 && <= 0xfff0 iCserver channel */
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

#define	OPS	".-++'^&|%*#/>({=][<:!@?ktwi"	/* DEBUG display of types - line up with pplstfix */

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
	UNDEF, AVAR, LVAR, LVAR, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE, YYERRCODE,\
	YYERRCODE, AOUT, LOUT, CVAR, TVAR, YYERRCODE,

#if INT_MAX == 32767
#define FUN_OFFSET	9	/* limits small model to 127 C functions, 511 arith values */
#else	/* INT_MAX != 32767 */
#define FUN_OFFSET	16	/* limits large model to 65535 C functions, 65535 arith values */
#endif	/* INT_MAX == 32767 */
#define VAR_ASSIGN	0x01	/* EA >> AU_OFS in comp.h */
#define VAR_USE		0x02	/* EU >> AU_OFS in comp.h */
#define USE_MASK	0x03	/* AU >> AU_OFS in comp.h */
									/*  small       large   */
#define VAL_MASK	((1<<FUN_OFFSET)-1)				/* 0x01ff    0x0000ffff */
#define FUN_MASK	(~VAL_MASK)					/* 0xfe00    0xffff0000 */
#define FUN_MAX		((1<<((sizeof(unsigned int)*8)-FUN_OFFSET))-1)	/* 0x007f    0x0000ffff */

#define USE_COUNT	(sizeof(unsigned int)*(8/2))	/* 8 bit byte / require 2 bits each */
#define USE_TEXT	"??","="," v","=v",		/* must be 4 entries for 2 bits under USE_MASK */

#define YYERRCODE	256	/* defined in comp.c but not exported */

/* Macro names defined once for genr.c init.c outp.c and cexe.h */
/* as well as here for CMACRO_NAMES */
#define MV	"iC_MV"		/* arithmetic int value in an iC arithmetic expression */

#define AV	"iC_AV"		/* arithmetic int value in a C statement in an iC if else or switch block */
#define LV	"iC_LV"		/* logical bit value in a C statement in an iC if else or switch block */
#define AA	"iC_AA"		/* arithmetic int assignment in a C statement in an iC if else or switch block */
#define LA	"iC_LA"		/* logical bit assignment in a C statement in an iC if else or switch block */
#define AVI	"iC_AVI"	/* indexed arithmetic int value in a C statement in an iC if else or switch block */
#define LVI	"iC_LVI"	/* indexed logical bit value in a C statement in an iC if else or switch block */
#define AAI	"iC_AAI"	/* indexed arithmetic int assignment in a C statement in an iC if else or switch block */
#define LAI	"iC_LAI"	/* indexed logical bit assignment in a C statement in an iC if else or switch block */
#define SIZ	"iC_SIZ"	/* value of a sizeof statment in a C statement in an iC if else or switch block */

#define AVL	"iC_AVL"	/* arithmetic int value in a C statement in a C literal block */
#define LVL	"iC_LVL"	/* logical bit value in a C statement in a C literal block */
#define AAL	"iC_AAL"	/* arithmetic int assignment in a C statement in a C literal block */
#define LAL	"iC_LAL"	/* logical bit assignment in a C statement in a C literal block */
#define AVIL	"iC_AVIL"	/* indexed arithmetic int value in a C statement in a C literal block */
#define LVIL	"iC_LVIL"	/* indexed logical bit value in a C statement in a C literal block */
#define AAIL	"iC_AAIL"	/* indexed arithmetic int assignment in a C statement in a C literal block */
#define LAIL	"iC_LAIL"	/* indexed logical bit assignment in a C statement in a C literal block */
#define SIZL	"iC_SIZL"	/* value of a sizeof statment in a C statement in a C literal block */

/* cMacro names generated by gram.y for ARITH and GATE values and assignments */
#define CMACRO_NAMES \
/* 0  ERR  */	"iC_ERR(",\
/* 1 ARITH */	AV "(",\
/* 2 GATE  */	LV "(",\
/* 3  asgn */	AA "(",\
/* 4       */	LA "(",\
/* 5 array */	AVI "(",\
/* 6       */	LVI "(",\
/* 7       */	AAI "(",\
/* 8       */	LAI "(",\
/* 9 size  */	SIZ "(",\
/* 10 literal */AVL "(",\
/* 11      */	LVL "(",\
/* 12      */	AAL "(",\
/* 13      */	LAL "(",\
/* 14      */	AVIL "(",\
/* 15      */	LVIL "(",\
/* 16      */	AAIL "(",\
/* 17      */	LAIL "(",\
/* 18 size */	SIZL "(",
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
extern FILE *		iC_outFP;	/* listing file pointer */
extern FILE *		iC_lstFP;	/* backup of iC_outFP for restoring when listing is unblocked */
extern FILE *		iC_nulFP;	/* listing file pointer to /dev/null when listing is blocked */
extern FILE *		iC_errFP;	/* error file pointer */
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

extern void	iC_link_cl(		/* link clocked Gate directly to iC_cList */
		Gate * gp, Gate * out_list);	/* during pass4 initialisation */


extern void	iC_icc(void);		/* initialise and execute iC load object */
extern Gate **	sTable;			/* pointer to Symbol Table */
extern Gate **	sTend;			/* end of Symbol Table */

extern unsigned short	iC_osc_max;
extern unsigned short	iC_osc_lim;
extern unsigned short	iC_osc_flag;

#define USE_ALIAS	(1<<0)
#define USE_STRICT	(1<<1)
#define USE_LIST	(1<<2)
#define MAXUSETYPE	3
#define USESTACKSZ	32		/* nested includes - should do */

extern unsigned int	iC_uses;	/* 01=alias 02=strict 04=list as bit field */
extern unsigned int	iC_useStack[USESTACKSZ];
extern unsigned int	iC_useStackIndex;

#define iC_Aflag (iC_uses & USE_ALIAS)	/* -A alias - generate ARITH alias nodes */
#define iC_Sflag (iC_uses & USE_STRICT)	/* -S strict - all imm variables must be declared - default */
#define iC_Zflag (iC_uses & USE_LIST)	/* no command line option - use list is default */

extern char *		iC_aflag;	/* -a list iC preprocessor commands with immac -Ma */
extern unsigned short	iC_Lflag;	/* -L append mode - compile with linking information */
extern unsigned short	iC_lflag;	/* -L build new aux files */

					/*   icc.c   */
extern char *		iC_progname;	/* name of this executable */
extern int		iC_argc;	/* extra options passed to iCbegin(int iC_argc, char** iC_argv) */
extern char **		iC_argv;
extern int		iC_argh;	/* block running iCserver before iCbegin() */
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

extern Gate *		iC_aList;
extern Gate *		iC_oList;
extern Gate *		iC_cList;
extern Gate *		iC_fList;
extern Gate *		iC_sList;	/* send bit and byte outputs */

extern unsigned int	iC_bit2[];
extern Gate *		iC_gx;		/* points to action Gate in chMbit and riMbit */
#if YYDEBUG && !defined(_WINDOWS)
extern short		iC_dc;		/* debug display counter in scan and rsff */
#endif
extern unsigned char	iC_bitIndex[];
extern unsigned char	iC_bitMask[];
#define	X_MASK		0xff		/* mask to detect used bits in bit I/O byte */
#define	W_MASK		0x100		/* marks output as word I/O */
#define	B_WIDTH		0x101		/* marks output as byte width signed */
#define	W_WIDTH		0x102		/* marks output as word width signed */
#if INT_MAX != 32767 || defined (LONG16)
#define	L_WIDTH		0x103		/* marks output as long width signed */
#define	H_WIDTH		0x104		/* marks output as long long width signed */
#define	S_WIDTH		0x108		/* marks output as string */
#endif
#ifdef	RASPBERRYPI
#define	X_FLAG		0x200		/* flag non-registration in gt_mark */
#define	P_FLAG		0x400		/* flag PiFace Port A out or PortB in Gate in gt_mark */
#define	PB_FLAG		0xC00		/* flag PiFace Port B out Gate in gt_mark */
#define	G_FLAG		0x800		/* flag GPIO in or out group Gate in gt_mark */
#define	PG_MASK		0xC00		/* mask for the above 3 flags in gt_mark */
#define OopsMask	1LL		/* mask to protect iCtherm from modprobe Oops failure */
extern int		(*iC_term)(int);	/* clear and unexport RASPBERRYPI stuff */
#endif	/* RASPBERRYPI */

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
#define	iC_null1	(iC_Functp)iC_null	/* null function for init lists */

extern iC_Functp	iC_gate_i[];
extern iC_Functp	iC_ff_i[];
extern iC_Functp 	iC_clock_i[];

#define	MARKMAX		4			/* number of oscillations allowed */
#define	OSC_WARN_CNT	10			/* number of oscillation warnings */
#define BS		1024			/* buffer size for I/O strings and chained command lines */

extern unsigned short	iC_mark_stamp;		/* incremented every scan */
extern Gate *		iC_osc_gp;		/* report oscillations */

extern unsigned		iC_scan_cnt;		/* count scan operations */
extern unsigned		iC_link_cnt;		/* count link operations */
#if YYDEBUG && (!defined(_WINDOWS) || defined(LOAD))
extern unsigned		iC_glit_cnt;		/* count glitches */
extern unsigned	long	iC_glit_nxt;		/* count glitch scan */
#endif

#ifdef	TCP
extern unsigned		iC_linked;		/* link Flag for iC_liveData() */
#define ENTRYSZ		24			/* accomodates ",SIX123456.7,RQX123456.7" */
extern unsigned short	C_channel;		/* channel for sending messages to Debug in ict.c */
#ifndef	EFENCE
extern char		regBuf[];
extern char		rpyBuf[];		/* Buffer in which iCserver input is read to */
extern char		msgBuf[];
#else	/* EFENCE */
extern char *		regBuf;
extern char *		rpyBuf;
extern char *		msgBuf;
extern char *		iC_outBuf;		/* Buffer to transfer out data from iC_output() */
#endif	/* EFENCE */
extern FILE *		iC_vcdFP;
extern FILE *		iC_savFP;

#ifdef	RASPBERRYPI
#include <stdint.h>			/* defines a set of integral type aliases */
#define MAXPF		8		/* maximum number of PiFace's */
#define IOUNITS		30		/* initial Units[] size */

/********************************************************************
 *	Used for marking I/O type in gt_live to influence registration.
 *	gt_live is only used for VCD generation and normal live operation
 *	after registration is completed. (gt_live is 0 because of emalloc()) 
 *******************************************************************/

#define External	0		/* default  IXn <== iCserver <== QXn */
#define Internal	1		/*          IXn <==   SIO    <== QXn */
					/* if opt_E    also iCserver <== IXn (display only) */
					/*             and  iCserver <== QXn */
#define Dummy		2		/* ignore -    output WARNING    *** */
#define ExtOut		3		/* like     iCserver (IXn)   <== SIO */
					/* iCpiFace SIO      (QXn)   <== iCserver */

/********************************************************************
 *
 *	piFaceIO array iC_pfL[unit] is organised as follows:
 *	  [unit] is the PiFace unit index in address order
 *	  each element of the array is a struct piFaceIO
 *
 *	A single PiFace can be addressed from 0 - 3 with jumpers.
 *	With CE0 selected /dev/spidev0.0 is used for SPI transfers.
 *	With CE1 selected /dev/spidev0.1 is used for SPI transfers.
 *
 *	A PiRack can accomodate up to 4 PiFaces (more if daisy-chained).
 *	Also CE for each PiFace can be changed from CE0 (default) to CE1.
 *	This changes the addresses above from 4 - 7 allowing a maximum
 *	of 8 PiFaces to be used.
 *
 *******************************************************************/

typedef struct	iqDetails {
    union {
	char *		name;		/* i.name IXn, IXn-i, QXn or QXn-i */
	Gate *		gate;		/* i.gate named IXn, IXn-i, QXn or QXn-i */
    } i;				/* name used in iCpiFace.c, gate used in load.c */
    int			inv;		/* in or out inversion mask */
    int			val;		/* previous input or output value */
    unsigned short	channel;	/* channel to send I or receive Q to/from iCserver */
    uint8_t		bmask;		/* selected bits for this input or output */
} iqDetails;

typedef struct	piFaceIO {
    iqDetails		s[3];		/* select QXn with iq=0, IXn with iq=1, QXn+ with iq=2 */
    unsigned short	pfa;		/* PiFace address 0 - 7 */
    uint8_t		intf;		/* PiFace INTFB PiFaceCAD INTFA */
    uint8_t		inpr;		/* PiFace GPIOB PiFaceCAD GPIOA */
    int			spiFN;		/* file number for CE0 or CE1 */
} piFaceIO;

/********************************************************************
 *
 *	gpioIO structures are organised as follows:
 *	  each structure describes an independent IXn or QXn bit group
 *	  which services up to 8 GPIO pins.
 *
 *******************************************************************/

typedef struct	gpioIO {
    iqDetails		s;		/* IXn or QXn details */
    unsigned short	gpioNr[8];	/* gpio number for bits 0 - 7 */
    int			gpioFN[8];	/* file number for bits 0 - 7 */
    struct gpioIO *	nextIO;		/* arrange in null terminated linked list */
} gpioIO;

#define Qname		s[0].i.name	/* name QXn or QXn-i, used in iCpiFace.c */
#define Iname		s[1].i.name	/* name IXn or IXn-i */
#define QPname		s[2].i.name	/* name QXn or QXn-i, used Relay+ */
#define Gname		s.i.name	/* GPIO name IXn QXn or IXn-i QXn-i */
#define Qgate		s[0].i.gate	/* gate named QXn or QXn-i, used in load.c */
#define Igate		s[1].i.gate	/* gate named IXn or IXn-i */
#define QPgate		s[2].i.gate	/* name QXn or QXn-i, used Relay+ */
#define Ggate		s.i.gate	/* GPIO gate named IXn QXn or IXn-i QXn-i */
#define Qinv		s[0].inv	/* PiFace out inversion mask */
#define Iinv		s[1].inv	/* PiFace in inversion mask */
#define QPinv		s[2].inv	/* PiFace out+ inversion mask */
#define Ginv		s.inv		/* GPIO in or out inversion mask */
#define Qval		s[0].val	/* PiFace previous out value */
#define Ival		s[1].val	/* PiFace previous in value */
#define QPval		s[2].val	/* PiFace previous out+ value */
#define Gval		s.val		/* GPIO previous input or output value */
#define Qchannel	s[0].channel	/* PiFace channel number to receive out from iCserver */
#define Ichannel	s[1].channel	/* PiFace channel number to send in to iCserver */
#define QPchannel	s[2].channel	/* PiFace channel number to receive out+ from iCserver */
#define Gchannel	s.channel	/* GPIO channel number to send or receive from iCserver */
#define Qbmask		s[0].bmask	/* PiFace bmask for out from iCserver */
#define Ibmask		s[1].bmask	/* PiFace bmask for in to iCserver */
#define QPbmask		s[2].bmask	/* PiFace bmask for out+ from iCserver */
#define Gbmask		s.bmask		/* GPIO for send or receive from iCserver */

/********************************************************************
 *
 *	channelSel structures are used in ict.c for controlling direct I/O
 *
 *******************************************************************/

typedef struct	channelSel {
    Gate *		g;		/* gate named IXn, IXn-i, QXn or QXn-i */
    int			pqs;		/* 0 no direct I/O, 1 piFaceIO, 2 gpioIO */
    int			iqs;		/* 0 Port A output, 1 Port B input, 2 Port B output */
    union {
	piFaceIO *	p;		/* pointer to selected PiFace for direct I/O */
	gpioIO *	q;		/* pointer to selected GPIO group for direct I/O */
    };
} channelSel;

extern int	iC_opt_P;
extern int	iC_opt_G;
extern int	iC_opt_B;
extern int	iC_opt_E;
extern int	iC_opt_L;
extern int	iC_npf;
extern int	gpio23FN;		/* /sys/class/gpio/gpio23/value LIRC file number */
extern int	gpio25FN;		/* /sys/class/gpio/gpio25/value SPI file number */
extern int	spidFN[];		/* /dev/spidev0.0, /dev/spidev0.1 SPI file numbers */
extern piFaceIO	iC_pfL[];		/* piFace names/gates and channels */
extern gpioIO *	iC_gpL[];		/* gpio name/gates and channels */
extern void	iC_gpio_pud(int gpio, int pud);	/*  execute iCgpioPUD(gpio, pud) to set pull-up/down */
#endif	/* RASPBERRYPI */
extern int	iC_opt_l;
#endif	/* TCP */

extern void		iC_initIO(void);	/* init signal and correct interrupt vectors */

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
extern void		iC_output(long val, unsigned short channel);
extern void		iC_liveData(Gate * gp, long value);	/* VCD and/or iClive */
#else	/* INT_MAX == 32767 && defined (LONG16) */
extern void		iC_output(int val, unsigned short channel);
extern void		iC_liveData(Gate * gp, int value);	/* VCD and/or iClive */
#endif	/* INT_MAX == 32767 && defined (LONG16) */
					/*   misc.c  */
#ifdef	_MSDOS_
#ifdef	MSC
extern void (interrupt far *oldhandler)(void);
#else	/* MSC */
extern void (interrupt *oldhandler)(void);
#endif	/* MSC */
#else	/* ! _MSDOS_ Linux */
extern int		ttyparmFlag;
extern struct termios	ttyparms;
#endif	/* ! _MSDOS_ Linux */
#ifdef	_WIN32
#define snprintf	 _snprintf
extern int		mkstemp(char * template);
#endif	/* _WIN32 */
#if defined TCP && defined RASPBERRYPI
#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */
#define GPIOCHIP "/dev/gpiochip0"	/* gpiochipX (0-4) depending on Pi model */
#define LINEEVENT_BUFFERS	 4	/* kernel buffers 16 line events */
#define GPIO_LIMIT		 64	/* GPIO numbers are limited 0 - 63 */
typedef struct {
    struct gpio_v2_line_config *	linecfg;
    struct gpio_v2_line_request *	linereq;
    struct gpio_v2_line_values *	linevals;
    int					fd;
} gpio_v2_t;
typedef struct gpio_T {
    gpioIO *		gep;
    unsigned short	val;		/* stores either val or bit */
} gpio_T;
extern int	maskIdx[17];
#ifdef LOAD
extern gpio_T	gpioArray[GPIO_LIMIT];
extern gpio_T	gepArray[LINEEVENT_BUFFERS];
extern struct gpio_v2_line_event	lineevent[LINEEVENT_BUFFERS];
extern gpio_v2_t pins;
extern int	gpio_line_cfg_ioctl (gpio_v2_t * gpio);
extern int	gpio_line_set_values (gpio_v2_t * gpio, __u64 bits,  __u64 mask);
extern int	gpio_line_get_values (gpio_v2_t * gpio, __u64 mask);
extern __u64	maskBit25;
#endif /* LOAD */
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
#endif /* defined TCP && defined RASPBERRYPI */

#endif	/* ICC_H */

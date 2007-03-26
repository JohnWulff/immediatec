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
 *	comp.h
 *	header for immcc compiler
 *
 *******************************************************************/

#ifndef COMP_H
#define COMP_H
static const char comp_h[] =
"@(#)$Id: comp.h,v 1.54 2007/03/21 13:33:29 jw Exp $";

#include	<setjmp.h>

#define NS		((char*)0)
#define	TSIZE		256
#define	IBUFSIZE	512
#define	IMMBUFSIZE	1024
#define	FUNUSESIZE	64

#define	NOT	1		/* used in List_e.le_val */
#define FEEDBACK

/* Stringification of a defined macro s */
#define SS(s)	S(s)
#define S(s)	#s

typedef	struct	List_e {	/* list element */
    struct Symbol *	le_sym;
    unsigned int	le_val;
    struct List_e *	le_next;
    char *		le_first;
    char *		le_last;
} List_e;

typedef	struct Type {		/* auxiliary compile type to preserve types and masks */
    unsigned char	type;	/* ARN XOR AND OR LATCH FF CLK TIM ... */
    unsigned char	ftype;	/* ARITH GATE S_FF R_FF D_FF CLCK TIMR ... */
    unsigned char	em;	/* bit to mark external Symbol */
    unsigned char	fm;	/* mark parameter and local var in function definition */
} Type;

typedef	struct Symbol {		/* symbol table entry */
    char *		name;	/* element name */
    unsigned char	type;	/* ARN XOR AND OR LATCH FF CLK TIM ... */
    unsigned char	ftype;	/* ARITH GATE S_FF R_FF D_FF CLCK TIMR ... */
    unsigned char	em;	/* bit to mark external Symbol */
    unsigned char	fm;	/* mark parameter and local var in function definition */
    List_e *		list;	/* output list pointer */
#if ! YYDEBUG || defined(SYUNION)
    union {
#endif
	Gate *		gate;	/* AND OR */
	List_e *	blist;	/* used in compile */
	unsigned int	val;	/* used to hold function number etc. */
#if ! YYDEBUG || defined(SYUNION)
    } u;
    union {
#endif
	unsigned int	cnt;	/* used to hold link_count in listNet() */
	List_e *	elist;	/* feedback list pointer */
	struct Symbol *	glist;	/* mark symbols in C compile */
#if ! YYDEBUG || defined(SYUNION)
    } v;
#endif
    struct Symbol *	next;	/* to link to another Symbol, mostly in ST */
} Symbol;

#if ! YYDEBUG || defined(SYUNION)
#define u_gate		u.gate
#define u_blist		u.blist
#define u_val		u.val
#define v_cnt		v.cnt
#define v_elist		v.elist
#define v_glist		v.glist
#else				/* no unions is easier for debugging with ddd */
#define u_gate		gate
#define u_blist		blist
#define u_val		val
#define v_cnt		cnt
#define v_elist		elist
#define v_glist		glist
#endif

/* for use in a union identical first elements can be accesed */
typedef struct { char *f; char *l; Symbol * v;     } Sym;
typedef struct { char *f; char *l; List_e * v;     } Lis;
typedef struct { char *f; char *l; unsigned int v; } Val;
typedef struct { char *f; char *l; Type v;         } Typ;
typedef struct { char *f; char *l; char v[2];      } Str;

typedef struct Token {		/* Token for gram.y grammar */
    unsigned int	start;
    unsigned int	end;
    Symbol *		symbol;
} Token;

					/*  comp.y  */
extern int  iCparse(void);		/* generated yacc parser function */
extern int  compile(char *, char *,
		    char *, char *);	/* compile iC language source */
extern void errmess(char *, char *, char *);	/* actual error message */
extern void errBit(void);		/* no constant allowed in bit expression */
extern void errInt(void);		/* no imm variable to trigger arithmetic expression */
extern void ierror(char *, char *);	/* print error message */
extern void warning(char *, char *);	/* print warning message */
extern void execerror(char *, char *,
		    char *, int);	/* recover from run-time error */

extern int	c_lex(void);		/* produced by lexc.l for gram.y */
#ifndef LMAIN
extern void yyerror(char * s);		/* called for yacc syntax error */
extern int  get(FILE* fp, int x);	/* character input shared with lexc.l */
#endif
extern int	ynerrs;			/* count of iCerror() calls */
		/* NOTE iCnerrs is reset for every call to yaccpar() */

extern int	iC_Pflag;
extern int	lexflag;
/********************************************************************
 *	lexflag is bitmapped and controls the input for lexers
 *
 *	bit	mask	iCparse	c_parse	function
 *  C_PARSE	01	00	01	select parser
 *  C_FIRST	02	02	02	set for first line of listing
 *  C_BLOCK	04	-	04	block c_parse source listing
 *  C_NO_COUNT	010	-	010	c_parse blocks counting chars
 *  C_LINE	020	-	020	# 1 or # line seen
 *  C_LINE1	040	-	040	# line seen
 *  C_BLOCK1	0100	-	0100	block c_parse listing completely
 *  C_PARA	0200	-	0200	C function parameters are being collected
 *  C_FUNCTION	0400	-	0400	C function is being parsed
 *******************************************************************/
#define C_PARSE		01
#define C_FIRST		02
#define C_BLOCK		04
#define C_NO_COUNT	010
#define C_LINE		020
#define C_LINE1		040
#define C_BLOCK1	0100
#define C_PARA		0200
#define C_FUNCTION	0400

extern int	lineno;
extern int	iC_maxErrCount;
extern int	iC_iErrCount;
extern jmp_buf	beginMain;
					/*   genr.c  */
extern int	c_number;		/* case number for cexe.c */
extern int	outFlag;		/* global flag for compiled output */
extern char *	cexeString[];		/* case or function string */

/********************************************************************
 * functionUse[] is a dynamic array of records with 2 fields: c_cnt, c_expr.
 *
 * functionUse[x].c_cnt - records the number of calls for each C function
 *   x = 1 2 3 ... generated. If a C function is generated in an iC function
 *   definition, which is never called, this count is 0 and the function is
 *   not copied into the generated C code.
 *
 * functionUse[x].c_expr - points to a copy of the C expression returned
 *   by the function. This is used to produce a better listing of cloned
 *   functions and is vital information for optimising arithmetic functions.
 *
 * functionUse[0].c_cnt or functionUse->c_cnt records the type of function called:
 *	bit	value	description
 * F_CALLED	01	any type of call
 * F_ARITHM	02	imm reference in an immediate arithmetic expression
 * F_FFEXPR	04	imm reference in an immediate if else or switch
 * F_LITERAL	010	imm reference generated in a literal block
 *******************************************************************/

#define F_CALLED	01
#define F_ARITHM	02
#define F_FFEXPR	04
#define F_LITERAL	010

typedef struct FuUse {			/* Function call count and C expression */
    int		c_cnt;			/* call count */
    char *	c_expr;			/* C expression */
} FuUse;

extern FuUse *	functionUse;		/* database to record function calls */
extern int	functionUseSize;	/* dynamic size adjusted with realloc */

#ifndef LMAIN
extern char *	szNames[];
#define inpFN	szNames[1]		/* input file name */
#endif	/* LMAIN */

#ifndef EFENCE
extern char	inpNM[];		/* currently scanned input file name */
extern char	iC_defines[];		/* -D -U definitions from command line */
extern char	iC_Cdefines[];		/* -C -V definitions from command line */
extern char	iCbuf[];		/* buffer to build imm statement size IMMBUFSIZE */
extern char	iFunBuffer[];		/* buffer to build imm function symbols */
#else
extern char *	inpNM;			/* currently scanned input file name */
extern char *	iC_defines;		/* -D -U definitions from command line */
extern char *	iC_Cdefines;		/* -C -V definitions from command line */
extern char *	iCbuf;			/* buffer to build imm statement size IMMBUFSIZE */
extern char *	iFunBuffer;		/* buffer to build imm function symbols */
#endif
extern char *	iFunEnd;		/* pointer to end */
extern char *	iFunSymExt;		/* flags that function is being compiled */
extern Sym	iRetSymbol;		/* .v is pointer to imm function return Symbol */
extern Symbol * assignExpression(	/* assignment of an aexpr to a variable */
	    Sym * sv, Lis * lv, int ioTyp);
extern void	assignOutput(		/* generate and assign an output node */
	    Symbol * rsp, int ftyp, int ioTyp);
extern void	listGenOut(		/* listing for undefined C variable */
	    Symbol * sp);
extern List_e *	delayOne(List_e * tp);	/* implicit delay of 1 tick for ctref : texpr ; */
extern List_e *	cCallCount(		/* check parameter count in 'cCall' */
	    Symbol * cName, List_e * cParams);
extern List_e *	cListCount(		/* count parameters in 'cList' */
	    List_e * cPlist, List_e * aexpr);
extern Symbol *	functionDefHead(	/* set up the function definition head */
	    unsigned int ftyp, Symbol * funTrigger, int retFlag);
extern List_e *	collectStatement(	/* collect statements in the function body */
	    Symbol * funcStatement);
extern Symbol *	returnStatement(	/* value function return statement */
	    Lis * actexpr);
extern Symbol *	functionDefinition(	/* finalise function definition */
	    Symbol * iFunHead, List_e * fParams);
extern Symbol *	pushFunCall(Symbol *);	/* save global variables for nested function calls */
extern List_e * handleRealParameter(	/* handle a real parameter of a called function */
	    List_e * plp, List_e * alp);
extern List_e *	cloneFunction(		/* clone a function template in a function call */
	    Symbol * functionHead, List_e * plp);
extern Symbol *	clearFunDef(		/* clear a previous function definition */
	    Symbol * functionHead);
extern List_e *	checkDecl(		/* check decleration of a function terminal */
	    Symbol * terminal);
extern void	initcode(void);		/* initialize for code generation */
extern List_e *	sy_push(Symbol *);	/* create List element for variable */
extern Symbol *	sy_pop(List_e *);	/* delete List element left over */
extern List_e * op_force(		/* force linked Symbol to ftype */
	    List_e *, unsigned char);	/*   lp, ftyp    */
extern List_e *	op_push(List_e *,	/* reduce List_e stack to links */
	    unsigned char, List_e *);	/*   left, typ, right   */
extern int	const_push(Lis * expr);	/* numeric constant push */
extern List_e *	op_not(List_e *);	/* logical negation */
extern void	writeCexeString(FILE * oFP, int cn);
extern Symbol *	op_asgn(Sym *, Lis *,	/* asign List_e stack to links */
	    unsigned char);		/*   var, right, ftyp   */
extern List_e * bltin(			/* generate built in iC functions */
	    Sym* fname,			/* function name and ftype */
	    Lis* ae1, Lis* cr1,		/* expression */
	    Lis* ae2, Lis* cr2,		/* optional set */
	    Lis* ae3, Lis* cr3,		/* optional reset */
	    Lis* crm,			/* optional mono-flop clock */
	    Val* pVal);			/* optional cblock# or off-delay */
#if YYDEBUG
extern void	pu(int t, char * token, Lis * node);
#endif

extern Symbol *	iclock;			/*   init.c  */
extern void	init(void);		/* install constants and built-ins */
extern const char initialFunctions[];	/* iC system function definitions */
extern const char * genLines[];		/* SHR, SHSR generate C functions 1 and 2 */
extern const char * genName;
extern int	    genLineNums[];
#define GEN_COUNT 2			/* number of C functions in precompiled functions */

					/*   symb.c   */
#define	HASHSIZ 54*16			/* for new sorted list algorithm */
extern Symbol *	symlist[];		/* symbol table: linked lists */
extern Symbol *	lookup(char *);		/* find string in symbol table */
extern Symbol *	install( char *, unsigned char,
		    unsigned char);	/* install Symbol in symbol table */
extern Symbol *	link_sym(Symbol *);	/* link Symbol into symbol table */
extern Symbol * unlink_sym(Symbol *);	/* unlink Symbol from symbol table */
extern void	uninstall(Symbol *);	/* uninstall Symbol from symbol table (free space) */

					/*   main.c   */
#define H1name	".iC_list1.h"
#define H2name	".iC_list2.h"

/* index 0 not used */
#define Iindex	1
#define Eindex	2
#define Lindex	3
#define Oindex	4
#define COindex	5
#define H1index	6
#define H2index	7
#define T0index	8
#define T1index	9
#define T2index	10
#define T3index	11
#define T4index	12
#define T5index	13
#define T6index	14

#define INITIAL_FILE_NAMES	0, 0, 0, 0, 0, 0, H1name, H2name, 0, 0, 0, 0, 0, 0, 0,

extern FILE *	T0FP;
extern FILE *	T1FP;
extern FILE *	T2FP;
extern FILE *	T3FP;
extern FILE *	T4FP;
extern FILE *	T5FP;
extern FILE *	T6FP;

extern char	T0FN[];
extern char	T2FN[];
extern char	T4FN[];
extern char	T5FN[];
extern char	T6FN[];
extern int	openT4T5(int mode);

					/*   outp.c   */
#define BUFS	256
extern int	IEC1131(char * name, char * buf, int bufLen,
			char * iqt, char * xbwl, int * bytep,
			int * bitp, char * tail);
extern int	toIEC1131(char * name, char * buf, int bufLen,
			  char * iqt, char * xbwl, int * bytep,
			  int * bitp, char * tail);

extern int	listNet(unsigned gate_count[]);		/* list generated network */
extern int	buildNet(Gate ** igpp,			/* generate execution network */
			    unsigned gate_count[]);
extern int	output(FILE * iFP, char * outfile);	/* generate network as C file */
extern int	c_compile(FILE * iFP, FILE * oFP, int flag, List_e * lp);
extern int	copyXlate(FILE * iFP, FILE * oFP, char * outfile, unsigned * lcp, int mode);
extern unsigned short	iC_Tflag;			/* define iC_tVar */

					/*   lexc.l   */
extern int	c_leng;
extern int	column;
extern int	gramOffset;
extern void	delete_sym(Token* tokp);
#ifndef LMAIN
extern void	markParaList(Symbol * sp); /* mark Symbol as parameter on glist */
extern void	clearParaList(int flag); /* clear parameter list from extraneous entries */
#endif
					/*   gram.y   */
extern int	c_parse(void);		/* generated yacc parser function */
extern void	copyAdjust(FILE* iFP, FILE* oFP, List_e* lp);
#endif	/* COMP_H */

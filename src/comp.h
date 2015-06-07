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
 *	comp.h
 *	header for immcc compiler
 *
 *******************************************************************/

#ifndef COMP_H
#define COMP_H
static const char comp_h[] =
"@(#)$Id: comp.h,v 1.67 2015/06/03 08:20:13 jw Exp $";

#include	<setjmp.h>

#define NS		((char*)0)
#define	TSIZE		256
#define	IBUFSIZE	512
#define	IMMBUFSIZE	32768	/* allows 16 bytes for each possible 508 inputs and 1 output in a statement */
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
	List_e *	blist;	/* used in compile (1st union member to allow initialisation) */
	Gate *		gate;	/* AND OR */
	unsigned int	val;	/* used to hold function number etc. */
#if ! YYDEBUG || defined(SYUNION)
    } u;
    union {
#endif
	List_e *	elist;	/* feedback list pointer */
	struct Symbol *	glist;	/* mark symbols in C compile */
	unsigned int	cnt;	/* used to hold link_count in listNet() */
#if ! YYDEBUG || defined(SYUNION)
    } v;
#endif
    struct Symbol *	next;	/* to link to another Symbol, mostly in ST */
} Symbol;

#if ! YYDEBUG || defined(SYUNION)
#define u_blist	u.blist
#define u_gate	u.gate
#define u_val	u.val
#define v_elist	v.elist
#define v_glist	v.glist
#define v_cnt	v.cnt
#else				/* no unions is easier for debugging with ddd */
#define u_blist	blist
#define u_gate	gate
#define u_val	val
#define v_elist	elist
#define v_glist	glist
#define v_cnt	cnt
#endif

#define EM	0x01	/* em bit mask for extern until locally declared */
#define EX	0x02	/* em bit mask for extern immC - can be used and defined externally */
#define TM1	0x04	/* em bit mask for TIMER1 */
#define AU_OFS	4	/* em offset for immC bit masks */
#define EA	0x10	/* (1<<AU_OFS)   em bit mask for immC variable defined (replaces VAR_ASSIGN) */
#define EU	0x20	/* (2<<AU_OFS+1) em bit mask for immC variable used (replaces VAR_USE) */
#define AU	0x30	/* (EA|EU)       em bit mask for immC variable used or defined (replaces USE_MASK) */
			/* if no higher bits in em AU need not be used after shifting EA or EU (use anyway) */

#define FU	0x03	/* fm bit mask for use count 0, 1 or 2 */
#define FI	0x10	/* fm increment for input count 0, 1 or 2 (0x00, 0x10, 0x20) */
#define FMI	0x30	/* fm bit mask for input count 0, 1 or 2 (0x00, 0x10, 0x20) */
#define FA	0x40	/* fm bit wihch marks assign parameter in function definition */
#define FM	0x80	/* fm bit wihch marks parameter and local var in function definition */

/* for use in a union identical first elements can be accesed */
typedef struct Sym { char *f; char *l; Symbol * v;     } Sym;
typedef struct Lis { char *f; char *l; List_e * v;     } Lis;
typedef struct Val { char *f; char *l; int v;          } Val;
typedef struct Typ { char *f; char *l; Type v;         } Typ;
typedef struct Str { char *f; char *l; char v[2];      } Str;

typedef struct Token {		/* Token for gram.y grammar */
    unsigned int	start;
    unsigned int	end;
    unsigned int	inds;
    unsigned int	inde;
    Symbol *		symbol;
} Token;

typedef struct BuiltIn {	/* initial Symbol info in init.c */
    char *		name;
    unsigned char	type;
    unsigned char	ftype;
    unsigned int	uVal;		/* yacc token for type KEYW */
    List_e *		list;		/* output list pointer */
    List_e *		blist;		/* forward blist pointer */
    List_e *		link1;		/* 2 back links for new built-in */
    List_e *		link2;		/* set up in BOOT_COMPILE */
} BuiltIn;

					/*  comp.y  */
extern int  iCparse(void);		/* generated yacc parser function */
extern int  iC_compile(char *, char *,
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
extern void yyerror(const char * s);	/* called for yacc syntax error */
extern int  get(FILE* fp, int x);	/* character input shared with lexc.l */
#endif
extern int	ynerrs;			/* count of yyerror() calls */
extern int	gnerrs;			/* count of ierror() calls */
		/* NOTE iCnerrs is reset for every call to yaccpar() */

extern int	iC_Pflag;
extern int	iC_Wflag;
#define W_DEPRECATED_LOGIC	01	/* on/no logic warnings */
#define W_FUNCTION_PARAMETER	02	/* on/no function parameter warnings */
#define W_FUNCTION_DELETE	04	/* on/no function delete warnings */
#define W_UNDEFINED		010	/* on/no undefined gate warnings */
#define W_UNUSED		020	/* on/no unused gate warnings */
#define W_ALL	(W_DEPRECATED_LOGIC | W_FUNCTION_PARAMETER | W_FUNCTION_DELETE |\
		 W_UNDEFINED | W_UNUSED)	/* all warnings are on */
/* define other Warning switches here */
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
extern char *	iCstmtp;		/* manipulated in iClex() (reset in clrBuf()) */
extern jmp_buf	beginMain;
					/*   genr.c  */
extern int	c_number;		/* case number for cexe.c */
extern int	outFlag;		/* global flag for compiled output */
extern const char * cexeString[];	/* case or function string */

/********************************************************************
 * functionUse[] is a dynamic array of records with 2 fields: c_cnt, c.expr.
 *
 * functionUse[x].c_cnt - records the number of calls for each C function
 *   x = 1 2 3 ... generated. If a C function is generated in an iC function
 *   definition, which is never called, this count is 0 and the function is
 *   not copied into the generated C code.
 *
 * functionUse[x].c.expr - points to a copy of the C expression returned
 *   by the function. This is used to produce a better listing of cloned
 *   functions and is vital information for optimising arithmetic functions.
 *
 * functionUse[0].c_cnt or functionUse->c_cnt records the type of function called:
 *	bit	value	description
 * F_CALLED	01	any type of call
 * F_ARITHM	02	imm reference in an immediate arithmetic expression
 * F_FFEXPR	04	imm reference in an immediate if else or switch
 * F_LITERAL	010	imm reference generated in a literal block
 * F_ARRAY	020	imm reference to an iC array
 * F_SIZE	040	imm reference to a sizeof operator
 *******************************************************************/

#define F_CALLED	01
#define F_ARITHM	02
#define F_FFEXPR	04
#define F_LITERAL	010
#define F_ARRAY		020
#define F_SIZE		040

typedef struct FuUse {			/* Function call count and C expression */
    int		c_cnt;			/* call count */
    union	c {
	char *	expr;			/* C expression */
	int	use;			/* final use count in outNet */
    } c;
#ifdef BOOT_COMPILE
    int		lineNo;			/* line number in iC source at actual compile time */
    char *	inpNm;			/* input name of iC source at actual compile time */
    char *	headNm;			/* function head name of iC function block at boot generation */
#endif	/* BOOT_COMPILE */
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
extern void	listGenOut(		/* listing for undefined immC variable */
	    Symbol * sp, int size);
extern List_e *	delayOne(List_e * tp);	/* implicit delay of 1 tick for ctref : texpr ; */
extern List_e *	cCallCount(		/* check parameter count in 'cCall' */
	    Symbol * cName, List_e * cParams);
extern List_e *	cListCount(		/* count parameters in 'cList' */
	    List_e * cPlist, List_e * aexpr);
extern Symbol *	functionDefHead(	/* set up the function definition head */
	    unsigned int ftyp, Symbol * funTrigger);
extern List_e *	collectStatement(	/* collect statements in the function body */
	    Symbol * funcStatement);
extern Symbol *	returnStatement(	/* value function return statement */
	    Lis * actexpr);
extern Symbol *	functionDefinition(	/* finalise function definition */
	    Symbol * iFunHead, List_e * fParams);
extern Symbol * getRealParameter(	/* get a real parameter of a called function */
	    Symbol * hsp, List_e * lp);
extern Lis	cloneFunction(		/* clone a function template in a function call */
	    Sym * fhs, Sym * hsym, Str * par);
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
extern void	writeCexeTail(FILE * oFP, const char * tail, int cn);
extern Symbol *	op_asgn(Sym *, Lis *,	/* asign List_e stack to links */
	    unsigned char);		/*   var, right, ftyp   */
extern List_e * bltin(			/* generate built in iC functions */
	    Sym* fname,			/* function name and ftype */
	    Lis* ae1, Lis* cr1,		/* expression */
	    Lis* ae2, Lis* cr2,		/* optional set */
	    Lis* ae3, Lis* cr3,		/* optional reset */
	    Val* pVal);			/* optional cblock# or off-delay */
#if YYDEBUG
enum stackType { SYM, LIS, VAL, TYP, STR };
extern void	pu(enum stackType t, const char * token, void * node);
#endif

					/*   init.c  */
extern Symbol *	iclock;			/* default clock */
extern Symbol *	iconst;			/* pointer to Symbol "iConst" */
extern Symbol *	icerr;			/* pointer to Symbol "iCerr" */
#ifdef BOOT_COMPILE
extern Symbol	iC_CHANGE_ar;		/* alternative arithmetic CHANGE */
#endif	/* BOOT_COMPILE */
extern void	iC_init(void);		/* install constants and built-ins */
extern char *	iC_genName;
extern int	iC_genLineNums[];
extern int	iC_genCount;		/* actual number of pre-comp C functions */
extern unsigned short	iC_gflag;	/* -g independent C code for gdb debugging */

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
extern int	iC_openT4T5(int mode);

					/*   outp.c   */
#define BUFS	256
extern int	iC_toIEC1131(char * name, char * buf, int bufLen,
			  char * iqt, char * xbwl, int * bytep,
			  int * bitp, char * tail);

extern int	iC_listNet(void);			/* list generated network */
#if defined(RUN) || defined(TCP)
extern int	iC_buildNet(
		Gate *** asTable, Gate *** asTend);	/* generate execution network */
#endif /* defined(RUN) || defined(TCP) */
extern int	iC_outNet(FILE * iFP, char * outfile);	/* generate network as C file */
extern int	iC_c_compile(FILE * iFP, FILE * oFP, int flag, List_e * lp);
extern int	iC_copyXlate(FILE * iFP, FILE * oFP, char * outfile, unsigned * lcp, int mode);

					/*   lexc.l   */
extern size_t	c_leng;			/* defined in lexc.c */
extern int	column;
extern int	gramOffset;		/* count input in lex */
extern void	delete_sym(Token* tokp);
#ifndef LMAIN
extern void	markParaList(Symbol * sp); /* mark Symbol as parameter on glist */
extern void	clearParaList(int flag); /* clear parameter list from extraneous entries */
#endif
					/*   gram.y   */
extern int	c_parse(void);		/* generated yacc parser function */
extern void	copyAdjust(FILE* iFP, FILE* oFP, List_e* lp);
#endif	/* COMP_H */

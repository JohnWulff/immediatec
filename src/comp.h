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
 *	comp.h
 *	header for icc compiler
 *
 *******************************************************************/

#ifndef COMP_H
#define COMP_H
static const char comp_h[] =
"@(#)$Id: comp.h,v 1.43 2004/02/21 17:29:11 jw Exp $";

#define NS		((char*)0)
#define	TSIZE		256
#define	IBUFSIZE	512
#define	IMMBUFSIZE	1024

#define	NOT	1		/* used in List_e.le_val */
#define FEEDBACK

typedef	struct	List_e {	/* list element */
    struct Symbol *	le_sym;
    unsigned int	le_val;
    struct List_e *	le_next;
    char *		le_first;
    char *		le_last;
} List_e;

typedef	struct Symbol {		/* symbol table entry */
    char *		name;	/* element name */
    unsigned char	type;	/* ARN AND OR LATCH FF CLK TIM ... */
    unsigned char	ftype;	/* ARITH GATE S_FF R_FF D_FF CLCK TIMR ... */
    List_e *		list;	/* output list pointer */
#ifdef FEEDBACK
    List_e *		elist;	/* feedback list pointer */
#endif	/* FEEDBACK */
#if ! YYDEBUG
    union {
#endif
	Gate *		gate;	/* AND OR */
	List_e *	blist;	/* used in compile */
	unsigned int	val;	/* used to hold function number etc. */
#if ! YYDEBUG
    } u;
#endif
    struct Symbol *	next;	/* to link to another */
} Symbol;

#if ! YYDEBUG
#define u_gate		u.gate
#define u_blist		u.blist
#define u_val		u.val
#else				/* easier for debugging */
#define u_gate		gate
#define u_blist		blist
#define u_val		val
#endif

/* for use in a union identical first elements can be accesed */
typedef struct { char *f; char *l; Symbol * v;     } Sym;
typedef struct { char *f; char *l; List_e * v;     } Lis;
typedef struct { char *f; char *l; unsigned int v; } Val;
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
#ifndef LMAIN
extern void yyerror(char * s);		/* called for yacc syntax error */
extern int  get(FILE* fp);		/* character input shared with lexc.l */
#endif
extern int	ynerrs;			/* count of iCerror() calls */
		/* NOTE iCnerrs is reset for every call to yaccpar() */

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
 *******************************************************************/
#define C_PARSE		01
#define C_FIRST		02
#define C_BLOCK		04
#define C_NO_COUNT	010
#define C_LINE		020
#define C_LINE1		040

extern int	lineno;
					/*   genr.c  */
extern int	c_number;		/* case number for cexe.c */
extern int	outFlag;		/* global flag for compiled output */
extern char *	cexeString[];		/* case or function string */
extern char	inpNM[];		/* currently scanned input file name */
extern char	iCbuf[];		/* buffer to build imm statement size IMMBUFSIZE */
extern char	iFunBuffer[];		/* buffer to build imm function symbols */
extern char *	iFunEnd;		/* pointer to end */
extern char *	iFunSymExt;		/* flags that function is being compiled */
extern Sym	iRetSymbol;		/* .v is pointer to imm function return Symbol */
extern Symbol * assignExpression(	/* assignment of an aexpr to a variable */
	    Sym * sv, Lis * lv, int ioTyp);
extern List_e *	delayOne(List_e * tp);	/* implicit delay of 1 tick for ctref : texpr ; */
extern Symbol *	functionHead(		/* set up the function definition head */
	    unsigned int typeVal, Symbol * funTrigger, int retFlag);
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
extern List_e *	op_xor(			/* special exclusive or push */
	    List_e *, List_e *);	/*   left, right   */
extern List_e *	op_not(List_e *);	/* logical negation */
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

					/*   symb.c   */
#define	HASHSIZ 54*16			/* for new sorted list algorithm */
extern Symbol *	symlist[];		/* symbol table: linked lists */
extern Symbol *	lookup(char *);		/* find string in symbol table */
extern Symbol *	place_sym(Symbol *);	/* place sp in symbol table */
extern Symbol *	install( char *, unsigned char,
		    unsigned char);	/* install Symbol in symbol table */
extern Symbol * unlink_sym(Symbol *);	/* unlink Symbol from symbol table */

					/*   main.c   */
#define Cname	"_list_.c"	/* unusual, not to be used for application name */
#define H1name	"_list1.h"
#define H2name	"_list2.h"

/* index 0 not used */
#define Iindex	1
#define Eindex	2
#define Lindex	3
#define Oindex	4
#define COindex	5
#define Cindex	6
#define H1index	7
#define H2index	8
#define T0index	9
#define T1index	10
#define T2index	11
#define T3index	12
#define T4index	13
#define T5index	14

#define INITIAL_FILE_NAMES	0, 0, 0, 0, 0, 0, Cname, H1name, H2name,\
				0, 0, 0, 0, 0, 0,

extern FILE *	T0FP;
extern FILE *	T1FP;
extern FILE *	T2FP;
extern FILE *	T3FP;
extern FILE *	T4FP;
extern FILE *	T5FP;

extern char	T0FN[];
extern char	T4FN[];
extern char	T5FN[];

					/*   outp.c   */
#define BUFS	128
extern int	IEC1131(char * name, char * buf, int bufLen,
			char * iqt, char * xbwl, int * bytep,
			int * bitp, char * tail);
extern int	toIEC1131(char * name, char * buf, int bufLen,
			  char * iqt, char * xbwl, int * bytep,
			  int * bitp, char * tail);

extern int	listNet(unsigned * gate_count);	/* list generated network */
extern int	buildNet(Gate ** igpp);	/* generate execution network */
extern int	output(char *);		/* generate network as C file */
extern int	c_compile(FILE *);
extern int	copyXlate(FILE *, char *, unsigned *, int);
					/*   lexc.l   */
extern int	c_leng;
extern int	column;
extern int	gramOffset;
extern void	delete_sym(Token* tokp);
					/*   gram.y   */
extern int	c_parse(void);		/* generated yacc parser function */
extern void	copyAdjust(FILE* iFP, FILE* oFP);
#endif	/* COMP_H */

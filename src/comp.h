static const char comp_h[] =
"@(#)$Id: comp.h,v 1.32 2002/08/17 22:07:35 jw Exp $";
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

#define NS	((char*)0)

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
    union {
	Gate *		gate;	/* AND OR */
	List_e *	blist;	/* used in compile */
	unsigned int	val;	/* used to hold function number etc. */
    } u;
    struct Symbol *	next;	/* to link to another */
} Symbol;

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
extern int  compile(char *, char *,
		    char *, char *);	/* compile iC language source */
extern void errmess(char *, char *, char *);	/* actual error message */
extern void error(char *, char *);	/* print error message */ 
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
extern int	lineno;
					/*   genr.c  */
extern int	c_number;		/* case number for cexe.c */
extern int	outFlag;		/* global flag for compiled output */
extern char *	cexeString[];		/* case or function string */
extern char *	inpNM;			/* original input file name */
extern char *	stmtp;			/* pointer into iCbuf */
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

extern Symbol *	iclock;			/*   init.c  */
extern void	init(void);		/* install constants and built-ins */

					/*   symb.c   */
#define	HASHSIZ 54*16			/* for new sorted list algorithm */
extern Symbol *	symlist[];		/* symbol table: linked lists */
extern void *	emalloc(unsigned);	/* check return from malloc */
extern Symbol *	lookup(char *);		/* find string in symbol table */
extern Symbol *	place_sym(Symbol *);	/* place sp in symbol table */
extern Symbol *	install( char *, unsigned char,
		    unsigned char);	/* install Symbol in symbol table */
extern Symbol * unlink_sym(Symbol *);	/* unlink Symbol from symbol table */

					/*   main.c   */
#define Cname	"_list_.c"	/* unusual, not to be used for application name */
#define H1name	"_list1.h"
#define H2name	"_list2.h"
#define TCname	"iCcTypes.def"

/* index 0 not used */
#define Iindex	1
#define Eindex	2
#define Lindex	3
#define Oindex	4
#define CIindex	5
#define COindex	6
#define Cindex	7
#define H1index	8
#define H2index	9
#define T1index	10
#define T2index	11
#define T3index	12
#define T4index	13
#define TCindex	14

#define INITIAL_FILE_NAMES	0, 0, 0, 0, 0, 0, 0, Cname, H1name, H2name,\
				0, 0, 0, 0, TCname,

extern FILE *	T1FP;
extern FILE *	T2FP;
extern FILE *	T3FP;
extern FILE *	typeCacheFP;
extern char	typeCacheFN[];

					/*   outp.c   */
#define BUFS	128
extern int	IEC1131(char * name, char * buf, int bufLen,
			char * iqt, char * bwx, int * bytep,
			int * bitp, char * tail);
extern int	toIEC1131(char * name, char * buf, int bufLen,
			  char * iqt, char * bwx, int * bytep,
			  int * bitp, char * tail);

extern int	listNet(unsigned * gate_count);	/* list generated network */
extern int	buildNet(Gate ** igpp);	/* generate execution network */
extern int	output(char *);		/* generate network as C file */
extern int	c_compile(FILE *);
extern int	copyXlate(FILE *, char *, unsigned *, int);
					/*   lexc.l   */
extern int	c_leng;
extern int	column;
extern void	delete_sym(Token* tokp);
extern int	readTypeCacheFile(void);
extern int	readTypesFromCache(char* includeName);
#define TCBUFS	1024
					/*   gram.y   */
extern void	copyAdjust(FILE* iFP, FILE* oFP);

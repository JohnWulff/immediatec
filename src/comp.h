static const char comp_h[] =
"@(#)$Id: comp.h,v 1.18 2001/03/11 15:10:19 jw Exp $";
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

#ifndef YYMAXDEPTH
#define YYMAXDEPTH (PPGATESIZE*3)		/* yacc stack size */
#endif

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
    uchar		type;	/* ARN AND OR LATCH FF CLK TIM ... */
    uchar		ftype;	/* ARITH GATE S_FF R_FF D_FF CLCK TIMR ... */
    List_e *		list;	/* output list pointer */
    union {
	Gate *		gate;	/* AND OR */
	List_e *	blist;	/* used in compile */
	unsigned int	val;	/* used to hold function number etc. */
    } u;
    struct Symbol *	next;	/* to link to another */
} Symbol;

typedef struct { Symbol * v;	char *f; char *l; }	Sym;
typedef struct { List_e * v;	char *f; char *l; }	Lis;
typedef struct { unsigned int v; char *f; char *l; }	Val;
typedef struct { char v[2];	char *f; char *l; }	Str;

extern int	compile( char *, char *,
    char *, char *, char *, char *);	/* compile iC language source */
extern void error(char *, char *);	/* print error message */ 
extern void warning(char *, char *);	/* print warning message */
extern void execerror(char *, char *);	/* recover from run-time error */ 
extern int	ynerrs;			/* count of yyerror() calls */
		/* NOTE yynerrs is reset for every call to yaccpar() */

extern int	lineno;			/*   genr.c  */
extern int	c_number;		/* case number for cexe.c */
extern int	outFlag;		/* global flag for compiled output */
extern FILE *	exoFP;			/* cexe out file pointer */
extern char *	cexeString[];		/* case or function string */
extern char *	inpNM;			/* original input file name */
extern char *	stmtp;			/* pointer into yybuf */
extern void	initcode(void);		/* initialize for code generation */
extern List_e *	sy_push(Symbol *);	/* create List element for variable */
extern Symbol *	sy_pop(List_e *);	/* delete List element left over */
extern List_e * op_force(		/* force linked Symbol to ftype */
	    List_e *, uchar);		/*   lp, ftyp    */
extern List_e *	op_push(		/* reduce List_e stack to links */
	    List_e *, uchar, List_e *);	/*   left, typ, right   */
extern int	const_push(Lis * expr);	/* numeric constant push */
extern List_e *	op_xor(			/* special exclusive or push */
	    List_e *, List_e *);	/*   left, right   */
extern List_e *	op_not(List_e *);	/* logical negation */
extern Symbol *	op_asgn(		/* asign List_e stack to links */
	    Sym *, Lis *, uchar);	/*   var, right, ftyp   */
extern List_e * bltin(			/* generate built in iC functions */
	    Sym* sym, Lis* ae1, Lis* cr1, Lis* ae2, Lis* cr2, Lis* tr, Val* pVal);

extern Symbol *	clk;			/*   init.c  */
extern void	init(void);		/* install constants and built-ins */

					/*   symb.c   */
#define	HASHSIZ 54*16			/* for new sorted list algorithm */
extern Symbol *	symlist[];		/* symbol table: linked lists */
extern void *	emalloc(unsigned);	/* check return from malloc */
extern Symbol *	lookup(char *);		/* find string in symbol table */
extern Symbol *	place_sym(Symbol *);	/* place sp in symbol table */
extern Symbol *	install(
		char *, uchar, uchar);	/* install Symbol in symbol table */
extern Symbol * unlink_sym(Symbol *);	/* unlink Symbol from symbol table */

					/*   main.c   */
#define Tname	"_cexe_.tmp"
#define Cname	"_list_.c"	/* unusual, not to be used for application name */
#define Hname	"_list1.h"
#define Lname	"_list2.h"
					/*   outp.c   */
#define BUFS	128
extern int	IEC1131(char * name, char * buf, int bufLen,
			char * iqt, char * bwx, int * bytep,
			int * bitp, char * tail);

extern int	listNet(unsigned * gate_count);	/* list generated network */
extern int	buildNet(Gate ** igpp);	/* generate execution network */
extern int	output(char *);		/* generate network as C file */
extern void	copyXlate(FILE * iFP, FILE * oFP, unsigned * lcp, int mode);

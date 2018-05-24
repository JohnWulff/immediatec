%{ static const char comp_y[] =
"@(#)$Id: comp.y 1.129 $";
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
 *	comp.y
 *	grammar for immcc compiler
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/stat.h>
#ifdef WIN32
#include	<process.h>
#else	/* WIN32 */
#include	<unistd.h>
#endif	/* WIN32 */
#include	<string.h>
#include	<ctype.h>
#include	<assert.h>
#include	<errno.h>

#include	"comp.h"
#include	"comp.tab.h"

static int	yylex(void);		/* lexer for immcc renamed iClex() */
static void	unget(int);		/* shares buffers with get() */
int		ynerrs;			/* count of yyerror() calls */
int		gnerrs;			/* count of ierror() calls */
		/* NOTE iCnerrs is reset for every call to yaccpar() */
static int	copyCfrag(char, char, char, FILE*);	/* copy C action */
static void	ffexprCompile(char *, List_e *, int);	/* c_compile cBlock */
static void	blockUnblockListing(void);
static List_e *	op_adjust(Lis * lv);	/* adjust logic expression with constant */
static unsigned char ccfrag;		/* flag for CCFRAG syntax */
static int	cBlockFlag;		/* flag to contol ffexpr termination */
static FILE *	ccFP;			/* FILE * for CCFRAG destination */
static Type	stype;			/* to save TYPE in decl */
%ifdef	BOOT_COMPILE
static Val	val1 = { 0, 0, 1, };	/* preset off 1 value for TIMER1 */
static void	errBit(void);
%endif	/* BOOT_COMPILE */
static Symbol	nSym = { "", ERR, GATE, };
static int	cFn = 0;
static char *	TaliasList[9][2] = {
	    { "EOI",	"TX0.0"  },	// end of initialisation
	    { "STDIN",	"TX0.1"  },	// stdin line received
	    { "LO",	"TX0.2"  },	// constant bit LO
	    { "HI",	"~TX0.2" },	// constant bit HI
	    { "T10ms",	"TX0.3"  },	// 10 ms internal timer
	    { "T100ms",	"TX0.4"  },	// 100 ms internal timer
	    { "T1sec",	"TX0.5"  },	// 1 second internal timer
	    { "T10sec",	"TX0.6"  },	// 10 second internal timer
	    { "T1min",	"TX0.7"  },	// 1 minute internal timer
};

#ifndef EFENCE
char		iCbuf[IMMBUFSIZE];	/* buffer to build imm statement */
char		iFunBuffer[IBUFSIZE];	/* buffer to build imm function symbols */
char *		iFunEnd = &iFunBuffer[IBUFSIZE];	/* pointer to end */
#else		/* malloc for EFENCE in main() */
char *		iCbuf;			/* buffer to build imm statement */
char *		iFunBuffer;		/* buffer to build imm function symbols */
char *		iFunEnd;		/* pointer to end */
#endif	/* EFENCE */

char *		iFunSymExt = 0;		/* pointer to imm function symbol Extension */
static char *	iFunSyText = 0;		/* pointer to function symbol text when active */
Sym		iRetSymbol;		/* .v is pointer to imm function return Symbol */
char *		iCstmtp;		/* manipulated in yylex() (reset in clrBuf()) */
static void	clrBuf(void);
%}

%union {		/* stack type */
    Sym		sym;			/* symbol table pointer */
    Lis		list;			/* linked list elements */
    Val		vai;			/* int numerical values */
    Vap		vap;			/* union { int, Symbol* } */
    Typ		typ;			/* type, ftype, em and fm */
    /* allows storing character as iClval.vai.v (2nd byte is NULL) */
    /* then char array can be referenced as $1.v, single char as $1.v[0] */
}

%{
static void
clrBuf(void)
{
    iCstmtp = iCbuf;
    memset(iCbuf, '\0', IMMBUFSIZE);
} /* clrBuf */

#if YYDEBUG
/********************************************************************
 *
 *	Detailed logic generation debugging output for different stack types
 *	usually only called if ((iC_debug & 0402) == 0402)
 *				pu(SYM, "dasgn: decl = aexpr", &$$);
 *		Parameter 1:	one of SYM LIS VAL VAP or TYP
 *		Parameter 2:	a message - usually the grammar rule
 *		Parameter 3:	a pointer to the full yacc stack type token
 *
 *	Each stack type token has identical first elements
 *		char * f	// start of generated compiler text
 *		char * l	// last+1 of generated compiler text
 *	The compiler text for this token is output after the name or value
 *	of the token itself which is in:
 *		Symbol * v	// Symbol name
 *		List_e * v	// name of Symbol List_e points to
 *		int      v	// numerical value
 *		Valp     v	// numerical value or List_e*
 *	or	Type     v	// type information
 *
 *******************************************************************/

void
pu(enum stackType t, const char * token, void * node)
{
    char *	t_first;
    char *	t_last;
    int		len;

    switch (t) {
    case SYM:
	if (((Sym*)node)->v) {
	    fprintf(iC_outFP, ">>>Sym	%s	%s :	", token, ((Sym*)node)->v->name);
	} else {
	    fprintf(iC_outFP, ">>>Sym	%s	0 =	", token);
	}
	break;
    case LIS:
	if (((Lis*)node)->v) {
	    fprintf(iC_outFP, ">>>Lis	%s	%s =	", token, ((Lis*)node)->v->le_sym ? ((Lis*)node)->v->le_sym->name : "@0");
	} else {
	    fprintf(iC_outFP, ">>>Lis	%s	0 =	", token);
	}
	break;
    case VAL:
	fprintf(iC_outFP, ">>>Val	%s	%d =	", token, ((Val*)node)->v);
	break;
    case VAP:
	if (((Vap*)node)->v.lfl == 0) {
	    fprintf(iC_outFP, ">>>Vap	%s	%d =	", token, ((Vap*)node)->v.nuv);
	} else
	if (((Vap*)node)->v.lfl == 1) {
	    fprintf(iC_outFP, ">>>Vap	%s *	%s =	", token, ((Vap*)node)->v.lpt->le_sym ? ((Vap*)node)->v.lpt->le_sym->name : "@0");
	} else {
	    fprintf(iC_outFP, ">>>Vap	%s *	error =	", token);
	}
	break;
    case TYP:
	/* single character for type, . normal or - external, ftype */
	fprintf(iC_outFP, ">>>Typ	%s	%c%c %c =	", token,
	    iC_os[((Typ*)node)->v.type],  iC_os[((Typ*)node)->v.em  & EM], iC_fos[((Typ*)node)->v.ftype]);
	break;
    }
    len = (t_last = ((Sym*)node)->l) - (t_first = ((Sym*)node)->f);
    if (len > 0 && t_first >= iCbuf && t_first < &iCbuf[IMMBUFSIZE] && t_last >= iCbuf && t_last < &iCbuf[IMMBUFSIZE]) {
	fprintf(iC_outFP, "%-*.*s\n", len, len, t_first);
    } else if (len != 0) {
	fprintf(iC_outFP, "\n*** Error: CODE GENERATION len = %d, f = %p l = %p iCbuf[%d] %p\n",
	    len, t_first, t_last, IMMBUFSIZE, iCbuf);
    } else {
	fprintf(iC_outFP, "\n");
    }
    fflush(iC_outFP);
} /* pu */

static void
pd(const char * token, Symbol * ss, Type s1, Symbol * s2)
{
    fprintf(iC_outFP, ">>>%s\t%s\t[%c%c %c] [%c%c %c] => [%c%c %c]\n",
	token, s2->name,
	/* single character for type, . normal or - external, ftype */
	iC_os[s1.type],  iC_os[s1.em  & EM], iC_fos[s1.ftype],
	iC_os[s2->type], iC_os[s2->em & EM], iC_fos[s2->ftype],
	iC_os[ss->type], iC_os[ss->em & EM], iC_fos[ss->ftype]);
    fflush(iC_outFP);
} /* pd */

#endif
%}

	/************************************************************
	 *
	 * iC token types
	 *
	 ***********************************************************/

%token	<sym>	UNDEF IMMCARRAY AVARC AVAR AOUT LVARC LVAR LOUT CVAR TVAR
%ifdef	BOOT_COMPILE
%token	<sym>	BFORCE BLTIN1 BLTINC BLTINS BLTIN2 BLTIN3 CBLTIN TBLTIN TBLTI1
%endif	/* BOOT_COMPILE */
%token	<sym>	IF ELSE SWITCH EXTERN ASSIGN RETURN USE USETYPE IMM IMMC VOID CONST TYPE SIZEOF
%token	<sym>	IFUNCTION CFUNCTION TFUNCTION VFUNCTION CNAME
%token	<vai>	CCFRAG NUMBER
%token	<vai>	LEXERR COMMENTEND LHEAD BE
%type	<sym>	program statement funcStatement simpleStatement asgnStatement iFunDef iFunHead
%type	<sym>	return_statement formalParameter lBlock dVariable dIVariable dIMVariable dMVariable
%type	<sym>	dCVariable valueVariable valueIVariable valueCVariable cVariable outVariable
%type	<sym>	useFlag decl declC extDecl immDecl immCDecl asgn dasgn casgn dcasgn tasgn dtasgn
%type	<sym>	iFunTrigger vFunCall rParams rPlist immT
%type	<sym>	extCdecl dVar immCarray immCarrayHead extimmCarray extimmCarrayHead immCarrayVariable
%type	<list>	expr aexpr cexpr texpr actexpr comma_expr cexpr_comma_expr texpr_comma_expr ractexpr
%ifdef	BOOT_COMPILE
%type	<list>	lexpr fexpr cfexpr tfexpr
%endif	/* BOOT_COMPILE */
%type	<list>	cref ctref ctdref cCall cParams cPlist dPar immCini immCiniList ifini ffexpr
%type	<list>	fParams fPlist funcBody iFunCall cFunCall tFunCall
%type	<vai>	cBlock dParams dPlist Constant
%type	<vap>	immCindex constExpr
%type	<typ>	declHead extDeclHead declCHead extCdeclHead formalParaTypeDecl formalConstTypeDecl
%type	<vai>	'{' '"' '\'' '}'	/* C/C++ brackets */
%left	<vai>	','		/* comma, function seperator */
%right	<vai>	'=' OPE
%right	<vai>	'?' ':'		/* ? : */
%left	<vai>	B_OR		/* || */
%left	<vai>	B_AND 		/* && */
%left	<vai>	'|'		/* logical or - must do before arithmetic operators */
%left	<vai>	'^'		/* logical exclusive or */
%left	<vai>	'&'		/* logical and */
%left	<vai>	EQU		/* equality operators == != */
%left	<vai>	CMP		/* comparison operators < <= > >= */
%left	<vai>	LEFT_SH RIGHT_SH/* arithmetic operators << >> */
%left	<vai>	'+' '-'		/* arithmetic operators + - */
%left	<vai>	'*' DIV		/* arithmetic operators * / % */
%right	<vai>	'!' '~' PPMM	/* unary operators ! ~ ++ -- (+ - & *) */
%left	<vai>	'(' ')' '[' ']' PR	/* parentheses -> . */
%%

	/************************************************************
	 *
	 * iC grammar
	 *
	 ***********************************************************/

program	: /* nothing */		{
		$$.f = $$.l = 0;
		$$.v = 0;
		clrBuf();
	    }
	| program statement	{
		$$   = $2;
		$$.f = $$.l = 0;
		clrBuf();
	    }
	| program error ';'	{
		$$.f = $$.l = 0;
		$$.v = 0;
		clrBuf();
		yyerrok;
	    }
	;

statement
	: simpleStatement ';'	{ $$   = $1; }		/* immediate statement (may be empty) */
	| ffexpr BE		{			/* if or switch */
		$$.f = $1.f; $$.l = $1.l;
		$$.v = op_asgn(0, &$1, GATE);		/* returns 0 for missing slave gate in ffexpr */
	    }
	| lBlock		{ $$.v = 0;  }		/* literal block */
	| iFunDef		{ $$   = $1; }		/* immediate function definition */
	;

funcStatement
	: simpleStatement ';'	{ $$   = $1; }		/* immediate statement (may be empty) */
	| ffexpr BE		{			/* if or switch */
		$$.f = $1.f; $$.l = $1.l;
		$$.v = op_asgn(0, &$1, GATE);		/* returns 0 for missing slave gate in ffexpr */
		collectStatement($$.v);
	    }
	| return_statement ';'	{			/* function return statement */
		$$ = $1;
		collectStatement($1.v);
	    }
	;

simpleStatement
	: asgnStatement		{ $$   = $1; }		/* empty or all assignments and vFunCall */
	| useFlag		{ $$   = $1; }		/* use flags */
	| immDecl		{			/* imm declaration */
		$$   = $1;
		iFunSyText = 0;
	    }
	| immDecl ','		{			/* " new style for immac which can generate ,; */
		$$   = $1;
		iFunSyText = 0;
	    }
	| immCDecl		{			/* immC declaration */
		$$   = $1;
		iFunSyText = 0;
	    }
	| immCDecl ','		{			/* " new style for immac which can generate ,; */
		$$   = $1;
		iFunSyText = 0;
	    }
	| immCarray		{			/* declare and initialise an immC array */
		$$   = $1;
		iFunSyText = 0;
	    }
	| immCarray ','		{			/* " new style for immac which can generate ,; */
		$$   = $1;
		iFunSyText = 0;
	    }
	| extCdecl		{ $$   = $1; }		/* external C function declaration */
	| extCdecl ','		{ $$   = $1; }		/* " new style for immac which can generate ,; */
	| extDecl		{ $$   = $1; }		/* external declaration */
	| extDecl ','		{ $$   = $1; }		/* " new style for immac which can generate ,; */
	| extimmCarray		{ $$   = $1; }		/* declare an immC array extern */
	| extimmCarray ','	{ $$   = $1; }		/* " new style for immac which can generate ,; */
	;

asgnStatement			/* all declarations are not 'asgnStatement' because they may contain a ',' */
	: /* nothing */		{ $$.v =  0; }		/* empty statement or empty comma_expr element */
	| asgn			{ $$   = $1; }		/* immediate value assignment */
	| casgn			{ $$   = $1; }		/* clock assignment */
	| tasgn			{ $$   = $1; }		/* timer assignment */
	| vFunCall		{ $$   = $1; }		/* void function call */
	;

dVariable
	: valueVariable		{ $$   = $1; }		/* imm value variable LVAR AVAR LVARC AVARC */
	| outVariable		{ $$   = $1; }		/* output variable LOUT AOUT */
	| dMVariable		{ $$   = $1; }		/* misc variable CVAR TVAR Constant */
	;

dIVariable
	: dIMVariable		{ $$   = $1; }		/* imm value variable LVAR AVAR CVAR TVAR Constant */
	| outVariable		{ $$   = $1; }		/* output variable LOUT AOUT */
	;

dIMVariable
	: valueIVariable	{ $$   = $1; }		/* imm value variable LVAR AVAR */
	| dMVariable		{ $$   = $1; }		/* misc variable CVAR TVAR Constant */
	;

dMVariable
	: CVAR			{ $$   = $1; }		/* clock variable CVAR */
	| TVAR			{ $$   = $1; }		/* timer variable TVAR */
	| Constant		{			/* numeric constant */
		$$.f = $1.f; $$.l = $1.l;
		$$.v = 0;				/* no node, do not need value */
	    }
	;

dCVariable
	: valueCVariable	{ $$   = $1; }		/* imm value variable LVARC AVARC */
	| outVariable		{ $$   = $1; }		/* output variable LOUT AOUT */
	;

valueVariable
	: valueIVariable	{ $$   = $1; }		/* LVAR AVAR */
	| valueCVariable	{ $$   = $1; }		/* LVARC AVARC */
	;

valueIVariable
	: LVAR			{ $$   = $1; }		/* logical bit variable */
	| AVAR			{ $$   = $1; }		/* arithmetic variable */
	;

valueCVariable
	: LVARC			{ $$   = $1; }		/* logical C bit variable */
	| AVARC			{ $$   = $1; }		/* arithmetic C variable */
	;

outVariable
	: LOUT			{ $$   = $1; }		/* output bit variable */
	| AOUT			{ $$   = $1; }		/* output arith. variable */
	;

	/************************************************************
	 *
	 * Pragmas or USE flags, which postset options for compiling code
	 *
	 *	use alias;		// equivalent to -A option
	 *	no alias;		// turns off -A option
	 *
	 *	use strict;		// equivalent to -S option - default - JW 20160925
	 *	no strict;		// equivalent to -N option
	 *
	 *	use list;		// restore listing output from the next line - default
	 *	no list;		// suppresses listing output from the next line
	 *
	 *	use strict, alias;	// equivalent to -AS option
	 *	no alias, strict;	// turn off both options
	 *
	 * With 'use' and 'no' a particular option can be set and reset
	 * after compilation has started, overriding the compiler flag
	 * options.
	 *
	 * With 'use alias' code which is linked from several iC sources
	 * can be forced to produce ALIAS nodes, which are needed for
	 * loading the combined code, without having to worry about setting
	 * the -A flag when compiling. The -A compiler flag is still useful
	 * for generating extra ALIAS nodes for debugging with iClive.
	 *
	 * The 'use strict' statement is now optional, because since September
	 * 2016 the 'strict' flag is set by default. It is left for completeness
	 * to be able to turn the 'strict' flag on if for some reason it was
	 * turned off with 'no strict' or the -N command line option.
	 *
	 * Nevertheless very simple legacy iC programs with mainly bit nodes
	 * can still be compiled without the 'strict' flag by compiling such a
	 * program with the new -N command line option or writing 'no strict'.
	 *
	 * The 'no strict' option should be avoided and is only included for
	 * completeness. It should be very easy to add extra declarations to
	 * satisfy the 'strict' criterion.  But one never knows and as a
	 * language designer I feel this is a good way.
	 * (grateful acknowledgements to the designers of PERL)
	 *
	 * As noted earlier, C functions and macros should be declared
	 * extern with their correct parameter ramp and return value.
	 * When "strict" is active, error messages are output if an
	 * undeclared C function or macro is called in an immediate C
	 * expression.
	 *
	 * The remaining compiler flags are very specific. The following
	 * mechanism allows extension to those as well.
	 *
	 * The scope of these pragmas is a file. If a pragma is enabled in
	 * one file it carries over to an included iC header file. If on the
	 * other hand a pragma is changed in a header file, it reverts to its
	 * previous value in the iC file after the #include statement, which
	 * includes the header file. This makes sure that sloppy iC programs,
	 * which include a header file, which uses "strict" syntax, will not
	 * report errors, because they do not follow the "strict" syntax.
	 *
	 * The 'no list' pragma stops listing output from the next line
	 * until a 'use list' statement starts listing output again.
	 * This pragma would mainly be used to suppress listing output of
	 * function block definitions, which may be regarded as clutter.
	 * Typical use:
	 *
	 *	no list;		// %include "adconvert.ih"
	 *	%include "adconvert.ih"
	 *	use list;
	 *
	 * Listing output is the 'no list' line only. The comment is
	 * recommended telling what has not been listed, which is the
	 * whole of the 'adconvert.ih' file plus the 'use list' line.
	 *
	 ***********************************************************/

useFlag	: USE USETYPE		{
		unsigned int usetype, use;
		$$ = $1;
		use = $1.v->ftype;			/* 0=no or 1=use */
		assert(use <= 1);
		usetype = $2.v->ftype;			/* 0=alias 1=strict 2=list */
		assert(usetype < MAXUSETYPE);
		if (use) {
		    iC_uses |= 1<<usetype;		/* set iC_Aflag or iC_Sflag or iC_Zflag */
		} else {
		    iC_uses &= ~(1<<usetype);		/* reset iC_Aflag or iC_Sflag or iC_Zflag */
		}
		blockUnblockListing();			/* block or unblock listing depending on iC_Zflag */
	    }
	| useFlag ',' USETYPE	{
		unsigned int usetype, use;
		$$ = $1;
		use = $1.v->ftype;			/* 0=no or 1=use */
		assert(use <= 1);
		usetype = $3.v->ftype;			/* 0=alias 1=strict 2=list */
		assert(usetype < MAXUSETYPE);
		if (use) {
		    iC_uses |= 1<<usetype;		/* set iC_Aflag or iC_Sflag or unblock list output */
		} else {
		    iC_uses &= ~(1<<usetype);		/* reset iC_Aflag or iC_Sflag or block list output */
		}
		blockUnblockListing();
	    }
	;

	/********************************************************************
	 *
	 * Extern C int variable, function or macro declaration
	 *
	 *	extern int var;			// C variable to use in an imm expression
	 *	extern int rand();		// C function with no parameters
	 *	extern int rand(void);		// alternative syntax
	 *	extern int abs(int);		// C function with 1 parameter
	 *	extern int min(int, int);	// macro with 2 parameters
	 *
	 * When 'strict' is active, any C variables, functions or macros, which
	 * are used in immediate expressions must be declared extern in the iC code.
	 * Since it is easy to mistype the names of iC function blocks and such
	 * non-defined function blocks will be compiled without error as a C
	 * function call, the error is not discovered until link time. With extern
	 * declarations clean error messages are produced and the extra effort is
	 * not great. When a C function or macro is called in an immediate expression,
	 * a check is made, that the number of parameters is correct. Otherwise an
	 * error message is issued. No check is made for C calls in C fragments
	 * controlled by if else or switch statements or other literal C code,
	 * since the compilation is handled by the follow up C compiler.
	 *
	 * In principle this information could be retrieved from the embedded
	 * C code, but in practice this gets extremely messy.
	 *
	 * The following declaration errors will evoke a warning if 'strict'
	 *
	 *	extern bit rand()		// wrong return type
	 *
	 * The following declaration errors will evoke warnings or errors if 'strict'
	 *
	 *	extern int var()		// was previously declared as a variable
	 *	extern int rand			// was previously declared as a function
	 *	extern clock rand()		// absolutely wrong return type
	 *	extern timer rand()		// absolutely wrong return type
	 *
	 *******************************************************************/

extCdecl
	: extCdeclHead UNDEF			{
		if ($2.v->ftype == UDFA) {		/* if not previously declared as imm */
		    $2.v->type = CWORD;			/* no longer an imm variable */
		    $2.v->u_val = CNAME;		/* yacc type */
		    $2.v->v_cnt = (unsigned int)-1;	/* mark as variable */
		    $$.v = $2.v;
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) {
			fprintf(iC_outFP, "extCdecl: %s is a variable\n", $2.v->name);
			fflush(iC_outFP);
		    }
#endif
		} else {
		    if (iC_Sflag) {
			ierror("strict: erroneous declaration of a C variable that was imm:", $2.v->name);
		    } else {
			warning("erroneous declaration of a C variable that was imm:", $2.v->name);
		    }
		    $$.v = 0;
		}
	    }
	| extCdeclHead UNDEF	'(' dParams ')'	{
		$2.v->type = CWORD;			/* no longer an imm variable */
		$2.v->u_val = CNAME;			/* yacc type */
		$2.v->v_cnt = (unsigned int)($4.v);	/* parameter count */
		$$.v = $2.v;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "extCdecl: %s has %d parameters\n", $2.v->name, (int)$4.v);
		    fflush(iC_outFP);
		}
#endif
	    }
	| extCdeclHead UNDEF '(' dParams error ')'	{
		$2.v->type = CWORD;			/* no longer an imm variable */
		$2.v->u_val = CNAME;			/* yacc type */
		$2.v->v_cnt = $4.v;			/* parameter count */
		$$.v = $2.v;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "extCdecl: %s has %d parameters\n", $2.v->name, (int)$4.v);
		    fflush(iC_outFP);
		}
#endif
		yyerrok;
	    }
	| extCdeclHead CNAME			{
		if (iC_Sflag) {
		    ierror("strict: erroneous re-declaration of a C function - no ( parameters ):", $2.v->name);
		} else {
		    warning("erroneous re-declaration of a C function - no ( parameters ):", $2.v->name);
		}
		$$.v = 0;
	    }
	| extCdeclHead CNAME '(' dParams ')'	{
		if ($2.v->v_cnt != $4.v) {
		    char	tempBuf[TSIZE];
		    snprintf(tempBuf, TSIZE, "%s %d (%d)", $2.v->name, (int)$4.v, (int)$2.v->v_cnt);
		    if (iC_Sflag) {
			ierror("strict: parameter count does not match a previous C function declaration:", tempBuf);
		    } else {
			warning("parameter count does not match a previous C function declaration - ignore:", tempBuf);
		    }
		}
		$$.v = $2.v;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "extCdecl: %s has %d parameters\n", $2.v->name, (int)$4.v);
		    fflush(iC_outFP);
		}
#endif
	    }
	| extCdeclHead CNAME '(' dParams error ')'	{
		if ($2.v->v_cnt != $4.v) {
		    char	tempBuf[TSIZE];
		    snprintf(tempBuf, TSIZE, "%s %d (%d)", $2.v->name, (int)$4.v, (int)$2.v->v_cnt);
		    if (iC_Sflag) {
			ierror("strict: parameter count does not match a previous C function declaration:", tempBuf);
		    } else {
			warning("parameter count does not match a previous C function declaration - ignore:", tempBuf);
		    }
		}
		$$.v = $2.v;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "extCdecl: %s has %d parameters\n", $2.v->name, (int)$4.v);
		    fflush(iC_outFP);
		}
#endif
		yyerrok;
	    }
	;

extCdeclHead
	: EXTERN TYPE		{
		if ($2.v->ftype != ARITH) {
		    if ($2.v->ftype == GATE) {
			warning("type of an extern C declaration is bit not int - ignore:", $1.v->name);
		    } else
		    if (iC_Sflag) {
			ierror("strict: incompatible type for an extern C declaration:", $2.v->name);
		    } else {
			warning("incompatible type for an extern C declaration:", $2.v->name);
		    }
		}
		stype.ftype = ARITH;
		stype.type  = ARN;
		stype.em    |= EM;		/* set em for extern declaration */
		stype.fm    = 0;
		$$.v = stype;
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "extCdeclHead: EXTERN TYPE", &$$);
#endif
	    }
	| extCdecl ','	{
		$$.v = stype;	/* first TYPE */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "extCdeclHead: extCdecl", &$$);
#endif
	    }
	;

dParams	: /* nothing */		{ $$.v =  0; }
	| VOID			{ $$.v =  0; }
	| dPlist		{ $$   = $1; }	/* do not allow extra comma for C parameters */
	;

dPlist	: dPar			{ $$.v = 1; }
	| dPlist ',' dPar	{ $$.v = $1.v + 1; }
	;

dPar	: TYPE dVar		{
		if ($1.v->ftype != ARITH) {
		    if ($1.v->ftype == GATE) {
			warning("parameter type in an extern C function declaration is bit not int - ignore:", $1.v->name);
		    } else
		    if (iC_Sflag) {
			ierror("strict: incompatible parameter type in an extern C function declaration:", $1.v->name);
		    } else {
			warning("incompatible parameter type in an extern C function declaration:", $1.v->name);
		    }
		}
		$$.v = 0;
	    }
	;

dVar	: /* nothing */		{ $$.v = 0; }
	| UNDEF			{
		if ($1.v->ftype == UDFA) {	/* if not previously declared as imm */
		    uninstall($1.v);		/* delete dummy Symbol */
		}
		$$.v = 0;
	    }
	| dVariable		{ $$.v = 0; }	/* since this is a dummy use any word or number */
	;

	/************************************************************
	 *
	 * Extern type declarations - no assignment in the same source
	 *
	 *	extern imm bit   b1;	extern imm int   a1;
	 *	extern imm clock c1;	extern imm timer t1;
	 *
	 * The extern type declaration in iC declares that an immediate
	 * variable will be assigned in another module and may be used
	 * as an rvalue in immediate expressions in iC or C code in this
	 * module.
	 *
	 * Because of the single assignment rule, such an extern immediate
	 * variable may normally not be assigned in this module. An exception
	 * is an immediate variable assigned in C statements. If such an
	 * immediate variable is declared extern, a special form must be used:
	 *
	 *	extern immC bit  b2;	extern immC int  a2;
	 *
	 * Variables declared extern with the special type modifier 'immC'
	 * may be used as rvalues in iC or C code and may be assigned in
	 * C statements in this module.
	 *
	 * To allow lists of extern immediate declarations of all variables
	 * in a common include file, a simple immediate type declaration
	 * after the extern immediate declaration (or after the #include
	 * statement containing the extern immeditate type declaration)
	 * declares that this variable will be assigned in the current
	 * module and must be assigned in this module. The type (bit, int,
	 * clock or timer) must match the type used previously in the
	 * extern declaration.
	 *
	 * The use of 'immC' in a simple type declaration defines the Gate
	 * object, like in C. It must match the previous 'extern immC' type.
	 * In any other context 'immC' is equivalent to 'imm'.
	 *
	 * IEC-1131 input and output variables are pre-declared for iC code
	 * and normally do not need to be declared except for the following
	 * cases:
	 *
	 * An 'extern imm' type declaration of an input or output variable
	 * is needed if that I/O variable is used in iC or C code in this
	 * module, but is defined in iC code in another module. If the
	 * declaration is 'extern immC' the output variable may be assigned
	 * in C code but not in iC code.
	 *
	 * See * Immediate type declaration * for more details
	 *
	 * If a variable is declared extern in several sources which will
	 * later be linked and that variable is erroneously declared and
	 * assigned, either in immediate assignments or C assignments in
	 * more than one source module, a linker error will occurr.
	 * (Multiple definition of the variable in C code).
	 *
	 * If a variable is declared extern and is never assigned in this
	 * module (after a simple declaration) or in another module,
	 * then a linker error will occurr. (Undefined reference).
	 *
	 * In the extern type declaration, bits EM and EX are set in em,
	 * declaring that this variable was declared extern. If it is
	 * declared again locally, EM is cleared but EX is not. This is
	 * used in iC_listNet to check usage and definition of the variable.
	 *      EM set	variable can only be defined in another module;
	 *              therefore no undefined warning in this module.
	 *      EX set	variable may be used in another module;
	 *              therefore no unused warning in this module.
	 * This also applies to 'extern immC' array declarations.
	 *
	 * See use of extern type declarations in function block definitions
	 * under * DefineFunction * in genr.c
	 * In particulat iFunSyText is not set for an extDecl and thus the
	 * variables declared extern keep their unchanged global names.
	 *
	 * If an extDecl occurrs in a function block definition, a check is
	 * made that the name does not clash with one of the local names in
	 * the function block.
	 *
	 ***********************************************************/

extDecl	: extDeclHead UNDEF	{
		$$.v = $2.v;
		$$.v->ftype = $1.v.ftype;
		$$.v->type  = $1.v.type;
		assert(($1.v.em & (EM|EX)) == (EM|EX));	/* has been set in extDeclHead */
		$$.v->em    |= EM|EX;		/* set em for extern declaration */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    pd("extDecl", $$.v, $1.v, $2.v);
		}
#endif
	    }
	| extDeclHead dVariable	{		/* LVAR AVAR LVARC AVARC LOUT AOUT CVAR TVAR Constant */
		int		typ1;
		int		typ2;
		int		ftyp;
		char *		cp;
		Symbol *	sp;
		typ1  = $1.v.type;		/* UDF for all TYPEs except ARNC LOGC INPX INPW */
		sp = $$.v = $2.v;
		if (sp) {
		    assert(($1.v.em & (EM|EX)) == (EM|EX));	/* has been set in extDeclHead */
		    if (((typ2 = sp->type) == INPW || typ2 == INPX) &&
			(typ1 == ARNC || typ1 == LOGC) &&
			sp->type != ERR) {
			ierror("extern immC declaration of an input variable is invalid:", sp->name);
		    } else
		    if (sp->ftype != (ftyp = $1.v.ftype) && sp->type != ERR) {
			ierror("extern declaration does not match previous declaration:", sp->name);
			sp->type = ERR;	/* cannot execute properly */
		    }
		    if (iFunSymExt && (cp = strchr(sp->name, '@'))) {
			warning("extern declaration of internal function variable - ignored:", cp+1);
		    } else
		    if (typ2 == UDF ||			/* extern of unused imm or QXx.y QBz */
			typ2 == INPW || typ2 == INPX ||	/* or IXx.y IBz */
			((sp->em & EM) && typ2 != ERR)) {	/* or prev extern but not ERROR */
			if (typ2 != INPW && typ2 != INPX) {
			    if ((sp->em & EO) == 0) {
				sp->ftype = ftyp;	/* adjust ftype from extern type unless QXx.y QBz */
			    }
			    sp->type  = typ1;		/* adjust type UDF unless ARNC or LOGC */
			}
			sp->em    |= EM|EX;		/* set em for extern declaration */
		    } else
		    if (iFunSymExt) {
			ierror("extern declaration in function definition after assignment:", sp->name);
			sp->type = ERR;	/* stop use as a statement in function */
			sp->em   |= EM|EX;
		    } else if (typ2 == ARNC || typ2 == LOGC) {
			warning("extern immC declaration after definition - ignored:", sp->name);
		    }
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) {
			pd("extDecl", $$.v, $1.v, $2.v);
		    }
#endif
		} else {
		    ierror("direct extern declaration of a number is illegal:", $2.f);
		}
	    }
	;

immT	: IMM			{ $$   = $1; }		/* imm */
	| IMMC			{ $$   = $1; }		/* immC */
	;

extDeclHead
	: EXTERN immT TYPE	{
		int		typ;
		int		ftyp;
		ftyp = $3.v->ftype;		/* ARITH GATE CLCKL TIMRL */
		if ($2.v->ftype == 1) {		/* ftype field in IMMC for immC */
		    if (ftyp == ARITH) {
			typ = ARNC;		/* immC int */
		    } else
		    if (ftyp == GATE) {
			typ = LOGC;		/* immC bit */
		    } else {
			warning("extern declaration of an immC type other than bit or int - ignore:", $3.v->name);
			goto extImmType;
		    }
		} else {			/* immT is imm */
		  extImmType:
		    if (ftyp >= CLCKL) {	/* check that CLCKL TIMRL */
			ftyp -= CLCKL - CLCK;	/* and CLCK TIMR are adjacent */
		    }
		    typ = iC_types[ftyp];	/* ARN OR CLK TIM */
		}
		stype.ftype = $3.v->ftype;	/* ftyp has changed for CLCKL and TIMRL */
		stype.type  = typ;
		stype.em    |= EM|EX;		/* set em for extern declaration */
		stype.fm    = 0;
		$$.v = stype;
		iFunSyText = 0;			/* no function symbols for extern */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "extDeclHead: EXTERN immT TYPE", &$$);
#endif
	    }
	| extDecl ','		{
		$$.v = stype;			/* first TYPE */
		iFunSyText = 0;			/* no function symbols for extern */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "extDeclHead: extDecl ,", &$$);
#endif
	    }
	| extimmCarray ','	{
		$$.v = stype;			/* first TYPE */
		iFunSyText = 0;			/* no function symbols for extern */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "extDeclHead: extimmCarray ,", &$$);
#endif
	    }
	;

	/************************************************************
	 *
	 * Immediate type declaration - may be combined with dasgn
	 *
	 *	imm bit   b1;		 imm int   a1;
	 *	imm clock c1;		 imm timer t1;
	 *
	 *	imm bit   b2 = d & e;	 imm int   a2 = a1 * 2;
	 *	imm clock c2 = CLOCK(x); imm timer t2 = TIMER(y);
	 *
	 * immediate variables declared with 'imm' must be assigned once
	 * and only once in the current source module (usually directly
	 * after the declaration like a C initalization, unless the
	 * declaration is a forward declaration, which is frequently
	 * necessary). Alternatively a variable declared with a simple
	 * 'imm' may be assigned in C code if 'no strict' mode has
	 * been specified (or -N on the command line). Such a C assignment
	 * to an 'imm' variable is deprecated - use 'immC' instead.
	 * Multiple declarations of the same variable with the same
	 * immediate type are silently ignored.
	 *
	 * IEC-1131 input and output variables are pre-declared for iC code
	 * and C code normally do not need to be declared except for the
	 * following cases:
	 *
	 * An 'imm' type declaration of an input or output variable is needed
	 * if that variable has already been declared with an 'extern imm'
	 * declaration in this source module (usually in an included .ih header)
	 * and its storage is to be defined in this source. This also means for
	 * declared output variables that they should be assigned in this source.
	 *
	 * These rules for input and output variables are the same as for
	 * ordinary immediate variables, except that an I/O variable which
	 * has not been declared 'extern imm' does not need to be declared
	 * at all (defined when used in iC code and C code by default) except
	 * that output variables which are to be assigned in C code must
	 * be declared with 'immC' like ordinary immediate variables, even
	 * when they have not been previously declared with 'extern immC'.
	 *
	 * declHead has type UDF for all TYPEs except ARNC and LOGC, which
	 * are the TYPEs given to completed 'immC' Gate objects.
	 *
	 ***********************************************************/

decl	: declHead UNDEF	{
		Symbol *	sp;
		$$.f = $1.f; $$.l = $2.l;
		sp = $$.v = $2.v;
		sp->ftype = $1.v.ftype;		/* bit int clock timer */
		sp->type  = $1.v.type;
		assert(($1.v.em & EM) == 0);	/* has been cleared in declHead */
		sp->em    &= ~EM;		/* clear only bit EM here, leave bit EX */
		assert(sp->type == UDF);
		if (iFunSymExt) {
		    char *	cp;
		    Symbol *	sp1;
		    cp = strchr(sp->name, '@'); /* locate original extension */
		    assert(cp && isprint(cp[1])); /* extension must be at least 1 character */
		    if ((sp1 = lookup(++cp)) != 0 && (sp1->em & EM)) {
			warning("declaration of an extern variable in a function - ignored:", cp);
		    }
		    collectStatement(sp);
		    iFunSyText = 0;		/* no more function symbols */
		}
#if YYDEBUG
		Symbol t = *(sp);
		if ((iC_debug & 0402) == 0402) pd("decl", sp, $1.v, &t);
#endif
	    }
	| declHead dIVariable	{		/* LVAR AVAR LOUT AOUT CVAR TVAR Constant */
		int		typ1;		/* not LVARC AVARC */
		int		typ2;
		int		ftyp;
		Symbol *	sp;
		$$.f = $1.f; $$.l = $2.l;
		ftyp = $1.v.ftype;		/* ARITH GATE CLCKL TIMRL */
		typ1  = $1.v.type;		/* UDF for all TYPEs except INPX INPW */
		sp = $$.v = $2.v;
		if (sp) {
		    assert(typ1 != LOGC && typ1 != ARNC);
		    if ((typ2 = sp->type) != ERR && sp->ftype != ftyp) {
			ierror("declaration does not match previous declaration:", sp->name);
			if (! iFunSymExt) $2.v->type = ERR;	/* cannot execute properly */
		    } else
		    if ((sp->em & EM) || typ2 == UDF) {
			if ((sp->em & EO) == 0) {
			    sp->ftype = ftyp;	/* adjust ftype from extern type unless QXx.y QBz */
			}
			assert(typ1 == UDF);	/* UDF for all TYPEs */
			if (typ2 != INPW && typ2 != INPX) {
			    sp->type = typ1;	/* fix type */
			}
			sp->em &= ~EM;		/* no longer extern - leave EX */
		    }
#if YYDEBUG
		    Symbol t = *(sp);
		    if ((iC_debug & 0402) == 0402) pd("decl", sp, $1.v, &t);
#endif
		} else {
		    ierror("direct declaration of a number is illegal:", $$.f);
		}
		iFunSyText = 0;			/* no more function symbols */
	    }
	| declHead valueCVariable	{	/* ARNC LOGC */
		Symbol *	sp;		/* not LVAR AVAR LOUT AOUT CVAR TVAR Constant */
		$$.f = $1.f; $$.l = $2.l;
		sp = $$.v = $2.v;
		sp->ftype = $1.v.ftype;		/* ARITH GATE */
		ierror("declaration does not match previous extern immC declaration:", sp->name);
	    }
	;

	/************************************************************
	 *
	 * Immediate immC type definition - may be initialised with a
	 * constant expression.
	 *
	 *	immC bit   b1;		immC int   a1;
	 *	immC bit   b2 = 1;	immC int   a2 = 15 * 3;
	 *
	 * The use of 'immC' in a simple type declaration defines the Gate
	 * object, like in C. An immC object may optionally be initialised
	 * with a constant expression in the declaration (default value 0)
	 * just like in C.
	 *
	 * Apart from that an immC Gate object may only be assigned in C code
	 * but may be used as an immediate rvalue in iC as well as in C code.
	 * By defining an object, no C assignment in the current source is
	 * necessary. An assignment can occurr in another source, in which
	 * the same variable is declared with an 'extern immC' declaration.
	 * An assignment in some source should take place (initialisation
	 * will do) to avoid an algorithmic error. The load module detects
	 * and counts assignments during initialization and warns if no
	 * assignments or initialisation have occurred.
	 *
	 * IEC-1131 input and output variables are pre-declared for iC code
	 * and C code normally do not need to be declared except for the
	 * following cases:
	 *
	 * An 'immC' type declaration of an output variable is needed if
	 * that variable has already been declared with an 'extern immC'
	 * declaration in this source module (usually in an included .ih
	 * header) and its storage is going to be defined in this source.
	 * This also means that the optional initialisation should be done
	 * in this source.
	 *
	 * Apart from initialisation with a constant expression, 'immC' type
	 * declarations may not be combined with a dasgn, since no immediate
	 * assignment may occurr if a variable has been declared and defined
	 * as an ARNC or LOGC type with 'immC'. Such an attempted assignment
	 * is flagged as a hard error.
	 *
	 * 'immC' type declarations may not be combined with an immediate
	 * function block definition - a function block can never be called
	 * in C code. A function block may return a type 'immC' internal
	 * variable as an 'imm' value, which is actually an 'immC' alias,
	 * although it is poor form to rely on this, because code in
	 * function blocks may change.
	 *
	 * declCHead has type ARNC and LOGC, which are the TYPEs given to
	 * completed 'immC' Gate objects.
	 *
	 ***********************************************************/

declC	: declCHead UNDEF	{
		Symbol *	sp;
		$$.f = $1.f; $$.l = $2.l;
		sp = $$.v = $2.v;
		sp->ftype = $1.v.ftype;		/* bit int */
		sp->type  = $1.v.type;
		assert(($1.v.em & EM) == 0);	/* has been cleared in declCHead */
		sp->em    &= ~EM;		/* clear only bit EM here, leave bit EX */
		assert(sp->type == ARNC || sp->type == LOGC);
		if (iFunSymExt) {
		    char *	cp;
		    Symbol *	sp1;
		    cp = strchr(sp->name, '@'); /* locate original extension */
		    assert(cp && isprint(cp[1])); /* extension must be at least 1 character */
		    if ((sp1 = lookup(++cp)) != 0 && (sp1->em & EM)) {
			warning("declaration of an extern variable in a function - ignored:", cp);
		    }
		    collectStatement(sp);
		    iFunSyText = 0;		/* no more function symbols */
		}
#if YYDEBUG
		Symbol t = *(sp);
		if ((iC_debug & 0402) == 0402) pd("declC: declCHead UNDEF", sp, $1.v, &t);
#endif
	    }
	| declCHead dCVariable	{		/* LVARC AVARC LOUT AOUT */
		int		typ2;		/* not LVAR AVAR CVAR TVAR Constant */
		int		typ1;
		int		ftyp;
		Symbol *	sp;
		$$.f = $1.f; $$.l = $2.l;
		ftyp = $1.v.ftype;		/* ARITH GATE CLCKL TIMRL */
		typ1  = $1.v.type;		/* UDF for all TYPEs except ARNC LOGC INPX INPW */
		sp = $$.v = $2.v;
		assert(sp && (typ1 == LOGC || typ1 == ARNC));
		if ((typ2 = sp->type) == INPW || typ2 == INPX) {
		    ierror("immC declaration of an input variable is invalid:", sp->name);
		    sp->em |= ED;		/* do not define */
		} else
		if (typ2 != ERR && sp->ftype != ftyp) {
		    ierror("declaration does not match previous declaration:", sp->name);
		    sp->em |= ED;		/* do not define */
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		} else
		if ((sp->em & EM) || sp->type == UDF) {
		    if ((sp->em & EO) == 0) {
			sp->ftype = ftyp;	/* adjust ftype from extern type unless QXx.y QBz */
		    }
		    sp->type = typ1;	/* ARNC or LOGC */
		    sp->em &= ~EM;		/* no longer extern - leave EX */
		} else if (sp->type == ARNC || sp->type == LOGC) {
		    warning("immC declaration after definition - ignored:", sp->name);
		}
#if YYDEBUG
		Symbol t = *(sp);
		if ((iC_debug & 0402) == 0402) pd("declC: declCHead dCVariable", sp, $1.v, &t);
#endif
		iFunSyText = 0;			/* no more function symbols */
	    }
	| declCHead dIMVariable	{		/* LVAR AVAR CVAR TVAR Constant */
		Symbol *	sp;		/* not LVARC AVARC LOUT AOUT */
		int		typ2;
		$$.f = $1.f; $$.l = $2.l;
		sp = $$.v = $2.v;
		sp->ftype = $1.v.ftype;		/* ARITH GATE */
		if (((typ2 = sp->type) == INPW || typ2 == INPX)) {
		    ierror("immC declaration of an input variable is invalid:", sp->name);
		} else {
		    ierror("declaration does not match previous extern imm declaration:", sp->name);
		}
		sp->em |= ED;			/* do not define */
	    }
	;

immDecl	: decl			{		/* imm declaration */
		$$   = $1;			/* not strictly necessary - stype holds data */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
	| dasgn			{		/* declaration data assignment */
		$$   = $1;			/* not strictly necessary - stype holds data */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
	| dcasgn		{		/* declaration clock assignment */
		$$   = $1;			/* not strictly necessary - stype holds data */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
	| dtasgn		{		/* declaration timer assignment */
		$$   = $1;			/* not strictly necessary - stype holds data */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
	;

immCDecl
	: declC			{		/* immC declaration */
		$$   = $1;			/* not strictly necessary - stype holds data */
		listGenOut($1.v, 1, 0);		/* list immC node without initialisation */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "immCdecl: declC", &$$);
#endif
	    }
	| declC '=' iFunCall	{		/* make immC variable ALIAS of return from const expression */
		List_e *	lp;
		Symbol *	sp;
		$$   = $1;			/* not strictly necessary - stype holds data */
		if ((lp = $3.v) != 0 &&
		    (sp = lp->le_sym) != 0) {
		    assert($1.v->type == ARNC || $1.v->type == LOGC);
		    if (sp->type == ARNC &&		/* TODO what about LOGC */
			(sp->em & EI) != 0 &&
			sp->u_blist == 0) {		/* this ensures assignment will be an ALIAS */
			$1.v->type = UDF;		/* to allow ALIAS assignment */
			if (($$.v = assignExpression(&$1, &$3, 0)) == 0) YYERROR;
		    } else if (sp->type == NCONST) {
			Valp	v;			/* initialisation similar to constExpr */
			v.lfl = 0;
			v.nuv = sp->u_val;		/* numeric value returned by function */
			listGenOut($1.v, -1, &v);	/* list immC node with initialiser value */
		    } else {
			goto NotConstFunReturn;
		    }
		} else {
		  NotConstFunReturn:
		    listGenOut($1.v, 1, 0);		/* list immC node without initialisation */
		    warning("immC initialiser not a constant function return:", $1.v->name);
		}
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "immCdecl: declC = iFunCall", &$$);
#endif
	    }
	| declC '=' constExpr	{
		$$   = $1;			/* not strictly necessary - stype holds data */
		listGenOut($1.v, -1, &($3.v));	/* list immC node with initialiser value */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "immCdecl: declC = constExpr", &$$);
#endif
	    }
	;

declHead
	: IMM TYPE		{
		$$.f = $$.l = $2.l;		/* do not include in expression string */
		assert($1.v->ftype == 0);	/* imm in init.c */
		stype.ftype = $2.v->ftype;	/* ARITH GATE CLCKL TIMRL */
		stype.type  = UDF;
		stype.em &= ~EM;		/* no longer extern - leave EX if it was set */
		stype.fm = 0;
		$$.v = stype;
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "declHead: IMM TYPE", &$$);
#endif
	    }
	| immDecl ','		{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = stype;			/* first TYPE */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "declHead: immDecl ,", &$$);
#endif
	    }
	;

declCHead
	: IMMC TYPE		{
		int		typ;
		int		ftyp;
		$$.f = $$.l = $2.l;		/* do not include in expression string */
		ftyp = $2.v->ftype;		/* ARITH GATE CLCKL TIMRL */
		assert($1.v->ftype == 1);	/* immC in init.c */
		if (ftyp == ARITH) {
		    typ = ARNC;		/* immC int */
		} else
		if (ftyp == GATE) {
		    typ = LOGC;		/* immC bit */
		} else {
		    warning("declaration of an immC type other than bit or int - ignore:", $2.v->name);
		    typ = UDF;
		}
		stype.ftype = ftyp;
		stype.type  = typ;
		stype.em &= ~EM;		/* no longer extern - leave EX if it was set */
		stype.fm = 0;
		$$.v = stype;
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "declHead: IMMC TYPE", &$$);
#endif
	    }
	| immCDecl ','		{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = stype;			/* first TYPE */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "declCHead: immCDecl ,", &$$);
#endif
	    }
	| immCarray ','		{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = stype;			/* first TYPE */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "declCHead: immCarray ,", &$$);
#endif
	    }
	;

	/************************************************************
	 *
	 * Assignment combined with a declaration
	 * an output is recognised by em & EO set (picks Q[XBWL]%d)
	 *
	 * Report an error if the type is an ARNC or LOGC, defined with immC
	 *
	 ***********************************************************/

dasgn	: decl '=' aexpr	{		/* dasgn is NOT an aexpr */
		char *	name;
		int	ioTyp;
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v) {
		    assert($1.v->type != ARNC && $1.v->type != LOGC);
		    if ($1.v->em & EO) {	/* QXx.y QBz etc */
			name = $1.v->name;
			assert(name[0] == 'Q');
			switch (name[1]) {
			case 'X':
			    ioTyp = OUTX;
			    break;
			case 'B':
			case 'W':
			case 'L':
			case 'H':
			    ioTyp = OUTW;
			    break;
			default:
			    assert(0);		/* illegal Q variable */
			    break;
			}
		    } else {
			ioTyp = 0;		/* flags that no I/O is generated */
		    }
		    if (($$.v = assignExpression(&$1, &$3, ioTyp)) == 0) YYERROR;
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) pu(SYM, "dasgn: decl = aexpr", &$$);
#endif
		}
	    }
	;

	/************************************************************
	 *
	 * Different forms of assignment
	 *
	 ***********************************************************/

asgn	: UNDEF '=' aexpr	{		/* asgn is an aexpr */
		Symbol *	sp;
		$$.f = $1.f; $$.l = $3.l;
		sp = $3.v->le_sym;
		assert(sp && (sp->ftype < MIN_ACT && sp->ftype != UDFA));	/* GATEX GATE ARITH */
		$1.v->ftype = sp->ftype;	/* not strict - implicitly declared from aexpr */
		if (($$.v = assignExpression(&$1, &$3, 0)) == 0) YYERROR;
		if (iC_Sflag) {
		    ierror("strict: assignment to an undeclared imm variable:", $1.v->name);
		    $1.v->type = ERR;		/* cannot execute properly */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "asgn: UNDEF", &$$);
#endif
	    }
	| LVAR '=' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = assignExpression(&$1, &$3, 0)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "asgn: LVAR", &$$);
#endif
	    }
	| AVAR '=' aexpr		{
		Symbol *	sp;
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v != NULL &&
		    (sp = $3.v->le_sym) != NULL &&
		    sp->type < MAX_GT &&
		    sp->u_blist == NULL) {
		    assert($1.v->ftype == ARITH);
		    sp->ftype = ARITH;		/* aexpr has never been used - change ftype */
		}
		if (($$.v = assignExpression(&$1, &$3, 0)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "asgn: AVAR", &$$);
#endif
	    }
	| LOUT '=' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = assignExpression(&$1, &$3, OUTX)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "asgn: LOUT", &$$);
#endif
	    }
	| AOUT '=' aexpr		{
		Symbol *	sp;
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v != NULL &&
		    (sp = $3.v->le_sym) != NULL &&
		    sp->type < MAX_GT &&
		    sp->u_blist == NULL) {
		    assert($1.v->ftype == ARITH);
		    sp->ftype = ARITH;		/* aexpr has never been used - change ftype */
		}
		if (($$.v = assignExpression(&$1, &$3, OUTW)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "asgn: AOUT", &$$);
#endif
	    }
	| Constant '=' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		ierror("assignment to a number is illegal:", $$.f);
		if (($$.v = op_asgn(0, &$3, F_ERR)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "asgn: Constant (ERR)", &$$);
#endif
	    }
	;

	/************************************************************
	 *
	 * Assignment as an expression - dasgn is NOT an aexpr
	 *
	 ***********************************************************/

aexpr	: expr			{
		$$ = $1;
		if ($$.v != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "aexpr: expr", &$$);
#endif
	    }
	| asgn			{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = sy_push($1.v);
		assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "aexpr: asgn", &$$);
#endif
	    }
	;

	/************************************************************
	 *
	 * Expressions - includes unclocked built in functions
	 *
	 ***********************************************************/

expr	: UNDEF			{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = checkDecl($1.v);
		$1.v->ftype = GATE;		/* imm bit default if not declared */
		assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: UNDEF", &$$);
#endif
		if (iC_Sflag) {
		    ierror("strict: use of an undeclared imm variable:", $1.v->name);
		    $1.v->type = ERR;		/* cannot execute properly */
		}
	    }
	| Constant		{
		$$.f = $1.f; $$.l = $1.l;	/* no node, value not used */
		$$.v = 0;			/* uses TX0.2 in bit expressions */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: Constant", &$$);
#endif
	    }
	| valueVariable		{		/* LVAR LVARC AVAR AVARC */
		$$.f = $1.f; $$.l = $1.l;
		$$.v = checkDecl($1.v);
		assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: valueVariable", &$$);
#endif
	    }
	| immCarrayVariable	{		/* immC array member variable */
		$$.f = $1.f; $$.l = $1.l;
		$$.v = checkDecl($1.v);
		assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: immCarrayVariable", &$$);
#endif
	    }
	| outVariable		{		/* LOUT AOUT */
		$$.f = $1.f; $$.l = $1.l;
		$$.v = sy_push($1.v);
		assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: outVariable", &$$);
#endif
	    }
	| cCall			{
		$$ = $1;
		if ($$.v) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: cCall", &$$);
#endif
	    }
	| iFunCall			{
		$$ = $1;
		if ($$.v) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: iFunCall", &$$);
#endif
	    }
	| '(' comma_expr ')'		{
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = $2.v) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: (comma_expr)", &$$);
#endif
	    }
	/************************************************************
	 * binary logical and arithmetic operators
	 ***********************************************************/
	| expr '|' expr		{			/* binary | */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		if (($1.v == 0 || $1.v->le_sym->ftype == ARITH) &&
		    ($3.v == 0 || $3.v->le_sym->ftype == ARITH)) {
		    $$.v = op_push($1.v, ARN, $3.v);	/* bitwise | */
		} else {
		    lpL = op_adjust(&$1);
		    lpR = op_adjust(&$3);
		    $$.v = op_push(lpL, OR, lpR);	/* logical | */
		}
		if ($$.v) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr | expr", &$$);
#endif
	    }
	| expr '^' expr		{			/* binary ^ */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		if (($1.v == 0 || $1.v->le_sym->ftype == ARITH) &&
		    ($3.v == 0 || $3.v->le_sym->ftype == ARITH)) {
		    $$.v = op_push($1.v, ARN, $3.v);	/* bitwise ^ */
		} else {
		    lpL = op_adjust(&$1);
		    lpR = op_adjust(&$3);
		    $$.v = op_push(lpL, XOR, lpR);	/* logical ^ */
		}
		if ($$.v) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr ^ expr", &$$);
#endif
	    }
	| expr '&' expr		{			/* binary & */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		if (($1.v == 0 || $1.v->le_sym->ftype == ARITH) &&
		    ($3.v == 0 || $3.v->le_sym->ftype == ARITH)) {
		    $$.v = op_push($1.v, ARN, $3.v);	/* bitwise & */
		} else {
		    lpL = op_adjust(&$1);
		    lpR = op_adjust(&$3);
		    $$.v = op_push(lpL, AND, lpR);	/* logical & */
		}
		if ($$.v) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr & expr", &$$);
#endif
	    }
	| expr CMP expr	{				/* < <= > >= */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		lpR = op_force($3.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    $$.v = op_force($$.v, GATE);	/* default output */
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr CMP expr", &$$);
#endif
	    }
	| expr EQU expr	{				/* == != */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		lpR = op_force($3.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    $$.v = op_force($$.v, GATE);	/* default output */
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr EQU expr", &$$);
#endif
	    }
	| expr '*' expr		{			/* binary * */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		lpR = op_force($3.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr * expr", &$$);
#endif
	    }
	| expr DIV expr		{			/* / % */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		lpR = op_force($3.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr DIV expr", &$$);
#endif
	    }
	| expr '+' expr		{			/* binary + */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		lpR = op_force($3.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr '+' expr", &$$);
#endif
	    }
	| expr '-' expr		{			/* binary - */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		lpR = op_force($3.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr '-' expr", &$$);
#endif
	    }
	| expr LEFT_SH expr		{		/* << */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		lpR = op_force($3.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr << expr", &$$);
#endif
	    }
	| expr RIGHT_SH expr		{		/* >> */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		lpR = op_force($3.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr >> expr", &$$);
#endif
	    }
	/************************************************************
	 * &&  ||
	 *
	 * If both operands of a binary && or || operator are imm bit
	 * more precisely ftype GATE or UDFA (!ARITH) && type >ARN ||
	 * UDF or a reduced ARN variable (of type GATE), then the
	 * operation is compiled as a logical bit, rather than con-
	 * verting both imm bit (GATE) to ARITH, doing an arithmetic
	 * operation and then having default GATE again.
	 ***********************************************************/
	| expr B_AND expr	{			/* binary && */
		Symbol *	sp;
		int		typ;
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v &&
		    (sp = $1.v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type) > ARN || typ == UDF || !sp->u_blist) &&
		    $3.v &&
		    (sp = $3.v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type) > ARN || typ == UDF || !sp->u_blist)) {
		    lpL = op_force($1.v, GATE);
		    lpR = op_force($3.v, GATE);
		    $$.v = op_push(lpL, AND, lpR);	/* logical & */
		    if (iC_Wflag & W_DEPRECATED_LOGIC) {
			if (iC_Sflag) {
			    ierror("strict: Use of '&&' with imm bit variables not allowed (use '&'):", sp->name);
			    sp->type = ERR;		/* cannot execute properly */
			} else {
			    warning("Use of '&&' with imm bit variables is deprecated (use '&'):", sp->name);
			}
		    }
		} else {
		    lpL = op_force($1.v, ARITH);
		    lpR = op_force($3.v, ARITH);
		    $$.v = op_push(lpL, ARN, lpR);	/* arithmetic && */
		    $$.v = op_force($$.v, GATE);	/* default GATE output */
		}
		if ($$.v) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr B_AND expr", &$$);
#endif
	    }
	| expr B_OR expr	{			/* binary || */
		Symbol *	sp;
		int		typ;
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v &&
		    (sp = $1.v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type) > ARN || typ == UDF || !sp->u_blist) &&
		    $3.v &&
		    (sp = $3.v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type) > ARN || typ == UDF || !sp->u_blist)) {
		    lpL = op_force($1.v, GATE);
		    lpR = op_force($3.v, GATE);
		    $$.v = op_push(lpL, OR, lpR);	/* logical | */
		    if (iC_Wflag & W_DEPRECATED_LOGIC) {
			if (iC_Sflag) {
			    ierror("strict: Use of '||' with imm bit variables not allowed (use '|'):", sp->name);
			    sp->type = ERR;		/* cannot execute properly */
			} else {
			    warning("Use of '||' with imm bit variables is deprecated (use '|'):", sp->name);
			}
		    }
		} else {
		    lpL = op_force($1.v, ARITH);
		    lpR = op_force($3.v, ARITH);
		    $$.v = op_push(lpL, ARN, lpR);	/* arithmetic || */
		    $$.v = op_force($$.v, GATE);	/* default GATE output */
		}
		if ($$.v) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr B_OR expr", &$$);
#endif
	    }
	/************************************************************
	 * Conditional expression expr1 ? expr2 : expr3
	 *
	 * All three expressions are forced to ftype ARITH.
	 * Even if expr1 is imm bit it is not possible to use the
	 * logical value directly - it must be forced to a new ARITH
	 * node, which takes part in the resulting arithmetic expression.
	 ***********************************************************/
	| expr '?' expr ':' expr	{		/* ? : */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $5.l;
		lpR = op_force($5.v, ARITH);
		lpL = op_force($3.v, ARITH);
		lpR = op_push(lpL, ARN, lpR);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr ? expr : expr", &$$);
#endif
	    }
	/************************************************************
	 * Conditional expression expr1 ? : expr3
	 * Excerpt from 'info gcc'
	 * ======================================
	 * 5.8 Conditionals with Omitted Operands
	 * ======================================
	 * The middle operand in a conditional expression may be omitted.  Then if
	 * the first operand is nonzero, its value is the value of the conditional
	 * expression.
	 *
	 *    Therefore, the expression
	 *
	 *      x ? : y
	 *
	 * has the value of `x' if that is nonzero; otherwise, the value of `y'.
	 *
	 *    This example is perfectly equivalent to
	 *
	 *      x ? x : y
	 *
	 * In this simple case, the ability to omit the middle operand is not
	 * especially useful.  When it becomes useful is when the first operand
	 * does, or may (if it is a macro argument), contain a side effect.  Then
	 * repeating the operand in the middle would perform the side effect
	 * twice.  Omitting the middle operand uses the value already computed
	 * without the undesirable effects of recomputing it.
	 *
	 * Both expressions are forced to ftype ARITH.
	 ***********************************************************/
	| expr '?' ':' expr	{			/* ? : */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $4.l;
		lpR = op_force($4.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (iC_Pflag) {
		    Symbol * sp = $4.v->le_sym;
		    warning("ISO C forbids omitting the middle term of a ?: expression", sp->name);
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr ? : expr", &$$);
#endif
	    }
	/************************************************************
	 * Logical negation or arithmetic bitwise complement
	 *
	 * For an imm bit operand both '~' and '!' operator result in
	 * logical negation.
	 *
	 * For an imm int operand the '~' operator results in a bitwise
	 * complement, whereas the '!' operator results in logical
	 * negation of the int converted to a truth value. For this
	 * purpose the resulting C expression does the job in both cases.
	 *
	 * The result of '!' on an int could be forced to GATE, but it
	 * would only influence combinations with && and ||, which are
	 * best left as complete arithmetic expressions in such a
	 * doubtful case. All combinations with other bit operands
	 * force it to GATE anyway.
	 ***********************************************************/
	| '~' expr 		{			/* unary ~ */
		Symbol *	sp;
		$$.f = $1.f; $$.l = $2.l;
		if ($2.v) {
		    int typ;
		    if ((sp = $2.v->le_sym)->ftype != ARITH &&
			(((typ = sp->type) != ARNC && typ != ARNF && typ != ARN &&
			typ != SH) || sp->u_blist == 0)) {
							/* logical complement */
			$$.v = op_not(op_force($2.v, GATE));
		    } else {
							/* bitwise complement */
			$$.v = op_push(0, ARN, op_force($2.v, ARITH));
		    }
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		} else {
		    $$.v = 0;				/* constant complement */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: ~expr", &$$);
#endif
	    }
	| '!' expr 		{			/* unary ! */
		Symbol *	sp;
		$$.f = $1.f; $$.l = $2.l;
		if ($2.v) {
		    int typ;
		    if ((sp = $2.v->le_sym)->ftype != ARITH &&
			(((typ = sp->type) != ARNC && typ != ARNF && typ != ARN &&
			typ != SH) || sp->u_blist == 0)) {
							/* logical complement */
			$$.v = op_not(op_force($2.v, GATE));
			if (iC_Wflag & W_DEPRECATED_LOGIC) {
			    if (iC_Sflag) {
				ierror("strict: Use of '!' with an imm bit variable not allowed (use '~'):", sp->name);
				sp->type = ERR;		/* cannot execute properly */
			    } else {
				warning("Use of '!' with an imm bit variable is deprecated (use '~'):", sp->name);
			    }
			}
		    } else {
							/* arithmetic complement */
			$$.v = op_push(0, ARN, op_force($2.v, ARITH));
		    }
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		} else {
		    $$.v = 0;				/* constant complement */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: !expr", &$$);
#endif
	    }
	/************************************************************
	 * Unary + or -
	 ***********************************************************/
	| '+' expr %prec '!'	{			/* unary + */
		$$.f = $1.f; $$.l = $2.l;
		if (($$.v = op_push(0, ARN, op_force($2.v, ARITH))) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: '+' expr", &$$);
#endif
	    }
	| '-' expr %prec '!'	{			/* unary - */
		$$.f = $1.f; $$.l = $2.l;
		if (($$.v = op_push(0, ARN, op_force($2.v, ARITH))) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: '-' expr", &$$);
#endif
	    }
%ifdef	BOOT_COMPILE
	| fexpr			{
		Symbol *	sp;
		if ($1.v == 0) YYERROR;		/* error in bltin() */
		sp = $1.v->le_sym;
		$$ = $1;
		if (sp->ftype != iC_ftypes[sp->type] && sp->ftype != CH_AR && sp->type != VF) {
		    warning("not enough arguments for function", sp->name);
		}
		sp->ftype = sp->type == SH ? ARITH : GATE;
		assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: fexpr", &$$);
#endif
	    }
	/************************************************************
	 * FORCE(aexpr,aexpr,aexpr)
	 *	FORCE(expr,on,off)
	 * also implements LATCH(set, reset) via a pre-defined function block
	 * also implements DLATCH(set, reset, clk) via a pre-defined function block
	 ***********************************************************/
	| BFORCE '(' aexpr ',' lexpr ')'	{	/* FORCE(expr,hi,lo) */
		$$.f = $1.f; $$.l = $6.l;
		if ($3.v == 0) { $$.v = 0; errBit(); YYERROR; }
		$$.v = op_push(op_force($3.v, GATE), LOGC, $5.v);
		$$.v->le_sym->type = LATCH;
		assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: BFORCE", &$$);
#endif
	    }
%endif	/* BOOT_COMPILE */
	;

	/************************************************************
	 * Comma expression - lowest precedence
	 * Very limited use in immediate C - only in parentheses
	 * All except the last element, whose value is returned,
	 * must be an assignment or void function call - otherwise
	 * there would be a free expression whose value goes nowhere.
	 ***********************************************************/

comma_expr
	: aexpr			{			/* comma expression */
		$$ = $1;
		if ($$.v != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "comma_expr: aexpr", &$$);
#endif
	    }
	| asgnStatement ',' comma_expr {		/* asgn casgn tasgn and vFunCall before , */
		$$ = $3;				/* only the last expression is returned */
		if ($$.v != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		memset($1.f, '#', $2.l - $1.f);		/* mark unused asgn , */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "comma_expr: asgnStatement , comma_expr", &$$);
#endif
	    }
	;

cexpr_comma_expr
	: cexpr			{			/* cexpr comma expression */
		$$ = $1;
		if ($$.v != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cexpr_comma_expr: cexpr", &$$);
#endif
	    }
	| asgnStatement ',' cexpr_comma_expr {		/* asgn casgn tasgn and vFunCall before , */
		$$ = $3;				/* only the last expression is returned */
		if ($$.v != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		memset($1.f, '#', $2.l - $1.f);		/* mark unused asgn , */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cexpr_comma_expr: asgnStatement , cexpr_comma_expr", &$$);
#endif
	    }
	;

texpr_comma_expr
	: texpr			{			/* texpr comma expression */
		$$ = $1;
		if ($$.v != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "texpr_comma_expr: texpr", &$$);
#endif
	    }
	| asgnStatement ',' texpr_comma_expr {		/* asgn casgn tasgn and vFunCall before , */
		$$ = $3;				/* only the last expression is returned */
		if ($$.v != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		memset($1.f, '#', $2.l - $1.f);		/* mark unused asgn , */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "texpr_comma_expr: asgnStatement , texpr_comma_expr", &$$);
#endif
	    }
	;

	/************************************************************
	 *
	 *  Constant, constant expression and mixed expression
	 *
	 ***********************************************************/

Constant
	: NUMBER		{
		$$ = $1;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(VAL, "Constant: NUMBER", &$$);
#endif
	    }
	| SIZEOF IMMCARRAY {				/* sizeof array */
		$$.f = $1.f; $$.l = $2.l;
		List_e *	lp;
		int		i, len1, len2;
		char		buf[TSIZE];
		assert($2.v && $2.v->ftype == UDFA);
		i = (lp = $2.v->list) ?
		    (lp->le_val & VAL_MASK) : 0;	/* 0 is returned if no size is specified */
		snprintf(buf, TSIZE, "%d", i);
		if ((len1 = strlen(buf)) > (len2 = $$.l - $$.f)) {
		    ierror("immC array is far too big:", buf);
		} else {
		    strncpy($$.f, buf, len2);		/* overwrite the words 'sizeof x' by the number text */
		    if ((len2 -= len1) > 0) {
			memset($$.f + len1, '#', len2);	/* active blank ##### */
		    }
		}
		$$.v = i;				/* value in case it is needed */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(VAL, "Constant: SIZEOF IMMCARRAY", &$$);
#endif
	    }
	;

constExpr
	: aexpr		{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = evalConstExpr(&$1);		/* returns a Valp */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(VAP, "constExpr: aexpr", &$$);
#endif
	    }
	;
%ifdef	BOOT_COMPILE

	/************************************************************
	 *
	 * lexpr - auxiliary grammar rule to support BFORCE
	 *
	 ***********************************************************/

lexpr	: aexpr ',' aexpr		{
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v == 0) { $$.v = $3.v; errBit(); YYERROR; }
		if ($3.v == 0) { $$.v = $1.v; errBit(); YYERROR; }
		lpR = op_not(op_force($3.v, GATE));
		lpL = op_force($1.v, GATE);
		if (($$.v = op_push(lpL, LOGC, lpR)) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "lexpr: aexpr , aexpr", &$$);
#endif
	    }
	;
%endif	/* BOOT_COMPILE */

	/************************************************************
	 *
	 * Literal block %{ ......... %}
	 *
	 * A literal block may be used anywhere before, between or after
	 * statements in an iC program. The contents of the literal block
	 * will be copied to the generated C file before any expressions
	 * generated by immediate statements.
	 *
	 * Use standard C pre-processor command in literal blocks:
	 *
	 * #include <stdio.h>
	 * #define  SIZE 4
	 * #ifdef etc
	 *
	 * Since immcc version 3, C pre-processor commands are written
	 * without a leading % in literal blocks, although a % before
	 * the #, which was mandatory for earlier versions is still
	 * stripped for backwards compatibility.
	 *
	 ***********************************************************/

lBlock	: LHEAD			{ ccfrag = '%'; ccFP = T1FP; }	/* %{ literal block %} */
		CCFRAG '}'	{ $$.v = 0;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) { fprintf(iC_outFP, ">>>lBlock end\n"); fflush(iC_outFP); }
#endif
		functionUse[0].c_cnt |= F_CALLED;
	    }
	;

	/************************************************************
	 *
	 * C block { ......... }
	 *
	 * A C block is used in if, if else and switch immediate expressions
	 *
	 ***********************************************************/

cBlock	: '{'			{ ccfrag = '{'; ccFP = T4FP; }	/* ccfrag must be set */
	      CCFRAG '}'	{
		$$ = $3;				/* function # is yacc token */
		cBlockFlag = 1;				/* hold up get() till another token found */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) { fprintf(iC_outFP, ">>>cBlock #%d end\n", $3.v >> FUN_OFFSET); fflush(iC_outFP); }
#endif
	    }
	;

	/************************************************************
	 *
	 * Clock and Timer parameters
	 *
	 * default clock is iClock if last clock parameter in list
	 *
	 * timer parameters must specify a delay, which is either
	 * Constant, an Alias for a Constant or an immediate expression
	 * which is forced to ftype ARITH
	 *
	 * default timer delay is 1 if last clock parameter in list
	 *
	 ***********************************************************/

	/************************************************************
	 *  [,(cexpr|texpr[,aexpr])]
	 ***********************************************************/
cref	: /* nothing */		{ $$.v = sy_push(iclock); }
	| ','			{ $$.v = sy_push(iclock); }
	| ',' ctref		{ $$ = $2; }		/* clock or timer */
	;

	/************************************************************
	 *  (cexpr|texpr[,aexpr])
	 ***********************************************************/
ctref	: ctdref		{ $$ = $1; }		/* clock or timer with delay */
	| ctdref ','		{ $$ = $1; }
	| texpr			{			/* timer */
		$$.f = $1.f; $$.l = $1.l;
		$$.v = delayOne($1.v);			/* with implicit delay of 1 */
	    }
	| texpr ','		{
		$$.f = $1.f; $$.l = $2.l;
		$$.v = delayOne($1.v);			/* with implicit delay of 1 */
	    }
	;

	/************************************************************
	 *  (cexpr|texpr,aexpr)
	 ***********************************************************/
ctdref	: cexpr			{ $$ = $1; }		/* clock */
	| texpr ',' aexpr	{			/* timer with delay expression */
		$$ = $1; $$.l = $3.l;
		if (checkConstExpr($3.v) == NULL) {
		    if (const_push(&$3)) { errInt(); YYERROR; }
		}
		$3.v = op_force($3.v, ARITH);
		$3.v->le_val = (unsigned)-1;		/* mark link as timer value */
		$$.v = op_push($1.v, TIM, $3.v);
	    }
	;
%ifdef	BOOT_COMPILE

	/************************************************************
	 *
	 * Built in iC functions
	 *
	 * These built in functions follow C syntax
	 *
	 * the actual function names are defined in file init.c
	 *
	 ***********************************************************/

	/************************************************************
	 * D(aexpr[,(cexpr|texpr[,aexpr])])
	 * SH(aexpr[,(cexpr|texpr[,aexpr])])
	 * RISE(aexpr[,(cexpr|texpr[,aexpr])])
	 *	D(..) SH(..) RISE(expr)
	 *	D(..) SH(..) RISE(expr,clk)
	 *	D(..) SH(..) RISE(expr,tim)
	 *	D(..) SH(..) RISE(expr,tim,delay)
	 ***********************************************************/
fexpr	: BLTIN1 '(' aexpr cref ')' {			/* D(expr); SH(expr); RISE(expr) */
		$$.f = $1.f; $$.l = $5.l;
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTIN1", &$$);
#endif
	    }
	/************************************************************
	 * CHANGE(aexpr[,(cexpr|texpr[,aexpr])])
	 *	CHANGE(expr)
	 *	CHANGE(expr,clk)
	 *	CHANGE(expr,tim)
	 *	CHANGE(expr,tim,delay)
	 ***********************************************************/
	| BLTINC '(' aexpr cref ')' {			/* CHANGE(expr) */
		Symbol *	sp;
		sp = $3.v->le_sym;			/* aexpr symbol */
		assert(sp && $1.v->ftype == CH_BIT);	/* process logical CHANGE via iC_chMbit() */
		if (sp->ftype == ARITH) {
		    $1.v = &iC_CHANGE_ar;		/* process arithmetic CHANGE via iC_chMar() */
		}
		$$.f = $1.f; $$.l = $5.l;
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTINC", &$$);
#endif
	    }
	/************************************************************
	 * DS(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	DS(expr,set)
	 *	DS(expr,set,clkExprSet
	 *	DS(expr,set,timExprSet
	 *	DS(expr,set,timExprSetdelayExprSet
	 ***********************************************************/
	| BLTINS '(' aexpr ',' aexpr cref ')' {		/* DS(expr,set) */
		$$.f = $1.f; $$.l = $7.l;
		$$.v = bltin(&$1, &$3, 0, &$5, &$6, 0, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTINS", &$$);
#endif
	    }
	/************************************************************
	 * DS(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	DS(expr,clkExpr,          set)
	 *	DS(expr,clkExpr,          set,clkSet
	 *	DS(expr,clkExpr,          set,timSet
	 *	DS(expr,clkExpr,          set,timSetdelaySet
	 *	DS(expr,timExpr,delayExpr,set)
	 *	DS(expr,timExpr,delayExpr,set,clkSet
	 *	DS(expr,timExpr,delayExpr,set,timSet
	 *	DS(expr,timExpr,delayExpr,set,timSetdelaySet
	 ***********************************************************/
	| BLTINS '(' aexpr ',' ctdref ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $9.l;
		$$.v = bltin(&$1, &$3, &$5, &$7, &$8, 0, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTINS", &$$);
#endif
	    }
	/************************************************************
	 * SR(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	SR(set,reset)
	 *	SR(set,reset,clkSetReset)
	 *	SR(set,reset,timSetReset)
	 *	SR(set,reset,timSetReset,delaySetReset)
	 * DR(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 * SHR(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	DR(..) SHR(expr,reset)
	 *	DR(..) SHR(expr,reset,clkExprReset)
	 *	DR(..) SHR(expr,reset,timExprReset)
	 *	DR(..) SHR(expr,reset,timExprReset,delayExprReset)
	 * also implements SRX(set, reset) via a pre-defined function block
	 * also implements JK(set, reset) via a pre-defined function block
	 ***********************************************************/
	| BLTIN2 '(' aexpr ',' aexpr cref ')' {		/* SR(set,reset); DR(expr,reset) */
		$$.f = $1.f; $$.l = $7.l;
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, &$6, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTIN2", &$$);
#endif
	    }
	/************************************************************
	 * SR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	SR(set,clkSet,         reset)
	 *	SR(set,clkSet,         reset,clkReset)
	 *	SR(set,clkSet,         reset,timReset)
	 *	SR(set,clkSet,         reset,timReset,delayReset)
	 *	SR(set,timSet,delaySet,reset)
	 *	SR(set,timSet,delaySet,reset,clkReset)
	 *	SR(set,timSet,delaySet,reset,timReset)
	 *	SR(set,timSet,delaySet,reset,timReset,delayReset)
	 * DR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SHR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	DR(..) SHR(expr,clkExpr,          reset)
	 *	DR(..) SHR(expr,clkExpr,          reset,clkReset)
	 *	DR(..) SHR(expr,clkExpr,          reset,timReset)
	 *	DR(..) SHR(expr,clkExpr,          reset,timReset,delayReset)
	 *	DR(..) SHR(expr,timExpr,delayExpr,reset)
	 *	DR(..) SHR(expr,timExpr,delayExpr,reset,clkReset)
	 *	DR(..) SHR(expr,timExpr,delayExpr,reset,timReset)
	 *	DR(..) SHR(expr,timExpr,delayExpr,reset,timReset,delayReset)
	 ***********************************************************/
	| BLTIN2 '(' aexpr ',' ctdref ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $9.l;
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, &$8, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTIN2", &$$);
#endif
	    }
	/************************************************************
	 * DSR(aexpr[,(cexpr|texpr[,aexpr])])
	 * SRR(aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SRR(..) SHSR(expr)
	 *	DSR(..) SRR(..) SHSR(expr,clk)
	 *	DSR(..) SRR(..) SHSR(expr,tim)
	 *	DSR(..) SRR(..) SHSR(expr,tim,delay)
	 ***********************************************************/
	| BLTIN3 '(' aexpr cref ')' {			/* DSR(expr); SHSR(expr) */
		$$.f = $1.f; $$.l = $5.l;
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTIN3", &$$);
#endif
	    }
	/************************************************************
	 * DSR(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 * SRR(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SRR(..) SHSR(expr,reset)
	 *	DSR(..) SRR(..) SHSR(expr,reset,clkExprReset)
	 *	DSR(..) SRR(..) SHSR(expr,reset,timExprReset)
	 *	DSR(..) SRR(..) SHSR(expr,reset,timExprReset,delayExprReset)
	 ***********************************************************/
	| BLTIN3 '(' aexpr ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $7.l;
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, &$6, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTIN3", &$$);
#endif
	    }
	/************************************************************
	 * DSR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SRR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          reset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          reset,clkReset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          reset,timReset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          reset,timReset,delayReset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,reset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,reset,clkReset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,reset,timReset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,reset,timReset,delayReset)
	 ***********************************************************/
	| BLTIN3 '(' aexpr ',' ctdref ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $9.l;
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, &$8, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTIN3", &$$);
#endif
	    }
	/************************************************************
	 * DSR(aexpr,aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 * SRR(aexpr,aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr,aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SRR(..) SHSR(expr,set,reset)
	 *	DSR(..) SRR(..) SHSR(expr,set,reset,clkExprReset)
	 *	DSR(..) SRR(..) SHSR(expr,set,reset,timExprReset)
	 *	DSR(..) SRR(..) SHSR(expr,set,reset,timExprReset,delayExprReset)
	 ***********************************************************/
	| BLTIN3 '(' aexpr ',' aexpr ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $9.l;
		$$.v = bltin(&$1, &$3, 0, &$5, 0, &$7, &$8, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTIN3", &$$);
#endif
	    }
	/************************************************************
	 * DSR(aexpr,aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SRR(aexpr,aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr,aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SRR(..) SHSR(expr,set,clkSet,         reset)
	 *	DSR(..) SRR(..) SHSR(expr,set,timSet,delaySet,reset)
	 *	DSR(..) SRR(..) SHSR(expr,set,clkSet,         reset,clkReset)
	 *	DSR(..) SRR(..) SHSR(expr,set,timSet,delaySet,reset,clkReset)
	 *	DSR(..) SRR(..) SHSR(expr,set,clkSet,         reset,timReset)
	 *	DSR(..) SRR(..) SHSR(expr,set,timSet,delaySet,reset,timReset)
	 *	DSR(..) SRR(..) SHSR(expr,set,clkSet,         reset,timReset,delayReset)
	 *	DSR(..) SRR(..) SHSR(expr,set,timSet,delaySet,reset,timReset,delayReset)
	 * These cases have been left out deliberately, beause it is
	 * not clear wether the default clock for 'expr' should be taken
	 * from the set clock or the reset clock - causes syntax errors.
	 * Therefore the user is forced to chose a clock for 'expr'.
	 * The following code would do the job - has been tested - JW 020630
	 * The clock for 'expr' is cloned from the reset clock.
	| BLTIN3 '(' aexpr ',' aexpr ',' ctdref ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $11.l;
		$$.v = bltin(&$1, &$3, 0, &$5, &$7, &$9, &$10, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTIN3", &$$);
#endif
	    }
	 ***********************************************************/
	/************************************************************
	 * DSR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SRR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          set,reset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          set,reset,clkSetReset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          set,reset,timSetReset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          set,reset,timSetReset,delaySetReset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,set,reset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,set,reset,clkSetReset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,set,reset,timSetReset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,set,reset,timSetReset,delaySetReset)
	 ***********************************************************/
	| BLTIN3 '(' aexpr ',' ctdref ',' aexpr ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $11.l;
		$$.v = bltin(&$1, &$3, &$5, &$7, 0, &$9, &$10, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTIN3", &$$);
#endif
	    }
	/************************************************************
	 * DSR(aexpr,(cexpr|texpr,aexpr),aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SRR(aexpr,(cexpr|texpr,aexpr),aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr,(cexpr|texpr,aexpr),aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          set,clkSet,         reset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          set,timSet,delaySet,reset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          set,clkSet,         reset,clkReset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          set,timSet,delaySet,reset,clkReset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          set,clkSet,         reset,timReset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          set,timSet,delaySet,reset,timReset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          set,clkSet,         reset,timReset,delayReset)
	 *	DSR(..) SRR(..) SHSR(expr,clkExpr,          set,timSet,delaySet,reset,timReset,delayReset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,set,clkSet,         reset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,set,timSet,delaySet,reset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,set,clkSet,         reset,clkReset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,set,timSet,delaySet,reset,clkReset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,set,clkSet,         reset,timReset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,set,timSet,delaySet,reset,timReset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,set,clkSet,         reset,timReset,delayReset)
	 *	DSR(..) SRR(..) SHSR(expr,timExpr,delayExpr,set,timSet,delaySet,reset,timReset,delayReset)
	 ***********************************************************/
	| BLTIN3 '(' aexpr ',' ctdref ',' aexpr ',' ctdref ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $13.l;
		$$.v = bltin(&$1, &$3, &$5, &$7, &$9, &$11, &$12, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fexpr: BLTIN3", &$$);
#endif
	    }
	;
%endif	/* BOOT_COMPILE */

	/************************************************************
	 *
	 * if, if else and switch immediate expressions
	 *
	 * no assignment allowed for ffexpr - they stand alone
	 *
	 ***********************************************************/

ifini	: IF '(' aexpr cref ')'		{		/* if (expr) { x++; } */
		if (iC_openT4T5(1)) ierror("IF: cannot open:", T4FN); /* rewind if necessary */
		writeCexeString(T4FP, ++c_number);	/* and record for copying */
		if (iFunSymExt == 0) {			/* defer freeing till called */
		    functionUse[0].c_cnt |= F_CALLED;	/* flag for copying temp file */
		    functionUse[c_number].c_cnt++;	/* free this 'if' function for copying */
		}
		fprintf(T4FP, "    if (iC_gf->gt_val < 0)\n");
	    }
				cBlock	{		/* { x++; } */
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, &$7);
	    }
	;

	/************************************************************
	 * if (aexpr[,(cexpr|texpr[,aexpr])]) { C code }
	 *	if (expr)	    { C code }
	 *	if (expr,clk)	    { C code }
	 *	if (expr,tim)	    { C code }
	 *	if (expr,tim,delay) { C code }
	 ***********************************************************/
ffexpr	: ifini				{		/* if (expr) { x++; } */
		$$.v = $1.v;
		ffexprCompile("IF", $$.v, c_number);	/* c_compile cBlock */
	    }
	/************************************************************
	 * if (aexpr[,(cexpr|texpr[,aexpr])]) { C code } else { C code }
	 *	if (expr)	    { C code } else { C code }
	 *	if (expr,clk)	    { C code } else { C code }
	 *	if (expr,tim)	    { C code } else { C code }
	 *	if (expr,tim,delay) { C code } else { C code }
	 ***********************************************************/
	| ifini ELSE			{		/* { x++; } else */
		Symbol *	sp;
		List_e *	lp;
		sp = $1.v->le_sym;			/* slave, deleted later */
		assert(sp);
		lp = sp->u_blist;
		assert(lp);
		sp = lp->le_sym;			/* master - currently ftype F_CF */
		assert(sp);
		sp->ftype = $2.v->ftype;		/* make it ftype F_CE from ELSE */
		fprintf(T4FP, "    else\n");
	    }
		     cBlock		{		/* { x--; } */
		$$.v = $1.v;
		ffexprCompile("IF ELSE", $$.v, c_number); /* c_compile cBlock's */
	    }
	/************************************************************
	 * switch (aexpr[,(cexpr|texpr[,aexpr])]) { C switch code }
	 *	switch (expr)		{ C switch code }
	 *	switch (expr,clk)	{ C switch code }
	 *	switch (expr,tim)	{ C switch code }
	 *	switch (expr,tim,delay) { C switch code }
	 ***********************************************************/
	| SWITCH '(' aexpr cref ')'		{	/* switch (expr) { case ...; } */
		if (iC_openT4T5(1)) ierror("SWITCH: cannot open:", T4FN); /* rewind if necessary */
		writeCexeString(T4FP, ++c_number);	/* and record for copying */
		if (iFunSymExt == 0) {			/* defer freeing till called */
		    functionUse[0].c_cnt |= F_CALLED;	/* flag for copying temp file */
		    functionUse[c_number].c_cnt++;	/* free this 'switch' function for copying */
		}
		fprintf(T4FP, "    switch (iC_gf->gt_new)\n");
	    }
				    cBlock	{	/* { x++; } */
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, &$7);
		ffexprCompile("SWITCH", $$.v, c_number); /* c_compile cBlock */
	    }
	;

	/************************************************************
	 *
	 * Clock built in expression
	 *
	 ***********************************************************/

dcasgn	: decl '=' cexpr	{			/* dcasgn is NOT an cexpr */
		if ($1.v == 0 || $1.v->ftype != CLCKL) {
		    ierror("assigning clock to variable declared differently:", $1.v->name);
		    if ($1.v) {
			$1.v->type = ERR;		/* cannot execute properly */
			$1.v->ftype = CLCKL;
		    }
		} else if ($1.v->type != UDF && $1.v->type != CLK) {
		    ierror("assigning clock to variable assigned differently:", $1.v->name);
		    $1.v->type = ERR;			/* cannot execute properly */
		}
		if ($1.v) $$.v = assignExpression(&$1, &$3, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "dcasgn: decl = cexpr", &$$);
#endif
	    }
	;

casgn	: UNDEF '=' cexpr	{
		$1.v->ftype = CLCKL;
		$$.v = assignExpression(&$1, &$3, 0);	/* not strict */
		if (iC_Sflag) {
		    ierror("strict: assignment to an undeclared 'imm clock':", $1.v->name);
		    $1.v->type = ERR;			/* cannot execute properly */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "casgn: UNDEF = cexpr", &$$);
#endif
	    }
	| CVAR '=' cexpr	{
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    ierror("multiple assignment clock:", $1.v->name);
		    $1.v->type = ERR;			/* cannot execute properly */
		}
		assert($1.v->ftype == CLCKL);
		$$.v = assignExpression(&$1, &$3, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "casgn: CVAR = cexpr", &$$);
#endif
	    }
	;

cexpr	: CVAR			{ $$.v = checkDecl($1.v); }
	| casgn			{ $$.v = sy_push($1.v); }
	| Constant		{
		$$.f = $1.f; $$.l = $1.l;	/* no node, value not used */
		$$.v = 0;			/* mark as illegal in clock expressions */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cexpr: Constant", &$$);
#endif
	    }
%ifdef	BOOT_COMPILE
	| cfexpr		{
		Symbol *	sp = $1.v->le_sym;
		assert(sp);
		if (sp->ftype != iC_ftypes[sp->type]) {
		    warning("not enough arguments for function", sp->name);
		}
		sp->ftype = CLCKL;			/* clock list head */
		$$ = $1;
	    }
%endif	/* BOOT_COMPILE */
	| cFunCall		{
		Symbol *	sp = &nSym;
		if ($1.v == 0 || (sp = $1.v->le_sym) == 0 || sp->ftype != CLCKL) {
		    ierror("called function does not return type clock:", sp->name);
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		}
		$$ = $1;
	    }
	| '(' cexpr_comma_expr ')'		{
		Symbol *	sp = &nSym;
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = $2.v) == 0 || (sp = $$.v->le_sym) == 0 || sp->ftype != CLCKL) {
		    ierror("value in parentheses is not type clock:", sp->name);
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		} else {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cexpr: ( cexpr_comma_expr )", &$$);
#endif
	    }
	;
%ifdef	BOOT_COMPILE

	/************************************************************
	 * CLOCK(aexpr[,(cexpr|texpr[,aexpr])])
	 *	CLOCK(set1)
	 *	CLOCK(set1,clkSet1)
	 *	CLOCK(set1,timSet1)
	 *	CLOCK(set1,timSet1,delaySet1)
	 ***********************************************************/
cfexpr	: CBLTIN '(' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, 0);
	    }
	/************************************************************
	 * CLOCK(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	CLOCK(set1,set2)
	 *	CLOCK(set1,set2,clkSet1Set2)
	 *	CLOCK(set1,set2,timSet1Set2)
	 *	CLOCK(set1,set2,timSet1Set2,delaySet1Set2)
	 ***********************************************************/
	| CBLTIN '(' aexpr ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, &$6, 0);
	    }
	/************************************************************
	 * CLOCK(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	CLOCK(set1,clkSet1,          set2)
	 *	CLOCK(set1,clkSet1,          set2,clkSet2)
	 *	CLOCK(set1,clkSet1,          set2,timSet2)
	 *	CLOCK(set1,clkSet1,          set2,timSet2,delaySet2)
	 *	CLOCK(set1,timSet1,delaySet1,set2)
	 *	CLOCK(set1,timSet1,delaySet1,set2,clkSet2)
	 *	CLOCK(set1,timSet1,delaySet1,set2,timSet2)
	 *	CLOCK(set1,timSet1,delaySet1,set2,timSet2,delaySet2)
	 ***********************************************************/
	| CBLTIN '(' aexpr ',' ctdref ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, &$8, 0);
	    }
	;
%endif	/* BOOT_COMPILE */

	/************************************************************
	 *
	 * Timer built in expression
	 *
	 ***********************************************************/

dtasgn	: decl '=' texpr	{			/* dtasgn is NOT an texpr */
		if ($1.v == 0 || $1.v->ftype != TIMRL) {
		    ierror("assigning timer to variable declared differently:", $1.v->name);
		    if ($1.v) {
			$1.v->type = ERR;		/* cannot execute properly */
			$1.v->ftype = TIMRL;
		    }
		} else if ($1.v->type != UDF && $1.v->type != TIM) {
		    ierror("assigning timer to variable assigned differently:", $1.v->name);
		    $1.v->type = ERR;			/* cannot execute properly */
		}
		if ($1.v) $$.v = assignExpression(&$1, &$3, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "dtasgn: decl = texpr", &$$);
#endif
	    }
	;

tasgn	: UNDEF '=' texpr	{
		$1.v->ftype = TIMRL;
		$$.v = assignExpression(&$1, &$3, 0);	/* not strict */
		if (iC_Sflag) {
		    ierror("strict: assignment to an undeclared 'imm timer':", $1.v->name);
		    $1.v->type = ERR;			/* cannot execute properly */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "tasgn: UNDEF = texpr", &$$);
#endif
	    }
	| TVAR '=' texpr	{
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    ierror("multiple assignment timer:", $1.v->name);
		    $1.v->type = ERR;			/* cannot execute properly */
		}
		assert($1.v->ftype == TIMRL);
		$$.v = assignExpression(&$1, &$3, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "tasgn: TVAR = texpr", &$$);
#endif
	    }
	;

texpr	: TVAR			{ $$.v = checkDecl($1.v); }
	| tasgn			{ $$.v = sy_push($1.v); }
	| Constant		{
		$$.f = $1.f; $$.l = $1.l;	/* no node, value not used */
		$$.v = 0;			/* mark as illegal in timer expressions */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "texpr: Constant", &$$);
#endif
	    }
%ifdef	BOOT_COMPILE
	| tfexpr		{
		Symbol *	sp = $1.v->le_sym;
		assert(sp);
		if (sp->ftype != iC_ftypes[sp->type]) {
		    warning("not enough arguments for function", sp->name);
		}
		sp->ftype = TIMRL;			/* timer list head */
		$$ = $1;
	    }
%endif	/* BOOT_COMPILE */
	| tFunCall		{
		Symbol *	sp = &nSym;
		if ($1.v == 0 || (sp = $1.v->le_sym) == 0 || sp->ftype != TIMRL) {
		    ierror("called function does not return type timer:", sp->name);
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		}
		$$ = $1;
	    }
	| '(' texpr_comma_expr ')'		{
		Symbol *	sp = &nSym;
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = $2.v) == 0 || (sp = $$.v->le_sym) == 0 || sp->ftype != TIMRL) {
		    ierror("value in parentheses is not type timer:", sp->name);
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		} else {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "texpr: ( texpr_comma_expr )", &$$);
#endif
	    }
	;
%ifdef	BOOT_COMPILE

	/************************************************************
	 *
	 * TIMER() has a preset off delay of 0.
	 * Such timers will clock with iClock on the falling edge of
	 * the master gate. They will clock with iClock on the rising
	 * edge if the on delay is 0.
	 *
	 ***********************************************************/

tfexpr	: TBLTIN '(' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, 0);
	    }
	/************************************************************
	 * TIMER(aexpr[,(cexpr|texpr[,aexpr])])
	 *	TIMER(set1)
	 *	TIMER(set1,clkSet1)
	 *	TIMER(set1,timSet1)
	 *	TIMER(set1,timSet1,delaySet1)
	 ***********************************************************/
	| TBLTIN '(' aexpr ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, &$6, 0);
	    }
	/************************************************************
	 * TIMER(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	TIMER(set1,set2)
	 *	TIMER(set1,set2,clkSet1Set2)
	 *	TIMER(set1,set2,timSet1Set2)
	 *	TIMER(set1,set2,timSet1Set2,delaySet1Set2)
	 ***********************************************************/
	| TBLTIN '(' aexpr ',' ctdref ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, &$8, 0);
	    }
	/************************************************************
	 * TIMER(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	TIMER(set1,clkSet1,          set2)
	 *	TIMER(set1,clkSet1,          set2,clkSet2)
	 *	TIMER(set1,clkSet1,          set2,timSet2)
	 *	TIMER(set1,clkSet1,          set2,timSet2,delaySet2)
	 *	TIMER(set1,timSet1,delaySet1,set2)
	 *	TIMER(set1,timSet1,delaySet1,set2,clkSet2)
	 *	TIMER(set1,timSet1,delaySet1,set2,timSet2)
	 *	TIMER(set1,timSet1,delaySet1,set2,timSet2,delaySet2)
	 ***********************************************************/

	/************************************************************
	 *
	 * Alternate TIMER1() has a preset off delay of 1.
	 * Such timers will clock with the next timer pulse on the
	 * falling edge of the master gate. They will clock with the
	 * next timer pulse on the rising edge if the on delay is 0.
	 * The timing is the same as if the timer generated by TIMER1()
	 * and used with a delay of 0 were a clock pulse.
	 *
	 ***********************************************************/

	/************************************************************
	 * TIMER1(aexpr[,(cexpr|texpr[,aexpr])])
	 *	TIMER1(set1)
	 *	TIMER1(set1,clkSet1)
	 *	TIMER1(set1,timSet1)
	 *	TIMER1(set1,timSet1,delaySet1)
	 ***********************************************************/
	| TBLTI1 '(' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, &val1);
	    }
	/************************************************************
	 * TIMER1(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	TIMER1(set1,set2)
	 *	TIMER1(set1,set2,clkSet1Set2)
	 *	TIMER1(set1,set2,timSet1Set2)
	 *	TIMER1(set1,set2,timSet1Set2,delaySet1Set2)
	 ***********************************************************/
	| TBLTI1 '(' aexpr ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, &$6, &val1);
	    }
	/************************************************************
	 * TIMER1(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	TIMER1(set1,clkSet1,          set2)
	 *	TIMER1(set1,clkSet1,          set2,clkSet2)
	 *	TIMER1(set1,clkSet1,          set2,timSet2)
	 *	TIMER1(set1,clkSet1,          set2,timSet2,delaySet2)
	 *	TIMER1(set1,timSet1,delaySet1,set2)
	 *	TIMER1(set1,timSet1,delaySet1,set2,clkSet2)
	 *	TIMER1(set1,timSet1,delaySet1,set2,timSet2)
	 *	TIMER1(set1,timSet1,delaySet1,set2,timSet2,delaySet2)
	 ***********************************************************/
	| TBLTI1 '(' aexpr ',' ctdref ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, &$8, &val1);
	    }
	;
%endif	/* BOOT_COMPILE */

	/************************************************************
	 *
	 * Embedded C variable, function or macro calls.
	 *
	 * These variables, functions or macros may be declared implicitly
	 * via a #include or #define in a preceding literal block or declared
	 * and or defined in a literal block.
	 *
	 * Functions should have at least 1 parameter which is immediate
	 * to trigger execution of the function. Any changes in
	 * non-immediate parameters will not trigger execution and
	 * should be handled with care. This applies particularly to
	 * C int variables whose latest assigned value will be used.
	 * They may be useful for command line parameters, which never
	 * change during the remaining execution of the program although
	 * even then the immediate expressions they are in are not
	 * triggered at initialisation. Therefore it is recommended that
	 * all variables used in C and iC code be declared as 'immC int',
	 * which will trigger execution.
	 *
	 * All imm bit expressions in parameters will be converted to imm int.
	 *
	 * The return value of all functions must be int. If a function
	 * with a different return value is to be used, define a wrapper
	 * function in a literal block.
	 *
	 * If a C variable, function or macro is going to be used in an iC
	 * immediate expression it must be declared with an extern C function
	 * declaration in the iC code - at least unless 'no strict' is active.
	 * (it isn't - is it) Such extern declarations make the debugging
	 * of iC code much simpler. C variables must be declared extern,
	 * even if not 'strict' because otherwise the compiler would
	 * identify them as undeclared imm bit variables.
	 *
	 * C functions in blocks of C code controlled by iC if else or
	 * switch are not limited in this way. They must fit in with
	 * declarations made in C literal blocks and produce consistent
	 * C code for the underlying C compiler.
	 *
	 ***********************************************************/

cCall	: CNAME			{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = cCallCount($1.v, NULL, -1);
		if (iC_Wflag & W_DEPRECATED_LOGIC) {
		    warning("C variable will not fire this expression:", $1.v->name);
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cCall: UNDEF", &$$);
#endif
	    }
	| UNDEF '(' cParams ')'	{
		$$.f = $1.f; $$.l = $4.l;
		$$.v = cCallCount($1.v, $3.v, $3.v ? $3.v->le_val : 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cCall: UNDEF ( cParams )", &$$);
#endif
	    }
	| CNAME '(' cParams ')'	{
		$$.f = $1.f; $$.l = $4.l;
		$$.v = cCallCount($1.v, $3.v, $3.v ? $3.v->le_val : 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cCall: CNAME ( cParams )", &$$);
#endif
	    }
	| UNDEF '(' cParams error ')'	{
		$$.f = $1.f; $$.l = $5.l;
		$$.v = cCallCount($1.v, $3.v, $3.v ? $3.v->le_val : 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cCall: UNDEF ( cParams error )", &$$);
#endif
		yyerrok;
	    }
	| CNAME '(' cParams error ')'	{
		$$.f = $1.f; $$.l = $5.l;
		$$.v = cCallCount($1.v, $3.v, $3.v ? $3.v->le_val : 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cCall: CNAME ( cParams error)", &$$);
#endif
		yyerrok;
	    }
	;

cParams	: /* nothing */		{ $$.v =  0; }
	| cPlist		{ $$   = $1; }	/* do not allow extra comma for C parameters */
	;

cPlist	: aexpr			{
		$$.f = $1.f; $$.l = $1.l;
		if (($$.v = cListCount(0, $1.v)) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cPlist: aexpr", &$$);
#endif
	    }
	| cPlist ',' aexpr	{
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = cListCount($1.v, $3.v)) != 0) {
		    assert($$.f == 0 || ($$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]));
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cPlist: cPlist , aexpr", &$$);
#endif
	    }
	;

	/************************************************************
	 *
	 *	Immediate functions
	 *
	 * A new immediate function definition name is an UNDEF.
	 * Alternatively an existing immediate function is an IFUNCTION
	 * etc, whose data objects are deleted to allow redefinition.
	 ***********************************************************/

iFunTrigger
	: UNDEF				{ $$ = $1; }
	| IFUNCTION			{ $$ = $1; $$.v = clearFunDef($1.v); }
	| CFUNCTION			{ $$ = $1; $$.v = clearFunDef($1.v); }
	| TFUNCTION			{ $$ = $1; $$.v = clearFunDef($1.v); }
	| VFUNCTION			{ $$ = $1; $$.v = clearFunDef($1.v); }
	;

	/************************************************************
	 * Immediate function with immediate return value
	 * Definition:
	 *	imm bit   bf1(bit p1, int p2, clock c1, timer t1, int delay) {...}
	 *	imm int   if1(assign bit a1, assign int a2, bit p1, int p2)  {...}
	 *	imm clock cf1(bit p1, clock c1) {...}
	 *	imm timer tf1(bit p1, clock c1) {...}
	 ***********************************************************/

iFunHead
	: declHead iFunTrigger '('	{
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v.type == ARNC || $1.v.type == LOGC) {
		    ierror("Definition of an immC function is illegal:", $$.f);
		}
		$$.v = functionDefHead($1.v.ftype, $2.v);	/* function head Symbol */
	    }

	/************************************************************
	 * Void immediate function
	 * Definition:
	 *	imm void  vf1(assign bit a1, assign int a2, bit p1, int p2) {...}
	 ***********************************************************/

	| immT VOID iFunTrigger '('	{
		$$.f = $1.f; $$.l = $4.l;
		if ($1.v->ftype == 1) {		/* immC in init.c */
		    ierror("Definition of an immC void function is illegal:", $$.f);
		}
		$$.v = functionDefHead($2.v->ftype, $3.v); /* function head Symbol */
	    }
	;

	/************************************************************
	 * Finalise function definition
	 ***********************************************************/

iFunDef	: iFunHead fParams ')' '{' funcBody '}'	{
		$$.f = $1.f; $$.l = $6.l;
		$$.v = functionDefinition($1.v, $2.v);
	    }
	| iFunHead fParams error ')' '{' funcBody '}'	{
		$$.f = $1.f; $$.l = $7.l;
		$$.v = functionDefinition($1.v, $2.v);
		yyerrok;
	    }
	;

	/************************************************************
	 * collect statements in the function body
	 * is now done for individual statements and assignments
	 ***********************************************************/

funcBody
	: /* nothing */			{
		$$.v = 0; $$.f = $$.l = 0;
		clrBuf();
	    }
	| funcBody funcStatement	{
		$$.v = 0;			/* $$.v is not required */
		$$.f = $$.l = 0;
		clrBuf();
	    }
	| funcBody error ';'		{
		$$.v = 0; $$.f = $$.l = 0;
		clrBuf();
		yyerrok;
	    }
	;

	/************************************************************
	 * Value function return statement
	 ***********************************************************/

return_statement
	: RETURN actexpr		{
		$$.f = $2.f; $$.l = $2.l;		/* TD $$.f should be func$ */
		$$.v = returnStatement(&$2);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "return_statement: RETURN actexpr", &$$);
#endif
	    }
	;


actexpr
	: aexpr				{ $$ = $1; }	/* use in return and real parameter */
	| cexpr				{ $$ = $1; }
	| texpr				{ $$ = $1; }
	;

ractexpr						/* use as real parameter */
	: actexpr			{ $$ = $1; }
	| IMMCARRAY			{
		Symbol *	sp;

		$$.f = $1.f; $$.l = $1.l;
		assert($1.f == 0 || ($1.f >= iCbuf && $1.l < &iCbuf[IMMBUFSIZE]));
		$$.v = checkDecl(sp = $1.v);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "ractexpr: immCarray", &$$);
#endif
	    }
	;

	/************************************************************
	 * formal parameter list for immediate function definitions
	 ***********************************************************/

fParams	: /* nothing */			{ $$.v =  0; }
	| fPlist			{ $$   = $1; }
	| fPlist ','			{ $$   = $1; }
	;

fPlist	: formalParameter		{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = sy_push($1.v);			/* first link in parameter list */
		assert($1.f == 0 || ($1.f >= iCbuf && $1.l < &iCbuf[IMMBUFSIZE]));
		$$.v->le_first = $1.f;
		$$.v->le_last = $1.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fPlist: formalParameter", &$$);
#endif
	    }
	| fPlist ',' formalParameter	{
		List_e *	lp;

		$$.f = $1.f; $$.l = $3.l;
		lp = $$.v;
		while (lp->le_next) {
		    lp = lp->le_next;
		}
		lp = lp->le_next = sy_push($3.v); /* link to end of parameter list */
		assert($3.f == 0 || ($3.f >= iCbuf && $3.l < &iCbuf[IMMBUFSIZE]));
		lp->le_first = $3.f;
		lp->le_last = $3.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fPlist: fPlist formalParameter", &$$);
#endif
	    }
	;

	/************************************************************
	 * individual formal parameter
	 * since formal parameters are declared before anything else
	 * in the function block definition and since they are in their
	 * own namespace, with the function name prefix @ they cannot
	 * be anything but UNDEF, except NUMBER or IO variables, which
	 * will cause a hard error.
	 ***********************************************************/

formalParameter
	: ASSIGN formalParaTypeDecl UNDEF	{
		$$ = $3;				/* formal assign parameter Symbol */
		$$.v->ftype = $2.v.ftype;		/* TYPE bit int clock timer */
		if ($2.v.type != UDF) {
		    warning("immC type is not allowed for formal assign parameter - ignore:", $$.v->name);
		}
		$$.v->type  = UDF;			/* assign parameter is set to UDF */
		iFunSyText = 0;				/* no more function symbols */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "formalParameter: ASSIGN formalParaTypeDecl UNDEF", &$$);
#endif
	    }
	| formalParaTypeDecl UNDEF	{
		int	ft;

		$$ = $2;				/* formal value parameter Symbol */
		$$.v->ftype = ft = $1.v.ftype;		/* TYPE bit int clock timer */
		if ($1.v.type != UDF) {
		    warning("immC type is not allowed for formal value parameter - ignore:", $$.v->name);
		}
		if (ft >= CLCKL) {			/* check that CLCKL TIMRL */
		    ft -= CLCKL - CLCK;			/* and CLCK and TIMR are adjacent */
		}
		$$.v->type = iC_types[ft];		/* value parameter has type set */
		iFunSyText = 0;				/* no more function symbols */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "formalParameter: formalParaTypeDecl UNDEF", &$$);
#endif
	    }
	| formalParaTypeDecl UNDEF '[' immCindex ']'	{
		int		typ;
		int		ftyp;
		Symbol *	sp;
		Symbol *	tsp;
		List_e *	lp;
		int		i;
		char		tempBuf[TSIZE];

		$$.f = $1.f; $$.l = $5.l;
		sp = $$.v = $2.v;
		ftyp = $1.v.ftype;			/* TYPE bit int */
		if ((typ = $1.v.type) == UDF) {
		    if (ftyp == ARITH) {
			typ = ARNC;			/* immC int */
		    } else
		    if (ftyp == GATE) {
			typ = LOGC;			/* immC bit */
		    } else {
			typ = UDF;
		    }
		}
		sp->type = typ;				/* only ARNC or LOGC (UDF on error) */
		sp->ftype = UDFA;			/* immC array */
		if (sp->type == ARNC || sp->type == LOGC) {
		    if ($4.v.lfl == 0) {
			if ((sp->v_cnt = (unsigned int)($4.v.nuv)) != 0) {	/* array size or 0 */
			    for (i = 0; i < sp->v_cnt; i++) {
				snprintf(tempBuf, TSIZE, "%s%d", sp->name, i);	/* base name + index */
				if ((tsp = lookup(tempBuf)) == 0) {
				    tsp = install(tempBuf, sp->type, iC_ftypes[sp->type]);
				}
				if (sp->list == 0) {
				    sp->list = lp = sy_push(tsp);	/* head of new member list */
				    lp->le_val = sp->v_cnt;
				} else {
				    lp = lp->le_next = sy_push(tsp);	/* link to end of new member list */
				}
			    }
			}				/* if array size 0 this is an anonymous array */
			sp->em &= ~EI;			/* mark as constant */
		    } else
		    if ($4.v.lfl == 1) {
			sp->v_elist = $4.v.lpt;		/* pointer to constant parameter */
			sp->em |= EI;			/* mark as pointer */
		    }
		} else {
		    ierror("formal array parameter other than immC bit or immC int:", sp->name);
		    sp->type = ERR;
		}
		iFunSyText = 0;				/* no more function symbols */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "formalParameter: formalParaTypeDecl UNDEF [ immCindex ]", &$$);
#endif
	    }
	| formalConstTypeDecl UNDEF	{
		$$ = $2;				/* formal constant parameter Symbol */
		assert($1.v.ftype == ARITH);
		$$.v->ftype = ARITH;
		$$.v->type = ARN;			/* int */
		$$.v->em |= EI;				/* mark parameter as a const */
		iFunSyText = 0;				/* no more function symbols */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "formalParameter: formalConstTypeDecl UNDEF", &$$);
#endif
	    }
	;

formalParaTypeDecl
	: TYPE				{
		$$.f = $$.l = $1.l;			/* do not include in expression string */
		$$.v.ftype = $1.v->ftype;		/* ARITH GATE CLCKL TIMRL */
		$$.v.type = UDF;
		$$.v.em &= ~EM;
		$$.v.fm = 0;
		iFunSyText = iFunBuffer;		/* expecting a new function symbol */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "formalParaTypeDecl: TYPE", &$$);
#endif
	    }
	| immT TYPE			{
		int		typ;
		int		ftyp;
		$$.f = $$.l = $2.l;			/* do not include in expression string */
		ftyp = $2.v->ftype;			/* ARITH GATE CLCKL TIMRL */
		if ($1.v->ftype == 1) {			/* immC in init.c */
		    if (ftyp == ARITH) {
			typ = ARNC;			/* immC int */
		    } else
		    if (ftyp == GATE) {
			typ = LOGC;			/* immC bit */
		    } else {
			warning("immC formal parameter other than bit or int - ignore:", $2.v->name);
			goto immTypeF;
		    }
		} else {				/* simple imm */
		  immTypeF:
		    typ = UDF;
		}
		$$.v.ftype = ftyp;			/* immT is optional */
		$$.v.type = typ;
		$$.v.em &= ~EM;
		$$.v.fm = 0;
		iFunSyText = iFunBuffer;		/* expecting a new function symbol */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "formalParaTypeDecl: immT TYPE", &$$);
#endif
	    }
	;

	/************************************************************
	 * Allowing CONST without a following TYPE (only int is legitimate)
	 * leads to a grammar inconsistency.
	 *	iFunSyText = iFunBuffer; would not be executed until the
	 * following UNDEF is seen, because a TYPE may follow.
	 * This is too late to precede the UNDEF with iFunSymExt.
	 * This would make the parameter look like an undefined extern.
	 ***********************************************************/

formalConstTypeDecl
	: CONST TYPE			{
		$$.f = $$.l = $2.l;			/* do not include in expression string */
		switch ($2.v->ftype) {
		case ARITH:
		    break;				/* correct type const int */
		case GATE:
		    ierror("const bit is not allowed as a formal parameter:", NS);
		    break;
		case CLCKL:
		    ierror("const clock is not allowed as a formal parameter:", NS);
		    break;
		case TIMRL:
		    ierror("const timer is not allowed as a formal parameter:", NS);
		    break;
		default:
		    assert(0);
		    break;
		}
		$$.v.type = UDF;			/* formal value parameter const int */
		$$.v.ftype = ARITH;
		$$.v.em &= ~EM;
		$$.v.fm = 0;
		iFunSyText = iFunBuffer;		/* expecting a new function symbol */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "formalConstTypeDecl: CONST TYPE", &$$);
#endif
	    }
	| immT CONST TYPE			{
		$$.f = $$.l = $3.l;			/* do not include in expression string */
		if ($1.v->ftype == 1) {			/* immC in init.c */
		    ierror("immC const is not allowed as a formal parameter:", NS);
		} else {
		    switch ($3.v->ftype) {
		    case ARITH:
			break;				/* correct type const int */
		    case GATE:
			ierror("imm const bit is not allowed as a formal parameter:", NS);
			break;
		    case CLCKL:
			ierror("imm const clock is not allowed as a formal parameter:", NS);
			break;
		    case TIMRL:
			ierror("imm const timer is not allowed as a formal parameter:", NS);
			break;
		    default:
			assert(0);
			break;
		    }
		}
		$$.v.type = UDF;			/* formal value parameter const int */
		$$.v.ftype = ARITH;
		$$.v.em &= ~EM;
		$$.v.fm = 0;
		iFunSyText = iFunBuffer;		/* expecting a new function symbol */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "formalConstTypeDecl: IMM CONST TYPE", &$$);
#endif
	    }
	;

	/************************************************************
	 * immediate function with immediate return value bit and int
	 * Call:
	 *	imm bit b1, b2; imm int i1, i2, delay; imm clock clk; imm timer tim;
	 *	b1  = bf1(b2, i1, clk, tim, delay);
	 *	i1  = if1(b2, i2, b1 & IX0.0, delay * 10);
	 * Mixed Call:
	 *	QX0.0  = bf1(b1 | b2, if1(...) + 5, cf1(...), tf1(...), 2);
	 ***********************************************************/

iFunCall: IFUNCTION '(' rParams ')'	{
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		$$ = cloneFunction(&$1, &$3, &$4);	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "iFunCall: IFUNCTION (rParams)", &$$);
#endif
	    }
	| IFUNCTION '(' rParams error ')'	{
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		$$ = cloneFunction(&$1, &$3, &$5);	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "iFunCall: IFUNCTION (rParams error)", &$$);
#endif
		yyerrok;
	    }
	;

	/************************************************************
	 * immediate function with immediate return value clock
	 * Call:
	 *	clk = cf1(b1, iClock);
	 ***********************************************************/

cFunCall: CFUNCTION '(' rParams ')'	{
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		$$ = cloneFunction(&$1, &$3, &$4);	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cFunCall: CFUNCTION (rParams)", &$$);
#endif
	    }
	| CFUNCTION '(' rParams error ')'	{
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		$$ = cloneFunction(&$1, &$3, &$5);	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cFunCall: CFUNCTION (rParams error)", &$$);
#endif
		yyerrok;
	    }
	;

	/************************************************************
	 * immediate function with immediate return value timer
	 * Call:
	 *	tim = tf1(b2, clk);
	 ***********************************************************/

tFunCall: TFUNCTION '(' rParams ')'	{
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		$$ = cloneFunction(&$1, &$3, &$4);	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "tFunCall: TFUNCTION (rParams)", &$$);
#endif
	    }
	| TFUNCTION '(' rParams error ')'	{
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		$$ = cloneFunction(&$1, &$3, &$5);	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "tFunCall: TFUNCTION (rParams error)", &$$);
#endif
		yyerrok;
	    }
	;

	/************************************************************
	 * void immediate function
	 * Call:
	 *	imm bit b1, b3; imm int i3, delay;
	 *	vf1(b3, i3, b1 & IX0.0, delay * 10);
	 ***********************************************************/

vFunCall: VFUNCTION '(' rParams ')'	{
		Lis	lis;
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		lis = cloneFunction(&$1, &$3, &$4);	/* clone function from call head */
		$$.f = lis.f; $$.l = lis.l;
		$$.v = 0;				/* $$.v is Sym - not compatible */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "vFunCall: VFUNCTION (rParams)", &$$);
#endif
	    }
	| VFUNCTION '(' rParams error ')'	{
		Lis	lis;
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		lis = cloneFunction(&$1, &$3, &$5);	/* clone function from call head */
		$$.f = lis.f; $$.l = lis.l;
		$$.v = 0;				/* $$.v is Sym - not compatible */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "vFunCall: VFUNCTION (rParams error)", &$$);
#endif
		yyerrok;
	    }
	;

	/************************************************************
	 *
	 *	Get real parameter list for immediate function calls
	 *
	 *	The first call to getRealParameter(0, $1.v) will set up a Symbol,
	 *	which is the head of a new parameter list. It is pointed at by hsp.
	 *		hsp->list points to the first parameter
	 *		hsp->u_blist points to the last parameter
	 *		hsp->fm is filled with a signature of value parameters
	 *
	 *	Subsequent calls link the next parameter to hsp->u_blist->le_next
	 *
	 *	All this is done before the execution of cloneFunction(), which
	 *	includes setting up the correct function head according to the
	 *	signature returned, analysing the parameters against the formal
	 *	parameters and actually cloning the function. Nested calls will
	 *	have their own *hsp, which obviates the need for an extra stack.
	 *
	 ***********************************************************/

rParams	: /* nothing */			{ $$.v =  0; }
	| rPlist			{ $$   = $1; }
	| rPlist ','			{ $$   = $1; $$.l = $2.l; }
	;

rPlist	: ractexpr			{
		$$.f = $1.f; $$.l = $1.l;
		if (checkConstExpr($1.v) == NULL) {
		    if (const_push(&$1)) { errInt(); YYERROR; }
		}
		$$.v = getRealParameter(0, $1.v);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "rPlist: ractexpr", &$$);
#endif
	    }
	| rPlist ',' ractexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if (checkConstExpr($3.v) == NULL) {			/* must extract value from aexpr */
		    if (const_push(&$3)) { errInt(); YYERROR; }
		}
		$$.v = getRealParameter($1.v, $3.v);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "rPlist: rPlist, ractexpr", &$$);
#endif
	    }
	;

	/********************************************************************
	 *
	 *	immC array declaration - these are arrays of immC bit or immC int nodes.
	 *
	 *	An 'immC' array is declared and initialised with 'immC' nodes in the
	 *	declaration. If no initialising list of members is given directly
	 *	in the declaration, the call will automatically generate a list of
	 *	'immC' nodes of the same type as the array being declared. The names
	 *	of the members are given the name of the array with a number 0 to n-1
	 *	appended, where n is the the size of the array, which must be specified
	 *	in this case. The number appended to the array base name is the index
	 *	of that member in the array (similar to iCa arrays).
	 *
	 *	    immC bit selBit[3];	// defines immC bit selBit0, selBit1, selBit2
	 *				// and initialises them as array members of
	 *				// array immC bit selBit, which is also defined
	 *	    immC int selInt[3];	// defines immC int selInt0, selInt1, selInt2
	 *				// and initialises them as array members of
	 *				// array immC int selInt, which is also defined
	 *
	 *	Alternatively an 'immC' array may be declared and initialised in the
	 *	declaration with differently named 'immC' variables of the same
	 *	'immC' type as the 'immC' array. In this case the size specification is
	 *	optional.
	 *
	 *	    immC bit selOpen[3] = { open0, open1, open2, };
	 *	    immC int selNumb[]  = { numb0, numb1, numb2, };
	 *
	 *	If a size for the array is specified it must match the number
	 *	of initialiser elements. If not specified the size is taken
	 *	from the number of initialisers.
	 *
	 *	In either case the member names of the array (either generated names
	 *	or names provided in the optional initialiser list) may be previously
	 *	declared 'immC' variables. In this case those 'immC' variables will be used.
	 *	A check is made, that those members are the same 'immC' type as the array.
	 *	Alternatively if the names are undefined, the 'immC' array declaration will
	 *	declare and reserve storage for the named members. The type ('immC bit' or
	 *	'immC int') will be taken from the type of the array being declared.
	 *
	 *	'immC' arrays may only be declared and optionally called with an initialiser
	 *	list once in one statement. Like simmple 'immC' declarations, 'immC' array
	 *	declarations reserve and initialise storage, but only to 0.
	 *
	 *	'immC' array declarations can be included in a comma separated list of
	 *	'immC' declarations. The keywords 'immC bit' or 'immC int' at the head
	 *	of the list need only be given once.
	 *
	 *	'immC' arrays are accessed with a numeric index, which selects a
	 *	particular 'immC' node stored during initialisation (similar to ALIAS).
	 *	Being immC, they may only be assigned in C code. Their value may be
	 *	accessed and used in any iC or C expression. In iC code only constant
	 *	numerical indices may be used, whereas in C code either immediate or
	 *	ordinary C variables in an index expression will be handled correctly.
	 *
	 *	    imm  bit trig = .;	// compute selection trigger trig in iC code
	 *	    imm  int si = ...;	// compute selection index si in iC code
	 *	    immC bit selAr[10];	// selection array
	 *	    %{ static int t; %}	// temporary C variable to remember si between
	 *				// the rising and falling edge of trig
	 *				// si may have changed before trig falls
	 *	    if (trig) {
	 *	       selAr[t=si] = 1;	// variable selection index si in C code
	 *	    } else {		// remember value of si in t for else part
	 *	       selAr[t]    = 0;	// selected array member is synchronous with trig
	 *	    }
	 *	    imm bit gate0 = ST(selAr[0], tick, 10);	// same as ST(selAr0, ...
	 *	    imm bit gate1 = ST(selAr[1], tick, 10);	// same as ST(selAr1, ...
	 *	    ...
	 *	    imm bit gate9 = ST(selAr[9], tick, 10);	// constant index in iC code
	 *
	 *	This selection is much faster than having large numbers of comparisons
	 *	with si as shown in the following iC code example doing the same job as
	 *	the code above:
	 *
	 *	    imm int si = ...	// compute selection index si in iC code
	 *	    imm bit gate0 = ST(trig & si == 0, tick, 10);
	 *	    imm bit gate1 = ST(trig & si == 1, tick, 10);
	 *	    ...
	 *	    imm bit gate2 = ST(trig & si == 9, tick, 10);
	 *
	 *	In this example all the selection comparisons are recomputed
	 *	every time si changes, although only one of them fires.
	 *	(Significant if selection is large).
	 *
	 *	For the selection with an 'immC' array only one selection array
	 *	element changes - the one selected by index si in the C fragment,
	 *	which triggers the correct gate immediately.
	 *
	 *******************************************************************/

immCarray
	: immCarrayHead			{		/* automatic declaration of members */
		Symbol *	sp;
		Symbol *	tsp;
		List_e *	lp;
		int		i;
		Valp		v;
		char		tempBuf[TSIZE];
		$$.f = $1.f; $$.l = $1.l;
		sp = $$.v = $1.v;
		assert((sp->em & EM) == 0 && (sp->type == ARNC || sp->type == LOGC || sp->type == ERR));
		if (sp->em & EI) {
		    v.lfl = 1;
		    v.lpt = sp->v_elist;
		    listGenOut(sp, -2, &v);			/* list immC array node with const parameter size */
		    assert(sp->list == 0);
		    sp->list = sp->v_elist;			/* transfer dynamic size expression pointer */
		} else {
		    if (sp->v_cnt == 0) {
			ierror("must specify array size since there is no member initialiser list:", $$.f);
			sp->type = ERR;
		    } else
		    if ((lp = sp->list) == 0) {
			listGenOut(sp, sp->v_cnt, 0);		/* list immC array node with numerical size */
			for (i = 0; i < sp->v_cnt; i++) {
			    snprintf(tempBuf, TSIZE, "%s%d", sp->name, i);	/* base name + index */
			    if ((tsp = lookup(tempBuf)) == 0) {
				tsp = install(tempBuf, sp->type, iC_ftypes[sp->type]);
				listGenOut(tsp, 0, 0);		/* list immC node and generate possible output */
			    }					/* not listed if member node already defined */
			    if (sp->type != tsp->type && sp->type != ERR) {
				ierror("array member type does not match type of whole array:", tsp->name);
				sp->type = ERR;			/* can only happen if member alread existed */
			    }
			    if (sp->list == 0) {
				sp->list = lp = sy_push(tsp);	/* head of new member list */
				lp->le_val = sp->v_cnt;
			    } else {
				lp = lp->le_next = sy_push(tsp);/* link to end of new member list */
			    }
			}
			listGenOut(NULL, 1, 0);			/* terminate immC array node list */
		    } else {
			assert(lp->le_val == sp->v_cnt);	/* previously declared extern */
			listGenOut(sp, sp->v_cnt, 0);		/* list immC array node */
			for (i = 0; i < sp->v_cnt; i++) {	/* use member list set up in extern declaration */
			    snprintf(tempBuf, TSIZE, "%s%d", sp->name, i);	/* base name + index */
			    if ((tsp = lookup(tempBuf)) != 0 && lp && tsp != lp->le_sym) {
				ierror("different array member to member in extern declaration:", tsp->name);
				sp->type = ERR;
			    } else {
				if (sp->type != lp->le_sym->type) {	/* ARNC or LOGC asserted above */
				    if (sp->type != ERR) {	/* TD might be assertion because tested in extern decl */
					ierror("array member type does not match type of whole array:", tsp->name);
					sp->type = ERR;	/* can only happen if member alread existed */
				    }
				} else if (tsp->em & EM) {	/* extern ARNC or LOGC ? */
				    tsp->em &= ~EM;		/* no longer extern */
				    listGenOut(tsp, 0, 0);	/* list immC node and generate possible output */
				}
			    }
			    lp = lp->le_next;
			}
			listGenOut(NULL, 1, 0);		/* terminate immC array node list */
		    }
		}
		if (iFunSymExt) {
		    char *	cp;
		    Symbol *	sp1;
		    cp = strchr(sp->name, '@');		/* locate original extension */
		    assert(cp && isprint(cp[1]));	/* extension must be at least 1 character */
		    if ((sp1 = lookup(++cp)) != 0 && (sp1->em & EM)) {
			warning("declaration of an extern variable in a function - ignored:", cp);
		    }
		    collectStatement(sp);
		    iFunSyText = 0;			/* no more function symbols */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "immCarray: immCarrayHead", &$$);
#endif
	    }
	| immCarrayHead '=' '{' immCini '}'	{
		Symbol *	sp;
		Symbol *	tsp;
		List_e *	lp;
		List_e *	tlp;
		List_e *	ttlp;
		int		i;
		$$.f = $1.f; $$.l = $5.l;
		sp = $$.v = $1.v;
		assert((sp->em & EM) == 0 && (sp->type == ARNC || sp->type == LOGC || sp->type == ERR));
		if (sp->em & EI) {
		    ierror("array size set by a constant parameter cannot have member initialisers:", $$.f);
		    sp->type = ERR;
		} else {
		    if (sp->v_cnt > 0 && sp->v_cnt != $4.v->le_val) {
			fprintf(stderr, "immCarray: array size %u, initaliser count %u\n", sp->v_cnt, $4.v->le_val);
			ierror("array size does not match number of member initialisers:", $$.f);
			sp->type = ERR;
		    }
		    sp->v_cnt = $4.v->le_val;			/* number of member initialisers is array size */
		    /* v_cnt is not preserved to outNet() - but list and count in list->le_val is */
		    tlp = ttlp = sp->list;			/* extern declaration if set */
		    listGenOut(sp, sp->v_cnt, 0);		/* list immC array node */
		    for (lp = $4.v, i = 0; lp; lp = lp->le_next, i++) {
			if (tlp && (ttlp = tlp->le_next, 1) && (tsp = sy_pop(tlp)) != lp->le_sym) {	/* remove extern List_e */
			    ierror("different array member to member in extern declaration:", tsp->name);
			    sp->type = ERR;
			}
			if (lp->le_sym->type == UDF) {
			    if (lp->le_sym->ftype == UDFA) {
				lp->le_sym->type = sp->type;	/* turn into immC variable */
				lp->le_sym->ftype = iC_ftypes[sp->type];
				assert((lp->le_sym->em & EM) == 0);
				listGenOut(lp->le_sym, 0, 0);	/* list immC node and generate possible output */
			    } else {
				goto TypErr;
			    }
			} else
			if (sp->type != lp->le_sym->type) {	/* ARNC or LOGC asserted above */
			    if (sp->type != ERR) {
			      TypErr:
				ierror("array member type does not match type of whole array:", lp->le_sym->name);
				sp->type = ERR;
			    }
			} else if (lp->le_sym->em & EM) {	/* extern ARNC or LOGC ? */
			    lp->le_sym->em &= ~EM;		/* no longer extern */
			    listGenOut(lp->le_sym, 0, 0);	/* list immC node and generate possible output */
			}
			tlp = ttlp;
		    }
		    listGenOut(NULL, 1, 0);			/* terminate immC array node list */
		    assert(tlp == NULL && i == sp->v_cnt);
		    sp->list = $4.v;				/* head of the member list - inherits initialiser count */
		}
		if (iFunSymExt) {
		    char *	cp;
		    Symbol *	sp1;
		    cp = strchr(sp->name, '@');			/* locate original extension */
		    assert(cp && isprint(cp[1]));		/* extension must be at least 1 character */
		    if ((sp1 = lookup(++cp)) != 0 && (sp1->em & EM)) {
			warning("declaration of an extern variable in a function - ignored:", cp);
		    }
		    collectStatement(sp);
		    iFunSyText = 0;				/* no more function symbols */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "immCarray: immCarrayHead = { immCini }", &$$);
#endif
	    }
	;

immCindex
	: /* nothing */		{ $$.v.lfl = $$.v.nuv = 0; }
	| constExpr		{ $$ = $1; }		/* constant expression or parameter* */
	;

immCarrayHead
	: declCHead UNDEF '[' immCindex ']'	{
		Symbol *	sp;
		$$.f = $1.f; $$.l = $5.l;
		sp = $$.v = $2.v;
		sp->type  = $1.v.type;			/* only ARNC and LOGC */
		sp->ftype = UDFA;			/* makes it a IMMCARRAY */
		assert(($1.v.em & EM) == 0);		/* has been cleared in declCHead */
		sp->em    &= ~EM;			/* clear bit EM here */
		if (sp->type == ARNC || sp->type == LOGC) {
		    if ($4.v.lfl == 0) {
			sp->v_cnt = (unsigned int)($4.v.nuv);	/* array size or 0 */
			sp->em &= ~EI;			/* mark as constant */
		    } else
		    if ($4.v.lfl == 1) {
			sp->v_elist = $4.v.lpt;		/* pointer to constant parameter */
			sp->em |= EI;			/* mark as pointer */
		    }
		} else {
		    ierror("Definition of a non immC array is illegal:", $$.f);
		    sp->type = ERR;
		}
#if YYDEBUG
		Symbol t = *(sp);
		if ((iC_debug & 0402) == 0402) pd("immCarrayHead: declCHead UNDEF [ immCindex ]", sp, $1.v, &t);
#endif
	    }
	| declCHead IMMCARRAY '[' immCindex ']'	{
		int		typ;			/* gets here when IMMCARRAY was */
		Symbol *	sp;			/* previously declared extern */
		$$.f = $1.f; $$.l = $5.l;
		sp = $$.v = $2.v;
		typ  = $1.v.type;			/* ARNC and LOGC */
		assert(sp && (typ == ARNC || typ == LOGC));
		if (sp->ftype != UDFA || sp->type != typ || sp->list->le_val == 0) {
		    ierror("declaration does not match previous declaration:", sp->name);
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		} else
		if (sp->em & EM) {
		    if ($4.v.lfl == 0) {
			if ($4.v.nuv > 0 && sp->list->le_val != (unsigned int)($4.v.nuv)) {
			    fprintf(stderr, "immCarrayHead: extern array size %u, array size %u\n", sp->list->le_val, $4.v.nuv);
			    ierror("array size does not match previous extern declaration:", $$.f);
			    sp->type = ERR;
			}
			sp->em &= ~EI;			/* mark as constant */
		    } else
		    if ($4.v.lfl == 1) {
			/* can only get here if extern declaration had a fixed size and this one is dynamic */
			fprintf(stderr, "immCarrayHead: extern array size %u\n", sp->list->le_val);
			ierror("dynamic array size cannot match previous fixed extern declaration:", $$.f);
			sp->type = ERR;
		    }
		    sp->em    &= ~EM;			/* clear bit EM here */
		    sp->v_cnt = sp->list->le_val;	/* array size from extern */
		} else {
		    ierror("array already defined - cannot define again:", $$.f);
		    sp->type = ERR;
		}
#if YYDEBUG
		Symbol t = *(sp);
		if ((iC_debug & 0402) == 0402) pd("immCarrayHead: declCHead IMMCARRAY [ immCindex ]", sp, $1.v, &t);
#endif
		iFunSyText = 0;				/* no more function symbols */
	    }
	;

	/************************************************************
	 * list of immC array members - must have at least one member
	 ***********************************************************/

immCini
	: immCiniList		{ $$ = $1; }
	| immCiniList ','	{ $$ = $1; }
	;

cVariable
	: UNDEF			{ $$   = $1; }		/* turned into immC variable */
	| LVARC			{ $$   = $1; }		/* logical immC bit variable */
	| AVARC			{ $$   = $1; }		/* arithmetic immC int variable */
	| immCarrayVariable	{ $$   = $1; }		/* immC array member variable */
	;

immCiniList
	: cVariable			{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = checkDecl($1.v);			/* first link in member list */
		assert($1.f == 0 || ($1.f >= iCbuf && $1.l < &iCbuf[IMMBUFSIZE]));
		$$.v->le_val = 1;			/* count first member */
		$$.v->le_first = $1.f;
		$$.v->le_last = $1.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "immCiniList: cVariable", &$$);
#endif
	    }
	| immCiniList ',' cVariable	{
		List_e *	lp;

		$$.f = $1.f; $$.l = $3.l;
		lp = $$.v;
		while (lp->le_next) {
		    lp = lp->le_next;
		}
		lp = lp->le_next = checkDecl($3.v);	/* link to end of member list */
		assert($3.f == 0 || ($3.f >= iCbuf && $3.l < &iCbuf[IMMBUFSIZE]));
		$$.v->le_val++;				/* count later members */
		lp->le_first = $3.f;
		lp->le_last = $3.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "immCiniList: immCiniList , cVariable", &$$);
#endif
	    }
	;

	/********************************************************************
	 *
	 *	Extern immC array declaration
	 *
	 *	An 'immC' array in another source may be declared extern, in which case
	 *	the array and its members may be accessed in that source.
	 *
	 *	The form of the extern declaration must be the same as the normal
	 *	declaration with the word 'extern' preceding the type. In particular
	 *	if the array will be finally declared with an initialiser list
	 *	the extern declaration must be provided with an identical initialiser
	 *	list. If the final declaration will be without an initialiser list
	 *	the extern declaration must be the same. The reason for this is, that
	 *	an extern array declaration not only declares the 'immC' array extern
	 *	but it also declares all of its members extern. Only this way can the
	 *	members of an 'immC' array be accessed correctly in another source.
	 *
	 *	The four arrays declared in the previous section are declared extern
	 *	as follows:
	 *
	 *	    extern immC bit selBit[3];
	 *	    extern immC int selInt[3];
	 *
	 *	    extern immC bit selOpen[3] = { open0, open1, open2, };
	 *	    extern immC int selNumb[]  = { numb0, numb1, numb2, };
	 *
	 *	These extern array declarations will generate the following extern
	 *	declarations internally.
	 *
	 *	    extern immC bit selBit0, selBit1, selBit2;
	 *	    extern immC int selInt0, selInt1, selInt2;
	 *
	 *	    extern immC bit open0, open1, open2;
	 *	    extern immC int numb0, numb1, numb2;
	 *
	 *	If these simple 'extern immC' declarations had already been coded
	 *	previously, only a check is made that the types match.
	 *
	 *	Like 'extern immC' variable declarations, 'extern immC' array declarations
	 *	do not reserve any storage. 'extern immC' array declarations can be
	 *	included in a comma separated list of 'extern immC' declarations. The
	 *	keywords 'extern immC bit' or 'extern immC int' at the head of the
	 *	list need only be given once.
	 *
	 *	Bits EM and EX are set in em, just like in extern declarations for
	 *	scalar immediate variables.
	 *
	 *	In good C practice, the 'extern immC' array declarations should be
	 *	coded in a header file, which is to be included in each source
	 *	using the 'immC' array. In the source finally declaring, defining and
	 *	initialising the array, the extern declaration in the header will
	 *	precede the full declaration. At that point a check is made, that
	 *	the type of the array and the types and names of all of its members
	 *	are identical to the previous extern declaration. That way a full
	 *	check is made at immediate C compile time - linker errors are avoided.
	 *
	 *******************************************************************/

extimmCarray
	: extimmCarrayHead			{	/* automatic declaration of members */
		Symbol *	sp;
		Symbol *	tsp;
		List_e *	lp;
		List_e *	tlp;
		int		i;
		char		tempBuf[TSIZE];
		$$.f = $1.f; $$.l = $1.l;
		sp = $$.v = $1.v;
		assert((sp->em & EM) && (sp->type == ARNC || sp->type == LOGC || sp->type == ERR));
		if ((lp = sp->list) != NULL) {
		    warning("previous extern array declaration - ignored:", sp->name);
		    while (lp) {
			tlp = lp->le_next;		/* pick up next before free */
			sy_pop(lp);			/* free previous List_e's */
			lp = tlp;
		    }
		    sp->list = NULL;
		}
		if (iFunSymExt) {
		    ierror("cannot extern declare an array in a function block:", $$.f);
		    sp->type = ERR;
		} else
		if (sp->em & EI) {
		    ierror("extern array cannot have size set by a constant parameter:", $$.f);
		    sp->type = ERR;
		} else {
		    if (sp->v_cnt == 0) {
			ierror("must specify array size since there is no member initialiser list:", $$.f);
			sp->type = ERR;
		    } else {
			for (i = 0; i < sp->v_cnt; i++) {
			    snprintf(tempBuf, TSIZE, "%s%d", sp->name, i);	/* base name + index */
			    if ((tsp = lookup(tempBuf)) == 0) {
				tsp = install(tempBuf, sp->type, iC_ftypes[sp->type]);
				tsp->em |= EM|EX;	/* make newly created member external immC */
			    }
			    if (sp->type != tsp->type && sp->type != ERR) {
				ierror("array member type does not match type of whole array:", tsp->name);
				sp->type = ERR;		/* can only happen if member alread existed */
			    }
			    if (sp->list == NULL) {
				sp->list = lp = sy_push(tsp);	/* head of the member list */
				lp->le_val = sp->v_cnt;
			    } else {
				lp = lp->le_next = sy_push(tsp);
			    }
			}
		    }
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "extimmCarray: extimmCarrayHead", &$$);
#endif
	    }
	| extimmCarrayHead '=' '{' immCini '}'	{
		Symbol *	sp;
		List_e *	lp;
		List_e *	tlp;
		$$.f = $1.f; $$.l = $5.l;
		sp = $$.v = $1.v;
		assert((sp->em & EM) && (sp->type == ARNC || sp->type == LOGC || sp->type == ERR));
		if ((lp = sp->list) != NULL) {
		    warning("previous extern array declaration - ignored:", sp->name);
		    while (lp) {
			tlp = lp->le_next;		/* pick up next before free */
			sy_pop(lp);			/* free previous List_e's */
			lp = tlp;
		    }
		    sp->list = NULL;
		}
		if (iFunSymExt) {
		    ierror("cannot extern declare an array in a function block:", $$.f);
		    sp->type = ERR;
		} else
		if (sp->em & EI) {
		    ierror("extern array cannot have size set by a constant parameter:", $$.f);
		    sp->type = ERR;
		} else {
		    if (sp->v_cnt > 0 && sp->v_cnt != $4.v->le_val) {
			fprintf(stderr, "extimmCarray: array size %u, initaliser count %u\n", sp->v_cnt, $4.v->le_val);
			ierror("array size does not match number of member initialisers:", $$.f);
			sp->type = ERR;
		    }
		    sp->v_cnt = $4.v->le_val;			/* number of member initialisers is array size */
		    /* v_cnt is not preserved to outNet() - but list and count in list->le_val is */
		    for (lp = $4.v; lp; lp = lp->le_next) {
			if (lp->le_sym->type == UDF) {
			    if (lp->le_sym->ftype == UDFA) {
				lp->le_sym->type = sp->type;	/* turn into immC variable */
				lp->le_sym->ftype = iC_ftypes[sp->type];
				lp->le_sym->em |= EM|EX;	/* make member external immC */
			    } else {
				goto ExtTypErr;
			    }
			}
			if (sp->type != lp->le_sym->type && sp->type != ERR) {
			  ExtTypErr:
			    ierror("array member type does not match type of whole array:", lp->le_sym->name);
			    sp->type = ERR;
			}
		    }
		    sp->list = $4.v;				/* head of the member list - inherits initialiser count */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "extimmCarray: extimmCarrayHead = { immCini }", &$$);
#endif
	    }
	;

extimmCarrayHead
	: extDeclHead UNDEF '[' immCindex ']'	{
		Symbol *	sp;
		$$.f = $1.f; $$.l = $5.l;
		sp = $$.v = $2.v;
		sp->type  = $1.v.type;
		sp->ftype = UDFA;			/* makes it a IMMCARRAY */
		assert($1.v.em & (EM|EX));		/* has been set in extDeclHead */
		sp->em    |= EM|EX;			/* set em for extern declaration */
		if (sp->type == ARNC || sp->type == LOGC) {
		    if ($4.v.lfl == 0) {
			sp->v_cnt = (unsigned int)($4.v.nuv);	/* array size or 0 */
			sp->em &= ~EI;			/* mark as constant */
		    } else
		    if ($4.v.lfl == 1) {
			ierror("extern declaration of a parameterised function internal immC array is illegal:", $$.f);
			sp->type = ERR;
		    }
		} else {
		    ierror("extern declaration of a non immC array is illegal:", $$.f);
		    sp->type = ERR;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    pd("extimmCarrayHead: extDeclHead UNDEF [ immCindex ]", sp, $1.v, $2.v);
		}
#endif
	    }
	| extDeclHead IMMCARRAY '[' immCindex ']'	{
		$$.f = $1.f; $$.l = $5.l;
		char *	cp;
		Symbol *	sp;
		sp = $$.v = $2.v;
		assert(sp && (sp->type == ARNC || sp->type == LOGC));
		if (sp->ftype != UDFA) {
		    ierror("extern array declaration does not match previous declaration:", $2.v->name);
		    $2.v->type = ERR;			/* cannot execute properly */
		}
		if (iFunSymExt && (cp = strchr($2.v->name, '@'))) {
		    warning("extern array declaration of internal function variable - ignored:", cp+1);
		} else
		if ((sp->em & EM) && $2.v->type != ERR) {	/* prev extern but not ERROR */
		    sp->type  = $1.v.type;
		    sp->ftype = UDFA;			/* makes it a IMMCARRAY */
		    assert($1.v.em & (EM|EX));		/* has been set in extDeclHead */
		    sp->em    |= EM|EX;			/* set em for extern declaration */
		    if ($4.v.lfl == 0) {
			sp->v_cnt = (unsigned int)($4.v.nuv);	/* array size or 0 */
			sp->em &= ~EI;			/* mark as constant */
		    } else
		    if ($4.v.lfl == 1) {
			ierror("extern declaration of a parameterised function internal immC array is illegal:", $$.f);
			sp->type = ERR;
		    }
		} else {
		    warning("second extern array declaration - ignored:", $2.v->name);
		} /* TD FIX */
		if (iFunSymExt) {
		    ierror("extern array declaration in function definition after assignment:", $2.v->name);
		    sp->type = ERR;			/* stop use as a statement in function */
		    sp->em   |= EM|EX;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    pd("extimmCarrayHead: extDeclHead IMMCARRAY [ immCindex ]", sp, $1.v, $2.v);
		}
#endif
	    }
	;

	/************************************************************
	 *
	 *	immC array variable
	 *
	 *	An 'immC' array member may be used as an immediate variable
	 *	like any other 'immC' variable.
	 *
	 *	    selOpen[0]
	 *	    selNumb[1]
	 *
	 *	A numerical index in the range of 0 to n-1 may be used
	 *	where n is the size of the 'immC' array
	 *
	 *	The return is the 'immC' member of the array, which can
	 *	even be used to initialise other 'immC' arrays
	 *
	 ***********************************************************/

immCarrayVariable
	: IMMCARRAY '[' constExpr ']'	{
		$$.f = $1.f; $$.l = $4.l;
		Symbol *	sp;
		List_e *	lp;
		int		index;
		int		i;
		$$.v = sp = $1.v;
		assert(sp && sp->ftype == UDFA);
		if (iFunSymExt && sp->list == NULL) {
		    warning("anonymous immC array - it's members cannot be accessed in iC code:", sp->name);
		}
		if ((lp = sp->list) == NULL) {
		    if (($$.v = lookup("iCerr")) == 0) {
			icerr = $$.v = install("iCerr", ERR, GATE);
		    }
		    $$.v->type = ERR;			/* in case of error */
		    ierror("immC array must be initialised to use its members:", sp->name);
		} else {
		    if ($3.v.lfl == 0) {
			index = $3.v.nuv;		/* array index */
			if (index < 0 || index >= lp->le_val) {
			    if (($$.v = lookup("iCerr")) == 0) {
				icerr = $$.v = install("iCerr", ERR, GATE);
			    }
			    $$.v->type = ERR;		/* in case of error */
			    ierror("immC array member is not in range:", sp->name);
			} else {
			    for (i = 0; i < index; i++) {
				lp = lp->le_next;	/* select indexed member */
				if (lp == NULL || lp->le_sym == NULL) {
				    execerror("immCarrayVariable: member not initialised by compiler", sp->name, __FILE__, __LINE__);
				    /* compiler error - flushes rest of file */
				}
			    }
			    $$.v = lp->le_sym;		/* return the member Symbol */
			    $$.v->em &= ~EI;		/* mark as member returned by constant index */
			}
		    } else
		    if ($3.v.lfl == 1) {
			$$.v->v_elist = $3.v.lpt;	/* pointer to constant parameter */
			$$.v->em |= EI;			/* mark as pointer */
		    }
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "immCarrayVariable: IMMCARRAY [ constExpr ]", &$$);
#endif
	    }
	;

	/************************************************************
	 *
	 * end of iC grammar
	 *
	 ***********************************************************/

%%

#ifdef NEWSTYLE
#define CBUFSZ 166
#define YTOKSZ 166
#else
#define CBUFSZ 125				/* listing just fits on 132  cols */
#define YTOKSZ 66
#endif
#define ABUFSZ 2048				/* allow for a large comment block */
static char	chbuf[2][CBUFSZ];		/* used in get() errline() andd yyerror() */
static char	prevNM[BUFS];
static char *	getp[2] = {NULL,NULL};		/* used in get() unget() andd yyerror() */
static char	iCtext[YTOKSZ];			/* lex token */
static int	iCleng;				/* length */
static int	lineflag;
static char *	errFilename;
static int	errFlag = 0;
static int	errRet = 0;
static int	eofLineno = 0;
static int	savedLineno = 0;
static int	errline = 0;

static char	auxBuf[ABUFSZ];
static char *	auxp = 0;
static char *	auxe = 0;

/********************************************************************
 *	lexflag is bitmapped and controls the input for lexers
 *******************************************************************/
int		lexflag = 0;
int		lineno = 0;			/* count first line on entry to get() */

int		c_number = 0;			/* case number for cexe.c */
int		outFlag = 0;			/* global flag for compiled output */
static jmp_buf	begin;
static int	lex_typ[] = { DEF_TYP };	/* tokens corresponding to type */
static int	lex_act[] = { DEF_ACT };	/* tokens corresponding to ftype */

/********************************************************************
 *
 *	Compile an iC language source file whose name is in 'inpPath'
 *	if inpPath is a null pointer use standard input (stdin)
 *	a copy of the source file name (or "stdin") is kept in inpNM for
 *	error messages. This name and the variable 'lineno' are
 *	modified by a #line 1 "file.ic" pre-processor directive.
 *
 *	listing text is directed to 'lstNM' (default stdout)
 *	error   text is directed to 'errNM' (default stderr)
 *
 *	a prototype for C function execution is read and extended
 *	with the C fragments possibly contained in the iC program
 *
 *******************************************************************/

int
iC_compile(
    char *	inpPath,			/* input file path */
    char *	lstNM,				/* list file name */
    char *	errNM,				/* error file name */
    char *	outNM)				/* C output file name */
{
    char	execBuf[BUFS];
    char	lineBuf[BUFS];
    int		fd;
    int		r  = 1;
    int		r1 = 1;

    lineno = 0;
#if YYDEBUG
    if ((iC_debug & 0402) != 0402)		/* unless logic generation */
#endif
	lexflag = C_FIRST;			/* suppress initial pre-processor lines */
    lineflag = 1;				/* previous line was complete */

    if (lstNM) {				/* list file nominated ? */
	if ((chmod(lstNM, 0644) &&		/* yes - make list file writable */
	errno != ENOENT) ||			/* if it exists - so it can be re-opened */
	(iC_outFP = fopen(lstNM, "w")) == NULL) {
	    perror("chmod or fopen");
	    return Lindex;
	}
    }						/* else iC_outFP = stdout from initialisation */
    iC_lstFP = iC_outFP;			/* back up iC_outFP for restoring after 'no list' */
    outFlag = outNM != 0;			/* global flag for compiled output */
    errFilename = errNM;			/* open on first error */
    iC_init();					/* install constants and built-ins - allows ierror() */
    if (inpPath) {
	strncpy(inpNM, inpPath, BUFS);
	r = 0;
	if (strlen(iC_defines) == 0) {
	    /* pre-compile if iC files contains any %include, %define %if etc */
#ifdef	WIN32
	    fflush(iC_outFP);
	    /* CMD.EXE does not know '' but does not interpret $, so use "" */
	    /* don't use system() because Win98 command.com does not return exit status */
	    /* use spawn() instead - spawnlp() searches Path */
	    snprintf(execBuf, BUFS, "\"$s=1; while (<>) { if (m/^\\s*%\\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error)\\b/) { $s=0; last; } } exit $s;\"");
	    r = _spawnlp( _P_WAIT, "perl",  "perl",  "-e", execBuf, inpPath, NULL );
	    if (r < 0) {
		ierror("cannot spawn perl -e ...", inpPath);
		perror("spawnlp");
		return Iindex;			/* error opening input file */
	    }
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### test: perl -e %s %s; $? = %d\n", execBuf, inpPath, r);
#endif
#else	/* not WIN32 */
	    snprintf(execBuf, BUFS, "%s %s",
		"perl -e '$s=1; while (<>) { if (m/^\\s*%\\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error)\\b/) { $s=0; last; } } exit $s;'",
		inpPath);
	    r = system(execBuf);		/* test with perl script if %define %include %if etc in input */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### test: %s; $? = %d\n", execBuf, r);
#endif
#endif	/* WIN32 */
	}
#if YYDEBUG
	else {
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### iC_defines = %s; $? = %d\n", iC_defines, r);
	}
#endif
	if (r == 0) {
	    /* iC_defines is not empty and has -Dyyy or -Uyyy or %include etc was found by perl script */
	    /* pass the input file through the 'immac -M' to resolve %includes and macros */
	    if ((fd = mkstemp(T0FN)) < 0 || close(fd) < 0 || unlink(T0FN) < 0) {
		ierror("compile: cannot make or unlink:", T0FN);
		perror("unlink");
		return T0index;			/* error unlinking temporary file */
	    }
	    snprintf(execBuf, BUFS, "immac -M%s %s -I/usr/local/include -o %s %s 2> %s",
		iC_aflag, iC_defines, T0FN, inpPath, T6FN);
	    r1 = system(execBuf);		/* Pre-compile iC file with immac -M or immac -Ma */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### pre-compile: %s; $? = %d\n", execBuf, r1>>8);
#endif
	    if (r1 != 0) {
		if ((T6FP = fopen(T6FN, "r")) == NULL) {
		    return T6index;		/* error opening immac error file */
		}
		while (fgets(lineBuf, sizeof lineBuf, T6FP)) {
		    ierror("immac:", lineBuf);	/* immac error message */
		}
		fclose(T6FP);
		if (!(iC_debug & 04000)) {
		    unlink(T6FN);
		}
		return Iindex;
	    }
	    if (!(iC_debug & 04000)) {
		unlink(T6FN);
	    }
	    if ((T0FP = fopen(T0FN, "r")) == NULL) {
		return T0index;			/* error opening intermediate file */
	    }
	} else if ((T0FP = fopen(inpNM, "r")) == NULL) {
	    return Iindex;			/* error opening input file */
	}
    }						/* else inpPath == NULL T0FP is stdin inpNM is "stdin" */
    strncpy(prevNM, inpNM, BUFS);
    if (iC_debug & 046) {			/* begin source listing */
	fprintf(iC_outFP, "******* %-15s ************************\n", inpNM);
    }
    setjmp(begin);
    for (initcode(); iCparse(); initcode()) {
	;
    }
    if (r == 0 && !(iC_debug & 04000)) {
	unlink(T0FN);
    }
    if (inpPath) fclose(T0FP);
    T0FP = 0;
    return errRet;
} /* iC_compile */

/********************************************************************
 *
 *	Get routine for iC and C grammars
 *
 *	fp	FILE pointer for the source of data
 *	x	0	uses getp[0] as its buffer - used by iC-compile
 *		1	uses getp[1] - used by lexc.l for C-compile
 *
 *******************************************************************/

int
get(FILE* fp, int x)
{
    int		c;
    int		temp1;
    int		slen;
    int		prevflag;
    char *	cp;
    char	tempNM[BUFS];
    char	tempBuf[TSIZE];
%ifndef	BOOT_COMPILE
    static int	genCount = 0;
%endif	/* BOOT_COMPILE */

    /************************************************************
     *  The following block became necessary because 2 problems
     *  occurred when compiling the "if (condition) block" code.
     *  The parser needs to do lookahead to the next token in
     *  case this is an "else", in which case the statement is
     *  extended. Unfortunately that token is usually on a new line,
     *  and that line will  be listed as soon as that token is read.
     *  This means that the generated logic output for the preceding
     *  "if" statement (without an "else") will be listed after a quite
     *  unrelated statement following the "if" statement.
     *
     *  This has been an untidiness, which has bugged me for years.
     *
     *  A more serious consequence is, that the text arithmetic, which
     *  works on complete statements will come undone for the follow on
     *  statement, because if that statement is an arithmetic assignment,
     *  the text buffer is cleared by the end of the "if" statement after
     *  the first token of the assignment is proceessd.
     *  This has not led to any serious compiler problems in all
     *  the years of development (the text of that first token is
     *  only needed in embedded assignments by which time the start
     *  of the statement has been passed). But it did show up in
     *  debugging output with the -d2 flag, where a negative increment
     *  was calculated for the length of a statement, which resulted
     *  in a huge buffer overflow.
     *
     *  The solution is to mark the end of recognising a completed
     *  cBlock by setting cBlockFlag. At this point the remaining
     *  input text is saved in auxBuf after a space character.
     *  A lookahead of as many lines as needed is then carried
     *  out to find a token (which is any non white space string).
     *  If that token is "else" proceed as before. Otherwise replace
     *  the initial space in auxBuf with the character '\x1f'. In
     *  yylex this will generate the BF yacc token, which must appear
     *  at the end of ffexpr in a statement (another change).
     *
     *  Unfortunately the whole comment recognising gumph must be put
     *  in too, because otherwise a comment between the initial
     *  "if" part and the "else" will lead to a syntax error.
     *
     *  This way the logic listing text for ffexpr is generated in
     *  op_asgn(0, ..) before the next line is actually read below.
     *  Thus the follow up listing lines will be in the correct place.
     *
     *  Input for get() is taken from auxBuf[] via auxp until that
     *  is exhausted. Then further input is take from the input file.
     ************************************************************/
    if (x == 0 && cBlockFlag) {
	auxp = auxBuf;
	*auxp++ = ' ';				/* placeholder to be overwritten by '\x1f' */
	auxe = 0;
	if (getp[0] == 0) {			/* happens if file ends without '\n' */
	    getp[0] = strchr(chbuf[0], '\0');	/* point to string terminator */
	}
	strncpy(auxp, getp[0], ABUFSZ-1);	/* transfer rest if input to auxBuf[] */
	while (1) {
	    while (sscanf(auxp, " %s", tempBuf) != 1) {
		cp = auxe;
		auxe = auxp + strlen(auxp);	/* the rest of the current line has no token */
		if ((auxp = fgets(auxe, &auxBuf[ABUFSZ] - auxp, fp)) == NULL) {
		    auxe = cp;			/* file has terminated after initial if () cBlock */
		    tempBuf[0] = '\0';		/* no else will be found - complete if statement */
		    goto endInput;
		}
	    }
	    if (memcmp(tempBuf, "/*", 2) == 0) { /* a C comment block has started */
		cp = strchr(auxp, '*');
		assert(cp);			/* because of previous compare */
		do {				/* start C style comment */
		    while ((cp = strchr(cp + 1, '*')) == 0) {
			cp = auxe;
			auxe = auxp + strlen(auxp);	/* the line has no comment end */
			if ((auxp = fgets(auxe, &auxBuf[ABUFSZ] - auxp, fp)) == NULL) {
			    auxe = cp;		/* file terminated in C comment after if ... */
			    tempBuf[0] = '\0';	/* no else will be found - complete if statement */
			    goto endInput;
			}
			cp = auxp - 1;
			continue;
		    }
		} while (*++cp != '/');
		auxp = cp + 1;			/* C comment end was found */
		continue;
	    } else
	    if (memcmp(tempBuf, "//", 2) == 0 || /* a C++ comment line was found */
		memcmp(tempBuf, "#", 1) == 0) {	/* or a C-preprocessor line - # 1 */
		auxp += strcspn(auxp, "\n");	/* both are terminated by '\n' */
		continue;
	    }
	  endInput:
	    break;				/* no comments */
	}
	slen = strcspn(auxBuf, "\n");
	if (auxBuf[slen] == '\n') {
	    slen++;
	}
	if (strcmp(tempBuf, "else") != 0) {	/* a non-white string was found - it is a token */
	    auxBuf[0] = '\x1f';			/* this character generates the BF yacc token */
	    getp[0] = strncpy(chbuf[0], auxBuf, slen); /* copy the BF + first line back to chbuf[] */
	    chbuf[0][slen] = '\0';		/* terminate */
	}
	auxp = auxe ? auxBuf + slen : 0;	/* get ready for more lines in auxBuf[] */
	cBlockFlag = 0;
    }
    prevflag = lineflag;
    while (getp[x] == 0 || (c = *getp[x]++) == 0) {
	/************************************************************
	 *  getp has reached end of previous chbuf filling
	 *  fill chbuf with a new line
	 *  NOTE: getp === NULL at start of file (chbuf has EOF line)
	 *
	 *  Fill either from auxBuf if some lines were saved or from fp
	 *  The first token will be in the last line of auxBuf[]
	 ************************************************************/
	if ((prevflag = lineflag) != 0) {
	    lineno++;				/* count previous line */
	}
	/************************************************************
	 *  Input for get() is taken from auxBuf[] via auxp until that
	 *  is exhausted. Then further input is take from the input file.
	 ************************************************************/
	if (x == 0 && auxp) {
	    slen = strcspn(auxp, "\n");
	    if (auxp[slen] == '\n') {
		slen++;
	    }
	    getp[0] = strncpy(chbuf[0], auxp, slen); /* copy the BF + first part back to chbuf[] */
	    chbuf[0][slen] = '\0';		/* terminate */
	    auxp += slen;			/* get ready for more lines in auxBuf[] */
	    if (strlen(auxp) == 0) {
		auxp = 0;			/* that was it - now input is taken from file again */
	    }
	} else
	if ((getp[x] = fgets(chbuf[x], CBUFSZ, fp)) == NULL) {
	    if ((lexflag & C_PARSE) == 0) {
		eofLineno = lineno;		/* iC parse */
	    } else {
		lineno = eofLineno;		/* C parse with lex */
	    }
	    errline = lineno;			/* no listing line at EOF */
	    return (EOF);
	}
	lineflag = chbuf[x][strlen(chbuf[x])-1] == '\n' ? 1 : 0; /* this line terminated with \n */

	/********************************************************
	 *  handle different preprocessor lines
	 *  identify leading '#' for efficiency
	 *  Note: white space in scanf format matches any amount of
	 *        white space including none (have used single blank)
	 *  NOTE: chbuf[] must be large enough to hold a complete
	 *        pre-processor line for the following sscanf()s
	 ********************************************************/
	if (prevflag && sscanf(chbuf[x], " %1s", tempBuf) == 1 && *tempBuf == '#') {
	    if ((lexflag & C_PARSE) && strncmp(chbuf[x], "##", 2) == 0) {
		lexflag |= C_BLOCK;		/* block source listing for lex */
		lineno = savedLineno;
	    }
	    /********************************************************
	     *  WIN32 - cl pre-processor
	     *    handle C-pre-processor #line 1 "c:\\usr\\include\\stdio.h"
	     *  Linux or Cygnus - gcc or Intel icc pre-processor
	     *    handle C-pre-processor # 1 "/usr/include/stdio.h"
	     ********************************************************/
	    if ((lexflag & C_PARSE) == 0) {
		/* iC parse only */
#ifdef	WIN32
		if (strchr(chbuf[x], '\\') != 0) {
		    int	    c;
		    int	    slFlag;
		    char *  srcp;
		    char *  destp;

		    slFlag = 0;
		    srcp = destp = chbuf[x];	/* modify source line */
		    while ((c = *srcp++) != 0) {
			if (c != '\\') {
			    *destp++ = c;
			    slFlag = 0;		/* copy '\' after another character */
			} else if (slFlag == 0) {
			    *destp++ = '/';	/* convert '\\' to '/' under WIN32 */
			    slFlag = 1;		/* skip 2nd, 4th ... '\' */
			} else {
			    slFlag = 0;		/* copy 3rd, 5th ... '\' */
			}
		    }
		    *destp++ = '\0';		/* terminate string */
		}
		/********************************************************
		 *  iC-compile
		 *  handle pre-processor #line 1 "file.ic"	// WIN32
		 *  handle pre-processor # 1 "file.ic"		// GCC
		 ********************************************************/
		if (sscanf(chbuf[x], " # line %d \"%[-/:A-Za-z_.0-9<>]\"",.&temp1, inpNM) == 2)
#else	/* not WIN32 */
		if (sscanf(chbuf[x], " # %d \"%[-/A-Za-z_.0-9<>]\"", &temp1, inpNM) == 2)
#endif	/* WIN32 */
		{
		    lineno = temp1 - 1;		/* handled in iC-code */
		    lexflag |= C_LINE;
		    if (strcmp(inpNM, prevNM) != 0 && strchr(inpNM, '<') == 0) {
			lexflag &= ~C_FIRST;	/* report # 1 if file has changed */
			/* don't change to inpNM <built-in> and sytem headers */
			if (temp1 <= 1) {
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, ">>> get: # %3d  %-10s >> %-10s, useStackIndex = %d, uses = %o\n", temp1, prevNM, inpNM, iC_useStackIndex, iC_uses);
#endif
			    assert(iC_useStackIndex < USESTACKSZ);
			    iC_useStack[iC_useStackIndex++] = iC_uses; /* save use values */
			} else {
			    assert(iC_useStackIndex > 0);
			    iC_uses = iC_useStack[--iC_useStackIndex]; /* restore use values */
			    blockUnblockListing();
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "<<< get: # %3d  %-10s << %-10s, useStackIndex = %d, uses = %o\n", temp1, inpNM, prevNM, iC_useStackIndex, iC_uses);
#endif
			}
			strncpy(prevNM, inpNM, BUFS);	/* inpNM has been modified */
		    }
		    getp[x] = NULL;		/* bypass parsing this line in iC */
		}
	    } else
	    /********************************************************
	     *  handle C-pre-processor # pragma ## comes out on MAC OsX
	     ********************************************************/
	    if (sscanf(chbuf[x], " # %s", tempBuf) == 1 && strcmp(tempBuf, "pragma") == 0) {
		getp[x] = NULL;			/* ignore and bypass parsing # pragma */
	    } else
	    /********************************************************
	     *  there should be no other # lines remaining in iC file
	     ********************************************************/
	    if ((lexflag & C_PARSE) == 0) {
		/* handle only local includes - not full C-precompiler features */
		ierror("get: invalid # pre-processor line in iC ???", NULL);
		getp[x] = NULL;			/* bypass parsing this line in iC */
	    } else
	    /********************************************************
	     *  C-compile
	     *  handle pre-processor #line 1 "file.ic"
	     ********************************************************/
	    if ((slen = sscanf(chbuf[x],
#ifdef	WIN32
		" # line %d \"%[-/:A-Za-z_.0-9<>]\"	%s",
#else	/* not WIN32 */
		" # line %d \"%[-/A-Za-z_.0-9<>]\"	%s",
#endif	/* WIN32 */
		&temp1, tempNM, tempBuf)) >= 2) {
		savedLineno = lineno;
		lineno = temp1 - 1;
%ifndef	BOOT_COMPILE
		if (slen == 3 &&		/* has comment string after #line x "init.c" */
		    iC_genName &&
		    strcmp(tempNM, iC_genName) == 0) {	/* in the system file "init.c" */
		    int	i;

		    genCount++;			/* count C functions in pre-compile code */
		    for (i = 1; i <= iC_genCount; i++) {
			/********************************************************
			 * System C CODE
			 * For function blocks with more than 1 C function, the
			 * same test will be repeated. All C functions for the
			 * function block will be listed if the function block is
			 * in use on the basis of the first entry for the line.
			 ********************************************************/
			if (temp1 == iC_genLineNums[i] &&	/* find i for this line */
			    functionUse[i].c_cnt) {		/* is function block in use? */
			    goto listCfunction;			/* yes */
			}
		    }
		    continue;			/* no listing for system C CODE not in use */
		} else if (genCount) {
		    assert(genCount == iC_genCount);	/* correct in init.c */
		    genCount = 0;
		}
	      listCfunction:
%endif	/* BOOT_COMPILE */
		strncpy(inpNM, tempNM, BUFS);	/* defer changing name until used */
		if (iC_debug & 06) {
		    if (lexflag & C_FIRST) {
			fprintf(iC_outFP, "******* C CODE          ************************\n");
		    }
		    if (strcmp(inpNM, prevNM) != 0) {
			lexflag &= ~C_BLOCK;	/* output #line for changed filename */
		    } else {
			fprintf(iC_outFP, "\n");	/* separate blocks in lex listing */
		    }
		}
		lexflag |= C_LINE|C_LINE1;	/* only in C-compile */
		strncpy(prevNM, inpNM, BUFS);
	    }
	}
	if ((iC_debug & 04) &&
	    (lexflag & C_BLOCK1) == 0 &&
	    sscanf(chbuf[x], cexeString[outFlag], &temp1) == 1) {
	    cFn = temp1;
	    assert(cFn < functionUseSize);
	}
	if ((iC_debug & 046) &&
	    ((lexflag & (C_BLOCK|C_BLOCK1)) == 0	/* iC listing */
#if YYDEBUG
						 || ((iC_debug & 0402) == 0402)
#endif
									    )) {
	    /********************************************************
	     *  output source listing line in debugging output
	     *  before any tokens are handed to the parser
	     *  then messages appear neatly after the listing line
	     ********************************************************/
	    if ((lexflag & C_LINE) == 0) {
		if (cFn && (iC_debug & 0402) != 0402) {
		    if (cFn > iC_genCount || functionUse[cFn].c_cnt) {
			fprintf(iC_outFP, "%03d\t(%d) %s", lineno, cFn, chbuf[x]); /* C function head listing */
		    }
		    cFn = 0;
		} else if (((iC_debug & 06)  && (lexflag & C_PARSE)) ||
		    ((iC_debug & 040)  && (lexflag & C_PARSE) == 0)) {
		    if (memcmp(chbuf[x], "//* ", 4) != 0) {
			fprintf(iC_outFP, "%03d\t%s", lineno, chbuf[x]);	/* iC listing or C listing with space */
		    } else {
			fprintf(iC_outFP, "\t%s", chbuf[x]);			/* immac Warning or Error */
			--lineno;						/* does not count as a listing line */
		    }
		}
		iFlag = 1;				/* may need correction by pplstfix */
	    } else if ((lexflag & (C_PARSE|C_FIRST)) != C_FIRST) {
		fprintf(iC_outFP, "\t%s", chbuf[x]);	/* # line or # 123 "name" */
	    }
	    if (lineflag == 0) putc('\n', iC_outFP);	/* current line not complete */
	}
	if ((lexflag & C_PARSE) == 0 && lineno > 0) {
	    lexflag &= ~C_FIRST;
	} else if (lexflag & C_LINE1) {
	    lexflag &= ~(C_FIRST|C_BLOCK|C_LINE1);	/* output source listing for lex */
	}
	lexflag &= ~C_LINE;
    }
    /****************************************************************
     *  return 1 character at a time from chbuf[x] and returns it
     *  transfer it to the token buffer iCtext and count iCleng if not lex
     ****************************************************************/
    if ((lexflag & C_PARSE) == 0) {
	iCtext[iCleng++] = c;			/* iC compile via iCparse() */
	if (iCleng >= sizeof(iCtext)) iCleng--;
	iCtext[iCleng] = '\0';
    }
    return c;
} /* get */

/********************************************************************
 *
 *	Unget for iC grammar only
 *
 *	c	character to push back - can go back to beginning of line
 *		always uses getp[0] as its buffer - used by iC-compile
 *
 *******************************************************************/

static void
unget(int c)
{
    if (c != EOF) {
	if(getp[0] <= chbuf[0]) {
	    execerror("unget: ???", NULL, __FILE__, __LINE__);
	}
	*--getp[0] = c;				/* use always insures 1 free place */
	iCtext[--iCleng] = '\0';
    }
} /* unget */

/********************************************************************
 *
 *	Lexer for iC grammar
 *
 *******************************************************************/

static int
yylex(void)
{
    int		c;
    int		c1;

    if (ccfrag) {
	if (ccfrag == '%') {
	    fprintf(ccFP, "%%{\n");		/* output "%{\n" now */
	}
	fprintf(ccFP, "#line %d \"%s\"\n", lineno, inpNM);
	unget('{');
	if (copyCfrag('{', ccfrag == '%' ? '%' : ';', '}', ccFP) == 0) {
	    ccfrag = 0;
	    return 0;				/* EOF in copy C block or statement */
	}
	iClval.vai.v = c_number << FUN_OFFSET;	/* return case # */
	ccfrag = 0;
	c = CCFRAG;
	goto retfl;
    }
    iCleng = 0;
    while ((c = get(T0FP, 0)) !=  EOF) {
	Symbol *	symp;
	Symbol *	sp;
	List_e *	lp;
	char *		cp;
	char *		ep;
	int		len;
	int		rest;
	char		tempBuf[TSIZE];		/* make long enough for format below */

	if (isspace(c)) {			/* space \f \n \r \t \v */
	    iCleng = 0;
	    continue;				/* ignore white space */
	}
	if (isdigit(c) || c == '\'') {
	    unget(c);				/* must be at least a single 0 */
	    iClval.vai.v = (unsigned int)getNumber(getp[0], &ep, 0); /* decimal octal, hex or character constant */
	    assert (ep <= &chbuf[0][CBUFSZ]);
	    for (cp = getp[0]; cp < ep; cp++) {
		get(T0FP, 0);			/* transfer to iCtext to mark text boundaries */
	    }
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "yylex: '%s' converted to %u\n", iCtext, iClval.vai.v);
		fflush(iC_outFP);
	    }
#endif
	    c = NUMBER;				/* value used in immCarrayHead TODO check */
	    goto retfl;
	}
	if (isalpha(c) || c == '_' || c == '$') {	/* first may not be '@' */
	    unsigned char	wplus = 0;
	    unsigned int	qtoken = 0;
	    unsigned long	qmask = 0;	/* mask to id bit, byte, word or long */
	    unsigned long	iomask = 0;	/* output Q is 0x0f, input I is 0xf0 */
	    unsigned char	typ = UDF;
	    unsigned char	ftyp = UDFA;
	    int			ixd = iC_maxIO;	/* limit for I/O index (-1 no limit) */
	    char		y0[2];
	    char		y1[2];
	    int			yn;
	    int			yt;
	    char		y2[2];

#if YYDEBUG
	    rest = 0;				/* used for =I5 0402 debug output */
#endif
	    /* step forward to first character (in c) after identifier to fill iCtext */
	    while (isalnum(c = get(T0FP, 0)) || c == '_' || c == '$' || c == '@');
	    if ((len = sscanf(iCtext, "%1[IQT]%1[XBWL]%d_%d%1[A-Z_a-z]",
		y0, y1, &yn, &yt, y2)) >= 4) {
		if (len == 4) {			/* 4 - QX%d_%d is not allowed */
		    ierror("I/O like names with _ instead of . clash with internal representation", iCtext);
		}				/* 5 - QX%d_%dA is OK - not I/O */
	    } else if (sscanf(iCtext, "%1[IQT]%1[XBWL]%d", y0, y1, &yn) == 3) {
		if (y1[0] == 'B') {
		    wplus = 1;
		    qmask = 0x02;		/* QB or IB */
		    iomask = 0x0f;
		    goto foundQIT;
		}
		if (y1[0] == 'W') {
#if INT_MAX == 32767 && ! defined (LONG16)
		  useWord:
#endif
		    wplus = 1;
		    qmask = 0x0404;		/* QW or IW */
		    iomask = 0x0f0f;
		    goto foundQIT;
		}
		if (y1[0] == 'L') {
#if INT_MAX != 32767 || defined (LONG16)
		    wplus = 1;
		    qmask = 0x08080808;		/* QL or IL */
		    iomask = 0x0f0f0f0f;
		    goto foundQIT;
#else
		    warning("32 bit I/O not supported in 16 bit environment:", iCtext);
		    goto useWord;		/* compile for 16 bit word */
#endif
		} else {			/* QX, IX or TX */
		    /********************************************************************
		     *  the Intel C/C++ v8.1 pre-compiler expands IX0.1 to IX0 .1
		     *  Generally if yacc were to handle the '.' as a separate token,
		     *  white space would be allowed as it would in C for the operator '.'
		     *  If we allow white space here we will have the same effect without
		     *  changing the grammar.
		     *******************************************************************/
		    len = iCleng;
		    while (isspace(c)) {		/* space \f \n \r \t \v */
			c = get(T0FP, 0);		/* iCleng++ if ! lexflag & C_PARSE */
		    }
		    if (c == '.') {			/* [IQT]Xn. ? */
			if (len < iCleng) {
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				fprintf(iC_outFP, ">I1:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				rest++;
			    }
#endif
			    iCtext[len-1] = c;		/* move '.' back over spaces */
			    iCtext[len] = '\0';
			    iCleng = len;		/* in case more than 1 space */
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				fprintf(iC_outFP, "<I1:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				fflush(iC_outFP);
			    }
#endif
			}
			/********************************************************************
			 *  delete space after the '.' although not really required (yet).
			 *******************************************************************/
			len++;				/* get one or more characters */
			while (isspace(c = get(T0FP, 0)));	/* space \f \n \r \t \v */
			if (isdigit(c1 = c)) {		/* can only be [IQT]X%d. */
			    int		i;
			    if (len < iCleng) {
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    fprintf(iC_outFP, ">I2:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				    rest++;
				}
#endif
				iCtext[len-1] = c;	/* move digit back over spaces */
				iCtext[len] = '\0';
				iCleng = len;		/* in case more than 1 space */
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    fprintf(iC_outFP, "<I2:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				    fflush(iC_outFP);
				}
#endif
			    }
			    for (i = 0; isdigit(c = get(T0FP, 0)); i++);
			    if (c1 >= '8' || i > 0) {
				ierror("I/O bit address must be less than 8:", iCtext);
			    } else {
				qmask = 0x01;	/* QX, IX or TX */
				iomask = 0x0f;
			    }
			foundQIT:
			    ftyp = GATE - wplus;	/* GATE or ARITH */
			    if (y0[0] == 'Q') {
				qtoken = lex_act[OUTX - wplus]; /* LOUT or AOUT */
			    } else {
				qtoken = 1;		/* I... */
				typ = INPX - wplus;	/* INPX or INPW */
				if (y0[0] != 'T') {
				    qmask <<= 4;	/* IX, IB, IW or IL */
				    iomask <<= 4;	/* input I */
				} else {
				    qmask = iomask = 0;	/* TX (TB or TW) */
				    ixd = y1[0] == 'X' ? 1 : 0;	/* TX0.7 is max */
				}
			    }
			} else {
			    while (len < iCleng) {	/* '.' not followed by %d - error anyway */
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    fprintf(iC_outFP, ">I3:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				    rest++;
				}
#endif
				c = iCtext[iCleng-1];	/* char after space and spaces */
				unget(c);		/* unget char, does --iCleng */
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    fprintf(iC_outFP, "<I3:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				    fflush(iC_outFP);
				}
#endif
			    }				/* finish up with first space */
			    unget(c);			/* unget it as well or char after '.' */
			    c = iCtext[iCleng-1];	/* restore correct c to unget below */
			}
		    } else if (len < iCleng) {		/* not [IQT]X%d. */
			while (len < iCleng) {
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				fprintf(iC_outFP, ">I4:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				rest++;
			    }
#endif
			    c = iCtext[iCleng-1];	/* char after space while space */
			    unget(c);			/* unget char, does --iCleng */
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				fprintf(iC_outFP, "<I4:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				fflush(iC_outFP);
			    }
#endif
			}				/* leave first space */
			c = iCtext[iCleng-1];		/* restore correct c to unget below */
		    }
		}
		if (ixd >= 0) {			/* no size limit for -ve ixd */
		    if (yn >= ixd) {
			snprintf(tempBuf, TSIZE, "I/O byte address must be less than %d:",
			    ixd);
			ierror(tempBuf, iCtext);/* hard error if outside range */
		    } else			/* no byte word long boundaries */
		    if (ixd <= IXD) {		/* test rest - old style tests (yn in array bounds) */
			if (wplus && y1[0] == 'W' && (yn & 0x01) != 0) {
			    ierror("WORD I/O must have even byte address:", iCtext);
			} else			/* test rest if yn < ixd (array bounds) */
			if (wplus && y1[0] == 'L' && (yn & 0x03) != 0) {
			    ierror("LONG I/O address must be on 4 byte boundary:", iCtext);
			} else			/* and yn is on correct byte boundary */
			if (qmask & 0x88888888) {
			    if (*(unsigned long*)&iC_QX_[yn] & ~qmask & iomask) {
				ierror("I/O addresses more than 1 of bit, byte or word:",
				    iCtext);	/* mixed mode warning */
			    }
			    *(unsigned long*)&iC_QX_[yn] |= qmask; /* note long I/O */
			} else
			if (qmask & 0x4444) {
			    if (*(unsigned short*)&iC_QX_[yn] & ~qmask & iomask) {
				ierror("I/O addresses more than 1 of bit, byte or word:",
				    iCtext);	/* mixed mode warning */
			    }
			    *(unsigned short*)&iC_QX_[yn] |= qmask; /* note word I/O */
			} else
			if (qmask) {
			    if (iC_QX_[yn] & ~qmask & iomask) {
				ierror("I/O addresses more than 1 of bit, byte or word:",
				    iCtext);	/* mixed mode warning */
			    }
			    iC_QX_[yn] |= qmask;	/* note bit or byte I/O */
			}
		    }
		}
	    } else if (sscanf(iCtext, "_%d%1[A-Z_a-z_]", &yn, y2) == 1) {
		ierror("Number preceded by '_' clashes with internal number representation", iCtext);
		typ = ERR;
	    } else if (sscanf(iCtext, "__%d%1[A-Z_a-z_]", &yn, y2) == 1) {
		ierror("Number preceded by '__' clashes with internal negative number representation", iCtext);
		typ = ERR;
	    } else if (!(iC_debug & 04000) && sscanf(iCtext, "iC_%1[A-Za-z_0-9]", y2) == 1) {
		/* this test suppressed for testing by option -d4000 - can cause link errors */
		ierror("Variable starting with iC_ clashes with iC-runtime global variable", iCtext);
		typ = ERR;
	    }
	    unget(c);
#if YYDEBUG
	    if (rest) {					/* set when other >Ix 402 debug output */
		fprintf(iC_outFP, "=I5:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d '%s'\n", lineno, iCtext, c, len, iCleng, chbuf[0]);
		fflush(iC_outFP);
	    }
#endif
	    symp = 0;
	    if (strcmp(iCtext, "__END__") == 0) {
		return 0;			/* early termination */
	    }
	    if (iFunSymExt && ! qtoken) {	/* in function definition and not I/O */
		if (iRetSymbol.v && strcmp(iCtext, "this") == 0) {
		    symp = iRetSymbol.v;	/* function return Symbol */
		} else {
		    char *	cp;
		    if (iC_Pflag && strchr(iCtext, '$')) {
			if (iC_Pflag >= 2) typ = ERR;
			warning("'$' character(s) in function block parameter or identifier:", iCtext);
		    }
		    if ((cp = strchr(iCtext, '@')) != 0) {
			/********************************************************************
			 *  Variable names with '@' are only legal if part of this
			 *  function definition in which case they match iFunBuffer
			 *  and no second '@' occurs - alternate syntax used previously
			 *  with '$' as the function variable separator
			 *******************************************************************/
			cp++;
			if (strchr(cp, '@') ||
			    strncmp(iCtext, iFunBuffer, iFunSymExt - iFunBuffer) != 0) {
			    ierror("'@' only allowed if it follows current function name:", iCtext);
			    return LEXERR;
			}
		    } else {
			cp = iCtext;		/* use the bare function variable */
		    }
		    strncpy(iFunSymExt, cp, iFunEnd - iFunSymExt);
		    if ((symp = lookup(iFunBuffer)) == 0 && iFunSyText) {
			symp = install(iFunBuffer, typ, ftyp); /* parameter or declaration */
		    }
		    *iFunSymExt = '\0';
		}
	    } else if (strchr(iCtext, '@')) {
		ierror("'@' only allowed if it follows function name in a function definition:", iCtext);
		return LEXERR;
	    } else if (iC_Pflag && strchr(iCtext, '$')) {
		/********************************************************************
		 *  This and the test just above, are the only places, where variables
		 *  containing the '$' symbol may be marked erroneous in iC code.
		 *  The internal C compiler, gcc and iClive can handle variables
		 *  containg a '$'
		 *  Excerpt from 'info gcc':
		 *  =====================================
		 *  5.29 Dollar Signs in Identifier Names
		 *  =====================================
		 *  In GNU C, you may normally use dollar signs in identifier names.  This
		 *  is because many traditional C implementations allow such identifiers.
		 *  However, dollar signs in identifiers are not supported on a few target
		 *  machines, typically because the target assembler does not allow them.
		 *
		 *  The internal C compiler warns about variables in C expressions starting
		 *  with a '$' symbol, which may lead to assembler errors. (see lexc.l)
		 *******************************************************************/
		if (iC_Pflag >= 2) typ = ERR;
		warning("'$' character(s) in identifier:", iCtext);
	    } else if (strpbrk(iCtext, "EHLST") == iCtext) {	/* speeds up search */
		/********************************************************************
		 *  The iC language defines internal input variables TX0.0 - TX0.7
		 *  The first 3 have special functionality and the remaining 5 are timers.
		 *  The following table defines aliases for these internal inputs.
		 *	static char *	TaliasList[9][2] = {
		 *	    { "EOI",	"TX0.0"  },	// end of initialisation
		 *	    { "STDIN",	"TX0.1"  },	// stdin line received
		 *	    { "LO",	"TX0.2"  },	// constant bit LO
		 *	    { "HI",	"~TX0.2" },	// constant bit HI
		 *	    { "T10ms",	"TX0.3"  },	// 10 ms internal timer
		 *	    { "T100ms",	"TX0.4"  },	// 100 ms internal timer
		 *	    { "T1sec",	"TX0.5"  },	// 1 second internal timer
		 *	    { "T10sec",	"TX0.6"  },	// 10 second internal timer
		 *	    { "T1min",	"TX0.7"  },	// 1 minute internal timer
		 *	};
		 *******************************************************************/
		int	i;
		int	invFlag = 0;
		char *	TXp;
		for (i = 0; i < 9; i++) {
		    if (strcmp(iCtext, TaliasList[i][0]) == 0) {
			if ((symp = lookup(iCtext)) == 0) {	/* lookup/install ALIAS */
			    symp = install(iCtext, ALIAS, GATE);
			    if (*(TXp = TaliasList[i][1]) == '~') {
				TXp++;
				invFlag = 1;
			    }
			    if ((sp = lookup(TXp)) == 0) {	/* lookup/install TX0.x */
				sp = install(TXp, INPX, GATE);
			    }
			    symp->list = lp = sy_push(sp);	/* complete ALIAS to TX0.x */
			    if (invFlag) {
				lp->le_val = NOT;		/* HI is inverting ALIAS of TX0.2 */
			    }
			}
			break;
		    }
		}
	    }
	    if (symp == 0) {
		/********************************************************************
		 *  Lookup or install all other names as undefined Gates
		 *******************************************************************/
		if ((symp = lookup(iCtext)) == 0) {
		    symp = install(iCtext, typ, ftyp); /* usually UDF UDFA */
		} else if (typ == ERR) {
		    symp->type = ERR;			/* mark ERROR in QX%d_%d */
		}
	    }
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "yylex: %s %s %s\n",
		    symp->name, iC_full_type[symp->type], iC_full_ftype[symp->ftype]);
		fflush(iC_outFP);
	    }
#endif
	    iClval.sym.v = symp;			/* return actual symbol */
	    /********************************************************************
	     *  resolve all ALIASes
	     *******************************************************************/
	    while ( (typ = symp->type) == ALIAS &&
		    (lp = symp->list) != 0 &&
		    (sp = lp->le_sym) != 0) {
		symp = sp;				/* with token of original */
	    }
	    /********************************************************************
	     *  no need to clear sp->em	- check - this refers to very old code Jw 20120515
	     *  EM was cleared as typ &= ~EM before - was not cleared in symp->type
	     *******************************************************************/
	    if (typ > IFUNCT || (symp->fm & FM) != 0) {	/* function definition symbols */
		c = LEXERR;			/* are not looked up directly */
	    } else
	    if (typ == IFUNCT) {
		switch (symp->ftype) {
		case UDFA:
		    c = VFUNCTION;
		    break;
		case ARITH:
		case GATE:
		case GATEX:
		    c = IFUNCTION;
		    break;
		case CLCKL:
		    c = CFUNCTION;
		    break;
		case TIMRL:
		    c = TFUNCTION;
		    break;
		default:
		    c = LEXERR;
		    break;
		}
	    } else
	    if (typ >= KEYW) {
		c = symp->u_val;		/* reserved word or C-type */
	    } else
	    if (qtoken > 1) {			/* ignore I... */
		c = qtoken;			/* first time LOUT or AOUT */
		symp->em |= EO;			/* marks variable as LOUT AOUT Q... */
	    } else
	    if ((c = lex_typ[typ]) == 0) {	/* 0 AVARC 0 0 LVARC 0 ... NUMBER ... */
		c = lex_act[symp->ftype];	/* UNDEF AVAR LVAR ..YYERRCODE.. AOUT LOUT CVAR TVAR */
	    } else if ((typ == ARNC || typ == LOGC) && symp->ftype == UDFA) {
		c = IMMCARRAY;			/* UDFA ARNC and UDFA LOGC are immC arrays not UNDEF */
	    } else if (typ == NCONST) {		/* c === NUMBER from lex_typ[NCONST] */
		iClval.vai.v = 0;		/* return vai although not needed - clean extended listing */
	    }
	    if (symp->ftype == OUTW && (lp = symp->list) != 0) {
		symp = lp->le_sym;		/* original via backptr */
	    }
	    iClval.sym.f = iCstmtp;		/* original name for expressions */
	    					/* CHECK if iCbuf changes now _() is missing */
	    if ((len = snprintf(iCstmtp, rest = &iCbuf[IMMBUFSIZE] - iCstmtp,
				"%s", symp->name)) < 0 || len >= rest) {
		iCbuf[IMMBUFSIZE-1] = '\0';
		len = rest - 1;			/* text + NUL which fits */
		ierror("statement too long at: ", symp->name);
	    }
	    iClval.sym.l = iCstmtp += len;
	} else {				/* not digit alpha '_' or '$' */
	    c1 = get(T0FP, 0);			/* unget possible EOF after c has been processed */
	    switch (c) {
	    case '!':
		if (c1 == '=') {
		    c = EQU; goto found;	/* != */
		}
	    case '~':
		break;				/* ! or ~ */
	    case '%':
		if (c1 == '{') {		/*    >>> '}' */
		    c = LHEAD; goto found;	/* %{ >>> %} */
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* %= */
		}
		c = DIV;			/* % */
		break;
	    case '&':
		if (c1 == '&') {
		    c = B_AND; goto found;	/* && */
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* &= */
		}
		break;
	    case '*':
		if (c1 == '/') {
		    c = COMMENTEND;		/* dangling end of a comment */
		    goto found;
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* *= */
		}
		break;
	    case '+':
		if (c1 == '+') {
		    c = PPMM; goto found;	/* ++ */
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* += */
		}
		break;				/* + binary or unary */
	    case '-':
		if (c1 == '-') {
		    c = PPMM; goto found;	/* -- */
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* -= */
		} else if (c1 == '>') {
		    c = PR; goto found;		/* -> */
		}
		break;				/* - binary or unary */
	    case '/':
		if (c1 == '/') {
		    do {			/* start C++ style comment */
			if ((c1 = get(T0FP, 0)) == EOF) return 0;
		    } while (c1 != '\n');
		} else if (c1 == '*') {
		    do {			/* start C style comment */
			while (c1 != '*') {
			    if ((c1 = get(T0FP, 0)) == EOF) return 0;
			}
		    } while ((c1 = get(T0FP, 0)) != '/');
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* /= */
		} else {
		    c = DIV;			/* / */
		    break;
		}
		iCleng = 0;			/* end of comment */
		continue;
	    case '<':
		if (c1 == '<') {
		    if ((c1 = get(T0FP, 0)) == '=') {
			c = OPE; goto found;	/* <<= */
		    }
		    c = LEFT_SH;		/* << */
		    break;
		}
		if (c1 != '=') {
		    unget(c1);
		}
		c = CMP; goto found;		/* < or <= */
	    case '=':
		if (c1 == '=') {
		    c = EQU; goto found;	/* == */
		}
		*iCtext = '#';			/* to recognise <= => transmogrify = */
		break;
	    case '>':
		if (c1 == '>') {
		    if ((c1 = get(T0FP, 0)) == '=') {
			c = OPE; goto found;	/* >>= */
		    }
		    c = RIGHT_SH;		/* >> */
		    break;
		}
		if (c1 != '=') {
		    unget(c1);
		}
		c = CMP; goto found;		/* > or >= */
	    case '^':
		if (c1 == '=') {
		    c = OPE; goto found;	/* ^= */
		}
		break;
	    case '|':
		if (c1 == '|') {
		    c = B_OR; goto found;	/* || */
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* |= */
		}
		break;
	    case '.':
		c = PR;				/* . */
		break;
	    case '\x1f':
		c = BE;				/* BE */
		break;
	    }
	    unget(c1);				/* c1 was not used (including EOF) */
	found:
	    iClval.vai.v = c;			/* return to yacc as is */
	retfl:
	    iClval.vai.f = iCstmtp;		/* all sources are iClval.vai */
	    if ((c == '+' || c == '-' || c == PPMM) &&
		iCstmtp > iCbuf && *(iCstmtp-1) == *iCtext) {
		*iCstmtp++ = ' ';		/* space between + + and - - */
						/* 1 byte will always fit */
	    }
	    rest = &iCbuf[IMMBUFSIZE] - iCstmtp;
	    len = strlen(iCtext);
	    if (len >= rest) {
		iCbuf[IMMBUFSIZE-1] = '\0';
		len = rest -1;
		ierror("statement too long at: ", iCtext);
	    }
	    iClval.vai.l = iCstmtp = strncpy(iCstmtp, iCtext, len) + len;
	}
	return c;				/* return token to yacc */
    }
    return 0;					/* EOF */
} /* yylex */

/********************************************************************
 *
 *	Output an error in source listing and error output
 *	The error file errFP is opened with the first error
 *
 *******************************************************************/

static void
errLine(void)					/* error file not openend if no errors */
{
    if (errFlag == 0) {
	if (errFilename && iC_errFP == stderr) {
	    if ((iC_errFP = fopen(errFilename, "w+")) == NULL) {
		errFilename = 0;		/* cannot open errFilename */
		iC_errFP = stderr;		/* just output to stderr */
		errRet = Eindex;		/* error return for compile() */
	    } else {
		errFlag = 1;			/* there is an error file */
	    }
	}
	if (iC_outFP != stdout) {
	    errFlag = 1;			/* errors to errFilename or stderr */
	    if (!(iC_debug & 046)) {		/* no source listing in debugging output */
		fprintf(iC_outFP, "******* %-15s ************************\n", inpNM);
	    }
	}
	if (errFlag) fprintf(iC_errFP, "******* %-15s ************************\n", inpNM);
    }
    if (lineno != errline) {
	errline = lineno;			/* dont print line twice */
	if (!(iC_debug & 040) || (lexflag & C_BLOCK)) {	/* no source listing in debugging output */
	    fprintf(iC_outFP, "%03d\t%s", lineno, chbuf[lexflag&C_PARSE]);
	    if (lineflag == 0) putc('\n', iC_outFP);	/* current line not complete */
	}
	if (errFlag) {
	    fprintf(iC_errFP, "%03d\t%s", lineno, chbuf[lexflag&C_PARSE]);
	    if (lineflag == 0)  putc('\n', iC_errFP);	/* current line not complete */
	}
    }
} /* errLine */

/********************************************************************
 *
 *	Common code for error messages
 *
 *******************************************************************/

void
errmess(					/* actual error message */
    char *	str0,
    char *	str1,
    char *	str2)
{
    errLine();					/* do not change - used as search key in iClive */
    fprintf(iC_outFP, "*** %s: %s", str0, str1);
    if (errFlag) fprintf(iC_errFP, "*** %s: %s", str0, str1);
    if (str2) {
	fprintf(iC_outFP, " %s", str2);
	if (errFlag) fprintf(iC_errFP, " %s", str2);
    } else {
	putc('.', iC_outFP);
	if (errFlag) putc('.', iC_errFP);
    }
#ifdef NEWSTYLE
    fprintf(iC_outFP, " '%s'\n", inpNM);
    if (errFlag) fprintf(iC_errFP, " '%s'\n", inpNM);
#else
    fprintf(iC_outFP, " File %s, line %d\n", inpNM, lineno);
    if (errFlag) fprintf(iC_errFP, " File %s, line %d\n", inpNM, lineno);
#endif
    fflush(iC_outFP);
    if (errFlag) fflush(iC_errFP);
} /* errmess */

/********************************************************************
 *
 *	Error message
 *	count in gnerrs to prevent execution of generated code
 *
 *******************************************************************/

void
ierror(						/* print error message */
    char *	str1,
    char *	str2)
{
    gnerrs++;					/* count mostly generate errors */
    errmess("Error", str1, str2);
    if (++iC_iErrCount >= iC_maxErrCount) {
	errmess("Error", "too many errors - compilation aborted", NS);
	if (T0FP) fseek(T0FP, 0L, SEEK_END);	/* flush rest of file */
	longjmp(beginMain, Iindex);
    }
} /* ierror */

/********************************************************************
 *
 *	Warning message
 *
 *******************************************************************/

void
warning(					/* print warning message */
    char *	str1,
    char *	str2)
{
    if (iC_Pflag <= 1) {
	errmess("Warning", str1, str2);		/* simple warning */
    } else {
	ierror(str1, str2);			/* convert warning to error */
    }
} /* warning */

/********************************************************************
 *
 *	Common error messages
 *
 *******************************************************************/
%ifdef	BOOT_COMPILE

static void
errBit(void)
{
    ierror("no constant allowed in bit expression", NS);
} /* errBit */
%endif	/* BOOT_COMPILE */

void
errInt(void)
{
    ierror("no imm variable to trigger arithmetic expression", NS);
} /* errInt */

/********************************************************************
 *
 *	Recovery from run-time errors
 *
 *******************************************************************/

void
execerror(					/* recover from run-time error */
    char *	str1,
    char *	str2,
    char *	file,
    int		line)
{
    errmess("Execerror", str1, str2);
    fprintf(iC_outFP, "in source file: %s line %d\n", file, line);
    fflush(iC_outFP);
    if (errFlag) {
	fprintf(iC_errFP, "in source file: %s line %d\n", file, line);
	fflush(iC_errFP);
    }
    fseek(T0FP, 0L, SEEK_END);			/* flush rest of file */
    longjmp(begin, 0);
} /* execerror */

/********************************************************************
 *
 *	Called from yacc parser on error (mostly syntax error)
 *
 *******************************************************************/

extern int	yyleng;			/* declare in comp.y to make it independent of lex.yy.c */

void
yyerror(const char *	s)
{
    char *	cp = chbuf[lexflag&C_PARSE];
    int		n, n1;
    static char * lc[2] = { "", "C " };	/* this used to be done in gram.pl - jw 20161106 */
    char	erbuf[TSIZE];

    errLine();
    fprintf(iC_outFP, "*** ");	/* do not change - used as search key in pplstfix and iClive */
    if (errFlag) fprintf(iC_errFP, "*** ");
    if ((lexflag & C_PARSE) == 0) {
	n = getp[0] - chbuf[0] - iCleng;
    } else {
	n = column - yyleng;
    }
    for (n1 = 0; n > 0; n--, cp++) {
	n1++;
	if (*cp == '\t') {
	    while (n1 % 8) {
		n1++;
	    }
	}
    }
    ynerrs++;			/* count mostly syntax errors */
#ifdef NEWSTYLE
    n = snprintf(erbuf, TSIZE, "%s%s '%s'", lc[lexflag&C_PARSE], s, inpNM);
#else
    n = snprintf(erbuf, TSIZE, "%s%s %d in %s", lc[lexflag&C_PARSE], s, ynerrs, inpNM);
#endif
    if (n1 < n + 5) {
	n1 += 4;
	while (n1--) {
	    putc('-', iC_outFP);
	    if (errFlag) putc('-', iC_errFP);
	}
	fprintf(iC_outFP, "^ %s\n", erbuf);
	if (errFlag) fprintf(iC_errFP, "^ %s\n", erbuf);
    } else {
	fprintf(iC_outFP, "%s ", erbuf);
	if (errFlag) fprintf(iC_errFP, "%s ", erbuf);
	n1 -= n - 3;
	while (n1--) {
	    putc('-', iC_outFP);
	    if (errFlag) putc('-', iC_errFP);
	}
	fprintf(iC_outFP, "^\n");
	if (errFlag) fprintf(iC_errFP, "^\n");
    }
} /* yyerror */

/********************************************************************
 *
 *	Copy a C fragment to T1FP for an lBlock and T4FP for a cBlock
 *	when yacc token CCFRAG is recognised in yylex()
 *
 *******************************************************************/

static int
copyCfrag(char s, char m, char e, FILE * oFP)
	/* copy C action to the next ; , or closing brace or bracket */
{
    int		brace;
    int		c;
    int		c1;
    int		match;

    for (brace = 0; (c = get(T0FP, 0)) != EOF; ) {
	if (c == s) {				/* '{' or '(' */
	    if (brace++ == 0 && m == '%') {	/* don't output "%{\w" */
		while ((c = get(T0FP, 0)) == ' ' || c == '\t');
		unget(c);
		continue;
	    }
	} else if (c == m) {			/* ';' or '%' or ',' */
	    if (brace <= 0) {
		unget(c);			/* use for next token */
		return m;			/* no longer used */
	    } else if (brace == 1 && c == '%') {
		if ((c = get(T0FP, 0)) == '}') {
		    fprintf(oFP, "\n%%##\n\n%%}\n");	/* {#line lineno "outNM"\n%} */
		    unget(c);
		    return m;
		}
		unget(c);
		c = '%';
	    }
	} else if (c == e) {			/* ')' or '}' */
	    if (--brace <= 0) {
		/* TD fix lineno and name */
		if (brace == 0 && c == '}') {
		    putc(c, oFP);		/* output '}' */
		}
		fprintf(oFP, "\n%%##\n");	/* #line lineno "outNM" */
		unget(c);			/* should not return without '}' */
		return e;
	    }
	} else {
	    switch (c) {
	    case '/':				/* look for comments */
		if ((c1 = get(T0FP, 0)) == '/') {
		    putc(c, oFP);
		    do {			/* start C++ style comment */
			putc(c1, oFP);
			if ((c1 = get(T0FP, 0)) == EOF) goto eof_error;
		    } while (c1 != '\n');
		    c = c1;
		} else if (c1 == '*') {
		    putc(c, oFP);
		    do {			/* start C style comment */
			putc(c1, oFP);
			while (c1 != '*') {
			    if ((c1 = get(T0FP, 0)) == EOF) goto eof_error;
			    putc(c1, oFP);
			}
		    } while ((c1 = get(T0FP, 0)) != '/');
		    c = c1;
		} else {
		    unget(c1);
		}
		break;

	    case '\'':				/* character constant */
		match = '\'';
		goto string;

	    case '"':				/* character string */
		match = '"';

	    string:
		putc(c, oFP);
		while ((c = get(T0FP, 0)) != match) {
		    if (c == '\\') {
			putc(c, oFP);
			if ((c = get(T0FP, 0)) == EOF) goto eof_error;
		    } else if (c == '\n') {
			iCleng = 1;		/* error pointer at newline */
			yyerror("C code: newline in \" \" or ' ', error");
		    } else if (c == EOF)  goto eof_error;
		    putc(c, oFP);
		}
		break;
	    }
	}
	putc(c, oFP);				/* output to T1FN or T4FN */
    }
eof_error:
    iCleng = 1;					/* error pointer at EOF */
    yyerror("C code: EOF, error");
    return 0;					/* EOF */
} /* copyCfrag */

/********************************************************************
 *
 *	Compile if, if else or switch
 *
 *******************************************************************/

static void
ffexprCompile(char * txt, List_e * lp, int cn)
{
    int		saveLexflag;
    int		saveLineno;
    Symbol *	sp;

    saveLexflag = lexflag;			/* iC compile state */
    saveLineno  = lineno;
    assert(lp);
    sp = lp->le_sym;
    assert(sp);
    lp = sp->u_blist;				/* ffexpr head link */
    assert(lp);
    writeCexeTail(T4FP, "    return 0;\n", cn);
    if (iC_c_compile(T4FP, T5FP, C_PARSE|C_BLOCK1, lp) || iC_copyXlate(T5FP, T1FP, 0, 0, 02)) {
	ierror("c_compile failed:", txt);
    }
    lexflag = saveLexflag;			/* restore iC compile state */
    lineno  = saveLineno;
} /* ffexprCompile */

/********************************************************************
 *
 *	If iC_Zflag is set, restore iC_outFP from iC_lstFP, which
 *	enables listing output.
 *	else open iC_nulFP to /dev/null unless it was already opened,
 *	copy it to iC_outFP, which blocks listing output.
 *
 *******************************************************************/

static void
blockUnblockListing(void)
{
    if (iC_Zflag) {
	iC_outFP = iC_lstFP;	/* listing output is now unblocked */
    } else {
	if (iC_nulFP == NULL && (iC_nulFP = fopen("/dev/null", "w")) == NULL) {
	    perror("fopen /dev/null");		/* unlikely to fail */
	    ierror("blocking listing failed:", NS);
	}
	iC_outFP = iC_nulFP;	/* listing output is now blocked */
    }
} /* blockUnblockListing */

/********************************************************************
 *
 *	Adjust logic expression with arithmetic or constant expression
 *
 *******************************************************************/

static List_e *
op_adjust(Lis * lv)
{
    List_e *	lp;
    Symbol *	sp;
    Valp	v;

    if ((lp = lv->v) == NULL) {			/* constant expression ? */
	if ((sp = lookup("TX0.2")) == 0) {	/* yes */
	    sp = install("TX0.2", INPX, GATE);
	}
	lp = sy_push(sp);			/* TX0.2 is permanent bit LO */
	v = evalConstExpr(lv);
	if (v.nuv) {
	    lp->le_val = NOT;			/* ~TX0.2 is permanent bit HI */
	}
    } else {
	lp = op_force(lp, GATE);		/* no - adjust possible arithmetic expression */
    }
    return lp;
} /* op_adjust */

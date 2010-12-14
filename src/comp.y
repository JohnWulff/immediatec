%{ static const char comp_y[] =
"@(#)$Id: comp.y,v 1.103 2010/12/14 07:05:06 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2009  John E. Wulff
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

#include	"icc.h"
#include	"comp.h"

static void	unget(int);		/* shares buffers with get() */
static long	getNumber(void);	/* shares buffers with get() */
static int	iClex(void);
int		ynerrs;			/* count of yyerror() calls */
		/* NOTE iCnerrs is reset for every call to yaccpar() */
static int	copyCfrag(char, char, char, FILE*);	/* copy C action */
static void	ffexprCompile(char *, List_e *, int);	/* c_compile cBlock */
static unsigned char ccfrag;		/* flag for CCFRAG syntax */
static int	cBlockFlag;		/* flag to contol ffexpr termination */
static FILE *	ccFP;			/* FILE * for CCFRAG destination */
static Type	stype;			/* to save TYPE in decl */
static Val	val1 = { 0, 0, 1, };	/* preset off 1 value for TIMER1 */
static Symbol	tSym = { "_tSym_", AND, GATE, };
static Symbol	nSym = { "", ERR, GATE, };
static int	cFn = 0;

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
char *		iCstmtp;		/* manipulated in iClex() (reset in clrBuf()) */
static void	clrBuf(void);
%}

%union {		/* stack type */
    Sym		sym;			/* symbol table pointer */
    Lis		list;			/* linked list elements */
    Val		val;			/* int numerical values */
    Typ		typ;			/* type, ftype, em and fm */
    Str		str;			/* one character array */
    /* allows storing character as iClval.val.v (2nd byte is NULL) */
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
void
pu(int t, char * token, Lis * node)
{
    char *	t_first;
    char *	t_last;
    int		len;

    switch (t) {
    case 0:
	if (node->v) {
	    fprintf(iC_outFP, ">>>Sym	%s	%s :	", token, ((Sym*)node)->v->name);
	} else {
	    fprintf(iC_outFP, ">>>Sym	%s	0 =	", token);
	}
	break;
    case 1:
	if (node->v) {
	    fprintf(iC_outFP, ">>>Lis	%s	%s =	", token, node->v->le_sym ? node->v->le_sym->name : "(0)");
	} else {
	    fprintf(iC_outFP, ">>>Lis	%s	0 =	", token);
	}
	break;
    case 2:
	fprintf(iC_outFP, ">>>Val	%s	%d =	", token, ((Val*)node)->v);
	break;
    case 3:
	fprintf(iC_outFP, ">>>Str	%s	%2.2s =	", token, ((Str*)node)->v);
	break;
    }
    len = (t_last = node->l) - (t_first = node->f);
    if (len > 0 && t_first >= iCbuf && t_first < &iCbuf[IMMBUFSIZE] && t_last >= iCbuf && t_last < &iCbuf[IMMBUFSIZE]) {
	fprintf(iC_outFP, "%-*.*s\n", len, len, t_first);
    } else if (len != 0) {
	fprintf(iC_outFP, "\n*** Error: CODE GENERATION len = %d, f = 0x%lx l = 0x%lx\n", len, (long)t_first, (long)t_last);
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
	iC_os[s1.type],  iC_os[s1.em],  iC_fos[s1.ftype],
	iC_os[s2->type], iC_os[s2->em], iC_fos[s2->ftype],
	iC_os[ss->type], iC_os[ss->em], iC_fos[ss->ftype]);
    fflush(iC_outFP);
} /* pd */

#endif
%}

	/************************************************************
	 *
	 * iC token types
	 *
	 ***********************************************************/

%token	<sym>	UNDEF AVARC AVAR LVARC LVAR AOUT LOUT BLATCH BFORCE DLATCH
%token	<sym>	BLTIN1 BLTIN2 BLTIN3 BLTINX BLTINJ BLTINT CVAR CBLTIN TVAR TBLTIN TBLTI1
%token	<sym>	IF ELSE SWITCH EXTERN ASSIGN RETURN USE USETYPE IMM VOID TYPE
%token	<sym>	IFUNCTION CFUNCTION TFUNCTION VFUNCTION CNAME NUMBER
%token	<val>	CCFRAG
%token	<str>	LEXERR COMMENTEND LHEAD BE
%type	<sym>	program statement funcStatement simpleStatement iFunDef iFunHead
%type	<sym>	returnStatement formalParameter lBlock variable valueVariable outVariable
%type	<sym>	useFlag decl extDecl immDecl asgn dasgn casgn dcasgn tasgn dtasgn
%type	<sym>	iFunTrigger vFunCall vFunCallHead iFunCallHead cFunCallHead tFunCallHead
%type	<sym>	extCfunDecl dVar
%type	<list>	expr aexpr lexpr fexpr cexpr cfexpr texpr actexpr tfexpr ifini ffexpr
%type	<list>	cref ctref ctdref cCall cParams cPlist dPar
%type	<list>	fParams fPlist funcBody iFunCall cFunCall tFunCall rParams rPlist
%type	<val>	cBlock dParams dPlist
%type	<typ>	declHead extDeclHead formalParaTypeDecl extCfunDeclHead
%type	<str>	'{' '[' '(' '"' '\'' ')' ']' '}' /* C/C++ brackets */
%right	<str>	','		/* function seperator */
%right	<str>	'=' OPE
%right	<str>	'?' ':'		/* ? : */
%right	<str>	OO		/* || */
%right	<str>	AA 		/* && */
%right	<str>	'|'		/* logical or */
%right	<str>	'^'		/* logical exclusive or */
%right	<str>	'&'		/* logical and */
%right	<str>	CMP		/* compare operators == != < <= > >= */
%right	<str>	AOP PM '*'	/* arithmetic operators << >> / % + - * */
%nonassoc <str>	'!' '~' PPMM	/* unary operators ! ~ ++ -- (+ - & *) */
%right	<str>	PR 		/* structure operators -> . */
%%

	/************************************************************
	 *
	 * iC grammar
	 *
	 ***********************************************************/

program	: /* nothing */		{ $$.v = 0;  $$.f = $$.l = 0; clrBuf(); }
	| program statement	{ $$   = $2; $$.f = $$.l = 0; clrBuf(); }
	| program error ';'	{ $$.v = 0;  $$.f = $$.l = 0; clrBuf(); iclock->type = ERR; yyerrok; }
	;

statement
	: ';'			{ $$.v = 0;  }		/* empty statement */
	| simpleStatement ';'	{ $$   = $1; }		/* immediate statement */
	| ffexpr BE		{ $$.v = op_asgn(0, &$1, GATE); } /* if or switch */
				    /* op_asgn(0,...) returns 0 for missing slave gate in ffexpr */
	| lBlock		{ $$.v = 0;  }		/* literal block */
	| iFunDef		{ $$   = $1; }		/* immediate function definition */
	;

funcStatement
	: ';'			{ $$.v = 0;  }		/* empty statement */
	| simpleStatement ';'	{ $$   = $1; }		/* immediate statement */
	| ffexpr BE		{ $$.v = op_asgn(0, &$1, GATE);	/* if or switch */
	    collectStatement($$.v); } /* op_asgn(0,...) returns dummy slave gate in ffexpr */
	| returnStatement ';'	{ $$   = $1;		/* function return statement */
	    collectStatement($1.v); }
	;

simpleStatement
	: useFlag		{ $$   = $1; }		/* use flags */
	| extCfunDecl		{ $$   = $1; }		/* external C function declaration */
	| extCfunDecl ','	{ $$   = $1; }		/* external C function declaration - new style */
	| extDecl		{ $$   = $1; }		/* external declaration */
	| extDecl ','		{ $$   = $1; }		/* external declaration - new style */
	| immDecl		{ $$   = $1;		/* immediate declaration */
	    iFunSyText = 0; }
	| immDecl ','		{ $$   = $1;		/* immediate declaration - new style */
	    iFunSyText = 0; }
	| asgn			{ $$   = $1; }		/* immediate value assignment */
	| casgn			{ $$   = $1; }		/* clock assignment */
	| tasgn			{ $$   = $1; }		/* timer assignment */
	| vFunCall		{ $$   = $1; }		/* void function call */
	;

variable
	: valueVariable		{ $$   = $1; }		/* value variable */
	| outVariable		{ $$   = $1; }		/* output variable */
	| CVAR			{ $$   = $1; }		/* clock variable */
	| TVAR			{ $$   = $1; }		/* timer variable */
	| NUMBER		{ $$   = $1; }		/* numeric constant */
	;

valueVariable
	: LVAR			{ $$   = $1; }		/* logical bit variable */
	| LVARC			{ $$   = $1; }		/* logical C bit variable */
	| AVAR			{ $$   = $1; }		/* arithmetic variable */
	| AVARC			{ $$   = $1; }		/* arithmetic C variable */
	;

outVariable
	: LOUT			{ $$   = $1; }		/* output bit variable */
	| AOUT			{ $$   = $1; }		/* output arith. variable */
	;

	/************************************************************
	 *
	 * USE flags postset options for compiling code
	 *
	 *	use alias;		// equivalent to -A option
	 *	no alias;		// turns off -A option
	 *
	 *	use strict;		// equivalent to -S option
	 *	no strict;		// turns off -S option
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
	 * The 'use strict' statement should always be put at the start of
	 * code in future iC programs. Nevertheless very simple iC programs
	 * with mainly bit nodes will still work without having to declare
	 * every variable. The 'no strict' option should be avoided and
	 * is only included for completeness. It should be very easy to
	 * add extra declarations to satisfy the 'strict' criterion.
	 * But one never knows and as a language designer I feel this is
	 * a good way. (grateful acknowledgements to the designers of PERL)
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
	 ***********************************************************/

useFlag	: USE USETYPE		{
		unsigned int usetype, use;
		$$ = $1;
		use = $1.v->ftype;			/* 0=no or 1=use */
		assert(use <= 1);
		usetype = $2.v->ftype;			/* 0=alias 1=strict */
		assert(usetype < MAXUSETYPE);
		if (use) {
		    iC_uses |= 1<<usetype;		/* set iC_Aflag or iC_Sflag */
		} else {
		    iC_uses &= ~(1<<usetype);		/* reset */
		}
	    }
	| useFlag ',' USETYPE	{
		unsigned int usetype, use;
		$$ = $1;
		use = $1.v->ftype;			/* 0=no or 1=use */
		assert(use <= 1);
		usetype = $3.v->ftype;			/* 0=alias 1=strict */
		assert(usetype < MAXUSETYPE);
		if (use) {
		    iC_uses |= 1<<usetype;		/* set iC_Aflag or iC_Sflag */
		} else {
		    iC_uses &= ~(1<<usetype);		/* reset */
		}
	    }
	;

	/********************************************************************
	 *
	 * Extern C function or macro declaration
	 *
	 *	extern int rand();		// function with no parameters
	 *	extern int rand(void);		// alternative syntax
	 *	extern int abs(int);		// function with 1 parameter
	 *	extern int min(int, int);	// macro with 2 parameters
	 *
	 * When 'strict' is active, any C functions or macros, which are called
	 * in immediate expressions must be declared in the iC code. Since it is
	 * easy to mistype the names of iC function blocks and such non-defined
	 * function blocks will be compiled without error as a C function call,
	 * the error is not discovered until link time. With declaration a clean
	 * error message is produced and the extra effort is not great. When a C
	 * function or macro is called in an immediate expression, a check is
	 * made, that the number of parameters is correct. Otherwise an error
	 * message is issued. No check is made for C calls in C fragments
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
	 *	extern int rand			// no ( parameters )
	 *	extern clock rand()		// absolutely wrong return type
	 *	extern timer rand()		// absolutely wrong return type
	 *
	 *******************************************************************/

extCfunDecl
	: extCfunDeclHead UNDEF			{
		if (iC_Sflag) {
		    ierror("strict: erroneous declaration of a C function - no ( parameters ):", $2.v->name);
		} else {
		    warning("erroneous declaration of a C function - no ( parameters ):", $2.v->name);
		}
		if ($2.v->ftype == UDFA) {	/* if not previously declared as imm */
		    uninstall($2.v);		/* delete dummy Symbol */
		}
		$$.v = 0;
	    }
	| extCfunDeclHead UNDEF	'(' dParams ')'	{
		$2.v->type = CWORD;			/* no longer an imm variable */
		$2.v->u_val = CNAME;			/* yacc type */
		$2.v->v_cnt = $4.v;			/* parameter count */
		$$.v = $2.v;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "extCfunDecl: %s has %d parameters\n", $2.v->name, $4.v);
		    fflush(iC_outFP);
		}
#endif
	    }
	| extCfunDeclHead UNDEF '(' dParams error ')'	{
		$2.v->type = CWORD;			/* no longer an imm variable */
		$2.v->u_val = CNAME;			/* yacc type */
		$2.v->v_cnt = $4.v;			/* parameter count */
		$$.v = $2.v;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "extCfunDecl: %s has %d parameters\n", $2.v->name, $4.v);
		    fflush(iC_outFP);
		}
#endif
		iclock->type = ERR; yyerrok;
	    }
	| extCfunDeclHead CNAME			{
		if (iC_Sflag) {
		    ierror("strict: erroneous re-declaration of a C function - no ( parameters ):", $2.v->name);
		} else {
		    warning("erroneous re-declaration of a C function - no ( parameters ):", $2.v->name);
		}
		$$.v = 0;
	    }
	| extCfunDeclHead CNAME '(' dParams ')'	{
		if ($2.v->v_cnt != $4.v) {
		    char	tempBuf[TSIZE];
		    snprintf(tempBuf, TSIZE, "%s %d (%d)", $2.v->name, $4.v, $2.v->v_cnt);
		    if (iC_Sflag) {
			ierror("strict: parameter count does not match a previous C function declaration:", tempBuf);
		    } else {
			warning("parameter count does not match a previous C function declaration - ignore:", tempBuf);
		    }
		    $2.v->v_cnt = $4.v;			/* latest parameter count */
		}
		$$.v = $2.v;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "extCfunDecl: %s has %d parameters\n", $2.v->name, $4.v);
		    fflush(iC_outFP);
		}
#endif
	    }
	| extCfunDeclHead CNAME '(' dParams error ')'	{
		if ($2.v->v_cnt != $4.v) {
		    char	tempBuf[TSIZE];
		    snprintf(tempBuf, TSIZE, "%s %d (%d)", $2.v->name, $4.v, $2.v->v_cnt);
		    if (iC_Sflag) {
			ierror("strict: parameter count does not match a previous C function declaration:", tempBuf);
		    } else {
			warning("parameter count does not match a previous C function declaration - ignore:", tempBuf);
		    }
		    $2.v->v_cnt = $4.v;			/* latest parameter count */
		}
		$$.v = $2.v;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "extCfunDecl: %s has %d parameters\n", $2.v->name, $4.v);
		    fflush(iC_outFP);
		}
#endif
		iclock->type = ERR; yyerrok;
	    }
	;

extCfunDeclHead
	: EXTERN TYPE		{
		if ($2.v->ftype != ARITH) {
		    if (iC_Sflag) {
			ierror("strict: incompatible return type for an extern C function declaration:", $2.v->name);
		    } else {
			warning("incompatible return type for an extern C function declaration:", $2.v->name);
		    }
		}
		stype.ftype = ARITH;
		stype.type  = ARN;
		stype.em    = 1;		/* set em for extern declaration */
		stype.fm    = 0;
		$$.v = stype;
	    }
	| extCfunDecl ','	{ $$.v = stype;	/* first TYPE */ }
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
			warning("parameter type in an extern C function declaration other than int - ignore:", $1.v->name);
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
	| variable		{ $$.v = 0; }	/* since this is a dummy use any word or number */
	;

	/************************************************************
	 *
	 * Extern type declarations - no assignment in the same source
	 *
	 *	extern imm bit   b1;	extern imm int   a1;
	 *	extern imm clock c1;	extern imm timer t1;
	 *
	 * The extern type declaration in iC declares that an immediate
	 * variable has been assigned in another module and may be used
	 * as an rvalue in immediate expressions in this module.
	 *
	 * Because of the single assignment rule, such an extern immediate
	 * variable may normally not be assigned in this module. An exception
	 * is an immediate variable assigned in C statements. If such an
	 * immediate variable is declared extern, a special form must be used:
	 *
	 *	extern immC bit  b2;	extern immC int  a2;
	 *
	 * Variables declared extern with the special type modifier 'immC'
	 * may be used as rvalues and may be assigned in a C statement.
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
	 * The use if 'immC' in a simple type declaration defines the Gate
	 * object, like in C. It must match the previous extern immC type.
	 * In any other context 'immC' is equivalent to 'imm'.
	 *
	 * An extern type declaration of a particular variable may not
	 * occurr after its simple declaration or its assignment or if it
	 * is an input variable, which is implicitly assigned. Since no
	 * real harm is done, a warning is issued and the extern declaration
	 * is ignored.
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
	 * See use of extern type declaration in a function definition
	 * under 'Immediate functions' below.
	 * In particulat iFunSyText is not set for an extDecl and thus the
	 * variables declared extern keep their unchanged global names.
	 *
	 * If an extDecl occurrs in a function definition, a check is made
	 * that the name does not clash with one of the local names in the
	 * function.
	 *
	 ***********************************************************/

extDecl	: extDeclHead UNDEF	{
		$$.v = $2.v;
		$$.v->ftype = $1.v.ftype;
		$$.v->type  = $1.v.type;
		$$.v->em    = $1.v.em;		/* has em set from extDeclHead */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    pd("extDecl", $$.v, $1.v, $2.v);
		}
#endif
	    }
	| extDeclHead variable	{
		char *	cp;
		Symbol *	sp;
		sp = $$.v = $2.v;
		if (sp) {
		    if ($2.v->ftype != $1.v.ftype) {
			ierror("extern declaration does not match previous declaration:", $2.v->name);
			$2.v->type = ERR;	/* cannot execute properly */
		    }
		    if (iFunSymExt && (cp = strchr($2.v->name, '@'))) {
			warning("extern declaration of internal function variable - ignored:", cp+1);
		    } else
		    if ($2.v->type == UDF ||	/* new extern or unused QXx.y QBz */
			($2.v->em && $2.v->type != ERR)) {	/* or prev extern but not ERROR */
			$$.v->ftype = $1.v.ftype;
			$$.v->type  = $1.v.type;
			$$.v->em    = $1.v.em;	/* has em set from extDeclHead */
		    } else
		    if (iFunSymExt) {
			ierror("extern declaration in function definition after assignment:", $2.v->name);
			$$.v->type = ERR;	/* stop use as a statement in function */
			$$.v->em   = 1;
		    } else
		    if ($2.v->type == INPW || $2.v->type == INPX) {
			if (iC_Sflag) {
			    warning("strict: extern declaration of an input variable - ignored:", $2.v->name);
			}
		    } else {
			warning("extern declaration after assignment - ignored:", $2.v->name);
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

extDeclHead
	: EXTERN IMM TYPE	{
		int		typ;
		int		ftyp;
		ftyp = $3.v->ftype;		/* ARITH GATE CLCKL TIMRL */
		if ($2.v->ftype == 1) {		/* ftype field in IMM for immC */
		    if (ftyp == ARITH) {
			typ = ARNC;		/* immC int */
		    } else
		    if (ftyp == GATE) {
			typ = LOGC;		/* immC bit */
		    } else {
			warning("extern declaration of an immC type other than bit or int - ignore:", $3.v->name);
			goto extImmType;
		    }
		} else {			/* IMM is imm */
		  extImmType:
		    if (ftyp >= CLCKL) {	/* check that CLCKL TIMRL */
			ftyp -= CLCKL - CLCK;	/* and CLCK TIMR are adjacent */
		    }
		    typ = iC_types[ftyp];	/* ARN OR CLK TIM */
		}
		stype.ftype = $3.v->ftype;
		stype.type  = typ;
		stype.em    = 1;		/* set em for extern declaration */
		stype.fm    = 0;
		$$.v = stype;
		iFunSyText = 0;			/* no function symbols for extern */
	    }
	| extDecl ','		{
		$$.v = stype;	/* first TYPE */
		iFunSyText = 0;			/* no function symbols for extern */
	    }
	;

	/************************************************************
	 *
	 * Immediate type declaration - may be combined with dasgn
	 *
	 *	imm bit   b1;		imm int   a1;
	 *	imm clock c1;		imm timer t1;
	 *
	 * Multiple declarations of the same variable with the same
	 * immediate type are silently ignored. If correctly declared
	 * after an assignment a warning is still issued, except for
	 * input variables which are never assigned, rather predeclared.
	 *
	 *	immC bit   b1;		immC int   a1;
	 *
	 * The use of 'immC' in a simple type declaration defines the Gate
	 * object, like in C. Such an object may only be used in a C assignment
	 * or as an immediate rvalue. By defining an object, no C assignment
	 * in the current source is necessary. An assignment can occurr in
	 * another source, in which the same variable is declared with an
	 * extern immC type. An assignment in some source should take place
	 * to avoid an algorithmic error. The load module detects and counts
	 * assignments during initialization and warns if no assignments have
	 * occurred.
	 *
	 * immC type declarations may not be combined with a dasgn, since no
	 * immediate assignment may occurr if a variable has been declared
	 * and defined as an ARNC or LOGC type with immC. Such an attempted
	 * assignment is flagged as an error.
	 *
	 * immC type declarations may also not be combined with an immediate
	 * function definition - that is a hard error.
	 *
	 * declHead has type UDF for all TYPEs except ARNC and LOGC, which
	 * are the TYPEs given to completed immC Gate objects.
	 *
	 ***********************************************************/

decl	: declHead UNDEF	{
		Symbol *	sp;
#if YYDEBUG
		Symbol t = *($2.v);
#endif
		$$.f = $1.f; $$.l = $2.l;
		sp = $$.v = $2.v;
		sp->ftype = $1.v.ftype;		/* bit int clock timer */
		sp->type  = $1.v.type;
		sp->em    = $1.v.em;
		if (sp->type != UDF) {
		    listGenOut(sp);		/* list immC node and generate possible output */
		}
		if (iFunSymExt) {
		    char *	cp;
		    Symbol *	sp1;
		    cp = strchr(sp->name, '@'); /* locate original extension */
		    assert(cp && isprint(cp[1])); /* extension must be at least 1 character */
		    if ((sp1 = lookup(++cp)) != 0 && sp1->em) {
			warning("declaration of an extern variable in a function - ignored:", cp);
		    }
		    collectStatement(sp);
		    iFunSyText = 0;		/* no more function symbols */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pd("decl", sp, $1.v, &t);
#endif
	    }
	| declHead variable	{
		int		typ;
		int		ftyp;
		Symbol *	sp;
		$$.f = $1.f; $$.l = $2.l;
		ftyp = $1.v.ftype;		/* TYPE bit int clock timer */
		typ  = $1.v.type;		/* UDF for all TYPEs except ARNC and LOGC */
		sp = $$.v = $2.v;
		if (sp) {
#if YYDEBUG
		    Symbol t = *(sp);
#endif
		    if (sp->ftype != ftyp) {
			ierror("declaration does not match previous declaration:", sp->name);
			if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		    } else
		    if (sp->em || sp->type == UDF) {
			sp->ftype = ftyp;	/* bit int clock timer */
			if (typ != UDF) {	/* UDF for all TYPEs except ARNC and LOGC */
			    char *	name;
			    char	y1[2];
			    int	yn;
			    if ((name = sp->name) &&
				sscanf(name, "Q%1[XBWL]%d", y1, &yn) != 2 &&
				sp->type != typ) {
				ierror("declaration does not match previous imm declaration:", name);
				typ = ERR;	/* cannot execute properly */
			    } else {
				sp->type = typ;	/* UDF for all TYPEs except ARNC and LOGC */
				sp->em = 0;
				listGenOut(sp);	/* list immC node and generate possible output */
			    }
			} else
			if (sp->em && (sp->type == ARNC || sp->type == LOGC)) {
			    ierror("declaration does not match previous immC declaration:", sp->name);
			    typ = ERR;		/* cannot execute properly */
			}
			sp->type = typ;
			sp->em = 0;
		    } else
		    if (sp->type == INPW || sp->type == INPX) {
			if (iC_Sflag) {
			    warning("strict: declaration of an input variable - ignored:", sp->name);
			}
		    } else
		    if (sp->type != ERR){
			warning("declaration after assignment - ignored:", sp->name);
		    }
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) pd("decl", sp, $1.v, &t);
#endif
		} else {
		    ierror("direct declaration of a number is illegal:", $$.f);
		}
		iFunSyText = 0;			/* no more function symbols */
	    }
	;

immDecl	: decl			{		/* immediate declaration */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
	| dasgn			{		/* declaration assignment */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
	| dcasgn		{		/* declaration clock assignment */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
	| dtasgn		{		/* declaration timer assignment */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
	;

declHead
	: IMM TYPE		{
		int		typ;
		int		ftyp;
		$$.f = $$.l = $2.l;		/* do not include in expression string */
		ftyp = $2.v->ftype;		/* ARITH GATE CLCKL TIMRL */
		if ($1.v->ftype == 1) {		/* immC in init.c */
		    if (ftyp == ARITH) {
			typ = ARNC;		/* immC int */
		    } else
		    if (ftyp == GATE) {
			typ = LOGC;		/* immC bit */
		    } else {
			warning("declaration of an immC type other than bit or int - ignore:", $2.v->name);
			goto immType;
		    }
		} else {			/* simple imm */
		  immType:
		    typ = UDF;
		}
		stype.ftype = ftyp;
		stype.type  = typ;
		stype.em    = stype.fm = 0;
		$$.v = stype;
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
	| immDecl ','		{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = stype;			/* first TYPE */
	    }
	;

	/************************************************************
	 *
	 * Assignment combined with a declaration
	 * an output can only be recognised by its name: Q[XBWL]%d
	 *
	 * Report an error if the type is an ARNC or LOGC, defined with immC
	 *
	 ***********************************************************/

dasgn	: decl '=' aexpr	{		/* dasgn is NOT an aexpr */
		char	y1[2];
		int	yn;
		int	ioTyp;
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v) {
		    if ($1.v->type == ARNC || $1.v->type == LOGC) {
			ierror("immC declaration may not be combined with an immediate assignment:", $1.v->name);
			$1.v->type = iFunSymExt ? UDF : ERR;
		    }
		    if (sscanf($1.v->name, "Q%1[XBWL]%d", y1, &yn) == 2) {
			ioTyp = (y1[0] == 'X') ? OUTX : OUTW;
		    } else {
			ioTyp = 0;		/* flags that no I/O is generated */
		    }
		    if (($$.v = assignExpression(&$1, &$3, ioTyp)) == 0) YYERROR;
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) pu(0, "dasgn: decl = aexpr", (Lis*)&$$);
#endif
		} else {
		    ierror("direct assignment to a number is illegal:", $$.f);
		}
	    }
	;

	/************************************************************
	 *
	 * Different forms of assignment
	 *
	 ***********************************************************/

asgn	: UNDEF '=' aexpr	{		/* asgn is an aexpr */
		$$.f = $1.f; $$.l = $3.l;
		$1.v->ftype = GATE;		/* not strict - implicitly declared as 'imm bit' */
		if (($$.v = assignExpression(&$1, &$3, 0)) == 0) YYERROR;
		if (iC_Sflag) {
		    ierror("strict: assignment to an undeclared imm variable:", $1.v->name);
		    $1.v->type = ERR;		/* cannot execute properly */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(0, "asgn: UNDEF", (Lis*)&$$);
#endif
	    }
	| LVAR '=' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = assignExpression(&$1, &$3, 0)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(0, "asgn: LVAR", (Lis*)&$$);
#endif
	    }
	| AVAR '=' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = assignExpression(&$1, &$3, 0)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(0, "asgn: AVAR", (Lis*)&$$);
#endif
	    }
	| LOUT '=' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = assignExpression(&$1, &$3, OUTX)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(0, "asgn: LOUT", (Lis*)&$$);
#endif
	    }
	| AOUT '=' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = assignExpression(&$1, &$3, OUTW)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(0, "asgn: AOUT", (Lis*)&$$);
#endif
	    }
	| NUMBER '=' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		ierror("assignment to a number is illegal:", $$.f);
		if (($$.v = op_asgn(0, &$3, F_ERR)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(0, "asgn: NUMBER (ERR)", (Lis*)&$$);
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
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "aexpr: expr", &$$);
#endif
	    }
	| asgn			{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = sy_push($1.v);
		assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "aexpr: asgn", &$$);
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
		$1.v->ftype = GATE;
		assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: UNDEF", &$$);
#endif
		if (iC_Sflag) {
		    ierror("strict: use of an undeclared imm variable:", $1.v->name);
		    $1.v->type = ERR;		/* cannot execute properly */
		}
	    }
	| NUMBER		{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = 0;			/* no node, do not need value */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: NUMBER", &$$);
#endif
	    }
	| valueVariable		{		/* LVAR LVARC AVAR AVARC */
		$$.f = $1.f; $$.l = $1.l;
		$$.v = checkDecl($1.v);
		assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: valueVariable", &$$);
#endif
	    }
	| outVariable		{		/* LOUT AOUT */
		$$.f = $1.f; $$.l = $1.l;
		$$.v = sy_push($1.v);
		assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: outVariable", &$$);
#endif
	    }
	| fexpr			{
		Symbol *	sp;

		if ($1.v == 0) YYERROR;		/* error in bltin() */
		sp = $1.v->le_sym;
		$$ = $1;
		if (sp->ftype != iC_ftypes[sp->type]) {
		    warning("not enough arguments for function", sp->name);
		}
		sp->ftype = sp->type == SH ? ARITH : GATE;
		assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: fexpr", &$$);
#endif
	    }
	| cCall			{
		$$ = $1;
		if ($$.v) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: cCall", &$$);
#endif
	    }
	| iFunCall			{
		$$ = $1;
		if ($$.v) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: iFunCall", &$$);
#endif
	    }
	| '(' aexpr ')'		{
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = $2.v) != 0) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: (aexpr)", &$$);
#endif
	    }
	/************************************************************
	 * LATCH(aexpr,aexpr)
	 *	LATCH(set,reset)
	 * ######### not required with iC system functions #########
	 * ######### is handled by BFORCE ##########################
	 ***********************************************************/
	| BLATCH '(' lexpr ')'	{		/* LATCH(set,reset) */
		Symbol *	nsp;
		List_e *	nlp;
		$$.f = $1.f; $$.l = $4.l;
		$$.v = op_push(sy_push($3.v->le_sym), LOGC, $3.v);
		$$.v->le_sym->type = LATCH;
		if (iFunSymExt) {
		    nsp = $$.v->le_sym;
		    assert(nsp && nsp->type < IFUNCT);	/* allows IFUNCT to use union v.cnt */
		    nlp =
#if ! YYDEBUG || defined(SYUNION)
			nsp->v_cnt <= 2 ? 0 :
#endif
			    nsp->v_elist;	/* feedback list */
		    nsp->v_elist = sy_push(nsp); /* feeds back to itself */
		    nsp->v_elist->le_next = nlp;
		}
		assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: BLATCH", &$$);
#endif
	    }
	/************************************************************
	 * FORCE(aexpr,aexpr,aexpr)
	 *	FORCE(expr,on,off)
	 ***********************************************************/
	| BFORCE '(' aexpr ',' lexpr ')'	{	/* FORCE(expr,hi,lo) */
		$$.f = $1.f; $$.l = $6.l;
		if ($3.v == 0) { $$.v = 0; errBit(); YYERROR; }
		$$.v = op_push(op_force($3.v, GATE), LOGC, $5.v);
		$$.v->le_sym->type = LATCH;
		assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: BFORCE", &$$);
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
		} else if ($1.v == 0 || $3.v == 0) {
		    errBit(); YYERROR;
		} else {
		    lpR = op_force($3.v, GATE);
		    lpL = op_force($1.v, GATE);
		    $$.v = op_push(lpL, OR, lpR);	/* logical | */
		}
		if ($$.v) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: expr | expr", &$$);
#endif
	    }
	| expr '^' expr		{			/* binary ^ */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		if (($1.v == 0 || $1.v->le_sym->ftype == ARITH) &&
		    ($3.v == 0 || $3.v->le_sym->ftype == ARITH)) {
		    $$.v = op_push($1.v, ARN, $3.v);	/* bitwise ^ */
		} else if ($1.v == 0 || $3.v == 0) {
		    errBit(); YYERROR;
		} else {
		    lpR = op_force($3.v, GATE);
		    lpL = op_force($1.v, GATE);
		    $$.v = op_push(lpL, XOR, lpR);	/* logical ^ */
		}
		if ($$.v) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: expr ^ expr", &$$);
#endif
	    }
	| expr '&' expr		{			/* binary & */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		if (($1.v == 0 || $1.v->le_sym->ftype == ARITH) &&
		    ($3.v == 0 || $3.v->le_sym->ftype == ARITH)) {
		    $$.v = op_push($1.v, ARN, $3.v);	/* bitwise & */
		} else if ($1.v == 0 || $3.v == 0) {
		    errBit(); YYERROR;
		} else {
		    lpR = op_force($3.v, GATE);
		    lpL = op_force($1.v, GATE);
		    $$.v = op_push(lpL, AND, lpR);	/* logical & */
		}
		if ($$.v) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: expr & expr", &$$);
#endif
	    }
	| expr CMP expr	{				/* == != < <= > >= */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		lpR = op_force($3.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    $$.v = op_force($$.v, GATE);	/* default output */
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: expr CMP expr", &$$);
#endif
	    }
	| expr AOP expr		{			/* << >> / % */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		lpR = op_force($3.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: expr AOP expr", &$$);
#endif
	    }
	| expr PM expr		{			/* binary + - */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		lpR = op_force($3.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: expr PM expr", &$$);
#endif
	    }
	| expr '*' expr		{			/* binary * */
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		lpR = op_force($3.v, ARITH);
		lpL = op_force($1.v, ARITH);
		if (($$.v = op_push(lpL, ARN, lpR)) != 0) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: expr * expr", &$$);
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
	| expr AA expr	{				/* binary && */
		Symbol *	sp;
		int		typ;
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v &&
		    (sp = $1.v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type) > ARN || typ == UDF || !sp->u_blist) &&
		    (sp = $3.v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type) > ARN || typ == UDF || !sp->u_blist)) {
		    lpR = op_force($3.v, GATE);
		    lpL = op_force($1.v, GATE);
		    $$.v = op_push(lpL, AND, lpR);	/* logical & */
		    if (iC_Wflag & W_DEPRECATED_LOGIC) {
			warning("Use of '&&' with imm bit variables is deprecated (use '&'):", sp->name);
		    }
		} else {
		    lpR = op_force($3.v, ARITH);
		    lpL = op_force($1.v, ARITH);
		    $$.v = op_push(lpL, ARN, lpR);	/* arithmetic && */
		    $$.v = op_force($$.v, GATE);	/* default GATE output */
		}
		if ($$.v) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: expr AA expr", &$$);
#endif
	    }
	| expr OO expr	{				/* binary || */
		Symbol *	sp;
		int		typ;
		List_e *	lpL;
		List_e *	lpR;
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v &&
		    (sp = $1.v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type) > ARN || typ == UDF || !sp->u_blist) &&
		    (sp = $3.v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type) > ARN || typ == UDF || !sp->u_blist)) {
		    lpR = op_force($3.v, GATE);
		    lpL = op_force($1.v, GATE);
		    $$.v = op_push(lpL, OR, lpR);	/* logical | */
		    if (iC_Wflag & W_DEPRECATED_LOGIC) {
			warning("Use of '||' with imm bit variables is deprecated (use '|'):", sp->name);
		    }
		} else {
		    lpR = op_force($3.v, ARITH);
		    lpL = op_force($1.v, ARITH);
		    $$.v = op_push(lpL, ARN, lpR);	/* arithmetic || */
		    $$.v = op_force($$.v, GATE);	/* default GATE output */
		}
		if ($$.v) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: expr OO expr", &$$);
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
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: expr ? expr : expr", &$$);
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
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (iC_Pflag) {
		    Symbol * sp = $4.v->le_sym;
		    warning("ISO C forbids omitting the middle term of a ?: expression", sp->name);
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: expr ? : expr", &$$);
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
							/* logical negation */
			$$.v = op_not(op_force($2.v, GATE));
		    } else {
							/* bitwise negation */
			$$.v = op_push(0, ARN, op_force($2.v, ARITH));
		    }
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		} else {
		    $$.v = 0;				/* constant negation */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: ~expr", &$$);
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
							/* logical negation */
			$$.v = op_not(op_force($2.v, GATE));
			if (iC_Wflag & W_DEPRECATED_LOGIC) {
			    warning("Use of '!' with an imm bit variable is deprecated (use '~'):", sp->name);
			}
		    } else {
							/* arithmetic negation */
			$$.v = op_push(0, ARN, op_force($2.v, ARITH));
		    }
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		} else {
		    $$.v = 0;				/* constant negation */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: !expr", &$$);
#endif
	    }

	/************************************************************
	 * Unary + or -
	 ***********************************************************/
	| PM expr %prec '!'	{			/* unary + or - */
		$$.f = $1.f; $$.l = $2.l;
		if (($$.v = op_push(0, ARN, op_force($2.v, ARITH))) != 0) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "expr: PM expr", &$$);
#endif
	    }
	;

	/************************************************************
	 *
	 * lexpr - auxiliary grammar rule to support BFORCE and BLATCH
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
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "lexpr: aexpr , aexpr", &$$);
#endif
	    }
	;

	/************************************************************
	 *
	 * Literal block %{ ......... %}
	 *
	 * A literal block may be used anywhere before, between or after
	 * statements in an iC program. The contents of the literal block
	 * will be copied to the generated C file before any expressions
	 * generated by immediate statements.
	 *
	 * The characters %# occurring at the start of a line in a literal
	 * block will be converted to a plain #. This allows the use of
	 * C-preprocessor statements in literal blocks which will be
	 * resolved after the iC compilation. They must be written as
	 * %#include
	 * %#define
	 * %#ifdef etc
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
	 * The characters %# occurring at the start of a line in a C block
	 * will be converted to a plain #. See literal block above.
	 *
	 ***********************************************************/

cBlock	: '{'			{ ccfrag = '{'; ccFP = T4FP; }	/* ccfrag must be set */
	      CCFRAG '}'	{
		$$ = $3;				/* function # is yacc token */
		cBlockFlag = 1;				/* hold up get() till another token found */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) { fprintf(iC_outFP, ">>>cBlock #%d end\n", $3.v); fflush(iC_outFP); }
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
	 * NUMBER, an Alias for a NUMBER or an immediate expression
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
		if ($3.v == 0) {
		    if (const_push(&$3)) { errInt(); YYERROR; }
		}
		$3.v = op_force($3.v, ARITH);
		$3.v->le_val = (unsigned)-1;		/* mark link as timer value */
		$$.v = op_push($1.v, TIM, $3.v);
	    }
	;

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
	 * CHANGE(aexpr[,(cexpr|texpr[,aexpr])])
	 * RISE(aexpr[,(cexpr|texpr[,aexpr])])
	 *	D(..) SH(..) CHANGE(..) RISE(expr)
	 *	D(..) SH(..) CHANGE(..) RISE(expr,clk)
	 *	D(..) SH(..) CHANGE(..) RISE(expr,tim)
	 *	D(..) SH(..) CHANGE(..) RISE(expr,tim,delay)
	 ***********************************************************/
fexpr	: BLTIN1 '(' aexpr cref ')' {			/* D(expr); SH etc */
		$$.f = $1.f; $$.l = $5.l;
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTIN1", &$$);
#endif
	    }
	/************************************************************
	 * DLATCH(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	DLATCH(set,reset)
	 *	DLATCH(set,reset,clkSetReset)
	 *	DLATCH(set,reset,timSetReset)
	 *	DLATCH(set,reset,timSetReset,delaySetReset)
	 * ######### not required with iC system functions #########
	 * ######### is handled by BFORCE ##########################
	 ***********************************************************/
	| DLATCH '(' lexpr cref ')'	{		/* DLATCH(set,reset) */
		Lis		li1 = $3;
		List_e *	lp1;
		$$.f = $1.f; $$.l = $5.l;
		lp1 = op_push(sy_push($3.v->le_sym), LOGC, $3.v);
		lp1->le_sym->type = LATCH;
		lp1 = op_push(sy_push($1.v), LATCH, lp1);
		/* DLATCH output is transferred as feed back in op_asgn */
		$$.v = bltin(&$1, &li1, &$4, 0, 0, 0, 0, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: DLATCH", &$$);
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
	 ***********************************************************/
	| BLTIN2 '(' aexpr ',' aexpr cref ')' {		/* SR(set,reset); DR(expr,reset) */
		$$.f = $1.f; $$.l = $7.l;
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, &$6, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTIN2", &$$);
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
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, &$8, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTIN2", &$$);
#endif
	    }
	/************************************************************
	 * DSR(aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SHSR(expr)
	 *	DSR(..) SHSR(expr,clk)
	 *	DSR(..) SHSR(expr,tim)
	 *	DSR(..) SHSR(expr,tim,delay)
	 ***********************************************************/
	| BLTIN3 '(' aexpr cref ')' {			/* DSR(expr); SHSR(expr) */
		$$.f = $1.f; $$.l = $5.l;
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTIN3", &$$);
#endif
	    }
	/************************************************************
	 * DSR(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SHSR(expr,reset)
	 *	DSR(..) SHSR(expr,reset,clkExprReset)
	 *	DSR(..) SHSR(expr,reset,timExprReset)
	 *	DSR(..) SHSR(expr,reset,timExprReset,delayExprReset)
	 ***********************************************************/
	| BLTIN3 '(' aexpr ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $7.l;
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, &$6, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTIN3", &$$);
#endif
	    }
	/************************************************************
	 * DSR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SHSR(expr,clkExpr,          reset)
	 *	DSR(..) SHSR(expr,clkExpr,          reset,clkReset)
	 *	DSR(..) SHSR(expr,clkExpr,          reset,timReset)
	 *	DSR(..) SHSR(expr,clkExpr,          reset,timReset,delayReset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,reset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,reset,clkReset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,reset,timReset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,reset,timReset,delayReset)
	 ***********************************************************/
	| BLTIN3 '(' aexpr ',' ctdref ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $9.l;
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, &$8, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTIN3", &$$);
#endif
	    }
	/************************************************************
	 * DSR(aexpr,aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr,aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SHSR(expr,set,reset)
	 *	DSR(..) SHSR(expr,set,reset,clkExprReset)
	 *	DSR(..) SHSR(expr,set,reset,timExprReset)
	 *	DSR(..) SHSR(expr,set,reset,timExprReset,delayExprReset)
	 ***********************************************************/
	| BLTIN3 '(' aexpr ',' aexpr ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $9.l;
		$$.v = bltin(&$1, &$3, 0, &$5, 0, &$7, &$8, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTIN3", &$$);
#endif
	    }
	/************************************************************
	 * DSR(aexpr,aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr,aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SHSR(expr,set,clkSet,         reset)
	 *	DSR(..) SHSR(expr,set,timSet,delaySet,reset)
	 *	DSR(..) SHSR(expr,set,clkSet,         reset,clkReset)
	 *	DSR(..) SHSR(expr,set,timSet,delaySet,reset,clkReset)
	 *	DSR(..) SHSR(expr,set,clkSet,         reset,timReset)
	 *	DSR(..) SHSR(expr,set,timSet,delaySet,reset,timReset)
	 *	DSR(..) SHSR(expr,set,clkSet,         reset,timReset,delayReset)
	 *	DSR(..) SHSR(expr,set,timSet,delaySet,reset,timReset,delayReset)
	 * These cases have been left out deliberately, beause it is
	 * not clear wether the default clock for 'expr' should be taken
	 * from the set clock or the reset clock - causes syntax errors.
	 * Therefore the user is forced to chose a clock for 'expr'.
	 * The following code would do the job - has been tested - JW 020630
	 * The clock for 'expr' is cloned from the reset clock.
	| BLTIN3 '(' aexpr ',' aexpr ',' ctdref ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $11.l;
		$$.v = bltin(&$1, &$3, 0, &$5, &$7, &$9, &$10, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTIN3", &$$);
#endif
	    }
	 ***********************************************************/
	/************************************************************
	 * DSR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SHSR(expr,clkExpr,          set,reset)
	 *	DSR(..) SHSR(expr,clkExpr,          set,reset,clkSetReset)
	 *	DSR(..) SHSR(expr,clkExpr,          set,reset,timSetReset)
	 *	DSR(..) SHSR(expr,clkExpr,          set,reset,timSetReset,delaySetReset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,set,reset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,set,reset,clkSetReset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,set,reset,timSetReset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,set,reset,timSetReset,delaySetReset)
	 ***********************************************************/
	| BLTIN3 '(' aexpr ',' ctdref ',' aexpr ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $11.l;
		$$.v = bltin(&$1, &$3, &$5, &$7, 0, &$9, &$10, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTIN3", &$$);
#endif
	    }
	/************************************************************
	 * DSR(aexpr,(cexpr|texpr,aexpr),aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 * SHSR(aexpr,(cexpr|texpr,aexpr),aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	DSR(..) SHSR(expr,clkExpr,          set,clkSet,         reset)
	 *	DSR(..) SHSR(expr,clkExpr,          set,timSet,delaySet,reset)
	 *	DSR(..) SHSR(expr,clkExpr,          set,clkSet,         reset,clkReset)
	 *	DSR(..) SHSR(expr,clkExpr,          set,timSet,delaySet,reset,clkReset)
	 *	DSR(..) SHSR(expr,clkExpr,          set,clkSet,         reset,timReset)
	 *	DSR(..) SHSR(expr,clkExpr,          set,timSet,delaySet,reset,timReset)
	 *	DSR(..) SHSR(expr,clkExpr,          set,clkSet,         reset,timReset,delayReset)
	 *	DSR(..) SHSR(expr,clkExpr,          set,timSet,delaySet,reset,timReset,delayReset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,set,clkSet,         reset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,set,timSet,delaySet,reset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,set,clkSet,         reset,clkReset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,set,timSet,delaySet,reset,clkReset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,set,clkSet,         reset,timReset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,set,timSet,delaySet,reset,timReset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,set,clkSet,         reset,timReset,delayReset)
	 *	DSR(..) SHSR(expr,timExpr,delayExpr,set,timSet,delaySet,reset,timReset,delayReset)
	 ***********************************************************/
	| BLTIN3 '(' aexpr ',' ctdref ',' aexpr ',' ctdref ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $13.l;
		$$.v = bltin(&$1, &$3, &$5, &$7, &$9, &$11, &$12, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTIN3", &$$);
#endif
	    }
	/************************************************************
	 * SRX(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	SRX(set,reset)
	 *	SRX(set,reset,clkSetReset)
	 *	SRX(set,reset,timSetReset)
	 *	SRX(set,reset,timSetReset,delaySetReset)
	 * ######### not required with iC system functions #########
	 * ######### is handled by SR ### ##########################
	 ***********************************************************/
	| BLTINX '(' aexpr ',' aexpr cref ')' {		/* SRX(set,reset) */
		Lis liS, liR, noS, noR, xxS, xxR;
		liS = noS = $3;
		liR = noR = $5;
		$$.f = $1.f; $$.l = $7.l;
		liS.v = op_force(liS.v, GATE);		/* set */
		liR.v = op_force(liR.v, GATE);		/* res */
		noS.v = sy_push(liS.v->le_sym);		/* copy set */
		noS.v->le_val ^= NOT;			/* ~set */
		assert(noS.f == 0 || noS.f >= iCbuf && noS.l < &iCbuf[IMMBUFSIZE]);
		noS.v->le_first = noS.f;
		noS.v->le_last  = noS.l;
		noR.v = sy_push(liR.v->le_sym);		/* copy res */
		noR.v->le_val ^= NOT;			/* ~res */
		assert(noR.f == 0 || noR.f >= iCbuf && noR.l < &iCbuf[IMMBUFSIZE]);
		noR.v->le_first = noR.f;
		noR.v->le_last  = noR.l;
		xxS.v = op_push(liS.v, AND, noR.v);	/* set & ~res */
		assert(liS.f == 0 || liS.f >= iCbuf && noR.l < &iCbuf[IMMBUFSIZE]);
		xxS.f = xxS.v->le_first = liS.f;
		xxS.l = xxS.v->le_last  = noR.l;
		xxR.v = op_push(noS.v, AND, liR.v);	/* ~set & res */
		assert(noS.f == 0 || noS.f >= iCbuf && liR.l < &iCbuf[IMMBUFSIZE]);
		xxR.f = xxR.v->le_first = noS.f;
		xxR.l = xxR.v->le_last  = liR.l;
		$$.v = bltin(&$1, &xxS, 0, 0, 0, &xxR, &$6, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTINX", &$$);
#endif
	    }
	/************************************************************
	 * SRX(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	SRX(set,clkSet,         reset)
	 *	SRX(set,clkSet,         reset,clkReset)
	 *	SRX(set,clkSet,         reset,timReset)
	 *	SRX(set,clkSet,         reset,timReset,delayReset)
	 *	SRX(set,timSet,delaySet,reset)
	 *	SRX(set,timSet,delaySet,reset,clkReset)
	 *	SRX(set,timSet,delaySet,reset,timReset)
	 *	SRX(set,timSet,delaySet,reset,timReset,delayReset)
	 * ######### not required with iC system functions #########
	 * ######### is handled by SR ### ##########################
	 ***********************************************************/
	| BLTINX '(' aexpr ',' ctdref ',' aexpr cref ')' {
		Lis liS, liR, noS, noR, xxS, xxR;
		liS = noS = $3;
		liR = noR = $7;
		$$.f = $1.f; $$.l = $9.l;
		liS.v = op_force(liS.v, GATE);		/* set */
		liR.v = op_force(liR.v, GATE);		/* res */
		noS.v = sy_push(liS.v->le_sym);		/* copy set */
		noS.v->le_val ^= NOT;			/* ~set */
		assert(noS.f == 0 || noS.f >= iCbuf && noS.l < &iCbuf[IMMBUFSIZE]);
		noS.v->le_first = noS.f;
		noS.v->le_last  = noS.l;
		noR.v = sy_push(liR.v->le_sym);		/* copy res */
		noR.v->le_val ^= NOT;			/* ~res */
		assert(noR.f == 0 || noR.f >= iCbuf && noR.l < &iCbuf[IMMBUFSIZE]);
		noR.v->le_first = noR.f;
		noR.v->le_last  = noR.l;
		xxS.v = op_push(liS.v, AND, noR.v);	/* set & ~res */
		assert(liS.f == 0 || liS.f >= iCbuf && noR.l < &iCbuf[IMMBUFSIZE]);
		xxS.f = xxS.v->le_first = liS.f;
		xxS.l = xxS.v->le_last  = noR.l;
		xxR.v = op_push(noS.v, AND, liR.v);	/* ~set & res */
		assert(noS.f == 0 || noS.f >= iCbuf && liR.l < &iCbuf[IMMBUFSIZE]);
		xxR.f = xxR.v->le_first = noS.f;
		xxR.l = xxR.v->le_last  = liR.l;
		$$.v = bltin(&$1, &xxS, &$5, 0, 0, &xxR, &$8, 0, 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTINX", &$$);
#endif
	    }
	/************************************************************
	 * JK(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	JK(set,reset)
	 *	JK(set,reset,clkSetReset)
	 *	JK(set,reset,timSetReset)
	 *	JK(set,reset,timSetReset,delaySetReset)
	 * ######### not required with iC system functions #########
	 * ######### is handled by SR ### ##########################
	 ***********************************************************/
	| BLTINJ '(' aexpr ',' aexpr cref ')' {		/* JK(set,reset) */
		Lis		liS = $3;		/* later = 0 TODO */
		Lis		liR = $5;
		List_e *	lpS;
		List_e *	lpR;
		Symbol *	nsp;
		List_e *	nlp;
		$$.f = $1.f; $$.l = $7.l;
		liS.v = op_not(sy_push(&tSym));		/* temporary Gate */
		if ((liS.v = op_push(liS.v, AND, op_force($3.v, GATE))) != 0) {
		    liS.v->le_first = liS.f = 0; liS.v->le_last = liS.l = 0;
		}
		liR.v = sy_push(&tSym);			/* temporary Gate */
		if ((liR.v = op_push(liR.v, AND, op_force($5.v, GATE))) != 0) {
		    liR.v->le_first = liR.f = 0; liR.v->le_last = liR.l = 0;
		}
		$$.v = bltin(&$1, &liS, 0, 0, 0, &liR, &$6, 0, 0);
		lpS = liS.v->le_sym->u_blist;
		while (lpS && lpS->le_sym != &tSym) {
		    lpS = lpS->le_next;
		}
		assert(lpS);
		lpR = liR.v->le_sym->u_blist;
		while (lpR && lpR->le_sym != &tSym) {
		    lpR = lpR->le_next;
		}
		assert(lpR);
		nsp = lpS->le_sym = lpR->le_sym = $$.v->le_sym;	/* JK feedback links */
		if (iFunSymExt) {
		    assert(nsp && nsp->type < IFUNCT);	/* allows IFUNCT to use union v.cnt */
		    nlp =
#if ! YYDEBUG || defined(SYUNION)
			nsp->v_cnt <= 2 ? 0 :
#endif
			    nsp->v_elist;		/* feedback list */
		    nsp->v_elist = sy_push(liS.v->le_sym);
		    nsp->v_elist->le_next = sy_push(liR.v->le_sym);
		    nsp->v_elist->le_next->le_next = nlp;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTINJ", &$$);
#endif
	    }
	/************************************************************
	 * JK(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	JK(set,clkSet,         reset)
	 *	JK(set,clkSet,         reset,clkReset)
	 *	JK(set,clkSet,         reset,timReset)
	 *	JK(set,clkSet,         reset,timReset,delayReset)
	 *	JK(set,timSet,delaySet,reset)
	 *	JK(set,timSet,delaySet,reset,clkReset)
	 *	JK(set,timSet,delaySet,reset,timReset)
	 *	JK(set,timSet,delaySet,reset,timReset,delayReset)
	 * ######### not required with iC system functions #########
	 * ######### is handled by SR ### ##########################
	 ***********************************************************/
	| BLTINJ '(' aexpr ',' ctdref ',' aexpr cref ')' {
		Lis		liS = $3;		/* later = 0 TODO */
		Lis		liR = $7;
		List_e *	lpS;
		List_e *	lpR;
		Symbol *	nsp;
		List_e *	nlp;
		$$.f = $1.f; $$.l = $9.l;
		liS.v = op_not(sy_push(&tSym));		/* temporary Gate */
		if ((liS.v = op_push(liS.v, AND, op_force($3.v, GATE))) != 0) {
		    liS.v->le_first = liS.f = 0; liS.v->le_last = liS.l = 0;
		}
		liR.v = sy_push(&tSym);			/* temporary Gate */
		if ((liR.v = op_push(liR.v, AND, op_force($7.v, GATE))) != 0) {
		    liR.v->le_first = liR.f = 0; liR.v->le_last = liR.l = 0;
		}
		$$.v = bltin(&$1, &liS, &$5, 0, 0, &liR, &$8, 0, 0);
		lpS = liS.v->le_sym->u_blist;
		while (lpS && lpS->le_sym != &tSym) {
		    lpS = lpS->le_next;
		}
		assert(lpS);
		lpR = liR.v->le_sym->u_blist;
		while (lpR && lpR->le_sym != &tSym) {
		    lpR = lpR->le_next;
		}
		assert(lpR);
		nsp = lpS->le_sym = lpR->le_sym = $$.v->le_sym;	/* JK feedback links */
		if (iFunSymExt) {
		    assert(nsp && nsp->type < IFUNCT);	/* allows IFUNCT to use union v.cnt */
		    nlp =
#if ! YYDEBUG || defined(SYUNION)
			nsp->v_cnt <= 2 ? 0 :
#endif
			    nsp->v_elist;		/* feedback list */
		    nsp->v_elist = sy_push(liS.v->le_sym);
		    nsp->v_elist->le_next = sy_push(liR.v->le_sym);
		    nsp->v_elist->le_next->le_next = nlp;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTINJ", &$$);
#endif
	    }
	/************************************************************
	 * SRT(aexpr[,(cexpr|texpr[,aexpr])])
	 *	SRT(set)
	 *	SRT(set,clk)
	 *	SRT(set,tim)
	 *	SRT(set,tim,delay)
	 ***********************************************************/
	| BLTINT '(' aexpr cref ')'	{		/* SRT(set,reset,tim,delay) */
		$$.f = $1.f; $$.l = $5.l;
		$$.v = bltin(&$1, &$3, 0, 0, 0, 0, 0, &$4, 0); /* monoflop without reset */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTINT", &$$);	/* set clocked by iClock */
#endif
	    }
	/************************************************************
	 * SRT(aexpr,(cexpr|texpr,aexpr),(cexpr|texpr[,aexpr]))
	 *	SRT(set,clkSet,         clk)
	 *	SRT(set,timSet,delaySet,clk)
	 *	SRT(set,clkSet,         tim)
	 *	SRT(set,timSet,delaySet,tim)
	 *	SRT(set,clkSet,         tim,delay)
	 *	SRT(set,timSet,delaySet,tim,delay)
	 ***********************************************************/
	| BLTINT '(' aexpr ',' ctdref ',' ctref ')'	{
		$$.f = $1.f; $$.l = $8.l;
		$$.v = bltin(&$1, &$3, &$5, 0, 0, 0, 0, &$7, 0); /* monoflop without reset */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTINT", &$$);	/* set clocked by ext clock or timer */
#endif
	    }
	/************************************************************
	 * SRT(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	SRT(set,reset)
	 *	SRT(set,reset,clk)
	 *	SRT(set,reset,tim)
	 *	SRT(set,reset,tim,delay)
	 ***********************************************************/
	| BLTINT '(' aexpr ',' aexpr cref ')'	{
		$$.f = $1.f; $$.l = $7.l;
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, 0, &$6, 0); /* monoflop with reset */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTINT", &$$);	/* set and reset clocked by iClock */
#endif
	    }
	/************************************************************
	 * SRT(aexpr,aexpr,(cexpr|texpr,aexpr),(cexpr|texpr[,aexpr]))
	 *	SRT(set,reset,clkSetReset,              clk)
	 *	SRT(set,reset,timSetReset,delaySetReset,clk)
	 *	SRT(set,reset,clkSetReset,              tim)
	 *	SRT(set,reset,timSetReset,delaySetReset,tim)
	 *	SRT(set,reset,clkSetReset,              tim,delay)
	 *	SRT(set,reset,timSetReset,delaySetReset,tim,delay)
	 ***********************************************************/
	| BLTINT '(' aexpr ',' aexpr ',' ctdref ',' ctref ')'	{
		$$.f = $1.f; $$.l = $10.l;
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, &$7, &$9, 0); /* monoflop with reset */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTINT", &$$);	/* set and reset clocked by same clock or timer */
#endif
	    }
	/************************************************************
	 * SRT(aexpr,(cexpr|texpr,aexpr),aexpr[,(cexpr|texpr[,aexpr])])
	 *	SRT(set,clkSet,         reset)
	 *	SRT(set,timSet,delaySet,reset)
	 *	SRT(set,clkSet,         reset,clk)
	 *	SRT(set,timSet,delaySet,reset,clk)
	 *	SRT(set,clkSet,         reset,tim)
	 *	SRT(set,timSet,delaySet,reset,tim)
	 *	SRT(set,clkSet,         reset,tim,delay)
	 *	SRT(set,timSet,delaySet,reset,tim,delay)
	 ***********************************************************/
	| BLTINT '(' aexpr ',' ctdref ',' aexpr cref ')'	{
		Lis	lis1;
		$$.f = $1.f; $$.l = $9.l;
		lis1.v = sy_push(iclock);		/* to avoid shift reduce conflict */
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, &lis1, &$8, 0); /* monoflop with reset */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTINT", &$$);	/* set clocked by ext clock or timer */
#endif
	    }						/* reset clocked by iClock */
	/************************************************************
	 * SRT(aexpr,(cexpr|texpr,aexpr),aexpr,(cexpr|texpr,aexpr),(cexpr|texpr[,aexpr]))
	 *	SRT(set,clkSet,         reset,clkReset,           clk)
	 *	SRT(set,timSet,delaySet,reset,clkReset,           clk)
	 *	SRT(set,clkSet,         reset,clkReset,           tim)
	 *	SRT(set,timSet,delaySet,reset,clkReset,           tim)
	 *	SRT(set,clkSet,         reset,clkReset,           tim,delay)
	 *	SRT(set,timSet,delaySet,reset,clkReset,           tim,delay)
	 *	SRT(set,clkSet,         reset,timReset,delayReset,clk)
	 *	SRT(set,timSet,delaySet,reset,timReset,delayReset,clk)
	 *	SRT(set,clkSet,         reset,timReset,delayReset,tim)
	 *	SRT(set,timSet,delaySet,reset,timReset,delayReset,tim)
	 *	SRT(set,clkSet,         reset,timReset,delayReset,tim,delay)
	 *	SRT(set,timSet,delaySet,reset,timReset,delayReset,tim,delay)
	 ***********************************************************/
	| BLTINT '(' aexpr ',' ctdref ',' aexpr ',' ctdref ',' ctref ')'	{
		$$.f = $1.f; $$.l = $12.l;
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, &$9, &$11, 0); /* monoflop with reset */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fexpr: BLTINT", &$$);	/* set clocked by ext clock or timer */
#endif
	    }						/* reset clocked by different clock or timer */
	;

	/************************************************************
	 *
	 * if, if else and switch immediate expressions
	 *
	 * no assignment allowed for ffexpr - they stand alone
	 *
	 ***********************************************************/

ifini	: IF '(' aexpr cref ')'		{		/* if (expr) { x++; } */
		if (openT4T5(1)) ierror("IF: cannot open:", T4FN); /* rewind if necessary */
		writeCexeString(T4FP, ++c_number);	/* and record for copying */
		if (iFunSymExt == 0) {			/* defer freeing till called */
		    functionUse[0].c_cnt |= F_CALLED;	/* flag for copying temp file */
		    functionUse[c_number].c_cnt++;	/* free this 'if' function for copying */
		}
		fprintf(T4FP, "    if (iC_gf->gt_val < 0)\n");
	    }
				cBlock	{		/* { x++; } */
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, 0, &$7);
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
		if (openT4T5(1)) ierror("SWITCH: cannot open:", T4FN); /* rewind if necessary */
		writeCexeString(T4FP, ++c_number);	/* and record for copying */
		if (iFunSymExt == 0) {			/* defer freeing till called */
		    functionUse[0].c_cnt |= F_CALLED;	/* flag for copying temp file */
		    functionUse[c_number].c_cnt++;	/* free this 'switch' function for copying */
		}
		fprintf(T4FP, "    switch (iC_gf->gt_new)\n");
	    }
				    cBlock	{	/* { x++; } */
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, 0, &$7);
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
	    }
	;

casgn	: UNDEF '=' cexpr	{
		$1.v->ftype = CLCKL;
		$$.v = assignExpression(&$1, &$3, 0);	/* not strict */
		if (iC_Sflag) {
		    ierror("strict: assignment to an undeclared 'imm clock':", $1.v->name);
		    $1.v->type = ERR;			/* cannot execute properly */
		}
	    }
	| CVAR '=' cexpr	{
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    ierror("multiple assignment clock:", $1.v->name);
		    $1.v->type = ERR;			/* cannot execute properly */
		}
		assert($1.v->ftype == CLCKL);
		$$.v = assignExpression(&$1, &$3, 0);
	    }
	;

cexpr	: CVAR			{ $$.v = checkDecl($1.v); }
	| casgn			{ $$.v = sy_push($1.v); }
	| cfexpr		{
		Symbol *	sp = $1.v->le_sym;
		assert(sp);
		if (sp->ftype != iC_ftypes[sp->type]) {
		    warning("not enough arguments for function", sp->name);
		}
		sp->ftype = CLCKL;			/* clock list head */
		$$ = $1;
	    }
	| cFunCall		{
		Symbol *	sp = &nSym;
		if ($1.v == 0 || (sp = $1.v->le_sym) == 0 || sp->ftype != CLCKL) {
		    ierror("called function does not return type clock:", sp->name);
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		}
		$$ = $1;
	    }
	| '(' cexpr ')'		{ $$ = $2; }
	;

	/************************************************************
	 * CLOCK(aexpr[,(cexpr|texpr[,aexpr])])
	 *	CLOCK(set1)
	 *	CLOCK(set1,clkSet1)
	 *	CLOCK(set1,timSet1)
	 *	CLOCK(set1,timSet1,delaySet1)
	 ***********************************************************/
cfexpr	: CBLTIN '(' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, 0, 0);
	    }
	/************************************************************
	 * CLOCK(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	CLOCK(set1,set2)
	 *	CLOCK(set1,set2,clkSet1Set2)
	 *	CLOCK(set1,set2,timSet1Set2)
	 *	CLOCK(set1,set2,timSet1Set2,delaySet1Set2)
	 ***********************************************************/
	| CBLTIN '(' aexpr ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, &$6, 0, 0);
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
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, &$8, 0, 0);
	    }
	;

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
	    }
	;

tasgn	: UNDEF '=' texpr	{
		$1.v->ftype = TIMRL;
		$$.v = assignExpression(&$1, &$3, 0);	/* not strict */
		if (iC_Sflag) {
		    ierror("strict: assignment to an undeclared 'imm timer':", $1.v->name);
		    $1.v->type = ERR;			/* cannot execute properly */
		}
	    }
	| TVAR '=' texpr	{
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    ierror("multiple assignment timer:", $1.v->name);
		    $1.v->type = ERR;			/* cannot execute properly */
		}
		assert($1.v->ftype == TIMRL);
		$$.v = assignExpression(&$1, &$3, 0);
	    }
	;

texpr	: TVAR			{ $$.v = checkDecl($1.v); }
	| tasgn			{ $$.v = sy_push($1.v); }
	| tfexpr		{
		Symbol *	sp = $1.v->le_sym;
		assert(sp);
		if (sp->ftype != iC_ftypes[sp->type]) {
		    warning("not enough arguments for function", sp->name);
		}
		sp->ftype = TIMRL;			/* timer list head */
		$$ = $1;
	    }
	| tFunCall		{
		Symbol *	sp = &nSym;
		if ($1.v == 0 || (sp = $1.v->le_sym) == 0 || sp->ftype != TIMRL) {
		    ierror("called function does not return type timer:", sp->name);
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		}
		$$ = $1;
	    }
	| '(' texpr ')'		{ $$ = $2; }
	;

	/************************************************************
	 *
	 * TIMER() has a preset off delay of 0.
	 * Such timers will clock with iClock on the falling edge of
	 * the master gate. They will clock with iClock on the rising
	 * edge if the on delay is 0.
	 *
	 ***********************************************************/

tfexpr	: TBLTIN '(' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, 0, 0);
	    }
	/************************************************************
	 * TIMER(aexpr[,(cexpr|texpr[,aexpr])])
	 *	TIMER(set1)
	 *	TIMER(set1,clkSet1)
	 *	TIMER(set1,timSet1)
	 *	TIMER(set1,timSet1,delaySet1)
	 ***********************************************************/
	| TBLTIN '(' aexpr ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, &$6, 0, 0);
	    }
	/************************************************************
	 * TIMER(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	TIMER(set1,set2)
	 *	TIMER(set1,set2,clkSet1Set2)
	 *	TIMER(set1,set2,timSet1Set2)
	 *	TIMER(set1,set2,timSet1Set2,delaySet1Set2)
	 ***********************************************************/
	| TBLTIN '(' aexpr ',' ctdref ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, &$8, 0, 0);
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
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0, 0, &val1);
	    }
	/************************************************************
	 * TIMER1(aexpr,aexpr[,(cexpr|texpr[,aexpr])])
	 *	TIMER1(set1,set2)
	 *	TIMER1(set1,set2,clkSet1Set2)
	 *	TIMER1(set1,set2,timSet1Set2)
	 *	TIMER1(set1,set2,timSet1Set2,delaySet1Set2)
	 ***********************************************************/
	| TBLTI1 '(' aexpr ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, &$6, 0, &val1);
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
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, &$8, 0, &val1);
	    }
	;

	/************************************************************
	 *
	 * Embedded C function  or macro calls.
	 * These functions or macros may be declared implicitly via a
	 * #include or #define in a preceding literal block or declared
	 * and or defined in a literal block.
	 *
	 * Such functions must have at least 1 parameter which is
	 * immediate to trigger execution of the function. Any changes
	 * in non-immediate parameters will not trigger execution and
	 * should be handled with care.
	 *
	 * All bit expressions in parameters will be converted to imm int.
	 *
	 * The return value of all functions must be int. If a function
	 * is to be used with a different return value, define a wrapper
	 * function in a literal block.
	 *
	 * If a C function or macro is going to be used in an iC immediate
	 * expression it must be declared with an extern C function
	 * declaration in the iC code - at least if 'use strict' is active.
	 * (it is - isn't it) Such extern declarations make the debugging
	 * of iC code much simpler.
	 *
	 * C functions in blocks of C code controlled by iC if else or
	 * switch are not limited in this way. They must fit in with
	 * declarations made in C literal blocks and produce consistent
	 * C code.
	 *
	 ***********************************************************/

cCall	: UNDEF '(' cParams ')'	{
		$$.f = $1.f; $$.l = $4.l;
		$$.v = cCallCount($1.v, $3.v);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "cCall: UNDEF", &$$);
#endif
	    }
	| CNAME '(' cParams ')'	{
		$$.f = $1.f; $$.l = $4.l;
		$$.v = cCallCount($1.v, $3.v);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "cCall: CNAME", &$$);
#endif
	    }
	| UNDEF '(' cParams error ')'	{
		$$.f = $1.f; $$.l = $5.l;
		$$.v = cCallCount($1.v, $3.v);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "cCall: UNDEF error", &$$);
#endif
		iclock->type = ERR; yyerrok;
	    }
	| CNAME '(' cParams error ')'	{
		$$.f = $1.f; $$.l = $5.l;
		$$.v = cCallCount($1.v, $3.v);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "cCall: CNAME error", &$$);
#endif
		iclock->type = ERR; yyerrok;
	    }
	;

cParams	: /* nothing */		{ $$.v =  0; }
	| cPlist		{ $$   = $1; }	/* do not allow extra comma for C parameters */
	;

cPlist	: aexpr			{
		$$.f = $1.f; $$.l = $1.l;
		if (($$.v = cListCount(0, $1.v)) != 0) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "cPlist: aexpr", &$$);
#endif
	    }
	| cPlist ',' aexpr	{
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = cListCount($1.v, $3.v)) != 0) {
		    assert($$.f == 0 || $$.f >= iCbuf && $$.l < &iCbuf[IMMBUFSIZE]);
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "cPlist: cPlist , aexpr", &$$);
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

	| IMM VOID iFunTrigger '('	{
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
		iclock->type = ERR; yyerrok;
	    }
	;

	/************************************************************
	 * collect statements in the function body
	 * is now done for individual statements and assignments
	 ***********************************************************/

funcBody
	: /* nothing */			{ $$.v = 0; $$.f = $$.l = 0; clrBuf(); }
	| funcBody funcStatement	{
		$$.v = 0;			/* $$.v is not required */
		$$.f = $$.l = 0;
		clrBuf();
	    }
	| funcBody error ';'	{ $$.v = 0; $$.f = $$.l = 0; clrBuf(); iclock->type = ERR; yyerrok; }
	;

	/************************************************************
	 * Value function return statement
	 ***********************************************************/

returnStatement
	: RETURN actexpr		{
		$$.f = $2.f; $$.l = $2.l;		/* TODO $$.f should be func$ */
		$$.v = returnStatement(&$2);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(0, "returnStatement: RETURN actexpr", (Lis*)&$$);
#endif
	    }
	;

actexpr	: aexpr				{ $$ = $1; }
	| cexpr				{ $$ = $1; }
	| texpr				{ $$ = $1; }
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
		assert($1.f == 0 || $1.f >= iCbuf && $1.l < &iCbuf[IMMBUFSIZE]);
		$$.v->le_first = $1.f;
		$$.v->le_last = $1.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fPlist: formalParameter", &$$);
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
		assert($3.f == 0 || $3.f >= iCbuf && $3.l < &iCbuf[IMMBUFSIZE]);
		lp->le_first = $3.f;
		lp->le_last = $3.l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "fPlist: fPlist , formalParameter", &$$);
#endif
	    }
	;

	/************************************************************
	 * individual formal parameter
	 * since formal parameters are declared before anything else
	 * in the function block definition and since they are in their
	 * own namespace, with the function name prefix @ they cannot
	 * be anything but UNDEF, except NUMBER or IO variables, which
	 * will cause a syntax error.
	 ***********************************************************/

formalParameter
	: ASSIGN formalParaTypeDecl UNDEF	{
		$$ = $3;				/* formal assign parameter Symbol */
		$$.v->ftype = $2.v.ftype;		/* TYPE bit int clock timer */
		$$.v->type  = $2.v.type;		/* assign parameter is set to UDF */
		iFunSyText = 0;				/* no more function symbols */
	    }
	| formalParaTypeDecl UNDEF	{
		int	ft;

		$$ = $2;				/* formal value parameter Symbol */
		$$.v->ftype = ft = $1.v.ftype;		/* TYPE bit int clock timer */
		if (ft >= CLCKL) {			/* check that CLCKL TIMRL */
		    ft -= CLCKL - CLCK;			/* and CLCK and TIMR are adjacent */
		}
		$$.v->type = iC_types[ft];		/* value parameter has type set */
		iFunSyText = 0;				/* no more function symbols */
	    }
	;

formalParaTypeDecl
	: TYPE				{
		$$.f = $$.l = $1.l;			/* do not include in expression string */
		$$.v.ftype = $1.v->ftype;
		$$.v.type = $$.v.em = $$.v.fm = 0;
		iFunSyText = iFunBuffer;		/* expecting a new function symbol */
	    }
	| IMM TYPE			{
		$$.f = $$.l = $2.l;			/* do not include in expression string */
		$$.v.ftype = $2.v->ftype;		/* IMM is optional */
		$$.v.type = $$.v.em = $$.v.fm = 0;
		iFunSyText = iFunBuffer;		/* expecting a new function symbol */
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

iFunCallHead
	: IFUNCTION '('			{
		$$.f = $1.f; $$.l = $2.l;
		$$.v = pushFunCall($1.v);		/* save globals for nested function calls */
	    }
	;

iFunCall: iFunCallHead rParams ')'	{
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		$$ = cloneFunction(&$1, &$2, &$3);	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "iFunCall", &$$);
#endif
	    }
	| iFunCallHead rParams error ')'	{
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		$$ = cloneFunction(&$1, &$2, &$4);	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "iFunCall error", &$$);
#endif
		iclock->type = ERR; yyerrok;
	    }
	;

	/************************************************************
	 * immediate function with immediate return value clock
	 * Call:
	 *	clk = cf1(b1, iClock);
	 ***********************************************************/

cFunCallHead
	: CFUNCTION '('			{
		$$.f = $1.f; $$.l = $2.l;
		$$.v = pushFunCall($1.v);		/* save globals for nested function calls */
	    }
	;

cFunCall: cFunCallHead rParams ')'	{
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		$$ = cloneFunction(&$1, &$2, &$3);	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "cFunCall", &$$);
#endif
	    }
	| cFunCallHead rParams error ')'	{
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		$$ = cloneFunction(&$1, &$2, &$4);	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "cFunCall error", &$$);
#endif
		iclock->type = ERR; yyerrok;
	    }
	;

	/************************************************************
	 * immediate function with immediate return value timer
	 * Call:
	 *	tim = tf1(b2, clk);
	 ***********************************************************/

tFunCallHead
	: TFUNCTION '('			{
		$$.f = $1.f; $$.l = $2.l;
		$$.v = pushFunCall($1.v);		/* save globals for nested function calls */
	    }
	;

tFunCall: tFunCallHead rParams ')'	{
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		$$ = cloneFunction(&$1, &$2, &$3);	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "tFunCall", &$$);
#endif
	    }
	| tFunCallHead rParams error ')'	{
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		$$ = cloneFunction(&$1, &$2, &$4);	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "tFunCall error", &$$);
#endif
		iclock->type = ERR; yyerrok;
	    }
	;

	/************************************************************
	 * void immediate function
	 * Call:
	 *	imm bit b1, b3; imm int i3, delay;
	 *	vf1(b3, i3, b1 & IX0.0, delay * 10);
	 ***********************************************************/

vFunCallHead
	: VFUNCTION '('			{
		$$.f = $1.f; $$.l = $2.l;
		$$.v = pushFunCall($1.v);		/* save globals for nested function calls */
	    }
	;

vFunCall: vFunCallHead rParams ')'	{
		Lis	lis;
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		lis = cloneFunction(&$1, &$2, &$3);	/* clone function from call head */
		$$.f = lis.f; $$.l = lis.l;
		$$.v = 0;				/* $$.v is Sym - not compatible */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(0, "vFunCall", (Lis*)&$$);
#endif
	    }
	| vFunCallHead rParams error ')'	{
		Lis	lis;
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		lis = cloneFunction(&$1, &$2, &$4);	/* clone function from call head */
		$$.f = lis.f; $$.l = lis.l;
		$$.v = 0;				/* $$.v is Sym - not compatible */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(0, "vFunCall error", (Lis*)&$$);
#endif
		iclock->type = ERR; yyerrok;
	    }
	;

	/************************************************************
	 * real parameter list for immediate function calls
	 ***********************************************************/

rParams	: /* nothing */			{ $$.v =  0; }
	| rPlist			{ $$   = $1; }
	| rPlist ','			{ $$   = $1; $$.l = $2.l; }
	;

rPlist	: actexpr			{
		$$.f = $1.f; $$.l = $1.l;
		if ($1.v == 0) {
		    if (const_push(&$1)) { errInt(); YYERROR; }
		}
		$$.v = handleRealParameter(0, $1.v);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "rPlist: actexpr", &$$);
#endif
	    }
	| rPlist ',' actexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v == 0) {			/* must extract value from aexpr */
		    if (const_push(&$3)) { errInt(); YYERROR; }
		}
		$$.v = handleRealParameter($1.v, $3.v);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(1, "rPlist: rPlist , actexpr", &$$);
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
static int	iniDebug;
static char const * iniPtr;
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
 *	a copy of the source file name (or stdin) is kept in inpNM for
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
compile(
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
    if ((iC_debug & 0402) != 0402)
#endif
    lexflag = C_FIRST;				/* supress initial pre-processor lines */
    lineflag = 1;				/* previous line was complete */

    if (lstNM &&
	((chmod(lstNM, 0644) &&			/* make list file writable */
	errno != ENOENT) ||			/* if it exists */
	(iC_outFP = fopen(lstNM, "w+")) == NULL)) {
	perror("chmod or fopen");
	return Lindex;
    }
    errFilename = errNM;			/* open on first error */
    init();					/* initialise symbol table - allows ierror() */
    if (inpPath) {
	strncpy(inpNM, inpPath, BUFS);
	r = 0;
	if (strlen(iC_defines) == 0) {
	    /* pre-compile if iC files contains any #include, #define #if etc */
#ifdef	WIN32
	    fflush(iC_outFP);
	    /* CMD.EXE does not know '' but does not interpret $, so use "" */
	    /* don't use system() because Win98 command.com does not return exit status */
	    /* use spawn() instead - spawnlp() searches Path */
	    r = _spawnlp( _P_WAIT, "perl",  "perl",  "-e", "\"$s=1; while (<>) { if (/^[ \\t]*#/) { $s=0; last; } } exit $s;\"", inpPath, NULL );
	    if (r < 0) {
		ierror("cannot spawn perl -e ...", inpPath);
		perror("spawnlp");
		return Iindex;			/* error opening input file */
	    }
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### test: perl -e \"$s=1; print 'perl $s = ', $s, '...'; while (<>) { if (/^[ \\t]*#/) { $s=0; print 'perl $_ = ', $_, '...'; last; } } print 'perl $s = ', $s, '...'; exit $s;\" %s; $? = %d\n", inpPath, r);
#endif
#else	/* not WIN32 */
	    snprintf(execBuf, BUFS, "perl -e '$s=1; while (<>) { if (/^[ \\t]*#/) { $s=0; last; } } exit $s;' %s", inpPath);
	    r = system(execBuf);		/* test with perl script if #include in input */
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
	    /* iC_defines is not empty and has -Dyyy or -Uyyy or #xxx was found by perl script */
	    /* pass the input file through the C pre-compiler to resolve #includes and macros */
	    if ((fd = mkstemp(T0FN)) < 0 || close(fd) < 0 || unlink(T0FN) < 0) {
		ierror("compile: cannot make or unlink:", T0FN);
		perror("unlink");
		return T0index;			/* error unlinking temporary file */
	    }
#ifdef	WIN32
	    /* cl does not know -x c, -E is preprocess to stdout, no -o option  */
	    snprintf(execBuf, BUFS, "cl %s -E -C -I/usr/local/include %s > %s 2> %s",
		iC_defines, inpPath, T0FN, T6FN);
#else	/* not WIN32 */
	    /* Cygnus does not understand cc - use macro CC=gcc, pass comments with -C */
	    snprintf(execBuf, BUFS, SS(CC) "%s -E -C -I/usr/local/include -x c %s -o %s 2> %s",
		iC_defines, inpPath, T0FN, T6FN);
#endif	/* WIN32 */
	    r1 = system(execBuf);		/* Pre-compile iC file */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### pre-compile: %s; $? = %d\n", execBuf, r1>>8);
#endif
	    if (r1 != 0) {
		if ((T6FP = fopen(T6FN, "r")) == NULL) {
		    return T6index;		/* error opening CC error file */
		}
		while (fgets(lineBuf, sizeof lineBuf, T6FP)) {
		    ierror(SS(CC) ":", lineBuf);	/* CC error message */
		}
		fclose(T6FP);
		if (!(iC_debug & 04000)) {
		    unlink(T6FN);
		}
		ierror("compile: cannot run:", execBuf);
		return T0index;
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
    }
    strncpy(prevNM, inpNM, BUFS);
    outFlag = outNM != 0;			/* global flag for compiled output */
    if (iC_debug & 046) {			/* begin source listing */
	fprintf(iC_outFP, "******* %-15s ************************\n", inpNM);
    }
    if ((iC_debug & 020000) == 0) {		/* new code with function definitions */
	iniPtr = initialFunctions;		/* read system function definitions first */
	iniDebug = iC_debug;			/* save iC_debug flag */
	if ((iC_debug & 010400) != 010400) iC_debug &= ~ 047; /* suppress logic expansion for ini code */
    } else if ((iC_debug & 021000) == 020000) {	/* suppress internal functions to test old code */
	c_number = iC_genCount;			/* skip internal function numbers for compatibility */
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
} /* compile */

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
    size_t	iniLen;
    char *	cp;
    char	tempNM[BUFS];
    char	tempBuf[TSIZE];
    static int	gen_count = 0;

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
     *  iClex this will generate the BF yacc token, which must appear
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
	 *  The following block takes input directly from the
	 *  iC system function definitions in init.c. These definitions
	 *  are parsed before any line of iC code is read.
	 ************************************************************/
	if (iniPtr) {
	    iniLen = strcspn(iniPtr, "\n");
	    if (iniPtr[iniLen] == '\n') {
		iniLen++;
	    }
	    else if (iniLen == 0) {
		iC_debug = iniDebug;		/* restore logic expansion */
		iniPtr = getp[0] = 0;		/* iC system function definitions read */
		clrBuf();			/* clear iCbuf after initial functions */
		continue;
	    }
	    assert(iniLen < CBUFSZ);
	    getp[0] = strncpy(chbuf[0], iniPtr, iniLen); /* copy with terminating \n */
	    chbuf[0][iniLen] = '\0';
	    iniPtr += iniLen;
	}
	/************************************************************
	 *  getp has reached end of previous chbuf filling
	 *  fill chbuf with a new line
	 *  NOTE: getp === NULL at start of file (chbuf has EOF line)
	 *
	 *  Fill either from auxBuf if some lines were saved or from fp
	 *  The first token will be in the last line of auxBuf[]
	 ************************************************************/
	else {
	    if ((prevflag = lineflag) != 0) {
		lineno++;			/* count previous line */
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
		errline = lineno;		/* no listing line at EOF */
		return (EOF);
	    }
	    lineflag = chbuf[x][strlen(chbuf[x])-1] == '\n' ? 1 : 0; /* this line terminated with \n */
	}

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
		if (sscanf(chbuf[x], " #line %d \"%[-/:A-Za-z_.0-9<>]\"",
#else	/* not WIN32 */
		if (sscanf(chbuf[x], " # %d \"%[-/A-Za-z_.0-9<>]\"",
#endif	/* WIN32 */
		    &temp1, inpNM) == 2) {
		    unsigned int usetype;
		    lineno = temp1 - 1;		/* handled in iC-code */
		    lexflag |= C_LINE;
		    if (strcmp(inpNM, prevNM) && strchr(inpNM, '<') == 0) {
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
#ifdef	WIN32
	    if ((slen = sscanf(chbuf[x], " # line %d \"%[-/:A-Za-z_.0-9<>]\"	%s",
#else	/* not WIN32 */
	    if ((slen = sscanf(chbuf[x], " # line %d \"%[-/A-Za-z_.0-9<>]\"	%s",
#endif	/* WIN32 */
		&temp1, tempNM, tempBuf)) >= 2) {
		savedLineno = lineno;
		lineno = temp1 - 1;
		if (slen == 3 &&		/* has comment string after #line x "init.c" */
		    strcmp(tempNM, genName) == 0) {	/* in the system file "init.c" */
		    int	i;
		    
		    gen_count++;		/* count C functions in pre-compile code */
		    if (iC_debug & 010000) {	/* generate listing of compiler */
			goto listCfunction;	/* internal functions*/
		    }
		    for (i = 1; i <= iC_genCount; i++) {
			/********************************************************
			 * System C CODE
			 * For function blocks with more than 1 C function, the
			 * same test will be repeated. All C functions for the
			 * function block will be listed if the function block is
			 * in use on the basis of the first entry for the line.
			 ********************************************************/
			if (temp1 == genLineNums[i] &&	/* find i for this line */
			    functionUse[i].c_cnt) {	/* is function block in use? */
			    goto listCfunction;		/* yes */
			}
		    }
		    continue;			/* no listing for system C CODE not in use */
		} else if (gen_count) {
		    assert(gen_count == iC_genCount);	/* correct in init.c */
		    gen_count = 0;
		}
	      listCfunction:
		strncpy(inpNM, tempNM, BUFS);	/* defer changing name until used */
		if (iC_debug & 06) {
		    if (lexflag & C_FIRST) {
			fprintf(iC_outFP, "******* C CODE          ************************\n");
		    }
		    if (strcmp(inpNM, prevNM)) {
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
		    if (cFn > iC_genCount || functionUse[cFn].c_cnt || (iC_debug & 010000)) {
			fprintf(iC_outFP, "%03d\t(%d) %s", lineno, cFn, chbuf[x]);
		    }
		    cFn = 0;
		} else if ((iC_debug & 06) && (lexflag & C_PARSE) && chbuf[x][0] != '\t' && chbuf[x][0] != ' ') {
		    fprintf(iC_outFP, "%03d\t    %s", lineno, chbuf[x]);
		} else if (((iC_debug & 06)  && (lexflag & C_PARSE)) ||
		    ((iC_debug & 040)  && (lexflag & C_PARSE) == 0)) {
		    fprintf(iC_outFP, "%03d\t%s", lineno, chbuf[x]);
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
 *	Get a number, which may be decimal, octal or hexadecimal
 *
 *	Convert from getp in chbuf - termination will be at least at
 *	newline supplied by fgets() or final '\0'. Usually it is
 *	erlier and that part is copied into iCtext with get().
 *
 *******************************************************************/

static long
getNumber(void)
{
    long	value;
    char *	cp;
    char *	ep;

    value = strtol(getp[0], &ep, 0);		/* convert to long */
    assert (ep <= &chbuf[0][CBUFSZ]);
    for (cp = getp[0]; cp < ep; cp++) {
	get(T0FP, 0);				/* transfer to iCtext */
    }
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "getNumber: '%s' converted to %ld\n", iCtext, value);
	fflush(iC_outFP);
    }
#endif
    return value;
} /* getNumber */

/********************************************************************
 *
 *	Lexer for iC grammar
 *
 *******************************************************************/

static int
iClex(void)
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
	iClval.val.v = c_number << 8;		/* return case # */
	ccfrag = 0;
	c = CCFRAG;
	goto retfl;
    }
    iCleng = 0;
    while ((c = get(T0FP, 0)) !=  EOF) {
	Symbol *	symp;
	Symbol *	sp;
	List_e *	lp;
	int		len;
	int		rest;
	char		tempBuf[TSIZE];		/* make long enough for format below */

	if (isspace(c)) {			/* space \f \n \r \t \v */
	    iCleng = 0;
	    continue;				/* ignore white space */
	}
	if (isdigit(c)) {
	    unget(c);				/* must be at least a single 0 */
	    getNumber();			/* decimal octal or hex - getNumber marks text boundaries */
	    iClval.val.v = 0;			/* value is never used */
	    c = NUMBER;
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
		  useWord:
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
		     *  Generally if yacc were to handle the '.' as a seperate token,
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
	    if (iFunSymExt && ! qtoken) {	/* in function definition */
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
			    strncmp(iCtext, iFunBuffer, iFunSymExt - iFunBuffer)) {
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
	    }
	    if (symp == 0) {
		if ((symp = lookup(iCtext)) == 0) {
		    symp = install(iCtext, typ, ftyp); /* usually UDF UDFA */
		} else if (typ == ERR) {
		    symp->type = ERR;		/* mark ERROR in QX%d_%d */
		}
	    }
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "iClex: %s %s %s\n",
		    symp->name, iC_full_type[symp->type], iC_full_ftype[symp->ftype]);
		fflush(iC_outFP);
	    }
#endif
	    iClval.sym.v = symp;		/* return actual symbol */
	    while ( (typ = symp->type) == ALIAS &&
		    (lp = symp->list) != 0 &&
		    (sp = lp->le_sym) != 0) {
		symp = sp;			/* with token of original */
	    }
	    /********************************************************************
	     *  no need to clear sp->em
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
	    if (qtoken) {
		c = qtoken;			/* LOUT or AOUT */
	    } else
	    if ((c = lex_typ[typ]) == 0) {	/* 0 AVARC 0 LVARC 0 ... NUMBER ... val not needed */
		c = lex_act[symp->ftype];	/* UNDEF AVAR LVAR ..YYERRCODE.. AOUT LOUT CVAR TVAR */
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
	} else {
	    c1 = get(T0FP, 0);			/* unget possible EOF after c has been processed */
	    switch (c) {
	    case '!':
		if (c1 == '=') {
		    c = CMP; goto found;	/* != */
		}
	    case '~':
		break;				/* ! or ~ */
	    case '%':
		if (c1 == '{') {		/*    >>> '}' */
		    c = LHEAD; goto found;	/* %{ >>> %} */
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* %= */
		}
		c = AOP;			/* % */
		break;
	    case '&':
		if (c1 == '&') {
		    c = AA; goto found;		/* && */
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
		c = PM;				/* + */
		break;				/* + binary or unary */
	    case '-':
		if (c1 == '-') {
		    c = PPMM; goto found;	/* -- */
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* -= */
		} else if (c1 == '>') {
		    c = PR; goto found;		/* -> */
		}
		c = PM;				/* - */
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
		    c = AOP;			/* / */
		    break;
		}
		iCleng = 0;			/* end of comment */
		continue;
	    case '<':
		if (c1 == '<') {
		    if ((c1 = get(T0FP, 0)) == '=') {
			c = OPE; goto found;	/* <<= */
		    }
		    c = AOP;			/* << */
		    break;
		}
		if (c1 != '=') {
		    unget(c1);
		}
		c = CMP; goto found;		/* < or <= */
	    case '=':
		if (c1 == '=') {
		    c = CMP; goto found;	/* == */
		}
		*iCtext = '#';			/* to recognise <= => transmogrify = */
		break;
	    case '>':
		if (c1 == '>') {
		    if ((c1 = get(T0FP, 0)) == '=') {
			c = OPE; goto found;	/* >>= */
		    }
		    c = AOP;			/* >> */
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
		    c = OO; goto found;		/* || */
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
	    iClval.val.v = c;			/* return to yacc as is */
	retfl:
	    iClval.val.f = iCstmtp;		/* all sources are iClval.val */
	    if ((c == PM || c == PPMM) &&
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
	    iClval.val.l = iCstmtp = strncpy(iCstmtp, iCtext, len) + len;
	}
	return c;				/* return token to yacc */
    }
    return 0;					/* EOF */
} /* iClex */

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
 *	sets iClock type to ERR to prevent execution of generated code
 *
 *******************************************************************/

void
ierror(						/* print error message */
    char *	str1,
    char *	str2)
{
    iclock->type = ERR;				/* prevent execution */
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

void
errBit(void)
{
    ierror("no constant allowed in bit expression", NS);
} /* errBit */

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

void
yyerror(char *	s)
{
    char *	cp = chbuf[lexflag&C_PARSE];
    int		n, n1;
    char	erbuf[TSIZE];

    errLine();
    fprintf(iC_outFP, "*** ");			/* do not change - used as search key in iClive */
    if (errFlag) fprintf(iC_errFP, "*** ");
    if ((lexflag & C_PARSE) == 0) {
	n = getp[0] - chbuf[0] - iCleng;
    } else {
	n = column - c_leng;
    }
    for (n1 = 0; n > 0; n--, cp++) {
	n1++;
	if (*cp == '\t') {
	    while (n1 % 8) {
		n1++;
	    }
	}
    }
    ynerrs++;
#ifdef NEWSTYLE
    n = snprintf(erbuf, TSIZE, "%s '%s'", s, inpNM);
#else
    n = snprintf(erbuf, TSIZE, "%s %d in %s", s, ynerrs, inpNM);
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
 *	when yacc token CCFRAG is recognised in iClex()
 *
 *******************************************************************/

static int
copyCfrag(char s, char m, char e, FILE * oFP)
	/* copy C action to the next ; , or closing brace or bracket */
{
    int		brace;
    int		c;
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
		/* TODO fix lineno and name */
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
		putc(c, oFP);
		if ((c = get(T0FP, 0)) == '/') {
		    do {			/* start C++ style comment */
			putc(c, oFP);
			if ((c = get(T0FP, 0)) == EOF) goto eof_error;
		    } while (c != '\n');
		} else if (c == '*') {
		    do {			/* start C style comment */
			putc(c, oFP);
			while (c != '*') {
			    if ((c = get(T0FP, 0)) == EOF) goto eof_error;
			    putc(c, oFP);
			}
		    } while ((c = get(T0FP, 0)) != '/');
		} else {
		    unget(c);
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
    if (c_compile(T4FP, T5FP, C_PARSE|C_BLOCK1, lp) || copyXlate(T5FP, T1FP, 0, 0, 02)) {
	ierror("c_compile failed:", txt);
    }
    lexflag = saveLexflag;			/* restore iC compile state */
    lineno  = saveLineno;
} /* ffexprCompile */

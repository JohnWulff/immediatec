%{ static const char comp_y[] =
"@(#)$Id: comp.y,v 1.52 2001/04/27 20:07:37 jw Exp $";
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
 *	comp.y
 *	grammar for icc compiler
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<setjmp.h>
#include	<assert.h>

#include	"icc.h"
#include	"comp.h"

/* "comp.y	3.70	95/02/03 Copyright (c) 1985-1993 by John E. Wulff" */

static int	get(void);
static void	unget(char);
static int	yylex(void);
static void	yyerror(char *);	/* called for yacc syntax error */
int		ynerrs;			/* count of yyerror() calls */
		/* NOTE yynerrs is reset for every call to yaccpar() */
static void	errBit(void);
static void	errInt(void);
static int	copyCfrag(char, char, char);	/* copy C action */
static unsigned char ccfrag;		/* flag for CCFRAG syntax */
static int	dflag = 0;		/* record states dexpr */
static unsigned int stype;		/* to save TYPE in decl */
static Val	val1 = { 1, 0, 0, };	/* preset off 1 value for timers */
static char	yybuf[1024];		/* buffer to build imm statement */
char *		stmtp = yybuf;		/* pointer into yybuf used in genr.c */
%}

%union {		/* stack type */
    Sym		sym;			/* symbol table pointer */
    Lis		list;			/* linked list elements */
    Val		val;			/* int numerical values */
    Str		str;			/* one character array */
    /* allows storing character as yylval.val.v (2nd byte is NULL) */
    /* then char array can be referenced as $1.v, single char as $1.v[0] */
}

%{

static void
pu(int t, char * token, Lis * node)
{
    char *	cp;
    char *	ep;

    switch (t) {
    case 0:
	fprintf(outFP, ">>>Sym	%s	%s :	", token, ((Sym*)node)->v->name);
	break;
    case 1:
	if (node->v) {
	    fprintf(outFP, ">>>Lis	%s	%s =	", token, node->v->le_sym->name);
	} else {
	    fprintf(outFP, ">>>Lis	%s	0 =	", token);
	}
	break;
    case 2:
	fprintf(outFP, ">>>Val	%s	%d =	", token, ((Val*)node)->v);
	break;
    case 3:
	fprintf(outFP, ">>>Str	%s	%2.2s =	", token, ((Str*)node)->v);
	break;
    }
    cp = node->f; ep = node->l;
    while (cp < ep) {
	putc(*cp++, outFP);
    }
    putc('\n', outFP);
    fflush(outFP);
} /* pu */

static void
pd(const char * token, Symbol * ss, unsigned int s1, Symbol * s2)
{
    fprintf(outFP, ">>>%s\t%s\t[%c%c %c] [%c%c %c] => [%c%c %c]\n",
	token, s2->name,
	os[(s1 >> 8)&TM], os[!!((s1 >> 8)&~TM)], fos[s1 & 0xff],
	os[s2->type&TM], os[!!(s2->type&~TM)], fos[s2->ftype],
	os[ss->type&TM], os[!!(ss->type&~TM)], fos[ss->ftype]);
    fflush(outFP);
} /* pd */

%}

	/************************************************************
	 *
	 * iC token types
	 *
	 ***********************************************************/

%token	<sym>	UNDEF AVARC AVAR LVARC LVAR ACTION AOUT LOUT BLTIN1 BLTIN2 BLTIN3
%token	<sym>	CVAR CBLTIN TVAR TBLTIN TBLTI1 NVAR STATIC BLATCH BFORCE DLATCH
%token	<sym>	EXTERN IMM TYPE IF ELSE SWITCH
%token	<val>	NUMBER CCFRAG
%token	<str>	LEXERR COMMENTEND LHEAD
%type	<sym>	program statement simplestatement lBlock variable valuevariable
%type	<sym>	decl extDecl asgn dasgn casgn dcasgn tasgn dtasgn
%type	<list>	expr aexpr lexpr fexpr cexpr cfexpr texpr tfexpr ifini ffexpr
%type	<list>	cref ctref ctdref dexpr funct params plist
%type	<val>	cBlock declHead extDeclHead
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
%nonassoc <str>	NOTL PPMM	/* unary operators ! ~ ++ -- (+ - & *) */
%right	<str>	PR 		/* structure operators -> . */
%%

	/************************************************************
	 *
	 * iC grammar
	 *
	 ***********************************************************/

program	: /* nothing */		{ $$.v = 0;  stmtp = yybuf; }
	| program statement	{ $$   = $2; stmtp = yybuf; }
	| program error ';'	{ $$.v = 0;  stmtp = yybuf; iclock->type = ERR; yyerrok; }
	;

statement
	: ';'			{ $$.v = 0;  }
	| simplestatement ';'	{ $$   = $1; }
	| ffexpr		{ $$.v = op_asgn(0, &$1, GATE); }
	    /* op_asgn(0,...) returns 0 for missing slave gate in ffexpr */
	| lBlock		{ $$.v = 0;  }
	;

simplestatement
	: extDecl		{ $$ = $1; }
	| decl			{ $$ = $1; }
	| asgn			{ $$ = $1; }
	| dasgn			{ $$ = $1; }
	| casgn			{ $$ = $1; }
	| dcasgn		{ $$ = $1; }
	| tasgn			{ $$ = $1; }
	| dtasgn		{ $$ = $1; }
	;

variable
	: valuevariable		{ $$ = $1; }		/* value variable */
	| CVAR			{ $$ = $1; }		/* clock variable */
	| TVAR			{ $$ = $1; }		/* timer variable */
	;

valuevariable
	: LVAR			{ $$ = $1; }		/* logical bit variable */
	| AVAR			{ $$ = $1; }		/* arithmetic variable */
	| LOUT			{ $$ = $1; }		/* output bit variable */
	| AOUT			{ $$ = $1; }		/* output arith. variable */
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
	 * variable may normally not be assigned in this module.
	 * To allow lists of extern immediate declarations of all variables
	 * in a common include file, a simple immediate type declaration
	 * after the extern immediate declaration (or after the #include
	 * statement containing the extern immeditate type declaration)
	 * declares that this variable will be assigned in the current
	 * module and must be assigned in this module. The type (bit, int,
	 * clock or timer) must match the type used previously in the
	 * extern declaration.
	 *
	 * An extern type declaration of a particular variable may not
	 * occurr after its simple declaration or its assignment.
	 *
	 ***********************************************************/

extDecl	: extDeclHead UNDEF	{
		Symbol t = *($2.v);
		$$.v = $2.v;
		$$.v->ftype = $1.v & 0xff;
		$$.v->type = $1.v >> 8;
		if (debug & 02) pd("extDecl", $$.v, $1.v, &t);
	    }
	| extDeclHead variable	{
		Symbol t = *($2.v);
		if ($2.v->ftype != ($1.v & 0xff)) {
		    error("extern declaration does not match previous declaration:", $2.v->name);
		    $2.v->type = ERR;	/* cannot execute properly */
		}
		$$.v = $2.v;
		if ($2.v->type == UDF) {		/* QXx.y QBz */
		    $$.v->ftype = $1.v & 0xff;
		    $$.v->type = $1.v >> 8;
		} else if ($2.v->type != INPW && $2.v->type != INPX) {
		    warning("extern declaration after assignment - ignored:", $2.v->name);
		}
		if (debug & 02) pd("extDecl", $$.v, $1.v, &t);
	    }
	;

extDeclHead
	: EXTERN IMM TYPE	{
		int	ftyp;
		ftyp = $3.v->ftype;
		if (ftyp >= CLCKL) {		/* check that CLCKL TIMRL */
		    ftyp -= CLCKL - CLCK;	/* and CLCK and TIMR are adjacent */
		}
		$$.v = stype = $3.v->ftype | (types[ftyp] | TM+1) << 8;
	    }
	| extDecl ','		{ $$.v = stype;	/* first TYPE */ }
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
	 ***********************************************************/

decl	: declHead UNDEF	{
		Symbol t = *($2.v);
		$$.v = $2.v;
		$$.v->ftype = $1.v & 0xff;	/* TYPE bit int clock timer */
		$$.v->type = $1.v >> 8;		/* UDF for all TYPEs */
		if (debug & 02) pd("decl", $$.v, $1.v, &t);
	    }
	| declHead variable	{
		Symbol t = *($2.v);
		if ($2.v->ftype != ($1.v & 0xff)) {
		    error("declaration does not match previous declaration:", $2.v->name);
		    $2.v->type = ERR;	/* cannot execute properly */
		}
		$$.v = $2.v;
		if (($2.v->type & ~TM) || $2.v->type == UDF) {
		    $$.v->ftype = $1.v & 0xff;	/* TYPE bit int clock timer */
		    $$.v->type = $1.v >> 8;	/* UDF for all TYPEs */
		} else if ($2.v->type != ERR && $2.v->type != INPW && $2.v->type != INPX) {
		    warning("declaration after assignment - ignored:", $2.v->name);
		}
		if (debug & 02) pd("decl", $$.v, $1.v, &t);
	    }
	;

declHead
	: IMM TYPE		{ $$.v = stype = $2.v->ftype; }
	| decl ','		{ $$.v = stype;	/* first TYPE */ }
	| dasgn ','		{ $$.v = stype;	/* first TYPE */ }
	;

	/************************************************************
	 *
	 * Assignment combined with a declaration
	 *
	 ***********************************************************/

dasgn	: decl '=' aexpr	{			/* dasgn is NOT an aexpr */
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v == 0) {
		    if ($1.v->ftype != ARITH) { $$.v = 0; errBit(); YYERROR; }
		    else if (const_push(&$3)) { $$.v = 0; errInt(); YYERROR; }
		}
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    error("assigning to wrong type ARNC or LOGC:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		}
		$$.v = op_asgn(&$1, &$3, $1.v->ftype);	/* int ARITH or bit GATE */
		if (debug & 02) pu(0, "dasgn", (Lis*)&$$);
	    }
	;

	/************************************************************
	 *
	 * Different forms of assignment
	 *
	 ***********************************************************/

asgn	: UNDEF '=' aexpr	{			/* asgn is an aexpr */
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v == 0) { $$.v = 0; errBit(); YYERROR; }
		$1.v->ftype = GATE;	/* implicitly declared as 'imm bit' */
		$$.v = op_asgn(&$1, &$3, GATE);	/* UNDEF is default GATE */
		if (debug & 02) pu(0, "asgn", (Lis*)&$$);
	    }
	| LVAR '=' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v == 0) { $$.v = 0; errBit(); YYERROR; }
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    error("multiple assignment to imm bit:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		}
		$$.v = op_asgn(&$1, &$3, GATE);
		if (debug & 02) pu(0, "asgn", (Lis*)&$$);
	    }
	| AVAR '=' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v == 0 && const_push(&$3)) { $$.v = 0; errInt(); YYERROR; }
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    error("multiple assignment to imm int:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		}
		$$.v = op_asgn(&$1, &$3, ARITH);
		if (debug & 02) pu(0, "asgn", (Lis*)&$$);
	    }
	| LOUT '=' aexpr		{
		Sym		sy;
		Lis		li;
		char		temp[30];
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v == 0) { $$.v = 0; errBit(); YYERROR; }
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    error("multiple assignment to imm bit:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		}
		sprintf(temp, "%s_0", $1.v->name);
		sy.v = install(temp, UDF, OUTX);	/* generate output Gate */
		li.v = sy_push($1.v);			/* provide a link to LOUT */
		if ((li.v = op_push(0, OR, li.v)) != 0) {
		    li.v->le_first = li.f = 0; li.v->le_last = li.l = 0;
		}
		op_asgn(&sy, &li, $1.v->ftype);
		$$.v = op_asgn(&$1, &$3, GATE);
		if (debug & 02) pu(0, "asgn", (Lis*)&$$);
	    }
	| AOUT '=' aexpr		{
		Sym		sy;
		Lis		li;
		char		temp[20];
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v == 0) { $$.v = 0; errInt(); YYERROR; }
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    error("multiple assignment to imm int:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		}
		sprintf(temp, "%s_0", $1.v->name);
		sy.v = install(temp, UDF, OUTW);	/* generate output Gate */
		li.v = sy_push($1.v);			/* provide a link to AOUT */
		if ((li.v = op_push(0, ARN, li.v)) != 0) {
		    li.v->le_first = li.f = 0; li.v->le_last = li.l = 0;
		}
		op_asgn(&sy, &li, $1.v->ftype);
		$$.v = op_asgn(&$1, &$3, ARITH);
		if (debug & 02) pu(0, "asgn", (Lis*)&$$);
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
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
	    }
	| asgn			{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = sy_push($1.v);
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "aexpr", &$$);
	    }
	;

	/************************************************************
	 *
	 * Expressions - includes unclocked built in functions
	 *
	 ***********************************************************/

expr	: UNDEF			{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = sy_push($1.v);
		$1.v->ftype = GATE;
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| NUMBER		{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = 0;			/* no node, do not need value */
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| valuevariable		{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = sy_push($1.v);
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| fexpr			{
		Symbol *	sp;

		if ($1.v == 0) YYERROR;			/* error in bltin() */
		sp = $1.v->le_sym;
		$$ = $1;
		if (sp->ftype != ftypes[sp->type & TM]) {
		    warning("not enough arguments for function", sp->name);
		}
		sp->ftype = sp->type == SH ? ARITH : GATE;
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| funct			{
		$$ = $1;
		if ($$.v) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
	    }
	| '(' aexpr ')'		{
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = $2.v) != 0) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| BLATCH '(' lexpr ')'	{		/* L(set,reset) */
		$$.f = $1.f; $$.l = $4.l;
		$$.v = op_push(sy_push($3.v->le_sym), LOGC, $3.v);
		$$.v->le_sym->type = LATCH;
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| BFORCE '(' aexpr ',' lexpr ')'	{	/* F(expr,hi,lo) */
		$$.f = $1.f; $$.l = $6.l;
		if ($3.v == 0) { $$.v = 0; errBit(); YYERROR; }
		$$.v = op_push(op_force($3.v, GATE), LOGC, $5.v);
		$$.v->le_sym->type = LATCH;
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| expr '|' expr		{		/* binary | */
		$$.f = $1.f; $$.l = $3.l;
		if (($1.v == 0 || $1.v->le_sym->ftype == ARITH) &&
		    ($3.v == 0 || $3.v->le_sym->ftype == ARITH)) {
		    $$.v = op_push($1.v, ARN, $3.v);	/* bitwise | */
		} else if ($1.v == 0 || $3.v == 0) {
		    errBit(); YYERROR;
		} else {
		    $$.v = op_push(op_force($1.v, GATE),
			OR, op_force($3.v, GATE));	/* logical | */
		}
		if ($$.v) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| expr '^' expr		{		/* binary ^ */
		$$.f = $1.f; $$.l = $3.l;
		if (($1.v == 0 || $1.v->le_sym->ftype == ARITH) &&
		    ($3.v == 0 || $3.v->le_sym->ftype == ARITH)) {
		    $$.v = op_push($1.v, ARN, $3.v);	/* bitwise ^ */
		} else if ($1.v == 0 || $3.v == 0) {
		    errBit(); YYERROR;
		} else {
		    $$.v = op_xor(op_force($1.v, GATE),
			op_force($3.v, GATE));		/* logical ^ */
		}
		if ($$.v) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| expr '&' expr		{		/* binary & */
		$$.f = $1.f; $$.l = $3.l;
		if (($1.v == 0 || $1.v->le_sym->ftype == ARITH) &&
		    ($3.v == 0 || $3.v->le_sym->ftype == ARITH)) {
		    $$.v = op_push($1.v, ARN, $3.v);	/* bitwise & */
		} else if ($1.v == 0 || $3.v == 0) {
		    errBit(); YYERROR;
		} else {
		    $$.v = op_push(op_force($1.v, GATE),
			AND, op_force($3.v, GATE));	/* logical & */
		}
		if ($$.v) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| expr CMP expr	{			/* == != < <= > >= */
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = op_push(op_force($1.v, ARITH),
		    ARN, op_force($3.v, ARITH))) != 0) {
		    $$.v = op_force($$.v, GATE);	/* default output */
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| expr AOP expr		{		/* << >> / % */
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = op_push(op_force($1.v, ARITH),
		    ARN, op_force($3.v, ARITH))) != 0) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| expr PM expr		{		/* binary + - */
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = op_push(op_force($1.v, ARITH),
		    ARN, op_force($3.v, ARITH))) != 0) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| expr '*' expr		{		/* binary * */
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = op_push(op_force($1.v, ARITH),
		    ARN, op_force($3.v, ARITH))) != 0) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }

	/************************************************************
	 *
	 * If both operands of a binary && or || operator are imm bit
	 * more precisely ftype GATE or UDFA (!ARITH) && type >ARN ||
	 * UDF or a reduced ARN variable (of type GATE), then the
	 * operation is compiled as a logical bit, rather than con-
	 * verting both imm bit (GATE) to ARITH, doing an arithmetic
	 * operation and then having default GATE again.
	 *
	 ***********************************************************/

	| expr AA expr	{			/* binary && */
		Symbol *	sp;
		int		typ;
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v &&
		    (sp = $1.v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type & TM) > ARN || typ == UDF || !sp->u.blist) &&
		    (sp = $3.v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type & TM) > ARN || typ == UDF || !sp->u.blist)) {
		    $$.v = op_push(op_force($1.v, GATE),
			AND, op_force($3.v, GATE));	/* logical & */
		} else {
		    $$.v = op_push(op_force($1.v, ARITH),
			ARN, op_force($3.v, ARITH));	/* arithmetic && */
		    $$.v = op_force($$.v, GATE);	/* default output */
		}
		if ($$.v) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| expr OO expr	{			/* binary || */
		Symbol *	sp;
		int		typ;
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v &&
		    (sp = $1.v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type & TM) > ARN || typ == UDF || !sp->u.blist) &&
		    (sp = $3.v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type & TM) > ARN || typ == UDF || !sp->u.blist)) {
		    $$.v = op_push(op_force($1.v, GATE),
			OR, op_force($3.v, GATE));	/* logical | */
		} else {
		    $$.v = op_push(op_force($1.v, ARITH),
			ARN, op_force($3.v, ARITH));	/* arithmetic || */
		    $$.v = op_force($$.v, GATE);	/* default output */
		}
		if ($$.v) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| expr '?' expr ':' expr	{	/* ? : */
		$$.f = $1.f; $$.l = $5.l;
		if (($$.v = op_push(op_force($1.v, ARITH),
		    ARN, op_push(op_force($3.v, ARITH),
		    ARN, op_force($5.v, ARITH)))) != 0) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| NOTL expr 		{		/* unary ~ or ! */
		Symbol *	sp;
		$$.f = $1.f; $$.l = $2.l;
		if ($2.v) {
		    int typ;
		    if ((sp = $2.v->le_sym)->ftype != ARITH &&
			(((typ = sp->type & TM) != ARNC && typ != ARN &&
			typ != SH) || sp->u.blist == 0)) {
						/* logical negation */
			$$.v = op_not(op_force($2.v, GATE));
		    } else {
						/* bitwise negation */
			$$.v = op_push(0, ARN, op_force($2.v, ARITH));
		    }
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		} else {
		    $$.v = 0;			/* constant negation */
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| PM expr %prec NOTL	{		/* unary + or - */
		$$.f = $1.f; $$.l = $2.l;
		if (($$.v = op_push(0, ARN, op_force($2.v, ARITH))) != 0) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "expr", &$$);
	    }
	;

lexpr	: aexpr ',' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v == 0) { $$.v = $3.v; errBit(); YYERROR; }
		if ($3.v == 0) { $$.v = $1.v; errBit(); YYERROR; }
		if (($$.v = op_push(op_force($1.v, GATE),
		    LOGC, op_not(op_force($3.v, GATE)))) != 0) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "lexpr", &$$);
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

lBlock	: LHEAD			{ ccfrag = '%'; }	/* %{ literal block %} */
	  CCFRAG '}'		{ $$.v = 0; }
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

cBlock	: '{'			{ ccfrag = '{'; }	/* ccfrag must be set */
	  CCFRAG '}'		{ $$ = $3; }		/* count dummy yacc token */
	;

	/************************************************************
	 *
	 * Clock and Timer parameters
	 *
	 * default clock is iClock if last clock parameter in list
	 *
	 * timer parameters must specify a delay, which is either
	 * number (NVAR), an Alias for a number or an immediate expression
	 * which is forced to ftype ARITH
	 *
	 * default timer delay is 1 if last clock parameter in list
	 *
	 ***********************************************************/

cref	: /* nothing */		{ $$.v = sy_push(iclock); }
	| ',' ctref		{ $$ = $2; }		/* clock or timer */
	;

ctref	: ctdref		{ $$ = $1; }		/* clock or timer with delay */
	| texpr			{			/* timer */
		Symbol *	sp;			/* with implicit delay of 1 */
		List_e *	lp;
		char		one[] = "1";
		if ((sp = lookup(one)) == 0) {
		    sp = install(one, NCONST, ARITH);	/* becomes NVAR */
		}
		lp = sy_push(sp);
		lp->le_val = (unsigned) -1;		/* mark link as timer value */
		$$.f = $1.f; $$.l = $1.l;
		$$.v = op_push($1.v, TIM, lp);
	    }
	;

ctdref	: cexpr			{ $$ = $1; }		/* clock */
	| texpr ','		{ dflag = 1; }		/* timer */
	  dexpr			{			/* with delay expression */
		$$.f = $1.f; $$.l = $4.l;
		$$.v = op_push($1.v, TIM, $4.v);
	    }
	;			/* TODO link instead - works for const NUMBER only */

dexpr	: NVAR			{
		Symbol *	sp = $1.v;
		while (sp->type == ALIAS) {
		    sp = sp->list->le_sym;	/* get token of original */
		}
		$$.v = sy_push(sp);
		$$.v->le_val = (unsigned) -1;	/* mark link as timer value */
	    }
	| aexpr			{
		$$ = $1;
		$$.v = op_force($1.v, ARITH);
		$$.v->le_val = (unsigned) -1;	/* mark link as timer value */
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

fexpr	: BLTIN1 '(' aexpr cref ')' {
		$$.f = $1.f; $$.l = $5.l;
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0);
		if (debug & 02) pu(1, "fexpr", &$$);
	    }
	| DLATCH '(' lexpr cref ')'	{		/* DL(set,reset) */
		$$.f = $1.f; $$.l = $5.l;
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0);
		if (debug & 02) pu(1, "fexpr", &$$);
	    }
	| BLTIN2 '(' aexpr ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $7.l;
		$$.v = bltin(&$1, &$3, &$6, &$5, 0, 0, 0);
		if (debug & 02) pu(1, "fexpr", &$$);
	    }
	| BLTIN2 '(' aexpr ',' ctdref ',' aexpr cref ')' {
		$$.f = $1.f; $$.l = $9.l;
		$$.v = bltin(&$1, &$3, &$5, &$7, &$8, 0, 0);
		if (debug & 02) pu(1, "fexpr", &$$);
	    }
	| BLTIN3 '(' aexpr ',' ctref ')'	{
		$$.f = $1.f; $$.l = $6.l;
		$$.v = bltin(&$1, &$3, 0, 0, 0, &$5, 0); /* monoflop without reset */
		if (debug & 02) pu(1, "fexpr", &$$);	/* set clocked by iClock */
	    }
	| BLTIN3 '(' aexpr ',' ctdref ',' ctref ')'	{
		$$.f = $1.f; $$.l = $8.l;
		$$.v = bltin(&$1, &$3, &$5, 0, 0, &$7, 0); /* monoflop without reset */
		if (debug & 02) pu(1, "fexpr", &$$);	/* set clocked by ext clock or timer */
	    }
	| BLTIN3 '(' aexpr ',' aexpr ',' ctref ')'	{
		$$.f = $1.f; $$.l = $8.l;
		$$.v = bltin(&$1, &$3, 0, &$5, 0, &$7, 0); /* monoflop with reset */
		if (debug & 02) pu(1, "fexpr", &$$);	/* set and reset clocked by iClock */
	    }
	| BLTIN3 '(' aexpr ',' aexpr ',' ctdref ',' ctref ')'	{
		$$.f = $1.f; $$.l = $10.l;
		$$.v = bltin(&$1, &$3, &$7, &$5, 0, &$9, 0); /* monoflop with reset */
		if (debug & 02) pu(1, "fexpr", &$$);	/* set and reset clocked by same clock or timer */
	    }
	| BLTIN3 '(' aexpr ',' ctdref ',' aexpr ',' ctref ')'	{
		Lis	lis1;
		$$.f = $1.f; $$.l = $10.l;
		lis1.v = sy_push(iclock);		/* to avoid shift reduce conflict */
		$$.v = bltin(&$1, &$3, &$5, &$7, &lis1, &$9, 0); /* monoflop with reset */
		if (debug & 02) pu(1, "fexpr", &$$);	/* set clocked by ext clock or timer */
	    }						/* reset clocked by iClock */
	| BLTIN3 '(' aexpr ',' ctdref ',' aexpr ',' ctdref ',' ctref ')'	{
		$$.f = $1.f; $$.l = $12.l;
		$$.v = bltin(&$1, &$3, &$5, &$7, &$9, &$11, 0); /* monoflop with reset */
		if (debug & 02) pu(1, "fexpr", &$$);	/* set clocked by ext clock or timer */
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
		fprintf(exoFP, cexeString[outFlag], ++c_number);
		fprintf(exoFP, "    if (_cexe_gf->gt_val < 0)\n");
	    }
	  cBlock		{			/* { x++; } */
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, &$7);
	    }
	;

ffexpr	: ifini				{		/* if (expr) { x++; } */
		fprintf(exoFP, "\n    return 0;\n%s", outFlag ? "}\n" : "");
	    }
	| ifini ELSE			{		/* { x++; } else */
		fprintf(exoFP, "\n    else\n");
	    }
	  cBlock			{		/* { x--; } */
		fprintf(exoFP, "\n    return 0;\n%s", outFlag ? "}\n" : "");
	    }
	| SWITCH '(' aexpr cref ')'	{		/* switch (expr) { case ...; } */
		fprintf(exoFP, cexeString[outFlag], ++c_number);
		fprintf(exoFP, "    switch (_cexe_gf->gt_new)\n");
	    }
	  cBlock		{			/* { x++; } */
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, &$7);
		fprintf(exoFP, "\n    return 0;\n%s", outFlag ? "}\n" : "");
	    }
	;

	/************************************************************
	 *
	 * Clock built in expression
	 *
	 ***********************************************************/

dcasgn	: decl '=' cexpr	{			/* dcasgn is NOT an cexpr */
		if ($1.v->ftype != CLCKL) {
		    error("assigning clock to variable declared differently:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		} else if ($1.v->type != UDF && $1.v->type != CLK && $1.v->type != ERR) {
		    error("assigning clock to variable assigned differently:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		}
		$$.v = op_asgn(&$1, &$3, CLCKL);
	    }
	;

casgn	: UNDEF '=' cexpr	{ $$.v = op_asgn(&$1, &$3, CLCKL); }
	| CVAR '=' cexpr	{
		if ($1.v->type != UDF) {
		    error("multiple assignment clock:", $1.v->name);
		}
		$$.v = op_asgn(&$1, &$3, CLCKL);
	    }
	;

cexpr	: CVAR			{ $$.v = sy_push($1.v); }
	| casgn			{ $$.v = sy_push($1.v); }
	| cfexpr		{
		Symbol *	sp = $1.v->le_sym;
		if (sp->ftype != ftypes[sp->type & TM]) {
		    warning("not enough arguments for function", sp->name);
		}
		sp->ftype = CLCKL;	/* clock list head */
		$$ = $1;
	    }
	| '(' cexpr ')'		{ $$ = $2; }
	;

cfexpr	: CBLTIN '(' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0);
	    }
	| CBLTIN '(' aexpr ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$6, &$5, 0, 0, 0);
	    }
	| CBLTIN '(' aexpr ',' ctdref ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$5, &$7, &$8, 0, 0);
	    }
	;

	/************************************************************
	 *
	 * Timer built in expression
	 *
	 ***********************************************************/

dtasgn	: decl '=' texpr	{			/* dtasgn is NOT an texpr */
		if ($1.v->ftype != TIMRL) {
		    error("assigning timer to variable declared differently:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		} else if ($1.v->type != UDF && $1.v->type != TIM && $1.v->type != ERR) {
		    error("assigning clock to variable assigned differently:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		}
		$$.v = op_asgn(&$1, &$3, TIMRL);
	    }
	;

tasgn	: UNDEF '=' texpr	{ $$.v = op_asgn(&$1, &$3, TIMRL); }
	| TVAR '=' texpr	{
		if ($1.v->type != UDF) {
		    error("multiple assignment timer:", $1.v->name);
		}
		$$.v = op_asgn(&$1, &$3, TIMRL);
	    }
	;

texpr	: TVAR			{ $$.v = sy_push($1.v); }
	| tasgn			{ $$.v = sy_push($1.v); }
	| tfexpr		{
		Symbol *	sp = $1.v->le_sym;
		if (sp->ftype != ftypes[sp->type & TM]) {
		    warning("not enough arguments for function", sp->name);
		}
		sp->ftype = TIMRL;	/* timer list head */
		$$ = $1;
	    }
	| '(' texpr ')'		{ $$ = $2; }
	;

	/************************************************************
	 *
	 * Timers (TIMER or T) have a preset off delay of 0.
	 * Such timers will clock with iClock on the falling edge of
	 * the master gate. They will clock with iClock on the rising
	 * edge if the on delay is 0.
	 *
	 ***********************************************************/

tfexpr	: TBLTIN '(' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, 0);
	    }
	| TBLTIN '(' aexpr ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$6, &$5, 0, 0, 0);
	    }
	| TBLTIN '(' aexpr ',' ctdref ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$5, &$7, &$8, 0, 0);
	    }

	/************************************************************
	 *
	 * Alternate timers (TIMER1 or T1) have a preset off delay of 1.
	 * Such timers will clock with the next timer pulse on the
	 * falling edge of the master gate. They wil clock with the
	 * next timer pulse on the rising edge if the on delay is 0.
	 *
	 ***********************************************************/

	| TBLTI1 '(' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0, &val1);
	    }
	| TBLTI1 '(' aexpr ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$6, &$5, 0, 0, &val1);
	    }
	| TBLTI1 '(' aexpr ',' ctdref ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$5, &$7, &$8, 0, &val1);
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
	 ***********************************************************/

funct	: UNDEF '(' params ')'	{
		$$.f = $1.f; $$.l = $4.l;
		assert($1.f[0] == '_' && $1.f[1] == '(' && $1.l[-1] == ')');
		$1.f[0] = $1.f[1] = $1.l[-1] = '#';
		if (lookup($1.v->name)) {	/* may be unlinked in same expr */
		    unlink_sym($1.v);		/* unlink Symbol from symbol table */
		    free($1.v);
		}
		$$.v = $3.v;
		if (debug & 02) pu(1, "funct", &$$);
	    }
	;

params	: /* nothing */		{ $$.v = 0; }
	| plist			{ $$   = $1; }
	;

plist	: aexpr			{
		$$.f = $1.f; $$.l = $1.l;
		if (($$.v = op_push(0, ARN, op_force($1.v, ARITH))) != 0) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "plist", &$$);
	    }
	| plist ',' aexpr	{
		$$.f = $1.f; $$.l = $3.l;
		if (($$.v = op_push($1.v, ARN, op_force($3.v, ARITH))) != 0) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "plist", &$$);
	    }
	;

	/************************************************************
	 *
	 * end of iC grammar
	 *
	 ***********************************************************/

%%

#define CBUFSZ 125			/* listing just fits on 132  cols */
#define YTOKSZ 66
static char	chbuf[CBUFSZ];
static char *	getp = chbuf;
static char *	fillp = chbuf;
static char	yytext[YTOKSZ];		/* lex token */
static int	yyleng;			/* length */
static char	inpBuf[YTOKSZ];		/* alternate file name */
static int	lineflag;
static char	tmpbuf[256];		/* buffer to build variable */
static char *	errFilename;
static int	errFlag = 0;
static int	errRet = 0;

int		lineno = 1;
int		c_number = 0;		/* case number for cexe.c */
int		outFlag = 0;		/* global flag for compiled output */
jmp_buf		begin;
int		lex_typ[] = { DEF_TYP };/* tokens corresponding to type */
int		lex_act[] = { DEF_ACT };/* tokens corresponding to ftype */

char *		inpNM = "stdin";	/* original input file name */
FILE *		inFP = 0;		/* input file pointer - stdin default */
FILE *		outFP;			/* listing file pointer */
FILE *		errFP;			/* error file pointer */
FILE *		exoFP;			/* cexe out file pointer */

/********************************************************************
 *
 *	compile an iC language source file whose name is in 'inpPath'
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
    char *	inpPath,		/* input file path */
    char *	lstNM,			/* list file name */
    char *	errNM,			/* error file name */
    char *	outNM,			/* C output file name */
    char *	exiNM,			/* cexe input file name */
    char *	exoNM)			/* cexe C output file name */
{
    if (lstNM && (outFP = fopen(lstNM, "w+")) == NULL) {
	return 3;
    }
    errFilename = errNM;		/* open on first error */
    if (inpPath && (inFP = fopen(inpNM = inpPath, "r")) == NULL) {
	return 1;
    }
    if ((exoFP = fopen(exoNM, "w+")) == NULL) {
	return 7;
    }
    outFlag = outNM != 0;	/* global flag for compiled output */
    init();		/* initialise symbol table */
    if (debug & 016) {	/* begin source listing */
	fprintf(outFP, "******* %-15s ************************\n", inpNM);
    }
    setjmp(begin);
    for (initcode(); yyparse(); initcode()) {
	;
    }
    if (debug & 016) {		/* end source listing */
	fprintf(outFP, "************************************************\n");
    }
    if (inpPath) fclose(inFP);
    return errRet;
} /* compile */

static int
get(void)
{
    int		temp;
    int		temp1;

    while (getp >= fillp) {
	if (lineflag) {
	    lineno++;
	}
	for (getp = fillp = chbuf; fillp < &chbuf[sizeof(chbuf)-1]
	    && (temp = getc(inFP)) != EOF
	    && (*fillp++ = temp) != '\n'; );
	if (fillp == chbuf) {
	    strcpy(chbuf, "*** EOF ***\n");
	    return (EOF);
	}
	*fillp = '\0';
	lineflag = (*(fillp-1) == '\n') ? 1 : 0;
	if (debug & 010) {	/* source listing in debugging output */
	    fprintf(outFP, "%03d\t%s", lineno, chbuf);
	    if (lineflag == 0) putc('\n', outFP);
	}
	/* handle pre-processor #line 1 "file.ic" */
	if (sscanf(getp, "#line %d \"%[^\"]\"\n", &temp1, inpBuf) == 2) {
	    lineno = temp1;
	    lineflag = 0;
	    inpNM = inpBuf;	/* another file name for messages */
	    getp = fillp;	/* do not pass to yylex() */
	}
    }
    yytext[yyleng++] = temp = *getp++;
    if (yyleng >= sizeof(yytext)) yyleng--;
    yytext[yyleng] = '\0';
    return temp;
} /* get */

static void
unget(char c)
{
    *--getp = c;		/* use always insures 1 free place */
    yytext[--yyleng] = '\0';
} /* unget */

char *	cexeString[] = {
    "    case %d:\n",
    "static int cexe_%d(Gate * _cexe_gf) {\n",
};

static int
yylex(void)
{
    int	c, c1;

    if (ccfrag) {
	if (ccfrag == '%') {
	    fprintf(exoFP, "%%{\n");	/* output "%{\n" now */
	}
	fprintf(exoFP, "#line %d \"%s\"\n", lineno, inpNM);
	unget('{');
	if (copyCfrag('{', ccfrag == '%' ? '%' : ';', '}') == 0) {
	    return 0;	/* copy C block or statement */
	}
	yylval.val.v = c_number;	/* return case # */
	ccfrag = 0;
	c = CCFRAG;
	goto retfl;
    }
    yyleng = 0;
    while ((c = get()) !=  EOF) {
	Symbol *	symp;
	List_e *	lp;
	if (c == ' ' || c == '\t' || c == '\n') {
	    yyleng = 0;
	    continue;			/* ignore white space */
	}
	if (isdigit(c)) {
	    char *	format;		/* number */
	    if (c == '0') {		/* oct or hex or 0 */
		if ((c = get()) == 'x') {
		    format = "0x%x%s";	/* hexadecimal */
		} else if (c == 'X') {
		    format = "0X%x%s";	/* hexadecimal */
		} else {
		    format = "%o%s";	/* octal */
		    unget(c);		/* may be a single 0 */
		}
	    } else {
		format = "%d%s";	/* decimal */
	    }
	    while ((c = get()) != EOF && isxdigit(c));
	    unget(c);
	    if (sscanf(yytext, format, &yylval.val.v, yytext) == 1) {
		if (dflag) {
		    if ((symp = lookup(yytext)) == 0) {
			symp = install(yytext, NCONST, ARITH); /* becomes NVAR */
		    }
		    if (symp->type == NCONST && symp->ftype == ARITH) {
			yylval.sym.v = symp;		/* return actual symbol */
			c = NVAR;			/* numeric symbol */
		    } else {
			c = NUMBER;			/* used in arith expression */
		    }
		} else {
		    c = NUMBER;				/* value in yylval.val.v */
		}
	    } else {
		c = LEXERR;
	    }
	    goto retfl;
	} else if (isalpha(c) || c == '_') {
	    unsigned char	wplus = 0;
	    unsigned int	qtoken = 0;
	    unsigned char	typ = UDF;
	    unsigned char	ftyp = UDFA;
	    unsigned char	y0[2];
	    unsigned char	y1[2];
	    int			yn;

	    while ((c = get()) != EOF && (isalnum(c) || c == '_'));
	    if (sscanf(yytext, "%1[IQT]%1[BWX]%d", y0, y1, &yn) == 3) {
		if (y1[0] == 'B' || y1[0] == 'W') {
		    wplus = 1;
		    goto foundQIT;
		} else if (c == '.') {
		    if (isdigit(c = get())) {	/* can only be QX%d. */
			while (isdigit(c = get()));
		    foundQIT:
			ftyp = GATE - wplus;		/* GATE or ARITH */
			if (y0[0] == 'Q') {
			    qtoken = lex_act[OUTX - wplus]; /* LOUT or AOUT */
			} else {
			    typ = INPX - wplus;		/* INPX or INPW */
			}
		    } else {
			unget(c);		/* the non digit, not '.' */
		    }
		} else if (sscanf(yytext, "%1[IQT]%1[BWX]%d__%d%1[A-Z_a-z]",
		    y0, y1, &yn, &yn, y1) == 4) {
		    warning("Variables with __ clash with I/O", yytext);
		    typ = ERR;			/* QX%d__%d not allowed */
		}				/* QX%d__%d_ABC is OK */
	    }
	    unget(c);
	    if ((symp = lookup(yytext)) == 0) {
		symp = install(yytext, typ, ftyp); /* usually UDF UDFA */
	    } else if (typ == ERR) {
		symp->type = ERR;		/* mark ERROR in QX%d__%d */
	    }
	    yylval.sym.v = symp;		/* return actual symbol */
	    while ((typ = symp->type) == ALIAS) {
		symp = symp->list->le_sym;	/* with token of original */
	    }
	    if (typ == KEYW) {
		c = symp->u.val;		/* reserved word */
	    } else if (qtoken) {
		c = qtoken;			/* LOUT or AOUT */
	    } else if (typ == ARNC || typ == LOGC || dflag && typ == NCONST) {
		c = lex_typ[symp->type & TM];	/* NCONST via ALIAS ==> NVAR */
	    } else {
		c = lex_act[symp->ftype];	/* alpha_numeric symbol */
	    }
	    if (symp->ftype == OUTW && (lp = symp->list) != 0) {
		symp = lp->le_sym;		/* original via backptr */
	    }
	    yylval.sym.f = stmtp;	/* original name for expressions */
	    yylval.sym.l = stmtp += sprintf(stmtp, "_(%s)", symp->name);
	} else {
	    c1 = get();
	    switch (c) {
	    case '!':
		if (c1 == '=') {
		    c = CMP; goto found;		/* != */
		}
	    case '~':
		c = NOTL;				/* ! or ~ */
		break;
	    case '%':
		if (c1 == '{') {
		    c = LHEAD; goto found;		/* %{ */
		} else if (c1 == '=') {
		    c = OPE; goto found;		/* %= */
		}
		c = AOP;				/* % */
		break;
	    case '&':
		if (c1 == '&') {
		    c = AA; goto found;			/* && */
		} else if (c1 == '=') {
		    c = OPE; goto found;		/* &= */
		}
		break;
	    case '*':
		if (c1 == '/') {
		    c = COMMENTEND;	/* dangling end of a comment */
		    goto found;
		} else if (c1 == '=') {
		    c = OPE; goto found;		/* *= */
		}
		break;
	    case '+':
		if (c1 == '+') {
		    c = PPMM; goto found;		/* ++ */
		} else if (c1 == '=') {
		    c = OPE; goto found;		/* += */
		}
		c = PM;					/* + */
		break;				/* + binary or unary */
	    case '-':
		if (c1 == '-') {
		    c = PPMM; goto found;		/* -- */
		} else if (c1 == '=') {
		    c = OPE; goto found;		/* -= */
		} else if (c1 == '>') {
		    c = PR; goto found;			/* -> */
		}
		c = PM;					/* - */
		break;				/* - binary or unary */
	    case '/':
		if (c1 == '/') {
		    do {		/* start C++ style comment */
			if ((c1 = get()) == EOF) return 0;
		    } while (c1 != '\n');
		} else if (c1 == '*') {
		    do {		/* start C style comment */
			while (c1 != '*') {
			    if ((c1 = get()) == EOF) return 0;
			}
		    } while ((c1 = get()) != '/');
		} else if (c1 == '=') {
		    c = OPE; goto found;		/* /= */
		} else {
		    c = AOP;				/* / */
		    break;
		}
		yyleng = 0;			/* end of comment */
		continue;
	    case '<':
		if (c1 == '<') {
		    if ((c1 = get()) == '=') {
			c = OPE; goto found;		/* <<= */
		    }
		    c = AOP;				/* << */
		    break;
		}
		if (c1 != '=') {
		    unget(c1);
		}
		c = CMP; goto found;			/* < or <= */
	    case '=':
		if (c1 == '=') {
		    c = CMP; goto found;		/* == */
		}
		*yytext = '#';	/* to recognise <= => transmogrify = */
		break;
	    case '>':
		if (c1 == '>') {
		    if ((c1 = get()) == '=') {
			c = OPE; goto found;		/* >>= */
		    }
		    c = AOP;				/* >> */
		    break;
		}
		if (c1 != '=') {
		    unget(c1);
		}
		c = CMP; goto found;			/* > or >= */
	    case '^':
		if (c1 == '=') {
		    c = OPE; goto found;		/* ^= */
		}
		break;
	    case '|':
		if (c1 == '|') {
		    c = OO; goto found;			/* || */
		} else if (c1 == '=') {
		    c = OPE; goto found;		/* |= */
		}
		break;
	    case '.':
		c = PR;					/* . */
		break;
	    }
	    unget(c1);
	found:
	    yylval.val.v = c;		/* return to yacc as is */
	retfl:
	    yylval.val.f = stmtp;	/* all sources are yylval.val */
	    if ((c == PM || c == PPMM) &&
		stmtp > yybuf && *(stmtp-1) == *yytext) {
		*stmtp++ = ' ';		/* space between + + and - - */
	    }
	    yylval.val.l = stmtp = strcpy(stmtp, yytext) + strlen(yytext);
	}
	dflag = 0;
	return c;			/* return token to yacc */
    }
    return (0);		/* EOF */
} /* yylex */

static void
errLine(void)			/* error file not openend if no errors */
{
    static int	errline = 0;

    if (errFlag == 0) {
	if (errFilename && errFP == stderr) {
	    if ((errFP = fopen(errFilename, "w+")) == NULL) {
		errFilename = 0;	/* cannot open errFilename */
		errFP = stderr;		/* just output to stderr */
		errRet = 2;		/* error return for compile() */
	    } else {
		errFlag = 1;		/* there is an error file */
		fprintf(errFP, "******* %-15s ************************\n", inpNM);
	    }
	}
	if (outFP != stdout) {
	    errFlag = 1;		/* listing is not to stdout */
	    if (!(debug & 016)) {	/* no source listing in debugging output */
		fprintf(outFP, "******* %-15s ************************\n", inpNM);
	    }
	}
    }
    if (lineno != errline) {
	errline = lineno;		/* dont print line twice */
	if (!(debug & 010)) {	/* no source listing in debugging output */
	    fprintf(outFP, "%03d\t%s", lineno, chbuf);
	    if (lineflag == 0) putc('\n', outFP);
	}
	if (errFlag) {
	    fprintf(errFP, "%03d\t%s", lineno, chbuf);
	    if (lineflag == 0)  putc('\n', errFP);
	}
    }
} /* errLine */

static void
errmess(				/* actual error message */
    char *	str0,
    char *	str1,
    char *	str2)
{
    errLine();
    fprintf(outFP, "*** %s: %s", str0, str1);
    if (errFlag) fprintf(errFP, "*** %s: %s", str0, str1);
    if (str2) {
	fprintf(outFP, " %s", str2);
	if (errFlag) fprintf(errFP, " %s", str2);
    } else {
	putc('.', outFP);
	if (errFlag) putc('.', errFP);
    }
    fprintf(outFP, " File %s, line %d\n", inpNM, lineno);
    if (errFlag) fprintf(errFP, " File %s, line %d\n", inpNM, lineno);
} /* errmess */

void
error(					/* print error message */
    char *	str1,
    char *	str2)
{
    iclock->type = ERR;			/* prevent execution */
    errmess("Error", str1, str2);
} /* error */

void
warning(				/* print warning message */
    char *	str1,
    char *	str2)
{
    errmess("Warning", str1, str2);
} /* warning */

static void
errBit(void)
{
    error("no constant allowed in bit expression", NULL);
} /* errBit */

static void
errInt(void)
{
    error("no imm variable to trigger arithmetic expression", NULL);
} /* errInt */

void
execerror(			/* recover from run-time error */
    char *	str1,
    char *	str2)
{
    errmess("Execerror", str1, str2);
    fseek(inFP, 0L, 2);	/* flush rest of file */
    longjmp(begin, 0);
} /* execerror */

static void
yyerror(char *	s)		/* called for yacc syntax error */
{
    char *	cp = chbuf;
    int		n, n1;
    char	erbuf[80];

    errLine();
    fprintf(outFP, "*** ");
    if (errFlag) fprintf(errFP, "*** ");
    for (n1 = 0, n = getp - cp - yyleng; n > 0; n--, cp++) {
	n1++;
	if (*cp == '\t') {
	    while (n1 % 8) {
		n1++;
	    }
	}
    }
    n = sprintf(erbuf, "%s %d in %s", s, ++ynerrs, inpNM);
    if (n1 < n + 5) {
	n1 += 4;
	while (n1--) {
	    putc('-', outFP);
	    if (errFlag) putc('-', errFP);
	}
	fprintf(outFP, "^ %s\n", erbuf);
	if (errFlag) fprintf(errFP, "^ %s\n", erbuf);
    } else {
	fprintf(outFP, "%s ", erbuf);
	if (errFlag) fprintf(errFP, "%s ", erbuf);
	n1 -= n - 3;
	while (n1--) {
	    putc('-', outFP);
	    if (errFlag) putc('-', errFP);
	}
	fprintf(outFP, "^\n");
	if (errFlag) fprintf(errFP, "^\n");
    }
} /* yyerror */

static int
copyCfrag(char s, char m, char e)
	/* copy C action to the next ; , or closing brace or bracket */
{
    int		brace;
    int		c;
    int		match;
    int		count;

    for (brace = 0; (c = get()) != EOF; ) {
	if (c == s) {			/* '{' or '(' */
	    if (brace++ == 0 && m == '%') {	/* don't output "%{\w" */
		while ((c = get()) == ' ' || c == '\t');
		unget(c);
		continue;
	    }
	} else if (c == m) {		/* ';' or '%' or ',' */
	    if (brace <= 0) {
		unget(c);		/* use for next token */
		return m;
	    } else if (brace == 1 && c == '%') {
		if ((c = get()) == '}') {
		    fprintf(exoFP, "\n%%}\n");	/* output "\n%}\n" */
		    unget(c);
		    return m;
		}
		unget(c);
		c = '%';
	    }
	} else if (c == e) {		/* ')' or '}' */
	    if (--brace <= 0) {
		if (brace < 0) {
		    unget(c);
		    return e;
		} else if (c == '}') {
		    putc(c, exoFP);	/* output '}' */
		    unget(c);
		    return e;
		}
	    }
	} else switch (c) {

	case '/':			/* look for comments */
	    putc(c, exoFP);
	    if ((c = get()) == '/') {
		do {			/* start C++ style comment */
		    putc(c, exoFP);
		    if ((c = get()) == EOF) goto eof_error;
		} while (c != '\n');
	    } else if (c == '*') {
		do {			/* start C style comment */
		    putc(c, exoFP);
		    while (c != '*') {
			if ((c = get()) == EOF) goto eof_error;
			putc(c, exoFP);
		    }
		} while ((c = get()) != '/');
	    } else {
		unget(c);
	    }
	    break;

	case '\'':			/* character constant */
	    match = '\'';
	    goto string;

	case '"':			/* character string */
	    match = '"';

	string:
	    putc(c, exoFP);
	    while ((c = get()) != match) {
		if (c == '\\') {
		    putc(c, exoFP);
		    if ((c = get()) == EOF) goto eof_error;
		} else if (c == '\n') {
		    yyerror("C code: newline in \" \" or ' ', error");
		} else if (c == EOF)  goto eof_error;
		putc(c, exoFP);
	    }
	    break;
	}
	putc(c, exoFP);			/* output to cexe.tmp */
    }
eof_error:
    yyerror("C code: EOF, error");
    return 0;
} /* copyCfrag */

%{ static const char comp_y[] =
"@(#)$Id: comp.y,v 1.15 2000/11/19 17:19:08 jw Exp $";
/********************************************************************
 *
 *	"comp.y"
 *	compiler for pplc
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<setjmp.h>

#include	"pplc.h"
#include	"comp.h"

/* "comp.y	3.70	95/02/03 Copyright (c) 1985-1993 by John E. Wulff" */

static int	get(void);
static void	unget(char);
static int	yylex(void);
static void	yyerror(char *);	/* called for yacc syntax error */
int		ynerrs;			/* count of yyerror() calls */
		/* NOTE yynerrs is reset for every call to yaccpar() */
static void	yyerrls(char *);	/* called for yacc error list */
static void	warn1(void);
static int	copyCfrag(char, char, char);	/* copy C action */
static char *	ccfrag;			/* flag for cexini CCFRAG syntax */
static char	ccbuf[32];		/* buffer for NUMBER CCFRAG */
static int	stflag;			/* record states of static */
unsigned int	stype;			/* to save TYPE in decl */
%}

%union {		/* stack type */
    Sym		sym;			/* symbol table pointer */
    Lis		list;			/* linked list elements */
    Val		val;			/* long numerical values */
    Str		str;			/* one character array */
    /* allows storing character as yylval.val.v (2nd byte is NULL) */
    /* then char array can be referenced as $1.v, single char as $1.v[0] */
}

%{

void
pu(int t, char * token, Lis * node)
{
    register char *	cp;
    register char *	ep;

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
} /* pu */

%}

%token	<sym>	UNDEF AVARC AVAR LVARC LVAR ACTION WACT XACT BLTIN1 BLTIN2
%token	<sym>	CVAR CBLTIN TVAR TBLTIN STATIC BLATCH BFORCE DLATCH
%token	<sym>	EXTERN IMM TYPE CKEYW IF ELSE SWITCH
%token	<val>	NUMBER CCNUMBER CCFRAG
%token	<str>	LEXERR COMMENTEND
%type	<sym>	stmt asgn wasgn xasgn casgn tasgn cstatic decl
%type	<list>	expr aexpr lexpr fexpr cexpr cfexpr texpr tfexpr ffexpr cref
%type	<val>	cblock ccexpr value declHead
%type	<str>	cbkini cstini cexini
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

stmt	: /* nothing */		{ stmtp = yybuf; $$.v = 0; }
	| stmt ';'		/* null statement */
	| stmt decl ';'		{ $$.v = $2.v; stmtp = yybuf; return (1); }
	| stmt decl error ';'	{ ($$.v = $2.v)->type = ERR; stmtp = yybuf; yyerrok; }
	| stmt cstatic ';'	{ $$.v = $2.v; stmtp = yybuf; return (1); }
	| stmt cstatic error ';'{ clk->type = ERR; stmtp = yybuf; yyerrok; }
	| stmt asgn ';'		{ $$.v = $2.v; stmtp = yybuf; return (1); }
	| stmt asgn error ';'	{ ($$.v = $2.v)->type = ERR; stmtp = yybuf; yyerrok; }
	| stmt wasgn ';'	{ $$.v = $2.v; stmtp = yybuf; return (1); }
	| stmt wasgn error ';'	{ ($$.v = $2.v)->type = ERR; stmtp = yybuf; yyerrok; }
	| stmt xasgn ';'	{ $$.v = $2.v; stmtp = yybuf; return (1); }
	| stmt xasgn error ';'	{ ($$.v = $2.v)->type = ERR; stmtp = yybuf; yyerrok; }
		/* op_asgn(0,...) returns 0 for missing slave gate in ffexpr */
	| stmt ffexpr '}'	{ $$.v = op_asgn(0, &$2, GATE); stmtp = yybuf; return (1); }
	| stmt ffexpr error '}'	{ $$.v = op_asgn(0, &$2, GATE); stmtp = yybuf; yyerrok; }
	| stmt ffexpr ';'	{ $$.v = op_asgn(0, &$2, GATE); stmtp = yybuf; return (1); }
	| stmt ffexpr error ';'	{ $$.v = op_asgn(0, &$2, GATE); stmtp = yybuf; yyerrok; }
	| stmt casgn ';'	{ $$.v = $2.v; return (1); }
	| stmt casgn error ';'	{ ($$.v = $2.v)->type = ERR; stmtp = yybuf; yyerrok; }
	| stmt tasgn ';'	{ $$.v = $2.v; return (1); }
	| stmt tasgn error ';'	{ ($$.v = $2.v)->type = ERR; stmtp = yybuf; yyerrok; }
	| stmt error ';'	{ clk->type = ERR; stmtp = yybuf; yyerrok; }
	;

decl	: declHead UNDEF	{
		$$.v = $2.v;
		$$.v->ftype = $1.v & 0xff;
		$$.v->type = $1.v >> 8;
	    }
	;

declHead			/* NOTE: stype gets type out of ftype */
	: TYPE			{		/* ARNC or LOGC */
		$$.v = stype = $1.v->ftype | types[$1.v->ftype] << 8;
	    }
	| IMM TYPE		{ $$.v = stype = $2.v->ftype; }
	| EXTERN IMM TYPE	{ $$.v = stype = $3.v->ftype; }
	| decl ','		{ $$.v = stype;	/* first TYPE */ }
	;

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
	| wasgn		{
		register List_e *	lp;
		$$.f = $1.f; $$.l = $1.l;
		if ((lp = $1.v->list) != 0) {
		    $$.v = sy_push(lp->le_sym);		/* output driver */
		} else {
		    error("output must be assigned before use:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		    $$.v = 0;
		}
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "aexpr", &$$);
	    }
	| xasgn		{
		register List_e *	lp;
		$$.f = $1.f; $$.l = $1.l;
		if ((lp = $1.v->list) != 0) {
		    $$.v = sy_push(lp->le_sym);		/* output driver */
		    $$.v->le_val = lp->le_val;		/* copy inversion */
		} else {
		    error("output must be assigned before use:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		    $$.v = 0;
		}
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "aexpr", &$$);
	    }
	;

wasgn	: WACT '=' aexpr	{
		Lis	li1;
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v == 0) { $$.v = 0; warn1(); YYERROR; }
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    error("multiple assignment to imm bit:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		}
		li1.v = op_force($3.v, ARITH);
		li1.f = $3.f;	/* for op_asgn */
		li1.l = $3.l;	/* for op_asgn */
		$1.v->list = sy_push(li1.v->le_sym);	/* create back link */
		li1.v = op_push((List_e *)0, UDF, li1.v);
		li1.v->le_sym->type = ARN;		/* type <== ARN */
		$$.v = op_asgn(&$1, &li1, OUTW);
	    }
	;

xasgn	: XACT '=' aexpr	{
		Lis	li1;
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v == 0) { $$.v = 0; warn1(); YYERROR; }
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    error("multiple assignment to imm bit:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		}
		li1.v = op_force($3.v, GATE);
		li1.f = $3.f;	/* for op_asgn */
		li1.l = $3.l;	/* for op_asgn */
		$1.v->list = sy_push(li1.v->le_sym);	/* create back link */
		$1.v->list->le_val = li1.v->le_val;	/* copy inversion */
		li1.v = op_push((List_e *)0, UDF, li1.v); /* type <== AND */
		li1.v->le_sym->type = OR;		/* type <== OR */
		$$.v = op_asgn(&$1, &li1, OUTX);
	    }
	;

asgn	: UNDEF '=' aexpr	{
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v == 0) { $$.v = 0; warn1(); YYERROR; }
		$1.v->ftype = GATE;
		$$.v = op_asgn(&$1, &$3, GATE);	/* UNDEF is default GATE */
		if (debug & 02) pu(0, "asgn", (Lis*)&$$);
	    }
	| LVAR '=' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v == 0) { $$.v = 0; warn1(); YYERROR; }
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    error("multiple assignment to imm bit:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		}
		$$.v = op_asgn(&$1, &$3, GATE);
		if (debug & 02) pu(0, "asgn", (Lis*)&$$);
	    }
	| AVAR '=' aexpr		{
		$$.f = $1.f; $$.l = $3.l;
		if ($3.v == 0) { $$.v = 0; warn1(); YYERROR; }
		if ($1.v->type != UDF && $1.v->type != ERR) {
		    error("multiple assignment to imm int:", $1.v->name);
		    $1.v->type = ERR;	/* cannot execute properly */
		}
		$$.v = op_asgn(&$1, &$3, ARITH);	/* arithmetic ALIAS */
		if (debug & 02) pu(0, "asgn", (Lis*)&$$);
	    }

expr	: UNDEF			{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = sy_push($1.v);
		$1.v->ftype = GATE;
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| NUMBER		{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = 0;			/* no node */
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| CCNUMBER		{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = 0;			/* no node */
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| LVAR			{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = sy_push($1.v);
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| XACT		{
		register List_e *	lp;
		$$.f = $1.f; $$.l = $1.l;
		if ((lp = $1.v->list) != 0) {
		    $$.v = sy_push(lp->le_sym);		/* output driver */
		    $$.v->le_val = lp->le_val;		/* copy inversion */
		} else {
		    /* output - modify later by following back link */
		    $$.v = sy_push($1.v);
		}
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| AVAR			{
		$$.f = $1.f; $$.l = $1.l;
		$$.v = sy_push($1.v);
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| WACT		{
		register List_e *	lp;
		$$.f = $1.f; $$.l = $1.l;
		if ((lp = $1.v->list) != 0) {
		    $$.v = sy_push(lp->le_sym);		/* output driver */
		} else {
		    /* output - modify later by applying generated macro */
		    $$.v = sy_push($1.v);
		}
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| fexpr			{
		register Symbol *	sp;

		if ($1.v == 0) YYERROR;			/* error in bltin() */
		sp = $1.v->le_sym;
		$$.f = $1.f; $$.l = $1.l;
		$$.v = $1.v;
		if (sp->ftype != ftypes[sp->type]) {
		    warning("not enough arguments for function", sp->name);
		}
		sp->ftype = sp->type == SH ? ARITH : GATE;
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "expr", &$$);
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
		if ($3.v == 0) { $$.v = 0; warn1(); YYERROR; }
		$$.v = op_push(op_force($3.v, GATE), LOGC, $5.v);
		$$.v->le_sym->type = LATCH;
		$$.v->le_first = $$.f; $$.v->le_last = $$.l;
		if (debug & 02) pu(1, "expr", &$$);
	    }
	| expr '|' expr		{		/* binary | */
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v == 0) {
		    $$.v = $3.v;
		    if ($$.v && $$.v->le_sym->ftype != ARITH) {
			warn1(); YYERROR;
		    }
		} else if ($3.v == 0) {
		    $$.v = $1.v;
		    if ($$.v->le_sym->ftype != ARITH) {
			warn1(); YYERROR;
		    }
		} else if ($1.v->le_sym->ftype == ARITH &&
		    $3.v->le_sym->ftype == ARITH) {
		    $$.v = op_push($1.v, ARN, $3.v);	/* bitwise | */
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
		if ($1.v == 0) {
		    $$.v = $3.v;
		    if ($$.v && $$.v->le_sym->ftype != ARITH) {
			warn1(); YYERROR;
		    }
		} else if ($3.v == 0) {
		    $$.v = $1.v;
		    if ($$.v->le_sym->ftype != ARITH) {
			warn1(); YYERROR;
		    }
		} else if ($1.v->le_sym->ftype == ARITH &&
		    $3.v->le_sym->ftype == ARITH) {
		    $$.v = op_push($1.v, ARN, $3.v);	/* bitwise ^ */
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
		if ($1.v == 0) {
		    $$.v = $3.v;
		    if ($$.v && $$.v->le_sym->ftype != ARITH) {
			warn1(); YYERROR;
		    }
		} else if ($3.v == 0) {
		    $$.v = $1.v;
		    if ($$.v->le_sym->ftype != ARITH) {
			warn1(); YYERROR;
		    }
		} else if ($1.v->le_sym->ftype == ARITH &&
		    $3.v->le_sym->ftype == ARITH) {
		    $$.v = op_push($1.v, ARN, $3.v);	/* bitwise & */
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
		register Symbol * sp;
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v &&
		    (sp = $1.v->le_sym)->ftype != ARITH &&
		    (sp->type > ARN || sp->type == UDF || !sp->u.blist) &&
		    (sp = $3.v->le_sym)->ftype != ARITH &&
		    (sp->type > ARN || sp->type == UDF || !sp->u.blist)) {
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
		register Symbol * sp;
		$$.f = $1.f; $$.l = $3.l;
		if ($1.v &&
		    (sp = $1.v->le_sym)->ftype != ARITH &&
		    (sp->type > ARN || sp->type == UDF || !sp->u.blist) &&
		    (sp = $3.v->le_sym)->ftype != ARITH &&
		    (sp->type > ARN || sp->type == UDF || !sp->u.blist)) {
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
		register Symbol * sp;
		$$.f = $1.f; $$.l = $2.l;
		if ($2.v) {
		    if ((sp = $2.v->le_sym)->ftype != ARITH &&
			((sp->type != ARNC && sp->type != ARN &&
			sp->type != SH) || sp->u.blist == 0)) {
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
		if ($1.v == 0) { $$.v = $3.v; warn1(); YYERROR; }
		if ($3.v == 0) { $$.v = $1.v; warn1(); YYERROR; }
		if (($$.v = op_push(op_force($1.v, GATE),
		    LOGC, op_not(op_force($3.v, GATE)))) != 0) {
		    $$.v->le_first = $$.f; $$.v->le_last = $$.l;
		}
		if (debug & 02) pu(1, "lexpr", &$$);
	    }
	;

value	: NUMBER		{ $$.v = $1.v;	/* terminates with , or ) */
		$$.f = $1.f; $$.l = $1.l;
		if ($$.v & 0x8000) {
		    warning("time value must be <= 32767", NULL);
		    $$.v = 32767;
		}
		if (debug & 02) pu(2, "value", (Lis*)&$$);
	    }
	| ccexpr		{ $$.v = -$1.v; }	/* case # */
	;

cblock	: cbkini CCFRAG		{ $$.v = $2.v; }	/* ccfrag must be set */
	| cexini		{ stflag |= 0x10; }
	  CCFRAG		{ $$.v = $3.v; }
	; 

cbkini	: '{' 			{ ccfrag = $1.v; }	/* '}' */
	| CKEYW			{ ccfrag = "{";		/* "}" */ }
	;

cstatic	: cstini CCFRAG		{ $$.v = 0; }	/* ccfrag must be set */
	; 

cstini	: STATIC 		{ ccfrag = "{"; stflag |= 0x01; }
	; 

ccexpr	: cexini CCFRAG		{ $$.v = $2.v; }	/* ccfrag must be set */
	; 				/* before CCFRAG is fetched */

cexini	: UNDEF			{
		$1.v->ftype = ARITH;	/* this Gate becomes AVARC */
		$1.v->type = ARNC;
		ccfrag = $1.v->name;
	    }
	| AVARC 		{ ccfrag = $1.v->name; }
	| LVAR	 		{ ccfrag = $1.v->name; }	/* ZZZ */
	| CCNUMBER		{ ccfrag = ccbuf; }	/* copy of yytext */
	| '(' 			{ ccfrag = $1.v; }	/* ')' */
	| '\'' 			{ ccfrag = $1.v; }
	| AOP 			{ ccfrag = $1.v; }	/* all unary */
	| PM 			{ ccfrag = $1.v; }
	| PPMM 			{ ccfrag = $1.v; }
	| '&' 			{ ccfrag = $1.v; }	/* address of */
	| '*' 			{ ccfrag = $1.v; }	/* pointer to */
	; 

cref	: /* nothing */		{ $$.v = sy_push(clk); }/* iClock */
	| ',' cexpr		{ $$.v = $2.v; }	/* other clock */
	| ',' texpr ',' value	{ $$.v = $2.v;		/* timer clock */
				  $$.v->le_val = $4.v; }/* time value */
	;

fexpr	: BLTIN1 '(' aexpr cref ')' {
		$$.f = $1.f; $$.l = $5.l;
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0);
		if (debug & 02) pu(1, "fexpr", &$$);
	    }
	| DLATCH '(' lexpr cref ')'	{		/* DL(set,reset) */
		$$.f = $1.f; $$.l = $5.l;
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0);
		if (debug & 02) pu(1, "fexpr", &$$);
	    }
	| BLTIN2 '(' aexpr ',' aexpr ')'	{
		$$.f = $1.f; $$.l = $6.l;
		$$.v = bltin(&$1, &$3, 0, &$5, 0, 0);
		if (debug & 02) pu(1, "fexpr", &$$);
	    }
	| BLTIN2 '(' aexpr ',' aexpr ',' cexpr ')' {
		$$.f = $1.f; $$.l = $8.l;
		$$.v = bltin(&$1, &$3, &$7, &$5, 0, 0);
		if (debug & 02) pu(1, "fexpr", &$$);
	    }
	| BLTIN2 '(' aexpr cref ',' aexpr ',' cexpr ')' {
		$$.f = $1.f; $$.l = $9.l;
		$$.v = bltin(&$1, &$3, &$4, &$6, &$8, 0);
		if (debug & 02) pu(1, "fexpr", &$$);
	    }
	| BLTIN2 '(' aexpr ',' texpr ',' value ')'	{
		$$.f = $1.f; $$.l = $8.l;
		$$.v = bltin(&$1, &$3, 0, 0, &$5, &$7); /* monoflop without reset */
		if (debug & 02) pu(1, "fexpr", &$$);	/* set clocked by iClock */
	    }
	| BLTIN2 '(' aexpr ',' aexpr ',' texpr ',' value ')'	{
		$$.f = $1.f; $$.l = $10.l;
		$$.v = bltin(&$1, &$3, 0, &$5, &$7, &$9); /* monoflop with reset */
		if (debug & 02) pu(1, "fexpr", &$$);	/* set and reset clocked by iClock */
	    }
	| BLTIN2 '(' aexpr cref ',' texpr ',' value ')'	{
		$$.f = $1.f; $$.l = $9.l;
		$$.v = bltin(&$1, &$3, &$4, 0, &$6, &$8); /* monoflop without reset */
		if (debug & 02) pu(1, "fexpr", &$$);	/* set clocked by ext clock or timer */
	    }
	| BLTIN2 '(' aexpr ',' aexpr cref ',' texpr ',' value ')'	{
		$$.f = $1.f; $$.l = $11.l;
		$$.v = bltin(&$1, &$3, &$6, &$5, &$8, &$10); /* monoflop with reset */
		if (debug & 02) pu(1, "fexpr", &$$);	/* set clocked by ext clock or timer */
	    }						/* reset clocked by iClock */
	;

/* no assignment allowed for ffexpr - they stand alone */

ffexpr	: IF '(' aexpr cref ')' cblock {		/* if (expr) { x++; } */
		$$.v = bltin(&$1, &$3, &$4, 0, 0, &$6);
	    }
	| IF '(' aexpr cref ')' cblock '}' ELSE cblock {
		$$.v = bltin(&$1, &$3, &$4, 0, 0, &$6);
	    }
	| SWITCH '(' aexpr cref ')' cblock {
		$$.v = bltin(&$1, &$3, &$4, 0, 0, &$6);
	    }
	;

casgn	: UNDEF '=' cexpr	{ $$.v = op_asgn(&$1, &$3, CLCKL); }
	| CVAR '=' cexpr	{ $$.v = op_asgn(&$1, &$3, CLCKL);
		  warning("multiple assignment clock:", $1.v->name); }
	;

cexpr	: CVAR			{ $$.v = sy_push($1.v); }
	| casgn			{ $$.v = sy_push($1.v); }
	| cfexpr		{
		register Symbol *	sp = $1.v->le_sym;
		if (sp->ftype != ftypes[sp->type]) {
		    warning("not enough arguments for function", sp->name);
		}
		sp->ftype = CLCKL;	/* clock list head */
		$$.v = $1.v;
	    }
	| '(' cexpr ')'		{ $$.v = $2.v; }
	;

cfexpr	: CBLTIN '(' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0);
	    }
	| CBLTIN '(' aexpr ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$6, &$5, 0, 0);
	    }
	| CBLTIN '(' aexpr cref ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$4, &$6, &$7, 0);
	    }
	;

tasgn	: UNDEF '=' texpr	{ $$.v = op_asgn(&$1, &$3, TIMRL); }
	| TVAR '=' texpr	{ $$.v = op_asgn(&$1, &$3, TIMRL);
		  warning("multiple assignment timer:", $1.v->name); }
	;

texpr	: TVAR			{ $$.v = sy_push($1.v); }
	| tasgn			{ $$.v = sy_push($1.v); }
	| tfexpr		{
		register Symbol *	sp = $1.v->le_sym;
		if (sp->ftype != ftypes[sp->type]) {
		    warning("not enough arguments for function", sp->name);
		}
		sp->ftype = TIMRL;	/* timer list head */
		$$.v = $1.v;
	    }
	| '(' texpr ')'		{ $$.v = $2.v; }
	;

tfexpr	: TBLTIN '(' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$4, 0, 0, 0);
	    }
	| TBLTIN '(' aexpr ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$6, &$5, 0, 0);
	    }
	| TBLTIN '(' aexpr cref ',' aexpr cref ')'	{
		$$.v = bltin(&$1, &$3, &$4, &$6, &$7, 0);
	    }
	;

%%	/* end of grammar */

int		lineno = 1;
int		c_number = 0;		/* case number for cexe.c */
int		outFlag = 0;		/* global flag for compiled output */
jmp_buf		begin;
int		lex_typ[] = { DEF_TYP };/* tokens corresponding to type */
int		lex_act[] = { DEF_ACT };/* tokens corresponding to ftype */

char *		inpNM = "standard input";/* original input file name */
FILE *		inFP;			/* input file pointer */
FILE *		outFP;			/* listing file pointer */
FILE *		errFP;			/* error file pointer */
FILE *		exiFP;			/* cexe in file pointer */
FILE *		exoFP;			/* cexe out file pointer */

/********************************************************************
 *
 *	compile an L language source file whose name is in 'szFile_g'
 *	if szFile_g is a null pointer use standard input (stdin)
 *	a copy of the source file name is kept in inpNM for
 *	error messages. This name and the variable 'lineno' are
 *	modified by a #line "file.p" 1 pre-processor directive.
 *
 *	listing text is directed to 'lstNM' (default stdout)
 *	error   text is directed to 'errNM' (default stderr)
 *
 *	a prototype for C function execution is read and extended
 *	with the C fragments possibly contained in the L program
 *
 *******************************************************************/

int
compile(char *lstNM, char *errNM, char *outNM, char *exiNM, char *exoNM)
{
    int	c;

    if (lstNM && (outFP = fopen(lstNM, "w+")) == NULL) {
	return 3;
    }
    if (errNM && (errFP = fopen(errNM, "w+")) == NULL) {
	return 2;
    }
    if (szFile_g && (inFP = fopen(inpNM = szFile_g, "r")) == NULL) {
	return 1;
    }
    if ((exoFP = fopen(exoNM, "w+")) == NULL) {
	return 6;
    }
    if (exiNM != 0) {
	if ((exiFP = fopen(exiNM, "r")) == NULL) {
	    return 5;
	}
	/* copy C execution file Part 1 from beginning up to 'V' */
	while ((c = getc(exiFP)) != 'V') {
	    if (c == EOF) {
		return 5;	/* unexpected end of exiNM */
	    }
	    putc(c, exoFP);
	}
    }
    outFlag = outNM != 0;	/* global flag for compiled output */
    init();		/* initialise symbol table */
    if (debug & 010) {	/* begin source listing */
	fprintf(outFP, "******* %-15s ************************\n", inpNM);
    }
    setjmp(begin);
    for (initcode(); yyparse(); initcode()) {
	;
    }
    if (debug & 010) {		/* end source listing */
	fprintf(outFP, "************************************************\n");
    }
    if (exiNM != 0) {
	/* copy C execution file Part 2 from character after 'V 'up to EOF */
	while ((c = getc(exiFP)) != EOF) {
	    putc(c, exoFP);
	}
	fclose(exiFP);
    }
    if (szFile_g) fclose(inFP);
    return 0;
} /* compile */

#define CBUFSZ 125			/* listing just fits on 132 */
#define YTOKSZ 66
static char	chbuf[CBUFSZ];
static char *	getp = chbuf;
static char *	fillp = chbuf;
static char	yytext[YTOKSZ];		/* lex token */
static int	yyleng;			/* length */
static char	inpBuf[YTOKSZ];		/* alternate file name */
static long	outBP = -1;		/* position in listing file */
static int	lineflag;
static char	yybuf[1024];		/* buffer to build imm statement */
static char	tmpbuf[256];		/* buffer to build variable */
static char *	stmtp = yybuf;		/* pointer into yybuf */

static int
get(void)
{
    register int	temp;
    int			temp1;

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
	    if (lineflag == 0) {
		putc('\n', outFP);
	    }
	    outBP = ftell(outFP);	/* note position in listing file */
	}
	/* handle pre-processor #line 1 "file.p" */
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
    "static int cexe_%d(void) {\n",
};

static int
yylex(void)
{
    int	c, c1;

    if (ccfrag) {
	if ((stflag & 0x05) == 0x01) {
	    if ((stflag & 0x2) == 0) {
		fprintf(exoFP, "#line %d \"%s\"\n", lineno, inpNM);
	    }
	    stflag |= 0x02;		/* set bit 1 */
	} else {
	    stflag |= 0x04;		/* set bit 2 */
	    if ((stflag & 0x08) == 0) {
		fprintf(exoFP, cexeString[outFlag], ++c_number);
	    }
	    fprintf(exoFP, "#line %d \"%s\"\n", lineno, inpNM);
	    if (*ccfrag != '{') {
		fprintf(exoFP, "    return ");
	    }
	}
	if (stflag & 0x01) {
	    fprintf(exoFP, "	static");
	} else if (strlen(ccfrag) == 1) {
	    unget(*ccfrag);	/* takes care of ( and ' at start */
	} else {
	    fprintf(exoFP, ccfrag);
	}
	if (*ccfrag == '{' || (stflag & 0x10)) {
	    if (copyCfrag('{', ';', '}') == 0) {
		return 0;	/* copy C block or statement */
	    }
	    if (stflag & 0x01) {
		stflag &= ~0x01;		/* clear bit 0 */
		if (stflag & 0x04) {
		    stflag |= 0x08;		/* set bit 3 */
		}
	    } else {
		if ((stflag & 0x10) == 0) {
		    fprintf(exoFP, " return 0");
		}
		stflag &= ~0x18;		/* clear bit 3 and 4 */
	    }
	} else {
	    if (copyCfrag('(', ',', ')') == 0) {
		return 0;	/* copy C expression */
	    }
	    stflag &= ~0x08;			/* clear bit 3 */
	}
	fprintf(exoFP, ";\n");			/* terminate case */
	if (outFlag != 0) {
	    fprintf(exoFP, "}\n\n");		/* terminate function */
	}
	yylval.val.v = c_number;		/* return case # */
	ccfrag = 0;
	c = CCFRAG;
	goto retfl;
    }
    yyleng = 0;
    while ((c = get()) !=  EOF) {
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
	    if (c != ',' && c != ')') {
		strncpy(ccbuf, yytext, sizeof ccbuf - 1); /* for cblock */
		c = CCNUMBER;
	    } else if (sscanf(yytext, format, &yylval.val.v, yytext) == 1) {
		c = NUMBER;
	    } else {
		c = LEXERR;
	    }
	    goto retfl;
	} else if (isalpha(c) || c == '_') {
	    register Symbol *	symp;
	    register List_e *	lp;
	    uchar		wplus = 0;
	    uchar		typ = UDF;
	    uchar		ftyp = UDFA;
	    uchar		y0[2];
	    uchar		y1[2];
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
			if (y0[0] == 'Q') {
			    ftyp = OUTX - wplus;	/* OUTX or OUTW */
			} else {
			    typ = INPX - wplus;		/* INPX or INPW */
			    ftyp = GATE - wplus;	/* GATE or ARITH */
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
	    while (symp->type == ALIAS) {
		symp = symp->list->le_sym;	/* with token of original */
	    }
	    if (symp->type == KEYW) {
		c = symp->u.val;		/* reserved word */
	    } else if ((symp->type & ~0x02) == 0x01){
		c = lex_typ[symp->type];	/* ARNC or LOGC */
	    } else {
		c = lex_act[symp->ftype];	/* alpha_numeric symbol */
	    }
	    if (symp->ftype == OUTW && (lp = symp->list) != 0) {
		symp = lp->le_sym;		/* original via backptr */
	    }
	    yylval.sym.f = stmtp;	/* original name for expressions */
	    yylval.sym.l = stmtp += sprintf(stmtp, "_(%s)", (LPSTR)symp->name);
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
		if (c1 == '=') {
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
	return c;			/* return token to yacc */
    }
    return (0);		/* EOF */
} /* yylex */

static void
errmess(				/* actual error message */
    char *	str0,
    char *	str1,
    char *	str2)
{
    if (ftell(outFP) != outBP) {
	fprintf(outFP, "%03d\t%s", lineno, chbuf);
	if (lineflag == 0) {
	    putc('\n', outFP);
	}
    }
    fprintf(outFP, "*** %s: %s", str0, str1);
    if (str2) {
	fprintf(outFP, " %s", str2);
    }
    fprintf(outFP, ". File %s, line %d\n", inpNM, lineno);
} /* errmess */

void
error(					/* print error message */
    char *	str1,
    char *	str2)
{
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
warn1(void)
{
    warning("only immediate variables allowed", NULL);
} /* warn1 */

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
yyerror(register char * s)	/* called for yacc syntax error */
{
    char *	cp = chbuf;
    int		n, n1;
    char	erbuf[80];

    if (ftell(outFP) != outBP) {
	fprintf(outFP, "%03d\t%s", lineno, chbuf);
	if (lineflag == 0) {
	    putc('\n', outFP);
	}
    }
    fprintf(outFP, "*** ");
    if (errFP != stderr) {
	fprintf(errFP, "%03d\t%s", lineno, chbuf);
	fprintf(errFP, "*** ");
    }
    for (n1 = 0, n = getp - cp - yyleng; n > 0; n--, cp++) {
	n1++;
	if (*cp == '\t') {
	    while (n1 % 8) {
		n1++;
	    }
	}
    }
    n = sprintf(erbuf, "%s %d in %s", (LPSTR)s, ++ynerrs, (LPSTR)inpNM);
    if (n1 < n + 5) {
	n1 += 4;
	while (n1--) {
	    putc('-', outFP);
	    if (errFP != stderr) putc('-', errFP);
	}
	fprintf(outFP, "^ %s\n", erbuf);
	if (errFP != stderr) fprintf(errFP, "^ %s\n", erbuf);
    } else {
	fprintf(outFP, "%s ", erbuf);
	if (errFP != stderr) fprintf(errFP, "%s ", erbuf);
	n1 -= n - 3;
	while (n1--) {
	    putc('-', outFP);
	    if (errFP != stderr) putc('-', errFP);
	}
	fprintf(outFP, "^\n");
	if (errFP != stderr) fprintf(errFP, "^\n");
    }
} /* yyerror */

static void
yyerrls(register char * s)	/* called for yacc error list */
{
    fprintf(outFP, s);
    if (errFP != stderr) fprintf(errFP, s);
} /* yyerrls */

static int
copyCfrag(char s, char m, char e)
	/* copy C action to the next ; , or closing brace or bracket */
{
    int		brace;
    int		c;
    int		match;

    for (brace = 0; (c = get()) != EOF; putc(c, exoFP)) {
	if (c == s) {			/* '{' or '(' ")}" */
	    brace++;
	} else if (c == m) {		/* ';' or ',' */
	    if (brace <= 0) {
		unget(c);		/* use for next token */
		return m;
	    }
	} else if (c == e) {		/* "{(" ')' or '}' */
	    if (--brace <= 0) {
		if (brace < 0) {
		    unget(c);
		    return e;
		} else if (c == '}') {
		    putc(c, exoFP);
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
    }
eof_error:
    yyerror("C code: EOF, error");
    return 0;
} /* copyCfrag */

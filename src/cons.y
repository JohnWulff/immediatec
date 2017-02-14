%{ static const char cons_y[] =
"@(#)$Id: cons.y 1.1 $";
/********************************************************************
 *
 *	Copyright (C) 2016  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	cons.y
 *
 *  Auxiliary grammar to evaluate constant expression strings for the
 *  immcc compiler at compile time.
 *
 *  Constant expression strings are generated when merging the output
 *  of function blocks returning one or more parameter values in an
 *  arithmetic expression possibly containing constants. When calling
 *  such a function block with those parameters as constants, the output
 *  is a constant expression. eg:
 *
 *	imm int increment(int x) { this = x + 1; }
 *
 *	    increment@x A ---+  increment@ A       increment@x // 1
 *						   +1;         // (2)
 *
 *	imm int setpoint = increment(10);
 *
 *  Such a constant expression does not have a trigger by an immediate
 *  variable to force its evaluation.
 *
 *  Previous versions of the compiler provided a dummy immediate
 *  variable 'iConst' to trigger constant expressions.
 *
 *	    iConst  A ---+  setpoint A             10+1;      // (3)
 *	    *** C Code ***	(3) 	return 10+1;
 *
 *  'iConst' fired the execution of the C code compiled for the constant
 *  expression during startup initialisation, which then gave the node
 *  associated with the constant expression its arithmetic value, which
 *  never changed.  The value of such an initialised constant variable
 *  was not available at compile time. Consequently such a constant
 *  could not be used as a constant expression to initialise immC
 *  variables or to index immC arrays.
 *
 *  To overcome this deficiency the new immcc compiler does not generate
 *  the above C code triggered by 'iConst' but instead uses a function
 *  int parseConstantExpression(char * expressionText) using this grammar
 *  to provide an int value to initalise a proper type NCONST constant
 *  with a numeric value in the main grammar. This is done for all
 *  constant expressions.
 *
 *  This auxiliary grammar allows all iC arithmetic operators for constant
 *  expressions. This is good because some constants are returned by
 *  function blocks and other are macros defined in the command line.
 *  This is probably overkill, but it avoids unnecessary error messages.
 *
 *******************************************************************/

#include	<limits.h>
#include	<stdio.h>
#include	<assert.h>
#include	"cons.tab.h"

#define STX	'\x02'
#define ETX	'\x03'
#ifndef	TESTCONS
extern void warning(char *, char *);	/* print warning message */
extern void iCerror(const char * s);	/* use full yyerror in comp.y */
#else	/* TESTCONS */
static void	warning (const char * warnMsg, const char * msg);
#endif	/* TESTCONS */
static void	yyerror (int * retValue, const char * msg);
static int	yylex(void);
static char *	in;
static char *	inp;
static int	supressErrorMsg = 0;
static int	stxFlag = 0;

/********************************************************************
 *
 *	Parse a constant expression text and return its numerical value
 *		in int* valp
 *
 *	return	0 if parsing was successful (constant numeric expression)
 *		1 if non numeric and r == 0
 *		if r != 0 suppress yacc syntax (and other) error messages
 *
 *******************************************************************/

int
parseConstantExpression(char * expressionText, int * valp, int r)
{
    in = inp = expressionText;		/* ready for yylex */
    supressErrorMsg = r;
    *valp = 0;
    return yyparse(valp);
} /* parseConstantExpression */

/* use default YYSTYPE int */
%}

%parse-param {int * retValue}

%token	NUMBER
%token	'(' ')'
%right	'?' ':'			/* ? : */
%left	OO			/* || */
%left	NN 			/* && */
%left	'|'			/* bitwise or */
%left	'^'			/* bitwise exclusive or */
%left	'&'			/* bitwise and */
%left	EQ NE			/* equality operators == != */
%left	'<' LE '>' GE		/* comparison operators < <= > >= */
%left	LEFT_SH RIGHT_SH	/* arithmetic operators << >> */
%left	'+' '-'			 /* arithmetic operators + - */
%left	'*' '/' '%'		/* arithmetic operators * / % */
%left	'!' '~'			/* unary ! ~ + - */

%%
constValue
	: constExpr		{ *retValue = $1; }
	;

constExpr
	: NUMBER		{ $$ = $1; }
	| constExpr '*' constExpr		{	/* * */
		$$ = $1 * $3;
	    }
	| constExpr '/' constExpr		{	/* / */
		if ($3 == 0) {
		    warning("division by zero", in);
		    $$ = (unsigned int)($1 >= 0 ? INT_MAX : INT_MIN);
		} else {
		    $$ = $1 / $3;
		}
	    }
	| constExpr '%' constExpr		{	/* % */
		if ($3 == 0) {
		    warning("remainder of division by zero", in);
		    $$ = 0;
		} else {
		    $$ = $1 % $3;
		}
	    }
	| constExpr '+' constExpr		{	/* binary + */
		$$ = $1 + $3;
	    }
	| constExpr '-' constExpr		{	/* binary - */
		$$ = $1 - $3;
	    }
	| constExpr LEFT_SH constExpr		{	/* << */
		$$ = $1 << $3;
	    }
	| constExpr RIGHT_SH constExpr		{	/* >> */
		$$ = $1 >> $3;
	    }
	| '+' constExpr %prec '!'		{	/* unary + */
		$$ = $2;
	    }
	| '-' constExpr %prec '!'		{	/* unary - */
		$$ = - $2;
	    }
	| '(' constExpr ')'			{
		$$ = $2;
	    }
	/********************************************************************
	 * Additional operators, which are not normally considered for
	 * constant expressions, but which may occurr in a function block
	 * and might as well be evaluated rather than causing an error.
	 * (uses exact C precedence to be compatible with other C expressions)
	 *******************************************************************/
	| constExpr '&' constExpr		{	/* & */
		$$ = $1 & $3;
	    }
	| constExpr '^' constExpr		{	/* ^ */
		$$ = $1 ^ $3;
	    }
	| constExpr '|' constExpr		{	/* | */
		$$ = $1 | $3;
	    }
	| constExpr '<' constExpr		{	/* < */
		$$ = $1 < $3;
	    }
	| constExpr LE  constExpr		{	/* <= */
		$$ = $1 <= $3;
	    }
	| constExpr '>' constExpr		{	/* > */
		$$ = $1 > $3;
	    }
	| constExpr GE  constExpr		{	/* >= */
		$$ = $1 >= $3;
	    }
	| constExpr EQ  constExpr		{	/* == */
		$$ = $1 == $3;
	    }
	| constExpr NE  constExpr		{	/* != */
		$$ = $1 != $3;
	    }
	| constExpr NN  constExpr		{	/* && */
		$$ = $1 && $3;
	    }
	| constExpr OO  constExpr		{	/* || */
		$$ = $1 || $3;
	    }
	| constExpr '?' constExpr ':' constExpr	{	/* ? : */
		$$ = $1 ? $3 : $5;
	    }
	| constExpr '?' ':' constExpr		{	/* ?: */
		$$ = $1 ? : $4;
	    }
	| '!' constExpr				{	/* unary arithmetic ! */
		$$ = ! $2;
	    }
	| '~' constExpr				{	/* unary bitwise ~ */
		$$ = ~ $2;
	    }
	/********************************************************************
	 * A comma ',' expression is split by iC and is never returned as an
	 * arithmetic text expression by an iC function block.
	 *******************************************************************/
	;

	/************************************************************
	 *
	 * end of constant expression grammar
	 *
	 ***********************************************************/
%%

#include	<stdlib.h>
#include	<ctype.h>

/********************************************************************
 *
 *	Get a number, which may be decimal, octal or hexadecimal.
 *	Use this simpler version with different parameters rather
 *	than the very different version in the main grammar.
 *
 *******************************************************************/

static long
getNumber(const char * numStr, int * len)
{
    long	value;
    char *	ep;

    value = strtol(numStr, &ep, 0);		/* convert to long */
    *len  = ep - numStr;
    return value;
} /* getNumber */

/********************************************************************
 *
 *	Lexer taylored to this simple grammar
 *
 *******************************************************************/

static int
yylex(void)
{
    int		c;
    int		c1;
    int		l;

    while ((c = *inp++) != 0) {
	if (isspace(c) || c == '#') continue;	/* ignore white space and # */
	if (c == STX) {
	    stxFlag++;
	    continue;
	}
	if (c == ETX) {
	    stxFlag--;
	    continue;
	}
	if (stxFlag > 0) {
	    continue;				/* ignore text bracketed by STX ETX */
	}
	assert(stxFlag == 0);			/* never should go -ve */
	if (isdigit(c)) {
	    yylval = (int)getNumber(--inp, &l); /* must be at least a single 0 */
	    inp += l;				/* step past number */
	    c = NUMBER;				/* decimal octal or hex */
	    goto found;
	}
	c1 = *inp++;
	switch (c) {
	case '!':
	    if (c1 == '=') {
		c = NE; goto found; 		/* != */
	    }
	    break;				/* ! */
	case '&':
	    if (c1 == '&') {
		c = NN; goto found;		/* && */
	    }
	    break;				/* & */
	case '|':
	    if (c1 == '|') {
		c = OO; goto found;		/* || */
	    }
	    break;				/* | */
	case '<':
	    if (c1 == '<') {
		c = LEFT_SH; goto found;	/* << */
	    }
	    if (c1 == '=') {
		c = LE; goto found;		/* <= */
	    }
	    break;				/* < */
	case '>':
	    if (c1 == '>') {
		c = RIGHT_SH; goto found;	/* >> */
	    }
	    if (c1 == '=') {
		c = GE; goto found;		/* >= */
	    }
	    break;				/* > */
	case '=':
	    if (c1 == '=') {
		c = EQ; goto found;		/* == */
	    }
	    break;				/* = not used */
	}
	--inp;
      found:
	return c;				/* ascii character or yacc token */
    }
    return 0;					/* end of string terminates parse */
} /* yylex */
#ifndef	TESTCONS

/********************************************************************
 *
 *	Since yyerror for this parser has an extra int return value
 *	must use this wrapper to call the full yyerror() for immcc
 *	without retValue, which is useless for syntax error messages.
 *
 *******************************************************************/

static void
yyerror (int * retValue, const char * msg)
{
    if (supressErrorMsg == 0) {
	iCerror("cons syntax error");				/* use full yyerror in comp.y */
    }
} /* yyerror */
#else	/* TESTCONS */

/********************************************************************
 *
 *	Main program for testing this parser
 *
 *******************************************************************/

static char *	iC_progname;		/* name of this executable */
static const char *	usage =
"Usage: %s constant_expression_text [...]\n"
"        -h              this help text\n"
"Copyright (C) 2016 John E. Wulff     <immediateC@gmail.com>\n"
;

int
main(int argc, char **argv)
{
    int		value;
    int		n = 0;

    iC_progname = *argv;
    if (argc <= 1) goto error;
    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
		case 'h':
		case '?':
		error:
		    printf(usage, iC_progname);
		    exit(1);
		default:
		    printf("WARNING: %s: unknown option -%c\n", iC_progname, **argv);
		    break;
		}
	    } while (*++*argv);
	} else {
	    if (parseConstantExpression(*argv, &value, 0)) {
		warning("not a constant expression", *argv);	/* parse error */
	    } else {
		printf("%d	'%s' ==> %d\n", ++n, *argv, value);
	    }
	}
    }
    return 0;
} /* main */

/********************************************************************
 *
 *	Warning function for test version - use full Warning for immcc
 *
 *******************************************************************/

static void
warning (const char * warnMsg, const char * msg)
{
    printf("*** Warning: %s: %s\n", warnMsg, msg);
} /* warning */

/********************************************************************
 *
 *	Called from yacc parser on error (mostly syntax error)
 *
 *******************************************************************/

static void
yyerror (int * retValue, const char * msg)
{
    printf("*** %s for return value %d\n", msg, *retValue);
} /* yyerror */
#endif	/* TESTCONS */

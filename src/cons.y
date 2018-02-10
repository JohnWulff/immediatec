%{ static const char cons_y[] =
"@(#)$Id: cons.y 1.5 $";
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
 *  function blocks and others are macros defined in the command line.
 *  This is probably overkill, but it avoids unnecessary error messages.
 *
 *******************************************************************/

#include	<limits.h>
#include	<stdio.h>
#include	<assert.h>
#include	"comp.h"
#include	"cons.tab.h"

#define STX	'\x02'
#define ETX	'\x03'
#ifdef	TESTCONS
static int	suppressWarning = 0;		/* output warnings in getNumber() for test compiler */
int		iC_Wflag = W_DEPRECATED_LOGIC;
#else	/* ! TESTCONS */
static int	suppressWarning = 1;		/* suppress warnings for constant expression compiler */
extern void	iCerror(const char * s);	/* use full yyerror in comp.y */
#endif	/* TESTCONS */
static void	yyerror (int * retValue, const char * msg);
static int	yylex(void);
static char *	in;
static char *	inp;
static int	suppressErrorMsg = 0;
static int	stxFlag = 0;

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
#include	<string.h>
#include	<ctype.h>

/********************************************************************
 *
 *	Parse a constant expression text and return its numerical value
 *		in int* valp
 *
 *	return	0 if parsing was successful (constant numeric expression)
 *		1 if non numeric and r == 0
 *		if r != 0 suppress yacc syntax (and other) error messages
 *
 *	parseConstantExpression() is used in comp.y
 *
 *******************************************************************/

int
parseConstantExpression(char * expressionText, int * valp, int r)
{
    in = inp = expressionText;		/* ready for yylex */
    suppressErrorMsg = r;
    *valp = 0;
    return cnparse(valp);
} /* parseConstantExpression */

/********************************************************************
 *
 *	Get a number, which may be decimal, octal or hexadecimal
 *	as well as a C type character constants '\0' '\n' 'a' etc
 *	as well as Octal or Hex character constants.
 *	    '\123' == 83
 *	    '\x23' == 35
 *	Multibyte character constants are handled like in C
 *	    '0123' == 0x30313233 == 808530483
 *	even
 *	    '\x30\x31\x32\x33' == 0x30313233 == 808530483
 *	or any mixture of octal or hex bytes and ascii characters in ''
 *
 *	One difference was found between this algorithm JW and the output
 *	of the gcc C compiler.
 *	Hex character constants longer then 2 characters are accepted with
 *	the warning: hex escape sequence out of range [enabled by default]
 *	MADE THE SAME by not defining JW (pretty obscure anyway).
 *	Intrestingly octal multi-byte character constants are limited
 *	to 3 characters by gcc C.
 *
 *	In C all multi-byte character constants evoke:
 *	warning: multi-character character constant [-Wmultichar]
 *	To be consistent the same has been done in iC.
 *
 *	Character constants are probably not very useful in iC
 *	but at least there will not be any errors if somebody
 *	decides to write obfuscated iC code.
 *
 *	getNumber() is also used in comp.y
 *
 *	Since getNumber is called twice for every number in a constant
 *	expression - once by comp.y and again by cons.y if the number
 *	is actually part of a constant expression - warnings are
 *	blocked for the second call in this parser to suppress 2nd
 *	identical warning.
 *		if r == 0 output compiler warnings
 *		if r != 0 suppress compiler warnings
 *
 *******************************************************************/

long
getNumber(char * numStr, char ** epp, int r)
{
    char *		cp = numStr;
    char *		cp1;
    long		value = 0L;
    unsigned int	v;
    int			c;
    int			n;
    int			i;

    if ((c = *cp++) == '\'') {
	for (i = 0; (c = *cp++) != '\'' && c != '\0'; i++) {
	    if (c == '\\') {
		v = 0;
		cp1 = cp;
		if (isdigit(c = *cp++)) {
		    if (sscanf(cp1, "%3o%n", &v, &n) == 1) {
			cp = cp1 + n;
		    }
		} else if (c == 'x' || c == 'X') {
#ifdef JW 
		    if (sscanf(cp, "%2x%n", &v, &n) == 1) {	/* n cannot be > 2 } */
#else
		    if (sscanf(cp, "%x%n", &v, &n) == 1) {
			if (r == 0 && n > 2) {	/* warning enabled by default as in C */
			    warning("hex escape sequence out of range:", numStr);
			}
#endif
			cp += n;
		    } else if (r == 0) {
			warning("\\x used with no following hex digits:", numStr);
		    }
		} else {
		    switch (c) {
		    case 'a':  v =  7; break;
		    case 'b':  v =  8; break;
		    case 't':  v =  9; break;
		    case 'n':  v = 10; break;
		    case 'v':  v = 11; break;
		    case 'f':  v = 12; break;
		    case 'r':  v = 13; break;
		    case '\"': v = 34; break;
		    case '\'': v = 39; break;
		    case '\?': v = 63; break;
		    case '\\': v = 92; break;
		    default:			/* warning enabled by default as in C */
			if (r == 0) warning("unknown escape sequence:", numStr);
			goto DirectConversion;	/* none of the above - treat '\c' as 'c' as in C */
		    }
		}
	    } else {
	      DirectConversion:
		v = c;
	    }
	    value <<= 8;
	    value |= v & 0xff;
	}
	if (r == 0) {
	    if (i == 0 || i > 4) {
		warning("character constant out of range:", numStr);
	    } else
	    if (i > 1 && iC_Wflag & W_DEPRECATED_LOGIC) {
		warning("multi-character character constant:", numStr);
	    }
	}
	*epp = cp;				/* 1 past character constant */
    } else {
	value = strtol(numStr, epp, 0);		/* convert to long */
    }
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
	if (isdigit(c) || c == '\'') {
	    yylval = (int)getNumber(--inp, &inp, suppressWarning); /* decimal octal, hex or character constant */
	    c = NUMBER;				/* must be at least a single 0 */
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
cnerror (int * retValue, const char * msg)
{
    if (suppressErrorMsg == 0) {
	iCerror("cons syntax error");				/* use full yyerror in comp.y */
    }
} /* cnerror */
#else	/* TESTCONS */

/********************************************************************
 *
 *	Warning function for test version - use full Warning for immcc
 *
 *******************************************************************/

void
warning(char * warnMsg, char * msg)
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

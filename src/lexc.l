%{
static const char lexc_c[] =
"@(#)$Id: lexc.l,v 1.29 2008/06/26 14:46:35 jw Exp $";
/********************************************************************
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <ic@je-wulff.de>
 *
 *	ANSI C Lexer
 *
 *	The ANSI C scanner/parser was posted on the net by Jeff Lee,
 *	in 1985. His net address is:
 *		jeff%gatech.CSNet@CSNet-Relay.ARPA, or jeff@gatech
 *	There was no open source license I could find.
 *
 *	Acknowledgement: Jeff Lee and the draft ANSI C standard grammar.
 *	Kernighan and Ritchie "The C Programming Language" 2nd Ed. A13.
 *
 *	This lexer was expanded to support most of the latest extensions
 *	used in gcc.
 *
 *	lexc.l
 *	C lexer for immcc compiler
 *
 *******************************************************************/

%}
D			[0-9]
L			[$a-zA-Z_]
S			[$a-zA-Z_0-9]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)
WS			[ \t\v\f\n]

%{
#include	<stdio.h>
#include	<ctype.h>
#include	<string.h>
#include	<assert.h>
#include	<errno.h>
#include	"icc.h"
#include	"comp.h"
#include	"gram_tab.h"

static int	c_comment(c_STYPE* u);
static int	cpp_comment(c_STYPE* u);
static int	preprocess(c_STYPE* u);
static int	ignore(c_STYPE* u);
static void	count(c_STYPE* u);
static int	check_type(c_STYPE* u, int qtoken);

#define YY_NO_UNPUT

#ifndef LMAIN
static Symbol	markerSymbol = { "marker", UDF, UDFA, };
static Symbol *	paraList = &markerSymbol;

#define YY_INPUT(buf,result,max_size) \
{\
    int c = '*', n;\
    for ( n = 0; n < max_size && (c = get( yyin, 1 )) != EOF && c != '\n'; ++n )\
	    buf[n] = (char) c;\
    if ( c == '\n' )\
	    buf[n++] = (char) c;\
    if ( c == EOF && ferror( yyin ) )\
	    YY_FATAL_ERROR( "input in flex scanner failed" );\
    result = n;\
}

#define ECHO
#endif

%}

%%
"/*"			{ if (c_comment(&c_lval)) yyterminate(); }
"//"			{ if (cpp_comment(&c_lval)) yyterminate(); }
^[ \t]*#		{ if (preprocess(&c_lval)) yyterminate(); }

"auto"			{ count(&c_lval); return AUTO; }
"break"			{ count(&c_lval); return BREAK; }
"case"			{ count(&c_lval); return CASE; }
"char"			{ count(&c_lval); return CHAR; }
(__)?const(__)?		{ count(&c_lval); return CONST; }
"continue"		{ count(&c_lval); return CONTINUE; }
"default"		{ count(&c_lval); return DEFAULT; }
"do"			{ count(&c_lval); return DO; }
"double"		{ count(&c_lval); return DOUBLE; }
"else"			{ count(&c_lval); return ELSE; }
"enum"			{ count(&c_lval); return ENUM; }
"extern"		{ count(&c_lval); return EXTERN; }
"float"			{ count(&c_lval); return FLOAT; }
"for"			{ count(&c_lval); return FOR; }
"goto"			{ count(&c_lval); return GOTO; }
"if"			{ count(&c_lval); return IF; }
"int"			{ count(&c_lval); return INT; }
"long"			{ count(&c_lval); return LONG; }
"register"		{ count(&c_lval); return REGISTER; }
"return"		{ count(&c_lval); return RETURN; }
"short"			{ count(&c_lval); return SHORT; }
(__)?signed(__)?	{ count(&c_lval); return SIGNED; }
"sizeof"		{ count(&c_lval); return SIZEOF; }
"static"		{ count(&c_lval); return STATIC; }
"struct"		{ count(&c_lval); return STRUCT; }
"switch"		{ count(&c_lval); return SWITCH; }
"typedef"		{ count(&c_lval); return TYPEDEF; }
(__)?typeof(__)?	{ count(&c_lval); return TYPEOF; }
"union"			{ count(&c_lval); return UNION; }
(__)?unsigned(__)?	{ count(&c_lval); return UNSIGNED; }
"void"			{ count(&c_lval); return VOID; }
(__)?volatile(__)?	{ count(&c_lval); return VOLATILE; }
"while"			{ count(&c_lval); return WHILE; }
(__)?builtin_va_list	{ count(&c_lval); return TYPE_NAME;	/* type in GCC 3.2 */ }
(__)?attribute(__)?	{ if (ignore(&c_lval)) yyterminate(); }
(__)?asm(__)?		{ if (ignore(&c_lval)) yyterminate(); }
(__)?extension(__)?	{ count(&c_lval);		/* ignore */ }
(__)?inline(__)?	{ count(&c_lval);		/* ignore */ }
(__)?restrict(__)?	{ count(&c_lval);		/* ignore */ }
"__LINE__"		{ count(&c_lval); return CONSTANT;	/* no need to identify octal */ }
"__FILE__"		{ count(&c_lval); return STRING_LITERAL; }

	/* allow white space because Intel C/C++ v8.1 pre-compiler expands IX0.1 to IX0 .1 */
(I|Q|T)X{D}+{WS}*\.{WS}*{D}+(_{D}+)?	{ count(&c_lval); return check_type(&c_lval, 2); }
(I|Q)(B|W|L){D}+(_{D}+)?		{ count(&c_lval); return check_type(&c_lval, 1); }
{L}{S}*					{ count(&c_lval); return check_type(&c_lval, 0); }

0[xX]{H}+{IS}*		{ count(&c_lval); return CONSTANT;	/* 0xac8 0XAC8UL */ }
{D}+{IS}*		{ count(&c_lval); return CONSTANT;	/* 12 12l 12u 12UL */ }
'(\\.|[^\\'])+'		{ count(&c_lval); return CONSTANT;	/* '\n' 'A' 'ABC' */ }

{D}+{E}{FS}?		{ count(&c_lval); return CONSTANT;	/* 12e2 12e2f 12E2L */ }
{D}*"."{D}+({E})?{FS}?	{ count(&c_lval); return CONSTANT;	/* .1 0.1 1.0 1.0e2 */ }
{D}+"."{D}*({E})?{FS}?	{ count(&c_lval); return CONSTANT;	/* 1. 1.e2 */ }

\"(\\.|[^\\"])*\"	{ count(&c_lval); return STRING_LITERAL; }

">>="			{ count(&c_lval); return RIGHT_ASSIGN; }
"<<="			{ count(&c_lval); return LEFT_ASSIGN; }
"+="			{ count(&c_lval); return ADD_ASSIGN; }
"-="			{ count(&c_lval); return SUB_ASSIGN; }
"*="			{ count(&c_lval); return MUL_ASSIGN; }
"/="			{ count(&c_lval); return DIV_ASSIGN; }
"%="			{ count(&c_lval); return MOD_ASSIGN; }
"&="			{ count(&c_lval); return AND_ASSIGN; }
"^="			{ count(&c_lval); return XOR_ASSIGN; }
"|="			{ count(&c_lval); return OR_ASSIGN; }
">>"			{ count(&c_lval); return RIGHT_OP; }
"<<"			{ count(&c_lval); return LEFT_OP; }
"++"			{ count(&c_lval); return INC_OP; }
"--"			{ count(&c_lval); return DEC_OP; }
"->"			{ count(&c_lval); return PTR_OP; }
"&&"			{ count(&c_lval); return AND_OP; }
"||"			{ count(&c_lval); return OR_OP; }
"<="			{ count(&c_lval); return LE_OP; }
">="			{ count(&c_lval); return GE_OP; }
"=="			{ count(&c_lval); return EQ_OP; }
"!="			{ count(&c_lval); return NE_OP; }
"..."			{ count(&c_lval); return ELIPSIS; }
";"			{ count(&c_lval); return ';'; }
"{"			{ count(&c_lval); return '{'; }
"}"			{ count(&c_lval); return '}'; }
","			{ count(&c_lval); return ','; }
":"			{ count(&c_lval); return ':'; }
"="			{ count(&c_lval); return '='; }
"("			{ count(&c_lval); return '('; }
")"			{ count(&c_lval); return ')'; }
"["			{ count(&c_lval); return '['; }
"]"			{ count(&c_lval); return ']'; }
"."			{ count(&c_lval); return '.'; }
"&"			{ count(&c_lval); return '&'; }
"!"			{ count(&c_lval); return '!'; }
"~"			{ count(&c_lval); return '~'; }
"-"			{ count(&c_lval); return '-'; }
"+"			{ count(&c_lval); return '+'; }
"*"			{ count(&c_lval); return '*'; }
"/"			{ count(&c_lval); return '/'; }
"%"			{ count(&c_lval); return '%'; }
"<"			{ count(&c_lval); return '<'; }
">"			{ count(&c_lval); return '>'; }
"^"			{ count(&c_lval); return '^'; }
"|"			{ count(&c_lval); return '|'; }
"?"			{ count(&c_lval); return '?'; }

[ \t\v\f]+		{ count(&c_lval); }
"\n"			{ count(&c_lval);	/* individual token */ }
.			{ count(&c_lval); 	/* ignore bad characters */ }

%%

int gramOffset = 0;

static int gramLine   = 1;
int column = 0;
FILE* saveFP = NULL;

int
yywrap(void)
{
    return 1;
}

/********************************************************************
 *
 *	C comment
 *
 *******************************************************************/

static int
c_comment(c_STYPE* u)
{
    int c;

    count(u);				/* count C comment start */

    if ((c = input()) == EOF) return 1;

    do {				/* start C style comment */
	while (c != '*') {
	    gramOffset++;
#if YYDEBUG && defined LMAIN
	    if (iC_debug & 040) putc(c, yyout);
#endif
	    if (c == '\n') {
		column = 0;
		gramLine++;
	    } else if (c == '\t') {
		column += 8 - (column % 8);
	    } else {
		column++;
	    }
	    if ((c = input()) == EOF) return 1;
	}
	column++;			/* count '*' */
	gramOffset++;
#if YYDEBUG && defined LMAIN
	if (iC_debug & 040) putc(c, yyout);
#endif
    } while ((c = input()) != '/');

    if (c == EOF) return 1;
    column++;				/* count '/' */
    gramOffset++;
#if YYDEBUG && defined LMAIN
    if (iC_debug & 040) putc(c, yyout);
#endif

    return 0;
} /* c_comment */

/********************************************************************
 *
 *	C++ comment
 *
 *	no need to process '\t'
 *
 *******************************************************************/

static int
cpp_comment(c_STYPE* u)
{
    int c;
#ifndef LMAIN
    static int errorCnt = 0;
#endif

    count(u);				/* count "//" */

    do {				/* start C++ style comment */
	if ((c = input()) == EOF) return 1;    /* error EOF in comment */
	gramOffset++;
#if YYDEBUG && defined LMAIN
	if (iC_debug & 040) putc(c, yyout);
#endif
    } while (c != '\n');
#ifndef LMAIN
    if (iC_Pflag && errorCnt++ == 0) {	/* report only once */
	warning("C++ style comments are not allowed in ISO C90:", c_text);
    }
#endif
    column = 0;
    gramLine++;
    return 0;
} /* cpp_comment */

/********************************************************************
 *
 *	Pre-processor line - more lines if last character is '\'
 *
 *	no need to process '\t'
 *
 *******************************************************************/

static int
preprocess(c_STYPE* u)
{
    int c = 0;
    int prev;

    count(u);				/* count "#" */

    do {				/* pre processeor line */
	do {
	    prev = c;
	    if ((c = input()) == EOF) return 1;    /* error EOF in line */
	    gramOffset++;
#if YYDEBUG && defined LMAIN
	    if (iC_debug & 040) putc(c, yyout);
#endif
	} while (c != '\n');
	column = 0;
	gramLine++;
    } while (prev == '\\');
    return 0;
} /* preprocess */

/********************************************************************
 *
 *	__attribute__ ((__mode__ (__HI__)))
 *	__asm__ __volatile__ ("btsl %1,%0" : "cc","memory")
 *
 *******************************************************************/

static int
ignore(c_STYPE* u)
{
    int c;
    int  bracketCount = 0;

    count(u);				/* count "__atribute__" */

    for (;;) {
	if ((c = input()) == EOF) return 1;    /* error EOF in comment */
	gramOffset++;
#if YYDEBUG && defined LMAIN
	if (iC_debug & 040) putc(c, yyout);
#endif
	if (c == '\n') {
	    column = 0;
	    gramLine++;
	} else if (c == '\t') {
	    column += 8 - (column % 8);
	} else {
	    column++;
	    if (c == '(') {
		bracketCount++;
	    } else if (c == ')' && --bracketCount <= 0) {
		break;
	    }
	}
    }
    return 0;
} /* ignore */

/********************************************************************
 *
 *	Count collumns, lines and bytes in gramOffset
 *
 *	When include files are processed (lexflag & C_NO_COUNT)
 *	it is important that "\n" are individual tokens not grouped
 *	as white space. Otherwise the trailing whitespace from an
 *	include file is grouped in c_text[] with the leading whitespace
 *	after the #include line and counted after C_NO_COUNT is reset.
 *
 *******************************************************************/

static void
count(c_STYPE* u)
{
    int i;

    u->tok.start = gramOffset;

    for (i = 0; c_text[i] != '\0'; i++) {
	if (c_text[i] == '\n') {
	    column = 0;
	    gramLine++;
	} else if (c_text[i] == '\t') {
	    column += 8 - (column % 8);
	} else {
	    column++;
	}
	gramOffset++;
    }

    u->tok.end = gramOffset;

#if YYDEBUG && defined LMAIN
    if (iC_debug & 040) { ECHO; }
#endif
} /* count */

/********************************************************************
 *
 *	Check type of an input token word
 *
 *	If token is a CTYPE return TYPE_NAME
 *	Else if token is an immediate variable of ftype ARITH or GATE
 *		return IMM_IDENTIFIER with pointer to Symbol in tok
 *		for any other imm variable return YYERRCODE
 *	Else token is a CWORD
 *		create a new Symbol and return CWORD
 *		when this Symbol is recognised as a CTYPE it is
 *			placed in the symbol table
 *		otherwise it is deleted
 *
 *******************************************************************/

static int
check_type(c_STYPE* u, int qtoken)
{
    Symbol *	sp = 0;
    int		n;
    char	iqt[2];			/* char buffers - space for 0 terminator */
    char	xbwl[2];
    int		byte;
    int		bit;
    int		ixd;
#ifdef LMAIN
#define buffer	c_text
    ixd = -1;				/* limit for I/O index (-1 no limit) */

#else
    int		c;
    int		i;
    char	buffer[BUFS];		/* buffer for modified names */
    char	tail[8];		/* compiler generated suffix _123456 max */

    ixd = iC_maxIO;			/* limit for I/O index (-1 no limit) */
    if (c_text[0] == '$') {
	/********************************************************************
	 *  Identifier starts with a '$'. gcc can handle this if the next character
	 *  is alpha but not if it is a digit.
	 *  Most assemblers have problems with leading '$' - hence the warning.
	 *******************************************************************/
	if (isdigit(c_text[1])) {
	    ierror("asm error due to '$' followed by digit starting identifier:", c_text);
	} else {
	    warning("possible asm error due to '$' character starting identifier:", c_text);
	}
    }
    if (qtoken == 2) {
	toIEC1131(c_text, buffer, BUFS, iqt, xbwl, &byte, &bit, tail);
	/********************************************************************
	 *  the Intel C/C++ v8.1 pre-compiler expands IX0.1 to IX0 .1
	 *  Generally if yacc were to handle the '.' as a seperate token,
	 *  white space would be allowed as it would in C for the operator '.'
	 *  If we allow white space here we will have the same effect without
	 *  changing the grammar.
	 *******************************************************************/
	for (i = n = 0; n <= c_leng; n++) {
	    c = buffer[n];
	    if (! isspace(c)) {		/* space \f \n \r \t \v */
		if (i < n) {
		    buffer[i] = c;
		}
		i++;
	    }
	}
    } else {
	strncpy(buffer, c_text, BUFS);
    }
    if (iFunSymExt && !qtoken) {
	if (iRetSymbol.v && strcmp(c_text, "this") == 0) {
	    sp = iRetSymbol.v;		/* function return Symbol */
	} else {
	    strncpy(iFunSymExt, c_text, iFunEnd - iFunSymExt);
	    sp = lookup(iFunBuffer);	/* maybe function internal Symbol */
	    *iFunSymExt = '\0';
	}
    }
#endif
    if (
#ifndef LMAIN
	sp == 0 &&
#endif
	(sp = lookup(buffer)) == 0) {
	/********************************************************************
	 *  Recognise and install undefined IEC1131 I/O variables. Otherwise
	 *  they have to be declared as imm variables, which is not consistent
	 *  with the iC use of I/O variables, which need not be declared.
	 *******************************************************************/
	if ((n = sscanf(buffer, "%1[IQT]%1[XBWL]%d.%d", iqt, xbwl, &byte, &bit)) >= 3) {
	    unsigned char	wplus = 0;
	    unsigned char	typ = UDF;
	    unsigned char	ftyp = UDFA;

	    if (xbwl[0] != 'X') {
		wplus = 1;		/* QB, QW, QL or IB, IW, IL */
		goto foundQIT;
	    }
	    if (n == 4) {		/* QX, IX or TX */
		if (bit < 8) {
		    foundQIT:
		    ftyp = GATE - wplus;	/* GATE or ARITH */
		    if (iqt[0] != 'Q') {
			typ = INPX - wplus;	/* INPX or INPW */
			if (iqt[0] == 'T') {
			    ixd = xbwl[0] == 'X' ? 1 : 0;	/* TX0.7 is max */
			}
		    }
		    if (ixd < 0 || byte < ixd) {
			sp = install(buffer, typ, ftyp);
			goto foundSymbol;
		    } else {
			YY_FATAL_ERROR( "I/O byte address must be less than 64" );\
		    }
		} else {
		    YY_FATAL_ERROR( "I/O bit address must be less than 8" );\
		}
	    }
	}
    } else {
	foundSymbol:
	if (sp->type <= MAX_LS && (sp->fm & FM) == 0) {
#ifndef LMAIN
	    if (lexflag & C_NO_COUNT) goto inInclude;	/* ignore imm in C includes */
	    if ((lexflag & C_BLOCK1) == 0) {
		if ((lexflag & C_PARA) == C_PARA) {
		    markParaList(sp);	/* mark Symbol as parameter on glist */
		}
		if (
#if YYDEBUG && ! defined(SYUNION)
		    sp->v_glist
#else
		    sp->v_cnt > 2	/* v_cnt instead of v_glist for SYUNION */
#endif
		) {
		    u->tok.symbol = sp;
		    return IDENTIFIER;	/* name of imm variable temporarily C parameter */
		}
	    }
#endif
	    while (sp->type == ALIAS) {
		sp = sp->list->le_sym;	/* with token of original */
	    }
	    u->tok.symbol = sp;
	    return IMM_IDENTIFIER;	/* analyze type errors in parser */
	} else if (sp->type == CTYPE) {
	    u->tok.symbol = sp;
	    return TYPE_NAME;
	} else if (sp->type == CWORD) {
	    u->tok.symbol = sp;
	    return IDENTIFIER;		/* CNAME C function recognized in comp.y */
	}
	return YYERRCODE;		/* internal imm node is an error */
    }
#ifndef LMAIN
  inInclude:
#endif
    sp = (Symbol *) iC_emalloc(sizeof(Symbol));
    sp->name = iC_emalloc(c_leng + 1);	/* +1 for '\0' */
    strcpy(sp->name, c_text);
    sp->type = CWORD;
    sp->ftype = UDFA;
    u->tok.symbol = sp;			/* sp->next === NULL */
#if YYDEBUG && defined LMAIN
    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "\nC %-15s %d %d\n", sp->name, sp->type, sp->ftype);
#endif
    return IDENTIFIER;
} /* check_type */

/********************************************************************
 *
 *	Delete unnecessary Symbol
 *
 *******************************************************************/

void
delete_sym(Token* tokp)
{
    Symbol *	sp;
    Symbol *	tsp;

    if ((sp = tokp->symbol) != NULL && sp->type > MAX_LS) {
	tokp->symbol = NULL;
#if YYDEBUG && defined LMAIN
	if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "\nD %-15s %d %d %u\n", sp->name, sp->type, sp->ftype, sp->u_val);
#endif
	if ((tsp = lookup(sp->name)) != 0 &&	/* check if in symbol table */
	    tsp == sp) {			/* and not generated by C parse */
	    unlink_sym(sp);			/* unlink Symbol from symbol table */
	}
	free(sp->name);
	free(sp);
    }
} /* delete_sym */
#ifndef LMAIN

/********************************************************************
 *
 *	Mark an imm Symbol as a parameter or function internal C variable
 *	on glist. The immediate Symbol is hidden till the end of the
 *	current C-function, when clearParaList() will clear the list.
 *	The same Symbol may appear several times - check for duplicates.
 *
 *	Alternatively mark an imm Symbol as a global C variable, which
 *	will be hidden in all subsequent literal blocks and C functions.
 *
 *******************************************************************/

void
markParaList(Symbol * sp)
{
    assert(sp && sp->type < IFUNCT);		/* allows IFUNCT to use union v.cnt */
    if (
#if YYDEBUG && ! defined(SYUNION)
	sp->v_glist == 0
#else
	sp->v_cnt <= 2				/* v_cnt instead of v_glist for SYUNION */
#endif
    ) {
	if ((lexflag & C_FUNCTION) == 0) {
	    sp->v_glist = &markerSymbol;	/* mark C globally */
	} else {
	    sp->v_glist = paraList;		/* mark on paraList */
	    paraList = sp;
	}
    }
} /* markParaList */

/********************************************************************
 *
 *	Clear parameter list from C function internal entries
 *
 *******************************************************************/

void
clearParaList(int flag)
{
    Symbol * sp;

    while (paraList != &markerSymbol) {
	sp = paraList->v_glist;
	assert(sp && sp->type < IFUNCT);	/* allows IFUNCT to use union v.cnt */
	paraList->v_glist = 0;
	paraList = sp;
    }
    if (flag) {
	lexflag |= (C_PARA|C_FUNCTION);
    } else {
	lexflag &= ~(C_PARA|C_FUNCTION);
    }
} /* clearParaList */
#endif

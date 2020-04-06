%{
static const char lexc_c[] =
"@(#)$Id: lexc.l 1.40 $";
/********************************************************************
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	ANSI C Lexer
 *
 *	The ANSI C scanner/parser was posted on the net by Jeff Lee,
 *	in 1985. His net address is:
 *		jeff%gatech.CSNet@CSNet-Relay.ARPA, or jeff@gatech.edu
 *	There was no open source license I could find.
 *
 *	Acknowledgement: Jeff Lee and the draft ANSI C standard grammar.
 *	Kernighan and Ritchie "The C Programming Language" 2nd Ed. A13.
 *
 *	This lexer was expanded to support most of the latest extensions
 *	used in gcc including _Floatn types.
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
#include	"comp.h"
#include	"gram.tab.h"

static int	yycomment(YYSTYPE* u);
static int	cpp_comment(YYSTYPE* u);
static int	preprocess(YYSTYPE* u);
static int	ignore(YYSTYPE* u);
static void	count(YYSTYPE* u);
static int	check_type(YYSTYPE* u, int qtoken);

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
"/*"			{ if (yycomment(&yylval)) yyterminate(); }
"//"			{ if (cpp_comment(&yylval)) yyterminate(); }
^[ \t]*#		{ if (preprocess(&yylval)) yyterminate(); }

"auto"			{ count(&yylval); return AUTO; }
"break"			{ count(&yylval); return BREAK; }
"case"			{ count(&yylval); return CASE; }
"char"			{ count(&yylval); return CHAR; }
(__)?const(__)?		{ count(&yylval); return CONST; }
"continue"		{ count(&yylval); return CONTINUE; }
"default"		{ count(&yylval); return DEFAULT; }
"do"			{ count(&yylval); return DO; }
"double"		{ count(&yylval); return DOUBLE; }
"else"			{ count(&yylval); return ELSE; }
"enum"			{ count(&yylval); return ENUM; }
"extern"		{ count(&yylval); return EXTERN; }
"float"			{ count(&yylval); return FLOAT; }
"_Float16"		{ count(&yylval); return FLOAT16; }
"_Float32"		{ count(&yylval); return FLOAT32; }
"_Float64"		{ count(&yylval); return FLOAT64; }
"_Float128"		{ count(&yylval); return FLOAT128; }
"for"			{ count(&yylval); return FOR; }
"goto"			{ count(&yylval); return GOTO; }
"if"			{ count(&yylval); return IF; }
"int"			{ count(&yylval); return INT; }
"long"			{ count(&yylval); return LONG; }
"register"		{ count(&yylval); return REGISTER; }
"return"		{ count(&yylval); return RETURN; }
"short"			{ count(&yylval); return SHORT; }
(__)?signed(__)?	{ count(&yylval); return SIGNED; }
"sizeof"		{ count(&yylval); return SIZEOF; }
"static"		{ count(&yylval); return STATIC; }
"struct"		{ count(&yylval); return STRUCT; }
"switch"		{ count(&yylval); return SWITCH; }
"typedef"		{ count(&yylval); return TYPEDEF; }
(__)?typeof(__)?	{ count(&yylval); return TYPEOF; }
"union"			{ count(&yylval); return UNION; }
(__)?unsigned(__)?	{ count(&yylval); return UNSIGNED; }
"void"			{ count(&yylval); return VOID; }
(__)?volatile(__)?	{ count(&yylval); return VOLATILE; }
"while"			{ count(&yylval); return WHILE; }
(__)?builtin_va_list	{ count(&yylval); return TYPE_NAME;	/* type in GCC 3.2 */ }
(__)?attribute(__)?	{ if (ignore(&yylval)) yyterminate(); }
(__)?asm(__)?		{ if (ignore(&yylval)) yyterminate(); }
(__)?extension(__)?	{ count(&yylval);		/* ignore */ }
(__)?inline(__)?	{ count(&yylval);		/* ignore */ }
(__)?restrict(__)?	{ count(&yylval);		/* ignore */ }
"__LINE__"		{ count(&yylval); return CONSTANT;	/* no need to identify octal */ }
"__FILE__"		{ count(&yylval); return STRING_LITERAL; }

	/* allow white space because Intel C/C++ v8.1 pre-compiler expands IX0.1 to IX0 .1 */
(I|Q|T)X{D}+{WS}*\.{WS}*{D}+(_{D}+)?	{ count(&yylval); return check_type(&yylval, 2); }
(I|Q)(B|W|L){D}+(_{D}+)?		{ count(&yylval); return check_type(&yylval, 1); }
{L}{S}*					{ count(&yylval); return check_type(&yylval, 0); }

0[xX]{H}+{IS}*		{ count(&yylval); return CONSTANT;	/* 0xac8 0XAC8UL */ }
{D}+{IS}*		{ count(&yylval); return CONSTANT;	/* 12 12l 12u 12UL */ }
'(\\.|[^\\'])+'		{ count(&yylval); return CONSTANT;	/* '\n' 'A' 'ABC' */ }

{D}+{E}{FS}?		{ count(&yylval); return CONSTANT;	/* 12e2 12e2f 12E2L */ }
{D}*"."{D}+({E})?{FS}?	{ count(&yylval); return CONSTANT;	/* .1 0.1 1.0 1.0e2 */ }
{D}+"."{D}*({E})?{FS}?	{ count(&yylval); return CONSTANT;	/* 1. 1.e2 */ }

\"(\\.|[^\\"])*\"	{ count(&yylval); return STRING_LITERAL; }

">>="			{ count(&yylval); return RIGHT_ASSIGN; }
"<<="			{ count(&yylval); return LEFT_ASSIGN; }
"+="			{ count(&yylval); return ADD_ASSIGN; }
"-="			{ count(&yylval); return SUB_ASSIGN; }
"*="			{ count(&yylval); return MUL_ASSIGN; }
"/="			{ count(&yylval); return DIV_ASSIGN; }
"%="			{ count(&yylval); return MOD_ASSIGN; }
"&="			{ count(&yylval); return AND_ASSIGN; }
"^="			{ count(&yylval); return XOR_ASSIGN; }
"|="			{ count(&yylval); return OR_ASSIGN; }
">>"			{ count(&yylval); return RIGHT_SH; }
"<<"			{ count(&yylval); return LEFT_SH; }
"++"			{ count(&yylval); return INC_OP; }
"--"			{ count(&yylval); return DEC_OP; }
"->"			{ count(&yylval); return PTR_OP; }
"&&"			{ count(&yylval); return AND_OP; }
"||"			{ count(&yylval); return OR_OP; }
"<="			{ count(&yylval); return LE_OP; }
">="			{ count(&yylval); return GE_OP; }
"=="			{ count(&yylval); return EQ_OP; }
"!="			{ count(&yylval); return NE_OP; }
"..."			{ count(&yylval); return ELIPSIS; }
";"			{ count(&yylval); return ';'; }
"{"			{ count(&yylval); return '{'; }
"}"			{ count(&yylval); return '}'; }
","			{ count(&yylval); return ','; }
":"			{ count(&yylval); return ':'; }
"="			{ count(&yylval); return '='; }
"("			{ count(&yylval); return '('; }
")"			{ count(&yylval); return ')'; }
"["			{ count(&yylval); return '['; }
"]"			{ count(&yylval); return ']'; }
"."			{ count(&yylval); return '.'; }
"&"			{ count(&yylval); return '&'; }
"!"			{ count(&yylval); return '!'; }
"~"			{ count(&yylval); return '~'; }
"-"			{ count(&yylval); return '-'; }
"+"			{ count(&yylval); return '+'; }
"*"			{ count(&yylval); return '*'; }
"/"			{ count(&yylval); return '/'; }
"%"			{ count(&yylval); return '%'; }
"<"			{ count(&yylval); return '<'; }
">"			{ count(&yylval); return '>'; }
"^"			{ count(&yylval); return '^'; }
"|"			{ count(&yylval); return '|'; }
"?"			{ count(&yylval); return '?'; }

[ \t\v\f]+		{ count(&yylval); }
"\n"			{ count(&yylval);	/* individual token */ }
.			{ count(&yylval); 	/* ignore bad characters */ }

%%

int		gramOffset = 0;		/* count input in lex */

static int	gramLine   = 1;
int		column = 0;
FILE*		saveFP = NULL;

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
yycomment(YYSTYPE* u)
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
} /* yycomment */

/********************************************************************
 *
 *	C++ comment
 *
 *	no need to process '\t'
 *
 *******************************************************************/

static int
cpp_comment(YYSTYPE* u)
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
	warning("C++ style comments are not allowed in ISO C90:", yytext);
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
preprocess(YYSTYPE* u)
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
ignore(YYSTYPE* u)
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
 *	include file is grouped in yytext[] with the leading whitespace
 *	after the #include line and counted after C_NO_COUNT is reset.
 *
 *******************************************************************/

static void
count(YYSTYPE* u)
{
    int i;

    u->tok.start = gramOffset;

    for (i = 0; yytext[i] != '\0'; i++) {
	if (yytext[i] == '\n') {
	    column = 0;
	    gramLine++;
	} else if (yytext[i] == '\t') {
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
 *	Else if token is an immediate variable of ftype UDFA and type
 *		ARNC or LOGC return IMM_ARRAY_IDENTIFIER with pointer
 *		to Symbol in tok
 *	Else if token is an immediate variable of ftype ARITH or GATE
 *		return IMM_IDENTIFIER with pointer to Symbol in tok
 *		for any other imm variable return YYERRCODE
 *	Else if token is "LO" or "HI" which have not been used before
 *		generate GATE "LO" and ALIAS "HI" linked to "LO"
 *		return IMM_IDENTIFIER with pointer to Symbol in tok
 *	Else token is a CWORD
 *		create a new Symbol and return CWORD
 *		when this Symbol is recognised as a CTYPE it is
 *			placed in the symbol table
 *		otherwise it is deleted
 *
 *******************************************************************/

static int
check_type(YYSTYPE* u, int qtoken)
{
    Symbol *	sp = 0;
    List_e *	lp;
    int		inv;
    int		n;
    char	iqt[2];			/* char buffers - space for 0 terminator */
    char	xbwl[2];
    int		byte;
    int		bit;
    int		ixd;
#ifdef LMAIN
#define buffer	yytext
    ixd = -1;				/* limit for I/O index (-1 no limit) */

#else
    int		c;
    int		i;
    char	buffer[BUFS];		/* buffer for modified names */
    char	tail[8];		/* compiler generated suffix _123456 max */

    ixd = iC_maxIO;			/* limit for I/O index (-1 no limit) */
    if (yytext[0] == '$') {
	/********************************************************************
	 *  Identifier starts with a '$'. gcc can handle this if the next character
	 *  is alpha but not if it is a digit.
	 *  Most assemblers have problems with leading '$' - hence the warning.
	 *******************************************************************/
	if (isdigit(yytext[1])) {
	    ierror("asm error due to '$' followed by digit starting identifier:", yytext);
	} else {
	    warning("possible asm error due to '$' character starting identifier:", yytext);
	}
    }
    if (qtoken == 2) {
	iC_toIEC1131(yytext, buffer, BUFS, iqt, xbwl, &byte, &bit, tail);
	/********************************************************************
	 *  the Intel C/C++ v8.1 pre-compiler expands IX0.1 to IX0 .1
	 *  Generally if yacc were to handle the '.' as a separate token,
	 *  white space would be allowed as it would in C for the operator '.'
	 *  If we allow white space here we will have the same effect without
	 *  changing the grammar.
	 *******************************************************************/
	for (i = n = 0; n <= yyleng; n++) {
	    c = buffer[n];
	    if (! isspace(c)) {		/* space \f \n \r \t \v */
		if (i < n) {
		    buffer[i] = c;
		}
		i++;
	    }
	}
    } else {
	strncpy(buffer, yytext, BUFS);
    }
    if (iFunSymExt && !qtoken) {
	if (iRetSymbol.v && strcmp(yytext, "this") == 0) {
	    sp = iRetSymbol.v;		/* function return Symbol */
	} else {
	    strncpy(iFunSymExt, yytext, iFunEnd - iFunSymExt);
	    sp = lookup(iFunBuffer);	/* maybe function internal Symbol */
	    *iFunSymExt = '\0';
	}
    }
#endif
    inv = 0;				/* start with no ALIAS inversion */
    if (
#ifndef LMAIN
	sp == 0 &&
#endif
	(sp = lookup(buffer)) == 0) {
	/********************************************************************
	 *  Recognise and install undefined IEC1131 I/O variables. Otherwise
	 *  they have to be declared as imm variables, which is not consistent
	 *  with the iC use of I/O variables, which need not be declared.
	 *  Mark QXx.y QBz etc by setting em |= EO (also done in iClex())
	 *******************************************************************/
	if ((n = sscanf(buffer, "%1[IQT]%1[XBWL]%d.%d", iqt, xbwl, &byte, &bit)) >= 3) {
	    unsigned char	wplus = 0;
	    unsigned char	typ = UDF;
	    unsigned char	ftyp = UDFA;

	    if (xbwl[0] != 'X') {
		wplus = GATE - ARITH;	/* QB, QW, QL or IB, IW, IL */
		assert(wplus == INPX - INPW);
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
			if (iqt[0] == 'Q') {
			    sp->em |= EO;	/* marks variable as Q... */
			}
			goto foundSymbol;	/* newly installed Q... output */
		    } else {
			char errorMsg[41];	/* length of message + 5 digit number + \0 */
			snprintf(errorMsg, 40, "I/O byte address must be less than %d", ixd);
			YY_FATAL_ERROR( errorMsg );\
		    }
		} else {
		    YY_FATAL_ERROR( "I/O bit address must be less than 8" );\
		}
	    }
	}					/* unknown token - generate LO HI or IDENTIFIER */
    } else {
      foundSymbol:				/* imm Symbol previously declared or new Q... output */
	if (sp->type < MAX_LS && (sp->fm & FM) == 0) {	/* only imm types */
#ifndef LMAIN
	    if (lexflag & C_NO_COUNT) goto inInclude;	/* ignore imm in C includes */
	    if ((lexflag & C_BLOCK1) == 0) {
		if ((lexflag & C_PARA) == C_PARA) {
		    markParaList(sp);	/* mark Symbol as parameter on blist */
		}
		if (sp->u_blist) {
		    u->tok.symbol = sp;
		    return IDENTIFIER;	/* name of imm variable temporarily C parameter */
		}
	    }
#endif
	    while (sp->type == ALIAS) {
		lp = sp->list;
		inv ^= lp->le_val;	/* ignore inv if final target is not GATE */
		sp = lp->le_sym;	/* with token of original */
	    }
#ifndef LMAIN
	  genToken:
#endif
	    u->tok.symbol = sp;
	    if (sp->ftype == ARITH || sp->ftype == GATE) {
		if (inv && sp->ftype == GATE) {
		    u->tok.start |= INVERT;	/* cannot mark sp */
		}
		return IMM_IDENTIFIER;	/* analyze type errors in parser */
	    } else if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
		return IMM_ARRAY_IDENTIFIER;	/* immC array */
	    } else {
		return YYERRCODE;	/* other imm node is an error */
	    }
#ifndef LMAIN
	} else if (sp->type == KEYW) {
	    goto inInclude;		/* KEYWs only required in iC compile */
#endif
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
    if (strcmp(yytext, "LO") == 0) {
	if ((sp = lookup("LO")) == 0) {		/* lookup/install permanent bit 0 gate LO */
	    sp = install("LO", OR, GATE);	/* defined but never changes */
	}
	goto genToken;				/* return IMM_IDENTIFIER LO (inv is 0) */
    }
    if (strcmp(yytext, "HI") == 0) {
	Symbol *	symp;
	if ((symp = lookup("HI")) == 0) {	/* lookup/install permanent bit 1 alias HI */
	    symp = install("HI", ALIAS, GATE);
	    if ((sp = lookup("LO")) == 0) {	/* lookup/install permanent bit 0 gate */
		sp = install("LO", OR, GATE);	/* defined but never changes */
	    }
	    symp->list = lp = sy_push(sp);	/* complete ALIAS to LO */
	    lp->le_val = NOT;			/* HI is inverting alias of LO */
	}
	inv = 1;				/* skip while loop */
	goto genToken;				/* return IMM_IDENTIFIER LO with inv 1 */
    }
  inInclude:
#endif
    /* generate a CWORD Symbol which is not installed in the iC symbol table */
    sp = (Symbol *) iC_emalloc(sizeof(Symbol));
    sp->name = iC_emalloc(yyleng + 1);	/* +1 for '\0' */
    strcpy(sp->name, yytext);
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
	if ((tsp = lookup(sp->name)) != NULL &&	/* check if in symbol table */
	    tsp == sp) {			/* and not generated by C parse */
	    unlink_sym(sp);			/* unlink Symbol from symbol table */
	}
	if (sp->name) {
	    free(sp->name);			/* completely delete name (no longer cleared in unlink_sym - jw 20141007) */
	}
	free(sp);				/* and Symbol */
    }
} /* delete_sym */
#ifndef LMAIN

/********************************************************************
 *
 *	Mark an imm Symbol as a parameter or function internal C variable
 *	on blist. The immediate Symbol is hidden till the end of the
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
    if (sp->u_blist == 0) {
	if ((lexflag & C_FUNCTION) == 0) {
	    sp->u_blist = (List_e *)(&markerSymbol);	/* mark C globally */
	} else {
	    sp->u_blist = (List_e *)paraList;		/* mark on paraList */
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
	sp = (Symbol *)paraList->u_blist;
	assert(sp && sp->type < IFUNCT);	/* allows IFUNCT to use union v.cnt */
	paraList->u_blist = 0;
	paraList = sp;
    }
    if (flag) {
	lexflag |= (C_PARA|C_FUNCTION);
    } else {
	lexflag &= ~(C_PARA|C_FUNCTION);
    }
} /* clearParaList */
#endif

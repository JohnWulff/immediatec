%{
static const char lexc_c[] =
"@(#)$Id: lexc.l,v 1.11 2002/08/17 22:34:57 jw Exp $";
/********************************************************************
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <john@je-wulff.de>
 *
 *	ANSI C Grammar and scanner
 *
 *	The ANSI C scanner/parser was posted on the net by Jeff Lee,
 *	whose net address is
 *		jeff%gatech.CSNet@CSNet-Relay.ARPA, or jeff@gatech
 *	The grammar is based on the draft ANSI standard.
 *
 *	Acknowledgement: This code is based on Jeff Lee's code.
 *
 *	lexc.l
 *	C lexer for icc compiler
 *
 *******************************************************************/

%}
D			[0-9]
L			[a-zA-Z_]
H			[a-fA-F0-9]
E			[Ee][+-]?{D}+
FS			(f|F|l|L)
IS			(u|U|l|L)

%{
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include "icc.h"
#include "comp.h"
#include "gram_tab.h"

static int	c_comment(c_STYPE* u);
static int	cpp_comment(c_STYPE* u);
static int	preprocess(c_STYPE* u);
static int	ignore(c_STYPE* u);
static void	count(c_STYPE* u);
static int	check_type(c_STYPE* u);

#ifndef LMAIN
#define YY_INPUT(buf,result,max_size) \
{\
    int c = '*', n;\
    for ( n = 0; n < max_size && (c = get( yyin )) != EOF && c != '\n'; ++n )\
	    buf[n] = (char) c;\
    if ( c == '\n' )\
	    buf[n++] = (char) c;\
    if ( c == EOF && ferror( yyin ) )\
	    YY_FATAL_ERROR( "input in flex scanner failed" );\
    result = n;\
}

#define ECHO
#define YY_NO_UNPUT
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
(__)?attribute(__)?	{ if (ignore(&c_lval)) yyterminate(); }
(__)?asm(__)?		{ if (ignore(&c_lval)) yyterminate(); }
(__)?extension(__)?	{ count(&c_lval); }		/* ignore */
(__)?inline(__)?	{ count(&c_lval); }		/* ignore */
(__)?restrict(__)?	{ count(&c_lval); }		/* ignore */

(I|Q|T)X{D}+\.{D}+	{ count(&c_lval); return check_type(&c_lval); }
{L}({L}|{D})*		{ count(&c_lval); return check_type(&c_lval); }

"__LINE__"		{ count(&c_lval); return CONSTANT; }	/* no need to identify octal */
0[xX]{H}+{IS}*		{ count(&c_lval); return CONSTANT; }	/* 0xac8 0XAC8UL */
{D}+{IS}*		{ count(&c_lval); return CONSTANT; }	/* 12 12l 12u 12UL */
'(\\.|[^\\'])+'		{ count(&c_lval); return CONSTANT; }	/* '\n' 'A' 'ABC' */

{D}+{E}{FS}?		{ count(&c_lval); return CONSTANT; }	/* 12e2 12e2f 12E2L */
{D}*"."{D}+({E})?{FS}?	{ count(&c_lval); return CONSTANT; }	/* .1 0.1 1.0 1.0e2 */
{D}+"."{D}*({E})?{FS}?	{ count(&c_lval); return CONSTANT; }	/* 1. 1.e2 */

"__FILE__"		{ count(&c_lval); return STRING_LITERAL; }
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

[ \t\v\n\f]+		{ count(&c_lval); }
.			{ count(&c_lval); /* ignore bad characters */ }

%%

static int gramOffset = 0;
static int gramLine   = 1;
int column = 0;

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

    count(u);			/* count "/*" */

    if ((c = input()) == EOF) return 1;

    do {		/* start C style comment */
	while (c != '*') {
	    gramOffset++;
#if YYDEBUG && defined LMAIN
	    if (debug & 010) putc(c, yyout);
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
	column++;		/* count '*' */
	gramOffset++;
#if YYDEBUG && defined LMAIN
	if (debug & 010) putc(c, yyout);
#endif
    } while ((c = input()) != '/');

    if (c == EOF) return 1;
    column++;			/* count '/' */
    gramOffset++;
#if YYDEBUG && defined LMAIN
    if (debug & 010) putc(c, yyout);
#endif

    return 0;
} /* c_comment */

/********************************************************************
 *
 *	C++ comment
 *
 *******************************************************************/

static int
cpp_comment(c_STYPE* u)
{
    int c;

    count(u);			/* count "//" */

    do {			/* start C++ style comment */
	if ((c = input()) == EOF) return 1;    /* error EOF in comment */
	gramOffset++;
#if YYDEBUG && defined LMAIN
	if (debug & 010) putc(c, yyout);
#endif
    } while (c != '\n');
    column = 0;
    gramLine++;
    return 0;
} /* cpp_comment */

/********************************************************************
 *
 *	Pre-processor line - more lines if last character is '\'
 *
 *******************************************************************/

static int
preprocess(c_STYPE* u)
{
    int c = 0;
    int prev;

    count(u);			/* count "#" */

    do {			/* pre processeor line */
	do {
	    prev = c;
	    if ((c = input()) == EOF) return 1;    /* error EOF in line */
	    gramOffset++;
#if YYDEBUG && defined LMAIN
	    if (debug & 010) putc(c, yyout);
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

    count(u);			/* count "__atribute__" */

    for (;;) {
	if ((c = input()) == EOF) return 1;    /* error EOF in comment */
	gramOffset++;
#if YYDEBUG && defined LMAIN
	if (debug & 010) putc(c, yyout);
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
    if (debug & 010) { ECHO; }
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
check_type(c_STYPE* u)
{
    Symbol *	sp;
#ifndef LMAIN
    int			mType;
    char		buffer[BUFS];	/* buffer for modified names */
    char		iqt[2];		/* char buffers - space for 0 terminator */
    char		bwx[2];
    int			byte;
    int			bit;
    char		tail[8];	/* compiler generated suffix _123456 max */

    toIEC1131(c_text, buffer, BUFS, iqt, bwx, &byte, &bit, tail);
#else
#define buffer		c_text
#endif

    if ((sp = lookup(buffer)) != 0) {
	if ((sp->type & TM) <= MAX_LS) {
	    while ((sp->type & TM) == ALIAS) {
		sp = sp->list->le_sym;	/* with token of original */
	    }
	    u->tok.symbol = sp;
	    return IMM_IDENTIFIER;	/* analyze type errors in parser */
	} else if (sp->type == CTYPE) {
	    u->tok.symbol = sp;
	    return TYPE_NAME;
	}
	return YYERRCODE;		/* internal imm node is an error */
    }
    sp = (Symbol *) emalloc(sizeof(Symbol));
    sp->name = emalloc(c_leng + 1);	/* +1 for '\0' */
    strcpy(sp->name, c_text);
    sp->type = CWORD;
    sp->ftype = UDFA;
    u->tok.symbol = sp;			/* sp->next === NULL */
#if YYDEBUG && defined LMAIN
    if ((debug & 0402) == 0402) fprintf(outFP, "\nC %-15s %d %d\n", sp->name, sp->type, sp->ftype);
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

    if ((sp = tokp->symbol) != NULL && (sp->type & TM) > MAX_LS) {
	tokp->symbol = NULL;
#if YYDEBUG && defined LMAIN
	if ((debug & 0402) == 0402) fprintf(outFP, "\nD %-15s %d %d\n", sp->name, sp->type, sp->ftype);
#endif
	free(sp->name);
	free(sp);
    }
} /* delete_sym */

/********************************************************************
 *
 *	Read C-Symbols cache file and store names of available include
 *	files and the file offset of the entries in the Symbol Table.
 *
 *	The cache file is left open for reading and when necessary, the
 *	lines with the named include file can be accessed with an fseek
 *	and the relevant type names read.
 *
 *	The entries for <stdio.h>, "icc.h" and "comp.h" should be read
 *	straight away, since they are declared as includes for generated
 *	C files.  It is not correct to read types for other include files
 *	available in the cache, since they may clash.
 *
 *	Once an entry line has been read in, the symbol table entry is
 *	marked with ftype = CRINC
 *
 *	Structure of C-Symbols cache file:	(by example)
 *
 *	<stdio.h> FILE size_t etc
 *	"icc.h" [data stamp] Gate Functp2 Functp CFunctp
 *	"comp.h" [data stamp] List_e Symbol Sym Lis Val Str
 *	<stdlib.h> uint ulong etc
 *	"../include/types.h" [data stamp] type1 type2
 *		lines longer than TCBUFS are folded at the end of a type_name
 *		follow up lines start with a space character.
 *
 *	Include files in angle brackets are assumed to be in the include
 *	path and are not date stamped.
 *
 *	Other include files are local or have a path relative to the
 *	local working directory.
 *
 *	The C-Symbols file is called "iCcTypes.def" and is in the
 *	same path as the iC compiler called. It is writable by all.
 *	It is a cache of all type_names used in include files in
 *	C-Code for the computer installation.
 *
 *	PROBLEM: under Linux the path of the exected file is not available
 *
 *	The local file names should be in a file in the local directory.
 *	Initial implementation: a local file for all includes.
 *
 *	Name of cache file defined in comp.h
 *
 *******************************************************************/

int
readTypeCacheFile(void)
{
    int		c;
    int		ftype;
    long	fileOffset;
    Symbol *	sp;
    char	nameBuf[BUFS];
    char	lineBuf[TCBUFS];

    if (typeCacheFP == NULL) {
	/* open the cache file and leave it open for reading details */
	if ((typeCacheFP = fopen(typeCacheFN, "r+")) == NULL) {
	    if (errno != ENOENT || (typeCacheFP = fopen(typeCacheFN, "w+")) == NULL) {
		perror("could not open file");
		return TCindex;			/* error opening file */
	    }
	}
    }

    for (fileOffset = 0;			/* start of the file */
	fgets(lineBuf, TCBUFS, typeCacheFP) != NULL;
	fileOffset += strlen(lineBuf))		/* start of the next line */
    {
	if ((c = *lineBuf) == '<') {
	    ftype = CGINC;			/* global include file */
	} else if (c == '"') {
	    ftype = CLINC;			/* local include file */
	} else {
	    if (c != ' ') {
		goto formatError;		/* strange line */
	    }
	    continue;				/* ignore follow up lines */
	}
	if (sscanf(lineBuf, "%[<\"/A-Za-z_.0-9>]", nameBuf) == 1) {
	    if (debug & 02) fprintf(outFP, "####### %s\n", nameBuf);
	    /* store include file name in symbol table */
	    if ((sp = lookup(nameBuf)) != 0) {
		if (sp->type == CINC) {
		    warning("readTypeCacheFile: multiple entry ???:", nameBuf);
		} else {
		    warning("readTypeCacheFile: wrong type in ST ???:", nameBuf);
		}
	    } else if ((sp = install(nameBuf, CINC, ftype)) != 0) {
		sp->u.val = fileOffset;		/* new entry */
	    } else {
		error("readTypeCacheFile: could not store symbol:", nameBuf);
		return TCindex;			/* error reading file */
	    }
	} else {
	  formatError:
	    error("readTypeCacheFile: strange line:", lineBuf);
	    return TCindex;			/* error reading file */
	}
    }

    /* process <stdio.h> "icc.h" and "comp.h" */
    if (readTypesFromCache("<stdio.h>")) return TCindex;
    if (readTypesFromCache("\"icc.h\"")) return TCindex;
    if (readTypesFromCache("\"comp.h\"")) return TCindex;

    return 0;
} /* readTypeCacheFile */

int
readTypesFromCache(char* includeName)
{
    int		c;
    int		fFlag;
    long	fileOffset;
    Symbol *	sp;
    char*	tokenPtr;
    char	lineBuf[TCBUFS];

    if (typeCacheFP == NULL) {
	/* open the cache file and leave it open for reading details */
	if (readTypeCacheFile() != 0) {
	    return TCindex;			/* error handling cache file */
	}
    }

    /* read the relevant symbol table entry and get offset */
    if ((sp = lookup(includeName)) == 0) {
	/* c_compile the actual include file to extend the cache */
	/* involves appending to cache file and installing in ST */
	error("readTypesFromCache: wrong file ???:", includeName);
	return TCindex;				/* cannot proceed */
    } else if (sp->type != CINC) {
	error("readTypesFromCache: wrong type in ST ???:", includeName);
	return TCindex;				/* cannot proceed */
    }

    fileOffset = sp->u.val;
    sp->ftype = CFINC;				/* entry has been read from file */
    if (debug & 02) fprintf(outFP, "####### %s", includeName);
    if (fseek(typeCacheFP, fileOffset, SEEK_SET) < 0) {
	error("readTypesFromCache: wrong offset in ST ???:", includeName);
	return TCindex;				/* cannot proceed */
    }

    fFlag = 0;
    while (fgets(lineBuf, TCBUFS, typeCacheFP) != NULL) {
	if ((c = *lineBuf) != ' ') {
	    if (fFlag == 0 && (c == '<' || c == '"')) {
		fFlag = 1;
		/* first strtok() skips name entry */
		if (strtok(lineBuf, " \n") == NULL) {
		    error("readTypesFromCache: wrong first token ???:", lineBuf);
		    return TCindex;				/* cannot proceed */
		}
		if (c == '"') {
		    /* could handle date stamp here */
		}
		tokenPtr = strtok(NULL, " \n");	/* first type token */
	    } else {
		break;				/* next entry - ignore errors */
	    }
	} else {
	    /* skip space at start of follow up line */
	    tokenPtr = strtok(lineBuf + 1, " \n");	/* first token from follow up */
	}
	/* break up the type entries in the line */
	while (tokenPtr && *tokenPtr != '\n') {
	    if ((sp = lookup(tokenPtr)) != 0) {
		if (sp->type != CTYPE) {
		    warning("readTypesFromCache: wrong type in ST ???:", tokenPtr);
		}
		/* ignore multiple type_names from different includes - happens a lot */
		if (debug & 02) fprintf(outFP, ";%s", tokenPtr);
	    } else if ((sp = install(tokenPtr, CTYPE, COLD)) == 0) {
		warning("readTypesFromCache: problem installing Symbol in ST ???:", tokenPtr);
	    } else {
		if (debug & 02) fprintf(outFP, ",%s", tokenPtr);
	    }
	    tokenPtr = strtok(NULL, " \n");	/* next type token */
	}
    }
    if (debug & 02) fprintf(outFP, ":\n");

    return 0;
} /* readTypesFromCache */

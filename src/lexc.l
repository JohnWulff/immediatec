%{
static const char lexc_c[] =
"@(#)$Id: lexc.l,v 1.3 2002/07/08 15:09:36 jw Exp $";
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
IS			(u|U|l|L)*

%{
#include <stdio.h>
#include "icc.h"
#include "comp.h"
#include "gram.h"
#include "gram_tab.h"

void count (c_STYPE* u);
%}

%%
"/*"			{ comment(); }

"auto"			{ count(&c_lval); return(AUTO); }
"break"			{ count(&c_lval); return(BREAK); }
"case"			{ count(&c_lval); return(CASE); }
"char"			{ count(&c_lval); return(CHAR); }
"const"			{ count(&c_lval); return(CONST); }
"continue"		{ count(&c_lval); return(CONTINUE); }
"default"		{ count(&c_lval); return(DEFAULT); }
"do"			{ count(&c_lval); return(DO); }
"double"		{ count(&c_lval); return(DOUBLE); }
"else"			{ count(&c_lval); return(ELSE); }
"enum"			{ count(&c_lval); return(ENUM); }
"extern"		{ count(&c_lval); return(EXTERN); }
"float"			{ count(&c_lval); return(FLOAT); }
"for"			{ count(&c_lval); return(FOR); }
"goto"			{ count(&c_lval); return(GOTO); }
"if"			{ count(&c_lval); return(IF); }
"int"			{ count(&c_lval); return(INT); }
"long"			{ count(&c_lval); return(LONG); }
"register"		{ count(&c_lval); return(REGISTER); }
"return"		{ count(&c_lval); return(RETURN); }
"short"			{ count(&c_lval); return(SHORT); }
"signed"		{ count(&c_lval); return(SIGNED); }
"sizeof"		{ count(&c_lval); return(SIZEOF); }
"static"		{ count(&c_lval); return(STATIC); }
"struct"		{ count(&c_lval); return(STRUCT); }
"switch"		{ count(&c_lval); return(SWITCH); }
"typedef"		{ count(&c_lval); return(TYPEDEF); }
"union"			{ count(&c_lval); return(UNION); }
"unsigned"		{ count(&c_lval); return(UNSIGNED); }
"void"			{ count(&c_lval); return(VOID); }
"volatile"		{ count(&c_lval); return(VOLATILE); }
"while"			{ count(&c_lval); return(WHILE); }

{L}({L}|{D})*		{ count(&c_lval); return(check_type()); }

0[xX]{H}+{IS}?		{ count(&c_lval); return(CONSTANT); }
0{D}+{IS}?		{ count(&c_lval); return(CONSTANT); }
{D}+{IS}?		{ count(&c_lval); return(CONSTANT); }
'(\\.|[^\\'])+'		{ count(&c_lval); return(CONSTANT); }

{D}+{E}{FS}?		{ count(&c_lval); return(CONSTANT); }
{D}*"."{D}+({E})?{FS}?	{ count(&c_lval); return(CONSTANT); }
{D}+"."{D}*({E})?{FS}?	{ count(&c_lval); return(CONSTANT); }

\"(\\.|[^\\"])*\"	{ count(&c_lval); return(STRING_LITERAL); }

">>="			{ count(&c_lval); return(RIGHT_ASSIGN); }
"<<="			{ count(&c_lval); return(LEFT_ASSIGN); }
"+="			{ count(&c_lval); return(ADD_ASSIGN); }
"-="			{ count(&c_lval); return(SUB_ASSIGN); }
"*="			{ count(&c_lval); return(MUL_ASSIGN); }
"/="			{ count(&c_lval); return(DIV_ASSIGN); }
"%="			{ count(&c_lval); return(MOD_ASSIGN); }
"&="			{ count(&c_lval); return(AND_ASSIGN); }
"^="			{ count(&c_lval); return(XOR_ASSIGN); }
"|="			{ count(&c_lval); return(OR_ASSIGN); }
">>"			{ count(&c_lval); return(RIGHT_OP); }
"<<"			{ count(&c_lval); return(LEFT_OP); }
"++"			{ count(&c_lval); return(INC_OP); }
"--"			{ count(&c_lval); return(DEC_OP); }
"->"			{ count(&c_lval); return(PTR_OP); }
"&&"			{ count(&c_lval); return(AND_OP); }
"||"			{ count(&c_lval); return(OR_OP); }
"<="			{ count(&c_lval); return(LE_OP); }
">="			{ count(&c_lval); return(GE_OP); }
"=="			{ count(&c_lval); return(EQ_OP); }
"!="			{ count(&c_lval); return(NE_OP); }
";"			{ count(&c_lval); return(';'); }
"{"			{ count(&c_lval); return('{'); }
"}"			{ count(&c_lval); return('}'); }
","			{ count(&c_lval); return(','); }
":"			{ count(&c_lval); return(':'); }
"="			{ count(&c_lval); return('='); }
"("			{ count(&c_lval); return('('); }
")"			{ count(&c_lval); return(')'); }
"["			{ count(&c_lval); return('['); }
"]"			{ count(&c_lval); return(']'); }
"."			{ count(&c_lval); return('.'); }
"&"			{ count(&c_lval); return('&'); }
"!"			{ count(&c_lval); return('!'); }
"~"			{ count(&c_lval); return('~'); }
"-"			{ count(&c_lval); return('-'); }
"+"			{ count(&c_lval); return('+'); }
"*"			{ count(&c_lval); return('*'); }
"/"			{ count(&c_lval); return('/'); }
"%"			{ count(&c_lval); return('%'); }
"<"			{ count(&c_lval); return('<'); }
">"			{ count(&c_lval); return('>'); }
"^"			{ count(&c_lval); return('^'); }
"|"			{ count(&c_lval); return('|'); }
"?"			{ count(&c_lval); return('?'); }

[ \t\v\n\f]+		{ count(&c_lval); }
.			{ count(&c_lval); /* ignore bad characters */ }

%%

yywrap()
{
    return(1);
}

comment()
{
    char c, c1;

loop:
    while ((c = input()) != '*' && c != 0) {
	putchar(c);
    }

    if ((c1 = input()) != '/' && c != 0) {
	unput(c1);
	goto loop;
    }

    if (c != 0) {
	putchar(c1);
    }
}

static int gramOffset = 0;
static int gramLine   = 1;
int column = 0;

void count(c_STYPE* u)
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

    ECHO;
}

int check_type()
{
/*
* pseudo code --- this is what it should check
*
*	if (yytext == type_name)
*		return(TYPE_NAME);
*
*	return(IDENTIFIER);
*/

    return(IDENTIFIER);
}

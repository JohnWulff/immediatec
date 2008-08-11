%{ static const char gram_y[] =
"@(#)$Id: gram.y,v 1.28 2008/06/26 14:46:43 jw Exp $";
/********************************************************************
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <ic@je-wulff.de>
 *
 *	ANSI C Grammar
 *
 *	The ANSI C scanner/parser was posted on the net by Jeff Lee,
 *	in 1985. His net address is:
 *		jeff%gatech.CSNet@CSNet-Relay.ARPA, or jeff@gatech
 *	There was no open source license I could find.
 *
 *	Acknowledgement: Jeff Lee and the draft ANSI C standard grammar.
 *	Kernighan and Ritchie "The C Programming Language" 2nd Ed. A13.
 *
 *	This grammar was expanded to support most of the latest extensions
 *	used in gcc.
 *
 *	gram.y
 *	C grammar for immcc compiler
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<assert.h>
#include	<stdarg.h>
#include	<string.h>

#include	"icc.h"
#include	"comp.h"

#define ENDSTACKSIZE	100
// #define LEAS		2
#define LEAI		170
#define LARGE		(~0U>>2)
static const char	idOp[] = "+-+-";	/* increment/decrement operator selection */

static Symbol		typedefSymbol = { "typedef", UDF, UDFA, };

#ifndef LMAIN
typedef struct LineEntry {
    unsigned int	pStart;
    unsigned int	vStart;
    unsigned int	equOp;
    unsigned int	vEnd;
    unsigned int	pEnd;
    Symbol *		sp;
    int			ppIdx;
} LineEntry;

static LineEntry *	lineEntryArray = NULL;
static LineEntry *	lep = NULL;		/* associated array allocated from heap */
#ifdef LEAS
static unsigned int	udfCount = LEAS + 2;	/* start with guard value space */
#else
static unsigned int	udfCount = LEAI;	/* 170 is approx 4 kB */
#endif
static unsigned int	endStack[ENDSTACKSIZE];
static unsigned int *	esp = endStack;

static void		immVarFound(unsigned int start, unsigned int end, Symbol* sp);
static void		immVarRemove(unsigned int start, unsigned int end, Symbol* sp);
static void		immAssignFound(unsigned int start, unsigned int operator,
			    unsigned int end, Symbol* sp, int ppi);
static unsigned int	pushEndStack(unsigned int value);
static unsigned int	popEndStack(void);
#else	/* LMAIN */
static void		yyerror(char *s, ...);
#endif	/* LMAIN */
%}
%union {					/* stack type */
    Token		tok;
}

%token	<tok> IDENTIFIER IMM_IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token	<tok> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	<tok> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	<tok> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	<tok> XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token	<tok> TYPEDEF TYPEOF EXTERN STATIC AUTO REGISTER
%token	<tok> CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token	<tok> STRUCT UNION ENUM ELIPSIS

%token	<tok> CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%type	<tok> translation_unit external_declaration function_definition function_body
%type	<tok> declaration declaration_list declaration_specifiers storage_class_specifier
%type	<tok> type_specifier type_qualifier struct_or_union_specifier struct_or_union_tag
%type	<tok> struct_or_union struct_declaration_list init_declarator_list init_declarator
%type	<tok> struct_declaration specifier_qualifier_list struct_declarator_list
%type	<tok> struct_declarator enum_specifier enumerator_list_comma enumerator_list
%type	<tok> enumerator declarator direct_declarator pointer type_qualifier_list
%type	<tok> parameter_head parameter_type_list parameter_identifier_list parameter_list
%type	<tok> parameter_declaration identifier_list initializer initializer_list
%type	<tok> type_name abstract_declarator direct_abstract_declarator statement
%type	<tok> labeled_statement expression_statement compound_statement statement_list
%type	<tok> selection_statement iteration_statement jump_statement expression
%type	<tok> assignment_expression assignment_operator conditional_expression
%type	<tok> constant_expression logical_OR_expression logical_AND_expression
%type	<tok> inclusive_OR_expression exclusive_OR_expression AND_expression
%type	<tok> equality_expression relational_expression shift_expression
%type	<tok> additive_expression multiplicative_expression cast_expression
%type	<tok> unary_expression unary_operator postfix_expression primary_expression
%type	<tok> imm_unary_expression imm_postfix_expression
%type	<tok> argument_expr_list string_literal imm_identifier

%type	<tok>	'{' '[' '(' ')' ']' '}'
%right	<tok>	'=' ':' ';' ','
%right	<tok>	'&' '*' '+' '-' '~' '!'

%start translation_unit
%%

/********************************************************************
 *
 *	start of C grammar
 *
 *******************************************************************/

translation_unit				/* 1 */
	: /* NOTHING allows an empty translation_unit as in gcc */ {
	    $$.start = 0;
	    $$.end = 0;
	    $$.symbol = NULL;
	}
	| translation_unit external_declaration {
	    $$.start = $1.start ? $1.start : $2.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

external_declaration				/* 2 */
	: function_definition {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| declaration {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	;

function_definition				/* 3 */
	: declarator function_body {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
#ifndef LMAIN
	    clearParaList(0);			/* restore all overloaded parameters */
#endif
	}
	| declaration_specifiers declarator {
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as function name */
#endif
	}
					    function_body {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    delete_sym(&$2);			/* not deleted if imm */
#ifndef LMAIN
	    clearParaList(0);			/* restore all overloaded parameters */
#endif
	}
	;

function_body					/* 4 */
	: compound_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| declaration_list compound_statement {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

declaration					/* 5 */
	: declaration_specifiers ';' {	/* may have empty init_declarator_list according to K&R */
	    $$.start = $1.start;	/* GCC issues a warning: */
	    $$.end = $2.end;		/* "useless keyword or typname in empty declaration" */
	    $$.symbol = NULL;		/* (see also rule 17) */
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as local var */
#endif
	}
	| declaration_specifiers init_declarator_list ';' {
	    Symbol *	sp;
	    $$.start = $1.start;
	    $$.end = $3.end;
	    sp = $2.symbol;
	    assert(sp);			/* ERROR: initialized in : init_declarator */
	    while (sp) {
		Symbol * sp1 = sp;
		sp = sp->next;		/* get next before Symbol is placed or deleted */
		if ($1.symbol && $1.symbol->type == UDF) {
		    sp1->type = CTYPE;		/* found a typedef */
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "\nP %-15s %d %d\n", sp1->name, sp1->type, sp1->ftype);
#endif
		    if (lookup(sp1->name) == 0) {
			link_sym(sp1);
		    }
		} else {
		    $$.symbol = sp1;		/* use $$ as transport Token for delete_sym */
		    delete_sym(&$$);
		}
	    }
	    $$.symbol = NULL;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as local var */
#endif
	}
	| declaration_specifiers error ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as local var */
#endif
	    yyclearin; yyerrok;
	}
	;

declaration_list				/* 6 */
	: declaration {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| declaration_list declaration {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

declaration_specifiers				/* 7 -- at least 1 type */
	: specifier_qualifier_list {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
#ifndef LMAIN
	    if (lexflag & C_FUNCTION) {
		lexflag |= C_PARA;		/* function internal C var is coming */
	    }
#endif
	}
	| storage_class_specifier specifier_qualifier_list {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = $1.symbol;		/* typedef information */
#ifndef LMAIN
	    if (lexflag & C_FUNCTION) {
		lexflag |= C_PARA;		/* function internal C var is coming */
	    }
#endif
	}
	;

storage_class_specifier				/* 8 */
	: AUTO {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| STATIC {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| EXTERN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| TYPEDEF {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = &typedefSymbol;
	}
	;

type_specifier					/* 9 */
	: VOID {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| REGISTER {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| CHAR {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| SHORT {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| INT {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| LONG {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| FLOAT {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| DOUBLE {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| SIGNED {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| UNSIGNED {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| struct_or_union_specifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| enum_specifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| TYPE_NAME {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| TYPEOF '(' unary_expression ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| TYPEOF '(' unary_expression error ')' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| TYPEOF '(' type_name ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| TYPEOF '(' type_name error ')' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

type_qualifier					/* 10 */
	: CONST {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| VOLATILE {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	;

struct_or_union_specifier			/* 11 */
	: struct_or_union_tag {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| struct_or_union_tag '{' struct_declaration_list '}' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| struct_or_union_tag '{' struct_declaration_list error '}' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

struct_or_union_tag				/* 12 */
	: struct_or_union {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| struct_or_union IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	}
	/* struct or union tags have a different name space to types */
	| struct_or_union TYPE_NAME {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	/* struct or union tags have a different name space to imm */
	| struct_or_union imm_identifier {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

struct_or_union					/* 13 */
	: STRUCT {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| UNION {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	;

struct_declaration_list				/* 14 */
	: /* NOTHING */ {		/* optional in modern C compilers (not K&R) */
	    $$.start = 0;
	    $$.end = 0;
	    $$.symbol = NULL;
	}
	| struct_declaration_list struct_declaration {
	    $$.start = $1.start ? $1.start : $2.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

init_declarator_list				/* 15 */
	: init_declarator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = $1.symbol;
	}
	| init_declarator_list ',' init_declarator {
	    Symbol *	sp;
	    $$.start = $1.start;
	    $$.end = $3.end;
	    sp = $$.symbol = $1.symbol;
	    assert(sp);			/* ERROR: initialized in : init_declarator */
	    if (sp->type == CWORD) {
		while (sp->next) {
		    sp = sp->next;
		}
		sp->next = $3.symbol;	/* place in a list */
	    }
	}
	;

init_declarator					/* 16 */
	: declarator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = $1.symbol;
	}
	| declarator '=' initializer {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $1.symbol;
	}
	;

struct_declaration				/* 17 */
	: specifier_qualifier_list ';' {/* nameless struct/union for moder C (not K&R) */
	    $$.start = $1.start;	/* should only be for struct or union, but GCC */
	    $$.end = $2.end;		/* only issues warning: "declaration does not declare anything" */
	    $$.symbol = NULL;		/* for 'TYPE ;' in a struct_declaration (similar to 'declaration' rule 5) */
	}
	| specifier_qualifier_list struct_declarator_list ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| specifier_qualifier_list error ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

specifier_qualifier_list			/* 18 */
	: type_specifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| type_specifier specifier_qualifier_list {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| type_qualifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| type_qualifier specifier_qualifier_list {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

struct_declarator_list				/* 19 */
	: struct_declarator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| struct_declarator_list ',' struct_declarator {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

struct_declarator				/* 20 */
	: declarator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	| ':' constant_expression {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| declarator ':' constant_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	;

enum_specifier					/* 21 */
	: ENUM '{' enumerator_list_comma '}' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| ENUM '{' enumerator_list_comma error '}' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| ENUM IDENTIFIER '{' enumerator_list_comma '}' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	}
	| ENUM IDENTIFIER '{' enumerator_list_comma error '}' {
	    $$.start = $1.start;
	    $$.end = $6.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| ENUM IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	}
	;

enumerator_list_comma				/* 22 */
	: enumerator_list {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| enumerator_list ',' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

enumerator_list					/* 23 */
	: enumerator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| enumerator_list ',' enumerator {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

enumerator					/* 24 */
	: IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	| IDENTIFIER '=' constant_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	;

declarator					/* 25 */
	: direct_declarator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    if ($1.symbol->type < MAX_LS &&
#if YYDEBUG && ! defined(SYUNION)
		$1.symbol->v_glist == 0
#else
		$1.symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immVarRemove($1.start, $1.end, $1.symbol);
	    }
#endif
	}
	| pointer direct_declarator {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = $2.symbol;
#ifndef LMAIN
	    if ($2.symbol->type < MAX_LS &&
#if YYDEBUG && ! defined(SYUNION)
		$2.symbol->v_glist == 0
#else
		$2.symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immVarRemove($2.start, $2.end, $2.symbol);
	    }
#endif
	}
	;

direct_declarator				/* 26 */
	: IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = $1.symbol;
	}
	| imm_identifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = $1.symbol;
	}
	| '(' declarator ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $2.symbol;
	}
	| '(' declarator error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $2.symbol;
	    yyclearin; yyerrok;
	}
	| direct_declarator '[' ']' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $1.symbol;
	}
	| direct_declarator '[' error ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $1.symbol;
	    yyclearin; yyerrok;
	}
	| direct_declarator '[' constant_expression ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $1.symbol;
	}
	| direct_declarator '[' constant_expression error ']' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = $1.symbol;
	    yyclearin; yyerrok;
	}
	| parameter_head ')' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif
	}
	| parameter_head error ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif
	    yyclearin; yyerrok;
	}
	| parameter_head parameter_type_list ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif
	}
	| parameter_head parameter_type_list error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif
	    yyclearin; yyerrok;
	}
	| parameter_head parameter_identifier_list ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif
	}
	| parameter_head parameter_identifier_list error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif
	    yyclearin; yyerrok;
	}
	;

parameter_head					/* 26a */
	: direct_declarator '('	{
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    clearParaList(1);			/* imm vars are temporarily overloaded */
#endif
	}
	;

pointer						/* 27 */
	: '*' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| '*' type_qualifier_list {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| '*' pointer {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| '*' type_qualifier_list pointer {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

type_qualifier_list				/* 28 */
	: type_qualifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| type_qualifier_list type_qualifier {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

parameter_type_list				/* 29 */
	: parameter_list {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| parameter_list ',' ELIPSIS {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

parameter_identifier_list			/* 30 */
	: identifier_list {			/* K&R C parameter list without types */
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| identifier_list ',' ELIPSIS {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

parameter_list					/* 31 */
	: parameter_declaration {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| parameter_list ',' parameter_declaration {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

parameter_declaration				/* 32 */
	: specifier_qualifier_list declarator {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	}
	| type_name {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	;

identifier_list					/* 33 */
	: IDENTIFIER {				/* K&R C parameter list without types */
	    $$.start = $1.start;
	    $$.end = $1.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	| identifier_list ',' IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$3);
	    $$.symbol = NULL;
	}
	;

initializer					/* 34 */
	: assignment_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| '{' initializer_list '}' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| '{' initializer_list error '}' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| '{' initializer_list ',' '}' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| '{' initializer_list error ',' '}' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

initializer_list				/* 35 */
	: initializer {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| initializer_list ',' initializer {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

type_name					/* 36 */
	: specifier_qualifier_list {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| specifier_qualifier_list abstract_declarator {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

abstract_declarator				/* 37 */
	: pointer {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| direct_abstract_declarator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| pointer direct_abstract_declarator {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

direct_abstract_declarator			/* 38 */
	: '(' abstract_declarator ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| '(' abstract_declarator error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| '[' ']' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| '[' error ']' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| '[' constant_expression ']' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| '[' constant_expression error ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| direct_abstract_declarator '[' ']' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| direct_abstract_declarator '[' error ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| direct_abstract_declarator '[' constant_expression ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| direct_abstract_declarator '[' constant_expression error ']' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| '(' ')' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| '(' error ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| '(' parameter_type_list ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| '(' parameter_type_list error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| direct_abstract_declarator '(' ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| direct_abstract_declarator '(' error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| direct_abstract_declarator '(' parameter_type_list ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| direct_abstract_declarator '(' parameter_type_list error ')' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

statement					/* 39 */
	: labeled_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| expression_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| compound_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| selection_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| iteration_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| jump_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	;

labeled_statement				/* 40 */
	: IDENTIFIER ':' statement {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	| CASE constant_expression ':' statement {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| DEFAULT ':' statement {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

expression_statement				/* 41 */
	: ';' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| expression ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| expression error ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

compound_statement				/* 42 */
	: '{' '}' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| '{' error '}' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| '{' statement_list '}' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| '{' statement_list error '}' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| '{' declaration_list '}' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| '{' declaration_list error '}' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| '{' declaration_list statement_list '}' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| '{' declaration_list statement_list error '}' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

statement_list					/* 43 */
	: statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| statement_list statement {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

selection_statement				/* 44 */
	: IF '(' expression ')' statement {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	}
	| IF '(' expression ')' statement ELSE statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	    $$.symbol = NULL;
	}
	| SWITCH '(' expression ')' statement {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	}
	;

iteration_statement				/* 45 */
	: WHILE '(' expression ')' statement {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	}
	| DO statement WHILE '(' expression ')' ';' {
	    $$.start = $1.start;
	    $$.end = $7.end;
	    $$.symbol = NULL;
	}
	| FOR '(' ';' ';' ')' statement {
	    $$.start = $1.start;
	    $$.end = $6.end;
	    $$.symbol = NULL;
	}
	| FOR '(' ';' ';' expression ')' statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	    $$.symbol = NULL;
	}
	| FOR '(' ';' expression ';' ')' statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	    $$.symbol = NULL;
	}
	| FOR '(' ';' expression ';' expression ')' statement {
	    $$.start = $1.start;
	    $$.end = $8.end;
	    $$.symbol = NULL;
	}
	| FOR '(' expression ';' ';' ')' statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	    $$.symbol = NULL;
	}
	| FOR '(' expression ';' ';' expression ')' statement {
	    $$.start = $1.start;
	    $$.end = $8.end;
	    $$.symbol = NULL;
	}
	| FOR '(' expression ';' expression ';' ')' statement {
	    $$.start = $1.start;
	    $$.end = $8.end;
	    $$.symbol = NULL;
	}
	| FOR '(' expression ';' expression ';' expression ')' statement {
	    $$.start = $1.start;
	    $$.end = $9.end;
	    $$.symbol = NULL;
	}
	;

jump_statement					/* 46 */
	: GOTO IDENTIFIER ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	}
	| CONTINUE ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| BREAK ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| RETURN ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| RETURN expression ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| RETURN expression error ';' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

expression					/* 47 */
	: assignment_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| expression ',' assignment_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

assignment_expression				/* 48 */
	: conditional_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	/* unary_expression can be lvalue via '*' cast_expression */
	| unary_expression assignment_operator assignment_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	/* imm_unary_expression can never be lvalue - use imm_identifier */
	| imm_identifier assignment_operator assignment_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "\n<%u = %u>\n", $$.start, $$.end);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		$1.symbol->v_glist == 0
#else
		$1.symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immAssignFound($1.start, $2.start, $3.end, $1.symbol, 6);
	    }
#endif
	    $$.symbol = NULL;
	}
	;

assignment_operator				/* 49 */
	: '=' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| MUL_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| DIV_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| MOD_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| ADD_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| SUB_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| LEFT_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| RIGHT_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| AND_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| XOR_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| OR_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	;

conditional_expression				/* 50 */
	: logical_OR_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| logical_OR_expression '?' logical_OR_expression ':' conditional_expression {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	}
	/* see info gcc: 5.8 Conditionals with Omitted Operands  */
	| logical_OR_expression '?' ':' conditional_expression {
	    $$.start = $1.start;
	    $$.end = $4.end;
#ifndef LMAIN
	    if (iC_Pflag) {
		Symbol * sp = $4.symbol;
		warning("ISO C forbids omitting the middle term of a ?: expression", sp ? sp->name : NULL);
	    }
	    $$.symbol = NULL;
#endif
	}
	;

constant_expression				/* 51 */
	: conditional_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	;

logical_OR_expression				/* 52 */
	: logical_AND_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| logical_OR_expression OR_OP logical_AND_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

logical_AND_expression				/* 53 */
	: inclusive_OR_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| logical_AND_expression AND_OP inclusive_OR_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

inclusive_OR_expression				/* 54 */
	: exclusive_OR_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| inclusive_OR_expression '|' exclusive_OR_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

exclusive_OR_expression				/* 55 */
	: AND_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| exclusive_OR_expression '^' AND_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

AND_expression					/* 56 */
	: equality_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| AND_expression '&' equality_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

equality_expression				/* 57 */
	: relational_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| equality_expression EQ_OP relational_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| equality_expression NE_OP relational_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

relational_expression				/* 58 */
	: shift_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| relational_expression '<' shift_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| relational_expression '>' shift_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| relational_expression LE_OP shift_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| relational_expression GE_OP shift_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

shift_expression				/* 59 */
	: additive_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| shift_expression LEFT_OP additive_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| shift_expression RIGHT_OP additive_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

additive_expression				/* 60 */
	: multiplicative_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| additive_expression '+' multiplicative_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| additive_expression '-' multiplicative_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

multiplicative_expression			/* 61 */
	: cast_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| multiplicative_expression '*' cast_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| multiplicative_expression '/' cast_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| multiplicative_expression '%' cast_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

cast_expression					/* 62 */
	: unary_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| imm_unary_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| '(' type_name ')' cast_expression {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| '(' type_name error ')' cast_expression {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

unary_expression				/* 63 */
	: postfix_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| INC_OP unary_expression {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| DEC_OP unary_expression {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| unary_operator cast_expression {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| SIZEOF unary_expression {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| SIZEOF '(' type_name ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| SIZEOF '(' type_name error ')' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

imm_unary_expression				/* 63a */
	: imm_postfix_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = $1.symbol;
	}
	| INC_OP imm_unary_expression {
	    $$.start = $1.start;
	    $$.end = $2.end;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "\n<++%u %u>\n", $$.start, $$.end);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		$2.symbol->v_glist == 0
#else
		$2.symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immAssignFound($2.start, $1.start, $2.end, $2.symbol, 0);
	    }
#endif
	    $$.symbol = NULL;
	}
	| DEC_OP imm_unary_expression {
	    $$.start = $1.start;
	    $$.end = $2.end;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "\n<--%u %u>\n", $$.start, $$.end);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		$2.symbol->v_glist == 0
#else
		$2.symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immAssignFound($2.start, $1.start, $2.end, $2.symbol, 1);
	    }
#endif
	    $$.symbol = NULL;
	}
	| SIZEOF imm_unary_expression {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

unary_operator					/* 64 */
	: '&' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| '*' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| '+' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| '-' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| '~' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| '!' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	;

postfix_expression				/* 65 */
	: primary_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| postfix_expression '[' expression ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| postfix_expression '[' expression error ']' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| postfix_expression '(' ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| postfix_expression '(' error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| postfix_expression '(' argument_expr_list ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| postfix_expression '(' argument_expr_list error ')' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| postfix_expression '.' IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$3);
	    $$.symbol = NULL;
	}
	| postfix_expression PTR_OP IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$3);
	    $$.symbol = NULL;
	}
	| postfix_expression INC_OP {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| postfix_expression DEC_OP {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

imm_postfix_expression				/* 65a */
	: imm_identifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = $1.symbol;
	}
	| imm_postfix_expression INC_OP {
	    $$.start = $1.start;
	    $$.end = $2.end;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "\n<%u %u++>\n", $$.start, $$.end);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		$1.symbol->v_glist == 0
#else
		$1.symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immAssignFound($1.start, $2.start, $2.end, $1.symbol, 2);
	    }
#endif
	    $$.symbol = NULL;
	}
	| imm_postfix_expression DEC_OP {
	    $$.start = $1.start;
	    $$.end = $2.end;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "\n<%u %u-->\n", $$.start, $$.end);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		$1.symbol->v_glist == 0
#else
		$1.symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immAssignFound($1.start, $2.start, $2.end, $1.symbol, 3);
	    }
#endif
	    $$.symbol = NULL;
	}
	;

primary_expression				/* 66 */
	: IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	| CONSTANT {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| string_literal {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| '(' expression ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| '(' expression error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| '(' compound_statement ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| '(' compound_statement error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

argument_expr_list				/* 67 */
	: assignment_expression {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| argument_expr_list ',' assignment_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

string_literal					/* 68 */
	: STRING_LITERAL {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| string_literal STRING_LITERAL {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

imm_identifier					/* 69 */
	: IMM_IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "[%u %u]\n", $$.start, $$.end);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		$$.symbol->v_glist == 0
#else
		$$.symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immVarFound($$.start, $$.end, $1.symbol);
	    }
#endif
	}
	| '(' imm_identifier ')' {
	    /* stops this being a primary_expression which would lead to C assignment */
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $2.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "{%u %u}\n", $$.start, $$.end);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		$$.symbol->v_glist == 0
#else
		$$.symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immVarFound($$.start, $$.end, NULL);	/* moves pStart and pEnd without changing vStart vEnd */
	    }
#endif
	}
	;

/********************************************************************
 *
 *	end of C grammar
 *
 *******************************************************************/
%%

#ifdef LMAIN
static void
yyerror(char *s, ...)
{
    fprintf(iC_outFP, "\n%*s\n%*s\n", column, "^", column, s);
    fflush(iC_outFP);
} /* yyerror */

void
ierror(						/* print error message */
    char *	str1,
    char *	str2)
{
    fprintf(iC_outFP, "*** Error: %s", str1);
    if (str2) {
	fprintf(iC_outFP, " %s", str2);
    } else {
	putc('.', iC_outFP);
    }
    fprintf(iC_outFP, "\n");
    fflush(iC_outFP);
} /* ierror */
#else

static char*	macro[] = { MACRO_NAMES };

/********************************************************************
 *
 *	immVarFound
 *
 *	The parser has found a bare immediate variable or an immediate
 *	variable in parentheses. (zany but allowed as an lvalue in C)
 *	To locate parentheses, only pStart and pEnd are adjusted.
 *	vStart and vEnd still mark the original variable.
 *
 *	sp == 0 is used to signal parenthesized immediate variable.
 *
 *******************************************************************/

static void
immVarFound(unsigned int start, unsigned int end, Symbol* sp)
{
    LineEntry *	newArray;

    if (sp) {
	lep->vStart = start;			/* of an imm variable */
	lep->equOp  = LARGE;			/* marks a value variable */
	lep->vEnd   = end;			/* of an imm variable */
	lep->sp     = sp;
	lep->ppIdx  = 6;
	if (sp->ftype != ARITH && sp->ftype != GATE && sp->type != ERR) {
	    ierror("C-statement tries to access an imm type not bit or int:", sp->name);
	    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
	}
    } else {					/* parenthesized variable found */
	--lep;					/* step back to previous entry */
    }
    lep->pStart = start;			/* of possible parentheses */
    lep->pEnd   = end;				/* of possible parentheses */
    lep++;
    if (lep > &lineEntryArray[udfCount-2]) {	/* allow for 2 guard entries at end */
	udfCount += LEAI;			/* increase the size of the array */
	newArray = (LineEntry*)realloc(lineEntryArray, udfCount * sizeof(LineEntry));
	assert(newArray);			/* FIX with quit */
	lep += newArray - lineEntryArray;
	lineEntryArray = newArray;		/* Array has been successfully resized */
    }
} /* immVarFound */

/********************************************************************
 *
 *	immVarRemove
 *
 *	The parser has found a bare immediate variable which is being
 *	declared as a C variable in C code. Remove it from the
 *	lineEntryArray and mark it so it is returned as a C-IDENTIFIER
 *	either globally or for the duration of this function. This
 *	effectively hides the imm Symbol with the same name in C code.
 *
 *******************************************************************/

static void
immVarRemove(unsigned int start, unsigned int end, Symbol* sp)
{
    --lep;					/* step back to previous entry */
    if(lep->pStart == start && lep->pEnd == end) {
	markParaList(sp);			/* mark imm Symbol so it is hidden */
	lep->pStart = lep->equOp = LARGE;
    } else {
#if YYDEBUG
	if ((iC_debug & 0402) == 0402)
	    fprintf(iC_outFP, "stack position (remove): start %u (%u) end %u (%u) Symbol %s\n",
	    lep->pStart, start, lep->pEnd, end, lep->sp->name);
#endif
	ierror("C name which is an imm variable cannot be hidden:", sp->name);
	lep++;					/* do not remove */
    }
} /* immVarRemove */

/********************************************************************
 *
 *	immAssignFound
 *
 *	The parser has found an assignment to an immediate variable
 *	The assignment operator may be either
 *	= which is simple assignment or
 *	+= -= *= /= %= &= ^= |= >>= <<= which is an operator assignment or
 *	++ -- with ppi 0 or 1, which is pre-increment/decrement or
 *	++ -- with ppi 2 or 3, which is post-increment/decrement.
 *
 *	Backtrack along the immediate variables found so far, to find
 *	the one being assigned to here. Its 'lep' entry is modified.
 *	equOp points to an actual assignment or locates inc/dec-operator.
 *	vStart and vEnd still mark the variable being assigned to.
 *	pEnd is adjusted to the very end of the expression being assigned,
 *	where the closing bracket for the assignment macro hast to be placed.
 *
 *	If no suitable immediate variable is found it is a compiler
 *	error, because an immediate assignment should have a bare
 *	immediate variable as its first token (same start position)
 *	Alternatively an immediate variable in parentheses is accepted.
 *
 *	Checked earlier that IMM_IDENTIFIER is ftype ARITH or GATE.
 *
 *	ftyp is derived from the ftype of the Symbol. It may only be
 *	ARITH === 1, GATE === 2, ARITH+2 and GATE+2. (UDFA === 0 is an error)
 *
 *	This did not work. jw 2004.02.23 found with Electric Fence 2.2.1
 *	in arnc5.ic:
 *		imm clock clk; if (IX0.1) { clk = 1; } // tries to use
 *		macro[ftyp] with ftyp = CLCKL 18, type = ERR 19 // FIXED
 *
 *	Use of ppIdx, ppi and equOp, operator:
 *	In immVarFound() p->ppIdx is initialized to 6 and p->equOp to LARGE
 *	which is then described as a used variable. If equOp is set to some
 *	operator position it becomes an assigned operator. All other imm
 *	variables in an assignment, which have ppIdx == 6 have the value
 *	set to 5. This marks the variable as used, even if it is an assignment
 *	variable. By this method multiple assignments are handled correctly.
 *	If ppi < 5 the current variable is involved in a pre/post-inc/dec
 *	operation and is marked as both assigned and used anyway.
 *
 *******************************************************************/

static void
immAssignFound(unsigned int start, unsigned int operator, unsigned int end, Symbol* sp, int ppi)
{
    LineEntry *	p;
    int		ftyp;
    int		typ;

    assert(sp);
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "start %u, op %u end %u sp =>%s ppi %d\n", start, operator, end, sp->name, ppi);
#endif

    for (p = lep - 1; p >= lineEntryArray; p-- ) {
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "--%u(%u %u %u)%u %d\n", p->pStart, p->vStart, p->equOp==LARGE?0:p->equOp, p->vEnd, p->pEnd, p->ppIdx);
#endif
	if (p->pStart == start) {		/* start position of imm variable assigned to */
	    p->equOp  = operator;		/* position of operator marks an assignment expression */
	    p->pEnd   = end;			/* end position of expression assigned from */
	    p->ppIdx  = ppi;			/* pre/post-inc/dec character value */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "= %u(%u %u %u)%u %d\n", p->pStart, p->vStart, p->equOp==LARGE?0:p->equOp, p->vEnd, p->pEnd, p->ppIdx);
#endif
	    if (p->sp == sp) {
		typ = sp->type;
		ftyp = sp->ftype;
		if (typ == UDF) {
		    if (iC_Sflag) {
			ierror("strict: C assignment to an imm variable (should be immC):", sp->name);
			if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
			else goto changeType;
		    } else {
			changeType:
			sp->type = iC_ctypes[ftyp];	/* must be ARNC or LOGC */
			listGenOut(sp);		/* list immC node and generate possible output */
		    }
		} else if ((typ != ARNC || ftyp != ARITH) &&
			   (typ != LOGC || ftyp != GATE) &&
			   (typ != ERR)) {	/* avoids multiple error messages */
		    if (((typ == ARN || typ == ARNF) && ftyp == ARITH) ||
		        (typ >= MIN_GT && typ < MAX_GT && ftyp == GATE)) {
			ierror("C-assignment to an imm variable already assigned in iC code:", sp->name);
		    } else {
			ierror("C-assignment to an incompatible imm type:", sp->name);
		    }
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		}
		return;
	    } else {
		break;				/* Error: Symbols don't match */
	    }
	} else if (p->pStart < start) {
	    break;				/* Error: will not find start */
	}
	if (p->ppIdx == 6) {			/* all other imm values in the assignment */
	    p->ppIdx = 5;			/* are used even if themselves assigned to */
	}
    }
    execerror("C-assignment: Symbol not found ???", sp->name, __FILE__, __LINE__);
} /* immAssignFound */

/********************************************************************
 *
 *	pushEndStack
 *
 *******************************************************************/

static unsigned int
pushEndStack(unsigned int value)
{
    assert(esp < &endStack[ENDSTACKSIZE]);
    return *esp++ = value;
} /* pushEndStack */

/********************************************************************
 *
 *	popEndStack
 *
 *******************************************************************/

static unsigned int
popEndStack(void)
{
    assert(esp >= endStack);
    return *--esp;
} /* popEndStack */

/********************************************************************
 *
 *	copyAdjustFile
 *
 *	Allocation of the lineEntryArray[] is carried out if iFP == 0.
 *
 *	There is one entry for every occurence of an immediate variable
 *	in the C code. Since only immediate variables which are still
 *	unassigned by the time the C-parse is executed are proper
 *	candidates and if 10 times this number is allowed for, the array
 *	is unlikely to overflow. This count is available by counting
 *	Symbols of type == UDF. This is also done later in listNet(),
 *	but we need the value earlier.
 *
 *	Also other imm variables are used as values, so a generous
 *	first guess and realloc() in immVarFound is a better choice.
 *
 *	Use the byte postions stored in lineEntryArray to insert
 *	macro calls for immediate variables and assignments
 *
 *	'lp' points to the ffexpr head when called as an auxiliary
 *	compile of one if - else or switch C fragment. Else NULL
 *
 *******************************************************************/

void
copyAdjust(FILE* iFP, FILE* oFP, List_e* lp)
{
    int			c;
    LineEntry*		p;
    unsigned int	start;
    unsigned int	vstart;
    unsigned int	vend;
    unsigned int	equop;
    unsigned int	earlyop;
    unsigned int	endop;
    unsigned int	end;
    unsigned int	bytePos;
    int			pFlag;
    int			ppi;
    Symbol *		sp;
    Symbol *		tsp;
    List_e *		lp1;
    List_e *		lp2;
    int			lNr = 0;
    int			sNr = 0;
#ifdef LEAS
    Symbol **		hsp;
#endif
    int			ml;
    int			ftyp;
    Symbol *		fsp = 0;
    static char *	f = "_f0_1";		/* name of literal function head */

    if (iFP == NULL) {
#ifdef LEAS
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if (sp->type == UDF) {
		    udfCount += LEAS;
		}
	    }
	}
#endif
	lineEntryArray = (LineEntry*)realloc(NULL, udfCount * sizeof(LineEntry));
	assert(lineEntryArray);			/* FIX with quit */
	lep = lineEntryArray;
	lep->pStart = lep->equOp = LARGE;	/* guard value in case first immVarFound(0) */
	lep++;
	lep->pStart = lep->equOp = LARGE;	/* value overwritten by first immVarFound */
	return;					/* lineEntryArray initialized */
    }

    if (lp) {					/* ffexpr head link */
	assert(lp->le_val>>8 && lp->le_next == 0); /* must have function # and callled only once */
	tsp = lp->le_sym;			/* master - must be ftype F_CF, F_CE or F_SW */
	assert(tsp && (tsp->ftype == F_CF || tsp->ftype == F_CE || tsp->ftype == F_SW));
	lp1 = tsp->u_blist;
	assert(lp1);
	tsp = lp1->le_sym;			/* clock for ffexpr head */
	assert(tsp);
	if (tsp->ftype == CLCKL) {
	    lNr = 2;				/* start of value list for C fragment */
	} else if (tsp->ftype == TIMRL) {
	    lNr = 3;				/* extra space for timer delay */
	} else {
	    assert(0);				/* must be a clock */
	}
    }

    lep->pStart   = lep->equOp = LARGE;		/* finalise lineEntryArray */
    lep++;
    lep->pStart   = lep->equOp = LARGE;		/* require 2 guard entries at the end */

    bytePos = 0;
    p       = lineEntryArray + 1;		/* miss guard entry at start */
    start   = p->pStart;
    if (p->equOp < start) {
	earlyop = p->equOp;			/* operator in pre-inc/dec entry handled early */
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "load bytePos = %u, earlyop = %u\n", bytePos, earlyop);
#endif
    } else {
	earlyop = LARGE;
    }
    vstart  = LARGE;
    vend    = LARGE;
    equop   = LARGE;
    endop   = LARGE;
    end     = LARGE;
    pFlag   = 1;				/* start by outputting C code till first variable or ++/-- */
    sp      = NULL;

    while ((c = getc(iFP)) != EOF) {
	while (bytePos >= end) {
	    putc(')', oFP);			/* end found - output end */
	    end = popEndStack();
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "popE bytePos = %u, earlyop = %u, end = %u\n", bytePos, earlyop, end);
#endif
	}
	if (bytePos >= start) {
	    pushEndStack(end);			/* push previous end */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "= %u(%u %u %u)%u %d\n", p->pStart, p->vStart, p->equOp==LARGE?0:p->equOp, p->vEnd, p->pEnd, p->ppIdx);
#endif
	    vstart = p->vStart;			/* start of actual variable */
	    vend   = p->vEnd;			/* end of actual variable */
	    equop  = p->equOp;			/* operator in this entry */
	    end    = p->pEnd;			/* end of this entry */
	    sp     = p->sp;			/* associated Symbol */
	    ppi    = p->ppIdx;			/* pre/post-inc/dec character value */
	    assert(sp);
	    ml     = lp		     ? 0		: MACRO_LITERAL;
	    ftyp  = sp->type == ERR ? UDFA
				     : (equop == LARGE) ? sp->ftype
							: sp->ftype + MACRO_OFFSET;
	    if (ppi >= 5) {
		/* assignment macro must be printed outside of enclosing parentheses */
		fprintf(oFP, macro[ml+ftyp]);	/* entry found - output macro start */
	    }
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "strt bytePos = %u, earlyop = %u, equop = %u, ppi = %d, sp =>%s\n", bytePos, earlyop, equop, ppi, sp->name);
#endif
	    p++;				/* next entry to locate possible earlyop */
	    assert(start <= vstart);
	    assert(vstart < vend);
	    assert(vend <= end);
	    assert(bytePos < p->pStart);
	    start = p->pStart;			/* start of next entry */
	    if (p->equOp < start) {
		earlyop = p->equOp;		/* operator in pre-inc/dec entry handled early */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "load bytePos = %u, earlyop = %u\n", bytePos, earlyop);
#endif
	    } else {
		earlyop = LARGE;
	    }
	}
	if (bytePos == vstart) {
	    assert(sp);				/* start of actual variable */
	    if (lp) {
		functionUse[0].c_cnt |= F_FFEXPR;	/* flag for copying ffexpr macro */
		lp1 = lp;
	    } else {
		functionUse[0].c_cnt |= F_LITERAL;	/* flag for copying literal macro */
		if (fsp) {
		    lp1 = fsp->list;		/* start with head of pointer list */
		} else
		if ((fsp = lookup(f)) == 0) {
		    fsp = install(f, OR, F_CF);	/* install new literal function head */
		    lp1 = sy_push(fsp);		/* head of literal pointer list */
		    fsp->list = lp1;		/* save in list for more pointers */
		} else {
		    if (fsp->type != CF && fsp->ftype != F_CF) {
			ierror("use of literal function head which was previously used for a different purpose:", f);
		    }
		    lp1 = fsp->list;		/* start with head of pointer list */
		}
		lNr = 0;
	    }
	    sNr = lNr;				/* start at array pos after clock or timer,val */
	    while ((lp2 = lp1->le_next) != 0) {
		if (sp == lp2->le_sym) {
		    assert((lp2->le_val & VAR_MASK) == sNr);
		    break;			/* variable occurred previously */
		}
		sNr++;
		assert(sNr < VAR_MASK);		/* limits # of variables to 16.384 if short */
		lp1 = lp2;
	    }
	    if (lp2 == 0) {
		lp2 = sy_push(sp);		/* new variable in this C fragment */
		lp2->le_val |= sNr;		/* offset in list */
		lp1->le_next = lp2;		/* place at end of list */
	    }
	    if (equop == LARGE) {
		lp2->le_val |= VAR_USE;		/* variable value is only used */
	    } else {
		lp2->le_val |= VAR_ASSIGN;	/* variable is assigned to */
		if (ppi < 6) {
		    lp2->le_val |= VAR_USE;	/* pre/post-inc/dec or marked as used */
		}
	    }
	    if (ppi >= 5) {
		fprintf(oFP, "%d", sNr);	/* output Symbol pointer offset */
	    } else {
		/* expanded pre/post-inc/dec macro may be printed inside enclosing parentheses */
		if (ppi < 2){
		    /* pre-increment/decrement */
		    fprintf(oFP, "%s%d , %s%d) %c 1",
			macro[ml+ftyp], sNr, macro[ml+sp->ftype], sNr, idOp[ppi]);
		    /* ++x; produces: iC_AA(2 , iC_AV(2) + 1); */
		} else {
		    /* post-increment/decrement */
		    iC_Tflag = 1;		/* triggers definition of iC_tVar in C outfile */
		    fprintf(oFP, "(iC_tVar = %s%d), %s%d , iC_tVar %c 1), iC_tVar",
			macro[ml+sp->ftype], sNr, macro[ml+ftyp], sNr, idOp[ppi]);
		    /* x--; produces: (iC_tVar = iC_AV(2), iC_AA(2 , iC_tVar - 1), iC_tVar); */
		}
	    }
	    pFlag = 0;				/* start of variable name, which is replaced by macro */
	}
	if (bytePos == vend) {
	    pFlag = 1;				/* end of the variable which is the next entry */
	}
	if (bytePos == equop) {
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "use  bytePos = %u, earlyop = %u, equop = %u\n", bytePos, earlyop, equop);
#endif
	    if (ppi >= 5) {
		if (c == '=') {
		    c = ',';			/* simple assignment, replace '=' by ',' */
		} else {
		    assert(sp);
		    assert(strchr("+-*/%&^|><", c));
		    /********************************************************************
		     *  output variable as value for operator assignment expression
		     *  QB1 *= IB2 + IB3; produces:
		     *  iC_AA(2 , iC_AV(2) * ( iC_AV(3) + iC_AV(4)));
		     *  NOTE: parenthesis around operator assigned expression
		     *******************************************************************/
		    fprintf(oFP, ", %s%d) ", macro[ml+sp->ftype], sNr);
		    endop = equop + ((c == '>' || c == '<') ? 2 : 1);	/* replace '=' by " (" */
		    pushEndStack(end);		/* push extra end ')' for operator assign expression */
		    assert(lp2);
		    lp2->le_val |= VAR_USE;	/* operator assignment is also used */
		}
	    } else {
		assert(ppi >= 2);		/* equop occurred before start for pre-inc/dec */
		endop = bytePos + 1;		/* ppi >= 2 && < 5 is post-inc/dec at equop */
		pFlag = 0;			/* suppress output of ++ or -- from post-inc/dec */
	    }
	}
	if (bytePos == earlyop) {
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "use  bytePos = %u, earlyop = %u\n", bytePos, earlyop);
#endif
	    endop = bytePos + 1;
	    pFlag = 0;				/* suppress output of ++ or -- from pre-inc/dec */
	}
	if (pFlag) {
	    if (bytePos != endop) {
		putc(c, oFP);			/* output all except variables and ++ or -- from inc/dec */
	    } else {
		fprintf(oFP, " (");		/* replace '=' by " (" for operator assignment */
	    }
	}
	if (bytePos == endop) {
	    endop = LARGE;
	    pFlag = 1;
	}
	bytePos++;
    }
    if ((iC_debug & 04) && (fsp = lookup(f)) != 0) {
	/********************************************************************
	 * compile listing output for literal function block
	 *******************************************************************/
	lp1 = fsp->list;			/* start with head of pointer list */
	assert(lp1->le_sym == fsp);
	fprintf(iC_outFP, "\n\n\t%s\t%c ---%c\t\t\t// (L)\n", fsp->name, iC_fos[fsp->ftype],
	    iC_os[CF]);
	while ((lp1 = lp1->le_next) != 0) {
	    Symbol *	gp;
	    int		use;
	    gp = lp1->le_sym;
	    assert(gp);
	    assert(gp->name);
	    use = lp1->le_val >> USE_OFFSET;
	    assert(use < Sizeof(iC_useText));
	    fprintf(iC_outFP, "\t%s\t%c<---%c\t\t\t// %d %s", gp->name, iC_fos[gp->ftype],
		iC_os[CF], lp1->le_val & 0xff, iC_useText[use]);
	    if ((iC_debug & 0200) && ! iFunSymExt) {	/* could use liveDisp, if it were global */
		fprintf(iC_outFP, "\t%s\t=", gp->name);
	    }
	    fprintf(iC_outFP, "\n");
	}
    }
} /* copyAdjust */
#endif

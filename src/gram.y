%{ static const char gram_y[] =
"@(#)$Id: gram.y,v 1.9 2002/08/14 16:43:45 jw Exp $";
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
 *	gram.y
 *	C grammar for icc compiler
 *
 *******************************************************************/

#include	<stdio.h>
#include	<assert.h>
#include	<stdarg.h>

#include	"icc.h"
#include	"comp.h"

typedef struct LineEntry {
    unsigned int	pStart;
    unsigned int	vStart;
    unsigned int	equOp;
    unsigned int	vEnd;
    unsigned int	pEnd;
    Symbol *		sp;
} LineEntry;

#define ENDSTACKSIZE	100
// #define LEAS		2
#define LEAI		170
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
static void		immAssignFound(unsigned int start, unsigned int operator,
			    unsigned int end, Symbol* sp);
static unsigned int	pushEndStack(unsigned int value);
static unsigned int	popEndStack(void);

static Symbol	typedefSymbol = { "typedef", UDF, UDFA, };

#ifdef LMAIN
static void	yyerror(char *s, ...);
#endif
%}
%union {		/* stack type */
    Token	tok;
}

%token	<tok> IDENTIFIER IMM_IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token	<tok> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	<tok> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	<tok> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	<tok> XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token	<tok> TYPEDEF TYPEOF EXTERN STATIC AUTO REGISTER
%token	<tok> CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token	<tok> STRUCT UNION ENUM ELIPSIS RANGE

%token	<tok> CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%type	<tok> primary_expr postfix_expr argument_expr_list unary_expr unary_operator
%type	<tok> cast_expr multiplicative_expr additive_expr shift_expr relational_expr
%type	<tok> equality_expr and_expr exclusive_or_expr inclusive_or_expr
%type	<tok> logical_and_expr logical_or_expr conditional_expr assignment_expr
%type	<tok> assignment_operator expr constant_expr declaration declaration_specifiers
%type	<tok> init_declarator_list init_declarator storage_class_specifier
%type	<tok> type_specifier struct_or_union_specifier struct_or_union
%type	<tok> struct_declaration_list struct_declaration struct_declarator_list
%type	<tok> struct_declarator enum_specifier enumerator_list enumerator declarator
%type	<tok> declarator2 pointer type_specifier_list parameter_identifier_list
%type	<tok> identifier_list parameter_type_list parameter_list parameter_declaration
%type	<tok> type_name abstract_declarator abstract_declarator2 initializer
%type	<tok> initializer_list statement labeled_statement compound_statement
%type	<tok> declaration_list statement_list expression_statement selection_statement
%type	<tok> iteration_statement jump_statement file external_definition
%type	<tok> function_definition function_body identifier
%type	<tok> imm_identifier string_literal enumerator_list_comma

%type	<tok>	'{' '[' '(' ')' ']' '}'
%right	<tok>	'=' ':' ';' ','
%right	<tok>	'&' '*' '+' '-' '~' '!'

%start file
%%

/********************************************************************
 *
 *	start of C grammar
 *
 *******************************************************************/

primary_expr
	: identifier {
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
	| '(' expr ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| '(' expr error ')' {
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

string_literal
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

postfix_expr
	: primary_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| postfix_expr '[' expr ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| postfix_expr '[' expr error ']' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| postfix_expr '(' ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| postfix_expr '(' error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| postfix_expr '(' argument_expr_list ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| postfix_expr '(' argument_expr_list error ')' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| postfix_expr '.' identifier {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$3);
	    $$.symbol = NULL;
	}
	| postfix_expr PTR_OP identifier {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$3);
	    $$.symbol = NULL;
	}
	| postfix_expr INC_OP {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| postfix_expr DEC_OP {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

argument_expr_list
	: assignment_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| argument_expr_list ',' assignment_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

unary_expr
	: postfix_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| imm_identifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| INC_OP unary_expr {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| DEC_OP unary_expr {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| unary_operator cast_expr {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| SIZEOF unary_expr {
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

unary_operator
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

cast_expr
	: unary_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| '(' type_name ')' cast_expr {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| '(' type_name error ')' cast_expr {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

multiplicative_expr
	: cast_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| multiplicative_expr '*' cast_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| multiplicative_expr '/' cast_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| multiplicative_expr '%' cast_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

additive_expr
	: multiplicative_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| additive_expr '+' multiplicative_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| additive_expr '-' multiplicative_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

shift_expr
	: additive_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| shift_expr LEFT_OP additive_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| shift_expr RIGHT_OP additive_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

relational_expr
	: shift_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| relational_expr '<' shift_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| relational_expr '>' shift_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| relational_expr LE_OP shift_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| relational_expr GE_OP shift_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

equality_expr
	: relational_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| equality_expr EQ_OP relational_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| equality_expr NE_OP relational_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

and_expr
	: equality_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| and_expr '&' equality_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

exclusive_or_expr
	: and_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| exclusive_or_expr '^' and_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

inclusive_or_expr
	: exclusive_or_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| inclusive_or_expr '|' exclusive_or_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

logical_and_expr
	: inclusive_or_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| logical_and_expr AND_OP inclusive_or_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

logical_or_expr
	: logical_and_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| logical_or_expr OR_OP logical_and_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

conditional_expr
	: logical_or_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| logical_or_expr '?' logical_or_expr ':' conditional_expr {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	}
	;

assignment_expr
	: conditional_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| unary_expr assignment_operator assignment_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
#ifndef LMAIN
#if YYDEBUG
	if ((debug & 0402) == 0402) fprintf(outFP, "@");
#endif
#endif
	}
	| imm_identifier '=' assignment_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
#ifndef LMAIN
#if YYDEBUG
	if ((debug & 0402) == 0402) fprintf(outFP, "\n<%u %u>", $$.start, $$.end);
#endif
	    immAssignFound($1.start, $2.start, $3.end, $1.symbol);
#endif
	    $$.symbol = NULL;
	}
	;

assignment_operator
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

expr
	: assignment_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| expr ',' assignment_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

constant_expr
	: conditional_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	;

declaration
	: declaration_specifiers ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
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
#ifdef YYDEBUG
		    if ((debug & 0402) == 0402) fprintf(outFP, "\nP %-15s %d %d\n", sp1->name, sp1->type, sp1->ftype);
#endif
		    place_sym(sp1);
		} else {
		    $$.symbol = sp1;	/* use $$ as transport Token for delete_sym */
		    delete_sym(&$$);
		}
	    }
	    $$.symbol = NULL;
	}
	| declaration_specifiers error ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

declaration_specifiers
	: type_specifier_list {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| storage_class_specifier type_specifier_list {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = $1.symbol;	/* typedef information */
	}
	| storage_class_specifier identifier {
	    /* this is a fudge to catch undefined types - make it optional */
	    Symbol *	sp;
	    $$.start = $1.start;
	    $$.end = $2.end;
	    sp = $2.symbol;		/* canot be in symbol table */
	    assert(sp);			/* ERROR: initialized in c_lex() */
	    sp->type = CTYPE;		/* found a TYPE_NAME after extern etc */
#ifdef YYDEBUG
	    if ((debug & 0402) == 0402) fprintf(outFP, "\nT %-15s %d %d\n", sp->name, sp->type, sp->ftype);
#endif
	    place_sym(sp);
	    $$.symbol = $1.symbol;	/* typedef information */
	}
	;

init_declarator_list
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
	    while (sp->next) {
		sp = sp->next;
	    }
	    sp->next = $3.symbol;
	}
	;

init_declarator
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

storage_class_specifier
	: TYPEDEF {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = &typedefSymbol;
	}
	| EXTERN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| STATIC {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| AUTO {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	;

type_specifier
	: CHAR {
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
	| CONST {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| VOLATILE {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| VOID {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| REGISTER {
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
	| TYPEOF '(' unary_expr ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| TYPEOF '(' unary_expr error ')' {
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

struct_or_union_specifier
	: struct_or_union identifier '{' struct_declaration_list '}' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	}
	| struct_or_union identifier '{' struct_declaration_list error '}' {
	    $$.start = $1.start;
	    $$.end = $6.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| struct_or_union '{' struct_declaration_list '}' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| struct_or_union '{' struct_declaration_list error '}' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| struct_or_union identifier {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	}
	;

struct_or_union
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

struct_declaration_list
	: /* NOTHING */ {
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

struct_declaration
	: type_specifier_list struct_declarator_list ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| type_specifier_list error ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

struct_declarator_list
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

struct_declarator
	: declarator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	| ':' constant_expr {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| declarator ':' constant_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	;

enum_specifier
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
	| ENUM identifier '{' enumerator_list_comma '}' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	}
	| ENUM identifier '{' enumerator_list_comma error '}' {
	    $$.start = $1.start;
	    $$.end = $6.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| ENUM identifier {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	}
	;

enumerator_list_comma
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

enumerator_list
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

enumerator
	: identifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	| identifier '=' constant_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	;

declarator
	: declarator2 {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = $1.symbol;
	}
	| pointer declarator2 {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = $2.symbol;
	}
	;

declarator2
	: identifier {
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
	| declarator2 '[' ']' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $1.symbol;
	}
	| declarator2 '[' error ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $1.symbol;
	    yyclearin; yyerrok;
	}
	| declarator2 '[' constant_expr ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $1.symbol;
	}
	| declarator2 '[' constant_expr error ']' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = $1.symbol;
	    yyclearin; yyerrok;
	}
	| declarator2 '(' ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $1.symbol;
	}
	| declarator2 '(' error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $1.symbol;
	    yyclearin; yyerrok;
	}
	| declarator2 '(' parameter_type_list ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $1.symbol;
	}
	| declarator2 '(' parameter_type_list error ')' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = $1.symbol;
	    yyclearin; yyerrok;
	}
	| declarator2 '(' parameter_identifier_list ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $1.symbol;
	}
	| declarator2 '(' parameter_identifier_list error ')' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = $1.symbol;
	    yyclearin; yyerrok;
	}
	;

pointer
	: '*' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| '*' type_specifier_list {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| '*' pointer {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| '*' type_specifier_list pointer {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	;

type_specifier_list
	: type_specifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| type_specifier_list type_specifier {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

parameter_identifier_list
	: identifier_list {
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

identifier_list
	: identifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	| identifier_list ',' identifier {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$3);
	    $$.symbol = NULL;
	}
	;

parameter_type_list
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

parameter_list
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

parameter_declaration
	: type_specifier_list declarator {
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

type_name
	: type_specifier_list {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| type_specifier_list abstract_declarator {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

abstract_declarator
	: pointer {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| abstract_declarator2 {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| pointer abstract_declarator2 {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

abstract_declarator2
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
	| '[' constant_expr ']' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| '[' constant_expr error ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| abstract_declarator2 '[' ']' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| abstract_declarator2 '[' error ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| abstract_declarator2 '[' constant_expr ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| abstract_declarator2 '[' constant_expr error ']' {
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
	| abstract_declarator2 '(' ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| abstract_declarator2 '(' error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	| abstract_declarator2 '(' parameter_type_list ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	}
	| abstract_declarator2 '(' parameter_type_list error ')' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

initializer
	: assignment_expr {
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

initializer_list
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

statement
	: labeled_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| compound_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| expression_statement {
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

labeled_statement
	: identifier ':' statement {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	| CASE constant_expr ':' statement {
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

compound_statement
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

declaration_list
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

statement_list
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

expression_statement
	: ';' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| expr ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| expr error ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

selection_statement
	: IF '(' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	}
	| IF '(' expr ')' statement ELSE statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	    $$.symbol = NULL;
	}
	| SWITCH '(' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	}
	;

iteration_statement
	: WHILE '(' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	}
	| DO statement WHILE '(' expr ')' ';' {
	    $$.start = $1.start;
	    $$.end = $7.end;
	    $$.symbol = NULL;
	}
	| FOR '(' ';' ';' ')' statement {
	    $$.start = $1.start;
	    $$.end = $6.end;
	    $$.symbol = NULL;
	}
	| FOR '(' ';' ';' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	    $$.symbol = NULL;
	}
	| FOR '(' ';' expr ';' ')' statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	    $$.symbol = NULL;
	}
	| FOR '(' ';' expr ';' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $8.end;
	    $$.symbol = NULL;
	}
	| FOR '(' expr ';' ';' ')' statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	    $$.symbol = NULL;
	}
	| FOR '(' expr ';' ';' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	    $$.symbol = NULL;
	}
	| FOR '(' expr ';' expr ';' ')' statement {
	    $$.start = $1.start;
	    $$.end = $8.end;
	    $$.symbol = NULL;
	}
	| FOR '(' expr ';' expr ';' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $9.end;
	    $$.symbol = NULL;
	}
	;

jump_statement
	: GOTO identifier ';' {
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
	| RETURN expr ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| RETURN expr error ';' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
	}
	;

file
	: /* NOTHING allows an empty file required for iC */ {
	    $$.start = 0;
	    $$.end = 0;
	    $$.symbol = NULL;
	}
	| file external_definition {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	;

external_definition
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

function_definition
	: declarator function_body {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    delete_sym(&$1);
	    $$.symbol = NULL;
	}
	| declaration_specifiers declarator function_body {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    delete_sym(&$2);
	    $$.symbol = NULL;
	}
	;

function_body
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

identifier
	: IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = $1.symbol;
	}
	;

imm_identifier
	: IMM_IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
#if YYDEBUG
	if ((debug & 0402) == 0402) fprintf(outFP, "[%u %u]", $$.start, $$.end);
#endif
	    immVarFound($$.start, $$.end, $1.symbol);
#endif
	}
	| '(' imm_identifier ')' {
	    /* stops this being a primary_expr which would lead to C assignment */
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $2.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((debug & 0402) == 0402) fprintf(outFP, "{%u %u}", $$.start, $$.end);
#endif
	    immVarFound($$.start, $$.end, NULL);	/* moves pStart and pEnd without changing vStart vEnd */
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
	fflush(stdout);
	printf("\n%*s\n%*s\n", column, "^", column, s);
}
#else

extern int column;
#define LARGE (~0U>>2)

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
	lep->vStart = start;		/* of an imm variable */
	lep->equOp  = LARGE;		/* marks a value variable */
	lep->vEnd   = end;		/* of an imm variable */
	lep->sp     = sp;
	if (sp->ftype != ARITH && sp->ftype != GATE && sp->type != ERR) {
	    error("C-statement tries to access an imm type not bit or int:", sp->name);
	    sp->type = ERR;		/* cannot execute properly */
	}
//fprintf(stderr, "immVarFound: %p %s\n", lep, sp->name); fflush(stderr);
    } else {				/* parenthesized variable found */
	--lep;				/* step back to previous entry */
    }
    lep->pStart = start;		/* of possible parentheses */
    lep->pEnd   = end;			/* of possible parentheses */
    lep++;
    if (lep > &lineEntryArray[udfCount-2]) {	/* allow for 2 guard entries at end */
	udfCount += LEAI;			/* increase the size of the array */
	newArray = (LineEntry*)realloc(lineEntryArray, udfCount * sizeof(LineEntry));
	assert(newArray);		/* FIX with quit */
	lep += newArray - lineEntryArray;
	lineEntryArray = newArray;	/* Array has been successfully resized */
//fprintf(stderr, "immVarFound: %d re-allocated for lineEntryArray %p\n", udfCount, lineEntryArray); fflush(stderr);
    }
} /* immVarFound */

/********************************************************************
 *
 *	immAssignFound
 *
 *	The parser has found an assignment to an immediate variable
 *
 *	Backtrack along the immediate variables found so far, to find
 *	the one being assigned to here. Its 'lep' entry is modified.
 *	Only pEnd is adjusted. vStart and vEnd still mark the variable
 *	being assigned to.
 *
 *	If no suitable immediate variable is found it is a compiler
 *	error, because an immediate assignment should have a bare
 *	immediate variable as its first token (same start position)
 *	Alternatively an immediate variable in parentheses is accepted.
 *
 *	Checked earlier that IMM_IDENTIFIER is ftype ARITH or GATE.
 *
 *	mType is derived from the ftype of the Symbol. It may only be
 *	ARITH === 1, GATE === 2, ARITH+2 and GATE+2. (UDFA === 0 is an error)
 *
 *******************************************************************/

static void
immAssignFound(unsigned int start, unsigned int operator, unsigned int end, Symbol* sp)
{
    LineEntry *	p;
    int		mType;
    int		typ;

    assert(sp);

    for (p = lep - 1; p >= lineEntryArray; p-- ) {
#if YYDEBUG
	if ((debug & 0402) == 0402) fprintf(outFP, "%u(%u %u)%u", p->pStart, p->vStart, p->vEnd, p->pEnd);
#endif
	if (p->pStart == start) {
	    p->equOp  = operator;		/* marks an assignment expression */
	    p->pEnd   = end;
	    if (p->sp == sp) {
		typ = sp->type & TM;
		mType = sp->ftype;
		if (typ == UDF) {
		    sp->type = (mType == ARITH) ? ARNC : LOGC;
		} else if ((typ != ARNC || mType != ARITH) &&
			   (typ != LOGC || mType != GATE) &&
			   (typ != ERR)) {	/* avoids multiple error messages */
		    if ((typ == ARN && mType == ARITH) ||
		        (typ >= AND && typ <= LATCH && mType == GATE)) {
			error("C-assignment to an imm variable already assigned in iC code:", sp->name);
		    } else {
			error("C-assignment to an incompatible imm type:", sp->name);
		    }
		    sp->type = ERR;	/* cannot execute properly */
		}
		return;
	    } else {
		break;			/* Error: Symbols don't match */
	    }
	} else if (p->pStart < start) {
	    break;			/* Error: will not find start */
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
 *******************************************************************/

void
copyAdjust(FILE* iFP, FILE* oFP)
{
    int			c;
    LineEntry*		p;
    unsigned int	start;
    unsigned int	vstart;
    unsigned int	vend;
    unsigned int	equop;
    unsigned int	end;
    unsigned int	bytePos;
    int			pFlag;
    Symbol *		sp;
    Symbol **		hsp;
    int			mType;
    char		buffer[BUFS];	/* buffer for modified names */
    char		iqt[2];		/* char buffers - space for 0 terminator */
    char		bwx[2];
    int			byte;
    int			bit;
    char		tail[8];	/* compiler generated suffix _123456 max */

    if (iFP == NULL) {
#ifdef LEAS
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if ((sp->type & TM) == UDF) {
//fprintf(stderr, "copyAdjust: %d: %2d %s\n", udfCount, sp->type, sp->name); fflush(stderr);
		    udfCount += LEAS;
		}
	    }
	}
#endif
	lineEntryArray = (LineEntry*)realloc(NULL, udfCount * sizeof(LineEntry));
	assert(lineEntryArray);		/* FIX with quit */
//fprintf(stderr, "copyAdjust: %d allocated for lineEntryArray      %p\n", udfCount, lineEntryArray); fflush(stderr);
	lep = lineEntryArray;
	lep++->pStart = LARGE;		/* guard value in case first immVarFound(0) */
	lep->pStart   = LARGE;		/* value overwritten by first immVarfound */
	return;				/* lineEntryArray initialized */
    }

    lep++->pStart = LARGE;		/* finalise lineEntryArray */
    lep->pStart   = LARGE;		/* require 2 guard entries at the end */

    p       = lineEntryArray + 1;	/* miss guard entry at start */
    start   = p->pStart;
    vstart  = LARGE;
    vend    = LARGE;
    equop   = LARGE;
    end     = LARGE;
    bytePos = 0;
    pFlag   = 1;
    sp      = NULL;

    while ((c = getc(iFP)) != EOF) {
	while (bytePos >= end) {
	    putc(')', oFP);		/* end found - output end */
	    end = popEndStack();
	}
	if (bytePos >= start) {
	    pushEndStack(end);		/* push previous end */
	    vstart = p->vStart;		/* start of actual variable */
	    vend   = p->vEnd;		/* end of actual variable */
	    equop  = p->equOp;		/* operator in this entry */
	    end    = p->pEnd;		/* end of this entry */
	    sp     = p->sp;		/* associated Symbol */
	    assert(sp);
	    mType  = (equop == LARGE) ? sp->ftype : sp->ftype + MACRO_OFFSET;
	    fprintf(oFP, macro[mType]);	/* entry found - output macro start */
	    p++;
	    assert(start <= vstart);
	    assert(vstart < vend);
	    assert(vend <= end);
	    assert(bytePos < p->pStart);
	    start = p->pStart;		/* start of next entry */
	}
	if (bytePos == vstart) {
	    assert(sp);
	    IEC1131(sp->name, buffer, BUFS, iqt, bwx, &byte, &bit, tail);
	    fprintf(oFP, "%s", buffer);	/* output real Symbol name not ALIAS */
	    pFlag = 0;
	}
	if (bytePos == vend) {
	    pFlag = 1;
	}
	if (bytePos == equop) {
	    assert(c == '=');
	    c = ',';			/* replace '=' by ',' */
	}
	if (pFlag) {
	    putc(c, oFP);		/* output all except variables */
	}
	bytePos++;
    }
//fprintf(stderr, "copyAdjust: %d used for lineEntryArray\n", lep - lineEntryArray); fflush(stderr);
} /* copyAdjust */
#endif

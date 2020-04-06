%{ static const char gram_y[] =
"@(#)$Id: gram.y 1.43 $";
/********************************************************************
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
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
 *	Modification John Wulff 2020.04.05 - add types _Floatn. The 2020
 *	version of "Using the GNU Compiler Collection (GCC)" #6.12 says:
 *	ISO/IEC TS 18661-3:2015 defines C support for additional floating
 *	types _Floatn and _Floatnx, and GCC supports these type names.
 *	It is expected in future versions of GCC that _Float128 and
 *	__float128 will be enabled automatically. (That is the case now).
 *
 *	The <math.h> failed in iC programs because type _Float128 was used
 *	in the declartion of extern functions, which would not compile.
 *
 *	gram.y
 *	C grammar for supporting the immcc compiler to extract iC elements
 *	from embedded C code in iC sources.
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<assert.h>
#include	<stdarg.h>
#include	<string.h>
#include	<ctype.h>
#ifndef LMAIN
#undef	yyerror
#define yyerror	iCerror				/* use full yyerror in comp.y */
#endif
#include	"comp.h"
#include	"gram.tab.h"

#define ENDSTACKSIZE	100
#define AUXBUFSIZE	256
// #define LEAS		2
#define LEAI		170

extern int		yylex(void);		/* produced by lexc.l */
extern char *		yytext;			/* defined in lexc.l */
static const char	idOp[] = "+-+-";	/* increment/decrement operator selection */

static Symbol		typedefSymbol = { "typedef", UDF, UDFA, };

#ifndef LMAIN
#if YYDEBUG
#define OUTBUFSIZE	128			/* for 0402 debug output only */
static char		inBuf[OUTBUFSIZE];
static char		outBuf[OUTBUFSIZE];
static char *		obp;
static char *		ibp;
#define OUTBUFEND	&outBuf[OUTBUFSIZE-1]
#define INBUFEND	&inBuf[OUTBUFSIZE-1]
static int		changeFlag;
#endif
typedef struct LineEntry {
    unsigned int	pStart;
    unsigned int	vStart;
    unsigned int	inds;
    unsigned int	inde;
    unsigned int	equOp;
    unsigned int	vEnd;
    unsigned int	pEnd;
    unsigned int	ppi;
    unsigned int	inv;
    Symbol *		sp;
} LineEntry;

static LineEntry *	lineEntryArray = NULL;
static LineEntry *	lep = NULL;		/* associated array allocated from heap */
#ifdef LEAS
static unsigned int	udfCount = LEAS + 2;	/* start with guard value space */
#else
static unsigned int	udfCount = LEAI;	/* 170 is approx 4 kB */
#endif
static unsigned int *	endStack = NULL;	/* these values will cause dynamic */
static unsigned int *	esp = NULL;		/* allocation of first stack block */
static int		endStackSize = 0;
static void		immVarFound(unsigned int start, unsigned int end, unsigned int inds, unsigned int inde, unsigned int invert, Symbol* sp);
static void		immVarRemove(unsigned int start, unsigned int end, Symbol* sp);
static void		immAssignFound(unsigned int start, unsigned int operator,
			    unsigned int end, Symbol* sp, unsigned int ppi);
static void		pushEndStack(unsigned int value);
static unsigned int	popEndStack(void);
static unsigned int	peekEndStack(void);
#else	/* LMAIN */
static void		yyerror(const char *s, ...);
#endif	/* LMAIN */
%}
%union {					/* stack type */
    Token		tok;
}

%token	<tok> IDENTIFIER IMM_IDENTIFIER IMM_ARRAY_IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token	<tok> PTR_OP INC_OP DEC_OP LEFT_SH RIGHT_SH LE_OP GE_OP EQ_OP NE_OP
%token	<tok> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	<tok> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	<tok> XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token	<tok> TYPEDEF TYPEOF EXTERN STATIC AUTO REGISTER
%token	<tok> CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE CONST VOLATILE VOID
%token	<tok> FLOAT16 FLOAT32 FLOAT64 FLOAT128
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
%type	<tok> imm_unary_expression imm_postfix_expression imm_lvalue imm_identifier
%type	<tok> imm_array_identifier argument_expr_list string_literal

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
#endif	/* LMAIN */
	}
	| declaration_specifiers declarator {
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as function name */
#endif	/* LMAIN */
	}
					    function_body {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    delete_sym(&$2);			/* not deleted if imm */
#ifndef LMAIN
	    clearParaList(0);			/* restore all overloaded parameters */
#endif	/* LMAIN */
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
#endif	/* LMAIN */
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
#endif	/* LMAIN */
	}
	| declaration_specifiers error ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as local var */
#endif	/* LMAIN */
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
#endif	/* LMAIN */
	}
	| storage_class_specifier specifier_qualifier_list {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = $1.symbol;		/* typedef information */
#ifndef LMAIN
	    if (lexflag & C_FUNCTION) {
		lexflag |= C_PARA;		/* function internal C var is coming */
	    }
#endif	/* LMAIN */
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
	| FLOAT16 {				/* _Float16 */
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| FLOAT32 {				/* _Float32 */
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| FLOAT64 {				/* _Float64 */
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.symbol = NULL;
	}
	| FLOAT128 {				/* _Float128 */
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
	    if (($1.symbol->type < MAX_LS && $1.symbol->u_blist == 0) || $1.symbol->type == NCONST) {
		immVarRemove($1.start, $1.end, $1.symbol);
	    }
#endif	/* LMAIN */
	}
	| pointer direct_declarator {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = $2.symbol;
#ifndef LMAIN
	    if (($2.symbol->type < MAX_LS && $2.symbol->u_blist == 0) || $2.symbol->type == NCONST) {
		immVarRemove($2.start, $2.end, $2.symbol);
	    }
#endif	/* LMAIN */
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
	    $$.inds = $1.inds;
	    $$.inde = $1.inde;
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
#endif	/* LMAIN */
	}
	| parameter_head error ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif	/* LMAIN */
	    yyclearin; yyerrok;
	}
	| parameter_head parameter_type_list ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif	/* LMAIN */
	}
	| parameter_head parameter_type_list error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif	/* LMAIN */
	    yyclearin; yyerrok;
	}
	| parameter_head parameter_identifier_list ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif	/* LMAIN */
	}
	| parameter_head parameter_identifier_list error ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif	/* LMAIN */
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
#endif	/* LMAIN */
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
	/* unary_expression can be lvalue via '*' cast_expression only */
	| unary_expression assignment_operator assignment_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	/* imm_unary_expression can never be lvalue - no * - use imm_lvalue */
	| imm_lvalue assignment_operator assignment_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.inds = $1.inds;
	    $$.inde = $1.inde;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "assignment_expression: imm_lvalue <%u> assignment_expression %u (%u) %u <%u> %s\n",
		$2.inds, $1.start, $2.start, $3.end, $2.inds, $1.symbol->name);
#endif
	    if ($1.symbol->u_blist == 0 || $1.symbol->type == NCONST) {
		immAssignFound($1.start, $2.start, $3.end, $1.symbol, $2.inds);	/* NOTE: assignment_operator.inds is ppi */
	    }
#endif	/* LMAIN */
	}
	;

assignment_operator				/* 49 */
	: '=' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = 0;		/* = */
	    $$.symbol = NULL;
	}
	| MUL_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = 2;		/* *= */
	    $$.symbol = NULL;
	}
	| DIV_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = 3;		/* /= */
	    $$.symbol = NULL;
	}
	| MOD_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = 4;		/* %= */
	    $$.symbol = NULL;
	}
	| ADD_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = 5;		/* += */
	    $$.symbol = NULL;
	}
	| SUB_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = 6;		/* -= */
	    $$.symbol = NULL;
	}
	| AND_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = 7;		/* &= */
	    $$.symbol = NULL;
	}
	| XOR_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = 8;		/* ^= */
	    $$.symbol = NULL;
	}
	| OR_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = 9;		/* |= */
	    $$.symbol = NULL;
	}
	| LEFT_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = 10;		/* <<= */
	    $$.symbol = NULL;
	}
	| RIGHT_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = 11;		/* >>= */
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
#endif	/* LMAIN */
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
	| shift_expression LEFT_SH additive_expression {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.symbol = NULL;
	}
	| shift_expression RIGHT_SH additive_expression {
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
	    $$.inds = $1.inds;
	    $$.inde = $1.inde;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_unary_expression: imm_postfix_expression %u %u %s\n",
		$$.start, $$.end, $$.symbol->name);
#endif
#endif	/* LMAIN */
	}
	| INC_OP imm_unary_expression {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.inds = $2.inds;
	    $$.inde = $2.inde;
	    $$.symbol = $2.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_unary_expression: ++ imm_unary_expression %u (%u) %u <12> %s\n",
		$2.start, $1.start, $2.end, $2.symbol->name);
#endif
	    if ($2.symbol->u_blist == 0 || $2.symbol->type == NCONST) {
		immAssignFound($2.start, $1.start, $2.end, $2.symbol, 12);	/* ++x */
	    }
#endif	/* LMAIN */
	}
	| DEC_OP imm_unary_expression {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.inds = $2.inds;
	    $$.inde = $2.inde;
	    $$.symbol = $2.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_unary_expression: -- imm_unary_expression %u (%u) %u <13> %s\n",
		$2.start, $1.start, $2.end, $2.symbol->name);
#endif
	    if ($2.symbol->u_blist == 0 || $2.symbol->type == NCONST) {
		immAssignFound($2.start, $1.start, $2.end, $2.symbol, 13);	/* --x */
	    }
#endif	/* LMAIN */
	}
	| SIZEOF imm_unary_expression {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.symbol = NULL;
	}
	| SIZEOF imm_array_identifier {
	    $$.start = $2.start;
	    $$.end = $2.end;
	    $$.symbol = $2.symbol;
#ifndef LMAIN
	    functionUse[0].c_cnt |= F_SIZE;	/* imm sizeof macro required */
	    $$.inds = 0;			/* mark 0 in imm_lvalue */
	    $$.inde = $1.start;			/* used to set earlyop to blank out sizeof */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_unary_expression: sizeof imm_unary_expression %u (%u) %u %s\n",
		$2.start, $1.start, $2.end, $2.symbol->name);
#endif
	    if ($$.symbol->u_blist == 0 || $$.symbol->type == NCONST) {
		immVarFound($$.start, $$.end, $$.inds, $$.inde, 0, NULL);	/* adjust pEnd, set inds 0 inde early */
	    }
#endif	/* LMAIN */
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
	: imm_lvalue {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = $1.inds;
	    $$.inde = $1.inde;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_postfix_expression: imm_lvalue %u %u %s\n",
		$$.start, $$.end, $$.symbol->name);
#endif
#endif	/* LMAIN */
	}
	| imm_postfix_expression INC_OP {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.inds = $1.inds;
	    $$.inde = $1.inde;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_postfix_expression: imm_postfix_expression ++ %u (%u) %u <14> %s\n",
		$1.start, $2.start, $2.end, $1.symbol->name);
#endif
	    if ($1.symbol->u_blist == 0 || $1.symbol->type == NCONST) {
		immAssignFound($1.start, $2.start, $2.end, $1.symbol, 14);	/* x++ */
	    }
#endif	/* LMAIN */
	}
	| imm_postfix_expression DEC_OP {
	    $$.start = $1.start;
	    $$.end = $2.end;
	    $$.inds = $1.inds;
	    $$.inde = $1.inde;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_postfix_expression: imm_postfix_expression -- %u (%u) %u <15> %s\n",
		$1.start, $2.start, $2.end, $1.symbol->name);
#endif
	    if ($1.symbol->u_blist == 0 || $1.symbol->type == NCONST) {
		immAssignFound($1.start, $2.start, $2.end, $1.symbol, 15);	/* x-- */
	    }
#endif	/* LMAIN */
	}
	;

imm_lvalue					/* 65b */
	: imm_identifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = $1.inds;
	    $$.inde = $1.inde;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_lvalue: imm_identifier %u %u %s\n",
		$$.start, $$.end, $$.symbol->name);
#endif
#endif	/* LMAIN */
	}
	/* cannot have multiple immC array references */
	| imm_array_identifier '[' expression ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    $$.inds = $2.start;			/* mark '[' in imm_lvalue */
	    $$.inde = $4.start;			/* mark ']' in imm_lvalue */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_lvalue: imm_array_identifier %u %u %s[%u %u]\n",
		$$.start, $$.end, $$.symbol->name, $$.inds, $$.inde);
#endif
	    if ($$.symbol->u_blist == 0 || $$.symbol->type == NCONST) {
		immVarFound($$.start, $$.end, $$.inds, $$.inde, 0, NULL);	/* adjust pEnd, set inds inde */
	    }
#endif	/* LMAIN */
	}
	| imm_array_identifier '[' expression error ']' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	    $$.symbol = NULL;
	    yyclearin; yyerrok;
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
#ifndef LMAIN
	    unsigned int invert;
	    invert = $1.start & INVERT;		/* inverted alias if set */
	    $$.start = $1.start & ~INVERT;	/* clear invert flag immediately */
#else	/* LMAIN */
	    $$.start = $1.start;
#endif	/* LMAIN */
	    $$.end = $1.end;
	    $$.inds = LARGE;
	    $$.inde = LARGE;
	    $$.symbol = $1.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_identifier: IMM_IDENTIFIER %u %u %c%s\n",
		$$.start, $$.end, invert ? '~' : ' ', $$.symbol->name);
#endif
	    if ($$.symbol->u_blist == 0 || $$.symbol->type == NCONST) {
		immVarFound($$.start, $$.end, $$.inds, $$.inde, invert, $$.symbol);
	    }
#endif	/* LMAIN */
	}
	| '(' imm_identifier ')' {
	    /* stops this being a primary_expression which would lead to C assignment */
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.inds = $2.inds;
	    $$.inde = $2.inde;
	    $$.symbol = $2.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_identifier: (imm_identifier) %u %u %s\n",
		$$.start, $$.end, $$.symbol->name);
#endif
	    if ($$.symbol->u_blist == 0 || $$.symbol->type == NCONST) {
		immVarFound($$.start, $$.end, $$.inds, $$.inde, 0, NULL);	/* moves pStart and pEnd without changing vStart vEnd inv */
	    }
#endif	/* LMAIN */
	}
	;

imm_array_identifier				/* 70 */
	: IMM_ARRAY_IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    $$.inds = LARGE;			/* will be set in imm_lvalue: */
	    $$.inde = LARGE;			/*   | imm_array_identifier [ expression ] */
	    $$.symbol = $1.symbol;
#ifndef LMAIN
	    functionUse[0].c_cnt |= F_ARRAY;	/* immC array macro required */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_array_identifier: IMM_ARRAY_IDENTIFIER %u %u %s\n",
		$$.start, $$.end, $$.symbol->name);
#endif
	    if ($$.symbol->u_blist == 0 || $$.symbol->type == NCONST) {
		immVarFound($$.start, $$.end, $$.inds, $$.inde, 0, $$.symbol);
	    }
#endif	/* LMAIN */
	}
	| '(' imm_array_identifier ')' {
	    /* stops this being a primary_expression which would lead to C assignment */
	    $$.start = $1.start;
	    $$.end = $3.end;
	    $$.inds = $2.inds;
	    $$.inde = $2.inde;
	    $$.symbol = $2.symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_array_identifier: (imm_array_identifier) %u %u %s\n",
		$$.start, $$.end, $$.symbol->name);
#endif
	    if ($$.symbol->u_blist == 0 || $$.symbol->type == NCONST) {
		immVarFound($$.start, $$.end, $$.inds, $$.inde, 0, NULL);	/* moves pStart and pEnd without changing vStart vEnd */
	    }
#endif	/* LMAIN */
	}
	;

/********************************************************************
 *
 *	end of C grammar
 *
 *******************************************************************/
%%

#ifndef	_WIN32
#include	<unistd.h>
#endif	/* _WIN32 */

#ifdef LMAIN
static void
yyerror(const char *s, ...)
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
#else	/* LMAIN */

static char*	cMacro[] = { CMACRO_NAMES };

/********************************************************************
 *
 *	immVarFound
 *
 *	The parser has found a bare immediate variable or an immediate
 *	variable in parentheses (zany but allowed as an lvalue in C)
 *	or an arraY_identifier, which require updating of inds and inde.
 *	To locate parentheses, only pStart and pEnd are adjusted.
 *	vStart and vEnd still mark the original variable.
 *
 *	sp == NULL is used to signal parenthesized immediate variable.
 *
 *******************************************************************/

static void
immVarFound(unsigned int start, unsigned int end, unsigned int inds, unsigned int inde, unsigned int invert, Symbol* sp)
{
    LineEntry *	p;
    LineEntry *	newArray;

    p = lep;
    if (sp) {
	p->pStart = p->vStart = start;		/* of an imm variable */
	p->equOp  = LARGE;			/* marks a value variable */
	p->inds   = inds;			/* array index (usually LARGE) */
	p->inde   = inde;			/* array index (usually LARGE) */
	p->vEnd   = p->pEnd   = end;		/* of an imm variable */
	p->ppi    = 0;
	p->inv    = invert;			/* inverted alias if set */
	p->sp     = sp;
	if (sp->ftype != ARITH && sp->ftype != GATE &&
	    (sp->ftype != UDFA || (sp->type != ARNC && sp->type != LOGC))
	    && sp->type != ERR) {
	    ierror("C-statement tries to access an imm type not bit or int:", sp->name);
	    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
	}
	if (lep >= &lineEntryArray[udfCount-2]) {	/* allow for 2 guard entries at end */
	    newArray = (LineEntry*)realloc(lineEntryArray,
		(udfCount + LEAI) * sizeof(LineEntry));
	    assert(newArray);
	    memset(&newArray[udfCount], '\0', LEAI * sizeof(LineEntry));
	    udfCount += LEAI;			/* increase the size of the array */
	    lep += newArray - lineEntryArray;	/* lep needs adjusting - cannot realloc lineEntryArray directly */
	    lineEntryArray = newArray;		/* Array has been successfully resized */
	}
	lep++;
    } else {
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "immVarFound(%u %u [%u %u] NULL)\n",
		start, end, inds, inde);
	}
#endif
	/********************************************************************
	 *	Backtrack to find parenthesised or array variable to modify
	 *******************************************************************/
	for (p = lep - 1;
	    inds < LARGE &&			/* exit on lep-1 if imm_identifier (LARGE) */
	    p->pStart > start &&		/* try further for imm_array_identifier (has inds) */
	    p >= lineEntryArray;		/* avoid underflow */
	    p--);
	if (p >= lineEntryArray) {
	    p->pStart = start;			/* of possible parentheses or index expression */
	    if ((p->inds = inds) == 0) {	/* adjust array index [ */
		p->equOp  = inde;		/* marks early sizeof */
		p->inde   = 0;
	    } else {
		p->inde   = inde;		/* adjust array index ] */
	    }
	    p->pEnd   = end;			/* of possible parentheses or index expression */
	} else {
	    execerror("immVarFound: Symbol not found ???", NULL, __FILE__, __LINE__);	/* underflow */
	}
    }
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "immVarFound: %u (%u %u) %u [%u %u] %c%s\n",
	    p->pStart, p->vStart, p->vEnd, p->pEnd, p->inds, p->inde, p->inv ? '~' : ' ', p->sp->name);
    }
#endif
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
	if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "stack position (remove): start %u (%u) end %u (%u) Symbol %s\n",
	    lep->pStart, start, lep->pEnd, end, lep->sp->name);
	}
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
 *	=                               which is a simple assignment or
 *	+= -= *= /= %= &= ^= |= >>= <<= which is an operator assignment or
 *	++x --x                         which is pre-increment/decrement or
 *	x++ x--                         which is post-increment/decrement.
 *
 *	Backtrack along the immediate variables found so far, to find
 *	the one being assigned to here. Its 'lep' entry is modified.
 *	equOp points to an actual assignment or locates inc/dec-operator.
 *	vStart and vEnd still mark the variable being assigned to.
 *	pEnd is adjusted to the very end of the expression being assigned,
 *	where the closing bracket for the assignment cMacro has to be placed.
 *
 *	If no suitable immediate variable is found it is a compiler
 *	error, because an immediate assignment should have a bare
 *	immediate variable as its first token (same start position)
 *	Alternatively an immediate variable in parentheses is accepted.
 *
 *	Checked earlier that IMM_IDENTIFIER is ftype ARITH or GATE.
 *
 *	ftyp is derived from the ftype of the Symbol.
 *	It may only be
 *	ARITH === 1, GATE === 2 or UDFA === 0 if type == ARNC or LOGC
 *
 *	This did not work. jw 2004.02.23 found with Electric Fence 2.2.1
 *	in arnc5.ic:
 *		imm clock clk; if (IX0.1) { clk = 1; } // tries to use
 *		cMacro[ftyp] with ftyp = CLCKL 18, type = ERR 19 // FIXED
 *
 *	Use of ppi and equOp operator:
 *	In immVarFound() p->ppi is initialized to 0 and p->equOp to LARGE
 *	which is then described as a used variable. If equOp is set to some
 *	operator position it becomes an assigned operator. All other imm
 *	variables in an assignment, which have ppi == 0 have the value
 *	set to 1. This marks the variable as used, even if it is an assignment
 *	variable. By this method multiple assignments are handled correctly.
 *	If ppi > 12 the current variable is involved in a pre/post-inc/dec
 *	operation and is marked as both assigned and used anyway.
 *
 *******************************************************************/

static void
immAssignFound(unsigned int start, unsigned int operator, unsigned int end, Symbol* sp, unsigned int ppi)
{
    LineEntry *	p;
    int		ftyp;
    int		typ;
    Symbol *	sp1;
    char	temp[TSIZE];

    assert(sp);
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "immAssignFound: %u (%u) %u <%u> %s\n",
	    start, operator, end, ppi, sp->name);
    }
#endif
    /********************************************************************
     *	Backtrack to find the variable being assigned to
     *******************************************************************/
    for (p = lep - 1; p >= lineEntryArray && p->pStart >= start; p--) {
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "--%u(%u %u %u)%u <%u>\n",
	    p->pStart, p->vStart, p->equOp==LARGE?0:p->equOp, p->vEnd, p->pEnd, p->ppi);
#endif
	if (p->pStart == start &&		/* start position of imm variable assigned to */
	    p->equOp == LARGE) {		/* not an lvalue if previously assigned */
	    p->equOp  = operator;		/* position of operator marks an assignment expression */
	    p->pEnd   = end;			/* end position of expression assigned from */
	    p->ppi    = ppi;			/* assignment operator or pre/post-inc/dec */
	    p->inv    = 0;			/* start return value of assignment with no inversion */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "= %u(%u %u %u)%u <%u>\n",
		p->pStart, p->vStart, p->equOp==LARGE?0:p->equOp, p->vEnd, p->pEnd, p->ppi);
#endif
	    if (p->sp == sp) {
		typ = sp->type;
		ftyp = sp->ftype;
		if (typ == UDF) {
		    int atn = 0;
		    if (iC_Sflag) {
			ierror("strict: C assignment to an imm variable (should be immC):", sp->name);
			if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
			else goto changeType;
		    } else {
			changeType:
			sp->type = iC_ctypes[ftyp];	/* must be ARNC or LOGC */
			if (iFunSymExt && strcmp(iFunBuffer, sp->name) == 0) {
			    /********************************************************************
			     * Found 'this' in a function definition - trying to pass it as an immC
			     * Instead change immC Symbol by writing a new name and making 'this' an ALIAS
			     * do not use a generated name used somewhere else already
			     *******************************************************************/
			    do {
				snprintf(temp, TSIZE, "%s_%d", sp->name, ++atn);
			    } while (lookup(temp) != 0);
			    sp1 = install(temp, sp->type, ftyp);	/* new real immC */
			    sp1->list = sp->list;	/* transfer expression */
			    listGenOut(sp1, 1, 0);	/* list immC node and generate possible output */
			    if (iFunSymExt) {
				collectStatement(sp1);	/* put immC node in stmtList */
			    }
			    sp->type = ALIAS;
			    sp->list = sy_push(sp1);	/* becomes an alias of immC (no negation) */
			    if (iC_debug & 04) {
				iFlag = 1;		/* may need correction by pplstfix */
				fprintf(iC_outFP, "\n\t%s\t%c ---%c\t%s", sp1->name, iC_fos[sp1->ftype], iC_os[sp->type], sp->name);
				if (sp->ftype != GATE) {	/* listing of the ALIAS */
				    fprintf(iC_outFP, "\t%c", iC_fos[sp->ftype]);
				}
				fprintf(iC_outFP, "\n\n");
			    }
			} else {
			    listGenOut(sp, 1, 0);	/* list immC node which is not 'this' */
			}
			if (iFunSymExt) {
			    collectStatement(sp);	/* put normal or alias node in stmtList */
			}
		    }
		} else if ((typ != ARNC || (ftyp != ARITH && ftyp != UDFA)) &&
			   (typ != LOGC || (ftyp != GATE  && ftyp != UDFA)) &&
			   (typ != ERR)) {	/* avoids multiple error messages */
		    /* either way this is a type error */
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
	} else if (p->ppi == 0) {		/* all "other" imm values in the assignment */
	    p->ppi = 1;				/* are used even if themselves assigned to */
	}
    }
    ierror("C-assignment to an imm expression, not an LVALUE:", yytext - end + start);
} /* immAssignFound */

/********************************************************************
 *
 *	pushEndStack - either end or inde equop ppi
 *	before being pushed
 *		value = end  << 1;	which leaves the value even.
 *		value = inde << 1 | 1;	which leaves the value odd.
 *
 *******************************************************************/

static void
pushEndStack(unsigned int value)
{
    if (esp >= endStack + endStackSize) {
	endStack = (unsigned int*)realloc(endStack,
	    (endStackSize + ENDSTACKSIZE) * sizeof(unsigned int));
	assert(endStack);
	esp = endStack + endStackSize;
	memset(esp, '\0', ENDSTACKSIZE * sizeof(unsigned int));
	endStackSize += ENDSTACKSIZE;
    }
    *esp++ = value;
} /* pushEndStack */

/********************************************************************
 *
 *	popEndStack
 *	if popped value is even it is an end which cause ')' output.
 *	if popped value is odd it is an inde etc used to insert
 *	',' for '=' etc	after inde of index i[x]
 *	  --------------------------------------^
 *
 *******************************************************************/

static unsigned int
popEndStack(void)
{
    assert(esp > endStack);
    return *--esp;
} /* popEndStack */

/********************************************************************
 *
 *	peekEndStack
 *	check previous value on stack to see if it is odd, which is inde
 *	without actually popping it.
 *
 *******************************************************************/

static unsigned int
peekEndStack(void)
{
    if (esp > endStack) {
	return *(esp - 1);	/* possible inde with end marker 1 */
    } else {
	return 0;		/* end marker 0 */
    }
} /* peekEndStack */

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
 *	cMacro calls for immediate variables and assignments
 *
 *	'lp' points to the ffexpr head when called as an auxiliary
 *	compile of one if - else or switch C fragment. Else NULL
 *
 *******************************************************************/

static void
copyAdjust(FILE* iFP, FILE* oFP, List_e* lp)
{
    int			c;
    LineEntry*		p;
    unsigned int	start;
    unsigned int	vstart;
    unsigned int	inds;
    unsigned int	inde;
    unsigned int	equop;
    unsigned int	ppi;
    unsigned int	earlyop;
    unsigned int	endop;
    unsigned int	parend;
    unsigned int	parnxt;
    unsigned int	parcnt;
    unsigned int	end;
    unsigned int	vend;
    unsigned int	bytePos;
    unsigned int	i;
    int			pFlag;
    int			ic;
    int			cc;
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
    int			ftypa;
    Symbol *		fsp = 0;
    static char *	f0_1 = "_f0_1";		/* name of literal function head */

    /********************************************************************
     *	Allocate initial LineEntryArray if iFP == NULL (called before c_compile)
     *******************************************************************/

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
	assert(lineEntryArray);
	memset(lineEntryArray, '\0', udfCount * sizeof(LineEntry));
	lep = lineEntryArray;
	lep->pStart = lep->equOp = LARGE;	/* guard value in case first immVarFound(0) */
	lep++;
	lep->pStart = lep->equOp = LARGE;	/* value overwritten by first immVarFound */
	return;					/* lineEntryArray initialized */
    }

    /********************************************************************
     *	Adjust offsets to 2 for ffexpr with clock or 3 for ffexpr with timer,delay
     *******************************************************************/
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "##### copyAdjust( %s )\n", lp ? "FFEXPR" : "LITERAL");
#endif

    if (lp) {					/* ffexpr head link */
	assert(lp->le_val >> FUN_OFFSET && lp->le_next == 0); /* must have function # and callled only once */
	tsp = lp->le_sym;			/* master - must be ftype F_SW, F_CF or F_CE */
	assert(tsp && (tsp->ftype == F_SW || tsp->ftype == F_CF || tsp->ftype == F_CE));
	lp1 = tsp->u_blist;
	assert(lp1);
	tsp = lp1->le_sym;			/* clock for ffexpr head */
	assert(tsp);
	if (tsp->ftype == CLCKL) {
	    lNr = 2;				/* start of value list for C fragment */
	} else if (tsp->ftype == TIMRL) {
	    lNr = 3;				/* extra space for timer delay */
	} else {
	    assert(0);				/* compile error - must be a clock or timer */
	}
    }

    /********************************************************************
     *	Initialise lineEntryArray
     *******************************************************************/

    lep->pStart   = lep->equOp = LARGE;		/* finalise lineEntryArray */
    lep++;
    lep->pStart   = lep->equOp = LARGE;		/* require 2 guard entries at the end */

    p       = lineEntryArray + 1;		/* miss guard entry at start */
    start   = p->pStart;
    if (p->equOp < start) {
	earlyop = p->equOp;			/* operator in pre-inc/dec entry handled early */
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "load bytePos = 0, earlyop = %u\n", earlyop);
#endif
    } else {
	earlyop = LARGE;
    }
    vstart  = LARGE;
    inds    = LARGE;
    inde    = LARGE;
    equop   = LARGE;
    endop   = LARGE;
    end     = LARGE;
    vend    = LARGE;
    parend  = LARGE;
    parnxt  = LARGE;
    parcnt  = 0;
    ppi     = 0;
    pFlag   = 1;				/* start by outputting C code till first variable or ++/-- */
    sp      = NULL;
#if YYDEBUG
    changeFlag = 0;
    ibp     = inBuf;
    obp     = outBuf;
#endif

    /********************************************************************
     *	Read input file
     *******************************************************************/

    bytePos = ic = cc = 0;
    while ((c = getc(iFP)) != EOF) {
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    if (ibp < INBUFEND) *ibp++ = c;	/* keep Input for detailed debugging */
	}
#endif
	if (bytePos == endop) {
	    endop = parnxt = LARGE;		/* do this first so same */
	    pFlag = 1;				/* bytePos can set pFlag again */
	}
	if (inde == LARGE && bytePos == vend && ppi >= 12) {
	    /* pre/post-increment/decrement */
	    /* ++x; --x; x++; x--; all now produce: iC_AA(2, 12, 0); iC_AA(2, 13, 0); etc jw120722 */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		obp += snprintf(obp, OUTBUFEND-obp, ", %u, 0", ppi);
		if (obp > OUTBUFEND) obp = OUTBUFEND;	/* catch buffer overflow */
		changeFlag++;
	    }
#endif
	    fprintf(oFP, ", %u, 0", ppi);	/* output inc/dec operator selection with 0 argument */
	}
	while (bytePos >= end) {
	    i = popEndStack();
	    assert((i & 1) == 0);
	    end = i >> 1;			/* 0 marker - new end for while */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		if (obp < OUTBUFEND) *obp++ = ')';
		changeFlag++;
	    }
#endif
	    putc(')', oFP);			/* end found - output end */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "popE bytePos = %u [%u %u] earlyop = %u, end = %u\n",
		bytePos, inds, inde, earlyop, end);
#endif
	    i = peekEndStack();
	    if ((i & 1) == 1) {			/* look for ] to restore inde */
		inde = i >> 1;			/* restore inde ] for this level */
		break;				/* will be popped at inde now restored */
	    }
	}
	/********************************************************************
	 *  Bitwise complement outside of the grammar.
	 *
	 *  Count occurences of '~' in ic and '(' following in cc. Do not
	 *  output yet. Output number of '~' counted and one '(' if counted
	 *  if any non-white space character is found, which is neither a '~'
	 *  or a first '(' or the first character of a logical imm variable,
	 *  which is either an imm or immC bit variable (ftype == GATE) or
	 *  an immC bit array variable (ftype == UDFA && type == LOGC).
	 *  Exclude operator 'sizeof' for an immC bit array variable, which
	 *  is characterised by p->inds == 0.
	 *
	 *  When the macro for one of these logical variables is output an
	 *  additional 2nd parameter c is output which is 0 for an even number
	 *  of '~' operators found preceding the variable (including none) or
	 *  1 for an odd number of '~' operators. The '~' and '(' characters
	 *  skipped in the input are not output. The correct complement
	 *  operation is executed in one of the logical macros or in the
	 *  iC_assignL(n, c, p, v) function called in the assign macros.
	 *******************************************************************/
	if (c == '~') {
	    ic++;				/* count occurence of '~' */
	    bytePos++;
	    continue;				/* don't output yet */
	}
	if (ic > 0) {
	    if (c == '(') {
		if (++cc > 1) {
		    goto outputComplement;
		}
		bytePos++;
		continue;			/* don't output yet */
	    }
	    if (! (isspace(c) ||				/* skip white space - output normally */
		(bytePos == p->vStart &&
		    (p->sp->ftype == GATE ||	/* or skip logic gate - '~' handled by macro */
			(p->sp->ftype == UDFA && p->sp->type == LOGC && p->inds != 0)	/* array but not sizeof */
		    )
		))) {
	      outputComplement:
		while (ic--) {
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) {
			if (obp < OUTBUFEND) *obp++ = '~';
			changeFlag++;
		    }
#endif
		    putc('~', oFP);			/* output 1 or more '~' now */
		}
		if (cc && start >= p->vStart) {		/* parenthesised assignment and not variable */
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) {
			if (obp < OUTBUFEND) *obp++ = '(';
			changeFlag++;
		    }
#endif
		    putc('(', oFP);			/* output 1 '(' now */
		}
		ic = cc = 0;
	    }
	}
	/********************************************************************
	 *  End of the bulk of the code for bitwise complement
	 *******************************************************************/
	if (bytePos >= start) {			/* ic and cc may still be set for GATE */
	    pushEndStack(end << 1);		/* push previous end (with 0 marker for end) */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "\n= %u(%u [%u %u] %u %u)%u <%u> %c%s\n",
		    p->pStart, p->vStart, p->inds, p->inde, p->equOp==LARGE?0:p->equOp, p->vEnd, p->pEnd, p->ppi, p->inv ? '~' : ' ', p->sp->name);
	    }
#endif
	    vstart = p->vStart;			/* start of actual variable */
	    vend   = p->vEnd;			/* end of actual variable */
	    inds   = p->inds;			/* array index start */
	    inde   = p->inde;			/* array index end */
	    equop  = inds ? p->equOp : LARGE;	/* operator in this entry unless sizeof */
	    end    = p->pEnd;			/* end of this entry */
	    ppi    = p->ppi;			/* pre/post-inc/dec character value */
	    if (p->inv) ic++;			/* another inversion for inverting alias */
	    sp     = p->sp;			/* associated Symbol */
	    assert(sp);
	    ml     = lp ? 0		 : CMACRO_LITERAL;	/*       0 or      9 */
	    ftypa  = (sp->type == ERR)	 ? UDFA
		   : (inds == 0)	 ? CMACRO_SIZE		/* sizeof macro */
		   : (sp->ftype != UDFA) ? ((equop == LARGE) ? sp->ftype	  : sp->ftype + CMACRO_ASSIGN)
		   : (sp->type == ARNC)	 ? ((equop == LARGE) ? ARITH+CMACRO_INDEX : ARITH+CMACRO_INDEX+CMACRO_ASSIGN)
		   : (sp->type == LOGC)	 ? ((equop == LARGE) ? GATE+CMACRO_INDEX  : GATE+CMACRO_INDEX+CMACRO_ASSIGN)
		   : UDFA;
	    /* assignment cMacro must be printed outside of enclosing parentheses */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "start bytePos = %u [%u %u] earlyop = %u, equop = %u, ppi = %u, sp =>%c%s\n",
		    bytePos, inds, inde, earlyop, equop, ppi, p->inv ? '~' : ' ', sp->name);
	    }
#endif
	    assert(cc == 0 || sp->ftype == GATE || (sp->ftype == UDFA && sp->type == LOGC && inds != 0));	/* cc == 0 unless GATE */
	    assert(start <= vstart);
	    assert(vstart < vend);
	    assert(vend <= end);
	    if (start < vstart) {
		endop = vstart;			/* suppress output of this paranthesis */
		pFlag = 0;
		parend = vend;			/* marks closing parenthesis which is also supressed */
		parcnt = 1;			/* could be more than 1 ( */
		parnxt = bytePos + 1;		/* allows counting of ( and spaces up to vstart */
	    } else if (cc) {
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    if (obp < OUTBUFEND) *obp++ = '(';
		    changeFlag++;
		}
#endif
		putc('(', oFP);			/* output 1 '(' now */
	    }
	    cc = 0;
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		obp += snprintf(obp, OUTBUFEND-obp, "%s", cMacro[ml+ftypa]);
		if (obp > OUTBUFEND) obp = OUTBUFEND;	/* catch buffer overflow */
		changeFlag++;
	    }
#endif
	    fprintf(oFP, "%s", cMacro[ml+ftypa]);	/* entry found - output cMacro start */
	    p++;				/* next entry to locate possible earlyop */
	    assert(bytePos < p->pStart);
	    start = p->pStart;			/* start of next entry */
	    if (p->equOp < start) {
		earlyop = p->equOp;		/* operator in pre-inc/dec or sizeof entry handled early */
	    } else {
		earlyop = LARGE;
	    }
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "load bytePos = %u, next start = %u, earlyop = %u\n",
		bytePos, start, earlyop);
#endif
	}
	if (bytePos == vstart) {
	    assert(sp);				/* start of this actual variable */
	    if (lp) {
		functionUse[0].c_cnt |= F_FFEXPR;	/* flag for copying ffexpr cMacro */
		lp1 = lp;
	    } else {
		functionUse[0].c_cnt |= F_LITERAL;	/* flag for copying literal cMacro */
		if (fsp) {
		    lp1 = fsp->list;		/* start with head of pointer list */
		} else
		if ((fsp = lookup(f0_1)) == 0) {
		    fsp = install(f0_1, OR, F_CF);	/* install new literal function head */
		    lp1 = sy_push(fsp);		/* head of literal pointer list */
		    fsp->list = lp1;		/* save in list for more pointers */
		} else {
		    if (fsp->type != CF && fsp->ftype != F_CF) {
			ierror("use of literal function head which was previously used for a different purpose:", f0_1);
		    }
		    lp1 = fsp->list;		/* start with head of pointer list */
		}
		lNr = 0;
	    }
	    sNr = lNr;				/* start at array pos after clock or timer,val */
	    while ((lp2 = lp1->le_next) != 0) {
		if (sp == lp2->le_sym) {
		    break;			/* variable occurred previously */
		}
		sNr++;
		lp1 = lp2;
	    }
	    if (lp2 == 0) {
		lp2 = sy_push(sp);		/* new variable in this C fragment */
		lp2->le_val |= sNr;		/* offset in list */
		lp1->le_next = lp2;		/* place at end of list */
	    }
	    if (equop == LARGE) {
		sp->em |= EU;			/* variable value is used - no assignment selection */
	    } else {
		sp->em |= EA;			/* variable is assigned to */
		if (ppi > 0) {
		    sp->em |= EU;		/* pre/post-inc/dec or marked as used */
		}
	    }
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "mark immC %s%s	em = 0x%02x\n",
		sp->name, inds < LARGE ? "[]	" : "", sp->em);
#endif
	    endop = (equop != LARGE && equop > vend)
		? equop : vend;			/* suppress variable name, which is replaced by cMacro */
	    pFlag = 0;
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		obp += snprintf(obp, OUTBUFEND-obp, "%d", sNr);
		if (sp->ftype == GATE || (sp->ftype == UDFA && sp->type == LOGC && inds != 0)) {
		    obp += snprintf(obp, OUTBUFEND-obp, ", %d", ic & 0x1);
		}
		if (obp > OUTBUFEND) obp = OUTBUFEND;	/* catch buffer overflow */
		changeFlag++;
	    }
#endif
	    fprintf(oFP, "%d", sNr);		/* output Symbol pointer offset instead of variable */
	    /********************************************************************
	     *  Output of the complement parameter for logic variables
	     *******************************************************************/
	    if (sp->ftype == GATE || (sp->ftype == UDFA && sp->type == LOGC && inds != 0)) {
		fprintf(oFP, ", %d", ic & 0x1);	/* count of '~' complements preceding logic variable */
	    }
	    ic = cc = 0;
	    /********************************************************************
	     *  End
	     *******************************************************************/
	}
	if (bytePos == earlyop) {
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "use  bytePos = %u [%u %u] earlyop = %u\n",
		bytePos, inds, inde, earlyop);
#endif
	    endop = bytePos + (p->inds ? 2 : 6);/* ++ and -- are 2 chars sizeof is 6 starting at earlyop */
	    pFlag = 0;				/* suppress output of ++ or -- from pre-inc/dec or sizeof */
	}
	if (bytePos == inds) {
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "use  bytePos = %u inds = %u\n",
		bytePos, inds);
#endif
	    endop = bytePos + 1;
	    pFlag = 0;				/* suppress output of [ */
	    pushEndStack(ppi   << 1 | 1);	/* push this ppi (in case embedded imm variable) */
	    pushEndStack(equop << 1 | 1);	/* push this equop (in case embedded imm variable) */
	    pushEndStack(inde  << 1 | 1);	/* push this index end (in case embedded imm variable) */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		obp += snprintf(obp, OUTBUFEND-obp, ", ");
		if (obp > OUTBUFEND) obp = OUTBUFEND;	/* catch buffer overflow */
		changeFlag++;
	    }
#endif
	    fprintf(oFP, ", ");			/* output ',' to start index */
	}
	if (bytePos == inde) {
	    int previnde = inde;
	    i = popEndStack();			/* pop this inde */
	    assert((i & 1) == 1);
	    inde = i >> 1;
	    i = popEndStack();			/* pop this equop */
	    assert((i & 1) == 1);
	    equop = i >> 1;
	    i = popEndStack();			/* pop this ppi */
	    assert((i & 1) == 1);
	    ppi = i >> 1;
	    endop = bytePos + 1;
	    pFlag = 0;				/* suppress output of ] */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "use  bytePos = %u inde = %u --inde = %u --equop = %u --ppi <%u> endop = %u\n",
		bytePos, previnde, inde, equop, ppi, endop);
#endif
	    if (ppi >= 12) {
		/* pre/post-increment/decrement */
		/* ++x; --x; x++; x--; all now produce: iC_AA(2, 12, 0); iC_AA(2, 13, 0); etc jw120722 */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    obp += snprintf(obp, OUTBUFEND-obp, ", %u, 0", ppi);
		    if (obp > OUTBUFEND) obp = OUTBUFEND;	/* catch buffer overflow */
		    changeFlag++;
		}
#endif
		fprintf(oFP, ", %u, 0", ppi);	/* output inc/dec operator selection with 0 argument */
	    }
	}
	if (bytePos == equop) {
	    assert(strchr("=+-*/%&^|><", c));
	    endop = bytePos + (
		ppi <  2 ? 1 :			/* = */
		ppi < 10 ? 2 :			/* *= /= %= += -= &= ^= |= */
		ppi < 12 ? 3 :			/* <<= >>= */
			   2);			/* ++ -- pre/post ++ -- */
	    pFlag = 0;				/* suppress output of assignment operator */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "use  bytePos = %u [%u %u] earlyop = %u, equop = %u, '%c', endop = %u <%u>\n",
		bytePos, inds, inde, earlyop, equop, c, endop, ppi);
#endif
	    if (ppi < 12) {
		/* simple or operator assignment */
		/* x = 15; produces: iC_AA(2, 1, 15); */
		/* x *= 15; produces: iC_AA(2, 3, 15); */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    obp += snprintf(obp, OUTBUFEND-obp, ", %u, ", ppi);
		    if (obp > OUTBUFEND) obp = OUTBUFEND;	/* catch buffer overflow */
		    changeFlag++;
		}
#endif
		fprintf(oFP, ", %u, ", ppi);	/* output assignment operator selection */
	    }
	}
	if (bytePos == parnxt) {
	    if (c == '(') {
		parcnt++;			/* count extra parentheses (might be spaces) */
	    }
	    parnxt = bytePos+1;			/* set to LARGE at endop - terminates counting */
	}
	if (bytePos == parend) {
	    parend = endop = bytePos+1;		/* suppress output of closing parantheses and spaces */
	    if (c == ')' && --parcnt <= 0) {	/* parenthesised imm_identifier or imm_array_identifier */
		    parend = LARGE;		/* count down ) until the required number */
	    }
	    pFlag = 0;
	}
	if (pFlag) {
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		if (obp < OUTBUFEND) *obp++ = c;
	    }
#endif
	    putc(c, oFP);			/* output all except variables and ++ or -- from inc/dec etc */
	}
	bytePos++;
#if YYDEBUG
	if (c == '\n' && (iC_debug & 0402) == 0402) {
	    if (changeFlag) {
		*ibp = *obp = '\0';
		fprintf(iC_outFP, "Input:  %s", inBuf);
		fprintf(iC_outFP, "Output: %s", outBuf);
		changeFlag = 0;
	    }
	    ibp = inBuf;
	    obp = outBuf;
	}
#endif
    }
    if ((iC_debug & 04) && (fsp = lookup(f0_1)) != 0) {
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
	    use = (gp->em & AU) >> AU_OFS;
	    assert(use < Sizeof(iC_useText));
	    fprintf(iC_outFP, "\t%s\t%c<---%c\t\t\t// %d %s", gp->name, iC_fos[gp->ftype],
		iC_os[CF], lp1->le_val & 0xff, iC_useText[use]);
	    if ((iC_debug & 0200) && ! iFunSymExt) {	/* could use liveDisp, if it were global */
		fprintf(iC_outFP, "\t%s\t=", gp->name);
	    }
	    fprintf(iC_outFP, "\n");
	}
    }
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "##### copyAdjust( END )\n");
#endif
} /* copyAdjust */

/********************************************************************
 *
 *      copy C intermediate file up to EOF to C output file.
 *      Translate any ALIAS references as part of C compile phase.
 *
 *      mode 01         Copy only literal blocks %{ ... %}
 *      mode 02 default Copy only C blocks, functions or cases
 *      mode 03         Copy literal blocks and C blocks
 *
 * For iC version 3, #include, #define, #ifdef etc are written without
 * preceding the # with %. For backward compatibility with iC version 2
 * the conversion in the next paragraph is still carried out here, but
 * for iC version 3 the % before # is optional.
 *
 * The characters %# occurring at the start of a line in a literal or C
 * block will be converted to a plain #. This allows the use of
 * C-preprocessor statements in literal or C blocks which will be
 * resolved after the iC compilation. They ''may'' be written as:
 *     iC vers 2                iC vers 3
 *     %#include                #include
 *     %#define                 #define
 *     %#ifdef etc              #ifdef etc
 *
 * The following still holds in iC version 3:
 * lines starting with %## will be replaced by #line nn "fn"
 * where nn is the following line number and fn the name of the iC file
 * NOTE: this line must be shorter than BUFS (currently 128).
 *       since %## is generated in the earlier code this should be OK
 * NOTE: lineBuf[] must be large enough to hold a complete
 *       pre-processor line for the following sscanf()s
 *
 * NOTE: the following is an important change in iC version 3:
 *       #include, #define, #ifdef etc lines outside of C literal blocks,
 *       which affect iC code and which was the rule for iC version 2
 *       files must now be written as %include, %define, %ifdef etc in
 *       iC version 3. This is not optional, but since pre-processor
 *       directives in iC code are fairly rare, whereas they are common
 *       in C code, most old style iC version 2 files will be backwards
 *       compatible and compile Ok. Any version 2 style # pre-processor
 *       directives in iC sections will lead to warnings or errors.
 *
 *******************************************************************/

static int	precompileFlag;

static int
copyBlocks(FILE * iFP, FILE * oFP, int mode)
{
    int		c;
    int		mask = 02;			/* default is functions or cases */
    int		lFlag = 0;			/* set by first non white space in a line */
    char *	cp;
    char	lstBuf[BUFS];			/* include file name */
    char	lineBuf[BUFS];			/* can be smaller than a line */

    /* rewind intermediate file */
    if (fseek(iFP, 0L, SEEK_SET) != 0) {
	return T1index;				/* error in temporary file */
    }

    /********************************************************
     *  read each line of the input file iFP
     ********************************************************/
    while (fgets(lineBuf, sizeof lineBuf, iFP)) {
	if (strcmp(lineBuf, "%{\n") == 0) {
	    mask = 01;				/* copy literal blocks */
	} else if (strcmp(lineBuf, "%}\n") == 0) {
	    mask = 02;				/* copy functions or cases */
	} else if (mode & mask) {		/* separates literal blocks and functions */
	    lFlag = 0;				/* start of a new line to copy */
	    /********************************************************
	     *  scan each character in a line
	     ********************************************************/
	    for (cp = lineBuf; (c = *cp) != 0; cp++) {
		if (lFlag == 0 && c == '%' && *(cp+1) == '#') {
		    c = *++cp;				/* remove optional % before first # */
		}
		putc(c, oFP);				/* copy character to output file oFP */
		if (lFlag == 0 && c == '#' &&
		    /********************************************************
		     *  cp now points to the first # in a line past optional
		     *  white space (and optional % which was not copied).
		     *  Handle pre-processor #include <stdio.h> or "icc.h"
		     ********************************************************/
		    sscanf(cp, " # include %[<\"/A-Za-z_.0-9>]", lstBuf) == 1) {
		    if(iFP == T4FP) {
			ierror("copyBlocks: if else or switch has:", lstBuf);
			continue;
		    }
		    if (precompileFlag == 0) {
			if ((c = iC_openT4T5(0)) != 0) return c;	/* re-open if necessary */
		    }
		    if (iC_debug & 02) fprintf(iC_outFP, "####### yyparse #include %s\n", lstBuf);
		    fprintf(T4FP, "#include %s\n", lstBuf);	/* a little C file !!! */
		    precompileFlag = lFlag = 1;			/* 1 marks #include */
		} else if (c != ' ' && c != '\t') {
		    lFlag = 1;				/* not white space */
		}
	    }
	}
    }
    return 0;
} /* copyBlocks */

/********************************************************************
 *
 *	To handle immediate variables in C code, the output of the first
 *	two passes of copyBlocks() is separated by a line containing
 *	## in C comment delimiters and output to T2FP.
 *
 *	copyBlocks() also isolates #include lines in C literal blocks and
 *	copies them into a small C file T4FN and then sets precompileFlag.
 *	if (precompileFlag != 0) {
 *	    T4FN will be pre-compiled by gcc -E T4FN -o T5FN
 *	    T5FN // is the expansion of the #include files in T4FN
 *	    which is then parsed by own C compiler yyparse()
 *	    to extract possible CTYPE definitions required in
 *	    C code embedded in the iC file for the main yyparse()
 *	}
 *	this is followed by the main yyparse() to convert all
 *	immediate variable names and assignments in C code to macros.
 *
 *	For auxiliary yyparse() of iC if - else and switch blocks,
 *	which have no need for C - pre-processor includes, iFP is T4FP
 *
 *******************************************************************/

extern FILE* yyin;
extern FILE* yyout;

int
iC_c_compile(FILE * iFP, FILE * oFP, int flag, List_e * lp)
{
    int		r;				/* copy literal blocks */
    char	execBuf[BUFS];			/* can be smaller than a line */
    char	lineBuf[BUFS];

    lexflag = flag;				/* output partial source listing */
    precompileFlag = 0;

    if (ftell(T2FP)) {
	fclose (T2FP);				/* overwrite intermediate file */
	if ((T2FP = fopen(T2FN, "w+")) == NULL) {
	    return T2index;
	}
    }
    if (copyBlocks(iFP, T2FP, 01)) {
	return T1index;				/* error in temporary file */
    }
    if (outFlag == 0) {				/* -c option to produce cexe.c */
#if INT_MAX == 32767 && defined (LONG16)
	fprintf(T2FP, "/*##*/long iC_exec(int iC_indx) { switch (iC_indx) {\n");
#else
	fprintf(T2FP, "/*##*/int iC_exec(int iC_indx) { switch (iC_indx) {\n");
#endif
    } else {
	fprintf(T2FP, "/*##*/\n");		/* -o option - separate blocks */
    }
    if (copyBlocks(iFP, T2FP, 02)) {
	return T1index;				/* error in temporary file */
    }
    if (outFlag == 0) {
	fprintf(T2FP, "/*##*/}}\n");
    }
    if (precompileFlag != 0) {
	/********************************************************
	 *  pre-process and C-parse #include <stdio.h> and "icc.h" etc
	 *  add CTYPE's as symbols to iC symbol table for main C-parse
	 ********************************************************/
	fflush(T4FP);
	/* Cygnus does not understand cc - use macro CC=gcc */
	snprintf(execBuf, BUFS, SS(CC) " -E -I/usr/local/include -x c %s -o %s 2> %s",
	    T4FN, T5FN, T6FN);
	r = system(execBuf);			/* Pre-compile C header file(s) */
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### pre-compile: %s; $? = %d\n", execBuf, r>>8);
#endif
	if (r != 0 || (T5FP = fopen(T5FN, "r")) == NULL) {
	    if ((T6FP = fopen(T6FN, "r")) == NULL) {
		return T6index;			/* error opening CC error file */
	    }
	    while (fgets(lineBuf, sizeof lineBuf, T6FP)) {
		ierror(SS(CC) ":", lineBuf);	/* CC error message */
	    }
	    fclose(T6FP);
	    if (!(iC_debug & 04000)) {
		unlink(T6FN);
	    }
	    ierror("c_compile: cannot open:", T5FN);
	    perror("open");
	    return T5index;
	}
	if (!(iC_debug & 04000)) {
	    unlink(T6FN);
	}
	if (iC_debug & 02) fprintf(iC_outFP, "####### compile include files via %s %s\n", T4FN, T5FN);
	yyin = T5FP;				/* lexc reads from include now */
	yyout = iC_outFP;			/* list file */
	lexflag |= C_PARSE|C_NO_COUNT|C_FIRST;	/* do not count characters for include files */
	gramOffset = lineno = 0;
	if (yyparse() != 0) {			/* C parse headers to obtain CTYPE's in ST */
	    ierror("c_compile: Parse error in includes\n", T5FN);
	}
	lexflag &= ~C_NO_COUNT;			/* count characters again */
    }
    if (fseek(T2FP, 0L, SEEK_SET) != 0) {	/* rewind intermediate file */
	return T2index;
    }
    copyAdjust(NULL, NULL, 0);			/* initialize lineEntryArray */
    gramOffset = lineno = 0;
    yyin = T2FP;				/* C input to C parser */
    yyout = iC_outFP;				/* list file */
    if (yyparse() != 0) {			/* actual C parser call */
	ierror("c_compile: Parse error\n", NULL);
    }
    if (fseek(T2FP, 0L, SEEK_SET) != 0) {	/* rewind intermediate file */
	return T2index;
    }
    copyAdjust(T2FP, oFP, lp);			/* output adjusted C-code */
    return 0;
} /* iC_c_compile */

/********************************************************************
 *
 *	Then this code is parsed by the C compiler and afterwards split on
 *	## in comments and output to oFP
 *
 *******************************************************************/

int
iC_copyXlate(FILE * iFP, FILE * oFP, char * outfile, unsigned * lcp, int mode)
{
    int		mask = 01;			/* copy literal blocks */
    int		flag = 1;
    int		cFn;				/* C function number */
    char	lineBuf[BUFS];			/* can be smaller than a line */

    if (fseek(iFP, 0L, SEEK_SET) != 0) {	/* rewind intermediate file */
	return T3index;
    }

    while (fgets(lineBuf, sizeof lineBuf, iFP)) {
	if (strncmp(lineBuf, "/*##*/", 6) == 0) {
	    mask = 02;				/* copy functions or cases */
	} else if (mode & mask) {
	    if (mask == 02 &&			/* do not look in literals - mode 01 */
		sscanf(lineBuf, cexeString[outFlag], &cFn) == 1) {
		assert(cFn < functionUseSize);
		flag = functionUse[cFn].c_cnt;	/* has this function or case been called ? */
	    }
	    if (flag || outfile == 0) {		/* skip functions or cases not called */
		if (lcp) (*lcp)++;		/* count lines actually output */
		if (outfile && strcmp(lineBuf, "##\n") == 0) {
		    fprintf(oFP, "#line %d \"%s\"\n", *lcp, outfile);
		} else {
		    fputs(lineBuf, oFP);
		}
	    }
	}
    }
    return 0;
} /* iC_copyXlate */
#endif	/* LMAIN */

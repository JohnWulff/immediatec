%{ static const char gram_y[] =
"@(#)$Id: gram.y,v 1.3 2002/07/10 21:15:29 jw Exp $";
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

#include	"gram.h"

typedef struct LineEntry {
    int		start;
    int		op;
    int		end;
    int		type;
} LineEntry;

#define ENDSTACKSIZE	100
static LineEntry	lineEntryArray[500];
static LineEntry*	lep = lineEntryArray;
static int		endStack[ENDSTACKSIZE];
static int*		esp = endStack;

static void	immVarFound(int start, int end, int type);
static void	immAssignFound(int start, int operator, int end, int type);
static int	pushEndStack(int value);
static int	popEndStack(void);

%}
%union {		/* stack type */
    Token	tok;
}

%token	<tok> IDENTIFIER CONSTANT STRING_LITERAL SIZEOF
%token	<tok> PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP LE_OP GE_OP EQ_OP NE_OP
%token	<tok> AND_OP OR_OP MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN
%token	<tok> SUB_ASSIGN LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token	<tok> XOR_ASSIGN OR_ASSIGN TYPE_NAME

%token	<tok> TYPEDEF EXTERN STATIC AUTO REGISTER
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

%type	<tok>	'{' '[' '(' ')' ']' '}'
%right	<tok>	'=' ':' ';'
%right	<tok>	'&' '*' '+' '-' '~' '!'

%start file
%%

primary_expr
	: identifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| CONSTANT {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| STRING_LITERAL {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| '(' expr ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

postfix_expr
	: primary_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| postfix_expr '[' expr ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	| postfix_expr '(' ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| postfix_expr '(' argument_expr_list ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	| postfix_expr '.' identifier {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| postfix_expr PTR_OP identifier {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| postfix_expr INC_OP {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| postfix_expr DEC_OP {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

argument_expr_list
	: assignment_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| argument_expr_list ',' assignment_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

unary_expr
	: postfix_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| INC_OP unary_expr {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| DEC_OP unary_expr {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| unary_operator cast_expr {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| SIZEOF unary_expr {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| SIZEOF '(' type_name ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	;

unary_operator
	: '&' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| '*' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| '+' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| '-' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| '~' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| '!' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	;

cast_expr
	: unary_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| '(' type_name ')' cast_expr {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	;

multiplicative_expr
	: cast_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| multiplicative_expr '*' cast_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| multiplicative_expr '/' cast_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| multiplicative_expr '%' cast_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

additive_expr
	: multiplicative_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| additive_expr '+' multiplicative_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| additive_expr '-' multiplicative_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

shift_expr
	: additive_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| shift_expr LEFT_OP additive_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| shift_expr RIGHT_OP additive_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

relational_expr
	: shift_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| relational_expr '<' shift_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| relational_expr '>' shift_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| relational_expr LE_OP shift_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| relational_expr GE_OP shift_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

equality_expr
	: relational_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| equality_expr EQ_OP relational_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| equality_expr NE_OP relational_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

and_expr
	: equality_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| and_expr '&' equality_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

exclusive_or_expr
	: and_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| exclusive_or_expr '^' and_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

inclusive_or_expr
	: exclusive_or_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| inclusive_or_expr '|' exclusive_or_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

logical_and_expr
	: inclusive_or_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| logical_and_expr AND_OP inclusive_or_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

logical_or_expr
	: logical_and_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| logical_or_expr OR_OP logical_and_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

conditional_expr
	: logical_or_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| logical_or_expr '?' logical_or_expr ':' conditional_expr {
	    $$.start = $1.start;
	    $$.end = $5.end;
	}
	;

assignment_expr
	: conditional_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| identifier '=' assignment_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    printf("<%d %d>", $$.start, $$.end);
	    immAssignFound($1.start, $2.start, $3.end, 1);
	}
	| unary_expr assignment_operator assignment_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	    printf("@");
	}
	;

assignment_operator
	: '=' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| MUL_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| DIV_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| MOD_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| ADD_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| SUB_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| LEFT_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| RIGHT_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| AND_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| XOR_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| OR_ASSIGN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	;

expr
	: assignment_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| expr ',' assignment_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

constant_expr
	: conditional_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	;

declaration
	: declaration_specifiers ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| declaration_specifiers init_declarator_list ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

declaration_specifiers
	: storage_class_specifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| storage_class_specifier declaration_specifiers {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| type_specifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| type_specifier declaration_specifiers {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

init_declarator_list
	: init_declarator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| init_declarator_list ',' init_declarator {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

init_declarator
	: declarator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| declarator '=' initializer {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

storage_class_specifier
	: TYPEDEF {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| EXTERN {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| STATIC {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| AUTO {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| REGISTER {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	;

type_specifier
	: CHAR {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| SHORT {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| INT {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| LONG {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| SIGNED {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| UNSIGNED {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| FLOAT {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| DOUBLE {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| CONST {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| VOLATILE {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| VOID {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| struct_or_union_specifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| enum_specifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| TYPE_NAME {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	;

struct_or_union_specifier
	: struct_or_union identifier '{' struct_declaration_list '}' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	}
	| struct_or_union '{' struct_declaration_list '}' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	| struct_or_union identifier {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

struct_or_union
	: STRUCT {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| UNION {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	;

struct_declaration_list
	: struct_declaration {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| struct_declaration_list struct_declaration {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

struct_declaration
	: type_specifier_list struct_declarator_list ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

struct_declarator_list
	: struct_declarator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| struct_declarator_list ',' struct_declarator {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

struct_declarator
	: declarator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| ':' constant_expr {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| declarator ':' constant_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

enum_specifier
	: ENUM '{' enumerator_list '}' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	| ENUM identifier '{' enumerator_list '}' {
	    $$.start = $1.start;
	    $$.end = $5.end;
	}
	| ENUM identifier {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

enumerator_list
	: enumerator {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| enumerator_list ',' enumerator {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

enumerator
	: identifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| identifier '=' constant_expr {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

declarator
	: declarator2 {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| pointer declarator2 {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

declarator2
	: identifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| '(' declarator ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| declarator2 '[' ']' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| declarator2 '[' constant_expr ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	| declarator2 '(' ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| declarator2 '(' parameter_type_list ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	| declarator2 '(' parameter_identifier_list ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	;

pointer
	: '*' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| '*' type_specifier_list {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| '*' pointer {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| '*' type_specifier_list pointer {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

type_specifier_list
	: type_specifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| type_specifier_list type_specifier {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

parameter_identifier_list
	: identifier_list {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| identifier_list ',' ELIPSIS {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

identifier_list
	: identifier {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| identifier_list ',' identifier {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

parameter_type_list
	: parameter_list {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| parameter_list ',' ELIPSIS {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

parameter_list
	: parameter_declaration {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| parameter_list ',' parameter_declaration {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

parameter_declaration
	: type_specifier_list declarator {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| type_name {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	;

type_name
	: type_specifier_list {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| type_specifier_list abstract_declarator {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

abstract_declarator
	: pointer {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| abstract_declarator2 {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| pointer abstract_declarator2 {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

abstract_declarator2
	: '(' abstract_declarator ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| '[' ']' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| '[' constant_expr ']' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| abstract_declarator2 '[' ']' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| abstract_declarator2 '[' constant_expr ']' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	| '(' ')' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| '(' parameter_type_list ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| abstract_declarator2 '(' ')' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| abstract_declarator2 '(' parameter_type_list ')' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	;

initializer
	: assignment_expr {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| '{' initializer_list '}' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| '{' initializer_list ',' '}' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	;

initializer_list
	: initializer {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| initializer_list ',' initializer {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

statement
	: labeled_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| compound_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| expression_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| selection_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| iteration_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| jump_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	;

labeled_statement
	: identifier ':' statement {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| CASE constant_expr ':' statement {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	| DEFAULT ':' statement {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

compound_statement
	: '{' '}' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| '{' statement_list '}' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| '{' declaration_list '}' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| '{' declaration_list statement_list '}' {
	    $$.start = $1.start;
	    $$.end = $4.end;
	}
	;

declaration_list
	: declaration {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| declaration_list declaration {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

statement_list
	: statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| statement_list statement {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

expression_statement
	: ';' {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| expr ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

selection_statement
	: IF '(' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $5.end;
	}
	| IF '(' expr ')' statement ELSE statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	}
	| SWITCH '(' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $5.end;
	}
	;

iteration_statement
	: WHILE '(' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $5.end;
	}
	| DO statement WHILE '(' expr ')' ';' {
	    $$.start = $1.start;
	    $$.end = $7.end;
	}
	| FOR '(' ';' ';' ')' statement {
	    $$.start = $1.start;
	    $$.end = $6.end;
	}
	| FOR '(' ';' ';' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	}
	| FOR '(' ';' expr ';' ')' statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	}
	| FOR '(' ';' expr ';' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $8.end;
	}
	| FOR '(' expr ';' ';' ')' statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	}
	| FOR '(' expr ';' ';' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $7.end;
	}
	| FOR '(' expr ';' expr ';' ')' statement {
	    $$.start = $1.start;
	    $$.end = $8.end;
	}
	| FOR '(' expr ';' expr ';' expr ')' statement {
	    $$.start = $1.start;
	    $$.end = $9.end;
	}
	;

jump_statement
	: GOTO identifier ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	| CONTINUE ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| BREAK ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| RETURN ';' {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| RETURN expr ';' {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

file
	: external_definition {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| file external_definition {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

external_definition
	: function_definition {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| declaration {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	;

function_definition
	: declarator function_body {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	| declaration_specifiers declarator function_body {
	    $$.start = $1.start;
	    $$.end = $3.end;
	}
	;

function_body
	: compound_statement {
	    $$.start = $1.start;
	    $$.end = $1.end;
	}
	| declaration_list compound_statement {
	    $$.start = $1.start;
	    $$.end = $2.end;
	}
	;

identifier
	: IDENTIFIER {
	    $$.start = $1.start;
	    $$.end = $1.end;
	    printf("{%d %d}", $$.start, $$.end);
	    immVarFound($$.start, $$.end, 1);
	}
	;
%%

#include <stdio.h>
#include <assert.h>

extern char yytext[];
extern int column;

void
yyerror(char *s, ...)
{
	fflush(stdout);
	printf("\n%*s\n%*s\n", column, "^", column, s);
}

static void
immVarFound(int start, int end, int type)
{
    lep->start = start;
    lep->op    = 32767;
    lep->end   = end;
    lep->type  = type;
    lep++;
} /* immVarFound */

static void
immAssignFound(int start, int operator, int end, int type)
{
    LineEntry*	p;

    for (p = lep - 1; p >= lineEntryArray; p-- ) {
	printf("\n ***** <%d, %d, %d>", p->start, p->end, p->type);
	if (p->start == start) {
	    p->op    = operator;
	    p->end   = end;
	    if (p->type == type) {
		p->type  = type + 2;	/* replace variable entry with assignment entry */
		return;
	    } else {
		break;			/* Error: types don't match */
	    }
	} else if (p->start < start) {
	    break;			/* Error: will not find start */
	}
    }
    printf(" ERROR <%d, %d, %d>", start, end, type);
} /* immAssignFound */

static int
pushEndStack(int value)
{
    assert(esp < &endStack[ENDSTACKSIZE]);
    return *esp++ = value;
} /* pushEndStack */

static int
popEndStack(void)
{
    assert(esp >= endStack);
    return *--esp;
} /* popEndStack */

static char*	text[] = { "LV(", "AV(", "LA(", "AA(", };

void
copyAdjust(FILE* fp)
{
    int		c;
    LineEntry*	p     = lineEntryArray;
    int		start = p->start;
    int		op    = 32767;		/* ZZZ FIX */
    int		end   = 32767;		/* ZZZ FIX */
    int		cnt   = 0;

    lep->start = 32767;
    lep++;

    while ((c = getc(fp)) != EOF) {
	while (cnt >= end) {
	    putchar(')');		/* end found - output end */
	    end = popEndStack();
	}
	if (cnt >= start) {
	    pushEndStack(end);		/* push previous end */
	    op    = p->op;		/* operator in this entry */
	    end   = p->end;		/* end of this entry */
	    printf(text[p->type]);	/* entry found - output start */
	    p++;
	    assert(start < p->start);
	    start = p->start;		/* start of next entry */
	}
	if (cnt == op) {
	    assert(c == '=');
	    c = ',';			/* replace '=' by ',' */
	}
	putchar(c);
	cnt++;
    }
} /* immVarFound */

/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison implementation for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2012 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.7"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 1 "gram.y"
 static const char gram_y[] =
"@(#)$Id: gram.tab.c 1.36 $";
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
 *	gram.y
 *	C grammar for immcc compiler
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<assert.h>
#include	<stdarg.h>
#include	<string.h>
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
#define LARGE		(~0U>>2)

extern int		yylex(void);		/* produced by lexc.l */
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
static void		immVarFound(unsigned int start, unsigned int end, unsigned int inds, unsigned int inde, Symbol* sp);
static void		immVarRemove(unsigned int start, unsigned int end, Symbol* sp);
static void		immAssignFound(unsigned int start, unsigned int operator,
			    unsigned int end, Symbol* sp, unsigned int ppi);
static void		pushEndStack(unsigned int value);
static unsigned int	popEndStack(void);
static unsigned int	peekEndStack(void);
#else	/* LMAIN */
static void		yyerror(const char *s, ...);
#endif	/* LMAIN */

/* Line 371 of yacc.c  */
#line 164 "gram.tab.c"

# ifndef YY_NULL
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULL nullptr
#  else
#   define YY_NULL 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "gram.tab.h".  */
#ifndef YY_YY_GRAM_TAB_H_INCLUDED
# define YY_YY_GRAM_TAB_H_INCLUDED
/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     IMM_IDENTIFIER = 259,
     IMM_ARRAY_IDENTIFIER = 260,
     CONSTANT = 261,
     STRING_LITERAL = 262,
     SIZEOF = 263,
     PTR_OP = 264,
     INC_OP = 265,
     DEC_OP = 266,
     LEFT_SH = 267,
     RIGHT_SH = 268,
     LE_OP = 269,
     GE_OP = 270,
     EQ_OP = 271,
     NE_OP = 272,
     AND_OP = 273,
     OR_OP = 274,
     MUL_ASSIGN = 275,
     DIV_ASSIGN = 276,
     MOD_ASSIGN = 277,
     ADD_ASSIGN = 278,
     SUB_ASSIGN = 279,
     LEFT_ASSIGN = 280,
     RIGHT_ASSIGN = 281,
     AND_ASSIGN = 282,
     XOR_ASSIGN = 283,
     OR_ASSIGN = 284,
     TYPE_NAME = 285,
     TYPEDEF = 286,
     TYPEOF = 287,
     EXTERN = 288,
     STATIC = 289,
     AUTO = 290,
     REGISTER = 291,
     CHAR = 292,
     SHORT = 293,
     INT = 294,
     LONG = 295,
     SIGNED = 296,
     UNSIGNED = 297,
     FLOAT = 298,
     DOUBLE = 299,
     CONST = 300,
     VOLATILE = 301,
     VOID = 302,
     STRUCT = 303,
     UNION = 304,
     ENUM = 305,
     ELIPSIS = 306,
     CASE = 307,
     DEFAULT = 308,
     IF = 309,
     ELSE = 310,
     SWITCH = 311,
     WHILE = 312,
     DO = 313,
     FOR = 314,
     GOTO = 315,
     CONTINUE = 316,
     BREAK = 317,
     RETURN = 318
   };
#endif


#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{
/* Line 387 of yacc.c  */
#line 96 "gram.y"
					/* stack type */
    Token		tok;


/* Line 387 of yacc.c  */
#line 275 "gram.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_YY_GRAM_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

/* Line 390 of yacc.c  */
#line 303 "gram.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(N) (N)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (YYID (0))
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  2
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1782

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  88
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  76
/* YYNRULES -- Number of rules.  */
#define YYNRULES  284
/* YYNRULES -- Number of states.  */
#define YYNSTATES  493

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   318

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    79,     2,     2,     2,    86,    74,     2,
      66,    67,    75,    76,    73,    77,    87,    85,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    71,    72,
      83,    70,    84,    80,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    65,     2,    68,    82,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    64,    81,    69,    78,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,     9,    11,    14,    15,    20,
      22,    25,    28,    32,    36,    38,    41,    43,    46,    48,
      50,    52,    54,    56,    58,    60,    62,    64,    66,    68,
      70,    72,    74,    76,    78,    80,    85,    91,    96,   102,
     104,   106,   108,   113,   119,   121,   124,   127,   130,   132,
     134,   135,   138,   140,   144,   146,   150,   153,   157,   161,
     163,   166,   168,   171,   173,   177,   179,   182,   186,   191,
     197,   203,   210,   213,   215,   218,   220,   224,   226,   230,
     232,   235,   237,   239,   243,   248,   252,   257,   262,   268,
     271,   275,   279,   284,   288,   293,   296,   298,   301,   304,
     308,   310,   313,   315,   319,   321,   325,   327,   331,   334,
     336,   338,   342,   344,   348,   353,   358,   364,   366,   370,
     372,   375,   377,   379,   382,   386,   391,   394,   398,   402,
     407,   411,   416,   421,   427,   430,   434,   438,   443,   447,
     452,   457,   463,   465,   467,   469,   471,   473,   475,   479,
     484,   488,   490,   493,   497,   500,   504,   508,   513,   517,
     522,   527,   533,   535,   538,   544,   552,   558,   564,   572,
     579,   587,   595,   604,   612,   621,   630,   640,   644,   647,
     650,   653,   657,   662,   664,   668,   670,   674,   678,   680,
     682,   684,   686,   688,   690,   692,   694,   696,   698,   700,
     702,   708,   713,   715,   717,   721,   723,   727,   729,   733,
     735,   739,   741,   745,   747,   751,   755,   757,   761,   765,
     769,   773,   775,   779,   783,   785,   789,   793,   795,   799,
     803,   807,   809,   811,   816,   822,   824,   827,   830,   833,
     836,   841,   847,   849,   852,   855,   858,   861,   863,   865,
     867,   869,   871,   873,   875,   880,   886,   890,   895,   900,
     906,   910,   914,   917,   920,   922,   925,   928,   930,   935,
     941,   943,   945,   947,   951,   956,   960,   965,   967,   971,
     973,   976,   978,   982,   984
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      89,     0,    -1,    -1,    89,    90,    -1,    91,    -1,    94,
      -1,   114,    93,    -1,    -1,    96,   114,    92,    93,    -1,
     132,    -1,    95,   132,    -1,    96,    72,    -1,    96,   104,
      72,    -1,    96,     1,    72,    -1,    94,    -1,    95,    94,
      -1,   107,    -1,    97,   107,    -1,    35,    -1,    34,    -1,
      33,    -1,    31,    -1,    47,    -1,    36,    -1,    37,    -1,
      38,    -1,    39,    -1,    40,    -1,    43,    -1,    44,    -1,
      41,    -1,    42,    -1,   100,    -1,   110,    -1,    30,    -1,
      32,    66,   153,    67,    -1,    32,    66,   153,     1,    67,
      -1,    32,    66,   126,    67,    -1,    32,    66,   126,     1,
      67,    -1,    45,    -1,    46,    -1,   101,    -1,   101,    64,
     103,    69,    -1,   101,    64,   103,     1,    69,    -1,   102,
      -1,   102,     3,    -1,   102,    30,    -1,   102,   162,    -1,
      48,    -1,    49,    -1,    -1,   103,   106,    -1,   105,    -1,
     104,    73,   105,    -1,   114,    -1,   114,    70,   124,    -1,
     107,    72,    -1,   107,   108,    72,    -1,   107,     1,    72,
      -1,    98,    -1,    98,   107,    -1,    99,    -1,    99,   107,
      -1,   109,    -1,   108,    73,   109,    -1,   114,    -1,    71,
     141,    -1,   114,    71,   141,    -1,    50,    64,   111,    69,
      -1,    50,    64,   111,     1,    69,    -1,    50,     3,    64,
     111,    69,    -1,    50,     3,    64,   111,     1,    69,    -1,
      50,     3,    -1,   112,    -1,   112,    73,    -1,   113,    -1,
     112,    73,   113,    -1,     3,    -1,     3,    70,   141,    -1,
     115,    -1,   117,   115,    -1,     3,    -1,   162,    -1,    66,
     114,    67,    -1,    66,   114,     1,    67,    -1,   115,    65,
      68,    -1,   115,    65,     1,    68,    -1,   115,    65,   141,
      68,    -1,   115,    65,   141,     1,    68,    -1,   116,    67,
      -1,   116,     1,    67,    -1,   116,   119,    67,    -1,   116,
     119,     1,    67,    -1,   116,   120,    67,    -1,   116,   120,
       1,    67,    -1,   115,    66,    -1,    75,    -1,    75,   118,
      -1,    75,   117,    -1,    75,   118,   117,    -1,    99,    -1,
     118,    99,    -1,   121,    -1,   121,    73,    51,    -1,   123,
      -1,   123,    73,    51,    -1,   122,    -1,   121,    73,   122,
      -1,   107,   114,    -1,   126,    -1,     3,    -1,   123,    73,
       3,    -1,   138,    -1,    64,   125,    69,    -1,    64,   125,
       1,    69,    -1,    64,   125,    73,    69,    -1,    64,   125,
       1,    73,    69,    -1,   124,    -1,   125,    73,   124,    -1,
     107,    -1,   107,   127,    -1,   117,    -1,   128,    -1,   117,
     128,    -1,    66,   127,    67,    -1,    66,   127,     1,    67,
      -1,    65,    68,    -1,    65,     1,    68,    -1,    65,   141,
      68,    -1,    65,   141,     1,    68,    -1,   128,    65,    68,
      -1,   128,    65,     1,    68,    -1,   128,    65,   141,    68,
      -1,   128,    65,   141,     1,    68,    -1,    66,    67,    -1,
      66,     1,    67,    -1,    66,   119,    67,    -1,    66,   119,
       1,    67,    -1,   128,    66,    67,    -1,   128,    66,     1,
      67,    -1,   128,    66,   119,    67,    -1,   128,    66,   119,
       1,    67,    -1,   130,    -1,   131,    -1,   132,    -1,   134,
      -1,   135,    -1,   136,    -1,     3,    71,   129,    -1,    52,
     141,    71,   129,    -1,    53,    71,   129,    -1,    72,    -1,
     137,    72,    -1,   137,     1,    72,    -1,    64,    69,    -1,
      64,     1,    69,    -1,    64,   133,    69,    -1,    64,   133,
       1,    69,    -1,    64,    95,    69,    -1,    64,    95,     1,
      69,    -1,    64,    95,   133,    69,    -1,    64,    95,   133,
       1,    69,    -1,   129,    -1,   133,   129,    -1,    54,    66,
     137,    67,   129,    -1,    54,    66,   137,    67,   129,    55,
     129,    -1,    56,    66,   137,    67,   129,    -1,    57,    66,
     137,    67,   129,    -1,    58,   129,    57,    66,   137,    67,
      72,    -1,    59,    66,    72,    72,    67,   129,    -1,    59,
      66,    72,    72,   137,    67,   129,    -1,    59,    66,    72,
     137,    72,    67,   129,    -1,    59,    66,    72,   137,    72,
     137,    67,   129,    -1,    59,    66,   137,    72,    72,    67,
     129,    -1,    59,    66,   137,    72,    72,   137,    67,   129,
      -1,    59,    66,   137,    72,   137,    72,    67,   129,    -1,
      59,    66,   137,    72,   137,    72,   137,    67,   129,    -1,
      60,     3,    72,    -1,    61,    72,    -1,    62,    72,    -1,
      63,    72,    -1,    63,   137,    72,    -1,    63,   137,     1,
      72,    -1,   138,    -1,   137,    73,   138,    -1,   140,    -1,
     153,   139,   138,    -1,   158,   139,   138,    -1,    70,    -1,
      20,    -1,    21,    -1,    22,    -1,    23,    -1,    24,    -1,
      27,    -1,    28,    -1,    29,    -1,    25,    -1,    26,    -1,
     142,    -1,   142,    80,   142,    71,   140,    -1,   142,    80,
      71,   140,    -1,   140,    -1,   143,    -1,   142,    19,   143,
      -1,   144,    -1,   143,    18,   144,    -1,   145,    -1,   144,
      81,   145,    -1,   146,    -1,   145,    82,   146,    -1,   147,
      -1,   146,    74,   147,    -1,   148,    -1,   147,    16,   148,
      -1,   147,    17,   148,    -1,   149,    -1,   148,    83,   149,
      -1,   148,    84,   149,    -1,   148,    14,   149,    -1,   148,
      15,   149,    -1,   150,    -1,   149,    12,   150,    -1,   149,
      13,   150,    -1,   151,    -1,   150,    76,   151,    -1,   150,
      77,   151,    -1,   152,    -1,   151,    75,   152,    -1,   151,
      85,   152,    -1,   151,    86,   152,    -1,   153,    -1,   154,
      -1,    66,   126,    67,   152,    -1,    66,   126,     1,    67,
     152,    -1,   156,    -1,    10,   153,    -1,    11,   153,    -1,
     155,   152,    -1,     8,   153,    -1,     8,    66,   126,    67,
      -1,     8,    66,   126,     1,    67,    -1,   157,    -1,    10,
     154,    -1,    11,   154,    -1,     8,   154,    -1,     8,   163,
      -1,    74,    -1,    75,    -1,    76,    -1,    77,    -1,    78,
      -1,    79,    -1,   159,    -1,   156,    65,   137,    68,    -1,
     156,    65,   137,     1,    68,    -1,   156,    66,    67,    -1,
     156,    66,     1,    67,    -1,   156,    66,   160,    67,    -1,
     156,    66,   160,     1,    67,    -1,   156,    87,     3,    -1,
     156,     9,     3,    -1,   156,    10,    -1,   156,    11,    -1,
     158,    -1,   157,    10,    -1,   157,    11,    -1,   162,    -1,
     163,    65,   137,    68,    -1,   163,    65,   137,     1,    68,
      -1,     3,    -1,     6,    -1,   161,    -1,    66,   137,    67,
      -1,    66,   137,     1,    67,    -1,    66,   132,    67,    -1,
      66,   132,     1,    67,    -1,   138,    -1,   160,    73,   138,
      -1,     7,    -1,   161,     7,    -1,     4,    -1,    66,   162,
      67,    -1,     5,    -1,    66,   163,    67,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   147,   147,   152,   160,   165,   173,   182,   182,   198,
     203,   211,   219,   246,   258,   263,   271,   281,   294,   299,
     304,   309,   317,   322,   327,   332,   337,   342,   347,   352,
     357,   362,   367,   372,   377,   382,   387,   393,   398,   407,
     412,   420,   425,   430,   439,   444,   451,   457,   465,   470,
     478,   483,   491,   496,   512,   517,   525,   530,   535,   544,
     549,   554,   559,   567,   572,   580,   586,   591,   600,   605,
     611,   617,   624,   633,   638,   646,   651,   659,   665,   674,
     690,   709,   714,   721,   726,   732,   737,   743,   748,   754,
     762,   771,   779,   788,   796,   808,   819,   824,   829,   834,
     842,   847,   855,   860,   868,   873,   881,   886,   894,   900,
     908,   914,   923,   928,   933,   939,   944,   953,   958,   966,
     971,   979,   984,   989,   997,  1002,  1008,  1013,  1019,  1024,
    1030,  1035,  1041,  1046,  1052,  1057,  1063,  1068,  1074,  1079,
    1085,  1090,  1099,  1104,  1109,  1114,  1119,  1124,  1132,  1138,
    1143,  1151,  1156,  1161,  1170,  1175,  1181,  1186,  1192,  1197,
    1203,  1208,  1217,  1222,  1230,  1235,  1240,  1248,  1253,  1258,
    1263,  1268,  1273,  1278,  1283,  1288,  1293,  1301,  1307,  1312,
    1317,  1322,  1327,  1336,  1341,  1349,  1355,  1361,  1386,  1392,
    1398,  1404,  1410,  1416,  1422,  1428,  1434,  1440,  1446,  1455,
    1460,  1466,  1480,  1488,  1493,  1501,  1506,  1514,  1519,  1527,
    1532,  1540,  1545,  1553,  1558,  1563,  1571,  1576,  1581,  1586,
    1591,  1599,  1604,  1609,  1617,  1622,  1627,  1635,  1640,  1645,
    1650,  1658,  1663,  1668,  1673,  1682,  1687,  1692,  1697,  1702,
    1707,  1712,  1721,  1734,  1756,  1778,  1783,  1809,  1814,  1819,
    1824,  1829,  1834,  1842,  1847,  1852,  1858,  1863,  1869,  1874,
    1880,  1886,  1892,  1897,  1905,  1918,  1940,  1965,  1979,  2001,
    2010,  2016,  2021,  2026,  2031,  2037,  2042,  2051,  2056,  2064,
    2069,  2077,  2099,  2125,  2148
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "IMM_IDENTIFIER",
  "IMM_ARRAY_IDENTIFIER", "CONSTANT", "STRING_LITERAL", "SIZEOF", "PTR_OP",
  "INC_OP", "DEC_OP", "LEFT_SH", "RIGHT_SH", "LE_OP", "GE_OP", "EQ_OP",
  "NE_OP", "AND_OP", "OR_OP", "MUL_ASSIGN", "DIV_ASSIGN", "MOD_ASSIGN",
  "ADD_ASSIGN", "SUB_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN", "AND_ASSIGN",
  "XOR_ASSIGN", "OR_ASSIGN", "TYPE_NAME", "TYPEDEF", "TYPEOF", "EXTERN",
  "STATIC", "AUTO", "REGISTER", "CHAR", "SHORT", "INT", "LONG", "SIGNED",
  "UNSIGNED", "FLOAT", "DOUBLE", "CONST", "VOLATILE", "VOID", "STRUCT",
  "UNION", "ENUM", "ELIPSIS", "CASE", "DEFAULT", "IF", "ELSE", "SWITCH",
  "WHILE", "DO", "FOR", "GOTO", "CONTINUE", "BREAK", "RETURN", "'{'",
  "'['", "'('", "')'", "']'", "'}'", "'='", "':'", "';'", "','", "'&'",
  "'*'", "'+'", "'-'", "'~'", "'!'", "'?'", "'|'", "'^'", "'<'", "'>'",
  "'/'", "'%'", "'.'", "$accept", "translation_unit",
  "external_declaration", "function_definition", "$@1", "function_body",
  "declaration", "declaration_list", "declaration_specifiers",
  "storage_class_specifier", "type_specifier", "type_qualifier",
  "struct_or_union_specifier", "struct_or_union_tag", "struct_or_union",
  "struct_declaration_list", "init_declarator_list", "init_declarator",
  "struct_declaration", "specifier_qualifier_list",
  "struct_declarator_list", "struct_declarator", "enum_specifier",
  "enumerator_list_comma", "enumerator_list", "enumerator", "declarator",
  "direct_declarator", "parameter_head", "pointer", "type_qualifier_list",
  "parameter_type_list", "parameter_identifier_list", "parameter_list",
  "parameter_declaration", "identifier_list", "initializer",
  "initializer_list", "type_name", "abstract_declarator",
  "direct_abstract_declarator", "statement", "labeled_statement",
  "expression_statement", "compound_statement", "statement_list",
  "selection_statement", "iteration_statement", "jump_statement",
  "expression", "assignment_expression", "assignment_operator",
  "conditional_expression", "constant_expression", "logical_OR_expression",
  "logical_AND_expression", "inclusive_OR_expression",
  "exclusive_OR_expression", "AND_expression", "equality_expression",
  "relational_expression", "shift_expression", "additive_expression",
  "multiplicative_expression", "cast_expression", "unary_expression",
  "imm_unary_expression", "unary_operator", "postfix_expression",
  "imm_postfix_expression", "imm_lvalue", "primary_expression",
  "argument_expr_list", "string_literal", "imm_identifier",
  "imm_array_identifier", YY_NULL
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   123,    91,    40,    41,    93,   125,
      61,    58,    59,    44,    38,    42,    43,    45,   126,    33,
      63,   124,    94,    60,    62,    47,    37,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    88,    89,    89,    90,    90,    91,    92,    91,    93,
      93,    94,    94,    94,    95,    95,    96,    96,    97,    97,
      97,    97,    98,    98,    98,    98,    98,    98,    98,    98,
      98,    98,    98,    98,    98,    98,    98,    98,    98,    99,
      99,   100,   100,   100,   101,   101,   101,   101,   102,   102,
     103,   103,   104,   104,   105,   105,   106,   106,   106,   107,
     107,   107,   107,   108,   108,   109,   109,   109,   110,   110,
     110,   110,   110,   111,   111,   112,   112,   113,   113,   114,
     114,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   116,   117,   117,   117,   117,
     118,   118,   119,   119,   120,   120,   121,   121,   122,   122,
     123,   123,   124,   124,   124,   124,   124,   125,   125,   126,
     126,   127,   127,   127,   128,   128,   128,   128,   128,   128,
     128,   128,   128,   128,   128,   128,   128,   128,   128,   128,
     128,   128,   129,   129,   129,   129,   129,   129,   130,   130,
     130,   131,   131,   131,   132,   132,   132,   132,   132,   132,
     132,   132,   133,   133,   134,   134,   134,   135,   135,   135,
     135,   135,   135,   135,   135,   135,   135,   136,   136,   136,
     136,   136,   136,   137,   137,   138,   138,   138,   139,   139,
     139,   139,   139,   139,   139,   139,   139,   139,   139,   140,
     140,   140,   141,   142,   142,   143,   143,   144,   144,   145,
     145,   146,   146,   147,   147,   147,   148,   148,   148,   148,
     148,   149,   149,   149,   150,   150,   150,   151,   151,   151,
     151,   152,   152,   152,   152,   153,   153,   153,   153,   153,
     153,   153,   154,   154,   154,   154,   154,   155,   155,   155,
     155,   155,   155,   156,   156,   156,   156,   156,   156,   156,
     156,   156,   156,   156,   157,   157,   157,   158,   158,   158,
     159,   159,   159,   159,   159,   159,   159,   160,   160,   161,
     161,   162,   162,   163,   163
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     1,     1,     2,     0,     4,     1,
       2,     2,     3,     3,     1,     2,     1,     2,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     4,     5,     4,     5,     1,
       1,     1,     4,     5,     1,     2,     2,     2,     1,     1,
       0,     2,     1,     3,     1,     3,     2,     3,     3,     1,
       2,     1,     2,     1,     3,     1,     2,     3,     4,     5,
       5,     6,     2,     1,     2,     1,     3,     1,     3,     1,
       2,     1,     1,     3,     4,     3,     4,     4,     5,     2,
       3,     3,     4,     3,     4,     2,     1,     2,     2,     3,
       1,     2,     1,     3,     1,     3,     1,     3,     2,     1,
       1,     3,     1,     3,     4,     4,     5,     1,     3,     1,
       2,     1,     1,     2,     3,     4,     2,     3,     3,     4,
       3,     4,     4,     5,     2,     3,     3,     4,     3,     4,
       4,     5,     1,     1,     1,     1,     1,     1,     3,     4,
       3,     1,     2,     3,     2,     3,     3,     4,     3,     4,
       4,     5,     1,     2,     5,     7,     5,     5,     7,     6,
       7,     7,     8,     7,     8,     8,     9,     3,     2,     2,
       2,     3,     4,     1,     3,     1,     3,     3,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       5,     4,     1,     1,     3,     1,     3,     1,     3,     1,
       3,     1,     3,     1,     3,     3,     1,     3,     3,     3,
       3,     1,     3,     3,     1,     3,     3,     1,     3,     3,
       3,     1,     1,     4,     5,     1,     2,     2,     2,     2,
       4,     5,     1,     2,     2,     2,     2,     1,     1,     1,
       1,     1,     1,     1,     4,     5,     3,     4,     4,     5,
       3,     3,     2,     2,     1,     2,     2,     1,     4,     5,
       1,     1,     1,     3,     4,     3,     4,     1,     3,     1,
       2,     1,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     1,    81,   281,    34,    21,     0,    20,    19,
      18,    23,    24,    25,    26,    27,    30,    31,    28,    29,
      39,    40,    22,    48,    49,     0,     0,    96,     3,     4,
       5,     0,     0,    59,    61,    32,    41,    44,    16,    33,
       0,    79,     0,     0,    82,     0,    72,     0,     0,    82,
     100,    98,    97,     0,    11,     0,    52,     7,    17,    60,
      62,    50,    45,    46,     0,    47,     0,     6,    14,     0,
       0,     9,     0,    95,     0,   110,    89,   119,     0,     0,
     102,   106,   104,   109,    80,   270,   271,   279,     0,     0,
       0,     0,   247,   248,   249,   250,   251,   252,   119,     0,
       0,     0,   235,   253,   272,     0,    77,     0,    73,    75,
       0,    83,   282,   101,    99,    13,    12,     0,     0,     0,
       0,     0,     0,   270,   283,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     154,   151,     0,   162,   142,   143,   144,     0,   145,   146,
     147,     0,   183,   185,   199,   203,   205,   207,   209,   211,
     213,   216,   221,   224,   227,   231,   232,   242,   264,   267,
       0,    15,    10,    54,     0,    85,   202,     0,   231,   264,
      90,     0,     0,   108,   121,   120,   122,     0,    91,     0,
      93,     0,     0,     0,   239,   236,   237,     0,     0,     0,
     121,     0,    37,     0,    35,   238,     0,   262,   263,     0,
       0,     0,   280,     0,     0,     0,    68,    74,    84,    53,
       0,    55,   112,     8,     0,    42,    51,     0,   155,     0,
       0,   245,   246,     0,   243,   244,     0,     0,     0,     0,
       0,     0,     0,     0,   178,   179,   180,     0,     0,   267,
       0,     0,   158,     0,     0,   156,   163,     0,   152,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   189,
     190,   191,   192,   193,   197,   198,   194,   195,   196,   188,
       0,   265,   266,     0,     0,    86,     0,    87,     0,   126,
       0,     0,   134,     0,     0,   123,     0,     0,    92,    94,
     103,   107,   111,   105,     0,     0,   275,     0,   273,    38,
      36,   261,     0,     0,   256,   277,     0,   260,     0,    70,
      78,    69,    76,   117,     0,    43,     0,     0,    56,     0,
      63,    65,   148,     0,   150,     0,     0,     0,     0,     0,
       0,   177,     0,   181,     0,     0,   284,   159,     0,   160,
     157,   153,   184,   204,     0,     0,   206,   208,   210,   212,
     214,   215,   219,   220,   217,   218,   222,   223,   225,   226,
     228,   229,   230,   186,   187,     0,    88,   127,     0,   128,
     135,     0,   136,     0,   124,     0,   130,     0,     0,   138,
       0,     0,   240,   276,   274,     0,   254,   257,     0,   258,
       0,    71,     0,   113,     0,    58,    66,    57,     0,     0,
     149,     0,     0,     0,     0,     0,     0,     0,   182,     0,
     233,   161,   201,     0,     0,   268,   129,   137,   125,   131,
       0,   132,   139,     0,   140,   241,   255,   259,   278,   114,
       0,   115,   118,    64,    67,   164,   166,   167,     0,     0,
       0,     0,     0,     0,   234,   200,   269,   133,   141,   116,
       0,     0,   169,     0,     0,     0,     0,     0,     0,   165,
     168,   170,   171,     0,   173,     0,     0,     0,   172,   174,
     175,     0,   176
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    28,    29,   119,    67,    68,    69,    70,    32,
      33,    34,    35,    36,    37,   120,    55,    56,   226,    38,
     339,   340,    39,   107,   108,   109,    48,    41,    42,    43,
      52,   303,    79,    80,    81,    82,   221,   334,    83,   185,
     186,   143,   144,   145,   146,   147,   148,   149,   150,   151,
     152,   290,   153,   177,   154,   155,   156,   157,   158,   159,
     160,   161,   162,   163,   164,   165,   166,   101,   102,   167,
     168,   103,   326,   104,   169,   170
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -218
static const yytype_int16 yypact[] =
{
    -218,  1411,  -218,  -218,  -218,  -218,  -218,   -52,  -218,  -218,
    -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,
    -218,  -218,  -218,  -218,  -218,   138,   156,     9,  -218,  -218,
    -218,   133,  1732,  1732,  1732,  -218,   -35,   267,  -218,  -218,
    1681,   168,  1611,   269,  -218,   894,   -30,    47,    90,    77,
    -218,  -218,     9,   -12,  -218,   250,  -218,   248,  -218,  -218,
    -218,  -218,  -218,  -218,    22,  -218,   488,  -218,  -218,  1681,
     133,  -218,   660,  -218,   108,  -218,  -218,   105,   191,   193,
     116,  -218,   146,  -218,   168,  -218,  -218,  -218,  1313,  1361,
    1361,   971,  -218,  -218,  -218,  -218,  -218,  -218,   -38,   194,
     215,  1278,     6,  -218,   249,    47,   166,    98,   186,  -218,
     199,  -218,  -218,  -218,  -218,  -218,  -218,   156,  1019,  1681,
    1561,    77,   212,   205,  -218,  1307,  1355,  1355,  1278,   236,
     225,   253,   258,   942,   265,   331,   264,   293,  1048,   844,
    -218,  -218,   567,  -218,  -218,  -218,  -218,   397,  -218,  -218,
    -218,     8,  -218,  -218,   155,   321,   288,   294,   318,   311,
      31,   317,   266,    91,  -218,   562,  -218,   353,   562,  -218,
     324,  -218,  -218,   327,   345,  -218,  -218,    43,  -218,  -218,
    -218,   674,  1461,  -218,   260,  -218,   302,   316,  -218,   332,
    -218,  1710,     2,   844,  -218,  -218,  -218,   219,    19,  1511,
     312,   349,  -218,   352,  -218,  -218,   419,  -218,  -218,  1278,
     753,   422,  -218,   149,  1278,   358,  -218,    47,  -218,  -218,
    1019,  -218,  -218,  -218,   359,  -218,  -218,    97,  -218,   942,
     844,  -218,   324,   971,  -218,  -218,   360,   942,  1278,  1278,
    1278,   372,  1062,   362,  -218,  -218,  -218,    23,   220,    77,
      20,   361,  -218,   646,   363,  -218,  -218,   365,  -218,  1278,
    1278,  1096,  1278,  1278,  1278,  1278,  1278,  1278,  1278,  1278,
    1278,  1278,  1278,  1278,  1278,  1278,  1278,  1278,  1278,  -218,
    -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,  -218,
    1278,  -218,  -218,  1278,  1278,  -218,   370,  -218,   374,  -218,
      74,   373,  -218,   222,   223,   302,   767,  1643,  -218,  -218,
    -218,  -218,  -218,  -218,   227,   376,  -218,   377,  -218,  -218,
    -218,  -218,    72,   378,  -218,  -218,   111,  -218,   383,  -218,
    -218,  -218,  -218,  -218,    56,  -218,   375,  1278,  -218,   307,
    -218,   391,  -218,   942,  -218,   -26,   115,   202,   380,  1139,
     309,  -218,   392,  -218,   398,  1278,  -218,  -218,   399,  -218,
    -218,  -218,  -218,   321,  1278,    -7,   288,   294,   318,   311,
      31,    31,   317,   317,   317,   317,   266,   266,    91,    91,
    -218,  -218,  -218,  -218,  -218,    81,  -218,  -218,   402,  -218,
    -218,   400,  -218,   410,  -218,   411,  -218,   132,   413,  -218,
     229,   414,  -218,  -218,  -218,   415,  -218,  -218,   417,  -218,
    1278,  -218,    66,  -218,   792,  -218,  -218,  -218,    55,  1278,
    -218,   942,   942,   942,  1278,  1153,   313,  1173,  -218,  1278,
    -218,  -218,  -218,  1278,   418,  -218,  -218,  -218,  -218,  -218,
     420,  -218,  -218,   423,  -218,  -218,  -218,  -218,  -218,  -218,
     409,  -218,  -218,  -218,  -218,   427,  -218,  -218,   207,   942,
     211,  1187,  1230,   315,  -218,  -218,  -218,  -218,  -218,  -218,
     942,   425,  -218,   942,   942,   228,   942,   233,  1264,  -218,
    -218,  -218,  -218,   942,  -218,   942,   942,   235,  -218,  -218,
    -218,   942,  -218
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -218,  -218,  -218,  -218,  -218,   366,     5,   421,   499,  -218,
    -218,   -17,  -218,  -218,  -218,  -218,  -218,   384,  -218,    86,
    -218,    87,  -218,   401,  -218,   285,     0,   -36,  -218,    -9,
    -218,   -40,  -218,  -218,   319,  -218,  -217,  -218,   -32,  -143,
    -152,   -50,  -218,  -218,   -29,   367,  -218,  -218,  -218,   -87,
     -97,   336,   -49,  -120,   246,   251,   252,   245,   275,   247,
     128,    41,   139,   140,   -63,   -23,   178,  -218,  -218,  -218,
      84,  -218,  -218,  -218,    -1,  -106
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -55
static const yytype_int16 yytable[] =
{
      44,    40,    78,   333,   198,   312,    30,    84,   236,   257,
      50,    71,   260,    99,    45,   206,   207,   208,    51,   232,
     317,   222,   100,   176,   352,    49,     4,   181,   199,    61,
      44,    57,   305,   250,   105,   113,    65,    27,   205,   304,
     172,   421,    44,   114,   296,   268,   269,   259,   305,   178,
     106,   247,   198,   313,    20,    21,   304,   412,     3,     4,
     115,   300,   197,   121,   433,   194,   195,   196,   184,    44,
     173,   209,   210,   405,   171,   388,    44,   183,   178,   176,
     258,   259,   434,   241,    27,   294,   318,   356,    64,   200,
      71,   110,   259,   211,   330,   353,   259,   256,   336,   215,
       3,     4,   194,   195,   196,   178,   198,   248,     3,     4,
     197,   297,   408,   325,   270,   271,    44,   173,    58,    59,
      60,    26,   322,   222,   250,   413,   337,   250,    77,   414,
      27,    98,   176,   440,    53,   449,     3,     4,   249,   450,
     406,    46,   389,   198,   112,   259,   198,   171,    84,   435,
     328,   345,   346,   347,   259,   350,   179,   111,   178,     3,
       4,   314,   362,    26,   197,   176,   276,   216,   337,   338,
     181,   182,    27,   184,   260,   180,   277,   278,   409,   342,
      27,    49,   422,    44,   410,   179,   397,   344,   259,   191,
     200,   178,   187,   383,   189,   201,   384,   452,   314,    26,
     441,   197,    47,   256,   197,    54,   227,   385,    27,   179,
     179,   179,   179,   380,   381,   382,   203,   416,   329,   192,
     315,   354,    26,   391,   393,    98,    44,   341,   401,   249,
     443,    27,   249,    72,    73,   261,   214,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   178,   178,   178,   178,
     178,   178,   178,   178,   178,   178,   212,   176,   188,   217,
     190,   202,   426,     3,     4,   179,   218,   400,    77,   423,
      62,     4,     3,     4,   471,   259,   229,    77,   473,    98,
     259,   228,   204,   178,   259,    77,   316,   355,   176,   392,
     394,   238,   430,   420,   402,   483,   444,    63,   179,   454,
     485,   259,   491,   231,   234,   235,   259,   237,   259,   372,
     373,   374,   375,   448,   178,   432,    98,   222,   118,   239,
     -54,   -54,   116,   117,   240,   181,   182,   266,   267,   272,
     273,   242,   178,    64,   243,    26,   244,   458,   460,   262,
     463,   178,   274,   275,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   291,   292,   245,   464,   306,   307,   263,
     176,   455,   456,   457,   475,   477,   264,   181,   199,   417,
     418,   427,   259,   308,   465,   461,   259,   478,   259,   294,
     179,   487,   265,    77,   370,   371,   178,   118,   254,   309,
     123,     4,   124,    86,    87,   125,   178,   126,   127,   472,
     178,   376,   377,   295,   378,   379,   319,    44,   341,   320,
     479,   179,   321,   481,   482,   327,   484,   331,   335,   348,
     357,   343,   360,   488,   351,   489,   490,   361,   386,   179,
     390,   492,   387,   403,   404,   407,   424,   415,   179,   128,
     129,   130,   411,   131,   132,   133,   134,   135,   136,   137,
     138,    66,   419,   139,   428,   429,   255,   437,   431,   141,
     436,    92,    93,    94,    95,    96,    97,   438,   469,   439,
     442,   445,   470,   446,   447,   223,   466,   142,   467,   122,
     468,   123,     4,   124,    86,    87,   125,   480,   126,   127,
      31,   219,   332,   179,   293,   453,   213,   365,   367,   253,
     311,   363,   369,   179,   366,     0,     0,   179,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,   368,
     128,   129,   130,     0,   131,   132,   133,   134,   135,   136,
     137,   138,    66,     0,   139,     0,     0,   140,     0,     0,
     141,     0,    92,    93,    94,    95,    96,    97,   251,     0,
     123,     4,   124,    86,    87,   125,     0,   126,   127,     0,
       0,     0,   279,   280,   281,   282,   283,   284,   285,   286,
     287,   288,     0,     0,     0,     0,     0,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,   128,
     129,   130,     0,   131,   132,   133,   134,   135,   136,   137,
     138,    66,   289,   139,     0,     0,   252,     0,     0,   141,
       0,    92,    93,    94,    95,    96,    97,   358,     0,   123,
       4,   124,    86,    87,   125,     0,   126,   127,     0,     0,
       0,   174,     0,    85,     4,   124,    86,    87,   125,     0,
     126,   127,     0,     0,     0,   298,     0,    85,     4,   124,
      86,    87,   125,     0,   126,   127,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   128,   129,
     130,     0,   131,   132,   133,   134,   135,   136,   137,   138,
      66,     0,   139,     0,     0,   359,     0,     0,   141,     0,
      92,    93,    94,    95,    96,    97,   139,     0,   175,     0,
       0,     0,     0,     0,    92,    93,    94,    95,    96,    97,
     139,     0,   299,     0,     0,     0,     0,     0,    92,    93,
      94,    95,    96,    97,   323,     0,    85,     4,   124,    86,
      87,   125,     0,   126,   127,     0,     0,     0,   395,     0,
      85,     4,   124,    86,    87,   125,     0,   126,   127,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    85,     4,   124,    86,    87,
     125,     0,   126,   127,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,   139,
     324,     0,     0,     0,     0,     0,     0,    92,    93,    94,
      95,    96,    97,   139,     0,   396,     0,     0,     0,     0,
       0,    92,    93,    94,    95,    96,    97,    85,     4,   124,
      86,    87,   125,     0,   126,   127,   220,     0,   139,     0,
       0,   451,     0,     0,     0,     0,    92,    93,    94,    95,
      96,    97,     0,     0,     5,     0,     7,     0,     0,     0,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     0,     0,    85,     0,     0,
      86,    87,    88,     0,    89,    90,     0,     0,    66,     0,
     139,     0,     0,     0,     0,     0,     0,     0,    92,    93,
      94,    95,    96,    97,     5,     0,     7,     0,     0,     0,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,   123,     4,   124,    86,    87,
     125,     0,   126,   127,     0,     0,     0,     0,     0,     0,
      91,     0,     0,     0,     0,     0,     0,     0,    92,    93,
      94,    95,    96,    97,    85,     4,   124,    86,    87,   125,
       0,   126,   127,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   128,   129,   130,     0,   131,   132,
     133,   134,   135,   136,   137,   138,    66,     0,   139,     0,
       0,     0,     0,     0,   141,     0,    92,    93,    94,    95,
      96,    97,    85,     4,   124,    86,    87,   125,     0,   126,
     127,     0,     0,     0,     0,    66,     0,   139,     0,     0,
       0,     0,     0,     0,     0,    92,    93,    94,    95,    96,
      97,    85,     4,   124,    86,    87,   125,     0,   126,   127,
       0,     0,     0,     0,     0,    85,     4,   124,    86,    87,
     125,     0,   126,   127,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   220,     0,   139,     0,     0,     0,     0,
       0,     0,     0,    92,    93,    94,    95,    96,    97,    85,
       4,   124,    86,    87,   125,     0,   126,   127,     0,     0,
       0,     0,     0,     0,   139,     0,     0,     0,     0,     0,
     246,     0,    92,    93,    94,    95,    96,    97,   139,     0,
       0,     0,     0,     0,   349,     0,    92,    93,    94,    95,
      96,    97,    85,     4,   124,    86,    87,   125,     0,   126,
     127,     0,     0,     0,     0,     0,    85,     4,   124,    86,
      87,   125,   139,   126,   127,     0,     0,   364,     0,     0,
      92,    93,    94,    95,    96,    97,    85,     4,   124,    86,
      87,   125,     0,   126,   127,     0,     0,     0,     0,     0,
      85,     4,   124,    86,    87,   125,     0,   126,   127,     0,
       0,     0,     0,     0,     0,   139,     0,     0,     0,     0,
       0,   425,     0,    92,    93,    94,    95,    96,    97,   139,
     459,     0,     0,     0,     0,     0,     0,    92,    93,    94,
      95,    96,    97,    85,     4,   124,    86,    87,   125,   139,
     126,   127,     0,     0,     0,   462,     0,    92,    93,    94,
      95,    96,    97,   139,   474,     0,     0,     0,     0,     0,
       0,    92,    93,    94,    95,    96,    97,    85,     4,   124,
      86,    87,   125,     0,   126,   127,     0,     0,     0,     0,
       0,    85,     4,   124,    86,    87,   125,     0,   126,   127,
       0,     0,     0,     0,     0,     0,   139,   476,     0,     0,
       0,     0,     0,     0,    92,    93,    94,    95,    96,    97,
      85,     4,   124,    86,    87,   125,    85,   126,   127,    86,
      87,    88,     0,    89,    90,     0,     0,     0,     0,     0,
     139,   486,     0,     0,     0,     0,     0,     0,    92,    93,
      94,    95,    96,    97,   139,     0,     0,     0,     0,     0,
       0,     0,    92,    93,    94,    95,    96,    97,    85,     4,
     124,    86,    87,   125,    85,   126,   127,    86,    87,    88,
       0,    89,    90,   230,     0,     0,     0,     0,     0,   193,
       0,    92,    93,    94,    95,    96,    97,    92,    93,    94,
      95,    96,    97,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     2,     0,     0,     3,     4,     0,     0,     0,     0,
       0,   233,     0,     0,     0,     0,     0,    91,     0,    92,
      93,    94,    95,    96,    97,    92,    93,    94,    95,    96,
      97,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,   301,     0,     3,     4,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    26,     0,     0,
       0,     0,     0,     0,     0,     0,    27,     0,     0,     0,
       0,     5,     0,     7,     0,     0,     0,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,   301,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   181,   182,   302,     0,
       0,     0,     0,     0,     0,     0,    27,     0,     0,     0,
       0,     5,     0,     7,     0,     0,     0,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,   224,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   181,   199,   302,     0,
       0,     0,     0,     0,     0,     0,    27,     0,     0,     0,
       0,     5,     0,     7,     0,     0,     0,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,    74,     0,    75,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     225,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     0,     7,   398,     0,     0,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     5,     0,     7,     0,     0,    76,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     399,     5,     6,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,     0,     0,     0,     0,     0,     0,     0,     0,
       5,     0,     7,     0,     0,    66,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,   310,     5,     0,     7,     0,     0,     0,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-218)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       1,     1,    42,   220,    91,     3,     1,    43,   128,     1,
      27,    40,    19,    45,    66,     9,    10,    11,    27,   125,
       1,   118,    45,    72,     1,    26,     4,    65,    66,    64,
      31,    31,   184,   139,    64,    52,    37,    75,   101,   182,
      69,    67,    43,    52,     1,    14,    15,    73,   200,    72,
       3,   138,   139,    51,    45,    46,   199,     1,     3,     4,
      72,   181,    91,    64,    71,    88,    89,    90,    77,    70,
      70,    65,    66,     1,    69,     1,    77,    77,   101,   128,
      72,    73,     1,   133,    75,    65,    67,    67,    66,    98,
     119,     1,    73,    87,   214,    72,    73,   147,     1,     1,
       3,     4,   125,   126,   127,   128,   193,   139,     3,     4,
     139,    68,     1,   210,    83,    84,   117,   117,    32,    33,
      34,    66,   209,   220,   230,    69,    71,   233,    42,    73,
      75,    45,   181,     1,     1,    69,     3,     4,   139,    73,
      68,     3,    68,   230,    67,    73,   233,   142,   184,    68,
       1,   238,   239,   240,    73,   242,    72,    67,   181,     3,
       4,   193,   259,    66,   193,   214,    75,    69,    71,    72,
      65,    66,    75,   182,    19,    67,    85,    86,    67,   229,
      75,   182,    67,   184,    73,   101,   306,   237,    73,    73,
     199,   214,     1,   290,     1,     1,   293,   414,   230,    66,
      68,   230,    64,   253,   233,    72,   120,   294,    75,   125,
     126,   127,   128,   276,   277,   278,     1,   337,    69,    73,
       1,     1,    66,     1,     1,   139,   227,   227,     1,   230,
       1,    75,   233,    65,    66,    80,    70,   260,   261,   262,
     263,   264,   265,   266,   267,   268,   269,   270,   271,   272,
     273,   274,   275,   276,   277,   278,     7,   306,    67,    73,
      67,    67,   349,     3,     4,   181,    67,   307,   182,    67,
       3,     4,     3,     4,    67,    73,    71,   191,    67,   193,
      73,    69,    67,   306,    73,   199,    67,    67,   337,    67,
      67,    66,   355,   343,    67,    67,    67,    30,   214,   419,
      67,    73,    67,   125,   126,   127,    73,    71,    73,   268,
     269,   270,   271,   410,   337,   364,   230,   414,    70,    66,
      72,    73,    72,    73,    66,    65,    66,    16,    17,    12,
      13,    66,   355,    66,     3,    66,    72,   424,   425,    18,
     427,   364,    76,    77,   260,   261,   262,   263,   264,   265,
     266,   267,   268,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,    10,    11,    72,   429,    65,    66,    81,
     419,   421,   422,   423,   461,   462,    82,    65,    66,    72,
      73,    72,    73,    67,   433,    72,    73,    72,    73,    65,
     306,   478,    74,   307,   266,   267,   419,    70,     1,    67,
       3,     4,     5,     6,     7,     8,   429,    10,    11,   459,
     433,   272,   273,    68,   274,   275,    67,   418,   418,    67,
     470,   337,     3,   473,   474,     3,   476,    69,    69,    57,
      69,    71,    69,   483,    72,   485,   486,    72,    68,   355,
      67,   491,    68,    67,    67,    67,    66,    72,   364,    52,
      53,    54,    69,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    71,    66,    72,    67,    69,    67,    69,    72,
      68,    74,    75,    76,    77,    78,    79,    67,    69,    68,
      67,    67,    55,    68,    67,   119,    68,    66,    68,     1,
      67,     3,     4,     5,     6,     7,     8,    72,    10,    11,
       1,   117,   217,   419,   168,   418,   105,   261,   263,   142,
     191,   260,   265,   429,   262,    -1,    -1,   433,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,   264,
      52,    53,    54,    -1,    56,    57,    58,    59,    60,    61,
      62,    63,    64,    -1,    66,    -1,    -1,    69,    -1,    -1,
      72,    -1,    74,    75,    76,    77,    78,    79,     1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    10,    11,    -1,
      -1,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    -1,    -1,    -1,    -1,    -1,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    -1,    52,
      53,    54,    -1,    56,    57,    58,    59,    60,    61,    62,
      63,    64,    70,    66,    -1,    -1,    69,    -1,    -1,    72,
      -1,    74,    75,    76,    77,    78,    79,     1,    -1,     3,
       4,     5,     6,     7,     8,    -1,    10,    11,    -1,    -1,
      -1,     1,    -1,     3,     4,     5,     6,     7,     8,    -1,
      10,    11,    -1,    -1,    -1,     1,    -1,     3,     4,     5,
       6,     7,     8,    -1,    10,    11,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    52,    53,
      54,    -1,    56,    57,    58,    59,    60,    61,    62,    63,
      64,    -1,    66,    -1,    -1,    69,    -1,    -1,    72,    -1,
      74,    75,    76,    77,    78,    79,    66,    -1,    68,    -1,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
      66,    -1,    68,    -1,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,     1,    -1,     3,     4,     5,     6,
       7,     8,    -1,    10,    11,    -1,    -1,    -1,     1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,    66,    -1,    68,    -1,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,     3,     4,     5,
       6,     7,     8,    -1,    10,    11,    64,    -1,    66,    -1,
      -1,    69,    -1,    -1,    -1,    -1,    74,    75,    76,    77,
      78,    79,    -1,    -1,    30,    -1,    32,    -1,    -1,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    -1,    -1,     3,    -1,    -1,
       6,     7,     8,    -1,    10,    11,    -1,    -1,    64,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    30,    -1,    32,    -1,    -1,    -1,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,     3,     4,     5,     6,     7,
       8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,     3,     4,     5,     6,     7,     8,
      -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    52,    53,    54,    -1,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    -1,    66,    -1,
      -1,    -1,    -1,    -1,    72,    -1,    74,    75,    76,    77,
      78,    79,     3,     4,     5,     6,     7,     8,    -1,    10,
      11,    -1,    -1,    -1,    -1,    64,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,
      79,     3,     4,     5,     6,     7,     8,    -1,    10,    11,
      -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
       8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    64,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    74,    75,    76,    77,    78,    79,     3,
       4,     5,     6,     7,     8,    -1,    10,    11,    -1,    -1,
      -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,    -1,
      72,    -1,    74,    75,    76,    77,    78,    79,    66,    -1,
      -1,    -1,    -1,    -1,    72,    -1,    74,    75,    76,    77,
      78,    79,     3,     4,     5,     6,     7,     8,    -1,    10,
      11,    -1,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
       7,     8,    66,    10,    11,    -1,    -1,    71,    -1,    -1,
      74,    75,    76,    77,    78,    79,     3,     4,     5,     6,
       7,     8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
       3,     4,     5,     6,     7,     8,    -1,    10,    11,    -1,
      -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,    -1,    -1,
      -1,    72,    -1,    74,    75,    76,    77,    78,    79,    66,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,    76,
      77,    78,    79,     3,     4,     5,     6,     7,     8,    66,
      10,    11,    -1,    -1,    -1,    72,    -1,    74,    75,    76,
      77,    78,    79,    66,    67,    -1,    -1,    -1,    -1,    -1,
      -1,    74,    75,    76,    77,    78,    79,     3,     4,     5,
       6,     7,     8,    -1,    10,    11,    -1,    -1,    -1,    -1,
      -1,     3,     4,     5,     6,     7,     8,    -1,    10,    11,
      -1,    -1,    -1,    -1,    -1,    -1,    66,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    74,    75,    76,    77,    78,    79,
       3,     4,     5,     6,     7,     8,     3,    10,    11,     6,
       7,     8,    -1,    10,    11,    -1,    -1,    -1,    -1,    -1,
      66,    67,    -1,    -1,    -1,    -1,    -1,    -1,    74,    75,
      76,    77,    78,    79,    66,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    74,    75,    76,    77,    78,    79,     3,     4,
       5,     6,     7,     8,     3,    10,    11,     6,     7,     8,
      -1,    10,    11,    66,    -1,    -1,    -1,    -1,    -1,    66,
      -1,    74,    75,    76,    77,    78,    79,    74,    75,    76,
      77,    78,    79,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,     0,    -1,    -1,     3,     4,    -1,    -1,    -1,    -1,
      -1,    66,    -1,    -1,    -1,    -1,    -1,    66,    -1,    74,
      75,    76,    77,    78,    79,    74,    75,    76,    77,    78,
      79,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     1,    -1,     3,     4,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    66,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    30,    -1,    32,    -1,    -1,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    30,    -1,    32,    -1,    -1,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    67,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    -1,    -1,    -1,
      -1,    30,    -1,    32,    -1,    -1,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,     1,    -1,     3,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      69,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    30,    -1,    32,     1,    -1,    -1,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    30,    -1,    32,    -1,    -1,    67,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    41,    42,    43,    44,    45,    46,    47,    48,
      49,    50,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      30,    -1,    32,    -1,    -1,    64,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    30,    -1,    32,    -1,    -1,    -1,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    89,     0,     3,     4,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    66,    75,    90,    91,
      94,    96,    97,    98,    99,   100,   101,   102,   107,   110,
     114,   115,   116,   117,   162,    66,     3,    64,   114,   162,
      99,   117,   118,     1,    72,   104,   105,   114,   107,   107,
     107,    64,     3,    30,    66,   162,    64,    93,    94,    95,
      96,   132,    65,    66,     1,     3,    67,   107,   119,   120,
     121,   122,   123,   126,   115,     3,     6,     7,     8,    10,
      11,    66,    74,    75,    76,    77,    78,    79,   107,   126,
     153,   155,   156,   159,   161,    64,     3,   111,   112,   113,
       1,    67,    67,    99,   117,    72,    72,    73,    70,    92,
     103,   162,     1,     3,     5,     8,    10,    11,    52,    53,
      54,    56,    57,    58,    59,    60,    61,    62,    63,    66,
      69,    72,    95,   129,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   140,   142,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   157,   158,   162,
     163,    94,   132,   114,     1,    68,   140,   141,   153,   158,
      67,    65,    66,   114,   117,   127,   128,     1,    67,     1,
      67,    73,    73,    66,   153,   153,   153,   132,   137,    66,
     117,     1,    67,     1,    67,   152,     9,    10,    11,    65,
      66,    87,     7,   111,    70,     1,    69,    73,    67,   105,
      64,   124,   138,    93,     1,    69,   106,   107,    69,    71,
      66,   154,   163,    66,   154,   154,   141,    71,    66,    66,
      66,   129,    66,     3,    72,    72,    72,   137,   126,   162,
     163,     1,    69,   133,     1,    69,   129,     1,    72,    73,
      19,    80,    18,    81,    82,    74,    16,    17,    14,    15,
      83,    84,    12,    13,    76,    77,    75,    85,    86,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    70,
     139,    10,    11,   139,    65,    68,     1,    68,     1,    68,
     141,     1,    67,   119,   127,   128,    65,    66,    67,    67,
      51,   122,     3,    51,   126,     1,    67,     1,    67,    67,
      67,     3,   137,     1,    67,   138,   160,     3,     1,    69,
     141,    69,   113,   124,   125,    69,     1,    71,    72,   108,
     109,   114,   129,    71,   129,   137,   137,   137,    57,    72,
     137,    72,     1,    72,     1,    67,    67,    69,     1,    69,
      69,    72,   138,   143,    71,   142,   144,   145,   146,   147,
     148,   148,   149,   149,   149,   149,   150,   150,   151,   151,
     152,   152,   152,   138,   138,   137,    68,    68,     1,    68,
      67,     1,    67,     1,    67,     1,    68,   141,     1,    67,
     119,     1,    67,    67,    67,     1,    68,    67,     1,    67,
      73,    69,     1,    69,    73,    72,   141,    72,    73,    71,
     129,    67,    67,    67,    66,    72,   137,    72,    72,    67,
     152,    69,   140,    71,     1,    68,    68,    67,    67,    68,
       1,    68,    67,     1,    67,    67,    68,    67,   138,    69,
      73,    69,   124,   109,   141,   129,   129,   129,   137,    67,
     137,    72,    72,   137,   152,   140,    68,    68,    67,    69,
      55,    67,   129,    67,    67,   137,    67,   137,    72,   129,
      72,   129,   129,    67,   129,    67,    67,   137,   129,   129,
     129,    67,   129
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  However,
   YYFAIL appears to be in use.  Nevertheless, it is formally deprecated
   in Bison 2.4.2's NEWS entry, where a plan to phase it out is
   discussed.  */

#define YYFAIL		goto yyerrlab
#if defined YYFAIL
  /* This is here to suppress warnings from the GCC cpp's
     -Wunused-macros.  Normally we don't worry about that warning, but
     some users do, and we want to make it easy for users to remove
     YYFAIL uses, which will produce warnings from Bison 2.5.  */
#endif

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))

/* Error token number */
#define YYTERROR	1
#define YYERRCODE	256


/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */
#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
        break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULL, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULL;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - Assume YYFAIL is not used.  It's too flawed to consider.  See
       <http://lists.gnu.org/archive/html/bison-patches/2009-12/msg00024.html>
       for details.  YYERROR is fine as it does not invoke this
       function.
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULL, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
        break;
    }
}




/* The lookahead symbol.  */
int yychar;


#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval YY_INITIAL_VALUE(yyval_default);

/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
/* Line 1792 of yacc.c  */
#line 147 "gram.y"
    {
	    (yyval.tok).start = 0;
	    (yyval.tok).end = 0;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 3:
/* Line 1792 of yacc.c  */
#line 152 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start ? (yyvsp[(1) - (2)].tok).start : (yyvsp[(2) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 4:
/* Line 1792 of yacc.c  */
#line 160 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 5:
/* Line 1792 of yacc.c  */
#line 165 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 6:
/* Line 1792 of yacc.c  */
#line 173 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    delete_sym(&(yyvsp[(1) - (2)].tok));
	    (yyval.tok).symbol = NULL;
#ifndef LMAIN
	    clearParaList(0);			/* restore all overloaded parameters */
#endif	/* LMAIN */
	}
    break;

  case 7:
/* Line 1792 of yacc.c  */
#line 182 "gram.y"
    {
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as function name */
#endif	/* LMAIN */
	}
    break;

  case 8:
/* Line 1792 of yacc.c  */
#line 187 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    delete_sym(&(yyvsp[(2) - (4)].tok));			/* not deleted if imm */
#ifndef LMAIN
	    clearParaList(0);			/* restore all overloaded parameters */
#endif	/* LMAIN */
	}
    break;

  case 9:
/* Line 1792 of yacc.c  */
#line 198 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 10:
/* Line 1792 of yacc.c  */
#line 203 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 11:
/* Line 1792 of yacc.c  */
#line 211 "gram.y"
    {	/* may have empty init_declarator_list according to K&R */
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;	/* GCC issues a warning: */
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;		/* "useless keyword or typname in empty declaration" */
	    (yyval.tok).symbol = NULL;		/* (see also rule 17) */
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as local var */
#endif	/* LMAIN */
	}
    break;

  case 12:
/* Line 1792 of yacc.c  */
#line 219 "gram.y"
    {
	    Symbol *	sp;
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    sp = (yyvsp[(2) - (3)].tok).symbol;
	    assert(sp);			/* ERROR: initialized in : init_declarator */
	    while (sp) {
		Symbol * sp1 = sp;
		sp = sp->next;		/* get next before Symbol is placed or deleted */
		if ((yyvsp[(1) - (3)].tok).symbol && (yyvsp[(1) - (3)].tok).symbol->type == UDF) {
		    sp1->type = CTYPE;		/* found a typedef */
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "\nP %-15s %d %d\n", sp1->name, sp1->type, sp1->ftype);
#endif
		    if (lookup(sp1->name) == 0) {
			link_sym(sp1);
		    }
		} else {
		    (yyval.tok).symbol = sp1;		/* use $$ as transport Token for delete_sym */
		    delete_sym(&(yyval.tok));
		}
	    }
	    (yyval.tok).symbol = NULL;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as local var */
#endif	/* LMAIN */
	}
    break;

  case 13:
/* Line 1792 of yacc.c  */
#line 246 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as local var */
#endif	/* LMAIN */
	    yyclearin; yyerrok;
	}
    break;

  case 14:
/* Line 1792 of yacc.c  */
#line 258 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 15:
/* Line 1792 of yacc.c  */
#line 263 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 16:
/* Line 1792 of yacc.c  */
#line 271 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
#ifndef LMAIN
	    if (lexflag & C_FUNCTION) {
		lexflag |= C_PARA;		/* function internal C var is coming */
	    }
#endif	/* LMAIN */
	}
    break;

  case 17:
/* Line 1792 of yacc.c  */
#line 281 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (2)].tok).symbol;		/* typedef information */
#ifndef LMAIN
	    if (lexflag & C_FUNCTION) {
		lexflag |= C_PARA;		/* function internal C var is coming */
	    }
#endif	/* LMAIN */
	}
    break;

  case 18:
/* Line 1792 of yacc.c  */
#line 294 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 19:
/* Line 1792 of yacc.c  */
#line 299 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 20:
/* Line 1792 of yacc.c  */
#line 304 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 21:
/* Line 1792 of yacc.c  */
#line 309 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = &typedefSymbol;
	}
    break;

  case 22:
/* Line 1792 of yacc.c  */
#line 317 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 23:
/* Line 1792 of yacc.c  */
#line 322 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 24:
/* Line 1792 of yacc.c  */
#line 327 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 25:
/* Line 1792 of yacc.c  */
#line 332 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 26:
/* Line 1792 of yacc.c  */
#line 337 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 27:
/* Line 1792 of yacc.c  */
#line 342 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 28:
/* Line 1792 of yacc.c  */
#line 347 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 29:
/* Line 1792 of yacc.c  */
#line 352 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 30:
/* Line 1792 of yacc.c  */
#line 357 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 31:
/* Line 1792 of yacc.c  */
#line 362 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 32:
/* Line 1792 of yacc.c  */
#line 367 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 33:
/* Line 1792 of yacc.c  */
#line 372 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 34:
/* Line 1792 of yacc.c  */
#line 377 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 35:
/* Line 1792 of yacc.c  */
#line 382 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 36:
/* Line 1792 of yacc.c  */
#line 387 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 37:
/* Line 1792 of yacc.c  */
#line 393 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 38:
/* Line 1792 of yacc.c  */
#line 398 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 39:
/* Line 1792 of yacc.c  */
#line 407 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 40:
/* Line 1792 of yacc.c  */
#line 412 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 41:
/* Line 1792 of yacc.c  */
#line 420 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 42:
/* Line 1792 of yacc.c  */
#line 425 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 43:
/* Line 1792 of yacc.c  */
#line 430 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 44:
/* Line 1792 of yacc.c  */
#line 439 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 45:
/* Line 1792 of yacc.c  */
#line 444 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    delete_sym(&(yyvsp[(2) - (2)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 46:
/* Line 1792 of yacc.c  */
#line 451 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 47:
/* Line 1792 of yacc.c  */
#line 457 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 48:
/* Line 1792 of yacc.c  */
#line 465 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 49:
/* Line 1792 of yacc.c  */
#line 470 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 50:
/* Line 1792 of yacc.c  */
#line 478 "gram.y"
    {		/* optional in modern C compilers (not K&R) */
	    (yyval.tok).start = 0;
	    (yyval.tok).end = 0;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 51:
/* Line 1792 of yacc.c  */
#line 483 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start ? (yyvsp[(1) - (2)].tok).start : (yyvsp[(2) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 52:
/* Line 1792 of yacc.c  */
#line 491 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (1)].tok).symbol;
	}
    break;

  case 53:
/* Line 1792 of yacc.c  */
#line 496 "gram.y"
    {
	    Symbol *	sp;
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    sp = (yyval.tok).symbol = (yyvsp[(1) - (3)].tok).symbol;
	    assert(sp);			/* ERROR: initialized in : init_declarator */
	    if (sp->type == CWORD) {
		while (sp->next) {
		    sp = sp->next;
		}
		sp->next = (yyvsp[(3) - (3)].tok).symbol;	/* place in a list */
	    }
	}
    break;

  case 54:
/* Line 1792 of yacc.c  */
#line 512 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (1)].tok).symbol;
	}
    break;

  case 55:
/* Line 1792 of yacc.c  */
#line 517 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (3)].tok).symbol;
	}
    break;

  case 56:
/* Line 1792 of yacc.c  */
#line 525 "gram.y"
    {/* nameless struct/union for moder C (not K&R) */
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;	/* should only be for struct or union, but GCC */
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;		/* only issues warning: "declaration does not declare anything" */
	    (yyval.tok).symbol = NULL;		/* for 'TYPE ;' in a struct_declaration (similar to 'declaration' rule 5) */
	}
    break;

  case 57:
/* Line 1792 of yacc.c  */
#line 530 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 58:
/* Line 1792 of yacc.c  */
#line 535 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 59:
/* Line 1792 of yacc.c  */
#line 544 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 60:
/* Line 1792 of yacc.c  */
#line 549 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 61:
/* Line 1792 of yacc.c  */
#line 554 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 62:
/* Line 1792 of yacc.c  */
#line 559 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 63:
/* Line 1792 of yacc.c  */
#line 567 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 64:
/* Line 1792 of yacc.c  */
#line 572 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 65:
/* Line 1792 of yacc.c  */
#line 580 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    delete_sym(&(yyvsp[(1) - (1)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 66:
/* Line 1792 of yacc.c  */
#line 586 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 67:
/* Line 1792 of yacc.c  */
#line 591 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    delete_sym(&(yyvsp[(1) - (3)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 68:
/* Line 1792 of yacc.c  */
#line 600 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 69:
/* Line 1792 of yacc.c  */
#line 605 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 70:
/* Line 1792 of yacc.c  */
#line 611 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    delete_sym(&(yyvsp[(2) - (5)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 71:
/* Line 1792 of yacc.c  */
#line 617 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (6)].tok).start;
	    (yyval.tok).end = (yyvsp[(6) - (6)].tok).end;
	    delete_sym(&(yyvsp[(2) - (6)].tok));
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 72:
/* Line 1792 of yacc.c  */
#line 624 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    delete_sym(&(yyvsp[(2) - (2)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 73:
/* Line 1792 of yacc.c  */
#line 633 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 74:
/* Line 1792 of yacc.c  */
#line 638 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 75:
/* Line 1792 of yacc.c  */
#line 646 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 76:
/* Line 1792 of yacc.c  */
#line 651 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 77:
/* Line 1792 of yacc.c  */
#line 659 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    delete_sym(&(yyvsp[(1) - (1)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 78:
/* Line 1792 of yacc.c  */
#line 665 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    delete_sym(&(yyvsp[(1) - (3)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 79:
/* Line 1792 of yacc.c  */
#line 674 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (1)].tok).symbol;
#ifndef LMAIN
	    if ((yyvsp[(1) - (1)].tok).symbol->type < MAX_LS &&
#if YYDEBUG && ! defined(SYUNION)
		(yyvsp[(1) - (1)].tok).symbol->v_glist == 0
#else
		(yyvsp[(1) - (1)].tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immVarRemove((yyvsp[(1) - (1)].tok).start, (yyvsp[(1) - (1)].tok).end, (yyvsp[(1) - (1)].tok).symbol);
	    }
#endif	/* LMAIN */
	}
    break;

  case 80:
/* Line 1792 of yacc.c  */
#line 690 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(2) - (2)].tok).symbol;
#ifndef LMAIN
	    if ((yyvsp[(2) - (2)].tok).symbol->type < MAX_LS &&
#if YYDEBUG && ! defined(SYUNION)
		(yyvsp[(2) - (2)].tok).symbol->v_glist == 0
#else
		(yyvsp[(2) - (2)].tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immVarRemove((yyvsp[(2) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(2) - (2)].tok).symbol);
	    }
#endif	/* LMAIN */
	}
    break;

  case 81:
/* Line 1792 of yacc.c  */
#line 709 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (1)].tok).symbol;
	}
    break;

  case 82:
/* Line 1792 of yacc.c  */
#line 714 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = (yyvsp[(1) - (1)].tok).inds;
	    (yyval.tok).inde = (yyvsp[(1) - (1)].tok).inde;
	    (yyval.tok).symbol = (yyvsp[(1) - (1)].tok).symbol;
	}
    break;

  case 83:
/* Line 1792 of yacc.c  */
#line 721 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(2) - (3)].tok).symbol;
	}
    break;

  case 84:
/* Line 1792 of yacc.c  */
#line 726 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(2) - (4)].tok).symbol;
	    yyclearin; yyerrok;
	}
    break;

  case 85:
/* Line 1792 of yacc.c  */
#line 732 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (3)].tok).symbol;
	}
    break;

  case 86:
/* Line 1792 of yacc.c  */
#line 737 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (4)].tok).symbol;
	    yyclearin; yyerrok;
	}
    break;

  case 87:
/* Line 1792 of yacc.c  */
#line 743 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (4)].tok).symbol;
	}
    break;

  case 88:
/* Line 1792 of yacc.c  */
#line 748 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (5)].tok).symbol;
	    yyclearin; yyerrok;
	}
    break;

  case 89:
/* Line 1792 of yacc.c  */
#line 754 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (2)].tok).symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif	/* LMAIN */
	}
    break;

  case 90:
/* Line 1792 of yacc.c  */
#line 762 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (3)].tok).symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif	/* LMAIN */
	    yyclearin; yyerrok;
	}
    break;

  case 91:
/* Line 1792 of yacc.c  */
#line 771 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (3)].tok).symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif	/* LMAIN */
	}
    break;

  case 92:
/* Line 1792 of yacc.c  */
#line 779 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (4)].tok).symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif	/* LMAIN */
	    yyclearin; yyerrok;
	}
    break;

  case 93:
/* Line 1792 of yacc.c  */
#line 788 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (3)].tok).symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif	/* LMAIN */
	}
    break;

  case 94:
/* Line 1792 of yacc.c  */
#line 796 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (4)].tok).symbol;
#ifndef LMAIN
	    lexflag &= ~C_PARA;			/* end of overloading imm as parameter */
#endif	/* LMAIN */
	    yyclearin; yyerrok;
	}
    break;

  case 95:
/* Line 1792 of yacc.c  */
#line 808 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (2)].tok).symbol;
#ifndef LMAIN
	    clearParaList(1);			/* imm vars are temporarily overloaded */
#endif	/* LMAIN */
	}
    break;

  case 96:
/* Line 1792 of yacc.c  */
#line 819 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 97:
/* Line 1792 of yacc.c  */
#line 824 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 98:
/* Line 1792 of yacc.c  */
#line 829 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 99:
/* Line 1792 of yacc.c  */
#line 834 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 100:
/* Line 1792 of yacc.c  */
#line 842 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 101:
/* Line 1792 of yacc.c  */
#line 847 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 102:
/* Line 1792 of yacc.c  */
#line 855 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 103:
/* Line 1792 of yacc.c  */
#line 860 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 104:
/* Line 1792 of yacc.c  */
#line 868 "gram.y"
    {			/* K&R C parameter list without types */
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 105:
/* Line 1792 of yacc.c  */
#line 873 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 106:
/* Line 1792 of yacc.c  */
#line 881 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 107:
/* Line 1792 of yacc.c  */
#line 886 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 108:
/* Line 1792 of yacc.c  */
#line 894 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    delete_sym(&(yyvsp[(2) - (2)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 109:
/* Line 1792 of yacc.c  */
#line 900 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 110:
/* Line 1792 of yacc.c  */
#line 908 "gram.y"
    {				/* K&R C parameter list without types */
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    delete_sym(&(yyvsp[(1) - (1)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 111:
/* Line 1792 of yacc.c  */
#line 914 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    delete_sym(&(yyvsp[(3) - (3)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 112:
/* Line 1792 of yacc.c  */
#line 923 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 113:
/* Line 1792 of yacc.c  */
#line 928 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 114:
/* Line 1792 of yacc.c  */
#line 933 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 115:
/* Line 1792 of yacc.c  */
#line 939 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 116:
/* Line 1792 of yacc.c  */
#line 944 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 117:
/* Line 1792 of yacc.c  */
#line 953 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 118:
/* Line 1792 of yacc.c  */
#line 958 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 119:
/* Line 1792 of yacc.c  */
#line 966 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 120:
/* Line 1792 of yacc.c  */
#line 971 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 121:
/* Line 1792 of yacc.c  */
#line 979 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 122:
/* Line 1792 of yacc.c  */
#line 984 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 123:
/* Line 1792 of yacc.c  */
#line 989 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 124:
/* Line 1792 of yacc.c  */
#line 997 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 125:
/* Line 1792 of yacc.c  */
#line 1002 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 126:
/* Line 1792 of yacc.c  */
#line 1008 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 127:
/* Line 1792 of yacc.c  */
#line 1013 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 128:
/* Line 1792 of yacc.c  */
#line 1019 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 129:
/* Line 1792 of yacc.c  */
#line 1024 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 130:
/* Line 1792 of yacc.c  */
#line 1030 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 131:
/* Line 1792 of yacc.c  */
#line 1035 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 132:
/* Line 1792 of yacc.c  */
#line 1041 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 133:
/* Line 1792 of yacc.c  */
#line 1046 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 134:
/* Line 1792 of yacc.c  */
#line 1052 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 135:
/* Line 1792 of yacc.c  */
#line 1057 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 136:
/* Line 1792 of yacc.c  */
#line 1063 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 137:
/* Line 1792 of yacc.c  */
#line 1068 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 138:
/* Line 1792 of yacc.c  */
#line 1074 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 139:
/* Line 1792 of yacc.c  */
#line 1079 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 140:
/* Line 1792 of yacc.c  */
#line 1085 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 141:
/* Line 1792 of yacc.c  */
#line 1090 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 142:
/* Line 1792 of yacc.c  */
#line 1099 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 143:
/* Line 1792 of yacc.c  */
#line 1104 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 144:
/* Line 1792 of yacc.c  */
#line 1109 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 145:
/* Line 1792 of yacc.c  */
#line 1114 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 146:
/* Line 1792 of yacc.c  */
#line 1119 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 147:
/* Line 1792 of yacc.c  */
#line 1124 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 148:
/* Line 1792 of yacc.c  */
#line 1132 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    delete_sym(&(yyvsp[(1) - (3)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 149:
/* Line 1792 of yacc.c  */
#line 1138 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 150:
/* Line 1792 of yacc.c  */
#line 1143 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 151:
/* Line 1792 of yacc.c  */
#line 1151 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 152:
/* Line 1792 of yacc.c  */
#line 1156 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 153:
/* Line 1792 of yacc.c  */
#line 1161 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 154:
/* Line 1792 of yacc.c  */
#line 1170 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 155:
/* Line 1792 of yacc.c  */
#line 1175 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 156:
/* Line 1792 of yacc.c  */
#line 1181 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 157:
/* Line 1792 of yacc.c  */
#line 1186 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 158:
/* Line 1792 of yacc.c  */
#line 1192 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 159:
/* Line 1792 of yacc.c  */
#line 1197 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 160:
/* Line 1792 of yacc.c  */
#line 1203 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 161:
/* Line 1792 of yacc.c  */
#line 1208 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 162:
/* Line 1792 of yacc.c  */
#line 1217 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 163:
/* Line 1792 of yacc.c  */
#line 1222 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 164:
/* Line 1792 of yacc.c  */
#line 1230 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 165:
/* Line 1792 of yacc.c  */
#line 1235 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (7)].tok).start;
	    (yyval.tok).end = (yyvsp[(7) - (7)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 166:
/* Line 1792 of yacc.c  */
#line 1240 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 167:
/* Line 1792 of yacc.c  */
#line 1248 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 168:
/* Line 1792 of yacc.c  */
#line 1253 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (7)].tok).start;
	    (yyval.tok).end = (yyvsp[(7) - (7)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 169:
/* Line 1792 of yacc.c  */
#line 1258 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (6)].tok).start;
	    (yyval.tok).end = (yyvsp[(6) - (6)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 170:
/* Line 1792 of yacc.c  */
#line 1263 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (7)].tok).start;
	    (yyval.tok).end = (yyvsp[(7) - (7)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 171:
/* Line 1792 of yacc.c  */
#line 1268 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (7)].tok).start;
	    (yyval.tok).end = (yyvsp[(7) - (7)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 172:
/* Line 1792 of yacc.c  */
#line 1273 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (8)].tok).start;
	    (yyval.tok).end = (yyvsp[(8) - (8)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 173:
/* Line 1792 of yacc.c  */
#line 1278 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (7)].tok).start;
	    (yyval.tok).end = (yyvsp[(7) - (7)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 174:
/* Line 1792 of yacc.c  */
#line 1283 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (8)].tok).start;
	    (yyval.tok).end = (yyvsp[(8) - (8)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 175:
/* Line 1792 of yacc.c  */
#line 1288 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (8)].tok).start;
	    (yyval.tok).end = (yyvsp[(8) - (8)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 176:
/* Line 1792 of yacc.c  */
#line 1293 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (9)].tok).start;
	    (yyval.tok).end = (yyvsp[(9) - (9)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 177:
/* Line 1792 of yacc.c  */
#line 1301 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    delete_sym(&(yyvsp[(2) - (3)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 178:
/* Line 1792 of yacc.c  */
#line 1307 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 179:
/* Line 1792 of yacc.c  */
#line 1312 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 180:
/* Line 1792 of yacc.c  */
#line 1317 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 181:
/* Line 1792 of yacc.c  */
#line 1322 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 182:
/* Line 1792 of yacc.c  */
#line 1327 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 183:
/* Line 1792 of yacc.c  */
#line 1336 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 184:
/* Line 1792 of yacc.c  */
#line 1341 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 185:
/* Line 1792 of yacc.c  */
#line 1349 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 186:
/* Line 1792 of yacc.c  */
#line 1355 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 187:
/* Line 1792 of yacc.c  */
#line 1361 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).inds = (yyvsp[(1) - (3)].tok).inds;
	    (yyval.tok).inde = (yyvsp[(1) - (3)].tok).inde;
	    (yyval.tok).symbol = (yyvsp[(1) - (3)].tok).symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "assignment_expression: imm_lvalue <%u> assignment_expression %u (%u) %u <%u> %s\n",
	    	(yyvsp[(2) - (3)].tok).inds, (yyvsp[(1) - (3)].tok).start, (yyvsp[(2) - (3)].tok).start, (yyvsp[(3) - (3)].tok).end, (yyvsp[(2) - (3)].tok).inds, (yyvsp[(1) - (3)].tok).symbol->name);	/* NOTE: assignment_operator.inds is ppi */
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		(yyvsp[(1) - (3)].tok).symbol->v_glist == 0
#else
		(yyvsp[(1) - (3)].tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immAssignFound((yyvsp[(1) - (3)].tok).start, (yyvsp[(2) - (3)].tok).start, (yyvsp[(3) - (3)].tok).end, (yyvsp[(1) - (3)].tok).symbol, (yyvsp[(2) - (3)].tok).inds);
	    }
#endif	/* LMAIN */
	}
    break;

  case 188:
/* Line 1792 of yacc.c  */
#line 1386 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 0;		/* = */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 189:
/* Line 1792 of yacc.c  */
#line 1392 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 2;		/* *= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 190:
/* Line 1792 of yacc.c  */
#line 1398 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 3;		/* /= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 191:
/* Line 1792 of yacc.c  */
#line 1404 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 4;		/* %= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 192:
/* Line 1792 of yacc.c  */
#line 1410 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 5;		/* += */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 193:
/* Line 1792 of yacc.c  */
#line 1416 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 6;		/* -= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 194:
/* Line 1792 of yacc.c  */
#line 1422 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 7;		/* &= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 195:
/* Line 1792 of yacc.c  */
#line 1428 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 8;		/* ^= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 196:
/* Line 1792 of yacc.c  */
#line 1434 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 9;		/* |= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 197:
/* Line 1792 of yacc.c  */
#line 1440 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 10;		/* <<= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 198:
/* Line 1792 of yacc.c  */
#line 1446 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 11;		/* >>= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 199:
/* Line 1792 of yacc.c  */
#line 1455 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 200:
/* Line 1792 of yacc.c  */
#line 1460 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 201:
/* Line 1792 of yacc.c  */
#line 1466 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
#ifndef LMAIN
	    if (iC_Pflag) {
		Symbol * sp = (yyvsp[(4) - (4)].tok).symbol;
		warning("ISO C forbids omitting the middle term of a ?: expression", sp ? sp->name : NULL);
	    }
	    (yyval.tok).symbol = NULL;
#endif	/* LMAIN */
	}
    break;

  case 202:
/* Line 1792 of yacc.c  */
#line 1480 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 203:
/* Line 1792 of yacc.c  */
#line 1488 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 204:
/* Line 1792 of yacc.c  */
#line 1493 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 205:
/* Line 1792 of yacc.c  */
#line 1501 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 206:
/* Line 1792 of yacc.c  */
#line 1506 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 207:
/* Line 1792 of yacc.c  */
#line 1514 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 208:
/* Line 1792 of yacc.c  */
#line 1519 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 209:
/* Line 1792 of yacc.c  */
#line 1527 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 210:
/* Line 1792 of yacc.c  */
#line 1532 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 211:
/* Line 1792 of yacc.c  */
#line 1540 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 212:
/* Line 1792 of yacc.c  */
#line 1545 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 213:
/* Line 1792 of yacc.c  */
#line 1553 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 214:
/* Line 1792 of yacc.c  */
#line 1558 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 215:
/* Line 1792 of yacc.c  */
#line 1563 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 216:
/* Line 1792 of yacc.c  */
#line 1571 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 217:
/* Line 1792 of yacc.c  */
#line 1576 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 218:
/* Line 1792 of yacc.c  */
#line 1581 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 219:
/* Line 1792 of yacc.c  */
#line 1586 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 220:
/* Line 1792 of yacc.c  */
#line 1591 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 221:
/* Line 1792 of yacc.c  */
#line 1599 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 222:
/* Line 1792 of yacc.c  */
#line 1604 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 223:
/* Line 1792 of yacc.c  */
#line 1609 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 224:
/* Line 1792 of yacc.c  */
#line 1617 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 225:
/* Line 1792 of yacc.c  */
#line 1622 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 226:
/* Line 1792 of yacc.c  */
#line 1627 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 227:
/* Line 1792 of yacc.c  */
#line 1635 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 228:
/* Line 1792 of yacc.c  */
#line 1640 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 229:
/* Line 1792 of yacc.c  */
#line 1645 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 230:
/* Line 1792 of yacc.c  */
#line 1650 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 231:
/* Line 1792 of yacc.c  */
#line 1658 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 232:
/* Line 1792 of yacc.c  */
#line 1663 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 233:
/* Line 1792 of yacc.c  */
#line 1668 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 234:
/* Line 1792 of yacc.c  */
#line 1673 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 235:
/* Line 1792 of yacc.c  */
#line 1682 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 236:
/* Line 1792 of yacc.c  */
#line 1687 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 237:
/* Line 1792 of yacc.c  */
#line 1692 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 238:
/* Line 1792 of yacc.c  */
#line 1697 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 239:
/* Line 1792 of yacc.c  */
#line 1702 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 240:
/* Line 1792 of yacc.c  */
#line 1707 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 241:
/* Line 1792 of yacc.c  */
#line 1712 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 242:
/* Line 1792 of yacc.c  */
#line 1721 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = (yyvsp[(1) - (1)].tok).inds;
	    (yyval.tok).inde = (yyvsp[(1) - (1)].tok).inde;
	    (yyval.tok).symbol = (yyvsp[(1) - (1)].tok).symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_unary_expression: imm_postfix_expression %u %u %s\n",
	    	(yyval.tok).start, (yyval.tok).end, (yyval.tok).symbol->name);
#endif
#endif	/* LMAIN */
	}
    break;

  case 243:
/* Line 1792 of yacc.c  */
#line 1734 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).inds = (yyvsp[(2) - (2)].tok).inds;
	    (yyval.tok).inde = (yyvsp[(2) - (2)].tok).inde;
	    (yyval.tok).symbol = (yyvsp[(2) - (2)].tok).symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_unary_expression: ++ imm_unary_expression %u (%u) %u <12> %s\n",
	    	(yyvsp[(2) - (2)].tok).start, (yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(2) - (2)].tok).symbol->name);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		(yyvsp[(2) - (2)].tok).symbol->v_glist == 0
#else
		(yyvsp[(2) - (2)].tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immAssignFound((yyvsp[(2) - (2)].tok).start, (yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(2) - (2)].tok).symbol, 12);	/* ++x */
	    }
#endif	/* LMAIN */
	}
    break;

  case 244:
/* Line 1792 of yacc.c  */
#line 1756 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).inds = (yyvsp[(2) - (2)].tok).inds;
	    (yyval.tok).inde = (yyvsp[(2) - (2)].tok).inde;
	    (yyval.tok).symbol = (yyvsp[(2) - (2)].tok).symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_unary_expression: -- imm_unary_expression %u (%u) %u <13> %s\n",
	    	(yyvsp[(2) - (2)].tok).start, (yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(2) - (2)].tok).symbol->name);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		(yyvsp[(2) - (2)].tok).symbol->v_glist == 0
#else
		(yyvsp[(2) - (2)].tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immAssignFound((yyvsp[(2) - (2)].tok).start, (yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(2) - (2)].tok).symbol, 13);	/* --x */
	    }
#endif	/* LMAIN */
	}
    break;

  case 245:
/* Line 1792 of yacc.c  */
#line 1778 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 246:
/* Line 1792 of yacc.c  */
#line 1783 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(2) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(2) - (2)].tok).symbol;
#ifndef LMAIN
	    functionUse[0].c_cnt |= F_SIZE;	/* imm sizeof macro required */
	    (yyval.tok).inds = 0;			/* mark 0 in imm_lvalue */
	    (yyval.tok).inde = (yyvsp[(1) - (2)].tok).start;			/* used to set earlyop to blank out sizeof */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_unary_expression: sizeof imm_unary_expression %u (%u) %u %s\n",
	    	(yyvsp[(2) - (2)].tok).start, (yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(2) - (2)].tok).symbol->name);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		(yyval.tok).symbol->v_glist == 0
#else
		(yyval.tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immVarFound((yyval.tok).start, (yyval.tok).end, (yyval.tok).inds, (yyval.tok).inde, NULL);	/* adjust pEnd, set inds 0 inde early */
	    }
#endif	/* LMAIN */
	}
    break;

  case 247:
/* Line 1792 of yacc.c  */
#line 1809 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 248:
/* Line 1792 of yacc.c  */
#line 1814 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 249:
/* Line 1792 of yacc.c  */
#line 1819 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 250:
/* Line 1792 of yacc.c  */
#line 1824 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 251:
/* Line 1792 of yacc.c  */
#line 1829 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 252:
/* Line 1792 of yacc.c  */
#line 1834 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 253:
/* Line 1792 of yacc.c  */
#line 1842 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 254:
/* Line 1792 of yacc.c  */
#line 1847 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 255:
/* Line 1792 of yacc.c  */
#line 1852 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 256:
/* Line 1792 of yacc.c  */
#line 1858 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 257:
/* Line 1792 of yacc.c  */
#line 1863 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 258:
/* Line 1792 of yacc.c  */
#line 1869 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 259:
/* Line 1792 of yacc.c  */
#line 1874 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 260:
/* Line 1792 of yacc.c  */
#line 1880 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    delete_sym(&(yyvsp[(3) - (3)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 261:
/* Line 1792 of yacc.c  */
#line 1886 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    delete_sym(&(yyvsp[(3) - (3)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 262:
/* Line 1792 of yacc.c  */
#line 1892 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 263:
/* Line 1792 of yacc.c  */
#line 1897 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 264:
/* Line 1792 of yacc.c  */
#line 1905 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = (yyvsp[(1) - (1)].tok).inds;
	    (yyval.tok).inde = (yyvsp[(1) - (1)].tok).inde;
	    (yyval.tok).symbol = (yyvsp[(1) - (1)].tok).symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_postfix_expression: imm_lvalue %u %u %s\n",
	    	(yyval.tok).start, (yyval.tok).end, (yyval.tok).symbol->name);
#endif
#endif	/* LMAIN */
	}
    break;

  case 265:
/* Line 1792 of yacc.c  */
#line 1918 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).inds = (yyvsp[(1) - (2)].tok).inds;
	    (yyval.tok).inde = (yyvsp[(1) - (2)].tok).inde;
	    (yyval.tok).symbol = (yyvsp[(1) - (2)].tok).symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_postfix_expression: imm_postfix_expression ++ %u (%u) %u <14> %s\n",
	    	(yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(1) - (2)].tok).symbol->name);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		(yyvsp[(1) - (2)].tok).symbol->v_glist == 0
#else
		(yyvsp[(1) - (2)].tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immAssignFound((yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(1) - (2)].tok).symbol, 14);	/* x++ */
	    }
#endif	/* LMAIN */
	}
    break;

  case 266:
/* Line 1792 of yacc.c  */
#line 1940 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).inds = (yyvsp[(1) - (2)].tok).inds;
	    (yyval.tok).inde = (yyvsp[(1) - (2)].tok).inde;
	    (yyval.tok).symbol = (yyvsp[(1) - (2)].tok).symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_postfix_expression: imm_postfix_expression -- %u (%u) %u <15> %s\n",
	    	(yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(1) - (2)].tok).symbol->name);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		(yyvsp[(1) - (2)].tok).symbol->v_glist == 0
#else
		(yyvsp[(1) - (2)].tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immAssignFound((yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(1) - (2)].tok).symbol, 15);	/* x-- */
	    }
#endif	/* LMAIN */
	}
    break;

  case 267:
/* Line 1792 of yacc.c  */
#line 1965 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = (yyvsp[(1) - (1)].tok).inds;
	    (yyval.tok).inde = (yyvsp[(1) - (1)].tok).inde;
	    (yyval.tok).symbol = (yyvsp[(1) - (1)].tok).symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_lvalue: imm_identifier %u %u %s\n",
		(yyval.tok).start, (yyval.tok).end, (yyval.tok).symbol->name);
#endif
#endif	/* LMAIN */
	}
    break;

  case 268:
/* Line 1792 of yacc.c  */
#line 1979 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (4)].tok).symbol;
#ifndef LMAIN
	    (yyval.tok).inds = (yyvsp[(2) - (4)].tok).start;			/* mark '[' in imm_lvalue */
	    (yyval.tok).inde = (yyvsp[(4) - (4)].tok).start;			/* mark ']' in imm_lvalue */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_lvalue: imm_array_identifier %u %u %s[%u %u]\n",
		(yyval.tok).start, (yyval.tok).end, (yyval.tok).symbol->name, (yyval.tok).inds, (yyval.tok).inde);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		(yyval.tok).symbol->v_glist == 0
#else
		(yyval.tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immVarFound((yyval.tok).start, (yyval.tok).end, (yyval.tok).inds, (yyval.tok).inde, NULL);	/* adjust pEnd, set inds inde */
	    }
#endif	/* LMAIN */
	}
    break;

  case 269:
/* Line 1792 of yacc.c  */
#line 2001 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 270:
/* Line 1792 of yacc.c  */
#line 2010 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    delete_sym(&(yyvsp[(1) - (1)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 271:
/* Line 1792 of yacc.c  */
#line 2016 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 272:
/* Line 1792 of yacc.c  */
#line 2021 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 273:
/* Line 1792 of yacc.c  */
#line 2026 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 274:
/* Line 1792 of yacc.c  */
#line 2031 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 275:
/* Line 1792 of yacc.c  */
#line 2037 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 276:
/* Line 1792 of yacc.c  */
#line 2042 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 277:
/* Line 1792 of yacc.c  */
#line 2051 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 278:
/* Line 1792 of yacc.c  */
#line 2056 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 279:
/* Line 1792 of yacc.c  */
#line 2064 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 280:
/* Line 1792 of yacc.c  */
#line 2069 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 281:
/* Line 1792 of yacc.c  */
#line 2077 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = LARGE;
	    (yyval.tok).inde = LARGE;
	    (yyval.tok).symbol = (yyvsp[(1) - (1)].tok).symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_identifier: IMM_IDENTIFIER %u %u %s\n",
	    	(yyval.tok).start, (yyval.tok).end, (yyval.tok).symbol->name);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		(yyval.tok).symbol->v_glist == 0
#else
		(yyval.tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immVarFound((yyval.tok).start, (yyval.tok).end, (yyval.tok).inds, (yyval.tok).inde, (yyval.tok).symbol);
	    }
#endif	/* LMAIN */
	}
    break;

  case 282:
/* Line 1792 of yacc.c  */
#line 2099 "gram.y"
    {
	    /* stops this being a primary_expression which would lead to C assignment */
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).inds = (yyvsp[(2) - (3)].tok).inds;
	    (yyval.tok).inde = (yyvsp[(2) - (3)].tok).inde;
	    (yyval.tok).symbol = (yyvsp[(2) - (3)].tok).symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_identifier: (imm_identifier) %u %u %s\n",
	    	(yyval.tok).start, (yyval.tok).end, (yyval.tok).symbol->name);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		(yyval.tok).symbol->v_glist == 0
#else
		(yyval.tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immVarFound((yyval.tok).start, (yyval.tok).end, (yyval.tok).inds, (yyval.tok).inde, NULL);	/* moves pStart and pEnd without changing vStart vEnd */
	    }
#endif	/* LMAIN */
	}
    break;

  case 283:
/* Line 1792 of yacc.c  */
#line 2125 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = LARGE;			/* will be set in imm_lvalue: */
	    (yyval.tok).inde = LARGE;			/*   | imm_array_identifier [ expression ] */
	    (yyval.tok).symbol = (yyvsp[(1) - (1)].tok).symbol;
#ifndef LMAIN
	    functionUse[0].c_cnt |= F_ARRAY;	/* immC array macro required */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_array_identifier: IMM_ARRAY_IDENTIFIER %u %u %s\n",
	    	(yyval.tok).start, (yyval.tok).end, (yyval.tok).symbol->name);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		(yyval.tok).symbol->v_glist == 0
#else
		(yyval.tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immVarFound((yyval.tok).start, (yyval.tok).end, (yyval.tok).inds, (yyval.tok).inde, (yyval.tok).symbol);
	    }
#endif	/* LMAIN */
	}
    break;

  case 284:
/* Line 1792 of yacc.c  */
#line 2148 "gram.y"
    {
	    /* stops this being a primary_expression which would lead to C assignment */
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).inds = (yyvsp[(2) - (3)].tok).inds;
	    (yyval.tok).inde = (yyvsp[(2) - (3)].tok).inde;
	    (yyval.tok).symbol = (yyvsp[(2) - (3)].tok).symbol;
#ifndef LMAIN
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "imm_array_identifier: (imm_array_identifier) %u %u %s\n",
	    	(yyval.tok).start, (yyval.tok).end, (yyval.tok).symbol->name);
#endif
	    if (
#if YYDEBUG && ! defined(SYUNION)
		(yyval.tok).symbol->v_glist == 0
#else
		(yyval.tok).symbol->v_cnt <= 2		/* v_cnt instead of v_glist for SYUNION */
#endif
	    ) {
		immVarFound((yyval.tok).start, (yyval.tok).end, (yyval.tok).inds, (yyval.tok).inde, NULL);	/* moves pStart and pEnd without changing vStart vEnd */
	    }
#endif	/* LMAIN */
	}
    break;


/* Line 1792 of yacc.c  */
#line 5469 "gram.tab.c"
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}


/* Line 2055 of yacc.c  */
#line 2178 "gram.y"


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
immVarFound(unsigned int start, unsigned int end, unsigned int inds, unsigned int inde, Symbol* sp)
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
	p->sp     = sp;
	if (sp->ftype != ARITH && sp->ftype != GATE &&
	    (sp->ftype != UDFA || (sp->type != ARNC && sp->type != LOGC))
	    && sp->type != ERR) {
	    ierror("C-statement tries to access an imm type not bit or int:", sp->name);
	    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
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
	fprintf(iC_outFP, "immVarFound: %u (%u %u) %u [%u %u] %s\n",
	    p->pStart, p->vStart, p->vEnd, p->pEnd, p->inds, p->inde, p->sp->name);
    }
#endif
    if (lep > &lineEntryArray[udfCount-2]) {	/* allow for 2 guard entries at end */
	newArray = (LineEntry*)realloc(lineEntryArray,
	    (udfCount + LEAI) * sizeof(LineEntry));
	assert(newArray);
	memset(&newArray[udfCount], '\0', LEAI * sizeof(LineEntry));
	udfCount += LEAI;			/* increase the size of the array */
	lep += newArray - lineEntryArray;	/* lep needs adjusting - cannot realloc lineEntryArray directly */
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
	if (p->pStart == start) {		/* start position of imm variable assigned to */
	    p->equOp  = operator;		/* position of operator marks an assignment expression */
	    p->pEnd   = end;			/* end position of expression assigned from */
	    p->ppi    = ppi;			/* assignment operator or pre/post-inc/dec */
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
    execerror("immAssignFound: Symbol not found ???", sp->name, __FILE__, __LINE__);
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

void
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

    bytePos = 0;
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
	if (bytePos >= start) {
	    pushEndStack(end << 1);		/* push previous end (with 0 marker for end) */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "\n= %u(%u [%u %u] %u %u)%u <%u> %s\n",
		    p->pStart, p->vStart, p->inds, p->inde, p->equOp==LARGE?0:p->equOp, p->vEnd, p->pEnd, p->ppi, p->sp->name);
	    }
#endif
	    vstart = p->vStart;			/* start of actual variable */
	    vend   = p->vEnd;			/* end of actual variable */
	    inds   = p->inds;			/* array index start */
	    inde   = p->inde;			/* array index end */
	    equop  = inds ? p->equOp : LARGE;	/* operator in this entry unless sizeof */
	    end    = p->pEnd;			/* end of this entry */
	    ppi    = p->ppi;			/* pre/post-inc/dec character value */
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
		fprintf(iC_outFP, "strt bytePos = %u [%u %u] earlyop = %u, equop = %u, ppi = %u, sp =>%s\n",
		    bytePos, inds, inde, earlyop, equop, ppi, sp->name);
		obp += snprintf(obp, OUTBUFEND-obp, "%s", cMacro[ml+ftypa]);
		if (obp > OUTBUFEND) obp = OUTBUFEND;	/* catch buffer overflow */
		changeFlag++;
	    }
#endif
	    fprintf(oFP, "%s", cMacro[ml+ftypa]);	/* entry found - output cMacro start */
	    assert(start <= vstart);
	    assert(vstart < vend);
	    assert(vend <= end);
	    if (start < vstart) {
		assert(c == '(');		/* parenthesised imm_identifier or imm_array_identifier */
		endop = vstart;			/* supress output of this paranthesis */
		pFlag = 0;
		parend = vend;			/* marks closing parenthesis which is also supressed */
		parcnt = 1;			/* could be more than 1 ( */
		parnxt = bytePos + 1;		/* allows counting of ( and spaces up to vstart */
	    }
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
		if (obp > OUTBUFEND) obp = OUTBUFEND;	/* catch buffer overflow */
		changeFlag++;
	    }
#endif
	    fprintf(oFP, "%d", sNr);		/* output Symbol pointer offset instead of variable */
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
	    parend = endop = bytePos+1;		/* supress output of closing parantheses and spaces */
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
#endif	/* LMAIN */

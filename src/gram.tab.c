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
"@(#)$Id: gram.tab.c 1.40 $";
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
     BIT_CONSTANT = 262,
     STRING_LITERAL = 263,
     SIZEOF = 264,
     PTR_OP = 265,
     INC_OP = 266,
     DEC_OP = 267,
     LEFT_SH = 268,
     RIGHT_SH = 269,
     LE_OP = 270,
     GE_OP = 271,
     EQ_OP = 272,
     NE_OP = 273,
     AND_OP = 274,
     OR_OP = 275,
     MUL_ASSIGN = 276,
     DIV_ASSIGN = 277,
     MOD_ASSIGN = 278,
     ADD_ASSIGN = 279,
     SUB_ASSIGN = 280,
     LEFT_ASSIGN = 281,
     RIGHT_ASSIGN = 282,
     AND_ASSIGN = 283,
     XOR_ASSIGN = 284,
     OR_ASSIGN = 285,
     TYPE_NAME = 286,
     TYPEDEF = 287,
     TYPEOF = 288,
     EXTERN = 289,
     STATIC = 290,
     AUTO = 291,
     REGISTER = 292,
     CHAR = 293,
     SHORT = 294,
     INT = 295,
     LONG = 296,
     SIGNED = 297,
     UNSIGNED = 298,
     FLOAT = 299,
     DOUBLE = 300,
     CONST = 301,
     VOLATILE = 302,
     VOID = 303,
     STRUCT = 304,
     UNION = 305,
     ENUM = 306,
     ELIPSIS = 307,
     CASE = 308,
     DEFAULT = 309,
     IF = 310,
     ELSE = 311,
     SWITCH = 312,
     WHILE = 313,
     DO = 314,
     FOR = 315,
     GOTO = 316,
     CONTINUE = 317,
     BREAK = 318,
     RETURN = 319
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
#line 276 "gram.tab.c"
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
#line 304 "gram.tab.c"

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
#define YYLAST   1819

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  89
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  76
/* YYNRULES -- Number of rules.  */
#define YYNRULES  285
/* YYNRULES -- Number of states.  */
#define YYNSTATES  494

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   319

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    80,     2,     2,     2,    87,    75,     2,
      67,    68,    76,    77,    74,    78,    88,    86,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    72,    73,
      84,    71,    85,    81,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    66,     2,    69,    83,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    65,    82,    70,    79,     2,     2,     2,
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
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64
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
     941,   943,   945,   947,   949,   953,   958,   962,   967,   969,
     973,   975,   978,   980,   984,   986
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      90,     0,    -1,    -1,    90,    91,    -1,    92,    -1,    95,
      -1,   115,    94,    -1,    -1,    97,   115,    93,    94,    -1,
     133,    -1,    96,   133,    -1,    97,    73,    -1,    97,   105,
      73,    -1,    97,     1,    73,    -1,    95,    -1,    96,    95,
      -1,   108,    -1,    98,   108,    -1,    36,    -1,    35,    -1,
      34,    -1,    32,    -1,    48,    -1,    37,    -1,    38,    -1,
      39,    -1,    40,    -1,    41,    -1,    44,    -1,    45,    -1,
      42,    -1,    43,    -1,   101,    -1,   111,    -1,    31,    -1,
      33,    67,   154,    68,    -1,    33,    67,   154,     1,    68,
      -1,    33,    67,   127,    68,    -1,    33,    67,   127,     1,
      68,    -1,    46,    -1,    47,    -1,   102,    -1,   102,    65,
     104,    70,    -1,   102,    65,   104,     1,    70,    -1,   103,
      -1,   103,     3,    -1,   103,    31,    -1,   103,   163,    -1,
      49,    -1,    50,    -1,    -1,   104,   107,    -1,   106,    -1,
     105,    74,   106,    -1,   115,    -1,   115,    71,   125,    -1,
     108,    73,    -1,   108,   109,    73,    -1,   108,     1,    73,
      -1,    99,    -1,    99,   108,    -1,   100,    -1,   100,   108,
      -1,   110,    -1,   109,    74,   110,    -1,   115,    -1,    72,
     142,    -1,   115,    72,   142,    -1,    51,    65,   112,    70,
      -1,    51,    65,   112,     1,    70,    -1,    51,     3,    65,
     112,    70,    -1,    51,     3,    65,   112,     1,    70,    -1,
      51,     3,    -1,   113,    -1,   113,    74,    -1,   114,    -1,
     113,    74,   114,    -1,     3,    -1,     3,    71,   142,    -1,
     116,    -1,   118,   116,    -1,     3,    -1,   163,    -1,    67,
     115,    68,    -1,    67,   115,     1,    68,    -1,   116,    66,
      69,    -1,   116,    66,     1,    69,    -1,   116,    66,   142,
      69,    -1,   116,    66,   142,     1,    69,    -1,   117,    68,
      -1,   117,     1,    68,    -1,   117,   120,    68,    -1,   117,
     120,     1,    68,    -1,   117,   121,    68,    -1,   117,   121,
       1,    68,    -1,   116,    67,    -1,    76,    -1,    76,   119,
      -1,    76,   118,    -1,    76,   119,   118,    -1,   100,    -1,
     119,   100,    -1,   122,    -1,   122,    74,    52,    -1,   124,
      -1,   124,    74,    52,    -1,   123,    -1,   122,    74,   123,
      -1,   108,   115,    -1,   127,    -1,     3,    -1,   124,    74,
       3,    -1,   139,    -1,    65,   126,    70,    -1,    65,   126,
       1,    70,    -1,    65,   126,    74,    70,    -1,    65,   126,
       1,    74,    70,    -1,   125,    -1,   126,    74,   125,    -1,
     108,    -1,   108,   128,    -1,   118,    -1,   129,    -1,   118,
     129,    -1,    67,   128,    68,    -1,    67,   128,     1,    68,
      -1,    66,    69,    -1,    66,     1,    69,    -1,    66,   142,
      69,    -1,    66,   142,     1,    69,    -1,   129,    66,    69,
      -1,   129,    66,     1,    69,    -1,   129,    66,   142,    69,
      -1,   129,    66,   142,     1,    69,    -1,    67,    68,    -1,
      67,     1,    68,    -1,    67,   120,    68,    -1,    67,   120,
       1,    68,    -1,   129,    67,    68,    -1,   129,    67,     1,
      68,    -1,   129,    67,   120,    68,    -1,   129,    67,   120,
       1,    68,    -1,   131,    -1,   132,    -1,   133,    -1,   135,
      -1,   136,    -1,   137,    -1,     3,    72,   130,    -1,    53,
     142,    72,   130,    -1,    54,    72,   130,    -1,    73,    -1,
     138,    73,    -1,   138,     1,    73,    -1,    65,    70,    -1,
      65,     1,    70,    -1,    65,   134,    70,    -1,    65,   134,
       1,    70,    -1,    65,    96,    70,    -1,    65,    96,     1,
      70,    -1,    65,    96,   134,    70,    -1,    65,    96,   134,
       1,    70,    -1,   130,    -1,   134,   130,    -1,    55,    67,
     138,    68,   130,    -1,    55,    67,   138,    68,   130,    56,
     130,    -1,    57,    67,   138,    68,   130,    -1,    58,    67,
     138,    68,   130,    -1,    59,   130,    58,    67,   138,    68,
      73,    -1,    60,    67,    73,    73,    68,   130,    -1,    60,
      67,    73,    73,   138,    68,   130,    -1,    60,    67,    73,
     138,    73,    68,   130,    -1,    60,    67,    73,   138,    73,
     138,    68,   130,    -1,    60,    67,   138,    73,    73,    68,
     130,    -1,    60,    67,   138,    73,    73,   138,    68,   130,
      -1,    60,    67,   138,    73,   138,    73,    68,   130,    -1,
      60,    67,   138,    73,   138,    73,   138,    68,   130,    -1,
      61,     3,    73,    -1,    62,    73,    -1,    63,    73,    -1,
      64,    73,    -1,    64,   138,    73,    -1,    64,   138,     1,
      73,    -1,   139,    -1,   138,    74,   139,    -1,   141,    -1,
     154,   140,   139,    -1,   159,   140,   139,    -1,    71,    -1,
      21,    -1,    22,    -1,    23,    -1,    24,    -1,    25,    -1,
      28,    -1,    29,    -1,    30,    -1,    26,    -1,    27,    -1,
     143,    -1,   143,    81,   143,    72,   141,    -1,   143,    81,
      72,   141,    -1,   141,    -1,   144,    -1,   143,    20,   144,
      -1,   145,    -1,   144,    19,   145,    -1,   146,    -1,   145,
      82,   146,    -1,   147,    -1,   146,    83,   147,    -1,   148,
      -1,   147,    75,   148,    -1,   149,    -1,   148,    17,   149,
      -1,   148,    18,   149,    -1,   150,    -1,   149,    84,   150,
      -1,   149,    85,   150,    -1,   149,    15,   150,    -1,   149,
      16,   150,    -1,   151,    -1,   150,    13,   151,    -1,   150,
      14,   151,    -1,   152,    -1,   151,    77,   152,    -1,   151,
      78,   152,    -1,   153,    -1,   152,    76,   153,    -1,   152,
      86,   153,    -1,   152,    87,   153,    -1,   154,    -1,   155,
      -1,    67,   127,    68,   153,    -1,    67,   127,     1,    68,
     153,    -1,   157,    -1,    11,   154,    -1,    12,   154,    -1,
     156,   153,    -1,     9,   154,    -1,     9,    67,   127,    68,
      -1,     9,    67,   127,     1,    68,    -1,   158,    -1,    11,
     155,    -1,    12,   155,    -1,     9,   155,    -1,     9,   164,
      -1,    75,    -1,    76,    -1,    77,    -1,    78,    -1,    79,
      -1,    80,    -1,   160,    -1,   157,    66,   138,    69,    -1,
     157,    66,   138,     1,    69,    -1,   157,    67,    68,    -1,
     157,    67,     1,    68,    -1,   157,    67,   161,    68,    -1,
     157,    67,   161,     1,    68,    -1,   157,    88,     3,    -1,
     157,    10,     3,    -1,   157,    11,    -1,   157,    12,    -1,
     159,    -1,   158,    11,    -1,   158,    12,    -1,   163,    -1,
     164,    66,   138,    69,    -1,   164,    66,   138,     1,    69,
      -1,     3,    -1,     6,    -1,     7,    -1,   162,    -1,    67,
     138,    68,    -1,    67,   138,     1,    68,    -1,    67,   133,
      68,    -1,    67,   133,     1,    68,    -1,   139,    -1,   161,
      74,   139,    -1,     8,    -1,   162,     8,    -1,     4,    -1,
      67,   163,    68,    -1,     5,    -1,    67,   164,    68,    -1
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
     684,   697,   702,   709,   714,   720,   725,   731,   736,   742,
     750,   759,   767,   776,   784,   796,   807,   812,   817,   822,
     830,   835,   843,   848,   856,   861,   869,   874,   882,   888,
     896,   902,   911,   916,   921,   927,   932,   941,   946,   954,
     959,   967,   972,   977,   985,   990,   996,  1001,  1007,  1012,
    1018,  1023,  1029,  1034,  1040,  1045,  1051,  1056,  1062,  1067,
    1073,  1078,  1087,  1092,  1097,  1102,  1107,  1112,  1120,  1126,
    1131,  1139,  1144,  1149,  1158,  1163,  1169,  1174,  1180,  1185,
    1191,  1196,  1205,  1210,  1218,  1223,  1228,  1236,  1241,  1246,
    1251,  1256,  1261,  1266,  1271,  1276,  1281,  1289,  1295,  1300,
    1305,  1310,  1315,  1324,  1329,  1337,  1343,  1349,  1368,  1374,
    1380,  1386,  1392,  1398,  1404,  1410,  1416,  1422,  1428,  1437,
    1442,  1448,  1462,  1470,  1475,  1483,  1488,  1496,  1501,  1509,
    1514,  1522,  1527,  1535,  1540,  1545,  1553,  1558,  1563,  1568,
    1573,  1581,  1586,  1591,  1599,  1604,  1609,  1617,  1622,  1627,
    1632,  1640,  1645,  1650,  1655,  1664,  1669,  1674,  1679,  1684,
    1689,  1694,  1703,  1716,  1732,  1748,  1753,  1773,  1778,  1783,
    1788,  1793,  1798,  1806,  1811,  1816,  1822,  1827,  1833,  1838,
    1844,  1850,  1856,  1861,  1869,  1882,  1898,  1917,  1931,  1947,
    1956,  1962,  1967,  1979,  1984,  1989,  1995,  2000,  2009,  2014,
    2022,  2027,  2035,  2051,  2071,  2088
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "IDENTIFIER", "IMM_IDENTIFIER",
  "IMM_ARRAY_IDENTIFIER", "CONSTANT", "BIT_CONSTANT", "STRING_LITERAL",
  "SIZEOF", "PTR_OP", "INC_OP", "DEC_OP", "LEFT_SH", "RIGHT_SH", "LE_OP",
  "GE_OP", "EQ_OP", "NE_OP", "AND_OP", "OR_OP", "MUL_ASSIGN", "DIV_ASSIGN",
  "MOD_ASSIGN", "ADD_ASSIGN", "SUB_ASSIGN", "LEFT_ASSIGN", "RIGHT_ASSIGN",
  "AND_ASSIGN", "XOR_ASSIGN", "OR_ASSIGN", "TYPE_NAME", "TYPEDEF",
  "TYPEOF", "EXTERN", "STATIC", "AUTO", "REGISTER", "CHAR", "SHORT", "INT",
  "LONG", "SIGNED", "UNSIGNED", "FLOAT", "DOUBLE", "CONST", "VOLATILE",
  "VOID", "STRUCT", "UNION", "ENUM", "ELIPSIS", "CASE", "DEFAULT", "IF",
  "ELSE", "SWITCH", "WHILE", "DO", "FOR", "GOTO", "CONTINUE", "BREAK",
  "RETURN", "'{'", "'['", "'('", "')'", "']'", "'}'", "'='", "':'", "';'",
  "','", "'&'", "'*'", "'+'", "'-'", "'~'", "'!'", "'?'", "'|'", "'^'",
  "'<'", "'>'", "'/'", "'%'", "'.'", "$accept", "translation_unit",
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
     315,   316,   317,   318,   319,   123,    91,    40,    41,    93,
     125,    61,    58,    59,    44,    38,    42,    43,    45,   126,
      33,    63,   124,    94,    60,    62,    47,    37,    46
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    89,    90,    90,    91,    91,    92,    93,    92,    94,
      94,    95,    95,    95,    96,    96,    97,    97,    98,    98,
      98,    98,    99,    99,    99,    99,    99,    99,    99,    99,
      99,    99,    99,    99,    99,    99,    99,    99,    99,   100,
     100,   101,   101,   101,   102,   102,   102,   102,   103,   103,
     104,   104,   105,   105,   106,   106,   107,   107,   107,   108,
     108,   108,   108,   109,   109,   110,   110,   110,   111,   111,
     111,   111,   111,   112,   112,   113,   113,   114,   114,   115,
     115,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   117,   118,   118,   118,   118,
     119,   119,   120,   120,   121,   121,   122,   122,   123,   123,
     124,   124,   125,   125,   125,   125,   125,   126,   126,   127,
     127,   128,   128,   128,   129,   129,   129,   129,   129,   129,
     129,   129,   129,   129,   129,   129,   129,   129,   129,   129,
     129,   129,   130,   130,   130,   130,   130,   130,   131,   131,
     131,   132,   132,   132,   133,   133,   133,   133,   133,   133,
     133,   133,   134,   134,   135,   135,   135,   136,   136,   136,
     136,   136,   136,   136,   136,   136,   136,   137,   137,   137,
     137,   137,   137,   138,   138,   139,   139,   139,   140,   140,
     140,   140,   140,   140,   140,   140,   140,   140,   140,   141,
     141,   141,   142,   143,   143,   144,   144,   145,   145,   146,
     146,   147,   147,   148,   148,   148,   149,   149,   149,   149,
     149,   150,   150,   150,   151,   151,   151,   152,   152,   152,
     152,   153,   153,   153,   153,   154,   154,   154,   154,   154,
     154,   154,   155,   155,   155,   155,   155,   156,   156,   156,
     156,   156,   156,   157,   157,   157,   157,   157,   157,   157,
     157,   157,   157,   157,   158,   158,   158,   159,   159,   159,
     160,   160,   160,   160,   160,   160,   160,   160,   161,   161,
     162,   162,   163,   163,   164,   164
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
       1,     1,     1,     1,     3,     4,     3,     4,     1,     3,
       1,     2,     1,     3,     1,     3
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint16 yydefact[] =
{
       2,     0,     1,    81,   282,    34,    21,     0,    20,    19,
      18,    23,    24,    25,    26,    27,    30,    31,    28,    29,
      39,    40,    22,    48,    49,     0,     0,    96,     3,     4,
       5,     0,     0,    59,    61,    32,    41,    44,    16,    33,
       0,    79,     0,     0,    82,     0,    72,     0,     0,    82,
     100,    98,    97,     0,    11,     0,    52,     7,    17,    60,
      62,    50,    45,    46,     0,    47,     0,     6,    14,     0,
       0,     9,     0,    95,     0,   110,    89,   119,     0,     0,
     102,   106,   104,   109,    80,   270,   271,   272,   280,     0,
       0,     0,     0,   247,   248,   249,   250,   251,   252,   119,
       0,     0,     0,   235,   253,   273,     0,    77,     0,    73,
      75,     0,    83,   283,   101,    99,    13,    12,     0,     0,
       0,     0,     0,     0,   270,   284,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   154,   151,     0,   162,   142,   143,   144,     0,   145,
     146,   147,     0,   183,   185,   199,   203,   205,   207,   209,
     211,   213,   216,   221,   224,   227,   231,   232,   242,   264,
     267,     0,    15,    10,    54,     0,    85,   202,     0,   231,
     264,    90,     0,     0,   108,   121,   120,   122,     0,    91,
       0,    93,     0,     0,     0,   239,   236,   237,     0,     0,
       0,   121,     0,    37,     0,    35,   238,     0,   262,   263,
       0,     0,     0,   281,     0,     0,     0,    68,    74,    84,
      53,     0,    55,   112,     8,     0,    42,    51,     0,   155,
       0,     0,   245,   246,     0,   243,   244,     0,     0,     0,
       0,     0,     0,     0,     0,   178,   179,   180,     0,     0,
     267,     0,     0,   158,     0,     0,   156,   163,     0,   152,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     189,   190,   191,   192,   193,   197,   198,   194,   195,   196,
     188,     0,   265,   266,     0,     0,    86,     0,    87,     0,
     126,     0,     0,   134,     0,     0,   123,     0,     0,    92,
      94,   103,   107,   111,   105,     0,     0,   276,     0,   274,
      38,    36,   261,     0,     0,   256,   278,     0,   260,     0,
      70,    78,    69,    76,   117,     0,    43,     0,     0,    56,
       0,    63,    65,   148,     0,   150,     0,     0,     0,     0,
       0,     0,   177,     0,   181,     0,     0,   285,   159,     0,
     160,   157,   153,   184,   204,     0,     0,   206,   208,   210,
     212,   214,   215,   219,   220,   217,   218,   222,   223,   225,
     226,   228,   229,   230,   186,   187,     0,    88,   127,     0,
     128,   135,     0,   136,     0,   124,     0,   130,     0,     0,
     138,     0,     0,   240,   277,   275,     0,   254,   257,     0,
     258,     0,    71,     0,   113,     0,    58,    66,    57,     0,
       0,   149,     0,     0,     0,     0,     0,     0,     0,   182,
       0,   233,   161,   201,     0,     0,   268,   129,   137,   125,
     131,     0,   132,   139,     0,   140,   241,   255,   259,   279,
     114,     0,   115,   118,    64,    67,   164,   166,   167,     0,
       0,     0,     0,     0,     0,   234,   200,   269,   133,   141,
     116,     0,     0,   169,     0,     0,     0,     0,     0,     0,
     165,   168,   170,   171,     0,   173,     0,     0,     0,   172,
     174,   175,     0,   176
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    28,    29,   120,    67,    68,    69,    70,    32,
      33,    34,    35,    36,    37,   121,    55,    56,   227,    38,
     340,   341,    39,   108,   109,   110,    48,    41,    42,    43,
      52,   304,    79,    80,    81,    82,   222,   335,    83,   186,
     187,   144,   145,   146,   147,   148,   149,   150,   151,   152,
     153,   291,   154,   178,   155,   156,   157,   158,   159,   160,
     161,   162,   163,   164,   165,   166,   167,   102,   103,   168,
     169,   104,   327,   105,   170,   171
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -216
static const yytype_int16 yypact[] =
{
    -216,  1493,  -216,  -216,  -216,  -216,  -216,   -23,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  -216,  -216,  -216,  -216,   121,   105,   228,  -216,  -216,
    -216,   159,  1768,  1768,  1768,  -216,   -25,   260,  -216,  -216,
    1717,   199,  1646,   219,  -216,   933,    75,    61,    74,    81,
    -216,  -216,   228,    86,  -216,   223,  -216,   100,  -216,  -216,
    -216,  -216,  -216,  -216,    28,  -216,   495,  -216,  -216,  1717,
     159,  -216,   763,  -216,    98,  -216,  -216,     5,   115,   137,
     102,  -216,   114,  -216,   199,  -216,  -216,  -216,  -216,   391,
     405,   405,  1060,  -216,  -216,  -216,  -216,  -216,  -216,   204,
     147,   149,  1350,    45,  -216,   182,    61,   158,    19,   144,
    -216,   152,  -216,  -216,  -216,  -216,  -216,  -216,   105,  1089,
    1717,  1440,    81,   188,   196,  -216,  1428,  1442,  1442,  1350,
     201,   218,   220,   225,   982,   255,   323,   258,   266,  1103,
     883,  -216,  -216,   575,  -216,  -216,  -216,  -216,   655,  -216,
    -216,  -216,    10,  -216,  -216,   108,   350,   291,   295,   306,
     303,    30,   311,   251,   203,  -216,   570,  -216,   322,   570,
    -216,   317,  -216,  -216,   319,   324,  -216,  -216,    16,  -216,
    -216,  -216,   777,  1544,  -216,    31,  -216,   269,   327,  -216,
     336,  -216,  1746,   172,   883,  -216,  -216,  -216,   153,    12,
    1595,   276,   337,  -216,   339,  -216,  -216,   406,  -216,  -216,
    1350,   791,   412,  -216,    22,  1350,   351,  -216,    61,  -216,
    -216,  1089,  -216,  -216,  -216,   352,  -216,  -216,    15,  -216,
     982,   883,  -216,   317,  1060,  -216,  -216,   348,   982,  1350,
    1350,  1350,   366,  1138,   353,  -216,  -216,  -216,    27,   166,
      81,    69,   357,  -216,   683,   358,  -216,  -216,   356,  -216,
    1350,  1350,  1181,  1350,  1350,  1350,  1350,  1350,  1350,  1350,
    1350,  1350,  1350,  1350,  1350,  1350,  1350,  1350,  1350,  1350,
    -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,  -216,
    -216,  1350,  -216,  -216,  1350,  1350,  -216,   361,  -216,   362,
    -216,    25,   364,  -216,   168,   191,   269,   805,  1679,  -216,
    -216,  -216,  -216,  -216,  -216,   192,   365,  -216,   367,  -216,
    -216,  -216,  -216,    52,   369,  -216,  -216,    57,  -216,   368,
    -216,  -216,  -216,  -216,  -216,    60,  -216,   370,  1350,  -216,
     271,  -216,   373,  -216,   982,  -216,   -31,   -27,   157,   379,
    1195,   292,  -216,   374,  -216,   380,  1350,  -216,  -216,   384,
    -216,  -216,  -216,  -216,   350,  1350,   124,   291,   295,   306,
     303,    30,    30,   311,   311,   311,   311,   251,   251,   203,
     203,  -216,  -216,  -216,  -216,  -216,    67,  -216,  -216,   387,
    -216,  -216,   392,  -216,   394,  -216,   390,  -216,    58,   395,
    -216,   193,   396,  -216,  -216,  -216,   404,  -216,  -216,   397,
    -216,  1350,  -216,   136,  -216,  1011,  -216,  -216,  -216,   205,
    1350,  -216,   982,   982,   982,  1350,  1216,   294,  1230,  -216,
    1350,  -216,  -216,  -216,  1350,   407,  -216,  -216,  -216,  -216,
    -216,   408,  -216,  -216,   410,  -216,  -216,  -216,  -216,  -216,
    -216,   409,  -216,  -216,  -216,  -216,   378,  -216,  -216,   208,
     982,   210,  1308,  1322,   298,  -216,  -216,  -216,  -216,  -216,
    -216,   982,   401,  -216,   982,   982,   231,   982,   233,  1336,
    -216,  -216,  -216,  -216,   982,  -216,   982,   982,   234,  -216,
    -216,  -216,   982,  -216
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -216,  -216,  -216,  -216,  -216,   355,     4,   420,   486,  -216,
    -216,   118,  -216,  -216,  -216,  -216,  -216,   371,  -216,   261,
    -216,    71,  -216,   382,  -216,   273,     0,   -29,  -216,    -3,
    -216,   -39,  -216,  -216,   301,  -216,  -215,  -216,   -33,  -150,
    -147,   -35,  -216,  -216,   -30,   354,  -216,  -216,  -216,   -88,
     -92,   325,   -50,  -122,   246,   248,   232,   247,   245,   250,
     109,    41,   106,   110,  -100,   -24,   190,  -216,  -216,  -216,
      85,  -216,  -216,  -216,    -1,  -111
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -55
static const yytype_int16 yytable[] =
{
      44,    40,   206,    78,   199,    30,   334,   237,     3,     4,
      71,   258,   100,   318,    84,   233,   337,   297,     3,     4,
     216,   101,   177,   329,    51,    49,   389,   223,   353,   251,
      44,    57,     4,   305,     3,     4,    65,   422,   306,   173,
      61,   423,    44,   260,    45,   269,   270,   260,   179,   115,
     305,   248,   199,   406,   306,   207,   208,   209,   409,   441,
     301,   413,   198,   122,   107,   195,   196,   197,   435,    44,
     174,   182,   183,   172,   185,   111,    44,   184,   179,   177,
     319,    27,    26,   259,   260,   298,   260,   338,   339,   217,
      71,    27,   330,   331,   390,    64,   201,   182,   183,   242,
     354,   260,   195,   196,   197,   179,   199,   249,     3,     4,
     198,   210,   211,   257,   271,   272,   188,    44,   174,   326,
     251,   407,   323,   251,    46,   410,   260,   442,   261,   223,
     414,   411,   177,   212,   415,   295,   436,   357,   190,   250,
     106,   260,   112,   199,   261,    50,   199,   172,   202,   113,
     204,   346,   347,   348,   316,   351,    84,   180,   179,   116,
      53,   315,     3,     4,   198,   177,   181,   355,   363,   392,
     114,   119,    26,   -54,   -54,   313,   192,   381,   382,   383,
     185,    27,    49,   189,    44,   398,    47,   180,   193,   262,
     213,   179,   394,   402,   444,   343,   434,   201,   315,   384,
     453,   198,   385,   345,   198,   191,   450,   386,     3,     4,
     451,   180,   180,   180,   180,   203,   417,   205,   218,   257,
     219,   317,     3,     4,   314,   424,    26,    44,   342,   215,
     250,   260,    54,   250,   356,    27,   393,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   179,   179,   179,   179,
     179,   179,   179,   179,   179,   179,   431,   177,   229,   395,
     403,   445,   427,    62,     4,    72,    73,   180,   230,   401,
     182,   200,    26,   238,    20,    21,   472,   338,   474,   277,
      27,    27,   260,   179,   260,   239,    26,   240,   177,   278,
     279,    63,   241,    58,    59,    60,   117,   118,   455,   484,
     180,   486,   492,    77,    27,   260,    99,   260,   260,   421,
     373,   374,   375,   376,   179,   433,   232,   235,   236,   449,
     267,   268,   243,   223,   273,   274,   244,    64,   275,   276,
     465,   245,   179,   292,   293,   307,   308,   459,   461,   246,
     464,   179,   182,   200,   418,   419,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   180,   180,   180,   180,   180,
     180,   180,   180,   180,   180,   428,   260,   462,   260,   263,
     177,   479,   260,   264,   476,   478,   371,   372,   265,   377,
     378,   266,   228,   295,   466,   379,   380,   456,   457,   458,
     119,   488,   180,   296,    85,   309,   179,    86,    87,    88,
      89,    99,    90,    91,   310,   320,   179,   321,    85,   322,
     179,    86,    87,    88,    89,   328,    90,    91,    44,   342,
     344,   332,   336,   180,   349,   473,   352,   358,   361,   362,
     387,   388,   391,   404,   471,   405,   480,   408,   412,   482,
     483,   180,   485,   416,    77,   420,   425,   429,   430,   489,
     180,   490,   491,    77,   432,    99,   437,   493,   194,   440,
     438,    77,   439,   443,   446,   448,    93,    94,    95,    96,
      97,    98,    92,   447,   481,   224,   467,   468,   469,   470,
      93,    94,    95,    96,    97,    98,   143,    31,   214,   220,
     454,   333,    99,   312,   294,   367,   123,   254,   124,     4,
     125,    86,    87,    88,   126,   180,   127,   128,   366,   364,
     369,   368,     0,     0,     0,   180,   370,     0,     0,   180,
       0,     0,     0,     0,     0,     0,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,     0,   129,   130,
     131,     0,   132,   133,   134,   135,   136,   137,   138,   139,
      66,     0,   140,     0,     0,   141,     0,     0,   142,    77,
      93,    94,    95,    96,    97,    98,   252,     0,   124,     4,
     125,    86,    87,    88,   126,     0,   127,   128,     0,     0,
       0,   280,   281,   282,   283,   284,   285,   286,   287,   288,
     289,     0,     0,     0,     0,     0,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,     0,   129,   130,
     131,     0,   132,   133,   134,   135,   136,   137,   138,   139,
      66,   290,   140,     0,     0,   253,     0,     0,   142,     0,
      93,    94,    95,    96,    97,    98,   255,     0,   124,     4,
     125,    86,    87,    88,   126,     0,   127,   128,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   359,     0,   124,     4,   125,    86,
      87,    88,   126,     0,   127,   128,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,   129,   130,
     131,     0,   132,   133,   134,   135,   136,   137,   138,   139,
      66,     0,   140,     0,     0,   256,     0,     0,   142,     0,
      93,    94,    95,    96,    97,    98,   129,   130,   131,     0,
     132,   133,   134,   135,   136,   137,   138,   139,    66,     0,
     140,     0,     0,   360,     0,     0,   142,     0,    93,    94,
      95,    96,    97,    98,   175,     0,    85,     4,   125,    86,
      87,    88,   126,     0,   127,   128,     0,     0,   299,     0,
      85,     4,   125,    86,    87,    88,   126,     0,   127,   128,
       0,     0,   324,     0,    85,     4,   125,    86,    87,    88,
     126,     0,   127,   128,     0,     0,   396,     0,    85,     4,
     125,    86,    87,    88,   126,     0,   127,   128,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     140,     0,   176,     0,     0,     0,     0,     0,    93,    94,
      95,    96,    97,    98,   140,     0,   300,     0,     0,     0,
       0,     0,    93,    94,    95,    96,    97,    98,   140,   325,
       0,     0,     0,     0,     0,     0,    93,    94,    95,    96,
      97,    98,   140,     0,   397,     0,     0,     0,     0,     0,
      93,    94,    95,    96,    97,    98,    85,     4,   125,    86,
      87,    88,   126,     0,   127,   128,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     5,     0,     7,     0,     0,     0,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,     0,    85,     0,     0,    86,
      87,    88,    89,     0,    90,    91,     0,     0,    66,     0,
     140,     0,     0,     0,     0,     0,     0,     0,    93,    94,
      95,    96,    97,    98,     5,     0,     7,     0,     0,     0,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,   124,     4,   125,    86,    87,
      88,   126,     0,   127,   128,     0,     0,     0,     0,     0,
      92,     0,     0,     0,     0,     0,     0,     0,    93,    94,
      95,    96,    97,    98,    85,     4,   125,    86,    87,    88,
     126,     0,   127,   128,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   129,   130,   131,     0,   132,
     133,   134,   135,   136,   137,   138,   139,    66,     0,   140,
       0,     0,     0,     0,     0,   142,     0,    93,    94,    95,
      96,    97,    98,    85,     4,   125,    86,    87,    88,   126,
       0,   127,   128,     0,     0,     0,   221,     0,   140,     0,
       0,   452,     0,     0,     0,     0,    93,    94,    95,    96,
      97,    98,    85,     4,   125,    86,    87,    88,   126,     0,
     127,   128,     0,     0,     0,     0,    85,     4,   125,    86,
      87,    88,   126,     0,   127,   128,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    66,     0,   140,     0,     0,
       0,     0,     0,     0,     0,    93,    94,    95,    96,    97,
      98,    85,     4,   125,    86,    87,    88,   126,     0,   127,
     128,     0,     0,     0,   221,     0,   140,     0,     0,     0,
       0,     0,     0,     0,    93,    94,    95,    96,    97,    98,
     140,     0,     0,     0,     0,     0,   247,     0,    93,    94,
      95,    96,    97,    98,    85,     4,   125,    86,    87,    88,
     126,     0,   127,   128,     0,     0,     0,     0,    85,     4,
     125,    86,    87,    88,   126,   140,   127,   128,     0,     0,
       0,   350,     0,    93,    94,    95,    96,    97,    98,    85,
       4,   125,    86,    87,    88,   126,     0,   127,   128,     0,
       0,     0,     0,    85,     4,   125,    86,    87,    88,   126,
       0,   127,   128,     0,     0,     0,     0,     0,   140,     0,
       0,     0,     0,   365,     0,     0,    93,    94,    95,    96,
      97,    98,   140,     0,     0,     0,     0,     0,   426,     0,
      93,    94,    95,    96,    97,    98,     0,     0,     0,     0,
       0,     0,     0,   140,   460,     0,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,   140,     0,     0,
       0,     0,     0,   463,     0,    93,    94,    95,    96,    97,
      98,    85,     4,   125,    86,    87,    88,   126,     0,   127,
     128,     0,     0,     0,     0,    85,     4,   125,    86,    87,
      88,   126,     0,   127,   128,     0,     0,     0,     0,    85,
       4,   125,    86,    87,    88,   126,     0,   127,   128,     0,
       0,     0,     0,    85,     4,   125,    86,    87,    88,   126,
       0,   127,   128,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,   140,   475,     0,     0,     0,
       0,     0,     0,    93,    94,    95,    96,    97,    98,   140,
     477,     0,     0,     0,     0,     0,     0,    93,    94,    95,
      96,    97,    98,   140,   487,     0,     0,     0,     0,     0,
       0,    93,    94,    95,    96,    97,    98,   140,     0,     0,
       0,     0,     0,     0,     0,    93,    94,    95,    96,    97,
      98,    85,     4,   125,    86,    87,    88,   126,     0,   127,
     128,   225,     0,     0,     0,    85,     4,   125,    86,    87,
      88,   126,     0,   127,   128,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     0,     7,     0,     0,     0,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    22,    23,
      24,    25,     0,     2,     0,   231,     3,     4,     0,     0,
       0,     0,     0,    93,    94,    95,    96,    97,    98,   234,
     226,     0,     0,     0,     0,     0,     0,    93,    94,    95,
      96,    97,    98,     0,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,    24,    25,   302,     0,     3,     4,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      26,     0,     0,     0,     0,     0,     0,     0,     0,    27,
       0,     0,     0,     0,     0,     5,     0,     7,     0,     0,
       0,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,   302,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     182,   183,   303,     0,     0,     0,     0,     0,     0,     0,
      27,     0,     0,     0,     0,     0,     5,     0,     7,     0,
       0,     0,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    74,     0,    75,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   182,   200,   303,     0,     0,     0,     0,     0,     0,
       0,    27,     0,     0,     0,     0,     0,     5,     0,     7,
     399,     0,     0,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       5,     0,     7,     0,    76,     0,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   400,     5,     6,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    22,    23,    24,    25,     0,
       0,     0,     0,     0,     0,     0,     0,     5,     0,     7,
       0,     0,    66,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,   311,     5,
       0,     7,     0,     0,     0,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-216)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       1,     1,   102,    42,    92,     1,   221,   129,     3,     4,
      40,     1,    45,     1,    43,   126,     1,     1,     3,     4,
       1,    45,    72,     1,    27,    26,     1,   119,     1,   140,
      31,    31,     4,   183,     3,     4,    37,    68,   185,    69,
      65,    68,    43,    74,    67,    15,    16,    74,    72,    52,
     200,   139,   140,     1,   201,    10,    11,    12,     1,     1,
     182,     1,    92,    64,     3,    89,    90,    91,     1,    70,
      70,    66,    67,    69,    77,     1,    77,    77,   102,   129,
      68,    76,    67,    73,    74,    69,    74,    72,    73,    70,
     120,    76,    70,   215,    69,    67,    99,    66,    67,   134,
      73,    74,   126,   127,   128,   129,   194,   140,     3,     4,
     140,    66,    67,   148,    84,    85,     1,   118,   118,   211,
     231,    69,   210,   234,     3,    68,    74,    69,    20,   221,
      70,    74,   182,    88,    74,    66,    69,    68,     1,   140,
      65,    74,    68,   231,    20,    27,   234,   143,     1,    68,
       1,   239,   240,   241,     1,   243,   185,    72,   182,    73,
       1,   194,     3,     4,   194,   215,    68,     1,   260,     1,
      52,    71,    67,    73,    74,     3,    74,   277,   278,   279,
     183,    76,   183,    68,   185,   307,    65,   102,    74,    81,
       8,   215,     1,     1,     1,   230,    72,   200,   231,   291,
     415,   231,   294,   238,   234,    68,    70,   295,     3,     4,
      74,   126,   127,   128,   129,    68,   338,    68,    74,   254,
      68,    68,     3,     4,    52,    68,    67,   228,   228,    71,
     231,    74,    73,   234,    68,    76,    68,   261,   262,   263,
     264,   265,   266,   267,   268,   269,   270,   271,   272,   273,
     274,   275,   276,   277,   278,   279,   356,   307,    70,    68,
      68,    68,   350,     3,     4,    66,    67,   182,    72,   308,
      66,    67,    67,    72,    46,    47,    68,    72,    68,    76,
      76,    76,    74,   307,    74,    67,    67,    67,   338,    86,
      87,    31,    67,    32,    33,    34,    73,    74,   420,    68,
     215,    68,    68,    42,    76,    74,    45,    74,    74,   344,
     269,   270,   271,   272,   338,   365,   126,   127,   128,   411,
      17,    18,    67,   415,    13,    14,     3,    67,    77,    78,
     430,    73,   356,    11,    12,    66,    67,   425,   426,    73,
     428,   365,    66,    67,    73,    74,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    73,    74,    73,    74,    19,
     420,    73,    74,    82,   462,   463,   267,   268,    83,   273,
     274,    75,   121,    66,   434,   275,   276,   422,   423,   424,
      71,   479,   307,    69,     3,    68,   420,     6,     7,     8,
       9,   140,    11,    12,    68,    68,   430,    68,     3,     3,
     434,     6,     7,     8,     9,     3,    11,    12,   419,   419,
      72,    70,    70,   338,    58,   460,    73,    70,    70,    73,
      69,    69,    68,    68,    56,    68,   471,    68,    70,   474,
     475,   356,   477,    73,   183,    72,    67,    73,    68,   484,
     365,   486,   487,   192,    70,   194,    69,   492,    67,    69,
      68,   200,    68,    68,    68,    68,    75,    76,    77,    78,
      79,    80,    67,    69,    73,   120,    69,    69,    68,    70,
      75,    76,    77,    78,    79,    80,    66,     1,   106,   118,
     419,   218,   231,   192,   169,   263,     1,   143,     3,     4,
       5,     6,     7,     8,     9,   420,    11,    12,   262,   261,
     265,   264,    -1,    -1,    -1,   430,   266,    -1,    -1,   434,
      -1,    -1,    -1,    -1,    -1,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    54,
      55,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    -1,    -1,    73,   308,
      75,    76,    77,    78,    79,    80,     1,    -1,     3,     4,
       5,     6,     7,     8,     9,    -1,    11,    12,    -1,    -1,
      -1,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    -1,    -1,    -1,    -1,    -1,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    -1,    53,    54,
      55,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    71,    67,    -1,    -1,    70,    -1,    -1,    73,    -1,
      75,    76,    77,    78,    79,    80,     1,    -1,     3,     4,
       5,     6,     7,     8,     9,    -1,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,     1,    -1,     3,     4,     5,     6,
       7,     8,     9,    -1,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    53,    54,
      55,    -1,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    -1,    67,    -1,    -1,    70,    -1,    -1,    73,    -1,
      75,    76,    77,    78,    79,    80,    53,    54,    55,    -1,
      57,    58,    59,    60,    61,    62,    63,    64,    65,    -1,
      67,    -1,    -1,    70,    -1,    -1,    73,    -1,    75,    76,
      77,    78,    79,    80,     1,    -1,     3,     4,     5,     6,
       7,     8,     9,    -1,    11,    12,    -1,    -1,     1,    -1,
       3,     4,     5,     6,     7,     8,     9,    -1,    11,    12,
      -1,    -1,     1,    -1,     3,     4,     5,     6,     7,     8,
       9,    -1,    11,    12,    -1,    -1,     1,    -1,     3,     4,
       5,     6,     7,     8,     9,    -1,    11,    12,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    69,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    78,    79,    80,    67,    -1,    69,    -1,    -1,    -1,
      -1,    -1,    75,    76,    77,    78,    79,    80,    67,    68,
      -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    80,    67,    -1,    69,    -1,    -1,    -1,    -1,    -1,
      75,    76,    77,    78,    79,    80,     3,     4,     5,     6,
       7,     8,     9,    -1,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    33,    -1,    -1,    -1,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    -1,     3,    -1,    -1,     6,
       7,     8,     9,    -1,    11,    12,    -1,    -1,    65,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    78,    79,    80,    31,    -1,    33,    -1,    -1,    -1,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,     3,     4,     5,     6,     7,
       8,     9,    -1,    11,    12,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,
      77,    78,    79,    80,     3,     4,     5,     6,     7,     8,
       9,    -1,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    53,    54,    55,    -1,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    -1,    67,
      -1,    -1,    -1,    -1,    -1,    73,    -1,    75,    76,    77,
      78,    79,    80,     3,     4,     5,     6,     7,     8,     9,
      -1,    11,    12,    -1,    -1,    -1,    65,    -1,    67,    -1,
      -1,    70,    -1,    -1,    -1,    -1,    75,    76,    77,    78,
      79,    80,     3,     4,     5,     6,     7,     8,     9,    -1,
      11,    12,    -1,    -1,    -1,    -1,     3,     4,     5,     6,
       7,     8,     9,    -1,    11,    12,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    65,    -1,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      80,     3,     4,     5,     6,     7,     8,     9,    -1,    11,
      12,    -1,    -1,    -1,    65,    -1,    67,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,    80,
      67,    -1,    -1,    -1,    -1,    -1,    73,    -1,    75,    76,
      77,    78,    79,    80,     3,     4,     5,     6,     7,     8,
       9,    -1,    11,    12,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,     8,     9,    67,    11,    12,    -1,    -1,
      -1,    73,    -1,    75,    76,    77,    78,    79,    80,     3,
       4,     5,     6,     7,     8,     9,    -1,    11,    12,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,     8,     9,
      -1,    11,    12,    -1,    -1,    -1,    -1,    -1,    67,    -1,
      -1,    -1,    -1,    72,    -1,    -1,    75,    76,    77,    78,
      79,    80,    67,    -1,    -1,    -1,    -1,    -1,    73,    -1,
      75,    76,    77,    78,    79,    80,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    67,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    75,    76,    77,    78,    79,    80,    67,    -1,    -1,
      -1,    -1,    -1,    73,    -1,    75,    76,    77,    78,    79,
      80,     3,     4,     5,     6,     7,     8,     9,    -1,    11,
      12,    -1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
       8,     9,    -1,    11,    12,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,     8,     9,    -1,    11,    12,    -1,
      -1,    -1,    -1,     3,     4,     5,     6,     7,     8,     9,
      -1,    11,    12,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    67,    68,    -1,    -1,    -1,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,    67,
      68,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    80,    67,    68,    -1,    -1,    -1,    -1,    -1,
      -1,    75,    76,    77,    78,    79,    80,    67,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    75,    76,    77,    78,    79,
      80,     3,     4,     5,     6,     7,     8,     9,    -1,    11,
      12,     1,    -1,    -1,    -1,     3,     4,     5,     6,     7,
       8,     9,    -1,    11,    12,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    31,    -1,    33,    -1,    -1,    -1,    37,    38,    39,
      40,    41,    42,    43,    44,    45,    46,    47,    48,    49,
      50,    51,    -1,     0,    -1,    67,     3,     4,    -1,    -1,
      -1,    -1,    -1,    75,    76,    77,    78,    79,    80,    67,
      70,    -1,    -1,    -1,    -1,    -1,    -1,    75,    76,    77,
      78,    79,    80,    -1,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,     1,    -1,     3,     4,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      67,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    76,
      -1,    -1,    -1,    -1,    -1,    31,    -1,    33,    -1,    -1,
      -1,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,     1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      66,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      76,    -1,    -1,    -1,    -1,    -1,    31,    -1,    33,    -1,
      -1,    -1,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,     1,    -1,     3,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    66,    67,    68,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    76,    -1,    -1,    -1,    -1,    -1,    31,    -1,    33,
       1,    -1,    -1,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      31,    -1,    33,    -1,    68,    -1,    37,    38,    39,    40,
      41,    42,    43,    44,    45,    46,    47,    48,    49,    50,
      51,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    68,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    49,    50,    51,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    31,    -1,    33,
      -1,    -1,    65,    37,    38,    39,    40,    41,    42,    43,
      44,    45,    46,    47,    48,    49,    50,    51,    52,    31,
      -1,    33,    -1,    -1,    -1,    37,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    49,    50,    51
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    90,     0,     3,     4,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    67,    76,    91,    92,
      95,    97,    98,    99,   100,   101,   102,   103,   108,   111,
     115,   116,   117,   118,   163,    67,     3,    65,   115,   163,
     100,   118,   119,     1,    73,   105,   106,   115,   108,   108,
     108,    65,     3,    31,    67,   163,    65,    94,    95,    96,
      97,   133,    66,    67,     1,     3,    68,   108,   120,   121,
     122,   123,   124,   127,   116,     3,     6,     7,     8,     9,
      11,    12,    67,    75,    76,    77,    78,    79,    80,   108,
     127,   154,   156,   157,   160,   162,    65,     3,   112,   113,
     114,     1,    68,    68,   100,   118,    73,    73,    74,    71,
      93,   104,   163,     1,     3,     5,     9,    11,    12,    53,
      54,    55,    57,    58,    59,    60,    61,    62,    63,    64,
      67,    70,    73,    96,   130,   131,   132,   133,   134,   135,
     136,   137,   138,   139,   141,   143,   144,   145,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   158,   159,
     163,   164,    95,   133,   115,     1,    69,   141,   142,   154,
     159,    68,    66,    67,   115,   118,   128,   129,     1,    68,
       1,    68,    74,    74,    67,   154,   154,   154,   133,   138,
      67,   118,     1,    68,     1,    68,   153,    10,    11,    12,
      66,    67,    88,     8,   112,    71,     1,    70,    74,    68,
     106,    65,   125,   139,    94,     1,    70,   107,   108,    70,
      72,    67,   155,   164,    67,   155,   155,   142,    72,    67,
      67,    67,   130,    67,     3,    73,    73,    73,   138,   127,
     163,   164,     1,    70,   134,     1,    70,   130,     1,    73,
      74,    20,    81,    19,    82,    83,    75,    17,    18,    15,
      16,    84,    85,    13,    14,    77,    78,    76,    86,    87,
      21,    22,    23,    24,    25,    26,    27,    28,    29,    30,
      71,   140,    11,    12,   140,    66,    69,     1,    69,     1,
      69,   142,     1,    68,   120,   128,   129,    66,    67,    68,
      68,    52,   123,     3,    52,   127,     1,    68,     1,    68,
      68,    68,     3,   138,     1,    68,   139,   161,     3,     1,
      70,   142,    70,   114,   125,   126,    70,     1,    72,    73,
     109,   110,   115,   130,    72,   130,   138,   138,   138,    58,
      73,   138,    73,     1,    73,     1,    68,    68,    70,     1,
      70,    70,    73,   139,   144,    72,   143,   145,   146,   147,
     148,   149,   149,   150,   150,   150,   150,   151,   151,   152,
     152,   153,   153,   153,   139,   139,   138,    69,    69,     1,
      69,    68,     1,    68,     1,    68,     1,    69,   142,     1,
      68,   120,     1,    68,    68,    68,     1,    69,    68,     1,
      68,    74,    70,     1,    70,    74,    73,   142,    73,    74,
      72,   130,    68,    68,    68,    67,    73,   138,    73,    73,
      68,   153,    70,   141,    72,     1,    69,    69,    68,    68,
      69,     1,    69,    68,     1,    68,    68,    69,    68,   139,
      70,    74,    70,   125,   110,   142,   130,   130,   130,   138,
      68,   138,    73,    73,   138,   153,   141,    69,    69,    68,
      70,    56,    68,   130,    68,    68,   138,    68,   138,    73,
     130,    73,   130,   130,    68,   130,    68,    68,   138,   130,
     130,   130,    68,   130
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
	    if (((yyvsp[(1) - (1)].tok).symbol->type < MAX_LS && (yyvsp[(1) - (1)].tok).symbol->u_blist == 0) || (yyvsp[(1) - (1)].tok).symbol->type == NCONST) {
		immVarRemove((yyvsp[(1) - (1)].tok).start, (yyvsp[(1) - (1)].tok).end, (yyvsp[(1) - (1)].tok).symbol);
	    }
#endif	/* LMAIN */
	}
    break;

  case 80:
/* Line 1792 of yacc.c  */
#line 684 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(2) - (2)].tok).symbol;
#ifndef LMAIN
	    if (((yyvsp[(2) - (2)].tok).symbol->type < MAX_LS && (yyvsp[(2) - (2)].tok).symbol->u_blist == 0) || (yyvsp[(2) - (2)].tok).symbol->type == NCONST) {
		immVarRemove((yyvsp[(2) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(2) - (2)].tok).symbol);
	    }
#endif	/* LMAIN */
	}
    break;

  case 81:
/* Line 1792 of yacc.c  */
#line 697 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (1)].tok).symbol;
	}
    break;

  case 82:
/* Line 1792 of yacc.c  */
#line 702 "gram.y"
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
#line 709 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(2) - (3)].tok).symbol;
	}
    break;

  case 84:
/* Line 1792 of yacc.c  */
#line 714 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(2) - (4)].tok).symbol;
	    yyclearin; yyerrok;
	}
    break;

  case 85:
/* Line 1792 of yacc.c  */
#line 720 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (3)].tok).symbol;
	}
    break;

  case 86:
/* Line 1792 of yacc.c  */
#line 725 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (4)].tok).symbol;
	    yyclearin; yyerrok;
	}
    break;

  case 87:
/* Line 1792 of yacc.c  */
#line 731 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (4)].tok).symbol;
	}
    break;

  case 88:
/* Line 1792 of yacc.c  */
#line 736 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (5)].tok).symbol;
	    yyclearin; yyerrok;
	}
    break;

  case 89:
/* Line 1792 of yacc.c  */
#line 742 "gram.y"
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
#line 750 "gram.y"
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
#line 759 "gram.y"
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
#line 767 "gram.y"
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
#line 776 "gram.y"
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
#line 784 "gram.y"
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
#line 796 "gram.y"
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
#line 807 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 97:
/* Line 1792 of yacc.c  */
#line 812 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 98:
/* Line 1792 of yacc.c  */
#line 817 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 99:
/* Line 1792 of yacc.c  */
#line 822 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 100:
/* Line 1792 of yacc.c  */
#line 830 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 101:
/* Line 1792 of yacc.c  */
#line 835 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 102:
/* Line 1792 of yacc.c  */
#line 843 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 103:
/* Line 1792 of yacc.c  */
#line 848 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 104:
/* Line 1792 of yacc.c  */
#line 856 "gram.y"
    {			/* K&R C parameter list without types */
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 105:
/* Line 1792 of yacc.c  */
#line 861 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 106:
/* Line 1792 of yacc.c  */
#line 869 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 107:
/* Line 1792 of yacc.c  */
#line 874 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 108:
/* Line 1792 of yacc.c  */
#line 882 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    delete_sym(&(yyvsp[(2) - (2)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 109:
/* Line 1792 of yacc.c  */
#line 888 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 110:
/* Line 1792 of yacc.c  */
#line 896 "gram.y"
    {				/* K&R C parameter list without types */
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    delete_sym(&(yyvsp[(1) - (1)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 111:
/* Line 1792 of yacc.c  */
#line 902 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    delete_sym(&(yyvsp[(3) - (3)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 112:
/* Line 1792 of yacc.c  */
#line 911 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 113:
/* Line 1792 of yacc.c  */
#line 916 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 114:
/* Line 1792 of yacc.c  */
#line 921 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 115:
/* Line 1792 of yacc.c  */
#line 927 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 116:
/* Line 1792 of yacc.c  */
#line 932 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 117:
/* Line 1792 of yacc.c  */
#line 941 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 118:
/* Line 1792 of yacc.c  */
#line 946 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 119:
/* Line 1792 of yacc.c  */
#line 954 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 120:
/* Line 1792 of yacc.c  */
#line 959 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 121:
/* Line 1792 of yacc.c  */
#line 967 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 122:
/* Line 1792 of yacc.c  */
#line 972 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 123:
/* Line 1792 of yacc.c  */
#line 977 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 124:
/* Line 1792 of yacc.c  */
#line 985 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 125:
/* Line 1792 of yacc.c  */
#line 990 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 126:
/* Line 1792 of yacc.c  */
#line 996 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 127:
/* Line 1792 of yacc.c  */
#line 1001 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 128:
/* Line 1792 of yacc.c  */
#line 1007 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 129:
/* Line 1792 of yacc.c  */
#line 1012 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 130:
/* Line 1792 of yacc.c  */
#line 1018 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 131:
/* Line 1792 of yacc.c  */
#line 1023 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 132:
/* Line 1792 of yacc.c  */
#line 1029 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 133:
/* Line 1792 of yacc.c  */
#line 1034 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 134:
/* Line 1792 of yacc.c  */
#line 1040 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 135:
/* Line 1792 of yacc.c  */
#line 1045 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 136:
/* Line 1792 of yacc.c  */
#line 1051 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 137:
/* Line 1792 of yacc.c  */
#line 1056 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 138:
/* Line 1792 of yacc.c  */
#line 1062 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 139:
/* Line 1792 of yacc.c  */
#line 1067 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 140:
/* Line 1792 of yacc.c  */
#line 1073 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 141:
/* Line 1792 of yacc.c  */
#line 1078 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 142:
/* Line 1792 of yacc.c  */
#line 1087 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 143:
/* Line 1792 of yacc.c  */
#line 1092 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 144:
/* Line 1792 of yacc.c  */
#line 1097 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 145:
/* Line 1792 of yacc.c  */
#line 1102 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 146:
/* Line 1792 of yacc.c  */
#line 1107 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 147:
/* Line 1792 of yacc.c  */
#line 1112 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 148:
/* Line 1792 of yacc.c  */
#line 1120 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    delete_sym(&(yyvsp[(1) - (3)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 149:
/* Line 1792 of yacc.c  */
#line 1126 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 150:
/* Line 1792 of yacc.c  */
#line 1131 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 151:
/* Line 1792 of yacc.c  */
#line 1139 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 152:
/* Line 1792 of yacc.c  */
#line 1144 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 153:
/* Line 1792 of yacc.c  */
#line 1149 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 154:
/* Line 1792 of yacc.c  */
#line 1158 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 155:
/* Line 1792 of yacc.c  */
#line 1163 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 156:
/* Line 1792 of yacc.c  */
#line 1169 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 157:
/* Line 1792 of yacc.c  */
#line 1174 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 158:
/* Line 1792 of yacc.c  */
#line 1180 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 159:
/* Line 1792 of yacc.c  */
#line 1185 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 160:
/* Line 1792 of yacc.c  */
#line 1191 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 161:
/* Line 1792 of yacc.c  */
#line 1196 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 162:
/* Line 1792 of yacc.c  */
#line 1205 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 163:
/* Line 1792 of yacc.c  */
#line 1210 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 164:
/* Line 1792 of yacc.c  */
#line 1218 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 165:
/* Line 1792 of yacc.c  */
#line 1223 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (7)].tok).start;
	    (yyval.tok).end = (yyvsp[(7) - (7)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 166:
/* Line 1792 of yacc.c  */
#line 1228 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 167:
/* Line 1792 of yacc.c  */
#line 1236 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 168:
/* Line 1792 of yacc.c  */
#line 1241 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (7)].tok).start;
	    (yyval.tok).end = (yyvsp[(7) - (7)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 169:
/* Line 1792 of yacc.c  */
#line 1246 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (6)].tok).start;
	    (yyval.tok).end = (yyvsp[(6) - (6)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 170:
/* Line 1792 of yacc.c  */
#line 1251 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (7)].tok).start;
	    (yyval.tok).end = (yyvsp[(7) - (7)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 171:
/* Line 1792 of yacc.c  */
#line 1256 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (7)].tok).start;
	    (yyval.tok).end = (yyvsp[(7) - (7)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 172:
/* Line 1792 of yacc.c  */
#line 1261 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (8)].tok).start;
	    (yyval.tok).end = (yyvsp[(8) - (8)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 173:
/* Line 1792 of yacc.c  */
#line 1266 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (7)].tok).start;
	    (yyval.tok).end = (yyvsp[(7) - (7)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 174:
/* Line 1792 of yacc.c  */
#line 1271 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (8)].tok).start;
	    (yyval.tok).end = (yyvsp[(8) - (8)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 175:
/* Line 1792 of yacc.c  */
#line 1276 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (8)].tok).start;
	    (yyval.tok).end = (yyvsp[(8) - (8)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 176:
/* Line 1792 of yacc.c  */
#line 1281 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (9)].tok).start;
	    (yyval.tok).end = (yyvsp[(9) - (9)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 177:
/* Line 1792 of yacc.c  */
#line 1289 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    delete_sym(&(yyvsp[(2) - (3)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 178:
/* Line 1792 of yacc.c  */
#line 1295 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 179:
/* Line 1792 of yacc.c  */
#line 1300 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 180:
/* Line 1792 of yacc.c  */
#line 1305 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 181:
/* Line 1792 of yacc.c  */
#line 1310 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 182:
/* Line 1792 of yacc.c  */
#line 1315 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 183:
/* Line 1792 of yacc.c  */
#line 1324 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 184:
/* Line 1792 of yacc.c  */
#line 1329 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 185:
/* Line 1792 of yacc.c  */
#line 1337 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 186:
/* Line 1792 of yacc.c  */
#line 1343 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 187:
/* Line 1792 of yacc.c  */
#line 1349 "gram.y"
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
	    if ((yyvsp[(1) - (3)].tok).symbol->u_blist == 0 || (yyvsp[(1) - (3)].tok).symbol->type == NCONST) {
		immAssignFound((yyvsp[(1) - (3)].tok).start, (yyvsp[(2) - (3)].tok).start, (yyvsp[(3) - (3)].tok).end, (yyvsp[(1) - (3)].tok).symbol, (yyvsp[(2) - (3)].tok).inds);
	    }
#endif	/* LMAIN */
	}
    break;

  case 188:
/* Line 1792 of yacc.c  */
#line 1368 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 0;		/* = */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 189:
/* Line 1792 of yacc.c  */
#line 1374 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 2;		/* *= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 190:
/* Line 1792 of yacc.c  */
#line 1380 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 3;		/* /= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 191:
/* Line 1792 of yacc.c  */
#line 1386 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 4;		/* %= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 192:
/* Line 1792 of yacc.c  */
#line 1392 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 5;		/* += */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 193:
/* Line 1792 of yacc.c  */
#line 1398 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 6;		/* -= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 194:
/* Line 1792 of yacc.c  */
#line 1404 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 7;		/* &= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 195:
/* Line 1792 of yacc.c  */
#line 1410 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 8;		/* ^= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 196:
/* Line 1792 of yacc.c  */
#line 1416 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 9;		/* |= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 197:
/* Line 1792 of yacc.c  */
#line 1422 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 10;		/* <<= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 198:
/* Line 1792 of yacc.c  */
#line 1428 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).inds = 11;		/* >>= */
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 199:
/* Line 1792 of yacc.c  */
#line 1437 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 200:
/* Line 1792 of yacc.c  */
#line 1442 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 201:
/* Line 1792 of yacc.c  */
#line 1448 "gram.y"
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
#line 1462 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 203:
/* Line 1792 of yacc.c  */
#line 1470 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 204:
/* Line 1792 of yacc.c  */
#line 1475 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 205:
/* Line 1792 of yacc.c  */
#line 1483 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 206:
/* Line 1792 of yacc.c  */
#line 1488 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 207:
/* Line 1792 of yacc.c  */
#line 1496 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 208:
/* Line 1792 of yacc.c  */
#line 1501 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 209:
/* Line 1792 of yacc.c  */
#line 1509 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 210:
/* Line 1792 of yacc.c  */
#line 1514 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 211:
/* Line 1792 of yacc.c  */
#line 1522 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 212:
/* Line 1792 of yacc.c  */
#line 1527 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 213:
/* Line 1792 of yacc.c  */
#line 1535 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 214:
/* Line 1792 of yacc.c  */
#line 1540 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 215:
/* Line 1792 of yacc.c  */
#line 1545 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 216:
/* Line 1792 of yacc.c  */
#line 1553 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 217:
/* Line 1792 of yacc.c  */
#line 1558 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 218:
/* Line 1792 of yacc.c  */
#line 1563 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 219:
/* Line 1792 of yacc.c  */
#line 1568 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 220:
/* Line 1792 of yacc.c  */
#line 1573 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 221:
/* Line 1792 of yacc.c  */
#line 1581 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 222:
/* Line 1792 of yacc.c  */
#line 1586 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 223:
/* Line 1792 of yacc.c  */
#line 1591 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 224:
/* Line 1792 of yacc.c  */
#line 1599 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 225:
/* Line 1792 of yacc.c  */
#line 1604 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 226:
/* Line 1792 of yacc.c  */
#line 1609 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 227:
/* Line 1792 of yacc.c  */
#line 1617 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 228:
/* Line 1792 of yacc.c  */
#line 1622 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 229:
/* Line 1792 of yacc.c  */
#line 1627 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 230:
/* Line 1792 of yacc.c  */
#line 1632 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 231:
/* Line 1792 of yacc.c  */
#line 1640 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 232:
/* Line 1792 of yacc.c  */
#line 1645 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 233:
/* Line 1792 of yacc.c  */
#line 1650 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 234:
/* Line 1792 of yacc.c  */
#line 1655 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 235:
/* Line 1792 of yacc.c  */
#line 1664 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 236:
/* Line 1792 of yacc.c  */
#line 1669 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 237:
/* Line 1792 of yacc.c  */
#line 1674 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 238:
/* Line 1792 of yacc.c  */
#line 1679 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 239:
/* Line 1792 of yacc.c  */
#line 1684 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 240:
/* Line 1792 of yacc.c  */
#line 1689 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 241:
/* Line 1792 of yacc.c  */
#line 1694 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 242:
/* Line 1792 of yacc.c  */
#line 1703 "gram.y"
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
#line 1716 "gram.y"
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
	    if ((yyvsp[(2) - (2)].tok).symbol->u_blist == 0 || (yyvsp[(2) - (2)].tok).symbol->type == NCONST) {
		immAssignFound((yyvsp[(2) - (2)].tok).start, (yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(2) - (2)].tok).symbol, 12);	/* ++x */
	    }
#endif	/* LMAIN */
	}
    break;

  case 244:
/* Line 1792 of yacc.c  */
#line 1732 "gram.y"
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
	    if ((yyvsp[(2) - (2)].tok).symbol->u_blist == 0 || (yyvsp[(2) - (2)].tok).symbol->type == NCONST) {
		immAssignFound((yyvsp[(2) - (2)].tok).start, (yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(2) - (2)].tok).symbol, 13);	/* --x */
	    }
#endif	/* LMAIN */
	}
    break;

  case 245:
/* Line 1792 of yacc.c  */
#line 1748 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 246:
/* Line 1792 of yacc.c  */
#line 1753 "gram.y"
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
	    if ((yyval.tok).symbol->u_blist == 0 || (yyval.tok).symbol->type == NCONST) {
		immVarFound((yyval.tok).start, (yyval.tok).end, (yyval.tok).inds, (yyval.tok).inde, NULL);	/* adjust pEnd, set inds 0 inde early */
	    }
#endif	/* LMAIN */
	}
    break;

  case 247:
/* Line 1792 of yacc.c  */
#line 1773 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 248:
/* Line 1792 of yacc.c  */
#line 1778 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 249:
/* Line 1792 of yacc.c  */
#line 1783 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 250:
/* Line 1792 of yacc.c  */
#line 1788 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 251:
/* Line 1792 of yacc.c  */
#line 1793 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 252:
/* Line 1792 of yacc.c  */
#line 1798 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 253:
/* Line 1792 of yacc.c  */
#line 1806 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 254:
/* Line 1792 of yacc.c  */
#line 1811 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 255:
/* Line 1792 of yacc.c  */
#line 1816 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 256:
/* Line 1792 of yacc.c  */
#line 1822 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 257:
/* Line 1792 of yacc.c  */
#line 1827 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 258:
/* Line 1792 of yacc.c  */
#line 1833 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 259:
/* Line 1792 of yacc.c  */
#line 1838 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 260:
/* Line 1792 of yacc.c  */
#line 1844 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    delete_sym(&(yyvsp[(3) - (3)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 261:
/* Line 1792 of yacc.c  */
#line 1850 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    delete_sym(&(yyvsp[(3) - (3)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 262:
/* Line 1792 of yacc.c  */
#line 1856 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 263:
/* Line 1792 of yacc.c  */
#line 1861 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 264:
/* Line 1792 of yacc.c  */
#line 1869 "gram.y"
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
#line 1882 "gram.y"
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
	    if ((yyvsp[(1) - (2)].tok).symbol->u_blist == 0 || (yyvsp[(1) - (2)].tok).symbol->type == NCONST) {
		immAssignFound((yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(1) - (2)].tok).symbol, 14);	/* x++ */
	    }
#endif	/* LMAIN */
	}
    break;

  case 266:
/* Line 1792 of yacc.c  */
#line 1898 "gram.y"
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
	    if ((yyvsp[(1) - (2)].tok).symbol->u_blist == 0 || (yyvsp[(1) - (2)].tok).symbol->type == NCONST) {
		immAssignFound((yyvsp[(1) - (2)].tok).start, (yyvsp[(2) - (2)].tok).start, (yyvsp[(2) - (2)].tok).end, (yyvsp[(1) - (2)].tok).symbol, 15);	/* x-- */
	    }
#endif	/* LMAIN */
	}
    break;

  case 267:
/* Line 1792 of yacc.c  */
#line 1917 "gram.y"
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
#line 1931 "gram.y"
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
	    if ((yyval.tok).symbol->u_blist == 0 || (yyval.tok).symbol->type == NCONST) {
		immVarFound((yyval.tok).start, (yyval.tok).end, (yyval.tok).inds, (yyval.tok).inde, NULL);	/* adjust pEnd, set inds inde */
	    }
#endif	/* LMAIN */
	}
    break;

  case 269:
/* Line 1792 of yacc.c  */
#line 1947 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (5)].tok).start;
	    (yyval.tok).end = (yyvsp[(5) - (5)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 270:
/* Line 1792 of yacc.c  */
#line 1956 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    delete_sym(&(yyvsp[(1) - (1)].tok));
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 271:
/* Line 1792 of yacc.c  */
#line 1962 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 272:
/* Line 1792 of yacc.c  */
#line 1967 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = (yyvsp[(1) - (1)].tok).symbol;
#ifndef LMAIN
	    functionUse[0].c_cnt |= F_LOHI;	/* #define LO 0, #define HI 1 required in C code */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "primary_expression: BIT_CONSTANT %u %u %s\n",
		(yyval.tok).start, (yyval.tok).end, (yyval.tok).symbol->name);
#endif
#endif	/* LMAIN */
	}
    break;

  case 273:
/* Line 1792 of yacc.c  */
#line 1979 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 274:
/* Line 1792 of yacc.c  */
#line 1984 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 275:
/* Line 1792 of yacc.c  */
#line 1989 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 276:
/* Line 1792 of yacc.c  */
#line 1995 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 277:
/* Line 1792 of yacc.c  */
#line 2000 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (4)].tok).start;
	    (yyval.tok).end = (yyvsp[(4) - (4)].tok).end;
	    (yyval.tok).symbol = NULL;
	    yyclearin; yyerrok;
	}
    break;

  case 278:
/* Line 1792 of yacc.c  */
#line 2009 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 279:
/* Line 1792 of yacc.c  */
#line 2014 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (3)].tok).start;
	    (yyval.tok).end = (yyvsp[(3) - (3)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 280:
/* Line 1792 of yacc.c  */
#line 2022 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (1)].tok).start;
	    (yyval.tok).end = (yyvsp[(1) - (1)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 281:
/* Line 1792 of yacc.c  */
#line 2027 "gram.y"
    {
	    (yyval.tok).start = (yyvsp[(1) - (2)].tok).start;
	    (yyval.tok).end = (yyvsp[(2) - (2)].tok).end;
	    (yyval.tok).symbol = NULL;
	}
    break;

  case 282:
/* Line 1792 of yacc.c  */
#line 2035 "gram.y"
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
	    if ((yyval.tok).symbol->u_blist == 0 || (yyval.tok).symbol->type == NCONST) {
		immVarFound((yyval.tok).start, (yyval.tok).end, (yyval.tok).inds, (yyval.tok).inde, (yyval.tok).symbol);
	    }
#endif	/* LMAIN */
	}
    break;

  case 283:
/* Line 1792 of yacc.c  */
#line 2051 "gram.y"
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
	    if ((yyval.tok).symbol->u_blist == 0 || (yyval.tok).symbol->type == NCONST) {
		immVarFound((yyval.tok).start, (yyval.tok).end, (yyval.tok).inds, (yyval.tok).inde, NULL);	/* moves pStart and pEnd without changing vStart vEnd */
	    }
#endif	/* LMAIN */
	}
    break;

  case 284:
/* Line 1792 of yacc.c  */
#line 2071 "gram.y"
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
	    if ((yyval.tok).symbol->u_blist == 0 || (yyval.tok).symbol->type == NCONST) {
		immVarFound((yyval.tok).start, (yyval.tok).end, (yyval.tok).inds, (yyval.tok).inde, (yyval.tok).symbol);
	    }
#endif	/* LMAIN */
	}
    break;

  case 285:
/* Line 1792 of yacc.c  */
#line 2088 "gram.y"
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
	    if ((yyval.tok).symbol->u_blist == 0 || (yyval.tok).symbol->type == NCONST) {
		immVarFound((yyval.tok).start, (yyval.tok).end, (yyval.tok).inds, (yyval.tok).inde, NULL);	/* moves pStart and pEnd without changing vStart vEnd */
	    }
#endif	/* LMAIN */
	}
    break;


/* Line 1792 of yacc.c  */
#line 5415 "gram.tab.c"
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
#line 2112 "gram.y"


#ifndef	WIN32
#include	<unistd.h>
#endif	/* WIN32 */

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
		endop = vstart;			/* suppress output of this paranthesis */
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

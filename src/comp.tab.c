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

/* Substitute the type names.  */
#define YYSTYPE         ICSTYPE
/* Substitute the variable and function names.  */
#define yyparse         iCparse
#define yylex           iClex
#define yyerror         iCerror
#define yylval          iClval
#define yychar          iCchar
#define yydebug         iCdebug
#define yynerrs         iCnerrs

/* Copy the first part of user declarations.  */
/* Line 371 of yacc.c  */
#line 1 "comp.y"
 static const char comp_y[] =
"@(#)$Id: comp.tab.c 1.127 $";
/********************************************************************
 *
 *	Copyright (C) 1985-2017  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	comp.y
 *	grammar for immcc compiler
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/stat.h>
#ifdef WIN32
#include	<process.h>
#else	/* WIN32 */
#include	<unistd.h>
#endif	/* WIN32 */
#include	<string.h>
#include	<ctype.h>
#include	<assert.h>
#include	<errno.h>

#include	"comp.h"
#include	"comp.tab.h"

static int	yylex(void);		/* lexer for immcc renamed iClex() */
static void	unget(int);		/* shares buffers with get() */
int		ynerrs;			/* count of yyerror() calls */
int		gnerrs;			/* count of ierror() calls */
		/* NOTE iCnerrs is reset for every call to yaccpar() */
static int	copyCfrag(char, char, char, FILE*);	/* copy C action */
static void	ffexprCompile(char *, List_e *, int);	/* c_compile cBlock */
static void	blockUnblockListing(void);
static List_e *	op_adjust(Lis * lv);	/* adjust logic expression with constant */
static unsigned char ccfrag;		/* flag for CCFRAG syntax */
static int	cBlockFlag;		/* flag to contol ffexpr termination */
static FILE *	ccFP;			/* FILE * for CCFRAG destination */
static Type	stype;			/* to save TYPE in decl */




static Symbol	nSym = { "", ERR, GATE, };
static int	cFn = 0;
static char *	TaliasList[9][2] = {
	    { "EOI",	"TX0.0"  },	// end of initialisation
	    { "STDIN",	"TX0.1"  },	// stdin line received
	    { "LO",	"TX0.2"  },	// constant bit LO
	    { "HI",	"~TX0.2" },	// constant bit HI
	    { "T10ms",	"TX0.3"  },	// 10 ms internal timer
	    { "T100ms",	"TX0.4"  },	// 100 ms internal timer
	    { "T1sec",	"TX0.5"  },	// 1 second internal timer
	    { "T10sec",	"TX0.6"  },	// 10 second internal timer
	    { "T1min",	"TX0.7"  },	// 1 minute internal timer
};

#ifndef EFENCE
char		iCbuf[IMMBUFSIZE];	/* buffer to build imm statement */
char		iFunBuffer[IBUFSIZE];	/* buffer to build imm function symbols */
char *		iFunEnd = &iFunBuffer[IBUFSIZE];	/* pointer to end */
#else		/* malloc for EFENCE in main() */
char *		iCbuf;			/* buffer to build imm statement */
char *		iFunBuffer;		/* buffer to build imm function symbols */
char *		iFunEnd;		/* pointer to end */
#endif	/* EFENCE */

char *		iFunSymExt = 0;		/* pointer to imm function symbol Extension */
static char *	iFunSyText = 0;		/* pointer to function symbol text when active */
Sym		iRetSymbol;		/* .v is pointer to imm function return Symbol */
char *		iCstmtp;		/* manipulated in yylex() (reset in clrBuf()) */
static void	clrBuf(void);

/* Line 371 of yacc.c  */
#line 157 "comp.tab.c"

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
   by #include "comp.tab.h".  */
#ifndef YY_IC_COMP_TAB_H_INCLUDED
# define YY_IC_COMP_TAB_H_INCLUDED
/* Enabling traces.  */
#ifndef ICDEBUG
# if defined YYDEBUG
#  if YYDEBUG
#   define ICDEBUG 1
#  else
#   define ICDEBUG 0
#  endif
# else /* ! defined YYDEBUG */
#  define ICDEBUG 0
# endif /* ! defined YYDEBUG */
#endif  /* ! defined ICDEBUG */
#if ICDEBUG
extern int iCdebug;
#endif

/* Tokens.  */
#ifndef ICTOKENTYPE
# define ICTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum iCtokentype {
     UNDEF = 258,
     IMMCARRAY = 259,
     AVARC = 260,
     AVAR = 261,
     AOUT = 262,
     LVARC = 263,
     LVAR = 264,
     LOUT = 265,
     CVAR = 266,
     TVAR = 267,
     IF = 268,
     ELSE = 269,
     SWITCH = 270,
     EXTERN = 271,
     ASSIGN = 272,
     RETURN = 273,
     USE = 274,
     USETYPE = 275,
     IMM = 276,
     IMMC = 277,
     VOID = 278,
     CONST = 279,
     TYPE = 280,
     SIZEOF = 281,
     IFUNCTION = 282,
     CFUNCTION = 283,
     TFUNCTION = 284,
     VFUNCTION = 285,
     CNAME = 286,
     CCFRAG = 287,
     NUMBER = 288,
     LEXERR = 289,
     COMMENTEND = 290,
     LHEAD = 291,
     BE = 292,
     OPE = 293,
     B_OR = 294,
     B_AND = 295,
     EQU = 296,
     CMP = 297,
     RIGHT_SH = 298,
     LEFT_SH = 299,
     DIV = 300,
     PPMM = 301,
     PR = 302
   };
#endif


#if ! defined ICSTYPE && ! defined ICSTYPE_IS_DECLARED
typedef union ICSTYPE
{
/* Line 387 of yacc.c  */
#line 82 "comp.y"
		/* stack type */
    Sym		sym;			/* symbol table pointer */
    Lis		list;			/* linked list elements */
    Val		vai;			/* int numerical values */
    Vap		vap;			/* union { int, Symbol* } */
    Typ		typ;			/* type, ftype, em and fm */
    /* allows storing character as iClval.vai.v (2nd byte is NULL) */
    /* then char array can be referenced as $1.v, single char as $1.v[0] */


/* Line 387 of yacc.c  */
#line 266 "comp.tab.c"
} ICSTYPE;
# define ICSTYPE_IS_TRIVIAL 1
# define iCstype ICSTYPE /* obsolescent; will be withdrawn */
# define ICSTYPE_IS_DECLARED 1
#endif

extern ICSTYPE iClval;

#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int iCparse (void *YYPARSE_PARAM);
#else
int iCparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int iCparse (void);
#else
int iCparse ();
#endif
#endif /* ! YYPARSE_PARAM */

#endif /* !YY_IC_COMP_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */
/* Line 390 of yacc.c  */
#line 92 "comp.y"

static void
clrBuf(void)
{
    iCstmtp = iCbuf;
    memset(iCbuf, '\0', IMMBUFSIZE);
} /* clrBuf */

#if YYDEBUG
/********************************************************************
 *
 *	Detailed logic generation debugging output for different stack types
 *	usually only called if ((iC_debug & 0402) == 0402)
 *				pu(SYM, "dasgn: decl = aexpr", &$$);
 *		Parameter 1:	one of SYM LIS VAL VAP or TYP
 *		Parameter 2:	a message - usually the grammar rule
 *		Parameter 3:	a pointer to the full yacc stack type token
 *
 *	Each stack type token has identical first elements
 *		char * f	// start of generated compiler text
 *		char * l	// last+1 of generated compiler text
 *	The compiler text for this token is output after the name or value
 *	of the token itself which is in:
 *		Symbol * v	// Symbol name
 *		List_e * v	// name of Symbol List_e points to
 *		int      v	// numerical value
 *		Valp     v	// numerical value or List_e*
 *	or	Type     v	// type information
 *
 *******************************************************************/

void
pu(enum stackType t, const char * token, void * node)
{
    char *	t_first;
    char *	t_last;
    int		len;

    switch (t) {
    case SYM:
	if (((Sym*)node)->v) {
	    fprintf(iC_outFP, ">>>Sym	%s	%s :	", token, ((Sym*)node)->v->name);
	} else {
	    fprintf(iC_outFP, ">>>Sym	%s	0 =	", token);
	}
	break;
    case LIS:
	if (((Lis*)node)->v) {
	    fprintf(iC_outFP, ">>>Lis	%s	%s =	", token, ((Lis*)node)->v->le_sym ? ((Lis*)node)->v->le_sym->name : "@0");
	} else {
	    fprintf(iC_outFP, ">>>Lis	%s	0 =	", token);
	}
	break;
    case VAL:
	fprintf(iC_outFP, ">>>Val	%s	%d =	", token, ((Val*)node)->v);
	break;
    case VAP:
	if (((Vap*)node)->v.lfl == 0) {
	    fprintf(iC_outFP, ">>>Vap	%s	%d =	", token, ((Vap*)node)->v.nuv);
	} else
	if (((Vap*)node)->v.lfl == 1) {
	    fprintf(iC_outFP, ">>>Vap	%s *	%s =	", token, ((Vap*)node)->v.lpt->le_sym ? ((Vap*)node)->v.lpt->le_sym->name : "@0");
	} else {
	    fprintf(iC_outFP, ">>>Vap	%s *	error =	", token);
	}
	break;
    case TYP:
	/* single character for type, . normal or - external, ftype */
	fprintf(iC_outFP, ">>>Typ	%s	%c%c %c =	", token,
	    iC_os[((Typ*)node)->v.type],  iC_os[((Typ*)node)->v.em  & EM], iC_fos[((Typ*)node)->v.ftype]);
	break;
    }
    len = (t_last = ((Sym*)node)->l) - (t_first = ((Sym*)node)->f);
    if (len > 0 && t_first >= iCbuf && t_first < &iCbuf[IMMBUFSIZE] && t_last >= iCbuf && t_last < &iCbuf[IMMBUFSIZE]) {
	fprintf(iC_outFP, "%-*.*s\n", len, len, t_first);
    } else if (len != 0) {
	fprintf(iC_outFP, "\n*** Error: CODE GENERATION len = %d, f = %p l = %p iCbuf[%d] %p\n",
	    len, t_first, t_last, IMMBUFSIZE, iCbuf);
    } else {
	fprintf(iC_outFP, "\n");
    }
    fflush(iC_outFP);
} /* pu */

static void
pd(const char * token, Symbol * ss, Type s1, Symbol * s2)
{
    fprintf(iC_outFP, ">>>%s\t%s\t[%c%c %c] [%c%c %c] => [%c%c %c]\n",
	token, s2->name,
	/* single character for type, . normal or - external, ftype */
	iC_os[s1.type],  iC_os[s1.em  & EM], iC_fos[s1.ftype],
	iC_os[s2->type], iC_os[s2->em & EM], iC_fos[s2->ftype],
	iC_os[ss->type], iC_os[ss->em & EM], iC_fos[ss->ftype]);
    fflush(iC_outFP);
} /* pd */

#endif

/* Line 390 of yacc.c  */
#line 393 "comp.tab.c"

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
	 || (defined ICSTYPE_IS_TRIVIAL && ICSTYPE_IS_TRIVIAL)))

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
#define YYLAST   958

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  69
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  88
/* YYNRULES -- Number of rules.  */
#define YYNRULES  247
/* YYNRULES -- Number of states.  */
#define YYNSTATES  423

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   302

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    60,    39,     2,     2,     2,    51,    40,
      63,    64,    58,    56,    42,    57,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    46,    68,
       2,    43,     2,    45,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    65,     2,    66,    50,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    38,    49,    41,    61,     2,     2,     2,
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
      35,    36,    37,    44,    47,    48,    52,    53,    54,    55,
      59,    62,    67
};

#if ICDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,     4,     7,    11,    14,    17,    19,    21,
      24,    27,    30,    32,    34,    36,    39,    41,    44,    46,
      49,    51,    54,    56,    59,    61,    64,    65,    67,    69,
      71,    73,    75,    77,    79,    81,    83,    85,    87,    89,
      91,    93,    95,    97,    99,   101,   103,   105,   107,   109,
     111,   113,   116,   120,   123,   129,   136,   139,   145,   152,
     155,   158,   159,   161,   163,   165,   169,   172,   173,   175,
     177,   180,   183,   185,   187,   191,   194,   197,   200,   203,
     206,   209,   212,   215,   217,   219,   221,   223,   225,   229,
     233,   236,   239,   242,   245,   248,   252,   256,   260,   264,
     268,   272,   276,   278,   280,   282,   284,   286,   288,   290,
     292,   294,   298,   302,   306,   310,   314,   318,   322,   326,
     330,   334,   338,   342,   346,   350,   356,   361,   364,   367,
     370,   373,   375,   379,   381,   385,   387,   391,   393,   396,
     398,   399,   404,   405,   410,   411,   413,   416,   418,   421,
     423,   426,   428,   432,   433,   441,   443,   444,   449,   450,
     458,   462,   466,   470,   472,   474,   476,   478,   482,   486,
     490,   494,   496,   498,   500,   502,   506,   508,   513,   518,
     524,   530,   531,   533,   535,   539,   541,   543,   545,   547,
     549,   553,   558,   565,   573,   574,   577,   581,   584,   586,
     588,   590,   592,   594,   595,   597,   600,   602,   606,   610,
     613,   619,   622,   624,   627,   630,   634,   639,   645,   650,
     656,   661,   667,   672,   678,   679,   681,   684,   686,   690,
     692,   698,   699,   701,   707,   713,   715,   718,   720,   722,
     724,   726,   728,   732,   734,   740,   746,   752
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int16 yyrhs[] =
{
      70,     0,    -1,    -1,    70,    71,    -1,    70,     1,    68,
      -1,    73,    68,    -1,   118,    37,    -1,   109,    -1,   132,
      -1,    73,    68,    -1,   118,    37,    -1,   134,    68,    -1,
      74,    -1,    84,    -1,    96,    -1,    96,    42,    -1,    97,
      -1,    97,    42,    -1,   148,    -1,   148,    42,    -1,    85,
      -1,    85,    42,    -1,    91,    -1,    91,    42,    -1,   154,
      -1,   154,    42,    -1,    -1,   101,    -1,   122,    -1,   125,
      -1,   145,    -1,    80,    -1,    83,    -1,    78,    -1,    77,
      -1,    83,    -1,    81,    -1,    78,    -1,    11,    -1,    12,
      -1,   107,    -1,    82,    -1,    83,    -1,    81,    -1,    82,
      -1,     9,    -1,     6,    -1,     8,    -1,     5,    -1,    10,
      -1,     7,    -1,    19,    20,    -1,    84,    42,    20,    -1,
      86,     3,    -1,    86,     3,    63,    87,    64,    -1,    86,
       3,    63,    87,     1,    64,    -1,    86,    31,    -1,    86,
      31,    63,    87,    64,    -1,    86,    31,    63,    87,     1,
      64,    -1,    16,    25,    -1,    85,    42,    -1,    -1,    23,
      -1,    88,    -1,    89,    -1,    88,    42,    89,    -1,    25,
      90,    -1,    -1,     3,    -1,    75,    -1,    93,     3,    -1,
      93,    75,    -1,    21,    -1,    22,    -1,    16,    92,    25,
      -1,    91,    42,    -1,   154,    42,    -1,    98,     3,    -1,
      98,    76,    -1,    98,    82,    -1,    99,     3,    -1,    99,
      79,    -1,    99,    77,    -1,    94,    -1,   100,    -1,   121,
      -1,   124,    -1,    95,    -1,    95,    43,   142,    -1,    95,
      43,   108,    -1,    21,    25,    -1,    96,    42,    -1,    22,
      25,    -1,    97,    42,    -1,   148,    42,    -1,    94,    43,
     102,    -1,     3,    43,   102,    -1,     9,    43,   102,    -1,
       6,    43,   102,    -1,    10,    43,   102,    -1,     7,    43,
     102,    -1,   107,    43,   102,    -1,   103,    -1,   101,    -1,
       3,    -1,   107,    -1,    80,    -1,   156,    -1,    83,    -1,
     127,    -1,   142,    -1,    63,   104,    64,    -1,   103,    49,
     103,    -1,   103,    50,   103,    -1,   103,    51,   103,    -1,
     103,    53,   103,    -1,   103,    52,   103,    -1,   103,    58,
     103,    -1,   103,    59,   103,    -1,   103,    56,   103,    -1,
     103,    57,   103,    -1,   103,    55,   103,    -1,   103,    54,
     103,    -1,   103,    48,   103,    -1,   103,    47,   103,    -1,
     103,    45,   103,    46,   103,    -1,   103,    45,    46,   103,
      -1,    61,   103,    -1,    60,   103,    -1,    56,   103,    -1,
      57,   103,    -1,   102,    -1,    74,    42,   104,    -1,   123,
      -1,    74,    42,   105,    -1,   126,    -1,    74,    42,   106,
      -1,    33,    -1,    26,     4,    -1,   102,    -1,    -1,    36,
     110,    32,    41,    -1,    -1,    38,   112,    32,    41,    -1,
      -1,    42,    -1,    42,   114,    -1,   115,    -1,   115,    42,
      -1,   126,    -1,   126,    42,    -1,   123,    -1,   126,    42,
     102,    -1,    -1,    13,    63,   102,   113,    64,   117,   111,
      -1,   116,    -1,    -1,   116,    14,   119,   111,    -1,    -1,
      15,    63,   102,   113,    64,   120,   111,    -1,    94,    43,
     123,    -1,     3,    43,   123,    -1,    11,    43,   123,    -1,
      11,    -1,   122,    -1,   107,    -1,   143,    -1,    63,   105,
      64,    -1,    94,    43,   126,    -1,     3,    43,   126,    -1,
      12,    43,   126,    -1,    12,    -1,   125,    -1,   107,    -1,
     144,    -1,    63,   106,    64,    -1,    31,    -1,     3,    63,
     128,    64,    -1,    31,    63,   128,    64,    -1,     3,    63,
     128,     1,    64,    -1,    31,    63,   128,     1,    64,    -1,
      -1,   129,    -1,   102,    -1,   129,    42,   102,    -1,     3,
      -1,    27,    -1,    28,    -1,    29,    -1,    30,    -1,    98,
     130,    63,    -1,    92,    23,   130,    63,    -1,   131,   137,
      64,    38,   133,    41,    -1,   131,   137,     1,    64,    38,
     133,    41,    -1,    -1,   133,    72,    -1,   133,     1,    68,
      -1,    18,   135,    -1,   102,    -1,   123,    -1,   126,    -1,
     135,    -1,     4,    -1,    -1,   138,    -1,   138,    42,    -1,
     139,    -1,   138,    42,   139,    -1,    17,   140,     3,    -1,
     140,     3,    -1,   140,     3,    65,   149,    66,    -1,   141,
       3,    -1,    25,    -1,    92,    25,    -1,    24,    25,    -1,
      92,    24,    25,    -1,    27,    63,   146,    64,    -1,    27,
      63,   146,     1,    64,    -1,    28,    63,   146,    64,    -1,
      28,    63,   146,     1,    64,    -1,    29,    63,   146,    64,
      -1,    29,    63,   146,     1,    64,    -1,    30,    63,   146,
      64,    -1,    30,    63,   146,     1,    64,    -1,    -1,   147,
      -1,   147,    42,    -1,   136,    -1,   147,    42,   136,    -1,
     150,    -1,   150,    43,    38,   151,    41,    -1,    -1,   108,
      -1,    99,     3,    65,   149,    66,    -1,    99,     4,    65,
     149,    66,    -1,   153,    -1,   153,    42,    -1,     3,    -1,
       8,    -1,     5,    -1,   156,    -1,   152,    -1,   153,    42,
     152,    -1,   155,    -1,   155,    43,    38,   151,    41,    -1,
      93,     3,    65,   149,    66,    -1,    93,     4,    65,   149,
      66,    -1,     4,    65,   108,    66,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   244,   244,   249,   254,   263,   264,   268,   269,   273,
     274,   279,   286,   287,   288,   292,   296,   300,   304,   308,
     312,   313,   314,   315,   316,   317,   321,   322,   323,   324,
     325,   329,   330,   331,   335,   336,   340,   341,   345,   346,
     347,   354,   355,   359,   360,   364,   365,   369,   370,   374,
     375,   452,   466,   521,   542,   554,   567,   575,   593,   615,
     635,   643,   644,   645,   648,   649,   652,   667,   668,   674,
     756,   768,   819,   820,   824,   855,   862,   915,   940,   974,
    1035,  1060,  1094,  1109,  1115,  1121,  1127,  1136,  1146,  1178,
    1191,  1206,  1216,  1243,  1250,  1268,  1308,  1323,  1330,  1345,
    1352,  1367,  1383,  1393,  1410,  1424,  1431,  1440,  1449,  1458,
    1468,  1478,  1491,  1511,  1531,  1551,  1566,  1581,  1595,  1609,
    1623,  1637,  1651,  1675,  1707,  1747,  1792,  1827,  1850,  1879,
    1889,  1944,  1954,  1968,  1978,  1992,  2002,  2022,  2028,  2053,
    2110,  2110,  2127,  2127,  2154,  2155,  2156,  2162,  2163,  2164,
    2168,  2177,  2178,  2471,  2471,  2492,  2503,  2503,  2526,  2526,
    2547,  2565,  2576,  2589,  2590,  2591,  2609,  2617,  2677,  2695,
    2706,  2719,  2720,  2721,  2739,  2747,  2897,  2907,  2914,  2921,
    2929,  2939,  2940,  2943,  2953,  2975,  2976,  2977,  2978,  2979,
    2992,  3006,  3019,  3023,  3036,  3040,  3045,  3057,  3068,  3069,
    3070,  3074,  3075,  3091,  3092,  3093,  3096,  3106,  3134,  3146,
    3163,  3218,  3232,  3243,  3283,  3310,  3353,  3360,  3376,  3383,
    3399,  3406,  3423,  3433,  3466,  3467,  3468,  3471,  3481,  3585,
    3665,  3737,  3738,  3742,  3768,  3813,  3814,  3818,  3819,  3820,
    3821,  3825,  3836,  3914,  3968,  4024,  4051,  4112
};
#endif

#if ICDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "UNDEF", "IMMCARRAY", "AVARC", "AVAR",
  "AOUT", "LVARC", "LVAR", "LOUT", "CVAR", "TVAR", "IF", "ELSE", "SWITCH",
  "EXTERN", "ASSIGN", "RETURN", "USE", "USETYPE", "IMM", "IMMC", "VOID",
  "CONST", "TYPE", "SIZEOF", "IFUNCTION", "CFUNCTION", "TFUNCTION",
  "VFUNCTION", "CNAME", "CCFRAG", "NUMBER", "LEXERR", "COMMENTEND",
  "LHEAD", "BE", "'{'", "'\"'", "'\\''", "'}'", "','", "'='", "OPE", "'?'",
  "':'", "B_OR", "B_AND", "'|'", "'^'", "'&'", "EQU", "CMP", "RIGHT_SH",
  "LEFT_SH", "'+'", "'-'", "'*'", "DIV", "'!'", "'~'", "PPMM", "'('",
  "')'", "'['", "']'", "PR", "';'", "$accept", "program", "statement",
  "funcStatement", "simpleStatement", "asgnStatement", "dVariable",
  "dIVariable", "dIMVariable", "dMVariable", "dCVariable", "valueVariable",
  "valueIVariable", "valueCVariable", "outVariable", "useFlag", "extCdecl",
  "extCdeclHead", "dParams", "dPlist", "dPar", "dVar", "extDecl", "immT",
  "extDeclHead", "decl", "declC", "immDecl", "immCDecl", "declHead",
  "declCHead", "dasgn", "asgn", "aexpr", "expr", "comma_expr",
  "cexpr_comma_expr", "texpr_comma_expr", "Constant", "constExpr",
  "lBlock", "$@1", "cBlock", "$@2", "cref", "ctref", "ctdref", "ifini",
  "$@3", "ffexpr", "$@4", "$@5", "dcasgn", "casgn", "cexpr", "dtasgn",
  "tasgn", "texpr", "cCall", "cParams", "cPlist", "iFunTrigger",
  "iFunHead", "iFunDef", "funcBody", "return_statement", "actexpr",
  "ractexpr", "fParams", "fPlist", "formalParameter", "formalParaTypeDecl",
  "formalConstTypeDecl", "iFunCall", "cFunCall", "tFunCall", "vFunCall",
  "rParams", "rPlist", "immCarray", "immCindex", "immCarrayHead",
  "immCini", "cVariable", "immCiniList", "extimmCarray",
  "extimmCarrayHead", "immCarrayVariable", YY_NULL
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
     285,   286,   287,   288,   289,   290,   291,   292,   123,    34,
      39,   125,    44,    61,   293,    63,    58,   294,   295,   124,
      94,    38,   296,   297,   298,   299,    43,    45,    42,   300,
      33,   126,   301,    40,    41,    91,    93,   302,    59
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    69,    70,    70,    70,    71,    71,    71,    71,    72,
      72,    72,    73,    73,    73,    73,    73,    73,    73,    73,
      73,    73,    73,    73,    73,    73,    74,    74,    74,    74,
      74,    75,    75,    75,    76,    76,    77,    77,    78,    78,
      78,    79,    79,    80,    80,    81,    81,    82,    82,    83,
      83,    84,    84,    85,    85,    85,    85,    85,    85,    86,
      86,    87,    87,    87,    88,    88,    89,    90,    90,    90,
      91,    91,    92,    92,    93,    93,    93,    94,    94,    94,
      95,    95,    95,    96,    96,    96,    96,    97,    97,    97,
      98,    98,    99,    99,    99,   100,   101,   101,   101,   101,
     101,   101,   102,   102,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
     103,   104,   104,   105,   105,   106,   106,   107,   107,   108,
     110,   109,   112,   111,   113,   113,   113,   114,   114,   114,
     114,   115,   115,   117,   116,   118,   119,   118,   120,   118,
     121,   122,   122,   123,   123,   123,   123,   123,   124,   125,
     125,   126,   126,   126,   126,   126,   127,   127,   127,   127,
     127,   128,   128,   129,   129,   130,   130,   130,   130,   130,
     131,   131,   132,   132,   133,   133,   133,   134,   135,   135,
     135,   136,   136,   137,   137,   137,   138,   138,   139,   139,
     139,   139,   140,   140,   141,   141,   142,   142,   143,   143,
     144,   144,   145,   145,   146,   146,   146,   147,   147,   148,
     148,   149,   149,   150,   150,   151,   151,   152,   152,   152,
     152,   153,   153,   154,   154,   155,   155,   156
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     0,     2,     3,     2,     2,     1,     1,     2,
       2,     2,     1,     1,     1,     2,     1,     2,     1,     2,
       1,     2,     1,     2,     1,     2,     0,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     2,     3,     2,     5,     6,     2,     5,     6,     2,
       2,     0,     1,     1,     1,     3,     2,     0,     1,     1,
       2,     2,     1,     1,     3,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     1,     1,     1,     1,     3,     3,
       2,     2,     2,     2,     2,     3,     3,     3,     3,     3,
       3,     3,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     5,     4,     2,     2,     2,
       2,     1,     3,     1,     3,     1,     3,     1,     2,     1,
       0,     4,     0,     4,     0,     1,     2,     1,     2,     1,
       2,     1,     3,     0,     7,     1,     0,     4,     0,     7,
       3,     3,     3,     1,     1,     1,     1,     3,     3,     3,
       3,     1,     1,     1,     1,     3,     1,     4,     4,     5,
       5,     0,     1,     1,     3,     1,     1,     1,     1,     1,
       3,     4,     6,     7,     0,     2,     3,     2,     1,     1,
       1,     1,     1,     0,     1,     2,     1,     3,     3,     2,
       5,     2,     1,     2,     2,     3,     4,     5,     4,     5,
       4,     5,     4,     5,     0,     1,     2,     1,     3,     1,
       5,     0,     1,     5,     5,     1,     2,     1,     1,     1,
       1,     1,     3,     1,     5,     5,     5,     4
};

/* YYDEFACT[STATE-NAME] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       2,     0,     1,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    72,    73,     0,     0,   137,
     140,     3,     0,    12,    13,    20,     0,    22,     0,     0,
      83,    87,    14,    16,     0,     0,    84,    27,     0,     7,
     155,     0,    85,    28,    86,    29,   203,     8,    30,    18,
     229,    24,   243,     4,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    72,    73,    59,     0,    51,    90,    92,
     138,   224,     0,     5,     0,    60,    53,    56,    75,     0,
      70,     0,    48,    46,    50,    47,    45,    49,    38,    39,
      71,    33,    31,    43,    44,    32,    40,     0,     0,    91,
      93,    77,   186,   187,   188,   189,    78,    34,    37,    36,
      79,    35,     0,    80,     0,    82,    81,    41,    42,     0,
     156,     6,     0,     0,   212,     0,     0,   204,   206,     0,
       0,    94,     0,    76,     0,   104,     0,    46,    50,    45,
      49,   163,   171,     0,     0,     0,   176,     0,     0,     0,
       0,    26,   106,   108,   103,    96,   102,   105,   164,   161,
     172,   169,   109,   110,   166,   174,   107,   104,    26,    98,
     105,   100,    97,    99,     0,    26,   165,   162,     0,    26,
     173,   170,   144,   144,    74,   202,   198,   199,   200,   201,
     227,     0,   225,     0,    52,    61,    61,   185,     0,   231,
     231,    95,   160,   168,   139,    89,   110,   190,   231,   231,
     101,     0,     0,     0,   214,     0,   213,     0,     0,   205,
     209,   211,     0,     0,   181,     0,   224,   224,   224,   181,
     104,   129,   105,   130,   128,   127,     0,    27,   131,     0,
       0,     0,    28,   133,    29,   135,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,   165,     0,     0,   173,   145,     0,
       0,     0,   222,   226,   141,    62,    67,     0,    63,    64,
       0,   191,   232,     0,     0,     0,     0,   142,   157,   208,
     215,     0,   194,   207,   231,   237,   239,   238,     0,   241,
     235,   240,     0,   183,     0,   182,     0,     0,     0,     0,
       0,    26,   111,   167,   175,     0,     0,   124,   123,   112,
     113,   114,   116,   115,   122,   121,   119,   120,   117,   118,
      26,    26,    26,     0,    26,   165,   146,   147,   151,   149,
     153,   158,   223,   228,    68,    69,    66,     0,    54,     0,
       0,    57,   245,   246,   233,   234,     0,   194,     0,     0,
     230,   236,   244,     0,   177,     0,   247,     0,   216,     0,
     218,     0,   220,     0,   178,   132,   134,   136,   126,     0,
       0,     0,   165,   148,   150,     0,     0,    55,    65,    58,
       0,     0,     0,     0,     0,     0,   192,   195,     0,     0,
       0,     0,   210,   242,   179,   184,   217,   219,   221,   180,
     125,    26,   152,   154,   159,   143,   193,   196,   197,     9,
      77,    10,    11
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     1,    21,   397,   398,    23,    90,   106,   107,   108,
     116,   152,    93,    94,   153,    24,    25,    26,   277,   278,
     279,   346,    27,   125,    29,    30,    31,    32,    33,   399,
      35,    36,   154,   204,   156,   239,   240,   241,   170,   282,
      39,    72,   288,   356,   269,   336,   337,    40,   385,   400,
     211,   386,    42,   158,   187,    44,   160,   188,   162,   304,
     305,   112,    46,    47,   358,   401,   189,   190,   126,   127,
     128,   129,   130,   163,   164,   165,    48,   191,   192,    49,
     283,    50,   298,   299,   300,    51,    52,   166
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -305
static const yytype_int16 yypact[] =
{
    -305,   423,  -305,   -47,    20,    43,    49,    68,    69,    73,
      79,    67,    78,    47,    60,   106,   131,    70,    94,  -305,
    -305,  -305,    90,  -305,   117,   121,    11,   122,   143,   867,
     124,   126,   129,   133,   836,   898,  -305,  -305,   130,  -305,
     162,   144,  -305,  -305,  -305,  -305,   104,  -305,  -305,   148,
     151,   149,   152,  -305,   526,   721,   721,   721,   721,   159,
     344,   721,   721,  -305,  -305,  -305,   161,  -305,  -305,  -305,
    -305,   587,   165,  -305,   178,   147,   139,   153,   150,    37,
     156,   158,  -305,  -305,  -305,  -305,  -305,  -305,  -305,  -305,
    -305,  -305,  -305,  -305,  -305,  -305,  -305,   526,   721,   160,
     194,   166,  -305,  -305,  -305,  -305,  -305,  -305,  -305,  -305,
    -305,  -305,   167,   200,   203,  -305,  -305,  -305,  -305,   721,
    -305,  -305,    98,   192,  -305,    26,    25,   177,  -305,   221,
     266,   202,   233,   205,   236,   -18,   211,    43,    49,    68,
      69,    73,    79,   214,   215,   216,   218,   729,   729,   729,
     729,   515,  -305,  -305,  -305,  -305,   716,   130,  -305,  -305,
    -305,  -305,  -305,  -305,  -305,  -305,  -305,    13,   648,  -305,
     130,  -305,  -305,  -305,   241,   798,  -305,  -305,   256,   826,
    -305,  -305,   267,   267,  -305,   211,  -305,  -305,  -305,  -305,
    -305,    31,   270,   272,  -305,    74,    74,  -305,   253,   721,
     721,  -305,  -305,  -305,  -305,  -305,   -11,  -305,   721,   721,
    -305,   279,   293,   316,  -305,   296,  -305,   258,   285,   104,
     259,  -305,   134,   134,   721,   721,   587,   587,   587,   721,
     263,  -305,  -305,  -305,  -305,  -305,   287,   278,  -305,   286,
     288,   289,   290,  -305,   291,  -305,   660,   729,   729,   729,
     729,   729,   729,   729,   729,   729,   729,   729,   729,   729,
     721,   318,   159,   319,   130,   344,   323,   130,   599,   294,
     305,   307,  -305,   587,  -305,  -305,   555,    40,   332,  -305,
      42,  -305,  -305,   261,   309,   310,   314,  -305,  -305,  -305,
    -305,   343,  -305,  -305,   721,  -305,  -305,  -305,   342,  -305,
     345,  -305,   347,  -305,    45,   351,   320,    46,    51,    53,
      54,   515,  -305,  -305,  -305,   729,   887,   863,   899,   154,
     583,   828,   234,   249,    95,    95,    85,    85,  -305,  -305,
     648,   798,   826,   341,   788,  -305,  -305,   352,  -305,   354,
    -305,  -305,  -305,  -305,  -305,  -305,  -305,   321,  -305,   364,
     335,  -305,  -305,  -305,  -305,  -305,   369,  -305,    72,   336,
    -305,   134,  -305,   339,  -305,   721,  -305,   340,  -305,   348,
    -305,   349,  -305,   350,  -305,  -305,  -305,  -305,   716,   729,
     599,   367,   130,  -305,   721,   279,   279,  -305,  -305,  -305,
     365,   471,   353,   526,   106,   131,  -305,  -305,   359,   689,
     374,   363,  -305,  -305,  -305,  -305,  -305,  -305,  -305,  -305,
     716,   788,  -305,  -305,  -305,  -305,  -305,  -305,  -305,  -305,
    -305,  -305,  -305
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -305,  -305,  -305,  -305,   415,  -131,   141,  -305,   383,   -16,
    -305,   -10,   -33,   -31,   -17,  -305,  -305,  -305,   223,  -305,
      88,  -305,  -305,    23,  -305,  -305,  -305,  -305,  -305,   421,
    -305,  -305,     4,   240,   -15,  -295,  -304,  -303,    -1,   -89,
    -305,  -305,  -285,  -305,   257,  -305,  -305,  -305,  -305,   440,
    -305,  -305,  -305,     9,   -48,  -305,    14,    17,  -305,   217,
    -305,   368,  -305,  -305,    86,  -305,    55,   179,  -305,  -305,
     231,   329,  -305,   356,  -305,  -305,  -305,  -166,  -305,  -305,
    -170,  -305,   232,    96,  -305,  -305,  -305,  -200
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -186
static const yytype_int16 yytable[] =
{
      38,   109,   109,   110,   117,    37,   159,   376,   377,   205,
      43,   177,    95,    91,    76,    45,   375,   111,   118,    92,
     236,    53,   301,   301,    28,    54,   217,   376,    96,   377,
     284,   -88,   271,    96,    96,   375,    66,   261,   285,   286,
     197,   347,    77,   350,   263,   224,   363,   367,   266,   202,
     215,   216,   369,   157,   371,   373,   260,   -88,   176,   180,
     307,   308,   309,    54,   102,   103,   104,   105,    63,    64,
     157,   161,    65,   392,    70,     4,   224,   181,     5,     6,
      67,     7,     8,     9,    10,    11,    55,    12,    13,   218,
     393,    14,    56,   394,   395,   272,   157,   275,    17,   276,
     413,   414,    18,   243,   348,    19,   351,   376,   377,   364,
     368,    57,    58,   396,   203,   370,    59,   372,   374,    63,
      64,   122,    60,   124,   359,    63,    64,   243,   123,   124,
      61,    68,   231,   233,   234,   235,   306,   295,   136,   296,
     -26,    62,   297,   258,   259,   212,   232,   232,   232,   232,
     157,   256,   257,   258,   259,   237,    69,    71,    73,    74,
     242,   301,   174,    75,    78,   244,    79,    97,   245,    98,
     141,    99,   237,   119,   264,   100,   120,    43,   267,    37,
     236,   121,    45,    37,   242,    17,   184,   144,    43,    45,
     131,   133,    19,   244,   132,   134,   245,   193,   194,   261,
     263,   266,   195,   381,   250,   251,   252,   253,   254,   255,
     256,   257,   258,   259,   159,   -21,   196,   214,   -23,   219,
     338,   199,   175,   200,   220,   157,   157,   157,   -15,  -185,
     207,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   232,   232,   232,   232,   232,
     232,   232,   232,   232,   232,   232,   232,   232,   232,    95,
      91,   176,   -17,   243,   180,   208,    92,   335,   209,   221,
     -19,   222,   157,   -25,   223,    96,   225,   226,   227,   228,
     381,   229,   161,   243,   262,   339,   243,   253,   254,   255,
     256,   257,   258,   259,   155,   169,   171,   172,   173,   265,
     378,   182,   183,   254,   255,   256,   257,   258,   259,   268,
     157,   186,   273,   274,   232,   237,   281,   287,   216,   289,
     242,   290,   291,   292,   294,   244,   224,   352,   245,   311,
     264,   267,   159,   382,   237,    37,    37,   201,    37,    43,
     242,    43,  -103,   242,    45,    45,   244,   178,   244,   245,
     312,   245,   313,   314,  -164,  -172,   142,    38,   340,   210,
     330,   331,    37,   243,   410,   332,   109,    43,   110,   341,
      17,   342,    45,   145,   349,   353,   354,    19,   232,   335,
     355,   357,   111,   360,   380,   387,   366,   361,   362,   276,
      38,   238,   157,   365,   383,    37,   384,   161,    96,   389,
      43,   390,   402,   404,   406,    45,   415,   179,   238,   411,
     382,   421,   407,   408,   409,    37,    22,   345,   115,   280,
     242,   417,    34,     2,     3,   244,     4,   419,   245,     5,
       6,   422,     7,     8,     9,    10,    11,   388,    12,    13,
     270,    41,    14,   391,    15,    16,   310,   198,   418,    17,
     293,   213,   343,    18,   206,   302,    19,   403,     0,    20,
       0,     0,     0,     0,   303,     0,   186,   186,   186,   303,
       0,     0,   392,     0,     4,     0,     0,     5,     6,     0,
       7,     8,     9,    10,    11,     0,    12,    13,     0,   393,
      14,   -26,   394,   395,     0,     0,     0,    17,     0,     0,
     155,    18,     0,     0,    19,     0,     0,     0,     0,     0,
       0,     0,   416,   186,     0,     0,     0,     0,   135,   136,
      82,   137,   138,    85,   139,   140,   141,   142,     0,   135,
     136,    82,   137,   138,    85,   139,   140,   141,   142,   -26,
       0,    17,   143,   144,   145,    18,   146,     0,    19,     0,
       0,   238,    17,   143,   144,   145,     0,   146,   344,    19,
      82,    83,    84,    85,    86,    87,    88,    89,     0,     0,
     238,   147,   148,     0,     0,   149,   150,     0,   151,     0,
       0,    17,   147,   148,     0,     0,   149,   150,    19,   151,
     135,   185,    82,   137,   138,    85,   139,   140,   141,   142,
       0,     0,   333,     0,     0,   405,     0,     0,     0,     0,
     141,   142,     0,    17,   143,   144,   145,     0,   146,     0,
      19,     0,     0,     0,   412,    17,     0,   144,   145,     0,
       0,     0,    19,   186,   251,   252,   253,   254,   255,   256,
     257,   258,   259,   147,   148,     0,     0,   149,   150,     0,
     151,   135,   136,    82,   137,   138,    85,   139,   140,     9,
      10,     0,   334,   230,   136,    82,    83,    84,    85,    86,
      87,     0,     0,     0,    17,   143,     0,     0,    18,   146,
       0,    19,     0,     0,     0,     0,    17,   143,     0,     0,
       0,   146,   420,    19,    82,    83,    84,    85,    86,    87,
      88,    89,     0,     0,   147,   148,   315,     0,   149,   150,
       0,   168,     0,     0,     0,    17,   147,   148,     0,     0,
     149,   150,    19,   168,   167,   136,    82,   137,   138,    85,
     139,   140,   230,   136,    82,    83,    84,    85,    86,    87,
       0,     0,     0,     0,     0,     0,     0,    17,   143,     0,
       0,     0,   146,     0,    19,    17,   143,     0,     0,     0,
     146,   246,    19,   247,   248,   249,   250,   251,   252,   253,
     254,   255,   256,   257,   258,   259,     0,   147,   148,     0,
       0,   149,   150,     0,   168,   147,   148,     0,     0,   149,
     150,     4,   168,     0,     5,     6,     0,     7,     8,   141,
     142,     4,     0,     0,     5,     6,     0,     7,     8,   141,
      10,     0,     0,     0,    17,     0,   144,   145,    18,     0,
       0,    19,     0,     0,    17,     0,   144,     0,    18,     4,
       0,    19,     5,     6,     0,     7,     8,     9,   142,   101,
       0,    82,    83,    84,    85,    86,    87,    88,    89,     0,
       0,   334,    17,     0,     0,   145,    18,     0,     0,    19,
       0,   175,    17,   102,   103,   104,   105,     0,     0,    19,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
     252,   253,   254,   255,   256,   257,   258,   259,     0,   179,
       0,     0,     0,    17,     0,     0,     0,     0,     0,     0,
      19,   113,   114,    82,    83,    84,    85,    86,    87,    88,
      89,   248,   249,   250,   251,   252,   253,   254,   255,   256,
     257,   258,   259,     0,    17,     0,     0,     0,     0,     0,
       0,    19,   246,   379,   247,   248,   249,   250,   251,   252,
     253,   254,   255,   256,   257,   258,   259,     0,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259
};

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-305)))

#define yytable_value_is_error(Yytable_value) \
  YYID (0)

static const yytype_int16 yycheck[] =
{
       1,    34,    35,    34,    35,     1,    54,   311,   311,    98,
       1,    59,    29,    29,     3,     1,   311,    34,    35,    29,
     151,    68,   222,   223,     1,    43,     1,   331,    29,   332,
     200,    42,     1,    34,    35,   330,    13,   168,   208,   209,
       3,     1,    31,     1,   175,    63,     1,     1,   179,    97,
      24,    25,     1,    54,     1,     1,    43,    68,    59,    60,
     226,   227,   228,    43,    27,    28,    29,    30,    21,    22,
      71,    54,    25,     1,     4,     3,    63,    60,     6,     7,
      20,     9,    10,    11,    12,    13,    43,    15,    16,    64,
      18,    19,    43,    21,    22,    64,    97,    23,    26,    25,
     385,   386,    30,   151,    64,    33,    64,   411,   411,    64,
      64,    43,    43,    41,    97,    64,    43,    64,    64,    21,
      22,    17,    43,    25,   294,    21,    22,   175,    24,    25,
      63,    25,   147,   148,   149,   150,   225,     3,     4,     5,
      68,    63,     8,    58,    59,   122,   147,   148,   149,   150,
     151,    56,    57,    58,    59,   151,    25,    63,    68,    42,
     151,   361,     3,    42,    42,   151,    23,    43,   151,    43,
      11,    42,   168,    43,   175,    42,    14,   168,   179,   175,
     311,    37,   168,   179,   175,    26,    25,    28,   179,   175,
      42,    42,    33,   179,    43,    43,   179,    32,    20,   330,
     331,   332,    63,   334,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,   262,    68,    63,    25,    68,    42,
     268,    65,    63,    65,     3,   226,   227,   228,    68,    63,
      63,   246,   247,   248,   249,   250,   251,   252,   253,   254,
     255,   256,   257,   258,   259,   246,   247,   248,   249,   250,
     251,   252,   253,   254,   255,   256,   257,   258,   259,   276,
     276,   262,    68,   311,   265,    65,   276,   268,    65,     3,
      68,    38,   273,    68,    38,   276,    65,    63,    63,    63,
     411,    63,   265,   331,    43,   268,   334,    53,    54,    55,
      56,    57,    58,    59,    54,    55,    56,    57,    58,    43,
     315,    61,    62,    54,    55,    56,    57,    58,    59,    42,
     311,    71,    42,    41,   315,   311,    63,    38,    25,     3,
     311,    25,    64,    38,    65,   311,    63,    66,   311,    42,
     331,   332,   380,   334,   330,   331,   332,    97,   334,   330,
     331,   332,    64,   334,   330,   331,   332,     3,   334,   332,
      64,   334,    64,    64,    64,    64,    12,   358,    64,   119,
      42,    42,   358,   411,   379,    42,   399,   358,   399,    64,
      26,    64,   358,    29,    42,    66,    66,    33,   379,   380,
      66,    38,   399,    41,    43,    64,    66,    42,    41,    25,
     391,   151,   393,    42,    42,   391,    42,   380,   399,    64,
     391,    32,    66,    64,    64,   391,    41,    63,   168,    42,
     411,    37,    64,    64,    64,   411,     1,   276,    35,   196,
     411,    68,     1,     0,     1,   411,     3,    68,   411,     6,
       7,    68,     9,    10,    11,    12,    13,   349,    15,    16,
     183,     1,    19,   357,    21,    22,   229,    79,   393,    26,
     219,   122,   273,    30,    98,   223,    33,   361,    -1,    36,
      -1,    -1,    -1,    -1,   224,    -1,   226,   227,   228,   229,
      -1,    -1,     1,    -1,     3,    -1,    -1,     6,     7,    -1,
       9,    10,    11,    12,    13,    -1,    15,    16,    -1,    18,
      19,    68,    21,    22,    -1,    -1,    -1,    26,    -1,    -1,
     260,    30,    -1,    -1,    33,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    41,   273,    -1,    -1,    -1,    -1,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    -1,     3,
       4,     5,     6,     7,     8,     9,    10,    11,    12,    68,
      -1,    26,    27,    28,    29,    30,    31,    -1,    33,    -1,
      -1,   311,    26,    27,    28,    29,    -1,    31,     3,    33,
       5,     6,     7,     8,     9,    10,    11,    12,    -1,    -1,
     330,    56,    57,    -1,    -1,    60,    61,    -1,    63,    -1,
      -1,    26,    56,    57,    -1,    -1,    60,    61,    33,    63,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      -1,    -1,     3,    -1,    -1,   365,    -1,    -1,    -1,    -1,
      11,    12,    -1,    26,    27,    28,    29,    -1,    31,    -1,
      33,    -1,    -1,    -1,   384,    26,    -1,    28,    29,    -1,
      -1,    -1,    33,   393,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    56,    57,    -1,    -1,    60,    61,    -1,
      63,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    -1,    63,     3,     4,     5,     6,     7,     8,     9,
      10,    -1,    -1,    -1,    26,    27,    -1,    -1,    30,    31,
      -1,    33,    -1,    -1,    -1,    -1,    26,    27,    -1,    -1,
      -1,    31,     3,    33,     5,     6,     7,     8,     9,    10,
      11,    12,    -1,    -1,    56,    57,    46,    -1,    60,    61,
      -1,    63,    -1,    -1,    -1,    26,    56,    57,    -1,    -1,
      60,    61,    33,    63,     3,     4,     5,     6,     7,     8,
       9,    10,     3,     4,     5,     6,     7,     8,     9,    10,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    26,    27,    -1,
      -1,    -1,    31,    -1,    33,    26,    27,    -1,    -1,    -1,
      31,    45,    33,    47,    48,    49,    50,    51,    52,    53,
      54,    55,    56,    57,    58,    59,    -1,    56,    57,    -1,
      -1,    60,    61,    -1,    63,    56,    57,    -1,    -1,    60,
      61,     3,    63,    -1,     6,     7,    -1,     9,    10,    11,
      12,     3,    -1,    -1,     6,     7,    -1,     9,    10,    11,
      12,    -1,    -1,    -1,    26,    -1,    28,    29,    30,    -1,
      -1,    33,    -1,    -1,    26,    -1,    28,    -1,    30,     3,
      -1,    33,     6,     7,    -1,     9,    10,    11,    12,     3,
      -1,     5,     6,     7,     8,     9,    10,    11,    12,    -1,
      -1,    63,    26,    -1,    -1,    29,    30,    -1,    -1,    33,
      -1,    63,    26,    27,    28,    29,    30,    -1,    -1,    33,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      52,    53,    54,    55,    56,    57,    58,    59,    -1,    63,
      -1,    -1,    -1,    26,    -1,    -1,    -1,    -1,    -1,    -1,
      33,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    -1,    26,    -1,    -1,    -1,    -1,    -1,
      -1,    33,    45,    46,    47,    48,    49,    50,    51,    52,
      53,    54,    55,    56,    57,    58,    59,    -1,    49,    50,
      51,    52,    53,    54,    55,    56,    57,    58,    59
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    70,     0,     1,     3,     6,     7,     9,    10,    11,
      12,    13,    15,    16,    19,    21,    22,    26,    30,    33,
      36,    71,    73,    74,    84,    85,    86,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   107,   109,
     116,   118,   121,   122,   124,   125,   131,   132,   145,   148,
     150,   154,   155,    68,    43,    43,    43,    43,    43,    43,
      43,    63,    63,    21,    22,    25,    92,    20,    25,    25,
       4,    63,   110,    68,    42,    42,     3,    31,    42,    23,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      75,    78,    80,    81,    82,    83,   107,    43,    43,    42,
      42,     3,    27,    28,    29,    30,    76,    77,    78,    81,
      82,    83,   130,     3,     4,    77,    79,    82,    83,    43,
      14,    37,    17,    24,    25,    92,   137,   138,   139,   140,
     141,    42,    43,    42,    43,     3,     4,     6,     7,     9,
      10,    11,    12,    27,    28,    29,    31,    56,    57,    60,
      61,    63,    80,    83,   101,   102,   103,   107,   122,   123,
     125,   126,   127,   142,   143,   144,   156,     3,    63,   102,
     107,   102,   102,   102,     3,    63,   107,   123,     3,    63,
     107,   126,   102,   102,    25,     4,   102,   123,   126,   135,
     136,   146,   147,    32,    20,    63,    63,     3,   130,    65,
      65,   102,   123,   126,   102,   108,   142,    63,    65,    65,
     102,   119,    92,   140,    25,    24,    25,     1,    64,    42,
       3,     3,    38,    38,    63,    65,    63,    63,    63,    63,
       3,   103,   107,   103,   103,   103,    74,   101,   102,   104,
     105,   106,   122,   123,   125,   126,    45,    47,    48,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      43,    74,    43,    74,   107,    43,    74,   107,    42,   113,
     113,     1,    64,    42,    41,    23,    25,    87,    88,    89,
      87,    63,   108,   149,   149,   149,   149,    38,   111,     3,
      25,    64,    38,   139,    65,     3,     5,     8,   151,   152,
     153,   156,   151,   102,   128,   129,   108,   146,   146,   146,
     128,    42,    64,    64,    64,    46,   103,   103,   103,   103,
     103,   103,   103,   103,   103,   103,   103,   103,   103,   103,
      42,    42,    42,     3,    63,   107,   114,   115,   123,   126,
      64,    64,    64,   136,     3,    75,    90,     1,    64,    42,
       1,    64,    66,    66,    66,    66,   112,    38,   133,   149,
      41,    42,    41,     1,    64,    42,    66,     1,    64,     1,
      64,     1,    64,     1,    64,   104,   105,   106,   103,    46,
      43,    74,   107,    42,    42,   117,   120,    64,    89,    64,
      32,   133,     1,    18,    21,    22,    41,    72,    73,    98,
     118,   134,    66,   152,    64,   102,    64,    64,    64,    64,
     103,    42,   102,   111,   111,    41,    41,    68,   135,    68,
       3,    37,    68
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
#if ICDEBUG

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
#else /* !ICDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !ICDEBUG */


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
#line 244 "comp.y"
    {
		(yyval.sym).f = (yyval.sym).l = 0;
		(yyval.sym).v = 0;
		clrBuf();
	    }
    break;

  case 3:
/* Line 1792 of yacc.c  */
#line 249 "comp.y"
    {
		(yyval.sym)   = (yyvsp[(2) - (2)].sym);
		(yyval.sym).f = (yyval.sym).l = 0;
		clrBuf();
	    }
    break;

  case 4:
/* Line 1792 of yacc.c  */
#line 254 "comp.y"
    {
		(yyval.sym).f = (yyval.sym).l = 0;
		(yyval.sym).v = 0;
		clrBuf();
		yyerrok;
	    }
    break;

  case 5:
/* Line 1792 of yacc.c  */
#line 263 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (2)].sym); }
    break;

  case 6:
/* Line 1792 of yacc.c  */
#line 264 "comp.y"
    {			/* if or switch */
		(yyval.sym).f = (yyvsp[(1) - (2)].list).f; (yyval.sym).l = (yyvsp[(1) - (2)].list).l;
		(yyval.sym).v = op_asgn(0, &(yyvsp[(1) - (2)].list), GATE);		/* returns 0 for missing slave gate in ffexpr */
	    }
    break;

  case 7:
/* Line 1792 of yacc.c  */
#line 268 "comp.y"
    { (yyval.sym).v = 0;  }
    break;

  case 8:
/* Line 1792 of yacc.c  */
#line 269 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 9:
/* Line 1792 of yacc.c  */
#line 273 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (2)].sym); }
    break;

  case 10:
/* Line 1792 of yacc.c  */
#line 274 "comp.y"
    {			/* if or switch */
		(yyval.sym).f = (yyvsp[(1) - (2)].list).f; (yyval.sym).l = (yyvsp[(1) - (2)].list).l;
		(yyval.sym).v = op_asgn(0, &(yyvsp[(1) - (2)].list), GATE);		/* returns 0 for missing slave gate in ffexpr */
		collectStatement((yyval.sym).v);
	    }
    break;

  case 11:
/* Line 1792 of yacc.c  */
#line 279 "comp.y"
    {			/* function return statement */
		(yyval.sym) = (yyvsp[(1) - (2)].sym);
		collectStatement((yyvsp[(1) - (2)].sym).v);
	    }
    break;

  case 12:
/* Line 1792 of yacc.c  */
#line 286 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 13:
/* Line 1792 of yacc.c  */
#line 287 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 14:
/* Line 1792 of yacc.c  */
#line 288 "comp.y"
    {			/* imm declaration */
		(yyval.sym)   = (yyvsp[(1) - (1)].sym);
		iFunSyText = 0;
	    }
    break;

  case 15:
/* Line 1792 of yacc.c  */
#line 292 "comp.y"
    {			/* " new style for immac which can generate ,; */
		(yyval.sym)   = (yyvsp[(1) - (2)].sym);
		iFunSyText = 0;
	    }
    break;

  case 16:
/* Line 1792 of yacc.c  */
#line 296 "comp.y"
    {			/* immC declaration */
		(yyval.sym)   = (yyvsp[(1) - (1)].sym);
		iFunSyText = 0;
	    }
    break;

  case 17:
/* Line 1792 of yacc.c  */
#line 300 "comp.y"
    {			/* " new style for immac which can generate ,; */
		(yyval.sym)   = (yyvsp[(1) - (2)].sym);
		iFunSyText = 0;
	    }
    break;

  case 18:
/* Line 1792 of yacc.c  */
#line 304 "comp.y"
    {			/* declare and initialise an immC array */
		(yyval.sym)   = (yyvsp[(1) - (1)].sym);
		iFunSyText = 0;
	    }
    break;

  case 19:
/* Line 1792 of yacc.c  */
#line 308 "comp.y"
    {			/* " new style for immac which can generate ,; */
		(yyval.sym)   = (yyvsp[(1) - (2)].sym);
		iFunSyText = 0;
	    }
    break;

  case 20:
/* Line 1792 of yacc.c  */
#line 312 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 21:
/* Line 1792 of yacc.c  */
#line 313 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (2)].sym); }
    break;

  case 22:
/* Line 1792 of yacc.c  */
#line 314 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 23:
/* Line 1792 of yacc.c  */
#line 315 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (2)].sym); }
    break;

  case 24:
/* Line 1792 of yacc.c  */
#line 316 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 25:
/* Line 1792 of yacc.c  */
#line 317 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (2)].sym); }
    break;

  case 26:
/* Line 1792 of yacc.c  */
#line 321 "comp.y"
    { (yyval.sym).v =  0; }
    break;

  case 27:
/* Line 1792 of yacc.c  */
#line 322 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 28:
/* Line 1792 of yacc.c  */
#line 323 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 29:
/* Line 1792 of yacc.c  */
#line 324 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 30:
/* Line 1792 of yacc.c  */
#line 325 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 31:
/* Line 1792 of yacc.c  */
#line 329 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 32:
/* Line 1792 of yacc.c  */
#line 330 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 33:
/* Line 1792 of yacc.c  */
#line 331 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 34:
/* Line 1792 of yacc.c  */
#line 335 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 35:
/* Line 1792 of yacc.c  */
#line 336 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 36:
/* Line 1792 of yacc.c  */
#line 340 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 37:
/* Line 1792 of yacc.c  */
#line 341 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 38:
/* Line 1792 of yacc.c  */
#line 345 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 39:
/* Line 1792 of yacc.c  */
#line 346 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 40:
/* Line 1792 of yacc.c  */
#line 347 "comp.y"
    {			/* numeric constant */
		(yyval.sym).f = (yyvsp[(1) - (1)].vai).f; (yyval.sym).l = (yyvsp[(1) - (1)].vai).l;
		(yyval.sym).v = 0;				/* no node, do not need value */
	    }
    break;

  case 41:
/* Line 1792 of yacc.c  */
#line 354 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 42:
/* Line 1792 of yacc.c  */
#line 355 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 43:
/* Line 1792 of yacc.c  */
#line 359 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 44:
/* Line 1792 of yacc.c  */
#line 360 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 45:
/* Line 1792 of yacc.c  */
#line 364 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 46:
/* Line 1792 of yacc.c  */
#line 365 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 47:
/* Line 1792 of yacc.c  */
#line 369 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 48:
/* Line 1792 of yacc.c  */
#line 370 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 49:
/* Line 1792 of yacc.c  */
#line 374 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 50:
/* Line 1792 of yacc.c  */
#line 375 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 51:
/* Line 1792 of yacc.c  */
#line 452 "comp.y"
    {
		unsigned int usetype, use;
		(yyval.sym) = (yyvsp[(1) - (2)].sym);
		use = (yyvsp[(1) - (2)].sym).v->ftype;			/* 0=no or 1=use */
		assert(use <= 1);
		usetype = (yyvsp[(2) - (2)].sym).v->ftype;			/* 0=alias 1=strict 2=list */
		assert(usetype < MAXUSETYPE);
		if (use) {
		    iC_uses |= 1<<usetype;		/* set iC_Aflag or iC_Sflag or iC_Zflag */
		} else {
		    iC_uses &= ~(1<<usetype);		/* reset iC_Aflag or iC_Sflag or iC_Zflag */
		}
		blockUnblockListing();			/* block or unblock listing depending on iC_Zflag */
	    }
    break;

  case 52:
/* Line 1792 of yacc.c  */
#line 466 "comp.y"
    {
		unsigned int usetype, use;
		(yyval.sym) = (yyvsp[(1) - (3)].sym);
		use = (yyvsp[(1) - (3)].sym).v->ftype;			/* 0=no or 1=use */
		assert(use <= 1);
		usetype = (yyvsp[(3) - (3)].sym).v->ftype;			/* 0=alias 1=strict 2=list */
		assert(usetype < MAXUSETYPE);
		if (use) {
		    iC_uses |= 1<<usetype;		/* set iC_Aflag or iC_Sflag or unblock list output */
		} else {
		    iC_uses &= ~(1<<usetype);		/* reset iC_Aflag or iC_Sflag or block list output */
		}
		blockUnblockListing();
	    }
    break;

  case 53:
/* Line 1792 of yacc.c  */
#line 521 "comp.y"
    {
		if ((yyvsp[(2) - (2)].sym).v->ftype == UDFA) {		/* if not previously declared as imm */
		    (yyvsp[(2) - (2)].sym).v->type = CWORD;			/* no longer an imm variable */
		    (yyvsp[(2) - (2)].sym).v->u_val = CNAME;		/* yacc type */
		    (yyvsp[(2) - (2)].sym).v->v_cnt = (unsigned int)-1;	/* mark as variable */
		    (yyval.sym).v = (yyvsp[(2) - (2)].sym).v;
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) {
			fprintf(iC_outFP, "extCdecl: %s is a variable\n", (yyvsp[(2) - (2)].sym).v->name);
			fflush(iC_outFP);
		    }
#endif
		} else {
		    if (iC_Sflag) {
			ierror("strict: erroneous declaration of a C variable that was imm:", (yyvsp[(2) - (2)].sym).v->name);
		    } else {
			warning("erroneous declaration of a C variable that was imm:", (yyvsp[(2) - (2)].sym).v->name);
		    }
		    (yyval.sym).v = 0;
		}
	    }
    break;

  case 54:
/* Line 1792 of yacc.c  */
#line 542 "comp.y"
    {
		(yyvsp[(2) - (5)].sym).v->type = CWORD;			/* no longer an imm variable */
		(yyvsp[(2) - (5)].sym).v->u_val = CNAME;			/* yacc type */
		(yyvsp[(2) - (5)].sym).v->v_cnt = (unsigned int)((yyvsp[(4) - (5)].vai).v);	/* parameter count */
		(yyval.sym).v = (yyvsp[(2) - (5)].sym).v;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "extCdecl: %s has %d parameters\n", (yyvsp[(2) - (5)].sym).v->name, (int)(yyvsp[(4) - (5)].vai).v);
		    fflush(iC_outFP);
		}
#endif
	    }
    break;

  case 55:
/* Line 1792 of yacc.c  */
#line 554 "comp.y"
    {
		(yyvsp[(2) - (6)].sym).v->type = CWORD;			/* no longer an imm variable */
		(yyvsp[(2) - (6)].sym).v->u_val = CNAME;			/* yacc type */
		(yyvsp[(2) - (6)].sym).v->v_cnt = (yyvsp[(4) - (6)].vai).v;			/* parameter count */
		(yyval.sym).v = (yyvsp[(2) - (6)].sym).v;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "extCdecl: %s has %d parameters\n", (yyvsp[(2) - (6)].sym).v->name, (int)(yyvsp[(4) - (6)].vai).v);
		    fflush(iC_outFP);
		}
#endif
		yyerrok;
	    }
    break;

  case 56:
/* Line 1792 of yacc.c  */
#line 567 "comp.y"
    {
		if (iC_Sflag) {
		    ierror("strict: erroneous re-declaration of a C function - no ( parameters ):", (yyvsp[(2) - (2)].sym).v->name);
		} else {
		    warning("erroneous re-declaration of a C function - no ( parameters ):", (yyvsp[(2) - (2)].sym).v->name);
		}
		(yyval.sym).v = 0;
	    }
    break;

  case 57:
/* Line 1792 of yacc.c  */
#line 575 "comp.y"
    {
		if ((yyvsp[(2) - (5)].sym).v->v_cnt != (yyvsp[(4) - (5)].vai).v) {
		    char	tempBuf[TSIZE];
		    snprintf(tempBuf, TSIZE, "%s %d (%d)", (yyvsp[(2) - (5)].sym).v->name, (int)(yyvsp[(4) - (5)].vai).v, (int)(yyvsp[(2) - (5)].sym).v->v_cnt);
		    if (iC_Sflag) {
			ierror("strict: parameter count does not match a previous C function declaration:", tempBuf);
		    } else {
			warning("parameter count does not match a previous C function declaration - ignore:", tempBuf);
		    }
		}
		(yyval.sym).v = (yyvsp[(2) - (5)].sym).v;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "extCdecl: %s has %d parameters\n", (yyvsp[(2) - (5)].sym).v->name, (int)(yyvsp[(4) - (5)].vai).v);
		    fflush(iC_outFP);
		}
#endif
	    }
    break;

  case 58:
/* Line 1792 of yacc.c  */
#line 593 "comp.y"
    {
		if ((yyvsp[(2) - (6)].sym).v->v_cnt != (yyvsp[(4) - (6)].vai).v) {
		    char	tempBuf[TSIZE];
		    snprintf(tempBuf, TSIZE, "%s %d (%d)", (yyvsp[(2) - (6)].sym).v->name, (int)(yyvsp[(4) - (6)].vai).v, (int)(yyvsp[(2) - (6)].sym).v->v_cnt);
		    if (iC_Sflag) {
			ierror("strict: parameter count does not match a previous C function declaration:", tempBuf);
		    } else {
			warning("parameter count does not match a previous C function declaration - ignore:", tempBuf);
		    }
		}
		(yyval.sym).v = (yyvsp[(2) - (6)].sym).v;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "extCdecl: %s has %d parameters\n", (yyvsp[(2) - (6)].sym).v->name, (int)(yyvsp[(4) - (6)].vai).v);
		    fflush(iC_outFP);
		}
#endif
		yyerrok;
	    }
    break;

  case 59:
/* Line 1792 of yacc.c  */
#line 615 "comp.y"
    {
		if ((yyvsp[(2) - (2)].sym).v->ftype != ARITH) {
		    if ((yyvsp[(2) - (2)].sym).v->ftype == GATE) {
			warning("type of an extern C declaration is bit not int - ignore:", (yyvsp[(1) - (2)].sym).v->name);
		    } else
		    if (iC_Sflag) {
			ierror("strict: incompatible type for an extern C declaration:", (yyvsp[(2) - (2)].sym).v->name);
		    } else {
			warning("incompatible type for an extern C declaration:", (yyvsp[(2) - (2)].sym).v->name);
		    }
		}
		stype.ftype = ARITH;
		stype.type  = ARN;
		stype.em    |= EM;		/* set em for extern declaration */
		stype.fm    = 0;
		(yyval.typ).v = stype;
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "extCdeclHead: EXTERN TYPE", &(yyval.typ));
#endif
	    }
    break;

  case 60:
/* Line 1792 of yacc.c  */
#line 635 "comp.y"
    {
		(yyval.typ).v = stype;	/* first TYPE */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "extCdeclHead: extCdecl", &(yyval.typ));
#endif
	    }
    break;

  case 61:
/* Line 1792 of yacc.c  */
#line 643 "comp.y"
    { (yyval.vai).v =  0; }
    break;

  case 62:
/* Line 1792 of yacc.c  */
#line 644 "comp.y"
    { (yyval.vai).v =  0; }
    break;

  case 63:
/* Line 1792 of yacc.c  */
#line 645 "comp.y"
    { (yyval.vai)   = (yyvsp[(1) - (1)].vai); }
    break;

  case 64:
/* Line 1792 of yacc.c  */
#line 648 "comp.y"
    { (yyval.vai).v = 1; }
    break;

  case 65:
/* Line 1792 of yacc.c  */
#line 649 "comp.y"
    { (yyval.vai).v = (yyvsp[(1) - (3)].vai).v + 1; }
    break;

  case 66:
/* Line 1792 of yacc.c  */
#line 652 "comp.y"
    {
		if ((yyvsp[(1) - (2)].sym).v->ftype != ARITH) {
		    if ((yyvsp[(1) - (2)].sym).v->ftype == GATE) {
			warning("parameter type in an extern C function declaration is bit not int - ignore:", (yyvsp[(1) - (2)].sym).v->name);
		    } else
		    if (iC_Sflag) {
			ierror("strict: incompatible parameter type in an extern C function declaration:", (yyvsp[(1) - (2)].sym).v->name);
		    } else {
			warning("incompatible parameter type in an extern C function declaration:", (yyvsp[(1) - (2)].sym).v->name);
		    }
		}
		(yyval.list).v = 0;
	    }
    break;

  case 67:
/* Line 1792 of yacc.c  */
#line 667 "comp.y"
    { (yyval.sym).v = 0; }
    break;

  case 68:
/* Line 1792 of yacc.c  */
#line 668 "comp.y"
    {
		if ((yyvsp[(1) - (1)].sym).v->ftype == UDFA) {	/* if not previously declared as imm */
		    uninstall((yyvsp[(1) - (1)].sym).v);		/* delete dummy Symbol */
		}
		(yyval.sym).v = 0;
	    }
    break;

  case 69:
/* Line 1792 of yacc.c  */
#line 674 "comp.y"
    { (yyval.sym).v = 0; }
    break;

  case 70:
/* Line 1792 of yacc.c  */
#line 756 "comp.y"
    {
		(yyval.sym).v = (yyvsp[(2) - (2)].sym).v;
		(yyval.sym).v->ftype = (yyvsp[(1) - (2)].typ).v.ftype;
		(yyval.sym).v->type  = (yyvsp[(1) - (2)].typ).v.type;
		assert(((yyvsp[(1) - (2)].typ).v.em & (EM|EX)) == (EM|EX));	/* has been set in extDeclHead */
		(yyval.sym).v->em    |= EM|EX;		/* set em for extern declaration */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    pd("extDecl", (yyval.sym).v, (yyvsp[(1) - (2)].typ).v, (yyvsp[(2) - (2)].sym).v);
		}
#endif
	    }
    break;

  case 71:
/* Line 1792 of yacc.c  */
#line 768 "comp.y"
    {		/* LVAR AVAR LVARC AVARC LOUT AOUT CVAR TVAR Constant */
		int		typ1;
		int		typ2;
		int		ftyp;
		char *		cp;
		Symbol *	sp;
		typ1  = (yyvsp[(1) - (2)].typ).v.type;		/* UDF for all TYPEs except ARNC LOGC INPX INPW */
		sp = (yyval.sym).v = (yyvsp[(2) - (2)].sym).v;
		if (sp) {
		    assert(((yyvsp[(1) - (2)].typ).v.em & (EM|EX)) == (EM|EX));	/* has been set in extDeclHead */
		    if (((typ2 = sp->type) == INPW || typ2 == INPX) &&
			(typ1 == ARNC || typ1 == LOGC) &&
			sp->type != ERR) {
			ierror("extern immC declaration of an input variable is invalid:", sp->name);
		    } else
		    if (sp->ftype != (ftyp = (yyvsp[(1) - (2)].typ).v.ftype) && sp->type != ERR) {
			ierror("extern declaration does not match previous declaration:", sp->name);
			sp->type = ERR;	/* cannot execute properly */
		    }
		    if (iFunSymExt && (cp = strchr(sp->name, '@'))) {
			warning("extern declaration of internal function variable - ignored:", cp+1);
		    } else
		    if (typ2 == UDF ||			/* extern of unused imm or QXx.y QBz */
			typ2 == INPW || typ2 == INPX ||	/* or IXx.y IBz */
			((sp->em & EM) && typ2 != ERR)) {	/* or prev extern but not ERROR */
			if (typ2 != INPW && typ2 != INPX) {
			    if ((sp->em & EO) == 0) {
				sp->ftype = ftyp;	/* adjust ftype from extern type unless QXx.y QBz */
			    }
			    sp->type  = typ1;		/* adjust type UDF unless ARNC or LOGC */
			}
			sp->em    |= EM|EX;		/* set em for extern declaration */
		    } else
		    if (iFunSymExt) {
			ierror("extern declaration in function definition after assignment:", sp->name);
			sp->type = ERR;	/* stop use as a statement in function */
			sp->em   |= EM|EX;
		    } else if (typ2 == ARNC || typ2 == LOGC) {
			warning("extern immC declaration after definition - ignored:", sp->name);
		    }
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) {
			pd("extDecl", (yyval.sym).v, (yyvsp[(1) - (2)].typ).v, (yyvsp[(2) - (2)].sym).v);
		    }
#endif
		} else {
		    ierror("direct extern declaration of a number is illegal:", (yyvsp[(2) - (2)].sym).f);
		}
	    }
    break;

  case 72:
/* Line 1792 of yacc.c  */
#line 819 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 73:
/* Line 1792 of yacc.c  */
#line 820 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 74:
/* Line 1792 of yacc.c  */
#line 824 "comp.y"
    {
		int		typ;
		int		ftyp;
		ftyp = (yyvsp[(3) - (3)].sym).v->ftype;		/* ARITH GATE CLCKL TIMRL */
		if ((yyvsp[(2) - (3)].sym).v->ftype == 1) {		/* ftype field in IMMC for immC */
		    if (ftyp == ARITH) {
			typ = ARNC;		/* immC int */
		    } else
		    if (ftyp == GATE) {
			typ = LOGC;		/* immC bit */
		    } else {
			warning("extern declaration of an immC type other than bit or int - ignore:", (yyvsp[(3) - (3)].sym).v->name);
			goto extImmType;
		    }
		} else {			/* immT is imm */
		  extImmType:
		    if (ftyp >= CLCKL) {	/* check that CLCKL TIMRL */
			ftyp -= CLCKL - CLCK;	/* and CLCK TIMR are adjacent */
		    }
		    typ = iC_types[ftyp];	/* ARN OR CLK TIM */
		}
		stype.ftype = (yyvsp[(3) - (3)].sym).v->ftype;	/* ftyp has changed for CLCKL and TIMRL */
		stype.type  = typ;
		stype.em    |= EM|EX;		/* set em for extern declaration */
		stype.fm    = 0;
		(yyval.typ).v = stype;
		iFunSyText = 0;			/* no function symbols for extern */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "extDeclHead: EXTERN immT TYPE", &(yyval.typ));
#endif
	    }
    break;

  case 75:
/* Line 1792 of yacc.c  */
#line 855 "comp.y"
    {
		(yyval.typ).v = stype;			/* first TYPE */
		iFunSyText = 0;			/* no function symbols for extern */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "extDeclHead: extDecl ,", &(yyval.typ));
#endif
	    }
    break;

  case 76:
/* Line 1792 of yacc.c  */
#line 862 "comp.y"
    {
		(yyval.typ).v = stype;			/* first TYPE */
		iFunSyText = 0;			/* no function symbols for extern */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "extDeclHead: extimmCarray ,", &(yyval.typ));
#endif
	    }
    break;

  case 77:
/* Line 1792 of yacc.c  */
#line 915 "comp.y"
    {
		Symbol *	sp;
		(yyval.sym).f = (yyvsp[(1) - (2)].typ).f; (yyval.sym).l = (yyvsp[(2) - (2)].sym).l;
		sp = (yyval.sym).v = (yyvsp[(2) - (2)].sym).v;
		sp->ftype = (yyvsp[(1) - (2)].typ).v.ftype;		/* bit int clock timer */
		sp->type  = (yyvsp[(1) - (2)].typ).v.type;
		assert(((yyvsp[(1) - (2)].typ).v.em & EM) == 0);	/* has been cleared in declHead */
		sp->em    &= ~EM;		/* clear only bit EM here, leave bit EX */
		assert(sp->type == UDF);
		if (iFunSymExt) {
		    char *	cp;
		    Symbol *	sp1;
		    cp = strchr(sp->name, '@'); /* locate original extension */
		    assert(cp && isprint(cp[1])); /* extension must be at least 1 character */
		    if ((sp1 = lookup(++cp)) != 0 && (sp1->em & EM)) {
			warning("declaration of an extern variable in a function - ignored:", cp);
		    }
		    collectStatement(sp);
		    iFunSyText = 0;		/* no more function symbols */
		}
#if YYDEBUG
		Symbol t = *(sp);
		if ((iC_debug & 0402) == 0402) pd("decl", sp, (yyvsp[(1) - (2)].typ).v, &t);
#endif
	    }
    break;

  case 78:
/* Line 1792 of yacc.c  */
#line 940 "comp.y"
    {		/* LVAR AVAR LOUT AOUT CVAR TVAR Constant */
		int		typ1;		/* not LVARC AVARC */
		int		typ2;
		int		ftyp;
		Symbol *	sp;
		(yyval.sym).f = (yyvsp[(1) - (2)].typ).f; (yyval.sym).l = (yyvsp[(2) - (2)].sym).l;
		ftyp = (yyvsp[(1) - (2)].typ).v.ftype;		/* ARITH GATE CLCKL TIMRL */
		typ1  = (yyvsp[(1) - (2)].typ).v.type;		/* UDF for all TYPEs except INPX INPW */
		sp = (yyval.sym).v = (yyvsp[(2) - (2)].sym).v;
		if (sp) {
		    assert(typ1 != LOGC && typ1 != ARNC);
		    if ((typ2 = sp->type) != ERR && sp->ftype != ftyp) {
			ierror("declaration does not match previous declaration:", sp->name);
			if (! iFunSymExt) (yyvsp[(2) - (2)].sym).v->type = ERR;	/* cannot execute properly */
		    } else
		    if ((sp->em & EM) || typ2 == UDF) {
			if ((sp->em & EO) == 0) {
			    sp->ftype = ftyp;	/* adjust ftype from extern type unless QXx.y QBz */
			}
			assert(typ1 == UDF);	/* UDF for all TYPEs */
			if (typ2 != INPW && typ2 != INPX) {
			    sp->type = typ1;	/* fix type */
			}
			sp->em &= ~EM;		/* no longer extern - leave EX */
		    }
#if YYDEBUG
		    Symbol t = *(sp);
		    if ((iC_debug & 0402) == 0402) pd("decl", sp, (yyvsp[(1) - (2)].typ).v, &t);
#endif
		} else {
		    ierror("direct declaration of a number is illegal:", (yyval.sym).f);
		}
		iFunSyText = 0;			/* no more function symbols */
	    }
    break;

  case 79:
/* Line 1792 of yacc.c  */
#line 974 "comp.y"
    {	/* ARNC LOGC */
		Symbol *	sp;		/* not LVAR AVAR LOUT AOUT CVAR TVAR Constant */
		(yyval.sym).f = (yyvsp[(1) - (2)].typ).f; (yyval.sym).l = (yyvsp[(2) - (2)].sym).l;
		sp = (yyval.sym).v = (yyvsp[(2) - (2)].sym).v;
		sp->ftype = (yyvsp[(1) - (2)].typ).v.ftype;		/* ARITH GATE */
		ierror("declaration does not match previous extern immC declaration:", sp->name);
	    }
    break;

  case 80:
/* Line 1792 of yacc.c  */
#line 1035 "comp.y"
    {
		Symbol *	sp;
		(yyval.sym).f = (yyvsp[(1) - (2)].typ).f; (yyval.sym).l = (yyvsp[(2) - (2)].sym).l;
		sp = (yyval.sym).v = (yyvsp[(2) - (2)].sym).v;
		sp->ftype = (yyvsp[(1) - (2)].typ).v.ftype;		/* bit int */
		sp->type  = (yyvsp[(1) - (2)].typ).v.type;
		assert(((yyvsp[(1) - (2)].typ).v.em & EM) == 0);	/* has been cleared in declCHead */
		sp->em    &= ~EM;		/* clear only bit EM here, leave bit EX */
		assert(sp->type == ARNC || sp->type == LOGC);
		if (iFunSymExt) {
		    char *	cp;
		    Symbol *	sp1;
		    cp = strchr(sp->name, '@'); /* locate original extension */
		    assert(cp && isprint(cp[1])); /* extension must be at least 1 character */
		    if ((sp1 = lookup(++cp)) != 0 && (sp1->em & EM)) {
			warning("declaration of an extern variable in a function - ignored:", cp);
		    }
		    collectStatement(sp);
		    iFunSyText = 0;		/* no more function symbols */
		}
#if YYDEBUG
		Symbol t = *(sp);
		if ((iC_debug & 0402) == 0402) pd("declC: declCHead UNDEF", sp, (yyvsp[(1) - (2)].typ).v, &t);
#endif
	    }
    break;

  case 81:
/* Line 1792 of yacc.c  */
#line 1060 "comp.y"
    {		/* LVARC AVARC LOUT AOUT */
		int		typ2;		/* not LVAR AVAR CVAR TVAR Constant */
		int		typ1;
		int		ftyp;
		Symbol *	sp;
		(yyval.sym).f = (yyvsp[(1) - (2)].typ).f; (yyval.sym).l = (yyvsp[(2) - (2)].sym).l;
		ftyp = (yyvsp[(1) - (2)].typ).v.ftype;		/* ARITH GATE CLCKL TIMRL */
		typ1  = (yyvsp[(1) - (2)].typ).v.type;		/* UDF for all TYPEs except ARNC LOGC INPX INPW */
		sp = (yyval.sym).v = (yyvsp[(2) - (2)].sym).v;
		assert(sp && (typ1 == LOGC || typ1 == ARNC));
		if ((typ2 = sp->type) == INPW || typ2 == INPX) {
		    ierror("immC declaration of an input variable is invalid:", sp->name);
		    sp->em |= ED;		/* do not define */
		} else
		if (typ2 != ERR && sp->ftype != ftyp) {
		    ierror("declaration does not match previous declaration:", sp->name);
		    sp->em |= ED;		/* do not define */
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		} else
		if ((sp->em & EM) || sp->type == UDF) {
		    if ((sp->em & EO) == 0) {
			sp->ftype = ftyp;	/* adjust ftype from extern type unless QXx.y QBz */
		    }
		    sp->type = typ1;	/* ARNC or LOGC */
		    sp->em &= ~EM;		/* no longer extern - leave EX */
		} else if (sp->type == ARNC || sp->type == LOGC) {
		    warning("immC declaration after definition - ignored:", sp->name);
		}
#if YYDEBUG
		Symbol t = *(sp);
		if ((iC_debug & 0402) == 0402) pd("declC: declCHead dCVariable", sp, (yyvsp[(1) - (2)].typ).v, &t);
#endif
		iFunSyText = 0;			/* no more function symbols */
	    }
    break;

  case 82:
/* Line 1792 of yacc.c  */
#line 1094 "comp.y"
    {		/* LVAR AVAR CVAR TVAR Constant */
		Symbol *	sp;		/* not LVARC AVARC LOUT AOUT */
		int		typ2;
		(yyval.sym).f = (yyvsp[(1) - (2)].typ).f; (yyval.sym).l = (yyvsp[(2) - (2)].sym).l;
		sp = (yyval.sym).v = (yyvsp[(2) - (2)].sym).v;
		sp->ftype = (yyvsp[(1) - (2)].typ).v.ftype;		/* ARITH GATE */
		if (((typ2 = sp->type) == INPW || typ2 == INPX)) {
		    ierror("immC declaration of an input variable is invalid:", sp->name);
		} else {
		    ierror("declaration does not match previous extern imm declaration:", sp->name);
		}
		sp->em |= ED;			/* do not define */
	    }
    break;

  case 83:
/* Line 1792 of yacc.c  */
#line 1109 "comp.y"
    {		/* imm declaration */
		(yyval.sym)   = (yyvsp[(1) - (1)].sym);			/* not strictly necessary - stype holds data */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
    break;

  case 84:
/* Line 1792 of yacc.c  */
#line 1115 "comp.y"
    {		/* declaration data assignment */
		(yyval.sym)   = (yyvsp[(1) - (1)].sym);			/* not strictly necessary - stype holds data */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
    break;

  case 85:
/* Line 1792 of yacc.c  */
#line 1121 "comp.y"
    {		/* declaration clock assignment */
		(yyval.sym)   = (yyvsp[(1) - (1)].sym);			/* not strictly necessary - stype holds data */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
    break;

  case 86:
/* Line 1792 of yacc.c  */
#line 1127 "comp.y"
    {		/* declaration timer assignment */
		(yyval.sym)   = (yyvsp[(1) - (1)].sym);			/* not strictly necessary - stype holds data */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
	    }
    break;

  case 87:
/* Line 1792 of yacc.c  */
#line 1136 "comp.y"
    {		/* immC declaration */
		(yyval.sym)   = (yyvsp[(1) - (1)].sym);			/* not strictly necessary - stype holds data */
		listGenOut((yyvsp[(1) - (1)].sym).v, 1, 0);		/* list immC node without initialisation */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "immCdecl: declC", &(yyval.sym));
#endif
	    }
    break;

  case 88:
/* Line 1792 of yacc.c  */
#line 1146 "comp.y"
    {		/* make immC variable ALIAS of return from const expression */
		List_e *	lp;
		Symbol *	sp;
		(yyval.sym)   = (yyvsp[(1) - (3)].sym);			/* not strictly necessary - stype holds data */
		if ((lp = (yyvsp[(3) - (3)].list).v) != 0 &&
		    (sp = lp->le_sym) != 0) {
		    assert((yyvsp[(1) - (3)].sym).v->type == ARNC || (yyvsp[(1) - (3)].sym).v->type == LOGC);
		    if (sp->type == ARNC &&		/* TODO what about LOGC */
			(sp->em & EI) != 0 &&
			sp->u_blist == 0) {		/* this ensures assignment will be an ALIAS */
			(yyvsp[(1) - (3)].sym).v->type = UDF;		/* to allow ALIAS assignment */
			if (((yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), 0)) == 0) YYERROR;
		    } else if (sp->type == NCONST) {
			Valp	v;			/* initialisation similar to constExpr */
			v.lfl = 0;
			v.nuv = sp->u_val;		/* numeric value returned by function */
			listGenOut((yyvsp[(1) - (3)].sym).v, -1, &v);	/* list immC node with initialiser value */
		    } else {
			goto NotConstFunReturn;
		    }
		} else {
		  NotConstFunReturn:
		    listGenOut((yyvsp[(1) - (3)].sym).v, 1, 0);		/* list immC node without initialisation */
		    warning("immC initialiser not a constant function return:", (yyvsp[(1) - (3)].sym).v->name);
		}
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "immCdecl: declC = iFunCall", &(yyval.sym));
#endif
	    }
    break;

  case 89:
/* Line 1792 of yacc.c  */
#line 1178 "comp.y"
    {
		(yyval.sym)   = (yyvsp[(1) - (3)].sym);			/* not strictly necessary - stype holds data */
		listGenOut((yyvsp[(1) - (3)].sym).v, -1, &((yyvsp[(3) - (3)].vap).v));	/* list immC node with initialiser value */
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "immCdecl: declC = constExpr", &(yyval.sym));
#endif
	    }
    break;

  case 90:
/* Line 1792 of yacc.c  */
#line 1191 "comp.y"
    {
		(yyval.typ).f = (yyval.typ).l = (yyvsp[(2) - (2)].sym).l;		/* do not include in expression string */
		assert((yyvsp[(1) - (2)].sym).v->ftype == 0);	/* imm in init.c */
		stype.ftype = (yyvsp[(2) - (2)].sym).v->ftype;	/* ARITH GATE CLCKL TIMRL */
		stype.type  = UDF;
		stype.em &= ~EM;		/* no longer extern - leave EX if it was set */
		stype.fm = 0;
		(yyval.typ).v = stype;
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "declHead: IMM TYPE", &(yyval.typ));
#endif
	    }
    break;

  case 91:
/* Line 1792 of yacc.c  */
#line 1206 "comp.y"
    {
		(yyval.typ).f = (yyvsp[(1) - (2)].sym).f; (yyval.typ).l = (yyvsp[(1) - (2)].sym).l;
		(yyval.typ).v = stype;			/* first TYPE */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "declHead: immDecl ,", &(yyval.typ));
#endif
	    }
    break;

  case 92:
/* Line 1792 of yacc.c  */
#line 1216 "comp.y"
    {
		int		typ;
		int		ftyp;
		(yyval.typ).f = (yyval.typ).l = (yyvsp[(2) - (2)].sym).l;		/* do not include in expression string */
		ftyp = (yyvsp[(2) - (2)].sym).v->ftype;		/* ARITH GATE CLCKL TIMRL */
		assert((yyvsp[(1) - (2)].sym).v->ftype == 1);	/* immC in init.c */
		if (ftyp == ARITH) {
		    typ = ARNC;		/* immC int */
		} else
		if (ftyp == GATE) {
		    typ = LOGC;		/* immC bit */
		} else {
		    warning("declaration of an immC type other than bit or int - ignore:", (yyvsp[(2) - (2)].sym).v->name);
		    typ = UDF;
		}
		stype.ftype = ftyp;
		stype.type  = typ;
		stype.em &= ~EM;		/* no longer extern - leave EX if it was set */
		stype.fm = 0;
		(yyval.typ).v = stype;
		if (iFunSymExt) {
		    iFunSyText = iFunBuffer;	/* expecting a new function symbol */
		}
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "declHead: IMMC TYPE", &(yyval.typ));
#endif
	    }
    break;

  case 93:
/* Line 1792 of yacc.c  */
#line 1243 "comp.y"
    {
		(yyval.typ).f = (yyvsp[(1) - (2)].sym).f; (yyval.typ).l = (yyvsp[(1) - (2)].sym).l;
		(yyval.typ).v = stype;			/* first TYPE */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "declCHead: immCDecl ,", &(yyval.typ));
#endif
	    }
    break;

  case 94:
/* Line 1792 of yacc.c  */
#line 1250 "comp.y"
    {
		(yyval.typ).f = (yyvsp[(1) - (2)].sym).f; (yyval.typ).l = (yyvsp[(1) - (2)].sym).l;
		(yyval.typ).v = stype;			/* first TYPE */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "declCHead: immCarray ,", &(yyval.typ));
#endif
	    }
    break;

  case 95:
/* Line 1792 of yacc.c  */
#line 1268 "comp.y"
    {		/* dasgn is NOT an aexpr */
		char *	name;
		int	ioTyp;
		(yyval.sym).f = (yyvsp[(1) - (3)].sym).f; (yyval.sym).l = (yyvsp[(3) - (3)].list).l;
		if ((yyvsp[(1) - (3)].sym).v) {
		    assert((yyvsp[(1) - (3)].sym).v->type != ARNC && (yyvsp[(1) - (3)].sym).v->type != LOGC);
		    if ((yyvsp[(1) - (3)].sym).v->em & EO) {	/* QXx.y QBz etc */
			name = (yyvsp[(1) - (3)].sym).v->name;
			assert(name[0] == 'Q');
			switch (name[1]) {
			case 'X':
			    ioTyp = OUTX;
			    break;
			case 'B':
			case 'W':
			case 'L':
			case 'H':
			    ioTyp = OUTW;
			    break;
			default:
			    assert(0);		/* illegal Q variable */
			    break;
			}
		    } else {
			ioTyp = 0;		/* flags that no I/O is generated */
		    }
		    if (((yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), ioTyp)) == 0) YYERROR;
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) pu(SYM, "dasgn: decl = aexpr", &(yyval.sym));
#endif
		}
	    }
    break;

  case 96:
/* Line 1792 of yacc.c  */
#line 1308 "comp.y"
    {		/* asgn is an aexpr */
		Symbol *	sp;
		(yyval.sym).f = (yyvsp[(1) - (3)].sym).f; (yyval.sym).l = (yyvsp[(3) - (3)].list).l;
		sp = (yyvsp[(3) - (3)].list).v->le_sym;
		assert(sp && (sp->ftype < MIN_ACT && sp->ftype != UDFA));	/* GATEX GATE ARITH */
		(yyvsp[(1) - (3)].sym).v->ftype = sp->ftype;	/* not strict - implicitly declared from aexpr */
		if (((yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), 0)) == 0) YYERROR;
		if (iC_Sflag) {
		    ierror("strict: assignment to an undeclared imm variable:", (yyvsp[(1) - (3)].sym).v->name);
		    (yyvsp[(1) - (3)].sym).v->type = ERR;		/* cannot execute properly */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "asgn: UNDEF", &(yyval.sym));
#endif
	    }
    break;

  case 97:
/* Line 1792 of yacc.c  */
#line 1323 "comp.y"
    {
		(yyval.sym).f = (yyvsp[(1) - (3)].sym).f; (yyval.sym).l = (yyvsp[(3) - (3)].list).l;
		if (((yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), 0)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "asgn: LVAR", &(yyval.sym));
#endif
	    }
    break;

  case 98:
/* Line 1792 of yacc.c  */
#line 1330 "comp.y"
    {
		Symbol *	sp;
		(yyval.sym).f = (yyvsp[(1) - (3)].sym).f; (yyval.sym).l = (yyvsp[(3) - (3)].list).l;
		if ((yyvsp[(3) - (3)].list).v != NULL &&
		    (sp = (yyvsp[(3) - (3)].list).v->le_sym) != NULL &&
		    sp->type < MAX_GT &&
		    sp->u_blist == NULL) {
		    assert((yyvsp[(1) - (3)].sym).v->ftype == ARITH);
		    sp->ftype = ARITH;		/* aexpr has never been used - change ftype */
		}
		if (((yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), 0)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "asgn: AVAR", &(yyval.sym));
#endif
	    }
    break;

  case 99:
/* Line 1792 of yacc.c  */
#line 1345 "comp.y"
    {
		(yyval.sym).f = (yyvsp[(1) - (3)].sym).f; (yyval.sym).l = (yyvsp[(3) - (3)].list).l;
		if (((yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), OUTX)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "asgn: LOUT", &(yyval.sym));
#endif
	    }
    break;

  case 100:
/* Line 1792 of yacc.c  */
#line 1352 "comp.y"
    {
		Symbol *	sp;
		(yyval.sym).f = (yyvsp[(1) - (3)].sym).f; (yyval.sym).l = (yyvsp[(3) - (3)].list).l;
		if ((yyvsp[(3) - (3)].list).v != NULL &&
		    (sp = (yyvsp[(3) - (3)].list).v->le_sym) != NULL &&
		    sp->type < MAX_GT &&
		    sp->u_blist == NULL) {
		    assert((yyvsp[(1) - (3)].sym).v->ftype == ARITH);
		    sp->ftype = ARITH;		/* aexpr has never been used - change ftype */
		}
		if (((yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), OUTW)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "asgn: AOUT", &(yyval.sym));
#endif
	    }
    break;

  case 101:
/* Line 1792 of yacc.c  */
#line 1367 "comp.y"
    {
		(yyval.sym).f = (yyvsp[(1) - (3)].vai).f; (yyval.sym).l = (yyvsp[(3) - (3)].list).l;
		ierror("assignment to a number is illegal:", (yyval.sym).f);
		if (((yyval.sym).v = op_asgn(0, &(yyvsp[(3) - (3)].list), F_ERR)) == 0) YYERROR;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "asgn: Constant (ERR)", &(yyval.sym));
#endif
	    }
    break;

  case 102:
/* Line 1792 of yacc.c  */
#line 1383 "comp.y"
    {
		(yyval.list) = (yyvsp[(1) - (1)].list);
		if ((yyval.list).v != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "aexpr: expr", &(yyval.list));
#endif
	    }
    break;

  case 103:
/* Line 1792 of yacc.c  */
#line 1393 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (1)].sym).f; (yyval.list).l = (yyvsp[(1) - (1)].sym).l;
		(yyval.list).v = sy_push((yyvsp[(1) - (1)].sym).v);
		assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		(yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "aexpr: asgn", &(yyval.list));
#endif
	    }
    break;

  case 104:
/* Line 1792 of yacc.c  */
#line 1410 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (1)].sym).f; (yyval.list).l = (yyvsp[(1) - (1)].sym).l;
		(yyval.list).v = checkDecl((yyvsp[(1) - (1)].sym).v);
		(yyvsp[(1) - (1)].sym).v->ftype = GATE;		/* imm bit default if not declared */
		assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		(yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: UNDEF", &(yyval.list));
#endif
		if (iC_Sflag) {
		    ierror("strict: use of an undeclared imm variable:", (yyvsp[(1) - (1)].sym).v->name);
		    (yyvsp[(1) - (1)].sym).v->type = ERR;		/* cannot execute properly */
		}
	    }
    break;

  case 105:
/* Line 1792 of yacc.c  */
#line 1424 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (1)].vai).f; (yyval.list).l = (yyvsp[(1) - (1)].vai).l;	/* no node, value not used */
		(yyval.list).v = 0;			/* uses TX0.2 in bit expressions */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: Constant", &(yyval.list));
#endif
	    }
    break;

  case 106:
/* Line 1792 of yacc.c  */
#line 1431 "comp.y"
    {		/* LVAR LVARC AVAR AVARC */
		(yyval.list).f = (yyvsp[(1) - (1)].sym).f; (yyval.list).l = (yyvsp[(1) - (1)].sym).l;
		(yyval.list).v = checkDecl((yyvsp[(1) - (1)].sym).v);
		assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		(yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: valueVariable", &(yyval.list));
#endif
	    }
    break;

  case 107:
/* Line 1792 of yacc.c  */
#line 1440 "comp.y"
    {		/* immC array member variable */
		(yyval.list).f = (yyvsp[(1) - (1)].sym).f; (yyval.list).l = (yyvsp[(1) - (1)].sym).l;
		(yyval.list).v = checkDecl((yyvsp[(1) - (1)].sym).v);
		assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		(yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: immCarrayVariable", &(yyval.list));
#endif
	    }
    break;

  case 108:
/* Line 1792 of yacc.c  */
#line 1449 "comp.y"
    {		/* LOUT AOUT */
		(yyval.list).f = (yyvsp[(1) - (1)].sym).f; (yyval.list).l = (yyvsp[(1) - (1)].sym).l;
		(yyval.list).v = sy_push((yyvsp[(1) - (1)].sym).v);
		assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		(yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: outVariable", &(yyval.list));
#endif
	    }
    break;

  case 109:
/* Line 1792 of yacc.c  */
#line 1458 "comp.y"
    {
		(yyval.list) = (yyvsp[(1) - (1)].list);
		if ((yyval.list).v) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: cCall", &(yyval.list));
#endif
	    }
    break;

  case 110:
/* Line 1792 of yacc.c  */
#line 1468 "comp.y"
    {
		(yyval.list) = (yyvsp[(1) - (1)].list);
		if ((yyval.list).v) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: iFunCall", &(yyval.list));
#endif
	    }
    break;

  case 111:
/* Line 1792 of yacc.c  */
#line 1478 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (3)].vai).f; (yyval.list).l = (yyvsp[(3) - (3)].vai).l;
		if (((yyval.list).v = (yyvsp[(2) - (3)].list).v) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: (comma_expr)", &(yyval.list));
#endif
	    }
    break;

  case 112:
/* Line 1792 of yacc.c  */
#line 1491 "comp.y"
    {			/* binary | */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		if (((yyvsp[(1) - (3)].list).v == 0 || (yyvsp[(1) - (3)].list).v->le_sym->ftype == ARITH) &&
		    ((yyvsp[(3) - (3)].list).v == 0 || (yyvsp[(3) - (3)].list).v->le_sym->ftype == ARITH)) {
		    (yyval.list).v = op_push((yyvsp[(1) - (3)].list).v, ARN, (yyvsp[(3) - (3)].list).v);	/* bitwise | */
		} else {
		    lpL = op_adjust(&(yyvsp[(1) - (3)].list));
		    lpR = op_adjust(&(yyvsp[(3) - (3)].list));
		    (yyval.list).v = op_push(lpL, OR, lpR);	/* logical | */
		}
		if ((yyval.list).v) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr | expr", &(yyval.list));
#endif
	    }
    break;

  case 113:
/* Line 1792 of yacc.c  */
#line 1511 "comp.y"
    {			/* binary ^ */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		if (((yyvsp[(1) - (3)].list).v == 0 || (yyvsp[(1) - (3)].list).v->le_sym->ftype == ARITH) &&
		    ((yyvsp[(3) - (3)].list).v == 0 || (yyvsp[(3) - (3)].list).v->le_sym->ftype == ARITH)) {
		    (yyval.list).v = op_push((yyvsp[(1) - (3)].list).v, ARN, (yyvsp[(3) - (3)].list).v);	/* bitwise ^ */
		} else {
		    lpL = op_adjust(&(yyvsp[(1) - (3)].list));
		    lpR = op_adjust(&(yyvsp[(3) - (3)].list));
		    (yyval.list).v = op_push(lpL, XOR, lpR);	/* logical ^ */
		}
		if ((yyval.list).v) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr ^ expr", &(yyval.list));
#endif
	    }
    break;

  case 114:
/* Line 1792 of yacc.c  */
#line 1531 "comp.y"
    {			/* binary & */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		if (((yyvsp[(1) - (3)].list).v == 0 || (yyvsp[(1) - (3)].list).v->le_sym->ftype == ARITH) &&
		    ((yyvsp[(3) - (3)].list).v == 0 || (yyvsp[(3) - (3)].list).v->le_sym->ftype == ARITH)) {
		    (yyval.list).v = op_push((yyvsp[(1) - (3)].list).v, ARN, (yyvsp[(3) - (3)].list).v);	/* bitwise & */
		} else {
		    lpL = op_adjust(&(yyvsp[(1) - (3)].list));
		    lpR = op_adjust(&(yyvsp[(3) - (3)].list));
		    (yyval.list).v = op_push(lpL, AND, lpR);	/* logical & */
		}
		if ((yyval.list).v) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr & expr", &(yyval.list));
#endif
	    }
    break;

  case 115:
/* Line 1792 of yacc.c  */
#line 1551 "comp.y"
    {				/* < <= > >= */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		lpR = op_force((yyvsp[(3) - (3)].list).v, ARITH);
		lpL = op_force((yyvsp[(1) - (3)].list).v, ARITH);
		if (((yyval.list).v = op_push(lpL, ARN, lpR)) != 0) {
		    (yyval.list).v = op_force((yyval.list).v, GATE);	/* default output */
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr CMP expr", &(yyval.list));
#endif
	    }
    break;

  case 116:
/* Line 1792 of yacc.c  */
#line 1566 "comp.y"
    {				/* == != */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		lpR = op_force((yyvsp[(3) - (3)].list).v, ARITH);
		lpL = op_force((yyvsp[(1) - (3)].list).v, ARITH);
		if (((yyval.list).v = op_push(lpL, ARN, lpR)) != 0) {
		    (yyval.list).v = op_force((yyval.list).v, GATE);	/* default output */
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr EQU expr", &(yyval.list));
#endif
	    }
    break;

  case 117:
/* Line 1792 of yacc.c  */
#line 1581 "comp.y"
    {			/* binary * */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		lpR = op_force((yyvsp[(3) - (3)].list).v, ARITH);
		lpL = op_force((yyvsp[(1) - (3)].list).v, ARITH);
		if (((yyval.list).v = op_push(lpL, ARN, lpR)) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr * expr", &(yyval.list));
#endif
	    }
    break;

  case 118:
/* Line 1792 of yacc.c  */
#line 1595 "comp.y"
    {			/* / % */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		lpR = op_force((yyvsp[(3) - (3)].list).v, ARITH);
		lpL = op_force((yyvsp[(1) - (3)].list).v, ARITH);
		if (((yyval.list).v = op_push(lpL, ARN, lpR)) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr DIV expr", &(yyval.list));
#endif
	    }
    break;

  case 119:
/* Line 1792 of yacc.c  */
#line 1609 "comp.y"
    {			/* binary + */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		lpR = op_force((yyvsp[(3) - (3)].list).v, ARITH);
		lpL = op_force((yyvsp[(1) - (3)].list).v, ARITH);
		if (((yyval.list).v = op_push(lpL, ARN, lpR)) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr '+' expr", &(yyval.list));
#endif
	    }
    break;

  case 120:
/* Line 1792 of yacc.c  */
#line 1623 "comp.y"
    {			/* binary - */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		lpR = op_force((yyvsp[(3) - (3)].list).v, ARITH);
		lpL = op_force((yyvsp[(1) - (3)].list).v, ARITH);
		if (((yyval.list).v = op_push(lpL, ARN, lpR)) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr '-' expr", &(yyval.list));
#endif
	    }
    break;

  case 121:
/* Line 1792 of yacc.c  */
#line 1637 "comp.y"
    {		/* << */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		lpR = op_force((yyvsp[(3) - (3)].list).v, ARITH);
		lpL = op_force((yyvsp[(1) - (3)].list).v, ARITH);
		if (((yyval.list).v = op_push(lpL, ARN, lpR)) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr << expr", &(yyval.list));
#endif
	    }
    break;

  case 122:
/* Line 1792 of yacc.c  */
#line 1651 "comp.y"
    {		/* >> */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		lpR = op_force((yyvsp[(3) - (3)].list).v, ARITH);
		lpL = op_force((yyvsp[(1) - (3)].list).v, ARITH);
		if (((yyval.list).v = op_push(lpL, ARN, lpR)) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr >> expr", &(yyval.list));
#endif
	    }
    break;

  case 123:
/* Line 1792 of yacc.c  */
#line 1675 "comp.y"
    {			/* binary && */
		Symbol *	sp;
		int		typ;
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		if ((yyvsp[(1) - (3)].list).v &&
		    (sp = (yyvsp[(1) - (3)].list).v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type) > ARN || typ == UDF || !sp->u_blist) &&
		    (yyvsp[(3) - (3)].list).v &&
		    (sp = (yyvsp[(3) - (3)].list).v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type) > ARN || typ == UDF || !sp->u_blist)) {
		    lpL = op_force((yyvsp[(1) - (3)].list).v, GATE);
		    lpR = op_force((yyvsp[(3) - (3)].list).v, GATE);
		    (yyval.list).v = op_push(lpL, AND, lpR);	/* logical & */
		    if (iC_Wflag & W_DEPRECATED_LOGIC) {
			warning("Use of '&&' with imm bit variables is deprecated (use '&'):", sp->name);
		    }
		} else {
		    lpL = op_force((yyvsp[(1) - (3)].list).v, ARITH);
		    lpR = op_force((yyvsp[(3) - (3)].list).v, ARITH);
		    (yyval.list).v = op_push(lpL, ARN, lpR);	/* arithmetic && */
		    (yyval.list).v = op_force((yyval.list).v, GATE);	/* default GATE output */
		}
		if ((yyval.list).v) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr B_AND expr", &(yyval.list));
#endif
	    }
    break;

  case 124:
/* Line 1792 of yacc.c  */
#line 1707 "comp.y"
    {			/* binary || */
		Symbol *	sp;
		int		typ;
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		if ((yyvsp[(1) - (3)].list).v &&
		    (sp = (yyvsp[(1) - (3)].list).v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type) > ARN || typ == UDF || !sp->u_blist) &&
		    (yyvsp[(3) - (3)].list).v &&
		    (sp = (yyvsp[(3) - (3)].list).v->le_sym)->ftype != ARITH &&
		    ((typ = sp->type) > ARN || typ == UDF || !sp->u_blist)) {
		    lpL = op_force((yyvsp[(1) - (3)].list).v, GATE);
		    lpR = op_force((yyvsp[(3) - (3)].list).v, GATE);
		    (yyval.list).v = op_push(lpL, OR, lpR);	/* logical | */
		    if (iC_Wflag & W_DEPRECATED_LOGIC) {
			warning("Use of '||' with imm bit variables is deprecated (use '|'):", sp->name);
		    }
		} else {
		    lpL = op_force((yyvsp[(1) - (3)].list).v, ARITH);
		    lpR = op_force((yyvsp[(3) - (3)].list).v, ARITH);
		    (yyval.list).v = op_push(lpL, ARN, lpR);	/* arithmetic || */
		    (yyval.list).v = op_force((yyval.list).v, GATE);	/* default GATE output */
		}
		if ((yyval.list).v) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr B_OR expr", &(yyval.list));
#endif
	    }
    break;

  case 125:
/* Line 1792 of yacc.c  */
#line 1747 "comp.y"
    {		/* ? : */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (5)].list).f; (yyval.list).l = (yyvsp[(5) - (5)].list).l;
		lpR = op_force((yyvsp[(5) - (5)].list).v, ARITH);
		lpL = op_force((yyvsp[(3) - (5)].list).v, ARITH);
		lpR = op_push(lpL, ARN, lpR);
		lpL = op_force((yyvsp[(1) - (5)].list).v, ARITH);
		if (((yyval.list).v = op_push(lpL, ARN, lpR)) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr ? expr : expr", &(yyval.list));
#endif
	    }
    break;

  case 126:
/* Line 1792 of yacc.c  */
#line 1792 "comp.y"
    {			/* ? : */
		List_e *	lpL;
		List_e *	lpR;
		(yyval.list).f = (yyvsp[(1) - (4)].list).f; (yyval.list).l = (yyvsp[(4) - (4)].list).l;
		lpR = op_force((yyvsp[(4) - (4)].list).v, ARITH);
		lpL = op_force((yyvsp[(1) - (4)].list).v, ARITH);
		if (((yyval.list).v = op_push(lpL, ARN, lpR)) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
		if (iC_Pflag) {
		    Symbol * sp = (yyvsp[(4) - (4)].list).v->le_sym;
		    warning("ISO C forbids omitting the middle term of a ?: expression", sp->name);
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: expr ? : expr", &(yyval.list));
#endif
	    }
    break;

  case 127:
/* Line 1792 of yacc.c  */
#line 1827 "comp.y"
    {			/* unary ~ */
		Symbol *	sp;
		(yyval.list).f = (yyvsp[(1) - (2)].vai).f; (yyval.list).l = (yyvsp[(2) - (2)].list).l;
		if ((yyvsp[(2) - (2)].list).v) {
		    int typ;
		    if ((sp = (yyvsp[(2) - (2)].list).v->le_sym)->ftype != ARITH &&
			(((typ = sp->type) != ARNC && typ != ARNF && typ != ARN &&
			typ != SH) || sp->u_blist == 0)) {
							/* logical negation */
			(yyval.list).v = op_not(op_force((yyvsp[(2) - (2)].list).v, GATE));
		    } else {
							/* bitwise negation */
			(yyval.list).v = op_push(0, ARN, op_force((yyvsp[(2) - (2)].list).v, ARITH));
		    }
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		} else {
		    (yyval.list).v = 0;				/* constant negation */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: ~expr", &(yyval.list));
#endif
	    }
    break;

  case 128:
/* Line 1792 of yacc.c  */
#line 1850 "comp.y"
    {			/* unary ! */
		Symbol *	sp;
		(yyval.list).f = (yyvsp[(1) - (2)].vai).f; (yyval.list).l = (yyvsp[(2) - (2)].list).l;
		if ((yyvsp[(2) - (2)].list).v) {
		    int typ;
		    if ((sp = (yyvsp[(2) - (2)].list).v->le_sym)->ftype != ARITH &&
			(((typ = sp->type) != ARNC && typ != ARNF && typ != ARN &&
			typ != SH) || sp->u_blist == 0)) {
							/* logical negation */
			(yyval.list).v = op_not(op_force((yyvsp[(2) - (2)].list).v, GATE));
			if (iC_Wflag & W_DEPRECATED_LOGIC) {
			    warning("Use of '!' with an imm bit variable is deprecated (use '~'):", sp->name);
			}
		    } else {
							/* arithmetic negation */
			(yyval.list).v = op_push(0, ARN, op_force((yyvsp[(2) - (2)].list).v, ARITH));
		    }
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		} else {
		    (yyval.list).v = 0;				/* constant negation */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: !expr", &(yyval.list));
#endif
	    }
    break;

  case 129:
/* Line 1792 of yacc.c  */
#line 1879 "comp.y"
    {			/* unary + */
		(yyval.list).f = (yyvsp[(1) - (2)].vai).f; (yyval.list).l = (yyvsp[(2) - (2)].list).l;
		if (((yyval.list).v = op_push(0, ARN, op_force((yyvsp[(2) - (2)].list).v, ARITH))) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: '+' expr", &(yyval.list));
#endif
	    }
    break;

  case 130:
/* Line 1792 of yacc.c  */
#line 1889 "comp.y"
    {			/* unary - */
		(yyval.list).f = (yyvsp[(1) - (2)].vai).f; (yyval.list).l = (yyvsp[(2) - (2)].list).l;
		if (((yyval.list).v = op_push(0, ARN, op_force((yyvsp[(2) - (2)].list).v, ARITH))) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "expr: '-' expr", &(yyval.list));
#endif
	    }
    break;

  case 131:
/* Line 1792 of yacc.c  */
#line 1944 "comp.y"
    {			/* comma expression */
		(yyval.list) = (yyvsp[(1) - (1)].list);
		if ((yyval.list).v != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "comma_expr: aexpr", &(yyval.list));
#endif
	    }
    break;

  case 132:
/* Line 1792 of yacc.c  */
#line 1954 "comp.y"
    {		/* asgn casgn tasgn and vFunCall before , */
		(yyval.list) = (yyvsp[(3) - (3)].list);				/* only the last expression is returned */
		if ((yyval.list).v != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
		memset((yyvsp[(1) - (3)].sym).f, '#', (yyvsp[(2) - (3)].vai).l - (yyvsp[(1) - (3)].sym).f);		/* mark unused asgn , */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "comma_expr: asgnStatement , comma_expr", &(yyval.list));
#endif
	    }
    break;

  case 133:
/* Line 1792 of yacc.c  */
#line 1968 "comp.y"
    {			/* cexpr comma expression */
		(yyval.list) = (yyvsp[(1) - (1)].list);
		if ((yyval.list).v != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cexpr_comma_expr: cexpr", &(yyval.list));
#endif
	    }
    break;

  case 134:
/* Line 1792 of yacc.c  */
#line 1978 "comp.y"
    {		/* asgn casgn tasgn and vFunCall before , */
		(yyval.list) = (yyvsp[(3) - (3)].list);				/* only the last expression is returned */
		if ((yyval.list).v != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
		memset((yyvsp[(1) - (3)].sym).f, '#', (yyvsp[(2) - (3)].vai).l - (yyvsp[(1) - (3)].sym).f);		/* mark unused asgn , */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cexpr_comma_expr: asgnStatement , cexpr_comma_expr", &(yyval.list));
#endif
	    }
    break;

  case 135:
/* Line 1792 of yacc.c  */
#line 1992 "comp.y"
    {			/* texpr comma expression */
		(yyval.list) = (yyvsp[(1) - (1)].list);
		if ((yyval.list).v != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "texpr_comma_expr: texpr", &(yyval.list));
#endif
	    }
    break;

  case 136:
/* Line 1792 of yacc.c  */
#line 2002 "comp.y"
    {		/* asgn casgn tasgn and vFunCall before , */
		(yyval.list) = (yyvsp[(3) - (3)].list);				/* only the last expression is returned */
		if ((yyval.list).v != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
		memset((yyvsp[(1) - (3)].sym).f, '#', (yyvsp[(2) - (3)].vai).l - (yyvsp[(1) - (3)].sym).f);		/* mark unused asgn , */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "texpr_comma_expr: asgnStatement , texpr_comma_expr", &(yyval.list));
#endif
	    }
    break;

  case 137:
/* Line 1792 of yacc.c  */
#line 2022 "comp.y"
    {
		(yyval.vai) = (yyvsp[(1) - (1)].vai);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(VAL, "Constant: NUMBER", &(yyval.vai));
#endif
	    }
    break;

  case 138:
/* Line 1792 of yacc.c  */
#line 2028 "comp.y"
    {				/* sizeof array */
		(yyval.vai).f = (yyvsp[(1) - (2)].sym).f; (yyval.vai).l = (yyvsp[(2) - (2)].sym).l;
		List_e *	lp;
		int		i, len1, len2;
		char		buf[TSIZE];
		assert((yyvsp[(2) - (2)].sym).v && (yyvsp[(2) - (2)].sym).v->ftype == UDFA);
		i = (lp = (yyvsp[(2) - (2)].sym).v->list) ?
		    (lp->le_val & VAL_MASK) : 0;	/* 0 is returned if no size is specified */
		snprintf(buf, TSIZE, "%d", i);
		if ((len1 = strlen(buf)) > (len2 = (yyval.vai).l - (yyval.vai).f)) {
		    ierror("immC array is far too big:", buf);
		} else {
		    strncpy((yyval.vai).f, buf, len2);		/* overwrite the words 'sizeof x' by the number text */
		    if ((len2 -= len1) > 0) {
			memset((yyval.vai).f + len1, '#', len2);	/* active blank ##### */
		    }
		}
		(yyval.vai).v = i;				/* value in case it is needed */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(VAL, "Constant: SIZEOF IMMCARRAY", &(yyval.vai));
#endif
	    }
    break;

  case 139:
/* Line 1792 of yacc.c  */
#line 2053 "comp.y"
    {
		(yyval.vap).f = (yyvsp[(1) - (1)].list).f; (yyval.vap).l = (yyvsp[(1) - (1)].list).l;
		(yyval.vap).v = evalConstExpr(&(yyvsp[(1) - (1)].list));		/* returns a Valp */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(VAP, "constExpr: aexpr", &(yyval.vap));
#endif
	    }
    break;

  case 140:
/* Line 1792 of yacc.c  */
#line 2110 "comp.y"
    { ccfrag = '%'; ccFP = T1FP; }
    break;

  case 141:
/* Line 1792 of yacc.c  */
#line 2111 "comp.y"
    { (yyval.sym).v = 0;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) { fprintf(iC_outFP, ">>>lBlock end\n"); fflush(iC_outFP); }
#endif
		functionUse[0].c_cnt |= F_CALLED;
	    }
    break;

  case 142:
/* Line 1792 of yacc.c  */
#line 2127 "comp.y"
    { ccfrag = '{'; ccFP = T4FP; }
    break;

  case 143:
/* Line 1792 of yacc.c  */
#line 2128 "comp.y"
    {
		(yyval.vai) = (yyvsp[(3) - (4)].vai);				/* function # is yacc token */
		cBlockFlag = 1;				/* hold up get() till another token found */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) { fprintf(iC_outFP, ">>>cBlock #%d end\n", (yyvsp[(3) - (4)].vai).v >> FUN_OFFSET); fflush(iC_outFP); }
#endif
	    }
    break;

  case 144:
/* Line 1792 of yacc.c  */
#line 2154 "comp.y"
    { (yyval.list).v = sy_push(iclock); }
    break;

  case 145:
/* Line 1792 of yacc.c  */
#line 2155 "comp.y"
    { (yyval.list).v = sy_push(iclock); }
    break;

  case 146:
/* Line 1792 of yacc.c  */
#line 2156 "comp.y"
    { (yyval.list) = (yyvsp[(2) - (2)].list); }
    break;

  case 147:
/* Line 1792 of yacc.c  */
#line 2162 "comp.y"
    { (yyval.list) = (yyvsp[(1) - (1)].list); }
    break;

  case 148:
/* Line 1792 of yacc.c  */
#line 2163 "comp.y"
    { (yyval.list) = (yyvsp[(1) - (2)].list); }
    break;

  case 149:
/* Line 1792 of yacc.c  */
#line 2164 "comp.y"
    {			/* timer */
		(yyval.list).f = (yyvsp[(1) - (1)].list).f; (yyval.list).l = (yyvsp[(1) - (1)].list).l;
		(yyval.list).v = delayOne((yyvsp[(1) - (1)].list).v);			/* with implicit delay of 1 */
	    }
    break;

  case 150:
/* Line 1792 of yacc.c  */
#line 2168 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (2)].list).f; (yyval.list).l = (yyvsp[(2) - (2)].vai).l;
		(yyval.list).v = delayOne((yyvsp[(1) - (2)].list).v);			/* with implicit delay of 1 */
	    }
    break;

  case 151:
/* Line 1792 of yacc.c  */
#line 2177 "comp.y"
    { (yyval.list) = (yyvsp[(1) - (1)].list); }
    break;

  case 152:
/* Line 1792 of yacc.c  */
#line 2178 "comp.y"
    {			/* timer with delay expression */
		(yyval.list) = (yyvsp[(1) - (3)].list); (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		if (checkConstExpr((yyvsp[(3) - (3)].list).v) == NULL) {
		    if (const_push(&(yyvsp[(3) - (3)].list))) { errInt(); YYERROR; }
		}
		(yyvsp[(3) - (3)].list).v = op_force((yyvsp[(3) - (3)].list).v, ARITH);
		(yyvsp[(3) - (3)].list).v->le_val = (unsigned)-1;		/* mark link as timer value */
		(yyval.list).v = op_push((yyvsp[(1) - (3)].list).v, TIM, (yyvsp[(3) - (3)].list).v);
	    }
    break;

  case 153:
/* Line 1792 of yacc.c  */
#line 2471 "comp.y"
    {		/* if (expr) { x++; } */
		if (iC_openT4T5(1)) ierror("IF: cannot open:", T4FN); /* rewind if necessary */
		writeCexeString(T4FP, ++c_number);	/* and record for copying */
		if (iFunSymExt == 0) {			/* defer freeing till called */
		    functionUse[0].c_cnt |= F_CALLED;	/* flag for copying temp file */
		    functionUse[c_number].c_cnt++;	/* free this 'if' function for copying */
		}
		fprintf(T4FP, "    if (iC_gf->gt_val < 0)\n");
	    }
    break;

  case 154:
/* Line 1792 of yacc.c  */
#line 2480 "comp.y"
    {		/* { x++; } */
		(yyval.list).v = bltin(&(yyvsp[(1) - (7)].sym), &(yyvsp[(3) - (7)].list), &(yyvsp[(4) - (7)].list), 0, 0, 0, 0, &(yyvsp[(7) - (7)].vai));
	    }
    break;

  case 155:
/* Line 1792 of yacc.c  */
#line 2492 "comp.y"
    {		/* if (expr) { x++; } */
		(yyval.list).v = (yyvsp[(1) - (1)].list).v;
		ffexprCompile("IF", (yyval.list).v, c_number);	/* c_compile cBlock */
	    }
    break;

  case 156:
/* Line 1792 of yacc.c  */
#line 2503 "comp.y"
    {		/* { x++; } else */
		Symbol *	sp;
		List_e *	lp;
		sp = (yyvsp[(1) - (2)].list).v->le_sym;			/* slave, deleted later */
		assert(sp);
		lp = sp->u_blist;
		assert(lp);
		sp = lp->le_sym;			/* master - currently ftype F_CF */
		assert(sp);
		sp->ftype = (yyvsp[(2) - (2)].sym).v->ftype;		/* make it ftype F_CE from ELSE */
		fprintf(T4FP, "    else\n");
	    }
    break;

  case 157:
/* Line 1792 of yacc.c  */
#line 2515 "comp.y"
    {		/* { x--; } */
		(yyval.list).v = (yyvsp[(1) - (4)].list).v;
		ffexprCompile("IF ELSE", (yyval.list).v, c_number); /* c_compile cBlock's */
	    }
    break;

  case 158:
/* Line 1792 of yacc.c  */
#line 2526 "comp.y"
    {	/* switch (expr) { case ...; } */
		if (iC_openT4T5(1)) ierror("SWITCH: cannot open:", T4FN); /* rewind if necessary */
		writeCexeString(T4FP, ++c_number);	/* and record for copying */
		if (iFunSymExt == 0) {			/* defer freeing till called */
		    functionUse[0].c_cnt |= F_CALLED;	/* flag for copying temp file */
		    functionUse[c_number].c_cnt++;	/* free this 'switch' function for copying */
		}
		fprintf(T4FP, "    switch (iC_gf->gt_new)\n");
	    }
    break;

  case 159:
/* Line 1792 of yacc.c  */
#line 2535 "comp.y"
    {	/* { x++; } */
		(yyval.list).v = bltin(&(yyvsp[(1) - (7)].sym), &(yyvsp[(3) - (7)].list), &(yyvsp[(4) - (7)].list), 0, 0, 0, 0, &(yyvsp[(7) - (7)].vai));
		ffexprCompile("SWITCH", (yyval.list).v, c_number); /* c_compile cBlock */
	    }
    break;

  case 160:
/* Line 1792 of yacc.c  */
#line 2547 "comp.y"
    {			/* dcasgn is NOT an cexpr */
		if ((yyvsp[(1) - (3)].sym).v == 0 || (yyvsp[(1) - (3)].sym).v->ftype != CLCKL) {
		    ierror("assigning clock to variable declared differently:", (yyvsp[(1) - (3)].sym).v->name);
		    if ((yyvsp[(1) - (3)].sym).v) {
			(yyvsp[(1) - (3)].sym).v->type = ERR;		/* cannot execute properly */
			(yyvsp[(1) - (3)].sym).v->ftype = CLCKL;
		    }
		} else if ((yyvsp[(1) - (3)].sym).v->type != UDF && (yyvsp[(1) - (3)].sym).v->type != CLK) {
		    ierror("assigning clock to variable assigned differently:", (yyvsp[(1) - (3)].sym).v->name);
		    (yyvsp[(1) - (3)].sym).v->type = ERR;			/* cannot execute properly */
		}
		if ((yyvsp[(1) - (3)].sym).v) (yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "dcasgn: decl = cexpr", &(yyval.sym));
#endif
	    }
    break;

  case 161:
/* Line 1792 of yacc.c  */
#line 2565 "comp.y"
    {
		(yyvsp[(1) - (3)].sym).v->ftype = CLCKL;
		(yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), 0);	/* not strict */
		if (iC_Sflag) {
		    ierror("strict: assignment to an undeclared 'imm clock':", (yyvsp[(1) - (3)].sym).v->name);
		    (yyvsp[(1) - (3)].sym).v->type = ERR;			/* cannot execute properly */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "casgn: UNDEF = cexpr", &(yyval.sym));
#endif
	    }
    break;

  case 162:
/* Line 1792 of yacc.c  */
#line 2576 "comp.y"
    {
		if ((yyvsp[(1) - (3)].sym).v->type != UDF && (yyvsp[(1) - (3)].sym).v->type != ERR) {
		    ierror("multiple assignment clock:", (yyvsp[(1) - (3)].sym).v->name);
		    (yyvsp[(1) - (3)].sym).v->type = ERR;			/* cannot execute properly */
		}
		assert((yyvsp[(1) - (3)].sym).v->ftype == CLCKL);
		(yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "casgn: CVAR = cexpr", &(yyval.sym));
#endif
	    }
    break;

  case 163:
/* Line 1792 of yacc.c  */
#line 2589 "comp.y"
    { (yyval.list).v = checkDecl((yyvsp[(1) - (1)].sym).v); }
    break;

  case 164:
/* Line 1792 of yacc.c  */
#line 2590 "comp.y"
    { (yyval.list).v = sy_push((yyvsp[(1) - (1)].sym).v); }
    break;

  case 165:
/* Line 1792 of yacc.c  */
#line 2591 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (1)].vai).f; (yyval.list).l = (yyvsp[(1) - (1)].vai).l;	/* no node, value not used */
		(yyval.list).v = 0;			/* mark as illegal in clock expressions */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cexpr: Constant", &(yyval.list));
#endif
	    }
    break;

  case 166:
/* Line 1792 of yacc.c  */
#line 2609 "comp.y"
    {
		Symbol *	sp = &nSym;
		if ((yyvsp[(1) - (1)].list).v == 0 || (sp = (yyvsp[(1) - (1)].list).v->le_sym) == 0 || sp->ftype != CLCKL) {
		    ierror("called function does not return type clock:", sp->name);
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		}
		(yyval.list) = (yyvsp[(1) - (1)].list);
	    }
    break;

  case 167:
/* Line 1792 of yacc.c  */
#line 2617 "comp.y"
    {
		Symbol *	sp = &nSym;
		(yyval.list).f = (yyvsp[(1) - (3)].vai).f; (yyval.list).l = (yyvsp[(3) - (3)].vai).l;
		if (((yyval.list).v = (yyvsp[(2) - (3)].list).v) == 0 || (sp = (yyval.list).v->le_sym) == 0 || sp->ftype != CLCKL) {
		    ierror("value in parentheses is not type clock:", sp->name);
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		} else {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cexpr: ( cexpr_comma_expr )", &(yyval.list));
#endif
	    }
    break;

  case 168:
/* Line 1792 of yacc.c  */
#line 2677 "comp.y"
    {			/* dtasgn is NOT an texpr */
		if ((yyvsp[(1) - (3)].sym).v == 0 || (yyvsp[(1) - (3)].sym).v->ftype != TIMRL) {
		    ierror("assigning timer to variable declared differently:", (yyvsp[(1) - (3)].sym).v->name);
		    if ((yyvsp[(1) - (3)].sym).v) {
			(yyvsp[(1) - (3)].sym).v->type = ERR;		/* cannot execute properly */
			(yyvsp[(1) - (3)].sym).v->ftype = TIMRL;
		    }
		} else if ((yyvsp[(1) - (3)].sym).v->type != UDF && (yyvsp[(1) - (3)].sym).v->type != TIM) {
		    ierror("assigning timer to variable assigned differently:", (yyvsp[(1) - (3)].sym).v->name);
		    (yyvsp[(1) - (3)].sym).v->type = ERR;			/* cannot execute properly */
		}
		if ((yyvsp[(1) - (3)].sym).v) (yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "dtasgn: decl = texpr", &(yyval.sym));
#endif
	    }
    break;

  case 169:
/* Line 1792 of yacc.c  */
#line 2695 "comp.y"
    {
		(yyvsp[(1) - (3)].sym).v->ftype = TIMRL;
		(yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), 0);	/* not strict */
		if (iC_Sflag) {
		    ierror("strict: assignment to an undeclared 'imm timer':", (yyvsp[(1) - (3)].sym).v->name);
		    (yyvsp[(1) - (3)].sym).v->type = ERR;			/* cannot execute properly */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "tasgn: UNDEF = texpr", &(yyval.sym));
#endif
	    }
    break;

  case 170:
/* Line 1792 of yacc.c  */
#line 2706 "comp.y"
    {
		if ((yyvsp[(1) - (3)].sym).v->type != UDF && (yyvsp[(1) - (3)].sym).v->type != ERR) {
		    ierror("multiple assignment timer:", (yyvsp[(1) - (3)].sym).v->name);
		    (yyvsp[(1) - (3)].sym).v->type = ERR;			/* cannot execute properly */
		}
		assert((yyvsp[(1) - (3)].sym).v->ftype == TIMRL);
		(yyval.sym).v = assignExpression(&(yyvsp[(1) - (3)].sym), &(yyvsp[(3) - (3)].list), 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "tasgn: TVAR = texpr", &(yyval.sym));
#endif
	    }
    break;

  case 171:
/* Line 1792 of yacc.c  */
#line 2719 "comp.y"
    { (yyval.list).v = checkDecl((yyvsp[(1) - (1)].sym).v); }
    break;

  case 172:
/* Line 1792 of yacc.c  */
#line 2720 "comp.y"
    { (yyval.list).v = sy_push((yyvsp[(1) - (1)].sym).v); }
    break;

  case 173:
/* Line 1792 of yacc.c  */
#line 2721 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (1)].vai).f; (yyval.list).l = (yyvsp[(1) - (1)].vai).l;	/* no node, value not used */
		(yyval.list).v = 0;			/* mark as illegal in timer expressions */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "texpr: Constant", &(yyval.list));
#endif
	    }
    break;

  case 174:
/* Line 1792 of yacc.c  */
#line 2739 "comp.y"
    {
		Symbol *	sp = &nSym;
		if ((yyvsp[(1) - (1)].list).v == 0 || (sp = (yyvsp[(1) - (1)].list).v->le_sym) == 0 || sp->ftype != TIMRL) {
		    ierror("called function does not return type timer:", sp->name);
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		}
		(yyval.list) = (yyvsp[(1) - (1)].list);
	    }
    break;

  case 175:
/* Line 1792 of yacc.c  */
#line 2747 "comp.y"
    {
		Symbol *	sp = &nSym;
		(yyval.list).f = (yyvsp[(1) - (3)].vai).f; (yyval.list).l = (yyvsp[(3) - (3)].vai).l;
		if (((yyval.list).v = (yyvsp[(2) - (3)].list).v) == 0 || (sp = (yyval.list).v->le_sym) == 0 || sp->ftype != TIMRL) {
		    ierror("value in parentheses is not type timer:", sp->name);
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		} else {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "texpr: ( texpr_comma_expr )", &(yyval.list));
#endif
	    }
    break;

  case 176:
/* Line 1792 of yacc.c  */
#line 2897 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (1)].sym).f; (yyval.list).l = (yyvsp[(1) - (1)].sym).l;
		(yyval.list).v = cCallCount((yyvsp[(1) - (1)].sym).v, NULL, -1);
		if (iC_Wflag & W_DEPRECATED_LOGIC) {
		    warning("C variable will not fire this expression:", (yyvsp[(1) - (1)].sym).v->name);
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cCall: UNDEF", &(yyval.list));
#endif
	    }
    break;

  case 177:
/* Line 1792 of yacc.c  */
#line 2907 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (4)].sym).f; (yyval.list).l = (yyvsp[(4) - (4)].vai).l;
		(yyval.list).v = cCallCount((yyvsp[(1) - (4)].sym).v, (yyvsp[(3) - (4)].list).v, (yyvsp[(3) - (4)].list).v ? (yyvsp[(3) - (4)].list).v->le_val : 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cCall: UNDEF ( cParams )", &(yyval.list));
#endif
	    }
    break;

  case 178:
/* Line 1792 of yacc.c  */
#line 2914 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (4)].sym).f; (yyval.list).l = (yyvsp[(4) - (4)].vai).l;
		(yyval.list).v = cCallCount((yyvsp[(1) - (4)].sym).v, (yyvsp[(3) - (4)].list).v, (yyvsp[(3) - (4)].list).v ? (yyvsp[(3) - (4)].list).v->le_val : 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cCall: CNAME ( cParams )", &(yyval.list));
#endif
	    }
    break;

  case 179:
/* Line 1792 of yacc.c  */
#line 2921 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (5)].sym).f; (yyval.list).l = (yyvsp[(5) - (5)].vai).l;
		(yyval.list).v = cCallCount((yyvsp[(1) - (5)].sym).v, (yyvsp[(3) - (5)].list).v, (yyvsp[(3) - (5)].list).v ? (yyvsp[(3) - (5)].list).v->le_val : 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cCall: UNDEF ( cParams error )", &(yyval.list));
#endif
		yyerrok;
	    }
    break;

  case 180:
/* Line 1792 of yacc.c  */
#line 2929 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (5)].sym).f; (yyval.list).l = (yyvsp[(5) - (5)].vai).l;
		(yyval.list).v = cCallCount((yyvsp[(1) - (5)].sym).v, (yyvsp[(3) - (5)].list).v, (yyvsp[(3) - (5)].list).v ? (yyvsp[(3) - (5)].list).v->le_val : 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cCall: CNAME ( cParams error)", &(yyval.list));
#endif
		yyerrok;
	    }
    break;

  case 181:
/* Line 1792 of yacc.c  */
#line 2939 "comp.y"
    { (yyval.list).v =  0; }
    break;

  case 182:
/* Line 1792 of yacc.c  */
#line 2940 "comp.y"
    { (yyval.list)   = (yyvsp[(1) - (1)].list); }
    break;

  case 183:
/* Line 1792 of yacc.c  */
#line 2943 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (1)].list).f; (yyval.list).l = (yyvsp[(1) - (1)].list).l;
		if (((yyval.list).v = cListCount(0, (yyvsp[(1) - (1)].list).v)) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cPlist: aexpr", &(yyval.list));
#endif
	    }
    break;

  case 184:
/* Line 1792 of yacc.c  */
#line 2953 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].list).l;
		if (((yyval.list).v = cListCount((yyvsp[(1) - (3)].list).v, (yyvsp[(3) - (3)].list).v)) != 0) {
		    assert((yyval.list).f == 0 || ((yyval.list).f >= iCbuf && (yyval.list).l < &iCbuf[IMMBUFSIZE]));
		    (yyval.list).v->le_first = (yyval.list).f; (yyval.list).v->le_last = (yyval.list).l;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cPlist: cPlist , aexpr", &(yyval.list));
#endif
	    }
    break;

  case 185:
/* Line 1792 of yacc.c  */
#line 2975 "comp.y"
    { (yyval.sym) = (yyvsp[(1) - (1)].sym); }
    break;

  case 186:
/* Line 1792 of yacc.c  */
#line 2976 "comp.y"
    { (yyval.sym) = (yyvsp[(1) - (1)].sym); (yyval.sym).v = clearFunDef((yyvsp[(1) - (1)].sym).v); }
    break;

  case 187:
/* Line 1792 of yacc.c  */
#line 2977 "comp.y"
    { (yyval.sym) = (yyvsp[(1) - (1)].sym); (yyval.sym).v = clearFunDef((yyvsp[(1) - (1)].sym).v); }
    break;

  case 188:
/* Line 1792 of yacc.c  */
#line 2978 "comp.y"
    { (yyval.sym) = (yyvsp[(1) - (1)].sym); (yyval.sym).v = clearFunDef((yyvsp[(1) - (1)].sym).v); }
    break;

  case 189:
/* Line 1792 of yacc.c  */
#line 2979 "comp.y"
    { (yyval.sym) = (yyvsp[(1) - (1)].sym); (yyval.sym).v = clearFunDef((yyvsp[(1) - (1)].sym).v); }
    break;

  case 190:
/* Line 1792 of yacc.c  */
#line 2992 "comp.y"
    {
		(yyval.sym).f = (yyvsp[(1) - (3)].typ).f; (yyval.sym).l = (yyvsp[(3) - (3)].vai).l;
		if ((yyvsp[(1) - (3)].typ).v.type == ARNC || (yyvsp[(1) - (3)].typ).v.type == LOGC) {
		    ierror("Definition of an immC function is illegal:", (yyval.sym).f);
		}
		(yyval.sym).v = functionDefHead((yyvsp[(1) - (3)].typ).v.ftype, (yyvsp[(2) - (3)].sym).v);	/* function head Symbol */
	    }
    break;

  case 191:
/* Line 1792 of yacc.c  */
#line 3006 "comp.y"
    {
		(yyval.sym).f = (yyvsp[(1) - (4)].sym).f; (yyval.sym).l = (yyvsp[(4) - (4)].vai).l;
		if ((yyvsp[(1) - (4)].sym).v->ftype == 1) {		/* immC in init.c */
		    ierror("Definition of an immC void function is illegal:", (yyval.sym).f);
		}
		(yyval.sym).v = functionDefHead((yyvsp[(2) - (4)].sym).v->ftype, (yyvsp[(3) - (4)].sym).v); /* function head Symbol */
	    }
    break;

  case 192:
/* Line 1792 of yacc.c  */
#line 3019 "comp.y"
    {
		(yyval.sym).f = (yyvsp[(1) - (6)].sym).f; (yyval.sym).l = (yyvsp[(6) - (6)].vai).l;
		(yyval.sym).v = functionDefinition((yyvsp[(1) - (6)].sym).v, (yyvsp[(2) - (6)].list).v);
	    }
    break;

  case 193:
/* Line 1792 of yacc.c  */
#line 3023 "comp.y"
    {
		(yyval.sym).f = (yyvsp[(1) - (7)].sym).f; (yyval.sym).l = (yyvsp[(7) - (7)].vai).l;
		(yyval.sym).v = functionDefinition((yyvsp[(1) - (7)].sym).v, (yyvsp[(2) - (7)].list).v);
		yyerrok;
	    }
    break;

  case 194:
/* Line 1792 of yacc.c  */
#line 3036 "comp.y"
    {
		(yyval.list).v = 0; (yyval.list).f = (yyval.list).l = 0;
		clrBuf();
	    }
    break;

  case 195:
/* Line 1792 of yacc.c  */
#line 3040 "comp.y"
    {
		(yyval.list).v = 0;			/* $$.v is not required */
		(yyval.list).f = (yyval.list).l = 0;
		clrBuf();
	    }
    break;

  case 196:
/* Line 1792 of yacc.c  */
#line 3045 "comp.y"
    {
		(yyval.list).v = 0; (yyval.list).f = (yyval.list).l = 0;
		clrBuf();
		yyerrok;
	    }
    break;

  case 197:
/* Line 1792 of yacc.c  */
#line 3057 "comp.y"
    {
		(yyval.sym).f = (yyvsp[(2) - (2)].list).f; (yyval.sym).l = (yyvsp[(2) - (2)].list).l;		/* TD $$.f should be func$ */
		(yyval.sym).v = returnStatement(&(yyvsp[(2) - (2)].list));
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "return_statement: RETURN actexpr", &(yyval.sym));
#endif
	    }
    break;

  case 198:
/* Line 1792 of yacc.c  */
#line 3068 "comp.y"
    { (yyval.list) = (yyvsp[(1) - (1)].list); }
    break;

  case 199:
/* Line 1792 of yacc.c  */
#line 3069 "comp.y"
    { (yyval.list) = (yyvsp[(1) - (1)].list); }
    break;

  case 200:
/* Line 1792 of yacc.c  */
#line 3070 "comp.y"
    { (yyval.list) = (yyvsp[(1) - (1)].list); }
    break;

  case 201:
/* Line 1792 of yacc.c  */
#line 3074 "comp.y"
    { (yyval.list) = (yyvsp[(1) - (1)].list); }
    break;

  case 202:
/* Line 1792 of yacc.c  */
#line 3075 "comp.y"
    {
		Symbol *	sp;

		(yyval.list).f = (yyvsp[(1) - (1)].sym).f; (yyval.list).l = (yyvsp[(1) - (1)].sym).l;
		assert((yyvsp[(1) - (1)].sym).f == 0 || ((yyvsp[(1) - (1)].sym).f >= iCbuf && (yyvsp[(1) - (1)].sym).l < &iCbuf[IMMBUFSIZE]));
		(yyval.list).v = checkDecl(sp = (yyvsp[(1) - (1)].sym).v);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "ractexpr: immCarray", &(yyval.list));
#endif
	    }
    break;

  case 203:
/* Line 1792 of yacc.c  */
#line 3091 "comp.y"
    { (yyval.list).v =  0; }
    break;

  case 204:
/* Line 1792 of yacc.c  */
#line 3092 "comp.y"
    { (yyval.list)   = (yyvsp[(1) - (1)].list); }
    break;

  case 205:
/* Line 1792 of yacc.c  */
#line 3093 "comp.y"
    { (yyval.list)   = (yyvsp[(1) - (2)].list); }
    break;

  case 206:
/* Line 1792 of yacc.c  */
#line 3096 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (1)].sym).f; (yyval.list).l = (yyvsp[(1) - (1)].sym).l;
		(yyval.list).v = sy_push((yyvsp[(1) - (1)].sym).v);			/* first link in parameter list */
		assert((yyvsp[(1) - (1)].sym).f == 0 || ((yyvsp[(1) - (1)].sym).f >= iCbuf && (yyvsp[(1) - (1)].sym).l < &iCbuf[IMMBUFSIZE]));
		(yyval.list).v->le_first = (yyvsp[(1) - (1)].sym).f;
		(yyval.list).v->le_last = (yyvsp[(1) - (1)].sym).l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fPlist: formalParameter", &(yyval.list));
#endif
	    }
    break;

  case 207:
/* Line 1792 of yacc.c  */
#line 3106 "comp.y"
    {
		List_e *	lp;

		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].sym).l;
		lp = (yyval.list).v;
		while (lp->le_next) {
		    lp = lp->le_next;
		}
		lp = lp->le_next = sy_push((yyvsp[(3) - (3)].sym).v); /* link to end of parameter list */
		assert((yyvsp[(3) - (3)].sym).f == 0 || ((yyvsp[(3) - (3)].sym).f >= iCbuf && (yyvsp[(3) - (3)].sym).l < &iCbuf[IMMBUFSIZE]));
		lp->le_first = (yyvsp[(3) - (3)].sym).f;
		lp->le_last = (yyvsp[(3) - (3)].sym).l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "fPlist: fPlist formalParameter", &(yyval.list));
#endif
	    }
    break;

  case 208:
/* Line 1792 of yacc.c  */
#line 3134 "comp.y"
    {
		(yyval.sym) = (yyvsp[(3) - (3)].sym);				/* formal assign parameter Symbol */
		(yyval.sym).v->ftype = (yyvsp[(2) - (3)].typ).v.ftype;		/* TYPE bit int clock timer */
		if ((yyvsp[(2) - (3)].typ).v.type != UDF) {
		    warning("immC type is not allowed for formal assign parameter - ignore:", (yyval.sym).v->name);
		}
		(yyval.sym).v->type  = UDF;			/* assign parameter is set to UDF */
		iFunSyText = 0;				/* no more function symbols */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "formalParameter: ASSIGN formalParaTypeDecl UNDEF", &(yyval.sym));
#endif
	    }
    break;

  case 209:
/* Line 1792 of yacc.c  */
#line 3146 "comp.y"
    {
		int	ft;

		(yyval.sym) = (yyvsp[(2) - (2)].sym);				/* formal value parameter Symbol */
		(yyval.sym).v->ftype = ft = (yyvsp[(1) - (2)].typ).v.ftype;		/* TYPE bit int clock timer */
		if ((yyvsp[(1) - (2)].typ).v.type != UDF) {
		    warning("immC type is not allowed for formal value parameter - ignore:", (yyval.sym).v->name);
		}
		if (ft >= CLCKL) {			/* check that CLCKL TIMRL */
		    ft -= CLCKL - CLCK;			/* and CLCK and TIMR are adjacent */
		}
		(yyval.sym).v->type = iC_types[ft];		/* value parameter has type set */
		iFunSyText = 0;				/* no more function symbols */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "formalParameter: formalParaTypeDecl UNDEF", &(yyval.sym));
#endif
	    }
    break;

  case 210:
/* Line 1792 of yacc.c  */
#line 3163 "comp.y"
    {
		int		typ;
		int		ftyp;
		Symbol *	sp;
		Symbol *	tsp;
		List_e *	lp;
		int		i;
		char		tempBuf[TSIZE];

		(yyval.sym).f = (yyvsp[(1) - (5)].typ).f; (yyval.sym).l = (yyvsp[(5) - (5)].vai).l;
		sp = (yyval.sym).v = (yyvsp[(2) - (5)].sym).v;
		ftyp = (yyvsp[(1) - (5)].typ).v.ftype;			/* TYPE bit int */
		if ((typ = (yyvsp[(1) - (5)].typ).v.type) == UDF) {
		    if (ftyp == ARITH) {
			typ = ARNC;			/* immC int */
		    } else
		    if (ftyp == GATE) {
			typ = LOGC;			/* immC bit */
		    } else {
			typ = UDF;
		    }
		}
		sp->type = typ;				/* only ARNC or LOGC (UDF on error) */
		sp->ftype = UDFA;			/* immC array */
		if (sp->type == ARNC || sp->type == LOGC) {
		    if ((yyvsp[(4) - (5)].vap).v.lfl == 0) {
			if ((sp->v_cnt = (unsigned int)((yyvsp[(4) - (5)].vap).v.nuv)) != 0) {	/* array size or 0 */
			    for (i = 0; i < sp->v_cnt; i++) {
				snprintf(tempBuf, TSIZE, "%s%d", sp->name, i);	/* base name + index */
				if ((tsp = lookup(tempBuf)) == 0) {
				    tsp = install(tempBuf, sp->type, iC_ftypes[sp->type]);
				}
				if (sp->list == 0) {
				    sp->list = lp = sy_push(tsp);	/* head of new member list */
				    lp->le_val = sp->v_cnt;
				} else {
				    lp = lp->le_next = sy_push(tsp);	/* link to end of new member list */
				}
			    }
			}				/* if array size 0 this is an anonymous array */
			sp->em &= ~EI;			/* mark as constant */
		    } else
		    if ((yyvsp[(4) - (5)].vap).v.lfl == 1) {
			sp->v_elist = (yyvsp[(4) - (5)].vap).v.lpt;		/* pointer to constant parameter */
			sp->em |= EI;			/* mark as pointer */
		    }
		} else {
		    ierror("formal array parameter other than immC bit or immC int:", sp->name);
		    sp->type = ERR;
		}
		iFunSyText = 0;				/* no more function symbols */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "formalParameter: formalParaTypeDecl UNDEF [ immCindex ]", &(yyval.sym));
#endif
	    }
    break;

  case 211:
/* Line 1792 of yacc.c  */
#line 3218 "comp.y"
    {
		(yyval.sym) = (yyvsp[(2) - (2)].sym);				/* formal constant parameter Symbol */
		assert((yyvsp[(1) - (2)].typ).v.ftype == ARITH);
		(yyval.sym).v->ftype = ARITH;
		(yyval.sym).v->type = ARN;			/* int */
		(yyval.sym).v->em |= EI;				/* mark parameter as a const */
		iFunSyText = 0;				/* no more function symbols */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "formalParameter: formalConstTypeDecl UNDEF", &(yyval.sym));
#endif
	    }
    break;

  case 212:
/* Line 1792 of yacc.c  */
#line 3232 "comp.y"
    {
		(yyval.typ).f = (yyval.typ).l = (yyvsp[(1) - (1)].sym).l;			/* do not include in expression string */
		(yyval.typ).v.ftype = (yyvsp[(1) - (1)].sym).v->ftype;		/* ARITH GATE CLCKL TIMRL */
		(yyval.typ).v.type = UDF;
		(yyval.typ).v.em &= ~EM;
		(yyval.typ).v.fm = 0;
		iFunSyText = iFunBuffer;		/* expecting a new function symbol */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "formalParaTypeDecl: TYPE", &(yyval.typ));
#endif
	    }
    break;

  case 213:
/* Line 1792 of yacc.c  */
#line 3243 "comp.y"
    {
		int		typ;
		int		ftyp;
		(yyval.typ).f = (yyval.typ).l = (yyvsp[(2) - (2)].sym).l;			/* do not include in expression string */
		ftyp = (yyvsp[(2) - (2)].sym).v->ftype;			/* ARITH GATE CLCKL TIMRL */
		if ((yyvsp[(1) - (2)].sym).v->ftype == 1) {			/* immC in init.c */
		    if (ftyp == ARITH) {
			typ = ARNC;			/* immC int */
		    } else
		    if (ftyp == GATE) {
			typ = LOGC;			/* immC bit */
		    } else {
			warning("immC formal parameter other than bit or int - ignore:", (yyvsp[(2) - (2)].sym).v->name);
			goto immTypeF;
		    }
		} else {				/* simple imm */
		  immTypeF:
		    typ = UDF;
		}
		(yyval.typ).v.ftype = ftyp;			/* immT is optional */
		(yyval.typ).v.type = typ;
		(yyval.typ).v.em &= ~EM;
		(yyval.typ).v.fm = 0;
		iFunSyText = iFunBuffer;		/* expecting a new function symbol */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "formalParaTypeDecl: immT TYPE", &(yyval.typ));
#endif
	    }
    break;

  case 214:
/* Line 1792 of yacc.c  */
#line 3283 "comp.y"
    {
		(yyval.typ).f = (yyval.typ).l = (yyvsp[(2) - (2)].sym).l;			/* do not include in expression string */
		switch ((yyvsp[(2) - (2)].sym).v->ftype) {
		case ARITH:
		    break;				/* correct type const int */
		case GATE:
		    ierror("const bit is not allowed as a formal parameter:", NS);
		    break;
		case CLCKL:
		    ierror("const clock is not allowed as a formal parameter:", NS);
		    break;
		case TIMRL:
		    ierror("const timer is not allowed as a formal parameter:", NS);
		    break;
		default:
		    assert(0);
		    break;
		}
		(yyval.typ).v.type = UDF;			/* formal value parameter const int */
		(yyval.typ).v.ftype = ARITH;
		(yyval.typ).v.em &= ~EM;
		(yyval.typ).v.fm = 0;
		iFunSyText = iFunBuffer;		/* expecting a new function symbol */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "formalConstTypeDecl: CONST TYPE", &(yyval.typ));
#endif
	    }
    break;

  case 215:
/* Line 1792 of yacc.c  */
#line 3310 "comp.y"
    {
		(yyval.typ).f = (yyval.typ).l = (yyvsp[(3) - (3)].sym).l;			/* do not include in expression string */
		if ((yyvsp[(1) - (3)].sym).v->ftype == 1) {			/* immC in init.c */
		    ierror("immC const is not allowed as a formal parameter:", NS);
		} else {
		    switch ((yyvsp[(3) - (3)].sym).v->ftype) {
		    case ARITH:
			break;				/* correct type const int */
		    case GATE:
			ierror("imm const bit is not allowed as a formal parameter:", NS);
			break;
		    case CLCKL:
			ierror("imm const clock is not allowed as a formal parameter:", NS);
			break;
		    case TIMRL:
			ierror("imm const timer is not allowed as a formal parameter:", NS);
			break;
		    default:
			assert(0);
			break;
		    }
		}
		(yyval.typ).v.type = UDF;			/* formal value parameter const int */
		(yyval.typ).v.ftype = ARITH;
		(yyval.typ).v.em &= ~EM;
		(yyval.typ).v.fm = 0;
		iFunSyText = iFunBuffer;		/* expecting a new function symbol */
#if YYDEBUG == 2
		if ((iC_debug & 0402) == 0402) pu(TYP, "formalConstTypeDecl: IMM CONST TYPE", &(yyval.typ));
#endif
	    }
    break;

  case 216:
/* Line 1792 of yacc.c  */
#line 3353 "comp.y"
    {
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		(yyval.list) = cloneFunction(&(yyvsp[(1) - (4)].sym), &(yyvsp[(3) - (4)].sym), &(yyvsp[(4) - (4)].vai));	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "iFunCall: IFUNCTION (rParams)", &(yyval.list));
#endif
	    }
    break;

  case 217:
/* Line 1792 of yacc.c  */
#line 3360 "comp.y"
    {
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		(yyval.list) = cloneFunction(&(yyvsp[(1) - (5)].sym), &(yyvsp[(3) - (5)].sym), &(yyvsp[(5) - (5)].vai));	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "iFunCall: IFUNCTION (rParams error)", &(yyval.list));
#endif
		yyerrok;
	    }
    break;

  case 218:
/* Line 1792 of yacc.c  */
#line 3376 "comp.y"
    {
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		(yyval.list) = cloneFunction(&(yyvsp[(1) - (4)].sym), &(yyvsp[(3) - (4)].sym), &(yyvsp[(4) - (4)].vai));	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cFunCall: CFUNCTION (rParams)", &(yyval.list));
#endif
	    }
    break;

  case 219:
/* Line 1792 of yacc.c  */
#line 3383 "comp.y"
    {
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		(yyval.list) = cloneFunction(&(yyvsp[(1) - (5)].sym), &(yyvsp[(3) - (5)].sym), &(yyvsp[(5) - (5)].vai));	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "cFunCall: CFUNCTION (rParams error)", &(yyval.list));
#endif
		yyerrok;
	    }
    break;

  case 220:
/* Line 1792 of yacc.c  */
#line 3399 "comp.y"
    {
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		(yyval.list) = cloneFunction(&(yyvsp[(1) - (4)].sym), &(yyvsp[(3) - (4)].sym), &(yyvsp[(4) - (4)].vai));	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "tFunCall: TFUNCTION (rParams)", &(yyval.list));
#endif
	    }
    break;

  case 221:
/* Line 1792 of yacc.c  */
#line 3406 "comp.y"
    {
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		(yyval.list) = cloneFunction(&(yyvsp[(1) - (5)].sym), &(yyvsp[(3) - (5)].sym), &(yyvsp[(5) - (5)].vai));	/* clone function from call head */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "tFunCall: TFUNCTION (rParams error)", &(yyval.list));
#endif
		yyerrok;
	    }
    break;

  case 222:
/* Line 1792 of yacc.c  */
#line 3423 "comp.y"
    {
		Lis	lis;
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		lis = cloneFunction(&(yyvsp[(1) - (4)].sym), &(yyvsp[(3) - (4)].sym), &(yyvsp[(4) - (4)].vai));	/* clone function from call head */
		(yyval.sym).f = lis.f; (yyval.sym).l = lis.l;
		(yyval.sym).v = 0;				/* $$.v is Sym - not compatible */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "vFunCall: VFUNCTION (rParams)", &(yyval.sym));
#endif
	    }
    break;

  case 223:
/* Line 1792 of yacc.c  */
#line 3433 "comp.y"
    {
		Lis	lis;
		/* adjust $$.f and $$.l to possibly modified arithmetic text in cloneFunction */
		lis = cloneFunction(&(yyvsp[(1) - (5)].sym), &(yyvsp[(3) - (5)].sym), &(yyvsp[(5) - (5)].vai));	/* clone function from call head */
		(yyval.sym).f = lis.f; (yyval.sym).l = lis.l;
		(yyval.sym).v = 0;				/* $$.v is Sym - not compatible */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "vFunCall: VFUNCTION (rParams error)", &(yyval.sym));
#endif
		yyerrok;
	    }
    break;

  case 224:
/* Line 1792 of yacc.c  */
#line 3466 "comp.y"
    { (yyval.sym).v =  0; }
    break;

  case 225:
/* Line 1792 of yacc.c  */
#line 3467 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 226:
/* Line 1792 of yacc.c  */
#line 3468 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (2)].sym); (yyval.sym).l = (yyvsp[(2) - (2)].vai).l; }
    break;

  case 227:
/* Line 1792 of yacc.c  */
#line 3471 "comp.y"
    {
		(yyval.sym).f = (yyvsp[(1) - (1)].list).f; (yyval.sym).l = (yyvsp[(1) - (1)].list).l;
		if (checkConstExpr((yyvsp[(1) - (1)].list).v) == NULL) {
		    if (const_push(&(yyvsp[(1) - (1)].list))) { errInt(); YYERROR; }
		}
		(yyval.sym).v = getRealParameter(0, (yyvsp[(1) - (1)].list).v);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "rPlist: ractexpr", &(yyval.sym));
#endif
	    }
    break;

  case 228:
/* Line 1792 of yacc.c  */
#line 3481 "comp.y"
    {
		(yyval.sym).f = (yyvsp[(1) - (3)].sym).f; (yyval.sym).l = (yyvsp[(3) - (3)].list).l;
		if (checkConstExpr((yyvsp[(3) - (3)].list).v) == NULL) {			/* must extract value from aexpr */
		    if (const_push(&(yyvsp[(3) - (3)].list))) { errInt(); YYERROR; }
		}
		(yyval.sym).v = getRealParameter((yyvsp[(1) - (3)].sym).v, (yyvsp[(3) - (3)].list).v);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "rPlist: rPlist, ractexpr", &(yyval.sym));
#endif
	    }
    break;

  case 229:
/* Line 1792 of yacc.c  */
#line 3585 "comp.y"
    {		/* automatic declaration of members */
		Symbol *	sp;
		Symbol *	tsp;
		List_e *	lp;
		int		i;
		Valp		v;
		char		tempBuf[TSIZE];
		(yyval.sym).f = (yyvsp[(1) - (1)].sym).f; (yyval.sym).l = (yyvsp[(1) - (1)].sym).l;
		sp = (yyval.sym).v = (yyvsp[(1) - (1)].sym).v;
		assert((sp->em & EM) == 0 && (sp->type == ARNC || sp->type == LOGC || sp->type == ERR));
		if (sp->em & EI) {
		    v.lfl = 1;
		    v.lpt = sp->v_elist;
		    listGenOut(sp, -2, &v);			/* list immC array node with const parameter size */
		    assert(sp->list == 0);
		    sp->list = sp->v_elist;			/* transfer dynamic size expression pointer */
		} else {
		    if (sp->v_cnt == 0) {
			ierror("must specify array size since there is no member initialiser list:", (yyval.sym).f);
			sp->type = ERR;
		    } else
		    if ((lp = sp->list) == 0) {
			listGenOut(sp, sp->v_cnt, 0);		/* list immC array node with numerical size */
			for (i = 0; i < sp->v_cnt; i++) {
			    snprintf(tempBuf, TSIZE, "%s%d", sp->name, i);	/* base name + index */
			    if ((tsp = lookup(tempBuf)) == 0) {
				tsp = install(tempBuf, sp->type, iC_ftypes[sp->type]);
				listGenOut(tsp, 0, 0);		/* list immC node and generate possible output */
			    }					/* not listed if member node already defined */
			    if (sp->type != tsp->type && sp->type != ERR) {
				ierror("array member type does not match type of whole array:", tsp->name);
				sp->type = ERR;			/* can only happen if member alread existed */
			    }
			    if (sp->list == 0) {
				sp->list = lp = sy_push(tsp);	/* head of new member list */
				lp->le_val = sp->v_cnt;
			    } else {
				lp = lp->le_next = sy_push(tsp);/* link to end of new member list */
			    }
			}
			listGenOut(NULL, 1, 0);			/* terminate immC array node list */
		    } else {
			assert(lp->le_val == sp->v_cnt);	/* previously declared extern */
			listGenOut(sp, sp->v_cnt, 0);		/* list immC array node */
			for (i = 0; i < sp->v_cnt; i++) {	/* use member list set up in extern declaration */
			    snprintf(tempBuf, TSIZE, "%s%d", sp->name, i);	/* base name + index */
			    if ((tsp = lookup(tempBuf)) != 0 && lp && tsp != lp->le_sym) {
				ierror("different array member to member in extern declaration:", tsp->name);
				sp->type = ERR;
			    } else {
				if (sp->type != lp->le_sym->type) {	/* ARNC or LOGC asserted above */
				    if (sp->type != ERR) {	/* TD might be assertion because tested in extern decl */
					ierror("array member type does not match type of whole array:", tsp->name);
					sp->type = ERR;	/* can only happen if member alread existed */
				    }
				} else if (tsp->em & EM) {	/* extern ARNC or LOGC ? */
				    tsp->em &= ~EM;		/* no longer extern */
				    listGenOut(tsp, 0, 0);	/* list immC node and generate possible output */
				}
			    }
			    lp = lp->le_next;
			}
			listGenOut(NULL, 1, 0);		/* terminate immC array node list */
		    }
		}
		if (iFunSymExt) {
		    char *	cp;
		    Symbol *	sp1;
		    cp = strchr(sp->name, '@');		/* locate original extension */
		    assert(cp && isprint(cp[1]));	/* extension must be at least 1 character */
		    if ((sp1 = lookup(++cp)) != 0 && (sp1->em & EM)) {
			warning("declaration of an extern variable in a function - ignored:", cp);
		    }
		    collectStatement(sp);
		    iFunSyText = 0;			/* no more function symbols */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "immCarray: immCarrayHead", &(yyval.sym));
#endif
	    }
    break;

  case 230:
/* Line 1792 of yacc.c  */
#line 3665 "comp.y"
    {
		Symbol *	sp;
		Symbol *	tsp;
		List_e *	lp;
		List_e *	tlp;
		List_e *	ttlp;
		int		i;
		(yyval.sym).f = (yyvsp[(1) - (5)].sym).f; (yyval.sym).l = (yyvsp[(5) - (5)].vai).l;
		sp = (yyval.sym).v = (yyvsp[(1) - (5)].sym).v;
		assert((sp->em & EM) == 0 && (sp->type == ARNC || sp->type == LOGC || sp->type == ERR));
		if (sp->em & EI) {
		    ierror("array size set by a constant parameter cannot have member initialisers:", (yyval.sym).f);
		    sp->type = ERR;
		} else {
		    if (sp->v_cnt > 0 && sp->v_cnt != (yyvsp[(4) - (5)].list).v->le_val) {
			fprintf(stderr, "immCarray: array size %u, initaliser count %u\n", sp->v_cnt, (yyvsp[(4) - (5)].list).v->le_val);
			ierror("array size does not match number of member initialisers:", (yyval.sym).f);
			sp->type = ERR;
		    }
		    sp->v_cnt = (yyvsp[(4) - (5)].list).v->le_val;			/* number of member initialisers is array size */
		    /* v_cnt is not preserved to outNet() - but list and count in list->le_val is */
		    tlp = ttlp = sp->list;			/* extern declaration if set */
		    listGenOut(sp, sp->v_cnt, 0);		/* list immC array node */
		    for (lp = (yyvsp[(4) - (5)].list).v, i = 0; lp; lp = lp->le_next, i++) {
			if (tlp && (ttlp = tlp->le_next, 1) && (tsp = sy_pop(tlp)) != lp->le_sym) {	/* remove extern List_e */
			    ierror("different array member to member in extern declaration:", tsp->name);
			    sp->type = ERR;
			}
			if (lp->le_sym->type == UDF) {
			    if (lp->le_sym->ftype == UDFA) {
				lp->le_sym->type = sp->type;	/* turn into immC variable */
				lp->le_sym->ftype = iC_ftypes[sp->type];
				assert((lp->le_sym->em & EM) == 0);
				listGenOut(lp->le_sym, 0, 0);	/* list immC node and generate possible output */
			    } else {
				goto TypErr;
			    }
			} else
			if (sp->type != lp->le_sym->type) {	/* ARNC or LOGC asserted above */
			    if (sp->type != ERR) {
			      TypErr:
				ierror("array member type does not match type of whole array:", lp->le_sym->name);
				sp->type = ERR;
			    }
			} else if (lp->le_sym->em & EM) {	/* extern ARNC or LOGC ? */
			    lp->le_sym->em &= ~EM;		/* no longer extern */
			    listGenOut(lp->le_sym, 0, 0);	/* list immC node and generate possible output */
			}
			tlp = ttlp;
		    }
		    listGenOut(NULL, 1, 0);			/* terminate immC array node list */
		    assert(tlp == NULL && i == sp->v_cnt);
		    sp->list = (yyvsp[(4) - (5)].list).v;				/* head of the member list - inherits initialiser count */
		}
		if (iFunSymExt) {
		    char *	cp;
		    Symbol *	sp1;
		    cp = strchr(sp->name, '@');			/* locate original extension */
		    assert(cp && isprint(cp[1]));		/* extension must be at least 1 character */
		    if ((sp1 = lookup(++cp)) != 0 && (sp1->em & EM)) {
			warning("declaration of an extern variable in a function - ignored:", cp);
		    }
		    collectStatement(sp);
		    iFunSyText = 0;				/* no more function symbols */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "immCarray: immCarrayHead = { immCini }", &(yyval.sym));
#endif
	    }
    break;

  case 231:
/* Line 1792 of yacc.c  */
#line 3737 "comp.y"
    { (yyval.vap).v.lfl = (yyval.vap).v.nuv = 0; }
    break;

  case 232:
/* Line 1792 of yacc.c  */
#line 3738 "comp.y"
    { (yyval.vap) = (yyvsp[(1) - (1)].vap); }
    break;

  case 233:
/* Line 1792 of yacc.c  */
#line 3742 "comp.y"
    {
		Symbol *	sp;
		(yyval.sym).f = (yyvsp[(1) - (5)].typ).f; (yyval.sym).l = (yyvsp[(5) - (5)].vai).l;
		sp = (yyval.sym).v = (yyvsp[(2) - (5)].sym).v;
		sp->type  = (yyvsp[(1) - (5)].typ).v.type;			/* only ARNC and LOGC */
		sp->ftype = UDFA;			/* makes it a IMMCARRAY */
		assert(((yyvsp[(1) - (5)].typ).v.em & EM) == 0);		/* has been cleared in declCHead */
		sp->em    &= ~EM;			/* clear bit EM here */
		if (sp->type == ARNC || sp->type == LOGC) {
		    if ((yyvsp[(4) - (5)].vap).v.lfl == 0) {
			sp->v_cnt = (unsigned int)((yyvsp[(4) - (5)].vap).v.nuv);	/* array size or 0 */
			sp->em &= ~EI;			/* mark as constant */
		    } else
		    if ((yyvsp[(4) - (5)].vap).v.lfl == 1) {
			sp->v_elist = (yyvsp[(4) - (5)].vap).v.lpt;		/* pointer to constant parameter */
			sp->em |= EI;			/* mark as pointer */
		    }
		} else {
		    ierror("Definition of a non immC array is illegal:", (yyval.sym).f);
		    sp->type = ERR;
		}
#if YYDEBUG
		Symbol t = *(sp);
		if ((iC_debug & 0402) == 0402) pd("immCarrayHead: declCHead UNDEF [ immCindex ]", sp, (yyvsp[(1) - (5)].typ).v, &t);
#endif
	    }
    break;

  case 234:
/* Line 1792 of yacc.c  */
#line 3768 "comp.y"
    {
		int		typ;			/* gets here when IMMCARRAY was */
		Symbol *	sp;			/* previously declared extern */
		(yyval.sym).f = (yyvsp[(1) - (5)].typ).f; (yyval.sym).l = (yyvsp[(5) - (5)].vai).l;
		sp = (yyval.sym).v = (yyvsp[(2) - (5)].sym).v;
		typ  = (yyvsp[(1) - (5)].typ).v.type;			/* ARNC and LOGC */
		assert(sp && (typ == ARNC || typ == LOGC));
		if (sp->ftype != UDFA || sp->type != typ || sp->list->le_val == 0) {
		    ierror("declaration does not match previous declaration:", sp->name);
		    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
		} else
		if (sp->em & EM) {
		    if ((yyvsp[(4) - (5)].vap).v.lfl == 0) {
			if ((yyvsp[(4) - (5)].vap).v.nuv > 0 && sp->list->le_val != (unsigned int)((yyvsp[(4) - (5)].vap).v.nuv)) {
			    fprintf(stderr, "immCarrayHead: extern array size %u, array size %u\n", sp->list->le_val, (yyvsp[(4) - (5)].vap).v.nuv);
			    ierror("array size does not match previous extern declaration:", (yyval.sym).f);
			    sp->type = ERR;
			}
			sp->em &= ~EI;			/* mark as constant */
		    } else
		    if ((yyvsp[(4) - (5)].vap).v.lfl == 1) {
			/* can only get here if extern declaration had a fixed size and this one is dynamic */
			fprintf(stderr, "immCarrayHead: extern array size %u\n", sp->list->le_val);
			ierror("dynamic array size cannot match previous fixed extern declaration:", (yyval.sym).f);
			sp->type = ERR;
		    }
		    sp->em    &= ~EM;			/* clear bit EM here */
		    sp->v_cnt = sp->list->le_val;	/* array size from extern */
		} else {
		    ierror("array already defined - cannot define again:", (yyval.sym).f);
		    sp->type = ERR;
		}
#if YYDEBUG
		Symbol t = *(sp);
		if ((iC_debug & 0402) == 0402) pd("immCarrayHead: declCHead IMMCARRAY [ immCindex ]", sp, (yyvsp[(1) - (5)].typ).v, &t);
#endif
		iFunSyText = 0;				/* no more function symbols */
	    }
    break;

  case 235:
/* Line 1792 of yacc.c  */
#line 3813 "comp.y"
    { (yyval.list) = (yyvsp[(1) - (1)].list); }
    break;

  case 236:
/* Line 1792 of yacc.c  */
#line 3814 "comp.y"
    { (yyval.list) = (yyvsp[(1) - (2)].list); }
    break;

  case 237:
/* Line 1792 of yacc.c  */
#line 3818 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 238:
/* Line 1792 of yacc.c  */
#line 3819 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 239:
/* Line 1792 of yacc.c  */
#line 3820 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 240:
/* Line 1792 of yacc.c  */
#line 3821 "comp.y"
    { (yyval.sym)   = (yyvsp[(1) - (1)].sym); }
    break;

  case 241:
/* Line 1792 of yacc.c  */
#line 3825 "comp.y"
    {
		(yyval.list).f = (yyvsp[(1) - (1)].sym).f; (yyval.list).l = (yyvsp[(1) - (1)].sym).l;
		(yyval.list).v = checkDecl((yyvsp[(1) - (1)].sym).v);			/* first link in member list */
		assert((yyvsp[(1) - (1)].sym).f == 0 || ((yyvsp[(1) - (1)].sym).f >= iCbuf && (yyvsp[(1) - (1)].sym).l < &iCbuf[IMMBUFSIZE]));
		(yyval.list).v->le_val = 1;			/* count first member */
		(yyval.list).v->le_first = (yyvsp[(1) - (1)].sym).f;
		(yyval.list).v->le_last = (yyvsp[(1) - (1)].sym).l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "immCiniList: cVariable", &(yyval.list));
#endif
	    }
    break;

  case 242:
/* Line 1792 of yacc.c  */
#line 3836 "comp.y"
    {
		List_e *	lp;

		(yyval.list).f = (yyvsp[(1) - (3)].list).f; (yyval.list).l = (yyvsp[(3) - (3)].sym).l;
		lp = (yyval.list).v;
		while (lp->le_next) {
		    lp = lp->le_next;
		}
		lp = lp->le_next = checkDecl((yyvsp[(3) - (3)].sym).v);	/* link to end of member list */
		assert((yyvsp[(3) - (3)].sym).f == 0 || ((yyvsp[(3) - (3)].sym).f >= iCbuf && (yyvsp[(3) - (3)].sym).l < &iCbuf[IMMBUFSIZE]));
		(yyval.list).v->le_val++;				/* count later members */
		lp->le_first = (yyvsp[(3) - (3)].sym).f;
		lp->le_last = (yyvsp[(3) - (3)].sym).l;
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(LIS, "immCiniList: immCiniList , cVariable", &(yyval.list));
#endif
	    }
    break;

  case 243:
/* Line 1792 of yacc.c  */
#line 3914 "comp.y"
    {	/* automatic declaration of members */
		Symbol *	sp;
		Symbol *	tsp;
		List_e *	lp;
		List_e *	tlp;
		int		i;
		char		tempBuf[TSIZE];
		(yyval.sym).f = (yyvsp[(1) - (1)].sym).f; (yyval.sym).l = (yyvsp[(1) - (1)].sym).l;
		sp = (yyval.sym).v = (yyvsp[(1) - (1)].sym).v;
		assert((sp->em & EM) && (sp->type == ARNC || sp->type == LOGC || sp->type == ERR));
		if ((lp = sp->list) != NULL) {
		    warning("previous extern array declaration - ignored:", sp->name);
		    while (lp) {
			tlp = lp->le_next;		/* pick up next before free */
			sy_pop(lp);			/* free previous List_e's */
			lp = tlp;
		    }
		    sp->list = NULL;
		}
		if (iFunSymExt) {
		    ierror("cannot extern declare an array in a function block:", (yyval.sym).f);
		    sp->type = ERR;
		} else
		if (sp->em & EI) {
		    ierror("extern array cannot have size set by a constant parameter:", (yyval.sym).f);
		    sp->type = ERR;
		} else {
		    if (sp->v_cnt == 0) {
			ierror("must specify array size since there is no member initialiser list:", (yyval.sym).f);
			sp->type = ERR;
		    } else {
			for (i = 0; i < sp->v_cnt; i++) {
			    snprintf(tempBuf, TSIZE, "%s%d", sp->name, i);	/* base name + index */
			    if ((tsp = lookup(tempBuf)) == 0) {
				tsp = install(tempBuf, sp->type, iC_ftypes[sp->type]);
				tsp->em |= EM|EX;	/* make newly created member external immC */
			    }
			    if (sp->type != tsp->type && sp->type != ERR) {
				ierror("array member type does not match type of whole array:", tsp->name);
				sp->type = ERR;		/* can only happen if member alread existed */
			    }
			    if (sp->list == NULL) {
				sp->list = lp = sy_push(tsp);	/* head of the member list */
				lp->le_val = sp->v_cnt;
			    } else {
				lp = lp->le_next = sy_push(tsp);
			    }
			}
		    }
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "extimmCarray: extimmCarrayHead", &(yyval.sym));
#endif
	    }
    break;

  case 244:
/* Line 1792 of yacc.c  */
#line 3968 "comp.y"
    {
		Symbol *	sp;
		List_e *	lp;
		List_e *	tlp;
		(yyval.sym).f = (yyvsp[(1) - (5)].sym).f; (yyval.sym).l = (yyvsp[(5) - (5)].vai).l;
		sp = (yyval.sym).v = (yyvsp[(1) - (5)].sym).v;
		assert((sp->em & EM) && (sp->type == ARNC || sp->type == LOGC || sp->type == ERR));
		if ((lp = sp->list) != NULL) {
		    warning("previous extern array declaration - ignored:", sp->name);
		    while (lp) {
			tlp = lp->le_next;		/* pick up next before free */
			sy_pop(lp);			/* free previous List_e's */
			lp = tlp;
		    }
		    sp->list = NULL;
		}
		if (iFunSymExt) {
		    ierror("cannot extern declare an array in a function block:", (yyval.sym).f);
		    sp->type = ERR;
		} else
		if (sp->em & EI) {
		    ierror("extern array cannot have size set by a constant parameter:", (yyval.sym).f);
		    sp->type = ERR;
		} else {
		    if (sp->v_cnt > 0 && sp->v_cnt != (yyvsp[(4) - (5)].list).v->le_val) {
			fprintf(stderr, "extimmCarray: array size %u, initaliser count %u\n", sp->v_cnt, (yyvsp[(4) - (5)].list).v->le_val);
			ierror("array size does not match number of member initialisers:", (yyval.sym).f);
			sp->type = ERR;
		    }
		    sp->v_cnt = (yyvsp[(4) - (5)].list).v->le_val;			/* number of member initialisers is array size */
		    /* v_cnt is not preserved to outNet() - but list and count in list->le_val is */
		    for (lp = (yyvsp[(4) - (5)].list).v; lp; lp = lp->le_next) {
			if (lp->le_sym->type == UDF) {
			    if (lp->le_sym->ftype == UDFA) {
				lp->le_sym->type = sp->type;	/* turn into immC variable */
				lp->le_sym->ftype = iC_ftypes[sp->type];
				lp->le_sym->em |= EM|EX;	/* make member external immC */
			    } else {
				goto ExtTypErr;
			    }
			}
			if (sp->type != lp->le_sym->type && sp->type != ERR) {
			  ExtTypErr:
			    ierror("array member type does not match type of whole array:", lp->le_sym->name);
			    sp->type = ERR;
			}
		    }
		    sp->list = (yyvsp[(4) - (5)].list).v;				/* head of the member list - inherits initialiser count */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "extimmCarray: extimmCarrayHead = { immCini }", &(yyval.sym));
#endif
	    }
    break;

  case 245:
/* Line 1792 of yacc.c  */
#line 4024 "comp.y"
    {
		Symbol *	sp;
		(yyval.sym).f = (yyvsp[(1) - (5)].typ).f; (yyval.sym).l = (yyvsp[(5) - (5)].vai).l;
		sp = (yyval.sym).v = (yyvsp[(2) - (5)].sym).v;
		sp->type  = (yyvsp[(1) - (5)].typ).v.type;
		sp->ftype = UDFA;			/* makes it a IMMCARRAY */
		assert((yyvsp[(1) - (5)].typ).v.em & (EM|EX));		/* has been set in extDeclHead */
		sp->em    |= EM|EX;			/* set em for extern declaration */
		if (sp->type == ARNC || sp->type == LOGC) {
		    if ((yyvsp[(4) - (5)].vap).v.lfl == 0) {
			sp->v_cnt = (unsigned int)((yyvsp[(4) - (5)].vap).v.nuv);	/* array size or 0 */
			sp->em &= ~EI;			/* mark as constant */
		    } else
		    if ((yyvsp[(4) - (5)].vap).v.lfl == 1) {
			ierror("extern declaration of a parameterised function internal immC array is illegal:", (yyval.sym).f);
			sp->type = ERR;
		    }
		} else {
		    ierror("extern declaration of a non immC array is illegal:", (yyval.sym).f);
		    sp->type = ERR;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    pd("extimmCarrayHead: extDeclHead UNDEF [ immCindex ]", sp, (yyvsp[(1) - (5)].typ).v, (yyvsp[(2) - (5)].sym).v);
		}
#endif
	    }
    break;

  case 246:
/* Line 1792 of yacc.c  */
#line 4051 "comp.y"
    {
		(yyval.sym).f = (yyvsp[(1) - (5)].typ).f; (yyval.sym).l = (yyvsp[(5) - (5)].vai).l;
		char *	cp;
		Symbol *	sp;
		sp = (yyval.sym).v = (yyvsp[(2) - (5)].sym).v;
		assert(sp && (sp->type == ARNC || sp->type == LOGC));
		if (sp->ftype != UDFA) {
		    ierror("extern array declaration does not match previous declaration:", (yyvsp[(2) - (5)].sym).v->name);
		    (yyvsp[(2) - (5)].sym).v->type = ERR;			/* cannot execute properly */
		}
		if (iFunSymExt && (cp = strchr((yyvsp[(2) - (5)].sym).v->name, '@'))) {
		    warning("extern array declaration of internal function variable - ignored:", cp+1);
		} else
		if ((sp->em & EM) && (yyvsp[(2) - (5)].sym).v->type != ERR) {	/* prev extern but not ERROR */
		    sp->type  = (yyvsp[(1) - (5)].typ).v.type;
		    sp->ftype = UDFA;			/* makes it a IMMCARRAY */
		    assert((yyvsp[(1) - (5)].typ).v.em & (EM|EX));		/* has been set in extDeclHead */
		    sp->em    |= EM|EX;			/* set em for extern declaration */
		    if ((yyvsp[(4) - (5)].vap).v.lfl == 0) {
			sp->v_cnt = (unsigned int)((yyvsp[(4) - (5)].vap).v.nuv);	/* array size or 0 */
			sp->em &= ~EI;			/* mark as constant */
		    } else
		    if ((yyvsp[(4) - (5)].vap).v.lfl == 1) {
			ierror("extern declaration of a parameterised function internal immC array is illegal:", (yyval.sym).f);
			sp->type = ERR;
		    }
		} else {
		    warning("second extern array declaration - ignored:", (yyvsp[(2) - (5)].sym).v->name);
		} /* TD FIX */
		if (iFunSymExt) {
		    ierror("extern array declaration in function definition after assignment:", (yyvsp[(2) - (5)].sym).v->name);
		    sp->type = ERR;			/* stop use as a statement in function */
		    sp->em   |= EM|EX;
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    pd("extimmCarrayHead: extDeclHead IMMCARRAY [ immCindex ]", sp, (yyvsp[(1) - (5)].typ).v, (yyvsp[(2) - (5)].sym).v);
		}
#endif
	    }
    break;

  case 247:
/* Line 1792 of yacc.c  */
#line 4112 "comp.y"
    {
		(yyval.sym).f = (yyvsp[(1) - (4)].sym).f; (yyval.sym).l = (yyvsp[(4) - (4)].vai).l;
		Symbol *	sp;
		List_e *	lp;
		int		index;
		int		i;
		(yyval.sym).v = sp = (yyvsp[(1) - (4)].sym).v;
		assert(sp && sp->ftype == UDFA);
		if (iFunSymExt && sp->list == NULL) {
		    warning("anonymous immC array - it's members cannot be accessed in iC code:", sp->name);
		}
		if ((lp = sp->list) == NULL) {
		    if (((yyval.sym).v = lookup("iCerr")) == 0) {
			icerr = (yyval.sym).v = install("iCerr", ERR, GATE);
		    }
		    (yyval.sym).v->type = ERR;			/* in case of error */
		    ierror("immC array must be initialised to use its members:", sp->name);
		} else {
		    if ((yyvsp[(3) - (4)].vap).v.lfl == 0) {
			index = (yyvsp[(3) - (4)].vap).v.nuv;		/* array index */
			if (index < 0 || index >= lp->le_val) {
			    if (((yyval.sym).v = lookup("iCerr")) == 0) {
				icerr = (yyval.sym).v = install("iCerr", ERR, GATE);
			    }
			    (yyval.sym).v->type = ERR;		/* in case of error */
			    ierror("immC array member is not in range:", sp->name);
			} else {
			    for (i = 0; i < index; i++) {
				lp = lp->le_next;	/* select indexed member */
				if (lp == NULL || lp->le_sym == NULL) {
				    execerror("immCarrayVariable: member not initialised by compiler", sp->name, __FILE__, __LINE__);
				    /* compiler error - flushes rest of file */
				}
			    }
			    (yyval.sym).v = lp->le_sym;		/* return the member Symbol */
			    (yyval.sym).v->em &= ~EI;		/* mark as member returned by constant index */
			}
		    } else
		    if ((yyvsp[(3) - (4)].vap).v.lfl == 1) {
			(yyval.sym).v->v_elist = (yyvsp[(3) - (4)].vap).v.lpt;	/* pointer to constant parameter */
			(yyval.sym).v->em |= EI;			/* mark as pointer */
		    }
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "immCarrayVariable: IMMCARRAY [ constExpr ]", &(yyval.sym));
#endif
	    }
    break;


/* Line 1792 of yacc.c  */
#line 5745 "comp.tab.c"
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
#line 4167 "comp.y"


#ifdef NEWSTYLE
#define CBUFSZ 166
#define YTOKSZ 166
#else
#define CBUFSZ 125				/* listing just fits on 132  cols */
#define YTOKSZ 66
#endif
#define ABUFSZ 2048				/* allow for a large comment block */
static char	chbuf[2][CBUFSZ];		/* used in get() errline() andd yyerror() */
static char	prevNM[BUFS];
static char *	getp[2] = {NULL,NULL};		/* used in get() unget() andd yyerror() */
static char	iCtext[YTOKSZ];			/* lex token */
static int	iCleng;				/* length */
static int	lineflag;
static char *	errFilename;
static int	errFlag = 0;
static int	errRet = 0;
static int	eofLineno = 0;
static int	savedLineno = 0;
static int	errline = 0;

static char	auxBuf[ABUFSZ];
static char *	auxp = 0;
static char *	auxe = 0;

/********************************************************************
 *	lexflag is bitmapped and controls the input for lexers
 *******************************************************************/
int		lexflag = 0;
int		lineno = 0;			/* count first line on entry to get() */

int		c_number = 0;			/* case number for cexe.c */
int		outFlag = 0;			/* global flag for compiled output */
static jmp_buf	begin;
static int	lex_typ[] = { DEF_TYP };	/* tokens corresponding to type */
static int	lex_act[] = { DEF_ACT };	/* tokens corresponding to ftype */

/********************************************************************
 *
 *	Compile an iC language source file whose name is in 'inpPath'
 *	if inpPath is a null pointer use standard input (stdin)
 *	a copy of the source file name (or "stdin") is kept in inpNM for
 *	error messages. This name and the variable 'lineno' are
 *	modified by a #line 1 "file.ic" pre-processor directive.
 *
 *	listing text is directed to 'lstNM' (default stdout)
 *	error   text is directed to 'errNM' (default stderr)
 *
 *	a prototype for C function execution is read and extended
 *	with the C fragments possibly contained in the iC program
 *
 *******************************************************************/

int
iC_compile(
    char *	inpPath,			/* input file path */
    char *	lstNM,				/* list file name */
    char *	errNM,				/* error file name */
    char *	outNM)				/* C output file name */
{
    char	execBuf[BUFS];
    char	lineBuf[BUFS];
    int		fd;
    int		r  = 1;
    int		r1 = 1;

    lineno = 0;
#if YYDEBUG
    if ((iC_debug & 0402) != 0402)		/* unless logic generation */
#endif
	lexflag = C_FIRST;			/* suppress initial pre-processor lines */
    lineflag = 1;				/* previous line was complete */

    if (lstNM) {				/* list file nominated ? */
	if ((chmod(lstNM, 0644) &&		/* yes - make list file writable */
	errno != ENOENT) ||			/* if it exists - so it can be re-opened */
	(iC_outFP = fopen(lstNM, "w")) == NULL) {
	    perror("chmod or fopen");
	    return Lindex;
	}
    }						/* else iC_outFP = stdout from initialisation */
    iC_lstFP = iC_outFP;			/* back up iC_outFP for restoring after 'no list' */
    outFlag = outNM != 0;			/* global flag for compiled output */
    errFilename = errNM;			/* open on first error */
    iC_init();					/* install constants and built-ins - allows ierror() */
    if (inpPath) {
	strncpy(inpNM, inpPath, BUFS);
	r = 0;
	if (strlen(iC_defines) == 0) {
	    /* pre-compile if iC files contains any %include, %define %if etc */
#ifdef	WIN32
	    fflush(iC_outFP);
	    /* CMD.EXE does not know '' but does not interpret $, so use "" */
	    /* don't use system() because Win98 command.com does not return exit status */
	    /* use spawn() instead - spawnlp() searches Path */
	    snprintf(execBuf, BUFS, "\"$s=1; while (<>) { if (m/^\\s*%\\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error)\\b/) { $s=0; last; } } exit $s;\"");
	    r = _spawnlp( _P_WAIT, "perl",  "perl",  "-e", execBuf, inpPath, NULL );
	    if (r < 0) {
		ierror("cannot spawn perl -e ...", inpPath);
		perror("spawnlp");
		return Iindex;			/* error opening input file */
	    }
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### test: perl -e %s %s; $? = %d\n", execBuf, inpPath, r);
#endif
#else	/* not WIN32 */
	    snprintf(execBuf, BUFS, "%s %s",
		"perl -e '$s=1; while (<>) { if (m/^\\s*%\\s*(define|undef|include|ifdef|ifndef|if|elif|else|endif|error)\\b/) { $s=0; last; } } exit $s;'",
		inpPath);
	    r = system(execBuf);		/* test with perl script if %define %include %if etc in input */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### test: %s; $? = %d\n", execBuf, r);
#endif
#endif	/* WIN32 */
	}
#if YYDEBUG
	else {
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### iC_defines = %s; $? = %d\n", iC_defines, r);
	}
#endif
	if (r == 0) {
	    /* iC_defines is not empty and has -Dyyy or -Uyyy or %include etc was found by perl script */
	    /* pass the input file through the 'immac -M' to resolve %includes and macros */
	    if ((fd = mkstemp(T0FN)) < 0 || close(fd) < 0 || unlink(T0FN) < 0) {
		ierror("compile: cannot make or unlink:", T0FN);
		perror("unlink");
		return T0index;			/* error unlinking temporary file */
	    }
	    snprintf(execBuf, BUFS, "immac -M%s %s -I/usr/local/include -o %s %s 2> %s",
		iC_aflag, iC_defines, T0FN, inpPath, T6FN);
	    r1 = system(execBuf);		/* Pre-compile iC file with immac -M or immac -Ma */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### pre-compile: %s; $? = %d\n", execBuf, r1>>8);
#endif
	    if (r1 != 0) {
		if ((T6FP = fopen(T6FN, "r")) == NULL) {
		    return T6index;		/* error opening immac error file */
		}
		while (fgets(lineBuf, sizeof lineBuf, T6FP)) {
		    ierror("immac:", lineBuf);	/* immac error message */
		}
		fclose(T6FP);
		if (!(iC_debug & 04000)) {
		    unlink(T6FN);
		}
		return Iindex;
	    }
	    if (!(iC_debug & 04000)) {
		unlink(T6FN);
	    }
	    if ((T0FP = fopen(T0FN, "r")) == NULL) {
		return T0index;			/* error opening intermediate file */
	    }
	} else if ((T0FP = fopen(inpNM, "r")) == NULL) {
	    return Iindex;			/* error opening input file */
	}
    }						/* else inpPath == NULL T0FP is stdin inpNM is "stdin" */
    strncpy(prevNM, inpNM, BUFS);
    if (iC_debug & 046) {			/* begin source listing */
	fprintf(iC_outFP, "******* %-15s ************************\n", inpNM);
    }
    setjmp(begin);
    for (initcode(); iCparse(); initcode()) {
	;
    }
    if (r == 0 && !(iC_debug & 04000)) {
	unlink(T0FN);
    }
    if (inpPath) fclose(T0FP);
    T0FP = 0;
    return errRet;
} /* iC_compile */

/********************************************************************
 *
 *	Get routine for iC and C grammars
 *
 *	fp	FILE pointer for the source of data
 *	x	0	uses getp[0] as its buffer - used by iC-compile
 *		1	uses getp[1] - used by lexc.l for C-compile
 *
 *******************************************************************/

int
get(FILE* fp, int x)
{
    int		c;
    int		temp1;
    int		slen;
    int		prevflag;
    char *	cp;
    char	tempNM[BUFS];
    char	tempBuf[TSIZE];

    static int	genCount = 0;


    /************************************************************
     *  The following block became necessary because 2 problems
     *  occurred when compiling the "if (condition) block" code.
     *  The parser needs to do lookahead to the next token in
     *  case this is an "else", in which case the statement is
     *  extended. Unfortunately that token is usually on a new line,
     *  and that line will  be listed as soon as that token is read.
     *  This means that the generated logic output for the preceding
     *  "if" statement (without an "else") will be listed after a quite
     *  unrelated statement following the "if" statement.
     *
     *  This has been an untidiness, which has bugged me for years.
     *
     *  A more serious consequence is, that the text arithmetic, which
     *  works on complete statements will come undone for the follow on
     *  statement, because if that statement is an arithmetic assignment,
     *  the text buffer is cleared by the end of the "if" statement after
     *  the first token of the assignment is proceessd.
     *  This has not led to any serious compiler problems in all
     *  the years of development (the text of that first token is
     *  only needed in embedded assignments by which time the start
     *  of the statement has been passed). But it did show up in
     *  debugging output with the -d2 flag, where a negative increment
     *  was calculated for the length of a statement, which resulted
     *  in a huge buffer overflow.
     *
     *  The solution is to mark the end of recognising a completed
     *  cBlock by setting cBlockFlag. At this point the remaining
     *  input text is saved in auxBuf after a space character.
     *  A lookahead of as many lines as needed is then carried
     *  out to find a token (which is any non white space string).
     *  If that token is "else" proceed as before. Otherwise replace
     *  the initial space in auxBuf with the character '\x1f'. In
     *  yylex this will generate the BF yacc token, which must appear
     *  at the end of ffexpr in a statement (another change).
     *
     *  Unfortunately the whole comment recognising gumph must be put
     *  in too, because otherwise a comment between the initial
     *  "if" part and the "else" will lead to a syntax error.
     *
     *  This way the logic listing text for ffexpr is generated in
     *  op_asgn(0, ..) before the next line is actually read below.
     *  Thus the follow up listing lines will be in the correct place.
     *
     *  Input for get() is taken from auxBuf[] via auxp until that
     *  is exhausted. Then further input is take from the input file.
     ************************************************************/
    if (x == 0 && cBlockFlag) {
	auxp = auxBuf;
	*auxp++ = ' ';				/* placeholder to be overwritten by '\x1f' */
	auxe = 0;
	if (getp[0] == 0) {			/* happens if file ends without '\n' */
	    getp[0] = strchr(chbuf[0], '\0');	/* point to string terminator */
	}
	strncpy(auxp, getp[0], ABUFSZ-1);	/* transfer rest if input to auxBuf[] */
	while (1) {
	    while (sscanf(auxp, " %s", tempBuf) != 1) {
		cp = auxe;
		auxe = auxp + strlen(auxp);	/* the rest of the current line has no token */
		if ((auxp = fgets(auxe, &auxBuf[ABUFSZ] - auxp, fp)) == NULL) {
		    auxe = cp;			/* file has terminated after initial if () cBlock */
		    tempBuf[0] = '\0';		/* no else will be found - complete if statement */
		    goto endInput;
		}
	    }
	    if (memcmp(tempBuf, "/*", 2) == 0) { /* a C comment block has started */
		cp = strchr(auxp, '*');
		assert(cp);			/* because of previous compare */
		do {				/* start C style comment */
		    while ((cp = strchr(cp + 1, '*')) == 0) {
			cp = auxe;
			auxe = auxp + strlen(auxp);	/* the line has no comment end */
			if ((auxp = fgets(auxe, &auxBuf[ABUFSZ] - auxp, fp)) == NULL) {
			    auxe = cp;		/* file terminated in C comment after if ... */
			    tempBuf[0] = '\0';	/* no else will be found - complete if statement */
			    goto endInput;
			}
			cp = auxp - 1;
			continue;
		    }
		} while (*++cp != '/');
		auxp = cp + 1;			/* C comment end was found */
		continue;
	    } else
	    if (memcmp(tempBuf, "//", 2) == 0 || /* a C++ comment line was found */
		memcmp(tempBuf, "#", 1) == 0) {	/* or a C-preprocessor line - # 1 */
		auxp += strcspn(auxp, "\n");	/* both are terminated by '\n' */
		continue;
	    }
	  endInput:
	    break;				/* no comments */
	}
	slen = strcspn(auxBuf, "\n");
	if (auxBuf[slen] == '\n') {
	    slen++;
	}
	if (strcmp(tempBuf, "else") != 0) {	/* a non-white string was found - it is a token */
	    auxBuf[0] = '\x1f';			/* this character generates the BF yacc token */
	    getp[0] = strncpy(chbuf[0], auxBuf, slen); /* copy the BF + first line back to chbuf[] */
	    chbuf[0][slen] = '\0';		/* terminate */
	}
	auxp = auxe ? auxBuf + slen : 0;	/* get ready for more lines in auxBuf[] */
	cBlockFlag = 0;
    }
    prevflag = lineflag;
    while (getp[x] == 0 || (c = *getp[x]++) == 0) {
	/************************************************************
	 *  getp has reached end of previous chbuf filling
	 *  fill chbuf with a new line
	 *  NOTE: getp === NULL at start of file (chbuf has EOF line)
	 *
	 *  Fill either from auxBuf if some lines were saved or from fp
	 *  The first token will be in the last line of auxBuf[]
	 ************************************************************/
	if ((prevflag = lineflag) != 0) {
	    lineno++;				/* count previous line */
	}
	/************************************************************
	 *  Input for get() is taken from auxBuf[] via auxp until that
	 *  is exhausted. Then further input is take from the input file.
	 ************************************************************/
	if (x == 0 && auxp) {
	    slen = strcspn(auxp, "\n");
	    if (auxp[slen] == '\n') {
		slen++;
	    }
	    getp[0] = strncpy(chbuf[0], auxp, slen); /* copy the BF + first part back to chbuf[] */
	    chbuf[0][slen] = '\0';		/* terminate */
	    auxp += slen;			/* get ready for more lines in auxBuf[] */
	    if (strlen(auxp) == 0) {
		auxp = 0;			/* that was it - now input is taken from file again */
	    }
	} else
	if ((getp[x] = fgets(chbuf[x], CBUFSZ, fp)) == NULL) {
	    if ((lexflag & C_PARSE) == 0) {
		eofLineno = lineno;		/* iC parse */
	    } else {
		lineno = eofLineno;		/* C parse with lex */
	    }
	    errline = lineno;			/* no listing line at EOF */
	    return (EOF);
	}
	lineflag = chbuf[x][strlen(chbuf[x])-1] == '\n' ? 1 : 0; /* this line terminated with \n */

	/********************************************************
	 *  handle different preprocessor lines
	 *  identify leading '#' for efficiency
	 *  Note: white space in scanf format matches any amount of
	 *        white space including none (have used single blank)
	 *  NOTE: chbuf[] must be large enough to hold a complete
	 *        pre-processor line for the following sscanf()s
	 ********************************************************/
	if (prevflag && sscanf(chbuf[x], " %1s", tempBuf) == 1 && *tempBuf == '#') {
	    if ((lexflag & C_PARSE) && strncmp(chbuf[x], "##", 2) == 0) {
		lexflag |= C_BLOCK;		/* block source listing for lex */
		lineno = savedLineno;
	    }
	    /********************************************************
	     *  WIN32 - cl pre-processor
	     *    handle C-pre-processor #line 1 "c:\\usr\\include\\stdio.h"
	     *  Linux or Cygnus - gcc or Intel icc pre-processor
	     *    handle C-pre-processor # 1 "/usr/include/stdio.h"
	     ********************************************************/
	    if ((lexflag & C_PARSE) == 0) {
		/* iC parse only */
#ifdef	WIN32
		if (strchr(chbuf[x], '\\') != 0) {
		    int	    c;
		    int	    slFlag;
		    char *  srcp;
		    char *  destp;

		    slFlag = 0;
		    srcp = destp = chbuf[x];	/* modify source line */
		    while ((c = *srcp++) != 0) {
			if (c != '\\') {
			    *destp++ = c;
			    slFlag = 0;		/* copy '\' after another character */
			} else if (slFlag == 0) {
			    *destp++ = '/';	/* convert '\\' to '/' under WIN32 */
			    slFlag = 1;		/* skip 2nd, 4th ... '\' */
			} else {
			    slFlag = 0;		/* copy 3rd, 5th ... '\' */
			}
		    }
		    *destp++ = '\0';		/* terminate string */
		}
		/********************************************************
		 *  iC-compile
		 *  handle pre-processor #line 1 "file.ic"	// WIN32
		 *  handle pre-processor # 1 "file.ic"		// GCC
		 ********************************************************/
		if (sscanf(chbuf[x], " # line %d \"%[-/:A-Za-z_.0-9<>]\"",.&temp1, inpNM) == 2)
#else	/* not WIN32 */
		if (sscanf(chbuf[x], " # %d \"%[-/A-Za-z_.0-9<>]\"", &temp1, inpNM) == 2)
#endif	/* WIN32 */
		{
		    lineno = temp1 - 1;		/* handled in iC-code */
		    lexflag |= C_LINE;
		    if (strcmp(inpNM, prevNM) != 0 && strchr(inpNM, '<') == 0) {
			lexflag &= ~C_FIRST;	/* report # 1 if file has changed */
			/* don't change to inpNM <built-in> and sytem headers */
			if (temp1 <= 1) {
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, ">>> get: # %3d  %-10s >> %-10s, useStackIndex = %d, uses = %o\n", temp1, prevNM, inpNM, iC_useStackIndex, iC_uses);
#endif
			    assert(iC_useStackIndex < USESTACKSZ);
			    iC_useStack[iC_useStackIndex++] = iC_uses; /* save use values */
			} else {
			    assert(iC_useStackIndex > 0);
			    iC_uses = iC_useStack[--iC_useStackIndex]; /* restore use values */
			    blockUnblockListing();
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "<<< get: # %3d  %-10s << %-10s, useStackIndex = %d, uses = %o\n", temp1, inpNM, prevNM, iC_useStackIndex, iC_uses);
#endif
			}
			strncpy(prevNM, inpNM, BUFS);	/* inpNM has been modified */
		    }
		    getp[x] = NULL;		/* bypass parsing this line in iC */
		}
	    } else
	    /********************************************************
	     *  handle C-pre-processor # pragma ## comes out on MAC OsX
	     ********************************************************/
	    if (sscanf(chbuf[x], " # %s", tempBuf) == 1 && strcmp(tempBuf, "pragma") == 0) {
		getp[x] = NULL;			/* ignore and bypass parsing # pragma */
	    } else
	    /********************************************************
	     *  there should be no other # lines remaining in iC file
	     ********************************************************/
	    if ((lexflag & C_PARSE) == 0) {
		/* handle only local includes - not full C-precompiler features */
		ierror("get: invalid # pre-processor line in iC ???", NULL);
		getp[x] = NULL;			/* bypass parsing this line in iC */
	    } else
	    /********************************************************
	     *  C-compile
	     *  handle pre-processor #line 1 "file.ic"
	     ********************************************************/
	    if ((slen = sscanf(chbuf[x],
#ifdef	WIN32
		" # line %d \"%[-/:A-Za-z_.0-9<>]\"	%s",
#else	/* not WIN32 */
		" # line %d \"%[-/A-Za-z_.0-9<>]\"	%s",
#endif	/* WIN32 */
		&temp1, tempNM, tempBuf)) >= 2) {
		savedLineno = lineno;
		lineno = temp1 - 1;

		if (slen == 3 &&		/* has comment string after #line x "init.c" */
		    iC_genName &&
		    strcmp(tempNM, iC_genName) == 0) {	/* in the system file "init.c" */
		    int	i;

		    genCount++;			/* count C functions in pre-compile code */
		    for (i = 1; i <= iC_genCount; i++) {
			/********************************************************
			 * System C CODE
			 * For function blocks with more than 1 C function, the
			 * same test will be repeated. All C functions for the
			 * function block will be listed if the function block is
			 * in use on the basis of the first entry for the line.
			 ********************************************************/
			if (temp1 == iC_genLineNums[i] &&	/* find i for this line */
			    functionUse[i].c_cnt) {		/* is function block in use? */
			    goto listCfunction;			/* yes */
			}
		    }
		    continue;			/* no listing for system C CODE not in use */
		} else if (genCount) {
		    assert(genCount == iC_genCount);	/* correct in init.c */
		    genCount = 0;
		}
	      listCfunction:

		strncpy(inpNM, tempNM, BUFS);	/* defer changing name until used */
		if (iC_debug & 06) {
		    if (lexflag & C_FIRST) {
			fprintf(iC_outFP, "******* C CODE          ************************\n");
		    }
		    if (strcmp(inpNM, prevNM) != 0) {
			lexflag &= ~C_BLOCK;	/* output #line for changed filename */
		    } else {
			fprintf(iC_outFP, "\n");	/* separate blocks in lex listing */
		    }
		}
		lexflag |= C_LINE|C_LINE1;	/* only in C-compile */
		strncpy(prevNM, inpNM, BUFS);
	    }
	}
	if ((iC_debug & 04) &&
	    (lexflag & C_BLOCK1) == 0 &&
	    sscanf(chbuf[x], cexeString[outFlag], &temp1) == 1) {
	    cFn = temp1;
	    assert(cFn < functionUseSize);
	}
	if ((iC_debug & 046) &&
	    ((lexflag & (C_BLOCK|C_BLOCK1)) == 0	/* iC listing */
#if YYDEBUG
						 || ((iC_debug & 0402) == 0402)
#endif
									    )) {
	    /********************************************************
	     *  output source listing line in debugging output
	     *  before any tokens are handed to the parser
	     *  then messages appear neatly after the listing line
	     ********************************************************/
	    if ((lexflag & C_LINE) == 0) {
		if (cFn && (iC_debug & 0402) != 0402) {
		    if (cFn > iC_genCount || functionUse[cFn].c_cnt) {
			fprintf(iC_outFP, "%03d\t(%d) %s", lineno, cFn, chbuf[x]); /* C function head listing */
		    }
		    cFn = 0;
		} else if (((iC_debug & 06)  && (lexflag & C_PARSE)) ||
		    ((iC_debug & 040)  && (lexflag & C_PARSE) == 0)) {
		    if (memcmp(chbuf[x], "//* ", 4) != 0) {
			fprintf(iC_outFP, "%03d\t%s", lineno, chbuf[x]);	/* iC listing or C listing with space */
		    } else {
			fprintf(iC_outFP, "\t%s", chbuf[x]);			/* immac Warning or Error */
			--lineno;						/* does not count as a listing line */
		    }
		}
		iFlag = 1;				/* may need correction by pplstfix */
	    } else if ((lexflag & (C_PARSE|C_FIRST)) != C_FIRST) {
		fprintf(iC_outFP, "\t%s", chbuf[x]);	/* # line or # 123 "name" */
	    }
	    if (lineflag == 0) putc('\n', iC_outFP);	/* current line not complete */
	}
	if ((lexflag & C_PARSE) == 0 && lineno > 0) {
	    lexflag &= ~C_FIRST;
	} else if (lexflag & C_LINE1) {
	    lexflag &= ~(C_FIRST|C_BLOCK|C_LINE1);	/* output source listing for lex */
	}
	lexflag &= ~C_LINE;
    }
    /****************************************************************
     *  return 1 character at a time from chbuf[x] and returns it
     *  transfer it to the token buffer iCtext and count iCleng if not lex
     ****************************************************************/
    if ((lexflag & C_PARSE) == 0) {
	iCtext[iCleng++] = c;			/* iC compile via iCparse() */
	if (iCleng >= sizeof(iCtext)) iCleng--;
	iCtext[iCleng] = '\0';
    }
    return c;
} /* get */

/********************************************************************
 *
 *	Unget for iC grammar only
 *
 *	c	character to push back - can go back to beginning of line
 *		always uses getp[0] as its buffer - used by iC-compile
 *
 *******************************************************************/

static void
unget(int c)
{
    if (c != EOF) {
	if(getp[0] <= chbuf[0]) {
	    execerror("unget: ???", NULL, __FILE__, __LINE__);
	}
	*--getp[0] = c;				/* use always insures 1 free place */
	iCtext[--iCleng] = '\0';
    }
} /* unget */

/********************************************************************
 *
 *	Lexer for iC grammar
 *
 *******************************************************************/

static int
yylex(void)
{
    int		c;
    int		c1;

    if (ccfrag) {
	if (ccfrag == '%') {
	    fprintf(ccFP, "%%{\n");		/* output "%{\n" now */
	}
	fprintf(ccFP, "#line %d \"%s\"\n", lineno, inpNM);
	unget('{');
	if (copyCfrag('{', ccfrag == '%' ? '%' : ';', '}', ccFP) == 0) {
	    ccfrag = 0;
	    return 0;				/* EOF in copy C block or statement */
	}
	iClval.vai.v = c_number << FUN_OFFSET;	/* return case # */
	ccfrag = 0;
	c = CCFRAG;
	goto retfl;
    }
    iCleng = 0;
    while ((c = get(T0FP, 0)) !=  EOF) {
	Symbol *	symp;
	Symbol *	sp;
	List_e *	lp;
	char *		cp;
	char *		ep;
	int		len;
	int		rest;
	char		tempBuf[TSIZE];		/* make long enough for format below */

	if (isspace(c)) {			/* space \f \n \r \t \v */
	    iCleng = 0;
	    continue;				/* ignore white space */
	}
	if (isdigit(c) || c == '\'') {
	    unget(c);				/* must be at least a single 0 */
	    iClval.vai.v = (unsigned int)getNumber(getp[0], &ep, 0); /* decimal octal, hex or character constant */
	    assert (ep <= &chbuf[0][CBUFSZ]);
	    for (cp = getp[0]; cp < ep; cp++) {
		get(T0FP, 0);			/* transfer to iCtext to mark text boundaries */
	    }
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "yylex: '%s' converted to %u\n", iCtext, iClval.vai.v);
		fflush(iC_outFP);
	    }
#endif
	    c = NUMBER;				/* value used in immCarrayHead TODO check */
	    goto retfl;
	}
	if (isalpha(c) || c == '_' || c == '$') {	/* first may not be '@' */
	    unsigned char	wplus = 0;
	    unsigned int	qtoken = 0;
	    unsigned long	qmask = 0;	/* mask to id bit, byte, word or long */
	    unsigned long	iomask = 0;	/* output Q is 0x0f, input I is 0xf0 */
	    unsigned char	typ = UDF;
	    unsigned char	ftyp = UDFA;
	    int			ixd = iC_maxIO;	/* limit for I/O index (-1 no limit) */
	    char		y0[2];
	    char		y1[2];
	    int			yn;
	    int			yt;
	    char		y2[2];

#if YYDEBUG
	    rest = 0;				/* used for =I5 0402 debug output */
#endif
	    /* step forward to first character (in c) after identifier to fill iCtext */
	    while (isalnum(c = get(T0FP, 0)) || c == '_' || c == '$' || c == '@');
	    if ((len = sscanf(iCtext, "%1[IQT]%1[XBWL]%d_%d%1[A-Z_a-z]",
		y0, y1, &yn, &yt, y2)) >= 4) {
		if (len == 4) {			/* 4 - QX%d_%d is not allowed */
		    ierror("I/O like names with _ instead of . clash with internal representation", iCtext);
		}				/* 5 - QX%d_%dA is OK - not I/O */
	    } else if (sscanf(iCtext, "%1[IQT]%1[XBWL]%d", y0, y1, &yn) == 3) {
		if (y1[0] == 'B') {
		    wplus = 1;
		    qmask = 0x02;		/* QB or IB */
		    iomask = 0x0f;
		    goto foundQIT;
		}
		if (y1[0] == 'W') {
#if INT_MAX == 32767 && ! defined (LONG16)
		  useWord:
#endif
		    wplus = 1;
		    qmask = 0x0404;		/* QW or IW */
		    iomask = 0x0f0f;
		    goto foundQIT;
		}
		if (y1[0] == 'L') {
#if INT_MAX != 32767 || defined (LONG16)
		    wplus = 1;
		    qmask = 0x08080808;		/* QL or IL */
		    iomask = 0x0f0f0f0f;
		    goto foundQIT;
#else
		    warning("32 bit I/O not supported in 16 bit environment:", iCtext);
		    goto useWord;		/* compile for 16 bit word */
#endif
		} else {			/* QX, IX or TX */
		    /********************************************************************
		     *  the Intel C/C++ v8.1 pre-compiler expands IX0.1 to IX0 .1
		     *  Generally if yacc were to handle the '.' as a separate token,
		     *  white space would be allowed as it would in C for the operator '.'
		     *  If we allow white space here we will have the same effect without
		     *  changing the grammar.
		     *******************************************************************/
		    len = iCleng;
		    while (isspace(c)) {		/* space \f \n \r \t \v */
			c = get(T0FP, 0);		/* iCleng++ if ! lexflag & C_PARSE */
		    }
		    if (c == '.') {			/* [IQT]Xn. ? */
			if (len < iCleng) {
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				fprintf(iC_outFP, ">I1:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				rest++;
			    }
#endif
			    iCtext[len-1] = c;		/* move '.' back over spaces */
			    iCtext[len] = '\0';
			    iCleng = len;		/* in case more than 1 space */
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				fprintf(iC_outFP, "<I1:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				fflush(iC_outFP);
			    }
#endif
			}
			/********************************************************************
			 *  delete space after the '.' although not really required (yet).
			 *******************************************************************/
			len++;				/* get one or more characters */
			while (isspace(c = get(T0FP, 0)));	/* space \f \n \r \t \v */
			if (isdigit(c1 = c)) {		/* can only be [IQT]X%d. */
			    int		i;
			    if (len < iCleng) {
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    fprintf(iC_outFP, ">I2:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				    rest++;
				}
#endif
				iCtext[len-1] = c;	/* move digit back over spaces */
				iCtext[len] = '\0';
				iCleng = len;		/* in case more than 1 space */
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    fprintf(iC_outFP, "<I2:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				    fflush(iC_outFP);
				}
#endif
			    }
			    for (i = 0; isdigit(c = get(T0FP, 0)); i++);
			    if (c1 >= '8' || i > 0) {
				ierror("I/O bit address must be less than 8:", iCtext);
			    } else {
				qmask = 0x01;	/* QX, IX or TX */
				iomask = 0x0f;
			    }
			foundQIT:
			    ftyp = GATE - wplus;	/* GATE or ARITH */
			    if (y0[0] == 'Q') {
				qtoken = lex_act[OUTX - wplus]; /* LOUT or AOUT */
			    } else {
				qtoken = 1;		/* I... */
				typ = INPX - wplus;	/* INPX or INPW */
				if (y0[0] != 'T') {
				    qmask <<= 4;	/* IX, IB, IW or IL */
				    iomask <<= 4;	/* input I */
				} else {
				    qmask = iomask = 0;	/* TX (TB or TW) */
				    ixd = y1[0] == 'X' ? 1 : 0;	/* TX0.7 is max */
				}
			    }
			} else {
			    while (len < iCleng) {	/* '.' not followed by %d - error anyway */
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    fprintf(iC_outFP, ">I3:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				    rest++;
				}
#endif
				c = iCtext[iCleng-1];	/* char after space and spaces */
				unget(c);		/* unget char, does --iCleng */
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    fprintf(iC_outFP, "<I3:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				    fflush(iC_outFP);
				}
#endif
			    }				/* finish up with first space */
			    unget(c);			/* unget it as well or char after '.' */
			    c = iCtext[iCleng-1];	/* restore correct c to unget below */
			}
		    } else if (len < iCleng) {		/* not [IQT]X%d. */
			while (len < iCleng) {
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				fprintf(iC_outFP, ">I4:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				rest++;
			    }
#endif
			    c = iCtext[iCleng-1];	/* char after space while space */
			    unget(c);			/* unget char, does --iCleng */
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				fprintf(iC_outFP, "<I4:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d\n", lineno, iCtext, c, len, iCleng);
				fflush(iC_outFP);
			    }
#endif
			}				/* leave first space */
			c = iCtext[iCleng-1];		/* restore correct c to unget below */
		    }
		}
		if (ixd >= 0) {			/* no size limit for -ve ixd */
		    if (yn >= ixd) {
			snprintf(tempBuf, TSIZE, "I/O byte address must be less than %d:",
			    ixd);
			ierror(tempBuf, iCtext);/* hard error if outside range */
		    } else			/* no byte word long boundaries */
		    if (ixd <= IXD) {		/* test rest - old style tests (yn in array bounds) */
			if (wplus && y1[0] == 'W' && (yn & 0x01) != 0) {
			    ierror("WORD I/O must have even byte address:", iCtext);
			} else			/* test rest if yn < ixd (array bounds) */
			if (wplus && y1[0] == 'L' && (yn & 0x03) != 0) {
			    ierror("LONG I/O address must be on 4 byte boundary:", iCtext);
			} else			/* and yn is on correct byte boundary */
			if (qmask & 0x88888888) {
			    if (*(unsigned long*)&iC_QX_[yn] & ~qmask & iomask) {
				ierror("I/O addresses more than 1 of bit, byte or word:",
				    iCtext);	/* mixed mode warning */
			    }
			    *(unsigned long*)&iC_QX_[yn] |= qmask; /* note long I/O */
			} else
			if (qmask & 0x4444) {
			    if (*(unsigned short*)&iC_QX_[yn] & ~qmask & iomask) {
				ierror("I/O addresses more than 1 of bit, byte or word:",
				    iCtext);	/* mixed mode warning */
			    }
			    *(unsigned short*)&iC_QX_[yn] |= qmask; /* note word I/O */
			} else
			if (qmask) {
			    if (iC_QX_[yn] & ~qmask & iomask) {
				ierror("I/O addresses more than 1 of bit, byte or word:",
				    iCtext);	/* mixed mode warning */
			    }
			    iC_QX_[yn] |= qmask;	/* note bit or byte I/O */
			}
		    }
		}
	    } else if (sscanf(iCtext, "_%d%1[A-Z_a-z_]", &yn, y2) == 1) {
		ierror("Number preceded by '_' clashes with internal number representation", iCtext);
		typ = ERR;
	    } else if (sscanf(iCtext, "__%d%1[A-Z_a-z_]", &yn, y2) == 1) {
		ierror("Number preceded by '__' clashes with internal negative number representation", iCtext);
		typ = ERR;
	    } else if (!(iC_debug & 04000) && sscanf(iCtext, "iC_%1[A-Za-z_0-9]", y2) == 1) {
		/* this test suppressed for testing by option -d4000 - can cause link errors */
		ierror("Variable starting with iC_ clashes with iC-runtime global variable", iCtext);
		typ = ERR;
	    }
	    unget(c);
#if YYDEBUG
	    if (rest) {					/* set when other >Ix 402 debug output */
		fprintf(iC_outFP, "=I5:%d	iCtext:%s:	c='%c', len = %d, iCleng = %d '%s'\n", lineno, iCtext, c, len, iCleng, chbuf[0]);
		fflush(iC_outFP);
	    }
#endif
	    symp = 0;
	    if (strcmp(iCtext, "__END__") == 0) {
		return 0;			/* early termination */
	    }
	    if (iFunSymExt && ! qtoken) {	/* in function definition and not I/O */
		if (iRetSymbol.v && strcmp(iCtext, "this") == 0) {
		    symp = iRetSymbol.v;	/* function return Symbol */
		} else {
		    char *	cp;
		    if (iC_Pflag && strchr(iCtext, '$')) {
			if (iC_Pflag >= 2) typ = ERR;
			warning("'$' character(s) in function block parameter or identifier:", iCtext);
		    }
		    if ((cp = strchr(iCtext, '@')) != 0) {
			/********************************************************************
			 *  Variable names with '@' are only legal if part of this
			 *  function definition in which case they match iFunBuffer
			 *  and no second '@' occurs - alternate syntax used previously
			 *  with '$' as the function variable separator
			 *******************************************************************/
			cp++;
			if (strchr(cp, '@') ||
			    strncmp(iCtext, iFunBuffer, iFunSymExt - iFunBuffer) != 0) {
			    ierror("'@' only allowed if it follows current function name:", iCtext);
			    return LEXERR;
			}
		    } else {
			cp = iCtext;		/* use the bare function variable */
		    }
		    strncpy(iFunSymExt, cp, iFunEnd - iFunSymExt);
		    if ((symp = lookup(iFunBuffer)) == 0 && iFunSyText) {
			symp = install(iFunBuffer, typ, ftyp); /* parameter or declaration */
		    }
		    *iFunSymExt = '\0';
		}
	    } else if (strchr(iCtext, '@')) {
		ierror("'@' only allowed if it follows function name in a function definition:", iCtext);
		return LEXERR;
	    } else if (iC_Pflag && strchr(iCtext, '$')) {
		/********************************************************************
		 *  This and the test just above, are the only places, where variables
		 *  containing the '$' symbol may be marked erroneous in iC code.
		 *  The internal C compiler, gcc and iClive can handle variables
		 *  containg a '$'
		 *  Excerpt from 'info gcc':
		 *  =====================================
		 *  5.29 Dollar Signs in Identifier Names
		 *  =====================================
		 *  In GNU C, you may normally use dollar signs in identifier names.  This
		 *  is because many traditional C implementations allow such identifiers.
		 *  However, dollar signs in identifiers are not supported on a few target
		 *  machines, typically because the target assembler does not allow them.
		 *
		 *  The internal C compiler warns about variables in C expressions starting
		 *  with a '$' symbol, which may lead to assembler errors. (see lexc.l)
		 *******************************************************************/
		if (iC_Pflag >= 2) typ = ERR;
		warning("'$' character(s) in identifier:", iCtext);
	    } else if (strpbrk(iCtext, "EHLST") == iCtext) {	/* speeds up search */
		/********************************************************************
		 *  The iC language defines internal input variables TX0.0 - TX0.7
		 *  The first 3 have special functionality and the remaining 5 are timers.
		 *  The following table defines aliases for these internal inputs.
		 *	static char *	TaliasList[9][2] = {
		 *	    { "EOI",	"TX0.0"  },	// end of initialisation
		 *	    { "STDIN",	"TX0.1"  },	// stdin line received
		 *	    { "LO",	"TX0.2"  },	// constant bit LO
		 *	    { "HI",	"~TX0.2" },	// constant bit HI
		 *	    { "T10ms",	"TX0.3"  },	// 10 ms internal timer
		 *	    { "T100ms",	"TX0.4"  },	// 100 ms internal timer
		 *	    { "T1sec",	"TX0.5"  },	// 1 second internal timer
		 *	    { "T10sec",	"TX0.6"  },	// 10 second internal timer
		 *	    { "T1min",	"TX0.7"  },	// 1 minute internal timer
		 *	};
		 *******************************************************************/
		int	i;
		int	invFlag = 0;
		char *	TXp;
		for (i = 0; i < 9; i++) {
		    if (strcmp(iCtext, TaliasList[i][0]) == 0) {
			if ((symp = lookup(iCtext)) == 0) {	/* lookup/install ALIAS */
			    symp = install(iCtext, ALIAS, GATE);
			    if (*(TXp = TaliasList[i][1]) == '~') {
				TXp++;
				invFlag = 1;
			    }
			    if ((sp = lookup(TXp)) == 0) {	/* lookup/install TX0.x */
				sp = install(TXp, INPX, GATE);
			    }
			    symp->list = lp = sy_push(sp);	/* complete ALIAS to TX0.x */
			    if (invFlag) {
				lp->le_val = NOT;		/* HI is inverting ALIAS of TX0.2 */
			    }
			}
			break;
		    }
		}
	    }
	    if (symp == 0) {
		/********************************************************************
		 *  Lookup or install all other names as undefined Gates
		 *******************************************************************/
		if ((symp = lookup(iCtext)) == 0) {
		    symp = install(iCtext, typ, ftyp); /* usually UDF UDFA */
		} else if (typ == ERR) {
		    symp->type = ERR;			/* mark ERROR in QX%d_%d */
		}
	    }
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "yylex: %s %s %s\n",
		    symp->name, iC_full_type[symp->type], iC_full_ftype[symp->ftype]);
		fflush(iC_outFP);
	    }
#endif
	    iClval.sym.v = symp;			/* return actual symbol */
	    /********************************************************************
	     *  resolve all ALIASes
	     *******************************************************************/
	    while ( (typ = symp->type) == ALIAS &&
		    (lp = symp->list) != 0 &&
		    (sp = lp->le_sym) != 0) {
		symp = sp;				/* with token of original */
	    }
	    /********************************************************************
	     *  no need to clear sp->em	- check - this refers to very old code Jw 20120515
	     *  EM was cleared as typ &= ~EM before - was not cleared in symp->type
	     *******************************************************************/
	    if (typ > IFUNCT || (symp->fm & FM) != 0) {	/* function definition symbols */
		c = LEXERR;			/* are not looked up directly */
	    } else
	    if (typ == IFUNCT) {
		switch (symp->ftype) {
		case UDFA:
		    c = VFUNCTION;
		    break;
		case ARITH:
		case GATE:
		case GATEX:
		    c = IFUNCTION;
		    break;
		case CLCKL:
		    c = CFUNCTION;
		    break;
		case TIMRL:
		    c = TFUNCTION;
		    break;
		default:
		    c = LEXERR;
		    break;
		}
	    } else
	    if (typ >= KEYW) {
		c = symp->u_val;		/* reserved word or C-type */
	    } else
	    if (qtoken > 1) {			/* ignore I... */
		c = qtoken;			/* first time LOUT or AOUT */
		symp->em |= EO;			/* marks variable as LOUT AOUT Q... */
	    } else
	    if ((c = lex_typ[typ]) == 0) {	/* 0 AVARC 0 0 LVARC 0 ... NUMBER ... */
		c = lex_act[symp->ftype];	/* UNDEF AVAR LVAR ..YYERRCODE.. AOUT LOUT CVAR TVAR */
	    } else if ((typ == ARNC || typ == LOGC) && symp->ftype == UDFA) {
		c = IMMCARRAY;			/* UDFA ARNC and UDFA LOGC are immC arrays not UNDEF */
	    } else if (typ == NCONST) {		/* c === NUMBER from lex_typ[NCONST] */
		iClval.vai.v = 0;		/* return vai although not needed - clean extended listing */
	    }
	    if (symp->ftype == OUTW && (lp = symp->list) != 0) {
		symp = lp->le_sym;		/* original via backptr */
	    }
	    iClval.sym.f = iCstmtp;		/* original name for expressions */
	    					/* CHECK if iCbuf changes now _() is missing */
	    if ((len = snprintf(iCstmtp, rest = &iCbuf[IMMBUFSIZE] - iCstmtp,
				"%s", symp->name)) < 0 || len >= rest) {
		iCbuf[IMMBUFSIZE-1] = '\0';
		len = rest - 1;			/* text + NUL which fits */
		ierror("statement too long at: ", symp->name);
	    }
	    iClval.sym.l = iCstmtp += len;
	} else {				/* not digit alpha '_' or '$' */
	    c1 = get(T0FP, 0);			/* unget possible EOF after c has been processed */
	    switch (c) {
	    case '!':
		if (c1 == '=') {
		    c = EQU; goto found;	/* != */
		}
	    case '~':
		break;				/* ! or ~ */
	    case '%':
		if (c1 == '{') {		/*    >>> '}' */
		    c = LHEAD; goto found;	/* %{ >>> %} */
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* %= */
		}
		c = DIV;			/* % */
		break;
	    case '&':
		if (c1 == '&') {
		    c = B_AND; goto found;	/* && */
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* &= */
		}
		break;
	    case '*':
		if (c1 == '/') {
		    c = COMMENTEND;		/* dangling end of a comment */
		    goto found;
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* *= */
		}
		break;
	    case '+':
		if (c1 == '+') {
		    c = PPMM; goto found;	/* ++ */
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* += */
		}
		break;				/* + binary or unary */
	    case '-':
		if (c1 == '-') {
		    c = PPMM; goto found;	/* -- */
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* -= */
		} else if (c1 == '>') {
		    c = PR; goto found;		/* -> */
		}
		break;				/* - binary or unary */
	    case '/':
		if (c1 == '/') {
		    do {			/* start C++ style comment */
			if ((c1 = get(T0FP, 0)) == EOF) return 0;
		    } while (c1 != '\n');
		} else if (c1 == '*') {
		    do {			/* start C style comment */
			while (c1 != '*') {
			    if ((c1 = get(T0FP, 0)) == EOF) return 0;
			}
		    } while ((c1 = get(T0FP, 0)) != '/');
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* /= */
		} else {
		    c = DIV;			/* / */
		    break;
		}
		iCleng = 0;			/* end of comment */
		continue;
	    case '<':
		if (c1 == '<') {
		    if ((c1 = get(T0FP, 0)) == '=') {
			c = OPE; goto found;	/* <<= */
		    }
		    c = LEFT_SH;		/* << */
		    break;
		}
		if (c1 != '=') {
		    unget(c1);
		}
		c = CMP; goto found;		/* < or <= */
	    case '=':
		if (c1 == '=') {
		    c = EQU; goto found;	/* == */
		}
		*iCtext = '#';			/* to recognise <= => transmogrify = */
		break;
	    case '>':
		if (c1 == '>') {
		    if ((c1 = get(T0FP, 0)) == '=') {
			c = OPE; goto found;	/* >>= */
		    }
		    c = RIGHT_SH;		/* >> */
		    break;
		}
		if (c1 != '=') {
		    unget(c1);
		}
		c = CMP; goto found;		/* > or >= */
	    case '^':
		if (c1 == '=') {
		    c = OPE; goto found;	/* ^= */
		}
		break;
	    case '|':
		if (c1 == '|') {
		    c = B_OR; goto found;	/* || */
		} else if (c1 == '=') {
		    c = OPE; goto found;	/* |= */
		}
		break;
	    case '.':
		c = PR;				/* . */
		break;
	    case '\x1f':
		c = BE;				/* BE */
		break;
	    }
	    unget(c1);				/* c1 was not used (including EOF) */
	found:
	    iClval.vai.v = c;			/* return to yacc as is */
	retfl:
	    iClval.vai.f = iCstmtp;		/* all sources are iClval.vai */
	    if ((c == '+' || c == '-' || c == PPMM) &&
		iCstmtp > iCbuf && *(iCstmtp-1) == *iCtext) {
		*iCstmtp++ = ' ';		/* space between + + and - - */
						/* 1 byte will always fit */
	    }
	    rest = &iCbuf[IMMBUFSIZE] - iCstmtp;
	    len = strlen(iCtext);
	    if (len >= rest) {
		iCbuf[IMMBUFSIZE-1] = '\0';
		len = rest -1;
		ierror("statement too long at: ", iCtext);
	    }
	    iClval.vai.l = iCstmtp = strncpy(iCstmtp, iCtext, len) + len;
	}
	return c;				/* return token to yacc */
    }
    return 0;					/* EOF */
} /* yylex */

/********************************************************************
 *
 *	Output an error in source listing and error output
 *	The error file errFP is opened with the first error
 *
 *******************************************************************/

static void
errLine(void)					/* error file not openend if no errors */
{
    if (errFlag == 0) {
	if (errFilename && iC_errFP == stderr) {
	    if ((iC_errFP = fopen(errFilename, "w+")) == NULL) {
		errFilename = 0;		/* cannot open errFilename */
		iC_errFP = stderr;		/* just output to stderr */
		errRet = Eindex;		/* error return for compile() */
	    } else {
		errFlag = 1;			/* there is an error file */
	    }
	}
	if (iC_outFP != stdout) {
	    errFlag = 1;			/* errors to errFilename or stderr */
	    if (!(iC_debug & 046)) {		/* no source listing in debugging output */
		fprintf(iC_outFP, "******* %-15s ************************\n", inpNM);
	    }
	}
	if (errFlag) fprintf(iC_errFP, "******* %-15s ************************\n", inpNM);
    }
    if (lineno != errline) {
	errline = lineno;			/* dont print line twice */
	if (!(iC_debug & 040) || (lexflag & C_BLOCK)) {	/* no source listing in debugging output */
	    fprintf(iC_outFP, "%03d\t%s", lineno, chbuf[lexflag&C_PARSE]);
	    if (lineflag == 0) putc('\n', iC_outFP);	/* current line not complete */
	}
	if (errFlag) {
	    fprintf(iC_errFP, "%03d\t%s", lineno, chbuf[lexflag&C_PARSE]);
	    if (lineflag == 0)  putc('\n', iC_errFP);	/* current line not complete */
	}
    }
} /* errLine */

/********************************************************************
 *
 *	Common code for error messages
 *
 *******************************************************************/

void
errmess(					/* actual error message */
    char *	str0,
    char *	str1,
    char *	str2)
{
    errLine();					/* do not change - used as search key in iClive */
    fprintf(iC_outFP, "*** %s: %s", str0, str1);
    if (errFlag) fprintf(iC_errFP, "*** %s: %s", str0, str1);
    if (str2) {
	fprintf(iC_outFP, " %s", str2);
	if (errFlag) fprintf(iC_errFP, " %s", str2);
    } else {
	putc('.', iC_outFP);
	if (errFlag) putc('.', iC_errFP);
    }
#ifdef NEWSTYLE
    fprintf(iC_outFP, " '%s'\n", inpNM);
    if (errFlag) fprintf(iC_errFP, " '%s'\n", inpNM);
#else
    fprintf(iC_outFP, " File %s, line %d\n", inpNM, lineno);
    if (errFlag) fprintf(iC_errFP, " File %s, line %d\n", inpNM, lineno);
#endif
    fflush(iC_outFP);
    if (errFlag) fflush(iC_errFP);
} /* errmess */

/********************************************************************
 *
 *	Error message
 *	count in gnerrs to prevent execution of generated code
 *
 *******************************************************************/

void
ierror(						/* print error message */
    char *	str1,
    char *	str2)
{
    gnerrs++;					/* count mostly generate errors */
    errmess("Error", str1, str2);
    if (++iC_iErrCount >= iC_maxErrCount) {
	errmess("Error", "too many errors - compilation aborted", NS);
	if (T0FP) fseek(T0FP, 0L, SEEK_END);	/* flush rest of file */
	longjmp(beginMain, Iindex);
    }
} /* ierror */

/********************************************************************
 *
 *	Warning message
 *
 *******************************************************************/

void
warning(					/* print warning message */
    char *	str1,
    char *	str2)
{
    if (iC_Pflag <= 1) {
	errmess("Warning", str1, str2);		/* simple warning */
    } else {
	ierror(str1, str2);			/* convert warning to error */
    }
} /* warning */

/********************************************************************
 *
 *	Common error messages
 *
 *******************************************************************/









void
errInt(void)
{
    ierror("no imm variable to trigger arithmetic expression", NS);
} /* errInt */

/********************************************************************
 *
 *	Recovery from run-time errors
 *
 *******************************************************************/

void
execerror(					/* recover from run-time error */
    char *	str1,
    char *	str2,
    char *	file,
    int		line)
{
    errmess("Execerror", str1, str2);
    fprintf(iC_outFP, "in source file: %s line %d\n", file, line);
    fflush(iC_outFP);
    if (errFlag) {
	fprintf(iC_errFP, "in source file: %s line %d\n", file, line);
	fflush(iC_errFP);
    }
    fseek(T0FP, 0L, SEEK_END);			/* flush rest of file */
    longjmp(begin, 0);
} /* execerror */

/********************************************************************
 *
 *	Called from yacc parser on error (mostly syntax error)
 *
 *******************************************************************/

void
yyerror(const char *	s)
{
    char *	cp = chbuf[lexflag&C_PARSE];
    int		n, n1;
    static char * lc[2] = { "", "C " };	/* this used to be done in gram.pl - jw 20161106 */
    char	erbuf[TSIZE];

    errLine();
    fprintf(iC_outFP, "*** ");	/* do not change - used as search key in pplstfix and iClive */
    if (errFlag) fprintf(iC_errFP, "*** ");
    if ((lexflag & C_PARSE) == 0) {
	n = getp[0] - chbuf[0] - iCleng;
    } else {
	n = column - yyleng;
    }
    for (n1 = 0; n > 0; n--, cp++) {
	n1++;
	if (*cp == '\t') {
	    while (n1 % 8) {
		n1++;
	    }
	}
    }
    ynerrs++;			/* count mostly syntax errors */
#ifdef NEWSTYLE
    n = snprintf(erbuf, TSIZE, "%s%s '%s'", lc[lexflag&C_PARSE], s, inpNM);
#else
    n = snprintf(erbuf, TSIZE, "%s%s %d in %s", lc[lexflag&C_PARSE], s, ynerrs, inpNM);
#endif
    if (n1 < n + 5) {
	n1 += 4;
	while (n1--) {
	    putc('-', iC_outFP);
	    if (errFlag) putc('-', iC_errFP);
	}
	fprintf(iC_outFP, "^ %s\n", erbuf);
	if (errFlag) fprintf(iC_errFP, "^ %s\n", erbuf);
    } else {
	fprintf(iC_outFP, "%s ", erbuf);
	if (errFlag) fprintf(iC_errFP, "%s ", erbuf);
	n1 -= n - 3;
	while (n1--) {
	    putc('-', iC_outFP);
	    if (errFlag) putc('-', iC_errFP);
	}
	fprintf(iC_outFP, "^\n");
	if (errFlag) fprintf(iC_errFP, "^\n");
    }
} /* yyerror */

/********************************************************************
 *
 *	Copy a C fragment to T1FP for an lBlock and T4FP for a cBlock
 *	when yacc token CCFRAG is recognised in yylex()
 *
 *******************************************************************/

static int
copyCfrag(char s, char m, char e, FILE * oFP)
	/* copy C action to the next ; , or closing brace or bracket */
{
    int		brace;
    int		c;
    int		c1;
    int		match;

    for (brace = 0; (c = get(T0FP, 0)) != EOF; ) {
	if (c == s) {				/* '{' or '(' */
	    if (brace++ == 0 && m == '%') {	/* don't output "%{\w" */
		while ((c = get(T0FP, 0)) == ' ' || c == '\t');
		unget(c);
		continue;
	    }
	} else if (c == m) {			/* ';' or '%' or ',' */
	    if (brace <= 0) {
		unget(c);			/* use for next token */
		return m;			/* no longer used */
	    } else if (brace == 1 && c == '%') {
		if ((c = get(T0FP, 0)) == '}') {
		    fprintf(oFP, "\n%%##\n\n%%}\n");	/* {#line lineno "outNM"\n%} */
		    unget(c);
		    return m;
		}
		unget(c);
		c = '%';
	    }
	} else if (c == e) {			/* ')' or '}' */
	    if (--brace <= 0) {
		/* TD fix lineno and name */
		if (brace == 0 && c == '}') {
		    putc(c, oFP);		/* output '}' */
		}
		fprintf(oFP, "\n%%##\n");	/* #line lineno "outNM" */
		unget(c);			/* should not return without '}' */
		return e;
	    }
	} else {
	    switch (c) {
	    case '/':				/* look for comments */
		if ((c1 = get(T0FP, 0)) == '/') {
		    putc(c, oFP);
		    do {			/* start C++ style comment */
			putc(c1, oFP);
			if ((c1 = get(T0FP, 0)) == EOF) goto eof_error;
		    } while (c1 != '\n');
		    c = c1;
		} else if (c1 == '*') {
		    putc(c, oFP);
		    do {			/* start C style comment */
			putc(c1, oFP);
			while (c1 != '*') {
			    if ((c1 = get(T0FP, 0)) == EOF) goto eof_error;
			    putc(c1, oFP);
			}
		    } while ((c1 = get(T0FP, 0)) != '/');
		    c = c1;
		} else {
		    unget(c1);
		}
		break;

	    case '\'':				/* character constant */
		match = '\'';
		goto string;

	    case '"':				/* character string */
		match = '"';

	    string:
		putc(c, oFP);
		while ((c = get(T0FP, 0)) != match) {
		    if (c == '\\') {
			putc(c, oFP);
			if ((c = get(T0FP, 0)) == EOF) goto eof_error;
		    } else if (c == '\n') {
			iCleng = 1;		/* error pointer at newline */
			yyerror("C code: newline in \" \" or ' ', error");
		    } else if (c == EOF)  goto eof_error;
		    putc(c, oFP);
		}
		break;
	    }
	}
	putc(c, oFP);				/* output to T1FN or T4FN */
    }
eof_error:
    iCleng = 1;					/* error pointer at EOF */
    yyerror("C code: EOF, error");
    return 0;					/* EOF */
} /* copyCfrag */

/********************************************************************
 *
 *	Compile if, if else or switch
 *
 *******************************************************************/

static void
ffexprCompile(char * txt, List_e * lp, int cn)
{
    int		saveLexflag;
    int		saveLineno;
    Symbol *	sp;

    saveLexflag = lexflag;			/* iC compile state */
    saveLineno  = lineno;
    assert(lp);
    sp = lp->le_sym;
    assert(sp);
    lp = sp->u_blist;				/* ffexpr head link */
    assert(lp);
    writeCexeTail(T4FP, "    return 0;\n", cn);
    if (iC_c_compile(T4FP, T5FP, C_PARSE|C_BLOCK1, lp) || iC_copyXlate(T5FP, T1FP, 0, 0, 02)) {
	ierror("c_compile failed:", txt);
    }
    lexflag = saveLexflag;			/* restore iC compile state */
    lineno  = saveLineno;
} /* ffexprCompile */

/********************************************************************
 *
 *	If iC_Zflag is set, restore iC_outFP from iC_lstFP, which
 *	enables listing output.
 *	else open iC_nulFP to /dev/null unless it was already opened,
 *	copy it to iC_outFP, which blocks listing output.
 *
 *******************************************************************/

static void
blockUnblockListing(void)
{
    if (iC_Zflag) {
	iC_outFP = iC_lstFP;	/* listing output is now unblocked */
    } else {
	if (iC_nulFP == NULL && (iC_nulFP = fopen("/dev/null", "w")) == NULL) {
	    perror("fopen /dev/null");		/* unlikely to fail */
	    ierror("blocking listing failed:", NS);
	}
	iC_outFP = iC_nulFP;	/* listing output is now blocked */
    }
} /* blockUnblockListing */

/********************************************************************
 *
 *	Adjust logic expression with arithmetic or constant expression
 *
 *******************************************************************/

static List_e *
op_adjust(Lis * lv)
{
    List_e *	lp;
    Symbol *	sp;
    Valp	v;

    if ((lp = lv->v) == NULL) {			/* constant expression ? */
	if ((sp = lookup("TX0.2")) == 0) {	/* yes */
	    sp = install("TX0.2", INPX, GATE);
	}
	lp = sy_push(sp);			/* TX0.2 is permanent bit LO */
	v = evalConstExpr(lv);
	if (v.nuv) {
	    lp->le_val = NOT;			/* ~TX0.2 is permanent bit HI */
	}
    } else {
	lp = op_force(lp, GATE);		/* no - adjust possible arithmetic expression */
    }
    return lp;
} /* op_adjust */

/* A Bison parser, made by GNU Bison 2.7.  */

/* Bison interface for Yacc-like parsers in C
   
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
/* Line 2058 of yacc.c  */
#line 68 "comp.y"
		/* stack type */
    Sym		sym;			/* symbol table pointer */
    Lis		list;			/* linked list elements */
    Val		vai;			/* int numerical values */
    Vap		vap;			/* union { int, Symbol* } */
    Typ		typ;			/* type, ftype, em and fm */
    /* allows storing character as iClval.vai.v (2nd byte is NULL) */
    /* then char array can be referenced as $1.v, single char as $1.v[0] */


/* Line 2058 of yacc.c  */
#line 123 "comp.tab.h"
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

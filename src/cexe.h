/********************************************************************
 *
 *	Copyright (C) 1985-2001  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <john@je-wulff.de>
 *
 *******************************************************************/

static const char cexe_part1[] = "\
/********************************************************************\n\
 *\n\
 *	Extension for icr and ict compilers with integrated run time\n\
 *	system to execute literal blocks and arithmetic expressions\n\
 *\n\
 *******************************************************************/\n\
\n\
static const char cexe_h[] =\n\
\"@(#)$Id: cexe.h,v 1.19 2004/01/02 14:32:55 jw Exp $\";\n\
\n\
#include	<stdio.h>\n\
#include	\"icg.h\"\n\
#include	\"icc.h\"\n\
#include	\"comp.h\"\n\
\n\
#define _AV(x) Lookup(#x)->u.gate->gt_new\n\
#define _LV(x) (Lookup(#x)->u.gate->gt_val < 0 ? 1 : 0)\n\
#define _AA(x,v) aAssign(Lookup(#x)->u.gate, v)\n\
#define _LA(x,v) lAssign(Lookup(#x)->u.gate, v)\n\
\n\
#if INT_MAX == 32767 && defined (LONG16)\n\
static long	_tVar;\n\
#else\n\
static int	_tVar;\n\
#endif\n\
\n\
static Symbol *\n\
Lookup(char *	string)	/* find string in symbol table at run time */\n\
{\n\
    Symbol *	sp;\n\
\n\
    if ((sp = lookup(string)) == 0) {\n\
#ifndef _WINDOWS \n\
	fflush(outFP);\n\
	fprintf(errFP,\n\
	    \"\\n*** Error: cexe.c: Lookup could not find Symbol '%%s' at run time.\\n\"\n\
	      \"*** Usually this is a term in a C function which does not match.\\n\"\n\
	      \"*** Check that 'cexe.c' was built from '%%s'\\n\"\n\
	      \"*** Rebuild compiler using '%%s -c %%s'\\n\"\n\
	      , string, inpNM, progname, inpNM);\n\
#endif\n\
	quit(-3);\n\
    }\n\
    return sp;				/* found */\n\
} /* Lookup */\n\
\n\
/********************************************************************\n\
 *\n\
 *	Literal blocks and embedded C fragment cases\n\
 *\n\
 *******************************************************************/\n\
\n\
";
static const int cexe_lines1 = 52;

static const char cexe_part2[] = "\
#if INT_MAX == 32767 && defined (LONG16)\n\
long\n\
#else\n\
int\n\
#endif\n\
c_exec(int pp_index, Gate * _cexe_gf)\n\
{\n\
    switch (pp_index) {\n\
";
static const int cexe_lines2 = 8;

static const char cexe_part3[] = "\
    default:\n\
#ifndef _WINDOWS \n\
	fflush(outFP);\n\
	fprintf(errFP,\n\
	    \"\\n*** Error: cexe.c: C function 'F(%%d)' is unknown.\\n\"\n\
	      \"*** Check that 'cexe.c' was built from '%%s'\\n\"\n\
	      \"*** Rebuild compiler using '%%s -c %%s'\\n\"\n\
	      , pp_index, inpNM, progname, inpNM);\n\
	quit(-1);\n\
#endif\n\
	break;\n\
    }\n\
#ifndef _WINDOWS \n\
    fflush(outFP);\n\
    fprintf(errFP,\n\
	\"\\n%%s: line %%d: Function fragment without return ???\\n\",\n\
	__FILE__, __LINE__);\n\
    quit(-2);\n\
#else\n\
    return 0;	/* for those cases where no return has been programmed */\n\
#endif\n\
} /* c_exec */\n\
";
static const int cexe_lines3 = 22;

static const char cexe_part4[] = "\
\n\
/********************************************************************\n\
 *		SOURCE:   %s\n\
 *******************************************************************/\n\
static char	COMPILER[] =\n\
\"%s\";\n\
";
static const int cexe_lines4 = 6;

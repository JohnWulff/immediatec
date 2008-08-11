/********************************************************************
 *
 *	Copyright (C) 1985-2008  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <ic@je-wulff.de>
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
\"@(#)$Id: cexe.h,v 1.26 2008/06/25 21:37:30 jw Exp $\";\n\
\n\
#include	<stdio.h>\n\
#include	\"icc.h\"\n\
#include	\"comp.h\"\n\
\n\
#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new\n\
#define iC_AV(n)	iC_gf->gt_list[n]->gt_new\n\
#define iC_LV(n)	(iC_gf->gt_list[n]->gt_val < 0 ? 1 : 0)\n\
#define iC_AA(n,v)	iC_assignA(iC_gf->gt_list[n], v)\n\
#define iC_LA(n,v)	iC_assignL(iC_gf->gt_list[n], v)\n\
#define iC_AVL(n)	iC_pf0_1->gt_list[n]->gt_new\n\
#define iC_LVL(n)	(iC_pf0_1->gt_list[n]->gt_val < 0 ? 1 : 0)\n\
#define iC_AAL(n,v)	iC_assignA(iC_pf0_1->gt_list[n], v)\n\
#define iC_LAL(n,v)	iC_assignL(iC_pf0_1->gt_list[n], v)\n\
\n\
#if INT_MAX == 32767 && defined (LONG16)\n\
static long	iC_tVar;\n\
#else\n\
static int	iC_tVar;\n\
#endif\n\
\n\
/********************************************************************\n\
 *\n\
 *	Literal blocks and embedded C fragment cases\n\
 *\n\
 *******************************************************************/\n\
\n\
";
static const int cexe_lines1 = 36;

static const char cexe_part2[] = "\
#if INT_MAX == 32767 && defined (LONG16)\n\
long\n\
#else\n\
int\n\
#endif\n\
iC_exec(int iC_index, iC_Gt * iC_gf)\n\
{\n\
    switch (iC_index) {\n\
";
static const int cexe_lines2 = 8;

static const char cexe_part3[] = "\
    default:\n\
#ifndef _WINDOWS \n\
	fflush(iC_outFP);\n\
	fprintf(iC_errFP,\n\
	    \"\\n*** Error: cexe.c: C function 'F(%%d)' is unknown.\\n\"\n\
	      \"*** Rebuild compiler using '%%s -c -%%sO%%o %%s'\\n\"\n\
	      \"*** was built with         '%s -c -%sO%o %s'\\n\"\n\
	      , iC_index, iC_progname, iC_gflag ? \"g\" : \"\", iC_optimise, inpNM);\n\
	iC_quit(-1);\n\
#endif\n\
	break;\n\
    }\n\
#ifndef _WINDOWS \n\
    fflush(iC_outFP);\n\
    fprintf(iC_errFP,\n\
	\"\\n%%s: line %%d: Function fragment without return ???\\n\",\n\
	__FILE__, __LINE__);\n\
    iC_quit(-2);\n\
#endif\n\
    iC_tVar = 0;			/* shut up -Wall */\n\
    return 0;	/* for those cases where no return has been programmed */\n\
} /* iC_exec */\n\
";
static const int cexe_lines3 = 22;

static const char cexe_part4[] = "\
\n\
/********************************************************************\n\
 *		SOURCE:   %s\n\
 *******************************************************************/\n\
static char	iC_compiler[] =\n\
\"%s -%sO%o\";\n\
";
static const int cexe_lines4 = 6;

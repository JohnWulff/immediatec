/********************************************************************
 *
 *	Copyright (C) 1985-2005  John E. Wulff
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
\"@(#)$Id: cexe.h,v 1.23 2005/01/26 16:47:45 jw Exp $\";\n\
\n\
#include	<stdio.h>\n\
#include	\"icc.h\"\n\
#include	\"comp.h\"\n\
\n\
#define iC_MV(n) iC_gf->gt_rlist[n]->gt_new\n\
#define iC_AV(n) iC_gf->gt_list[n]->gt_new\n\
#define iC_LV(n) (iC_gf->gt_list[n]->gt_val < 0 ? 1 : 0)\n\
#define iC_AA(n,v) iC_assign(iC_gf->gt_list[n], v)\n\
#define iC_LA(n,v) iC_assign(iC_gf->gt_list[n], v)\n\
#define iC_AVL(x) iC_Lookup(#x)->u_gate->gt_new\n\
#define iC_LVL(x) (iC_Lookup(#x)->u_gate->gt_val < 0 ? 1 : 0)\n\
#define iC_AAL(x,v) iC_assign(iC_Lookup(#x)->u_gate, v)\n\
#define iC_LAL(x,v) iC_assign(iC_Lookup(#x)->u_gate, v)\n\
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
static const int cexe_lines1 = 37;

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
	      \"*** Check that 'cexe.c' was built from '%%s'\\n\"\n\
	      \"*** Rebuild compiler using '%%s -c %%s'\\n\"\n\
	      , iC_index, inpNM, iC_progname, inpNM);\n\
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
    iC_tVar = (int)iC_Lookup(\"iClock\");			/* shut up -Wall */\n\
    return 0;	/* for those cases where no return has been programmed */\n\
} /* iC_exec */\n\
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

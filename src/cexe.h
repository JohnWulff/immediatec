/********************************************************************
 *
 *	Copyright (C) 1985-2009  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *******************************************************************/

static const char cexe_part1[] =
"/********************************************************************\n"
" *\n"
" *	Extension for icr and ict compilers with integrated run time\n"
" *	system to execute literal blocks and arithmetic expressions\n"
" *\n"
" *******************************************************************/\n"
"\n"
"static const char cexe_h[] =\n"
"\"@(#)$Id: cexe.h 1.32 $\";\n"
"\n"
"#include	<stdio.h>\n"
"#include	<signal.h>\n"
"#include	<string.h>\n"
"#include	\"icc.h\"\n"
"#include	\"comp.h\"\n"
"\n"
"#define " MV "(n)	iC_gf->gt_rlist[n]->gt_new\n"
"#define " AV "(n)	iC_gf->gt_list[n]->gt_new\n"
"#define " LV "(n,c)	((iC_gf->gt_list[n]->gt_val < 0) ^ c ? 1 : 0)\n"
"#define " AA "(n,p,v)	iC_assignA(iC_gf->gt_list[n], p, v)\n"
"#define " LA "(n,c,p,v)	iC_assignL(iC_gf->gt_list[n], c, p, v)\n"
"#define " AVI "(n,i)	iC_index(iC_gf->gt_list[n], i)->gt_new\n"
"#define " LVI "(n,c,i)	((iC_index(iC_gf->gt_list[n], i)->gt_val < 0) ^ c ? 1 : 0)\n"
"#define " AAI "(n,i,p,v)	iC_assignA(iC_index(iC_gf->gt_list[n], i), p, v)\n"
"#define " LAI "(n,c,i,p,v)	iC_assignL(iC_index(iC_gf->gt_list[n], i), c, p, v)\n"
"#define " SIZ "(n)	iC_gf->gt_list[n]->gt_old\n"
"\n"
"#define " AVL "(n)	iC_pf0_1->gt_list[n]->gt_new\n"
"#define " LVL "(n,c)	((iC_pf0_1->gt_list[n]->gt_val < 0) ^ c ? 1 : 0)\n"
"#define " AAL "(n,p,v)	iC_assignA(iC_pf0_1->gt_list[n], p, v)\n"
"#define " LAL "(n,c,p,v)	iC_assignL(iC_pf0_1->gt_list[n], c, p, v)\n"
"#define " AVIL "(n,i)	iC_index(iC_pf0_1->gt_list[n], i)->gt_new\n"
"#define " LVIL "(n,c,i)	((iC_index(iC_pf0_1->gt_list[n], i)->gt_val < 0) ^ c ? 1 : 0)\n"
"#define " AAIL "(n,i,p,v)	iC_assignA(iC_index(iC_pf0_1->gt_list[n], i), p, v)\n"
"#define " LAIL "(n,c,i,p,v)	iC_assignL(iC_index(iC_pf0_1->gt_list[n], i), c, p, v)\n"
"#define " SIZL "(n)	iC_pf0_1->gt_list[n]->gt_old\n"
"\n"
"/********************************************************************\n"
" *\n"
" *	Literal blocks and embedded C fragment cases\n"
" *\n"
" *******************************************************************/\n"
"\n"
;
static const int cexe_lines1 = 43;

static const char cexe_part2[] =
"#if INT_MAX == 32767 && defined (LONG16)\n"
"long\n"
"#else\n"
"int\n"
"#endif\n"
"iC_exec(int iC_indx, iC_Gt * iC_gf)\n"
"{\n"
"    static int	flag = 1;\n"
"\n"
"    if (flag-- > 0 && strcmp(inpNM, \"%s\") != 0) {\n"
"	fflush(iC_outFP);	/* do strcmp() only once */\n"
"	fprintf(iC_errFP,\n"
"	    \"\\n*** Error: cexe.c: was built with '%s -c -%sO%o %s'\\n\"\n"
"	      \"*** Rebuild compilers using '%%s -c -%%sO%%o %%s; m -rt'\\n\"\n"
"	      , iC_progname, iC_gflag ? \"g\" : \"\", iC_optimise, inpNM);\n"
"	iC_quit(SIGUSR1);\n"
"    }\n"
"\n"
"    switch (iC_indx) {\n"
;
static const int cexe_lines2 = 20;

static const char cexe_part3[] =
"    default:\n"
"	fflush(iC_outFP);\n"
"	fprintf(iC_errFP,\n"
"	    \"\\n*** Error: cexe.c: C function 'F(%%d)' is unknown ???\\n\"\n"
"	      , iC_indx);\n"
"	iC_quit(SIGUSR1);\n"
"	break;\n"
"    }\n"
"    fflush(iC_outFP);\n"
"    fprintf(iC_errFP,\n"
"	\"\\n%%s: line %%d: Function fragment without return ???\\n\",\n"
"	__FILE__, __LINE__);\n"
"    iC_quit(SIGUSR1);\n"
"    return 0;	/* for those cases where no return has been programmed */\n"
"} /* iC_exec */\n"
;
static const int cexe_lines3 = 15;

static const char cexe_part4[] =
"\n"
"/********************************************************************\n"
" *		SOURCE:   %s\n"
" *******************************************************************/\n"
"static char	iC_compiler[] =\n"
"\"%s -%sO%o\";\n"
;
static const int cexe_lines4 = 6;

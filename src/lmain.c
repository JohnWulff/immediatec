static const char lmain_c[] =
"@(#)$Id: lmain.c,v 1.13 2005/03/06 12:25:12 jw Exp $";
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
 *	lmain.c
 *	Standalone check of gram.y and lexc.l C compiler
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include 	"icc.h"
#include 	"comp.h"

static const char *	usage =
"USAGE: %s [-leh] [-d<debug>] <C_program>\n"
"        -l <listFN>     name of list file  (default is stdout)\n"
"        -d <debug>  40  source listing\n"
#if YYDEBUG
"                   +20  symbol table after parse\n"
"                    +4  debugging info\n"
"                    +2  generation of insert markers\n"
"                    +1  yacc debug info\n"
#else
"                        output space separated list of typenames\n"
#endif
"        <C_program>    any C language program file (extension .c)\n"
"        -h              this help text\n"
"Copyright (C) 1985-2005 John E. Wulff     <john@je-wulff.de>\n"
"%s\n";

FILE *	yyin;
FILE *	yyout;
FILE *	iC_outFP;
char *		iC_progname;		/* name of this executable */
const char *	inpFN = 0;		/* name of input file */
const char *	listFN = 0;		/* name of list file */
short		iC_debug = 0400;
#if YYDEBUG
extern	int	c_debug;
#endif

int
main(
    int		argc,
    char **	argv)
{
    int		r = 0;		/* return value of compile */

    /* Process the arguments */
    if ((iC_progname = strrchr(*argv, '/')) == NULL) {
	iC_progname = *argv;
    } else {
	iC_progname++;		/*  path has been stripped */
    }
    iC_outFP = stdout;		/* listing file pointer */
    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
		    int	debi;

		case 'd':
		    if (! *++*argv) { --argc, ++argv; }
		    sscanf(*argv, "%o", &debi);
		    iC_debug |= debi;	/* short */
#if YYDEBUG
		    if (iC_debug & 0400) c_debug = iC_debug & 01;
#endif
		    goto break2;
		case 'l':
		    if (! *++*argv) { --argc, ++argv; }
		    listFN = *argv;	/* listing file name */
		    goto break2;
		default:
		    fprintf(stderr,
			"%s: unknown flag '%c'\n", iC_progname, **argv);
		case 'h':
		case '?':
		    fprintf(stderr, usage, iC_progname, lmain_c);
		    exit(1);
		}
	    } while (*++*argv);
	    break2: ;
	} else {
	    inpFN = *argv;
	}
    }
    iC_debug &= 07777;			/* allow only cases specified */

    if (listFN && (iC_outFP = fopen(listFN, "w+")) == NULL) {
	fprintf(stderr, "%s: cannot open list file '%s'\n", iC_progname, listFN);
	return -3;
    }
    yyout = iC_outFP;

    if (inpFN) {
	if ((yyin = fopen(inpFN, "r")) == NULL) {
	    fprintf(stderr, "%s: cannot open input file '%s'\n", iC_progname, inpFN);
	    return -2;
	}
    } else {
	fprintf(stderr, "%s: no input file\n", iC_progname);
	return -1;
    }

    r = c_parse();
    if (r == 0) {
	Symbol *	sp;
	Symbol **	hsp;

#if YYDEBUG
	if (iC_debug & 020) {
	    fprintf(iC_outFP, "\nSYMBOL TABLE\n\n");
	    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
		for (sp = *hsp; sp; sp = sp->next) {
		    fprintf(iC_outFP, "%-15s %d %d\n", sp->name, sp->type, sp->ftype);
		}
	    }
	}
	if ((iC_debug & ~0400) == 0) {
#endif
	/* output list of TYPE_NAMEs as a space separated list terminated by a new line */
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		fprintf(iC_outFP, " %s", sp->name);
	    }
	}
	fprintf(iC_outFP, "\n");
#if YYDEBUG
	}
#endif
    }
    return r;
} /* main */

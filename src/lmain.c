static const char lmain_c[] =
"@(#)$Id: lmain.c,v 1.9 2004/01/26 19:44:47 jw Exp $";
/********************************************************************
 *
 *	Standalone check of gram.y and lexc.l C compiler
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"icg.h"
#include 	"icc.h"
#include 	"comp.h"

static const char *	usage =
"USAGE: %s [-leh] [-d<debug>] <C_program>\n"
"        -l <listFN>     name of list file  (default is stdout)\n"
"        -d <debug>  10  source listing\n"
#if YYDEBUG
"                   +20  symbol table after parse\n"
"                    +4  debugging info\n"
"                    +2  generation of insert markers\n"
"                    +1  yacc debug info\n"
#else
"                        output space seperated list of typenames\n"
#endif
"        <C_program>    any C language program file (extension .c)\n"
"        -h              this help text\n"
"Copyright (C) 1985-2002 John E. Wulff     <john@je-wulff.de>\n"
"%s\n";

FILE *	yyin;
FILE *	yyout;
FILE *	outFP;
const char *	progname;		/* name of this executable */
const char *	inpFN = 0;		/* name of input file */
const char *	listFN = 0;		/* name of list file */
short		debug = 0400;
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
    if ((progname = strrchr(*argv, '/')) == NULL) {
	progname = *argv;
    } else {
	progname++;		/*  path has been stripped */
    }
    outFP = stdout;		/* listing file pointer */
    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
		    int	debi;

		case 'd':
		    if (! *++*argv) { --argc, ++argv; }
		    sscanf(*argv, "%o", &debi);
		    debug |= debi;	/* short */
#if YYDEBUG
		    if (debug & 0400) c_debug = debug & 01;
#endif
		    goto break2;
		case 'l':
		    if (! *++*argv) { --argc, ++argv; }
		    listFN = *argv;	/* listing file name */
		    goto break2;
		default:
		    fprintf(stderr,
			"%s: unknown flag '%c'\n", progname, **argv);
		case 'h':
		case '?':
		    fprintf(stderr, usage, progname, lmain_c);
		    exit(1);
		}
	    } while (*++*argv);
	    break2: ;
	} else {
	    inpFN = *argv;
	}
    }
    debug &= 07777;			/* allow only cases specified */

    if (listFN && (outFP = fopen(listFN, "w+")) == NULL) {
	fprintf(stderr, "%s: cannot open list file '%s'\n", progname, listFN);
	return -3;
    }
    yyout = outFP;

    if (inpFN) {
	if ((yyin = fopen(inpFN, "r")) == NULL) {
	    fprintf(stderr, "%s: cannot open input file '%s'\n", progname, inpFN);
	    return -2;
	}
    } else {
	fprintf(stderr, "%s: no input file\n", progname);
	return -1;
    }

    r = c_parse();
    if (r == 0) {
	Symbol *	sp;
	Symbol **	hsp;

#if YYDEBUG
	if (debug & 020) {
	    fprintf(outFP, "\nSYMBOL TABLE\n\n");
	    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
		for (sp = *hsp; sp; sp = sp->next) {
		    fprintf(outFP, "%-15s %d %d\n", sp->name, sp->type, sp->ftype);
		}
	    }
	}
	if ((debug & ~0400) == 0) {
#endif
	/* output list of TYPE_NAMEs as a space seperated list terminated by a new line */
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		fprintf(outFP, " %s", sp->name);
	    }
	}
	fprintf(outFP, "\n");
#if YYDEBUG
	}
#endif
    }
    return r;
} /* main */

static const char lmain_c[] =
"@(#)$Id: lmain.c,v 1.3 2002/08/01 13:47:03 jw Exp $";
/********************************************************************
 *
 *	simple driver for MKS yacc
 *
 *******************************************************************/

#include	<stdio.h>
#include	<string.h>
#include 	"icc.h"
#include 	"comp.h"

static const char *	usage =
"USAGE: %s [-leh] [-d<debug>] <C_program>\n"
"        -l <listFN>     name of list file  (default is stdout)\n"
"        -d <debug>  10  source listing\n"
#ifdef YYDEBUG
"                   +20  symbol table after parse\n"
"                    +4  debugging info\n"
"                    +2  generation of insert markers\n"
"                    +1  yacc debug info\n"
#endif
"        <C_program>    any C language program file (extension .c)\n"
"        -h              this help text\n"
"Copyright (C) 1985-2002 John E. Wulff     <john@je-wulff.de>\n"
"%s\n";

extern void copyAdjust(FILE* fp);

FILE *	yyin;
FILE *	yyout;
FILE *	outFP;
const char *	progname;		/* name of this executable */
const char *	inpFN = 0;		/* name of input file */
const char *	listFN = 0;		/* name of list file */
short		debug = 0400;
#ifdef YYDEBUG
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
#ifdef YYDEBUG
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
		error:
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

    copyAdjust(NULL);			/* initialize lineEntryArray */
//    for (; c_parse();) {
//	;
//    }
    r = c_parse();
    if (r == 0) {
#ifdef YYDEBUG
	if (debug & 020) {
	    Symbol *	sp;
	    Symbol **	hsp;

	    fprintf(outFP, "\nSYMBOL TABLE\n\n");
	    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
		for (sp = *hsp; sp; sp = sp->next) {
		    fprintf(outFP, "%-15s %d %d\n", sp->name, sp->type, sp->ftype);
		}
	    }
	}
#endif
	rewind(yyin);
	copyAdjust(yyin);
    }
    return r;
} /* main */

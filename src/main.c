static const char main_c[] =
"@(#)$Id: main.c,v 1.2 1996/07/30 16:41:47 john Exp $";
/*
 *	"main.c"
 *	compiler for pplc
 *
 *	"main.c	1.10	95/02/09"
 */

#include	<stdio.h>
#include	<stdlib.h>
#include	"pplc.h"
#include	"comp.h"

char *		szFile_g;		/* file name to process */
#define progFN	szNames[0]		/* for error messages */
#define inpFN	szNames[1]		/* list file name */
#define errFN	szNames[2]		/* error file name */
#define listFN	szNames[3]		/* list file name */
#define outFN	szNames[4]		/* compiler output file name */
#define exiFN	szNames[5]		/* cexe input file name */
#define exoFN	szNames[6]		/* cexe output file name */
char *		szNames[7] = {		/* matches return in compile */
    	0, 0, 0, 0, 0, 0, "cexe.tmp",
};
short		debug = 0;
ushort		osc_max = MARKMAX;
#ifdef YYDEBUG
extern	int	yydebug;
#endif
static char *	usage = "USAGE:\n\
pplc [-d<debug>] [-n<count>] [-o<out>] [-l<list>] [-e<err>] [-xa] <Lprogram>\n\
	-d <debug>2000	display scan_cnt and link_cnt\n\
		 +1000	I0 toggled every second\n\
		  +400	exit after initialisation\n\
		  +200	display loop info (+old style logic)\n\
		  +100	initialisation and run time info\n\
		   +40	net statistics\n\
		   +20	net topology\n\
		   +10	source listing\n\
		    +4	logic expansion\n"
#ifdef YYDEBUG
"		    +2	logic generation\n\
		    +1	yacc debug info\n"
#endif
"	-n <count>	maxinum loop count (default is %d, limit 15)\n\
	-o <outFN>	name of compiler output file\n\
	-l <listFN>	name of list file  (default is stdout)\n\
	-e <errFN>	name of error file (default is stderr)\n\
	-x		generate auxiliary file cexe.c to extend compiler\n\
	-a		output ARITHMETIC ALIAS nodes for symbol debugging\n\
	<Lprogram>	any L language program file (extension .p)\n\
			- or default is stdin\n\
";

char * OutputMessage[] = {
    0,					// [0] no error
    "%s: syntax or generate errors",	// [1]
    "%s: block count error",		// [2]
    "%s: link count error",		// [3]
    "%s: cannot open file %s",		// [4]
};


/********************************************************************
 *
 *	main program
 *
 *******************************************************************/

int
main(
    int		argc,
    char * *	argv)
{
    int		r;		/* return value of compile */
    progFN = *argv;		/* Process the arguments */
    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
		    int	debi;

		case '\0':
		    szFile_g = 0;	/* - is standard input */
		case 'd':
		    if (! *++*argv) { --argc, ++argv; }
		    sscanf(*argv, "%o", &debi);
		    debug = debi;	/* short */
#ifdef YYDEBUG
		    yydebug = debug & 01;
#endif
		    goto break2;
		case 'n':
		    if (! *++*argv) { --argc, ++argv; }
		    osc_max = atoi(*argv);
		    if (osc_max > 15) goto error;
		    goto break2;
		case 'o':
		    if (! *++*argv) { --argc, ++argv; }
		    outFN = *argv;	/* compiler output file name */
		    exiFN = 0;
		    exoFN = "cexe.tmp";
		    goto break2;
		case 'l':
		    if (! *++*argv) { --argc, ++argv; }
		    listFN = *argv;	/* listing file name */
		    goto break2;
		case 'e':
		    if (! *++*argv) { --argc, ++argv; }
		    errFN = *argv;	/* error file name */
		    goto break2;
		case 'x':
		    if (outFN == 0) {
			exiFN = "cexe.h";
			exoFN = "cexe.c";
		    }
		case 'a':
		    aaflag = 1;		/* generate ARITH ALIAS in outFN */
		    break;
		default:
		    fprintf(stderr,
			"%s: unknown flag '%c'\n", progFN, **argv);
		case '?':
		error:
		    fprintf(stderr, usage, MARKMAX);
		    exit(1);
		}
	    } while (*++*argv);
	    break2: ;
	} else {
	    inpFN = szFile_g = *argv;
	}
    }
    if ((r = compile(listFN, errFN, outFN, exiFN, exoFN)) != 0) {
	fprintf(stderr, OutputMessage[4], progFN, szNames[r]);
    } else if ((r = output(outFN)) != 0) {
	fprintf(stderr, OutputMessage[r < 4 ? r : 4], progFN, szNames[r]);
	r += 10;
    }
    fclose(exoFP);
    return (r);	/* 1 - 6 compile errors, 11 - 16 output errors */
} /* main */

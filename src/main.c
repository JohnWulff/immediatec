static const char main_c[] =
"@(#)$Id: main.c,v 1.28 2002/08/08 23:25:33 jw Exp $";
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
 *	main.c
 *	command line interpretation and starter for icc compiler
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<signal.h>
#include	<assert.h>
#include	"icc.h"
#include	"comp.h"
#ifdef TCP
#include	"tcpc.h"
#endif

extern const char	SC_ID[];

static const char *	usage =
"USAGE for compile mode:\n"
"  %s [-aAc] [-o<out>] [-l<list>] [-e<err>] [-d<debug>] [-P<path>] <iC_program>\n"
"      Options in compile mode only:\n"
"        -o <outFN>      name of compiler output file - sets compile mode\n"
"        -a              append linking info for 2nd and later files\n"
"        -A              compile output ARITHMETIC ALIAS nodes for symbol debugging\n"
"        -c              generate auxiliary file cexe.c to extend %s compiler\n"
"                        (cannot be used if also compiling with -o)\n"
"USAGE for run mode: (direct interpretation)\n"
"  %s [-txh]"
#ifdef TCP
" [-m[m]] [-s <server>] [-p <port>] [-u <unitID>]\n      "
#endif
" [-l<list>] [-e<err>] [-d<debug>] [-P<path>] [-n<count>] <iC_program>\n"
"      Options in both modes:\n"
#ifdef TCP
"        -s host ID of server      (default '%s')\n"
"        -p service port of server (default '%s')\n"
"        -u unit ID of this client (default '%s')\n"
#endif
"        -l <listFN>     name of list file  (default is stdout)\n"
"        -e <errFN>      name of error file (default is stderr)\n"
"        -d <debug>4000  supress listing alias post processor\n"
"                 +2000  display scan_cnt and link_cnt\n"
"                 +1000  I0 toggled every second\n"
"                  +400  exit after initialisation\n"
"                  +200  display loop info (+old style logic)\n"
"                  +100  initialisation and run time info\n"
"                   +40  net statistics\n"
"                   +20  net topology\n"
"                   +10  source listing\n"
"                    +4  logic expansion\n"
#ifdef YYDEBUG
"                    +2  logic generation (requires +400)\n"
"                    +1  yacc debug info  (requires +400)\n"
#endif
#ifdef TCP
"                    +2  trace I/O receive buffer\n"
"                    +1  trace I/O send buffer\n"
#endif
"        -P <path>       Path of script pplstfix when not on PATH (usually ./)\n"
"        <iC_program>    any iC language program file (extension .ic)\n"
"        -               or default: take iC source from stdin\n"
"      Options in run (interpreter) mode only:\n"
"        -t              trace debug (equivalent to -d 100)\n"
"                        can be toggled at run time by typing t\n"
#ifdef TCP
"        -m              microsecond timing info\n"
"        -mm             more microsecond timing (internal time base)\n"
"                        can be toggled at run time by typing m\n"
#endif
"        -x              arithmetic info in hexadecimal (default decimal)\n"
"                        can be changed at run time by typing x or d\n"
"        -n <count>      maximum oscillator count (default is %d, limit 15)\n"
"        -h              this help text\n"
"      An <iC_program> containing only logical expressions can be interpreted\n"
"      with  %s -t <iC_program>. An <iC_program> containing arithmetic\n"
"      expressions requires relinking of %s with a new cexe.c generated\n"
"      by %s -c <iC_program> before <iC_program> can be interpreted.\n"
#ifndef TCP
"      Typing 0 to 7 toggles simulated inputs IX0.0 to IX0.7\n"
"      Typing b<number> or w<number> alters simulated inputs IB1 or IW2\n"
"              <number> may be decimal 255, octal 0177 or hexadecimal 0xff\n"
"      Programmed outputs QX0.0 to QX0.7, QB1 and QW2 are displayed.\n"
#endif
"      Typing q or ctrl-C quits run mode.\n"
"Copyright (C) 1985-2001 John E. Wulff     <john@je-wulff.de>\n"
"%s\n";

const char *	progname;		/* name of this executable */
short		debug = 0;
int		micro = 0;
unsigned short	xflag;
unsigned short	iFlag;
unsigned short	osc_max = MARKMAX;
#ifdef YYDEBUG
extern	int	iCdebug;
#endif

#define inpFN	szNames[1]		/* input file name */
#define errFN	szNames[2]		/* error file name */
#define listFN	szNames[3]		/* list file name */
#define outFN	szNames[4]		/* C output file name */
#define exiFN	szNames[5]		/* cexe input file name */
#define excFN	szNames[6]		/* cexe C out file name */
char *		szNames[] = {		/* matches return in compile */
    INITIAL_FILE_NAMES
};

static FILE *	exiFP;			/* cexe in file pointer */
static FILE *	excFP;			/* cexe C out file pointer */
static char *	ppPath = "";		/* default pplstfix on PATH */

char * OutputMessage[] = {
    0,					/* [0] no error */
    "%s: syntax or generate errors\n",	/* [1] */
    "%s: block count error\n",		/* [2] */
    "%s: link count error\n",		/* [3] */
    "%s: cannot open file %s\n",	/* [4] */
    "%s: cannot open file %s in iC compile\n",	/* [5] */
    "%s: cannot open file %s in C compile\n",	/* [6] */
    "%s: cannot open file %s in output\n",	/* [7] */
};

FILE *	T1FP;
FILE *	T2FP;
FILE *	T3FP;
    
static char	T1FN[] = "ic1.XXXXXX";
static char	T2FN[] = "ic2.XXXXXX";
static char	T3FN[] = "ic3.XXXXXX";
static char	T4FN[] = "ic4.XXXXXX";


/********************************************************************
 *
 *	main program
 *
 *******************************************************************/

int
main(
    int		argc,
    char **	argv)
{
    int		fd;
    int		r = 0;		/* return value of compile */
    int		ro = 4;		/* output message index */

    /* Process the arguments */
    if ((progname = strrchr(*argv, '/')) == NULL) {
	progname = *argv;
    } else {
	progname++;		/*  path has been stripped */
    }
    inFP = stdin;		/* input file pointer */
    outFP = stdout;		/* listing file pointer */
    errFP = stderr;		/* error file pointer */
    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
		    int	debi;

		case '\0':
		    inpFN = 0;	/* - is standard input */
#ifdef TCP
		case 's':
		    if (! *++*argv) { --argc, ++argv; }
		    hostNM = *argv;
		    goto break2;
		case 'p':
		    if (! *++*argv) { --argc, ++argv; }
		    portNM = *argv;
		    goto break2;
		case 'u':
		    if (! *++*argv) { --argc, ++argv; }
		    iccNM = *argv;
		    goto break2;
		case 'm':
		    micro++;		/* microsecond info */
		    break;
#endif
		case 'd':
		    if (! *++*argv) { --argc, ++argv; }
		    sscanf(*argv, "%o", &debi);
		    debug |= debi;	/* short */
#ifdef YYDEBUG
		    if (debug & 0400) iCdebug = debug & 01;
#endif
		    goto break2;
		case 't':
		    debug |= 0100;	/* trace only */
		    break;
		case 'n':
		    if (! *++*argv) { --argc, ++argv; }
		    osc_max = atoi(*argv);
		    if (osc_max > 15) goto error;
		    goto break2;
		case 'o':
		    if (exiFN == 0) {
			if (! *++*argv) { --argc, ++argv; }
			outFN = *argv;	/* compiler output file name */
			goto break2;
		    } else {
			fprintf(stderr,
			    "%s: cannot use both -c and -o option\n", progname);
			goto error;
		    }
		case 'l':
		    if (! *++*argv) { --argc, ++argv; }
		    listFN = *argv;	/* listing file name */
		    goto break2;
		case 'e':
		    if (! *++*argv) { --argc, ++argv; }
		    errFN = *argv;	/* error file name */
		    goto break2;
		case 'c':
		    if (outFN == 0) {
			exiFN = "cexe.h";
			excFN = "cexe.c";
		    } else {
			fprintf(stderr,
			    "%s: cannot use both -o and -c option\n", progname);
			goto error;
		    }
		case 'A':
		    Aflag = 1;		/* generate ARITH ALIAS in outFN */
		    break;
		case 'a':
		    aflag = 1;		/* append for compile */
		    break;
		case 'x':
		    xflag = 1;		/* start with hexadecimal display */
		    break;
		case 'P':
		    if (! *++*argv) { --argc, ++argv; }
		    ppPath = *argv;	/* path of pplstfix */
		    goto break2;
		default:
		    fprintf(stderr,
			"%s: unknown flag '%c'\n", progname, **argv);
		case 'h':
		case '?':
		error:
		    fprintf(stderr, usage, progname, progname, progname,
#ifdef TCP
		    hostNM, portNM, iccNM,
#endif
		    MARKMAX, progname, progname, progname, SC_ID);
		    exit(1);
		}
	    } while (*++*argv);
	    break2: ;
	} else {
	    inpFN = *argv;
	}
    }
    debug &= 07777;			/* allow only cases specified */
    signal(SIGSEGV, quit);		/* catch memory access signal */	
    iFlag = 0;
    
    szNames[T1index] = T1FN;
    szNames[T2index] = T2FN;
    szNames[T3index] = T3FN;
    szNames[T4index] = T4FN;

    if ((fd = mkstemp(T1FN)) < 0 ||
	(T1FP = fdopen(fd, "w+")) == 0) {
	r = T1index;			/* error opening temporary file */
    } else if ((fd = mkstemp(T2FN)) < 0 ||
	(T2FP = fdopen(fd, "w+")) == 0) {
	r = T2index;			/* error opening temporary file */
    } else if ((fd = mkstemp(T3FN)) < 0 ||
	(T3FP = fdopen(fd, "w+")) == 0) {
	r = T3index;			/* error opening temporary file */
    } else if ((fd = mkstemp(T4FN)) < 0 ||
	unlink(T4FN) < 0) {
	fprintf(stderr, "cannot unlink %s\n", T4FN);
	perror("unlink");
	r = T4index;			/* error opening temporary file */
    } else

/********************************************************************
 *
 *	Call the iC compiler which builds a Symbol table and a logic
 *	net made up of the same Symbol structures linked by List_e's
 *
 *	As a side effect, all C-code fragments made up of literal blocks
 *	and C-actions are collected in the temporary file T1FN
 *
 *******************************************************************/

    if ((r = compile(inpFN, listFN, errFN, outFN)) != 0) {
	ro = 5;				/* compile error */
    } else

/********************************************************************
 *
 *	Regroup the generated C-code int literal blocks first, followed
 *	by C-actions. This is necessary to get all declarations at the
 *	start of the C-code.
 *
 *	Call the C compiler which recognizes any variables declared as
 *	imm bit or imm int variables. These variables can be used as
 *	values anywhere in the C-code and appropriate modification is
 *	carried out. Immediate variables which have been declared but
 *	not yet assigned may be assigned to in the C-code. Such assignment
 *	expressions are recognised and converted to a function calls.
 *
 *	If an immediate variable has already been assigned in the iC
 *	code, an attempt to assign it in the C-code is an error.
 *
 *******************************************************************/

    if ((r = c_compile(T1FP)) != 0) {
	ro = 6;				/* C-compile error */
    } else {
	Gate *		igp;
	unsigned	gate_count[MAX_LS];	/* accessed by icc() */

	if ((r = listNet(gate_count)) == 0) {
	    if (outFN == 0) {
		if (exiFN) {			/* -c option */
		    if ((excFP = fopen(excFN, "w")) == NULL) {
			r = COindex;
		    } else if ((exiFP = fopen(exiFN, "r")) == NULL) {
			r = CIindex;
		    } else {
			unsigned	linecnt = 1;
			char		lineBuf[256];

			/* copy C execution file Part 1 from beginning up to 'Q' */
			while (fgets(lineBuf, sizeof lineBuf, exiFP)) {
			    if (*lineBuf == 'Q') break;
			    fputs(lineBuf, excFP);
			    linecnt++;
			}
			/* copy C intermediate file up to EOF to C output file */
			copyXlate(excFP, excFN, &linecnt, 01);

			/* rewind intermediate file T1FN again */
			if (fseek(T1FP, 0L, SEEK_SET) != 0) {
			    r = T1index;
			} else {
			    /* copy C execution file Part 2 from remainder up to 'V' */
			    while (fgets(lineBuf, sizeof lineBuf, exiFP)) {
				if (*lineBuf == 'V') break;
				fputs(lineBuf, excFP);
				linecnt++;
			    }
			    /* copy C intermediate file up to EOF to C output file */
			    copyXlate(excFP, excFN, &linecnt, 02);

			    /* copy C execution file Part 3 from character after 'V 'up to EOF */
			    while (fgets(lineBuf, sizeof lineBuf, exiFP)) {
				fputs(lineBuf, excFP);
				linecnt++;
			    }
			}
			if (debug & 040) {
			    fprintf(outFP, "\nC OUTPUT: %s  (%d lines)\n", excFN, linecnt-1);
			}
			fclose(exiFP);
			fclose(excFP);
		    }
		} else if ((r = buildNet(&igp)) == 0) {/* generate execution network */
		    Symbol * sp = lookup("iClock");
		    assert (sp);		/* iClock initialized in init() */
		    c_list = sp->u.gate;	/* initialise clock list */
		    icc(igp, gate_count);	/* execute the compiled logic */
		}
	    } else {
		r = output(outFN);		/* generate network as C file */
	    }
	}
	if (r != 0) {
	    ro = 7;				/* error in output */
	    fprintf(stderr, OutputMessage[r < 4 ? r : 7], progname, szNames[r]);
	    r += 10;
	    goto closeFiles;
	}
    }
    if (r != 0) {
	fprintf(stderr, OutputMessage[ro], progname, szNames[r]);
    }

closeFiles: ;
    if (outFP != stdout) {
	fclose(outFP);
	if (listFN && iFlag) {
	    if (inversionCorrection() != 0) {
		r += 100;
	    }
	    iFlag = 0;
	}
    }
    if (errFP != stderr) fclose(errFP);
    if (T1FP) {
	fclose(T1FP);
	if (!(debug & 04000)) {
	    unlink(T1FN);
	}
    }
    if (T2FP) {
	fclose(T2FP);
	if (!(debug & 04000)) {
	    unlink(T2FN);
	}
    }
    if (T3FP) {
	fclose(T3FP);
	if (!(debug & 04000)) {
	    unlink(T3FN);
	}
    }
    return (r);	/* 1 - 6 compile errors, 11 - 16 output errors, 1xx pplstfix error */
} /* main */

/********************************************************************
 *
 *	Wrapper to call perl script 'pplstfix' as a post-processor
 *	to modify listings to resolve aliases. These occurr in the
 *	listing if an output is used, before it is defined as an alias.
 *
 *	In particular the automatic alias allocation associated with
 *	outputs used as inputs, before that output is assigned to, is
 *	cleared up in the listing. These changes affect only the
 *	listing. The output is correctly compiled.
 *
 *******************************************************************/

int
inversionCorrection(void)
{
    char	exStr[256];	
    char	tempName[] = "pplstfix.XXXXXX";
    int		r = 0;

    if ((debug & 04024) == 024) {	/* not suppressed, NET TOPOLOGY and LOGIC */
	r = 1;
	if (mkstemp(tempName) >= 0) {
	    unlink(tempName);		/* mktemp() is deemed dangerous */
	    r = rename(listFN, tempName); /* inversion correction needed */
	    if (r == 0) {
		sprintf(exStr, "%spplstfix %s > %s", ppPath, tempName, listFN);
		r = system(exStr);
		unlink(tempName);
	    }
	}
	if (r != 0) {
	    fprintf(stderr, "%s: system(\"%s\") could not be executed\n",
		progname, exStr);
	}
    }
    return r;
} /* inversionCorrection */

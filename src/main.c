static const char main_c[] =
"@(#)$Id: main.c,v 1.41 2004/01/05 16:58:18 jw Exp $";
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
#include	<assert.h>
#ifdef TCP
#include	"tcpc.h"
#endif	/* TCP */
#include	"icg.h"
#include	"icc.h"
#include	"comp.h"
#include	"cexe.h"

extern const char	SC_ID[];

static const char *	usage =
"USAGE:\n"
"  %s [-aAch] [-o<out>] [-l<list>] [-e<err>] [-d<debug>] [-P<path>] <iC_program>\n"
"      Options in compile mode only:\n"
"        -o <outFN>      name of compiler output file - sets compile mode\n"
"        -l <listFN>     name of list file  (default is stdout)\n"
"        -e <errFN>      name of error file (default is stderr)\n"
"        -a              append linking info for 2nd and later files\n"
"        -A              compile output ARITHMETIC ALIAS nodes for symbol debugging\n"
"        -c              generate auxiliary file cexe.c to extend %s compiler\n"
"                        (cannot be used if also compiling with -o)\n"
"        -d <debug>4000  supress listing alias post processor (save temp files)\n"
#if defined(RUN) || defined(TCP)
"                  +400  exit after initialisation\n"
#endif	/* RUN or TCP */
"                   +40  source listing\n"
"                   +20  net topology\n"
"                   +10  net statistics\n"
"                    +4  logic expansion\n"
#if YYDEBUG && !defined(_WINDOWS)
#if defined(RUN) || defined(TCP)
"                  |402  logic generation (exits after initialisation)\n"
"                  |401  yacc debug info  (listing on stdout only)\n"
#else	/* not RUN and not TCP */
"                    +2  logic generation\n"
"                    +1  yacc debug info  (listing on stdout only)\n"
#endif	/* RUN or TCP */
#endif	/* YYDEBUG and not _WINDOWS */
"        -P <path>       Path of script pplstfix when not on PATH (usually ./)\n"
"        <iC_program>    any iC language program file (extension .ic)\n"
"        -               or default: take iC source from stdin\n"
"        -h              this help text\n"
#if defined(RUN) || defined(TCP)
"Extra options for run mode: (direct interpretation)\n"
"  [-n<count>] [-"
#ifdef TCP
"s <server>] [-p <port>] [-u <unitID>] [-m[m]"
#endif	/* TCP */
#if YYDEBUG && !defined(_WINDOWS)
"t"
#endif	/* YYDEBUG and not _WINDOWS */
"x]\n"
"        -n <count>      maximum oscillator count (default is %d, limit 15)\n"
#if YYDEBUG && !defined(_WINDOWS)
"        -d <debug>2000  display scan_cnt and link_cnt\n"
#ifdef RUN
"                 +1000  I0 toggled every second\n"
#else	/* TCP */
"                 +1000  do not trace non-active timers TX0.n\n"
#endif	/* RUN */
"                  +200  display oscillator info\n"
"                  +100  initialisation and run time trace\n"
#ifdef TCP
"                    +2  trace I/O receive buffer\n"
"                    +1  trace I/O send buffer\n"
#endif	/* TCP */
#endif	/* YYDEBUG and not _WINDOWS */
#ifdef TCP
"        -s host ID of server      (default '%s')\n"
"        -p service port of server (default '%s')\n"
"        -u unit ID of this client (default '%s')\n"
"        -m              microsecond timing info\n"
"        -mm             more microsecond timing (internal time base)\n"
"                        can be toggled at run time by typing m\n"
#endif	/* TCP */
#if YYDEBUG && !defined(_WINDOWS)
"        -t              trace debug (equivalent to -d 100)\n"
"                        can be toggled at run time by typing t\n"
#endif	/* YYDEBUG and not _WINDOWS */
"        -x              arithmetic info in hexadecimal (default decimal)\n"
"                        can be changed at run time by typing x or d\n"
"      An <iC_program> containing only logical expressions can be interpreted\n"
"      with  %s -t <iC_program>. An <iC_program> containing arithmetic\n"
"      expressions requires relinking of %s with a new cexe.c generated\n"
"      by %s -c <iC_program> before <iC_program> can be interpreted.\n"
#ifndef TCP
"      Typing 0 to 7 toggles simulated inputs IX0.0 to IX0.7\n"
"      Typing b<num> w<num> or l<num> alters simulated inputs IB1, IW2 or IL4\n"
"              <num> may be decimal 255, octal 0177 or hexadecimal 0xff\n"
"      Programmed outputs QX0.0 to QX0.7, QB1, QB2 and QL4 are displayed.\n"
#endif	/* not TCP */
"      Typing q or ctrl-C quits run mode.\n"
#endif	/* RUN or TCP */
"Copyright (C) 1985-2001 John E. Wulff     <john@je-wulff.de>\n"
"%s\n";

const char *	progname;		/* name of this executable */
short		debug = 0;
int		micro = 0;
unsigned short	xflag;
unsigned short	iFlag;
unsigned short	osc_max = MARKMAX;
#if YYDEBUG
extern	int	iCdebug;
#endif	/* YYDEBUG */

#define inpFN	szNames[1]		/* input file name */
#define errFN	szNames[2]		/* error file name */
#define listFN	szNames[3]		/* list file name */
#define outFN	szNames[4]		/* C output file name */
#define excFN	szNames[5]		/* cexe C out file name */
char *		szNames[] = {		/* matches return in compile */
    INITIAL_FILE_NAMES
};

char		inpNM[BUFS] = "stdin";	/* original input file name */

FILE *		outFP;			/* listing file pointer */
FILE *		errFP;			/* error file pointer */

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

FILE *	T0FP = NULL;
FILE *	T1FP = NULL;
FILE *	T2FP = NULL;
FILE *	T3FP = NULL;
FILE *	T4FP = NULL;
FILE *	T5FP = NULL;

char		T0FN[] = "ic0.XXXXXX";
static char	T1FN[] = "ic1.XXXXXX";
static char	T2FN[] = "ic2.XXXXXX";
static char	T3FN[] = "ic3.XXXXXX";
char		T4FN[] = "ic4.XXXXXX";	/* must be in current directory */
char		T5FN[] = "ic5.XXXXXX";

static void	unlinkTfiles(void);

/********************************************************************
 *
 *	Constant compile typing data
 *
 *******************************************************************/

char *		full_type[]  = { FULL_TYPE };
char *		full_ftype[] = { FULL_FTYPE };
unsigned char	types[]      = { TYPES };
unsigned char	ftypes[]     = { FTYPES };
char		os[]         = OPS;
char		fos[]        = FOPS;

unsigned char	bitMask[]    = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,	/* 0 1 2 3 4 5 6 7 */
};

/********************************************************************
 *
 *	I/O arrays also used at compile time
 *
 *******************************************************************/

Gate *		IX_[IXD*8];		/* pointers to bit Input Gates */
Gate *		IB_[IXD];		/* pointers to byte Input Gates */
Gate *		IW_[IXD];		/* pointers to word Input Gates */
#if INT_MAX != 32767 || defined (LONG16)
Gate *		IL_[IXD];		/* pointers to long Input Gates */
#endif	/* INT_MAX != 32767 or LONG16 */
Gate *		TX_[TXD*8];		/* pointers to bit System Gates */
unsigned char	QX_[IXD];		/* Output bit field slots */
char		QT_[IXD];		/* Output type of slots */

/********************************************************************
 *
 *	Main program
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
    T0FP = stdin;		/* input file pointer */
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
#endif	/* TCP */
		case 'd':
		    if (! *++*argv) { --argc, ++argv; }
		    sscanf(*argv, "%o", &debi);
		    debug |= debi;	/* short */
#if !defined(RUN) && !defined(TCP)
		    debug |= 0400;	/* always stops */
#endif	/* not RUN and not TCP */
#if YYDEBUG
		    if (debug & 0400) iCdebug = debug & 01;
#endif	/* YYDEBUG */
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
		    if (excFN == 0) {
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
		    fprintf(stderr, usage, progname, progname,
#if defined(RUN) || defined(TCP)
		    MARKMAX,
#ifdef TCP
		    hostNM, portNM, iccNM,
#endif	/* TCP */
		    progname, progname, progname,
#endif	/* RUN or TCP */
		    SC_ID);
		    exit(1);
		}
	    } while (*++*argv);
	    break2: ;
	} else {
	    inpFN = *argv;
	}
    }
    debug &= 07777;			/* allow only cases specified */
    iFlag = 0;

/********************************************************************
 *
 *	Generate and open temporary files T1FN T2FN T3FN
 *	T0FN, T4FN and T5FN are only used if iC and/or C-include files
 *	are actually parsed
 *
 *******************************************************************/

    szNames[T0index] = T0FN;
    szNames[T1index] = T1FN;
    szNames[T2index] = T2FN;
    szNames[T3index] = T3FN;
    szNames[T4index] = T4FN;
    szNames[T5index] = T5FN;

    if ((fd = mkstemp(T1FN)) < 0 || (T1FP = fdopen(fd, "w+")) == 0) {
	r = T1index;			/* error opening temporary file */
    } else if ((fd = mkstemp(T2FN)) < 0 || (T2FP = fdopen(fd, "w+")) == 0) {
	r = T2index;			/* error opening temporary file */
    } else if ((fd = mkstemp(T3FN)) < 0 || (T3FP = fdopen(fd, "w+")) == 0) {
	r = T3index;			/* error opening temporary file */
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
 *	Regroup the generated C-code into literal blocks first, followed
 *	by C-actions. This is necessary to get all C-declarations at the
 *	start of the C-code.
 *
 *	Call the C compiler which recognizes any variables declared as
 *	imm bit or imm int variables. These variables can be used as
 *	values anywhere in the C-code and appropriate modification is
 *	carried out. Immediate variables which have been declared but
 *	not yet assigned may be (multiply) assigned to in the C-code.
 *	Such assignment expressions are recognised and converted to
 *	function calls.
 *
 *	If an immediate variable has already been (singly) assigned to
 *	in the iC code, an attempt to assign to it in the C-code is an
 *	error.
 *
 *******************************************************************/

    if ((r = c_compile(T1FP)) != 0) {
	ro = 6;				/* C-compile error */
    } else {
	Gate *		igp;
	unsigned	gate_count[MAX_LS];	/* accessed by icc() */

/********************************************************************
 *
 *	List network topology and statistics - this completes listing
 *
 *******************************************************************/

	if ((r = listNet(gate_count)) == 0) {

/********************************************************************
 *
 *	-o option: Output a C-file of all Gates, C-code and links
 *
 *******************************************************************/

	    if (outFN) {			/* -o option */
		r = output(outFN);		/* generate network as C file */
	    } else

/********************************************************************
*
*	-c option: Output a C-file cexe.c to rebuild compiler with C-code
*
*******************************************************************/

	    if (excFN) {			/* -c option */
		if ((excFP = fopen(excFN, "w")) == NULL) {
		    r = COindex;
		} else {
		    unsigned	linecnt = 1;

		    /* write C execution file Part 1 */
		    fprintf(excFP, cexe_part1);
		    linecnt += cexe_lines1;
		    /* copy C intermediate file up to EOF to C output file */
		    copyXlate(excFP, excFN, &linecnt, 01);

		    /* rewind intermediate file T1FN again */
		    if (fseek(T1FP, 0L, SEEK_SET) != 0) {
			r = T1index;
		    } else {
			/* write C execution file Part 2 */
			fprintf(excFP, cexe_part2);
			linecnt += cexe_lines2;
			/* copy C intermediate file up to EOF to C output file */
			copyXlate(excFP, excFN, &linecnt, 02);

			/* write C execution file Part 3 */
			fprintf(excFP, cexe_part3);
			linecnt += cexe_lines3;
			if (linecnt > (1 + cexe_lines1 + cexe_lines2 + cexe_lines3)) {
			    fprintf(excFP, cexe_part4, inpNM, SC_ID);
			    linecnt += cexe_lines4;
			    if (debug & 010) {
				fprintf(outFP, "\nC OUTPUT: %s  (%d lines)\n", excFN, linecnt-1);
			    }
			}
		    }
		    fclose(excFP);
		}
#if defined(RUN) || defined(TCP)
	    } else

/********************************************************************
*
*	Build a network of Gates and links for direct execution
*
*******************************************************************/

	    if ((r = buildNet(&igp)) == 0) {
		Symbol * sp = lookup("iClock");
		unlinkTfiles();

/********************************************************************
*
*	Execute the compiled iC logic directly
*
*******************************************************************/

		assert (sp);			/* iClock initialized in init() */
		c_list = sp->u.gate;		/* initialise clock list */
		icc(igp, gate_count);		/* execute the compiled logic */
		/* never returns - exits via quit() */
#endif	/* RUN or TCP */
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
	fflush(outFP);
	fclose(outFP);
	if (listFN && iFlag) {
	    if (inversionCorrection() != 0) {
		r += 100;
	    }
	    iFlag = 0;
	}
    }
    if (errFP != stderr) {
	fflush(errFP);
	fclose(errFP);
    }
    unlinkTfiles();
    return (r);	/* 1 - 6 compile errors, 11 - 16 output errors, 1xx pplstfix error */
} /* main */

/********************************************************************
 *
 *	close and unlink temporary files T1FN T2FN T3FN
 *
 *******************************************************************/

static void
unlinkTfiles(void)
{
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
    if (T4FP) {
	fclose(T4FP);
	if (!(debug & 04000)) {
	    unlink(T4FN);
	}
    }
    if (T5FP) {
	fclose(T5FP);
	if (!(debug & 04000)) {
	    unlink(T5FN);
	}
    }
} /* unlinkTfiles */

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
    int		fd;
    int		r = 0;

    if ((debug & 04024) == 024) {	/* not suppressed, NET TOPOLOGY and LOGIC */
	/* mktemp() is deemed dangerous */
	/* Cygnus won't unlink till file is closed */
	if ((fd = mkstemp(tempName)) < 0 ||
	    close(fd) < 0 ||
	    unlink(tempName) < 0 ||
	    rename(listFN, tempName) < 0 /* inversion correction needed */
	) {
	    fprintf(stderr, "%s: rename(%s, %s) failed\n",
		progname, listFN, tempName);
	} else {
	    sprintf(exStr, "%spplstfix%s %s > %s", ppPath, progname + strcspn(progname, "0123456789"), tempName, listFN);
	    r = system(exStr);
	    unlink(tempName);
	    if (r != 0) {
		fprintf(stderr, "%s: system(\"%s\") could not be executed\n",
		    progname, exStr);
	    }
	}
    }
    return r;
} /* inversionCorrection */

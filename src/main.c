static const char main_c[] =
"@(#)$Id: main.c,v 1.18 2001/03/02 12:56:32 jw Exp $";
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
#include	"icc.h"
#include	"comp.h"
#ifdef TCP
#include	"tcpc.h"
#endif

extern const char	SC_ID[];

static const char *	usage = "\
USAGE for compile mode:\n\
  %s [-aAc] [-o<out>] [-l<list>] [-e<err>] [-d<debug>] <iC_program>\n\
       Options in compile mode only:\n\
        -o <outFN>      name of compiler output file - sets compile mode\n\
        -a              append linking info for 2nd and later files\n\
        -A              compile output ARITHMETIC ALIAS nodes for symbol debugging\n\
        -c              generate auxiliary file cexe.c to extend icc compiler\n\
                        (cannot be used if also compiling with -o)\n\
USAGE for run mode:\n\
  %s [-txh]"
#ifdef TCP
" [-m[m]] [-s <server>] [-p <port>] [-u <unitID>]\n      "
#endif
" [-l<list>] [-e<err>] [-d<debug>] [-n<count>] <iC_program>\n\
       Options in both modes:\n"
#ifdef TCP
"        -s host ID of server      (default '%s')\n\
        -p service port of server (default '%s')\n\
        -u unit ID of this client (default '%s')\n"
#endif
"        -l <listFN>     name of list file  (default is stdout)\n\
        -e <errFN>      name of error file (default is stderr)\n\
        -d <debug>4000  supress listing alias post processor\n\
                 +2000  display scan_cnt and link_cnt\n\
                 +1000  I0 toggled every second\n\
                  +400  exit after initialisation\n\
                  +200  display loop info (+old style logic)\n\
                  +100  initialisation and run time info\n\
                   +40  net statistics\n\
                   +20  net topology\n\
                   +10  source listing\n\
                    +4  logic expansion\n"
#ifdef YYDEBUG
"                    +2  logic generation\n\
                    +1  yacc debug info\n"
#endif
"       Options in run mode only:\n\
        -t              trace debug (equivalent to -d 100)\n\
                        can be toggled at run time typing t\n"
#ifdef TCP
"        -m	        microsecond timing info\n\
        -mm	        more microsecond timing (internal time base)\n\
                        can be toggled at run time typing m\n"
#endif
"        -x              arithmetic info in hexadecimal (default decimal)\n\
                        can be changed at run time by typing x or d\n\
        -n <count>      maxinum loop count (default is %d, limit 15)\n\
        -h              this help text\n\
\n\
        <iC_program>    any iC language program file (extension .iC)\n\
                        - or default is stdin\n\
			typing q or ctrl-C quits run time mode\n\
Copyright (C) 1985-2001 John E. Wulff     <john.wulff@inka.de>\n\
%s\n";

short		debug = 0;
#ifdef TCP
int		micro = 0;
#endif
unsigned short	xflag;
unsigned short	iFlag;
unsigned short	osc_max = MARKMAX;
#ifdef YYDEBUG
extern	int	yydebug;
#endif

char *		szFile_g;		/* file name to process */
#define progFN	szNames[0]		/* for error messages */
#define inpFN	szNames[1]		/* input file name */
#define errFN	szNames[2]		/* error file name */
#define listFN	szNames[3]		/* list file name */
#define outFN	szNames[4]		/* compiler output file name */
#define exiFN	szNames[5]		/* cexe input file name */
#define excFN	szNames[6]		/* cexe C out file name */
#define exoFN	szNames[7]		/* cexe output file name */
char *		szNames[11] = {		/* matches return in compile */
    	0, 0, 0, 0, 0, 0, 0, Tname, Cname, Hname, Lname,
};

static FILE *	exiFP;			/* cexe in file pointer */
static FILE *	excFP;			/* cexe C out file pointer */

char * OutputMessage[] = {
    0,					/* [0] no error */
    "%s: syntax or generate errors\n",	/* [1] */
    "%s: block count error\n",		/* [2] */
    "%s: link count error\n",		/* [3] */
    "%s: cannot open file %s\n",	/* [4] */
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
		    szFile_g = 0;	/* - is standard input */
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
		    debug = debi;	/* short */
#ifdef YYDEBUG
		    yydebug = debug & 01;
#endif
		    goto break2;
		case 't':
		    debug = 0100;		/* trace only */
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
			exoFN = Tname;
			goto break2;
		    } else {
			fprintf(stderr,
			    "%s: cannot use both -c and -o option\n", progFN);
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
			    "%s: cannot use both -o and -c option\n", progFN);
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
		default:
		    fprintf(stderr,
			"%s: unknown flag '%c'\n", progFN, **argv);
		case 'h':
		case '?':
		error:
		    fprintf(stderr, usage, progFN, progFN,
#ifdef TCP
		    hostNM, portNM, iccNM,
#endif
		    MARKMAX, SC_ID);
		    exit(1);
		}
	    } while (*++*argv);
	    break2: ;
	} else {
	    inpFN = szFile_g = *argv;
	}
    }
    debug &= 07777;			/* allow only cases specified */
    iFlag = 0;
    if ((r = compile(listFN, errFN, outFN, exiFN, exoFN)) != 0) {
	fprintf(stderr, OutputMessage[4], progFN, szNames[r]);
    } else {
	Gate *		igp;
	unsigned	gate_count[MAX_LS];	/* accessed by icc() */

	if ((r = listNet(gate_count)) == 0) {
	    if (outFN == 0) {
		if (exiFN != 0 && exoFP) {
		    /* rewind intermediate file Tname */
		    if (fseek(exoFP, 0L, SEEK_SET) != 0) {
			r = 7;
		    } else if ((excFP = fopen(excFN, "w")) == NULL) {
			r = 6;
		    } else if ((exiFP = fopen(exiFN, "r")) == NULL) {
			r = 5;
		    } else {
			int		c;
			unsigned	linecnt = 0;	/* not neede here */

			/* copy C execution file Part 1 from beginning up to 'Q' */
			while ((c = getc(exiFP)) != 'Q') {
			    if (c == EOF) {
				r = 5;	/* unexpected end of exiNM */
				break;
			    }
			    putc(c, excFP);
			}
			/* copy C intermediate file up to EOF to C output file */
			/* translate any ALIAS references of type '_(QB1_0)' */
			copyXlate(exoFP, excFP, &linecnt, 01);

			/* rewind intermediate file Tname again */
			if (fseek(exoFP, 0L, SEEK_SET) != 0) {
			    r = 7;
			} else {
			    /* copy C execution file Part 2 from remainder up to 'V' */
			    while ((c = getc(exiFP)) != 'V') {
				if (c == EOF) {
				    r = 5;	/* unexpected end of exiNM */
				    break;
				}
				putc(c, excFP);
			    }
			    /* copy C intermediate file up to EOF to C output file */
			    /* translate any ALIAS references of type '_(QB1_0)' */
			    copyXlate(exoFP, excFP, &linecnt, 02);

			    /* copy C execution file Part 3 from character after 'V 'up to EOF */
			    while ((c = getc(exiFP)) != EOF) {
				putc(c, excFP);
			    }
			}
			fclose(exiFP);
			fclose(excFP);
		    }
		} else if ((r = buildNet(&igp)) == 0) {/* generate execution network */
		    c_list = (lookup("iClock"))->u.gate;/* initialise clock list */
		    icc(igp, gate_count);	/* execute the compiled logic */
		}
	    } else {
		r = output(outFN);		/* generate network as C file */
	    }
	}
	if (r != 0) {
	    fprintf(stderr, OutputMessage[r < 4 ? r : 4], progFN, szNames[r]);
	    r += 10;
	}
    }
    if (outFP != stdout) {
#ifndef _WINDOWS
	fclose(outFP);
#endif
	if (r == 0 && listFN && iFlag) {
	    r = inversionCorrection();
	    iFlag = 0;
	}
    }
    if (errFP != stderr) fclose(errFP);
    if (exoFP) {
	fclose(exoFP);
	if (exoFN && !(debug & 04000)) {
	    unlink(Tname);
	}
    }
    return (r);	/* 1 - 6 compile errors, 11 - 16 output errors */
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
	if (mkstemp(tempName)) {
	    unlink(tempName);		/* mktemp() is deemed dangerous */
	    r = rename(listFN, tempName); /* inversion correction needed */
	    if (r == 0) {
		sprintf(exStr, "pplstfix %s > %s", tempName, listFN);
		r = system(exStr);		/* pplstfix must be in $PATH */
		unlink(tempName);
	    }
	}
	if (r != 0) {
	    fprintf(stderr, "%s: system(\"%s\") could not be executed\n",
		progFN, exStr);
	}
    }
    return r;
} /* inversionCorrection */

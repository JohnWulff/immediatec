static const char lmain_c[] =
"@(#)$Id: lmain.c,v 1.14 2005/08/27 21:53:53 jw Exp $";
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
#include	<unistd.h>
#include	<string.h>
#include 	"icc.h"
#include 	"comp.h"

static const char *	usage =
"USAGE: %s [-lh][ -Dmacro[=defn]...][ -Umacro...][ -d<debug>] <C_program>\n"
"        -l <listFN>     name of list file  (default is stdout)\n"
"        -D <macro>      predefine <macro> for the iC and C preprocessor phases\n"
"        -U <macro>      cancel any previous definitions of <macro>\n"
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
#ifdef EFENCE
"        -E              test Electric Fence ABOVE - SIGSEGV signal unless\n"
"        -B              export EF_PROTECT_BELOW=1 which tests access BELOW \n"
"        -F              export EF_PROTECT_FREE=1 which tests access FREE\n"
#endif	/* EFENCE */
"        -h              this help text\n"
"Copyright (C) 1985-2005 John E. Wulff     <john@je-wulff.de>\n"
"%s\n";

FILE *	yyin;
FILE *	yyout;
FILE *	iC_outFP;
FILE *	T6FP = NULL;
char *		iC_progname;		/* name of this executable */
char		T0FN[] = "ic0.XXXXXX";
char		T6FN[] = "ic6.XXXXXX";
char *		inpPath = 0;		/* name of input file */
#ifndef EFENCE
char		inpNM[BUFS] = "stdin";	/* original input file name */
char		iC_defines[BUFS] = "";	/* -D definitions from command line */
#else
char *		inpNM;
char *		iC_defines;
unsigned short	iC_xflag;
#endif	/* EFENCE */
char *		listFN = 0;		/* name of list file */
short		iC_debug = 0400;
#if YYDEBUG
extern	int	c_debug;
#endif

int
main(
    int		argc,
    char **	argv)
{
    char	execBuf[BUFS];
    char	lineBuf[BUFS];
    int		fd;
    int		r = 1;

    /* Process the arguments */
    if ((iC_progname = strrchr(*argv, '/')) == NULL) {
	iC_progname = *argv;
    } else {
	iC_progname++;		/*  path has been stripped */
    }
#ifdef EFENCE
    inpNM = iC_emalloc(BUFS);
    inpNM = strcpy(inpNM, "stdin");	/* original input file name */
    iC_defines = iC_emalloc(BUFS);	/* initialized with empty string "" */
#endif	/* EFENCE */
    iC_outFP = stdout;		/* listing file pointer */
    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
		    int	debi;
		    int	slen;
		    int	arg;

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
		case 'D':
		case 'U':
		    arg = **argv;
		    if (! *++*argv) { --argc, ++argv; }
		    if (strlen(*argv)) {
			slen = strlen(iC_defines);	/* space used so far */
			if (snprintf(iC_defines + slen, BUFS - slen, " -%c%s", arg, *argv) >= BUFS - slen) {
			    fprintf(stderr, "%s: -%c %s: does not fit in internal buffer\n",
				iC_progname, arg, *argv);
			    goto error;
			}
		    }
		    goto break2;
#ifdef EFENCE
		case 'E':
		    iC_xflag = inpNM[BUFS-1];
		    iC_xflag = inpNM[BUFS];
		    iC_xflag = inpNM[BUFS+1];
		    iC_xflag = inpNM[BUFS+2];
		    iC_xflag = inpNM[BUFS+3];	/* test Electric Fence ABOVE */
		    goto error;			/* should cause SIGSEGV signal  */
		case 'B':
		    iC_xflag = inpNM[0];
		    iC_xflag = inpNM[-1];	/* test Electric Fence BELOW */
		    goto error;			/* SIGSEGV if EF_PROTECT_BELOW  */
		case 'F':
		    iC_xflag = inpNM[0];
		    free(inpNM);
		    iC_xflag = inpNM[0];	/* test Electric Fence FREE */
		    goto error;			/* SIGSEGV if EF_PROTECT_FREE  */
#endif	/* EFENCE */
		default:
		    fprintf(stderr,
			"%s: unknown flag '%c'\n", iC_progname, **argv);
		case 'h':
		case '?':
		error:
		    fprintf(stderr, usage, iC_progname, lmain_c);
		    exit(1);
		}
	    } while (*++*argv);
	    break2: ;
	} else {
	    inpPath = *argv;
	}
    }
    iC_debug &= 07777;			/* allow only cases specified */

    if (listFN && (iC_outFP = fopen(listFN, "w+")) == NULL) {
	ierror("lmain: cannot open list file:", listFN);
	return -3;
    }
    yyout = iC_outFP;

    if (inpPath) {
	strncpy(inpNM, inpPath, BUFS);
	r = 0;
	if (strlen(iC_defines) == 0) {
	    /* pre-compile if C files contains any #include, #define #if etc */
	    snprintf(execBuf, BUFS, "grep -q '^[ \t]*#' %s", inpPath);
	    r = system(execBuf);		/* test with grep if #include in input */
	}
	if (r == 0) {
	    /* iC_defines is not empty and has -Dyyy or -Uyyy or #xxx was found by grep */
	    /* pass the input file through the C pre-compiler to resolve #includes */
	    if ((fd = mkstemp(T0FN)) < 0 || close(fd) < 0 || unlink(T0FN) < 0) {
		ierror("compile: cannot make or unlink:", T0FN);
		perror("unlink");
		return -1;			/* error unlinking temporary file */
	    } else if ((fd = mkstemp(T6FN)) < 0 || close(fd) < 0 || unlink(T6FN) < 0) {
		ierror("compile: cannot make or unlink:", T6FN);
		perror("unlink");
		return -1;			/* error unlinking temporary file */
	    }
	    /* Cygnus does not understand cc - use macro CC=gcc - pass comments with -C */
	    snprintf(execBuf, BUFS, SS(CC) "%s -E -C -I/usr/local/include -x c %s -o %s 2> %s",
		iC_defines, inpPath, T0FN, T6FN);
	    r = system(execBuf);		/* Pre-compile iC file */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### compile: %s; $? = %d\n", execBuf, r>>8);
#endif
	    if (r != 0) {
		if ((T6FP = fopen(T6FN, "r")) == NULL) {
		    return T6index;		/* error opening CC error file */
		}
		while (fgets(lineBuf, sizeof lineBuf, T6FP)) {
		    ierror(SS(CC) ":", lineBuf);	/* CC error message */
		}
		fclose(T6FP);
		if (!(iC_debug & 04000)) {
		    unlink(T6FN);
		}
		ierror("compile: cannot run:", execBuf);
		return -1;
	    }
	    if (!(iC_debug & 04000)) {
		unlink(T6FN);
	    }
	    if ((yyin = fopen(T0FN, "r")) == NULL) {
		ierror("lmain: cannot open intermediate file:", T0FN);
		return -1;			/* error opening intermediate file */
	    }
	} else if ((yyin = fopen(inpNM, "r")) == NULL) {
	    ierror("lmain: cannot open input file:", inpNM);
	    return -2;				/* error opening input file */
	}
    } else {
	ierror("lmain: no input file:", NULL);
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
#ifdef EFENCE
    free(inpNM);			/* clean up for purify */
#endif	/* EFENCE */
    return r;
} /* main */

static const char icc_c[] =
"@(#)$Id: icc.c,v 1.60 2008/03/27 11:59:24 jw Exp $";
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
 *	main.c
 *	command line interpretation and starter for immcc compiler
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#ifndef	WIN32
#include	<unistd.h>
#endif	/* WIN32 */
#include	<string.h>
#include	<setjmp.h>
#include	<assert.h>
#include	<errno.h>
#include	"icc.h"
#include	"comp.h"
#ifdef TCP
#include	"tcpc.h"
#endif	/* TCP */
#include	"cexe.h"
#ifdef	LOAD
#error - must be compiled without LOAD defined to make an executable compiler
#endif	/* LOAD */

extern const char	iC_ID[];
unsigned short		iC_gflag = 0;		/* -g independent C code for gdb debugging */
unsigned short		iC_Aflag = 0;		/* -A flag signals ARITH alias */
unsigned short		iC_Sflag = 0;		/* not strict */
unsigned short *	iC_useTypes[] = { USETYPEFLAGS };
unsigned short		iC_Arestore = 0;	/* saved -A flag signals ARITH alias */
unsigned short		iC_Srestore = 0;	/* saved strict */
unsigned short *	iC_useRestore[] = { USERESTOREFLAGS };
unsigned short		iC_optimise = 07;	/* optimisation levels 0 - 7 */
int			iC_genCount;

static const char *	usage =
"USAGE: %s [-acgASRh][ -o<out>][ -l<lst>][ -e<err>][ -k<lim>][ -d<deb>][ -O<level>]\n"
"    [ -Dmacro[=defn]...][ -Umacro...][ -Cmacro[=defn]...][ -Vmacro...][ -W[no-]<warn>] <src.ic>\n"
#if defined(RUN) || defined(TCP)
"Options in compile mode (-o or -c):\n"
#endif	/* RUN or TCP */
"        -o <out>        name of generated C output file\n"
"        -c              generate C source cexe.c to extend 'icr' or 'ict' compiler\n"
"                        (cannot be used if also compiling with -o)\n"
"        -l <lst>        name of list file  (default: none, '' is stdout) output:\n"
"                        listing with logic expansion, net topology and statistics\n"
"        -e <err>        name of error file (default is stderr)\n"
"        -a              compile with linking information in auxiliary files\n"
"        -g              each expression has its own C code for debugging with gdb\n"
"        -A              compile output ARITHMETIC ALIAS nodes for symbol debugging\n"
"        -S              strict compile - all immediate variables must be declared\n"
"        -P              pedantic: warning if variable contains $ (default $ allowed)\n"
"        -PP             pedantic-error: error if variable contains $\n"
"        -R              no maximum error count (default: abort after 100 errors)\n"
"        -O <level>      optimisation -O0 none -O1 bit -O2 arithmetic -O4 eliminate \n"
"                        duplicate arithmetic expressions; -O7 all (default)\n"
"        -D <macro>      predefine <macro> for the iC preprocessor phase\n"
"        -U <macro>      cancel previous definition of <macro> for the iC phase\n"
"                        Note: do not use the same macros for the iC and the C phase\n"
"        -C <macro>      predefine <macro> for the C preprocessor phase\n"
"        -V <macro>      cancel previous definition of <macro> for the C phase\n"
"        -W[no-]<warn>   positive/negative warning options\n"
"                        -Wno-deprecated-logic -Wno-deprecated -Wdeprecated (default)\n"
#if defined(RUN) || defined(TCP)
"        -k <lim>        highest I/O index (default: %d; also run and -c mode limit)\n"
"                        if lim <= %d, mixed byte, word and long indices are tested\n"
#else	/* not RUN or TCP */
"        -k <lim>        highest I/O index (default: no limit; %d for -c mode)\n"
"                        if lim <= %d, mixed byte, word and long indices are tested\n"
"                        default: any index for bit, byte, word or long is allowed\n"
#endif	/* RUN or TCP */
"        -d <deb>                          LIST options\n"
"                   +40  source listing\n"
"                   +20  net topology\n"
"                   +10  net statistics\n"
"                    +4  logic expansion\n"
"                  +204  numerical values prepared for iClive display\n"
#if defined(RUN) || defined(TCP)
"                  +400  exit after initialisation\n"
#endif	/* RUN or TCP */
#if YYDEBUG && !defined(_WINDOWS)
"                                          DEBUG options\n"
#if defined(RUN) || defined(TCP)
"                  +402  logic generation (exits after initialisation)\n"
#ifdef YACC
"                  +401  iC yacc debug info  (on stdout only) (+4401 C yacc)\n"
#else	/* BISON */
"                  +401  iC bison debug info (on stderr only) (+4401 C bison)\n"
#endif	/* YACC */
#else	/* not RUN and not TCP */
"                    +2  logic generation                     (+4002 Symbol Table)\n"
#ifdef YACC
"                    +1  iC yacc debug info  (on stdout only) (+4001 C yacc)\n"
#else	/* BISON */
"                    +1  iC bison debug info (on stderr only) (+4001 C bison)\n"
#endif	/* YACC */
#endif	/* RUN or TCP */
"                 +4000  supress listing alias post processor (save temp files)\n"
"                +10000  generate listing of compiler internal functions\n"
"                +20000  use old style imm functions - no internal functions\n"
"                +21000  outputs first (really old style) - no internal functions\n"
#endif	/* YYDEBUG and not _WINDOWS */
"        <src.ic>        iC language source file (extension .ic)\n"
"        -               or default: take iC source from stdin\n"
"        -h              this help text\n"
#ifdef EFENCE
"        -E              test Electric Fence ABOVE - SIGSEGV signal unless\n"
"        -B              export EF_PROTECT_BELOW=1 which tests access BELOW \n"
"        -F              export EF_PROTECT_FREE=1 which tests access FREE\n"
#endif	/* EFENCE */
#if defined(RUN) || defined(TCP)
"Extra options for run mode: (direct interpretation)\n"
"  [-n<count>"
#ifdef TCP
"][ -s <server>][ -p <port>][ -u <unitID>"
#endif	/* TCP */
#if YYDEBUG && !defined(_WINDOWS)
"][ -tx"
#ifdef TCP
"m[m]"
#endif	/* TCP */
#endif	/* YYDEBUG and not _WINDOWS */
"]\n"
#ifdef TCP
"        -s host ID of server      (default '%s')\n"
"        -p service port of server (default '%s')\n"
"        -u unit ID of this client (default base name of <src.ic>)\n"
"        -i instance ID of this client (default '%s'; 1 to %d numeric digits)\n"
#endif	/* TCP */
"        -n <count>      maximum oscillator count (default is %d, limit 15)\n"
#if YYDEBUG && !defined(_WINDOWS)
"        -d <debug>2000  display scan_cnt and link_cnt\n"
"                 +1000  do not trace non-active timers TX0.n\n"
"                  +200  display oscillator info\n"
"                  +100  initialisation and run time trace\n"
#ifdef TCP
"                    +2  trace I/O receive buffer\n"
"                    +1  trace I/O send buffer\n"
#endif	/* TCP */
#endif	/* YYDEBUG and not _WINDOWS */
#if YYDEBUG && !defined(_WINDOWS)
"        -t              trace gate activity (equivalent to -d 1100)\n"
"                        can be toggled at run time by typing t\n"
"        -x              arithmetic info in hexadecimal (default decimal)\n"
"                        can be changed at run time by typing x or d\n"
#ifdef TCP
"        -m              microsecond timing info\n"
"        -mm             more microsecond timing (internal time base)\n"
"                        can be toggled at run time by typing m\n"
#endif	/* TCP */
#endif	/* YYDEBUG and not _WINDOWS */
"      A <src.ic> containing only logical expressions can be interpreted\n"
"      with  %s -t <src.ic>. A <src.ic> containing arithmetic expressions\n"
"      requires relinking of %s with a new cexe.c generated by executing\n"
#ifdef TCP
"      %s -c <src.ic>; makeAll -T; before <src.ic> can be interpreted.\n"
#else	/* not TCP */
"      %s -c <src.ic>; makeAll -R; before <src.ic> can be interpreted.\n"
"      Typing 0 to 7 toggles simulated inputs IX0.0 to IX0.7\n"
"      Typing b<num> w<num> or l<num> alters simulated inputs IB1, IW2 or IL4\n"
"              <num> may be decimal 255, octal 0177 or hexadecimal 0xff\n"
"      Programmed outputs QX0.0 to QX0.7, QB1, QB2 and QL4 are displayed.\n"
#endif	/* not TCP */
"      Typing q or ctrl-C quits run mode.\n"
#endif	/* RUN or TCP */
"%s\n"
"Copyright (C) 1985-2005 John E. Wulff     <john@je-wulff.de>\n"
;

char *		iC_progname;		/* name of this executable */
short		iC_debug = 0;
int		iC_micro = 0;
int		iC_Pflag = 0;		/* pedantic warning/error flag */
int		iC_Wflag = W_DEPRECATED_LOGIC;	/* do/no logic warnings */
unsigned short	iC_xflag;
unsigned short	iFlag;
unsigned short	iC_osc_max = MARKMAX;
unsigned short	iC_osc_lim = MARKMAX;
#if YYDEBUG
extern	int	iCdebug;
extern	int	c_debug;
#endif	/* YYDEBUG */

#define errFN	szNames[2]		/* error file name */
#define listFN	szNames[3]		/* list file name */
#define outFN	szNames[4]		/* C output file name */
#define excFN	szNames[5]		/* cexe C out file name */
char *		szNames[] = {		/* matches return in compile */
    INITIAL_FILE_NAMES
};

#ifndef EFENCE
char		inpNM[BUFS] = "stdin";	/* original input file name */
char		iC_defines[BUFS] = "";	/* -D -U definitions from command line */
char		iC_Cdefines[BUFS] = "";	/* -C -V definitions from command line */
#else
char *		inpNM;
char *		iC_defines;
char *		iC_Cdefines;
#endif	/* EFENCE */

FILE *		iC_outFP;		/* listing file pointer */
FILE *		iC_errFP;		/* error file pointer */

static FILE *	excFP;			/* cexe C out file pointer */
static char *	iC_path;		/* default pplstfix on PATH */
jmp_buf		beginMain;
int		iC_maxErrCount = 100;	/* default error count at which to abort compile */
int		iC_iErrCount = 0;	/* count errors - abort after 100 errors */

char * OutputMessage[] = {
    0,					/* [0] no error */
    "%s: syntax or generate errors\n",	/* [1] */
    "%s: block count error\n",		/* [2] */
    "%s: link count error\n",		/* [3] */
    "%s: cannot open file %s\n",	/* [4] */
    "%s: cannot open or compile file %s in iC compile\n",	/* [5] */
    "%s: cannot open or compile file %s in C compile\n",	/* [6] */
    "%s: cannot open file %s in output\n",	/* [7] */
};

FILE *	T0FP = NULL;
FILE *	T1FP = NULL;
FILE *	T2FP = NULL;
FILE *	T3FP = NULL;
FILE *	T4FP = NULL;
FILE *	T5FP = NULL;
FILE *	T6FP = NULL;

char		T0FN[] = "ic0.XXXXXX";
static char	T1FN[] = "ic1.XXXXXX";
char		T2FN[] = "ic2.XXXXXX";
static char	T3FN[] = "ic3.XXXXXX";
char		T4FN[] = "ic4.XXXXXX";	/* must be in current directory */
char		T5FN[] = "ic5.XXXXXX";
char		T6FN[] = "ic6.XXXXXX";

static void	unlinkTfiles(void);

/********************************************************************
 *
 *	Constant compile typing data
 *
 *******************************************************************/

char *		iC_full_type[]	= { FULL_TYPE };
char *		iC_full_ftype[]	= { FULL_FTYPE };
unsigned char	iC_types[]	= { TYPES };
unsigned char	iC_ctypes[]	= { CTYPES };
unsigned char	iC_ftypes[]	= { FTYPES };
char		iC_os[]		= OPS;
char		iC_fos[]	= FOPS;
char *		iC_useText[4]	= { USE_TEXT };

unsigned char	iC_bitMask[]    = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,	/* 0 1 2 3 4 5 6 7 */
};

/********************************************************************
 *
 *	I/O arrays also used at compile time
 *
 *******************************************************************/

#if defined(RUN)
Gate *		iC_IX_[IXD*8];		/* pointers to bit Input Gates */
Gate *		iC_IB_[IXD];		/* pointers to byte Input Gates */
Gate *		iC_IW_[IXD];		/* pointers to word Input Gates */
#if INT_MAX != 32767 || defined (LONG16)
Gate *		iC_IL_[IXD];		/* pointers to long Input Gates */
#endif	/* INT_MAX != 32767 or LONG16 */
Gate *		iC_TX_[TXD*8];		/* pointers to bit System Gates */
char		iC_QT_[IXD];		/* Output type of slots */
int		iC_maxIO = IXD;		/* I/O index limited to 64 */
#else	/*  not RUN */
int		iC_maxIO = -1;		/* I/O index no limit */
#endif	/*  not RUN */

#if defined(RUN) || defined(TCP)
Gate *		iC_pf0_1;		/* pointer to _f0_1 if generated in buildNet() */
#endif /* defined(RUN) || defined(TCP) */

unsigned char	iC_QX_[IXD];		/* Output bit field slots (also used in compile) */

#if defined(TCP)
/********************************************************************
 *
 *	I/O links for display()
 *
 *******************************************************************/

Gate *		iC_TX0p;			/* pointer to bit System byte Gates */

Gate *		iC_IX0p;			/* pointer to Input Gate for bit iC_display */
Gate *		iC_QX0p;			/* pointer to Output Gate for bit iC_display */
Gate *		iC_IB1p;			/* pointer to Input Gate for byte iC_display */
Gate *		iC_QB1p;			/* pointer to Output Gate for byte iC_display */
Gate *		iC_IW2p;			/* pointer to Input Gate for word iC_display */
Gate *		iC_QW2p;			/* pointer to Output Gate for word iC_display */
#if	INT_MAX != 32767 || defined (LONG16)
Gate *		iC_IL4p;			/* pointer to Input Gate for long iC_display */
Gate *		iC_QL4p;			/* pointer to Output Gate for long iC_display */
#endif	/* INT_MAX != 32767 || defined (LONG16) */
#endif	/* TCP */

/********************************************************************
 *
 *	Open auxiliary files T4FN and T5FN
 *			make and open T4FN; close and re-open if it exists
 *	mode == 1:	make and open T5FN; close and re-open if it exists
 *	mode == 0:	make and close T5FN; gcc -E call will overwrite it
 *
 *******************************************************************/

int
openT4T5(int mode)
{
    int		fd;
    static int	T5flag = 0;		/* set when mkstemp(T5FN) is executed */

    if (T4FP == 0) {
	if ((fd = mkstemp(T4FN)) < 0 || (T4FP = fdopen(fd, "w+")) == 0) {
	    ierror("openT4T5: cannot open:", T4FN);
	    return T4index;		/* error opening temporary file */
	}
    } else
    if (fclose(T4FP) < 0 || (T4FP = fopen(T4FN, "w+")) == NULL) {
	ierror("openT4T5: cannot open:", T4FN);
	return T4index;
    }
    if (mode) {
	if (T5FP == 0) {
	    if (T5flag == 0) {
		if ((fd = mkstemp(T5FN)) < 0 || (T5FP = fdopen(fd, "w+")) == 0) {
		    ierror("openT4T5: cannot open:", T5FN);
		    return T5index;	/* error opening temporary file */
		}
		T5flag = 1;		/* generate name only once */
	    } else
	    if ((T5FP = fopen(T5FN, "w+")) == NULL) {
		ierror("openT4T5: cannot open:", T5FN);
		return T5index;		/* error opening temporary file */
	    }
	} else
	if (fclose(T5FP) < 0 || (T5FP = fopen(T5FN, "w+")) == NULL) {
	    ierror("openT4T5: cannot open:", T5FN);
	    return T5index;
	}
    } else {
	if (T5FP) {
	    assert(T5flag);
	    if (fclose (T5FP) < 0 || unlink(T5FN) < 0) {
		ierror("openT4T5: cannot close or unlink:", T5FN);
		perror("unlink");
		return T5index;		/* error unlinking temporary file */
	    }
	    T5FP = 0;			/* overwrite intermediate file */
	} else
	if (T5flag == 0) {
	    if ((fd = mkstemp(T5FN)) < 0 || close(fd) < 0 || unlink(T5FN) < 0) {
		ierror("openT4T5: cannot make or unlink:", T5FN);
		perror("unlink");
		return T5index;		/* error unlinking temporary file */
	    }
	    T5flag = 1;			/* generate name only once */
	}
    }
    return 0;
} /* openT4T5 */

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
#ifdef TCP
#endif	/* TCP */
    int		r = 0;			/* return value of compile */
    int		ro = 4;			/* output message index */
    char *	cp;

    /* Process the arguments */
    iC_path = *argv;			/* in case there is a leading path/ */
    if ((iC_progname = strrchr(iC_path, '/')) == NULL &&
        (iC_progname = strrchr(iC_path, '\\')) == NULL) {
	iC_progname = iC_path;		/* no leading path */
	iC_path     = "";		/* default PATH for pplstfix */
    } else {
	*iC_progname++ = '\0';		/* path has been stripped and isolated */
    }

#ifdef EFENCE
    inpNM = iC_emalloc(BUFS);
    inpNM = strcpy(inpNM, "stdin");	/* original input file name */
    iC_defines = iC_emalloc(BUFS);	/* initialized with empty string "" */
    iC_Cdefines = iC_emalloc(BUFS);	/* initialized with empty string "" */
    iCbuf = iC_emalloc(IMMBUFSIZE);	/* buffer to build imm statement */
    iFunBuffer = iC_emalloc(IBUFSIZE);	/* buffer to build imm function symbols */
    iFunEnd = &iFunBuffer[IBUFSIZE];	/* pointer to end */
#endif	/* EFENCE */
    functionUseSize = FUNUSESIZE;	/* initialise function use database */
    functionUse = (FuUse*)realloc(NULL, FUNUSESIZE * sizeof(FuUse));
    memset(functionUse, '\0', FUNUSESIZE * sizeof(FuUse));
    T0FP = stdin;			/* input file pointer */
    iC_outFP = stdout;			/* listing file pointer */
    iC_errFP = stderr;			/* error file pointer */
    iC_aflag = iC_lflag = 0;		/* don't append for compile */
    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
		    int		debi;
		    int		slen;
		    int		arg;
		    char *	mp;
		    char *	op;
		    char	tempBuf[BUFS];

		case '\0':
		    inpFN = 0;		/* - is standard input */
#if defined(RUN) || defined(TCP)
		case 'n':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    iC_osc_lim = iC_osc_max = atoi(*argv);
		    if (iC_osc_max > 15) goto error;
		    goto break2;
#ifdef TCP
		case 's':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (strlen(*argv)) iC_hostNM = *argv;
		    goto break2;
		case 'p':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (strlen(*argv)) iC_portNM = *argv;
		    goto break2;
		case 'u':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (strlen(*argv)) iC_iccNM = *argv;
		    goto break2;
		case 'i':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if ((slen = strlen(*argv)) > INSTSIZE ||
			slen != strspn(*argv, "0123456789")) {
			fprintf(stderr, "WARNING '-i %s' is non numeric or longer than %d characters - ignored\n",
			    *argv, INSTSIZE);
		    } else {
			iC_iidNM = iC_emalloc(INSTSIZE+1);	/* +1 for '\0' */
			strncpy(iC_iidNM, *argv, INSTSIZE);
		    }
		    goto break2;
		case 'm':
		    iC_micro++;		/* microsecond info */
		    break;
#endif	/* TCP */
		case 'x':
		    iC_xflag = 1;	/* start with hexadecimal display */
		    break;
		case 't':
		    iC_debug |= 01100;	/* trace gate activity */
		    break;
#endif	/* RUN or TCP */
		case 'd':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    sscanf(*argv, "%o", &debi);
		    iC_debug |= debi;	/* short */
#if !defined(RUN) && !defined(TCP)
		    iC_debug |= 0400;	/* always stops */
#endif	/* not RUN and not TCP */
#if YYDEBUG
		    if (iC_debug & 0400) {
			if (iC_debug & 04000) {
			    c_debug = iC_debug & 01;
			} else {
			    iCdebug = iC_debug & 01;
			}
		    }
#endif	/* YYDEBUG */
		    goto break2;
		case 'o':
		    if (excFN == 0) {
			if (! *++*argv) { --argc; if(! *++argv) goto error; }
#ifdef	WIN32
			if (strlen(*argv)) {
			    outFN = iC_emalloc(strlen(*argv)+1);	/* +1 for '\0' */
			    strcpy(outFN, *argv);
			    while ((cp = strchr(outFN, '\\')) != 0) {
				*cp = '/';		/* convert '\' to '/' under WIN32 */
			    }
			}
#else	/* not WIN32 */
			if (strlen(*argv)) outFN = *argv;	/* compiler output file name */
#endif	/* WIN32 */
			goto break2;
		    } else {
			fprintf(stderr,
			    "%s: cannot use both -c and -o option\n", iC_progname);
			goto error;
		    }
		case 'l':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
#ifdef	WIN32
		    if (strlen(*argv)) {
			listFN = iC_emalloc(strlen(*argv)+1);	/* +1 for '\0' */
			strcpy(listFN, *argv);
			while ((cp = strchr(listFN, '\\')) != 0) {
			    *cp = '/';		/* convert '\' to '/' under WIN32 */
			}
		    }
#else	/* not WIN32 */
		    if (strlen(*argv)) listFN = *argv;	/* listing file name */
#endif	/* WIN32 */
		    if ((iC_debug & 077) == 0) {
			iC_debug |= 074;	/* default listing, topology, stats and logic expansion */
		    }
		    goto break2;
		case 'e':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
#ifdef	WIN32
		    if (strlen(*argv)) {
			errFN = iC_emalloc(strlen(*argv)+1);	/* +1 for '\0' */
			strcpy(errFN, *argv);
			while ((cp = strchr(errFN, '\\')) != 0) {
			    *cp = '/';		/* convert '\' to '/' under WIN32 */
			}
		    }
#else	/* not WIN32 */
		    if (strlen(*argv)) errFN = *argv;	/* error file name */
#endif	/* WIN32 */
		    goto break2;
		case 'k':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    iC_maxIO = atoi(*argv) + 1;
		    if (excFN != 0 && (iC_maxIO < 0 || iC_maxIO > IXD)) goto maxIOerror;
		    goto break2;
		case 'c':
		    if (outFN == 0) {
			excFN = "cexe.c";
			if (iC_maxIO < 0) iC_maxIO = IXD;	/* pre-set for immcc is -1 */
			if (iC_maxIO > IXD) {		/* I/O limit for -c mode */
			    maxIOerror: fprintf(stderr,
				"%s: -k %d exceeds %d for -c option\n",
				iC_progname, iC_maxIO-1, IXD-1);
			    goto error;
			}
		    } else {
			fprintf(stderr,
			    "%s: cannot use both -o and -c option\n", iC_progname);
			goto error;
		    }
		    /* fall through -c implies -A */
		case 'A':
		    iC_Aflag = iC_Arestore = 1;	/* generate ARITH ALIAS in outFN */
		    break;
		case 'S':
		    iC_Sflag = iC_Srestore = 1;	/* strict - all imm variables must be declared */
		    break;
		case 'R':
		    iC_maxErrCount = INT_MAX;	/* maximum error count very high */
		    break;
		case 'a':
		    iC_aflag = 1;		/* append for compile */
		    break;
		case 'P':
		    iC_Pflag++;			/* -P is pedantic, -PP or more is pedantic-error*/
		    break;
		case 'W':
		    if (strcmp(*argv, "Wno-deprecated") == 0) {
			iC_Wflag &= ~(W_DEPRECATED_LOGIC);	/* or all deprecated flags */
		    } else if (strcmp(*argv, "Wdeprecated") == 0) {
			iC_Wflag |= (W_DEPRECATED_LOGIC);	/* or all deprecated flags */
		    } else if (strcmp(*argv, "Wno-deprecated-logic") == 0) {
			iC_Wflag &= ~W_DEPRECATED_LOGIC;
		    } else if (strcmp(*argv, "Wdeprecated-logic") == 0) {
			iC_Wflag |= W_DEPRECATED_LOGIC;
		    /* Insert other Waning switches here */
		    } else {
			goto cerror;
		    }
		    goto break2;
		case 'g':
		    iC_gflag = 1;		/* independent C code for gdb debugging */
		    break;	/* allows setting breakpoints in C code in iC listings */
		case 'O':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (sscanf(*argv, "%o%s", &debi, tempBuf) != 1 || debi > 07) {
			fprintf(stderr,
			    "%s: -O levels can only be 0 - 7 (7 is default)\n", iC_progname);
			goto error;
		    }
		    iC_optimise = debi;		/* short */
		    goto break2;
		case 'C':
		    arg = 'D';
		    goto buildCdefines;
		case 'V':
		    arg = 'U';
		  buildCdefines:
		    mp = iC_Cdefines;
		    op = iC_defines;
		    goto buildDefines;
		case 'D':
		case 'U':
		    mp = iC_defines;
		    op = iC_Cdefines;
		    arg = **argv;
		  buildDefines:
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (strlen(*argv)) {
			if (strlen(op)) {
			    strncpy(tempBuf, op, BUFS);
			    cp = strtok(tempBuf+1, " ");	/* miss first space */
			    while (cp) {
				if ((cp = strtok(NULL, " ")) == NULL) break;	/* next token */
				if (strcmp(*argv, cp) == 0) {
				    fprintf(stderr, "%s: must not use same macro '%s' for C and iC\n",
					iC_progname, *argv);
				    goto error;
				}
				cp = strtok(NULL, " ");	/* -D -C -U or -V */
			    }
			}
			slen = strlen(mp);	/* space used so far */
			if (snprintf(mp + slen, BUFS - slen, " -%c %s", arg, *argv) >= BUFS - slen) {
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
		cerror:
		default:
		    fprintf(stderr,
			"%s: unknown command line switch '%s'\n", iC_progname, *argv);
		case 'h':
		case '?':
		error:
		    fprintf(stderr, usage, iC_progname, IXD-1, IXD-1,
#if defined(RUN) || defined(TCP)
#ifdef	TCP
		    iC_hostNM, iC_portNM, iC_iidNM, INSTSIZE,
#endif	/* TCP */
		    MARKMAX, iC_progname, iC_progname, iC_progname,
#endif	/* RUN or TCP */
		    iC_ID);
		    exit(1);
		}
	    } while (*++*argv);
	    break2: ;
	} else {
#ifdef	WIN32
	    if (strlen(*argv)) {
		inpFN = iC_emalloc(strlen(*argv)+1);	/* +1 for '\0' */
		strcpy(inpFN, *argv);
		while ((cp = strchr(inpFN, '\\')) != 0) {
		    *cp = '/';		/* convert '\' to '/' under WIN32 */
		}
	    }
#else	/* not WIN32 */
	    if (strlen(*argv)) inpFN = *argv;
#endif	/* WIN32 */
#ifdef	TCP
	    if (strcmp(iC_iccNM, "stdin") == 0) {
		iC_iccNM = iC_emalloc(strlen(inpFN)+1);	/* +1 for '\0' */
		strcpy(iC_iccNM, inpFN);
		if ((cp = strrchr(iC_iccNM, '.')) != 0 &&
		    cp[1] == 'i' &&
		    cp[2] == 'c' &&
		    cp[3] == '\0') {
		    *cp = '\0';		/* terminate at trailing extension .ic */
		}
	    }
	    if (strlen(iC_iidNM) > 0) {
		snprintf(iC_iccNM + strlen(iC_iccNM), INSTSIZE+2, "-%s", iC_iidNM);
	    }
#endif	/* TCP */
	}
    }
#if defined(RUN)
    if (outFN == 0 && (iC_maxIO < 0 || iC_maxIO > IXD)) {	/* I/O limit for run mode */
	fprintf(stderr, "%s: -k %d exceeds %d for run mode\n", iC_progname, iC_maxIO-1, IXD-1);
	goto error;
    }
#endif	/* RUN */
    iC_debug &= 037777;			/* allow only cases specified */
    iC_genCount = ((iC_optimise & 04) != 0) ? 1 : GEN_COUNT;
    iFlag = 0;
    /********************************************************************
     *	Generate and open temporary files T1FN T2FN T3FN
     *	T0FN, T4FN and T5FN are only used if iC and/or C-include files
     *	are actually parsed
     *******************************************************************/
    szNames[T0index] = T0FN;
    szNames[T1index] = T1FN;
    szNames[T2index] = T2FN;
    szNames[T3index] = T3FN;
    szNames[T4index] = T4FN;
    szNames[T5index] = T5FN;
    szNames[T6index] = T6FN;
    if ((fd = mkstemp(T1FN)) < 0 || (T1FP = fdopen(fd, "w+")) == 0) {
	r = T1index;			/* error opening temporary file */
    } else if ((fd = mkstemp(T2FN)) < 0 || (T2FP = fdopen(fd, "w+")) == 0) {
	r = T2index;			/* error opening temporary file */
    } else if ((fd = mkstemp(T3FN)) < 0 || (T3FP = fdopen(fd, "w+")) == 0) {
	r = T3index;			/* error opening temporary file */
    } else if ((fd = mkstemp(T6FN)) < 0 || close(fd) < 0 || unlink(T6FN) < 0) {
	r = T6index;			/* error making temporary name */
    } else
    /********************************************************************
     *	Call the iC compiler which builds a Symbol table and a logic
     *	net made up of the same Symbol structures linked by List_e's
     *
     *	As a side effect, all C-code fragments made up of literal blocks
     *	and C-actions are collected in the temporary file T1FN
     *	Also as a side effect outFlag is set if outFN is set (-o option)
     *******************************************************************/
    if ((r = setjmp(beginMain)) ||
	(r = compile(inpFN, listFN, errFN, outFN)) != 0) {
	ro = 5;				/* compile error */
    } else
    /********************************************************************
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
     *******************************************************************/
    if ((r = c_compile(T1FP, T3FP, C_PARSE|C_FIRST|C_BLOCK, 0)) != 0) {
	ro = 6;				/* C-compile error */
    } else {
#if defined(RUN) || defined(TCP)
	Gate *		igp;
#endif	/* RUN or TCP */
	unsigned	gate_count[MAX_LS];	/* used in listNet and buildNet only */
	if (inpFN) {
	    strncpy(inpNM, inpFN, BUFS);	/* restore name if last #line was not inpFN */
	}
	/********************************************************************
	 *	List network topology and statistics - this completes listing
	 *******************************************************************/
	if ((r = listNet(gate_count)) == 0) {
	    /********************************************************************
	     *	-o option: Output a C-file of all Gates, C-code and links
	     *******************************************************************/
	    if (outFlag) {			/* -o option */
		r = outNet(T3FP, outFN);	/* generate network as C file */
	    } else
	    /********************************************************************
	     *	-c option: Output a C-file cexe.c to rebuild compiler with C-code
	     *******************************************************************/
	    if (excFN) {			/* -c option */
		if ((excFP = fopen(excFN, "w")) == NULL) {
		    r = COindex;
		} else {
		    unsigned	linecnt = 1;

		    /* write C execution file Part 1 */
		    fprintf(excFP, cexe_part1);
		    linecnt += cexe_lines1;
		    /* copy literal blocks from C intermediate file to C output file */
		    copyXlate(T3FP, excFP, excFN, &linecnt, 01);
		    /* write C execution file Part 2 */
		    fprintf(excFP, cexe_part2);
		    linecnt += cexe_lines2;
		    /* copy called function cases from C intermediate file to C output file */
		    copyXlate(T3FP, excFP, excFN, &linecnt, 02);
		    /* write C execution file Part 3 */
		    fprintf(excFP, cexe_part3, iC_progname, iC_gflag ? "g" : "", iC_optimise, inpNM);
		    linecnt += cexe_lines3;
		    if (linecnt > (1 + cexe_lines1 + cexe_lines2 + cexe_lines3)) {
			fprintf(excFP, cexe_part4,
			    inpNM, iC_ID, iC_gflag ? "g" : "", iC_optimise);
			linecnt += cexe_lines4;
			if (iC_debug & 010) {
			    fprintf(iC_outFP, "\nC OUTPUT: %s  (%d lines)\n", excFN, linecnt-1);
			}
		    }
		    fclose(excFP);
		}
#if defined(RUN) || defined(TCP)
	    } else
	    /********************************************************************
	     *	Build a network of Gates and links for direct execution
	     *******************************************************************/
	    if ((r = buildNet(&igp, gate_count)) == 0) {
		Symbol * sp = lookup("iClock");
		unlinkTfiles();
		/********************************************************************
		 *	Execute the compiled iC logic directly
		 *******************************************************************/
		assert (sp);			/* iClock initialized in init() */
		iC_c_list = sp->u_gate;		/* initialise clock list */
		iC_icc(igp, gate_count);	/* execute the compiled logic */
		/********************************************************************
		 * never returns - exits via iC_quit()
		 *******************************************************************/
#endif	/* RUN or TCP */
	    }
	}
	/********************************************************************
	 *	iC and C compilation, listing and generated C output complete
	 *******************************************************************/
	if (r != 0) {
	    ro = 7;				/* error in output */
	    fprintf(stderr, OutputMessage[r < 4 ? r : 7], iC_progname, szNames[r]);
	    r += 10;
	    goto closeFiles;
	}
    }
    if (r != 0) {
	fprintf(stderr, OutputMessage[ro], iC_progname, szNames[r]);
    }
closeFiles: ;
    if (iC_outFP != stdout) {
	fflush(iC_outFP);
	fclose(iC_outFP);
	if (listFN && iFlag) {
	    if (iC_inversionCorrection() != 0) {
		r += 100;
	    }
	    iFlag = 0;
	}
    }
    if (iC_errFP != stderr) {
	fflush(iC_errFP);
	fclose(iC_errFP);
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
	if (!(iC_debug & 04000)) {
	    unlink(T1FN);
	}
    }
    if (T2FP) {
	fclose(T2FP);
	if (!(iC_debug & 04000)) {
	    unlink(T2FN);
	}
    }
    if (T3FP) {
	fclose(T3FP);
	if (!(iC_debug & 04000)) {
	    unlink(T3FN);
	}
    }
    if (T4FP) {
	fclose(T4FP);
	T4FP = 0;
	if (!(iC_debug & 04000)) {
	    unlink(T4FN);
	}
    }
    if (T5FP) {
	fclose(T5FP);
	if (!(iC_debug & 04000)) {
	    unlink(T5FN);
	}
    }
#ifdef EFENCE
    free(inpNM);			/* clean up for purify */
    free(iCbuf);
    free(iFunBuffer);
#endif	/* EFENCE */
    free(functionUse);
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
 *	When iC_path is not "", pplstfix fill be called with the log
 *	option -l, which outputs to pplstfix.log. In this case
 *	when iC_progname is not just "immcc" but ends with a revision
 *	number, eg. immcc109, pplstfix will be called as pplstfix109.
 *
 *	pplstfix is called if logic expansion and net topology is
 *	specified -d24. The latter defines the alias equivalences.
 *
 *	pplstfix is suppressed if yacc output -d1, logic generation -d2
 *	or pplstfix suppression -d4000 is specified. Particularly the
 *	logic generation is a very detailed listing, and the listing
 *	should be in the sequence the lexer and parser generate the code.
 *	pplstfix sorts and re-arranges this sequence and changes names
 *	to resolve aliases, which is good for the final listing, but
 *	not for the logic generation or yacc listing.
 *
 *******************************************************************/

int
iC_inversionCorrection(void)
{
    int		r = 0;

    if ((iC_debug & 04027) == 024) {	/* required: logic expansion and net topology */
	int	fd;
	char *	versionTail;
	char	tempName[] = "pplstfix.XXXXXX";
	char	exStr[TSIZE];

	/* mktemp() is deemed dangerous */
	/* Cygnus won't unlink till file is closed */
	if ((fd = mkstemp(tempName)) < 0 ||
	    close(fd) < 0 ||
	    unlink(tempName) < 0 ||
	    rename(listFN, tempName) < 0 /* inversion correction needed */
	) {
	    fprintf(stderr, "%s: rename(%s, %s) failed\n",
		iC_progname, listFN, tempName);
	} else {
	    if (strlen(iC_path) == 0) {
		snprintf(exStr, TSIZE, "pplstfix -o %s %s", listFN, tempName);	/* on $PATH */
	    } else {
		versionTail = iC_progname + strcspn(iC_progname, "0123456789");
		snprintf(exStr, TSIZE, "perl \"%s\"/pplstfix%s -o %s -l pplstfix%s.log %s",	/* developer debug version */
		    iC_path, versionTail, listFN, versionTail, tempName);
	    }
	    r = system(exStr);
	    if (r == 0) {
		unlink(tempName);
	    } else {
		perror("pplstfix");
		fprintf(stderr, "%s: system(\"%s\") could not be executed $? = %d\n",
		    iC_progname, exStr, r);
	    }
	}
    }
    return r;
} /* inversionCorrection */

/* ############ POD to generate man page ##################################

=head1 NAME

 immcc - the immediate-C to C compiler

=head1 SYNOPSIS

 immcc [-acgASRh][ -o<out>][ -l<lst>][ -e<err>][ -k<lim>][ -d<deb>]
       [ -O<level>][ -Dmacro[=defn]...][ -Umacro...]
       [ -Cmacro[=defn]...][ -Vmacro...] <src.ic>
    -o <out> name of generated C output file
    -c       generate C source cexe.c to extend 'icr' or 'ict' compiler
             (cannot be used if also compiling with -o)
    -l <lst> name of list file  (default: none, '' is stdout) output:
             listing with logic expansion, net topology and statistics
    -e <err> name of error file (default is stderr)
    -a       compile with linking information in auxiliary files
    -g       each expression has its own C code for debugging with gdb
    -A       compile output ARITHMETIC ALIAS nodes for symbol debugging
    -S       strict compile - all immediate variables must be declared
    -P       pedantic: warning if variable contains $ (default $ allowed)
    -PP      pedantic-error: error if variable contains $
    -R       no maximum error count (default: abort after 100 errors)
    -O <level> optimisation -O0 none -O1 bit -O2 arithmetic -O4 eliminate
             duplicate arithmetic expressions; -O7 all (default)\n"
    -D <macro> predefine <macro> for the iC preprocessor phase
    -U <macro> cancel previous definition of <macro> for the iC phase
            Note: do not use the same macros for the iC and the C phase
    -C <macro> predefine <macro> for the C preprocessor phase
    -V <macro> cancel previous definition of <macro> for the C phase
    -k <lim> highest I/O index (default: no limit; 63 for -c mode)
             if lim <= 63, mixed byte, word and long indices are tested
             default: any index for bit, byte, word or long is allowed
    -d <deb>                  LIST options
        +40  source listing
        +20  net topology
        +10  net statistics
         +4  logic expansion
       +204  numerical values prepared for iClive display
                              DEBUG options
         +2  logic generation                     (+4002 Symbol Table)
         +1  iC bison iC_debug info (on stderr only) (+4001 C bison)
      +4000  supress listing alias post processor (save temp files)
     +10000  generate listing of compiler internal functions
     +20000  use old style imm functions - no internal functions
     +21000  outputs first (really old style) - no internal functions

    <src.ic> iC language source file (extension .ic)
    -        or default: take iC source from stdin
    -h       this help text

=head1 DESCRIPTION

B<immcc> translates an iC-source (extension: .ic) with iC-includes
(extension: .ih) into a C file which can be compiled with a C
compiler and linked with the iC project run time library B<libict.a>
(link option -lict) to produce an executable which communicates via
TCP/IP with B<iCserver> and associated I/O-clients - usually B<iCbox>.

B<immcc> reads and translates one iC-source eg file.ic. If no options are
specified, only compilation errors (if any) are reported on 'stderr'.

Before translation starts, file.ic is optionally passed through the
C-preprocessor if it contains any lines starting with '#' or if -D or -U
options were used in the command line.

Immediate-C code consists of immediate statements and C literal blocks.
Immediate statements translate into data tables for use at run time
and C-actions generated by the immediate B<if>, B<else> and B<switch>
statements. C literal blocks are C code enclosed in B<%{> and B<%}>
braces.

C code in C-actions and literal blocks is copied nearly verbatim into the
target C file. Any C-preprocessor statements for the C-code in C-actions
and literal blocks must have a B<%> immediately preceding the leading
B<#>, eg B<%#include "f.h">. The B<'%'> is stripped before being copied
to the target.

The generated C-code is re-grouped into literal blocks first, followed
by C-actions. This is necessary to place all C-declarations in literal
blocks before any C-actions, which may require them.

In a next pass an integrated special-purpose C compiler is run
(preceded by a pass through the C-preprocessor, if necessary). The C
compiler recognizes any immediate variables in the embedded C code.
Immediate variables are declared with B<imm bit> or B<imm int>
in the iC code section.  These variables can be used as values
anywhere in the C-code and appropriate modification of the C code is
carried out. Immediate variables which have been declared but not yet
assigned may be (multiple) assigned to in the C-code.  Such assignment
expressions are recognised and converted to function calls, which fire
all immediate expressions dependent in the immediate variable.  If the
B<strict> option (-S) is used for compilation, immediate variables
to be assigned in C code must be declared with the type modifier
B<immC> rather than with B<imm>.  This is particularly important if
an immediate variable assigned to in C code is going to be used in
another iC source.  If an immediate variable has already been (single)
assigned to in an immediate statement, an attempt to assign to it in
the C-code is an error. A syntax check of the embedded C code with
appropriate error messages is a by-product of this procedure.

The iC and C compiler also produces an optional listing (default file.lst).
The listing displays the iC source lines with line numbers. These
are interspersed with a detailed logic expansion of the generated
code produced by the compiler. Study of this code gives insights
into how iC-code is compiled and executed.  Aliases appear in the
raw listing because of one pass compiler limitations. Aliases are
completely eliminated from the executable code, and should therefore
not confuse readers of the listings. A correction is carried out by
passing the listing - and only the listing through the perlscript
'pplstfix', which resolves aliases in the listing of a single source
file. Aliases will still occurr for iC-applications spanning several
source files. Here the reader must resolve any aliases.

=head1 DEVELOPER INFORMATION

For debugging the compiler and 'pplstfix', if the compiler is called
with a path, eg. './immcc', 'pplstfix' will be called with the same
path, namely './pplstfix'.  If a path is specified and the name of
the compiler is not just 'immcc' but ends with a revision number,
eg. 'immcc109', 'pplstfix' will be called as 'pplstfix109'. This
way several versions of the compiler and pplstfix can be kept in
the development directory.  Also if a path is specified 'pplstfix'
will be called with the log option -l. This outputs a log file
to './pplstfix109.log', if the compiler call was './immcc109'.
Finally the script iCmake will link to ./libict109.a rather than to
the standard libict.a

=head1 AUTHOR

John E. Wulff

=head1 BUGS

Email bug reports to B<john@je-wulff.de> with L<iC Project> in the
subject field.

=head1 SEE ALSO

L<iCmake(1)>, L<makeAll(1)>, L<iCserver(1)>, L<iCbox(1)>

=head1 COPYRIGHT

COPYRIGHT (C) 2000-2005  John E. Wulff

You may distribute under the terms of either the GNU General Public
License or the Artistic License, as specified in the README file.

For more information about this program, or for information on how
to contact the author, see the README file.

=cut

   ############ end of POD to generate man page ########################### */

static const char icc_c[] =
"@(#)$Id: icc.c 1.75 $";
/********************************************************************
 *
 *	Copyright (C) 1985-2012  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	main.c
 *	command line interpretation and starter for immcc compiler
 *
 *******************************************************************/

#ifdef	LOAD
#error - must be compiled without LOAD defined to make an executable compiler
#endif	/* LOAD */

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/stat.h>
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
#if defined(RUN) || defined(TCP)
#include	"cexe.h"
#endif	/* RUN or TCP */

extern const char	iC_ID[];
extern const char	iC_PATCH[];
unsigned short		iC_gflag = 0;		/* -g independent C code for gdb debugging */

unsigned int		iC_uses = 0;		/* 01=alias 02=strict as bit field */
unsigned int		iC_useStack[USESTACKSZ];
unsigned int		iC_useStackIndex = 0;

unsigned short		iC_optimise = 07;	/* optimisation levels 0 - 7 */

static const char *	usage =
"Usage:\n"
" %s [-"
#if defined(RUN) || defined(TCP)
"c"
#endif	/* RUN or TCP */
"agASpR"
#if !defined(RUN) && !defined(TCP)
"v"
#endif	/* not RUN and not TCP */
"Ph][ -o<out>][ -l<lst>][ -e<err>][ -k<lim>][ -d<deb>]\n"
"       [ -O<level>][ -Dmacro[=defn]...][ -Umacro...]\n"
"       [ -Cmacro[=defn]...][ -Vmacro...][ -W[no-]<warn>...][ <src.ic>]\n"
#if defined(RUN) || defined(TCP)
"       [ --[ -h] ...]"
#ifdef	TCP
"|[ -R <app ...>]"
#endif	/* TCP */
"\n"
"Options in compile mode (-c or -o):\n"
"        -c              generate C source cexe.c to extend 'icr' or 'ict' compiler\n"
"                        (cannot be used if also compiling with -o)\n"
#endif	/* RUN or TCP */
"        -o <out>        name of generated C output file\n"
"        -l <lst>        name of list file  (default: none, '' is stdout) output:\n"
"                        listing with logic expansion, net topology and statistics\n"
"        -e <err>        name of error file (default is stderr)\n"
"        -a              compile with linking information in auxiliary files\n"
"        -g              each expression has its own C code for debugging with gdb\n"
"        -A              compile output ARITHMETIC ALIAS nodes for symbol debugging\n"
"        -S              strict compile - all immediate variables must be declared\n"
"        -p              pedantic: warning if variable contains $ (default $ allowed)\n"
"        -pp             pedantic-error: error if variable contains $\n"
"        -R              no maximum error count (default: abort after 100 errors)\n"
"        -O <level>      optimisation -O0 none -O1 bit -O2 arithmetic -O4 eliminate \n"
"                        duplicate arithmetic expressions; -O7 all (default)\n"
"        -D <macro>      predefine <macro> for the iC preprocessor phase\n"
"        -U <macro>      cancel previous definition of <macro> for the iC phase\n"
"                        Note: do not use the same macros for the iC and the C phase\n"
"        -C <macro>      predefine <macro> for the C preprocessor phase\n"
"        -V <macro>      cancel previous definition of <macro> for the C phase\n"
"        -W[no-]<warn>                  positive/negative warning options\n"
"            -W[no-]deprecated-logic    use of && || ! in pure bit expressions\n"
"            -W[no-]function-parameter  unused parameters in functions\n"
"            -W[no-]function-delete     delete before function re-definition\n"
"            -W[no-]deprecated          [none]/all of the above\n"
"            -W[no-]undefined-gate      undefined gate\n"
"            -W[no-]unused-gate         unused gate\n"
"            -W[no-]all                 [no]/all warnings\n"
#if defined(RUN) || defined(TCP)
"        -k <lim>        highest I/O index (default: %d; also run and -c mode limit)\n"
"                        if lim <= %d, mixed byte, word and long indices are tested\n"
#else	/* not RUN or TCP */
"        -k <lim>        highest I/O index (default: no limit)\n"
"                        if lim <= %d, mixed byte, word and long indices are tested\n"
"                        default: any index for bit, byte, word or long is allowed\n"
#endif	/* RUN or TCP */
"        -d <deb>      LIST options\n"
"                   +40  source listing\n"
"                   +20  net topology\n"
"                   +10  net statistics\n"
"                    +4  logic expansion\n"
"                  +204  numerical values prepared for iClive display\n"
#if defined(RUN) || defined(TCP)
"                  +400  exit after initialisation\n"
#endif	/* RUN or TCP */
#if YYDEBUG && !defined(_WINDOWS)
"                      DEBUG options\n"
#if defined(RUN) || defined(TCP)
"                  +402  logic generation    (exits after initialisation)\n"
#ifdef YACC
"                  +401  iC yacc debug info  (on stdout only) (+4401 C yacc info)\n"
#else	/* BISON */
"                  +401  iC bison debug info (on stderr only) (+4401 C bison info)\n"
#endif	/* YACC */
#else	/* not RUN and not TCP */
"                    +2  logic generation                     (+4002 Symbol Table)\n"
#ifdef YACC
"                    +1  iC yacc debug info  (on stdout only) (+4001 C yacc info)\n"
#else	/* BISON */
"                    +1  iC bison debug info (on stderr only) (+4001 C bison info)\n"
#endif	/* YACC */
#endif	/* RUN or TCP */
"                 +4000  supress listing alias post processor (save temp files)\n"
#ifdef BOOT_COMPILE
"                +20000  generate pre-compiled function blocks for init.c\n"
#endif	/* BOOT_COMPILE */
#endif	/* YYDEBUG and not _WINDOWS */
"        <src.ic>        iC language source file (extension .ic)\n"
"                        default: take iC source from stdin\n"
#if !defined(RUN) && !defined(TCP)
"        -v              version\n"
#endif	/* not RUN and not TCP */
"        -P              GIT patch if made with dirty version\n"
"        -h              this help text\n"
#ifdef EFENCE
"        -E              test Electric Fence ABOVE - SIGSEGV signal unless\n"
"        -B              export EF_PROTECT_BELOW=1 which tests access BELOW \n"
"        -F              export EF_PROTECT_FREE=1 which tests access FREE\n"
#endif	/* EFENCE */
#if defined(RUN) || defined(TCP)
"Extra options for run mode: (direct interpretation)\n"
" [-"
#ifdef	TCP
"l"
#endif	/* TCP */
#if YYDEBUG && !defined(_WINDOWS)
"t"
#endif	/* YYDEBUG and not _WINDOWS */
#ifdef RUN
"x"
#endif	/* RUN */
#ifdef TCP
"m[m]"
#endif	/* TCP */
"]"
"[ -n<count>"
#ifdef TCP
"][ -s <host>][ -p <port>][ -u <unitID>][ -i <instanceID>]\n"
" [ -v <file.vcd>"
#endif	/* TCP */
"]\n"
"        -n <count>      maximum oscillator count (default is %d, limit 15)\n"
"                        0 allows unlimited oscillations\n"
#ifdef TCP
"        -l              start 'iClive' with correct source\n"
"        -s host         IP address of iCserver           (default '%s')\n"
"        -p port         service port of iCserver (not p) (default '%s')\n"
"        -u unitID       of this client (default base name of <src.ic>)\n"
"        -i instanceID   of this client (default '%s'; 1 to %d numeric digits)\n"
"        -v <file.vcd>   output a .vcd and a .sav file for gtkwave\n"
#endif	/* TCP */
#if YYDEBUG && !defined(_WINDOWS)
"        -d <debug>2000  display scan_cnt and link_cnt\n"
"                 +1000  do not trace non-active timers TX0.n\n"
"                  +200  display oscillator info\n"
"                  +100  initialisation and run time trace\n"
#ifdef TCP
"                    +4  extra install debug info\n"
"                    +2  trace I/O receive buffer\n"
"                    +1  trace I/O send buffer\n"
#endif	/* TCP */
#endif	/* YYDEBUG and not _WINDOWS */
#if YYDEBUG && !defined(_WINDOWS)
"        -t              trace gate activity (equivalent to -d 1100)\n"
"                        can be toggled at run time by typing t\n"
#endif	/* YYDEBUG and not _WINDOWS */
#ifdef RUN
"        -x              arithmetic info in hexadecimal (default decimal)\n"
"                        can be changed at run time by typing x or d\n"
#endif	/* RUN */
#ifdef TCP
"        -m              microsecond timing info\n"
"        -mm             more microsecond timing (internal time base)\n"
"                        can be toggled at run time by typing m\n"
"        -q      quiet - do not report clients connecting and disconnecting\n"
"        -z      block keyboard input on this app - used by -R\n"
#endif	/* TCP */
"                      EXTRA arguments\n"
"        --      any further arguments after -- are passed to the app\n"
"        --h     help with command line options particular to this app\n"
#ifdef	TCP
"                      AUXILIARY app\n"
"        -R <app ...> run auxiliary app followed by -z and its arguments\n"
"                     as a separate process; -R ... must be last arguments.\n"
#endif	/* TCP */
"      A <src.ic> containing only logical expressions can be interpreted\n"
"      with  %s -t <src.ic>. A <src.ic> containing arithmetic expressions\n"
"      requires relinking of %s with a new cexe.c generated by executing\n"
#ifdef TCP
"      %s -c <src.ic>; makeAll -T; before <src.ic> can be interpreted.\n"
#endif	/* not TCP */
#ifdef RUN
"      %s -c <src.ic>; makeAll -R; before <src.ic> can be interpreted.\n"
"\n"
"      Typing 0 to 7 toggles simulated inputs IX0.0 to IX0.7\n"
"      Normally 0 to 7 acts immediately. Preceding it with +\n"
"      allows more simultaneous logic inputs - one for each +\n"
"      Typing b<num> w<num> or l<num> alters simulated inputs IB1, IW2 or IL4\n"
"              <num> may be decimal 255, octal 0177 or hexadecimal 0xff\n"
"      Typing h provides a short help for simulated inputs\n"
"\n"
"      Programmed outputs QX0.0 to QX0.7, QB1, QB2 and QL4 are displayed.\n"
#endif	/* RUN */
"      Typing q or ctrl+D quits run mode.\n"
#endif	/* RUN or TCP */
"%s\n"
"Copyright (C) 1985-2015 John E. Wulff     <immediateC@gmail.com>\n"
;

char *		iC_progname;		/* name of this executable */
char *		iC_vcd = NULL;
short		iC_debug = 0;
int		iC_micro = 0;
int		iC_Pflag = 0;		/* pedantic warning/error flag */
int		iC_Wflag = W_ALL;	/* by default all warnings are on */
unsigned short	iC_xflag;
unsigned short	iFlag;
unsigned short	iC_osc_max = 0;		/* 0 during Initialisation, when no oscillations */
unsigned short	iC_osc_lim = MARKMAX;
unsigned short	iC_osc_flag = 0;
int		iC_argc;		/* extra options passed to iCbegin(int argc, char** argv) */
char **		iC_argv;
int		iC_argh = 0;		/* block running iCserver before iCbegin() */
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

#if defined(RUN) || defined(TCP)
static FILE *	excFP;			/* cexe C out file pointer */
#endif /* defined(RUN) || defined(TCP) */
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
int		iC_maxIO = IXD;		/* I/O index limited to 64 */
#else	/*  not RUN */
int		iC_maxIO = -1;		/* I/O index no limit */
#endif	/*  not RUN */

#if defined(RUN) || defined(TCP)
Gate *		iC_pf0_1;		/* pointer to _f0_1 if generated in buildNet() */
#endif /* defined(RUN) || defined(TCP) */

unsigned char	iC_QX_[IXD];		/* Output bit field slots (also used in compile) */

#if defined(TCP)
Gate *		iC_TX0p;		/* pointer to bit System byte Gates */
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
iC_openT4T5(int mode)
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
} /* iC_openT4T5 */

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
    int		r = 0;			/* return value of compile */
    int		ro = 4;			/* output message index */
    int		b = 0;			/* aux flag */
    char *	cp;
#ifdef	TCP
    char *	mqz = "-qz";
    char *	mz  = "-z";
#endif	/* TCP */

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
    writeCexeString(NULL, 0);		/* initialise function use database */
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

#if defined(RUN) || defined(TCP)
		case 'n':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) iC_osc_lim = atoi(*argv); else goto missing;
		    if (iC_osc_lim > 15) {
			fprintf(iC_errFP, "ERROR: %s: -n %hu value > 15\n", iC_progname, iC_osc_lim);
			goto error;
		    }
		    goto break2;
#ifdef TCP
		case 's':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) iC_hostNM = *argv; else goto missing;
		    goto break2;
		case 'p':
		    if (! *++*argv) { --argc; if(! *++argv) goto Pedantic; }
		    if (strlen(*argv)) {	/* simply -p */
			if (**argv != '-' && strchr(*argv, '.') == 0) {
			    if (strcmp(*argv, "p") != 0) {
				iC_portNM = *argv;
				goto break2;	/* port number or name (not "p" or <in>.ic) */
			    }
			    iC_Pflag++;		/* -pp or -p p is pedantic-error*/
			} else {
			    ++argc; --argv;	/* push back -x option or <in>.ic after -p */
			  Pedantic:
			    iC_Pflag++;
			    iC_Wflag |= W_ALL;	/* by default all warnings are on */
			    goto break2;
			}
		    }
		    iC_Pflag++;			/* -p is pedantic, -p -p or more is pedantic-error*/
		    iC_Wflag |= W_ALL;
		    break;
		case 'u':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) {
			if (strcmp(iC_iccNM, "stdin") == 0) {
			    iC_iccNM = iC_emalloc(strlen(*argv)+INSTSIZE+2);	/* +2 for '-...\0' */
			    strcpy(iC_iccNM, *argv);
			}
		    } else {
			goto missing;
		    }
		    goto break2;
		case 'i':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if ((slen = strlen(*argv)) > INSTSIZE ||
			slen != strspn(*argv, "0123456789")) {
			fprintf(iC_errFP, "WARNING '-i %s' is non numeric or longer than %d characters - ignored\n",
			    *argv, INSTSIZE);
		    } else if (*iC_iidNM != '\0') {
			fprintf(iC_errFP, "WARNING '-i %s' called a second time - ignored\n", *argv);
		    } else {
			iC_iidNM = *argv;
		    }
		    goto break2;
		case 'v':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) iC_vcd = *argv; else goto missing;
		    goto break2;	/* output vcd dump file for gtkwave */
		case 'm':
		    iC_micro++;		/* microsecond info */
		    break;
#endif	/* TCP */
#if YYDEBUG
		case 't':
		    iC_debug |= 01100;	/* trace gate activity */
		    break;
#endif	/* YYDEBUG */
#ifdef RUN
		case 'x':
		    iC_xflag = 1;	/* start with hexadecimal display */
		    break;
#endif	/* RUN */
#endif	/* RUN or TCP */
		case 'd':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (!(slen = strlen(*argv))) goto missing;
		    if (slen != strspn(*argv, "01234567") ||	/* octal digits only */
			sscanf(*argv, "%o", &debi) != 1 ||
			(debi & (DZ | DQ | ~0xffff))
		    ) {
			fprintf(iC_errFP, "ERROR: %s: '-d %s' is not a well formed octal string or exceeds range 37777\n",
			    iC_progname, *argv);
			exit(1);
		    }
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
#if defined(RUN) || defined(TCP)
		    if (excFN == 0) {
#endif /* defined(RUN) || defined(TCP) */
			if (! *++*argv) { --argc; if(! *++argv) goto missing; }
			if (strlen(*argv)) {
#ifdef	WIN32
			    outFN = iC_emalloc(strlen(*argv)+1);	/* +1 for '\0' */
			    strcpy(outFN, *argv);
			    while ((cp = strchr(outFN, '\\')) != 0) {
				*cp = '/';		/* convert '\' to '/' under WIN32 */
			    }
#else	/* not WIN32 */
			    outFN = *argv;		/* compiler output file name */
#endif	/* WIN32 */
			} else goto missing;
			goto break2;
#if defined(RUN) || defined(TCP)
		    } else {
			fprintf(iC_errFP,
			    "%s: cannot use both -c and -o option\n", iC_progname);
			goto error;
		    }
#endif /* defined(RUN) || defined(TCP) */
		case 'l':
		    if (! *++*argv) {			/* step past 'l' */
			--argc;
			b = 1;				/* step to next argv */
			if(! *++argv) goto tryLive;
		    }
		    if (strlen(*argv) && (cp = strrchr(*argv, '.')) != 0) {
			if (memcmp(cp, ".lst", 3) == 0) {	/* accept .lst6 etc */
    #ifdef	WIN32
			    listFN = iC_emalloc(strlen(*argv)+1);	/* +1 for '\0' */
			    strcpy(listFN, *argv);
			    while ((cp = strchr(listFN, '\\')) != 0) {
				*cp = '/';		/* convert '\' to '/' under WIN32 */
			    }
    #else	/* not WIN32 */
			    listFN = *argv;		/* listing file name */
    #endif	/* WIN32 */
			} else if (strcmp(cp, ".ic") == 0) {
#ifdef	TCP
			    if (outFN == 0 && excFN == 0) {
				iC_opt_l = 1;		/* start iClive with correct source */
				goto setInpFN;		/* found input name */
			    }
#endif	/* TCP */
			    if (b == 0) ++argv;
			    goto missing;
			} else {
			    goto strangeListFN;
			}
		    } else if (**argv == '-') {
#ifdef	TCP
			if (outFN == 0 && excFN == 0) {
			    iC_opt_l = 1;		/* start iClive with correct source */
			    break;			/* found more -options */
			}
#endif	/* TCP */
			if (b == 0) ++argv;
			goto missing;
		    } else if (b) {
		      strangeListFN:
			fprintf(iC_errFP, "%s: WARNING: invalid list file -l '%s' - ignore\n", iC_progname, *argv);
			goto break2;
		    } else {
			--argc;
			++argv;
		      tryLive:
			++argc;
			--*(--argv);
#ifdef	TCP
			if (outFN == 0 && excFN == 0) {
			    iC_opt_l = 1;		/* start iClive with correct source */
			    break;
			}
#endif	/* TCP */
			++*(argv++);
			goto missing;
		    }
		    if ((iC_debug & 077) == 0) {
			iC_debug |= 074;	/* default listing, topology, stats and logic expansion */
		    }
		    goto break2;
		case 'e':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) {
#ifdef	WIN32
			errFN = iC_emalloc(strlen(*argv)+1);	/* +1 for '\0' */
			strcpy(errFN, *argv);
			while ((cp = strchr(errFN, '\\')) != 0) {
			    *cp = '/';		/* convert '\' to '/' under WIN32 */
			}
#else	/* not WIN32 */
			errFN = *argv;		/* error file name */
#endif	/* WIN32 */
		    } else goto missing;
		    goto break2;
		case 'k':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) iC_maxIO = atoi(*argv) + 1; else goto missing;
		    if (excFN != 0 && (iC_maxIO < 0 || iC_maxIO > IXD)) {
#if defined(RUN) || defined(TCP)
		      maxIOerror:
#endif /* defined(RUN) || defined(TCP) */
			fprintf(iC_errFP,
			    "%s: -k %d exceeds %d for -c option\n",
			    iC_progname, iC_maxIO-1, IXD-1);
			goto error;
		    }
		    goto break2;
#if defined(RUN) || defined(TCP)
		case 'c':
		    if (outFN == 0) {
			excFN = "cexe.c";
			if (iC_maxIO < 0) iC_maxIO = IXD;	/* pre-set for immcc is -1 */
			if (iC_maxIO > IXD) goto maxIOerror;	/* I/O limit for -c mode */
		    } else {
			fprintf(iC_errFP,
			    "%s: cannot use both -o and -c option\n", iC_progname);
			goto error;
		    }
		    /* fall through -c implies -A */
#endif /* defined(RUN) || defined(TCP) */
		case 'A':
		    iC_uses |= USE_ALIAS;	/* generate ARITH ALIAS in outFN */
		    break;
		case 'S':
		    iC_uses |= USE_STRICT;	/* strict - all imm variables must be declared */
		    break;
		case 'a':
		    iC_aflag = 1;		/* append for compile */
		    break;
#ifndef TCP
		case 'R':			/* for TCP this option is interpreted with -R <call+opts> */
		    iC_maxErrCount = INT_MAX;	/* maximum error count very high */
		    break;
		case 'p':			/* for TCP this option is interpreted with -p <port> */
		    iC_Pflag++;			/* -p is pedantic, -pp or more is pedantic-error*/
		    iC_Wflag |= W_ALL;		/* by default all warnings are on */
		    break;
#endif	/* not TCP */
		case 'W':
		    if (! iC_Pflag) {
			if (strcmp(*argv, "Wall") == 0) {	/* default */
			    iC_Wflag |= W_ALL;
			} else if (strcmp(*argv, "Wno-all") == 0) {
			    iC_Wflag &= ~W_ALL;
			} else if (strcmp(*argv, "Wdeprecated") == 0) {
			    iC_Wflag |= (W_DEPRECATED_LOGIC | W_FUNCTION_PARAMETER | W_FUNCTION_DELETE);
			} else if (strcmp(*argv, "Wno-deprecated") == 0) {
			    iC_Wflag &= ~(W_DEPRECATED_LOGIC | W_FUNCTION_PARAMETER | W_FUNCTION_DELETE);
			} else if (strcmp(*argv, "Wdeprecated-logic") == 0) {
			    iC_Wflag |= W_DEPRECATED_LOGIC;
			} else if (strcmp(*argv, "Wno-deprecated-logic") == 0) {
			    iC_Wflag &= ~W_DEPRECATED_LOGIC;
			} else if (strcmp(*argv, "Wfunction-parameter") == 0) {
			    iC_Wflag |= W_FUNCTION_PARAMETER;
			} else if (strcmp(*argv, "Wno-function-parameter") == 0) {
			    iC_Wflag &= ~W_FUNCTION_PARAMETER;
			} else if (strcmp(*argv, "Wfunction-delete") == 0) {
			    iC_Wflag |= W_FUNCTION_DELETE;
			} else if (strcmp(*argv, "Wno-function-delete") == 0) {
			    iC_Wflag &= ~W_FUNCTION_DELETE;
			} else if (strcmp(*argv, "Wundefined-gate") == 0) {
			    iC_Wflag |= W_UNDEFINED;
			} else if (strcmp(*argv, "Wno-undefined-gate") == 0) {
			    iC_Wflag &= ~W_UNDEFINED;
			} else if (strcmp(*argv, "Wunused-gate") == 0) {
			    iC_Wflag |= W_UNUSED;
			} else if (strcmp(*argv, "Wno-unused-gate") == 0) {
			    iC_Wflag &= ~W_UNUSED;
			    /* Insert other Waning switches here */
			} else {
			    fprintf(iC_errFP, "WARNING: %s: unknown option -%s\n", iC_progname, *argv);
			}
		    }
		    goto break2;
		case 'g':
		    iC_gflag = 1;		/* independent C code for gdb debugging */
		    break;	/* allows setting breakpoints in C code in iC listings */
		case 'O':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (sscanf(*argv, "%o%s", &debi, tempBuf) != 1 || debi > 07) {
			fprintf(iC_errFP,
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
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) {
			// fprintf(iC_errFP, "*** *argv = %s iC_defines = '%s' iC_Cdefines = '%s'\n",
			//     *argv, iC_defines, iC_Cdefines);
			/********************************************************************
			 * copy -arg *argv to iC_defines or iC_Cdefines
			 *******************************************************************/
			slen = strlen(mp);			/* space used so far */
			if (snprintf(mp + slen, BUFS - slen, " -%c %s", arg, *argv) >= BUFS - slen) {
			    fprintf(iC_errFP, "%s: -%c %s: does not fit in internal buffer\n",
				iC_progname, arg, *argv);
			    goto error;
			}
			/********************************************************************
			 * check that token (without =part) is not in alternate iC_defines
			 *******************************************************************/
			if (strlen(op)) {
			    strncpy(tempBuf, op, BUFS);
			    slen = strlen(tempBuf);
			    if ((op = strpbrk(*argv, "=(")) != NULL) {
				// fprintf(iC_errFP, "*#* *argv = '%s' op = '%s'\n", *argv, op);
				*op = '\0';			/* terminate token on = or ) (no longer needed) */
			    }
			    cp = strtok(tempBuf+1, " ");	/* miss first -D -C -U or -V */
			    while (cp) {
				// fprintf(iC_errFP, "*1* cp = '%s'\n", cp);
				if ((cp = strtok(NULL, " ")) == NULL) break;	/* next token */
				// fprintf(iC_errFP, "*2* cp = '%s'\n", cp);
				if ((mp = strpbrk(cp, "=(")) != NULL) {
				    *mp = '\0';			/* terminate token on ( or = */
				    // fprintf(iC_errFP, "*3* cp = '%s'\n", cp);
				}
				if (strcmp(*argv, cp) == 0) {
				    fprintf(iC_errFP, "%s: must not use same macro '%s' for -D or -C\n",
					iC_progname, *argv);
				    goto error;
				}
				cp = strtok(NULL, " ");		/* -D -C -U or -V */
			    }
			}
		    } else goto missing;
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
#ifdef	TCP
		case 'q':
		    iC_debug |= DQ;	/* -q    quiet operation of all apps and iCserver */
		    break;
		case 'z':
		    iC_debug |= DZ;	/* -z    block all STDIN interrupts for this app */
		    break;
		case 'R':
		    /********************************************************************
		     *  Run auxiliary app with rest of command line
		     *  splice in the "-z" option to block STDIN interrupts in chained apps
		     *  alternatively "-qz" option for quiet operation and to block STDIN
		     *******************************************************************/
		    if (! *++*argv) { --argc; if(! *++argv) goto maxErr; }
		    *(argv-1) = *argv;	/* move app string to previous argv array member */
		    *argv = iC_debug & DQ ?  mqz : mz; /* point to "-qz"  or "-z" in current argv */	
		    argv--;		/* start call with app string */
		    argc = -argc - 1;	/* negative argc signals -R option */
		    goto break3;
		  maxErr:
		    iC_maxErrCount = INT_MAX;	/* maximum error count very high */
		    break;
#endif	/* TCP */
#if defined(RUN) || defined(TCP)
		case '-':
		    /********************************************************************
		     *  Pass rest of command line to app as extra options
		     *  use the first option switch if it comes directly after -- (--h ---h)
		     *******************************************************************/
		    if (*++*argv) {	/* --    rest are extra options for compiled app */
			++argc;		/*       positive argc signals -- option */
			if (**argv != '-') {
			    --*argv;	/* --x  */
			}		/*  ^v  *argv points here */
			--argv;		/* ---x */
		    }			/* -- -x equivalent to previous 2 versions */
		    goto break3;
#else	/* not RUN and not TCP */
		case 'v':
		    fprintf(iC_outFP, "%s\n", iC_ID);
		    exit(0);		/* output version */
#endif	/* not RUN and not TCP */
		missing:
		    fprintf(iC_errFP, "ERROR: %s: missing value after '-%1.1s'\n", iC_progname, ((*--argv)--, *argv));
		    exit(1);
		case 'P':
		    fprintf(iC_outFP, "%s", iC_PATCH);
		    exit(0);		/* output GIT patch if made with dirty version */
		case 'h':
		case '?':
		error:
		    fprintf(iC_errFP, usage, iC_progname, IXD-1,
#if defined(RUN) || defined(TCP)
		    IXD-1, MARKMAX,
#ifdef	TCP
		    iC_hostNM, iC_portNM, iC_iidNM, INSTSIZE,
#endif	/* TCP */
		    iC_progname, iC_progname, iC_progname,
#endif	/* RUN or TCP */
		    iC_ID);
		    exit(1);
		default:
		    fprintf(iC_errFP, "WARNING: %s: unknown option -%c\n", iC_progname, **argv);
		    break;
		}
	    } while (*++*argv);
	  break2: ;
	} else {
#ifdef	TCP
	  setInpFN:
#endif	/* TCP */
	    if (strlen(*argv)) {
#ifdef	WIN32
		inpFN = iC_emalloc(strlen(*argv)+1);	/* +1 for '\0' */
		strcpy(inpFN, *argv);
		while ((cp = strchr(inpFN, '\\')) != 0) {
		    *cp = '/';		/* convert '\' to '/' under WIN32 */
		}
#else	/* not WIN32 */
		inpFN = *argv;
#endif	/* WIN32 */
	    }
#ifdef	TCP
	    if (strcmp(iC_iccNM, "stdin") == 0) {
		iC_iccNM = iC_emalloc(strlen(inpFN)+INSTSIZE+2);	/* +2 for '-...\0' */
		strcpy(iC_iccNM, inpFN);
		if ((cp = strrchr(iC_iccNM, '.')) != 0 && strcmp(cp, ".ic") == 0) {
		    *cp = '\0';		/* terminate at trailing extension .ic */
		}
	    }
#endif	/* TCP */
	}
    }
#if defined(RUN) || defined(TCP)
#ifdef	TCP
    if (strcmp(iC_iccNM, "stdin") != 0 && strlen(iC_iidNM) > 0) {
	snprintf(iC_iccNM + strlen(iC_iccNM), INSTSIZE+2, "-%s", iC_iidNM);
    }
#endif	/* TCP */
  break3:
    /********************************************************************
     *  Extra option switches and other arguments have been isolated after
     *  -R or alternatively option -- (cannot have both)
     *
     *  Extra options passed to iCbegin(iC_argc, iC_argv) in the same way
     *  as the shell passes the initial arguments to main(int argc, char** argv).
     *  if argc < 0 argv contains call + arguments from -R option
     *******************************************************************/
    if (argc == 0) argc = 1;	/* no arguments - pass iC_progname anyway */
    if (argc >= 0 && inpFN) {
	int	i;
	*argv = iC_emalloc(i = strlen(iC_progname) + strlen(inpFN) + 2);
	snprintf(*argv, i, "%s %s", iC_progname, inpFN);
    }
    iC_argc = argc;		/* actual global variables passed to iCbegin() */
    iC_argv = argv;
    if (iC_debug & 0200) fprintf(iC_outFP, "Extra arguments:\n"
					   "argc %d	%s\n", argc, *argv);
    if (argc < 0) argc = -argc; /* -R call + options */
    while (--argc > 0) {
	if (iC_argc > 0) { 	/* else -- + options */
	    if (**++argv == '-') {
		cp = *argv;		/* save *argv before modifying it */
		++*argv;
		do {
		    switch (**argv) {
		    case 'R':
			if (!iC_argh) iC_argh = -32768;	/* stop acting on --h in this app */
			break;
		    case 'h':
		    case '?':
			iC_argh++;	/* block running iCserver before iCbegin() */
		    default:
			break;
		    }
		} while (*++*argv);
		*argv = cp;		/* restore *argv after modifying it */
	    }
	} else {
	    ++argv;
	}
	if (iC_debug & 0200) fprintf(iC_outFP, "	%s\n", *argv);
    }
    if (*++argv != NULL) {
	fprintf(iC_errFP, "WARNING: extra arguments are not NULL terminated: '%s'\n", *argv);
    }
#endif	/* RUN or TCP */
#if defined(RUN)
    if (outFN == 0 && (iC_maxIO < 0 || iC_maxIO > IXD)) {	/* I/O limit for run mode */
	fprintf(iC_errFP, "%s: -k %d exceeds %d for run mode\n", iC_progname, iC_maxIO-1, IXD-1);
	goto error;
    }
#endif	/* RUN */
    iFlag = 0;
    /********************************************************************
     *  Generate and open temporary files T1FN T2FN T3FN
     *  T0FN, T4FN and T5FN are only used if iC and/or C-include files
     *  are actually parsed
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
     *  Call the iC compiler which builds a Symbol table of all immediate
     *  variables and a logic net made up of Symbol's in the Symbol table
     *  linked by List_e's.
     *
     *  As a side effect, all C-code fragments made up of literal blocks
     *  and C-actions are collected in the temporary file T1FN
     *  Also as a side effect outFlag is set if outFN is set (-o option)
     *******************************************************************/
    if ((r = setjmp(beginMain)) ||
	(r = iC_compile(inpFN, listFN, errFN, outFN)) != 0) {
	ro = 5;					/* iC compile error */
    } else
    /********************************************************************
     *  Regroup the generated C-code into literal blocks first, followed
     *  by C-actions. This is necessary to get all C-declarations at the
     *  start of the C-code.
     *
     *  Call the C compiler which recognizes any variables declared as
     *  imm bit or imm int variables. These variables can be used as
     *  values anywhere in the C-code and appropriate modification is
     *  carried out. Immediate variables which have been declared but
     *  not yet assigned may be (multiply) assigned to in the C-code.
     *  Such assignment expressions are recognised and converted to
     *  function calls.
     *
     *  If an immediate variable has already been (singly) assigned to
     *  in the iC code, an attempt to assign to it in the C-code is an
     *  error.
     *******************************************************************/
    if ((r = iC_c_compile(T1FP, T3FP, C_PARSE|C_FIRST|C_BLOCK, 0)) != 0) {
	ro = 6;					/* C compile error */
    } else {
	if (inpFN) {
	    strncpy(inpNM, inpFN, BUFS);	/* restore name if last #line was not inpFN */
	}
	/********************************************************************
	 *  List network topology and statistics - this completes listing
	 *******************************************************************/
	if ((r = iC_listNet()) == 0) {
	    /********************************************************************
	     *  -o option: Output a C-file of all Gates, C-code and links
	     *******************************************************************/
	    if (outFlag) {			/* -o option */
		r = iC_outNet(T3FP, outFN);	/* generate network as C file */
#if defined(RUN) || defined(TCP)
	    } else if (excFN) {			/* -c option */
		/********************************************************************
		 *  -c option: Output a C-file cexe.c to rebuild icr or ict compiler
		 *******************************************************************/
		if ((excFP = fopen(excFN, "w")) == NULL) {
		    r = COindex;		/* cannot open cexe.c for writing */
		} else {
		    unsigned	linecnt = 1;

		    /* write C execution file Part 1 */
		    fprintf(excFP, cexe_part1);
		    linecnt += cexe_lines1;
		    /* copy literal blocks from C intermediate file to C output file */
		    iC_copyXlate(T3FP, excFP, excFN, &linecnt, 01);
		    /* write C execution file Part 2 */
		    fprintf(excFP, cexe_part2, inpNM, iC_progname, iC_gflag ? "g" : "", iC_optimise, inpNM);
		    linecnt += cexe_lines2;
		    /* copy called function cases from C intermediate file to C output file */
		    iC_copyXlate(T3FP, excFP, excFN, &linecnt, 02);
		    /* write C execution file Part 3 */
		    fprintf(excFP, cexe_part3);
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
	    } else if (listFN == 0 && errFN == 0) {
		Gate **		sTable;		/* pointer to Symbol Table */
		Gate **		sTend;		/* end of Symbol Table */
		/********************************************************************
		 *  Build a network of Gates and links for direct execution
		 *******************************************************************/
		if ((r = iC_buildNet(&sTable, &sTend)) == 0) {
		    Symbol * sp = lookup("iClock");
		    unlinkTfiles();
		    /********************************************************************
		     *  Execute the compiled iC logic directly
		     *******************************************************************/
		    assert (sp);		/* iClock initialized in init() */
		    iC_c_list = sp->u_gate;	/* initialise clock list */
		    iC_icc(sTable, sTend);	/* execute the compiled logic */
		    /********************************************************************
		     * never returns - exits via iC_quit()
		     *******************************************************************/
		}
#endif	/* RUN or TCP */
	    }
	}
	/********************************************************************
	 *  iC and C compilation, listing and generated C output complete
	 *******************************************************************/
	if (r != 0) {
	    ro = 7;				/* error in output */
	    fprintf(iC_errFP, OutputMessage[r < 4 ? r : 7], iC_progname, szNames[r]);
	    r += 10;
	    goto closeFiles;
	}
    }
    if (r != 0) {
	fprintf(iC_errFP, OutputMessage[ro], iC_progname, szNames[r]);
    }
closeFiles: ;
    if (iC_outFP && iC_outFP != stdout) {
	fflush(iC_outFP);
	fclose(iC_outFP);
	if (iC_inversionCorrection() != 0) {
	    r += 100;
	}
    }
    if (iC_errFP && iC_errFP != stderr) {
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
 *	When iC_path is not "", pplstfix will be called with the log
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
 *	JW 20120531
 *	Some work has been done on pplstfix to work correctly with
 *	the logic generation listing -d2. Nevertheless it is not
 *	called automatically. To use it filter the logic generation
 *	listing manually with pplstfix.
 *
 *******************************************************************/

int
iC_inversionCorrection(void)
{
    int		r = 0;

    if (listFN &&
	iFlag &&
	(iC_debug & 04027) == 024) {	/* required: logic expansion and net topology */
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
	    fprintf(iC_errFP, "%s: rename(%s, %s) failed\n",
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
		fprintf(iC_errFP, "%s: system(\"%s\") could not be executed $? = %d\n",
		    iC_progname, exStr, r);
	    }
	}
    }
    iFlag = 0;
    r |= chmod(listFN, 0444);		/* make list file read-only */
    return r;
} /* iC_inversionCorrection */

/* ############ POD to generate immcc man page ############################

=encoding utf8

=head1 NAME

immcc - the immediate-C to C compiler

=head1 SYNOPSIS

 immcc [-agASRh][ -o<out>][ -l<lst>][ -e<err>][ -k<lim>][ -d<deb>]
       [ -O<level>][ -Dmacro[=defn]...][ -Umacro...]
       [ -Cmacro[=defn]...][ -Vmacro...][ -W[no-]<warn>...][ <src.ic>]
    -o <out> name of generated C output file
    -l <lst> name of list file  (default: none, '' is stdout) output:
             listing with logic expansion, net topology and statistics
    -e <err> name of error file (default is stderr)
    -a       compile with linking information in auxiliary files
    -g       each expression has its own C code for debugging with gdb
    -A       compile output ARITHMETIC ALIAS nodes for symbol debugging
    -S       strict compile - all immediate variables must be declared
    -p       pedantic: warning if variable contains $ (default $ allowed)
    -pp      pedantic-error: error if variable contains $
    -R       no maximum error count (default: abort after 100 errors)
    -O <level> optimisation -O0 none -O1 bit -O2 arithmetic -O4 eliminate
             duplicate arithmetic expressions; -O7 all (default)
    -D <macro> predefine <macro> for the iC preprocessor phase
    -U <macro> cancel previous definition of <macro> for the iC phase
            Note: do not use the same macros for the iC and the C phase
    -C <macro> predefine <macro> for the C preprocessor phase
    -V <macro> cancel previous definition of <macro> for the C phase
    -W[no-]<warn>                  positive/negative warning options
        -W[no-]deprecated-logic    use of && || ! in pure bit expressions
        -W[no-]function-parameter  unused parameters in functions
        -W[no-]function-delete     delete before function re-definition
        -W[no-]deprecated          [none]/all of the above
        -W[no-]undefined-gate      undefined gate
        -W[no-]unused-gate         unused gate
        -W[no-]all                 [no]/all warnings
    -k <lim> highest I/O index (default: no limit)
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
         +1  iC yacc debug info  (on stdout only) (+4001 C yacc info)
      +4000  supress listing alias post processor (save temp files)
     +20000  generate pre-compiled function blocks for init.c (BOOT_COMPILE)

    <src.ic> iC language source file (extension .ic)
             default: take iC source from stdin
    -v       version\n"
    -P       GIT patch if made with dirty version\n"
    -h       this help text

=head1 DESCRIPTION

B<immcc> translates an iC-source (extension: .ic) with iC-includes
(extension: .ih) into a C file (extension: .c) which can be compiled
with a C compiler and linked with either the dynamic iC project
run time library B<libict.so> or the static library B<libict.a>
(link option -lict) to produce an executable which communicates via
TCP/IP with B<iCserver> and associated I/O-clients - usually B<iCbox>.

B<immcc> reads and translates one iC-source eg file.ic. If no options are
specified, only compilation errors (if any) are reported on 'stderr'.

Before translation starts, file.ic is passed through B<immac -m>
if it contains any lines starting with '#' or if -D or -U options
were used in the command line.

Immediate-C code consists of immediate statements and C literal blocks.
Immediate statements translate into pre-initialised data tables
of nodes and their interconnections and C-actions generated by the
immediate B<if>, B<else> and B<switch> statements. C literal blocks
are C code enclosed in B<%{> and B<%}> braces.

C code in C-actions and in literal blocks is copied nearly verbatim
into the target C file. Any C-preprocessor statements, such as #define
etc, in the C-code in C-actions and in literal blocks must have a B<%>
immediately preceding the leading B<#>, eg B<%#define X 3>. The B<'%'>
is stripped before being copied to the target.

The generated C-code is re-grouped into literal blocks first, followed
by C-actions. This is necessary to place all C-declarations in literal
blocks before any C-actions, which may require them.

In a next pass an integrated special-purpose C compiler is run
(preceded by a pass through B<immac -m>, if necessary). The special C
compiler recognizes any immediate variables in the embedded C code.
Immediate variables are declared with B<imm bit> or B<imm int>
in the iC code section.  These variables can be used as values
anywhere in the C-code and appropriate modification of the C code is
carried out. Immediate variables which have been declared but not yet
assigned may be (multiple) assigned to in the C-code.  Such assignment
expressions are recognised and converted to function calls, which fire
all immediate expressions dependent in the immediate variable when its
value changes.  If the B<strict> option (-S) is used for compilation,
immediate variables to be assigned in C code must be declared with the
type modifier B<immC> rather than with B<imm>.  This is particularly
important if an immediate variable assigned to in C code is going to be
used in another iC source.  If an immediate variable has already been
(single) assigned to in an immediate statement, an attempt to assign
to it in the C-code is an error. A syntax check of the embedded C
code with appropriate error messages is a by-product of this procedure.

The iC and C compiler also produces an optional listing (default
file.lst).  The listing displays the iC source lines with line
numbers. These are interspersed with a detailed logic expansion of
the generated code produced by the compiler. Study of this code gives
insights into how iC-code is compiled and executed.  Aliases appear in
the raw listing because of one pass compiler limitations. Aliases are
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

Email bug reports to B<immediateC@gmail.com> with L<iC Project> in the
subject field.

=head1 SEE ALSO

L<immac(1)>, L<iCmake(1)>, L<makeAll(1)>, L<iCserver(1)>, L<iCbox(1)>

=head1 COPYRIGHT

COPYRIGHT (C) 2000-2012  John E. Wulff

You may distribute under the terms of either the GNU General Public
License or the Artistic License, as specified in the README file.

For more information about this program, or for information on how
to contact the author, see the README file.

=cut

   ############ end of POD to generate immcc man page ##################### */

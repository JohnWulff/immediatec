static const char load_c[] =
"@(#)$Id: load.c 1.69 $";
/********************************************************************
 *
 *  Copyright (C) 1985-2017  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	load.c
 *	This module prepares the data structures for the run time
 *	system from the loaded data and calls the immediate C execution.
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<assert.h>
#include	<fcntl.h>
#ifndef	LOAD
#error - must be compiled with LOAD defined to make a linkable library
#else	/* LOAD */
#ifdef	TCP
#include	"tcpc.h"
#endif	/* TCP */
#include	"icc.h"

#define STS	1

extern const char	iC_ID[];
extern const char	iC_PATCH[];

Gate		iClock = { -1, -CLK, CLCKL, 0, "iClock" };	/* active */

unsigned	errCount;
char *		iC_progname;		/* name of this executable */
char *		iC_vcd = NULL;
short		iC_debug = 0;
int		iC_micro = 0;
unsigned short	iC_osc_max = 0;		/* 0 during Initialisation, when no oscillations */
unsigned short	iC_osc_lim = MARKMAX;
unsigned short	iC_osc_flag = 0;
int		iC_argc;		/* extra options passed to iCbegin(int argc, char** argv) */
char **		iC_argv;
int		iC_argh = 0;		/* block running iCserver before iCbegin() */

FILE *		iC_outFP;		/* listing file pointer */
FILE *		iC_errFP;		/* error file pointer */
#ifdef	RASPBERRYPI

#include	<errno.h>
#include	"rpi_rev.h"		/* Determine Raspberry Pi Board Rev */
#include	"rpi_gpio.h"
#include	"mcp23s17.h"
#include	"bcm2835.h"
#include	"pifacecad.h"
static char	buffer[BS];
static char	IQ[] = "QIQ";
static char *	markings[] = { "External", "Internal", "Dummy", "ExtOut", };
static long long ownUsed = 0LL;
static long	convert_nr(const char * str, char ** endptr);
static int	termQuit(int sig);		/* clear and unexport RASPBERRYPI stuff */
int		(*iC_term)(int) = &termQuit;	/* function pointer to clear and unexport RASPBERRYPI stuff */
#endif	/* RASPBERRYPI */
static short	errorFlag = 0;

static const char *	usage =
"Usage: %s\n          [-"
#if	YYDEBUG
"t"
#endif	/* YYDEBUG */
#ifdef	RASPBERRYPI
"PGEBLIf"
#endif	/* RASPBERRYPI */
#ifdef	TCP
"lqz"
#endif	/* TCP */
"h]"
#ifdef	TCP
#if	YYDEBUG
"[ -m[m]]"
#endif	/* YYDEBUG */
"[ -s <host>][ -p <port>][ -i <inst>]"
#endif	/* TCP */
"\n          "
#ifdef	TCP
"[ -e I|<equivalence>][ -v <file.vcd>]"
#endif	/* TCP */
"[ -n <count>][ -d <debug>]\n"
#ifdef	RASPBERRYPI
"          [ [~]<IEC>[-<IEC>|+[<mask>]][:<pfa>][-<inst>] ...]\n"
"          [ [~]<IEC>,<gpio>[,<gpio>,...][-<inst>] ...]\n"
"          [ [~]<IEC>.<bit>,<gpio>[-<inst>] ...]\n"
#endif	/* RASPBERRYPI */
"          [ --[ -h] ...]"
#ifdef	TCP
"|[ -R <app ...>]"
#endif	/* TCP */
"\n"
#ifdef	TCP
"    -s <host> IP address of iCserver   (default '%s')\n"
"    -p <port> service port of iCserver (default '%s')\n"
"    -i <inst> instance of this client  (default '') or 1 to %d digits\n"
"    -e <equivalence> eg: IX0=IX0-1     (can be used more than once)\n"
"              or IX0 which is changed to the form above using <inst>\n"
"              or a comma separated list of either of the above forms\n"
"              only IEC inputs can be equivalenced (see iCserver)\n"
"    -e I      equivalence all IEC inputs to the same names-<inst>\n"
"    -v <file.vcd> output a .vcd and a .sav file for gtkwave\n"
#endif	/* TCP */
"    -n <count> maximum oscillator count (default is %d, limit 15)\n"
"               0 allows unlimited oscillations\n"
#ifdef	TCP
#ifdef	RASPBERRYPI
"                      PIFACE and GPIO options\n"
"    -P      direct PiFace and/or GPIO I/O rather than via TCP/IP - or\n"
"    -G      direct GPIO I/O rather than via TCP/IP (block PiFaces) - or\n"
"    -E      send PiFace/GPIO output via TCP/IP as well as acting directly\n"
"            this allows equivalencing the output at iCserver (implies -P)\n"
"    -B      start iCbox -d to monitor PiFace and/or GPIO I/O (implies -E)\n"
"    -L      connect to iCserver for live dislay, even if not required for I/O\n"
"    -l      start 'iClive' with correct source               (implies -L)\n"
"          All the above options also allow direct action of GPIO I/O\n"
"            A 1 to PiFace and GPIO outputs generates hi - LED and relay off\n"
"            A 0 to PiFace and GPIO outputs generates lo - LED and relay on\n"
"    -I      invert all PiFace and GPIO inputs and outputs\n"
"            PiFace and GPIO input circuits are normally hi - 1 and\n"
"            when a switch on the input is pressed it goes lo - 0. Therefore\n"
"            it is appropriate to invert inputs and outputs. When inverted\n"
"            a switch pressed on an input generates a 1 for the IEC inputs and\n"
"            a 1 on on IEC output turns a LED and relay on, which is natural.\n"
"    NOTE: the supplied PiFace driver inverts outputs but not inputs - go figure\n"
"    -W GPIO number used by the w1-gpio kernel module (default 4, maximum 31).\n"
"            When the GPIO with this number is used in this app, iCtherm is\n"
"            permanently blocked to avoid Oops errors in module w1-gpio.\n"
"    -f      force use of all interrupting gpio inputs - in particular\n"
"            force use of gpio 25 - some programs do not unexport correctly.\n"
"                      PIFACE arguments\n"
"          For each PiFace or PiFaceCAD connected to a Raspberry Pi\n"
"          8 bits of digital input and 8 bits of digital output with\n"
"          known IEC numbers are generated.\n"
"            When linked directly to an application the IEC I/Os IXx.y and\n"
"          QXx.y used in the application are generated as direct inputs\n"
"          and outputs in ascending order for each PiFace present.\n"
"          ALTERNATIVELY to change the order or name unused PiFace I/Os:\n"
"    IEC      X1 X2 IX8 QX9  (for IEC-1131 names - see iCserver man page)\n"
"          The argument Xn can optionally be preceded by either I for\n"
"          inputs or Q for outputs. The IEC-1131 IEC's generated are\n"
"          followed by a bit selector in the range .0 to .7.\n"
"          For the above list X1 X2 IX8 QX9 the following is generated:\n"
"            inputs IX1.0 - .7 and outputs QX1.0 - .7 for PiFace unit 0\n"
"            inputs IX2.0 - .7 and outputs QX2.0 - .7 for PiFace unit 1\n"
"            inputs IX8.0 - .7 and outputs QX9.0 - .7 for PiFace unit 2\n"
"          A range of IEC's can be specified with X0-X3, IX4-IX6 or QX7-QX9\n"
"          Actual PiFace unit addresses in the range 0 - 7 are selected by\n"
"          jumpers JP1 and JP2 as well as CE0/CE1 (JP1 - JP8 on the PiRack).\n"
"            The program scans which addresses are present and associates\n"
"          them with unit numbers in ascending PiFace address order.\n"
"          Normally IEC addresses will be chosen to coincide with PiFace\n"
"          unit numbers, but this is not mandatory. What is mandatory is that\n"
"          there are enough PiFaces to cover all the IEC's in the arguments.\n"
"    IEC:<pfa> Associate a particular IEC or IEC range with a specific PiFace\n"
"          unit address or address range. <pfa> must be one of numbers 0 - 7\n"
"          and a PiFace with that unit address must be available.\n"
"            By choosing X1-X3:1 for 3 PiFaces, the IEC numbers will\n"
"          be the same as the PiFace addresses which are 1 to 3. This\n"
"          would be good programming practice. On the other hand there\n"
"          are good reasons to choose different IEC arguments to fit in with\n"
"          the iC application code and other I/O's.\n"
"    D ID QD may be used as dummy IEC's instead of Xn IXn QXn to exclude a\n"
"          PiFace unit or an individual PiFace input or output.\n"
"                      PIFACE GPIOB+ extra arguments\n"
"    QXn+  The whole GPIOB register is a 2nd output rather than an input\n"
"    QXn+<mask>  Only the masked bits of GPIOB register are output bits\n"
"    IXn+        The whole GPIOB register is an input (same as IXn)\n"
"    IXn+<mask>  Only the masked bits of GPIOB register are input bits\n"
"          <mask> for bits 0 - 3 may be expressed as 0b1111, 0xf, 017 or 15\n"
"          The masks for output and input may not both contain the same bit\n"
"    QXn+:<pfa> etc Associate a particular IEC+ with a specific PiFace address\n"
"          PiFace GPIOB+ selections cannot be part of a range.\n"
"                      GPIO arguments (never generated automatically)\n"
"          Any IEC I/Os IXx.y and QXx.y used in the application which are\n"
"          to be linked to GPIO direct inputs or outputs must be named in\n"
"          the argument list as follows:\n"
"    IEC,<gpio-list> Associate the bits of a particular input or output IEC\n"
"          with a list of gpio numbers.  The IEC must start with either\n"
"          I or Q in this case and no IEC range is allowed.  Up to 8\n"
"          gpio numbers can be given in a comma separated list. The\n"
"          first gpio number will be aassociated with bit 0, the second\n"
"          with bit 1 etc and the eighth with bit 7. If the list is\n"
"          shorter than 8 the trailing bits are not used. The letter\n"
"          'd' can be used in the list instead of a gpio number to mark\n"
"          that bit as unused.  eg. IX3,17,18,19,20,d,d,21 associates\n"
"          IX3.0 - IX3.3 with GPIO17 - GPIO20 and IX3.6 with GPIO21.\n"
"    IEC.<bit>,<gpio> Alternatively each input or ourput <bit> 0 to 7 can\n"
"          be associated with one gpio nunber eg. IX3.0,17 QX3.7,18\n"
"                      COMMON extensions\n"
"    ~IEC  Each individual IEC or range can be preceded by a tilde '~',\n"
"          which inverts individual IEC inputs or outputs (see -I flag).\n"
"          If all IEC inputs and outputs are already inverted with -I\n"
"          ~ inverts it again, which makes it non-inverted. Individual\n"
"          bits for one IEC can be inverted or left normal for GPIO IEC's\n"
"          by declaring the IEC more than once with non-overlapping\n"
"          gpio bit lists or single bits. (Not possible for PiFaces)\n"
"    IEC-inst Each individual IEC or range can be followed by -<inst>,\n"
"          where <inst> consists of 1 to %d numeric digits.\n"
"          Such an appended instance code takes precedence over the\n"
"          general instance specified with the -i <inst> option above.\n"
"                      SPARE IEC's\n"
"            Any spare IEC arguments not used in the application are made\n"
"          available to iCserver via TCP/IP for use by other applications.\n"
"          This applies particularly to IEC-inst arguments, which cannot be\n"
"          used directly in an iC application program. For these arguments\n"
"          the application behaves exactly like iCpiFace or iCpiGPIO.\n"
"            On the other hand any missing IEC arguments (particulatly all\n"
"          analog IEC's) will be supplied by iCserver via TCP/IP from\n"
"          external I/O units in the usual way.\n"
"    For a full Description and Technical Background of the PiFace and GPIO\n"
"    driver software see the iCpiFace man page\n"
"                      CAVEAT\n"
"    Only one instance of this app with IEC parameters or iCpiFace may be run\n"
"    and all GPIOs, PiFaces and an optional PiFaceCAD must be controlled by\n"
"    this one instance. If two instances were running, the common interrupts\n"
"    would clash. Also no other program controlling GPIOs and PiFaces like\n"
"    'PiFace Digital Emulator' may be run at the same time as this application.\n"
"    An exception is iCpiPWM which controls GPIOs by DMA and not by interrupts.\n"
"    Another exception is iCtherm which controls GPIO 4 by the 1Wire interface.\n"
"    Care is taken that any GPIOs or PiFaces used in one app, iCpiFace, iCpiPWM\n"
"    or even iCtherm do not clash with another app (using ~/.iC/gpios.used).\n"
#else	/* RASPBERRYPI */
"    -l      start 'iClive' with correct source\n"
#endif	/* RASPBERRYPI */
#endif	/* TCP */
"                      DEBUG options\n"
#if	YYDEBUG
"    -d <debug>2000  display scan_cnt and link_cnt\n"
"             +1000  do not trace non-active timers TX0.n\n"
"              +400  exit after initialisation\n"
"              +200  display oscillator info\n"
"              +100  initialisation and run time trace\n"
#ifdef	TCP
"        or     +20  trace only logic inputs as binary bytes\n"
#endif	/* TCP */
#else	/* YYDEBUG */
"    -d <debug> 400  exit after initialisation\n"
#endif	/* YYDEBUG */
"               +40  load listing\n"
"                +4  extra install debug info\n"
#if	YYDEBUG
"                +2  trace I/O receive buffer\n"
"                +1  trace I/O send buffer\n"
"    -t      trace gate activity (equivalent to -d 1100)\n"
"         t  at run time toggles gate activity trace\n"
#ifdef	TCP
"    -m      microsecond timing info\n"
"    -mm     more microsecond timing (internal time base)\n"
"         m  at run time toggles microsecond timing trace\n"
#endif	/* TCP */
#endif	/* YYDEBUG */
#ifdef	TCP
"    -q      quiet - do not report clients connecting and disconnecting\n"
"    -z      block keyboard input on this app - used by -R\n"
#endif	/* TCP */
"    -Z      GIT patch if made with dirty version\n"
"    -h      this help text\n"
"         T  at run time displays registrations and equivalences\n"
"         q  or ctrl+D  at run time stops %s\n"
"                      EXTRA arguments\n"
"    --      any further arguments after -- are passed to the app\n"
"    --h     help with command line options particular to this app\n"
#ifdef	TCP
"                      AUXILIARY app\n"
"    -R <app ...> run auxiliary app followed by -z and its arguments\n"
"                 as a separate process; -R ... must be last arguments.\n"
#endif	/* TCP */
"compiled by:\n"
"%s\n"
"Copyright (C) 1985-2015 John E. Wulff     <immediateC@gmail.com>\n"
;

/********************************************************************
 *
 *  Constant compile typing data
 *
 *******************************************************************/

char *		iC_full_type[]  = { FULL_TYPE };
char *		iC_full_ftype[] = { FULL_FTYPE };
unsigned char	iC_types[]      = { TYPES };
unsigned char	iC_ctypes[]     = { CTYPES };
unsigned char	iC_ftypes[]     = { FTYPES };
char		iC_os[]         = OPS;
char		iC_fos[]        = FOPS;
char *		iC_useText[4]	= { USE_TEXT };

Gate *		iC_TX0p;		/* pointer to bit System byte Gates */
Gate **		sTable;			/* pointer to dynamic Symbol Table array */
Gate **		sTend;			/* end of dynamic Symbol Table array */

/********************************************************************
 *
 *  Error message routine for errors in loading application
 *
 *******************************************************************/

static void
inError(int line, Gate * op, Gate * gp, const char* errMsg)
{
    fprintf(iC_errFP,
	"ERROR %s: line %d: op = %s gp = %s; %s\n",
	    __FILE__, line, op->gt_ids, gp ? gp->gt_ids : "'null'", errMsg);
    errCount++;
} /* inError */

/********************************************************************
 *
 *  Main for the whole application
 *
 *******************************************************************/

int
main(
    int		argc,
    char **	argv)
{
    Gate *		op;
    Gate *		gp;
    Gate ***		oppp;
    Gate **		opp;
    Gate **		lp;
    Gate **		slp;
    Gate **		tlp;
    Gate *		tgp;
    Gate *		ttgp;
    Gate *		e_list = 0;
    unsigned		inversion;
    unsigned		val;
    unsigned		e_cnt;
    unsigned		link_count = 0;
    Gate **		fp;
    Gate **		ifp;
    int			i;
    unsigned		df = 0;
    char *		cp;
    int			len;
    char		iqt[2];		/* single char buffer - space for 0 terminator */
    char		xbwl[2];	/* single char buffer - space for 0 terminator */
    int			byte;
    unsigned short	bit;
    char		tail[128];	/* must match sscanf formats %127s for tail */
    char		eBuf[ESIZE];
#ifdef	TCP
    char *		mqz = "-qz";
    char *		mz  = "-z";
    char *		qp;			/* points to next entry in rpyBuf for equivalence inputs */
    int			qc;			/* length of filled entries in rpyBuf */
    int			ql;			/* length of individual entry in rpyBuf */
    int			qFlag;
#endif	/* TCP */
#ifdef	RASPBERRYPI
    char **		argp;
    int			pfce;
    unsigned short	pfa;
    unsigned short	pfe;
    unsigned short	pff;
    int			value;
    int			sig;
    int			n;
    int			m;
    int			un;
    int			iq;
    int			unit;
    int			maxUnit = 0;
    int			unitA[3] = { 0, 0, 0 };
    unsigned short	iid;
    char		iids[6];
    unsigned short	iidN = 0xffff;	/* internal instance "" initially */
    int			forceFlag = 0;
    char *		iC_fullProgname;
    piFaceIO *		pfp;
    iqDetails *		pfq;
    char		xd[2]  = { '\0', '\0' };
    char		iqc[2] = { '\0', '\0' };
    char		iqe[2] = { '\0', '\0' };
    char		dum[2] = { '\0', '\0' };
    int			iqStart;
    int			iqEnd;
    int			iqExtra;
    int			ieStart = 0;
    int			ieEnd   = 0;
    unsigned short	bitStart, bitEnd;
    unsigned short	gpio;
    unsigned short	directFlag;		/* or PF for PiFace, GP for GPIO, DR direct for both */
    #define		PF	1
    #define		GP	2
    #define		DR	4
    gpioIO *		gep;
    gpioIO **		gpioLast;
    unsigned short	gpioSav[8];
    unsigned short	u;
    int			mask;
    int			b;
    int			invMask;
    int			invFlag;
    int			gpioTherm;
    char **		argip;
    int			argic;
    long long		gpioMask;
    ProcValidUsed *	gpiosp;

    /********************************************************************
     *  By default do not invert PiFace and GPIO inputs as well as GPIO outputs
     *  By default invert PiFace outputs
     *  Both are inverted again with -I and '~'
     *******************************************************************/

    invMask = invFlag = 0x00;
    gpioTherm = 4;				/* default GPIO number used by kernel module w1-gpio */
    argip = iC_emalloc(argc * sizeof(char *));	/* temporary array to hold IEC arguments */
    argic = 0;
    iC_opt_B = iC_opt_E = iC_opt_L = iC_opt_l = iC_opt_G = iC_opt_P = 0;	/* PiFace/GPIO only active with -P or -G option */
    iC_fullProgname = iC_emalloc(strlen(*argv)+1);
    strcpy(iC_fullProgname, *argv);	/* for chmod */
#endif	/* RASPBERRYPI */

    /********************************************************************
     *
     *  Determine program name
     *
     *******************************************************************/

    if ((iC_progname = strrchr(*argv, '/')) == NULL
#ifdef	WIN32
	&& (iC_progname = strrchr(*argv, '\\')) == NULL
#endif	/* WIN32 */
    ) {
	iC_progname = *argv;
    } else {
	iC_progname++;			/*  path has been stripped */
    }
#ifdef	TCP
    iC_iccNM = iC_emalloc(strlen(iC_progname)+INSTSIZE+2);	/* +2 for '-...\0' */
    strcpy(iC_iccNM, iC_progname);
#ifdef	WIN32
    if ((cp = strrchr(iC_iccNM, '.')) != 0 && strcmp(cp, ".exe") == 0) {
	*cp = '\0';			/* terminate at trailing extension .exe */
    }
#endif	/* WIN32 */
#endif	/* TCP */
    iC_outFP = stdout;			/* listing file pointer */
    iC_errFP = stderr;			/* error file pointer */

#ifdef	EFENCE
    regBuf = iC_emalloc(REQUEST);
    rpyBuf = iC_emalloc(REPLY);
#endif	/* EFENCE */
    qp = rpyBuf;		/* prepare to collect comma separated list of equivalences */
    qc = REPLY;			/* for -e options */
    ql = qFlag = 0;

    /********************************************************************
     *
     *  Process command line arguments
     *
     *******************************************************************/

    while (--argc > 0) {
	if (**++argv == '-') {
	    /********************************************************************
	     *  Analyze -x switch options
	     *******************************************************************/
	    ++*argv;
	    do {
		switch (**argv) {
		    int		slen;
#ifdef	TCP
		case 's':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) iC_hostNM = *argv; else goto missing;
		    goto break2;
		case 'p':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) iC_portNM = *argv; else goto missing;
		    goto break2;
		case 'i':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if ((slen = strlen(*argv)) > INSTSIZE ||
			slen != strspn(*argv, "0123456789")) {
			fprintf(iC_errFP, "ERROR: %s: '-i %s' is non numeric or longer than %d digits\n",
			    iC_progname, *argv, INSTSIZE);
			errorFlag++;
		    } else {
			iC_iidNM = *argv;
			snprintf(iC_iccNM + strlen(iC_iccNM), INSTSIZE+2, "-%s", iC_iidNM);
#ifdef	RASPBERRYPI
			iidN = (unsigned short)atoi(iC_iidNM);	/* internal instance from command line -i<instance> */
#endif	/* RASPBERRYPI */
		    }
		    goto break2;
		case 'e':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    ql = snprintf(qp, qc, ",%s", *argv);	/* equivalence directly into rpyBuf */
		    qc -= ql;
		    if (qc > 0) {
			qp += ql;			/* ready for next entry */
		    } else {
			qc += ql;			/* rpyBuf overflowed - ignore for equivalences */
			*qp = '\0';			/* delete latest entry - this works in the test case */
			if (qFlag == 0) {
			    fprintf(iC_errFP, "WARNING: %s: too many -e equivalences - ignore '%s' and later\n",
				iC_progname, *argv);
			    qFlag = 1;			/* unlikely to overflow, but code has been tested */
			}				/* with small rpyBuf[13] trying to fit 14 bytes */
		    }
		    goto break2;
		case 'v':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) iC_vcd = *argv; else goto missing;
		    goto break2;	/* output vcd dump file for gtkwave */
		case 'm':
		    iC_micro++;		/* microsecond info */
		    break;
		case 'l':
		    iC_opt_l = 1;	/* start iClive with correct source */
#ifdef	RASPBERRYPI
		    iC_opt_L = 1;	/* -l implies -L but not -P for RPi */
		    break;
		case 'B':
		    iC_opt_B = 1;	/* start iCbox -d to monitor active PiFace or GPIO I/O */
		    /* fall through */
		case 'E':
		    iC_opt_E = 1;	/* PiFace or GPIO direct action + TCP/IP I/O for equivalence + live */
		    /* fall through */
		case 'L':
		    iC_opt_L = 1;	/* PiFace or GPIO direct action + connect to iCserver for live display */
		    /* fall through */
		case 'P':
		    iC_opt_P = 1;	/* PiFace or GPIO direct action I/O - only connect if not enough PiFaces */
		    break;
		case 'G':
		    iC_opt_G = 1;	/* GPIO direct action only - block PiFace IO, even if PiFaces present */
		    iC_opt_P = 1;	/* -G can be used in conjunction with -B -E or -L to block PiFaces */
		    break;
		case 'I':
		    invMask = 0xff;	/* invert PiFace and GPIO inputs as well as GPIO outputs with -I */
		    iC_opt_P = 1;	/* do not invert PiFace outputs with -I (inverted a second time) */
		    break;
		case 'W':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) gpioTherm = atoi(*argv); else goto missing;
		    if (gpioTherm > 31) {
			fprintf(iC_errFP, "ERROR: %s: -W %hu value > 31\n", iC_progname, gpioTherm);
			errorFlag++;
		    }
		    goto break2;
		case 'f':
		    forceFlag = 1;	/* force use of gpio 25 */
#endif	/* RASPBERRYPI */
		    break;
#endif	/* TCP */
		case 'd':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (!(slen = strlen(*argv))) goto missing;
		    if (slen != strspn(*argv, "01234567") ||	/* octal digits only */
			sscanf(*argv, "%o", &df) != 1 ||
			df & ~03777) {
			fprintf(iC_errFP, "ERROR: %s: '-d %s' is not a well formed octal string or exceeds range 3777\n",
			    iC_progname, *argv);
			errorFlag++;
		    }
		    iC_debug |= df;	/* short */
		    df &= 040;		/* load listing */
		    goto break2;
		case 't':
		    iC_debug |= 01100;	/* trace gate activity */
		    break;
		case 'n':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) iC_osc_lim = atoi(*argv); else goto missing;
		    if (iC_osc_lim > 15) {
			fprintf(iC_errFP, "ERROR: %s: -n %hu value > 15\n", iC_progname, iC_osc_lim);
			errorFlag++;
		    }
		    goto break2;
		case 'q':
		    iC_debug |= DQ;	/* -q    quiet operation of all apps and iCserver */
		    break;
		case 'z':
		    iC_debug |= DZ;	/* -z    block all STDIN interrupts for this app */
		    break;
		case 'Z':
		    fprintf(iC_outFP, "%s", iC_PATCH);
		    exit(0);		/* output GIT patch if made with dirty version */
#ifdef	TCP
		case 'R':
		    /********************************************************************
		     *  Run auxiliary app with rest of command line
		     *  splice in the "-z" option to block STDIN interrupts in chained apps
		     *  alternatively "-qz" option for quiet operation and to block STDIN
		     *******************************************************************/
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    *(argv-1) = *argv;	/* move app string to previous argv array member */
		    *argv = iC_debug & DQ ?  mqz : mz; /* point to "-qz"  or "-z" in current argv */
		    argv--;		/* start call with app string */
		    argc = -argc - 1;	/* negative argc signals -R option */
		    goto break3;
#endif	/* TCP */
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
		missing:
		    fprintf(iC_errFP, "ERROR: %s: missing value after '%s'\n", iC_progname, ((*--argv)-=2, *argv));
		case 'h':
		case '?':
		    fprintf(iC_errFP, usage, iC_progname,
#ifdef	TCP
		    iC_hostNM, iC_portNM, INSTSIZE,
#endif	/* TCP */
		    MARKMAX,
#ifdef	RASPBERRYPI
		    INSTSIZE,
#endif	/* RASPBERRYPI */
		    iC_progname, iC_ID);
		    exit(-1);
		default:
		    fprintf(iC_errFP, "WARNING: %s: unknown option -%c\n", iC_progname, **argv);
		    break;
		}
	    } while (*++*argv);
	    break2: ;
	} else {
#ifdef	RASPBERRYPI
	    /********************************************************************
	     *  Save IEC arguments for later analysis after all options have
	     *  been determined, in particlar -G, and after Pifaces have
	     *  been counted. IEC arguments and options may be mixed.
	     *  IEC arguments must start with ~ X I Q or D.
	     *  A special case is the HOME evironment variable, which is generated
	     *  by the Linux shell for ~. It is converted back to ~ here.
	     *******************************************************************/
	    iC_opt_P = 1;	/* using an IEC argument is equivalent to -P option */
	    if (strspn(*argv, "~IQXD") == 0) {
		if (strcmp(*argv, getenv("HOME")) == 0) {
		    argip[argic++] = "~";	/* transmogrify shell translation of HOME to ~ */
		    continue;			/* get next command line argument */
		}
		fprintf(iC_errFP, "ERROR: %s: invalid argument '%s' (IEC variables start with ~ I Q X or D for dummy)\n",
		    iC_progname, *argv);
		errorFlag++;
	    }
	    argip[argic++] = *argv;
#else	/* ! RASPBERRYPI */
	    fprintf(iC_errFP, "WARNING: %s: invalid argument '%s'\n",
		iC_progname, *argv);
#endif	/* ! RASPBERRYPI */
	}
    }
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
    if (argc >= 0) {
	*argv = iC_progname;	/* pass iC_progname in argv[0] for -- */
    }
    iC_argc = argc;		/* actual global variables passed to iCbegin() */
    iC_argv = argv;
 //    if (iC_debug & 0200) fprintf(iC_outFP, "Extra arguments:\n"
 //					   "argc %d	%s\n", argc, *argv);
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
 //	if (iC_debug & 0200) fprintf(iC_outFP, "	%s\n", *argv);
    }
    if (*++argv != NULL) {
	fprintf(iC_errFP, "WARNING: extra arguments are not NULL terminated: '%s'\n", *argv);
    }
#ifdef	RASPBERRYPI
    directFlag = 0;
    if (! errorFlag && iC_opt_P) {
	/********************************************************************
	 *  Open or create and lock the auxiliary file ~/.iC/gpios.rev,
	 *  which contains one binary struct ProcValidUsed gpios.
	 *  If rpi.rev exists, gpios.proc and gpios.valid is checked
	 *  against the values returned by boardrev(). (Must be the same)
	 *  For an RPi B+ processor it contains:
	 *  	gpios.proc	1
	 *  	gpios.valid	0x00000000fbc6cf9c
	 *  	gpios.used	0x0000000000000000
	 *
	 *  gpios.proc is the int returned by boardrev() // Raspberry Pi Board revision
	 *
	 *  gpios.valid has 64 bits - a bit is set for each valid GPIO
	 *
	 *  gpios.used has 64 bits - a bit is set for each GPIO used in other apps
	 *  If -f (forceFlag) is set the used word is cleared unconditionally
	 *  Else only GPIO's not used in other apps can be used in this app.
	 *******************************************************************/
	if ((gpiosp = openLockGpios(forceFlag)) == NULL) {
	    fprintf(iC_errFP, "ERROR: %s: in openLockGpios()\n",
		iC_progname);
	    goto FreeNow;
	}
	if (iC_debug & 0200) fprintf(iC_outFP,
	    "Raspberry Pi Board revision = %d (0 = A,A+; 1 = B; 2 = B+,2B)\n"
	    "valid    = 0x%016llx\n",
	    gpiosp->proc, gpiosp->valid);
	if (! iC_opt_G) {
	    /********************************************************************
	     *  Initialise PiFace software control structures
	     *******************************************************************/
	    for (pfp = iC_pfL; pfp < &iC_pfL[MAXPF]; pfp++) {
		for (iq = 0; iq < 3; iq++) {
		    pfq = &(pfp->s[iq]);
		    pfq->i.name = NULL;		/* PiFace IEC in or output not active */
		    pfq->val = -1;		/* Dummy place holder for all PiFaces */
		    pfq->channel = 0xffff;
		    pfq->bmask = 0x00;
		}
		pfp->pfa = 0xffff;		/* no pfa assigned */
		pfp->spiFN = -1;		/* no SPI file open */
	    }
	    /********************************************************************
	     *
	     *  Identify and count PiFaces available.
	     *
	     *  Do this before checking whether GPIO 7 - 11 are not used by other apps.
	     *  GPIO 7 - 11 (CE1 CE0 MISO MOSI SPI_CLK) are used during SPI and MCP23S17
	     *  setup, but are left unchanged if no PiFaces are found. TODO check
	     *  If a PiFace is found and they are in use by another app this will
	     *  show up in the next section (iC_npf > 0) and an error will occurr.
	     *  It is up to the user to also stop those other apps and the error message
	     *  will point this out. There is no use trying to use GPIO 7 - 11 with
	     *  PiFace hardware present, which will interfere. This can be attempted
	     *  by using the -G switch (opt_G set) with PiFace hardware. It may work.
	     *
	     *  If no PiFaces are found proceed with opt_G set. An error will then occurr
	     *  only if an argument requesting PiFace IO (Xn or Xn:<pfa>) is found.
	     *
	     *  Initialisation of /dev/spidev0.0 and/or /dev/spidev0.1 for SPI
	     *  modes and read/writeBytes
	     *
	     *  Test for the presence of a PiFace by writing to the IOCON register and
	     *  checking if the value returned is the same. If not, there is no PiFace
	     *  at that address. Do this for all 4 addresses on both CE0 and CE1.
	     *
	     *  Initialise all PiFaces found by writing to all relevant MCP23S17
	     *  registers because there may be input interrupts.
	     *
	     *******************************************************************/
	    pfp = iC_pfL;
	    pfa = pfe = pff = value = iC_npf = 0;
	    for (pfce = 0; pfce < 2; pfce++) {		/* scan CE0 and CE1 groups */
		if ((spidFN[pfce] = setupSPI(pfce)) < 0) {
		    fprintf(iC_errFP, "ERROR: %s: setup /dev/spidev0.%d failed: %s\n", iC_progname, pfce, strerror(errno));
		}
		m = 0;
		for (pfa = pfe ; pfa < 8; pfa++) {	/* scan 8 or 4 PiFaces in each group */
		    if (setupMCP23S17(spidFN[pfce], pfa, IOCON_ODR, 0, 0x00, 0x00) < 0) {	/* detect presence */
			continue;			/* no PiFace at this address */
		    }
		    pff = pfa;
		    assert(iC_npf < MAXPF);
		    pfp->pfa = pfa;			/* PiFace             selection fixed in iC_pfL[] entry */
		    if (pfa == 4 && pfe == 4) {		/* detect PiFaceCAD */
			/********************************************************************
			 *  potentially a PiFaceCAD - test gpio 23 the LIRC output
			 *******************************************************************/
			if ((sig = gpio_export(23, "in", "falling", forceFlag, iC_fullProgname)) != 0) {
			    iC_quit(sig);		/* another program is using gpio 23 */
			}
			if ((gpio23FN = gpio_fd_open(23, O_RDONLY)) < 0) {
			    fprintf(iC_errFP, "ERROR: %s: PiFace open gpio 23: %s\n", iC_progname, strerror(errno));
			}
			/********************************************************************
			 *  read the initial /sys/class/gpio/gpio23/value
			 *******************************************************************/
			if ((value = gpio_read(gpio23FN)) == -1) {
			    fprintf(iC_errFP, "ERROR: %s: PiFace read gpio 23: %s\n", iC_progname, strerror(errno));
			}
			if (iC_debug & 0200) fprintf(iC_outFP, "Initial read 23 = %d (PiFaceCAD LIRC output)\n",
			    value);
			/********************************************************************
			 *  Close gpio23/LIRC value
			 *******************************************************************/
			if (gpio23FN > 0) {
			    close(gpio23FN);		/* close connection to /sys/class/gpio/gpio23/value */
			    gpio23FN = -1;
			}
			/********************************************************************
			 *  free up the sysfs for gpio 23 - is not marked in gpios->u.used
			 *******************************************************************/
			if ((sig = gpio_unexport(23)) != 0) {
			    iC_quit(sig);		/* unable to unexport gpio 23 */
			}
		    }
		    if (pfa != 4 || value == 0) {	/* PiFace */
			pfp->intf = INTFB;		/* input on MCP23S17 GPB input interrupt register */
			pfp->inpr = GPIOB;		/* INTCAPB fails on bouncing switches */
			if (iC_debug & 0200) fprintf(iC_outFP, "PiFace %hu found\n", pfa);
		    } else {				/* PiFaceCAD is address 4 and GPIO23 LIRC is 1 */
			pfp->intf = INTFA;		/* input on MCP23S17 GPA input interrupt register */
			pfp->inpr = GPIOA;		/* INTCAPA fails on bouncing switches */
			if (iC_debug & 0200) fprintf(iC_outFP, "PiFaceCAD %hu found\n", pfa);
		    }
		    pfp->spiFN = spidFN[pfce];		/* SPI file number for this active unit */
		    pfp++;				/* next iC_pfL[] element */
		    m++;				/* count active PiFaces in this CE group */
		    iC_npf++;				/* count all active PiFaces */
		}
		if (m == 0) {
		    close(spidFN[pfce]);		/* no active PiFaces in this CE group */
		    spidFN[pfce] = -1;
		}
		pfe += 4;
		if (pff >= pfe) {
		    break;				/* PiFace Relay+ - adress range 0 - 7 */
		}
	    }
	    if (iC_npf) {
		/********************************************************************
		 *  PiFaces found
		 *  Check if GPIO 7-11,25 required by PiFace have been used in another app
		 *******************************************************************/
		gpioMask = 0x02000f80LL;		/* gpio 7-11,25 */
		assert((gpiosp->valid & gpioMask) == gpioMask);	/* assume these are valid for all RPi's */
		if (iC_debug & 0200) fprintf(iC_outFP,
		    "gpio     = 7, 8, 9, 10, 11, 25\n"
		    "used     = 0x%016llx\n"
		    "gpioMask = 0x%016llx\n",
		    gpiosp->u.used, gpioMask);
		if ((gpioMask & gpiosp->u.used)) {
		    gpioMask &= gpiosp->u.used;
		    fprintf(iC_errFP, "ERROR: %s: The following GPIO's required by PiFace have been used in another app\n",
			iC_progname);
		    for (b = 7; b <= 25; b++) {
			if ((gpioMask & (1LL << b))) {
			    fprintf(iC_errFP, "		GPIO %d\n", b);
			}
		    }
		    fprintf(iC_errFP,
			"	Cannot run PiFaces correctly - also stop apps using these GPIOs,\n"
			"	because they will be interfered with by PiFace hardware\n");
		    errorFlag++;
		    goto UnlockGpios;
		}
		/********************************************************************
		 *  Free to use PiFaces
		 *  Block GPIOs 7-11,25 required by PiFace hardware
		 *******************************************************************/
		gpiosp->u.used |= gpioMask;		/* mark gpio used for ~/.iC/gpios.rev */
		ownUsed        |= gpioMask;		/* mark gpio in ownUsed for termination */
		/********************************************************************
		 *  Initialisation using the /sys/class/gpio interface to the GPIO
		 *  systems - slightly slower, but always usable as a non-root user
		 *  export GPIO25, which is INTB of the MCP23S17 device.
		 *
		 *  Wait for Interrupts on GPIO25 which has been exported and then
		 *  opening /sys/class/gpio/gpio25/value
		 *
		 *  This is actually done via the /sys/class/gpio interface regardless of
		 *  the wiringPi access mode in-use. Maybe sometime it might get a better
		 *  way for a bit more efficiency.	(comment by Gordon Henderson)
		 *
		 *  Set the gpio25/INTB for falling edge interrupts
		 *******************************************************************/
		if ((sig = gpio_export(25, "in", "falling", forceFlag, iC_fullProgname)) != 0) {
		    iC_quit(sig);			/* another program is using gpio 25 */
		}
		/********************************************************************
		 *  open /sys/class/gpio/gpio25/value permanently for obtaining
		 *  out-of-band interrupts
		 *******************************************************************/
		if ((gpio25FN = gpio_fd_open(25, O_RDONLY)) < 0) {
		    fprintf(iC_errFP, "ERROR: %s: PiFace open gpio 25: %s\n", iC_progname, strerror(errno));
		}
		if (gpio25FN > iC_maxFN) {
		    iC_maxFN = gpio25FN;
		}
		/********************************************************************
		 *  read the initial /sys/class/gpio/gpio25/value
		 *******************************************************************/
		if ((value = gpio_read(gpio25FN)) == -1) {
		    fprintf(iC_errFP, "ERROR: %s: PiFace read gpio 25: %s\n", iC_progname, strerror(errno));
		}
		if (iC_debug & 0200) fprintf(iC_outFP, "Initial read 25 = %d\n", value);
		/********************************************************************
		 *  To allow several PiFace units and possibly a PiFaceCAD to run
		 *  simultaneously on a PiRack, theINTB/GPIO25 line requires a pullup
		 *  resistor, so the individual INTB outputs from each MCP23S17 can signal
		 *  their interrupts via open-drain outputs. There is no pullup resistor on
		 *  the PiFace, PiFaceCAD or the PiRack. Initially I solved this problem
		 *  by soldering a 100 Kohm resistor on the PiRack, but this meant the
		 *  software had to distinguish between a single PiFace mounted directly
		 *  on the RPi and several mounted on a PiRack. In the single case, the
		 *  INTB line can be hard driven without a pullup rather than open_drain.
		 *
		 *  A much simpler solution is to activate the 60 Kohm pullup resistor
		 *  on the GPIO25 output of the RPI. Then open_drain on INTB can be used
		 *  in both cases.
		 ********************************************************************
		 *  Execute the SUID root progran iCgpioPUD(25, 2) to to this.
		 *******************************************************************/
		iC_gpio_pud(25, BCM2835_GPIO_PUD_UP);	/* Enable Pull Up */
	    } else {
		iC_opt_G = 1;		/* no Pifaces - block all PiFace command line arguments */
	    }
	}
	/********************************************************************
	 *  Analyze IEC arguments X0 X1 IX2 QX3 etc or groups X4-X7 and any
	 *  D ID QD dummy arguments
	 *
	 *  Match PiFaces, an optional PiFaceCAD and GPIOs to IEC arguments
	 *  in the command line.
	 *******************************************************************/
	for (argp = argip; argp < &argip[argic]; argp++) {
	    pfa = 0xffff;
	    ieStart = ieEnd = 0;
	    directFlag &= DR;				/* clear all bits except DR for each new argument */
	    if (strlen(*argp) >= 128) {
		fprintf(iC_errFP, "ERROR: %s: command line argument too long: '%s'\n", iC_progname, *argp);
		exit(1);
	    }
	    iid = iidN;					/* global instance either 0xffff for "" or 0 - 999 */
	    *iids = '\0';				/* track argument instance for error messgaes */
	    strncpy(tail, *argp, 128);			/* start scans with current argument */
	    /********************************************************************
	     *  Scan for an optional tilde ~ before any IEC argument, which inverts
	     *  the input or output sense of the following IEC argument.
	     *  ~ either buts up to the IEC argument or can be separated by a space,
	     *  although this may cause problems, since the Linux shell converts a
	     *  lone ~ to the home directory string, which is not what we want.
	     *  To act as expected, the HOME directory string is converted back
	     *  to ~ in the first argument scan above.
	     *  More than 1 tilde is a bit silly, but each one inverts the
	     *  output sense - thus keeping the logic correct.
	     *******************************************************************/
	    while ((n = sscanf(tail, "%1[~]%127s", dum, tail)) > 0) {
		invFlag ^= 0xff;			/* invert following argument logic sense */
		if (n == 1) {
		    goto skipInvFlag;			/* lone '~' - apply invFlag to next argument */
		}
	    }
	    /********************************************************************
	     *  Look for a '+' in the argument for GPIOB+ outputs
	     *******************************************************************/
	    iqExtra = (strchr(tail, '+')) ? 2 : 0;
	    mask = 0xff;				/* default is all bits of input or output active */
	    /********************************************************************
	     *  Scan for an optional I or Q in IEC arguments IXx or QXx
	     *******************************************************************/
	    if (sscanf(tail, "%1[IQ]%127s", iqc, tail) == 2) {
		iqStart = iqEnd = (*iqc == 'I') ? 1 : iqExtra;	/* 0 for 'Q', 1 for 'I', 2 for Qn+ */
	    } else {
		if (iqExtra == 2) goto illFormed;	/* fails if Xn+ without I or Q */
		*iqc    = '\0';				/* no I or Q IEC */
		iqStart = 0;				/* or lone I or Q - trapped in next sscanf */
		iqEnd   = 1;
		unitA[0] = unitA[1] = max(unitA[0], unitA[1]);	/* Xx or D are paired */
	    }
	    /********************************************************************
	     *  Scan for a mandatory Xx or D in IEC arguments Xx IXx or QXx or D or ID or QD
	     *******************************************************************/
	    if ((n = sscanf(tail, "%1[XD]%5d%127s", xd, &ieStart, tail)) == 1 && *xd == 'D') {
		for (iq = iqStart; iq <= iqEnd; iq++) {	/* D dummy found - uses up one PiFace */
		    if (unitA[iq]++ >= iC_npf) {		/* maximum unassigned unit found so far */
			fprintf(iC_errFP, "ERROR: %s: 'too many PiFace dummy IEC arguments (%d max)\n", iC_progname, iC_npf);
			errorFlag++;
		    }
		}
		goto endOneArg;
	    }
	    if (n >= 2 && *xd == 'X') {
		ieEnd = ieStart;			/* assume a single Xn IEC */
		*iqe = *iqc;
		if (n == 2) {
		    *tail = '\0';			/* single IEC argument without -<i> or + or :pfa */
		    goto check_opt_G;
		}
		if (*tail == '+') {			/* extra I/O for GPIOB+ */
		    if (iqExtra == 0) goto illFormed;	/* GPIOB+ extra I/O must be 1 for 'I' or 2 for 'Q'*/
		    if ((n = sscanf(tail, "+%16[0-9xa-fXA-F]%126s", buffer, tail)) > 0) {
			char * endptr;
			mask = convert_nr(buffer, &endptr);
			if (mask > 0xff || *endptr != '\0') {
			    goto illFormed;		/* not a proper number or >= 8 */
			} else if (mask == 0) {
			    fprintf(iC_errFP, "WARNING: %s: Useless!! No bits selected in %s - not registered\n", iC_progname, *argp);
			}
			if (n == 1) *tail = '\0';
		    } else {
			memmove(tail, tail+1, 126);	/* mask == 0xff by default */
		    }
		    if (*tail != ':') goto check_opt_G;
		}
		if (*tail == '-') {			/* range or instance alternatives start with '-' */
		    /********************************************************************
		     *  PiFace IEC arguments with optional range and :pfa
		     *******************************************************************/
		    if ((n = sscanf(tail, "-%1[IQ]%126s", iqe, tail+1)) == 2 && *iqc != *iqe) {	/* skip '-' */
			goto illFormed;			/* fails if first Xn did not have I or Q */
		    }
		    if ((n = sscanf(tail+1, "X%5d%127s", &ieEnd, tail)) == 0) {
			ieEnd = ieStart;		/* a single Xn-i IEC with -instance and no :pfa */
			goto check_opt_G;
		    }
		    if (iqExtra == 2) goto illFormed;	/* GPIOB+ extra output may not have a range */
		    if (ieEnd < ieStart) {		/* *tail remains '-' */
			fprintf(iC_errFP, "ERROR: %s: '%s' is not a positive IEC range\n", iC_progname, *argp);
			errorFlag++;
			goto endOneArg;
		    }
		    if (n == 1) {
			*tail = '\0';			/* a range Xn-Xm without instance or :pfa */
			goto check_opt_G;
		    }
		    if (*tail == '-') {
			goto check_opt_G;		/* a range Xn-Xm with -instance */
		    }					/* a range Xn-Xm with :pfa */
		}
		if (*tail == ':') {
		    if ((n = sscanf(tail, ":%hu%127s", &pfa, tail)) >= 1) {
			if (pfa >= MAXPF) {
			    goto illFormed;
			}
		    } else {
			pfa = 0xffff;			/* no pfa assigned */
		    }
		    if (n <= 1) *tail = '\0';
		  check_opt_G:
		    if (iC_opt_G) {
			fprintf(iC_errFP, "ERROR: %s: '%s' no PiFace IEC arguments allowed with option -G\n", iC_progname, *argp);
			errorFlag++;
			goto endOneArg;
		    }
		    if (iC_debug & 0200) fprintf(iC_outFP, "%sX%d-%sX%d:%hu%s\n",
			iqc, ieStart, iqe, ieEnd, pfa, iids);
		    directFlag |= PF | DR;
		} else {
		    /********************************************************************
		     *  GPIO IEC arguments with .<bit>,gpio or ,gpio,gpio,...
		     *******************************************************************/
		    if (iqStart != iqEnd || ieStart != ieEnd || iqExtra == 2) {
			goto illFormed;		/* GPIO argument must be single IXn or QXn */
		    }
		    if (*tail == '.') {
			if (sscanf(tail, ".%hu%127s", &bit, tail) != 2 || bit >= 8 || *tail == '-') {
			    goto illFormed;		/* must have tail for gpio number */
			}
			bitStart = bitEnd = bit;	/* only 1 bit processed */
		    } else {
			bitStart = 0;
			bitEnd   = 7;		/* process all 8 bits */
		    }
		    for (bit = 0; bit <= 7; bit++) {
			gpioSav[bit] = 0xffff;	/* initialise empty Save list */
		    }
		    for (bit = bitStart, n = 2; bit <= bitEnd && n == 2; bit++) {
			if (*tail == '-') goto checkInstance;
			*dum = '\0';
			gpio = 0;
			if (((n = sscanf(tail, ",%hu%127s", &gpio, tail)) < 1 &&
			    (n = sscanf(tail, ",%1[d]%127s", dum, tail)) < 1) ||
			    gpio > 55) {
			    goto illFormed;
			}
			if (n == 1) *tail = '\0';
			directFlag |= GP | DR;
			if (*dum == '\0') {
			    gpioSav[bit] = gpio;	/*  Save list of GPIO numbers for later building */
			    if (iC_debug & 0200) fprintf(iC_outFP, "%sX%d.%hu	gpio = %hu\n",
				iqc, ieStart, bit, gpio);
			}
		    }
		    if (n == 2 && *tail == ',') {
			fprintf(iC_errFP, "ERROR: %s: '%s' gpio list limited to 8 comma separated numbers or 1 if .bit\n", iC_progname, *argp);
			errorFlag++;
			goto endOneArg;
		    }
		}
	      checkInstance:
		if (*tail == '-') {
		    if (n != 1 && (n = sscanf(tail, "-%3hu%127s", &iid, tail)) == 1) {	/* individual instance has precedence */
			snprintf(iids, 6, "-%hu", iid);	/* instance "-0" to "-999" */
		    } else {
			goto illFormed;
		    }
		} else if (*tail != '\0') {
		    goto illFormed;			/* a tail other than -instance eg -999 */
		}
	    } else {					/* n == 0 oe 1 or *xd != 'X' */
	      illFormed:
		fprintf(iC_errFP, "ERROR: %s: '%s' is not a well formed IEC\n", iC_progname, *argp);
		errorFlag++;
		goto endOneArg;
	    }
	    /********************************************************************
	     *  Use the IEC argument to extend PiFace or GPIO control structures
	     *  Check IEC argument was not used before
	     *******************************************************************/
	    if (directFlag & PF) {
		assert((directFlag & GP) == 0);		/* should not have both */
		int		ie;
		/********************************************************************
		 *  Build one or more PiFace control structures iC_pfL[un]
		 *      ieStart - ieEnd covers range of PiFace IEC argument (-1 for Dummy)
		 *      iqStart - iqEnd 0 = 'I', 1 = 'Q'
		 *  temporarily store ieStart (IEC number) in val of iqDetails
		 *  temporarily store iid (instance) in channel of iqDetails
		 *  pfa is 0 - 7 or 0xffff if not yet defined - select a iC_pfL[unit]
		 *******************************************************************/
		for (ie = ieStart; ie <= ieEnd; ie++) {
		    int		ie1;
		    for (iq = iqStart; iq <= iqEnd; iq++) {
			int	em;
			for (un = 0; un < iC_npf; un++) {
			    if ((ie1 = iC_pfL[un].s[iq].val) == ie &&
				ie >= 0 &&	/* can have any number of dummies on same pfa selection */
				iC_pfL[un].s[iq].channel == iid) {
				fprintf(iC_errFP, "ERROR: %s: PiFace IEC %cX%d%s used more than once\n",
				    iC_progname, IQ[iq], ie, iids);
				errorFlag++;
				goto endOneArg;
			    }
			    if (pfa != 0xffff && iC_pfL[un].pfa == pfa) {	/* not 0xffff and pfa found */
				if (ie1 < 0) {			/* no name for this I/O ? */
				    unit = un;			/* use unused iC_pfL[] entry with this pfa */
				    if (unit == unitA[iq]) {	/* is this the next available iC_pfL[] entry ? */
					unitA[iq]++;		/* step past it for use with direct pfa later */
				    }
				    if (unit > maxUnit) maxUnit = unit;
				    goto pfaFound;
				}
				fprintf(iC_errFP, "ERROR: %s: PiFace %hu used more than once with %cX%d%s and %cX%d%s\n",
				    iC_progname, pfa, IQ[iq], ie1, iids, IQ[iq], ie, iids);
				errorFlag++;
				goto endOneArg;
			    }
			}
			if (pfa == 0xffff) {
			    do {
				if ((unit = unitA[iq]++) >= iC_npf) {	/* maximum unassigned unit found so far */
				    fprintf(iC_errFP, "ERROR: %s: '%d is too many PiFace IEC arguments (%d max)\n", iC_progname, unit+1, iC_npf);
				    errorFlag++;
				    goto endOneArg;
				}
			    } while (iC_pfL[unit].s[iq].val >= 0);	/* use the next unused pfa entry */
			    if (unit > maxUnit) maxUnit = unit;
			} else {
			    fprintf(iC_errFP, "ERROR: %s: PiFace %hu named in %cX%d:%hu%s is not available\n",
				iC_progname, pfa, IQ[iq], ie, pfa, iids);
			    errorFlag++;
			    goto endOneArg;
			}
		      pfaFound:
			for (gep = iC_gpL[iq & 0x01]; gep; gep = gep->nextIO) {	/* check IEC not used for GPIO */
			    if (gep->Gval == ie &&
				gep->Gchannel == iid) {
				fprintf(iC_errFP, "ERROR: %s: PiFace IEC %cX%d:%hu%s has already been used as a GPIO IEC\n",
				    iC_progname, IQ[iq], ie, pfa, iids);
				errorFlag++;
				goto endOneArg;
			    }
			}
			pfp = &iC_pfL[unit];
			pfq = &(pfp->s[iq]);
			pfq->inv = invMask ^ invFlag;	/* by default do not invert PiFace inputs and Port B outputs */
			if (iq == 0) {
			    pfq->inv ^= 0xff;		/* by default invert PiFace Port A outputs - inverted again with -I or '~' */
			} else
			if ((em = mask & pfp->s[iq^0x03].bmask)) {	/* IXn or QXn+ */
			    unsigned short	iid1;
			    char		iids1[6];
			    *iids1 = '\0';		/* instance string of alternate I/O */
			    if ((iid1 = pfp->s[iq^0x03].channel) != 0xffff) {
				snprintf(iids1, 6, "-%hu", iid1);	/* instance "-0" to "-999" */
			    }
			    fprintf(iC_errFP, "WARNING: %s: %cX%d+0x%02x:%hu%s conflicts with %cX%d+0x%02x:%hu%s in bits 0x%02x - ignore these bits\n",
				iC_progname, IQ[iq], ie, mask, pfa, iids,
				IQ[iq^0x03], pfp->s[iq^0x03].val, pfp->s[iq^0x03].bmask, pfa, iids1, em);
			    mask &= ~em;		/* clear conflicting bits */
			}
			pfq->inv &= mask;		/* masked bits are never inverted */
			pfq->bmask = mask;		/* bmask may be 0, which means it is not registered */
			pfq->val = ie;			/* valid IEC argument Xn (iq in index) */
			pfq->channel = iid;		/* argument instance or 0xffff */
		    }
		    if (pfa < MAXPF) {
			pfa++;					/* takes care of range:pfa */
		    } else {
			pfa = 0xffff;
		    }
		}
	    } else if (directFlag & GP) {
		/********************************************************************
		 *  Build or extend a new gpioIO structure and store in iC_gpL[iqStart]
		 *      iqStart 0 = 'Q', 1 = 'I'
		 *  temporarily store ieStart (IEC number) in val of iqDetails
		 *  temporarily store iid (instance) in channel of iqDetails
		 *  Check that the GPIO for the IEC argument is valid for this RPi
		 *  and was not used before in this or another app.
		 *******************************************************************/
		gpioLast = &iC_gpL[iqStart];	/* scan previous gpio list */
		for (gep = iC_gpL[iqStart]; gep; gep = gep->nextIO) {
		    if (gep->Gval == ieStart &&
			gep->Gchannel == iid) {
			goto previousElement;	/* already partially filled */
		    }
		    gpioLast = &gep->nextIO;
		}
		for (un = 0; un < iC_npf; un++) {	/* check IEC not used for PiFace */
		    if (iC_pfL[un].s[iqStart].val == ieStart &&
			iC_pfL[un].s[iqStart].channel == iid) {
			fprintf(iC_errFP, "ERROR: %s: GPIO IEC %cX%d%s has already been used as a PiFace IEC\n",
			    iC_progname, IQ[iqStart], ieStart, iids);
			errorFlag++;
			goto endOneArg;
		    }
		}
		*gpioLast = gep = iC_emalloc(sizeof(gpioIO));	/* new gpioIO element */
		gep->Ggate = NULL;		/* GPIO IEC in or output not active */
		gep->Gbmask = 0;
		gep->Gval = ieStart;
		gep->Gchannel = iid;		/* temporary IEC details in new element */
		for (bit = 0; bit <= 7; bit++) {
		    gep->gpioNr[bit] = 0xffff;	/* mark gpio number unused */
		    gep->gpioFN[bit] = -1;	/* mark file number unused */
		}
	      previousElement:
		for (bit = 0; bit <= 7; bit++) {
		    if((gpio = gpioSav[bit]) != 0xffff) {
			mask = iC_bitMask[bit];
			assert(gpio <= 55);
			gpioMask = 1LL << gpio;		/* gpio is 0 - 55 max */
			if (iC_debug & 0200) fprintf(iC_outFP,
			    "gpio     = %hu\n"
			    "used     = 0x%016llx\n"
			    "gpioMask = 0x%016llx\n",
			    gpio, gpiosp->u.used, gpioMask);
			if ((u = gep->gpioNr[bit]) != 0xffff &&
			    ((mask & gep->Ginv) != (mask & invFlag) ||
			    (u != gpio))) {		/* ignore 2nd identical definition */
			    fprintf(iC_errFP, "ERROR: %s: %c%sX%d.%hu,%hu%s cannot override %c%sX%d.%hu,%hu%s\n",
				iC_progname,
				invFlag & mask ? '~' : ' ', iqc, ieStart, bit, gpio, iids,
				gep->Ginv & mask ? '~' : ' ', iqc, ieStart, bit, gep->gpioNr[bit], iids);
			    errorFlag++;
			} else if (!(gpiosp->valid & gpioMask)) {
			    fprintf(iC_errFP, "ERROR: %s: trying to use invalid gpio %hu on %sX%d.%hu%s on RPi board rev %d\n",
				iC_progname, gpio, iqc, ieStart, bit, iids, gpiosp->proc);
			    errorFlag++;
			} else if (gpiosp->u.used & gpioMask) {
			    fprintf(iC_errFP, "ERROR: %s: trying to use gpio %hu a 2nd time on %c%sX%d.%hu%s\n",
				iC_progname, gpio, invFlag & mask ? '~' : ' ', iqc, ieStart, bit, iids);
			    errorFlag++;
			} else {
			    gep->Ginv |= mask & invFlag;/* by default do not invert GPIO in or outputs - inverted with -I or '~' */
			    gpiosp->u.used |= gpioMask;	/* mark gpio used for ~/.iC/gpios.rev */
			    ownUsed        |= gpioMask;	/* mark gpio in ownUsed for termination */
			    gep->gpioNr[bit] = gpio;	/* save gpio number for this bit */
			    gep->Gbmask |= iC_bitMask[bit];	/* OR hex 01 02 04 08 10 20 40 80 */
			    /********************************************************************
			     *  Check if GPIO number is used by w1-gpio kernel module to drive iCtherm
			     *  If yes and bit 0 (OopsMask) has not been set in gpios->u.oops before
			     *  remove w1-therm and w1-gpio and set bit 0 to block iCtherm
			     *******************************************************************/
			    if (gpio == gpioTherm) {
				strncpy(buffer, "sudo modprobe -r w1-therm w1-gpio", BS);	/* remove kernel modules */
				if (iC_debug & 0200) fprintf(iC_outFP, "%s\n", buffer);
				if ((b = system(buffer)) != 0) {
				    perror("sudo modprobe");
				    fprintf(iC_errFP, "WARNING: %s: system(\"%s\") could not be executed $? = %d - ignore\n",
					iC_progname, buffer, b);
				}
				gpiosp->u.oops |= OopsMask;	/* block iCtherm permanently */
			    }
			}
		    }
		}
	    }
	  endOneArg:
	    invFlag = 0x00;				/* start new tilde '~' analysis for next argument */
	  skipInvFlag:;
	}
      UnlockGpios:
	if (iC_debug & 0200) fprintf(iC_outFP, "used     = 0x%016llx\n"
					       "oops     = 0x%016llx\n", gpiosp->u.used, gpiosp->u.oops);
	if (writeUnlockCloseGpios() < 0) {
	    fprintf(iC_errFP, "ERROR: %s: in writeUnlockCloseGpios()\n",
		iC_progname);
	    errorFlag++;
	}
    }
  FreeNow:
    free(argip);
#endif	/* ! RASPBERRYPI */
    if (errorFlag) {
	iC_quit(-3);					/* after all the command line has been analyzed */
    }

/********************************************************************
 *
 *  Scan iC_list[] for PASS 0. Each entry contains a Gate** to the
 *  Gates in each separately compiled module. These Gates are linked
 *  via gt_next in the compiled code for each module.
 *
 *  PASS 0
 *
 *  Count all nodes to allocate a symbol table array sTable which
 *  is sorted in INTERLUDE 0. sTable is used for linear scans for all
 *  later initialisation and for finding nodes by name for debugging.
 *  It is important to have one list for multiple sources, so ALIAS's
 *  in different sources are resolved in the early passes.
 *
 *  Transfer timer preset value in gt_mark to gt_old for run time.
 *  NCONST and INPW nodes have gt_val initialised to -1.
 *  Clear all other gt_val, gt_old and all gt_mark entries except
 *  ALIAS nodes which hold inversion flag in gt_mark.
 *
 *  Generate an INPW/TRAB Gate IXx for each first IXx.y of gt_ini -INPX
 *  Generate an INPW/TRAB Gate TXx for each first TXx.y of gt_ini -INPX
 *  Generate an INPB/OUTW Gate QXx for each first QXx.y_0 of gt_fni OUTX
 *  These new Gates are linked in an extra list e_list, which can be scanned
 *  reasonably quickly for duplicates. At the end of Pass 0, the extra
 *  e_list is linked into the start of the first entry in iC_list[], so the
 *  newly generated Gates are also scanned in Pass 1 and 2 and then sorted.
 *
 *  For RASPBERRYPI direct PiFace and GPIO I/O the entries in e_list
 *  (except the TX0 entry) are the only candidates for such direct I/O
 *
 *******************************************************************/

    if (df) { fprintf(iC_outFP, "PASS 0\n"); fflush(iC_outFP); }
    val = e_cnt = 0;
    for (oppp = iC_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    val++;					/* count node */
	    op->gt_val = (op->gt_ini == -NCONST	|| op->gt_ini == -INPW) ? -1 : 0;
	    if (op->gt_ini != -ALIAS) {
		op->gt_old = (op->gt_fni == TIMRL) ? op->gt_mark : 0;
		if ((op->gt_ini != -ARNC && op->gt_ini != -LOGC) || op->gt_fni != UDFA) {
		    op->gt_mark = 0;
		}
		/********************************************************************
		 * handle bit I/O
		 *******************************************************************/
		if (op->gt_ini == -INPX || op->gt_fni == OUTX) {
		    strncpy(eBuf, op->gt_ids, ESIZE);
		    cp = strchr(eBuf, '.');
		    i = sscanf(eBuf, "%1[IQT]X%5d.%5hu%7s", iqt, &byte, &bit, tail);
		    tgp = 0;				/* for correct inError message */
		    if (cp && bit < 8) {
			*cp = '\0';			/* terminate extra name */
			len = cp - eBuf + 1;
			for (tgp = e_list; tgp != 0; tgp = tgp->gt_next) {
			    if (strcmp(tgp->gt_ids, eBuf) == 0) {
				goto linkIO;		/* previously generated */
			    }
			}
			/* generate a new auxiliary Gate for bit I/O and put it on e_list */
			tgp = (Gate *) iC_emalloc(sizeof(Gate)); /* all bytes 0, gt_live == External */
			tgp->gt_ids = iC_emalloc(len);
			strncpy(tgp->gt_ids, eBuf, len);
			if ((tgp->gt_next = e_list) == 0) {	/* link rest of e_list to new Gate */
			    ttgp = tgp;			/* last entry in new e_list for linking rest */
			}
			e_list = tgp;			/* new Gate at front for speed */
			e_cnt++;			/* count e_list node */
			if (op->gt_ini == -INPX) {
			    tgp->gt_ini = -INPW;
			    tgp->gt_fni = TRAB;
			    tgp->gt_list = (Gate **) iC_emalloc(8 * sizeof(Gate *));
			} else {			/* (op->gt_fni == OUTX) */
			    tgp->gt_ini = -INPB;
			    tgp->gt_fni = OUTW;
			}
		      linkIO:
			if (op->gt_ini == -INPX) {
			    if (i != 3 || op->gt_rlist != 0 || strcmp(iqt, "Q") == 0) goto pass0Err;
			    tgp->gt_list[bit] = op;	/* pointer to bit Gate */
			    /* ###### no back link ####### */
			} else {			/* (op->gt_fni == OUTX) */
			    if (i != 4 || strcmp(tail, "_0") != 0 || strcmp(iqt, "Q") != 0) goto pass0Err;
			    op->gt_mark = iC_bitMask[bit];
			    op->gt_ptr = tgp;		/* direct pointer (union with gt_list) */
			    /* ###### no back link ####### */
			}
			tgp->gt_mark |= iC_bitMask[bit];	/* note used bits for registration */
		    } else {
		      pass0Err:
			inError(__LINE__, op, tgp, "PASS 0: invalid bit I/O Gate");
		    }
		}
	    }
	}
    }
    if (errCount) {
	exit(1);					/* pass 0 failed */
    }
#ifdef	RASPBERRYPI
    if (iC_opt_P) {
	/********************************************************************
	 *  Analyze I/O variables found in the command line and the current
	 *  application
	 *
	 *  Associate bit variables with iC_pfL[] and/or iC_gpL[] entries
	 *  generated in the command line analysis or generate new entries
	 *  if not found in iC_pfL[] or iC_gpL[] lists.
	 *
	 *  Possible outcomes for all variables:
	 *  1)  Bit variables which have a PFI or GPIO entry and which are
	 *      confirmed here as being used in the application and whose
	 *      instance matches the global instance are marked as 'Internal'.
	 *  2)  Bit variables which have a PFI or GPIO entry and which are not
	 *      confirmed here as being used in the application or whose
	 *      instance does not match the global instance are marked 'ExtOut'.
	 *  3)  New bit variables - generate a PFI entry - mark as 'Internal'
	 *      unless their iC_pfL index would exceed iC_npf, in which case
	 *      they are NOT associated with a PFI entry - mark as 'External'
	 *	NOTE: All GPIO entries must appear as a command line argument.
	 *  4)  Bit and int variables which have no PFI or GPIO entry are 'External'
	 *      by default. They are handled by iCserver in the normal way.
	 *  Since PiFaces have already been counted iC_npf is known and PFI entries
	 *  are correctly filled with argument IECs or NULL
	 *
	 *  Use marking values in gt_live to influence registration:
	 *
	 *  0   External    // default  IXx <== iCserver
	 *		    //		        iCserver <== QXx
	 *
	 *  1   Internal    //          IXx <== SIO/GPIO
	 *		    //		        SIO/GPIO <== QXx
	 *		    // if opt_E    also iCserver <== IXx (display only)
	 *		    //             and  iCserver <== QXx
	 *
	 *  2   Dummy	    // ignore -    output WARNING    ***
	 *
	 *  3   ExtOut      // like iCpiFace    iCserver <== SIO/GPIO (IXx)
	 *		    //		        SIO/GPIO <== iCserver (QXx)
	 *
	 *  Generate IEC names for all arguments which have a PiFace and a
	 *	non-zero bmask. PiFace arguments with zero bmask are not registered.
	 *  Do this here because instance was not yet determind when IEC name
	 *  was found
	 *******************************************************************/
	for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
	    for (iq = 0; iq < 3; iq++) {
		pfq = &(pfp->s[iq]);
		if (pfq->bmask && (n = pfq->val) >= 0) {	/* valid IEC argument Xn ? */
		    len = snprintf(buffer, BS, "%cX%d", IQ[iq], n);	/* IEC name without instance */
		    if ((iid = pfq->channel) != iidN) {
			/********************************************************************
			 *  Instance does not match instance of this executable - ExtOut
			 *  If there is a variable in e_list with the same name it is a
			 *  different instance and not to be confused with the local instance.
			 *******************************************************************/
			goto GenerateExtOut;
		    }
		    /********************************************************************
		     *  Instance matches - internal IEC name never has an instance extension
		     *******************************************************************/
		    for (tgp = e_list; tgp != NULL; tgp = tgp->gt_next) {
			if (strcmp(buffer, tgp->gt_ids) == 0) {
			    tgp->gt_live = Internal;
			    goto NextVariable;
			}
		    }
		    /********************************************************************
		     *  Command line variable not used in this application - ExtOut
		     *  Generate a new variable to use for communication with iCserver
		     *******************************************************************/
		  GenerateExtOut:
		    if (iid != 0xffff) {		/* append possible global instance */
			len = snprintf(buffer, BS, "%cX%d-%hu", IQ[iq], n, iid);
		    }
		    /* generate a new auxiliary Gate for bit I/O and also put it on e_list */
		    tgp = (Gate *) iC_emalloc(sizeof(Gate)); /* all bytes 0, gt_live == External */
		    tgp->gt_ids = iC_emalloc(++len);
		    strncpy(tgp->gt_ids, buffer, len);
		    if ((tgp->gt_next = e_list) == 0) {	/* link rest of e_list to new Gate */
			ttgp = tgp;			/* last entry in new e_list for linking rest */
		    }
		    e_list = tgp;			/* new Gate at front for speed */
		    e_cnt++;				/* count e_list node */
		    if (iq == 1) {			/* IN */
			tgp->gt_ini = -INPW;
			tgp->gt_fni = TRAB;		/* tgp->gt_list is left NULL */
		    } else {				/* (iq == 0 || iq == 2) OUT */
			tgp->gt_ini = -INPB;
			tgp->gt_fni = OUTW;
		    }
		    tgp->gt_live = ExtOut;
		  NextVariable:
		    assert(tgp && (tgp->gt_mark & PG_MASK) == 0);
		    tgp->gt_mark |= (iq < 2) ? P_FLAG : PB_FLAG;	/* PiFace device Gate */
		    tgp->gt_pfp = pfp;			/* store pfp * till registration - then gt_channel */
		    pfq->i.gate = tgp;			/* store whole Gate rather than just the name */
		    if (iC_debug & 0200) {
			if (iq == 0) {			/* Port A output */
			    fprintf(iC_outFP, "### store iC_pfL[%d][0]	%c%s\n", pfp - iC_pfL, pfp->Qinv ? ' ' : '~', tgp->gt_ids);
			} else if (iq == 1) {		/* Port B input */
			    fprintf(iC_outFP, "### store iC_pfL[%d][1]	%c%s\n", pfp - iC_pfL, pfp->Iinv ? '~' : ' ', tgp->gt_ids);
			} else {			/* Port B output */
			    fprintf(iC_outFP, "### store iC_pfL[%d][2]	%c%s\n", pfp - iC_pfL, pfp->QPinv ? '~' : ' ', tgp->gt_ids);
			}
		    }
		}
	    }
	}
	/********************************************************************
	 *  Generate IEC names for all GPIO output and input arguments
	 *  Do the same for iC_gpL[0] and iC_gpL[1]
	 *******************************************************************/
	for (iq = 0; iq < 2; iq++) {
	    for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
		if ((n = gep->Gval) >= 0) {	/* valid IEC argument IXn QXn ? */
		    len = snprintf(buffer, BS, "%cX%d", IQ[iq], n);	/* IEC name without instance */
		    if ((iid = gep->Gchannel) != iidN) {
			/********************************************************************
			 *  Instance does not match instance of this executable - ExtOut
			 *  If there is a variable in e_list with the same name it is a
			 *  different instance and not to be confused with the local instance.
			 *******************************************************************/
			goto GenerateExtOutG;
		    }
		    /********************************************************************
		     *  Instance matches - internal IEC name never has an instance extension
		     *******************************************************************/
		    for (tgp = e_list; tgp != NULL; tgp = tgp->gt_next) {
			if (strcmp(buffer, tgp->gt_ids) == 0) {
			    tgp->gt_live = Internal;
			    goto NextVariableG;
			}
		    }
		    /********************************************************************
		     *  Command line variable not used in this application - ExtOut
		     *  Generate a new variable to use for communication with iCserver
		     *******************************************************************/
		  GenerateExtOutG:
		    if (iid != 0xffff) {		/* append possible global instance */
			len = snprintf(buffer, BS, "%cX%d-%hu", IQ[iq], n, iid);
		    }
		    /* generate a new auxiliary Gate for bit I/O and also put it on e_list */
		    tgp = (Gate *) iC_emalloc(sizeof(Gate)); /* all bytes 0, gt_live == External */
		    tgp->gt_ids = iC_emalloc(++len);
		    strncpy(tgp->gt_ids, buffer, len);
		    if ((tgp->gt_next = e_list) == 0) {	/* link rest of e_list to new Gate */
			ttgp = tgp;			/* last entry in new e_list for linking rest */
		    }
		    e_list = tgp;			/* new Gate at front for speed */
		    e_cnt++;				/* count e_list node */
		    if (iq == 0) {			/* OUT */
			tgp->gt_ini = -INPB;
			tgp->gt_fni = OUTW;
		    } else {				/* (iq == 1) IN */
			tgp->gt_ini = -INPW;
			tgp->gt_fni = TRAB;		/* tgp->gt_list is left NULL */
		    }
		    tgp->gt_live = ExtOut;
		  NextVariableG:
		    assert(tgp && (tgp->gt_mark & PG_MASK) == 0);
		    tgp->gt_mark |= G_FLAG;		/* GPIO group Gate */
		    tgp->gt_gep = gep;			/* store gep * till registration - then gt_channel */
		    gep->Ggate = tgp;			/* store whole Gate rather than just the name */
		    if (iC_debug & 0200) fprintf(iC_outFP, "### store iC_gpL[%d]	%s\n", iq, tgp->gt_ids);
		}
	    }
	}
	/********************************************************************
	 *  Check for variables used in this application which have not been
	 *  marked in the above scan - these are new variables - Internal
	 *  Generate extra iC_pfL[] entries for these new Gates until PiFaces
	 *  run out.
	 *  Sort e_list first, to associate lower IEC numbered bit variables
	 *  with lower addressed PiFaces. (qsort requires early sTable)
	 *
	 *  pfp points to remaining iC_pfL[] entries (not used here).
	 *  unitA[0], initA[1] (maxUnit+1) are the starting indeces for the
	 *  remaining iC_pfL[] entries, which can be filled independently (I, Q).
	 *  This is necessary, because all the IXx variables come before
	 *  all the QXx variables after sorting with qsort().
	 *  NOTE: PiFace Port B outputs (iq == 2) are not set up automatically.
	 *******************************************************************/
	assert(maxUnit < MAXPF);
	unitA[0] = unitA[1] = maxUnit + 1;	/* unused PiFace(s) beyond maxUnit */
	val += e_cnt + 1;			/* count e_list and iClock */
	sTable = sTend = (Gate **)calloc(val, sizeof(Gate *));	/* node* */
	for (tgp = e_list; tgp != NULL; tgp = tgp->gt_next) {
	    *sTend++ = tgp;			/* enter node into sTable early */
	}
	qsort(sTable, e_cnt, sizeof(Gate*), (iC_fptr)iC_cmp_gt_ids);	/* sort I/O nodes only */
	for (opp = sTable; opp < sTend; opp++) {
	    tgp = *opp;
	    if (tgp->gt_live == External &&	/* ignore previously marked or TX0 IB1 QW2_0 etc */
		sscanf(tgp->gt_ids, "%1[IQ]X%5d%31s", iqc, &n, tail) == 2 &&
		(unit = unitA[iq = (*iqc == 'I')]++) < iC_npf) {	/* rest remain External */
		pfp = &iC_pfL[unit];
		assert(pfp->pfa != 0xffff);/* by only going up to iC_npf we should not strike uninitialised Pifaces */
		tgp->gt_live = Internal;
		assert((tgp->gt_mark & PG_MASK) == 0);
		tgp->gt_mark |= P_FLAG;		/* PiFace device Gate - no automatic Port B output */
		tgp->gt_pfp = &iC_pfL[unit];	/* store pfp* till registration - then gt_channel */
		pfp->s[iq].i.gate = tgp;	/* store whole Gate rather than just the name */
		if (iq == 1) {
		    pfp->Iinv = invMask;	/* by default do not invert PiFace inputs - inverted with -I or '~' */
		    if (iC_debug & 0200) fprintf(iC_outFP, "### store iC_pfL[%d][1]	%c%s\n", unit, pfp->Iinv ? '~' : ' ', tgp->gt_ids);
		} else {
		    pfp->Qinv = invMask ^ 0xff; /* by default invert PiFace outputs - inverted again with -I or '~' */
		    if (iC_debug & 0200) fprintf(iC_outFP, "### store iC_pfL[%d][0]	%c%s\n", unit, pfp->Qinv ? ' ' : '~', tgp->gt_ids);
		}
		directFlag |= PF | DR;
	    }
	}
	/********************************************************************
	 *  Clear channel members in iC_pfL[] entries
	 *******************************************************************/
	for (pfp = iC_pfL; pfp < &iC_pfL[MAXPF]; pfp++) {
	    for (iq = 0; iq < 3; iq++) {
		pfq = &(pfp->s[iq]);
		pfq->val = 0;			/* restore values */
		pfq->channel = 0;
	    }
	}
	/********************************************************************
	 *  Do the same for iC_gpL[0] and iC_gpL[1] - finalise Ginv
	 *******************************************************************/
	for (iq = 0; iq < 2; iq++) {
	    for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
		assert(gep->Ggate);		/* later scans rely on a gate */
		gep->Ginv ^= invMask;		/* by default do not invert GPIO in or outputs - inverted with -I or '~' */
		gep->Gval = 0;			/* restore values */
		gep->Gchannel = 0;
	    }
	}
	if (directFlag == 0) {
	    iC_opt_P = 0;			/* no IEC arguments for direct I/O - network I/O only */
	}
    } else
#endif	/* RASPBERRYPI */

/********************************************************************
 *
 *  INTERLUDE 0
 *
 *  Allocate space for the symbol table array.
 *
 *  Enter each node in iC_list and iClock into symbol table sTable.
 *  At the end of this pass sTend will hold the end of the table.
 *
 *  The length of a pointer array for such a symbol table is
 *  sTend - sTable.
 *
 *  Sort the symbol table in order of gt_ids.
 *
 *******************************************************************/

    {	/* ! iC_opt_P or ! RASPBERRYPI */
	if (e_list) {
	    ttgp->gt_next = *iC_list[0];	/* link iClist[0] to last entry in e_list  */
	    *iC_list[0] = e_list;		/* usually a non-null entry - works even if not */
	}
	val += e_cnt + 1;			/* count e_list and iClock */
	sTable = sTend = (Gate **)calloc(val, sizeof(Gate *));	/* node* */
    }
    *sTend++ = &iClock;				/* enter iClock into sTable */

    for (oppp = iC_list; (opp = *oppp++) != 0; ) {
	for (op = *opp; op != 0; op = op->gt_next) {
	    *sTend++ = op;				/* enter node into sTable */
	}
    }

    if (val != (sTend - sTable)) {
	fprintf(iC_errFP,
	    "\nERROR: %s: line %d: Gate allocation %d vs Symbol Table size %d\n",
	    __FILE__, __LINE__, val, (int)(sTend - sTable));
	exit(2);
    }

    qsort(sTable, val, sizeof(Gate*), (iC_fptr)iC_cmp_gt_ids);	/* sort the whole symbol table */

/********************************************************************
 *
 *  All the activity lists for action nodes are already correctly
 *  compiled and do not need to be altered except for a link from
 *  the SH slave gate to the corresponding delay master in gt_rlist.
 *
 *  They are now also joined and sorted. Resolve all ALIAS'es now.
 *
 *  PASS 1
 *
 *  Determine how much space is required for the output activity
 *  lists for ARITH and GATE nodes.
 *
 *  For each ARN and LOGIC node op accumulate count of outputs in
 *  gp->gt_mark, where gp is each of the nodes which is input to op.
 *  Also accumulate total count in link_count and the input count
 *  in op->gt_val for a consistency check. For C parameter values and
 *  timer delay references (these are ARITH nodes) accumulate the count
 *  in gp->gt_old without also accumulating a link_count, since there is
 *  no link. If a C parameter is assigned, which means it has an input
 *  in the C code, accumulate that count in gt_val. Such a C parameter
 *  must be of type ARNC or LOGC, which is checked.
 *
 *  For each ARITH and GATE node op, which is not an ALIAS, count
 *  1 extra link for ARITH nodes and 2 extra links for GATE nodes.
 *  These are space for the activity list terminators.
 *
 *  Resolve inputs from ARITH and GATE ALIAS nodes and OUTW nodes,
 *  which may occurr because of multiple modules linked together.
 *  In particular adjust each member of an ARITH ALIAS chain, so that
 *  it points to the final input. This is needed for execution of
 *  cexe_n() functions, where inputs can only resolve 1 level of ALIAS.
 *
 *  After this pass the input lists contain no aliases.
 *
 *******************************************************************/

    if (df) { fprintf(iC_outFP, "PASS 1 - name gt_ini gt_fni: input list\n"); fflush(iC_outFP); }
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	/********************************************************************
	 *  part A
	 *  arithmetic node or logical output
	 *******************************************************************/
	if (op->gt_ini == -ARN) {
	    if (df) fprintf(iC_outFP, " %-8s%6s%7s:", op->gt_ids,
			iC_full_type[-op->gt_ini], iC_full_ftype[op->gt_fni]);
	    if ((lp = op->gt_rlist) == 0) {
		inError(__LINE__, op, 0, "PASS 1: arithmetic node with no input list");
		goto partB;
	    }
	    lp++;					/* skip function vector (may be 0) */
	    while ((gp = *lp++) != 0) {			/* for ARN scan 1 list */
		if (gp->gt_ini == -ALIAS) {		/* resolve arithmetic ALIAS */
		    tgp = gp;				/* remember start of ALIAS chain */
		    while (gp->gt_ini == -ALIAS) {
			if (df) fprintf(iC_outFP, "	%s@", gp->gt_ids);
			gp = gp->gt_rptr;		/* adjust */
		    }
		    tlp = lp - 1;
		    *tlp = gp;				/* swap in real input */
		    /* adjust ALIAS chain, so that each ALIAS points to real input */
		    while (tgp->gt_ini == -ALIAS) {
			ttgp = tgp->gt_rptr;
			tgp->gt_rptr = gp;		/* point to final input gate */
			tgp = ttgp;
		    }
		}
		if (df) fprintf(iC_outFP, "	%s,", gp->gt_ids);
		if (op->gt_val < PPGATESIZE) {
		    op->gt_val++;			/* count input */
		}
		if (gp->gt_fni == ARITH) {
		    gp->gt_mark++;			/* arithmetic output at gp */
		    link_count++;
		} else
		if (gp->gt_fni != OUTX) {
		    inError(__LINE__, op, gp, "PASS 1: arithmetic node points back to non ARITH");
		    goto partB;
		}
	    }
	} else
	/********************************************************************
	 *  logical node
	 *******************************************************************/
	if (op->gt_ini <= -MIN_GT && op->gt_ini > -MAX_GT &&
	    op->gt_mcnt == 0) {				/* leave out _f0_1 or immC array */
	    if (df) {
		if (op->gt_ini < 0) {
		    fprintf(iC_outFP, " %-8s%6s%7s:", op->gt_ids,
			iC_full_type[-op->gt_ini], iC_full_ftype[op->gt_fni]);
		} else {
		    /* should no longer occurr because of above test */
		    fprintf(iC_outFP, "*** Warning: wrong gt_ini: %-8s%6d%7s:",
			op->gt_ids, op->gt_ini, iC_full_ftype[op->gt_fni]);
		}
	    }
	    if ((lp = op->gt_rlist) == 0) {
		inError(__LINE__, op, 0, "PASS 1: logic node with no input list");
		goto partB;
	    }
	    i = 1;					/* LOGIC nodes XOR, AND, OR or LATCH */
	    /* for LOGIC scan 2 lists with i = 1 and -1 */
	    do {
		while ((gp = *lp++) != 0) {
		    inversion = 0;			/* resolve logical ALIAS */
		    if (gp->gt_ini == -ALIAS) {
			while (gp->gt_ini == -ALIAS) {
			    if (df) fprintf(iC_outFP, "	%c%s@",		/* @ */
				inversion ? '~' : ' ', gp->gt_ids);
			    inversion ^= gp->gt_mark;
			    gp = gp->gt_rptr;
			}
			tlp = lp - 1;		/* destination */
			if (inversion) {
			    slp = tlp + i;		/* source */
			    if (i == 1) {
				while ((*tlp++ = *slp++) != 0);
				*tlp = gp;		/* swap in real input */
				lp--;
				continue;		/* counted as inverse */
			    }
			    while ((*tlp-- = *slp--) != 0);
			}
			*tlp = gp;			/* swap in real input */
		    }
		    if (df) fprintf(iC_outFP, "	%c%s,",
			((i >> 1) & 0x1) ^ inversion ? '~' : ' ', gp->gt_ids);
		    op->gt_val++;			/* count input */
		    if (gp->gt_fni == GATE) {
			gp->gt_mark++;		/* logic output at gp */
			link_count++;
		    } else {
			if (gp->gt_fni < MAX_FTY && gp->gt_ini > -MAX_LS) {
			    inError(__LINE__, op, gp, "PASS 1: logic node points to non GATE");
			} else {
			    inError(__LINE__, op, 0, "PASS 1: logic node points to non Gate struct");
			}
			fprintf(iC_errFP,
			    "ERROR %s: line %d: ftype = %d type = %d ???\n",
			    __FILE__, __LINE__, gp->gt_fni, gp->gt_ini);
			goto failed;		/* to avoid memory access error */
		    }
		}
	    } while ((i = -i) != 1);
	} else {
	    /********************************************************************
	     *  remaining types not touched in part A
	     *******************************************************************/
	    if (df) fprintf(iC_outFP, " %s\t\t\t\t\tlink count = %d\n", op->gt_ids, link_count);
	    goto partB;				/* rest not touched in in part A */
	}
      failed:
	if (df) fprintf(iC_outFP, "\t\tlink count = %d\n", link_count);
      partB:
	/********************************************************************
	 *  part B
	 *  count links
	 *******************************************************************/
	if (op->gt_ini != -ALIAS &&
	    (op->gt_fni == ARITH || op->gt_fni == GATE)) {
	    if (op->gt_list != 0) {
		inError(__LINE__, op, 0, "PASS 1: no forward list for ARITH or GATE");
	    }
	    link_count++;				/* 1 terminator for ARITH */
	    if (op->gt_fni == GATE) {
		link_count++;				/* 2 terminators for GATE */
	    }
	} else
	/********************************************************************
	 *  action nodes
	 *******************************************************************/
	if (op->gt_fni >= MIN_ACT && op->gt_fni < MAX_ACT) {
	    op->gt_mark++;				/* count self */
	    lp = op->gt_list;
	    if (op->gt_mcnt == 0) {			/* leave out _f0_1 or immC array */
		if (lp == 0 || (gp = *lp++) == 0) {
		    inError(__LINE__, op, 0, "PASS 1: no slave node or funct for action");
		} else
		if (op->gt_fni != F_SW && op->gt_fni != F_CF && op->gt_fni != F_CE) {
		    gp->gt_val++;			/* mark slave node */
		    if (op->gt_fni == D_SH) {
			if (gp->gt_rlist) {
			    inError(__LINE__, op, gp, "PASS 1: backlink for D_SH slave already installed");
			} else {
			    gp->gt_rptr = op;		/* back pointer to delay master */
			}
		    }
		}
		if ((gp = *lp++) == 0 || (gp->gt_fni != CLCKL && gp->gt_fni != TIMRL)) {
		    inError(__LINE__, op, 0, "PASS 1: action has no clock or timer");
		} else {
		    if (gp->gt_ini == -ALIAS) {		/* resolve clock/timer alias */
			if (df) fprintf(iC_outFP, "    resolve clock  >");
			while (gp->gt_ini == -ALIAS) {
			    if (df) fprintf(iC_outFP, "	%s@", gp->gt_ids);
			    gp = gp->gt_rptr;		/* adjust */
			}
			if (df) fprintf(iC_outFP, "	%c%s\n", iC_os[-gp->gt_ini], gp->gt_ids);
			tlp = lp - 1;
			*tlp = gp;			/* swap in real clock or timer */
		    }
		    gp->gt_mark++;			/* clock or timer node */
		    if (gp->gt_fni == TIMRL) {
			if ((gp = *lp++) == 0) {
			    inError(__LINE__, op, 0, "PASS 1: timed action has no delay");
			} else {
			    if (gp->gt_ini == -ALIAS) {	/* resolve arithmetic alias */
				if (df) fprintf(iC_outFP, "    resolve delay  >");
				while (gp->gt_ini == -ALIAS) {
				    if (df) fprintf(iC_outFP, "	%s@", gp->gt_ids);
				    gp = gp->gt_rptr;	/* adjust */
				}
				if (df) fprintf(iC_outFP, "	<%s\n", gp->gt_ids);
				tlp = lp - 1;
				*tlp = gp;		/* swap in real input */
			    }
			    gp->gt_old++;		/* timer delay */
			}
		    }
		}
	    }
	    if (op->gt_fni == F_SW || op->gt_fni == F_CF || op->gt_fni == F_CE) {
		if ((slp = op->gt_rlist) == 0) {	/* F_SW to F_CE */
		    inError(__LINE__, op, 0, "PASS 1: no condition for switch, if else action");
		} else
		if (lp < slp) {
		    /********************************************************************
		     * slp also delimits C parameter list starting at lp
		     * for every USE_COUNT parameters there is 1 use word at the end.
		     * Therefore divide by USE_COUNT + 1, because slp points past extra
		     * use word(s).
		     *******************************************************************/
		    Gate **		ulp;
		    Gate **		elp;
		    int			uc;
		    int			j;
		    unsigned int	useWord = 0	/* satisfies -Wall */;

		    uc = slp - lp;
		    assert(uc >= 2);			/* at least one pointer and one use word */
		    uc = (USE_COUNT + uc) / (USE_COUNT + 1);	/* # of use words */
		    ulp = elp = slp - uc;
		    j = USE_COUNT - 1;
		    while (lp < elp) {
			if (++j >= USE_COUNT) {
			    useWord = (unsigned int)(unsigned long)*ulp++;	/* next use word */
			    j = 0;			/* executed at least once - useWord is initialized */
			} else {
			    useWord >>= 2;		/* next set of useBits */
			}
			if ((gp = *lp++) == 0) {
			    inError(__LINE__, op, 0, "PASS 1: C parameter missing");
			} else {
			    if (gp->gt_ini == -ALIAS) {	/* resolve C parameter alias */
				if (df) fprintf(iC_outFP, "    resolve C param>");
				while (gp->gt_ini == -ALIAS) {
				    if (df) fprintf(iC_outFP, "	%s@", gp->gt_ids);
				    gp = gp->gt_rptr;	/* adjust */
				}
				if (df) fprintf(iC_outFP, "	<%s\n", gp->gt_ids);
				tlp = lp - 1;
				*tlp = gp;		/* swap in real input */
			    }
			    if (useWord & VAR_USE) {	/* (EU >> AU_OFS) */
				gp->gt_old++;		/* C function value parameter */
			    }
			    if (useWord & VAR_ASSIGN) {	/* (EA >> AU_OFS) */
				if (gp->gt_ini != -ARNC && gp->gt_ini != -LOGC) {
				    inError(__LINE__, op, gp, "PASS 1: variable is not declared immC");
				}
				gp->gt_val++;		/* C parameter is assigned */
			    }
			}
		    }
		    assert(ulp == slp);
		    lp = slp;				/* skip over use words */
		}
	    }
	}
    }
    if (df) fprintf(iC_outFP, " link count = %d\n", link_count);
    if (errCount) {
	exit(1);					/* pass 1 failed */
    }

/********************************************************************
 *
 *  INTERLUDE 1
 *
 *  Allocate space for the output activity lists for ARITH and
 *  GATE nodes.
 *
 *******************************************************************/

    ifp = fp = (Gate **)calloc(link_count, sizeof(Gate *));	/* links */

/********************************************************************
 *
 *  PASS 2
 *
 *  Using the counts accumulated in op->gt_mark determine the
 *  position of the last terminator for ARITH and GATE nodes.
 *  store this value in op->gt_list and clear the last terminator.
 *
 *  Using op->gt_val check the node has inputs.
 *  Using op->gt_mark or op->gt_old check the node has outputs.
 *
 *  The length of a pointer array for such a symbol table is
 *  sTend - sTable.
 *
 *******************************************************************/

    if (df) { fprintf(iC_outFP, "PASS 2 - symbol table: name inputs outputs delay-references\n"); fflush(iC_outFP); }
    /* iClock is sorted into Symbol table and does not need to be reported separately */
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	if (op->gt_ini != -ALIAS) {
	    if (df) {
		fprintf(iC_outFP, " %-8s %3d %3d", op->gt_ids, op->gt_val, op->gt_mark);
		if (op->gt_old) {
#if	INT_MAX == 32767 && defined (LONG16)
		    fprintf(iC_outFP, " %3ld\n", op->gt_old);	/* delay references */
#else	/* INT_MAX == 32767 && defined (LONG16) */
		    fprintf(iC_outFP, " %3d\n", op->gt_old);	/* delay references */
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		} else {
		    fprintf(iC_outFP, "\n");
		}
	    }
	    if (op->gt_val == 0 &&
		op->gt_mcnt == 0 &&		/* leave out _f0_1 or immC array */
		op->gt_ini != -ARNC &&
		op->gt_ini != -LOGC &&
		op->gt_ini != -INPB &&
		op->gt_ini != -INPW &&
		op->gt_ini != -INPX) {
		fprintf(iC_errFP, "WARNING '%s' has no input\n", op->gt_ids);
		if (df) fprintf(iC_outFP, "*** Warning: '%s' has no input\n", op->gt_ids);
	    }
	    if (op->gt_mark == 0 &&
		op->gt_old == 0 &&
		op->gt_ini != -ARNC &&
		op->gt_ini != -LOGC &&
		op->gt_fni != TRAB &&
		op->gt_fni != OUTW &&
		op->gt_fni != OUTX &&
		op != & iClock) {
		fprintf(iC_errFP, "WARNING '%s' has no output\n", op->gt_ids);
		if (df) fprintf(iC_outFP, "*** Warning: '%s' has no output\n", op->gt_ids);
	    }
	    if (op->gt_fni == ARITH) {
		op->gt_old = 0;			/* clear for run time after ARITH was delay */
		fp += op->gt_mark;
		*fp = 0;			/* last output terminator */
		op->gt_list = fp++;
	    } else
	    if (op->gt_fni == GATE) {
		fp += op->gt_mark + 1;
		*fp = 0;			/* last output terminator */
		op->gt_list = fp++;
	    }
	    if (op->gt_ini != -INPW && op->gt_fni != OUTW && op->gt_fni != OUTX &&
		((op->gt_ini != -ARNC && op->gt_ini != -LOGC) || op->gt_fni != UDFA)) {
		op->gt_mark = 0;		/* must be cleared for run-time */
	    }
	} else
	if (df) {
	    int inverse = 0;			/* print ALIAS in symbol table */
	    fprintf(iC_outFP, " %s@	", op->gt_ids);
	    gp = op;
	    while (gp->gt_ini == -ALIAS) {
		if (gp->gt_fni == GATE) {
		    inverse ^= gp->gt_mark;	/* holds ALIAS inversion flag */
		}
		gp = gp->gt_rptr;		/* resolve ALIAS */
	    }
	    fprintf(iC_outFP, "%s%s\n", inverse ? "~" : " ", gp->gt_ids);
	}
    }

    if ((val = fp - ifp) != link_count) {
	fprintf(iC_errFP,
	    "\nERROR: %s: line %d: link error %d vs link_count %d\n",
	    __FILE__, __LINE__, val, link_count);
	exit(2);
    }

/********************************************************************
 *
 *  PASS 3
 *
 *  Transfer LOGIC inverted inputs, storing them via gp->gt_list,
 *  which is pre-decremented. (same in PASS 4 and 5)
 *
 *  Since lists are now joined and sorted adjust remaining ALIAS's
 *  which arise in different sources. Do the same in PASS 5 for
 *  normal logic and arithmetic nodes and in PASS 6 for misc nodes.
 *
 *******************************************************************/

    if (df) { fprintf(iC_outFP, "PASS 3\n"); fflush(iC_outFP); }
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	if (op->gt_ini <= -MIN_GT && op->gt_ini > -MAX_GT &&
	    op->gt_mcnt == 0) {			/* leave out _f0_1 or immC array */
	    lp = op->gt_rlist;			/* reverse or input list */
	    while (*lp++ != 0);			/* skip normal inputs */
	    while ((gp = *lp++) != 0) {
		if (op->gt_ini == -XOR && gp->gt_fni == GATE) {
		    gp->gt_fni = GATEX;		/* this source must handle XOR */
		}
		*(--(gp->gt_list)) = op;	/* transfer inverted */
	    }
	}
    }

/********************************************************************
 *
 *  PASS 4
 *
 *  Insert first terminator for GATE nodes via op->gt_list.
 *
 *******************************************************************/

    if (df) { fprintf(iC_outFP, "PASS 4\n"); fflush(iC_outFP); }
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	if (op->gt_ini != -ALIAS && (op->gt_fni == GATE || op->gt_fni == GATEX)) {
	    *(--(op->gt_list)) = 0;		/* GATE normal terminator */
	}
    }

/********************************************************************
 *
 *  PASS 5
 *
 *  Transfer inputs for ARN and normal inputs for LOGIC nodes.
 *  At the end of this pass, the gt_list entries all point to
 *  the start of their respective activity lists.
 *
 *******************************************************************/

    if (df) { fprintf(iC_outFP, "PASS 5\n"); fflush(iC_outFP); }
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	if (op->gt_ini == -ARN ||
	    (op->gt_ini <= -MIN_GT && op->gt_ini > -MAX_GT &&
	    op->gt_mcnt == 0)) {		/* leave out _f0_1 or immC array */
	    lp = op->gt_rlist + (op->gt_ini == -ARN);
	    while ((gp = *lp++) != 0) {
		if (op->gt_ini == -XOR && gp->gt_fni == GATE) {
		    gp->gt_fni = GATEX;		/* this source must handle XOR */
		}
		*(--(gp->gt_list)) = op;	/* transfer normal */
	    }
	}
    }

/********************************************************************
 *
 *  PASS 6
 *
 *  Do a full consistency check on pre-compiled structures in
 *  case somebody messed around with the generated intermediate
 *  C files or the iC compiler is in error (!!)
 *
 *******************************************************************/

    if (df) { fprintf(iC_outFP, "PASS 6 - name gt_ini gt_fni: output list\n"); fflush(iC_outFP); }
    for (opp = sTable; opp < sTend; opp++) {
	op = *opp;
	if (op->gt_ini != -ALIAS) {
	    if (df) {
		if (op->gt_ini < 0) {
		    fprintf(iC_outFP, " %-8s%6s%7s:", op->gt_ids,
			iC_full_type[-op->gt_ini], iC_full_ftype[op->gt_fni]);
		} else {
		    fprintf(iC_outFP, " %-8s%6d%7s:", op->gt_ids,
			op->gt_ini, iC_full_ftype[op->gt_fni]);
		}
	    }
	    if ((op->gt_ini == -ARNC || op->gt_ini == -LOGC) && op->gt_fni == UDFA) {
		int	n = 0;				/* immC array */
		if ((lp = op->gt_rlist) == 0) {
		    inError(__LINE__, op, 0, "PASS 6: immC array node with no output list");
		}
		while ((gp = *lp++) != 0) {		/* scan array initialisers and check count */
		    if (df) fprintf(iC_outFP, "\t[%d] %s", n, gp->gt_ids);
		    n++;
		}
		if (n != op->gt_mark) {			/* use gt_mark, which is short to allow arrays up to size 32767 */
		    inError(__LINE__, op, 0, "PASS 6: immC array has incorrect number of initialisers");
		}
		/* gt_mark gets clobberd by oscillator marking - transferred to gt_old in INI Pass 3 */
	    } else {
		if (op->gt_ini == -ARN) {
		    if ((lp = op->gt_rlist) == 0) {
			inError(__LINE__, op, 0, "PASS 6: arithmetic node with no input list");
		    }
		    gp = *lp++;				/* skip function vector (may be 0) */
#ifdef	HEXADDRESS
		    if (df) fprintf(iC_outFP, "	0x%lx()", (long)gp);	/* cexe_n */
#else	/* HEXADDRESS */
		    if (df) fprintf(iC_outFP, "	0x0()");		/* dummy */
#endif	/* HEXADDRESS */
		    while ((gp = *lp++) != 0) {		/* for ARN scan 1 input list */
			if (gp->gt_ini == -ALIAS) {		/* arithmetic ALIAS */
			    inError(__LINE__, op, gp, "PASS 6: input int ALIAS not resolved");
			}
		    }
		} else
		/********************************************************************
		 *  arithmetic or logical input linkage to physical I/O (mainly display)
		 *******************************************************************/
		if (op->gt_ini == -INPW) {
		    if (sscanf(op->gt_ids, "%1[IT]%1[XBWL]%5d%7s",
			    iqt, xbwl, &byte, tail) == 3) {
			switch (iqt[0]) {
			case 'T':
			    if (byte != 0) goto inErr;	/* TXD must be 1 */
			    iC_TX0p = op;			/* forward input link */
			    /* fall through */
			case 'I':
			    break;
			default:
			    goto inErr;
			}
		    } else {
		      inErr:
;//			inError(__LINE__, op, 0, "PASS 6: no valid input word definition");

		    }
		}
		if (op->gt_fni == UDFA) {
		    inError(__LINE__, op, 0, "PASS 6: has undefined output type UDFA");
		} else
		if (op->gt_fni < MIN_ACT) {
		    tlp = lp = op->gt_list;		/* ARITH or GATE */
		    while (*tlp++ != 0);		/* skip inputs */
		    /* sort gate list */
		    qsort(lp, tlp - lp - 1, sizeof(Gate*), (iC_fptr)iC_cmp_gt_ids);
		    if (df) {
			while ((gp = *lp++) != 0) {
			    if (gp->gt_ini == -ALIAS) {	/* resolve bit/int alias */
				inError(__LINE__, op, gp, "PASS 6: bit or int ALIAS not resolved");
				fprintf(iC_outFP, "	@%s,", gp->gt_ids);
			    } else {
				fprintf(iC_outFP, "	%s,", gp->gt_ids);
			    }
			}
		    }
		    if (op->gt_fni == GATE || op->gt_fni == GATEX) {
			lp = tlp;
			while (*tlp++ != 0);		/* skip inputs */
			/* sort gate list */
			qsort(lp, tlp - lp - 1, sizeof(Gate*), (iC_fptr)iC_cmp_gt_ids);
			if (df) {
			    while ((gp = *lp++) != 0) {
				if (gp->gt_ini == -ALIAS) {	/* resolve inverted bit alias */
				    inError(__LINE__, op, gp, "PASS 6: inverted bit ALIAS not resolved");
				    fprintf(iC_outFP, "	@~%s,", gp->gt_ids);
				} else {
				    fprintf(iC_outFP, "	~%s,", gp->gt_ids);
				}
			    }
			}
		    }
		} else
		if (op->gt_fni < MAX_ACT) {
		    lp = op->gt_list;				/* RI_BIT to TIMR */
		    i = 0;						/* offset for _f0_1 */
		    if (op->gt_mcnt == 0) {				/* leave out _f0_1 or immC array */
			if (lp == 0 || (gp = *lp++) == 0) {
			    inError(__LINE__, op, 0, "PASS 6: no slave for master action RI_BIT to TIMR");
			} else
			if (op->gt_fni != F_SW && op->gt_fni != F_CF && op->gt_fni != F_CE) {
			    if (df) fprintf(iC_outFP, "	%s,", gp->gt_ids);
			} else {
#ifdef	HEXADDRESS
			    if (df) fprintf(iC_outFP, "	0x%lx()", (long)gp);	/* cexe_n */
#else	/* HEXADDRESS */
			    if (df) fprintf(iC_outFP, "	0x0(),");		/* dummy */
#endif	/* HEXADDRESS */
			}
			i = 2;					/* offset for clock */
			if ((gp = *lp++) == 0) {
			    inError(__LINE__, op, 0, "PASS 6: no clock or timer for master action");
			} else
			if (gp->gt_fni != CLCKL && gp->gt_fni != TIMRL) {
			    inError(__LINE__, op, gp, "PASS 6: strange clock or timer");
			} else {
			    if (gp->gt_ini == -ALIAS) {		/* resolve clock/timer alias */
				inError(__LINE__, op, gp, "PASS 6: clock or timer ALIAS not resolved");
			    }
			    if ((gp->gt_fni != CLCKL || gp->gt_ini != -CLK) &&
				(gp->gt_fni != TIMRL || gp->gt_ini != -TIM)) {
				inError(__LINE__, op, gp, "PASS 6: clock or timer master does not match slave");
			    }
			    if (df) fprintf(iC_outFP, "	%c%s,", iC_os[-gp->gt_ini], gp->gt_ids);
			    if (gp->gt_fni == TIMRL) {
				if ((gp = *lp++) == 0) {
				    inError(__LINE__, op, 0, "PASS 6: timed action has no delay");
				} else {
				    if (gp->gt_ini == -ALIAS) {	/* resolve arithmetic alias */
					inError(__LINE__, op, gp, "PASS 6: arithmetic delay ALIAS not resolved");
				    }
				    if (df) fprintf(iC_outFP, "	<%s,", gp->gt_ids);
				}
				i = 3;				/* offset for timer */
			    }
			}
		    }
		    if (op->gt_fni == F_SW || op->gt_fni == F_CF || op->gt_fni == F_CE) {	/* includes literal block _f0_1 */
			if ((slp = op->gt_rlist) == 0) {	/* F_SW to F_CE */
			    inError(__LINE__, op, 0, "PASS 6: no condition for switch, if else action");
			} else
			if (lp < slp) {
			    /********************************************************************
			     * slp also delimits C parameter list starting at lp
			     * for every USE_COUNT parameters there is 1 use word at the end.
			     * Therefore divide by USE_COUNT + 1, because slp points past extra
			     * use word(s).
			     *******************************************************************/
			    Gate **		ulp;
			    Gate **		elp;
			    int		uc;
			    int		j;
			    unsigned int	useWord = 0	/* satisfies -Wall */;

			    uc = slp - lp;
			    assert(uc >= 2);			/* at least one pointer and one use word */
			    uc = (USE_COUNT + uc) / (USE_COUNT + 1);	/* # of use words */
			    ulp = elp = slp - uc;
			    j = USE_COUNT - 1;
			    while (lp < elp) {
				if (++j >= USE_COUNT) {
				    useWord = (unsigned int)(unsigned long)*ulp++;	/* next use word */
				    j = 0;			/* executed at least once - useWord is initialized */
				} else {
				    useWord >>= 2;			/* next set of useBits */
				}
				if ((gp = *lp++) == 0) {
				    inError(__LINE__, op, 0, "PASS 6: C parameter missing");
				} else {
				    if (gp->gt_ini == -ALIAS) {	/* arithmetic alias */
					inError(__LINE__, op, gp, "PASS 6: C parameter ALIAS not resolved");
				    }
				    if (df) fprintf(iC_outFP, "\tC%d %s %s,", i++, gp->gt_ids,
					iC_useText[useWord & USE_MASK]);	/* (AU >> AU_OFS) */
				}
			    }
			    lp = slp;				/* skip over use words */
			}
		    }
		} else
		/********************************************************************
		 *  arithmetic or logical output linkage to physical I/O
		 *******************************************************************/
		if (op->gt_fni == OUTW) {
		    if ((i = sscanf(op->gt_ids, "Q%1[XBWL]%5d%7s", xbwl, &byte, tail)) >= 2) {
			switch (xbwl[0]) {
			case 'X':
			    if (i > 2) goto outErr;		/* no tail _0 allowed for QXn */
			    break;				/* op->gt_mark set to used bits 0x01 to 0x80 in Pass 0 */
			case 'B':
			    if (i != 3 || strcmp(tail, "_0") != 0) goto outErr;
			    op->gt_out = 0;			/* initial default value (gt_rlist no longer needed) */
			    op->gt_mark = B_WIDTH;
			    break;
			case 'W':
			    if (i != 3 || strcmp(tail, "_0") != 0) goto outErr;
			    op->gt_out = 0;			/* initial default value (gt_rlist no longer needed) */
			    op->gt_mark = W_WIDTH;
			    break;
#if	INT_MAX != 32767 || defined (LONG16)
			case 'L':
			    if (i != 3 || strcmp(tail, "_0") != 0) goto outErr;
			    op->gt_out = 0;			/* initial default value (gt_rlist no longer needed) */
			    op->gt_mark = L_WIDTH;
			    break;
#endif	/* INT_MAX != 32767 || defined (LONG16) */
			default:
			    goto outErr;
			}
			if (df) fprintf(iC_outFP, "	0x%02x", op->gt_mark);
		    } else {
		      outErr:
;//			inError(__LINE__, op, 0, "PASS 6: no valid output word definition");

		    }
		} else
		if (op->gt_fni < CLCKL) {
		    if (df) {
			if (op->gt_fni == OUTX) {
			    gp = op->gt_ptr;		/* OUTX */
			    fprintf(iC_outFP, "	%s	0x%02x", gp->gt_ids, op->gt_mark);
			}
		    }
		} else
		if (op->gt_fni <= TIMRL) {
		    if (op->gt_list != 0) {
			inError(__LINE__, op, 0, "PASS 6: clock or timer with initial forward list");
		    }
		    if (op->gt_fni == TIMRL && op->gt_old > 0) {
#if	INT_MAX == 32767 && defined (LONG16)
			if (df) fprintf(iC_outFP, "	(%ld)", op->gt_old);
#else	/* INT_MAX == 32767 && defined (LONG16) */
			if (df) fprintf(iC_outFP, "	(%d)", op->gt_old);
#endif	/* INT_MAX == 32767 && defined (LONG16) */
		    }
		} else {
		    inError(__LINE__, op, 0, "PASS 6: unknown output type (ftype)");
		}
	    }
	    if (df) fprintf(iC_outFP, "\n");
	} else
	if (df) {
	    int inverse = 0;				/* print ALIAS in symbol table */
	    gp = op;
	    fprintf(iC_outFP, " %-8s%6s%7s:", op->gt_ids,
		iC_full_type[-op->gt_ini], iC_full_ftype[op->gt_fni]);
	    while (gp->gt_ini == -ALIAS) {
		if (gp->gt_fni == GATE || gp->gt_fni == GATEX) {
		    inverse ^= gp->gt_mark;		/* holds ALIAS inversion flag */
		}
		gp = gp->gt_rptr;			/* resolve ALIAS */
	    }
	    fprintf(iC_outFP, "	%s%s\n", inverse ? "~" : "", gp->gt_ids);
	}
    }
    if (errCount) {
	exit(6);					/* pass 6 failed */
    }
#ifdef	RASPBERRYPI
    if (iC_opt_P) {
	/********************************************************************
	 *  End of PiFace detection
	 *******************************************************************/
	for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
	    if (pfp->Qgate || pfp->Igate || pfp->QPgate) {
		char *	piFext;
		uint8_t	inputA;
		uint8_t	inputB;
		if (pfp->intf == INTFA) {
		    piFext = "CAD";
		    inputA = 0xff;		/* PiFaceCAD all bits input on Port A */
		    inputB = 0x00;		/* PiFaceCAD has no input on Port B */
		} else {
		    piFext = "   ";
		    inputA = 0x00;		/* PiFace has no input on Port A */
		    inputB = pfp->Ibmask;	/* PiFace input on port B, may be 0 if all output */
		}
		if (iC_debug & 0200) {
		    gp = pfp->Qgate;
		    tgp = pfp->Igate;
		    ttgp = pfp->QPgate;
		    fprintf(iC_outFP, "###	%s%s  un = %d pfa = %d	%-6s %s	%-6s %s	%-6s %s\n",
			pfp->intf ? "PiFace" : "      ", piFext,	/* may be Dummy */
			pfp - iC_pfL, pfp->pfa,
			gp   ? gp->gt_ids   : NULL, markings[gp   ? gp->gt_live   : Dummy],
			tgp  ? tgp->gt_ids  : NULL, markings[tgp  ? tgp->gt_live  : Dummy],
			ttgp ? ttgp->gt_ids : NULL, markings[ttgp ? ttgp->gt_live : Dummy]
		    );
		}
		/********************************************************************
		 *  Final setup with correct IOCON_ODR for all PiFace Units with I/O.
		 *  Active driver output if only one PiFace else open drain
		 *******************************************************************/
		if (setupMCP23S17(pfp->spiFN, pfp->pfa, (iC_npf == 1) ? 0x00 : IOCON_ODR, 1, inputA, inputB) < 0) {
		    fprintf(iC_errFP, "ERROR: %s: PiFace%s %hu not found after succesful test ???\n",
			iC_progname, piFext, pfp->pfa);
		}
	    }
	}
	/********************************************************************
	 *  Export and open all gpio files for all GPIO arguments
	 *  open /sys/class/gpio/gpioN/value permanently for obtaining
	 *  out-of-band interrupts and to allow read and write operations
	 *******************************************************************/
	for (iq = 0; iq < 2; iq++) {
	    for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
		for (bit = 0; bit <= 7; bit++) {
		    if ((gpio = gep->gpioNr[bit]) != 0xffff) {
			if (iq == 0) {
			    /********************************************************************
			     *  QXn gpio N export with direction out and no interrupts
			     *******************************************************************/
			    if ((sig = gpio_export(gpio, "out", "none", forceFlag, iC_fullProgname)) != 0) {
				iC_quit(sig);		/* another program is using gpio N */
			    }
			    if ((gep->gpioFN[bit] = gpio_fd_open(gpio, O_RDWR)) < 0) {
				fprintf(iC_errFP, "ERROR: %s: Cannot open GPIO %d for output: %s\n", iC_progname, gpio, strerror(errno));
			    } else {
				gpio_write(gep->gpioFN[bit], 0);	/* write 0 to GPIO to initialise it */
			    }
			} else {
			    /********************************************************************
			     *  IXn gpio N export with direction in and interrupts on both edges
			     *******************************************************************/
			    if ((sig = gpio_export(gpio, "in", "both", forceFlag, iC_fullProgname)) != 0) {
				iC_quit(sig);		/* another program is using gpio N */
			    }
			    if ((gep->gpioFN[bit] = gpio_fd_open(gpio, O_RDONLY)) < 0) {
				fprintf(iC_errFP, "ERROR: %s: Cannot open GPIO %d for input: %s\n", iC_progname, gpio, strerror(errno));
			    }
			    /********************************************************************
			     *  Execute the SUID root progran iCgpioPUD(gpio, pud) to set pull-up/down
			     *    for normal   input set pull down (logic 0 = low)
			     *    for inverted input set pull up   (logic 0 = high)
			     *  Different bits for one GPIO IEC variable can be normal or inverted
			     *******************************************************************/
			    iC_gpio_pud(gpio, gep->Ginv & iC_bitMask[bit] ?
				BCM2835_GPIO_PUD_UP :	/* inverted input bit */
				BCM2835_GPIO_PUD_DOWN);	/* normal input bit */
			}
			if (gep->gpioFN[bit] > iC_maxFN) {
			    iC_maxFN = gep->gpioFN[bit];
			}
		    } else if (iq == 1) {
			gep->Ginv &= ~iC_bitMask[bit];	/* blank out missing GPIO input bits IXx.y */
		    }
		}
	    }
	}
    } /* iC_opt_P */
#endif	/* RASPBERRYPI */

    /********************************************************************
     * Execute iC load object
     *******************************************************************/
    iC_icc();				/* Gate ** sTable, Gate ** sTend are global */
    /********************************************************************
     * never returns - exits via iC_quit()
     *******************************************************************/
    return 0;
} /* main */
#ifdef	RASPBERRYPI

/********************************************************************
 *
 *	Convert a string to integer using the first characters to determine the base
 *	    0		return 0
 *	    0b[01]*	convert binary string
 *	    0x[0-9a-f]*	convert hexadeximal string
 *	    0X[0-9A-F]*	convert hexadeximal string
 *	    0[0-7]*	convert octal string
 *	    [1-9][0-9]*	convert decimal string
 *	    error	return 0x7fffffff	// LONG_MAX
 *
 *******************************************************************/

static long
convert_nr(const char * str, char ** endptr)
{
    long	val;

    errno = 0;				/* to distinguisg success/failure after call */
    if (*str == '0' && (*(str + 1) == 'b' || *(str + 1) == 'B')) {
	val = strtol(str+2, endptr, 2);	/* convert base 2 binary */
    } else {
	val = strtol(str, endptr, 0);	/* convert hex, octal or decimal */
    }
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
	|| (errno != 0 && val == 0)) {	/* Check for various possible errors */
	return LONG_MAX;
    }
    return val;
} /* convert_nr */

/********************************************************************
 *
 *	Unexport GPIOs and close PiFaces if any real I/O parameters
 *	Clear gpiosp->u.used bits for GPIOs and A/D channels used in this app
 *
 *******************************************************************/

static int
termQuit(int sig)
{
    if (iC_opt_P || iC_npf) {	/* test iC_npf in case -P option with PiFace but no IEC parameters and no IX or QX in app */
	if (iC_debug & 0200) fprintf(iC_outFP, "=== Unexport GPIOs and close PiFaces =======\n");
	piFaceIO *	pfp;
	int		pfce;
	int		iq;
	gpioIO *	gep;
	unsigned short	bit;
	unsigned short	gpio;
	int		fn;
	ProcValidUsed *	gpiosp;

	/********************************************************************
	 *  Unexport and close all gpio files for all GPIO arguments
	 *******************************************************************/
	for (iq = 0; iq <= 1; iq++) {
	    for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
		for (bit = 0; bit <= 7; bit++) {
		    if ((gpio = gep->gpioNr[bit]) != 0xffff) {
			/********************************************************************
			 *  Execute the SUID root progran iCgpioPUD(gpio, pud) to turn off pull-up/down
			 *******************************************************************/
			if (iq == 0) iC_gpio_pud(gpio, BCM2835_GPIO_PUD_OFF);	/* inputs only */
			/********************************************************************
			 *  Close GPIO N value
			 *******************************************************************/
			if ((fn = gep->gpioFN[bit])> 0) {
			    close(fn);			/* close connection to /sys/class/gpio/gpio_N/value */
			}
			/********************************************************************
			 *  Force all outputs and inputs to direction "in" and "none" interrupts
			 *******************************************************************/
			if (sig != SIGUSR2 && gpio_export(gpio, "in", "none", 1, iC_progname) != 0) {
			    sig = SIGUSR1;		/* unable to export gpio_N */
			}
			/********************************************************************
			 *  free up the sysfs for gpio N unless used by another program (SIGUSR2)
			 *******************************************************************/
			if (iC_debug & 0200) fprintf(iC_outFP, "### Unexport GPIO %hu\n", gpio);
			if (sig != SIGUSR2 && gpio_unexport(gpio) != 0) {
			    sig = SIGUSR1;		/* unable to unexport gpio_N */
			}
		    }
		}
	    }
	}
	/********************************************************************
	 *  PiFaces
	 *******************************************************************/
	if (iC_npf) {
	    if ((iC_debug & 0200) != 0) fprintf(iC_outFP, "### Shutdown active PiFace units\n");
	    /********************************************************************
	     *  Turn off the pullup resistor on gpio25/INTB
	     *******************************************************************/
	    iC_gpio_pud(25, BCM2835_GPIO_PUD_OFF);
	    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
		/********************************************************************
		 *  Clear active PiFace outputs and turn off active PiFaceCad
		 *  Shutdown all active PiFace Units leaving interrupts off and open drain
		 *******************************************************************/
		if (pfp->Qgate || pfp->Igate || pfp->QPgate) {		/* works for both iCpiFace and apps */
		    if (pfp->Qgate) {
			if (pfp->pfa != 4 || pfp->intf != INTFA) {	/* PiFace */
			    writeByte(pfp->spiFN, pfp->pfa, OLATA, pfp->Qinv);
			} else {					/* PiFaceCAD */
			    pifacecad_lcd_clear();
			    pifacecad_lcd_display_off();
			    pifacecad_lcd_set_backlight(0);
			}
		    }
		    if (pfp->QPgate) {
			writeByte(pfp->spiFN, pfp->pfa, OLATB, pfp->QPinv);
		    }
		    if (setupMCP23S17(pfp->spiFN, pfp->pfa, IOCON_ODR, 0, 0x00, 0x00) < 0) {
			fprintf(iC_errFP, "ERROR: %s: PiFace %d not found after succesful test ???\n",
			    iC_progname, pfp->pfa);
		    }
		}
	    }
	    /********************************************************************
	     *  Close selected spidev devices
	     *******************************************************************/
	    for (pfce = 0; pfce < 2; pfce++) {
		if (spidFN[pfce] > 0) {
		    close(spidFN[pfce]);		/* close connection to /dev/spidev0.0, /dev/spidev0.1 */
		}
	    }
	    /********************************************************************
	     *  Close GPIO25/INTB/INTA value
	     *  free up the sysfs for gpio 25 unless used by another program (SIGUSR2)
	     *******************************************************************/
	    if (gpio25FN > 0) {
		close(gpio25FN);			/* close connection to /sys/class/gpio/gpio25/value */
		if (iC_debug & 0200) fprintf(iC_outFP, "### Unexport GPIO 25\n");
		if (sig != SIGUSR2 && gpio_unexport(25) != 0) {
		    sig = SIGUSR1;			/* unable to unexport gpio 25 */
		}
	    }
	}
	/********************************************************************
	 *  Open and lock the auxiliary file ~/.iC/gpios.rev again
	 *  Other apps may have set used bits since this app was started
	 *******************************************************************/
	if ((gpiosp = openLockGpios(0)) == NULL) {
	    fprintf(iC_errFP, "ERROR: %s: in openLockGpios()\n",
		iC_progname);
	    return (SIGUSR1);			/* error quit */
	}
	gpiosp->u.used &= ~ownUsed;		/* clear all bits for GPIOs and A/D channels used in this app */
	if (writeUnlockCloseGpios() < 0) {	/* unlink (remove) ~/.iC/gpios.rev if gpios->u.used and oops is 0 */
	    fprintf(iC_errFP, "ERROR: %s: in writeUnlockCloseGpios()\n",
		iC_progname);
	    return (SIGUSR1);			/* error quit */
	}
	if (iC_debug & 0200) fprintf(iC_outFP, "=== End Unexport GPIOs and close PiFaces ===\n");
    } /* iC_opt_P */
    return (sig);			/* finally quit */
} /* termQuit */
#endif	/* RASPBERRYPI */
#endif	/* LOAD */

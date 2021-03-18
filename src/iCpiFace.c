/********************************************************************
 *
 *	Copyright (C) 2015  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	iCpiFace
 *	iCpiGPIO	(alternative name soft linked to the above)
 *
 *	I/O client for iCserver in the immediate C environment of a
 *	Raspberry Pi handling real 8 bit inputs and 8 bit outputs
 *	for each PiFace controllers attached to a Raspberry Pi
 *	or from a number of direct GPIO pins on the Raspberry Pi
 *	or both.  It also handles 8 bits of push button input and
 *	control of an LCD Text panel for a PiFaceCAD.
 *
 *	All GPIO pins on a Raspberry Pi A, B or B+ may be selected as
 *	either input bits or output bits independent of whether PiFaces
 *	are present or not, except GPIO 25 if PiFaces are also processed.
 *	All GPIO inputs are handled by interrupts.
 *
 *	If no PiFace controllers are found or if the program is called
 *	with the name iCpiGPIO, only GPIO pins will be handled.
 *
 *	A maximum of 8 PiFace controllers can be handled altogether
 *	- either one plugged directly into a Raspberry Pi
 *	- or 4 on one PiRack - 7 on 2 PiRacks - 8 require 3 PiRacks
 *
 *	One of the PiFaces (at address 4) can be a PiFaceCAD, which
 *	has 8 button inputs and a 2 line x 16 character LCD display
 *
 *	Similar I/O operations for both PiFaces and Raspberry Pi GPIOs
 *	can be obtained by linking the driver directly to an iC app,
 *	in which case these I/O operations are not handled by iCserver.
 *
 *******************************************************************/

#include	<signal.h>
#include	<ctype.h>
#include	<assert.h>
#include	<errno.h>
#include	<sys/stat.h>
#include	<fcntl.h>

#if !defined(RASPBERRYPI) || !defined(TCP) || defined(_WINDOWS)
#error - must be compiled with RASPBERRYPI and TCP defined and not _WINDOWS
#else	/* defined(RASPBERRYPI) && defined(TCP) && !defined(_WINDOWS) */

#include	"tcpc.h"
#include	"icc.h"			/* declares iC_emalloc() in misc.c */
#include	"rpi_rev.h"		/* Determine Raspberry Pi Board Rev */
#include	"rpi_gpio.h"
#include	"mcp23s17.h"
#include	"pifacecad.h"
#include	"bcm2835.h"

/********************************************************************
 *
 *  Select either a piFaceIO iC_pfL[un] with a small int un as index
 *  Alternativeley select a gpioIO element with the pointer gep
 *
 *******************************************************************/

typedef union	chS {
    int		un		/* always < 8 for PiFace, > 8 if gpioIO* for GPIO */;
    gpioIO *	gep;
} chS;

static fd_set		infds;			/* initialised file descriptor set for normal iC_wait_for_next_event() */
static fd_set		ixfds;			/* initialised extra descriptor set for normal iC_wait_for_next_event() */
static struct timeval	toCini = { 0,  50000 };	/* 50 ms select() timeout - initial value */
static struct timeval	toC750 = { 0, 750000 };	/* 750 ms select() timeout - re-iniialising value */
static struct timeval	toCnt  = { 0,  50000 };	/* 50 ms select() timeout */
static struct timeval *	toCntp = NULL;		/* select() timeout initial value off */
static const char *	pfCADname = "PFCAD4";	/* iC string */
static piFaceIO *	pfCADpfp = NULL;	/* for PiFaceCAD string via PFCAD4 */
static unsigned short	pfCADchannel = 0;

static const char *	usage =
"Usage:\n"
" %s [-GBIftmqzh][ -s <host>][ -p <port>][ -n <name>][ -i <inst>]\n"
"          [ -d <deb>]\n"
"          [ [~]<IEC>[-<IEC>|+[<mask>]][:<pfa>][-<inst>] ...]\n"
"          [ [~]<IEC>,<gpio>[,<gpio>,...][-<inst>] ...]\n"
"          [ [~]<IEC>.<bit>,<gpio>[-<inst>] ...]    # at least 1 IEC argument\n"
"          [ -R <aux_app>[ <aux_app_argument> ...]] # must be last arguments\n"
"    -s host IP address of server    (default '%s')\n"
"    -p port service port of server  (default '%s')\n"
"    -i inst instance of this client (default '') or 1 to %d digits\n"
"    -G      service GPIO I/O only - block PiFaces and PiFace arguments\n"
"            calling with alternate name \"iCpiGPIO\" has the same effect\n"
"    -B      start iCbox -d to monitor active PiFace and/or GPIO I/O\n"
"            A 1 to PiFace and GPIO outputs generates hi - LED and relay off\n"
"            A 0 to PiFace and GPIO outputs generates lo - LED and relay on\n"
"    -I      invert all PiFace and GPIO inputs and outputs\n"
"            PiFace and GPIO input circuits are normally hi - 1 and\n"
"            when a switch on the input is pressed it goes lo - 0. Therefore\n"
"            it is appropriate to invert inputs and outputs. When inverted\n"
"            a switch pressed on an input generates a 1 for the IEC inputs and\n"
"            a 1 on an IEC output turns a LED and relay on, which is natural.\n"
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
"          GPIOB+ selections cannot be part of a range.\n"
"                      GPIO arguments\n"
"          Any IEC I/Os IXx.y and QXx.y which are to be linked to GPIO\n"
"          inputs or outputs must be named in the argument list as follows:\n"
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
"    NOTE: there must be at least 1 PiFace or GPIO IEC argument.\n"
"          No IEC arguments are generated automatically for %s.\n"
"    For a full Description and Technical Background of the PiFace and GPIO\n"
"    driver software see the iCpiFace man page\n"
"                      CAVEAT\n"
"    Only one instance of iCpiFace or an app with IEC parameters may be run\n"
"    and all GPIOs, PiFaces and an optional PiFaceCAD must be controlled by\n"
"    this one instance. If two instances were running, the common interrupts\n"
"    would clash. Also no other program controlling GPIOs and PiFaces like\n"
"    'PiFace Digital Emulator' may be run at the same time as this application.\n"
"    An exception is iCpiPWM which controls GPIOs by DMA and not by interrupts.\n"
"    Another exception is iCtherm which controls GPIO 4 by the 1Wire interface.\n"
"    Care is taken that any GPIOs or PiFaces used in one app, iCpiFace, iCpiPWM\n"
"    or even iCtherm do not clash with another app (using ~/.iC/gpios.used).\n"
"                      DEBUG options\n"
"    -t      trace arguments and activity (equivalent to -d100)\n"
"         t  at run time toggles gate activity trace\n"
"    -m      display microsecond timing info\n"
"         m  at run time toggles microsecond timing trace\n"
"    -d deb  +1   trace TCP/IP send actions\n"
"            +2   trace TCP/IP rcv  actions\n"
#ifdef	TRACE
"            +4   trace MCP and HD44780 calls\n"
#endif	/* TRACE */
"            +10  trace SIO  input  actions\n"
"            +20  trace SIO  output actions\n"
#ifdef	TRACE
"            +40  debug MCP and HD44780 calls\n"
#endif	/* TRACE */
"            +100 show arguments\n"
"            +200 show more debugging details\n"
"            +400 exit after initialisation\n"
"    -q      quiet - do not report clients connecting and disconnecting\n"
"    -z      block keyboard input on this app - used by -R\n"
"    -h      this help text\n"
"         T  at run time displays registrations and equivalences\n"
#ifdef	TRACE
"         i  at run time reports MCP23S17 IOCON, GPINTEN settings\n"
"         I  at run time restores MCP23S17 IOCON, GPINTEN settings\n"
#endif	/* TRACE */
"         q  or ctrl+D  at run time stops %s\n"
"                      AUXILIARY app\n"
"    -R <app ...> run auxiliary app followed by -z and its arguments\n"
"                 as a separate process; -R ... must be last arguments.\n"
"\n"
"Copyright (C) 2014-2015 John E. Wulff     <immediateC@gmail.com>\n"
"Version	$Id: iCpiFace.c 1.13 $\n"
;

char *		iC_progname;		/* name of this executable */
static char *	iC_path;
short		iC_debug = 0;
int		iC_micro = 0;
unsigned short	iC_osc_lim = 1;

/********************************************************************
 *
 *  piFaceIO array iC_pfL[unit] is organised as follows:
 *    [unit] is the PiFace unit index in address order
 *    each element of the array is a struct piFaceIO
 *
 *  A single PiFace can be addressed from 0 - 3 with jumpers.
 *  With CE0 selected /dev/spidev0.0 is used for SPI transfers.
 *  With CE1 selected /dev/spidev0.1 is used for SPI transfers.
 *
 *  A PiRack can accomodate up to 4 PiFaces (more if daisy-chained).
 *  Also CE for each PiFace can be changed from CE0 (default) to CE1.
 *  This changes the addresses above from 4 - 7 allowing a maximum
 *  of 8 PiFaces to be used.
 *
 *******************************************************************/

static int	unit = 0;
static unsigned short	topChannel = 0;
static char	buffer[BS];
static char	SR[] = "RSR";		/* generates RQ for iq 0, SI for iq 1, RQ for iq 2 */
static char	IQ[] = "QIQ";
static short	errorFlag = 0;
static chS *	Units = NULL;		/* dynamic array to store PiFace unit numbers indexed by channel or GPIOl */
static int	ioUnits = 0;		/* dynamically allocated size of Units[] */
static long long ownUsed = 0LL;
char		iC_stdinBuf[REPLY];	/* store a line of STDIN - used by PiFaceCAD input */
static long	convert_nr(const char * str, char ** endptr);
static int	termQuit(int sig);		/* clear and unexport RASPBERRYPI stuff */
int		(*iC_term)(int) = &termQuit;	/* function pointer to clear and unexport RASPBERRYPI stuff */

static void	storeUnit(unsigned short channel, chS s);
static void	writeGPIO(gpioIO * gep, unsigned short channel, int val);

FILE *		iC_outFP;		/* listing file pointer */
FILE *		iC_errFP;		/* error file pointer */

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
    char **		argp;
    int			pfce;
    unsigned short	pfa;
    unsigned short	pfe;
    unsigned short	pff;
    int			len;
    int			value;
    int			sig;
    int			n;
    int			c;
    int			m;
    int			m1;
    int			un;
    int			iq;
    int			unitA[3] = { 0, 0, 0 };
    int			val;
    int			slr;
    int			cbc;
    unsigned short	iid;
    char		iids[6];
    int			regBufLen = 0;
    unsigned short	iidN = -1;	/* internal instance "" initially */
    int			forceFlag = 0;
    char *		iC_fullProgname;
    char *		cp;
    char *		np;
    char *		op;
    int			ol;
    unsigned short	channel = 0;
    int			retval;
    char		tail[128];	/* must match sscanf formats %127s for tail */
    char *		mqz = "-qz";
    char *		mz  = "-z";
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
    unsigned short	bit, bitStart, bitEnd;
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
    int			gpioCnt = 0;
    int			invMask;
    int			invFlag;
    int			gpioTherm;
    char **		argip;
    int			argic;
    long long		gpioMask;
    ProcValidUsed *	gpiosp;

    iC_outFP = stdout;			/* listing file pointer */
    iC_errFP = stderr;			/* error file pointer */

#ifdef	EFENCE
    regBuf = iC_emalloc(REQUEST);
    rpyBuf = iC_emalloc(REPLY);
#endif	/* EFENCE */
    signal(SIGSEGV, iC_quit);			/* catch memory access signal */

    /********************************************************************
     *  By default do not invert PiFace and GPIO inputs as well as GPIO outputs
     *  By default invert PiFace outputs
     *  Both are inverted again with -I and '~'
     *******************************************************************/

    invMask = invFlag = 0x00;
    gpioTherm = 4;				/* default GPIO number used by kernel module w1-gpio */
    argip = iC_emalloc(argc * sizeof(char *));	/* temporary array to hold IEC arguments */
    argic = 0;

    /********************************************************************
     *
     *  Determine program name
     *
     *******************************************************************/

    iC_path = argv[0];			/* in case there is a leading path/ */
    len = strlen(iC_path);
    iC_fullProgname = iC_emalloc(len+1);
    strcpy(iC_fullProgname, iC_path);
    if ((iC_progname = strrchr(iC_path, '/')) == NULL &&
        (iC_progname = strrchr(iC_path, '\\')) == NULL
    ) {
	iC_progname = iC_path;		/* no leading path */
	iC_path     = "";		/* default path */
    } else {
	*iC_progname++ = '\0';		/* path has been stripped and isolated */
    }
    if (strcmp(iC_progname, "iCpiGPIO") == 0) {
	iC_opt_G = 1;			/* block PiFace I/O - same as -G flag */
    }
    iC_iccNM = iC_progname;		/* generate our own name - not 'stdin' from tcpc.c */
    if (iC_debug & 0200)  fprintf(iC_outFP, "fullPath = '%s' path = '%s' progname = '%s'\n", iC_fullProgname, iC_path, iC_progname);

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
		    int		df;
		    int		slen;
		case 's':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) iC_hostNM = *argv;
		    goto break2;
		case 'p':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strlen(*argv)) iC_portNM = *argv;
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
			iidN = atoi(iC_iidNM);	/* internal instance from command line -i<instance> */
		    }
		    goto break2;
		case 'G':
		    iC_opt_G = 1;	/* GPIO direct action only - block PiFace IO, even if PiFaces present */
		    break;
		case 'B':
		    iC_opt_B = 1;	/* start iCbox -d to monitor active PiFace and/or GPIO I/O */
		    break;
		case 'I':
		    invMask = 0xff;	/* invert PiFace and GPIO inputs as well as GPIO outputs with -I */
		    iC_opt_P = 1;	/* do not invert PiFace outputs with -I (inverted a second time) */
		    break;
		case 'W':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    gpioTherm = atoi(*argv);
		    if (gpioTherm > 31) {
			fprintf(iC_errFP, "ERROR: %s: -W %hu value > 31\n", iC_progname, gpioTherm);
			goto error;
		    }
		    goto break2;
		case 'f':
		    forceFlag = 1;	/* force use of GPIO interrupts - in particular GPIO 25 */
		    break;
		case 'm':
		    iC_micro++;		/* microsecond info */
		    break;
		case 'd':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if ((slen = strlen(*argv)) != strspn(*argv, "01234567") ||	/* octal digits only */
			sscanf(*argv, "%o", &df) != 1 ||
			df & ~0777) {
			fprintf(iC_errFP, "ERROR: %s: '-d %s' is not a well formed octal string or exceeds range 777\n",
			    iC_progname, *argv);
			errorFlag++;
		    }
		    iC_debug |= df;	/* short */
		    goto break2;
		case 't':
		    iC_debug |= 0100;	/* trace arguments and activity */
		    break;
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
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    *(argv-1) = *argv;	/* move app string to previous argv array member */
		    *argv = iC_debug & DQ ?  mqz : mz; /* point to "-qz"  or "-z" in current argv */
		    argv--;			/* start call with app string */
		    goto break3;
		missing:
		    fprintf(iC_errFP, "ERROR: %s: missing value after '%s'\n", iC_progname, ((*--argv)-=2, *argv));
		case 'h':
		case '?':
	      error:
		    fprintf(iC_errFP, usage, iC_progname, iC_hostNM, iC_portNM, INSTSIZE, INSTSIZE, iC_progname, iC_progname);
		    exit(-1);
		default:
		    fprintf(iC_errFP, "WARNING: %s: unknown option -%c\n", iC_progname, **argv);
		    break;
		}
	    } while (*++*argv);
	  break2: ;
	} else {
	    /********************************************************************
	     *  Save IEC arguments for later analysis after all options have
	     *  been determined, in particlar -G, and after Pifaces have
	     *  been counted. IEC arguments and options may be mixed (except -R).
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
		fprintf(stderr, "ERROR: %s: invalid argument '%s' (IEC variables start with ~ I Q X or D for dummy)\n", iC_progname, *argv);
		errorFlag++;
	    }
	    argip[argic++] = *argv;
	}
    }
  break3:
    /********************************************************************
     *  if argc != 0 then -R and argv points to auxialliary app + arguments
     *               do not fork and execute aux app until this app has
     *               connected to iCserver and registered all its I/Os
     *******************************************************************/
    if (! errorFlag && iC_opt_P) {
	/********************************************************************
	 *  Open or create and lock the auxiliary file ~/.iC/gpios.used,
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
		gpiosp->u.used |= gpioMask;		/* mark gpio used for ~/.iC/gpios.used */
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
		 *
		 *  Execute the SUID root progran iCgpioPUD(25, 2) to do this.
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
	directFlag = 0;
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
		    if (unitA[iq]++ >= iC_npf) {	/* maximum unassigned unit found so far */
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
	    } else {					/* n == 0 or 1 or *xd != 'X' */
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
		gep->Gname = NULL;		/* GPIO IEC in or output not active */
		gep->Gval = ieStart;
		gep->Gchannel = iid;		/* temporary IEC details in new element */
		for (bit = 0; bit <= 7; bit++) {
		    gep->gpioNr[bit] = 0xffff;	/* mark gpio number unused */
		    gep->gpioFN[bit] = -1;	/* mark file number unused */
		}
		gpioCnt++;
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
			    gpiosp->u.used |= gpioMask;	/* mark gpio used for ~/.iC/gpios.used */
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
    if (errorFlag) {
	iC_quit(-3);					/* after all the command line has been analyzed */
    }
    if (directFlag == 0) {
	fprintf(iC_errFP, "ERROR: %s: no IEC arguments? there must be at least 1 PiFace or GPIO argument\n", iC_progname);
	iC_quit(-4);					/* call termQuit() to terminate I/O */
    }
    /********************************************************************
     *  Generate IEC names for all arguments which have a PiFace and a
     *	non-zero bmask. PiFace arguments with zero bmask are not registered.
     *  Do this here because instance was not yet determind when IEC name
     *  was found
     *******************************************************************/
    unit = max(max(unitA[0], unitA[1]), unitA[2]);
    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
	for (iq = 0; iq < 3; iq++) {
	    pfq = &(pfp->s[iq]);
	    if (pfq->bmask && (n = pfq->val) >= 0) {	/* valid IEC argument Xn ? */
		len = snprintf(buffer, BS, "%cX%d", IQ[iq], n);	/* IEC name without instance */
		if ((iid = pfq->channel) != 0xffff) {	/* append possible instance */
		    len += snprintf(buffer + len, BS, "-%hu", iid);
		}
		pfq->i.name = iC_emalloc(++len);
		strcpy(pfq->i.name, buffer);		/* store name */
	    }
	}
    }
    /********************************************************************
     *  Generate IEC names for all GPIO output and input arguments
     *  Do the same for iC_gpL[0] and iC_gpL[1]
     *******************************************************************/
    for (iq = 0; iq < 2; iq++) {
	for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
	    len = snprintf(buffer, BS, "%cX%d", IQ[iq], gep->Gval);	/* IEC name without instance */
	    if ((iid = gep->Gchannel) != 0xffff) {			/* append possible instance */
		len += snprintf(buffer + len, BS, "-%hu", iid);
	    }
	    gep->Gname = iC_emalloc(++len);
	    strcpy(gep->Gname, buffer);		/* store name */
	}
    }
    /********************************************************************
     *  Clear val and channel members in iC_pfL[] entries
     *******************************************************************/
    for (pfp = iC_pfL; pfp < &iC_pfL[MAXPF]; pfp++) {
	for (iq = 0; iq < 3; iq++) {
	    pfq = &(pfp->s[iq]);
	    pfq->val = 0;			/* restore values */
	    pfq->channel = 0;
	}
    }
    /********************************************************************
     *  Do the same for iC_gpL[0] and iC_gpL[1]
     *******************************************************************/
    for (iq = 0; iq < 2; iq++) {
	for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
	    assert(gep->Gname);		/* later scans rely on a name */
	    gep->Ginv ^= invMask;	/* by default do not invert GPIO in or outputs - inverted with -I or '~' */
	    gep->Gval = 0;		/* restore values */
	    gep->Gchannel = 0;
	}
    }
    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {	/* TODO may not be needed */
	if (pfp->intf == 0) {
	    fprintf(iC_errFP, "ERROR: %s: PiFace %d address %hu: was requested in the call but was not found\n",
		iC_progname, pfp - iC_pfL, pfp->pfa);
	    iC_quit(-4);				/* call termQuit() to terminate I/O */
	}
    }
    /********************************************************************
     *  Adjust sizes if number of IEC arguments is incorrect
     *******************************************************************/
    if (iC_npf < unit) {
	fprintf(iC_errFP, "WARNING: %s: too many IEC arguments (%d) for the %d PiFaces found - ignore extra IEC's\n",
	    iC_progname, unit, iC_npf);		/* takes account of dummies */
    } else if (iC_npf > unit) {
	fprintf(iC_errFP, "WARNING: %s: not enough IEC arguments (%d) for the %d PiFaces found - ignore extra PiFaces\n",
	    iC_progname, unit, iC_npf);		/* takes account of dummies */
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "### unit = %d iC_npf = %d\n", unit, iC_npf);
    unit = iC_npf;					/* deal only with real PiFaces */
    if (unit == 0 && gpioCnt == 0) {
	fprintf(iC_errFP, "ERROR: %s: no PiFaces or gpio's to run\n", iC_progname);
	iC_quit(-2);
    }
    /********************************************************************
     *  End of PiFace detection
     *******************************************************************/
    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
	if (pfp->Qname || pfp->Iname || pfp->QPname) {
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
	    if (iC_debug & 0200) fprintf(iC_outFP, "###	PiFace%s un = %d pfa = %hu	%s	%s	%s\n",
		piFext, pfp - iC_pfL, pfp->pfa, pfp->Qname, pfp->Iname, pfp->QPname);
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
			 *    for normal   input (logic 0 = low)  set pull down pud = 1
			 *    for inverted input (logic 0 = high) set pull up   pud = 2
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
    /********************************************************************
     *  Generate a meaningful name for network registration
     *******************************************************************/
    len = snprintf(buffer, BS, "%s", iC_iccNM);
    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
	if (pfp->Iname || pfp->Qname) {
	    len += snprintf(buffer+len, BS-len, "_%hu", pfp->pfa);	/* name PiFace pfa */
	}
    }
    if (gpioCnt) {
	len += snprintf(buffer+len, BS-len, "_G");		/* name GPIO marker */
    }
    if (*iC_iidNM) {
	len += snprintf(buffer+len, BS-len, "-%s", iC_iidNM);	/* name-instance header */
    }
    iC_iccNM = iC_emalloc(len + 1);
    strcpy(iC_iccNM, buffer);
    if (iC_micro) iC_microReset(0);		/* start timing */
    if (iC_debug & 0200) fprintf(iC_outFP, "host = %s, port = %s, name = %s\n", iC_hostNM, iC_portNM, iC_iccNM);
    if (iC_debug & 0400) iC_quit(0);
    signal(SIGINT, iC_quit);			/* catch ctrlC and Break */
#ifdef	SIGTTIN
    /********************************************************************
     *  The following behaviour was observed on Linux kernel 2.2
     *  When this process is running in the background, and a command
     *  (even a command which does not exist) is run from the bash from
     *  the same terminal on which this process was started, then this
     *  process is sent signal SIGTTIN. If the signal SIGTTIN is ignored,
     *  then stdin receives EOF (a few thousand times) every time a
     *  character is entered at the terminal.
     *
     *  To prevent this process from hanging when run in the background,
     *  SIGTTIN is ignored, and when the first EOF is received on stdin,
     *  FD_CLR(0, &infds) is executed to stop selects on stdin from then on.
     *  This means, that stdin can still be used normally for the q, t and
     *  m command when the process is run in the foreground, and behaves
     *  correctly (does not STOP mysteriously) when run in the background.
     *
     *  This means that such a process cannot be stopped with q, only with
     *  ctrl-D, when it has been brought to the foreground with fg.
     *******************************************************************/
    signal(SIGTTIN, SIG_IGN);			/* ignore tty input signal in bg */
#endif	/* SIGTTIN */

    /********************************************************************
     *
     *  All arguments have been taken care of
     *
     *  Start TCP/IP communication, build registration string and register
     *
     *******************************************************************/

    iC_sockFN = iC_connect_to_server(iC_hostNM, iC_portNM);
    if (iC_sockFN > iC_maxFN) {
	iC_maxFN = iC_sockFN;
    }

    regBufLen = REQUEST;			/* really ready to copy iecId's into regBuf  Nname...,Z\n */
    len = snprintf(regBuf, regBufLen, "N%s", iC_iccNM);	/* name header */
    cp = regBuf + len;
    regBufLen -= len;
    if (!iC_opt_G) {
	/********************************************************************
	 *  Generate registration string made up of all active PiFace I/O names
	 *  There is either 1 input or 1 output name or both per active PiFace
	 *******************************************************************/
	for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
	    assert(regBufLen > ENTRYSZ);	/* accomodates ",SIX123456,RQX123456,Z" */
	    for (iq = 0; iq < 3; iq++) {
		if ((np = pfp->s[iq].i.name) != NULL) {
		    assert(*np == IQ[iq]);
		    len = snprintf(cp, regBufLen, ",%c%s", SR[iq], np);
		    cp += len;			/* input send name or output receive name */
		    regBufLen -= len;
		}
	    }
	    /********************************************************************
	     *  Initialise and register active PiFaceCad
	     *******************************************************************/
	    if (pfp->pfa == 4 && pfp->intf == INTFA && pfp->Iname) {
		pifacecad_lcd_clear();
		pifacecad_lcd_display_on();	/* default is on */
		pifacecad_lcd_set_backlight(1);	/* default is on */
		len = snprintf(cp, regBufLen, ",R%s", pfCADname);
		cp += len;				/* output receive name */
		regBufLen -= len;
		pfCADpfp = pfp;			/* use for ACK */
	    }
	}
    }
    /********************************************************************
     *  Extend registration string with all active GPIO I/O names
     *******************************************************************/
    for (iq = 0; iq < 2; iq++) {
	for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
	    np = gep->Gname;
	    assert(regBufLen > ENTRYSZ);	/* accomodates ",SIX123456,RQX123456,Z" */
	    len = snprintf(cp, regBufLen, ",%c%s", SR[iq], np);
	    cp += len;				/* input send name or output receive name */
	    regBufLen -= len;
	}
    }
    /********************************************************************
     *  Send registration string made up of all active I/O names
     *******************************************************************/
    if (iC_debug & 0200)  fprintf(iC_outFP, "regBufLen = %d\n", regBufLen);
    snprintf(cp, regBufLen, ",Z");		/* Z terminator */
    if (iC_debug & 0200)  fprintf(iC_outFP, "register:%s\n", regBuf);
    if (iC_micro) iC_microPrint("send registration", 0);
    iC_send_msg_to_server(iC_sockFN, regBuf);		/* register IOs with iCserver */
    /********************************************************************
     *  Receive a channel number for each I/O name sent in registration
     *  Distribute read and/or write channels returned in acknowledgment
     *******************************************************************/
    if (iC_rcvd_msg_from_server(iC_sockFN, rpyBuf, REPLY) != 0) {	/* busy wait for acknowledgment reply */
	chS	sel;				/* can store either NULL or gpioIO* and un/mask */
	if (iC_micro) iC_microPrint("ack received", 0);
	if (iC_debug & 0200) fprintf(iC_outFP, "reply:%s\n", rpyBuf);
	if (iC_opt_B) {				/* prepare to execute iCbox -d */
	    len = snprintf(buffer, BS, "iCbox -dz%s -n %s-DI", (iC_debug & DQ) ? "q" : "", iC_iccNM);
	    b = 4;				/* initial number of tokens in buffer */
	    op = buffer + len;			/* optional host and port appended in iC_fork_and_exec() */
	    ol = BS - len;
	}
	cp = rpyBuf - 1;	/* increment to first character in rpyBuf in first use of cp */
	/********************************************************************
	 *  Channels for PiFace acknowledgments
	 *******************************************************************/
	for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
	    for (iq = 0; iq < 3; iq++) {
		pfq = &(pfp->s[iq]);
		if ((np = pfq->i.name) != NULL) {
		    assert(cp);			/* not enough channels in ACK string */
		    channel = atoi(++cp);	/* read channel from ACK string */
		    assert(channel > 0);
		    if (channel > topChannel) {
			topChannel = channel;
		    }
		    if (iC_debug & 0200) fprintf(iC_outFP, "PiFace %d %s  on channel %hu\n",
			pfp->pfa, np, channel);
		    pfq->channel = channel;	/* link send channel to PiFace (ignore receive channel) */
		    sel.un = (pfp - iC_pfL) | (iq & 0x02) << 2;	/* iC_pfL index un identifies PiFace GPIOB+ */
		    storeUnit(channel, sel);		/* link PiFace unit number to send channel */
		    if (iC_opt_B) {
			assert(ol > 10);
			len = snprintf(op, ol, " %s", np);	/* add I/O name to execute iCbox -d */
			b++;				/* count tokens in buffer */
			op += len;
			ol -= len;
			if ((mask = pfq->bmask) != 0xff) {
			    len = snprintf(op, ol, ",%d", mask);	/* mask out any PiFace bits not used */
			    op += len;
			    ol -= len;
			}
		    }
		    cp = strchr(cp, ',');
		}
	    }
	    /********************************************************************
	     *  Channel for PiFaceCAD string acknowledgment
	     *******************************************************************/
	    if (pfp == pfCADpfp) {
		assert(cp);			/* not enough channels in ACK string */
		channel = atoi(++cp);		/* read channel from ACK string */
		assert(channel > 0);
		if (channel > topChannel) {
		    topChannel = channel;
		}
		if (iC_debug & 0200) fprintf(iC_outFP, "PiFaceCAD %d %s on channel %hu\n",
		    pfp->pfa, pfCADname, channel);
		sel.un = pfp - iC_pfL;		/* iC_pfL index un identifies PiFace */
		storeUnit(channel, sel);	/* link PiFace unit number to send channel */
		pfCADchannel = channel;
		cp = strchr(cp, ',');
	    }
	}
	/********************************************************************
	 *  Channels for GPIO acknowledgments
	 *******************************************************************/
	for (iq = 0; iq < 2; iq++) {
	    for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
		np = gep->Gname;		/* not NULL assert previously */
		assert(cp);			/* not enough channels in ACK string */
		channel = atoi(++cp);		/* read channel from ACK string */
		assert(channel > 0);
		if (channel > topChannel) {
		    topChannel = channel;
		}
		if (iC_debug & 0200) {
		    fprintf(iC_outFP, "GPIO");
		    for (bit = 0; bit <= 7; bit++) {
			if ((gpio = gep->gpioNr[bit]) != 0xffff) {
			    fprintf(iC_outFP, ",%hu", gpio);
			} else {
			    fprintf(iC_outFP, ",d");
			}
		    }
		    fprintf(iC_outFP, "	%s  on channel %hu\n", np, channel);
		}
		gep->Gchannel = channel;	/* link send channel to GPIO (ignore receive channel) */
		sel.gep = gep;			/* identifies GPIO */
		storeUnit(channel, sel);	/* link GPIO element pointer to send channel */
		if (iC_opt_B) {
		    assert(ol > 14);
		    len = snprintf(op, ol, " %s", np);	/* add I/O name to execute iCbox -d */
		    b++;				/* count tokens in buffer */
		    op += len;
		    ol -= len;
		    if ((mask = gep->Gbmask) != 0xff) {
			len = snprintf(op, ol, ",%d", mask);	/* mask out any GPIO bits not used */
			op += len;
			ol -= len;
		    }
		}
		cp = strchr(cp, ',');
	    }
	}
	assert(cp == NULL);			/* Ack string matches Registration */
	/********************************************************************
	 *  ACK string complete
	 *******************************************************************/
	if (iC_opt_B && b > 4) {
	    iC_fork_and_exec(iC_string2argv(buffer, b));	/* fork iCbox -d */
	}
	if (iC_debug & 0200) fprintf(iC_outFP, "reply: top channel = %hu\n", topChannel);
    } else {
	iC_quit(QUIT_SERVER);			/* quit normally with 0 length message */
    }
    /********************************************************************
     *  Report results of argument analysis
     *  Port A outputs are inverted by hardware
     *  Port B inputs and outputs are direct
     *******************************************************************/
    if (iC_debug & 0300) {
	if (iC_npf) {
	    fprintf(iC_outFP, "Allocation for %d PiFace unit%s, global instance = \"%s\"\n",
		unit, unit == 1 ? "" : "s", iC_iidNM);
	    fprintf(iC_outFP, "	IEC-out	IEC-in	mask-i	IEC+out	mask+o	ch-out	ch-in	ch+out	pfa unit\n\n");
	    for (pfp = iC_pfL; pfp < &iC_pfL[unit]; pfp++) {
		if (pfp->Qname != NULL) {
		    assert(pfp->Qbmask == 0xff);		/* all 8 bits always output */
		    fprintf(iC_outFP, "	%c%s", pfp->Qinv ? ' ' : '~', pfp->Qname);	/* double inversion */
		} else {
		    fprintf(iC_outFP, "	Dummy");
		}
		if (pfp->Iname != NULL) {
		    if (pfp->Ibmask == 0xff) {
			fprintf(iC_outFP, "	%c%s	", pfp->Iinv ? '~' : ' ', pfp->Iname);
		    } else {
			fprintf(iC_outFP, "	%c%s	0x%02x", pfp->Iinv ? '~' : ' ', pfp->Iname, pfp->Ibmask);
		    }
		} else {
		    fprintf(iC_outFP, (pfp->QPname != NULL) ? "		" : "	Dummy	");
		}
		if (pfp->QPname != NULL) {
		    if (pfp->QPbmask == 0xff) {
			fprintf(iC_outFP, "	%c%s	", pfp->QPinv ? '~' : ' ', pfp->QPname);
		    } else {
			fprintf(iC_outFP, "	%c%s	0x%02x", pfp->QPinv ? '~' : ' ', pfp->QPname, pfp->QPbmask);
		    }
		} else {
		    fprintf(iC_outFP, "		");
		}
		fprintf(iC_outFP, "	%3hu	%3hu	%3hu	%2hu  %2d\n",
		    pfp->Qchannel, pfp->Ichannel, pfp->QPchannel, pfp->pfa, pfp - iC_pfL);
		if (pfp == pfCADpfp) {
		    fprintf(iC_outFP, "	 %s					%3hu			%2hu  %2d\n",
			pfCADname, pfCADchannel, pfp->pfa, pfp - iC_pfL);
		}
	    }
	    fprintf(iC_outFP, "\n");
	}
	if (gpioCnt) {
	    char *	iidPtr;
	    char *	iidSep;
	    fprintf(iC_outFP, "Allocation for %d GPIO element%s, global instance = \"%s\"\n",
		gpioCnt, gpioCnt == 1 ? "" : "s", iC_iidNM);
	    fprintf(iC_outFP, "	Bit IEC	inst	gpio	channel\n\n");
	    for (iq = 0; iq < 2; iq++) {
		for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
		    strcpy(buffer, gep->Gname);		/* retrieve name[-instance] */
		    if ((iidPtr = strchr(buffer, '-')) != NULL) {
			iidSep = "-";
			*iidPtr++ = '\0';		/* separate name and instance */
		    } else {
			iidSep = iidPtr = "";		/* null or global instance */
		    }
		    for (bit = 0; bit <= 7; bit++) {
			if ((gpio = gep->gpioNr[bit]) != 0xffff) {	/* saved gpio number for this bit */
			    fprintf(iC_outFP, "	%c%s.%hu	%s%s	%3hu	%3hu\n",
				gep->Ginv & iC_bitMask[bit] ? '~' : ' ',
				buffer, bit, iidSep, iidPtr, gpio, gep->Gchannel);
			}
		    }
		}
		fprintf(iC_outFP, "\n");
	    }
	}
    }
    cp = regBuf;
    regBufLen = REPLY;
    if (iC_debug & 0100) {
	op = buffer;
	ol = BS;
    }
    /********************************************************************
     *  Generate PiFace initialisation inputs and outputs for active PiFaces
     *  Send possible TCP/IP after GPIO done
     *******************************************************************/
    if (!iC_opt_G) {
	assert(gpio25FN > 0);
	if (iC_debug & 0200) fprintf(iC_outFP, "### Initialise %d unit(s)\n", iC_npf);
	gpio_read(gpio25FN);			/* dummy read to clear interrupt on /dev/class/gpio25/value */
	if (iC_micro) iC_microPrint("SPI initialise", 0);
	for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
	    if (((pfa = pfp->pfa) != 4 || pfp->intf != INTFA) && pfp->Qname) {	/* PiFace */
		writeByte(pfp->spiFN, pfa, OLATA, pfp->Qinv);	/* normally write inverted data to PiFace A output */
		pfp->Qval = 0;			/* force all output bits (TODO not used) */
	    }					/* PiFaceCAD output does not need initialising */
	    if (pfp->Iname) {
		assert(regBufLen > 11);		/* fits largest data telegram */
		val = readByte(pfp->spiFN, pfa, pfp->inpr);	/* read registered PiFace B/A at initialisation */
		/* by default do not invert PiFace inputs - they are inverted with -I */
		len = snprintf(cp, regBufLen, ",%hu:%d",	/* data telegram */
				pfp->Ichannel,
				(val ^ pfp->Iinv) & pfp->Ibmask
			      );
		cp += len;
		regBufLen -= len;
		if (iC_debug & 0100) {
		    len = snprintf(op, ol, " P%d %s(INI)", pfp->pfa, pfp->Iname);	/* source name */
		    op += len;
		    ol -= len;
		}
		pfp->Ival = val;		/* initialise input for comparison */
	    }
	    if (pfp->QPname) {							/* PiFace GPIOB+ */
		writeByte(pfp->spiFN, pfa, OLATB, pfp->QPinv);	/* normally write non-inverted data to PiFace B output */
		pfp->QPval = 0;			/* force all output bits (TODO not used) */
	    }
	}
    }
    /********************************************************************
     *  Extend with GPIO IXn initialisation inputs
     *  There may be no GPIOs or only GPIO outputs - nothing to initialise
     *******************************************************************/
    for (gep = iC_gpL[1]; gep; gep = gep->nextIO) {	/* IXn inputs only */
	val = 0;
	for (bit = 0; bit <= 7; bit++) {
	    if ((gpio = gep->gpioNr[bit]) != 0xffff) {
		if ((n = gpio_read(gep->gpioFN[bit])) != -1) {
		    if (n) val |= iC_bitMask[bit];
		} else {
		    fprintf(iC_errFP, "WARNING: %s: GPIO %hu returns invalid value -1 (not 0 or 1 !!)\n",
			iC_progname, gpio);		/* should not happen */
		}
	    }
	}
	assert(gep->Gname);
	assert(regBufLen > 11);			/* fits largest data telegram */
	/* by default do not invert GPIO inputs - they are inverted with -I */
	len = snprintf(cp, regBufLen, ",%hu:%d", gep->Gchannel, val ^ gep->Ginv);	/* data telegram */
	cp += len;
	regBufLen -= len;
	if (iC_debug & 0100) {
	    len = snprintf(op, ol, " G %s(INI)", gep->Gname);	/* source name */
	    op += len;
	    ol -= len;
	}
	gep->Gval = val;			/* initialise input for comparison */
    }
    /********************************************************************
     *  Send IXn inputs if any - to iCsever to initialise receivers
     *******************************************************************/
    if (cp > regBuf) {
	iC_send_msg_to_server(iC_sockFN, regBuf+1);	/* send data telegram(s) to iCserver (skip initial ',') */
	if (iC_debug & 0100) fprintf(iC_outFP, "P: %s:	%s	<%s\n", iC_iccNM, regBuf+1, buffer);
    }
    /********************************************************************
     *  Write GPIO QXn initialisation outputs
     *******************************************************************/
    for (gep = iC_gpL[0]; gep; gep = gep->nextIO) {	/* QXn outputs only */
	val = 0;				/* initialise with logical 0 */
	gep->Gval = gep->Ginv ^ 0xff;		/* force all output bits */
	writeGPIO(gep, gep->Gchannel, val);
    }
    if (argc != 0) {
	/********************************************************************
	 *  -R and argv points to auxialliary app + arguments
	 *  This app has now connected to iCserver and registered all its I/Os
	 *******************************************************************/
	assert(argv && *argv);
	iC_fork_and_exec(argv);			/* run auxiliary app */
    }
    /********************************************************************
     *  Clear and then set all bits to wait for interrupts
     *******************************************************************/
    FD_ZERO(&infds);				/* should be done centrally if more than 1 connect */
    FD_ZERO(&ixfds);				/* should be done centrally if more than 1 connect */
    FD_SET(iC_sockFN, &infds);			/* watch sock for inputs */
    if (iC_npf) FD_SET(gpio25FN, &ixfds);	/* watch GPIO25 for out-of-band input - do after iC_connect_to_server() */
    /********************************************************************
     *  Set all GPIO IXn input bits for interrupts
     *******************************************************************/
    for (gep = iC_gpL[1]; gep; gep = gep->nextIO) {	/* IXn inputs only */
	for (bit = 0; bit <= 7; bit++) {
	    if ((gpio = gep->gpioNr[bit]) != 0xffff) {
		assert(gep->gpioFN[bit] > 0);		/* make sure it has been opened */
		FD_SET(gep->gpioFN[bit], &ixfds);	/* watch GPIO N for out-of-band input */
	    }
	}
    }
    if ((iC_debug & DZ) == 0) FD_SET(0, &infds);	/* watch stdin for inputs unless - FD_CLR on EOF */
    if (iC_debug & 0200) fprintf(iC_outFP, "iC_sockFN = %d	gpio25FN = %d spidFN[0] = %d spidFN[1] = %d\n",
	iC_sockFN, gpio25FN, spidFN[0], spidFN[1]);
    /********************************************************************
     *  External input (TCP/IP via socket, SIO from PiFace, GPIO and STDIN)
     *  Wait for input in a select statement most of the time
     *  750 ms Timer is only switched on for left shifting lcd text
     *******************************************************************/
    slr = 0;
    cbc = 0x03;					/* first input will switch to cursor/blink off */
    for (;;) {
	if ((retval = iC_wait_for_next_event(&infds, &ixfds, toCntp)) == 0)
	{
	    if (toCnt.tv_sec == 0 && toCnt.tv_usec == 0) {
		toCnt = toC750;			/* re-initialise timeout value */
		if (slr & 0x08) {
		    pifacecad_lcd_move_left();	/* move text left every 750 ms */
		} else {
		    pifacecad_lcd_move_right();	/* move text right every 750 ms */
		}
	    }
	} else if (retval > 0) {
	    /********************************************************************
	     *  TCP/IP input from iCserver
	     *******************************************************************/
	    if (FD_ISSET(iC_sockFN, &iC_rdfds)) {
		if (iC_rcvd_msg_from_server(iC_sockFN, rpyBuf, REPLY) != 0) {
		    if (iC_micro) iC_microPrint("TCP input received", 0);
		    assert(Units);
		    cp = rpyBuf - 1;		/* increment to first character in rpyBuf in first use of cp */
		    if (isdigit(rpyBuf[0])) {
			char *	cpe;
			char *	cps;
			chS	sel;		/* union can store either int un or gpioIO * gep */

			do {
			    if ((cpe = strchr(++cp, ',')) != NULL) { /* find next comma in input */
				*cpe = '\0';	/* split off leading comma separated token */
			    }
			    if (
				(cps = strchr(cp, ':')) != NULL &&	/* strip only first ':' separating channel and data */
				(channel = (unsigned short)atoi(cp)) > 0 &&
				channel <= topChannel
			    ) {
				val = atoi(++cps);
				if (channel == pfCADchannel) {
				    /********************************************************************
				     *  TCP/IP string output for PiFaceCAD
				     *******************************************************************/
				    cp = cps - 1;		/* on : before display string */
				    while ((cp = strchr(cp+1 , '\036')) != NULL) {	/* ASCII RS */
					*cp = ',';		/* replace every RS by a comma */
				    }
				    writePiFaceCAD(cps, 0);	/* display on local PiFaceCAD */
				} else
				if ((un = (sel = Units[channel]).un) < 16) {
				    iq = (un & 0x08) >> 2;	/* selects OLATA or OLATB */
				    un &= 0x07;			/* Unit # for pfa */
				    pfp = &iC_pfL[un];
				    pfa = pfp->pfa;
				    pfq = &(pfp->s[iq]);
				    /********************************************************************
				     *  TCP/IP output for a PiFace or PiFaceCAD
				     *******************************************************************/
				    if (pfq->i.name) {				/* is output registered? */
					if (iC_debug & 0100) fprintf(iC_outFP, "P: %s:	%hu:%d P%d	> %s\n",
					    iC_iccNM, channel, (int)val, pfa, pfq->i.name);
					if (pfa != 4 || pfp->intf != INTFA) {	/* PiFace */
					    /********************************************************************
					     *  Direct output to a PiFace
					     *******************************************************************/
					    writeByte(pfp->spiFN, pfa,
							iq ? OLATB : OLATA,
							(val & pfq->bmask) ^ pfq->inv
						     );	/* normally write inverted data to PiFace A output */
					} else if ((m = val ^ pfq->val) != 0) {
					    /********************************************************************
					     *  PiFaceCAD has no digital outputs
					     *  These output bits manipulate the text display by software
					     *  The logical value of the bits are used - no inversion applies
					     *******************************************************************/
					    if (m & 0x07) {
						/* TODO */
					    }
					    if (m & 0x18) {			/* shift left or right */
						if ((slr = val & 0x18) != 0) {	/* start 50 ms timeout */
						    toCnt = toCini;		/* set initial timeout value */
						    toCntp = &toCnt;		/* for regular shift */
						} else {			/* slr holds shift direction */
						    toCntp = NULL;		/* stop 750 ms timeout */
						}				/* for both off */
					    }
					    if ((m & 0x20) && (val & 0x20)) {
						switch (cbc = (cbc + 1) & 0x03) {	/* cbc holds cursor and blink state */
						case 0x01:
						    pifacecad_lcd_cursor_on();
						    pifacecad_lcd_blink_off();
						    break;
						case 0x02:
						    pifacecad_lcd_cursor_off();
						    pifacecad_lcd_blink_on();
						    break;
						case 0x03:
						    pifacecad_lcd_cursor_on();
						    pifacecad_lcd_blink_on();
						    break;
						case 0x00:
						default:
						    pifacecad_lcd_cursor_off();
						    pifacecad_lcd_blink_off();		/* default is both off */
						    break;
						}
					    }
					    if (m & 0x40) {
						if (val & 0x40) {
						    pifacecad_lcd_display_off();
						} else {
						    pifacecad_lcd_display_on();		/* default is on */
						}
					    }
					    if (m & 0x80) {
						pifacecad_lcd_set_backlight(!(val & 0x80));	/* default is on */
					    }
					    pfq->val = val;		/* store change for comparison */
					}
				    } else {
					fprintf(iC_errFP, "WARNING: %s: unit = %d, iq = %d, channel = %hu, val = %d; no output registered - should not happen\n",
					    iC_progname, un, iq, channel, val);		/* should not happen */
				    }
				} else {
				    /********************************************************************
				     *  TCP/IP output for a GPIO
				     *******************************************************************/
				    writeGPIO(sel.gep, channel, val);
				}
			    } else {
				fprintf(iC_errFP, "channel = %hu, topChannel = %hu\n", channel, topChannel);
				goto RcvWarning;
			    }
			} while ((cp = cpe) != NULL);		/* next token if any */
		    }
		    else {
		      RcvWarning:
			fprintf(iC_errFP, "WARNING: %s: received '%s' from iCserver ???\n", iC_iccNM, rpyBuf);
		    }
		} else {
		    iC_quit(QUIT_SERVER);	/* quit normally with 0 length message from iCserver */
		}
	    }	/* end of TCP/IP input */
	    /********************************************************************
	     *  GPIO25 interrupt means SIO input from a PiFace
	     *******************************************************************/
	    if (gpio25FN > 0 && FD_ISSET(gpio25FN, &iC_exfds)) {	/* watch for out-of-band GPIO25 input */
		m1 = 0;
		if (iC_micro) iC_microPrint("SPI input received", 0);
		cp = regBuf;
		regBufLen = REPLY;
		if (iC_debug & 0300) {
		    op = buffer;
		    ol = BS;
		}
		do {
		    /********************************************************************
		     *  Scan PiFace units for input interrupts (even those not used)
		     *  More interrupts can arrive for PiFace's already scanned, especially
		     *  with bouncing mechanical contacts - repeat the scan until GPIO25 == 1
		     *******************************************************************/
		    m = 0;
		    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {
			if (readByte(pfp->spiFN, pfp->pfa, pfp->intf)) {	/* interrupt flag on this unit ? */
			    assert(regBufLen > 11);				/* fits largest data telegram */
			    val = readByte(pfp->spiFN, pfp->pfa, pfp->inpr);	/* read PiFace B/A at interrupt */
			    if (val != pfp->Ival) {
				if (pfp->Iname) {
				    /* by default do not invert PiFace inputs - they are inverted with -I */
				    len = snprintf(cp, regBufLen, ",%hu:%d",	/* data telegram */
						    pfp->Ichannel,
						    (val ^ pfp->Iinv) & pfp->Ibmask
						  );
				    cp += len;
				    regBufLen -= len;
				    if (iC_debug & 0300) {
					len = snprintf(op, ol, " P%d %s", pfp->pfa, pfp->Iname); /* source name */
					op += len;
					ol -= len;
				    }
				} else if (iC_debug & 0100) {
				    fprintf(iC_outFP, "P: %s: %d input on unregistered PiFace %d\n", iC_iccNM, val, pfp->pfa);
				}
				pfp->Ival = val;				/* store change for comparison */
			    }
			    m++;						/* count INTF interrupts found */
			}
		    }
		    m1++;
		    if ((val = gpio_read(gpio25FN)) == -1) {
			perror("GPIO25 read");
			fprintf(iC_errFP, "ERROR: %s: GPIO25 read failed\n", iC_progname);
			break;
		    }
		} while (val != 1);			/* catch interrupts which came in during for loop */
		if (cp > regBuf) {
		    iC_send_msg_to_server(iC_sockFN, regBuf+1);			/* send data telegram(s) to iCserver */
		    if (iC_debug & 0100) fprintf(iC_outFP, "P: %s:	%s	<%s\n", iC_iccNM, regBuf+1, buffer);
		}
		if ((iC_debug & (DQ | 0200)) == 0200) {
		    if (m1 > 1){
			fprintf(iC_errFP, "WARNING: %s: GPIO25 interrupt %d loops %d changes \"%s\"\n", iC_progname, m1, m, regBuf+1);
		    } else if (m == 0) {	/* for some reason this happens occasionaly - no inputs are missed though */
			fprintf(iC_errFP, "WARNING: %s: GPIO25 interrupt and no INTF set on PiFaces\n", iC_progname);
		    }
		    *(regBuf+1) = '\0';			/* clean debug output next time */
		}
	    }	/*  end of GPIO25 interrupt */
	    /********************************************************************
	     *  GPIO N interrupt means GPIO n input
	     *******************************************************************/
	    m1 = 0;
	    cp = regBuf;
	    regBufLen = REPLY;
	    if (iC_debug & 0100) {
		op = buffer;
		ol = BS;
	    }
	    for (gep = iC_gpL[1]; gep; gep = gep->nextIO) {	/* IXn inputs only */
		assert(regBufLen > 11);		/* fits largest data telegram */
		m = 0;
		val = gep->Gval;
		for (bit = 0; bit <= 7; bit++) {
		    if ((gpio = gep->gpioNr[bit]) != 0xffff) {
			assert(gep->gpioFN[bit] > 0);
			if (FD_ISSET(gep->gpioFN[bit], &iC_exfds)) {	/* any out-of-band GPIO N input */
			    if ((n = gpio_read(gep->gpioFN[bit])) == 0) {
				val &= ~(1 << bit);
			    } else if (n == 1) {
				val |= 1 << bit;
			    } else {
				fprintf(iC_errFP, "WARNING: %s: GPIO %hu returns invalid value %d (not 0 or 1 !!)\n",
				    iC_progname, gpio, n);		/* should not happen */
			    }
			    m++;
			    if (m1++ == 0 && iC_micro) iC_microPrint("GPIO input received", 0);
			}
		    }
		}
		if (m && val != gep->Gval) {
		    /* by default do not invert GPIO inputs - they are inverted with -I */
		    len = snprintf(cp, regBufLen, ",%hu:%d", gep->Gchannel, val ^ gep->Ginv); /* data telegram */
		    cp += len;
		    regBufLen -= len;
		    if (iC_debug & 0100) {
			len = snprintf(op, ol, " G %s", gep->Gname); /* source name */
			op += len;
			ol -= len;
		    }
		    gep->Gval = val;		/* store change for comparison */
		}
	    }	/*  end of GPIO N interrupt */
	    if (cp > regBuf) {
		iC_send_msg_to_server(iC_sockFN, regBuf+1);	/* send data telegram(s) to iCserver */
		if (iC_debug & 0100) fprintf(iC_outFP, "P: %s:	%s	<%s\n", iC_iccNM, regBuf+1, buffer);
	    }
	    /********************************************************************
	     *  STDIN interrupt
	     *******************************************************************/
	    if (FD_ISSET(0, &iC_rdfds)) {
		if (fgets(iC_stdinBuf, REPLY, stdin) == NULL) {	/* get input before TX0.1 notification */
		    FD_CLR(0, &infds);			/* ignore EOF - happens in bg or file - turn off interrupts */
		    iC_stdinBuf[0] = '\0';		/* notify EOF to iC application by zero length iC_stdinBuf */
		}
		if ((c = iC_stdinBuf[0]) == 'q' || c == '\0') {
		    iC_quit(QUIT_TERMINAL);		/* quit normally with 'q' or ctrl+D */
		} else if (c == 't') {
		    iC_debug ^= 0100;			/* toggle -t flag */
		} else if (c == 'm') {
		    iC_micro ^= 1;			/* toggle micro */
		} else if (c == 'T') {
		    iC_send_msg_to_server(iC_sockFN, "T");	/* print iCserver tables */
#ifdef	TRACE
		} else if (c == 'i') {
		    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {	/* report MCP23S17 IOCON, GPINTEN */
			fprintf(iC_outFP, "%s: un = %d pfa = %d IOCON = 0x%02x GPINTEN = 0x%02x\n",
			    pfp->Iname, pfp - iC_pfL, pfp->pfa,
			    readByte(pfp->spiFN, pfp->pfa, IOCON),
			    readByte(pfp->spiFN, pfp->pfa, pfp->intf == INTFB ? GPINTENB : GPINTENA));
		    }
		} else if (c == 'I') {
		    for (pfp = iC_pfL; pfp < &iC_pfL[iC_npf]; pfp++) {	/* restore MCP23S17 IOCON, GPINTEN */
			fprintf(iC_outFP, "%s: un = %d pfa = %d restore IOCON <== 0x%02x GPINTEN <== 0x%02x\n",
			    pfp->Iname, pfp - iC_pfL, pfp->pfa,
			    IOCON_SEQOP | IOCON_HAEN | ((iC_npf == 1) ? 0 : IOCON_ODR), 0xff);
			writeByte(pfp->spiFN, pfp->pfa, IOCON, IOCON_SEQOP | IOCON_HAEN | ((iC_npf == 1) ? 0 : IOCON_ODR));
			writeByte(pfp->spiFN, pfp->pfa, pfp->intf == INTFB ? GPINTENB : GPINTENA, 0xff);
		    }
#endif	/* TRACE */
		} else if (c != '\n') {
		    fprintf(iC_errFP, "no action coded for '%c' - try t, m, T, i, I, or q followed by ENTER\n", c);
		}
	    }	/*  end of STDIN interrupt */
	} else {
	    perror("ERROR: select failed");				/* retval -1 */
	    iC_quit(SIGUSR1);		/* error quit */
	}
    }
} /* main */

/********************************************************************
 *
 *	Initalise and expand dynamic array Units[] as necessary
 *	Store PiFace unit numbers in Units[] indexed by channel
 *	Alternatively store gpioIO * to current GPIO element
 *
 *******************************************************************/

static void
storeUnit(unsigned short channel, chS sel)
{
    while (channel >= ioUnits) {
	Units = (chS *)realloc(Units,		/* initially NULL */
	    (ioUnits + IOUNITS) * sizeof(chS));
	assert(Units);
	memset(&Units[ioUnits], '\0', IOUNITS * sizeof(chS));
	ioUnits += IOUNITS;			/* increase the size of the array */
	if (iC_debug & 0200) fprintf(iC_outFP, "storeUnit: Units[%d] increase\n", ioUnits);
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "storeUnit: Units[%d] <= %d\n", channel, sel.un);
    Units[channel] = sel;			/* store PiFace unit number gpioIO * */
} /* storeUnit */

/********************************************************************
 *
 *	Write changed GPIO output bits
 *
 *******************************************************************/

static void
writeGPIO(gpioIO * gep, unsigned short channel, int val)
{
    int			diff;
    int			fd;
    int			mask;
    unsigned short	bit;

    assert(gep && gep->Gname && *gep->Gname == 'Q');	/* make sure this is really a GPIO output */
    if (iC_debug & 0100) fprintf(iC_outFP, "P: %s:	%hu:%d G	> %s\n",
	iC_iccNM, channel, (int)val, gep->Gname);
    val ^= gep->Ginv;			/* normally write non-inverted data to GPIO output */
    diff = val ^ gep->Gval;		/* bits which are going to change */
    assert ((diff & ~0xff) == 0);	/* may receive a message which involves no change */
    while (diff) {
	bit = iC_bitIndex[diff];	/* returns 0 - 7 for values 1 - 255 (avoid 0) */
	mask  = iC_bitMask[bit];	/* returns hex 01 02 04 08 10 20 40 80 */
	if ((fd = gep->gpioFN[bit]) != -1) {	/* is GPIO pin open ? */
	    gpio_write(fd, val & mask);	/* yes - write to GPIO (does not need to be a bit in pos 0) */
	} else if ((iC_debug & 0300) == 0300) {
	    fprintf(iC_errFP, "WARNING: %s: no GPIO associated with %s.%hu\n",
		iC_progname, gep->Gname, bit);
	}
	diff &= ~mask;			/* clear the bit just processed */
    }
    gep->Gval = val;			/* ready for next output */
} /* writeGPIO */

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

    errno = 0;				/* to distinguish success/failure after call */
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
 *	Unexport GPIOs and close PiFaces
 *	Clear gpiosp->u.used bits for GPIOs and A/D channels used in this app
 *
 *******************************************************************/

static int
termQuit(int sig)
{
    if (iC_opt_P) {
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
	for (iq = 0; iq < 2; iq++) {
	    for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
		for (bit = 0; bit <= 7; bit++) {
		    if ((gpio = gep->gpioNr[bit]) != 0xffff) {
			/********************************************************************
			 *  Execute the SUID root progran iCgpioPUD(gpio, pud) to turn off pull-up/down
			 *******************************************************************/
			if (iq == 1) iC_gpio_pud(gpio, BCM2835_GPIO_PUD_OFF);	/* inputs only */
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
	 *  Open and lock the auxiliary file ~/.iC/gpios.used again
	 *  Other apps may have set used bits since this app was started
	 *******************************************************************/
	if ((gpiosp = openLockGpios(0)) == NULL) {
	    fprintf(iC_errFP, "ERROR: %s: in openLockGpios()\n",
		iC_progname);
	    return (SIGUSR1);		/* error quit */
	}
	gpiosp->u.used &= ~ownUsed;		/* clear all bits for GPIOs and A/D channels used in this app */
	if (writeUnlockCloseGpios() < 0) {	/* unlink (remove) ~/.iC/gpios.used if gpios->u.used and oops is 0 */
	    fprintf(iC_errFP, "ERROR: %s: in writeUnlockCloseGpios()\n",
		iC_progname);
	    return (SIGUSR1);		/* error quit */
	}
	if (iC_debug & 0200) fprintf(iC_outFP, "=== End Unexport GPIOs and close PiFaces ===\n");
    } /* iC_opt_P */
    return (sig);			/* finally quit */
} /* termQuit */
#endif	/* defined(RASPBERRYPI) && defined(TCP) && !defined(_WINDOWS) */

/* ############ POD to generate iCpiFace man page ############################

=encoding utf8

=head1 NAME

iCpiFace - real digital I/O on a Raspberry Pi for the iC environment

=head1 SYNOPSIS

 iCpiFace | iCpiGPIO
          [-GBIftmqzh][ -s <host>][ -p <port>][ -n <name>][ -i <inst>]
          [ -d <deb>]
          [ [~]<IEC>[-<IEC>|+[<mask>]][:<pfa>][-<inst>] ...]
          [ [~]<IEC>,<gpio>[,<gpio>,...][-<inst>] ...]
          [ [~]<IEC>.<bit>,<gpio>[-<inst>] ...]    # at least 1 IEC argument
          [ -R <aux_app>[ <aux_app_argument> ...]] # must be last arguments
    -s host IP address of server    (default 'localhost')
    -p port service port of server  (default '8778')
    -i inst instance of this client (default '') or 1 to 3 digits
    -G      service GPIO I/O only - block PiFaces and PiFace arguments
            calling with alternate name "iCpiGPIO" has the same effect
    -B      start iCbox -d to monitor active PiFace and/or GPIO I/O
            A 1 to PiFace and GPIO outputs generates hi - LED and relay off
            A 0 to PiFace and GPIO outputs generates lo - LED and relay on
    -I      invert all PiFace and GPIO inputs and outputs
            PiFace and GPIO input circuits are normally hi - 1 and
            when a switch on the input is pressed it goes lo - 0. Therefore
            it is appropriate to invert inputs and outputs. When inverted
            a switch pressed on an input generates a 1 for the IEC inputs and
            a 1 on an IEC output turns a LED and relay on, which is natural.
    NOTE: the supplied PiFace driver inverts outputs but not inputs - go figure
    -W GPIO number used by the w1-gpio kernel module (default 4, maximum 31).
            When the GPIO with this number is used in this app, iCtherm is
            permanently blocked to avoid Oops errors in module w1-gpio.
    -f      force use of all interrupting gpio inputs - in particular
            force use of gpio 25 - some programs do not unexport correctly.
                      PIFACE arguments
          For each PiFace or PiFaceCAD connected to a Raspberry Pi
          8 bits of digital input and 8 bits of digital output with
          known IEC numbers are generated.
    IEC      X1 X2 IX8 QX9  (for IEC-1131 names - see iCserver man page)
          The argument Xn can optionally be preceded by either I for
          inputs or Q for outputs. The IEC-1131 IEC's generated are
          followed by a bit selector in the range .0 to .7.
          For the above list X1 X2 IX8 QX9 the following is generated:
            inputs IX1.0 - .7 and outputs QX1.0 - .7 for PiFace unit 0
            inputs IX2.0 - .7 and outputs QX2.0 - .7 for PiFace unit 1
            inputs IX8.0 - .7 and outputs QX9.0 - .7 for PiFace unit 2
          A range of IEC's can be specified with X0-X3, IX4-IX6 or QX7-QX9
          Actual PiFace unit addresses in the range 0 - 7 are selected by
          jumpers JP1 and JP2 as well as CE0/CE1 (JP1 - JP8 on the PiRack).
            The program scans which addresses are present and associates
          them with unit numbers in ascending PiFace address order.
          Normally IEC addresses will be chosen to coincide with PiFace
          unit numbers, but this is not mandatory. What is mandatory is that
          there are enough PiFaces to cover all the IEC's in the parameters.
    IEC:<pfa> Associate a particular IEC or IEC range with a specific PiFace
          unit address or address range. <pfa> must be one of numbers 0 - 7
          and a PiFace with that unit address must be available.
            By choosing X1-X3:1 for 3 PiFaces, the IEC numbers will
          be the same as the PiFace addresses which are 1 to 3. This
          would be good programming practice. On the other hand there
          are good reasons to choose different IEC's to fit in with
          the iC application code and other I/O's.
    D ID QD may be used as dummy IEC's instead of Xn IXn QXn to exclude a
          PiFace unit or an individual PiFace input or output.
                      PIFACE GPIOB+ extra arguments
    QXn+  The whole GPIOB register is a 2nd output rather than an input
    QXn+<mask>  Only the masked bits of GPIOB register are output bits
    IXn+        The whole GPIOB register is an input (same as IXn)
    IXn+<mask>  Only the masked bits of GPIOB register are input bits
          <mask> for bits 0 - 3 may be expressed as 0b1111, 0xf, 017 or 15
          The masks for output and input may not both contain the same bit
    QXn+:<pfa> etc Associate a particular IEC+ with a specific PiFace address
          PiFace GPIOB+ selections cannot be part of a range.
                      GPIO arguments
          Any IEC I/Os IXx.y and QXx.y which are to be linked to GPIO
          inputs or outputs must be named in the argument list as follows:
    IEC,<gpio-list> Associate the bits of a particular input or output IEC
          with a list of gpio numbers.  The IEC must start with either
          I or Q in this case and no IEC range is allowed.  Up to 8
          gpio numbers can be given in a comma separated list. The
          first gpio number will be aassociated with bit 0, the second
          with bit 1 etc and the eighth with bit 7. If the list is
          shorter than 8 the trailing bits are not used. The letter
          'd' can be used in the list instead of a gpio number to mark
          that bit as unused.  eg. IX3,17,18,19,20,d,d,21 associates
          IX3.0 - IX3.3 with GPIO17 - GPIO20 and IX3.6 with GPIO21.
    IEC.<bit>,<gpio> Alternatively each input or ourput <bit> 0 to 7 can
          be associated with one gpio nunber eg. IX3.0,17 QX3.7,18
                      COMMON extensions
    ~IEC  Each individual IEC or range can be preceded by a tilde '~',
          which inverts individual IEC inputs or outputs (see -I flag).
          If all IEC inputs and outputs are already inverted with -I
          ~ inverts it again, which makes it non-inverted. Individual
          bits for one IEC can be inverted or left normal for GPIO IEC's
          by declaring the IEC more than once with non-overlapping
          gpio bit lists or single bits. (Not possible for PiFaces)
    IEC-inst Each individual IEC or range can be followed by -<inst>,
          where <inst> consists of 1 to 3 numeric digits.
          Such an appended instance code takes precedence over the
          general instance specified with the -i <inst> option above.
    NOTE: there must be at least 1 PiFace or GPIO IEC argument.
          No IEC arguments are generated automatically for iCpiFace.
                      CAVEAT
    Only one instance of iCpiFace or an app with IEC parameters may be run
    and all GPIOs, PiFaces and an optional PiFaceCAD must be controlled by
    this one instance. If two instances were running, the common interrupts
    would clash. Also no other program controlling GPIOs and PiFaces like
    'PiFace Digital Emulator' may be run at the same time as this application.
    An exception is iCpiPWM which controls GPIOs by DMA and not by interrupts.
    Another exception is iCtherm which controls GPIO 4 by the 1Wire interface.
    Care is taken that any GPIOs or PiFaces used in one app, iCpiFace, iCpiPWM
    or even iCtherm do not clash with another app (using ~/.iC/gpios.used).
                      DEBUG options
    -t      trace arguments and activity (equivalent to -d100)
         t  at run time toggles gate activity trace
    -m      display microsecond timing info
         m  at run time toggles microsecond timing trace
    -d deb  +1   trace TCP/IP send actions
            +2   trace TCP/IP rcv  actions
            +10  trace SIO  input  actions
            +20  trace SIO  output actions
            +100 show arguments
            +200 show more debugging details
            +400 exit after initialisation
    -q      quiet - do not report clients connecting and disconnecting
    -z      block keyboard input on this app - used by -R
    -h      this help text
         T  at run time displays registrations and equivalences
         q  or ctrl+D  at run time stops iCpiFace
                      AUXILIARY app
    -R <app ...> run auxiliary app followed by -z and its arguments
                 as a separate process; -R ... must be last arguments.

=head1 DESCRIPTION

B<iCpiFace> is an I/O client for iCserver in the immediate C
environment of a Raspberry Pi handling real 8 bit inputs and 8
bit outputs for each PiFace controllers attached to a Raspberry Pi
or from a number of direct GPIO pins on the Raspberry Pi or both.
It also handles 8 bits of push button input and control of an LCD
Text panel for a PiFaceCAD.

All GPIO pins on a Raspberry Pi A, B or B+ may be selected as either
an input bit or output bit independent of whether PiFaces are present
or not, except GPIO 7-11 and GPIO 25 if PiFaces are also processed.
All GPIO inputs are handled by interrupts.

If no PiFace controllers are found or if the program is called with -G
or with the alternate name B<iCpiGPIO>, only GPIO pins will be handled.

A maximum of 8 PiFace controllers can be handled altogether - either
one plugged directly into a Raspberry Pi - or 4 on one PiRack -
7 on 2 PiRacks - 8 require 3 PiRacks.

One of the PiFaces (at address 4) can be a PiFaceCAD, which has 8
button inputs and a 2 line x 16 character LCD display. The PiFaceCAD
fits neatly as the last controller on a PiRack or can be mounted on
its own on the Raspberry Pi.

Similar I/O operations for both PiFaces and Raspberry Pi GPIOs can be
obtained by linking the driver directly to an iC app, in which case
these I/O operations are not handled by iCserver.

=head1 TECHNICAL BACKGROUND

The direct GPIO connections on the Raspberry Pi:

 0) The Raspberry Pi A or B brings out 17 GPIO signals on a 26 pin
    connecteor, 5 of which double up to do the SPI interface, 2 for
    a UART, leaving 10 for general purpose input/output (there are
    4 more on a 2nd connector, which is not normally fitted). The
    Raspberry Pi B+ brings out 9 more GPIO signals on its 40 pin
    connector making a total of 19 free GPIO pins.  The Linux "sysfs"
    can access the value of these GPIO pins from user space and more
    importantly can generate interrupts from them.

    All GPIO pin I/O is handled by the Linux "sysfs" and its interrupts.

    For details see:
    http://www.auctoris.co.uk/2012/07/19/gpio-with-sysfs-on-a-raspberry-pi/

    The "sysfs" export, unexport, direction, edge and value commands
    can be run by normal (non root) users if they belong to group gpio.
    To achieve this the true path that the link /sys/class/gpio/gpio*
    points to must be adjusted to root:gpio by a system script.
    The path in the kernel and the matching udev script has changed a
    number of times between 2012 and 2015 causing much confusion.

    The following change works for me as of December 2015
    https://github.com/raspberrypi/linux/issues/1117

    popcornmix commented on Aug 25 2015
    Looks like something may have changed in the later kernel where
    /sys/devices/soc/*.gpio/gpio is now /sys/devices/platform/soc/*.gpio/gpio
    Can you try editing:
    /lib/udev/rules.d/60-python-pifacecommon.rules and
    /lib/udev/rules.d/60-python3-pifacecommon.rules (if they both exist) to be:

 KERNEL=="spidev*", GROUP="spi", MODE="0660"
 SUBSYSTEM=="gpio*", PROGRAM="/bin/sh -c 'chown -R root:gpio /sys/class/gpio &&\
 chmod -R 770 /sys/class/gpio; chown -R root:gpio /sys/devices/virtual/gpio &&\
 chmod -R 770 /sys/devices/virtual/gpio;\
 chown -R root:gpio /sys/devices/platform/soc/*.gpio/gpio &&\
 chmod -R 770 /sys/devices/platform/soc/*.gpio/gpio'"

    Another complication is, that after a "sysfs" export, the first access
    to the direction (or any other) register takes about 200 ms. I have
    taken care of this by retrying  every 50 ms for a max of 2.5 seconds
    before giving up. (Discussed in a number of forums).

Lines marked with ** apply only to PiFace Relay+ and PiFace Relay Extra.

The I/O connections on any PiFace board are controlled from the
Raspberry Pi via four different paths:

 1) the kernel SPI system can write and read to and from the MCP23S17
    16-Bit I/O Expander chip on a Piface. For Details see:
    http://ww1.microchip.com/downloads/en/DeviceDoc/21952b.pdf
    There are two SPI channels on the Raspberry Pi:
        /dev/spidev0.0 - which enables CS CE0 when SPI writes or reads.
        /dev/spidev0.1 - which enables CS CE1 when SPI writes or reads.
    The MCP23S17 is a slave SPI device. The slave address contains four
    fixed bits and three user-defined hardware address bits pins A0, A1
    and A2 (if enabled via IOCON.HAEN).
    NOTE: A2 is not used in the PiFace Digital 1 (28 pin) and 2 (40 pin),
          which means selection of PiFace 4 - 7 is via CE1 (see below).
      **  A2 is used with A1/A0 on PiFace Relay+ to select 8 units on CE0.
    The Read/Write bit fills out the control byte.
          Jumpers JP1 and JP2 select A0 and A1 on PiFace 1, 2 and Relay+.
      **  Jumper JP3 selects A2 on PiFace Relay+ (does not need CE1).
    Jumpers JP1-JP8 on a PiRack can swap CE1 for CE0 for every PiFace 1 or 2
    on a PiRack. Together this makes for 8 possible PiFace addresses 0 - 7.
    The PiFaceCAD board has fixed wiring with CE1 hi and address 0,
    making it combined address 4 (unless CE0 is swapped on the PiRack).

 2) The MCP23S17 has a number of 8 bit registers which can be written
    and read via the above SPI channels. The most important of these
    for the PiFace driver are:
        IOCON   - which sets the configuration of the MCP23S17.
        IODIRA  - set to 0x00, which makes all 8 port A pins outputs.
        IODIRB  - set to 0xff, which makes all 8 port B pins inputs.
        OLATA   - which sets the 8 output pins when written to.
        GPIOB   - which returns the 8 input pins when read.
        GPINTENB- set to 0xff, a change on any port B pin activates INTB.
        INTFB   - set for any pin which changed and caused an interrupt.
        INTCAPB - which returns the 8 input pins at interrupt time.

    NOTE: GPIOB of any PiFace can be switched to be either an input or a
          2nd output register. In fact individual bits may be switched
          to be input bits or output bits, but only for GPIOB.
     **   The labelling of the extra GPIOB input or output bits for a
     **   Relay+ and a Relay Extra is confusing:
     **   x0 x1 x2 x3 on a PiFace Relay+ are bit 7 6 5 4 of GPIOB.
     **   y0 y1 y2 y3 on a PiFace Relay Extra are bit 3 2 1 0 of GPIOB.
     **   These I/Os have no screw connectors. They can be wire-wrapped.

    The INTB output, which signals a change on any of the input bits,
    is set to active-low in an open-drain configuration to allow for more
    than one PiFace. INTB is connected to pin 22 on all PiFaces going to
    GPIO25 on the Raspberry Pi.

    Reading either GPIOB or INTCAPB resets the source of the input
    interrupt.  INTCAPB only supplies the state at interrupt time,
    which does not necessarily reflect the state of the input when
    executing the interrupt handler. Using INTCAPB sometimes led to
    lost input changes. Therefore I only read GPIOB.

    When INTFB from several PiFaces are scanned in the interrupt
    handler, it sometimes happens that another change of input
    (usually due to fast bounces from a pushbutton contact) occurrs
    before the last PiFace has been dealt with sending its INTFB low
    again. This means that INTB will remain low after the last PiFace
    has been dealt with. Since the GPIO25 interrupt only occurrs on
    a falling edge, no interrupt will be seen for the latest change
    and the system hangs with INTB permanently low. That is why it is
    important to read the GPIO25 value after all Pifaces have been
    scanned. If it is still low, the scan is repeated until it goes
    high, which makes GPIO25 ready for the next falling edge interrupt.

 3) GPIO25 (the interrupt from the PiFace inputs) is exported with
    "sysfs" in the PiFace driver and set to interrupt on its falling
    edge. The interrupt is out-of-band, so if select(2) is used,
    the third exception file descriptor set must be used. select is
    generally used in the iC system, rather than poll(2), because
    select handles timeouts correctly when interrupted.

    NOTE: only one instance of iCpiFace may be run and all PiFaces and
    an optional PiFaceCAD as well as extra GPIO's must be controlled
    by this one instance. If two instances were running, the common
    interrupts would clash.  If GPIO25 has already been exported by
    another program or by another instance of iCpiFace the program
    exits with an error message, since GPIO25 cannot be used by more
    than one program simultaneously.  Some programs (PiFace Digital
    Emulator) do not unexport GPIO25 when they close. The -f command
    line option can be used to force use of GPIO25 if you are sure
    there is no running program actually using GPIO25.

 4) To allow several PiFace units and possibly a PiFaceCAD to run
    simultaneously on a PiRack, the INTB/GPIO25 line requires a pullup
    resistor, so the individual INTB outputs from each MCP23S17 can
    signal their interrupts via open-drain outputs. There is no pullup
    resistor on the PiFace, PiFaceCAD or the PiRack. But each GPIO
    pin on the Raspberry Pi can be configured to have a 60 Kohm pull-up
    or pull-down resistor. Unfortunately there is no way to control
    this with the "sysfs". To do this the BCM2835 direct memory access
    interface is used when BCM2835 is defined. For details see:
        http://www.airspayce.com/mikem/bcm2835/index.html
    This package does not support interrupts, so only the setting of
    the pullup resistor on INTB/GPIO25 is done with this package.

    Unfortunately this means iCpiFace must be run as SUID root, which
    prevents debugging and is a nuisance generally.  You can solve this
    problem for multiple PiFaces by soldering a 100 Kohm resistor from
    one of the GPIO25 (pin 22) to 5V on the PiRack.  Take your pick!!

    Another option is to use an independent simple SUID program to set
    pullup resistors on selected GPIO pins. iCgpioPUD does this.  In the
    latest version iCgpioPUD is called internally if the requested
    options require pullups on GPIO25 or any other GPIO input pins.

For the PiFaceCAD there are two additional paths from the Raspberry
Pi to consider:

 5) The PiFaceCAD uses GPIOA for 8 bits of input with INTA connected
    to pin 22 (GPIO25) for interrupts and GPIOB for output to a
    HD44780U Dot Matrix Liquid Crystal Display Controller. The C
    software provided by Thomas Preston was used for the display.
        https://github.com/piface/pifacecad

    The commands pifacecad_lcd_clear() and pifacecad_lcd_home() require
    a delay of 2.6 ms to bridge the execution time used internally for
    clearing the display buffer.  For the first command no execution
    time is specified in the HITACHI specification - the second one
    is specified at 1.52 ms. Actual measurement using the Busy Flag
    determined that the execution time if pifacecad_lcd_clear()
    is between 1.75 and 2.6 ms. Without a delay of 2.6 ms after
    this command a following pifacecad_lcd_write() would frequently
    garble the text. A pifacecad_lcd_display_off() or _on() does not
    execute at all unless there is a delay of 2.6 ms after a prior
    piface_lcd_clear(). All the other commands are followed by a
    delay of 40 us, which is in line with the HITACHI spec.

 6) The PiFaceCAD also has a LIRC output on pin 16 (GPIO23). This
    output is high if a PiFaceCAD is present - low otherwise.  It is
    only used in iCpiFace to distinguish between a PiFaceCAD or
    a plain PiFace at address 4 during the initialisation scan.
    The LIRC signal is best handled by independent LIRC software
    available in the Linux Kernel.
        http://piface.github.io/pifacecad/lirc.html
        #setting-up-the-infrared-receiver%3E%60_%20yourself

=head1 AUTHOR

John E. Wulff

=head1 BUGS

Email bug reports to B<immediateC@gmail.com> with L<iC Project> in the
subject field.

=head1 SEE ALSO

L<iCpiPWM(1)>, L<iCtherm(1)>, L<iCserver(1)>, L<iCbox(1)>, L<makeAll(1)>, L<select(2)>

=head1 COPYRIGHT

COPYRIGHT (C) 2014-2015  John E. Wulff

You may distribute under the terms of either the GNU General Public
License or the Artistic License, as specified in the README file.

For more information about this program, or for information on how
to contact the author, see the README file.

=cut

   ############ end of POD to generate iCpiFace man page ##################### */

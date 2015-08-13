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
 *	iCpiPWM
 *
 *	I/O client for iCserver in the immediate C environment of a
 *	Raspberry Pi handling analog output to one or more direct
 *	GPIO pins on the Raspberry Pi either as servo pulses in the
 *	range 500 to 2500 microseconds at 20 ms intervals (50 Hz) or
 *	as PWM pulses at different frequencies with a duty cycle in
 *	the range 0 to 100.
 *
 *	All GPIO pins on a Raspberry Pi A, B or B+ may be selected for
 *	analog output as long as they are not used by iCpiFace or an
 *	iC app linked with PiFace or direct GPIO I/O capabilities.
 *	NOTE: GPIO 25 is already used if PiFaces are also processed.
 *
 *	The servo and PWM functionality use the DMA and PWM or PCM
 *	peripherals to control and schedule the pulse lengths and
 *	dutycycles using calls to the pigpio library created by
 *	joan@abyz.co.uk, which is in the public domain.
 *	(gratefully acknowledged)
 *
 *	Also handles input from an MCP3008 ADC as used on a "Wombat"
 *	board.
 *
 *******************************************************************/

#include	"tcpc.h"
#include	<signal.h>
#include	<string.h>
#include	<ctype.h>
#include	<assert.h>
#include	<errno.h>
#include	<unistd.h>
#include	<sys/stat.h>
#include	<fcntl.h>

#if !defined(RASPBERRYPI) || !defined(PWM) || !defined(TCP) || defined(_WINDOWS)
#error - must be compiled with RASPBERRYPI, PWM and TCP defined and not _WINDOWS
#else	/* defined(RASPBERRYPI) && defined(PWM) && defined(TCP) && !defined(_WINDOWS) */

#include	"icc.h"			/* declares iC_emalloc() in misc.c and iC_quit() */
#include	"rpi_rev.h"		/* Determine Raspberry Pi Board Rev */
#include	<pigpio.h>		/* pigpio library */

#define		DEFAULT_PWM_RANGE	100
#define		DEFAULT_PWM_FREQUENCY	20000
#ifndef IOUNITS
#define IOUNITS	30			/* initial Units[] size */
#endif
#undef  BS
#define BS	1024			/* buffer size for I/O strings */

static struct timeval	toC100   = { 0, 100000 };	/* 100 ms select() timeout - re-iniialising value */
static struct timeval	toCnt    = { 0,  50000 };	/* 50 ms select() initial timeout */
static struct timeval *	toCntp   = NULL;		/* select() timeout initial value off */
static int		spiFN    = -1;			/* A/D on /dev/spidev0.0 */
static char		txBuf[4] = "\x01\x80\0";	/* A/D SPI transmit buffer */
static char		rxBuf[4];			/* A/D SPI receive buffer */

static const char *	usage =
"Usage:\n"
" %s [-Bftmh][ -s <host>][ -p <port>][ -n <name>][ -i <inst>]\n"
"          [ -a <val>][ -b <val>][ -D <val>][ -E <val>]\n"
"          [ -S <val>][ -C <val>][ -d <deb>]\n"
"          [ [~]QW<x>,<gpio>,p[,<range>[,<freq>]][-<inst>] ...]\n"
"          [ QW<x>,<gpio>,s[-<inst>] ...]\n"
"          [ IW<x>,<adc_channel>[-<inst>] ...]\n"
"                           # at least 1 IEC QW<x> or IW<x> argument\n"
"    -s host IP address of server    (default '%s')\n"
"    -p port service port of server  (default '%s')\n"
"    -i inst instance of this client (default '') or 1 to %d digits\n"
"    -B      start iCbox -d to monitor active analog I/O\n"
"    -f      force use of GPIO's required by this program\n" 
"                      PIGPIO initialisation arguments\n"
"    -a val  DMA mode, 0=AUTO, 1=PMAP, 2=MBOX,   default AUTO\n"
"    -b val  gpio sample buffer in milliseconds, default 120\n"
"    -D val  primary DMA channel, 0-14,          default 14\n"
"    -E val  secondary DMA channel, 0-6,         default 5\n"
"    -S val  sample rate, 1, 2, 4, 5, 8, or 10,  default 5 us\n"
"    -C val  clock peripheral, 0=PWM 1=PCM,      default PCM\n"
"                      GPIO SERVO and PWM IEC output arguments\n"
"    QW<x>,<gpio>,p[,<range>[,<freq>]]\n"
"            generates pulse width modulated (PWM) pulses on the gpio,\n"
"            QW<x> = 0 (off) to <range> (fully on)\n"
"          <x>     is any unique and valid IEC number (0 - 65535)\n"
"          <gpio>  is any unique GPIO supported by the board revision\n"
"                  and not used on iCpiFace or an iC app with direct I/O\n"
"          <range> PWM range                     default %d\n"
"          <freq>  PWM frequency >= 0            default %d Hz\n"
"                  (selects closest valid frequency)\n"
"          for more details see the %s(3) and pigpio(3) man pages\n"
"    ~QW<x>,<gpio>,p[,<range>[,<freq>]]\n"
"            generates inverted pulse width modulated (PWM) pulses on\n"
"            the gpio, QW<x> = 0 (off = <range>) to <range> (on = 0 out).\n"
"            Use this when the gpio output is inverted electrically.\n"
"    QW<x>,<gpio>,s\n"
"            generates microsecond servo pulses at 50Hz on the gpio,\n"
"            QW<x> < 500 (off - no pulses)\n"
"            QW<x> = 500 (most anti-clockwise) to 2500 (most clockwise)\n"
"            NOTE: inversion is not available for servo pulse outputs.\n"
"                      MCP3008 ADC IEC input arguments\n"
"    IW<x>,<adc_channel>\n"
"            adc_channel can be 0 to 7\n"
"	    TODO add optional measurement frequency and smoothing rate\n"
"                      COMMON extension\n"
"    IEC-inst Each individual IEC argument can be followed by -<inst>,\n"
"          where <inst> consists of 1 to %d numeric digits.\n"
"          Such an appended instance code takes precedence over the\n"
"          general instance specified with the -i <inst> option above.\n"
"    NOTE: there must be at least one GPIO IEC argument.\n"
"          No IEC arguments are generated automatically for %s.\n"
"                      DEBUG options\n"
"    -t      trace arguments and activity (equivalent to -d100)\n"
"    -m      display microsecond timing info\n"
"    -d deb  +1   trace TCP/IP send actions\n"
"            +2   trace TCP/IP rcv  actions\n"
"            +100 show arguments\n"
"            +200 show more debugging details\n"
"            +400 exit after initialisation\n"
"    -h      help, ouput this Usage text only\n"
"\n"
"Copyright (C) 2014-2015 John E. Wulff     <immediateC@gmail.com>\n"
"Version	$Id: iCpiPWM.c,v 1.1 2015/08/13 01:21:29 jw Exp $\n"
;

/********************************************************************
 *
 *	gpioQW structures are organised as follows:
 *	  each structure describes an independent QWx word
 *
 *******************************************************************/

typedef struct	gpioQW {
    char *		name;		/* QWx QWx-i */
    unsigned		val;		/* previous output value */
    unsigned short	channel;	/* channel to send I or receive Q to/from iCserver */
    unsigned short	gpio;		/* gpio number */
    unsigned		range;		/* 0 = servo, >0 = pwm range */
    unsigned		inv;		/* normal/inverted pwm range */
    unsigned		freq;		/* pwm frequency */
    struct gpioQW *	next;		/* arrange in null terminated linked list */
} gpioQW;

char *		iC_progname;		/* name of this executable */
static char *	iC_path;
short		iC_debug = 0;
int		iC_micro = 0;
unsigned short	iC_osc_lim = 1;
int		iC_opt_B = 0;
static unsigned short	topChannel = 0;
static char	buffer[BS];
static short	errorFlag = 0;
static gpioQW *	gpioWords = NULL;	/* linked list of IEC word outputs */
static gpioQW**	Units = NULL;		/* dynamic array to store GPIO links */
static int	ioUnits = 0;		/* dynamically allocated size of Units[] */
static unsigned bufferSizeMilliseconds = PI_DEFAULT_BUFFER_MILLIS;
static unsigned clockMicros            = PI_DEFAULT_CLK_MICROS;
static unsigned clockPeripheral        = PI_DEFAULT_CLK_PERIPHERAL;
static unsigned DMAprimaryChannel      = PI_DEFAULT_DMA_PRIMARY_CHANNEL;
static unsigned DMAsecondaryChannel    = PI_DEFAULT_DMA_SECONDARY_CHANNEL;
static unsigned memAllocMode           = PI_DEFAULT_MEM_ALLOC_MODE;

static void	storeUnit(unsigned short channel, gpioQW * gep);
static void	terminate_quit(int sig);	/* terminate pigpio functions - quit with correct interrupt vectors */

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
    int			proc;		/* processor board for Raspberry Pi */
    char **		argp;
    int			len;
    int			b;
    int			val;
    unsigned short	iid;
    char		iids[6];
    int			regBufLen = 0;
    unsigned short	iidN = 0xffff;	/* internal instance "" initially */
    int			forceFlag = 0;
    char *		iC_fullProgname;
    char *		cp;
    char *		op;
    int			ol;
    unsigned short	channel = 0;
    int			retval;
    unsigned short	gpio;
    char		tail[128];	/* must match sscanf formats %127s for tail */
    char		iqc[2] = { '\0', '\0' };
    char		dum[2] = { '\0', '\0' };
    char		sp[2]  = { '\0', '\0' };
    int			ieStart = 0;
    gpioQW *		gep;
    gpioQW *		gpioLast = NULL;
    unsigned		range;
    int			resolution = 1;
    unsigned		freq;
    int			invFlag;
    char **		argip;
    int			argic;
    char *		iidPtr;
    char *		iidSep;

    iC_outFP = stdout;			/* listing file pointer */
    iC_errFP = stderr;			/* error file pointer */

    assert(argc);
    invFlag = 0x00;
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
		    int		i;
		    int		slen;
		case 's':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (strlen(*argv)) iC_hostNM = *argv;
		    goto break2;
		case 'p':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (strlen(*argv)) iC_portNM = *argv;
		    goto break2;
		case 'i':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
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
		case 'B':
		    iC_opt_B = 1;	/* start iCbox -d to monitor active GPIO I/O */
		    break;
		case 'f':
		    forceFlag = 1;	/* force use of GPIO's required by this program */
		    break;
		 case 'a':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    i = atoi(*argv);
		    if ((i >= PI_MEM_ALLOC_AUTO) && (i <= PI_MEM_ALLOC_MAILBOX)) {
			memAllocMode = i;
		    } else {
			fprintf(iC_errFP, "ERROR: %s: invalid -a option (%d)\n", iC_progname, i);
			errorFlag++;
		    }
		    goto break2;
		 case 'b':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    i = atoi(*argv);
		    if ((i >= PI_BUF_MILLIS_MIN) && (i <= PI_BUF_MILLIS_MAX)) {
			bufferSizeMilliseconds = i;
		    } else {
			fprintf(iC_errFP, "ERROR: %s: invalid -b option (%d)\n", iC_progname, i);
			errorFlag++;
		    }
		    goto break2;
		 case 'D':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    i = atoi(*argv);
		    if ((i >= PI_MIN_DMA_CHANNEL) && (i <= PI_MAX_PRIMARY_CHANNEL)) {
			DMAprimaryChannel = i;
		    } else {
			fprintf(iC_errFP, "ERROR: %s: invalid -D option (%d)\n", iC_progname, i);
			errorFlag++;
		    }
		    goto break2;
		 case 'E':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    i = atoi(*argv);
		    if ((i >= PI_MIN_DMA_CHANNEL) && (i <= PI_MAX_SECONDARY_CHANNEL)) {
			DMAsecondaryChannel = i;
		    } else {
			fprintf(iC_errFP, "ERROR: %s: invalid -E option (%d)\n", iC_progname, i);
			errorFlag++;
		    }
		    goto break2;
		 case 'S':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    i = atoi(*argv);
		    switch(i) {
		    case 1:
		    case 2:
		    case 4:
		    case 5:
		    case 8:
		    case 10:
			clockMicros = i;
			break;
		    default:
			fprintf(iC_errFP, "ERROR: %s: invalid -S option (%d)\n", iC_progname, i);
			errorFlag++;
			break;
		    }
		    goto break2;
		 case 'C':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    i = atoi(*argv);
		    if ((i >= PI_CLOCK_PWM) && (i <= PI_CLOCK_PCM)) {
			clockPeripheral = i;
		    } else {
			fprintf(iC_errFP, "ERROR: %s: invalid -C option (%d)\n", iC_progname, i);
			errorFlag++;
		    }
		    goto break2;
		case 'd':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if ((slen = strlen(*argv)) != strspn(*argv, "01234567") ||	/* octal digits only */
			sscanf(*argv, "%o", &i) != 1 ||
			i & ~0777) {
			fprintf(iC_errFP, "ERROR: %s: '-d %s' is not a well formed octal string or exceeds range 777\n",
			    iC_progname, *argv);
			errorFlag++;
		    }
		    iC_debug |= i;	/* short */
		    goto break2;
		case 't':
		    iC_debug |= 0100;	/* trace arguments and activity */
		    break;
		default:
		    fprintf(iC_errFP,
			"%s: unknown command line switch '%s'\n", iC_progname, *argv);
		case 'h':
		case '?':
	      error:
		    fprintf(iC_errFP, usage, iC_progname, iC_hostNM, iC_portNM, INSTSIZE,
			DEFAULT_PWM_RANGE, DEFAULT_PWM_FREQUENCY, iC_progname, INSTSIZE, iC_progname);
		    exit(1);
		}
	    } while (*++*argv);
	  break2: ;
	} else {
	    /********************************************************************
	     *  Save IEC arguments for later analysis after all options have
	     *  been determined. IEC arguments and options may be mixed.
	     *  IEC arguments must start with ~ I or Q.
	     *  A special case is the HOME evironment variable, which is generated
	     *  by the Linux shell for ~. It is converted back to ~ here.
	     *******************************************************************/
	    if (strspn(*argv, "~IQXD") == 0) {
		if (strcmp(*argv, getenv("HOME")) == 0) {
		    argip[argic++] = "~";	/* transmogrify shell translation of HOME to ~ */
		    continue;			/* get next command line argument */
		}
		fprintf(stderr, "ERROR: %s: invalid argument '%s' (analog IEC variables start with ~ I or Q)\n", iC_progname, *argv);
		errorFlag++;
	    }
	    argip[argic++] = *argv;
	}
    }
    proc = boardrev();			/* Determine Raspberry Pi Board revision 0=A+ 1=B 2=B+ */
    if (iC_debug & 0200) fprintf(iC_outFP, "Raspberry Pi Board revision = %d (0 = A,A+; 1 = B; 2 = B+,2B)\n", proc);
    if (! errorFlag && argic) {
	/********************************************************************
	 *  Analyze IEC arguments
	 *******************************************************************/
	for (argp = argip; argp < &argip[argic]; argp++) {
	    if (strlen(*argp) >= 128) {
		fprintf(iC_errFP, "ERROR: %s: command line argument too long: '%s'\n", iC_progname, *argp);
		errorFlag++;
		continue;
	    }
	    iid   = iidN;				/* global instance either 0xffff for "" or 0 - 999 */
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
	    while ((b = sscanf(tail, "%1[~]%127s", dum, tail)) > 0) {
		invFlag ^= 0xff;			/* invert following argument logic sense */
		if (b == 1) {
		    goto skipInvFlag;			/* lone '~' - apply invFlag to next argument */
		}
	    }
	    /********************************************************************
	     *  Scan for IEC arguments IWX or QWx ..
	     *******************************************************************/
	    if ((b = sscanf(tail, "%1[IQ]W%5d,%hu%127s", iqc, &ieStart, &gpio, tail)) >= 3) {
		if (b == 3) *tail = '\0';
		if (*iqc == 'Q') {
		    /********************************************************************
		     *  QWx - scan for ,p PWM or ,s SERVO
		     *******************************************************************/
		    range = DEFAULT_PWM_RANGE;
		    freq  = DEFAULT_PWM_FREQUENCY;
		    if ((b = sscanf(tail, ",%1[sp]%127s", sp, tail)) >= 1) {
			if (b == 1) *tail = '\0';
			if (*sp == 'p') {		/* -p PWM */
			    if ((b = sscanf(tail, ",%5u%127s", &range, tail)) > 1) {
				if ((b = sscanf(tail, ",%5u%127s", &freq, tail)) > 1) {
				    goto checkInstance;
				}
			    }
			    if (b == 1) *tail = '\0';
			} else {			/* ,s SERVO */
			    range = freq = 0;		/* range == 0 for SERVO (cannot be inverted) */
			}
		    } else {
			goto illFormed;			/* must be ,s or ,p */
		    }
		} else {
		    range = freq = 0;			/* IWx,adc_channel in gpio */
		}
	      checkInstance:
		if (*tail == '-') {
		    if (b != 1 && (b = sscanf(tail, "-%3hu%127s", &iid, tail)) == 1) {	/* individual instance has precedence */
			snprintf(iids, 6, "-%hu", iid);	/* instance "-0" to "-999" */
		    } else {
			goto illFormed;
		    }
		} else if (*tail != '\0') {
		    goto illFormed;			/* a tail other than -instance eg -999 */
		}
	    } else {
	      illFormed:
		fprintf(iC_errFP, "ERROR: %s: '%s' is not a well formed IEC,<gpio>,<s|p>\n", iC_progname, *argp);
		errorFlag++;
		goto endOneArg;
	    }
	    if (iC_debug & 0200) fprintf(iC_outFP,
		"%cW%d gpio = %hu range = %u freq = %u instance = '%s' b = %d sp = '%s' tail = '%s'\n",
		*iqc, ieStart, gpio, range, freq, iids, b, sp, tail);
	    /********************************************************************
	     *  Use the IEC argument to extend GPIO control structures
	     *  Check IEC argument was not used before
	     *******************************************************************/
	    if (*iqc == 'Q' && ((b = gpioUsed[gpio]) & (1 << proc)) == 0) {	/* TODO RPi model dependent GPIOs */
		if (b == 0100000) {
		    fprintf(iC_errFP, "ERROR: %s: trying to use gpio %hu a 2nd time on QW%hu%s\n",
			iC_progname, gpio, ieStart, iids);
		} else {
		    fprintf(iC_errFP, "ERROR: %s: trying to use invalid gpio %hu on QW%hu%s on RPi board rev %d\n",
			iC_progname, gpio, ieStart, iids, proc);
		}
		errorFlag++;
	    } else {
		/********************************************************************
		 *  Generate a unique IEC name for network registration
		 *******************************************************************/
		if (iid != 0xffff && *iids == '\0') {
		    snprintf(iids, 6, "-%d", iid);	/* global instance */
		}
		len = snprintf(buffer, BS, "%cW%d%s", *iqc, ieStart, iids);	/* IEC name with possible instance */
		for (gep = gpioWords; gep; gep = gep->next) {
		    if (strcmp(gep->name, buffer) == 0) {
			fprintf(iC_errFP, "ERROR: %s: trying to use %s a 2nd time\n", iC_progname, buffer);
			errorFlag++;
			goto endOneArg;		/* not unique */
		    }
		}
		if (*iqc == 'Q') gpioUsed[gpio] = 0100000;	/* TODO mark gpio as used */
		gep = iC_emalloc(sizeof(gpioQW));	/* new gpioQW element */
		if (gpioWords == NULL) {
		    gpioWords = gep;		/* first element */
		} else {
		    assert(gpioLast);
		    gpioLast->next = gep;	/* previous last element linked to new element */
		}
		gpioLast = gep;			/* this is new last element */
		gep->name = iC_emalloc(++len);
		strcpy(gep->name, buffer);	/* store IEC name */
		gep->gpio = gpio;		/* gpio number for this word - A/D channel for IWx */
		gep->range = range;		/* range (0 = servo or PWM range) for this word */
		if (range && *buffer == 'Q') {
		    gep->inv = invFlag;		/* normal/inverted PWM output range unless SERVO */
		} else if (invFlag) {
		    fprintf(iC_errFP, "WARNING: %s: cannot invert SERVO output or A/D input %s - ignore\n", iC_progname, buffer);
		}
		gep->freq = freq;		/* PWM freq for this word */
	    }
	  endOneArg: ;
	    invFlag = 0x00;			/* start new tilde '~' analysis for next argument */
	  skipInvFlag:;
	}
    }
    free(argip);
    if (gpioWords == NULL) {
	fprintf(iC_errFP, "ERROR: %s: no valid IEC arguments? there must be at least 1 valid argument\n",
	    iC_progname);
	goto error;					/* output usage */
    }
    if (errorFlag) {
	iC_quit(-3);					/* after all the command line has been analyzed */
    }
    /* configure PIGPIO library */
    gpioCfgBufferSize(bufferSizeMilliseconds);
    gpioCfgClock(clockMicros, clockPeripheral, 0);
    gpioCfgInterfaces(PI_DISABLE_FIFO_IF | PI_DISABLE_SOCK_IF);	/* no external control used */
    gpioCfgDMAchannels(DMAprimaryChannel, DMAsecondaryChannel);
    gpioCfgMemAlloc(memAllocMode);
    // if (updateMaskSet) gpioCfgPermissions(updateMask);
    /* start PIGPIO library */
    if (gpioInitialise() < 0) {
	fprintf(iC_errFP, "ERROR: %s: Can't initialise pigpio library\n", iC_progname);
	exit(1);
    }
    /********************************************************************
     *  Generate a meaningful name for network registration
     *******************************************************************/
    len = snprintf(buffer, BS, "%s", iC_iccNM);
    for (gep = gpioWords; gep; gep = gep->next) {
	len += snprintf(buffer+len, BS-len, "_%hu", gep->gpio);	/* name GPIO gpio */
    }
    if (strlen(iC_iidNM) > 0) {
	len += snprintf(buffer+len, BS-len, "-%s", iC_iidNM);	/* name-instance header */
    }
    iC_iccNM = iC_emalloc(len + 1);
    strcpy(iC_iccNM, buffer);
    if (iC_micro) iC_microReset(0);		/* start timing */
    if (iC_debug & 0200) fprintf(iC_outFP, "host = %s, port = %s, name = %s\n", iC_hostNM, iC_portNM, iC_iccNM);
    if (iC_debug & 0400) terminate_quit(0);
    signal(SIGINT, terminate_quit);			/* catch ctrlC and Break */
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
     *  ctrl-C, when it has been brought to the foreground with fg.
     *******************************************************************/
    signal(SIGTTIN, SIG_IGN);			/* ignore tty input signal in bg */
#endif	/* SIGTTIN */
    /********************************************************************
     *  All arguments have been taken care of
     *  Start TCP/IP communication, build registration string and register
     *******************************************************************/
    iC_sockFN = iC_connect_to_server(iC_hostNM, iC_portNM);
    if (iC_sockFN > iC_maxFN) {
	iC_maxFN = iC_sockFN;
    }
    regBufLen = REQUEST;			/* really ready to copy iecId's into regBuf  Nname...,Z\n */
    len = snprintf(regBuf, regBufLen, "N%s", iC_iccNM);	/* name header */
    cp = regBuf + len;
    regBufLen -= len;
    /********************************************************************
     *  Extend registration string with all active GPIO output names
     *******************************************************************/
    for (gep = gpioWords; gep; gep = gep->next) {
	assert(regBufLen > ENTRYSZ);		/* accomodates ",RQW123456,Z" */
	len = snprintf(cp, regBufLen, ",R%s", gep->name);
	cp += len;				/* output receive name */
	regBufLen -= len;
    }
    /********************************************************************
     *  Send registration string made up of all active I/O names
     *******************************************************************/
    if (iC_debug & 0200)  fprintf(iC_outFP, "regBufLen = %d\n", regBufLen);
    snprintf(cp, regBufLen, ",Z");		/* Z terminator */
    if (iC_debug & 0200)  fprintf(iC_outFP, "register:%s\n", regBuf);
    if (iC_micro) iC_microPrint("send registration", 0);
    iC_send_msg_to_server(iC_sockFN, regBuf);	/* register IOs with iCserver */
    /********************************************************************
     *  Receive a channel number for each I/O name sent in registration
     *  Distribute read and/or write channels returned in acknowledgment
     *******************************************************************/
    if (iC_rcvd_msg_from_server(iC_sockFN, rpyBuf, REPLY)) {	/* busy wait for acknowledgment reply */
	if (iC_micro) iC_microPrint("ack received", 0);
	if (iC_debug & 0200) fprintf(iC_outFP, "reply:%s\n", rpyBuf);
	if (iC_opt_B) {
	    len = snprintf(buffer, BS, "perl -S iCbox -d -s %s -p %s -n %s-DI",
		iC_hostNM, iC_portNM, iC_iccNM);	/* prepare to execute iCbox -d */
	    op = buffer + len;
	    ol = BS - len;
	}
	cp = rpyBuf - 1;
	/********************************************************************
	 *  Channels for GPIO acknowledgments
	 *******************************************************************/
	for (gep = gpioWords; gep; gep = gep->next) {
	    assert(cp);				/* not enough channels in ACK string */
	    channel = atoi(++cp);		/* read channel from ACK string */
	    assert(channel > 0);
	    if (channel > topChannel) {
		topChannel = channel;
	    }
	    if (iC_debug & 0200) {
		fprintf(iC_outFP, "GPIO %hu	%s  on channel %hu\n", gep->gpio, gep->name, channel);
	    }
	    gep->channel = channel;		/* link channel to GPIO (ignore receive channel) */
	    storeUnit(channel, gep);		/* link GPIO element pointer to send channel */
	    if (iC_opt_B) {
		assert(ol > 14);
		len = snprintf(op, ol, " %s", gep->name);	/* add I/O name[-inst] to execute iCbox -d */
		op += len;
		ol -= len;
		if (*gep->name == 'I') {
		    len = snprintf(op, ol, ",0,1023");		/* A/D input iCbox slider */
		} else
		if ((range = gep->range) == 0) {
		    len = snprintf(op, ol, ",500,2500,20");	/* SERVO range of iCbox slider */
		} else {
		    resolution = range / 100;
		    if (resolution <= 1) {
			len = snprintf(op, ol, ",0,%u", range);	/* PWM range of iCbox slider */
		    } else {
			len = snprintf(op, ol, ",0,%u,%d", range, resolution);
		    }
		}
		op += len;
		ol -= len;
	    }
	    cp = strchr(cp, ',');
	}
	assert(cp == NULL);			/* Ack string matches Registration */
	/********************************************************************
	 *  ACK string complete
	 *******************************************************************/
	if (iC_opt_B) {
	    int		i = 0;
	    char *	ex_arg = "/usr/bin/perl";	/* use execv() - do not search path because SUID */
	    char *	ex_args[66];			/* execute iCbox -d as a separate process */
	    pid_t	c_pid;
	    uid_t	uid;

	    fprintf(iC_outFP, "%s\n", buffer);
	    /* retrieve first token from buffer, separated using " " */
	    ex_args[i] = strtok(buffer, " ");
	    if (iC_debug & 0200) fprintf(iC_outFP, "*** %d:	%s\n", i, ex_args[i]);
	    i++;
	    /* continue to retrieve tokens until NULL returned */
	    while(i < 66 && (ex_args[i] = strtok(NULL, " ")) != NULL) {
		if (iC_debug & 0200) fprintf(iC_outFP, "*** %d:	%s\n", i, ex_args[i]);
		i++;
	    }
	    if (i >= 66) {				/* should fit with a max of 32 possible GPIO's */
		fprintf(iC_errFP, "ERROR: %s: iCbox -d call is limited to call + 64 parameters + terminator\n",
		    iC_progname);
		terminate_quit(SIGUSR1);
	    }
	    uid = getuid();
	    if (iC_debug & 0200) fprintf(iC_outFP, "uid = %d euid = %d\n", (int)uid, (int)geteuid());
	    if ((c_pid = fork()) == 0) {
		/* child process */
		if (seteuid(uid) != 0) {		/* execute iCbox at uid privileges */
		    perror("seteuid failed");		/* hard ERROR */
		    terminate_quit(SIGUSR1);
		}
		execv(ex_arg, ex_args);			/* execute iCbox -d call in parallel child process */
		/* only get here if exec fails */
		perror("execv failed");			/* hard ERROR */
		terminate_quit(SIGUSR1);
	    } else if (c_pid < 0) {
		perror("fork failed");			/* hard ERROR */
		terminate_quit(SIGUSR1);
	    }
	    /* continue parent process with extended privileges */
	}
	if (iC_debug & 0200) fprintf(iC_outFP, "reply: top channel = %hu\n", topChannel);
    } else {
	terminate_quit(QUIT_SERVER);			/* quit normally with 0 length message */
    }
    /********************************************************************
     *  Report results of argument analysis
     *******************************************************************/
    if (iC_debug & 0300) {
	fprintf(iC_outFP, "Allocation for GPIO elements, global instance = \"%s\"\n", iC_iidNM);
	fprintf(iC_outFP, "	IEC	gpio	range	 freq	channel	instance\n\n");
	for (gep = gpioWords; gep; gep = gep->next) {
	    strcpy(buffer, gep->name);		/* retrieve name[-instance] */
	    if ((iidPtr = strchr(buffer, '-')) != NULL) {
		*iidPtr++ = '\0';		/* separate name and instance */
		iidSep = "	-";
	    } else {
		iidPtr = iidSep = "";
	    }
	    fprintf(iC_outFP, "	%s	%3hu	%c%4u	%5u	%3hu%s%s\n",
		buffer, gep->gpio, gep->inv ? '~' : ' ', gep->range, gep->freq, gep->channel, iidSep, iidPtr);
	}
	fprintf(iC_outFP, "\n");
    }
    /********************************************************************
     *  Generate GPIO PWM initialisation for active outputs
     *  Set range, frequency and initial value:
     *    SERVO: 0,     0,    0 # which turns SERVO off (no pulses)
     *    PWM:   range, freq, 0 # which turns PWM off (duty/cycle 0 = 0V out)
     *******************************************************************/
    for (gep = gpioWords; gep; gep = gep->next) {
	assert(gep->val == 0);			/* all gpioQW entries val initialised to 0 */
	if (*(gep->name) == 'Q') {		/* QWx for PWM or SERVO output */
	    if ((range = gep->range) == 0) {
		/********************************************************************
		 *  Write GPIO QWx SERVO initialisation outputs
		 *******************************************************************/
		if ((b = gpioServo(gep->gpio, gep->val)) < 0) {
		    fprintf(iC_errFP, "WARNING: %s: Can't execute initial 'gpioServo(%hu, %u)' ?%d in pigpio library\n",
			iC_progname, gep->gpio, gep->val, b);
		}
	    } else {
		if ((b = gpioSetPWMfrequency(gep->gpio, gep->freq)) < 0) {	/* PWM frequency >= 0 - set nearest frequency */
		    fprintf(iC_errFP, "WARNING: %s: Can't execute initial 'gpioSetPWMfrequency(%hu, %u)' ?%d in pigpio library\n",
			iC_progname, gep->gpio, gep->freq, b);
		}
		if (iC_debug & 0100) fprintf(iC_outFP, "GPIO %hu: Real frequency = %d for requested frequency %u\n",
		    gep->gpio, b, gep->freq);
		gep->freq = (unsigned)b;
		if ((b = gpioSetPWMrange(gep->gpio, range)) < 0) {	/* PWM frequency >= 0 - set nearest frequency */
		    fprintf(iC_errFP, "WARNING: %s: Can't execute initial 'gpioSetPWMrange(%hu, %u)' ?%d in pigpio library\n",
			iC_progname, gep->gpio, range, b);
		}
		if (iC_debug & 0100) fprintf(iC_outFP, "GPIO %hu: Real range = %d for requested range %u\n",
		    gep->gpio, b, range);
//	    gep->range = (unsigned)b;	/* TODO check relatiomship of freq, real range etc */
		/********************************************************************
		 *  Write GPIO QWx PWM initialisation outputs
		 *******************************************************************/
		val = gep->inv ? range : 0;				/* normal/inverted initial PWM output */
		if ((b = gpioPWM(gep->gpio, val)) < 0) {
		    fprintf(iC_errFP, "WARNING: %s: Can't execute initial 'gpioPWM(%hu, %u)' ?%d in pigpio library\n",
			iC_progname, gep->gpio, val, b);
		}
	    }
	} else if (spiFN < 0) {			/* IWx for A/D conversion input */
	    if ((spiFN = spiOpen(0, 100000, 0)) < 0) {
		fprintf(iC_errFP, "WARNING: %s: open A/D on /dev/spidev0.0 failed\n", iC_progname);
	    } else {
		toCnt = toC100;			/* set timeout value for A/D conversions */
		toCntp = &toCnt;
	    }
	}
    }
    /********************************************************************
     *  Clear and then set all bits to wait for interrupts
     *******************************************************************/
    FD_ZERO(&iC_infds);				/* should be done centrally if more than 1 connect */
    FD_SET(iC_sockFN, &iC_infds);		/* watch sock for inputs */
    if (iC_debug & 0200) fprintf(iC_outFP, "iC_sockFN = %d\n", iC_sockFN);
#ifndef	WIN32
    FD_SET(0, &iC_infds);			/* watch stdin for inputs - FD_CLR on EOF */
    /* can only use sockets, not file descriptors under WINDOWS - use kbhit() */
#endif	/* WIN32 */
    /********************************************************************
     *  External input (TCP/IP via socket and STDIN)
     *  Wait for input in a select statement most of the time
     *******************************************************************/
    for (;;) {
	if ((retval = iC_wait_for_next_event(toCntp)) == 0)
	{
	    if (toCnt.tv_sec == 0 && toCnt.tv_usec == 0) {
		toCnt = toC100;			/* re-initialise timeout value */
		/********************************************************************
		 *  Carry out A/D conversion on all allocated A/D channels
		 *******************************************************************/
		cp = regBuf;
		regBufLen = REPLY;
		if (iC_debug & 0100) {
		    op = buffer;
		    ol = BS;
		}
		for (gep = gpioWords; gep; gep = gep->next) {
		    if (*(gep->name) == 'I') {	/* IWx for A/D conversion input */
			txBuf[1] = 0x80 | gep->gpio << 4;	/* single ended adc_channel */
			if ((b = spiXfer(spiFN, txBuf, rxBuf, 3)) != 3) {
			    fprintf(iC_errFP, "WARNING: %s: xfer A/D on /dev/spidev0.0 failed (%d)\n",
				iC_progname, b);
			}
			val = (rxBuf[1] & 0x03) << 8 | rxBuf[2];	/* 10 bit A/D conversion value */
			if (val != gep->val) {
			    len = snprintf(cp, regBufLen, ",%hu:%d", gep->channel, val); /* data telegram */
			    cp += len;
			    regBufLen -= len;
			    if (iC_debug & 0100) {
				len = snprintf(op, ol, " A %s,%d", gep->name, gep->gpio); /* source name, adc_channel */
				op += len;
				ol -= len;
			    }
			    gep->val = val;	/* latest A/D value for comparison */
			}
		    }
		}
		if (cp > regBuf) {
		    iC_send_msg_to_server(iC_sockFN, regBuf+1);	/* send data telegram(s) to iCserver */
		    if (iC_debug & 0100) fprintf(iC_outFP, "P: %s:	%s	<%s\n",
			iC_iccNM, regBuf+1, buffer);
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
		    cp = rpyBuf - 1;		/* point to dummy comma before first message in input */
		    if (isdigit(rpyBuf[0])) {
			char *	cpe;
			char *	cps;

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
				gep = Units[channel];
				if(gep) {		/* message channel is for this app */
				    /********************************************************************
				     *  TCP/IP output for a GPIO
				     *******************************************************************/
				    if (val != gep->val) {
					gep->val = val;
					if ((range = gep->range) == 0) {
					    /********************************************************************
					     *  Write GPIO QWx SERVO output
					     *******************************************************************/
					    if (val < 500) val = 0;		/* hard limits for SERVO */
					    if (val > 2500) val = 2500;
					    if ((b = gpioServo(gep->gpio, val)) < 0) {
						fprintf(iC_errFP, "WARNING: %s: Can't execute 'gpioServo(%hu, %u)' ?%d in pigpio library\n",
						    iC_progname, gep->gpio, val, b);
					    }
					} else {
					    /********************************************************************
					     *  Write GPIO QWx PWM output with previously set range and frequency
					     *******************************************************************/
					    if (val > range) val = range;	/* soft limit for PWM */
					    if (gep->inv) val = range - val;	/* inverts PWM output */
					    if ((b = gpioPWM(gep->gpio, val)) < 0) {
						fprintf(iC_errFP, "WARNING: %s: Can't execute 'gpioPWM(%hu, %u)' ?%d in pigpio library\n",
						    iC_progname, gep->gpio, val, b);
					    }
					}
				    }
				} else {
				    fprintf(iC_errFP, "channel = %hu is not registered in Units[] for this app\n",
					channel);
				    goto RcvWarning;
				}
			    } else {
				fprintf(iC_errFP, "channel = %hu, topChannel = %hu\n", channel, topChannel);
				goto RcvWarning;
			    }
			} while ((cp = cpe) != NULL);		/* next token if any */
		    }
		    else {
		      RcvWarning:
			fprintf(iC_errFP, "WARNING: %s: received '%s' from iCserver ???\n", iC_iccNM, cp);
		    }
		} else {
		    terminate_quit(QUIT_SERVER);	/* quit normally with 0 length message from iCserver */
		}
	    }	/* end of TCP/IP input */
	    /********************************************************************
	     *  STDIN interrupt
	     *******************************************************************/
	    if (FD_ISSET(0, &iC_rdfds)) {
		if (fgets(buffer, BS, stdin) == NULL) {
		    FD_CLR(0, &iC_infds);	/* ignore EOF - happens in bg or file - turn off interrupts */
		    buffer[0] = '\0';		/* notify EOF to iC application by zero length buffer */
		}
		if ((b = buffer[0]) == 'q') {
		    terminate_quit(QUIT_TERMINAL);	/* quit normally */
		} else if (b == 't') {
		    iC_debug ^= 0100;		/* toggle -t flag */
		} else if (b == 'm') {
		    iC_micro ^= 1;		/* toggle micro */
		} else if (b != '\n') {
		    fprintf(iC_errFP, "no action coded for '%c' - try t, m, or q followed by ENTER\n", b);
		}
	    }	/*  end of STDIN interrupt */
	} else {
	    perror("ERROR: select failed");	/* retval -1 */
	    terminate_quit(SIGUSR1);
	}
    }
} /* main */

/********************************************************************
 *
 *	Initalise and expand dynamic array Units[] as necessary
 *	Store gpioQW * to current GPIO element
 *
 *******************************************************************/

static void
storeUnit(unsigned short channel, gpioQW * gep)
{
    while (channel >= ioUnits) {
	Units = (gpioQW **)realloc(Units,		/* initially NULL */
	    (ioUnits + IOUNITS) * sizeof(gpioQW *));
	assert(Units);
	memset(&Units[ioUnits], '\0', IOUNITS * sizeof(gpioQW *));
	ioUnits += IOUNITS;			/* increase the size of the array */
	if (iC_debug & 0200) fprintf(iC_outFP, "storeUnit: Units[%d] increase\n", ioUnits);
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "storeUnit: Units[%d] <= %s\n", channel, gep->name);
    Units[channel] = gep;			/* store gpioQW * */
} /* storeUnit */

/********************************************************************
 *
 *	Terminate pigpio functions
 *	Quit with correct interrupt vectors
 *
 *******************************************************************/

static void
terminate_quit(int sig)
{
    int			b;
    gpioQW *		gep;

    for (gep = gpioWords; gep; gep = gep->next) {
	gep->val = 0;		/* all gpioQW entries val back to 0 */
	if (gep->range == 0) {
	    /********************************************************************
	     *  Write GPIO QWx SERVO final 0 output
	     *******************************************************************/
	    if ((b = gpioServo(gep->gpio, gep->val)) < 0) {
		fprintf(iC_errFP, "WARNING: %s: Can't execute final 'gpioServo(%hu, %u)' ?%d in pigpio library\n",
		    iC_progname, gep->gpio, gep->val, b);
	    }
	} else {
	    /********************************************************************
	     *  Write GPIO QWx PWM final 0 output
	     *******************************************************************/
	    if ((b = gpioPWM(gep->gpio, gep->val)) < 0) {
		fprintf(iC_errFP, "WARNING: %s: Can't execute final 'gpioPWM(%hu, %u)' ?%d in pigpio library\n",
		    iC_progname, gep->gpio, gep->val, b);
	    }
	}
    }
    gpioDelay(100000);			/* settle for another 100 ms */
    if (spiFN >= 0) {
	spiClose(spiFN);
	spiFN = -1;
    }
    gpioTerminate();			/* pigpio library stuff */
    iC_quit(sig);			/* finally quit */
} /* terminate_quit */
#endif	/* defined(RASPBERRYPI) && defined(PWM) && defined(TCP) && !defined(_WINDOWS) */

/* ############ POD to generate iCpiPWM man page ############################

=encoding utf8

=head1 NAME

iCpiPWM - real PWM analog I/O on a Raspberry Pi for the iC environment

=head1 SYNOPSIS

 iCpiPWM [-Bftmh][ -s <host>][ -p <port>][ -n <name>][ -i <inst>]
         [ -a <val>][ -b <val>][ -D <val>][ -E <val>]
         [ -S <val>][ -C <val>][ -d <deb>]
         [ [~]QW<x>,<gpio>,p[,<range>[,<freq>]][-<inst>] ...]
         [ QW<x>,<gpio>,s[-<inst>] ...]
         [ IW<x>,<adc_channel>[-<inst>] ...]
                           # at least 1 IEC QW<x> or IW<x> argument
    -s host IP address of server    (default 'localhost')
    -p port service port of server  (default '8778')
    -i inst instance of this client (default '') or 1 to 3 digits
    -B      start iCbox -d to monitor active analog I/O
    -f      force use of GPIO's required by this program\n" 
                      PIGPIO initialisation arguments
    -a val  DMA mode, 0=AUTO, 1=PMAP, 2=MBOX,   default AUTO
    -b val  gpio sample buffer in milliseconds, default 120
    -D val  primary DMA channel, 0-14,          default 14
    -E val  secondary DMA channel, 0-6,         default 5
    -S val  sample rate, 1, 2, 4, 5, 8, or 10,  default 5 us
    -C val  clock peripheral, 0=PWM 1=PCM,      default PCM
                      GPIO PWM and SERVO IEC output arguments
    QW<x>,<gpio>,p[,<range>[,<freq>]]
            generates pulse width modulated (PWM) pulses on the gpio,
            QW<x> = 0 (off) to <range> (fully on)
          <x>     is any unique and valid IEC number (0 - 65535)
          <gpio>  is any unique GPIO supported by the board revision
                  and not used on iCpiFace or an iC app with direct I/O
          <range> PWM range                     default 100
          <freq>  PWM frequency >= 0            default 20000 Hz
                  (selects closest valid frequency)
          for more details see the pigpio(3) man page
    ~QW<x>,<gpio>,p[,<range>[,<freq>]]
            generates inverted pulse width modulated (PWM) pulses on
            the gpio, QW<x> = 0 (off = <range>) to <range> (on = 0 out).
            Use this when the gpio output is inverted electrically.
    QW<x>,<gpio>,s
            generates microsecond servo pulses at 50Hz on the gpio,
            QW<x> < 500 (off - no pulses)
            QW<x> = 500 (most anti-clockwise) to 2500 (most clockwise)
            NOTE: inversion is not available for servo pulse outputs.
                      MCP3008 ADC IEC input arguments
    IW<x>,<adc_channel>
            adc_channel can be 0 to 7
                      COMMON extension
    IEC-inst Each individual IEC argument can be followed by -<inst>,
          where <inst> consists of 1 to 3 numeric digits.
          Such an appended instance code takes precedence over the
          general instance specified with the -i <inst> option above.
    NOTE: there must be at least one GPIO IEC argument.
          No IEC arguments are generated automatically for iCpiPWM.
                      DEBUG options
    -t      trace arguments and activity (equivalent to -d100)
    -m      display microsecond timing info
    -d deb  +1   trace TCP/IP send actions
            +2   trace TCP/IP rcv  actions
            +100 show arguments
            +200 show more debugging details
            +400 exit after initialisation
    -h      help, ouput this Usage text only

=head1 DESCRIPTION

B<iCpiPWM> is an I/O client for iCserver in the immediate C environment
of a Raspberry Pi handling analog output to one or more direct GPIO
pins on the Raspberry Pi either as servo pulses in the range 500
to 2500 microseconds at 20 ms intervals (50 Hz) or as PWM pulses at
different frequencies with a duty cycle in the range 0 to 100.

All GPIO pins on a Raspberry Pi A, B, B+ or 2B may be selected for analog
output as long as they are not used by iCpiFace or an iC app linked
with PiFace or direct GPIO I/O capabilities.  NOTE: GPIO 7, 8, 9, 10, 11
and 25 are already used if actual PiFaces are also processed.

B<iCpiPWM> -f forces use of GPIO's by this program. In this case iCpiFace
or a direct output iC app must be started afterwards. These other apps
also check that the GPIO's they use are free.

B<iCpiPWM> also handles input from an MCP3008 ADC as used on a
"Wombat" board.

=head1 TECHNICAL BACKGROUND

All GPIO PWM and servo pulses are timed using the DMA and PWM peripherals.
This is achieved by calls to the pigpio library created by <joan@abyz.co.uk>,
which is in the public domain (gratefully acknowledged).
<http://abyz.co.uk/rpi/pigpio/>

The use of DMA and PWM peripherals in the pigpio library was inspired by
Richard Hirst's servoblaster kernel module.
<https://github.com/richardghirst/PiBits/tree/master/ServoBlaster>

=head1 AUTHOR

John E. Wulff

=head1 BUGS

Email bug reports to B<immediateC@gmail.com> with L<iC Project> in the
subject field.

=head1 SEE ALSO

L<iCpiFace(1)>, L<iCserver(1)>, L<iCbox(1)>, L<pigpio(3)>

=head1 COPYRIGHT

COPYRIGHT (C) 2014-2015  John E. Wulff

You may distribute under the terms of either the GNU General Public
License or the Artistic License, as specified in the README file.

For more information about this program, or for information on how
to contact the author, see the README file.

=cut

   ############ end of POD to generate iCpiPWM man page ##################### */

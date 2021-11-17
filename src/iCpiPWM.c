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

#include	<stdio.h>
#include	<stdlib.h>
#include	<signal.h>
#include	<string.h>
#include	<ctype.h>
#include	<assert.h>
#include	<errno.h>
#include	<unistd.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<time.h>

#if !defined(RASPBERRYPI) || !defined(PWM) || !defined(TCP) || defined(_WINDOWS)
#error - must be compiled with RASPBERRYPI, PWM and TCP defined and not _WINDOWS
#else	/* defined(RASPBERRYPI) && defined(PWM) && defined(TCP) && !defined(_WINDOWS) */

#include	"icc.h"			/* declares iC_emalloc() in misc.c */
#include	"tcpc.h"
#include	"rpi_rev.h"		/* Determine Raspberry Pi Board Rev */
#include	<pigpio.h>		/* pigpio library */

#define		DEFAULT_PWM_RANGE	500
#define		DEFAULT_PWM_FREQUENCY	400
#ifndef IOUNITS
#define IOUNITS	30			/* initial Units[] size */
#endif
#undef  BS
#define BS	1024			/* buffer size for I/O strings */

static fd_set		infds;			/* initialised file descriptor set for normal iC_wait_for_next_event() */
static struct timeval	toCnt;				/* A/D measurement repetition timeout counter */
static struct timeval	toRep    = { 0, 100000 };	/* timeout re-initialising value (default 100 ms) */
static struct timeval *	toCntp   = NULL;		/* select() timeout initial value off */
static int		spiFN    = -1;			/* A/D on /dev/spidev0.0 */
static char		txBuf[4] = "\x01\x80\x00";	/* A/D SPI transmit buffer */
static char		rxBuf[4];			/* A/D SPI receive buffer */
static struct timespec	ms100 = { 0, 100000000, };

static const char *	usage =
"Usage:\n"
" %s [-Bftmqzh][ -s <host>][ -p <port>][ -n <name>][ -i <inst>]\n"
"         [ -a <val>][ -b <val>][ -D <val>][ -E <val>]\n"
"         [ -A <val>][ -C <val>][ -d <deb>]\n"
"         [ [~]QW<x>,<gpio>,p[,<range>[,<freq>]][-<inst>] ...]\n"
"         [ QW<x>,<gpio>,s[-<inst>] ...]\n"
"         [ IW<x>,<adc_channel>[-<inst>] ...]\n"
"         [ -r <time>][ -e <expf>][ -H <hyst>]\n"
"         [ -R <aux_app>[ <aux_app_argument> ...]] # must be last arguments\n"
"    -s host IP address of server    (default '%s')\n"
"    -p port service port of server  (default '%s')\n"
"    -i inst instance of this client (default '') or 1 to %d digits\n"
"    -B      start iCbox -d to monitor active analog I/O\n"
"    -W GPIO number used by the w1-gpio kernel module (default 4, maximum 31).\n"
"            When the GPIO with this number is used in this app, iCtherm is\n"
"            permanently blocked to avoid Oops errors in module w1-gpio.\n"
"    -f      force use of GPIO's required by this program\n"
"                      PIGPIO initialisation arguments\n"
"    -a val  DMA mode, 0=AUTO, 1=PMAP, 2=MBOX,   default AUTO\n"
"    -b val  gpio sample buffer in milliseconds, default 120\n"
"    -D val  primary DMA channel, 0-14,          default 14\n"
"    -E val  secondary DMA channel, 0-6,         default 5\n"
"    -A val  sample rate, 1, 2, 4, 5, 8, or 10,  default 5 us\n"
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
"          <x>     is any unique and valid IEC number (0 - 65535)\n"
"          <adc_channel> can be 0 to 7 or 10 to 17\n"
"            0   single-ended CH0\n"
"            1   single-ended CH1\n"
"            2   single-ended CH2\n"
"            3   single-ended CH3\n"
"            4   single-ended CH4\n"
"            5   single-ended CH5\n"
"            6   single-ended CH6\n"
"            7   single-ended CH7\n"
"            10  differential CH0 = IN+ CH1 = IN-\n"
"            11  differential CH0 = IN- CH1 = IN+\n"
"            12  differential CH2 = IN+ CH3 = IN-\n"
"            13  differential CH2 = IN- CH3 = IN+\n"
"            14  differential CH4 = IN+ CH5 = IN-\n"
"            15  differential CH4 = IN- CH5 = IN+\n"
"            16  differential CH6 = IN+ CH7 = IN-\n"
"            17  differential CH6 = IN- CH7 = IN+\n"
"    -r time A/D measurement repetition time in ms (default 100 ms)\n"
"    -e expf A/D measurement exponential smoothing factor (default 1)\n"
"            If expf > 1, exponential smoothing is applied to each\n"
"            measurement, which is equivalent to using an R/C filter.\n"
"            Out = previousOut * (expf-1)/expf + measurement * 1/expf\n"
"    -H hyst A/D measurement hysteresis (default 1)\n"
"            If hyst > 1 measurements are only output if\n"
"            abs(Out - prevOut) >= hyst\n"
"                      COMMON extension\n"
"    IEC-inst Each individual IEC argument can be followed by -<inst>,\n"
"          where <inst> consists of 1 to %d numeric digits.\n"
"          Such an appended instance code takes precedence over the\n"
"          general instance specified with the -i <inst> option above.\n"
"    NOTE: there must be at least 1 IEC QW<x> or IW<x> argument.\n"
"          No IEC arguments are generated automatically for %s.\n"
"                      DEBUG options\n"
#if YYDEBUG && !defined(_WINDOWS)
"    -t      trace arguments and activity (equivalent to -d100)\n"
"         t  at run time toggles gate activity trace\n"
"    -m      display microsecond timing info\n"
"         m  at run time toggles microsecond timing trace\n"
#endif	/* YYDEBUG && !defined(_WINDOWS) */
"    -d deb  +1   trace TCP/IP send actions\n"
"            +2   trace TCP/IP rcv  actions\n"
"            +100 show arguments\n"
"            +200 show more debugging details\n"
"             300 show exponential averaging\n"
"            +400 exit after initialisation\n"
"    -q      quiet - do not report clients connecting and disconnecting\n"
"    -z      block keyboard input on this app - used by -R\n"
"    -h      this help text\n"
"         T  at run time displays registrations and equivalences\n"
"         q  or ctrl+D  at run time stops %s\n"
"                      AUXILIARY app\n"
"    -R <app ...> run auxiliary app followed by -z and its arguments\n"
"                 as a separate process; -R ... must be last arguments.\n"
"\n"
"Copyright (C) 2014-2015 John E. Wulff     <immediateC@gmail.com>\n"
"Version	$Id: iCpiPWM.c 1.7 $\n"
;

/********************************************************************
 *
 *	gpioAD structures are organised as follows:
 *	  each structure describes an independent QWx or IWx word
 *
 *******************************************************************/

typedef struct	gpioAD {
    char *		name;		/* QWx QWx-i or IWx IWx-i */
    unsigned		val;		/* previous I/O value */
    double		dval;		/* previous I/O float value for exponential smoothing */
    unsigned short	channel;	/* channel to receive Q or send I from/to iCserver */
    union {
	unsigned short	gpio;		/* gpio number for QWx */
	unsigned short	adch;		/* A/D channel for IWx */
    };
    unsigned		range;		/* 0 = servo, >0 = pwm range */
    unsigned		inv;		/* normal/inverted pwm range */
    unsigned		freq;		/* pwm frequency */
    struct gpioAD *	next;		/* arrange in null terminated linked list */
} gpioAD;

char *		iC_progname;		/* name of this executable */
static char *	iC_path;
short		iC_debug = 0;
#if YYDEBUG && !defined(_WINDOWS)
int		iC_micro = 0;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
unsigned short	iC_osc_lim = 1;
int		iC_opt_B = 0;
static unsigned short	topChannel = 0;
static char	buffer[BS];
static char	SR[] = "SR";		/* generates SI for iq 0, RQ for iq 1 */
static char	IQ[] = "IQ";
static char *	ag[] = { "adch", "gpio" };
static short	errorFlag = 0;
static gpioAD *	gpioADlist[2] = { NULL, NULL };	/* linked lists of IEC word outputs or inputs */
static gpioAD**	Units = NULL;		/* dynamic array to store GPIO links */
static int	ioUnits = 0;		/* dynamically allocated size of Units[] */
static long long ownUsed = 0LL;
static unsigned bufferSizeMilliseconds = PI_DEFAULT_BUFFER_MILLIS;
static unsigned clockMicros            = PI_DEFAULT_CLK_MICROS;
static unsigned clockPeripheral        = PI_DEFAULT_CLK_PERIPHERAL;
static unsigned DMAprimaryChannel      = PI_DEFAULT_DMA_PRIMARY_CHANNEL;
static unsigned DMAsecondaryChannel    = PI_DEFAULT_DMA_SECONDARY_CHANNEL;
static unsigned memAllocMode           = PI_DEFAULT_MEM_ALLOC_MODE;

static void	storeUnit(unsigned short channel, gpioAD * gep);
static unsigned	readADC(gpioAD * gep);	/* Read A/D value */
static int	termQuit(int sig);		/* terminate pigpio functions - clear and unexport RASPBERRYPI stuff */
int		(*iC_term)(int) = &termQuit;	/* function pointer to clear and unexport RASPBERRYPI stuff */

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
    int			len;
    int			b;
    int			iq;
    int			iq1;
    unsigned		val;
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
    unsigned short	adch;
    char		tail[128];	/* must match sscanf formats %127s for tail */
    char *		mqz = "-qz";
    char *		mz  = "-z";
    char		iqc[2] = { '\0', '\0' };
    char		dum[2] = { '\0', '\0' };
    char		sp[2]  = { '\0', '\0' };
    int			iec;
    gpioAD *		gep;
    gpioAD *		gpioLast[2] = { NULL, NULL };
    unsigned		range;
    unsigned		freq;
    int			invFlag;
    int			gpioTherm;
    char **		argip;
    int			argic;
    char *		iidPtr;
    char *		iidSep;
    unsigned		ad_expf = 1;
    double		ad_expf0;
    double		ad_expf1;
    int			ad_hyst = 1;
    long long		gpioMask;
    ProcValidUsed *	gpiosp;
    uid_t		euid;
    uid_t		uid;

    iC_outFP = stdout;			/* listing file pointer */
    iC_errFP = stderr;			/* error file pointer */

#ifdef	EFENCE
    regBuf = iC_emalloc(REQUEST);
    rpyBuf = iC_emalloc(REPLY);
#endif	/* EFENCE */
    signal(SIGSEGV, iC_quit);			/* catch memory access signal */

    assert(argc);
    invFlag = 0x00;
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
    iC_iccNM = iC_progname;		/* generate our own name - not 'stdin' from tcpc.c */
    uid = getuid();
    euid = geteuid();

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
		case 'B':
		    iC_opt_B = 1;	/* start iCbox -d to monitor active GPIO I/O */
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
		    forceFlag = 1;	/* force use of GPIO's required by this program */
		    break;
		 case 'a':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    i = atoi(*argv);
		    if ((i >= PI_MEM_ALLOC_AUTO) && (i <= PI_MEM_ALLOC_MAILBOX)) {
			memAllocMode = i;
		    } else {
			fprintf(iC_errFP, "ERROR: %s: invalid -a option (%d)\n", iC_progname, i);
			errorFlag++;
		    }
		    goto break2;
		 case 'b':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    i = atoi(*argv);
		    if ((i >= PI_BUF_MILLIS_MIN) && (i <= PI_BUF_MILLIS_MAX)) {
			bufferSizeMilliseconds = i;
		    } else {
			fprintf(iC_errFP, "ERROR: %s: invalid -b option (%d)\n", iC_progname, i);
			errorFlag++;
		    }
		    goto break2;
		 case 'D':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    i = atoi(*argv);
		    if ((i >= PI_MIN_DMA_CHANNEL) && (i <= PI_MAX_PRIMARY_CHANNEL)) {
			DMAprimaryChannel = i;
		    } else {
			fprintf(iC_errFP, "ERROR: %s: invalid -D option (%d)\n", iC_progname, i);
			errorFlag++;
		    }
		    goto break2;
		 case 'E':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    i = atoi(*argv);
		    if ((i >= PI_MIN_DMA_CHANNEL) && (i <= PI_MAX_SECONDARY_CHANNEL)) {
			DMAsecondaryChannel = i;
		    } else {
			fprintf(iC_errFP, "ERROR: %s: invalid -E option (%d)\n", iC_progname, i);
			errorFlag++;
		    }
		    goto break2;
		 case 'A':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
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
			fprintf(iC_errFP, "ERROR: %s: invalid -A option (%d)\n", iC_progname, i);
			errorFlag++;
			break;
		    }
		    goto break2;
		 case 'C':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    i = atoi(*argv);
		    if ((i >= PI_CLOCK_PWM) && (i <= PI_CLOCK_PCM)) {
			clockPeripheral = i;
		    } else {
			fprintf(iC_errFP, "ERROR: %s: invalid -C option (%d)\n", iC_progname, i);
			errorFlag++;
		    }
		    goto break2;
		 case 'r':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    b = atoi(*argv);		/* A/D measurement repetition time in ms */
		    if (b < 1) b = 1;			/* TODO adjust for slow RPi, OK on RPi2 */
		    toRep.tv_sec = b / 1000;		/* seconds */
		    toRep.tv_usec = (b % 1000) * 1000;	/* remainder in us */
		    goto break2;
		 case 'e':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    ad_expf = atoi(*argv);	/* A/D exponential smoothing factor */
		    if (ad_expf > 65536) {
			fprintf(iC_errFP, "ERROR: %s: exponential smoothing factor should be <= 65536 (%d)\n",
			    iC_progname, ad_expf);
			errorFlag++;
		    }
		    ad_expf0 = (double)ad_expf;
		    ad_expf1 = (double)(ad_expf - 1);
		    goto break2;
		 case 'H':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    ad_hyst = atoi(*argv);	/* A/D measurement hysteresis */
		    if (ad_hyst > 511) {
			fprintf(iC_errFP, "ERROR: %s: hysteresis should be <= 511 (%d)\n",
			    iC_progname, ad_hyst);
			errorFlag++;
		    }
		    if (ad_hyst < 1) ad_hyst = 1;
		    goto break2;
		case 'd':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if ((slen = strlen(*argv)) != strspn(*argv, "01234567") ||	/* octal digits only */
			sscanf(*argv, "%o", &i) != 1 ||
			i & ~0777) {
			fprintf(iC_errFP, "ERROR: %s: '-d %s' is not a well formed octal string or exceeds range 777\n",
			    iC_progname, *argv);
			errorFlag++;
		    }
		    iC_debug |= i;	/* short */
		    goto break2;
#if YYDEBUG && !defined(_WINDOWS)
		case 't':
		    iC_debug |= 0100;	/* trace arguments and activity */
		    break;
		case 'm':
		    iC_micro++;		/* microsecond info */
		    break;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
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
		    fprintf(iC_errFP, usage, iC_progname, iC_hostNM, iC_portNM, INSTSIZE,
			DEFAULT_PWM_RANGE, DEFAULT_PWM_FREQUENCY, iC_progname, INSTSIZE, iC_progname, iC_progname);
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
	     *  been determined. IEC arguments and options may be mixed (except -R).
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
  break3:
    /********************************************************************
     *  if argc != 0 then -R and argv points to auxialliary app + arguments
     *               do not fork and execute aux app until this app has
     *               connected to iCserver and registered all its I/Os
     *******************************************************************/
    if (iC_debug & 0200) fprintf(iC_outFP, "fullPath = '%s' path = '%s' progname = '%s'\n",
	iC_fullProgname, iC_path, iC_progname);
    if (iC_debug & 0200) fprintf(iC_outFP, "uid = %d euid = %d\n", (int)uid, (int)euid);
    if (! errorFlag && argic) {
	/********************************************************************
	 *  Open or create and lock the auxiliary file ~/.iC/gpios.rev,
	 *  which contains one binary struct ProcValidUsed gpios.
	 *  Must be created with normal uid, not euid 0 (root), so that
	 *  other processes like iCpiFace etc can modify gpios.rev.
	 *
	 *  If gpios.rev exists, gpios.proc and gpios.valid is checked
	 *  against the values returned by boardrev(). (Must be the same)
	 *  For an RPi B+ processor gpios.rev contains:
	 *  	gpios.proc	1
	 *  	gpios.valid	0x00000000fbc6cf9c
	 *  	gpios.used	0x0000000000000000
	 *
	 *  gpios.proc is the int returned by boardrev() // Raspberry Pi Board revision
	 *
	 *  gpios.valid has 64 bits - a bit is set for each valid GPIO
	 *
	 *  gpios.used has 64 bits - a bit is set for each GPIO used in other apps.
	 *  If -f (forceFlag) is set the gpios.used is cleared unconditionally
	 *  Else only GPIO's not used in other apps can be used in this app.
	 *
	 *  Subsequently bits are set for GPIO's used in this app, so they are
	 *  not used a 2nd time, which is effective immediately in this app.
	 *
	 *  Finally the modified gpios.used is written to gpios.rev by a call to
	 *  writeUnlockCloseGpios(), so other apps see which GPIO's have been used.
	 *******************************************************************/
	if (euid == 0) {
	    if (seteuid(uid) != 0) {		/* execute openLockGpios() at uid privileges */
		perror("seteuid failed");	/* hard ERROR */
		iC_quit(SIGUSR1);		/* error quit */
	    }
	}
	if ((gpiosp = openLockGpios(forceFlag)) == NULL) {
	    fprintf(iC_errFP, "ERROR: %s: in openLockGpios()\n",
		iC_progname);
	    errorFlag++;
	    goto FreeNow;
	}
	if (euid == 0) {
	    if (seteuid(euid) != 0) {		/* restore root privileges */
		perror("seteuid failed");	/* hard ERROR */
		iC_quit(SIGUSR1);		/* error quit */
	    }
	}
	gpiosp->valid |= 0xffLL << 56;			/* TODO make dependent on available A/D */
	if (iC_debug & 0200) fprintf(iC_outFP,
	    "Raspberry Pi Board revision = %d (0 = A,A+; 1 = B; 2 = B+,2B)\n"
	    "valid    = 0x%016llx\n",
	    gpiosp->proc, gpiosp->valid);
	/********************************************************************
	 *  Analyze IEC arguments
	 *******************************************************************/
	gpioMask = 0LL;
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
	    if ((b = sscanf(tail, "%1[IQ]W%5d,%hu%127s", iqc, &iec, &gpio, tail)) >= 3) {
		if (b == 3) *tail = '\0';
		if ((iq = (*iqc == 'Q') ? 1 : 0)) {
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
		    /********************************************************************
		     *  IWx,<adc_channel> - single-ended or differential adc_channel
		     *  NOTE: single/diff bit 3 in parameter is inverted for MCP3008 hardware
		     *******************************************************************/
		    if (gpio == 8 || gpio == 9 || gpio >= 18) {
			fprintf(iC_errFP, "ERROR: %s: IW%d,%hu - adc_channel must be 0 to 7 or 10 to 17\n",
			    iC_progname, iec, gpio);
			errorFlag++;
			goto endOneArg;
		    }
		    if ((adch = gpio) >= 10) {
			adch -= 2;			/* convert decimal to octal */
		    }
		    adch ^= 0x08;			/* invert single/diff bit 3 */
		    adch <<= 4;				/* move to bit 7 to 4 for txBuf[1] */
		    range = freq = 0;
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
	    /********************************************************************
	     *  Use the IEC argument to extend GPIO control structures
	     *  For QWx check that the GPIO for the IEC argument is valid for this RPi
	     *  and was not used before in this or another app.
	     *  For IWx check that the A/D channel was not used before in this app
	     *  NOTE: for differential A/D 2 channels are used for each channel value
	     *******************************************************************/
	    gpioMask = iq ? 1LL << gpio						/* gpio is 0 - 55 max */
		      : adch & 0x80 ? 1LL << (((adch & 0x0070) >> 4) + 56)	/* bit 56 - 63, single ended A/D */
				    : 3LL << (((adch & 0x0060) >> 4) + 56);	/* bit 56 - 63, differential A/D */
	    if (iC_debug & 0200) fprintf(iC_outFP,
		"%cW%d %s = %hu range = %u freq = %u instance = '%s' b = %d sp = '%s' tail = '%s'\n"
		"used     = 0x%016llx\n"
		"gpioMask = 0x%016llx\n",
		IQ[iq], iec, ag[iq], gpio, range, freq, iids, b, sp, tail,
		gpiosp->u.used, gpioMask);
	    if (!(gpiosp->valid & gpioMask)) {
		fprintf(iC_errFP, "ERROR: %s: trying to use invalid %s %hu on %cW%hu%s on RPi board rev %d\n",
		    iC_progname, ag[iq], gpio, IQ[iq], iec, iids, gpiosp->proc);
		errorFlag++;
	    } else if (gpiosp->u.used & gpioMask) {
		fprintf(iC_errFP, "ERROR: %s: trying to use %s %hu a 2nd time on %cW%hu%s\n",
		    iC_progname, ag[iq], gpio, IQ[iq], iec, iids);
		errorFlag++;
	    } else {
		/********************************************************************
		 *  Generate a unique IEC name for network registration
		 *******************************************************************/
		if (iid != 0xffff && *iids == '\0') {
		    snprintf(iids, 6, "-%d", iid);	/* global instance */
		}
		len = snprintf(buffer, BS, "%cW%d%s", IQ[iq], iec, iids);	/* IEC name with possible instance */
		for (iq1 = 0; iq1 <= 1; iq1++) {
		    for (gep = gpioADlist[iq1]; gep; gep = gep->next) {
			if (strcmp(gep->name, buffer) == 0) {
			    fprintf(iC_errFP, "ERROR: %s: trying to use %s a 2nd time\n", iC_progname, buffer);
			    errorFlag++;
			    goto endOneArg;		/* not unique */
			}
		    }
		}
		gpiosp->u.used |= gpioMask;		/* mark gpio or adch used for ~/.iC/gpios.rev */
		ownUsed        |= gpioMask;		/* mark gpio or adch in ownUsed for termination */
		gep = iC_emalloc(sizeof(gpioAD));	/* new gpioAD element */
		if (gpioADlist[iq] == NULL) {
		    gpioADlist[iq] = gep;		/* first element */
		} else {
		    assert(gpioLast[iq]);
		    gpioLast[iq]->next = gep;	/* previous last element linked to new element */
		}
		gpioLast[iq] = gep;		/* this is new last element */
		gep->name = iC_emalloc(++len);
		strcpy(gep->name, buffer);	/* store IEC name */
		if (iq) {
		    gep->gpio = gpio;		/* gpio number for this QWx */
		} else {
		    gep->adch = adch;		/* A/D channel for IWx */
		}
		gep->range = range;		/* range (0 = servo or PWM range) for this word */
		if (range && iq) {
		    gep->inv = invFlag;		/* normal/inverted PWM output range unless SERVO */
		} else if (invFlag) {
		    fprintf(iC_errFP, "WARNING: %s: cannot invert SERVO output or A/D input %s - ignore\n", iC_progname, buffer);
		}
		gep->freq = freq;		/* PWM freq for this word */
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
	  endOneArg: ;
	    invFlag = 0x00;			/* start new tilde '~' analysis for next argument */
	  skipInvFlag:;
	}
	gpiosp->valid &= ~(0xffLL << 56);		/* TODO make dependent on available A/D */
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
    if (gpioADlist[0] == NULL && gpioADlist[1] == NULL) {
	fprintf(iC_errFP, "ERROR: %s: no valid IEC arguments? there must be at least 1 valid argument\n",
	    iC_progname);
	iC_quit(-4);					/* call termQuit() to terminate I/O */
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
    len = snprintf(buffer, BS, "%s_", iC_iccNM);
    for (gep = gpioADlist[0]; gep; gep = gep->next) {
	len += snprintf(buffer+len, BS-len, "A%ho", (gep->adch ^ 0x80) >> 4);	/* A/D channel */
    }
    for (gep = gpioADlist[1]; gep; gep = gep->next) {
	len += snprintf(buffer+len, BS-len, "G%hu", gep->gpio);	/* GPIO */
    }
    if (strlen(iC_iidNM) > 0) {
	len += snprintf(buffer+len, BS-len, "-%s", iC_iidNM);	/* name-instance header */
    }
    iC_iccNM = iC_emalloc(len + 1);
    strcpy(iC_iccNM, buffer);
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_micro) iC_microReset(0);		/* start timing */
#endif	/* YYDEBUG && !defined(_WINDOWS) */
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
     *  Extend registration string with all active GPIO output and A/D input names
     *******************************************************************/
    for (iq = 0; iq <= 1; iq++) {
	for (gep = gpioADlist[iq]; gep; gep = gep->next) {
	    assert(regBufLen > ENTRYSZ);	/* accomodates ",RQW123456,Z" */
	    len = snprintf(cp, regBufLen, ",%c%s", SR[iq], gep->name);
	    cp += len;				/* input send name or output receive name */
	    regBufLen -= len;
	}
    }
    /********************************************************************
     *  Send registration string made up of all active I/O names
     *******************************************************************/
    if (iC_debug & 0200) fprintf(iC_outFP, "regBufLen = %d\n", regBufLen);
    snprintf(cp, regBufLen, ",Z");		/* Z terminator */
    if (iC_debug & 0200) fprintf(iC_outFP, "register:%s\n", regBuf);
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_micro) iC_microPrint("send registration", 0);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    iC_send_msg_to_server(iC_sockFN, regBuf);	/* register IOs with iCserver */
    /********************************************************************
     *  Receive a channel number for each I/O name sent in registration
     *  Distribute read and/or write channels returned in acknowledgment
     *******************************************************************/
    if (iC_rcvd_msg_from_server(iC_sockFN, rpyBuf, REPLY) != 0) {	/* busy wait for acknowledgment reply */
	int	ntok;
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_micro) iC_microPrint("ack received", 0);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	if (iC_debug & 0200) fprintf(iC_outFP, "reply:%s\n", rpyBuf);
	if (iC_opt_B) {				/* prepare to execute iCbox -d */
	    len = snprintf(buffer, BS, "iCbox -dz%s -n %s-DI", (iC_debug & DQ) ? "q" : "", iC_iccNM);
	    b = ntok = 4;			/* initial number of tokens in buffer */
	    op = buffer + len;			/* optional host and port appended in iC_fork_and_exec() */
	    ol = BS - len;
	}
	cp = rpyBuf - 1;	/* increment to first character in rpyBuf in first use of cp */
	/********************************************************************
	 *  Channels for GPIO acknowledgments
	 *******************************************************************/
	for (iq = 0; iq <= 1; iq++) {
	    for (gep = gpioADlist[iq]; gep; gep = gep->next) {
		assert(cp);			/* not enough channels in ACK string */
		channel = atoi(++cp);		/* read channel from ACK string */
		assert(channel > 0);
		if (channel > topChannel) {
		    topChannel = channel;
		}
		if (iC_debug & 0200) {
		    if (iq == 0) {
			fprintf(iC_outFP, "A/D %c%hu	%s  on channel %hu\n",
			    (gep->adch & 0x80) ? 'S' : 'D', (gep->adch & 0x70) >> 4, gep->name, channel);
		    } else {
			fprintf(iC_outFP, "GPIO %hu	%s  on channel %hu\n", gep->gpio, gep->name, channel);
		    }
		}
		gep->channel = channel;		/* link channel to GPIO (ignore receive channel) */
		storeUnit(channel, gep);	/* link GPIO element pointer to send channel */
		if (iC_opt_B) {
		    assert(ol > 24);
		    len = snprintf(op, ol, " %s", gep->name);	/* add I/O name[-inst] to execute iCbox -d */
		    op += len;
		    ol -= len;
		    if (iq == 0) {
			len = snprintf(op, ol, ",0,1023");	/* A/D input iCbox slider */
		    } else
		    if ((range = gep->range) == 0) {
			len = snprintf(op, ol, ",500,2500");	/* SERVO range of iCbox slider */
		    } else {
			len = snprintf(op, ol, ",0,%u", range);	/* PWM range of iCbox slider */
		    }
		    b++;			/* count tokens in buffer */
		    op += len;
		    ol -= len;
		}
		cp = strchr(cp, ',');
	    }
	}
	assert(cp == NULL);			/* Ack string matches Registration */
	/********************************************************************
	 *  ACK string complete
	 *******************************************************************/
	if (iC_opt_B && b > ntok) {
	    iC_fork_and_exec(iC_string2argv(buffer, b));	/* fork iCbox -d */
	}
	if (iC_debug & 0200) fprintf(iC_outFP, "reply: top channel = %hu\n", topChannel);
    } else {
	iC_quit(QUIT_SERVER);				/* quit normally with 0 length message */
    }
    /********************************************************************
     *  Report results of argument analysis
     *******************************************************************/
    if (iC_debug & 0300) {
	fprintf(iC_outFP, "Allocation for A/D and GPIO elements, global instance = \"%s\"\n", iC_iidNM);
	fprintf(iC_outFP, "	IEC   adch/gpio	 range	 freq	channel	instance\n\n");
	for (iq = 0; iq <= 1; iq++) {
	    for (gep = gpioADlist[iq]; gep; gep = gep->next) {
		strcpy(buffer, gep->name);	/* retrieve name[-instance] */
		if ((iidPtr = strchr(buffer, '-')) != NULL) {
		    *iidPtr++ = '\0';		/* separate name and instance */
		    iidSep = "	-";
		} else {
		    iidPtr = iidSep = "";
		}
		if (iq == 0) {
		    fprintf(iC_outFP, "	%s	 %c%hu			%4hu%s%s\n",
			buffer, (gep->adch & 0x80) ? 'S' : 'D', (gep->adch & 0x70) >> 4,
			gep->channel, iidSep, iidPtr);
		} else {
		    fprintf(iC_outFP, "	%s	 %2hu	%c%4u	%5u	%4hu%s%s\n",
			buffer, gep->gpio, gep->inv ? '~' : ' ', gep->range, gep->freq, gep->channel, iidSep, iidPtr);
		}
	    }
	}
	fprintf(iC_outFP, "\n");
    }
    /********************************************************************
     *  Open A/D on /dev/spidev0.0 and set timeout value if any IW<x> parameters
     *******************************************************************/
    if (gpioADlist[0]) {
	assert(spiFN < 0);
	if ((spiFN = spiOpen(0, 100000, 0)) < 0) {
	    fprintf(iC_errFP, "WARNING: %s: open A/D on /dev/spidev0.0 failed\n", iC_progname);
	} else {
	    toCnt = toRep;			/* set timeout value for A/D conversions */
	    toCntp = &toCnt;
	}
    }
    /********************************************************************
     *  Initialize active A/D inputs
     *  Set initial dval to start off exponential smoothing correctly
     *  Set initial val to an impossible value to force TCP output on first measurement
     *******************************************************************/
    for (gep = gpioADlist[0]; gep; gep = gep->next) {
	gep->dval = readADC(gep);		/* 10 bit A/D conversion value */
	gep->val = 65535;			/* impossible value > 1023 */
    }
    /********************************************************************
     *  Generate GPIO PWM initialisation for active outputs
     *  Set range, frequency and initial value:
     *    SERVO: 0,     0,    0 # which turns SERVO off (no pulses)
     *    PWM:   range, freq, 0 # which turns PWM off (duty/cycle 0 = 0V out)
     *******************************************************************/
    for (gep = gpioADlist[1]; gep; gep = gep->next) {
	assert(gep->val == 0);			/* all gpioAD entries val initialised to 0 */
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
    FD_SET(iC_sockFN, &infds);		/* watch sock for inputs */
    if ((iC_debug & DZ) == 0) FD_SET(0, &infds);	/* watch stdin for inputs unless - FD_CLR on EOF */
    if (iC_debug & 0200) fprintf(iC_outFP, "iC_sockFN = %d\n", iC_sockFN);
    /********************************************************************
     *  External input (TCP/IP via socket and STDIN)
     *  Wait for input in a select statement most of the time
     *******************************************************************/
    for (;;) {
	if ((retval = iC_wait_for_next_event(&infds, 0, toCntp)) == 0)
	{
	    if (toCnt.tv_sec == 0 && toCnt.tv_usec == 0) {
		toCnt = toRep;			/* re-initialise timeout value */
		/********************************************************************
		 *  Carry out A/D conversion on all allocated A/D channels
		 *******************************************************************/
		cp = regBuf;
		regBufLen = REPLY;
		if (iC_debug & 0100) {
		    op = buffer;
		    ol = BS;
		}
		for (gep = gpioADlist[0]; gep; gep = gep->next) {	/* scan A/D conversion inputs */
		    val = readADC(gep);				/* 10 bit A/D conversion value */
		    if ((iC_debug & 0300) == 0300) fprintf(iC_outFP, "prev = %u	val = %u", gep->val, val);
		    if (ad_expf > 1) {
			/********************************************************************
			 *  If expf > 1, exponential smoothing is applied to each
			 *  measurement, which is equivalent to using an R/C filter.
			 *  Out = previousOut * (expf-1)/expf + measurement * 1/expf
			 *      ad_expf0 = (double)ad_expf;
			 *      ad_expf1 = (double)(ad_expf - 1);
			 *******************************************************************/
			val = (unsigned)((gep->dval = (gep->dval * ad_expf1 + (double)val) / ad_expf0) + 0.5);
			if ((iC_debug & 0300) == 0300) fprintf(iC_outFP, "	dval = %4.6f	val = %u", gep->dval, val);
		    }
		    if ((iC_debug & 0300) == 0300) fprintf(iC_outFP, "\n");
		    if (ad_hyst > 1) {
			/********************************************************************
			 *  If hyst > 1 measurements are only output if
			 *  abs(Out - prevOut) >= hyst
			 *******************************************************************/
			if (abs(val - gep->val) < ad_hyst) {
			    goto skipDataTelegram;
			}
		    }
		    if (val != gep->val) {
			len = snprintf(cp, regBufLen, ",%hu:%u", gep->channel, val); /* data telegram */
			cp += len;
			regBufLen -= len;
			if (iC_debug & 0100) {
			    len = snprintf(op, ol, " A %s,%ho",	/* source name, adc_channel */
				gep->name, (gep->adch ^ 0x80) >> 4);
			    op += len;
			    ol -= len;
			}
			gep->val = val;		/* latest A/D value for comparison */
		    }
		  skipDataTelegram: ;
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
#if YYDEBUG && !defined(_WINDOWS)
		    if (iC_micro) iC_microPrint("TCP input received", 0);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		    assert(Units);
		    cp = rpyBuf - 1;		/* increment to first character in rpyBuf in first use of cp */
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
			fprintf(iC_errFP, "WARNING: %s: received '%s' from iCserver ???\n", iC_iccNM, rpyBuf);
		    }
		} else {
		    iC_quit(QUIT_SERVER);	/* quit normally with 0 length message from iCserver */
		}
	    }	/* end of TCP/IP input */
	    /********************************************************************
	     *  STDIN interrupt
	     *******************************************************************/
	    if (FD_ISSET(0, &iC_rdfds)) {
		if (fgets(buffer, BS, stdin) == NULL) {
		    FD_CLR(0, &infds);		/* ignore EOF - happens in bg or file - turn off interrupts */
		    buffer[0] = '\0';		/* notify EOF to iC application by zero length buffer */
		}
		if ((b = buffer[0]) == 'q' || b == '\0') {
		    iC_quit(QUIT_TERMINAL);	/* quit normally with 'q' or ctrl+D */
#if YYDEBUG && !defined(_WINDOWS)
		} else if (b == 't') {
		    iC_debug ^= 0100;		/* toggle -t flag */
		} else if (b == 'm') {
		    iC_micro ^= 1;		/* toggle micro */
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		} else if (b == 'T') {
		    iC_send_msg_to_server(iC_sockFN, "T");	/* print iCserver tables */
		} else if (b != '\n') {
		    fprintf(iC_errFP, "no action coded for '%c' - try t, m, T, or q followed by ENTER\n", b);
		}
	    }	/*  end of STDIN interrupt */
	} else {
	    perror("ERROR: select failed");	/* retval -1 */
	    iC_quit(SIGUSR1);			/* error quit */
	}
    }
} /* main */

/********************************************************************
 *
 *	Initalise and expand dynamic array Units[] as necessary
 *	Store gpioAD * to current GPIO element
 *
 *******************************************************************/

static void
storeUnit(unsigned short channel, gpioAD * gep)
{
    while (channel >= ioUnits) {
	Units = (gpioAD **)realloc(Units,		/* initially NULL */
	    (ioUnits + IOUNITS) * sizeof(gpioAD *));
	assert(Units);
	memset(&Units[ioUnits], '\0', IOUNITS * sizeof(gpioAD *));
	ioUnits += IOUNITS;			/* increase the size of the array */
	if (iC_debug & 0200) fprintf(iC_outFP, "storeUnit: Units[%d] increase\n", ioUnits);
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "storeUnit: Units[%d] <= %s\n", channel, gep->name);
    Units[channel] = gep;			/* store gpioAD * */
} /* storeUnit */

/********************************************************************
 *
 *	Read A/D value
 *
 *******************************************************************/

static unsigned
readADC(gpioAD * gep)
{
    txBuf[0] = 0x01;				/* start bit */
    txBuf[1] = gep->adch;			/* single-ended or differential adc_channel */
						/* txBuf[2] = Dont't Care; */
    if (spiXfer(spiFN, txBuf, rxBuf, 3) != 3) {
	fprintf(iC_errFP, "WARNING: %s: xfer A/D on /dev/spidev0.0 failed\n", iC_progname);
    }
    return (rxBuf[1] & 0x03) << 8 | rxBuf[2];	/* 10 bit A/D conversion value */
} /* readADC */

/********************************************************************
 *
 *	Terminate pigpio functions
 *	Clear gpiosp->u.used bits for GPIOs and A/D channels used in this app
 *
 *******************************************************************/

static int
termQuit(int sig)
{
    if (iC_debug & 0200) fprintf(iC_outFP, "=== Terminate GPIOs and pigpio functions ===\n");
    int			b;
    gpioAD *		gep;
    ProcValidUsed *	gpiosp;

    for (gep = gpioADlist[1]; gep; gep = gep->next) {
	gep->val = 0;		/* all gpioAD entries val back to 0 */
	if (gep->range == 0) {
	    /********************************************************************
	     *  Write GPIO QWx SERVO final 0 output
	     *******************************************************************/
	    if (iC_debug & 0200) fprintf(iC_outFP, "### Terminate SERVO output on GPIO %d\n", gep->gpio);
	    if ((b = gpioServo(gep->gpio, gep->val)) < 0) {
		fprintf(iC_errFP, "WARNING: %s: Can't execute final 'gpioServo(%hu, %u)' ?%d in pigpio library\n",
		    iC_progname, gep->gpio, gep->val, b);
	    }
	} else {
	    /********************************************************************
	     *  Write GPIO QWx PWM final 0 output
	     *******************************************************************/
	    if (iC_debug & 0200) fprintf(iC_outFP, "### Terminate PWM output on GPIO %d\n", gep->gpio);
	    if ((b = gpioPWM(gep->gpio, gep->val)) < 0) {
		fprintf(iC_errFP, "WARNING: %s: Can't execute final 'gpioPWM(%hu, %u)' ?%d in pigpio library\n",
		    iC_progname, gep->gpio, gep->val, b);
	    }
	}
    }
    nanosleep(&ms100, NULL);		/* settle for another 100 ms */
    if (spiFN >= 0) {
	spiClose(spiFN);
	spiFN = -1;
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "### Terminate pigpio functions\n");
    gpioTerminate();			/* pigpio library stuff */
    /********************************************************************
     *  Open and lock the auxiliary file ~/.iC/gpios.rev again
     *  Other apps may have set used bits since this app was started
     *******************************************************************/
    if ((gpiosp = openLockGpios(0)) == NULL) {
	fprintf(iC_errFP, "ERROR: %s: in openLockGpios()\n",
	    iC_progname);
	return (SIGUSR1);		/* error quit */
    }
    gpiosp->u.used &= ~ownUsed;		/* clear all bits for GPIOs and A/D channels used in this app */
    if (writeUnlockCloseGpios() < 0) {	/* unlink (remove) ~/.iC/gpios.rev if gpios->u.used and oops is 0 */
	fprintf(iC_errFP, "ERROR: %s: in writeUnlockCloseGpios()\n",
	    iC_progname);
	return (SIGUSR1);		/* error quit */
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "=== End Terminate GPIOs and pigpio =========\n");
    return (sig);			/* finally quit */
} /* termQuit */
#endif	/* defined(RASPBERRYPI) && defined(PWM) && defined(TCP) && !defined(_WINDOWS) */

/* ############ POD to generate iCpiPWM man page ############################

=encoding utf8

=head1 NAME

iCpiPWM - real PWM analog I/O on a Raspberry Pi for the iC environment

=head1 SYNOPSIS

 iCpiPWM [-Bftmqzh][ -s <host>][ -p <port>][ -n <name>][ -i <inst>]
         [ -a <val>][ -b <val>][ -D <val>][ -E <val>]
         [ -A <val>][ -C <val>][ -d <deb>]
         [ [~]QW<x>,<gpio>,p[,<range>[,<freq>]][-<inst>] ...]
         [ QW<x>,<gpio>,s[-<inst>] ...]
         [ IW<x>,<adc_channel>[-<inst>] ...]
         [ -r <time>][ -e <expf>][ -H <hyst>]
         [ -R <aux_app>[ <aux_app_argument> ...]] # must be last arguments
    -s host IP address of server    (default '127.0.0.1')
    -p port service port of server  (default '8778')
    -i inst instance of this client (default '') or 1 to 3 digits
    -B      start iCbox -d to monitor active analog I/O
    -W GPIO number used by the w1-gpio kernel module (default 4, maximum 31).
            When the GPIO with this number is used in this app, iCtherm is
            permanently blocked to avoid Oops errors in module w1-gpio.
    -f      force use of GPIO's required by this program
                      PIGPIO initialisation arguments
    -a val  DMA mode, 0=AUTO, 1=PMAP, 2=MBOX,   default AUTO
    -b val  gpio sample buffer in milliseconds, default 120
    -D val  primary DMA channel, 0-14,          default 14
    -E val  secondary DMA channel, 0-6,         default 5
    -A val  sample rate, 1, 2, 4, 5, 8, or 10,  default 5 us
    -C val  clock peripheral, 0=PWM 1=PCM,      default PCM
                      GPIO PWM and SERVO IEC output arguments
    QW<x>,<gpio>,p[,<range>[,<freq>]]
            generates pulse width modulated (PWM) pulses on the gpio,
            QW<x> = 0 (off) to <range> (fully on)
          <x>     is any unique and valid IEC number (0 - 65535)
          <gpio>  is any unique GPIO supported by the board revision
                  and not used on iCpiFace or an iC app with direct I/O
          <range> PWM range                     default 500
          <freq>  PWM frequency >= 0            default 400 Hz
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
          <x>     is any unique and valid IEC number (0 - 65535)
          <adc_channel> can be 0 to 7 or 10 to 17
            0   single-ended CH0
            1   single-ended CH1
            2   single-ended CH2
            3   single-ended CH3
            4   single-ended CH4
            5   single-ended CH5
            6   single-ended CH6
            7   single-ended CH7
            10  differential CH0 = IN+ CH1 = IN-
            11  differential CH0 = IN- CH1 = IN+
            12  differential CH2 = IN+ CH3 = IN-
            13  differential CH2 = IN- CH3 = IN+
            14  differential CH4 = IN+ CH5 = IN-
            15  differential CH4 = IN- CH5 = IN+
            16  differential CH6 = IN+ CH7 = IN-
            17  differential CH6 = IN- CH7 = IN+
    -r time A/D measurement repetition time in ms (default 100 ms)
    -e expf A/D measurement exponential smoothing factor (default 1)
            If expf > 1, exponential smoothing is applied to each
            measurement, which is equivalent to using an R/C filter.
            Out = previousOut * (expf-1)/expf + measurement * 1/expf
    -H hyst A/D measurement hysteresis (default 1)
            If hyst > 1 measurements are only output if
            abs(Out - prevOut) >= hyst
                      COMMON extension
    IEC-inst Each individual IEC argument can be followed by -<inst>,
          where <inst> consists of 1 to 3 numeric digits.
          Such an appended instance code takes precedence over the
          general instance specified with the -i <inst> option above.
    NOTE: there must be at least 1 IEC QW<x> or IW<x> argument.
          No IEC arguments are generated automatically for iCpiPWM.
                      DEBUG options
    -t      trace arguments and activity (equivalent to -d100)
    -m      display microsecond timing info
    -d deb  +1   trace TCP/IP send actions
            +2   trace TCP/IP rcv  actions
            +100 show arguments
            +200 show more debugging details
             300 show exponential averaging
            +400 exit after initialisation
    -q      quiet - do not report clients connecting and disconnecting
    -z      block keyboard input on this app - used by -R
    -h      this help text
         T  at run time displays registrations and equivalences
         q  or ctrl+D  at run time stops iCpiPWM
                      AUXILIARY app
    -R <app ...> run auxiliary app followed by -z and its arguments
                 as a separate process; -R ... must be last arguments.

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

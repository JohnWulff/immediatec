/********************************************************************
 *
 *	Copyright (C) 2023  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	iCpiI2C
 *
 *	I/O driver for iCserver in the immediate C environment of a
 *	Raspberry Pi handling real 16 bit inputs and/or 16 bit outputs
 *	for each MCP23017 controller attached to a Raspberry Pi via 8
 *	multiplexed I2C busses of a PCA9548A I2C Switch, or from a number
 *	of direct GPIO pins on the Raspberry Pi or both.
 *
 *	To enable the PCA9548A I2C Mux/Switch in the Linux kernel, enable
 *	I2C in "3 Interface Options" in "raspi-config" and also add the
 *	following lines (without *s) to the /boot/config.txt file:
 *	    [all]
 *	    # Activate the I2C mux in the Linux kernel
 *	    dtoverlay=i2c-mux,pca9548,addr=0x70
 *	This will activate the mux at address 0x70 and create /dev/i2c-11
 *	to /dev/i2c-18 for SD0/SC0 to SD7/SC7 on the mux.
 *
 *	A maximum of 63 MCP23017 controllers can be handled altogether,
 *	8 MCP23017 controllers per multiplexed I2C channel /dev/i2c-11 to
 *	/dev/i2c-17 and 7 MCP23017 controllers for channel /dev/i2c-18.
 *	Each MCP23017 has 16 bit I/Os in two 8 bit registers A and B. Each
 *	bit I/O can be either an input or an output or not used at all.
 *
 *	To concentrate the interrupts from INTA and INTB of all 8 groups
 *	of MCP23017 controllers a special MCP23017 is used at address 0x27
 *	of /dev/i2c-18. The INTA and INTB output of this MCP are mirrored
 *	and are connected to GPIO 27 for interrupt handling.
 *
 *	All GPIO pins on a Raspberry Pi A, B, B+, 3B+ or 4 may be selected
 *	as either input bits or output bits independent of whether MCP23017s
 *	are present or not, except GPIO 2, 3 or 27 if MCP23017s are also
 *	processed. All MCP23017 and GPIO inputs are handled by interrupts.
 *
 *	If no MCP23017 controllers are found or if the program is called
 *	with the -G option, only GPIO pins will be handled.
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
#include	"mcp23017.h"
#include	"bcm2835.h"

/********************************************************************
 *  Select either a mcpIO mcpL[un] with a small int un as index
 *  Alternativeley select a gpioIO element with the pointer gep
 *******************************************************************/

typedef struct	mcDetails {
    char *		name;		/* name IXn, IXn-i, QXn or QXn-i */
    int			val;		/* previous input or output value */
    unsigned short	channel;	/* channel to send I or receive Q to/from iCserver */
    uint8_t		inv;		/* in or out inversion mask */
    uint8_t		bmask;		/* selected bits for this input or output */
} mcDetails;

typedef union	chS {
    int		un		/* always < 128 for MCP23017, > 128 if gpioIO* for GPIO */;
    gpioIO *	gep;
} chS;

/********************************************************************
 *	mcpIO structures are organised as follows:
 *	  Each MCP register can have both input and output bits.
 *	  Each structure describes an MCP register A or B with
 *	  independent IXn or QXn bit groupsi selected with iq.
 *      Using s[iq][g] allows A or B selection with g value from IEC
 *******************************************************************/

typedef struct	mcpIO {
    mcDetails		s[2][2];	/* IXn or QXn details / register A or B */
    unsigned short	cn;		/* MCP23017 address 0 - 61 */
    int			i2cFN;		/* file number for I2C channels 11 to 18 */
} mcpIO;

static	int	i2cFdA[10] =			/* file descriptors for open I2C channels */
	    { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };
static int	i2cFdCnt   = 0;			/* number of open I2C channels */
static int	gpio27FN = -1;			/* /sys/class/gpio/gpio27/value I2C file number */
static mcpIO *	mcpL[64];			/* array of pointers to MCP23017 info *mcpP */
						/* -1     MCP23017 not found */
						/* NULL   MCP23017 found but not configured */
						/* *mcpP  MCP23017 info configured */

static  char **		argp;
static  int		offset  = 0;
static  int		invFlag = 0;
static fd_set		infds;			/* initialised file descriptor set for normal iC_wait_for_next_event() */
static fd_set		ixfds;			/* initialised extra descriptor set for normal iC_wait_for_next_event() */

static int scanIEC(const char * format, char * iqs, char * iqe, char * cng, int * ieStartp, int * ieEndp, char * tail);

static const char *	usage =
"Usage:\n"
" iCpiI2C  [-GBIftmqzh][ -s <host>][ -p <port>][ -n <name>][ -i <inst>]\n"
"          [ -o <offs>][ -d <deb>]\n"
"          [ [~]IXcng[,IXcng,...][,<mask>][-<inst>] ...]\n"
"          [ [~]QXcng[,QXcng,...][,<mask>][-<inst>] ...]\n"
"          [ [~]IXcng-IXcng[,<mask>][-<inst>] ...]\n"
"          [ [~]QXcng-QXcng[,<mask>][-<inst>] ...]\n"
"          [ [~]IXn,<gpio>[,<gpio>,...][-<inst>] ...]\n"
"          [ [~]QXn,<gpio>[,<gpio>,...][-<inst>] ...]\n"
"          [ [~]IXn.<bit>,<gpio>[-<inst>] ...]\n"
"          [ [~]QXn.<bit>,<gpio>[-<inst>] ...]      # at least 1 IEC argument\n"
"          [ -R <aux_app>[ <aux_app_argument> ...]] # must be last arguments\n"
"    -s host IP address of server    (default '%s')\n"
"    -p port service port of server  (default '%s')\n"
"    -i inst instance of this client (default '') or 1 to %d digits\n"
"    -o offs offset for IEC GPIO selection in a second iCpiI2C in the same\n"
"            network (default 0; 2, 4, 6, 8 for extra iCpI2C's - see below)\n"
"            All IXcng and QXcng declarations must use g = offs or offs+1\n"
"    -G      service GPIO I/O only - block MCP23017s and MCP23017 arguments\n"
"    -B      start iCbox -d to monitor active MCP23017 and/or GPIO I/O\n"
"    -I      invert all MCP23017 and GPIO inputs and outputs\n"
"            MCP23017 and GPIO input circuits are normally hi - 1 and\n"
"            when a switch on the input is pressed it goes lo - 0. Therefore\n"
"            it is appropriate to invert inputs and outputs. When inverted\n"
"            a switch pressed on an input generates a 1 for the IEC inputs and\n"
"            a 1 on an IEC output turns a LED and relay on, which is natural.\n"
"            An exception is a relay with an inverting driver, in which case\n"
"            that output must be non-inverting.\n"
"    -W GPIO number used by the w1-gpio kernel module (default 4, maximum 31).\n"
"            When the GPIO with this number is used in this app, iCtherm is\n"
"            permanently blocked to avoid Oops errors in module w1-gpio.\n"
"    -f      force use of all interrupting gpio inputs - in particular\n"
"            force use of gpio 27 - some programs do not unexport correctly.\n"
"\n"
"                      MCP23017 arguments\n"
"          For each MCP23017 connected to a Raspberry Pi two 8 bit registers\n"
"          GPIOA and GPIOB must be configured as IEC inputs (IXcng) or\n"
"          outputs (IXcng) or both, where cng is a three digit number.\n"
"          For MCP23017 IEC base identifiers the number cng defines the\n"
"          complete address of a particular MCP23017 GPIO register:\n"
"           c  is the channel 1 to 8 for /dev/i2c-11 to /dev/i2c-18\n"
"           n  is the address 0 to 7 of a particular MCP23017 for the\n"
"              address range 0x20 to 0x27 selected with A0 to A2\n"
"           g  selects the GPIO register\n"
"              0 2 4 6 8 is GPIOA\n"
"              1 3 5 7 9 is GPIOB\n"
"              normally only g = 0 and 1 is used for GPIOA and GPIOB,\n"
"              but if a second instance of iCpiI2C is used on another\n"
"              Raspberry Pi, but connected to the one iCsever the\n"
"              alternate numbers differentiate between IEC variables\n"
"              for the two RPi's.  Second or more instances of iCpiI2C\n"
"              must be called with the option -o 2, -o 4 etc for\n"
"              using 2 and 3 or 4 and 5 etc to select GPIOA and GPIOB.\n"
"          Both registers GPIOA and GPIOB can have input bits and\n"
"          output bits, but any input bit cannot also be an output bit\n"
"          or vice versa.  Each input or output IEC variable (or list\n"
"          of variables) may be followed by a comma separated number,\n"
"          interpreted as a mask, declaring which bits in the base IEC\n"
"          variable byte are inputs for an input variable or outputs\n"
"          for an output variable. The mask is best written as a binary\n"
"          number eg 0b10001111 for bits 0, 1, 2, 3 and 7.  If no mask\n"
"          is supplied the default mask is 0b11111111 - all 8 bits set.\n"
"    IXcng[,IXcng,...][,<mask>]   list of input variables with the same mask\n"
"    QXcng[,QXcng,...][,<mask>]   list of output variables with the same mask\n"
"    IXcng-IXcng[,<mask>]         range of input variables with the same mask\n"
"    QXcng-QXcng[,<mask>]         range of output variables with the same mask\n"
"         IEC lists and ranges must contain either all inputs or all outputs.\n"
"         The only sensible range is in the digit n for the MCP23017 address.\n"
"\n"
"                      GPIO arguments\n"
"	  Any IEC I/Os IXn.y and QXn.y which are to be linked to GPIO\n"
"	  inputs or outputs must be named in the argument list. The\n"
"	  number n in the IEC must be one or at most two digits to\n"
"	  differentiate GPIO IEC's from three digit MCP23017 IEC's.\n"
"    IXn,<gpio>[,<gpio>,...]\n"
"    QXn,<gpio>[,<gpio>,...]\n"
"          Associate the bits of a particular input or output IEC\n"
"          with a list of gpio numbers.  Up to 8 gpio numbers can\n"
"          be given in a comma separated list. The first gpio number\n"
"          will be associated with bit 0, the second with bit 1 etc\n"
"          and the eighth with bit 7. If the list is shorter than\n"
"          8 the trailing bits are not used. The letter 'd' can be\n"
"          used in the list instead of a gpio number to mark that bit\n"
"          as unused.  eg. IX3,17,18,19,20,d,d,21 associates IX3.0 to\n"
"          IX3.3 with GPIO 17 to GPIO 20 and IX3.6 with GPIO 21.\n"
"          Alternatively:\n"
"    IXn.<bit>,<gpio>\n"
"    QXn.<bit>,<gpio>\n"
"          Each input or output <bit> 0 to 7 is associated with one\n"
"          gpio nunber eg. IX3.0,17 QX3.7,18\n"
"\n"
"                      COMMON extensions\n"
"    ~IEC  Each leading IEC of a list or range may be preceded by a\n"
"          tilde '~', which inverts all IEC inputs or outputs of a\n"
"          list or range.  If all IEC inputs and outputs are already\n"
"          inverted with -I, then ~ inverts them again, which makes them\n"
"          non-inverted. Individual bits for one IEC can be inverted\n"
"          or left normal for GPIO IEC's by declaring the IEC more than\n"
"          once with non-overlapping gpio bit lists or single bits. Also\n"
"          possible for MCP23017 I/O with two non overlapping masks -\n"
"          one inverting, the other non-inverting.\n"
"    IEC-inst Each individual IEC or range can be followed by -<inst>,\n"
"          where <inst> consists of 1 to %d numeric digits.\n"
"          Such an appended instance code takes precedence over the\n"
"          general instance specified with the -i <inst> option above.\n"
"    For a full Description and Technical Background of the MCP23017 I2C\n"
"    and GPIO driver software see the iCpiI2C man page\n"
"\n"
"                      CAVEAT\n"
"    There must be at least 1 MCP23017 or GPIO IEC argument. No IEC\n"
"    arguments are generated automatically for iC[iI2C.\n"
"    Only one instance of iCpiI2C or an app with real IEC parameters\n"
"    like iCpiFace may be run on one RPi and all GPIOs and MCP23017s\n"
"    must be controlled by this one instance. If two instances were\n"
"    running, the common interrupts would clash. Also no other program\n"
"    controlling GPIOs and MCP23017s like 'MCP23017 Digital Emulator'\n"
"    may be run at the same time as this application.  An exception\n"
"    is iCpiPWM which controls GPIOs by DMA and not by interrupts.\n"
"    Another exception is iCtherm which controls GPIO 4 by the 1Wire\n"
"    interface.  Care is taken that any GPIOs or MCP23017s used in one\n"
"    app, iCpiI2C, iCpiPWM or even iCtherm do not clash with another\n"
"    app (using ~/.iC/gpios.used).\n"
"\n"
"                      DEBUG options\n"
#if YYDEBUG && !defined(_WINDOWS)
"    -t      trace arguments and activity (equivalent to -d100)\n"
"         t  at run time toggles gate activity trace\n"
"    -m      display microsecond timing info\n"
"         m  at run time toggles microsecond timing trace\n"
#endif	/* YYDEBUG && !defined(_WINDOWS) */
"    -d deb  +1   trace TCP/IP send actions\n"
"            +2   trace TCP/IP rcv  actions\n"
#ifdef	TRACE
"            +4   trace MCP calls\n"
#endif	/* TRACE */
"            +10  trace I2C  input  actions\n"
"            +20  trace I2C  output actions\n"
#ifdef	TRACE
"            +40  debug MCP calls\n"
#endif	/* TRACE */
"            +100 show arguments\n"
"            +200 show more debugging details\n"
"            +400 exit after initialisation\n"
"    -q      quiet - do not report clients connecting and disconnecting\n"
"    -z      block keyboard input on this app - used by -R\n"
"    -h      this help text\n"
"         T  at run time displays registrations and equivalences in iCserver\n"
"         q  or ctrl+D  at run time stops iCpiI2C\n"
"\n"
"                      AUXILIARY app\n"
"    -R <app ...> run auxiliary app followed by -z and its arguments\n"
"                 as a separate process; -R ... must be last arguments.\n"
"\n"
"Copyright (C) 2023 John E. Wulff     <immediateC@gmail.com>\n"
"Version	$Id: iCpiI2C.c 1.1 $\n"
;

char *		iC_progname;		/* name of this executable */
static char *	iC_path;
short		iC_debug = 0;
#if YYDEBUG && !defined(_WINDOWS)
int		iC_micro = 0;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
unsigned short	iC_osc_lim = 1;

/********************************************************************
 *
 *  mcpIO array mcpL[unit] is organised as follows:
 *    [unit] is the MCP23017 unit index in address order
 *    each element of the array is a struct mcpIO
 *
 *  A single MCP23017 can be addressed from 0 - 3 with fixed wiring or
 *  jumpers to address pins A0 - A2.
 *
 *******************************************************************/

static int	unit = 0;
static unsigned short	topChannel = 0;
static char	buffer[BS];
static char	SR[] = "RSR";		/* generates RQ for iq 0, SI for iq 1, RQ for iq 2 */
static char	IQ[] = "QIQ";
static short	errorFlag = 0;
static chS *	Units = NULL;		/* dynamic array to store MCP23017 unit numbers indexed by channel or GPIOl */
static int	ioUnits = 0;		/* dynamically allocated size of Units[] */
static long long ownUsed = 0LL;
char		iC_stdinBuf[REPLY];	/* store a line of STDIN - used by MCP23017CAD input */
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
    int			fd;
    int			len;
    int			value;
    int			sig;
    int			n;
    int			c;
    int			m;
    int			mcpCnt;
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
    char		iqs[2] = { '\0', '\0' };
    char		iqe[2] = { '\0', '\0' };
    char		cng[4] = { '\0', '\0', '\0', '\0' };
    char		dum[2] = { '\0', '\0' };
    int			iqStart;
    int			iqEnd;
    int			ieStart = 0;
    int			ieEnd   = 0;
    unsigned short	bit, bitStart, bitEnd;
    unsigned short	gpio;
    unsigned short	directFlag;		/* or MC for MCP23017, GP for GPIO, DR direct for both */
    #define		MC	1
    #define		GP	2
    #define		DR	4
    gpioIO *		gep;
    gpioIO **		gpioLast;
    unsigned short	gpioSav[8];
    unsigned short	u;
    int			mask;
    int			b;
    int			ch;
    int			ns;
    int			gs;
    int			cn;
    int			che;
    int			nse;
    int			gse;
    int			gpioCnt = 0;
    int			invMask;
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
     *  By default do not invert MCP23017 and GPIO inputs and outputs.
     *  Both are inverted with -I for all inputs and outputs and '~' for
     *  individual inputs or outputs.
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
		case 'o':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    if (strspn(*argv, "02468") != 1) {
			fprintf(iC_errFP, "ERROR: %s: '-o %s' is not a proper offset (0 2 4 6 8)\n",
			    iC_progname, *argv);
			errorFlag++;
		    } else {
			offset = **argv - '0';	/* offset for MCP23017 GPIOA/GPIOB selection */
		    }
		    goto break2;
		case 'G':
		    iC_opt_G = 1;	/* GPIO direct action only - block MCP23017 IO, even if MCP23017s present */
		    break;
		case 'B':
		    iC_opt_B = 1;	/* start iCbox -d to monitor active MCP23017 and/or GPIO I/O */
		    break;
		case 'I':
		    invMask = 0xff;	/* invert MCP23017 and GPIO inputs and outputs with -I */
		    iC_opt_P = 1;	/* do not invert MCP23017 outputs with -I (inverted a second time) */
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
		    forceFlag = 1;	/* force use of GPIO interrupts - in particular GPIO 27 */
		    break;
#if YYDEBUG && !defined(_WINDOWS)
		case 't':
		    iC_debug |= 0100;	/* trace arguments and activity */
		    break;
		case 'm':
		    iC_micro++;		/* microsecond info */
		    break;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
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
		    fprintf(iC_errFP, usage, iC_hostNM, iC_portNM, INSTSIZE, INSTSIZE);
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
	     *  been determined, in particlar -G, and after MCP23017s have
	     *  been counted. IEC arguments and options may be mixed (except -R).
	     *  IEC arguments must start with I ~I Q or ~Q.
	     *  A special case is a lone argument ~, which is converted to the HOME
	     *  environment variable by the Linux shell. It is converted back to ~ here.
	     *******************************************************************/
	    iC_opt_P = 1;	/* using an IEC argument is equivalent to -P option */
	    if (strspn(*argv, "~IQ") == 0) {
		if (strcmp(*argv, getenv("HOME")) == 0) {
		    argip[argic++] = "~";	/* transmogrify shell translation of HOME to "~" */
		    continue;			/* get next command line argument */
		}
		fprintf(stderr, "ERROR: %s: invalid argument '%s' (IEC variables start with I ~I Q ~Q)\n", iC_progname, *argv);
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
	 *  gpios.proc is the int returned by boardrev() // Raspberry Pi Board revision
	 *
	 *  gpios.valid has 64 bits - a bit is set for each valid GPIO
	 *
	 *        .... 3333 3322 2222 2222 1111 1111 11
	 *  GPIO  .... 5432 1098 7654 3210 9876 5432 1098 7654 3210
	 *  valid .... ---- ---- 1111 1111 1111 1111 1111 1111 11--
	 *        ....    0    0    f    f    f    f    f    f    c
	 *
	 *  gpios.used has 64 bits - a bit is set for each GPIO used in other apps
	 *  If -f (forceFlag) is set the used word is cleared unconditionally
	 *  Else only GPIO's not used in other apps can be used in this app.
	 *
	 *  For an RPi B+ processor it contains:
	 *  	gpios.proc	1
	 *  	gpios.valid	0x00000000fbc6cf9c
	 *  	gpios.used	0x0000000000000000
	 *
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
	     *
	     *  Identify and count MCP23017s available.
	     *
	     *  Do this before checking whether GPIO 27 is not used by other apps.
	     *  GPIO 27 is used during I2C and MCB23017  setup, but is left unchanged
	     * if no MCP23017s is found.
	     *
	     *  If no MCP23017s are found proceed with opt_G set. An error will then occur
	     *  only if an argument requesting MCP23017 IO (Innn or Qnnn) is found.
	     *
	     *  Initialisation of /dev/i2cdev0.0 and/or /dev/i2cdev0.1 for I2C
	     *  modes and read/writeBytes
	     *
	     *  Test for the presence of a MCP23017 by writing to the IOCON register and
	     *  checking if the value returned is the same. If not, there is no MCP23017
	     *  at that address. Do this for all 8 mux channels and 8 MCP's each channel.
	     *
	     *  Initialise all MCP23017s found by writing to all relevant MCB23017
	     *  registers because there may be input interrupts.
	     *
	     *******************************************************************/
	    value = mcpCnt = 0;
	    for (ch = 0; ch < 8; ch++) {		/* scan /dev/ic2-11 to /dev/i2c-18 */
		if ((fd = setupI2C(ch+1)) < 0) {
		    fprintf(stdout, "ERROR: %s: failed to init I2C communication on channel %d: %s\n",
			iC_progname, 11+ch, strerror(errno));
		    exit(1);
		}
		i2cFdCnt++;				/* flags that at least one I2C channel is valid */
		m = 0;
		for (ns = 0; ns < 8; ns++) {
		    /********************************************************************
		     *  Detect MCP23017's
		     *******************************************************************/
		    cn = (ch << 3) | ns;
		    if (setupMCP23017(fd, detect, 0x20+ns, IOCON_ODR, 0x00, 0x00) < 0) {
			mcpL[cn] = (void *) -1;
//	printf("ch = %d ns = %d cn = %02o\n", ch, ns, cn);
			continue;			/* no MCP23017 at this address */
		    }
//		    mcpL[cn] = NULL;
		    m++;				/* count active MCP23017s in this I2C channel */
		    mcpCnt++;				/* count all active MCP23017s */
		}
		if (m == 0) {
		    close(fd);				/* no active MCP23017s in this I2C channel */
		    fd = -1;
		}
		i2cFdA[ch] = fd;			/* save the file descriptor for this I2C channel */
	    }
	    if (mcpCnt) {
		/********************************************************************
		 *  MCP23017s found
		 *  Check if GPIO 27 required by MCP23017 has been used in another app
		 *******************************************************************/
		gpioMask = 0x0800000cLL;		/* gpio 2,3,27 */
		assert((gpiosp->valid & gpioMask) == gpioMask);	/* assume these are valid for all RPi's */
		if (iC_debug & 0200) fprintf(iC_outFP,
		    "gpio     = 27\n"
		    "used     = 0x%016llx\n"
		    "gpioMask = 0x%016llx\n",
		    gpiosp->u.used, gpioMask);
		if ((gpioMask & gpiosp->u.used)) {
		    gpioMask &= gpiosp->u.used;
		    fprintf(iC_errFP, "ERROR: %s: The following GPIO's required by iCpiI2C have been used in another app\n",
			iC_progname);
		    for (b = 2; b <= 27; b++) {
			if ((gpioMask & (1LL << b))) {
			    fprintf(iC_errFP, "		GPIO %d\n", b);
			}
		    }
		    fprintf(iC_errFP,
			"	Cannot run MCP23017s correctly - also stop apps using these GPIOs,\n"
			"	because they will be interfered with by MCP23017 hardware\n");
		    errorFlag++;
		    goto UnlockGpios;
		}
		/********************************************************************
		 *  Free to use MCP23017s
		 *  Block GPIOs 2,3,27 required by MCP23017 hardware
		 *******************************************************************/
		gpiosp->u.used |= gpioMask;		/* mark gpio used for ~/.iC/gpios.used */
		ownUsed        |= gpioMask;		/* mark gpio in ownUsed for termination */
		/********************************************************************
		 *  Initialisation using the /sys/class/gpio interface to the GPIO
		 *  systems - slightly slower, but always usable as a non-root user
		 *  export GPIO 27, which is INTB of the MCB23017 device.
		 *
		 *  Wait for Interrupts on GPIO 27 which has been exported and then
		 *  opening /sys/class/gpio/gpio27/value
		 *
		 *  This is actually done via the /sys/class/gpio interface regardless of
		 *  the wiringPi access mode in-use. Maybe sometime it might get a better
		 *  way for a bit more efficiency.	(comment by Gordon Henderson)
		 *
		 *  Set the gpio27/INTB for falling edge interrupts
		 *******************************************************************/
printf("### test gpio_export 27\n");
		if ((sig = gpio_export(27, "in", "falling", forceFlag, iC_fullProgname)) != 0) {
		    iC_quit(sig);			/* another program is using gpio 27 */
		}
		/********************************************************************
		 *  open /sys/class/gpio/gpio27/value permanently for obtaining
		 *  out-of-band interrupts
		 *******************************************************************/
		if ((gpio27FN = gpio_fd_open(27, O_RDONLY)) < 0) {
		    fprintf(iC_errFP, "ERROR: %s: MCP23017 open gpio 27: %s\n", iC_progname, strerror(errno));
		}
		if (gpio27FN > iC_maxFN) {
		    iC_maxFN = gpio27FN;
		}
		/********************************************************************
		 *  read the initial /sys/class/gpio/gpio27/value
		 *******************************************************************/
		if ((value = gpio_read(gpio27FN)) == -1) {
		    fprintf(iC_errFP, "ERROR: %s: MCP23017 read gpio 27: %s\n", iC_progname, strerror(errno));
		}
		if (iC_debug & 0200) fprintf(iC_outFP, "Initial read 27 = %d\n", value);
		/********************************************************************
		 *  The INTA output of the interrupt concntrator MCP23017 is configured
		 *  as active high output and does not need a pullup resistor at gpio 27.
		 *******************************************************************/
	    } else {
		iC_opt_G = 1;		/* no MCP23017s - block all MCP23017 command line arguments */
	    }
	}
	/********************************************************************
	 *  Match MCP23017s and GPIOs to IEC arguments in the command line.
	 *******************************************************************/
	directFlag = 0;
	for (argp = argip; argp < &argip[argic]; argp++) {
	    directFlag &= DR;				/* clear all bits except DR for each new argument */
	    if (strlen(*argp) >= 128) {
		fprintf(iC_errFP, "ERROR: %s: command line argument too long: '%s'\n", iC_progname, *argp);
		exit(1);
	    }
	    iid = iidN;					/* global instance either 0xffff for "" or 0 - 999 */
	    *iids = '\0';				/* track argument instance for error messages */
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
	    mask = 0xff;				/* default is all bits of input or output active */
	    /********************************************************************
	     *  Scan for a mandatory first IXn or QXn starting IEC argument
	     *******************************************************************/
	    switch (scanIEC("%1[IQ]X%5[0-9]%127s", iqs, iqe, cng, &ieStart, &ieEnd, tail)) {
		case 1:
		    /********************************************************************
		     *  check for ,<mask>
		     *******************************************************************/
		  checkMask:
		    if ((n = sscanf(tail, ",%16[0-9xa-fXA-F]%126s", buffer, tail)) > 0) {
			char * endptr;
			mask = convert_nr(buffer, &endptr);
			if (mask <= 0 || mask > 0xff || *endptr != '\0') {
			    fprintf(iC_errFP, "ERROR: %s: '%s' mask 0x%02x is too small or too large\n", iC_progname, *argp, mask);
			    errorFlag++;
			    goto endOneArg;
			}
			if (n == 1) tail[0] = '\0';
    printf("tail = '%s'\n", tail);
			if (tail[0] == '-') goto checkInst;
		    } else {
			memmove(tail, tail+1, 126);	/* mask == 0b11111111 by default TODO check */
		    }
		    // TODO distribute mask in the MCP23017 list
		    /* fall through */
		case 2:
		    /********************************************************************
		     *  check option -G is not used when MCP23017 IECs have been seen
		     *******************************************************************/
		  checkOptionG:
		    if (ieStart && iC_opt_G) {
			fprintf(iC_errFP, "ERROR: %s: '%s' no MCP23017 IEC arguments allowed with option -G\n", iC_progname, *argp);
			errorFlag++;
			goto endOneArg;
		    }
		    if (iC_debug & 0200) fprintf(iC_outFP, "%s%sX%d-%sX%d,0x%02x%s\n",
			invFlag ? "~" : "", iqs, ieStart, iqs, ieEnd, mask, iids);	/* TODO print list */
		    directFlag |= MC | DR;
		    break;
		case 3:
		    /********************************************************************
		     *  GPIO IEC arguments with ,gpio,gpio,... or .<bit>,gpio
		     *******************************************************************/
		  processGPIO:
		    iqStart = (*iqe == 'I') ? 1 : 0;	/* 1 for 'IX', 0 for 'QX' */
		    assert((iqStart & -2) == 0);	/* iqStart may be 0 and 1 only */
		    switch (tail[0]) {		/* test for mask or instance */
			case '.':
			    if (sscanf(tail, ".%hu%127s", &bit, tail) != 2 || bit >= 8 || tail[0] == '-') {
				goto illFormed;	/* must have tail for gpio number */
			    }
			    bitStart = bitEnd = bit;	/* only 1 bit processed */
			    break;
			case ',':
			    bitStart = 0;
			    bitEnd   = 7;		/* process all 8 bits */
			    break;
			default:
			    fprintf(iC_errFP, "ERROR: %s: '%s' GPIO IEC variable without gpio number\n", iC_progname, *argp);
			    errorFlag++;
			    goto endOneArg;
		    }
		    for (bit = 0; bit <= 7; bit++) {
			gpioSav[bit] = 0xffff;	/* initialise empty Save list */
		    }
		    for (bit = bitStart, n = 2; bit <= bitEnd && n == 2; bit++) {
			if (tail[0] == '-') goto checkInst;
			*dum = '\0';
			gpio = 0;
			if (((n = sscanf(tail, ",%hu%127s", &gpio, tail)) < 1 &&
			    (n = sscanf(tail, ",%1[d]%127s", dum, tail)) < 1) ||
			    gpio > 55) {
			    goto illFormed;
			}
			if (n == 1) tail[0] = '\0';
			directFlag |= GP | DR;
			if (*dum == '\0') {
			    /********************************************************************
			     *  TODO Check later that a GPIO is not used twice for different IEC's.
			     *  Also check later that a bit in an IEC of a particular instance
			     *  is not linked with a GPIO more than once.
			     *******************************************************************/
			    gpioSav[bit] = gpio;	/*  Save list of GPIO numbers for later building */
			    if (iC_debug & 0200) fprintf(iC_outFP, "%s%sX%d.%hu	gpio = %hu\n",
				invFlag ? "~" : "", iqe, ieEnd, bit, gpio);
			}
		    }
		    if (n == 2 && tail[0] == ',') {
			fprintf(iC_errFP, "ERROR: %s: '%s' gpio list limited to 8 comma separated numbers or 1 if .bit\n", iC_progname, *argp);
			errorFlag++;
			goto endOneArg;
		    }
		    /* fall through */
		case 4:
		    /********************************************************************
		     *  check instance -0 to -999
		     *******************************************************************/
		  checkInst:
		    if (tail[0] == '-') {
			if (n != 1 && (n = sscanf(tail, "-%3hu%127s", &iid, tail)) == 1) {	/* individual instance has precedence */
			    snprintf(iids, 6, "-%hu", iid);	/* instance "-0" to "-999" */
			} else {
			    goto illFormed;
			}
		    } else if (tail[0] != '\0') {
			goto illFormed;			/* a tail other than -instance eg -999 */
		    }
	    printf("### iids = '%s'\n", iids);
		    if (ieEnd > 99) {
			goto checkOptionG;		/* MCP23017 IEC variable */
		    }
		    break;
		case 5:
		    /********************************************************************
		     *  ill formed argument
		     *******************************************************************/
		  illFormed:
		    fprintf(iC_errFP, "ERROR: %s: '%s' is not a well formed IEC, IEC list or IEC range\n", iC_progname, *argp);
		    errorFlag++;
		    /* fall through */
		case 6: goto endOneArg;
	    }
#if 0 /* ################################################################## */
	    /********************************************************************
	     *  Use the IEC argument to extend MCP23017 or GPIO control structures
	     *  Check IEC argument was not used before
	     *******************************************************************/
	    if (directFlag & MC) {
		assert((directFlag & GP) == 0);		/* should not have both */
		int		ie;
		/********************************************************************
		 *  Build one or more MCP23017 control structures iC_pfL[un]
		 *      ieStart - ieEnd covers range of MCP23017 IEC argument (-1 for Dummy)
		 *      iqStart - iqEnd 0 = 'I', 1 = 'Q'
		 *  temporarily store ieStart (IEC number) in val of iqDetails
		 *  temporarily store iid (instance) in channel of iqDetails
		 *  pfa is 0 - 7 or 0xffff if not yet defined - select a iC_pfL[unit]
		 *******************************************************************/
		for (ie = ieStart; ie <= ieEnd; ie++) {
		    int		ie1;
		    for (iq = iqStart; iq <= iqEnd; iq++) {
			int	em;
			for (un = 0; un < mcpCnt; un++) {
			    if ((ie1 = iC_pfL[un].s[iq].val) == ie &&
				ie >= 0 &&	/* can have any number of dummies on same pfa selection */
				iC_pfL[un].s[iq].channel == iid) {
				fprintf(iC_errFP, "ERROR: %s: MCP23017 IEC %cX%d%s used more than once\n",
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
				fprintf(iC_errFP, "ERROR: %s: MCP23017 %hu used more than once with %cX%d%s and %cX%d%s\n",
				    iC_progname, pfa, IQ[iq], ie1, iids, IQ[iq], ie, iids);
				errorFlag++;
				goto endOneArg;
			    }
			}
			if (pfa == 0xffff) {
			    do {
				if ((unit = unitA[iq]++) >= mcpCnt) {	/* maximum unassigned unit found so far */
				    fprintf(iC_errFP, "ERROR: %s: '%d is too many MCP23017 IEC arguments (%d max)\n", iC_progname, unit+1, mcpCnt);
				    errorFlag++;
				    goto endOneArg;
				}
			    } while (iC_pfL[unit].s[iq].val >= 0);	/* use the next unused pfa entry */
			} else {
			    fprintf(iC_errFP, "ERROR: %s: MCP23017 %hu named in %cX%d:%hu%s is not available\n",
				iC_progname, pfa, IQ[iq], ie, pfa, iids);
			    errorFlag++;
			    goto endOneArg;
			}
		      pfaFound:
			for (gep = iC_gpL[iq & 0x01]; gep; gep = gep->nextIO) {	/* check IEC not used for GPIO */
			    if (gep->Gval == ie &&
				gep->Gchannel == iid) {
				fprintf(iC_errFP, "ERROR: %s: MCP23017 IEC %cX%d:%hu%s has already been used as a GPIO IEC\n",
				    iC_progname, IQ[iq], ie, pfa, iids);
				errorFlag++;
				goto endOneArg;
			    }
			}
			pfp = &iC_pfL[unit];
			pfq = &(pfp->s[iq]);
			pfq->inv = invMask ^ invFlag;	/* by default do not invert MCP23017 inputs and Port B outputs */
			if (iq == 0) {
			    pfq->inv ^= 0xff;		/* by default invert MCP23017 Port A outputs - inverted again with -I or '~' */
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
	    } else
#endif /* ################################################################## */
	    if (directFlag & GP) {
		/********************************************************************
		 *  Build or extend a new gpioIO structure and store in iC_gpL[iqStart]
		 *      iqStart 0 = 'Q', 1 = 'I'
		 *  temporarily store ieEnd (IEC number) in val of iqDetails
		 *  temporarily store iid (instance) in channel of iqDetails
		 *  Check that the GPIO for the IEC argument is valid for this RPi
		 *  and was not used before in this or another app.
		 *******************************************************************/
		gpioLast = &iC_gpL[iqStart];	/* scan previous gpio list */
		for (gep = iC_gpL[iqStart]; gep; gep = gep->nextIO) {
		    if (gep->Gval == ieEnd &&
			gep->Gchannel == iid) {
			goto previousElement;	/* already partially filled */
		    }
		    gpioLast = &gep->nextIO;
		}
#if 0 /* ################################################################## */
		for (un = 0; un < mcpCnt; un++) {	/* check IEC not used for MCP23017 */
		    if (iC_pfL[un].s[iqStart].val == ieEnd &&
			iC_pfL[un].s[iqStart].channel == iid) {
			fprintf(iC_errFP, "ERROR: %s: GPIO IEC %cX%d%s has already been used as a MCP23017 IEC\n",
			    iC_progname, IQ[iqStart], ieEnd, iids);
			errorFlag++;
			goto endOneArg;
		    }
		}
#endif /* ################################################################## */
		*gpioLast = gep = iC_emalloc(sizeof(gpioIO));	/* new gpioIO element */
		gep->Gname = NULL;		/* GPIO IEC in or output not active */
		gep->Gval = ieEnd;
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
				invFlag & mask ? '~' : ' ', iqs, ieEnd, bit, gpio, iids,
				gep->Ginv & mask ? '~' : ' ', iqs, ieEnd, bit, gep->gpioNr[bit], iids);
			    errorFlag++;
			} else if (!(gpiosp->valid & gpioMask)) {
			    fprintf(iC_errFP, "ERROR: %s: trying to use invalid gpio %hu on %sX%d.%hu%s on RPi board rev %d\n",
				iC_progname, gpio, iqs, ieEnd, bit, iids, gpiosp->proc);
			    errorFlag++;
			} else if (gpiosp->u.used & gpioMask) {
			    fprintf(iC_errFP, "ERROR: %s: trying to use gpio %hu a 2nd time on %c%sX%d.%hu%s\n",
				iC_progname, gpio, invFlag & mask ? '~' : ' ', iqs, ieEnd, bit, iids);
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
	fprintf(iC_errFP, "ERROR: %s: no IEC arguments? there must be at least 1 MCP23017 or GPIO argument\n", iC_progname);
	iC_quit(-4);					/* call termQuit() to terminate I/O */
    }
#if 0 /* ################################################################## */
    /********************************************************************
     *  Generate IEC names for all arguments which have a MCP23017 and a
     *	non-zero bmask. MCP23017 arguments with zero bmask are not registered.
     *  Do this here because instance was not yet determind when IEC name
     *  was found
     *******************************************************************/
    unit = max(max(unitA[0], unitA[1]), unitA[2]);
    for (pfp = iC_pfL; pfp < &iC_pfL[mcpCnt]; pfp++) {
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
#endif /* ################################################################## */
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
#if 0 /* ################################################################## */
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
#endif /* ################################################################## */
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
#if 0 /* ################################################################## */
    for (pfp = iC_pfL; pfp < &iC_pfL[mcpCnt]; pfp++) {	/* TODO may not be needed */
	if (pfp->intf == 0) {
	    fprintf(iC_errFP, "ERROR: %s: MCP23017 %d address %hu: was requested in the call but was not found\n",
		iC_progname, pfp - iC_pfL, pfp->pfa);
	    iC_quit(-4);				/* call termQuit() to terminate I/O */
	}
    }
    /********************************************************************
     *  Adjust sizes if number of IEC arguments is incorrect
     *******************************************************************/
    if (mcpCnt < unit) {
	fprintf(iC_errFP, "WARNING: %s: too many IEC arguments (%d) for the %d MCP23017s found - ignore extra IEC's\n",
	    iC_progname, unit, mcpCnt);		/* takes account of dummies */
    } else if (mcpCnt > unit) {
	fprintf(iC_errFP, "WARNING: %s: not enough IEC arguments (%d) for the %d MCP23017s found - ignore extra MCP23017s\n",
	    iC_progname, unit, mcpCnt);		/* takes account of dummies */
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "### unit = %d mcpCnt = %d\n", unit, iC_npf);
    unit = mcpCnt;					/* deal only with real MCP23017s */
    if (unit == 0 && gpioCnt == 0) {
	fprintf(iC_errFP, "ERROR: %s: no MCP23017s or gpio's to run\n", iC_progname);
	iC_quit(-2);
    }
    /********************************************************************
     *  End of MCP23017 detection
     *******************************************************************/
    for (pfp = iC_pfL; pfp < &iC_pfL[mcpCnt]; pfp++) {
	if (pfp->Qname || pfp->Iname || pfp->QPname) {
	    char *	piFext;
	    uint8_t	inputA;
	    uint8_t	inputB;
	    if (pfp->intf == INTFA) {
		piFext = "CAD";
		inputA = 0xff;		/* MCP23017CAD all bits input on Port A */
		inputB = 0x00;		/* MCP23017CAD has no input on Port B */
	    } else {
		piFext = "   ";
		inputA = 0x00;		/* MCP23017 has no input on Port A */
		inputB = pfp->Ibmask;	/* MCP23017 input on port B, may be 0 if all output */
	    }
	    if (iC_debug & 0200) fprintf(iC_outFP, "###	MCP23017%s un = %d pfa = %hu	%s	%s	%s\n",
		piFext, pfp - iC_pfL, pfp->pfa, pfp->Qname, pfp->Iname, pfp->QPname);
	    /********************************************************************
	     *  Final setup with correct IOCON_ODR for all MCP23017 Units with I/O.
	     *  Active driver output if only one MCP23017 else open drain
	     *******************************************************************/
	    if (setupMCB23017(pfp->i2cFN, pfp->pfa, (mcpCnt == 1) ? 0x00 : IOCON_ODR, 1, inputA, inputB) < 0) {
		fprintf(iC_errFP, "ERROR: %s: MCP23017%s %hu not found after succesful test ???\n",
		    iC_progname, piFext, pfp->pfa);
	    }
	}
    }
#endif /* ################################################################## */
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
#if 0 /* ################################################################## */
    for (pfp = iC_pfL; pfp < &iC_pfL[mcpCnt]; pfp++) {
	if (pfp->Iname || pfp->Qname) {
	    len += snprintf(buffer+len, BS-len, "_%hu", pfp->pfa);	/* name MCP23017 pfa */
	}
    }
#endif /* ################################################################## */
    if (gpioCnt) {
	len += snprintf(buffer+len, BS-len, "_G");		/* name GPIO marker */
    }
    if (*iC_iidNM) {
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
#if 0 /* ################################################################## */
    if (!iC_opt_G) {
	/********************************************************************
	 *  Generate registration string made up of all active MCP23017 I/O names
	 *  There is either 1 input or 1 output name or both per active MCP23017
	 *******************************************************************/
	for (pfp = iC_pfL; pfp < &iC_pfL[mcpCnt]; pfp++) {
	    assert(regBufLen > ENTRYSZ);	/* accomodates ",SIX123456,RQX123456,Z" */
	    for (iq = 0; iq < 3; iq++) {
		if ((np = pfp->s[iq].i.name) != NULL) {
		    assert(*np == IQ[iq]);
		    len = snprintf(cp, regBufLen, ",%c%s", SR[iq], np);
		    cp += len;			/* input send name or output receive name */
		    regBufLen -= len;
		}
	    }
	}
    }
#endif /* ################################################################## */
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
#if YYDEBUG && !defined(_WINDOWS)
    if (iC_micro) iC_microPrint("send registration", 0);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
    iC_send_msg_to_server(iC_sockFN, regBuf);		/* register IOs with iCserver */
    /********************************************************************
     *  Receive a channel number for each I/O name sent in registration
     *  Distribute read and/or write channels returned in acknowledgment
     *******************************************************************/
    if (iC_rcvd_msg_from_server(iC_sockFN, rpyBuf, REPLY) != 0) {	/* busy wait for acknowledgment reply */
	chS	sel;				/* can store either NULL or gpioIO* and un/mask */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_micro) iC_microPrint("ack received", 0);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	if (iC_debug & 0200) fprintf(iC_outFP, "reply:%s\n", rpyBuf);
	if (iC_opt_B) {				/* prepare to execute iCbox -d */
	    len = snprintf(buffer, BS, "iCbox -dz%s -n %s-DI", (iC_debug & DQ) ? "q" : "", iC_iccNM);
	    b = 4;				/* initial number of tokens in buffer */
	    op = buffer + len;			/* optional host and port appended in iC_fork_and_exec() */
	    ol = BS - len;
	}
	cp = rpyBuf - 1;	/* increment to first character in rpyBuf in first use of cp */
#if 0 /* ################################################################## */
	/********************************************************************
	 *  Channels for MCP23017 acknowledgments
	 *******************************************************************/
	for (pfp = iC_pfL; pfp < &iC_pfL[mcpCnt]; pfp++) {
	    for (iq = 0; iq < 3; iq++) {
		pfq = &(pfp->s[iq]);
		if ((np = pfq->i.name) != NULL) {
		    assert(cp);			/* not enough channels in ACK string */
		    channel = atoi(++cp);	/* read channel from ACK string */
		    assert(channel > 0);
		    if (channel > topChannel) {
			topChannel = channel;
		    }
		    if (iC_debug & 0200) fprintf(iC_outFP, "MCP23017 %d %s  on channel %hu\n",
			pfp->pfa, np, channel);
		    pfq->channel = channel;	/* link send channel to MCP23017 (ignore receive channel) */
		    sel.un = (pfp - iC_pfL) | (iq & 0x02) << 2;	/* iC_pfL index un identifies MCP23017 GPIOB+ */
		    storeUnit(channel, sel);		/* link MCP23017 unit number to send channel */
		    if (iC_opt_B) {
			assert(ol > 10);
			len = snprintf(op, ol, " %s", np);	/* add I/O name to execute iCbox -d */
			b++;				/* count tokens in buffer */
			op += len;
			ol -= len;
			if ((mask = pfq->bmask) != 0xff) {
			    len = snprintf(op, ol, ",%d", mask);	/* mask out any MCP23017 bits not used */
			    op += len;
			    ol -= len;
			}
		    }
		    cp = strchr(cp, ',');
		}
	    }
	}
#endif /* ################################################################## */
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
#if 0 /* ################################################################## */
	if (mcpCnt) {
	    fprintf(iC_outFP, "Allocation for %d MCP23017 unit%s, global instance = \"%s\"\n",
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
	    }
	    fprintf(iC_outFP, "\n");
	}
#endif /* ################################################################## */
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
#if 0 /* ################################################################## */
    /********************************************************************
     *  Generate MCP23017 initialisation inputs and outputs for active MCP23017s
     *  Send possible TCP/IP after GPIO done
     *******************************************************************/
    if (!iC_opt_G) {
	assert(gpio27FN > 0);
	if (iC_debug & 0200) fprintf(iC_outFP, "### Initialise %d unit(s)\n", mcpCnt);
	gpio_read(gpio27FN);			/* dummy read to clear interrupt on /dev/class/gpio27/value */
#if YYDEBUG && !defined(_WINDOWS)
	if (iC_micro) iC_microPrint("I2C initialise", 0);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
	for (pfp = iC_pfL; pfp < &iC_pfL[mcpCnt]; pfp++) {
	    if (((pfa = pfp->pfa) != 4 || pfp->intf != INTFA) && pfp->Qname) {	/* MCP23017 */
		writeByte(pfp->i2cFN, pfa, OLATA, pfp->Qinv);	/* normally write inverted data to MCP23017 A output */
		pfp->Qval = 0;			/* force all output bits (TODO not used) */
	    }
	    if (pfp->Iname) {
		assert(regBufLen > 11);		/* fits largest data telegram */
		val = readByte(pfp->i2cFN, pfa, pfp->inpr);	/* read registered MCP23017 B/A at initialisation */
		/* by default do not invert MCP23017 inputs - they are inverted with -I */
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
	    if (pfp->QPname) {							/* MCP23017 GPIOB+ */
		writeByte(pfp->i2cFN, pfa, OLATB, pfp->QPinv);	/* normally write non-inverted data to MCP23017 B output */
		pfp->QPval = 0;			/* force all output bits (TODO not used) */
	    }
	}
    }
#endif /* ################################################################## */
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
    if (mcpCnt) FD_SET(gpio27FN, &ixfds);	/* watch GPIO 27 for out-of-band input - do after iC_connect_to_server() */
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
    if (iC_debug & 0200) fprintf(iC_outFP, "iC_sockFN = %d	gpio27FN = %d i2cFdA[0] = %d i2cFdA[1] = %d\n",
	iC_sockFN, gpio27FN, i2cFdA[0], i2cFdA[1]);
    /********************************************************************
     *  External input (TCP/IP via socket, I2C from MCP23017, GPIO and STDIN)
     *  Wait for input in a select statement most of the time
     *  750 ms Timer is only switched on for left shifting lcd text
     *******************************************************************/
    slr = 0;
    cbc = 0x03;					/* first input will switch to cursor/blink off */
    for (;;) {
	if ((retval = iC_wait_for_next_event(&infds, &ixfds, NULL)) > 0) {
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
				if ((un = (sel = Units[channel]).un) < 16) {
#if 0 /* ################################################################## */
				    iq = (un & 0x08) >> 2;	/* selects OLATA or OLATB */
				    un &= 0x07;			/* Unit # for pfa */
				    pfp = &iC_pfL[un];
				    pfa = pfp->pfa;
				    pfq = &(pfp->s[iq]);
				    /********************************************************************
				     *  TCP/IP output for a MCP23017
				     *******************************************************************/
				    if (pfq->i.name) {				/* is output registered? */
					if (iC_debug & 0100) fprintf(iC_outFP, "P: %s:	%hu:%d P%d	> %s\n",
					    iC_iccNM, channel, (int)val, pfa, pfq->i.name);
					if (pfa != 4 || pfp->intf != INTFA) {	/* MCP23017 */
					    /********************************************************************
					     *  Direct output to a MCP23017
					     *******************************************************************/
					    writeByte(pfp->i2cFN, pfa,
							iq ? OLATB : OLATA,
							(val & pfq->bmask) ^ pfq->inv
						     );	/* normally write inverted data to MCP23017 A output */
					}
				    } else {
					fprintf(iC_errFP, "WARNING: %s: unit = %d, iq = %d, channel = %hu, val = %d; no output registered - should not happen\n",
					    iC_progname, un, iq, channel, val);		/* should not happen */
				    }
#endif /* ################################################################## */
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
	     *  GPIO 27 interrupt means I2C input from a MCP23017
	     *******************************************************************/
	    if (gpio27FN > 0 && FD_ISSET(gpio27FN, &iC_exfds)) {	/* watch for out-of-band GPIO 27 input */
		m1 = 0;
#if YYDEBUG && !defined(_WINDOWS)
		if (iC_micro) iC_microPrint("I2C input received", 0);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		cp = regBuf;
		regBufLen = REPLY;
		if (iC_debug & 0300) {
		    op = buffer;
		    ol = BS;
		}
#if 0 /* ################################################################## */
		do {
		    /********************************************************************
		     *  Scan MCP23017 units for input interrupts (even those not used)
		     *  More interrupts can arrive for MCP23017's already scanned, especially
		     *  with bouncing mechanical contacts - repeat the scan until GPIO 27 == 1
		     *******************************************************************/
		    m = 0;
		    for (pfp = iC_pfL; pfp < &iC_pfL[mcpCnt]; pfp++) {
			if (readByte(pfp->i2cFN, pfp->pfa, pfp->intf)) {	/* interrupt flag on this unit ? */
			    assert(regBufLen > 11);				/* fits largest data telegram */
			    val = readByte(pfp->i2cFN, pfp->pfa, pfp->inpr);	/* read MCP23017 B/A at interrupt */
			    if (val != pfp->Ival) {
				if (pfp->Iname) {
				    /* by default do not invert MCP23017 inputs - they are inverted with -I */
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
				    fprintf(iC_outFP, "P: %s: %d input on unregistered MCP23017 %d\n", iC_iccNM, val, pfp->pfa);
				}
				pfp->Ival = val;				/* store change for comparison */
			    }
			    m++;						/* count INTF interrupts found */
			}
		    }
		    m1++;
		    if ((val = gpio_read(gpio27FN)) == -1) {
			perror("GPIO 27 read");
			fprintf(iC_errFP, "ERROR: %s: GPIO 27 read failed\n", iC_progname);
			break;
		    }
		} while (val != 1);			/* catch interrupts which came in during for loop */
#endif /* ################################################################## */
		if (cp > regBuf) {
		    iC_send_msg_to_server(iC_sockFN, regBuf+1);			/* send data telegram(s) to iCserver */
		    if (iC_debug & 0100) fprintf(iC_outFP, "P: %s:	%s	<%s\n", iC_iccNM, regBuf+1, buffer);
		}
		if ((iC_debug & (DQ | 0200)) == 0200) {
		    if (m1 > 1){
			fprintf(iC_errFP, "WARNING: %s: GPIO 27 interrupt %d loops %d changes \"%s\"\n", iC_progname, m1, m, regBuf+1);
		    } else if (m == 0) {	/* for some reason this happens occasionaly - no inputs are missed though */
			fprintf(iC_errFP, "WARNING: %s: GPIO 27 interrupt and no INTF set on MCP23017s\n", iC_progname);
		    }
		    *(regBuf+1) = '\0';			/* clean debug output next time */
		}
	    }	/*  end of GPIO 27 interrupt */
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
#if YYDEBUG && !defined(_WINDOWS)
			    if (m1++ == 0 && iC_micro) iC_microPrint("GPIO input received", 0);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
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
#if YYDEBUG && !defined(_WINDOWS)
		} else if (c == 't') {
		    iC_debug ^= 0100;			/* toggle -t flag */
		} else if (c == 'm') {
		    iC_micro ^= 1;			/* toggle micro */
#endif	/* YYDEBUG && !defined(_WINDOWS) */
		} else if (c == 'T') {
		    iC_send_msg_to_server(iC_sockFN, "T");	/* print iCserver tables */
#ifdef	TRACE
		} else if (c == 'i') {
		    for (pfp = iC_pfL; pfp < &iC_pfL[mcpCnt]; pfp++) {	/* report MCB23017 IOCON, GPINTEN */
			fprintf(iC_outFP, "%s: un = %d pfa = %d IOCON = 0x%02x GPINTEN = 0x%02x\n",
			    pfp->Iname, pfp - iC_pfL, pfp->pfa,
			    readByte(pfp->i2cFN, pfp->pfa, IOCON),
			    readByte(pfp->i2cFN, pfp->pfa, pfp->intf == INTFB ? GPINTENB : GPINTENA));
		    }
		} else if (c == 'I') {
		    for (pfp = iC_pfL; pfp < &iC_pfL[mcpCnt]; pfp++) {	/* restore MCB23017 IOCON, GPINTEN */
			fprintf(iC_outFP, "%s: un = %d pfa = %d restore IOCON <== 0x%02x GPINTEN <== 0x%02x\n",
			    pfp->Iname, pfp - iC_pfL, pfp->pfa,
			    IOCON_SEQOP | IOCON_HAEN | ((mcpCnt == 1) ? 0 : IOCON_ODR), 0xff);
			writeByte(pfp->i2cFN, pfp->pfa, IOCON, IOCON_SEQOP | IOCON_HAEN | ((mcpCnt == 1) ? 0 : IOCON_ODR));
			writeByte(pfp->i2cFN, pfp->pfa, pfp->intf == INTFB ? GPINTENB : GPINTENA, 0xff);
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
		    iC_quit(QUIT_TERMINAL);		/* TODO remove */
} /* main */

/********************************************************************
 *
 *	Scan and process either one MCP23017 IEC argument or a list
 *	or a range of them. MCP23017 IEC lists or ranges are either
 *	all input or output IECs.
 *	  single:  IX100		or   QX101
 *	  list:    IX200,IX210,IX250	or   QX201,QX211,QX251
 *	  range:   IX300-IX370		or   QX301-QX371
 *	Each of the three argument groups can be preceded by '~' for
 *	inverting each IEC in the group or followed by ,<mask> or
 *	-<instance> or ,<mask>-<instance> in which case .
 *
 *******************************************************************/

static int
scanIEC(const char * format, char * iqs, char * iqe, char * cng, int * ieStartp, int * ieEndp, char * tail)
{
    int			n;
    int			r;
    int			ch;
    int			ns;
    int			gs;

    static int		chs;
    static int		nss;
    static int		gss;
//    mcpIO **		mcpP;		/* pointer to MCP23017 info matrix */
//    mcpIO *		mcp;		/* pointer to MCP23017 info */

    if ((n = sscanf(tail, format, iqe, cng, tail)) >= 1) {
	/* the first test cannot occur in the initial scan because format lacks [~] */
	if (*iqe == '~') {
	    fprintf(iC_errFP, "ERROR: %s: '%s' no '~' inversion operator allowed in an MCP list or range\n", iC_progname, *argp);
	    errorFlag++;
	    return 6;				/* goto endOneArg */
	}
	if (*format == '%') {
	    if (n <= 1) {
		return 5;			/* goto illFormed */
	    }
	} else {
	    if (n <= 0) {
		return 5;			/* goto illFormed */
	    }
	    if (n == 1) {
		if (isdigit(*iqe)) {
		    switch (tail[0]) {		/* test for mask or instance */
			case ',': return 1;	/* goto checkMask */
			case '-': return 4;	/* goto checkInst */
			default:  return 5;	/* goto illFormed */
		    }
		}
	    }
	}
	assert(n >= 2);
	*ieEndp = atoi(cng);			/* assume a single IXn or QXn */
	if (*ieEndp <= 99) {
	    // IEC is GPIO variable
	    if (*format == '%') {
		return 3;			/* goto processGPIO */
	    } else {
		fprintf(iC_errFP, "ERROR: %s: '%s' GPIO IEC variable in an MCP23017 list or range\n", iC_progname, *argp);
		errorFlag++;
		return 6;			/* goto endOneArg */
	    }
	}
	if (*format != '%' && *iqs != *iqe) {
	    fprintf(iC_errFP, "ERROR: %s: '%s' mixed input and output in an MCP23017 list or range\n", iC_progname, *argp);
	    errorFlag++;
	    return 6;				/* goto endOneArg */
	}
	ch = cng[0] - '0';
	ns = cng[1] - '0';
	gs = cng[2] - '0' - offset;
	if (*ieEndp > 999 || ch < 1 || ch > 8 || ns > 7 || gs < 0 || gs > 1 || (ch == 8 && ns == 7)) {
	    fprintf(iC_errFP, "ERROR: %s: '%s' does not address a data MCP23017 with offset %d\n", iC_progname, *argp, offset);
	    errorFlag++;
	    return 6;				/* goto endOneArg */
	}
//    printf("ch = %d chs = %d gs = %d gss = %d\n", ch, chs, gs, gss);
	if (*format == '-') {
	    if (ch != chs || gs != gss) {
		fprintf(iC_errFP, "ERROR: %s: '%s' only allowed MCP23017 range is in dev address (middle digit)\n", iC_progname, *argp, offset);
		errorFlag++;
		return 6;			/* goto endOneArg */
	    }
	    if (nss >= ns) {
		fprintf(iC_errFP, "ERROR: %s: '%s' MCP23017 range in dev address (middle digit) is not positive\n", iC_progname, *argp, offset);
		errorFlag++;
		return 6;			/* goto endOneArg */
	    }
	}
#if 0 /* ################################################################## */
	// TODO start MCP23017 list with *ieStartp
	mcpP = &mcpL[(ch-1 << 3) | ns];			/* index into mcpL[] */
	if (*mcpP == (void *) -1) {
	    fprintf(iC_errFP, "ERROR: %s: '%s' the addressed MCP23017 does not exist\n", iC_progname, *argp);
	    errorFlag++;
	    return 6;				/* goto endOneArg */
	}
	if (*mcpP) {				/* not NULL (or -1) */
	    /* TODO shift all this to after mask has been determined to allow multiple use with disjoint masks */
	    fprintf(iC_errFP, "ERROR: %s: '%s' the addressed MCP23017 has been used\n", iC_progname, *argp);
	    errorFlag++;
	    return 6;				/* goto endOneArg */
	}
	*mcpP = mcp = iC_emalloc(sizeof(mcpIO));	/* new gpioIO element */
#endif /* ################################################################## */
    printf("	MCP23017 %s%cX%d\n", invFlag ? "~" : "", *iqe, *ieEndp);
	if (n == 2) {
	    tail[0] = '\0';			/* single IEC argument without ,<mask> or -<inst> */
	    return 2;				/* goto checkOptionG */
	}
	if (*format == '%') {
	    /********************************************************************
	     *  Scan more IEC arguments in a list or range
	     *******************************************************************/
	    *iqs = *iqe;			/* enter here only in initial scan */
//	    iqStart = iqEnd;
	    *ieStartp = *ieEndp;
	    chs = ch;
	    nss = ns;
	    gss = gs;
	    while (tail[0] == ',') {
		/********************************************************************
		 *  Scan next IEC in an MCP23017 IEC argument list
		 *******************************************************************/
		if ((r = scanIEC(",%1[~IQ0-9]X%5[0-9]%127s", iqs, iqe, cng, ieStartp, ieEndp, tail)) != 0) {
		    return r;			/* a goto condition has been found */
		}
	    }
	    if (tail[0] == '-') {
		/********************************************************************
		 *  Scan last IEC in an MCP23017 IEC argument range
		 *******************************************************************/
		if ((r = scanIEC("-%1[~IQ0-9]X%5[0-9]%127s", iqs, iqe, cng, ieStartp, ieEndp, tail)) != 0) {
		    return r;			/* a goto condition has been found */
		}
    printf("tail = '%s'\n", tail);
		if (!isdigit(tail[1])) {
		    return 5;			/* goto illFormed */
		}
		switch (tail[0]) {		/* test for mask or instance */
		    case ',': return 1;		/* goto checkMask */
		    case '-': return 4;		/* goto checkInst */
		    default:  return 5;		/* goto illFormed */
		}
	    }
	}
    }
    return 0;
} /* scanIEC */

/********************************************************************
 *
 *	Initalise and expand dynamic array Units[] as necessary
 *	Store MCP23017 unit numbers in Units[] indexed by channel
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
    Units[channel] = sel;			/* store MCP23017 unit number gpioIO * */
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
 *	Unexport GPIOs and close MCP23017s
 *	Clear gpiosp->u.used bits for GPIOs and A/D channels used in this app
 *
 *******************************************************************/

static int
termQuit(int sig)
{
    if (iC_opt_P) {
	if (iC_debug & 0200) fprintf(iC_outFP, "=== Unexport GPIOs and close MCP23017s =======\n");
	mcpIO *		pfp;
	int		sel;
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
#if 0 /* ################################################################## */
	/********************************************************************
	 *  MCP23017s
	 *******************************************************************/
	if (mcpCnt) {
	    if ((iC_debug & 0200) != 0) fprintf(iC_outFP, "### Shutdown active MCP23017 units\n");
	    /********************************************************************
	     *  Turn off the pullup resistor on gpio27/INTB
	     *******************************************************************/
	    iC_gpio_pud(27, BCM2835_GPIO_PUD_OFF);
	    for (pfp = iC_pfL; pfp < &iC_pfL[mcpCnt]; pfp++) {
		/********************************************************************
		 *  Clear active MCP23017 outputs and turn off active MCP23017Cad
		 *  Shutdown all active MCP23017 Units leaving interrupts off and open drain
		 *******************************************************************/
		if (pfp->Qgate || pfp->Igate || pfp->QPgate) {		/* works for both iCpiI2C and apps */
		    if (pfp->Qgate) {
			if (pfp->pfa != 4 || pfp->intf != INTFA) {	/* MCP23017 */
			    writeByte(pfp->i2cFN, pfp->pfa, OLATA, pfp->Qinv);
			}
		    }
		    if (pfp->QPgate) {
			writeByte(pfp->i2cFN, pfp->pfa, OLATB, pfp->QPinv);
		    }
		    if (setupMCP23017(pfp->i2cFN, pfp->pfa, IOCON_ODR, 0, 0x00, 0x00) < 0) {
			fprintf(iC_errFP, "ERROR: %s: MCP23017 %d not found after succesful test ???\n",
			    iC_progname, pfp->pfa);
		    }
		}
	    }
	    /********************************************************************
	     *  Close selected i2cdev devices
	     *******************************************************************/
	    for (sel = 0; sel < 2; sel++) {
		if (i2cFdA[sel] > 0) {
		    close(i2cFdA[sel]);		/* close connection to /dev/i2cdev0.0, /dev/i2cdev0.1 */
		}
	    }
	    /********************************************************************
	     *  Close GPIO 27/INTB/INTA value
	     *  free up the sysfs for gpio 27 unless used by another program (SIGUSR2)
	     *******************************************************************/
	    if (gpio27FN > 0) {
		close(gpio27FN);			/* close connection to /sys/class/gpio/gpio27/value */
		if (iC_debug & 0200) fprintf(iC_outFP, "### Unexport GPIO 27\n");
		if (sig != SIGUSR2 && gpio_unexport(27) != 0) {
		    sig = SIGUSR1;			/* unable to unexport gpio 27 */
		}
	    }
	}
#endif /* ################################################################## */
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
	if (iC_debug & 0200) fprintf(iC_outFP, "=== End Unexport GPIOs and close MCP23017s ===\n");
    } /* iC_opt_P */
    return (sig);			/* finally quit */
} /* termQuit */
#endif	/* defined(RASPBERRYPI) && defined(TCP) && !defined(_WINDOWS) */

/* ############ POD to generate iCpiI2C man page ############################

=encoding utf8

=head1 NAME

iCpiI2C - real I2C digital I/O on a Raspberry Pi for the iC environment

=head1 SYNOPSIS

 iCpiI2C  [-GBIftmqzh][ -s <host>][ -p <port>][ -n <name>][ -i <inst>]
          [ -o <offs>][ -d <deb>]
          [ [~]IXcng[,IXcng,...][,<mask>][-<inst>] ...]
          [ [~]QXcng[,QXcng,...][,<mask>][-<inst>] ...]
          [ [~]IXcng-IXcng[,<mask>][-<inst>] ...]
          [ [~]QXcng-QXcng[,<mask>][-<inst>] ...]
          [ [~]IXn,<gpio>[,<gpio>,...][-<inst>] ...]
          [ [~]QXn,<gpio>[,<gpio>,...][-<inst>] ...]
          [ [~]IXn.<bit>,<gpio>[-<inst>] ...]
          [ [~]QXn.<bit>,<gpio>[-<inst>] ...]      # at least 1 IEC argument
          [ -R <aux_app>[ <aux_app_argument> ...]] # must be last arguments
    -s host IP address of server    (default '127.0.0.1')
    -p port service port of server  (default '8778')
    -i inst instance of this client (default '') or 1 to 3 digits
    -o offs offset for MCP GPIOA/GPIOB selection in a second iCpiI2C in the
            same network (default 0; 2, 4, 6, 8 for extra iCpI2C's)
            All IXcng and QXcng declarations must use g = offs or offs+1
    -G      service GPIO I/O only - block MCP23017s and MCP23017 arguments
    -B      start iCbox -d to monitor active MCP23017 and/or GPIO I/O
    -I      invert all MCP23017 and GPIO inputs and outputs
            MCP23017 and GPIO input circuits are normally hi - 1 and
            when a switch on the input is pressed it goes lo - 0. Therefore
            it is appropriate to invert inputs and outputs. When inverted
            a switch pressed on an input generates a 1 for the IEC inputs and
            a 1 on an IEC output turns a LED and relay on, which is natural.
            An exception is a relay with an inverting driver, whose output
            must be non-inverting (~ operator inverts bit variables).
    -W GPIO number used by the w1-gpio kernel module (default 4, maximum 31).
            When the GPIO with this number is used in this app, iCtherm is
            permanently blocked to avoid Oops errors in module w1-gpio.
    -f      force use of all interrupting gpio inputs - in particular
            force use of gpio 27 - some programs do not unexport correctly.

                      MCP23017 arguments
          For each MCP23017 connected to a Raspberry Pi two 8 bit registers
          GPIOA and GPIOB must be configured as IEC inputs (IXcng) or
          outputs (QXcng) or both, where cng is a three digit number.
          For MCP23017 IEC base identifiers the number cng defines the
          complete address of a particular MCP23017 GPIO register:
           c  is the channel 1 to 8 for /dev/i2c-11 to /dev/i2c-18
           n  is the address 0 to 7 of a particular MCP23017 for the
              address range 0x20 to 0x27 selected with A0 to A2
           g  selects the GPIO register
              g = 0 2 4 6 or 8 selects GPIOA
              g = 1 3 5 7 or 9 selects GPIOB
              normally only g = 0 and 1 is used for GPIOA and GPIOB,
              but if a second instance of iCpiI2C is used on another
              Raspberry Pi, but connected to the one iCsever the
              alternate numbers differentiate between IEC variables
              for the two RPi's.  Second or more instances of iCpiI2C
              must be called with the option -o 2, -o 4 etc for
              using 2 and 3 or 4 and 5 etc to select GPIOA and GPIOB.
          Both registers GPIOA and GPIOB can have input bits and
          output bits, but any input bit cannot also be an output bit
          or vice versa.  Each input or output IEC variable (or list
          of variables) may be followed by a comma separated number,
          interpreted as a mask, declaring which bits in the base IEC
          variable byte are inputs for an input variable or outputs
          for an output variable. The mask is best written as a binary
          number eg 0b10001111 for bits 0, 1, 2, 3 and 7.  If no mask
          is supplied the default mask is 0b11111111 - all 8 bits set.
    IXcng[,IXcng,...][,<mask>]   list of input variables with the same mask
    QXcng[,QXcng,...][,<mask>]   list of output variables with the same mask
    IXcng-IXcng[,<mask>]         range of input variables with the same mask
    QXcng-QXcng[,<mask>]         range of output variables with the same mask
         IEC lists and ranges must contain either all inputs or all outputs.
         The only range supported is in the digit n for the MCP23017 address.

                      GPIO arguments
          Any IEC I/Os IXn.y and QXn.y which are to be linked to GPIO
          inputs or outputs must be named in the argument list. The
          number n in the IEC must be one or at most two digits to
          differentiate GPIO IEC's from three digit MCP23017 IEC's.
    IXn,<gpio>[,<gpio>,...]
    QXn,<gpio>[,<gpio>,...]
          Associate the bits of a particular input or output IEC
          with a list of gpio numbers.  Up to 8 gpio numbers can
          be given in a comma separated list. The first gpio number
          will be associated with bit 0, the second with bit 1 etc
          and the eighth with bit 7. If the list is shorter than
          8 the trailing bits are not used. The letter 'd' can be
          used in the list instead of a gpio number to mark that bit
          as unused.  eg. IX3,17,18,19,20,d,d,21 associates IX3.0 to
          IX3.3 with GPIO 17 to GPIO 20 and IX3.6 with GPIO 21.
          Alternatively:
    IXn.<bit>,<gpio>
    QXn.<bit>,<gpio>
          Each input or output <bit> 0 to 7 is associated with one
          gpio nunber eg. IX3.0,17 QX3.7,18

                      COMMON extensions
    ~IEC  Each leading IEC of a list or range may be preceded by a
          tilde '~', which inverts all IEC inputs or outputs of a
          list or range.  If all IEC inputs and outputs are already
          inverted with -I, then ~ inverts them again, which makes them
          non-inverted. Individual bits for one IEC can be inverted
          or left normal for GPIO IEC's by declaring the IEC more than
          once with non-overlapping gpio bit lists or single bits. Also
          possible for MCP23017 I/O with two non overlapping masks -
          one inverting, the other non-inverting.
    IEC-inst Each individual IEC or range can be followed by -<inst>,
          where <inst> consists of 1 to 3 numeric digits.
          Such an appended instance code takes precedence over the
          general instance specified with the -i <inst> option above.

                      CAVEAT
    There must be at least 1 MCP23017 or GPIO IEC argument. No IEC
    arguments are generated automatically for iCpiI2C.
    Only one instance of iCpiI2C or an app with real IEC parameters
    like iCpiFace may be run on one RPi and all GPIOs and MCP23017s
    must be controlled by this one instance. If two instances were
    running, the common interrupts would clash. Also no other program
    controlling GPIOs and MCP23017s like 'MCP23017 Digital Emulator'
    may be run at the same time as this application.  An exception
    is iCpiPWM which controls GPIOs by DMA and not by interrupts.
    Another exception is iCtherm which controls GPIO 4 by the 1Wire
    interface.  Care is taken that any GPIOs or MCP23017s used in one
    app, iCpiI2C, iCpiPWM or even iCtherm do not clash with another
    app (using ~/.iC/gpios.used).

                      DEBUG options
    -t      trace arguments and activity (equivalent to -d100)
         t  at run time toggles gate activity trace
    -m      display microsecond timing info
         m  at run time toggles microsecond timing trace
    -d deb  +1   trace TCP/IP send actions
            +2   trace TCP/IP rcv  actions
            +10  trace I2C  input  actions
            +20  trace I2C  output actions
            +100 show arguments
            +200 show more debugging details
            +400 exit after initialisation
    -q      quiet - do not report clients connecting and disconnecting
    -z      block keyboard input on this app - used by -R
    -h      this help text
         T  at run time displays registrations and equivalences in iCserver
         q  or ctrl+D  at run time stops iCpiI2C

                      AUXILIARY app
    -R <app ...> run auxiliary app followed by -z and its arguments
                 as a separate process; -R ... must be last arguments.

=head1 DESCRIPTION

B<iCpiI2C> is an I/O client for iCserver in the immediate C environment
of a Raspberry Pi handling real 16 bit inputs and/or 16 bit outputs for
each MCP23017 controller attached to a Raspberry Pi via 8 multiplexed
I2C busses, or from a number of direct GPIO pins on the Raspberry Pi
or both.

A maximum of 127 MCP23017 controllers can be handled altogether.
8 MCP23017 controllers per multiplexed I2C channel /dev/i2c-11 to
/dev/i2c-17 and 7 MCP23017 controllers for channel /dev/i2c-18.
Each MCP23017 can have 16 I/Os in two 8 bit registers A and B.
Each I/O can be either an input or an output or not used at all.

All GPIO pins on a Raspberry Pi A, B, B+, 3B+ or 4 may be selected
as either input bits or output bits independent of whether MCP23017s
are present or not, except GPIO 2, 3 and 27 if MCP23017s are also
processed.  All MCP23017 and GPIO inputs are handled by interrupts.

=head1 TECHNICAL BACKGROUND

The direct GPIO connections on the Raspberry Pi:

 0) The Raspberry Pi A or B brings out 17 GPIO signals on a 26 pin
    connecteor, 5 of which double up to do the I2C interface, 2 for
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

 KERNEL=="i2cdev*", GROUP="i2c", MODE="0660"
 SUBSYSTEM=="gpio*", PROGRAM="/bin/sh -c 'chown -R root:gpio /sys/class/gpio &&\
 chmod -R 770 /sys/class/gpio; chown -R root:gpio /sys/devices/virtual/gpio &&\
 chmod -R 770 /sys/devices/virtual/gpio;\
 chown -R root:gpio /sys/devices/platform/soc/*.gpio/gpio &&\
 chmod -R 770 /sys/devices/platform/soc/*.gpio/gpio'"

    Another complication is, that after a "sysfs" export, the first access
    to the direction (or any other) register takes about 200 ms. I have
    taken care of this by retrying  every 50 ms for a max of 2.5 seconds
    before giving up. (Discussed in a number of forums).

The I/O connections on any MCP23017 board are controlled from the
Raspberry Pi via four different paths:

 1) the kernel I2C system can write and read to and from the MCB23017
    16-Bit I/O Expander chip on a Piface. For Details see:
    http://ww1.microchip.com/downloads/en/DeviceDoc/21952b.pdf
    There are two I2C channels on the Raspberry Pi:
        /dev/i2cdev0.0 - which enables CS CE0 when I2C writes or reads.
        /dev/i2cdev0.1 - which enables CS CE1 when I2C writes or reads.
    The MCB23017 is a slave I2C device. The slave address contains four
    fixed bits and three user-defined hardware address bits pins A0, A1
    and A2 (if enabled via IOCON.HAEN).
    NOTE: A2 is not used in the MCP23017 Digital 1 (28 pin) and 2 (40 pin),
          which means selection of MCP23017 4 - 7 is via CE1 (see below).
      **  A2 is used with A1/A0 on MCP23017 Relay+ to select 8 units on CE0.
    The Read/Write bit fills out the control byte.
          Jumpers JP1 and JP2 select A0 and A1 on MCP23017 1, 2 and Relay+.
      **  Jumper JP3 selects A2 on MCP23017 Relay+ (does not need CE1).
    Jumpers JP1-JP8 on a PiRack can swap CE1 for CE0 for every MCP23017 1 or 2
    on a PiRack. Together this makes for 8 possible MCP23017 addresses 0 - 7.

 2) The MCB23017 has a number of 8 bit registers which can be written
    and read via the above I2C channels. The most important of these
    for the MCP23017 driver are:
        IOCON   - which sets the configuration of the MCB23017.
        IODIRA  - set to 0x00, which makes all 8 port A pins outputs.
        IODIRB  - set to 0xff, which makes all 8 port B pins inputs.
        OLATA   - which sets the 8 output pins when written to.
        GPIOB   - which returns the 8 input pins when read.
        GPINTENB- set to 0xff, a change on any port B pin activates INTB.
        INTFB   - set for any pin which changed and caused an interrupt.
        INTCAPB - which returns the 8 input pins at interrupt time.

    The INTB output, which signals a change on any of the input bits,
    is set to active-low in an open-drain configuration to allow for more
    than one MCP23017. INTB is connected to pin 13 on all MCP23017s going to
    GPIO 27 on the Raspberry Pi.

    Reading either GPIOB or INTCAPB resets the source of the input
    interrupt.  INTCAPB only supplies the state at interrupt time,
    which does not necessarily reflect the state of the input when
    executing the interrupt handler. Using INTCAPB sometimes led to
    lost input changes. Therefore I only read GPIOB.

    When INTFB from several MCP23017s are scanned in the interrupt
    handler, it sometimes happens that another change of input
    (usually due to fast bounces from a pushbutton contact) occurs
    before the last MCP23017 has been dealt with sending its INTFB low
    again. This means that INTB will remain low after the last MCP23017
    has been dealt with. Since the GPIO 27 interrupt only occurs on
    a falling edge, no interrupt will be seen for the latest change
    and the system hangs with INTB permanently low. That is why it is
    important to read the GPIO 27 value after all Pifaces have been
    scanned. If it is still low, the scan is repeated until it goes
    high, which makes GPIO 27 ready for the next falling edge interrupt.

 3) GPIO 27 (the interrupt from the MCP23017 inputs) is exported with
    "sysfs" in the MCP23017 driver and set to interrupt on its falling
    edge. The interrupt is out-of-band, so if select(2) is used,
    the third exception file descriptor set must be used. select is
    generally used in the iC system, rather than poll(2), because
    select handles timeouts correctly when interrupted.

    NOTE: only one instance of iCpiI2C may be run and all MCP23017s as
    well as extra GPIO's must be controlled by this one instance. If
    two instances were running, the common interrupts would clash.
    If GPIO 27 has already been exported by another program or by
    another instance of iCpiI2C the program exits with an error
    message, since GPIO 27 cannot be used by more than one program
    simultaneously.  Some programs (MCP23017 Digital Emulator) do
    not unexport GPIO 27 when they close. The -f command line option
    can be used to force use of GPIO 27 if you are sure there is no
    running program actually using GPIO 27.

 4) To allow several MCP23017 units to run
    simultaneously on a PiRack, the INTB/GPIO 27 line requires a pullup
    resistor, so the individual INTB outputs from each MCB23017 can
    signal their interrupts via open-drain outputs. There is no pullup
    resistor on the MCP23017 or the PiRack. But each GPIO
    pin on the Raspberry Pi can be configured to have a 60 Kohm pull-up
    or pull-down resistor. Unfortunately there is no way to control
    this with the "sysfs". To do this the BCM2835 direct memory access
    interface is used when BCM2835 is defined. For details see:
        http://www.airspayce.com/mikem/bcm2835/index.html
    This package does not support interrupts, so only the setting of
    the pullup resistor on INTB/GPIO 27 is done with this package.

    Unfortunately this means iCpiI2C must be run as SUID root, which
    prevents debugging and is a nuisance generally.  You can solve this
    problem for multiple MCP23017s by soldering a 100 Kohm resistor from
    one of the GPIO 27 (pin 13) to 5V on the PiRack.  Take your pick!!

    Another option is to use an independent simple SUID program to set
    pullup resistors on selected GPIO pins. iCgpioPUD does this.  In the
    latest version iCgpioPUD is called internally if the requested
    options require pullups on GPIO 27 or any other GPIO input pins.

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

   ############ end of POD to generate iCpiI2C man page ##################### */

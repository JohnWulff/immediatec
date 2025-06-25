/********************************************************************
 *
 *	COPYRIGHT (C) 2014 - 2025  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	iCpiGPIO
 *
 *	I/O client for iCserver in the immediate C environment of a
 *	Raspberry Pi handling real bit inputs and bit outputs on
 *	GPIO pins.
 *
 *	All GPIO pins on a Raspberry Pi A, B or B+ may be selected as
 *	either input bits or output bits unless they are already used
 *	in another application like GPIO 7 to GPIO 11 and GPIO 25 in
 *	an iCpiFace.
 *
 *	All GPIO inputs are handled by interrupts.
 *
 *	The original version of this driver, as implemented in iCpiFace
 *	to handle real I/O's for both PiFace extenders and GPIO pins
 *	was based on 'sysfs', which has been deprecated since 2022.
 *	I have used 'sysfs' without problems for 11 years. This driver
 *	calls 'sysfs' for Raspberry PI OS's using older versions of the
 *	Linux kernel. This allows the use of older Raspberry PI OS's.
 *
 *	Since version 6.6 of the Linux kernel 'sysfs' is no longer available
 *	and a new interface called 'GPIO Character Device Userspace API V2'
 *	has been added.
 *
 i	This interface has been under development over recent years. It
 *	comes in two versions 'ABI V1' and 'ABI V2', which have different
 *	IOCTL calls and data structures. 'ABI V1' is now deprecated.
 *
 *	Both versions are published with a wrapper called 'libgpiopd'.
 *	I could never get 'libgpiod V1' working properly. 'libgpiod V2'
 *	is still in development and is not avialable in the latest RPi OS
 *	labelled 'bookworm'.
 *
 *	This driver uses direct IOCTL calls to 'ABI V2' for RPi OS's with
 *	Linux kernels >= 5.10, which works well.
 *
 *******************************************************************/

#if !defined(RASPBERRYPI) || !defined(TCP) || defined(_WINDOWS)
#error - must be compiled with RASPBERRYPI and TCP defined and not _WINDOWS
#else	/* defined(RASPBERRYPI) && defined(TCP) && !defined(_WINDOWS) */

#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */
#include	<linux/gpio.h>
#include	<sys/ioctl.h>
#include	<stdlib.h>
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
#include	<signal.h>
#include	<ctype.h>
#include	<assert.h>
#include	<errno.h>
#include	<sys/stat.h>
#include	<fcntl.h>

#include	"tcpc.h"
#include	"icc.h"			/* declares iC_emalloc() in misc.c */
#include	"rpi_rev.h"		/* Determine Raspberry Pi Board Rev */
#if RASPBERRYPI < 5010	/* sysfs */
#include	"rpi_gpio.h"
#include	"bcm2835.h"		/* iC_gpio_pud parameters */
#endif	/* RASPBERRYPI < 5010 - sysfs */

static const char *	usage =
"Usage:\n"
" %s [-BIftmqzh][ -s <host>][ -p <port>][ -n <name>][ -i <inst>]\n"
#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */
"           [ -D <db>]"
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
"          [ -W <GPIO_number>][ -d <deb>]\n"
"          [ [~]IXn,<gpio>[,<gpio>,...][-<inst>] ...]\n"
"          [ [~]QXn,<gpio>[,<gpio>,...][-<inst>] ...]\n"
"          [ [~]IXn.<bit>,<gpio>[-<inst>] ...]\n"
"          [ [~]QXn.<bit>,<gpio>[-<inst>] ...]      # at least 1 IEC argument\n"
"          [ -R <aux_app>[ <aux_app_argument> ...]] # must be last arguments\n"
"    -s host IP address of server    (default '%s')\n"
"    -p port service port of server  (default '%s')\n"
"    -i inst instance of this client (default '') or 1 to %d digits\n"
"    -B      start iCbox -d to monitor active GPIO I/O\n"
"    -I      invert all GPIO inputs and outputs\n"
"            GPIO input circuits are normally hi - 1 and when a switch on the\n"
"            input is pressed it goes lo - 0. Therefore it is appropriate to\n"
"            invert inputs and outputs. When inverted a switch pressed on an\n"
"            input generates a 1 for the IEC inputs and a 1 on an IEC output\n"
"            turns a LED and relay on, which is natural.\n"
#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */
"    -D db   microsecend debounce for GPIO inputs (default 0 - no debounce)\n"
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
"    -W GPIO number used by the w1-gpio kernel module (default 4, maximum 31).\n"
"            When the GPIO with this number is used in this app, iCtherm is\n"
"            permanently blocked to avoid Oops errors in module w1-gpio.\n"
"    -f      force use of all interrupting gpio inputs.\n"
"            some programs do not unexport correctly.\n"
"\n"
"                      GPIO arguments\n"
"          Any IEC I/Os IXn.y and QXn.y which are to be linked to GPIO\n"
"          inputs or outputs must be named in the argument list as follows:\n"
"    IXn,<gpio>[,<gpio>,...]\n"
"    QXn,<gpio>[,<gpio>,...]\n"
"          Associate the bits of a particular input or output IEC\n"
"          with a list of gpio numbers.  Up to 8 gpio numbers can\n"
"          be given in a comma separated list. The first gpio number\n"
"          will be aassociated with bit 0, the second with bit 1 etc\n"
"          and the eighth with bit 7. If the list is shorter than\n"
"          8 the trailing bits are not used. The letter 'd' can be\n"
"          used in the list instead of a gpio number to mark that bit\n"
"          as unused.  eg. IX3,17,18,19,20,d,d,21 associates IX3.0 to\n"
"          IX3.3 with GPIO17 to GPIO20 and IX3.6 with GPIO21.\n"
"          Alternatively:\n"
"    IXn.<bit>,<gpio>\n"
"    QXn.<bit>,<gpio>\n"
"          Associate a single input or output <bit> 0 to 7 with one\n"
"          gpio nunber eg. IX3.0,17 QX3.7,18\n"
"    ~IEC  Each lone IEC or leading IEC of a range may be preceded by a\n"
"          tilde '~', which inverts lone IEC inputs or outputs or all\n"
"          members of a range.  If all IEC inputs and outputs are already\n"
"          inverted with -I, then ~ inverts them again, which makes them\n"
"          non-inverted. Individual bits for one IEC can be inverted\n"
"          or left normal for GPIO IEC's by declaring the IEC more than\n"
"          once with non-overlapping gpio bit lists or single bits.\n"
"    IEC-inst Each individual IEC or range can be followed by -<inst>,\n"
"          where <inst> consists of 1 to %d numeric digits.\n"
"          Such an appended instance code takes precedence over the\n"
"          general instance specified with the -i <inst> option above.\n"
"    For a full Description and Technical Background of the 'sysfs' GPIO\n"
"    driver software see the %s man page.\n"
"\n"
"                      CAVEAT\n"
"    There must be at least one GPIO IEC argument. No IEC\n"
"    arguments are generated automatically for %s.\n"
"    Several instances of this program may be run as long as they are different\n"
"    instances (-i option) as well as iCpiFace and iCpiI2C. These different\n"
"    programs may not share a GPIO number and IEC's must not clash\n"
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
"            +100 show arguments\n"
"            +200 show more debugging details\n"
"            +400 exit after initialisation\n"
"    -q      quiet - do not report clients connecting and disconnecting\n"
"    -z      block keyboard input on this app - used by -R\n"
"    -h      this help text\n"
"         T  at run time displays registrations and equivalences\n"
"         q  or ctrl+D  at run time stops %s\n"
"\n"
"                      AUXILIARY app\n"
"    -R <app ...> run auxiliary app followed by -z and its arguments\n"
"                 as a separate process; -R ... must be last arguments.\n"
"\n"
"Copyright (C) 2014-2025 John E. Wulff     <immediateC@gmail.com>\n"
"Version	$Id: iCpiGPIO.c 1.4 $\n"
;

char *		iC_progname;		/* name of this executable */
static char *	iC_path;
short		iC_debug = 0;
#if YYDEBUG && !defined(_WINDOWS)
int		iC_micro = 0;
#endif	/* YYDEBUG && !defined(_WINDOWS) */
unsigned short	iC_osc_lim = 1;

static fd_set		infds;		/* initialised file descriptor set for normal iC_wait_for_next_event() */
static fd_set		ixfds;		/* initialised extra descriptor set for normal iC_wait_for_next_event() */

static unsigned short	topChannel = 0;
static char	buffer[BS];
static char	SR[] = "RSR";		/* generates RQ for iq 0, SI for iq 1, RQ for iq 2 */
static char	IQ[] = "QIQ";
static short	errorFlag = 0;
static gpioIO**	Units = NULL;		/* dynamic array to store GPIOl */
static int	ioUnits = 0;		/* dynamically allocated size of Units[] */
static long long ownUsed = 0LL;
char		iC_stdinBuf[REPLY];	/* store a line of STDIN */
static int	termQuit(int sig);		/* clear and unexport RASPBERRYPI stuff */
int		(*iC_term)(int) = &termQuit;	/* function pointer to clear and unexport RASPBERRYPI stuff */

static void	storeUnit(unsigned short channel, gpioIO * gep);
static void	writeGPIO(gpioIO * gep, unsigned short channel, int val);

FILE *		iC_outFP;		/* listing file pointer */
FILE *		iC_errFP;		/* error file pointer */
#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */

#define GPIOCHIP "/dev/gpiochip0"	/* gpiochipX (0-4) depending on Pi model */
int		chipFN;
/********************************************************************
 * data struct of values needed for reading gpio pins
 *
 * struct gpio_v2_line_config - Configuration for GPIO lines
 * @flags: flags for the GPIO lines, with values from &enum
 * gpio_v2_line_flag, such as %GPIO_V2_LINE_FLAG_ACTIVE_LOW,
 * %GPIO_V2_LINE_FLAG_OUTPUT etc, added together.  This is the default for
 * all requested lines but may be overridden for particular lines using @attrs.
 * @num_attrs: the number of attributes in @attrs
 * @padding: reserved for future use and must be zero filled
 * @attrs: the configuration attributes associated with the requested
 * lines.  Any attribute should only be associated with a particular line
 * once.  If an attribute is associated with a line multiple times then the
 * first occurrence (i.e. lowest index) has precedence.
 *
  * enum gpio_v2_line_flag - &struct gpio_v2_line_attribute.flags values
 * @GPIO_V2_LINE_FLAG_USED: line is not available for request
 * @GPIO_V2_LINE_FLAG_ACTIVE_LOW: line active state is physical low
 * @GPIO_V2_LINE_FLAG_INPUT: line is an input
 * @GPIO_V2_LINE_FLAG_OUTPUT: line is an output
 * @GPIO_V2_LINE_FLAG_EDGE_RISING: line detects rising (inactive to active)
 * edges
 * @GPIO_V2_LINE_FLAG_EDGE_FALLING: line detects falling (active to
 * inactive) edges
 * @GPIO_V2_LINE_FLAG_OPEN_DRAIN: line is an open drain output
 * @GPIO_V2_LINE_FLAG_OPEN_SOURCE: line is an open source output
 * @GPIO_V2_LINE_FLAG_BIAS_PULL_UP: line has pull-up bias enabled
 * @GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN: line has pull-down bias enabled
 * @GPIO_V2_LINE_FLAG_BIAS_DISABLED: line has bias disabled
 *
 * struct gpio_v2_line_request - Information about a request for GPIO lines
 * @offsets: an array of desired lines, specified by offset index for the
 * associated GPIO chip
 * @consumer: a desired consumer label for the selected GPIO lines such as
 * "my-bitbanged-relay"
 * @config: requested configuration for the lines
 * @num_lines: number of lines requested in this request, i.e. the number
 * of valid fields in the %GPIO_V2_LINES_MAX sized arrays, set to 1 to
 * request a single line
 * @event_buffer_size: a suggested minimum number of line events that the
 * kernel should buffer.  This is only relevant if edge detection is
 * enabled in the configuration. Note that this is only a suggested value
 * and the kernel may allocate a larger buffer or cap the size of the
 * buffer. If this field is zero then the buffer size defaults to a minimum
 * of @num_lines * 16.
 * @padding: reserved for future use and must be zero filled
 * @fd: after a successful %GPIO_V2_GET_LINE_IOCTL operation, contains
 * a valid anonymous file descriptor representing the request
 *
 * struct gpio_v2_line_values - Values of GPIO lines
 * @bits: a bitmap containing the value of the lines, set to 1 for active
 * and 0 for inactive
 * @mask: a bitmap identifying the lines to get or set, with each bit number
 * corresponding to the index into &struct gpio_v2_line_request.offsets
 *******************************************************************/
typedef struct {
    struct gpio_v2_line_config *	linecfg;
    struct gpio_v2_line_request *	linereq;
    struct gpio_v2_line_values *	linevals;
    int					fd;
} gpio_v2_t;
/********************************************************************
 * gpio_v2 line config, line request and line values,
 *******************************************************************/
static struct gpio_v2_line_config linecfg = {		/* default write normal pins */
    .flags =	GPIO_V2_LINE_FLAG_OUTPUT         |
		GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN	/* active hi by default */
};
static struct gpio_v2_line_request linereq = {
    .offsets = {0}
};
static struct gpio_v2_line_values linevals = {
    .bits = 0,
    .mask = 0
};
static int		idx;
static __u64		maskBit;
/********************************************************************
 * pins is a convenience struct of gpio_v2 config structs to which
* a pointer can be provided as the parameter for the various functions.
 *******************************************************************/
static gpio_v2_t pins =  {
    .linecfg = &linecfg,
    .linereq = &linereq,
    .linevals = &linevals
};
/********************************************************************
 * gpio_v2 attribute and attribute config for flags for out and in pins
 * active hi (non-inverted) write pins will use default linecfg flags
 * active lo (inverted) write pins will use wi_attr and wi_cfg_attr
 * active hi (non-inverted) read pins will use rd_attr and rd_cfg_attr
 * active lo (inverted) read pins will use ri_attr and ri_cfg_attr
 *
 * struct gpio_v2_line_attribute - a configurable attribute of a line
 * @id: attribute identifier with value from &enum gpio_v2_line_attr_id
 * @padding: reserved for future use and must be zero filled
 * @flags: if id is %GPIO_V2_LINE_ATTR_ID_FLAGS, the flags for the GPIO
 * line, with values from &enum gpio_v2_line_flag, such as
 * %GPIO_V2_LINE_FLAG_ACTIVE_LOW, %GPIO_V2_LINE_FLAG_OUTPUT etc, added
 * together.  This overrides the default flags contained in the &struct
 * gpio_v2_line_config for the associated line. (see gpio_v2_line_config)
 * @values: if id is %GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES, a bitmap
 * containing the values to which the lines will be set, with each bit
 * number corresponding to the index into &struct
 * gpio_v2_line_request.offsets
 * @debounce_period_us: if id is %GPIO_V2_LINE_ATTR_ID_DEBOUNCE, the
 * desired debounce period, in microseconds
 *
 * enum gpio_v2_line_attr_id - &struct gpio_v2_line_attribute.id values
 * identifying which field of the attribute union is in use.
 * @GPIO_V2_LINE_ATTR_ID_FLAGS: flags field is in use
 * @GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES: values field is in use
 * @GPIO_V2_LINE_ATTR_ID_DEBOUNCE: debounce_period_us field is in use
 *
 * struct gpio_v2_line_config_attribute - a configuration attribute
 * associated with one or more of the requested lines.
 * @attr: the configurable attribute
 * @mask: a bitmap identifying the lines to which the attribute applies,
 * with each bit number corresponding to the index into &struct
 * gpio_v2_line_request.offsets
 *******************************************************************/
struct gpio_v2_line_attribute		wi_attr = {
    .id = GPIO_V2_LINE_ATTR_ID_FLAGS
};
struct gpio_v2_line_config_attribute	wi_cfg_attr = {
    .attr = {0}
};
struct gpio_v2_line_attribute		rd_attr = {
    .id = GPIO_V2_LINE_ATTR_ID_FLAGS
};
struct gpio_v2_line_config_attribute	rd_cfg_attr = {
    .attr = {0}
};
struct gpio_v2_line_attribute		ri_attr = {
    .id = GPIO_V2_LINE_ATTR_ID_FLAGS
};
struct gpio_v2_line_config_attribute	ri_cfg_attr = {
    .attr = {0}
};
struct gpio_v2_line_attribute		rb_attr = {
    .id = GPIO_V2_LINE_ATTR_ID_DEBOUNCE
};
struct gpio_v2_line_config_attribute	rb_cfg_attr = {
    .attr = {0}
};
/********************************************************************
 * gpio_v2 line event struct as storage for data read from pin
 *
 * struct gpio_v2_line_event - The actual event being pushed to userspace
 * @timestamp_ns: best estimate of time of event occurrence, in nanoseconds
 * @id: event identifier with value from &enum gpio_v2_line_event_id
 * @offset: the offset of the line that triggered the event
 * @seqno: the sequence number for this event in the sequence of events for
 * all the lines in this line request
 * @line_seqno: the sequence number for this event in the sequence of
 * events on this particular line
 * @padding: reserved for future use
 *
 * enum gpio_v2_line_event_id - &struct gpio_v2_line_event.id values
 * @GPIO_V2_LINE_EVENT_RISING_EDGE: event triggered by a rising edge
 * @GPIO_V2_LINE_EVENT_FALLING_EDGE: event triggered by a falling edge
 *******************************************************************/
#define LINEEVENT_BUFFERS	 4	/* kernel buffers 16 line events */
#define GPIO_LIMIT		 64	/* GPIO numbers are limited 0 - 63 */
struct gpio_v2_line_event	lineevent[LINEEVENT_BUFFERS];
typedef struct gpio_T {
    gpioIO *		gep;
    unsigned short	val;		/* stores either val or bit */
} gpio_T;
static gpio_T			gpioArray[GPIO_LIMIT];
static gpio_T			gepArray[LINEEVENT_BUFFERS];
static int	gpio_line_cfg_ioctl (gpio_v2_t * gpio);
static int	gpio_line_set_values (gpio_v2_t * gpio, __u64 bits,  __u64 mask);
static int	gpio_line_get_values (gpio_v2_t * gpio, __u64 mask);
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */

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
    int			n;
    int			c;
    int			m;
    int			iq;
    int			val;
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
    char		xd[2]  = { '\0', '\0' };
    char		iqc[2] = { '\0', '\0' };
    char		iqe[2] = { '\0', '\0' };
    char		dum[2] = { '\0', '\0' };
    int			iqStart;
    int			iqEnd;
    int			ieStart = 0;
    int			ieEnd   = 0;
    unsigned short	bit, bitStart, bitEnd;
    unsigned short	gpio;
    unsigned short	directFlag;		/* GP for GPIO */
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
#if RASPBERRYPI < 5010	/* sysfs */
    int			sig;
#else	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
    char *		endptr;
    long		opt_D = 0;	/* debounce microseconds if > 0 */
    int			i;
    int			j;
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */

    iC_outFP = stdout;			/* listing file pointer */
    iC_errFP = stderr;			/* error file pointer */

#ifdef	EFENCE
    regBuf = iC_emalloc(REQUEST);
    rpyBuf = iC_emalloc(REPLY);
#endif	/* EFENCE */
    signal(SIGSEGV, iC_quit);			/* catch memory access signal */

    /********************************************************************
     *  By default do not invert GPIO inputs as well as GPIO outputs
     *  Both are inverted with -I and '~'
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
		case 'B':
		    iC_opt_B = 1;	/* start iCbox -d to monitor active GPIO I/O */
		    break;
		case 'I':
		    invMask = 0xff;	/* invert GPIO inputs as well as GPIO outputs with -I */
		    break;
#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */
		case 'D':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    opt_D = strtol(*argv, &endptr, 10);	/* convert decimal */
		    if (*endptr != '\0' || opt_D < 0) {
			fprintf(iC_errFP, "ERROR: %s: -D %s microsecond debounce is not numeric or negative\n", iC_progname, *argv);
			goto error;
		    }
		    goto break2;
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
		case 'W':
		    if (! *++*argv) { --argc; if(! *++argv) goto missing; }
		    gpioTherm = atoi(*argv);
		    if (gpioTherm > 31) {
			fprintf(iC_errFP, "ERROR: %s: -W %hu gpioTherm > 31\n", iC_progname, gpioTherm);
			goto error;
		    }
		    goto break2;
		case 'f':
		    forceFlag = 1;	/* force use of GPIO interrupts - in particular GPIO 25 */
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
		    *argv = iC_debug & DQ ?  mqz : mz;	/* point to "-qz"  or "-z" in current argv */
		    argv--;			/* start call with app string */
		    goto break3;
		missing:
		    fprintf(iC_errFP, "ERROR: %s: missing value after '%s'\n", iC_progname, ((*--argv)-=2, *argv));
		case 'h':
		case '?':
	      error:
		    fprintf(iC_errFP, usage, iC_progname, iC_hostNM, iC_portNM,
			INSTSIZE, INSTSIZE, iC_progname, iC_progname, iC_progname, iC_progname, iC_progname);
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
	     *  IEC arguments must start with ~ I Q or X.
	     *  A special case is the HOME evironment variable, which is generated
	     *  by the Linux shell for ~. It is converted back to ~ here.
	     *******************************************************************/
	    if (strspn(*argv, "~IQX") == 0) {
		if (strcmp(*argv, getenv("HOME")) == 0) {
		    argip[argic++] = "~";	/* transmogrify shell translation of HOME to ~ */
		    continue;			/* get next command line argument */
		}
		fprintf(stderr, "ERROR: %s: invalid argument '%s' (IEC variables start with ~ I Q or X\n", iC_progname, *argv);
		errorFlag++;
	    }
	    argip[argic++] = *argv;		/* erroneous arguments are passed on, but none are used */
	}
    }
  break3:
    if (iC_debug & 0200) {
#if RASPBERRYPI < 5010	/* sysfs */
	fprintf(iC_outFP, "fullPath = '%s' path = '%s' progname = '%s'\n"
			  "kernel = %d.%d ( < 5.10... )		use sysfs\n",
			  iC_fullProgname, iC_path, iC_progname, RASPBERRYPI/1000, RASPBERRYPI%1000);
#else	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
	fprintf(iC_outFP, "fullPath = '%s' path = '%s' progname = '%s'\n"
			  "kernel = %d.%d ( >= 5.10...	)	use GPIO Character Device Userspace API (V2)\n",
			  iC_fullProgname, iC_path, iC_progname, RASPBERRYPI/1000, RASPBERRYPI%1000);
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
    }
    /********************************************************************
     *  if argc != 0 then -R and argv points to auxialliary app + arguments
     *               do not fork and execute aux app until this app has
     *               connected to iCserver and registered all its I/Os
     *******************************************************************/
    directFlag = 0;
    if (! errorFlag) {
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
	/********************************************************************
	 *  Analyze IEC arguments X0 X1 IX2 QX3 etc or groups X4-X7
	 *
	 *  Match GPIOs to IEC arguments in the command line.
	 *******************************************************************/
	for (argp = argip; argp < &argip[argic]; argp++) {
	    ieStart = ieEnd = 0;
	    if (strlen(*argp) >= 128) {
		fprintf(iC_errFP, "ERROR: %s: command line argument too long: '%s'\n", iC_progname, *argp);
		if (writeUnlockCloseGpios() < 0) {
		    fprintf(iC_errFP, "ERROR: %s: in writeUnlockCloseGpios()\n", iC_progname);
		}
		iC_quit(SIGUSR1);		/* error quit */
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
	    /********************************************************************
	     *  Scan for an optional I or Q in IEC arguments IXx or QXx
	     *******************************************************************/
	    if (sscanf(tail, "%1[IQ]%127s", iqc, tail) == 2) {
		iqStart = iqEnd = (*iqc == 'I') ? 1 : 0;	/* 0 for 'Q', 1 for 'I' */
	    } else {
		*iqc    = '\0';				/* no I or Q IEC */
		iqStart = 0;				/* or lone I or Q - trapped in next sscanf */
		iqEnd   = 1;
	    }
	    /********************************************************************
	     *  Scan for a mandatory Xx in IEC arguments Xx IXx or QXx
	     *******************************************************************/
	    if ((n = sscanf(tail, "%1[X]%5d%127s", xd, &ieStart, tail)) == 1) {
		errorFlag++;
		goto endOneArg;
	    }
	    if (n >= 2 && *xd == 'X') {
		ieEnd = ieStart;			/* assume a single Xn IEC */
		*iqe = *iqc;
		if (n == 2) {
		    *tail = '\0';			/* single IEC argument without -<i> or */
		    errorFlag++;
		    goto endOneArg;
		}
		if (*tail == '-') {			/* range or instance alternatives start with '-' */
		    /********************************************************************
		     *  IEC arguments with optional range
		     *******************************************************************/
		    if ((n = sscanf(tail, "-%1[IQ]%126s", iqe, tail+1)) == 2 && *iqc != *iqe) {	/* skip '-' */
			goto illFormed;			/* fails if first Xn did not have I or Q */
		    }
		    if ((n = sscanf(tail+1, "X%5d%127s", &ieEnd, tail)) == 0) {
			ieEnd = ieStart;		/* a single Xn-i IEC with -instance */
			errorFlag++;
			goto endOneArg;
		    }
		    if (ieEnd <= ieStart) {		/* *tail remains '-' */
			fprintf(iC_errFP, "ERROR: %s: '%s' is not a positive IEC range\n", iC_progname, *argp);
			errorFlag++;
			goto endOneArg;
		    }
		    if (n == 1) {
			*tail = '\0';			/* a range Xn-Xm without instance */
			errorFlag++;
			goto endOneArg;
		    }
		    if (*tail == '-') {
			errorFlag++;
			goto endOneArg;
		    }
		}
		/********************************************************************
		 *  GPIO IEC arguments with .<bit>,gpio or ,gpio,gpio,...
		 *******************************************************************/
		if (iqStart != iqEnd || ieStart != ieEnd) {
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
		    directFlag = 1;
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
	     *  Use the IEC argument to extend GPIO control structures
	     *  Check IEC argument was not used before
	     *******************************************************************/
	    if (directFlag) {
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
		*gpioLast = gep = iC_emalloc(sizeof(gpioIO));	/* new gpioIO element */
		gep->Gname = NULL;		/* GPIO IEC in or output not defined */
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
	/* UnlockGpios */
	if (iC_debug & 0200) fprintf(iC_outFP, "used     = 0x%016llx\n"
					       "oops     = 0x%016llx\n", gpiosp->u.used, gpiosp->u.oops);
	if (writeUnlockCloseGpios() < 0) {
	    fprintf(iC_errFP, "ERROR: %s: in writeUnlockCloseGpios()\n", iC_progname);
	    errorFlag++;
	}
    }
  FreeNow:
    free(argip);
    if (errorFlag) {
	iC_quit(-3);					/* after all the command line has been analyzed */
    }
    if (! directFlag) {
	fprintf(iC_errFP, "ERROR: %s: no IEC arguments? there must be at least 1 GPIO argument\n", iC_progname);
	iC_quit(-4);					/* call termQuit() to terminate I/O */
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
     *  Clear val and channel members in  iC_gpL[0] and iC_gpL[1]
     *******************************************************************/
    for (iq = 0; iq < 2; iq++) {
	for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
	    assert(gep->Gname);		/* later scans rely on a name */
	    gep->Ginv ^= invMask;	/* by default do not invert GPIO in or outputs - inverted with -I or '~' */
	    gep->Gval = 0;		/* restore values */
	    gep->Gchannel = 0;
	}
    }
#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */
    /********************************************************************
     *  open /dev/gpiochipX device for control of gpio pins via ioctl().
     *******************************************************************/
    if ((chipFN = open (GPIOCHIP, O_RDONLY)) < 0) {
	perror ("open-GPIOCHIP");
    }
    if (chipFN > iC_maxFN) {
	iC_maxFN = chipFN;
    }
    pins.fd = chipFN;
    pins.linereq->num_lines = 0;
    idx = 0;				/* index into linereq offsets array */
    maskBit = 1;			/* bit in 64 bit masks displaced by idx */
    /********************************************************************
     *  Provide separate attributes for write invertedpins as well as
     * read normal and inverted pins to catch both edges
     *******************************************************************/
    wi_attr.id =	GPIO_V2_LINE_ATTR_ID_FLAGS;
    wi_attr.flags =	GPIO_V2_LINE_FLAG_OUTPUT        |
			GPIO_V2_LINE_FLAG_ACTIVE_LOW    |
			GPIO_V2_LINE_FLAG_BIAS_PULL_UP;

    rd_attr.id =	GPIO_V2_LINE_ATTR_ID_FLAGS;
    rd_attr.flags =	GPIO_V2_LINE_FLAG_INPUT         |	/* active high by default */
			GPIO_V2_LINE_FLAG_EDGE_RISING   |
			GPIO_V2_LINE_FLAG_EDGE_FALLING  |
			GPIO_V2_LINE_FLAG_BIAS_PULL_DOWN;

    ri_attr.id =	GPIO_V2_LINE_ATTR_ID_FLAGS;
    ri_attr.flags =	GPIO_V2_LINE_FLAG_INPUT         |
			GPIO_V2_LINE_FLAG_ACTIVE_LOW    |
			GPIO_V2_LINE_FLAG_EDGE_RISING   |
			GPIO_V2_LINE_FLAG_EDGE_FALLING  |
			GPIO_V2_LINE_FLAG_BIAS_PULL_UP;

    if (opt_D) {
	rb_attr.id =	GPIO_V2_LINE_ATTR_ID_DEBOUNCE;
	rb_attr.debounce_period_us = opt_D;		/* microseonds debounce */
    }
    /********************************************************************
     *  gpio_v2 attribute config for write inverted pins as well as
     *  read normal and inverted pins
     *******************************************************************/
    wi_cfg_attr.attr = wi_attr;
    rd_cfg_attr.attr = rd_attr;
    ri_cfg_attr.attr = ri_attr;
    if (opt_D) {
	rb_cfg_attr.attr = rb_attr;
    }
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
    /********************************************************************
     *  Export and open all gpio files for all GPIO arguments
     *  open /sys/class/gpio/gpioN/value permanently for obtaining
     *  out-of-band interrupts and to allow read and write operations
     *******************************************************************/
    for (iq = 0; iq < 2; iq++) {
	for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
	    for (bit = 0; bit <= 7; bit++) {
		if ((gpio = gep->gpioNr[bit]) != 0xffff) {
#if RASPBERRYPI < 5010	/* sysfs */
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
			 *  Execute the SUID root progran iC_gpio_pud(gpio, pud) to set pull-up/down
			 *  to 3.3 volt/0 volt. It is not recommended to connect a GPIO input to 5 volt,
			 *  although I have never had a problem doing so. JW 2023-07-04
			 *  This requires the following to obtain correct initialisation of inputs:
			 *    for normal   input (logic 0 = low)  set pull down pud = 1 BCM2835_GPIO_PUD_DOWN
			 *    for inverted input (logic 0 = high) set pull up   pud = 2 BCM2835_GPIO_PUD_UP
			 *******************************************************************/
			iC_gpio_pud(gpio, gep->Ginv ? BCM2835_GPIO_PUD_UP : BCM2835_GPIO_PUD_DOWN);
		    }
		    if (gep->gpioFN[bit] > iC_maxFN) {
			iC_maxFN = gep->gpioFN[bit];
		    }
		    if (iC_debug & 0200) fprintf(iC_outFP, "configure %c%s.%hu,%hu\n",
			gep->Ginv  ? '~' : ' ', gep->Gname, bit, gpio);
#else	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
		    /********************************************************************
		     *  Configure gpio line request and line configurations
		     *  for all GPIO arguments
		     * gpio_v2 line request for lines (pins)
		     *******************************************************************/
		    pins.linereq->offsets[idx] = gpio;
		    gep->gpioFN[bit] = idx;			/* store offsets[index] in gep for this bit */
		    if (iq == 0) {
			/********************************************************************
			 *  QXn gpio N configure with direction out and no interrupts
			 *******************************************************************/
			if (gep->Ginv & iC_bitMask[bit]) {
			    wi_cfg_attr.mask |= maskBit;	/* inverted output */
			}
		    } else {
			/********************************************************************
			 *  IXn gpio N configure with direction in and interrupts on both edges
			 *******************************************************************/
			assert(gpio < GPIO_LIMIT);
			gpioArray[gpio].gep = gep;
			gpioArray[gpio].val = bit;
			if (gep->Ginv & iC_bitMask[bit]) {
			    ri_cfg_attr.mask |= maskBit;	/* inverted input */
			} else {
			    rd_cfg_attr.mask |= maskBit;	/* normal input */
			}
			if (opt_D) {
			    rb_cfg_attr.mask |= maskBit;	/* both with debounce */
			}
		    }
		    if (iC_debug & 0200) fprintf(iC_outFP, "configure %c%s.%hu,%hu, offsets[%d] maskBit = 0x%llx\n",
			gep->Ginv & iC_bitMask[bit] ? '~' : ' ', gep->Gname, bit, gpio, idx, maskBit);
		    idx++;
		    maskBit <<= 1;
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
		} else if (iq == 1) {
		    gep->Ginv &= ~iC_bitMask[bit];	/* blank out missing GPIO input bits IXx.y */
		}
	    }
	}
    }
#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */
    /********************************************************************
     *  gpio_v2 line lines (pins) configuration
     *******************************************************************/
    i = 0;
    pins.linecfg->attrs[i++] = wi_cfg_attr;	/* assign write inverted attr to linecfg array */
    pins.linecfg->attrs[i++] = rd_cfg_attr;	/* assign read attr to linecfg array */
    pins.linecfg->attrs[i++] = ri_cfg_attr;	/* assign read inverted attr to linecfg array */
    if (opt_D) {
	pins.linecfg->attrs[i++] = rb_cfg_attr;	/* assign read de-bounce attr to linecfg array */
    }
    pins.linecfg->num_attrs = i;		/* for write and read pin and debounce attributes */
    pins.linereq->num_lines = idx;
    /********************************************************************
     *  Set line (pin) configuration
     *  Generate anonymous file descriptor
     *******************************************************************/
    if (gpio_line_cfg_ioctl (&pins) == -1) {
	return 1;
    }
    if (pins.linereq->fd > iC_maxFN) {
	iC_maxFN = pins.linereq->fd;
    }
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
    /********************************************************************
     *  Generate a meaningful name for network registration
     *******************************************************************/
    len = snprintf(buffer, BS, "%s", iC_iccNM);
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
		storeUnit(channel, gep);	/* link GPIO element pointer to send channel */
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
     *******************************************************************/
    if (iC_debug & 0300) {
	if (gpioCnt) {
	    char *	iidPtr;
	    char *	iidSep;
	    fprintf(iC_outFP, "Allocation for %d GPIO group%s, global instance = \"%s\"\n",
		gpioCnt, gpioCnt == 1 ? "" : "s", iC_iidNM);
	    fprintf(iC_outFP, "	IEC bit	inst	gpio	iC channel\n\n");
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
			    fprintf(iC_outFP, "	%c%s.%hu	%s%s	%3hu	%5hu\n",
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
     *  Extend with GPIO IXn initialisation inputs
     *  There may be no GPIOs or only GPIO outputs - nothing to initialise
     *******************************************************************/
    for (gep = iC_gpL[1]; gep; gep = gep->nextIO) {	/* IXn inputs only */
	val = 0;
	for (bit = 0; bit <= 7; bit++) {
	    if ((gpio = gep->gpioNr[bit]) != 0xffff) {
#if RASPBERRYPI < 5010	/* sysfs */
		if ((n = gpio_read(gep->gpioFN[bit])) != -1) {
		    if (iC_debug & 0200) fprintf(iC_outFP, "Initial value: %c%s.%hu,%hu = %d\n",
			gep->Ginv  ? '~' : ' ', gep->Gname, bit, gpio, n);
		    if (n) val |= iC_bitMask[bit];
		} else {
		    fprintf(iC_errFP, "WARNING: %s: GPIO %hu returns invalid value -1 (not 0 or 1 !!)\n",
			iC_progname, gpio);		/* should not happen */
		}
#else	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
		__u64 inMask = 1LL << gep->gpioFN[bit];	/* shift with idx in gep->gpioFN[bit] */
		n = gpio_line_get_values (&pins, inMask);
		if (iC_debug & 0200) fprintf(iC_outFP, "Initial value: %c%s.%hu,%hu = %d\n",
		    gep->Ginv  ? '~' : ' ', gep->Gname, bit, gpio, n);
		if (n) val |= iC_bitMask[bit];
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
	    }
	}
	assert(gep->Gname);
	assert(regBufLen > 11);			/* fits largest data telegram */
#if RASPBERRYPI < 5010	/* sysfs */
	/* by default do not invert GPIO inputs - they are inverted with -I */
	len = snprintf(cp, regBufLen, ",%hu:%d", gep->Gchannel, val ^ gep->Ginv);	/* data telegram */
#else	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
	/* in V2 ABI for ioctl val is active in bit which must not be inverted again */
	len = snprintf(cp, regBufLen, ",%hu:%d", gep->Gchannel, val);			/* data telegram */
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
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
	if (iC_debug & 0100) fprintf(iC_outFP, "G: %s:	%s	<%s\n", iC_iccNM, regBuf+1, buffer);
    }
    /********************************************************************
     *  Write GPIO QXn initialisation outputs
     *******************************************************************/
    for (gep = iC_gpL[0]; gep; gep = gep->nextIO) {	/* QXn outputs only */
	val = 0;				/* initialise with logical 0 */
	gep->Gval = gep->Ginv ^ 0xff;		/* force all output bits */
	writeGPIO(gep, gep->Gchannel, val);	/* some bits may not have a GPIO pin */
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
    FD_SET(iC_sockFN, &infds);			/* watch TCP/IP socket for inputs */
#if RASPBERRYPI < 5010	/* sysfs */
    FD_ZERO(&ixfds);
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
#else	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
    FD_SET(pins.linereq->fd, &infds);				/* watch all GPIO's for interrupts */
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
    if ((iC_debug & DZ) == 0) FD_SET(0, &infds);	/* watch stdin for inputs unless - FD_CLR on EOF */
    if (iC_debug & 0200) fprintf(iC_outFP, "iC_sockFN = %d\n", iC_sockFN);
    /********************************************************************
     *  External input (TCP/IP via socket, GPIO and STDIN)
     *  Wait for input in a select statement most of the time
     *******************************************************************/
    for (;;) {
#if RASPBERRYPI < 5010	/* sysfs */
	if ((retval = iC_wait_for_next_event(&infds, &ixfds, NULL)) > 0)
#else	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
	if ((retval = iC_wait_for_next_event(&infds, NULL, NULL)) > 0)
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
	{
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
				/********************************************************************
				 *  TCP/IP output for a GPIO
				 *******************************************************************/
				writeGPIO(Units[channel], channel, val);
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
	     *  GPIO N interrupt means GPIO n input
	     *******************************************************************/
#if RASPBERRYPI < 5010	/* sysfs */
	    m = 0;
#else	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
	    if (! (FD_ISSET(pins.linereq->fd, &iC_rdfds))) goto END_GPIO_INTERRUPT;
	    if ((m = read(pins.linereq->fd, lineevent, sizeof lineevent[0]*LINEEVENT_BUFFERS)) == -1) {
		perror ("read - lineevent");
		goto END_GPIO_INTERRUPT;
	    }
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
	    cp = regBuf;
	    regBufLen = REPLY;
	    if (iC_debug & 0100) {
		op = buffer;
		ol = BS;
	    }
#if RASPBERRYPI < 5010	/* sysfs */
	    for (gep = iC_gpL[1]; gep; gep = gep->nextIO) {	/* IXn inputs only */
		assert(regBufLen > 11);		/* fits largest data telegram */
		val = gep->Gval;
		for (bit = 0; bit <= 7; bit++) {
		    int	fd;
		    if ((gpio = gep->gpioNr[bit]) != 0xffff) {
			fd = gep->gpioFN[bit];	/* file decriptor for sysfs read or offsets(index] for ioctl */
			assert(fd >= 0);
			if (FD_ISSET(fd, &iC_exfds)) {	/* any out-of-band GPIO N input */
			    if ((n = gpio_read(fd)) == 0) {
				val &= ~(1 << bit);
			    } else if (n == 1) {
				val |= 1 << bit;
			    } else {
				fprintf(iC_errFP, "WARNING: %s: GPIO %hu returns invalid value %d (not 0 or 1 !!)\n",
				    iC_progname, gpio, n);		/* should not happen */
			    }
#if YYDEBUG && !defined(_WINDOWS)
			    if (m++ == 0 && iC_micro) iC_microPrint("GPIO input received", 0);
#endif	/* YYDEBUG && !defined(_WINDOWS) */
			}
		    }
		}
		if (val != gep->Gval) {
		    gep->Gval = val;		/* store change for comparison */
		    /* by default do not invert GPIO inputs - they are inverted with -I */
		    val ^= gep->Ginv;
		    len = snprintf(cp, regBufLen, ",%hu:%d", gep->Gchannel, val);	/* data telegram */
		    cp += len;
		    regBufLen -= len;
		    if (iC_debug & 0100) {
			len = snprintf(op, ol, " %s", gep->Gname);	/* source name */
			op += len;
			ol -= len;
		    }
		}
	    }	/*  end of sysfs GPIO N interrupt */
#else	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
	    m /= sizeof lineevent[0];		/* number of lineevent buffers read */
	    if (iC_debug & 0200) {
		fprintf(iC_outFP, "ioctl GPIO %d interrupts:", m);
		for (n = 0; n < m; n++) {
		    gpio = lineevent[n].offset;	/* GPIO number from offset */
		    assert(gpio < GPIO_LIMIT && gpioArray[gpio].gep);
		    gep = gpioArray[gpio].gep;
		    bit = gpioArray[gpio].val;	/* is actually the bit */
		    val = (lineevent[n].id == GPIO_V2_LINE_EVENT_RISING_EDGE) ? 1 :
			  (lineevent[n].id == GPIO_V2_LINE_EVENT_FALLING_EDGE) ? 0 : -1;
		    fprintf(iC_outFP, " [%d] %s.%d,%d(%d) = %d", n, gep->Gname, bit, gpio, gep->Gchannel, val);
		}
		fprintf(iC_outFP, "\n");
	    }
	    j = 0;
	    for (n = 0; n < m; n++) {
		gpio = lineevent[n].offset;	/* GPIO number from offset */
		gep = gpioArray[gpio].gep;
		bit = gpioArray[gpio].val;	/* is actually the bit */
		for (i = 0; i < j; i++) {
		    if (gep == gepArray[i].gep) {
			val = gepArray[i].val;	/* restore bits for this IEC */
			goto OLD_GEP;
		    }
		}
		gepArray[i].gep = gep;		/* a new IEC with it's own gep */
		val = gep->Gval;
		j++;
		assert(j <= LINEEVENT_BUFFERS);
	      OLD_GEP:
		if (lineevent[n].id == GPIO_V2_LINE_EVENT_RISING_EDGE) {
		    val |= 1 << bit;		/* set bit for this IEC */
		} else if (lineevent[n].id == GPIO_V2_LINE_EVENT_FALLING_EDGE) {
		    val &= ~(1 << bit);		/* clear bit for this IEC */
		} else {
		    fprintf(iC_errFP, "WARNING: %s: GPIO %hu input interrupt returns neither rising or falling edge !!)\n",
			iC_progname, gpio);		/* should not happen */
		}
		gepArray[i].val = val;		/* save modified bits for this IEC */
	    }
	    for (i = 0; i < j; i++) {
		gep = gepArray[i].gep;
		val = gepArray[i].val;
		if (val != gep->Gval) {		/* bit in val may have been set and cleared by different interrupts */
		    gep->Gval = val;		/* ready for next interrupts */
		    len = snprintf(cp, regBufLen, ",%hu:%d", gep->Gchannel, val);	/* data telegram */
		    cp += len;
		    regBufLen -= len;
		    if (iC_debug & 0100) {
			len = snprintf(op, ol, " %s", gep->Gname);	/* source name */
			op += len;
			ol -= len;
		    }
		}
	    }
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
	    if (cp > regBuf) {
		iC_send_msg_to_server(iC_sockFN, regBuf+1);	/* send data telegram(s) to iCserver */
		if (iC_debug & 0100) fprintf(iC_outFP, "G: %s:	%s	<%s\n", iC_iccNM, regBuf+1, buffer);
	    }
#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */
	  END_GPIO_INTERRUPT:;
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
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
		} else if (c != '\n') {
		    fprintf(iC_errFP, "no action coded for '%c' - try t, m, T, i, I, or q followed by ENTER\n", c);
		}
	    }	/*  end of STDIN interrupt */
	} else {
	    perror("ERROR: select failed");		/* retval <= 0 */
	    iC_quit(SIGUSR1);				/* error quit */
	}
    }
} /* main */

/********************************************************************
 *
 *	Initalise and expand dynamic array Units[] as necessary
 *	Store gpioIO * to current GPIO element
 *
 *******************************************************************/

static void
storeUnit(unsigned short channel, gpioIO * gep)
{
    while (channel >= ioUnits) {
	Units = (gpioIO * *)realloc(Units,		/* initially NULL */
	    (ioUnits + IOUNITS) * sizeof(gpioIO *));
	assert(Units);
	memset(&Units[ioUnits], '\0', IOUNITS * sizeof(gpioIO *));
	ioUnits += IOUNITS;			/* increase the size of the array */
	if (iC_debug & 0200) fprintf(iC_outFP, "storeUnit: Units[%d] increase\n", ioUnits);
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "storeUnit: Units[%d] <= %s\n", channel, gep->Gname);
    Units[channel] = gep;			/* store gpioIO * */
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
#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */
    __u64		om;
    __u64		outMask = 0;
    __u64		outBit = 0;
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */

    assert(gep && gep->Gname && *gep->Gname == 'Q');	/* make sure this is really a GPIO output */
    if (iC_debug & 0100) fprintf(iC_outFP, "G: %s:	%hu:%d	> %s\n",
	iC_iccNM, channel, (int)val, gep->Gname);
#if RASPBERRYPI < 5010	/* sysfs */
    val ^= gep->Ginv;			/* normally write non-inverted data to GPIO output */
#endif	/* RASPBERRYPI < 5010 - sysfs */
    diff = val ^ gep->Gval;		/* bits which are going to change */
    assert ((diff & ~0xff) == 0);	/* may receive a message which involves no change */
    while (diff) {
	bit = iC_bitIndex[diff];	/* returns 0 - 7 for values 1 - 255 (avoid 0) */
	mask  = iC_bitMask[bit];	/* returns hex 01 02 04 08 10 20 40 80 */
	if ((fd = gep->gpioFN[bit]) != -1) {	/* is GPIO pin open ? */
#if RASPBERRYPI < 5010	/* sysfs */
	    gpio_write(fd, val & mask);	/* yes - write to GPIO (does not need to be a bit in pos 0) */
#else	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
	    om = 1LL << fd;			/* fd = index into offsets array */
	    outMask |= om;			/* bit to be written in this call (may be more than one) */
	    outBit |= (val & mask) ?  om : 0;	/* write GPIO bit active or non-active */
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
	} else if ((iC_debug & 0300) == 0300) {
		fprintf(iC_errFP, "WARNING: %s: no GPIO associated with %s.%hu\n",
		iC_progname, gep->Gname, bit);
	}
	diff &= ~mask;			/* clear the bit just processed */
    }
#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */
    if (outMask) {
	if (iC_debug & 0200) fprintf(iC_outFP, "writeGPIO: output %s outBit = %llu outMask = %llu\n",
	    gep->Gname, outBit, outMask);
	if (gpio_line_set_values (&pins, outBit, outMask) == -1) {
	    fprintf(iC_errFP, "ERROR: %s: writeGPIO: could not output %s outBit = %llu outMask = %llu\n",
		iC_progname, gep->Gname, outBit, outMask);
	}
    }
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
    gep->Gval = val;			/* ready for next output */
} /* writeGPIO */
#if RASPBERRYPI >= 5010	/* GPIO V2 ABI for icoctl */

/********************************************************************
 *  Configure ioctl for gpio control using ABI V2 access.
 *  @param gpio gpio v2_struct holding pointers to linecfg and linereq
 *  structs and open file descriptor returned by prior call to
 *  gpio_dev_open()..
 *  linecfg - v2 ioctl config struct for gpio pins (lines).
 *  linereq - v2 ioctl request struct member of linecfg containing
 *  separate file-descriptor used for read/write access of configured
 *  gpio pins (lines).
 *  @return returns 0 on success, -1 otherwise.
 *******************************************************************/

static int
gpio_line_cfg_ioctl (gpio_v2_t * gpio)
{
    /********************************************************************
     * get ioctl values for line request
     *******************************************************************/
    if (ioctl (gpio->fd, GPIO_V2_GET_LINE_IOCTL, gpio->linereq) < 0) {
	perror ("ioctl-GPIO_V2_GET_LINE_IOCTL");
	return -1;
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "++++ gpio->fd = %d\n", gpio->fd);
    /********************************************************************
     * set the line config for the retured linereq file descriptor
     *******************************************************************/
    if (ioctl (gpio->linereq->fd, GPIO_V2_LINE_SET_CONFIG_IOCTL,
      gpio->linecfg) < 0) {
	perror ("ioctl-GPIO_V2_LINE_SET_CONFIG_IOCTL");
	return -1;
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "++++ gpio->linereq->fd = %d\n", gpio->linereq->fd);
    return 0;
} /* gpio_line_cfg_ioctl */

/********************************************************************
 *  Write gpio pin (line) values (HI/LO) from bits set or clearned
 *  in gpio->linevals->bits for pins with bit set high in gpio->linevals->mask
 *  from mask using gpio_v2 ioct line request.
 *  @param linereq pointer to gpio_v2_line_request struct holding linereq with
 *  open linereq file descriptor set by prior call to gpio_line_cfg_ioctl()
 *  used to write linevals to gpio pin index(s) in linereq->offsets specified
 *  by bits HI in mask.
 *  @param bits gpio value to write (0 - LO, 1 - HI) to set bit in
 *  linereq->bits for bits specified in mask.
 *  @param mask bitmap with bits 1 (HI) that correspond to index in
 *  gpio->linereq->offsets pin array that will be set.
 *  @return returns 0 on success, -1 otherwise.
 *******************************************************************/

static int
gpio_line_set_values (gpio_v2_t * gpio, __u64 bits, __u64 mask)
{
    gpio->linevals->bits = bits;		/* new line */
    gpio->linevals->mask = mask;		/* set linevals mask to mask */
    /********************************************************************
     *  set GPIO pin value to bit in lineval->bits (0 or 1) for pins with
     *  bit == 1 in mask.
     *******************************************************************/
    if (ioctl (gpio->linereq->fd, GPIO_V2_LINE_SET_VALUES_IOCTL, gpio->linevals) < 0) {
	perror ("ioctl-GPIO_V2_LINE_SET_VALUES_IOCTL-1");
	return -1;
    }
    return 0;
} /* gpio_line_set_values */

/********************************************************************
 *  Read gpio pin (line) values (HI/LO) from bits set or clearned
 *  in gpio->linevals->bits for pins with bit set high in gpio->linevals->mask
 *  from mask using gpio_v2 ioct line request.
 *  @param linereq pointer to gpio_v2_line_request struct holding linereq with
 *  open linereq file descriptor set by prior call to gpio_line_cfg_ioctl()
 *  used to read linevals to gpio pin index(s) in linereq->offsets specified
 *  by bits HI in mask.
 *  @param mask bitmap with bits 1 (HI) that correspond to index in
 *  gpio->linereq->offsets pin array that will be set.
 *  @return returns bit value 0 or 1 on success, -1 otherwise.
 *******************************************************************/

static int
gpio_line_get_values (gpio_v2_t * gpio, __u64 mask)
{
    /********************************************************************
     *  get GPIO pin value to bit in lineval->bits (0 or 1) for pins with
     *  bit == 1 in mask.
     *******************************************************************/
    struct gpio_v2_line_values * data = gpio->linevals;
    data->mask = mask;				/* set linevals mask to mask */
    if (ioctl(gpio->linereq->fd, GPIO_V2_LINE_GET_VALUES_IOCTL, data) < 0) {
	perror ("ioctl-GPIO_V2_LINE_GET_VALUES_IOCTL-1");
	return -1;
    }
    return (data->bits & mask) ? 1 : 0;
} /* gpio_line_get_values */

/********************************************************************
 * Closes the open line request file descriptor for v2 linereq.
 * @param gpio gpio v2 data struct holding line request struct with open
 * linereq file descriptor opened by prior call to gpio_line_cfg_ioctl().
 * @return returns 0 on success, -1 otherwise.
 *******************************************************************/

static int
gpio_line_close_fd (gpio_v2_t *gpio)
{
  if (close (gpio->linereq->fd) < 0) {    /* close linereq fd */
    perror ("close-linereq.fd");
    return -1;
  }
  return 0;
}

/********************************************************************
 * Closes gpio file descriptor associated with "/dev/gpiochipX"
 * @param gpiofd file descriptor retured by previos call to gpio_dev_open().
 * @return returns 0 on success, -1 otherwise.
 *******************************************************************/

static int
gpio_dev_close (int gpiofd)
{
  if (close (gpiofd) < 0) {         /* close gpiochipX fd */
    perror ("close-gpiofd");
    return -1;
  }
  return 0;
}
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */

/********************************************************************
 *
 *	Unexport and close GPIOs
 *	Clear gpiosp->u.used bits for GPIOs used in this app
 *
 *******************************************************************/

static int
termQuit(int sig)
{
    gpioIO *		gep;
    ProcValidUsed *	gpiosp;
#if RASPBERRYPI < 5010	/* sysfs */
    if (iC_debug & 0200) fprintf(iC_outFP, "=== Unexport GPIOs =======\n");
    int			iq;
    unsigned short	bit;
    unsigned short	gpio;
    int		fn;
    /********************************************************************
     *  Unexport and close all gpio files for all GPIO arguments
     *******************************************************************/
    for (iq = 0; iq < 2; iq++) {
	for (gep = iC_gpL[iq]; gep; gep = gep->nextIO) {
	    for (bit = 0; bit <= 7; bit++) {
		if ((gpio = gep->gpioNr[bit]) != 0xffff) {
		    /********************************************************************
		     *  Execute the SUID root progran iC_gpio_pud(gpio, pud) to turn off pull-up/down
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
#else	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
    /********************************************************************
     *  Write GPIO QXn termination outputs
     *******************************************************************/
    for (gep = iC_gpL[0]; gep; gep = gep->nextIO) {	/* QXn outputs only */
	writeGPIO(gep, gep->Gchannel, 0);	/* some bits may not have a GPIO pin */
    }
    /********************************************************************
     *  Close gpio file descriptor associated with anonymous llinereq->fd
     *  and "/dev/gpiochipX"
     *******************************************************************/
    if (iC_debug & 0200) fprintf(iC_outFP, "=== Close chip and GPIOs =======\n");
    gpio_line_close_fd (&pins);
    gpio_dev_close (chipFN);
#endif	/* RASPBERRYPI >= 5010 - GPIO V2 ABI for icoctl */
    /********************************************************************
     *  Open and lock the auxiliary file ~/.iC/gpios.used again
     *  Other apps may have set used bits since this app was started
     *******************************************************************/
    if ((gpiosp = openLockGpios(0)) == NULL) {
	fprintf(iC_errFP, "ERROR: %s: in openLockGpios()\n",
	    iC_progname);
	return (SIGUSR1);		/* error quit */
    }
    if ((iC_debug & 0200) != 0) fprintf(iC_outFP, "### %s: openLock, used = 0x%016llx, ownUsed = 0x%016llx\n", iC_progname, gpiosp->u.used, ownUsed);
    gpiosp->u.used &= ~ownUsed;		/* clear all bits for GPIOs and A/D channels used in this app */
    if ((iC_debug & 0200) != 0) fprintf(iC_outFP, "### %s: writeUnlockCloseGpios, used = 0x%016llx\n", iC_progname, gpiosp->u.used);
    if (writeUnlockCloseGpios() < 0) {	/* unlink (remove) ~/.iC/gpios.used if gpios->u.used and oops is 0 */
	fprintf(iC_errFP, "ERROR: %s: in writeUnlockCloseGpios()\n",
	    iC_progname);
	return (SIGUSR1);		/* error quit */
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "=== End GPIOs ===\n");
    return (sig);			/* finally quit */
} /* termQuit */
#endif	/* defined(RASPBERRYPI) && defined(TCP) && !defined(_WINDOWS) */

/* ############ POD to generate iCpiGPIO man page ############################

=encoding utf8

=head1 NAME

iCpiGPIO - real digital I/O on a Raspberry Pi for the iC environment

=head1 SYNOPSIS

 iCpiGPIO [-BIftmqzh][ -s <host>][ -p <port>][ -n <name>][ -i <inst>]
          [ -D <db>]
          [ -W <GPIO_number>][ -d <deb>]
          [ [~]IXn,<gpio>[,<gpio>,...][-<inst>] ...]
          [ [~]QXn,<gpio>[,<gpio>,...][-<inst>] ...]
          [ [~]IXn.<bit>,<gpio>[-<inst>] ...]
          [ [~]QXn.<bit>,<gpio>[-<inst>] ...]      # at least 1 IEC argument
          [ -R <aux_app>[ <aux_app_argument> ...]] # must be last arguments
    -s host IP address of server    (default '127.0.0.1')
    -p port service port of server  (default '8778')
    -i inst instance of this client (default '') or 1 to 3 digits
    -B      start iCbox -d to monitor active GPIO I/O
    -I      invert all GPIO inputs and outputs
            GPIO input circuits are normally hi - 1 and when a switch on the
            input is pressed it goes lo - 0. Therefore it is appropriate to
            invert inputs and outputs. When inverted a switch pressed on an
            input generates a 1 for the IEC inputs and a 1 on an IEC output
            turns a LED and relay on, which is natural.
          Debounce is only available for Raspberry Pi OS 'bullseye' and above.
    -D db   microsecend debounce for GPIO inputs (default 0 - no debounce)
    -W GPIO number used by the w1-gpio kernel module (default 4, maximum 31).
            When the GPIO with this number is used in this app, iCtherm is
            permanently blocked to avoid Oops errors in module w1-gpio.
    -f      force use of all interrupting gpio inputs.
            some programs do not unexport correctly.

                      GPIO arguments
          Any IEC I/Os IXn.y and QXn.y which are to be linked to GPIO
          inputs or outputs must be named in the argument list as follows:
    IXn,<gpio>[,<gpio>,...]
    QXn,<gpio>[,<gpio>,...]
          Associate the bits of a particular input or output IEC
          with a list of gpio numbers.  Up to 8 gpio numbers can
          be given in a comma separated list. The first gpio number
          will be aassociated with bit 0, the second with bit 1 etc
          and the eighth with bit 7. If the list is shorter than
          8 the trailing bits are not used. The letter 'd' can be
          used in the list instead of a gpio number to mark that bit
          as unused.  eg. IX3,17,18,19,20,d,d,21 associates IX3.0 to
          IX3.3 with GPIO17 to GPIO20 and IX3.6 with GPIO21.
          Alternatively:
    IXn.<bit>,<gpio>
    QXn.<bit>,<gpio>
          Associate a single input or output <bit> 0 to 7 with one
          gpio nunber eg. IX3.0,17 QX3.7,18
    ~IEC  Each lone IEC or leading IEC of a range may be preceded by a
          tilde '~', which inverts lone IEC inputs or outputs or all
          members of a range.  If all IEC inputs and outputs are already
          inverted with -I, then ~ inverts them again, which makes them
          non-inverted. Individual bits for one IEC can be inverted
          or left normal for GPIO IEC's by declaring the IEC more than
          once with non-overlapping gpio bit lists or single bits.
    IEC-inst Each individual IEC or range can be followed by -<inst>,
          where <inst> consists of 1 to 3 numeric digits.
          Such an appended instance code takes precedence over the
          general instance specified with the -i <inst> option above.
    For a full Description and Technical Background of the 'sysfs' GPIO
    driver software see the iCpiFace man page.

                      CAVEAT
    There must be at least one GPIO IEC argument. No IEC
    arguments are generated automatically for %s.
    Several instances of this program may be run as long as they are different
    instances (-i option) as well as iCpiFace and iCpiI2C. These different
    programs may not share a GPIO number and IEC's must not clash

                      DEBUG options
    -t      trace arguments and activity (equivalent to -d100)
         t  at run time toggles gate activity trace
    -m      display microsecond timing info
         m  at run time toggles microsecond timing trace
    -d deb  +1   trace TCP/IP send actions
            +2   trace TCP/IP rcv  actions
            +100 show arguments
            +200 show more debugging details
            +400 exit after initialisation
    -q      quiet - do not report clients connecting and disconnecting
    -z      block keyboard input on this app - used by -R
    -h      this help text
         T  at run time displays registrations and equivalences
         q  or ctrl+D  at run time stops %s

                      AUXILIARY app
    -R <app ...> run auxiliary app followed by -z and its arguments
                 as a separate process; -R ... must be last arguments.

=head1 DESCRIPTION

B<iCpiGPIO> is an I/O client for iCserver in the immediate C
environment of a Raspberry Pi handling real bit inputs and bit
outputs for a number of direct GPIO pins on the Raspberry Pi.

All GPIO pins on a Raspberry Pi A, B or B+ or higher may be selected
as either an input bit or output bit, unless other device drivers
like iCpiFace use some of the GPIO pins.

=head1 TECHNICAL BACKGROUND

The direct GPIO connections on the Raspberry Pi:

    The Raspberry Pi A or B brings out 17 GPIO signals on a 26 pin
    connecteor, 5 of which double up to do the SPI interface, 2 for
    a UART, leaving 10 for general purpose input/output (there are
    4 more on a 2nd connector, which is not normally fitted). The
    Raspberry Pi B+ and later releases brings out 9 more GPIO signals
    on its 40 pin connector making a total of 19 free GPIO pins.
    
    The Linux 'sysfs' can access the value of these GPIO pins from user
    space and more importantly can generate interrupts from them.

    For RPi OS's with a Linux kernel version < 5.10  GPIO pin I/O is
    handled by the Linux 'sysfs' and its interrupts.
    Full details for 'sysfs' GPIO control is in iCpiFace(1).

    For RPi OS's with a Linux kernel version >= 5.10  GPIO pin I/O is
    handled by the Linux kernel interface called:
    'GPIO Character Device Userspace API V2'. https://docs.kernel.org/
    userspace-api/gpio/chardev.html#gpio-v2-line-request

=head1 AUTHOR

John E. Wulff

=head1 BUGS

Email bug reports to B<immediateC@gmail.com> with L<iC Project> in the
subject field.

=head1 SEE ALSO

L<iCpiFace(1)>, L<iCpiPWM(1)>, L<iCtherm(1)>, L<iCserver(1)>, L<iCbox(1)>,
L<select(2)>

=head1 COPYRIGHT

COPYRIGHT (C) 2014-2025  John E. Wulff

You may distribute under the terms of either the GNU General Public
License or the Artistic License, as specified in the README file.

For more information about this program, or for information on how
to contact the author, see the README file.

=cut

   .############ end of POD to generate iCpiGPIO man page ##################### */

/********************************************************************
 *
 *	Copyright (C) 2014  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	iCpiFace
 *	I/O client for iCserver in the immediate C environment of a
 *	Raspberry Pi handling real 8 bit inputs and 8 bit outputs from
 *	one or more  PiFace controllers attached to a Raspberry Pi
 *
 *	A maximum of 8 PiFace controllers can be handled altogether
 *	- either one plugged directly into a Raspberry Pi
 *	- or 4 on one PiRack - 7 on 2 PiRacks - 8 require 3 PiRacks
 *
 *	One of the PiFaces (at address 4) can be a PiFaceCAD, which
 *	has 8 button inputs and a 2 line x 16 character LCD display
 *
 *******************************************************************/

#include	"tcpc.h"
#include	<signal.h>
#include	<assert.h>
#include	<errno.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#ifdef	POLL 
#include	<poll.h>
#endif	/* POLL */

#if !defined(RASPBERRYPI) || !defined(TCP) || defined(_WINDOWS)
#error - must be compiled with RASPBERRYPI and TCP defined and not _WINDOWS
#else	/* defined(RASPBERRYPI) && defined(TCP) && !defined(_WINDOWS) */

#include	"icc.h"			/* declares iC_emalloc() in misc.c and iC_quit() */
#include	"rpi_gpio.h"
#include	"mcp23s17.h"
#ifdef	BCM2835
#include	"bcm2835.h"
#endif	/* BCM2835 */
#include	"pifacecad.h"

#ifndef	POLL 
static struct timeval	toCini = { 0, 750000 };	/* 750 ms select() timeout - re-iniialising value */
static struct timeval	toCnt  = { 0, 750000 };	/* 750 ms select() timeout */
static struct timeval *	toCoff = NULL;		/* select() timeout off when NULl pointer */
static struct timeval *	toCntp = NULL;		/* select() timeout initial value off */
#else	/* POLL */
static int		toCnt  = 750;		/* 750 ms poll() timeout */
static int		toM1   = -1;
static int *		toCoff = &toM1;		/* poll() timeout off when -1 */
static int *		toCntp = &toM1;		/* poll() timeout initial value off */
#endif	/* POLL */

static const char *	usage =
"Usage:\n"
" %s [-fBtmh][ -s <host>][ -p <port>][ -n <name>][ -i <inst>]\n"
"	  [ -d <deb>][ [<pfa>:]<ID>[-<ID>][-<inst>] ...]\n"
"	-s host	IP address of server    (default '%s')\n"
"	-p port	service port of server  (default '%s')\n"
"	-n name	of this client          (default 'PIFACE_0')\n"
"	-i inst	instance of this client (default '') or 1 to %d digits\n"
"	-B	start iCbox -d to monitor active PiFace I/O\n"
"	-f	force use of gpio 25 - some programs do not unexport correctly\n"
"	      For each PiFace or PiFaceCAD connected to a Raspberry Pi\n"
"	      8 bits of digital input and 8 bits of digital output with\n"
"	      known IEC numbers are generated.\n"
"	ID	X1 X2 IX8 QX9\n"
"	      The argument Xn can optionally be preceded by either I for\n"
"	      inputs or Q for outputs. The IEC-1131 ID's generated are\n"
"	      followed by a bit selector in the range .0 to .7.\n"
"	      For the above list X1 X2 IX7 QX4 the following is generated:\n"
"		inputs IX1.0 - .7 and outputs QX1.0 - .7 for PiFace unit 0\n"
"		inputs IX2.0 - .7 and outputs QX2.0 - .7 for PiFace unit 1\n"
"		inputs IX8.0 - .7 and outputs QX9.0 - .7 for PiFace unit 2\n"
"	      A range of ID's can be specified with X0-X3, IX4-IX6 or QX7-QX9\n"
"	      The actual PiFace addresses in the range 0 - 7 are selected by\n"
"	      jumpers JP1 and JP2 as well as CE0/CE1 (JP1 - JP8 on the PiRack).\n"
"		The program scans which addresses are present and associates\n"
"	      them with unit numbers in ascending PiFace address order.\n"
"	      Normally PiFace addresses will be chosen to coincide with unit\n"
"	      numbers, but this is not mandatory. What is mandatory is that\n"
"	      there are enough PiFaces to cover all the ID's in the parameters.\n"
"	pfa:ID	Associate a particular ID or ID range with a specific PiFace\n"
"	      address or address range. pfa has to be one number out of 0 - 7\n"
"	      and a PiFace with that address must be available.\n"
"		By choosing 1:X1-X3 for 3 PiFaces, the IEC numbers will\n"
"	      be the same as the PiFace addresses which are 1 to 3.\n"
"		This would be good programming practice. On the other hand\n"
"	      there are good reasons to choose different IEC ID's to fit in\n"
"	      with the iC application code and other I/O's.\n"
"	D ID QD	may be used as dummy ID's instead of Xn IXn QXn to exclude a\n"
"	      PiFace unit or individual I/O (really only useful for testing).\n"
"	ID-inst	Each individual ID or range can be followed by -<inst>,\n"
"	      where <inst> consists of 1 to %d numeric digits.\n"
"	      Such an appended instance code takes precedence over the\n"
"	      general instance specified with the -i <inst> option above.\n"
"					DEBUG options\n"
"	-t	trace arguments and activity (equivalent to -d100)\n"
"	-m	display microsecond timing info\n"
"	-d deb	+1   trace TCP/IP send actions\n"
"		+2   trace TCP/IP rcv  actions\n"
"		+4   trace MCP and HD44780 calls\n"
"		+10  trace SIO  input  actions\n"
"		+20  trace SIO  output actions\n"
"		+40  debug MCP and HD44780 calls\n"
"		+100 show arguments	DEBUG options\n"
"		+200 show more debugging details\n"
"		+400 exit after initialisation\n"
"	-h	help, ouput this Usage text only\n"
"\n"
"	NOTE: only one instance of iCpiFace may be run and all PiFaces and\n"
"	an optional PiFaceCAD must be controlled by this one instance. If\n"
"	two instances were running, the common interrupts would clash.\n"
"\n"
"Copyright (C) 2014 John E. Wulff     <immediateC@gmail.com>\n"
"Version	$Id: iCpiFace.c,v 1.3 2014/08/11 05:43:25 jw Exp $\n"
;

char *		texts[8] = {		/* current software can handle exactly 8 texts */
    "00000000011111111112222222222333333333341234567890123456789012345678901234567890",
    "John E Wulff, 37 Belbird Crescent,\nBowen Mountain 2753 NSW Australia",
    "OzBerryPi Users Group, 66 Oxford Street\nDarlinghurst 2010 NSW Australia",
    "texts[3]",
    "texts[4]",
    "texts[5]",
    "texts[6]",
    "texts[7]",
};

char *		iC_progname;		/* name of this executable */
static char *	iC_path;
short		iC_debug = 0;
int		iC_micro = 0;
unsigned short	iC_osc_lim = 1;

/********************************************************************
 *
 *  piFaceIO array pfi[unit] is organised as follows:
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

piFaceIO	pfi[MAXPF];		/* piFace names/gates and channels */
static int	unit = 0;
static unsigned short	topChannel = 0;
static char	buffer[BS];
static char	SR[] = "SR";		/* generates SI for iq 0, RQ for iq 1 */
static char	IQ[] = "IQ";
static short	errorFlag = 0;
static int	ioUnits = 0;		/* dynamically allocated size of Units[] */
static int *	Units = NULL;		/* dynamic array to store piFace unit numbers indexed by channel */

static void	storeUnit(unsigned short channel, int un);

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
    int		pfce;
    unsigned short pfa;
    int		pfe;
    int		len;
    int		value;
    int		sig;
    int		n;
    int		m;
    int		un;
    int		iq;
    int		unitA[2] = { 0, 0 };
    int		opt_B = 0;
    int		val;
    int		slr;
    int		cbc;
    unsigned short iid;
    int		regBufLen = 0;
    unsigned short iidN = -1;		/* internal instance "" initially */
    int		forceFlag = 0;
    char *	iC_fullProgname;
    char *	cp;
    char *	np;
    char *	op;
    int		ol;
    unsigned short	channel;
    int		retval;
#ifdef	POLL
    int		soc;			/* TCP/IP socket pollfd array index */
    int		gpi;			/* GPIO25 out-of-band pollfd array index */
    int		sti;			/* STDIN pollfd array index */
#endif	/* POLL */
    char	tail[32];
    piFaceIO *	pfp;
    piFaceIO *	pfp1;
    char	xd[2];
    char	iqc[2];
    char	iqe[2];
    int		iqStart;
    int		iqEnd;

    iC_outFP = stdout;			/* listing file pointer */
    iC_errFP = stderr;			/* error file pointer */

    iC_initIO();			/* catch memory access signal */

    /********************************************************************
     *
     *  Determine program name
     *
     *******************************************************************/

    iC_opt_P = 1;			/* PiFace always active for this app */
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
    iC_iccNM = NULL;			/* generate our own name - not 'stdin' from tcpc.c */
    if (iC_debug & 0200)  fprintf(iC_outFP, "fullPath = '%s' path = '%s' progname = '%s'\n", iC_fullProgname, iC_path, iC_progname);

#ifdef	BCM2835
    if (geteuid() != 0) {
	fprintf(iC_errFP, "ERROR: %s: Must be root to run. Program should be suid root\n", iC_progname);
	exit(1);
    }

#endif	/* BCM2835 */

    /********************************************************************
     *  Initialise PiFace control structures
     *******************************************************************/
    for (pfp = pfi; pfp < &pfi[MAXPF]; pfp++) {
	for (iq = 0; iq <= 1; iq++) {
	    pfp->s[iq].val = -1;		/* Dummy place holder for all PiFaces */
	    pfp->s[iq].channel = 0xffff;
	}
	pfp->pfa = 0xffff;			/* no pfa assigned */
    }

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
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (strlen(*argv)) iC_hostNM = *argv;
		    goto break2;
		case 'p':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (strlen(*argv)) iC_portNM = *argv;
		    goto break2;
		case 'n':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
		    if (strlen(*argv)) iC_iccNM = *argv;
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
		    opt_B = 1;		/* start iCbox -d to monitor active PiFace I/O */
		case 'f':
		    break;
		    forceFlag = 1;	/* force use of gpio 25 */
		    break;
		case 'm':
		    iC_micro++;		/* microsecond info */
		    break;
		case 'd':
		    if (! *++*argv) { --argc; if(! *++argv) goto error; }
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
		default:
		    fprintf(iC_errFP,
			"%s: unknown command line switch '%s'\n", iC_progname, *argv);
		case 'h':
		case '?':
	      error:
		    fprintf(iC_errFP, usage, iC_progname, iC_hostNM, iC_portNM, INSTSIZE, INSTSIZE);
		    exit(1);
		}
	    } while (*++*argv);
	  break2: ;
	} else {
	    /********************************************************************
	     *  Analyze ID arguments X0 X1 etc or groups X2-X4 and any D dummies
	     *******************************************************************/
	    int	pfStart = 0;
	    int	pfEnd   = 0;

	    iid = iidN;					/* global instance either 0xffff for "" or 0 - 999 */
	    if (sscanf(*argv, "%hu:%s", &pfa, tail) == 2) {
		if (pfa >= MAXPF) {
		    goto illFormed;
		}
		*argv += 2;				/* skip 0:, 1: .. 7: */
	    } else {
		pfa = 0xffff;				/* no pfa assigned */
	    }
	    if (sscanf(*argv, "%1[IQ]%31s", iqc, tail) == 2) {
		iqStart = iqEnd = (*iqc == 'Q');
		++*argv;				/* skip I or Q */
	    } else {
		*iqc    = '\0';				/* no I or Q ID */
		iqStart = 0;				/* or lone I or Q - trapped in next sscanf */
		iqEnd   = 1;
		unitA[0] = unitA[1] = max(unitA[0], unitA[1]);	/* Xx or D are paired */
	    }
	    if ((n = sscanf(*argv, "%1[XD]%5d%31s", xd, &pfStart, tail)) == 1 && *xd == 'D') {
		pfStart = pfEnd = -1;			/* dummy */
	    } else if (n > 1 && *xd == 'X') {
		if (n == 2) {
		    pfEnd = pfStart;			/* only a single Xn or D ID */
		} else if (n == 3) {
		    if (*tail != '-') {			/* both alternatives start with '-' */
			goto illFormed;
		    }
		    if ((n = sscanf(tail+1, "%1[IQ]%31s", iqe, tail+1)) == 2 && *iqc != *iqe) {	/* skip '-' */
			goto illFormed;			/* fails if first Xn did not have I or Q */
		    }
		    if ((n = sscanf(tail+1, "X%5d%31s", &pfEnd, tail)) == 0) {
			pfEnd = pfStart;		/* a single Xn-i ID with instance ID */
		    }
		    if (n != 1 && (n = sscanf(tail, "-%3hu%31s", &iid, tail)) != 1) {	/* individual instance has precedence */
			goto illFormed;
		    }
		    if (iC_debug & 0200) fprintf(iC_outFP, "%sX%d-%sX%d	iid = %hu\n",
			iqc, pfStart, iqc, pfEnd, iid);
		} else if (n == 0) {
		    goto illFormed;
		}
		if (pfEnd < pfStart) {
		    fprintf(iC_errFP, "ERROR: %s: '%s' is not a positive ID range\n", iC_progname, *argv);
		    errorFlag++;
		    goto endOneArg;
		}
	    } else {					/* n == 0 or *xd != 'X' */
	      illFormed:
		fprintf(iC_errFP, "ERROR: %s: '%s' is not a well formed ID\n", iC_progname, *argv);
		errorFlag++;
		goto endOneArg;
	    }
	    for (n = pfStart; n <= pfEnd; n++) {
		int		n1;
		int		unit1;
		int		pfa1;

		for (iq = iqStart; iq <= iqEnd; iq++) {
		    unit1 = unit = unitA[iq];
		    for (un = 0; un < unit1; un++) {
			if ((n1 = pfi[un].s[iq].val) == n &&
			    n >= 0 &&	/* can have any number of dummies on same pfa selection */
			    pfi[un].s[iq].channel == iid) {
			    if (iid != 0xffff) {
				fprintf(iC_errFP, "ERROR: %s: attempt to use %cX%d-%hu more than once\n",
				    iC_progname, IQ[iq], n, iid);
			    } else {
				fprintf(iC_errFP, "ERROR: %s: attempt to use %cX%d more than once\n",
				    iC_progname, IQ[iq], n);
			    }
			    errorFlag++;
			    goto endOneArg;
			}
			if (pfa < MAXPF && pfi[un].pfa == pfa) {	/* not 0xffff and == */
			    if (n1 < 0) {		/* name for this I/O ? */
				unit = un;		/* no - use previous pfi[] entry with same pfa */
			    } else {
				fprintf(iC_errFP, "ERROR: %s: attempt to use PiFace %hu more than once with %cX%d\n",
				    iC_progname, pfa, IQ[iq], n);
				errorFlag++;
				goto endOneArg;
			    }
			}
		    }
		    if (unit >= unit1 && unitA[iq]++ >= MAXPF) {
		      TooManyArguments:
			fprintf(iC_errFP, "ERROR: %s: 'too many ID arguments (%d max)\n", iC_progname, MAXPF);
			goto error;
		    }
		    /********************************************************************
		     *  Skip units which have been addressed directly before on their
		     *  alternate I/O. They must also be directly addressed.
		     *******************************************************************/
		    while ((pfa1 = pfi[unit].pfa) != 0xffff && pfa1 != pfa) {
			if ((unit = unitA[iq]++) >= MAXPF) goto TooManyArguments;
		    }
		    pfi[unit].s[iq].val = n;		/* valid ID argument Xn */
		    pfi[unit].s[iq].channel = iid;	/* argument instance or 0xffff */
		    if (pfi[unit].pfa == 0xffff) {
			pfi[unit].pfa = pfa;		/* argument PiFace address or 0xffff */
		    }
		    /********************************************************************
		     *  At the end of this loop all pfi[] entries can have an optional pfa
		     *  address, which will be used when PiFaces are detected or 0xffff.
		     *  It will be an error if that PiFace address is not found. 
		     *******************************************************************/
		}
		if (pfa < MAXPF) {
		    pfa++;
		} else {
		    pfa = 0xffff;
		}
	    }
	  endOneArg:;
	}
    }
    if (errorFlag) {
	goto error;					/* after all the command line has been analyzed */
    }
    if ((unit = max(unitA[0], unitA[1])) == 0) {
	for (iq = 0; iq <= 1; iq++) {
	    pfi[0].s[iq].val = 0;			/* valid ID argument IX0 QX0 */
	    pfi[0].s[iq].channel = iidN;		/* global instance or 0xffff */
	}
	unit = 1;
    }
    /********************************************************************
     *  Generate IEC names for all arguments which have a PiFace
     *******************************************************************/
    for (pfp = pfi; pfp < &pfi[unit]; pfp++) {
	for (iq = 0; iq <= 1; iq++) {
	    if ((n = pfp->s[iq].val) >= 0) {		/* valid ID argument Xn ? */
		len = snprintf(buffer, BS, "%cX%d", IQ[iq], n);	/* IEC name without instance */
		if ((iid = pfp->s[iq].channel) != 0xffff) {	/* append possible instance */
		    len += snprintf(buffer + len, BS, "-%hu", iid);
		}
		pfp->s[iq].i.name = iC_emalloc(++len);
		strcpy(pfp->s[iq].i.name, buffer);	/* store name */
	    }
	}
    }
    if (iC_debug & 0200) {
	printf("Provisional allocation for %d PiFace unit%s, global instance = \"%s\"\n",
	    unit, unit == 1 ? "" : "s", iC_iidNM);
	for (pfp = pfi; pfp < &pfi[unit]; pfp++) {
	    printf("pfi[%d]", pfp - pfi);
	    for (iq = 0; iq <= 1; iq++) {
		if (pfp->s[iq].i.name != NULL) {
		    printf("	%s", pfp->s[iq].i.name);
		} else {
		    printf("	Dummy");
		}
	    }
	    printf("	pfa = %hu\n", pfp->pfa);
	}
    }
    for (pfp = pfi; pfp < &pfi[MAXPF]; pfp++) {
	for (iq = 0; iq <= 1; iq++) {
	    pfp->s[iq].val = 0;		/* restore values */
	    pfp->s[iq].channel = 0;
	}
    }
#ifdef	EXIT 
    exit(0);

#endif
    /********************************************************************
     *
     *  Setup PiFaces
     *
     *  Initialisation of /dev/spidev0.0 and/or /dev/spidev0.1 for SPI
     *  modes and read/writeBytes
     *
     *  Initialisation using the /sys/class/gpio interface to the GPIO
     *  systems - slightly slower, but always usable as a non-root user
     *  export GPIO25, which is INTB of the MCP23S17 device.
     *
     *  Write to all relevant MCP23S17 registers to setup PIFACE
     *
     *  Wait for Interrupts on GPIO25 which has been exported and then
     *  opening /sys/class/gpio/gpio25/value
     *
     *  This is actually done via the /sys/class/gpio interface regardless of
     *  the wiringPi access mode in-use. Maybe sometime it might get a better
     *  way for a bit more efficiency.	(comment by Gordon Henderson)
     *
     *******************************************************************/

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
     *  Open the BCM2835 for direct memory access to GPIO to allow pullup.
     *  Set the pullup resistor on gpio25/INTB to allow open-drain interrupts.
     *  These actions are reversed in iC_quit().
     *
     *  Unfortunately this means the the program must be run as SUID root,
     *  which spoils debugging and is a nuisance generally. Take your pick!!
     *
     *  Compile with #define BCM2835 to allow PiRack without 100 Kohm resistor
     *  but requires the program to be SUID root.
     *******************************************************************/
#ifdef	BCM2835
    if (!bcm2835_init()) {
	fprintf(iC_errFP, "ERROR: %s: bcm2835_init failed - run as SUID root\n", iC_progname);
	iC_opt_P = 0;			/* cannot close PiFace units - not opened yet */
	iC_quit(sig);			/* not SUID */
    }
    bcm2835_gpio_set_pud(25, BCM2835_GPIO_PUD_UP);	/* Enable Pull Up */
#endif	/* BCM2835 */
    /********************************************************************
     *  Set the gpio25/INTB for falling edge interrupts
     *******************************************************************/
    if ((sig = doEdge(25, "falling", forceFlag, iC_fullProgname)) != 0) {
	unit = 0;
	iC_quit(sig);					/* another program is using gpio 25 */
    }
    /********************************************************************
     *  open /sys/class/gpio/gpio25/value permanently for obtaining
     *  out-of-band interrupts
     *******************************************************************/
    if ((gpio25FN = gpio_fd_open(25)) < 0) {
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
     *  Initialise all PiFaces - even Dummies, because there may be input
     *  interrupts.
     *
     *  Match PiFaces and an optional PiFaceCad to ID arguments in the
     *  command line.
     *
     *  Either an IXx or QXx argument or both or Xx mean an active PiFace.
     *  Any D dummies are counted but not activated. Any extra Pifaces
     *  are treated like dummies and not activated.
     *
     *  Test for the presence of a PiFace by writing to the IOCON register and
     *  checking if the value returned is the same. If not, there is no PiFace
     *  at that address. Do this for all 4 addresses on both CE0 and CE1.
     *
     *  Initially check for uninitalised pfa entries.
     *******************************************************************/
    pfp1 = pfi;
    for (pfa = pfe = npf = pfce = 0; pfce < 2; pfce++) {
	if ((spidFN[pfce] = setupSPI(pfce)) < 0) {
	    fprintf(iC_errFP, "ERROR: %s: setup /dev/spidev0.%d failed: %s\n", iC_progname, pfce, strerror(errno));
	}
	pfe += 4;
	m = 0;
	for ( ; pfa < pfe; pfa++) {
	    if (setupMCP23S17(spidFN[pfce], pfa, IOCON_ODR, 0x00, 0) < 0) {
		continue;			/* no PiFace at this address */
	    }
	    if ((iC_debug & 0300) == 0100) fprintf(iC_outFP, "PiFace %d found\n", pfa);
	    assert(npf < MAXPF);
	    for (pfp = pfi; pfp < &pfi[unit]; pfp++) {
		if (pfa == pfp->pfa) {		/* only unit entries could be initialised */
		    goto SkipIncrement;		/* if not used intf entry is not initalised */
		}
	    }
	    for ( ; pfp1 < &pfi[MAXPF]; pfp1++) {
		if (pfp1->pfa == 0xffff) break;	/* find next uninitalised pfi[].pfa entry */
	    }
	    assert(pfp1 < &pfi[MAXPF]);	/* should never run out of entries with these loops */
	    pfp = pfp1;			/* use next free pfi[] entry */
	    pfp->pfa = pfa;			/* will never see same pfa twice because of outer pfa++ loop */
	  SkipIncrement:
	    if (pfa == 4) {			/* PiFaceCAD ? */
		/********************************************************************
		 *  potentially a PiFaceCAD - test gpio 23 the LIRC output
		 *******************************************************************/
		if ((sig = doEdge(23, "falling", forceFlag, iC_fullProgname)) != 0) {
		    unit = 0;
		    iC_quit(sig);		/* another program is using gpio 23 */
		}
		if ((gpio23FN = gpio_fd_open(23)) < 0) {
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
		}
		/********************************************************************
		 *  free up the sysfs for gpio 23
		 *******************************************************************/
		if ((sig = doUnexport(23)) != 0) {
		    unit = 0;
		    iC_quit(sig);		/* unable to unexport gpio 23 */
		}
	    }
	    if (pfa != 4 || value == 0) {	/* PiFace */
		pfp->intf = INTFB;		/* input on MCP23S17 GPB register */
		pfp->inpr = GPIOB;		/* INTCAPB fails on bouncing switches */
	    } else {				/* PiFaceCAD is address 4 and GPIO23 LIRC is 1 */
		pfp->intf = INTFA;		/* input on MCP23S17 GPA register */
		pfp->inpr = GPIOA;		/* INTCAPA fails on bouncing switches */
	    }
	    pfp->spiFN = spidFN[pfce];		/* SPI file number for this active unit */
	    pfp++;
	    m++;				/* count active PiFaces in this CE group */
	    npf++;					/* count all PiFaces including dummies */
	}
	if (m == 0) {
	    close(spidFN[pfce]);			/* no active PiFaces in this CE group */
	    spidFN[pfce] = -1;
	}
    }
    for (pfp = pfi; pfp < &pfi[npf]; pfp++) {
	if (pfp->intf == 0) {
	    fprintf(iC_errFP, "ERROR: %s: PiFace %d address %hu: was requested in the call but was not found\n",
		iC_progname, pfp - pfi, pfp->pfa);
	    goto error;
	}
    }
    /********************************************************************
     *  Adjust sizes if number of ID arguments is incorrect
     *******************************************************************/
    if (npf < unit) {
	fprintf(iC_errFP, "WARNING: %s: too many ID arguments (%d) for the %d PiFaces found - ignore extra ID's\n",
	    iC_progname, unit, npf);		/* takes account of dummies */
    } else if (npf > unit) {
	fprintf(iC_errFP, "WARNING: %s: not enough ID arguments (%d) for the %d PiFaces found - ignore extra PiFaces\n",
	    iC_progname, unit, npf);		/* takes account of dummies */
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "### unit = %d npf = %d\n", unit, npf);
    unit = npf;					/* deal only with real PiFaces */
    if (unit == 0) {
	fprintf(iC_errFP, "ERROR: %s: no PiFaces to run\n", iC_progname);
	iC_quit(-2);
    }
    /********************************************************************
     *  End of PiFace detection
     *******************************************************************/
    if (iC_debug & 0200) {
	for (pfp = pfi; pfp < &pfi[npf]; pfp++) {
	    fprintf(iC_outFP, "***	PiFace%s un = %d pfa = %hu	%s	%s\n",
		pfp->intf == INTFB ? "   " : "CAD", pfp - pfi, pfp->pfa, pfp->Iname, pfp->Qname);
	}
    }
    /********************************************************************
     *  Final setup with correct IOCON_ODR for all PiFace Units
     *  Active driver output if only one PiFace else open drain
     *******************************************************************/
    for (pfp = pfi; pfp < &pfi[npf]; pfp++) {
	if (iC_debug & 0200) fprintf(iC_outFP, "###	un = %d pfa = %hu	%s	%s\n",
	    pfp - pfi, pfp->pfa, pfp->Iname, pfp->Qname);
	if (setupMCP23S17(pfp->spiFN, pfp->pfa, (npf == 1) ? 0 : IOCON_ODR, 0xff, pfp->intf) < 0) {
	    fprintf(iC_errFP, "ERROR: %s: PiFace %hu not found after succesful test ???\n",
		iC_progname, pfp->pfa);
	}
    }
    /********************************************************************
     *  Generate a meaningful name for network registration
     *******************************************************************/
    if (iC_iccNM == NULL) {
	strncpy(buffer, "PIFACE", BS);
	len = strlen(buffer);
	for (pfp = pfi; pfp < &pfi[npf]; pfp++) {
	    len += snprintf(buffer+len, BS-len, "_%hu", pfp->pfa);
	    if (iC_debug & 0100) fprintf(iC_outFP, "%22s	%s	%s\n",
		buffer, pfp->Iname, pfp->Qname);
	}
	iC_iccNM = iC_emalloc(len + 1);
	strcpy(iC_iccNM, buffer);
    }
    if (iC_micro) iC_microReset(0);		/* start timing */
    if (iC_debug & 0100) fprintf(iC_outFP, "host = %s, port = %s, name = %s, instance = '%s'\n", iC_hostNM, iC_portNM, iC_iccNM, iC_iidNM);
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
     *  ctrl-C, when it has been brought to the foreground with fg.
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
    if (strlen(iC_iidNM) == 0) {
	len = snprintf(regBuf, regBufLen, "N%s", iC_iccNM);			/* name header */
    } else {
	len = snprintf(regBuf, regBufLen, "N%s-%s", iC_iccNM, iC_iidNM);	/* name-instance header */
    }
    cp = regBuf + len;
    regBufLen -= len;
    /********************************************************************
     *  Send a registration string made up of all active I/O names
     *  There is either 1 input or 1 output name or both per active PiFace
     *******************************************************************/
    for (pfp = pfi; pfp < &pfi[npf]; pfp++) {
	assert(regBufLen > ENTRYSZ);			/* accomodates ",SIX123456,RQX123456,Z" */
	for (iq = 0; iq <= 1; iq++) {
	    if ((np = pfp->s[iq].i.name) != NULL) {
		assert(*np == IQ[iq]);	
		len = snprintf(cp, regBufLen, ",%c%s", SR[iq], np);
		cp += len;				/* input send name or output receive name */
		regBufLen -= len;
	    }
	}
    }
    if (iC_debug & 0200)  fprintf(iC_outFP, "regBufLen = %d\n", regBufLen);
    snprintf(cp, regBufLen, ",Z");		/* Z terminator */
    if (iC_debug & 0100)  fprintf(iC_outFP, "register:%s\n", regBuf);
    if (iC_micro) iC_microPrint("send registration", 0);
    iC_send_msg_to_server(iC_sockFN, regBuf);		/* register IOs with iCserver */
    /********************************************************************
     *  Receive a channel number for each I/O name sent in registration
     *  Distribute read and/or write channels returned in acknowledgment
     *******************************************************************/
    if (iC_rcvd_msg_from_server(iC_sockFN, rpyBuf, REPLY)) {	/* busy wait for acknowledgment reply */
	if (iC_micro) iC_microPrint("ack received", 0);
	if (iC_debug & 0100) fprintf(iC_outFP, "reply:%s\n", rpyBuf);
	if (opt_B) {
	    if (strlen(iC_iidNM) == 0) {
		len = snprintf(buffer, BS, "iCbox -d -n %s-IO", iC_iccNM);	/* prepare to execute iCbox -d */
	    } else {
		len = snprintf(buffer, BS, "iCbox -d -n %s-%s-IO", iC_iccNM, iC_iidNM);
	    }
	    op = buffer + len;
	    ol = BS - len;
	}
	cp = rpyBuf - 1;
	for (pfp = pfi, un = 0; un < npf; pfp++, un++) {
	    for (iq = 0; iq <= 1; iq++) {
		if ((np = pfp->s[iq].i.name) != NULL) {
		    assert(cp);			/* not enough channels in ACK string */
		    channel = atoi(++cp);	/* read channel from ACK string */
		    assert(channel > 0);
		    if (channel > topChannel) {
			topChannel = channel;
		    }
		    if (iC_debug & 0100) fprintf(iC_outFP, "PiFace %d %s  on channel %hu\n",
			pfp->pfa, np, channel);
		    pfp->s[iq].channel = channel;	/* link send channel to PiFace */
		    storeUnit(channel, un);		/* link piFace unit number to send channel */
		    if (opt_B) {
			assert(ol > 10);
			len = snprintf(op, ol, " %s", np);	/* add I/O name to execute iCbox -d */
			op += len;
			ol -= len;
		    }
		    cp = strchr(cp, ',');
		}
	    }
	}
	assert(cp == NULL);			/* Ack string matches Registration */
	if (opt_B) {
	    int r;
	    snprintf(op, ol, " &");		/* execute iCbox -d as a separate process */
	    if ((r = system(buffer)) != 0) {
		perror("iCbox");
		fprintf(stderr, "WARNING: %s: system(\"%s\") could not be executed $? = %d - ignore\n",
		    iC_progname, buffer, r);
	    }
	}
	if (iC_debug & 0100) fprintf(iC_outFP, "reply: top channel = %hu\n", topChannel);
    } else {
	iC_quit(QUIT_SERVER);			/* quit normally with 0 length message */
    }
    /********************************************************************
     *  Send IXn inputs to iCsever to initialise receivers
     *  Initialize a PiFaceCad
     *******************************************************************/
    if (iC_debug & 0200) fprintf(iC_outFP, "### Initialise %d unit(s)\n", npf);
    read (gpio25FN, &val, 1);			/* dummy read to clear interrupt on /dev/class/gpio25/value */
    if (iC_micro) iC_microPrint("SPI initialise", 0);
    cp = regBuf;
    regBufLen = REPLY;
    if (iC_debug & 0100) {
	op = buffer;
	ol = BS;
    }
    for (pfp = pfi; pfp < &pfi[npf]; pfp++) {
	assert(regBufLen > 11);					/* fits largest data telegram */
	if ((pfa = pfp->pfa) == 4 && pfp->intf == INTFA && pfp->Qname) {	/* registered PiFaceCAD */
	    pifacecad_lcd_clear();
	    pifacecad_lcd_write(texts[0]);			/* initial text */
	    pifacecad_lcd_display_on();				/* default is on */
	    pifacecad_lcd_set_backlight(1);			/* default is on */
	}
	if (pfp->Iname) {
	    val = readByte(pfp->spiFN, pfa, pfp->inpr);		/* read registered PiFace B/A at initialisation */
	    len = snprintf(cp, regBufLen, ",%hu:%d", pfp->channelSI, val);	/* data telegram */
	    cp += len;
	    regBufLen -= len;
	    if (iC_debug & 0100) {
		len = snprintf(op, ol, " %s(INI)", pfp->Iname);		/* source name */
		op += len;
		ol -= len;
	    }
	}
	pfp->valI = val;					/* initialise input for comparison */
	pfp->valQ = 0;						/* initialise output for comparison (ignore if not registered) */
    }
    if (cp > regBuf) {
	iC_send_msg_to_server(iC_sockFN, regBuf+1);		/* send data telegram(s) to iCserver (skip initial ',') */
	if (iC_debug & 0100) fprintf(iC_outFP, "P: %s: %s	<%s\n", iC_iccNM, regBuf+1, buffer);
    } else {
	fprintf(iC_errFP, "WARNING: %s: no PiFace inputs to initialise\n", iC_progname);
    }

    /********************************************************************
     *
     *  Set all bits to wait for interrupts
     *
     *******************************************************************/

#ifndef	POLL 
    FD_ZERO(&iC_infds);				/* should be done centrally if more than 1 connect */
    FD_ZERO(&iC_ixfds);				/* should be done centrally if more than 1 connect */
    FD_SET(iC_sockFN, &iC_infds);		/* watch sock for inputs */
    FD_SET(gpio25FN, &iC_ixfds);		/* watch GPIO25 for out-of-band input - do after iC_connect_to_server() */
#ifndef	WIN32
    FD_SET(0, &iC_infds);			/* watch stdin for inputs - FD_CLR on EOF */
    /* can only use sockets, not file descriptors under WINDOWS - use kbhit() */
#endif	/* WIN32 */
#else	/* POLL */
    soc = pollSet(iC_sockFN, POLLIN);		/* watch TCP/IP socket for inputs */
    gpi = pollSet(gpio25FN, POLLPRI);		/* watch GPIO25 for out-of-band input - do after iC_connect_to_server() */
#ifndef	WIN32
    /* can only use sockets, not file descriptors under WINDOWS - use kbhit() */
    sti = pollSet(0, POLLIN);			/* watch stdin for inputs - pollClr(sti) on EOF */
#endif	/* WIN32 */
#endif	/* POLL */

    if (iC_debug & 0200) fprintf(iC_outFP, "iC_sockFN = %d	gpio25FN = %d spidFN[0] = %d spidFN[1] = %d\n",
	iC_sockFN, gpio25FN, spidFN[0], spidFN[1]);

    /********************************************************************
     *
     *  External input (TCP/IP via socket and SIO from PiFace)
     *  Wait for input in a poll/select statement most of the time
     *  750 ms Timer is only switched on for left shifting lcd text
     *
     *******************************************************************/

    slr = 0;
    cbc = 0x03;					/* first input will switch to cursor/blink off */
    for (;;) {
#ifndef	POLL 
	if ((retval = iC_wait_for_next_event(toCntp)) == 0)
#else	/* POLL */
	if ((retval = iC_wait_for_next_event(*toCntp)) == 0)
#endif	/* POLL */
	{
#ifndef	POLL 
	    if (toCnt.tv_sec == 0 && toCnt.tv_usec == 0) {
		toCnt = toCini;			/* re-initialise timeout value */
#endif	/* POLL */
		if (slr & 0x08) {
		    pifacecad_lcd_move_left();	/* move text left every 750 ms */
		} else {
		    pifacecad_lcd_move_right();	/* move text right every 750 ms */
		}
#ifndef	POLL 
	    }
#endif	/* POLL */
	} else if (retval > 0) {
#ifndef	POLL 
	    if (FD_ISSET(iC_sockFN, &iC_rdfds))
#else	/* POLL */
	    if (pollEvent(soc, POLLIN))
#endif	/* POLL */
	    {
		/********************************************************************
		 *  TCP/IP input
		 *******************************************************************/
		if (iC_rcvd_msg_from_server(iC_sockFN, rpyBuf, REPLY)) {
		    if (iC_micro) iC_microPrint("TCP input received", 0);
		    assert(Units);
		    cp = rpyBuf - 1;
		    do {
			if (sscanf(++cp, "%hu:%d", &channel, &val) == 2 &&
			    channel > 0	&&
			    channel <= topChannel &&
			    (un = Units[channel]) < 8 &&
			    (pfp = &pfi[un])->Qname			/* is output registered? */
			) {
			    if (iC_debug & 0100) fprintf(iC_outFP, "P: %s: %hu:%d	> %s\n",
				iC_iccNM, channel, val, pfp->Qname);
			    if ((pfa = pfp->pfa) != 4 || pfp->intf != INTFA) {	/* PiFace */
				writeByte(pfp->spiFN, pfa, GPIOA, val);	/* write data to PiFace A output */
			    } else if ((m = val ^ pfp->valQ) != 0) {	/* PiFaceCAD */
				if (m & 0x07) {
				    pifacecad_lcd_clear();
				    if (iC_micro) iC_microPrint("pifacecad lcd cleared", 0);
				    pifacecad_lcd_write(texts[val & 0x07]);	/* new text */
				    if (iC_micro) iC_microPrint("pifacecad lcd text written", 0);
				}
				if (m & 0x18) {				/* shift left or right */
				    if ((slr = val & 0x18) != 0) {	/* start 750 ms timeout */
					toCntp = &toCnt;		/* for regular shift */
				    } else {				/* slr holds shift direction */
					toCntp = toCoff;		/* stop 750 ms timeout */
				    }					/* for both off */
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
				pfp->valQ = val;		/* store change for comparison */
			    }
			} else if (pfp->Qname == 0) {
			    fprintf(iC_errFP, "WARNING: %s: unit = %d, channel = %hu, val = %d; no output registered - should not happen\n",
				iC_progname, un, channel, val);		/* should not happen */
			} else {
			    fprintf(iC_errFP, "ERROR: %s: unit = %d, channel = %hu, topChannel = %hu, val = %d, '%s'\n",
				iC_progname, un, channel, topChannel, val, cp);
			    break;
			}
		    } while ((cp = strchr(cp, ',')) != NULL);
		} else {
		    iC_quit(QUIT_SERVER);	/* quit normally with 0 length message from iCserver */
		}
	    }
#ifndef	POLL 
	    if (FD_ISSET(gpio25FN, &iC_exfds))	/* watch for out-of-band GPIO25 input */
#else	/* POLL */
	    if (pollEvent(gpi, POLLPRI))	/* watch for out-of-band GPIO25 input */
#endif	/* POLL */
	    {
		int	m1 = 0;
		/********************************************************************
		 *  GPIO25 interrupt means SIO input
		 *******************************************************************/
		if (iC_micro) iC_microPrint("SPI input received", 0);
		cp = regBuf;
		regBufLen = REPLY;
		if (iC_debug & 0100) {
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
		    for (pfp = pfi; pfp < &pfi[npf]; pfp++) {
			if (readByte(pfp->spiFN, pfp->pfa, pfp->intf)) {	/* interrupt flag on this unit ? */
			    assert(regBufLen > 11);				/* fits largest data telegram */
			    val = readByte(pfp->spiFN, pfp->pfa, pfp->inpr);	/* read PiFace B/A at interrupt */
			    if (val != pfp->valI) {
				if (pfp->Iname) {
				    len = snprintf(cp, regBufLen, ",%hu:%d", pfp->channelSI, val); /* data telegram */
				    cp += len;
				    regBufLen -= len;
				    if (iC_debug & 0100) {
					len = snprintf(op, ol, " %s", pfp->Iname); /* source name */
					op += len;
					ol -= len;
				    }
				} else if (iC_debug & 0100) {
				    fprintf(iC_outFP, "P: %s: %d input on unregistered PiFace %d\n", iC_iccNM, val, pfp->pfa);
				}
				pfp->valI = val;				/* store change for comparison */
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
		    if (iC_debug & 0100) fprintf(iC_outFP, "P: %s: %s	<%s\n", iC_iccNM, regBuf+1, buffer);
		}
		if (iC_debug & 0200) {
		    if (m1 > 1){
			fprintf(iC_errFP, "WARNING: %s: GPIO25 interrupt %d loops %d changes \"%s\"\n", iC_progname, m1, m, regBuf+1);
		    } else if (m == 0) {	/* for some reason this happens occasionaly - no inputs are missed though */
			fprintf(iC_errFP, "WARNING: %s: GPIO25 interrupt and no INTF set on PiFaces\n", iC_progname);
		    }
		    *(regBuf+1) = '\0';			/* clean debug output next time */
		}
	    }
#ifndef	POLL 
	    if (FD_ISSET(0, &iC_rdfds))
#else	/* POLL */
	    if (pollEvent(sti, POLLIN))
#endif	/* POLL */
	    {
		/********************************************************************
		 *  STDIN
		 *******************************************************************/
		if ((n = getchar()) == EOF) {
		    fprintf(iC_errFP, "EOF received in stdin\n");
#ifndef	POLL 
		    FD_CLR(0, &iC_infds);	/* ignore EOF - happens in bg */
#else	/* POLL */
		    pollClr(sti);		/* ignore EOF - happens in bg */
#endif	/* POLL */
		} else if (n == 'q') {
		    iC_quit(QUIT_TERMINAL);	/* quit normally */
		} else if (n == 't') {
		    iC_debug ^= 0100;		/* toggle -t flag */
		} else if (n == 'm') {
		    iC_micro ^= 1;		/* toggle micro */
		} else if (n == 'i') {
		    for (pfp = pfi; pfp < &pfi[npf]; pfp++) {		/* report MCP23S17 IOCON, GPINTEN */
			fprintf(iC_outFP, "%s: un = %d pfa = %d IOCON = 0x%02x GPINTEN = 0x%02x\n",
			    pfp->Iname, pfp - pfi, pfp->pfa,
			    readByte(pfp->spiFN, pfp->pfa, IOCON),
			    readByte(pfp->spiFN, pfp->pfa, pfp->intf == INTFB ? GPINTENB : GPINTENA));
		    }
		} else if (n == 'I') {
		    for (pfp = pfi; pfp < &pfi[npf]; pfp++) {		/* restore MCP23S17 IOCON, GPINTEN */
			fprintf(iC_outFP, "%s: un = %d pfa = %d restore IOCON <== 0x%02x GPINTEN <== 0x%02x\n",
			    pfp->Iname, pfp - pfi, pfp->pfa,
			    IOCON_SEQOP | IOCON_HAEN | ((npf == 1) ? 0 : IOCON_ODR), 0xff);
			writeByte(pfp->spiFN, pfp->pfa, IOCON, IOCON_SEQOP | IOCON_HAEN | ((npf == 1) ? 0 : IOCON_ODR));
			writeByte(pfp->spiFN, pfp->pfa, pfp->intf == INTFB ? GPINTENB : GPINTENA, 0xff);
		    }
#ifdef	BCM2835
		} else if (n == 'G') {
		    fprintf(iC_outFP, "Restore Raspberry Pi GPIO25 pullup\n");
		    bcm2835_gpio_set_pud(25, BCM2835_GPIO_PUD_UP);	/* restore GPIO25 pullup */
#endif	/* BCM2835 */
		} else if (n != '\n') {
		    fprintf(iC_errFP, "no action coded for '%c' - try t, m, i, I, G, or q followed by ENTER\n", n);
		}
	    }
	} else {
	    perror("ERROR: select failed");				/* retval -1 */
	    iC_quit(SIGUSR1);
	}
    }
} /* main */

/********************************************************************
 *
 *	Initalise and expand dynamic array Units[] as necessary
 *	Store piFace unit numbers in Units[] indexed by channel
 *
 *******************************************************************/

static void
storeUnit(unsigned short channel, int un)
{
    while (channel >= ioUnits) {
	Units = (int *)realloc(Units,		/* initially NULL */
	    (ioUnits + IOUNITS) * sizeof(int));
	assert(Units);
	memset(&Units[ioUnits], '\0', IOUNITS * sizeof(int));
	ioUnits += IOUNITS;			/* increase the size of the array */
	if (iC_debug & 0200) fprintf(iC_outFP, "storeUnit: Units[%d] increase\n", ioUnits);
    }
    if (iC_debug & 0200) fprintf(iC_outFP, "storeUnit: Units[%d] <= %d\n", channel, un);
    Units[channel] = un;			/* store piFace unit number */
} /* storeUnit */
#endif	/* defined(RASPBERRYPI) && defined(TCP) && !defined(_WINDOWS) */

/* ############ POD to generate iCpiFace man page ############################

=encoding utf8

=head1 NAME

iCpiFace - real digital I/O for the iC environment on a Raspberry Pi

=head1 SYNOPSIS

 iCpiFace [-fBtmh][ -s <host>][ -p <port>][ -n <name>][ -i <inst>]
          [ -d <deb>][ <ID>[-<ID>][-<inst>] ...]
    -s host IP address of server    (default 'localhost')
    -p port service port of server  (default '8778')
    -n name of this client          (default 'PIFACE_0')
    -i inst instance of this client (default '') or 1 to 3 digits
    -B      start iCbox -d to monitor active PiFace I/O
    -f      force use of gpio 25 - some programs do not unexport correctly
          For each PiFace or PiFaceCAD connected to a Raspberry Pi
          8 bits of digital input and 8 bits of digital output with
          known IEC numbers are generated.
    ID      X1 X2 IX8 QX9
          The argument Xn can optionally be preceded by either I for
          inputs or Q for outputs. The IEC-1131 ID's generated are
          followed by a bit selector in the range .0 to .7.
          For the above list X1 X2 IX7 QX4 the following is generated:
            inputs IX1.0 - .7 and outputs QX1.0 - .7 for PiFace unit 0
            inputs IX2.0 - .7 and outputs QX2.0 - .7 for PiFace unit 1
            inputs IX8.0 - .7 and outputs QX9.0 - .7 for PiFace unit 2
          A range of ID's can be specified with X0-X3, IX4-IX6 or QX7-QX9
          The actual PiFace addresses in the range 0 - 7 are selected by
          jumpers JP1 and JP2 as well as CE0/CE1 (JP1 - JP8 on the PiRack).
            The program scans which addresses are present and associates
          them with unit numbers in ascending PiFace address order.
          Normally PiFace addresses will be chosen to coincide with unit
          numbers, but this is not mandatory. What is mandatory is that
          there are enough PiFaces to cover all the ID's in the parameters.
    pfa:ID  Associate a particular ID or ID range with a specific PiFace
          address or address range. pfa has to be one number out of 0 - 7
          and a PiFace with that address must be available.
            By choosing 1:X1-X3 for 3 PiFaces, the IEC numbers will
          be the same as the PiFace addresses which are 1 to 3.
            This would be good programming practice. On the other hand
          there are good reasons to choose different IEC ID's to fit in
          with the iC application code and other I/O's.
    D ID QD may be used as dummy ID's instead of Xn IXn QXn to exclude a
          PiFace unit or individual I/O (really only useful for testing).
    ID-inst Each individual ID or range can be followed by -<inst>,
          where <inst> consists of 1 to %d numeric digits.
          Such an appended instance code takes precedence over the
          general instance specified with the -i <inst> option above.
                                    DEBUG options
    -t      trace arguments and activity (equivalent to -d100)
    -m      display microsecond timing info
    -d deb  +1   trace TCP/IP send actions
            +2   trace TCP/IP rcv  actions
            +4   trace MCP and HD44780 calls
            +10  trace SIO  input  actions
            +20  trace SIO  output actions
            +40  debug MCP and HD44780 calls
            +100 show arguments     DEBUG options
            +200 show more debugging details
            +400 exit after initialisation
    -h      help, ouput this Usage text only

    NOTE: only one instance of iCpiFace may be run and all PiFaces and
    an optional PiFaceCAD must be controlled by this one instance. If
    two instances were running, the common interrupts would clash.

=head1 DESCRIPTION

B<iCpiFace> is an I/O client for iCserver in the immediate C
environment handling real 8 bit inputs and 8 bit outputs for each
PiFace controller attached to a Raspberry Pi. It also handles
real 8 bit inputs and control of an LCD Text panel for a PiFaceCAD.

A maximum of 8 PiFace controllers can be handled altogether - either
one plugged directly into a Raspberry Pi - or 4 on one PiRack -
7 on 2 PiRacks - 8 require 3 PiRacks.

One of the PiFaces (at address 4) can be a PiFaceCAD, which has 8
button inputs and a 2 line x 16 character LCD display. The PiFaceCAD
fits neatly as the last controller on a PiRack or can be mounted on
its own on the Raspberry Pi.

=head1 TECHNICAL BACKGROUND

The I/O connections on the PiFace board are controlled from the
Raspberry Pi via four different paths:

 1) the kernel SPI system can write and read to and from the MCP23S17
    16-Bit I/O Expander chip on the Piface. For Details see:
    http://ww1.microchip.com/downloads/en/DeviceDoc/21952b.pdf
    There are two SPI channels on the Raspberry Pi:
        /dev/spidev0.0 - which enables CS CE0 when SPI writes or reads.
        /dev/spidev0.1 - which enables CS CE1 when SPI writes or reads.
    The MCP23S17 is a slave SPI device. The slave address contains four
    fixed bits and three user-defined hardware address bits pins A2, A1
    and A0 (if enabled via IOCON.HAEN - A2 is not used in the PiFace).
    The Read/Write bit fills out the control byte. Jumpers JP1 and
    JP2 select A0 and A1 on each PiFace. Jumpers JP1 - JP8 select
    CE0 or CE1 for every PiFace on a PiRack. Together this makes for
    8 possible PiFace addresses 0 - 7. The PiFaceCAD board has fixed
    wiring with CE1 and address 0, making it combined address 4 (unless
    CE0 is swapped on the PiRack - which you wouldn't do - would you!).

 2) The MCP23S17 has a number of 8 bit registers which can be written
    and read via the above SPI channels. The most important of these
    for the PiFace driver are:
        IOCON   - which sets the configuration of the MCP23S17.
        IODIRA  - set to 0x00, which makes all 8 port A pins outputs.
        IODIRB  - set to 0xff, which makes all 8 port B pins inputs.
        GPIOA   - which sets the 8 output pins when written to.
        GPIOB   - which returns the 8 input pins when read.
        GPINTENB- set to 0xff, a change on any port B pin activates INTB.
        INTFB   - set for any pin which changed and caused an interrupt.
        INTCAPB - which returns the 8 input pins at interrupt time.
    The INTB output, which signals a change on any of the input bits,
    is set to active-low in an open-drain configuration to allow for more
    than one PiFace. INTB is connected to pin 22 (GPIO25) going to
    the Raspberry Pi.

    Reading either GPIOB or INTCAPB reset the source of the input
    interrupt.  INTCAPB only supplies the state at interrupt time,
    which does not necessarily reflect the state of the input when
    executing the interrupt handler. Using INTCAPB sometimes led to
    lost input changes. Therefore I do not use it.

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

 3) The Raspberry Pi brings out 17 GPIO signals, 5 of which double up to
    do the SPI interface, 2 for a UART, leaving 10 for general purpose
    input/output (there are 4 more on a 2nd connector, which is not fitted
    normally). The Linux "sysfs" can access the value of these GPIO pins
    from user space and more importantly can generate interrupts from them.
    For details see:
    http://www.auctoris.co.uk/2012/07/19/gpio-with-sysfs-on-a-raspberry-pi/

    GPIO25 (the interrupt from the PiFace inputs) is exported with
    "sysfs" in the PiFace driver and set to interrupt on its falling
    edge. The interrupt is out-of-band, so if select(2) is used,
    the third exception file descriptor set must be used. select is
    generally used in the iC system, because it handles timeouts
    correctly when interrupted.  Alternatively if poll(2) is used
    by defining POLL, it must be polled with POLLPRI rather than
    with POLLIN.

    NOTE: only one instance of iCpiFace may be run and all PiFaces and
    an optional PiFaceCAD must be controlled by this one instance. If
    two instances were running, the common interrupts would clash.
    If GPIO25 has already been exported by another program or by
    another instance of iCpiFace the program exits with an error
    message, since GPIO25 cannot be used by more than one program
    simultaneously.  Some programs (PiFace Digital Emulator) do not
    unexport GPIO25 when they close. The -f command line option can
    be used to force use of GPIO25 if you are sure there is no running
    program actually using GPIO25.

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

    Unfortunately this means iC_piFace must be run as SUID root, which
    prevents debugging and is a nuisance generally.  You can solve this
    problem for multiple PiFaces by soldering a 100 Kohm resistor from
    one of the GPIO25 (pin 22) to 5V on the PiRack.  Take your pick!!

For the PiFaceCad there are two additional paths from the Raspberry
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

 6) The PiFaceCad also has a LIRC output on pin 16 (GPIO23). This
    output is high if a PiFaceCad is present - low otherwise.  It is
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

L<iCrfid(1)>, L<iCserver(1)>, L<iCbox(1)>, L<makeAll(1)>, L<select(2)>, L<poll(2)>

=head1 COPYRIGHT

COPYRIGHT (C) 2014  John E. Wulff

You may distribute under the terms of either the GNU General Public
License or the Artistic License, as specified in the README file.

For more information about this program, or for information on how
to contact the author, see the README file.

=cut

   ############ end of POD to generate iCpiFace man page ##################### */

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
 *	iCgpioPUD
 *
 *	Auxiliary SUID program to turn the pull-up/down resistors
 *	for GPIO pads of the BCM2835 processor on a Raspberry PI
 *	on or off.
 *
 *  For technical details of GPIO direct memory access see bcm2835.h
 *
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
 *  This means the the program must be run as SUID root.  This spoils
 *  debugging, but this is why we made this very simple aux program.
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#ifndef RASPBERRYPI
#error - must be compiled with RASPBERRYPI
#else	/* RASPBERRYPI */

#include	"bcm2835.h"

static const char *	usage =
"Usage: %s [-h] -g <gpio> -p <pud>\n"
"    -g gpio GPIO number in the range 0 to 27\n"
"    -p pud  pull-up/down; 0 = off; 1 = down; 2 = up;\n"
"    -h      help, ouput this Usage text only\n"
"    NOTE:   This program must be suid root to run\n"
"\n"
"Copyright (C) 2015 John E. Wulff     <immediateC@gmail.com>\n"
"Version	$Id: iCgpioPUD.c,v 1.1 2015/04/08 23:45:27 pi Exp $\n"
;

char *		iC_progname;		/* name of this executable */

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
    uint8_t	gpio = 0xff;
    uint8_t	pud  = 0xff;
    int		errFlag = 0;

    iC_progname = argv[0];

    /********************************************************************
     *
     *  Process command line arguments
     *
     *******************************************************************/

    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
		case 'g':
		    if (! *++*argv) { --argc; if(! *++argv) goto errorp; }
		    if (strlen(*argv) && isdigit(**argv)) gpio = (uint8_t)atoi(*argv);
		    goto break2;
		case 'p':
		    if (! *++*argv) { --argc; if(! *++argv) goto errorp; }
		    if (strlen(*argv) && isdigit(**argv)) pud = (uint8_t)atoi(*argv);
		    goto break2;
		default:
		    fprintf(stderr,
			"%s: unknown command line switch '-%c'\n", iC_progname, **argv);
		    errFlag++;
		    break;
	      errorp:
		    fprintf(stderr,
			"%s: value for final command line switch '-%c' missing\n",
			iC_progname, *--*--argv);
		case 'h':
		case '?':
	      error:
		    fprintf(stderr, usage, iC_progname);
		    exit(1);
		}
	    } while (*++*argv);
	  break2: ;
	} else {
	    fprintf(stderr,
		"%s: unknown command line argument '%s'\n", iC_progname, *argv);
	    errFlag++;
	}
    }
    if (gpio > 27) {
	fprintf(stderr,
	    "%s: must specify -g <gpio> where gpio must be <= 27 (not -g %d)\n",
	    iC_progname, (int)gpio);
	errFlag++;
    }
    if (pud > 2) {
	fprintf(stderr,
	    "%s: must specify -p <pud>  where pud  must be <= 2  (not -p %d)\n",
	    iC_progname, (int)pud);
	errFlag++;
    }
    if (errFlag) {
	goto error;
    }
    if (geteuid() != 0) {
	fprintf(stderr, "ERROR: %s: Must be root to run. Program should be suid root\n", iC_progname);
	exit(1);
    }
    /********************************************************************
     *  Turn the actual pull-up/down resistor on or off
     *******************************************************************/
    if (!bcm2835_init()) {
	fprintf(stderr, "ERROR: %s: bcm2835_init failed - run as SUID root\n", iC_progname);
	exit(1);
    }
    bcm2835_gpio_set_pud(gpio, pud);			/* Modify Pull Up/Down */
    return 0;
} /* main */
#endif	/* RASPBERRYPI */

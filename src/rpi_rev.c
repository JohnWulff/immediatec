static const char rpi_rev_c[] =
"$Id: rpi_rev.c,v 1.2 2015/02/25 21:07:47 jw Exp $";
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
 *	rpi_rev.c
 *	adapted from raspberryalphaomega.org.uk by John E. Wulff 2015
 *
 *	Returns the following board revisions for differen Raspberry Pi models
 *		0	(8  0x08)	Rpi A
 *		1	(15 0x0e)	Rpi B	tested by JW
 *		2	(16 0x10)	Rpi B+	tested by JW
 *
 *	according to http://elinux.org/RPi_HardwareHistory
 *
 *    Revision 	Release Date 	Model 	PCB Rev	Memory 	Notes
 *	Beta 	Q1 2012 	B  	 ? 	256MB 	Beta Board
 *	0002 	Q1 2012 	B 	1.0 	256MB 	
 *	0003 	Q3 2012 	B  	1.0 	256MB 	(ECN0001) Fuses mod and D14 removed
 *	0004 	Q3 2012 	B 	2.0 	256MB 	(Mfg by Sony)
 *	0005 	Q4 2012 	B 	2.0 	256MB 	(Mfg by Qisda)
 *	0006 	Q4 2012 	B 	2.0 	256MB 	(Mfg by Egoman)
 *	0007 	Q1 2013 	A 	2.0 	256MB 	(Mfg by Egoman)
 *	0008 	Q1 2013 	A 	2.0 	256MB 	(Mfg by Sony)
 *	0009 	Q1 2013 	A 	2.0 	256MB 	(Mfg by Qisda)
 *	000d 	Q4 2012 	B 	2.0 	512MB 	(Mfg by Egoman)
 *	000e 	Q4 2012 	B 	2.0 	512MB 	(Mfg by Sony)
 *	000f 	Q4 2012 	B 	2.0 	512MB 	(Mfg by Qisda)
 *	0010 	Q3 2014 	B+ 	1.0 	512MB 	(Mfg by Sony)
 *	0011 	Q2 2014  Compute Module	1.0 	512MB 	(Mfg by Sony)
 *	0012 	Q4 2014 	A+ 	1.0 	256MB 	(Mfg by Sony) 
 *	If you see a "1000" at the front of the Revision, e.g. 10000002
 *	then it indicates[1] that your Raspberry Pi has been over-volted,
 *	and your board revision is simply the last 4 digits (i.e. 0002 in this example).
 *
 *	Other models need to be checked as they become available
 *
 *******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpi_rev.h"

/********************************************************************
 *	Validate gpio pins for different processors
 *	01 (1<<0) for Raspberry Pi A  (rev = 0x8  or 8)
 *	02 (1<<1) for Raspberry Pi B  (rev = 0xe  or 15)
 *	04 (1<<2) for Raspberry Pi B+ (rev = 0x10 or 16)
 *******************************************************************/
int	gpioUsed[] = {			/* marked as used by app by writing 0100000 */
    01, 01, 06, 06, 07, 04, 04, 07,	/* gpio 0  - 7  * 0 1 PiA, 2 3 PiB(+) 5 6 PiB+ only */
    07, 07, 07, 07, 04, 04, 07, 07,	/* gpio 8  - 15 * 12 13 PiB+ only */
    04, 07, 07, 04, 04, 04, 07, 07,	/* gpio 16 - 23 * 16 19 20 21 PiB+ only */
    07, 07, 04, 07, 0,  0,  0,  0, 	/* gpio 24 - 31 * 26 PiB+ only */
    0,  0,  0,  0,  0,  0,  0,  0, 	/* gpio 32 - 39 */
    0,  0,  0,  0,  0,  0,  0,  0, 	/* gpio 40 - 47 */
    0,  0,  0,  0,  0,  0,  0,  0, 	/* gpio 48 - 55 */
    0,  0,  0,  0,  0,  0,  0,  0, 	/* gpio 56 - 63 */
};

int
boardrev(void) {
    FILE*	f;
    int 	rev;
    char	buf[1024];
    char *	needle;
    int		proc;

    if ((f = fopen("/proc/cmdline", "r")) == NULL) {
	perror("rpi_rev: fopen(/proc/cmdline\")");
	exit(-1);
    }
    fread(buf, 1, 1023, f);
    fclose(f);
    if ((needle = strstr(buf, "boardrev")) == NULL ||
	sscanf(needle, "boardrev=0x%x", &rev) != 1) {
	fprintf(stderr, "rpi_rev: cannot find \"boardrev\" in:\n%s\n", buf);
	exit(-1);
    }
    switch (rev & 0xffff) {	/* ignore overvolting */
    /* Revision   Release Date Model  PCB Rev	Memory	Notes */
    case 0x02:	/* Q1 2012 	B 	1.0 	256MB	 */
    case 0x03:	/* Q3 2012 	B  	1.0 	256MB 	(ECN0001) Fuses mod and D14 removed */
    case 0x04:	/* Q3 2012 	B 	2.0 	256MB 	(Mfg by Sony) */
    case 0x05:	/* Q4 2012 	B 	2.0 	256MB 	(Mfg by Qisda) */
    case 0x06:	/* Q4 2012 	B 	2.0 	256MB 	(Mfg by Egoman) */
	proc = 1;	/* Rpi B */
	break;
    case 0x07:	/* Q1 2013 	A 	2.0 	256MB 	(Mfg by Egoman) */
    case 0x08:	/* Q1 2013 	A 	2.0 	256MB 	(Mfg by Sony) */
    case 0x09:	/* Q1 2013 	A 	2.0 	256MB 	(Mfg by Qisda) */
	proc = 0;	/* Rpi A */
	break;
    case 0x0d:	/* Q4 2012 	B 	2.0 	512MB 	(Mfg by Egoman) */
    case 0x0e:	/* Q4 2012 	B 	2.0 	512MB 	(Mfg by Sony) */
    case 0x0f:	/* Q4 2012 	B 	2.0 	512MB 	(Mfg by Qisda) */
	proc = 1;	/* Rpi B */
	break;
    case 0x10:	/* Q3 2014 	B+ 	1.0 	512MB 	(Mfg by Sony) */
	proc = 2;	/* Rpi B+ */
	break;
    case 0x11:	/* Q2 2014  Comp Module	1.0 	512MB 	(Mfg by Sony) */
	proc = 2;	/* Rpi Compute Module? */
	break;
    case 0x12:	/* Q4 2014 	A+ 	1.0 	256MB 	(Mfg by Sony)  */
    default:
	proc = 2;	/* Rpi A+ ? */
	break;
    }
    return proc;
} /* boardrev */

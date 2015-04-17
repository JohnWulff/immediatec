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

#ifndef RPI_REV_H
#define RPI_REV_H
static const char rpi_rev_h[] =
"$Id: rpi_rev.h,v 1.2 2015/02/25 21:06:04 jw Exp $";

extern int	boardrev(void);
extern int	gpioUsed[];

#endif	/* RPI_REV_H */

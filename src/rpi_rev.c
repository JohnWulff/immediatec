static const char rpi_rev_c[] =
"$Id: rpi_rev.c 1.6 $";
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

 *	Determine the RPi Board Revision and use it to Validate gpio pins for that board
 *
 *	according to https://elinux.org/RPi_HardwareHistory (updated 17/04/2018)
 *
 *	Board Revision History
 *
 *	The reliable way to find out your board revision is to use the following command:
 *		cat /proc/cpuinfo
 *	You will see your device data including:
 *		Hardware	: BCM2708
 *		Revision	: 0003
 *
 *	If you see a "1000" at the front of the Revision, e.g. 10000002
 *	then it indicates[1] that your Raspberry Pi has been over-volted,
 *	and your board revision is simply the last 4 digits (i.e. 0002 in this example).
 *
 *	The previous way to extract "boardrev=0003" from /proc/cmdline no longer works with RPi 3B+
 *
 *    Revision 	Release Date 	Model 	PCB Rev	Memory 	Notes
 *	Beta 	Q1 2012 	B (Beta) ? 	256 MB 	Beta Board
 *	0002 	Q1 2012 	B 	1.0 	256 MB
 *	0003 	Q3 2012 	B	1.0 	256 MB  (ECN0001) Fuses mod and D14 removed
 *	0004 	Q3 2012 	B 	2.0 	256 MB 	(Mfg by Sony)
 *	0005 	Q4 2012 	B 	2.0 	256 MB 	(Mfg by Qisda)
 *	0006 	Q4 2012 	B 	2.0 	256 MB 	(Mfg by Egoman)
 *	0007 	Q1 2013 	A 	2.0 	256 MB 	(Mfg by Egoman)
 *	0008 	Q1 2013 	A 	2.0 	256 MB 	(Mfg by Sony)
 *	0009 	Q1 2013 	A 	2.0 	256 MB 	(Mfg by Qisda)
 *	000d 	Q4 2012 	B 	2.0 	512 MB 	(Mfg by Egoman)
 *	000e 	Q4 2012 	B 	2.0 	512 MB 	(Mfg by Sony)
 *	000f 	Q4 2012 	B 	2.0 	512 MB 	(Mfg by Qisda)
 *	0010 	Q3 2014 	B+ 	1.0 	512 MB 	(Mfg by Sony)
 *	0011 	Q2 2014 Compute Module1	1.0 	512 MB 	(Mfg by Sony)
 *	0012 	Q4 2014 	A+ 	1.1 	256 MB 	(Mfg by Sony)
 *	0013 	Q1 2015 	B+ 	1.2 	512 MB 	(Mfg by Embest)
 *	0014 	Q2 2014 Compute Module1	1.0	512 MB 	(Mfg by Embest)
 *	0015 	 ? 		A+ 	1.1 256/512 MB 	(Mfg by Embest)
 *	a01040 	Unknown 	2 B 	1.0 	1 GB 	(Mfg by Sony)
 *	a01041 	Q1 2015 	2 B 	1.1 	1 GB 	(Mfg by Sony)
 *	a21041 	Q1 2015 	2 B 	1.1 	1 GB 	(Mfg by Embest)
 *	a22042 	Q3 2016 	2 B 	1.2 	1 GB 	(Mfg by Embest) (with BCM2837)
 *	900021 	Q3 2016 	A+ 	1.1 	512 MB 	(Mfg by Sony)
 *	900032 	Q2 2016? 	B+ 	1.2 	512 MB 	(Mfg by Sony)
 *	900092 	Q4 2015 	Zero 	1.2 	512 MB 	(Mfg by Sony)
 *	900093 	Q2 2016 	Zero 	1.3 	512 MB 	(Mfg by Sony)
 *	920093 	Q4 2016? 	Zero 	1.3 	512 MB 	(Mfg by Embest)
 *	9000c1 	Q1 2017 	Zero W 	1.1 	512 MB 	(Mfg by Sony)
 *	a02082 	Q1 2016 	3 B 	1.2 	1 GB 	(Mfg by Sony)
 *	a020a0 	Q1 2017 Compute Module3	1.0 	1 GB 	(Mfg by Sony) (and CM3 Lite)
 *	a22082 	Q1 2016 	3 B 	1.2 	1 GB 	(Mfg by Embest)
 *	a32082 	Q4 2016 	3 B 	1.2 	1 GB 	(Mfg by Sony Japan)
 *	a020d3 	Q1 2018 	3 B+ 	1.3 	1 GB 	(Mfg by Sony)
 *
 *	Other models need to be checked as they become available
 *
 *	Returns the following board revisions for differen Raspberry Pi models
 *		0	(8  0x08)	Rpi A
 *		1	(15 0x0e)	Rpi B	tested by JW
 *		2	(16 0x10)	Rpi B+	tested by JW
 *		2	(0xa21041)	Rpi 2B	tested by JW (checked 17/04/2018)
 *		2	(0xa22082)	Rpi 3B	tested by JW (checked 17/04/2018)
 *		2	(0xa020d3)	Rpi 3B+	tested by JW (checked 17/04/2018)
 *
 *******************************************************************/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>

#ifndef RASPBERRYPI
#error - must be compiled with RASPBERRYPI
#else	/* RASPBERRYPI */

#include "rpi_rev.h"
#include "icc.h"			/* declares iC_emalloc() */

/********************************************************************
 *	Validate gpio pins for different RPi boards
 *******************************************************************/
static long long	gpioValid[] = {
    0x000000000bc6cf93LL,	/* proc == 0 for Raspberry Pi A  (rev = 0x8  or 8 ) */
    0x00000000fbc6cf9cLL,	/* proc == 1 for Raspberry Pi B  (rev = 0xe  or 15) */
    0x000080480ffffffcLL,	/* proc == 2 for Raspberry Pi B+ (rev = 0x10 or 16) */
				/* 	  or for Raspberry Pi 2B (rev = 0xa21041  ) */
				/* 	  or for Raspberry Pi 3B (rev = 0xa22082  ) */
				/* 	  or for Raspberry Pi 3B+(rev = 0xa020d3  ) */
};

static int
boardrev(void) {
    FILE*	f;
    int 	rev;
    char	buf[2048];
    char *	needle;
    int		proc;

    if ((f = fopen("/proc/cpuinfo", "r")) == NULL) {
	perror("rpi_rev: fopen(/proc/cpuinfo)");
	exit(-1);
    }
    fread(buf, 2047, 1, f);
    fclose(f);
    if ((needle = strstr(buf, "Revision")) == NULL ||
	sscanf(needle, "Revision : %x", &rev) != 1) {
	fprintf(stderr, "*** error: rpi_rev: cannot find \"Revision\" in:\n%s\n", buf);
	exit(-1);
    }
    switch (rev & 0xffff) {	/* ignore overvolting */
    /* Revision   Release Date Model  PCB Rev	Memory	Notes */
    case 0x07:	/* Q1 2013 	A 	2.0 	256MB 	(Mfg by Egoman) */
    case 0x08:	/* Q1 2013 	A 	2.0 	256MB 	(Mfg by Sony) */
    case 0x09:	/* Q1 2013 	A 	2.0 	256MB 	(Mfg by Qisda) */
	proc = 0;	/* Rpi A */
	break;
    case 0x02:	/* Q1 2012 	B 	1.0 	256MB	 */
    case 0x03:	/* Q3 2012 	B  	1.0 	256MB 	(ECN0001) Fuses mod and D14 removed */
    case 0x04:	/* Q3 2012 	B 	2.0 	256MB 	(Mfg by Sony) */
    case 0x05:	/* Q4 2012 	B 	2.0 	256MB 	(Mfg by Qisda) */
    case 0x06:	/* Q4 2012 	B 	2.0 	256MB 	(Mfg by Egoman) */
    case 0x0d:	/* Q4 2012 	B 	2.0 	512MB 	(Mfg by Egoman) */
    case 0x0e:	/* Q4 2012 	B 	2.0 	512MB 	(Mfg by Sony) */
    case 0x0f:	/* Q4 2012 	B 	2.0 	512MB 	(Mfg by Qisda) */
	proc = 1;	/* Rpi B */
	break;
    default:
	proc = 2;	/* Rpi A+ Zero B+ 2B 3B 3B+ same GPIO 40 pins as B+ */
	break;
    }
    return proc;
} /* boardrev */

/********************************************************************
 *
 *	Determine proc and valid
 *
 *	Open or create and lock the auxiliary file ~/.iC/gpios.used,
 *	which contains the struct Used u - one 64 bit word u.used
 *					 - one 64 bit word u.oops
 *
 *	return proc, valid, u.used and u.oops in static struct ProcValidUsed gpios
 *
 *	For an RPi B+ processor it contains:
 *		gpios.proc	1			// returned by boardrev()
 *		gpios.valid	0x00000000fbc6cf9c	// GPIOs valid for this RPi
 *		gpios.u.used	0x0000000000000000	// GPIOs used in other apps
 *		gpios.u.oops	0x0000000000000000	// bit to block iCtherm
 *
 *	If -f (forceFlag) is set the gpios.u.used is cleared unconditionally
 *		gpios.u.oops must stay set for iCtherm (cleared with iCtherm -f).
 *	Else only GPIO's not used in other apps can be used in this app.
 *
 *******************************************************************/

static char *		mem;
static char *		buff;
static ProcValidUsed	gpios;
static int		gpiosFN;
static char *		gpiosName;
static int		blksize;

ProcValidUsed *
openLockGpios(int force)
{
    int		exists = 1;
    char *	home;
    struct stat	sb;
    int		align;

    gpios.proc = boardrev();
    assert(gpios.proc < sizeof gpioValid / sizeof gpioValid[0]);
    gpios.valid = gpioValid[gpios.proc];

    if ((home = getenv("HOME")) != NULL) {
	gpiosName = iC_emalloc(strlen(home)+16);	/* +1 for '\0' */
	sprintf(gpiosName, "%s/.iC", home);		/* directory ~/.iC */;

	blksize = 0x1000;				/* for RPi */
	align = blksize-1;
	mem = (char *) malloc((int)blksize+align);
	buff = (char *)(((uintptr_t)mem+align)&~((uintptr_t)align));

	if (stat(gpiosName, &sb) < 0 ||
	    ! S_ISDIR(sb.st_mode & S_IFMT)) {
	    if (errno == ENOENT) {
		if (mkdir(gpiosName, 0755) < 0) {
		    perror("rpi_rev: cannot create directory ~/.iC");
		    return NULL;	/* error */
		}
	    } else {
		perror("rpi_rev: cannot access ~/.iC/gpios.used");
		return NULL;		/* error */
	    }
	}
	/* directory ~/.iC existed or has just been created */
	sprintf(gpiosName, "%s/.iC/gpios.used", home);
	if (stat(gpiosName, &sb) < 0 ||
	    ! S_ISREG(sb.st_mode & S_IFMT)) {
	    if (errno == ENOENT) {
		if ((gpiosFN = open(gpiosName,
		    O_RDWR|O_CREAT|O_TRUNC|O_DIRECT|O_SYNC,
		    S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH)) == -1) {
		    perror("rpi_rev: cannot create ~/.iC/gpios.used");
		    return NULL;	/* error */
		}
		exists = 0;
		gpios.u.used  = 0LL;
		gpios.u.oops  = 0LL;
	    } else {
		perror("rpi_rev: cannot access ~/.iC/gpios.used");
		return NULL;	/* error */
	    }
	}
	/* file ~/.iC/gpios.used existed or has just been created */
    } else {
	fprintf(iC_errFP, "*** error: rpi_rev: no environment variable HOME ???\n");
	return NULL;	/* error */
    }
    if (exists && (gpiosFN = open(gpiosName, O_RDWR|O_DIRECT|O_SYNC)) == -1) {
	perror("rpi_rev: cannot open ~/.iC/gpios.used");
	return NULL;		/* error */
    }
    if (flock(gpiosFN, LOCK_EX) < 0) {	/* block until file is unlocked - lock the file */
	perror("rpi_rev: cannot lock ~/.iC/gpios.used");
	return NULL;		/* error */
    }
    if (force || ! exists) {
	gpios.u.used  = 0LL;		/* keep u.oops forever once set */
    } else {
	if (read(gpiosFN, buff, (size_t)blksize) < 1) {	/* 16 bytes u.used and u.oops */
	    fprintf(iC_errFP, "*** error: rpi_rev: openLockGpios: read failed\n");
	    return NULL;		/* error */
	}
	memcpy(&gpios.u, buff, sizeof gpios.u);
    }
    return &gpios;			/* current proc valid used and oops */
} /* openLockGpios */

/********************************************************************
 *
 *	Write unlock and close the auxiliary file ~/.iC/gpios.used
 *	Unlink (remove) ~/.iC/gpios.used if !gpios.u.used && !gpios.u.oops
 *
 *******************************************************************/

int
writeUnlockCloseGpios(void)
{
    struct stat	sb;

    if (lseek(gpiosFN, 0L, SEEK_SET) < 0) {	/* rewind to re-write the file */
	perror("rpi_rev: cannot lseek ~/.iC/gpios.used");
	return -1;		/* error */
    }
    memcpy(buff, &gpios.u, sizeof gpios.u);
    if (write(gpiosFN, buff, (size_t)blksize) < 1) {	/* 16 bytes u.used and u.oops */
	perror("rpi_rev: cannot write ~/.iC/gpios.used");
	free((char *)mem);
	return -1;		/* error */
    }
    if (flock(gpiosFN, LOCK_UN) < 0) {		/* remove the lock held by this process */
	perror("rpi_rev: cannot unlock ~/.iC/gpios.used");
	return -1;		/* error */
    }
    if (close(gpiosFN) < 0) {
	perror("rpi_rev: cannot close ~/.iC/gpios.used");
	return -1;		/* error */
    }
    free((char *)mem);
    if (stat(gpiosName, &sb) == 0 &&
	S_ISREG(sb.st_mode & S_IFMT) &&
	!gpios.u.used &&
	!gpios.u.oops &&
	unlink(gpiosName) < 0) {
	perror("rpi_rev: cannot unlink ~/.iC/gpios.used");
	return -1;		/* error */
    }
    return 0;
} /* writeUnlockCloseGpios */
#endif	/* RASPBERRYPI */

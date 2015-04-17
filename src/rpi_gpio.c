static const char rpi_gpio_c[] =
"$Id: rpi_gpio.c,v 1.3 2015/04/08 04:33:54 jw Exp $";
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
 *	rpi_gpio.c
 *	communication to/from GPIO pins on the Raspberry Pi using
 *	the /sys/class/gpio interface to the GPIO system
 *
 *
 *  The following routines were partly extracted from gpio.c
 *  Copyright (c) 2012 Gordon Henderson
 *
 *  It seems tidier to use these routines directly to access the
 *  /sys/class/gpio device interface rather than execute 'gpio' from C
 *
 *  This version has been modified  by John E. Wulff	2014/03/25
 *  for the immediate C system to interface with a PiFace in
 *  accordance with the terms of the GNU Lesser General Public License
 *
 *******************************************************************/

#include	<sys/ioctl.h>
#include	<errno.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<assert.h>
#include	<time.h>
#include	<string.h>
#include	"rpi_gpio.h"

static struct timespec	ms50 = { 0, 50000000, };

/********************************************************************
 *
 *	gpio_export
 *	gpio		# RPi gpio pin number N
 *	direction	# in or out
 *	mode		# none, rising, falling or both
 *	force		# 1 force, 0 block use if already exported
 *	fullProgname	# full name with path to change owner
 *
 *	Export gpio pin N unless already in use or 'force'
 *	Set the direction to in or out
 *	If direction is 'in' set the interrupt mode to none, rising, falling or both
 *	This uses the 'sysfs' /sys/class/gpio device interface.
 *
 *******************************************************************/

int
gpio_export(int gpio, const char * direction, const char * mode, int force, const char * fullProgname)
{
    FILE *	fd;
    char	fName[128];
    struct stat	sb;
    int		r;
    int		ret = 0;

    assert(gpio >= 0 && gpio < 64);

    /* check that this gpio pin has not been exported by another program */

    sprintf(fName, "/sys/class/gpio/gpio%d", gpio);
    if (stat(fName, &sb) == 0) {
	if (force == 0) {
	    fprintf(iC_errFP, "%s: GPIO%d is in use by another program - cannot use it for I/O or interrupts\n"
		"try -f if you are sure no other running program is using GPIO%d\n", iC_progname, gpio, gpio);
	    return SIGUSR2;		/* gpio pin in use by another program */
	} else if ((ret = gpio_unexport(gpio)) != 0) {	/* force */
	    return ret;			/* unable to unexport gpio N */
	}
    }

    /* Export gpio pin N */

    if ((fd = fopen("/sys/class/gpio/export", "w")) == NULL) {
	fprintf(iC_errFP, "%s: Unable to open GPIO export interface: %s\n", iC_progname, strerror(errno));
	return SIGUSR1;
    }
    fprintf(fd, "%d\n", gpio);
    fclose(fd);

    /********************************************************************
     *  This may take some time if not SUID root
     *  Measured 53 to 120 ms to change permissions - go figure
     *  Takes even longer when app is started with ddd via gdb - why ?!
     *******************************************************************/
    sprintf(fName, "/sys/class/gpio/gpio%d/direction", gpio);
    for (r = 0; (fd = fopen(fName, "w")) == NULL; r++) {
	if (r < 50) {			/* wait 2.5 seconds max - will break within 50 ms of opening */
#if	YYDEBUG && defined(TRACE)
	    if (iC_debug & 04) fprintf(iC_outFP, "*** open direction r = %d for gpio %d\n", r, gpio);
#endif	/* YYDEBUG && defined(TRACE) */
	    nanosleep(&ms50, NULL);	/* 50 ms */
	} else {
	    fprintf(iC_errFP, "%s: Unable to open GPIO direction interface for gpio %d: %s\n", iC_progname, gpio, strerror(errno));
	    return SIGUSR1;
	}
    }
    if (strcasecmp (direction, "in")  == 0 ||
	strcasecmp (direction, "out") == 0) {
	fprintf(fd, "%s\n", direction);
    } else {
	fprintf(iC_errFP, "%s: Invalid direction: %s. Should be in or out\n", iC_progname, direction);
	return SIGUSR1;
    }
    fclose(fd);

    /* For drection 'in' set the edge for interrupts */

    if (strcasecmp (direction, "in")  == 0) {
	sprintf(fName, "/sys/class/gpio/gpio%d/edge", gpio);
	if ((fd = fopen(fName, "w")) == NULL) {
	    fprintf(iC_errFP, "%s: Unable to open GPIO edge interface for gpio %d: %s\n", iC_progname, gpio, strerror(errno));
	    return SIGUSR1;
	}

	if (strcasecmp (mode, "none")    == 0 ||
	    strcasecmp (mode, "rising")  == 0 ||
	    strcasecmp (mode, "falling") == 0 ||
	    strcasecmp (mode, "both")    == 0) {
	    fprintf(fd, "%s\n", mode);
	} else {
	    fprintf(iC_errFP, "%s: Invalid mode: %s. Should be none, rising, falling or both\n", iC_progname, mode);
	    return SIGUSR1;
	}
	fclose(fd);
    }
    return ret;
} /* gpio_export */

/****************************************************************
 *
 *	gpio_fd_open	# open /sys/class/gpio/gpioN/value
 *	gpio		# RPi gpio pin number N
 *	flags		# O_RDONLY or O_RDWR (O_WRONLY)
 *
 ****************************************************************/

int
gpio_fd_open(int gpio, int flags)
{
    char	buf[32];

    snprintf(buf, 32, "/sys/class/gpio/gpio%d/value", gpio);
    return open(buf, flags | O_NONBLOCK );
} /* gpio_fd_open */

/****************************************************************
 *
 *	gpio_read	# digital read of /sys/class/gpio/gpioN/value
 *	fd		# file number previously opened for pin N
 *	return		# 0 or 1 (-1 if value returns other than "0" or "1")
 *
 ****************************************************************/

int
gpio_read(int fd)
{
    char	c;

    if (fd == -1) return -1;
    lseek(fd, 0L, SEEK_SET);
    read(fd, &c, 1);
    return (c == '1') ? 1 : (c == '0') ? 0 : -1;
} /* gpio_read */

/****************************************************************
 *
 *	gpio_write	# digital write to /sys/class/gpio/gpioN/value
 *	fd		# file number previously opened for pin N
 *	value		# 0 writes 0 to GPIO, != 0 writes 1 to GPIO
 *
 ****************************************************************/

void
gpio_write(int fd, int value)
{
    if (fd != -1) {
	if (value == 0) {
	    write(fd, "0\n", 2);
	} else {
	    write(fd, "1\n", 2);
	}
    }
} /* gpio_write */

/********************************************************************
 *
 *	gpio_unexport
 *	gpio		# RPi gpio pin number
 *
 *	This uses the /sys/class/gpio device interface.
 *
 *******************************************************************/

int
gpio_unexport(int gpio)
{
    FILE *	fd;

    if ((fd = fopen("/sys/class/gpio/unexport", "w")) == NULL) {
	fprintf(iC_errFP, "%s: Unable to open GPIO unexport interface: %s\n", iC_progname, strerror(errno));
	return SIGUSR1;
    }

    fprintf(fd, "%d\n", gpio);
    fclose(fd);
    return 0;
} /* gpio_unexport */

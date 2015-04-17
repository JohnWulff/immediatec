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
 *	rpi_gpio.h
 *	communication to/from GPIO pins on the Raspberry Pi using
 *	the /sys/class/gpio interface to the GPIO system
 *
 *******************************************************************/

#ifndef RPI_GPIO_H
#define RPI_GPIO_H
static const char rpi_gpio_h[] =
"$Id: rpi_gpio.h,v 1.3 2015/03/05 06:10:16 jw Exp $";

#include	<stdio.h>

extern char *	iC_progname;			/* name of this executable */
extern FILE *	iC_outFP;			/* listing file pointer */
extern FILE *	iC_errFP;			/* error file pointer */
extern short	iC_debug;

extern int	gpio_export(int gpio, const char * direction, const char * mode, int force, const char * fullName);
extern int	gpio_fd_open(int gpio, int flags);
extern int	gpio_read(int fd);
extern void	gpio_write(int fd, int value);
extern int	gpio_unexport(int gpio);

#endif	/* RPI_GPIO_H */

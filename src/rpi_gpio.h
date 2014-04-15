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
"$Id: rpi_gpio.h,v 1.1 2014/04/15 09:39:53 pi Exp $";

#include	<stdio.h>

extern char *		iC_progname;			/* name of this executable */
extern FILE *		iC_errFP;			/* error file pointer */
extern short		iC_debug;

extern int		gpio_fd_open(unsigned int gpio);
extern int		gpio_read(int fd);
extern void		doEdge(int gpio, const char * mode, const char * fullName);
extern void		doUnexport(int gpio);
extern void		iC_quit(int sig);

#endif	/* RPI_GPIO_H */

static const char const_c[] =
"@(#)$Id: const.c 1.1 $";
/********************************************************************
 *
 *	Copyright (C) 2016  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	cons_test.c
 *
 *  Test auxiliary grammar to evaluate constant expression strings for
 *  the immcc compiler at compile time.
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	"comp.h"

#ifndef	TESTCONS
#error - must be compiled with TESTCONS defined to link correctly
#else	/* TESTCONS */

extern void	warning(char *, char *);	/* print warning message */

/********************************************************************
 *
 *	Main program for testing the cons.y parser
 *
 *******************************************************************/

char *	iC_progname;			/* name of this executable */
static const char *	usage =
"Usage: %s constant_expression_text [...]\n"
"        -h              this help text\n"
"Copyright (C) 2016 John E. Wulff     <immediateC@gmail.com>\n"
;

int
main(int argc, char **argv)
{
    int		value;
    int		n = 0;

    iC_progname = *argv;
    if (argc <= 1) goto error;
    while (--argc > 0) {
	if (**++argv == '-') {
	    ++*argv;
	    do {
		switch (**argv) {
		case 'h':
		case '?':
		error:
		    printf(usage, iC_progname);
		    exit(1);
		default:
		    printf("WARNING: %s: unknown option -%c\n", iC_progname, **argv);
		    break;
		}
	    } while (*++*argv);
	} else {
	    if (parseConstantExpression(*argv, &value, 0)) {
		warning("not a constant expression", *argv);	/* parse error */
	    } else {
		printf("%d	'%s' ==> %d\n", ++n, *argv, value);
	    }
	}
    }
    return 0;
} /* main */
#endif	/* TESTCONS */

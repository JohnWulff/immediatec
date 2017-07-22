static const char icbegin_c[] =
"@(#)$Id: icbegin.c 1.8 $";
/********************************************************************
 *
 *	Copyright (C) 1985-2017  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	icbegin.c
 *	dummy initialisation routine
 *
 *******************************************************************/

/********************************************************************
 *  The following initialisation function is one of two empty functions
 *  in the libict.a support library. The other is iCend().
 *
 *  iCbegin() can be used to initialise immC variables etc.
 *  Either or both may be implemented in a literal block in iC source(s),
 *  in which case those function will be linked in preference.
 *
 *  This bare bones version, which is linked into every generated iC app
 *  unless a different function iCbegin() is defined in the iC app source,
 *  implements the --h and --R command line options to provide individual
 *  help for each app and code to implement running another app as a separate
 *  process. This is called "chaining". The initial Usage called up with the
 *  --h option explains chaining of auxiliary apps with the --R option.
 *
 *  To obtain an extended iCbegin() in your own iC app:
 *  1) copy the following code  starting at the line containing
 *     "Start of literal C code" to the end into your new iC source.
 *  2) remove the // coments from the first three lines and the last line.
 *  3) add extra "case 'x':" lines for extra --x command line options after
 *     "switch (**argv) {". ('x' may any letter except 'h' or 'R').
 *  4) follow the the new "case" line with individual code - usually
 *     setting a flag: eg "opt_x = 1;"
 *  5) follow this with a "break;" for a -x command line switch without
 *     arguments. If an argument is required see example code in main()
 *     in "load.c". (The "case 'R':" is a rather complex example).
 *  6) declare "static int opt_x;" and any other new variables before
 *     "int iCbegin(...) {...}".
 *  7) change "return -1;" at the end of "int iCbegin(...) {...}" to
 *     "return 0;" if you want execution of your iCbegin() to be reported
 *     on start up of your app with the -d100 option.
 *  8) add help lines after "Usage:..." for extra --x command line options.
 *  9) add extra C code before the line containing "End of literal C code".
 *     Usually this is
 *     a) extra #include lines.
 *     b) extra static variable declarations.
 *     c) extra C functions using the new options.
 *******************************************************************/

#include	<stdio.h>
#include	"icc.h"		/* use <icg.h> in iC app.ic sources */

// %{	/* Start of literal C code */
// #include	<stdio.h>
// #include	<icg.h>

static const char *	usage =
"Usage: %s --[ -h]|[ -R <app ...>]\n"
"   --h           this help text\n"
"                      AUXILIARY app\n"
"   --R <app ...> run auxiliary app followed by -z and its arguments\n"
"                 as a separate process; -R ... must be last arguments.\n"
"\n"
#if YYDEBUG && !defined(RUN)
"            typing t toggles the -t debug info flag\n"
"            typing m toggles the -m timing info flag\n"
#endif	/* YYDEBUG && !defined(RUN) */
"            typing q or ctrl+D stops the app\n"
;

int
iCbegin(int argc, char** argv)	/* default initialisation function */
{
    char *	progname = *argv;
    char *	mqz = "-qz";
    char *	mz  = "-z";

    if (argc >= 0) {
	/********************************************************************
	 *  Process command line arguments
	 *******************************************************************/
	while (--argc > 0) {
	    if (**++argv == '-') {
		++*argv;
		do {
		    switch (**argv) {
		    case '\0':
			goto break2;		/* - ignore */
		    case '-':
			break;			/* ignore extra - */
		    case 'R':
			/********************************************************************
			 *  Run auxiliary app with rest of command line
			 *  splice in the "-z" option to block STDIN interrupts in chained apps
			 *  alternatively "-qz" option for quiet operation and to block STDIN
			 *******************************************************************/
			if (! *++*argv) { --argc; if(! *++argv) goto missing; }
			*(argv-1) = *argv;	/* move app string to previous argv array member */
			*argv = iC_debug & DQ ?  mqz : mz; /* point to "-qz"  or "-z" in current argv */	
			argv--;			/* start call with app string */
			goto break3;
		    missing:
			fprintf(stderr, "ERROR: %s: missing value after '%s'\n", progname, ((*--argv)-=2, *argv));
		    case 'h':
		    case '?':
			fprintf(stderr, usage, progname);
			iC_quit(-2);
		    default:
			fprintf(stderr, "WARNING: %s: unknown option -%c\n", progname, **argv);
			break;
		    }
		} while (*++*argv);
		break2: ;
	    } else fprintf(stderr, "WARNING: %s: unknown argument '%s'\n", progname, *argv);
	}				/* end of command line analysis */
    } else {
	/********************************************************************
	 *  Run auxiliary app
	 *******************************************************************/
      break3:
	iC_fork_and_exec(argv);
    }
    return -1;			/* does nothing */
} /* iCbegin */
// %}	/* End of literal C code */

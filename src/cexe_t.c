static const char cexe_t_c[] =
"@(#)$Id: cexe_t.c 1.2 $";
/********************************************************************
 *
 *	Copyright (C) 1985-2013  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	cexe_t.c
 *	template for cexe.c to bootstrap icr and ict
 *	if cexe.c does not exist
 *
 *******************************************************************/

#include	<stdio.h>
#include	<signal.h>
#include	"comp.h"

#if INT_MAX == 32767 && defined (LONG16)
long
#else
int
#endif
iC_exec(int iC_indx, iC_Gt * iC_gf)
{
    fflush(iC_outFP);
    fprintf(iC_errFP,
	"\n*** Error: cexe.c: does not support arithmetic expressions yet.\n"
	  "*** Rebuild compilers using '%s -c -%sO%o %s; m -rt'\n"
	  , iC_progname, iC_gflag ? "g" : "", iC_optimise, inpNM);
    iC_quit(SIGUSR1);
    return 0;	/* never gets here */
} /* iC_exec */

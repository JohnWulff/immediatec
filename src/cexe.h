static const char cexe_h[] =
"@(#)$Id: cexe.h,v 1.11 2001/03/02 12:56:32 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2001  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <john@je-wulff.de>
 *
 *	cexe.h
 *	template to generate extensions to the icc compiler
 *
 *******************************************************************/

#include <stdio.h>
#include "icc.h"
#include "comp.h"

#line 16 "cexe.h"
#define _(x) lookup(#x)->u.gate->gt_old
#define A(x,v) assign(lookup(#x)->u.gate, v)
Q
int
c_exec(int pp_index, Gate * _cexe_gf)
{
    switch (pp_index) {
V
#line 25 "cexe.h"
    default:
#ifndef _WINDOWS
	fflush(outFP);
	fprintf(errFP,
	    "\n%s: line %d: F%d C function unknown\n",
	    __FILE__, __LINE__, pp_index);
	quit(-1);
#endif
	break;
    }
#ifndef _WINDOWS
    fflush(outFP);
    fprintf(errFP,
	"\n%s: line %d: Function fragment without return ???\n",
	__FILE__, __LINE__);
    quit(-2);
#else
    return 0;	/* for those cases where no return has been programmed */
#endif
} /* c_exec */

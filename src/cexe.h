static const char cexe_h[] =
"@(#)$Id: cexe.h,v 1.7 2000/11/11 13:36:37 jw Exp $";
/********************************************************************
 *
 *	Routine to execute C code fragments in the PPLC system
 *
 *	"cexe.h	1.06	95/01/31"
 *
 *******************************************************************/

#include <stdio.h>
#include "pplc.h"
#include "comp.h"

#line 16 "cexe.h"
#define _(x) lookup(#x)->u.gate->gt_old
#define A(x,v) assign(lookup(#x)->u.gate, v)

int
c_exec(int pp_index, Gate * gp)
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

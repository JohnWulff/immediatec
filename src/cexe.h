static const char cexe_h[] =
"@(#)$Id: cexe.h,v 1.5 1999/08/04 18:28:55 jw Exp $";
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
c_exec(int pp_index)
{
    switch (pp_index) {
V
#line 25 "cexe.h"
    default:
#ifndef _WINDOWS
	fprintf(errFP,
	    "\n%s: line %d: F%d C function unknown\n",
	    __FILE__, __LINE__, pp_index);
	quit(-1);
#endif
	break;
    }
#ifndef _WINDOWS
    fprintf(errFP,
	"\n%s: line %d: Function fragment without return ???\n",
	__FILE__, __LINE__);
    quit(-2);
#endif
    return 0;	/* for those cases where no return has been programmed */
} /* c_exec */

static const char cexe_h[] =
"@(#)$Id: cexe.h,v 1.3 1998/10/02 11:15:55 john Exp $";
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
static int x, y, z;
static Gate* px;
extern int getCount(void);
extern int assign(Gate * lv, int rv);

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
    return 0;	/* for those cases where no return has been programmed */
} /* c_exec */

/********************************************************************
 *
 *	test L referencing via it's symbol table
 *	test timer return value when run down
 *
 *******************************************************************/

int
getCount(void)
{
    static Gate * gp;
    static Gate * ap;
    Gate * tp;

    if (gp == 0) {
	gp = lookup("du_1")->u.gate;	/* address of action gate */
	ap = gp->gt_clk;		/* timer for this action */
    }
    if (gp->gt_val > 0) {
	y = 1000 - gp->gt_mark;		/* action gate low */
	return 0;
    }
    tp = ap;				/* action gate high */
    y = 0;
    while ((tp = tp->gt_next) != ap && (y += tp->gt_mark, tp != gp));
    y = 1000 - y;
    return tp == gp;			/* gate found, valid time value */
} /* getCount */

/********************************************************************
 *
 *	Assign to an imm int node
 *
 *	The node is linked to a_list when rv != new && new == old.
 *	If a second assignment occurs while node is still linked to
 *	a_list (new != old), if rv != old new is simply updated
 *	else node is unlinked and new updated, making new == old.
 *	When rv == new, there is no change at all.
 *
 *******************************************************************/

int
assign(Gate * lv, int rv)
{
    if (rv != lv->gt_new) {
	if (lv->gt_new == lv->gt_old || rv == lv->gt_old) {
	    link_ol(lv, a_list);	/* first change or glitch */
	}
	lv->gt_new = rv;		/* first or later change */
    }
    return rv;
} /* assign */

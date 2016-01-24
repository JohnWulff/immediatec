/********************************************************************
 *
 *	SOURCE:   ./Test0/if1.ic
 *	OUTPUT:   ./Test0/if1.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: if1.c,v 1.1 2015/05/24 00:51:53 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, 0 };
static iC_Gt _f1_1   = { 1, -iC_OR, iC_F_CF, 0, "_f1_1", {&iC_l_[0]}, {&iC_l_[2]}, &IX0_0 };
iC_Gt in1      = { 1, -iC_ALIAS, iC_GATE, 0, "in1", {0}, {(iC_Gt**)&IX0_0}, &_f1_1, 0 };

iC_Gt *		iC___Test0_if1_list = &in1;
iC_Gt **	iC_list[] = { &iC___Test0_if1_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 1 "./Test0/if1.ic"

#include	<stdio.h>

#line 40 "./Test0/if1.c"

static int iC_2(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 6 "./Test0/if1.ic"
{ printf("iC is here!!\n"); }
#line 46 "./Test0/if1.c"
    return 0;
} /* iC_2 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* _f1_1 */	(iC_Gt*)iC_2, &iClock,	&IX0_0, 0, 0,
};

/********************************************************************
 *
 *	SOURCE:   ./Test0/if2.ic
 *	OUTPUT:   ./Test0/if2.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: if2.c,v 1.1 2015/05/24 00:51:53 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, 0 };
static iC_Gt _f1_1   = { 1, -iC_OR, iC_F_CE, 0, "_f1_1", {&iC_l_[0]}, {&iC_l_[2]}, &IX0_1 };

iC_Gt *		iC___Test0_if2_list = &_f1_1;
iC_Gt **	iC_list[] = { &iC___Test0_if2_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 1 "./Test0/if2.ic"

#include	<stdio.h>

#line 39 "./Test0/if2.c"

static int iC_2(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 5 "./Test0/if2.ic"
{
    printf("IX0.0 is on!!\n");
}
#line 47 "./Test0/if2.c"
    else
#line 7 "./Test0/if2.ic"
{
    printf("IX0.0 is off again!!\n");
}
#line 53 "./Test0/if2.c"
    return 0;
} /* iC_2 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* _f1_1 */	(iC_Gt*)iC_2, &iClock,	&IX0_1, 0, 0,
};

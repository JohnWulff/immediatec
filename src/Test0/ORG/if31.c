/********************************************************************
 *
 *	SOURCE:   ./Test0/if31.ic
 *	OUTPUT:   ./Test0/if31.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: if31.c,v 1.1 2015/05/24 00:51:53 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, 0 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB1 };
static iC_Gt _f1_1   = { 1, -iC_OR, iC_F_CE, 0, "_f1_1", {&iC_l_[0]}, {&iC_l_[2]}, &IX0_0 };
static iC_Gt _f2_1   = { 1, -iC_ARN, iC_F_CE, 0, "_f2_1", {&iC_l_[5]}, {&iC_l_[7]}, &_f1_1 };

iC_Gt *		iC___Test0_if31_list = &_f2_1;
iC_Gt **	iC_list[] = { &iC___Test0_if31_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 1 "./Test0/if31.ic"

#include	<stdio.h>

#line 41 "./Test0/if31.c"

static int iC_2(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 5 "./Test0/if31.ic"
{
    printf("IX0.0 is TRUE\n");
}
#line 49 "./Test0/if31.c"
    else
#line 7 "./Test0/if31.ic"
{
    printf("IX0.0 is FALSE\n");
}
#line 55 "./Test0/if31.c"
    return 0;
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 10 "./Test0/if31.ic"
{
    printf("IB1 is TRUE\n");
}
#line 65 "./Test0/if31.c"
    else
#line 12 "./Test0/if31.ic"
{
    printf("IB1 is FALSE\n");
}
#line 71 "./Test0/if31.c"
    return 0;
} /* iC_3 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* _f1_1 */	(iC_Gt*)iC_2, &iClock,	&IX0_0, 0, 0,
/* _f2_1 */	(iC_Gt*)iC_3, &iClock,	(iC_Gt*)0, &IB1, 0,
};

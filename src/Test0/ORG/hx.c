/********************************************************************
 *
 *	SOURCE:   ./Test0/hx.ic
 *	OUTPUT:   ./Test0/hx.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: hx.c,v 1.1 2015/10/15 06:27:21 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, 0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
static iC_Gt _f1_1   = { 1, -iC_OR, iC_F_CF, 0, "_f1_1", {&iC_l_[0]}, {&iC_l_[2]}, &IX0_3 };
static iC_Gt _f2_1   = { 1, -iC_OR, iC_F_CF, 0, "_f2_1", {&iC_l_[5]}, {&iC_l_[7]}, &_f1_1 };
static iC_Gt _f3_1   = { 1, -iC_OR, iC_F_CF, 0, "_f3_1", {&iC_l_[10]}, {&iC_l_[12]}, &_f2_1 };
static iC_Gt _f4_1   = { 1, -iC_OR, iC_F_CF, 0, "_f4_1", {&iC_l_[15]}, {&iC_l_[17]}, &_f3_1 };
static iC_Gt _f5_1   = { 1, -iC_OR, iC_F_CF, 0, "_f5_1", {&iC_l_[20]}, {&iC_l_[22]}, &_f4_1 };
static iC_Gt _f6_1   = { 1, -iC_OR, iC_F_CF, 0, "_f6_1", {&iC_l_[25]}, {&iC_l_[27]}, &_f5_1 };

iC_Gt *		iC___Test0_hx_list = &_f6_1;
iC_Gt **	iC_list[] = { &iC___Test0_hx_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 7 "./Test0/hx.ic"

#include	<stdio.h>

#line 46 "./Test0/hx.c"

static int iC_2(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 11 "./Test0/hx.ic"
{
	    printf("Hello Nr. %d, 0\n", 1);
	}
#line 54 "./Test0/hx.c"
    return 0;
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 14 "./Test0/hx.ic"
{
	    printf("Hello Nr. %d, 0\n", 2);
	}
#line 64 "./Test0/hx.c"
    return 0;
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 17 "./Test0/hx.ic"
{
	    printf("Hello Nr. %d, 1\n", 2);
	}
#line 74 "./Test0/hx.c"
    return 0;
} /* iC_4 */

static int iC_5(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 20 "./Test0/hx.ic"
{
	    printf("Hello Nr. %d, 0\n", 3);
	}
#line 84 "./Test0/hx.c"
    return 0;
} /* iC_5 */

static int iC_6(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 23 "./Test0/hx.ic"
{
	    printf("Hello Nr. %d, 1\n", 3);
	}
#line 94 "./Test0/hx.c"
    return 0;
} /* iC_6 */

static int iC_7(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 26 "./Test0/hx.ic"
{
	    printf("Hello Nr. %d, 2\n", 3);
	}
#line 104 "./Test0/hx.c"
    return 0;
} /* iC_7 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* _f1_1 */	(iC_Gt*)iC_2, &iClock,	&IX0_1, 0, 0,
/* _f2_1 */	(iC_Gt*)iC_3, &iClock,	&IX0_2, 0, 0,
/* _f3_1 */	(iC_Gt*)iC_4, &iClock,	&IX0_2, 0, 0,
/* _f4_1 */	(iC_Gt*)iC_5, &iClock,	&IX0_3, 0, 0,
/* _f5_1 */	(iC_Gt*)iC_6, &iClock,	&IX0_3, 0, 0,
/* _f6_1 */	(iC_Gt*)iC_7, &iClock,	&IX0_3, 0, 0,
};

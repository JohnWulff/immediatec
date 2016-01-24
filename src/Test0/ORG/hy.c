/********************************************************************
 *
 *	SOURCE:   ./Test0/hy.ic
 *	OUTPUT:   ./Test0/hy.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: hy.c,v 1.1 2015/10/15 06:27:21 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB0      = { 1, -iC_INPW, iC_ARITH, 0, "IB0", {0}, {0}, 0 };
iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, &IB0 };
iC_Gt IB2      = { 1, -iC_INPW, iC_ARITH, 0, "IB2", {0}, {0}, &IB1 };
iC_Gt IB3      = { 1, -iC_INPW, iC_ARITH, 0, "IB3", {0}, {0}, &IB2 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IB3 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
iC_Gt QB0_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB0_0", {0}, {&iC_l_[0]}, &IX0_3 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[3]}, &QB0_0 };
iC_Gt QB2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB2_0", {0}, {&iC_l_[6]}, &QB1_0 };
iC_Gt QB3_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB3_0", {0}, {&iC_l_[9]}, &QB2_0 };
static iC_Gt _f1_1   = { 1, -iC_OR, iC_F_CE, 0, "_f1_1", {&iC_l_[12]}, {&iC_l_[14]}, &QB3_0 };
static iC_Gt _f2_1   = { 1, -iC_OR, iC_F_CE, 0, "_f2_1", {&iC_l_[17]}, {&iC_l_[19]}, &_f1_1 };
static iC_Gt _f3_1   = { 1, -iC_OR, iC_F_CE, 0, "_f3_1", {&iC_l_[22]}, {&iC_l_[24]}, &_f2_1 };
static iC_Gt _f4_1   = { 1, -iC_OR, iC_F_CE, 0, "_f4_1", {&iC_l_[27]}, {&iC_l_[29]}, &_f3_1 };
static iC_Gt _f5_1   = { 1, -iC_OR, iC_F_CE, 0, "_f5_1", {&iC_l_[32]}, {&iC_l_[34]}, &_f4_1 };
static iC_Gt _f6_1   = { 1, -iC_OR, iC_F_CE, 0, "_f6_1", {&iC_l_[37]}, {&iC_l_[39]}, &_f5_1 };
iC_Gt QB0      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB0", {0}, {(iC_Gt**)&IB0}, &_f6_1, 0 };
iC_Gt QB1      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB1", {0}, {(iC_Gt**)&IB1}, &QB0, 0 };
iC_Gt QB2      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB2", {0}, {(iC_Gt**)&IB2}, &QB1, 0 };
iC_Gt QB3      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB3", {0}, {(iC_Gt**)&IB3}, &QB2, 0 };

iC_Gt *		iC___Test0_hy_list = &QB3;
iC_Gt **	iC_list[] = { &iC___Test0_hy_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 7 "./Test0/hy.ic"

#include	<stdio.h>

#line 58 "./Test0/hy.c"

static int iC_2(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 11 "./Test0/hy.ic"
{
	    printf("Hello Nr. %d, 0\n", 1);
	}
#line 66 "./Test0/hy.c"
    else
#line 13 "./Test0/hy.ic"
{
	    printf("Bye Nr. %d, 0\n", 1);
	}
#line 72 "./Test0/hy.c"
    return 0;
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 16 "./Test0/hy.ic"
{
	    printf("Hello Nr. %d, 0\n", 2);
	}
#line 82 "./Test0/hy.c"
    else
#line 18 "./Test0/hy.ic"
{
	    printf("Bye Nr. %d, 0\n", 2);
	}
#line 88 "./Test0/hy.c"
    return 0;
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 21 "./Test0/hy.ic"
{
	    printf("Hello Nr. %d, 1\n", 2);
	}
#line 98 "./Test0/hy.c"
    else
#line 23 "./Test0/hy.ic"
{
	    printf("Bye Nr. %d, 1\n", 2);
	}
#line 104 "./Test0/hy.c"
    return 0;
} /* iC_4 */

static int iC_5(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 26 "./Test0/hy.ic"
{
	    printf("Hello Nr. %d, 0\n", 3);
	}
#line 114 "./Test0/hy.c"
    else
#line 28 "./Test0/hy.ic"
{
	    printf("Bye Nr. %d, 0\n", 3);
	}
#line 120 "./Test0/hy.c"
    return 0;
} /* iC_5 */

static int iC_6(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 31 "./Test0/hy.ic"
{
	    printf("Hello Nr. %d, 1\n", 3);
	}
#line 130 "./Test0/hy.c"
    else
#line 33 "./Test0/hy.ic"
{
	    printf("Bye Nr. %d, 1\n", 3);
	}
#line 136 "./Test0/hy.c"
    return 0;
} /* iC_6 */

static int iC_7(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 36 "./Test0/hy.ic"
{
	    printf("Hello Nr. %d, 2\n", 3);
	}
#line 146 "./Test0/hy.c"
    else
#line 38 "./Test0/hy.ic"
{
	    printf("Bye Nr. %d, 2\n", 3);
	}
#line 152 "./Test0/hy.c"
    return 0;
} /* iC_7 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB0_0 */	(iC_Gt*)0, &IB0, 0,
/* QB1_0 */	(iC_Gt*)0, &IB1, 0,
/* QB2_0 */	(iC_Gt*)0, &IB2, 0,
/* QB3_0 */	(iC_Gt*)0, &IB3, 0,
/* _f1_1 */	(iC_Gt*)iC_2, &iClock,	&IX0_1, 0, 0,
/* _f2_1 */	(iC_Gt*)iC_3, &iClock,	&IX0_2, 0, 0,
/* _f3_1 */	(iC_Gt*)iC_4, &iClock,	&IX0_2, 0, 0,
/* _f4_1 */	(iC_Gt*)iC_5, &iClock,	&IX0_3, 0, 0,
/* _f5_1 */	(iC_Gt*)iC_6, &iClock,	&IX0_3, 0, 0,
/* _f6_1 */	(iC_Gt*)iC_7, &iClock,	&IX0_3, 0, 0,
};

/********************************************************************
 *
 *	SOURCE:   ./Test0/h8.ic
 *	OUTPUT:   ./Test0/h8.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: h8.c,v 1.1 2015/10/15 06:27:21 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, 0 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_6    = { 1, -iC_INPX, iC_GATE, 0, "IX0.6", {0}, {0}, &IX0_2 };
iC_Gt IX0_7    = { 1, -iC_INPX, iC_GATE, 0, "IX0.7", {0}, {0}, &IX0_6 };
iC_Gt QX0_7    = { 1, -iC_AND, iC_GATE, 0, "QX0.7", {0}, {&iC_l_[0]}, &IX0_7 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[4]}, &QX0_7 };
static iC_Gt _f1_1   = { 1, -iC_OR, iC_F_CF, 0, "_f1_1", {&iC_l_[7]}, {&iC_l_[9]}, &QX0_7_0 };
static iC_Gt _f2_1   = { 1, -iC_OR, iC_F_CF, 0, "_f2_1", {&iC_l_[12]}, {&iC_l_[14]}, &_f1_1 };
static iC_Gt _f3_1   = { 1, -iC_OR, iC_F_CF, 0, "_f3_1", {&iC_l_[17]}, {&iC_l_[19]}, &_f2_1 };

iC_Gt *		iC___Test0_h8_list = &_f3_1;
iC_Gt **	iC_list[] = { &iC___Test0_h8_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 7 "./Test0/h8.ic"

#include	<stdio.h>

#line 47 "./Test0/h8.c"

static int iC_2(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 11 "./Test0/h8.ic"
{
	printf("%d: Immediate Nr. %d\n", 0, 2);	/* { */
    }
#line 55 "./Test0/h8.c"
    return 0;
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 14 "./Test0/h8.ic"
{
	printf("%d: Immediate Nr. %d\n", 1, 2);	/* { */
    }
#line 65 "./Test0/h8.c"
    return 0;
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 17 "./Test0/h8.ic"
{
	printf("%d: Immediate Nr. %d\n", 2, 2);	/* { */
    }
#line 75 "./Test0/h8.c"
    return 0;
} /* iC_4 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QX0.7 */	&IX0_7, &IX0_6, 0, 0,
/* QX0.7_0 */	&QX0_7, 0, 0,
/* _f1_1 */	(iC_Gt*)iC_2, &iClock,	&IX0_0, 0, 0,
/* _f2_1 */	(iC_Gt*)iC_3, &iClock,	&IX0_1, 0, 0,
/* _f3_1 */	(iC_Gt*)iC_4, &iClock,	&IX0_2, 0, 0,
};

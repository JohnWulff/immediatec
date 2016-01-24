/********************************************************************
 *
 *	SOURCE:   ./Test0/h5.ic
 *	OUTPUT:   ./Test0/h5.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: h5.c,v 1.1 2015/10/15 06:27:20 jw Exp $ -O7";

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
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_7    = { 1, -iC_INPX, iC_GATE, 0, "IX0.7", {0}, {0}, &IX0_2 };
static iC_Gt _f1_1   = { 1, -iC_OR, iC_F_CE, 0, "_f1_1", {&iC_l_[0]}, {&iC_l_[2]}, &IX0_7 };
static iC_Gt _f2_1   = { 1, -iC_OR, iC_F_CF, 0, "_f2_1", {&iC_l_[5]}, {&iC_l_[7]}, &_f1_1 };
static iC_Gt _f3_1   = { 1, -iC_OR, iC_F_CF, 0, "_f3_1", {&iC_l_[10]}, {&iC_l_[12]}, &_f2_1 };
static iC_Gt _f4_1   = { 1, -iC_OR, iC_F_CF, 0, "_f4_1", {&iC_l_[15]}, {&iC_l_[17]}, &_f3_1 };
static iC_Gt _f5_1   = { 1, -iC_ARN, iC_F_SW, 0, "_f5_1", {&iC_l_[20]}, {&iC_l_[22]}, &_f4_1 };

iC_Gt *		iC___Test0_h5_list = &_f5_1;
iC_Gt **	iC_list[] = { &iC___Test0_h5_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 7 "./Test0/h5.ic"

#include	<stdio.h>

#line 47 "./Test0/h5.c"

#line 11 "./Test0/h5.ic"

    int n;

#line 53 "./Test0/h5.c"

static int iC_2(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 15 "./Test0/h5.ic"
{
  for (n = 0; n <= 2; n++) {
    printf("%d: Hello world Nr. %d\n", n, 2);
  }
}
#line 63 "./Test0/h5.c"
    else
#line 19 "./Test0/h5.ic"
{
  for (n = 0; n <= 2; n++) {
    printf("%d: Good bye Nr. %d\n", n, 2);
  }
}
#line 71 "./Test0/h5.c"
    return 0;
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 25 "./Test0/h5.ic"
{
	printf("%d: Immediate Nr. %d\n", 0, 2);
    }
#line 81 "./Test0/h5.c"
    return 0;
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 28 "./Test0/h5.ic"
{
	printf("%d: Immediate Nr. %d\n", 1, 2);
    }
#line 91 "./Test0/h5.c"
    return 0;
} /* iC_4 */

static int iC_5(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 31 "./Test0/h5.ic"
{
	printf("%d: Immediate Nr. %d\n", 2, 2);
    }
#line 101 "./Test0/h5.c"
    return 0;
} /* iC_5 */

static int iC_6(iC_Gt * iC_gf) {
    switch (iC_gf->gt_new)
#line 35 "./Test0/h5.ic"
{
case 0:
  for (n = 0; n <= 2; n++) {
    printf("%d: Low Nr. %d\n", n, 0);
  }
    break;
case 1:
  for (n = 0; n <= 2; n++) {
    printf("%d: Med Nr. %d\n", n, 1);
  }
    break;
case 2:
  for (n = 0; n <= 2; n++) {
    printf("%d: Hi  Nr. %d\n", n, 2);
  }
    break;
default:
  for (n = 0; n <= 2; n++) {
    printf("%d: Off Nr. %d\n", n, 99);
  }
    break;
}
#line 130 "./Test0/h5.c"
    return 0;
} /* iC_6 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* _f1_1 */	(iC_Gt*)iC_2, &iClock,	&IX0_7, 0, 0,
/* _f2_1 */	(iC_Gt*)iC_3, &iClock,	&IX0_0, 0, 0,
/* _f3_1 */	(iC_Gt*)iC_4, &iClock,	&IX0_1, 0, 0,
/* _f4_1 */	(iC_Gt*)iC_5, &iClock,	&IX0_2, 0, 0,
/* _f5_1 */	(iC_Gt*)iC_6, &iClock,	(iC_Gt*)0, &IB1, 0,
};

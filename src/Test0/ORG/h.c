/********************************************************************
 *
 *	SOURCE:   ./Test0/h.ic
 *	OUTPUT:   ./Test0/h.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: h.c,v 1.1 2015/10/15 06:27:20 jw Exp $ -O7";

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
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
iC_Gt IX0_4    = { 1, -iC_INPX, iC_GATE, 0, "IX0.4", {0}, {0}, &IX0_3 };
iC_Gt IX0_5    = { 1, -iC_INPX, iC_GATE, 0, "IX0.5", {0}, {0}, &IX0_4 };
iC_Gt IX0_6    = { 1, -iC_INPX, iC_GATE, 0, "IX0.6", {0}, {0}, &IX0_5 };
iC_Gt IX0_7    = { 1, -iC_INPX, iC_GATE, 0, "IX0.7", {0}, {0}, &IX0_6 };
iC_Gt IX1_0    = { 1, -iC_INPX, iC_GATE, 0, "IX1.0", {0}, {0}, &IX0_7 };
iC_Gt IX1_1    = { 1, -iC_INPX, iC_GATE, 0, "IX1.1", {0}, {0}, &IX1_0 };
static iC_Gt _f10_1  = { 1, -iC_OR, iC_F_CF, 0, "_f10_1", {&iC_l_[0]}, {&iC_l_[2]}, &IX1_1 };
static iC_Gt _f1_1   = { 1, -iC_OR, iC_F_CF, 0, "_f1_1", {&iC_l_[5]}, {&iC_l_[7]}, &_f10_1 };
static iC_Gt _f2_1   = { 1, -iC_OR, iC_F_CF, 0, "_f2_1", {&iC_l_[10]}, {&iC_l_[12]}, &_f1_1 };
static iC_Gt _f3_1   = { 1, -iC_OR, iC_F_CF, 0, "_f3_1", {&iC_l_[15]}, {&iC_l_[17]}, &_f2_1 };
static iC_Gt _f4_1   = { 1, -iC_OR, iC_F_CF, 0, "_f4_1", {&iC_l_[20]}, {&iC_l_[22]}, &_f3_1 };
static iC_Gt _f5_1   = { 1, -iC_OR, iC_F_CF, 0, "_f5_1", {&iC_l_[25]}, {&iC_l_[27]}, &_f4_1 };
static iC_Gt _f6_1   = { 1, -iC_OR, iC_F_CF, 0, "_f6_1", {&iC_l_[30]}, {&iC_l_[32]}, &_f5_1 };
static iC_Gt _f7_1   = { 1, -iC_OR, iC_F_CF, 0, "_f7_1", {&iC_l_[35]}, {&iC_l_[37]}, &_f6_1 };
static iC_Gt _f8_1   = { 1, -iC_OR, iC_F_CF, 0, "_f8_1", {&iC_l_[40]}, {&iC_l_[42]}, &_f7_1 };
static iC_Gt _f9_1   = { 1, -iC_OR, iC_F_CF, 0, "_f9_1", {&iC_l_[45]}, {&iC_l_[47]}, &_f8_1 };

iC_Gt *		iC___Test0_h_list = &_f9_1;
iC_Gt **	iC_list[] = { &iC___Test0_h_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 7 "./Test0/h.ic"

#include	<stdio.h>

#line 57 "./Test0/h.c"

static int iC_2(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 11 "./Test0/h.ic"
{
	printf("Hello Nr. %d\n", 0);
    }
#line 65 "./Test0/h.c"
    return 0;
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 14 "./Test0/h.ic"
{
	printf("Hello Nr. %d\n", 1);
    }
#line 75 "./Test0/h.c"
    return 0;
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 17 "./Test0/h.ic"
{
	printf("Hello Nr. %d\n", 2);
    }
#line 85 "./Test0/h.c"
    return 0;
} /* iC_4 */

static int iC_5(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 20 "./Test0/h.ic"
{
	printf("Hello Nr. %d\n", 3);
    }
#line 95 "./Test0/h.c"
    return 0;
} /* iC_5 */

static int iC_6(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 23 "./Test0/h.ic"
{
	printf("Hello Nr. %d\n", 4);
    }
#line 105 "./Test0/h.c"
    return 0;
} /* iC_6 */

static int iC_7(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 26 "./Test0/h.ic"
{
	printf("Hello Nr. %d\n", 5);
    }
#line 115 "./Test0/h.c"
    return 0;
} /* iC_7 */

static int iC_8(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 29 "./Test0/h.ic"
{
	printf("Hello Nr. %d\n", 6);
    }
#line 125 "./Test0/h.c"
    return 0;
} /* iC_8 */

static int iC_9(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 32 "./Test0/h.ic"
{
	printf("Hello Nr. %d\n", 7);
    }
#line 135 "./Test0/h.c"
    return 0;
} /* iC_9 */

static int iC_10(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 35 "./Test0/h.ic"
{
	printf("Hello Nr. %d\n", 8);
    }
#line 145 "./Test0/h.c"
    return 0;
} /* iC_10 */

static int iC_11(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 38 "./Test0/h.ic"
{
	printf("Hello Nr. %d\n", 9);
    }
#line 155 "./Test0/h.c"
    return 0;
} /* iC_11 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* _f10_1 */	(iC_Gt*)iC_11, &iClock,	&IX1_1, 0, 0,
/* _f1_1 */	(iC_Gt*)iC_2, &iClock,	&IX0_0, 0, 0,
/* _f2_1 */	(iC_Gt*)iC_3, &iClock,	&IX0_1, 0, 0,
/* _f3_1 */	(iC_Gt*)iC_4, &iClock,	&IX0_2, 0, 0,
/* _f4_1 */	(iC_Gt*)iC_5, &iClock,	&IX0_3, 0, 0,
/* _f5_1 */	(iC_Gt*)iC_6, &iClock,	&IX0_4, 0, 0,
/* _f6_1 */	(iC_Gt*)iC_7, &iClock,	&IX0_5, 0, 0,
/* _f7_1 */	(iC_Gt*)iC_8, &iClock,	&IX0_6, 0, 0,
/* _f8_1 */	(iC_Gt*)iC_9, &iClock,	&IX0_7, 0, 0,
/* _f9_1 */	(iC_Gt*)iC_10, &iClock,	&IX1_0, 0, 0,
};

/********************************************************************
 *
 *	SOURCE:   ./Test0/alex10.ic
 *	OUTPUT:   ./Test0/alex10.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: alex10.c,v 1.1 2015/05/24 00:51:48 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB4      = { 1, -iC_INPW, iC_ARITH, 0, "IB4", {0}, {0}, 0 };
iC_Gt IB5      = { 1, -iC_INPW, iC_ARITH, 0, "IB5", {0}, {0}, &IB4 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB5 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[0]}, &IX0_3 };
iC_Gt QW2      = { 1, -iC_ARN, iC_ARITH, 0, "QW2", {0}, {&iC_l_[3]}, &QB1_0 };
iC_Gt QW2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QW2_0", {0}, {&iC_l_[6]}, &QW2 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[9]}, &QW2_0 };
iC_Gt QX0_1    = { 1, -iC_OR, iC_GATE, 0, "QX0.1", {0}, {&iC_l_[12]}, &QX0_0_0 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[18]}, &QX0_1 };
iC_Gt QX0_2    = { 1, -iC_OR, iC_GATE, 0, "QX0.2", {0}, {&iC_l_[21]}, &QX0_1_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[25]}, &QX0_2 };
iC_Gt QX0_2_1  = { 1, -iC_AND, iC_GATE, 0, "QX0.2_1", {0}, {&iC_l_[28]}, &QX0_2_0 };
iC_Gt QX0_2_2  = { 1, -iC_AND, iC_GATE, 0, "QX0.2_2", {0}, {&iC_l_[32]}, &QX0_2_1 };
iC_Gt QX0_3    = { 1, -iC_AND, iC_GATE, 0, "QX0.3", {0}, {&iC_l_[36]}, &QX0_2_2 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[42]}, &QX0_3 };
iC_Gt QX0_4    = { 1, -iC_ARN, iC_GATE, 0, "QX0.4", {0}, {&iC_l_[45]}, &QX0_3_0 };
iC_Gt QX0_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.4_0", {0}, {&iC_l_[49]}, &QX0_4 };
iC_Gt QX0_5    = { 1, -iC_ARN, iC_GATE, 0, "QX0.5", {0}, {&iC_l_[52]}, &QX0_4_0 };
iC_Gt QX0_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.5_0", {0}, {&iC_l_[56]}, &QX0_5 };
iC_Gt QX0_6    = { 1, -iC_ARN, iC_GATE, 0, "QX0.6", {0}, {&iC_l_[59]}, &QX0_5_0 };
iC_Gt QX0_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.6_0", {0}, {&iC_l_[63]}, &QX0_6 };
iC_Gt QX0_7    = { 1, -iC_OR, iC_GATE, 0, "QX0.7", {0}, {&iC_l_[66]}, &QX0_6_0 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[72]}, &QX0_7 };
iC_Gt QX0_7_1  = { 1, -iC_ARN, iC_GATE, 0, "QX0.7_1", {0}, {&iC_l_[75]}, &QX0_7_0 };
iC_Gt QX0_7_2  = { 1, -iC_ARN, iC_GATE, 0, "QX0.7_2", {0}, {&iC_l_[78]}, &QX0_7_1 };
iC_Gt QX0_7_3  = { 1, -iC_ARN, iC_GATE, 0, "QX0.7_3", {0}, {&iC_l_[81]}, &QX0_7_2 };
iC_Gt QX0_7_4  = { 1, -iC_ARN, iC_GATE, 0, "QX0.7_4", {0}, {&iC_l_[84]}, &QX0_7_3 };
static iC_Gt _f1_1   = { 1, -iC_ARN, iC_F_CF, 0, "_f1_1", {&iC_l_[87]}, {&iC_l_[89]}, &QX0_7_4 };
iC_Gt sum      = { 1, -iC_ARN, iC_ARITH, 0, "sum", {0}, {&iC_l_[93]}, &_f1_1 };
iC_Gt QB1      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB1", {0}, {(iC_Gt**)&IB5}, &sum, 0 };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&IX0_0}, &QB1, 0 };
iC_Gt x        = { 1, -iC_ALIAS, iC_ARITH, 0, "x", {0}, {(iC_Gt**)&IB4}, &QX0_0, 0 };
iC_Gt y        = { 1, -iC_ALIAS, iC_ARITH, 0, "y", {0}, {(iC_Gt**)&IB5}, &x, 0 };

iC_Gt *		iC___Test0_alex10_list = &y;
iC_Gt **	iC_list[] = { &iC___Test0_alex10_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 5 "./Test0/alex10.ic"

#include	<stdio.h>

#line 73 "./Test0/alex10.c"

static int iC_2(iC_Gt * iC_gf) {
#line 21 "./Test0/alex10.ic"
	return !(iC_MV(1)>100)?iC_MV(1):200-iC_MV(1);
#line 78 "./Test0/alex10.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
#line 22 "./Test0/alex10.ic"
	return iC_MV(1)+iC_MV(2);
#line 84 "./Test0/alex10.c"
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
#line 24 "./Test0/alex10.ic"
	return iC_MV(1)<iC_MV(2);
#line 90 "./Test0/alex10.c"
} /* iC_4 */

static int iC_5(iC_Gt * iC_gf) {
#line 25 "./Test0/alex10.ic"
	return iC_MV(1)==iC_MV(2);
#line 96 "./Test0/alex10.c"
} /* iC_5 */

static int iC_6(iC_Gt * iC_gf) {
#line 26 "./Test0/alex10.ic"
	return iC_MV(1)>iC_MV(2);
#line 102 "./Test0/alex10.c"
} /* iC_6 */

static int iC_7(iC_Gt * iC_gf) {
#line 28 "./Test0/alex10.ic"
	return iC_MV(1)==100;
#line 108 "./Test0/alex10.c"
} /* iC_7 */

static int iC_8(iC_Gt * iC_gf) {
#line 28 "./Test0/alex10.ic"
	return iC_MV(1)==0;
#line 114 "./Test0/alex10.c"
} /* iC_8 */

static int iC_9(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 30 "./Test0/alex10.ic"
{ printf("alex was here !\n"); }
#line 121 "./Test0/alex10.c"
    return 0;
} /* iC_9 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1_0 */	(iC_Gt*)0, &IB5, 0,
/* QW2 */	(iC_Gt*)iC_2, &sum, 0,
/* QW2_0 */	(iC_Gt*)0, &QW2, 0,
/* QX0.0_0 */	&IX0_0, 0, 0,
/* QX0.1 */	&IX0_3, &IX0_2, &IX0_1, &IX0_0, 0, 0,
/* QX0.1_0 */	&QX0_1, 0, 0,
/* QX0.2 */	&QX0_2_2, &QX0_2_1, 0, 0,
/* QX0.2_0 */	&QX0_2, 0, 0,
/* QX0.2_1 */	&IX0_1, &IX0_0, 0, 0,
/* QX0.2_2 */	&IX0_3, &IX0_2, 0, 0,
/* QX0.3 */	&IX0_3, &IX0_2, &IX0_1, &IX0_0, 0, 0,
/* QX0.3_0 */	&QX0_3, 0, 0,
/* QX0.4 */	(iC_Gt*)iC_4, &IB4, &IB5, 0,
/* QX0.4_0 */	&QX0_4, 0, 0,
/* QX0.5 */	(iC_Gt*)iC_5, &IB4, &IB5, 0,
/* QX0.5_0 */	&QX0_5, 0, 0,
/* QX0.6 */	(iC_Gt*)iC_6, &IB4, &IB5, 0,
/* QX0.6_0 */	&QX0_6, 0, 0,
/* QX0.7 */	&QX0_7_4, &QX0_7_3, &QX0_7_2, &QX0_7_1, 0, 0,
/* QX0.7_0 */	&QX0_7, 0, 0,
/* QX0.7_1 */	(iC_Gt*)iC_8, &IB4, 0,
/* QX0.7_2 */	(iC_Gt*)iC_7, &IB4, 0,
/* QX0.7_3 */	(iC_Gt*)iC_8, &IB5, 0,
/* QX0.7_4 */	(iC_Gt*)iC_7, &IB5, 0,
/* _f1_1 */	(iC_Gt*)iC_9, &iClock,	(iC_Gt*)iC_4, &IB4, &IB5, 0,
/* sum */	(iC_Gt*)iC_3, &IB4, &IB5, 0,
};

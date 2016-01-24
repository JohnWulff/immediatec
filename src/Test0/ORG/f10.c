/********************************************************************
 *
 *	SOURCE:   ./Test0/f10.ic
 *	OUTPUT:   ./Test0/f10.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: f10.c,v 1.1 2015/05/24 00:51:51 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, 0 };
iC_Gt IB2      = { 1, -iC_INPW, iC_ARITH, 0, "IB2", {0}, {0}, &IB1 };
iC_Gt IW10     = { 1, -iC_INPW, iC_ARITH, 0, "IW10", {0}, {0}, &IB2 };
iC_Gt IW8      = { 1, -iC_INPW, iC_ARITH, 0, "IW8", {0}, {0}, &IW10 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IW8 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt QB0      = { 1, -iC_ARN, iC_ARITH, 0, "QB0", {0}, {&iC_l_[0]}, &IX0_2 };
iC_Gt QB0_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB0_0", {0}, {&iC_l_[3]}, &QB0 };
iC_Gt QB1      = { 1, -iC_ARN, iC_ARITH, 0, "QB1", {0}, {&iC_l_[6]}, &QB0_0 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[9]}, &QB1 };
iC_Gt QB2      = { 1, -iC_ARN, iC_ARITH, 0, "QB2", {0}, {&iC_l_[12]}, &QB1_0 };
iC_Gt QB2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB2_0", {0}, {&iC_l_[15]}, &QB2 };
iC_Gt QB3      = { 1, -iC_ARN, iC_ARITH, 0, "QB3", {0}, {&iC_l_[18]}, &QB2_0 };
iC_Gt QB3_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB3_0", {0}, {&iC_l_[22]}, &QB3 };
iC_Gt QB4      = { 1, -iC_ARN, iC_ARITH, 0, "QB4", {0}, {&iC_l_[25]}, &QB3_0 };
iC_Gt QB4_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB4_0", {0}, {&iC_l_[28]}, &QB4 };
iC_Gt QB5      = { 1, -iC_ARN, iC_ARITH, 0, "QB5", {0}, {&iC_l_[31]}, &QB4_0 };
iC_Gt QB5_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB5_0", {0}, {&iC_l_[34]}, &QB5 };
iC_Gt QB6      = { 1, -iC_ARN, iC_ARITH, 0, "QB6", {0}, {&iC_l_[37]}, &QB5_0 };
iC_Gt QB6_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB6_0", {0}, {&iC_l_[40]}, &QB6 };
iC_Gt QB7      = { 1, -iC_ARN, iC_ARITH, 0, "QB7", {0}, {&iC_l_[43]}, &QB6_0 };
iC_Gt QB7_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB7_0", {0}, {&iC_l_[47]}, &QB7 };
iC_Gt QB7_1    = { 1, -iC_OR, iC_ARITH, 0, "QB7_1", {0}, {&iC_l_[50]}, &QB7_0 };
iC_Gt QB7_2    = { 1, -iC_AND, iC_GATE, 0, "QB7_2", {0}, {&iC_l_[54]}, &QB7_1 };
iC_Gt QW10     = { 1, -iC_ARN, iC_ARITH, 0, "QW10", {0}, {&iC_l_[58]}, &QB7_2 };
iC_Gt QW10_0   = { 1, -iC_ARN, iC_OUTW, 0, "QW10_0", {0}, {&iC_l_[62]}, &QW10 };
iC_Gt QW8      = { 1, -iC_ARN, iC_ARITH, 0, "QW8", {0}, {&iC_l_[65]}, &QW10_0 };
iC_Gt QW8_0    = { 1, -iC_ARN, iC_OUTW, 0, "QW8_0", {0}, {&iC_l_[69]}, &QW8 };

iC_Gt *		iC___Test0_f10_list = &QW8_0;
iC_Gt **	iC_list[] = { &iC___Test0_f10_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 1 "./Test0/f10.ic"

#include <stdlib.h>
static int Div(int numer, int denom);
static int Remainder(int numer, int denom);

#line 68 "./Test0/f10.c"

#line 11 "./Test0/f10.ic"

#define max(x,y) ((x) > (y) ? (x) : (y))
#define min(x,y) ((x) < (y) ? (x) : (y))

#line 75 "./Test0/f10.c"

#line 24 "./Test0/f10.ic"

static int
Div(int numer, int denom)
{
    return denom ? div(numer, denom).quot : 0;
}

static int
Remainder(int numer, int denom)
{
    return denom ? div(numer, denom).rem : 0;
}

#line 91 "./Test0/f10.c"

static int iC_2(iC_Gt * iC_gf) {
#line 7 "./Test0/f10.ic"
	return rand()+iC_MV(1);
#line 96 "./Test0/f10.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
#line 8 "./Test0/f10.ic"
	return iC_MV(1)+rand();
#line 102 "./Test0/f10.c"
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
#line 9 "./Test0/f10.ic"
	return abs(iC_MV(1));
#line 108 "./Test0/f10.c"
} /* iC_4 */

static int iC_5(iC_Gt * iC_gf) {
#line 16 "./Test0/f10.ic"
	return min(iC_MV(1),iC_MV(2));
#line 114 "./Test0/f10.c"
} /* iC_5 */

static int iC_6(iC_Gt * iC_gf) {
#line 17 "./Test0/f10.ic"
	return min(20,iC_MV(1));
#line 120 "./Test0/f10.c"
} /* iC_6 */

static int iC_7(iC_Gt * iC_gf) {
#line 18 "./Test0/f10.ic"
	return max(iC_MV(1),10);
#line 126 "./Test0/f10.c"
} /* iC_7 */

static int iC_8(iC_Gt * iC_gf) {
#line 19 "./Test0/f10.ic"
	return min(20,max(iC_MV(1),10));
#line 132 "./Test0/f10.c"
} /* iC_8 */

static int iC_9(iC_Gt * iC_gf) {
#line 20 "./Test0/f10.ic"
	return min(max(iC_MV(1),iC_MV(2)),20);
#line 138 "./Test0/f10.c"
} /* iC_9 */

static int iC_10(iC_Gt * iC_gf) {
#line 21 "./Test0/f10.ic"
	return Div(iC_MV(1),10)+iC_MV(2);
#line 144 "./Test0/f10.c"
} /* iC_10 */

static int iC_11(iC_Gt * iC_gf) {
#line 22 "./Test0/f10.ic"
	return Remainder(iC_MV(1),10)+iC_MV(2);
#line 150 "./Test0/f10.c"
} /* iC_11 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB0 */	(iC_Gt*)iC_2, &IB1, 0,
/* QB0_0 */	(iC_Gt*)0, &QB0, 0,
/* QB1 */	(iC_Gt*)iC_3, &IB1, 0,
/* QB1_0 */	(iC_Gt*)0, &QB1, 0,
/* QB2 */	(iC_Gt*)iC_4, &IB1, 0,
/* QB2_0 */	(iC_Gt*)0, &QB2, 0,
/* QB3 */	(iC_Gt*)iC_5, &IB1, &IB2, 0,
/* QB3_0 */	(iC_Gt*)0, &QB3, 0,
/* QB4 */	(iC_Gt*)iC_6, &IB2, 0,
/* QB4_0 */	(iC_Gt*)0, &QB4, 0,
/* QB5 */	(iC_Gt*)iC_7, &IB1, 0,
/* QB5_0 */	(iC_Gt*)0, &QB5, 0,
/* QB6 */	(iC_Gt*)iC_8, &IB2, 0,
/* QB6_0 */	(iC_Gt*)0, &QB6, 0,
/* QB7 */	(iC_Gt*)iC_9, &QB7_1, &IB2, 0,
/* QB7_0 */	(iC_Gt*)0, &QB7, 0,
/* QB7_1 */	&QB7_2, &IX0_2, 0, 0,
/* QB7_2 */	&IX0_1, &IX0_0, 0, 0,
/* QW10 */	(iC_Gt*)iC_11, &IW8, &IW10, 0,
/* QW10_0 */	(iC_Gt*)0, &QW10, 0,
/* QW8 */	(iC_Gt*)iC_10, &IW8, &IW10, 0,
/* QW8_0 */	(iC_Gt*)0, &QW8, 0,
};

/********************************************************************
 *
 *	SOURCE:   ./Test0/aa4z2.ic
 *	OUTPUT:   ./Test0/aa4z2.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"$Revision: icc_2.6-6-ge1cef2e-dirty $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

static iC_Gt _18     = { 1, -iC_NCONST, iC_ARITH, 0, "18", {0}, {0}, 0 };
iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, &_18 };
iC_Gt IB2      = { 1, -iC_INPW, iC_ARITH, 0, "IB2", {0}, {0}, &IB1 };
iC_Gt IB3      = { 1, -iC_INPW, iC_ARITH, 0, "IB3", {0}, {0}, &IB2 };
iC_Gt IB4      = { 1, -iC_INPW, iC_ARITH, 0, "IB4", {0}, {0}, &IB3 };
iC_Gt QB1      = { 1, -iC_ARN, iC_ARITH, 0, "QB1", {0}, {&iC_l_[0]}, &IB4 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[4]}, &QB1 };
iC_Gt QB2      = { 1, -iC_ARN, iC_ARITH, 0, "QB2", {0}, {&iC_l_[7]}, &QB1_0 };
iC_Gt QB2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB2_0", {0}, {&iC_l_[11]}, &QB2 };
iC_Gt QB3      = { 1, -iC_ARN, iC_ARITH, 0, "QB3", {0}, {&iC_l_[14]}, &QB2_0 };
iC_Gt QB3_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB3_0", {0}, {&iC_l_[20]}, &QB3 };
iC_Gt QB4      = { 1, -iC_ARN, iC_ARITH, 0, "QB4", {0}, {&iC_l_[23]}, &QB3_0 };
iC_Gt QB4_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB4_0", {0}, {&iC_l_[27]}, &QB4 };
iC_Gt QB5      = { 1, -iC_ARN, iC_ARITH, 0, "QB5", {0}, {&iC_l_[30]}, &QB4_0 };
iC_Gt QB5_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB5_0", {0}, {&iC_l_[34]}, &QB5 };
iC_Gt QB6_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB6_0", {0}, {&iC_l_[37]}, &QB5_0 };
iC_Gt QB6      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB6", {0}, {(iC_Gt**)&_18}, &QB6_0, 0 };

iC_Gt *		iC___Test0_aa4z2_list = &QB6;
iC_Gt **	iC_list[] = { &iC___Test0_aa4z2_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_8(iC_Gt * iC_gf) {
#line 46 "./Test0/aa4z2.ic"
	return ((iC_MV(1)+iC_MV(2))+4)+4;
#line 53 "./Test0/aa4z2.c"
} /* iC_8 */

static int iC_9(iC_Gt * iC_gf) {
#line 47 "./Test0/aa4z2.ic"
	return 4+(4+(iC_MV(1)+iC_MV(2)));
#line 59 "./Test0/aa4z2.c"
} /* iC_9 */

static int iC_10(iC_Gt * iC_gf) {
#line 48 "./Test0/aa4z2.ic"
	return ((iC_MV(1)+iC_MV(2))+(iC_MV(3)+iC_MV(4)))+(iC_MV(3)+iC_MV(4));
#line 65 "./Test0/aa4z2.c"
} /* iC_10 */

static int iC_11(iC_Gt * iC_gf) {
#line 49 "./Test0/aa4z2.ic"
	return 8+(8+(iC_MV(1)+iC_MV(2)));
#line 71 "./Test0/aa4z2.c"
} /* iC_11 */

static int iC_12(iC_Gt * iC_gf) {
#line 50 "./Test0/aa4z2.ic"
	return ((iC_MV(1)+iC_MV(2))+8)+8;
#line 77 "./Test0/aa4z2.c"
} /* iC_12 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1 */	(iC_Gt*)iC_8, &IB1, &IB2, 0,
/* QB1_0 */	(iC_Gt*)0, &QB1, 0,
/* QB2 */	(iC_Gt*)iC_9, &IB3, &IB4, 0,
/* QB2_0 */	(iC_Gt*)0, &QB2, 0,
/* QB3 */	(iC_Gt*)iC_10, &IB1, &IB2, &IB3, &IB4, 0,
/* QB3_0 */	(iC_Gt*)0, &QB3, 0,
/* QB4 */	(iC_Gt*)iC_11, &IB3, &IB4, 0,
/* QB4_0 */	(iC_Gt*)0, &QB4, 0,
/* QB5 */	(iC_Gt*)iC_12, &IB3, &IB4, 0,
/* QB5_0 */	(iC_Gt*)0, &QB5, 0,
/* QB6_0 */	(iC_Gt*)0, &_18, 0,
};

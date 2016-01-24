/********************************************************************
 *
 *	SOURCE:   ./Test0/plus1.ic
 *	OUTPUT:   ./Test0/plus1.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: plus1.c,v 1.1 2015/05/24 00:51:54 jw Exp $ -O7";

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
iC_Gt QB0      = { 1, -iC_ARN, iC_ARITH, 0, "QB0", {0}, {&iC_l_[0]}, &IB3 };
iC_Gt QB0_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB0_0", {0}, {&iC_l_[4]}, &QB0 };
iC_Gt QB1      = { 1, -iC_ARN, iC_ARITH, 0, "QB1", {0}, {&iC_l_[7]}, &QB0_0 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[13]}, &QB1 };
iC_Gt QB2      = { 1, -iC_ARN, iC_ARITH, 0, "QB2", {0}, {&iC_l_[16]}, &QB1_0 };
iC_Gt QB2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB2_0", {0}, {&iC_l_[22]}, &QB2 };
iC_Gt QB3      = { 1, -iC_ARN, iC_ARITH, 0, "QB3", {0}, {&iC_l_[25]}, &QB2_0 };
iC_Gt QB3_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB3_0", {0}, {&iC_l_[31]}, &QB3 };
iC_Gt QB4      = { 1, -iC_ARN, iC_ARITH, 0, "QB4", {0}, {&iC_l_[34]}, &QB3_0 };
iC_Gt QB4_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB4_0", {0}, {&iC_l_[40]}, &QB4 };

iC_Gt *		iC___Test0_plus1_list = &QB4_0;
iC_Gt **	iC_list[] = { &iC___Test0_plus1_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_2(iC_Gt * iC_gf) {
#line 24 "./Test0/plus1.ic"
	return iC_MV(1)+iC_MV(2);
#line 50 "./Test0/plus1.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
#line 28 "./Test0/plus1.ic"
	return (iC_MV(1)+iC_MV(2))+(iC_MV(3)+iC_MV(4));
#line 56 "./Test0/plus1.c"
} /* iC_3 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB0 */	(iC_Gt*)iC_2, &IB0, &IB1, 0,
/* QB0_0 */	(iC_Gt*)0, &QB0, 0,
/* QB1 */	(iC_Gt*)iC_3, &IB0, &IB1, &IB2, &IB3, 0,
/* QB1_0 */	(iC_Gt*)0, &QB1, 0,
/* QB2 */	(iC_Gt*)iC_3, &IB0, &IB1, &IB2, &IB3, 0,
/* QB2_0 */	(iC_Gt*)0, &QB2, 0,
/* QB3 */	(iC_Gt*)iC_3, &IB0, &IB1, &IB2, &IB3, 0,
/* QB3_0 */	(iC_Gt*)0, &QB3, 0,
/* QB4 */	(iC_Gt*)iC_3, &IB0, &IB1, &IB2, &IB3, 0,
/* QB4_0 */	(iC_Gt*)0, &QB4, 0,
};

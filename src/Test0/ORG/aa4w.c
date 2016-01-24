/********************************************************************
 *
 *	SOURCE:   ./Test0/aa4w.ic
 *	OUTPUT:   ./Test0/aa4w.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: aa4w.c,v 1.1 2015/05/24 00:51:47 jw Exp $ -O7";

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
iC_Gt QB1      = { 1, -iC_ARN, iC_ARITH, 0, "QB1", {0}, {&iC_l_[0]}, &IB2 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[3]}, &QB1 };
iC_Gt QB2      = { 1, -iC_ARN, iC_ARITH, 0, "QB2", {0}, {&iC_l_[6]}, &QB1_0 };
iC_Gt QB2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB2_0", {0}, {&iC_l_[9]}, &QB2 };
iC_Gt QB2_1    = { 1, -iC_OR, iC_ARITH, 0, "QB2_1", {0}, {&iC_l_[12]}, &QB2_0 };
iC_Gt x        = { 1, -iC_ARN, iC_ARITH, 0, "x", {0}, {&iC_l_[15]}, &QB2_1 };
iC_Gt y        = { 1, -iC_ARN, iC_GATE, 0, "y", {0}, {&iC_l_[18]}, &x };

iC_Gt *		iC___Test0_aa4w_list = &y;
iC_Gt **	iC_list[] = { &iC___Test0_aa4w_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_2(iC_Gt * iC_gf) {
#line 2 "./Test0/aa4w.ic"
	return iC_MV(1)+2;
#line 45 "./Test0/aa4w.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
#line 10 "./Test0/aa4w.ic"
	return iC_MV(1)+3;
#line 51 "./Test0/aa4w.c"
} /* iC_3 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1 */	(iC_Gt*)iC_3, &x, 0,
/* QB1_0 */	(iC_Gt*)0, &QB1, 0,
/* QB2 */	(iC_Gt*)iC_3, &QB2_1, 0,
/* QB2_0 */	(iC_Gt*)0, &QB2, 0,
/* QB2_1 */	&y, 0, 0,
/* x */		(iC_Gt*)iC_2, &IB1, 0,
/* y */		(iC_Gt*)iC_2, &IB2, 0,
};

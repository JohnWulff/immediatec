/********************************************************************
 *
 *	SOURCE:   ./Test0/aa4f1.ic
 *	OUTPUT:   ./Test0/aa4f1.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: aa4f1.c,v 1.1 2015/05/24 00:51:46 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, 0 };
iC_Gt QB1      = { 1, -iC_ARN, iC_ARITH, 0, "QB1", {0}, {&iC_l_[0]}, &IB1 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[3]}, &QB1 };
iC_Gt QB2      = { 1, -iC_ARN, iC_ARITH, 0, "QB2", {0}, {&iC_l_[6]}, &QB1_0 };
iC_Gt QB2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB2_0", {0}, {&iC_l_[9]}, &QB2 };
iC_Gt QX0_0    = { 1, -iC_ARN, iC_GATE, 0, "QX0.0", {0}, {&iC_l_[12]}, &QB2_0 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[15]}, &QX0_0 };
extern iC_Gt iConst; /* 1 */

iC_Gt *		iC___Test0_aa4f1_list = &QX0_0_0;
iC_Gt **	iC_list[] = { &iC___Test0_aa4f1_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_2(iC_Gt * iC_gf) {
#line 1 "./Test0/aa4f1.ic"
	return ~iC_MV(1);
#line 44 "./Test0/aa4f1.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
#line 3 "./Test0/aa4f1.ic"
	return ~4;
#line 50 "./Test0/aa4f1.c"
} /* iC_3 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1 */	(iC_Gt*)iC_2, &IB1, 0,
/* QB1_0 */	(iC_Gt*)0, &QB1, 0,
/* QB2 */	(iC_Gt*)iC_3, &iConst, 0,
/* QB2_0 */	(iC_Gt*)0, &QB2, 0,
/* QX0.0 */	(iC_Gt*)iC_2, &IB1, 0,
/* QX0.0_0 */	&QX0_0, 0, 0,
};

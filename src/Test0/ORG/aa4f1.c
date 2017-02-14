/********************************************************************
 *
 *	SOURCE:   ./Test0/aa4f1.ic
 *	OUTPUT:   ./Test0/aa4f1.c
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

static iC_Gt __5     = { 1, -iC_NCONST, iC_ARITH, 0, "-5", {0}, {0}, 0 };
iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, &__5 };
iC_Gt QB1      = { 1, -iC_ARN, iC_ARITH, 0, "QB1", {0}, {&iC_l_[0]}, &IB1 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[3]}, &QB1 };
iC_Gt QB2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB2_0", {0}, {&iC_l_[6]}, &QB1_0 };
iC_Gt QX0_0    = { 1, -iC_ARN, iC_GATE, 0, "QX0.0", {0}, {&iC_l_[9]}, &QB2_0 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[12]}, &QX0_0 };
iC_Gt QB2      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB2", {0}, {(iC_Gt**)&__5}, &QX0_0_0, 0 };

iC_Gt *		iC___Test0_aa4f1_list = &QB2;
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

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1 */	(iC_Gt*)iC_2, &IB1, 0,
/* QB1_0 */	(iC_Gt*)0, &QB1, 0,
/* QB2_0 */	(iC_Gt*)0, &__5, 0,
/* QX0.0 */	(iC_Gt*)iC_2, &IB1, 0,
/* QX0.0_0 */	&QX0_0, 0, 0,
};

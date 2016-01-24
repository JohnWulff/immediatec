/********************************************************************
 *
 *	SOURCE:   ./Test0/c12w.ic
 *	OUTPUT:   ./Test0/c12w.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: c12w.c,v 1.1 2015/05/24 00:51:50 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
#define iC_AVL(n)	_f0_1.gt_list[n]->gt_new
#define iC_LVL(n)	(_f0_1.gt_list[n]->gt_val < 0 ? 1 : 0)
#define iC_AAL(n,p,v)	iC_assignA(_f0_1.gt_list[n], p, v)
#define iC_LAL(n,p,v)	iC_assignL(_f0_1.gt_list[n], p, v)
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, 0 };
iC_Gt IB2      = { 1, -iC_INPW, iC_ARITH, 0, "IB2", {0}, {0}, &IB1 };
iC_Gt IB4      = { 1, -iC_INPW, iC_ARITH, 0, "IB4", {0}, {0}, &IB2 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB4 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[0]}, &IX0_1 };
iC_Gt QB2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB2_0", {0}, {&iC_l_[3]}, &QB1_0 };
iC_Gt QB3_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB3_0", {0}, {&iC_l_[6]}, &QB2_0 };
iC_Gt QB4      = { 1, -iC_ARN, iC_ARITH, 0, "QB4", {0}, {&iC_l_[9]}, &QB3_0 };
iC_Gt QB4_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB4_0", {0}, {&iC_l_[12]}, &QB4 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[15]}, &QB4_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[18]}, &QX0_1_0 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[21]}, &QX0_2_0 };
static iC_Gt _f0_1   = { 1, -iC_OR, iC_F_CF, 1, "_f0_1", {&iC_l_[24]}, {&iC_l_[31]}, &QX0_3_0 };
iC_Gt a1       = { 1, -iC_ARNC, iC_ARITH, 0, "a1", {0}, {0}, &_f0_1 };
iC_Gt a2       = { 1, -iC_ARNC, iC_ARITH, 0, "a2", {0}, {0}, &a1 };
iC_Gt a3       = { 1, -iC_ARN, iC_ARITH, 0, "a3", {0}, {&iC_l_[31]}, &a2 };
iC_Gt b1       = { 1, -iC_LOGC, iC_GATE, 0, "b1", {0}, {0}, &a3 };
iC_Gt b2       = { 1, -iC_LOGC, iC_GATE, 0, "b2", {0}, {0}, &b1 };
iC_Gt b3       = { 1, -iC_AND, iC_GATE, 0, "b3", {0}, {&iC_l_[35]}, &b2 };
iC_Gt QB1      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB1", {0}, {(iC_Gt**)&a1}, &b3, 0 };
iC_Gt QB2      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB2", {0}, {(iC_Gt**)&a2}, &QB1, 0 };
iC_Gt QB3      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB3", {0}, {(iC_Gt**)&a3}, &QB2, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&b1}, &QB3, 0 };
iC_Gt QX0_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.2", {0}, {(iC_Gt**)&b2}, &QX0_1, 0 };
iC_Gt QX0_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.3", {0}, {(iC_Gt**)&b3}, &QX0_2, 0 };

iC_Gt *		iC___Test0_c12w_list = &QX0_3;
iC_Gt **	iC_list[] = { &iC___Test0_c12w_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 10 "./Test0/c12w.ic"

int
cf1(void)
{
    iC_AAL(0, 0,  iC_LVL(1) * iC_AVL(2));	// must be very careful variables are
    iC_AAL(3, 0,  iC_AVL(0) + iC_AVL(2));	// updated sequentially
    iC_LAL(4, 0,  iC_AVL(3) < 10);	// if a2 is evaluated before a1, a2 will be wrong
    iC_LAL(5, 0,  iC_AVL(3) < 20 || iC_LVL(1));
    return iC_AVL(0) + iC_AVL(3);
} /* cf1 */

#line 75 "./Test0/c12w.c"

static int iC_2(iC_Gt * iC_gf) {
#line 21 "./Test0/c12w.ic"
	return iC_MV(1)+iC_MV(2);
#line 80 "./Test0/c12w.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
#line 27 "./Test0/c12w.ic"
	return iC_MV(1)+cf1();
#line 86 "./Test0/c12w.c"
} /* iC_3 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1_0 */	(iC_Gt*)0, &a1, 0,
/* QB2_0 */	(iC_Gt*)0, &a2, 0,
/* QB3_0 */	(iC_Gt*)0, &a3, 0,
/* QB4 */	(iC_Gt*)iC_3, &IB4, 0,
/* QB4_0 */	(iC_Gt*)0, &QB4, 0,
/* QX0.1_0 */	&b1, 0, 0,
/* QX0.2_0 */	&b2, 0, 0,
/* QX0.3_0 */	&b3, 0, 0,
/* _f0_1 */	&a1, &b3, &a3, &a2, &b1, &b2, (iC_Gt*)0x5eb,
/* a3 */	(iC_Gt*)iC_2, &IB1, &IB2, 0,
/* b3 */	&IX0_1, &IX0_0, 0, 0,
};

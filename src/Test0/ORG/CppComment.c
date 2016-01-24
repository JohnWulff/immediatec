/********************************************************************
 *
 *	SOURCE:   ./Test0/CppComment.ic
 *	OUTPUT:   ./Test0/CppComment.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: CppComment.c,v 1.1 2015/05/24 00:51:45 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
#define iC_AV(n)	iC_gf->gt_list[n]->gt_new
#define iC_LV(n)	(iC_gf->gt_list[n]->gt_val < 0 ? 1 : 0)
#define iC_AA(n,p,v)	iC_assignA(iC_gf->gt_list[n], p, v)
#define iC_LA(n,p,v)	iC_assignL(iC_gf->gt_list[n], p, v)
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, 0 };
iC_Gt IB2      = { 1, -iC_INPW, iC_ARITH, 0, "IB2", {0}, {0}, &IB1 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB2 };
iC_Gt QB1      = { 1, -iC_ARNC, iC_ARITH, 0, "QB1", {0}, {0}, &IX0_0 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[0]}, &QB1 };
static iC_Gt _f1_1   = { 1, -iC_OR, iC_F_CF, 0, "_f1_1", {&iC_l_[3]}, {&iC_l_[9]}, &QB1_0 };

iC_Gt *		iC___Test0_CppComment_list = &_f1_1;
iC_Gt **	iC_list[] = { &iC___Test0_CppComment_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_2(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 9 "./Test0/CppComment.ic"
{
    iC_AA(2, 0,  iC_AV(3) + iC_AV(4));	// C++ comment in embedded C reported by -P option
}
#line 49 "./Test0/CppComment.c"
    return 0;
} /* iC_2 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1_0 */	(iC_Gt*)0, &QB1, 0,
/* _f1_1 */	(iC_Gt*)iC_2, &iClock, &QB1, &IB1, &IB2, (iC_Gt*)0x29,
		&IX0_0, 0, 0,
};

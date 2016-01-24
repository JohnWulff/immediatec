/********************************************************************
 *
 *	SOURCE:   ./Test0/aa4c1.ic
 *	OUTPUT:   ./Test0/aa4c1.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: aa4c1.c,v 1.1 2015/05/24 00:51:46 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB6      = { 1, -iC_INPW, iC_ARITH, 0, "IB6", {0}, {0}, 0 };
iC_Gt IB7      = { 1, -iC_INPW, iC_ARITH, 0, "IB7", {0}, {0}, &IB6 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB7 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
iC_Gt IX0_4    = { 1, -iC_INPX, iC_GATE, 0, "IX0.4", {0}, {0}, &IX0_3 };
iC_Gt IX0_5    = { 1, -iC_INPX, iC_GATE, 0, "IX0.5", {0}, {0}, &IX0_4 };
iC_Gt QB0      = { 1, -iC_ARN, iC_ARITH, 0, "QB0", {0}, {&iC_l_[0]}, &IX0_5 };
iC_Gt QB0_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB0_0", {0}, {&iC_l_[6]}, &QB0 };
iC_Gt QB0_1    = { 1, -iC_OR, iC_ARITH, 0, "QB0_1", {0}, {&iC_l_[9]}, &QB0_0 };
iC_Gt QB0_2    = { 1, -iC_AND, iC_ARITH, 0, "QB0_2", {0}, {&iC_l_[13]}, &QB0_1 };
iC_Gt QB0_3    = { 1, -iC_OR, iC_GATE, 0, "QB0_3", {0}, {&iC_l_[17]}, &QB0_2 };
iC_Gt QB0_4    = { 1, -iC_AND, iC_GATE, 0, "QB0_4", {0}, {&iC_l_[21]}, &QB0_3 };

iC_Gt *		iC___Test0_aa4c1_list = &QB0_4;
iC_Gt **	iC_list[] = { &iC___Test0_aa4c1_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_2(iC_Gt * iC_gf) {
#line 17 "./Test0/aa4c1.ic"
	return iC_MV(1)+iC_MV(2)*iC_MV(3)*iC_MV(4);
#line 50 "./Test0/aa4c1.c"
} /* iC_2 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB0 */	(iC_Gt*)iC_2, &IB6, &QB0_1, &QB0_2, &IB7, 0,
/* QB0_0 */	(iC_Gt*)0, &QB0, 0,
/* QB0_1 */	&QB0_4, &IX0_2, 0, 0,
/* QB0_2 */	&QB0_3, &IX0_3, 0, 0,
/* QB0_3 */	&IX0_5, &IX0_4, 0, 0,
/* QB0_4 */	&IX0_1, &IX0_0, 0, 0,
};

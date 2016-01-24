/********************************************************************
 *
 *	SOURCE:   ./Test0/aa4c.ic
 *	OUTPUT:   ./Test0/aa4c.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: aa4c.c,v 1.1 2015/05/24 00:51:46 jw Exp $ -O7";

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
iC_Gt IB4      = { 1, -iC_INPW, iC_ARITH, 0, "IB4", {0}, {0}, &IB3 };
iC_Gt IB5      = { 1, -iC_INPW, iC_ARITH, 0, "IB5", {0}, {0}, &IB4 };
iC_Gt IB6      = { 1, -iC_INPW, iC_ARITH, 0, "IB6", {0}, {0}, &IB5 };
iC_Gt IB7      = { 1, -iC_INPW, iC_ARITH, 0, "IB7", {0}, {0}, &IB6 };
iC_Gt QB0      = { 1, -iC_ARN, iC_ARITH, 0, "QB0", {0}, {&iC_l_[0]}, &IB7 };
iC_Gt QB0_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB0_0", {0}, {&iC_l_[10]}, &QB0 };

iC_Gt *		iC___Test0_aa4c_list = &QB0_0;
iC_Gt **	iC_list[] = { &iC___Test0_aa4c_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_4(iC_Gt * iC_gf) {
#line 17 "./Test0/aa4c.ic"
	return iC_MV(1)+(iC_MV(2)*iC_MV(3)+iC_MV(4))*(iC_MV(5)*(iC_MV(6)+iC_MV(7)))*iC_MV(8);
#line 46 "./Test0/aa4c.c"
} /* iC_4 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB0 */	(iC_Gt*)iC_4, &IB6, &IB0, &IB1, &IB2, &IB3, &IB4, &IB5,
		&IB7, 0,
/* QB0_0 */	(iC_Gt*)0, &QB0, 0,
};

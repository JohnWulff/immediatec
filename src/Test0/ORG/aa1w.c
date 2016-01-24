/********************************************************************
 *
 *	SOURCE:   ./Test0/aa1w.ic
 *	OUTPUT:   ./Test0/aa1w.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: aa1w.c,v 1.1 2015/05/24 00:51:45 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

static iC_Gt _10     = { 1, -iC_NCONST, iC_ARITH, 0, "10", {0}, {0}, 0 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &_10 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt QX0_1    = { 1, -iC_FF, iC_GATE, 0, "QX0.1", {0}, {0}, &IX0_2 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[0]}, &QX0_1 };
iC_Gt QX0_1_1  = { 1, -iC_OR, iC_D_FF, 0, "QX0.1_1", {&iC_l_[3]}, {&iC_l_[6]}, &QX0_1_0 };
iC_Gt QX0_2    = { 1, -iC_FF, iC_GATE, 0, "QX0.2", {0}, {0}, &QX0_1_1 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[9]}, &QX0_2 };
iC_Gt QX0_2_1  = { 1, -iC_OR, iC_D_FF, 0, "QX0.2_1", {&iC_l_[12]}, {&iC_l_[15]}, &QX0_2_0 };
iC_Gt QX0_2_2  = { 1, -iC_ARN, iC_ARITH, 0, "QX0.2_2", {0}, {&iC_l_[18]}, &QX0_2_1 };
extern iC_Gt iConst; /* 1 */
iC_Gt t        = { 1, -iC_TIM, iC_TIMRL, 0, "t", {0}, {0}, &QX0_2_2 };
iC_Gt t_1      = { 1, -iC_OR, iC_TIMR, 0, "t_1", {&iC_l_[21]}, {&iC_l_[23]}, &t };
iC_Gt t_2      = { 1, -iC_OR, iC_TIMR, 0, "t_2", {&iC_l_[26]}, {&iC_l_[28]}, &t_1 };

iC_Gt *		iC___Test0_aa1w_list = &t_2;
iC_Gt **	iC_list[] = { &iC___Test0_aa1w_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_2(iC_Gt * iC_gf) {
#line 3 "./Test0/aa1w.ic"
	return 010*0xa;
#line 51 "./Test0/aa1w.c"
} /* iC_2 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QX0.1_0 */	&QX0_1, 0, 0,
/* QX0.1_1 */	&QX0_1, &t, &_10,	&IX0_1, 0, 0,
/* QX0.2_0 */	&QX0_2, 0, 0,
/* QX0.2_1 */	&QX0_2, &t, &QX0_2_2,	&IX0_2, 0, 0,
/* QX0.2_2 */	(iC_Gt*)iC_2, &iConst, 0,
/* t_1 */	&t, &iClock,	&IX0_0, 0, 0,
/* t_2 */	&t, &iClock,	0, &IX0_0, 0,
};

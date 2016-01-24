/********************************************************************
 *
 *	SOURCE:   ./Test0/aa4e.ic
 *	OUTPUT:   ./Test0/aa4e.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: aa4e.c,v 1.1 2015/05/24 00:51:46 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB0      = { 1, -iC_INPW, iC_ARITH, 0, "IB0", {0}, {0}, 0 };
iC_Gt IB3      = { 1, -iC_INPW, iC_ARITH, 0, "IB3", {0}, {0}, &IB0 };
iC_Gt IB6      = { 1, -iC_INPW, iC_ARITH, 0, "IB6", {0}, {0}, &IB3 };
iC_Gt IB7      = { 1, -iC_INPW, iC_ARITH, 0, "IB7", {0}, {0}, &IB6 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB7 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt QB0      = { 1, -iC_ARN, iC_ARITH, 0, "QB0", {0}, {&iC_l_[0]}, &IX0_1 };
iC_Gt QB0_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB0_0", {0}, {&iC_l_[6]}, &QB0 };
iC_Gt QB0_1    = { 1, -iC_SH, iC_ARITH, 0, "QB0_1", {0}, {0}, &QB0_0 };
iC_Gt QB0_2    = { 1, -iC_SH, iC_ARITH, 0, "QB0_2", {0}, {0}, &QB0_1 };
iC_Gt QB0_3    = { 1, -iC_ARN, iC_D_SH, 0, "QB0_3", {&iC_l_[9]}, {&iC_l_[11]}, &QB0_2 };
iC_Gt QB0_4    = { 1, -iC_ARN, iC_D_SH, 0, "QB0_4", {&iC_l_[14]}, {&iC_l_[16]}, &QB0_3 };
iC_Gt QB1      = { 1, -iC_ARN, iC_ARITH, 0, "QB1", {0}, {&iC_l_[19]}, &QB0_4 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[25]}, &QB1 };
iC_Gt QB1_1    = { 1, -iC_SH, iC_ARITH, 0, "QB1_1", {0}, {0}, &QB1_0 };
iC_Gt QB1_2    = { 1, -iC_SH, iC_ARITH, 0, "QB1_2", {0}, {0}, &QB1_1 };
iC_Gt QB1_3    = { 1, -iC_ARN, iC_D_SH, 0, "QB1_3", {&iC_l_[28]}, {&iC_l_[30]}, &QB1_2 };
iC_Gt QB1_4    = { 1, -iC_OR, iC_R_SH, 0, "QB1_4", {&iC_l_[35]}, {&iC_l_[37]}, &QB1_3 };
iC_Gt QB1_5    = { 1, -iC_OR, iC_ARITH, 0, "QB1_5", {0}, {&iC_l_[40]}, &QB1_4 };
iC_Gt QB1_6    = { 1, -iC_ARN, iC_D_SH, 0, "QB1_6", {&iC_l_[43]}, {&iC_l_[45]}, &QB1_5 };
iC_Gt QB1_7    = { 1, -iC_OR, iC_R_SH, 0, "QB1_7", {&iC_l_[50]}, {&iC_l_[52]}, &QB1_6 };
iC_Gt QB1_8    = { 1, -iC_OR, iC_ARITH, 0, "QB1_8", {0}, {&iC_l_[55]}, &QB1_7 };
iC_Gt QB2      = { 1, -iC_ARN, iC_ARITH, 0, "QB2", {0}, {&iC_l_[58]}, &QB1_8 };
iC_Gt QB2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB2_0", {0}, {&iC_l_[64]}, &QB2 };
iC_Gt QB2_1    = { 1, -iC_SH, iC_ARITH, 0, "QB2_1", {0}, {0}, &QB2_0 };
iC_Gt QB2_10   = { 1, -iC_OR, iC_ARITH, 0, "QB2_10", {0}, {&iC_l_[67]}, &QB2_1 };
iC_Gt QB2_2    = { 1, -iC_SH, iC_ARITH, 0, "QB2_2", {0}, {0}, &QB2_10 };
iC_Gt QB2_3    = { 1, -iC_ARN, iC_D_SH, 0, "QB2_3", {&iC_l_[71]}, {&iC_l_[73]}, &QB2_2 };
iC_Gt QB2_4    = { 1, -iC_AND, iC_S_SH, 0, "QB2_4", {&iC_l_[78]}, {&iC_l_[80]}, &QB2_3 };
iC_Gt QB2_5    = { 1, -iC_AND, iC_R_SH, 0, "QB2_5", {&iC_l_[84]}, {&iC_l_[86]}, &QB2_4 };
iC_Gt QB2_6    = { 1, -iC_OR, iC_ARITH, 0, "QB2_6", {0}, {&iC_l_[90]}, &QB2_5 };
iC_Gt QB2_7    = { 1, -iC_ARN, iC_D_SH, 0, "QB2_7", {&iC_l_[94]}, {&iC_l_[96]}, &QB2_6 };
iC_Gt QB2_8    = { 1, -iC_AND, iC_S_SH, 0, "QB2_8", {&iC_l_[101]}, {&iC_l_[103]}, &QB2_7 };
iC_Gt QB2_9    = { 1, -iC_AND, iC_R_SH, 0, "QB2_9", {&iC_l_[107]}, {&iC_l_[109]}, &QB2_8 };

iC_Gt *		iC___Test0_aa4e_list = &QB2_9;
iC_Gt **	iC_list[] = { &iC___Test0_aa4e_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_1(iC_Gt * iC_gf) {
#line 139 "init_t.ic"	/* in pre-compiled function block SHR */
	return iC_MV(1)?iC_MV(2):iC_MV(3);
#line 70 "./Test0/aa4e.c"
} /* iC_1 */

static int iC_2(iC_Gt * iC_gf) {
#line 14 "./Test0/aa4e.ic"
	return iC_MV(1)+iC_MV(2)*iC_MV(3)*iC_MV(4);
#line 76 "./Test0/aa4e.c"
} /* iC_2 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB0 */	(iC_Gt*)iC_2, &IB6, &QB0_1, &QB0_2, &IB7, 0,
/* QB0_0 */	(iC_Gt*)0, &QB0, 0,
/* QB0_3 */	&QB0_2, &iClock,	(iC_Gt*)0, &IB3, 0,
/* QB0_4 */	&QB0_1, &iClock,	(iC_Gt*)0, &IB0, 0,
/* QB1 */	(iC_Gt*)iC_2, &IB6, &QB1_1, &QB1_2, &IB7, 0,
/* QB1_0 */	(iC_Gt*)0, &QB1, 0,
/* QB1_3 */	&QB1_2, &iClock,	(iC_Gt*)iC_1, &QB1_5, &QB1_2,
		&IB3, 0,
/* QB1_4 */	&QB1_2, &iClock,	&IX0_1, 0, 0,
/* QB1_5 */	&IX0_1, 0, 0,
/* QB1_6 */	&QB1_1, &iClock,	(iC_Gt*)iC_1, &QB1_8, &QB1_1,
		&IB0, 0,
/* QB1_7 */	&QB1_1, &iClock,	&IX0_1, 0, 0,
/* QB1_8 */	&IX0_1, 0, 0,
/* QB2 */	(iC_Gt*)iC_2, &IB6, &QB2_1, &QB2_2, &IB7, 0,
/* QB2_0 */	(iC_Gt*)0, &QB2, 0,
/* QB2_10 */	&IX0_1, &IX0_0, 0, 0,
/* QB2_3 */	&QB2_2, &iClock,	(iC_Gt*)iC_1, &QB2_6, &QB2_2,
		&IB3, 0,
/* QB2_4 */	&QB2_2, &iClock,	&IX0_0, 0, &IX0_1, 0,
/* QB2_5 */	&QB2_2, &iClock,	&IX0_1, 0, &IX0_0, 0,
/* QB2_6 */	&IX0_1, &IX0_0, 0, 0,
/* QB2_7 */	&QB2_1, &iClock,	(iC_Gt*)iC_1, &QB2_10, &QB2_1,
		&IB0, 0,
/* QB2_8 */	&QB2_1, &iClock,	&IX0_0, 0, &IX0_1, 0,
/* QB2_9 */	&QB2_1, &iClock,	&IX0_1, 0, &IX0_0, 0,
};

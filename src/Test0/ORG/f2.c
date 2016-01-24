/********************************************************************
 *
 *	SOURCE:   ./Test0/f2.ic
 *	OUTPUT:   ./Test0/f2.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: f2.c,v 1.1 2015/05/24 00:51:51 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, 0 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB1 };
iC_Gt QB1      = { 1, -iC_SH, iC_ARITH, 0, "QB1", {0}, {0}, &IX0_0 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[0]}, &QB1 };
iC_Gt QB1_1    = { 1, -iC_ARN, iC_D_SH, 0, "QB1_1", {&iC_l_[3]}, {&iC_l_[5]}, &QB1_0 };
iC_Gt QX0_0    = { 1, -iC_LATCH, iC_GATE, 0, "QX0.0", {0}, {&iC_l_[8]}, &QB1_1 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[13]}, &QX0_0 };
iC_Gt QX0_0_1  = { 1, -iC_ARN, iC_GATE, 0, "QX0.0_1", {0}, {&iC_l_[16]}, &QX0_0_0 };
iC_Gt QX0_1    = { 1, -iC_FF, iC_GATE, 0, "QX0.1", {0}, {0}, &QX0_0_1 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[20]}, &QX0_1 };
iC_Gt QX0_1_1  = { 1, -iC_OR, iC_S_FF, 0, "QX0.1_1", {&iC_l_[23]}, {&iC_l_[25]}, &QX0_1_0 };
iC_Gt QX0_1_2  = { 1, -iC_OR, iC_R_FF, 0, "QX0.1_2", {&iC_l_[28]}, {&iC_l_[30]}, &QX0_1_1 };
iC_Gt QX0_2    = { 1, -iC_FF, iC_GATE, 0, "QX0.2", {0}, {0}, &QX0_1_2 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[33]}, &QX0_2 };
iC_Gt QX0_2_1  = { 1, -iC_OR, iC_S_FF, 0, "QX0.2_1", {&iC_l_[36]}, {&iC_l_[38]}, &QX0_2_0 };
iC_Gt QX0_2_2  = { 1, -iC_OR, iC_R_FF, 0, "QX0.2_2", {&iC_l_[41]}, {&iC_l_[43]}, &QX0_2_1 };
iC_Gt c        = { 1, -iC_CLK, iC_CLCKL, 0, "c", {0}, {0}, &QX0_2_2 };
iC_Gt c_1      = { 1, -iC_AND, iC_CLCK, 0, "c_1", {&iC_l_[46]}, {&iC_l_[48]}, &c };
iC_Gt c_2      = { 1, -iC_OR, iC_GATE, 0, "c_2", {0}, {&iC_l_[52]}, &c_1 };
iC_Gt c_3      = { 1, -iC_VF, iC_GATE, 0, "c_3", {0}, {0}, &c_2 };
iC_Gt c_4      = { 1, -iC_ARN, iC_CH_AR, 0, "c_4", {&iC_l_[56]}, {&iC_l_[58]}, &c_3 };
iC_Gt hilo     = { 1, -iC_OR, iC_GATE, 0, "hilo", {0}, {&iC_l_[61]}, &c_4 };
iC_Gt hilo_1   = { 1, -iC_ARN, iC_GATE, 0, "hilo_1", {0}, {&iC_l_[65]}, &hilo };
iC_Gt hilo_2   = { 1, -iC_ARN, iC_GATE, 0, "hilo_2", {0}, {&iC_l_[69]}, &hilo_1 };
iC_Gt reset    = { 1, -iC_VF, iC_GATE, 0, "reset", {0}, {0}, &hilo_2 };
iC_Gt reset_1  = { 1, -iC_OR, iC_CH_BIT, 0, "reset_1", {&iC_l_[73]}, {&iC_l_[75]}, &reset };

iC_Gt *		iC___Test0_f2_list = &reset_1;
iC_Gt **	iC_list[] = { &iC___Test0_f2_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_2(iC_Gt * iC_gf) {
#line 4 "./Test0/f2.ic"
	return iC_MV(1)-iC_MV(2)>1;
#line 62 "./Test0/f2.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
#line 5 "./Test0/f2.ic"
	return iC_MV(1)==iC_MV(2);
#line 68 "./Test0/f2.c"
} /* iC_3 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1_0 */	(iC_Gt*)0, &QB1, 0,
/* QB1_1 */	&QB1, &c,	(iC_Gt*)0, &IB1, 0,
/* QX0.0 */	&hilo, &QX0_0, 0, &QX0_0_1, 0,
/* QX0.0_0 */	&QX0_0, 0, 0,
/* QX0.0_1 */	(iC_Gt*)iC_3, &IB1, &QB1, 0,
/* QX0.1_0 */	&QX0_1, 0, 0,
/* QX0.1_1 */	&QX0_1, &iClock,	&hilo, 0, 0,
/* QX0.1_2 */	&QX0_1, &iClock,	&reset, 0, 0,
/* QX0.2_0 */	&QX0_2, 0, 0,
/* QX0.2_1 */	&QX0_2, &c,	&hilo, 0, 0,
/* QX0.2_2 */	&QX0_2, &iClock,	&reset, 0, 0,
/* c_1 */	&c, &iClock,	&c_2, 0, &QX0_2, 0,
/* c_2 */	&reset, &c_3, 0, 0,
/* c_4 */	&c_3, &iClock,	(iC_Gt*)0, &IB1, 0,
/* hilo */	&hilo_2, &hilo_1, 0, 0,
/* hilo_1 */	(iC_Gt*)iC_2, &IB1, &QB1, 0,
/* hilo_2 */	(iC_Gt*)iC_2, &QB1, &IB1, 0,
/* reset_1 */	&reset, &c,	&IX0_0, 0, 0,
};

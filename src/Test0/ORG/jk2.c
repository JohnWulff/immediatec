/********************************************************************
 *
 *	SOURCE:   ./Test0/jk2.ic
 *	OUTPUT:   ./Test0/jk2.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: jk2.c,v 1.1 2015/05/24 00:51:54 jw Exp $ -O7";

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
iC_Gt IB3      = { 1, -iC_INPW, iC_ARITH, 0, "IB3", {0}, {0}, &IB2 };
iC_Gt IB4      = { 1, -iC_INPW, iC_ARITH, 0, "IB4", {0}, {0}, &IB3 };
iC_Gt IB5      = { 1, -iC_INPW, iC_ARITH, 0, "IB5", {0}, {0}, &IB4 };
iC_Gt IB6      = { 1, -iC_INPW, iC_ARITH, 0, "IB6", {0}, {0}, &IB5 };
iC_Gt IB7      = { 1, -iC_INPW, iC_ARITH, 0, "IB7", {0}, {0}, &IB6 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB7 };
iC_Gt IX0_6    = { 1, -iC_INPX, iC_GATE, 0, "IX0.6", {0}, {0}, &IX0_0 };
iC_Gt IX0_7    = { 1, -iC_INPX, iC_GATE, 0, "IX0.7", {0}, {0}, &IX0_6 };
iC_Gt QX0_0    = { 1, -iC_FF, iC_GATE, 0, "QX0.0", {0}, {0}, &IX0_7 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[0]}, &QX0_0 };
iC_Gt QX0_0_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.0_1", {&iC_l_[3]}, {&iC_l_[5]}, &QX0_0_0 };
iC_Gt QX0_0_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.0_2", {&iC_l_[9]}, {&iC_l_[11]}, &QX0_0_1 };
iC_Gt QX0_0_3  = { 1, -iC_OR, iC_GATE, 0, "QX0.0_3", {0}, {&iC_l_[15]}, &QX0_0_2 };
iC_Gt QX0_0_4  = { 1, -iC_ARN, iC_GATE, 0, "QX0.0_4", {0}, {&iC_l_[19]}, &QX0_0_3 };
iC_Gt QX0_0_5  = { 1, -iC_ARN, iC_GATE, 0, "QX0.0_5", {0}, {&iC_l_[23]}, &QX0_0_4 };
iC_Gt QX0_1    = { 1, -iC_FF, iC_GATE, 0, "QX0.1", {0}, {0}, &QX0_0_5 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[27]}, &QX0_1 };
iC_Gt QX0_1_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.1_1", {&iC_l_[30]}, {&iC_l_[32]}, &QX0_1_0 };
iC_Gt QX0_1_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.1_2", {&iC_l_[36]}, {&iC_l_[38]}, &QX0_1_1 };
iC_Gt QX0_1_3  = { 1, -iC_ARN, iC_GATE, 0, "QX0.1_3", {0}, {&iC_l_[42]}, &QX0_1_2 };
iC_Gt QX0_1_4  = { 1, -iC_ARN, iC_GATE, 0, "QX0.1_4", {0}, {&iC_l_[45]}, &QX0_1_3 };
iC_Gt QX0_2    = { 1, -iC_FF, iC_GATE, 0, "QX0.2", {0}, {0}, &QX0_1_4 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[48]}, &QX0_2 };
iC_Gt QX0_2_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.2_1", {&iC_l_[51]}, {&iC_l_[53]}, &QX0_2_0 };
iC_Gt QX0_2_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.2_2", {&iC_l_[57]}, {&iC_l_[59]}, &QX0_2_1 };
iC_Gt QX0_2_3  = { 1, -iC_ARN, iC_GATE, 0, "QX0.2_3", {0}, {&iC_l_[63]}, &QX0_2_2 };
iC_Gt QX0_2_4  = { 1, -iC_ARN, iC_GATE, 0, "QX0.2_4", {0}, {&iC_l_[66]}, &QX0_2_3 };
iC_Gt clk1     = { 1, -iC_CLK, iC_CLCKL, 0, "clk1", {0}, {0}, &QX0_2_4 };
iC_Gt clk1_1   = { 1, -iC_OR, iC_CLCK, 0, "clk1_1", {&iC_l_[69]}, {&iC_l_[71]}, &clk1 };
iC_Gt clk1_2   = { 1, -iC_OR, iC_CLCK, 0, "clk1_2", {&iC_l_[74]}, {&iC_l_[76]}, &clk1_1 };
iC_Gt clk2     = { 1, -iC_CLK, iC_CLCKL, 0, "clk2", {0}, {0}, &clk1_2 };
iC_Gt clk2_1   = { 1, -iC_OR, iC_CLCK, 0, "clk2_1", {&iC_l_[79]}, {&iC_l_[81]}, &clk2 };
iC_Gt clk2_2   = { 1, -iC_OR, iC_CLCK, 0, "clk2_2", {&iC_l_[84]}, {&iC_l_[86]}, &clk2_1 };

iC_Gt *		iC___Test0_jk2_list = &clk2_2;
iC_Gt **	iC_list[] = { &iC___Test0_jk2_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_2(iC_Gt * iC_gf) {
#line 3 "./Test0/jk2.ic"
	return (iC_MV(1)&iC_MV(2))==iC_MV(2);
#line 71 "./Test0/jk2.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
#line 3 "./Test0/jk2.ic"
	return (iC_MV(1)&0x7f)==0x41+(iC_MV(2)<<1);
#line 77 "./Test0/jk2.c"
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
#line 4 "./Test0/jk2.ic"
	return iC_MV(1)-1;
#line 83 "./Test0/jk2.c"
} /* iC_4 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QX0.0_0 */	&QX0_0, 0, 0,
/* QX0.0_1 */	&QX0_0, &iClock,	&QX0_0_4, 0, &QX0_0, 0,
/* QX0.0_2 */	&QX0_0, &iClock,	&QX0_0_3, &QX0_0, 0, 0,
/* QX0.0_3 */	&QX0_0_5, &IX0_0, 0, 0,
/* QX0.0_4 */	(iC_Gt*)iC_3, &IB1, &IB2, 0,
/* QX0.0_5 */	(iC_Gt*)iC_2, &IB1, &IB3, 0,
/* QX0.1_0 */	&QX0_1, 0, 0,
/* QX0.1_1 */	&QX0_1, &clk2,	&QX0_1_4, 0, &QX0_1, 0,
/* QX0.1_2 */	&QX0_1, &clk2,	&QX0_1_3, &QX0_1, 0, 0,
/* QX0.1_3 */	(iC_Gt*)iC_4, &IB5, 0,
/* QX0.1_4 */	(iC_Gt*)iC_4, &IB4, 0,
/* QX0.2_0 */	&QX0_2, 0, 0,
/* QX0.2_1 */	&QX0_2, &clk1,	&QX0_2_4, 0, &QX0_2, 0,
/* QX0.2_2 */	&QX0_2, &clk2,	&QX0_2_3, &QX0_2, 0, 0,
/* QX0.2_3 */	(iC_Gt*)iC_4, &IB7, 0,
/* QX0.2_4 */	(iC_Gt*)iC_4, &IB6, 0,
/* clk1_1 */	&clk1, &iClock,	&IX0_6, 0, 0,
/* clk1_2 */	&clk1, &iClock,	0, &IX0_6, 0,
/* clk2_1 */	&clk2, &iClock,	&IX0_7, 0, 0,
/* clk2_2 */	&clk2, &iClock,	0, &IX0_7, 0,
};

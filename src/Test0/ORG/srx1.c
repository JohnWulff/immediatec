/********************************************************************
 *
 *	SOURCE:   ./Test0/srx1.ic
 *	OUTPUT:   ./Test0/srx1.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: srx1.c,v 1.1 2015/05/24 00:51:59 jw Exp $ -O7";

#include	<icg.h>

static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, 0 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_6    = { 1, -iC_INPX, iC_GATE, 0, "IX0.6", {0}, {0}, &IX0_1 };
iC_Gt IX0_7    = { 1, -iC_INPX, iC_GATE, 0, "IX0.7", {0}, {0}, &IX0_6 };
iC_Gt QX0_0    = { 1, -iC_FF, iC_GATE, 0, "QX0.0", {0}, {0}, &IX0_7 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[0]}, &QX0_0 };
iC_Gt QX0_0_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.0_1", {&iC_l_[3]}, {&iC_l_[5]}, &QX0_0_0 };
iC_Gt QX0_0_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.0_2", {&iC_l_[9]}, {&iC_l_[11]}, &QX0_0_1 };
iC_Gt QX0_1    = { 1, -iC_FF, iC_GATE, 0, "QX0.1", {0}, {0}, &QX0_0_2 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[15]}, &QX0_1 };
iC_Gt QX0_1_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.1_1", {&iC_l_[18]}, {&iC_l_[20]}, &QX0_1_0 };
iC_Gt QX0_1_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.1_2", {&iC_l_[24]}, {&iC_l_[26]}, &QX0_1_1 };
iC_Gt QX0_2    = { 1, -iC_FF, iC_GATE, 0, "QX0.2", {0}, {0}, &QX0_1_2 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[30]}, &QX0_2 };
iC_Gt QX0_2_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.2_1", {&iC_l_[33]}, {&iC_l_[35]}, &QX0_2_0 };
iC_Gt QX0_2_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.2_2", {&iC_l_[39]}, {&iC_l_[41]}, &QX0_2_1 };
iC_Gt clk1     = { 1, -iC_CLK, iC_CLCKL, 0, "clk1", {0}, {0}, &QX0_2_2 };
iC_Gt clk1_1   = { 1, -iC_OR, iC_CLCK, 0, "clk1_1", {&iC_l_[45]}, {&iC_l_[47]}, &clk1 };
iC_Gt clk1_2   = { 1, -iC_OR, iC_CLCK, 0, "clk1_2", {&iC_l_[50]}, {&iC_l_[52]}, &clk1_1 };
iC_Gt clk2     = { 1, -iC_CLK, iC_CLCKL, 0, "clk2", {0}, {0}, &clk1_2 };
iC_Gt clk2_1   = { 1, -iC_OR, iC_CLCK, 0, "clk2_1", {&iC_l_[55]}, {&iC_l_[57]}, &clk2 };
iC_Gt clk2_2   = { 1, -iC_OR, iC_CLCK, 0, "clk2_2", {&iC_l_[60]}, {&iC_l_[62]}, &clk2_1 };

iC_Gt *		iC___Test0_srx1_list = &clk2_2;
iC_Gt **	iC_list[] = { &iC___Test0_srx1_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QX0.0_0 */	&QX0_0, 0, 0,
/* QX0.0_1 */	&QX0_0, &iClock,	&IX0_0, 0, &IX0_1, 0,
/* QX0.0_2 */	&QX0_0, &iClock,	&IX0_1, 0, &IX0_0, 0,
/* QX0.1_0 */	&QX0_1, 0, 0,
/* QX0.1_1 */	&QX0_1, &clk2,	&IX0_0, 0, &IX0_1, 0,
/* QX0.1_2 */	&QX0_1, &clk2,	&IX0_1, 0, &IX0_0, 0,
/* QX0.2_0 */	&QX0_2, 0, 0,
/* QX0.2_1 */	&QX0_2, &clk1,	&IX0_0, 0, &IX0_1, 0,
/* QX0.2_2 */	&QX0_2, &clk2,	&IX0_1, 0, &IX0_0, 0,
/* clk1_1 */	&clk1, &iClock,	&IX0_6, 0, 0,
/* clk1_2 */	&clk1, &iClock,	0, &IX0_6, 0,
/* clk2_1 */	&clk2, &iClock,	&IX0_7, 0, 0,
/* clk2_2 */	&clk2, &iClock,	0, &IX0_7, 0,
};

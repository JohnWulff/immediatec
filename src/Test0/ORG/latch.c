/********************************************************************
 *
 *	SOURCE:   ./Test0/latch.ic
 *	OUTPUT:   ./Test0/latch.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: latch.c,v 1.1 2015/05/24 00:51:54 jw Exp $ -O7";

#include	<icg.h>

static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, 0 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt QX0_0    = { 1, -iC_LATCH, iC_GATE, 0, "QX0.0", {0}, {&iC_l_[0]}, &IX0_2 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[5]}, &QX0_0 };
iC_Gt QX0_1    = { 1, -iC_LATCH, iC_GATE, 0, "QX0.1", {0}, {&iC_l_[8]}, &QX0_0_0 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[13]}, &QX0_1 };
iC_Gt QX0_2    = { 1, -iC_FF, iC_GATE, 0, "QX0.2", {0}, {0}, &QX0_1_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[16]}, &QX0_2 };
iC_Gt QX0_2_1  = { 1, -iC_LATCH, iC_D_FF, 0, "QX0.2_1", {&iC_l_[19]}, {&iC_l_[21]}, &QX0_2_0 };
iC_Gt QX0_3    = { 1, -iC_FF, iC_GATE, 0, "QX0.3", {0}, {0}, &QX0_2_1 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[26]}, &QX0_3 };
iC_Gt QX0_3_1  = { 1, -iC_OR, iC_D_FF, 0, "QX0.3_1", {&iC_l_[29]}, {&iC_l_[31]}, &QX0_3_0 };
iC_Gt QX0_3_2  = { 1, -iC_LATCH, iC_GATE, 0, "QX0.3_2", {0}, {&iC_l_[34]}, &QX0_3_1 };
iC_Gt QX0_4    = { 1, -iC_FF, iC_GATE, 0, "QX0.4", {0}, {0}, &QX0_3_2 };
iC_Gt QX0_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.4_0", {0}, {&iC_l_[39]}, &QX0_4 };
iC_Gt QX0_4_1  = { 1, -iC_LATCH, iC_D_FF, 0, "QX0.4_1", {&iC_l_[42]}, {&iC_l_[44]}, &QX0_4_0 };
iC_Gt QX0_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.5_0", {0}, {&iC_l_[49]}, &QX0_4_1 };
iC_Gt QX0_6    = { 1, -iC_FF, iC_GATE, 0, "QX0.6", {0}, {0}, &QX0_5_0 };
iC_Gt QX0_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.6_0", {0}, {&iC_l_[52]}, &QX0_6 };
iC_Gt QX0_6_1  = { 1, -iC_OR, iC_D_FF, 0, "QX0.6_1", {&iC_l_[55]}, {&iC_l_[57]}, &QX0_6_0 };
iC_Gt QX0_7    = { 1, -iC_FF, iC_GATE, 0, "QX0.7", {0}, {0}, &QX0_6_1 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[60]}, &QX0_7 };
iC_Gt QX0_7_1  = { 1, -iC_OR, iC_D_FF, 0, "QX0.7_1", {&iC_l_[63]}, {&iC_l_[65]}, &QX0_7_0 };
iC_Gt QX1_0    = { 1, -iC_FF, iC_GATE, 0, "QX1.0", {0}, {0}, &QX0_7_1 };
iC_Gt QX1_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.0_0", {0}, {&iC_l_[68]}, &QX1_0 };
iC_Gt QX1_0_1  = { 1, -iC_OR, iC_S_FF, 0, "QX1.0_1", {&iC_l_[71]}, {&iC_l_[73]}, &QX1_0_0 };
iC_Gt QX1_0_2  = { 1, -iC_OR, iC_R_FF, 0, "QX1.0_2", {&iC_l_[76]}, {&iC_l_[78]}, &QX1_0_1 };
iC_Gt c        = { 1, -iC_CLK, iC_CLCKL, 0, "c", {0}, {0}, &QX1_0_2 };
iC_Gt c_1      = { 1, -iC_OR, iC_CLCK, 0, "c_1", {&iC_l_[81]}, {&iC_l_[83]}, &c };
iC_Gt c_2      = { 1, -iC_VF, iC_GATE, 0, "c_2", {0}, {0}, &c_1 };
iC_Gt c_3      = { 1, -iC_OR, iC_CH_BIT, 0, "c_3", {&iC_l_[86]}, {&iC_l_[88]}, &c_2 };
iC_Gt t5       = { 1, -iC_FF, iC_GATE, 0, "t5", {0}, {0}, &c_3 };
iC_Gt t5_1     = { 1, -iC_LATCH, iC_D_FF, 0, "t5_1", {&iC_l_[91]}, {&iC_l_[93]}, &t5 };
iC_Gt t6       = { 1, -iC_LATCH, iC_GATE, 0, "t6", {0}, {&iC_l_[98]}, &t5_1 };
iC_Gt t7       = { 1, -iC_LATCH, iC_GATE, 0, "t7", {0}, {&iC_l_[103]}, &t6 };
iC_Gt QX0_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.5", {0}, {(iC_Gt**)&t5}, &t7, 0 };

iC_Gt *		iC___Test0_latch_list = &QX0_5;
iC_Gt **	iC_list[] = { &iC___Test0_latch_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QX0.0 */	&QX0_0, &IX0_0, 0, &IX0_1, 0,
/* QX0.0_0 */	&QX0_0, 0, 0,
/* QX0.1 */	&QX0_1, &IX0_0, 0, &IX0_1, 0,
/* QX0.1_0 */	&QX0_1, 0, 0,
/* QX0.2_0 */	&QX0_2, 0, 0,
/* QX0.2_1 */	&QX0_2, &c,	&QX0_2, &IX0_0, 0, &IX0_1, 0,
/* QX0.3_0 */	&QX0_3, 0, 0,
/* QX0.3_1 */	&QX0_3, &c,	&QX0_3_2, 0, 0,
/* QX0.3_2 */	&QX0_3_2, &IX0_0, 0, &IX0_1, 0,
/* QX0.4_0 */	&QX0_4, 0, 0,
/* QX0.4_1 */	&QX0_4, &c,	&QX0_4, &IX0_0, 0, &IX0_1, 0,
/* QX0.5_0 */	&t5, 0, 0,
/* QX0.6_0 */	&QX0_6, 0, 0,
/* QX0.6_1 */	&QX0_6, &c,	&t6, 0, 0,
/* QX0.7_0 */	&QX0_7, 0, 0,
/* QX0.7_1 */	&QX0_7, &c,	&t7, 0, 0,
/* QX1.0_0 */	&QX1_0, 0, 0,
/* QX1.0_1 */	&QX1_0, &c,	&IX0_0, 0, 0,
/* QX1.0_2 */	&QX1_0, &c,	&IX0_1, 0, 0,
/* c_1 */	&c, &iClock,	&c_2, 0, 0,
/* c_3 */	&c_2, &iClock,	&IX0_2, 0, 0,
/* t5_1 */	&t5, &c,	&t5, &IX0_0, 0, &IX0_1, 0,
/* t6 */	&t6, &IX0_0, 0, &IX0_1, 0,
/* t7 */	&t7, &IX0_0, 0, &IX0_1, 0,
};

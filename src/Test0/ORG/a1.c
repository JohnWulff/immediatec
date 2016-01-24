/********************************************************************
 *
 *	SOURCE:   ./Test0/a1.ic
 *	OUTPUT:   ./Test0/a1.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: a1.c,v 1.1 2015/05/24 00:51:45 jw Exp $ -O7";

#include	<icg.h>

static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, 0 };
iC_Gt IW2      = { 1, -iC_INPW, iC_ARITH, 0, "IW2", {0}, {0}, &IB1 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IW2 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
iC_Gt IX0_4    = { 1, -iC_INPX, iC_GATE, 0, "IX0.4", {0}, {0}, &IX0_3 };
iC_Gt IX0_5    = { 1, -iC_INPX, iC_GATE, 0, "IX0.5", {0}, {0}, &IX0_4 };
iC_Gt IX0_6    = { 1, -iC_INPX, iC_GATE, 0, "IX0.6", {0}, {0}, &IX0_5 };
iC_Gt IX0_7    = { 1, -iC_INPX, iC_GATE, 0, "IX0.7", {0}, {0}, &IX0_6 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[0]}, &IX0_7 };
iC_Gt QW2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QW2_0", {0}, {&iC_l_[3]}, &QB1_0 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[6]}, &QW2_0 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[9]}, &QX0_0_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[12]}, &QX0_1_0 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[15]}, &QX0_2_0 };
iC_Gt QX0_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.4_0", {0}, {&iC_l_[18]}, &QX0_3_0 };
iC_Gt QX0_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.5_0", {0}, {&iC_l_[21]}, &QX0_4_0 };
iC_Gt QX0_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.6_0", {0}, {&iC_l_[24]}, &QX0_5_0 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[27]}, &QX0_6_0 };
iC_Gt QB1      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB1", {0}, {(iC_Gt**)&IB1}, &QX0_7_0, 0 };
iC_Gt QW2      = { 1, -iC_ALIAS, iC_ARITH, 0, "QW2", {0}, {(iC_Gt**)&IW2}, &QB1, 0 };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&IX0_0}, &QW2, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&IX0_1}, &QX0_0, 0 };
iC_Gt QX0_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.2", {0}, {(iC_Gt**)&IX0_2}, &QX0_1, 0 };
iC_Gt QX0_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.3", {0}, {(iC_Gt**)&IX0_3}, &QX0_2, 0 };
iC_Gt QX0_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.4", {0}, {(iC_Gt**)&IX0_4}, &QX0_3, 0 };
iC_Gt QX0_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.5", {0}, {(iC_Gt**)&IX0_5}, &QX0_4, 0 };
iC_Gt QX0_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.6", {0}, {(iC_Gt**)&IX0_6}, &QX0_5, 0 };
iC_Gt QX0_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.7", {0}, {(iC_Gt**)&IX0_7}, &QX0_6, 0 };

iC_Gt *		iC___Test0_a1_list = &QX0_7;
iC_Gt **	iC_list[] = { &iC___Test0_a1_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1_0 */	(iC_Gt*)0, &IB1, 0,
/* QW2_0 */	(iC_Gt*)0, &IW2, 0,
/* QX0.0_0 */	&IX0_0, 0, 0,
/* QX0.1_0 */	&IX0_1, 0, 0,
/* QX0.2_0 */	&IX0_2, 0, 0,
/* QX0.3_0 */	&IX0_3, 0, 0,
/* QX0.4_0 */	&IX0_4, 0, 0,
/* QX0.5_0 */	&IX0_5, 0, 0,
/* QX0.6_0 */	&IX0_6, 0, 0,
/* QX0.7_0 */	&IX0_7, 0, 0,
};

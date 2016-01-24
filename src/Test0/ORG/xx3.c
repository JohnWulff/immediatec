/********************************************************************
 *
 *	SOURCE:   ./Test0/xx3.ic
 *	OUTPUT:   ./Test0/xx3.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: xx3.c,v 1.1 2015/05/24 00:51:59 jw Exp $ -O7";

#include	<icg.h>

static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB0      = { 1, -iC_INPW, iC_ARITH, 0, "IB0", {0}, {0}, 0 };
iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, &IB0 };
iC_Gt IB10     = { 1, -iC_INPW, iC_ARITH, 0, "IB10", {0}, {0}, &IB1 };
iC_Gt IB11     = { 1, -iC_INPW, iC_ARITH, 0, "IB11", {0}, {0}, &IB10 };
iC_Gt IB12     = { 1, -iC_INPW, iC_ARITH, 0, "IB12", {0}, {0}, &IB11 };
iC_Gt IB13     = { 1, -iC_INPW, iC_ARITH, 0, "IB13", {0}, {0}, &IB12 };
iC_Gt IB14     = { 1, -iC_INPW, iC_ARITH, 0, "IB14", {0}, {0}, &IB13 };
iC_Gt IB15     = { 1, -iC_INPW, iC_ARITH, 0, "IB15", {0}, {0}, &IB14 };
iC_Gt IB16     = { 1, -iC_INPW, iC_ARITH, 0, "IB16", {0}, {0}, &IB15 };
iC_Gt IB17     = { 1, -iC_INPW, iC_ARITH, 0, "IB17", {0}, {0}, &IB16 };
iC_Gt IB18     = { 1, -iC_INPW, iC_ARITH, 0, "IB18", {0}, {0}, &IB17 };
iC_Gt IB19     = { 1, -iC_INPW, iC_ARITH, 0, "IB19", {0}, {0}, &IB18 };
iC_Gt IB2      = { 1, -iC_INPW, iC_ARITH, 0, "IB2", {0}, {0}, &IB19 };
iC_Gt IB20     = { 1, -iC_INPW, iC_ARITH, 0, "IB20", {0}, {0}, &IB2 };
iC_Gt IB21     = { 1, -iC_INPW, iC_ARITH, 0, "IB21", {0}, {0}, &IB20 };
iC_Gt IB22     = { 1, -iC_INPW, iC_ARITH, 0, "IB22", {0}, {0}, &IB21 };
iC_Gt IB23     = { 1, -iC_INPW, iC_ARITH, 0, "IB23", {0}, {0}, &IB22 };
iC_Gt IB3      = { 1, -iC_INPW, iC_ARITH, 0, "IB3", {0}, {0}, &IB23 };
iC_Gt IB4      = { 1, -iC_INPW, iC_ARITH, 0, "IB4", {0}, {0}, &IB3 };
iC_Gt IB5      = { 1, -iC_INPW, iC_ARITH, 0, "IB5", {0}, {0}, &IB4 };
iC_Gt IB6      = { 1, -iC_INPW, iC_ARITH, 0, "IB6", {0}, {0}, &IB5 };
iC_Gt IB7      = { 1, -iC_INPW, iC_ARITH, 0, "IB7", {0}, {0}, &IB6 };
iC_Gt IB8      = { 1, -iC_INPW, iC_ARITH, 0, "IB8", {0}, {0}, &IB7 };
iC_Gt IB9      = { 1, -iC_INPW, iC_ARITH, 0, "IB9", {0}, {0}, &IB8 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB9 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
iC_Gt IX0_4    = { 1, -iC_INPX, iC_GATE, 0, "IX0.4", {0}, {0}, &IX0_3 };
iC_Gt IX0_5    = { 1, -iC_INPX, iC_GATE, 0, "IX0.5", {0}, {0}, &IX0_4 };
iC_Gt IX1_0    = { 1, -iC_INPX, iC_GATE, 0, "IX1.0", {0}, {0}, &IX0_5 };
iC_Gt IX1_1    = { 1, -iC_INPX, iC_GATE, 0, "IX1.1", {0}, {0}, &IX1_0 };
iC_Gt IX1_2    = { 1, -iC_INPX, iC_GATE, 0, "IX1.2", {0}, {0}, &IX1_1 };
iC_Gt IX1_3    = { 1, -iC_INPX, iC_GATE, 0, "IX1.3", {0}, {0}, &IX1_2 };
iC_Gt IX1_4    = { 1, -iC_INPX, iC_GATE, 0, "IX1.4", {0}, {0}, &IX1_3 };
iC_Gt IX1_5    = { 1, -iC_INPX, iC_GATE, 0, "IX1.5", {0}, {0}, &IX1_4 };
iC_Gt a0       = { 1, -iC_ARN, iC_GATE, 0, "a0", {0}, {&iC_l_[0]}, &IX1_5 };
iC_Gt a0xabc   = { 1, -iC_AND, iC_GATE, 0, "a0xabc", {0}, {&iC_l_[3]}, &a0 };
iC_Gt a0xdef   = { 1, -iC_AND, iC_GATE, 0, "a0xdef", {0}, {&iC_l_[7]}, &a0xabc };
iC_Gt a0xghi   = { 1, -iC_AND, iC_GATE, 0, "a0xghi", {0}, {&iC_l_[11]}, &a0xdef };
iC_Gt a0xjkl   = { 1, -iC_AND, iC_GATE, 0, "a0xjkl", {0}, {&iC_l_[15]}, &a0xghi };
iC_Gt a1       = { 1, -iC_ARN, iC_GATE, 0, "a1", {0}, {&iC_l_[19]}, &a0xjkl };
iC_Gt a10      = { 1, -iC_ARN, iC_GATE, 0, "a10", {0}, {&iC_l_[22]}, &a1 };
iC_Gt a11      = { 1, -iC_ARN, iC_GATE, 0, "a11", {0}, {&iC_l_[25]}, &a10 };
iC_Gt a12      = { 1, -iC_ARN, iC_GATE, 0, "a12", {0}, {&iC_l_[28]}, &a11 };
iC_Gt a13      = { 1, -iC_ARN, iC_GATE, 0, "a13", {0}, {&iC_l_[31]}, &a12 };
iC_Gt a14      = { 1, -iC_ARN, iC_GATE, 0, "a14", {0}, {&iC_l_[34]}, &a13 };
iC_Gt a15      = { 1, -iC_ARN, iC_GATE, 0, "a15", {0}, {&iC_l_[37]}, &a14 };
iC_Gt a16      = { 1, -iC_ARN, iC_GATE, 0, "a16", {0}, {&iC_l_[40]}, &a15 };
iC_Gt a17      = { 1, -iC_ARN, iC_GATE, 0, "a17", {0}, {&iC_l_[43]}, &a16 };
iC_Gt a18      = { 1, -iC_ARN, iC_GATE, 0, "a18", {0}, {&iC_l_[46]}, &a17 };
iC_Gt a19      = { 1, -iC_ARN, iC_GATE, 0, "a19", {0}, {&iC_l_[49]}, &a18 };
iC_Gt a1xabc   = { 1, -iC_AND, iC_GATE, 0, "a1xabc", {0}, {&iC_l_[52]}, &a19 };
iC_Gt a1xdef   = { 1, -iC_AND, iC_GATE, 0, "a1xdef", {0}, {&iC_l_[56]}, &a1xabc };
iC_Gt a1xghi   = { 1, -iC_AND, iC_GATE, 0, "a1xghi", {0}, {&iC_l_[60]}, &a1xdef };
iC_Gt a1xjkl   = { 1, -iC_AND, iC_GATE, 0, "a1xjkl", {0}, {&iC_l_[64]}, &a1xghi };
iC_Gt a2       = { 1, -iC_ARN, iC_GATE, 0, "a2", {0}, {&iC_l_[68]}, &a1xjkl };
iC_Gt a20      = { 1, -iC_ARN, iC_GATE, 0, "a20", {0}, {&iC_l_[71]}, &a2 };
iC_Gt a21      = { 1, -iC_ARN, iC_GATE, 0, "a21", {0}, {&iC_l_[74]}, &a20 };
iC_Gt a22      = { 1, -iC_ARN, iC_GATE, 0, "a22", {0}, {&iC_l_[77]}, &a21 };
iC_Gt a23      = { 1, -iC_ARN, iC_GATE, 0, "a23", {0}, {&iC_l_[80]}, &a22 };
iC_Gt a2xabc   = { 1, -iC_AND, iC_GATE, 0, "a2xabc", {0}, {&iC_l_[83]}, &a23 };
iC_Gt a2xdef   = { 1, -iC_AND, iC_GATE, 0, "a2xdef", {0}, {&iC_l_[87]}, &a2xabc };
iC_Gt a2xghi   = { 1, -iC_AND, iC_GATE, 0, "a2xghi", {0}, {&iC_l_[91]}, &a2xdef };
iC_Gt a2xjkl   = { 1, -iC_AND, iC_GATE, 0, "a2xjkl", {0}, {&iC_l_[95]}, &a2xghi };
iC_Gt a3       = { 1, -iC_ARN, iC_GATE, 0, "a3", {0}, {&iC_l_[99]}, &a2xjkl };
iC_Gt a3xabc   = { 1, -iC_AND, iC_GATE, 0, "a3xabc", {0}, {&iC_l_[102]}, &a3 };
iC_Gt a3xdef   = { 1, -iC_AND, iC_GATE, 0, "a3xdef", {0}, {&iC_l_[106]}, &a3xabc };
iC_Gt a3xghi   = { 1, -iC_AND, iC_GATE, 0, "a3xghi", {0}, {&iC_l_[110]}, &a3xdef };
iC_Gt a3xjkl   = { 1, -iC_AND, iC_GATE, 0, "a3xjkl", {0}, {&iC_l_[114]}, &a3xghi };
iC_Gt a4       = { 1, -iC_ARN, iC_GATE, 0, "a4", {0}, {&iC_l_[118]}, &a3xjkl };
iC_Gt a4xabc   = { 1, -iC_AND, iC_GATE, 0, "a4xabc", {0}, {&iC_l_[121]}, &a4 };
iC_Gt a4xdef   = { 1, -iC_AND, iC_GATE, 0, "a4xdef", {0}, {&iC_l_[125]}, &a4xabc };
iC_Gt a4xghi   = { 1, -iC_AND, iC_GATE, 0, "a4xghi", {0}, {&iC_l_[129]}, &a4xdef };
iC_Gt a4xjkl   = { 1, -iC_AND, iC_GATE, 0, "a4xjkl", {0}, {&iC_l_[133]}, &a4xghi };
iC_Gt a5       = { 1, -iC_ARN, iC_GATE, 0, "a5", {0}, {&iC_l_[137]}, &a4xjkl };
iC_Gt a5xabc   = { 1, -iC_AND, iC_GATE, 0, "a5xabc", {0}, {&iC_l_[140]}, &a5 };
iC_Gt a5xdef   = { 1, -iC_AND, iC_GATE, 0, "a5xdef", {0}, {&iC_l_[144]}, &a5xabc };
iC_Gt a5xghi   = { 1, -iC_AND, iC_GATE, 0, "a5xghi", {0}, {&iC_l_[148]}, &a5xdef };
iC_Gt a5xjkl   = { 1, -iC_AND, iC_GATE, 0, "a5xjkl", {0}, {&iC_l_[152]}, &a5xghi };
iC_Gt a6       = { 1, -iC_ARN, iC_GATE, 0, "a6", {0}, {&iC_l_[156]}, &a5xjkl };
iC_Gt a7       = { 1, -iC_ARN, iC_GATE, 0, "a7", {0}, {&iC_l_[159]}, &a6 };
iC_Gt a8       = { 1, -iC_ARN, iC_GATE, 0, "a8", {0}, {&iC_l_[162]}, &a7 };
iC_Gt a9       = { 1, -iC_ARN, iC_GATE, 0, "a9", {0}, {&iC_l_[165]}, &a8 };

iC_Gt *		iC___Test0_xx3_list = &a9;
iC_Gt **	iC_list[] = { &iC___Test0_xx3_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* a0 */	(iC_Gt*)0, &IB0, 0,
/* a0xabc */	&IX1_0, &IX0_0, 0, 0,
/* a0xdef */	&IX1_0, &IX0_0, 0, 0,
/* a0xghi */	&IX1_0, &IX0_0, 0, 0,
/* a0xjkl */	&IX1_0, &IX0_0, 0, 0,
/* a1 */	(iC_Gt*)0, &IB1, 0,
/* a10 */	(iC_Gt*)0, &IB10, 0,
/* a11 */	(iC_Gt*)0, &IB11, 0,
/* a12 */	(iC_Gt*)0, &IB12, 0,
/* a13 */	(iC_Gt*)0, &IB13, 0,
/* a14 */	(iC_Gt*)0, &IB14, 0,
/* a15 */	(iC_Gt*)0, &IB15, 0,
/* a16 */	(iC_Gt*)0, &IB16, 0,
/* a17 */	(iC_Gt*)0, &IB17, 0,
/* a18 */	(iC_Gt*)0, &IB18, 0,
/* a19 */	(iC_Gt*)0, &IB19, 0,
/* a1xabc */	&IX1_1, &IX0_1, 0, 0,
/* a1xdef */	&IX1_1, &IX0_1, 0, 0,
/* a1xghi */	&IX1_1, &IX0_1, 0, 0,
/* a1xjkl */	&IX1_1, &IX0_1, 0, 0,
/* a2 */	(iC_Gt*)0, &IB2, 0,
/* a20 */	(iC_Gt*)0, &IB20, 0,
/* a21 */	(iC_Gt*)0, &IB21, 0,
/* a22 */	(iC_Gt*)0, &IB22, 0,
/* a23 */	(iC_Gt*)0, &IB23, 0,
/* a2xabc */	&IX1_2, &IX0_2, 0, 0,
/* a2xdef */	&IX1_2, &IX0_2, 0, 0,
/* a2xghi */	&IX1_2, &IX0_2, 0, 0,
/* a2xjkl */	&IX1_2, &IX0_2, 0, 0,
/* a3 */	(iC_Gt*)0, &IB3, 0,
/* a3xabc */	&IX1_3, &IX0_3, 0, 0,
/* a3xdef */	&IX1_3, &IX0_3, 0, 0,
/* a3xghi */	&IX1_3, &IX0_3, 0, 0,
/* a3xjkl */	&IX1_3, &IX0_3, 0, 0,
/* a4 */	(iC_Gt*)0, &IB4, 0,
/* a4xabc */	&IX1_4, &IX0_4, 0, 0,
/* a4xdef */	&IX1_4, &IX0_4, 0, 0,
/* a4xghi */	&IX1_4, &IX0_4, 0, 0,
/* a4xjkl */	&IX1_4, &IX0_4, 0, 0,
/* a5 */	(iC_Gt*)0, &IB5, 0,
/* a5xabc */	&IX1_5, &IX0_5, 0, 0,
/* a5xdef */	&IX1_5, &IX0_5, 0, 0,
/* a5xghi */	&IX1_5, &IX0_5, 0, 0,
/* a5xjkl */	&IX1_5, &IX0_5, 0, 0,
/* a6 */	(iC_Gt*)0, &IB6, 0,
/* a7 */	(iC_Gt*)0, &IB7, 0,
/* a8 */	(iC_Gt*)0, &IB8, 0,
/* a9 */	(iC_Gt*)0, &IB9, 0,
};

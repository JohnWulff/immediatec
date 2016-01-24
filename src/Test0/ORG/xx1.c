/********************************************************************
 *
 *	SOURCE:   ./Test0/xx1.ic
 *	OUTPUT:   ./Test0/xx1.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: xx1.c,v 1.1 2015/05/24 00:51:59 jw Exp $ -O7";

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
iC_Gt IX1_0    = { 1, -iC_INPX, iC_GATE, 0, "IX1.0", {0}, {0}, &IX0_3 };
iC_Gt IX1_1    = { 1, -iC_INPX, iC_GATE, 0, "IX1.1", {0}, {0}, &IX1_0 };
iC_Gt IX1_2    = { 1, -iC_INPX, iC_GATE, 0, "IX1.2", {0}, {0}, &IX1_1 };
iC_Gt IX1_3    = { 1, -iC_INPX, iC_GATE, 0, "IX1.3", {0}, {0}, &IX1_2 };
iC_Gt IX2_0    = { 1, -iC_INPX, iC_GATE, 0, "IX2.0", {0}, {0}, &IX1_3 };
iC_Gt IX2_1    = { 1, -iC_INPX, iC_GATE, 0, "IX2.1", {0}, {0}, &IX2_0 };
iC_Gt IX2_2    = { 1, -iC_INPX, iC_GATE, 0, "IX2.2", {0}, {0}, &IX2_1 };
iC_Gt IX2_3    = { 1, -iC_INPX, iC_GATE, 0, "IX2.3", {0}, {0}, &IX2_2 };
iC_Gt IX3_0    = { 1, -iC_INPX, iC_GATE, 0, "IX3.0", {0}, {0}, &IX2_3 };
iC_Gt IX3_1    = { 1, -iC_INPX, iC_GATE, 0, "IX3.1", {0}, {0}, &IX3_0 };
iC_Gt IX3_2    = { 1, -iC_INPX, iC_GATE, 0, "IX3.2", {0}, {0}, &IX3_1 };
iC_Gt IX3_3    = { 1, -iC_INPX, iC_GATE, 0, "IX3.3", {0}, {0}, &IX3_2 };
iC_Gt IX4_0    = { 1, -iC_INPX, iC_GATE, 0, "IX4.0", {0}, {0}, &IX3_3 };
iC_Gt IX4_1    = { 1, -iC_INPX, iC_GATE, 0, "IX4.1", {0}, {0}, &IX4_0 };
iC_Gt IX4_2    = { 1, -iC_INPX, iC_GATE, 0, "IX4.2", {0}, {0}, &IX4_1 };
iC_Gt IX4_3    = { 1, -iC_INPX, iC_GATE, 0, "IX4.3", {0}, {0}, &IX4_2 };
iC_Gt IX5_0    = { 1, -iC_INPX, iC_GATE, 0, "IX5.0", {0}, {0}, &IX4_3 };
iC_Gt IX5_1    = { 1, -iC_INPX, iC_GATE, 0, "IX5.1", {0}, {0}, &IX5_0 };
iC_Gt IX5_2    = { 1, -iC_INPX, iC_GATE, 0, "IX5.2", {0}, {0}, &IX5_1 };
iC_Gt IX5_3    = { 1, -iC_INPX, iC_GATE, 0, "IX5.3", {0}, {0}, &IX5_2 };
iC_Gt QX0_0    = { 1, -iC_AND, iC_GATE, 0, "QX0.0", {0}, {&iC_l_[0]}, &IX5_3 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[4]}, &QX0_0 };
iC_Gt QX0_1    = { 1, -iC_AND, iC_GATE, 0, "QX0.1", {0}, {&iC_l_[7]}, &QX0_0_0 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[11]}, &QX0_1 };
iC_Gt QX0_2    = { 1, -iC_AND, iC_GATE, 0, "QX0.2", {0}, {&iC_l_[14]}, &QX0_1_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[18]}, &QX0_2 };
iC_Gt QX0_3    = { 1, -iC_AND, iC_GATE, 0, "QX0.3", {0}, {&iC_l_[21]}, &QX0_2_0 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[25]}, &QX0_3 };
iC_Gt QX1_0    = { 1, -iC_AND, iC_GATE, 0, "QX1.0", {0}, {&iC_l_[28]}, &QX0_3_0 };
iC_Gt QX1_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.0_0", {0}, {&iC_l_[32]}, &QX1_0 };
iC_Gt QX1_1    = { 1, -iC_AND, iC_GATE, 0, "QX1.1", {0}, {&iC_l_[35]}, &QX1_0_0 };
iC_Gt QX1_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.1_0", {0}, {&iC_l_[39]}, &QX1_1 };
iC_Gt QX1_2    = { 1, -iC_AND, iC_GATE, 0, "QX1.2", {0}, {&iC_l_[42]}, &QX1_1_0 };
iC_Gt QX1_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.2_0", {0}, {&iC_l_[46]}, &QX1_2 };
iC_Gt QX1_3    = { 1, -iC_AND, iC_GATE, 0, "QX1.3", {0}, {&iC_l_[49]}, &QX1_2_0 };
iC_Gt QX1_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.3_0", {0}, {&iC_l_[53]}, &QX1_3 };
iC_Gt QX2_0    = { 1, -iC_AND, iC_GATE, 0, "QX2.0", {0}, {&iC_l_[56]}, &QX1_3_0 };
iC_Gt QX2_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.0_0", {0}, {&iC_l_[60]}, &QX2_0 };
iC_Gt QX2_1    = { 1, -iC_AND, iC_GATE, 0, "QX2.1", {0}, {&iC_l_[63]}, &QX2_0_0 };
iC_Gt QX2_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.1_0", {0}, {&iC_l_[67]}, &QX2_1 };
iC_Gt QX2_2    = { 1, -iC_AND, iC_GATE, 0, "QX2.2", {0}, {&iC_l_[70]}, &QX2_1_0 };
iC_Gt QX2_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.2_0", {0}, {&iC_l_[74]}, &QX2_2 };
iC_Gt QX2_3    = { 1, -iC_AND, iC_GATE, 0, "QX2.3", {0}, {&iC_l_[77]}, &QX2_2_0 };
iC_Gt QX2_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.3_0", {0}, {&iC_l_[81]}, &QX2_3 };
iC_Gt QX3_0    = { 1, -iC_AND, iC_GATE, 0, "QX3.0", {0}, {&iC_l_[84]}, &QX2_3_0 };
iC_Gt QX3_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.0_0", {0}, {&iC_l_[88]}, &QX3_0 };
iC_Gt QX3_1    = { 1, -iC_AND, iC_GATE, 0, "QX3.1", {0}, {&iC_l_[91]}, &QX3_0_0 };
iC_Gt QX3_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.1_0", {0}, {&iC_l_[95]}, &QX3_1 };
iC_Gt QX3_2    = { 1, -iC_AND, iC_GATE, 0, "QX3.2", {0}, {&iC_l_[98]}, &QX3_1_0 };
iC_Gt QX3_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.2_0", {0}, {&iC_l_[102]}, &QX3_2 };
iC_Gt QX3_3    = { 1, -iC_AND, iC_GATE, 0, "QX3.3", {0}, {&iC_l_[105]}, &QX3_2_0 };
iC_Gt QX3_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.3_0", {0}, {&iC_l_[109]}, &QX3_3 };
iC_Gt QX4_0    = { 1, -iC_AND, iC_GATE, 0, "QX4.0", {0}, {&iC_l_[112]}, &QX3_3_0 };
iC_Gt QX4_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.0_0", {0}, {&iC_l_[116]}, &QX4_0 };
iC_Gt QX4_1    = { 1, -iC_AND, iC_GATE, 0, "QX4.1", {0}, {&iC_l_[119]}, &QX4_0_0 };
iC_Gt QX4_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.1_0", {0}, {&iC_l_[123]}, &QX4_1 };
iC_Gt QX4_2    = { 1, -iC_AND, iC_GATE, 0, "QX4.2", {0}, {&iC_l_[126]}, &QX4_1_0 };
iC_Gt QX4_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.2_0", {0}, {&iC_l_[130]}, &QX4_2 };
iC_Gt QX4_3    = { 1, -iC_AND, iC_GATE, 0, "QX4.3", {0}, {&iC_l_[133]}, &QX4_2_0 };
iC_Gt QX4_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.3_0", {0}, {&iC_l_[137]}, &QX4_3 };
iC_Gt QX5_0    = { 1, -iC_AND, iC_GATE, 0, "QX5.0", {0}, {&iC_l_[140]}, &QX4_3_0 };
iC_Gt QX5_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.0_0", {0}, {&iC_l_[144]}, &QX5_0 };
iC_Gt QX5_1    = { 1, -iC_AND, iC_GATE, 0, "QX5.1", {0}, {&iC_l_[147]}, &QX5_0_0 };
iC_Gt QX5_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.1_0", {0}, {&iC_l_[151]}, &QX5_1 };
iC_Gt QX5_2    = { 1, -iC_AND, iC_GATE, 0, "QX5.2", {0}, {&iC_l_[154]}, &QX5_1_0 };
iC_Gt QX5_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.2_0", {0}, {&iC_l_[158]}, &QX5_2 };
iC_Gt QX5_3    = { 1, -iC_AND, iC_GATE, 0, "QX5.3", {0}, {&iC_l_[161]}, &QX5_2_0 };
iC_Gt QX5_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.3_0", {0}, {&iC_l_[165]}, &QX5_3 };
iC_Gt a0       = { 1, -iC_ARN, iC_GATE, 0, "a0", {0}, {&iC_l_[168]}, &QX5_3_0 };
iC_Gt a1       = { 1, -iC_ARN, iC_GATE, 0, "a1", {0}, {&iC_l_[171]}, &a0 };
iC_Gt a10      = { 1, -iC_ARN, iC_GATE, 0, "a10", {0}, {&iC_l_[174]}, &a1 };
iC_Gt a11      = { 1, -iC_ARN, iC_GATE, 0, "a11", {0}, {&iC_l_[177]}, &a10 };
iC_Gt a12      = { 1, -iC_ARN, iC_GATE, 0, "a12", {0}, {&iC_l_[180]}, &a11 };
iC_Gt a13      = { 1, -iC_ARN, iC_GATE, 0, "a13", {0}, {&iC_l_[183]}, &a12 };
iC_Gt a14      = { 1, -iC_ARN, iC_GATE, 0, "a14", {0}, {&iC_l_[186]}, &a13 };
iC_Gt a15      = { 1, -iC_ARN, iC_GATE, 0, "a15", {0}, {&iC_l_[189]}, &a14 };
iC_Gt a16      = { 1, -iC_ARN, iC_GATE, 0, "a16", {0}, {&iC_l_[192]}, &a15 };
iC_Gt a17      = { 1, -iC_ARN, iC_GATE, 0, "a17", {0}, {&iC_l_[195]}, &a16 };
iC_Gt a18      = { 1, -iC_ARN, iC_GATE, 0, "a18", {0}, {&iC_l_[198]}, &a17 };
iC_Gt a19      = { 1, -iC_ARN, iC_GATE, 0, "a19", {0}, {&iC_l_[201]}, &a18 };
iC_Gt a2       = { 1, -iC_ARN, iC_GATE, 0, "a2", {0}, {&iC_l_[204]}, &a19 };
iC_Gt a20      = { 1, -iC_ARN, iC_GATE, 0, "a20", {0}, {&iC_l_[207]}, &a2 };
iC_Gt a21      = { 1, -iC_ARN, iC_GATE, 0, "a21", {0}, {&iC_l_[210]}, &a20 };
iC_Gt a22      = { 1, -iC_ARN, iC_GATE, 0, "a22", {0}, {&iC_l_[213]}, &a21 };
iC_Gt a23      = { 1, -iC_ARN, iC_GATE, 0, "a23", {0}, {&iC_l_[216]}, &a22 };
iC_Gt a3       = { 1, -iC_ARN, iC_GATE, 0, "a3", {0}, {&iC_l_[219]}, &a23 };
iC_Gt a4       = { 1, -iC_ARN, iC_GATE, 0, "a4", {0}, {&iC_l_[222]}, &a3 };
iC_Gt a5       = { 1, -iC_ARN, iC_GATE, 0, "a5", {0}, {&iC_l_[225]}, &a4 };
iC_Gt a6       = { 1, -iC_ARN, iC_GATE, 0, "a6", {0}, {&iC_l_[228]}, &a5 };
iC_Gt a7       = { 1, -iC_ARN, iC_GATE, 0, "a7", {0}, {&iC_l_[231]}, &a6 };
iC_Gt a8       = { 1, -iC_ARN, iC_GATE, 0, "a8", {0}, {&iC_l_[234]}, &a7 };
iC_Gt a9       = { 1, -iC_ARN, iC_GATE, 0, "a9", {0}, {&iC_l_[237]}, &a8 };

iC_Gt *		iC___Test0_xx1_list = &a9;
iC_Gt **	iC_list[] = { &iC___Test0_xx1_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QX0.0 */	&a0, &IX0_0, 0, 0,
/* QX0.0_0 */	&QX0_0, 0, 0,
/* QX0.1 */	&a1, &IX0_1, 0, 0,
/* QX0.1_0 */	&QX0_1, 0, 0,
/* QX0.2 */	&a2, &IX0_2, 0, 0,
/* QX0.2_0 */	&QX0_2, 0, 0,
/* QX0.3 */	&a3, &IX0_3, 0, 0,
/* QX0.3_0 */	&QX0_3, 0, 0,
/* QX1.0 */	&a4, &IX1_0, 0, 0,
/* QX1.0_0 */	&QX1_0, 0, 0,
/* QX1.1 */	&a5, &IX1_1, 0, 0,
/* QX1.1_0 */	&QX1_1, 0, 0,
/* QX1.2 */	&a6, &IX1_2, 0, 0,
/* QX1.2_0 */	&QX1_2, 0, 0,
/* QX1.3 */	&a7, &IX1_3, 0, 0,
/* QX1.3_0 */	&QX1_3, 0, 0,
/* QX2.0 */	&a8, &IX2_0, 0, 0,
/* QX2.0_0 */	&QX2_0, 0, 0,
/* QX2.1 */	&a9, &IX2_1, 0, 0,
/* QX2.1_0 */	&QX2_1, 0, 0,
/* QX2.2 */	&a10, &IX2_2, 0, 0,
/* QX2.2_0 */	&QX2_2, 0, 0,
/* QX2.3 */	&a11, &IX2_3, 0, 0,
/* QX2.3_0 */	&QX2_3, 0, 0,
/* QX3.0 */	&a12, &IX3_0, 0, 0,
/* QX3.0_0 */	&QX3_0, 0, 0,
/* QX3.1 */	&a13, &IX3_1, 0, 0,
/* QX3.1_0 */	&QX3_1, 0, 0,
/* QX3.2 */	&a14, &IX3_2, 0, 0,
/* QX3.2_0 */	&QX3_2, 0, 0,
/* QX3.3 */	&a15, &IX3_3, 0, 0,
/* QX3.3_0 */	&QX3_3, 0, 0,
/* QX4.0 */	&a16, &IX4_0, 0, 0,
/* QX4.0_0 */	&QX4_0, 0, 0,
/* QX4.1 */	&a17, &IX4_1, 0, 0,
/* QX4.1_0 */	&QX4_1, 0, 0,
/* QX4.2 */	&a18, &IX4_2, 0, 0,
/* QX4.2_0 */	&QX4_2, 0, 0,
/* QX4.3 */	&a19, &IX4_3, 0, 0,
/* QX4.3_0 */	&QX4_3, 0, 0,
/* QX5.0 */	&a20, &IX5_0, 0, 0,
/* QX5.0_0 */	&QX5_0, 0, 0,
/* QX5.1 */	&a21, &IX5_1, 0, 0,
/* QX5.1_0 */	&QX5_1, 0, 0,
/* QX5.2 */	&a22, &IX5_2, 0, 0,
/* QX5.2_0 */	&QX5_2, 0, 0,
/* QX5.3 */	&a23, &IX5_3, 0, 0,
/* QX5.3_0 */	&QX5_3, 0, 0,
/* a0 */	(iC_Gt*)0, &IB0, 0,
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
/* a2 */	(iC_Gt*)0, &IB2, 0,
/* a20 */	(iC_Gt*)0, &IB20, 0,
/* a21 */	(iC_Gt*)0, &IB21, 0,
/* a22 */	(iC_Gt*)0, &IB22, 0,
/* a23 */	(iC_Gt*)0, &IB23, 0,
/* a3 */	(iC_Gt*)0, &IB3, 0,
/* a4 */	(iC_Gt*)0, &IB4, 0,
/* a5 */	(iC_Gt*)0, &IB5, 0,
/* a6 */	(iC_Gt*)0, &IB6, 0,
/* a7 */	(iC_Gt*)0, &IB7, 0,
/* a8 */	(iC_Gt*)0, &IB8, 0,
/* a9 */	(iC_Gt*)0, &IB9, 0,
};

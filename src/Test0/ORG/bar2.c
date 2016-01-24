/********************************************************************
 *
 *	SOURCE:   ./Test0/bar2.ic
 *	OUTPUT:   ./Test0/bar2.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: bar2.c,v 1.1 2015/05/24 00:51:48 jw Exp $ -O7";

#include	<icg.h>

static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB4      = { 1, -iC_INPW, iC_ARITH, 0, "IB4", {0}, {0}, 0 };
iC_Gt IB5      = { 1, -iC_INPW, iC_ARITH, 0, "IB5", {0}, {0}, &IB4 };
iC_Gt IB6      = { 1, -iC_INPW, iC_ARITH, 0, "IB6", {0}, {0}, &IB5 };
iC_Gt IB7      = { 1, -iC_INPW, iC_ARITH, 0, "IB7", {0}, {0}, &IB6 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB7 };
iC_Gt IX1_0    = { 1, -iC_INPX, iC_GATE, 0, "IX1.0", {0}, {0}, &IX0_0 };
iC_Gt IX2_0    = { 1, -iC_INPX, iC_GATE, 0, "IX2.0", {0}, {0}, &IX1_0 };
iC_Gt IX3_0    = { 1, -iC_INPX, iC_GATE, 0, "IX3.0", {0}, {0}, &IX2_0 };
iC_Gt QB4_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB4_0", {0}, {&iC_l_[0]}, &IX3_0 };
iC_Gt QB5_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB5_0", {0}, {&iC_l_[3]}, &QB4_0 };
iC_Gt QB6_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB6_0", {0}, {&iC_l_[6]}, &QB5_0 };
iC_Gt QB7_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB7_0", {0}, {&iC_l_[9]}, &QB6_0 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[12]}, &QB7_0 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[15]}, &QX0_0_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[18]}, &QX0_1_0 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[21]}, &QX0_2_0 };
iC_Gt QX0_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.4_0", {0}, {&iC_l_[24]}, &QX0_3_0 };
iC_Gt QX0_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.5_0", {0}, {&iC_l_[27]}, &QX0_4_0 };
iC_Gt QX0_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.6_0", {0}, {&iC_l_[30]}, &QX0_5_0 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[33]}, &QX0_6_0 };
iC_Gt QX1_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.0_0", {0}, {&iC_l_[36]}, &QX0_7_0 };
iC_Gt QX1_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.1_0", {0}, {&iC_l_[39]}, &QX1_0_0 };
iC_Gt QX1_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.2_0", {0}, {&iC_l_[42]}, &QX1_1_0 };
iC_Gt QX1_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.3_0", {0}, {&iC_l_[45]}, &QX1_2_0 };
iC_Gt QX1_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.4_0", {0}, {&iC_l_[48]}, &QX1_3_0 };
iC_Gt QX1_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.5_0", {0}, {&iC_l_[51]}, &QX1_4_0 };
iC_Gt QX1_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.6_0", {0}, {&iC_l_[54]}, &QX1_5_0 };
iC_Gt QX1_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.7_0", {0}, {&iC_l_[57]}, &QX1_6_0 };
iC_Gt QX2_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.0_0", {0}, {&iC_l_[60]}, &QX1_7_0 };
iC_Gt QX2_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.1_0", {0}, {&iC_l_[63]}, &QX2_0_0 };
iC_Gt QX2_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.2_0", {0}, {&iC_l_[66]}, &QX2_1_0 };
iC_Gt QX2_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.3_0", {0}, {&iC_l_[69]}, &QX2_2_0 };
iC_Gt QX2_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.4_0", {0}, {&iC_l_[72]}, &QX2_3_0 };
iC_Gt QX2_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.5_0", {0}, {&iC_l_[75]}, &QX2_4_0 };
iC_Gt QX2_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.6_0", {0}, {&iC_l_[78]}, &QX2_5_0 };
iC_Gt QX2_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.7_0", {0}, {&iC_l_[81]}, &QX2_6_0 };
iC_Gt QX3_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.0_0", {0}, {&iC_l_[84]}, &QX2_7_0 };
iC_Gt QX3_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.1_0", {0}, {&iC_l_[87]}, &QX3_0_0 };
iC_Gt QX3_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.2_0", {0}, {&iC_l_[90]}, &QX3_1_0 };
iC_Gt QX3_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.3_0", {0}, {&iC_l_[93]}, &QX3_2_0 };
iC_Gt QX3_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.4_0", {0}, {&iC_l_[96]}, &QX3_3_0 };
iC_Gt QX3_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.5_0", {0}, {&iC_l_[99]}, &QX3_4_0 };
iC_Gt QX3_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.6_0", {0}, {&iC_l_[102]}, &QX3_5_0 };
iC_Gt QX3_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.7_0", {0}, {&iC_l_[105]}, &QX3_6_0 };
iC_Gt QX8_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX8.0_0", {0}, {&iC_l_[108]}, &QX3_7_0 };
iC_Gt QX8_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX8.1_0", {0}, {&iC_l_[111]}, &QX8_0_0 };
iC_Gt QX8_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX8.2_0", {0}, {&iC_l_[114]}, &QX8_1_0 };
iC_Gt QX8_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX8.3_0", {0}, {&iC_l_[117]}, &QX8_2_0 };
iC_Gt QX8_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX8.4_0", {0}, {&iC_l_[120]}, &QX8_3_0 };
iC_Gt QX8_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX8.5_0", {0}, {&iC_l_[123]}, &QX8_4_0 };
iC_Gt QX8_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX8.6_0", {0}, {&iC_l_[126]}, &QX8_5_0 };
iC_Gt QX8_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX8.7_0", {0}, {&iC_l_[129]}, &QX8_6_0 };
iC_Gt TX0_4    = { 1, -iC_INPX, iC_GATE, 0, "TX0.4", {0}, {0}, &QX8_7_0 };
iC_Gt b0       = { 1, -iC_OR, iC_GATE, 0, "b0", {0}, {&iC_l_[132]}, &TX0_4 };
iC_Gt b0_1     = { 1, -iC_FF, iC_GATE, 0, "b0_1", {0}, {0}, &b0 };
iC_Gt b0_2     = { 1, -iC_VF, iC_GATE, 0, "b0_2", {0}, {0}, &b0_1 };
iC_Gt b0_3     = { 1, -iC_AND, iC_CH_BIT, 0, "b0_3", {&iC_l_[136]}, {&iC_l_[138]}, &b0_2 };
iC_Gt b0_4     = { 1, -iC_AND, iC_D_FF, 0, "b0_4", {&iC_l_[142]}, {&iC_l_[145]}, &b0_3 };
iC_Gt b1       = { 1, -iC_OR, iC_GATE, 0, "b1", {0}, {&iC_l_[149]}, &b0_4 };
iC_Gt b1_1     = { 1, -iC_FF, iC_GATE, 0, "b1_1", {0}, {0}, &b1 };
iC_Gt b1_2     = { 1, -iC_VF, iC_GATE, 0, "b1_2", {0}, {0}, &b1_1 };
iC_Gt b1_3     = { 1, -iC_AND, iC_CH_BIT, 0, "b1_3", {&iC_l_[153]}, {&iC_l_[155]}, &b1_2 };
iC_Gt b1_4     = { 1, -iC_AND, iC_D_FF, 0, "b1_4", {&iC_l_[159]}, {&iC_l_[162]}, &b1_3 };
iC_Gt b2       = { 1, -iC_OR, iC_GATE, 0, "b2", {0}, {&iC_l_[166]}, &b1_4 };
iC_Gt b2_1     = { 1, -iC_FF, iC_GATE, 0, "b2_1", {0}, {0}, &b2 };
iC_Gt b2_2     = { 1, -iC_VF, iC_GATE, 0, "b2_2", {0}, {0}, &b2_1 };
iC_Gt b2_3     = { 1, -iC_AND, iC_CH_BIT, 0, "b2_3", {&iC_l_[170]}, {&iC_l_[172]}, &b2_2 };
iC_Gt b2_4     = { 1, -iC_AND, iC_D_FF, 0, "b2_4", {&iC_l_[176]}, {&iC_l_[179]}, &b2_3 };
iC_Gt b3       = { 1, -iC_OR, iC_GATE, 0, "b3", {0}, {&iC_l_[183]}, &b2_4 };
iC_Gt b3_1     = { 1, -iC_FF, iC_GATE, 0, "b3_1", {0}, {0}, &b3 };
iC_Gt b3_2     = { 1, -iC_VF, iC_GATE, 0, "b3_2", {0}, {0}, &b3_1 };
iC_Gt b3_3     = { 1, -iC_AND, iC_CH_BIT, 0, "b3_3", {&iC_l_[187]}, {&iC_l_[189]}, &b3_2 };
iC_Gt b3_4     = { 1, -iC_AND, iC_D_FF, 0, "b3_4", {&iC_l_[193]}, {&iC_l_[196]}, &b3_3 };
iC_Gt c0       = { 1, -iC_CLK, iC_CLCKL, 0, "c0", {0}, {0}, &b3_4 };
iC_Gt c0_1     = { 1, -iC_OR, iC_CLCK, 0, "c0_1", {&iC_l_[200]}, {&iC_l_[202]}, &c0 };
iC_Gt c1       = { 1, -iC_CLK, iC_CLCKL, 0, "c1", {0}, {0}, &c0_1 };
iC_Gt c1_1     = { 1, -iC_OR, iC_CLCK, 0, "c1_1", {&iC_l_[205]}, {&iC_l_[207]}, &c1 };
iC_Gt c2       = { 1, -iC_CLK, iC_CLCKL, 0, "c2", {0}, {0}, &c1_1 };
iC_Gt c2_1     = { 1, -iC_OR, iC_CLCK, 0, "c2_1", {&iC_l_[210]}, {&iC_l_[212]}, &c2 };
iC_Gt c3       = { 1, -iC_CLK, iC_CLCKL, 0, "c3", {0}, {0}, &c2_1 };
iC_Gt c3_1     = { 1, -iC_OR, iC_CLCK, 0, "c3_1", {&iC_l_[215]}, {&iC_l_[217]}, &c3 };
iC_Gt f0       = { 1, -iC_ARN, iC_GATE, 0, "f0", {0}, {&iC_l_[220]}, &c3_1 };
iC_Gt f1       = { 1, -iC_ARN, iC_GATE, 0, "f1", {0}, {&iC_l_[223]}, &f0 };
iC_Gt f2       = { 1, -iC_ARN, iC_GATE, 0, "f2", {0}, {&iC_l_[226]}, &f1 };
iC_Gt f3       = { 1, -iC_ARN, iC_GATE, 0, "f3", {0}, {&iC_l_[229]}, &f2 };
iC_Gt m00      = { 1, -iC_FF, iC_GATE, 0, "m00", {0}, {0}, &f3 };
iC_Gt m00_1    = { 1, -iC_OR, iC_S_FF, 0, "m00_1", {&iC_l_[232]}, {&iC_l_[234]}, &m00 };
iC_Gt m00_2    = { 1, -iC_AND, iC_R_FF, 0, "m00_2", {&iC_l_[237]}, {&iC_l_[239]}, &m00_1 };
iC_Gt m01      = { 1, -iC_FF, iC_GATE, 0, "m01", {0}, {0}, &m00_2 };
iC_Gt m01_1    = { 1, -iC_AND, iC_S_FF, 0, "m01_1", {&iC_l_[243]}, {&iC_l_[245]}, &m01 };
iC_Gt m01_2    = { 1, -iC_AND, iC_R_FF, 0, "m01_2", {&iC_l_[249]}, {&iC_l_[251]}, &m01_1 };
iC_Gt m02      = { 1, -iC_FF, iC_GATE, 0, "m02", {0}, {0}, &m01_2 };
iC_Gt m02_1    = { 1, -iC_AND, iC_S_FF, 0, "m02_1", {&iC_l_[255]}, {&iC_l_[257]}, &m02 };
iC_Gt m02_2    = { 1, -iC_AND, iC_R_FF, 0, "m02_2", {&iC_l_[261]}, {&iC_l_[263]}, &m02_1 };
iC_Gt m03      = { 1, -iC_FF, iC_GATE, 0, "m03", {0}, {0}, &m02_2 };
iC_Gt m03_1    = { 1, -iC_AND, iC_S_FF, 0, "m03_1", {&iC_l_[267]}, {&iC_l_[269]}, &m03 };
iC_Gt m03_2    = { 1, -iC_AND, iC_R_FF, 0, "m03_2", {&iC_l_[273]}, {&iC_l_[275]}, &m03_1 };
iC_Gt m04      = { 1, -iC_FF, iC_GATE, 0, "m04", {0}, {0}, &m03_2 };
iC_Gt m04_1    = { 1, -iC_AND, iC_S_FF, 0, "m04_1", {&iC_l_[279]}, {&iC_l_[281]}, &m04 };
iC_Gt m04_2    = { 1, -iC_AND, iC_R_FF, 0, "m04_2", {&iC_l_[285]}, {&iC_l_[287]}, &m04_1 };
iC_Gt m05      = { 1, -iC_FF, iC_GATE, 0, "m05", {0}, {0}, &m04_2 };
iC_Gt m05_1    = { 1, -iC_AND, iC_S_FF, 0, "m05_1", {&iC_l_[291]}, {&iC_l_[293]}, &m05 };
iC_Gt m05_2    = { 1, -iC_AND, iC_R_FF, 0, "m05_2", {&iC_l_[297]}, {&iC_l_[299]}, &m05_1 };
iC_Gt m06      = { 1, -iC_FF, iC_GATE, 0, "m06", {0}, {0}, &m05_2 };
iC_Gt m06_1    = { 1, -iC_AND, iC_S_FF, 0, "m06_1", {&iC_l_[303]}, {&iC_l_[305]}, &m06 };
iC_Gt m06_2    = { 1, -iC_AND, iC_R_FF, 0, "m06_2", {&iC_l_[309]}, {&iC_l_[311]}, &m06_1 };
iC_Gt m07      = { 1, -iC_FF, iC_GATE, 0, "m07", {0}, {0}, &m06_2 };
iC_Gt m07_1    = { 1, -iC_AND, iC_S_FF, 0, "m07_1", {&iC_l_[315]}, {&iC_l_[317]}, &m07 };
iC_Gt m07_2    = { 1, -iC_OR, iC_R_FF, 0, "m07_2", {&iC_l_[321]}, {&iC_l_[323]}, &m07_1 };
iC_Gt m10      = { 1, -iC_FF, iC_GATE, 0, "m10", {0}, {0}, &m07_2 };
iC_Gt m10_1    = { 1, -iC_OR, iC_S_FF, 0, "m10_1", {&iC_l_[326]}, {&iC_l_[328]}, &m10 };
iC_Gt m10_2    = { 1, -iC_AND, iC_R_FF, 0, "m10_2", {&iC_l_[331]}, {&iC_l_[333]}, &m10_1 };
iC_Gt m11      = { 1, -iC_FF, iC_GATE, 0, "m11", {0}, {0}, &m10_2 };
iC_Gt m11_1    = { 1, -iC_AND, iC_S_FF, 0, "m11_1", {&iC_l_[337]}, {&iC_l_[339]}, &m11 };
iC_Gt m11_2    = { 1, -iC_AND, iC_R_FF, 0, "m11_2", {&iC_l_[343]}, {&iC_l_[345]}, &m11_1 };
iC_Gt m12      = { 1, -iC_FF, iC_GATE, 0, "m12", {0}, {0}, &m11_2 };
iC_Gt m12_1    = { 1, -iC_AND, iC_S_FF, 0, "m12_1", {&iC_l_[349]}, {&iC_l_[351]}, &m12 };
iC_Gt m12_2    = { 1, -iC_AND, iC_R_FF, 0, "m12_2", {&iC_l_[355]}, {&iC_l_[357]}, &m12_1 };
iC_Gt m13      = { 1, -iC_FF, iC_GATE, 0, "m13", {0}, {0}, &m12_2 };
iC_Gt m13_1    = { 1, -iC_AND, iC_S_FF, 0, "m13_1", {&iC_l_[361]}, {&iC_l_[363]}, &m13 };
iC_Gt m13_2    = { 1, -iC_AND, iC_R_FF, 0, "m13_2", {&iC_l_[367]}, {&iC_l_[369]}, &m13_1 };
iC_Gt m14      = { 1, -iC_FF, iC_GATE, 0, "m14", {0}, {0}, &m13_2 };
iC_Gt m14_1    = { 1, -iC_AND, iC_S_FF, 0, "m14_1", {&iC_l_[373]}, {&iC_l_[375]}, &m14 };
iC_Gt m14_2    = { 1, -iC_AND, iC_R_FF, 0, "m14_2", {&iC_l_[379]}, {&iC_l_[381]}, &m14_1 };
iC_Gt m15      = { 1, -iC_FF, iC_GATE, 0, "m15", {0}, {0}, &m14_2 };
iC_Gt m15_1    = { 1, -iC_AND, iC_S_FF, 0, "m15_1", {&iC_l_[385]}, {&iC_l_[387]}, &m15 };
iC_Gt m15_2    = { 1, -iC_AND, iC_R_FF, 0, "m15_2", {&iC_l_[391]}, {&iC_l_[393]}, &m15_1 };
iC_Gt m16      = { 1, -iC_FF, iC_GATE, 0, "m16", {0}, {0}, &m15_2 };
iC_Gt m16_1    = { 1, -iC_AND, iC_S_FF, 0, "m16_1", {&iC_l_[397]}, {&iC_l_[399]}, &m16 };
iC_Gt m16_2    = { 1, -iC_AND, iC_R_FF, 0, "m16_2", {&iC_l_[403]}, {&iC_l_[405]}, &m16_1 };
iC_Gt m17      = { 1, -iC_FF, iC_GATE, 0, "m17", {0}, {0}, &m16_2 };
iC_Gt m17_1    = { 1, -iC_AND, iC_S_FF, 0, "m17_1", {&iC_l_[409]}, {&iC_l_[411]}, &m17 };
iC_Gt m17_2    = { 1, -iC_OR, iC_R_FF, 0, "m17_2", {&iC_l_[415]}, {&iC_l_[417]}, &m17_1 };
iC_Gt m20      = { 1, -iC_FF, iC_GATE, 0, "m20", {0}, {0}, &m17_2 };
iC_Gt m20_1    = { 1, -iC_OR, iC_S_FF, 0, "m20_1", {&iC_l_[420]}, {&iC_l_[422]}, &m20 };
iC_Gt m20_2    = { 1, -iC_AND, iC_R_FF, 0, "m20_2", {&iC_l_[425]}, {&iC_l_[427]}, &m20_1 };
iC_Gt m21      = { 1, -iC_FF, iC_GATE, 0, "m21", {0}, {0}, &m20_2 };
iC_Gt m21_1    = { 1, -iC_AND, iC_S_FF, 0, "m21_1", {&iC_l_[431]}, {&iC_l_[433]}, &m21 };
iC_Gt m21_2    = { 1, -iC_AND, iC_R_FF, 0, "m21_2", {&iC_l_[437]}, {&iC_l_[439]}, &m21_1 };
iC_Gt m22      = { 1, -iC_FF, iC_GATE, 0, "m22", {0}, {0}, &m21_2 };
iC_Gt m22_1    = { 1, -iC_AND, iC_S_FF, 0, "m22_1", {&iC_l_[443]}, {&iC_l_[445]}, &m22 };
iC_Gt m22_2    = { 1, -iC_AND, iC_R_FF, 0, "m22_2", {&iC_l_[449]}, {&iC_l_[451]}, &m22_1 };
iC_Gt m23      = { 1, -iC_FF, iC_GATE, 0, "m23", {0}, {0}, &m22_2 };
iC_Gt m23_1    = { 1, -iC_AND, iC_S_FF, 0, "m23_1", {&iC_l_[455]}, {&iC_l_[457]}, &m23 };
iC_Gt m23_2    = { 1, -iC_AND, iC_R_FF, 0, "m23_2", {&iC_l_[461]}, {&iC_l_[463]}, &m23_1 };
iC_Gt m24      = { 1, -iC_FF, iC_GATE, 0, "m24", {0}, {0}, &m23_2 };
iC_Gt m24_1    = { 1, -iC_AND, iC_S_FF, 0, "m24_1", {&iC_l_[467]}, {&iC_l_[469]}, &m24 };
iC_Gt m24_2    = { 1, -iC_AND, iC_R_FF, 0, "m24_2", {&iC_l_[473]}, {&iC_l_[475]}, &m24_1 };
iC_Gt m25      = { 1, -iC_FF, iC_GATE, 0, "m25", {0}, {0}, &m24_2 };
iC_Gt m25_1    = { 1, -iC_AND, iC_S_FF, 0, "m25_1", {&iC_l_[479]}, {&iC_l_[481]}, &m25 };
iC_Gt m25_2    = { 1, -iC_AND, iC_R_FF, 0, "m25_2", {&iC_l_[485]}, {&iC_l_[487]}, &m25_1 };
iC_Gt m26      = { 1, -iC_FF, iC_GATE, 0, "m26", {0}, {0}, &m25_2 };
iC_Gt m26_1    = { 1, -iC_AND, iC_S_FF, 0, "m26_1", {&iC_l_[491]}, {&iC_l_[493]}, &m26 };
iC_Gt m26_2    = { 1, -iC_AND, iC_R_FF, 0, "m26_2", {&iC_l_[497]}, {&iC_l_[499]}, &m26_1 };
iC_Gt m27      = { 1, -iC_FF, iC_GATE, 0, "m27", {0}, {0}, &m26_2 };
iC_Gt m27_1    = { 1, -iC_AND, iC_S_FF, 0, "m27_1", {&iC_l_[503]}, {&iC_l_[505]}, &m27 };
iC_Gt m27_2    = { 1, -iC_OR, iC_R_FF, 0, "m27_2", {&iC_l_[509]}, {&iC_l_[511]}, &m27_1 };
iC_Gt m30      = { 1, -iC_FF, iC_GATE, 0, "m30", {0}, {0}, &m27_2 };
iC_Gt m30_1    = { 1, -iC_OR, iC_S_FF, 0, "m30_1", {&iC_l_[514]}, {&iC_l_[516]}, &m30 };
iC_Gt m30_2    = { 1, -iC_AND, iC_R_FF, 0, "m30_2", {&iC_l_[519]}, {&iC_l_[521]}, &m30_1 };
iC_Gt m31      = { 1, -iC_FF, iC_GATE, 0, "m31", {0}, {0}, &m30_2 };
iC_Gt m31_1    = { 1, -iC_AND, iC_S_FF, 0, "m31_1", {&iC_l_[525]}, {&iC_l_[527]}, &m31 };
iC_Gt m31_2    = { 1, -iC_AND, iC_R_FF, 0, "m31_2", {&iC_l_[531]}, {&iC_l_[533]}, &m31_1 };
iC_Gt m32      = { 1, -iC_FF, iC_GATE, 0, "m32", {0}, {0}, &m31_2 };
iC_Gt m32_1    = { 1, -iC_AND, iC_S_FF, 0, "m32_1", {&iC_l_[537]}, {&iC_l_[539]}, &m32 };
iC_Gt m32_2    = { 1, -iC_AND, iC_R_FF, 0, "m32_2", {&iC_l_[543]}, {&iC_l_[545]}, &m32_1 };
iC_Gt m33      = { 1, -iC_FF, iC_GATE, 0, "m33", {0}, {0}, &m32_2 };
iC_Gt m33_1    = { 1, -iC_AND, iC_S_FF, 0, "m33_1", {&iC_l_[549]}, {&iC_l_[551]}, &m33 };
iC_Gt m33_2    = { 1, -iC_AND, iC_R_FF, 0, "m33_2", {&iC_l_[555]}, {&iC_l_[557]}, &m33_1 };
iC_Gt m34      = { 1, -iC_FF, iC_GATE, 0, "m34", {0}, {0}, &m33_2 };
iC_Gt m34_1    = { 1, -iC_AND, iC_S_FF, 0, "m34_1", {&iC_l_[561]}, {&iC_l_[563]}, &m34 };
iC_Gt m34_2    = { 1, -iC_AND, iC_R_FF, 0, "m34_2", {&iC_l_[567]}, {&iC_l_[569]}, &m34_1 };
iC_Gt m35      = { 1, -iC_FF, iC_GATE, 0, "m35", {0}, {0}, &m34_2 };
iC_Gt m35_1    = { 1, -iC_AND, iC_S_FF, 0, "m35_1", {&iC_l_[573]}, {&iC_l_[575]}, &m35 };
iC_Gt m35_2    = { 1, -iC_AND, iC_R_FF, 0, "m35_2", {&iC_l_[579]}, {&iC_l_[581]}, &m35_1 };
iC_Gt m36      = { 1, -iC_FF, iC_GATE, 0, "m36", {0}, {0}, &m35_2 };
iC_Gt m36_1    = { 1, -iC_AND, iC_S_FF, 0, "m36_1", {&iC_l_[585]}, {&iC_l_[587]}, &m36 };
iC_Gt m36_2    = { 1, -iC_AND, iC_R_FF, 0, "m36_2", {&iC_l_[591]}, {&iC_l_[593]}, &m36_1 };
iC_Gt m37      = { 1, -iC_FF, iC_GATE, 0, "m37", {0}, {0}, &m36_2 };
iC_Gt m37_1    = { 1, -iC_AND, iC_S_FF, 0, "m37_1", {&iC_l_[597]}, {&iC_l_[599]}, &m37 };
iC_Gt m37_2    = { 1, -iC_OR, iC_R_FF, 0, "m37_2", {&iC_l_[603]}, {&iC_l_[605]}, &m37_1 };
iC_Gt t        = { 1, -iC_TIM, iC_TIMRL, 0, "t", {0}, {0}, &m37_2 };
iC_Gt t_1      = { 1, -iC_OR, iC_TIMR, 0, "t_1", {&iC_l_[608]}, {&iC_l_[610]}, &t };
iC_Gt t_2      = { 1, -iC_OR, iC_TIMR, 0, "t_2", {&iC_l_[613]}, {&iC_l_[615]}, &t_1 };
iC_Gt up0      = { 1, -iC_FF, iC_GATE, 0, "up0", {0}, {0}, &t_2 };
iC_Gt up0_1    = { 1, -iC_AND, iC_S_FF, 0, "up0_1", {&iC_l_[618]}, {&iC_l_[620]}, &up0 };
iC_Gt up0_2    = { 1, -iC_AND, iC_R_FF, 0, "up0_2", {&iC_l_[624]}, {&iC_l_[626]}, &up0_1 };
iC_Gt up1      = { 1, -iC_FF, iC_GATE, 0, "up1", {0}, {0}, &up0_2 };
iC_Gt up1_1    = { 1, -iC_AND, iC_S_FF, 0, "up1_1", {&iC_l_[630]}, {&iC_l_[632]}, &up1 };
iC_Gt up1_2    = { 1, -iC_AND, iC_R_FF, 0, "up1_2", {&iC_l_[636]}, {&iC_l_[638]}, &up1_1 };
iC_Gt up2      = { 1, -iC_FF, iC_GATE, 0, "up2", {0}, {0}, &up1_2 };
iC_Gt up2_1    = { 1, -iC_AND, iC_S_FF, 0, "up2_1", {&iC_l_[642]}, {&iC_l_[644]}, &up2 };
iC_Gt up2_2    = { 1, -iC_AND, iC_R_FF, 0, "up2_2", {&iC_l_[648]}, {&iC_l_[650]}, &up2_1 };
iC_Gt up3      = { 1, -iC_FF, iC_GATE, 0, "up3", {0}, {0}, &up2_2 };
iC_Gt up3_1    = { 1, -iC_AND, iC_S_FF, 0, "up3_1", {&iC_l_[654]}, {&iC_l_[656]}, &up3 };
iC_Gt up3_2    = { 1, -iC_AND, iC_R_FF, 0, "up3_2", {&iC_l_[660]}, {&iC_l_[662]}, &up3_1 };
iC_Gt QB4      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB4", {0}, {(iC_Gt**)&IB4}, &up3_2, 0 };
iC_Gt QB5      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB5", {0}, {(iC_Gt**)&IB5}, &QB4, 0 };
iC_Gt QB6      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB6", {0}, {(iC_Gt**)&IB6}, &QB5, 0 };
iC_Gt QB7      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB7", {0}, {(iC_Gt**)&IB7}, &QB6, 0 };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&m00}, &QB7, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&m01}, &QX0_0, 0 };
iC_Gt QX0_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.2", {0}, {(iC_Gt**)&m02}, &QX0_1, 0 };
iC_Gt QX0_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.3", {0}, {(iC_Gt**)&m03}, &QX0_2, 0 };
iC_Gt QX0_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.4", {0}, {(iC_Gt**)&m04}, &QX0_3, 0 };
iC_Gt QX0_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.5", {0}, {(iC_Gt**)&m05}, &QX0_4, 0 };
iC_Gt QX0_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.6", {0}, {(iC_Gt**)&m06}, &QX0_5, 0 };
iC_Gt QX0_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.7", {0}, {(iC_Gt**)&m07}, &QX0_6, 0 };
iC_Gt QX1_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.0", {0}, {(iC_Gt**)&m10}, &QX0_7, 0 };
iC_Gt QX1_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.1", {0}, {(iC_Gt**)&m11}, &QX1_0, 0 };
iC_Gt QX1_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.2", {0}, {(iC_Gt**)&m12}, &QX1_1, 0 };
iC_Gt QX1_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.3", {0}, {(iC_Gt**)&m13}, &QX1_2, 0 };
iC_Gt QX1_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.4", {0}, {(iC_Gt**)&m14}, &QX1_3, 0 };
iC_Gt QX1_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.5", {0}, {(iC_Gt**)&m15}, &QX1_4, 0 };
iC_Gt QX1_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.6", {0}, {(iC_Gt**)&m16}, &QX1_5, 0 };
iC_Gt QX1_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.7", {0}, {(iC_Gt**)&m17}, &QX1_6, 0 };
iC_Gt QX2_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.0", {0}, {(iC_Gt**)&m20}, &QX1_7, 0 };
iC_Gt QX2_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.1", {0}, {(iC_Gt**)&m21}, &QX2_0, 0 };
iC_Gt QX2_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.2", {0}, {(iC_Gt**)&m22}, &QX2_1, 0 };
iC_Gt QX2_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.3", {0}, {(iC_Gt**)&m23}, &QX2_2, 0 };
iC_Gt QX2_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.4", {0}, {(iC_Gt**)&m24}, &QX2_3, 0 };
iC_Gt QX2_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.5", {0}, {(iC_Gt**)&m25}, &QX2_4, 0 };
iC_Gt QX2_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.6", {0}, {(iC_Gt**)&m26}, &QX2_5, 0 };
iC_Gt QX2_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.7", {0}, {(iC_Gt**)&m27}, &QX2_6, 0 };
iC_Gt QX3_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.0", {0}, {(iC_Gt**)&m30}, &QX2_7, 0 };
iC_Gt QX3_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.1", {0}, {(iC_Gt**)&m31}, &QX3_0, 0 };
iC_Gt QX3_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.2", {0}, {(iC_Gt**)&m32}, &QX3_1, 0 };
iC_Gt QX3_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.3", {0}, {(iC_Gt**)&m33}, &QX3_2, 0 };
iC_Gt QX3_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.4", {0}, {(iC_Gt**)&m34}, &QX3_3, 0 };
iC_Gt QX3_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.5", {0}, {(iC_Gt**)&m35}, &QX3_4, 0 };
iC_Gt QX3_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.6", {0}, {(iC_Gt**)&m36}, &QX3_5, 0 };
iC_Gt QX3_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.7", {0}, {(iC_Gt**)&m37}, &QX3_6, 0 };
iC_Gt QX8_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX8.0", {0}, {(iC_Gt**)&up0}, &QX3_7, 0 };
iC_Gt QX8_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX8.1", {0}, {(iC_Gt**)&up1}, &QX8_0, 0 };
iC_Gt QX8_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX8.2", {0}, {(iC_Gt**)&up2}, &QX8_1, 0 };
iC_Gt QX8_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX8.3", {0}, {(iC_Gt**)&up3}, &QX8_2, 0 };
iC_Gt QX8_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX8.4", {0}, {(iC_Gt**)&f0}, &QX8_3, 0 };
iC_Gt QX8_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX8.5", {0}, {(iC_Gt**)&f1}, &QX8_4, 0 };
iC_Gt QX8_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX8.6", {0}, {(iC_Gt**)&f2}, &QX8_5, 0 };
iC_Gt QX8_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX8.7", {0}, {(iC_Gt**)&f3}, &QX8_6, 0 };
iC_Gt d0       = { 1, -iC_ALIAS, iC_ARITH, 0, "d0", {0}, {(iC_Gt**)&IB4}, &QX8_7, 0 };
iC_Gt d1       = { 1, -iC_ALIAS, iC_ARITH, 0, "d1", {0}, {(iC_Gt**)&IB5}, &d0, 0 };
iC_Gt d2       = { 1, -iC_ALIAS, iC_ARITH, 0, "d2", {0}, {(iC_Gt**)&IB6}, &d1, 0 };
iC_Gt d3       = { 1, -iC_ALIAS, iC_ARITH, 0, "d3", {0}, {(iC_Gt**)&IB7}, &d2, 0 };

iC_Gt *		iC___Test0_bar2_list = &d3;
iC_Gt **	iC_list[] = { &iC___Test0_bar2_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB4_0 */	(iC_Gt*)0, &IB4, 0,
/* QB5_0 */	(iC_Gt*)0, &IB5, 0,
/* QB6_0 */	(iC_Gt*)0, &IB6, 0,
/* QB7_0 */	(iC_Gt*)0, &IB7, 0,
/* QX0.0_0 */	&m00, 0, 0,
/* QX0.1_0 */	&m01, 0, 0,
/* QX0.2_0 */	&m02, 0, 0,
/* QX0.3_0 */	&m03, 0, 0,
/* QX0.4_0 */	&m04, 0, 0,
/* QX0.5_0 */	&m05, 0, 0,
/* QX0.6_0 */	&m06, 0, 0,
/* QX0.7_0 */	&m07, 0, 0,
/* QX1.0_0 */	&m10, 0, 0,
/* QX1.1_0 */	&m11, 0, 0,
/* QX1.2_0 */	&m12, 0, 0,
/* QX1.3_0 */	&m13, 0, 0,
/* QX1.4_0 */	&m14, 0, 0,
/* QX1.5_0 */	&m15, 0, 0,
/* QX1.6_0 */	&m16, 0, 0,
/* QX1.7_0 */	&m17, 0, 0,
/* QX2.0_0 */	&m20, 0, 0,
/* QX2.1_0 */	&m21, 0, 0,
/* QX2.2_0 */	&m22, 0, 0,
/* QX2.3_0 */	&m23, 0, 0,
/* QX2.4_0 */	&m24, 0, 0,
/* QX2.5_0 */	&m25, 0, 0,
/* QX2.6_0 */	&m26, 0, 0,
/* QX2.7_0 */	&m27, 0, 0,
/* QX3.0_0 */	&m30, 0, 0,
/* QX3.1_0 */	&m31, 0, 0,
/* QX3.2_0 */	&m32, 0, 0,
/* QX3.3_0 */	&m33, 0, 0,
/* QX3.4_0 */	&m34, 0, 0,
/* QX3.5_0 */	&m35, 0, 0,
/* QX3.6_0 */	&m36, 0, 0,
/* QX3.7_0 */	&m37, 0, 0,
/* QX8.0_0 */	&up0, 0, 0,
/* QX8.1_0 */	&up1, 0, 0,
/* QX8.2_0 */	&up2, 0, 0,
/* QX8.3_0 */	&up3, 0, 0,
/* QX8.4_0 */	&f0, 0, 0,
/* QX8.5_0 */	&f1, 0, 0,
/* QX8.6_0 */	&f2, 0, 0,
/* QX8.7_0 */	&f3, 0, 0,
/* b0 */	&b0_2, &b0_1, 0, 0,
/* b0_3 */	&b0_2, &iClock,	&IX0_0, 0, &f0, 0,
/* b0_4 */	&b0_1, &t, &IB4,	&f0, 0, &b0, 0,
/* b1 */	&b1_2, &b1_1, 0, 0,
/* b1_3 */	&b1_2, &iClock,	&IX1_0, 0, &f1, 0,
/* b1_4 */	&b1_1, &t, &IB5,	&f1, 0, &b1, 0,
/* b2 */	&b2_2, &b2_1, 0, 0,
/* b2_3 */	&b2_2, &iClock,	&IX2_0, 0, &f2, 0,
/* b2_4 */	&b2_1, &t, &IB6,	&f2, 0, &b2, 0,
/* b3 */	&b3_2, &b3_1, 0, 0,
/* b3_3 */	&b3_2, &iClock,	&IX3_0, 0, &f3, 0,
/* b3_4 */	&b3_1, &t, &IB7,	&f3, 0, &b3, 0,
/* c0_1 */	&c0, &iClock,	&b0, 0, 0,
/* c1_1 */	&c1, &iClock,	&b1, 0, 0,
/* c2_1 */	&c2, &iClock,	&b2, 0, 0,
/* c3_1 */	&c3, &iClock,	&b3, 0, 0,
/* f0 */	(iC_Gt*)0, &IB4, 0,
/* f1 */	(iC_Gt*)0, &IB5, 0,
/* f2 */	(iC_Gt*)0, &IB6, 0,
/* f3 */	(iC_Gt*)0, &IB7, 0,
/* m00_1 */	&m00, &c0,	0, &up0, 0,
/* m00_2 */	&m00, &c0,	&up0, 0, &m01, 0,
/* m01_1 */	&m01, &c0,	&m00, 0, &up0, 0,
/* m01_2 */	&m01, &c0,	&up0, 0, &m02, 0,
/* m02_1 */	&m02, &c0,	&m01, 0, &up0, 0,
/* m02_2 */	&m02, &c0,	&up0, 0, &m03, 0,
/* m03_1 */	&m03, &c0,	&m02, 0, &up0, 0,
/* m03_2 */	&m03, &c0,	&up0, 0, &m04, 0,
/* m04_1 */	&m04, &c0,	&m03, 0, &up0, 0,
/* m04_2 */	&m04, &c0,	&up0, 0, &m05, 0,
/* m05_1 */	&m05, &c0,	&m04, 0, &up0, 0,
/* m05_2 */	&m05, &c0,	&up0, 0, &m06, 0,
/* m06_1 */	&m06, &c0,	&m05, 0, &up0, 0,
/* m06_2 */	&m06, &c0,	&up0, 0, &m07, 0,
/* m07_1 */	&m07, &c0,	&m06, 0, &up0, 0,
/* m07_2 */	&m07, &c0,	&up0, 0, 0,
/* m10_1 */	&m10, &c1,	0, &up1, 0,
/* m10_2 */	&m10, &c1,	&up1, 0, &m11, 0,
/* m11_1 */	&m11, &c1,	&m10, 0, &up1, 0,
/* m11_2 */	&m11, &c1,	&up1, 0, &m12, 0,
/* m12_1 */	&m12, &c1,	&m11, 0, &up1, 0,
/* m12_2 */	&m12, &c1,	&up1, 0, &m13, 0,
/* m13_1 */	&m13, &c1,	&m12, 0, &up1, 0,
/* m13_2 */	&m13, &c1,	&up1, 0, &m14, 0,
/* m14_1 */	&m14, &c1,	&m13, 0, &up1, 0,
/* m14_2 */	&m14, &c1,	&up1, 0, &m15, 0,
/* m15_1 */	&m15, &c1,	&m14, 0, &up1, 0,
/* m15_2 */	&m15, &c1,	&up1, 0, &m16, 0,
/* m16_1 */	&m16, &c1,	&m15, 0, &up1, 0,
/* m16_2 */	&m16, &c1,	&up1, 0, &m17, 0,
/* m17_1 */	&m17, &c1,	&m16, 0, &up1, 0,
/* m17_2 */	&m17, &c1,	&up1, 0, 0,
/* m20_1 */	&m20, &c2,	0, &up2, 0,
/* m20_2 */	&m20, &c2,	&up2, 0, &m21, 0,
/* m21_1 */	&m21, &c2,	&m20, 0, &up2, 0,
/* m21_2 */	&m21, &c2,	&up2, 0, &m22, 0,
/* m22_1 */	&m22, &c2,	&m21, 0, &up2, 0,
/* m22_2 */	&m22, &c2,	&up2, 0, &m23, 0,
/* m23_1 */	&m23, &c2,	&m22, 0, &up2, 0,
/* m23_2 */	&m23, &c2,	&up2, 0, &m24, 0,
/* m24_1 */	&m24, &c2,	&m23, 0, &up2, 0,
/* m24_2 */	&m24, &c2,	&up2, 0, &m25, 0,
/* m25_1 */	&m25, &c2,	&m24, 0, &up2, 0,
/* m25_2 */	&m25, &c2,	&up2, 0, &m26, 0,
/* m26_1 */	&m26, &c2,	&m25, 0, &up2, 0,
/* m26_2 */	&m26, &c2,	&up2, 0, &m27, 0,
/* m27_1 */	&m27, &c2,	&m26, 0, &up2, 0,
/* m27_2 */	&m27, &c2,	&up2, 0, 0,
/* m30_1 */	&m30, &c3,	0, &up3, 0,
/* m30_2 */	&m30, &c3,	&up3, 0, &m31, 0,
/* m31_1 */	&m31, &c3,	&m30, 0, &up3, 0,
/* m31_2 */	&m31, &c3,	&up3, 0, &m32, 0,
/* m32_1 */	&m32, &c3,	&m31, 0, &up3, 0,
/* m32_2 */	&m32, &c3,	&up3, 0, &m33, 0,
/* m33_1 */	&m33, &c3,	&m32, 0, &up3, 0,
/* m33_2 */	&m33, &c3,	&up3, 0, &m34, 0,
/* m34_1 */	&m34, &c3,	&m33, 0, &up3, 0,
/* m34_2 */	&m34, &c3,	&up3, 0, &m35, 0,
/* m35_1 */	&m35, &c3,	&m34, 0, &up3, 0,
/* m35_2 */	&m35, &c3,	&up3, 0, &m36, 0,
/* m36_1 */	&m36, &c3,	&m35, 0, &up3, 0,
/* m36_2 */	&m36, &c3,	&up3, 0, &m37, 0,
/* m37_1 */	&m37, &c3,	&m36, 0, &up3, 0,
/* m37_2 */	&m37, &c3,	&up3, 0, 0,
/* t_1 */	&t, &iClock,	&TX0_4, 0, 0,
/* t_2 */	&t, &iClock,	0, &TX0_4, 0,
/* up0_1 */	&up0, &c0,	&m06, 0, &m07, 0,
/* up0_2 */	&up0, &c0,	&m00, 0, &m01, 0,
/* up1_1 */	&up1, &c1,	&m16, 0, &m17, 0,
/* up1_2 */	&up1, &c1,	&m10, 0, &m11, 0,
/* up2_1 */	&up2, &c2,	&m26, 0, &m27, 0,
/* up2_2 */	&up2, &c2,	&m20, 0, &m21, 0,
/* up3_1 */	&up3, &c3,	&m36, 0, &m37, 0,
/* up3_2 */	&up3, &c3,	&m30, 0, &m31, 0,
};

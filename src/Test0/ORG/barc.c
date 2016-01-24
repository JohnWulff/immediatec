/********************************************************************
 *
 *	SOURCE:   ./Test0/barc.ic
 *	OUTPUT:   ./Test0/barc.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: barc.c,v 1.1 2015/05/24 00:51:49 jw Exp $ -O7";

#include	<icg.h>

static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, 0 };
iC_Gt IB3      = { 1, -iC_INPW, iC_ARITH, 0, "IB3", {0}, {0}, &IB1 };
iC_Gt IB5      = { 1, -iC_INPW, iC_ARITH, 0, "IB5", {0}, {0}, &IB3 };
iC_Gt IB7      = { 1, -iC_INPW, iC_ARITH, 0, "IB7", {0}, {0}, &IB5 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB7 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[0]}, &IX0_0 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[3]}, &QX0_0_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[6]}, &QX0_1_0 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[9]}, &QX0_2_0 };
iC_Gt QX0_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.4_0", {0}, {&iC_l_[12]}, &QX0_3_0 };
iC_Gt QX0_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.5_0", {0}, {&iC_l_[15]}, &QX0_4_0 };
iC_Gt QX0_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.6_0", {0}, {&iC_l_[18]}, &QX0_5_0 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[21]}, &QX0_6_0 };
iC_Gt QX1_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.0_0", {0}, {&iC_l_[24]}, &QX0_7_0 };
iC_Gt QX2_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.0_0", {0}, {&iC_l_[27]}, &QX1_0_0 };
iC_Gt QX2_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.1_0", {0}, {&iC_l_[30]}, &QX2_0_0 };
iC_Gt QX2_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.2_0", {0}, {&iC_l_[33]}, &QX2_1_0 };
iC_Gt QX2_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.3_0", {0}, {&iC_l_[36]}, &QX2_2_0 };
iC_Gt QX2_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.4_0", {0}, {&iC_l_[39]}, &QX2_3_0 };
iC_Gt QX2_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.5_0", {0}, {&iC_l_[42]}, &QX2_4_0 };
iC_Gt QX2_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.6_0", {0}, {&iC_l_[45]}, &QX2_5_0 };
iC_Gt QX2_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.7_0", {0}, {&iC_l_[48]}, &QX2_6_0 };
iC_Gt QX3_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.0_0", {0}, {&iC_l_[51]}, &QX2_7_0 };
iC_Gt QX4_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.0_0", {0}, {&iC_l_[54]}, &QX3_0_0 };
iC_Gt QX4_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.1_0", {0}, {&iC_l_[57]}, &QX4_0_0 };
iC_Gt QX4_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.2_0", {0}, {&iC_l_[60]}, &QX4_1_0 };
iC_Gt QX4_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.3_0", {0}, {&iC_l_[63]}, &QX4_2_0 };
iC_Gt QX4_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.4_0", {0}, {&iC_l_[66]}, &QX4_3_0 };
iC_Gt QX4_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.5_0", {0}, {&iC_l_[69]}, &QX4_4_0 };
iC_Gt QX4_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.6_0", {0}, {&iC_l_[72]}, &QX4_5_0 };
iC_Gt QX4_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.7_0", {0}, {&iC_l_[75]}, &QX4_6_0 };
iC_Gt QX5_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.0_0", {0}, {&iC_l_[78]}, &QX4_7_0 };
iC_Gt QX6_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.0_0", {0}, {&iC_l_[81]}, &QX5_0_0 };
iC_Gt QX6_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.1_0", {0}, {&iC_l_[84]}, &QX6_0_0 };
iC_Gt QX6_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.2_0", {0}, {&iC_l_[87]}, &QX6_1_0 };
iC_Gt QX6_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.3_0", {0}, {&iC_l_[90]}, &QX6_2_0 };
iC_Gt QX6_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.4_0", {0}, {&iC_l_[93]}, &QX6_3_0 };
iC_Gt QX6_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.5_0", {0}, {&iC_l_[96]}, &QX6_4_0 };
iC_Gt QX6_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.6_0", {0}, {&iC_l_[99]}, &QX6_5_0 };
iC_Gt QX6_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.7_0", {0}, {&iC_l_[102]}, &QX6_6_0 };
iC_Gt QX7_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX7.0_0", {0}, {&iC_l_[105]}, &QX6_7_0 };
iC_Gt TX0_4    = { 1, -iC_INPX, iC_GATE, 0, "TX0.4", {0}, {0}, &QX7_0_0 };
iC_Gt b0       = { 1, -iC_OR, iC_GATE, 0, "b0", {0}, {&iC_l_[108]}, &TX0_4 };
iC_Gt b0_1     = { 1, -iC_FF, iC_GATE, 0, "b0_1", {0}, {0}, &b0 };
iC_Gt b0_2     = { 1, -iC_VF, iC_GATE, 0, "b0_2", {0}, {0}, &b0_1 };
iC_Gt b0_3     = { 1, -iC_AND, iC_CH_BIT, 0, "b0_3", {&iC_l_[112]}, {&iC_l_[114]}, &b0_2 };
iC_Gt b0_4     = { 1, -iC_AND, iC_D_FF, 0, "b0_4", {&iC_l_[118]}, {&iC_l_[121]}, &b0_3 };
iC_Gt b1       = { 1, -iC_OR, iC_GATE, 0, "b1", {0}, {&iC_l_[125]}, &b0_4 };
iC_Gt b1_1     = { 1, -iC_FF, iC_GATE, 0, "b1_1", {0}, {0}, &b1 };
iC_Gt b1_2     = { 1, -iC_VF, iC_GATE, 0, "b1_2", {0}, {0}, &b1_1 };
iC_Gt b1_3     = { 1, -iC_AND, iC_CH_BIT, 0, "b1_3", {&iC_l_[129]}, {&iC_l_[131]}, &b1_2 };
iC_Gt b1_4     = { 1, -iC_AND, iC_D_FF, 0, "b1_4", {&iC_l_[135]}, {&iC_l_[138]}, &b1_3 };
iC_Gt b2       = { 1, -iC_OR, iC_GATE, 0, "b2", {0}, {&iC_l_[142]}, &b1_4 };
iC_Gt b2_1     = { 1, -iC_FF, iC_GATE, 0, "b2_1", {0}, {0}, &b2 };
iC_Gt b2_2     = { 1, -iC_VF, iC_GATE, 0, "b2_2", {0}, {0}, &b2_1 };
iC_Gt b2_3     = { 1, -iC_AND, iC_CH_BIT, 0, "b2_3", {&iC_l_[146]}, {&iC_l_[148]}, &b2_2 };
iC_Gt b2_4     = { 1, -iC_AND, iC_D_FF, 0, "b2_4", {&iC_l_[152]}, {&iC_l_[155]}, &b2_3 };
iC_Gt b3       = { 1, -iC_OR, iC_GATE, 0, "b3", {0}, {&iC_l_[159]}, &b2_4 };
iC_Gt b3_1     = { 1, -iC_FF, iC_GATE, 0, "b3_1", {0}, {0}, &b3 };
iC_Gt b3_2     = { 1, -iC_VF, iC_GATE, 0, "b3_2", {0}, {0}, &b3_1 };
iC_Gt b3_3     = { 1, -iC_AND, iC_CH_BIT, 0, "b3_3", {&iC_l_[163]}, {&iC_l_[165]}, &b3_2 };
iC_Gt b3_4     = { 1, -iC_AND, iC_D_FF, 0, "b3_4", {&iC_l_[169]}, {&iC_l_[172]}, &b3_3 };
iC_Gt c0       = { 1, -iC_CLK, iC_CLCKL, 0, "c0", {0}, {0}, &b3_4 };
iC_Gt c0_1     = { 1, -iC_OR, iC_CLCK, 0, "c0_1", {&iC_l_[176]}, {&iC_l_[178]}, &c0 };
iC_Gt c1       = { 1, -iC_CLK, iC_CLCKL, 0, "c1", {0}, {0}, &c0_1 };
iC_Gt c1_1     = { 1, -iC_OR, iC_CLCK, 0, "c1_1", {&iC_l_[181]}, {&iC_l_[183]}, &c1 };
iC_Gt c2       = { 1, -iC_CLK, iC_CLCKL, 0, "c2", {0}, {0}, &c1_1 };
iC_Gt c2_1     = { 1, -iC_OR, iC_CLCK, 0, "c2_1", {&iC_l_[186]}, {&iC_l_[188]}, &c2 };
iC_Gt c3       = { 1, -iC_CLK, iC_CLCKL, 0, "c3", {0}, {0}, &c2_1 };
iC_Gt c3_1     = { 1, -iC_OR, iC_CLCK, 0, "c3_1", {&iC_l_[191]}, {&iC_l_[193]}, &c3 };
iC_Gt f0       = { 1, -iC_ARN, iC_GATE, 0, "f0", {0}, {&iC_l_[196]}, &c3_1 };
iC_Gt f1       = { 1, -iC_ARN, iC_GATE, 0, "f1", {0}, {&iC_l_[199]}, &f0 };
iC_Gt f2       = { 1, -iC_ARN, iC_GATE, 0, "f2", {0}, {&iC_l_[202]}, &f1 };
iC_Gt f3       = { 1, -iC_ARN, iC_GATE, 0, "f3", {0}, {&iC_l_[205]}, &f2 };
iC_Gt m0x0     = { 1, -iC_FF, iC_GATE, 0, "m0x0", {0}, {0}, &f3 };
iC_Gt m0x0_1   = { 1, -iC_OR, iC_S_FF, 0, "m0x0_1", {&iC_l_[208]}, {&iC_l_[210]}, &m0x0 };
iC_Gt m0x0_2   = { 1, -iC_AND, iC_R_FF, 0, "m0x0_2", {&iC_l_[213]}, {&iC_l_[215]}, &m0x0_1 };
iC_Gt m0x1     = { 1, -iC_FF, iC_GATE, 0, "m0x1", {0}, {0}, &m0x0_2 };
iC_Gt m0x1_1   = { 1, -iC_AND, iC_S_FF, 0, "m0x1_1", {&iC_l_[219]}, {&iC_l_[221]}, &m0x1 };
iC_Gt m0x1_2   = { 1, -iC_AND, iC_R_FF, 0, "m0x1_2", {&iC_l_[225]}, {&iC_l_[227]}, &m0x1_1 };
iC_Gt m0x2     = { 1, -iC_FF, iC_GATE, 0, "m0x2", {0}, {0}, &m0x1_2 };
iC_Gt m0x2_1   = { 1, -iC_AND, iC_S_FF, 0, "m0x2_1", {&iC_l_[231]}, {&iC_l_[233]}, &m0x2 };
iC_Gt m0x2_2   = { 1, -iC_AND, iC_R_FF, 0, "m0x2_2", {&iC_l_[237]}, {&iC_l_[239]}, &m0x2_1 };
iC_Gt m0x3     = { 1, -iC_FF, iC_GATE, 0, "m0x3", {0}, {0}, &m0x2_2 };
iC_Gt m0x3_1   = { 1, -iC_AND, iC_S_FF, 0, "m0x3_1", {&iC_l_[243]}, {&iC_l_[245]}, &m0x3 };
iC_Gt m0x3_2   = { 1, -iC_AND, iC_R_FF, 0, "m0x3_2", {&iC_l_[249]}, {&iC_l_[251]}, &m0x3_1 };
iC_Gt m0x4     = { 1, -iC_FF, iC_GATE, 0, "m0x4", {0}, {0}, &m0x3_2 };
iC_Gt m0x4_1   = { 1, -iC_AND, iC_S_FF, 0, "m0x4_1", {&iC_l_[255]}, {&iC_l_[257]}, &m0x4 };
iC_Gt m0x4_2   = { 1, -iC_AND, iC_R_FF, 0, "m0x4_2", {&iC_l_[261]}, {&iC_l_[263]}, &m0x4_1 };
iC_Gt m0x5     = { 1, -iC_FF, iC_GATE, 0, "m0x5", {0}, {0}, &m0x4_2 };
iC_Gt m0x5_1   = { 1, -iC_AND, iC_S_FF, 0, "m0x5_1", {&iC_l_[267]}, {&iC_l_[269]}, &m0x5 };
iC_Gt m0x5_2   = { 1, -iC_AND, iC_R_FF, 0, "m0x5_2", {&iC_l_[273]}, {&iC_l_[275]}, &m0x5_1 };
iC_Gt m0x6     = { 1, -iC_FF, iC_GATE, 0, "m0x6", {0}, {0}, &m0x5_2 };
iC_Gt m0x6_1   = { 1, -iC_AND, iC_S_FF, 0, "m0x6_1", {&iC_l_[279]}, {&iC_l_[281]}, &m0x6 };
iC_Gt m0x6_2   = { 1, -iC_AND, iC_R_FF, 0, "m0x6_2", {&iC_l_[285]}, {&iC_l_[287]}, &m0x6_1 };
iC_Gt m0x7     = { 1, -iC_FF, iC_GATE, 0, "m0x7", {0}, {0}, &m0x6_2 };
iC_Gt m0x7_1   = { 1, -iC_AND, iC_S_FF, 0, "m0x7_1", {&iC_l_[291]}, {&iC_l_[293]}, &m0x7 };
iC_Gt m0x7_2   = { 1, -iC_OR, iC_R_FF, 0, "m0x7_2", {&iC_l_[297]}, {&iC_l_[299]}, &m0x7_1 };
iC_Gt m0x8     = { 1, -iC_FF, iC_GATE, 0, "m0x8", {0}, {0}, &m0x7_2 };
iC_Gt m0x8_1   = { 1, -iC_AND, iC_S_FF, 0, "m0x8_1", {&iC_l_[302]}, {&iC_l_[304]}, &m0x8 };
iC_Gt m0x8_2   = { 1, -iC_AND, iC_R_FF, 0, "m0x8_2", {&iC_l_[308]}, {&iC_l_[310]}, &m0x8_1 };
iC_Gt m1x0     = { 1, -iC_FF, iC_GATE, 0, "m1x0", {0}, {0}, &m0x8_2 };
iC_Gt m1x0_1   = { 1, -iC_OR, iC_S_FF, 0, "m1x0_1", {&iC_l_[314]}, {&iC_l_[316]}, &m1x0 };
iC_Gt m1x0_2   = { 1, -iC_AND, iC_R_FF, 0, "m1x0_2", {&iC_l_[319]}, {&iC_l_[321]}, &m1x0_1 };
iC_Gt m1x1     = { 1, -iC_FF, iC_GATE, 0, "m1x1", {0}, {0}, &m1x0_2 };
iC_Gt m1x1_1   = { 1, -iC_AND, iC_S_FF, 0, "m1x1_1", {&iC_l_[325]}, {&iC_l_[327]}, &m1x1 };
iC_Gt m1x1_2   = { 1, -iC_AND, iC_R_FF, 0, "m1x1_2", {&iC_l_[331]}, {&iC_l_[333]}, &m1x1_1 };
iC_Gt m1x2     = { 1, -iC_FF, iC_GATE, 0, "m1x2", {0}, {0}, &m1x1_2 };
iC_Gt m1x2_1   = { 1, -iC_AND, iC_S_FF, 0, "m1x2_1", {&iC_l_[337]}, {&iC_l_[339]}, &m1x2 };
iC_Gt m1x2_2   = { 1, -iC_AND, iC_R_FF, 0, "m1x2_2", {&iC_l_[343]}, {&iC_l_[345]}, &m1x2_1 };
iC_Gt m1x3     = { 1, -iC_FF, iC_GATE, 0, "m1x3", {0}, {0}, &m1x2_2 };
iC_Gt m1x3_1   = { 1, -iC_AND, iC_S_FF, 0, "m1x3_1", {&iC_l_[349]}, {&iC_l_[351]}, &m1x3 };
iC_Gt m1x3_2   = { 1, -iC_AND, iC_R_FF, 0, "m1x3_2", {&iC_l_[355]}, {&iC_l_[357]}, &m1x3_1 };
iC_Gt m1x4     = { 1, -iC_FF, iC_GATE, 0, "m1x4", {0}, {0}, &m1x3_2 };
iC_Gt m1x4_1   = { 1, -iC_AND, iC_S_FF, 0, "m1x4_1", {&iC_l_[361]}, {&iC_l_[363]}, &m1x4 };
iC_Gt m1x4_2   = { 1, -iC_AND, iC_R_FF, 0, "m1x4_2", {&iC_l_[367]}, {&iC_l_[369]}, &m1x4_1 };
iC_Gt m1x5     = { 1, -iC_FF, iC_GATE, 0, "m1x5", {0}, {0}, &m1x4_2 };
iC_Gt m1x5_1   = { 1, -iC_AND, iC_S_FF, 0, "m1x5_1", {&iC_l_[373]}, {&iC_l_[375]}, &m1x5 };
iC_Gt m1x5_2   = { 1, -iC_AND, iC_R_FF, 0, "m1x5_2", {&iC_l_[379]}, {&iC_l_[381]}, &m1x5_1 };
iC_Gt m1x6     = { 1, -iC_FF, iC_GATE, 0, "m1x6", {0}, {0}, &m1x5_2 };
iC_Gt m1x6_1   = { 1, -iC_AND, iC_S_FF, 0, "m1x6_1", {&iC_l_[385]}, {&iC_l_[387]}, &m1x6 };
iC_Gt m1x6_2   = { 1, -iC_AND, iC_R_FF, 0, "m1x6_2", {&iC_l_[391]}, {&iC_l_[393]}, &m1x6_1 };
iC_Gt m1x7     = { 1, -iC_FF, iC_GATE, 0, "m1x7", {0}, {0}, &m1x6_2 };
iC_Gt m1x7_1   = { 1, -iC_AND, iC_S_FF, 0, "m1x7_1", {&iC_l_[397]}, {&iC_l_[399]}, &m1x7 };
iC_Gt m1x7_2   = { 1, -iC_OR, iC_R_FF, 0, "m1x7_2", {&iC_l_[403]}, {&iC_l_[405]}, &m1x7_1 };
iC_Gt m1x8     = { 1, -iC_FF, iC_GATE, 0, "m1x8", {0}, {0}, &m1x7_2 };
iC_Gt m1x8_1   = { 1, -iC_AND, iC_S_FF, 0, "m1x8_1", {&iC_l_[408]}, {&iC_l_[410]}, &m1x8 };
iC_Gt m1x8_2   = { 1, -iC_AND, iC_R_FF, 0, "m1x8_2", {&iC_l_[414]}, {&iC_l_[416]}, &m1x8_1 };
iC_Gt m2x0     = { 1, -iC_FF, iC_GATE, 0, "m2x0", {0}, {0}, &m1x8_2 };
iC_Gt m2x0_1   = { 1, -iC_OR, iC_S_FF, 0, "m2x0_1", {&iC_l_[420]}, {&iC_l_[422]}, &m2x0 };
iC_Gt m2x0_2   = { 1, -iC_AND, iC_R_FF, 0, "m2x0_2", {&iC_l_[425]}, {&iC_l_[427]}, &m2x0_1 };
iC_Gt m2x1     = { 1, -iC_FF, iC_GATE, 0, "m2x1", {0}, {0}, &m2x0_2 };
iC_Gt m2x1_1   = { 1, -iC_AND, iC_S_FF, 0, "m2x1_1", {&iC_l_[431]}, {&iC_l_[433]}, &m2x1 };
iC_Gt m2x1_2   = { 1, -iC_AND, iC_R_FF, 0, "m2x1_2", {&iC_l_[437]}, {&iC_l_[439]}, &m2x1_1 };
iC_Gt m2x2     = { 1, -iC_FF, iC_GATE, 0, "m2x2", {0}, {0}, &m2x1_2 };
iC_Gt m2x2_1   = { 1, -iC_AND, iC_S_FF, 0, "m2x2_1", {&iC_l_[443]}, {&iC_l_[445]}, &m2x2 };
iC_Gt m2x2_2   = { 1, -iC_AND, iC_R_FF, 0, "m2x2_2", {&iC_l_[449]}, {&iC_l_[451]}, &m2x2_1 };
iC_Gt m2x3     = { 1, -iC_FF, iC_GATE, 0, "m2x3", {0}, {0}, &m2x2_2 };
iC_Gt m2x3_1   = { 1, -iC_AND, iC_S_FF, 0, "m2x3_1", {&iC_l_[455]}, {&iC_l_[457]}, &m2x3 };
iC_Gt m2x3_2   = { 1, -iC_AND, iC_R_FF, 0, "m2x3_2", {&iC_l_[461]}, {&iC_l_[463]}, &m2x3_1 };
iC_Gt m2x4     = { 1, -iC_FF, iC_GATE, 0, "m2x4", {0}, {0}, &m2x3_2 };
iC_Gt m2x4_1   = { 1, -iC_AND, iC_S_FF, 0, "m2x4_1", {&iC_l_[467]}, {&iC_l_[469]}, &m2x4 };
iC_Gt m2x4_2   = { 1, -iC_AND, iC_R_FF, 0, "m2x4_2", {&iC_l_[473]}, {&iC_l_[475]}, &m2x4_1 };
iC_Gt m2x5     = { 1, -iC_FF, iC_GATE, 0, "m2x5", {0}, {0}, &m2x4_2 };
iC_Gt m2x5_1   = { 1, -iC_AND, iC_S_FF, 0, "m2x5_1", {&iC_l_[479]}, {&iC_l_[481]}, &m2x5 };
iC_Gt m2x5_2   = { 1, -iC_AND, iC_R_FF, 0, "m2x5_2", {&iC_l_[485]}, {&iC_l_[487]}, &m2x5_1 };
iC_Gt m2x6     = { 1, -iC_FF, iC_GATE, 0, "m2x6", {0}, {0}, &m2x5_2 };
iC_Gt m2x6_1   = { 1, -iC_AND, iC_S_FF, 0, "m2x6_1", {&iC_l_[491]}, {&iC_l_[493]}, &m2x6 };
iC_Gt m2x6_2   = { 1, -iC_AND, iC_R_FF, 0, "m2x6_2", {&iC_l_[497]}, {&iC_l_[499]}, &m2x6_1 };
iC_Gt m2x7     = { 1, -iC_FF, iC_GATE, 0, "m2x7", {0}, {0}, &m2x6_2 };
iC_Gt m2x7_1   = { 1, -iC_AND, iC_S_FF, 0, "m2x7_1", {&iC_l_[503]}, {&iC_l_[505]}, &m2x7 };
iC_Gt m2x7_2   = { 1, -iC_OR, iC_R_FF, 0, "m2x7_2", {&iC_l_[509]}, {&iC_l_[511]}, &m2x7_1 };
iC_Gt m2x8     = { 1, -iC_FF, iC_GATE, 0, "m2x8", {0}, {0}, &m2x7_2 };
iC_Gt m2x8_1   = { 1, -iC_AND, iC_S_FF, 0, "m2x8_1", {&iC_l_[514]}, {&iC_l_[516]}, &m2x8 };
iC_Gt m2x8_2   = { 1, -iC_AND, iC_R_FF, 0, "m2x8_2", {&iC_l_[520]}, {&iC_l_[522]}, &m2x8_1 };
iC_Gt m3x0     = { 1, -iC_FF, iC_GATE, 0, "m3x0", {0}, {0}, &m2x8_2 };
iC_Gt m3x0_1   = { 1, -iC_OR, iC_S_FF, 0, "m3x0_1", {&iC_l_[526]}, {&iC_l_[528]}, &m3x0 };
iC_Gt m3x0_2   = { 1, -iC_AND, iC_R_FF, 0, "m3x0_2", {&iC_l_[531]}, {&iC_l_[533]}, &m3x0_1 };
iC_Gt m3x1     = { 1, -iC_FF, iC_GATE, 0, "m3x1", {0}, {0}, &m3x0_2 };
iC_Gt m3x1_1   = { 1, -iC_AND, iC_S_FF, 0, "m3x1_1", {&iC_l_[537]}, {&iC_l_[539]}, &m3x1 };
iC_Gt m3x1_2   = { 1, -iC_AND, iC_R_FF, 0, "m3x1_2", {&iC_l_[543]}, {&iC_l_[545]}, &m3x1_1 };
iC_Gt m3x2     = { 1, -iC_FF, iC_GATE, 0, "m3x2", {0}, {0}, &m3x1_2 };
iC_Gt m3x2_1   = { 1, -iC_AND, iC_S_FF, 0, "m3x2_1", {&iC_l_[549]}, {&iC_l_[551]}, &m3x2 };
iC_Gt m3x2_2   = { 1, -iC_AND, iC_R_FF, 0, "m3x2_2", {&iC_l_[555]}, {&iC_l_[557]}, &m3x2_1 };
iC_Gt m3x3     = { 1, -iC_FF, iC_GATE, 0, "m3x3", {0}, {0}, &m3x2_2 };
iC_Gt m3x3_1   = { 1, -iC_AND, iC_S_FF, 0, "m3x3_1", {&iC_l_[561]}, {&iC_l_[563]}, &m3x3 };
iC_Gt m3x3_2   = { 1, -iC_AND, iC_R_FF, 0, "m3x3_2", {&iC_l_[567]}, {&iC_l_[569]}, &m3x3_1 };
iC_Gt m3x4     = { 1, -iC_FF, iC_GATE, 0, "m3x4", {0}, {0}, &m3x3_2 };
iC_Gt m3x4_1   = { 1, -iC_AND, iC_S_FF, 0, "m3x4_1", {&iC_l_[573]}, {&iC_l_[575]}, &m3x4 };
iC_Gt m3x4_2   = { 1, -iC_AND, iC_R_FF, 0, "m3x4_2", {&iC_l_[579]}, {&iC_l_[581]}, &m3x4_1 };
iC_Gt m3x5     = { 1, -iC_FF, iC_GATE, 0, "m3x5", {0}, {0}, &m3x4_2 };
iC_Gt m3x5_1   = { 1, -iC_AND, iC_S_FF, 0, "m3x5_1", {&iC_l_[585]}, {&iC_l_[587]}, &m3x5 };
iC_Gt m3x5_2   = { 1, -iC_AND, iC_R_FF, 0, "m3x5_2", {&iC_l_[591]}, {&iC_l_[593]}, &m3x5_1 };
iC_Gt m3x6     = { 1, -iC_FF, iC_GATE, 0, "m3x6", {0}, {0}, &m3x5_2 };
iC_Gt m3x6_1   = { 1, -iC_AND, iC_S_FF, 0, "m3x6_1", {&iC_l_[597]}, {&iC_l_[599]}, &m3x6 };
iC_Gt m3x6_2   = { 1, -iC_AND, iC_R_FF, 0, "m3x6_2", {&iC_l_[603]}, {&iC_l_[605]}, &m3x6_1 };
iC_Gt m3x7     = { 1, -iC_FF, iC_GATE, 0, "m3x7", {0}, {0}, &m3x6_2 };
iC_Gt m3x7_1   = { 1, -iC_AND, iC_S_FF, 0, "m3x7_1", {&iC_l_[609]}, {&iC_l_[611]}, &m3x7 };
iC_Gt m3x7_2   = { 1, -iC_OR, iC_R_FF, 0, "m3x7_2", {&iC_l_[615]}, {&iC_l_[617]}, &m3x7_1 };
iC_Gt m3x8     = { 1, -iC_FF, iC_GATE, 0, "m3x8", {0}, {0}, &m3x7_2 };
iC_Gt m3x8_1   = { 1, -iC_AND, iC_S_FF, 0, "m3x8_1", {&iC_l_[620]}, {&iC_l_[622]}, &m3x8 };
iC_Gt m3x8_2   = { 1, -iC_AND, iC_R_FF, 0, "m3x8_2", {&iC_l_[626]}, {&iC_l_[628]}, &m3x8_1 };
iC_Gt t        = { 1, -iC_TIM, iC_TIMRL, 0, "t", {0}, {0}, &m3x8_2 };
iC_Gt t_1      = { 1, -iC_OR, iC_TIMR, 0, "t_1", {&iC_l_[632]}, {&iC_l_[634]}, &t };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&m0x0}, &t_1, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&m0x1}, &QX0_0, 0 };
iC_Gt QX0_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.2", {0}, {(iC_Gt**)&m0x2}, &QX0_1, 0 };
iC_Gt QX0_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.3", {0}, {(iC_Gt**)&m0x3}, &QX0_2, 0 };
iC_Gt QX0_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.4", {0}, {(iC_Gt**)&m0x4}, &QX0_3, 0 };
iC_Gt QX0_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.5", {0}, {(iC_Gt**)&m0x5}, &QX0_4, 0 };
iC_Gt QX0_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.6", {0}, {(iC_Gt**)&m0x6}, &QX0_5, 0 };
iC_Gt QX0_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.7", {0}, {(iC_Gt**)&m0x7}, &QX0_6, 0 };
iC_Gt QX1_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.0", {0}, {(iC_Gt**)&m0x8}, &QX0_7, 0 };
iC_Gt QX2_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.0", {0}, {(iC_Gt**)&m1x0}, &QX1_0, 0 };
iC_Gt QX2_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.1", {0}, {(iC_Gt**)&m1x1}, &QX2_0, 0 };
iC_Gt QX2_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.2", {0}, {(iC_Gt**)&m1x2}, &QX2_1, 0 };
iC_Gt QX2_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.3", {0}, {(iC_Gt**)&m1x3}, &QX2_2, 0 };
iC_Gt QX2_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.4", {0}, {(iC_Gt**)&m1x4}, &QX2_3, 0 };
iC_Gt QX2_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.5", {0}, {(iC_Gt**)&m1x5}, &QX2_4, 0 };
iC_Gt QX2_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.6", {0}, {(iC_Gt**)&m1x6}, &QX2_5, 0 };
iC_Gt QX2_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.7", {0}, {(iC_Gt**)&m1x7}, &QX2_6, 0 };
iC_Gt QX3_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.0", {0}, {(iC_Gt**)&m1x8}, &QX2_7, 0 };
iC_Gt QX4_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.0", {0}, {(iC_Gt**)&m2x0}, &QX3_0, 0 };
iC_Gt QX4_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.1", {0}, {(iC_Gt**)&m2x1}, &QX4_0, 0 };
iC_Gt QX4_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.2", {0}, {(iC_Gt**)&m2x2}, &QX4_1, 0 };
iC_Gt QX4_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.3", {0}, {(iC_Gt**)&m2x3}, &QX4_2, 0 };
iC_Gt QX4_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.4", {0}, {(iC_Gt**)&m2x4}, &QX4_3, 0 };
iC_Gt QX4_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.5", {0}, {(iC_Gt**)&m2x5}, &QX4_4, 0 };
iC_Gt QX4_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.6", {0}, {(iC_Gt**)&m2x6}, &QX4_5, 0 };
iC_Gt QX4_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.7", {0}, {(iC_Gt**)&m2x7}, &QX4_6, 0 };
iC_Gt QX5_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX5.0", {0}, {(iC_Gt**)&m2x8}, &QX4_7, 0 };
iC_Gt QX6_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.0", {0}, {(iC_Gt**)&m3x0}, &QX5_0, 0 };
iC_Gt QX6_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.1", {0}, {(iC_Gt**)&m3x1}, &QX6_0, 0 };
iC_Gt QX6_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.2", {0}, {(iC_Gt**)&m3x2}, &QX6_1, 0 };
iC_Gt QX6_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.3", {0}, {(iC_Gt**)&m3x3}, &QX6_2, 0 };
iC_Gt QX6_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.4", {0}, {(iC_Gt**)&m3x4}, &QX6_3, 0 };
iC_Gt QX6_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.5", {0}, {(iC_Gt**)&m3x5}, &QX6_4, 0 };
iC_Gt QX6_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.6", {0}, {(iC_Gt**)&m3x6}, &QX6_5, 0 };
iC_Gt QX6_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.7", {0}, {(iC_Gt**)&m3x7}, &QX6_6, 0 };
iC_Gt QX7_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX7.0", {0}, {(iC_Gt**)&m3x8}, &QX6_7, 0 };
iC_Gt d0       = { 1, -iC_ALIAS, iC_ARITH, 0, "d0", {0}, {(iC_Gt**)&IB1}, &QX7_0, 0 };
iC_Gt d1       = { 1, -iC_ALIAS, iC_ARITH, 0, "d1", {0}, {(iC_Gt**)&IB3}, &d0, 0 };
iC_Gt d2       = { 1, -iC_ALIAS, iC_ARITH, 0, "d2", {0}, {(iC_Gt**)&IB5}, &d1, 0 };
iC_Gt d3       = { 1, -iC_ALIAS, iC_ARITH, 0, "d3", {0}, {(iC_Gt**)&IB7}, &d2, 0 };

iC_Gt *		iC___Test0_barc_list = &d3;
iC_Gt **	iC_list[] = { &iC___Test0_barc_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QX0.0_0 */	&m0x0, 0, 0,
/* QX0.1_0 */	&m0x1, 0, 0,
/* QX0.2_0 */	&m0x2, 0, 0,
/* QX0.3_0 */	&m0x3, 0, 0,
/* QX0.4_0 */	&m0x4, 0, 0,
/* QX0.5_0 */	&m0x5, 0, 0,
/* QX0.6_0 */	&m0x6, 0, 0,
/* QX0.7_0 */	&m0x7, 0, 0,
/* QX1.0_0 */	&m0x8, 0, 0,
/* QX2.0_0 */	&m1x0, 0, 0,
/* QX2.1_0 */	&m1x1, 0, 0,
/* QX2.2_0 */	&m1x2, 0, 0,
/* QX2.3_0 */	&m1x3, 0, 0,
/* QX2.4_0 */	&m1x4, 0, 0,
/* QX2.5_0 */	&m1x5, 0, 0,
/* QX2.6_0 */	&m1x6, 0, 0,
/* QX2.7_0 */	&m1x7, 0, 0,
/* QX3.0_0 */	&m1x8, 0, 0,
/* QX4.0_0 */	&m2x0, 0, 0,
/* QX4.1_0 */	&m2x1, 0, 0,
/* QX4.2_0 */	&m2x2, 0, 0,
/* QX4.3_0 */	&m2x3, 0, 0,
/* QX4.4_0 */	&m2x4, 0, 0,
/* QX4.5_0 */	&m2x5, 0, 0,
/* QX4.6_0 */	&m2x6, 0, 0,
/* QX4.7_0 */	&m2x7, 0, 0,
/* QX5.0_0 */	&m2x8, 0, 0,
/* QX6.0_0 */	&m3x0, 0, 0,
/* QX6.1_0 */	&m3x1, 0, 0,
/* QX6.2_0 */	&m3x2, 0, 0,
/* QX6.3_0 */	&m3x3, 0, 0,
/* QX6.4_0 */	&m3x4, 0, 0,
/* QX6.5_0 */	&m3x5, 0, 0,
/* QX6.6_0 */	&m3x6, 0, 0,
/* QX6.7_0 */	&m3x7, 0, 0,
/* QX7.0_0 */	&m3x8, 0, 0,
/* b0 */	&b0_2, &b0_1, 0, 0,
/* b0_3 */	&b0_2, &iClock,	&IX0_0, 0, &f0, 0,
/* b0_4 */	&b0_1, &t, &IB1,	&f0, 0, &b0, 0,
/* b1 */	&b1_2, &b1_1, 0, 0,
/* b1_3 */	&b1_2, &iClock,	&IX0_0, 0, &f1, 0,
/* b1_4 */	&b1_1, &t, &IB3,	&f1, 0, &b1, 0,
/* b2 */	&b2_2, &b2_1, 0, 0,
/* b2_3 */	&b2_2, &iClock,	&IX0_0, 0, &f2, 0,
/* b2_4 */	&b2_1, &t, &IB5,	&f2, 0, &b2, 0,
/* b3 */	&b3_2, &b3_1, 0, 0,
/* b3_3 */	&b3_2, &iClock,	&IX0_0, 0, &f3, 0,
/* b3_4 */	&b3_1, &t, &IB7,	&f3, 0, &b3, 0,
/* c0_1 */	&c0, &iClock,	&b0, 0, 0,
/* c1_1 */	&c1, &iClock,	&b1, 0, 0,
/* c2_1 */	&c2, &iClock,	&b2, 0, 0,
/* c3_1 */	&c3, &iClock,	&b3, 0, 0,
/* f0 */	(iC_Gt*)0, &IB1, 0,
/* f1 */	(iC_Gt*)0, &IB3, 0,
/* f2 */	(iC_Gt*)0, &IB5, 0,
/* f3 */	(iC_Gt*)0, &IB7, 0,
/* m0x0_1 */	&m0x0, &c0,	0, &m0x8, 0,
/* m0x0_2 */	&m0x0, &c0,	&m0x8, 0, &m0x1, 0,
/* m0x1_1 */	&m0x1, &c0,	&m0x0, 0, &m0x8, 0,
/* m0x1_2 */	&m0x1, &c0,	&m0x8, 0, &m0x2, 0,
/* m0x2_1 */	&m0x2, &c0,	&m0x1, 0, &m0x8, 0,
/* m0x2_2 */	&m0x2, &c0,	&m0x8, 0, &m0x3, 0,
/* m0x3_1 */	&m0x3, &c0,	&m0x2, 0, &m0x8, 0,
/* m0x3_2 */	&m0x3, &c0,	&m0x8, 0, &m0x4, 0,
/* m0x4_1 */	&m0x4, &c0,	&m0x3, 0, &m0x8, 0,
/* m0x4_2 */	&m0x4, &c0,	&m0x8, 0, &m0x5, 0,
/* m0x5_1 */	&m0x5, &c0,	&m0x4, 0, &m0x8, 0,
/* m0x5_2 */	&m0x5, &c0,	&m0x8, 0, &m0x6, 0,
/* m0x6_1 */	&m0x6, &c0,	&m0x5, 0, &m0x8, 0,
/* m0x6_2 */	&m0x6, &c0,	&m0x8, 0, &m0x7, 0,
/* m0x7_1 */	&m0x7, &c0,	&m0x6, 0, &m0x8, 0,
/* m0x7_2 */	&m0x7, &c0,	&m0x8, 0, 0,
/* m0x8_1 */	&m0x8, &c0,	&m0x7, 0, &m0x8, 0,
/* m0x8_2 */	&m0x8, &c0,	&m0x8, 0, &m0x0, 0,
/* m1x0_1 */	&m1x0, &c1,	0, &m1x8, 0,
/* m1x0_2 */	&m1x0, &c1,	&m1x8, 0, &m1x1, 0,
/* m1x1_1 */	&m1x1, &c1,	&m1x0, 0, &m1x8, 0,
/* m1x1_2 */	&m1x1, &c1,	&m1x8, 0, &m1x2, 0,
/* m1x2_1 */	&m1x2, &c1,	&m1x1, 0, &m1x8, 0,
/* m1x2_2 */	&m1x2, &c1,	&m1x8, 0, &m1x3, 0,
/* m1x3_1 */	&m1x3, &c1,	&m1x2, 0, &m1x8, 0,
/* m1x3_2 */	&m1x3, &c1,	&m1x8, 0, &m1x4, 0,
/* m1x4_1 */	&m1x4, &c1,	&m1x3, 0, &m1x8, 0,
/* m1x4_2 */	&m1x4, &c1,	&m1x8, 0, &m1x5, 0,
/* m1x5_1 */	&m1x5, &c1,	&m1x4, 0, &m1x8, 0,
/* m1x5_2 */	&m1x5, &c1,	&m1x8, 0, &m1x6, 0,
/* m1x6_1 */	&m1x6, &c1,	&m1x5, 0, &m1x8, 0,
/* m1x6_2 */	&m1x6, &c1,	&m1x8, 0, &m1x7, 0,
/* m1x7_1 */	&m1x7, &c1,	&m1x6, 0, &m1x8, 0,
/* m1x7_2 */	&m1x7, &c1,	&m1x8, 0, 0,
/* m1x8_1 */	&m1x8, &c1,	&m1x7, 0, &m1x8, 0,
/* m1x8_2 */	&m1x8, &c1,	&m1x8, 0, &m1x0, 0,
/* m2x0_1 */	&m2x0, &c2,	0, &m2x8, 0,
/* m2x0_2 */	&m2x0, &c2,	&m2x8, 0, &m2x1, 0,
/* m2x1_1 */	&m2x1, &c2,	&m2x0, 0, &m2x8, 0,
/* m2x1_2 */	&m2x1, &c2,	&m2x8, 0, &m2x2, 0,
/* m2x2_1 */	&m2x2, &c2,	&m2x1, 0, &m2x8, 0,
/* m2x2_2 */	&m2x2, &c2,	&m2x8, 0, &m2x3, 0,
/* m2x3_1 */	&m2x3, &c2,	&m2x2, 0, &m2x8, 0,
/* m2x3_2 */	&m2x3, &c2,	&m2x8, 0, &m2x4, 0,
/* m2x4_1 */	&m2x4, &c2,	&m2x3, 0, &m2x8, 0,
/* m2x4_2 */	&m2x4, &c2,	&m2x8, 0, &m2x5, 0,
/* m2x5_1 */	&m2x5, &c2,	&m2x4, 0, &m2x8, 0,
/* m2x5_2 */	&m2x5, &c2,	&m2x8, 0, &m2x6, 0,
/* m2x6_1 */	&m2x6, &c2,	&m2x5, 0, &m2x8, 0,
/* m2x6_2 */	&m2x6, &c2,	&m2x8, 0, &m2x7, 0,
/* m2x7_1 */	&m2x7, &c2,	&m2x6, 0, &m2x8, 0,
/* m2x7_2 */	&m2x7, &c2,	&m2x8, 0, 0,
/* m2x8_1 */	&m2x8, &c2,	&m2x7, 0, &m2x8, 0,
/* m2x8_2 */	&m2x8, &c2,	&m2x8, 0, &m2x0, 0,
/* m3x0_1 */	&m3x0, &c3,	0, &m3x8, 0,
/* m3x0_2 */	&m3x0, &c3,	&m3x8, 0, &m3x1, 0,
/* m3x1_1 */	&m3x1, &c3,	&m3x0, 0, &m3x8, 0,
/* m3x1_2 */	&m3x1, &c3,	&m3x8, 0, &m3x2, 0,
/* m3x2_1 */	&m3x2, &c3,	&m3x1, 0, &m3x8, 0,
/* m3x2_2 */	&m3x2, &c3,	&m3x8, 0, &m3x3, 0,
/* m3x3_1 */	&m3x3, &c3,	&m3x2, 0, &m3x8, 0,
/* m3x3_2 */	&m3x3, &c3,	&m3x8, 0, &m3x4, 0,
/* m3x4_1 */	&m3x4, &c3,	&m3x3, 0, &m3x8, 0,
/* m3x4_2 */	&m3x4, &c3,	&m3x8, 0, &m3x5, 0,
/* m3x5_1 */	&m3x5, &c3,	&m3x4, 0, &m3x8, 0,
/* m3x5_2 */	&m3x5, &c3,	&m3x8, 0, &m3x6, 0,
/* m3x6_1 */	&m3x6, &c3,	&m3x5, 0, &m3x8, 0,
/* m3x6_2 */	&m3x6, &c3,	&m3x8, 0, &m3x7, 0,
/* m3x7_1 */	&m3x7, &c3,	&m3x6, 0, &m3x8, 0,
/* m3x7_2 */	&m3x7, &c3,	&m3x8, 0, 0,
/* m3x8_1 */	&m3x8, &c3,	&m3x7, 0, &m3x8, 0,
/* m3x8_2 */	&m3x8, &c3,	&m3x8, 0, &m3x0, 0,
/* t_1 */	&t, &iClock,	&TX0_4, 0, 0,
};

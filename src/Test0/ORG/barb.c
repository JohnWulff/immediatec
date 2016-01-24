/********************************************************************
 *
 *	SOURCE:   ./Test0/barb.ic
 *	OUTPUT:   ./Test0/barb.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: barb.c,v 1.1 2015/05/24 00:51:49 jw Exp $ -O7";

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
iC_Gt m0       = { 1, -iC_FF, iC_GATE, 0, "m0", {0}, {0}, &f3 };
iC_Gt m0_1     = { 1, -iC_OR, iC_S_FF, 0, "m0_1", {&iC_l_[208]}, {&iC_l_[210]}, &m0 };
iC_Gt m0_2     = { 1, -iC_AND, iC_R_FF, 0, "m0_2", {&iC_l_[213]}, {&iC_l_[215]}, &m0_1 };
iC_Gt m1       = { 1, -iC_FF, iC_GATE, 0, "m1", {0}, {0}, &m0_2 };
iC_Gt m16      = { 1, -iC_FF, iC_GATE, 0, "m16", {0}, {0}, &m1 };
iC_Gt m16_1    = { 1, -iC_OR, iC_S_FF, 0, "m16_1", {&iC_l_[219]}, {&iC_l_[221]}, &m16 };
iC_Gt m16_2    = { 1, -iC_AND, iC_R_FF, 0, "m16_2", {&iC_l_[224]}, {&iC_l_[226]}, &m16_1 };
iC_Gt m17      = { 1, -iC_FF, iC_GATE, 0, "m17", {0}, {0}, &m16_2 };
iC_Gt m17_1    = { 1, -iC_AND, iC_S_FF, 0, "m17_1", {&iC_l_[230]}, {&iC_l_[232]}, &m17 };
iC_Gt m17_2    = { 1, -iC_AND, iC_R_FF, 0, "m17_2", {&iC_l_[236]}, {&iC_l_[238]}, &m17_1 };
iC_Gt m18      = { 1, -iC_FF, iC_GATE, 0, "m18", {0}, {0}, &m17_2 };
iC_Gt m18_1    = { 1, -iC_AND, iC_S_FF, 0, "m18_1", {&iC_l_[242]}, {&iC_l_[244]}, &m18 };
iC_Gt m18_2    = { 1, -iC_AND, iC_R_FF, 0, "m18_2", {&iC_l_[248]}, {&iC_l_[250]}, &m18_1 };
iC_Gt m19      = { 1, -iC_FF, iC_GATE, 0, "m19", {0}, {0}, &m18_2 };
iC_Gt m19_1    = { 1, -iC_AND, iC_S_FF, 0, "m19_1", {&iC_l_[254]}, {&iC_l_[256]}, &m19 };
iC_Gt m19_2    = { 1, -iC_AND, iC_R_FF, 0, "m19_2", {&iC_l_[260]}, {&iC_l_[262]}, &m19_1 };
iC_Gt m1_1     = { 1, -iC_AND, iC_S_FF, 0, "m1_1", {&iC_l_[266]}, {&iC_l_[268]}, &m19_2 };
iC_Gt m1_2     = { 1, -iC_AND, iC_R_FF, 0, "m1_2", {&iC_l_[272]}, {&iC_l_[274]}, &m1_1 };
iC_Gt m2       = { 1, -iC_FF, iC_GATE, 0, "m2", {0}, {0}, &m1_2 };
iC_Gt m20      = { 1, -iC_FF, iC_GATE, 0, "m20", {0}, {0}, &m2 };
iC_Gt m20_1    = { 1, -iC_AND, iC_S_FF, 0, "m20_1", {&iC_l_[278]}, {&iC_l_[280]}, &m20 };
iC_Gt m20_2    = { 1, -iC_AND, iC_R_FF, 0, "m20_2", {&iC_l_[284]}, {&iC_l_[286]}, &m20_1 };
iC_Gt m21      = { 1, -iC_FF, iC_GATE, 0, "m21", {0}, {0}, &m20_2 };
iC_Gt m21_1    = { 1, -iC_AND, iC_S_FF, 0, "m21_1", {&iC_l_[290]}, {&iC_l_[292]}, &m21 };
iC_Gt m21_2    = { 1, -iC_AND, iC_R_FF, 0, "m21_2", {&iC_l_[296]}, {&iC_l_[298]}, &m21_1 };
iC_Gt m22      = { 1, -iC_FF, iC_GATE, 0, "m22", {0}, {0}, &m21_2 };
iC_Gt m22_1    = { 1, -iC_AND, iC_S_FF, 0, "m22_1", {&iC_l_[302]}, {&iC_l_[304]}, &m22 };
iC_Gt m22_2    = { 1, -iC_AND, iC_R_FF, 0, "m22_2", {&iC_l_[308]}, {&iC_l_[310]}, &m22_1 };
iC_Gt m23      = { 1, -iC_FF, iC_GATE, 0, "m23", {0}, {0}, &m22_2 };
iC_Gt m23_1    = { 1, -iC_AND, iC_S_FF, 0, "m23_1", {&iC_l_[314]}, {&iC_l_[316]}, &m23 };
iC_Gt m23_2    = { 1, -iC_OR, iC_R_FF, 0, "m23_2", {&iC_l_[320]}, {&iC_l_[322]}, &m23_1 };
iC_Gt m24      = { 1, -iC_FF, iC_GATE, 0, "m24", {0}, {0}, &m23_2 };
iC_Gt m24_1    = { 1, -iC_AND, iC_S_FF, 0, "m24_1", {&iC_l_[325]}, {&iC_l_[327]}, &m24 };
iC_Gt m24_2    = { 1, -iC_AND, iC_R_FF, 0, "m24_2", {&iC_l_[331]}, {&iC_l_[333]}, &m24_1 };
iC_Gt m2_1     = { 1, -iC_AND, iC_S_FF, 0, "m2_1", {&iC_l_[337]}, {&iC_l_[339]}, &m24_2 };
iC_Gt m2_2     = { 1, -iC_AND, iC_R_FF, 0, "m2_2", {&iC_l_[343]}, {&iC_l_[345]}, &m2_1 };
iC_Gt m3       = { 1, -iC_FF, iC_GATE, 0, "m3", {0}, {0}, &m2_2 };
iC_Gt m32      = { 1, -iC_FF, iC_GATE, 0, "m32", {0}, {0}, &m3 };
iC_Gt m32_1    = { 1, -iC_OR, iC_S_FF, 0, "m32_1", {&iC_l_[349]}, {&iC_l_[351]}, &m32 };
iC_Gt m32_2    = { 1, -iC_AND, iC_R_FF, 0, "m32_2", {&iC_l_[354]}, {&iC_l_[356]}, &m32_1 };
iC_Gt m33      = { 1, -iC_FF, iC_GATE, 0, "m33", {0}, {0}, &m32_2 };
iC_Gt m33_1    = { 1, -iC_AND, iC_S_FF, 0, "m33_1", {&iC_l_[360]}, {&iC_l_[362]}, &m33 };
iC_Gt m33_2    = { 1, -iC_AND, iC_R_FF, 0, "m33_2", {&iC_l_[366]}, {&iC_l_[368]}, &m33_1 };
iC_Gt m34      = { 1, -iC_FF, iC_GATE, 0, "m34", {0}, {0}, &m33_2 };
iC_Gt m34_1    = { 1, -iC_AND, iC_S_FF, 0, "m34_1", {&iC_l_[372]}, {&iC_l_[374]}, &m34 };
iC_Gt m34_2    = { 1, -iC_AND, iC_R_FF, 0, "m34_2", {&iC_l_[378]}, {&iC_l_[380]}, &m34_1 };
iC_Gt m35      = { 1, -iC_FF, iC_GATE, 0, "m35", {0}, {0}, &m34_2 };
iC_Gt m35_1    = { 1, -iC_AND, iC_S_FF, 0, "m35_1", {&iC_l_[384]}, {&iC_l_[386]}, &m35 };
iC_Gt m35_2    = { 1, -iC_AND, iC_R_FF, 0, "m35_2", {&iC_l_[390]}, {&iC_l_[392]}, &m35_1 };
iC_Gt m36      = { 1, -iC_FF, iC_GATE, 0, "m36", {0}, {0}, &m35_2 };
iC_Gt m36_1    = { 1, -iC_AND, iC_S_FF, 0, "m36_1", {&iC_l_[396]}, {&iC_l_[398]}, &m36 };
iC_Gt m36_2    = { 1, -iC_AND, iC_R_FF, 0, "m36_2", {&iC_l_[402]}, {&iC_l_[404]}, &m36_1 };
iC_Gt m37      = { 1, -iC_FF, iC_GATE, 0, "m37", {0}, {0}, &m36_2 };
iC_Gt m37_1    = { 1, -iC_AND, iC_S_FF, 0, "m37_1", {&iC_l_[408]}, {&iC_l_[410]}, &m37 };
iC_Gt m37_2    = { 1, -iC_AND, iC_R_FF, 0, "m37_2", {&iC_l_[414]}, {&iC_l_[416]}, &m37_1 };
iC_Gt m38      = { 1, -iC_FF, iC_GATE, 0, "m38", {0}, {0}, &m37_2 };
iC_Gt m38_1    = { 1, -iC_AND, iC_S_FF, 0, "m38_1", {&iC_l_[420]}, {&iC_l_[422]}, &m38 };
iC_Gt m38_2    = { 1, -iC_AND, iC_R_FF, 0, "m38_2", {&iC_l_[426]}, {&iC_l_[428]}, &m38_1 };
iC_Gt m39      = { 1, -iC_FF, iC_GATE, 0, "m39", {0}, {0}, &m38_2 };
iC_Gt m39_1    = { 1, -iC_AND, iC_S_FF, 0, "m39_1", {&iC_l_[432]}, {&iC_l_[434]}, &m39 };
iC_Gt m39_2    = { 1, -iC_OR, iC_R_FF, 0, "m39_2", {&iC_l_[438]}, {&iC_l_[440]}, &m39_1 };
iC_Gt m3_1     = { 1, -iC_AND, iC_S_FF, 0, "m3_1", {&iC_l_[443]}, {&iC_l_[445]}, &m39_2 };
iC_Gt m3_2     = { 1, -iC_AND, iC_R_FF, 0, "m3_2", {&iC_l_[449]}, {&iC_l_[451]}, &m3_1 };
iC_Gt m4       = { 1, -iC_FF, iC_GATE, 0, "m4", {0}, {0}, &m3_2 };
iC_Gt m40      = { 1, -iC_FF, iC_GATE, 0, "m40", {0}, {0}, &m4 };
iC_Gt m40_1    = { 1, -iC_AND, iC_S_FF, 0, "m40_1", {&iC_l_[455]}, {&iC_l_[457]}, &m40 };
iC_Gt m40_2    = { 1, -iC_AND, iC_R_FF, 0, "m40_2", {&iC_l_[461]}, {&iC_l_[463]}, &m40_1 };
iC_Gt m48      = { 1, -iC_FF, iC_GATE, 0, "m48", {0}, {0}, &m40_2 };
iC_Gt m48_1    = { 1, -iC_OR, iC_S_FF, 0, "m48_1", {&iC_l_[467]}, {&iC_l_[469]}, &m48 };
iC_Gt m48_2    = { 1, -iC_AND, iC_R_FF, 0, "m48_2", {&iC_l_[472]}, {&iC_l_[474]}, &m48_1 };
iC_Gt m49      = { 1, -iC_FF, iC_GATE, 0, "m49", {0}, {0}, &m48_2 };
iC_Gt m49_1    = { 1, -iC_AND, iC_S_FF, 0, "m49_1", {&iC_l_[478]}, {&iC_l_[480]}, &m49 };
iC_Gt m49_2    = { 1, -iC_AND, iC_R_FF, 0, "m49_2", {&iC_l_[484]}, {&iC_l_[486]}, &m49_1 };
iC_Gt m4_1     = { 1, -iC_AND, iC_S_FF, 0, "m4_1", {&iC_l_[490]}, {&iC_l_[492]}, &m49_2 };
iC_Gt m4_2     = { 1, -iC_AND, iC_R_FF, 0, "m4_2", {&iC_l_[496]}, {&iC_l_[498]}, &m4_1 };
iC_Gt m5       = { 1, -iC_FF, iC_GATE, 0, "m5", {0}, {0}, &m4_2 };
iC_Gt m50      = { 1, -iC_FF, iC_GATE, 0, "m50", {0}, {0}, &m5 };
iC_Gt m50_1    = { 1, -iC_AND, iC_S_FF, 0, "m50_1", {&iC_l_[502]}, {&iC_l_[504]}, &m50 };
iC_Gt m50_2    = { 1, -iC_AND, iC_R_FF, 0, "m50_2", {&iC_l_[508]}, {&iC_l_[510]}, &m50_1 };
iC_Gt m51      = { 1, -iC_FF, iC_GATE, 0, "m51", {0}, {0}, &m50_2 };
iC_Gt m51_1    = { 1, -iC_AND, iC_S_FF, 0, "m51_1", {&iC_l_[514]}, {&iC_l_[516]}, &m51 };
iC_Gt m51_2    = { 1, -iC_AND, iC_R_FF, 0, "m51_2", {&iC_l_[520]}, {&iC_l_[522]}, &m51_1 };
iC_Gt m52      = { 1, -iC_FF, iC_GATE, 0, "m52", {0}, {0}, &m51_2 };
iC_Gt m52_1    = { 1, -iC_AND, iC_S_FF, 0, "m52_1", {&iC_l_[526]}, {&iC_l_[528]}, &m52 };
iC_Gt m52_2    = { 1, -iC_AND, iC_R_FF, 0, "m52_2", {&iC_l_[532]}, {&iC_l_[534]}, &m52_1 };
iC_Gt m53      = { 1, -iC_FF, iC_GATE, 0, "m53", {0}, {0}, &m52_2 };
iC_Gt m53_1    = { 1, -iC_AND, iC_S_FF, 0, "m53_1", {&iC_l_[538]}, {&iC_l_[540]}, &m53 };
iC_Gt m53_2    = { 1, -iC_AND, iC_R_FF, 0, "m53_2", {&iC_l_[544]}, {&iC_l_[546]}, &m53_1 };
iC_Gt m54      = { 1, -iC_FF, iC_GATE, 0, "m54", {0}, {0}, &m53_2 };
iC_Gt m54_1    = { 1, -iC_AND, iC_S_FF, 0, "m54_1", {&iC_l_[550]}, {&iC_l_[552]}, &m54 };
iC_Gt m54_2    = { 1, -iC_AND, iC_R_FF, 0, "m54_2", {&iC_l_[556]}, {&iC_l_[558]}, &m54_1 };
iC_Gt m55      = { 1, -iC_FF, iC_GATE, 0, "m55", {0}, {0}, &m54_2 };
iC_Gt m55_1    = { 1, -iC_AND, iC_S_FF, 0, "m55_1", {&iC_l_[562]}, {&iC_l_[564]}, &m55 };
iC_Gt m55_2    = { 1, -iC_OR, iC_R_FF, 0, "m55_2", {&iC_l_[568]}, {&iC_l_[570]}, &m55_1 };
iC_Gt m56      = { 1, -iC_FF, iC_GATE, 0, "m56", {0}, {0}, &m55_2 };
iC_Gt m56_1    = { 1, -iC_AND, iC_S_FF, 0, "m56_1", {&iC_l_[573]}, {&iC_l_[575]}, &m56 };
iC_Gt m56_2    = { 1, -iC_AND, iC_R_FF, 0, "m56_2", {&iC_l_[579]}, {&iC_l_[581]}, &m56_1 };
iC_Gt m5_1     = { 1, -iC_AND, iC_S_FF, 0, "m5_1", {&iC_l_[585]}, {&iC_l_[587]}, &m56_2 };
iC_Gt m5_2     = { 1, -iC_AND, iC_R_FF, 0, "m5_2", {&iC_l_[591]}, {&iC_l_[593]}, &m5_1 };
iC_Gt m6       = { 1, -iC_FF, iC_GATE, 0, "m6", {0}, {0}, &m5_2 };
iC_Gt m6_1     = { 1, -iC_AND, iC_S_FF, 0, "m6_1", {&iC_l_[597]}, {&iC_l_[599]}, &m6 };
iC_Gt m6_2     = { 1, -iC_AND, iC_R_FF, 0, "m6_2", {&iC_l_[603]}, {&iC_l_[605]}, &m6_1 };
iC_Gt m7       = { 1, -iC_FF, iC_GATE, 0, "m7", {0}, {0}, &m6_2 };
iC_Gt m7_1     = { 1, -iC_AND, iC_S_FF, 0, "m7_1", {&iC_l_[609]}, {&iC_l_[611]}, &m7 };
iC_Gt m7_2     = { 1, -iC_OR, iC_R_FF, 0, "m7_2", {&iC_l_[615]}, {&iC_l_[617]}, &m7_1 };
iC_Gt m8       = { 1, -iC_FF, iC_GATE, 0, "m8", {0}, {0}, &m7_2 };
iC_Gt m8_1     = { 1, -iC_AND, iC_S_FF, 0, "m8_1", {&iC_l_[620]}, {&iC_l_[622]}, &m8 };
iC_Gt m8_2     = { 1, -iC_AND, iC_R_FF, 0, "m8_2", {&iC_l_[626]}, {&iC_l_[628]}, &m8_1 };
iC_Gt t        = { 1, -iC_TIM, iC_TIMRL, 0, "t", {0}, {0}, &m8_2 };
iC_Gt t_1      = { 1, -iC_OR, iC_TIMR, 0, "t_1", {&iC_l_[632]}, {&iC_l_[634]}, &t };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&m0}, &t_1, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&m1}, &QX0_0, 0 };
iC_Gt QX0_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.2", {0}, {(iC_Gt**)&m2}, &QX0_1, 0 };
iC_Gt QX0_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.3", {0}, {(iC_Gt**)&m3}, &QX0_2, 0 };
iC_Gt QX0_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.4", {0}, {(iC_Gt**)&m4}, &QX0_3, 0 };
iC_Gt QX0_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.5", {0}, {(iC_Gt**)&m5}, &QX0_4, 0 };
iC_Gt QX0_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.6", {0}, {(iC_Gt**)&m6}, &QX0_5, 0 };
iC_Gt QX0_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.7", {0}, {(iC_Gt**)&m7}, &QX0_6, 0 };
iC_Gt QX1_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.0", {0}, {(iC_Gt**)&m8}, &QX0_7, 0 };
iC_Gt QX2_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.0", {0}, {(iC_Gt**)&m16}, &QX1_0, 0 };
iC_Gt QX2_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.1", {0}, {(iC_Gt**)&m17}, &QX2_0, 0 };
iC_Gt QX2_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.2", {0}, {(iC_Gt**)&m18}, &QX2_1, 0 };
iC_Gt QX2_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.3", {0}, {(iC_Gt**)&m19}, &QX2_2, 0 };
iC_Gt QX2_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.4", {0}, {(iC_Gt**)&m20}, &QX2_3, 0 };
iC_Gt QX2_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.5", {0}, {(iC_Gt**)&m21}, &QX2_4, 0 };
iC_Gt QX2_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.6", {0}, {(iC_Gt**)&m22}, &QX2_5, 0 };
iC_Gt QX2_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.7", {0}, {(iC_Gt**)&m23}, &QX2_6, 0 };
iC_Gt QX3_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.0", {0}, {(iC_Gt**)&m24}, &QX2_7, 0 };
iC_Gt QX4_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.0", {0}, {(iC_Gt**)&m32}, &QX3_0, 0 };
iC_Gt QX4_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.1", {0}, {(iC_Gt**)&m33}, &QX4_0, 0 };
iC_Gt QX4_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.2", {0}, {(iC_Gt**)&m34}, &QX4_1, 0 };
iC_Gt QX4_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.3", {0}, {(iC_Gt**)&m35}, &QX4_2, 0 };
iC_Gt QX4_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.4", {0}, {(iC_Gt**)&m36}, &QX4_3, 0 };
iC_Gt QX4_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.5", {0}, {(iC_Gt**)&m37}, &QX4_4, 0 };
iC_Gt QX4_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.6", {0}, {(iC_Gt**)&m38}, &QX4_5, 0 };
iC_Gt QX4_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.7", {0}, {(iC_Gt**)&m39}, &QX4_6, 0 };
iC_Gt QX5_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX5.0", {0}, {(iC_Gt**)&m40}, &QX4_7, 0 };
iC_Gt QX6_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.0", {0}, {(iC_Gt**)&m48}, &QX5_0, 0 };
iC_Gt QX6_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.1", {0}, {(iC_Gt**)&m49}, &QX6_0, 0 };
iC_Gt QX6_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.2", {0}, {(iC_Gt**)&m50}, &QX6_1, 0 };
iC_Gt QX6_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.3", {0}, {(iC_Gt**)&m51}, &QX6_2, 0 };
iC_Gt QX6_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.4", {0}, {(iC_Gt**)&m52}, &QX6_3, 0 };
iC_Gt QX6_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.5", {0}, {(iC_Gt**)&m53}, &QX6_4, 0 };
iC_Gt QX6_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.6", {0}, {(iC_Gt**)&m54}, &QX6_5, 0 };
iC_Gt QX6_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.7", {0}, {(iC_Gt**)&m55}, &QX6_6, 0 };
iC_Gt QX7_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX7.0", {0}, {(iC_Gt**)&m56}, &QX6_7, 0 };
iC_Gt d0       = { 1, -iC_ALIAS, iC_ARITH, 0, "d0", {0}, {(iC_Gt**)&IB1}, &QX7_0, 0 };
iC_Gt d1       = { 1, -iC_ALIAS, iC_ARITH, 0, "d1", {0}, {(iC_Gt**)&IB3}, &d0, 0 };
iC_Gt d2       = { 1, -iC_ALIAS, iC_ARITH, 0, "d2", {0}, {(iC_Gt**)&IB5}, &d1, 0 };
iC_Gt d3       = { 1, -iC_ALIAS, iC_ARITH, 0, "d3", {0}, {(iC_Gt**)&IB7}, &d2, 0 };

iC_Gt *		iC___Test0_barb_list = &d3;
iC_Gt **	iC_list[] = { &iC___Test0_barb_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QX0.0_0 */	&m0, 0, 0,
/* QX0.1_0 */	&m1, 0, 0,
/* QX0.2_0 */	&m2, 0, 0,
/* QX0.3_0 */	&m3, 0, 0,
/* QX0.4_0 */	&m4, 0, 0,
/* QX0.5_0 */	&m5, 0, 0,
/* QX0.6_0 */	&m6, 0, 0,
/* QX0.7_0 */	&m7, 0, 0,
/* QX1.0_0 */	&m8, 0, 0,
/* QX2.0_0 */	&m16, 0, 0,
/* QX2.1_0 */	&m17, 0, 0,
/* QX2.2_0 */	&m18, 0, 0,
/* QX2.3_0 */	&m19, 0, 0,
/* QX2.4_0 */	&m20, 0, 0,
/* QX2.5_0 */	&m21, 0, 0,
/* QX2.6_0 */	&m22, 0, 0,
/* QX2.7_0 */	&m23, 0, 0,
/* QX3.0_0 */	&m24, 0, 0,
/* QX4.0_0 */	&m32, 0, 0,
/* QX4.1_0 */	&m33, 0, 0,
/* QX4.2_0 */	&m34, 0, 0,
/* QX4.3_0 */	&m35, 0, 0,
/* QX4.4_0 */	&m36, 0, 0,
/* QX4.5_0 */	&m37, 0, 0,
/* QX4.6_0 */	&m38, 0, 0,
/* QX4.7_0 */	&m39, 0, 0,
/* QX5.0_0 */	&m40, 0, 0,
/* QX6.0_0 */	&m48, 0, 0,
/* QX6.1_0 */	&m49, 0, 0,
/* QX6.2_0 */	&m50, 0, 0,
/* QX6.3_0 */	&m51, 0, 0,
/* QX6.4_0 */	&m52, 0, 0,
/* QX6.5_0 */	&m53, 0, 0,
/* QX6.6_0 */	&m54, 0, 0,
/* QX6.7_0 */	&m55, 0, 0,
/* QX7.0_0 */	&m56, 0, 0,
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
/* m0_1 */	&m0, &c0,	0, &m8, 0,
/* m0_2 */	&m0, &c0,	&m8, 0, &m1, 0,
/* m16_1 */	&m16, &c1,	0, &m24, 0,
/* m16_2 */	&m16, &c1,	&m24, 0, &m17, 0,
/* m17_1 */	&m17, &c1,	&m16, 0, &m24, 0,
/* m17_2 */	&m17, &c1,	&m24, 0, &m18, 0,
/* m18_1 */	&m18, &c1,	&m17, 0, &m24, 0,
/* m18_2 */	&m18, &c1,	&m24, 0, &m19, 0,
/* m19_1 */	&m19, &c1,	&m18, 0, &m24, 0,
/* m19_2 */	&m19, &c1,	&m24, 0, &m20, 0,
/* m1_1 */	&m1, &c0,	&m0, 0, &m8, 0,
/* m1_2 */	&m1, &c0,	&m8, 0, &m2, 0,
/* m20_1 */	&m20, &c1,	&m19, 0, &m24, 0,
/* m20_2 */	&m20, &c1,	&m24, 0, &m21, 0,
/* m21_1 */	&m21, &c1,	&m20, 0, &m24, 0,
/* m21_2 */	&m21, &c1,	&m24, 0, &m22, 0,
/* m22_1 */	&m22, &c1,	&m21, 0, &m24, 0,
/* m22_2 */	&m22, &c1,	&m24, 0, &m23, 0,
/* m23_1 */	&m23, &c1,	&m22, 0, &m24, 0,
/* m23_2 */	&m23, &c1,	&m24, 0, 0,
/* m24_1 */	&m24, &c1,	&m23, 0, &m24, 0,
/* m24_2 */	&m24, &c1,	&m24, 0, &m16, 0,
/* m2_1 */	&m2, &c0,	&m1, 0, &m8, 0,
/* m2_2 */	&m2, &c0,	&m8, 0, &m3, 0,
/* m32_1 */	&m32, &c2,	0, &m40, 0,
/* m32_2 */	&m32, &c2,	&m40, 0, &m33, 0,
/* m33_1 */	&m33, &c2,	&m32, 0, &m40, 0,
/* m33_2 */	&m33, &c2,	&m40, 0, &m34, 0,
/* m34_1 */	&m34, &c2,	&m33, 0, &m40, 0,
/* m34_2 */	&m34, &c2,	&m40, 0, &m35, 0,
/* m35_1 */	&m35, &c2,	&m34, 0, &m40, 0,
/* m35_2 */	&m35, &c2,	&m40, 0, &m36, 0,
/* m36_1 */	&m36, &c2,	&m35, 0, &m40, 0,
/* m36_2 */	&m36, &c2,	&m40, 0, &m37, 0,
/* m37_1 */	&m37, &c2,	&m36, 0, &m40, 0,
/* m37_2 */	&m37, &c2,	&m40, 0, &m38, 0,
/* m38_1 */	&m38, &c2,	&m37, 0, &m40, 0,
/* m38_2 */	&m38, &c2,	&m40, 0, &m39, 0,
/* m39_1 */	&m39, &c2,	&m38, 0, &m40, 0,
/* m39_2 */	&m39, &c2,	&m40, 0, 0,
/* m3_1 */	&m3, &c0,	&m2, 0, &m8, 0,
/* m3_2 */	&m3, &c0,	&m8, 0, &m4, 0,
/* m40_1 */	&m40, &c2,	&m39, 0, &m40, 0,
/* m40_2 */	&m40, &c2,	&m40, 0, &m32, 0,
/* m48_1 */	&m48, &c3,	0, &m56, 0,
/* m48_2 */	&m48, &c3,	&m56, 0, &m49, 0,
/* m49_1 */	&m49, &c3,	&m48, 0, &m56, 0,
/* m49_2 */	&m49, &c3,	&m56, 0, &m50, 0,
/* m4_1 */	&m4, &c0,	&m3, 0, &m8, 0,
/* m4_2 */	&m4, &c0,	&m8, 0, &m5, 0,
/* m50_1 */	&m50, &c3,	&m49, 0, &m56, 0,
/* m50_2 */	&m50, &c3,	&m56, 0, &m51, 0,
/* m51_1 */	&m51, &c3,	&m50, 0, &m56, 0,
/* m51_2 */	&m51, &c3,	&m56, 0, &m52, 0,
/* m52_1 */	&m52, &c3,	&m51, 0, &m56, 0,
/* m52_2 */	&m52, &c3,	&m56, 0, &m53, 0,
/* m53_1 */	&m53, &c3,	&m52, 0, &m56, 0,
/* m53_2 */	&m53, &c3,	&m56, 0, &m54, 0,
/* m54_1 */	&m54, &c3,	&m53, 0, &m56, 0,
/* m54_2 */	&m54, &c3,	&m56, 0, &m55, 0,
/* m55_1 */	&m55, &c3,	&m54, 0, &m56, 0,
/* m55_2 */	&m55, &c3,	&m56, 0, 0,
/* m56_1 */	&m56, &c3,	&m55, 0, &m56, 0,
/* m56_2 */	&m56, &c3,	&m56, 0, &m48, 0,
/* m5_1 */	&m5, &c0,	&m4, 0, &m8, 0,
/* m5_2 */	&m5, &c0,	&m8, 0, &m6, 0,
/* m6_1 */	&m6, &c0,	&m5, 0, &m8, 0,
/* m6_2 */	&m6, &c0,	&m8, 0, &m7, 0,
/* m7_1 */	&m7, &c0,	&m6, 0, &m8, 0,
/* m7_2 */	&m7, &c0,	&m8, 0, 0,
/* m8_1 */	&m8, &c0,	&m7, 0, &m8, 0,
/* m8_2 */	&m8, &c0,	&m8, 0, &m0, 0,
/* t_1 */	&t, &iClock,	&TX0_4, 0, 0,
};

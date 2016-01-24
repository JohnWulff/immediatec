/********************************************************************
 *
 *	SOURCE:   ./Test0/sort5.ic
 *	OUTPUT:   ./Test0/sort5.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: sort5.c,v 1.1 2015/05/24 00:51:58 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

static iC_Gt _10     = { 1, -iC_NCONST, iC_ARITH, 0, "10", {0}, {0}, 0 };
static iC_Gt _5      = { 1, -iC_NCONST, iC_ARITH, 0, "5", {0}, {0}, &_10 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &_5 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
iC_Gt IX0_4    = { 1, -iC_INPX, iC_GATE, 0, "IX0.4", {0}, {0}, &IX0_3 };
iC_Gt IX0_6    = { 1, -iC_INPX, iC_GATE, 0, "IX0.6", {0}, {0}, &IX0_4 };
iC_Gt IX0_7    = { 1, -iC_INPX, iC_GATE, 0, "IX0.7", {0}, {0}, &IX0_6 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[0]}, &IX0_7 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[3]}, &QX0_0_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[6]}, &QX0_1_0 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[9]}, &QX0_2_0 };
iC_Gt QX0_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.4_0", {0}, {&iC_l_[12]}, &QX0_3_0 };
iC_Gt QX0_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.5_0", {0}, {&iC_l_[15]}, &QX0_4_0 };
iC_Gt QX0_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.6_0", {0}, {&iC_l_[18]}, &QX0_5_0 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[21]}, &QX0_6_0 };
iC_Gt QX1_0    = { 1, -iC_ARN, iC_GATE, 0, "QX1.0", {0}, {&iC_l_[24]}, &QX0_7_0 };
iC_Gt QX1_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.0_0", {0}, {&iC_l_[27]}, &QX1_0 };
iC_Gt QX1_1    = { 1, -iC_ARN, iC_GATE, 0, "QX1.1", {0}, {&iC_l_[30]}, &QX1_0_0 };
iC_Gt QX1_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.1_0", {0}, {&iC_l_[33]}, &QX1_1 };
iC_Gt QX1_2    = { 1, -iC_ARN, iC_GATE, 0, "QX1.2", {0}, {&iC_l_[36]}, &QX1_1_0 };
iC_Gt QX1_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.2_0", {0}, {&iC_l_[39]}, &QX1_2 };
iC_Gt QX1_3    = { 1, -iC_ARN, iC_GATE, 0, "QX1.3", {0}, {&iC_l_[42]}, &QX1_2_0 };
iC_Gt QX1_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.3_0", {0}, {&iC_l_[45]}, &QX1_3 };
iC_Gt QX1_4    = { 1, -iC_ARN, iC_GATE, 0, "QX1.4", {0}, {&iC_l_[48]}, &QX1_3_0 };
iC_Gt QX1_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.4_0", {0}, {&iC_l_[51]}, &QX1_4 };
iC_Gt QX1_5    = { 1, -iC_ARN, iC_GATE, 0, "QX1.5", {0}, {&iC_l_[54]}, &QX1_4_0 };
iC_Gt QX1_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.5_0", {0}, {&iC_l_[57]}, &QX1_5 };
iC_Gt QX1_6    = { 1, -iC_ARN, iC_GATE, 0, "QX1.6", {0}, {&iC_l_[60]}, &QX1_5_0 };
iC_Gt QX1_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.6_0", {0}, {&iC_l_[63]}, &QX1_6 };
iC_Gt QX1_7    = { 1, -iC_ARN, iC_GATE, 0, "QX1.7", {0}, {&iC_l_[66]}, &QX1_6_0 };
iC_Gt QX1_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.7_0", {0}, {&iC_l_[69]}, &QX1_7 };
iC_Gt QX2_0    = { 1, -iC_ARN, iC_GATE, 0, "QX2.0", {0}, {&iC_l_[72]}, &QX1_7_0 };
iC_Gt QX2_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.0_0", {0}, {&iC_l_[75]}, &QX2_0 };
iC_Gt QX2_1    = { 1, -iC_ARN, iC_GATE, 0, "QX2.1", {0}, {&iC_l_[78]}, &QX2_0_0 };
iC_Gt QX2_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.1_0", {0}, {&iC_l_[81]}, &QX2_1 };
iC_Gt QX2_2    = { 1, -iC_ARN, iC_GATE, 0, "QX2.2", {0}, {&iC_l_[84]}, &QX2_1_0 };
iC_Gt QX2_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.2_0", {0}, {&iC_l_[87]}, &QX2_2 };
iC_Gt QX2_3    = { 1, -iC_ARN, iC_GATE, 0, "QX2.3", {0}, {&iC_l_[90]}, &QX2_2_0 };
iC_Gt QX2_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.3_0", {0}, {&iC_l_[93]}, &QX2_3 };
iC_Gt QX2_4    = { 1, -iC_ARN, iC_GATE, 0, "QX2.4", {0}, {&iC_l_[96]}, &QX2_3_0 };
iC_Gt QX2_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.4_0", {0}, {&iC_l_[99]}, &QX2_4 };
iC_Gt QX2_5    = { 1, -iC_ARN, iC_GATE, 0, "QX2.5", {0}, {&iC_l_[102]}, &QX2_4_0 };
iC_Gt QX2_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.5_0", {0}, {&iC_l_[105]}, &QX2_5 };
iC_Gt QX2_6    = { 1, -iC_ARN, iC_GATE, 0, "QX2.6", {0}, {&iC_l_[108]}, &QX2_5_0 };
iC_Gt QX2_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.6_0", {0}, {&iC_l_[111]}, &QX2_6 };
iC_Gt QX2_7    = { 1, -iC_ARN, iC_GATE, 0, "QX2.7", {0}, {&iC_l_[114]}, &QX2_6_0 };
iC_Gt QX2_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.7_0", {0}, {&iC_l_[117]}, &QX2_7 };
iC_Gt QX3_0    = { 1, -iC_ARN, iC_GATE, 0, "QX3.0", {0}, {&iC_l_[120]}, &QX2_7_0 };
iC_Gt QX3_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.0_0", {0}, {&iC_l_[123]}, &QX3_0 };
iC_Gt QX3_1    = { 1, -iC_ARN, iC_GATE, 0, "QX3.1", {0}, {&iC_l_[126]}, &QX3_0_0 };
iC_Gt QX3_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.1_0", {0}, {&iC_l_[129]}, &QX3_1 };
iC_Gt QX3_2    = { 1, -iC_ARN, iC_GATE, 0, "QX3.2", {0}, {&iC_l_[132]}, &QX3_1_0 };
iC_Gt QX3_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.2_0", {0}, {&iC_l_[135]}, &QX3_2 };
iC_Gt QX3_3    = { 1, -iC_ARN, iC_GATE, 0, "QX3.3", {0}, {&iC_l_[138]}, &QX3_2_0 };
iC_Gt QX3_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.3_0", {0}, {&iC_l_[141]}, &QX3_3 };
iC_Gt QX3_4    = { 1, -iC_ARN, iC_GATE, 0, "QX3.4", {0}, {&iC_l_[144]}, &QX3_3_0 };
iC_Gt QX3_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.4_0", {0}, {&iC_l_[147]}, &QX3_4 };
iC_Gt QX3_5    = { 1, -iC_ARN, iC_GATE, 0, "QX3.5", {0}, {&iC_l_[150]}, &QX3_4_0 };
iC_Gt QX3_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.5_0", {0}, {&iC_l_[153]}, &QX3_5 };
iC_Gt QX3_6    = { 1, -iC_ARN, iC_GATE, 0, "QX3.6", {0}, {&iC_l_[156]}, &QX3_5_0 };
iC_Gt QX3_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.6_0", {0}, {&iC_l_[159]}, &QX3_6 };
iC_Gt QX3_7    = { 1, -iC_ARN, iC_GATE, 0, "QX3.7", {0}, {&iC_l_[162]}, &QX3_6_0 };
iC_Gt QX3_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.7_0", {0}, {&iC_l_[165]}, &QX3_7 };
iC_Gt QX4_0    = { 1, -iC_ARN, iC_GATE, 0, "QX4.0", {0}, {&iC_l_[168]}, &QX3_7_0 };
iC_Gt QX4_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.0_0", {0}, {&iC_l_[171]}, &QX4_0 };
iC_Gt QX4_1    = { 1, -iC_ARN, iC_GATE, 0, "QX4.1", {0}, {&iC_l_[174]}, &QX4_0_0 };
iC_Gt QX4_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.1_0", {0}, {&iC_l_[177]}, &QX4_1 };
iC_Gt QX4_2    = { 1, -iC_ARN, iC_GATE, 0, "QX4.2", {0}, {&iC_l_[180]}, &QX4_1_0 };
iC_Gt QX4_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.2_0", {0}, {&iC_l_[183]}, &QX4_2 };
iC_Gt QX4_3    = { 1, -iC_ARN, iC_GATE, 0, "QX4.3", {0}, {&iC_l_[186]}, &QX4_2_0 };
iC_Gt QX4_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.3_0", {0}, {&iC_l_[189]}, &QX4_3 };
iC_Gt QX4_4    = { 1, -iC_ARN, iC_GATE, 0, "QX4.4", {0}, {&iC_l_[192]}, &QX4_3_0 };
iC_Gt QX4_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.4_0", {0}, {&iC_l_[195]}, &QX4_4 };
iC_Gt QX4_5    = { 1, -iC_ARN, iC_GATE, 0, "QX4.5", {0}, {&iC_l_[198]}, &QX4_4_0 };
iC_Gt QX4_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.5_0", {0}, {&iC_l_[201]}, &QX4_5 };
iC_Gt QX4_6    = { 1, -iC_ARN, iC_GATE, 0, "QX4.6", {0}, {&iC_l_[204]}, &QX4_5_0 };
iC_Gt QX4_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.6_0", {0}, {&iC_l_[207]}, &QX4_6 };
iC_Gt QX4_7    = { 1, -iC_ARN, iC_GATE, 0, "QX4.7", {0}, {&iC_l_[210]}, &QX4_6_0 };
iC_Gt QX4_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.7_0", {0}, {&iC_l_[213]}, &QX4_7 };
iC_Gt QX5_0    = { 1, -iC_ARN, iC_GATE, 0, "QX5.0", {0}, {&iC_l_[216]}, &QX4_7_0 };
iC_Gt QX5_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.0_0", {0}, {&iC_l_[219]}, &QX5_0 };
iC_Gt QX5_1    = { 1, -iC_ARN, iC_GATE, 0, "QX5.1", {0}, {&iC_l_[222]}, &QX5_0_0 };
iC_Gt QX5_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.1_0", {0}, {&iC_l_[225]}, &QX5_1 };
iC_Gt QX5_2    = { 1, -iC_ARN, iC_GATE, 0, "QX5.2", {0}, {&iC_l_[228]}, &QX5_1_0 };
iC_Gt QX5_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.2_0", {0}, {&iC_l_[231]}, &QX5_2 };
iC_Gt QX5_3    = { 1, -iC_ARN, iC_GATE, 0, "QX5.3", {0}, {&iC_l_[234]}, &QX5_2_0 };
iC_Gt QX5_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.3_0", {0}, {&iC_l_[237]}, &QX5_3 };
iC_Gt QX5_4    = { 1, -iC_ARN, iC_GATE, 0, "QX5.4", {0}, {&iC_l_[240]}, &QX5_3_0 };
iC_Gt QX5_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.4_0", {0}, {&iC_l_[243]}, &QX5_4 };
iC_Gt QX5_5    = { 1, -iC_ARN, iC_GATE, 0, "QX5.5", {0}, {&iC_l_[246]}, &QX5_4_0 };
iC_Gt QX5_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.5_0", {0}, {&iC_l_[249]}, &QX5_5 };
iC_Gt QX5_6    = { 1, -iC_ARN, iC_GATE, 0, "QX5.6", {0}, {&iC_l_[252]}, &QX5_5_0 };
iC_Gt QX5_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.6_0", {0}, {&iC_l_[255]}, &QX5_6 };
iC_Gt QX5_7    = { 1, -iC_ARN, iC_GATE, 0, "QX5.7", {0}, {&iC_l_[258]}, &QX5_6_0 };
iC_Gt QX5_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.7_0", {0}, {&iC_l_[261]}, &QX5_7 };
iC_Gt QX6_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.0_0", {0}, {&iC_l_[264]}, &QX5_7_0 };
iC_Gt QX6_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.1_0", {0}, {&iC_l_[267]}, &QX6_0_0 };
iC_Gt QX6_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.2_0", {0}, {&iC_l_[270]}, &QX6_1_0 };
iC_Gt QX6_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.3_0", {0}, {&iC_l_[273]}, &QX6_2_0 };
iC_Gt QX6_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.4_0", {0}, {&iC_l_[276]}, &QX6_3_0 };
iC_Gt QX6_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.5_0", {0}, {&iC_l_[279]}, &QX6_4_0 };
iC_Gt QX6_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.6_0", {0}, {&iC_l_[282]}, &QX6_5_0 };
iC_Gt QX6_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX6.7_0", {0}, {&iC_l_[285]}, &QX6_6_0 };
iC_Gt TX0_4    = { 1, -iC_INPX, iC_GATE, 0, "TX0.4", {0}, {0}, &QX6_7_0 };
iC_Gt TX0_5    = { 1, -iC_INPX, iC_GATE, 0, "TX0.5", {0}, {0}, &TX0_4 };
iC_Gt c        = { 1, -iC_CLK, iC_CLCKL, 0, "c", {0}, {0}, &TX0_5 };
iC_Gt c_1      = { 1, -iC_OR, iC_CLCK, 0, "c_1", {&iC_l_[288]}, {&iC_l_[290]}, &c };
iC_Gt c_2      = { 1, -iC_AND, iC_CLCK, 0, "c_2", {&iC_l_[293]}, {&iC_l_[295]}, &c_1 };
iC_Gt c_3      = { 1, -iC_VF, iC_GATE, 0, "c_3", {0}, {0}, &c_2 };
iC_Gt c_4      = { 1, -iC_AND, iC_CH_BIT, 0, "c_4", {&iC_l_[299]}, {&iC_l_[301]}, &c_3 };
iC_Gt in       = { 1, -iC_OR, iC_GATE, 0, "in", {0}, {&iC_l_[305]}, &c_4 };
iC_Gt in0      = { 1, -iC_OR, iC_GATE, 0, "in0", {0}, {&iC_l_[311]}, &in };
iC_Gt in0_1    = { 1, -iC_AND, iC_GATE, 0, "in0_1", {0}, {&iC_l_[315]}, &in0 };
iC_Gt in0_2    = { 1, -iC_AND, iC_GATE, 0, "in0_2", {0}, {&iC_l_[319]}, &in0_1 };
iC_Gt in0_3    = { 1, -iC_FF, iC_GATE, 0, "in0_3", {0}, {0}, &in0_2 };
iC_Gt in0_4    = { 1, -iC_OR, iC_S_FF, 0, "in0_4", {&iC_l_[323]}, {&iC_l_[325]}, &in0_3 };
iC_Gt in0_5    = { 1, -iC_OR, iC_R_FF, 0, "in0_5", {&iC_l_[328]}, {&iC_l_[331]}, &in0_4 };
iC_Gt in0_6    = { 1, -iC_VF, iC_GATE, 0, "in0_6", {0}, {0}, &in0_5 };
iC_Gt in0_7    = { 1, -iC_OR, iC_CH_BIT, 0, "in0_7", {&iC_l_[334]}, {&iC_l_[336]}, &in0_6 };
iC_Gt in1      = { 1, -iC_OR, iC_GATE, 0, "in1", {0}, {&iC_l_[339]}, &in0_7 };
iC_Gt in1_1    = { 1, -iC_AND, iC_GATE, 0, "in1_1", {0}, {&iC_l_[343]}, &in1 };
iC_Gt in1_2    = { 1, -iC_AND, iC_GATE, 0, "in1_2", {0}, {&iC_l_[347]}, &in1_1 };
iC_Gt in1_3    = { 1, -iC_FF, iC_GATE, 0, "in1_3", {0}, {0}, &in1_2 };
iC_Gt in1_4    = { 1, -iC_OR, iC_S_FF, 0, "in1_4", {&iC_l_[351]}, {&iC_l_[353]}, &in1_3 };
iC_Gt in1_5    = { 1, -iC_OR, iC_R_FF, 0, "in1_5", {&iC_l_[356]}, {&iC_l_[359]}, &in1_4 };
iC_Gt in1_6    = { 1, -iC_VF, iC_GATE, 0, "in1_6", {0}, {0}, &in1_5 };
iC_Gt in1_7    = { 1, -iC_OR, iC_CH_BIT, 0, "in1_7", {&iC_l_[362]}, {&iC_l_[364]}, &in1_6 };
iC_Gt in2      = { 1, -iC_OR, iC_GATE, 0, "in2", {0}, {&iC_l_[367]}, &in1_7 };
iC_Gt in2_1    = { 1, -iC_AND, iC_GATE, 0, "in2_1", {0}, {&iC_l_[371]}, &in2 };
iC_Gt in2_2    = { 1, -iC_AND, iC_GATE, 0, "in2_2", {0}, {&iC_l_[375]}, &in2_1 };
iC_Gt in2_3    = { 1, -iC_FF, iC_GATE, 0, "in2_3", {0}, {0}, &in2_2 };
iC_Gt in2_4    = { 1, -iC_OR, iC_S_FF, 0, "in2_4", {&iC_l_[379]}, {&iC_l_[381]}, &in2_3 };
iC_Gt in2_5    = { 1, -iC_OR, iC_R_FF, 0, "in2_5", {&iC_l_[384]}, {&iC_l_[387]}, &in2_4 };
iC_Gt in2_6    = { 1, -iC_VF, iC_GATE, 0, "in2_6", {0}, {0}, &in2_5 };
iC_Gt in2_7    = { 1, -iC_OR, iC_CH_BIT, 0, "in2_7", {&iC_l_[390]}, {&iC_l_[392]}, &in2_6 };
iC_Gt in3      = { 1, -iC_OR, iC_GATE, 0, "in3", {0}, {&iC_l_[395]}, &in2_7 };
iC_Gt in3_1    = { 1, -iC_AND, iC_GATE, 0, "in3_1", {0}, {&iC_l_[399]}, &in3 };
iC_Gt in3_2    = { 1, -iC_AND, iC_GATE, 0, "in3_2", {0}, {&iC_l_[403]}, &in3_1 };
iC_Gt in3_3    = { 1, -iC_FF, iC_GATE, 0, "in3_3", {0}, {0}, &in3_2 };
iC_Gt in3_4    = { 1, -iC_OR, iC_S_FF, 0, "in3_4", {&iC_l_[407]}, {&iC_l_[409]}, &in3_3 };
iC_Gt in3_5    = { 1, -iC_OR, iC_R_FF, 0, "in3_5", {&iC_l_[412]}, {&iC_l_[415]}, &in3_4 };
iC_Gt in3_6    = { 1, -iC_VF, iC_GATE, 0, "in3_6", {0}, {0}, &in3_5 };
iC_Gt in3_7    = { 1, -iC_OR, iC_CH_BIT, 0, "in3_7", {&iC_l_[418]}, {&iC_l_[420]}, &in3_6 };
iC_Gt lock1    = { 1, -iC_FF, iC_GATE, 0, "lock1", {0}, {0}, &in3_7 };
iC_Gt lock1_1  = { 1, -iC_OR, iC_S_FF, 0, "lock1_1", {&iC_l_[423]}, {&iC_l_[425]}, &lock1 };
iC_Gt lock1_2  = { 1, -iC_ARN, iC_R_FF, 0, "lock1_2", {&iC_l_[428]}, {&iC_l_[430]}, &lock1_1 };
iC_Gt lock2    = { 1, -iC_FF, iC_GATE, 0, "lock2", {0}, {0}, &lock1_2 };
iC_Gt lock2_1  = { 1, -iC_AND, iC_S_FF, 0, "lock2_1", {&iC_l_[433]}, {&iC_l_[435]}, &lock2 };
iC_Gt lock2_2  = { 1, -iC_OR, iC_R_FF, 0, "lock2_2", {&iC_l_[439]}, {&iC_l_[441]}, &lock2_1 };
iC_Gt m06      = { 1, -iC_FF, iC_GATE, 0, "m06", {0}, {0}, &lock2_2 };
iC_Gt m06_1    = { 1, -iC_OR, iC_D_FF, 0, "m06_1", {&iC_l_[444]}, {&iC_l_[446]}, &m06 };
iC_Gt m07      = { 1, -iC_FF, iC_GATE, 0, "m07", {0}, {0}, &m06_1 };
iC_Gt m07_1    = { 1, -iC_OR, iC_D_FF, 0, "m07_1", {&iC_l_[449]}, {&iC_l_[451]}, &m07 };
iC_Gt m07_2    = { 1, -iC_AND, iC_GATE, 0, "m07_2", {0}, {&iC_l_[455]}, &m07_1 };
iC_Gt m07_3    = { 1, -iC_OR, iC_GATE, 0, "m07_3", {0}, {&iC_l_[459]}, &m07_2 };
iC_Gt m08      = { 1, -iC_FF, iC_GATE, 0, "m08", {0}, {0}, &m07_3 };
iC_Gt m08_1    = { 1, -iC_OR, iC_D_FF, 0, "m08_1", {&iC_l_[463]}, {&iC_l_[465]}, &m08 };
iC_Gt m08_2    = { 1, -iC_AND, iC_GATE, 0, "m08_2", {0}, {&iC_l_[469]}, &m08_1 };
iC_Gt m08_3    = { 1, -iC_OR, iC_GATE, 0, "m08_3", {0}, {&iC_l_[473]}, &m08_2 };
iC_Gt m09      = { 1, -iC_FF, iC_GATE, 0, "m09", {0}, {0}, &m08_3 };
iC_Gt m09_1    = { 1, -iC_OR, iC_D_FF, 0, "m09_1", {&iC_l_[477]}, {&iC_l_[479]}, &m09 };
iC_Gt pip      = { 1, -iC_EF, iC_GATE, 0, "pip", {0}, {0}, &m09_1 };
iC_Gt pip_1    = { 1, -iC_AND, iC_RI_BIT, 0, "pip_1", {&iC_l_[483]}, {&iC_l_[485]}, &pip };
iC_Gt s10      = { 1, -iC_SH, iC_ARITH, 0, "s10", {0}, {0}, &pip_1 };
iC_Gt s10_1    = { 1, -iC_ARN, iC_D_SH, 0, "s10_1", {&iC_l_[489]}, {&iC_l_[491]}, &s10 };
iC_Gt s10_2    = { 1, -iC_OR, iC_ARITH, 0, "s10_2", {0}, {&iC_l_[495]}, &s10_1 };
iC_Gt s11      = { 1, -iC_SH, iC_ARITH, 0, "s11", {0}, {0}, &s10_2 };
iC_Gt s11_1    = { 1, -iC_ARN, iC_D_SH, 0, "s11_1", {&iC_l_[498]}, {&iC_l_[500]}, &s11 };
iC_Gt s12      = { 1, -iC_SH, iC_ARITH, 0, "s12", {0}, {0}, &s11_1 };
iC_Gt s12_1    = { 1, -iC_ARN, iC_D_SH, 0, "s12_1", {&iC_l_[504]}, {&iC_l_[506]}, &s12 };
iC_Gt t        = { 1, -iC_TIM, iC_TIMRL, 0, "t", {0}, {0}, &s12_1 };
iC_Gt t100     = { 1, -iC_TIM, iC_TIMRL, 0, "t100", {0}, {0}, &t };
iC_Gt t100_1   = { 1, -iC_OR, iC_TIMR, 0, "t100_1", {&iC_l_[510]}, {&iC_l_[512]}, &t100 };
iC_Gt t_1      = { 1, -iC_OR, iC_TIMR, 0, "t_1", {&iC_l_[515]}, {&iC_l_[517]}, &t100_1 };
iC_Gt t_2      = { 1, -iC_AND, iC_TIMR, 0, "t_2", {&iC_l_[520]}, {&iC_l_[522]}, &t_1 };
iC_Gt t_3      = { 1, -iC_VF, iC_GATE, 0, "t_3", {0}, {0}, &t_2 };
iC_Gt t_4      = { 1, -iC_AND, iC_CH_BIT, 0, "t_4", {&iC_l_[526]}, {&iC_l_[528]}, &t_3 };
iC_Gt tor0     = { 1, -iC_FF, iC_GATE, 0, "tor0", {0}, {0}, &t_4 };
iC_Gt tor0_1   = { 1, -iC_ARN, iC_S_FF, 0, "tor0_1", {&iC_l_[532]}, {&iC_l_[534]}, &tor0 };
iC_Gt tor0_2   = { 1, -iC_OR, iC_R_FF, 0, "tor0_2", {&iC_l_[537]}, {&iC_l_[540]}, &tor0_1 };
iC_Gt tor1     = { 1, -iC_FF, iC_GATE, 0, "tor1", {0}, {0}, &tor0_2 };
iC_Gt tor1_1   = { 1, -iC_ARN, iC_S_FF, 0, "tor1_1", {&iC_l_[543]}, {&iC_l_[545]}, &tor1 };
iC_Gt tor1_2   = { 1, -iC_OR, iC_R_FF, 0, "tor1_2", {&iC_l_[548]}, {&iC_l_[551]}, &tor1_1 };
iC_Gt tor2     = { 1, -iC_FF, iC_GATE, 0, "tor2", {0}, {0}, &tor1_2 };
iC_Gt tor2_1   = { 1, -iC_ARN, iC_S_FF, 0, "tor2_1", {&iC_l_[554]}, {&iC_l_[556]}, &tor2 };
iC_Gt tor2_2   = { 1, -iC_OR, iC_R_FF, 0, "tor2_2", {&iC_l_[559]}, {&iC_l_[562]}, &tor2_1 };
iC_Gt tor3     = { 1, -iC_FF, iC_GATE, 0, "tor3", {0}, {0}, &tor2_2 };
iC_Gt tor3_1   = { 1, -iC_ARN, iC_S_FF, 0, "tor3_1", {&iC_l_[565]}, {&iC_l_[567]}, &tor3 };
iC_Gt tor3_2   = { 1, -iC_OR, iC_R_FF, 0, "tor3_2", {&iC_l_[570]}, {&iC_l_[573]}, &tor3_1 };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&tor0}, &tor3_2, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&tor1}, &QX0_0, 0 };
iC_Gt QX0_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.2", {0}, {(iC_Gt**)&tor2}, &QX0_1, 0 };
iC_Gt QX0_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.3", {0}, {(iC_Gt**)&tor3}, &QX0_2, 0 };
iC_Gt QX0_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.4", {0}, {(iC_Gt**)&m06}, &QX0_3, 0 };
iC_Gt QX0_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.5", {0}, {(iC_Gt**)&m07}, &QX0_4, 0 };
iC_Gt QX0_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.6", {0}, {(iC_Gt**)&m08}, &QX0_5, 0 };
iC_Gt QX0_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.7", {0}, {(iC_Gt**)&m09}, &QX0_6, 0 };
iC_Gt QX6_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.0", {0}, {(iC_Gt**)&in0}, &QX0_7, 0 };
iC_Gt QX6_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.1", {0}, {(iC_Gt**)&in1}, &QX6_0, 0 };
iC_Gt QX6_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.2", {0}, {(iC_Gt**)&in2}, &QX6_1, 0 };
iC_Gt QX6_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.3", {0}, {(iC_Gt**)&in3}, &QX6_2, 0 };
iC_Gt QX6_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.4", {0}, {(iC_Gt**)&in}, &QX6_3, 0 };
iC_Gt QX6_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.5", {0}, {(iC_Gt**)&pip}, &QX6_4, 0 };
iC_Gt QX6_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.6", {0}, {(iC_Gt**)&lock1}, &QX6_5, 0 };
iC_Gt QX6_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX6.7", {0}, {(iC_Gt**)&lock2}, &QX6_6, 0 };
iC_Gt in4      = { 1, -iC_ALIAS, iC_GATE, 0, "in4", {0}, {(iC_Gt**)&IX0_4}, &QX6_7, 0 };

iC_Gt *		iC___Test0_sort5_list = &in4;
iC_Gt **	iC_list[] = { &iC___Test0_sort5_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_2(iC_Gt * iC_gf) {
#line 32 "./Test0/sort5.ic"
	return iC_MV(1)&1<<0;
#line 244 "./Test0/sort5.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
#line 32 "./Test0/sort5.ic"
	return iC_MV(1)&1<<8;
#line 250 "./Test0/sort5.c"
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
#line 33 "./Test0/sort5.ic"
	return iC_MV(1)&1<<1;
#line 256 "./Test0/sort5.c"
} /* iC_4 */

static int iC_5(iC_Gt * iC_gf) {
#line 33 "./Test0/sort5.ic"
	return iC_MV(1)&1<<9;
#line 262 "./Test0/sort5.c"
} /* iC_5 */

static int iC_6(iC_Gt * iC_gf) {
#line 34 "./Test0/sort5.ic"
	return iC_MV(1)&1<<2;
#line 268 "./Test0/sort5.c"
} /* iC_6 */

static int iC_7(iC_Gt * iC_gf) {
#line 34 "./Test0/sort5.ic"
	return iC_MV(1)&1<<10;
#line 274 "./Test0/sort5.c"
} /* iC_7 */

static int iC_8(iC_Gt * iC_gf) {
#line 35 "./Test0/sort5.ic"
	return iC_MV(1)&1<<3;
#line 280 "./Test0/sort5.c"
} /* iC_8 */

static int iC_9(iC_Gt * iC_gf) {
#line 35 "./Test0/sort5.ic"
	return iC_MV(1)&1<<11;
#line 286 "./Test0/sort5.c"
} /* iC_9 */

static int iC_10(iC_Gt * iC_gf) {
#line 36 "./Test0/sort5.ic"
	return iC_MV(1)&1<<4;
#line 292 "./Test0/sort5.c"
} /* iC_10 */

static int iC_11(iC_Gt * iC_gf) {
#line 36 "./Test0/sort5.ic"
	return iC_MV(1)&1<<12;
#line 298 "./Test0/sort5.c"
} /* iC_11 */

static int iC_12(iC_Gt * iC_gf) {
#line 37 "./Test0/sort5.ic"
	return iC_MV(1)&1<<5;
#line 304 "./Test0/sort5.c"
} /* iC_12 */

static int iC_13(iC_Gt * iC_gf) {
#line 37 "./Test0/sort5.ic"
	return iC_MV(1)&1<<13;
#line 310 "./Test0/sort5.c"
} /* iC_13 */

static int iC_14(iC_Gt * iC_gf) {
#line 38 "./Test0/sort5.ic"
	return iC_MV(1)&1<<6;
#line 316 "./Test0/sort5.c"
} /* iC_14 */

static int iC_15(iC_Gt * iC_gf) {
#line 38 "./Test0/sort5.ic"
	return iC_MV(1)&1<<14;
#line 322 "./Test0/sort5.c"
} /* iC_15 */

static int iC_16(iC_Gt * iC_gf) {
#line 39 "./Test0/sort5.ic"
	return iC_MV(1)&1<<7;
#line 328 "./Test0/sort5.c"
} /* iC_16 */

static int iC_17(iC_Gt * iC_gf) {
#line 39 "./Test0/sort5.ic"
	return iC_MV(1)&1<<15;
#line 334 "./Test0/sort5.c"
} /* iC_17 */

static int iC_18(iC_Gt * iC_gf) {
#line 57 "./Test0/sort5.ic"
	return (iC_MV(1)&0x9<<1)==0x9<<1;
#line 340 "./Test0/sort5.c"
} /* iC_18 */

static int iC_19(iC_Gt * iC_gf) {
#line 65 "./Test0/sort5.ic"
	return (iC_MV(1)<<1)+iC_MV(2);
#line 346 "./Test0/sort5.c"
} /* iC_19 */

static int iC_20(iC_Gt * iC_gf) {
#line 66 "./Test0/sort5.ic"
	return (iC_MV(1)<<1)+(iC_MV(2)>>15&1);
#line 352 "./Test0/sort5.c"
} /* iC_20 */

static int iC_21(iC_Gt * iC_gf) {
#line 69 "./Test0/sort5.ic"
	return (iC_MV(1)&0xf<<6)==0x9<<6;
#line 358 "./Test0/sort5.c"
} /* iC_21 */

static int iC_22(iC_Gt * iC_gf) {
#line 70 "./Test0/sort5.ic"
	return (iC_MV(1)&0xf<<0)==0xb<<0;
#line 364 "./Test0/sort5.c"
} /* iC_22 */

static int iC_23(iC_Gt * iC_gf) {
#line 71 "./Test0/sort5.ic"
	return (iC_MV(1)&0xf<<10)==0xd<<10;
#line 370 "./Test0/sort5.c"
} /* iC_23 */

static int iC_24(iC_Gt * iC_gf) {
#line 72 "./Test0/sort5.ic"
	return (iC_MV(1)&0xf<<4)==0xf<<4;
#line 376 "./Test0/sort5.c"
} /* iC_24 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QX0.0_0 */	&tor0, 0, 0,
/* QX0.1_0 */	&tor1, 0, 0,
/* QX0.2_0 */	&tor2, 0, 0,
/* QX0.3_0 */	&tor3, 0, 0,
/* QX0.4_0 */	&m06, 0, 0,
/* QX0.5_0 */	&m07, 0, 0,
/* QX0.6_0 */	&m08, 0, 0,
/* QX0.7_0 */	&m09, 0, 0,
/* QX1.0 */	(iC_Gt*)iC_2, &s10, 0,
/* QX1.0_0 */	&QX1_0, 0, 0,
/* QX1.1 */	(iC_Gt*)iC_4, &s10, 0,
/* QX1.1_0 */	&QX1_1, 0, 0,
/* QX1.2 */	(iC_Gt*)iC_6, &s10, 0,
/* QX1.2_0 */	&QX1_2, 0, 0,
/* QX1.3 */	(iC_Gt*)iC_8, &s10, 0,
/* QX1.3_0 */	&QX1_3, 0, 0,
/* QX1.4 */	(iC_Gt*)iC_10, &s10, 0,
/* QX1.4_0 */	&QX1_4, 0, 0,
/* QX1.5 */	(iC_Gt*)iC_12, &s10, 0,
/* QX1.5_0 */	&QX1_5, 0, 0,
/* QX1.6 */	(iC_Gt*)iC_14, &s10, 0,
/* QX1.6_0 */	&QX1_6, 0, 0,
/* QX1.7 */	(iC_Gt*)iC_16, &s10, 0,
/* QX1.7_0 */	&QX1_7, 0, 0,
/* QX2.0 */	(iC_Gt*)iC_3, &s10, 0,
/* QX2.0_0 */	&QX2_0, 0, 0,
/* QX2.1 */	(iC_Gt*)iC_5, &s10, 0,
/* QX2.1_0 */	&QX2_1, 0, 0,
/* QX2.2 */	(iC_Gt*)iC_7, &s10, 0,
/* QX2.2_0 */	&QX2_2, 0, 0,
/* QX2.3 */	(iC_Gt*)iC_9, &s10, 0,
/* QX2.3_0 */	&QX2_3, 0, 0,
/* QX2.4 */	(iC_Gt*)iC_11, &s10, 0,
/* QX2.4_0 */	&QX2_4, 0, 0,
/* QX2.5 */	(iC_Gt*)iC_13, &s10, 0,
/* QX2.5_0 */	&QX2_5, 0, 0,
/* QX2.6 */	(iC_Gt*)iC_15, &s10, 0,
/* QX2.6_0 */	&QX2_6, 0, 0,
/* QX2.7 */	(iC_Gt*)iC_17, &s10, 0,
/* QX2.7_0 */	&QX2_7, 0, 0,
/* QX3.0 */	(iC_Gt*)iC_2, &s11, 0,
/* QX3.0_0 */	&QX3_0, 0, 0,
/* QX3.1 */	(iC_Gt*)iC_4, &s11, 0,
/* QX3.1_0 */	&QX3_1, 0, 0,
/* QX3.2 */	(iC_Gt*)iC_6, &s11, 0,
/* QX3.2_0 */	&QX3_2, 0, 0,
/* QX3.3 */	(iC_Gt*)iC_8, &s11, 0,
/* QX3.3_0 */	&QX3_3, 0, 0,
/* QX3.4 */	(iC_Gt*)iC_10, &s11, 0,
/* QX3.4_0 */	&QX3_4, 0, 0,
/* QX3.5 */	(iC_Gt*)iC_12, &s11, 0,
/* QX3.5_0 */	&QX3_5, 0, 0,
/* QX3.6 */	(iC_Gt*)iC_14, &s11, 0,
/* QX3.6_0 */	&QX3_6, 0, 0,
/* QX3.7 */	(iC_Gt*)iC_16, &s11, 0,
/* QX3.7_0 */	&QX3_7, 0, 0,
/* QX4.0 */	(iC_Gt*)iC_3, &s11, 0,
/* QX4.0_0 */	&QX4_0, 0, 0,
/* QX4.1 */	(iC_Gt*)iC_5, &s11, 0,
/* QX4.1_0 */	&QX4_1, 0, 0,
/* QX4.2 */	(iC_Gt*)iC_7, &s11, 0,
/* QX4.2_0 */	&QX4_2, 0, 0,
/* QX4.3 */	(iC_Gt*)iC_9, &s11, 0,
/* QX4.3_0 */	&QX4_3, 0, 0,
/* QX4.4 */	(iC_Gt*)iC_11, &s11, 0,
/* QX4.4_0 */	&QX4_4, 0, 0,
/* QX4.5 */	(iC_Gt*)iC_13, &s11, 0,
/* QX4.5_0 */	&QX4_5, 0, 0,
/* QX4.6 */	(iC_Gt*)iC_15, &s11, 0,
/* QX4.6_0 */	&QX4_6, 0, 0,
/* QX4.7 */	(iC_Gt*)iC_17, &s11, 0,
/* QX4.7_0 */	&QX4_7, 0, 0,
/* QX5.0 */	(iC_Gt*)iC_2, &s12, 0,
/* QX5.0_0 */	&QX5_0, 0, 0,
/* QX5.1 */	(iC_Gt*)iC_4, &s12, 0,
/* QX5.1_0 */	&QX5_1, 0, 0,
/* QX5.2 */	(iC_Gt*)iC_6, &s12, 0,
/* QX5.2_0 */	&QX5_2, 0, 0,
/* QX5.3 */	(iC_Gt*)iC_8, &s12, 0,
/* QX5.3_0 */	&QX5_3, 0, 0,
/* QX5.4 */	(iC_Gt*)iC_10, &s12, 0,
/* QX5.4_0 */	&QX5_4, 0, 0,
/* QX5.5 */	(iC_Gt*)iC_12, &s12, 0,
/* QX5.5_0 */	&QX5_5, 0, 0,
/* QX5.6 */	(iC_Gt*)iC_14, &s12, 0,
/* QX5.6_0 */	&QX5_6, 0, 0,
/* QX5.7 */	(iC_Gt*)iC_16, &s12, 0,
/* QX5.7_0 */	&QX5_7, 0, 0,
/* QX6.0_0 */	&in0, 0, 0,
/* QX6.1_0 */	&in1, 0, 0,
/* QX6.2_0 */	&in2, 0, 0,
/* QX6.3_0 */	&in3, 0, 0,
/* QX6.4_0 */	&in, 0, 0,
/* QX6.5_0 */	&pip, 0, 0,
/* QX6.6_0 */	&lock1, 0, 0,
/* QX6.7_0 */	&lock2, 0, 0,
/* c_1 */	&c, &iClock,	&c_3, 0, 0,
/* c_2 */	&c, &iClock,	&TX0_5, 0, &IX0_4, 0,
/* c_4 */	&c_3, &iClock,	&IX0_7, &IX0_4, 0, 0,
/* in */	&in3, &in2, &in1, &in0, 0, 0,
/* in0 */	&in0_2, &in0_1, 0, 0,
/* in0_1 */	&IX0_4, &IX0_0, 0, 0,
/* in0_2 */	&in0_3, 0, &IX0_4, 0,
/* in0_4 */	&in0_3, &iClock,	&in0_6, 0, 0,
/* in0_5 */	&in0_3, &t100, &_10,	&in0_3, 0, 0,
/* in0_7 */	&in0_6, &iClock,	&IX0_0, 0, 0,
/* in1 */	&in1_2, &in1_1, 0, 0,
/* in1_1 */	&IX0_4, &IX0_1, 0, 0,
/* in1_2 */	&in1_3, 0, &IX0_4, 0,
/* in1_4 */	&in1_3, &iClock,	&in1_6, 0, 0,
/* in1_5 */	&in1_3, &t100, &_10,	&in1_3, 0, 0,
/* in1_7 */	&in1_6, &iClock,	&IX0_1, 0, 0,
/* in2 */	&in2_2, &in2_1, 0, 0,
/* in2_1 */	&IX0_4, &IX0_2, 0, 0,
/* in2_2 */	&in2_3, 0, &IX0_4, 0,
/* in2_4 */	&in2_3, &iClock,	&in2_6, 0, 0,
/* in2_5 */	&in2_3, &t100, &_10,	&in2_3, 0, 0,
/* in2_7 */	&in2_6, &iClock,	&IX0_2, 0, 0,
/* in3 */	&in3_2, &in3_1, 0, 0,
/* in3_1 */	&IX0_4, &IX0_3, 0, 0,
/* in3_2 */	&in3_3, 0, &IX0_4, 0,
/* in3_4 */	&in3_3, &iClock,	&in3_6, 0, 0,
/* in3_5 */	&in3_3, &t100, &_10,	&in3_3, 0, 0,
/* in3_7 */	&in3_6, &iClock,	&IX0_3, 0, 0,
/* lock1_1 */	&lock1, &c,	&pip, 0, 0,
/* lock1_2 */	&lock1, &c,	(iC_Gt*)iC_18, &s10, 0,
/* lock2_1 */	&lock2, &iClock,	&lock1, 0, &in, 0,
/* lock2_2 */	&lock2, &iClock,	0, &lock1, 0,
/* m06_1 */	&m06, &c,	&pip, 0, 0,
/* m07_1 */	&m07, &c,	&m07_2, &m06, 0, 0,
/* m07_2 */	&pip, &m07_3, 0, 0,
/* m07_3 */	&in3, &in2, 0, 0,
/* m08_1 */	&m08, &c,	&m08_2, &m07, 0, 0,
/* m08_2 */	&pip, &m08_3, 0, 0,
/* m08_3 */	&in3, &in1, 0, 0,
/* m09_1 */	&m09, &c,	&pip, &m08, 0, 0,
/* pip_1 */	&pip, &c,	&in, 0, &lock2, 0,
/* s10_1 */	&s10, &c,	(iC_Gt*)iC_19, &s10, &s10_2, 0,
/* s10_2 */	&m09, 0, 0,
/* s11_1 */	&s11, &c,	(iC_Gt*)iC_20, &s11, &s10, 0,
/* s12_1 */	&s12, &c,	(iC_Gt*)iC_20, &s12, &s11, 0,
/* t100_1 */	&t100, &iClock,	&TX0_4, 0, 0,
/* t_1 */	&t, &iClock,	&t_3, 0, 0,
/* t_2 */	&t, &iClock,	&TX0_4, 0, &IX0_4, 0,
/* t_4 */	&t_3, &iClock,	&IX0_6, &IX0_4, 0, 0,
/* tor0_1 */	&tor0, &iClock,	(iC_Gt*)iC_21, &s10, 0,
/* tor0_2 */	&tor0, &t, &_5,	&tor0, 0, 0,
/* tor1_1 */	&tor1, &iClock,	(iC_Gt*)iC_22, &s11, 0,
/* tor1_2 */	&tor1, &t, &_5,	&tor1, 0, 0,
/* tor2_1 */	&tor2, &iClock,	(iC_Gt*)iC_23, &s11, 0,
/* tor2_2 */	&tor2, &t, &_5,	&tor2, 0, 0,
/* tor3_1 */	&tor3, &iClock,	(iC_Gt*)iC_24, &s12, 0,
/* tor3_2 */	&tor3, &t, &_5,	&tor3, 0, 0,
};

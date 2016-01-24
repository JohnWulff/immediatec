/********************************************************************
 *
 *	SOURCE:   ./Test0/sort32u.ic
 *	OUTPUT:   ./Test0/sort32u.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: sort32u.c,v 1.1 2015/05/24 00:51:56 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, 0 };
iC_Gt IB3      = { 1, -iC_INPW, iC_ARITH, 0, "IB3", {0}, {0}, &IB1 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB3 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_5    = { 1, -iC_INPX, iC_GATE, 0, "IX0.5", {0}, {0}, &IX0_1 };
iC_Gt IX0_6    = { 1, -iC_INPX, iC_GATE, 0, "IX0.6", {0}, {0}, &IX0_5 };
iC_Gt IX0_7    = { 1, -iC_INPX, iC_GATE, 0, "IX0.7", {0}, {0}, &IX0_6 };
iC_Gt IX2_0    = { 1, -iC_INPX, iC_GATE, 0, "IX2.0", {0}, {0}, &IX0_7 };
iC_Gt IX2_1    = { 1, -iC_INPX, iC_GATE, 0, "IX2.1", {0}, {0}, &IX2_0 };
iC_Gt IX2_6    = { 1, -iC_INPX, iC_GATE, 0, "IX2.6", {0}, {0}, &IX2_1 };
iC_Gt QX0_0    = { 1, -iC_FF, iC_GATE, 0, "QX0.0", {0}, {0}, &IX2_6 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[0]}, &QX0_0 };
iC_Gt QX0_0_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.0_1", {&iC_l_[3]}, {&iC_l_[5]}, &QX0_0_0 };
iC_Gt QX0_0_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.0_2", {&iC_l_[9]}, {&iC_l_[11]}, &QX0_0_1 };
iC_Gt QX0_0_3  = { 1, -iC_ARN, iC_GATE, 0, "QX0.0_3", {0}, {&iC_l_[15]}, &QX0_0_2 };
iC_Gt QX0_0_4  = { 1, -iC_OR, iC_GATE, 0, "QX0.0_4", {0}, {&iC_l_[18]}, &QX0_0_3 };
iC_Gt QX0_0_5  = { 1, -iC_ARN, iC_GATE, 0, "QX0.0_5", {0}, {&iC_l_[22]}, &QX0_0_4 };
iC_Gt QX0_1    = { 1, -iC_FF, iC_GATE, 0, "QX0.1", {0}, {0}, &QX0_0_5 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[26]}, &QX0_1 };
iC_Gt QX0_1_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.1_1", {&iC_l_[29]}, {&iC_l_[31]}, &QX0_1_0 };
iC_Gt QX0_1_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.1_2", {&iC_l_[35]}, {&iC_l_[37]}, &QX0_1_1 };
iC_Gt QX0_1_3  = { 1, -iC_ARN, iC_GATE, 0, "QX0.1_3", {0}, {&iC_l_[41]}, &QX0_1_2 };
iC_Gt QX0_1_4  = { 1, -iC_OR, iC_GATE, 0, "QX0.1_4", {0}, {&iC_l_[44]}, &QX0_1_3 };
iC_Gt QX0_1_5  = { 1, -iC_ARN, iC_GATE, 0, "QX0.1_5", {0}, {&iC_l_[48]}, &QX0_1_4 };
iC_Gt QX0_2    = { 1, -iC_FF, iC_GATE, 0, "QX0.2", {0}, {0}, &QX0_1_5 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[52]}, &QX0_2 };
iC_Gt QX0_2_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.2_1", {&iC_l_[55]}, {&iC_l_[57]}, &QX0_2_0 };
iC_Gt QX0_2_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.2_2", {&iC_l_[61]}, {&iC_l_[63]}, &QX0_2_1 };
iC_Gt QX0_2_3  = { 1, -iC_ARN, iC_GATE, 0, "QX0.2_3", {0}, {&iC_l_[67]}, &QX0_2_2 };
iC_Gt QX0_2_4  = { 1, -iC_OR, iC_GATE, 0, "QX0.2_4", {0}, {&iC_l_[70]}, &QX0_2_3 };
iC_Gt QX0_2_5  = { 1, -iC_ARN, iC_GATE, 0, "QX0.2_5", {0}, {&iC_l_[74]}, &QX0_2_4 };
iC_Gt QX0_3    = { 1, -iC_FF, iC_GATE, 0, "QX0.3", {0}, {0}, &QX0_2_5 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[78]}, &QX0_3 };
iC_Gt QX0_3_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.3_1", {&iC_l_[81]}, {&iC_l_[83]}, &QX0_3_0 };
iC_Gt QX0_3_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.3_2", {&iC_l_[87]}, {&iC_l_[89]}, &QX0_3_1 };
iC_Gt QX0_3_3  = { 1, -iC_ARN, iC_GATE, 0, "QX0.3_3", {0}, {&iC_l_[93]}, &QX0_3_2 };
iC_Gt QX0_3_4  = { 1, -iC_OR, iC_GATE, 0, "QX0.3_4", {0}, {&iC_l_[96]}, &QX0_3_3 };
iC_Gt QX0_3_5  = { 1, -iC_ARN, iC_GATE, 0, "QX0.3_5", {0}, {&iC_l_[100]}, &QX0_3_4 };
iC_Gt QX0_4    = { 1, -iC_FF, iC_GATE, 0, "QX0.4", {0}, {0}, &QX0_3_5 };
iC_Gt QX0_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.4_0", {0}, {&iC_l_[104]}, &QX0_4 };
iC_Gt QX0_4_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.4_1", {&iC_l_[107]}, {&iC_l_[109]}, &QX0_4_0 };
iC_Gt QX0_4_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.4_2", {&iC_l_[113]}, {&iC_l_[115]}, &QX0_4_1 };
iC_Gt QX0_4_3  = { 1, -iC_ARN, iC_GATE, 0, "QX0.4_3", {0}, {&iC_l_[119]}, &QX0_4_2 };
iC_Gt QX0_4_4  = { 1, -iC_OR, iC_GATE, 0, "QX0.4_4", {0}, {&iC_l_[122]}, &QX0_4_3 };
iC_Gt QX0_4_5  = { 1, -iC_ARN, iC_GATE, 0, "QX0.4_5", {0}, {&iC_l_[126]}, &QX0_4_4 };
iC_Gt QX0_5    = { 1, -iC_FF, iC_GATE, 0, "QX0.5", {0}, {0}, &QX0_4_5 };
iC_Gt QX0_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.5_0", {0}, {&iC_l_[130]}, &QX0_5 };
iC_Gt QX0_5_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.5_1", {&iC_l_[133]}, {&iC_l_[135]}, &QX0_5_0 };
iC_Gt QX0_5_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.5_2", {&iC_l_[139]}, {&iC_l_[141]}, &QX0_5_1 };
iC_Gt QX0_5_3  = { 1, -iC_ARN, iC_GATE, 0, "QX0.5_3", {0}, {&iC_l_[145]}, &QX0_5_2 };
iC_Gt QX0_5_4  = { 1, -iC_OR, iC_GATE, 0, "QX0.5_4", {0}, {&iC_l_[148]}, &QX0_5_3 };
iC_Gt QX0_5_5  = { 1, -iC_ARN, iC_GATE, 0, "QX0.5_5", {0}, {&iC_l_[152]}, &QX0_5_4 };
iC_Gt QX0_6    = { 1, -iC_FF, iC_GATE, 0, "QX0.6", {0}, {0}, &QX0_5_5 };
iC_Gt QX0_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.6_0", {0}, {&iC_l_[156]}, &QX0_6 };
iC_Gt QX0_6_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.6_1", {&iC_l_[159]}, {&iC_l_[161]}, &QX0_6_0 };
iC_Gt QX0_6_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.6_2", {&iC_l_[165]}, {&iC_l_[167]}, &QX0_6_1 };
iC_Gt QX0_6_3  = { 1, -iC_ARN, iC_GATE, 0, "QX0.6_3", {0}, {&iC_l_[171]}, &QX0_6_2 };
iC_Gt QX0_6_4  = { 1, -iC_OR, iC_GATE, 0, "QX0.6_4", {0}, {&iC_l_[174]}, &QX0_6_3 };
iC_Gt QX0_6_5  = { 1, -iC_ARN, iC_GATE, 0, "QX0.6_5", {0}, {&iC_l_[178]}, &QX0_6_4 };
iC_Gt QX0_7    = { 1, -iC_FF, iC_GATE, 0, "QX0.7", {0}, {0}, &QX0_6_5 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[182]}, &QX0_7 };
iC_Gt QX0_7_1  = { 1, -iC_AND, iC_S_FF, 0, "QX0.7_1", {&iC_l_[185]}, {&iC_l_[187]}, &QX0_7_0 };
iC_Gt QX0_7_2  = { 1, -iC_AND, iC_R_FF, 0, "QX0.7_2", {&iC_l_[191]}, {&iC_l_[193]}, &QX0_7_1 };
iC_Gt QX0_7_3  = { 1, -iC_ARN, iC_GATE, 0, "QX0.7_3", {0}, {&iC_l_[197]}, &QX0_7_2 };
iC_Gt QX0_7_4  = { 1, -iC_OR, iC_GATE, 0, "QX0.7_4", {0}, {&iC_l_[200]}, &QX0_7_3 };
iC_Gt QX0_7_5  = { 1, -iC_ARN, iC_GATE, 0, "QX0.7_5", {0}, {&iC_l_[204]}, &QX0_7_4 };
iC_Gt QX1_0    = { 1, -iC_FF, iC_GATE, 0, "QX1.0", {0}, {0}, &QX0_7_5 };
iC_Gt QX1_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.0_0", {0}, {&iC_l_[208]}, &QX1_0 };
iC_Gt QX1_0_1  = { 1, -iC_AND, iC_S_FF, 0, "QX1.0_1", {&iC_l_[211]}, {&iC_l_[213]}, &QX1_0_0 };
iC_Gt QX1_0_2  = { 1, -iC_AND, iC_R_FF, 0, "QX1.0_2", {&iC_l_[217]}, {&iC_l_[219]}, &QX1_0_1 };
iC_Gt QX1_0_3  = { 1, -iC_ARN, iC_GATE, 0, "QX1.0_3", {0}, {&iC_l_[223]}, &QX1_0_2 };
iC_Gt QX1_0_4  = { 1, -iC_OR, iC_GATE, 0, "QX1.0_4", {0}, {&iC_l_[226]}, &QX1_0_3 };
iC_Gt QX1_0_5  = { 1, -iC_ARN, iC_GATE, 0, "QX1.0_5", {0}, {&iC_l_[230]}, &QX1_0_4 };
iC_Gt QX1_1    = { 1, -iC_FF, iC_GATE, 0, "QX1.1", {0}, {0}, &QX1_0_5 };
iC_Gt QX1_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.1_0", {0}, {&iC_l_[234]}, &QX1_1 };
iC_Gt QX1_1_1  = { 1, -iC_AND, iC_S_FF, 0, "QX1.1_1", {&iC_l_[237]}, {&iC_l_[239]}, &QX1_1_0 };
iC_Gt QX1_1_2  = { 1, -iC_AND, iC_R_FF, 0, "QX1.1_2", {&iC_l_[243]}, {&iC_l_[245]}, &QX1_1_1 };
iC_Gt QX1_1_3  = { 1, -iC_ARN, iC_GATE, 0, "QX1.1_3", {0}, {&iC_l_[249]}, &QX1_1_2 };
iC_Gt QX1_1_4  = { 1, -iC_OR, iC_GATE, 0, "QX1.1_4", {0}, {&iC_l_[252]}, &QX1_1_3 };
iC_Gt QX1_1_5  = { 1, -iC_ARN, iC_GATE, 0, "QX1.1_5", {0}, {&iC_l_[256]}, &QX1_1_4 };
iC_Gt QX2_0    = { 1, -iC_FF, iC_GATE, 0, "QX2.0", {0}, {0}, &QX1_1_5 };
iC_Gt QX2_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.0_0", {0}, {&iC_l_[260]}, &QX2_0 };
iC_Gt QX2_0_1  = { 1, -iC_AND, iC_S_FF, 0, "QX2.0_1", {&iC_l_[263]}, {&iC_l_[265]}, &QX2_0_0 };
iC_Gt QX2_0_2  = { 1, -iC_AND, iC_R_FF, 0, "QX2.0_2", {&iC_l_[269]}, {&iC_l_[271]}, &QX2_0_1 };
iC_Gt QX2_0_3  = { 1, -iC_ARN, iC_GATE, 0, "QX2.0_3", {0}, {&iC_l_[275]}, &QX2_0_2 };
iC_Gt QX2_0_4  = { 1, -iC_OR, iC_GATE, 0, "QX2.0_4", {0}, {&iC_l_[278]}, &QX2_0_3 };
iC_Gt QX2_0_5  = { 1, -iC_ARN, iC_GATE, 0, "QX2.0_5", {0}, {&iC_l_[282]}, &QX2_0_4 };
iC_Gt QX2_1    = { 1, -iC_FF, iC_GATE, 0, "QX2.1", {0}, {0}, &QX2_0_5 };
iC_Gt QX2_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.1_0", {0}, {&iC_l_[286]}, &QX2_1 };
iC_Gt QX2_1_1  = { 1, -iC_AND, iC_S_FF, 0, "QX2.1_1", {&iC_l_[289]}, {&iC_l_[291]}, &QX2_1_0 };
iC_Gt QX2_1_2  = { 1, -iC_AND, iC_R_FF, 0, "QX2.1_2", {&iC_l_[295]}, {&iC_l_[297]}, &QX2_1_1 };
iC_Gt QX2_1_3  = { 1, -iC_ARN, iC_GATE, 0, "QX2.1_3", {0}, {&iC_l_[301]}, &QX2_1_2 };
iC_Gt QX2_1_4  = { 1, -iC_OR, iC_GATE, 0, "QX2.1_4", {0}, {&iC_l_[304]}, &QX2_1_3 };
iC_Gt QX2_1_5  = { 1, -iC_ARN, iC_GATE, 0, "QX2.1_5", {0}, {&iC_l_[308]}, &QX2_1_4 };
iC_Gt QX2_2    = { 1, -iC_FF, iC_GATE, 0, "QX2.2", {0}, {0}, &QX2_1_5 };
iC_Gt QX2_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.2_0", {0}, {&iC_l_[312]}, &QX2_2 };
iC_Gt QX2_2_1  = { 1, -iC_AND, iC_S_FF, 0, "QX2.2_1", {&iC_l_[315]}, {&iC_l_[317]}, &QX2_2_0 };
iC_Gt QX2_2_2  = { 1, -iC_AND, iC_R_FF, 0, "QX2.2_2", {&iC_l_[321]}, {&iC_l_[323]}, &QX2_2_1 };
iC_Gt QX2_2_3  = { 1, -iC_ARN, iC_GATE, 0, "QX2.2_3", {0}, {&iC_l_[327]}, &QX2_2_2 };
iC_Gt QX2_2_4  = { 1, -iC_OR, iC_GATE, 0, "QX2.2_4", {0}, {&iC_l_[330]}, &QX2_2_3 };
iC_Gt QX2_2_5  = { 1, -iC_ARN, iC_GATE, 0, "QX2.2_5", {0}, {&iC_l_[334]}, &QX2_2_4 };
iC_Gt QX2_3    = { 1, -iC_FF, iC_GATE, 0, "QX2.3", {0}, {0}, &QX2_2_5 };
iC_Gt QX2_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.3_0", {0}, {&iC_l_[338]}, &QX2_3 };
iC_Gt QX2_3_1  = { 1, -iC_AND, iC_S_FF, 0, "QX2.3_1", {&iC_l_[341]}, {&iC_l_[343]}, &QX2_3_0 };
iC_Gt QX2_3_2  = { 1, -iC_AND, iC_R_FF, 0, "QX2.3_2", {&iC_l_[347]}, {&iC_l_[349]}, &QX2_3_1 };
iC_Gt QX2_3_3  = { 1, -iC_ARN, iC_GATE, 0, "QX2.3_3", {0}, {&iC_l_[353]}, &QX2_3_2 };
iC_Gt QX2_3_4  = { 1, -iC_OR, iC_GATE, 0, "QX2.3_4", {0}, {&iC_l_[356]}, &QX2_3_3 };
iC_Gt QX2_3_5  = { 1, -iC_ARN, iC_GATE, 0, "QX2.3_5", {0}, {&iC_l_[360]}, &QX2_3_4 };
iC_Gt QX2_4    = { 1, -iC_FF, iC_GATE, 0, "QX2.4", {0}, {0}, &QX2_3_5 };
iC_Gt QX2_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.4_0", {0}, {&iC_l_[364]}, &QX2_4 };
iC_Gt QX2_4_1  = { 1, -iC_AND, iC_S_FF, 0, "QX2.4_1", {&iC_l_[367]}, {&iC_l_[369]}, &QX2_4_0 };
iC_Gt QX2_4_2  = { 1, -iC_AND, iC_R_FF, 0, "QX2.4_2", {&iC_l_[373]}, {&iC_l_[375]}, &QX2_4_1 };
iC_Gt QX2_4_3  = { 1, -iC_ARN, iC_GATE, 0, "QX2.4_3", {0}, {&iC_l_[379]}, &QX2_4_2 };
iC_Gt QX2_4_4  = { 1, -iC_OR, iC_GATE, 0, "QX2.4_4", {0}, {&iC_l_[382]}, &QX2_4_3 };
iC_Gt QX2_4_5  = { 1, -iC_ARN, iC_GATE, 0, "QX2.4_5", {0}, {&iC_l_[386]}, &QX2_4_4 };
iC_Gt QX2_5    = { 1, -iC_FF, iC_GATE, 0, "QX2.5", {0}, {0}, &QX2_4_5 };
iC_Gt QX2_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.5_0", {0}, {&iC_l_[390]}, &QX2_5 };
iC_Gt QX2_5_1  = { 1, -iC_AND, iC_S_FF, 0, "QX2.5_1", {&iC_l_[393]}, {&iC_l_[395]}, &QX2_5_0 };
iC_Gt QX2_5_2  = { 1, -iC_AND, iC_R_FF, 0, "QX2.5_2", {&iC_l_[399]}, {&iC_l_[401]}, &QX2_5_1 };
iC_Gt QX2_5_3  = { 1, -iC_ARN, iC_GATE, 0, "QX2.5_3", {0}, {&iC_l_[405]}, &QX2_5_2 };
iC_Gt QX2_5_4  = { 1, -iC_OR, iC_GATE, 0, "QX2.5_4", {0}, {&iC_l_[408]}, &QX2_5_3 };
iC_Gt QX2_5_5  = { 1, -iC_ARN, iC_GATE, 0, "QX2.5_5", {0}, {&iC_l_[412]}, &QX2_5_4 };
iC_Gt QX2_6    = { 1, -iC_FF, iC_GATE, 0, "QX2.6", {0}, {0}, &QX2_5_5 };
iC_Gt QX2_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.6_0", {0}, {&iC_l_[416]}, &QX2_6 };
iC_Gt QX2_6_1  = { 1, -iC_AND, iC_S_FF, 0, "QX2.6_1", {&iC_l_[419]}, {&iC_l_[421]}, &QX2_6_0 };
iC_Gt QX2_6_2  = { 1, -iC_AND, iC_R_FF, 0, "QX2.6_2", {&iC_l_[425]}, {&iC_l_[427]}, &QX2_6_1 };
iC_Gt QX2_6_3  = { 1, -iC_ARN, iC_GATE, 0, "QX2.6_3", {0}, {&iC_l_[431]}, &QX2_6_2 };
iC_Gt QX2_6_4  = { 1, -iC_OR, iC_GATE, 0, "QX2.6_4", {0}, {&iC_l_[434]}, &QX2_6_3 };
iC_Gt QX2_6_5  = { 1, -iC_ARN, iC_GATE, 0, "QX2.6_5", {0}, {&iC_l_[438]}, &QX2_6_4 };
iC_Gt QX2_7    = { 1, -iC_FF, iC_GATE, 0, "QX2.7", {0}, {0}, &QX2_6_5 };
iC_Gt QX2_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.7_0", {0}, {&iC_l_[442]}, &QX2_7 };
iC_Gt QX2_7_1  = { 1, -iC_AND, iC_S_FF, 0, "QX2.7_1", {&iC_l_[445]}, {&iC_l_[447]}, &QX2_7_0 };
iC_Gt QX2_7_2  = { 1, -iC_AND, iC_R_FF, 0, "QX2.7_2", {&iC_l_[451]}, {&iC_l_[453]}, &QX2_7_1 };
iC_Gt QX2_7_3  = { 1, -iC_ARN, iC_GATE, 0, "QX2.7_3", {0}, {&iC_l_[457]}, &QX2_7_2 };
iC_Gt QX2_7_4  = { 1, -iC_OR, iC_GATE, 0, "QX2.7_4", {0}, {&iC_l_[460]}, &QX2_7_3 };
iC_Gt QX2_7_5  = { 1, -iC_ARN, iC_GATE, 0, "QX2.7_5", {0}, {&iC_l_[464]}, &QX2_7_4 };
iC_Gt QX3_0    = { 1, -iC_FF, iC_GATE, 0, "QX3.0", {0}, {0}, &QX2_7_5 };
iC_Gt QX3_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.0_0", {0}, {&iC_l_[468]}, &QX3_0 };
iC_Gt QX3_0_1  = { 1, -iC_AND, iC_S_FF, 0, "QX3.0_1", {&iC_l_[471]}, {&iC_l_[473]}, &QX3_0_0 };
iC_Gt QX3_0_2  = { 1, -iC_AND, iC_R_FF, 0, "QX3.0_2", {&iC_l_[477]}, {&iC_l_[479]}, &QX3_0_1 };
iC_Gt QX3_0_3  = { 1, -iC_ARN, iC_GATE, 0, "QX3.0_3", {0}, {&iC_l_[483]}, &QX3_0_2 };
iC_Gt QX3_0_4  = { 1, -iC_OR, iC_GATE, 0, "QX3.0_4", {0}, {&iC_l_[486]}, &QX3_0_3 };
iC_Gt QX3_0_5  = { 1, -iC_ARN, iC_GATE, 0, "QX3.0_5", {0}, {&iC_l_[490]}, &QX3_0_4 };
iC_Gt QX3_1    = { 1, -iC_FF, iC_GATE, 0, "QX3.1", {0}, {0}, &QX3_0_5 };
iC_Gt QX3_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.1_0", {0}, {&iC_l_[494]}, &QX3_1 };
iC_Gt QX3_1_1  = { 1, -iC_AND, iC_S_FF, 0, "QX3.1_1", {&iC_l_[497]}, {&iC_l_[499]}, &QX3_1_0 };
iC_Gt QX3_1_2  = { 1, -iC_AND, iC_R_FF, 0, "QX3.1_2", {&iC_l_[503]}, {&iC_l_[505]}, &QX3_1_1 };
iC_Gt QX3_1_3  = { 1, -iC_ARN, iC_GATE, 0, "QX3.1_3", {0}, {&iC_l_[509]}, &QX3_1_2 };
iC_Gt QX3_1_4  = { 1, -iC_OR, iC_GATE, 0, "QX3.1_4", {0}, {&iC_l_[512]}, &QX3_1_3 };
iC_Gt QX3_1_5  = { 1, -iC_ARN, iC_GATE, 0, "QX3.1_5", {0}, {&iC_l_[516]}, &QX3_1_4 };
iC_Gt QX4_0    = { 1, -iC_ARN, iC_GATE, 0, "QX4.0", {0}, {&iC_l_[520]}, &QX3_1_5 };
iC_Gt QX4_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.0_0", {0}, {&iC_l_[523]}, &QX4_0 };
iC_Gt QX4_1    = { 1, -iC_ARN, iC_GATE, 0, "QX4.1", {0}, {&iC_l_[526]}, &QX4_0_0 };
iC_Gt QX4_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.1_0", {0}, {&iC_l_[529]}, &QX4_1 };
iC_Gt QX4_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.2_0", {0}, {&iC_l_[532]}, &QX4_1_0 };
iC_Gt QX4_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX4.3_0", {0}, {&iC_l_[535]}, &QX4_2_0 };
iC_Gt TX0_4    = { 1, -iC_INPX, iC_GATE, 0, "TX0.4", {0}, {0}, &QX4_3_0 };
iC_Gt belt_1_car0 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_car0", {0}, {&iC_l_[538]}, &TX0_4 };
iC_Gt belt_1_car1 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_car1", {0}, {&iC_l_[541]}, &belt_1_car0 };
iC_Gt belt_1_car10 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_car10", {0}, {&iC_l_[544]}, &belt_1_car1 };
iC_Gt belt_1_car2 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_car2", {0}, {&iC_l_[547]}, &belt_1_car10 };
iC_Gt belt_1_car3 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_car3", {0}, {&iC_l_[550]}, &belt_1_car2 };
iC_Gt belt_1_car4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_car4", {0}, {&iC_l_[553]}, &belt_1_car3 };
iC_Gt belt_1_car5 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_car5", {0}, {&iC_l_[556]}, &belt_1_car4 };
iC_Gt belt_1_car6 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_car6", {0}, {&iC_l_[559]}, &belt_1_car5 };
iC_Gt belt_1_car7 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_car7", {0}, {&iC_l_[562]}, &belt_1_car6 };
iC_Gt belt_1_car8 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_car8", {0}, {&iC_l_[565]}, &belt_1_car7 };
iC_Gt belt_1_car9 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_car9", {0}, {&iC_l_[568]}, &belt_1_car8 };
iC_Gt belt_1_clk = { 1, -iC_CLK, iC_CLCKL, 0, "belt_1_clk", {0}, {0}, &belt_1_car9 };
iC_Gt belt_1_clk_1 = { 1, -iC_OR, iC_CLCK, 0, "belt_1_clk_1", {&iC_l_[571]}, {&iC_l_[573]}, &belt_1_clk };
iC_Gt belt_1_clk_2 = { 1, -iC_VF, iC_GATE, 0, "belt_1_clk_2", {0}, {0}, &belt_1_clk_1 };
iC_Gt belt_1_clk_3 = { 1, -iC_OR, iC_CH_BIT, 0, "belt_1_clk_3", {&iC_l_[576]}, {&iC_l_[578]}, &belt_1_clk_2 };
iC_Gt belt_1_clk_4 = { 1, -iC_AND, iC_GATE, 0, "belt_1_clk_4", {0}, {&iC_l_[582]}, &belt_1_clk_3 };
iC_Gt belt_1_clk_5 = { 1, -iC_AND, iC_GATE, 0, "belt_1_clk_5", {0}, {&iC_l_[586]}, &belt_1_clk_4 };
iC_Gt belt_1_mask = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_mask", {0}, {&iC_l_[590]}, &belt_1_clk_5 };
iC_Gt belt_1_mask1 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_mask1", {0}, {&iC_l_[593]}, &belt_1_mask };
iC_Gt belt_1_mask10 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_mask10", {0}, {&iC_l_[596]}, &belt_1_mask1 };
iC_Gt belt_1_mask2 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_mask2", {0}, {&iC_l_[599]}, &belt_1_mask10 };
iC_Gt belt_1_mask3 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_mask3", {0}, {&iC_l_[602]}, &belt_1_mask2 };
iC_Gt belt_1_mask4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_mask4", {0}, {&iC_l_[605]}, &belt_1_mask3 };
iC_Gt belt_1_mask5 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_mask5", {0}, {&iC_l_[608]}, &belt_1_mask4 };
iC_Gt belt_1_mask6 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_mask6", {0}, {&iC_l_[611]}, &belt_1_mask5 };
iC_Gt belt_1_mask7 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_mask7", {0}, {&iC_l_[614]}, &belt_1_mask6 };
iC_Gt belt_1_mask8 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_mask8", {0}, {&iC_l_[617]}, &belt_1_mask7 };
iC_Gt belt_1_mask9 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_mask9", {0}, {&iC_l_[620]}, &belt_1_mask8 };
iC_Gt belt_1_pip = { 1, -iC_EF, iC_GATE, 0, "belt_1_pip", {0}, {0}, &belt_1_mask9 };
iC_Gt belt_1_pip_1 = { 1, -iC_AND, iC_RI_BIT, 0, "belt_1_pip_1", {&iC_l_[623]}, {&iC_l_[625]}, &belt_1_pip };
iC_Gt belt_1_pip_2 = { 1, -iC_FF, iC_GATE, 0, "belt_1_pip_2", {0}, {0}, &belt_1_pip_1 };
iC_Gt belt_1_pip_3 = { 1, -iC_AND, iC_S_FF, 0, "belt_1_pip_3", {&iC_l_[630]}, {&iC_l_[632]}, &belt_1_pip_2 };
iC_Gt belt_1_pip_4 = { 1, -iC_AND, iC_R_FF, 0, "belt_1_pip_4", {&iC_l_[636]}, {&iC_l_[638]}, &belt_1_pip_3 };
iC_Gt belt_1_pip_5 = { 1, -iC_OR, iC_GATE, 0, "belt_1_pip_5", {0}, {&iC_l_[642]}, &belt_1_pip_4 };
iC_Gt belt_1_pip_6 = { 1, -iC_ARN, iC_GATE, 0, "belt_1_pip_6", {0}, {&iC_l_[646]}, &belt_1_pip_5 };
iC_Gt belt_1_shift = { 1, -iC_SH, iC_ARITH, 0, "belt_1_shift", {0}, {0}, &belt_1_pip_6 };
iC_Gt belt_1_shift1 = { 1, -iC_SH, iC_ARITH, 0, "belt_1_shift1", {0}, {0}, &belt_1_shift };
iC_Gt belt_1_shift10 = { 1, -iC_SH, iC_ARITH, 0, "belt_1_shift10", {0}, {0}, &belt_1_shift1 };
iC_Gt belt_1_shift10_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_1_shift10_1", {&iC_l_[650]}, {&iC_l_[652]}, &belt_1_shift10 };
iC_Gt belt_1_shift10_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_1_shift10_2", {&iC_l_[657]}, {&iC_l_[659]}, &belt_1_shift10_1 };
iC_Gt belt_1_shift10_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_1_shift10_3", {0}, {&iC_l_[662]}, &belt_1_shift10_2 };
iC_Gt belt_1_shift10_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_shift10_4", {0}, {&iC_l_[665]}, &belt_1_shift10_3 };
iC_Gt belt_1_shift1_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_1_shift1_1", {&iC_l_[669]}, {&iC_l_[671]}, &belt_1_shift10_4 };
iC_Gt belt_1_shift1_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_1_shift1_2", {&iC_l_[676]}, {&iC_l_[678]}, &belt_1_shift1_1 };
iC_Gt belt_1_shift1_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_1_shift1_3", {0}, {&iC_l_[681]}, &belt_1_shift1_2 };
iC_Gt belt_1_shift1_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_shift1_4", {0}, {&iC_l_[684]}, &belt_1_shift1_3 };
iC_Gt belt_1_shift2 = { 1, -iC_SH, iC_ARITH, 0, "belt_1_shift2", {0}, {0}, &belt_1_shift1_4 };
iC_Gt belt_1_shift2_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_1_shift2_1", {&iC_l_[688]}, {&iC_l_[690]}, &belt_1_shift2 };
iC_Gt belt_1_shift2_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_1_shift2_2", {&iC_l_[695]}, {&iC_l_[697]}, &belt_1_shift2_1 };
iC_Gt belt_1_shift2_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_1_shift2_3", {0}, {&iC_l_[700]}, &belt_1_shift2_2 };
iC_Gt belt_1_shift2_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_shift2_4", {0}, {&iC_l_[703]}, &belt_1_shift2_3 };
iC_Gt belt_1_shift3 = { 1, -iC_SH, iC_ARITH, 0, "belt_1_shift3", {0}, {0}, &belt_1_shift2_4 };
iC_Gt belt_1_shift3_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_1_shift3_1", {&iC_l_[707]}, {&iC_l_[709]}, &belt_1_shift3 };
iC_Gt belt_1_shift3_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_1_shift3_2", {&iC_l_[714]}, {&iC_l_[716]}, &belt_1_shift3_1 };
iC_Gt belt_1_shift3_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_1_shift3_3", {0}, {&iC_l_[719]}, &belt_1_shift3_2 };
iC_Gt belt_1_shift3_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_shift3_4", {0}, {&iC_l_[722]}, &belt_1_shift3_3 };
iC_Gt belt_1_shift4 = { 1, -iC_SH, iC_ARITH, 0, "belt_1_shift4", {0}, {0}, &belt_1_shift3_4 };
iC_Gt belt_1_shift4_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_1_shift4_1", {&iC_l_[726]}, {&iC_l_[728]}, &belt_1_shift4 };
iC_Gt belt_1_shift4_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_1_shift4_2", {&iC_l_[733]}, {&iC_l_[735]}, &belt_1_shift4_1 };
iC_Gt belt_1_shift4_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_1_shift4_3", {0}, {&iC_l_[738]}, &belt_1_shift4_2 };
iC_Gt belt_1_shift4_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_shift4_4", {0}, {&iC_l_[741]}, &belt_1_shift4_3 };
iC_Gt belt_1_shift5 = { 1, -iC_SH, iC_ARITH, 0, "belt_1_shift5", {0}, {0}, &belt_1_shift4_4 };
iC_Gt belt_1_shift5_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_1_shift5_1", {&iC_l_[745]}, {&iC_l_[747]}, &belt_1_shift5 };
iC_Gt belt_1_shift5_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_1_shift5_2", {&iC_l_[752]}, {&iC_l_[754]}, &belt_1_shift5_1 };
iC_Gt belt_1_shift5_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_1_shift5_3", {0}, {&iC_l_[757]}, &belt_1_shift5_2 };
iC_Gt belt_1_shift5_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_shift5_4", {0}, {&iC_l_[760]}, &belt_1_shift5_3 };
iC_Gt belt_1_shift6 = { 1, -iC_SH, iC_ARITH, 0, "belt_1_shift6", {0}, {0}, &belt_1_shift5_4 };
iC_Gt belt_1_shift6_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_1_shift6_1", {&iC_l_[764]}, {&iC_l_[766]}, &belt_1_shift6 };
iC_Gt belt_1_shift6_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_1_shift6_2", {&iC_l_[771]}, {&iC_l_[773]}, &belt_1_shift6_1 };
iC_Gt belt_1_shift6_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_1_shift6_3", {0}, {&iC_l_[776]}, &belt_1_shift6_2 };
iC_Gt belt_1_shift6_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_shift6_4", {0}, {&iC_l_[779]}, &belt_1_shift6_3 };
iC_Gt belt_1_shift7 = { 1, -iC_SH, iC_ARITH, 0, "belt_1_shift7", {0}, {0}, &belt_1_shift6_4 };
iC_Gt belt_1_shift7_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_1_shift7_1", {&iC_l_[783]}, {&iC_l_[785]}, &belt_1_shift7 };
iC_Gt belt_1_shift7_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_1_shift7_2", {&iC_l_[790]}, {&iC_l_[792]}, &belt_1_shift7_1 };
iC_Gt belt_1_shift7_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_1_shift7_3", {0}, {&iC_l_[795]}, &belt_1_shift7_2 };
iC_Gt belt_1_shift7_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_shift7_4", {0}, {&iC_l_[798]}, &belt_1_shift7_3 };
iC_Gt belt_1_shift8 = { 1, -iC_SH, iC_ARITH, 0, "belt_1_shift8", {0}, {0}, &belt_1_shift7_4 };
iC_Gt belt_1_shift8_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_1_shift8_1", {&iC_l_[802]}, {&iC_l_[804]}, &belt_1_shift8 };
iC_Gt belt_1_shift8_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_1_shift8_2", {&iC_l_[809]}, {&iC_l_[811]}, &belt_1_shift8_1 };
iC_Gt belt_1_shift8_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_1_shift8_3", {0}, {&iC_l_[814]}, &belt_1_shift8_2 };
iC_Gt belt_1_shift8_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_shift8_4", {0}, {&iC_l_[817]}, &belt_1_shift8_3 };
iC_Gt belt_1_shift9 = { 1, -iC_SH, iC_ARITH, 0, "belt_1_shift9", {0}, {0}, &belt_1_shift8_4 };
iC_Gt belt_1_shift9_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_1_shift9_1", {&iC_l_[821]}, {&iC_l_[823]}, &belt_1_shift9 };
iC_Gt belt_1_shift9_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_1_shift9_2", {&iC_l_[828]}, {&iC_l_[830]}, &belt_1_shift9_1 };
iC_Gt belt_1_shift9_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_1_shift9_3", {0}, {&iC_l_[833]}, &belt_1_shift9_2 };
iC_Gt belt_1_shift9_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_shift9_4", {0}, {&iC_l_[836]}, &belt_1_shift9_3 };
iC_Gt belt_1_shift_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_1_shift_1", {&iC_l_[840]}, {&iC_l_[842]}, &belt_1_shift9_4 };
iC_Gt belt_1_shift_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_1_shift_2", {&iC_l_[847]}, {&iC_l_[849]}, &belt_1_shift_1 };
iC_Gt belt_1_shift_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_1_shift_3", {0}, {&iC_l_[852]}, &belt_1_shift_2 };
iC_Gt belt_1_shift_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_1_shift_4", {0}, {&iC_l_[855]}, &belt_1_shift_3 };
iC_Gt belt_1_shift_5 = { 1, -iC_OR, iC_ARITH, 0, "belt_1_shift_5", {0}, {&iC_l_[860]}, &belt_1_shift_4 };
iC_Gt belt_2_car0 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_car0", {0}, {&iC_l_[863]}, &belt_1_shift_5 };
iC_Gt belt_2_car1 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_car1", {0}, {&iC_l_[866]}, &belt_2_car0 };
iC_Gt belt_2_car10 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_car10", {0}, {&iC_l_[869]}, &belt_2_car1 };
iC_Gt belt_2_car2 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_car2", {0}, {&iC_l_[872]}, &belt_2_car10 };
iC_Gt belt_2_car3 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_car3", {0}, {&iC_l_[875]}, &belt_2_car2 };
iC_Gt belt_2_car4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_car4", {0}, {&iC_l_[878]}, &belt_2_car3 };
iC_Gt belt_2_car5 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_car5", {0}, {&iC_l_[881]}, &belt_2_car4 };
iC_Gt belt_2_car6 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_car6", {0}, {&iC_l_[884]}, &belt_2_car5 };
iC_Gt belt_2_car7 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_car7", {0}, {&iC_l_[887]}, &belt_2_car6 };
iC_Gt belt_2_car8 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_car8", {0}, {&iC_l_[890]}, &belt_2_car7 };
iC_Gt belt_2_car9 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_car9", {0}, {&iC_l_[893]}, &belt_2_car8 };
iC_Gt belt_2_clk = { 1, -iC_CLK, iC_CLCKL, 0, "belt_2_clk", {0}, {0}, &belt_2_car9 };
iC_Gt belt_2_clk_1 = { 1, -iC_OR, iC_CLCK, 0, "belt_2_clk_1", {&iC_l_[896]}, {&iC_l_[898]}, &belt_2_clk };
iC_Gt belt_2_clk_2 = { 1, -iC_VF, iC_GATE, 0, "belt_2_clk_2", {0}, {0}, &belt_2_clk_1 };
iC_Gt belt_2_clk_3 = { 1, -iC_OR, iC_CH_BIT, 0, "belt_2_clk_3", {&iC_l_[901]}, {&iC_l_[903]}, &belt_2_clk_2 };
iC_Gt belt_2_clk_4 = { 1, -iC_AND, iC_GATE, 0, "belt_2_clk_4", {0}, {&iC_l_[907]}, &belt_2_clk_3 };
iC_Gt belt_2_clk_5 = { 1, -iC_AND, iC_GATE, 0, "belt_2_clk_5", {0}, {&iC_l_[911]}, &belt_2_clk_4 };
iC_Gt belt_2_mask = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_mask", {0}, {&iC_l_[915]}, &belt_2_clk_5 };
iC_Gt belt_2_mask1 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_mask1", {0}, {&iC_l_[918]}, &belt_2_mask };
iC_Gt belt_2_mask10 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_mask10", {0}, {&iC_l_[921]}, &belt_2_mask1 };
iC_Gt belt_2_mask2 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_mask2", {0}, {&iC_l_[924]}, &belt_2_mask10 };
iC_Gt belt_2_mask3 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_mask3", {0}, {&iC_l_[927]}, &belt_2_mask2 };
iC_Gt belt_2_mask4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_mask4", {0}, {&iC_l_[930]}, &belt_2_mask3 };
iC_Gt belt_2_mask5 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_mask5", {0}, {&iC_l_[933]}, &belt_2_mask4 };
iC_Gt belt_2_mask6 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_mask6", {0}, {&iC_l_[936]}, &belt_2_mask5 };
iC_Gt belt_2_mask7 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_mask7", {0}, {&iC_l_[939]}, &belt_2_mask6 };
iC_Gt belt_2_mask8 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_mask8", {0}, {&iC_l_[942]}, &belt_2_mask7 };
iC_Gt belt_2_mask9 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_mask9", {0}, {&iC_l_[945]}, &belt_2_mask8 };
iC_Gt belt_2_pip = { 1, -iC_EF, iC_GATE, 0, "belt_2_pip", {0}, {0}, &belt_2_mask9 };
iC_Gt belt_2_pip_1 = { 1, -iC_AND, iC_RI_BIT, 0, "belt_2_pip_1", {&iC_l_[948]}, {&iC_l_[950]}, &belt_2_pip };
iC_Gt belt_2_pip_2 = { 1, -iC_FF, iC_GATE, 0, "belt_2_pip_2", {0}, {0}, &belt_2_pip_1 };
iC_Gt belt_2_pip_3 = { 1, -iC_AND, iC_S_FF, 0, "belt_2_pip_3", {&iC_l_[955]}, {&iC_l_[957]}, &belt_2_pip_2 };
iC_Gt belt_2_pip_4 = { 1, -iC_AND, iC_R_FF, 0, "belt_2_pip_4", {&iC_l_[961]}, {&iC_l_[963]}, &belt_2_pip_3 };
iC_Gt belt_2_pip_5 = { 1, -iC_OR, iC_GATE, 0, "belt_2_pip_5", {0}, {&iC_l_[967]}, &belt_2_pip_4 };
iC_Gt belt_2_pip_6 = { 1, -iC_ARN, iC_GATE, 0, "belt_2_pip_6", {0}, {&iC_l_[971]}, &belt_2_pip_5 };
iC_Gt belt_2_shift = { 1, -iC_SH, iC_ARITH, 0, "belt_2_shift", {0}, {0}, &belt_2_pip_6 };
iC_Gt belt_2_shift1 = { 1, -iC_SH, iC_ARITH, 0, "belt_2_shift1", {0}, {0}, &belt_2_shift };
iC_Gt belt_2_shift10 = { 1, -iC_SH, iC_ARITH, 0, "belt_2_shift10", {0}, {0}, &belt_2_shift1 };
iC_Gt belt_2_shift10_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_2_shift10_1", {&iC_l_[975]}, {&iC_l_[977]}, &belt_2_shift10 };
iC_Gt belt_2_shift10_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_2_shift10_2", {&iC_l_[982]}, {&iC_l_[984]}, &belt_2_shift10_1 };
iC_Gt belt_2_shift10_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_2_shift10_3", {0}, {&iC_l_[987]}, &belt_2_shift10_2 };
iC_Gt belt_2_shift10_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_shift10_4", {0}, {&iC_l_[990]}, &belt_2_shift10_3 };
iC_Gt belt_2_shift1_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_2_shift1_1", {&iC_l_[994]}, {&iC_l_[996]}, &belt_2_shift10_4 };
iC_Gt belt_2_shift1_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_2_shift1_2", {&iC_l_[1001]}, {&iC_l_[1003]}, &belt_2_shift1_1 };
iC_Gt belt_2_shift1_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_2_shift1_3", {0}, {&iC_l_[1006]}, &belt_2_shift1_2 };
iC_Gt belt_2_shift1_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_shift1_4", {0}, {&iC_l_[1009]}, &belt_2_shift1_3 };
iC_Gt belt_2_shift2 = { 1, -iC_SH, iC_ARITH, 0, "belt_2_shift2", {0}, {0}, &belt_2_shift1_4 };
iC_Gt belt_2_shift2_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_2_shift2_1", {&iC_l_[1013]}, {&iC_l_[1015]}, &belt_2_shift2 };
iC_Gt belt_2_shift2_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_2_shift2_2", {&iC_l_[1020]}, {&iC_l_[1022]}, &belt_2_shift2_1 };
iC_Gt belt_2_shift2_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_2_shift2_3", {0}, {&iC_l_[1025]}, &belt_2_shift2_2 };
iC_Gt belt_2_shift2_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_shift2_4", {0}, {&iC_l_[1028]}, &belt_2_shift2_3 };
iC_Gt belt_2_shift3 = { 1, -iC_SH, iC_ARITH, 0, "belt_2_shift3", {0}, {0}, &belt_2_shift2_4 };
iC_Gt belt_2_shift3_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_2_shift3_1", {&iC_l_[1032]}, {&iC_l_[1034]}, &belt_2_shift3 };
iC_Gt belt_2_shift3_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_2_shift3_2", {&iC_l_[1039]}, {&iC_l_[1041]}, &belt_2_shift3_1 };
iC_Gt belt_2_shift3_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_2_shift3_3", {0}, {&iC_l_[1044]}, &belt_2_shift3_2 };
iC_Gt belt_2_shift3_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_shift3_4", {0}, {&iC_l_[1047]}, &belt_2_shift3_3 };
iC_Gt belt_2_shift4 = { 1, -iC_SH, iC_ARITH, 0, "belt_2_shift4", {0}, {0}, &belt_2_shift3_4 };
iC_Gt belt_2_shift4_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_2_shift4_1", {&iC_l_[1051]}, {&iC_l_[1053]}, &belt_2_shift4 };
iC_Gt belt_2_shift4_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_2_shift4_2", {&iC_l_[1058]}, {&iC_l_[1060]}, &belt_2_shift4_1 };
iC_Gt belt_2_shift4_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_2_shift4_3", {0}, {&iC_l_[1063]}, &belt_2_shift4_2 };
iC_Gt belt_2_shift4_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_shift4_4", {0}, {&iC_l_[1066]}, &belt_2_shift4_3 };
iC_Gt belt_2_shift5 = { 1, -iC_SH, iC_ARITH, 0, "belt_2_shift5", {0}, {0}, &belt_2_shift4_4 };
iC_Gt belt_2_shift5_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_2_shift5_1", {&iC_l_[1070]}, {&iC_l_[1072]}, &belt_2_shift5 };
iC_Gt belt_2_shift5_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_2_shift5_2", {&iC_l_[1077]}, {&iC_l_[1079]}, &belt_2_shift5_1 };
iC_Gt belt_2_shift5_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_2_shift5_3", {0}, {&iC_l_[1082]}, &belt_2_shift5_2 };
iC_Gt belt_2_shift5_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_shift5_4", {0}, {&iC_l_[1085]}, &belt_2_shift5_3 };
iC_Gt belt_2_shift6 = { 1, -iC_SH, iC_ARITH, 0, "belt_2_shift6", {0}, {0}, &belt_2_shift5_4 };
iC_Gt belt_2_shift6_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_2_shift6_1", {&iC_l_[1089]}, {&iC_l_[1091]}, &belt_2_shift6 };
iC_Gt belt_2_shift6_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_2_shift6_2", {&iC_l_[1096]}, {&iC_l_[1098]}, &belt_2_shift6_1 };
iC_Gt belt_2_shift6_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_2_shift6_3", {0}, {&iC_l_[1101]}, &belt_2_shift6_2 };
iC_Gt belt_2_shift6_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_shift6_4", {0}, {&iC_l_[1104]}, &belt_2_shift6_3 };
iC_Gt belt_2_shift7 = { 1, -iC_SH, iC_ARITH, 0, "belt_2_shift7", {0}, {0}, &belt_2_shift6_4 };
iC_Gt belt_2_shift7_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_2_shift7_1", {&iC_l_[1108]}, {&iC_l_[1110]}, &belt_2_shift7 };
iC_Gt belt_2_shift7_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_2_shift7_2", {&iC_l_[1115]}, {&iC_l_[1117]}, &belt_2_shift7_1 };
iC_Gt belt_2_shift7_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_2_shift7_3", {0}, {&iC_l_[1120]}, &belt_2_shift7_2 };
iC_Gt belt_2_shift7_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_shift7_4", {0}, {&iC_l_[1123]}, &belt_2_shift7_3 };
iC_Gt belt_2_shift8 = { 1, -iC_SH, iC_ARITH, 0, "belt_2_shift8", {0}, {0}, &belt_2_shift7_4 };
iC_Gt belt_2_shift8_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_2_shift8_1", {&iC_l_[1127]}, {&iC_l_[1129]}, &belt_2_shift8 };
iC_Gt belt_2_shift8_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_2_shift8_2", {&iC_l_[1134]}, {&iC_l_[1136]}, &belt_2_shift8_1 };
iC_Gt belt_2_shift8_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_2_shift8_3", {0}, {&iC_l_[1139]}, &belt_2_shift8_2 };
iC_Gt belt_2_shift8_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_shift8_4", {0}, {&iC_l_[1142]}, &belt_2_shift8_3 };
iC_Gt belt_2_shift9 = { 1, -iC_SH, iC_ARITH, 0, "belt_2_shift9", {0}, {0}, &belt_2_shift8_4 };
iC_Gt belt_2_shift9_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_2_shift9_1", {&iC_l_[1146]}, {&iC_l_[1148]}, &belt_2_shift9 };
iC_Gt belt_2_shift9_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_2_shift9_2", {&iC_l_[1153]}, {&iC_l_[1155]}, &belt_2_shift9_1 };
iC_Gt belt_2_shift9_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_2_shift9_3", {0}, {&iC_l_[1158]}, &belt_2_shift9_2 };
iC_Gt belt_2_shift9_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_shift9_4", {0}, {&iC_l_[1161]}, &belt_2_shift9_3 };
iC_Gt belt_2_shift_1 = { 1, -iC_ARN, iC_D_SH, 0, "belt_2_shift_1", {&iC_l_[1165]}, {&iC_l_[1167]}, &belt_2_shift9_4 };
iC_Gt belt_2_shift_2 = { 1, -iC_OR, iC_R_SH, 0, "belt_2_shift_2", {&iC_l_[1172]}, {&iC_l_[1174]}, &belt_2_shift_1 };
iC_Gt belt_2_shift_3 = { 1, -iC_OR, iC_ARITH, 0, "belt_2_shift_3", {0}, {&iC_l_[1177]}, &belt_2_shift_2 };
iC_Gt belt_2_shift_4 = { 1, -iC_ARN, iC_ARITH, 0, "belt_2_shift_4", {0}, {&iC_l_[1180]}, &belt_2_shift_3 };
iC_Gt belt_2_shift_5 = { 1, -iC_OR, iC_ARITH, 0, "belt_2_shift_5", {0}, {&iC_l_[1185]}, &belt_2_shift_4 };
extern iC_Gt iConst; /* 22 */
iC_Gt tick_1_fast = { 1, -iC_OR, iC_GATE, 0, "tick_1_fast", {0}, {&iC_l_[1188]}, &belt_2_shift_5 };
iC_Gt tick_2_fast = { 1, -iC_OR, iC_GATE, 0, "tick_2_fast", {0}, {&iC_l_[1192]}, &tick_1_fast };
iC_Gt QX4_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.2", {0}, {(iC_Gt**)&belt_1_pip_2}, &tick_2_fast, 0 };
iC_Gt QX4_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX4.3", {0}, {(iC_Gt**)&belt_2_pip_2}, &QX4_2, 0 };
iC_Gt reset    = { 1, -iC_ALIAS, iC_GATE, 0, "reset", {0}, {(iC_Gt**)&IX0_7}, &QX4_3, 0 };

iC_Gt *		iC___Test0_sort32u_list = &reset;
iC_Gt **	iC_list[] = { &iC___Test0_sort32u_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_1(iC_Gt * iC_gf) {
#line 139 "init_t.ic"	/* in pre-compiled function block SHR */
	return iC_MV(1)?iC_MV(2):iC_MV(3);
#line 381 "./Test0/sort32u.c"
} /* iC_1 */

static int iC_2(iC_Gt * iC_gf) {
#line 30 "./Test0/sort32u.ic"
	return (iC_MV(1)<<1)+(iC_MV(2)*(0x41+(iC_MV(3)<<1)));
#line 387 "./Test0/sort32u.c"
} /* iC_2 */

static int iC_5(iC_Gt * iC_gf) {
#line 34 "./Test0/sort32u.ic"
	return (iC_MV(1)&iC_MV(2))==iC_MV(2);
#line 393 "./Test0/sort32u.c"
} /* iC_5 */

static int iC_6(iC_Gt * iC_gf) {
#line 55 "./Test0/sort32u.ic"
	return (iC_MV(1)<<1)+iC_MV(2);
#line 399 "./Test0/sort32u.c"
} /* iC_6 */

static int iC_8(iC_Gt * iC_gf) {
#line 101 "./Test0/sort32u.ic"
	return 0x41<<11;
#line 405 "./Test0/sort32u.c"
} /* iC_8 */

static int iC_9(iC_Gt * iC_gf) {
#line 101 "./Test0/sort32u.ic"
	return (iC_MV(1)>>12)&1;
#line 411 "./Test0/sort32u.c"
} /* iC_9 */

static int iC_10(iC_Gt * iC_gf) {
#line 102 "./Test0/sort32u.ic"
	return 0x41<<7;
#line 417 "./Test0/sort32u.c"
} /* iC_10 */

static int iC_11(iC_Gt * iC_gf) {
#line 102 "./Test0/sort32u.ic"
	return (iC_MV(1)&0x7f)==0x41+(0<<1);
#line 423 "./Test0/sort32u.c"
} /* iC_11 */

static int iC_12(iC_Gt * iC_gf) {
#line 103 "./Test0/sort32u.ic"
	return (iC_MV(1)&0x7f)==0x41+(1<<1);
#line 429 "./Test0/sort32u.c"
} /* iC_12 */

static int iC_13(iC_Gt * iC_gf) {
#line 104 "./Test0/sort32u.ic"
	return (iC_MV(1)&0x7f)==0x41+(2<<1);
#line 435 "./Test0/sort32u.c"
} /* iC_13 */

static int iC_14(iC_Gt * iC_gf) {
#line 105 "./Test0/sort32u.ic"
	return (iC_MV(1)&0x7f)==0x41+(3<<1);
#line 441 "./Test0/sort32u.c"
} /* iC_14 */

static int iC_15(iC_Gt * iC_gf) {
#line 106 "./Test0/sort32u.ic"
	return (iC_MV(1)&0x7f)==0x41+(4<<1);
#line 447 "./Test0/sort32u.c"
} /* iC_15 */

static int iC_16(iC_Gt * iC_gf) {
#line 107 "./Test0/sort32u.ic"
	return (iC_MV(1)&0x7f)==0x41+(5<<1);
#line 453 "./Test0/sort32u.c"
} /* iC_16 */

static int iC_17(iC_Gt * iC_gf) {
#line 108 "./Test0/sort32u.ic"
	return (iC_MV(1)&0x7f)==0x41+(6<<1);
#line 459 "./Test0/sort32u.c"
} /* iC_17 */

static int iC_18(iC_Gt * iC_gf) {
#line 109 "./Test0/sort32u.ic"
	return (iC_MV(1)&0x7f)==0x41+(7<<1);
#line 465 "./Test0/sort32u.c"
} /* iC_18 */

static int iC_19(iC_Gt * iC_gf) {
#line 110 "./Test0/sort32u.ic"
	return (iC_MV(1)&0x7f)==0x41+(8<<1);
#line 471 "./Test0/sort32u.c"
} /* iC_19 */

static int iC_20(iC_Gt * iC_gf) {
#line 111 "./Test0/sort32u.ic"
	return (iC_MV(1)&0x7f)==0x41+(9<<1);
#line 477 "./Test0/sort32u.c"
} /* iC_20 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QX0.0_0 */	&QX0_0, 0, 0,
/* QX0.0_1 */	&QX0_0, &belt_1_clk,	&QX0_0_3, 0, &QX0_0_4, 0,
/* QX0.0_2 */	&QX0_0, &belt_1_clk,	&QX0_0_4, 0, &QX0_0_3, 0,
/* QX0.0_3 */	(iC_Gt*)iC_11, &belt_1_shift1, 0,
/* QX0.0_4 */	&QX0_0_5, &IX0_7, 0, 0,
/* QX0.0_5 */	(iC_Gt*)iC_5, &belt_1_shift1, &belt_1_mask1, 0,
/* QX0.1_0 */	&QX0_1, 0, 0,
/* QX0.1_1 */	&QX0_1, &belt_1_clk,	&QX0_1_3, 0, &QX0_1_4, 0,
/* QX0.1_2 */	&QX0_1, &belt_1_clk,	&QX0_1_4, 0, &QX0_1_3, 0,
/* QX0.1_3 */	(iC_Gt*)iC_12, &belt_1_shift2, 0,
/* QX0.1_4 */	&QX0_1_5, &IX0_7, 0, 0,
/* QX0.1_5 */	(iC_Gt*)iC_5, &belt_1_shift2, &belt_1_mask2, 0,
/* QX0.2_0 */	&QX0_2, 0, 0,
/* QX0.2_1 */	&QX0_2, &belt_1_clk,	&QX0_2_3, 0, &QX0_2_4, 0,
/* QX0.2_2 */	&QX0_2, &belt_1_clk,	&QX0_2_4, 0, &QX0_2_3, 0,
/* QX0.2_3 */	(iC_Gt*)iC_13, &belt_1_shift3, 0,
/* QX0.2_4 */	&QX0_2_5, &IX0_7, 0, 0,
/* QX0.2_5 */	(iC_Gt*)iC_5, &belt_1_shift3, &belt_1_mask3, 0,
/* QX0.3_0 */	&QX0_3, 0, 0,
/* QX0.3_1 */	&QX0_3, &belt_1_clk,	&QX0_3_3, 0, &QX0_3_4, 0,
/* QX0.3_2 */	&QX0_3, &belt_1_clk,	&QX0_3_4, 0, &QX0_3_3, 0,
/* QX0.3_3 */	(iC_Gt*)iC_14, &belt_1_shift4, 0,
/* QX0.3_4 */	&QX0_3_5, &IX0_7, 0, 0,
/* QX0.3_5 */	(iC_Gt*)iC_5, &belt_1_shift4, &belt_1_mask4, 0,
/* QX0.4_0 */	&QX0_4, 0, 0,
/* QX0.4_1 */	&QX0_4, &belt_1_clk,	&QX0_4_3, 0, &QX0_4_4, 0,
/* QX0.4_2 */	&QX0_4, &belt_1_clk,	&QX0_4_4, 0, &QX0_4_3, 0,
/* QX0.4_3 */	(iC_Gt*)iC_15, &belt_1_shift5, 0,
/* QX0.4_4 */	&QX0_4_5, &IX0_7, 0, 0,
/* QX0.4_5 */	(iC_Gt*)iC_5, &belt_1_shift5, &belt_1_mask5, 0,
/* QX0.5_0 */	&QX0_5, 0, 0,
/* QX0.5_1 */	&QX0_5, &belt_1_clk,	&QX0_5_3, 0, &QX0_5_4, 0,
/* QX0.5_2 */	&QX0_5, &belt_1_clk,	&QX0_5_4, 0, &QX0_5_3, 0,
/* QX0.5_3 */	(iC_Gt*)iC_16, &belt_1_shift6, 0,
/* QX0.5_4 */	&QX0_5_5, &IX0_7, 0, 0,
/* QX0.5_5 */	(iC_Gt*)iC_5, &belt_1_shift6, &belt_1_mask6, 0,
/* QX0.6_0 */	&QX0_6, 0, 0,
/* QX0.6_1 */	&QX0_6, &belt_1_clk,	&QX0_6_3, 0, &QX0_6_4, 0,
/* QX0.6_2 */	&QX0_6, &belt_1_clk,	&QX0_6_4, 0, &QX0_6_3, 0,
/* QX0.6_3 */	(iC_Gt*)iC_17, &belt_1_shift7, 0,
/* QX0.6_4 */	&QX0_6_5, &IX0_7, 0, 0,
/* QX0.6_5 */	(iC_Gt*)iC_5, &belt_1_shift7, &belt_1_mask7, 0,
/* QX0.7_0 */	&QX0_7, 0, 0,
/* QX0.7_1 */	&QX0_7, &belt_1_clk,	&QX0_7_3, 0, &QX0_7_4, 0,
/* QX0.7_2 */	&QX0_7, &belt_1_clk,	&QX0_7_4, 0, &QX0_7_3, 0,
/* QX0.7_3 */	(iC_Gt*)iC_18, &belt_1_shift8, 0,
/* QX0.7_4 */	&QX0_7_5, &IX0_7, 0, 0,
/* QX0.7_5 */	(iC_Gt*)iC_5, &belt_1_shift8, &belt_1_mask8, 0,
/* QX1.0_0 */	&QX1_0, 0, 0,
/* QX1.0_1 */	&QX1_0, &belt_1_clk,	&QX1_0_3, 0, &QX1_0_4, 0,
/* QX1.0_2 */	&QX1_0, &belt_1_clk,	&QX1_0_4, 0, &QX1_0_3, 0,
/* QX1.0_3 */	(iC_Gt*)iC_19, &belt_1_shift9, 0,
/* QX1.0_4 */	&QX1_0_5, &IX0_7, 0, 0,
/* QX1.0_5 */	(iC_Gt*)iC_5, &belt_1_shift9, &belt_1_mask9, 0,
/* QX1.1_0 */	&QX1_1, 0, 0,
/* QX1.1_1 */	&QX1_1, &belt_1_clk,	&QX1_1_3, 0, &QX1_1_4, 0,
/* QX1.1_2 */	&QX1_1, &belt_1_clk,	&QX1_1_4, 0, &QX1_1_3, 0,
/* QX1.1_3 */	(iC_Gt*)iC_20, &belt_1_shift10, 0,
/* QX1.1_4 */	&QX1_1_5, &IX0_7, 0, 0,
/* QX1.1_5 */	(iC_Gt*)iC_5, &belt_1_shift10, &belt_1_mask10, 0,
/* QX2.0_0 */	&QX2_0, 0, 0,
/* QX2.0_1 */	&QX2_0, &belt_2_clk,	&QX2_0_3, 0, &QX2_0_4, 0,
/* QX2.0_2 */	&QX2_0, &belt_2_clk,	&QX2_0_4, 0, &QX2_0_3, 0,
/* QX2.0_3 */	(iC_Gt*)iC_11, &belt_2_shift1, 0,
/* QX2.0_4 */	&QX2_0_5, &IX0_7, 0, 0,
/* QX2.0_5 */	(iC_Gt*)iC_5, &belt_2_shift1, &belt_2_mask1, 0,
/* QX2.1_0 */	&QX2_1, 0, 0,
/* QX2.1_1 */	&QX2_1, &belt_2_clk,	&QX2_1_3, 0, &QX2_1_4, 0,
/* QX2.1_2 */	&QX2_1, &belt_2_clk,	&QX2_1_4, 0, &QX2_1_3, 0,
/* QX2.1_3 */	(iC_Gt*)iC_12, &belt_2_shift2, 0,
/* QX2.1_4 */	&QX2_1_5, &IX0_7, 0, 0,
/* QX2.1_5 */	(iC_Gt*)iC_5, &belt_2_shift2, &belt_2_mask2, 0,
/* QX2.2_0 */	&QX2_2, 0, 0,
/* QX2.2_1 */	&QX2_2, &belt_2_clk,	&QX2_2_3, 0, &QX2_2_4, 0,
/* QX2.2_2 */	&QX2_2, &belt_2_clk,	&QX2_2_4, 0, &QX2_2_3, 0,
/* QX2.2_3 */	(iC_Gt*)iC_13, &belt_2_shift3, 0,
/* QX2.2_4 */	&QX2_2_5, &IX0_7, 0, 0,
/* QX2.2_5 */	(iC_Gt*)iC_5, &belt_2_shift3, &belt_2_mask3, 0,
/* QX2.3_0 */	&QX2_3, 0, 0,
/* QX2.3_1 */	&QX2_3, &belt_2_clk,	&QX2_3_3, 0, &QX2_3_4, 0,
/* QX2.3_2 */	&QX2_3, &belt_2_clk,	&QX2_3_4, 0, &QX2_3_3, 0,
/* QX2.3_3 */	(iC_Gt*)iC_14, &belt_2_shift4, 0,
/* QX2.3_4 */	&QX2_3_5, &IX0_7, 0, 0,
/* QX2.3_5 */	(iC_Gt*)iC_5, &belt_2_shift4, &belt_2_mask4, 0,
/* QX2.4_0 */	&QX2_4, 0, 0,
/* QX2.4_1 */	&QX2_4, &belt_2_clk,	&QX2_4_3, 0, &QX2_4_4, 0,
/* QX2.4_2 */	&QX2_4, &belt_2_clk,	&QX2_4_4, 0, &QX2_4_3, 0,
/* QX2.4_3 */	(iC_Gt*)iC_15, &belt_2_shift5, 0,
/* QX2.4_4 */	&QX2_4_5, &IX0_7, 0, 0,
/* QX2.4_5 */	(iC_Gt*)iC_5, &belt_2_shift5, &belt_2_mask5, 0,
/* QX2.5_0 */	&QX2_5, 0, 0,
/* QX2.5_1 */	&QX2_5, &belt_2_clk,	&QX2_5_3, 0, &QX2_5_4, 0,
/* QX2.5_2 */	&QX2_5, &belt_2_clk,	&QX2_5_4, 0, &QX2_5_3, 0,
/* QX2.5_3 */	(iC_Gt*)iC_16, &belt_2_shift6, 0,
/* QX2.5_4 */	&QX2_5_5, &IX0_7, 0, 0,
/* QX2.5_5 */	(iC_Gt*)iC_5, &belt_2_shift6, &belt_2_mask6, 0,
/* QX2.6_0 */	&QX2_6, 0, 0,
/* QX2.6_1 */	&QX2_6, &belt_2_clk,	&QX2_6_3, 0, &QX2_6_4, 0,
/* QX2.6_2 */	&QX2_6, &belt_2_clk,	&QX2_6_4, 0, &QX2_6_3, 0,
/* QX2.6_3 */	(iC_Gt*)iC_17, &belt_2_shift7, 0,
/* QX2.6_4 */	&QX2_6_5, &IX0_7, 0, 0,
/* QX2.6_5 */	(iC_Gt*)iC_5, &belt_2_shift7, &belt_2_mask7, 0,
/* QX2.7_0 */	&QX2_7, 0, 0,
/* QX2.7_1 */	&QX2_7, &belt_2_clk,	&QX2_7_3, 0, &QX2_7_4, 0,
/* QX2.7_2 */	&QX2_7, &belt_2_clk,	&QX2_7_4, 0, &QX2_7_3, 0,
/* QX2.7_3 */	(iC_Gt*)iC_18, &belt_2_shift8, 0,
/* QX2.7_4 */	&QX2_7_5, &IX0_7, 0, 0,
/* QX2.7_5 */	(iC_Gt*)iC_5, &belt_2_shift8, &belt_2_mask8, 0,
/* QX3.0_0 */	&QX3_0, 0, 0,
/* QX3.0_1 */	&QX3_0, &belt_2_clk,	&QX3_0_3, 0, &QX3_0_4, 0,
/* QX3.0_2 */	&QX3_0, &belt_2_clk,	&QX3_0_4, 0, &QX3_0_3, 0,
/* QX3.0_3 */	(iC_Gt*)iC_19, &belt_2_shift9, 0,
/* QX3.0_4 */	&QX3_0_5, &IX0_7, 0, 0,
/* QX3.0_5 */	(iC_Gt*)iC_5, &belt_2_shift9, &belt_2_mask9, 0,
/* QX3.1_0 */	&QX3_1, 0, 0,
/* QX3.1_1 */	&QX3_1, &belt_2_clk,	&QX3_1_3, 0, &QX3_1_4, 0,
/* QX3.1_2 */	&QX3_1, &belt_2_clk,	&QX3_1_4, 0, &QX3_1_3, 0,
/* QX3.1_3 */	(iC_Gt*)iC_20, &belt_2_shift10, 0,
/* QX3.1_4 */	&QX3_1_5, &IX0_7, 0, 0,
/* QX3.1_5 */	(iC_Gt*)iC_5, &belt_2_shift10, &belt_2_mask10, 0,
/* QX4.0 */	(iC_Gt*)0, &belt_1_car10, 0,
/* QX4.0_0 */	&QX4_0, 0, 0,
/* QX4.1 */	(iC_Gt*)0, &belt_2_car10, 0,
/* QX4.1_0 */	&QX4_1, 0, 0,
/* QX4.2_0 */	&belt_1_pip_2, 0, 0,
/* QX4.3_0 */	&belt_2_pip_2, 0, 0,
/* belt_1_car0 */	(iC_Gt*)iC_9, &belt_1_shift, 0,
/* belt_1_car1 */	(iC_Gt*)iC_9, &belt_1_shift1, 0,
/* belt_1_car10 */	(iC_Gt*)iC_9, &belt_1_shift10, 0,
/* belt_1_car2 */	(iC_Gt*)iC_9, &belt_1_shift2, 0,
/* belt_1_car3 */	(iC_Gt*)iC_9, &belt_1_shift3, 0,
/* belt_1_car4 */	(iC_Gt*)iC_9, &belt_1_shift4, 0,
/* belt_1_car5 */	(iC_Gt*)iC_9, &belt_1_shift5, 0,
/* belt_1_car6 */	(iC_Gt*)iC_9, &belt_1_shift6, 0,
/* belt_1_car7 */	(iC_Gt*)iC_9, &belt_1_shift7, 0,
/* belt_1_car8 */	(iC_Gt*)iC_9, &belt_1_shift8, 0,
/* belt_1_car9 */	(iC_Gt*)iC_9, &belt_1_shift9, 0,
/* belt_1_clk_1 */	&belt_1_clk, &iClock,	&belt_1_clk_2, 0, 0,
/* belt_1_clk_3 */	&belt_1_clk_2, &iClock,	&belt_1_clk_5,
		&belt_1_clk_4, 0, 0,
/* belt_1_clk_4 */	&IX0_0, 0, &tick_1_fast, 0,
/* belt_1_clk_5 */	&tick_1_fast, &TX0_4, 0, 0,
/* belt_1_mask */	(iC_Gt*)iC_8, &iConst, 0,
/* belt_1_mask1 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_1_mask10 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_1_mask2 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_1_mask3 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_1_mask4 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_1_mask5 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_1_mask6 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_1_mask7 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_1_mask8 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_1_mask9 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_1_pip_1 */	&belt_1_pip, &belt_1_clk,	&IX0_1, 0,
		&belt_1_pip_2, &IX0_7, 0,
/* belt_1_pip_3 */	&belt_1_pip_2, &belt_1_clk,	&belt_1_pip, 0,
		&belt_1_pip_5, 0,
/* belt_1_pip_4 */	&belt_1_pip_2, &belt_1_clk,	&belt_1_pip_5, 0,
		&belt_1_pip, 0,
/* belt_1_pip_5 */	&belt_1_pip_6, &IX0_7, 0, 0,
/* belt_1_pip_6 */	(iC_Gt*)iC_5, &belt_1_shift, &belt_1_mask, 0,
/* belt_1_shift10_1 */	&belt_1_shift10, &belt_1_clk,	(iC_Gt*)iC_1,
		&belt_1_shift10_3, &belt_1_shift10, &belt_1_shift10_4, 0,
/* belt_1_shift10_2 */	&belt_1_shift10, &iClock,	&IX0_7, 0, 0,
/* belt_1_shift10_3 */	&IX0_7, 0, 0,
/* belt_1_shift10_4 */	(iC_Gt*)iC_6, &belt_1_shift10, &belt_1_car9, 0,
/* belt_1_shift1_1 */	&belt_1_shift1, &belt_1_clk,	(iC_Gt*)iC_1,
		&belt_1_shift1_3, &belt_1_shift1, &belt_1_shift1_4, 0,
/* belt_1_shift1_2 */	&belt_1_shift1, &iClock,	&IX0_7, 0, 0,
/* belt_1_shift1_3 */	&IX0_7, 0, 0,
/* belt_1_shift1_4 */	(iC_Gt*)iC_6, &belt_1_shift1, &belt_1_car0, 0,
/* belt_1_shift2_1 */	&belt_1_shift2, &belt_1_clk,	(iC_Gt*)iC_1,
		&belt_1_shift2_3, &belt_1_shift2, &belt_1_shift2_4, 0,
/* belt_1_shift2_2 */	&belt_1_shift2, &iClock,	&IX0_7, 0, 0,
/* belt_1_shift2_3 */	&IX0_7, 0, 0,
/* belt_1_shift2_4 */	(iC_Gt*)iC_6, &belt_1_shift2, &belt_1_car1, 0,
/* belt_1_shift3_1 */	&belt_1_shift3, &belt_1_clk,	(iC_Gt*)iC_1,
		&belt_1_shift3_3, &belt_1_shift3, &belt_1_shift3_4, 0,
/* belt_1_shift3_2 */	&belt_1_shift3, &iClock,	&IX0_7, 0, 0,
/* belt_1_shift3_3 */	&IX0_7, 0, 0,
/* belt_1_shift3_4 */	(iC_Gt*)iC_6, &belt_1_shift3, &belt_1_car2, 0,
/* belt_1_shift4_1 */	&belt_1_shift4, &belt_1_clk,	(iC_Gt*)iC_1,
		&belt_1_shift4_3, &belt_1_shift4, &belt_1_shift4_4, 0,
/* belt_1_shift4_2 */	&belt_1_shift4, &iClock,	&IX0_7, 0, 0,
/* belt_1_shift4_3 */	&IX0_7, 0, 0,
/* belt_1_shift4_4 */	(iC_Gt*)iC_6, &belt_1_shift4, &belt_1_car3, 0,
/* belt_1_shift5_1 */	&belt_1_shift5, &belt_1_clk,	(iC_Gt*)iC_1,
		&belt_1_shift5_3, &belt_1_shift5, &belt_1_shift5_4, 0,
/* belt_1_shift5_2 */	&belt_1_shift5, &iClock,	&IX0_7, 0, 0,
/* belt_1_shift5_3 */	&IX0_7, 0, 0,
/* belt_1_shift5_4 */	(iC_Gt*)iC_6, &belt_1_shift5, &belt_1_car4, 0,
/* belt_1_shift6_1 */	&belt_1_shift6, &belt_1_clk,	(iC_Gt*)iC_1,
		&belt_1_shift6_3, &belt_1_shift6, &belt_1_shift6_4, 0,
/* belt_1_shift6_2 */	&belt_1_shift6, &iClock,	&IX0_7, 0, 0,
/* belt_1_shift6_3 */	&IX0_7, 0, 0,
/* belt_1_shift6_4 */	(iC_Gt*)iC_6, &belt_1_shift6, &belt_1_car5, 0,
/* belt_1_shift7_1 */	&belt_1_shift7, &belt_1_clk,	(iC_Gt*)iC_1,
		&belt_1_shift7_3, &belt_1_shift7, &belt_1_shift7_4, 0,
/* belt_1_shift7_2 */	&belt_1_shift7, &iClock,	&IX0_7, 0, 0,
/* belt_1_shift7_3 */	&IX0_7, 0, 0,
/* belt_1_shift7_4 */	(iC_Gt*)iC_6, &belt_1_shift7, &belt_1_car6, 0,
/* belt_1_shift8_1 */	&belt_1_shift8, &belt_1_clk,	(iC_Gt*)iC_1,
		&belt_1_shift8_3, &belt_1_shift8, &belt_1_shift8_4, 0,
/* belt_1_shift8_2 */	&belt_1_shift8, &iClock,	&IX0_7, 0, 0,
/* belt_1_shift8_3 */	&IX0_7, 0, 0,
/* belt_1_shift8_4 */	(iC_Gt*)iC_6, &belt_1_shift8, &belt_1_car7, 0,
/* belt_1_shift9_1 */	&belt_1_shift9, &belt_1_clk,	(iC_Gt*)iC_1,
		&belt_1_shift9_3, &belt_1_shift9, &belt_1_shift9_4, 0,
/* belt_1_shift9_2 */	&belt_1_shift9, &iClock,	&IX0_7, 0, 0,
/* belt_1_shift9_3 */	&IX0_7, 0, 0,
/* belt_1_shift9_4 */	(iC_Gt*)iC_6, &belt_1_shift9, &belt_1_car8, 0,
/* belt_1_shift_1 */	&belt_1_shift, &belt_1_clk,	(iC_Gt*)iC_1,
		&belt_1_shift_3, &belt_1_shift, &belt_1_shift_4, 0,
/* belt_1_shift_2 */	&belt_1_shift, &iClock,	&IX0_7, 0, 0,
/* belt_1_shift_3 */	&IX0_7, 0, 0,
/* belt_1_shift_4 */	(iC_Gt*)iC_2, &belt_1_shift, &belt_1_shift_5, &IB1, 0,
/* belt_1_shift_5 */	&belt_1_pip, 0, 0,
/* belt_2_car0 */	(iC_Gt*)iC_9, &belt_2_shift, 0,
/* belt_2_car1 */	(iC_Gt*)iC_9, &belt_2_shift1, 0,
/* belt_2_car10 */	(iC_Gt*)iC_9, &belt_2_shift10, 0,
/* belt_2_car2 */	(iC_Gt*)iC_9, &belt_2_shift2, 0,
/* belt_2_car3 */	(iC_Gt*)iC_9, &belt_2_shift3, 0,
/* belt_2_car4 */	(iC_Gt*)iC_9, &belt_2_shift4, 0,
/* belt_2_car5 */	(iC_Gt*)iC_9, &belt_2_shift5, 0,
/* belt_2_car6 */	(iC_Gt*)iC_9, &belt_2_shift6, 0,
/* belt_2_car7 */	(iC_Gt*)iC_9, &belt_2_shift7, 0,
/* belt_2_car8 */	(iC_Gt*)iC_9, &belt_2_shift8, 0,
/* belt_2_car9 */	(iC_Gt*)iC_9, &belt_2_shift9, 0,
/* belt_2_clk_1 */	&belt_2_clk, &iClock,	&belt_2_clk_2, 0, 0,
/* belt_2_clk_3 */	&belt_2_clk_2, &iClock,	&belt_2_clk_5,
		&belt_2_clk_4, 0, 0,
/* belt_2_clk_4 */	&IX2_0, 0, &tick_2_fast, 0,
/* belt_2_clk_5 */	&tick_2_fast, &TX0_4, 0, 0,
/* belt_2_mask */	(iC_Gt*)iC_8, &iConst, 0,
/* belt_2_mask1 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_2_mask10 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_2_mask2 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_2_mask3 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_2_mask4 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_2_mask5 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_2_mask6 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_2_mask7 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_2_mask8 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_2_mask9 */	(iC_Gt*)iC_10, &iConst, 0,
/* belt_2_pip_1 */	&belt_2_pip, &belt_2_clk,	&IX2_1, 0,
		&belt_2_pip_2, &IX0_7, 0,
/* belt_2_pip_3 */	&belt_2_pip_2, &belt_2_clk,	&belt_2_pip, 0,
		&belt_2_pip_5, 0,
/* belt_2_pip_4 */	&belt_2_pip_2, &belt_2_clk,	&belt_2_pip_5, 0,
		&belt_2_pip, 0,
/* belt_2_pip_5 */	&belt_2_pip_6, &IX0_7, 0, 0,
/* belt_2_pip_6 */	(iC_Gt*)iC_5, &belt_2_shift, &belt_2_mask, 0,
/* belt_2_shift10_1 */	&belt_2_shift10, &belt_2_clk,	(iC_Gt*)iC_1,
		&belt_2_shift10_3, &belt_2_shift10, &belt_2_shift10_4, 0,
/* belt_2_shift10_2 */	&belt_2_shift10, &iClock,	&IX0_7, 0, 0,
/* belt_2_shift10_3 */	&IX0_7, 0, 0,
/* belt_2_shift10_4 */	(iC_Gt*)iC_6, &belt_2_shift10, &belt_2_car9, 0,
/* belt_2_shift1_1 */	&belt_2_shift1, &belt_2_clk,	(iC_Gt*)iC_1,
		&belt_2_shift1_3, &belt_2_shift1, &belt_2_shift1_4, 0,
/* belt_2_shift1_2 */	&belt_2_shift1, &iClock,	&IX0_7, 0, 0,
/* belt_2_shift1_3 */	&IX0_7, 0, 0,
/* belt_2_shift1_4 */	(iC_Gt*)iC_6, &belt_2_shift1, &belt_2_car0, 0,
/* belt_2_shift2_1 */	&belt_2_shift2, &belt_2_clk,	(iC_Gt*)iC_1,
		&belt_2_shift2_3, &belt_2_shift2, &belt_2_shift2_4, 0,
/* belt_2_shift2_2 */	&belt_2_shift2, &iClock,	&IX0_7, 0, 0,
/* belt_2_shift2_3 */	&IX0_7, 0, 0,
/* belt_2_shift2_4 */	(iC_Gt*)iC_6, &belt_2_shift2, &belt_2_car1, 0,
/* belt_2_shift3_1 */	&belt_2_shift3, &belt_2_clk,	(iC_Gt*)iC_1,
		&belt_2_shift3_3, &belt_2_shift3, &belt_2_shift3_4, 0,
/* belt_2_shift3_2 */	&belt_2_shift3, &iClock,	&IX0_7, 0, 0,
/* belt_2_shift3_3 */	&IX0_7, 0, 0,
/* belt_2_shift3_4 */	(iC_Gt*)iC_6, &belt_2_shift3, &belt_2_car2, 0,
/* belt_2_shift4_1 */	&belt_2_shift4, &belt_2_clk,	(iC_Gt*)iC_1,
		&belt_2_shift4_3, &belt_2_shift4, &belt_2_shift4_4, 0,
/* belt_2_shift4_2 */	&belt_2_shift4, &iClock,	&IX0_7, 0, 0,
/* belt_2_shift4_3 */	&IX0_7, 0, 0,
/* belt_2_shift4_4 */	(iC_Gt*)iC_6, &belt_2_shift4, &belt_2_car3, 0,
/* belt_2_shift5_1 */	&belt_2_shift5, &belt_2_clk,	(iC_Gt*)iC_1,
		&belt_2_shift5_3, &belt_2_shift5, &belt_2_shift5_4, 0,
/* belt_2_shift5_2 */	&belt_2_shift5, &iClock,	&IX0_7, 0, 0,
/* belt_2_shift5_3 */	&IX0_7, 0, 0,
/* belt_2_shift5_4 */	(iC_Gt*)iC_6, &belt_2_shift5, &belt_2_car4, 0,
/* belt_2_shift6_1 */	&belt_2_shift6, &belt_2_clk,	(iC_Gt*)iC_1,
		&belt_2_shift6_3, &belt_2_shift6, &belt_2_shift6_4, 0,
/* belt_2_shift6_2 */	&belt_2_shift6, &iClock,	&IX0_7, 0, 0,
/* belt_2_shift6_3 */	&IX0_7, 0, 0,
/* belt_2_shift6_4 */	(iC_Gt*)iC_6, &belt_2_shift6, &belt_2_car5, 0,
/* belt_2_shift7_1 */	&belt_2_shift7, &belt_2_clk,	(iC_Gt*)iC_1,
		&belt_2_shift7_3, &belt_2_shift7, &belt_2_shift7_4, 0,
/* belt_2_shift7_2 */	&belt_2_shift7, &iClock,	&IX0_7, 0, 0,
/* belt_2_shift7_3 */	&IX0_7, 0, 0,
/* belt_2_shift7_4 */	(iC_Gt*)iC_6, &belt_2_shift7, &belt_2_car6, 0,
/* belt_2_shift8_1 */	&belt_2_shift8, &belt_2_clk,	(iC_Gt*)iC_1,
		&belt_2_shift8_3, &belt_2_shift8, &belt_2_shift8_4, 0,
/* belt_2_shift8_2 */	&belt_2_shift8, &iClock,	&IX0_7, 0, 0,
/* belt_2_shift8_3 */	&IX0_7, 0, 0,
/* belt_2_shift8_4 */	(iC_Gt*)iC_6, &belt_2_shift8, &belt_2_car7, 0,
/* belt_2_shift9_1 */	&belt_2_shift9, &belt_2_clk,	(iC_Gt*)iC_1,
		&belt_2_shift9_3, &belt_2_shift9, &belt_2_shift9_4, 0,
/* belt_2_shift9_2 */	&belt_2_shift9, &iClock,	&IX0_7, 0, 0,
/* belt_2_shift9_3 */	&IX0_7, 0, 0,
/* belt_2_shift9_4 */	(iC_Gt*)iC_6, &belt_2_shift9, &belt_2_car8, 0,
/* belt_2_shift_1 */	&belt_2_shift, &belt_2_clk,	(iC_Gt*)iC_1,
		&belt_2_shift_3, &belt_2_shift, &belt_2_shift_4, 0,
/* belt_2_shift_2 */	&belt_2_shift, &iClock,	&IX0_7, 0, 0,
/* belt_2_shift_3 */	&IX0_7, 0, 0,
/* belt_2_shift_4 */	(iC_Gt*)iC_2, &belt_2_shift, &belt_2_shift_5, &IB3, 0,
/* belt_2_shift_5 */	&belt_2_pip, 0, 0,
/* tick_1_fast */	&IX0_6, &IX0_5, 0, 0,
/* tick_2_fast */	&IX2_6, &IX0_5, 0, 0,
};

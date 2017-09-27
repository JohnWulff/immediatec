/********************************************************************
 *
 *	SOURCE:   ./Test0/compTest2a.ic
 *	OUTPUT:   ./Test0/compTest2a.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"$Revision: icc_3.1-5-gaaeb885-dirty $ -O7";

#include	<icg.h>

#error "iC: Action gate 'QX0.6_1' controlled by timer '1' has no clock"

#error "iC: Action gate 'QX0.7_1' controlled by timer '3' has no clock"

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

static iC_Gt _1      = { 1, -iC_NCONST, iC_ARITH, 0, "1", {0}, {0}, 0 };
static iC_Gt _3      = { 1, -iC_NCONST, iC_ARITH, 0, "3", {0}, {0}, &_1 };
iC_Gt DSR_10_i = { 1, -iC_OR, iC_GATE, 0, "DSR_10_i", {0}, {&iC_l_[0]}, &_3 };
iC_Gt DSR_10_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_10_i_1", {0}, {0}, &DSR_10_i };
iC_Gt DSR_10_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_10_i_2", {&iC_l_[4]}, {&iC_l_[6]}, &DSR_10_i_1 };
iC_Gt DSR_11_i = { 1, -iC_OR, iC_GATE, 0, "DSR_11_i", {0}, {&iC_l_[9]}, &DSR_10_i_2 };
iC_Gt DSR_11_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_11_i_1", {0}, {0}, &DSR_11_i };
iC_Gt DSR_11_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_11_i_2", {&iC_l_[13]}, {&iC_l_[15]}, &DSR_11_i_1 };
iC_Gt DSR_12_i = { 1, -iC_OR, iC_GATE, 0, "DSR_12_i", {0}, {&iC_l_[18]}, &DSR_11_i_2 };
iC_Gt DSR_12_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_12_i_1", {0}, {0}, &DSR_12_i };
iC_Gt DSR_12_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_12_i_2", {&iC_l_[22]}, {&iC_l_[24]}, &DSR_12_i_1 };
iC_Gt DSR_13_i = { 1, -iC_OR, iC_GATE, 0, "DSR_13_i", {0}, {&iC_l_[27]}, &DSR_12_i_2 };
iC_Gt DSR_13_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_13_i_1", {0}, {0}, &DSR_13_i };
iC_Gt DSR_13_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_13_i_2", {&iC_l_[31]},
#error "iC: Action gate 'DSR_13_i_2' has no valid clock"
 {&iC_l_[34]}, &DSR_13_i_1 };
iC_Gt DSR_14_i = { 1, -iC_OR, iC_GATE, 0, "DSR_14_i", {0}, {&iC_l_[37]}, &DSR_13_i_2 };
iC_Gt DSR_14_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_14_i_1", {0}, {0}, &DSR_14_i };
iC_Gt DSR_14_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_14_i_2", {&iC_l_[41]},
#error "iC: Action gate 'DSR_14_i_2' has no valid clock"
 {&iC_l_[44]}, &DSR_14_i_1 };
iC_Gt DSR_15_i = { 1, -iC_OR, iC_GATE, 0, "DSR_15_i", {0}, {&iC_l_[47]}, &DSR_14_i_2 };
iC_Gt DSR_15_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_15_i_1", {0}, {0}, &DSR_15_i };
iC_Gt DSR_15_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_15_i_2", {&iC_l_[51]},
#error "iC: Action gate 'DSR_15_i_2' has no valid clock"
 {&iC_l_[54]}, &DSR_15_i_1 };
iC_Gt DSR_16_i = { 1, -iC_OR, iC_GATE, 0, "DSR_16_i", {0}, {&iC_l_[57]}, &DSR_15_i_2 };
iC_Gt DSR_16_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_16_i_1", {0}, {0}, &DSR_16_i };
iC_Gt DSR_16_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_16_i_2", {&iC_l_[61]},
#error "iC: Action gate 'DSR_16_i_2' has no valid clock"
 {&iC_l_[64]}, &DSR_16_i_1 };
iC_Gt DSR_17_i = { 1, -iC_OR, iC_GATE, 0, "DSR_17_i", {0}, {&iC_l_[67]}, &DSR_16_i_2 };
iC_Gt DSR_17_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_17_i_1", {0}, {0}, &DSR_17_i };
iC_Gt DSR_17_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_17_i_2", {&iC_l_[71]}, {&iC_l_[73]}, &DSR_17_i_1 };
iC_Gt DSR_18_i = { 1, -iC_OR, iC_GATE, 0, "DSR_18_i", {0}, {&iC_l_[76]}, &DSR_17_i_2 };
iC_Gt DSR_18_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_18_i_1", {0}, {0}, &DSR_18_i };
iC_Gt DSR_18_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_18_i_2", {&iC_l_[80]},
#error "iC: Action gate 'DSR_18_i_2' has no valid clock"
 {&iC_l_[82]}, &DSR_18_i_1 };
iC_Gt DSR_19_i = { 1, -iC_OR, iC_GATE, 0, "DSR_19_i", {0}, {&iC_l_[85]}, &DSR_18_i_2 };
iC_Gt DSR_19_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_19_i_1", {0}, {0}, &DSR_19_i };
iC_Gt DSR_19_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_19_i_2", {&iC_l_[89]},
#error "iC: Action gate 'DSR_19_i_2' has no valid clock"
 {&iC_l_[91]}, &DSR_19_i_1 };
iC_Gt DSR_1_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_1_i", {0}, {&iC_l_[94]}, &DSR_19_i_2 };
iC_Gt DSR_1_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_1_i_1", {0}, {0}, &DSR_1_i };
iC_Gt DSR_1_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_1_i_2", {&iC_l_[98]}, {&iC_l_[100]}, &DSR_1_i_1 };
iC_Gt DSR_20_i = { 1, -iC_OR, iC_GATE, 0, "DSR_20_i", {0}, {&iC_l_[103]}, &DSR_1_i_2 };
iC_Gt DSR_20_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_20_i_1", {0}, {0}, &DSR_20_i };
iC_Gt DSR_20_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_20_i_2", {&iC_l_[107]},
#error "iC: Action gate 'DSR_20_i_2' has no valid clock"
 {&iC_l_[109]}, &DSR_20_i_1 };
iC_Gt DSR_21_i = { 1, -iC_OR, iC_GATE, 0, "DSR_21_i", {0}, {&iC_l_[112]}, &DSR_20_i_2 };
iC_Gt DSR_21_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_21_i_1", {0}, {0}, &DSR_21_i };
iC_Gt DSR_21_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_21_i_2", {&iC_l_[116]}, {&iC_l_[118]}, &DSR_21_i_1 };
iC_Gt DSR_22_i = { 1, -iC_OR, iC_GATE, 0, "DSR_22_i", {0}, {&iC_l_[121]}, &DSR_21_i_2 };
iC_Gt DSR_22_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_22_i_1", {0}, {0}, &DSR_22_i };
iC_Gt DSR_22_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_22_i_2", {&iC_l_[125]}, {&iC_l_[127]}, &DSR_22_i_1 };
iC_Gt DSR_23_i = { 1, -iC_OR, iC_GATE, 0, "DSR_23_i", {0}, {&iC_l_[130]}, &DSR_22_i_2 };
iC_Gt DSR_23_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_23_i_1", {0}, {0}, &DSR_23_i };
iC_Gt DSR_23_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_23_i_2", {&iC_l_[134]}, {&iC_l_[136]}, &DSR_23_i_1 };
iC_Gt DSR_24_i = { 1, -iC_OR, iC_GATE, 0, "DSR_24_i", {0}, {&iC_l_[139]}, &DSR_23_i_2 };
iC_Gt DSR_24_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_24_i_1", {0}, {0}, &DSR_24_i };
iC_Gt DSR_24_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_24_i_2", {&iC_l_[143]}, {&iC_l_[145]}, &DSR_24_i_1 };
iC_Gt DSR_25_i = { 1, -iC_OR, iC_GATE, 0, "DSR_25_i", {0}, {&iC_l_[148]}, &DSR_24_i_2 };
iC_Gt DSR_25_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_25_i_1", {0}, {0}, &DSR_25_i };
iC_Gt DSR_25_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_25_i_2", {&iC_l_[152]},
#error "iC: Action gate 'DSR_25_i_2' has no valid clock"
 {&iC_l_[155]}, &DSR_25_i_1 };
iC_Gt DSR_26_i = { 1, -iC_OR, iC_GATE, 0, "DSR_26_i", {0}, {&iC_l_[158]}, &DSR_25_i_2 };
iC_Gt DSR_26_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_26_i_1", {0}, {0}, &DSR_26_i };
iC_Gt DSR_26_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_26_i_2", {&iC_l_[162]},
#error "iC: Action gate 'DSR_26_i_2' has no valid clock"
 {&iC_l_[165]}, &DSR_26_i_1 };
iC_Gt DSR_27_i = { 1, -iC_OR, iC_GATE, 0, "DSR_27_i", {0}, {&iC_l_[168]}, &DSR_26_i_2 };
iC_Gt DSR_27_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_27_i_1", {0}, {0}, &DSR_27_i };
iC_Gt DSR_27_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_27_i_2", {&iC_l_[172]},
#error "iC: Action gate 'DSR_27_i_2' has no valid clock"
 {&iC_l_[175]}, &DSR_27_i_1 };
iC_Gt DSR_28_i = { 1, -iC_OR, iC_GATE, 0, "DSR_28_i", {0}, {&iC_l_[178]}, &DSR_27_i_2 };
iC_Gt DSR_28_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_28_i_1", {0}, {0}, &DSR_28_i };
iC_Gt DSR_28_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_28_i_2", {&iC_l_[182]},
#error "iC: Action gate 'DSR_28_i_2' has no valid clock"
 {&iC_l_[185]}, &DSR_28_i_1 };
iC_Gt DSR_2_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_2_i", {0}, {&iC_l_[188]}, &DSR_28_i_2 };
iC_Gt DSR_2_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_2_i_1", {0}, {0}, &DSR_2_i };
iC_Gt DSR_2_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_2_i_2", {&iC_l_[192]},
#error "iC: Action gate 'DSR_2_i_2' has no valid clock"
 {&iC_l_[194]}, &DSR_2_i_1 };
iC_Gt DSR_3_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_3_i", {0}, {&iC_l_[197]}, &DSR_2_i_2 };
iC_Gt DSR_3_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_3_i_1", {0}, {0}, &DSR_3_i };
iC_Gt DSR_3_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_3_i_2", {&iC_l_[201]},
#error "iC: Action gate 'DSR_3_i_2' has no valid clock"
 {&iC_l_[203]}, &DSR_3_i_1 };
iC_Gt DSR_4_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_4_i", {0}, {&iC_l_[206]}, &DSR_3_i_2 };
iC_Gt DSR_4_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_4_i_1", {0}, {0}, &DSR_4_i };
iC_Gt DSR_4_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_4_i_2", {&iC_l_[210]},
#error "iC: Action gate 'DSR_4_i_2' has no valid clock"
 {&iC_l_[212]}, &DSR_4_i_1 };
iC_Gt DSR_5_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_5_i", {0}, {&iC_l_[215]}, &DSR_4_i_2 };
iC_Gt DSR_5_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_5_i_1", {0}, {0}, &DSR_5_i };
iC_Gt DSR_5_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_5_i_2", {&iC_l_[219]}, {&iC_l_[221]}, &DSR_5_i_1 };
iC_Gt DSR_6_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_6_i", {0}, {&iC_l_[224]}, &DSR_5_i_2 };
iC_Gt DSR_6_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_6_i_1", {0}, {0}, &DSR_6_i };
iC_Gt DSR_6_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_6_i_2", {&iC_l_[228]},
#error "iC: Action gate 'DSR_6_i_2' has no valid clock"
 {&iC_l_[230]}, &DSR_6_i_1 };
iC_Gt DSR_7_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_7_i", {0}, {&iC_l_[233]}, &DSR_6_i_2 };
iC_Gt DSR_7_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_7_i_1", {0}, {0}, &DSR_7_i };
iC_Gt DSR_7_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_7_i_2", {&iC_l_[237]},
#error "iC: Action gate 'DSR_7_i_2' has no valid clock"
 {&iC_l_[239]}, &DSR_7_i_1 };
iC_Gt DSR_8_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_8_i", {0}, {&iC_l_[242]}, &DSR_7_i_2 };
iC_Gt DSR_8_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_8_i_1", {0}, {0}, &DSR_8_i };
iC_Gt DSR_8_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_8_i_2", {&iC_l_[246]},
#error "iC: Action gate 'DSR_8_i_2' has no valid clock"
 {&iC_l_[248]}, &DSR_8_i_1 };
iC_Gt DSR_9_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_9_i", {0}, {&iC_l_[251]}, &DSR_8_i_2 };
iC_Gt DSR_9_i_1 = { 1, -iC_FF, iC_GATE, 0, "DSR_9_i_1", {0}, {0}, &DSR_9_i };
iC_Gt DSR_9_i_2 = { 1, -iC_OR, iC_D_FF, 0, "DSR_9_i_2", {&iC_l_[255]}, {&iC_l_[257]}, &DSR_9_i_1 };
iC_Gt IB16     = { 1, -iC_INPW, iC_ARITH, 0, "IB16", {0}, {0}, &DSR_9_i_2 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB16 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
iC_Gt IX0_4    = { 1, -iC_INPX, iC_GATE, 0, "IX0.4", {0}, {0}, &IX0_3 };
iC_Gt IX0_5    = { 1, -iC_INPX, iC_GATE, 0, "IX0.5", {0}, {0}, &IX0_4 };
iC_Gt IX0_6    = { 1, -iC_INPX, iC_GATE, 0, "IX0.6", {0}, {0}, &IX0_5 };
iC_Gt QB16     = { 1, -iC_SH, iC_ARITH, 0, "QB16", {0}, {0}, &IX0_6 };
iC_Gt QB16_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB16_0", {0}, {&iC_l_[260]}, &QB16 };
iC_Gt QB16_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB16_1", {&iC_l_[263]}, {&iC_l_[265]}, &QB16_0 };
iC_Gt QB17     = { 1, -iC_SH, iC_ARITH, 0, "QB17", {0}, {0}, &QB16_1 };
iC_Gt QB17_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB17_0", {0}, {&iC_l_[268]}, &QB17 };
iC_Gt QB17_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB17_1", {&iC_l_[271]},
#error "iC: Action gate 'QB17_1' has no valid clock"
 {&iC_l_[273]}, &QB17_0 };
iC_Gt QB18     = { 1, -iC_SH, iC_ARITH, 0, "QB18", {0}, {0}, &QB17_1 };
iC_Gt QB18_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB18_0", {0}, {&iC_l_[276]}, &QB18 };
iC_Gt QB18_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB18_1", {&iC_l_[279]},
#error "iC: Action gate 'QB18_1' has no valid clock"
 {&iC_l_[281]}, &QB18_0 };
iC_Gt QB19     = { 1, -iC_SH, iC_ARITH, 0, "QB19", {0}, {0}, &QB18_1 };
iC_Gt QB19_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB19_0", {0}, {&iC_l_[284]}, &QB19 };
iC_Gt QB19_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB19_1", {&iC_l_[287]},
#error "iC: Action gate 'QB19_1' has no valid clock"
 {&iC_l_[289]}, &QB19_0 };
iC_Gt QB20     = { 1, -iC_SH, iC_ARITH, 0, "QB20", {0}, {0}, &QB19_1 };
iC_Gt QB20_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB20_0", {0}, {&iC_l_[292]}, &QB20 };
iC_Gt QB20_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB20_1", {&iC_l_[295]}, {&iC_l_[297]}, &QB20_0 };
iC_Gt QB20_2   = { 1, -iC_OR, iC_R_SH, 0, "QB20_2", {&iC_l_[302]}, {&iC_l_[304]}, &QB20_1 };
iC_Gt QB20_3   = { 1, -iC_OR, iC_ARITH, 0, "QB20_3", {0}, {&iC_l_[307]}, &QB20_2 };
iC_Gt QB21     = { 1, -iC_SH, iC_ARITH, 0, "QB21", {0}, {0}, &QB20_3 };
iC_Gt QB21_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB21_0", {0}, {&iC_l_[310]}, &QB21 };
iC_Gt QB21_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB21_1", {&iC_l_[313]},
#error "iC: Action gate 'QB21_1' has no valid clock"
 {&iC_l_[315]}, &QB21_0 };
iC_Gt QB21_2   = { 1, -iC_OR, iC_R_SH, 0, "QB21_2", {&iC_l_[320]},
#error "iC: Action gate 'QB21_2' has no valid clock"
 {&iC_l_[322]}, &QB21_1 };
iC_Gt QB21_3   = { 1, -iC_OR, iC_ARITH, 0, "QB21_3", {0}, {&iC_l_[325]}, &QB21_2 };
iC_Gt QB22     = { 1, -iC_SH, iC_ARITH, 0, "QB22", {0}, {0}, &QB21_3 };
iC_Gt QB22_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB22_0", {0}, {&iC_l_[328]}, &QB22 };
iC_Gt QB22_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB22_1", {&iC_l_[331]},
#error "iC: Action gate 'QB22_1' has no valid clock"
 {&iC_l_[333]}, &QB22_0 };
iC_Gt QB22_2   = { 1, -iC_OR, iC_R_SH, 0, "QB22_2", {&iC_l_[338]},
#error "iC: Action gate 'QB22_2' has no valid clock"
 {&iC_l_[340]}, &QB22_1 };
iC_Gt QB22_3   = { 1, -iC_OR, iC_ARITH, 0, "QB22_3", {0}, {&iC_l_[343]}, &QB22_2 };
iC_Gt QB23     = { 1, -iC_SH, iC_ARITH, 0, "QB23", {0}, {0}, &QB22_3 };
iC_Gt QB23_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB23_0", {0}, {&iC_l_[346]}, &QB23 };
iC_Gt QB23_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB23_1", {&iC_l_[349]},
#error "iC: Action gate 'QB23_1' has no valid clock"
 {&iC_l_[351]}, &QB23_0 };
iC_Gt QB23_2   = { 1, -iC_OR, iC_R_SH, 0, "QB23_2", {&iC_l_[356]},
#error "iC: Action gate 'QB23_2' has no valid clock"
 {&iC_l_[358]}, &QB23_1 };
iC_Gt QB23_3   = { 1, -iC_OR, iC_ARITH, 0, "QB23_3", {0}, {&iC_l_[361]}, &QB23_2 };
iC_Gt QB24     = { 1, -iC_SH, iC_ARITH, 0, "QB24", {0}, {0}, &QB23_3 };
iC_Gt QB24_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB24_0", {0}, {&iC_l_[364]}, &QB24 };
iC_Gt QB24_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB24_1", {&iC_l_[367]},
#error "iC: Action gate 'QB24_1' has no valid clock"
 {&iC_l_[369]}, &QB24_0 };
iC_Gt QB24_2   = { 1, -iC_OR, iC_R_SH, 0, "QB24_2", {&iC_l_[374]}, {&iC_l_[376]}, &QB24_1 };
iC_Gt QB24_3   = { 1, -iC_OR, iC_ARITH, 0, "QB24_3", {0}, {&iC_l_[379]}, &QB24_2 };
iC_Gt QB25     = { 1, -iC_SH, iC_ARITH, 0, "QB25", {0}, {0}, &QB24_3 };
iC_Gt QB25_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB25_0", {0}, {&iC_l_[382]}, &QB25 };
iC_Gt QB25_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB25_1", {&iC_l_[385]},
#error "iC: Action gate 'QB25_1' has no valid clock"
 {&iC_l_[387]}, &QB25_0 };
iC_Gt QB25_2   = { 1, -iC_OR, iC_R_SH, 0, "QB25_2", {&iC_l_[392]},
#error "iC: Action gate 'QB25_2' has no valid clock"
 {&iC_l_[394]}, &QB25_1 };
iC_Gt QB25_3   = { 1, -iC_OR, iC_ARITH, 0, "QB25_3", {0}, {&iC_l_[397]}, &QB25_2 };
iC_Gt QB26     = { 1, -iC_SH, iC_ARITH, 0, "QB26", {0}, {0}, &QB25_3 };
iC_Gt QB26_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB26_0", {0}, {&iC_l_[400]}, &QB26 };
iC_Gt QB26_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB26_1", {&iC_l_[403]},
#error "iC: Action gate 'QB26_1' has no valid clock"
 {&iC_l_[405]}, &QB26_0 };
iC_Gt QB26_2   = { 1, -iC_OR, iC_R_SH, 0, "QB26_2", {&iC_l_[410]},
#error "iC: Action gate 'QB26_2' has no valid clock"
 {&iC_l_[412]}, &QB26_1 };
iC_Gt QB26_3   = { 1, -iC_OR, iC_ARITH, 0, "QB26_3", {0}, {&iC_l_[415]}, &QB26_2 };
iC_Gt QB27     = { 1, -iC_SH, iC_ARITH, 0, "QB27", {0}, {0}, &QB26_3 };
iC_Gt QB27_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB27_0", {0}, {&iC_l_[418]}, &QB27 };
iC_Gt QB27_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB27_1", {&iC_l_[421]},
#error "iC: Action gate 'QB27_1' has no valid clock"
 {&iC_l_[423]}, &QB27_0 };
iC_Gt QB27_2   = { 1, -iC_OR, iC_R_SH, 0, "QB27_2", {&iC_l_[428]},
#error "iC: Action gate 'QB27_2' has no valid clock"
 {&iC_l_[430]}, &QB27_1 };
iC_Gt QB27_3   = { 1, -iC_OR, iC_ARITH, 0, "QB27_3", {0}, {&iC_l_[433]}, &QB27_2 };
iC_Gt QB28     = { 1, -iC_SH, iC_ARITH, 0, "QB28", {0}, {0}, &QB27_3 };
iC_Gt QB28_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB28_0", {0}, {&iC_l_[436]}, &QB28 };
iC_Gt QB28_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB28_1", {&iC_l_[439]},
#error "iC: Action gate 'QB28_1' has no valid clock"
 {&iC_l_[441]}, &QB28_0 };
iC_Gt QB28_2   = { 1, -iC_OR, iC_R_SH, 0, "QB28_2", {&iC_l_[446]}, {&iC_l_[448]}, &QB28_1 };
iC_Gt QB28_3   = { 1, -iC_OR, iC_ARITH, 0, "QB28_3", {0}, {&iC_l_[451]}, &QB28_2 };
iC_Gt QB29     = { 1, -iC_SH, iC_ARITH, 0, "QB29", {0}, {0}, &QB28_3 };
iC_Gt QB29_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB29_0", {0}, {&iC_l_[454]}, &QB29 };
iC_Gt QB29_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB29_1", {&iC_l_[457]},
#error "iC: Action gate 'QB29_1' has no valid clock"
 {&iC_l_[459]}, &QB29_0 };
iC_Gt QB29_2   = { 1, -iC_OR, iC_R_SH, 0, "QB29_2", {&iC_l_[464]},
#error "iC: Action gate 'QB29_2' has no valid clock"
 {&iC_l_[466]}, &QB29_1 };
iC_Gt QB29_3   = { 1, -iC_OR, iC_ARITH, 0, "QB29_3", {0}, {&iC_l_[469]}, &QB29_2 };
iC_Gt QB30     = { 1, -iC_SH, iC_ARITH, 0, "QB30", {0}, {0}, &QB29_3 };
iC_Gt QB30_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB30_0", {0}, {&iC_l_[472]}, &QB30 };
iC_Gt QB30_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB30_1", {&iC_l_[475]},
#error "iC: Action gate 'QB30_1' has no valid clock"
 {&iC_l_[477]}, &QB30_0 };
iC_Gt QB30_2   = { 1, -iC_OR, iC_R_SH, 0, "QB30_2", {&iC_l_[482]},
#error "iC: Action gate 'QB30_2' has no valid clock"
 {&iC_l_[484]}, &QB30_1 };
iC_Gt QB30_3   = { 1, -iC_OR, iC_ARITH, 0, "QB30_3", {0}, {&iC_l_[487]}, &QB30_2 };
iC_Gt QB31     = { 1, -iC_SH, iC_ARITH, 0, "QB31", {0}, {0}, &QB30_3 };
iC_Gt QB31_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB31_0", {0}, {&iC_l_[490]}, &QB31 };
iC_Gt QB31_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB31_1", {&iC_l_[493]},
#error "iC: Action gate 'QB31_1' has no valid clock"
 {&iC_l_[495]}, &QB31_0 };
iC_Gt QB31_2   = { 1, -iC_OR, iC_R_SH, 0, "QB31_2", {&iC_l_[500]},
#error "iC: Action gate 'QB31_2' has no valid clock"
 {&iC_l_[502]}, &QB31_1 };
iC_Gt QB31_3   = { 1, -iC_OR, iC_ARITH, 0, "QB31_3", {0}, {&iC_l_[505]}, &QB31_2 };
iC_Gt QB32     = { 1, -iC_SH, iC_ARITH, 0, "QB32", {0}, {0}, &QB31_3 };
iC_Gt QB32_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB32_0", {0}, {&iC_l_[508]}, &QB32 };
iC_Gt QB32_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB32_1", {&iC_l_[511]}, {&iC_l_[513]}, &QB32_0 };
iC_Gt QB32_2   = { 1, -iC_AND, iC_S_SH, 0, "QB32_2", {&iC_l_[518]}, {&iC_l_[520]}, &QB32_1 };
iC_Gt QB32_3   = { 1, -iC_AND, iC_R_SH, 0, "QB32_3", {&iC_l_[524]}, {&iC_l_[526]}, &QB32_2 };
iC_Gt QB32_4   = { 1, -iC_OR, iC_ARITH, 0, "QB32_4", {0}, {&iC_l_[530]}, &QB32_3 };
iC_Gt QB33     = { 1, -iC_SH, iC_ARITH, 0, "QB33", {0}, {0}, &QB32_4 };
iC_Gt QB33_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB33_0", {0}, {&iC_l_[534]}, &QB33 };
iC_Gt QB33_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB33_1", {&iC_l_[537]},
#error "iC: Action gate 'QB33_1' has no valid clock"
 {&iC_l_[539]}, &QB33_0 };
iC_Gt QB33_2   = { 1, -iC_AND, iC_S_SH, 0, "QB33_2", {&iC_l_[544]},
#error "iC: Action gate 'QB33_2' has no valid clock"
 {&iC_l_[546]}, &QB33_1 };
iC_Gt QB33_3   = { 1, -iC_AND, iC_R_SH, 0, "QB33_3", {&iC_l_[550]},
#error "iC: Action gate 'QB33_3' has no valid clock"
 {&iC_l_[552]}, &QB33_2 };
iC_Gt QB33_4   = { 1, -iC_OR, iC_ARITH, 0, "QB33_4", {0}, {&iC_l_[556]}, &QB33_3 };
iC_Gt QB34     = { 1, -iC_SH, iC_ARITH, 0, "QB34", {0}, {0}, &QB33_4 };
iC_Gt QB34_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB34_0", {0}, {&iC_l_[560]}, &QB34 };
iC_Gt QB34_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB34_1", {&iC_l_[563]},
#error "iC: Action gate 'QB34_1' has no valid clock"
 {&iC_l_[565]}, &QB34_0 };
iC_Gt QB34_2   = { 1, -iC_AND, iC_S_SH, 0, "QB34_2", {&iC_l_[570]},
#error "iC: Action gate 'QB34_2' has no valid clock"
 {&iC_l_[572]}, &QB34_1 };
iC_Gt QB34_3   = { 1, -iC_AND, iC_R_SH, 0, "QB34_3", {&iC_l_[576]},
#error "iC: Action gate 'QB34_3' has no valid clock"
 {&iC_l_[578]}, &QB34_2 };
iC_Gt QB34_4   = { 1, -iC_OR, iC_ARITH, 0, "QB34_4", {0}, {&iC_l_[582]}, &QB34_3 };
iC_Gt QB35     = { 1, -iC_SH, iC_ARITH, 0, "QB35", {0}, {0}, &QB34_4 };
iC_Gt QB35_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB35_0", {0}, {&iC_l_[586]}, &QB35 };
iC_Gt QB35_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB35_1", {&iC_l_[589]},
#error "iC: Action gate 'QB35_1' has no valid clock"
 {&iC_l_[591]}, &QB35_0 };
iC_Gt QB35_2   = { 1, -iC_AND, iC_S_SH, 0, "QB35_2", {&iC_l_[596]},
#error "iC: Action gate 'QB35_2' has no valid clock"
 {&iC_l_[598]}, &QB35_1 };
iC_Gt QB35_3   = { 1, -iC_AND, iC_R_SH, 0, "QB35_3", {&iC_l_[602]},
#error "iC: Action gate 'QB35_3' has no valid clock"
 {&iC_l_[604]}, &QB35_2 };
iC_Gt QB35_4   = { 1, -iC_OR, iC_ARITH, 0, "QB35_4", {0}, {&iC_l_[608]}, &QB35_3 };
iC_Gt QB36     = { 1, -iC_SH, iC_ARITH, 0, "QB36", {0}, {0}, &QB35_4 };
iC_Gt QB36_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB36_0", {0}, {&iC_l_[612]}, &QB36 };
iC_Gt QB36_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB36_1", {&iC_l_[615]},
#error "iC: Action gate 'QB36_1' has no valid clock"
 {&iC_l_[617]}, &QB36_0 };
iC_Gt QB36_2   = { 1, -iC_AND, iC_S_SH, 0, "QB36_2", {&iC_l_[622]}, {&iC_l_[624]}, &QB36_1 };
iC_Gt QB36_3   = { 1, -iC_AND, iC_R_SH, 0, "QB36_3", {&iC_l_[628]}, {&iC_l_[630]}, &QB36_2 };
iC_Gt QB36_4   = { 1, -iC_OR, iC_ARITH, 0, "QB36_4", {0}, {&iC_l_[634]}, &QB36_3 };
iC_Gt QB37     = { 1, -iC_SH, iC_ARITH, 0, "QB37", {0}, {0}, &QB36_4 };
iC_Gt QB37_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB37_0", {0}, {&iC_l_[638]}, &QB37 };
iC_Gt QB37_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB37_1", {&iC_l_[641]},
#error "iC: Action gate 'QB37_1' has no valid clock"
 {&iC_l_[643]}, &QB37_0 };
iC_Gt QB37_2   = { 1, -iC_AND, iC_S_SH, 0, "QB37_2", {&iC_l_[648]},
#error "iC: Action gate 'QB37_2' has no valid clock"
 {&iC_l_[650]}, &QB37_1 };
iC_Gt QB37_3   = { 1, -iC_AND, iC_R_SH, 0, "QB37_3", {&iC_l_[654]},
#error "iC: Action gate 'QB37_3' has no valid clock"
 {&iC_l_[656]}, &QB37_2 };
iC_Gt QB37_4   = { 1, -iC_OR, iC_ARITH, 0, "QB37_4", {0}, {&iC_l_[660]}, &QB37_3 };
iC_Gt QB38     = { 1, -iC_SH, iC_ARITH, 0, "QB38", {0}, {0}, &QB37_4 };
iC_Gt QB38_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB38_0", {0}, {&iC_l_[664]}, &QB38 };
iC_Gt QB38_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB38_1", {&iC_l_[667]},
#error "iC: Action gate 'QB38_1' has no valid clock"
 {&iC_l_[669]}, &QB38_0 };
iC_Gt QB38_2   = { 1, -iC_AND, iC_S_SH, 0, "QB38_2", {&iC_l_[674]},
#error "iC: Action gate 'QB38_2' has no valid clock"
 {&iC_l_[676]}, &QB38_1 };
iC_Gt QB38_3   = { 1, -iC_AND, iC_R_SH, 0, "QB38_3", {&iC_l_[680]},
#error "iC: Action gate 'QB38_3' has no valid clock"
 {&iC_l_[682]}, &QB38_2 };
iC_Gt QB38_4   = { 1, -iC_OR, iC_ARITH, 0, "QB38_4", {0}, {&iC_l_[686]}, &QB38_3 };
iC_Gt QB39     = { 1, -iC_SH, iC_ARITH, 0, "QB39", {0}, {0}, &QB38_4 };
iC_Gt QB39_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB39_0", {0}, {&iC_l_[690]}, &QB39 };
iC_Gt QB39_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB39_1", {&iC_l_[693]},
#error "iC: Action gate 'QB39_1' has no valid clock"
 {&iC_l_[695]}, &QB39_0 };
iC_Gt QB39_2   = { 1, -iC_AND, iC_S_SH, 0, "QB39_2", {&iC_l_[700]},
#error "iC: Action gate 'QB39_2' has no valid clock"
 {&iC_l_[702]}, &QB39_1 };
iC_Gt QB39_3   = { 1, -iC_AND, iC_R_SH, 0, "QB39_3", {&iC_l_[706]},
#error "iC: Action gate 'QB39_3' has no valid clock"
 {&iC_l_[708]}, &QB39_2 };
iC_Gt QB39_4   = { 1, -iC_OR, iC_ARITH, 0, "QB39_4", {0}, {&iC_l_[712]}, &QB39_3 };
iC_Gt QB40     = { 1, -iC_SH, iC_ARITH, 0, "QB40", {0}, {0}, &QB39_4 };
iC_Gt QB40_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB40_0", {0}, {&iC_l_[716]}, &QB40 };
iC_Gt QB40_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB40_1", {&iC_l_[719]},
#error "iC: Action gate 'QB40_1' has no valid clock"
 {&iC_l_[721]}, &QB40_0 };
iC_Gt QB40_2   = { 1, -iC_AND, iC_S_SH, 0, "QB40_2", {&iC_l_[726]}, {&iC_l_[728]}, &QB40_1 };
iC_Gt QB40_3   = { 1, -iC_AND, iC_R_SH, 0, "QB40_3", {&iC_l_[732]}, {&iC_l_[734]}, &QB40_2 };
iC_Gt QB40_4   = { 1, -iC_OR, iC_ARITH, 0, "QB40_4", {0}, {&iC_l_[738]}, &QB40_3 };
iC_Gt QB41     = { 1, -iC_SH, iC_ARITH, 0, "QB41", {0}, {0}, &QB40_4 };
iC_Gt QB41_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB41_0", {0}, {&iC_l_[742]}, &QB41 };
iC_Gt QB41_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB41_1", {&iC_l_[745]},
#error "iC: Action gate 'QB41_1' has no valid clock"
 {&iC_l_[747]}, &QB41_0 };
iC_Gt QB41_2   = { 1, -iC_AND, iC_S_SH, 0, "QB41_2", {&iC_l_[752]}, {&iC_l_[754]}, &QB41_1 };
iC_Gt QB41_3   = { 1, -iC_AND, iC_R_SH, 0, "QB41_3", {&iC_l_[758]},
#error "iC: Action gate 'QB41_3' has no valid clock"
 {&iC_l_[760]}, &QB41_2 };
iC_Gt QB41_4   = { 1, -iC_OR, iC_ARITH, 0, "QB41_4", {0}, {&iC_l_[764]}, &QB41_3 };
iC_Gt QB42     = { 1, -iC_SH, iC_ARITH, 0, "QB42", {0}, {0}, &QB41_4 };
iC_Gt QB42_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB42_0", {0}, {&iC_l_[768]}, &QB42 };
iC_Gt QB42_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB42_1", {&iC_l_[771]},
#error "iC: Action gate 'QB42_1' has no valid clock"
 {&iC_l_[773]}, &QB42_0 };
iC_Gt QB42_2   = { 1, -iC_AND, iC_S_SH, 0, "QB42_2", {&iC_l_[778]}, {&iC_l_[780]}, &QB42_1 };
iC_Gt QB42_3   = { 1, -iC_AND, iC_R_SH, 0, "QB42_3", {&iC_l_[784]},
#error "iC: Action gate 'QB42_3' has no valid clock"
 {&iC_l_[786]}, &QB42_2 };
iC_Gt QB42_4   = { 1, -iC_OR, iC_ARITH, 0, "QB42_4", {0}, {&iC_l_[790]}, &QB42_3 };
iC_Gt QB43     = { 1, -iC_SH, iC_ARITH, 0, "QB43", {0}, {0}, &QB42_4 };
iC_Gt QB43_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB43_0", {0}, {&iC_l_[794]}, &QB43 };
iC_Gt QB43_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB43_1", {&iC_l_[797]},
#error "iC: Action gate 'QB43_1' has no valid clock"
 {&iC_l_[799]}, &QB43_0 };
iC_Gt QB43_2   = { 1, -iC_AND, iC_S_SH, 0, "QB43_2", {&iC_l_[804]}, {&iC_l_[806]}, &QB43_1 };
iC_Gt QB43_3   = { 1, -iC_AND, iC_R_SH, 0, "QB43_3", {&iC_l_[810]},
#error "iC: Action gate 'QB43_3' has no valid clock"
 {&iC_l_[812]}, &QB43_2 };
iC_Gt QB43_4   = { 1, -iC_OR, iC_ARITH, 0, "QB43_4", {0}, {&iC_l_[816]}, &QB43_3 };
iC_Gt QB44     = { 1, -iC_SH, iC_ARITH, 0, "QB44", {0}, {0}, &QB43_4 };
iC_Gt QB44_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB44_0", {0}, {&iC_l_[820]}, &QB44 };
iC_Gt QB44_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB44_1", {&iC_l_[823]},
#error "iC: Action gate 'QB44_1' has no valid clock"
 {&iC_l_[825]}, &QB44_0 };
iC_Gt QB44_2   = { 1, -iC_AND, iC_S_SH, 0, "QB44_2", {&iC_l_[830]},
#error "iC: Action gate 'QB44_2' has no valid clock"
 {&iC_l_[833]}, &QB44_1 };
iC_Gt QB44_3   = { 1, -iC_AND, iC_R_SH, 0, "QB44_3", {&iC_l_[837]}, {&iC_l_[839]}, &QB44_2 };
iC_Gt QB44_4   = { 1, -iC_OR, iC_ARITH, 0, "QB44_4", {0}, {&iC_l_[843]}, &QB44_3 };
iC_Gt QB45     = { 1, -iC_SH, iC_ARITH, 0, "QB45", {0}, {0}, &QB44_4 };
iC_Gt QB45_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB45_0", {0}, {&iC_l_[847]}, &QB45 };
iC_Gt QB45_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB45_1", {&iC_l_[850]},
#error "iC: Action gate 'QB45_1' has no valid clock"
 {&iC_l_[852]}, &QB45_0 };
iC_Gt QB45_2   = { 1, -iC_AND, iC_S_SH, 0, "QB45_2", {&iC_l_[857]},
#error "iC: Action gate 'QB45_2' has no valid clock"
 {&iC_l_[860]}, &QB45_1 };
iC_Gt QB45_3   = { 1, -iC_AND, iC_R_SH, 0, "QB45_3", {&iC_l_[864]},
#error "iC: Action gate 'QB45_3' has no valid clock"
 {&iC_l_[866]}, &QB45_2 };
iC_Gt QB45_4   = { 1, -iC_OR, iC_ARITH, 0, "QB45_4", {0}, {&iC_l_[870]}, &QB45_3 };
iC_Gt QB46     = { 1, -iC_SH, iC_ARITH, 0, "QB46", {0}, {0}, &QB45_4 };
iC_Gt QB46_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB46_0", {0}, {&iC_l_[874]}, &QB46 };
iC_Gt QB46_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB46_1", {&iC_l_[877]},
#error "iC: Action gate 'QB46_1' has no valid clock"
 {&iC_l_[879]}, &QB46_0 };
iC_Gt QB46_2   = { 1, -iC_AND, iC_S_SH, 0, "QB46_2", {&iC_l_[884]},
#error "iC: Action gate 'QB46_2' has no valid clock"
 {&iC_l_[887]}, &QB46_1 };
iC_Gt QB46_3   = { 1, -iC_AND, iC_R_SH, 0, "QB46_3", {&iC_l_[891]},
#error "iC: Action gate 'QB46_3' has no valid clock"
 {&iC_l_[893]}, &QB46_2 };
iC_Gt QB46_4   = { 1, -iC_OR, iC_ARITH, 0, "QB46_4", {0}, {&iC_l_[897]}, &QB46_3 };
iC_Gt QB47     = { 1, -iC_SH, iC_ARITH, 0, "QB47", {0}, {0}, &QB46_4 };
iC_Gt QB47_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB47_0", {0}, {&iC_l_[901]}, &QB47 };
iC_Gt QB47_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB47_1", {&iC_l_[904]},
#error "iC: Action gate 'QB47_1' has no valid clock"
 {&iC_l_[906]}, &QB47_0 };
iC_Gt QB47_2   = { 1, -iC_AND, iC_S_SH, 0, "QB47_2", {&iC_l_[911]},
#error "iC: Action gate 'QB47_2' has no valid clock"
 {&iC_l_[914]}, &QB47_1 };
iC_Gt QB47_3   = { 1, -iC_AND, iC_R_SH, 0, "QB47_3", {&iC_l_[918]},
#error "iC: Action gate 'QB47_3' has no valid clock"
 {&iC_l_[920]}, &QB47_2 };
iC_Gt QB47_4   = { 1, -iC_OR, iC_ARITH, 0, "QB47_4", {0}, {&iC_l_[924]}, &QB47_3 };
iC_Gt QB48     = { 1, -iC_SH, iC_ARITH, 0, "QB48", {0}, {0}, &QB47_4 };
iC_Gt QB48_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB48_0", {0}, {&iC_l_[928]}, &QB48 };
iC_Gt QB48_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB48_1", {&iC_l_[931]},
#error "iC: Action gate 'QB48_1' has no valid clock"
 {&iC_l_[933]}, &QB48_0 };
iC_Gt QB48_2   = { 1, -iC_AND, iC_S_SH, 0, "QB48_2", {&iC_l_[938]}, {&iC_l_[940]}, &QB48_1 };
iC_Gt QB48_3   = { 1, -iC_AND, iC_R_SH, 0, "QB48_3", {&iC_l_[944]}, {&iC_l_[946]}, &QB48_2 };
iC_Gt QB48_4   = { 1, -iC_OR, iC_ARITH, 0, "QB48_4", {0}, {&iC_l_[950]}, &QB48_3 };
iC_Gt QB49     = { 1, -iC_SH, iC_ARITH, 0, "QB49", {0}, {0}, &QB48_4 };
iC_Gt QB49_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB49_0", {0}, {&iC_l_[954]}, &QB49 };
iC_Gt QB49_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB49_1", {&iC_l_[957]},
#error "iC: Action gate 'QB49_1' has no valid clock"
 {&iC_l_[959]}, &QB49_0 };
iC_Gt QB49_2   = { 1, -iC_AND, iC_S_SH, 0, "QB49_2", {&iC_l_[964]},
#error "iC: Action gate 'QB49_2' has no valid clock"
 {&iC_l_[966]}, &QB49_1 };
iC_Gt QB49_3   = { 1, -iC_AND, iC_R_SH, 0, "QB49_3", {&iC_l_[970]},
#error "iC: Action gate 'QB49_3' has no valid clock"
 {&iC_l_[972]}, &QB49_2 };
iC_Gt QB49_4   = { 1, -iC_OR, iC_ARITH, 0, "QB49_4", {0}, {&iC_l_[976]}, &QB49_3 };
iC_Gt QB50     = { 1, -iC_SH, iC_ARITH, 0, "QB50", {0}, {0}, &QB49_4 };
iC_Gt QB50_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB50_0", {0}, {&iC_l_[980]}, &QB50 };
iC_Gt QB50_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB50_1", {&iC_l_[983]},
#error "iC: Action gate 'QB50_1' has no valid clock"
 {&iC_l_[985]}, &QB50_0 };
iC_Gt QB50_2   = { 1, -iC_AND, iC_S_SH, 0, "QB50_2", {&iC_l_[990]},
#error "iC: Action gate 'QB50_2' has no valid clock"
 {&iC_l_[992]}, &QB50_1 };
iC_Gt QB50_3   = { 1, -iC_AND, iC_R_SH, 0, "QB50_3", {&iC_l_[996]},
#error "iC: Action gate 'QB50_3' has no valid clock"
 {&iC_l_[998]}, &QB50_2 };
iC_Gt QB50_4   = { 1, -iC_OR, iC_ARITH, 0, "QB50_4", {0}, {&iC_l_[1002]}, &QB50_3 };
iC_Gt QB51     = { 1, -iC_SH, iC_ARITH, 0, "QB51", {0}, {0}, &QB50_4 };
iC_Gt QB51_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB51_0", {0}, {&iC_l_[1006]}, &QB51 };
iC_Gt QB51_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB51_1", {&iC_l_[1009]},
#error "iC: Action gate 'QB51_1' has no valid clock"
 {&iC_l_[1011]}, &QB51_0 };
iC_Gt QB51_2   = { 1, -iC_AND, iC_S_SH, 0, "QB51_2", {&iC_l_[1016]},
#error "iC: Action gate 'QB51_2' has no valid clock"
 {&iC_l_[1018]}, &QB51_1 };
iC_Gt QB51_3   = { 1, -iC_AND, iC_R_SH, 0, "QB51_3", {&iC_l_[1022]},
#error "iC: Action gate 'QB51_3' has no valid clock"
 {&iC_l_[1024]}, &QB51_2 };
iC_Gt QB51_4   = { 1, -iC_OR, iC_ARITH, 0, "QB51_4", {0}, {&iC_l_[1028]}, &QB51_3 };
iC_Gt QB52     = { 1, -iC_SH, iC_ARITH, 0, "QB52", {0}, {0}, &QB51_4 };
iC_Gt QB52_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB52_0", {0}, {&iC_l_[1032]}, &QB52 };
iC_Gt QB52_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB52_1", {&iC_l_[1035]},
#error "iC: Action gate 'QB52_1' has no valid clock"
 {&iC_l_[1037]}, &QB52_0 };
iC_Gt QB52_2   = { 1, -iC_AND, iC_S_SH, 0, "QB52_2", {&iC_l_[1042]}, {&iC_l_[1044]}, &QB52_1 };
iC_Gt QB52_3   = { 1, -iC_AND, iC_R_SH, 0, "QB52_3", {&iC_l_[1048]}, {&iC_l_[1050]}, &QB52_2 };
iC_Gt QB52_4   = { 1, -iC_OR, iC_ARITH, 0, "QB52_4", {0}, {&iC_l_[1054]}, &QB52_3 };
iC_Gt QB53     = { 1, -iC_SH, iC_ARITH, 0, "QB53", {0}, {0}, &QB52_4 };
iC_Gt QB53_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB53_0", {0}, {&iC_l_[1058]}, &QB53 };
iC_Gt QB53_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB53_1", {&iC_l_[1061]},
#error "iC: Action gate 'QB53_1' has no valid clock"
 {&iC_l_[1063]}, &QB53_0 };
iC_Gt QB53_2   = { 1, -iC_AND, iC_S_SH, 0, "QB53_2", {&iC_l_[1068]}, {&iC_l_[1070]}, &QB53_1 };
iC_Gt QB53_3   = { 1, -iC_AND, iC_R_SH, 0, "QB53_3", {&iC_l_[1074]},
#error "iC: Action gate 'QB53_3' has no valid clock"
 {&iC_l_[1076]}, &QB53_2 };
iC_Gt QB53_4   = { 1, -iC_OR, iC_ARITH, 0, "QB53_4", {0}, {&iC_l_[1080]}, &QB53_3 };
iC_Gt QB54     = { 1, -iC_SH, iC_ARITH, 0, "QB54", {0}, {0}, &QB53_4 };
iC_Gt QB54_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB54_0", {0}, {&iC_l_[1084]}, &QB54 };
iC_Gt QB54_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB54_1", {&iC_l_[1087]},
#error "iC: Action gate 'QB54_1' has no valid clock"
 {&iC_l_[1089]}, &QB54_0 };
iC_Gt QB54_2   = { 1, -iC_AND, iC_S_SH, 0, "QB54_2", {&iC_l_[1094]}, {&iC_l_[1096]}, &QB54_1 };
iC_Gt QB54_3   = { 1, -iC_AND, iC_R_SH, 0, "QB54_3", {&iC_l_[1100]},
#error "iC: Action gate 'QB54_3' has no valid clock"
 {&iC_l_[1102]}, &QB54_2 };
iC_Gt QB54_4   = { 1, -iC_OR, iC_ARITH, 0, "QB54_4", {0}, {&iC_l_[1106]}, &QB54_3 };
iC_Gt QB55     = { 1, -iC_SH, iC_ARITH, 0, "QB55", {0}, {0}, &QB54_4 };
iC_Gt QB55_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB55_0", {0}, {&iC_l_[1110]}, &QB55 };
iC_Gt QB55_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB55_1", {&iC_l_[1113]},
#error "iC: Action gate 'QB55_1' has no valid clock"
 {&iC_l_[1115]}, &QB55_0 };
iC_Gt QB55_2   = { 1, -iC_AND, iC_S_SH, 0, "QB55_2", {&iC_l_[1120]}, {&iC_l_[1122]}, &QB55_1 };
iC_Gt QB55_3   = { 1, -iC_AND, iC_R_SH, 0, "QB55_3", {&iC_l_[1126]},
#error "iC: Action gate 'QB55_3' has no valid clock"
 {&iC_l_[1128]}, &QB55_2 };
iC_Gt QB55_4   = { 1, -iC_OR, iC_ARITH, 0, "QB55_4", {0}, {&iC_l_[1132]}, &QB55_3 };
iC_Gt QB56     = { 1, -iC_SH, iC_ARITH, 0, "QB56", {0}, {0}, &QB55_4 };
iC_Gt QB56_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB56_0", {0}, {&iC_l_[1136]}, &QB56 };
iC_Gt QB56_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB56_1", {&iC_l_[1139]},
#error "iC: Action gate 'QB56_1' has no valid clock"
 {&iC_l_[1141]}, &QB56_0 };
iC_Gt QB56_2   = { 1, -iC_AND, iC_S_SH, 0, "QB56_2", {&iC_l_[1146]},
#error "iC: Action gate 'QB56_2' has no valid clock"
 {&iC_l_[1149]}, &QB56_1 };
iC_Gt QB56_3   = { 1, -iC_AND, iC_R_SH, 0, "QB56_3", {&iC_l_[1153]}, {&iC_l_[1155]}, &QB56_2 };
iC_Gt QB56_4   = { 1, -iC_OR, iC_ARITH, 0, "QB56_4", {0}, {&iC_l_[1159]}, &QB56_3 };
iC_Gt QB57     = { 1, -iC_SH, iC_ARITH, 0, "QB57", {0}, {0}, &QB56_4 };
iC_Gt QB57_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB57_0", {0}, {&iC_l_[1163]}, &QB57 };
iC_Gt QB57_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB57_1", {&iC_l_[1166]},
#error "iC: Action gate 'QB57_1' has no valid clock"
 {&iC_l_[1168]}, &QB57_0 };
iC_Gt QB57_2   = { 1, -iC_AND, iC_S_SH, 0, "QB57_2", {&iC_l_[1173]},
#error "iC: Action gate 'QB57_2' has no valid clock"

#error "too many errors - iC compilation aborted"

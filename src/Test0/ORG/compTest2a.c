/********************************************************************
 *
 *	SOURCE:   ./Test0/compTest2a.ic
 *	OUTPUT:   ./Test0/compTest2a.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: compTest2a.c,v 1.1 2015/06/03 00:58:54 jw Exp $ -O7";

#include	<icg.h>

#warning "iC: Action gate 'QX0.6_1' controlled by timer '1' has no clock"

#warning "iC: Action gate 'QX0.7_1' controlled by timer '3' has no clock"

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
iC_Gt DSR_11_i = { 1, -iC_OR, iC_GATE, 0, "DSR_11_i", {0}, {&iC_l_[4]}, &DSR_10_i };
iC_Gt DSR_12_i = { 1, -iC_OR, iC_GATE, 0, "DSR_12_i", {0}, {&iC_l_[8]}, &DSR_11_i };
iC_Gt DSR_13_i = { 1, -iC_OR, iC_GATE, 0, "DSR_13_i", {0}, {&iC_l_[12]}, &DSR_12_i };
iC_Gt DSR_14_i = { 1, -iC_OR, iC_GATE, 0, "DSR_14_i", {0}, {&iC_l_[16]}, &DSR_13_i };
iC_Gt DSR_15_i = { 1, -iC_OR, iC_GATE, 0, "DSR_15_i", {0}, {&iC_l_[20]}, &DSR_14_i };
iC_Gt DSR_16_i = { 1, -iC_OR, iC_GATE, 0, "DSR_16_i", {0}, {&iC_l_[24]}, &DSR_15_i };
iC_Gt DSR_17_i = { 1, -iC_OR, iC_GATE, 0, "DSR_17_i", {0}, {&iC_l_[28]}, &DSR_16_i };
iC_Gt DSR_18_i = { 1, -iC_OR, iC_GATE, 0, "DSR_18_i", {0}, {&iC_l_[32]}, &DSR_17_i };
iC_Gt DSR_19_i = { 1, -iC_OR, iC_GATE, 0, "DSR_19_i", {0}, {&iC_l_[36]}, &DSR_18_i };
iC_Gt DSR_1_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_1_i", {0}, {&iC_l_[40]}, &DSR_19_i };
iC_Gt DSR_20_i = { 1, -iC_OR, iC_GATE, 0, "DSR_20_i", {0}, {&iC_l_[44]}, &DSR_1_i };
iC_Gt DSR_21_i = { 1, -iC_OR, iC_GATE, 0, "DSR_21_i", {0}, {&iC_l_[48]}, &DSR_20_i };
iC_Gt DSR_22_i = { 1, -iC_OR, iC_GATE, 0, "DSR_22_i", {0}, {&iC_l_[52]}, &DSR_21_i };
iC_Gt DSR_23_i = { 1, -iC_OR, iC_GATE, 0, "DSR_23_i", {0}, {&iC_l_[56]}, &DSR_22_i };
iC_Gt DSR_24_i = { 1, -iC_OR, iC_GATE, 0, "DSR_24_i", {0}, {&iC_l_[60]}, &DSR_23_i };
iC_Gt DSR_25_i = { 1, -iC_OR, iC_GATE, 0, "DSR_25_i", {0}, {&iC_l_[64]}, &DSR_24_i };
iC_Gt DSR_26_i = { 1, -iC_OR, iC_GATE, 0, "DSR_26_i", {0}, {&iC_l_[68]}, &DSR_25_i };
iC_Gt DSR_27_i = { 1, -iC_OR, iC_GATE, 0, "DSR_27_i", {0}, {&iC_l_[72]}, &DSR_26_i };
iC_Gt DSR_28_i = { 1, -iC_OR, iC_GATE, 0, "DSR_28_i", {0}, {&iC_l_[76]}, &DSR_27_i };
iC_Gt DSR_2_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_2_i", {0}, {&iC_l_[80]}, &DSR_28_i };
iC_Gt DSR_3_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_3_i", {0}, {&iC_l_[84]}, &DSR_2_i };
iC_Gt DSR_4_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_4_i", {0}, {&iC_l_[88]}, &DSR_3_i };
iC_Gt DSR_5_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_5_i", {0}, {&iC_l_[92]}, &DSR_4_i };
iC_Gt DSR_6_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_6_i", {0}, {&iC_l_[96]}, &DSR_5_i };
iC_Gt DSR_7_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_7_i", {0}, {&iC_l_[100]}, &DSR_6_i };
iC_Gt DSR_8_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_8_i", {0}, {&iC_l_[104]}, &DSR_7_i };
iC_Gt DSR_9_i  = { 1, -iC_OR, iC_GATE, 0, "DSR_9_i", {0}, {&iC_l_[108]}, &DSR_8_i };
iC_Gt IB16     = { 1, -iC_INPW, iC_ARITH, 0, "IB16", {0}, {0}, &DSR_9_i };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB16 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
iC_Gt IX0_4    = { 1, -iC_INPX, iC_GATE, 0, "IX0.4", {0}, {0}, &IX0_3 };
iC_Gt IX0_5    = { 1, -iC_INPX, iC_GATE, 0, "IX0.5", {0}, {0}, &IX0_4 };
iC_Gt IX0_6    = { 1, -iC_INPX, iC_GATE, 0, "IX0.6", {0}, {0}, &IX0_5 };
iC_Gt QB16     = { 1, -iC_SH, iC_ARITH, 0, "QB16", {0}, {0}, &IX0_6 };
iC_Gt QB16_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB16_0", {0}, {&iC_l_[112]}, &QB16 };
iC_Gt QB16_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB16_1", {&iC_l_[115]}, {&iC_l_[117]}, &QB16_0 };
iC_Gt QB17     = { 1, -iC_SH, iC_ARITH, 0, "QB17", {0}, {0}, &QB16_1 };
iC_Gt QB17_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB17_0", {0}, {&iC_l_[120]}, &QB17 };
iC_Gt QB17_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB17_1", {&iC_l_[123]},
#warning "iC: Action gate 'QB17_1' has no valid clock"
 {&iC_l_[125]}, &QB17_0 };
iC_Gt QB18     = { 1, -iC_SH, iC_ARITH, 0, "QB18", {0}, {0}, &QB17_1 };
iC_Gt QB18_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB18_0", {0}, {&iC_l_[128]}, &QB18 };
iC_Gt QB18_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB18_1", {&iC_l_[131]},
#warning "iC: Action gate 'QB18_1' has no valid clock"
 {&iC_l_[133]}, &QB18_0 };
iC_Gt QB19     = { 1, -iC_SH, iC_ARITH, 0, "QB19", {0}, {0}, &QB18_1 };
iC_Gt QB19_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB19_0", {0}, {&iC_l_[136]}, &QB19 };
iC_Gt QB19_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB19_1", {&iC_l_[139]},
#warning "iC: Action gate 'QB19_1' has no valid clock"
 {&iC_l_[141]}, &QB19_0 };
iC_Gt QB20     = { 1, -iC_SH, iC_ARITH, 0, "QB20", {0}, {0}, &QB19_1 };
iC_Gt QB20_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB20_0", {0}, {&iC_l_[144]}, &QB20 };
iC_Gt QB20_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB20_1", {&iC_l_[147]}, {&iC_l_[149]}, &QB20_0 };
iC_Gt QB20_2   = { 1, -iC_OR, iC_R_SH, 0, "QB20_2", {&iC_l_[154]}, {&iC_l_[156]}, &QB20_1 };
iC_Gt QB20_3   = { 1, -iC_OR, iC_ARITH, 0, "QB20_3", {0}, {&iC_l_[159]}, &QB20_2 };
iC_Gt QB21     = { 1, -iC_SH, iC_ARITH, 0, "QB21", {0}, {0}, &QB20_3 };
iC_Gt QB21_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB21_0", {0}, {&iC_l_[162]}, &QB21 };
iC_Gt QB21_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB21_1", {&iC_l_[165]},
#warning "iC: Action gate 'QB21_1' has no valid clock"
 {&iC_l_[167]}, &QB21_0 };
iC_Gt QB21_2   = { 1, -iC_OR, iC_R_SH, 0, "QB21_2", {&iC_l_[172]},
#warning "iC: Action gate 'QB21_2' has no valid clock"
 {&iC_l_[174]}, &QB21_1 };
iC_Gt QB21_3   = { 1, -iC_OR, iC_ARITH, 0, "QB21_3", {0}, {&iC_l_[177]}, &QB21_2 };
iC_Gt QB22     = { 1, -iC_SH, iC_ARITH, 0, "QB22", {0}, {0}, &QB21_3 };
iC_Gt QB22_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB22_0", {0}, {&iC_l_[180]}, &QB22 };
iC_Gt QB22_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB22_1", {&iC_l_[183]},
#warning "iC: Action gate 'QB22_1' has no valid clock"
 {&iC_l_[185]}, &QB22_0 };
iC_Gt QB22_2   = { 1, -iC_OR, iC_R_SH, 0, "QB22_2", {&iC_l_[190]},
#warning "iC: Action gate 'QB22_2' has no valid clock"
 {&iC_l_[192]}, &QB22_1 };
iC_Gt QB22_3   = { 1, -iC_OR, iC_ARITH, 0, "QB22_3", {0}, {&iC_l_[195]}, &QB22_2 };
iC_Gt QB23     = { 1, -iC_SH, iC_ARITH, 0, "QB23", {0}, {0}, &QB22_3 };
iC_Gt QB23_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB23_0", {0}, {&iC_l_[198]}, &QB23 };
iC_Gt QB23_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB23_1", {&iC_l_[201]},
#warning "iC: Action gate 'QB23_1' has no valid clock"
 {&iC_l_[203]}, &QB23_0 };
iC_Gt QB23_2   = { 1, -iC_OR, iC_R_SH, 0, "QB23_2", {&iC_l_[208]},
#warning "iC: Action gate 'QB23_2' has no valid clock"
 {&iC_l_[210]}, &QB23_1 };
iC_Gt QB23_3   = { 1, -iC_OR, iC_ARITH, 0, "QB23_3", {0}, {&iC_l_[213]}, &QB23_2 };
iC_Gt QB24     = { 1, -iC_SH, iC_ARITH, 0, "QB24", {0}, {0}, &QB23_3 };
iC_Gt QB24_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB24_0", {0}, {&iC_l_[216]}, &QB24 };
iC_Gt QB24_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB24_1", {&iC_l_[219]},
#warning "iC: Action gate 'QB24_1' has no valid clock"
 {&iC_l_[221]}, &QB24_0 };
iC_Gt QB24_2   = { 1, -iC_OR, iC_R_SH, 0, "QB24_2", {&iC_l_[226]}, {&iC_l_[228]}, &QB24_1 };
iC_Gt QB24_3   = { 1, -iC_OR, iC_ARITH, 0, "QB24_3", {0}, {&iC_l_[231]}, &QB24_2 };
iC_Gt QB25     = { 1, -iC_SH, iC_ARITH, 0, "QB25", {0}, {0}, &QB24_3 };
iC_Gt QB25_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB25_0", {0}, {&iC_l_[234]}, &QB25 };
iC_Gt QB25_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB25_1", {&iC_l_[237]},
#warning "iC: Action gate 'QB25_1' has no valid clock"
 {&iC_l_[239]}, &QB25_0 };
iC_Gt QB25_2   = { 1, -iC_OR, iC_R_SH, 0, "QB25_2", {&iC_l_[244]},
#warning "iC: Action gate 'QB25_2' has no valid clock"
 {&iC_l_[246]}, &QB25_1 };
iC_Gt QB25_3   = { 1, -iC_OR, iC_ARITH, 0, "QB25_3", {0}, {&iC_l_[249]}, &QB25_2 };
iC_Gt QB26     = { 1, -iC_SH, iC_ARITH, 0, "QB26", {0}, {0}, &QB25_3 };
iC_Gt QB26_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB26_0", {0}, {&iC_l_[252]}, &QB26 };
iC_Gt QB26_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB26_1", {&iC_l_[255]},
#warning "iC: Action gate 'QB26_1' has no valid clock"
 {&iC_l_[257]}, &QB26_0 };
iC_Gt QB26_2   = { 1, -iC_OR, iC_R_SH, 0, "QB26_2", {&iC_l_[262]},
#warning "iC: Action gate 'QB26_2' has no valid clock"
 {&iC_l_[264]}, &QB26_1 };
iC_Gt QB26_3   = { 1, -iC_OR, iC_ARITH, 0, "QB26_3", {0}, {&iC_l_[267]}, &QB26_2 };
iC_Gt QB27     = { 1, -iC_SH, iC_ARITH, 0, "QB27", {0}, {0}, &QB26_3 };
iC_Gt QB27_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB27_0", {0}, {&iC_l_[270]}, &QB27 };
iC_Gt QB27_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB27_1", {&iC_l_[273]},
#warning "iC: Action gate 'QB27_1' has no valid clock"
 {&iC_l_[275]}, &QB27_0 };
iC_Gt QB27_2   = { 1, -iC_OR, iC_R_SH, 0, "QB27_2", {&iC_l_[280]},
#warning "iC: Action gate 'QB27_2' has no valid clock"
 {&iC_l_[282]}, &QB27_1 };
iC_Gt QB27_3   = { 1, -iC_OR, iC_ARITH, 0, "QB27_3", {0}, {&iC_l_[285]}, &QB27_2 };
iC_Gt QB28     = { 1, -iC_SH, iC_ARITH, 0, "QB28", {0}, {0}, &QB27_3 };
iC_Gt QB28_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB28_0", {0}, {&iC_l_[288]}, &QB28 };
iC_Gt QB28_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB28_1", {&iC_l_[291]},
#warning "iC: Action gate 'QB28_1' has no valid clock"
 {&iC_l_[293]}, &QB28_0 };
iC_Gt QB28_2   = { 1, -iC_OR, iC_R_SH, 0, "QB28_2", {&iC_l_[298]}, {&iC_l_[300]}, &QB28_1 };
iC_Gt QB28_3   = { 1, -iC_OR, iC_ARITH, 0, "QB28_3", {0}, {&iC_l_[303]}, &QB28_2 };
iC_Gt QB29     = { 1, -iC_SH, iC_ARITH, 0, "QB29", {0}, {0}, &QB28_3 };
iC_Gt QB29_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB29_0", {0}, {&iC_l_[306]}, &QB29 };
iC_Gt QB29_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB29_1", {&iC_l_[309]},
#warning "iC: Action gate 'QB29_1' has no valid clock"
 {&iC_l_[311]}, &QB29_0 };
iC_Gt QB29_2   = { 1, -iC_OR, iC_R_SH, 0, "QB29_2", {&iC_l_[316]},
#warning "iC: Action gate 'QB29_2' has no valid clock"
 {&iC_l_[318]}, &QB29_1 };
iC_Gt QB29_3   = { 1, -iC_OR, iC_ARITH, 0, "QB29_3", {0}, {&iC_l_[321]}, &QB29_2 };
iC_Gt QB30     = { 1, -iC_SH, iC_ARITH, 0, "QB30", {0}, {0}, &QB29_3 };
iC_Gt QB30_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB30_0", {0}, {&iC_l_[324]}, &QB30 };
iC_Gt QB30_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB30_1", {&iC_l_[327]},
#warning "iC: Action gate 'QB30_1' has no valid clock"
 {&iC_l_[329]}, &QB30_0 };
iC_Gt QB30_2   = { 1, -iC_OR, iC_R_SH, 0, "QB30_2", {&iC_l_[334]},
#warning "iC: Action gate 'QB30_2' has no valid clock"
 {&iC_l_[336]}, &QB30_1 };
iC_Gt QB30_3   = { 1, -iC_OR, iC_ARITH, 0, "QB30_3", {0}, {&iC_l_[339]}, &QB30_2 };
iC_Gt QB31     = { 1, -iC_SH, iC_ARITH, 0, "QB31", {0}, {0}, &QB30_3 };
iC_Gt QB31_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB31_0", {0}, {&iC_l_[342]}, &QB31 };
iC_Gt QB31_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB31_1", {&iC_l_[345]},
#warning "iC: Action gate 'QB31_1' has no valid clock"
 {&iC_l_[347]}, &QB31_0 };
iC_Gt QB31_2   = { 1, -iC_OR, iC_R_SH, 0, "QB31_2", {&iC_l_[352]},
#warning "iC: Action gate 'QB31_2' has no valid clock"
 {&iC_l_[354]}, &QB31_1 };
iC_Gt QB31_3   = { 1, -iC_OR, iC_ARITH, 0, "QB31_3", {0}, {&iC_l_[357]}, &QB31_2 };
iC_Gt QB32     = { 1, -iC_SH, iC_ARITH, 0, "QB32", {0}, {0}, &QB31_3 };
iC_Gt QB32_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB32_0", {0}, {&iC_l_[360]}, &QB32 };
iC_Gt QB32_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB32_1", {&iC_l_[363]}, {&iC_l_[365]}, &QB32_0 };
iC_Gt QB32_2   = { 1, -iC_AND, iC_S_SH, 0, "QB32_2", {&iC_l_[370]}, {&iC_l_[372]}, &QB32_1 };
iC_Gt QB32_3   = { 1, -iC_AND, iC_R_SH, 0, "QB32_3", {&iC_l_[376]}, {&iC_l_[378]}, &QB32_2 };
iC_Gt QB32_4   = { 1, -iC_OR, iC_ARITH, 0, "QB32_4", {0}, {&iC_l_[382]}, &QB32_3 };
iC_Gt QB33     = { 1, -iC_SH, iC_ARITH, 0, "QB33", {0}, {0}, &QB32_4 };
iC_Gt QB33_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB33_0", {0}, {&iC_l_[386]}, &QB33 };
iC_Gt QB33_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB33_1", {&iC_l_[389]},
#warning "iC: Action gate 'QB33_1' has no valid clock"
 {&iC_l_[391]}, &QB33_0 };
iC_Gt QB33_2   = { 1, -iC_AND, iC_S_SH, 0, "QB33_2", {&iC_l_[396]},
#warning "iC: Action gate 'QB33_2' has no valid clock"
 {&iC_l_[398]}, &QB33_1 };
iC_Gt QB33_3   = { 1, -iC_AND, iC_R_SH, 0, "QB33_3", {&iC_l_[402]},
#warning "iC: Action gate 'QB33_3' has no valid clock"
 {&iC_l_[404]}, &QB33_2 };
iC_Gt QB33_4   = { 1, -iC_OR, iC_ARITH, 0, "QB33_4", {0}, {&iC_l_[408]}, &QB33_3 };
iC_Gt QB34     = { 1, -iC_SH, iC_ARITH, 0, "QB34", {0}, {0}, &QB33_4 };
iC_Gt QB34_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB34_0", {0}, {&iC_l_[412]}, &QB34 };
iC_Gt QB34_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB34_1", {&iC_l_[415]},
#warning "iC: Action gate 'QB34_1' has no valid clock"
 {&iC_l_[417]}, &QB34_0 };
iC_Gt QB34_2   = { 1, -iC_AND, iC_S_SH, 0, "QB34_2", {&iC_l_[422]},
#warning "iC: Action gate 'QB34_2' has no valid clock"
 {&iC_l_[424]}, &QB34_1 };
iC_Gt QB34_3   = { 1, -iC_AND, iC_R_SH, 0, "QB34_3", {&iC_l_[428]},
#warning "iC: Action gate 'QB34_3' has no valid clock"
 {&iC_l_[430]}, &QB34_2 };
iC_Gt QB34_4   = { 1, -iC_OR, iC_ARITH, 0, "QB34_4", {0}, {&iC_l_[434]}, &QB34_3 };
iC_Gt QB35     = { 1, -iC_SH, iC_ARITH, 0, "QB35", {0}, {0}, &QB34_4 };
iC_Gt QB35_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB35_0", {0}, {&iC_l_[438]}, &QB35 };
iC_Gt QB35_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB35_1", {&iC_l_[441]},
#warning "iC: Action gate 'QB35_1' has no valid clock"
 {&iC_l_[443]}, &QB35_0 };
iC_Gt QB35_2   = { 1, -iC_AND, iC_S_SH, 0, "QB35_2", {&iC_l_[448]},
#warning "iC: Action gate 'QB35_2' has no valid clock"
 {&iC_l_[450]}, &QB35_1 };
iC_Gt QB35_3   = { 1, -iC_AND, iC_R_SH, 0, "QB35_3", {&iC_l_[454]},
#warning "iC: Action gate 'QB35_3' has no valid clock"
 {&iC_l_[456]}, &QB35_2 };
iC_Gt QB35_4   = { 1, -iC_OR, iC_ARITH, 0, "QB35_4", {0}, {&iC_l_[460]}, &QB35_3 };
iC_Gt QB36     = { 1, -iC_SH, iC_ARITH, 0, "QB36", {0}, {0}, &QB35_4 };
iC_Gt QB36_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB36_0", {0}, {&iC_l_[464]}, &QB36 };
iC_Gt QB36_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB36_1", {&iC_l_[467]},
#warning "iC: Action gate 'QB36_1' has no valid clock"
 {&iC_l_[469]}, &QB36_0 };
iC_Gt QB36_2   = { 1, -iC_AND, iC_S_SH, 0, "QB36_2", {&iC_l_[474]}, {&iC_l_[476]}, &QB36_1 };
iC_Gt QB36_3   = { 1, -iC_AND, iC_R_SH, 0, "QB36_3", {&iC_l_[480]}, {&iC_l_[482]}, &QB36_2 };
iC_Gt QB36_4   = { 1, -iC_OR, iC_ARITH, 0, "QB36_4", {0}, {&iC_l_[486]}, &QB36_3 };
iC_Gt QB37     = { 1, -iC_SH, iC_ARITH, 0, "QB37", {0}, {0}, &QB36_4 };
iC_Gt QB37_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB37_0", {0}, {&iC_l_[490]}, &QB37 };
iC_Gt QB37_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB37_1", {&iC_l_[493]},
#warning "iC: Action gate 'QB37_1' has no valid clock"
 {&iC_l_[495]}, &QB37_0 };
iC_Gt QB37_2   = { 1, -iC_AND, iC_S_SH, 0, "QB37_2", {&iC_l_[500]},
#warning "iC: Action gate 'QB37_2' has no valid clock"
 {&iC_l_[502]}, &QB37_1 };
iC_Gt QB37_3   = { 1, -iC_AND, iC_R_SH, 0, "QB37_3", {&iC_l_[506]},
#warning "iC: Action gate 'QB37_3' has no valid clock"
 {&iC_l_[508]}, &QB37_2 };
iC_Gt QB37_4   = { 1, -iC_OR, iC_ARITH, 0, "QB37_4", {0}, {&iC_l_[512]}, &QB37_3 };
iC_Gt QB38     = { 1, -iC_SH, iC_ARITH, 0, "QB38", {0}, {0}, &QB37_4 };
iC_Gt QB38_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB38_0", {0}, {&iC_l_[516]}, &QB38 };
iC_Gt QB38_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB38_1", {&iC_l_[519]},
#warning "iC: Action gate 'QB38_1' has no valid clock"
 {&iC_l_[521]}, &QB38_0 };
iC_Gt QB38_2   = { 1, -iC_AND, iC_S_SH, 0, "QB38_2", {&iC_l_[526]},
#warning "iC: Action gate 'QB38_2' has no valid clock"
 {&iC_l_[528]}, &QB38_1 };
iC_Gt QB38_3   = { 1, -iC_AND, iC_R_SH, 0, "QB38_3", {&iC_l_[532]},
#warning "iC: Action gate 'QB38_3' has no valid clock"
 {&iC_l_[534]}, &QB38_2 };
iC_Gt QB38_4   = { 1, -iC_OR, iC_ARITH, 0, "QB38_4", {0}, {&iC_l_[538]}, &QB38_3 };
iC_Gt QB39     = { 1, -iC_SH, iC_ARITH, 0, "QB39", {0}, {0}, &QB38_4 };
iC_Gt QB39_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB39_0", {0}, {&iC_l_[542]}, &QB39 };
iC_Gt QB39_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB39_1", {&iC_l_[545]},
#warning "iC: Action gate 'QB39_1' has no valid clock"
 {&iC_l_[547]}, &QB39_0 };
iC_Gt QB39_2   = { 1, -iC_AND, iC_S_SH, 0, "QB39_2", {&iC_l_[552]},
#warning "iC: Action gate 'QB39_2' has no valid clock"
 {&iC_l_[554]}, &QB39_1 };
iC_Gt QB39_3   = { 1, -iC_AND, iC_R_SH, 0, "QB39_3", {&iC_l_[558]},
#warning "iC: Action gate 'QB39_3' has no valid clock"
 {&iC_l_[560]}, &QB39_2 };
iC_Gt QB39_4   = { 1, -iC_OR, iC_ARITH, 0, "QB39_4", {0}, {&iC_l_[564]}, &QB39_3 };
iC_Gt QB40     = { 1, -iC_SH, iC_ARITH, 0, "QB40", {0}, {0}, &QB39_4 };
iC_Gt QB40_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB40_0", {0}, {&iC_l_[568]}, &QB40 };
iC_Gt QB40_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB40_1", {&iC_l_[571]},
#warning "iC: Action gate 'QB40_1' has no valid clock"
 {&iC_l_[573]}, &QB40_0 };
iC_Gt QB40_2   = { 1, -iC_AND, iC_S_SH, 0, "QB40_2", {&iC_l_[578]}, {&iC_l_[580]}, &QB40_1 };
iC_Gt QB40_3   = { 1, -iC_AND, iC_R_SH, 0, "QB40_3", {&iC_l_[584]}, {&iC_l_[586]}, &QB40_2 };
iC_Gt QB40_4   = { 1, -iC_OR, iC_ARITH, 0, "QB40_4", {0}, {&iC_l_[590]}, &QB40_3 };
iC_Gt QB41     = { 1, -iC_SH, iC_ARITH, 0, "QB41", {0}, {0}, &QB40_4 };
iC_Gt QB41_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB41_0", {0}, {&iC_l_[594]}, &QB41 };
iC_Gt QB41_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB41_1", {&iC_l_[597]},
#warning "iC: Action gate 'QB41_1' has no valid clock"
 {&iC_l_[599]}, &QB41_0 };
iC_Gt QB41_2   = { 1, -iC_AND, iC_S_SH, 0, "QB41_2", {&iC_l_[604]}, {&iC_l_[606]}, &QB41_1 };
iC_Gt QB41_3   = { 1, -iC_AND, iC_R_SH, 0, "QB41_3", {&iC_l_[610]},
#warning "iC: Action gate 'QB41_3' has no valid clock"
 {&iC_l_[612]}, &QB41_2 };
iC_Gt QB41_4   = { 1, -iC_OR, iC_ARITH, 0, "QB41_4", {0}, {&iC_l_[616]}, &QB41_3 };
iC_Gt QB42     = { 1, -iC_SH, iC_ARITH, 0, "QB42", {0}, {0}, &QB41_4 };
iC_Gt QB42_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB42_0", {0}, {&iC_l_[620]}, &QB42 };
iC_Gt QB42_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB42_1", {&iC_l_[623]},
#warning "iC: Action gate 'QB42_1' has no valid clock"
 {&iC_l_[625]}, &QB42_0 };
iC_Gt QB42_2   = { 1, -iC_AND, iC_S_SH, 0, "QB42_2", {&iC_l_[630]}, {&iC_l_[632]}, &QB42_1 };
iC_Gt QB42_3   = { 1, -iC_AND, iC_R_SH, 0, "QB42_3", {&iC_l_[636]},
#warning "iC: Action gate 'QB42_3' has no valid clock"
 {&iC_l_[638]}, &QB42_2 };
iC_Gt QB42_4   = { 1, -iC_OR, iC_ARITH, 0, "QB42_4", {0}, {&iC_l_[642]}, &QB42_3 };
iC_Gt QB43     = { 1, -iC_SH, iC_ARITH, 0, "QB43", {0}, {0}, &QB42_4 };
iC_Gt QB43_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB43_0", {0}, {&iC_l_[646]}, &QB43 };
iC_Gt QB43_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB43_1", {&iC_l_[649]},
#warning "iC: Action gate 'QB43_1' has no valid clock"
 {&iC_l_[651]}, &QB43_0 };
iC_Gt QB43_2   = { 1, -iC_AND, iC_S_SH, 0, "QB43_2", {&iC_l_[656]}, {&iC_l_[658]}, &QB43_1 };
iC_Gt QB43_3   = { 1, -iC_AND, iC_R_SH, 0, "QB43_3", {&iC_l_[662]},
#warning "iC: Action gate 'QB43_3' has no valid clock"
 {&iC_l_[664]}, &QB43_2 };
iC_Gt QB43_4   = { 1, -iC_OR, iC_ARITH, 0, "QB43_4", {0}, {&iC_l_[668]}, &QB43_3 };
iC_Gt QB44     = { 1, -iC_SH, iC_ARITH, 0, "QB44", {0}, {0}, &QB43_4 };
iC_Gt QB44_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB44_0", {0}, {&iC_l_[672]}, &QB44 };
iC_Gt QB44_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB44_1", {&iC_l_[675]},
#warning "iC: Action gate 'QB44_1' has no valid clock"
 {&iC_l_[677]}, &QB44_0 };
iC_Gt QB44_2   = { 1, -iC_AND, iC_S_SH, 0, "QB44_2", {&iC_l_[682]},
#warning "iC: Action gate 'QB44_2' has no valid clock"
 {&iC_l_[685]}, &QB44_1 };
iC_Gt QB44_3   = { 1, -iC_AND, iC_R_SH, 0, "QB44_3", {&iC_l_[689]}, {&iC_l_[691]}, &QB44_2 };
iC_Gt QB44_4   = { 1, -iC_OR, iC_ARITH, 0, "QB44_4", {0}, {&iC_l_[695]}, &QB44_3 };
iC_Gt QB45     = { 1, -iC_SH, iC_ARITH, 0, "QB45", {0}, {0}, &QB44_4 };
iC_Gt QB45_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB45_0", {0}, {&iC_l_[699]}, &QB45 };
iC_Gt QB45_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB45_1", {&iC_l_[702]},
#warning "iC: Action gate 'QB45_1' has no valid clock"
 {&iC_l_[704]}, &QB45_0 };
iC_Gt QB45_2   = { 1, -iC_AND, iC_S_SH, 0, "QB45_2", {&iC_l_[709]},
#warning "iC: Action gate 'QB45_2' has no valid clock"
 {&iC_l_[712]}, &QB45_1 };
iC_Gt QB45_3   = { 1, -iC_AND, iC_R_SH, 0, "QB45_3", {&iC_l_[716]},
#warning "iC: Action gate 'QB45_3' has no valid clock"
 {&iC_l_[718]}, &QB45_2 };
iC_Gt QB45_4   = { 1, -iC_OR, iC_ARITH, 0, "QB45_4", {0}, {&iC_l_[722]}, &QB45_3 };
iC_Gt QB46     = { 1, -iC_SH, iC_ARITH, 0, "QB46", {0}, {0}, &QB45_4 };
iC_Gt QB46_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB46_0", {0}, {&iC_l_[726]}, &QB46 };
iC_Gt QB46_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB46_1", {&iC_l_[729]},
#warning "iC: Action gate 'QB46_1' has no valid clock"
 {&iC_l_[731]}, &QB46_0 };
iC_Gt QB46_2   = { 1, -iC_AND, iC_S_SH, 0, "QB46_2", {&iC_l_[736]},
#warning "iC: Action gate 'QB46_2' has no valid clock"
 {&iC_l_[739]}, &QB46_1 };
iC_Gt QB46_3   = { 1, -iC_AND, iC_R_SH, 0, "QB46_3", {&iC_l_[743]},
#warning "iC: Action gate 'QB46_3' has no valid clock"
 {&iC_l_[745]}, &QB46_2 };
iC_Gt QB46_4   = { 1, -iC_OR, iC_ARITH, 0, "QB46_4", {0}, {&iC_l_[749]}, &QB46_3 };
iC_Gt QB47     = { 1, -iC_SH, iC_ARITH, 0, "QB47", {0}, {0}, &QB46_4 };
iC_Gt QB47_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB47_0", {0}, {&iC_l_[753]}, &QB47 };
iC_Gt QB47_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB47_1", {&iC_l_[756]},
#warning "iC: Action gate 'QB47_1' has no valid clock"
 {&iC_l_[758]}, &QB47_0 };
iC_Gt QB47_2   = { 1, -iC_AND, iC_S_SH, 0, "QB47_2", {&iC_l_[763]},
#warning "iC: Action gate 'QB47_2' has no valid clock"
 {&iC_l_[766]}, &QB47_1 };
iC_Gt QB47_3   = { 1, -iC_AND, iC_R_SH, 0, "QB47_3", {&iC_l_[770]},
#warning "iC: Action gate 'QB47_3' has no valid clock"
 {&iC_l_[772]}, &QB47_2 };
iC_Gt QB47_4   = { 1, -iC_OR, iC_ARITH, 0, "QB47_4", {0}, {&iC_l_[776]}, &QB47_3 };
iC_Gt QB48     = { 1, -iC_SH, iC_ARITH, 0, "QB48", {0}, {0}, &QB47_4 };
iC_Gt QB48_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB48_0", {0}, {&iC_l_[780]}, &QB48 };
iC_Gt QB48_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB48_1", {&iC_l_[783]},
#warning "iC: Action gate 'QB48_1' has no valid clock"
 {&iC_l_[785]}, &QB48_0 };
iC_Gt QB48_2   = { 1, -iC_AND, iC_S_SH, 0, "QB48_2", {&iC_l_[790]}, {&iC_l_[792]}, &QB48_1 };
iC_Gt QB48_3   = { 1, -iC_AND, iC_R_SH, 0, "QB48_3", {&iC_l_[796]}, {&iC_l_[798]}, &QB48_2 };
iC_Gt QB48_4   = { 1, -iC_OR, iC_ARITH, 0, "QB48_4", {0}, {&iC_l_[802]}, &QB48_3 };
iC_Gt QB49     = { 1, -iC_SH, iC_ARITH, 0, "QB49", {0}, {0}, &QB48_4 };
iC_Gt QB49_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB49_0", {0}, {&iC_l_[806]}, &QB49 };
iC_Gt QB49_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB49_1", {&iC_l_[809]},
#warning "iC: Action gate 'QB49_1' has no valid clock"
 {&iC_l_[811]}, &QB49_0 };
iC_Gt QB49_2   = { 1, -iC_AND, iC_S_SH, 0, "QB49_2", {&iC_l_[816]},
#warning "iC: Action gate 'QB49_2' has no valid clock"
 {&iC_l_[818]}, &QB49_1 };
iC_Gt QB49_3   = { 1, -iC_AND, iC_R_SH, 0, "QB49_3", {&iC_l_[822]},
#warning "iC: Action gate 'QB49_3' has no valid clock"
 {&iC_l_[824]}, &QB49_2 };
iC_Gt QB49_4   = { 1, -iC_OR, iC_ARITH, 0, "QB49_4", {0}, {&iC_l_[828]}, &QB49_3 };
iC_Gt QB50     = { 1, -iC_SH, iC_ARITH, 0, "QB50", {0}, {0}, &QB49_4 };
iC_Gt QB50_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB50_0", {0}, {&iC_l_[832]}, &QB50 };
iC_Gt QB50_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB50_1", {&iC_l_[835]},
#warning "iC: Action gate 'QB50_1' has no valid clock"
 {&iC_l_[837]}, &QB50_0 };
iC_Gt QB50_2   = { 1, -iC_AND, iC_S_SH, 0, "QB50_2", {&iC_l_[842]},
#warning "iC: Action gate 'QB50_2' has no valid clock"
 {&iC_l_[844]}, &QB50_1 };
iC_Gt QB50_3   = { 1, -iC_AND, iC_R_SH, 0, "QB50_3", {&iC_l_[848]},
#warning "iC: Action gate 'QB50_3' has no valid clock"
 {&iC_l_[850]}, &QB50_2 };
iC_Gt QB50_4   = { 1, -iC_OR, iC_ARITH, 0, "QB50_4", {0}, {&iC_l_[854]}, &QB50_3 };
iC_Gt QB51     = { 1, -iC_SH, iC_ARITH, 0, "QB51", {0}, {0}, &QB50_4 };
iC_Gt QB51_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB51_0", {0}, {&iC_l_[858]}, &QB51 };
iC_Gt QB51_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB51_1", {&iC_l_[861]},
#warning "iC: Action gate 'QB51_1' has no valid clock"
 {&iC_l_[863]}, &QB51_0 };
iC_Gt QB51_2   = { 1, -iC_AND, iC_S_SH, 0, "QB51_2", {&iC_l_[868]},
#warning "iC: Action gate 'QB51_2' has no valid clock"
 {&iC_l_[870]}, &QB51_1 };
iC_Gt QB51_3   = { 1, -iC_AND, iC_R_SH, 0, "QB51_3", {&iC_l_[874]},
#warning "iC: Action gate 'QB51_3' has no valid clock"
 {&iC_l_[876]}, &QB51_2 };
iC_Gt QB51_4   = { 1, -iC_OR, iC_ARITH, 0, "QB51_4", {0}, {&iC_l_[880]}, &QB51_3 };
iC_Gt QB52     = { 1, -iC_SH, iC_ARITH, 0, "QB52", {0}, {0}, &QB51_4 };
iC_Gt QB52_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB52_0", {0}, {&iC_l_[884]}, &QB52 };
iC_Gt QB52_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB52_1", {&iC_l_[887]},
#warning "iC: Action gate 'QB52_1' has no valid clock"
 {&iC_l_[889]}, &QB52_0 };
iC_Gt QB52_2   = { 1, -iC_AND, iC_S_SH, 0, "QB52_2", {&iC_l_[894]}, {&iC_l_[896]}, &QB52_1 };
iC_Gt QB52_3   = { 1, -iC_AND, iC_R_SH, 0, "QB52_3", {&iC_l_[900]}, {&iC_l_[902]}, &QB52_2 };
iC_Gt QB52_4   = { 1, -iC_OR, iC_ARITH, 0, "QB52_4", {0}, {&iC_l_[906]}, &QB52_3 };
iC_Gt QB53     = { 1, -iC_SH, iC_ARITH, 0, "QB53", {0}, {0}, &QB52_4 };
iC_Gt QB53_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB53_0", {0}, {&iC_l_[910]}, &QB53 };
iC_Gt QB53_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB53_1", {&iC_l_[913]},
#warning "iC: Action gate 'QB53_1' has no valid clock"
 {&iC_l_[915]}, &QB53_0 };
iC_Gt QB53_2   = { 1, -iC_AND, iC_S_SH, 0, "QB53_2", {&iC_l_[920]}, {&iC_l_[922]}, &QB53_1 };
iC_Gt QB53_3   = { 1, -iC_AND, iC_R_SH, 0, "QB53_3", {&iC_l_[926]},
#warning "iC: Action gate 'QB53_3' has no valid clock"
 {&iC_l_[928]}, &QB53_2 };
iC_Gt QB53_4   = { 1, -iC_OR, iC_ARITH, 0, "QB53_4", {0}, {&iC_l_[932]}, &QB53_3 };
iC_Gt QB54     = { 1, -iC_SH, iC_ARITH, 0, "QB54", {0}, {0}, &QB53_4 };
iC_Gt QB54_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB54_0", {0}, {&iC_l_[936]}, &QB54 };
iC_Gt QB54_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB54_1", {&iC_l_[939]},
#warning "iC: Action gate 'QB54_1' has no valid clock"
 {&iC_l_[941]}, &QB54_0 };
iC_Gt QB54_2   = { 1, -iC_AND, iC_S_SH, 0, "QB54_2", {&iC_l_[946]}, {&iC_l_[948]}, &QB54_1 };
iC_Gt QB54_3   = { 1, -iC_AND, iC_R_SH, 0, "QB54_3", {&iC_l_[952]},
#warning "iC: Action gate 'QB54_3' has no valid clock"
 {&iC_l_[954]}, &QB54_2 };
iC_Gt QB54_4   = { 1, -iC_OR, iC_ARITH, 0, "QB54_4", {0}, {&iC_l_[958]}, &QB54_3 };
iC_Gt QB55     = { 1, -iC_SH, iC_ARITH, 0, "QB55", {0}, {0}, &QB54_4 };
iC_Gt QB55_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB55_0", {0}, {&iC_l_[962]}, &QB55 };
iC_Gt QB55_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB55_1", {&iC_l_[965]},
#warning "iC: Action gate 'QB55_1' has no valid clock"
 {&iC_l_[967]}, &QB55_0 };
iC_Gt QB55_2   = { 1, -iC_AND, iC_S_SH, 0, "QB55_2", {&iC_l_[972]}, {&iC_l_[974]}, &QB55_1 };
iC_Gt QB55_3   = { 1, -iC_AND, iC_R_SH, 0, "QB55_3", {&iC_l_[978]},
#warning "iC: Action gate 'QB55_3' has no valid clock"
 {&iC_l_[980]}, &QB55_2 };
iC_Gt QB55_4   = { 1, -iC_OR, iC_ARITH, 0, "QB55_4", {0}, {&iC_l_[984]}, &QB55_3 };
iC_Gt QB56     = { 1, -iC_SH, iC_ARITH, 0, "QB56", {0}, {0}, &QB55_4 };
iC_Gt QB56_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB56_0", {0}, {&iC_l_[988]}, &QB56 };
iC_Gt QB56_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB56_1", {&iC_l_[991]},
#warning "iC: Action gate 'QB56_1' has no valid clock"
 {&iC_l_[993]}, &QB56_0 };
iC_Gt QB56_2   = { 1, -iC_AND, iC_S_SH, 0, "QB56_2", {&iC_l_[998]},
#warning "iC: Action gate 'QB56_2' has no valid clock"
 {&iC_l_[1001]}, &QB56_1 };
iC_Gt QB56_3   = { 1, -iC_AND, iC_R_SH, 0, "QB56_3", {&iC_l_[1005]}, {&iC_l_[1007]}, &QB56_2 };
iC_Gt QB56_4   = { 1, -iC_OR, iC_ARITH, 0, "QB56_4", {0}, {&iC_l_[1011]}, &QB56_3 };
iC_Gt QB57     = { 1, -iC_SH, iC_ARITH, 0, "QB57", {0}, {0}, &QB56_4 };
iC_Gt QB57_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB57_0", {0}, {&iC_l_[1015]}, &QB57 };
iC_Gt QB57_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB57_1", {&iC_l_[1018]},
#warning "iC: Action gate 'QB57_1' has no valid clock"
 {&iC_l_[1020]}, &QB57_0 };
iC_Gt QB57_2   = { 1, -iC_AND, iC_S_SH, 0, "QB57_2", {&iC_l_[1025]},
#warning "iC: Action gate 'QB57_2' has no valid clock"
 {&iC_l_[1028]}, &QB57_1 };
iC_Gt QB57_3   = { 1, -iC_AND, iC_R_SH, 0, "QB57_3", {&iC_l_[1032]},
#warning "iC: Action gate 'QB57_3' has no valid clock"
 {&iC_l_[1034]}, &QB57_2 };
iC_Gt QB57_4   = { 1, -iC_OR, iC_ARITH, 0, "QB57_4", {0}, {&iC_l_[1038]}, &QB57_3 };
iC_Gt QB58     = { 1, -iC_SH, iC_ARITH, 0, "QB58", {0}, {0}, &QB57_4 };
iC_Gt QB58_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB58_0", {0}, {&iC_l_[1042]}, &QB58 };
iC_Gt QB58_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB58_1", {&iC_l_[1045]},
#warning "iC: Action gate 'QB58_1' has no valid clock"
 {&iC_l_[1047]}, &QB58_0 };
iC_Gt QB58_2   = { 1, -iC_AND, iC_S_SH, 0, "QB58_2", {&iC_l_[1052]},
#warning "iC: Action gate 'QB58_2' has no valid clock"
 {&iC_l_[1055]}, &QB58_1 };
iC_Gt QB58_3   = { 1, -iC_AND, iC_R_SH, 0, "QB58_3", {&iC_l_[1059]},
#warning "iC: Action gate 'QB58_3' has no valid clock"
 {&iC_l_[1061]}, &QB58_2 };
iC_Gt QB58_4   = { 1, -iC_OR, iC_ARITH, 0, "QB58_4", {0}, {&iC_l_[1065]}, &QB58_3 };
iC_Gt QB59     = { 1, -iC_SH, iC_ARITH, 0, "QB59", {0}, {0}, &QB58_4 };
iC_Gt QB59_0   = { 1, -iC_ARN, iC_OUTW, 0, "QB59_0", {0}, {&iC_l_[1069]}, &QB59 };
iC_Gt QB59_1   = { 1, -iC_ARN, iC_D_SH, 0, "QB59_1", {&iC_l_[1072]},
#warning "iC: Action gate 'QB59_1' has no valid clock"
 {&iC_l_[1074]}, &QB59_0 };
iC_Gt QB59_2   = { 1, -iC_AND, iC_S_SH, 0, "QB59_2", {&iC_l_[1079]},
#warning "iC: Action gate 'QB59_2' has no valid clock"
 {&iC_l_[1082]}, &QB59_1 };
iC_Gt QB59_3   = { 1, -iC_AND, iC_R_SH, 0, "QB59_3", {&iC_l_[1086]},
#warning "iC: Action gate 'QB59_3' has no valid clock"
 {&iC_l_[1088]}, &QB59_2 };
iC_Gt QB59_4   = { 1, -iC_OR, iC_ARITH, 0, "QB59_4", {0}, {&iC_l_[1092]}, &QB59_3 };
iC_Gt QX0_1    = { 1, -iC_LATCH, iC_GATE, 0, "QX0.1", {0}, {&iC_l_[1096]}, &QB59_4 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[1101]}, &QX0_1 };
iC_Gt QX0_3    = { 1, -iC_LATCH, iC_GATE, 0, "QX0.3", {0}, {&iC_l_[1104]}, &QX0_1_0 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[1109]}, &QX0_3 };
iC_Gt QX0_4    = { 1, -iC_FF, iC_GATE, 0, "QX0.4", {0}, {0}, &QX0_3_0 };
iC_Gt QX0_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.4_0", {0}, {&iC_l_[1112]}, &QX0_4 };
iC_Gt QX0_4_1  = { 1, -iC_OR, iC_D_FF, 0, "QX0.4_1", {&iC_l_[1115]}, {&iC_l_[1117]}, &QX0_4_0 };
iC_Gt QX0_5    = { 1, -iC_FF, iC_GATE, 0, "QX0.5", {0}, {0}, &QX0_4_1 };
iC_Gt QX0_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.5_0", {0}, {&iC_l_[1120]}, &QX0_5 };
iC_Gt QX0_5_1  = { 1, -iC_OR, iC_D_FF, 0, "QX0.5_1", {&iC_l_[1123]},
#warning "iC: Action gate 'QX0.5_1' has no valid clock"
 {&iC_l_[1125]}, &QX0_5_0 };
iC_Gt QX0_6    = { 1, -iC_FF, iC_GATE, 0, "QX0.6", {0}, {0}, &QX0_5_1 };
iC_Gt QX0_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.6_0", {0}, {&iC_l_[1128]}, &QX0_6 };
iC_Gt QX0_6_1  = { 1, -iC_OR, iC_D_FF, 0, "QX0.6_1", {&iC_l_[1131]},
#warning "iC: Action gate 'QX0.6_1' has no valid clock"
 {&iC_l_[1133]}, &QX0_6_0 };
iC_Gt QX0_7    = { 1, -iC_FF, iC_GATE, 0, "QX0.7", {0}, {0}, &QX0_6_1 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[1136]}, &QX0_7 };
iC_Gt QX0_7_1  = { 1, -iC_OR, iC_D_FF, 0, "QX0.7_1", {&iC_l_[1139]},
#warning "iC: Action gate 'QX0.7_1' has no valid clock"
 {&iC_l_[1141]}, &QX0_7_0 };
iC_Gt QX1_0    = { 1, -iC_FF, iC_GATE, 0, "QX1.0", {0}, {0}, &QX0_7_1 };
iC_Gt QX1_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.0_0", {0}, {&iC_l_[1144]}, &QX1_0 };
iC_Gt QX1_0_1  = { 1, -iC_OR, iC_D_FF, 0, "QX1.0_1", {&iC_l_[1147]}, {&iC_l_[1149]}, &QX1_0_0 };
iC_Gt QX1_0_2  = { 1, -iC_OR, iC_R_FF, 0, "QX1.0_2", {&iC_l_[1153]}, {&iC_l_[1155]}, &QX1_0_1 };
iC_Gt QX1_0_3  = { 1, -iC_AND, iC_GATE, 0, "QX1.0_3", {0}, {&iC_l_[1158]}, &QX1_0_2 };
iC_Gt QX1_0_4  = { 1, -iC_AND, iC_GATE, 0, "QX1.0_4", {0}, {&iC_l_[1162]}, &QX1_0_3 };
iC_Gt QX1_1    = { 1, -iC_FF, iC_GATE, 0, "QX1.1", {0}, {0}, &QX1_0_4 };
iC_Gt QX1_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.1_0", {0}, {&iC_l_[1166]}, &QX1_1 };
iC_Gt QX1_1_1  = { 1, -iC_OR, iC_D_FF, 0, "QX1.1_1", {&iC_l_[1169]},
#warning "iC: Action gate 'QX1.1_1' has no valid clock"
 {&iC_l_[1171]}, &QX1_1_0 };
iC_Gt QX1_1_2  = { 1, -iC_OR, iC_R_FF, 0, "QX1.1_2", {&iC_l_[1175]},
#warning "iC: Action gate 'QX1.1_2' has no valid clock"
 {&iC_l_[1177]}, &QX1_1_1 };
iC_Gt QX1_1_3  = { 1, -iC_AND, iC_GATE, 0, "QX1.1_3", {0}, {&iC_l_[1180]}, &QX1_1_2 };
iC_Gt QX1_1_4  = { 1, -iC_AND, iC_GATE, 0, "QX1.1_4", {0}, {&iC_l_[1184]}, &QX1_1_3 };
iC_Gt QX1_2    = { 1, -iC_FF, iC_GATE, 0, "QX1.2", {0}, {0}, &QX1_1_4 };
iC_Gt QX1_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.2_0", {0}, {&iC_l_[1188]}, &QX1_2 };
iC_Gt QX1_2_1  = { 1, -iC_OR, iC_D_FF, 0, "QX1.2_1", {&iC_l_[1191]},
#warning "iC: Action gate 'QX1.2_1' has no valid clock"
 {&iC_l_[1193]}, &QX1_2_0 };
iC_Gt QX1_2_2  = { 1, -iC_OR, iC_R_FF, 0, "QX1.2_2", {&iC_l_[1197]},
#warning "iC: Action gate 'QX1.2_2' has no valid clock"
 {&iC_l_[1199]}, &QX1_2_1 };
iC_Gt QX1_2_3  = { 1, -iC_AND, iC_GATE, 0, "QX1.2_3", {0}, {&iC_l_[1202]}, &QX1_2_2 };
iC_Gt QX1_2_4  = { 1, -iC_AND, iC_GATE, 0, "QX1.2_4", {0}, {&iC_l_[1206]}, &QX1_2_3 };
iC_Gt QX1_3    = { 1, -iC_FF, iC_GATE, 0, "QX1.3", {0}, {0}, &QX1_2_4 };
iC_Gt QX1_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.3_0", {0}, {&iC_l_[1210]}, &QX1_3 };
iC_Gt QX1_3_1  = { 1, -iC_OR, iC_D_FF, 0, "QX1.3_1", {&iC_l_[1213]},
#warning "iC: Action gate 'QX1.3_1' has no valid clock"
 {&iC_l_[1215]}, &QX1_3_0 };
iC_Gt QX1_3_2  = { 1, -iC_OR, iC_R_FF, 0, "QX1.3_2", {&iC_l_[1219]},
#warning "iC: Action gate 'QX1.3_2' has no valid clock"
 {&iC_l_[1221]}, &QX1_3_1 };
iC_Gt QX1_3_3  = { 1, -iC_AND, iC_GATE, 0, "QX1.3_3", {0}, {&iC_l_[1224]}, &QX1_3_2 };
iC_Gt QX1_3_4  = { 1, -iC_AND, iC_GATE, 0, "QX1.3_4", {0}, {&iC_l_[1228]}, &QX1_3_3 };
iC_Gt QX1_4    = { 1, -iC_FF, iC_GATE, 0, "QX1.4", {0}, {0}, &QX1_3_4 };
iC_Gt QX1_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.4_0", {0}, {&iC_l_[1232]}, &QX1_4 };
iC_Gt QX1_4_1  = { 1, -iC_OR, iC_D_FF, 0, "QX1.4_1", {&iC_l_[1235]},
#warning "iC: Action gate 'QX1.4_1' has no valid clock"

#error "too many errors - iC compilation aborted"

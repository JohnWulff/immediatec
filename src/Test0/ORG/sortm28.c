/********************************************************************
 *
 *	SOURCE:   ./Test0/sortm28.ic
 *	OUTPUT:   ./Test0/sortm28.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"$Revision: icc_3.3-5-gc2c370a-dirty $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
#define iC_AV(n)	iC_gf->gt_list[n]->gt_new
#define iC_LV(n,c)	((iC_gf->gt_list[n]->gt_val < 0) ^ c ? 1 : 0)
#define iC_AA(n,p,v)	iC_assignA(iC_gf->gt_list[n], p, v)
#define iC_LA(n,c,p,v)	iC_assignL(iC_gf->gt_list[n], c, p, v)
#define iC_AVI(n,i)	iC_index(iC_gf->gt_list[n], i)->gt_new
#define iC_LVI(n,c,i)	((iC_index(iC_gf->gt_list[n], i)->gt_val < 0) ^ c ? 1 : 0)
#define iC_AAI(n,i,p,v)	iC_assignA(iC_index(iC_gf->gt_list[n], i), p, v)
#define iC_LAI(n,c,i,p,v)	iC_assignL(iC_index(iC_gf->gt_list[n], i), c, p, v)
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

static iC_Gt _10     = { 1, -iC_NCONST, iC_ARITH, 0, "10", {0}, {0}, 0 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &_10 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
iC_Gt IX0_4    = { 1, -iC_INPX, iC_GATE, 0, "IX0.4", {0}, {0}, &IX0_3 };
iC_Gt IX0_5    = { 1, -iC_INPX, iC_GATE, 0, "IX0.5", {0}, {0}, &IX0_4 };
iC_Gt IX0_6    = { 1, -iC_INPX, iC_GATE, 0, "IX0.6", {0}, {0}, &IX0_5 };
iC_Gt IX0_7    = { 1, -iC_INPX, iC_GATE, 0, "IX0.7", {0}, {0}, &IX0_6 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[0]}, &IX0_7 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[3]}, &QX0_0_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[6]}, &QX0_1_0 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[9]}, &QX0_2_0 };
iC_Gt QX0_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.4_0", {0}, {&iC_l_[12]}, &QX0_3_0 };
iC_Gt QX0_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.5_0", {0}, {&iC_l_[15]}, &QX0_4_0 };
iC_Gt QX0_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.6_0", {0}, {&iC_l_[18]}, &QX0_5_0 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[21]}, &QX0_6_0 };
iC_Gt TX0_4    = { 1, -iC_INPX, iC_GATE, 0, "TX0.4", {0}, {0}, &QX0_7_0 };
static iC_Gt _f1_1   = { 1, -iC_OR, iC_F_CE, 0, "_f1_1", {&iC_l_[24]}, {&iC_l_[29]}, &TX0_4 };
iC_Gt busa     = { 1, -iC_ARN, iC_ARITH, 0, "busa", {0}, {&iC_l_[32]}, &_f1_1 };
iC_Gt busa_1   = { 1, -iC_OR, iC_ARITH, 0, "busa_1", {0}, {&iC_l_[50]}, &busa };
iC_Gt busa_2   = { 1, -iC_OR, iC_ARITH, 0, "busa_2", {0}, {&iC_l_[53]}, &busa_1 };
iC_Gt busa_3   = { 1, -iC_OR, iC_ARITH, 0, "busa_3", {0}, {&iC_l_[56]}, &busa_2 };
iC_Gt busa_4   = { 1, -iC_OR, iC_ARITH, 0, "busa_4", {0}, {&iC_l_[59]}, &busa_3 };
iC_Gt busa_5   = { 1, -iC_OR, iC_ARITH, 0, "busa_5", {0}, {&iC_l_[62]}, &busa_4 };
iC_Gt busa_6   = { 1, -iC_OR, iC_ARITH, 0, "busa_6", {0}, {&iC_l_[65]}, &busa_5 };
iC_Gt busa_7   = { 1, -iC_OR, iC_ARITH, 0, "busa_7", {0}, {&iC_l_[68]}, &busa_6 };
iC_Gt busa_8   = { 1, -iC_OR, iC_ARITH, 0, "busa_8", {0}, {&iC_l_[71]}, &busa_7 };
iC_Gt del      = { 1, -iC_ARN, iC_ARITH, 0, "del", {0}, {&iC_l_[74]}, &busa_8 };
iC_Gt dest     = { 1, -iC_ARN, iC_ARITH, 0, "dest", {0}, {&iC_l_[77]}, &del };
iC_Gt dest_1   = { 1, -iC_OR, iC_ARITH, 0, "dest_1", {0}, {&iC_l_[82]}, &dest };
iC_Gt dest_2   = { 1, -iC_OR, iC_ARITH, 0, "dest_2", {0}, {&iC_l_[85]}, &dest_1 };
iC_Gt dest_3   = { 1, -iC_OR, iC_ARITH, 0, "dest_3", {0}, {&iC_l_[88]}, &dest_2 };
iC_Gt dt0      = { 1, -iC_EF, iC_GATE, 0, "dt0", {0}, {0}, &dest_3 };
iC_Gt dt0_1    = { 1, -iC_OR, iC_RI_BIT, 0, "dt0_1", {&iC_l_[91]}, {&iC_l_[93]}, &dt0 };
iC_Gt dt1      = { 1, -iC_EF, iC_GATE, 0, "dt1", {0}, {0}, &dt0_1 };
iC_Gt dt1_1    = { 1, -iC_OR, iC_RI_BIT, 0, "dt1_1", {&iC_l_[96]}, {&iC_l_[98]}, &dt1 };
iC_Gt dt2      = { 1, -iC_EF, iC_GATE, 0, "dt2", {0}, {0}, &dt1_1 };
iC_Gt dt2_1    = { 1, -iC_OR, iC_RI_BIT, 0, "dt2_1", {&iC_l_[101]}, {&iC_l_[103]}, &dt2 };
iC_Gt dt3      = { 1, -iC_EF, iC_GATE, 0, "dt3", {0}, {0}, &dt2_1 };
iC_Gt dt3_1    = { 1, -iC_OR, iC_RI_BIT, 0, "dt3_1", {&iC_l_[106]}, {&iC_l_[108]}, &dt3 };
iC_Gt dt4      = { 1, -iC_EF, iC_GATE, 0, "dt4", {0}, {0}, &dt3_1 };
iC_Gt dt4_1    = { 1, -iC_OR, iC_RI_BIT, 0, "dt4_1", {&iC_l_[111]}, {&iC_l_[113]}, &dt4 };
iC_Gt dt5      = { 1, -iC_EF, iC_GATE, 0, "dt5", {0}, {0}, &dt4_1 };
iC_Gt dt5_1    = { 1, -iC_OR, iC_RI_BIT, 0, "dt5_1", {&iC_l_[116]}, {&iC_l_[118]}, &dt5 };
iC_Gt dt6      = { 1, -iC_EF, iC_GATE, 0, "dt6", {0}, {0}, &dt5_1 };
iC_Gt dt6_1    = { 1, -iC_OR, iC_RI_BIT, 0, "dt6_1", {&iC_l_[121]}, {&iC_l_[123]}, &dt6 };
iC_Gt dt7      = { 1, -iC_EF, iC_GATE, 0, "dt7", {0}, {0}, &dt6_1 };
iC_Gt dt7_1    = { 1, -iC_OR, iC_RI_BIT, 0, "dt7_1", {&iC_l_[126]}, {&iC_l_[128]}, &dt7 };
iC_Gt gate0    = { 1, -iC_FF, iC_GATE, 0, "gate0", {0}, {0}, &dt7_1 };
iC_Gt gate0_1  = { 1, -iC_OR, iC_S_FF, 0, "gate0_1", {&iC_l_[131]}, {&iC_l_[133]}, &gate0 };
iC_Gt gate0_2  = { 1, -iC_OR, iC_R_FF, 0, "gate0_2", {&iC_l_[136]}, {&iC_l_[139]}, &gate0_1 };
iC_Gt gate1    = { 1, -iC_FF, iC_GATE, 0, "gate1", {0}, {0}, &gate0_2 };
iC_Gt gate1_1  = { 1, -iC_OR, iC_S_FF, 0, "gate1_1", {&iC_l_[142]}, {&iC_l_[144]}, &gate1 };
iC_Gt gate1_2  = { 1, -iC_OR, iC_R_FF, 0, "gate1_2", {&iC_l_[147]}, {&iC_l_[150]}, &gate1_1 };
iC_Gt gate2    = { 1, -iC_FF, iC_GATE, 0, "gate2", {0}, {0}, &gate1_2 };
iC_Gt gate2_1  = { 1, -iC_OR, iC_S_FF, 0, "gate2_1", {&iC_l_[153]}, {&iC_l_[155]}, &gate2 };
iC_Gt gate2_2  = { 1, -iC_OR, iC_R_FF, 0, "gate2_2", {&iC_l_[158]}, {&iC_l_[161]}, &gate2_1 };
iC_Gt gate3    = { 1, -iC_FF, iC_GATE, 0, "gate3", {0}, {0}, &gate2_2 };
iC_Gt gate3_1  = { 1, -iC_OR, iC_S_FF, 0, "gate3_1", {&iC_l_[164]}, {&iC_l_[166]}, &gate3 };
iC_Gt gate3_2  = { 1, -iC_OR, iC_R_FF, 0, "gate3_2", {&iC_l_[169]}, {&iC_l_[172]}, &gate3_1 };
iC_Gt gate4    = { 1, -iC_FF, iC_GATE, 0, "gate4", {0}, {0}, &gate3_2 };
iC_Gt gate4_1  = { 1, -iC_OR, iC_S_FF, 0, "gate4_1", {&iC_l_[175]}, {&iC_l_[177]}, &gate4 };
iC_Gt gate4_2  = { 1, -iC_OR, iC_R_FF, 0, "gate4_2", {&iC_l_[180]}, {&iC_l_[183]}, &gate4_1 };
iC_Gt gate5    = { 1, -iC_FF, iC_GATE, 0, "gate5", {0}, {0}, &gate4_2 };
iC_Gt gate5_1  = { 1, -iC_OR, iC_S_FF, 0, "gate5_1", {&iC_l_[186]}, {&iC_l_[188]}, &gate5 };
iC_Gt gate5_2  = { 1, -iC_OR, iC_R_FF, 0, "gate5_2", {&iC_l_[191]}, {&iC_l_[194]}, &gate5_1 };
iC_Gt gate6    = { 1, -iC_FF, iC_GATE, 0, "gate6", {0}, {0}, &gate5_2 };
iC_Gt gate6_1  = { 1, -iC_OR, iC_S_FF, 0, "gate6_1", {&iC_l_[197]}, {&iC_l_[199]}, &gate6 };
iC_Gt gate6_2  = { 1, -iC_OR, iC_R_FF, 0, "gate6_2", {&iC_l_[202]}, {&iC_l_[205]}, &gate6_1 };
iC_Gt gate7    = { 1, -iC_FF, iC_GATE, 0, "gate7", {0}, {0}, &gate6_2 };
iC_Gt gate7_1  = { 1, -iC_OR, iC_S_FF, 0, "gate7_1", {&iC_l_[208]}, {&iC_l_[210]}, &gate7 };
iC_Gt gate7_2  = { 1, -iC_OR, iC_R_FF, 0, "gate7_2", {&iC_l_[213]}, {&iC_l_[216]}, &gate7_1 };
iC_Gt inp0     = { 1, -iC_OR, iC_GATE, 0, "inp0", {0}, {&iC_l_[219]}, &gate7_2 };
iC_Gt inp1     = { 1, -iC_OR, iC_GATE, 0, "inp1", {0}, {&iC_l_[225]}, &inp0 };
iC_Gt inp2     = { 1, -iC_OR, iC_GATE, 0, "inp2", {0}, {&iC_l_[231]}, &inp1 };
iC_Gt ma0      = { 1, -iC_SH, iC_ARITH, 0, "ma0", {0}, {0}, &inp2 };
iC_Gt ma0_1    = { 1, -iC_ARN, iC_D_SH, 0, "ma0_1", {&iC_l_[237]}, {&iC_l_[239]}, &ma0 };
iC_Gt ma0_2    = { 1, -iC_OR, iC_ARITH, 0, "ma0_2", {0}, {&iC_l_[244]}, &ma0_1 };
iC_Gt ma1      = { 1, -iC_SH, iC_ARITH, 0, "ma1", {0}, {0}, &ma0_2 };
iC_Gt ma1_1    = { 1, -iC_ARN, iC_D_SH, 0, "ma1_1", {&iC_l_[247]}, {&iC_l_[249]}, &ma1 };
iC_Gt ma1_2    = { 1, -iC_OR, iC_ARITH, 0, "ma1_2", {0}, {&iC_l_[254]}, &ma1_1 };
iC_Gt ma2      = { 1, -iC_SH, iC_ARITH, 0, "ma2", {0}, {0}, &ma1_2 };
iC_Gt ma2_1    = { 1, -iC_ARN, iC_D_SH, 0, "ma2_1", {&iC_l_[257]}, {&iC_l_[259]}, &ma2 };
iC_Gt ma2_2    = { 1, -iC_OR, iC_ARITH, 0, "ma2_2", {0}, {&iC_l_[264]}, &ma2_1 };
iC_Gt ma3      = { 1, -iC_SH, iC_ARITH, 0, "ma3", {0}, {0}, &ma2_2 };
iC_Gt ma3_1    = { 1, -iC_ARN, iC_D_SH, 0, "ma3_1", {&iC_l_[267]}, {&iC_l_[269]}, &ma3 };
iC_Gt ma3_2    = { 1, -iC_OR, iC_ARITH, 0, "ma3_2", {0}, {&iC_l_[274]}, &ma3_1 };
iC_Gt ma4      = { 1, -iC_SH, iC_ARITH, 0, "ma4", {0}, {0}, &ma3_2 };
iC_Gt ma4_1    = { 1, -iC_ARN, iC_D_SH, 0, "ma4_1", {&iC_l_[277]}, {&iC_l_[279]}, &ma4 };
iC_Gt ma4_2    = { 1, -iC_OR, iC_ARITH, 0, "ma4_2", {0}, {&iC_l_[284]}, &ma4_1 };
iC_Gt ma5      = { 1, -iC_SH, iC_ARITH, 0, "ma5", {0}, {0}, &ma4_2 };
iC_Gt ma5_1    = { 1, -iC_ARN, iC_D_SH, 0, "ma5_1", {&iC_l_[287]}, {&iC_l_[289]}, &ma5 };
iC_Gt ma5_2    = { 1, -iC_OR, iC_ARITH, 0, "ma5_2", {0}, {&iC_l_[294]}, &ma5_1 };
iC_Gt ma6      = { 1, -iC_SH, iC_ARITH, 0, "ma6", {0}, {0}, &ma5_2 };
iC_Gt ma6_1    = { 1, -iC_ARN, iC_D_SH, 0, "ma6_1", {&iC_l_[297]}, {&iC_l_[299]}, &ma6 };
iC_Gt ma6_2    = { 1, -iC_OR, iC_ARITH, 0, "ma6_2", {0}, {&iC_l_[304]}, &ma6_1 };
iC_Gt ma7      = { 1, -iC_SH, iC_ARITH, 0, "ma7", {0}, {0}, &ma6_2 };
iC_Gt ma7_1    = { 1, -iC_ARN, iC_D_SH, 0, "ma7_1", {&iC_l_[307]}, {&iC_l_[309]}, &ma7 };
iC_Gt ma7_2    = { 1, -iC_OR, iC_ARITH, 0, "ma7_2", {0}, {&iC_l_[314]}, &ma7_1 };
iC_Gt openGate = { 1, -iC_LOGC, iC_UDFA, 0, "openGate", {0}, {&iC_l_[317]}, &ma7_2, 8 };
iC_Gt openGate0 = { 1, -iC_LOGC, iC_GATE, 0, "openGate0", {0}, {0}, &openGate };
iC_Gt openGate1 = { 1, -iC_LOGC, iC_GATE, 0, "openGate1", {0}, {0}, &openGate0 };
iC_Gt openGate2 = { 1, -iC_LOGC, iC_GATE, 0, "openGate2", {0}, {0}, &openGate1 };
iC_Gt openGate3 = { 1, -iC_LOGC, iC_GATE, 0, "openGate3", {0}, {0}, &openGate2 };
iC_Gt openGate4 = { 1, -iC_LOGC, iC_GATE, 0, "openGate4", {0}, {0}, &openGate3 };
iC_Gt openGate5 = { 1, -iC_LOGC, iC_GATE, 0, "openGate5", {0}, {0}, &openGate4 };
iC_Gt openGate6 = { 1, -iC_LOGC, iC_GATE, 0, "openGate6", {0}, {0}, &openGate5 };
iC_Gt openGate7 = { 1, -iC_LOGC, iC_GATE, 0, "openGate7", {0}, {0}, &openGate6 };
iC_Gt pt0      = { 1, -iC_FF, iC_GATE, 0, "pt0", {0}, {0}, &openGate7 };
iC_Gt pt0_1    = { 1, -iC_OR, iC_D_FF, 0, "pt0_1", {&iC_l_[326]}, {&iC_l_[329]}, &pt0 };
iC_Gt pt1      = { 1, -iC_FF, iC_GATE, 0, "pt1", {0}, {0}, &pt0_1 };
iC_Gt pt1_1    = { 1, -iC_OR, iC_D_FF, 0, "pt1_1", {&iC_l_[332]}, {&iC_l_[335]}, &pt1 };
iC_Gt pt2      = { 1, -iC_FF, iC_GATE, 0, "pt2", {0}, {0}, &pt1_1 };
iC_Gt pt2_1    = { 1, -iC_OR, iC_D_FF, 0, "pt2_1", {&iC_l_[338]}, {&iC_l_[341]}, &pt2 };
iC_Gt pt3      = { 1, -iC_FF, iC_GATE, 0, "pt3", {0}, {0}, &pt2_1 };
iC_Gt pt3_1    = { 1, -iC_OR, iC_D_FF, 0, "pt3_1", {&iC_l_[344]}, {&iC_l_[347]}, &pt3 };
iC_Gt pt4      = { 1, -iC_FF, iC_GATE, 0, "pt4", {0}, {0}, &pt3_1 };
iC_Gt pt4_1    = { 1, -iC_OR, iC_D_FF, 0, "pt4_1", {&iC_l_[350]}, {&iC_l_[353]}, &pt4 };
iC_Gt pt5      = { 1, -iC_FF, iC_GATE, 0, "pt5", {0}, {0}, &pt4_1 };
iC_Gt pt5_1    = { 1, -iC_OR, iC_D_FF, 0, "pt5_1", {&iC_l_[356]}, {&iC_l_[359]}, &pt5 };
iC_Gt pt6      = { 1, -iC_FF, iC_GATE, 0, "pt6", {0}, {0}, &pt5_1 };
iC_Gt pt6_1    = { 1, -iC_OR, iC_D_FF, 0, "pt6_1", {&iC_l_[362]}, {&iC_l_[365]}, &pt6 };
iC_Gt pt7      = { 1, -iC_FF, iC_GATE, 0, "pt7", {0}, {0}, &pt6_1 };
iC_Gt pt7_1    = { 1, -iC_OR, iC_D_FF, 0, "pt7_1", {&iC_l_[368]}, {&iC_l_[371]}, &pt7 };
iC_Gt tick     = { 1, -iC_TIM, iC_TIMRL, 0, "tick", {0}, {0}, &pt7_1 };
iC_Gt tick_1   = { 1, -iC_OR, iC_TIMR, 0, "tick_1", {&iC_l_[374]}, {&iC_l_[376]}, &tick };
iC_Gt tick_2   = { 1, -iC_OR, iC_TIMR, 0, "tick_2", {&iC_l_[379]}, {&iC_l_[381]}, &tick_1 };
iC_Gt tr0      = { 1, -iC_FF, iC_GATE, 0, "tr0", {0}, {0}, &tick_2 };
iC_Gt tr0_1    = { 1, -iC_AND, iC_S_FF, 0, "tr0_1", {&iC_l_[384]}, {&iC_l_[386]}, &tr0 };
iC_Gt tr0_2    = { 1, -iC_OR, iC_R_FF, 0, "tr0_2", {&iC_l_[390]}, {&iC_l_[392]}, &tr0_1 };
iC_Gt tr1      = { 1, -iC_FF, iC_GATE, 0, "tr1", {0}, {0}, &tr0_2 };
iC_Gt tr1_1    = { 1, -iC_AND, iC_S_FF, 0, "tr1_1", {&iC_l_[395]}, {&iC_l_[397]}, &tr1 };
iC_Gt tr1_2    = { 1, -iC_OR, iC_R_FF, 0, "tr1_2", {&iC_l_[402]}, {&iC_l_[404]}, &tr1_1 };
iC_Gt tr2      = { 1, -iC_FF, iC_GATE, 0, "tr2", {0}, {0}, &tr1_2 };
iC_Gt tr2_1    = { 1, -iC_AND, iC_S_FF, 0, "tr2_1", {&iC_l_[407]}, {&iC_l_[409]}, &tr2 };
iC_Gt tr2_2    = { 1, -iC_OR, iC_R_FF, 0, "tr2_2", {&iC_l_[415]}, {&iC_l_[417]}, &tr2_1 };
iC_Gt tr3      = { 1, -iC_FF, iC_GATE, 0, "tr3", {0}, {0}, &tr2_2 };
iC_Gt tr3_1    = { 1, -iC_AND, iC_S_FF, 0, "tr3_1", {&iC_l_[420]}, {&iC_l_[422]}, &tr3 };
iC_Gt tr3_2    = { 1, -iC_OR, iC_R_FF, 0, "tr3_2", {&iC_l_[429]}, {&iC_l_[431]}, &tr3_1 };
iC_Gt tr4      = { 1, -iC_FF, iC_GATE, 0, "tr4", {0}, {0}, &tr3_2 };
iC_Gt tr4_1    = { 1, -iC_AND, iC_S_FF, 0, "tr4_1", {&iC_l_[434]}, {&iC_l_[436]}, &tr4 };
iC_Gt tr4_2    = { 1, -iC_OR, iC_R_FF, 0, "tr4_2", {&iC_l_[444]}, {&iC_l_[446]}, &tr4_1 };
iC_Gt tr5      = { 1, -iC_FF, iC_GATE, 0, "tr5", {0}, {0}, &tr4_2 };
iC_Gt tr5_1    = { 1, -iC_AND, iC_S_FF, 0, "tr5_1", {&iC_l_[449]}, {&iC_l_[451]}, &tr5 };
iC_Gt tr5_2    = { 1, -iC_OR, iC_R_FF, 0, "tr5_2", {&iC_l_[460]}, {&iC_l_[462]}, &tr5_1 };
iC_Gt tr6      = { 1, -iC_FF, iC_GATE, 0, "tr6", {0}, {0}, &tr5_2 };
iC_Gt tr6_1    = { 1, -iC_AND, iC_S_FF, 0, "tr6_1", {&iC_l_[465]}, {&iC_l_[467]}, &tr6 };
iC_Gt tr6_2    = { 1, -iC_OR, iC_R_FF, 0, "tr6_2", {&iC_l_[477]}, {&iC_l_[479]}, &tr6_1 };
iC_Gt tr7      = { 1, -iC_FF, iC_GATE, 0, "tr7", {0}, {0}, &tr6_2 };
iC_Gt tr7_1    = { 1, -iC_AND, iC_S_FF, 0, "tr7_1", {&iC_l_[482]}, {&iC_l_[484]}, &tr7 };
iC_Gt tr7_2    = { 1, -iC_OR, iC_R_FF, 0, "tr7_2", {&iC_l_[495]}, {&iC_l_[497]}, &tr7_1 };
iC_Gt trans    = { 1, -iC_EF, iC_GATE, 0, "trans", {0}, {0}, &tr7_2 };
iC_Gt trans_1  = { 1, -iC_OR, iC_RI_BIT, 0, "trans_1", {&iC_l_[500]}, {&iC_l_[502]}, &trans };
iC_Gt trig     = { 1, -iC_OR, iC_GATE, 0, "trig", {0}, {&iC_l_[512]}, &trans_1 };
iC_Gt xt0      = { 1, -iC_AND, iC_GATE, 0, "xt0", {0}, {&iC_l_[522]}, &trig };
iC_Gt xt1      = { 1, -iC_AND, iC_GATE, 0, "xt1", {0}, {&iC_l_[526]}, &xt0 };
iC_Gt xt2      = { 1, -iC_AND, iC_GATE, 0, "xt2", {0}, {&iC_l_[531]}, &xt1 };
iC_Gt xt3      = { 1, -iC_AND, iC_GATE, 0, "xt3", {0}, {&iC_l_[537]}, &xt2 };
iC_Gt xt4      = { 1, -iC_AND, iC_GATE, 0, "xt4", {0}, {&iC_l_[544]}, &xt3 };
iC_Gt xt5      = { 1, -iC_AND, iC_GATE, 0, "xt5", {0}, {&iC_l_[552]}, &xt4 };
iC_Gt xt6      = { 1, -iC_AND, iC_GATE, 0, "xt6", {0}, {&iC_l_[561]}, &xt5 };
iC_Gt xt7      = { 1, -iC_AND, iC_GATE, 0, "xt7", {0}, {&iC_l_[571]}, &xt6 };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&gate0}, &xt7, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&gate1}, &QX0_0, 0 };
iC_Gt QX0_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.2", {0}, {(iC_Gt**)&gate2}, &QX0_1, 0 };
iC_Gt QX0_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.3", {0}, {(iC_Gt**)&gate3}, &QX0_2, 0 };
iC_Gt QX0_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.4", {0}, {(iC_Gt**)&gate4}, &QX0_3, 0 };
iC_Gt QX0_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.5", {0}, {(iC_Gt**)&gate5}, &QX0_4, 0 };
iC_Gt QX0_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.6", {0}, {(iC_Gt**)&gate6}, &QX0_5, 0 };
iC_Gt QX0_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.7", {0}, {(iC_Gt**)&gate7}, &QX0_6, 0 };

iC_Gt *		iC___Test0_sortm28_list = &QX0_7;
iC_Gt **	iC_list[] = { &iC___Test0_sortm28_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 125 "./Test0/sortm28.ic"

static int	temp;

#line 216 "./Test0/sortm28.c"

static int iC_1(iC_Gt * iC_gf) {
#line 139 "init_t.ic"	/* in pre-compiled function block SHR */
	return iC_MV(1)?iC_MV(2):iC_MV(3);
#line 221 "./Test0/sortm28.c"
} /* iC_1 */

static int iC_2(iC_Gt * iC_gf) {
#line 32 "./Test0/sortm28.ic"
	return iC_MV(1)+iC_MV(2)*2+iC_MV(3)*4;
#line 227 "./Test0/sortm28.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
#line 43 "./Test0/sortm28.ic"
	return 12+iC_MV(1)*34/3;
#line 233 "./Test0/sortm28.c"
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
#line 119 "./Test0/sortm28.ic"
	return iC_MV(1)?iC_MV(2):iC_MV(3)?iC_MV(4):iC_MV(5)?iC_MV(6):iC_MV(7)?iC_MV(8):iC_MV(9)?iC_MV(10):iC_MV(11)?iC_MV(12):iC_MV(13)?iC_MV(14):iC_MV(15)?iC_MV(16):0;
#line 239 "./Test0/sortm28.c"
} /* iC_4 */

static int iC_5(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 129 "./Test0/sortm28.ic"
{
    iC_LAI(2, 0, temp = iC_AV(3) , 0,  1);
}
#line 248 "./Test0/sortm28.c"
    else
#line 131 "./Test0/sortm28.ic"
{
    iC_LAI(2, 0, temp , 0,  0);	/* temp is changed on the next rising trig */
}
#line 254 "./Test0/sortm28.c"
    return 0;
} /* iC_5 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QX0.0_0 */	&gate0, 0, 0,
/* QX0.1_0 */	&gate1, 0, 0,
/* QX0.2_0 */	&gate2, 0, 0,
/* QX0.3_0 */	&gate3, 0, 0,
/* QX0.4_0 */	&gate4, 0, 0,
/* QX0.5_0 */	&gate5, 0, 0,
/* QX0.6_0 */	&gate6, 0, 0,
/* QX0.7_0 */	&gate7, 0, 0,
/* _f1_1 */	(iC_Gt*)iC_5, &iClock, &openGate, &busa, (iC_Gt*)0x9,
		&trig, 0, 0,
/* busa */	(iC_Gt*)iC_4, &busa_1, &ma0, &busa_2, &ma1, &busa_3,
		&ma2, &busa_4, &ma3, &busa_5, &ma4, &busa_6, &ma5,
		&busa_7, &ma6, &busa_8, &ma7, 0,
/* busa_1 */	&xt0, 0, 0,
/* busa_2 */	&xt1, 0, 0,
/* busa_3 */	&xt2, 0, 0,
/* busa_4 */	&xt3, 0, 0,
/* busa_5 */	&xt4, 0, 0,
/* busa_6 */	&xt5, 0, 0,
/* busa_7 */	&xt6, 0, 0,
/* busa_8 */	&xt7, 0, 0,
/* del */	(iC_Gt*)iC_3, &dest, 0,
/* dest */	(iC_Gt*)iC_2, &dest_1, &dest_2, &dest_3, 0,
/* dest_1 */	&inp0, 0, 0,
/* dest_2 */	&inp1, 0, 0,
/* dest_3 */	&inp2, 0, 0,
/* dt0_1 */	&dt0, &iClock,	&tr0, 0, 0,
/* dt1_1 */	&dt1, &iClock,	&tr1, 0, 0,
/* dt2_1 */	&dt2, &iClock,	&tr2, 0, 0,
/* dt3_1 */	&dt3, &iClock,	&tr3, 0, 0,
/* dt4_1 */	&dt4, &iClock,	&tr4, 0, 0,
/* dt5_1 */	&dt5, &iClock,	&tr5, 0, 0,
/* dt6_1 */	&dt6, &iClock,	&tr6, 0, 0,
/* dt7_1 */	&dt7, &iClock,	&tr7, 0, 0,
/* gate0_1 */	&gate0, &iClock,	&openGate0, 0, 0,
/* gate0_2 */	&gate0, &tick, &_10,	&gate0, 0, 0,
/* gate1_1 */	&gate1, &iClock,	&openGate1, 0, 0,
/* gate1_2 */	&gate1, &tick, &_10,	&gate1, 0, 0,
/* gate2_1 */	&gate2, &iClock,	&openGate2, 0, 0,
/* gate2_2 */	&gate2, &tick, &_10,	&gate2, 0, 0,
/* gate3_1 */	&gate3, &iClock,	&openGate3, 0, 0,
/* gate3_2 */	&gate3, &tick, &_10,	&gate3, 0, 0,
/* gate4_1 */	&gate4, &iClock,	&openGate4, 0, 0,
/* gate4_2 */	&gate4, &tick, &_10,	&gate4, 0, 0,
/* gate5_1 */	&gate5, &iClock,	&openGate5, 0, 0,
/* gate5_2 */	&gate5, &tick, &_10,	&gate5, 0, 0,
/* gate6_1 */	&gate6, &iClock,	&openGate6, 0, 0,
/* gate6_2 */	&gate6, &tick, &_10,	&gate6, 0, 0,
/* gate7_1 */	&gate7, &iClock,	&openGate7, 0, 0,
/* gate7_2 */	&gate7, &tick, &_10,	&gate7, 0, 0,
/* inp0 */	&IX0_7, &IX0_5, &IX0_3, &IX0_1, 0, 0,
/* inp1 */	&IX0_7, &IX0_6, &IX0_3, &IX0_2, 0, 0,
/* inp2 */	&IX0_7, &IX0_6, &IX0_5, &IX0_4, 0, 0,
/* ma0_1 */	&ma0, &iClock,	(iC_Gt*)iC_1, &ma0_2, &dest, &ma0, 0,
/* ma0_2 */	&dt0, 0, 0,
/* ma1_1 */	&ma1, &iClock,	(iC_Gt*)iC_1, &ma1_2, &dest, &ma1, 0,
/* ma1_2 */	&dt1, 0, 0,
/* ma2_1 */	&ma2, &iClock,	(iC_Gt*)iC_1, &ma2_2, &dest, &ma2, 0,
/* ma2_2 */	&dt2, 0, 0,
/* ma3_1 */	&ma3, &iClock,	(iC_Gt*)iC_1, &ma3_2, &dest, &ma3, 0,
/* ma3_2 */	&dt3, 0, 0,
/* ma4_1 */	&ma4, &iClock,	(iC_Gt*)iC_1, &ma4_2, &dest, &ma4, 0,
/* ma4_2 */	&dt4, 0, 0,
/* ma5_1 */	&ma5, &iClock,	(iC_Gt*)iC_1, &ma5_2, &dest, &ma5, 0,
/* ma5_2 */	&dt5, 0, 0,
/* ma6_1 */	&ma6, &iClock,	(iC_Gt*)iC_1, &ma6_2, &dest, &ma6, 0,
/* ma6_2 */	&dt6, 0, 0,
/* ma7_1 */	&ma7, &iClock,	(iC_Gt*)iC_1, &ma7_2, &dest, &ma7, 0,
/* ma7_2 */	&dt7, 0, 0,
/* openGate */	&openGate0, &openGate1, &openGate2, &openGate3, &openGate4, &openGate5, &openGate6, &openGate7, 0,
/* pt0_1 */	&pt0, &tick, &del,	&tr0, 0, 0,
/* pt1_1 */	&pt1, &tick, &del,	&tr1, 0, 0,
/* pt2_1 */	&pt2, &tick, &del,	&tr2, 0, 0,
/* pt3_1 */	&pt3, &tick, &del,	&tr3, 0, 0,
/* pt4_1 */	&pt4, &tick, &del,	&tr4, 0, 0,
/* pt5_1 */	&pt5, &tick, &del,	&tr5, 0, 0,
/* pt6_1 */	&pt6, &tick, &del,	&tr6, 0, 0,
/* pt7_1 */	&pt7, &tick, &del,	&tr7, 0, 0,
/* tick_1 */	&tick, &iClock,	&TX0_4, 0, 0,
/* tick_2 */	&tick, &iClock,	0, &TX0_4, 0,
/* tr0_1 */	&tr0, &iClock,	&trans, 0, &tr0, 0,
/* tr0_2 */	&tr0, &iClock,	&xt0, 0, 0,
/* tr1_1 */	&tr1, &iClock,	&trans, &tr0, 0, &tr1, 0,
/* tr1_2 */	&tr1, &iClock,	&xt1, 0, 0,
/* tr2_1 */	&tr2, &iClock,	&trans, &tr1, &tr0, 0, &tr2, 0,
/* tr2_2 */	&tr2, &iClock,	&xt2, 0, 0,
/* tr3_1 */	&tr3, &iClock,	&trans, &tr2, &tr1, &tr0, 0, &tr3, 0,
/* tr3_2 */	&tr3, &iClock,	&xt3, 0, 0,
/* tr4_1 */	&tr4, &iClock,	&trans, &tr3, &tr2, &tr1, &tr0, 0, &tr4, 0,
/* tr4_2 */	&tr4, &iClock,	&xt4, 0, 0,
/* tr5_1 */	&tr5, &iClock,	&trans, &tr4, &tr3, &tr2, &tr1, &tr0, 0,
		&tr5, 0,
/* tr5_2 */	&tr5, &iClock,	&xt5, 0, 0,
/* tr6_1 */	&tr6, &iClock,	&trans, &tr5, &tr4, &tr3, &tr2, &tr1,
		&tr0, 0, &tr6, 0,
/* tr6_2 */	&tr6, &iClock,	&xt6, 0, 0,
/* tr7_1 */	&tr7, &iClock,	&trans, &tr6, &tr5, &tr4, &tr3, &tr2,
		&tr1, &tr0, 0, &tr7, 0,
/* tr7_2 */	&tr7, &iClock,	&xt7, 0, 0,
/* trans_1 */	&trans, &iClock,	&IX0_7, &IX0_6, &IX0_5, &IX0_4,
		&IX0_3, &IX0_2, &IX0_1, &IX0_0, 0, 0,
/* trig */	&xt7, &xt6, &xt5, &xt4, &xt3, &xt2, &xt1, &xt0, 0, 0,
/* xt0 */	&tr0, &pt0, 0, 0,
/* xt1 */	&tr1, &pt1, 0, &pt0, 0,
/* xt2 */	&tr2, &pt2, 0, &pt1, &pt0, 0,
/* xt3 */	&tr3, &pt3, 0, &pt2, &pt1, &pt0, 0,
/* xt4 */	&tr4, &pt4, 0, &pt3, &pt2, &pt1, &pt0, 0,
/* xt5 */	&tr5, &pt5, 0, &pt4, &pt3, &pt2, &pt1, &pt0, 0,
/* xt6 */	&tr6, &pt6, 0, &pt5, &pt4, &pt3, &pt2, &pt1, &pt0, 0,
/* xt7 */	&tr7, &pt7, 0, &pt6, &pt5, &pt4, &pt3, &pt2, &pt1, &pt0, 0,
};

/********************************************************************
 *
 *	SOURCE:   ./Test0/demo.ic
 *	OUTPUT:   ./Test0/demo.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: demo.c,v 1.1 2015/05/24 00:51:51 jw Exp $ -O7";

#include	<icg.h>

static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

static iC_Gt _10     = { 1, -iC_NCONST, iC_ARITH, 0, "10", {0}, {0}, 0 };
static iC_Gt _11     = { 1, -iC_NCONST, iC_ARITH, 0, "11", {0}, {0}, &_10 };
static iC_Gt _2      = { 1, -iC_NCONST, iC_ARITH, 0, "2", {0}, {0}, &_11 };
static iC_Gt _20     = { 1, -iC_NCONST, iC_ARITH, 0, "20", {0}, {0}, &_2 };
static iC_Gt _3      = { 1, -iC_NCONST, iC_ARITH, 0, "3", {0}, {0}, &_20 };
static iC_Gt _30     = { 1, -iC_NCONST, iC_ARITH, 0, "30", {0}, {0}, &_3 };
static iC_Gt _4      = { 1, -iC_NCONST, iC_ARITH, 0, "4", {0}, {0}, &_30 };
static iC_Gt _5      = { 1, -iC_NCONST, iC_ARITH, 0, "5", {0}, {0}, &_4 };
static iC_Gt _6      = { 1, -iC_NCONST, iC_ARITH, 0, "6", {0}, {0}, &_5 };
static iC_Gt _8      = { 1, -iC_NCONST, iC_ARITH, 0, "8", {0}, {0}, &_6 };
static iC_Gt _15     = { 1, -iC_NCONST, iC_ARITH, 0, "15", {0}, {0}, &_8 };
static iC_Gt _27     = { 1, -iC_NCONST, iC_ARITH, 0, "27", {0}, {0}, &_15 };
iC_Gt I0       = { 1, -iC_OR, iC_GATE, 0, "I0", {0}, {&iC_l_[0]}, &_27 };
iC_Gt I0_1     = { 1, -iC_AND, iC_GATE, 0, "I0_1", {0}, {&iC_l_[6]}, &I0 };
iC_Gt I0_2     = { 1, -iC_AND, iC_GATE, 0, "I0_2", {0}, {&iC_l_[10]}, &I0_1 };
iC_Gt I0_3     = { 1, -iC_AND, iC_GATE, 0, "I0_3", {0}, {&iC_l_[14]}, &I0_2 };
iC_Gt I0_4     = { 1, -iC_AND, iC_GATE, 0, "I0_4", {0}, {&iC_l_[18]}, &I0_3 };
iC_Gt I1       = { 1, -iC_OR, iC_GATE, 0, "I1", {0}, {&iC_l_[22]}, &I0_4 };
iC_Gt I1_1     = { 1, -iC_AND, iC_GATE, 0, "I1_1", {0}, {&iC_l_[28]}, &I1 };
iC_Gt I1_2     = { 1, -iC_AND, iC_GATE, 0, "I1_2", {0}, {&iC_l_[32]}, &I1_1 };
iC_Gt I1_3     = { 1, -iC_AND, iC_GATE, 0, "I1_3", {0}, {&iC_l_[36]}, &I1_2 };
iC_Gt I1_4     = { 1, -iC_AND, iC_GATE, 0, "I1_4", {0}, {&iC_l_[40]}, &I1_3 };
iC_Gt I2       = { 1, -iC_OR, iC_GATE, 0, "I2", {0}, {&iC_l_[44]}, &I1_4 };
iC_Gt I2_1     = { 1, -iC_AND, iC_GATE, 0, "I2_1", {0}, {&iC_l_[50]}, &I2 };
iC_Gt I2_2     = { 1, -iC_AND, iC_GATE, 0, "I2_2", {0}, {&iC_l_[54]}, &I2_1 };
iC_Gt I2_3     = { 1, -iC_AND, iC_GATE, 0, "I2_3", {0}, {&iC_l_[58]}, &I2_2 };
iC_Gt I2_4     = { 1, -iC_AND, iC_GATE, 0, "I2_4", {0}, {&iC_l_[62]}, &I2_3 };
iC_Gt I3       = { 1, -iC_OR, iC_GATE, 0, "I3", {0}, {&iC_l_[66]}, &I2_4 };
iC_Gt I3_1     = { 1, -iC_AND, iC_GATE, 0, "I3_1", {0}, {&iC_l_[72]}, &I3 };
iC_Gt I3_2     = { 1, -iC_AND, iC_GATE, 0, "I3_2", {0}, {&iC_l_[76]}, &I3_1 };
iC_Gt I3_3     = { 1, -iC_AND, iC_GATE, 0, "I3_3", {0}, {&iC_l_[80]}, &I3_2 };
iC_Gt I3_4     = { 1, -iC_AND, iC_GATE, 0, "I3_4", {0}, {&iC_l_[84]}, &I3_3 };
iC_Gt I4       = { 1, -iC_OR, iC_GATE, 0, "I4", {0}, {&iC_l_[88]}, &I3_4 };
iC_Gt I4_1     = { 1, -iC_AND, iC_GATE, 0, "I4_1", {0}, {&iC_l_[94]}, &I4 };
iC_Gt I4_2     = { 1, -iC_AND, iC_GATE, 0, "I4_2", {0}, {&iC_l_[98]}, &I4_1 };
iC_Gt I4_3     = { 1, -iC_AND, iC_GATE, 0, "I4_3", {0}, {&iC_l_[102]}, &I4_2 };
iC_Gt I4_4     = { 1, -iC_AND, iC_GATE, 0, "I4_4", {0}, {&iC_l_[106]}, &I4_3 };
iC_Gt I5       = { 1, -iC_OR, iC_GATE, 0, "I5", {0}, {&iC_l_[110]}, &I4_4 };
iC_Gt I5_1     = { 1, -iC_AND, iC_GATE, 0, "I5_1", {0}, {&iC_l_[116]}, &I5 };
iC_Gt I5_2     = { 1, -iC_AND, iC_GATE, 0, "I5_2", {0}, {&iC_l_[120]}, &I5_1 };
iC_Gt I5_3     = { 1, -iC_AND, iC_GATE, 0, "I5_3", {0}, {&iC_l_[124]}, &I5_2 };
iC_Gt I5_4     = { 1, -iC_AND, iC_GATE, 0, "I5_4", {0}, {&iC_l_[128]}, &I5_3 };
iC_Gt I6       = { 1, -iC_OR, iC_GATE, 0, "I6", {0}, {&iC_l_[132]}, &I5_4 };
iC_Gt I6_1     = { 1, -iC_AND, iC_GATE, 0, "I6_1", {0}, {&iC_l_[138]}, &I6 };
iC_Gt I6_2     = { 1, -iC_AND, iC_GATE, 0, "I6_2", {0}, {&iC_l_[142]}, &I6_1 };
iC_Gt I6_3     = { 1, -iC_AND, iC_GATE, 0, "I6_3", {0}, {&iC_l_[146]}, &I6_2 };
iC_Gt I6_4     = { 1, -iC_AND, iC_GATE, 0, "I6_4", {0}, {&iC_l_[150]}, &I6_3 };
iC_Gt I7       = { 1, -iC_OR, iC_GATE, 0, "I7", {0}, {&iC_l_[154]}, &I6_4 };
iC_Gt I7_1     = { 1, -iC_AND, iC_GATE, 0, "I7_1", {0}, {&iC_l_[160]}, &I7 };
iC_Gt I7_2     = { 1, -iC_AND, iC_GATE, 0, "I7_2", {0}, {&iC_l_[164]}, &I7_1 };
iC_Gt I7_3     = { 1, -iC_AND, iC_GATE, 0, "I7_3", {0}, {&iC_l_[168]}, &I7_2 };
iC_Gt I7_4     = { 1, -iC_AND, iC_GATE, 0, "I7_4", {0}, {&iC_l_[172]}, &I7_3 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &I7_4 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
iC_Gt IX0_4    = { 1, -iC_INPX, iC_GATE, 0, "IX0.4", {0}, {0}, &IX0_3 };
iC_Gt IX0_5    = { 1, -iC_INPX, iC_GATE, 0, "IX0.5", {0}, {0}, &IX0_4 };
iC_Gt IX0_6    = { 1, -iC_INPX, iC_GATE, 0, "IX0.6", {0}, {0}, &IX0_5 };
iC_Gt IX0_7    = { 1, -iC_INPX, iC_GATE, 0, "IX0.7", {0}, {0}, &IX0_6 };
iC_Gt IX1_0    = { 1, -iC_INPX, iC_GATE, 0, "IX1.0", {0}, {0}, &IX0_7 };
iC_Gt IX1_1    = { 1, -iC_INPX, iC_GATE, 0, "IX1.1", {0}, {0}, &IX1_0 };
iC_Gt IX1_2    = { 1, -iC_INPX, iC_GATE, 0, "IX1.2", {0}, {0}, &IX1_1 };
iC_Gt IX1_3    = { 1, -iC_INPX, iC_GATE, 0, "IX1.3", {0}, {0}, &IX1_2 };
iC_Gt IX1_4    = { 1, -iC_INPX, iC_GATE, 0, "IX1.4", {0}, {0}, &IX1_3 };
iC_Gt IX1_5    = { 1, -iC_INPX, iC_GATE, 0, "IX1.5", {0}, {0}, &IX1_4 };
iC_Gt IX1_6    = { 1, -iC_INPX, iC_GATE, 0, "IX1.6", {0}, {0}, &IX1_5 };
iC_Gt IX1_7    = { 1, -iC_INPX, iC_GATE, 0, "IX1.7", {0}, {0}, &IX1_6 };
iC_Gt IX2_0    = { 1, -iC_INPX, iC_GATE, 0, "IX2.0", {0}, {0}, &IX1_7 };
iC_Gt IX2_1    = { 1, -iC_INPX, iC_GATE, 0, "IX2.1", {0}, {0}, &IX2_0 };
iC_Gt IX2_2    = { 1, -iC_INPX, iC_GATE, 0, "IX2.2", {0}, {0}, &IX2_1 };
iC_Gt IX2_3    = { 1, -iC_INPX, iC_GATE, 0, "IX2.3", {0}, {0}, &IX2_2 };
iC_Gt IX2_4    = { 1, -iC_INPX, iC_GATE, 0, "IX2.4", {0}, {0}, &IX2_3 };
iC_Gt IX2_5    = { 1, -iC_INPX, iC_GATE, 0, "IX2.5", {0}, {0}, &IX2_4 };
iC_Gt IX2_6    = { 1, -iC_INPX, iC_GATE, 0, "IX2.6", {0}, {0}, &IX2_5 };
iC_Gt IX2_7    = { 1, -iC_INPX, iC_GATE, 0, "IX2.7", {0}, {0}, &IX2_6 };
iC_Gt IX3_0    = { 1, -iC_INPX, iC_GATE, 0, "IX3.0", {0}, {0}, &IX2_7 };
iC_Gt IX3_1    = { 1, -iC_INPX, iC_GATE, 0, "IX3.1", {0}, {0}, &IX3_0 };
iC_Gt IX3_2    = { 1, -iC_INPX, iC_GATE, 0, "IX3.2", {0}, {0}, &IX3_1 };
iC_Gt IX3_3    = { 1, -iC_INPX, iC_GATE, 0, "IX3.3", {0}, {0}, &IX3_2 };
iC_Gt IX3_4    = { 1, -iC_INPX, iC_GATE, 0, "IX3.4", {0}, {0}, &IX3_3 };
iC_Gt IX3_5    = { 1, -iC_INPX, iC_GATE, 0, "IX3.5", {0}, {0}, &IX3_4 };
iC_Gt IX3_6    = { 1, -iC_INPX, iC_GATE, 0, "IX3.6", {0}, {0}, &IX3_5 };
iC_Gt IX3_7    = { 1, -iC_INPX, iC_GATE, 0, "IX3.7", {0}, {0}, &IX3_6 };
iC_Gt O0       = { 1, -iC_OR, iC_GATE, 0, "O0", {0}, {&iC_l_[176]}, &IX3_7 };
iC_Gt O0_1     = { 1, -iC_AND, iC_GATE, 0, "O0_1", {0}, {&iC_l_[181]}, &O0 };
iC_Gt O0_2     = { 1, -iC_AND, iC_GATE, 0, "O0_2", {0}, {&iC_l_[185]}, &O0_1 };
iC_Gt O0_3     = { 1, -iC_AND, iC_GATE, 0, "O0_3", {0}, {&iC_l_[189]}, &O0_2 };
iC_Gt O1       = { 1, -iC_OR, iC_GATE, 0, "O1", {0}, {&iC_l_[193]}, &O0_3 };
iC_Gt O10      = { 1, -iC_AND, iC_GATE, 0, "O10", {0}, {&iC_l_[198]}, &O1 };
iC_Gt O1_1     = { 1, -iC_AND, iC_GATE, 0, "O1_1", {0}, {&iC_l_[202]}, &O10 };
iC_Gt O1_2     = { 1, -iC_AND, iC_GATE, 0, "O1_2", {0}, {&iC_l_[206]}, &O1_1 };
iC_Gt O1_3     = { 1, -iC_AND, iC_GATE, 0, "O1_3", {0}, {&iC_l_[210]}, &O1_2 };
iC_Gt O2       = { 1, -iC_OR, iC_GATE, 0, "O2", {0}, {&iC_l_[214]}, &O1_3 };
iC_Gt O2_1     = { 1, -iC_AND, iC_GATE, 0, "O2_1", {0}, {&iC_l_[219]}, &O2 };
iC_Gt O2_2     = { 1, -iC_AND, iC_GATE, 0, "O2_2", {0}, {&iC_l_[223]}, &O2_1 };
iC_Gt O2_3     = { 1, -iC_AND, iC_GATE, 0, "O2_3", {0}, {&iC_l_[227]}, &O2_2 };
iC_Gt O3       = { 1, -iC_OR, iC_GATE, 0, "O3", {0}, {&iC_l_[231]}, &O2_3 };
iC_Gt O3_1     = { 1, -iC_AND, iC_GATE, 0, "O3_1", {0}, {&iC_l_[236]}, &O3 };
iC_Gt O3_2     = { 1, -iC_AND, iC_GATE, 0, "O3_2", {0}, {&iC_l_[240]}, &O3_1 };
iC_Gt O3_3     = { 1, -iC_AND, iC_GATE, 0, "O3_3", {0}, {&iC_l_[244]}, &O3_2 };
iC_Gt O4       = { 1, -iC_OR, iC_GATE, 0, "O4", {0}, {&iC_l_[248]}, &O3_3 };
iC_Gt O4_1     = { 1, -iC_AND, iC_GATE, 0, "O4_1", {0}, {&iC_l_[253]}, &O4 };
iC_Gt O4_2     = { 1, -iC_AND, iC_GATE, 0, "O4_2", {0}, {&iC_l_[257]}, &O4_1 };
iC_Gt O4_3     = { 1, -iC_AND, iC_GATE, 0, "O4_3", {0}, {&iC_l_[261]}, &O4_2 };
iC_Gt O5       = { 1, -iC_OR, iC_GATE, 0, "O5", {0}, {&iC_l_[265]}, &O4_3 };
iC_Gt O5_1     = { 1, -iC_AND, iC_GATE, 0, "O5_1", {0}, {&iC_l_[270]}, &O5 };
iC_Gt O5_2     = { 1, -iC_AND, iC_GATE, 0, "O5_2", {0}, {&iC_l_[274]}, &O5_1 };
iC_Gt O5_3     = { 1, -iC_AND, iC_GATE, 0, "O5_3", {0}, {&iC_l_[278]}, &O5_2 };
iC_Gt O6       = { 1, -iC_OR, iC_GATE, 0, "O6", {0}, {&iC_l_[282]}, &O5_3 };
iC_Gt O6_1     = { 1, -iC_AND, iC_GATE, 0, "O6_1", {0}, {&iC_l_[287]}, &O6 };
iC_Gt O6_2     = { 1, -iC_AND, iC_GATE, 0, "O6_2", {0}, {&iC_l_[291]}, &O6_1 };
iC_Gt O6_3     = { 1, -iC_AND, iC_GATE, 0, "O6_3", {0}, {&iC_l_[295]}, &O6_2 };
iC_Gt O7       = { 1, -iC_OR, iC_GATE, 0, "O7", {0}, {&iC_l_[299]}, &O6_3 };
iC_Gt O7_1     = { 1, -iC_AND, iC_GATE, 0, "O7_1", {0}, {&iC_l_[304]}, &O7 };
iC_Gt O7_2     = { 1, -iC_AND, iC_GATE, 0, "O7_2", {0}, {&iC_l_[308]}, &O7_1 };
iC_Gt O7_3     = { 1, -iC_AND, iC_GATE, 0, "O7_3", {0}, {&iC_l_[312]}, &O7_2 };
iC_Gt O8       = { 1, -iC_FF, iC_GATE, 0, "O8", {0}, {0}, &O7_3 };
iC_Gt O8_1     = { 1, -iC_OR, iC_D_FF, 0, "O8_1", {&iC_l_[316]}, {&iC_l_[318]}, &O8 };
iC_Gt O9       = { 1, -iC_FF, iC_GATE, 0, "O9", {0}, {0}, &O8_1 };
iC_Gt O9_1     = { 1, -iC_AND, iC_D_FF, 0, "O9_1", {&iC_l_[322]}, {&iC_l_[324]}, &O9 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[328]}, &O9_1 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[331]}, &QX0_0_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[334]}, &QX0_1_0 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[337]}, &QX0_2_0 };
iC_Gt QX0_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.4_0", {0}, {&iC_l_[340]}, &QX0_3_0 };
iC_Gt QX0_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.5_0", {0}, {&iC_l_[343]}, &QX0_4_0 };
iC_Gt QX0_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.6_0", {0}, {&iC_l_[346]}, &QX0_5_0 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[349]}, &QX0_6_0 };
iC_Gt QX1_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.0_0", {0}, {&iC_l_[352]}, &QX0_7_0 };
iC_Gt QX1_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.1_0", {0}, {&iC_l_[355]}, &QX1_0_0 };
iC_Gt QX1_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.2_0", {0}, {&iC_l_[358]}, &QX1_1_0 };
iC_Gt QX1_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.3_0", {0}, {&iC_l_[361]}, &QX1_2_0 };
iC_Gt QX1_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.4_0", {0}, {&iC_l_[364]}, &QX1_3_0 };
iC_Gt QX1_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.5_0", {0}, {&iC_l_[367]}, &QX1_4_0 };
iC_Gt QX1_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.6_0", {0}, {&iC_l_[370]}, &QX1_5_0 };
iC_Gt QX1_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.7_0", {0}, {&iC_l_[373]}, &QX1_6_0 };
iC_Gt QX2_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.0_0", {0}, {&iC_l_[376]}, &QX1_7_0 };
iC_Gt QX2_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.1_0", {0}, {&iC_l_[379]}, &QX2_0_0 };
iC_Gt QX2_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.2_0", {0}, {&iC_l_[382]}, &QX2_1_0 };
iC_Gt QX2_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.3_0", {0}, {&iC_l_[385]}, &QX2_2_0 };
iC_Gt QX2_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.4_0", {0}, {&iC_l_[388]}, &QX2_3_0 };
iC_Gt QX2_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.5_0", {0}, {&iC_l_[391]}, &QX2_4_0 };
iC_Gt QX2_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.6_0", {0}, {&iC_l_[394]}, &QX2_5_0 };
iC_Gt QX2_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.7_0", {0}, {&iC_l_[397]}, &QX2_6_0 };
iC_Gt QX3_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.0_0", {0}, {&iC_l_[400]}, &QX2_7_0 };
iC_Gt QX3_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.1_0", {0}, {&iC_l_[403]}, &QX3_0_0 };
iC_Gt QX3_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.2_0", {0}, {&iC_l_[406]}, &QX3_1_0 };
iC_Gt QX3_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.3_0", {0}, {&iC_l_[409]}, &QX3_2_0 };
iC_Gt QX3_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.4_0", {0}, {&iC_l_[412]}, &QX3_3_0 };
iC_Gt QX3_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.5_0", {0}, {&iC_l_[415]}, &QX3_4_0 };
iC_Gt QX3_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.6_0", {0}, {&iC_l_[418]}, &QX3_5_0 };
iC_Gt QX3_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX3.7_0", {0}, {&iC_l_[421]}, &QX3_6_0 };
iC_Gt TX0_4    = { 1, -iC_INPX, iC_GATE, 0, "TX0.4", {0}, {0}, &QX3_7_0 };
iC_Gt TX0_5    = { 1, -iC_INPX, iC_GATE, 0, "TX0.5", {0}, {0}, &TX0_4 };
iC_Gt bank0    = { 1, -iC_AND, iC_GATE, 0, "bank0", {0}, {&iC_l_[424]}, &TX0_5 };
iC_Gt bank1    = { 1, -iC_AND, iC_GATE, 0, "bank1", {0}, {&iC_l_[428]}, &bank0 };
iC_Gt bank2    = { 1, -iC_AND, iC_GATE, 0, "bank2", {0}, {&iC_l_[432]}, &bank1 };
iC_Gt bank3    = { 1, -iC_AND, iC_GATE, 0, "bank3", {0}, {&iC_l_[436]}, &bank2 };
iC_Gt c        = { 1, -iC_CLK, iC_CLCKL, 0, "c", {0}, {0}, &bank3 };
iC_Gt c1       = { 1, -iC_CLK, iC_CLCKL, 0, "c1", {0}, {0}, &c };
iC_Gt c1_1     = { 1, -iC_OR, iC_CLCK, 0, "c1_1", {&iC_l_[440]}, {&iC_l_[442]}, &c1 };
iC_Gt c_1      = { 1, -iC_OR, iC_CLCK, 0, "c_1", {&iC_l_[445]}, {&iC_l_[447]}, &c1_1 };
iC_Gt c_2      = { 1, -iC_AND, iC_GATE, 0, "c_2", {0}, {&iC_l_[453]}, &c_1 };
iC_Gt c_3      = { 1, -iC_AND, iC_GATE, 0, "c_3", {0}, {&iC_l_[457]}, &c_2 };
iC_Gt m0       = { 1, -iC_FF, iC_GATE, 0, "m0", {0}, {0}, &c_3 };
iC_Gt m0_1     = { 1, -iC_OR, iC_D_FF, 0, "m0_1", {&iC_l_[461]}, {&iC_l_[463]}, &m0 };
iC_Gt m1       = { 1, -iC_FF, iC_GATE, 0, "m1", {0}, {0}, &m0_1 };
iC_Gt m1_1     = { 1, -iC_OR, iC_D_FF, 0, "m1_1", {&iC_l_[466]}, {&iC_l_[468]}, &m1 };
iC_Gt m2       = { 1, -iC_FF, iC_GATE, 0, "m2", {0}, {0}, &m1_1 };
iC_Gt m2_1     = { 1, -iC_OR, iC_D_FF, 0, "m2_1", {&iC_l_[471]}, {&iC_l_[473]}, &m2 };
iC_Gt m3       = { 1, -iC_FF, iC_GATE, 0, "m3", {0}, {0}, &m2_1 };
iC_Gt m3_1     = { 1, -iC_OR, iC_D_FF, 0, "m3_1", {&iC_l_[476]}, {&iC_l_[478]}, &m3 };
iC_Gt m4       = { 1, -iC_FF, iC_GATE, 0, "m4", {0}, {0}, &m3_1 };
iC_Gt m4_1     = { 1, -iC_OR, iC_D_FF, 0, "m4_1", {&iC_l_[481]}, {&iC_l_[483]}, &m4 };
iC_Gt m5       = { 1, -iC_FF, iC_GATE, 0, "m5", {0}, {0}, &m4_1 };
iC_Gt m5_1     = { 1, -iC_OR, iC_D_FF, 0, "m5_1", {&iC_l_[486]}, {&iC_l_[488]}, &m5 };
iC_Gt m6       = { 1, -iC_FF, iC_GATE, 0, "m6", {0}, {0}, &m5_1 };
iC_Gt m6_1     = { 1, -iC_OR, iC_D_FF, 0, "m6_1", {&iC_l_[491]}, {&iC_l_[493]}, &m6 };
iC_Gt m7       = { 1, -iC_FF, iC_GATE, 0, "m7", {0}, {0}, &m6_1 };
iC_Gt m7_1     = { 1, -iC_OR, iC_D_FF, 0, "m7_1", {&iC_l_[496]}, {&iC_l_[498]}, &m7 };
iC_Gt o2       = { 1, -iC_FF, iC_GATE, 0, "o2", {0}, {0}, &m7_1 };
iC_Gt o2_1     = { 1, -iC_AND, iC_S_FF, 0, "o2_1", {&iC_l_[501]}, {&iC_l_[503]}, &o2 };
iC_Gt o2_2     = { 1, -iC_OR, iC_R_FF, 0, "o2_2", {&iC_l_[507]}, {&iC_l_[510]}, &o2_1 };
iC_Gt o5       = { 1, -iC_FF, iC_GATE, 0, "o5", {0}, {0}, &o2_2 };
iC_Gt o5_1     = { 1, -iC_AND, iC_S_FF, 0, "o5_1", {&iC_l_[513]}, {&iC_l_[515]}, &o5 };
iC_Gt o5_2     = { 1, -iC_OR, iC_R_FF, 0, "o5_2", {&iC_l_[519]}, {&iC_l_[522]}, &o5_1 };
iC_Gt t        = { 1, -iC_TIM, iC_TIMRL, 0, "t", {0}, {0}, &o5_2 };
iC_Gt t0       = { 1, -iC_FF, iC_GATE, 0, "t0", {0}, {0}, &t };
iC_Gt t0_1     = { 1, -iC_OR, iC_D_FF, 0, "t0_1", {&iC_l_[525]}, {&iC_l_[528]}, &t0 };
iC_Gt t1       = { 1, -iC_FF, iC_GATE, 0, "t1", {0}, {0}, &t0_1 };
iC_Gt t1_1     = { 1, -iC_OR, iC_D_FF, 0, "t1_1", {&iC_l_[531]}, {&iC_l_[534]}, &t1 };
iC_Gt t2       = { 1, -iC_FF, iC_GATE, 0, "t2", {0}, {0}, &t1_1 };
iC_Gt t2_1     = { 1, -iC_OR, iC_D_FF, 0, "t2_1", {&iC_l_[537]}, {&iC_l_[540]}, &t2 };
iC_Gt t3       = { 1, -iC_FF, iC_GATE, 0, "t3", {0}, {0}, &t2_1 };
iC_Gt t3_1     = { 1, -iC_OR, iC_D_FF, 0, "t3_1", {&iC_l_[543]}, {&iC_l_[546]}, &t3 };
iC_Gt t4       = { 1, -iC_FF, iC_GATE, 0, "t4", {0}, {0}, &t3_1 };
iC_Gt t4_1     = { 1, -iC_OR, iC_D_FF, 0, "t4_1", {&iC_l_[549]}, {&iC_l_[552]}, &t4 };
iC_Gt t5       = { 1, -iC_FF, iC_GATE, 0, "t5", {0}, {0}, &t4_1 };
iC_Gt t5_1     = { 1, -iC_OR, iC_D_FF, 0, "t5_1", {&iC_l_[555]}, {&iC_l_[558]}, &t5 };
iC_Gt t6       = { 1, -iC_FF, iC_GATE, 0, "t6", {0}, {0}, &t5_1 };
iC_Gt t6_1     = { 1, -iC_OR, iC_D_FF, 0, "t6_1", {&iC_l_[561]}, {&iC_l_[564]}, &t6 };
iC_Gt t7       = { 1, -iC_FF, iC_GATE, 0, "t7", {0}, {0}, &t6_1 };
iC_Gt t7_1     = { 1, -iC_OR, iC_D_FF, 0, "t7_1", {&iC_l_[567]}, {&iC_l_[570]}, &t7 };
iC_Gt t8       = { 1, -iC_FF, iC_GATE, 0, "t8", {0}, {0}, &t7_1 };
iC_Gt t8_1     = { 1, -iC_OR, iC_D_FF, 0, "t8_1", {&iC_l_[573]}, {&iC_l_[576]}, &t8 };
iC_Gt t_1      = { 1, -iC_OR, iC_TIMR, 0, "t_1", {&iC_l_[579]}, {&iC_l_[581]}, &t8_1 };
iC_Gt v0       = { 1, -iC_AND, iC_GATE, 0, "v0", {0}, {&iC_l_[584]}, &t_1 };
iC_Gt v1       = { 1, -iC_OR, iC_GATE, 0, "v1", {0}, {&iC_l_[588]}, &v0 };
iC_Gt v2       = { 1, -iC_FF, iC_GATE, 0, "v2", {0}, {0}, &v1 };
iC_Gt v2_1     = { 1, -iC_OR, iC_S_FF, 0, "v2_1", {&iC_l_[592]}, {&iC_l_[594]}, &v2 };
iC_Gt v2_2     = { 1, -iC_OR, iC_R_FF, 0, "v2_2", {&iC_l_[597]}, {&iC_l_[599]}, &v2_1 };
iC_Gt v3       = { 1, -iC_FF, iC_GATE, 0, "v3", {0}, {0}, &v2_2 };
iC_Gt v3_1     = { 1, -iC_OR, iC_S_FF, 0, "v3_1", {&iC_l_[602]}, {&iC_l_[604]}, &v3 };
iC_Gt v3_2     = { 1, -iC_OR, iC_R_FF, 0, "v3_2", {&iC_l_[608]}, {&iC_l_[610]}, &v3_1 };
iC_Gt v3_3     = { 1, -iC_OR, iC_R_FF, 0, "v3_3", {&iC_l_[613]}, {&iC_l_[616]}, &v3_2 };
iC_Gt v4       = { 1, -iC_FF, iC_GATE, 0, "v4", {0}, {0}, &v3_3 };
iC_Gt v4_1     = { 1, -iC_OR, iC_D_FF, 0, "v4_1", {&iC_l_[619]}, {&iC_l_[622]}, &v4 };
iC_Gt v5_1     = { 1, -iC_FF, iC_GATE, 0, "v5_1", {0}, {0}, &v4_1 };
iC_Gt v5_2     = { 1, -iC_OR, iC_D_FF, 0, "v5_2", {&iC_l_[625]}, {&iC_l_[628]}, &v5_1 };
iC_Gt v6       = { 1, -iC_LATCH, iC_GATE, 0, "v6", {0}, {&iC_l_[631]}, &v5_2 };
iC_Gt v7       = { 1, -iC_FF, iC_GATE, 0, "v7", {0}, {0}, &v6 };
iC_Gt v7_1     = { 1, -iC_LATCH, iC_D_FF, 0, "v7_1", {&iC_l_[636]}, {&iC_l_[638]}, &v7 };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&v0}, &v7_1, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&v1}, &QX0_0, 0 };
iC_Gt QX0_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.2", {0}, {(iC_Gt**)&v2}, &QX0_1, 0 };
iC_Gt QX0_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.3", {0}, {(iC_Gt**)&v3}, &QX0_2, 0 };
iC_Gt QX0_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.4", {0}, {(iC_Gt**)&v4}, &QX0_3, 0 };
iC_Gt QX0_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.5", {0}, {(iC_Gt**)&v5_1}, &QX0_4, 1 };
iC_Gt QX0_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.6", {0}, {(iC_Gt**)&v6}, &QX0_5, 0 };
iC_Gt QX0_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.7", {0}, {(iC_Gt**)&v7}, &QX0_6, 0 };
iC_Gt QX1_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.0", {0}, {(iC_Gt**)&m0}, &QX0_7, 0 };
iC_Gt QX1_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.1", {0}, {(iC_Gt**)&m1}, &QX1_0, 0 };
iC_Gt QX1_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.2", {0}, {(iC_Gt**)&m2}, &QX1_1, 0 };
iC_Gt QX1_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.3", {0}, {(iC_Gt**)&m3}, &QX1_2, 0 };
iC_Gt QX1_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.4", {0}, {(iC_Gt**)&m4}, &QX1_3, 0 };
iC_Gt QX1_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.5", {0}, {(iC_Gt**)&m5}, &QX1_4, 0 };
iC_Gt QX1_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.6", {0}, {(iC_Gt**)&m6}, &QX1_5, 0 };
iC_Gt QX1_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.7", {0}, {(iC_Gt**)&m7}, &QX1_6, 0 };
iC_Gt QX2_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.0", {0}, {(iC_Gt**)&t0}, &QX1_7, 0 };
iC_Gt QX2_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.1", {0}, {(iC_Gt**)&t1}, &QX2_0, 0 };
iC_Gt QX2_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.2", {0}, {(iC_Gt**)&t2}, &QX2_1, 0 };
iC_Gt QX2_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.3", {0}, {(iC_Gt**)&t3}, &QX2_2, 0 };
iC_Gt QX2_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.4", {0}, {(iC_Gt**)&t4}, &QX2_3, 0 };
iC_Gt QX2_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.5", {0}, {(iC_Gt**)&t5}, &QX2_4, 0 };
iC_Gt QX2_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.6", {0}, {(iC_Gt**)&t6}, &QX2_5, 0 };
iC_Gt QX2_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.7", {0}, {(iC_Gt**)&t7}, &QX2_6, 0 };
iC_Gt QX3_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.0", {0}, {(iC_Gt**)&O0}, &QX2_7, 0 };
iC_Gt QX3_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.1", {0}, {(iC_Gt**)&O1}, &QX3_0, 0 };
iC_Gt QX3_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.2", {0}, {(iC_Gt**)&O2}, &QX3_1, 0 };
iC_Gt QX3_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.3", {0}, {(iC_Gt**)&O3}, &QX3_2, 0 };
iC_Gt QX3_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.4", {0}, {(iC_Gt**)&O4}, &QX3_3, 0 };
iC_Gt QX3_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.5", {0}, {(iC_Gt**)&O5}, &QX3_4, 0 };
iC_Gt QX3_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.6", {0}, {(iC_Gt**)&O6}, &QX3_5, 0 };
iC_Gt QX3_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX3.7", {0}, {(iC_Gt**)&O7}, &QX3_6, 0 };
iC_Gt T100M    = { 1, -iC_ALIAS, iC_GATE, 0, "T100M", {0}, {(iC_Gt**)&TX0_4}, &QX3_7, 0 };
iC_Gt T1S      = { 1, -iC_ALIAS, iC_GATE, 0, "T1S", {0}, {(iC_Gt**)&TX0_5}, &T100M, 0 };
iC_Gt i00      = { 1, -iC_ALIAS, iC_GATE, 0, "i00", {0}, {(iC_Gt**)&IX0_0}, &T1S, 0 };
iC_Gt i01      = { 1, -iC_ALIAS, iC_GATE, 0, "i01", {0}, {(iC_Gt**)&IX0_1}, &i00, 0 };
iC_Gt i02      = { 1, -iC_ALIAS, iC_GATE, 0, "i02", {0}, {(iC_Gt**)&IX0_2}, &i01, 0 };
iC_Gt i03      = { 1, -iC_ALIAS, iC_GATE, 0, "i03", {0}, {(iC_Gt**)&IX0_3}, &i02, 0 };
iC_Gt i04      = { 1, -iC_ALIAS, iC_GATE, 0, "i04", {0}, {(iC_Gt**)&IX0_4}, &i03, 0 };
iC_Gt i05      = { 1, -iC_ALIAS, iC_GATE, 0, "i05", {0}, {(iC_Gt**)&IX0_5}, &i04, 0 };
iC_Gt i06      = { 1, -iC_ALIAS, iC_GATE, 0, "i06", {0}, {(iC_Gt**)&IX0_6}, &i05, 0 };
iC_Gt i07      = { 1, -iC_ALIAS, iC_GATE, 0, "i07", {0}, {(iC_Gt**)&IX0_7}, &i06, 0 };
iC_Gt i10      = { 1, -iC_ALIAS, iC_GATE, 0, "i10", {0}, {(iC_Gt**)&IX1_0}, &i07, 0 };
iC_Gt i11      = { 1, -iC_ALIAS, iC_GATE, 0, "i11", {0}, {(iC_Gt**)&IX1_1}, &i10, 0 };
iC_Gt i12      = { 1, -iC_ALIAS, iC_GATE, 0, "i12", {0}, {(iC_Gt**)&IX1_2}, &i11, 0 };
iC_Gt i13      = { 1, -iC_ALIAS, iC_GATE, 0, "i13", {0}, {(iC_Gt**)&IX1_3}, &i12, 0 };
iC_Gt i14      = { 1, -iC_ALIAS, iC_GATE, 0, "i14", {0}, {(iC_Gt**)&IX1_4}, &i13, 0 };
iC_Gt i15      = { 1, -iC_ALIAS, iC_GATE, 0, "i15", {0}, {(iC_Gt**)&IX1_5}, &i14, 0 };
iC_Gt i16      = { 1, -iC_ALIAS, iC_GATE, 0, "i16", {0}, {(iC_Gt**)&IX1_6}, &i15, 0 };
iC_Gt i17      = { 1, -iC_ALIAS, iC_GATE, 0, "i17", {0}, {(iC_Gt**)&IX1_7}, &i16, 0 };
iC_Gt i20      = { 1, -iC_ALIAS, iC_GATE, 0, "i20", {0}, {(iC_Gt**)&IX2_0}, &i17, 0 };
iC_Gt i21      = { 1, -iC_ALIAS, iC_GATE, 0, "i21", {0}, {(iC_Gt**)&IX2_1}, &i20, 0 };
iC_Gt i22      = { 1, -iC_ALIAS, iC_GATE, 0, "i22", {0}, {(iC_Gt**)&IX2_2}, &i21, 0 };
iC_Gt i23      = { 1, -iC_ALIAS, iC_GATE, 0, "i23", {0}, {(iC_Gt**)&IX2_3}, &i22, 0 };
iC_Gt i24      = { 1, -iC_ALIAS, iC_GATE, 0, "i24", {0}, {(iC_Gt**)&IX2_4}, &i23, 0 };
iC_Gt i25      = { 1, -iC_ALIAS, iC_GATE, 0, "i25", {0}, {(iC_Gt**)&IX2_5}, &i24, 0 };
iC_Gt i26      = { 1, -iC_ALIAS, iC_GATE, 0, "i26", {0}, {(iC_Gt**)&IX2_6}, &i25, 0 };
iC_Gt i27      = { 1, -iC_ALIAS, iC_GATE, 0, "i27", {0}, {(iC_Gt**)&IX2_7}, &i26, 0 };
iC_Gt i30      = { 1, -iC_ALIAS, iC_GATE, 0, "i30", {0}, {(iC_Gt**)&IX3_0}, &i27, 0 };
iC_Gt i31      = { 1, -iC_ALIAS, iC_GATE, 0, "i31", {0}, {(iC_Gt**)&IX3_1}, &i30, 0 };
iC_Gt i32      = { 1, -iC_ALIAS, iC_GATE, 0, "i32", {0}, {(iC_Gt**)&IX3_2}, &i31, 0 };
iC_Gt i33      = { 1, -iC_ALIAS, iC_GATE, 0, "i33", {0}, {(iC_Gt**)&IX3_3}, &i32, 0 };
iC_Gt i34      = { 1, -iC_ALIAS, iC_GATE, 0, "i34", {0}, {(iC_Gt**)&IX3_4}, &i33, 0 };
iC_Gt i35      = { 1, -iC_ALIAS, iC_GATE, 0, "i35", {0}, {(iC_Gt**)&IX3_5}, &i34, 0 };
iC_Gt i36      = { 1, -iC_ALIAS, iC_GATE, 0, "i36", {0}, {(iC_Gt**)&IX3_6}, &i35, 0 };
iC_Gt i37      = { 1, -iC_ALIAS, iC_GATE, 0, "i37", {0}, {(iC_Gt**)&IX3_7}, &i36, 0 };
iC_Gt v5       = { 1, -iC_ALIAS, iC_GATE, 0, "v5", {0}, {(iC_Gt**)&v5_1}, &i37, 1 };

iC_Gt *		iC___Test0_demo_list = &v5;
iC_Gt **	iC_list[] = { &iC___Test0_demo_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* I0 */	&I0_4, &I0_3, &I0_2, &I0_1, 0, 0,
/* I0_1 */	&bank0, &IX0_0, 0, 0,
/* I0_2 */	&bank1, &IX1_0, 0, 0,
/* I0_3 */	&bank2, &IX2_0, 0, 0,
/* I0_4 */	&bank3, &IX3_0, 0, 0,
/* I1 */	&I1_4, &I1_3, &I1_2, &I1_1, 0, 0,
/* I1_1 */	&bank0, &IX0_1, 0, 0,
/* I1_2 */	&bank1, &IX1_1, 0, 0,
/* I1_3 */	&bank2, &IX2_1, 0, 0,
/* I1_4 */	&bank3, &IX3_1, 0, 0,
/* I2 */	&I2_4, &I2_3, &I2_2, &I2_1, 0, 0,
/* I2_1 */	&bank0, &IX0_2, 0, 0,
/* I2_2 */	&bank1, &IX1_2, 0, 0,
/* I2_3 */	&bank2, &IX2_2, 0, 0,
/* I2_4 */	&bank3, &IX3_2, 0, 0,
/* I3 */	&I3_4, &I3_3, &I3_2, &I3_1, 0, 0,
/* I3_1 */	&bank0, &IX0_3, 0, 0,
/* I3_2 */	&bank1, &IX1_3, 0, 0,
/* I3_3 */	&bank2, &IX2_3, 0, 0,
/* I3_4 */	&bank3, &IX3_3, 0, 0,
/* I4 */	&I4_4, &I4_3, &I4_2, &I4_1, 0, 0,
/* I4_1 */	&bank0, &IX0_4, 0, 0,
/* I4_2 */	&bank1, &IX1_4, 0, 0,
/* I4_3 */	&bank2, &IX2_4, 0, 0,
/* I4_4 */	&bank3, &IX3_4, 0, 0,
/* I5 */	&I5_4, &I5_3, &I5_2, &I5_1, 0, 0,
/* I5_1 */	&bank0, &IX0_5, 0, 0,
/* I5_2 */	&bank1, &IX1_5, 0, 0,
/* I5_3 */	&bank2, &IX2_5, 0, 0,
/* I5_4 */	&bank3, &IX3_5, 0, 0,
/* I6 */	&I6_4, &I6_3, &I6_2, &I6_1, 0, 0,
/* I6_1 */	&bank0, &IX0_6, 0, 0,
/* I6_2 */	&bank1, &IX1_6, 0, 0,
/* I6_3 */	&bank2, &IX2_6, 0, 0,
/* I6_4 */	&bank3, &IX0_6, 0, 0,
/* I7 */	&I7_4, &I7_3, &I7_2, &I7_1, 0, 0,
/* I7_1 */	&bank0, &IX0_7, 0, 0,
/* I7_2 */	&bank1, &IX1_7, 0, 0,
/* I7_3 */	&bank2, &IX2_7, 0, 0,
/* I7_4 */	&bank3, &IX0_7, 0, 0,
/* O0 */	&O0_3, &O0_2, &O0_1, 0, 0,
/* O0_1 */	&v0, &O8, 0, 0,
/* O0_2 */	&m0, &O9, 0, 0,
/* O0_3 */	&t0, &O10, 0, 0,
/* O1 */	&O1_3, &O1_2, &O1_1, 0, 0,
/* O10 */	0, &O9, &O8, 0,
/* O1_1 */	&v1, &O8, 0, 0,
/* O1_2 */	&m1, &O9, 0, 0,
/* O1_3 */	&t1, &O10, 0, 0,
/* O2 */	&O2_3, &O2_2, &O2_1, 0, 0,
/* O2_1 */	&v2, &O8, 0, 0,
/* O2_2 */	&m2, &O9, 0, 0,
/* O2_3 */	&t2, &O10, 0, 0,
/* O3 */	&O3_3, &O3_2, &O3_1, 0, 0,
/* O3_1 */	&v3, &O8, 0, 0,
/* O3_2 */	&m3, &O9, 0, 0,
/* O3_3 */	&t3, &O10, 0, 0,
/* O4 */	&O4_3, &O4_2, &O4_1, 0, 0,
/* O4_1 */	&v4, &O8, 0, 0,
/* O4_2 */	&m4, &O9, 0, 0,
/* O4_3 */	&t4, &O10, 0, 0,
/* O5 */	&O5_3, &O5_2, &O5_1, 0, 0,
/* O5_1 */	&O8, 0, &v5_1, 0,
/* O5_2 */	&m5, &O9, 0, 0,
/* O5_3 */	&t5, &O10, 0, 0,
/* O6 */	&O6_3, &O6_2, &O6_1, 0, 0,
/* O6_1 */	&v6, &O8, 0, 0,
/* O6_2 */	&m6, &O9, 0, 0,
/* O6_3 */	&t6, &O10, 0, 0,
/* O7 */	&O7_3, &O7_2, &O7_1, 0, 0,
/* O7_1 */	&v7, &O8, 0, 0,
/* O7_2 */	&m7, &O9, 0, 0,
/* O7_3 */	&t7, &O10, 0, 0,
/* O8_1 */	&O8, &c1,	&bank3, &O10, 0, 0,
/* O9_1 */	&O9, &c1,	&O8, 0, &bank3, 0,
/* QX0.0_0 */	&v0, 0, 0,
/* QX0.1_0 */	&v1, 0, 0,
/* QX0.2_0 */	&v2, 0, 0,
/* QX0.3_0 */	&v3, 0, 0,
/* QX0.4_0 */	&v4, 0, 0,
/* QX0.5_0 */	0, &v5_1, 0,
/* QX0.6_0 */	&v6, 0, 0,
/* QX0.7_0 */	&v7, 0, 0,
/* QX1.0_0 */	&m0, 0, 0,
/* QX1.1_0 */	&m1, 0, 0,
/* QX1.2_0 */	&m2, 0, 0,
/* QX1.3_0 */	&m3, 0, 0,
/* QX1.4_0 */	&m4, 0, 0,
/* QX1.5_0 */	&m5, 0, 0,
/* QX1.6_0 */	&m6, 0, 0,
/* QX1.7_0 */	&m7, 0, 0,
/* QX2.0_0 */	&t0, 0, 0,
/* QX2.1_0 */	&t1, 0, 0,
/* QX2.2_0 */	&t2, 0, 0,
/* QX2.3_0 */	&t3, 0, 0,
/* QX2.4_0 */	&t4, 0, 0,
/* QX2.5_0 */	&t5, 0, 0,
/* QX2.6_0 */	&t6, 0, 0,
/* QX2.7_0 */	&t7, 0, 0,
/* QX3.0_0 */	&O0, 0, 0,
/* QX3.1_0 */	&O1, 0, 0,
/* QX3.2_0 */	&O2, 0, 0,
/* QX3.3_0 */	&O3, 0, 0,
/* QX3.4_0 */	&O4, 0, 0,
/* QX3.5_0 */	&O5, 0, 0,
/* QX3.6_0 */	&O6, 0, 0,
/* QX3.7_0 */	&O7, 0, 0,
/* bank0 */	0, &IX3_7, &IX3_6, 0,
/* bank1 */	&IX3_7, 0, &IX3_6, 0,
/* bank2 */	&IX3_6, 0, &IX3_7, 0,
/* bank3 */	&IX3_7, &IX3_6, 0, 0,
/* c1_1 */	&c1, &iClock,	0, &t8, 0,
/* c_1 */	&c, &iClock,	&o5, &o2, &c_3, &c_2, 0, 0,
/* c_2 */	&bank0, &TX0_4, 0, 0,
/* c_3 */	&bank3, &TX0_5, 0, 0,
/* m0_1 */	&m0, &c,	0, &m7, 0,
/* m1_1 */	&m1, &c,	&m0, 0, 0,
/* m2_1 */	&m2, &c,	&m1, 0, 0,
/* m3_1 */	&m3, &c,	&m2, 0, 0,
/* m4_1 */	&m4, &c,	&m3, 0, 0,
/* m5_1 */	&m5, &c,	&m4, 0, 0,
/* m6_1 */	&m6, &c,	&m5, 0, 0,
/* m7_1 */	&m7, &c,	&m6, 0, 0,
/* o2_1 */	&o2, &iClock,	&bank1, 0, &o2, 0,
/* o2_2 */	&o2, &t, &_2,	&o2, 0, 0,
/* o5_1 */	&o5, &iClock,	&bank2, 0, &o5, 0,
/* o5_2 */	&o5, &t, &_5,	&o5, 0, 0,
/* t0_1 */	&t0, &t, &_2,	0, &t8, 0,
/* t1_1 */	&t1, &t, &_3,	&t0, 0, 0,
/* t2_1 */	&t2, &t, &_4,	&t1, 0, 0,
/* t3_1 */	&t3, &t, &_6,	&t2, 0, 0,
/* t4_1 */	&t4, &t, &_8,	&t3, 0, 0,
/* t5_1 */	&t5, &t, &_11,	&t4, 0, 0,
/* t6_1 */	&t6, &t, &_15,	&t5, 0, 0,
/* t7_1 */	&t7, &t, &_20,	&t6, 0, 0,
/* t8_1 */	&t8, &t, &_27,	&t7, 0, 0,
/* t_1 */	&t, &iClock,	&TX0_4, 0, 0,
/* v0 */	&I1, &I0, 0, 0,
/* v1 */	&I3, &I2, 0, 0,
/* v2_1 */	&v2, &iClock,	&v0, 0, 0,
/* v2_2 */	&v2, &iClock,	&v1, 0, 0,
/* v3_1 */	&v3, &iClock,	&t8, &I4, 0, 0,
/* v3_2 */	&v3, &iClock,	&I5, 0, 0,
/* v3_3 */	&v3, &t, &_30,	&v3, 0, 0,
/* v4_1 */	&v4, &t, &_20,	&I4, 0, 0,
/* v5_2 */	&v5_1, &t, &_10,	0, &I5, 0,
/* v6 */	&v6, &I6, 0, &I7, 0,
/* v7_1 */	&v7, &c,	&v7, &I6, 0, &I7, 0,
};

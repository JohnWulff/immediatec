/********************************************************************
 *
 *	SOURCE:   ./Test0/bar3.ic
 *	OUTPUT:   ./Test0/bar3.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: bar3.c,v 1.1 2015/05/24 00:51:49 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

static iC_Gt _10     = { 1, -iC_NCONST, iC_ARITH, 0, "10", {0}, {0}, 0 };
iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, &_10 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB1 };
iC_Gt IX2_0    = { 1, -iC_INPX, iC_GATE, 0, "IX2.0", {0}, {0}, &IX0_0 };
iC_Gt IX2_1    = { 1, -iC_INPX, iC_GATE, 0, "IX2.1", {0}, {0}, &IX2_0 };
iC_Gt IX2_2    = { 1, -iC_INPX, iC_GATE, 0, "IX2.2", {0}, {0}, &IX2_1 };
iC_Gt IX2_3    = { 1, -iC_INPX, iC_GATE, 0, "IX2.3", {0}, {0}, &IX2_2 };
iC_Gt IX2_4    = { 1, -iC_INPX, iC_GATE, 0, "IX2.4", {0}, {0}, &IX2_3 };
iC_Gt IX2_5    = { 1, -iC_INPX, iC_GATE, 0, "IX2.5", {0}, {0}, &IX2_4 };
iC_Gt IX2_6    = { 1, -iC_INPX, iC_GATE, 0, "IX2.6", {0}, {0}, &IX2_5 };
iC_Gt IX2_7    = { 1, -iC_INPX, iC_GATE, 0, "IX2.7", {0}, {0}, &IX2_6 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[0]}, &IX2_7 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[3]}, &QB1_0 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[6]}, &QX0_0_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[9]}, &QX0_1_0 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[12]}, &QX0_2_0 };
iC_Gt QX0_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.4_0", {0}, {&iC_l_[15]}, &QX0_3_0 };
iC_Gt QX0_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.5_0", {0}, {&iC_l_[18]}, &QX0_4_0 };
iC_Gt QX0_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.6_0", {0}, {&iC_l_[21]}, &QX0_5_0 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[24]}, &QX0_6_0 };
iC_Gt QX2_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.0_0", {0}, {&iC_l_[27]}, &QX0_7_0 };
iC_Gt QX2_3    = { 1, -iC_FF, iC_GATE, 0, "QX2.3", {0}, {0}, &QX2_0_0 };
iC_Gt QX2_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.3_0", {0}, {&iC_l_[30]}, &QX2_3 };
iC_Gt QX2_3_1  = { 1, -iC_OR, iC_D_FF, 0, "QX2.3_1", {&iC_l_[33]}, {&iC_l_[36]}, &QX2_3_0 };
iC_Gt QX2_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.4_0", {0}, {&iC_l_[39]}, &QX2_3_1 };
iC_Gt QX2_5    = { 1, -iC_FF, iC_GATE, 0, "QX2.5", {0}, {0}, &QX2_4_0 };
iC_Gt QX2_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.5_0", {0}, {&iC_l_[42]}, &QX2_5 };
iC_Gt QX2_5_1  = { 1, -iC_OR, iC_D_FF, 0, "QX2.5_1", {&iC_l_[45]}, {&iC_l_[47]}, &QX2_5_0 };
iC_Gt QX2_6    = { 1, -iC_OR, iC_GATE, 0, "QX2.6", {0}, {&iC_l_[50]}, &QX2_5_1 };
iC_Gt QX2_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.6_0", {0}, {&iC_l_[54]}, &QX2_6 };
iC_Gt QX2_6_1  = { 1, -iC_AND, iC_GATE, 0, "QX2.6_1", {0}, {&iC_l_[57]}, &QX2_6_0 };
iC_Gt QX2_6_2  = { 1, -iC_AND, iC_GATE, 0, "QX2.6_2", {0}, {&iC_l_[62]}, &QX2_6_1 };
iC_Gt QX2_7    = { 1, -iC_OR, iC_GATE, 0, "QX2.7", {0}, {&iC_l_[67]}, &QX2_6_2 };
iC_Gt QX2_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.7_0", {0}, {&iC_l_[71]}, &QX2_7 };
iC_Gt QX2_7_1  = { 1, -iC_XOR, iC_GATE, 0, "QX2.7_1", {0}, {&iC_l_[74]}, &QX2_7_0 };
iC_Gt QX2_7_2  = { 1, -iC_ARN, iC_GATE, 0, "QX2.7_2", {0}, {&iC_l_[78]}, &QX2_7_1 };
iC_Gt TX0_4    = { 1, -iC_INPX, iC_GATE, 0, "TX0.4", {0}, {0}, &QX2_7_2 };
iC_Gt a        = { 1, -iC_ARN, iC_ARITH, 0, "a", {0}, {&iC_l_[81]}, &TX0_4 };
iC_Gt b0       = { 1, -iC_OR, iC_GATE, 0, "b0", {0}, {&iC_l_[84]}, &a };
iC_Gt b0_1     = { 1, -iC_FF, iC_GATE, 0, "b0_1", {0}, {0}, &b0 };
iC_Gt b0_2     = { 1, -iC_VF, iC_GATE, 0, "b0_2", {0}, {0}, &b0_1 };
iC_Gt b0_3     = { 1, -iC_AND, iC_CH_BIT, 0, "b0_3", {&iC_l_[88]}, {&iC_l_[90]}, &b0_2 };
iC_Gt b0_4     = { 1, -iC_AND, iC_D_FF, 0, "b0_4", {&iC_l_[94]}, {&iC_l_[97]}, &b0_3 };
iC_Gt b1       = { 1, -iC_VF, iC_GATE, 0, "b1", {0}, {0}, &b0_4 };
iC_Gt b1_1     = { 1, -iC_ARN, iC_CH_AR, 0, "b1_1", {&iC_l_[101]}, {&iC_l_[103]}, &b1 };
iC_Gt c0       = { 1, -iC_CLK, iC_CLCKL, 0, "c0", {0}, {0}, &b1_1 };
iC_Gt c0_1     = { 1, -iC_OR, iC_CLCK, 0, "c0_1", {&iC_l_[106]}, {&iC_l_[108]}, &c0 };
iC_Gt c1       = { 1, -iC_CLK, iC_CLCKL, 0, "c1", {0}, {0}, &c0_1 };
iC_Gt c1_1     = { 1, -iC_OR, iC_CLCK, 0, "c1_1", {&iC_l_[111]}, {&iC_l_[113]}, &c1 };
iC_Gt f0       = { 1, -iC_ARN, iC_GATE, 0, "f0", {0}, {&iC_l_[116]}, &c1_1 };
iC_Gt m0       = { 1, -iC_FF, iC_GATE, 0, "m0", {0}, {0}, &f0 };
iC_Gt m0_1     = { 1, -iC_OR, iC_S_FF, 0, "m0_1", {&iC_l_[119]}, {&iC_l_[121]}, &m0 };
iC_Gt m0_2     = { 1, -iC_AND, iC_R_FF, 0, "m0_2", {&iC_l_[124]}, {&iC_l_[126]}, &m0_1 };
iC_Gt m1       = { 1, -iC_FF, iC_GATE, 0, "m1", {0}, {0}, &m0_2 };
iC_Gt m1_1     = { 1, -iC_AND, iC_S_FF, 0, "m1_1", {&iC_l_[130]}, {&iC_l_[132]}, &m1 };
iC_Gt m1_2     = { 1, -iC_AND, iC_R_FF, 0, "m1_2", {&iC_l_[136]}, {&iC_l_[138]}, &m1_1 };
iC_Gt m2       = { 1, -iC_FF, iC_GATE, 0, "m2", {0}, {0}, &m1_2 };
iC_Gt m2_1     = { 1, -iC_AND, iC_S_FF, 0, "m2_1", {&iC_l_[142]}, {&iC_l_[144]}, &m2 };
iC_Gt m2_2     = { 1, -iC_AND, iC_R_FF, 0, "m2_2", {&iC_l_[148]}, {&iC_l_[150]}, &m2_1 };
iC_Gt m3       = { 1, -iC_FF, iC_GATE, 0, "m3", {0}, {0}, &m2_2 };
iC_Gt m3_1     = { 1, -iC_AND, iC_S_FF, 0, "m3_1", {&iC_l_[154]}, {&iC_l_[156]}, &m3 };
iC_Gt m3_2     = { 1, -iC_AND, iC_R_FF, 0, "m3_2", {&iC_l_[160]}, {&iC_l_[162]}, &m3_1 };
iC_Gt m4       = { 1, -iC_FF, iC_GATE, 0, "m4", {0}, {0}, &m3_2 };
iC_Gt m4_1     = { 1, -iC_AND, iC_S_FF, 0, "m4_1", {&iC_l_[166]}, {&iC_l_[168]}, &m4 };
iC_Gt m4_2     = { 1, -iC_AND, iC_R_FF, 0, "m4_2", {&iC_l_[172]}, {&iC_l_[174]}, &m4_1 };
iC_Gt m5       = { 1, -iC_FF, iC_GATE, 0, "m5", {0}, {0}, &m4_2 };
iC_Gt m5_1     = { 1, -iC_AND, iC_S_FF, 0, "m5_1", {&iC_l_[178]}, {&iC_l_[180]}, &m5 };
iC_Gt m5_2     = { 1, -iC_AND, iC_R_FF, 0, "m5_2", {&iC_l_[184]}, {&iC_l_[186]}, &m5_1 };
iC_Gt m6       = { 1, -iC_FF, iC_GATE, 0, "m6", {0}, {0}, &m5_2 };
iC_Gt m6_1     = { 1, -iC_AND, iC_S_FF, 0, "m6_1", {&iC_l_[190]}, {&iC_l_[192]}, &m6 };
iC_Gt m6_2     = { 1, -iC_AND, iC_R_FF, 0, "m6_2", {&iC_l_[196]}, {&iC_l_[198]}, &m6_1 };
iC_Gt m7       = { 1, -iC_FF, iC_GATE, 0, "m7", {0}, {0}, &m6_2 };
iC_Gt m7_1     = { 1, -iC_AND, iC_S_FF, 0, "m7_1", {&iC_l_[202]}, {&iC_l_[204]}, &m7 };
iC_Gt m7_2     = { 1, -iC_OR, iC_R_FF, 0, "m7_2", {&iC_l_[208]}, {&iC_l_[210]}, &m7_1 };
iC_Gt t        = { 1, -iC_TIM, iC_TIMRL, 0, "t", {0}, {0}, &m7_2 };
iC_Gt t_1      = { 1, -iC_OR, iC_TIMR, 0, "t_1", {&iC_l_[213]}, {&iC_l_[215]}, &t };
iC_Gt up       = { 1, -iC_FF, iC_GATE, 0, "up", {0}, {0}, &t_1 };
iC_Gt up_1     = { 1, -iC_AND, iC_S_FF, 0, "up_1", {&iC_l_[218]}, {&iC_l_[220]}, &up };
iC_Gt up_2     = { 1, -iC_AND, iC_R_FF, 0, "up_2", {&iC_l_[224]}, {&iC_l_[226]}, &up_1 };
iC_Gt QB1      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB1", {0}, {(iC_Gt**)&IB1}, &up_2, 0 };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&m0}, &QB1, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&m1}, &QX0_0, 0 };
iC_Gt QX0_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.2", {0}, {(iC_Gt**)&m2}, &QX0_1, 0 };
iC_Gt QX0_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.3", {0}, {(iC_Gt**)&m3}, &QX0_2, 0 };
iC_Gt QX0_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.4", {0}, {(iC_Gt**)&m4}, &QX0_3, 0 };
iC_Gt QX0_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.5", {0}, {(iC_Gt**)&m5}, &QX0_4, 0 };
iC_Gt QX0_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.6", {0}, {(iC_Gt**)&m6}, &QX0_5, 0 };
iC_Gt QX0_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.7", {0}, {(iC_Gt**)&m7}, &QX0_6, 0 };
iC_Gt QX2_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.0", {0}, {(iC_Gt**)&up}, &QX0_7, 0 };
iC_Gt QX2_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX2.4", {0}, {(iC_Gt**)&f0}, &QX2_0, 0 };
iC_Gt d0       = { 1, -iC_ALIAS, iC_ARITH, 0, "d0", {0}, {(iC_Gt**)&IB1}, &QX2_4, 0 };

iC_Gt *		iC___Test0_bar3_list = &d0;
iC_Gt **	iC_list[] = { &iC___Test0_bar3_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_2(iC_Gt * iC_gf) {
#line 52 "./Test0/bar3.ic"
	return iC_MV(1)/5;
#line 127 "./Test0/bar3.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
#line 57 "./Test0/bar3.ic"
	return iC_MV(1)<10;
#line 133 "./Test0/bar3.c"
} /* iC_3 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1_0 */	(iC_Gt*)0, &IB1, 0,
/* QX0.0_0 */	&m0, 0, 0,
/* QX0.1_0 */	&m1, 0, 0,
/* QX0.2_0 */	&m2, 0, 0,
/* QX0.3_0 */	&m3, 0, 0,
/* QX0.4_0 */	&m4, 0, 0,
/* QX0.5_0 */	&m5, 0, 0,
/* QX0.6_0 */	&m6, 0, 0,
/* QX0.7_0 */	&m7, 0, 0,
/* QX2.0_0 */	&up, 0, 0,
/* QX2.3_0 */	&QX2_3, 0, 0,
/* QX2.3_1 */	&QX2_3, &t, &_10,	&IX2_0, 0, 0,
/* QX2.4_0 */	&f0, 0, 0,
/* QX2.5_0 */	&QX2_5, 0, 0,
/* QX2.5_1 */	&QX2_5, &c1,	0, &QX2_5, 0,
/* QX2.6 */	&QX2_6_2, &QX2_6_1, 0, 0,
/* QX2.6_0 */	&QX2_6, 0, 0,
/* QX2.6_1 */	&IX2_2, &IX2_1, &IX2_0, 0, 0,
/* QX2.6_2 */	&IX2_5, &IX2_4, &IX2_3, 0, 0,
/* QX2.7 */	&QX2_7_2, &QX2_7_1, 0, 0,
/* QX2.7_0 */	&QX2_7, 0, 0,
/* QX2.7_1 */	&IX2_7, &IX2_6, 0, 0,
/* QX2.7_2 */	(iC_Gt*)iC_3, &a, 0,
/* a */		(iC_Gt*)iC_2, &IB1, 0,
/* b0 */	&b0_2, &b0_1, 0, 0,
/* b0_3 */	&b0_2, &iClock,	&IX0_0, 0, &f0, 0,
/* b0_4 */	&b0_1, &t, &IB1,	&f0, 0, &b0, 0,
/* b1_1 */	&b1, &iClock,	(iC_Gt*)0, &a, 0,
/* c0_1 */	&c0, &iClock,	&b0, 0, 0,
/* c1_1 */	&c1, &iClock,	&b1, 0, 0,
/* f0 */	(iC_Gt*)0, &IB1, 0,
/* m0_1 */	&m0, &c0,	0, &up, 0,
/* m0_2 */	&m0, &c0,	&up, 0, &m1, 0,
/* m1_1 */	&m1, &c0,	&m0, 0, &up, 0,
/* m1_2 */	&m1, &c0,	&up, 0, &m2, 0,
/* m2_1 */	&m2, &c0,	&m1, 0, &up, 0,
/* m2_2 */	&m2, &c0,	&up, 0, &m3, 0,
/* m3_1 */	&m3, &c0,	&m2, 0, &up, 0,
/* m3_2 */	&m3, &c0,	&up, 0, &m4, 0,
/* m4_1 */	&m4, &c0,	&m3, 0, &up, 0,
/* m4_2 */	&m4, &c0,	&up, 0, &m5, 0,
/* m5_1 */	&m5, &c0,	&m4, 0, &up, 0,
/* m5_2 */	&m5, &c0,	&up, 0, &m6, 0,
/* m6_1 */	&m6, &c0,	&m5, 0, &up, 0,
/* m6_2 */	&m6, &c0,	&up, 0, &m7, 0,
/* m7_1 */	&m7, &c0,	&m6, 0, &up, 0,
/* m7_2 */	&m7, &c0,	&up, 0, 0,
/* t_1 */	&t, &iClock,	&TX0_4, 0, 0,
/* up_1 */	&up, &c0,	&m7, 0, &up, 0,
/* up_2 */	&up, &c0,	&up, 0, &m0, 0,
};

/********************************************************************
 *
 *	SOURCE:   ./Test0/bar1.ic
 *	OUTPUT:   ./Test0/bar1.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: bar1.c,v 1.1 2015/05/24 00:51:48 jw Exp $ -O7";

#include	<icg.h>

static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB4      = { 1, -iC_INPW, iC_ARITH, 0, "IB4", {0}, {0}, 0 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB4 };
iC_Gt QB4_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB4_0", {0}, {&iC_l_[0]}, &IX0_0 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[3]}, &QB4_0 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[6]}, &QX0_0_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[9]}, &QX0_1_0 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[12]}, &QX0_2_0 };
iC_Gt QX0_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.4_0", {0}, {&iC_l_[15]}, &QX0_3_0 };
iC_Gt QX0_5_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.5_0", {0}, {&iC_l_[18]}, &QX0_4_0 };
iC_Gt QX0_6_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.6_0", {0}, {&iC_l_[21]}, &QX0_5_0 };
iC_Gt QX0_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.7_0", {0}, {&iC_l_[24]}, &QX0_6_0 };
iC_Gt QX1_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.0_0", {0}, {&iC_l_[27]}, &QX0_7_0 };
iC_Gt QX1_4_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.4_0", {0}, {&iC_l_[30]}, &QX1_0_0 };
iC_Gt TX0_4    = { 1, -iC_INPX, iC_GATE, 0, "TX0.4", {0}, {0}, &QX1_4_0 };
iC_Gt b0       = { 1, -iC_OR, iC_GATE, 0, "b0", {0}, {&iC_l_[33]}, &TX0_4 };
iC_Gt b0_1     = { 1, -iC_FF, iC_GATE, 0, "b0_1", {0}, {0}, &b0 };
iC_Gt b0_2     = { 1, -iC_VF, iC_GATE, 0, "b0_2", {0}, {0}, &b0_1 };
iC_Gt b0_3     = { 1, -iC_AND, iC_CH_BIT, 0, "b0_3", {&iC_l_[37]}, {&iC_l_[39]}, &b0_2 };
iC_Gt b0_4     = { 1, -iC_AND, iC_D_FF, 0, "b0_4", {&iC_l_[43]}, {&iC_l_[46]}, &b0_3 };
iC_Gt c0       = { 1, -iC_CLK, iC_CLCKL, 0, "c0", {0}, {0}, &b0_4 };
iC_Gt c0_1     = { 1, -iC_OR, iC_CLCK, 0, "c0_1", {&iC_l_[50]}, {&iC_l_[52]}, &c0 };
iC_Gt f0       = { 1, -iC_ARN, iC_GATE, 0, "f0", {0}, {&iC_l_[55]}, &c0_1 };
iC_Gt m0       = { 1, -iC_FF, iC_GATE, 0, "m0", {0}, {0}, &f0 };
iC_Gt m0_1     = { 1, -iC_OR, iC_S_FF, 0, "m0_1", {&iC_l_[58]}, {&iC_l_[60]}, &m0 };
iC_Gt m0_2     = { 1, -iC_AND, iC_R_FF, 0, "m0_2", {&iC_l_[63]}, {&iC_l_[65]}, &m0_1 };
iC_Gt m1       = { 1, -iC_FF, iC_GATE, 0, "m1", {0}, {0}, &m0_2 };
iC_Gt m1_1     = { 1, -iC_AND, iC_S_FF, 0, "m1_1", {&iC_l_[69]}, {&iC_l_[71]}, &m1 };
iC_Gt m1_2     = { 1, -iC_AND, iC_R_FF, 0, "m1_2", {&iC_l_[75]}, {&iC_l_[77]}, &m1_1 };
iC_Gt m2       = { 1, -iC_FF, iC_GATE, 0, "m2", {0}, {0}, &m1_2 };
iC_Gt m2_1     = { 1, -iC_AND, iC_S_FF, 0, "m2_1", {&iC_l_[81]}, {&iC_l_[83]}, &m2 };
iC_Gt m2_2     = { 1, -iC_AND, iC_R_FF, 0, "m2_2", {&iC_l_[87]}, {&iC_l_[89]}, &m2_1 };
iC_Gt m3       = { 1, -iC_FF, iC_GATE, 0, "m3", {0}, {0}, &m2_2 };
iC_Gt m3_1     = { 1, -iC_AND, iC_S_FF, 0, "m3_1", {&iC_l_[93]}, {&iC_l_[95]}, &m3 };
iC_Gt m3_2     = { 1, -iC_AND, iC_R_FF, 0, "m3_2", {&iC_l_[99]}, {&iC_l_[101]}, &m3_1 };
iC_Gt m4       = { 1, -iC_FF, iC_GATE, 0, "m4", {0}, {0}, &m3_2 };
iC_Gt m4_1     = { 1, -iC_AND, iC_S_FF, 0, "m4_1", {&iC_l_[105]}, {&iC_l_[107]}, &m4 };
iC_Gt m4_2     = { 1, -iC_AND, iC_R_FF, 0, "m4_2", {&iC_l_[111]}, {&iC_l_[113]}, &m4_1 };
iC_Gt m5       = { 1, -iC_FF, iC_GATE, 0, "m5", {0}, {0}, &m4_2 };
iC_Gt m5_1     = { 1, -iC_AND, iC_S_FF, 0, "m5_1", {&iC_l_[117]}, {&iC_l_[119]}, &m5 };
iC_Gt m5_2     = { 1, -iC_AND, iC_R_FF, 0, "m5_2", {&iC_l_[123]}, {&iC_l_[125]}, &m5_1 };
iC_Gt m6       = { 1, -iC_FF, iC_GATE, 0, "m6", {0}, {0}, &m5_2 };
iC_Gt m6_1     = { 1, -iC_AND, iC_S_FF, 0, "m6_1", {&iC_l_[129]}, {&iC_l_[131]}, &m6 };
iC_Gt m6_2     = { 1, -iC_AND, iC_R_FF, 0, "m6_2", {&iC_l_[135]}, {&iC_l_[137]}, &m6_1 };
iC_Gt m7       = { 1, -iC_FF, iC_GATE, 0, "m7", {0}, {0}, &m6_2 };
iC_Gt m7_1     = { 1, -iC_AND, iC_S_FF, 0, "m7_1", {&iC_l_[141]}, {&iC_l_[143]}, &m7 };
iC_Gt m7_2     = { 1, -iC_OR, iC_R_FF, 0, "m7_2", {&iC_l_[147]}, {&iC_l_[149]}, &m7_1 };
iC_Gt t        = { 1, -iC_TIM, iC_TIMRL, 0, "t", {0}, {0}, &m7_2 };
iC_Gt t_1      = { 1, -iC_OR, iC_TIMR, 0, "t_1", {&iC_l_[152]}, {&iC_l_[154]}, &t };
iC_Gt t_2      = { 1, -iC_OR, iC_TIMR, 0, "t_2", {&iC_l_[157]}, {&iC_l_[159]}, &t_1 };
iC_Gt up       = { 1, -iC_FF, iC_GATE, 0, "up", {0}, {0}, &t_2 };
iC_Gt up_1     = { 1, -iC_AND, iC_S_FF, 0, "up_1", {&iC_l_[162]}, {&iC_l_[164]}, &up };
iC_Gt up_2     = { 1, -iC_AND, iC_R_FF, 0, "up_2", {&iC_l_[168]}, {&iC_l_[170]}, &up_1 };
iC_Gt QB4      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB4", {0}, {(iC_Gt**)&IB4}, &up_2, 0 };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&m0}, &QB4, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&m1}, &QX0_0, 0 };
iC_Gt QX0_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.2", {0}, {(iC_Gt**)&m2}, &QX0_1, 0 };
iC_Gt QX0_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.3", {0}, {(iC_Gt**)&m3}, &QX0_2, 0 };
iC_Gt QX0_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.4", {0}, {(iC_Gt**)&m4}, &QX0_3, 0 };
iC_Gt QX0_5    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.5", {0}, {(iC_Gt**)&m5}, &QX0_4, 0 };
iC_Gt QX0_6    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.6", {0}, {(iC_Gt**)&m6}, &QX0_5, 0 };
iC_Gt QX0_7    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.7", {0}, {(iC_Gt**)&m7}, &QX0_6, 0 };
iC_Gt QX1_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.0", {0}, {(iC_Gt**)&up}, &QX0_7, 0 };
iC_Gt QX1_4    = { 1, -iC_ALIAS, iC_GATE, 0, "QX1.4", {0}, {(iC_Gt**)&f0}, &QX1_0, 0 };
iC_Gt d0       = { 1, -iC_ALIAS, iC_ARITH, 0, "d0", {0}, {(iC_Gt**)&IB4}, &QX1_4, 0 };

iC_Gt *		iC___Test0_bar1_list = &d0;
iC_Gt **	iC_list[] = { &iC___Test0_bar1_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB4_0 */	(iC_Gt*)0, &IB4, 0,
/* QX0.0_0 */	&m0, 0, 0,
/* QX0.1_0 */	&m1, 0, 0,
/* QX0.2_0 */	&m2, 0, 0,
/* QX0.3_0 */	&m3, 0, 0,
/* QX0.4_0 */	&m4, 0, 0,
/* QX0.5_0 */	&m5, 0, 0,
/* QX0.6_0 */	&m6, 0, 0,
/* QX0.7_0 */	&m7, 0, 0,
/* QX1.0_0 */	&up, 0, 0,
/* QX1.4_0 */	&f0, 0, 0,
/* b0 */	&b0_2, &b0_1, 0, 0,
/* b0_3 */	&b0_2, &iClock,	&IX0_0, 0, &f0, 0,
/* b0_4 */	&b0_1, &t, &IB4,	&f0, 0, &b0, 0,
/* c0_1 */	&c0, &iClock,	&b0, 0, 0,
/* f0 */	(iC_Gt*)0, &IB4, 0,
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
/* t_2 */	&t, &iClock,	0, &TX0_4, 0,
/* up_1 */	&up, &c0,	&m6, 0, &m7, 0,
/* up_2 */	&up, &c0,	&m0, 0, &m1, 0,
};

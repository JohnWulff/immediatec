/********************************************************************
 *
 *	SOURCE:   ./Test0/mixmod.ic
 *	OUTPUT:   ./Test0/mixmod.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: mixmod.c,v 1.1 2015/05/24 00:51:54 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, 0 };
iC_Gt IB5      = { 1, -iC_INPW, iC_ARITH, 0, "IB5", {0}, {0}, &IB1 };
iC_Gt IW2      = { 1, -iC_INPW, iC_ARITH, 0, "IW2", {0}, {0}, &IB5 };
iC_Gt IW4      = { 1, -iC_INPW, iC_ARITH, 0, "IW4", {0}, {0}, &IW2 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IW4 };
iC_Gt IX1_1    = { 1, -iC_INPX, iC_GATE, 0, "IX1.1", {0}, {0}, &IX0_0 };
iC_Gt IX2_0    = { 1, -iC_INPX, iC_GATE, 0, "IX2.0", {0}, {0}, &IX1_1 };
iC_Gt IX3_0    = { 1, -iC_INPX, iC_GATE, 0, "IX3.0", {0}, {0}, &IX2_0 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[0]}, &IX3_0 };
iC_Gt QB6      = { 1, -iC_ARN, iC_ARITH, 0, "QB6", {0}, {&iC_l_[3]}, &QB1_0 };
iC_Gt QB6_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB6_0", {0}, {&iC_l_[6]}, &QB6 };
iC_Gt QB9      = { 1, -iC_ARN, iC_ARITH, 0, "QB9", {0}, {&iC_l_[9]}, &QB6_0 };
iC_Gt QB9_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB9_0", {0}, {&iC_l_[13]}, &QB9 };
iC_Gt QW11_0   = { 1, -iC_ARN, iC_OUTW, 0, "QW11_0", {0}, {&iC_l_[16]}, &QB9_0 };
iC_Gt QW2      = { 1, -iC_AND, iC_ARITH, 0, "QW2", {0}, {&iC_l_[19]}, &QW11_0 };
iC_Gt QW2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QW2_0", {0}, {&iC_l_[23]}, &QW2 };
iC_Gt QW4      = { 1, -iC_ARN, iC_ARITH, 0, "QW4", {0}, {&iC_l_[26]}, &QW2_0 };
iC_Gt QW4_0    = { 1, -iC_ARN, iC_OUTW, 0, "QW4_0", {0}, {&iC_l_[29]}, &QW4 };
iC_Gt QW4_1    = { 1, -iC_OR, iC_ARITH, 0, "QW4_1", {0}, {&iC_l_[32]}, &QW4_0 };
iC_Gt QW6_0    = { 1, -iC_ARN, iC_OUTW, 0, "QW6_0", {0}, {&iC_l_[35]}, &QW4_1 };
iC_Gt QW8_0    = { 1, -iC_ARN, iC_OUTW, 0, "QW8_0", {0}, {&iC_l_[38]}, &QW6_0 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[41]}, &QW8_0 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[44]}, &QX0_0_0 };
iC_Gt QX1_0    = { 1, -iC_OR, iC_GATE, 0, "QX1.0", {0}, {&iC_l_[47]}, &QX0_1_0 };
iC_Gt QX1_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX1.0_0", {0}, {&iC_l_[51]}, &QX1_0 };
iC_Gt QX2_0    = { 1, -iC_AND, iC_GATE, 0, "QX2.0", {0}, {&iC_l_[54]}, &QX1_0_0 };
iC_Gt QX2_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX2.0_0", {0}, {&iC_l_[58]}, &QX2_0 };
iC_Gt QX5_7    = { 1, -iC_XOR, iC_GATE, 0, "QX5.7", {0}, {&iC_l_[61]}, &QX2_0_0 };
iC_Gt QX5_7_0  = { 1, -iC_OR, iC_OUTX, 0, "QX5.7_0", {0}, {&iC_l_[65]}, &QX5_7 };
iC_Gt QB1      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB1", {0}, {(iC_Gt**)&IB1}, &QX5_7_0, 0 };
iC_Gt QW11     = { 1, -iC_ALIAS, iC_ARITH, 0, "QW11", {0}, {(iC_Gt**)&IB1}, &QB1, 0 };
iC_Gt QW6      = { 1, -iC_ALIAS, iC_ARITH, 0, "QW6", {0}, {(iC_Gt**)&QB6}, &QW11, 0 };
iC_Gt QW8      = { 1, -iC_ALIAS, iC_ARITH, 0, "QW8", {0}, {(iC_Gt**)&QB9}, &QW6, 0 };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&IX0_0}, &QW8, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&IX1_1}, &QX0_0, 0 };
iC_Gt bit2     = { 1, -iC_ALIAS, iC_GATE, 0, "bit2", {0}, {(iC_Gt**)&IX2_0}, &QX0_1, 0 };
iC_Gt bit3     = { 1, -iC_ALIAS, iC_GATE, 0, "bit3", {0}, {(iC_Gt**)&IX3_0}, &bit2, 0 };
iC_Gt byte5    = { 1, -iC_ALIAS, iC_ARITH, 0, "byte5", {0}, {(iC_Gt**)&IB5}, &bit3, 0 };
iC_Gt word2    = { 1, -iC_ALIAS, iC_ARITH, 0, "word2", {0}, {(iC_Gt**)&IW2}, &byte5, 0 };
iC_Gt word4    = { 1, -iC_ALIAS, iC_ARITH, 0, "word4", {0}, {(iC_Gt**)&IW4}, &word2, 0 };

iC_Gt *		iC___Test0_mixmod_list = &word4;
iC_Gt **	iC_list[] = { &iC___Test0_mixmod_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

static int iC_2(iC_Gt * iC_gf) {
#line 29 "./Test0/mixmod.ic"
	return 5*iC_MV(1);
#line 76 "./Test0/mixmod.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
#line 33 "./Test0/mixmod.ic"
	return iC_MV(1)/5;
#line 82 "./Test0/mixmod.c"
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
#line 34 "./Test0/mixmod.ic"
	return iC_MV(1)+iC_MV(2);
#line 88 "./Test0/mixmod.c"
} /* iC_4 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1_0 */	(iC_Gt*)0, &IB1, 0,
/* QB6 */	(iC_Gt*)iC_3, &IW4, 0,
/* QB6_0 */	(iC_Gt*)0, &QB6, 0,
/* QB9 */	(iC_Gt*)iC_4, &IB5, &IW2, 0,
/* QB9_0 */	(iC_Gt*)0, &QB9, 0,
/* QW11_0 */	(iC_Gt*)0, &IB1, 0,
/* QW2 */	&QX2_0, &QX1_0, 0, 0,
/* QW2_0 */	(iC_Gt*)0, &QW2, 0,
/* QW4 */	(iC_Gt*)iC_2, &QW4_1, 0,
/* QW4_0 */	(iC_Gt*)0, &QW4, 0,
/* QW4_1 */	&QX5_7, 0, 0,
/* QW6_0 */	(iC_Gt*)0, &QB6, 0,
/* QW8_0 */	(iC_Gt*)0, &QB9, 0,
/* QX0.0_0 */	&IX0_0, 0, 0,
/* QX0.1_0 */	&IX1_1, 0, 0,
/* QX1.0 */	&IX3_0, &IX2_0, 0, 0,
/* QX1.0_0 */	&QX1_0, 0, 0,
/* QX2.0 */	&IX3_0, &IX2_0, 0, 0,
/* QX2.0_0 */	&QX2_0, 0, 0,
/* QX5.7 */	&IX3_0, &IX2_0, 0, 0,
/* QX5.7_0 */	&QX5_7, 0, 0,
};

/********************************************************************
 *
 *	SOURCE:   ./Test0/n1.ic
 *	OUTPUT:   ./Test0/n1.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: n1.c,v 1.1 2015/05/24 00:51:54 jw Exp $ -O7";

#include	<icg.h>

static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, 0 };
iC_Gt IX0_1    = { 1, -iC_INPX, iC_GATE, 0, "IX0.1", {0}, {0}, &IX0_0 };
iC_Gt IX0_2    = { 1, -iC_INPX, iC_GATE, 0, "IX0.2", {0}, {0}, &IX0_1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_2 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[0]}, &IX0_3 };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[3]}, &QX0_0_0 };
iC_Gt QX0_2_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.2_0", {0}, {&iC_l_[6]}, &QX0_1_0 };
iC_Gt QX0_3_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.3_0", {0}, {&iC_l_[9]}, &QX0_2_0 };
iC_Gt a        = { 1, -iC_AND, iC_GATE, 0, "a", {0}, {&iC_l_[12]}, &QX0_3_0 };
iC_Gt b        = { 1, -iC_AND, iC_GATE, 0, "b", {0}, {&iC_l_[16]}, &a };
iC_Gt c        = { 1, -iC_OR, iC_GATE, 0, "c", {0}, {&iC_l_[20]}, &b };
iC_Gt d        = { 1, -iC_XOR, iC_GATE, 0, "d", {0}, {&iC_l_[24]}, &c };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&a}, &d, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&b}, &QX0_0, 0 };
iC_Gt QX0_2    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.2", {0}, {(iC_Gt**)&c}, &QX0_1, 0 };
iC_Gt QX0_3    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.3", {0}, {(iC_Gt**)&d}, &QX0_2, 0 };

iC_Gt *		iC___Test0_n1_list = &QX0_3;
iC_Gt **	iC_list[] = { &iC___Test0_n1_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QX0.0_0 */	&a, 0, 0,
/* QX0.1_0 */	&b, 0, 0,
/* QX0.2_0 */	&c, 0, 0,
/* QX0.3_0 */	&d, 0, 0,
/* a */		&IX0_1, &IX0_0, 0, 0,
/* b */		&IX0_3, &IX0_2, 0, 0,
/* c */		&b, &a, 0, 0,
/* d */		&b, &a, 0, 0,
};

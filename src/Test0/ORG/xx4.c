/********************************************************************
 *
 *	SOURCE:   ./Test0/xx4.ic
 *	OUTPUT:   ./Test0/xx4.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: xx4.c,v 1.1 2015/05/24 00:51:59 jw Exp $ -O7";

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
iC_Gt IX1_0    = { 1, -iC_INPX, iC_GATE, 0, "IX1.0", {0}, {0}, &IX0_3 };
iC_Gt IX1_1    = { 1, -iC_INPX, iC_GATE, 0, "IX1.1", {0}, {0}, &IX1_0 };
iC_Gt IX1_2    = { 1, -iC_INPX, iC_GATE, 0, "IX1.2", {0}, {0}, &IX1_1 };
iC_Gt IX1_3    = { 1, -iC_INPX, iC_GATE, 0, "IX1.3", {0}, {0}, &IX1_2 };
iC_Gt abcx0    = { 1, -iC_AND, iC_GATE, 0, "abcx0", {0}, {&iC_l_[0]}, &IX1_3 };
iC_Gt abcx1    = { 1, -iC_AND, iC_GATE, 0, "abcx1", {0}, {&iC_l_[4]}, &abcx0 };
iC_Gt abcx2    = { 1, -iC_AND, iC_GATE, 0, "abcx2", {0}, {&iC_l_[8]}, &abcx1 };
iC_Gt abcx3    = { 1, -iC_AND, iC_GATE, 0, "abcx3", {0}, {&iC_l_[12]}, &abcx2 };
iC_Gt defx0    = { 1, -iC_AND, iC_GATE, 0, "defx0", {0}, {&iC_l_[16]}, &abcx3 };
iC_Gt defx1    = { 1, -iC_AND, iC_GATE, 0, "defx1", {0}, {&iC_l_[20]}, &defx0 };
iC_Gt defx2    = { 1, -iC_AND, iC_GATE, 0, "defx2", {0}, {&iC_l_[24]}, &defx1 };
iC_Gt defx3    = { 1, -iC_AND, iC_GATE, 0, "defx3", {0}, {&iC_l_[28]}, &defx2 };
iC_Gt ghix0    = { 1, -iC_AND, iC_GATE, 0, "ghix0", {0}, {&iC_l_[32]}, &defx3 };
iC_Gt ghix1    = { 1, -iC_AND, iC_GATE, 0, "ghix1", {0}, {&iC_l_[36]}, &ghix0 };
iC_Gt ghix2    = { 1, -iC_AND, iC_GATE, 0, "ghix2", {0}, {&iC_l_[40]}, &ghix1 };
iC_Gt ghix3    = { 1, -iC_AND, iC_GATE, 0, "ghix3", {0}, {&iC_l_[44]}, &ghix2 };
iC_Gt jklx0    = { 1, -iC_AND, iC_GATE, 0, "jklx0", {0}, {&iC_l_[48]}, &ghix3 };
iC_Gt jklx1    = { 1, -iC_AND, iC_GATE, 0, "jklx1", {0}, {&iC_l_[52]}, &jklx0 };
iC_Gt jklx2    = { 1, -iC_AND, iC_GATE, 0, "jklx2", {0}, {&iC_l_[56]}, &jklx1 };
iC_Gt jklx3    = { 1, -iC_AND, iC_GATE, 0, "jklx3", {0}, {&iC_l_[60]}, &jklx2 };

iC_Gt *		iC___Test0_xx4_list = &jklx3;
iC_Gt **	iC_list[] = { &iC___Test0_xx4_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* abcx0 */	&IX1_0, &IX0_0, 0, 0,
/* abcx1 */	&IX1_1, &IX0_1, 0, 0,
/* abcx2 */	&IX1_2, &IX0_2, 0, 0,
/* abcx3 */	&IX1_3, &IX0_3, 0, 0,
/* defx0 */	&IX1_0, &IX0_0, 0, 0,
/* defx1 */	&IX1_1, &IX0_1, 0, 0,
/* defx2 */	&IX1_2, &IX0_2, 0, 0,
/* defx3 */	&IX1_3, &IX0_3, 0, 0,
/* ghix0 */	&IX1_0, &IX0_0, 0, 0,
/* ghix1 */	&IX1_1, &IX0_1, 0, 0,
/* ghix2 */	&IX1_2, &IX0_2, 0, 0,
/* ghix3 */	&IX1_3, &IX0_3, 0, 0,
/* jklx0 */	&IX1_0, &IX0_0, 0, 0,
/* jklx1 */	&IX1_1, &IX0_1, 0, 0,
/* jklx2 */	&IX1_2, &IX0_2, 0, 0,
/* jklx3 */	&IX1_3, &IX0_3, 0, 0,
};

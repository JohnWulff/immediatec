/********************************************************************
 *
 *	SOURCE:   ./Test0/minus.ic
 *	OUTPUT:   ./Test0/minus.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: minus.c,v 1.1 2015/05/24 00:51:54 jw Exp $ -O7";

#include	<icg.h>

static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

static iC_Gt __1     = { 1, -iC_NCONST, iC_ARITH, 0, "-1", {0}, {0}, 0 };
static iC_Gt __1001  = { 1, -iC_NCONST, iC_ARITH, 0, "-1001", {0}, {0}, &__1 };
static iC_Gt __999   = { 1, -iC_NCONST, iC_ARITH, 0, "-999", {0}, {0}, &__1001 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[0]}, &__999 };
iC_Gt QL4_0    = { 1, -iC_ARN, iC_OUTW, 0, "QL4_0", {0}, {&iC_l_[3]}, &QB1_0 };
iC_Gt QW2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QW2_0", {0}, {&iC_l_[6]}, &QL4_0 };
iC_Gt QB1      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB1", {0}, {(iC_Gt**)&__1}, &QW2_0, 0 };
iC_Gt QL4      = { 1, -iC_ALIAS, iC_ARITH, 0, "QL4", {0}, {(iC_Gt**)&__1001}, &QB1, 0 };
iC_Gt QW2      = { 1, -iC_ALIAS, iC_ARITH, 0, "QW2", {0}, {(iC_Gt**)&__999}, &QL4, 0 };

iC_Gt *		iC___Test0_minus_list = &QW2;
iC_Gt **	iC_list[] = { &iC___Test0_minus_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1_0 */	(iC_Gt*)0, &__1, 0,
/* QL4_0 */	(iC_Gt*)0, &__1001, 0,
/* QW2_0 */	(iC_Gt*)0, &__999, 0,
};

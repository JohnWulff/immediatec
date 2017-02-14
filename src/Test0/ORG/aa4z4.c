/********************************************************************
 *
 *	SOURCE:   ./Test0/aa4z4.ic
 *	OUTPUT:   ./Test0/aa4z4.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"$Revision: icc_2.6-6-ge1cef2e-dirty $ -O7";

#include	<icg.h>

static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

static iC_Gt _13     = { 1, -iC_NCONST, iC_ARITH, 0, "13", {0}, {0}, 0 };
static iC_Gt _25     = { 1, -iC_NCONST, iC_ARITH, 0, "25", {0}, {0}, &_13 };
static iC_Gt _33     = { 1, -iC_NCONST, iC_ARITH, 0, "33", {0}, {0}, &_25 };
static iC_Gt _43     = { 1, -iC_NCONST, iC_ARITH, 0, "43", {0}, {0}, &_33 };
static iC_Gt _17     = { 1, -iC_NCONST, iC_ARITH, 0, "17", {0}, {0}, &_43 };
iC_Gt QB1_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB1_0", {0}, {&iC_l_[0]}, &_17 };
iC_Gt QB2_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB2_0", {0}, {&iC_l_[3]}, &QB1_0 };
iC_Gt QB3_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB3_0", {0}, {&iC_l_[6]}, &QB2_0 };
iC_Gt QB4_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB4_0", {0}, {&iC_l_[9]}, &QB3_0 };
iC_Gt QB5_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB5_0", {0}, {&iC_l_[12]}, &QB4_0 };
iC_Gt QB6_0    = { 1, -iC_ARN, iC_OUTW, 0, "QB6_0", {0}, {&iC_l_[15]}, &QB5_0 };
iC_Gt QB1      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB1", {0}, {(iC_Gt**)&_17}, &QB6_0, 0 };
iC_Gt QB2      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB2", {0}, {(iC_Gt**)&_25}, &QB1, 0 };
iC_Gt QB3      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB3", {0}, {(iC_Gt**)&_43}, &QB2, 0 };
iC_Gt QB4      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB4", {0}, {(iC_Gt**)&_33}, &QB3, 0 };
iC_Gt QB5      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB5", {0}, {(iC_Gt**)&_33}, &QB4, 0 };
iC_Gt QB6      = { 1, -iC_ALIAS, iC_ARITH, 0, "QB6", {0}, {(iC_Gt**)&_13}, &QB5, 0 };

iC_Gt *		iC___Test0_aa4z4_list = &QB6;
iC_Gt **	iC_list[] = { &iC___Test0_aa4z4_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* QB1_0 */	(iC_Gt*)0, &_17, 0,
/* QB2_0 */	(iC_Gt*)0, &_25, 0,
/* QB3_0 */	(iC_Gt*)0, &_43, 0,
/* QB4_0 */	(iC_Gt*)0, &_33, 0,
/* QB5_0 */	(iC_Gt*)0, &_33, 0,
/* QB6_0 */	(iC_Gt*)0, &_13, 0,
};

/********************************************************************
 *
 *	SOURCE:   ./Test0/h1.ic
 *	OUTPUT:   ./Test0/h1.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: h1.c,v 1.1 2015/10/15 06:27:20 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB0      = { 1, -iC_INPW, iC_ARITH, 0, "IB0", {0}, {0}, 0 };
iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, &IB0 };
iC_Gt IB2      = { 1, -iC_INPW, iC_ARITH, 0, "IB2", {0}, {0}, &IB1 };
iC_Gt IB3      = { 1, -iC_INPW, iC_ARITH, 0, "IB3", {0}, {0}, &IB2 };
iC_Gt IB4      = { 1, -iC_INPW, iC_ARITH, 0, "IB4", {0}, {0}, &IB3 };
iC_Gt IB5      = { 1, -iC_INPW, iC_ARITH, 0, "IB5", {0}, {0}, &IB4 };
iC_Gt IB6      = { 1, -iC_INPW, iC_ARITH, 0, "IB6", {0}, {0}, &IB5 };
iC_Gt IB7      = { 1, -iC_INPW, iC_ARITH, 0, "IB7", {0}, {0}, &IB6 };
iC_Gt IB8      = { 1, -iC_INPW, iC_ARITH, 0, "IB8", {0}, {0}, &IB7 };
iC_Gt IB9      = { 1, -iC_INPW, iC_ARITH, 0, "IB9", {0}, {0}, &IB8 };
static iC_Gt _f10_1  = { 1, -iC_ARN, iC_F_SW, 0, "_f10_1", {&iC_l_[0]}, {&iC_l_[2]}, &IB9 };
static iC_Gt _f1_1   = { 1, -iC_ARN, iC_F_SW, 0, "_f1_1", {&iC_l_[5]}, {&iC_l_[7]}, &_f10_1 };
static iC_Gt _f2_1   = { 1, -iC_ARN, iC_F_SW, 0, "_f2_1", {&iC_l_[10]}, {&iC_l_[12]}, &_f1_1 };
static iC_Gt _f3_1   = { 1, -iC_ARN, iC_F_SW, 0, "_f3_1", {&iC_l_[15]}, {&iC_l_[17]}, &_f2_1 };
static iC_Gt _f4_1   = { 1, -iC_ARN, iC_F_SW, 0, "_f4_1", {&iC_l_[20]}, {&iC_l_[22]}, &_f3_1 };
static iC_Gt _f5_1   = { 1, -iC_ARN, iC_F_SW, 0, "_f5_1", {&iC_l_[25]}, {&iC_l_[27]}, &_f4_1 };
static iC_Gt _f6_1   = { 1, -iC_ARN, iC_F_SW, 0, "_f6_1", {&iC_l_[30]}, {&iC_l_[32]}, &_f5_1 };
static iC_Gt _f7_1   = { 1, -iC_ARN, iC_F_SW, 0, "_f7_1", {&iC_l_[35]}, {&iC_l_[37]}, &_f6_1 };
static iC_Gt _f8_1   = { 1, -iC_ARN, iC_F_SW, 0, "_f8_1", {&iC_l_[40]}, {&iC_l_[42]}, &_f7_1 };
static iC_Gt _f9_1   = { 1, -iC_ARN, iC_F_SW, 0, "_f9_1", {&iC_l_[45]}, {&iC_l_[47]}, &_f8_1 };

iC_Gt *		iC___Test0_h1_list = &_f9_1;
iC_Gt **	iC_list[] = { &iC___Test0_h1_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 7 "./Test0/h1.ic"

#include	<stdio.h>

#line 57 "./Test0/h1.c"

static int iC_2(iC_Gt * iC_gf) {
    switch (iC_gf->gt_new)
#line 11 "./Test0/h1.ic"
{
	case 0:
	printf("Hello Nr. %d\n", 0);
    }
#line 66 "./Test0/h1.c"
    return 0;
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
    switch (iC_gf->gt_new)
#line 15 "./Test0/h1.ic"
{
	case 1:
	printf("Hello Nr. %d\n", 1);
    }
#line 77 "./Test0/h1.c"
    return 0;
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
    switch (iC_gf->gt_new)
#line 19 "./Test0/h1.ic"
{
	case 2:
	printf("Hello Nr. %d\n", 2);
    }
#line 88 "./Test0/h1.c"
    return 0;
} /* iC_4 */

static int iC_5(iC_Gt * iC_gf) {
    switch (iC_gf->gt_new)
#line 23 "./Test0/h1.ic"
{
	case 3:
	printf("Hello Nr. %d\n", 3);
    }
#line 99 "./Test0/h1.c"
    return 0;
} /* iC_5 */

static int iC_6(iC_Gt * iC_gf) {
    switch (iC_gf->gt_new)
#line 27 "./Test0/h1.ic"
{
	case 4:
	printf("Hello Nr. %d\n", 4);
    }
#line 110 "./Test0/h1.c"
    return 0;
} /* iC_6 */

static int iC_7(iC_Gt * iC_gf) {
    switch (iC_gf->gt_new)
#line 31 "./Test0/h1.ic"
{
	case 5:
	printf("Hello Nr. %d\n", 5);
    }
#line 121 "./Test0/h1.c"
    return 0;
} /* iC_7 */

static int iC_8(iC_Gt * iC_gf) {
    switch (iC_gf->gt_new)
#line 35 "./Test0/h1.ic"
{
	case 6:
	printf("Hello Nr. %d\n", 6);
    }
#line 132 "./Test0/h1.c"
    return 0;
} /* iC_8 */

static int iC_9(iC_Gt * iC_gf) {
    switch (iC_gf->gt_new)
#line 39 "./Test0/h1.ic"
{
	case 7:
	printf("Hello Nr. %d\n", 7);
    }
#line 143 "./Test0/h1.c"
    return 0;
} /* iC_9 */

static int iC_10(iC_Gt * iC_gf) {
    switch (iC_gf->gt_new)
#line 43 "./Test0/h1.ic"
{
	case 8:
	printf("Hello Nr. %d\n", 8);
    }
#line 154 "./Test0/h1.c"
    return 0;
} /* iC_10 */

static int iC_11(iC_Gt * iC_gf) {
    switch (iC_gf->gt_new)
#line 47 "./Test0/h1.ic"
{
	case 9:
	printf("Hello Nr. %d\n", 9);
    }
#line 165 "./Test0/h1.c"
    return 0;
} /* iC_11 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* _f10_1 */	(iC_Gt*)iC_11, &iClock,	(iC_Gt*)0, &IB9, 0,
/* _f1_1 */	(iC_Gt*)iC_2, &iClock,	(iC_Gt*)0, &IB0, 0,
/* _f2_1 */	(iC_Gt*)iC_3, &iClock,	(iC_Gt*)0, &IB1, 0,
/* _f3_1 */	(iC_Gt*)iC_4, &iClock,	(iC_Gt*)0, &IB2, 0,
/* _f4_1 */	(iC_Gt*)iC_5, &iClock,	(iC_Gt*)0, &IB3, 0,
/* _f5_1 */	(iC_Gt*)iC_6, &iClock,	(iC_Gt*)0, &IB4, 0,
/* _f6_1 */	(iC_Gt*)iC_7, &iClock,	(iC_Gt*)0, &IB5, 0,
/* _f7_1 */	(iC_Gt*)iC_8, &iClock,	(iC_Gt*)0, &IB6, 0,
/* _f8_1 */	(iC_Gt*)iC_9, &iClock,	(iC_Gt*)0, &IB7, 0,
/* _f9_1 */	(iC_Gt*)iC_10, &iClock,	(iC_Gt*)0, &IB8, 0,
};

/********************************************************************
 *
 *	SOURCE:   ./Test0/h3.ic
 *	OUTPUT:   ./Test0/h3.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: h3.c,v 1.1 2015/10/15 06:27:20 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
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
iC_Gt IX0_4    = { 1, -iC_INPX, iC_GATE, 0, "IX0.4", {0}, {0}, &IX0_3 };
iC_Gt IX0_5    = { 1, -iC_INPX, iC_GATE, 0, "IX0.5", {0}, {0}, &IX0_4 };
iC_Gt IX0_6    = { 1, -iC_INPX, iC_GATE, 0, "IX0.6", {0}, {0}, &IX0_5 };
iC_Gt IX0_7    = { 1, -iC_INPX, iC_GATE, 0, "IX0.7", {0}, {0}, &IX0_6 };
iC_Gt IX1_0    = { 1, -iC_INPX, iC_GATE, 0, "IX1.0", {0}, {0}, &IX0_7 };
iC_Gt IX1_1    = { 1, -iC_INPX, iC_GATE, 0, "IX1.1", {0}, {0}, &IX1_0 };
static iC_Gt _f10_1  = { 1, -iC_OR, iC_F_CE, 0, "_f10_1", {&iC_l_[0]}, {&iC_l_[2]}, &IX1_1 };
static iC_Gt _f1_1   = { 1, -iC_OR, iC_F_CE, 0, "_f1_1", {&iC_l_[5]}, {&iC_l_[7]}, &_f10_1 };
static iC_Gt _f2_1   = { 1, -iC_OR, iC_F_CE, 0, "_f2_1", {&iC_l_[10]}, {&iC_l_[12]}, &_f1_1 };
static iC_Gt _f3_1   = { 1, -iC_OR, iC_F_CE, 0, "_f3_1", {&iC_l_[15]}, {&iC_l_[17]}, &_f2_1 };
static iC_Gt _f4_1   = { 1, -iC_OR, iC_F_CE, 0, "_f4_1", {&iC_l_[20]}, {&iC_l_[22]}, &_f3_1 };
static iC_Gt _f5_1   = { 1, -iC_OR, iC_F_CE, 0, "_f5_1", {&iC_l_[25]}, {&iC_l_[27]}, &_f4_1 };
static iC_Gt _f6_1   = { 1, -iC_OR, iC_F_CE, 0, "_f6_1", {&iC_l_[30]}, {&iC_l_[32]}, &_f5_1 };
static iC_Gt _f7_1   = { 1, -iC_OR, iC_F_CE, 0, "_f7_1", {&iC_l_[35]}, {&iC_l_[37]}, &_f6_1 };
static iC_Gt _f8_1   = { 1, -iC_OR, iC_F_CE, 0, "_f8_1", {&iC_l_[40]}, {&iC_l_[42]}, &_f7_1 };
static iC_Gt _f9_1   = { 1, -iC_OR, iC_F_CE, 0, "_f9_1", {&iC_l_[45]}, {&iC_l_[47]}, &_f8_1 };

iC_Gt *		iC___Test0_h3_list = &_f9_1;
iC_Gt **	iC_list[] = { &iC___Test0_h3_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 7 "./Test0/h3.ic"

#include	<stdio.h>

#line 57 "./Test0/h3.c"

static int iC_2(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 11 "./Test0/h3.ic"
{
	int n;
	for (n = 0; n <= 0; n++) {
	    printf("%d: Hello Nr. %d\n", n, 0);
	}
    }
#line 68 "./Test0/h3.c"
    else
#line 16 "./Test0/h3.ic"
{
	printf("Good bye Nr.0\n");
    }
#line 74 "./Test0/h3.c"
    return 0;
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 19 "./Test0/h3.ic"
{
	int n;
	for (n = 0; n <= 1; n++) {
	    printf("%d: Hello Nr. %d\n", n, 1);
	}
    }
#line 87 "./Test0/h3.c"
    else
#line 24 "./Test0/h3.ic"
{
	printf("Good bye Nr.1\n");
    }
#line 93 "./Test0/h3.c"
    return 0;
} /* iC_3 */

static int iC_4(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 27 "./Test0/h3.ic"
{
	int n;
	for (n = 0; n <= 2; n++) {
	    printf("%d: Hello Nr. %d\n", n, 2);
	}
    }
#line 106 "./Test0/h3.c"
    else
#line 32 "./Test0/h3.ic"
{
	printf("Good bye Nr.2\n");
    }
#line 112 "./Test0/h3.c"
    return 0;
} /* iC_4 */

static int iC_5(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 35 "./Test0/h3.ic"
{
	int n;
	for (n = 0; n <= 3; n++) {
	    printf("%d: Hello Nr. %d\n", n, 3);
	}
    }
#line 125 "./Test0/h3.c"
    else
#line 40 "./Test0/h3.ic"
{
	printf("Good bye Nr.3\n");
    }
#line 131 "./Test0/h3.c"
    return 0;
} /* iC_5 */

static int iC_6(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 43 "./Test0/h3.ic"
{
	int n;
	for (n = 0; n <= 4; n++) {
	    printf("%d: Hello Nr. %d\n", n, 4);
	}
    }
#line 144 "./Test0/h3.c"
    else
#line 48 "./Test0/h3.ic"
{
	printf("Good bye Nr.4\n");
    }
#line 150 "./Test0/h3.c"
    return 0;
} /* iC_6 */

static int iC_7(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 51 "./Test0/h3.ic"
{
	int n;
	for (n = 0; n <= 5; n++) {
	    printf("%d: Hello Nr. %d\n", n, 5);
	}
    }
#line 163 "./Test0/h3.c"
    else
#line 56 "./Test0/h3.ic"
{
	printf("Good bye Nr.5\n");
    }
#line 169 "./Test0/h3.c"
    return 0;
} /* iC_7 */

static int iC_8(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 59 "./Test0/h3.ic"
{
	int n;
	for (n = 0; n <= 6; n++) {
	    printf("%d: Hello Nr. %d\n", n, 6);
	}
    }
#line 182 "./Test0/h3.c"
    else
#line 64 "./Test0/h3.ic"
{
	printf("Good bye Nr.6\n");
    }
#line 188 "./Test0/h3.c"
    return 0;
} /* iC_8 */

static int iC_9(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 67 "./Test0/h3.ic"
{
	int n;
	for (n = 0; n <= 7; n++) {
	    printf("%d: Hello Nr. %d\n", n, 7);
	}
    }
#line 201 "./Test0/h3.c"
    else
#line 72 "./Test0/h3.ic"
{
	printf("Good bye Nr.7\n");
    }
#line 207 "./Test0/h3.c"
    return 0;
} /* iC_9 */

static int iC_10(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 75 "./Test0/h3.ic"
{
	int n;
	for (n = 0; n <= 8; n++) {
	    printf("%d: Hello Nr. %d\n", n, 8);
	}
    }
#line 220 "./Test0/h3.c"
    else
#line 80 "./Test0/h3.ic"
{
	printf("Good bye Nr.8\n");
    }
#line 226 "./Test0/h3.c"
    return 0;
} /* iC_10 */

static int iC_11(iC_Gt * iC_gf) {
    if (iC_gf->gt_val < 0)
#line 83 "./Test0/h3.ic"
{
	int n;
	for (n = 0; n <= 9; n++) {
	    printf("%d: Hello Nr. %d\n", n, 9);
	}
    }
#line 239 "./Test0/h3.c"
    else
#line 88 "./Test0/h3.ic"
{
	printf("Good bye Nr.9\n");
    }
#line 245 "./Test0/h3.c"
    return 0;
} /* iC_11 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* _f10_1 */	(iC_Gt*)iC_11, &iClock,	&IX1_1, 0, 0,
/* _f1_1 */	(iC_Gt*)iC_2, &iClock,	&IX0_0, 0, 0,
/* _f2_1 */	(iC_Gt*)iC_3, &iClock,	&IX0_1, 0, 0,
/* _f3_1 */	(iC_Gt*)iC_4, &iClock,	&IX0_2, 0, 0,
/* _f4_1 */	(iC_Gt*)iC_5, &iClock,	&IX0_3, 0, 0,
/* _f5_1 */	(iC_Gt*)iC_6, &iClock,	&IX0_4, 0, 0,
/* _f6_1 */	(iC_Gt*)iC_7, &iClock,	&IX0_5, 0, 0,
/* _f7_1 */	(iC_Gt*)iC_8, &iClock,	&IX0_6, 0, 0,
/* _f8_1 */	(iC_Gt*)iC_9, &iClock,	&IX0_7, 0, 0,
/* _f9_1 */	(iC_Gt*)iC_10, &iClock,	&IX1_0, 0, 0,
};

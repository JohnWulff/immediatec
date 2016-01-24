/********************************************************************
 *
 *	SOURCE:   ./Test0/if33.ic
 *	OUTPUT:   ./Test0/if33.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: if33.c,v 1.1 2015/05/24 00:51:53 jw Exp $ -O7";

#include	<icg.h>

#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new
static iC_Gt *	iC_l_[];

/********************************************************************
 *
 *	Gate list
 *
 *******************************************************************/

iC_Gt IB1      = { 1, -iC_INPW, iC_ARITH, 0, "IB1", {0}, {0}, 0 };
iC_Gt IX0_0    = { 1, -iC_INPX, iC_GATE, 0, "IX0.0", {0}, {0}, &IB1 };
iC_Gt IX0_3    = { 1, -iC_INPX, iC_GATE, 0, "IX0.3", {0}, {0}, &IX0_0 };
static iC_Gt _f1_1   = { 1, -iC_ARN, iC_F_SW, 0, "_f1_1", {&iC_l_[0]}, {&iC_l_[3]}, &IX0_3 };
iC_Gt del1     = { 1, -iC_ARN, iC_ARITH, 0, "del1", {0}, {&iC_l_[6]}, &_f1_1 };
iC_Gt del1_1   = { 1, -iC_OR, iC_ARITH, 0, "del1_1", {0}, {&iC_l_[10]}, &del1 };
iC_Gt t        = { 1, -iC_TIM, iC_TIMRL, 0, "t", {0}, {0}, &del1_1 };
iC_Gt t_1      = { 1, -iC_OR, iC_TIMR, 0, "t_1", {&iC_l_[13]}, {&iC_l_[15]}, &t };
iC_Gt t_2      = { 1, -iC_VF, iC_GATE, 0, "t_2", {0}, {0}, &t_1 };
iC_Gt t_3      = { 1, -iC_OR, iC_CH_BIT, 0, "t_3", {&iC_l_[18]}, {&iC_l_[20]}, &t_2 };
iC_Gt del2     = { 1, -iC_ALIAS, iC_ARITH, 0, "del2", {0}, {(iC_Gt**)&del1}, &t_3, 0 };
iC_Gt ib1      = { 1, -iC_ALIAS, iC_ARITH, 0, "ib1", {0}, {(iC_Gt**)&IB1}, &del2, 0 };
iC_Gt t1       = { 1, -iC_ALIAS, iC_TIMRL, 0, "t1", {0}, {(iC_Gt**)&t}, &ib1, 0 };

iC_Gt *		iC___Test0_if33_list = &t1;
iC_Gt **	iC_list[] = { &iC___Test0_if33_list, 0, };

/********************************************************************
 *
 *	Literal blocks and embedded C fragment functions
 *
 *******************************************************************/

#line 1 "./Test0/if33.ic"

#include	<stdio.h>

#line 50 "./Test0/if33.c"

static int iC_2(iC_Gt * iC_gf) {
#line 8 "./Test0/if33.ic"
	return iC_MV(1)?iC_MV(2):5;
#line 55 "./Test0/if33.c"
} /* iC_2 */

static int iC_3(iC_Gt * iC_gf) {
    switch (iC_gf->gt_new)
#line 11 "./Test0/if33.ic"
{
    case 0:
	printf("IB1 is 0!!\n");
	break;
    case 1:
	printf("IB1 is 1!!\n");
	break;
    case 2:
	printf("IB1 is 2!!\n");
	break;
    case 3:
	printf("IB1 is 3!!\n");
	break;
    default:
	printf("IB1 is >= 4!!\n");
	break;
}
#line 78 "./Test0/if33.c"
    return 0;
} /* iC_3 */

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* _f1_1 */	(iC_Gt*)iC_3, &t, &del1,	(iC_Gt*)0, &IB1, 0,
/* del1 */	(iC_Gt*)iC_2, &del1_1, &IB1, 0,
/* del1_1 */	&IX0_0, 0, 0,
/* t_1 */	&t, &iClock,	&t_2, 0, 0,
/* t_3 */	&t_2, &iClock,	&IX0_3, 0, 0,
};

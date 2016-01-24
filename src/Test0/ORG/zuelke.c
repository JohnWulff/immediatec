/********************************************************************
 *
 *	SOURCE:   ./Test0/zuelke.ic
 *	OUTPUT:   ./Test0/zuelke.c
 *
 *******************************************************************/

static const char	iC_compiler[] =
"@(#)     $Id: zuelke.c,v 1.1 2015/05/24 00:52:00 jw Exp $ -O7";

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
iC_Gt IX0_4    = { 1, -iC_INPX, iC_GATE, 0, "IX0.4", {0}, {0}, &IX0_3 };
iC_Gt MotorDown = { 1, -iC_LATCH, iC_GATE, 0, "MotorDown", {0}, {&iC_l_[0]}, &IX0_4 };
iC_Gt MotorDown_1 = { 1, -iC_AND, iC_GATE, 0, "MotorDown_1", {0}, {&iC_l_[5]}, &MotorDown };
iC_Gt MotorDown_2 = { 1, -iC_OR, iC_GATE, 0, "MotorDown_2", {0}, {&iC_l_[9]}, &MotorDown_1 };
iC_Gt MotorUp  = { 1, -iC_LATCH, iC_GATE, 0, "MotorUp", {0}, {&iC_l_[13]}, &MotorDown_2 };
iC_Gt MotorUp_1 = { 1, -iC_AND, iC_GATE, 0, "MotorUp_1", {0}, {&iC_l_[18]}, &MotorUp };
iC_Gt MotorUp_2 = { 1, -iC_OR, iC_GATE, 0, "MotorUp_2", {0}, {&iC_l_[22]}, &MotorUp_1 };
iC_Gt Moving   = { 1, -iC_OR, iC_GATE, 0, "Moving", {0}, {&iC_l_[26]}, &MotorUp_2 };
iC_Gt QX0_0_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.0_0", {0}, {&iC_l_[30]}, &Moving };
iC_Gt QX0_1_0  = { 1, -iC_OR, iC_OUTX, 0, "QX0.1_0", {0}, {&iC_l_[33]}, &QX0_0_0 };
iC_Gt Bot      = { 1, -iC_ALIAS, iC_GATE, 0, "Bot", {0}, {(iC_Gt**)&IX0_4}, &QX0_1_0, 0 };
iC_Gt Down     = { 1, -iC_ALIAS, iC_GATE, 0, "Down", {0}, {(iC_Gt**)&IX0_1}, &Bot, 0 };
iC_Gt QX0_0    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.0", {0}, {(iC_Gt**)&MotorUp}, &Down, 0 };
iC_Gt QX0_1    = { 1, -iC_ALIAS, iC_GATE, 0, "QX0.1", {0}, {(iC_Gt**)&MotorDown}, &QX0_0, 0 };
iC_Gt Stop     = { 1, -iC_ALIAS, iC_GATE, 0, "Stop", {0}, {(iC_Gt**)&IX0_2}, &QX0_1, 0 };
iC_Gt Top      = { 1, -iC_ALIAS, iC_GATE, 0, "Top", {0}, {(iC_Gt**)&IX0_3}, &Stop, 0 };
iC_Gt Up       = { 1, -iC_ALIAS, iC_GATE, 0, "Up", {0}, {(iC_Gt**)&IX0_0}, &Top, 0 };

iC_Gt *		iC___Test0_zuelke_list = &Up;
iC_Gt **	iC_list[] = { &iC___Test0_zuelke_list, 0, };

/********************************************************************
 *
 *	Connection lists
 *
 *******************************************************************/

static iC_Gt *	iC_l_[] = {
/* MotorDown */	&MotorDown_1, &MotorDown, 0, &MotorDown_2, 0,
/* MotorDown_1 */	&IX0_1, 0, &Moving, 0,
/* MotorDown_2 */	&IX0_4, &IX0_2, 0, 0,
/* MotorUp */	&MotorUp_1, &MotorUp, 0, &MotorUp_2, 0,
/* MotorUp_1 */	&IX0_0, 0, &Moving, 0,
/* MotorUp_2 */	&IX0_3, &IX0_2, 0, 0,
/* Moving */	&MotorUp, &MotorDown, 0, 0,
/* QX0.0_0 */	&MotorUp, 0, 0,
/* QX0.1_0 */	&MotorDown, 0, 0,
};

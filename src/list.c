static const char list_c[] =
"@(#)$Id: list.c,v 1.3 2000/05/31 15:51:38 jw Exp $";
/********************************************************************
 *
 *	list.c
 *
 *	This module prepares provides an array for linking modules
 *	This source must be compiled every time when linking modules
 *	with load.a library to pick up info from inc1.h and inc2.h
 *
 *******************************************************************/

#include	<stdio.h>
#include	<pplc.h>
#include	"inc1.h"
#include	"inc2.h"

Gate **		i_list[] = { I_LIST 0 };	/* load several modules */

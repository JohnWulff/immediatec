static const char list_c[] =
"@(#)$Id: list.c,v 1.1 1999/08/06 19:10:37 jw Exp $";
/********************************************************************
 *
 *	list.c
 *
 *	This module prepares provides an array for linking modules
 *	This source must be compiled every time when linking modules
 *	with load.a library to pick up info from list.h
 *
 *******************************************************************/

#include	"list.h"

Gate **		i_list[] = { I_LIST
				, 0 };	/* load several modules */

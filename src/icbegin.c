static const char icbegin_c[] =
"@(#)$Id: icbegin.c,v 1.4 2009/08/21 06:04:02 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2009  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	icbegin.c
 *	parallel plc - dummy initialisation routine
 *
 *******************************************************************/

/********************************************************************
 *  The following initialisation function is one of two empty functions
 *  in the libict.a support library. The other is iCend().
 *  Either or both may be implemented in a literal block in iC source(s),
 *  in which case those function will be linked in preference.
 *******************************************************************/

/********************************************************************
 *  iCbegin() can be used to initialise immC variables etc.
 *******************************************************************/

int
iCbegin(void)				/* default initialisation function */
{
    return 0;				/* does nothing */
} /* iCbegin */

static const char icend_c[] =
"@(#)$Id: icend.c 1.5 $";
/********************************************************************
 *
 *	Copyright (C) 1985-2017  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	icend.c
 *	dummy termination routine
 *
 *******************************************************************/

/********************************************************************
 *  The following termination function is one of two empty functions
 *  in the libict.a support library. The other is iCbegin().
 *  Either or both may be implemented in a literal block in an iC source,
 *  in which case those function will be linked in preference.
 *******************************************************************/

/********************************************************************
 *  iCend() can be used to free allocated memory etc.  It may also
 *  contain a wait() call for any processes forked in iCbegin()
 *******************************************************************/

int
iCend(void)				/* default termination function */
{
    return -1;				/* does nothing */
} /* iCend */

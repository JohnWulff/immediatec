static const char inic_c[] =
"@(#)$Id: icbegin.c,v 1.1 2008/02/22 12:11:45 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2005  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <john@je-wulff.de>
 *
 *	inic.c
 *	parallel plc - dummy initialisation routine of immC variables
 *
 *******************************************************************/

/********************************************************************
 *  The following initialisation function is an empty function
 *  in the libict.a support library. It may be implemented in a
 *  literal block of an iC source, in which case that function will
 *  be linked in preference.
 *
 *  It can be used to initialise immC variables etc.
 *******************************************************************/

void
iniC(void)				/* default initialisation function */
{
					/* does nothing */
} /* iniC */

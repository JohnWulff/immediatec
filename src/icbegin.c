static const char icbegin_c[] =
"@(#)$Id: icbegin.c,v 1.5 2015/04/02 02:35:58 jw Exp $";
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
 *  
 *  The following initialisation function is one of two empty functions
 *  in the libict.a support library. The other is iCend().
 *  Either or both may be implemented in a literal block in iC source(s),
 *  in which case those function will be linked in preference.
 *
 *  iCbegin() can be used to initialise immC variables etc.
 *
 ********* PiFaceCAD handling ***************************************
 *
 *  Replace
 *    unsigned short P_channel = 0xffff;	// set here
 *  by    
 *    unsigned short P_channel = 0xfffe;	// to write direct to PiFaceCAD
 *
 *  if the system finds a real PiFaceCAD linked to the app, the system
 *  will set P_channel to 0, which causes writePiFaceCAD() to write
 *  direct to the PiFaceCAD
 *  else
 *  the system will register the string Gate named "PFCAD4" and set
 *  P_channel to send strings to it with writePiFaceCAD() via iCserver.
 *  On iCpiFace or an app with a PiFaceCAD, which has the PiFaceCAD digital
 *  input declared ExternOut, PFCAD4 is registered as a string receiver.
 *  When strings are received via PFCAD4 they are written to the PiFaceCAD.
 *
 *  NOTE 1: as with any immediate input or output IEC variable, there must
 *  be either both a sender and at least one receiver for the PFCAD4
 *  channel or the I/O can be direct on the one app. Otherwise run-time
 *  errors will happen.
 *
 *  NOTE 2: any code, which defines iCbegin() must also define P_channel
 *  and vice versa. Otherwise the linker will not find one or the other.
 *  
 *******************************************************************/

unsigned short	P_channel = 0xffff;	/* invalid PiFaceCAD channel */

int
iCbegin(void)				/* default initialisation function */
{
    return 0;				/* does nothing */
} /* iCbegin */

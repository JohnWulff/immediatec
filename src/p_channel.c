static const char p_channel_c[] =
"@(#)$Id: p_channel.c,v 1.2 2015/10/16 12:33:47 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 2015  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	p_channel.c
 *	parallel plc - dummy initialisation of P_channel
 *
 ********************************************************************
 *  
 *  The following initialisation only defines and initialise P_channel to 0xffff
 *  in the libict.a support library. This neutralises PiFaceCAD code in ict.c
 *
 ********************************************************************/

unsigned short P_channel = 0xffff;		/* not interested in PiFaceCAD */

//******** PiFaceCAD handling ***************************************
//
//  To write direct to PiFaceCAD include the following literal C code in app.ic
//  
//*******************************************************************

//    %{	/* Start of literal C code */
//    ...
//    unsigned short P_channel = 0xfffe;	/* want to write direct to PiFaceCAD */
//
//    int
//    iCbegin(int argc, char** argv)
//    {
//	...					/* code supporting PiFaceCAD */
//	return 1;
//    } /* iCbegin */
//    ...
//    %}	/* End of literal C code */

//*******************************************************************
//
//  if the system finds a real PiFaceCAD linked to the app
//     and P_channel == 0xfffe
//  the system will set P_channel to 0, which causes writePiFaceCAD()
//  to write direct to the PiFaceCAD
//  else
//  the system will register the string Gate named "PFCAD4" and set
//  P_channel to send strings to it with writePiFaceCAD() via iCserver.
//  On iCpiFace or an app with a PiFaceCAD, which has the PiFaceCAD digital
//  input declared ExternOut, PFCAD4 is registered as a string receiver.
//  When strings are received via PFCAD4 they are written to the PiFaceCAD.
//
//  NOTE: as with any immediate input or output IEC variable, there must
//  be either both a sender and at least one receiver for the PFCAD4
//  channel or the I/O can be direct on the one app. Otherwise run-time
//  errors will happen.
//  
//*******************************************************************

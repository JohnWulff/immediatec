static const char lmain_c[] =
"@(#)$Id: lmain.c,v 1.1 2002/07/08 13:01:29 jw Exp $";
/********************************************************************
 *
 *	simple driver for MKS yacc
 *
 *******************************************************************/

#include <stdio.h>

int
main(int argc, char **argv)
{
	return c_parse();
} /* main */

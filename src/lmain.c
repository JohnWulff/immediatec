static const char lmain_c[] =
"@(#)$Id: lmain.c,v 1.2 2002/07/26 18:47:44 jw Exp $";
/********************************************************************
 *
 *	simple driver for MKS yacc
 *
 *******************************************************************/

#include <stdio.h>

extern void copyAdjust(FILE* fp);

FILE *	yyin;

int
main(int argc, char **argv)
{
	int rv = 0;
	yyin = fopen(*++argv, "r");
	rv = c_parse();
	rewind(yyin);
	copyAdjust(yyin);
	
	return rv;
} /* main */

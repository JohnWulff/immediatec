static const char comp_h[] =
"@(#)$Id: gram.h,v 1.1 2002/07/07 13:18:33 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2001  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <john.wulff@inka.de>
 *
 *	gram.h
 *	header for C compiler
 *
 *******************************************************************/

typedef struct Token {
    int		start;
    int		end;
} Token;

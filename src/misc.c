static const char misc_c[] =
"@(#)$Id: misc.c,v 1.1 2004/01/03 21:44:49 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2001  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <john@je-wulff.de>
 *
 *	misc.c
 *	miscellanious functions shared by all modules
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"icg.h"
#include	"icc.h"

/********************************************************************
 *
 * version of malloc() which checks return value and works under Windows
 *
 *******************************************************************/

void *
emalloc(unsigned	nbytes)	/* check return from malloc */
{
    void *	bp;
#ifdef _WINDOWS
#if defined(_LARGE_) || defined(_HUGE_)
    GLOBALHANDLE		hglobal;

    if ((hglobal = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, nbytes)) == 0) {
	execerror("out of memory", NS, __FILE__, __LINE__);
    }
    bp = GlobalLock(hglobal);		/* actual pointer */
#else
    LOCALHANDLE		hlocal;

    if ((hlocal = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, nbytes)) == 0) {
	execerror("out of memory", NS, __FILE__, __LINE__);
    }
    bp = LocalLock(hlocal);		/* actual pointer */
#endif
#else

    if ((bp = malloc(nbytes)) == NULL) {
    }
#endif
    memset(bp, 0, nbytes);	/* when free() is used memory can be non zero */
    return bp;
}
#ifdef _WINDOWS

/********************************************************************
 *
 * if free under windows gives trouble, make this a null function
 *
 *******************************************************************/

void
efree(void *	p)
{
#if defined(_LARGE_) || defined(_HUGE_)
    GLOBALHANDLE		hglobal;

    hglobal = GlobalHandle(p);	/* retrieve the handle */
    GlobalUnlock(hglobal);
    GlobalFree(hglobal);		/* big deal */
#else
    LOCALHANDLE		hlocal;

    hlocal = LocalHandle(p);	/* retrieve the handle */
    LocalUnlock(hlocal);
    LocalFree(hlocal);		/* big deal */
#endif
} /* efree */
#endif

static const char misc_c[] =
"@(#)$Id: misc.c,v 1.10 2012/06/20 05:58:58 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2011  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file
 *
 *	misc.c
 *	miscellanious functions shared by all modules
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"icc.h"

unsigned char	iC_bitMask[]    = {
    0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,	/* 0 1 2 3 4 5 6 7 */
};

#ifdef	WIN32
#include	<io.h>
#include	<fcntl.h>
#include	<sys/stat.h>

/********************************************************************
 *
 * version of mkstemp() for Windows
 *
 *******************************************************************/

int
mkstemp(char * templ)
{
    /* must use _S_IWRITE, otherwise newly created file will be read only when closed */
    return open(mktemp(templ), _O_CREAT | _O_RDWR | _O_TRUNC, _S_IWRITE);
}
#endif	/* WIN32 */

/********************************************************************
 *
 * version of malloc() which checks return value and works under Windows
 *
 *******************************************************************/

void *
iC_emalloc(unsigned	nbytes)		/* check return from malloc */
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
    memset(bp, 0, nbytes);		/* when free() is used memory can be non zero */
    return bp;
} /* iC_emalloc */
#ifdef _WINDOWS

/********************************************************************
 *
 * if free under windows gives trouble, make this a null function
 *
 *******************************************************************/

void
iC_efree(void *	p)
{
#if defined(_LARGE_) || defined(_HUGE_)
    GLOBALHANDLE		hglobal;

    hglobal = GlobalHandle(p);		/* retrieve the handle */
    GlobalUnlock(hglobal);
    GlobalFree(hglobal);		/* big deal */
#else
    LOCALHANDLE		hlocal;

    hlocal = LocalHandle(p);		/* retrieve the handle */
    LocalUnlock(hlocal);
    LocalFree(hlocal);			/* big deal */
#endif
} /* iC_efree */
#endif
#if defined(RUN) || defined (TCP) || defined(LOAD)

/********************************************************************
 *
 *	Compare gt_ids in two Gates support of qsort()
 *
 *******************************************************************/

int
iC_cmp_gt_ids( const Gate ** a, const Gate ** b)
{
    return( strcmp((*a)->gt_ids, (*b)->gt_ids) );
} /* iC_cmp_gt_ids */
#endif /* RUN) or TCP or LOAD */

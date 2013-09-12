static const char misc_c[] =
"@(#)$Id: misc.c,v 1.12 2013/08/07 05:28:04 jw Exp $";
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

#define		_GNU_SOURCE
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<assert.h>
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
 *	change the collating position of '_' before digits ('.' '/')
 *	change all '_' to '/' to get correct ordering (/ never occurs in C variable)
 *	use strverscmp(), which puts sequences of digits in number order.
 *
 *******************************************************************/

#define		ABSIZE	256
static char *	aBuf = NULL;
static int	aSiz = 0;	/* dynamic size adjusted with realloc */
static char *	bBuf = NULL;
static int	bSiz = 0;	/* dynamic size adjusted with realloc */

int
iC_cmp_gt_ids( const Gate ** a, const Gate ** b)
{
    char *	ap;
    char *	bp;
    char *	cp;

    if ((cp = strchr(ap = (*a)->gt_ids, '_')) != 0) {
	while (strlen(ap) >= aSiz) {
	    aBuf = (char *)realloc(aBuf, (aSiz + ABSIZE) * sizeof(char));
	    assert(aBuf);
	    memset(&aBuf[aSiz], '\0', ABSIZE * sizeof(char));
	    aSiz += ABSIZE;
	}
	cp += aBuf - ap;
	ap = strncpy(aBuf, ap, aSiz);
	do {
	    *cp++ = '/';		/* change all '_' to '/' */
	} while ((cp = strchr(cp, '_')) != 0);
    }
    if ((cp = strchr(bp = (*b)->gt_ids, '_')) != 0) {
	while (strlen(bp) >= bSiz) {
	    bBuf = (char *)realloc(bBuf, (bSiz + ABSIZE) * sizeof(char));
	    assert(bBuf);
	    memset(&bBuf[bSiz], '\0', ABSIZE * sizeof(char));
	    bSiz += ABSIZE;
	}
	cp += bBuf - bp;
	bp = strncpy(bBuf, bp, bSiz);
	do {
	    *cp++ = '/';		/* change all '_' to '/' */
	} while ((cp = strchr(cp, '_')) != 0);
    }
    return( strverscmp(ap, bp) );
} /* iC_cmp_gt_ids */
#endif /* RUN) or TCP or LOAD */

static const char misc_c[] =
"@(#)$Id: misc.c,v 1.3 2005/01/26 15:18:10 jw Exp $";
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
 *	misc.c
 *	miscellanious functions shared by all modules
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"icc.h"

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
#if defined(RUN) || ((defined (LOAD) || defined(TCP)) && defined (YYDEBUG))

/********************************************************************
 *
 *	Display inputs & outputs
 *
 *******************************************************************/

void
iC_display(int * dis_cntp, int dis_max)
{
    int			n;
    Gate *		gp;
    unsigned char	x0;
    char		b1;
    short		w2;
#if	INT_MAX != 32767 || defined (LONG16)
    long		l4;
#endif	/* INT_MAX != 32767 || defined (LONG16) */

#ifndef	LOAD
#define iC_IX0p	iC_IX_[0]
#define iC_IB1p	iC_IB_[1]
#define iC_IW2p	iC_IW_[2]
#if	INT_MAX != 32767 || defined (LONG16)
#define iC_IL4p	iC_IL_[4]
#endif	/* INT_MAX != 32767 || defined (LONG16) */
#endif	/* LOAD */

    if ((*dis_cntp)++ >= dis_max) {	/* display header line */
	*dis_cntp = 1;
#ifdef	LOAD
	if (iC_IX0p != 0) {
	    for (n = 0; n < MAX_IO; n++) {
		fprintf(iC_outFP, " I%d", n);
	    }
	}
#else	/* LOAD */
	for (n = 0; n < MAX_IO; n++) {
	    if ((gp = iC_IX_[n]) != 0) fprintf(iC_outFP, " I%d", n);
	}
#endif	/* LOAD */
	if (iC_IB1p != 0) fprintf(iC_outFP, "  IB1");
	if (iC_IW2p != 0) fprintf(iC_outFP, "    IW2");
#if	INT_MAX != 32767 || defined (LONG16)
	if (iC_IL4p != 0) fprintf(iC_outFP, "      IL4");
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	fprintf(iC_outFP, "   ");
	for (n = 0; n < MAX_IO; n++) {
	    fprintf(iC_outFP, " Q%d", n);
	}
	fprintf(iC_outFP, "  QB1    QW2");
#if	INT_MAX != 32767 || defined (LONG16)
	fprintf(iC_outFP, "      QL4");
#endif	/* INT_MAX == 32767 && defined (LONG16) */
	fprintf(iC_outFP, "\n");
    }
    /* display IX0 .. IX7 - any that are active */
#ifdef	LOAD
    if ((gp = iC_IX0p) != 0) {
	x0 = gp->gt_new;
	for (n = 0; n < MAX_IO; n++) {
	    fprintf(iC_outFP, "  %c", (x0 & 0x0001) ? '1' : '0');
	    x0 >>= 1;			/* scan input bits */
	}
    }
#else	/* LOAD */
    for (n = 0; n < MAX_IO; n++) {
	if ((gp = iC_IX_[n]) != 0) {
	    fprintf(iC_outFP, "  %c", gp->gt_val < 0 ? '1' : '0');
	}
    }
#endif	/* LOAD */
    /* display IB1, IW2 and IL4 if active */
    if (!iC_xflag) {
	if ((gp = iC_IB1p) != 0) fprintf(iC_outFP, " %4d", gp->gt_new & 0xff);
	if ((gp = iC_IW2p) != 0) fprintf(iC_outFP, " %6hd", (short)gp->gt_new);
#if	INT_MAX == 32767
#if	defined (LONG16)
	if ((gp = iC_IL4p) != 0) fprintf(iC_outFP, " %8ld", gp->gt_new);
#endif	/* defined (LONG16) */
#else	/* INT_MAX == 32767 */
	if ((gp = iC_IL4p) != 0) fprintf(iC_outFP, " %8d", gp->gt_new);
#endif	/* INT_MAX == 32767 */
    } else {
	if ((gp = iC_IB1p) != 0) fprintf(iC_outFP, "   %02x", gp->gt_new & 0xff);
	if ((gp = iC_IW2p) != 0) fprintf(iC_outFP, "   %04x", gp->gt_new & 0xffff);
#if	INT_MAX == 32767
#if	defined (LONG16)
	if ((gp = iC_IL4p) != 0) fprintf(iC_outFP, " %08lx", gp->gt_new);
#endif	/* defined (LONG16) */
#else	/* INT_MAX == 32767 */
	if ((gp = iC_IL4p) != 0) fprintf(iC_outFP, " %08x", gp->gt_new);
#endif	/* INT_MAX == 32767 */
    }
    fprintf(iC_outFP, "   ");

#ifdef	LOAD
    x0 = iC_QX0p ? iC_QX0p->gt_new : 0;
    b1 = iC_QB1p ? iC_QB1p->gt_new : 0;
    w2 = iC_QW2p ? iC_QW2p->gt_new : 0;
#if	INT_MAX != 32767 || defined (LONG16)
    l4 = iC_QL4p ? iC_QL4p->gt_new : 0;
#endif	/* INT_MAX != 32767 || defined (LONG16) */
#else	/* LOAD */
    x0 = iC_QX_[0];
    b1 = iC_QX_[1];
    w2 = *(short*)&iC_QX_[2];
#if	INT_MAX == 32767
    l4 = *(long*)&iC_QX_[4];
#else	/* INT_MAX == 32767 */
    l4 = *(int*)&iC_QX_[4];
#endif	/* INT_MAX == 32767 */
#endif	/* LOAD */
    /* display QX0 .. QX7 */
    for (n = 0; n < MAX_IO; n++) {
	fprintf(iC_outFP, "  %c", (x0 & 0x0001) ? '1' : '0');
	x0 >>= 1;			/* scan output bits */
    }
    /* display QB1, QW2 and QL4 */
    if (!iC_xflag) {
#if	INT_MAX != 32767 || defined (LONG16)
	fprintf(iC_outFP, " %4d %6hd %8ld\n", (int)b1, w2, l4);
#else	/* INT_MAX != 32767 || defined (LONG16) */
	fprintf(iC_outFP, " %4d %6hd\n", (int)b1, w2);	/* no QL4 */
#endif	/* INT_MAX != 32767 || defined (LONG16) */
    } else {
#if	INT_MAX != 32767 || defined (LONG16)
	fprintf(iC_outFP, "   %02x   %04hx %08lx\n", (int)b1, w2, l4);
#else	/* INT_MAX != 32767 || defined (LONG16) */
	fprintf(iC_outFP, "   %02x   %04hx\n", (int)b1, w2);	/* no QL4 */
#endif	/* INT_MAX != 32767 || defined (LONG16) */
    }
    fflush(iC_outFP);
} /* iC_display */
#endif	/* RUN or ((LOAD orTCP) and YYDEBUG) */

static const char symb_c[] =
"@(#)$Id: symb.c,v 1.4 2000/11/27 11:29:23 jw Exp $";
/*
 *	"symb.c"
 */

#ifdef _WINDOWS
#include	<windows.h>
#endif
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<assert.h>
#include	"pplc.h"
#include	"comp.h"
#include	"y.tab.h"

/* "symb.c	3.09	95/01/13" */

Symbol		*symlist[HASHSIZ];	/* symbol table: linked lists */

typedef struct hts {
    char first;
    char last;
    char offset;
} hts;

hts	hta[] =	{
#ifdef _WINDOWS
    { '0', '9', '0'-2,  },
    { 'A', 'Z', 'A'-12, },
    { '_', '_', '_'-1, },
    { 'a', 'z', 'a'-38, },
#else
    { '0', '9', '0'-1,  },
    { 'A', 'Z', 'A'-11, },
    { '_', '_', '_'-37, },
    { 'a', 'z', 'a'-38, },
#endif
};

static int
hash(register char	*string)	/* hash a string to table index */
{
    register char	ch;
    unsigned	hsh;
    unsigned	correct;
    unsigned	i;
    hts *	htp;

    hsh = 0;
    correct = 10;		/* range of 1st character 64 - 10 */
    for (i = 0; i < 2; i++) {
	ch = *string++;
	hsh <<= 6;		/* multiply by 64, range of 2nd char */
	for (htp = &hta[1-i]; htp < &hta[sizeof hta/sizeof(hts)]; htp++) {
	    if (ch < htp->first) {
		break;		/* not a proper character ==> 0 */
	    }
	    if (ch <= htp->last) {
		ch -= (htp->offset + correct);
		if (ch <= 0) {
		    ch = 1;	/* _ under _WINDOWS */
		}
		hsh += ch;
		break;		/* proper character */
	    }
	}
	correct = 0;		/* range of 2nd character 64 */
    }
    return (hsh >> 2);		/* divide by 4, range 54 * 64 / 4 */
}

void *
emalloc(register unsigned	nbytes)	/* check return from malloc */
{
    register char	*bp;
    register char	*tp;
#ifdef _WINDOWS
#if defined(_LARGE_) || defined(_HUGE_)
    GLOBALHANDLE		hglobal;

    if ((hglobal = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, nbytes)) == 0) {
	execerror("out of memory", (char *) 0);	/* ZZZ */
    }
    bp = tp = GlobalLock(hglobal);		/* actual pointer */
#else
    LOCALHANDLE		hlocal;

    if ((hlocal = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, nbytes)) == 0) {
	execerror("out of memory", (char *) 0);	/* ZZZ */
    }
    bp = tp = LocalLock(hlocal);		/* actual pointer */
#endif
#else

    if ((bp = tp = (char *)malloc(nbytes)) == 0) {
	execerror("out of memory", (char *) 0);
    }
#endif
    while (nbytes--) {
	*tp++ = 0;	/* when free() is used memory can be non zero */
    }
    return (bp);
}
#ifdef _WINDOWS

/********************************************************************
 *
 * if free under windows gives trouble, make this a null function
 *
 *******************************************************************/

void
efree(void *p)
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

static Symbol **	spl;

Symbol *
lookup(register char	*string)	/* find string in symbol table */
{
    register Symbol	*sp;
    int			i = 1;		/* must be != 0 in case empty */

    spl = &symlist[hash(string)];
    for (sp = *spl; sp != 0; spl = &sp->next, sp = *spl) {
	if ((i = strcmp(string, sp->name)) <= 0) {
	    break;	/* position where a new Symbol should go */
	}
    }
    if (i == 0) {
	return (sp);	/* found */
    }
    return (0);		/* 0 ==> not found */
}

Symbol *
place_sym(register Symbol	*sp)	/* place sp in symbol table */
{
    register Symbol *	tsp;

    if (spl == 0 || (tsp = *spl) != 0 && strcmp(tsp->name, sp->name) != 0) {
	if (lookup(sp->name) != 0) {	/* locate sorted position */
	    execerror("trying to place existing symbol:", sp->name);
	}
    }
    sp->next = *spl;	/* point from this to next Symbol */
    *spl = sp;		/* point from previous to this Symbol */
    spl = 0;		/* force execution of lookup() for next call */
    return (sp);
}

Symbol *
install(				/* install string in symbol table */
    register char	*string,
    uchar		typ,
    uchar		ftyp)
{
    register Symbol	*sp;

    sp = (Symbol *) emalloc(sizeof(Symbol));
    sp->name = emalloc(strlen(string)+1);	/* +1 for '\0' */
    strcpy(sp->name, string);
    sp->type = typ;
    sp->ftype = ftyp;
    return (place_sym(sp));
}

Symbol *
unlink_sym(register Symbol	*sp)	/* unlink Symbol from symbol table */
{
    register Symbol *	tsp;

    if (sp) {
	if ((tsp = lookup(sp->name)) != 0) {	/* locate sorted position */
	    assert(tsp == sp);
	    *spl = sp->next;		/* skip this Symbol, point previous to next */
	    spl = 0;			/* force execution of lookup() for next call */
	} else {
	    error("trying to delete a Symbol not yet installed:", sp->name);
	}
    }
    return (sp);	/* Symbol has not been deleted from heap yet */
}

static const char symb_c[] =
"@(#)$Id: symb.c,v 1.22 2014/10/06 07:03:24 jw Exp $";
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
 *	symb.c
 *	parallel plc - symbol table lookup and insert
 *
 *******************************************************************/

#ifdef _WINDOWS
#include	<windows.h>
#endif
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<assert.h>
#include	"icc.h"
#include	"comp.h"
#include	"comp_tab.h"

/* "symb.c	3.09	95/01/13" */

Symbol *	symlist[HASHSIZ];		/* symbol table: linked lists */

typedef struct hts {
    char first;
    char last;
    char offset;
} hts;

static hts	hta[] =	{
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
hash(char *	string)				/* hash a string to table index */
{
    char	ch;
    unsigned	hsh;
    unsigned	correct;
    unsigned	i;
    hts *	htp;

    hsh = 0;
    correct = 10;				/* range of 1st character 64 - 10 */
    for (i = 0; i < 2; i++) {
	ch = *string++;
	hsh <<= 6;				/* multiply by 64, range of 2nd char */
	for (htp = &hta[1-i]; htp < &hta[sizeof hta/sizeof(hts)]; htp++) {
	    if (ch < htp->first) {
		break;				/* not a proper character ==> 0 */
	    }
	    if (ch <= htp->last) {
		ch -= (htp->offset + correct);
		if (ch <= 0) {
		    ch = 1;			/* _ under _WINDOWS */
		}
		hsh += ch;
		break;				/* proper character */
	    }
	}
	correct = 0;				/* range of 2nd character 64 */
    }
    return (hsh >> 2);				/* divide by 4, range 54 * 64 / 4 */
} /* hash */

static Symbol **	spl;

Symbol *
lookup(char *	string)				/* find string in symbol table */
{
    Symbol *	sp;
    int		i = 1;				/* must be != 0 in case empty */

    spl = &symlist[hash(string)];
    for (sp = *spl; sp != 0; spl = &sp->next, sp = *spl) {
	if ((i = strcmp(string, sp->name)) <= 0) {
	    break;				/* position where a new Symbol should go */
	}
    }
    if (i == 0) {
	return (sp);				/* found */
    }
    return (0);					/* 0 ==> not found */
} /* lookup */

Symbol *
install(					/* install string in symbol table */
    char *		string,
    unsigned char	typ,
    unsigned char	ftyp)
{
    Symbol *		sp;

    sp = (Symbol *) iC_emalloc(sizeof(Symbol));
    sp->name = iC_emalloc(strlen(string)+1);	/* +1 for '\0' */
    strcpy(sp->name, string);
    sp->type = typ;
    sp->ftype = ftyp;
    return (link_sym(sp));
} /* install */

Symbol *
link_sym(Symbol *	sp)			/* link Symbol into symbol table */
{
    Symbol *	tsp;

    if (spl == 0 || ((tsp = *spl) != 0 && strcmp(tsp->name, sp->name) != 0)) {
	if (lookup(sp->name) != 0) {		/* locate sorted position */
	    ierror("trying to place existing symbol:", sp->name);
	}
    }
    sp->next = *spl;				/* point from this to next Symbol */
    *spl = sp;					/* point from previous to this Symbol */
    spl = 0;					/* force execution of lookup() for next call */
    return (sp);
} /* link_sym */

Symbol *
unlink_sym(Symbol *	sp)			/* unlink Symbol from symbol table - keep name and attributes */
{
    Symbol *	tsp;

    if (sp) {
	if ((tsp = lookup(sp->name)) != 0) {	/* locate sorted position */
	    assert(tsp == sp);
	    *spl = sp->next;			/* skip this Symbol, point previous to next */
	    spl = 0;				/* force execution of lookup() for next call */
	} else {
	    ierror("trying to delete a Symbol not yet installed:", sp->name);
	}
	sp->next = 0;				/* flags that Symbol is not linked to S.T. */
    }
    return (sp);				/* Symbol and its name has not been deleted from heap yet */
} /* unlink_sym */

void
uninstall(Symbol *	sp)			/* completely remove the Symbol */
{
    unlink_sym(sp);				/* unlink Symbol from symbol table */
    if (sp->name) {
	free(sp->name);				/* free name space */
    }
    free(sp);					/* delete Symbol and free Symbol space */
} /* uninstall */

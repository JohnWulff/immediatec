static const char genr_c[] =
"@(#)$Id: genr.c,v 1.56 2004/02/02 07:56:27 jw Exp $";
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
 *	genr.c
 *	generator functions for icc compiler
 *
 *******************************************************************/

#ifdef _WINDOWS
#include	<windows.h>
#endif
#include	<stdio.h>
#ifndef _WINDOWS 
#include	<stdlib.h>
#endif
#include	<string.h>
#include	<assert.h>
#include	<ctype.h>
#include	"icg.h"
#include	"icc.h"
#include	"comp.h"
#include	"comp_tab.h"

#define v(lp)	(lp->le_val) ? '~' : ' ', lp->le_sym->name
#define w(lp)	(lp->le_val) ? '~' : ' '
#define EBSIZE	2048

static Symbol *	templist;		/* temp list of un-named symbols */
static short	ttn;			/* for generating temp f object name */
#if YYDEBUG
static short	tn;
#endif
char		eBuf[EBSIZE];		/* temporary expression text buffer */
static Symbol *	iCallHead = 0;		/* function head seen at start of imm call */

/********************************************************************
 *
 *	initialize for code generation
 *
 *******************************************************************/

void
initcode(void)			/* initialize for code generation */
{
    templist = 0;
#if YYDEBUG
    tn = 0;
    if ((debug & 0402) == 0402) {
	fprintf(outFP, "initcode:\n");
	fflush(outFP);
    }
#endif
} /* init_code */

/********************************************************************
 *
 *	create List element for variable
 *	allow pushing a NULL var* for formal parameter links
 *
 *******************************************************************/

List_e *
sy_push(Symbol * var)	/* create List element for variable */
{
    List_e *	lp;

    lp = (List_e *) emalloc(sizeof(List_e));
    lp->le_sym = var;	/* point to variables Symbol entry */
#if YYDEBUG
    if ((debug & 0402) == 0402) {
	fprintf(outFP, "sy_push:++%s\n", var ? var->name : "(null)");
	fflush(outFP);
    }
#endif
    return (lp);	/* return pointer to List_e to yacc */
} /* sy_push */

/********************************************************************
 *
 *	delete List element left over
 *	allow popping a NULL var* (just in case)
 *
 *******************************************************************/

Symbol *
sy_pop(List_e * lp)	/* delete List element left over */
{
    Symbol *	sp;

    sp = lp->le_sym;	/* point to variables Symbol entry */
    free(lp);
#if YYDEBUG
    if ((debug & 0402) == 0402) {
	fprintf(outFP, "sy_pop:   %s--\n", sp ? sp->name : "(null)");
	fflush(outFP);
    }
#endif
    return (sp);	/* return pointer to Symbol to yacc */
} /* sy_pop */

/********************************************************************
 *
 *	force linked Symbol to correct ftype
 *
 *******************************************************************/

List_e *
op_force(		/* force linked Symbol to correct ftype */
    List_e *		lp,
    unsigned char	ftyp)
{
    Symbol *		sp;
    List_e *		lp1;
    int			typ;

    if (lp && (sp = lp->le_sym)->ftype != ftyp) {
	if (sp->u_blist == 0 ||			/* not a $ symbol or */
	    (sp->type & TM) >= MAX_GT ||	/* SH, FF, EF, VF, SW, CF or */
	    (sp->u_blist->le_sym == sp && sp->type == LATCH)) { /* L(r,s) */
	    if ((typ = types[sp->ftype]) == ERR) {
		ierror("cannot force from", full_ftype[sp->ftype]);
	    }
	    lp1 = op_push(0, typ, lp);
	    lp1->le_first = lp->le_first;
	    lp1->le_last = lp->le_last;
	    lp = lp1;	/* create a new $ symbol linked to old */
	    sp = lp->le_sym;
	}
#if YYDEBUG
	if ((debug & 0402) == 0402) {
	    fprintf(outFP, "op_force: %s from %s to %s\n",
		sp->name, full_ftype[sp->ftype], full_ftype[ftyp]);
	    fflush(outFP);
	}
#endif
	sp->ftype = ftyp;	/* convert old or new to ftyp */
    }
    return (lp);	/* return 0 or link to old or new Symbol */
} /* op_force */

/********************************************************************
 *
 *	reduce List_e stack to links
 *
 *******************************************************************/

List_e *
op_push(			/* reduce List_e stack to links */
    List_e *		left,
    unsigned char	op,
    List_e *		right)
{
    List_e *		rlp;
    Symbol *		sp;	/* current temporary Symbol */
    Symbol *		lsp;
    Symbol *		tsp;
    List_e *		lp;
    int			typ;
#if YYDEBUG
    char 		temp[TSIZE];
#endif

    if (right == 0) {
	if ((right = left) == 0) return 0;	/* nothing to push */
	left = 0;				/* swapped left to right */
    }
    rlp = right;
    sp = rlp->le_sym;
    typ = sp->type & TM;
    if (left && op > OR && op < MAX_LV && op != typ) {
	warning("function types incompatible", NS);
    }
    if (sp->u_blist == 0 || op != typ) {
	/* right not a $ symbol or new operator - force new level */
	sp = (Symbol *) emalloc(sizeof(Symbol));
	sp->name = NS;		/* no name at present */
#if YYDEBUG
	if ((debug & 0402) == 0402) {	/* DEBUG name */
	    snprintf(temp, TSIZE, "$%d", ++tn);
	    sp->name = emalloc(strlen(temp)+1);	/* +1 for '\0' */
	    strcpy(sp->name, temp);
	}
#endif
	sp->type = op != UDF ? op : AND; /* operator OR or AND (default) */
	sp->ftype = rlp->le_sym->ftype;	 /* used in op_xor() with op UDF */
	sp->next = templist;	/* put at front of templist */
	templist = sp;
	rlp->le_next = 0;	/* sp->u_blist is 0 for new sp */
	sp->u_blist = rlp;	/* link right of expression */
	rlp = sy_push(sp);	/* push new list element on stack */
    }
    if (left) {
	lsp = left->le_sym;	/* test works correctly with ftype - handles ALIAS */
	if (lsp->ftype >= MIN_ACT && lsp->ftype < MAX_ACT) {
	    if (sp->ftype < S_FF) {
		sp->ftype = 0;	/* OK for any value of GATE */
	    }
	    sp->ftype |= lsp->ftype;	/* modify S_FF ==> D_FF */
	}
	if ((typ = lsp->type & TM) < MAX_LS) {
	    if ((lp = lsp->u_blist) == 0 ||	/* left not a $ symbol */
		sp == lsp ||			/* or right == left */
		(typ != op &&			/* or new operator */
			/* ZZZ watch this when typ is ALIAS or UDF */
		typ != TIM) ||			/* but left is not a timer */
		(typ == op &&			/* or old operator */
		right->le_val == (unsigned)-1))	/* and right is a delay for timer */
	    {
		left->le_next = sp->u_blist;	/* extend expression */
		sp->u_blist = left;		/* link left of expr */
#if YYDEBUG
		if ((debug & 0402) == 0402) {
		    fprintf(outFP, "op_push:  %c%s %c %c%s\n",
			v(left), os[op], v(right));
		    fflush(outFP);
		}
#endif
	    } else {	/* left is a $ symbol and ... - merge left into right */
		while (lp->le_next) {
		    lp = lp->le_next;		/* scan to end of left list */
		}
		lp->le_next = sp->u_blist;	/* move connect list */
		sp->u_blist = lsp->u_blist;	/* in the right order */
		if (templist != lsp) {		/* bypass lsp on templist and then delete left Link and Symbol */
		    tsp = templist;		/* scan templist */
		    while (tsp->next != lsp) {
			tsp = tsp->next;	/* find lsp in templist */
			if (tsp == 0) {
			    execerror("left temp not found ???\n", NS, __FILE__, __LINE__);
			}
		    }
		    tsp->next = lsp->next;	/* unlink lsp from templist */
		} else {
		    templist = lsp->next;	/* unlink first object */
		}
#if YYDEBUG
		if ((debug & 0402) == 0402) {
		    fprintf(outFP, "\t%c%s %c %c%s\n",
			v(left), os[op], v(right));
		    fflush(outFP);
		}
		sy_pop(left);			/* left Link_e */
		if ((debug & 0402) == 0402) {
		    free(lsp->name);		/* free name space */
		}
		free(lsp);			/* left Symbol */
#else
		free(sy_pop(left));		/* left Link and Symbol */
#endif
	    }
	} else {				/* discard left BLT, CLT */
	    sy_pop(left);			/* Link_e only */
	}
#if YYDEBUG
    } else if ((debug & 0402) == 0402) {		/* fexpr : sexpr { left is 0 } */
	fprintf(outFP, "\t(0) %c %c%s\n", os[op], v(right));
	fflush(outFP);
#endif
    }
    return (rlp);
} /* op_push */

/********************************************************************
 *
 *	numeric constant push
 *
 *******************************************************************/

int
const_push(Lis * expr)
{
    char *	cp = expr->f;
    char *	ep = expr->l;
    int		bc = TSIZE;
    char	buf[TSIZE];
    char *	bp = buf;
    Symbol *	symp;
    char *	endptr;
    long	value;

    while (cp < ep) {
	if (--bc == 0 || !isprint(*bp++ = *cp++)) {
	    return 1;			/* error - too big or not printable */
	}
    }
    *bp = 0;				/* terminate - there is room in buf */
    value = strtol(buf, &endptr, 0);	/* convert to check for error */
    assert(value >= 0);			/* must be unsigned positive */
#if YYDEBUG
    if ((debug & 0402) == 0402) {
	fprintf(outFP, "const_push: '%s' converted to %ld\n", buf, value);
	fflush(outFP);
    }
#endif
    if (*endptr != '\0') {
	return 1;			/* error */
    }
    snprintf(buf, TSIZE, "%ld", value);
    if ((symp = lookup(buf)) == 0) {	/* install name of decimal conversion */
	symp = install(buf, NCONST, ARITH); /* becomes NVAR */
    }
    expr->v = sy_push(symp);
    return 0;				/* correct - no error */
} /* const_push */

/********************************************************************
 *
 *	special exclusive or push
 *
 *******************************************************************/

List_e *
op_xor(				/* special exclusive or push */
    List_e *	left,
    List_e *	right)
{
    List_e *	inv_left;
    List_e *	inv_right;
    List_e*	lp1;
    List_e*	lp2;

#if YYDEBUG
    if ((debug & 0402) == 0402) {
	fprintf(outFP, "op_xor:   %c%s ^ %c%s\n", v(left), v(right));
	fflush(outFP);
    }
#endif
    inv_left = sy_push(left->le_sym);	/* duplicate arg list entries */
    inv_right = sy_push(right->le_sym);
    inv_left->le_val = left->le_val ^ NOT;	/* invert */
    inv_right->le_val = right->le_val ^ NOT;	/* invert */
    /*
     * left ^ right === (left & ~right) | (~left & right)
     * the op "UDF" forces a new level in op_push
     * it is then changed to AND 
     * thus a new level is forced even if left or right is AND
     */
    lp2 = op_push(inv_left, UDF, right);	/* ~left & right */
    lp1 = op_push(left, UDF, inv_right);	/* left & ~right */
    lp2->le_first = lp1->le_first = inv_left->le_first = left->le_first;
    inv_left->le_last = left->le_last;
    inv_right->le_first = right->le_first;
    lp2->le_last = lp1->le_last = inv_right->le_last = right->le_last;
    return op_push(lp1, OR, lp2);		/* left ^ right */
} /* op_xor */
 
/********************************************************************
 *
 *	logical negation
 *
 *******************************************************************/

List_e *
op_not(List_e * right)		/* logical negation */
{
    Symbol *	sp = right->le_sym;
    List_e *	lp = sp->u_blist;

#if YYDEBUG
    if ((debug & 0402) == 0402) {
	fprintf(outFP, "op_not:   ~%s\n", sp->name);
	fflush(outFP);
    }
#endif
    if (lp == 0) {
	right->le_val ^= NOT;		/* negate logical value */
    } else {
	switch (sp->type) {			/* $ symbol */
	case AND:
	case OR:
	case EXT_AND:
	case EXT_OR:
	    sp->type ^= (AND ^ OR);		/* de Morgans rule */
	case LATCH:
	    while (lp) {
		if (lp->le_sym != sp) {		/* ignore latch feedback */
		    lp->le_val ^= NOT;
		}
		lp = lp->le_next;
	    }
	    break;
	case ARNC:
	case ARN:
	case LOGC:
	case SH:
	case FF:
	case VF:
	case EF:
	case SW:
	case CF:
	case NCONST:			/* impossible to generate ? */
	case EXT_ARN:
	    right->le_val ^= NOT;	/* negate logical value */
	    				/* forces creation of alias */
	    break;			/* if assigned immediately */
	case INPW:
	case INPX:
	    execerror("INPUT has other inputs in op_not() ???", sp->name, __FILE__, __LINE__);
	    break;
	default:
	    execerror("negation of non-logical value attempted", sp->name, __FILE__, __LINE__);
	    break;
	}
    }
    return (right);
} /* op_not */

/********************************************************************
 *
 *	asign List_e stack to links
 *
 *	Sym sv contains Symbol *v and char *f and *l to source
 *	Lis lr contains List_e *v and char *f and *l to source
 *	unsigned char ft is the ftype which right must be forced to
 *
 *******************************************************************/

Symbol *
op_asgn(				/* asign List_e stack to links */
    Sym *		sv,		/* may be 0 for ffexpr */
    Lis *		rl,
    unsigned char	ft)
{
    Symbol *	var;
    Symbol *	sp;
    List_e *	lp;
    Symbol *	gp;
    Symbol *	rsp;
    List_e *	tlp;
    List_e *	right;
    short	atn;
    short	sflag;
    Symbol *	sr;
    char *	t_last = 0;
    char *	t_first = 0;
    int		typ;
    char	temp[TSIZE];

    varList = 0;			/* start of a new function expression */
    right = op_force(rl->v, ft);	/* force Symbol on right to ftype ft */
    if (sv == 0) {
	/* null var - generate a temporary Symbol of type UNDEF */
	var = (Symbol *) emalloc(sizeof(Symbol));
	snprintf(temp, TSIZE, "_f%d", ++ttn);
	var->name = emalloc(strlen(temp)+1);	/* +1 for '\0' */
	strcpy(var->name, temp);	/* name needed for derived Sy's */
	sflag = 0;			/* don't output name */
    } else {
	var = sv->v;			/* Symbol * var */
	sflag = 1;			/* print output name */
    }
#if YYDEBUG
    if ((debug & 0402) == 0402) {
	fprintf(outFP, "op_asgn:  %s = %c%s\n", var->name, v(right));
	fflush(outFP);
    }
#endif
    assert(right);			/* must have something to assign */
    rsp = right->le_sym;
    if ((typ = var->type & TM) >= AND && typ != (rsp->type & TM)) {
	if (typ != ERR) {
	    ierror("type mismatch in multiple assignment:", var->name);
	    var->type = ERR;		/* reduce anyway to clear list */
	}
    } else {
	var->type = rsp->type & TM;
	if (var->ftype < MIN_ACT) {
	    var->ftype = rsp->ftype;
	}
    }
    if (rsp->type == NCONST || rsp->u_blist == 0) {		/* right must be a $ symbol */
	if (var->type != ERR) {
	    var->type = ALIAS;		/* alias found */
	}
	/***********************************************************
	 *  Change right to point to source of ALIAS, adjusting
	 *  negation correctly. An ALIAS must have a gate it points
	 *  to via list.
	 ***********************************************************/
	while (rsp->type == ALIAS) {	/* scan down list of aliases */
	    if (var == rsp) {
		var->type = ERR;	/* error found */
		ierror("circular list of aliases:", var->name);
		break;			/* circular list of aliases */
	    }
	    right->le_val ^= rsp->list->le_val;
	    if (!(rsp = rsp->list->le_sym)) {
		execerror("ALIAS points to nothing ???",
		right->le_sym->name, __FILE__, __LINE__);
	    }
	    right->le_sym = rsp;
	}
	/***********************************************************
	 *  If there are gates driven by var, which is an ALIAS,
	 *  the output list must be scanned and negations carried out.
	 *  Link rsp's output list to end of scanned list and
	 *  link var's list to rsp and point var to right->rsp.
	 *  This means that future use of the ALIAS will be resolved.
	 ***********************************************************/
	if ((lp = tlp = var->list) != 0) {	/* start of left list */
	    while (lp->le_next) {
		lp->le_val ^= right->le_val;
		lp = lp->le_next;	/* scan to end of left list */
	    }
	    lp->le_val ^= right->le_val;
	    lp->le_next = rsp->list;	/* link right to end of left */
	    rsp->list = tlp;		/* link full list to right */
	}
	var->list = right;		/* alias points to original */
	if (debug & 04) {
	    iFlag = 1;
	    fprintf(outFP, "\n\t%s\t%c ---%c\t%s\n\n", rsp->name,
		((typ = rsp->type & TM) >= MAX_LV) ? os[typ] : w(right),
		os[var->type & TM], var->name);
	}
	if (sv == 0) {
	    execerror("ALIAS points to temp ???", var->name, __FILE__, __LINE__);
	} else
if (iCallHead == 0)
	{
	    t_first = sv->f; t_last = sv->l;	/* full text of var */
	    while (t_first && t_first < t_last) {
		*t_first++ = '#';		/* mark left var, leave ALIAS */
	    }
	}
	return (var);			/* needs no reduction */
    }

    if (rsp != (sp = templist)) {
	if (sp == 0) goto FailTemplist;
	while (rsp != sp->next) {
	    if ((sp = sp->next) == 0) {			/* DEBUG */
	      FailTemplist:
		execerror("right->le_sym not found in templist ???",
		    right->le_sym->name, __FILE__, __LINE__);
	    }
	}
	sp->next = rsp->next;	/* link tail to head in front of rsp */
	rsp->next = templist;	/* link head to rsp */
	templist = rsp;		/* now rsp is head of templist */
    }
    if (((typ = rsp->type & TM) == CLK || typ == TIM) && var->ftype != rsp->ftype) {
	warning("clock or timer assignment from wrong ftype:", var->name);
    }

    sr = rsp->u_blist->le_sym;		/* gate linked to var */
    if (right->le_val == (NOT^NOT)) {
	var->u_blist = rsp->u_blist;	/* move blist from rsp to var */
	templist = rsp->next;		/* bypass rsp */
    } else {
	/* make var an ALIAS because of FF negation */
	rsp->ftype = GATE;		/* may be odd value from gen */
	var->type = ALIAS;		/* make var negated ALIAS */
	var->u_blist = right;		/* link var to right */
    }

    atn = 0;
    sp = var;				/* start reduction with var */
    t_first = rl->f; t_last = rl->l;	/* full text of expression */
#if YYDEBUG
    if (
iCallHead == 0 &&
	(debug & 0402) == 0402) fprintf(outFP, "resolve \"%s\" to \"%s\"\n", t_first, t_last);
#endif
    do {				/* marked symbol */
	int	gt_input;
	char *	ep = eBuf;		/* temporary expression buffer */
	char *	en = &eBuf[EBSIZE];	/* end of temporary expression buffer */
	List_e*	saveBlist;
	if (debug & 04) {
	    fprintf(outFP, "\n");
	}
	gt_input = 0;			/* output scan for 1 gate */
	if (iFunSymExt) saveBlist = sp->u_blist;
	while ((lp = sp->u_blist) != 0) {
	    sp->u_blist = lp->le_next;
	    if ((gp = lp->le_sym) == rsp && var->type != ALIAS) {
		gp = var;		/* link points to right */
	    }
	    if (iFunSymExt == 0) {			/* global reduction only */
		while (gp->type == ALIAS) {		/* not for var ALIAS */
		    lp->le_val ^= gp->list->le_val;	/* negate if necessary */
		    gp = gp->list->le_sym;		/* point to original */
		}
		if (sp == gp && sp->ftype >= MIN_ACT) {
		    if (sp->type == LATCH && sp->ftype == D_FF) {
			gp = sp->list->le_sym;	/* feedback via D output */
		    } else if (gt_input) {
			warning("input equals output at function gate:", sp->name);
		    } else {
			/****************************************************
			 * no logical inputs yet: can split function and logic
			 *
			 *	O1 = CLOCK(L(I1, I2));
			 *			clock	: ---%	O1_1	C ---:	O1
			 *	O1_2	  ---%	O1_2	  ---|	O1_1
			 *	I1	  ---%	O1_2
			 *	I2	~ ---%	O1_2	<--- new gp
			 ***************************************************/
			gp = (Symbol *) emalloc(sizeof(Symbol));
			gp->name = NS;			/* no name at present */
    #if YYDEBUG
			if ((debug & 0402) == 0402) {	/* DEBUG name */
			    snprintf(temp, TSIZE, "$%d", ++tn);
			    gp->name = emalloc(strlen(temp)+1);
			    strcpy(gp->name, temp);
			}
    #endif
			gp->type = sp->type;
			gp->ftype = GATE;
			sp->type = OR;			/* OR default for 1 input */
			gp->next = templist;		/* put at front of list */
			templist = gp;
			gp->u_blist = tlp = sy_push(gp);/* link self */
			tlp->le_next = sp->u_blist;	/* rest of inputs on sp */
			sp->u_blist = 0;
		    }
		}
		if (sp->type == ALIAS) {
		    /* var was made an ALIAS because of FF negation */
		    gp->list = tlp = sp->list;		/* start of left list */
		    while (tlp) {
			tlp->le_val ^= lp->le_val;	/* negate if necesary */
			tlp = tlp->le_next;		/* scan to end of ALIAS list */
		    }
		    sp->list = lp;			/* link ALIAS to right */
		} else {
		    /* link Symbols to the end of gp->list to maintain order */
		    lp->le_next = 0;
		    if (gp->ftype == ARITH &&		/* && gp->u_blist ZZZ */
			sp->ftype != OUTW &&
			lp->le_val != (unsigned) -1) {
			lp->le_val = c_number + 1;	/* arith case # */
		    }
		    if ((tlp = gp->list) == 0) {
			gp->list = lp;			/* this is the first Symbol */
		    } else {
			/* loop to find duplicate link (possibly inverted) */
		      loop:				/* special loop with test in middle */
			if (tlp->le_sym == sp) {
			    if (gp->ftype == OUTW || gp->ftype == OUTX) {
				warning("input equals output at output gate:", gp->name);
			    } else if (tlp->le_val == lp->le_val) {
				sy_pop(lp);		/* ignore duplicate link */
				continue;		/* output scan for 1 gate */
			    } else if (gp->ftype != ARITH) {
				warning("gate has input and inverse:", gp->name);
			    }
			}
			if (tlp->le_next) {
			    tlp = tlp->le_next;		/* find last Symbol */
			    goto loop;
			}
			tlp->le_next = lp;		/* put new Symbol after last */
		    }
		    lp->le_sym = sp;			/* completely symmetrical */
		    if ((gp->type & TM) < MAX_LV) {
			gt_input++;			/* count the gate inputs */
		    }
		}
	    }						/* end of global reduction only */
	    if (! gp->name
#if YYDEBUG
		|| *(gp->name) == '$'
#endif
	    ) {						/* not marked symbol */
#if YYDEBUG
		if ((debug & 0402) == 0402) {
		    if (debug & 04) {
			fprintf(outFP, "%s =", gp->name);
		    } else {
			fprintf(outFP, "\t  %s cleared\n", gp->name);
		    }
		    free(gp->name);			/* free name space */
		}
#endif
		snprintf(temp, TSIZE, "%s_%d", var->name, ++atn);
		gp->name = emalloc(strlen(temp)+1);	/* +1 for '\0' */
		strcpy(gp->name, temp);			/* mark Symbol */
	    }
	    if (debug & 04) {
		if ((typ = gp->type & TM) >= MAX_LV) {
		    fprintf(outFP, "\t%s\t%c ---%c", gp->name, os[typ],
			os[sp->type & TM]);
		} else if (gp->ftype < MAX_AR && lp->le_val == (unsigned) -1) {
		    /* reference to a timer value - no link */
		    fprintf(outFP, "\t%s\t%c<---%c", gp->name, fos[gp->ftype],
			os[sp->type & TM]);
		} else if (gp->ftype != GATE) {
		    fprintf(outFP, "\t%s\t%c ---%c", gp->name, fos[gp->ftype],
			os[sp->type & TM]);
		} else {
		    if (sp->type == ALIAS) iFlag = 1;
		    fprintf(outFP, "\t%s\t%c ---%c", gp->name, w(lp),
			os[sp->type & TM]);
		}
		if (sflag) {
		    fprintf(outFP, "\t%s", sp->name);
		    if (sp->ftype != GATE) {
			fprintf(outFP, "\t%c", fos[sp->ftype]);
		    }
		}
		if (gp->ftype == F_SW || gp->ftype == F_CF || gp->ftype == F_CE ||
		    (gp->ftype == TIMR && lp->le_val > 0)) {
		    /*
		     * case number of "if" or "switch" C fragment
		     * or timer preset off value
		     */
		    fprintf(outFP, " (%d)", lp->le_val);
		}
	    }
	    if (gp->ftype == ARITH && (sp->type & TM) == ARN && lp->le_val != (unsigned) -1 &&
		(gp->u_blist || gp->type == NCONST)) {
		char	buffer[BUFS];			/* buffer for modified names */
		char	iqt[2];				/* char buffers - space for 0 terminator */
		char	xbwl[2];
		int	byte;
		int	bit;
		char	tail[8];			/* compiler generated suffix _123456 max */

		if (debug & 04) {
		    /* only logic gate or SH can be aux expression */
		    if (sflag) {
			if (sp->ftype == GATE) {
			    putc('\t', outFP);
			}
			putc('\t', outFP);
		    } else {
			fprintf(outFP, "\t\t\t");
		    }
		}
if (iCallHead == 0) {
		while (t_first && t_first < lp->le_first) {	/* end of arith */
		    if (*t_first != '#') {		/* transmogrified '=' */
			if (debug & 04) putc(*t_first, outFP);
			*ep++ = *t_first;
		    }
		    t_first++;
		}
		if (debug & 04) fprintf(outFP, "%s", gp->name);
		/* modify numbers, IXx.x and QXx.x names for compiled output only */
	    				/* CHECK if ep changes now _() is missing */
		IEC1131(gp->name, buffer, BUFS, iqt, xbwl, &byte, &bit, tail);
		ep += snprintf(ep, en - ep, "%s", buffer);
		t_first = lp->le_last;			/* skip logic expr's */
}
	    }
	    if (debug & 04) {
		fprintf(outFP, "\n");
		sflag = debug & 0200;
	    }
	    if (sp == gp && (sp->type != LATCH || lp->le_val != (NOT^NOT))) {
		warning("input equals output at gate:", sp->name);
	    }
	}						/* end output scan for 1 gate */
	if (iFunSymExt) sp->u_blist = saveBlist;
	if (
iCallHead == 0 &&
	    (sp->type & TM) == ARN) {
	    if (debug & 04) fprintf(outFP, "\t\t\t\t\t");
	    while (t_first && t_first < t_last) {
		if (*t_first != '#') {			/* transmogrified '=' */
		    if (debug & 04) putc(*t_first, outFP);
		    *ep++ = *t_first;
		}
		t_first++;
	    }
	    *ep++ = 0;
							/* start case or function */
	    if (sp->ftype != OUTW) {			/* output cexe function */
		fprintf(T1FP, cexeString[outFlag], ++c_number);
		fprintf(T1FP, "#line %d \"%s\"\n", lineno, inpNM);
		fprintf(T1FP, "	return %s;\n", eBuf);
		fprintf(T1FP, "%%##\n%s", outFlag ? "}\n\n" : "\n");
		if (debug & 04) fprintf(outFP, "; (%d)\n", c_number);
	    }
	}
	sflag = 1;					/* print output name */
	if (gt_input > PPGATESIZE) {
	    sp->type = ERR;				/* cannot execute properly */
	    ierror("too many inputs on gate:", sp->name);
	}
	if ((gp = sp = templist) != 0) {
	    if (sp->name
#if YYDEBUG
		&& *(sp->name) != '$'
#endif
	    ) {						/* marked symbol is first */
		templist = sp->next;			/* by_pass marked symbol */
	    } else {
		while ((sp = sp->next) != 0 && (!sp->name
#if YYDEBUG
		    || *(sp->name) == '$'
#endif
		)) {
		    gp = sp;				/* look for marked symbol */
		}
		if (sp) {
		    gp->next = sp->next;/* by_pass marked symbol */
		}
	    }
	    if (sp) {
		if (iFunSymExt == 0) {
		    (void) place_sym(sp);/* place sp in the symbol table (changes sp->next) */
		    if (
iCallHead == 0 &&
			(sp->type & TM) == ARN) {
			assert(sp->list);
			t_first = sp->list->le_first;
			t_last = sp->list->le_last;
			assert(t_first && t_last);
		    }
		} else {
		    sp->next = varList;	/* put newly marked Symbol on function varList */
		    varList = sp;	/* which is used to reconstitute templist when cloned */
		}
	    }
	}
    } while (sp);			/* 1 symbol resolved */

    /*
     * right Symbol must remain intact until after reduction, because
     * there may be a reference to it which is tested in if(gp == rsp).
     * When rsp is freed before reduction, rsp may point to a newly
     * generated Symbol, which is not a reference to right Symbol.
     * Therefore free here.
     */

    if (right->le_val == (NOT^NOT)) {
	sy_pop(right);			/* right Symbol and List_e */
#if YYDEBUG
	if ((debug & 0402) == 0402) {
	    free(rsp->name);		/* free name space of $x */
	}
#endif
	free(rsp);			/* free right Symbol */
    }
    if (debug & 04) {
	fprintf(outFP, "\n");
    }

    /*
     * A Symbol is marked by storing a pointer value in ->name
     * which points to a string which does not start with $.
     * Only marked symbols are reduced.
     * Any remaining new Symbols on 'templist' must belong to an outer
     * assignment which will be reduced later.
     */

if (iCallHead == 0) {
    t_first = rl->f; t_last = rl->l;	/* full text of expression */
    while (t_first && t_first < t_last) {
	*t_first++ = '#';		/* mark embedded assignments */
    }
}
    if (sv == 0) {
	lp = sr->list;			/* link action to temp */
	if (lp->le_sym != var || lp->le_next) {
	    execerror("error in unlinking temp:", var->name, __FILE__, __LINE__);
	}
	lp->le_sym = 0;			/* erase reference to temp */
#if YYDEBUG
	if ((debug & 0402) == 0402) {
	    fprintf(outFP, "\t  %s deleted\n\n", var->name);
	    fflush(outFP);
	}
#endif
	free(var->name);		/* free name space */
	free(var);			/* temporary Symbol */
	return 0;
    }
    if (iFunSymExt) {
	assert(var->list == 0);		/* function target is not linked into temp logic */
	var->list = var->u_blist;
	var->u_blist = 0;
    }
    return (sv->v);
} /* op_asgn */

/********************************************************************
 *
 *	Asign to QBx or QWx	when ft == ARITH
 *	Asign to QXx.y		when ft == GATE
 *
 *	Outputs are ordinary nodes of type ARITH or GATE which may
 *	be used as values anywhere, even before they are assigned.
 *
 *	When the right expression of the assignment is a direct Symbol
 *	that Symbol drives the output gate. This assignment generates
 *	an ALIAS in the usual way. This ALIAS may contain a logic
 *	inversion.
 *
 *	The actual output action is handled by an auxiliary node of
 *	ftype OUTW or OUTX, which is linked to the output value node,
 *	carrying the name of the output. This auxiliary node is never
 *	and can never be used as an input value.
 *
 *	The use of output values before assignment is necessary in iC,
 *	to allow the implementation of feedback in single clocked
 *	expressions. eg: the following output word of 1 second counts
 *
 *		QW1 = SH(QW1 + 1, clock1Second);
 *
 *	The same could of course be realised by using a counter
 *	variable and assigning the output independently. But this way
 *	the compiler does the job - which is what it should do.
 *
 *	With this implementation, the fact that outputs are not full
 *	logic or arithmetic values is completely hidden from the user,
 *	which removes unnecessary errors in user code.
 *
 *******************************************************************/

/********************************************************************
 *
 *	Return operator to use in built in iC functions
 *
 *	this logic ensures that type is taken over if possible to
 *	allow a gate to become an action gate without unnecessary
 *	forcing of levels.
 *
 *	only if a gate is undefined or drives an output use a type
 *	derived from the ftype.
 *
 *	Master gates lead to either ARN or OR
 *
 *******************************************************************/

static unsigned char
bTyp(List_e * lp)
{
    Symbol *		symp;
    unsigned char	tp;
    unsigned char	ft;

    symp = lp->le_sym;
    while (symp->type == ALIAS) {
	symp = symp->list->le_sym;	/* with token of original */
    }
    tp = symp->type & TM;
    return (tp >= MAX_GT) ? ((tp == SH || tp == NCONST || tp == INPW) ? ARN : OR)
			  : (tp == UDF ||
			    symp->u_blist == 0 ||
    (symp->list &&
    ((ft = symp->list->le_sym->ftype) == OUTX || ft == OUTW))) ? types[symp->ftype]
							       : tp;
} /* bTyp */

/********************************************************************
 *
 *	Push one parameter with its clock for a built in iC function
 *
 *******************************************************************/

static List_e *
para_push(
    Sym* fname,
    Lis* aex, Lis* crx,				/* expression and clock pair */
    Lis* cr3,					/* default clock to clone if no own clock */
    List_e* lp3,
    unsigned char ft,				/* 0 or S_FF or R_FF */
    List_e** alp1)
{
    List_e *	lp1;
    List_e *	lp2;
    List_e *	lpc;
    Symbol *	sp;

    /* lpc is either own clock crx->v or clock cloned from cr3->v or iClock */
    lp1 = 0;
    lpc = crx ? crx->v				/* individul clock or timer crx */
	      : cr3 && cr3->v && (sp = cr3->v->le_sym)
		    ? sy_push(sp->ftype != CLCKL && (lp1 = sp->u_blist)
			? lp1->le_sym		/* or clone last timer cr3 */
			: sp)			/* or clone last clock cr3 */
		    : sy_push(iclock);		/* or clone default clock iClock */
    if (lp1 && (sp = lp1->le_sym) && sp->ftype == TIMRL) {
	lp1 = lp1->le_next;			/* type TIM, EXT_TIM, UDF or ALIAS */
	assert(lp1);				/* clone associated timer value */
	assert(lp1->le_val == (unsigned) -1);
	lp2 = sy_push(lp1->le_sym);
	lp2->le_val = (unsigned) -1;		/* mark link as -1 timer before op_push */
	lp2->le_first = lp1->le_first;		/* copy expr text */
	lp2->le_last  = lp1->le_last;		/* copy expr termination */
	lpc = op_push(lpc, TIM, lp2);
    }
    lp1 = op_push(sy_push(fname->v), bTyp(aex->v), aex->v);
    lp1->le_first = aex->f; lp1->le_last = aex->l;
    if (ft) {
	if (lp1->le_sym->ftype == D_FF ||	/* force ft for set or reset */
	    lp1->le_sym->ftype == S_FF) {
	    lp1->le_sym->ftype = ft;		/* right ftype for SR, DSR, DR */
	} else if (lp1->le_sym->ftype == D_SH ||
	    lp1->le_sym->ftype == S_SH) {
	    lp1->le_sym->ftype = S_SH + ft - S_FF;/* right ftype for SHSR, SHR */
	}
    }
    *alp1 = op_push(lpc, lp1->le_sym->type & TM, lp1);	/* return lp1 for pVal */
    lp2 = op_push((List_e *)0, types[lp1->le_sym->ftype], lp1);
    return lp3 ? op_push(lp3, types[lp3->le_sym->ftype], lp2) : lp2;
} /* para_push */

/********************************************************************
 *
 *	Common code to generate built in iC functions
 *
 *******************************************************************/

List_e *
bltin(
    Sym* fname,					/* function name and ftype */
    Lis* ae1, Lis* cr1,				/* expression */
    Lis* ae2, Lis* cr2,				/* optional set */
    Lis* ae3, Lis* cr3,				/* optional reset */
    Lis* crm,					/* optional mono-flop clock */
    Val* pVal)					/* optional cblock# or off-delay */
{
    List_e *	lp1;
    List_e *	lp2;
    List_e *	lp3;

    if (ae1 == 0 || ae1->v == 0) {
	warning("first parameter missing. builtin: ", fname->v->name);
	return 0;				/* YYERROR in fexpr */
    }
    lp3 = para_push(fname, ae1, cr1, cr3, 0, 0, &lp1);	/* lp1 needed for pVal */

    if (ae2) {
	if (ae2->v == 0) {
	    warning("second parameter missing. builtin: ", fname->v->name);
	    return 0;				/* YYERROR in fexpr */
	}
	lp3 = para_push(fname, ae2, cr2, cr3, lp3, S_FF, &lp2);
    }

    if (ae3) {
	if (ae3->v == 0) {
	    warning("third parameter missing. builtin: ", fname->v->name);
	    return 0;				/* YYERROR in fexpr */
	}
	lp3 = para_push(fname, ae3, cr3, 0, lp3, R_FF, &lp2);	/* 0 stops cloning */
    }

    if (crm) {
	/* extra Master for mono-flop is reset fed back from own output */
	lp1 = sy_push(ae1->v->le_sym);		/* use dummy ae1 fill link */
	lp2 = op_push(sy_push(fname->v), UDF, lp1);
	if (lp2->le_sym->ftype == S_FF) {
	    lp2->le_sym->ftype = R_FF;		/* next ftype for SR flip flop*/
	}
	lp2 = op_push(crm->v, lp2->le_sym->type & TM, lp2);
	lp2 = op_push((List_e *)0, types[lp2->le_sym->ftype], lp2);
	lp3 = op_push(lp3, types[lp3->le_sym->ftype], lp2);

	lp1->le_sym = lp3->le_sym;		/* fix link from own */
    }

    if (pVal) {
	/* cblock number for ffexpr or preset off delay for timer */
	lp1->le_val = pVal->v;			/* unsigned int value for case # */
    }
    return lp3;
} /* bltin */

/********************************************************************
 *
 *	Combined code for assignment of an aexpr to an
 *	UNDEF LVAR AVAR LOUT or AOUT
 *
 *******************************************************************/

Symbol *
assignExpression(Sym * sv, Lis * lv, int ioTyp)
{
    Sym		sy;
    Lis		li;
    int		ftyp;
    char	temp[TSIZE];

    ftyp = sv->v->ftype;
    if (lv->v == 0) {
	if (ftyp != ARITH)       { errBit(); return 0;	/* YYERROR */ }
	else if (const_push(lv)) { errInt(); return 0;	/* YYERROR */ }
    }
    if (sv->v->type != UDF) {
	if (ftyp == ARITH) {
	    ierror("multiple assignment to imm int:", sv->v->name);
	} else if (ftyp == GATE) {
	    ierror("multiple assignment to imm bit:", sv->v->name);
	} else {
	    ierror("multiple assignment to wrong imm type:", sv->v->name);
	}
	sv->v->type = ERR;			/* cannot execute properly */
    }
    if (ioTyp) {
	snprintf(temp, TSIZE, "%s_0", sv->v->name);
	sy.v = install(temp, UDF, ioTyp);	/* generate output Gate OUTX or OUTW */
	li.v = sy_push(sv->v);			/* provide a link to LOUT or AOUT */
	if ((li.v = op_push(0, types[ftyp], li.v)) != 0) {
	    li.v->le_first = li.f = 0; li.v->le_last = li.l = 0;
	}
	op_asgn(&sy, &li, ftyp);
    }
    return op_asgn(sv, lv, ftyp);
} /* assignExpression */

/********************************************************************
 *
 *	Code for generating immediate function calls
 *
 *	save global variables for nested function calls
 *	save all links in parameter and statement lists for recursive calls
 *
 *******************************************************************/

struct sF {
    struct sF *	saveFunBs;		/* base of previous save block or 0 */
    Symbol *	iCallHead;		/* function head seen at start of imm call */
    List_e *	iFormNext;		/* next pointer when scanning formal paramaters */
    List_e *	iFunClock;		/* temporary list of unresolved clock parameters */
    List_e *	iSav[1];		/* array for saving parameter links */
};

static struct sF *	saveFunBs = 0;	/* base pointer to save block for recursive calls */
static List_e *		iFormNext = 0;	/* next pointer when scanning formal paramaters */
static List_e *		iFunClock = 0;	/* temporary list of unresolved clock parameters */
static Symbol *		cloneSymbol(Symbol * sp);
static List_e *		cloneList(List_e * slp, Symbol * rsp);

Symbol *
pushFunCall(Symbol * functionHead)
{
    List_e *		lp;
    List_e *		lp1;
    Symbol *		sp;
    Symbol *		sp1;
    struct sF *		oldSFunBs = saveFunBs;
    int			saveCount;
    int			cF;

    lp = functionHead->list;
    if (lp == 0) {
	return;					/* report error later - no popFunCall */
    }
    lp = lp->le_next;				/* first varList link */
    assert(lp);					/* must be a pair */
    saveCount = lp->le_val;			/* allows call to store save block */
    saveFunBs = (struct sF *) emalloc(sizeof(struct sF) - 1 + saveCount * sizeof(void *));
    saveFunBs->saveFunBs = oldSFunBs;
    saveFunBs->iCallHead = iCallHead;
    saveFunBs->iFormNext = iFormNext;
    saveFunBs->iFunClock = iFunClock;
    /* other variables are saved in pushFunParameter() */
    /********************************************************************
     * Save function template parameter and declared variable links
     * Assign parameter links are saved in Pass 1 with other parameters but not cleared.
     * Declarations and assigns are saved and cleared in Pass 2 (assigns are saved twice).
     * Assign parameter links are cleared in Pass 3 with other parameters.
     * Pass 1: parameter list
     *******************************************************************/
    cF = 0;
    lp = functionHead->u_blist;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;
	assert(sp);
	saveFunBs->iSav[cF++] = sp->u_blist;	/* save parameter template link */
	lp = lp->le_next;
    }
    /********************************************************************
     * Pass 2: statement list
     *******************************************************************/
    lp = functionHead->list;			/* statement list */
    while (lp) {
	sp = lp->le_sym;
	assert(sp);
	sp1 = ((lp1 = sp->u_blist) != 0) ? lp1->le_sym : 0;
	if (sp1 != functionHead) {		/* bypass return link */
	    saveFunBs->iSav[cF++] = sp->u_blist;/* save declaration or assign link */
	    sp->u_blist = 0;			/* clear link */
	}
	lp = lp->le_next;			/* next varList link */
	assert(lp);				/* statement list is in pairs */
	lp = lp->le_next;			/* next statement link */
    }
    assert(cF == saveCount);
    /********************************************************************
     * Pass 3: parameter list again - clear links now
     *******************************************************************/
    lp = functionHead->u_blist;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;
	sp->u_blist = 0;			/* clear parameter link */
	lp = lp->le_next;
    }
    /********************************************************************
     * set up for cloning
     *******************************************************************/
    iCallHead = functionHead;			/* make avalable globally */
    assert(iCallHead);				/* must have a function head */
    iFormNext = iCallHead->u_blist;		/* first formal parameter - may be 0 */
    iFunClock = 0;				/* no unresolved clock parameters */
    return functionHead;
} /* pushFunCall */

/********************************************************************
 *
 *	Handle a real parameter of a called immediate function
 *
 *	Parameter 1:	plp is address of previous timer link from yacc stack
 *			also flags, that last parameter was a timer
 *			now expecting a real delay or default delay of 1
 *	Parameter 2:	lp is link to current real parameter if >0
 *			else clean up call in which defaults are handled
 *	return		if lp == 0 timer address if clock is a timer else 0
 *			else for final clean up 0 on error, >0 if OK
 *
 *	This scheme handles recursive function calls in function delay parameters,
 *	which are rather obscure and hard to produce, but must be handled
 *	Example: imm timer tim = TIMER1(TX0.5);	// 1 second timer
 *		 imm int delay(int x, clock c, bit y) { return SH(y?x:1, c); }
 *	Call:	 QB3 = delay(QB1, tim, delay(QB2, tim, 3, IX0.1), IX0.0);
 *
 *******************************************************************/

List_e *
handleRealParameter(List_e * plp, List_e * lp)
{
    Symbol *	fsp;				/* formal Symbol */
    Symbol *	rsp;				/* real Symbol */
    Symbol *	psp;	
    List_e *	rlp;				/* return link if clock is a timer */
    List_e *	clp;				/* list element for missing clocks */
    List_e *	lp1;
    int		formalType;

    if (lp == 0) {				/* final call from cloneFunction() */
	if (plp) {				/* to clear unresolved formal parameters */
	    if ((rsp = lookup(One)) == 0) {	/* default delay 1 after last timer parameter */
		rsp = install(One, NCONST, ARITH); /* becomes NVAR */
	    }
	} else {
	    rsp = iclock;			/* default iClock if no real clock parameter */
	}
	rlp = sy_push(rsp);			/* dummy link, which will be popped */
    } else {
	rlp = lp;
	rsp = rlp->le_sym;			/* current real parameter Symbol */
    }
    assert(rsp);

    while (iFormNext) {
	fsp = iFormNext->le_sym;		/* current formal parameter Symbol */
	assert(fsp);				/* u_blist not cleared on parse error */
	formalType = fsp->ftype;
	if (plp) {
	    if (rsp->ftype == GATE) {		/* previous clock parameter was a timer */
		rlp = op_force(rlp, ARITH);	/* force to int */
		rsp = rlp->le_sym;		/* needed for iFunClock scan */
	    } else
	    if (rsp->ftype != ARITH) {
		ierror("wrong parameter type for timer delay:", rsp->name);
		return 0;			/* error */
	    }
	    psp = plp->le_sym;			/* previous timer Symbol */
	    assert(psp->ftype == TIMRL);
	    /********************************************************************
	     * when cloning formal clock, this link and ftype will indicate timer
	     * which must be cloned differently - action gate must be linked to
	     * le_next of delay link rather than to clock link which is now timer
	     *******************************************************************/
	    plp->le_next = rlp;			/* install delay with previous timer */
	    rlp->le_val = (unsigned) -1;	/* mark link as timer value */
	    while (iFunClock) {			/* find unresolved formal clock parameters */
		iFunClock->le_sym = psp;	/* link unresloved clock to this timer */
		clp = iFunClock->le_next;	/* next unresolved clock */
		iFunClock->le_next = lp1 = sy_push(rsp);	/* clone a delay link */
		lp1->le_val = (unsigned) -1;	/* mark link as timer value */
		iFunClock = clp;
	    }
	} else {
	    switch (formalType) {
	    case ARITH:
		if (rsp->ftype == GATE) {
		    rlp = op_force(rlp, ARITH);	/* force to int */
		} else
		if (rsp->ftype != ARITH) {
		    ierror("wrong parameter type for int:", rsp->name);
		    return 0;			/* error */
		}
		break;
	    case GATE:
		if (rsp->ftype == ARITH) {
		    rlp = op_force(rlp, GATE);	/* force to bit */
		} else
		if (rsp->ftype != GATE) {
		    ierror("wrong parameter type for bit:", rsp->name);
		    return 0;			/* error */
		}
		break;
	    case CLCKL:
		if (rsp->ftype == CLCKL) {
		    while (iFunClock) {		/* find unresolved formal clock parameters */
			iFunClock->le_sym = rsp; /* link unresloved clock to this clock */
			clp = iFunClock->le_next; /* next unresolved clock */
			iFunClock->le_next = 0;	/* clean up list */
			iFunClock = clp;
		    }
		} else
		if (rsp->ftype == TIMRL) {
		    fsp->u_blist = rlp;		/* link real to formal parameter */
		    dFlag = 1;			/* get delay as an NVAR object */
						/* - not next formal parameter */
		    return rlp;			/* current real timer parameter */
		} else {
		    fsp->u_blist = clp = sy_push(0); /* clock is currently unresolved */
		    clp->le_next = iFunClock;	/* because there is no clock parameter */
		    iFunClock = clp;		/* resolve in function call */
		    iFormNext = iFormNext->le_next; /* next formal paramater to process */
		    continue;			/* do not step to next real parameter */
		}
		break;
	    case TIMRL:
		if (rsp->ftype != TIMRL) {
		    ierror("wrong parameter type for timer:", rsp->name);
		    rlp = 0;			/* error */
		}
		dFlag = 1;			/* as an NVAR object */
		break;
	    default:
		ierror("wrong type for function call parameter:", rsp->name);
		rlp = 0;			/* error */
		break;
	    }
	    fsp->u_blist = rlp;			/* link real to formal parameter */
	}
	iFormNext = iFormNext->le_next;		/* next formal paramater to process */
	return lp ? 0 : rlp;			/* real parameter if final call */
    }
    ierror("called function has too many real parameters:", iCallHead ? iCallHead->name : 0);
    return 0;					/* error */
} /* handleRealParameter */

/********************************************************************
 *
 *	Clone a function template in a function call
 *
 *	Cloning is done by scanning each entry pair in the statement list.
 *	The blist of each Symbol found is cloned recursively, unless
 *	it is a parameter or assignable, which are marked by ->type | FM
 *
 *	The first List_e in the statement list holds an instance counter for
 *	generated function internal variables - to allow unique naming.
 *	This field le_val is zero when function is first compiled for
 *	each source. When a function is included in several sources
 *	the counter must be passed to later runs via _List1.h or environment.
 *
 *******************************************************************/

List_e *
cloneFunction(Symbol * functionHead, List_e * plp)
{
    List_e *		rlp = 0;		/* return pointer */
    List_e *		slp;			/* link for statement Symbol */
    List_e *		vlp;			/* link for varList */
    List_e *		lp;
    List_e *		lp1;
    List_e *		lp2;
    Symbol *		sp;
    Symbol *		sp1;
    Symbol *		ssp;			/* Symbol in expression template */
    Symbol *		vsp;			/* Symbol in varList */
    Symbol *		csp;			/* Symbol in cloned exprsssion */
    Symbol *		rsp;			/* return Symbol for feedback */
    Sym			sv;			/* Sym to assign cloned expression to */
    Lis			sl;			/* Lis pointing to cloned expression */
    int			instanceNum;		/* this function call instance number */
    int			saveCount;		/* saveFunPt,iFormHead,iFormNext,iFunClock */
    int			cF;
    struct sF *		oldSFunBs;
    char		temp[TSIZE];

    slp = lp = functionHead->list;	/* get 2 numbers in first elements of statement list */
    if (slp == 0) {
	ierror("called function has no statements - cannot execute!", functionHead->name);
	return 0;
    }
    assert(saveFunBs);				/* no pop without previous push */
    instanceNum = ++lp->le_val;			/* this function call instance number */
    lp = lp->le_next;				/* first varList link */
    assert(lp);					/* must be a pair */
    saveCount = lp->le_val;			/* allows call to store save block */
    /********************************************************************
     * previously parameter links where set up from formal to real parameters
     * find unresolved clock parameters (since real clock parameters optional)
     *******************************************************************/
    while (iFormNext) {
	if (handleRealParameter(plp, 0) == 0) break;
    }
    /********************************************************************
     * real parameters have been scanned and checked against formal parameters
     *******************************************************************/
    if (iFormNext || iFunClock) {
	ierror("called function has too few real parameters:", functionHead->name);
    }
    /********************************************************************
     * Clone the statements of the called function.
     * Scan the statement list starting at the function head in 2 passes.
     * There ar two links per statement:
     *    the first link points to the statement head
     *    the second link points to varList for the statement
     * Pass 1: a) reconstitute templist from Symbols on varList.
     *         b) clone the function internal statement heads to make
     *            them available to all other expressions.
     *         c) clone the return statement head to make it available
     *            to all other expressions.
     *******************************************************************/
    while (slp) {
	vlp = slp->le_next;
	assert(vlp);				/* statement list is in pairs */
	/********************************************************************
	 * reconstitute templist from Symbols on varList
	 *******************************************************************/
	vsp = vlp->le_sym;			/* varList of temp Symbols */
	while (vsp) {				/* varList may be empty */
	    assert(vsp->list == 0);
	    (Symbol*)vsp->list = cloneSymbol(vsp); /* point to new internal Symbol */
	    vsp = vsp->next;			/* next varList Symbol */
	}
	/********************************************************************
	 * clone the expression head associated with this statement
	 * u_blist == 0 indicates a function internal Symbol
	 *******************************************************************/
	ssp = slp->le_sym;			/* expression head template */
	if ((lp = ssp->u_blist) == 0) {
	    snprintf(temp, TSIZE, "%s_%d", ssp->name, instanceNum);
	    *(strchr(temp, '$')) = '_';		/* replace '$' by '_' in cloned object */
	    sv.v = install(temp, UDF, ssp->ftype); /* Symbol for declared variable */
	    ssp->u_blist = sy_push(sv.v);	/* link internal cloned Symbol to template */
	} else
	if (lp != 0 && lp->le_sym == functionHead) {
	    rsp = cloneSymbol(ssp);		/* clone return expression head Symbol */
	}
	slp = vlp->le_next;			/* next statement */
    }
    /********************************************************************
     * Pass 2:
     *******************************************************************/
    slp = functionHead->list;
    while (slp) {
	vlp = slp->le_next;
	ssp = slp->le_sym;			/* expression head template */
	assert(ssp->type & FM);			/* marked statement list head Symbol */
	assert(ssp->list);			/* link to expression net */
	/********************************************************************
	 * clone the expression head associated with this statement
	 *******************************************************************/
	lp = ssp->u_blist;
	assert(lp);				/* internal Symbols linked in pass 1 */
	if (lp->le_sym == functionHead) {
	    rsp->u_blist =			/* return Symbol */
	    cloneList(ssp->list, rsp);		/* clone the rest of the expression */
	    rlp = sy_push(rsp);			/* set up function return link */
	} else {
	    unsigned char	y1[2];
	    int			yn;
	    int			ftyp;
	    int			ioTyp;
	    sv.v = lp->le_sym;			/* assign parameter or internal Symbol */
	    assert(sv.v);			/* ZZZ what happens to link if not used ??? */
	    csp = cloneSymbol(ssp);		/* clone parameter expression head Symbol */
	    csp->u_blist =
	    cloneList(ssp->list, rsp);		/* clone the rest of the expression */
	    /********************************************************************
	     * assign the expression associated with this statement
	     * an output can only be recognised by its name: Q[XBWL]%d
	     *******************************************************************/
	    sl.v = sy_push(csp);		/* link to expression head */
	    if (sscanf(sv.v->name, "Q%1[XBWL]%d", y1, &yn) == 2) {
		ioTyp = (y1[0] == 'X') ? OUTX : OUTW;
	    } else {
		ioTyp = 0;			/* flags that no I/O is generated */
	    }
	    assignExpression(&sv, &sl, ioTyp);	/* assign to internal variable or parameter */
#if YYDEBUG
	    if ((debug & 0402) == 0402) pu(0, "clone", (Lis*)&sv);
#endif
	}
	slp = vlp->le_next;			/* next statement */
    }
    /********************************************************************
     * Cloning is now complete
     * Free links and restore internal pointers in function template.
     * The order is the same as in pushFunCall() to restore correctly.
     * Assign parameter links are restored in Pass 3 with other parameters.
     * Declaration and assigns are restored in Pass 4 (assigns are restored twice).
     * Pass 3: parameter list
     *******************************************************************/
    cF = 0;
    lp = functionHead->u_blist;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;
	assert(sp);
	lp1 = sp->u_blist;
	while (lp1) {
	    lp2 = lp1->le_next;
	    sy_pop(lp1);			/* free link pushed in handleRealParameter */
	    lp1 = lp2;
	}
	sp->u_blist = saveFunBs->iSav[cF++];	/* restore previous parameter template link */
	lp = lp->le_next;
    }
    /********************************************************************
     * Pass 4: statement list
     *******************************************************************/
    slp = functionHead->list;			/* statement list */
    while (slp) {
	sp = slp->le_sym;
	assert(sp);
	sp1 = ((lp1 = sp->u_blist) != 0) ? lp1->le_sym : 0;
	if (sp1 != functionHead) {		/* bypass return link */
	    while (lp1) {
		lp2 = lp1->le_next;
		sy_pop(lp1);			/* free link pushed in cloneFunction */
		lp1 = lp2;
	    }
	    sp->u_blist = saveFunBs->iSav[cF++];/* restore declaration and assign link */
	}
	vlp = slp->le_next;			/* next varList link */
	assert(vlp);				/* statement list is in pairs */
	sp = vlp->le_sym;			/* varList */
	while (sp) {
	    sp->list = 0;			/* clear internal Symbol pointers */
	    sp = sp->next;
	}
	slp = vlp->le_next;			/* next statement link */
    }
    assert(cF == saveCount);
    /********************************************************************
     * restore globals from nested function call
     *******************************************************************/
    iFunClock = saveFunBs->iFunClock;
    iFormNext = saveFunBs->iFormNext;
    iCallHead = saveFunBs->iCallHead;
    oldSFunBs = saveFunBs->saveFunBs;
    free(saveFunBs);				/* free memory - no need for size */
    saveFunBs = oldSFunBs;			/* base of previous save block or 0 */
    return rlp;					/* return expression or 0 for void function */
} /* cloneFunction */

/********************************************************************
 *
 *	Clone one Symbol without any links
 *
 *******************************************************************/

static Symbol *
cloneSymbol(Symbol * sp)
{
    Symbol *		rsp;			/* return pointer */
#if YYDEBUG
    char 		temp[TSIZE];
#endif

    rsp = (Symbol *) emalloc(sizeof(Symbol));
    rsp->name = NS;		/* no name at present */
#if YYDEBUG
    if ((debug & 0402) == 0402) {	/* DEBUG name */
	snprintf(temp, TSIZE, "$%d", ++tn);
	rsp->name = emalloc(strlen(temp)+1);	/* +1 for '\0' */
	strcpy(rsp->name, temp);
    }
#endif
    rsp->type = sp->type & ~FM;			/* some formal Symbols are marked with FM */
    rsp->ftype = sp->ftype;
    rsp->next = templist;			/* put at front of templist */
    templist = rsp;
    return rsp;
} /* cloneSymbol */

/********************************************************************
 *
 *	Clone the links from one Symbol and the rest recursively
 *
 *******************************************************************/

static List_e *
cloneList(List_e * slp, Symbol * rsp)
{
    List_e *		rlp = 0;		/* return pointer */
    List_e *		tlp = 0;		/* temp pointer */
    List_e *		clp;			/* cloned list link */
    List_e *		nlp;			/* next cloned list link */
    Symbol *		ssp;			/* current source Symbol */
    Symbol *		nsp;			/* cloned new Symbol */
    static Symbol *	first_rsp;

    if (rsp) {
	first_rsp = rsp;			/* feedback Symbol */
    }
    while (slp) {
	ssp = slp->le_sym;
	assert(ssp);
	if (ssp->type & FM) {			/* parameter or declared value */
	    if ((nlp = ssp->u_blist) != 0) {
		nsp = nlp->le_sym;		/* pointer to real para set up before */
		if (nsp == iCallHead) {
		    nsp = first_rsp;		/* apply feedback */
		    nlp = 0;
		} else {
		    nlp = nlp->le_next;		/* possible timer parameter link */
		}
		assert(nsp);
	    } else {
		ierror("no link from formal to real parameter:", ssp->name);
		break;				/* cannot continue */
	    }
	} else
	if (ssp->name) {			/* global variable Symbol */
	    nsp = ssp;				/* pointer to global variable */
	    nlp = 0;
	} else {				/* function internal Symbol ->name == 0 */
	    nsp = (Symbol*)ssp->list;		/* direct link set up in cloneFunction */
	    assert(nsp);
	    assert(nsp->u_blist == 0);
	    nsp->u_blist =
	    cloneList(ssp->u_blist, 0);		/* clone internal expression recursively */
	    nlp = 0;
	}
	clp = sy_push(nsp);			/* clone one list element */
	clp->le_val = slp->le_val;		/* negation or arithmetic index in para */
	/* ZZZ copy le_first and le_last */
	if (tlp == 0) {
	    tlp = rlp = clp;			/* first element is returned */
	} else {
	    tlp = tlp->le_next = clp;		/* other elements are linked in order */
	}
	while (nlp) {
	    nsp = nlp->le_sym;			/* delay parameter */
	    assert(nsp);
	    clp = sy_push(nsp);			/* clone follow up delay element */
	    clp->le_val = nlp->le_val;		/* negation or arithmetic index in delay */
	    /* ZZZ copy le_first and le_last */
	    tlp = tlp->le_next = clp;		/* also link delay element */
	    nlp = nlp->le_next;			/* ZZZ 0 unless other parameter expression */
	}
	slp = slp->le_next;			/* next formal list element */
    }
    return rlp;					/* first link in cloned chain */
} /* cloneList */

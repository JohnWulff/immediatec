static const char genr_c[] =
"@(#)$Id: genr.c,v 1.28 2001/01/13 17:47:02 jw Exp $";
/************************************************************
 * 
 *	"genr.c"
 *
 *	"genr.c	3.51	95/02/04"
 *
 ************************************************************/

#ifdef _WINDOWS
#include	<windows.h>
#endif
#include	<stdio.h>
#ifndef _WINDOWS 
#include	<stdlib.h>
#endif
#include	<string.h>
#include	<assert.h>
#include	"pplc.h"
#include	"comp.h"
#include	"y.tab.h"

#define v(lp)	(lp->le_val) ? '~' : ' ', lp->le_sym->name
#define w(lp)	(lp->le_val) ? '~' : ' '

static Symbol *	templist;	/* temp list of un-named symbols */
static short	ttn;		/* for generating temp f object name */
#ifdef YYDEBUG
static short	tn;
#endif
char		eBuf[1024];	/* temporary expression text buffer */

/********************************************************************
 *
 *	initialize for code generation
 *
 *******************************************************************/

void
initcode(void)			/* initialize for code generation */
{
    templist = 0;
#ifdef YYDEBUG
    tn = 0;
#endif
} /* init_code */

/********************************************************************
 *
 *	create List element for variable
 *
 *******************************************************************/

List_e *
sy_push(register Symbol * var)	/* create List element for variable */
{
    register List_e *	lp;

    lp = (List_e *) emalloc(sizeof(List_e));
    lp->le_sym = var;	/* point to variables Symbol entry */
#ifdef YYDEBUG
    if (debug & 02) {
	fprintf(outFP, "++%s\n", var->name);
	fflush(outFP);
    }
#endif
    return (lp);	/* return pointer to List_e to yacc */
} /* sy_push */

/********************************************************************
 *
 *	delete List element left over
 *
 *******************************************************************/

Symbol *
sy_pop(register List_e * lp)	/* delete List element left over */
{
    register Symbol *	sp;

    sp = lp->le_sym;	/* point to variables Symbol entry */
    free(lp);
#ifdef YYDEBUG
    if (debug & 02) {
	fprintf(outFP, "  %s--\n", sp->name);
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
    register List_e *	lp,
    register uchar	ftyp)
{
    register Symbol *	sp;
    register List_e *	lp1;

    if (lp && (sp = lp->le_sym)->ftype != ftyp) {
	if (sp->u.blist == 0 ||			/* not a $ symbol or */
	    sp->type >= MAX_GT ||		/* SH, FF, EF, VF, SW, CF or */
	    sp->u.blist->le_sym == sp && sp->type == LATCH) { /* L(r,s) */
	    lp1 = op_push(0, types[sp->ftype], lp);
	    lp1->le_first = lp->le_first;
	    lp1->le_last = lp->le_last;
	    lp = lp1;	/* create a new $ symbol linked to old */
	    sp = lp->le_sym;
	}
#ifdef YYDEBUG
	if (debug & 02) {
	    fprintf(outFP, "\tforce %s from %s to %s\n",
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
    register List_e *	left,
    register uchar	op,
    List_e *		right)
{
    register List_e *	rlp;
    register Symbol *	sp;	/* current temporary Symbol */
    char 		temp[8];
    Symbol *		lsp;
    Symbol *		tsp;
    List_e *		lp;

    if (right == 0) {
	if ((right = left) == 0) return 0;	/* nothing to push */
	left = 0;
    }
    rlp = right;
    sp = rlp->le_sym;
    if (left && op > OR && op < MAX_LV && op != sp->type) {
	warning("function types incompatible", NS);
    }
    if (sp->u.blist == 0 || op != sp->type) {
	/* right not a $ symbol or new operator - force new level */
	sp = (Symbol *) emalloc(sizeof(Symbol));
	sp->name = NS;		/* no name at present */
#ifdef YYDEBUG
	if (debug & 02) {	/* DEBUG name */
	    sprintf(temp, "$%d", ++tn);
	    sp->name = emalloc(strlen(temp)+1);	/* +1 for '\0' */
	    strcpy(sp->name, temp);
	}
#endif
	sp->type = op != UDF ? op : AND; /* operator OR or AND (default) */
	sp->ftype = rlp->le_sym->ftype;	 /* used in op_xor() with op UDF */
	sp->next = templist;	/* put at front of list */
	templist = sp;
	rlp->le_next = sp->u.blist;
	sp->u.blist = rlp;	/* link right of expression */
	rlp = sy_push(sp);	/* push new list element on stack */
    }
    if (left) {
	lsp = left->le_sym;
	if (lsp->ftype >= MIN_ACT && lsp->ftype < MAX_ACT) {
	    if (sp->ftype < S_FF) {
		sp->ftype = 0;	/* OK for any value of GATE */
	    }
	    sp->ftype |= lsp->ftype;	/* modify S_FF ==> D_FF */
	}
	if (lsp->type < MAX_LS) {
	    if ((lp = lsp->u.blist) == 0 ||	/* left not a $ symbol */
		sp == lsp ||			/* or right == left */
		lsp->type != op &&		/* or new operator */
		lsp->type != TIM ||		/* but left is not a timer */
		lsp->type == op &&		/* or old operator */
		right->le_val == (unsigned) -1)	/* and right is a delay for timer */
	    {
		left->le_next = sp->u.blist;	/* extend expression */
		sp->u.blist = left;		/* link left of expr */
#ifdef YYDEBUG
		if (debug & 02) {
		    fprintf(outFP, "\t%c%s %c %c%s\n",
			v(left), os[op], v(right));
		    fflush(outFP);
		}
#endif
	    } else {	/* left is a $ symbol and ... - merge left into right */
		while (lp->le_next) {
		    lp = lp->le_next;
		}
		lp->le_next = sp->u.blist;	/* move connect list */
		sp->u.blist = lsp->u.blist;	/* in the right order */
		if (templist != lsp) {
		    tsp = templist;			/* scan templist */
		    while (tsp->next != lsp) {
			tsp = tsp->next;
			if (tsp == 0) {
			    fprintf(outFP, "%s: line %d\t", __FILE__,__LINE__);
			    execerror("left temp not found ???\n", NS);
			}
		    }
		    tsp->next = lsp->next;		/* unlink lsp from templist */
		} else {
		    templist = lsp->next;		/* unlink first object */
		}
#ifdef YYDEBUG
		if (debug & 02) {
		    fprintf(outFP, "\t%c%s %c %c%s\n",
			v(left), os[op], v(right));
		    fflush(outFP);
		}
		sy_pop(left);			/* left Link_e */
		if (debug & 02) {
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
#ifdef YYDEBUG
    } else if (debug & 02) {		/* fexpr : sexpr { left is 0 } */
	fprintf(outFP, "\t(0) %c %c%s\n", os[op], v(right));
	fflush(outFP);
#endif
    }
    return (rlp);
} /* op_push */

/********************************************************************
 *
 *	special exclusive or push
 *
 *******************************************************************/

List_e *
op_xor(				/* special exclusive or push */
    register List_e *	left,
    register List_e *	right)
{
    register List_e *	inv_left;
    register List_e *	inv_right;
    List_e*		lp1;
    List_e*		lp2;

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
op_not(register List_e * right)		/* logical negation */
{
    register Symbol *	sp;
    register List_e *	lp;
    register Symbol *	ssp;
    register List_e *	llp;

    if (!(lp = (sp = right->le_sym)->u.blist)) {
	right->le_val ^= NOT;		/* negate logical value */
    } else {
	switch (sp->type) {			/* $ symbol */
	case AND:
	case OR:
	    sp->type ^= (AND ^ OR);		/* de Morgans rule */
	case LATCH:
	    while (lp) {
		if (lp->le_sym != sp) {		/* ignore latch feedback */
		    lp->le_val ^= NOT;
		}
		lp = lp->le_next;
	    }
	    break;
	case ARN:
	case ARNC:
	case SH:
	case FF:
	case VF:
	case EF:
	case SW:
	case CF:
	    right->le_val ^= NOT;	/* negate logical value */
	    				/* forces creation of alias */
	    break;			/* if assigned immediately */
	case INPW:
	case INPX:
	    fprintf(outFP, "%s: line %d\t", __FILE__, __LINE__);
	    execerror("INPUT has other inputs in op_not() ???", sp->name);
	    break;
	default:
	    fprintf(outFP, "%s: line %d\t", __FILE__, __LINE__);
	    execerror("negation of non-logical value attempted", sp->name);
	    break;
	}
    }
#ifdef YYDEBUG
    if (debug & 02) {
	fprintf(outFP, "    ~%s\n", right->le_sym->name);
	fflush(outFP);
    }
#endif
    return (right);
} /* op_not */
 
/********************************************************************
 *
 *	asign List_e stack to links
 *
 *	Sym sv contains Symbol *v and char *f and *l to source
 *	Lis lr contains List_e *v and char *f and *l to source
 *	uchar ft is the ftype which right must be forced to
 *
 *******************************************************************/

Symbol *
op_asgn(			/* asign List_e stack to links */
    Sym *	sv,		/* may be 0 for ffexpr */
    Lis *	rl,
    uchar	ft)
{
    register Symbol *	var;
    register Symbol *	sp;
    register List_e *	lp;
    register Symbol *	gp;
    register Symbol *	rsp;
    register List_e *	tlp;
    List_e *		right;
    char		temp[100];
    short		atn;
    short		sflag;
    Symbol *		sr;
    char *		t_last = 0;
    char *		t_first = 0;

    right = op_force(rl->v, ft);   /* force Symbol on right to ftype ft */
    if (sv == 0) {
	/* null var - generate a temporary Symbol of type UNDEF */
	var = (Symbol *) emalloc(sizeof(Symbol));
	sprintf(temp, "_f%d", ++ttn);
	var->name = emalloc(strlen(temp)+1);	/* +1 for '\0' */
	strcpy(var->name, temp);	/* name needed for derived Sy's */
	sflag = 0;			/* don't output name */
    } else {
	var = sv->v;			/* Symbol * var */
	sflag = 1;			/* print output name */
    }
#ifdef YYDEBUG
    if (debug & 02) {
	fprintf(outFP, "\t  %s = %c%s\n", var->name, v(right));
	fflush(outFP);
    }
#endif
    rsp = right->le_sym;
    if (var->type >= AND && var->type != rsp->type) {
	error("type mismatch in multiple assignment:", var->name);
	var->type = ERR;		/* reduce anyway to clear list */
    } else {
	var->type = rsp->type;
	if (var->ftype < MIN_ACT) {
	    var->ftype = rsp->ftype;
	}
    }
    if (rsp->type == NCONST || rsp->u.blist == 0) {		/* right must be a $ symbol */
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
		error("circular list of aliases:", var->name);
		break;			/* circular list of aliases */
	    }
	    right->le_val ^= rsp->list->le_val;
	    if (!(rsp = rsp->list->le_sym)) {
		fprintf(outFP, "%s: line %d\t", __FILE__, __LINE__);
		execerror("ALIAS points to nothing ???",
		right->le_sym->name);
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
		(rsp->type >= MAX_LV) ? os[rsp->type] : w(right),
		os[var->type], var->name);
	}
	if (sv == 0) {
	    fprintf(outFP, "%s: line %d\t", __FILE__, __LINE__);
	    execerror("ALIAS points to temp ???", var->name);
	} else {
	    t_first = sv->f; t_last = sv->l;	/* full text of var */
	    while (t_first && t_first < t_last) {
		*t_first++ = '#';		/* mark left var, leave ALIAS */
	    }
	}
	return (var);			/* needs no reduction */
    }

    if (rsp != (sp = templist)) {
	while (rsp != sp->next) {
	    if ((sp = sp->next) == 0) {			/* DEBUG */
		fprintf(outFP, "%s: line %d\t", __FILE__, __LINE__);
		execerror("right->le_sym not found in templist ???",
		    right->le_sym->name);
	    }
	}
	sp->next = rsp->next;	/* link tail to head in front of rsp */
	rsp->next = templist;	/* link head to rsp */
	templist = rsp;		/* now rsp is head of templist */
    }
    if ((rsp->type == CLK || rsp->type == TIM) && var->list) {
	warning("clock or timer has been used as variable:", var->name);
    }

    sr = rsp->u.blist->le_sym;		/* gate linked to var */
    if (right->le_val == NOT ^ NOT) {
	var->u.blist = rsp->u.blist;	/* move blist from rsp to var */
	templist = rsp->next;		/* bypass rsp */
    } else {
	/* make var an ALIAS because of FF negation */
	rsp->ftype = GATE;		/* may be odd value from gen */
	var->type = ALIAS;		/* make var negated ALIAS */
	var->u.blist = right;		/* link var to right */
    }

    atn = (debug & 0100000) ? -1 : 0;	/* QW0_0 generated for qp_value ALIAS */
    sp = var;				/* start reduction with var */
    t_first = rl->f; t_last = rl->l;	/* full text of expression */
    if (debug & 02) fprintf(outFP, "resolve \"%s\" to \"%s\"\n", t_first, t_last);
    do {				/* marked symbol */
	int	gt_input;
	char *	ep = eBuf;		/* temporary expression buffer */
	if (debug & 04) {
	    fprintf(outFP, "\n");
	}
	gt_input = 0;			/* output scan for 1 gate */
	while ((lp = sp->u.blist) != 0) {
	    sp->u.blist = lp->le_next;
	    if ((gp = lp->le_sym) == rsp && var->type != ALIAS) {
		gp = var;		/* link points to right */
	    }
	    while (gp->type == ALIAS) {
		lp->le_val ^= gp->list->le_val;	/* negate if necessary */
		gp = gp->list->le_sym;		/* point to original */
	    }
	    if (sp == gp && sp->ftype >= MIN_ACT) {
		if (sp->ftype == D_FF) {
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
		    gp->name = NS;		/* no name at present */
#ifdef YYDEBUG
		    if (debug & 02) {	/* DEBUG name */
			sprintf(temp, "$%d", ++tn);
			gp->name = emalloc(strlen(temp)+1);
			strcpy(gp->name, temp);
		    }
#endif
		    gp->type = sp->type;
		    gp->ftype = GATE;
		    sp->type = OR;		/* OR default for 1 input */
		    gp->next = templist;	/* put at front of list */
		    templist = gp;
		    gp->u.blist = tlp = sy_push(gp);	/* link self */
		    tlp->le_next = sp->u.blist;	/* rest of inputs on sp */
		    sp->u.blist = 0;
		}
	    }
	    if (sp->type == ALIAS) {
		/* var was made an ALIAS because of FF negation */
		gp->list = tlp = sp->list;	/* start of left list */
		while (tlp) {
		    tlp->le_val ^= lp->le_val;	/* negate if necesary */
		    tlp = tlp->le_next;		/* scan to end of list */
		}
		sp->list = lp;		/* link ALIAS to right */
	    } else {
		/* link Symbols to the end of gp->list to maintain order */
		lp->le_next = 0;
					/* && gp->u.blist ZZZ */
		if (gp->ftype == ARITH && lp->le_val != (unsigned) -1) {
		    lp->le_val = c_number + 1;	/* arith case # */
		}
		if ((tlp = gp->list) == 0) {
		    gp->list = lp;	/* this is the first Symbol */
		} else {
		  loop:		/* special loop with test in middle */
		    if (tlp->le_sym == sp) {
			if (gp->ftype == OUTW || gp->ftype == OUTX) {
		    warning("input equals output at output gate:", gp->name);
			} else if (tlp->le_val != lp->le_val) {
			    warning("gate has input and inverse:", gp->name);
			} else {
			    sy_pop(lp);	/* ignore duplicate link */
			    continue;	/* output scan for 1 gate */
			}
		    }
		    if (tlp->le_next) {
			tlp = tlp->le_next;	/* find last Symbol */
			goto loop;
		    }
		    tlp->le_next = lp;	/* put new Symbol after last */
		}
		lp->le_sym = sp;	/* completely symmetrical */
		if (gp->type < MAX_LV) {
		    gt_input++;		/* count the gate inputs */
		}
	    }
	    if (! gp->name
#ifdef YYDEBUG
		|| *(gp->name) == '$'
#endif
	    ) {			/* not marked symbol */
#ifdef YYDEBUG
		if (debug & 02) {
		    if (debug & 04) {
			fprintf(outFP, "%s =", gp->name);
		    } else {
			fprintf(outFP, "\t  %s cleared\n", gp->name);
		    }
		    free(gp->name);		/* free name space */
		}
#endif
		sprintf(temp, "%s_%d", var->name, ++atn);
		gp->name = emalloc(strlen(temp)+1);	/* +1 for '\0' */
		strcpy(gp->name, temp);	/* mark Symbol */
	    }
	    if (debug & 04) {
		if (gp->type >= MAX_LV) {
		    fprintf(outFP, "\t%s\t%c ---%c", gp->name, os[gp->type],
			os[sp->type]);
		} else if (gp->ftype < MAX_AR && lp->le_val == (unsigned) -1) {
		    /* reference to a timer value - no link */
		    fprintf(outFP, "\t%s\t%c<---%c", gp->name, fos[gp->ftype],
			os[sp->type]);
		} else if (gp->ftype != GATE) {
		    fprintf(outFP, "\t%s\t%c ---%c", gp->name, fos[gp->ftype],
			os[sp->type]);
		} else {
		    if (sp->type == ALIAS) iFlag = 1;
		    fprintf(outFP, "\t%s\t%c ---%c", gp->name, w(lp),
			os[sp->type]);
		}
		if (sflag) {
		    fprintf(outFP, "\t%s", sp->name);
		    if (sp->ftype != GATE) {
			fprintf(outFP, "\t%c", fos[sp->ftype]);
		    }
		}
		if (gp->ftype == F_SW || gp->ftype == F_CF) {
		    /* function case # */
		    fprintf(outFP, "\t(%d)", lp->le_val);
		}
	    }
	    if (gp->ftype == ARITH && sp->type == ARN &&
		(gp->u.blist || gp->type == NCONST)) {
		char	buffer[BUFS];	/* buffer for modified names */
		char	iqt[2];		/* char buffers - space for 0 terminator */
		char	bwx[2];
		unsigned	byte;
		unsigned	bit;
		char	tail[8];	/* compiler generated suffix _123456 max */

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
		while (t_first && t_first < lp->le_first) {	/* end of arith */
		    if (*t_first != '#') {	/* transmogrified '=' */
			if (debug & 04) putc(*t_first, outFP);
			*ep++ = *t_first;
		    }
		    t_first++;
		}
		if (debug & 04) fprintf(outFP, "_(%s)", gp->name);
		if (outFlag == 0) {
		    ep += sprintf(ep, "_(%s)", gp->name);	/* -c compile */
		} else {
		    /* modify IXx.x and QXx.x names for compiled output only */
		    IEC1131(gp->name, buffer, BUFS, iqt, bwx, &byte, &bit, tail);
		    ep += sprintf(ep, "_(%s%s)", gp->type == NCONST ? "_"
								    : "", buffer);
		}
		t_first = lp->le_last;	/* skip logic expr's */
	    }
	    if (debug & 04) {
		fprintf(outFP, "\n");
		sflag = debug & 0200;
	    }
	    if (sp == gp && (sp->type != LATCH || lp->le_val != NOT ^ NOT)) {
		warning("input equals output at gate:", sp->name);
	    }
	}					/* end output scan for 1 gate */
	if (sp->type == ARN) {
	    if (debug & 04) fprintf(outFP, "\t\t\t\t\t");
	    while (t_first && t_first < t_last) {
		if (*t_first != '#') {	/* transmogrified '=' */
		    if (debug & 04) putc(*t_first, outFP);
		    *ep++ = *t_first;
		}
		t_first++;
	    }
	    *ep++ = 0;
						/* start case or function */
	    if ((debug & 0100000) == 0) {	/* output cexe function */
		fprintf(exoFP, cexeString[outFlag], ++c_number);
		fprintf(exoFP, "#line %d \"%s\"\n", lineno, inpNM);
		fprintf(exoFP, "	return %s;\n", eBuf);
		if (outFlag != 0) {
		    fprintf(exoFP, "}\n\n");	/* terminate function */
		}
		if (debug & 04) fprintf(outFP, "; (%d)\n", c_number);
	    } else {
		if (debug & 04) fprintf(outFP, "; (dummy assignment)\n");
	    }
	}
	sflag = 1;			/* print output name */
	if (gt_input > PPGATESIZE) {
	    sp->type = ERR;		/* cannot execute properly */
	    error("too many inputs on gate:", sp->name);
	}
	if ((gp = sp = templist) != 0) {
	    if (sp->name
#ifdef YYDEBUG
		&& *(sp->name) != '$'
#endif
	    ) {				/* marked symbol is first */
		templist = sp->next;	/* by_pass marked symbol */
	    } else {
		while ((sp = sp->next) != 0 && (!sp->name
#ifdef YYDEBUG
		    || *(sp->name) == '$'
#endif
		)) {
		    gp = sp;		/* look for marked symbol */
		}
		if (sp) {
		    gp->next = sp->next;/* by_pass marked symbol */
		}
	    }
	    if (sp) {
		(void) place_sym(sp);	/* place sp in the symbol table */
		if (sp->type == ARN) {
		    t_first = sp->list->le_first;
		    t_last = sp->list->le_last;
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

    if (right->le_val == NOT ^ NOT) {
	sy_pop(right);			/* right Symbol and List_e */
    #ifdef YYDEBUG
	if (debug & 02) {
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

    t_first = rl->f; t_last = rl->l;	/* full text of expression */
    while (t_first && t_first < t_last) {
	*t_first++ = '#';		/* mark embedded assignments */
    }
    if (sv == 0) {
	lp = sr->list;			/* link action to temp */
	if (lp->le_sym != var || lp->le_next) {
	    fprintf(outFP, "%s: line %d\t", __FILE__, __LINE__);
	    execerror("error in unlinking temp:", var->name);
	}
	lp->le_sym = 0;			/* erase reference to temp */
	if (debug & 02) {
	    fprintf(outFP, "\t  %s deleted\n\n", var->name);
	    fflush(outFP);
	}
	free(var->name);		/* free name space */
	free(var);			/* temporary Symbol */
	return 0;
    }
    return (sv->v);
} /* op_asgn */
 
/********************************************************************
 *
 *	Value from QBx or QWx	when ft == ARITH
 *	Value from QXx.y	when ft == GATE
 *
 *	Once an output symbol has been assigned to with qp_asgn() an
 *	auxiliary gate named QW1_1 etc points to the output QW1, which
 *	is the name used in expressions. yylex() returns QW1_1 by use
 *	of the backpointer generated from QW1 in qp_asgn(). In this
 *	case act->v->list != 0 and most of the code below is skipped.
 *
 *	When an output gate is used for input, but has never been
 *	assigned to yet, act->v->list == 0 and the code below assigns
 *	the output gate from a temporary gate variable using qp_asgn().
 *	This ensures that a provisional auxiliary driver gate exist.
 *	This provisional driver gate is named QW1_0 etc.
 *
 *	Extra code in qp_asgn() re-assign the output gate again,
 *	without notifying a multiple assignment. The provisional driver
 *	QW1_0 is made an ALIAS to the auxiliary driver generated in the
 *	new assignment.  All outputs from the previous auxiliary output
 *	driver (now an ALIAS) are transferred to the new auxiliary driver.
 *
 *	This use of output values before assignment is necessary in iC,
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
 *	Lis expr contains List_e *v and char *f and *l to source
 *	Sym act contains Symbol *v and char *f and *l to source
 *
 *******************************************************************/

void
qp_value(Lis * expr, Sym * act, uchar ft)
{
    Lis		li;

    expr->f = li.f = act->f; expr->l = li.l = act->l;
    if ((li.v = act->v->list) == 0) {
	Symbol *	sp;		/* output not yet assigned */
	char		temp[100];
	short		saveDebug;
	char *		bp;
	char *		cp;
	char *		dp;

	sp = install("__Tmp_0", INPW, OUTX);	/* temporary __Tmp symbol */
	li.v = sy_push(sp);			/* provide a link to __Tmp */
	saveDebug = debug;
	if ((debug & 06) == 04) {
	    debug &= ~04;			/* supress listing output */
	}
	debug |= 0100000;			/* supress cexe output */
	li.v = qp_asgn(act, &li, ft)->list;	/* ft is ARITH or GATE */
	li.v->le_sym->type = UDF;		/* not really defined yet */
	debug = saveDebug;			/* restore listing output */
	if (ft == ARITH) {
	    cp = temp;
	    cp += sprintf(temp, "_(%s)", li.v->le_sym->name);
	    bp = act->l;			/* copy rest of lexed statement */
	    expr->l = li.l = act->l = act->f + (cp - temp);
	    while (bp < stmtp) {
		*cp++ = *bp++;
	    }
	    bp = temp;
	    dp = act->f;			/* copy extended statement */
	    while (bp < cp) {
		*dp++ = *bp++;
	    }
	    stmtp = dp;				/* fix WACT name */
	}
	unlink_sym(sp);				/* unlink __Tmp symbol */
	free(sp->name);
	sy_pop(sp->list);
	free(sp);			/* free all references to W_ symbol */
    }
    expr->v = sy_push(li.v->le_sym);	/* link to output driver */
    expr->v->le_val = li.v->le_val;	/* copy function number or inversion */
    expr->v->le_first = expr->f; expr->v->le_last = expr->l;
} /* qp_value */
 
/********************************************************************
 *
 *	Asign to QBx or QWx	when ft == ARITH
 *	Asign to QXx.y		when ft == GATE
 *
 *	When the right expression of the assignment is a direct Symbol
 *	that Symbol drives the output gate. This assignment generates
 *	a backpointer in the output gate to the Symbol driving the output.
 *	The backpointer is used to locate the actual logic or arithmetic
 *	value, which that output represents. The backpointer may contain
 *	a logic inversion. In this way the use of an output gate as input
 *	is handled just like an ALIAS.
 *
 *	In the more usual case, where the right expression is a logical
 *	or arithmetic expression, that expression resolves into a gate,
 *	which is named QW1_1 etc. This is the driver gate and is linked
 *	to the actual output gate just as described above. The driver is
 *	used for all inputs, since output gates of ftyp OUTW and OUTX
 *	cannot drive logic or arithmetic directly.
 *
 *	Sym sv contains Symbol *v and char *f and *l to source
 *	Lis lr contains List_e *v and char *f and *l to source
 *
 *******************************************************************/

Symbol *
qp_asgn(
    Sym *	sv,
    Lis *	rl,
    uchar	ft)
{
    Lis		li1;
    List_e *	lp;
    List_e *	lpf;
    List_e **	lpp;
    Symbol *	svv;
    Symbol *	sp;
    Symbol *	fsp;
    Symbol *	bsp = 0;
    List_e *	lpb = 0;
    uchar	oft = OUTW - ARITH + ft;	/* ensure order of ftypes is correct */

    if ((svv = sv->v)->type != UDF) {
	if (svv->type != ERR && (
	    svv->ftype != oft ||		/* oft is OUTW or OUTX */
	    (lpf = svv->list) == 0 ||
	    (bsp = lpf->le_sym) == 0 ||
	    bsp->type != UDF
	)) {
	    error("multiple assignment to imm bit or int:", svv->name);
	    svv->type = ERR;			/* cannot execute properly */
	} else {
	    svv->type = UDF;
	}
	free(lpf);				/* old back link */
    }
    lpf = op_force(rl->v, ft);			/* ARITH or GATE */
    fsp = lpf->le_sym;				/* aexpr symbol */
    svv->list = sy_push(lpf->le_sym);		/* create back link */
    svv->list->le_val = lpf->le_val;		/* copy inversion to back link */
    li1.v = op_push((List_e *)0, UDF, lpf);	/* type <== AND */
    li1.v->le_sym->type = types[ft];		/* type <== ARN or OR */
    li1.f = rl->f;	/* for op_asgn */
    li1.l = rl->l;	/* for op_asgn */
    if (bsp) {
	/*
	 * QW1_0 is made an ALIAS to the auxiliary driver generated in the
	 * new assignment.  All outputs from the previous auxiliary output
	 * driver (now an ALIAS) are transferred to the new auxiliary driver.
	 *
	 * ALIAS should be complete before the op_asgn() below, so that any
	 * inputs of its own output are resolved correctly in that assignment.
	 */
	assert(fsp);
	bsp->type = ALIAS;			/* convert old driver to ALIAS */
	for (lpp = &bsp->list, lp = *lpp; lp; lpp = &lp->le_next, lp = *lpp) {
	    if (lp->le_sym == svv) {
		lp->le_sym = fsp;		/* is ALIAS to driver gate */
		*lpp = lp->le_next;		/* rest of output list */ 
		lp->le_next = 0;		/* ALIAS has backward link only */
		lp->le_val = lpf->le_val;	/* handle inversion */
		lpb = bsp->list;		/* old output list or 0 */ 
		bsp->list = lp;			/* now link ALIAS correctly */
		break;
	    }
	}
	assert(lp != 0);			/* WHAT! no link to output ? */
    }
    sp = op_asgn(sv, &li1, oft);		/* oft is OUTW or OUTX */
    if (bsp) {
	if (lpb) {
	    if ((lp = fsp->list) != 0) {
		fsp->list = lpb;		/* put old output list first */
		while (lpb->le_next != 0) {
		    lpb = lpb->le_next;
		}
		lpb->le_next = lp;		/* link new output at end of old */
		if (debug & 04) {		/* generate listing output */
		    iFlag = 1;
		    fprintf(outFP, "\t%s\t%c ---%c\t%s\n\n", fsp->name,
			(fsp->ftype != GATE) ? fos[fsp->ftype] : w(lpf),
			os[bsp->type], bsp->name);
		}
	    }
	} else if (bsp->ftype >= MAX_AR) {
	    unlink_sym(bsp);			/* ALIAS not required */
	    free(bsp->name);
	    sy_pop(bsp->list);
	    free(bsp);				/* free all references to ALIAS */
	} else {
	    /* these symbol table entries are needed to edit cexe.tmp */
	    if (debug & 04) iFlag = 1;
	    bsp->type = DALIAS;			/* deleted arithmetic ALIAS */
	}
    }
    return sp;
} /* qp_asgn */

/********************************************************************
 *
 *	return operator to use in built in iC functions
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

static uchar
bTyp(List_e * lp)
{
    Symbol *	symp;
    uchar	tp;
    uchar	ft;

    symp = lp->le_sym;
    while (symp->type == ALIAS) {
	symp = symp->list->le_sym;	/* with token of original */
    }
    tp = symp->type;
    return tp >= MAX_GT ? (tp == SH || tp == INPW ? ARN : OR)
			: (tp == UDF || symp->list &&
	((ft = symp->list->le_sym->ftype) == OUTX || ft == OUTW) ? types[symp->ftype]
								 : tp);
} /* bTyp */

/********************************************************************
 *
 *	common code to generate built in iC functions
 *
 *******************************************************************/

List_e *
bltin(Sym* sym, Lis* ae1, Lis* cr1, Lis* ae2, Lis* cr2, Lis* cr3, Val* pVal)
{
    List_e *	lp1;
    List_e *	lp2;
    List_e *	lp3;
    List_e *	lpc;
    List_e *	lpt = 0;

    if (ae1 == 0 || ae1->v == 0) { warn(1); return 0; }	/* YYERROR in fexpr */

    if (ae2) {
	lpc = cr2 ? cr2->v			/* individul clock or timer cr2 */
		  : cr1 ? sy_push((lpt = cr1->v->le_sym->u.blist) ? lpt->le_sym
								  : cr1->v->le_sym)
						/* or clone first clock or timer cr1 */
			: sy_push(clk);		/* or clone default clock iClock */
	if (lpt && lpt->le_sym->type == TIM) {
	    assert(lpt->le_next);		/* clone associated timer value */
	    assert(lpt->le_next->le_val == (unsigned) -1);
	    lpc = op_push(lpc, TIM, sy_push(lpt->le_next->le_sym));
	    lpt = lpc->le_sym->u.blist;
	    assert(lpt && lpt->le_next);
	    lpt->le_next->le_val = (unsigned) -1;/* mark link as timer value */
	}
    }

    if (ae1->v->le_sym->type == LOGC) {		/* DLATCH(set,reset) */
	lp1 = op_push(sy_push(ae1->v->le_sym), LOGC, ae1->v);
	lp1->le_sym->type = LATCH;
	lp1 = op_push(sy_push(sym->v), LATCH, lp1);
    } else {
	lp1 = op_push(sy_push(sym->v), bTyp(ae1->v), ae1->v);
    }
    lp1->le_first = ae1->f; lp1->le_last = ae1->l;
    lp1 = op_push(cr1 ? cr1->v : sy_push(clk), lp1->le_sym->type, lp1);
    lp3 = op_push((List_e *)0, types[lp1->le_sym->ftype], lp1);

    if (ae2) {
	if (ae2->v == 0) { warn(1); return 0; }	/* YYERROR in fexpr */

	lp2 = op_push(sy_push(sym->v), bTyp(ae2->v), ae2->v);
	lp2->le_first = ae2->f; lp2->le_last = ae2->l;
	if (lp2->le_sym->ftype == S_FF) {
	    lp2->le_sym->ftype = R_FF;		/* next ftype for SR flip flop*/
	}
	lp2 = op_push(lpc, lp2->le_sym->type, lp2);
	lp2 = op_push((List_e *)0, types[lp2->le_sym->ftype], lp2);
	lp3 = op_push(lp3, types[lp3->le_sym->ftype], lp2);
    }

    if (cr3) {
	/* extra Master is timed reset fed back from own output */
	lp1 = sy_push(ae1->v->le_sym);	/* use dummy ae1 fill link */

	lp2 = op_push(sy_push(sym->v), UDF, lp1);
	if (lp2->le_sym->ftype == S_FF) {
	    lp2->le_sym->ftype = R_FF;	/* next ftype for SR flip flop*/
	}
	lp2 = op_push(cr3->v, lp2->le_sym->type, lp2);
	lp2 = op_push((List_e *)0, types[lp2->le_sym->ftype], lp2);
	lp3 = op_push(lp3, types[lp3->le_sym->ftype], lp2);

	lp1->le_sym = lp3->le_sym;		/* fix link from own */
    }
    if (pVal) {
	/* cblock for ffexpr */
	lp1->le_val = pVal->v;		/* unsigned int value for case # */
    }
    return lp3;
} /* bltin */

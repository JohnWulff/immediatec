static const char genr_c[] =
"@(#)$Id: genr.c,v 1.11 2000/12/22 19:15:52 jw Exp $";
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

    while (lp && (sp = lp->le_sym)->ftype != ftyp) {
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
	}
#endif
	sp->ftype = ftyp;	/* convert old or new to ftyp */
	break;
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
		lsp->type < MAX_LV)		/* but left is not a clock */
	    {
		left->le_next = sp->u.blist;	/* extend expression */
		sp->u.blist = left;		/* link left of expr */
#ifdef YYDEBUG
		if (debug & 02) {
		    fprintf(outFP, "\t%c%s %c %c%s\n",
			v(left), os[op], v(right));
		}
#endif
	    } else {
		/*
		 * left is a $ symbol and same operator or clock type
		 * merge left into right
		 */
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

    atn = 0;
    sp = var;				/* start reduction with var */
    t_first = rl->f; t_last = rl->l;	/* full text of expression */
    do {				/* marked symbol */
	int	gt_input;
	char *	ep = eBuf;		/* temporary expression buffer */
	if (debug & 04) {
	    fprintf(outFP, "\n");
	}
	gt_input = 0;
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
		if (gp->ftype == ARITH /* && gp->u.blist ZZZ */) {
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
			    continue;	/* continue with next link */
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
		} else if (gp->ftype != GATE) {
		    fprintf(outFP, "\t%s\t%c ---%c", gp->name, fos[gp->ftype],
			os[sp->type]);
		} else {
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
	    if (gp->ftype == ARITH && sp->type == ARN && gp->u.blist) {
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
		/* modify IXx.x and QXx.x names for compiled output only */
		IEC1131(gp->name, buffer, BUFS, iqt, bwx, &byte, &bit, tail);
		ep += sprintf(ep, "_(%s%s)", gp->type == NCONST ? "_" : "", buffer);
		t_first = lp->le_last;	/* skip logic expr's */
	    }
	    if (debug & 04) {
		fprintf(outFP, "\n");
		sflag = debug & 0200;
	    }
	    if (sp == gp && (sp->type != LATCH || lp->le_val != NOT ^ NOT)) {
		warning("input equals output at gate:", sp->name);
	    }
	}
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
	    fprintf(exoFP, cexeString[outFlag], ++c_number);
	    fprintf(exoFP, "#line %d \"%s\"\n", lineno, inpNM);
	    fprintf(exoFP, "	return %s;\n", eBuf);
	    if (outFlag != 0) {
		fprintf(exoFP, "}\n\n");	/* terminate function */
	    }
	    if (debug & 04) fprintf(outFP, "; (%d)\n", c_number);
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
	}
	free(var->name);		/* free name space */
	free(var);			/* temporary Symbol */
	return 0;
    }
    return (sv->v);
} /* op_asgn */

/********************************************************************
 *
 *	return operator to use in built in iC functions
 *
 *******************************************************************/

static uchar
bTyp(List_e * lp)
{
    register Symbol *	symp;
    register uchar	tp;

    symp = lp->le_sym;
    while (symp->type == ALIAS) {
	symp = symp->list->le_sym;	/* with token of original */
    }
    tp = symp->type;
    return tp >= MAX_GT ? (tp == SH || tp == INPW ? ARN : OR)
			: (tp == UDF ? types[symp->ftype] : tp);
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
	    assert(lpt->le_next);		/* clone timer value as well */
	    lpc = op_push(lpc, TIM, sy_push(lpt->le_next->le_sym));
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

static const char genr_c[] =
"@(#)$Id: genr.c,v 1.62 2004/12/22 16:54:22 jw Exp $";
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

int *		functionUse = NULL;	/* database to record function calls */
int		functionUseSize = 0;	/* dynamic size adjusted with realloc */

static Symbol *	templist;		/* temp list of un-named symbols */
static int	ttn;			/* for generating temp f object name */
#if YYDEBUG
static int	tn;
#endif
static char	eBuf[EBSIZE];		/* temporary expression text buffer */
static char *	ePtr = 0;		/* temporary expression buffer pointer */
static char *	eEnd = 0;		/* end of temporary expression buffer */
static char *	t_first = 0;
static char *	t_last = 0;
static char	One[] = "1";		/* name for constant 1 Symbol */
static Symbol *	iCallHead = 0;		/* function head seen at start of imm call */
static Symbol *	varList = 0;		/* list of temp Symbols while compiling function */
static List_e *	stmtList = 0;		/* list of statement Symbols making function body */
static int	ffexprFlag = 0;		/* if - else or switch seen in function */

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
    List_e *		tlp;
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
	if ((lp = sp->v_elist) != 0 && (sp->name == 0
#if YYDEBUG
						    || *(sp->name) == '$'
#endif
	)) {					/* not marked symbol */
	    sp->v_elist = 0;			/* clear the feedback list */
	    while (lp) {			/* list with feedback to this Symbol */
		tlp = lp->le_next;		/* next link to a feedback Symbol */
		sy_pop(lp);			/* delete a feedback list link */
		lp = tlp;
	    }
	}
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
	    if ((lp = lsp->v_elist) != 0 && (lsp->name == 0
#if YYDEBUG
							|| *(lsp->name) == '$'
#endif
	    )) {				/* not marked symbol */
		lsp->v_elist = 0;		/* clear the feedback list */
		while (lp) {			/* list with feedback to this Symbol */
		    tlp = lp->le_next;		/* next link to a feedback Symbol */
		    sy_pop(lp);			/* delete a feedback list link */
		    lp = tlp;
		}
	    }
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
	symp = install(buf, NCONST, ARITH);
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
    /********************************************************************
     * left ^ right === (left & ~right) | (~left & right)
     * the op "UDF" forces a new level in op_push
     * it is then changed to AND 
     * thus a new level is forced even if left or right is AND
     *******************************************************************/
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
	case ALIAS:			/* unresolved alias from negated function */
	    lp->le_val ^= NOT;		/* negate via alias */
	    break;
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
 *	copy arithmetic up to the current expression or constant;
 *	then copy in the indexed macro of the expression or constant
 *
 *******************************************************************/

static void
copyArithmetic(List_e * lp, Symbol * sp, Symbol * gp, int gt_input, int sflag)
{
    if (gp->ftype == ARITH && (sp->type & TM) == ARN && lp->le_val != (unsigned) -1) {
	char *	cp = ePtr;

	if (t_first) {				/* end of arith */
	    assert(t_first >= iCbuf && lp->le_first < &iCbuf[IMMBUFSIZE]);
	    while (t_first < lp->le_first) {
		if (*t_first != '#') {		/* transmogrified '=' */
		    *ePtr++ = *t_first;
		}
		t_first++;
	    }
	}
	*ePtr = 0;
	if (debug & 04) {
	    /* only logic gate or SH can be aux expression */
	    if (sflag == 1) {
		fprintf(outFP, "\t\t\t\t\t");
	    } else
	    if (sflag) {
		assert(sflag == 0200);
		if (sp->ftype == GATE) {
		    putc('\t', outFP);
		}
		putc('\t', outFP);
	    } else {
		fprintf(outFP, "\t\t\t");
	    }
	    fprintf(outFP, "%s%s	// %d",
		cp, gp->name, gt_input);	/* expression till now */
	    if (sflag == 1) {
		fprintf(outFP, "\n");
	    }
	}
	ePtr += snprintf(ePtr, eEnd - ePtr, "_MV(%d)", gt_input);
	t_first = lp->le_last;			/* skip current expression */
    }
} /* copyArithmetic */

/********************************************************************
 *
 *	write the cexeString and record in function use database
 *
 *******************************************************************/

void
writeCexeString(FILE * oFP, int cn)
{
    fprintf(oFP, cexeString[outFlag], cn);
    while (cn >= functionUseSize) {
	functionUse = (int*)realloc(functionUse,
	    (functionUseSize + FUNUSESIZE) * sizeof(int));
	memset(&functionUse[functionUseSize], '\0', FUNUSESIZE * sizeof(int));
	functionUseSize += FUNUSESIZE;
    }
    if (iFunSymExt == 0) {		/* defer freeing till called */
	functionUse[0] |= F_CALLED;	/* flag for copying temp file */
	functionUse[cn]++;		/* free this function for copying */
    }
} /* writeCexeString */

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
    List_e *	nlp;
    List_e *	right;
    List_e **	lpp;
    int		atn;
    int		sflag;
    int		fflag;
    Symbol *	sr;
    int		typ;
    char	temp[TSIZE];

    assert(varList == 0);		/* checks that collectStatement has been executed */
    right = op_force(rl->v, ft);	/* force Symbol on right to ftype ft */
    if (sv == 0) {
	/* null var - generate a temporary Symbol of type UNDEF */
	var = (Symbol *) emalloc(sizeof(Symbol));
	snprintf(temp, TSIZE, "%s_f%d", iFunSymExt ? iFunBuffer : "", ++ttn);
	var->name = emalloc(strlen(temp)+1);	/* +1 for '\0' */
	strcpy(var->name, temp);	/* name needed for derived Sy's */
	sflag = 0;			/* don't output name */
    } else {
	var = sv->v;			/* Symbol * var */
	sflag = 0200;			/* print output name */
    }
    assert(var->name);
    if (iFunSymExt && strncmp(var->name, iFunBuffer, iFunSymExt - iFunBuffer) != 0) {
	ierror("assignment to a variable which does not belong to this function:", var->name);
	var->type = ERR;		/* reduce anyway to clear list */
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
	} else if ((t_first = sv->f) != 0) {
	    assert(t_first >= iCbuf && sv->l < &iCbuf[IMMBUFSIZE]);
	    memset(t_first, '#', sv->l - t_first);	/* mark left var, leave ALIAS */
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
    if ((lp = rsp->v_elist) != 0 && (rsp->name == 0
#if YYDEBUG
						|| *(rsp->name) == '$'
#endif
    )) {				/* not marked symbol */
	rsp->v_elist = 0;		/* clear the feedback list */
	while (lp) {			/* list of Symbols with feedback to this Symbol */
	    sp = lp->le_sym;		/* points to Symbol with feedback */
	    tlp = sp->u_blist;		/* input list */
	    atn = 0;
	    while (tlp) {
		sr = tlp->le_sym;	/* input Symbol */
		if (sr == rsp) {
		    tlp->le_sym = var;	/* change feedback to var - watch negation */
		    atn++;		/* must have 1 feedback */
#if YYDEBUG
		    if ((debug & 0402) == 0402) {
			fprintf(outFP, "*** feedback at %s from %s to %s\n",
			    sr->name ? sr->name : "(null)",
			    sp->name ? sp->name : "(null)",
			    var->name ? var->name : "(null)");
			fflush(outFP);
		    }
#endif
		}
		tlp = tlp->le_next;	/* next input link */
	    }
	    assert(atn);		/* checks there is a feedback */
	    tlp = lp->le_next;		/* next link to a feedback Symbol */
	    sy_pop(lp);			/* delete a feedback list link */
	    lp = tlp;
	}
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
    assert((t_first == 0 || t_first >= iCbuf) && t_last < &iCbuf[IMMBUFSIZE]);
#if YYDEBUG
    if ((debug & 0402) == 0402) fprintf(outFP, "resolve \"%s\" to \"%s\"\n", t_first, t_last);
#endif
    do {				/* marked symbol */
	List_e*	saveBlist = 0;		/* prevent warning - only used when iFunSymExt != 0 */
	List_e*	plp;
	int	gt_input;
	char *	cp;
	int	cFn = 0;

	ePtr = eBuf;			/* temporary expression buffer pointer */
	eEnd = &eBuf[EBSIZE];		/* end of temporary expression buffer */
#if YYDEBUG
	if ((debug & 0402) == 0402) {
	    memset(eBuf, '\0', EBSIZE);
	}
#endif
	if (debug & 04) {
	    fprintf(outFP, "\n");
	}
	gt_input = 0;			/* output scan for 1 gate */
	if (iFunSymExt) saveBlist = sp->u_blist;
	lp = 0;
	fflag = (sp->type == SW || sp->type == CF) ? 1 : 0;
	while ((plp = lp, lp = sp->u_blist) != 0) {
	    sp->u_blist = lp->le_next;
	    if ((gp = lp->le_sym) == rsp && var->type != ALIAS) {
		gp = var;		/* link points to right */
	    }
	    if ((nlp = gp->v_elist) != 0 && (gp->name == 0
#if YYDEBUG
							|| *(gp->name) == '$'
#endif
	    )) {				/* not marked symbol */
		/********************************************************************
		 * A feedback element has occurred in the middle of an expression
		 * reduction. Since this element will not be assigned to a differnt
		 * var, the feedback list can be removed now.
		 *******************************************************************/
		gp->v_elist = 0;		/* clear the feedback list */
		while (nlp) {			/* list with feedback to this Symbol */
		    tlp = nlp->le_next;		/* next link to a feedback Symbol */
		    sy_pop(nlp);		/* delete a feedback list link */
		    nlp = tlp;
		}
	    }
	    if (! fflag || ! plp) {
		while (gp->type == ALIAS) {	/* not for var ALIAS */
		    if ((tlp = gp->list) == 0 && (tlp = gp->u_blist) == 0) {
			assert(0);			/* ALIAS without left or right link */
		    }
		    lp->le_val ^= tlp->le_val;	/* negate if necessary */
		    gp = tlp->le_sym;		/* point to original */
		}
		if (iFunSymExt && (debug & 020000) &&
		    gp->type == LATCH && gp->ftype == D_FF && gp->list == 0) {
		    gp->list = sy_push(sp);	/* extra link for feedback */
		}
		if (sp == gp && sp->ftype >= MIN_ACT) {
		    if (sp->type == LATCH && sp->ftype == D_FF) {
			assert(sp->list);
			gp = sp->list->le_sym;	/* feedback via D output */
			if (iFunSymExt) {
			    sy_pop(sp->list);	/* delete extra link for feedback */
			    sp->list = 0;	/* restore feedback head */
			}
		    } else if (gt_input || sp->type != LATCH) {
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
			sp->type = OR;		/* OR default for 1 input */
			gp->next = templist;	/* put at front of list */
			templist = gp;
			gp->u_blist = tlp = sy_push(gp);	/* link self */
			tlp->le_next = sp->u_blist;		/* rest of inputs on sp */
			sp->u_blist = 0;			/* this link scan now complete */
			if (iFunSymExt) {
			    lp->le_sym = gp;		/* for function definition */
			    lp->le_next = 0;
			}
		    }
		}
		if (gp->ftype == ARITH &&		/* && gp->u_blist ZZZ */
		    sp->ftype != OUTW &&
		    sp->type != ALIAS) {
		    int	val;
		    if ((val = lp->le_val) == (unsigned) -1) {
			gt_input--;			/* delay is not an arithmetic input */
		    } else if (val == 0){
			lp->le_val = ((c_number + 1) << 8)	/* arithmetic case number */
				     + gt_input + 1;	/* arithmetic input number */
		    } else {
			assert((val & 0xff) == gt_input+1);	/* input number must match definition */
			val >>= 8;
			if (cFn == 0) {
			    cFn = val;			/* case number from function */
			    if (iFunSymExt == 0) {
				assert(cFn < functionUseSize); /* adjusted when function generated */
				functionUse[0] |= F_CALLED; /* flag for copying temp file*/
				functionUse[cFn]++;	/* free this function for copying */
			    }
			} else {
			    assert(cFn == val);		/* other members must be the same */
			}
		    }
		}
		if (iFunSymExt == 0) {			/* global reduction only */
		    if (sp->type == ALIAS) {
			/* var was made an ALIAS because of FF negation */
			for (lpp = &gp->list; (tlp = *lpp) != 0; ) {
			    lpp = &tlp->le_next;
			}
			*lpp = tlp = sp->list;		/* start of left list */
			while (tlp) {
			    if (sp->ftype == GATE) {
				tlp->le_val ^= lp->le_val;	/* negate if necesary */
			    }
			    tlp = tlp->le_next;		/* scan to end of ALIAS list */
			}
			sp->list = lp;			/* link ALIAS to right */
		    } else {
			/* link Symbols to the end of gp->list to maintain order */
			if (! fflag) {
			    lp->le_next = 0;		/* unless if - else or switch */
			}
			if ((tlp = gp->list) == 0) {
			    gp->list = lp;		/* this is the first Symbol */
			} else {
			    /* loop to find duplicate link (possibly inverted) */
			  loop:					/* special loop with test in middle */
			    if (tlp->le_sym == sp) {
				if (gp->ftype == OUTW || gp->ftype == OUTX) {
				    warning("input equals output at output gate:", gp->name);
				} else if (gp->ftype == ARITH || tlp->le_val == lp->le_val) {
				    copyArithmetic(lp, sp, gp, tlp->le_val & 0xff, 1);
				    sy_pop(lp);		/* ignore duplicate link */
				    goto nextInputLink;
				} else {
				    warning("gate has input and inverse:", gp->name);
				}
			    }
			    if (tlp->le_next) {
				tlp = tlp->le_next;	/* find last Symbol */
				goto loop;
			    }
			    tlp->le_next = lp;		/* put new Symbol after last */
			}
			lp->le_sym = sp;		/* completely symmetrical */
		    }
		} else					/* function reduction */
		if (sp->type != ALIAS) {		/* ALIAS linked to u_blist is OK */
		    /* test very carefully so no global Symbols are linked */
		    if (gp->list == 0 && gp->u_blist && gp->type == ARN) {
			gp->list = lp;			/* reverse first link for arith text */
		    }
		    for (tlp = saveBlist; tlp && tlp != lp; tlp = tlp->le_next) {
			if (tlp->le_sym == gp) {
			    if (gp->ftype == OUTW || gp->ftype == OUTX) {
				warning("input equals output at output gate:", gp->name);
			    } else if (gp->ftype == ARITH || tlp->le_val == lp->le_val) {
				assert(plp);	/* cannot be first link in chain */
				plp->le_next = sp->u_blist;	/* relink the chain */
				copyArithmetic(lp, sp, gp, tlp->le_val & 0xff, 1);
				sy_pop(lp);		/* ignore duplicate link */
				goto nextInputLink;
			    } else {
				warning("gate has input and inverse:", gp->name);
			    }
			}
		    }
		}					/* end of function reduction */
	    }
	    if ((gp->ftype) < MIN_ACT) {		/* recognise UDF clock & timer */
		gt_input++;				/* count the gate inputs */
	    }
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
		if (fflag && plp) {
		    /* reference to a C fragment variable */
		    assert(lp->le_val & 0xff);
		    fprintf(outFP, "\t%s\t%c<---%c\t\t\t// %d", gp->name, fos[gp->ftype],
			os[sp->type & TM], lp->le_val & 0xff);
		} else
		if ((typ = gp->type & TM) >= MAX_LV) {
		    fprintf(outFP, "\t%s\t%c ---%c", gp->name, os[typ],
			os[sp->type & TM]);
		} else
		if (gp->ftype < MAX_AR && lp->le_val == (unsigned) -1) {
		    /* reference to a timer value - no link */
		    fprintf(outFP, "\t%s\t%c<---%c", gp->name, fos[gp->ftype],
			os[sp->type & TM]);
		} else
		if (gp->ftype != GATE) {
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
		if (gp->ftype == F_SW || gp->ftype == F_CF || gp->ftype == F_CE) {
		    /********************************************************************
		     * case number of "if" or "switch" C fragment
		     *******************************************************************/
		    fprintf(outFP, " (%d)", lp->le_val >> 8);
		} else
		if ((gp->ftype == TIMR && lp->le_val > 0)) {
		    /********************************************************************
		     * Timer preset off value
		     *******************************************************************/
		    fprintf(outFP, " (%d)", lp->le_val);
		}
	    }
	    copyArithmetic(lp, sp, gp, gt_input, sflag); /* with current expression */
	    if (debug & 04) {
		fprintf(outFP, "\n");
		sflag = debug & 0200;
	    }
	    if (sp == gp && (sp->type != LATCH || lp->le_val != (NOT^NOT))) {
		warning("input equals output at gate:", sp->name);
	    }
	    nextInputLink: ;
	}						/* end output scan for 1 gate */
	if (iFunSymExt) sp->u_blist = saveBlist;
	if ((sp->type & TM) == ARN) {
	    /********************************************************************
	     * copy rest of arithmetic expression and finalise C-Code
	     *******************************************************************/
	    cp = ePtr;
	    if (t_first) {
		assert(t_first >= iCbuf && t_last < &iCbuf[IMMBUFSIZE]);
		while (t_first < t_last) {
		    if (*t_first != '#') {		/* transmogrified '=' */
			*ePtr++ = *t_first;
		    }
		    t_first++;
		}
	    }
	    *ePtr++ = 0;
							/* start case or function */
	    if (sp->ftype != OUTW) {			/* output cexe function */
		if (cFn == 0) {
		    if (strcmp(eBuf, "_MV(1)") == 0) {
			assert(strlen(cp) == 0);
			plp->le_val = 0;		/* correct _l[] generation */
		    } else {
			functionUse[0] |= F_ARITHM;	/* flag for copying macro */
			writeCexeString(T1FP, ++c_number); /* and record for copying */
			fprintf(T1FP, "#line %d \"%s\"\n", lineno, inpNM);
			fprintf(T1FP, "	return %s;\n", eBuf);
			fprintf(T1FP, "%%##\n%s", outFlag ? "}\n\n" : "\n");
			if (debug & 04) fprintf(outFP, "\t\t\t\t\t%s;	// (%d)\n", cp, c_number);
		    }
		} else if (debug & 04) fprintf(outFP, "\t\t\t\t\t;	// (%d)\n", cFn);
	    }
	}
	sflag = 0200;					/* print output name */
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
		    if (lookup(sp->name) == 0) {
			(void) place_sym(sp);/* place sp in the symbol table (changes sp->next) */
		    }
		} else {
		    sp->next = varList;	/* put newly marked Symbol on function varList */
		    varList = sp;	/* which is used to reconstitute templist when cloned */
		}
		if ((sp->type & TM) == ARN) {
		    assert(sp->list);
		    t_first = sp->list->le_first;
		    t_last = sp->list->le_last;
		}
	    }
	}
    } while (sp);			/* 1 symbol resolved */
    /********************************************************************
     * right Symbol must remain intact until after reduction, because
     * there may be a reference to it which is tested in if(gp == rsp).
     * When rsp is freed before reduction, rsp may point to a newly
     * generated Symbol, which is not a reference to right Symbol.
     * Therefore free here.
     *******************************************************************/
    if (right->le_val == (NOT^NOT)) {
	sy_pop(right);			/* right Symbol and List_e */
#if YYDEBUG
	if ((debug & 0402) == 0402) {
	    free(rsp->name);		/* free name space of $x */
	}
#endif
	free(rsp);			/* free right Symbol */
    }
    if (debug & 04) fprintf(outFP, "\n");
    /********************************************************************
     * A Symbol is marked by storing a pointer value in ->name
     * which points to a string which does not start with $.
     * Only marked symbols are reduced.
     * Any remaining new Symbols on 'templist' must belong to an outer
     * assignment which will be reduced later.
     *******************************************************************/
    if ((t_first = rl->f) != 0) {	/* full text of expression */
	assert(t_first >= iCbuf && rl->l < &iCbuf[IMMBUFSIZE]);
	memset(t_first, '#', rl->l - t_first);	/* mark embedded assignments */
    }
    if (iFunSymExt) {
	var->list = var->u_blist;	/* save in ->list till end of function definition */
	var->u_blist = 0;		/* must be cleared for op_push like normal op_asgn */
	if (var->type == SW || var->type == CF) {
	    ffexprFlag++;		/* suppress undefined warning in functions till end */
	}
    } else
    if (sv == 0) {
	lp = sr->list;			/* link action to temp */
	assert(lp && lp->le_sym == var);
	lp->le_sym = 0;			/* erase reference to temp either way */
#if YYDEBUG
	if ((debug & 0402) == 0402) {
	    fprintf(outFP, "\t  %s deleted\n\n", var->name);
	    fflush(outFP);
	}
#endif
	free(var->name);		/* free name space */
	free(var);			/* temporary Symbol */
	var = 0;
    }
    return var;
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
 *	also handles clock and timer CVAR or TVAR
 *	sv	assign to this Symbol
 *	lv	expression to assign
 *	ioTyp	0	for LVAR or AVAR
 *		OUTX	for LOUT
 *		OUTW	for AOUT
 *		CLK	for CVAR
 *		TIM	for TVAR
 *
 *******************************************************************/

Symbol *
assignExpression(Sym * sv, Lis * lv, int ioTyp)
{
    Sym		sy;
    Lis		li;
    Symbol *	rsp;
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
	if (iFunSymExt) sv->v->list = 0;	/* do 2nd assignment for listing */
    }
    rsp = (debug & 020000) ? sv->v
			   : op_asgn(sv, lv, ftyp);	/* new code before Output */
    if (ioTyp >= MAX_ACT) {			/* OUTW or OUTX */
	snprintf(temp, TSIZE, "%s_0", rsp->name);
	if ((sy.v = lookup(temp)) == 0) {	/* locate position in ST */
	    sy.v = install(temp, UDF, ioTyp);	/* generate output Gate OUTX or OUTW */
	    li.v = sy_push(rsp);		/* provide a link to LOUT or AOUT */
	    if ((li.v = op_push(0, types[ftyp], li.v)) != 0) {
		li.v->le_first = li.f = 0; li.v->le_last = li.l = 0;
	    }
	    op_asgn(&sy, &li, ftyp);		/* Output assignment */
	}
    }
    if (debug & 020000) rsp =  op_asgn(sv, lv, ftyp);	/* old code after Output */
    if (iFunSymExt) collectStatement(rsp);	/* update varList in definition stmtList */
    return rsp;
} /* assignExpression */

/********************************************************************
 *
 *	Implicit delay of 1 tick for ctref : texpr ; without delay
 *	(cexpr|texpr[,dexpr])
 *
 *******************************************************************/

List_e *
delayOne(List_e * tp)
{
    Symbol *	sp;				/* with implicit delay of 1 */
    List_e *	lp;
    if ((sp = lookup(One)) == 0) {
	sp = install(One, NCONST, ARITH);
    }
    lp = sy_push(sp);
    lp->le_val = (unsigned) -1;			/* mark link as timer value */
    return op_push(tp, TIM, lp);
} /* delayOne */

/********************************************************************
 ********************************************************************
 *
 * class DefineFunction
 *
 *	Code for generating the template for an immediate function.
 *
 * Algorithm:
 *	The UNDEF function head Symbol for a function definition
 *	called <fName> is marked with type IFUNCT which will return
 *	YACC type VFUNCTION, IFUNCTION, CFUNCTION, or TFUNCTION from iClex
 *	when the function name <fName> later appears as a function call in
 *	subsequent code.
 *
 *	A void function VFUNCTION is returned if the ftype is UDFA. For
 *	a value function IFUNCTION is returned for ftypes ARITH or GATE
 *	and CFUNCTION or TFUNCTION for CKLCK or TIMRL respectively. For
 *	a value function called "fun", a Symbol named "fun$" is installed
 *	and made known via Sym iRetSymbol to the return statement.
 *	A text buffer iFunBuffer holds the text "fun$" and a pointer
 *	iFunSymExt is set to point just past the $ symbol.
 *
 *	When scanning the formal parameter list or when declaring immediate
 *	variables in the body of a function (signalled by iFunSymExt
 *	being set), the name of the item is concatenated to the "fun$"
 *	string and a new undefined Symbol with this extended name is
 *	installed in the Symbol Table. That puts the formal parameter
 *	names and variables declared in a function in a private name space
 *	for that function. Only the function name is global and may not
 *	clash with any other global name.
 *
 *	The Symbols in the parameter list are linked to 'list' of the
 *	function head Symbol. A linked statement list of declarations and
 *	assignments are linked to 'blist' of the function head Symbol.
 *	These are the possible return Symbol, any assignable parameters
 *	and variables declared inside the function. When the function
 *	definition has been completely compiled, both lists are checked to
 *	see if they contain any undefined Symbols. If this is the case, an
 *	assignment is missing in the function.
 *
 *	Each assignment head has an expression net linked to it, which will
 *	link back to value parameters (or assignment parameters or even the
 *	return Symbol used as values) or function internal or global immediate
 *	variables. The expression net is unresolved at this stage.
 *
 *	Any global variables (even ones finally declared and assigned in the
 *	same file), which are to be used as values in the expressions of a
 *	function, must be declared extern in the function. This means that
 *	in a function, any variable must be declared before it is used. The
 *	possible options are:
 *	1)  imm <type> functionName(<parameter list>) for value functions
 *	    the name of the function return object in expressions is "this".
 *	2a) assign <type> <name> for an assign parameter or
 *	2b) <type> <name> for a value parameter, both in the comma seperated
 *	    parameter list.
 *	3)  imm <type> <name> in the body of the function declares a function
 *	    internal variable which will be cloned with different names in
 *	    different instances, when the function is called.
 *	4)  extern <type> <name> means that name will be assigned outside the
 *	    function. Such a global variable may not be declared or assigned
 *	    before its declaration in the function. After the function definition
 *	    it must be declared in the source in which it will be assigned.
 *	The upshot is, that any variable used in a function must be declared
 *	before it is used in a function. This was found to be more important
 *	for functions, where bugs can easily creep in when one assumes a
 *	certain variable is function internal, but was not declared as such
 *	and it is then accidentally compiled as global. This would not become
 *	evident until the function is used. It also makes the use of global
 *	variables in a function a little bit more onerous, which it should be,
 *	since their use can cause loss of a broad overview. By having the extern
 *	declaration, at least one knows which names are used for what in each
 *	function. Breaking this rule will cause a warning - not an error.
 *
 ********************************************************************
 *******************************************************************/

/********************************************************************
 *
 *	Set up the function definition head Symbol before any parameters
 *	have been parsed.
 *
 *	iFunBuffef contains the function head name for the whole of the
 *		   definition.
 *	iFunSymExt points past the $ in the name and is the place where
 *		   local name extensions can temprarily be written.
 *		   It also flags, that a function is being defined.
 *
 *	For a value function also set up the return Symbol
 *
 *******************************************************************/

Symbol *
functionHead(unsigned int typeVal, Symbol * funTrigger, int retFlag)
{
    unsigned char	ftyp = typeVal & 0xff;	/* UDFA GATE ARITH CLCKL TIMRL */

    funTrigger->type = IFUNCT;			/* function head */
    funTrigger->ftype = ftyp;			/* void bit int clock timer */
    iFunSymExt = strncpy(iFunBuffer, funTrigger->name, IBUFSIZE);
    iFunSymExt += strlen(iFunBuffer);		/* point past text */
    if (iFunEnd - iFunSymExt < 32) {
	execerror("iFunBuffer for function symbol too small", funTrigger->name, __FILE__, __LINE__);
    }
    *iFunSymExt++ = '$';			/* append '$' */
    if (retFlag) {
	if ((iRetSymbol.v = lookup(iFunBuffer)) == 0) {
	    iRetSymbol.v = install(iFunBuffer, UDF, ftyp);	/* return Symbol */
	}
#if YYDEBUG
	if ((debug & 0402) == 0402) {
	    fprintf(outFP, "iFunHead: imm %s %s\n",
		full_ftype[ftyp], iRetSymbol.v->name);
	    fflush(outFP);
	}
#endif
    } else {
	iRetSymbol.v = 0;			/* void function has no return Symbol */
#if YYDEBUG
	if ((debug & 0402) == 0402) {
	    fprintf(outFP, "vFunHead: imm %s %s\n", full_ftype[ftyp], funTrigger->name);
	    fflush(outFP);
	}
#endif
    }
    assert(stmtList == 0);			/* function definitions cannot nest */
    return funTrigger;
} /* functionHead */

/********************************************************************
 *
 *	Collect statements in the function body
 *
 *	this function is also called for cloned statements from a
 *	nested function call
 *
 *******************************************************************/

List_e *
collectStatement(Symbol * funcStatement)
{
    List_e *		slp;			/* link for statement Symbol */
    List_e *		vlp;			/* link for varList */
    Symbol *		sp;
    unsigned char	typ;

    if ((sp = funcStatement) != 0) {		/* miss a void function call */
	if ((typ = sp->type) <= MAX_OP) {
#if YYDEBUG
	    if ((debug & 0402) == 0402) {
		fprintf(outFP, "collectStatement: %s type: %s, ftype: %s\n",
		    sp->name, full_type[typ&TM], full_ftype[sp->ftype]);
		fflush(outFP);
	    }
#endif
	    if (stmtList == 0) {		/* list reset for each new function */
		stmtList = slp = sy_push(sp);	/* first link in statement list */
		slp->le_next = sy_push(varList);
	    } else {
		slp = stmtList;
		vlp = slp->le_next;
		while (slp->le_sym != sp && vlp->le_next) {
		    slp = vlp->le_next;
		    vlp = slp->le_next;
		    assert(vlp);
		}
		if (slp->le_sym != sp) {	/* new statement Symbol ? */
		    vlp->le_next = slp = sy_push(sp);	/* yes */
		    slp->le_next = sy_push(varList); /* link to end of list */
		} else if (varList) {
		    vlp->le_sym = varList;	/* not set in declaration */
		}
	    }
	} else if ((typ & EM) == 0 && typ != ERR) {	/* ignore extern */
	    fprintf(outFP, "type: %s, ftype: %s\n",
		full_type[typ&TM], full_ftype[sp->ftype]);
	    ierror("function statement is not int, bit, clock or timer:", sp->name);
	    sp->type = (sp->type & FM) | ERR;
	}
	varList = 0;				/* ready for next function assignment */
    }
    return stmtList;
} /* collectStatement */

/********************************************************************
 *
 *	Value function return statement
 *
 *******************************************************************/

Symbol *
returnStatement(Lis * actexpr)
{
    List_e *	lp;
    Symbol *	sp;
    int		retType;

    if (iRetSymbol.v) {
	retType = iRetSymbol.v->ftype;
	if ((lp = actexpr->v) == 0 || (sp = lp->le_sym) == 0) {
	    ierror("no expression to return:", iFunBuffer);
	} else {
	    switch (retType) {
	    case ARITH:
	    case GATE:
		if (sp->ftype != ARITH && sp->ftype != GATE) {
		    ierror("wrong return type for int or bit:", iFunBuffer);
		}
		break;
	    case CLCKL:
	    case TIMRL:
		if (sp->ftype != retType) {
		    ierror("wrong return type for clock or timer:", iFunBuffer);
		}
		break;
	    default:
		ierror("wrong type for function return:", iFunBuffer);
		break;
	    }
	}
	sp = op_asgn(&iRetSymbol, actexpr, retType);
    } else {
	sp = 0;
	ierror("return statement in void function:", iFunBuffer);
    }
    return sp;
} /* returnStatement */

/********************************************************************
 *
 *	Finalise the function definition after parameters and function
 *	body have been parsed.
 *
 *******************************************************************/

Symbol *
functionDefinition(Symbol * iFunHead, List_e * fParams)
{
    List_e *	lp;
    Symbol *	sp;
    int		instanceNum;			/* save early union u.val u.blist */
    int		saveCount = 0;			/* count parameter links for saving */

    instanceNum = iFunHead->u_val;		/* read from file if -a option */
    if (iRetSymbol.v) {
	if (iRetSymbol.v->type == UDF) {
	    ierror("no return statement in function:", iFunHead->name);
	}
	assert(iRetSymbol.v->u_blist == 0);	/* state after expression reduction */
	iRetSymbol.v->u_blist = iRetSymbol.v->list;
	iRetSymbol.v->type |= FM;		/* mark return Symbol */
	iRetSymbol.v->list = sy_push(iFunHead);	/* with own function head */
	iRetSymbol.v = 0;			/* no need to report as undefined */
    }
    iFunSymExt = 0;				/* end of function compilation */
    iFunHead->u_blist = lp = stmtList;		/* global value */
    stmtList = 0;
    while (lp) {				/* mark function statement list */
	sp = lp->le_sym;			/* assign and internal declarations */
	assert(sp);
	if (sp->type == UDF && ffexprFlag == 0) { /* assigns are marked twice */
	    warning("undefined gate in function:", sp->name);
	}
	if (sp->u_blist) {
	    assert(sp->list && sp->list->le_sym == iFunHead);	/* marked return */
	} else {
	    sp->u_blist = sp->list;		/* expression now ready for cloning */
	    sp->type |= FM;			/* mark as function Symbol */
	    sp->list = 0;			/* clear pointer to real Symbol */
	    saveCount++;			/* space for saving nested decl */
	}
	lp = lp->le_next;			/* next varList link */
	assert(lp);				/* statement list is in pairs */
	sp = lp->le_sym;			/* first varList Symbol */
	while (sp) {				/* varList my be empty */
	    free(sp->name);			/* free name space generated for listing name */
	    sp->name = 0;			/* mark the Symbol as function internal (no '$') */
	    sp->list = 0;			/* clear internal Symbol pointers */
	    sp = sp->next;			/* next varList Symbol */
	}
	lp = lp->le_next;			/* next statement link */
    }
    iFunHead->list = lp = fParams;		/* yacc stack value */
    while (lp) {				/* mark formal parameter list */
	sp = lp->le_sym;			/* assign and value parameters */
	assert(sp);
	if (sp->type == UDF && ffexprFlag == 0) {
	    warning("undefined gate in function:", sp->name);
	}
	if (sp->list) {				/* link for real parameters */
	    ierror("trying to assign to a value parameter:", sp->name);
	}
	sp->list = 0;				/* clear pointer to real parameter */
	sp->type |= FM;				/* mark as function Symbol */
	saveCount++;				/* space for saving nested para */
	lp = lp->le_next;			/* next formal parameter link */
    }
    lp = iFunHead->u_blist;	/* save 2 numbers in first elements of statement list */
    if (lp) {			/* any sensible function has at least 1 statement */
	lp->le_val = instanceNum;		/* allows call to number instances */
	lp = lp->le_next;			/* first varList link */
	assert(lp);				/* must be a pair */
	lp->le_val = saveCount;			/* allows call to get save block */
    } else {
	ierror("function has no statements!", iFunHead->name);
    }
#if YYDEBUG
    if ((debug & 0402) == 0402) {
	fprintf(outFP, "iFunDef:  imm %s %s\n", full_ftype[iFunHead->ftype], iFunHead->name);
	fflush(outFP);
    }
#endif
    return iFunHead;				/* yacc stack value */
} /* fuctionDefinition */

/********************************************************************
 *
 *	Clear a previous function definition by deleting all its data
 *	objects
 *
 *******************************************************************/

Symbol *
clearFunDef(Symbol * functionHead)
{
    List_e *		lp;
    List_e *		slp;
    List_e *		vlp;
    List_e *		lp1;
    Symbol *		sp;
    Symbol *		vsp;
    int			instanceNum;		/* restore instance number later */

    /********************************************************************
     * Pass 1: statement list
     *******************************************************************/
    slp = functionHead->u_blist;		/* start of statement list */
    if (slp == 0) {
	return functionHead;			/* already cleared */
    }
    warning("existing function definition is deleted:", functionHead->name);
    functionHead->u_blist = 0;			/* clear for next definition */
    instanceNum = slp->le_val;			/* this function call instance number */
    while (slp) {
	sp = slp->le_sym;			/* formal satement head Symbol */
	assert(sp);
	lp = sp->u_blist;				/* cloned expression links */
	while (lp) {
	    lp1 = lp->le_next;			/* next expression link */
	    free(lp);				/* delete expression link */
	    lp = lp1;
	}
	lp = sp->list;			/* possible link to function head */
	while (lp) {
	    lp1 = lp->le_next;			/* next expression link */
	    free(lp);				/* delete expression link */
	    lp = lp1;				/* has no follow ups - but just in case */
	}
	unlink_sym(sp);				/* unlink Symbol from symbol table */
	free(sp->name);
	free(sp);				/* delete formal statement head Symbol */
	vlp = slp->le_next;			/* next varList link */
	free(slp);				/* delete statement link */
	assert(vlp);				/* statement list is in pairs */
	vsp = vlp->le_sym;			/* varList of temp Symbols */
	while (vsp) {				/* varList may be empty */
	    lp = vsp->u_blist;				/* cloned expression links */
	    while (lp) {
		lp1 = lp->le_next;			/* next expression link */
		free(lp);				/* delete expression link */
		lp = lp1;
	    }
	    sp = vsp->next;			/* next varList Symbol */
	    free(vsp);				/* delete varList Symbol which has no name */
	    vsp = sp;				/* and is not in the Symbol Table */
	}
	slp = vlp->le_next;			/* next statement link */
	free(vlp);				/* delete varList link */
    }
    /********************************************************************
     * Pass 2: parameter list
     * links to real parameters were deleted at the end of a call
     *******************************************************************/
    slp = functionHead->list;		/* parameter list */
    functionHead->list = 0;			/* clear for next definition */
    while (slp) {
	sp = slp->le_sym;			/* formal parameter Symbol */
	assert(sp && sp->list == 0);		/* call leaves link to real para cleared */
	unlink_sym(sp);				/* unlink Symbol from symbol table */
	free(sp->name);
	free(sp);				/* delete formal parameter Symbol */
	vlp = slp->le_next;
	free(slp);				/* delete formal parameter link */
	slp = vlp;
    }
    functionHead->u_val = instanceNum;		/* in case instances of old definition */
    functionHead->ftype = UDFA;			/* in case new def has different TYPE */
    /********************************************************************
     * now ready for a new definition
     *******************************************************************/
    return functionHead;
} /* clearFunDef */

/********************************************************************
 *
 *	Check if a terminal Symbol in a function expression is declared
 *	extern if global.
 *
 *	Normal global variables must be declared extern in or before
 *	use in a function.  Like all variables declared extern, these
 *	must be declared again simply before they are assigned.
 *
 *	I/O variables are universally global and do not need to be
 *	declared extern (Input variables cannot be declared extern)
 *	Output variables can be declared extern, but don't need to be
 *	because if they are they need to be declared again, which would
 *	be confusing. This is only done if Output variables are assigned
 *	in a different source. The the source in which they are used
 *	need an extern declaration to prevent them being flagged as
 *	undefined.
 *
 *	iClock ia a universal global variable which needs no external
 *	declaration.
 *
 *******************************************************************/

List_e *
checkDecl(Symbol * terminal)
{
    int		typ;

    if (iFunSymExt &&
	strncmp(terminal->name, iFunBuffer, iFunSymExt - iFunBuffer) &&
	((typ = terminal->type) & EM) == 0 &&
	typ != INPX &&
	typ != INPW &&
	terminal != iclock) {
	warning("global variable not declared extern:", terminal->name);
    }
    return sy_push(terminal);			/* link for expression expansion */
} /* checkDecl */

/********************************************************************
 ********************************************************************
 *
 * class CallFunction
 *
 *	Code for cloning a copy of an immediate function in a call.
 *	The copy may be a directly executable immediate connection net
 *	or part of another function definition, when that definition
 *	contains a nested call.
 *
 *	save global variables for nested function calls
 *	save all links in parameter and statement lists for recursive calls
 *
 * Function Calls:
 *	When a void or value function is called, its real parameter list
 *	is scanned and type checked against the list of formal prameters
 *	linked to the function head Symbol. Links from formal to real
 *	parameters and statement heads are via 'list' in the formal Symbol,
 *	which are marked with type|FM. The return Symbol 'list' is linked back
 *	to the function head Symbol.
 *
 *	If that is OK, the assignment head list is scanned and each associated
 *	net is cloned, replacing any value parameters by links to the real
 *	parameter Symbols. For each assignment head the unfinished assignment
 *	is now carried out with the cloned net, which gives temporary names
 *	associated with each real assignment variable to internal Symbols.
 *	Also the full logic of assigning to variables which have already been
 *	used as values is thus taken care of. Internal Variables do not have
 *	a link in 'list', and a Symbol to assign to must be cloned for them.
 *
 ********************************************************************
 *******************************************************************/

struct sF {
    struct sF *	saveFunBs;		/* base of previous save block or 0 */
    Symbol *	iCallHead;		/* function head seen at start of imm call */
    List_e *	iFormNext;		/* next pointer when scanning formal parameters */
    List_e *	iFunClock;		/* temporary list of unresolved clock parameters */
    List_e *	iSav[1];		/* array for saving parameter links */
};

static struct sF *	saveFunBs = 0;	/* base pointer to save block for recursive calls */
static List_e *		iFormNext = 0;	/* next pointer when scanning formal parameters */
static List_e *		iFunClock = 0;	/* temporary list of unresolved clock parameters */
static Symbol *		cloneSymbol(Symbol * sp);
static List_e *		cloneList(List_e * slp, Symbol * csp, Symbol * rsp);

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

    lp = functionHead->u_blist;
    if (lp == 0) {
	return 0;				/* report error later - no popFunCall */
    }
    lp = lp->le_next;				/* first varList link */
    assert(lp);					/* must be a pair */
    saveCount = lp->le_val;			/* allows call to store save block */
    saveFunBs = (struct sF *) emalloc(sizeof(struct sF) - 1 + saveCount * sizeof(void *));
    saveFunBs->saveFunBs = oldSFunBs;
    saveFunBs->iCallHead = iCallHead;		/* will be set up during the call */
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
    lp = functionHead->list;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;
	assert(sp);
	saveFunBs->iSav[cF++] = sp->list;	/* save parameter template link */
	lp = lp->le_next;
    }
    /********************************************************************
     * Pass 2: statement list
     *******************************************************************/
    lp = functionHead->u_blist;			/* statement list */
    while (lp) {
	sp = lp->le_sym;
	assert(sp);
	sp1 = ((lp1 = sp->list) != 0) ? lp1->le_sym : 0;
	if (sp1 != functionHead) {		/* bypass return link */
	    saveFunBs->iSav[cF++] = sp->list;/* save declaration or assign link */
	    sp->list = 0;			/* clear link */
	}
	lp = lp->le_next;			/* next varList link */
	assert(lp);				/* statement list is in pairs */
	lp = lp->le_next;			/* next statement link */
    }
    assert(cF == saveCount);
    /********************************************************************
     * Pass 3: parameter list again - clear links now
     *******************************************************************/
    lp = functionHead->list;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;
	sp->list = 0;			/* clear parameter link */
	lp = lp->le_next;
    }
    /********************************************************************
     * set up for cloning
     *******************************************************************/
    iCallHead = functionHead;			/* make avalable globally */
    assert(iCallHead);				/* must have a function head */
    iFormNext = iCallHead->list;		/* first formal parameter - may be 0 */
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
		rsp = install(One, NCONST, ARITH);
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
		sy_pop(rlp);			/* parameter not used */
		rlp = 0;			/* error */
	    }
	    if (rlp) {
		psp = plp->le_sym;		/* previous timer Symbol */
		assert(psp->ftype == TIMRL);
		/********************************************************************
		 * when cloning formal clock, this link and ftype will indicate timer
		 * which must be cloned differently - action gate must be linked to
		 * le_next of delay link rather than to clock link which is now timer
		 *******************************************************************/
		lp = plp->le_next = rlp;	/* install delay with previous timer */
		rlp->le_val = (unsigned) -1;	/* mark link as timer value */
		while (iFunClock) {		/* find unresolved formal clock parameters */
		    iFunClock->le_sym = psp;	/* link unresloved clock to this timer */
		    clp = iFunClock->le_next;	/* next unresolved clock */
		    iFunClock->le_next = lp1 = sy_push(rsp); /* clone a delay link */
		    lp1->le_val = (unsigned) -1; /* mark link as timer value */
		    iFunClock = clp;
		}
	    }
	} else {
	    switch (formalType) {
	    case ARITH:
		if (rsp->ftype == GATE) {
		    rlp = op_force(rlp, ARITH);	/* force to int */
		} else
		if (rsp->ftype != ARITH) {
		    ierror("wrong parameter type for int:", rsp->name);
		    sy_pop(rlp);		/* parameter not used */
		    rlp = 0;			/* error */
		}
		break;
	    case GATE:
		if (rsp->ftype == ARITH) {
		    rlp = op_force(rlp, GATE);	/* force to bit */
		} else
		if (rsp->ftype != GATE) {
		    ierror("wrong parameter type for bit:", rsp->name);
		    sy_pop(rlp);		/* parameter not used */
		    rlp = 0;			/* error */
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
		    lp = rlp;			/* in case dummy iClock */
		} else
		if (rsp->ftype == TIMRL) {
		    fsp->list = rlp;		/* link real to formal parameter */
						/* - not next formal parameter */
		    return rlp;			/* current real timer parameter */
		} else {
		    fsp->list = clp = sy_push(0); /* clock is currently unresolved */
		    clp->le_next = iFunClock;	/* because there is no clock parameter */
		    iFunClock = clp;		/* resolve in function call */
		    iFormNext = iFormNext->le_next; /* next formal parameter to process */
		    continue;			/* do not step to next real parameter */
		}
		break;
	    case TIMRL:
		if (rsp->ftype != TIMRL) {
		    ierror("wrong parameter type for timer:", rsp->name);
		    sy_pop(rlp);		/* parameter not used */
		    rlp = 0;			/* error */
		}
		break;
	    default:
		ierror("wrong type for function call parameter:", rsp->name);
		rlp = 0;			/* error */
		break;
	    }
	    fsp->list = rlp;			/* link real to formal parameter */
	}
	iFormNext = iFormNext->le_next;		/* next formal parameter to process */
	if (lp == 0) break;
	return 0;
    }
    if (lp) {
	ierror("called function has too many real parameters:", rsp ? rsp->name : 0);
    }
    rsp = iclock;				/* default clock */
    while (iFunClock) {				/* resolve formal clock parameters now */
	iFunClock->le_sym = rsp;		/* link unresloved clock to this clock */
	clp = iFunClock->le_next;		/* next unresolved clock */
	iFunClock->le_next = 0;			/* clean up list */
	iFunClock = clp;
    }
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
 *	the counter is passed to later runs via _List1.h.
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
    int			instanceFlg;
    int			saveCount;		/* saveFunPt,iFormHead,iFormNext,iFunClock */
    int			cF;
    struct sF *		oldSFunBs;
    char *		cp;
    char		temp[TSIZE];

    slp = lp = functionHead->u_blist;	/* get 2 numbers in first elements of statement list */
    if (slp == 0) {
	ierror("called function has no statements - cannot execute!", functionHead->name);
	return 0;
    }
    assert(saveFunBs);				/* no pop without previous push */
    instanceNum = lp->le_val + 1;		/* this function call instance number */
    instanceFlg = 0;				/* update only if it was used */
    lp = lp->le_next;				/* first varList link */
    assert(lp);					/* must be a pair */
    saveCount = lp->le_val;			/* allows call to store save block */
    /********************************************************************
     * previously parameter links where set up from formal to real parameters
     * find unresolved clock parameters (since real clock parameters optional)
     *******************************************************************/
    while (iFormNext || iFunClock) {
	handleRealParameter(plp, 0);
	if (iFormNext) {
	    ierror("called function has too few real parameters:", functionHead->name);
	    while (iFormNext) {
		handleRealParameter(plp, 0);
	    }
	}
    }
    /********************************************************************
     * Clone the statements of the called function.
     * Scan the statement list starting at the function head in 3 passes.
     * There ar two links per statement:
     *    the first link points to the statement head
     *    the second link points to varList for the statement
     * Pass 1: a) reconstitute templist from Symbols on varList and clone
     *		  each varList Symbol.
     *         b) clone the function internal statement heads to make
     *            them available to all other expressions.
     *         c) clone the return statement head to make it available
     *            to all other expressions.
     *******************************************************************/
    rsp = 0;					/* return must be set in a value function */
    while (slp) {
	vlp = slp->le_next;
	assert(vlp);				/* statement list is in pairs */
	/********************************************************************
	 * reconstitute templist from Symbols on varList
	 *******************************************************************/
	vsp = vlp->le_sym;			/* varList of temp Symbols */
	while (vsp) {				/* varList may be empty */
	    assert(vsp->list == 0);
	    vsp->list = (List_e *) cloneSymbol(vsp); /* point to new internal Symbol */
	    vsp = vsp->next;			/* next varList Symbol */
	}
	/********************************************************************
	 * clone the expression head associated with this statement
	 * u_blist == 0 indicates a function internal Symbol
	 * by doing this after varList, cloned head is at front of templist
	 *******************************************************************/
	ssp = slp->le_sym;			/* expression head template */
	assert(ssp);				/* check in Pass 1 only */
	assert(ssp->name);
	if ((lp = ssp->list) == 0) {		/* function internal variable */
	    if (iFunSymExt) {
		int	n = 0;			/* cloned in a function definition */
		cp = strchr(ssp->name, '$');	/* locate original extension */
		assert(cp && isprint(cp[1]));	/* extension must be at least 1 character */
		strncpy(iFunSymExt, cp+1, iFunEnd - iFunSymExt);/* copy ext to new fun name */
		cp = iFunBuffer + strlen(iFunBuffer);		/* end of new var name */
		while (lookup(iFunBuffer) != 0) {
		    /********************************************************************
		     * Different instances of the same extension can occurr for multiple
		     * calls of the same function in one function definition or for the
		     * same name used in different functions called in the definition.
		     * This heuristic assumes there are not hundreds of instances in
		     * one function definition. (Even a very large number will work -
		     * the temporaries generated from it are _1 etc) eg: fun$i123456_1
		     * The heuristic can handle extensions ending in numerals - it will
		     * keep trying until it finds one with a different numeral.
		     * The heuristic handles multiply nested function calls reasonably.
		     * These names do not touch the instance number until the function
		     * is finally called, when an instance number will be appended.
		     * This scheme als sees to it, that the same template is generated
		     * in each file it is used in - providing the set of functions is the
		     * same. This helps analysis of the generated code.
		     *******************************************************************/
		    snprintf(cp, iFunEnd - cp, "%d", ++n);	/* simply count up */
		}				/* positon in ST located in while */
		sv.v = install(iFunBuffer, UDF, ssp->ftype);
		*iFunSymExt = '\0';
		collectStatement(sv.v);		/* put in definition stmtList like a decl */
	    } else {
		/********************************************************************
		 * Naming for global internal names needs instanceNum, because names
		 * may be generated in several independent source files, unlike names
		 * in function definitions above, which are contained in one file.
		 *******************************************************************/
		snprintf(temp, TSIZE, "%s_%d", ssp->name, instanceNum);
		instanceFlg++;			/* instanceNum was used so update */
		cp = strchr(temp, '$');		/* locate original extension */
		assert(cp);			/* has failed during development */
		*cp = '_';			/* replace '$' by '_' in cloned object */
		if ((sv.v = lookup(temp)) == 0) {	/* locate position in ST */
		    sv.v = install(temp, UDF, ssp->ftype); /* Symbol for declared variable */
		}
	    }
	    ssp->list = sy_push(sv.v);		/* link internal cloned Symbol to template */
	} else
	if (lp->le_sym == functionHead) {
	    rsp = cloneSymbol(ssp);		/* clone return expression head Symbol */
	}					/* assign parameters alrady have a link */
	slp = vlp->le_next;			/* next statement */
    }
    /********************************************************************
     * Pass 2: scan again and clone the expression for each statement
     *******************************************************************/
    slp = functionHead->u_blist;
    while (slp) {
	vlp = slp->le_next;
	ssp = slp->le_sym;			/* expression head template */
	assert(ssp->type & FM);			/* marked statement list head Symbol */
	assert(ssp->u_blist || ssp->type == (UDF|FM) ||
	       ssp->type == (ARNC|FM) || ssp->type == (LOGC|FM));
	/********************************************************************
	 * clone the expression head associated with this statement
	 *******************************************************************/
	lp = ssp->list;				/* link to real return, assign parameter or */
	assert(lp);				/* internal Symbol linked in pass 1 */
	if ((sv.v = lp->le_sym) == functionHead) {
	    assert(rsp);			/* return Symbol was set in Pass 1 */
	    rsp->u_blist = cloneList(ssp->u_blist, rsp, rsp); /* clone the rest of the expression */
	    rlp = sy_push(rsp);			/* set up function return link */
	} else if (ssp->u_blist) {		/* assign parameter or internal Symbol */
	    assert(sv.v);			/* ZZZ what happens to link if not used ??? */
	    csp = cloneSymbol(ssp);		/* clone parameter expression head Symbol */
	    csp->u_blist = cloneList(ssp->u_blist, csp, rsp); /* clone the rest of the expression */
	    assert(sv.v->u_blist == 0);		/* will be assigning to it */
	    sv.v->u_blist = sy_push(csp);	/* link to expression head for Pass 3 */
	}
	slp = vlp->le_next;			/* next statement */
    }
    /********************************************************************
     * Pass 3: assign internally declared variables and assign parameters
     *******************************************************************/
    slp = functionHead->u_blist;
    while (slp) {
	vlp = slp->le_next;
	ssp = slp->le_sym;			/* expression head template */
	lp = ssp->list;				/* link to real Symbol */
	assert(lp);				/* internal Symbols linked in pass 1 */
	if ((sv.v = lp->le_sym) != functionHead) {
	    unsigned char	y1[2];		/* assign parameter or internal Symbol */
	    int			yn;
	    int			ioTyp;
	    /********************************************************************
	     * assign the expression associated with this assign para or internal statement
	     * an output can only be recognised by its name: Q[XBWL]%d
	     *******************************************************************/
	    sl.v = sv.v->u_blist;		/* link to expression head */
	    sv.f = sv.l = sl.f = sl.l = 0;	/* clear internal expression text */
	    if (ssp->u_blist == 0) {
		assert(sl.v == 0);		 /* template is defined - otherwise error */
		sv.v->type = ctypes[sv.v->ftype]; /* must be ARNC or LOGC */
	    } else
	    if (iFunSymExt || (ssp->type != (SW|FM) && ssp->type != (CF|FM))) {
		sv.v->u_blist = 0;		/* restore for op_asgn */
		if (lp->le_val == NOT && sv.v->ftype == GATE) {
		    op_not(sl.v);		/* negated assign parameter */
		}
		if (sscanf(sv.v->name, "Q%1[XBWL]%d", y1, &yn) == 2) {
		    ioTyp = (y1[0] == 'X') ? OUTX : OUTW;
		} else {
		    ioTyp = 0;			/* flags that no I/O is generated */
		}
		assignExpression(&sv, &sl, ioTyp); /* assign to internal variable or parameter */
#if YYDEBUG
		if ((debug & 0402) == 0402) pu(0, "clone", (Lis*)&sv);
#endif
	    } else {				/* iFunSymExt == 0 */
		int	cFn;
		cFn = ssp->u_blist->le_val >> 8; /* function number */
		assert(cFn && cFn < functionUseSize);
		functionUse[0] |= F_CALLED;	/* flag for copying temp file*/
		functionUse[cFn]++;		/* free this function for copying */
		sy_pop(lp);			/* free link before target is unlinked */
		ssp->list = 0;			/* free for next cloning */
		unlink_sym(sv.v);		/* unlink Symbol from symbol table */
		free(sv.v->name);		/* free name space */
		free(sv.v);			/* temporary Symbol */
		op_asgn(0, &sl, GATE);		/* delete missing slave gate in ffexpr */
	    }
	}
	slp = vlp->le_next;			/* next statement */
    }
    /********************************************************************
     * Cloning is now complete
     * Free links and restore internal pointers in function template.
     * The order is the same as in pushFunCall() to restore correctly.
     * Assign parameter links are restored in Pass 4 with other parameters.
     * Declaration and assigns are restored in Pass 5 (assigns are restored twice).
     *******************************************************************/
    cF = 0;
    /********************************************************************
     * Pass 4: Clean up parameter list
     *******************************************************************/
    lp = functionHead->list;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;
	assert(sp);
	lp1 = sp->list;
	while (lp1) {
	    lp2 = lp1->le_next;
	    sy_pop(lp1);			/* free link pushed in handleRealParameter */
	    lp1 = lp2;
	}
	sp->list = saveFunBs->iSav[cF++];	/* restore previous parameter template link */
	lp = lp->le_next;
    }
    /********************************************************************
     * Pass 5: Clean up statement list
     *******************************************************************/
    slp = functionHead->u_blist;		/* statement list */
    if (instanceFlg) slp->le_val = instanceNum;	/* update function call instance number */
    while (slp) {
	sp = slp->le_sym;
	assert(sp);
	sp1 = ((lp1 = sp->list) != 0) ? lp1->le_sym : 0;
	if (sp1 != functionHead) {		/* bypass return link */
	    while (lp1) {
		lp2 = lp1->le_next;
		sy_pop(lp1);			/* free link pushed in cloneFunction */
		lp1 = lp2;
	    }
	    sp->list = saveFunBs->iSav[cF++];/* restore declaration and assign link */
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
cloneList(List_e * slp, Symbol * csp, Symbol * rsp)
{
    List_e *		rlp = 0;		/* return pointer */
    List_e *		tlp = 0;		/* temp pointer */
    List_e *		clp;			/* cloned list link */
    List_e *		nlp;			/* next cloned list link */
    Symbol *		ssp;			/* current source Symbol */
    Symbol *		nsp;			/* cloned new Symbol */
    unsigned int	nval;			/* logical sign of a real parameter */
    char *		first;			/* catch arithmetic text */
    char *		last;
    static unsigned	recursions = 0;

    assert(recursions < 1000);
    recursions++;
    while (slp) {
	ssp = slp->le_sym;
	assert(ssp);
	if (ssp->type & FM) {			/* formal parameter or declared value */
	    if ((nlp = ssp->list) != 0 &&	/* link to real Symbol */
		(nsp = nlp->le_sym) != 0) {	/* real Symbol cloned before */
		nval = nlp->le_val;		/* logical sign of a real GATE parameter */
		first = nlp->le_first;		/* transfer arithmetic text */
		last = nlp->le_last;
		assert(ssp->ftype == GATE || nval == 0);
		if (nsp == iCallHead) {		/* iCallHead used as dummy real return */
		    nsp = rsp;			/* real return Symbol of current statement */
		    nlp = nsp->v_elist;		/* feedback list */
		    nsp->v_elist = sy_push(csp);
		    nsp->v_elist->le_next = nlp;
		    nlp = 0;
		} else {
		    nlp = nlp->le_next;		/* possible timer parameter link */
		}
	    } else {
		ierror("no link from formal to real parameter:", ssp->name);
		break;				/* cannot continue */
	    }
	} else
	if (ssp->name) {			/* global variable Symbol */
	    nsp = ssp;				/* pointer to global variable */
	    nlp = 0;
	    nval = 0;
	    first = last = 0;
	} else {				/* function internal Symbol ->name == 0 */
	    nsp = (Symbol*)ssp->list;		/* link set up in cloneFunction varList scan */
	    assert(nsp);
	    if (nsp->u_blist == 0) {		/* clone internal expression recursively */
		nsp->u_blist = slp;		/* set in case of tight feedback */
		nsp->u_blist = cloneList(ssp->u_blist, nsp, rsp); /* unless in parallel branch */
	    }
	    nlp = 0;
	    nval = 0;
	    first = last = 0;
	}
	clp = sy_push(nsp);			/* clone one list element */
	clp->le_val = slp->le_val ^ nval;	/* negation or arithmetic index in para */
	clp->le_first = first;			/* transfer arithmetic text */
	clp->le_last = last;
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
	    clp->le_first = nlp->le_first;	/* transfer arithmetic text */
	    clp->le_last = nlp->le_last;
	    tlp = tlp->le_next = clp;		/* also link delay element */
	    nlp = nlp->le_next;			/* 0 unless other parameter expression */
	}
	slp = slp->le_next;			/* next formal list element */
    }
    --recursions;
    return rlp;					/* first link in cloned chain */
} /* cloneList */

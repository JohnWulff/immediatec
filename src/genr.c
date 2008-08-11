static const char genr_c[] =
"@(#)$Id: genr.c,v 1.73 2008/07/02 15:47:08 jw Exp $";
/********************************************************************
 *
 *	Copyright (C) 1985-2008  John E. Wulff
 *
 *  You may distribute under the terms of either the GNU General Public
 *  License or the Artistic License, as specified in the README file.
 *
 *  For more information about this program, or for information on how
 *  to contact the author, see the README file or <ic@je-wulff.de>
 *
 *	genr.c
 *	generator functions for immcc compiler
 *
 *******************************************************************/

#ifdef _WINDOWS
#include	<windows.h>
#endif
#include	<stdio.h>
#ifndef	WIN32
#include	<stdlib.h>
#endif	/* WIN32 */
#include	<string.h>
#include	<assert.h>
#include	<ctype.h>
#include	"icc.h"
#include	"comp.h"
#include	"comp_tab.h"

#define v(lp)	(lp->le_val) ? '~' : ' ', lp->le_sym ? lp->le_sym->name : "(0)"
#define w(lp)	(lp->le_val) ? '~' : ' '
#define EBSIZE	2048
#define STX	'\x02'
#define ETX	'\x03'

FuUse *		functionUse = NULL;	/* database to record function calls */
int		functionUseSize = 0;	/* dynamic size adjusted with realloc */

static Symbol *	templist;		/* temp list of un-named symbols */
static List_e *	freelist;		/* list of links merged in optimisation */
static int	ttn;			/* for generating temp f object name */
#if YYDEBUG
static int	tn;
#endif
static char	eBuf[EBSIZE];		/* temporary C expression text buffer */
static char	cBuf[EBSIZE];		/* temporary clone expression out buffer */
static char	gBuf[EBSIZE];		/* temporary clone expression in buffer */
static char	tBuf[EBSIZE];		/* temporary text expression buffer */
static char *	ePtr = 0;		/* temporary C expression buffer input pointer */
static char *	eEnd = 0;		/* end of temporary C expression buffer */
static char *	cPtr = 0;		/* temporary clone expression out buffer input pointer */
static char *	cEnd = 0;		/* end of temporary clone expression out buffer */
static char *	gPtr = 0;		/* temporary clone expression in buffer input pointer */
static char *	tPtr = 0;		/* temporary text expression buffer input pointer */
static char *	tOut = 0;		/* temporary text expression buffer output pointer */
static char *	t_first = 0;
static char *	t_last = 0;
static char *	iC_One = "1";		/* name for constant 1 Symbol (installed in S.T.) */
static Symbol *	iCallHead = 0;		/* function head seen at start of imm call */
static Symbol *	varList = 0;		/* list of temp Symbols while compiling function */
static List_e *	stmtList = 0;		/* list of statement Symbols making function body */
static int	ffexprFlag = 0;		/* if - else or switch seen in function */
#if YYDEBUG
static char *	tfirst;			/* start of replacement arithmetic string */
static char *	tlast;			/* rll was set up early in cloneFunction */
#endif
static char *	ttp;			/* pointer into extended arithmetic text */

typedef struct SymListP {		/* allow alternate storage of Symbol or List_e pointers */
    List_e *	l;			/* store link List_e * */
    Symbol *	s;			/* as well as Symbol * */
    int		x;			/* real index */
} SymListP;
static SymListP	caList[PPGATESIZE+1];	/* auxiliary input gate list for copyArithmetic */

static Sym	vaList[0x80];		/* stores text and Symbol pointers for repeat elements */
int		z = 0;			/* index into vaList[] - circular list of 128 elements */
static int	liveDisp;		/* set when arithmetic values will generate live display */

/********************************************************************
 *
 *	initialize for code generation
 *
 *******************************************************************/

void
initcode(void)				/* initialize for code generation */
{
    templist = 0;
    freelist = 0;
#if YYDEBUG
    tn = 0;				/* re-initialisze for each new pass */
    liveDisp = iC_debug & 0200;		/* initially not in function generation - live display if -d200 */
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "initcode:\n");
	fflush(iC_outFP);
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
sy_push(Symbol * var)			/* create List element for variable */
{
    List_e *	lp;

    lp = (List_e *) iC_emalloc(sizeof(List_e));
    lp->le_sym = var;			/* point to variables Symbol entry */
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "sy_push:++%s\n", var ? var->name : "(null)");
	fflush(iC_outFP);
    }
#endif
    return lp;			/* return pointer to List_e to yacc */
} /* sy_push */

/********************************************************************
 *
 *	delete List element left over
 *	allow popping a NULL var* (just in case)
 *
 *******************************************************************/

Symbol *
sy_pop(List_e * lp)			/* delete List element left over */
{
    Symbol *	sp;

    sp = lp->le_sym;			/* point to variables Symbol entry */
    free(lp);
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "sy_pop:   %s--\n", sp ? sp->name : "(null)");
	fflush(iC_outFP);
    }
#endif
    return sp;				/* return pointer to Symbol to yacc */
} /* sy_pop */

/********************************************************************
 *
 *	create a new temporary '@' Symbol
 *
 *******************************************************************/

static Symbol *
atSymbol(void)
{
    Symbol *	sp;
#if YYDEBUG
    char 	temp[TSIZE];
#endif

    sp = (Symbol *) iC_emalloc(sizeof(Symbol));
    sp->name = NS;			/* no name at present */
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {	/* DEBUG name */
	snprintf(temp, TSIZE, "@%d", ++tn);
	sp->name = iC_emalloc(strlen(temp)+1);	/* +1 for '\0' */
	strcpy(sp->name, temp);
    }
#endif
    return sp;				/* return pointer to new '@' Symbol */
} /* atSymbol */

/********************************************************************
 *
 *	force linked Symbol to correct ftype
 *
 *******************************************************************/

List_e *
op_force(				/* force linked Symbol to correct ftype */
    List_e *		lp,
    unsigned char	ftyp)
{
    Symbol *		sp;
    List_e *		lp1;
    int			typ;

    if (lp && (sp = lp->le_sym) != 0 && sp->ftype != ftyp) {
	if (sp->u_blist == 0 ||			/* not a @ symbol or */
	    sp->type >= MAX_GT ||	/* SH, FF, EF, VF, SW, CF or */
	    (sp->u_blist->le_sym == sp && sp->type == LATCH)) { /* L(r,s) */
	    if ((typ = iC_types[sp->ftype]) == ERR) {
		ierror("cannot force from", iC_full_ftype[sp->ftype]);
	    }
	    lp1 = op_push(0, typ, lp);
	    assert(lp->le_first == 0 || lp->le_first >= iCbuf && lp->le_last < &iCbuf[IMMBUFSIZE]);
	    lp1->le_first = lp->le_first;
	    lp1->le_last = lp->le_last;
	    lp = lp1;	/* create a new @ symbol linked to old */
	    sp = lp->le_sym;
	}
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "op_force: %s from %s to %s\n",
		sp->name, iC_full_ftype[sp->ftype], iC_full_ftype[ftyp]);
	    fflush(iC_outFP);
	}
#endif
	sp->ftype = ftyp;		/* convert old or new to ftyp */
    }
    return lp;				/* return 0 or link to old or new Symbol */
} /* op_force */

/********************************************************************
 *
 *	Unlink a temporary Symbol from templist and free the
 *	Link and the Symbol and the name of the Symbol if YYDEBUG
 *
 *******************************************************************/

static void
freeTemp(List_e * lp)
{
    Symbol * tsp;
    Symbol * sp;

    sp =  sy_pop(lp);		/* free Link */
    if (templist != sp) {	/* bypass sp on templist */
	tsp = templist;		/* scan templist */
	while (tsp->next != sp) {
	    tsp = tsp->next;	/* find sp in templist */
	    if (tsp == 0) {
		execerror("temp not found ???\n", NS, __FILE__, __LINE__);
	    }
	}
	tsp->next = sp->next;	/* unlink sp from templist */
    } else {
	templist = sp->next;	/* unlink first object */
    }
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	free(sp->name);		/* free name space */
    }
#endif
    free(sp);			/* free Symbol */
} /* freeTemp */

/********************************************************************
 *
 *	reduce List_e stack to links
 *
 *******************************************************************/

List_e *
op_push(				/* reduce List_e stack to links */
    List_e *		left,
    unsigned char	op,
    List_e *		right)
{
    List_e *		rlp;
    Symbol *		sp;		/* current temporary Symbol */
    Symbol *		lsp;
    Symbol *		tsp;
    List_e *		lp;
    List_e *		tlp;
    int			typ;
    int			ftyp;
    char *		tf;
    char *		tl;
    int			c;
    int			stxFlag;
#if YYDEBUG
    int			len1;
#endif

    if (right == 0) {
	if ((right = left) == 0) return 0;	/* nothing to push */
	left = 0;				/* swapped left to right */
    }
    sp = right->le_sym;
    lsp = left ? left->le_sym : 0;
    rlp = right;
    if (iC_optimise & 02) {			/* only required for arithmetic optimisation */
	if (sp) {
	    if (sp->type == ARNF && op == ARN) {
		tf = right->le_first;
		tl = right->le_last;
		stxFlag = 0;
		assert(*tf == STX);
		do {
		    if ((c = *tf++) == STX) {
			stxFlag++;		/* block copying call text */
		    } else
		    if (c == ETX) {
			--stxFlag;		/* end of call text */
		    }
		} while (stxFlag && tf < tl);
		--tl;
		assert(tf < tl);
		if (*tf == '#' && *tl == '#') {
		    *tf = '(';			/* parenthesise function call in expression */
		    *tl = ')';
		}
		sp->type = ARN;			/* merge ARNF with ARN when optimising */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    len1 = right->le_last - right->le_first;
		    fprintf(iC_outFP, "op_push:  right %s type change from ARNF to ARN '%*.*s'\n",
			sp->name, len1, len1, right->le_first);
		    fflush(iC_outFP);
		}
#endif
	    }
	    if (left &&				/* must retain either left or right */
		right->le_val != (unsigned)-1 &&/* and right is not a delay for timer */
#if ! YYDEBUG || defined(SYUNION)
		sp->type < KEYW &&		/* required to stop using u_blist when u_val = CNAME */
#endif
		(tlp = sp->u_blist) != 0 &&	/* must parenthesise call returning */
		(tsp = tlp->le_sym) != 0 &&	/* constant expression before deleting */
		tsp == iconst)			/* link to "iConst" */
	    {
		assert(right->le_next == 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "op_push:  free right %s which points to iConst\n", sp->name);
		    fflush(iC_outFP);
		}
#endif
		sp->u_blist = tlp->le_next;
		sy_pop(tlp);			/* pop link to "iConst" */
		if (sp->u_blist == 0) {
		    freeTemp(right);		/* bypass sp on templist and delete right and sp */
		    right = 0;			/* needed for Electric fence */
		    rlp = left;			/* swapped left to right */
		    sp = rlp->le_sym;
		}
	    }
	}
	if (lsp) {
	    if (lsp->type == ARNF && op == ARN) {
		tf = left->le_first;
		tl = left->le_last;
		stxFlag = 0;
		assert(*tf == STX);
		do {
		    if ((c = *tf++) == STX) {
			stxFlag++;		/* block copying call text */
		    } else
		    if (c == ETX) {
			--stxFlag;		/* end of call text */
		    }
		} while (stxFlag && tf < tl);
		--tl;
		assert(tf < tl);
		if (*tf == '#' && *tl == '#') {
		    *tf = '(';			/* parenthesise function call in expression */
		    *tl = ')';
		}
		lsp->type = ARN;		/* merge ARNF with ARN when optimising */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    len1 = left->le_last - left->le_first;
		    fprintf(iC_outFP, "op_push:  left %s type change from ARNF to ARN '%*.*s'\n",
			lsp->name, len1, len1, left->le_first);
		    fflush(iC_outFP);
		}
#endif
	    }
	    if (rlp == right &&			/* don't delete both right and left */
		left->le_val != (unsigned)-1 &&	/* and left is not a delay for timer */
#if ! YYDEBUG || defined(SYUNION)
		lsp->type < KEYW &&		/* required to stop using u_blist when u_val = CNAME */
#endif
		(tlp = lsp->u_blist) != 0 &&	/* must parenthesise call returning */
		(tsp = tlp->le_sym) != 0 &&	/* constant expression before deleting */
		tsp == iconst)			/* link to "iConst" */
	    {
		assert(left->le_next == 0);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "op_push:  free left %s which points to iConst\n", lsp->name);
		    fflush(iC_outFP);
		}
#endif
		lsp->u_blist = tlp->le_next;
		sy_pop(tlp);			/* pop link to "iConst" */
		if (lsp->u_blist == 0) {
		    freeTemp(left);		/* bypass lsp on templist and delete left and tsp */
		    left = 0;
		}
	    }
	}
	if (rlp != right) {
	    left = 0;				/* right is only used in left section */
	    right = rlp;			/* and final debug message */
	}
    }
    assert(sp == 0 || sp->type < IFUNCT);	/* allows IFUNCT to use union v.cnt */
    typ = sp ? sp->type : ARN;
    ftyp = sp ? sp->ftype : ARITH;
    if (left && op > OR && op < MAX_LV && op != typ) {
	warning("function iC_types incompatible", NS);
    }
    if (sp == 0 || sp->u_blist == 0 || op != typ) {
	if (sp &&
#if ! YYDEBUG || defined(SYUNION)
	    (lp = 0, sp->v_cnt) > 2 &&
#endif
	    (lp = sp->v_elist) != 0 && (sp->name == 0
#if YYDEBUG
						      || (sp->name)[0] == '@'
#endif
	)) {					/* not marked symbol */
#if ! YYDEBUG || defined(SYUNION)
	    sp->v_cnt = 2;			/* clear the feedback list but retain high use count */
#else
	    sp->v_elist = 0;			/* clear the feedback list */
#endif
	    while (lp) {			/* list with feedback to this Symbol */
		tlp = lp->le_next;		/* next link to a feedback Symbol */
		sy_pop(lp);			/* delete a feedback list link */
		lp = tlp;
	    }
	}
	sp = atSymbol();		/* right not a @ symbol or new operator - force new level */
	sp->type = op != UDF ? op : AND; /* operator OR or AND (default) */
	sp->ftype = ftyp;		/* used in op_xor() with op UDF (defunct) */
	sp->next = templist;		/* put at front of templist */
	templist = sp;
	rlp->le_next = 0;		/* sp->u_blist is 0 for new sp */
	sp->u_blist = rlp;		/* link right of expression */
	rlp = sy_push(sp);		/* push new list element on stack */
    }
    if (left && lsp) {			/* test works correctly with ftype - handles ALIAS */
	assert(lsp->type < IFUNCT);	/* allows IFUNCT to use union v.cnt */
	if (lsp->ftype >= MIN_ACT && lsp->ftype < MAX_ACT) {
	    if (sp->ftype < S_FF) {
		sp->ftype = 0;		/* OK for any value of GATE */
	    }
	    sp->ftype |= lsp->ftype;	/* modify S_FF ==> D_FF */
	    /* this requires S_FF R_FF D_FF to be 1001 1010 1011 to work !!! */
	    /* does nothing for S_SH R_SH D_SH - not required because no analog SR */
	    assert(rlp && rlp->le_val <= NOT);
	    if (rlp->le_val == NOT) {
		rlp->le_val = NOT^NOT;
		op_not(rlp);		/* double negate so input to action gate not inverted */
	    }
	}
	if ((typ = lsp->type) < MAX_LS) {
	    if (
#if ! YYDEBUG || defined(SYUNION)
		(lp = 0, lsp->v_cnt) > 2 &&
#endif
		(lp = lsp->v_elist) != 0 && (lsp->name == 0
#if YYDEBUG
							    || (lsp->name)[0] == '@'
#endif
	    )) {				/* not marked symbol */
#if ! YYDEBUG || defined(SYUNION)
		lsp->v_cnt = 2;			/* clear the feedback list but retain high use count */
#else
		lsp->v_elist = 0;		/* clear the feedback list */
#endif
		while (lp) {			/* list with feedback to this Symbol */
		    tlp = lp->le_next;		/* next link to a feedback Symbol */
		    sy_pop(lp);			/* delete a feedback list link */
		    lp = tlp;
		}
	    }
	    if ((lp = lsp->u_blist) == 0 ||	/* left not a @ symbol */
		sp == lsp ||			/* or right == left */
		(typ != op &&			/* or new operator */
			/* TODO watch this when typ is ALIAS or UDF */
		typ != TIM) ||			/* but left is not a timer */
		(typ == op &&			/* or old operator */
		right->le_val == (unsigned)-1))	/* and right is a delay for timer */
	    {
		left->le_next = sp->u_blist;	/* extend expression */
		sp->u_blist = left;		/* link left of expr */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "op_push:  %c%s %c %c%s\n",
			v(left), iC_os[op], v(right));
		    fflush(iC_outFP);
		}
#endif
	    } else {	/* left is a @ symbol and ... - merge left into right */
		while (lp->le_next) {
		    lp = lp->le_next;		/* scan to end of left list */
		}
		lp->le_next = sp->u_blist;	/* move connect list */
		sp->u_blist = lsp->u_blist;	/* in the right order */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "op_push:  %c%s %c %c%s\n",
			v(left), iC_os[op], v(right));
		    fflush(iC_outFP);
		}
#endif
		freeTemp(left);			/* bypass lsp on templist and delete left and lsp */
		left = 0;
	    }
	} else {				/* discard left BLT, CLT */
	    sy_pop(left);			/* Link_e only */
	}
#if YYDEBUG
    } else if ((iC_debug & 0402) == 0402) {	/* fexpr : sexpr { left is 0 } */
	fprintf(iC_outFP, "op_push:  (0) %c %c%s\n", iC_os[op], v(right));
	fflush(iC_outFP);
#endif
    }
    return rlp;
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
    Symbol *	sp;
    List_e *	lp;
    char *	endptr;
    long	value;

    while (cp < ep) {
	if (--bc == 0 || !isprint(*bp++ = *cp++)) {
	    return 1;			/* error - too big or not printable */
	}
    }
    *bp = 0;				/* terminate - there is room in buf */
    value = strtol(buf, &endptr, 0);	/* convert to check for error */
    if (*endptr != '\0') {
	/* const @ symbol */
	assert(iconst);			/* only cleared in listNet() if not used */
	lp = sy_push(iconst);		/* link to "iConst" */
	expr->v = op_push(0, ARN, lp);
	assert(expr->f == 0 || expr->f >= iCbuf && expr->l < &iCbuf[IMMBUFSIZE]);
	expr->v->le_first = lp->le_first = expr->f;	/* identifies as const expression */
	expr->v->le_last  = lp->le_last  = expr->l;
    } else {
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "const_push: '%s' converted to %ld\n", buf, value);
	    fflush(iC_outFP);
	}
#endif
	snprintf(buf, TSIZE, "%ld", value);
	if ((sp = lookup(buf)) == 0) {	/* install name of decimal conversion */
	    sp = install(buf, NCONST, ARITH);
	} else if (sp->type != NCONST && sp->ftype != ARITH) {
	    ierror("use of a constant which was previously used for a different purpose:", buf);
	}
	expr->v = sy_push(sp);		/* expr->v->le_first == 0 is simple const */
	assert(expr->f == 0 || expr->f >= iCbuf && expr->l < &iCbuf[IMMBUFSIZE]);
	expr->v->le_first = expr->f;	/* identifies as const expression */
	expr->v->le_last  = expr->l;	/* no need to be 0 - JW 12.Apr.2007 */
    }
    return 0;				/* correct - no error */
} /* const_push */

/********************************************************************
 *
 *	logical negation
 *
 *******************************************************************/

List_e *
op_not(List_e * right)			/* logical negation */
{
    Symbol *	sp = right->le_sym;
    List_e *	lp = sp->u_blist;

#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "op_not:   ~%s\n", sp->name);
	fflush(iC_outFP);
    }
#endif
    if (lp == 0) {
	right->le_val ^= NOT;		/* negate logical value */
    } else {
	switch (sp->type) {		/* @ symbol */
	case AND:
	case OR:
	    sp->type ^= (AND ^ OR);	/* de Morgans rule - switch AND/OR */
	case LATCH:			/* -                 but not LATCH */
	    while (lp) {
		if (lp->le_sym != sp) {
		    lp->le_val ^= NOT;	/* - and negate every input */
		}			/* - except LATCH feedback */
		lp = lp->le_next;	/* - AND/OR has no direct feedback */
	    }
	    break;

	case XOR:			/* negate only the first input to XOR */
	case ALIAS:			/* unresolved alias from negated function */
	    lp->le_val ^= NOT;		/* negate via alias or one only XOR input */
	    break;

	case ARNC:
	case ARNF:
	case ARN:
	case LOGC:
	case SH:
	case FF:
	case VF:
	case EF:
	case SW:
	case CF:
	case NCONST:			/* impossible to generate ? */
	    right->le_val ^= NOT;	/* negate logical value - forces creation */
	    break;			/* of alias, if assigned immediately */

	case INPW:
	case INPX:
	    execerror("INPUT has other inputs in op_not() ???", sp->name, __FILE__, __LINE__);
	    break;

	default:
	    execerror("negation of non-logical value attempted", sp->name, __FILE__, __LINE__);
	    break;
	}
    }
    return right;
} /* op_not */

/********************************************************************
 *
 *  STANDARD C EXPRESSION
 *	Copy arithmetic from t_first to the current C expression (lp->le_first)
 *	into tBuf, eBuf and gBuf. Any NCONST was previously copied into the
 *	buffers. Output this part of the expression (tBuf) into the listing
 *	followed by the name of the input and // x. x is gt_input count.
 *	Copy the indexed macro iC_MV(x) of the expression or constant
 *	into eBuf, which finishes the C expression up to this point.
 *	For a generated function copy "\xff\<x>" into gBuf, which is easier
 *	to parse as a cloned expression, than the full C expression.
 *	Move t_first to point to after the end of the current subexpression.
 *
 *  CLONED C EXPRESSION
 *	For a cloned Gate, cFn is set.
 *	Copy cPtr up to the first occurence of "\xff\<x>" (check that
 *	n == x). Then output this into the listing, again followed
 *	by the current name of the input (real parameter) and // x
 *	Move cPtr to point to after "\xff\<x>". Look ahead for the next
 *	occurence of "\xff\<x>" and output a line for a duplicate, if x
 *	is < n+1. Repeat until x == n+1 or string terminates. (x > n+1
 *	is an error).
 *
 *******************************************************************/

static void
copyArithmetic(List_e * lp, Symbol * sp, Symbol * gp, int x, int sflag, int cFn)
{
    Symbol *	sp1;
    int		typ;
    int		c;
    int		stxFlag;
    int		y;
    int		z1;
    int		len;
    char *	cp;
    char *	ep;
    static int	x1;

    if (((typ = sp->type) == ARN || typ == ARNF) &&
	gp->ftype == ARITH &&
	lp->le_val != (unsigned)-1) {
	assert(ePtr && gPtr && tPtr && tOut);
	if (cFn == 0) {				/* STANDARD C EXPRESSION */
	    if (t_first) {				/* end of arith */
#ifndef EFENCE
		assert(t_first >= iCbuf && lp->le_first < &iCbuf[IMMBUFSIZE]);
#else
		if (t_first < iCbuf || lp->le_first >= &iCbuf[IMMBUFSIZE]) {
		    fprintf(iC_outFP, "\nsflag = 0%o; t_first = %p >= iCbuf = %p && lp->le_first = %p < &iCbuf[IMMBUFSIZE] = %p\n",
			sflag, t_first, iCbuf, lp->le_first, &iCbuf[IMMBUFSIZE]);
		    fflush(iC_outFP);
		    ierror("Assertion 1:", NULL);
		    exit(-1);
		}
#endif
		stxFlag = 0;
		while (t_first < lp->le_first) {
		    if ((c = *t_first++ & 0xff) == STX) {
			stxFlag++;			/* block copying call text */
		    }
		    if (stxFlag == 0 && c != '#') {	/* transmogrified '=' */
			if (c >= 0x80) {
			    z1 = c - 0x80;
			    sp1 = vaList[z1].v;
			    assert(sp1);
			    cp = sp1->name;		/* this variable to tBuf including NCONST */
			    len = strlen(cp);
			    if (iFunSymExt) {
				sp1->fm |= 0x02;	/* double usage in function arithmetic expression */
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    fprintf(iC_outFP, "copyArithmetic:  use count %d for %s\n", sp1->fm & FU, sp1->name);
				    fflush(iC_outFP);
				}
#endif
			    }
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				int len1 = vaList[z1].l - vaList[z1].f;
				fprintf(iC_outFP, "\ncA: z1=%2d	%-15s %-6s v_cnt = %2d [%*.*s] '%1.1s' {%*.*s}\n",
				    z1, sp1->name, iC_full_type[sp1->type],
				    sp1->v_cnt, len, len, cp, t_first-1, len1, len1, vaList[z1].f);
				fflush(iC_outFP);
			    }
#endif
			    tPtr = strncpy(tPtr, cp, len+1) + len;
			    if (sp1->type == NCONST) {
				ePtr = strncpy(ePtr, cp, len+1) + len;
				gPtr = strncpy(gPtr, cp, len+1) + len;
			    } else {
				for (y = 1; y <= PPGATESIZE; y++) {
				    if (sp1 == caList[y].s) break;
				}
				assert(y <= PPGATESIZE);	/* repeated term */
				if (iC_debug & 04) {
				    fprintf(iC_outFP, "\t\t\t\t\t%s\t// %d", tOut, y);
				    if (liveDisp) {
					fprintf(iC_outFP, "\t%s\t=", gp->name);
				    }
				    fprintf(iC_outFP, "\n");
				}
				tOut = tPtr;
				ePtr += sprintf(ePtr, "iC_MV(%d)", y);
				gPtr += sprintf(gPtr, "\xff%c", y);	/* bound to fit since shorter */
			    }
			    assert(ePtr + 10 < eEnd);	/* checks previous copying */
			} else
			*tPtr++ = *gPtr++ = *ePtr++ = c;
		    }
		    if (c == ETX) {
			assert(stxFlag > 0);
			--stxFlag;			/* end of call text */
		    }
		}
		assert(stxFlag == 0);
	    }
	    *tPtr = '\0';
	    if (gp == iconst) {				/* "iConst" */
		assert(sp->u_blist == 0);		/* make sure no other inputs */
		if (t_first) {				/* end of arith */
		    if (lp->le_first == 0) lp->le_first = t_first;
		    if (lp->le_last == 0) lp->le_last = t_last;
		    assert(t_first == lp->le_first);
		    assert(t_first >= iCbuf && lp->le_last >= t_first && lp->le_last < &iCbuf[IMMBUFSIZE]);
		    stxFlag = 0;
		    while (t_first < lp->le_last) {
			if ((c = *t_first++ & 0xff) == STX) {
			    stxFlag++;			/* block copying call text */
			}
			if (stxFlag == 0 && c != '#') {	/* transmogrified '=' */
			    if (c >= 0x80) {
				z1 = c - 0x80;
				sp1 = vaList[z1].v;
				assert(sp1);
				cp = sp1->name;		/* this variable to tBuf including NCONST */
				len = strlen(cp);
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    int len1 = vaList[z1].l - vaList[z1].f;
				    fprintf(iC_outFP, "\ncA: z1=%2d	%-15s %-6s v_cnt = %2d [%*.*s] '%1.1s' {%*.*s}\n",
					z1, sp1->name, iC_full_type[sp1->type],
					sp1->v_cnt, len, len, cp, t_first-1, len1, len1, vaList[z1].f);
				    fflush(iC_outFP);
				}
#endif
				tPtr = strncpy(tPtr, cp, len+1) + len;
				assert(sp1->type == NCONST);
				ePtr = strncpy(ePtr, cp, len+1) + len;
				gPtr = strncpy(gPtr, cp, len+1) + len;
				assert(ePtr + 10 < eEnd);	/* checks previous copying */
			    } else
			    *tPtr++ = *gPtr++ = *ePtr++ = c;
			}
			if (c == ETX) {
			    assert(stxFlag > 0);
			    --stxFlag;			/* end of call text */
			}
		    }
		    assert(stxFlag == 0);
		}
		*tPtr = '\0';
	    } else {
		cp = gp->name;				/* this variable to tBuf including NCONST */
		len = strlen(cp);
		tPtr = strncpy(tPtr, cp, len+1) + len;	/* variable name */
		if (sflag == 04) {
		    assert(gp->type == NCONST);		/* auxiliary storage of NCONST text */
		    ePtr = strncpy(ePtr, cp, len+1) + len;
		    gPtr = strncpy(gPtr, cp, len+1) + len;
		} else {
		    if (iC_debug & 04) {
			/* only logic gate or SH can be aux expression */
			if (sflag == 02) {
			    fprintf(iC_outFP, "\t\t\t\t\t"); /* auxiliary call for duplicate link */
			} else
			if (sflag & 0200) {
			    if (sp->ftype == GATE) {
				putc('\t', iC_outFP);
			    }
			    putc('\t', iC_outFP);
			} else {
			    fprintf(iC_outFP, "\t\t\t");
			}				/* expression till now */
			fprintf(iC_outFP, "%s\t// %d", tOut, x);
			if (liveDisp) {
			    fprintf(iC_outFP, "\t%s\t=", gp->name);
			}
			fprintf(iC_outFP, "\n");
		    }
		    tOut = tPtr;			/* alternative tOut = tPtr = tBuf */
		    assert(ePtr + 10 < eEnd);		/* checks previous copying */
		    ePtr += sprintf(ePtr, "iC_MV(%d)", x);
		    gPtr += sprintf(gPtr, "\xff%c", x);	/* bound to fit since shorter */
		}
		t_first = lp->le_last;			/* skip current expression */
#ifndef EFENCE
		assert(t_first == 0 || t_first >= iCbuf);
#else
		if (t_first != 0 && t_first < iCbuf) {
		    fprintf(iC_outFP, "\nt_first = %p >= iCbuf = %p\n", t_first, iCbuf);
		    fflush(iC_outFP);
		    ierror("Assertion 2:", NULL);
//##		    exit(-1);
		}
#endif
		if (t_last == 0) {
		    t_last = t_first;			/* TODO */
		}
	    }
	} else {				/* CLONED C EXPRESSION */
	    assert(cPtr);	/* cp points to current start of cloning expression in cBuf */
	    cp = cPtr;
	    ep = strchr(cp, '\xff');
	    if (ep) {
		y = ep[1];				/* numeric byte value */
		cPtr = ep;
		ep += 2;
	    } else {
		y = 0;
		ep = cPtr = cp + strlen(cp);		/* no more macros */
	    }
	    *cPtr = '\0';				/* modify \xff to \0 */
	    cPtr = ep;
	    len = strlen(cp);
	    tPtr = strncpy(tPtr, cp, len+1) + len;	/* code from cloning expression */
	    ePtr = strncpy(ePtr, cp, len+1) + len;
	    gPtr = strncpy(gPtr, cp, len+1) + len;
	    if (gp != iconst) {				/* not "iConst" */
		if (y != x) {
		    assert(sflag == 02);		/* duplicate call needs adjusting */
		    caList[y].x = x;			/* in case also duplicate in template */
		    x = y;				/* look for duplicates up to and including y */
		}
		cp = gp->name;				/* this variable to tBuf including NCONST */
		len = strlen(cp);
		tPtr = strncpy(tPtr, cp, len+1) + len;	/* variable name */
		if (sflag == 04) {
		    assert(gp->type == NCONST);		/* auxiliary storage of NCONST text */
		    ePtr = strncpy(ePtr, cp, len+1) + len;
		    gPtr = strncpy(gPtr, cp, len+1) + len;
		} else {
		    x1 = caList[y].x;			/* real index */
		    if (iC_debug & 04) {
			if (sflag & 0200) {
			    if (sp->ftype == GATE) {
				putc('\t', iC_outFP);
			    }
			    putc('\t', iC_outFP);
			} else {
			    fprintf(iC_outFP, "\t\t\t");
			}				/* expression till now */
			fprintf(iC_outFP, "%s\t// %d", tOut, x1);
			if (liveDisp) {
			    fprintf(iC_outFP, "\t%s\t=", gp->name);
			}
			fprintf(iC_outFP, "\n");
		    }
		    tOut = tPtr;			/* alternative tOut = tPtr = tBuf */
		    assert(ePtr + 10 < eEnd);		/* checks previous copying */
		    ePtr += sprintf(ePtr, "iC_MV(%d)", x1);
		    gPtr += sprintf(gPtr, "\xff%c", x1);	/* bound to fit since shorter */
		}
		while ((cp = strchr(ep, '\xff')) != 0 &&/* look ahead to next macro */
		    (y = cp[1]) <= x) {
		    gp = caList[y].s;			/* y should be stored in pre-scan */
		    assert(gp);
		    *cp = '\0';				/* modify \xff to \0 */
		    tPtr += sprintf(tPtr, "%s%s", ep, gp->name);	/* NCONST caught also */
		    assert(ePtr + 10 < eEnd);		/* checks previous copying */
		    if (gp->type == NCONST) {
			ePtr += sprintf(ePtr, "%s%s", ep, gp->name);
			gPtr += sprintf(gPtr, "%s%s", ep, gp->name);
		    } else {
			x1 = caList[y].x;		/* real index of non NCONST's */
			if (iC_debug & 04) {		/* repeated term */
			    fprintf(iC_outFP, "\t\t\t\t\t%s\t// %d", tOut, x1);
			    if (liveDisp) {
				fprintf(iC_outFP, "\t%s\t=", gp->name);
			    }
			    fprintf(iC_outFP, "\n");
			}
			tOut = tPtr;
			ePtr += sprintf(ePtr, "%siC_MV(%d)", ep, x1);
			gPtr += sprintf(gPtr, "%s\xff%c", ep, x1);	/* bound to fit since shorter */
		    }
		    cPtr = ep = cp + 2;
		}
	    }
	}
    } else
    if (iC_debug & 04 && sflag != 02) {
	if (liveDisp && (sflag & 0200) && (
	    sp->ftype == ARITH  ||
	    sp->ftype == D_SH   ||
	    sp->ftype == F_SW   ||
	    sp->ftype == OUTW   ||
	    sp->ftype == CH_BIT && (typ == ARN || typ == ARNF))) {	/* only arithmetic CHANGE */
	    fprintf(iC_outFP, "\t\t//\t%s\t=", sp->name);
	}
	fprintf(iC_outFP, "\n");		/* all non Arithmetic and delay */
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
	functionUse = (FuUse*)realloc(functionUse,
	    (functionUseSize + FUNUSESIZE) * sizeof(FuUse));
	memset(&functionUse[functionUseSize], '\0', FUNUSESIZE * sizeof(FuUse));
	functionUseSize += FUNUSESIZE;
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
    Lis *		rl,		/* in case of parse error rl->v may be 0 */
    unsigned char	ft)
{
    Symbol *	var;
    Symbol *	sp;
    List_e *	lp;
    Symbol *	gp = 0;
    Symbol *	rsp;
    List_e *	tlp;
    List_e *	nlp;
    List_e *	right;
    List_e *	ilp;
    List_e **	lpp;
    int		atn;
    int		sflag;
    int		fflag;
    int		c;
    int		stxFlag;
    Symbol *	sr;
    int		typ;
    int		typ1;
    char	temp[TSIZE];

    if (sv == 0) {
	/* null var - generate a temporary Symbol of type UNDEF */
	var = (Symbol *) iC_emalloc(sizeof(Symbol));
	do {
	    /* do not use generated name used somewhere else already */
	    snprintf(temp, TSIZE, "%s_f%d", iFunSymExt ? iFunBuffer : "", ++ttn);
	} while (lookup(temp) != 0);
	var->name = iC_emalloc(strlen(temp)+1);	/* +1 for '\0' */
	strcpy(var->name, temp);	/* name needed for derived Sy's */
	sflag = 0;			/* don't output name */
    } else {
	var = sv->v;			/* Symbol * var */
	sflag = 0200;			/* print output name - iflag = 0 */
    }
    assert(var->name);
    if (rl->v == 0) {
	if (!iFunSymExt) var->type = ERR;
	return var;
    }
    assert(varList == 0);		/* checks that collectStatement has been executed */
    right = op_force(rl->v, ft);	/* force Symbol on right to ftype ft */
    if (iFunSymExt && strncmp(var->name, iFunBuffer, iFunSymExt - iFunBuffer) != 0) {
	ierror("assignment to a variable which does not belong to this function:", var->name);
	var->type = ERR;		/* reduce anyway to clear list */
    }
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "op_asgn:  %s = %c%s\n", var->name, v(right));
	fflush(iC_outFP);
    }
#endif
    assert(right);			/* must have something to assign */
    rsp = right->le_sym;
    assert(rsp && rsp->type < IFUNCT);	/* allows IFUNCT to use union v.cnt */
    if ((typ = var->type) >= MIN_GT && typ != rsp->type) {
	if (typ != ERR) {
	    ierror("type mismatch in multiple assignment:", var->name);
	    var->type = ERR;		/* reduce anyway to clear list */
	}
    } else {
	var->type = rsp->type;
	if (var->ftype < MIN_ACT) {
	    var->ftype = rsp->ftype;
	}
    }
    if (templist == 0 || rsp->u_blist == 0 || rsp->type == NCONST) {	/* right must be a @ symbol */
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
	if (iC_debug & 04) {
	    iFlag = 1;			/* may need correction by pplstfix */
	    fprintf(iC_outFP, "\n\t%s\t%c ---%c\t%s", rsp->name,
		rsp->ftype != GATE ? iC_fos[rsp->ftype] : w(right),
		iC_os[var->type], var->name);	/* type is ALIAS or ERR */
	    if (var->ftype != GATE) {
		fprintf(iC_outFP, "\t%c", iC_fos[var->ftype]);
		if (liveDisp && var->ftype == ARITH) {
		    fprintf(iC_outFP, "\t\t//\t%s\t=", var->name);
		}
	    }
	    fprintf(iC_outFP, "\n\n");
	}
	if (sv == 0) {
	    execerror("ALIAS points to temp ???", var->name, __FILE__, __LINE__);
	} else if ((t_last = sv->l, t_first = sv->f) != 0) {
	    assert(t_first >= iCbuf && t_last >= t_first && t_last < &iCbuf[IMMBUFSIZE]);
	    memset(t_first, '#', sv->l - t_first);	/* mark left var, leave ALIAS */
	}
	return var;			/* needs no reduction */
    }

    /********************************************************************
     * 'rsp' is put at head of templist, if it is not already there
     *******************************************************************/
    if (rsp != (sp = templist)) {
	if (sp == 0) goto FailTemplist;
	while (rsp != sp->next) {
	    if ((sp = sp->next) == 0) {			/* DEBUG */
	      FailTemplist:
		execerror("right->le_sym not found in templist ???",
		    right->le_sym->name, __FILE__, __LINE__);
	    }
	}
	sp->next = rsp->next;		/* link tail to head in front of rsp */
	rsp->next = templist;		/* link head to rsp */
	templist = rsp;			/* now rsp is head of templist */
    }
    if (((typ = rsp->type) == CLK || typ == TIM) && var->ftype != rsp->ftype) {
	warning("clock or timer assignment from wrong ftype:", var->name);
    }
    /********************************************************************
     * resolve feedback passed in 'rsp->v.elist'
     *******************************************************************/
    if (
#if ! YYDEBUG || defined(SYUNION)
	(lp = 0, rsp->v_cnt) > 2 &&
#endif
	(lp = rsp->v_elist) != 0 && (rsp->name == 0
#if YYDEBUG
						    || (rsp->name)[0] == '@'
#endif
    )) {				/* not marked symbol */
#if ! YYDEBUG || defined(SYUNION)
	rsp->v_cnt = 2;			/* clear the feedback list but retain high use count */
#else
	rsp->v_elist = 0;		/* clear the feedback list */
#endif
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
		    if ((iC_debug & 0402) == 0402) {
			fprintf(iC_outFP, "op_asgn:  *** feedback at %s from %s to %s\n",
			    sr->name ? sr->name : "(null)",
			    sp->name ? sp->name : "(null)",
			    var->name ? var->name : "(null)");
			fflush(iC_outFP);
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
    assert(sr);
    /********************************************************************
     * negate 'right' if possible
     *******************************************************************/
    if (right->le_val == NOT && rsp->type >= MIN_GT && rsp->type < MAX_GT) {
	right->le_val = NOT^NOT;
	op_not(right);			/* double negate simple gate (produced by function) */
	var->type = rsp->type;		/* adjust to negated type gate */
    }
    t_first = rl->f; t_last = rl->l;	/* full text of expression */
    /********************************************************************
     * Cloned optimised function call is now merged as ARN in op_push()
     *******************************************************************/
    if (rsp->type == ARNF && (iC_optimise & 02)) {
	assert(var->type == ARNF);
	var->type = rsp->type = ARN;	/* no longer ARNF */
    }
    /********************************************************************
     * right Symbol must remain intact until after reduction, because
     * there may be a reference to it which is tested in if(gp == rsp).
     *******************************************************************/
    if (right->le_val == (NOT^NOT)) {
	var->u_blist = rsp->u_blist;	/* move blist from rsp to var */
	templist = rsp->next;		/* bypass rsp */
    } else if (right->le_val != NOT) {
	execerror("attempting to assign delay ???", sr->name, __FILE__, __LINE__);
    } else {
	/* make var an ALIAS because of negation of a builtin function */
	rsp->ftype = GATE;		/* may be odd value from gen */
	var->type = ALIAS;		/* make var negated ALIAS */
	var->u_blist = right;		/* link var to right */
    }
    /********************************************************************
     * Normal reduction of expression to be assigned to 'var'
     *******************************************************************/
    atn = 0;
    sp = var;				/* start reduction with var */
    assert((t_first == 0 && t_last == 0) || (t_first >= iCbuf && t_last >= t_first && t_last < &iCbuf[IMMBUFSIZE]));
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "resolve \"%s\" to \"%s\"\n", t_first, t_last);
#endif
    do {				/* marked symbol */
	List_e *	saveBlist = 0;	/* prevent warning - only used when iFunSymExt != 0 */
	List_e **	saveAdress;
	List_e **	calpp;
	List_e *	plp;
	Symbol *	sp1;
	Symbol *	gp1;
	char *		cp;
	int		gt_input;
	int		gt_count;
	int		len;
	unsigned int	val;
	int		cFn = 0;
	int		z1;
	int		y;

	ePtr = eBuf;			/* temporary expression buffer pointer */
	eEnd = &eBuf[EBSIZE];		/* end of temporary expression buffer */
	cPtr = 0;			/* temporary clone expression out buffer pointer (set later) */
	cEnd = &cBuf[EBSIZE];		/* end of temporary expression buffer */
	gPtr = gBuf;			/* temporary clone expression in buffer pointer */
	tOut = tPtr = tBuf;		/* temporary text expression buffer pointers */
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    memset(eBuf, '\0', EBSIZE);
	    memset(cBuf, '\0', EBSIZE);
	    memset(gBuf, '\0', EBSIZE);
	    memset(tBuf, '\0', EBSIZE);
	}
#endif
	/********************************************************************
	 * Prepare a list of input Symbol pointers in caList[].s for
	 * copyArithmetic, while the list is not reversed yet to allow
	 * the C code index to be resolved by name for multiple occurences.
	 *
	 * Leave out NCONST for GLOBAL reduction - count NCONST for CLONE
	 *******************************************************************/
	if (sp->type == ARN || sp->type == ARNF) {
	    memset(caList, '\0', (PPGATESIZE+1) * sizeof(SymListP));	/* clear for check */
	    gt_count = gt_input = 0;			/* output pre-scan for 1 gate */
	    for (lp = sp->u_blist; lp; lp = lp->le_next) {
		gp = lp->le_sym;
		val = lp->le_val;
		if (gp->ftype == ARITH &&
		    val != (unsigned)-1 &&
		    ((gp->type != NCONST && ++gt_input) || val)) {
		    gt_count++;				/* count all gate inputs */
		    assert(gt_count <= PPGATESIZE);
		    if (gp->type != NCONST) {
			caList[gt_count].l = lp;	/* for re-numbering optimisation */
		    }
		    caList[gt_count].s = gp;		/* for checking duplicates */
		    caList[gt_count].x = gt_input;	/* for indexing without NCONST */
		}
	    }
	}
	if (iC_debug & 04) {
	    fprintf(iC_outFP, "\n");
	}
	gt_count = gt_input = 0;			/* output scan for 1 gate */
	if (iFunSymExt) {
	    saveBlist = sp->u_blist;
	    saveAdress = &saveBlist;
	}
	lp = 0;
	fflag = (sp->type == SW || sp->type == CF) ? 1 : 0;
	sflag &= ~01;				/* stop loop in case of error */
	ilp = 0;
	while ((plp = lp, lp = sp->u_blist) != 0) {
	    sp->u_blist = lp->le_next;
	    if ((gp = lp->le_sym) == rsp && var->type != ALIAS) {
		gp = var;			/* link points to right */
	    }
	    assert(gp && gp->type < IFUNCT);	/* allows IFUNCT to use union v.cnt */
	    if (gp->type == NCONST && lp->le_val != (unsigned)-1 && (sflag & 01) == 0) {
		if ((val = lp->le_val >> 8) > 0) {
		    if (cFn == 0) {
			cFn = val;		/* case number from function */
			assert(cFn > 0 && cFn < functionUseSize);
			cp = functionUse[cFn].c.expr;	/* C expression prepared for cloning */
			len = strlen(cp);
			assert(cBuf + len < cEnd);
			strncpy(cBuf, cp, len+1);	/* copy cloning expression + terminator */
			cPtr = cBuf;		/* temporary cloned out expression buffer pointer */
		    } else {
			assert(cFn == val);	/* other members must be the same */
		    }
		    gt_input++;			/* count NCONST if cloned */
		}
		if (gp == iconst) {		/* ignore iConst if it was injected at end */
		    ilp = lp;			/* save link with its function number */
		} else {			/* not iConst */
		    copyArithmetic(lp, sp, gp, gt_input, 04, cFn);	/* insert numeric text */
		    if (iFunSymExt) {
			assert(saveAdress);
			*saveAdress = lp->le_next;
		    }
		    sy_pop(lp);			/* pop all NCONST links */
		}				/* link in caList is 0 */
		lp = plp;
		goto nextInputLink;
	    }
	    if (iFunSymExt) {
		saveAdress = &lp->le_next;
	    }
	    if (
#if ! YYDEBUG || defined(SYUNION)
		(nlp = 0, gp->v_cnt) > 2 &&
#endif
		(nlp = gp->v_elist) != 0 && (gp->name == 0
#if YYDEBUG
							   || (gp->name)[0] == '@'
#endif
	    )) {			/* not marked symbol */
		/********************************************************************
		 * A feedback element has occurred in the middle of an expression
		 * reduction. Since this element will not be assigned to a differnt
		 * var, the feedback list can be removed now.
		 *******************************************************************/
#if ! YYDEBUG || defined(SYUNION)
		gp->v_cnt = 2;			/* clear the feedback list but retain high use count */
#else
		gp->v_elist = 0;		/* clear the feedback list */
#endif
		while (nlp) {			/* list with feedback to this Symbol */
		    tlp = nlp->le_next;		/* next link to a feedback Symbol */
		    sy_pop(nlp);		/* delete a feedback list link */
		    nlp = tlp;
		}
	    }
	    if (! fflag || ! plp) {		/* bypass first link for if else or switch */
		while (gp->type == ALIAS) {	/* not for var ALIAS */
		    if ((tlp = gp->list) == 0 && (tlp = gp->u_blist) == 0) {
			assert(0);		/* ALIAS without left or right link */
		    }
		    lp->le_val ^= tlp->le_val;	/* negate if necessary */
		    gp = tlp->le_sym;		/* point to original */
		    assert(gp);
		}
		if (iFunSymExt && (iC_debug & 020000) &&
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
			gp = atSymbol();
			gp->type = sp->type;
			gp->ftype = GATE;
			sp->type = OR;		/* OR default for 1 input */
			gp->next = templist;	/* put at front of list */
			templist = gp;
			gp->u_blist = tlp = sy_push(gp);	/* link self */
			tlp->le_next = sp->u_blist;		/* rest of inputs on sp */
			sp->u_blist = 0;			/* this link scan now complete */
			if (iFunSymExt) {
			    lp->le_sym = gp;	/* for function definition */
			    lp->le_next = 0;
			}
		    }
		}
		if (gp->ftype == ARITH &&	/* && gp->u_blist TODO */
		    sp->ftype != OUTW &&
		    sp->type != ALIAS) {
		    if ((val = lp->le_val) == (unsigned)-1) {
			gt_input--;		/* delay is not an arithmetic input */
			gt_count--;
		    } else if (val == 0){
			lp->le_val = ((c_number + 1) << 8)	/* arithmetic case number */
				     + gt_input + 1;	/* arithmetic input number */
		    } else {
			val >>= 8;
			if (cFn == 0) {
			    cFn = val;		/* case number from function */
			    assert(cFn > 0 && cFn < functionUseSize);
			    cp = functionUse[cFn].c.expr;	/* C expression prepared for cloning */
			    len = strlen(cp);
			    assert(cBuf + len < cEnd);
			    strncpy(cBuf, cp, len+1);	/* copy cloning expression + terminator */
			    cPtr = cBuf;			/* temporary cloned out expression buffer pointer */
			} else {
			    assert(cFn == val);		/* other members must be the same */
			}
		    }
		}
		if (iFunSymExt == 0) {		/* GLOBAL REDUCTION */
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
			sp->list = lp;		/* link ALIAS to right */
		    } else {
			/* link Symbols to the end of gp->list to maintain order */
			if (! fflag) {
			    lp->le_next = 0;	/* unless if - else or switch */
			}
			if ((tlp = gp->list) == 0) {
			    gp->list = lp;		/* this is the first Symbol */
			} else {
			    /********************************************************************
			     * loop to find duplicate link (possibly inverted)
			     * start at beginning and test for duplicate up to end
			     * put new Symbol after last unless duplicate
			     *******************************************************************/
			  loop:				/* special loop with test in middle */
			    if (tlp->le_sym == sp) {
				if (gp->ftype == OUTW || gp->ftype == OUTX) {
				    warning("input equals output at output gate:", gp->name);
				} else if (gp->ftype == ARITH || tlp->le_val == lp->le_val) {
				    if (gp != iconst) {
					copyArithmetic(lp, sp, gp, tlp->le_val & 0xff, 02, cFn);
					gt_count++;	/* count duplicates except iConst */
				    }
				    if (sp->type == ARN || sp->type == ARNF) {
					calpp = &caList[gt_count].l;
					assert(*calpp == lp);
					*calpp = 0;	/* remove link to freed List_e */
				    }
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
		} else				/* FUNCTION BLOCK REDUCTION */
		if (sp->type != ALIAS) {		/* ALIAS linked to u_blist is OK */
		    /* test very carefully so no global Symbols are linked */
		    if (gp->list == 0 && gp->u_blist &&
			((typ1 = gp->type) == ARN || typ1 == ARNF)) {
			gp->list = lp;			/* reverse first link for arith text */
		    }
		    /********************************************************************
		     * loop to find duplicate link (possibly inverted)
		     * start at beginning and test for duplicate up to current link
		     *******************************************************************/
		    for (tlp = saveBlist; tlp && tlp != lp; tlp = tlp->le_next) {
			if (tlp->le_sym == gp) {
			    if (gp->ftype == OUTW || gp->ftype == OUTX) {
				warning("input equals output at output gate:", gp->name);
			    } else if (gp->ftype == ARITH || tlp->le_val == lp->le_val) {
				assert(plp);		/* cannot be first link in chain */
				plp->le_next = sp->u_blist;	/* relink the chain */
				if (gp->type == ARN || gp->type == ARNF) {
				    gp->fm |= 0x02;	/* double usage in function arithmetic expression */
#if YYDEBUG
				    if ((iC_debug & 0402) == 0402) {
					fprintf(iC_outFP, "op_asgn:  use count %d for %s\n", gp->fm & FU, gp->name);
					fflush(iC_outFP);
				    }
#endif
				}
				if (gp != iconst) {
				    copyArithmetic(lp, sp, gp, tlp->le_val & 0xff, 02, cFn);
				    gt_count++;		/* count duplicates except iConst */
				}
				if (sp->type == ARN || sp->type == ARNF) {
				    calpp = &caList[gt_count].l;
				    assert(*calpp == lp);
				    *calpp = 0;		/* remove link to freed List_e */
				}
				sy_pop(lp);		/* ignore duplicate link */
				lp = plp;
				goto nextInputLink;
			    } else {
				warning("gate has input and inverse:", gp->name);
			    }
			}
		    }
		}					/* end of function reduction */
	    }
	    if (gp->ftype < MIN_ACT) {			/* recognise UDF clock & timer */
		gt_input++;				/* count the gate inputs */
		gt_count++;				/* count all inputs */
	    }
	    if (! gp->name
#if YYDEBUG
			   || (gp->name)[0] == '@'
#endif
	    ) {						/* not marked symbol */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    if (iC_debug & 04) {
			fprintf(iC_outFP, "%s =", gp->name);
		    } else {
			fprintf(iC_outFP, "\t  %s cleared\n", gp->name);
		    }
		    free(gp->name);			/* free name space "@x" */
		}
#endif
		/********************************************************************
		 * Symbol is marked by writing a new name
		 * do not use a generated name used somewhere else already
		 *******************************************************************/
		do {
		    snprintf(temp, TSIZE, "%s_%d", var->name, ++atn);
		} while (lookup(temp) != 0);
		gp->name = iC_emalloc(strlen(temp)+1);	/* +1 for '\0' */
		strcpy(gp->name, temp);			/* mark Symbol */
	    }
	    if (iC_debug & 04) {
		if (fflag && plp) {
		    int		use;
		    /* reference to a C fragment variable */
		    assert(lp->le_val & 0xff);
		    use = lp->le_val >> USE_OFFSET;
		    assert(use < Sizeof(iC_useText));
		    fprintf(iC_outFP, "\t%s\t%c<---%c\t\t\t// %d %s", gp->name, iC_fos[gp->ftype],
			iC_os[sp->type], lp->le_val & 0xff, iC_useText[use]);
		    if (liveDisp) {
			fprintf(iC_outFP, "\t%s\t=", gp->name);
		    }
		} else
		if ((typ = gp->type) >= MAX_LV) {
		    fprintf(iC_outFP, "\t%s\t%c ---%c", gp->name, iC_os[typ],
			iC_os[sp->type]);
		} else
		if (gp->ftype < MAX_AR && lp->le_val == (unsigned)-1) {
		    /* reference to a timer value - no link */
		    fprintf(iC_outFP, "\t%s\t%c<---%c", gp->name, iC_fos[gp->ftype], iC_os[sp->type]);
		    if (liveDisp) {
			fprintf(iC_outFP, "\t\t\t\t//\t%s\t=", gp->name);
		    }
		} else
		if (gp->ftype != GATE) {
		    fprintf(iC_outFP, "\t%s\t%c ---%c", gp->name, iC_fos[gp->ftype], iC_os[sp->type]);
		} else {
		    if (sp->type == ALIAS) iFlag = 1;	/* may need correction by pplstfix */
		    fprintf(iC_outFP, "\t%s\t%c ---%c", gp->name, w(lp), iC_os[sp->type]);
		}
		if (sflag & 0200) {
		    fprintf(iC_outFP, "\t%s", sp->name);
		    if (sp->ftype != GATE) {
			fprintf(iC_outFP, "\t%c", iC_fos[sp->ftype]);
		    }
		}
		if (gp->ftype == F_SW || gp->ftype == F_CF || gp->ftype == F_CE) {
		    /********************************************************************
		     * case number of "if" or "switch" C fragment
		     *******************************************************************/
		    fprintf(iC_outFP, "\t// (%d)", lp->le_val >> 8);
		    if (liveDisp) {
			fprintf(iC_outFP, "\t%s\t=", gp->name);
		    }
		} else
		if ((gp->ftype == TIMR && lp->le_val > 0)) {
		    /********************************************************************
		     * Timer preset off value
		     *******************************************************************/
		    fprintf(iC_outFP, "%d", lp->le_val);
		}
	    }
	    copyArithmetic(lp, sp, gp, gt_input, sflag, cFn); /* with current expression */
	    sflag &= ~0200;				/* iCdebug & 0200 old style listing removed */
	    if (sp == gp && (sp->type != LATCH || lp->le_val != (NOT^NOT))) {
		warning("input equals output at gate:", sp->name);
	    }
	  nextInputLink:
	    if (sp->u_blist == 0 && gp->ftype < MIN_ACT && gt_count == 0) {
		sp->u_blist = lp = ilp ? ilp : sy_push(iconst);	/* only NCONST's - inject "iConst" */
		caList[1].l = lp;			/* for re-numbering optimisation */
		caList[1].s = iconst;			/* for checking duplicates */
		caList[1].x = 1;			/* for indexing */
		ilp = 0;
		if (iFunSymExt) *saveAdress = lp;
		assert((sflag & 01) == 0);		/* stop loop in case of error */
		sflag |= 01;				/* may only occurr once per node */
		gt_input = 0;				/* now iConst is output with // 1 */
	    }
	}						/* end output scan for 1 gate */
	if (ilp) {
	    sy_pop(ilp);				/* iConst in expression was merged */
	}
	if (iFunSymExt) sp->u_blist = saveBlist;
	if ((typ1 = sp->type) == ARN || typ1 == ARNF) {
	    /********************************************************************
	     * copy rest of arithmetic expression from tOut and finalise C-Code
	     *******************************************************************/
	    if (cFn == 0) {			/* GLOBAL TAIL */
		if (t_first) {
		    assert(t_first >= iCbuf && t_last >= t_first && t_last < &iCbuf[IMMBUFSIZE]);
		    stxFlag = 0;
		    while (t_first < t_last) {
			if ((c = *t_first++ & 0xff) == STX) {
			    stxFlag++;			/* block copying call text */
			}
			if (stxFlag == 0 && c != '#') {	/* transmogrified '=' */
			    if (c >= 0x80) {
				z1 = c - 0x80;		/* insert duplicate node name not merged */
				sp1 = vaList[z1].v;
				assert(sp1);
				cp = sp1->name;		/* this variable to tBuf including NCONST */
				len = strlen(cp);
				if (iFunSymExt) {
				    sp1->fm |= 0x02;	/* double usage in function arithmetic expression */
#if YYDEBUG
				    if ((iC_debug & 0402) == 0402) {
					fprintf(iC_outFP, "op_asgn:  use count %d for %s\n", sp1->fm & FU, sp1->name);
					fflush(iC_outFP);
				    }
#endif
				}
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    int len1 = vaList[z1].l - vaList[z1].f;
				    fprintf(iC_outFP, "GT: z1=%2d	%-15s %-6s v_cnt = %2d [%*.*s] '%1.1s' {%*.*s}\n",
					z1, sp1->name, iC_full_type[sp1->type],
					sp1->v_cnt, len, len, cp, t_first-1, len1, len1, vaList[z1].f);
				    fflush(iC_outFP);
				}
#endif
				tPtr = strncpy(tPtr, cp, len+1) + len;
				if (sp1->type == NCONST) {
				    ePtr = strncpy(ePtr, cp, len+1) + len;
				    gPtr = strncpy(gPtr, cp, len+1) + len;
				} else {
				    for (y = 1; y <= PPGATESIZE; y++) {
					if (sp1 == caList[y].s) break;
				    }
				    assert(y <= PPGATESIZE);
				    if (iC_debug & 04) {	/* repeated term */
					fprintf(iC_outFP, "\t\t\t\t\t%s\t// %d", tOut, y);
					if (liveDisp) {
					    fprintf(iC_outFP, "\t%s\t=", gp->name);
					}
					fprintf(iC_outFP, "\n");
				    }
				    tOut = tPtr;
				    ePtr += sprintf(ePtr, "iC_MV(%d)", y);
				    gPtr += sprintf(gPtr, "\xff%c", y);	/* bound to fit since shorter */
				}
			    } else
			    *tPtr++ = *gPtr++ = *ePtr++ = c;
			}
			if (c == ETX) {
			    assert(stxFlag > 0);
			    --stxFlag;			/* end of call text */
			}
		    }
		    assert(stxFlag == 0);
		}
		*tPtr = *gPtr = *ePtr = '\0';		/* terminate expression strings */
	    } else {				/* CLONED TAIL */
		len = strlen(cPtr);			/* code tail from cloning expression */
		tPtr = strncpy(tPtr, cPtr, len+1) + len;
		ePtr = strncpy(ePtr, cPtr, len+1) + len;
		gPtr = strncpy(gPtr, cPtr, len+1) + len;
		cPtr += len;
	    }					/* END TAIL */
	    if (sp->ftype != OUTW) {			/* output cexe function */
		assert(gp);				/* gp must be initialised */
		if (strcmp(gBuf, "\xff\x01") == 0) {
		    assert(strlen(tOut) == 0);
		    plp->le_val = 0;		/* correct iC_l_[] generation */
		} else {
		    if (cFn == 0) {		/* GLOBAL TRANSFER */
			z1 = ++c_number;
			if ((iC_optimise & 04) != 0) {	/* for full optimising */
			    for (z1 = 1; z1 < c_number; z1++) {
				if (functionUse[z1].c.expr && strcmp(gBuf, functionUse[z1].c.expr) == 0) {
				    break;		/* identical expression found */
				}
			    }
			}
			if (z1 == c_number ||		/* new expression */
			    (iC_gflag && functionUse[z1].c_cnt && (z1 = c_number))) {
			    functionUse[0].c_cnt |= F_ARITHM;  /* flag for copying macro */
			    writeCexeString(T1FP, z1); /* and record for copying */
			    if (lineno == 0) {
				assert(z1 <= iC_genCount);
				fprintf(T1FP, "%s\n", genLines[z1]);
			    } else {
				fprintf(T1FP, "#line %d \"%s\"\n", lineno, inpNM);
			    }
			    fprintf(T1FP, "	return %s;\n", eBuf);
			    fprintf(T1FP, "%%##\n%s", outFlag ? "}\n\n" : "\n");
			    if (iFunSymExt ||
				(iC_optimise & 04) != 0) {	/* save expression for full optimising as well */
				len = strlen(gBuf);		/* C expression prepared for cloning */
				assert(len);
				cp = iC_emalloc(len+1);	/* +1 for '\0' */
				strncpy(cp, gBuf, len+1);
				assert(z1 && z1 < functionUseSize);
				functionUse[z1].c.expr = cp;
			    }
			} else {
			    c_number--;			/* repeated expression */
			    goto adjustLinks;
			}
		    } else {			/* CLONED TRANSFER */
			z1 = cFn;
			if ((strcmp(gBuf, functionUse[z1].c.expr)) != 0 ||	/* has C expression changed ? */
			    iC_gflag) {			/* or clone indpendent functions for gdb debugging */
			    z1 = ++c_number;			/* YES */
			    if ((iC_optimise & 04) != 0) {	/* for full optimising */
				for (z1 = 1; z1 < c_number; z1++) {
				    if (functionUse[z1].c.expr && strcmp(gBuf, functionUse[z1].c.expr) == 0) {
					break;		/* identical expression found */
				    }
				}
			    }
			    if (z1 == c_number ||	/* new modified expression */
				(iC_gflag && functionUse[z1].c_cnt && (z1 = c_number))) {
				functionUse[0].c_cnt |= F_ARITHM;  /* flag for copying macro */
				writeCexeString(T1FP, z1); /* and record for copying */
				assert(lineno > 0);		/* no nested calls in pre-defines */
				fprintf(T1FP, "#line %d \"%s\"\n", lineno, inpNM);
				fprintf(T1FP, "	return %s;\n", eBuf);
				fprintf(T1FP, "%%##\n%s", outFlag ? "}\n\n" : "\n");
				if (iFunSymExt ||
				    (iC_optimise & 04) != 0) {	/* save expression for full optimising as well */
				    len = strlen(gBuf);		/* C expression prepared for cloning */
				    assert(len);
				    cp = iC_emalloc(len+1);	/* +1 for '\0' */
				    strncpy(cp, gBuf, len+1);
				    assert(z1 && z1 < functionUseSize);
				    functionUse[z1].c.expr = cp;
				}
			    } else {
				c_number--;		/* repeated modified expression */
			    }
			  adjustLinks:
			    len = (z1 << 8) + 1;	/* new initial expression reference */
			    for (y = 1; (gp1 = caList[y].s) != 0; y++) {
				if ((lp = caList[y].l) != 0 &&	/* skip duplicate links */
				    (val = lp->le_val) > NOT &&	/* only arithmetic refs */
				    val != (unsigned)-1 &&		/* skip delays */
				    gp1->ftype > UDFA &&
				    gp1->ftype < MAX_ACT &&
				    (gp1->type != NCONST || gp1 == iconst)) {
				    lp->le_val = len++;	/* update expression reference */
				}
			    }
			    assert(y <= PPGATESIZE);
			}				/* cloned expression has not changed */
		    }				/* END TRANSFER */
		    if (iFunSymExt == 0) {			/* defer freeing till called */
			functionUse[0].c_cnt |= F_CALLED;	/* flag for copying temp file */
			functionUse[z1].c_cnt++;		/* free this function for copying */
		    }
		    if (iC_debug & 04) {
			fprintf(iC_outFP, "\t\t\t\t\t%s;\t// (%d)", tOut, z1);
			if (liveDisp) {
			    fprintf(iC_outFP, "\t%s\t=", sp->name);
			}
			fprintf(iC_outFP, "\n");
		    }
		}
	    }
	}
	sflag |= 0200;					/* print output name */
	if (gt_count > PPGATESIZE) {
	    ierror("too many inputs on gate:", sp->name);
	    if (! iFunSymExt) sp->type = ERR;		/* cannot execute properly */
	}
	if ((gp = sp = templist) != 0) {
	    if (sp->name
#if YYDEBUG
			 && (sp->name)[0] != '@'
#endif
	    ) {						/* marked symbol is first */
		templist = sp->next;			/* by_pass marked symbol */
	    } else {
		while ((sp = sp->next) != 0 && (!sp->name
#if YYDEBUG
							  || (sp->name)[0] == '@'
#endif
		)) {
		    gp = sp;				/* look for marked symbol */
		}
		if (sp) {
		    gp->next = sp->next;		/* by_pass marked symbol */
		}
	    }
	    if (sp) {
		if (iFunSymExt == 0) {
		    if (lookup(sp->name) == 0) {
			link_sym(sp);	/* place sp in the symbol table (changes sp->next) */
		    }
		} else {
		    sp->next = varList;	/* put newly marked Symbol on function varList */
		    varList = sp;	/* which is used to reconstitute templist when cloned */
		}
		if ((typ1 = sp->type) == ARN || typ1 == ARNF) {
		    tlp = sp->list;
		    assert(tlp);
		    t_first = tlp->le_first;
		    t_last = tlp->le_last;
		    if (sp->v_cnt > 1 && t_first && (c = *t_first & 0xff) >= 0x80) {
			assert(t_last == t_first + 1);
			c -= 0x80;
			assert(sp == vaList[c].v);
#if YYDEBUG
			cp = t_first;			/* for debug output */
#endif
			t_first = vaList[c].f;
			t_last = vaList[c].l;
#if YYDEBUG
			if ((iC_debug & 0402) == 0402) {
			    len = t_last - t_first;
			    assert(len < IMMBUFSIZE);
			    fprintf(iC_outFP, "SP: c=%2d	%-15s %-6s v_cnt = %2d '%1.1s' {%*.*s}\n",
				c, sp->name, iC_full_type[sp->type], sp->v_cnt, cp, len, len, t_first);
			    fflush(iC_outFP);
			}
#endif
		    }
		    assert((t_first == 0 && t_last == 0) || (t_first >= iCbuf && t_last >= t_first && t_last < &iCbuf[IMMBUFSIZE]));
		}
	    }
	}
    } while (sp);			/* 1 symbol resolved */
    if ((t_last = right->le_last, t_first = right->le_first) != 0) {	/* full text of expression */
	assert(t_first >= iCbuf && t_last >= t_first && t_last < &iCbuf[IMMBUFSIZE]);
	memset(t_first, '#', t_last - t_first);	/* mark embedded assignments */
    }					/* before right is freed */
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
	if ((iC_debug & 0402) == 0402) {
	    free(rsp->name);		/* free name space of @x */
	}
#endif
	free(rsp);			/* free right Symbol */
    }
    /********************************************************************
     * A Symbol is marked by storing a pointer value in ->name
     * which points to a string which does not start with @.
     * Only marked symbols are reduced.
     * Any remaining Symbols on 'templist' must belong to an outer
     * assignment which will be reduced later.
     *******************************************************************/
    if (iFunSymExt) {
	var->list = var->u_blist;	/* save in ->list till end of function definition */
	var->u_blist = 0;		/* must be cleared for op_push like normal op_asgn */
	if (var->type == SW || var->type == CF) {
	    ffexprFlag++;		/* suppress undefined warning in functions till end */
	}
    } else
    if (sv == 0) {
	if (var->ftype == F_ERR) {
	    var->type = ERR;
	} else {
	    lp = sr->list;		/* link action to temp */
	    assert(lp && lp->le_sym == var);
	    lp->le_sym = 0;		/* erase reference to temp either way */
    #if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "op_asgn:  %s deleted\n\n", var->name);
		fflush(iC_outFP);
	    }
    #endif
	    free(var->name);		/* free name space */
	    free(var);			/* temporary Symbol */
	    var = 0;
	}
    }
    if (iC_debug & 04) fprintf(iC_outFP, "\n");
    return var;
} /* op_asgn */

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
	assert(symp->list);
	symp = symp->list->le_sym;	/* with token of original */
    }
    tp = symp->type;
    return (tp >= MAX_GT) ? ((tp == SH || tp == NCONST || tp == INPW) ? ARN : OR)
			  : (tp == UDF ||
			    symp->u_blist == 0 ||
    (symp->list &&
    ((ft = symp->list->le_sym->ftype) == OUTX || ft == OUTW))) ? iC_types[symp->ftype]
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
    Lis* aex, Lis* crx,			/* expression and clock pair */
    Lis* cr3,				/* default clock to clone if no own clock */
    List_e* lp3,
    unsigned char ft,			/* 0 or S_FF or R_FF */
    List_e** alp1)
{
    List_e *	lp1;
    List_e *	lp2;
    List_e *	lpc;
    Symbol *	sp;

    /* lpc is either own clock crx->v or clock cloned from cr3->v or iClock */
    lp1 = 0;
    lpc = crx ? crx->v			/* individul clock or timer crx */
	      : cr3 && cr3->v && (sp = cr3->v->le_sym)
		    ? sy_push(sp->ftype != CLCKL && (lp1 = sp->u_blist)
			? lp1->le_sym	/* or clone last timer cr3 */
			: sp)		/* or clone last clock cr3 */
		    : sy_push(iclock);	/* or clone default clock iClock */
    if (lp1 && (sp = lp1->le_sym) != 0 && sp->ftype == TIMRL) {
	lp1 = lp1->le_next;		/* type TIM, UDF or ALIAS */
	assert(lp1);			/* clone associated timer value */
	assert(lp1->le_val == (unsigned)-1);
	lp2 = sy_push(lp1->le_sym);	/* lp1 has changed */
	lp2->le_val = (unsigned)-1;	/* mark link as -1 timer before op_push */
	assert(lp1->le_first == 0 || lp1->le_first >= iCbuf && lp1->le_last < &iCbuf[IMMBUFSIZE]);
	lp2->le_first = lp1->le_first;	/* copy expr text */
	lp2->le_last  = lp1->le_last;	/* copy expr termination */
	lpc = op_push(lpc, TIM, lp2);
    }
    lp1 = op_push(sy_push(fname->v), bTyp(aex->v), aex->v);
    assert(aex->f == 0 || aex->f >= iCbuf && aex->l < &iCbuf[IMMBUFSIZE]);
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
    *alp1 = op_push(lpc, lp1->le_sym->type, lp1);	/* return lp1 for pVal */
    lp2 = op_push((List_e *)0, iC_types[lp1->le_sym->ftype], lp1);
    return lp3 ? op_push(lp3, iC_types[lp3->le_sym->ftype], lp2) : lp2;
} /* para_push */

/********************************************************************
 *
 *	Common code to generate built in iC functions
 *
 *******************************************************************/

List_e *
bltin(
    Sym* fname,				/* function name and ftype */
    Lis* ae1, Lis* cr1,			/* expression */
    Lis* ae2, Lis* cr2,			/* optional set */
    Lis* ae3, Lis* cr3,			/* optional reset */
    Lis* crm,				/* optional mono-flop clock */
    Val* pVal)				/* optional cblock# or off-delay */
{
    List_e *	lp1;
    List_e *	lp2;
    List_e *	lp3;

    if (ae1 == 0 || ae1->v == 0) {
	warning("first parameter missing. builtin: ", fname->v->name);
	return 0;			/* YYERROR in fexpr */
    }
    lp3 = para_push(fname, ae1, cr1, cr3, 0, 0, &lp1);	/* lp1 needed for pVal */

    if (ae2) {
	if (ae2->v == 0) {
	    warning("second parameter missing. builtin: ", fname->v->name);
	    return 0;			/* YYERROR in fexpr */
	}
	lp3 = para_push(fname, ae2, cr2, cr3, lp3, S_FF, &lp2);
    }

    if (ae3) {
	if (ae3->v == 0) {
	    warning("third parameter missing. builtin: ", fname->v->name);
	    return 0;			/* YYERROR in fexpr */
	}
	lp3 = para_push(fname, ae3, cr3, 0, lp3, R_FF, &lp2);	/* 0 stops cloning */
    }

    if (crm) {
	/* extra Master for mono-flop is reset fed back from own output */
	lp1 = sy_push(ae1->v->le_sym);	/* use dummy ae1 fill link */
	lp2 = op_push(sy_push(fname->v), UDF, lp1);
	if (lp2->le_sym->ftype == S_FF) {
	    lp2->le_sym->ftype = R_FF;	/* next ftype for SR flip flop*/
	}
	lp2 = op_push(crm->v, lp2->le_sym->type, lp2);
	lp2 = op_push((List_e *)0, iC_types[lp2->le_sym->ftype], lp2);
	lp3 = op_push(lp3, iC_types[lp3->le_sym->ftype], lp2);

	lp1->le_sym = lp3->le_sym;	/* fix link from own */
    }

    if (pVal) {
	/* cblock number for ffexpr or preset off delay for timer */
	lp1->le_val = pVal->v;		/* unsigned int value for case # */
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
    int		ftyp;
    Symbol *	rsp;
    Symbol *	sp = sv->v;

    assert(sp);
    ftyp = sp->ftype;
    if (lv->v == 0) {
	if (ftyp != ARITH)       { errBit(); return 0;	/* YYERROR */ }
	else if (const_push(lv)) { errInt(); return 0;	/* YYERROR */ }
    }
    if (sp->type != UDF) {
	if (sp->type != ERR) {
	    if (ftyp == ARITH) {
		ierror("multiple assignment to imm int:", sp->name);
	    } else if (ftyp == GATE) {
		ierror("multiple assignment to imm bit:", sp->name);
	    } else {
		ierror("multiple assignment to wrong imm type:", sp->name);
	    }
	    sp->type = ERR;			/* cannot execute properly */
	}
	if (iFunSymExt) sp->list = 0;		/* do 2nd assignment for listing */
    }
    rsp = ((iC_debug & 021000) == 021000) ? sp
			   : op_asgn(sv, lv, ftyp);	/* new: code before Output */
    if (ioTyp >= MAX_ACT) {			/* OUTW or OUTX */
	assignOutput(rsp, ftyp, ioTyp);
    }
    if ((iC_debug & 021000) == 021000)		/* not needed - can be activated with +21000 */
			rsp =  op_asgn(sv, lv, ftyp);	/* old: code after Output */
    if (iFunSymExt) collectStatement(rsp);	/* update varList in definition stmtList */
    return rsp;
} /* assignExpression */

/********************************************************************
 *
 *	Generate and assign an output node of ftype OUTX or OUTW
 *	called from assignExpression() and gram.y immAssignFound()
 *	via listGenOut().
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

void
assignOutput(Symbol * rsp, int ftyp, int ioTyp)
{
    Sym		sy;
    Lis		li;
    char	temp[TSIZE];

    snprintf(temp, TSIZE, "%s_0", rsp->name);
    if ((sy.v = lookup(temp)) == 0) {		/* locate position in ST */
	sy.v = install(temp, UDF, ioTyp);	/* generate output Gate OUTX or OUTW */
	li.v = sy_push(rsp);			/* provide a link to LOUT or AOUT */
	sy.f = sy.l = li.f = li.l = 0;
	if ((li.v = op_push(0, iC_types[ftyp], li.v)) != 0) {
	    li.v->le_first = li.v->le_last = 0;
	}
	op_asgn(&sy, &li, ftyp);		/* Output assignment */
    }
} /* assignOutput */

/********************************************************************
 *
 *	Listing for undefined C variable
 *	Generate and assign output I/O if it is a Q[XBWL]n variable
 *
 *******************************************************************/

void
listGenOut(Symbol * sp)
{
    char *	name;
    char	y1[2];
    int		yn;
    int		ioTyp;

    if (iC_debug & 04) {
	/********************************************************************
	 * compile listing output for undefined C variable
	 *******************************************************************/
	fprintf(iC_outFP, "\n\t\t= ---%c\t%s", iC_os[sp->type], sp->name);
	if (sp->ftype != GATE) {
	    fprintf(iC_outFP, "\t%c", iC_fos[sp->ftype]);
	}
	fprintf(iC_outFP, "\n\n");
    }
    if ((name = sp->name) && sscanf(name, "Q%1[XBWL]%d", y1, &yn) == 2) {
	/********************************************************************
	 * generate and assign output I/O
	 *******************************************************************/
	ioTyp = (y1[0] == 'X') ? OUTX : OUTW;
	assignOutput(sp, sp->ftype, ioTyp);	/* assign to I/O output variable */
    }
} /* listGenOut */

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
    if ((sp = lookup(iC_One)) == 0) {
	sp = install(iC_One, NCONST, ARITH);
    }
    lp = sy_push(sp);
    lp->le_val = (unsigned)-1;			/* mark link as timer value */
    return op_push(tp, TIM, lp);
} /* delayOne */

/********************************************************************
 *
 *	For 'strict' processing check parameter count in 'cCall'
 *	and clean away dummy list elements produced in 'cList'
 *
 *	Algorithm:
 *	The first link in 'cParams' (if there is one) contains the
 *	parameter count collected in cListCount(). If there are no
 *	links, parameter count is 0.
 *	All dummy links (with le_sym == 0) generated in cListCount()
 *	and which are linked in a list to u.blist of the first Symbol
 *	pointed to by cParams must be removed. If all links are dummys
 *	u.blist in the first Symbol will be 0 and this Symbol was a
 *	temp generated in cListCount(). It must be removed from templist
 *	and removed removed with its name and link.
 *
 *	Finally compare the parameter count with v.cnt in 'cName', which
 *	contains the parameter count collected in the extern C function
 *	decleration.
 *
 *******************************************************************/

List_e *
cCallCount(Symbol * cName, List_e * cParams)
{
    if (iC_Sflag) {
	List_e *	lp;
	List_e **	lpp;
	Symbol *	sp;
	int		pcnt = 0;

	if ((lp = cParams) != 0) {
	    sp = lp->le_sym;
	    assert(sp);
	    pcnt = lp->le_val;
	    for (lpp = &sp->u_blist; (lp = *lpp) != 0; ) {
		if (lp->le_sym == 0) {
		    *lpp = lp->le_next;		/* remove dummy link from list */
		    sy_pop(lp);
		} else {
		    lpp = &lp->le_next;		/* next list element */
		}
	    }
	    if (sp->u_blist == 0) {		/* sp has only links which have been popped */
		freeTemp(cParams);		/* bypass sp on templist and delete cParams and sp */
		cParams = 0;
	    } else {
		cParams->le_val = 0;
	    }
	}
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "cCall: %s has %d parameters\n", cName->name, pcnt);
	    fflush(iC_outFP);
	}
#endif
	if (cName->type == UDF) {
	    cName->v_cnt = pcnt;		/* UNDEF stop more error messages */
	    ierror("strict: call of an undeclared C function or macro:", cName->name);
	} else
	if (pcnt != cName->v_cnt) {
	    char	tempBuf[TSIZE];		/* CNAME */
	    snprintf(tempBuf, TSIZE, "%s %d (%d)", cName->name, pcnt, cName->v_cnt);
	    ierror("strict: call parameter count does not match C function declaration:", tempBuf);
	}
    }
    if (cName->type == UDF) {
	cName->type  = CWORD;			/* no longer an imm variable */
	cName->u_val = CNAME;			/* yacc type */
    }
    return cParams;
} /* cCallCount */

/********************************************************************
 *
 *	For 'strict' processing count parameters in 'cList'
 *	If 'aexpr == 0' generate a dummy link with a null pointer
 *	in le_sym for transporting pcnt in le_val. This link will
 *	be removed in 'cCall'
 *	If the first parameter is a dummy (aexpr == 0), a dummy
 *	temp Symbol is put on templist by op_push(), which must
 *	also be removed in 'cCall' if all other parameters are dummys
 *
 *******************************************************************/

List_e *
cListCount(List_e * cPlist, List_e * aexpr)
{
    int		pcnt = 0;

    if (iC_Sflag) {
	if (cPlist) {
	    pcnt = cPlist->le_val;	/* count from cPlist before sy_pop in op_push */
	    cPlist->le_val = 0;		/* restore so that expression is correct */
	}
	if (aexpr == 0) {
	    aexpr = sy_push(0);		/* dummy link for counting */
	}
    }
    aexpr = op_push(cPlist, ARN, op_force(aexpr, ARITH));
    if (iC_Sflag) {
	assert(aexpr);
	aexpr->le_val = pcnt + 1;
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "cPlist: parameter %d\n", aexpr->le_val);
	    fflush(iC_outFP);
	}
#endif
    }
    return aexpr;
} /* cListCount */

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
 *	a value function called "fun", a Symbol named "fun@" is installed
 *	and made known via Sym iRetSymbol to the return statement.
 *	A text buffer iFunBuffer holds the text "fun@" and a pointer
 *	iFunSymExt is set to point just past the '@' symbol.
 *
 *	When scanning the formal parameter list or when declaring immediate
 *	variables in the body of a function (signalled by iFunSymExt
 *	being set), the name of the item is concatenated to the "fun@"
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
 *	2b) <type> <name> for a value parameter, both in the comma separated
 *	    parameter list. The modifier imm is optional before <type>.
 *	3)  imm <type> <name> in the body of the function declares a function
 *	    internal variable which will be cloned with different names in
 *	    different instances, when the function is called.
 *	4)  extern imm <type> <name> means that name will be assigned outside the
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
 *	iFunSymExt points past the '@' in the name and is the place where
 *		   local name extensions can temprarily be written.
 *		   It also flags, that a function is being defined.
 *
 *	For a value function also set up the return Symbol
 *
 *******************************************************************/

Symbol *
functionDefHead(unsigned int ftyp, Symbol * funTrigger, int retFlag)
{
    funTrigger->type = IFUNCT;			/* function head */
    funTrigger->ftype = ftyp;			/* void bit int clock timer */
    iFunSymExt = strncpy(iFunBuffer, funTrigger->name, IBUFSIZE);
    liveDisp = 0;				/* no live display in function generation */
    iFunSymExt += strlen(iFunBuffer);		/* point past text */
    if (iFunEnd - iFunSymExt < 32) {
	execerror("iFunBuffer for function symbol too small", funTrigger->name, __FILE__, __LINE__);
    }
    *iFunSymExt++ = '@';			/* append '@' */
    if (retFlag) {
	if ((iRetSymbol.v = lookup(iFunBuffer)) == 0) {
	    iRetSymbol.v = install(iFunBuffer, UDF, ftyp);	/* return Symbol */
	}
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "iFunHead: imm %s %s\n",
		iC_full_ftype[ftyp], iRetSymbol.v->name);
	    fflush(iC_outFP);
	}
#endif
    } else {
	iRetSymbol.v = 0;			/* void function has no return Symbol */
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "vFunHead: imm %s %s\n", iC_full_ftype[ftyp], funTrigger->name);
	    fflush(iC_outFP);
	}
#endif
    }
    assert(stmtList == 0);			/* function definitions cannot nest */
    return funTrigger;
} /* functionDefHead */

/********************************************************************
 *
 *	Collect statements in the function body
 *
 *	While 'iFunSymExt' is set - marking the function definition phase -
 *	any Symbols reduced in op_asgn() are linked to the head of 'varList'
 *	instead of being installed in the Symbol Table.
 *
 *	In this function the 'funcStatement' is linked to the end of
 *	the statement list unless it is already there because of a
 *	a declaration (in that case 'varList' will be '0'). Either
 *	way 'varList' is linked in with the matching statement. A
 *	declaration already links in a statement entry so that a check
 *	can be made in functionDefinition(), that the matching assignment
 *	has actually been collected and executed (otherwise Symbol is UDF).
 *
 *	The statement list consists of pairs of 'List_e' nodes linked
 *	with each first node pointing to the statement Symbol and the
 *	other node pointing to the head of a 'varList', which consists
 *	of reduced Symbols linked via the 'next' pointer.
 *
 *	This function is also called for cloned statements from a
 *	nested function call.
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
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "collectStatement: %s type: %s, ftype: %s\n",
		    sp->name, iC_full_type[typ], iC_full_ftype[sp->ftype]);
		fflush(iC_outFP);
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
		    slp->le_next = sy_push(varList);	/* link to end of list */
		} else if (varList) {			/* no */
		    /********************************************************************
		     * previous declaration or if multiple assignment, previous 'varList'
		     * is overwritten to match latest version of statement linked tp 'sp'.
		     * If not overwritten, the members of the old 'varList' do not match
		     * the Symbols of the expression net linked to 'sp' and an execerror
		     * results when the expression is cloned - which might not matter
		     * since there is a hard error anyway, but it is untidy.
		     *******************************************************************/
		    vlp->le_sym = varList;	/* no 'varList' in declaration */
		}
	    }
	} else if (sp->em == 0 && typ != ERR) {	/* ignore extern */
	    fprintf(iC_outFP, "type: %s, ftype: %s\n",
		iC_full_type[typ], iC_full_ftype[sp->ftype]);
	    ierror("function statement is not int, bit, clock or timer:", sp->name);
	    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
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
    Symbol *	sp;
    int		ftyp;

    if (iRetSymbol.v) {
	ftyp = iRetSymbol.v->ftype;
	if (actexpr->v == 0) {
	    if (ftyp != ARITH)            { errBit(); return 0;	/* YYERROR */ }
	    else if (const_push(actexpr)) { errInt(); return 0;	/* YYERROR */ }
	}
	if ((sp = actexpr->v->le_sym) == 0) {
	    ierror("no expression to return:", iFunBuffer);
	} else {
	    switch (ftyp) {
	    case ARITH:
	    case GATE:
		if (sp->ftype != ARITH && sp->ftype != GATE) {
		    ierror("wrong return type for int or bit:", iFunBuffer);
		}
		break;
	    case CLCKL:
	    case TIMRL:
		if (sp->ftype != ftyp) {
		    ierror("wrong return type for clock or timer:", iFunBuffer);
		}
		break;
	    default:
		ierror("wrong type for function return:", iFunBuffer);
		break;
	    }
	}
	sp = op_asgn(&iRetSymbol, actexpr, ftyp);
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
    List_e *	lp1;
    List_e *	vlp;
    List_e **	lpp;
    Symbol *	sp;
    Symbol *	sp1;
    Symbol **	spp;
    char *	np;
    int		instanceNum;			/* save early union u.val u.blist */
    int		saveCount = 0;			/* count parameter links for saving */

    instanceNum = iFunHead->v_cnt;		/* read from file if -a option */
    if (iRetSymbol.v) {
	if (iRetSymbol.v->type == UDF) {
	    ierror("no return statement in function:", iFunHead->name);
	}
	assert(iRetSymbol.v->u_blist == 0);	/* state after expression reduction */
	iRetSymbol.v->u_blist = iRetSymbol.v->list;
	iRetSymbol.v->fm |= FM;			/* mark return Symbol - not used */
	iRetSymbol.v->list = sy_push(iFunHead);	/* with own function head */
	iRetSymbol.v = 0;			/* no need to report as undefined */
    }
    iFunSymExt = 0;				/* end of function compilation */
    liveDisp = iC_debug & 0200;			/* not in function generation - live display if -d200 */
    /********************************************************************
     * Pass 1
     * Scan statement list built during compilation of the function body
     * Mark each statement head in fm except the function head which is
     * marked just above.
     * Count space for saving in case of nested definitions.
     * Clear the names of all Symbols of the associated varList.
     *******************************************************************/
    iFunHead->u_blist = lp = stmtList;		/* global value */
    stmtList = 0;				/* ready for next function */
    while (lp) {				/* mark function statement list */
	sp = lp->le_sym;			/* assign parameters and internal declarations */
	assert(sp);
	if (sp->type == UDF && ffexprFlag == 0) {
	    warning("undefined gate in function:", sp->name);
	}
	if (sp->u_blist) {
	    assert(sp->list && sp->list->le_sym == iFunHead);	/* marked return */
	} else {
	    sp->u_blist = sp->list;		/* expression now ready for cloning */
	    sp->list = 0;			/* clear pointer to real Symbol */
	    saveCount++;			/* space for saving nested decl or assign parameter */
	    sp->fm |= FM;			/* mark as function Symbol */
	}
	lp = lp->le_next;			/* next varList link */
	assert(lp);				/* statement list is in pairs */
	sp = lp->le_sym;			/* first varList Symbol */
	while (sp) {				/* varList may be empty */
	    assert((sp->fm & FM) == 0);		/* newly generated expression net Symbol */
	    free(sp->name);			/* free name space generated for listing name */
	    sp->name = 0;			/* mark the Symbol as function internal (no '@') */
	    sp->list = 0;			/* clear internal Symbol pointers */
	    sp = sp->next;			/* next varList Symbol */
	}
	lp = lp->le_next;			/* next statement link */
    }
    /********************************************************************
     * Pass 2
     * Scan the parameter list collected during parsing
     * Mark each formal parameter in fm except assign parameters, which
     * were already marked in the statement scan.
     * Count space for saving in case of nested definitions.
     *
     * If an assign parameter is an alias, swap its symbol with the alias
     * target. This involves relinking both symbols in the Symbol Table.
     *******************************************************************/
    iFunHead->list = lp = fParams;		/* yacc stack value of parameter list */
    while (lp) {				/* mark formal parameter list */
	sp = lp->le_sym;			/* assign and value parameters */
	assert(sp);
	if (sp->type == UDF && ffexprFlag == 0) {
	    warning("undefined parameter in function:", sp->name);
	}
	if (sp->list != 0) {			/* was expression assigned and not on statement list ? */
	    ierror("trying to assign to a value parameter:", sp->name);
	    for (lp1 = sp->list; lp1; lp1 = vlp) {	/* has no follow ups - but just in case */
		vlp = lp1->le_next;			/* next expression link */
		free(lp1);				/* delete expression link */
	    }
	    sp->list = 0;			/* clear pointer to real parameter */
	}
	if (sp->u_blist == 0) {
	    saveCount++;			/* space for saving nested read parameter */
	    sp->fm |= FM;			/* mark as function Symbol */
	} else {
	    assert((sp->fm & FM) != 0);		/* assign was marked in statement list scan */
	    while (sp->type == ALIAS) {
		lp1 = sp->u_blist;
		assert(lp1);
		sp1 = lp1->le_sym;
		assert(sp1);
		if (sp1->type < ARNF || sp1->type == LOGC || sp1->type > LATCH) break;
		np = sp->name;
		assert(np);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "iFunDef:  swap assign alias %s <==> %s\n", np, sp1->name);
		    fflush(iC_outFP);
		}
#endif
		unlink_sym(sp);			/* take named alias symbol out of ST */
		unlink_sym(sp1);		/* take named statement symbol out of ST */
		sp->name = sp1->name;
		link_sym(sp);			/* put back in ST with its new name */
		sp1->name = np;			/* swap names from assign alias to target */
		link_sym(sp1);			/* put back in ST with its new name */
		lp->le_sym = sp1;		/* make target the assign parameter */
		assert((sp1->fm & FM) != 0);	/* target was marked in statement list scan */
		if (lp1->le_val == NOT) {
		    op_not(lp1);		/* negate target expression */
		}
		lp1->le_sym = 0;		/* isolate assignment alias */
		sp = sp1;
	    }
	    sp->fm = FM|0x02;			/* set assign parameter as used - no merge */
	}
	lp = lp->le_next;			/* next formal parameter link */
    }
    /********************************************************************
     * Pass 3
     * Resolve inputs to expressions which are aliases - replace the
     * alias by the target for the alias in the expression recursively.
     *
     * Generate use count for all non-global nodes to simplify optimised
     * cloning. Scan each statement head and all members of its varList.
     *     fm bits 0:1 can be 0	- never used in any expression
     *                        1	- used once in an expression - merge
     *                        2	- used more than once - don't merge
     *     mask FU            3
     * The use count can be used to check, whether a parameter is used
     * and for expression nodes, whether they are candidates for merging.
     * Assign parameters were previously set to 2 - no merging allowed.
     *******************************************************************/
    lp = iFunHead->u_blist;			/* statement list */
    while (lp) {
	sp = lp->le_sym;			/* return, assign parameter or internal declaration */
	lp = lp->le_next;			/* next varList link */
	spp = &lp->le_sym;			/* pointer to first varList Symbol */
	while (1) {
	    for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {
		if ((sp1 = lp1->le_sym) != 0) {	/* input to this target */
		    if (sp1->type == ALIAS) {
			vlp = sp1->u_blist;	/* inversion is taken care of */
			assert(vlp);
			sp1 = vlp->le_sym;
			assert(sp1);
			lp1->le_sym = sp1;	/* transfer alias target to net list element */
		    }
		    if (((sp1->fm & FM) != 0 || sp1->name == 0) &&	/* no globals */
			(sp1->fm & 0x02) == 0) {/* target not used more than once */
			sp1->fm++;		/* count use in merge candidate */
#if YYDEBUG
			if ((iC_debug & 0402) == 0402) {
			    fprintf(iC_outFP, "iFunDef:  use count %d for %s\n", sp1->fm & FU, sp1->name);
			    fflush(iC_outFP);
			}
#endif
		    }
		}
	    }
	    if ((sp = *spp) == 0) break;	/* next varList Symbol */
	    spp = &sp->next;			/* pointer to next varList Symbol */
	}
	lp = lp->le_next;			/* next formal parameter link */
    }
    /********************************************************************
     * Pass 4
     * Delete statement entries which are aliases - they have all been resolved
     * Free the alias Symbol, its leading link pair and the following link
     *******************************************************************/
    lpp = &iFunHead->u_blist;			/* statement list */
    while ((lp = *lpp) != 0) {
	sp = lp->le_sym;			/* return, assign parameter or internal declaration */
	vlp = lp->le_next;			/* next varList link */
	if (sp->type == ALIAS && (sp->list == 0 || sp->list->le_sym != iFunHead)) {
	    lp1 = sp->u_blist;
	    assert(lp1);
	    sp1 = lp1->le_sym;
	    if (sp1 == 0 ||			/* keep out UDF, ARNC and LOGC because ->u_blist == 0 */
		(sp1->type >= ARNF && sp1->type != LOGC && sp1->type <= LATCH)) {
		*lpp = vlp->le_next;		/* bypass statement and varList link */
		sy_pop(lp);
		assert(vlp->le_sym == 0);	/* alias statement should not have a varList */
		sy_pop(vlp);
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "iFunDef:  uninstall alias %s\n", sp->name);
		    fflush(iC_outFP);
		}
#endif
		uninstall(sp);			/* delete alias Symbol */
		sy_pop(lp1);
		saveCount--;
		/* if more levels of alias handle it here */
		continue;			/* go straight to next pair */
	    }
	}
	lpp = &vlp->le_next;			/* next statement link */
    }
    /********************************************************************
     * Pass 5
     * Check that all read parameters have been used - otherwise issue a
     * warning.
     *******************************************************************/
    lp = iFunHead->list;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;			/* assign or read parameter */
	if (sp && (sp->fm & FU) == 0) {
	    warning("function parameter was never used:", sp->name);
	}
	lp = lp->le_next;			/* next formal parameter link */
    }
    /********************************************************************
     * Finally
     * Store instance number and number of slots to save when nesting
     * in the link pair of the first statement.
     * Any sensible function has at least one statement.
     * If not, report a hard error in the definition of the function.
     *******************************************************************/
    lp = iFunHead->u_blist;			/* statement list */
    if (lp) {
	lp->le_val = instanceNum;		/* allows call to number instances */
	lp = lp->le_next;			/* first varList link */
	assert(lp);				/* must be a pair */
	assert(saveCount >= 0);
	lp->le_val = saveCount;			/* allows call to get save block */
    } else {
	ierror("function has no statements!", iFunHead->name);
    }
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "iFunDef:  imm %s %s\n", iC_full_ftype[iFunHead->ftype], iFunHead->name);
	fflush(iC_outFP);
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
    assert(functionHead);
    slp = functionHead->u_blist;		/* start of statement list */
    if (slp == 0) {
	return functionHead;			/* already cleared */
    }						/* instanceNum for dummy from file not changed */
    warning("existing function definition is deleted:", functionHead->name);
    functionHead->u_blist = 0;			/* clear for next definition */
    instanceNum = slp->le_val;			/* this function call instance number */
    while (slp) {
	sp = slp->le_sym;			/* formal satement head Symbol */
	assert(sp);
	lp = sp->u_blist;			/* cloned expression links */
	while (lp) {
	    lp1 = lp->le_next;			/* next expression link */
	    free(lp);				/* delete expression link */
	    lp = lp1;
	}
	lp = sp->list;				/* possible link to function head */
	while (lp) {
	    lp1 = lp->le_next;			/* next expression link */
	    free(lp);				/* delete expression link */
	    lp = lp1;				/* has no follow ups - but just in case */
	}
	uninstall(sp);				/* delete formal statement head Symbol */
	vlp = slp->le_next;			/* next varList link */
	free(slp);				/* delete statement link */
	assert(vlp);				/* statement list is in pairs */
	vsp = vlp->le_sym;			/* varList of temp Symbols */
	while (vsp) {				/* varList may be empty */
	    lp = vsp->u_blist;			/* cloned expression links */
	    while (lp) {
		lp1 = lp->le_next;		/* next expression link */
		free(lp);			/* delete expression link */
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
    slp = functionHead->list;			/* parameter list */
    functionHead->list = 0;			/* clear for next definition */
    while (slp) {
	sp = slp->le_sym;			/* formal parameter Symbol */
	assert(sp && sp->list == 0);		/* call leaves link to real para cleared */
	uninstall(sp);				/* delete formal parameter Symbol */
	vlp = slp->le_next;
	free(slp);				/* delete formal parameter link */
	slp = vlp;
    }
    functionHead->v_cnt = instanceNum;		/* in case instances of old definition */
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
 *	in a different source. Then the source in which they are used
 *	need an extern declaration to prevent them being flagged as
 *	undefined.
 *
 *	iClock is a universal global variable which needs no external
 *	declaration.
 *
 *******************************************************************/

List_e *
checkDecl(Symbol * terminal)
{
    int		typ = terminal->type;

    if (iFunSymExt &&
	strncmp(terminal->name, iFunBuffer, iFunSymExt - iFunBuffer) &&
	terminal->em == 0 &&
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
 *	is scanned and type checked against the list of formal parameters
 *	linked to the function head Symbol. Links from formal to real
 *	parameters and statement heads are via 'list' in the formal Symbol,
 *	which are marked with fm |= FM (0x80) when formal Symbol is generated.
 *	fm&FU == 1 when formal Symbol is used once and 2 when used more than once.
 *	The return Symbol 'list' is linked back to the function head Symbol.
 *
 *	If that is OK, the assignment head list is scanned and each associated
 *	net is cloned, replacing any value parameters by links to the real
 *	parameter Symbols. For each assignment head the unfinished assignment
 *	is now carried out with the cloned net, which gives temporary names
 *	associated with each real assignment variable to internal Symbols.
 *	Also the full logic of assigning to variables which have already been
 *	used as values is thus taken care of. Internal variables do not have
 *	a link in 'list', and a Symbol to assign to must be cloned for them.
 *
 ********************************************************************
 *******************************************************************/

#define	DS	8			/* show 8 elements of isav[] for debugging */

struct sF {
    struct sF *	prevFunBs;		/* base of previous save block or 0 */
    Symbol *	iCallHead;		/* function head seen at start of imm call */
    List_e *	iFormNext;		/* next pointer when scanning formal parameters */
    List_e *	iFunClock;		/* temporary list of unresolved clock parameters */
    List_e *	iSav[DS];		/* dynamic array for saving parameter + statement links */
};

static struct sF *	saveFunBs = 0;	/* base pointer to save block for recursive calls */
static List_e *		iFormNext = 0;	/* next pointer when scanning formal parameters */
static List_e *		iFunClock = 0;	/* temporary list of unresolved clock parameters */
static Symbol *		cloneSymbol(Symbol * sp);
static List_e *		cloneList(Symbol * ssp, Symbol ** cspp, Symbol * rsp, int x);

/********************************************************************
 *
 *	Push a function call
 *
 *	Push parameters of a nested call of this function on stack 'saveFunBs'
 *	Save function template parameter and declared variable links
 *	Value parameter links are saved and cleared in Pass 1.
 *	Assign parameters and Declarations are saved and cleared in Pass 2.
 *
 *******************************************************************/

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

    assert(functionHead);
    lp = functionHead->u_blist;
    if (lp != 0) {
	lp = lp->le_next;			/* first varList link */
	assert(lp);				/* must be a pair */
	saveCount = lp->le_val;			/* allows call to store save block */
	saveFunBs = (struct sF *) iC_emalloc(sizeof(struct sF) + (saveCount - DS) * sizeof(List_e *));
	saveFunBs->prevFunBs = oldSFunBs;
	saveFunBs->iCallHead = iCallHead;	/* will be set up during the call */
	saveFunBs->iFormNext = iFormNext;	/* other variables are saved */
	saveFunBs->iFunClock = iFunClock;	/* in pushFunParameter() */
	/********************************************************************
	 * Pass 1: parameter list
	 *******************************************************************/
	cF = 0;
	lp = functionHead->list;		/* parameter list */
	while (lp) {
	    sp = lp->le_sym;
	    assert(sp);
	    if (sp->u_blist == 0) {
		saveFunBs->iSav[cF++] = sp->list; /* save parameter template link */
		sp->list = 0;			/* clear value parameter link */
	    }
	    lp = lp->le_next;
	}
	/********************************************************************
	 * Pass 2: statement list
	 *******************************************************************/
	lp = functionHead->u_blist;		/* statement list */
	while (lp) {
	    sp = lp->le_sym;
	    assert(sp);
	    sp1 = ((lp1 = sp->list) != 0) ? lp1->le_sym : 0;
	    if (sp1 != functionHead) {		/* bypass return link */
		saveFunBs->iSav[cF++] = sp->list; /* save declaration or assign link */
		sp->list = 0;			/* clear link */
	    }
	    lp = lp->le_next;			/* next varList link */
	    assert(lp);				/* statement list is in pairs */
	    lp = lp->le_next;			/* next statement link */
	}
	assert(cF == saveCount);
	/********************************************************************
	 * set up for cloning
	 *******************************************************************/
	iCallHead = functionHead;		/* make avalable globally */
	assert(iCallHead);			/* must have a function head */
	iFormNext = iCallHead->list;		/* first formal parameter - may be 0 */
	iFunClock = 0;				/* no unresolved clock parameters */
    }
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
 *	Process each real parameter with its formal parameter.
 *	Parameters for imm bit and int may be mismatched, in which case
 *	value parameters will be forced to the ftype of the formal parameter.
 *	Mismatched bit and int assign parameters are forced in op_asgn.
 *	Both value and assign parameters for imm timer must be matched.
 *	See delay handling after timer above.
 *	A value imm clock parameter may be filled by either a clock or a timer
 *	parameter (with optional delay count) or may be missing altogether in
 *	which case it will be the same as the next clock parameter or iClock if
 *	no clock parameter at all is filled in the call parameter list.
 *	An assign clock parameter must be an unassigned variable declared
 *	as imm clock. It can fill in as value for prior missing clocks.
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
	    if ((rsp = lookup(iC_One)) == 0) {	/* default delay 1 after last timer parameter */
		rsp = install(iC_One, NCONST, ARITH);
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
	assert(fsp && fsp->fm);			/* u_blist not cleared on parse error */
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
		rlp->le_val = (unsigned)-1;	/* mark link as timer value */
		while (iFunClock) {		/* find unresolved formal clock parameters */
		    iFunClock->le_sym = psp;	/* link unresloved clock to this timer */
		    clp = iFunClock->le_next;	/* next unresolved clock */
		    iFunClock->le_next = lp1 = sy_push(rsp); /* clone a delay link */
		    lp1->le_val = (unsigned)-1;	/* mark link as timer value */
		    iFunClock = clp;
		}
	    }
	} else {
	    switch (formalType) {
	    case ARITH:
		if (rsp->ftype == GATE) {
		    if (fsp->u_blist == 0) {	/* assign parameter forced in op_asgn */
			rlp = op_force(rlp, ARITH);	/* force value parameter to int */
		    }
		} else
		if (rsp->ftype != ARITH) {
		    ierror("wrong parameter type for int:", rsp->name);
		    sy_pop(rlp);		/* parameter not used */
		    rlp = 0;			/* error */
		}
#if ! YYDEBUG || defined(SYUNION)
		if (rsp->v_cnt <= 2)
#endif
		    rsp->v_cnt = fsp->fm & FU;	/* transfer use count */
		break;
	    case GATE:
		if (rsp->ftype == ARITH) {
		    if (fsp->u_blist == 0) {	/* assign parameter forced in op_asgn */
			rlp = op_force(rlp, GATE);	/* force value parameter to bit */
		    }
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
		if (fsp->u_blist) {		/* assign parameter must be correct ftype */
		    ierror("wrong parameter type for 'assign clock':", rsp->name);
		    sy_pop(rlp);		/* parameter not used */
		    rlp = 0;			/* error */
		    rsp->type = ERR;
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
 *	it is a parameter or assignable, which are marked by ->fm&FU (0x80)
 *
 *	The first List_e in the statement list holds an instance counter for
 *	generated function internal variables - to allow unique naming.
 *	This field le_val is zero when function is first compiled for
 *	each source. When a function is included in several sources
 *	the counter is passed to later runs via _List1.h.
 *
 *******************************************************************/

Lis
cloneFunction(Sym * fhs, Lis * plpl, Str * par)
{
    Symbol *		functionHead = fhs->v;	/* actual function head */
    List_e *		plp = plpl->v;		/* actual rParams */
    Lis			rll;			/* return Lis */
    List_e *		slp;			/* link for statement Symbol */
    List_e *		vlp;			/* link for varList */
    List_e *		lp;
    List_e *		lp1;
    List_e *		lp2;
    List_e **		lpp;
    Symbol *		sp;
    Symbol *		sp1;
    Symbol *		sp2;			/* Symbol in expression template */
    Symbol *		vsp;			/* Symbol in varList */
    Symbol *		csp;			/* Symbol in cloned exprsssion */
    Symbol *		rsp;			/* return Symbol for feedback */
    Sym			sv;			/* Sym to assign cloned expression to */
    Lis			sl;			/* Lis pointing to cloned expression */
    int			instanceNum;		/* this function call instance number */
    int			instanceFlg;
    int			saveCount;		/* saveFunPt,iFormHead,iFormNext,iFunClock */
    int			cF;
    int			typ;
    unsigned int	lval;			/* logical sign of a temp */
    struct sF *		oldSFunBs;
    char *		cp;
    char		temp[TSIZE];

    assert(functionHead);
    rll.v = 0;					/* return link */
    rll.f = fhs->f; rll.l = par->l;		/* adjust old text limits */
    slp = lp = functionHead->u_blist;		/* get 2 numbers in first elements of statement list */
    if (slp == 0) {
	ierror("called function has no statements - cannot execute!", functionHead->name);
	return rll;				/* with 0 pointer */
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
     *         b) clone the return statement head to make it available
     *            to all other expressions.
     *         c) clone the function internal statement heads to make
     *            them available to all other expressions.
     *******************************************************************/
    rsp = 0;					/* return must be set in a value function */
    while (slp) {				/* scan statement list */
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
	 * Clone the expression head associated with this statement
	 * blist == 0 indicates a function internal Symbol
	 * by doing this after varList, cloned head is at front of templist
	 * The remaining internal statement head Symbols are not cloned till Pass2.
	 * They then come even earlier on templist. This is necessary because
	 * these statements are assigned before the return statement.
	 *******************************************************************/
	sp2 = slp->le_sym;			/* expression head template */
	assert(sp2);				/* check in Pass 1 only */
	assert(sp2->name);
	if ((lp = sp2->list) == 0) {		/* function internal variable */
	    if (iFunSymExt) {
		int	n = 0;			/* cloned in a function definition */
		cp = sp2->name + strlen(functionHead->name);	/* locate original extension */
		assert(cp[0] == '@' && isprint(cp[1]));		/* extension must be at least 1 character */
		strncpy(iFunSymExt, cp+1, iFunEnd - iFunSymExt);/* copy ext to new fun name */
		cp = iFunBuffer + strlen(iFunBuffer);		/* end of new var name */
		while (lookup(iFunBuffer) != 0) {
		    /********************************************************************
		     * Different instances of the same extension can occurr for multiple
		     * calls of the same function in one function definition or for the
		     * same name used in different functions called in the definition.
		     * This heuristic assumes there are not hundreds of instances in
		     * one function definition. (Even a very large number will work -
		     * the temporaries generated from it are fun@i1 fun@i2 from fun@i.
		     * The heuristic can handle extensions ending in numerals - it will
		     * keep trying until it finds one with a different numeral.
		     * The heuristic handles multiply nested function calls reasonably.
		     * These names do not touch the instance number until the function
		     * is finally called, when an instance number will be appended.
		     * This scheme also sees to it, that the same template is generated
		     * in each file it is used in - providing the set of functions is the
		     * same. This helps analysis of the generated code.
		     *******************************************************************/
		    snprintf(cp, iFunEnd - cp, "%d", ++n);	/* simply count up */
		}				/* position in ST located in while */
		sv.v = install(iFunBuffer, UDF, sp2->ftype);
		*iFunSymExt = '\0';
		collectStatement(sv.v);		/* put in definition stmtList like a decl */
	    } else {
		/********************************************************************
		 * Naming for global internal names needs instanceNum, because names
		 * may be generated in several independent source files, unlike names
		 * in function definitions above, which are contained in one file.
		 *******************************************************************/
		cp = sp2->name + strlen(functionHead->name);	/* locate original extension */
		assert(cp[0] == '@');		/* has failed during development */
#ifdef OLD_INSTANCE 
		snprintf(temp, TSIZE, "%s_%s_%d", functionHead->name, cp+1, instanceNum);
#else
		snprintf(temp, TSIZE, "%s_%d_%s", functionHead->name, instanceNum, cp+1);
#endif
		instanceFlg++;			/* instanceNum was used so update */
		if ((sv.v = lookup(temp)) == 0) {	/* locate position in ST */
		    sv.v = install(temp, UDF, sp2->ftype); /* Symbol for declared variable */
		}
	    }
	    sp2->list = sy_push(sv.v);		/* link internal cloned Symbol to template */
	} else
	if (lp->le_sym == functionHead) {
	    rsp = cloneSymbol(sp2);		/* clone return expression head Symbol */
	    if (rsp->type == ARN) {
		/********************************************************************
		 * Int expressions returned by an imm int function block are
		 * associated with a pre-compiled C-function, which cannot be
		 * merged with the expression in which that function might be
		 * called, which is associated with a different C-function, in
		 * which the called function block is one element. For this
		 * reason special provisions have been taken in the compiler to
		 * make the 'type' returned by an imm int function block
		 * different from a normal int (ARNF rather than ARN). This
		 * prevents merging of ARNF expressions with ARN expressions
		 * in op_push(). Otherwise ARNF and ARN expressions are handled
		 * the same and produce the same code. Later in the first loop
		 * of listNet() all ARNF nodes are changed back to ARN nodes
		 * so the execution code did not need to be changed. No ARNF
		 * nodes are visible to the user; both are marked by + in the
		 * listing.
		 *******************************************************************/
		rsp->type = ARNF;		/* change ARN to ARNF */
	    }
	}					/* assign parameters alrady have a link */
	slp = vlp->le_next;			/* next statement */
    }
    /********************************************************************
     * Pass 2: scan again and clone the expression for each statement
     *******************************************************************/
    if (iC_optimise & 02) {
#if YYDEBUG
	tfirst = rll.f;				/* start of replacement arithmetic string */
	tlast = rll.l;				/* rll was set up early in clonFunction */
#endif
	*(rll.f) = STX;				/* STX marks start of call text */
	ttp = rll.l -1;				/* internal expressions are retained */
	assert(*ttp == ')');
	*ttp = ETX;				/* ETX marks end of call text */
	ttp = iCstmtp;				/* pointer into optimised text */
    }
    slp = functionHead->u_blist;		/* statement list */
    while (slp) {
	vlp = slp->le_next;
	sp2 = slp->le_sym;			/* expression head template */
	assert(sp2 && sp2->fm & FM);		/* marked statement list head Symbol */
	typ = sp2->type;			/* type's below are marked with fmxx */
	assert(sp2->u_blist || typ == UDF || typ == ARNC || typ == LOGC || typ == ERR);
	/********************************************************************
	 * clone the expression head associated with this statement
	 *******************************************************************/
	lp = sp2->list;				/* link to real return, assign parameter or */
	assert(lp);				/* internal Symbol linked in pass 1 */
	if ((sv.v = lp->le_sym) == functionHead) {
	    assert(rsp);			/* return Symbol was set in Pass 1 */
	    rll.v = cloneList(sp2, &rsp, rsp, 1); /* clone the rest of the expression */
	} else if (sp2->u_blist) {		/* assign parameter or internal Symbol */
	    assert(sv.v);			/* TODO what happens to link if not used ??? */
	    csp = cloneSymbol(sp2);		/* clone parameter expression head Symbol */
	    sv.v->u_blist = cloneList(sp2, &csp, rsp, 2); /* clone the rest of the expression */
	}
	slp = vlp->le_next;			/* next statement */
    }
    if ((iC_optimise & 02) && ttp > iCstmtp) {
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "cloneFunction: ar optimisation  %s\t'%s'\n",
		rsp ? iC_full_type[rsp->type] : "VOID", iCbuf);
	    fflush(iC_outFP);
	}
#endif
	iCstmtp = rll.l = ttp;			/* end of optimised text */
    }
    /********************************************************************
     * The following logical parameter optimisation on the cloned expression net
     * is carried out in 3 sub-passes when cloning a real expression. This
     * optimises the cloned net of all nested calls in the function definition.
     * Pass 6 cleans up freelist after optimisation.
     *******************************************************************/
    if (iFunSymExt == 0 && (iC_optimise & 01)) {
	/********************************************************************
	 * Pass 2.1: scan templist and store use count in temp symbols v.cnt
	 *           v.cnt can be 0	- never used in any expression
	 *                        1	- used once in an expression - merge
	 *                        2	- used more than once - don't merge
	 *                        > 2	- is a v.elist entry - is also used
	 *                                more than once and cannot be merged
	 *           When compiling with YYDEBUG without SYUNION the above union
	 *           of v.cnt and v.elist must be simulated. Otherwise a gate
	 *           with feedback may be merged, causing corruption.
	 *
	 *           Also count inputs to the target gates - if AND or OR gate
	 *           (XOR will always have at least 2 inputs) has only 1 input
	 *           (usually an action gate) its logical type may be changed
	 *           to any of AND, OR or XOR and then merged.
	 *
	 *           Counting inputs is done in unsigned char 'fm' - number of
	 *           inputs is always less than or equal to 127 (PPGATESIZE)
	 *******************************************************************/
	int		mcnt = 0;		/* count merge candidate found */
	int		ecnt = 0;		/* count excess merge candidate */
#if YYDEBUG
	int		xcnt = 0;
#endif

	for (sp = templist; sp; sp = sp->next) {	/* target gate */
	    assert(sp->fm == 0);			/* fm must be free for temp gates */
	    if (sp->type >= XOR &&			/* XOR, AND or OR */
		sp->type <= OR) {			/* target can be any ftype */
		for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {
		    sp1 = lp1->le_sym;			/* input to this XOR, AND or OR */
		    assert(sp1);
		    if (sp1->ftype == ARITH || sp1->ftype == GATE) {
			sp->fm++;			/* count inputs to target gate */
			if (sp1->type >= XOR  &&	/* XOR, AND or OR */
			    sp1->type <= OR   &&
			    sp1->u_blist != 0 &&
#if YYDEBUG && ! defined(SYUNION)
			    sp1->v_elist == 0 &&	/* simulate union v.cnt v.elist */
#endif
			    sp1->v_cnt < 2) {		/* but not used more than once */
			    if (sp1->v_cnt++ == 0) {	/* count use in merge candidate */
				mcnt++;			/* merge candidate found */
			    } else {
				ecnt++;			/* excess v.cnt for merge candidate */
			    }
			}
		    }
		}
	    }
#if YYDEBUG
	    xcnt++;
#endif
	}
	/********************************************************************
	 * Pass 2.2: merge AND, OR and XOR nodes which have v.cnt == 1 into
	 *           target plain or action gates which satisfy merge criteria.
	 * These are:
	 *           a) If target has only one input - can merge with AND OR or XOR
	 *              simply make type of target the type of single merging gate.
	 *           b) Else check if merge type equals target type after transposing
	 *              AND and OR if merge gate is negated.
	 *              If types are equal and negated - op_not merge gate.
	 *           Transfer gate links from merge gate to target gate.
	 *           Remove merge gate from templist.
	 *           Free link, possible name and merge gate Symbol if not YYDEBUG.
	 *
	 *           Care must be taken when actually merging, that 'fm' is
	 *           incremented. A test against PPGATESIZE is done also.
	 *******************************************************************/
	if (mcnt > ecnt) {	/* there is a true merge candidate whose v.cnt is 1 */
	    for (sp = templist; sp; sp = sp->next) {	/* target gate */
		if (sp->type >= XOR &&			/* XOR, AND or OR */
		    sp->type <= OR) {			/* target can be any ftype */
		    for (lpp = &sp->u_blist; (lp1 = *lpp) != 0; ) {
			sp1 = lp1->le_sym;		/* input to this XOR, AND or OR */
			assert(sp1);
			lval = lp1->le_val;
			if (sp1->type >= XOR  &&	/* XOR, AND or OR */
			    sp1->type <= OR   &&
			    lval != (unsigned)-1 &&	/* not a delay for timer */
			    (lp2 = sp1->u_blist) != 0 &&/* save first link */
			    sp1->v_cnt == 1) {		/* can be merged */
			    assert(sp1->ftype == GATE);
			    if (sp->fm == 0) {
				execerror("input count at temp gate is zero ???\n", NS, __FILE__, __LINE__);
			    }
			    typ = sp1->type;
			    if (lval == NOT && sp1->type != XOR) {
				typ ^= AND^OR;		/* transpose AND OR type */
			    }
			    if (sp->type != typ && sp->fm != 1) {
				goto skipMerge;		/* cannot be merged because types not compatible */
			    }
			    if (lval == NOT) {
				op_not(lp1);		/* negate AND/OR - could be XOR */
			    }
			    /* now merge a gate */
			    sp->type = typ;		/* adjust if single input and transposed */
			    *lpp = lp2;			/* first link of merge gate to target list */
			    while (lp2->le_next) {
				if (sp->fm <= PPGATESIZE) {
				    sp->fm++;		/* add inputs to target gate */
				} else {
				    ierror("too many inputs on gate during optimisation:", sp->name);
				    if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
				}
				lp2 = lp2->le_next;		/* scan to end of merge list */
			    }
			    lp2->le_next = lp1->le_next;	/* link rest of target list to merged tail */
			    if (templist != sp1) {		/* bypass sp1 on templist and delete left Link and Symbol */
				sp2 = templist;			/* scan templist */
				while (sp2->next != sp1) {
				    sp2 = sp2->next;		/* find sp1 in templist */
				    if (sp2 == 0) {
					execerror("merge temp not found ???\n", NS, __FILE__, __LINE__);
				    }
				}
				sp2->next = sp1->next;		/* unlink sp1 from templist */
			    } else {
				templist = sp1->next;		/* unlink first object */
			    }
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				fprintf(iC_outFP, "cloneFunction: merge  %s  %c%s\n",
				    sp->name, v(lp1));
				fflush(iC_outFP);
			    }
#endif
			    lp1->le_next = freelist;	/* must retain link for repeats */
			    freelist = lp1;			/* pop and free in Pass 6 */
			    continue;	/* after merging continue scan with merged links */
			}
		      skipMerge:  
			lpp = &lp1->le_next;		/* is skipped if merged  */
		    }
		}
	    }
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "cloneFunction: Merge:	%s	xcnt %d, mcnt %d, ecnt %d\n",
		    functionHead->name, xcnt, mcnt, ecnt);
		fflush(iC_outFP);
	    }
	} else if (mcnt && (iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "cloneFunction: NoMerge:	%s	xcnt %d, mcnt %d, ecnt %d\n",
		functionHead->name, xcnt, mcnt, ecnt);
	    fflush(iC_outFP);
#endif
	}
	/********************************************************************
	 * Pass 2.3: restore v.cnt values 1 and 2 to zero, so v.elist is correct
	 *******************************************************************/
	for (sp = templist; sp; sp = sp->next) {	/* target gate */
	    sp->fm = 0;					/* restore fm for temp gates */
	    if (mcnt &&		/* there is a merge candidate whose v.cnt must be cleared */
		sp->type >= XOR &&			/* XOR, AND or OR */
		sp->type <= OR) {			/* target can be any ftype */
		for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {
		    sp1 = lp1->le_sym;			/* input to this XOR, AND or OR */
		    assert(sp1);
		    if (sp1->ftype == ARITH || sp1->ftype == GATE) {
			if (sp1->type >= XOR  &&	/* XOR, AND or OR */
			    sp1->type <= OR   &&
			    sp1->u_blist != 0 &&
			    sp1->v_cnt <= 2) {		/* but not used on v.elist */
			    sp1->v_cnt = 0;		/* restore v.cnt */
			}
		    }
		}
	    }
	}
    }						/* end of 3 optimisation sub-passes */
    /********************************************************************
     * Pass 3: assign internally declared variables and assign parameters
     *         'this' is not assigned until cloneFunction returns
     *******************************************************************/
    slp = functionHead->u_blist;		/* statement list */
    while (slp) {
	vlp = slp->le_next;
	sp2 = slp->le_sym;			/* expression head template */
	lp = sp2->list;				/* link to real Symbol */
	assert(lp);				/* internal Symbols linked in pass 1 */
	if ((sv.v = lp->le_sym) != functionHead) {
	    char	y1[2];			/* assign parameter or internal Symbol */
	    int		yn;
	    int		ioTyp;
	    /********************************************************************
	     * assign the expression associated with this assign para or internal statement
	     * an output can only be recognised by its name: Q[XBWL]%d
	     *******************************************************************/
	    sl.v = sv.v->u_blist;		/* link to expression head */
	    sv.f = sv.l = sl.f = sl.l = 0;	/* clear internal expression text */
	    if (sp2->u_blist == 0) {
		assert(sl.v == 0);		 /* template is defined - otherwise error */
		sv.v->type = iC_ctypes[sv.v->ftype]; /* must be ARNC or LOGC */
	    } else
	    if (iFunSymExt ||
		(sp2->fm & FM) == 0 ||
		(sp2->type != SW && sp2->type != CF)) {
		char *	name;
		sv.v->u_blist = 0;		/* restore for op_asgn */
		if (lp->le_val == NOT && sv.v->ftype == GATE) {
		    op_not(sl.v);		/* negated assign parameter */
		}
		if ((name = sv.v->name) && sscanf(name, "Q%1[XBWL]%d", y1, &yn) == 2) {
		    ioTyp = (y1[0] == 'X') ? OUTX : OUTW;
		} else {
		    ioTyp = 0;			/* flags that no I/O is generated */
		}
		assignExpression(&sv, &sl, ioTyp); /* assign to internal variable or parameter */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(0, "clone", (Lis*)&sv);
#endif
	    } else {	/* iFunSymExt == 0 && (sp2->fm & FM) && (sp2->type == SW || sp2->type == CF) */
		int	cFn;
		cFn = sp2->u_blist->le_val >> 8; /* function number */
		assert(cFn && cFn < functionUseSize);
		functionUse[0].c_cnt |= F_CALLED;	/* flag for copying temp file */
		functionUse[cFn].c_cnt++;	/* free this 'if' or 'switch' function for copying */
		sy_pop(lp);			/* free link before target is unlinked */
		sp2->list = 0;			/* free for next cloning */
		uninstall(sv.v);		/* delete temporary Symbol */
		op_asgn(0, &sl, GATE);		/* delete missing slave gate in ffexpr */
	    }
	}
	slp = vlp->le_next;			/* next statement */
    }
    /********************************************************************
     * Cloning is now complete
     * Free links and restore internal pointers in function template.
     * The order is the same as in pushFunCall() to restore correctly.
     * Value parameter links are restored in Pass 4.
     * Assign parameter and Declaration links are restored in Pass 5.
     * (This new scheme makes sure link is not freed twice - JW 9-Apr-2007)
     *******************************************************************/
    cF = 0;
    /********************************************************************
     * Pass 4: Clean up parameter list
     *******************************************************************/
    lp = functionHead->list;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;
	if (sp->u_blist == 0) {
	    lp1 = sp->list;
	    while (lp1) {
		if ((sp->fm & FU) == 0 && (sp1 = lp1->le_sym) != 0) {
		    warning("call parameter is never used:", sp1->name);
		}
		lp2 = lp1->le_next;
		sy_pop(lp1);			/* free link pushed in handleRealParameter */
		lp1 = lp2;			/* required if delay for a timer */
	    }
	    sp->list = saveFunBs->iSav[cF++];	/* restore previous value parameter template link */
	}
	if (sp->v_cnt <= 2) {
	    sp->v_cnt = 0;			/* clear v_cnt ==> v_glist for c_compile */
	}
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
	if (sp->v_cnt <= 2) {
	    sp->v_cnt = 0;			/* clear v_cnt ==> v_glist for c_compile */
	}
	sp1 = ((lp1 = sp->list) != 0) ? lp1->le_sym : 0;
	if (sp1 != functionHead) {		/* bypass return link */
	    while (lp1) {
		lp2 = lp1->le_next;
		sy_pop(lp1);			/* free link pushed in cloneFunction */
		lp1 = lp2;			/* or handleRealParameter if assign parameter */
	    }
	    sp->list = saveFunBs->iSav[cF++];	/* restore declaration and assign link */
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
     * Pass 6: Free list of merged Symbols from Pass 2.2 after name was
     *         used in sy_pop() of 2nd link to Symbol in Pass 4 and 5
     *         If no optimisation was done freelist == 0
     *******************************************************************/
    while (freelist) {
	lp1 = freelist;
	freelist = lp1->le_next;		/* before link is popped */
#if YYDEBUG
	sp1 = lp1->le_sym;
	sy_pop(lp1);				/* free merge List_e */
	if (sp1 && sp1->name) {
	    free(sp1->name);			/* free name space */
	}
	free(sp1);				/* free merge Symbol */
#else
	free(sy_pop(lp1));			/* merge List_ and Symbol */
#endif
    }
    /********************************************************************
     * restore globals from nested function call
     *******************************************************************/
    iFunClock = saveFunBs->iFunClock;
    iFormNext = saveFunBs->iFormNext;
    iCallHead = saveFunBs->iCallHead;
    oldSFunBs = saveFunBs->prevFunBs;
    free(saveFunBs);				/* free memory - no need for size */
    saveFunBs = oldSFunBs;			/* base of previous save block or 0 */
    return rll;					/* return expression or 0 for void function */
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

    rsp = atSymbol();
    rsp->type = sp->type;
    rsp->ftype = sp->ftype;
    rsp->v_cnt = sp->fm & FU;			/* transfer use count */
    rsp->next = templist;			/* put at front of templist */
    templist = rsp;
    return rsp;
} /* cloneSymbol */

/********************************************************************
 *
 *	Clone the links from one Symbol and the rest recursively
 *
 *	ssp	formal head of the expression
 *	cspp	pointer to cloned head of the expression
 *	rsp	cloned head of the function block return
 *	int	call: reports call instance; 0 if recursion
 *	return	rlp	first element of linked chain of expression
 *
 *******************************************************************/

static List_e *
cloneList(Symbol * ssp, Symbol ** cspp, Symbol * rsp, int call)
{
    List_e *		flp;			/* link to formal expression */
    List_e *		rlp = 0;		/* return pointer */
    List_e *		tlp = 0;		/* temp pointer */
    List_e *		clp;			/* cloned list link */
    List_e *		nlp;			/* next cloned list link */
    Symbol *		fsp;			/* current source Symbol */
    Symbol *		nsp;			/* cloned new Symbol */
    Symbol *		csp;			/* cloned head of the expression */
    unsigned int	nval;			/* logical sign of a real parameter */
    char *		first;			/* catch arithmetic text */
    char *		last;
    int			len1;
    static unsigned	recursions = 0;

    assert(recursions < 1000);
    flp = ssp->u_blist;				/* link to formal expression */
    csp = *cspp;				/* cloned head of the expression */
    recursions++;
    while (flp) {
	fsp = flp->le_sym;
	assert(fsp);
	if (fsp->fm & FM) {			/* formal parameter or declared value */
	    if ((nlp = fsp->list) != 0 &&	/* link to real Symbol */
		(nsp = nlp->le_sym) != 0) {	/* real Symbol cloned before */
		nval = nlp->le_val;		/* logical sign of a real GATE parameter */
		first = nlp->le_first;		/* transfer arithmetic text */
		last = nlp->le_last;
		assert(fsp->ftype == GATE || nval == 0);
		if (nsp == iCallHead) {		/* iCallHead used as dummy real return */
		    nsp = rsp;			/* real return Symbol of current statement */
		    assert(nsp && nsp->type < IFUNCT);	/* allows IFUNCT to use union v.cnt */
		    nlp =
#if ! YYDEBUG || defined(SYUNION)
			nsp->v_cnt <= 2 ? 0 :
#endif
			    nsp->v_elist;	/* feedback list */
		    nsp->v_elist = sy_push(csp);
		    nsp->v_elist->le_next = nlp;
		    nlp = 0;
		} else {
		    nlp = nlp->le_next;		/* possible timer parameter link */
		}
	    } else {
		ierror("no link from formal to real parameter:", fsp->name);
		break;				/* cannot continue */
	    }
	} else {
	    first = last = 0;
	    while (fsp->type == ALIAS && (nlp = fsp->list) != 0) {	// was u_blist TODO
		fsp = nlp->le_sym;		/* resolve ALIAS in function */
		assert(fsp);
	    }
	    if (fsp->name) {			/* global variable Symbol */
		nsp = fsp;			/* pointer to global variable */
	    } else {				/* function internal Symbol ->name == 0 */
		nsp = (Symbol*)fsp->list;	/* link set up in cloneFunction varList scan */
		assert(nsp);
		if (nsp->u_blist == 0) {	/* clone internal expression recursively */
		    nsp->u_blist = flp;		/* set in case of tight feedback */
		    cloneList(fsp, &nsp, rsp, 0); /* unless in parallel branch */
		}
	    }
	    nlp = 0;
	    nval = 0;
	}
	clp = sy_push(nsp);			/* clone one list element */
	clp->le_val = flp->le_val ^ nval;	/* negation or arithmetic index in para */
	assert(first == 0 || first >= iCbuf && last < &iCbuf[IMMBUFSIZE]);
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
	    assert(nlp->le_first == 0 || nlp->le_first >= iCbuf && nlp->le_last < &iCbuf[IMMBUFSIZE]);
	    clp->le_first = nlp->le_first;	/* transfer arithmetic text */
	    clp->le_last = nlp->le_last;
	    tlp = tlp->le_next = clp;		/* also link delay element */
	    nlp = nlp->le_next;			/* 0 unless other parameter expression */
	}
	flp = flp->le_next;			/* next formal list element */
    }
    --recursions;
    csp->u_blist = rlp;				/* first link in cloned chain */
    if (rlp && call) {				/* rlp == 0 if break: no link from formal to real */
	assert(recursions == 0);
	nlp = sy_push(csp);			/* link to expression head for Pass 3 */
	if (csp && csp->type == ALIAS) {
	    assert(csp->em == 0 && csp->fm == 0); /* not an external or function type */
	    *cspp = rlp->le_sym;		/* allow use of Symbol in optimisation */
	    assert(*cspp);
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "checkAlias %d: %s %c%c=> %s\n",
		    call, csp->name, nlp->le_val ? '~' : '=', rlp->le_val ? '~' : '=', (*cspp)->name);
		fflush(iC_outFP);
	    }
	    csp = *cspp;
	    freeTemp(nlp);			/* free link and Symbol and name of Smbol if YYDEBUG */
	    nlp = 0;
	} else {
	    rlp = nlp;
	}
    }
    /********************************************************************
     * The following arithmetic optimisation is carried out here, because
     * during cloning the final bracket of the call has just been read
     * and the full call text is at the end of iCbuf and iCstmtp points
     * just past the final bracket
     *
     * Transfer gate links from merge gate to target gate.
     *
     * Replace arithmetic expression text in iCbuf (le_first - le_last-1 of
     * target) by (expr_txt) where expr_txt is: (parenthesis are important!!)
     *   ARNF: copy of functionUse[cFn].c.expr with each <ff><x> replaced
     *         by caList[x].l (build caList[] while scanning ARNF target)
     * Remove merge gate from templist.
     * Free link and merge gate Symbol and temp @n name if YYDEBUG by
     * putting them on freelist. They remain accesible for repeat expressions.
     * Pass 6 cleans up freelist after optimisation.
     *
     * Optimisation for secondary statements is difficult (test -O17)
     * would require the following if condition and the assert:
     * ((csp->type == ARN || csp->type == ARNF) && (iC_optimise & 02) && (call == 1 || (iC_optimise & 010)))
     * assert(csp->type == ARNF || call != 1);
     * All csp->type's for call == 1 are ARNF (see assert above) - re-confirmed JW 20070608
     * Has been abandoned for now - JW 20070627 - Mezieres/Switzerland
     *******************************************************************/
    if (csp->type == ARNF && (iC_optimise & 02) && call == 1) {
	Symbol *	icp;
	Symbol *	sp1;
	Symbol *	sp2;
	List_e *	lp1;
	List_e *	lp2;
	List_e **	lpp;
	char *		ccp;			/* pointer in function template */
	char *		cp;
	char *		ep;
	int		x;
	int		y;
	int		z1;
	int		z2;
	int		index;
	int		offset;
	int		repeat;
	unsigned int	lval;			/* logical sign of a temp */
#if YYDEBUG
	int		len2, len3;		/* auxiliary lengths for debug output */
#endif
	static char	repeatMark[2] = { '\0', '\0' };

#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "cloneList: expression  %s 	%-6s %2d call = %d\n",
		csp->name, iC_full_type[csp->type], csp->v_cnt, call);
	    fflush(iC_outFP);
	}
#endif
	assert(ttp == 0 || ttp >= iCbuf);
	rlp->le_first = ttp;			/* first of this statement expression text */
	*ttp++ = '#';				/* room to parenthesise arithmetic function expression */
	icp = 0;
	index = 0;
	for (lpp = &csp->u_blist; (lp1 = *lpp) != 0; ) {
	    sp1 = lp1->le_sym;		/* input to this ARN */
	    assert(sp1);
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		len1 = tfirst && lp1->le_first ? lp1->le_first - tfirst : 0;
		len2 = lp1->le_first && lp1->le_last ? lp1->le_last - lp1->le_first : 0;
		len3 = lp1->le_last && tlast ? tlast - lp1->le_last : 0;
		fprintf(iC_outFP, "cloneList: target  %s	%-6s %2d '%*.*s [%*.*s] %*.*s'\n",
		    sp1->name, iC_full_type[sp1->type], sp1->v_cnt,
		    len1, len1, tfirst,
		    len2, len2, lp1->le_first,
		    len3, len3, lp1->le_last);
		fflush(iC_outFP);
	    }
#endif
	    /********************************************************************
	     * Copy the parameter expression text from lp1 now.
	     *
	     * Repeated variables in a function work, but the code puts direct
	     * references to the repeated variable in the generated C code, which
	     * are resolved with an extra _f0_1 parameter block.
	     * An alternative would be to mark repeated expression as doubly used
	     * in the function definition, which is actually the case and these
	     * will then not be merged.
	     *******************************************************************/
	    lval = lp1->le_val;			/* save the function indices */
	    if ((sp1->ftype == GATE ||
		(sp1->ftype == ARITH &&		/* exclude clock, timer */
		lval != (unsigned)-1))) {	/* and delay for timer */
		if (lval != 0) {
		    if (index++ == 0) {
			x = lval >> 8;		/* function number */
			assert(x < functionUseSize);
			ccp = strncpy(cBuf, functionUse[x].c.expr, EBSIZE);	/* cloning expression */
		    }
		    assert(index == (lval & 0xff));		/* function offset */
		    caList[index].l = lp1;
		    repeat = 0;
		    repeatMark[0] = '\0';
		    do {
			cp = ccp;			/* current position in cBuf */
			ep = strchr(cp, '\xff');
			if (ep) {
			    y = ep[1];		/* numeric byte value */
			    if (y > index) break;	/* lookahead has gone past index */
			    ccp = ep;
			    ep += 2;
			} else {
			    y = 0;
			    ep = ccp = cp + strlen(cp);	/* no more macros */
			}
			*ccp = '\0';		/* modify \xff to \0 */
			len1 = cp ? strlen(cp) : 0;
			if (len1 > 0) {
			    strncpy(ttp, cp, len1);	/* copy the function text to end of iCbuf */
			}
			ttp += len1;
			assert(y <= index);
			if (sp1 == iconst) {
			    assert(ccp == ep && y == 0);
			    icp = iconst;			/* retain if no other links */
			    *lpp = lp1->le_next;		/* point to next link */
			    sy_pop(lp1);			/* unlink iConst */
			    lp1 = 0;
			    goto putTrailer;
			} else
			if (y == 0) break;		/* tail is finalised */
			ccp = ep;
			lp2 = caList[y].l;
			assert(lp2);
			sp2 = lp2->le_sym;
			assert(sp2);
			if ((sp2->type == ARN || sp2->type == ARNF) &&
			    sp2->u_blist &&
			    sp2->v_cnt == 1) {
			    *ttp++ = '(';		/* parameter expressions must be parenthesised */
			}
			/********************************************************************
			 * Either a ARN(F), INPW or SH.
			 * Some function variables have no le_first - le_last (iConst ...)
			 * Other temp variables have a null name pointer - so have both.
			 * Arithmetic built in (SH is only candidate) is left as a null string
			 * (or temp name @n for logic generation output). The parameters are
			 * scanned from templist and their expansions appended to iCbuf rather
			 * than expanding the parameters of the built in as they are passed from
			 * the iC code in iCbuf.
			 *******************************************************************/
			len1 = 0;				/* in case temp variable has sp->name == 0 */
			if ((sp2->type == ARN || sp2->type == ARNF || sp2->type == INPW || sp2->u_blist == 0) &&
			    (len1 = lp2->le_first && lp2->le_last ? lp2->le_last - lp2->le_first : 0) != 0) {
			    if (sp2->v_cnt <= 1 && sp2->type != INPW) {
				assert(len1 > 0);		/* text of variable from iC source */
				cp = lp2->le_first;
			    } else {
				if (repeat == 0 || (repeatMark[0] == '\0' && sp2->type == INPW)) {
				  setMark:
				    repeatMark[0] = '\x80' + z;
				    vaList[z].f = lp2->le_first;
				    vaList[z].l = lp2->le_last;
				    vaList[z].v = sp2;
#if YYDEBUG
				    z1 = z;
				    z2 = -2;
#endif
				    if (++z >= 0x80) z = 0;
				} else {
				    for (z1 = z > 0 ? z - 1 : 0x7f, z2 = 0; vaList[z1].v != sp2; z2++) {
					if (--z1 < 0) z1 = 0x7f;	/* circular list */
					if (z2 >= 0x80) goto setMark;	/* stop runaway of this algorithm */
				    }
				    repeatMark[0] = '\x80' + z1;
				}
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    fprintf(iC_outFP, "cL: z1=%2d z2=%2d	%-15s %-6s v_cnt = %2d [%*.*s] '%s'\n",
					z1, z2, sp2->name, iC_full_type[sp2->type],
					sp2->v_cnt, len1, len1, lp2->le_first, repeatMark);
				    fflush(iC_outFP);
				}
#endif
				len1 = 1;
				cp = repeatMark;		/* repeatMark in text allows recovery of z */
			    }
			} else if (sp2->name) {
			    len1 = strlen(sp2->name);
			    assert(len1);
			    cp = sp2->name;			/* name of variable from generated node */
			}
			if (len1 && strncmp(cp, "iConst", len1) == 0) {
			    len1 = 0;				/* leave out "iConst" which is only a trigger */
			}
			if (len1) {
			    strncpy(ttp, cp, len1);		/* copy the expression to end of iCbuf */
			}
			if (y == index && repeat == 0) {
			    offset = ttp - lp2->le_first;
			    lp2->le_first += offset;	/* adjust offset */
			    lp2->le_last = lp2->le_first + len1;
			    repeat++;
			}
			ttp += len1;
			if ((sp2->type == ARN || sp2->type == ARNF) &&
			    sp2->u_blist &&
			    sp2->v_cnt == 1) {
			    *ttp++ = ')';		/* parameter expressions must be parenthesised */
			}
#if YYDEBUG
			if ((iC_debug & 0402) == 0402) {
			    fprintf(iC_outFP, "cloneList:  x = %d y = %d %s '%s'\n",
				x, y, sp2->name, iCstmtp);
			    fflush(iC_outFP);
			}
#endif
		    } while (y <= index || lp1->le_next == 0);
		    lp1->le_val = 0;		/* action on lp1 TODO */
		} else if (lp1->le_first) {
		    assert(lp1->le_last);
		    lp1->le_first += offset;	/* adjust offset in components of expression */
		    lp1->le_last  += offset;
		}
	    }
	    /********************************************************************
	     * end of copy parameter expression text from lp1
	     *******************************************************************/
	    if ((sp1->ftype == GATE ||
		(sp1->ftype == ARITH &&		/* exclude clock, timer */
		lval != (unsigned)-1)) &&	/* and delay for timer */
		(sp1->type == ARN || sp1->type == ARNF) &&
		(lp2 = sp1->u_blist) != 0 &&	/* save first link */
#if YYDEBUG && ! defined(SYUNION)
		sp1->v_elist == 0 &&		/* simulate union v.cnt v.elist */
#endif
		sp1->v_cnt == 1) {		/* can be merged */
		assert(sp1->ftype == ARITH);
		/* now merge a gate */
		*lpp = lp2;			/* first link of merge gate to target list */
		while (lp2->le_next) {
		    lp2 = lp2->le_next;		/* scan to end of merge list */
		}
		lp2->le_next = lp1->le_next;	/* link rest of target list to merged tail */
		if (templist != sp1) {		/* bypass sp1 on templist and delete left Link and Symbol */
		    sp2 = templist;			/* scan templist */
		    while (sp2->next != sp1) {
			sp2 = sp2->next;		/* find sp1 in templist */
			if (sp2 == 0) {
			    execerror("merge temp not found ???\n", NS, __FILE__, __LINE__);
			}
		    }
		    sp2->next = sp1->next;		/* unlink sp1 from templist */
		} else {
		    templist = sp1->next;		/* unlink first object */
		}
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    lp1->le_val = 0;		/* clear the function indices */
		    fprintf(iC_outFP, "cloneList: merge arithmetic  %s  %c%s\n",
			csp->name, v(lp1));
		    fflush(iC_outFP);
		}
#endif
		lp1->le_next = freelist;	/* must retain link for repeats */
		freelist = lp1;			/* pop and free in Pass 6 */
		continue;		/* after merging continue scan with merged links */
	    }
	    lp1->le_val = 0;			/* clear the function indices */
	    lpp = &lp1->le_next;		/* is skipped if merged  */
	}
      putTrailer:
	*ttp++ = '#';				/* room to parenthesise arithmetic function expression */
	if (icp && csp->u_blist == 0) {		/* only constants - no other links */
	    csp->u_blist = sy_push(icp);	/* link pointer to iConst */
	}
	rlp->le_last = ttp;			/* last of this statement expression text */
    }					/* end of arithmetic optimisation */
    return rlp;
} /* cloneList */

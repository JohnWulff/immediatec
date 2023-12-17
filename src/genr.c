static const char genr_c[] =
"@(#)$Id: genr.c 1.102 $";
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
 *	genr.c
 *	generator functions for immcc compiler
 *
 *******************************************************************/

#include	<stdio.h>
#include	<stddef.h>
#include	<stdlib.h>
#include	<string.h>
#include	<assert.h>
#include	<ctype.h>
#include	<limits.h>
#include	"comp.h"
#include	"comp.tab.h"

#define VSIZE	11
#define v(lp)	(lp->le_val == 0) ? " " : (lp->le_val == NOT) ? "~" : (snprintf(vBuf, VSIZE, "<%x>", lp->le_val), vBuf),\
lp->le_sym ? lp->le_sym->name : "(0)"
#define w(lp)	(lp->le_val) ? '~' : ' '
#define EBSIZE	16384
#define STX	'\x02'
#define ETX	'\x03'
#define CASIZE	64
#define FLPSIZE	64
#define INISIZE	64

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
static char	vBuf[VSIZE];		/* temporary hex text buffer */
static char *	ePtr = 0;		/* temporary C expression buffer input pointer */
static char *	eEnd = 0;		/* end of temporary C expression buffer */
static char *	cPtr = 0;		/* temporary clone expression out buffer input pointer */
static char *	cEnd = 0;		/* end of temporary clone expression out buffer */
static char *	gPtr = 0;		/* temporary clone expression in buffer input pointer */
static char *	tPtr = 0;		/* temporary text expression buffer input pointer */
static char *	tOut = 0;		/* temporary text expression buffer output pointer */
static char *	t_first = 0;
static char *	t_last = 0;
static Symbol *	iCallHead = 0;		/* function head seen at start of imm call */
static Symbol *	varList = 0;		/* list of temp Symbols while compiling function */
static List_e *	stmtList = 0;		/* list of statement Symbols making function body */
static int	ffexprFlag = 0;		/* if - else or switch seen in function */
#if YYDEBUG
static char *	tfirst;			/* start of replacement arithmetic string */
static char *	tlast;			/* rll was set up early in cloneFunction */
#endif
static char *	ttp;			/* pointer into extended arithmetic text */
#ifndef	NOCONSTLIST
static char *	constExprString;
#endif	/* NOCONSTLIST */

typedef struct SymList {		/* allow alternate storage of Symbol or List_e pointers */
    List_e *	l;			/* store link List_e * */
    Symbol *	s;			/* as well as Symbol * */
    int		x;			/* real index */
} SymList;
static SymList*	caList = NULL;		/* dynamic auxiliary input gate list for copyArithmetic */
static int	caSize = 0;		/* allocated size of caList */
static List_e**	flpList = NULL;		/* dynamic auxiliary list for if else switch variables */
static int	flpSize = 0;		/* allocated size of flpList */
static List_e**	flpp = NULL;
immCini*	iC_iniList = NULL;	/* dynamic auxiliary list for saving immC initialisers */
static int	iniSize = 0;		/* allocated size of iC_iniList */
immCini*	iC_inip = NULL;

static Sym	vaList[0x80];		/* stores text and Symbol pointers for repeat elements */
int		z = 0;			/* index into vaList[] - circular list of 128 elements */
static int	liveDisp;		/* set when arithmetic values will generate live display */
static void	assignOutput(		/* generate and assign an output node */
			     Symbol * rsp, int ftyp, int ioTyp);
static int	evalConstParameter(List_e * lp);

#if YYDEBUG
typedef struct MsL {
    char *	msgp;
    struct MsL*	next;
} MsL;
static MsL *	debugLines = 0;
static MsL *	debugTail = 0;
static MsL *	dlp;
static void	pushMessage(char *);
#endif

/********************************************************************
 *
 *	initialize for code generation
 *
 *******************************************************************/

void
initcode(void)				/* initialize for code generation */
{
    freelist = 0;
    templist = 0;
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

    sp = lp ? lp->le_sym: NULL;		/* point to variables Symbol entry */
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
    sp->fm |= FT;			/* marks temporary Symbol */
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
 *	Force linked Symbol to correct ftype
 *	Constant expression is forced to a logical expression by linking
 *	to LO for 0 or ~LO for 1 (!= 0) (ignore possible constant sp)
 *
 *******************************************************************/

List_e *
op_force(				/* force linked Symbol to correct ftype */
    Lis *		lv,
    unsigned char	ftyp)
{
    Symbol *		sp;
    List_e *		lp = lv->v;
    List_e *		lp1;
    Valp		v;
    int			typ;
    unsigned char	ft;

    if (lp && (sp = lp->le_sym) != 0 && (ftyp != GATE || sp->type != NCONST)) {
	if (sp->ftype != ftyp) {
	    if (sp->u_blist == 0   ||	/* not a @ symbol or */
		sp->type >= MAX_GT ||	/* SH, FF, EF, VF, SW, CF or */
		(sp->type == LATCH && sp->u_blist->le_sym == sp)	/* LATCH(set,res) */
	    ) {
		if ((typ = iC_types[ft = sp->ftype]) == ERR ||
		    ft == OUTW ||
		    ft == OUTX) {
		    ierror("cannot force from", iC_full_ftype[ft]);	/* OUTW OUTX CLCKL or TIMRL */
		    typ = ERR;
		}
		lp1 = op_push(0, typ, lp);
		assert(lp1);
		assert(lp->le_first == 0 || (lp->le_first >= iCbuf && lp->le_last < &iCbuf[IMMBUFSIZE]));
		lp1->le_first = lp->le_first;
		lp1->le_last = lp->le_last;
		lp = lp1;		/* create a new @ symbol linked to old */
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
    } else if (ftyp == GATE) {
	/* lp == NULL || (ftyp == GATE && sp->type == NCONST) */
	if ((sp = lookup("LO")) == 0) {	/* constant expression used in logic */
	    sp = install("LO", OR, GATE);
	}
	lp = sy_push(sp);		/* LO is permanent bit 0 */
	v = evalConstExpr(lv);
	if (v.nuv) {
	    lp->le_val = NOT;		/* ~LO is permanent bit 1 */
	}
    }
    return lp;				/* return 0 or link to old or new Symbol */
} /* op_force */

/********************************************************************
 *
 *	Unlink a temporary object from templist - free the list
 *	element and the Symbol and the name of the Symbol if YYDEBUG
 *	unless templist is empty, in which case lp does not point
 *	to a temporary @ Symbol.
 *
 *******************************************************************/

static void
freeTempSym(Symbol * sp)			/* free Symbol without popping link */
{
    Symbol * tsp;

    if (sp) {
	assert(templist);
	if (templist == sp) {
	    templist = sp->next;		/* unlink first object */
	} else {
	    tsp = templist;			/* bypass sp on templist */
	    while (tsp->next != sp) {		/* scan templist */
		tsp = tsp->next;		/* find sp in templist */
		if (tsp == 0) {
		    execerror("freeTemp: not found in templist", sp->name, __FILE__, __LINE__);
		}
	    }
	    tsp->next = sp->next;		/* unlink sp from templist */
	}
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    free(sp->name);			/* free name space */
	}
#endif
	free(sp);				/* free Symbol */
    }
} /* freeTempSym */

void
freeTemp(List_e * lp)
{
    if (lp && templist) {
	freeTempSym(sy_pop(lp));		/* free link, Symbol and its name */
    }
} /* freeTemp */

/********************************************************************
 *
 *	op_push() is the central function to generate iC linked network
 *	expressions from one or two expressions joined by an operator.
 *
 *	Parameter 1:	left	a list element pointing to either:
 *				a new Symbol in which case u_blist == 0
 *				or an established expression (a @ Symbol
 *				 which is on 'templist') in which case 'u_blist'
 *				 points to a linked list of elements pointing
 *				 to Symbols making up the members of the
 *				 expression and 'type' determines the
 *				 operational function of the expression.
 *				or 0 if 'left' is a const expression or 'op'
 *				is a unary operator.
 *	Parameter 2:	op	the type of expression to link 'left' and
 *				'right' together.
 *	Parameter 3:	right	another list element pointer like 'left'.
 *
 *	If 'right' is 0 - ie a const expression, 'right' and 'left are swapped'.
 *
 *	If 'right' is not a @ Symbol or 'op' does not equal right->type
 *	A new @ Symbol is created and linked to the front of 'templist'
 *	whereas the right link (pointing to the previous right Symbol)
 *	is linked to u_blist of the new @ Symbol.
 *
 *	Similarly if 'left' is not a @ Symbol 'left' is linked into 'right'
 *	else 'left' is a @ Symbol and 'left' is merged into 'right'.
 *
 *	More complex actions are required for arithmetic optimisation and
 *	dealing with feedback lists.
 *
 *******************************************************************/

List_e *
op_push(					/* reduce List_e stack to links */
    List_e *		left,
    unsigned char	op,
    List_e *		right)
{
    List_e *		rlp;
    Symbol *		rsp;			/* current temporary Symbol */
    Symbol *		lsp;
    List_e *		lp;
    List_e *		tlp;
    int			typ;
    int			ftyp;
    char *		tf;
    char *		tl;
    int			c;
    int			stxFlag;

    if (right == 0) {				/* is 'right' a const expression ? */
	if ((right = left) == 0) return 0;	/* yes - is 'left' const or unary ? nothing to push */
	left = 0;				/*       no - swapped 'left' to 'right' */
    }
    rsp = right->le_sym;
    lsp = left ? left->le_sym : 0;
    rlp = right;
    if (iC_optimise & 02) {			/* only required for arithmetic optimisation */
	if (rsp) {
	    if (rsp->type == ARNF && op == ARN) {
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
		rsp->type = ARN;		/* merge ARNF with ARN when optimising */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    if (right->le_last && right->le_first) {
			int len1 = right->le_last - right->le_first;
			fprintf(iC_outFP, "op_push:  right %s type change from ARNF to ARN '%*.*s'\n",
			    rsp->name, len1, len1, right->le_first);
		    } else {
			fprintf(iC_outFP, "op_push:  right %s type change from ARNF to ARN ''\n", rsp->name);
		    }
		    fflush(iC_outFP);
		}
#endif
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
		    if (left->le_last && left->le_first) {
			int len1 = left->le_last - left->le_first;
			fprintf(iC_outFP, "op_push:  left %s type change from ARNF to ARN '%*.*s'\n",
			    lsp->name, len1, len1, left->le_first);
		    } else {
			fprintf(iC_outFP, "op_push:  left %s type change from ARNF to ARN ''\n", lsp->name);
		    }
		    fflush(iC_outFP);
		}
#endif
	    }
	}
	if (rlp != right) {
	    left = 0;				/* 'right' is only used in 'left' section */
	    right = rlp;			/* and final debug message */
	}
    }
    assert(rsp == 0 || rsp->type < IFUNCT);	/* allows IFUNCT to use union v.cnt */
    typ = rsp ? rsp->type : ARN;
    ftyp = rsp ? rsp->ftype : ARITH;
    if (left && op > OR && op < MAX_LV && op != typ) {
	warning("function iC_types incompatible", NS);
    }
    if (rsp == 0 || rsp->u_blist == 0 || op != typ) {
	if (rsp &&
#if ! YYDEBUG || defined(SYUNION)
	    (lp = 0, rsp->v_cnt) > 2 &&
#endif
	    (lp = rsp->v_elist) != 0 && (rsp->fm & FT) == 0) {	/* not marked symbol */
	    rsp->v_elist = 0;			/* clear the feedback list (sizeof elist > sizeof cnt for 64 bit) */
#if ! YYDEBUG || defined(SYUNION)
	    rsp->v_cnt = 2;			/* but retain high use count when elist was high */
#endif
	    while (lp) {			/* list with feedback to this Symbol */
		tlp = lp->le_next;		/* next link to a feedback Symbol */
		sy_pop(lp);			/* delete a feedback list link */
		lp = tlp;
	    }
	}
	rsp = atSymbol();			/* 'right' not a @ symbol or new operator - force new level */
	rsp->type = op != UDF ? op : AND;	/* operator 'op' or AND (default) */
	rsp->ftype = ftyp;			/* used in op_xor() with op UDF (defunct) */
	rsp->next = templist;			/* put at front of templist */
	templist = rsp;
	rlp->le_next = 0;			/* rsp->u_blist is 0 for new rsp */
	rsp->u_blist = rlp;			/* link 'right' of expression */
	rlp = sy_push(rsp);			/* push new list element on stack */
    }
    if (left && lsp) {				/* test works correctly with ftype - handles ALIAS */
	assert(lsp->type < IFUNCT);		/* allows IFUNCT to use union v.cnt */
	if (lsp->ftype >= MIN_ACT && lsp->ftype < MAX_ACT) {
#ifndef	BOOT_COMPILE
	    assert(lsp->ftype == F_SW || lsp->ftype == F_CF || lsp->ftype == F_CE);
#endif	/* BOOT_COMPILE */
	    if (rsp->ftype < S_FF) {		/* 'left' is a master action gate */
		rsp->ftype = 0;			/* OK for any value of GATE */
	    }
	    rsp->ftype |= lsp->ftype;		/* modify S_FF ==> D_FF or restore lsp->ftype */
	    /* this requires S_FF R_FF D_FF to be 1001 1010 1011 to work !!! */
	    /* does nothing for S_SH R_SH D_SH - not required because no analog SR */
	    assert(rlp && rlp->le_val <= NOT);
	    if (rlp->le_val == NOT) {
		rlp->le_val = NOT^NOT;
		op_not(rlp);			/* double negate so input to action gate not inverted */
	    }
	}
	if ((typ = lsp->type) < MAX_LS) {
	    if (
#if ! YYDEBUG || defined(SYUNION)
		(lp = 0, lsp->v_cnt) > 2 &&
#endif
		(lp = lsp->v_elist) != 0 && (lsp->fm & FT) == 0) {	/* not marked symbol */
		lsp->v_elist = 0;		/* clear the feedback list (sizeof elist > sizeof cnt for 64 bit) */
#if ! YYDEBUG || defined(SYUNION)
		lsp->v_cnt = 2;			/* but retain high use count when elist was high */
#endif
		while (lp) {			/* list with feedback to this Symbol */
		    tlp = lp->le_next;		/* next link to a feedback Symbol */
		    sy_pop(lp);			/* delete a feedback list link */
		    lp = tlp;
		}
	    }
	    if ((lp = lsp->u_blist) == 0 ||	/* 'left' not a @ symbol */
		rsp == lsp ||			/* or 'right' == 'left' */
		(typ != op && typ != TIM) ||	/* or new operator but 'left' is not a timer */
			/* TD watch this when typ is ALIAS or UDF */
		(typ == op &&			/* or operator is same as type of 'left' */
		right->le_val == (unsigned)-1))	/* and 'right' is a delay for a timer */
	    {
		left->le_next = rsp->u_blist;	/* extend expression */
		rsp->u_blist = left;		/* link 'left' of expr */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "op_push:  %s%s %c %s%s\n",
			v(left), iC_os[op], v(right));
		    fflush(iC_outFP);
		}
#endif
	    } else {	/* 'left' is a @ symbol and ... - merge 'left' into 'right' */
		while (lp->le_next) {
		    lp = lp->le_next;		/* scan to end of 'left' list */
		}
		lp->le_next = rsp->u_blist;	/* move connect list */
		rsp->u_blist = lsp->u_blist;	/* in the right order */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) {
		    fprintf(iC_outFP, "op_push:  %s%s %c %s%s\n",
			v(left), iC_os[op], v(right));
		    fflush(iC_outFP);
		}
#endif
		freeTemp(left);			/* bypass lsp on templist and delete 'left' and lsp */
		left = 0;
	    }
	} else {				/* discard 'left' BLT, CLT */
	    sy_pop(left);			/* Link_e only */
	}
#if YYDEBUG
    } else if ((iC_debug & 0402) == 0402) {	/* fexpr : sexpr { 'left' is 0 } */
	fprintf(iC_outFP, "op_push:  (0) %c %s%s\n", iC_os[op], v(right));
	fflush(iC_outFP);
#endif
    }
    return rlp;
} /* op_push */

/********************************************************************
 *
 *	Numeric constant expresssiont push
 *
 *	return	link to a new or old NCONST Symbol with numeric name
 *		if non numeric and r == 0	link to an NCONST Symbol "0"
 *		if non numeric and r != 0	NULL
 *
 *******************************************************************/

static List_e *
constExpr_push(char * exprBuf, int r)
{
    Symbol *	sp;
#ifndef	NOCONSTLIST
    char *	cp;
    char *	dp;
    int		stxFlag;
    int		c;
#endif	/* NOCONSTLIST */
    int		value;
    char	buf[TSIZE];

    if (parseConstantExpression(exprBuf, &value, r)) {	/* convert constant expression */
	if (r) return NULL;				/* not numeric for r != 0 */
	value = 0;					/* return a link to "0" Symbol */
	ierror("not a constant expression:", exprBuf);
    }
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "constExpr_push: '%s' converted to %d\n", exprBuf, value);
	fflush(iC_outFP);
    }
#endif
    snprintf(buf, TSIZE, "%d", value);
    if ((sp = lookup(buf)) == 0) {		/* install name of decimal conversion */
	sp = install(buf, NCONST, ARITH);
	sp->u_val = (unsigned int)value;
    } else if (sp->type != NCONST || sp->ftype != ARITH || sp->u_val != (unsigned int)value) {
	ierror("use of a constant which was previously used for a different purpose:", buf);
    }
#ifndef	NOCONSTLIST
    constExprString = strdup(exprBuf);
    stxFlag = 0;
    for (cp = dp = constExprString; (c = *cp) != 0; cp++) {
	if (isspace(c) || c == '#') continue;	/* ignore white space and # */
	if (c == STX) {
	    stxFlag++;
	    continue;
	}
	if (c == ETX) {
	    stxFlag--;
	    continue;
	}
	if (stxFlag > 0) {
	    continue;				/* ignore text bracketed by STX ETX */
	}
	assert(stxFlag == 0);			/* never should go -ve */
	*dp++ = c;				/* copy expression text - always fits */
    }
    *dp = '\0';					/* terminate early if changed */
#endif	/* NOCONSTLIST */
    return sy_push(sp);
} /* constExpr_push */

/********************************************************************
 *
 *	Numeric constant push
 *
 *	Previous version used to only generate an NCONST node for a
 *	a string which consisted only of numerical digits.
 *
 *	New version (jw 20161112) converts all constant expression
 *	strings to an int and generates an NCONST node for each.
 *
 *******************************************************************/

int
const_push(Lis * expr)
{
    int		len2;
    char	buf[TSIZE];

    len2 = expr->l - expr->f;
    assert(expr->f >= iCbuf && expr->f <= expr->l && expr->l < &iCbuf[IMMBUFSIZE] && len2 < TSIZE);
    strncpy(buf, expr->f, len2);
    buf[len2] = '\0';			/* null terminated expression text in buf[] */
    expr->v = constExpr_push(buf, 0);
    expr->v->le_first = expr->f;	/* identifies as const expression */
    expr->v->le_last  = expr->l;	/* no need to be 0 - JW 12.Apr.2007 */
    return 0;				/* correct - no error */
} /* const_push */

/********************************************************************
 *
 *	Check for a constant or a constant expression
 *
 *******************************************************************/

List_e *
checkConstExpr(List_e * lp)
{
    Symbol *	sp;
    List_e *	lp1;
    Symbol *	sp1;

    if (lp == NULL ||			/* a constant */
	    ((sp = lp->le_sym) != NULL &&
	    sp->ftype == ARITH &&
	    (sp->type == ARN || sp->type == ARNF) &&
	    (lp1 = sp->u_blist) != NULL &&
	    (sp1 = lp1->le_sym) != NULL &&
	    sp1->type == NCONST)) {	/* or a constant expression */
	return NULL;
    }
    return lp;
} /* checkConstExpr */

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
 *	Copy the indexed cMacro iC_MV(x) (MV) of the expression or constant
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
#if YYDEBUG
    char	debugTemp[80];
#endif

    if (((typ = sp->type) == ARN || typ == ARNF) &&
	gp->ftype == ARITH &&
	lp->le_val != (unsigned)-1) {
	assert(caList && ePtr && gPtr && tPtr && tOut);
	if (x == 0xff) {
	    for (y = 1; y < caSize; y++) {
		if (gp == caList[y].s) {
		    x = caList[y].x;			/* reference to (unsigned)-1 delay */
		}
	    }
	}
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
			    assert(cp);
			    len = strlen(cp);
			    assert(len);
			    if (iFunSymExt) {
				sp1->fm |= 0x02;	/* double usage in function arithmetic expression */
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    sprintf(debugTemp, "copyArithmetic:  use count %d for %s\n", sp1->fm & FU, sp1->name);
				    pushMessage(debugTemp);
				}
#endif
			    }
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				if (vaList[z1].l && vaList[z1].f) {
				    int len1 = vaList[z1].l - vaList[z1].f;
				    sprintf(debugTemp, "cA: z1=%2d	%-15s %-6s v_cnt = %2d [%*.*s] '%1.1s' {%*.*s}\n",
					z1, sp1->name, iC_full_type[sp1->type], sp1->v_cnt, len, len, cp, t_first-1,
					len1, len1, vaList[z1].f);
				} else {
				    sprintf(debugTemp, "cA: z1=%2d	%-15s %-6s v_cnt = %2d [%*.*s] '%1.1s' {}\n",
					z1, sp1->name, iC_full_type[sp1->type], sp1->v_cnt, len, len, cp, t_first-1);
				}
				pushMessage(debugTemp);
			    }
#endif
			    tPtr = strncpy(tPtr, cp, len+1) + len;
			    if (sp1->type == NCONST) {
				ePtr = strncpy(ePtr, cp, len+1) + len;
				gPtr = strncpy(gPtr, cp, len+1) + len;
			    } else {
				for (y = 1; y < caSize; y++) {
				    if (sp1 == caList[y].s) break;
				}
				assert(y < caSize);	/* repeated term */
				if (iC_debug & 04) {
				    fprintf(iC_outFP, "\t\t\t\t\t%s\t// %d", tOut, y);
				    if (liveDisp && gp->ftype == ARITH) {
					fprintf(iC_outFP, "\t%s\t=", gp->name);
				    }
				    fprintf(iC_outFP, "\n");
				}
				tOut = tPtr;
				ePtr += sprintf(ePtr, MV "(%d)", y);	/* iC_MV */
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
		    }					/* expression till now */
		    fprintf(iC_outFP, "%s\t// %d", tOut, x);
		    if (liveDisp && gp->ftype == ARITH) {
			fprintf(iC_outFP, "\t%s\t=", gp->name);
		    }
		    fprintf(iC_outFP, "\n");
		}
		tOut = tPtr;				/* alternative tOut = tPtr = tBuf */
		assert(ePtr + 10 < eEnd);		/* checks previous copying */
		ePtr += sprintf(ePtr, MV "(%d)", x);	/* iC_MV */
		gPtr += sprintf(gPtr, "\xff%c", x);	/* bound to fit since shorter */
	    }
	    t_first = lp->le_last;			/* skip current expression */
#ifndef EFENCE
	    assert(t_first == 0 || (t_first >= iCbuf && t_first < &iCbuf[IMMBUFSIZE]));
#else
	    if (t_first != 0 && (t_first < iCbuf || t_first >= &iCbuf[IMMBUFSIZE])) {
		fprintf(iC_outFP, "\nt_first = %p >= iCbuf = %p\n", t_first, iCbuf);
		fflush(iC_outFP);
		ierror("Assertion 2:", NULL);
	    }
#endif
	    if (t_last < t_first) {
		t_last = t_first;			/* skip part of arithmetic expression */
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
	    if (y != x) {
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
		    if (liveDisp && gp->ftype == ARITH) {
			fprintf(iC_outFP, "\t%s\t=", gp->name);
		    }
		    fprintf(iC_outFP, "\n");
		}
		tOut = tPtr;			/* alternative tOut = tPtr = tBuf */
		assert(ePtr + 10 < eEnd);		/* checks previous copying */
		ePtr += sprintf(ePtr, MV "(%d)", x1);	/* iC_MV */
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
			if (liveDisp && gp->ftype == ARITH) {
			    fprintf(iC_outFP, "\t%s\t=", gp->name);
			}
			fprintf(iC_outFP, "\n");
		    }
		    tOut = tPtr;
		    ePtr += sprintf(ePtr, "%s" MV "(%d)", ep, x1);	/* iC_MV */
		    gPtr += sprintf(gPtr, "%s\xff%c", ep, x1);	/* bound to fit since shorter */
		}
		cPtr = ep = cp + 2;
	    }
	}
    } else
    if (iC_debug & 04 && sflag != 02) {
	if (liveDisp && (sflag & 0200) && (
	    sp->ftype == ARITH  ||
	    sp->ftype == D_SH   ||
	    sp->ftype == CH_AR  ||		/* only arithmetic CHANGE */
	    sp->ftype == F_SW   ||
	    sp->ftype == OUTW   ||
	    sp->ftype == TIMRL)) {		/* timers display count state */
	    fprintf(iC_outFP, "\t\t//\t%s\t=", sp->name);
	}
	fprintf(iC_outFP, "\n");		/* all non Arithmetic and delay */
    }
#if YYDEBUG
    while ((dlp = debugLines) != 0) {
	char *	msg;
	msg = dlp->msgp;
	debugLines = dlp->next;
	fprintf(iC_outFP, "%s", msg);		/* FIX CLANG warning JW 20150120 */
	free(msg);
	free(dlp);
    }
    debugTail = 0;				/* debugLines is also 0 */
    fflush(iC_outFP);
#endif
} /* copyArithmetic */

#if YYDEBUG
/********************************************************************
 *
 *	push a message for output at end of copyArithmetic
 *
 *******************************************************************/

static void
pushMessage(char * message)
{
    dlp = (MsL *) iC_emalloc(sizeof(MsL));
    dlp->msgp = iC_emalloc(strlen(message)+1);	/* +1 for '\0' */
    strcpy(dlp->msgp, message);
    dlp->next = 0;
    if (debugTail) {
	debugTail->next = dlp;			/* link this message to previous message */
    } else {
	debugLines = dlp;			/* ready for outputing first message */
    }
    debugTail = dlp;				/* ready for another message */
} /* pushMessage */
#endif

/********************************************************************
 *
 *	write the cexeString and record in function use database
 *
 *******************************************************************/

const char *	cexeString[] = {
    "/**/case %d:\n",				/* begin with empty comment for uniqueness */
#if INT_MAX == 32767 && defined (LONG16)
    "static long iC_%d(iC_Gt * iC_gf) {\n",
#else
    "static int iC_%d(iC_Gt * iC_gf) {\n",
#endif
};

void
writeCexeString(FILE * oFP, int cn)
{
    if (oFP) fprintf(oFP, cexeString[outFlag], cn);
    while (cn >= functionUseSize) {
	functionUse = (FuUse*)realloc(functionUse,
	    (functionUseSize + FUNUSESIZE) * sizeof(FuUse));
	assert(functionUse);
	memset(&functionUse[functionUseSize], '\0', FUNUSESIZE * sizeof(FuUse));
	functionUseSize += FUNUSESIZE;
    }
} /* writeCexeString */

/********************************************************************
 *
 *	write the cexeTail
 *
 *******************************************************************/

static const char *	cexeTail[] = {
    "%s\n",
    "%s} /* iC_%d */\n\n",
};

void
writeCexeTail(FILE * oFP, const char * tail, int cn)
{
    fprintf(oFP, cexeTail[outFlag], tail, cn);	/* FIX CLANG warning JW 20150120 */
} /* writeCexeTail */

/********************************************************************
 *
 *	assign List_e stack to links
 *
 *	Sym sv contains Symbol *v and char *f and *l to source
 *	Lis lr contains List_e *v and char *f and *l to source
 *	unsigned char ft is the ftype which right must be forced to
 *
 *******************************************************************/

Symbol *
op_asgn(				/* assign List_e stack to links */
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
    List_e **	lpp;
    int		atn;
    int		sflag;
    int		fflag;
    int		c;
    int		stxFlag;
    int		len;
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
    right = op_force(rl, ft);		/* force Symbol on right to ftype ft */
    if (iFunSymExt && strncmp(var->name, iFunBuffer, iFunSymExt - iFunBuffer) != 0) {
	ierror("assignment to a variable which does not belong to this function:", var->name);
	var->type = ERR;		/* reduce anyway to clear list */
    }
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "op_asgn:  %s = %s%s\n", var->name, v(right));
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
      makeAlias:
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
	assert(rsp == right->le_sym);
	rsp->em |= var->em & EX;	/* extern EX from ALIAS to target - USED if EX == 1 */
	if (iC_debug & 04) {
	    iFlag = 1;			/* may need correction by pplstfix */
	    fprintf(iC_outFP, "\n\t%s\t%c ---%c\t%s", rsp->name,
		rsp->ftype != GATE ? iC_fos[rsp->ftype] : w(right),
		iC_os[var->type], var->name);	/* type is ALIAS or ERR */
	    if (var->ftype != GATE) {
		fprintf(iC_outFP, "\t%c", iC_fos[var->ftype]);
#ifndef	NOCONSTLIST
		if (constExprString && rsp->type == NCONST) {
		    fprintf(iC_outFP, "\t%s", constExprString);
		    free(constExprString);
		    constExprString = NULL;
		}
#endif	/* NOCONSTLIST */
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
	    memset(t_first, '#', t_last - t_first);	/* mark left var, leave ALIAS */
	}
	return sv->v;			/* needs no reduction */
    }	/* end of ALIAS */

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
	(lp = rsp->v_elist) != 0 && (rsp->fm & FT) == 0) {	/* not marked symbol */
	rsp->v_elist = 0;		/* clear the feedback list (sizeof elist > sizeof cnt for 64 bit) */
#if ! YYDEBUG || defined(SYUNION)
	rsp->v_cnt = 2;			/* but retain high use count when elist was high */
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
    if ((iC_debug & 0402) == 0402) {
	if (t_first && t_last) {
	    int len1 = t_last - t_first;
	    fprintf(iC_outFP, "resolve \"%*.*s\"\n", len1, len1, t_first);
	}
	fflush(iC_outFP);
    }
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
	int		gt_flag;
	unsigned int	val;
	unsigned int	val1;
	int		cFn = 0;
	int		z1;
	int		y;

	ePtr = eBuf;			/* temporary expression buffer pointer */
	eEnd = &eBuf[EBSIZE];		/* end of temporary expression buffer */
	cPtr = 0;			/* temporary clone expression out buffer pointer (set later) */
	cEnd = &cBuf[EBSIZE];		/* end of temporary expression buffer */
	gPtr = gBuf;			/* temporary clone expression in buffer pointer */
	tOut = tPtr = tBuf;		/* temporary text expression buffer pointers */
	memset(eBuf, '\0', EBSIZE);
	memset(cBuf, '\0', EBSIZE);
	memset(gBuf, '\0', EBSIZE);
	memset(tBuf, '\0', EBSIZE);
	/********************************************************************
	 * Check if any input Symbols to a non ARITH are not NCONST.
	 * If only NCONST inputs to a non ARITH those input(s) must be treated
	 * like a regular input later to have proper type conversion node.
	 *******************************************************************/
	gt_flag = 1;			/* accept most cases for arithmetic expression */
	if ((sp->type == ARN || sp->type == ARNF) && sp->ftype != ARITH) {
	    gt_flag = 0;
	    for (lp = sp->u_blist; lp; lp = lp->le_next) {
		gp = lp->le_sym;
		val = lp->le_val;
		if (gp->ftype == ARITH &&
		    val != (unsigned)-1 &&
		    (gp->type != NCONST || val)) {	/* dont need gt_input */
		    gt_flag++;				/* one non NCONST found */
		    break;				/* do arithmetic expression */
		}
	    }
	}
	/********************************************************************
	 * Prepare a list of input Symbol pointers in caList[].s for
	 * copyArithmetic, while the list is not reversed yet to allow
	 * the C code index to be resolved by name for multiple occurences.
	 *
	 * Leave out NCONST for GLOBAL reduction - count NCONST for CLONE
	 *******************************************************************/
	if (sp->type == ARN || sp->type == ARNF) {
	    if (caList) memset(caList, '\0', caSize * sizeof(SymList));	/* clear for check */
	    gt_count = gt_input = 0;			/* output pre-scan for 1 gate */
	    for (lp = sp->u_blist; lp; lp = lp->le_next) {
		gp = lp->le_sym;
		val = lp->le_val;
		if (gp->ftype == ARITH &&
		    val != (unsigned)-1) {
		    while (gt_count+1 >= caSize) {
			caList = (SymList*)realloc(caList,	/* initially NULL */
			    (caSize + CASIZE) * sizeof(SymList));
			assert(caList);
			memset(&caList[caSize], '\0', CASIZE * sizeof(SymList));
			caSize += CASIZE;		/* increase the size of the array */
#if YYDEBUG
			if ((iC_debug & 0402) == 0402)
			    fprintf(iC_outFP, "op_asgn: caList[%d] increase\n", caSize);
#endif
		    }
		    if (((gp->type != NCONST || gt_flag == 0) && ++gt_input) || val) {	/* gt_input always true if incremented */
			gt_count++;			/* count all gate inputs */
			if (gp->type != NCONST) {
			    caList[gt_count].l = lp;	/* for re-numbering optimisation */
			}
			caList[gt_count].s = gp;	/* for checking duplicates */
			caList[gt_count].x = gt_input;	/* for indexing without NCONST */
#if YYDEBUG
			if ((iC_debug & 0402) == 0402)
			    fprintf(iC_outFP, "op_asgn: caList[%d] .l->le_sym %s%c le_val = 0x%04x .s %s type = %d .x %d\n",
				gt_count, lp->le_sym->name, (int)gp->type != NCONST ? ' ' : '*', val,
				gp->name, gp->type, gt_input);
#endif
		    }
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
	while ((plp = lp, lp = sp->u_blist) != 0) {
	    sp->u_blist = lp->le_next;
	    if ((gp = lp->le_sym) == rsp && var->type != ALIAS) {
		gp = var;			/* link points to right */
	    }
#ifdef	BOOT_COMPILE
	    if (sp->ftype >= MIN_ACT &&
		gt_input == 0 &&
		sp->type == LATCH &&
		(nlp = lp->le_next) != 0 &&
		nlp->le_sym == sp) {
		/****************************************************
		 * no logical inputs yet: can split function and logic
		 *
		 *	O1 = CLOCK(LATCH(I1, I2));
		 *			clock	: ---|	O1_1	C ---:	O1
		 *	O1_2	  ---%	O1_2	  ---|	O1_1
		 *	I1	  ---%	O1_2
		 *	I2	~ ---%	O1_2	<--- new gp1
		 ***************************************************/
		gp1 = atSymbol();
		gp1->type = sp->type;
		gp1->ftype = GATE;
		sp->type = OR;			/* OR default for 1 input */
		gp1->next = templist;		/* put at front of list */
		templist = gp1;
		gp1->u_blist = tlp = sy_push(gp1);	/* link self */
		tlp->le_next = nlp->le_next;		/* rest of inputs on sp */
		nlp->le_sym = gp1;
		nlp->le_next = 0;
	    }
#endif	/* BOOT_COMPILE */
	    assert(gp && gp->type < IFUNCT);	/* allows IFUNCT to use union v.cnt */
	    if (gp->type == NCONST &&
		gt_flag != 0 &&			/* dont add lone NCONST for GATE to arith expression */
		lp->le_val != (unsigned)-1 &&
		!fflag &&
		(sflag & 01) == 0) {
		if ((val = lp->le_val >> FUN_OFFSET) > 0) {
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
		copyArithmetic(lp, sp, gp, gt_input, 04, cFn);	/* insert numeric text */
		if (iFunSymExt) {
		    assert(saveAdress);
		    *saveAdress = lp->le_next;
		}
		sy_pop(lp);			/* pop all NCONST links */
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
		gp->ftype != UDFA &&		/* not an immC array */
		(nlp = gp->v_elist) != 0 &&	/* not marked symbol */
		(gp->fm & FT) == 0) {
		/********************************************************************
		 * A feedback element has occurred in the middle of an expression
		 * reduction. Since this element will not be assigned to a differnt
		 * var, the feedback list can be removed now.
		 *******************************************************************/
		gp->v_elist = 0;		/* clear the feedback list (sizeof elist > sizeof cnt for 64 bit) */
#if ! YYDEBUG || defined(SYUNION)
		gp->v_cnt = 2;			/* but retain high use count when elist was high */
#endif
		while (nlp) {			/* list with feedback to this Symbol */
		    tlp = nlp->le_next;		/* next link to a feedback Symbol */
		    sy_pop(nlp);		/* delete a feedback list link */
		    nlp = tlp;
		}
	    }
	    if (fflag && gp->ftype == ARITH) {
		if (gp->type == NCONST && gp->list == 0) {
		    gp->list = lp;		/* mark NCONST for use count in iC_listNet() */
		} else if (gp->u_blist) {
		    assert(gp->list == NULL);	/* check for expression to if else or switch */
		    /********************************************************************
		     *  Save List_e* lp pointing to an expandable expression, which is used
		     *  in an if else switch C block arithmetic, in dynamic array flpList.
		     *  These require the le_first and le_last members of the full expression.
		     *  These expressions are reduced in reverse order.
		     *******************************************************************/
		    if (flpp - flpList >= flpSize) {
			flpList = (List_e**)realloc(flpList,	/* initially NULL */
			    (flpSize + FLPSIZE) * sizeof(List_e*));
			assert(flpList);
			memset(&flpList[flpSize], '\0', FLPSIZE * sizeof(List_e*));
			flpSize += FLPSIZE;		/* increase the size of the array */
			if (flpp == NULL) {
			    flpp = flpList;
			}
#if YYDEBUG
			if ((iC_debug & 0402) == 0402)
			    fprintf(iC_outFP, "op_asgn: flpList[%d] increase\n", flpSize);
#endif
		    }
		    *flpp++ = lp;		/* fully expand arithmetic expression linked to C code */
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
		if (sp == gp && sp->ftype >= MIN_ACT && (gt_input || sp->type != LATCH)) {
		    ierror("input equals output at function gate:", sp->name);
		    sp->type = ERR;		/* cannot execute properly */
		}
		if (gp->ftype == ARITH &&	/* && gp->u_blist TD */
		    sp->ftype != OUTW &&
		    sp->type != ALIAS) {
		    if ((val = lp->le_val) == (unsigned)-1) {
			gt_input--;		/* delay is not an arithmetic input */
			gt_count--;
		    } else if (val == 0) {
			char	temp[TSIZE];
			if ((c_number + 1) > FUN_MAX) {
			    snprintf(temp, TSIZE, " %s: %d: limit is %d functions", sp->name, (c_number + 1), FUN_MAX);
			    ierror("too many C functions for this model:", temp);
			    sp->type = ERR;		/* cannot execute properly */
			}
			if ((gt_input + 1) > VAL_MASK) {
			    snprintf(temp, TSIZE, " %s: %d: limit is %d inputs", sp->name, (gt_input + 1), VAL_MASK);
			    ierror("too many inputs on arithmetic gate for this model:", temp);
			    sp->type = ERR;		/* cannot execute properly */
			}
			if (sp->type != ERR) {
			    lp->le_val = ((c_number + 1) << FUN_OFFSET)	/* arithmetic case number */
					 + gt_input + 1;	/* arithmetic input number */
			}
		    } else {
			val >>= FUN_OFFSET;
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
			     *
			     * Duplicate inputs for AND and OR Gates are silently ignored unless
			     * they are an input and its inverse which produces a Warning.
			     * Duplicate inputs for XOR and LATCH Gates always produce Warnings.
			     * Duplicate arithmetic Gate inputs are expanded in copyArithmetic()
			     * unless the first was a timer delay, in which case the arithmetic
			     * input is a new input.
			     *******************************************************************/
			  loop:				/* special loop with test in middle */
			    if (tlp->le_sym == sp) {
				val = tlp->le_val;
				if (gp->ftype == OUTW || gp->ftype == OUTX) {
				    warning("input equals output at output gate:", gp->name);
				} else if (sp->type == XOR) {
				    if (val == lp->le_val) {
					warning("XOR gate has 2 identical inputs which is always 0:", gp->name);
				    } else {
					warning("XOR gate has input and inverse which is always 1:", gp->name);
				    }
				} else if (sp->type == LATCH) {
				    warning("LATCH gate has 2 identical inputs which is indeterminate:", gp->name);
				} else if (val == lp->le_val ||
				    (gp->ftype == ARITH && val != (unsigned)-1)) {
				    copyArithmetic(lp, sp, gp, tlp->le_val & 0xff, 02, cFn);
				    gt_count++;		/* count duplicates */
				    if (sp->type == ARN || sp->type == ARNF) {
					calpp = &caList[gt_count].l;
					assert(*calpp == lp);
					*calpp = 0;	/* remove link to freed List_e */
				    }
				    sy_pop(lp);		/* ignore duplicate link */
				    goto nextInputLink;
				} else if (val != (unsigned)-1) {
				    if (sp->type == AND) {
					warning("AND gate has input and inverse which is always 0:", gp->name);
				    } else {
					warning("OR gate has input and inverse which is always 1:", gp->name);
				    }
				}
			    }
			    if (tlp->le_next) {
				tlp = tlp->le_next;	/* find last Symbol */
				goto loop;
			    }
			    assert(lp != tlp);		/* bad link ??? */
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
			    val = tlp->le_val;
			    if (gp->ftype == OUTW || gp->ftype == OUTX) {
				warning("input equals output at output gate:", gp->name);
			    } else if (sp->type == XOR) {
				if (val == lp->le_val) {
				    warning("XOR gate has 2 identical inputs which is always 0:", gp->name);
				} else {
				    warning("XOR gate has input and inverse which is always 1:", gp->name);
				}
			    } else if (sp->type == LATCH) {
				warning("LATCH gate has 2 identical inputs which is indeterminate:", gp->name);
			    } else if (val == lp->le_val ||
				(gp->ftype == ARITH && val != (unsigned)-1)) {
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
				copyArithmetic(lp, sp, gp, tlp->le_val & 0xff, 02, cFn);
				gt_count++;		/* count duplicates */
				if (sp->type == ARN || sp->type == ARNF) {
				    calpp = &caList[gt_count].l;
				    assert(*calpp == lp);
				    *calpp = 0;		/* remove link to freed List_e */
				}
				sy_pop(lp);		/* ignore duplicate link */
				lp = plp;
				goto nextInputLink;
			    } else if (val != (unsigned)-1) {
				if (sp->type == AND) {
				    warning("AND gate has input and inverse which is always 0:", gp->name);
				} else {
				    warning("OR gate has input and inverse which is always 1:", gp->name);
				}
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
		    use = (gp->em & AU) >> AU_OFS;
		    assert(use < Sizeof(iC_useText));
		    fprintf(iC_outFP, "\t%s\t%c<---%c\t\t\t// %d %s", gp->name, iC_fos[gp->ftype],
			iC_os[sp->type], lp->le_val & 0xff, iC_useText[use]);
		    if (liveDisp && gp->ftype == ARITH) {
			fprintf(iC_outFP, "\t%s\t=", gp->name);
		    }
		} else
		if ((typ = gp->type) >= MAX_LV) {
		    if (typ == TIM && (gp->em & TM1)) {	/* Timer preset off */
			fprintf(iC_outFP, "\t%s\t%c1---%c", gp->name, iC_os[typ], iC_os[sp->type]);
		    } else {
			fprintf(iC_outFP, "\t%s\t%c ---%c", gp->name, iC_os[typ], iC_os[sp->type]);
		    }
		} else
		if (gp->ftype < MAX_AR && lp->le_val == (unsigned)-1) {
		    /* reference to a timer value - no link */
		    fprintf(iC_outFP, "\t%s\t%c<---%c", gp->name, iC_fos[gp->ftype], iC_os[sp->type]);
		    if (liveDisp && gp->ftype == ARITH) {
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
		    fprintf(iC_outFP, "\t// (%d)", lp->le_val >> FUN_OFFSET);
		} else
		if ((gp->ftype == TIMR && lp->le_val > 0)) {
		    /********************************************************************
		     * Timer preset off value
		     *******************************************************************/
		    fprintf(iC_outFP, "%d", lp->le_val);
		    assert(lp->le_sym == (iFunSymExt ? gp : sp) && sp->type == TIM);
		    sp->em |= TM1;			/* mark for further debug output */
		}
	    }
	    copyArithmetic(lp, sp, gp, gt_input, sflag, cFn); /* final "\n" with current expression */
	    sflag &= ~0200;				/* iCdebug & 0200 old style listing removed */
	    if (sp == gp && (sp->type != LATCH || lp->le_val != (NOT^NOT))) {
		ierror("input equals output at gate:", sp->name);
		sp->type = ERR;				/* cannot execute properly */
	    }
	  nextInputLink: ;
	}						/* end output scan for 1 gate */
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
				assert(cp);
				len = strlen(cp);
				assert(len);
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
				    if (vaList[z1].l && vaList[z1].f) {
					int len1 = vaList[z1].l - vaList[z1].f;
					fprintf(iC_outFP, "gt: z1=%2d	%-15s %-6s v_cnt = %2d [%*.*s] '%1.1s' {%*.*s}\n",
					    z1, sp1->name, iC_full_type[sp1->type], sp1->v_cnt, len, len, cp, t_first-1,
					    len1, len1, vaList[z1].f);
				    } else {
					fprintf(iC_outFP, "gt: z1=%2d	%-15s %-6s v_cnt = %2d [%*.*s] '%1.1s' {}\n",
					    z1, sp1->name, iC_full_type[sp1->type], sp1->v_cnt, len, len, cp, t_first-1);
				    }
				    fflush(iC_outFP);
				}
#endif
				tPtr = strncpy(tPtr, cp, len+1) + len;
				if (sp1->type == NCONST) {
				    ePtr = strncpy(ePtr, cp, len+1) + len;
				    gPtr = strncpy(gPtr, cp, len+1) + len;
				} else {
				    for (y = 1; y < caSize; y++) {
					if (sp1 == caList[y].s) break;
				    }
				    assert(y < caSize);
				    if (iC_debug & 04) {	/* repeated term */
					fprintf(iC_outFP, "\t\t\t\t\t%s\t// %d", tOut, y);
					if (liveDisp && gp->ftype == ARITH) {
					    fprintf(iC_outFP, "\t%s\t=", gp->name);
					}
					fprintf(iC_outFP, "\n");
				    }
				    tOut = tPtr;
				    ePtr += sprintf(ePtr, MV "(%d)", y);	/* iC_MV */
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
	    if (sp->u_blist == 0 && gp->ftype < MIN_ACT && gt_count == 0) {
		if (iFunSymExt && sp->list != 0 && gp->type == NCONST && gp->ftype == ARITH) {
		    /********************************************************************
		     * nested constant in a cloned function definition must be treated
		     * like a normal arithmetic variable triggering an expression. jw 17/12/2023
		     *******************************************************************/
		    lp = sy_push(gp);
		    lp->le_val = ((c_number + 1) << FUN_OFFSET)	/* arithmetic case number */
				 + gt_input + 1;	/* arithmetic input number */
		    sp->u_blist = lp;
		    if (iC_debug & 04) {
			iFlag = 1;			/* may need correction by pplstfix */
			fprintf(iC_outFP, "\t%s\t%c ---%c\t%s\t%c",
			    gp->name, iC_fos[gp->ftype],
			    iC_os[sp->type], sp->name, iC_fos[sp->ftype]);
		    }
		    gp->name[0] = '\0';			/* do not extend tBuf with this name */
		} else {
		    right = constExpr_push(tBuf, 0);
		    rsp = right->le_sym;
		    var = sp;				/* tail is a constant expression */
		    goto makeAlias;
		}
	    }
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
			    fprintf(T1FP, "#line %d \"%s\"\n", lineno, inpNM);
			    fprintf(T1FP, "	return %s;\n", eBuf);
			    writeCexeTail(T1FP, "%##\n", z1);
			    if (iFunSymExt ||
				(iC_optimise & 04) != 0) {	/* save expression for full optimising as well */
				len = strlen(gBuf);		/* C expression prepared for cloning */
				cp = iC_emalloc(len+1);		/* +1 for '\0' */
				strncpy(cp, gBuf, len+1);
				assert(z1 && z1 < functionUseSize);
				functionUse[z1].c.expr = cp;
#ifdef	BOOT_COMPILE
				len = strlen(inpNM);		/* record input name if iC source for BOOT_COMPILE */
				cp = iC_emalloc(len+1);		/* +1 for '\0' */
				strncpy(cp, inpNM, len+1);
				functionUse[z1].inpNm = cp;
				functionUse[z1].lineNo = lineno;/* as well as line number at compile time */
#endif	/* BOOT_COMPILE */
			    }
			} else {
			    c_number--;				/* repeated expression */
			    goto adjustLinks;
			}
		    } else {			/* CLONED TRANSFER */
			z1 = cFn;
			if (functionUse[z1].c.expr == 0 ||
			    strcmp(gBuf, functionUse[z1].c.expr) != 0 ||	/* has C expression changed ? */
			    iC_gflag) {				/* or clone indpendent functions for gdb debugging */
			    z1 = ++c_number;			/* YES */
			    if ((iC_optimise & 04) != 0) {	/* for full optimising */
				for (z1 = 1; z1 < c_number; z1++) {
				    if (functionUse[z1].c.expr && strcmp(gBuf, functionUse[z1].c.expr) == 0) {
					break;			/* identical expression found */
				    }
				}
			    }
			    if (z1 == c_number ||		/* new modified expression */
				(iC_gflag && functionUse[z1].c_cnt && (z1 = c_number))) {
				functionUse[0].c_cnt |= F_ARITHM; /* flag for copying macro */
				writeCexeString(T1FP, z1);	/* and record for copying */
				assert(lineno > 0);		/* no nested calls in pre-defines */
				fprintf(T1FP, "#line %d \"%s\"\n", lineno, inpNM);
				fprintf(T1FP, "	return %s;\n", eBuf);
				writeCexeTail(T1FP, "%##\n", z1);
				if (iFunSymExt ||
				    (iC_optimise & 04) != 0) {	/* save expression for full optimising as well */
				    len = strlen(gBuf);		/* C expression prepared for cloning */
				    assert(len);
				    cp = iC_emalloc(len+1);	/* +1 for '\0' */
				    strncpy(cp, gBuf, len+1);
				    assert(z1 && z1 < functionUseSize);
				    functionUse[z1].c.expr = cp;
#ifdef	BOOT_COMPILE
				    len = strlen(inpNM);	/* record input name if iC source for BOOT_COMPILE */
				    cp = iC_emalloc(len+1);	/* +1 for '\0' */
				    strncpy(cp, inpNM, len+1);
				    functionUse[z1].inpNm = cp;
				    functionUse[z1].lineNo = lineno;/* as well as line number at compile time */
#endif	/* BOOT_COMPILE */
				}
			    } else {
				c_number--;			/* repeated modified expression */
			    }
			  adjustLinks:
			    val1 = (z1 << FUN_OFFSET) + 1;	/* new initial expression reference */
			    for (y = 1; (gp1 = caList[y].s) != 0; y++) {
				if ((lp = caList[y].l) != 0 &&	/* skip duplicate links */
				    (val = lp->le_val) > NOT &&	/* only arithmetic refs */
				    val != (unsigned)-1 &&	/* skip delays */
				    gp1->ftype > UDFA &&
				    gp1->ftype < MAX_ACT &&
				    gp1->type != NCONST) {
				    lp->le_val = val1++;	/* update expression reference */
				}
			    }
			    assert(y < caSize);
			}				/* cloned expression has not changed */
		    }				/* END TRANSFER */
		    if (iFunSymExt == 0) {			/* defer freeing till called */
			functionUse[0].c_cnt |= F_CALLED;	/* flag for copying temp file */
			functionUse[z1].c_cnt++;		/* free this function for copying */
		    }
		    if (iC_debug & 04) {
			fprintf(iC_outFP, "\t\t\t\t\t%s;\t// (%d)", tOut, z1);
			if (liveDisp && sp->ftype == ARITH) {
			    fprintf(iC_outFP, "\t%s\t=", sp->name);
			}
			fprintf(iC_outFP, "\n");
		    }
		}
	    }
	}
	sflag |= 0200;					/* print output name */
	if (sp->type == AND || sp->type == OR) {
	    if (gt_count > PPGATESIZE*PPGATESIZE) {	/* this is rather large and requires splitting if > 127 */
		ierror("too many inputs on AND or OR gate > 16,129:", sp->name);
		if (! iFunSymExt) sp->type = ERR;	/* cannot execute properly */
	    }
	} else
	if (sp->type == LATCH && gt_count > PPGATESIZE) {	/* cannot split LATCH - should not be necessary */
	    ierror("too many inputs on LATCH gate > 127:", sp->name);
	    if (! iFunSymExt) sp->type = ERR;		/* cannot execute properly */
	}
	/* ARN and XOR gates can be any size */
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
		    if (flpp > flpList && (var->type == SW || var->type == CF)) {
			assert(flpp <= &flpList[flpSize]);	/* not over */
			nlp  = *(flpp-1);
			assert(nlp);
			if (nlp->le_sym == sp) {
			    tlp = nlp;			/* SW or CF - inputs are not reduced */
			    --flpp;
			}
		    }
		    if (tlp) {
			t_first = tlp->le_first;
			t_last = tlp->le_last;
			if (sp->v_cnt > 1 && t_first && (c = *t_first & 0xff) >= 0x80) {
			    assert(t_last == t_first + 1);
			    c -= 0x80;
			    assert(sp == vaList[c].v);
#if YYDEBUG
			    cp = t_first;		/* for debug output */
#endif
			    t_first = vaList[c].f;
			    t_last = vaList[c].l;
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				if (t_last && t_first) {
				    int len1 = t_last - t_first;
				    fprintf(iC_outFP, "sp: c=%2d	%-15s %-6s v_cnt = %2d '%1.1s' {%*.*s}\n",
					c, sp->name, iC_full_type[sp->type], sp->v_cnt, cp,
					len1, len1, t_first);
				} else {
				    fprintf(iC_outFP, "sp: c=%2d	%-15s %-6s v_cnt = %2d '%1.1s' {}\n",
					c, sp->name, iC_full_type[sp->type], sp->v_cnt, cp);
				}
				fflush(iC_outFP);
			    }
#endif
			}
			assert((t_first == 0 && t_last == 0) || (t_first >= iCbuf && t_last >= t_first && t_last < &iCbuf[IMMBUFSIZE]));
		    }
		}
	    }
	}
    } while (sp);			/* 1 symbol resolved */
    assert(flpp == flpList);				/* not used or restored correctly */
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
	    if (lookup(var->name) == 0) {
		link_sym(var);		/* place var in the symbol table (changes sp->next) */
	    }
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
	assert(lp1->le_first == 0 || (lp1->le_first >= iCbuf && lp1->le_last < &iCbuf[IMMBUFSIZE]));
	lp2->le_first = lp1->le_first;	/* copy expr text */
	lp2->le_last  = lp1->le_last;	/* copy expr termination */
	lpc = op_push(lpc, TIM, lp2);
    }
    lp1 = op_push(sy_push(fname->v), bTyp(aex->v), aex->v);
    assert(aex->f == 0 || (aex->f >= iCbuf && aex->l < &iCbuf[IMMBUFSIZE]));
    lp1->le_first = aex->f; lp1->le_last = aex->l;
    if (ft) {
	if (lp1->le_sym->ftype == D_FF) {	/* force ft for set or reset */
	    lp1->le_sym->ftype = ft;		/* right ftype for DSR, DR */
	} else
	if (lp1->le_sym->ftype == S_FF) {
	    lp1->le_sym->ftype = R_FF;		/* right ftype for SRR */
	} else
	if (lp1->le_sym->ftype == D_SH ||
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

    if (pVal) {
	/* cblock number for ffexpr or preset off delay for timer */
	lp1->le_val = (unsigned int)(pVal->v);		/* unsigned int value for case # */
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
	if (ftyp == GATE) {			/* constant expression */
	    lv->v = op_force(lv, GATE);		/* LO or ~LO for 0 or 1 */
	} else if (ftyp != ARITH) {
	    if (ftyp == CLCKL) {
		ierror("constant assignment to imm clock:", sp->name);
	    } else if (ftyp == TIMRL) {
		ierror("constant assignment to imm timer:", sp->name);
	    } else {
		ierror("constant assignment to wrong imm type:", sp->name);
	    }
	    return 0;				/* YYERROR */
	} else if (const_push(lv)) {		/* constant to arithmetic expression */
	    ierror("no imm variable to trigger arithmetic assignment", sp->name);
	    return 0;				/* YYERROR */
	}
    }
    if (sp->type != UDF) {
	if (sp->type != ERR) {
	    if (ftyp == ARITH) {
		ierror("multiple assignment to imm int:", sp->name);
	    } else if (ftyp == GATE) {
		ierror("multiple assignment to imm bit:", sp->name);
	    } else if (ftyp == CLCKL) {
		ierror("multiple assignment to imm clock:", sp->name);
	    } else if (ftyp == TIMRL) {
		ierror("multiple assignment to imm timer:", sp->name);
	    } else {
		ierror("multiple assignment to wrong imm type:", sp->name);
	    }
	    sp->type = ERR;			/* cannot execute properly */
	}
	if (iFunSymExt) sp->list = 0;		/* do 2nd assignment for listing */
    }
    rsp = op_asgn(sv, lv, ftyp);		/* new: code before Output */
    if (ioTyp >= MAX_ACT) {			/* OUTW or OUTX */
	assignOutput(rsp, ftyp, ioTyp);
    }
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

static void
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
 *	Save immC int or immC bit const int initialiser value
 *	or a Symbol * to the const int parameter in a function block
 *	definition in a dynamic array 'iC_iniList[]'.
 *	This is necessary because there are no spare members in Symbol.
 *
 *******************************************************************/

static void
saveIniList(Symbol * sp, Valp * ini)
{
    if (iC_inip - iC_iniList >= iniSize) {
	iC_iniList = (immCini*)realloc(iC_iniList,	/* initially NULL */
	    (iniSize + INISIZE) * sizeof(immCini));
	assert(iC_iniList);
	memset(&iC_iniList[iniSize], '\0', INISIZE * sizeof(immCini));
	iniSize += INISIZE;		/* increase the size of the array */
	if (iC_inip == NULL) {
	    iC_inip = iC_iniList;
	}
#if YYDEBUG
	if ((iC_debug & 0402) == 0402)
	    fprintf(iC_outFP, "saveIniList: iC_iniList[%d] increase\n", iniSize);
#endif
    }
    iC_inip->immCvar = sp;		/* allows searching for the Symbol in iC_iniList */
    iC_inip->v       = *ini;		/* ini.lfl: 0 is a numerical initialiser; 1 is initialising const parameter; 2 err */
    iC_inip++;				/* ready for next entry to iC_iniList */
    sp->em |= EI;			/* mark that initialiser is available */
} /* saveIniList */

/********************************************************************
 *
 *	Extract immC int or immC bit const int initialiser value
 *	or a Symbol * to the const int parameter in a function block
 *	definition from the dynamic array 'iC_iniList[]'.
 *
 *	if an immC variable has been declared with an initial value sp->em & EI is set.
 *	immC initialisers are stored in the dynamic array 'iC_iniList[]'
 *	scan 'iC_iniList' for the current Symbol sp and fetch the
 *	corresponding inital value.
 *
 *******************************************************************/

Valp
extractConstIni(Symbol * sp)
{
    Valp	v;
    immCini*	ip;

    assert(sp->em & EI);
    v.lfl = v.nuv = 0;
    for (ip = iC_iniList; ip < iC_inip; ip++) {
	if (sp == ip->immCvar) {
	    if (ip->v.lfl == 0) {
		v = ip->v;
	    } else
	    if (ip->v.lfl == 1) {
		if (iFunSymExt == 0) {
		    v.nuv = evalConstParameter(ip->v.lpt);
		} else {
		    ierror("function with nested const parameters not implemented:", sp->name);	/* TODO */
		}
	    }			/* ignore ip->v.lfl == 2 err */
	    break;
	}
    }
    assert(ip < iC_inip);		/* compiler error if initialiser not found */
    return v;
} /* extractConstIni */

/********************************************************************
 *
 *	Evaluate a constant expression
 *
 *******************************************************************/

Valp
evalConstExpr(Lis * lv)
{
    List_e *	lp;
    List_e *	lp1;
    Symbol *	sp;
    Symbol *	sp1;
    char *	cp;
    int		len;
    int		inv;
    int		mal;
    Valp	v;
    char	buf[TSIZE];
    char	temp[TSIZE];

    v.lfl = 0;					/* assume an int val return */
    if (checkConstExpr(lp = lv->v) == NULL) {
	len = lv->l - lv->f;
	assert(lv->f >= iCbuf && lv->l < &iCbuf[IMMBUFSIZE] && len < TSIZE);
	strncpy(buf, lv->f, len);
	buf[len] = '\0';			/* null terminated expression text in buf[] */
	if (parseConstantExpression(buf, &v.nuv, 0)) { /* determine numerical value of constant expression */
	    ierror("not a constant expression:", buf);
	}
#if YYDEBUG
	else if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "evalConstExpr: buf '%s' has value %d\n", buf, v.nuv);
	}
#endif
    } else
    if ((sp = lp->le_sym) != 0) {
	inv = lp->le_val;
	while ( sp->type == ALIAS &&
		(lp = sp->list) != 0 &&
		(sp = lp->le_sym) != 0) {
	    lp->le_val ^= inv;
	    inv = lp->le_val;
	}
	if (sp->type == NCONST) {
	    v.nuv = sp->u_val;			/* value of constant for immCindex and immC ini */
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "evalConstExpr: Constant '%s' has value %d\n", sp->name, v.nuv);
	    }
#endif
	} else
	if (sp->type == ARNC) {			/* only immC int variable can have an initialiser */
	    if (iC_iniList && (sp->em & (ED|EI)) == (ED|EI) && sp->u_blist == 0) {
		v = extractConstIni(sp);
		sp->em |= EU;			/* mark immC variable as used */
	    } else {
		goto NotConstant;
	    }
	} else
	if (sp->type == ARN || sp->type == ARNF) {
	    if (iFunSymExt == 0) {
		len = lp->le_last - lp->le_first;
		assert(lp->le_next == 0 && lp->le_first >= iCbuf && lp->le_last < &iCbuf[IMMBUFSIZE] && len < TSIZE);
		strncpy(buf, lp->le_first, TSIZE);	/* the whole expression */
		buf[len] = '\0';			/* without statement terminator ; */
		mal = 0;
		for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {
		    if ((sp1 = lp1->le_sym) != 0 && sp1->type == ARNC &&
			iC_iniList && (sp1->em & (ED|EI)) == (ED|EI) &&
			sp1->u_blist == 0) {
			v = extractConstIni(sp1);	/* obtain numerical value */
			len = lp1->le_last - lp1->le_first;
			assert(lp1->le_first >= iCbuf && lp1->le_last < &iCbuf[IMMBUFSIZE] && len < TSIZE);
			strncpy(temp, lp1->le_first, len);
			temp[len] = '\0';		/* null terminated function text in temp[] */
			if ((cp = strstr(buf, temp)) != 0) {	/* find function text in the expression */
			    strncpy(temp, cp + len, TSIZE);	/* save remaining expression after function text */
			    snprintf(cp, TSIZE, "%d", v.nuv);	/* append numerical value returned by function */
			    strncat(buf, temp, TSIZE);		/* append saved remaining expression */
			    sp1->em |= EU;			/* mark immC variable as used */
			} else {
			    mal++;			/* each function text must occur for each member of the expression */
			}
		    } else {
			mal++;				/* no non const function calls in expression */
		    }
		}
		if (mal || parseConstantExpression(buf, &(v.nuv), 1)) { /* determine numerical value of constant expression */
		    goto NotConstant;
		}
	    } else {				/* iFunSymExt != 0 */
		len = 0;				/* clear for use as const expression flag */
		if ((sp->em & EI) && sp->u_blist == 0) {
		    len = 1;				/* flags constant parameter */
		    if ((sp->fm & FU) <= 1) {		/* target not used more than once */
			sp->fm++;			/* count use of const parameter*/
#if YYDEBUG
			if ((iC_debug & 0402) == 0402) {
			    fprintf(iC_outFP, "evalConstExpr:  const parameter use count %d for %s\n", sp->fm & FU, sp->name);
			    fflush(iC_outFP);
			}
#endif
		    }
		} else if ((sp->fm & FT) && sp->u_blist) {
		    for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {
			sp1 = lp1->le_sym;		/* scan members of the @ expression */
			assert(sp1);
			if ((sp1->type == ARN || sp1->type == ARNF) &&
			    (sp1->em & EI) && sp1->u_blist == 0) {
			    if (len == 0) len = 1;	/* flags constant parameter in expression */
			} else {
			    len = 2;			/* a non const in the expression blocks other const */
			    sp1->em |= EU;		/* mark variable as used to suppress warning */
			}
			if ((sp1->fm & FU) <= 1) {	/* target not used more than once */
			    sp1->fm++;			/* count use of parameter*/
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				fprintf(iC_outFP, "evalConstExpr:  parameter use count %d for %s\n", sp1->fm & FU, sp1->name);
				fflush(iC_outFP);
			    }
#endif
			}
		    }
		}
		if (len == 1) {			/* constant parameter or expression with only constant parameters */
		    /********************************************************************
		     *  Store a permanent copy of the constant expression at le_first/le_last
		     *  in a separate storage block and point le_first/le_last to that permanent block.
		     *******************************************************************/
		    assert(lp && lp->le_first >= iCbuf && lp->le_last < &iCbuf[IMMBUFSIZE]);
		    len = lp->le_last - lp->le_first;
		    cp = iC_emalloc(len+1);		/* +1 for '\0' */
		    strncpy(cp, lp->le_first, len+1);
		    cp[len] = '\0';			/* null terminated expression text in permanent copy */
		    lp->le_first = cp;		/* point to permanent copy */
		    lp->le_last  = cp + len;	/* point to end of permanent copy */
		    v.lpt = lp;			/* return pointer to constant parameter expression */
		    v.lfl = 1;			/* mark as pointer return */
		} else {
		    goto NotConstant;
		}
	    }
	} else
	if (sp->name && strcmp(sp->name, "LO") == 0) {
	    /********************************************************************
	     *  Convert bit constant LO to 0
	     *  Convert bit constant ~LO or its alias HI to 1
	     *******************************************************************/
	    assert(sp->type == OR);		/* alias HI has been resolved above */
	    sp->em |= EU;			/* LO has been used */
	    v.nuv = inv;			/* bit value 0 or 1 */
	} else {
	    goto NotConstant;
	}
    } else {
      NotConstant:
	v.lfl = 2;				/* mark as error */
	v.nuv = 0;				/* do not mark sp with ERR - it is OK */
	if (sp) ierror("constExpr is not a constant:", sp->name);
	else ierror("constExpr is not a constant:", NULL);
    }
    return v;
} /* evalConstExpr */

/********************************************************************
 *
 *	Evaluate a const parameter expression
 *
 *	    List_e * lp either points to a const arithmetic parameter
 *			marked by em & EI and u_blist == 0
 *	    or		by an arithmetic @ expression containing only
 *			const arithmetic parameters marked by em & EI
 *
 *	A permanent copy of the constant expression available during
 *	function block definition has been copied to a permanent storage
 *	block and linked to lp->le_first instead of the volatile version
 *	in iCtext.
 *
 *	Since constant expressions are executed here at compile time,
 *	every time the function block containing this constant expression
 *	is cloned, this different mechanism is called for.
 *
 *	For regular immediate arithmetic expressions, which compile to a
 *	C function executed at run time, similar blocks are created and
 *	linked to functionUse[z1].c.exp, indexed by C function number z1.
 *
 *******************************************************************/

static int
evalConstParameter(List_e * lp)
{
    List_e *	lp1;
    Symbol *	sp;
    Symbol *	sp1;
    int		mal = 0;
    int		value;
    char *	formalName;
    char *	callName;
    char *	cp;
    char	buf[TSIZE];
    char	temp[TSIZE];

    assert(lp && lp->le_sym && lp->le_next == 0 && lp->le_first && lp->le_last && *(lp->le_last) == '\0');
    strncpy(buf, lp->le_first, TSIZE);
    if ((sp = lp->le_sym)->u_blist == 0) {
	if ((sp->em & EI) == 0) goto MalFormed;					/* else evaluate single constant parameter */
    } else {
	if ((sp->em & EI) != 0 || (lp = sp->u_blist) == 0) goto MalFormed;	/* else evaluate constant parameter expression */
    }
    for ( ; lp; lp = lp->le_next) {	/* scan either single or expression parameters */
	sp = lp->le_sym;
	assert(sp && sp->name && sp->type == ARN && sp->list && sp->u_blist == 0);
	if ((sp->em & EI) == 0) {
	    mal++;
	    continue;
	}
	formalName = sp->name;		/* parameter name used in the constant expression */
	lp1 = sp->list;
	assert(lp1 && lp1->le_next == 0);
	sp1 = lp1->le_sym;
	if (!sp1 || !sp1->name || sp1->type != NCONST) goto MalFormed;
	callName = sp1->name;		/* numerical constant text */
	if ((cp = strstr(buf, formalName)) != 0) {
	    strncpy(temp, cp + strlen(formalName), TSIZE);
	    *cp = '\0';
	    strncat(buf, callName, TSIZE);
	    strncat(buf, temp, TSIZE);	/* replace formal name by call name in expression */
	} else {
	    mal++;			/* each formal name must occur for each member of the expression */
	}
    }
    if (mal || parseConstantExpression(buf, &value, 0)) { /* determine numerical value of constant expression */
      MalFormed:
	ierror("const parameter expression is mal-formed:", buf);
	value = 0;
    }
#ifndef	NOCONSTLIST
    constExprString = strdup(buf);
#endif	/* NOCONSTLIST */
    return value;
 } /* evalConstParameter */

/********************************************************************
 *
 *	Listing for undefined C variable
 *	Generate and assign output I/O if it is a Q[XBWL]n variable
 *
 *	Format lists with size - if size == 0 put lines close together
 *				 if size != 0 space 1 line before and after
 *				 if size >= 0 no ini value output
 *				 if size == -1 output ini.val or ini.sym in listing
 *					      save ini in a dynamic array
 *				 if sp is an immC array ((ARNC || LOGC) && UDFA) &&
 *				    size >   1 size is array size
 *				    size == -2 ini.sym is array size const parameter
 *
 *	Set 'em' bit ED in Symbol to indicate that an immC variable has
 *	been defined.
 *	If ini value has been saved set 'em' bit EI in Symbol to indicate
 *	that an initialiser has been saved.
 *
 *******************************************************************/

void
listGenOut(Symbol * sp, int size, Valp * ini)
{
    char *	name;
    int		ioTyp;
    static int	shortFlag;
    List_e *	lp;
    int		len;

    if (sp == NULL || (sp->em & ED) == 0) {	/* do not repeat if already defined */
	if (iC_debug & 04) {
	    /********************************************************************
	     * compile listing output for undefined C variable
	     *******************************************************************/
	    if (sp) {
		if (size != 0) {
		    fprintf(iC_outFP, "\n");/* NL at beginning of item */
		    shortFlag = 0;		/* reset list */
		} else {
		    shortFlag = 1;		/* a list item was seen */
		}
		fprintf(iC_outFP, "\t\t= ---%c\t%s", iC_os[sp->type], sp->name);
		if (sp->ftype != GATE) {
		    if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
			if (size == -2) {
			    if (ini->lfl == 1) {
				lp = ini->lpt;
				assert(lp);
				len = lp->le_last - lp->le_first;
				fprintf(iC_outFP, "\t[%*.*s]", len, len, lp->le_first);	/* immC array size expression */
			    }			/* ignore ip->v.lfl == 2 err */
			} else {
			    fprintf(iC_outFP, "\t[%d]", size);		/* immC array numeric size */
			}
		    } else {
			fprintf(iC_outFP, "\t%c", iC_fos[sp->ftype]);
		    }
		}
		if (size == -1) {
		    if (sp->ftype == GATE) fprintf(iC_outFP, "\t");
		    if (ini->lfl == 0) {
			fprintf(iC_outFP, "\t%d", ini->nuv);		/* immc initialiser numeric value */
		    } else
		    if (ini->lfl == 1) {
			lp = ini->lpt;
			assert(lp);
			len = lp->le_last - lp->le_first;
			fprintf(iC_outFP, "\t%*.*s", len, len, lp->le_first);	/* immC initialiser const expression */
		    }							/* ignore ip->v.lfl == 2 err */
		}
		fprintf(iC_outFP, "\n");	/* NL at end of item */
	    }
	    if (size != 0 && (sp != NULL || shortFlag)) {
		fprintf(iC_outFP, "\n");	/* extra NL at end of single item or list */
	    }
	}
	if (sp) {
	    sp->em |= ED;			/* immC variable has been defined */
	    if (size == -1) {
		/********************************************************************
		 * Save immC int or immC bit initialiser value in a dynamic array
		 * 'iC_iniList[]', since there are no spare members in Symbol.
		 *******************************************************************/
		saveIniList(sp, ini);
	    }
	    if (sp->em & EO) {		/* QXx.y QBz etc */
		/********************************************************************
		 * generate and assign output I/O
		 *******************************************************************/
		name = sp->name;
		assert(name[0] == 'Q');
		switch (name[1]) {
		case 'X':
		    ioTyp = OUTX;
		    break;
		case 'B':
		case 'W':
		case 'L':
		case 'H':
		    ioTyp = OUTW;
		    break;
		default:
		    assert(0);		/* illegal Q variable */
		    break;
		}
		assignOutput(sp, sp->ftype, ioTyp);	/* assign to I/O output variable */
	    }
	}
    }
} /* listGenOut */

/********************************************************************
 *
 *	Implicit delay of 1 tick for ctref : texpr ; without delay
 *	(cexpr|texpr[,dexpr])
 *
 *******************************************************************/

static Symbol *
symbolOne(void)
{
    Symbol *	sp;
    if ((sp = lookup("1")) == 0) {		/* Constant Symbol "1" */
	sp = install("1", NCONST, ARITH);
	sp->u_val = 1;
    }
    return sp;
} /* symbolOne */

List_e *
delayOne(List_e * tp)
{
    List_e *	lp;
    lp = sy_push(symbolOne());
    lp->le_val = (unsigned)-1;			/* mark link as timer value */
    return op_push(tp, TIM, lp);
} /* delayOne */

/********************************************************************
 *
 *	Check parameter count in 'cCall' and clean away dummy list
 *	elements produced in 'cList'
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
 *	and removed with its name and link.
 *
 *	Finally compare the parameter count with v.cnt in 'cName', which
 *	contains the parameter count collected in the extern C function
 *	declaration.
 *
 *******************************************************************/

List_e *
cCallCount(Symbol * cName, List_e * cParams, int pcnt)
{
    List_e *	lp;
    List_e **	lpp;
    Symbol *	sp;

    if ((lp = cParams) != NULL) {
	sp = lp->le_sym;
	assert(sp);
	for (lpp = &sp->u_blist; (lp = *lpp) != 0; ) {
	    if (lp->le_sym == 0) {
		*lpp = lp->le_next;		/* remove dummy link from list */
		sy_pop(lp);
	    } else {
		lpp = &lp->le_next;		/* next list element */
	    }
	}
	if (sp->u_blist == 0) {			/* sp has only links which have been popped */
	    freeTemp(cParams);			/* bypass sp on templist and delete cParams and sp */
	    cParams = NULL;
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
	cName->type  = CWORD;			/* no longer an imm variable */
	cName->u_val = CNAME;			/* yacc type */
	cName->v_cnt = pcnt;			/* UNDEF stop more error messages */
	if (iC_Sflag) ierror("strict: call of an undeclared C function or macro:", cName->name);
    } else
    if (pcnt != cName->v_cnt) {
	char	tempBuf[TSIZE];			/* CNAME */
	snprintf(tempBuf, TSIZE, "%s %d (%d)", cName->name, pcnt, (int)cName->v_cnt);
	if (iC_Sflag) {
	    ierror("strict: call parameter count does not match extern C declaration:", tempBuf);
	} else {
	    warning("call parameter count does not match extern C declaration:", tempBuf);
	}
    }
    if (pcnt == 0 && (iC_Wflag & W_DEPRECATED_LOGIC)) {
	warning("C function will not fire this expression:", cName->name);
    }
    return cParams;
} /* cCallCount */

/********************************************************************
 *
 *	Count parameters in 'cList'
 *	If 'aexpr == NULL' generate a dummy link with a null pointer
 *	in le_sym for transporting pcnt in le_val. This link will
 *	be removed in 'cCall' - actually cCallCount()
 *	If the first parameter is a dummy (aexpr == 0), a dummy
 *	temp Symbol is put on templist by op_push(), which must
 *	also be removed in 'cCall' if all other parameters are dummys
 *
 *******************************************************************/

List_e *
cListCount(List_e * cPlist, Lis * lv)
{
    List_e *	aexpr;
    int		pcnt = 0;

    if (cPlist) {
	pcnt = cPlist->le_val;	/* count from cPlist before sy_pop in op_push */
	cPlist->le_val = 0;		/* restore so that expression is correct */
    }
    if (lv->v == NULL) {
	lv->v = sy_push(NULL);		/* dummy link for counting */
    } else {
	aexpr = lv->v;
    }
    aexpr = op_push(cPlist, ARN, op_force(lv, ARITH));
    assert(aexpr);
    aexpr->le_val = pcnt + 1;
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "cPlist: parameter %d\n", aexpr->le_val);
	fflush(iC_outFP);
    }
#endif
    return aexpr;
} /* cListCount */

/********************************************************************
 ********************************************************************
 *
 * class DefineFunction
 *
 *	Code for generating the template for an immediate function.
 *	In the rest of the documentation these are called immediate
 *	function blocks to distinguish them from C-functions.
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
 *
 *	1a) imm void <function name>(<parameter list>) {<function body>} for
 *	    void functions or
 *	1b) imm <type> <function name>(<parameter list>) {<function body>} for
 *	    value functions. The name of the function return object in value
 *	    function expressions is "this" (no "this" in void functions).
 *
 *	2a) <type> <name> for a value parameter or
 *	2b) assign <type> <name> for an assign parameter or
 *	2c) <type> <name>[<size>] for an array parameter (<size> is optional).
 *	    The modifier imm is optional before <type> for 2a) to 2c) - or
 *	2d) const int <name> for a constant value parameter (no imm allowed).
 *	    Any number of 2a) to 2d) parameters can be used in any order in
 *	    the comma separated <parameter list>.
 *
 *	3)  imm <type> <name> in the <function body> declares a function
 *	    internal variable which will be cloned with different names in
 *	    different instances, when the function is called.
 *
 *	4)  extern imm <type> <name> in the <function body> declares that
 *	    name will be assigned outside the function. Such a global variable
 *	    may not be declared locally or assigned before its declaration in
 *	    the function. After the function definition it must be declared in
 *	    the source in which it will be assigned or used.
 *
 *	The upshot is, that any variable used in a function must be declared
 *	before it is used in a function. This was found to be more important
 *	for functions, where bugs can easily creep in when one assumes a
 *	certain variable is function internal, but was not declared as such
 *	and it is then accidentally compiled as global. This would not become
 *	evident until the function is used. It also makes the use of global
 *	variables in a function a little bit more onerous, which it should be,
 *	since their use can cause loss of a broad overview. By having the
 *	extern declaration, at least one knows which names are used for what in
 *	each function. Breaking this rule will cause a warning, not an error.
 *
 ********************************************************************
 *******************************************************************/

/********************************************************************
 *
 *	Set up the function definition head Symbol before any parameters
 *	have been parsed.
 *
 *	iFunBuffer contains the function head name for the whole of the
 *		   definition.
 *	iFunSymExt points past the '@' in the name and is the place where
 *		   local name extensions can temprarily be written.
 *		   It also flags, that a function is being defined.
 *
 *	For a value function also set up the return Symbol
 *
 *******************************************************************/

Symbol *
functionDefHead(unsigned int ftyp, Symbol * funTrigger)
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
    if (ftyp == UDFA) {
	iRetSymbol.v = 0;			/* void function has no return Symbol */
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "vFunHead: imm %s %s\n", iC_full_ftype[ftyp], funTrigger->name);
	    fflush(iC_outFP);
	}
#endif
    } else {
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
		     * is overwritten to match latest version of statement linked to 'sp'.
		     * If not overwritten, the members of the old 'varList' do not match
		     * the Symbols of the expression net linked to 'sp' and an execerror
		     * results when the expression is cloned - which might not matter
		     * since there is a hard error anyway, but it is untidy.
		     *******************************************************************/
		    vlp->le_sym = varList;	/* no 'varList' in declaration */
		}
	    }
	} else if ((sp->em & EM) == 0 && typ != ERR) {	/* ignore extern */
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
returnStatement(Lis * lv)
{
    Symbol *	sp;
    int		ftyp;

    if ((sp = iRetSymbol.v) != NULL) {
	ftyp = sp->ftype;
	if (lv->v == 0) {
	    if (ftyp == GATE) {			/* constant expression */
		lv->v = op_force(lv, GATE);	/* LO or ~LO for 0 or 1 */
	    }
	    else if (ftyp != ARITH) {
		if (ftyp == CLCKL) {
		    ierror("constant assignment to imm clock:", sp->name);
		} else if (ftyp == TIMRL) {
		    ierror("constant assignment to imm timer:", sp->name);
		} else {
		    ierror("constant assignment to wrong imm type:", sp->name);
		}
		return 0;			/* YYERROR */
	    } else if (const_push(lv)) {	/* constant to arithmetic expression */
		ierror("no imm variable to trigger arithmetic assignment", sp->name);
		return 0;			/* YYERROR */
	    }
	}
	if ((sp = lv->v->le_sym) == 0) {
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
	sp = op_asgn(&iRetSymbol, lv, ftyp);
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
functionDefinition(Symbol * functionHead, List_e * fParams)
{
    List_e *	lp;
    List_e *	lp1;
    List_e *	vlp;
    List_e **	lpp;
    Symbol *	sp;
    Symbol *	sp1;
    Symbol **	spp;
    int		instanceNum;			/* save early union u.val u.blist */
    int		saveCount = 0;			/* count parameter links for saving */

    if (functionHead->fm &&			/* built-in function signature */
	(iC_Wflag & W_FUNCTION_DELETE)) {
	warning("CAREFUL - redefining a built-in function with alternative:", functionHead->name);
    }
    instanceNum = functionHead->v_cnt;		/* read from file if -a option */
    if (iRetSymbol.v) {
	if (iRetSymbol.v->type == UDF) {
	    ierror("no return statement in function:", functionHead->name);
	}
	assert(iRetSymbol.v->u_blist == 0);	/* state after expression reduction */
	iRetSymbol.v->u_blist = iRetSymbol.v->list;
	iRetSymbol.v->fm |= FM|FA|FP|0x01;	/* mark return Symbol 'this' as used once, assigned and parameter */
	iRetSymbol.v->list = sy_push(functionHead);	/* point to own function head */
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
    functionHead->u_blist = lp = stmtList;	/* global value */
    stmtList = 0;				/* ready for next function */
    while (lp) {				/* mark function statement list */
	sp = lp->le_sym;			/* assign parameters and internal declarations */
	assert(sp);
	if (sp->type == UDF && ffexprFlag == 0) {
	    warning("undefined gate in function:", sp->name);
	}
	if (sp->u_blist == 0) {
	    sp->u_blist = sp->list;		/* expression now ready for cloning */
	    sp->list = 0;			/* clear pointer to real Symbol */
	    saveCount++;			/* space for saving nested decl or assign parameter */
	    sp->fm |= FM|FA;			/* mark as assigned function Symbol */
	    if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
		for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {	/* immC array - scan member list */
		    assert(lp1->le_sym);
		    lp1->le_sym->fm |= FM;	/* mark array member as non-assigned function Symbol */
		    saveCount++;		/* space for saving nested statement array members */
		}
	    }
	} else {
	    if (!sp->list || sp->list->le_sym != functionHead) {	/* 'this' marked above */
		if (sp->list) {
		    fprintf(iC_errFP, "*** Error: %s ->list %s does not equal functionHead %s\n", sp->name, sp->list->le_sym->name, functionHead->name);
		} else {
		    fprintf(iC_errFP, "*** Error: %s ->list %p\n", sp->name, sp->list);
		}
		exit(-1);
	    }
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
     *******************************************************************/
    functionHead->list = lp = fParams;		/* yacc stack value of parameter list */
    while (lp) {				/* mark formal parameter list */
	sp = lp->le_sym;			/* assign and value parameters */
	assert(sp);
	if (sp->type == UDF && ffexprFlag == 0) {
	    warning("undefined parameter in function:", sp->name);
	}
	if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
	    sp->u_blist = sp->list;		/* immC array now ready for cloning */
	    sp->list = 0;			/* clear pointer to real Symbol */
	    saveCount++;			/* space for saving nested decl or assign parameter */
	    sp->fm = FM|FP|0x02;		/* set immC array parameter as used - no merge */
	    for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {	/* immC array - scan member list */
		assert(lp1->le_sym);
		lp1->le_sym->fm |= FM|FP;	/* mark parameter array member as non-assigned function Symbol */
		saveCount++;			/* space for saving nested array parameter members */
	    }
	} else {
	    if (sp->list != 0) {		/* was expression assigned and not on statement list ? */
		ierror("trying to assign to a value parameter:", sp->name);
		for (lp1 = sp->list; lp1; lp1 = vlp) {	/* has no follow ups - but just in case */
		    vlp = lp1->le_next;		/* next expression link */
		    free(lp1);			/* delete expression link */
		}
		sp->list = 0;			/* clear pointer to real parameter */
	    }
	    sp->fm |= FM|FP;			/* mark as function Symbol and parameter */
	    if (sp->u_blist == 0) {
		saveCount++;			/* space for saving nested read parameter */
	    } else {
		assert(sp->fm & FA);		/* assign was marked in statement list scan */
		sp->fm |= 0x02;			/* mark used - no merge (ALIAS and target have FM set) */
	    }
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
    lp = functionHead->u_blist;			/* statement list */
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
			if (((sp1->fm & FM) != 0 || sp1->name == 0) &&	/* no globals, no '@' in Pass 1 */
			    (sp1->fm & FU) <= 1) {/* target not used more than once */
			    sp1->fm++;		/* count ALIAS use in merge candidate */
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				fprintf(iC_outFP, "iFunDef:  ALIAS use count %d for %s\n", sp1->fm & FU, sp1->name);
				fflush(iC_outFP);
			    }
#endif
			}
			sp1 = vlp->le_sym;
			assert(sp1);
			lp1->le_sym = sp1;	/* transfer alias target to net list element */
		    } else			/* do not count target of ALIAS twice */
		    if (((sp1->fm & FM) != 0 || sp1->name == 0) &&	/* no globals, no '@' in Pass 1 */
			(sp1->fm & FU) <= 1) {/* target not used more than once */
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
    lpp = &functionHead->u_blist;		/* statement list */
    while ((lp = *lpp) != 0) {
	sp = lp->le_sym;			/* return, assign parameter or internal declaration */
	if (sp && (sp->fm & FU) == 0 &&
	    sp->type != SW && sp->type != CF && sp->type != ALIAS &&
	    (iC_Wflag & W_FUNCTION_PARAMETER)) {
	    warning("function variable was never used:", sp->name);
	}
	vlp = lp->le_next;			/* next varList link */
	if (sp->type == ALIAS &&		/* exclude ALIAS assignment to parameter or this */
	    (sp->fm & (FA|FP)) != (FA|FP) &&
	    (sp->list == 0 || sp->list->le_sym != functionHead)) {
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
    lp = functionHead->list;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;			/* assign or read parameter */
	if (sp && (sp->fm & FU) == 0 && (iC_Wflag & W_FUNCTION_PARAMETER)) {
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
    lp = functionHead->u_blist;			/* statement list */
    if (lp) {
	lp->le_val = instanceNum;		/* allows call to number instances */
	lp = lp->le_next;			/* first varList link */
	assert(lp);				/* must be a pair */
	assert(saveCount >= 0);
	lp->le_val = saveCount;			/* allows call to get save block */
    } else {
	ierror("function has no statements!", functionHead->name);
    }
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "iFunDef:  imm %s %s(%d)\n", iC_full_ftype[functionHead->ftype], functionHead->name, saveCount);
	fflush(iC_outFP);
    }
#endif
    return functionHead;			/* yacc stack value */
} /* fuctionDefinition */

/********************************************************************
 *
 *	Clear a previous function definition by deleting all its data
 *	objects
 *
 *	It is possible to clear pre-defined built-in functions and it is
 *	possible to re-define some of them, but not all. All functions
 *	pre-defined in terms of themself (which is boot-compiled initially)
 *	will cause compile errors on re-definitionin (function body is cleared).
 *	These functions are D() SH() RISE() CHANGE() and CHANGE2().
 *	It would be possible to re-define CLOCK() and CLOCK(2) in terms
 *	of TIMER1() and TIMER12() and have functioning code. It is of course
 *	possible to re-define functions in terms of others, but it would
 *	only obfuscate the sources. Thus the warning VERY DANGEROUS.
 *
 *	The only reason clearing of pre-defined built-ins is allowed, is
 *	that it lets compilation continue without error and assertion faults
 *	and it does no harm if the cleared function block is not used.
 *
 *	The only sensible use of this feature would be to re-define SR() to
 *	have the functionality of SRX(), although even that has traps,
 *	because SRX() does not behave exactly like SR() in some cases.
 *	Another use would be to define FORCE() in terms of AND and OR gates.
 *	To get the same truth table as the pre-defined FORCE() function block.
 *	This is only of academic interest - it does work but the new function
 *	block uses 4 gates instead of 1 gate.
 *
 *	Care is taken not to free the links and Symbols of the pre-defined net
 *	wich are not on the heap.
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
    Symbol *		clearList;
    int			instanceNum;		/* restore instance number later */
    int			d;

    /********************************************************************
     * Pass 1: statement list
     *******************************************************************/
    assert(functionHead);
    slp = functionHead->u_blist;		/* start of statement list */
    if (slp == 0) {
	return functionHead;			/* already cleared */
    }						/* instanceNum for dummy from file not changed */
    d = 1;					/* flag to allow free() of non pre-defined objects */
    if (functionHead->fm) {
	if (iC_Wflag & W_FUNCTION_DELETE) {
	    warning("VERY DANGEROUS to delete a built-in function:", functionHead->name);
	}
	d = 0;					/* do not free() objects - old function net simply left to rot */
	if (functionHead->v_glist == NULL) {	/* unless function block has an alternative, fm must remain */
	    functionHead->fm = 0;		/* re-definition can be free()d but does not have to be */
	}
    }
    if (iC_Wflag & W_FUNCTION_DELETE) {
	warning("existing function definition is deleted:", functionHead->name);
    }
    instanceNum = slp->le_val;			/* this function call instance number */
    clearList = 0;				/* unlinked Symbols */
    while (slp) {
	sp = slp->le_sym;			/* formal satement head Symbol */
	assert(sp);
	if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
	    for (lp1 = sp->u_blist; lp1; lp1 = vlp) {
		vsp = lp1->le_sym;		/* immC array - scan member list */
		assert(vsp && vsp->list == 0);	/* call leaves link to real member cleared */
		if (vsp->name) {		/* name was cleared if already unlinked */
		    unlink_sym(vsp);		/* unlink formal member Symbol */
		    if (d) free(vsp->name);	/* free name space */
		    vsp->name = NULL;		/* no longer done in unlink_sym - jw 20141007 */
		    vsp->next = clearList;
		    clearList = vsp;
		}
		vlp = lp1->le_next;
		if (d) free(lp1);		/* delete formal member link */
	    }
	} else {
	    lp = sp->u_blist;			/* cloned expression links */
	    while (lp) {
		lp1 = lp->le_next;		/* next expression link */
		if (d) free(lp);		/* delete expression link */
		lp = lp1;
	    }
	    lp = sp->list;			/* possible link to function head */
	    while (lp) {
		lp1 = lp->le_next;		/* next expression link */
		if (d) free(lp);		/* delete expression link */
		lp = lp1;			/* has no follow ups - but just in case */
	    }
	}
	if (sp->name) {				/* name was cleared if already unlinked */
	    unlink_sym(sp);			/* unlink formal statement head Symbol */
	    if (d) free(sp->name);		/* free name space */
	    sp->name = NULL;			/* no longer done in unlink_sym - jw 20141007 */
	    sp->next = clearList;
	    clearList = sp;
	}
	vlp = slp->le_next;			/* next varList link */
	if (d) free(slp);			/* delete statement link */
	assert(vlp);				/* statement list is in pairs */
	vsp = vlp->le_sym;			/* varList of temp Symbols */
	while (vsp) {				/* varList may be empty */
	    lp = vsp->u_blist;			/* cloned expression links */
	    while (lp) {
		lp1 = lp->le_next;		/* next expression link */
		if (d) free(lp);		/* delete expression link */
		lp = lp1;
	    }
	    sp = vsp->next;			/* next varList Symbol */
	    if (d) free(vsp);				/* delete varList Symbol which has no name */
	    vsp = sp;				/* and is not in the Symbol Table */
	}
	slp = vlp->le_next;			/* next statement link */
	if (d) free(vlp);			/* delete varList link */
    }
    /********************************************************************
     * Pass 2: parameter list
     * links to real parameters were deleted at the end of a call
     *******************************************************************/
    slp = functionHead->list;			/* parameter list */
    while (slp) {
	sp = slp->le_sym;			/* formal parameter Symbol */
	assert(sp && sp->list == 0);		/* call leaves link to real para cleared */
	if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
	    for (lp1 = sp->u_blist; lp1; lp1 = vlp) {
		vsp = lp1->le_sym;		/* immC array - scan member list */
		assert(vsp && vsp->list == 0);	/* call leaves link to real member cleared */
		if (vsp->name) {		/* name was cleared if already unlinked */
		    unlink_sym(vsp);		/* unlink formal member Symbol */
		    if (d) free(vsp->name);	/* free name space */
		    vsp->name = NULL;		/* no longer done in unlink_sym - jw 20141007 */
		    vsp->next = clearList;
		    clearList = vsp;
		}
		vlp = lp1->le_next;
		if (d) free(lp1);		/* delete formal member link */
	    }
	}
	if (sp->name) {				/* name was cleared if already unlinked */
	    unlink_sym(sp);			/* unlink formal parameter Symbol */
	    if (d) free(sp->name);		/* free name space */
	    sp->name = NULL;			/* no longer done in unlink_sym - jw 20141007 */
	    sp->next = clearList;
	    clearList = sp;
	}
	vlp = slp->le_next;
	if (d) free(slp);			/* delete formal parameter link */
	slp = vlp;
    }
    /********************************************************************
     * Finally: free the unlinked Symbols
     * they might have been linked twice if immC members on arrays
     *******************************************************************/
    for (sp = clearList; sp; sp = clearList) {
	clearList = sp->next;
	if (d) free(sp);			/* free previously unlinked Symbol */
    }
    functionHead->u_blist = functionHead->list = 0;	/* clear for next definition */
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
 *	Constants and I/O variables are universally global and do not
 *	need to be declared extern (Input variables not declared extern).
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
    int		typ;
    if (terminal) {
	typ = terminal->type;
	if (iFunSymExt &&
	    strncmp(terminal->name, iFunBuffer, iFunSymExt - iFunBuffer) != 0 &&
	    (terminal->em & EM) == 0 &&
	    typ != NCONST &&
	    typ != INPX &&
	    typ != INPW &&
	    typ != ERR &&
	    terminal != iclock) {
	    warning("global variable not declared extern:", terminal->name);
	}
	return sy_push(terminal);		/* link for expression expansion */
    }
    execerror("terminal Symbol not defined ???", NS, __FILE__, __LINE__);
    return 0;					/* never gets here - for -Wall */
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

/********************************************************************
 *
 *	Get a real parameter of a called immediate function
 *
 *	Parameter 1:	if 0 build a new parameter list head
 *			else use the Symbol
 *	Parameter 2:	lp is link to current real parameter
 *	return		points to Symbol head of the parameter list
 *
 *	The first call to getRealParameter(0, $1.v) will set up a Symbol,
 *	which is the head of a new parameter list. It is pointed at by hsp.
 *		hsp->list points to the first parameter
 *		hsp->u_blist points to the last parameter
 *		hsp->fm is filled with a signature of value parameters
 *			a value following a timer is not a value parameter
 *
 *	Subsequent calls link the next parameter to hsp->u_blist->le_next
 *
 *	All this is done before the execution of cloneFunction(), which
 *	includes setting up the correct function head according to the
 *	signature returned, analysing the parameters against the formal
 *	parameters and actually cloning the function. Nested calls will
 *	have their own *hsp, which obviates the need for an extra stack.
 *
 *******************************************************************/

Symbol *
getRealParameter(Symbol * hsp, List_e * lp)
{
    unsigned char	ftyp;
    unsigned int	signature;
    static unsigned int	prevFtype;

    assert(lp && lp->le_sym && lp->le_next == 0);
    if (hsp == 0) {
	hsp = (Symbol *) iC_emalloc(sizeof(Symbol));
	hsp->list = hsp->u_blist = lp;	/* first and only parameter so far */
	prevFtype = 0;
    } else {
	assert(hsp->u_blist && hsp->u_blist->le_next == 0);
	hsp->u_blist->le_next = lp;
	hsp->u_blist = lp;		/* next and current last parameter */
    }
    ftyp = lp->le_sym->ftype;
    signature = (ftyp == GATE) ? 1 : (ftyp == ARITH) ? 2 : 0;
    if (signature && prevFtype != TIMRL) {
	hsp->fm <<= 2;
	hsp->fm |= signature;
    }
    prevFtype = ftyp;
    return hsp;
} /* getRealParameter */

static List_e *		iFormCurr = 0;	/* current formal parameter pointer in handleRealParameters() */
static Symbol *		iFormPrev = 0;	/* previous formal parameter pointer in handleRealParameters() */
static List_e *		iFunClock = 0;	/* temporary list of unresolved clock parameters */
static List_e **	iSav;		/* dynamic array for saving parameter + statement links */

static Symbol *		cloneSymbol(Symbol * sp);
static List_e *		cloneList(Symbol * ssp, Symbol ** cspp, Symbol * rsp, int x);

/********************************************************************
 *
 *	Push a function call
 *
 *	Push parameters of a nested call of this function on stack 'saveFunBs'
 *	Save function template parameter and declared variable links
 *
 *	Array parameter links are saved in Pass 1.1.
 *	Value parameter links are saved in Pass 1.2.
 *	Declared array links are saved in Pass 2.1.
 *	Assign parameters and Declarations are saved in Pass 2.1.
 *
 *	None of the links are cleared in these passes in case they
 *	occur on two list like immC array and immC parameter list
 *	or assign parameter and statement list. These double values
 *	will be save twice but also restored correctly twice.
 *
 *	The order must match the order these links are restored at the
 *	end of cloneFunction()
 *
 *	Array parameter links are cleared in Pass 3.1.
 *	Value parameter links are cleared in Pass 3.2.
 *	Declared array links are cleared in Pass 4.1.
 *	Assign parameters and Declarations are cleared in Pass 4.1.
 *
 *	The order in Passes 3 and 4 do not matter.
 *
 *******************************************************************/

static Symbol *
pushFunCall(Symbol * functionHead)
{
    List_e *		lp;
    List_e *		lp1;
    Symbol *		sp;
    Symbol *		sp1;
    int			saveCount;
    int			cF;
    int			cFlag;

    assert(functionHead);
    lp = functionHead->u_blist;
    if (lp != 0) {
	lp = lp->le_next;			/* first varList link */
	assert(lp);				/* must be a pair */
	saveCount = lp->le_val;			/* allows call to store save block */
	iSav = saveCount ? (List_e **) iC_emalloc(saveCount * sizeof(List_e *)) : NULL;
	/********************************************************************
	 * Pass 1.1: Save array members in parameter list
	 *******************************************************************/
	cFlag = cF = 0;
	lp = functionHead->list;			/* parameter list */
	while (lp) {
	    sp = lp->le_sym;
	    if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
		for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {
		    sp1 = lp1->le_sym;			/* immC array - scan member list */
		    assert(sp1);
		    assert(iSav);
		    iSav[cF++] = sp1->list;		/* save previous member template link */
		    if (sp1->list) cFlag++;		/* clear link later */
		}
	    }
	    lp = lp->le_next;
	}
	/********************************************************************
	 * Pass 1.2: Save parameter list
	 *******************************************************************/
	lp = functionHead->list;			/* parameter list */
	while (lp) {
	    sp = lp->le_sym;
	    assert(sp);
	    if (sp->u_blist == 0 || (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC))) {
		iSav[cF++] = sp->list;			/* save previous parameter template link */
		if (sp->list) cFlag++;			/* clear link later */
	    }
	    lp = lp->le_next;
	}
	/********************************************************************
	 * Pass 2.1: Save array members in statement list
	 *******************************************************************/
	lp = functionHead->u_blist;			/* statement list */
	while (lp) {
	    sp = lp->le_sym;
	    if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
		for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {
		    sp1 = lp1->le_sym;			/* immC array - scan member list */
		    assert(sp1);
		    iSav[cF++] = sp1->list;		/* save previous member template link */
		    if (sp1->list) cFlag++;		/* clear link later */
		}
	    }
	    lp = lp->le_next;				/* next varList link */
	    assert(lp);					/* statement list is in pairs */
	    lp = lp->le_next;				/* next statement link */
	}
	/********************************************************************
	 * Pass 2.2: Save statement list
	 *******************************************************************/
	lp = functionHead->u_blist;			/* statement list */
	while (lp) {
	    sp = lp->le_sym;
	    assert(sp);
	    sp1 = ((lp1 = sp->list) != 0) ? lp1->le_sym : 0;
	    if (sp1 != functionHead) {			/* bypass return link */
		iSav[cF++] = sp->list;		/* save previous declaration or assign link */
		if (sp->list) cFlag++;			/* clear link later */
	    }
	    lp = lp->le_next->le_next;			/* next statement link */
	}
	assert(cF == saveCount);
	if (cFlag) {
	    /********************************************************************
	     * Pass 3.1: Clear array members in parameter list
	     *******************************************************************/
	    lp = functionHead->list;			/* parameter list */
	    while (lp) {
		sp = lp->le_sym;
		if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
		    for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {
			sp1 = lp1->le_sym;		/* immC array - scan member list */
			sp1->list = 0;			/* clear link */
		    }
		}
		lp = lp->le_next;
	    }
	    /********************************************************************
	     * Pass 3.2: Clear parameter list
	     *******************************************************************/
	    lp = functionHead->list;			/* parameter list */
	    while (lp) {
		sp = lp->le_sym;
		assert(sp);
		if (sp->u_blist == 0 || (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC))) {
		    sp->list = 0;			/* clear link */
		}
		lp = lp->le_next;
	    }
	    /********************************************************************
	     * Pass 4.1: Clear array members in statement list
	     *******************************************************************/
	    lp = functionHead->u_blist;			/* statement list */
	    while (lp) {
		sp = lp->le_sym;
		if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
		    for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {
			sp1 = lp1->le_sym;		/* immC array - scan member list */
			sp1->list = 0;			/* clear link */
		    }
		}
		lp = lp->le_next->le_next;		/* next statement link */
	    }
	    /********************************************************************
	     * Pass 4.2: Clear statement list
	     *******************************************************************/
	    lp = functionHead->u_blist;			/* statement list */
	    while (lp) {
		sp = lp->le_sym;
		assert(sp);
		sp1 = ((lp1 = sp->list) != 0) ? lp1->le_sym : 0;
		if (sp1 != functionHead) {		/* bypass return link */
		    sp->list = 0;			/* clear link */
		}
		lp = lp->le_next->le_next;		/* next statement link */
	    }
	}
	/********************************************************************
	 * set up for cloning
	 *******************************************************************/
	iCallHead = functionHead;		/* make avalable globally */
	assert(iCallHead);			/* must have a function head */
	iFormCurr = iCallHead->list;		/* first formal parameter - may be 0 */
	iFormPrev = 0;				/* first previous formal parameter is 0 */
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
 *	(jw 24 Aug 2012)
 *	See modified clock handling for two consecutive formal clocks under CLCKL
 *	This is needed to implement functions like ST(set, clock), where the
 *	final clock is usually a timer,delay to control this as a monoflop
 *	and not the clock for the set parameter. New monoflop as function block:
 *	    imm bit ST(bit set, clock sc, clock tc	// tc not optional
 *		{ this = SR_(set, sc, this, tc); 	// without extra reset
 *	    imm bit SRT(bit set, clock sc, bit reset, clock rc, clock tc)
 *		{ this = SRR_(set, sc, reset, rc, this, tc); } // with extra reset
 *	old SRT() fails when called in a function block and was scrapped.
 *
 *******************************************************************/

static List_e *
handleRealParameter(List_e * plp, List_e * lp)
{
    Symbol *	fsp;				/* formal parameter Symbol */
    Symbol *	rsp;				/* real parameter Symbol */
    Symbol *	psp;				/* previous real parameter Symbol */
    Symbol *	tsp;				/* temporary Symbol */
    Symbol *	dsp = 0;			/* delay Symbol */
    List_e *	rlp;				/* return link if clock is a timer */
    List_e *	clp;				/* list element for missing clocks */
    List_e *	lp1;
    List_e *	flp;				/* link to formal array member */
    List_e *	tlp;				/* link to real array member */
    int		i;
    Lis		sl;

    if (lp == 0) {				/* final call from cloneFunction() */
	if (plp) {				/* to clear unresolved formal parameters */
	    rsp = symbolOne();			/* default delay 1 after last timer parameter */
	} else {
	    if (iFormCurr && iFormPrev && iFormPrev->ftype == CLCKL) {
		if ((rsp = lookup("iCerr")) == 0) {
		    icerr = rsp = install("iCerr", ARN, GATE);
		}
	    } else {
		rsp = iclock;			/* default iClock if no real clock parameter (not ERR) */
	    }
	}
	rlp = sy_push(rsp);			/* dummy link, which will be popped */
    } else {
	rlp = lp;
	rsp = rlp->le_sym;			/* current real parameter Symbol */
    }
    assert(rsp);
    if (rsp->type == NCONST) {
	assert(rsp->name && (isdigit(rsp->name[0]) || rsp->name[0] == '-'));
	sl.f = rsp->name;			/* provides a numeric string for evalConstExpr */
	sl.l = rsp->name + strspn(rsp->name, "-012345689");	/* even if dummy timer delay 1 */
    } else {
	sl.f = rlp->le_first; sl.l = rlp->le_last;
    }
    sl.v = rlp;					/* allows evalConstExpr in op_force */

    while (iFormCurr) {
	fsp = iFormCurr->le_sym;		/* current formal parameter Symbol */
	assert(fsp && fsp->fm);			/* u_blist not cleared on parse error */
	if (plp) {
	    if (rsp->ftype == GATE && iFormCurr->le_next) {	/* previous clock parameter was a timer */
		/********************************************************************
		 * If there is no formal parameter for this bit, it is a "wrong parameter type".
		 *
		 * A real bit parameter after a timer is not usually meant to be a
		 * delay value, whereas an int after a timer usually is a delay. Therefore
		 * provide the default 1 delay before a real bit parameter after a timer..
		 * If an int parameter after a timer is not meant to be the delay, one
		 * must precede it with a real parameter 1 to simulate the default delay.
		 * Else a parameter count error.
		 *
		 * If for some obscure reason a bit is supposed to be the delay
		 * (can only be 0 or 1) the bit must be forced to an int by an expression
		 * ie: +IX0.0.  (This is the bit to int cast mechanism in iC).
		 * Else a parameter count error.
		 *******************************************************************/
		dsp = symbolOne();		/* default delay 1 for timer parameter before bit */
		iFormPrev = fsp;		/* save previous parameter Symbol on stack */
		iFormCurr = iFormCurr->le_next;	/* skip timer - next formal parameter to process */
		fsp = iFormCurr->le_sym;	/* now current formal parameter Symbol */
		goto HandleRealParameter;	/* handle this bit; then distribute previous timer */
	    } else
	    if (rsp->ftype != ARITH) {
		ierror("wrong parameter type for timer delay:", rsp->name);
		rsp->type = ERR;
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
	    }
	    /* used up a real ARITH or GATE parameter for timer delay - else error */
	} else {
	  HandleRealParameter:
	    switch (fsp->ftype) {
	    case UDFA:
		if (rsp->ftype != UDFA ||
		    rsp->type != fsp->type ||
		    rsp->list == 0 ||
		    (fsp->v_cnt != 0 && fsp->v_cnt != (rsp->list->le_val & VAL_MASK))) {
		    ierror("wrong immC array parameter type or array size:", rsp->name);
		    rsp->type = ERR;
		} else if (fsp->v_cnt) {
		    flp = fsp->u_blist;		/* head of formal array member list */
		    tlp = rsp->list;		/* head of real array member list */
		    for (i = 0; i < fsp->v_cnt; i++) {
			assert(flp && flp->le_sym && flp->le_sym->list == NULL && tlp && tlp->le_sym);
			flp->le_sym->list = sy_push(tlp->le_sym);	/* link real member to template */
			flp = flp->le_next;	/* next formal member link */
			tlp = tlp->le_next;	/* next real member link */
		    }
		    assert(flp == NULL && tlp == NULL);
		}
		break;
	    case ARITH:
		/********************************************************************
		 *  A formal const parameter must be matched by a real parameter with
		 *  type NCONST, which is either a constant number or a constant expression
		 *******************************************************************/
		if ((fsp->em & EI) &&
		    rsp->type != NCONST &&
		    ! (rsp->name &&
		       (strcmp(rsp->name, "LO") == 0 ||	/* GATE constant LO will be forced to ARITH 0 */
		        strcmp(rsp->name, "HI") == 0	/* GATE constant HI will be forced to ARITH 1 */
		       )
		      )
		   ) {
		    ierror("formal const parameter not matched by a constant expression:", rsp->name);
		    rsp->type = ERR;		/* check for ERR when using this parameter */
		}	/* could use else here because bit clock or timer will not be NCONST (?) */
		if (rsp->ftype == GATE) {
		    if (fsp->u_blist == 0) {	/* assign parameter forced in op_asgn */
			rlp = op_force(&sl, ARITH);	/* force value parameter to int */
		    }
		} else
		if (rsp->ftype != ARITH) {
		    ierror("wrong parameter type for int:", rsp->name);
		    rsp->type = ERR;
		}
#if ! YYDEBUG || defined(SYUNION)
		if (rsp->v_cnt <= 2)
#endif
		    rsp->v_cnt = fsp->fm & FU;	/* transfer use count for op_push and op_asgn */
		break;
	    case GATE:
		if (rsp->ftype == ARITH) {
		    if (fsp->u_blist == 0) {	/* assign parameter forced in op_asgn */
			rlp = op_force(&sl, GATE); /* force value parameter or constant to bit */
		    }
		} else
		if (rsp->ftype != GATE) {
		    ierror("wrong parameter type for bit:", rsp->name);
		    rsp->type = ERR;
		}
		break;
	    case CLCKL:
		/********************************************************************
		 * This clock might be iClock if next formal parameter is also a clock
		 * in which case this clock will go to that next clock unless the next
		 * real parameter is also a clock or timer. Second clock is not optional.
		 *
		 * Must be careful with recursive calls of functions returning this clock.
		 * Maintain iFormPrev on saveFunBs stack to hold previous formal parameter
		 * Store link to this real clock or timer in current formal parameter fsp->list
		 * If previous formal parameter is also a clock move it from previous iFormPrev->list
		 * to this second clock (might be timer with a delay already stored (or not))
		 * unless this real parameter is also a clock or timer, in which case it stays.
		 * Replace previous iFormPrev->list with iClock if it was moved and resolve
		 * iFunClock chain with that iClock.
		 *
		 * Only now can you link clock or timer in previous formal clock parameter to
		 * iFunClock chain. This must be tested for all parameter ftypes and final call.
		 * Note: the clock to distribute is the 2nd last real clock (in iFormPrev) and
		 * not the last real clock if there are two consecutive clocks last).
		 *******************************************************************/
		if (rsp->ftype == CLCKL) {
		    lp = rlp;			/* in case dummy iClock */
		} else
		if (fsp->u_blist) {		/* assign parameter must be ftype CLCKL */
		    ierror("wrong parameter type for 'assign clock':", rsp->name);
		    rsp->type = ERR;
		} else
		if (rsp->ftype == TIMRL) {
		    fsp->list = rlp;		/* link real timer to formal parameter */
						/* - not next formal parameter iFormNext */
						/* - still have same iFormPrev with CLCKL */
		    return rlp;			/* current real timer parameter -> plp next */
		} else				/* parameter is not a clock or timer */
		if (iFormPrev && iFormPrev->ftype == CLCKL) {
		    fsp->list = clp = iFormPrev->list;	/* move previous clock parameter to the 2nd clock */
		    iFormPrev->list = sy_push(iclock);	/* 1st clock is default iClock */
		    if (clp->le_sym == 0 ) {
			ierror("2nd consecutive clock is not optional - wrong type:", iFormPrev->name);
			rsp->type = ERR;
		    } else {
			tsp = iclock;			/* default iClock */
			while (iFunClock) {		/* resolve formal clock parameters now */
			    iFunClock->le_sym = tsp;	/* link unresloved clock to this clock */
			    clp = iFunClock->le_next;	/* next unresolved clock */
			    iFunClock->le_next = 0;	/* clean up list */
			    iFunClock = clp;
			}
			if (lp == 0) {
			    sy_pop(rlp);		/* dummy parameter not used */
			}
			iFormPrev = fsp;		/* save previous parameter Symbol on stack */
			iFormCurr = iFormCurr->le_next;	/* next formal parameter to process */
			continue;
		    }
		} else {
		    fsp->list = clp = sy_push(0);	/* clock is currently unresolved */
		    clp->le_next = iFunClock;		/* because there is no clock parameter */
		    iFunClock = clp;			/* resolve in function call */
		    iFormPrev = fsp;			/* save previous parameter Symbol on stack */
		    iFormCurr = iFormCurr->le_next;	/* next formal parameter to process */
		    continue;				/* do not step to next real parameter */
		}
		break;
	    case TIMRL:
		if (rsp->ftype != TIMRL) {
		    ierror("wrong parameter type for timer:", rsp->name);
		    rsp->type = ERR;
		}
		break;
	    default:
		ierror("wrong type for function call parameter:", rsp->name);
		rsp->type = ERR;
		break;
	    }
	    fsp->list = rlp;			/* link real to formal parameter */
	}
	/********************************************************************
	 * Here if formal ftype was UDFA, ARITH, GATE, TIMRL or error
	 *******************************************************************/
	if (iFormPrev && iFormPrev->ftype == CLCKL) {
	    tlp = iFormPrev->list;		/* not two consecutive clocks */
	    if ((tsp = tlp->le_sym) != 0) {	/* previous clock or timer */
		/********************************************************************
		 * Link the previous real clock or timer + delay, which was deferred
		 * until this non CLCKL to all previous unresolved formal clock parameters
		 *******************************************************************/
		if (dsp) {
		    assert(tlp->le_next == 0);	/* no delay attached to timer yet */
		    tlp->le_next = lp1 = sy_push(dsp);	/* default delay 1 before a bit parameter */
		    lp1->le_val = (unsigned)-1;	/* mark link as timer value */
		}
		if ((tlp = tlp->le_next) != 0) {
		    dsp = tlp->le_sym;		/* previous timer delay */
		    assert(dsp->ftype == ARITH);
		}				/* else clock with tlp == 0 */
		while (iFunClock) {		/* find unresolved formal clock parameters */
		    iFunClock->le_sym = tsp;	/* link unresloved clock to this clock */
		    clp = iFunClock->le_next;	/* next unresolved clock */
		    if (tlp) {
			iFunClock->le_next = lp1 = sy_push(dsp);/* clone a delay link */
			lp1->le_val = (unsigned)-1;		/* mark link as timer value */
		    } else {
			iFunClock->le_next = 0;	/* clean up list */
		    }
		    iFunClock = clp;
		}
	    }
	}
	iFormPrev = fsp;			/* save previous parameter Symbol on stack */
	iFormCurr = iFormCurr->le_next;		/* next formal parameter to process */
	if (lp) return 0;
	if (iFormPrev->ftype == CLCKL) {	/* out of real parameters - set up dummy correctly */;
	    if ((rsp = lookup("iCerr")) == 0) {
		icerr = rsp = install("iCerr", ERR, GATE);
	    }
	} else {
	    rsp = iclock;			/* dummy iClock */
	}
	rsp->type = ERR;
	ierror("no link from formal to real parameter:", fsp->name);
	rlp = sy_push(rsp);			/* dummy link, which will be popped */
    }
    /* formal parameters exhausted */
    if (lp) {
	ierror("called function has too many real parameters:", rsp ? rsp->name : 0);
    }
    if (iFormPrev && iFormPrev->ftype == CLCKL) {
	tlp = iFormPrev->list;			/* not two consecutive clocks */
	if ((tsp = tlp->le_sym) != 0 && (tlp = tlp->le_next) != 0) {
	    assert(tsp->ftype == TIMRL);
	    dsp = tlp->le_sym;			/* previous timer delay */
	}
    } else {
	tsp = iclock;				/* default iClock when lp == 0 */
	tlp = 0;
    }
    while (iFunClock) {				/* find unresolved formal clock parameters */
	iFunClock->le_sym = tsp;		/* link unresloved clock to this clock */
	clp = iFunClock->le_next;		/* next unresolved clock */
	if (tlp && tsp) {
	    iFunClock->le_next = lp1 = sy_push(dsp); /* clone a delay link */
	    lp1->le_val = (unsigned)-1;		/* mark link as timer value */
	} else {
	    iFunClock->le_next = 0;		/* clean up list */
	}
	iFunClock = clp;
    }
    return 0;
} /* handleRealParameter */

/********************************************************************
 *
 *	Clone a function template in a function call
 *
 *	This call is preceded by getting the parameter list, which is
 *	passed in hsym.v independly of the actual function called in fhs.
 *	Nested functions will all have been evaluated by now.
 *
 *	hsym.v->fm contains a signature for the value parameters, which
 *	is used initially to get the correct function head. This is then
 *	set up by pushFunCall() (although no actual pushing on a stack is
 *	needed any more). Subsequently parameters linked to hsym.v are
 *	passed to handleRealParameters().
 *
 *	Cloning is done by scanning each entry pair in the statement list.
 *	The blist of each Symbol found is cloned recursively, unless
 *	it is a parameter or assignable, which are marked by ->fm&FM (0x80)
 *
 *	The first List_e in the statement list holds an instance counter for
 *	generated function internal variables - to allow unique naming.
 *	This field le_val is zero when function is first compiled for
 *	each source. When a function is included in several sources
 *	the counter is passed to later runs via _List1.h.
 *
 *******************************************************************/

Lis
cloneFunction(Sym * fhs, Sym * hsym, Val * par)
{
    Symbol *		functionHead = fhs->v;	/* actual function head */
    Symbol *		hsp = hsym->v;		/* head of actual rParams - could be 0 */
    List_e *		plp = 0;
    Lis			rll;			/* return Lis */
    List_e *		slp;			/* link for statement Symbol */
    List_e *		vlp;			/* link for varList */
    List_e *		lp;
    List_e *		lp1;
    List_e *		lp2;
    List_e *		lp3 = 0;
    List_e **		lpp;
    Symbol *		sp;
    Symbol *		sp1;
    Symbol *		sp2;			/* Symbol in expression template */
    Symbol *		sp3;
    Symbol *		vsp;			/* Symbol in varList */
    Symbol *		csp;			/* Symbol in cloned expresssion */
    Symbol *		rsp;			/* return Symbol for feedback */
    Symbol *		tend = templist;	/* end of templist for this function */
    Sym			sv;			/* Sym to assign cloned expression to */
    Lis			sl;			/* Lis pointing to cloned expression */
    int			instanceNum;		/* this function call instance number */
    int			instanceFlg;
    int			saveCount;		/* saveFunPt,iFormHead,iFormCurr,iFormPrev,iFunClock */
    int			cF;
    int			typ;
    unsigned char	fm;
    unsigned int	lval;			/* logical sign of a temp */
    char *		cp;
    char *		ep;
    int			len;
    int			len1;
    int			size;
    int			i;
    char		temp[TSIZE];

    assert(functionHead);
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "cloneFunction: start\n");
	fflush(iC_outFP);
    }
#endif
#ifndef	NOCONSTLIST
    constExprString = NULL;
#endif	/* NOCONSTLIST */
    rll.v = 0;					/* return link */
    rll.f = fhs->f; rll.l = par->l;		/* adjust old text limits */
    /********************************************************************
     * check for the correct function head to match the parameter signature
     *					CLOCK	CLOCK2	1 or 2 bit or int
     *	fm = functionHead->fm		00 11	11 11	0x3	0xf
     *	hsp->fm (hsp->fm & fm)	00 01	00 01=	00 01= but already found
     *				00 10	00 10=	00 10= but already found
     *				01 01	00 01x	01 01=
     *				01 10	00 10x	01 10=
     *				10 01	00 01x	10 01=
     *				10 10	00 10x	10 10=
     *					CHANGE	CHANGE2	1 bit or 1 int
     *	fm = functionHead->fm		00 01	00 10	0x1	0x2
     *	hsp->fm (hsp->fm & fm)	00 01	00 01=	00 00x
     *				00 10	00 00x	00 10=
     *				01 01	00 01x	00 00x
     *				01 10	00 00x	00 10x etc
     *******************************************************************/
    fm = functionHead->fm;			/* save functionHead->fm to restore it */
    if (hsp) {
	while (fm !=0) {
	    if (hsp->fm == (hsp->fm & fm)) {	
		break;				/* correct function head found */
	    } else {
		if (functionHead->v_glist) {	/* this function has an alternative parameter ramp */
		    functionHead = functionHead->v_glist;
		    fm = functionHead->fm;	/* save again to restore it */
		} else {
		    warning("called function has incorrect parameters:", functionHead->name);
		    break;
		}
	    }
	}
    }
    /********************************************************************
     * push this function head and distribute real parameters
     *******************************************************************/
    pushFunCall(functionHead);
    if (hsp) {
	for (lp = hsp->list; lp; lp = lp1) {
	    lp1 = lp->le_next;
	    lp->le_next = 0;
	    plp = handleRealParameter(plp, lp);
	}
    }
    /********************************************************************
     * now do the actual cloning
     *******************************************************************/
    slp = lp = functionHead->u_blist;		/* get 2 numbers in first elements of statement list */
    if (slp == 0) {
	ierror("called function has no statements - cannot execute!", functionHead->name);
	return rll;				/* with 0 pointer */
    }
    instanceNum = lp->le_val + 1;		/* this function call instance number */
    instanceFlg = 0;				/* update only if it was used */
    lp = lp->le_next;				/* first varList link */
    assert(lp);					/* must be a pair */
    saveCount = lp->le_val;			/* allows call to store save block */
    /********************************************************************
     * previously parameter links were set up from formal to real parameters
     * find unresolved clock parameters (since real clock parameters optional)
     *******************************************************************/
    while (iFormCurr || iFunClock) {
	vlp = handleRealParameter(plp, 0);
	if (plp) {
	    vlp = handleRealParameter(vlp, 0);
	}
	if (iFormCurr) {
	    ierror("called function has too few real parameters:", functionHead->name);
	    while (iFormCurr) {
		vlp = handleRealParameter(vlp, 0);
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
	    vsp->list = (List_e *) cloneSymbol(vsp); /* point to new internal Symbol on templist */
	    vsp = vsp->next;			/* next varList Symbol */
	}
	/********************************************************************
	 * if list != 0 clone the expression head associated with this statement.
	 *   By doing this after varList, cloned head is at front of templist.
	 * if list == 0 indicates a function internal Symbol - only generate
	 *   simple immC variables with our without fixed or dynamic initialiser
	 *   and immC arrays with fixed or dynamic size.
	 * The remaining internal statement head Symbols are not cloned till Pass2.
	 * They then come even earlier on templist. This is necessary because
	 * these statements are assigned before the return statement.
	 *******************************************************************/
	sp1 = slp->le_sym;			/* expression head template */
	assert(sp1);				/* check in Pass 1 only */
	assert(sp1->name);
	if ((lp = sp1->list) == 0) {		/* function internal variable */
	    if ((typ = sp1->type) == ARNC || typ == LOGC) {
		if (sp1->ftype == UDFA) {	/* immC array */
		    if ((sp1->em & EI) && (lp1 = sp1->u_blist) != 0) {
			/********************************************************************
			 * generate a dynamic immC array whose size is determined by const parameter(s)
			 *******************************************************************/
			if (iFunSymExt == 0) {
			    size = evalConstParameter(lp1);
#if YYDEBUG
			    if ((iC_debug & 0402) == 0402) {
				fprintf(iC_outFP, "cloneFunction: %s[%d] has dynamic size\n", sp1->name, size);
				fflush(iC_outFP);
			    }
#endif
			    assert(size > 0);
			    cp = sp1->name + strlen(functionHead->name);/* locate original extension of dynamic array */
			    assert(cp[0] == '@');			/* has failed during development */
			    snprintf(temp, TSIZE, "%s_%d_%s", functionHead->name, instanceNum, cp+1);
			    instanceFlg++;				/* instanceNum was used so update */
			    for (i = 0; i < size; i++) {		/* generate consecutively numbered members */
				snprintf(temp, TSIZE, "%s_%d_%s%d", functionHead->name, instanceNum, cp+1, i);
				if ((sv.v = lookup(temp)) == 0) {	/* locate position in ST */
				    sv.v = install(temp, UDF, iC_ftypes[sp1->type]);	/* UDF to locate auto member in Pass 3 */
				}
				if (i == 0) {
				    lp3 = lp2 = sy_push(sv.v);		/* head of new member list */
				    lp2->le_val = size;
				} else {
				    lp2 = lp2->le_next = sy_push(sv.v);	/* link to end of new member list */
				}
			    }

			} else {
			    ierror("function with nested const parameters not implemented:", sp1->name);	/* TODO */
			}
		    } else {
			/********************************************************************
			 * generate a fixed immC array whose size is determined by a constant
			 *******************************************************************/
			for (lp1 = sp1->u_blist; lp1; lp1 = lp1->le_next) {
			    sp2 = lp1->le_sym;	/* clone immC members */
			    assert(sp2 && (sp2->type == ARNC || sp2->type == LOGC));
			    if (sp2->list == 0) {	/* unless immC member separately declared */
				if (iFunSymExt) {
				    int	n = 0;	/* cloned in a function definition */
				    cp = sp2->name + strlen(functionHead->name);/* locate original extension */
				    assert(cp[0] == '@' && isprint(cp[1]));	/* extension must be at least 1 character */
				    strncpy(iFunSymExt, cp+1, iFunEnd - iFunSymExt);/* copy ext to new fun name */
				    cp = iFunBuffer + strlen(iFunBuffer);	/* end of new var name */
				    while (lookup(iFunBuffer) != 0) {		/* accepts original name for first instance */
					snprintf(cp, iFunEnd - cp, "%d", ++n);	/* simply count up */
				    }						/* position in ST located in while */
				    sv.v = install(iFunBuffer, UDF, sp2->ftype);/* UDF to locate auto member in Pass 3 */
				    *iFunSymExt = '\0';
				} else {				/* see naming comments in next section */
				    cp = sp2->name + strlen(functionHead->name);	/* locate original extension */
				    assert(cp[0] == '@');		/* has failed during development */
				    snprintf(temp, TSIZE, "%s_%d_%s", functionHead->name, instanceNum, cp+1);
				    instanceFlg++;			/* instanceNum was used so update */
				    if ((sv.v = lookup(temp)) == 0) {	/* locate position in ST */
					sv.v = install(temp, UDF, sp2->ftype);	/* UDF to locate auto member in Pass 3 */
				    }
				}
				sp2->list = sy_push(sv.v);		/* link internal cloned Symbol to template */
			    } else {
				sv.v = sp2->list->le_sym;		/* previously declared immC Symbol */
			    }
			    if (lp1 == sp1->u_blist) {
				lp3 = lp2 = sy_push(sv.v);		/* link cloned or previously declared Symbol to array */
				lp2->le_val = lp1->le_val;		/* transfer array size to first new link */
			    } else {
				lp2 = lp2->le_next = sy_push(sv.v);	/* link later link to previous link */
			    }
			}
		    }
		}
	    } else {
		/********************************************************************
		 * generate a simple immC variable
		 *******************************************************************/
		typ = UDF;			/* imm */
	    }
	    if (iFunSymExt) {
		int	n = 0;			/* cloned in a function definition */
		cp = sp1->name + strlen(functionHead->name);	/* locate original extension */
		assert(cp[0] == '@' && isprint(cp[1]));		/* extension must be at least 1 character */
		strncpy(iFunSymExt, cp+1, iFunEnd - iFunSymExt);/* copy ext to new fun name */
		cp = iFunBuffer + strlen(iFunBuffer);		/* end of new var name */
		while (lookup(iFunBuffer) != 0) {		/* accepts original name for first instance */
		    /********************************************************************
		     * Different instances of the same extension can occur for multiple
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
		sv.v = install(iFunBuffer, typ, sp1->ftype);
		*iFunSymExt = '\0';
		collectStatement(sv.v);		/* put in definition stmtList like a decl */
	    } else {
		/********************************************************************
		 * Naming for global internal names needs instanceNum, because names
		 * may be generated in several independent source files, unlike names
		 * in function definitions above, which are contained in one file.
		 *******************************************************************/
		cp = sp1->name + strlen(functionHead->name);	/* locate original extension */
		assert(cp[0] == '@');		/* has failed during development */
		snprintf(temp, TSIZE, "%s_%d_%s", functionHead->name, instanceNum, cp+1);
		instanceFlg++;			/* instanceNum was used so update */
		if ((sv.v = lookup(temp)) == 0) {	/* locate position in ST */
		    sv.v = install(temp, typ, sp1->ftype); /* Symbol for declared variable */
		}
	    }
	    sp1->list = sy_push(sv.v);		/* link internal cloned Symbol to template */
	    if (sp1->type == ARNC || sp1->type == LOGC) {
		Valp	v;
		if (sp1->ftype == UDFA) {
		    sv.v->list = lp3;		/* complete immC array with link to member list */
		} else if (iC_iniList && (sp1->em & EI)) {
		    v = extractConstIni(sp1);
		    saveIniList(sv.v, &v);	/* transfer const ini value or expression link to cloned immC variable */
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402) {
			fprintf(iC_outFP, "cloneFunction: Pass 1: %s immC variable with initialiser %d saved\n", sv.v->name, v.nuv);
			fflush(iC_outFP);
		    }
#endif
		}
	    }
	} else
	if (lp->le_sym == functionHead) {
	    rsp = cloneSymbol(sp1);		/* clone return expression head Symbol */
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
	    } else if (rsp->type == ALIAS) {
		iC_optimise |= 0x80;		/* block iC_optimise & 02 for ALIAS function return */
	    }
	}					/* assign parameters alrady have a link */
	slp = vlp->le_next;			/* next statement */
    }
    /********************************************************************
     * Pass 2: scan again and clone the expression for each statement
     *******************************************************************/
    if ((iC_optimise & 0x82) == 0x02) {		/* do not optimise if ALIAS function return */
#if YYDEBUG
	tfirst = rll.f;				/* start of replacement arithmetic string */
	tlast = rll.l;				/* rll was set up early in clonFunction */
#endif
	*(rll.f) = STX;				/* STX marks start of call text */
	ttp = rll.l -1;				/* internal expressions are retained */
	assert(ttp && *ttp == ')');
	*ttp = ETX;				/* ETX marks end of call text */
	ttp = iCstmtp;				/* pointer into optimised text - initialised to &iCbuf[0] */
	assert(ttp);
    }
    slp = functionHead->u_blist;		/* statement list */
    while (slp) {
	vlp = slp->le_next;
	sp2 = slp->le_sym;			/* expression head template */
	assert(sp2 && (sp2->fm & FM));		/* marked statement list head Symbol */
	typ = sp2->type;			/* type's below are marked with fmxx */
	if (typ != ARNC && typ != LOGC) {
	    assert(sp2->u_blist || typ == UDF || typ == ERR);
	    /********************************************************************
	     * clone the expression head associated with this statement (except immC)
	     *******************************************************************/
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "cloneFunction: statement  %s\t%s\n",
		    sp2->name, iC_full_type[typ]);
		fflush(iC_outFP);
	    }
#endif
	    lp = sp2->list;			/* link to real return, assign parameter or */
	    assert(lp);				/* internal Symbol linked in pass 1 */
	    if ((sv.v = lp->le_sym) == functionHead) {
		assert(rsp);			/* return Symbol was set in Pass 1 */
		rll.v = cloneList(sp2, &rsp, rsp, 1); /* clone the rest of the expression */
	    } else if (sp2->u_blist) {		/* assign parameter or internal Symbol */
		assert(sv.v);			/* TD what happens to link if not used ??? */
		csp = cloneSymbol(sp2);		/* clone parameter expression head Symbol */
		sv.v->u_blist = cloneList(sp2, &csp, rsp, 2); /* clone the rest of the expression */
	    }
	}
	slp = vlp->le_next;			/* next statement */
    }
    if ((iC_optimise & 0x82) == 0x02 && ttp > iCstmtp) {
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
     * Pass 8 cleans up freelist after optimisation.
     *******************************************************************/
    if (iC_optimise & 01) {
	List_e*		lp3;
	int		mcnt = 0;		/* count merge candidate found */
	int		ecnt = 0;		/* count excess merge candidate */
#if YYDEBUG
	int		xcnt = 0;
#endif
	/********************************************************************
	 * Pass 2.0: detect ->fm&FU values 1 and 2
	 *           Some gates have an initial count set in ->fm&FU
	 *           in cloneSymbol(). If the value is 1 increment mcnt so that
	 *           it matches ecnt if ->fm&FU is incremented again in Pass 2.1.
	 *******************************************************************/
	for (sp = templist; sp && sp != tend; sp = sp->next) {	/* target gate */
	    sp->fm &= ~(FM | FMI);			/* zero FM and FMI for temp gates */
	    for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {
		sp1 = lp1->le_sym;			/* input to this gate */
		assert(sp1);
		if ((sp->type == XOR ||			/* XOR, AND or OR */
		     sp->type == AND ||
		     sp->type == OR  ||
		     sp->type == ARN ||			/* target can be any ftype */
		     sp->type == SW  ||			/* SW or CF can also have XOR AND OR as input */
		     sp->type == CF)) {			/* so they add to ->fm&FU to inhibit merging */
		    if (sp1->ftype == ARITH || sp1->ftype == GATE) {
			if ((sp1->type == XOR   ||	/* XOR, AND, OR, LATCH or ARN */
			     sp1->type == AND   ||
			     sp1->type == OR    ||
			     sp1->type == LATCH ||
			     sp1->type == ARN   ||
			     sp1->type == ARNF)	&&
			    sp1->u_blist != 0	&&
#if ! YYDEBUG || defined(SYUNION)
			    sp1->v_cnt <= 2	&&	/* test v_elist in union for any valid v_cnt */
#else
			    sp1->v_elist == 0	&&
#endif
			    (sp1->fm & FU) == 1) {	/* used once during function definition */
			    mcnt++;			/* merge candidate from function definition */
			}
		    }
		}
	    }
	}
	/********************************************************************
	 * Pass 2.1: scan templist and store use count in temp symbols ->fm&FU
	 *           fm&FU can be 0	- never used in any expression
	 *                        1	- used once in an expression - merge
	 *                        2	- used more than once - don't merge
	 *
	 *           Target gates of types XOR AND or OR may be merged, but
	 *           targets of type SW or CF may also have gates as input,
	 *           so those extra inputs must also be counted in ->fm&FU
	 *
	 *           Also count inputs to the target gates - if AND or OR gate
	 *           (XOR will always have at least 2 inputs) has only 1 input
	 *           (usually an action gate) its type may be changed to any of
	 *           AND, OR, XOR, LATCH or even ARN and then merged.
	 *
	 *           Counting inputs is done in unsigned char 'fm' - number of
	 *           inputs is always less than or equal to 127 (PPGATESIZE)
	 *******************************************************************/
	for (sp = templist; sp && sp != tend; sp = sp->next) {	/* target gate */
	    for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {
		sp1 = lp1->le_sym;			/* input to this gate */
		assert(sp1);
		if (sp->type == XOR ||			/* XOR, AND, OR or ARN */
		    sp->type == AND ||
		    sp->type == OR  ||
		    sp->type == ARN ||			/* target can be any ftype */
		    sp->type == SW  ||			/* SW or CF can also have XOR AND OR as input */
		    sp->type == CF) {			/* so they add to ->fm&FU to inhibit merging */
		    if ( sp1->ftype == GATE ||
			(sp1->ftype == ARITH && lp1->le_val != (unsigned)-1) ) {
			if ((sp->type == XOR ||		/* XOR, AND, OR or ARN */
			     sp->type == AND ||
			     sp->type == OR  ||
			     sp->type == ARN)   &&	/* only if possible to merge */
			    (sp->fm & FMI) <= FI) {
			    sp->fm += FI;		/* count input in target temp gate up to 2 */
			}
			if ((sp1->type == XOR   ||	/* XOR, AND, OR, LATCH or ARN */
			     sp1->type == AND   ||
			     sp1->type == OR    ||
			     sp1->type == LATCH ||
			     sp1->type == ARN   ||
			     sp1->type == ARNF) &&	/* merge ARNF here */
			    sp1->u_blist != 0	&&
#if ! YYDEBUG || defined(SYUNION)
			    sp1->v_cnt <= 2	&&	/* test v_elist in union for any valid v_cnt */
#else
			    sp1->v_elist == 0	&&
#endif
			    (sp1->fm & FU) <= 1) {	/* not used more than once */
			    if ((sp1->fm++ & FU) == 0) {/* count use in merge candidate */
				mcnt++;			/* merge candidate found */
			    } else {
				ecnt++;			/* excess use count for merge candidate */
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
	 * Pass 2.2: merge AND, OR and XOR nodes which have ->fm&FU == 1 into
	 *           target plain or action gates which satisfy merge criteria.
	 * These are:
	 *           a) If target has only one input - can merge with XOR, AND, OR, LATCH
	 *              or ARN simply make type of target the type of single merging gate.
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
	if (mcnt > ecnt) {	/* there is a true merge candidate whose ->fm&FU is 1 */
	    for (sp = templist; sp && sp != tend; sp = sp->next) {	/* target gate */
		if (sp->type == XOR ||		/* XOR, AND, OR or ARN (SW or CF cannot be merged) */
		    sp->type == AND ||
		    sp->type == OR  ||
		    sp->type == ARN) {		/* only if possible to merge */
		    for (lpp = &sp->u_blist; (lp1 = *lpp) != 0; ) {
			sp1 = lp1->le_sym;		/* input to this XOR, AND or OR */
			assert(sp1);
			lval = lp1->le_val;
			typ = sp1->type;
			if ((typ == XOR   ||		/* XOR, AND, OR, LATCH or ARN */
			     typ == AND   ||
			     typ == OR    ||
			     typ == LATCH ||
			     typ == ARN   ||
			     typ == ARNF)             &&
			    lval != (unsigned)-1      &&/* not a delay for timer */
			    (lp2 = sp1->u_blist) != 0 &&/* save first link */
#if ! YYDEBUG || defined(SYUNION)
			    sp1->v_cnt <= 2           &&/* test v_elist in union for any valid v_cnt */
#else
			    sp1->v_elist == 0         &&
#endif
			    (sp1->fm & FU) == 1) {	/* can be merged */
			    assert(sp1->ftype == GATE || sp1->ftype == ARITH);
			    if ((sp->fm & FMI) == 0) {
				execerror("input count at temp gate is zero ???\n", NS, __FILE__, __LINE__);
			    }
			    if (lval == NOT && (typ == AND || typ == OR)) {
				typ ^= AND^OR;		/* transpose AND OR type */
			    }
			    if (sp->type == typ ||	/* can be merged if types are compatible */
				(sp->fm & FMI) == FI) {	/* or target has only one input */
				if (typ == ARN || typ == ARNF) {
				    if (lval == NOT) {
					goto noMerge;	/* cannot merge arithmetic into negated logic */
				    }
				    for (sp2 = templist; sp2; sp2 = sp2->next) {	/* scan whole templist */
					for (lp3 = sp2->u_blist; lp3; lp3 = lp3->le_next) {
					    if (lp3->le_sym == sp) {
						if (lp3->le_val == 0 && lp1->le_val == 0) {
						    assert(lp1->le_first == 0 || (lp1->le_first >= iCbuf && lp1->le_last < &iCbuf[IMMBUFSIZE]));
						    lp3->le_first = lp1->le_first;	/* merge ARN node */
						    lp3->le_last = lp1->le_last;
						    goto Merge;
						}
						goto noMerge;
					    }
					}
				    }
				    goto noMerge;	/* sp not found - must have been a head of a statement */
				}			/* never sees else */
				if (lval == NOT) {
				    op_not(lp1);	/* negate AND/OR - could be XOR */
				}
				/* now merge a gate */
			      Merge:
				sp->type = typ;		/* adjust if single input and transposed */
				*lpp = lp2;		/* first link of merge gate to target list */
				while (lp2->le_next) {
				    if ((sp->fm & FMI) <= FI) {
					sp->fm += FI;	/* count merged input in target temp gate up to 2 */
				    }
				    lp2 = lp2->le_next;	/* scan to end of merge list */
				}
				lp2->le_next = lp1->le_next;/* link rest of target list to merged tail */
				assert(sp1);
				if (templist == sp1) {	/* bypass sp1 on templist and delete left Link and Symbol */
				    templist = sp1->next;	/* unlink first object */
				    if (tend == sp1) {
					tend = sp1->next;
				    }
				}
				for (sp2 = templist; sp2; sp2 = sp2->next) {	/* scan templist again */
				    if (sp2->next == sp1) {
					sp2->next = sp1->next;	/* unlink sp1 from templist */
					if (tend == sp1) {
					    tend = sp1->next;
					}
					break;
				    }
				}
				if (sp2 == 0) {
				    execerror("merge temp not found ???\n", NS, __FILE__, __LINE__);
				}
#if YYDEBUG
				if ((iC_debug & 0402) == 0402) {
				    fprintf(iC_outFP, "cloneFunction: merge  %s  %s%s\n",
					sp->name, v(lp1));
				    fflush(iC_outFP);
				}
#endif
				lp1->le_next = freelist;	/* must retain link for repeats */
				freelist = lp1;		/* pop and free in Pass 8 */
				continue;		/* after merging continue scan with merged links */
			    }
			}
		      noMerge:
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
	 * Pass 2.3: restore ->fm&FU values 1 and 2 to zero
	 *******************************************************************/
	for (sp = templist; sp && sp != tend; sp = sp->next) {	/* target gate */
	    sp->fm = 0;					/* restore fm for temp gates */
	    for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {
		sp1 = lp1->le_sym;			/* input to this gate */
		if (mcnt &&		/* there is a merge candidate whose ->fm&FU must be cleared */
		    (sp->type == XOR ||			/* XOR, AND, OR or ARN */
		     sp->type == AND ||
		     sp->type == OR  ||
		     sp->type == ARN ||
		     sp->type == SW  ||			/* SW or CF can also have XOR AND OR as input */
		     sp->type == CF)) {			/* so they add to ->fm&FU to inhibit merging */
		    if (sp1->ftype == ARITH || sp1->ftype == GATE) {
			if ((sp1->type == XOR   ||	/* XOR, AND, OR, LATCH or ARN */
			     sp1->type == AND   ||
			     sp1->type == OR    ||
			     sp1->type == LATCH ||
			     sp1->type == ARN   ||
			     sp1->type == ARNF) &&
			    sp1->u_blist != 0) {
			    sp1->fm &= ~FU;		/* restore use count */
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
	sp1 = slp->le_sym;			/* expression head template */
	lp  = sp1->list;			/* link to real Symbol */
	typ = sp1->type;
	assert(lp);				/* internal Symbols linked in pass 1 */
	if ((sv.v = lp->le_sym) != functionHead) {
	    int		ioTyp;
	    /********************************************************************
	     * assign the expression associated with this assign para or internal statement
	     * an output can only be recognised by its name: Q[XBWL]%d
	     *******************************************************************/
	    sl.v = sv.v->u_blist;		/* link to expression head */
	    sv.f = sv.l = sl.f = sl.l = 0;	/* clear internal expression text */
	    if (typ == ARNC || typ == LOGC) {
		if (sp1->ftype == UDFA) {
		    assert(sv.v->list);
		    listGenOut(sv.v, sv.v->list->le_val & VAL_MASK, 0);/* listing of immC array variable */
		    for (lp1 = sv.v->list; lp1; lp1 = lp1->le_next) {
			assert(lp1->le_sym);
			if ((sp2 = lp1->le_sym)->type == UDF) {
			    sp2->type = typ;			/* type UDF for auto members in Pass1 */
			    listGenOut(lp1->le_sym, 0, 0);	/* listing of auto immC array member */
			}
		    }
		} else
		if (sp1->u_blist == 0) {
		    if ((sp1->em & EI) == 0) {
			listGenOut(sv.v, 1, 0);			/* listing of immC variable + possible real output */
		    } else
		    if (iC_iniList) {
			Valp	v;
			v = extractConstIni(sv.v);
#if YYDEBUG
			if ((iC_debug & 0402) == 0402) {
			    fprintf(iC_outFP, "cloneFunction: Pass 3: %s immC variable with initialiser %d extracted\n", sv.v->name, v.nuv);
			    fflush(iC_outFP);
			}
#endif
			listGenOut(sv.v, -1, &v);	/* listing of immC variable + initialiser */
		    }
		} else {
		    sv.v->type = iC_ctypes[sv.v->ftype]; /* TD check sp1->type == UDF */
		}
	    } else
	    if (iFunSymExt ||
		(sp1->fm & FM) == 0 ||
		(typ != SW && typ != CF)) {
		char *	name;
		sv.v->u_blist = 0;		/* restore for op_asgn */
		if (lp->le_val == NOT && sv.v->ftype == GATE) {
		    op_not(sl.v);		/* negated assign parameter */
		} else {
		    /********************************************************************
		     *  Remove arithmetic alias in a function block and assign real expression
		     *******************************************************************/
		    while ( sl.v &&
			    (sp2 = sl.v->le_sym) &&
			    sp2->type == ALIAS &&
			    (lp2 = sp2->u_blist) != NULL &&
			    (sp3 = lp2->le_sym) != NULL &&
			    (sp3->type == ARN || sp3->type == ARNF)
			) {
			sl.v = lp2;
			sl.f = sl.v->le_first;
			sl.l = sl.v->le_last;
		    }
		}
		if (sv.v->em & EO) {		/* QXx.y QBz etc */
		    name = sv.v->name;
		    assert(name[0] == 'Q');
		    switch (name[1]) {
		    case 'X':
			ioTyp = OUTX;
			break;
		    case 'B':
		    case 'W':
		    case 'L':
		    case 'H':
			ioTyp = OUTW;
			break;
		    default:
			assert(0);		/* illegal Q variable */
			break;
		    }
		} else {
		    ioTyp = 0;			/* flags that no I/O is generated */
		}
		assignExpression(&sv, &sl, ioTyp); /* assign to internal variable or parameter */
#if YYDEBUG
		if ((iC_debug & 0402) == 0402) pu(SYM, "clone", &sv);
#endif
	    } else {	/* iFunSymExt == 0 && (sp1->fm & FM) && (typ == SW || typ == CF) */
		int	cFn;
		cFn = sp1->u_blist->le_val >> FUN_OFFSET; /* function number */
		assert(cFn && cFn < functionUseSize);
		functionUse[0].c_cnt |= F_CALLED;	/* flag for copying temp file */
		functionUse[cFn].c_cnt++;	/* free this 'if' or 'switch' function for copying */
		sy_pop(lp);			/* free link before target is unlinked */
		sp1->list = 0;			/* free for next cloning */
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
     * immC Array member links are cleared in Pass 4.1 and 5.1. They are
     * restored from saved value if the member was also a parameter or
     * declared in Pass 4.2 or Pass 5.2.
     * Value parameter links are restored in Pass 4.2.
     * Assign parameter and Declaration links are restored in Pass 5.2.
     * (This new scheme makes sure link is not freed twice - JW 9-Apr-2007)
     * (Refined for arrays - makes sure link is not freed twice - JW 21-Aug-2012)
     *******************************************************************/
    /********************************************************************
     * Pass 4.2: Clean up parameter list
     *******************************************************************/
    lp = functionHead->list;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;
	if (sp->u_blist == 0 || (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC))) {
	    lp1 = sp->list;
	    while (lp1) {
		if ((sp->fm & FU) == 0 &&
		    (sp1 = lp1->le_sym) != 0 &&
		    (iC_Wflag & W_FUNCTION_PARAMETER)) {
		    warning("call parameter is never used:", sp1->name);
		}
		lp2 = lp1->le_next;
		sy_pop(lp1);			/* free link pushed in handleRealParameter */
		lp1 = lp2;			/* required if delay for a timer */
	    }
	    sp->list = 0;
	}
	lp = lp->le_next;
    }
    /********************************************************************
     * Pass 5.2: Clean up statement list
     *******************************************************************/
    slp = functionHead->u_blist;		/* statement list */
    if (instanceFlg) slp->le_val = instanceNum;	/* update function call instance number */
    while (slp) {
	sp = slp->le_sym;
	sp1 = ((lp1 = sp->list) != 0) ? lp1->le_sym : 0;
	if (sp1 != functionHead) {		/* bypass return link */
	    while (lp1) {			/* 0 if already sy_pop in parameter list */
		lp2 = lp1->le_next;
		sy_pop(lp1);			/* free link pushed in cloneFunction */
		lp1 = lp2;			/* or handleRealParameter if assign parameter */
	    }
	    sp->list = 0;
	}
	vlp = slp->le_next;			/* next varList link */
	sp = vlp->le_sym;			/* varList */
	while (sp) {
	    sp->list = 0;			/* clear internal Symbol pointers */
	    sp = sp->next;
	}
	slp = vlp->le_next;			/* next statement link */
    }
    /********************************************************************
     * Pass 5.1: Clean array members in statement list
     *******************************************************************/
    slp = functionHead->u_blist;		/* statement list */
    while (slp) {
	sp = slp->le_sym;
	if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
	    for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {	/* immC array - scan member list */
		assert(lp1->le_sym);
		if ((sp1 = lp1->le_sym)->list != 0) {
		    sy_pop(sp1->list);		/* free link pushed in cloneFunction */
		    sp1->list = 0;
		}
	    }
	}
	slp = slp->le_next->le_next;		/* next statement link */
    }
    /********************************************************************
     * Pass 4.1: Clean array members in parameter list
     *		 Done last in case member is also on statement list
     *******************************************************************/
    lp = functionHead->list;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;
	if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
	    for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {	/* immC array - scan member list */
		assert(lp1->le_sym);
		if ((sp1 = lp1->le_sym)->list != 0) {
		    sy_pop(sp1->list);		/* free link pushed in handleRealParameter */
		    sp1->list = 0;
		}
	    }
	}
	lp = lp->le_next;
    }
    /********************************************************************
     * Pass 6.1: Restore array members in parameter list
     *******************************************************************/
    cF = 0;					/* order must be the same as in pushFunCall() */
    lp = functionHead->list;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;
	if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
	    for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {	/* immC array - scan member list */
		sp1 = lp1->le_sym;
		assert(sp1->list == 0);
		sp1->list = iSav[cF++];		/* restore previous member template link */
	    }
	}
	lp = lp->le_next;
    }
    /********************************************************************
     * Pass 6.2: Restore parameter list
     *******************************************************************/
    lp = functionHead->list;			/* parameter list */
    while (lp) {
	sp = lp->le_sym;
	if (sp->u_blist == 0 || (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC))) {
	    assert(sp->list == 0);
	    sp->list = iSav[cF++];		/* restore previous value parameter template link */
	}
	lp = lp->le_next;
    }
    /********************************************************************
     * Pass 7.1: Restore array members in statement list
     *******************************************************************/
    slp = functionHead->u_blist;		/* statement list */
    while (slp) {
	sp = slp->le_sym;
	if (sp->ftype == UDFA && (sp->type == ARNC || sp->type == LOGC)) {
	    for (lp1 = sp->u_blist; lp1; lp1 = lp1->le_next) {	/* immC array - scan member list */
		sp1 = lp1->le_sym;
		assert(sp1->list == 0);
		sp1->list = iSav[cF++];		/* restore previous member template link */
	    }
	}
	slp = slp->le_next->le_next;		/* next statement link */
    }
    /********************************************************************
     * Pass 7.2: Restore statement list
     *******************************************************************/
    slp = functionHead->u_blist;		/* statement list */
    if (instanceFlg) slp->le_val = instanceNum;	/* update function call instance number */
    while (slp) {
	sp = slp->le_sym;
	sp1 = ((lp1 = sp->list) != 0) ? lp1->le_sym : 0;
	if (sp1 != functionHead) {		/* bypass return link - not cleared in Pass 5.2 */
	    assert(sp->list == 0);
	    sp->list = iSav[cF++];		/* restore declaration and assign link */
	}
	slp = slp->le_next->le_next;		/* next statement link */
    }
    assert(cF == saveCount);
    /********************************************************************
     * Pass 8: Free list of merged Symbols from Pass 2.2 after name was
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
    free(iSav);					/* free memory - no need for size */
    functionHead->fm = fm;			/* restore original functionHead->fm */
    /********************************************************************
     * adjust call string if ALIAS function return for arithmetic expansion
     * by trans-mogrifying function name
     *******************************************************************/
    if ((iC_optimise & 0x80) && (cp = strstr(rll.f, functionHead->name)) != NULL) {
	ep = cp + strlen(functionHead->name);
	while (cp < ep) {
	    *cp++ = '#';
	}
	assert(*cp == '(');
    }
    iC_optimise &= ~0x80;
    if ((iC_optimise & 02) && rsp && rsp->type == ARNF) {
	lp = rll.v;
	assert(lp && lp->le_first >= iCbuf && lp->le_first <= lp->le_last && lp->le_last < &iCbuf[IMMBUFSIZE]);
	len = lp->le_last - lp->le_first;
	strncpy(temp, lp->le_first, len);
	temp[len] = '\0';
	if ((lp1 = constExpr_push(temp, 1)) != NULL) {
	    cp = lp1->le_sym->name;
	    len1 = strlen(cp);
	    assert(len1 <= len);		/* TODO this may not be true if division is used */
	    cp = strncpy(lp->le_first, cp, len1) + len1;	/* replace expression by numeric value */
	    ep = cp + len;
	    while (cp < ep) {
		*cp++ = '#';
	    }
	    freeTemp(lp);			/* free previous link and temporary Symbol */
	    rll.v = lp1;
	}
    }
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "cloneFunction: end\n");
	fflush(iC_outFP);
    }
#endif
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
    rsp->fm |= sp->fm & (FA|FP|FU);		/* transfer parameter type and use count */
    rsp->em |= sp->em & AU;			/* transfer assign+use bits */
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
    assert(ssp);
    flp = ssp->u_blist;				/* link to formal expression */
    csp = *cspp;				/* cloned head of the expression */
    recursions++;
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "cloneList: expression  %s ==> %s	%-6s %2d call = %d recursions = %d\n",
	    ssp->name, csp->name, iC_full_type[csp->type], csp->fm&FU, call, recursions);
	fflush(iC_outFP);
    }
#endif
    while (flp) {
	fsp = flp->le_sym;
	assert(fsp);
	if (fsp->fm & FM) {			/* formal parameter or declared value */
	    if ((nlp = fsp->list) != 0 &&	/* link to real Symbol */
		(nsp = nlp->le_sym) != 0) {	/* real Symbol cloned before */
		if ((nsp->fm & FU) == 0 && (fsp->fm & FU) > 1) {
		    nsp->fm |= fsp->fm & FU;	/* transfer high use count for op_push'ed nodes */
		}
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
	    while (fsp->type == ALIAS && (nlp = fsp->list) != 0) {
		fsp = nlp->le_sym;		/* resolve ALIAS in function */
		assert(fsp);
	    }
	    if (fsp->name) {			/* global variable Symbol */
		nsp = fsp;			/* pointer to global variable */
	    } else {				/* internal Symbol ->name == 0, no '@' in Pass 1 */
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
	nsp->em |= fsp->em & AU;		/* transfer assign+use bits */
	clp = sy_push(nsp);			/* clone one list element */
	clp->le_val = flp->le_val ^ nval;	/* negation or arithmetic index in para */
	assert(first == 0 || (first >= iCbuf && last < &iCbuf[IMMBUFSIZE]));
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
	    assert(nlp->le_first == 0 || (nlp->le_first >= iCbuf && nlp->le_last < &iCbuf[IMMBUFSIZE]));
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
	if (csp && csp->type == ALIAS &&
	    ((csp->fm & (FA|FP)) != (FA|FP) ||	/* alias but not assign parameter*/
	    (ssp->list && ssp->list->le_sym == iCallHead))) {
	    assert((csp->em & EM) == 0 && (csp->fm & FM) == 0); /* not an external or function type */
	    rlp->le_sym->em |= csp->em & AU;	/* transfer assign+use bits */
	    *cspp = rlp->le_sym;		/* allow use of Symbol in optimisation */
	    assert(*cspp);
	    if ((iC_debug & 0402) == 0402) {
		fprintf(iC_outFP, "cloneList: checkAlias %d: %s %c%c=> %s\n",
		    call, csp->name, nlp->le_val ? '~' : '=', rlp->le_val ? '~' : '=', (*cspp)->name);
		fflush(iC_outFP);
	    }
	    csp = *cspp;
	    freeTemp(nlp);			/* free link and Symbol and name of Symbol if YYDEBUG */
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
    if (csp->type == ARNF && (iC_optimise & 0x82) == 0x02 && call == 1) {
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
	ptrdiff_t	offset;			/* handle difference of pointers portably in 32/64 bit systems */
	int		repeat;
	unsigned int	lval;			/* logical sign of a temp */
	static char	repeatMark[2] = { '\0', '\0' };

#if YYDEBUG
	if ((iC_debug & 0402) == 0402) {
	    fprintf(iC_outFP, "cloneList: ar optimisation  %s 	%-6s %2d call = %d\n",
		csp->name, iC_full_type[csp->type], csp->fm&FU, call);
	    fflush(iC_outFP);
	}
#endif
	assert(ttp >= iCbuf && ttp < &iCbuf[IMMBUFSIZE]);	/* ttp = iCstmtp if iC_optimise & 02 in Pass 2 */
	rlp->le_first = ttp;			/* first of this statement expression text */
	*ttp++ = '#';				/* room to parenthesise arithmetic function expression */
	index = 0;
	for (lpp = &csp->u_blist; (lp1 = *lpp) != 0; ) {
	    sp1 = lp1->le_sym;		/* input to this ARN */
	    assert(sp1);
#if YYDEBUG
	    if ((iC_debug & 0402) == 0402) {
		if (tfirst && lp1->le_first && lp1->le_last && tlast) {
		    int len2 = lp1->le_first - tfirst;
		    int len3 = lp1->le_last - lp1->le_first;
		    int len4 = tlast - lp1->le_last;
		    fprintf(iC_outFP, "cloneList: ar target  %s	%-6s %2d '%*.*s [%*.*s] %*.*s'\n",
			sp1->name, iC_full_type[sp1->type], sp1->fm&FU,
			len2, len2, tfirst,
			len3, len3, lp1->le_first,
			len4, len4, lp1->le_last);
		} else {
		    fprintf(iC_outFP, "cloneList: ar target  %s	%-6s %2d ' [] '\n",
			sp1->name, iC_full_type[sp1->type], sp1->fm&FU);
		}
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
			x = lval >> FUN_OFFSET;		/* function number */
			assert(x < functionUseSize);
			ccp = strncpy(cBuf, functionUse[x].c.expr, EBSIZE);	/* cloning expression */
#if YYDEBUG
			if ((iC_debug & 0402) == 0402)
			    fprintf(iC_outFP, "cloneList: functionUse[%d].c.epr = '%s'\n", x, ccp);
#endif
		    }
		    assert(index == (lval & VAL_MASK));		/* function offset */
		    caList[index].l = lp1;
#if YYDEBUG
		    if ((iC_debug & 0402) == 0402)
			fprintf(iC_outFP, "cloneList: caList[%d] .l->le_sym %s le_val = 0x%04x\n",
			    index, lp1->le_sym->name, lval);
#endif
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
			if (y == 0) break;		/* tail is finalised */
			ccp = ep;
			lp2 = caList[y].l;
			assert(lp2);
			sp2 = lp2->le_sym;
			assert(sp2);
			if ((((sp2->type == ARN || sp2->type == ARNF) && sp2->u_blist) ||
			    sp2->type == NCONST) &&
			    sp2->v_cnt == 1) {
			    *ttp++ = '(';		/* parameter expressions must be parenthesised */
			}
			/********************************************************************
			 * Either a ARN(F), INPW or SH.
			 * Some function variables have no le_first - le_last
			 * Other temp variables have a null name pointer - so have both.
			 * Arithmetic built in (SH is only candidate) is left as a null string
			 * (or temp name @n for logic generation output). The parameters are
			 * scanned from templist and their expansions appended to iCbuf rather
			 * than expanding the parameters of the built in as they are passed from
			 * the iC code in iCbuf.
			 *******************************************************************/
			len1 = 0;				/* in case temp variable has sp->name == 0 */
			if ((sp2->type == ARN   ||
			    sp2->type == ARNF   ||
			    sp2->type == INPW   ||
			    sp2->type == NCONST ||
			    sp2->u_blist == 0) &&		/* NCONST u_val union u_blist if SYUNION */
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
				    fprintf(iC_outFP, "cloneList: z1=%2d z2=%2d	%-15s %-6s v_cnt = %2d [%*.*s] '%s'\n",
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
			if (len1) {
			    strncpy(ttp, cp, len1);		/* copy the expression to end of iCbuf */
			} else {
			    len1 = 2;				/* for case where temporary sp2->name is NULL */
			    strcpy(ttp, "@0");			/* copy dummy expression to make it non-constant */
			}
			if (y == index && repeat == 0) {
			    offset = lp2->le_first ? ttp - lp2->le_first : 0;	/* avoid 32 bit int overflow in subtraction */
			    lp2->le_first = ttp;		/* adjust offset */
			    lp2->le_last = ttp + len1;
			    repeat++;
			}
			ttp += len1;
			if ((((sp2->type == ARN || sp2->type == ARNF) && sp2->u_blist) ||
			    sp2->type == NCONST) &&
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
		    lp1->le_val = 0;		/* action on lp1 TD */
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
		    fprintf(iC_outFP, "cloneList: merge arithmetic  %s  %s%s\n",
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
	*ttp++ = '#';				/* room to parenthesise arithmetic function expression */
	rlp->le_last = ttp;			/* last of this statement expression text */
    }	/* end of arithmetic optimisation */
#if YYDEBUG
    if ((iC_debug & 0402) == 0402) {
	fprintf(iC_outFP, "cloneList: expression end  %s ==> %s\n",
	    ssp->name, csp->name);
	fflush(iC_outFP);
    }
#endif
    return rlp;
} /* cloneList */

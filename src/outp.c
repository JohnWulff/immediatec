static const char outp_c[] =
"@(#)$Id: outp.c,v 1.67 2003/12/12 14:23:18 jw Exp $";
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
 *	outp.c
 *	parallel plc - output code or run machine
 *
 *******************************************************************/

/* J.E. Wulff	24-April-89 */

#ifdef _WINDOWS
#include <windows.h>
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<assert.h>
#include	<errno.h>
#include	"icg.h"
#include	"icc.h"
#include	"comp.h"

extern const char	SC_ID[];

/********************************************************************
 *
 *	IEC1131() modifies IEC-1131 bit names in the buffer buf
 *	The routine returns a count of fields found.
 *	If count is less the 4 the unmodified name is returned in buf
 *	The count can be used to characterize IBx, QBx etc fields also
 *	All fields are a null string ("") or 0 if not modified.
 *	Return:	number of fields modified.
 *	Fields:	iqt[]	single character I, Q or T in first position.
 *		bwx[]	single character B, W or X in second position.
 *		*bytep	int value of numerical field starting at pos. 3
 *		*bitp	int value of numerical field folowing '.'
 *		tail[]	any extra characters folowing numerical field.
 *		buf[]	modified string if "X" type with '.' number
 *			else unmodified name (or if outFlag == 0).
 *		eg: IX123.7_5 returns 5, "I" in iqt, "X" in bwx,
 *			    123 in bytep, 7 in bitp, "_5" in tail and
 *			    "IX123_7_5" in buf.
 *
 *	Also converts plain numbers to numbers preceded by an underscore.
 *	Stores the numerical value via parameter bytep.
 *	Returned count is 1 for a plain integer and 2 for an integer
 *	followed by extra characters. These are returned in tail.
 *		eg: 123_5  returns 2, 123 in bytep, "_5" in tail and
 *			   "_123_5" in buf.
 *	(done first, so that the parameter values returned are correct)
 *
 *	bufLen should be >= 23 (sscanf format length)
 *
 *******************************************************************/

int
IEC1131(char * name, char * buf, int bufLen,
	char * iqt, char * bwx, int * bytep, int * bitp, char * tail)
{
    int count;

    assert(bufLen >= 23);
    iqt[0] = bwx[0] = tail[0] = *bytep = *bitp = count = 0;	/* clear for later check */
    if (outFlag && (count = sscanf(name, "%d%7s", bytep, tail)) >= 1) {
	snprintf(buf, bufLen-1, "_%d%s", *bytep, tail);
    } else if (outFlag &&
	(count = sscanf(name, "%1[IQT]%1[BWX]%5d.%5d%7s",
			    iqt, bwx, bytep, bitp, tail)) >= 4 &&
	*bwx == 'X') {
	snprintf(buf, bufLen-1, "%s%s%d_%d%s",
			    iqt, bwx, *bytep, *bitp, tail);
    } else {
    //##tail NOT USED##	outFlag && (count = sscanf(name, "%1[IQT]%1[BWX]%5d%7s", iqt, bwx, bytep, tail));
	strncpy(buf, name, bufLen-1);
    }
    return count;
} /* IEC1131 */

/********************************************************************
 *
 *	toIEC1131() changes modified names back to IEC-1131 bit names
 *
 *	bufLen should be >= 24 (sscanf format length)
 *
 *******************************************************************/

int
toIEC1131(char * name, char * buf, int bufLen,
	char * iqt, char * bwx, int * bytep, int * bitp, char * tail)
{
    int count;

    assert(bufLen >= 24);
    iqt[0] = bwx[0] = tail[0] = *bytep = *bitp = count = 0;	/* clear for later check */
    if (outFlag && (count = sscanf(name, "_%d%7s", bytep, tail)) == 1) {
	snprintf(buf, bufLen-1, "%d", *bytep);
    } else if (outFlag &&
	(count = sscanf(name, "%1[IQT]%1[BWX]%5d_%5d%7s",
			    iqt, bwx, bytep, bitp, tail)) >= 4 &&
	*bwx == 'X') {
	snprintf(buf, bufLen-1, "%s%s%d.%d%s",
			    iqt, bwx, *bytep, *bitp, tail);
    } else {
	strncpy(buf, name, bufLen-1);
    }
    return count;
} /* toIEC1131 */

/********************************************************************
 *
 *	This routine mN() modifies IEC-1131 bit names and stores the
 *	modified string in alternate arrays of names[].
 *
 *	As a side-effect this routine provides the 1st letter of the
 *	modified name in iqt[], the 2nd letter in bwx[] and the 1st
 *	and possible the 2nd number in byte and bit respectively.
 *
 *	If name is not modified, name is copied to names and iqt[0]
 *	is cleared.
 *
 *	Because names[] has 3 static arrays, used in rotating-pong fashion,
 *	a pointer to the current names entry can still be used after a 2nd
 *	and a 3rd call to mN(). Used in loops to provide previous name.
 *
 *******************************************************************/

#define SZ	4		/* one more than 3 for bad surprises */
static char	names[SZ][BUFS];/* ping pong buffer for modified names */
static int	ix;		/* index for accessing alternate arrays */
static char	iqt[2];		/* char buffers - space for 0 terminator */
static char	bwx[2];
static int	byte;
static int	bit;
static char	tail[8];	/* compiler generated suffix _123456 max */
static unsigned	cnt;		/* used as side-effect in OUTW - INPX */

static char *
mN(Symbol * sp)
{
    char * np = names[ix++];			/* alternate ix = 0, 1 or 2 */
    if (ix >= SZ) ix = 0;			/* rotate buffers */
    if (sp == 0) return strncpy(np, "0", 2);	/* in case of suprises */
    cnt = IEC1131(sp->name, np, BUFS, iqt, bwx, &byte, &bit, tail);
    return np;
} /* mN */

unsigned short	Aflag;			/* -A flag signals ARITH alias */
unsigned short	aflag;			/* -a on compile to append output */
unsigned short	Tflag;			/* define _tVar in _list_.c _list1.h */

static unsigned	block_total;		/* shared by listNet and buildNet */
static unsigned	link_count;		/* shared by listNet and buildNet */
static int	extFlag;		/* set if extern has been recognised */

/********************************************************************
 *
 *	errorEmit outputs a warning line in the generated code
 *	and an error message in the listing
 *
 *******************************************************************/

static char	errorBuf[256];	/* used for error lines in emitting code */

static void
errorEmit(FILE* Fp, char* errorMsg, unsigned* lcp)
{
    fprintf(Fp, "#warning \"iC: %s\"\n", errorMsg);
    (*lcp)++;		/* count lines actually output */
    errmess("ErrorEmit", errorMsg, NS);	/* error sets iClock->type to ERR */
} /* errorEmit */

/********************************************************************
 *
 *	Gather information on total memory required for generation
 *	including link count and gate count for each type of Gate.
 *	This must be done after the iC and C compile phase but before
 *	buildNet() or output().
 *
 *	if (debug & 020) list the generated network
 *	if (debug & 040) list network statistics
 *	output warnings and error statistics - terminate if fatal
 *
 *******************************************************************/

int
listNet(unsigned * gate_count)
{
    Symbol *	sp;
    List_e *	lp;
    Symbol **	hsp;
    short	dc;
    short	typ;
    long	byte_total;

    byte_total = link_count = block_total = 0;	/* init each time */
    for (typ = 0; typ < MAX_LS; typ++) {
	gate_count[typ] = 0;
    }
    if (debug & 020) {
	/* do not change spelling - used in 'pplstfix' */
	fprintf(outFP, "\n******* NET TOPOLOGY    ************************\n\n");
    }
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if (sp->type & ~TM) {
		extFlag = 1;
	    }
	    if ((typ = sp->type & TM) < MAX_LS) {
		gate_count[typ]++;
		if (typ < MAX_OP) {
		    block_total++;
		}
		if (typ < MAX_LV &&	/* don't count outputs */
		    sp->ftype != OUTX && sp->ftype != OUTW) {
		    for (lp = sp->list; lp; lp = lp->le_next) {
			if (sp->ftype >= MAX_AR || lp->le_val != (unsigned) -1) {
			    link_count++;
			}
		    }
		    link_count++;	/* for terminator */
		    if (sp->ftype >= MAX_AR) {
			link_count++;	/* 2nd terminator for inverted */
		    }			/* or time for TIMER action */
		}
		if (debug & 020) {		/* print directed graph */
		    fprintf(outFP, "%s\t%c  %c", sp->name,
			os[typ], fos[sp->ftype]);
		    dc = 0;
		    for (lp = sp->list; lp; lp = lp->le_next) {
			Symbol * tsp = lp->le_sym;
			if (dc++ >= 8) {
			    dc = 1;
			    fprintf(outFP, "\n\t");
			}
			if (sp->ftype == F_SW || sp->ftype == F_CF || sp->ftype == F_CE) {
			    /* case number of "if" or "switch" C fragment */
			    fprintf(outFP, "\t%c (%d)",
				os[types[sp->ftype]], lp->le_val);
			} else if (sp->ftype == TIMR && lp->le_val > 0) {
			     /* timer preset off value */
			    fprintf(outFP, "\t %s%c (%d)",
				tsp->name, os[tsp->type & TM], lp->le_val);
			} else if (sp->ftype < MAX_AR && lp->le_val == (unsigned) -1) {
			    /* reference to a timer value - no link */
			    fprintf(outFP, "\t<%s%c", tsp->name, os[tsp->type & TM]);
			} else {
			    fprintf(outFP, "\t%c%s%c",
				(sp->ftype == GATE || sp->ftype == OUTX) &&
				lp->le_val ? '~' : ' ',
				tsp->name, os[tsp->type & TM]);
			}
		    }
		    fprintf(outFP, "\n");
		}
		if (typ == UDF) {
		    warning("undefined gate:", sp->name);
		} else if (typ == ERR) {
		    ierror("gate:", sp->name);
		}
	    }
	}
    }

    if (debug & 040) {
	byte_total = (long)block_total * sizeof(Gate)
		   + (long)link_count * sizeof(Gate *);
	/* do not change spelling - used in 'pplstfix' */
	fprintf(outFP, "\n******* NET STATISTICS  ************************\n\n");
	for (typ = 0; typ < MAX_LS; typ++) {
	    if (gate_count[typ]) {
		fprintf(outFP, "%s\t%c %6d%s\n", full_type[typ], os[typ],
		    gate_count[typ], typ < MAX_OP ? " blocks" : "");
	    }
	}
	fprintf(outFP, "\nTOTAL\t%8u blocks\n", block_total);
	fprintf(outFP, "\t%8u links\n", link_count);
	fprintf(outFP, "\t%8ld bytes\n", byte_total);
    }
    if (debug & 076) fprintf(outFP, "\ncompiled by:\n%s\n", SC_ID);
    if (gate_count[UDF]) {
	char undBuf[32];
	snprintf(undBuf, 32, "%d undefined gate%s",
	    gate_count[UDF],
	    gate_count[UDF] > 1 ? "s" : "");
	warning(undBuf, NS);
    }
    if (ynerrs || gate_count[ERR]) {
	char synBuf[16];
	char genBuf[16];
	char errBuf[64];
	snprintf(synBuf, 16, "%d syntax", ynerrs);
	snprintf(genBuf, 16, "%d generate", gate_count[ERR]);
	snprintf(errBuf, 64, "%s%s%s error%s - cannot execute",
	    ynerrs			?  synBuf	: "",
	    ynerrs && gate_count[ERR]	? " and "	: "",
	    gate_count[ERR]		?  genBuf	: "",
	    ynerrs + gate_count[ERR] >1	? "s"		: "");
	ierror(errBuf, NS);
	return 1;
    }
    return 0;
} 							/* listNet */

/********************************************************************
 *
 *	generate execution network
 *
 *******************************************************************/

int
buildNet(Gate ** igpp)
{
    Symbol *	sp;
    List_e *	lp;
    Symbol **	hsp;
    short	typ;
    unsigned	val;
    unsigned	rc = 0;		/* return code */
    char	bw[2];		/* "B" or "W" */
    Gate *	gp;
    Gate **	fp;
    Gate **	ifp;

    if (extFlag) {
	fprintf(errFP, "extern declarations used - cannot execute\n");
	return 1;	/* syntax or generate error */
    }			/* no need to mask ->type & TM in buildNet() */

    /* initialise executable gates */

    *igpp = gp = (Gate *)calloc(block_total, sizeof(Gate));	/* gates */
    ifp = fp = (Gate **)calloc(link_count, sizeof(Gate *));	/* links */

    for (typ = 0; typ < MAX_OP; typ++) {
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if (sp->type == typ) {
		    gp->gt_fni = sp->ftype;	/* basic gate first */
		    gp->gt_ids = sp->name;	/* gate to symbol name */
		    sp->u.gate = gp++;		/* symbol to gate */
		}
	    }
	}
    }
    if ((val = gp - *igpp) == block_total) {
	gp = *igpp;				/* repeat to initialise links */
	for (typ = 0; typ <= MAX_OP; typ++) {	/* include ALIAS */
	    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
		for (sp = *hsp; sp; sp = sp->next) {
		    if (sp->type == typ) {
			if (typ < MAX_LV) {
			    gp->gt_list = fp;	/* start of gate list */
			    if (sp->ftype < MIN_ACT) {
				val = (sp->ftype < MAX_AR) ? NOT : 0;
				do {	/* go through list twice for LVs */
				    for (lp = sp->list; lp; lp = lp->le_next) {
					if (sp->ftype < MAX_AR) {
					    if (lp->le_val == (unsigned) -1) {
						continue; /* timer value link */
					    }
					    lp->le_sym->u.gate->gt_rlist =
					    (Gate**)lp->le_val;
					} else if (val != lp->le_val) {
					    continue;	/* not right value */
					}
					*fp++ = lp->le_sym->u.gate;
				    }
				    *fp++ = 0;		/* gate list terminator */
				} while (val ^= NOT);

				if (typ == INPW) {
				    if (sscanf(gp->gt_ids, "I%1[BW]%d",
					bw, &byte) == 2 && byte < IXD) {
					if (bw[0] == 'B') {
					    IB_[byte] = gp;
					} else {
					    IW_[byte] = gp;
					}
				    } else {
					goto inErr;
				    }
				} else if (typ == INPX) {
				    if (sscanf(sp->name, "IX%d.%d",
					&byte, &bit) == 2 &&
					byte < IXD && bit < 8) {
					IX_[byte * 8 + bit] = gp;
				    } else if (sscanf(sp->name, "TX%d.%d",
					&byte, &bit) == 2 &&
					byte < TXD && bit < 8) {
					TX_[byte * 8 + bit] = gp;
				    } else {
				    inErr:
		    ierror("INPUT byte or bit address exceeds limit:", sp->name);
				    }
				}
			    } else if (sp->ftype < MAX_ACT) {
				Symbol * tsp;	/* D_SH - F_CF */
				/* relies on action gates */
				/* having only one output */
				lp = sp->list;
				if ((tsp = lp->le_sym) != 0) {
				    if (sp->ftype == TIMR &&
					tsp->u.gate->gt_old < lp->le_val) {
					/* transfer timer preset off value */
					tsp->u.gate->gt_old = lp->le_val;
				    }
				    *fp++ = tsp->u.gate;
				} else {
				    /* F_SW, F_CF or F_CE action gate points to function */
				    *fp++ = (Gate*)lp->le_val;
				}
				/* room for clock or timer entry */
				*fp++ = 0;
				/* room for time value */
				*fp++ = 0;
				/* ZZZ modify later to do this only for */
				/* ZZZ action gates controlled by a TIMER */
			    } else if (sp->ftype == OUTW) {
				if (sscanf(gp->gt_ids, "Q%1[BW]%d", bw, &byte) == 2 &&
				    byte < IXD) {
				    gp->gt_list = (Gate**)byte;
				    gp->gt_mark = bw[0] == 'B' ? B_WIDTH : W_WIDTH;
				    QT_[byte] = bw[0];	/* 'B' or 'W' */
				} else {
				    goto outErr;
				}
			    } else if (sp->ftype == OUTX) {
				if (sscanf(gp->gt_ids, "QX%d.%d", &byte, &bit) == 2 &&
				    byte < IXD && bit < 8) {
				    gp->gt_list = (Gate**)byte;
				    gp->gt_mark = bitMask[bit];
				    QT_[byte] = 'X';
				} else {
				outErr:
		    ierror("OUTPUT byte or bit address exceeds limit:", gp->gt_ids);
				    gp->gt_list = (Gate**)0;
				    gp->gt_mark = 0;	/* no change in bits */
				}
			    } else {
		    ierror("OUTPUT strange ftype:", gp->gt_ids);
			    }
			    gp++;
			} else if (typ < MAX_OP) {
			    /* CLK || TIM list headers - convert to ftype */
			    gp->gt_fni = typ == CLK ? CLCKL
				       : typ == TIM ? TIMRL : 0;
			    /*
			     * this initialisation of clock references relies
			     * on gates which execute a function ftype != GATE
			     * having only one output which is in the first
			     * location of the function list (FL_GATE). The
			     * clock reference is put in the second location
			     * which was previously cleared by a terminator.
			     * The 3rd location holds a pointer to a Gate of
			     * ftype ARITH holding a time value (ARN or NCONST).
			     * All action gates were initialised first, because
			     * typ < MAX_LV were done first.
			     */

			    for (lp = sp->list; lp; lp = lp->le_next) {
				lp->le_sym->u.gate->gt_clk = gp;
			    }
			    gp++;
			}
		    }
		}
	    }
	}
	if (iclock->type == ERR) {
	    gp++;			/* error - count iClock */
	    rc = 1;			/* generate error */
	}
	if ((val = gp - *igpp) == block_total) {
	    gp = *igpp;			/* repeat to initialise timer links */
	    for (typ = 0; typ < MAX_OP; typ++) {	/* keep gp in same order */
		for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
		    for (sp = *hsp; sp; sp = sp->next) {
			if (sp->type == typ) {
			    if (sp->ftype < MAX_AR) {
				/*
				 * The 3rd location holds a pointer to a Gate of
				 * ftype ARITH holding a time value (ARN or NCONST).
				 */
				for (lp = sp->list; lp; lp = lp->le_next) {
				    if (lp->le_val == (unsigned) -1) {
					lp->le_sym->u.gate->gt_time = gp;
				    }
				}
			    }
			    gp++;
			}
		    }
		}
	    }
	    if (iclock->type == ERR) {
		gp++;			/* error - count iClock */
		rc = 1;			/* generate error */
	    }
	}
    }

    if ((val = gp - *igpp) != block_total) {
	fprintf(errFP,		/* either first or second scan above */
	    "\n%s: line %d: block error %d vs block_total %d\n",
	    __FILE__, __LINE__, val, block_total);
	rc = 2;
    } else if ((val = fp - ifp) != link_count) {
	fprintf(errFP,
	    "\n%s: line %d: link error %d vs link_count %d\n",
	    __FILE__, __LINE__, val, link_count);
	rc = 3;
    }

    return rc;		/* return code */
} /* buildNet */

/********************************************************************
 *
 *	generate network as C file
 *
 *******************************************************************/

int
output(char * outfile)
{
    Symbol *	sp;
    Symbol *	gp;
    List_e *	lp;
    Symbol **	hsp;
    short	dc;
    short	typ;
    unsigned	val;
    unsigned	rc = 0;		/* return code */
    unsigned	mask;
    Symbol *	tsp;
    List_e *	tlp;
    List_e **	lpp;
    char *	modName;
    char *	nxs;
    char *	sam;
    int		li;		/* link index into connection list */
    int		lc;		/* link count in connection list */
    FILE *	Fp;		/* C output file */
    FILE *	H1p;		/* list _list_.c + header _list1.h */
    FILE *	H2p;		/* list header _list2.h */
    char *	s1;
    char *	module;
    unsigned	linecnt = 1;
    int		aliasArithFlag = 0;

    /* open output file */

    if ((Fp = fopen(outfile, "w")) == 0) {
	rc = Oindex; goto end;
    }

    if ((H2p = fopen(H2name, aflag ? "a" : "w")) == 0) {	/* list header _list2.h */
	rc = H2index; goto endl;
    }

    if (aflag == 0) {
	fprintf(H2p, "#define	I_LIST\\\n");		/* list header _list2.h */

	/* write _list_.c once, so that it is locally present */
	if ((H1p = fopen(Cname, "w")) == 0) {		/* list _list_.c */
	    rc = Cindex; goto endh;
	}
	fprintf(H1p, "\
#include	<icg.h>\n\
#include	\"%s\"\n\
#include	\"%s\"\n\
Gate **		i_list[] = { I_LIST 0, };\n\
", H1name, H2name);					/* list _list_.c */
	if (Tflag) {			/* imm_identifier++ -- used in c_compile */
	    fprintf(H1p, "int		_tVar;\n");
	}
	fclose(H1p);					/* list _list_.c */
    }

    if ((H1p = fopen(H1name, aflag ? "a" : "w")) == 0) {	/* header _list1.h */
	rc = H1index; goto endh;
    }

    fprintf(Fp, "\
/********************************************************************\n\
 *\n\
 *	SOURCE:   %s\n\
 *	OUTPUT:   %s\n\
 *\n\
 *******************************************************************/\n\
\n\
static char	COMPILER[] =\n\
\"%s\";\n\
\n\
#include	<icg.h>\n\
#include	\"%s\"\n\
\n\
#ifdef ALIAS_ARITH\n\
#define _AV(x) (x.gt_ini==-ALIAS ? ((Gate*)x.gt_rlist)->gt_new : x.gt_new)\n\
#define _LV(x) (x.gt_ini==-ALIAS ? (((Gate*)x.gt_rlist)->gt_val < 0 ? 1 : 0)\\\n\
				 : (x.gt_val < 0 ? 1 : 0))\n\
#define _AA(x,v) aAssign(x.gt_ini==-ALIAS ? (Gate*)x.gt_rlist : &x, v)\n\
#define _LA(x,v) lAssign(x.gt_ini==-ALIAS ? (Gate*)x.gt_rlist : &x, v)\n\
#else\n\
#define _AV(x) x.gt_new\n\
#define _LV(x) (x.gt_val < 0 ? 1 : 0)\n\
#define _AA(x,v) aAssign(&x, v)\n\
#define _LA(x,v) lAssign(&x, v)\n\
#endif\n\
extern Gate *	_l_[];\n\
", inpNM, outfile, SC_ID, H1name);
    linecnt += 26;

/********************************************************************
 *
 *	reverse action links to input links for simple Gates
 *
 *******************************************************************/

    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((typ = sp->type) == UDF || typ & ~TM) {
		fprintf(Fp, "extern Gate	%s;\n", mN(sp));
		linecnt++;
	    }
	    if ((typ &= TM) < MAX_LV) {
		if ((dc = sp->ftype) == ARITH || dc == GATE) {
		    for (lpp = &sp->list; (lp = *lpp) != 0; ) {
			/* leave out timing controls */
			if (lp->le_val != (unsigned) -1) {
			    tsp = lp->le_sym;	/* reverse action links */
			    tlp = tsp->u.blist;
			    tsp->u.blist = lp;	/* to input links */
			    *lpp = lp->le_next;
			    lp->le_sym = sp;
			    lp->le_next = tlp;	/* lpp is not changed */
			} else {
			    lpp = &lp->le_next;	/* lpp to next link */
			}
		    }
		} else if (dc == TIMR &&
		    (lp = sp->list) != 0 &&
		    (tsp = lp->le_sym) != 0 &&
		    tsp->u.val < lp->le_val) {
		    tsp->u.val = lp->le_val;	/* store timer preset off value */
		}				/* temporarily in u (which is 0) */
		if (dc == OUTW && (lp = sp->list) != 0) {
		    assert(0);			/* #define no longer in use */
		}
	    } else if (typ < MAX_OP) {

/********************************************************************
 *
 *	this initialisation of clock references relies on gates which
 *	execute a function ftype != GATE having only one output which
 *	is in the first location of the function list (FL_GATE).
 *	The clock reference is put in the second location which was
 *	previously cleared by a terminator. The 3rd location holds a
 *	pointer to a Gate of ftype ARITH holding a time value (ARN or NCONST).
 *	During execution of an action this pointer is used to find the
 *	correct clock block, which is used as the head of a clock list
 *	to which the action is linked. The actual clock block is made
 *	empty in the following, and action blocks are linked to it
 *	dynamically at run time.
 *
 *******************************************************************/

		for (lp = sp->list; lp; lp = sp->list) {
		    tsp = lp->le_sym;		/* action gate */
		    if (tsp->ftype == GATE) {
			snprintf(errorBuf, sizeof errorBuf,
			    "Simple gate '%s' on clock list '%s'",
			    tsp->name, sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
			break;
		    }
		    if ((tlp = tsp->list) == 0) {
			snprintf(errorBuf, sizeof errorBuf,
			    "Action gate '%s' on clock list '%s' has no output",
			    tsp->name, sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
			break;
		    }
		    if (tlp->le_next) {
			snprintf(errorBuf, sizeof errorBuf,
			    "Action gate '%s' on clock list '%s' has more than 1 output",
			    tsp->name, sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
			break;
		    }
		    tlp->le_next = lp;
		    sp->list = lp->le_next;
		    lp->le_sym = sp;
		    lp->le_next = 0;
		}
	    } else if (typ == ALIAS) {
		if ((lp = sp->list) == 0) {
		    snprintf(errorBuf, sizeof errorBuf,
			"Alias '%s' has no output",
			sp->name);
		    errorEmit(Fp, errorBuf, &linecnt);
		} else if (lp->le_next) {
		    snprintf(errorBuf, sizeof errorBuf,
			"Alias '%s' has more than 1 output",
			sp->name);
		    errorEmit(Fp, errorBuf, &linecnt);
		} else if (sp->ftype == ARITH) {
		    if (aliasArithFlag == 0) {
			fprintf(H1p, "#define ALIAS_ARITH\n");	/* header _list1.h */
			aliasArithFlag = 1;
		    }
		} else if (sp->ftype != GATE && sp->ftype != CLCKL && sp->ftype != TIMRL) {
		    snprintf(errorBuf, sizeof errorBuf,
			"Alias '%s' has wrong ftype %s",
			sp->name, full_ftype[sp->ftype]);
		    errorEmit(Fp, errorBuf, &linecnt);
		}
	    }
	}
    }

    /* do the timing controls last, to link them after their timer clock */
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((sp->type & TM) < MAX_LV && sp->ftype < MAX_AR) {
		for (lp = sp->list; lp; ) {
		    if (lp->le_val == (unsigned) -1) {
			tsp = lp->le_sym;		/* action gate */
			if (tsp->ftype == GATE) {
			    snprintf(errorBuf, sizeof errorBuf,
				"Simple gate '%s' controlled by timer '%s'",
				tsp->name, sp->name);
			    errorEmit(Fp, errorBuf, &linecnt);
			    break;
			}
			if ((tlp = tsp->list) == 0) {
			    snprintf(errorBuf, sizeof errorBuf,
				"Action gate '%s' controlled by timer '%s' has no output",
				tsp->name, sp->name);
			    errorEmit(Fp, errorBuf, &linecnt);
			    break;
			}
			if ((tlp = tlp->le_next) == 0) {
			    snprintf(errorBuf, sizeof errorBuf,
				"Action gate '%s' controlled by timer '%s' has no clock",
				tsp->name, sp->name);
			    errorEmit(Fp, errorBuf, &linecnt);
			    break;
			}
			if (tlp->le_next) {
			    snprintf(errorBuf, sizeof errorBuf,
				"Action gate '%s' controlled by timer '%s' has more than output and clock",
				tsp->name, sp->name);
			    errorEmit(Fp, errorBuf, &linecnt);
			    break;
			}
			tlp->le_next = lp;
			sp->list = lp->le_next;
			lp->le_sym = sp;
			lp->le_next = 0;
			lp = sp->list;
		    } else {
			lp = lp->le_next;
		    }
		}
	    }
	}
    }

/********************************************************************
 *
 *	output executable gates
 *
 *	this modifies the symbol table entries
 *	the CLK and TIM list Symbols are reconnected
 *	to the action Gates which they control
 *
 *******************************************************************/

    fprintf(Fp, "\n\
/********************************************************************\n\
 *\n\
 *	Gate list\n\
 *\n\
 *******************************************************************/\n\
\n");
    linecnt += 7;

    li = 0;
    nxs = "0";			/* 0 terminator for linked gate list */
    sam = "";			/* no & for terminator in linked Gate list */
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((typ = sp->type) > UDF && typ < MAX_OP && /* leave out EXT_TYPES */
		strcmp(sp->name, "iClock") != 0) {
		mask = 0;
		modName = mN(sp);	/* modified string, byte and bit */
		if (typ >= AND && typ < MAX_GT) {	/* AND OR LATCH */
		    if (typ == OR) {
			dc = 1;				/* typ == OR */
		    } else {
			dc = 0;				/* typ == AND LATCH */
			for (lp = sp->u.blist; lp; lp = lp->le_next) {
			    dc++;			/* space in input list */
			}
			if (typ == LATCH) {
			    dc++;
			    dc >>= 1;			/* halve for LATCH */
			}
		    }
		    fprintf(Fp, "Gate %-8s = { %d, %d,", modName, dc, dc);
		} else {
		    if (typ == NCONST) {
			/* NCONST Gate must be static because same constant */
			/* may be used in several linked modules - not extern */
			fprintf(Fp, "static Gate %-7s", modName);
		    } else {
			fprintf(Fp, "Gate %-8s", modName);
		    }
		    fprintf(Fp, " = { 1, -%s,", full_type[typ]);
		}
		fprintf(Fp, " %s, 0, \"%s\",",
		    full_ftype[dc = sp->ftype], sp->name);
		if (dc >= MIN_ACT && dc < MAX_ACT) {
		    /* gt_list */
		    fprintf(Fp, " &_l_[%d],", li);
		    li += 2;	/* space for action or function pointer + clock */
		    if ((lp = sp->list->le_next) != 0 &&
			(lp->le_sym->type & TM) == TIM) {
			li++;		/* space for pointer to delay time Gate */
		    }
		} else if (dc == OUTW) {
		    if (iqt[0] == 'Q' &&	/* QB0_0 is cnt == 3 (no tail) */
			bwx[0] != 'X' &&	/* can only be B or W */
			cnt == 3 && byte < IXD) {
			fprintf(Fp, " (Gate**)%d,", byte);
			mask = bwx[0] == 'B' ? B_WIDTH : W_WIDTH;
		    } else {
			goto OutErr;
		    }
		} else if (dc == OUTX) {
		    if (iqt[0] == 'Q' &&
			bwx[0] == 'X' &&	/* QX0.0_0 is cnt == 5 */
			cnt == 5 && byte < IXD && bit < 8) {
			/* OUTPUT byte part of bit pointer */
			fprintf(Fp, " (Gate**)%d,", byte);
			mask = bitMask[bit];
		    } else {
		    OutErr:
			fprintf(Fp, " (Gate**)0,\n");
			linecnt++;
			snprintf(errorBuf, sizeof errorBuf,
			    "OUTPUT byte or bit address exceeds limit: %s",
			    sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
			mask = 0;	/* no output because mask is 0x00 */
		    }
		} else {
		    fprintf(Fp, " 0,");		/* no gt_list */
		    if (dc == TIMRL) {
			if (sp->u.val > 0) {
			    mask = sp->u.val;
			    sp->u.val = 0;	/* restore temporary u to 0 */
			}
		    }
		}
		if (typ == ARN || (typ >= AND && typ < MAX_GT)) {
		    fprintf(Fp, " &_l_[%d],", li);	/* gt_rlist */
		    for (lp = sp->u.blist; lp; lp = lp->le_next) {
			li++;	/* space in input list */
		    }
		    /* space for dual GATE list or ARITH with FUNCTION */
		    li += 2;
		} else if (typ == INPW) {
		    if (iqt[0] == 'I' &&	/* IB0 is cnt == 3 */
			bwx[0] != 'X' &&	/* can only be B or W */
			cnt == 3 && byte < IXD) {
			fprintf(Fp, " &I%s_[%d],", bwx, byte);
		    } else {
			goto InErr;
		    }
		} else if (typ == INPX) {
		    if (iqt[0] != 'Q' &&	/* can only be I or T */
			bwx[0] == 'X' &&	/* IX0.0 is cnt == 4 */
			cnt == 4 && byte < IXD && bit < 8) {
			fprintf(Fp, " &%sX_[%d],", iqt, byte * 8 + bit);
		    } else {
		    InErr:
			fprintf(Fp, " 0,\n");
			linecnt++;
			snprintf(errorBuf, sizeof errorBuf,
			    "INPUT byte or bit address exceeds limit: %s",
			    sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
		    }
		} else {
		    fprintf(Fp, " 0,");		/* no gt_rlist */
		}
		fprintf(Fp, " %s%s", sam, nxs);
		if (mask != 0) {
		    fprintf(Fp, ", %d", mask);	/* bitMask for OUT */
		}
		fprintf(Fp, " };\n");
		linecnt++;
		nxs = modName;		/* previous Symbol name */
		sam = "&";
	    }
	}
    }

/********************************************************************
 *
 *	Do ALIASes last to avoid forward references of Gates in gt_list
 *	Resolve multiple ALIASes here for the same reason.
 *
 *******************************************************************/

    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((typ = sp->type) == ALIAS && sp->list != 0 &&
		((dc = sp->ftype) == GATE ||
		(Aflag && (dc == ARITH || dc == CLCKL || dc == TIMRL)))) {
		modName = mN(sp);	/* modified string, byte and bit */
		fprintf(Fp, "Gate %-8s", modName);	/* ZZZZ */
		val = sp->list->le_val;
		gp = sp->list->le_sym;
		while (gp->type == ALIAS) {
		    val ^= gp->list->le_val;		/* negate if necessary */
		    gp = gp->list->le_sym;		/* point to original */
		}
		fprintf(Fp,
		    " = { 1, -%s, %s, 0, \"%s\", 0, (Gate**)&%s, %s%s, %d };\n",
		    full_type[typ], full_ftype[dc], sp->name, mN(gp), sam, nxs, val);
		linecnt++;
		nxs = modName;		/* previous Symbol name */
		sam = "&";
	    }
	}
    }

/********************************************************************
 *
 *	generate linked list header, for linking several independently
 *	compiled modules
 *
 *	The following algorithm fails if two files are linked with names
 *	'ab.cd.ic' and 'ab_cd.ic' - by Goedel there is hardly a way out of this.
 *	A multiple define error occurs for the name 'ab_cd_i_list' at link time.
 *	The same error occurs if unfortunate path combinations are used
 *	eg: ab/cd.ic and either of the above
 *
 *******************************************************************/

    module = emalloc(strlen(inpNM)+1);	/* +1 for '\0' */
    strcpy(module, inpNM);		/* source file name */
    if ((s1 = strrchr(module, '.')) != 0) {
	*s1 = '\0';			/* module name without extensions */
    }
    while ((s1 = strchr(module, '/')) != 0) {
	*s1 = '_';			/*  replace all '/'s with '_' */
    }
    while ((s1 = strchr(module, '\\')) != 0) {
	*s1 = '_';			/*  replace all '\'s with '_' */
    }
    while ((s1 = strchr(module, '.')) != 0) {
	*s1 = '_';			/*  replace further '.'s with '_' */
    }
    fprintf(Fp, "\nGate *		%s_i_list = %s%s;\n", module, sam, nxs);
    linecnt += 2;
    fprintf(H1p, "extern Gate *	%s_i_list;\n", module);	/* header _list1.h */
    if (Tflag) {			/* imm_identifier++ -- used in c_compile */
	fprintf(H1p, "extern int	_tVar;\n");
    }
    fprintf(H2p, "	&%s_i_list,\\\n", module);	/* list header _list2.h */
    free(module);

    fprintf(Fp, "\n\
/********************************************************************\n\
 *\n\
 *	Literal blocks and embedded C fragment functions\n\
 *\n\
 *******************************************************************/\n\
\n");
    linecnt += 7;

    /* copy C intermediate file up to EOF to C output file */
    /* translate any imm variables and ALIAS references of type 'QB1_0' */

    if ((rc = copyXlate(Fp, outfile, &linecnt, 01)) != 0) { /* copy literal blocks */
	goto endm;
    }
    if ((rc = copyXlate(Fp, outfile, &linecnt, 02)) != 0) { /* copy functions */
	goto endm;
    }

/********************************************************************
 *
 *	produce initialised connection lists
 *	using modified symbol table
 *
 *******************************************************************/

    if (li == 0) goto endm;	/* MS-C does not hack 0 length array - gcc does */
    fprintf(Fp, "\n\
/********************************************************************\n\
 *\n\
 *	Connection lists\n\
 *\n\
 *******************************************************************/\n\
\n\
static Gate *	_l_[] = {\n");
    linecnt += 8;

    lc = 0;			/* count links */
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((typ = sp->type) > UDF && typ < MAX_GT) { /* leave aout EXT_TYPES */
		int		len = 16;
		char *	fs = strlen(sp->name) > 1 ? "\t" : "\t\t";

		fprintf(Fp, "/* %s */", sp->name);
		if ((dc = sp->ftype) >= MIN_ACT && dc < MAX_ACT) {
		    if ((lp = sp->list) == 0) {
			snprintf(errorBuf, sizeof errorBuf,
			    "Action gate '%s' has no action list",
			    sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
		    } else {
			if (lp->le_sym == 0) {	/* dc == F_SW, F_CF or F_CE */
			    /* Function Pointer for "if" or "switch" */
			    len += 17;	/* assume len of %d is 2 */
			    fprintf(Fp, "%s(Gate*)_c_%d,",
				fs, lp->le_val);
			} else {
			    len += strlen((tsp = lp->le_sym)->name) + 3;
			    fprintf(Fp, "%s&%s,",
				fs, mN(tsp));		/* action */
			}
			lc++;
			fs = " ";
			if ((lp = lp->le_next) == 0) {
			    len += 3;		/* 0 filler - no clock or timer */
			    fprintf(Fp, "%s0,", fs);
			    snprintf(errorBuf, sizeof errorBuf,
				"Action gate '%s' has no clock or timer",
				sp->name);
			    errorEmit(Fp, errorBuf, &linecnt);
			} else {
			    len += strlen((tsp = lp->le_sym)->name) + 3;
			    fprintf(Fp, "%s&%s,",
				fs, mN(tsp));		/* clock or timer */
			    if ((tsp->type & TM) == TIM) {
				if ((lp = lp->le_next) != 0) {
				    len += strlen((tsp = lp->le_sym)->name) + 3;
				    fprintf(Fp, "%s&%s,", fs, mN(tsp));	/* delay time */
				} else {
				    len += 3;		/* 0 filler - no delay time */
				    fprintf(Fp, "%s0,", fs);
				    snprintf(errorBuf, sizeof errorBuf,
					"Action gate '%s' has timer '%s' with no delay",
					sp->name, tsp->name );
				    errorEmit(Fp, errorBuf, &linecnt);
				}
				lc++;
			    }
			}
			lc++;
			fs = "\t";
			len += 8; len &= 0xfff8;
		    }
		}

		val = 0;
		if (typ == ARN) {
		    if ((lp = sp->u.blist) == 0) {
			snprintf(errorBuf, sizeof errorBuf,
			    "Arithmetic gate '%s' has no function",
			    sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
		    } else {
			/* Function Pointer at start of input list */
			len += 17;	/* assume len of %d is 2 */
			if (lp->le_val) {
			    fprintf(Fp, "%s(Gate*)_c_%d,", fs, lp->le_val);
			} else {
			    fprintf(Fp, "%s(Gate*)0,", fs);	/* OUTW */
			}
			lc++;
			fs = " ";
			val = NOT;	/* force single input list */
		    }
		    goto n1;
		}
		if (typ >= AND && typ < MAX_GT) {
		n1:
		    do {
			for (lp = sp->u.blist; lp; lp = lp->le_next) {
			    if (lp->le_val == val || typ == ARN) {
				len += strlen(lp->le_sym->name) + 3;
				if (len > 73) {
				    fs = "\n\t\t";
				    linecnt++;
				    len = 16 + strlen(lp->le_sym->name) + 3;
				}
				fprintf(Fp, "%s&%s,", fs, mN(lp->le_sym));
				lc++;
				fs = " ";
			    }
			}
			fprintf(Fp, "%s0,", fs);
			lc++;
			len += 3;
			fs = " ";
		    } while (val ^= NOT);
		}
		fprintf(Fp, "\n");
		linecnt++;
	    }
	}
    }
    fprintf(Fp, "};\n");
    linecnt++;
    if (li != lc) {
	snprintf(errorBuf, sizeof errorBuf,
	    "Link count %d from gate list is not %d in connection list",
	    li, lc);
	errorEmit(Fp, errorBuf, &linecnt);
    }
endm:
    if (debug & 040) {
	fprintf(outFP, "\nC OUTPUT: %s  (%d lines)\n", outfile, linecnt-1);
    }
    fclose(H1p);	/* close the header file in case other actions */
endh:
    fclose(H2p);	/* close the list header file in case other actions */
endl:
    fclose(Fp);	/* close the C output file in case other actions */
end:
    return rc;		/* return code */
} /* output */

/********************************************************************
 *
 *	copy C intermediate file up to EOF to C output file.
 *	Translate any ALIAS references as part of C compile phase.
 *
 *	mode 01         Copy only literal blocks %{ ... %}
 *	mode 02 default Copy only C blocks, functions or cases
 *	mode 03         Copy literal blocks and C blocks
 *
 * The characters %# occurring at the start of a line in a literal or C
 * block will be converted to a plain #. This allows the use of
 * C-preprocessor statements in literal or C blocks which will be
 * resolved after the iC compilation. They must be written as
 * %#include
 * %#define
 * %#ifdef etc
 *
 * lines starting with %## will be replaced by #line nn "fn"
 * where nn is the following line number and fn the name of the iC file
 * NOTE: this line must be shorter than BUFS (currently 128).
 *       since %## is generated in the earlier code this should be OK
 * NOTE: lineBuf[] must be large enough to hold a complete
 *       pre-processor line for the following sscanf()s
 *
 *******************************************************************/

static int
copyBlocks(FILE * iFP, FILE * oFP, int mode)
{
    int		c;
    int		mask = 02;	/* default is functions or cases */
    int		lf = 0;		/* set by first non white space in a line */
    int		fd;
    char *	lp;
    char	lstBuf[BUFS];	/* include file name */
    char	lineBuf[BUFS];	/* can be smaller than a line */

    /* rewind intermediate file */
    if (fseek(iFP, 0L, SEEK_SET) != 0) {
	return T1index;
    }

    while (fgets(lineBuf, sizeof lineBuf, iFP)) {
	if (strcmp(lineBuf, "%{\n") == 0) {
	    mask = 01;				/* copy literal blocks */
	} else if (strcmp(lineBuf, "%}\n") == 0) {
	    mask = 02;				/* copy functions or cases */
	} else if (mode & mask) {
	    for (lp = lineBuf; (c = *lp++) != 0; ) {
		if (lf || c != '%' || *lp != '#') {	/* converts %# to # */
		    putc(c, oFP);
		    if (c == '\n') {
			lf = 0;			/* start of a new line */
		    } else if (c != ' ' && c != '\t') {
			lf = 1;			/* not white space */
		    }
		} else
		/********************************************************
		 *  handle pre-processor #include <stdio.h> or "icc.h"
		 ********************************************************/
		if (sscanf(lp, " # include %[<\"/A-Za-z_.0-9>]", lstBuf) == 1) {
		    if (T4FP == NULL) {
			if ((fd = mkstemp(T4FN)) < 0 || (T4FP = fdopen(fd, "w+")) == 0) {
			    ierror("copyBlocks: cannot open:", T4FN);
			    return T4index;			/* error opening temporary file */
			}
			if ((fd = mkstemp(T5FN)) < 0 || close(fd) < 0 || unlink(T5FN) < 0) {
			    ierror("copyBlocks: cannot make or unlink:", T5FN);
			    perror("unlink");
			    return T5index;			/* error unlinking temporary file */
			}
			if (debug & 02) fprintf(outFP, "####### ready to c_parse includes via %s %s\n", T4FN, T5FN);
		    }
		    if (debug & 02) fprintf(outFP, "####### c_parse #include %s\n", lstBuf);
		    fprintf(T4FP, "#include %s\n", lstBuf);	/* a little C file !!! */
		}
	    }
	}
    }
    return 0;
} /* copyBlocks */

/********************************************************************
 *
 *	To handle immediate variables in C code, the output of the first
 *	two passes of copyBlocks is seperated by a line containing
 *	## in C comment delimiters
 *	and output to T2FP.
 *
 *******************************************************************/

extern FILE* yyin;
extern FILE* yyout;

int
c_compile(FILE * iFP)
{
    int		mask = 01;			/* copy literal blocks */
    int		r;
    char	lineBuf[BUFS];	/* can be smaller than a line */

//fprintf(stderr, "c_compile: start\n"); fflush(stderr);
    lexflag = C_PARSE|C_FIRST|C_BLOCK;		/* output partial source listing */

    if (copyBlocks(iFP, T2FP, 01)) {
	return T1index;
    }
    if (outFlag == 0) {
	fprintf(T2FP, "/*##*/int c_exec(int pp_index) { switch (pp_index) {\n");
    }
    if (copyBlocks(iFP, T2FP, 02)) {
	return T1index;
    }
    if (outFlag == 0) {
	fprintf(T2FP, "/*##*/}}\n");
    }

    /********************************************************
     *  handle pre-processor #include <stdio.h> or "icc.h"
     ********************************************************/
    if (T4FP) {
	fflush(T4FP);
	/* Cygnus does not understand cc - use gcc */
	sprintf(lineBuf, "gcc -E -x c %s -o %s", T4FN, T5FN);
	if (debug & 02) fprintf(outFP, "####### pre-compile: %s\n", lineBuf);
	r = system(lineBuf);			/* Pre-compile C file */
	if (debug & 02) fprintf(outFP, "####### pre-compile: return %d\n", r);

	if (r != 0 || (T5FP = fopen(T5FN, "r")) == NULL) {
	    ierror("c_compile: cannot open:", T5FN);
	    perror("open");
	    return T5index;
	}
	if (debug & 02) fprintf(outFP, "####### compile include files via %s %s\n", T4FN, T5FN);
	yyin = T5FP;			/* lexc reads from include now */
	yyout = outFP;			/* list file */
	lexflag |= C_PARSE|C_NO_COUNT|C_FIRST;	/* do not count characters for include files */
	gramOffset = lineno = 0;
	if (c_parse() != 0) {
	    ierror("c_compile: Parse error in includes\n", T5FN);
	}
	lexflag &= ~C_NO_COUNT;		/* count characters again */
    }
//fprintf(stderr, "c_compile: start compile\n"); fflush(stderr);
    /* rewind intermediate file */
    if (fseek(T2FP, 0L, SEEK_SET) != 0) {
	return T2index;
    }

    yyin = T2FP;
    yyout = outFP;			/* list file */
    copyAdjust(NULL, T3FP);		/* initialize lineEntryArray */
    gramOffset = lineno = 0;
    if (c_parse() != 0) {
	ierror("c_compile: Parse error\n", NULL);
    }
//fprintf(stderr, "c_compile: end compile\n"); fflush(stderr);
    rewind(yyin);
    copyAdjust(yyin, T3FP);		/* output adjusted C-code */
//fprintf(stderr, "c_compile end\n"); fflush(stderr);
    return 0;
} /* c_compile */

/********************************************************************
 *
 *	Then this code is parsed by the C compiler and afterwards split on
 *	## in comments and output to oFP
 *
 *******************************************************************/

int
copyXlate(FILE * oFP, char * outfile, unsigned * lcp, int mode)
{
    int		mask = 01;			/* copy literal blocks */
    char	lineBuf[BUFS];	/* can be smaller than a line */

    /* rewind intermediate file */
    if (fseek(T3FP, 0L, SEEK_SET) != 0) {
	return T3index;
    }

    while (fgets(lineBuf, sizeof lineBuf, T3FP)) {
	if (strncmp(lineBuf, "/*##*/", 6) == 0) {
	    mask = 02;				/* copy functions or cases */
	} else if (mode & mask) {
	    (*lcp)++;				/* count lines actually output */
	    if (strcmp(lineBuf, "##\n") == 0) {
		fprintf(oFP, "#line %d \"%s\"\n", *lcp, outfile);
	    } else {
		fputs(lineBuf, oFP);
	    }
	}
    }
    return 0;
} /* copyXlate */

static const char outp_c[] =
"@(#)$Id: outp.c,v 1.78 2005/01/28 15:10:26 jw Exp $";
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
#include	<unistd.h>
#include	<string.h>
#include	<ctype.h>
#include	<assert.h>
#include	<errno.h>
#include	"icc.h"
#include	"comp.h"

extern const char	iC_ID[];

static char *		iC_ext_type[]  = { iC_EXT_TYPE };
static char *		iC_ext_ftype[] = { iC_EXT_FTYPE };

/********************************************************************
 *
 *	IEC1131() modifies IEC-1131 bit names in the buffer buf
 *	The routine returns a count of fields found.
 *	If count is less the 4 the unmodified name is returned in buf
 *	The count can be used to characterize IBx, QBx etc fields also
 *	All fields are a null string ("") or 0 if not modified.
 *	Return:	number of fields modified.
 *	Fields:	iqt[]	single character 'I', 'Q' or 'T' in first position.
 *		xbwl[]	single character 'X', 'B', 'W' or 'L' in second position.
 *		*bytep	int value of numerical field starting at pos. 3
 *		*bitp	int value of numerical field folowing '.'
 *		tail[]	any extra characters folowing numerical field.
 *		buf[]	modified string if "X" type with '.' number
 *			else unmodified name (or if outFlag == 0).
 *		eg: IX123.7_5 returns 5, "I" in iqt, "X" in xbwl,
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
	char * iqt, char * xbwl, int * bytep, int * bitp, char * tail)
{
    int count;

    assert(bufLen >= 23);
    iqt[0] = xbwl[0] = tail[0] = *bytep = *bitp = count = 0;	/* clear for later check */
    if (outFlag && (count = sscanf(name, "%d%7s", bytep, tail)) >= 1) {
	snprintf(buf, bufLen-1, "_%d%s", *bytep, tail);
    } else if (outFlag &&
	(count = sscanf(name, "%1[IQT]%1[XBWL]%5d.%5d%7s",
			    iqt, xbwl, bytep, bitp, tail)) >= 4 &&
	*xbwl == 'X') {
	snprintf(buf, bufLen-1, "%s%s%d_%d%s",
			    iqt, xbwl, *bytep, *bitp, tail);
    } else {
    //##tail NOT USED##	outFlag && (count = sscanf(name, "%1[IQT]%1[XBWL]%5d%7s", iqt, xbwl, bytep, tail));
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
	char * iqt, char * xbwl, int * bytep, int * bitp, char * tail)
{
    int count;

    assert(bufLen >= 24);
    iqt[0] = xbwl[0] = tail[0] = *bytep = *bitp = count = 0;	/* clear for later check */
    if (outFlag && (count = sscanf(name, "_%d%7s", bytep, tail)) == 1) {
	snprintf(buf, bufLen-1, "%d", *bytep);
    } else if (outFlag &&
	(count = sscanf(name, "%1[IQT]%1[XBWL]%5d_%5d%7s",
			    iqt, xbwl, bytep, bitp, tail)) >= 4 &&
	*xbwl == 'X') {
	snprintf(buf, bufLen-1, "%s%s%d.%d%s",
			    iqt, xbwl, *bytep, *bitp, tail);
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
 *	modified name in iqt[], the 2nd letter in xbwl[] and the 1st
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

#define SZ	4				/* one more than 3 for bad surprises */
static char	names[SZ][BUFS];		/* ping pong buffer for modified names */
static int	ix;				/* index for accessing alternate arrays */
static char	iqt[2];				/* char buffers - space for 0 terminator */
static char	xbwl[2];
static int	byte;
static int	bit;
static char	tail[8];			/* compiler generated suffix _123456 max */
static unsigned	cnt;				/* used as side-effect in OUTW - INPX */

static char *
mN(Symbol * sp)
{
    char * np = names[ix++];			/* alternate ix = 0, 1 or 2 */
    if (ix >= SZ) ix = 0;			/* rotate buffers */
    if (sp == 0) return strncpy(np, "0", 2);	/* in case of suprises */
    cnt = IEC1131(sp->name, np, BUFS, iqt, xbwl, &byte, &bit, tail);
    return np;
} /* mN */

unsigned short	iC_Aflag;			/* -A flag signals ARITH alias */
unsigned short	iC_aflag;			/* -a on compile to append output */
unsigned short	iC_lflag;			/* -a on compile to append output */
unsigned short	iC_Tflag;			/* define iC_tVar */

static unsigned	block_total;			/* shared by listNet and buildNet */
static unsigned	link_count;			/* shared by listNet and buildNet */
static int	extFlag;			/* set if extern has been recognised */

/********************************************************************
 *
 *	errorEmit outputs a warning line in the generated code
 *	and an error message in the listing
 *
 *******************************************************************/

static char	errorBuf[256];			/* used for error lines in emitting code */

static void
errorEmit(FILE* Fp, char* errorMsg, unsigned* lcp)
{
    fprintf(Fp, "#warning \"iC: %s\"\n", errorMsg);
    (*lcp)++;					/* count lines actually output */
    errmess("ErrorEmit", errorMsg, NS);		/* error sets iClock->type to ERR */
} /* errorEmit */

/********************************************************************
 *
 *	Gather information on total memory required for generation
 *	including link count and gate count for each type of Gate.
 *	This must be done after the iC and C compile phase but before
 *	buildNet() or output().
 *
 *	if (iC_debug & 020) list the generated network
 *	if (iC_debug & 010) list network statistics
 *	output warnings and error statistics - terminate if fatal
 *
 *******************************************************************/

int
listNet(unsigned gate_count[])
{
    Symbol *	sp;
    Symbol *	tsp;
    List_e *	lp;
    Symbol **	hsp;
    short	iC_dc;
    short	typ;
    int		undefined;
    long	byte_total;

    byte_total = link_count = block_total = undefined = 0;	/* init each time */
    for (typ = 0; typ < MAX_LS; typ++) {
	gate_count[typ] = 0;
    }
#if YYDEBUG	/* ############################################### */
    if ((iC_debug & 04002) == 04002) {
	int hspn = 0;
	int hspf = 0;
	fprintf(iC_outFP, "******* SYMBOL TABLE    ************************\n");
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if (hspf == 0) {
		    fprintf(iC_outFP, "%3d:", hspn);
		    hspf = 1;
		}
		if (sp && sp->name) {
		    fprintf(iC_outFP, "\n\t%-20s %-6s %-2s %-2s %-6s",
			sp->name,
			iC_full_type[sp->type & TM],
			sp->type & EM ? "EM" : "",
			sp->type & FM ? "FM" : "",
			iC_full_ftype[sp->ftype]);
		    if ((sp->type & ~FM) == ALIAS) {
			Symbol * sp1 = sp;
			List_e * lp1;
			Symbol * sp2;
			while (sp1 &&
			    ((sp1->type == ALIAS && (lp1 = sp1->list) != 0) ||
			    (sp1->type == (ALIAS|FM) && (lp1 = sp1->u_blist) != 0)) &&
			    (sp2 = lp1->le_sym) != 0) {
			    fprintf(iC_outFP, " %s ", sp2->name);
			    sp1 = sp2;
			}
		    }
		} else {
		    fprintf(iC_outFP, "\n\t*** BAD Symbol");
		}
	    }
	    hspn++;
	    if (hspf != 0) {
		fprintf(iC_outFP, "\n");
		hspf = 0;
	    }
	}
    }
#endif		/* ############################################### */
    if (iC_debug & 020) {
	/* do not change spelling - used in 'pplstfix' */
	fprintf(iC_outFP, "\n******* NET TOPOLOGY    ************************\n\n");
    }
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    sp->v_cnt = 0;			/* v_elist & v_glist no longer needed */
	}
    }
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if (sp->type & EM) {
		extFlag = 1;
	    }
	    if ((typ = sp->type & ~EM) < MAX_LS) {
		gate_count[typ]++;
		if (typ < MAX_OP) {
		    block_total++;
		}
		if (typ < MAX_LV &&		/* don't count outputs */
		    sp->ftype != OUTX && sp->ftype != OUTW) {
		    for (lp = sp->list; lp; lp = lp->le_next) {
			if (lp->le_val != (unsigned) -1) {
			    link_count++;
			    if (sp->ftype < MAX_AR && lp->le_val != 0) {
				tsp = lp->le_sym;	/* arithmetic function */
				assert(tsp && (tsp->type == ARN || tsp->type == ERR));
				tsp->v_cnt++;		/* count reverse parameter */
			    }
			}
		    }
		    link_count++;		/* for terminator */
		    if (sp->ftype >= MAX_AR) {
			link_count++;		/* 2nd terminator for inverted */
		    }				/* or time for TIMER action */
		}
		if (iC_debug & 020) {		/* print directed graph */
		    fprintf(iC_outFP, "%s\t%c  %c", sp->name,
			iC_os[typ], iC_fos[sp->ftype]);
		    iC_dc = 0;
		    for (lp = sp->list; lp; lp = lp->le_next) {
			tsp = lp->le_sym;
			if (iC_dc++ >= 8) {
			    iC_dc = 1;
			    fprintf(iC_outFP, "\n\t");
			}
			if (sp->ftype == F_SW || sp->ftype == F_CF || sp->ftype == F_CE) {
			    /* case number of "if" or "switch" C fragment */
			    if (lp->le_val >= 0x100) {
				fprintf(iC_outFP, "\t%c (%d)",
				    iC_os[iC_types[sp->ftype]], lp->le_val >> 8);
			    }
			} else if (sp->ftype == TIMR && lp->le_val > 0) {
			     /* timer preset off value */
			    fprintf(iC_outFP, "\t %s%c (%d)",
				tsp->name, iC_os[tsp->type & TM], lp->le_val);
			} else if (sp->ftype < MAX_AR && lp->le_val == (unsigned) -1) {
			    /* reference to a timer value - no link */
			    fprintf(iC_outFP, "\t<%s%c", tsp->name, iC_os[tsp->type & TM]);
			} else {
			    fprintf(iC_outFP, "\t%c%s%c",
				(sp->ftype == GATE || sp->ftype == OUTX) &&
				lp->le_val ? '~' : ' ',
				tsp->name, iC_os[tsp->type & TM]);
			}
		    }
		    fprintf(iC_outFP, "\n");
		}
		if (typ == UDF) {
		    warning("undefined gate:", sp->name);
		} else if (typ == ERR) {
		    ierror("gate:", sp->name);
		}
	    } else if (typ == (UDF|FM)) {
		undefined++;	/* cannot execute if function internal gate not defined */
		ierror("undefined gate in function:", sp->name);
	    }
	}
    }
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if (sp->v_cnt) {
		assert(sp->type == ARN || sp->type == ERR);
		link_count += sp->v_cnt + 1;	/* space for reverse links + function # */
	    }
	    sp->v_cnt = 0;
	}
    }

    if (iC_debug & 010) {
	byte_total = (long)block_total * sizeof(Gate)
		   + (long)link_count * sizeof(Gate *);
	/* do not change spelling - used in 'pplstfix' */
	fprintf(iC_outFP, "\n******* NET STATISTICS  ************************\n\n");
	for (typ = 0; typ < MAX_LS; typ++) {
	    if (gate_count[typ]) {
		fprintf(iC_outFP, "%s\t%c %6d%s\n", iC_full_type[typ], iC_os[typ],
		    gate_count[typ], typ < MAX_OP ? " blocks" : "");
	    }
	}
	fprintf(iC_outFP, "\nTOTAL\t%8u blocks\n", block_total);
	fprintf(iC_outFP, "\t%8u links\n", link_count);
	fprintf(iC_outFP, "\t%8ld bytes\n", byte_total);
    }
    if (iC_debug & 076) fprintf(iC_outFP, "\ncompiled by:\n%s\n", iC_ID);
    if ((undefined += gate_count[UDF]) > 0) {
	char undBuf[32];
	snprintf(undBuf, 32, "%d undefined gate%s",
	    undefined,
	    undefined > 1 ? "s" : "");
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
} /* listNet */
#if defined(RUN) || defined(TCP) && ! defined(LOAD)

/********************************************************************
 *
 *	Generate execution network for icr or ict direct execution
 *	igpp ==> array of Gates generated with calloc(block_total)
 *	gate_count splits this array into different type Gates in icc()
 *
 *******************************************************************/

int
buildNet(Gate ** igpp, unsigned gate_count[])
{
    Symbol *	sp;
    Symbol *	tsp;
    List_e *	lp;
    Symbol **	hsp;
    short	typ;
    unsigned	val;
    unsigned	i;
    unsigned	rc = 0;				/* return code */
    char	bwl[2];				/* "B" or "W" */
    Gate *	gp;
    Gate *	tgp;
    Gate **	fp;
    Gate **	ifp;

    if (extFlag) {
	fprintf(iC_errFP, "extern declarations used - cannot execute\n");
	return 1;				/* syntax or generate error */
    }						/* no need to mask ->type & TM in buildNet() */

    /* initialise executable gates */

    *igpp = gp = (Gate *)calloc(block_total, sizeof(Gate));	/* gates */
    ifp = fp = (Gate **)calloc(link_count, sizeof(Gate *));	/* links */

    for (typ = 0; typ < MAX_OP; typ++) {
	val = 0;
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if (sp->type == typ) {
		    gp->gt_fni = sp->ftype;	/* basic gate first */
		    gp->gt_ids = sp->name;	/* gate to symbol name */
		    sp->u_gate = gp++;		/* symbol to gate */
		    val++;
		}
	    }
	}
	assert(val == gate_count[typ]);		/* check once only */
    }
    if ((gp - *igpp) == block_total) {
	gp = *igpp;				/* repeat to initialise links */
	for (typ = 0; typ <= MAX_OP; typ++) {	/* include ALIAS */
	    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
		for (sp = *hsp; sp; sp = sp->next) {
		    if (sp->type == typ) {
			if (typ < MAX_LV) {
			    gp->gt_list = fp;	/* start of gate list */
			    if (sp->ftype < MIN_ACT) {
				val = (sp->ftype < MAX_AR) ? NOT : 0;
				do {		/* go through list twice for LVs */
				    for (lp = sp->list; lp; lp = lp->le_next) {
					if (sp->ftype < MAX_AR) {
					    if (lp->le_val == (unsigned) -1) {
						continue; /* timer value link */
					    }
					    tsp = lp->le_sym;
					    assert(tsp && tsp->u_gate);
					    tgp = tsp->u_gate;
					    if (lp->le_val != 0) {
						assert(tsp->type == ARN || tsp->type == ERR);
						tgp->gt_new++;	/* count ARITH inputs */
					    }
					} else if (val != lp->le_val) {
					    continue;	/* not right logical value */
					}
					*fp++ = lp->le_sym->u_gate;
				    }
				    *fp++ = 0;		/* gate list terminator */
				} while (val ^= NOT);

				if (typ == INPW) {
				    if (sscanf(gp->gt_ids, "I%1[BWL]%d",
					bwl, &byte) == 2 && byte < IXD) {
					if (bwl[0] == 'B') {
					    iC_IB_[byte] = gp;
					} else if (bwl[0] == 'W') {
					    iC_IW_[byte] = gp;
					} else if (bwl[0] == 'L') {
#if INT_MAX != 32767 || defined (LONG16)
					    iC_IL_[byte] = gp;
#else
		    ierror("32 bit INPUT not supported in 16 bit environment:", sp->name);
#endif
					} else {
					    assert(0);	/* must be 'B', 'W' or 'L' */
					}
				    } else {
					goto inErr;
				    }
				} else if (typ == INPX) {
				    if (sscanf(sp->name, "IX%d.%d",
					&byte, &bit) == 2 &&
					byte < IXD && bit < 8) {
					iC_IX_[byte * 8 + bit] = gp;
				    } else if (sscanf(sp->name, "TX%d.%d",
					&byte, &bit) == 2 &&
					byte < TXD && bit < 8) {
					iC_TX_[byte * 8 + bit] = gp;
				    } else {
				    inErr:
		    ierror("INPUT byte or bit address exceeds limit:", sp->name);
				    }
				}
			    } else if (sp->ftype < MAX_ACT) {
				/* D_SH - TIMR relies on action gates having only one output */
				lp = sp->list;
				if ((tsp = lp->le_sym) != 0) {
				    tgp = tsp->u_gate;
				    assert(tgp);
				    if (sp->ftype == TIMR &&
					tgp->gt_old < lp->le_val) {
					/* transfer timer preset off value */
					tgp->gt_old = lp->le_val;
				    }
				    *fp++ = tgp;
				    *fp++ = 0;		/* room for clock or timer entry */
				    *fp++ = 0;		/* room for time delay */
				    /* ZZZ modify later to do this only for */
				    /* ZZZ action gates controlled by a TIMER */
				} else {
				    /* F_SW, F_CF or F_CE action gate points to function */
				    assert((sp->ftype == F_SW ||
					    sp->ftype == F_CF ||
					    sp->ftype == F_CE) &&
					   (lp->le_val & 0xff) == 0);
				    fp[0] = (Gate*)(lp->le_val >> 8);
				    fp[1] = 0;	/* room for clock or timer entry */
				    fp[2] = 0;	/* room for time delay or first parameter */
				    i   = 3;	/* offset for above */
				    val = 1;
				    while ((lp = lp->le_next) != 0) {
					assert(val < lp->le_val);
					val = lp->le_val;
					tsp = lp->le_sym;
					assert(tsp && tsp->u_gate);
					fp[val] = tsp->u_gate;
					i++;	/* count parameters */
				    }
				    fp += i;	/* space for above entries */
				}
			    } else if (sp->ftype == OUTW) {
				if (sscanf(gp->gt_ids, "Q%1[BWL]%d", bwl, &byte) == 2 &&
				    byte < IXD) {
				    gp->gt_list = (Gate**)byte;
				    gp->gt_mark = bwl[0] == 'B' ? B_WIDTH :
						  bwl[0] == 'W' ? W_WIDTH :
#if INT_MAX != 32767 || defined (LONG16)
						  bwl[0] == 'L' ? L_WIDTH :
#endif
								  0;
				    iC_QT_[byte] = bwl[0];	/* 'B', 'W' or 'L' */
				} else {
				    goto outErr;
				}
			    } else if (sp->ftype == OUTX) {
				if (sscanf(gp->gt_ids, "QX%d.%d", &byte, &bit) == 2 &&
				    byte < IXD && bit < 8) {
				    gp->gt_list = (Gate**)byte;
				    gp->gt_mark = iC_bitMask[bit];
				    iC_QT_[byte] = 'X';
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
			    /****************************************************
			     * this initialisation of clock references relies
			     * on gates which execute a function ftype != GATE
			     * having only one output which is in the first
			     * location of the function list (FL_GATE). The
			     * clock reference is put in the second location
			     * which was previously cleared by a terminator.
			     * The 3rd location holds a pointer to a Gate of
			     * ftype ARITH holding a time delay (ARN or NCONST).
			     * All action gates were initialised first, because
			     * typ < MAX_LV were done first.
			     ****************************************************/
			    for (lp = sp->list; lp; lp = lp->le_next) {
				lp->le_sym->u_gate->gt_clk = gp;
			    }
			    gp++;
			}
		    }
		}
	    }
	}
	if (iclock->type == ERR) {
	    gp++;				/* error - count iClock */
	    rc = 1;				/* generate error */
	}
	if ((gp - *igpp) == block_total) {
	    gp = *igpp;				/* repeat to initialise timer links */
	    for (typ = 0; typ < MAX_OP; typ++) {	/* keep gp in same order */
		for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
		    for (sp = *hsp; sp; sp = sp->next) {
			if (sp->type == typ) {
			    if (sp->ftype < MAX_AR) {	/* Arithmetic Gate */
				for (lp = sp->list; lp; lp = lp->le_next) {
				    tsp = lp->le_sym;
				    assert(tsp && tsp->u_gate);
				    if ((val = lp->le_val) != 0) {
					tgp = tsp->u_gate;
					if (val == (unsigned) -1) {
					    /**************************************************
					     * The 3rd location holds a pointer to a Gate of
					     * ftype ARITH holding a time delay (ARN or NCONST).
					     **************************************************/
					    tgp->gt_time = gp;
					} else {
					    /**************************************************
					     * ftype ARITH - generate backward input list
					     **************************************************/
					    i = val & 0xff;
					    val >>= 8;
					    assert((tsp->type == ARN || tsp->type == ERR) &&
						   val && i && i <= tgp->gt_new);
					    if (tgp->gt_rlist == 0) {
						tgp->gt_rlist = fp;
						fp += tgp->gt_new + 1;
						tgp->gt_rlist[0] = (Gate*)val;
					    } else {
						assert((Gate*)val == tgp->gt_rlist[0]);
					    }
					    tgp->gt_rlist[i] = gp;
					}
				    }
				}
			    }
			    gp++;
			}
		    }
		}
	    }
	    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
		for (sp = *hsp; sp; sp = sp->next) {
		    if (sp->type < MAX_OP && (tgp = sp->u_gate) != 0) {
			if (tgp->gt_new != 0) {
			    tgp->gt_new = 0;	/* restore gt_new */
			}
		    }
		}
	    }
	    if (iclock->type == ERR) {
		gp++;				/* error - count iClock */
		rc = 1;				/* generate error */
	    }
	}
    }

    if ((val = gp - *igpp) != block_total) {
	fprintf(iC_errFP,			/* either first or second scan above */
	    "\n%s: line %d: block error %d vs block_total %d\n",
	    __FILE__, __LINE__, val, block_total);
	rc = 2;
    } else if ((val = fp - ifp) != link_count) {
	fprintf(iC_errFP,
	    "\n%s: line %d: link error %d vs link_count %d\n",
	    __FILE__, __LINE__, val, link_count);
	rc = 3;
    }

    return rc;					/* return code */
} /* buildNet */
#endif /* defined(RUN) || defined(TCP) && ! defined(LOAD) */

/********************************************************************
 *
 *	Generate network as C file
 *
 *******************************************************************/

int
output(FILE * iFP, char * outfile)
{
    Symbol *	sp;
    Symbol *	gp;
    List_e *	lp;
    Symbol **	hsp;
    short	iC_dc;
    short	typ;
    unsigned	val;
    unsigned	rc = 0;				/* return code */
    unsigned	mask;
    Symbol *	tsp;
    List_e *	tlp;
    List_e *	nlp;
    List_e **	lpp;
    char *	modName;
    char *	nxs;
    char *	sam;
    int		li;				/* link index into connection list */
    int		lc;				/* link count in connection list */
    FILE *	Fp;				/* C output file */
    FILE *	H1p;				/* list _list_.c + header _list1.h */
    FILE *	H2p;				/* list header _list2.h */
    char *	cp;				/* auxiliary char pointer */
    char *	module;				/* module name built from path/name */
    char *	H1path;				/* path of PWD relative to target */
    unsigned	linecnt = 1;
    int		aliasArithFlag;
    int		fflag;

    /********************************************************************
     *	Generate linked list header, for linking several independently
     *	compiled modules
     *
     *	The following algorithm fails if two files are linked with names
     *	'ab.cd.ic' and 'ab_cd.ic' - by Goedel there is hardly a way out of this.
     *	A multiple define error occurs for the name '_ab_cd_list' at link time.
     *	The same error occurs if unfortunate path combinations are used
     *	eg: ab/cd.ic and either of the above
     *	Spaces and non-alpha-numeric characters are treated the same way
     *
     *	To allow file names starting with numerals (YUK) precede the generated
     *	variable names with a '_' in that case only (does not break regression tests)
     *	eg: 0.ic generates _0_list (iC_0_list in latest version)
     *	This example generates the executable 0, which requires special
     *	handling in Perl scripts (particularly iClive) (has been fixed - JW)
     *******************************************************************/
    module = iC_emalloc(strlen(inpNM)+2);	/* +2 for possible leading '_' and '\0' */
    if (isdigit(*inpNM)) {
	*module = '_';
	strcpy(module + 1, inpNM);		/* _<source file name> */
    } else {
	strcpy(module, inpNM);		/* <source file name> */
    }
    if ((cp = strrchr(module, '.')) != 0) {
	*cp = '\0';				/* module name without extensions */
    }
    for (cp = module; *cp != 0; cp++) {
	if (! isalnum(*cp) && *cp != '_') {
	    *cp = '_';				/*  replace all non-alnums with '_' */
	}
    }
    aliasArithFlag = iC_Aflag;

    /* open output file */

    if ((Fp = fopen(outfile, "w")) == 0) {
	rc = Oindex; goto end;
    }

    /********************************************************************
     *	Output C file header and includes
     *******************************************************************/
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
",	inpNM, outfile, iC_ID);
    linecnt += 11;

    /********************************************************************
     *	if iC_aflag generate auxiliary files <base>.h1 and <base>.h2
     *  else write direct to C file
     *******************************************************************/

    if (iC_aflag) {
	/********************************************************************
	 *  include auxiliary files <base>.h1 and <base>.h2
	 *******************************************************************/
	fprintf(Fp, "\
#include	\"%s\"\n\
#include	\"%s\"\n\
",	H1name, H2name);				/* auxiliary headers */
	linecnt += 2;
	if (iC_lflag != 0) {				/* no previous auxiliary files */
	    /********************************************************************
	     *	iC_LIST and define optional tVar definition once
	     *******************************************************************/
	    fprintf(Fp, "\
iC_Gt **	iC_list[] = { iC_LIST 0, };\n\
");							/* auxiliary list */
	    linecnt += 1;
	    /********************************************************************
	     *	generate auxiliary files <base>.h1 and <base>.h2
	     *******************************************************************/
	    if ((H1p = fopen(H1name, "w")) == 0) {	/* decleration header <base>.h1 */
		rc = H1index; goto endh;
	    }
	    fprintf(H1p, "/*### Declaration header for '%s' ###*/\n", outfile);

	    if ((H2p = fopen(H2name, "w")) == 0) {	/* list header <base>.h2 */
		rc = H2index; goto endd;
	    }
	    fprintf(H2p, "/*### List header for '%s' ###*/\n", outfile);
	    fprintf(H2p, "#define	iC_LIST\\\n");	/* list header <base>.h2 */
	} else {
	    /********************************************************************
	     *	append to auxiliary files <base>.h1 and <base>.h2
	     *******************************************************************/
	    if ((H1p = fopen(H1name, "a")) == 0) {	/* decleration header <base>.h1 */
		rc = H1index; goto endh;
	    }
	    if ((H2p = fopen(H2name, "a")) == 0) {	/* list header <base>.h2 */
		rc = H2index; goto endd;
	    }
	}
    }

    /********************************************************************
     *	Reverse action links to input links for simple Gates
     *	for Gates of ftype ARITH, keep the links in ascending order
     *******************************************************************/
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((typ = sp->type) == UDF || typ & EM) {
		fprintf(Fp, "extern iC_Gt	%s;\n", mN(sp));
		linecnt++;
	    }
	    if ((typ &= ~EM) < MAX_LV) {
		iC_dc = sp->ftype;
		if (typ == INPX) {
		    assert(iC_dc == GATE || iC_dc == ARITH);
		    assert(sp->u_blist == 0);
		}
		if (iC_dc == ARITH) {
		    for (lpp = &sp->list; (lp = *lpp) != 0; ) {
			List_e **	tlpp;
			/* leave out timing controls */
			if ((val = lp->le_val) != (unsigned) -1) {
			    tsp = lp->le_sym;	/* reverse action links */
			    for (tlpp = &tsp->u_blist;
				(tlp = *tlpp) != 0 && tlp->le_val <= val;
				tlpp = &tlp->le_next) {
				assert(val != tlp->le_val);
			    }
			    *tlpp = lp;		/* to input links */
			    *lpp = lp->le_next;
			    lp->le_sym = sp;
			    lp->le_next = tlp;	/* lpp is not changed */
			} else {
			    lpp = &lp->le_next;	/* lpp to next link */
			}
		    }
		} else
		if (iC_dc == GATE) {
		    for (lpp = &sp->list; (lp = *lpp) != 0; ) {
			/* leave out timing controls */
			if (lp->le_val != (unsigned) -1) {
			    tsp = lp->le_sym;	/* reverse action links */
			    tlp = tsp->u_blist;
			    tsp->u_blist = lp;	/* to input links */
			    *lpp = lp->le_next;
			    lp->le_sym = sp;
			    lp->le_next = tlp;	/* lpp is not changed */
			} else {
			    assert(0);
			    lpp = &lp->le_next;	/* lpp to next link */
			}
		    }
		} else
		if (iC_dc == OUTX) {
		    assert(typ == OR);
		} else
		if (iC_dc == TIMR &&
		    (lp = sp->list) != 0 &&
		    (tsp = lp->le_sym) != 0 &&
		    tsp->u_val < lp->le_val) {
		    tsp->u_val = lp->le_val;	/* store timer preset off value */
		}				/* temporarily in u (which is 0) */
		assert(iC_dc != OUTW || sp->list == 0);	/* #define no longer in use */
	    } else
	    if (typ < MAX_OP) {
		/********************************************************************
		 * this initialisation of clock references relies on gates which
		 * execute a function ftype != GATE having only one output which
		 * is in the first location of the function list (FL_GATE).
		 * The clock reference is put in the second location which was
		 * previously cleared by a terminator. The 3rd location holds a
		 * pointer to a Gate of ftype ARITH holding a time delay (ARN or NCONST).
		 * During execution of an action this pointer is used to find the
		 * correct clock block, which is used as the head of a clock list
		 * to which the action is linked. The actual clock block is made
		 * empty in the following, and action blocks are linked to it
		 * dynamically at run time.
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
		    if ((nlp = tlp->le_next) != 0 &&
			(sp->ftype == F_SW || sp->ftype == F_CF || sp->ftype == F_CE)
		    ) {
			snprintf(errorBuf, sizeof errorBuf,
			    "Action gate '%s' on clock list '%s' has more than 1 output",
			    tsp->name, sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
			break;
		    }
		    tlp->le_next = lp;
		    sp->list = lp->le_next;
		    lp->le_sym = sp;
		    lp->le_next = nlp;		/* restore ffexpr value list */
		}
	    } else
	    if (typ == ALIAS) {
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
		    if (aliasArithFlag == 1) {			/* write only once */
			aliasArithFlag = 2;			/* use extended macro now */
		    }
		} else if (sp->ftype != GATE && sp->ftype != CLCKL && sp->ftype != TIMRL) {
		    snprintf(errorBuf, sizeof errorBuf,
			"Alias '%s' has wrong ftype %s",
			sp->name, iC_full_ftype[sp->ftype]);
		    errorEmit(Fp, errorBuf, &linecnt);
		}
	    }
	}
    }

    /* do the timing controls last, to link them after their timer clock */
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((sp->type & ~EM) < MAX_LV && sp->ftype < MAX_AR) {
		for (lp = sp->list; lp; ) {
		    if (lp->le_val == (unsigned) -1) {
			tsp = lp->le_sym;	/* action gate */
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
			if ((nlp = tlp->le_next) != 0 &&
			    (sp->ftype == F_SW || sp->ftype == F_CF || sp->ftype == F_CE)
			) {
			    snprintf(errorBuf, sizeof errorBuf,
				"Action gate '%s' controlled by timer '%s' has more than output and clock",
				tsp->name, sp->name);
			    errorEmit(Fp, errorBuf, &linecnt);
			    break;
			}
			tlp->le_next = lp;
			sp->list = lp->le_next;
			lp->le_sym = sp;
			lp->le_next = nlp;	/* restore ffexpr value list */
			lp = sp->list;
		    } else {
			lp = lp->le_next;
		    }
		}
	    }
	}
    }

    /********************************************************************
     *	Output C file macros
     *******************************************************************/
    fprintf(Fp, "\
\n\
");
    linecnt += 1;
    if (functionUse[0] & F_CALLED) {		/* has any function been called ? */
	if (aliasArithFlag == 2) {
	    if (functionUse[0] & (F_ARITHM|F_FFEXPR)) {
		fprintf(Fp, "\
static iC_Gt *	iC_Mt;\n\
");	    linecnt += 1;
	    }
	    if (functionUse[0] & F_ARITHM) {
		fprintf(Fp, "\
#define iC_MV(n) ((iC_Mt=iC_gf->gt_rlist[n])->gt_ini==-iC_ALIAS\\\n\
			? ((iC_Gt*)iC_Mt->gt_rlist)->gt_new\\\n\
			: iC_Mt->gt_new)\n\
");	    linecnt += 3;
	    }
	    if (functionUse[0] & F_FFEXPR) {
		fprintf(Fp, "\
#define iC_AV(n) ((iC_Mt=iC_gf->gt_list[n])->gt_ini==-iC_ALIAS\\\n\
			? ((iC_Gt*)iC_Mt->gt_rlist)->gt_new\\\n\
			: iC_Mt->gt_new)\n\
#define iC_LV(n) ((iC_Mt=iC_gf->gt_list[n])->gt_ini==-iC_ALIAS\\\n\
			? (((iC_Gt*)iC_Mt->gt_rlist)->gt_val < 0 ? 1 : 0)\\\n\
			: (iC_Mt->gt_val < 0 ? 1 : 0))\n\
#define iC_AA(n,v) iC_assign((iC_Mt=iC_gf->gt_list[n])->gt_ini==-iC_ALIAS\\\n\
				? ((iC_Gt*)iC_Mt->gt_rlist)\\\n\
				: iC_Mt, v)\n\
#define iC_LA(n,v) iC_assign((iC_Mt=iC_gf->gt_list[n])->gt_ini==-iC_ALIAS\\\n\
					? ((iC_Gt*)iC_Mt->gt_rlist)\\\n\
					: iC_Mt, v)\n\
");	    linecnt += 12;
	    }
	    if (functionUse[0] & F_LITERAL) {
		fprintf(Fp, "\
#define iC_AVL(x) (x.gt_ini==-iC_ALIAS ? ((iC_Gt*)x.gt_rlist)->gt_new : x.gt_new)\n\
#define iC_LVL(x) (x.gt_ini==-iC_ALIAS ? (((iC_Gt*)x.gt_rlist)->gt_val < 0 ? 1 : 0)\\\n\
					: (x.gt_val < 0 ? 1 : 0))\n\
#define iC_AAL(x,v) iC_assign(x.gt_ini==-iC_ALIAS ? (iC_Gt*)x.gt_rlist : &x, v)\n\
#define iC_LAL(x,v) iC_assign(x.gt_ini==-iC_ALIAS ? (iC_Gt*)x.gt_rlist : &x, v)\n\
");	    linecnt += 5;
	    }
	} else {
	    if (functionUse[0] & F_ARITHM) {
		fprintf(Fp, "\
#define iC_MV(n) iC_gf->gt_rlist[n]->gt_new\n\
");	    linecnt += 1;
	    }
	    if (functionUse[0] & F_FFEXPR) {
		fprintf(Fp, "\
#define iC_AV(n) iC_gf->gt_list[n]->gt_new\n\
#define iC_LV(n) (iC_gf->gt_list[n]->gt_val < 0 ? 1 : 0)\n\
#define iC_AA(n,v) iC_assign(iC_gf->gt_list[n], v)\n\
#define iC_LA(n,v) iC_assign(iC_gf->gt_list[n], v)\n\
");	    linecnt += 4;
	    }
	    if (functionUse[0] & F_LITERAL) {
		fprintf(Fp, "\
#define iC_AVL(x) x.gt_new\n\
#define iC_LVL(x) (x.gt_val < 0 ? 1 : 0)\n\
#define iC_AAL(x,v) iC_assign(&x, v)\n\
#define iC_LAL(x,v) iC_assign(&x, v)\n\
");	    linecnt += 4;
	    }
	}
    }
    fprintf(Fp, "\
extern iC_Gt *	iC_l_[];\n\
");
    linecnt += 1;
    /********************************************************************
     *	Output executable gates
     *
     *	This modifies the symbol table entries
     *	the CLK and TIM list Symbols are reconnected
     *	to the action Gates which they control
     *******************************************************************/
    fprintf(Fp, "\
\n\
/********************************************************************\n\
 *\n\
 *	Gate list\n\
 *\n\
 *******************************************************************/\n\
\n\
");
    linecnt += 7;

    li = 0;
    nxs = "0";					/* 0 terminator for linked gate list */
    sam = "";					/* no & for terminator in linked Gate list */
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((typ = sp->type) > UDF && typ < MAX_OP && /* leave out EXT_TYPES */
		strcmp(sp->name, "iClock") != 0) {
		mask = 0;
		modName = mN(sp);		/* modified string, byte and bit */
		/********************************************************************
		 * mN() sets cnt, iqt, xbwl, byte and bit via IEC1131() as side effect
		 * generate gt_val (initial gate count for XOR AND OR LATCH)
		 *   0       for XOR
		 *   1       for OR
		 *   n       for AND where n is number of inputs
		 *   (n+1)/2 for LATCH
		 * and
		 * gt_ini (-type except for XOR AND OR LATCH, which repeat gate count)
		 *******************************************************************/
		if (typ >= MIN_GT && typ < MAX_GT) {	/* XOR AND OR LATCH */
		    iC_dc = 0;				/* typ == XOR */
		    if (typ == OR) {
			iC_dc = 1;			/* typ == OR */
		    } else if (typ != XOR) {		/* typ == AND LATCH */
			for (lp = sp->u_blist; lp; lp = lp->le_next) {
			    iC_dc++;			/* space in input list */
			}
			if (typ == LATCH) {
			    iC_dc++;
			    iC_dc >>= 1;		/* halve for LATCH */
			}
		    }
		    fprintf(Fp, "iC_Gt %-8s = { %d, %d,", modName, iC_dc, iC_dc);
		} else {
		    if (typ == NCONST) {
			/* NCONST Gate must be static because same constant */
			/* may be used in several linked modules - not extern */
			fprintf(Fp, "static iC_Gt %-7s", modName);
		    } else {
			fprintf(Fp, "iC_Gt %-8s", modName);
		    }
		    fprintf(Fp, " = { 1, -%s,", iC_ext_type[typ]);	/* -gt_ini */
		}
		/* generate gt_fni (ftype), gt_mcnt (0) and gt_ids */
		fprintf(Fp, " %s, 0, \"%s\",",
		    iC_ext_ftype[iC_dc = sp->ftype], sp->name);
		/* generate gt_list */
		if (iC_dc >= MIN_ACT && iC_dc < MAX_ACT) {
		    /* gt_list */
		    fprintf(Fp, " &iC_l_[%d],", li);
		    li += 2;			/* space for action or function pointer + clock */
		    if ((lp = sp->list->le_next) != 0 &&
			(lp->le_sym->type & ~EM) == TIM) {
			li++;			/* space for pointer to delay time Gate */
			lp = lp->le_next;	/* point past delay time */
		    }
		    assert(lp);
		    if (iC_dc == F_SW || iC_dc == F_CF || iC_dc == F_CE) {
			/* Function Pointer for "if" or "switch" */
			while ((lp = lp->le_next) != 0) {
			    li++;		/* count C var on list */
			}
		    }
		} else
		if (iC_dc == OUTW) {
		    if (iqt[0] == 'Q' &&	/* QB0_0 is cnt == 3 (no tail) */
			xbwl[0] != 'X' &&	/* can only be 'B', 'W' or 'L' */
			cnt == 3) {
			fprintf(Fp, " 0,");
		    } else {
			goto OutErr;
		    }
		} else
		if (iC_dc == OUTX) {
		    if (iqt[0] == 'Q' &&
			xbwl[0] == 'X' &&	/* QX0.0_0 is cnt == 5 */
			cnt == 5 && bit < 8) {
			fprintf(Fp, " 0,");
		    } else {
		    OutErr:
			fprintf(Fp, " 0,\n");
			linecnt++;
			snprintf(errorBuf, sizeof errorBuf,
			    "OUTPUT byte or bit address exceeds limit: %s",
			    sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
		    }
		} else {
		    fprintf(Fp, " 0,");		/* no gt_list */
		    if (iC_dc == TIMRL) {
			if (sp->u_val > 0) {
			    mask = sp->u_val;
			    sp->u_val = 0;	/* restore temporary u to 0 */
			}
		    }
		}
		/* generate gt_rlist */
		if (typ == ARN || (typ >= MIN_GT && typ < MAX_GT)) {
		    fprintf(Fp, " &iC_l_[%d],", li);	/* gt_rlist */
		    for (lp = sp->u_blist; lp; lp = lp->le_next) {
			li++;			/* space in input list */
		    }
		    /* space for dual GATE list or ARITH with FUNCTION */
		    li += 2;
		} else
		if (typ == INPW) {
		    if (iqt[0] == 'I' &&	/* IB0 is cnt == 3 */
			xbwl[0] != 'X' &&	/* can only be 'B', 'W' or 'L' */
			cnt == 3) {
			fprintf(Fp, " 0,");
		    } else {
			goto InErr;
		    }
		} else
		if (typ == INPX) {
		    if (iqt[0] != 'Q' &&	/* can only be 'I' or 'T' */
			xbwl[0] == 'X' &&	/* IX0.0 is cnt == 4 */
			cnt == 4 && bit < 8) {
			fprintf(Fp, " 0,");
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
		/* generate gt_next, which points to previous Gate */
		fprintf(Fp, " %s%s", sam, nxs);
		/* optionally generate timer pre-set value in gt_mark */
		if (mask != 0) {
		    fprintf(Fp, ", %d", mask);	/* bitMask for OUT */
		}
		fprintf(Fp, " };\n");
		linecnt++;
		nxs = modName;			/* previous Symbol name */
		sam = "&";
	    }
	}
    }
    /********************************************************************
     *	Do ALIASes last to avoid forward references of Gates in gt_list
     *	Resolve multiple ALIASes here for the same reason.
     *	Generate code for ARITH ALIAS only if iC_Aflag is set (-A option)
     *******************************************************************/
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((typ = sp->type) == ALIAS && sp->list != 0 &&
		((iC_dc = sp->ftype) == GATE ||
		(iC_Aflag && (iC_dc == ARITH || iC_dc == CLCKL || iC_dc == TIMRL)))) {
		modName = mN(sp);		/* modified string, byte and bit */
		fprintf(Fp, "iC_Gt %-8s", modName);	/* ZZZZ */
		val = sp->list->le_val;
		gp = sp->list->le_sym;
		while (gp->type == ALIAS) {
		    val ^= gp->list->le_val;	/* negate if necessary */
		    gp = gp->list->le_sym;	/* point to original */
		}
		fprintf(Fp,
		    " = { 1, -%s, %s, 0, \"%s\", 0, (iC_Gt**)&%s, %s%s, %d };\n",
		    iC_ext_type[typ], iC_ext_ftype[iC_dc], sp->name, mN(gp), sam, nxs, val);
		linecnt++;
		nxs = modName;			/* previous Symbol name */
		sam = "&";
	    }
	}
    }
    /********************************************************************
     *	Link counting in output() counts all reverse links and is thus very
     *	different to listNet() therefore cannot compare link_count and li
     *
     *	module string generated at start
     *******************************************************************/
    fprintf(Fp, "\
\n\
iC_Gt *		iC_%s_list = %s%s;\n\
",  module, sam, nxs);
    linecnt += 2;
    if (iC_aflag) {
	/********************************************************************
	 *	iC_list will be built from aux files
	 *******************************************************************/
	fprintf(H1p, "extern iC_Gt *	iC_%s_list;\n", module);	/* header _list1.h */
	fprintf(H2p, "	&iC_%s_list,\\\n", module);	/* list header _list2.h */
    } else {
	/********************************************************************
	 *	iC_list already complete - no need for aux files
	 *******************************************************************/
	fprintf(Fp, "\
iC_Gt **	iC_list[] = { &iC_%s_list, 0, };\n\
",	module);
	linecnt += 1;
    }
    /********************************************************************
     *	define optional tVar definition
     *******************************************************************/
    if (iC_Tflag) {				/* imm_identifier++ -- used in c_compile */
#if INT_MAX == 32767 && defined (LONG16)
	fprintf(Fp, "static long	iC_tVar;\n");
#else
	fprintf(Fp, "static int	iC_tVar;\n");
#endif
	linecnt++;
    }
    free(module);
    fprintf(Fp, "\
\n\
");
    linecnt += 1;

    if (functionUse[0] & F_CALLED) {		/* has any function been called ? */
	fprintf(Fp, "\
/********************************************************************\n\
 *\n\
 *	Literal blocks and embedded C fragment functions\n\
 *\n\
 *******************************************************************/\n\
\n\
");
	linecnt += 6;

	/* copy C intermediate file up to EOF to C output file */
	/* translate any imm variables and ALIAS references of type 'QB1_0' */

	if ((rc = copyXlate(iFP, Fp, outfile, &linecnt, 01)) != 0) { /* copy literal blocks */
	    goto endm;
	}
	if ((rc = copyXlate(iFP, Fp, outfile, &linecnt, 02)) != 0) { /* copy called functions */
	    goto endm;
	}
    }
    /********************************************************************
     *	produce initialised connection lists
     *	using modified symbol table
     *******************************************************************/
    if (li == 0) goto endm;		/* MS-C does not hack 0 length array - gcc does */
    fprintf(Fp, "\
/********************************************************************\n\
 *\n\
 *	Connection lists\n\
 *\n\
 *******************************************************************/\n\
\n\
static iC_Gt *	iC_l_[] = {\n\
");
    linecnt += 7;

    lc = 0;					/* count links */
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((typ = sp->type) == ARN ||	/* leave out UDF, ARNC, LOGC */
		(typ >= MIN_GT && typ < MAX_GT)) {	/* leave out EXT_TYPES */
		int		len = 16;
		char *	fs = strlen(sp->name) > 1 ? "\t" : "\t\t";

		fprintf(Fp, "/* %s */", sp->name);
		if ((iC_dc = sp->ftype) >= MIN_ACT && iC_dc < MAX_ACT) {
		    if ((lp = sp->list) == 0) {
			snprintf(errorBuf, sizeof errorBuf,
			    "Action gate '%s' has no action list",
			    sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
		    } else {
			fflag = 0;
			if (lp->le_sym == 0) {	/* dc == F_SW, F_CF or F_CE */
			    assert(iC_dc == F_SW || iC_dc == F_CF || iC_dc == F_CE);
			    /* Function Pointer for "if" or "switch" */
			    fflag = 1;
			    len += 17;		/* assume len of %d is 2 */
			    fprintf(Fp, "%s(iC_Gt*)iC_%d,", fs, lp->le_val >> 8);
			} else {
			    len += strlen((tsp = lp->le_sym)->name) + 3;
			    fprintf(Fp, "%s&%s,",
				fs, mN(tsp));	/* action */
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
			    fflag = 0;		/* do not try to scan C var list */
			} else {
			    len += strlen((tsp = lp->le_sym)->name) + 3;
			    assert((tsp->type & ~EM) == CLK || (tsp->type & ~EM) == TIM);
			    fprintf(Fp, "%s&%s,",
				fs, mN(tsp));	/* clock or timer */
			    if ((tsp->type & ~EM) == TIM) {
				if ((lp = lp->le_next) != 0) {
				    len += strlen((tsp = lp->le_sym)->name) + 3;
				    fprintf(Fp, "%s&%s,", fs, mN(tsp));	/* delay time */
				} else {
				    len += 3;	/* 0 filler - no delay time */
				    fprintf(Fp, "%s0,", fs);
				    snprintf(errorBuf, sizeof errorBuf,
					"Action gate '%s' has timer '%s' with no delay",
					sp->name, tsp->name );
				    errorEmit(Fp, errorBuf, &linecnt);
				    fflag = 0;	/* do not try to scan C var list */
				}
				lc++;
			    }
			}
			lc++;
			if (fflag) {		/* if else or switch */
			    while ((lp = lp->le_next) != 0) {	/* scan C var list */
				tsp = lp->le_sym;
				while (tsp->type == ALIAS) {
				    tsp = tsp->list->le_sym;	/* point to original */
				}
				len += strlen(tsp->name) + 3;
				fprintf(Fp, "%s&%s,", fs, mN(tsp));
				lc++;
			    }
			}
			fs = "\t";
			len += 8; len &= 0xfff8;
		    }
		}

		val = 0;
		if (typ == ARN) {
		    if ((lp = sp->u_blist) == 0) {
			snprintf(errorBuf, sizeof errorBuf,
			    "Arithmetic gate '%s' has no function",
			    sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
		    } else {
			/* Function Pointer at start of input list */
			len += 17;		/* assume len of %d is 2 */
			if (lp->le_val) {
			    fprintf(Fp, "%s(iC_Gt*)iC_%d,", fs, lp->le_val >> 8);
			} else {
			    fprintf(Fp, "%s(iC_Gt*)0,", fs);	/* OUTW */
			}
			lc++;
			fs = " ";
		    }
		    for (lp = sp->u_blist; lp; lp = lp->le_next) {
			len += strlen(lp->le_sym->name) + 3;
			if (len > 73) {
			    fs = "\n\t\t";
			    linecnt++;
			    len = 16 + strlen(lp->le_sym->name) + 3;
			}
			/* check order of arithmetic input index from op_asgn() */
			assert(lp->le_val == 0 || ++val == (lp->le_val & 0xff));
			fprintf(Fp, "%s&%s,", fs, mN(lp->le_sym));
			lc++;
			fs = " ";
		    }
		    fprintf(Fp, "%s0,", fs);
		    lc++;
		    len += 3;
		    fs = " ";
		} else
		if (typ >= MIN_GT && typ < MAX_GT) {
		    do {
			for (lp = sp->u_blist; lp; lp = lp->le_next) {
			    if (lp->le_val == val) {
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
	    } else if (iC_aflag	&&		/* appending more modules */
		typ == IFUNCT &&		/* function has occurred */
		(lp = sp->u_blist) != 0 &&	/* and been defined in this module */
		(val = lp->le_val) != 0) {	/* and has been called */
		/********************************************************************
		 * The instance number is only incremented for those calls, in which
		 * at least one local variable name with an instance number extension
		 * is generated and the function has actually been called.
		 * In case a module does not define a particular function, it cannot
		 * have been called in this module, and no new instance number update
		 * need be written.
		 *******************************************************************/
		fprintf(H1p, "/* %s\t%d */\n", sp->name, val);	/* header _list1.h */
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
    if (iC_debug & 010) {
	fprintf(iC_outFP, "\nC OUTPUT: %s  (%d lines)\n", outfile, linecnt-1);
    }
    if (iC_aflag) {
	fclose(H2p);			/* close list header in case other actions */
endd:
	fclose(H1p);			/* close declaration header in case other actions */
    }
endh:
    fclose(Fp);				/* close C output file in case other actions */
end:
    return rc;				/* return code */
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

static int	 precompileFlag = 0;

static int
copyBlocks(FILE * iFP, FILE * oFP, int mode)
{
    int		c;
    int		mask = 02;			/* default is functions or cases */
    int		lf = 0;				/* set by first non white space in a line */
    char *	lp;
    char	lstBuf[BUFS];			/* include file name */
    char	lineBuf[BUFS];			/* can be smaller than a line */

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
		    if(iFP == T4FP) {
			ierror("copyBlocks: if else or switch has:", lstBuf);
			continue;
		    }
		    if ((c = openT4T5(0)) != 0) return c;	/* re-open if necessary */
		    if (iC_debug & 02) fprintf(iC_outFP, "####### c_parse #include %s\n", lstBuf);
		    fprintf(T4FP, "#include %s\n", lstBuf);	/* a little C file !!! */
		    precompileFlag++;
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
 *	For auxiliary parse of iC if - else and switch functions
 *	iFP == T4FP which have no need for C - pre-processor includes
 *
 *******************************************************************/

extern FILE* yyin;
extern FILE* yyout;

int
c_compile(FILE * iFP, FILE * oFP, int flag, List_e * lp)
{
    int		r;				/* copy literal blocks */
    char	lineBuf[BUFS];			/* can be smaller than a line */

    lexflag = flag;				/* output partial source listing */

    if (ftell(T2FP)) {
	fclose (T2FP);				/* overwrite intermediate file */
	if ((T2FP = fopen(T2FN, "w+")) == NULL) {
	    return T2index;
	}
    }
    if (copyBlocks(iFP, T2FP, 01)) {
	return T1index;
    }
    if (outFlag == 0) {				/* -c option to produce cexe.c */
#if INT_MAX == 32767 && defined (LONG16)
	fprintf(T2FP, "/*##*/long iC_exec(int iC_index) { switch (iC_index) {\n");
#else
	fprintf(T2FP, "/*##*/int iC_exec(int iC_index) { switch (iC_index) {\n");
#endif
    } else {
	fprintf(T2FP, "/*##*/\n");		/* -o option - seperate blocks */
    }
    if (copyBlocks(iFP, T2FP, 02)) {
	return T1index;
    }
    if (outFlag == 0) {
	fprintf(T2FP, "/*##*/}}\n");
    }
    if (precompileFlag) {
	/********************************************************
	 *  handle pre-processor #include <stdio.h> and "icc.h" etc
	 *  add CTYPE's as symbols to iC symbol table for main C-parse
	 ********************************************************/
	fflush(T4FP);
	/* Cygnus does not understand cc - use gcc */
	snprintf(lineBuf, sizeof lineBuf, "gcc -E -x c %s -o %s", T4FN, T5FN);
	if (iC_debug & 02) fprintf(iC_outFP, "####### pre-compile: %s\n", lineBuf);
	r = system(lineBuf);			/* Pre-compile C file */
	if (iC_debug & 02) fprintf(iC_outFP, "####### pre-compile: return %d\n", r);

	if (r != 0 || (T5FP = fopen(T5FN, "r")) == NULL) {
	    ierror("c_compile: cannot open:", T5FN);
	    perror("open");
	    return T5index;
	}
	if (iC_debug & 02) fprintf(iC_outFP, "####### compile include files via %s %s\n", T4FN, T5FN);
	yyin = T5FP;				/* lexc reads from include now */
	yyout = iC_outFP;			/* list file */
	lexflag |= C_PARSE|C_NO_COUNT|C_FIRST;	/* do not count characters for include files */
	gramOffset = lineno = 0;
	if (c_parse() != 0) {			/* C parse headers to obtain CTYPE's in ST */
	    ierror("c_compile: Parse error in includes\n", T5FN);
	}
	lexflag &= ~C_NO_COUNT;			/* count characters again */
    }
    if (fseek(T2FP, 0L, SEEK_SET) != 0) {	/* rewind intermediate file */
	return T2index;
    }
    copyAdjust(NULL, NULL, lp);			/* initialize lineEntryArray */
    gramOffset = lineno = 0;
    yyin = T2FP;				/* C input to C parser */
    yyout = iC_outFP;				/* list file */
    if (c_parse() != 0) {			/* actual C parser call */
	ierror("c_compile: Parse error\n", NULL);
    }
    if (fseek(T2FP, 0L, SEEK_SET) != 0) {	/* rewind intermediate file */
	return T2index;
    }
    copyAdjust(T2FP, oFP, lp);			/* output adjusted C-code */
    return 0;
} /* c_compile */

/********************************************************************
 *
 *	Then this code is parsed by the C compiler and afterwards split on
 *	## in comments and output to oFP
 *
 *******************************************************************/

int
copyXlate(FILE * iFP, FILE * oFP, char * outfile, unsigned * lcp, int mode)
{
    int		mask = 01;			/* copy literal blocks */
    int		flag = 1;
    int		cFn;				/* C function number */
    char	lineBuf[BUFS];			/* can be smaller than a line */

    if (fseek(iFP, 0L, SEEK_SET) != 0) {	/* rewind intermediate file */
	return T3index;
    }

    while (fgets(lineBuf, sizeof lineBuf, iFP)) {
	if (strncmp(lineBuf, "/*##*/", 6) == 0) {
	    mask = 02;				/* copy functions or cases */
	} else if (mode & mask) {
	    if (mask == 02 &&			/* do not look in literals - mode 01 */
		sscanf(lineBuf, cexeString[outFlag], &cFn) == 1) {
		assert(cFn < functionUseSize);
		flag = functionUse[cFn];	/* has this function or case been called ? */
	    }
	    if (flag || outfile == 0) {		/* skip functions or cases not called */
		if (lcp) (*lcp)++;		/* count lines actually output */
		if (outfile && strcmp(lineBuf, "##\n") == 0) {
		    fprintf(oFP, "#line %d \"%s\"\n", *lcp, outfile);
		} else {
		    fputs(lineBuf, oFP);
		}
	    }
	}
    }
    return 0;
} /* copyXlate */

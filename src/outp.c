static const char outp_c[] =
"@(#)$Id: outp.c,v 1.81 2006/02/23 17:41:09 jw Exp $";
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
#include	<windows.h>
#endif	/* _WINDOWS */
#include	<stdio.h>
#include	<stdlib.h>
#ifndef	WIN32
#include	<unistd.h>
#endif	/* WIN32 */
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
 *	Also converts -ve numbers to number preceded by 2 undersores.
 *		eg: -1    returns 1, -1 in bytep "" and "__1" in buf.
 *		eg: -123_5  returns 2, -123 in bytep, "_5" in tail and
 *			   "__123_5" in buf.
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
	if (*bytep >= 0) {
	    snprintf(buf, bufLen-1, "_%d%s", *bytep, tail);
	} else {
	    snprintf(buf, bufLen-1, "__%d%s", -(*bytep), tail);
	}
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
    if (outFlag && (count = sscanf(name, "__%d%7s", bytep, tail)) == 1) {
	snprintf(buf, bufLen-1, "-%d", *bytep);
    } else if (outFlag && (count = sscanf(name, "_%d%7s", bytep, tail)) == 1) {
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
    fprintf(Fp, "\n#warning \"iC: %s\"\n", errorMsg);
    (*lcp)++;					/* count lines actually output */
    errmess("ErrorEmit", errorMsg, NS);
    if (++iC_iErrCount >= iC_maxErrCount) {
	iclock->type = ERR;			/* prevent execution */
	fprintf(Fp, "\n#error \"too many errors - iC compilation aborted\"\n");
	fflush(Fp);
	errmess("ErrorEmit", "too many errors - iC compilation aborted", NS);
	if (T0FP) fseek(T0FP, 0L, SEEK_END);	/* flush rest of file */
	longjmp(beginMain, Iindex);
    }
} /* errorEmit */

/********************************************************************
 *
 *	Output the forward network as NET TOPOLOGY in the listing file.
 *	Gather information on total memory required for generation in
 *	buildNeti(). These are block_total, link_count and gate_count[]
 *	for each type of Gate. This must be done after the iC and
 *	C compile phase but before buildNet().
 *
 *	output() and later load generate their own counts.
 *
 *	The collected counts are output as NET STATISTICS in the listing.
 *	They only apply precisely for the old fashioned generation with
 *	buildNet(), but they give an idea of memory requirements for fully
 *	linked iC applications.
 *
 *	if (iC_debug & 020) list NET TOPOLOGY
 *	if (iC_debug & 010) list NET STATISTICS
 *	if (iC_debug & 04002) list SYMBOL TABLE as a debugging aid
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
    int		iClockAlias;
    int		iClockHidden;
    int		fcnt;
    int		typ;
    int		undefined;
    long	byte_total;
    int		fflag;

    link_count = block_total = undefined = iClockAlias = 0;	/* init each time */
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
    sp = lookup(ICONST);			/* iConst */
    assert(sp);
    if (sp->u_val == 0) {
	unlink_sym(sp);				/* iConst was not used - delete from ST */
    }
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    sp->v_cnt = 0;			/* v_elist & v_glist no longer needed */
	    /* check if iClock is referenced in and ALIAS */
	    if ((sp->type & ~EM) == ALIAS && sp->ftype == CLCKL) {
		tsp = sp;
		while ((tsp->type & ~EM) == ALIAS) {
		    tsp = tsp->list->le_sym;	/* point to original */
		    assert(tsp->ftype == CLCKL);
		}
		if (tsp == iclock) {
		    iClockAlias = 1;		/* include iClock below if ALIAS */
		}
	    }
	    if ((sp->type & TM) == ARNF) {
		sp->type += (ARN-ARNF);		/* convert ARNF back to ARN */
	    }
	}
    }
    iClockHidden = 0;
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if (sp->type & EM) {
		extFlag = 1;
	    }
	    if ((typ = sp->type & ~EM) < MAX_LS) {
		/* ignore iClock unless used, ERR (! CLK) or referenced in an ALIAS */
		if (sp != iclock || sp->list != 0 || typ != CLK || iClockAlias) {
		    gate_count[typ]++;		/* count ALIAS and ERR typ */
		    if (typ < MAX_OP) {
			block_total++;		/* don't count ALIAS and ERR typ */
		    }
		    if (typ < MAX_LV &&		/* don't count outputs */
			sp->ftype != OUTX && sp->ftype != OUTW) {
			fflag = 0;
			for (lp = sp->list; lp; lp = lp->le_next) {
			    if (sp->ftype == F_CF && lp->le_sym == sp) {
				fflag = 2;	/* detected _f0_1 */
			    }
			    if (lp->le_val != (unsigned) -1) {
				if (fflag == 2) {
				    fflag = 1;	/* miss only first link of _f0_1 */
				} else {
				    link_count++;
				    if (sp->ftype < MAX_AR && lp->le_val != 0) {
					tsp = lp->le_sym;	/* arithmetic function */
					assert(tsp && (tsp->type == ARN || tsp->type == SH || tsp->type == NCONST || tsp->type == ARNC || tsp->type == ERR));
					tsp->v_cnt++;	/* count reverse parameter */
				    }
				}
			    }
			}
			if (fflag == 0) {	/* miss for _f0_1 */
			    link_count++;		/* for terminator or clock or timer */
			    if (sp->ftype >= MAX_AR) {
				link_count++;		/* 2nd terminator for inverted */
			    }				/* or time for TIMER action */
			}
		    }
		    if (iC_debug & 020) {
			fprintf(iC_outFP, "%s\t%c  %c", sp->name,
			    iC_os[typ], iC_fos[sp->ftype]);
			fcnt = 0;
			for (lp = sp->list; lp; lp = lp->le_next) {
			    tsp = lp->le_sym;
			    if (fcnt++ >= 8) {
				fcnt = 1;
				fprintf(iC_outFP, "\n\t");
			    }
			    if (sp->ftype == F_SW || sp->ftype == F_CF || sp->ftype == F_CE) {
				/* case number of "if" or "switch" C fragment */
				if ((unsigned)lp->le_val <= VAR_MASK && lp->le_val != 0) {
				    fprintf(iC_outFP, "\t%c (%d)",
					iC_os[iC_types[sp->ftype]], lp->le_val >> 8);
				} else if (tsp == sp) {
				    fcnt--;		/* dummy link of _f0_1 */
				} else {
				    goto normalOut;	/* reference link of _f0_1 */
				}
			    } else if (sp->ftype == TIMR && lp->le_val > 0) {
				 /* timer preset off value */
				fprintf(iC_outFP, "\t %s%c%d",
				    tsp->name, iC_os[tsp->type & TM], lp->le_val);
			    } else if (sp->ftype < MAX_AR && lp->le_val == (unsigned) -1) {
				/* reference to a timer value - no link */
				fprintf(iC_outFP, "\t<%s%c", tsp->name, iC_os[tsp->type & TM]);
			    } else {
				normalOut:
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
		} else {
		    iClockHidden = 1;
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
		assert(sp->type == ARN || sp->type == SH || sp->type == NCONST || sp->type == ARNC || sp->type == ERR);
		link_count += sp->v_cnt + 1;	/* space for reverse links + function # */
	    }
	    sp->v_cnt = 0;
	}
    }

    if ((iC_debug & 010) &&
	(byte_total = (long)block_total * sizeof(Gate) + (long)link_count * sizeof(Gate *))
    ) {
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
    if (iClockHidden) {
	block_total++;				/* iClock is generated anyway in buildNet() */
	gate_count[CLK]++;
    }
    /* do not change spelling - used in 'pplstfix' */
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

Gate **		iC_sTable;			/* pointer to dynamic array */
Gate **		iC_sTend;			/* end of dynamic array */

/********************************************************************
 *
 *	Generate execution network for icr or ict direct execution
 *	igpp ==> array of Gates generated with calloc(block_total)
 *	gate_count splits this array into different type Gates in iC_icc()
 *
 *******************************************************************/

int
buildNet(Gate ** igpp, unsigned gate_count[])
{
    Symbol *	sp;
    Symbol *	tsp;
    List_e *	lp;
    Symbol **	hsp;
    int		typ;
    unsigned	val;
    unsigned	i;
    unsigned	rc = 0;				/* return code */
    Gate *	gp;
    Gate *	tgp;
    Gate **	fp;
    Gate **	tfp;
    Gate **	ifp;
    Gate *	op;
    Gate *	g_list = 0;
    int		byte1;
    int		bit1;
#ifndef	TCP
    char	bwl[2];				/* "B" or "W" */
#else	/* TCP */
    int		ftyp;
    Gate *	e_list = 0;
    char *	cp;
    int		eLen;
    char	iqt1[2];			/* single char buffer - space for 0 terminator */
    char	xbwl1[2];			/* single char buffer - space for 0 terminator */
    char	tail1[8];			/* compiler generated suffix _123456 max */
    char	eBuf[ESIZE];
#endif	/* TCP */

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
		    gp->gt_ini = -typ;		/* overwritten for AND OR LATCH types */
		    gp->gt_fni = sp->ftype;	/* basic gate first */
		    gp->gt_ids = sp->name;	/* gate to symbol name */
		    gp->gt_next = g_list;	/* link rest of g_list to new Gate */
		    g_list = sp->u_gate = gp++;	/* symbol to gate */
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
				    tfp = fp;	/* start of new gate list */
				    for (lp = sp->list; lp; lp = lp->le_next) {
					tsp = lp->le_sym;
					assert(tsp && tsp->u_gate);
					if (sp->ftype < MAX_AR) {
					    if (lp->le_val == (unsigned) -1) {
						continue; /* timer value link */
					    }
					    tgp = tsp->u_gate;
					    if (lp->le_val != 0) {
						assert(tsp->type == ARN || tsp->type == ERR);
						tgp->gt_new++;	/* count ARITH inputs */
					    }
					} else if (val != lp->le_val) {
					    continue;	/* not right logical value */
					}
					*fp++ = tsp->u_gate;
				    }
				    /* sort gate list */
				    qsort(tfp, fp - tfp, sizeof(Gate*), (iC_fptr)iC_cmp_gt_ids);
				    *fp++ = 0;		/* gate list terminator */
				} while (val ^= NOT);
#ifndef	TCP
				if (typ == INPW) {
				    if (sscanf(gp->gt_ids, "I%1[BWL]%d",
					bwl, &byte1) == 2 && byte1 < IXD) {
					if (bwl[0] == 'B') {
					    iC_IB_[byte1] = gp;
					} else if (bwl[0] == 'W') {
					    iC_IW_[byte1] = gp;
					} else if (bwl[0] == 'L') {
#if INT_MAX != 32767 || defined (LONG16)
					    iC_IL_[byte1] = gp;
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
					&byte1, &bit1) == 2 &&
					byte1 < IXD && bit1 < 8) {
					iC_IX_[byte1 * 8 + bit1] = gp;
				    } else if (sscanf(sp->name, "TX%d.%d",
					&byte1, &bit1) == 2 &&
					byte1 < TXD && bit1 < 8) {
					iC_TX_[byte1 * 8 + bit1] = gp;
				    } else {
				    inErr:
		    ierror("INPUT byte or bit address exceeds limit:", sp->name);
				    }
				}
#endif	/* TCP */
			    } else if (sp->ftype < MAX_ACT) {
				/* D_SH - TIMR relies on action gates having only one output */
				lp = sp->list;
				if ((tsp = lp->le_sym) != 0 && tsp != sp) {
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
				    if (lp->le_val == 0) {
					tgp = sp->u_gate;
					assert(tgp && tsp == sp && strcmp(sp->name, "_f0_1") == 0);
					iC_pf0_1 = tgp;	/* pointer to _f01 used in cexe.c */
					tgp->gt_mcnt = 1;
					i = 0;
				    } else {
					fp[0] = (Gate*)(lp->le_val >> 8);
					fp[1] = 0;	/* room for clock or timer entry */
					fp[2] = 0;	/* room for time delay or first parameter */
					i   = 3;	/* offset for above */
				    }
				    while ((lp = lp->le_next) != 0) {
					val = lp->le_val & VAR_MASK;
					tsp = lp->le_sym;
					while (tsp->type == ALIAS) {
					    tsp = tsp->list->le_sym;	/* point to original */
					}
					assert(tsp && tsp->u_gate);
					fp[val] = tsp->u_gate;
					i++;	/* count parameters */
				    }
				    fp += i;	/* space for above entries */
				}
#ifndef	TCP
			    } else if (sp->ftype == OUTW) {
				if (sscanf(gp->gt_ids, "Q%1[BWL]%d", bwl, &byte1) == 2 &&
				    byte1 < IXD) {
				    gp->gt_list = (Gate**)byte1;
				    gp->gt_mark = bwl[0] == 'B' ? B_WIDTH :
						  bwl[0] == 'W' ? W_WIDTH :
#if INT_MAX != 32767 || defined (LONG16)
						  bwl[0] == 'L' ? L_WIDTH :
#endif
								  0;
				    iC_QT_[byte1] = bwl[0];	/* 'B', 'W' or 'L' */
				} else {
				    goto outErr;
				}
			    } else if (sp->ftype == OUTX) {
				if (sscanf(gp->gt_ids, "QX%d.%d", &byte1, &bit1) == 2 &&
				    byte1 < IXD && bit1 < 8) {
				    gp->gt_list = (Gate**)byte1;
				    gp->gt_mark = iC_bitMask[bit1];
				    iC_QT_[byte1] = 'X';
				} else {
				outErr:
		    ierror("OUTPUT byte or bit address exceeds limit:", gp->gt_ids);
				    gp->gt_list = (Gate**)0;
				    gp->gt_mark = 0;	/* no change in bits */
				}
			    } else {
#else	/* TCP */
			    } else if (sp->ftype != OUTW && sp->ftype != OUTX) {
#endif	/* TCP */
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
				tsp = lp->le_sym;
				assert(tsp && tsp->u_gate);
				tsp->u_gate->gt_clk = gp;
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
					} else
					if ((val &= VAR_MASK) != 0) {
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
			tgp->gt_new = 0;	/* restore gt_new */
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
    } else
    if ((val = fp - ifp) != link_count) {
	fprintf(iC_errFP,
	    "\n%s: line %d: link error %d vs link_count %u\n",
	    __FILE__, __LINE__, val, link_count);
	rc = 3;
    } else {
#ifdef	TCP
	/********************************************************************
	 * Do ALIASes last to avoid forward references of Gates in gt_list
	 * Resolve multiple ALIASes here for the same reason.
	 * Generate all ALIASes for display, independent of -A flag
	 *******************************************************************/
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if ((typ = sp->type) == ALIAS && sp->list != 0 &&
		    ((ftyp = sp->ftype) == GATE || ftyp == ARITH || ftyp == CLCKL || ftyp == TIMRL)) {
		    val = sp->list->le_val;
		    tsp = sp->list->le_sym;
		    while (tsp->type == ALIAS) {
			val ^= tsp->list->le_val;	/* negate if necessary */
			tsp = tsp->list->le_sym;	/* point to original */
		    }
		    /* generate a new auxiliary Gate for ALIAS */
		    eLen = strlen(sp->name) + 1;
		    tgp = (Gate *) iC_emalloc(sizeof(Gate));
		    tgp->gt_ids = iC_emalloc(eLen);	/* all bytes 0 */
		    strncpy(tgp->gt_ids, sp->name, eLen);
		    tgp->gt_ini = -typ;		/* ALIAS */
		    tgp->gt_fni = ftyp;	/* ftype */
		    tgp->gt_rlist = (Gate **)tsp->u_gate;	/* link to original */
		    tgp->gt_mark = val;			/* ALIAS inversion flag */
		    tgp->gt_next = g_list;		/* link rest of g_list to new Gate */
		    g_list = sp->u_gate = tgp;		/* symbol to gate and new Gate at front of g_list */
		    block_total++;
		}
	    }
	}
	/********************************************************************
	 * Generate an INPW/TRAB Gate IXx for each first IXx.y of gt_ini -INPX
	 * Generate an INPW/TRAB Gate TXx for each first TXx.y of gt_ini -INPX
	 * Generate an INPB/OUTW Gate QXx for each first QXx.y_0 of gt_fni OUTX
	 * These new Gates are linked in an extra list, which can be scanned
	 * reasonably quickly for duplicates. At the end of Pass 0, the extra
	 * list is linked into the start of the first entry in iC_list[], so the
	 * newly generated Gates are also scanned in Pass 1 and 2 and then sorted.
	 *******************************************************************/
	for (op = g_list; op; op = op->gt_next) {
	    /********************************************************************
	     * handle bit I/O
	     *******************************************************************/
	    if (op->gt_ini == -INPX || op->gt_fni == OUTX) {
		strncpy(eBuf, op->gt_ids, ESIZE);
		cp = strchr(eBuf, '.');
		i = sscanf(eBuf, "%1[IQT]X%5d.%5d%7s", iqt1, &byte1, &bit1, tail1);
		tgp = 0;				/* for correct inError message */
		if (cp && bit1 < 8) {
		    *cp = '\0';			/* terminate extra name */
		    eLen = cp - eBuf + 1;
		    for (tgp = e_list; tgp != 0; tgp = tgp->gt_next) {
			if (strcmp(tgp->gt_ids, eBuf) == 0) {
			    goto linkIO;		/* previously generated */
			}
		    }
		    /* generate a new auxiliary Gate for bit I/O */
		    tgp = (Gate *) iC_emalloc(sizeof(Gate));
		    tgp->gt_ids = iC_emalloc(eLen);	/* all bytes 0 */
		    strncpy(tgp->gt_ids, eBuf, eLen);
		    tgp->gt_next = e_list;		/* link rest of e_list to new Gate */
		    e_list = tgp;			/* new Gate at front for speed */
		    block_total++;
		    if (op->gt_ini == -INPX) {
			tgp->gt_ini = -INPW;
			tgp->gt_fni = TRAB;
			tgp->gt_list = (Gate **) iC_emalloc(8 * sizeof(Gate *));
		    } else {			/* (op->gt_fni == OUTX) */
			tgp->gt_ini = -INPB;
			tgp->gt_fni = OUTW;
		    }
		  linkIO:
		    if (op->gt_ini == -INPX) {
			if (i != 3 || op->gt_rlist != 0 || strcmp(iqt1, "Q") == 0) goto pass0Err;
			tgp->gt_list[bit1] = op;	/* pointer to bit Gate */
			/* ###### no back link ####### */
		    } else {			/* (op->gt_fni == OUTX) */
			if (i != 4 || strcmp(tail1, "_0") || strcmp(iqt1, "Q")) goto pass0Err;
			op->gt_mark = iC_bitMask[bit1];
			op->gt_list = (Gate**)tgp;	/* pointer directly in gt_list */
			/* ###### no back link ####### */
		    }
		    tgp->gt_mark |= iC_bitMask[bit1];	/* note used bits for registration */
		} else {
		  pass0Err:
		    ierror("OUTPUT invalid I/O Gate:", op->gt_ids);
		    return 2;
		}
	    }
	    gp = op;					/* remember last entry of g_list */
	}
	gp->gt_next = e_list;				/* concatenate e_list to tail of g_list */
	tgp = 0;					/* for correct inError message */
#endif	/* TCP */
	iC_sTable = iC_sTend = (Gate **)calloc(block_total, sizeof(Gate *));
	for (op = g_list; op; op = op->gt_next) {
	    *iC_sTend++ = op;				/* enter node into sTable */
#ifdef	TCP
	    /********************************************************************
	     *  arithmetic or logical input linkage to physical I/O (mainly display)
	     *******************************************************************/
	    if (op->gt_ini == -INPW) {
		if ((i = sscanf(op->gt_ids, "%1[IT]%1[XBWL]%5d%7s",
			iqt1, xbwl1, &byte1, tail1)) == 3) {
		    switch (iqt1[0]) {
		    case 'I':
			switch (xbwl1[0]) {
			case 'X':
			    if (byte1 == 0) {
				iC_IX0p = op;		/* link for iC_display logic only */
			    }
			    break;
			case 'B':
			    if (byte1 == 1) {
				iC_IB1p = op;		/* link for iC_display logic only */
			    }
			    break;
			case 'W':
			    if (byte1 == 2) {
				iC_IW2p = op;		/* link for iC_display logic only */
			    }
			    break;
#if INT_MAX != 32767 || defined (LONG16)
			case 'L':
			    if (byte1 == 4) {
				iC_IL4p = op;		/* link for iC_display logic only */
			    }
			    break;
#endif
			default:
			    goto pass0Err;
			}
			break;
		    case 'T':
			if (byte1 != 0) goto pass0Err;	/* TXD must be 1 */
			iC_TX0p = op;			/* forward input link */
			break;
		    default:
			goto pass0Err;
		    }
		}
	    }
	    /********************************************************************
	     *  arithmetic or logical output linkage to physical I/O
	     *******************************************************************/
	    if (op->gt_fni == OUTW) {
		if ((i = sscanf(op->gt_ids, "Q%1[XBWL]%5d%7s", xbwl1, &byte1, tail1)) >= 2) {
		    switch (xbwl1[0]) {
		    case 'X':
			if (i > 2) goto pass0Err;	/* no tail1 _0 allowed for QXn */
			if (byte1 == 0) {
			    iC_QX0p = op;		/* link for iC_display logic only */
			}
			break;				/* op->gt_mark set to used bits 0x01 to 0x80 in Pass 0 */
		    case 'B':
			op->gt_mark = B_WIDTH;
			if (byte1 == 1) {
			    iC_QB1p = op;		/* link for iC_display logic only */
			}
			break;
		    case 'W':
			op->gt_mark = W_WIDTH;
			if (byte1 == 2) {
			    iC_QW2p = op;		/* link for iC_display logic only */
			}
			break;
#if INT_MAX != 32767 || defined (LONG16)
		    case 'L':
			op->gt_mark = L_WIDTH;
			if (byte1 == 4) {
			    iC_QL4p = op;		/* link for iC_display logic only */
			}
			break;
#endif
		    default:
			goto pass0Err;
		    }
		} else {
		    goto pass0Err;
		}
	    }
#endif	/* TCP */
	}
	if ((val = iC_sTend - iC_sTable) != block_total) {
	    fprintf(iC_errFP,			/* either first or second scan above */
		"\n%s: line %d: Symbol Table size %d vs block_total %d\n",
		__FILE__, __LINE__, val, block_total);
	    rc = 2;
	} else {
	    /********************************************************************
	     * Sort the symbol table in order of gt_ids.
	     *******************************************************************/
	    qsort(iC_sTable, iC_sTend - iC_sTable, sizeof(Gate*), (iC_fptr)iC_cmp_gt_ids);
	}
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
    List_e *	lp;
    Symbol **	hsp;
    int		ftyp;
    int		typ;
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
    FILE *	H1p = 0;			/* list _list_.c + header _list1.h */
    FILE *	H2p = 0;			/* list header _list2.h */
    char *	cp;				/* auxiliary char pointer */
    char *	module;				/* module name built from path/name */
    unsigned	linecnt = 1;
    int		fflag;
    int		idx;
    int		ext;
    char	tail2[8];			/* compiler generated suffix _123456 max */

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
		ftyp = sp->ftype;
		if (typ == INPX) {
		    assert(ftyp == GATE || ftyp == ARITH);
		    assert(sp->u_blist == 0);
		}
		if (ftyp == ARITH) {
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
		if (ftyp == GATE) {
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
		if (ftyp == OUTX) {
		    assert(typ == OR);
		} else
		if (ftyp == TIMR &&
		    (lp = sp->list) != 0 &&
		    (tsp = lp->le_sym) != 0 &&
		    tsp->u_val < lp->le_val) {
		    tsp->u_val = lp->le_val;	/* store timer preset off value */
		}				/* temporarily in u (which is 0) */
		assert(ftyp != OUTW || sp->list == 0);	/* #define no longer in use */
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
		} else if (sp->ftype != ARITH && sp->ftype != GATE &&
		    sp->ftype != CLCKL && sp->ftype != TIMRL) {
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
"); linecnt += 1;
    if (functionUse[0] & F_CALLED) {		/* has any function been called ? */
	if (functionUse[0] & F_ARITHM) {
	    fprintf(Fp, "\
#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new\n\
");	    linecnt += 1;
	}
	if (functionUse[0] & F_FFEXPR) {
	    fprintf(Fp, "\
#define iC_AV(n)	iC_gf->gt_list[n]->gt_new\n\
#define iC_LV(n)	(iC_gf->gt_list[n]->gt_val < 0 ? 1 : 0)\n\
#define iC_AA(n,v)	iC_assignA(iC_gf->gt_list[n], v)\n\
#define iC_LA(n,v)	iC_assignL(iC_gf->gt_list[n], v)\n\
");	    linecnt += 4;
	}
	if (functionUse[0] & F_LITERAL) {
	    fprintf(Fp, "\
#define iC_AVL(n)	_f0_1.gt_list[n]->gt_new\n\
#define iC_LVL(n)	(_f0_1.gt_list[n]->gt_val < 0 ? 1 : 0)\n\
#define iC_AAL(n,v)	iC_assignA(_f0_1.gt_list[n], v)\n\
#define iC_LAL(n,v)	iC_assignL(_f0_1.gt_list[n], v)\n\
");	    linecnt += 4;
	}
    }
    fprintf(Fp, "\
extern iC_Gt *	iC_l_[];\n\
"); linecnt += 1;
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
"); linecnt += 7;

    li = 0;
    nxs = "0";					/* 0 terminator for linked gate list */
    sam = "";					/* no & for terminator in linked Gate list */
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((typ = sp->type) > UDF && typ < MAX_OP && /* leave out EXT_TYPES */
		sp != iclock) {
		mask = 0;
		/********************************************************************
		 * mN() sets cnt, iqt, xbwl, byte, bit and tail via IEC1131() as side effect
		 *******************************************************************/
		modName = mN(sp);		/* modified string, bit is used in block */
		if (typ == NCONST) {
		    if (strcmp(modName, ICONST) == 0) {
			assert(sp->u_val);	/* unused iConst was deleted from ST */
			fprintf(Fp, "extern iC_Gt %s; /* %d */\n", modName, sp->u_val);
			continue;
		    }
		    /* other NCONST Gates must be static because same constant */
		    /* may be used in several linked modules - not extern */
		    fprintf(Fp, "static iC_Gt %-7s", modName);
		} else if (sscanf(modName, "_f%d_%d%7s", &idx, &ext, tail2) == 2) {
		    /* _fx.y if else switch Gates must be static because they are */
		    /* generated and may be used in several linked modules - not extern */
		    fprintf(Fp, "static iC_Gt %-7s", modName);
		} else {
		    fprintf(Fp, "iC_Gt %-8s", modName);
		}
		fprintf(Fp, " = { 1, -%s,", iC_ext_type[typ]);	/* -gt_ini */
		if ((lp = sp->list) != 0 && lp->le_sym == sp) {
		    fflag = 1;			/* leave out _f0_1 */
		    /* generate gt_fni (ftype), gt_mcnt (1) and gt_ids */
		    fprintf(Fp, " %s, 1, \"%s\",",
			iC_ext_ftype[ftyp = sp->ftype], sp->name);
		} else {
		    fflag = 0;
		    /* generate gt_fni (ftype), gt_mcnt (0) and gt_ids */
		    fprintf(Fp, " %s, 0, \"%s\",",
			iC_ext_ftype[ftyp = sp->ftype], sp->name);
		}
		/* generate gt_list */
		if (ftyp >= MIN_ACT && ftyp < MAX_ACT) {
		    /* gt_list */
		    fprintf(Fp, " &iC_l_[%d],", li);
		    if (fflag == 0) {		/* leave out _f0_1 */
			li += 2;		/* space for action or function pointer + clock */
			if ((lp = lp->le_next) != 0 &&
			    (tsp = lp->le_sym) != 0 &&
			    (tsp->type & ~EM) == TIM) {
			    li++;		/* space for pointer to delay time Gate */
			    lp = lp->le_next;	/* point to delay time */
			}
		    }
		    if (lp == 0) {		/* lp points to clock or delay */
			snprintf(errorBuf, sizeof errorBuf,
			    "Action gate '%s' has no valid clock",
			    sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
		    } else
		    if (ftyp == F_SW || ftyp == F_CF || ftyp == F_CE) {
			/* Function Pointer for "if" or "switch" */
			lc = li;
			while ((lp = lp->le_next) != 0) {
			    li++;		/* count C var on list */
			}
			lc -= li;		/* negative count of C vars */
			li += (USE_COUNT - 1 - lc) / USE_COUNT;	/* # of words extra */
		    }
		} else
		if (ftyp == OUTW) {
		    if (iqt[0] == 'Q' &&	/* QB0_0 is cnt == 3 (no tail) */
			xbwl[0] != 'X' &&	/* can only be 'B', 'W' or 'L' */
			cnt == 3) {
			fprintf(Fp, " 0,");
		    } else {
			goto OutErr;
		    }
		} else
		if (ftyp == OUTX) {
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
		    if (ftyp == TIMRL) {
			if (sp->u_val > 0) {
			    mask = sp->u_val;
			    sp->u_val = 0;	/* restore temporary u to 0 */
			}
		    }
		}
		/* generate gt_rlist */
		if (typ == ARN || (typ >= MIN_GT && typ < MAX_GT)) {
		    fprintf(Fp, " &iC_l_[%d],", li);	/* gt_rlist */
		    if (fflag == 0) {		/* leave dummy pointer for _f0_1 in gt_rlist */
			for (lp = sp->u_blist; lp; lp = lp->le_next) {
			    li++;			/* space in input list */
			}
			/* space for dual GATE list or ARITH with FUNCTION */
			li += 2;
		    }
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
		((ftyp = sp->ftype) == GATE ||
		(iC_Aflag && (ftyp == ARITH || ftyp == CLCKL || ftyp == TIMRL)))) {
		modName = mN(sp);		/* modified string, byte and bit */
		fprintf(Fp, "iC_Gt %-8s", modName);	/* ZZZZ */
		val = sp->list->le_val;
		tsp = sp->list->le_sym;
		while (tsp->type == ALIAS) {
		    val ^= tsp->list->le_val;	/* negate if necessary */
		    tsp = tsp->list->le_sym;	/* point to original */
		}
		fprintf(Fp,
		    " = { 1, -%s, %s, 0, \"%s\", 0, (iC_Gt**)&%s, %s%s, %d };\n",
		    iC_ext_type[typ], iC_ext_ftype[ftyp], sp->name, mN(tsp), sam, nxs, val);
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
	assert(H1p && H2p);
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
"); linecnt += 1;

    if (functionUse[0] & F_CALLED) {		/* has any function been called ? */
	fprintf(Fp, "\
/********************************************************************\n\
 *\n\
 *	Literal blocks and embedded C fragment functions\n\
 *\n\
 *******************************************************************/\n\
\n\
");	linecnt += 6;

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
"); linecnt += 6;
    if (iC_Pflag) {
	fprintf(Fp, "\
#ifdef __GNUC__\n\
__extension__		/* suppress -pedantic warning: static follows non-static */\n\
#endif\n\
");	linecnt += 3;
    }
    fprintf(Fp, "\
static iC_Gt *	iC_l_[] = {\n\
"); linecnt += 1;

    lc = 0;					/* count links */
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((typ = sp->type) == ARN ||	/* leave out UDF, ARNC, LOGC */
		(typ >= MIN_GT && typ < MAX_GT)) {	/* leave out EXT_TYPES */
		int		len = 16;
		char *	fs = strlen(sp->name) > 1 ? "\t" : "\t\t";

		fflag = 0;
		fprintf(Fp, "/* %s */", sp->name);
		if ((ftyp = sp->ftype) >= MIN_ACT && ftyp < MAX_ACT) {
		    if ((lp = sp->list) == 0) {
			snprintf(errorBuf, sizeof errorBuf,
			    "Action gate '%s' has no action list",
			    sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
		    } else {
			if (lp->le_sym == sp) {
			    fflag = 2;		/* leave out _f0_1 */
			} else
			if (lp->le_sym == 0) {	/* ftyp == F_SW, F_CF or F_CE */
			    assert(ftyp == F_SW || ftyp == F_CF || ftyp == F_CE);
			    /* Function Pointer for "if" or "switch" */
			    fflag = 1;
			    len += 13;		/* assume len of %d is 2 */
			    if (len > 73) {
				fs = "\n\t\t";
				linecnt++;
				len = 16 + 13;
			    }
			    fprintf(Fp, "%s(iC_Gt*)iC_%d,", fs, lp->le_val >> 8);
			    fs = " ";
			    lc++;
			} else {
			    len += strlen((tsp = lp->le_sym)->name) + 3;
			    fprintf(Fp, "%s&%s,",
				fs, mN(tsp));	/* action */
			    fs = " ";
			    lc++;
			}
			if (fflag != 2) {	/* not _f0_1 */
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
				fprintf(Fp, "%s&%s,", fs, mN(tsp));	/* clock or timer */
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
			}
			if (fflag) {		/* if else or switch or _f0_1 */
			    /********************************************************************
			     *  Scan the imm C variable list generated by the C compile phase
			     *  either as part of an if else or switch C fragment (_f1_1 ...)
			     *  or as C statements in a literal block (_f0_1).
			     *  Two use bits are set at the top end of le_val in the link for
			     *  each imm variable found by the C compiler (USE_MASK).
			     *  VAR_USE marks, that the variable is used as a value in an
			     *  expression (marked by v in listing).
			     *  VAR_ASSIGN marks, that the variable is assigned to in a
			     *  statement (marked by = in listing). Care is taken, that the
			     *  second or later assigned variable in a multiple assignment is
			     *  also marked as a value. Operator assignments and pre/post-
			     *  inc/dec operations mark both bits.
			     *  In the following the use bit pairs are grouped into words,
			     *  as many as will fit and these words are then appended to the
			     *  list of variables. The load routine collects the bits for
			     *  each variable and can thereby determine, if any variable has
			     *  been assigned somewhere (input) and has been used somewhere as
			     *  a value (output). This was always done for immdiate statements.
			     *  This is now done for immediate variables used in C code, which
			     *  makes the usage analysis complete. No analysis of C variables
			     *  is done - here a correct algorithmic design and the interpre-
			     *  tation of the final C compiler warnings, as in any C code
			     *  is called for.
			     *******************************************************************/
			    int	uc = 0;
			    tlp = lp;		/* do not modify lp yet */
			    while ((tlp = tlp->le_next) != 0) {
				uc++;		/* count C vars on list */
			    }
			    if (uc) {
				Gate **	gpp;
				int	useWord;
				int	useBits;
				int	i;
				int	i1;
				int	j;

				uc = (USE_COUNT - 1 + uc) / USE_COUNT;	/* # of words extra */
				gpp = (Gate **)calloc(uc, sizeof(Gate *));	/* use words */
				j = i = useWord = 0;
				while ((lp = lp->le_next) != 0) {	/* scan C var list */
				    tsp = lp->le_sym;
				    useBits = lp->le_val >> USE_OFFSET;	/* use bits */
				    assert(useBits > 0 && useBits < 4);
				    useWord |= useBits << i;
				    i += 2;
				    if (i >= (USE_COUNT << 1)) {
					assert(j < uc);
					gpp[j++] = (Gate *)useWord;
					i = useWord = 0;
				    }
				    while (tsp->type == ALIAS) {
					tsp = tsp->list->le_sym;	/* point to original */
				    }
				    len += strlen(tsp->name) + 3;
				    if (len > 73) {
					fs = "\n\t\t";
					linecnt++;
					len = 16 + strlen(tsp->name) + 3;
				    }
				    fprintf(Fp, "%s&%s,", fs, mN(tsp));
				    fs = " ";
				    lc++;
				}
				if (i) {
				    gpp[j++] = (Gate *)useWord;
				    assert(j == uc);
				} else {
				    i = USE_COUNT << 1;
				}
				i1 = 12 + ((USE_COUNT + 1) >> 1);
				for (--uc, j = 0; j <= uc; j++) {
				    if (j == uc) i1 = 12 + ((i + 2) >> 2);	/* # of digits in last mask */
				    len += i1;
				    if (len > 73) {
					fs = "\n\t\t";
					linecnt++;
					len = 16 + i1;
				    }
				    fprintf(Fp, "%s(iC_Gt*)%p,", fs, gpp[j]);	/* write use bits */
				    fs = " ";
				    lc++;
				}
				free(gpp);		/* free the temporary array */
			    }
			}
			len += 8; len &= 0xfff8;
			fs = "\t";
			if (len > 48) {
			    len = 96;			/* force line break if continuation */
			}
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
			len += 13;		/* assume len of %d is 2 */
			if (len > 73) {
			    fs = "\n\t\t";
			    linecnt++;
			    len = 16 + 13;
			}
			if (lp->le_val) {
			    fprintf(Fp, "%s(iC_Gt*)iC_%d,", fs, lp->le_val >> 8);
			} else {
			    fprintf(Fp, "%s(iC_Gt*)0,", fs);	/* OUTW */
			}
			fs = " ";
			lc++;
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
			fs = " ";
			lc++;
		    }
		    len += 3;
		    fprintf(Fp, "%s0,", fs);
		    fs = " ";
		    lc++;
		} else
		if ((typ >= MIN_GT && typ < MAX_GT) && fflag != 2) {
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
				fs = " ";
				lc++;
			    }
			}
			len += 3;
			if (len > 76) {
			    fs = "\n\t\t";
			    linecnt++;
			    len = 16 + 3;
			}
			fprintf(Fp, "%s0,", fs);
			fs = " ";
			lc++;
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
		assert(H1p);
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
	assert(H2p);
	fclose(H2p);			/* close list header in case other actions */
endd:
	assert(H1p);
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

#ifdef PRECOMPILE_C_OUTPUT
    if(iFP != T4FP && strlen(iC_Cdefines)) {
	if (precompileFlag == 0) {		/* may only be used in final C-compile */
	    if ((c = openT4T5(0)) != 0) return c;	/* re-open if necessary */
	}
	precompileFlag |= 2;			/* 2 marks -D, #define or #if */
    }
#endif
    while (fgets(lineBuf, sizeof lineBuf, iFP)) {
	if (strcmp(lineBuf, "%{\n") == 0) {
	    mask = 01;				/* copy literal blocks */
	} else if (strcmp(lineBuf, "%}\n") == 0) {
	    mask = 02;				/* copy functions or cases */
	} else if (mode & mask) {		/* seperates literal blocks and functions */
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
		 *  lp now points to the beginning of a line past % but to #
		 *  handle pre-processor #include <stdio.h> or "icc.h"
		 ********************************************************/
		if (precompileFlag <= 1 &&
		    sscanf(lp, " # include %[<\"/A-Za-z_.0-9>]", lstBuf) == 1) {
		    if(iFP == T4FP) {
			ierror("copyBlocks: if else or switch has:", lstBuf);
			continue;
		    }
		    if (precompileFlag == 0) {
			if ((c = openT4T5(0)) != 0) return c;	/* re-open if necessary */
		    }
		    if (iC_debug & 02) fprintf(iC_outFP, "####### c_parse #include %s\n", lstBuf);
		    fprintf(T4FP, "#include %s\n", lstBuf);	/* a little C file !!! */
		    precompileFlag |= 1;	/* 1 marks #include */
		}
#ifdef PRECOMPILE_C_OUTPUT
		/********************************************************
		 *  Handle pre-processor #define and #if
		 *  No need to scan for #else or #endif; if they occurr
		 *  there must have been a #if - unless serious C error.
		 *  Other preprocessor directives do not affect outcome.
		 ********************************************************/
		else if (sscanf(lp, " # define %[A-Za-z_0-9]", lstBuf) == 1 ||
		    sscanf(lp, " # if %s", lstBuf) == 1 ||
		    sscanf(lp, " # ifdef %s", lstBuf) == 1 ||
		    sscanf(lp, " # ifndef %s", lstBuf) == 1) {
		    if(iFP == T4FP) {
			ierror("copyBlocks: if else or switch has:", lstBuf);
			continue;
		    }
		    if (precompileFlag == 0) {
			if ((c = openT4T5(0)) != 0) return c;	/* re-open if necessary */
		    }
		    precompileFlag |= 2;	/* 2 marks -D, #define or #if */
		}
#endif
	    }
	}
    }
    return 0;
} /* copyBlocks */

/********************************************************************
 *
 *	To handle immediate variables in C code, the output of the first
 *	two passes of copyBlocks is separated by a line containing
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
    char	execBuf[BUFS];			/* can be smaller than a line */
    char	lineBuf[BUFS];

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
	fprintf(T2FP, "/*##*/\n");		/* -o option - separate blocks */
    }
    if (copyBlocks(iFP, T2FP, 02)) {
	return T1index;
    }
    if (outFlag == 0) {
	fprintf(T2FP, "/*##*/}}\n");
    }
#ifdef PRECOMPILE_C_OUTPUT
    fclose (T2FP);				/* close intermediate file */
#endif
    if (precompileFlag == 1) {
	/********************************************************
	 *  pre-process and C-parse #include <stdio.h> and "icc.h" etc
	 *  add CTYPE's as symbols to iC symbol table for main C-parse
	 ********************************************************/
	fflush(T4FP);
	/* Cygnus does not understand cc - use macro CC=gcc */
	snprintf(execBuf, BUFS, SS(CC) " -E -I/usr/local/include -x c %s -o %s 2> %s",
	    T4FN, T5FN, T6FN);
	r = system(execBuf);			/* Pre-compile C header file(s) */
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### pre-compile: %s; $? = %d\n", execBuf, r>>8);
#endif
	if (r != 0 || (T5FP = fopen(T5FN, "r")) == NULL) {
	    if ((T6FP = fopen(T6FN, "r")) == NULL) {
		return T6index;			/* error opening CC error file */
	    }
	    while (fgets(lineBuf, sizeof lineBuf, T6FP)) {
		ierror(SS(CC) ":", lineBuf);	/* CC error message */
	    }
	    fclose(T6FP);
	    if (!(iC_debug & 04000)) {
		unlink(T6FN);
	    }
	    ierror("c_compile: cannot open:", T5FN);
	    perror("open");
	    return T5index;
	}
	if (!(iC_debug & 04000)) {
	    unlink(T6FN);
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
#ifdef PRECOMPILE_C_OUTPUT
    /* The listing produced is not complete - functionality is doubtful */
    else if (precompileFlag > 1) {
	/********************************************************
	 *  Alternatively
	 *  pre-process generated C code with # define and/or #if
	 *  or if iC_Cdefines contains -C or -V definitions
	 ********************************************************/
	int	r1;
	/* Cygnus does not understand cc - use macro CC=gcc - pass comments with -C */
	snprintf(execBuf, BUFS, SS(CC) "%s -E -C -I/usr/local/include -x c %s -o %s 2> %s",
	    iC_Cdefines, T2FN, T5FN, T6FN);
	r1 = system(execBuf);			/* Pre-compile generated C file */
#if YYDEBUG
	if ((iC_debug & 0402) == 0402) fprintf(iC_outFP, "####### pre-compile: %s; $? = %d\n", execBuf, r1>>8);
#endif
	if (r1 != 0) {
	    if ((T6FP = fopen(T6FN, "r")) == NULL) {
		return T6index;			/* error opening CC error file */
	    }
	    while (fgets(lineBuf, sizeof lineBuf, T6FP)) {
		ierror(SS(CC) ":", lineBuf);	/* CC error message */
	    }
	    fclose(T6FP);
	    if (!(iC_debug & 04000)) {
		unlink(T6FN);
	    }
	    ierror("compile: cannot run:", execBuf);
	    return T0index;
	}
	if (!(iC_debug & 04000)) {
	    unlink(T6FN);
	}
	if ((T2FP = fopen(T5FN, "r")) == NULL) {
	    return T2index;			/* error opening intermediate file */
	}
    }
    else if ((T2FP = fopen(T2FN, "r")) == NULL) {
	return T2index;				/* error opening input file */
    }
#else
    if (fseek(T2FP, 0L, SEEK_SET) != 0) {	/* rewind intermediate file */
	return T2index;
    }
#endif
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

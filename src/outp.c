static const char outp_c[] =
"@(#)$Id: outp.c,v 1.46 2002/06/26 19:42:58 jw Exp $";
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
#include	"icc.h"
#include	"comp.h"

extern const char	SC_ID[];

/********************************************************************
 *
 *	IEC1131() modifies IEC-1131 bit names in the buffer buf
 *	The routine returns a count of fields found.
 *	If count is less the 4 the unmodified name is returned in buf
 *	The count can be used to charecterize IBx, QBx etc fields also
 *
 *	bufLen should be >= 21 (sscanf format length)
 *
 *******************************************************************/

int
IEC1131(char * name, char * buf, int bufLen,
	char * iqt, char * bwx, int * bytep, int * bitp, char * tail)
{
    int count;

    assert(bufLen >= 21);
    if (outFlag && (count = sscanf(name, "%1[IQT]%1[BWX]%5d.%5d%7s",
				iqt, bwx, bytep, bitp, tail)) >= 4) {
	sprintf(buf, "%s%s%d__%d%s", iqt, bwx, *bytep, *bitp, tail);
    } else {
	strncpy(buf, name, bufLen - 1);
    }
    return count;
} /* IEC1131 */

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
 *	a 3rd call to mN(). Used in loops to provide previous name.
 *
 *******************************************************************/

#define SZ	4
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
    iqt[0] = bwx[0] = byte = bit = 0;		/* clear for later check */
    if (sp == 0) return strncpy(np, "0", 2);	/* in case of suprises */
    tail[0] = 0;
    cnt = IEC1131(sp->name, np, BUFS, iqt, bwx, &byte, &bit, tail);
    return np;
} /* mN */

unsigned short	Aflag;			/* -A flag signals ARITH alias */
unsigned short	aflag;			/* -a on compile to append output */

static unsigned	block_total;		/* shared by listNet and buildNet */
static unsigned	link_count;		/* shared by listNet and buildNet */
static int	extFlag;		/* set if extern has been recognised */

/********************************************************************
 *
 *	errorEmit outputs an error line in the generated code
 *	and an error message
 *
 *******************************************************************/

static char	errorBuf[256];	/* used for error lines in emitting code */

void
errorEmit(FILE* Fp, char* errorMsg)
{
    fprintf(Fp, "/* error in emitting code. %s */\n", errorMsg);
    errmess("ErrorEmit", errorMsg, NS);	/* error sets iClock->type to ERR */
} /* errorEmit */

/********************************************************************
 *
 *	list the generated network
 *	list network statistics
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
    if (debug & 020) fprintf(outFP, "\nNET TOPOLOGY\n\n");
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
			if (sp->ftype == F_SW || sp->ftype == F_CF) {
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
		    error("gate:", sp->name);
		}
	    }
	}
    }

    if (debug & 040) {
	byte_total = (long)block_total * sizeof(Gate)
		   + (long)link_count * sizeof(Gate *);
	fprintf(outFP, "\nNET STATISTICS\n\n");
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
	error(errBuf, NS);
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
		    warning("INPUT configuration too small for:", sp->name);
				    /* ZZZ set some error bit to stop exec */
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
				    /* F_SW or F_CF action gate points to function */
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
		    warning("OUTPUT configuration too small for:", gp->gt_ids);
				    gp->gt_list = (Gate**)0;
				    gp->gt_mark = 0;	/* no change in bits */
				}
			    } else {
		    warning("OUTPUT strange ftype:", gp->gt_ids);
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
    int		li;
    FILE *	Fp;
    FILE *	Hp;		/* list _list_.c + header _list1.h */
    FILE *	Lp;		/* list header _list2.h */
    char *	s1;
    char *	module;
    unsigned	linecnt;
    int		aliasArithFlag = 0;

    /* open output file */

    if ((Fp = fopen(outfile, "w")) == 0) {
	rc = 4; goto end;
    }

    if ((Lp = fopen(Lname, aflag ? "a" : "w")) == 0) {	/* list header _list2.h */
	rc = 10; goto endl;
    }

    if (aflag == 0) {
	fprintf(Lp, "#define	I_LIST\\\n");		/* list header _list2.h */

	/* write _list_.c once, so that it is locally present */
	if ((Hp = fopen(Cname, "w")) == 0) {		/* list _list_.c */
	    rc = 8; goto endh;
	}
	fprintf(Hp, "\
#include	<stdio.h>\n\
#include	<icc.h>\n\
#include	\"%s\"\n\
#include	\"%s\"\n\
Gate **		i_list[] = { I_LIST 0 };\n\
", Hname, Lname);					/* list _list_.c */
	fclose(Hp);					/* list _list_.c */
    }

    if ((Hp = fopen(Hname, aflag ? "a" : "w")) == 0) {	/* header _list1.h */
	rc = 9; goto endh;
    }

    /* rewind intermediate file */

    if (fseek(exoFP, 0L, SEEK_SET) != 0) {
	rc = 7; goto endm;
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
#include	<stdio.h>\n\
#include	<icc.h>\n\
#include	\"%s\"\n\
\n\
#ifdef ALIAS_ARITH\n\
#define _(x) (x.gt_ini==-ALIAS?((Gate*)x.gt_rlist)->gt_old:x.gt_old)\n\
#else\n\
#define _(x) x.gt_old\n\
#endif\n\
#define A(x,v) assign(&x, v)\n\
extern Gate *	l_[];\n\
", inpNM, outfile, SC_ID, Hname);
linecnt = 21;

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
		    fprintf(Hp, "#define %s	%s\n", 	/* header _list1.h *//* ZZZ no longer in use */
			sp->name, lp->le_sym->name); /* back link to Hname */
			/* important to use unmodified names here */
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
			fprintf(Fp,
    "/* error in emitting code. Simple gate '%s' on clock list '%s' */\n",
			    tsp->name, sp->name);
			linecnt++;
			break;
		    }
		    if ((tlp = tsp->list) == 0) {
			fprintf(Fp,
    "/* error in emitting code. Action gate '%s' on clock list '%s' has no output */\n",
			    tsp->name, sp->name);
			linecnt++;
			break;
		    }
		    if (tlp->le_next) {
			fprintf(Fp,
    "/* error in emitting code. Action gate '%s' on clock list '%s' has more than 1 output */\n",
			    tsp->name, sp->name);
			linecnt++;
			break;
		    }
		    tlp->le_next = lp;
		    sp->list = lp->le_next;
		    lp->le_sym = sp;
		    lp->le_next = 0;
		}
	    } else if (typ == ALIAS) {
		if ((lp = sp->list) == 0) {
		    fprintf(Fp,
    "/* error in emitting code. Alias '%s' has no output */\n",
			sp->name);
		    linecnt++;
		} else if (lp->le_next) {
		    fprintf(Fp,
    "/* error in emitting code. Alias '%s' has more than 1 output */\n",
			sp->name);
		    linecnt++;
		} else if (sp->ftype == ARITH) {
		    if (aliasArithFlag == 0) {
			fprintf(Hp, "#define ALIAS_ARITH\n");	/* header _list1.h */
			aliasArithFlag = 1;
		    }
		} else if (sp->ftype != GATE && sp->ftype != CLCKL && sp->ftype != TIMRL) {
		    sprintf(errorBuf,
			"Alias '%s' has wrong ftype %s",
			sp->name, full_ftype[sp->ftype]);
		    errorEmit(Fp, errorBuf);
		    linecnt++;
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
			    fprintf(Fp,
    "/* error in emitting code. Simple gate '%s' controlled by timer '%s' */\n",
				tsp->name, sp->name);
			    linecnt++;
			    break;
			}
			if ((tlp = tsp->list) == 0) {
			    fprintf(Fp,
    "/* error in emitting code. Action gate '%s' controlled by timer '%s' has no output */\n",
				tsp->name, sp->name);
			    linecnt++;
			    break;
			}
			if ((tlp = tlp->le_next) == 0) {
			    fprintf(Fp,
    "/* error in emitting code. Action gate '%s' controlled by timer '%s' has no clock */\n",
				tsp->name, sp->name);
			    linecnt++;
			    break;
			}
			if (tlp->le_next) {
			    fprintf(Fp,
    "/* error in emitting code. Action gate '%s' controlled by timer '%s' has more than output and clock */\n",
				tsp->name, sp->name);
			    linecnt++;
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
    nxs = "0";
    sam = "";
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
		    if (sp->ftype == OUTW) {
			assert(typ != NCONST);
			fprintf(Fp, "Gate _%-7s", modName); /* modify */
		    } else if (typ == NCONST) {
			/* NCONST Gate must be static because same constant */
			/* may be used in several linked modules - not extern */
			fprintf(Fp, "static Gate _%-7s", modName); /* modify */
		    } else {
			fprintf(Fp, "Gate %-8s", modName);
		    }
		    fprintf(Fp, " = { 1, -%s,", full_type[typ]);
		}
		fprintf(Fp, " %s, 0, \"%s\",",
		    full_ftype[dc = sp->ftype], sp->name);
		if (dc >= MIN_ACT && dc < MAX_ACT) {
		    /* gt_list */
		    fprintf(Fp, " &l_[%d],", li);
		    /* space for action or function pointer + clock */
		    li += 2;
		    if ((lp = sp->list->le_next) == 0) {
			fprintf(Fp,
    "/* error in emitting code. Action gate '%s' has no clock */\n",
			    sp->name);	/* ZZZ wrong format */
		    } else if ((lp->le_sym->type & TM) == TIM) {
			li++;	/* space for pointer to time value Gate */
		    }
		} else if (dc == OUTW) {
		    if (iqt[0] == 'Q' &&
			bwx[0] != 'X' &&	/* can only be B or W */
			cnt == 3 && byte < IXD) {
			fprintf(Fp, " (Gate**)%d,", byte);
			mask = bwx[0] == 'B' ? B_WIDTH : W_WIDTH;
		    } else {
			goto OutErr;
		    }
		} else if (dc == OUTX) {
		    if (iqt[0] == 'Q' &&
			bwx[0] == 'X' &&
			cnt == 5 && byte < IXD && bit < 8) {
			/* OUTPUT byte part of bit pointer */
			fprintf(Fp, " (Gate**)%d,", byte);
			mask = bitMask[bit];
		    } else {
		    OutErr:
			fprintf(Fp, " (Gate**)0,\n"
    "/* error in emitting code. OUT configuration error %s */\n", sp->name);
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
		    fprintf(Fp, " &l_[%d],", li);	/* gt_rlist */
		    for (lp = sp->u.blist; lp; lp = lp->le_next) {
			li++;	/* space in input list */
		    }
		    /* space for dual GATE list or ARITH with FUNCTION */
		    li += 2;
		} else if (typ == INPW) {
		    if (iqt[0] == 'I' &&
			bwx[0] != 'X' &&	/* can only be B or W */
			cnt == 3 && byte < IXD) {
			fprintf(Fp, " &I%s_[%d],", bwx, byte);
		    } else {
			goto InErr;
		    }
		} else if (typ == INPX) {
		    if (iqt[0] != 'Q' &&	/* can only be I or T */
			bwx[0] == 'X' &&
			cnt == 4 && byte < IXD && bit < 8) {
			fprintf(Fp, " &%sX_[%d],", iqt, byte * 8 + bit);
		    } else {
		    InErr:
			fprintf(Fp, " 0,\n"
    "/* error in emitting code. IN configuration error %s */\n", sp->name);
			linecnt += 2;
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
		sam = (dc == OUTW || typ == NCONST) ? "&_" : "&";
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
		" = { 1, -%s, %s, 0, \"%s\", 0, (Gate**)&%s%s, %s%s, %d };\n",
		full_type[typ], full_ftype[dc], sp->name,
		(gp->ftype == OUTW || gp->type == NCONST) ? "_" : "",
		mN(gp), sam, nxs, val);
		linecnt++;
		nxs = modName;		/* previous Symbol name */
		sam = "&";		/* ZZZZ */
	    }
	}
    }

    module = strdup(inpNM);		/* source file name */
    if ((s1 = strrchr(module, '.')) != 0) {
	*s1 = '\0';			/* module name without extensions */
    }
    /*
     * The following algorithm fails if two files are linked with names
     * 'ab.cd.ic' and 'ab_cd.ic' - by Goedel there is hardly a way out of this.
     * A multiple define error occurs for the name 'ab_cd_i_list' at link time.
     * The same error occurs if unfortunate path combinations are used
     * eg: ab/cd.ic and either of the above
     */
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
    fprintf(Hp, "extern Gate *	%s_i_list;\n", module);	/* header _list1.h */
    fprintf(Lp, "	&%s_i_list,\\\n", module);	/* list header _list2.h */
    free(module);

    fprintf(Fp, "\n\
/********************************************************************\n\
 *\n\
 *	Embedded C fragment functions\n\
 *\n\
 *******************************************************************/\n\
\n");
    linecnt += 9;		/* includes 2 lines at #line */

    /* copy C intermediate file up to EOF to C output file */
    /* translate any ALIAS references of type '_(QB1_0)' */

    copyXlate(exoFP, Fp, &linecnt, 01);		/* copy literal blocks */

    /* rewind intermediate file */
    if (fseek(exoFP, 0L, SEEK_SET) != 0) {
	rc = 7; goto endm;
    }

    copyXlate(exoFP, Fp, &linecnt, 02);		/* copy functions */

/********************************************************************
 *
 *	produce initialised connection lists
 *	using modified symbol table
 *
 *******************************************************************/

    if (li == 0) goto endm;	/* MS-C does not hack 0 length array - gcc does */
    fprintf(Fp, "\
#line %d \"%s\"\n\
/********************************************************************\n\
 *\n\
 *	Connection lists\n\
 *\n\
 *******************************************************************/\n\
\n\
static Gate *	l_[] = {\n", linecnt, outfile);

    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if ((typ = sp->type) > UDF && typ < MAX_GT) { /* leave aout EXT_TYPES */
		int		len = 16;
		char *	fs = strlen(sp->name) > 1 ? "\t" : "\t\t";

		fprintf(Fp, "/* %s */", sp->name);
		if ((dc = sp->ftype) >= MIN_ACT && dc < MAX_ACT) {
		    if ((lp = sp->list) == 0) {
			fprintf(Fp,
    "/* error in emitting code. action gate '%s' has no action list */\n",
			    sp->name);
		    } else {
			if (lp->le_sym == 0) {	/* dc == F_SW or F_CF */
			    /* Function Pointer for "if" or "switch" */
			    len += 17;	/* assume len of %d is 2 */
			    fprintf(Fp, "%s(Gate*)cexe_%d,",
				fs, lp->le_val);
			} else {
			    len += strlen((tsp = lp->le_sym)->name) + 3;
			    fprintf(Fp, "%s&%s,",
				fs, mN(tsp));		/* action */
			}
			fs = " ";
			if ((lp = lp->le_next) != 0) {
			    /* error message already in last loop */
			    len += strlen((tsp = lp->le_sym)->name) + 3;
			    fprintf(Fp, "%s&%s,",
				fs, mN(tsp));		/* clock or timer */
			    if ((tsp->type & TM) == TIM && (lp = lp->le_next) != 0) {
				/* error message already in last loop */
				len += strlen((tsp = lp->le_sym)->name) + 3;
				fprintf(Fp, "%s&%s%s,",	/* time value */
				    fs, tsp->type == NCONST ? "_" : "", mN(tsp));
			    }
			}
			fs = "\t";
			len += 8; len &= 0xfff8;
		    }
		}

		val = 0;
		if (typ == ARN) {
		    if ((lp = sp->u.blist) == 0) {
			fprintf(Fp,
    "/* error in emitting code. ARITHMETIC gate '%s' has no function */\n",
			    sp->name);
		    } else {
			/* Function Pointer at start of input list */
			len += 17;	/* assume len of %d is 2 */
			if (lp->le_val) {
			    fprintf(Fp, "%s(Gate*)cexe_%d,", fs, lp->le_val);
			} else {
			    fprintf(Fp, "%s(Gate*)0,", fs);	/* OUTW */
			}
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
				    len = 16 + strlen(lp->le_sym->name) + 3;
				}
				if (lp->le_sym->type == NCONST) {
				    fprintf(Fp,
					"%s&_%s,", fs, mN(lp->le_sym));	/* modify */
				} else {
				    fprintf(Fp,
					"%s&%s,", fs, mN(lp->le_sym));
				}
				fs = " ";
			    }
			}
			fprintf(Fp, "%s0,", fs);
			len += 3;
			fs = " ";
		    } while (val ^= NOT);
		}
		fprintf(Fp, "\n");
	    }
	}
    }
    fprintf(Fp, "};\n");
endm:
    fclose(Hp);	/* close the header file in case other actions */
endh:
    fclose(Lp);	/* close the list header file in case other actions */
endl:
    fclose(Fp);	/* close the C output file in case other actions */
end:
    return rc;		/* return code */
} /* output */

/********************************************************************
 *
 *	copy C intermediate file up to EOF to C output file translate
 *	any ALIAS references of type '_(abcd0)' or '_(QB10_0)'.
 *	Any arithmetic variable name may be an ALIAS and must
 *	be resolved.
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
 *	TODO error generation
 *
 *******************************************************************/

void
copyXlate(FILE * iFP, FILE * oFP, unsigned * lcp, int mode)
{
    int		c;
    int		mask = 02;	/* default is functions or cases */
    int		lf = 0;		/* set by first non white space in a line */
    Symbol *	sp;
    char	lineBuf[256];
    char	buffer[256];
    char *	lp;
    char *	bp;

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
			(*lcp)++;		/* count lines actually output */
			lf = 0;			/* start of a new line */
		    } else if (c != ' ' && c != '\t') {
			lf = 1;			/* not white space */
			if (c == '_') {
			    if ((c = *lp++) == '(') {
				putc(c, oFP);
				/* accept any token which might be in the symbol table */
				bp = buffer;
				while ((c = *lp++) != 0 && (isalnum(c) || c == '_')) {
				    *bp++ = c;
				}
				*bp = '\0';	/* terminate string */
				if (c == ')') {
				    /* token found - xlate it */
				    if ((sp = lookup(buffer)) != 0) {
					while (sp->type == ALIAS) {
					    if (sp->ftype != ARITH) {
						/* generate error */
					    }
					    sp = sp->list->le_sym;	/* resolve ALIAS */
					}
					if (sp->ftype != ARITH) {
					    /* CHECK may be type ARN or SH */
					    /* generate error */
					}
					fputs(sp->name, oFP);
				    } else {
					/* generate error - symbol should at least be in table */
					fputs(buffer, oFP);
				    }
				} else {
				    fputs(buffer, oFP);	/* strange but true */
				}
			    }
			    putc(c, oFP);	/* char after '_' or buffer */
			}
		    }
		}
	    }
	}
    }
} /* copyXlate */

static const char outp_c[] =
"@(#)$Id: outp.c,v 1.16 2000/12/21 21:19:00 jw Exp $";
/* parallel plc - output code or run machine */

/* J.E. Wulff	24-April-89 */

/* "outp.c	1.45	95/02/14" */

#ifdef _WINDOWS
#include <windows.h>
#endif
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	"pplc.h"
#include	"comp.h"

#define Hname	"inc1.h"
#define Lname	"inc2.h"

extern const char	SC_ID[];

unsigned short	bitmask[] = {
    0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000, 0x8000,
};

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
 *	Because names[] has 2 static arrays, used in ping-pong fashion,
 *	a pointer to the current names entry can still be used after
 *	a 2nd call to mN(). Used in loops to provide previous name.
 *
 *******************************************************************/

static char	names[2][128];	/* ping pong buffer for modified names */
static int	ix;		/* index for accessing alternate arrays */
static char	iqt[2];		/* char buffers - space for 0 terminator */
static char	bwx[2];
static unsigned	byte;
static unsigned	bit;
static char	tail[8];	/* compiler generated suffix _123456 max */
static unsigned	cnt;		/* used as side-effect in OUTW - INPX */

static char *
mN(Symbol * sp)
{
    char * np = names[ix ^= 1];			/* alternate ix 1 and 0 */
    iqt[0] = bwx[0] = byte = bit = 0;		/* clear for later check */
    if (sp == 0) return strncpy(np, "0", 2);	/* in case of suprises */
    tail[0] = 0;
    if ((cnt = sscanf(sp->name, "%1[IQT]%1[BWX]%5d.%5d%7s",
	iqt, bwx, &byte, &bit, tail)) >= 4) {
	sprintf(np, "%s%s%d__%d%s", iqt, bwx, byte, bit, tail);
	return np;
    } 
    return strncpy(np, sp->name, 127);
} /* mN */

char *		full_type[] = { FULL_TYPE };
char *		full_ftype[] = { FULL_FTYPE };
uchar		types[] = { TYPES };
uchar		ftypes[] = { FTYPES };
char		os[] = OPS;
char		fos[] = FOPS;
unsigned	gate_count[MAX_LS];	/* accessed by pplc() */
unsigned short	Aflag;			/* -A flag signals ARITH alias */
unsigned short	aflag;			/* -a on compile to append output */
		

int
output(char * outfile)			/* emit code in C */
{
    register Symbol *	sp;
    register List_e *	lp;
    Symbol **		hsp;
    short		dc;
    short		typ;
    unsigned		val;
    unsigned		block_total;
    unsigned		link_count;
    unsigned long	byte_total;
    unsigned		rc;		/* return code */
    unsigned		mask;
    char		bw[2];		/* "B" or "W" */

    byte_total = link_count = block_total = rc = 0;	/* init each time */
    for (typ = 0; typ < MAX_LS; typ++) {
	gate_count[typ] = 0;
    }
    if (debug & 020) fprintf(outFP, "\nNET TOPOLOGY\n");
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if (sp->type < MAX_LS) {
		gate_count[sp->type]++;
		if (sp->type < MAX_OP) {
		    block_total++;
		}
		if (debug & 020) {		/* print directed graph */
		    fprintf(outFP, "\n%s\t%c  %c", sp->name,
			os[sp->type], fos[sp->ftype]);
		    dc = 0;
		}
		for (lp = sp->list; lp; lp = lp->le_next) {
		    if (sp->type < MAX_LV &&	/* don't count back links */
			sp->ftype != OUTX && sp->ftype != OUTW) {
			link_count++;
		    }
		    if (debug & 020) {
			register Symbol * tsp = lp->le_sym;
			if (dc++ >= 8) {
			    dc = 1;
			    fprintf(outFP, "\n\t");
			}
			if (sp->type != TIM &&
			    sp->ftype != F_SW && sp->ftype != F_CF) {
			    fprintf(outFP, "\t%c%s%c",
				(sp->ftype == GATE || sp->ftype == OUTX) &&
				lp->le_val ? '~' : ' ',
				tsp->name, os[tsp->type]);
			} else if (tsp) {
			    fprintf(outFP, "\t %s%c\t(%d)",
				tsp->name, os[tsp->type], lp->le_val);
			    dc++;
			} else {
			    fprintf(outFP, "\t%c (%d)", os[types[sp->ftype]], lp->le_val);
			}
		    }
		}
		if (sp->type < MAX_LV &&	/* don't count outputs */
		    sp->ftype != OUTX && sp->ftype != OUTW) {
		    link_count++;	/* for terminator */
		    /* ZZZ modify later to do this only for */
		    /* ZZZ action gates controlled by a TIMER */
		    if (sp->ftype >= MAX_AR) {
			link_count++;	/* 2nd terminator for inverted */
		    }			/* or time for TIMER action */
		}
	    }
	}
    }
    if (debug & 020) fprintf(outFP, "\n");
    byte_total = (unsigned long)block_total * sizeof(Gate)
	       + (unsigned long)link_count * sizeof(Gate *);

    if (debug & 040) {
	fprintf(outFP, "\nNET STATISTICS\n\n");
	for (typ = 0; typ < MAX_LS; typ++) {
	    if (gate_count[typ]) {
		fprintf(outFP, "%s\t%c %6d%s\n", full_type[typ], os[typ],
		    gate_count[typ], typ < MAX_OP ? " blocks" : "");
	    }
	}
	fprintf(outFP, "\nTOTAL\t%8d blocks\n", block_total);
	fprintf(outFP, "\t%8d links\n", link_count);
	fprintf(outFP, "\t%8ld bytes\n", byte_total);
    }
    if (gate_count[UDF]) {
	warning("undefined gates and functions", NS);
    }
#ifdef _WINDOWS
    if (outFP != stdout) fclose(outFP);	/* this is end of listing output */
#endif
    if (ynerrs || gate_count[ERR]) {
	fprintf(errFP, "%d syntax + %d generate errors - cannot execute\n",
	    ynerrs, gate_count[ERR]);
	rc = 1; goto end;
    }

    if (outfile == 0) {
	register Gate *	gp;
	register Gate **fp;
	Gate *		igp;
	Gate **		ifp;

	/* initialise executable gates */

	igp = gp = (Gate *)calloc(block_total, sizeof(Gate));	/* gates */
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
	if ((val = gp - igp) != block_total) {
	    fprintf(errFP,
		"\n%s: line %d: block error %d vs block_total %d\n",
		__FILE__, __LINE__, val, block_total);
	    rc = 2; goto end;
	}

	gp = igp;				/* repeat to initalise links */
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
					    lp->le_sym->u.gate->gt_rlist =
					    (Gate**)lp->le_val;
					    goto storeLink;
					}
					if (val == lp->le_val) {
					storeLink:
					    *fp++ = lp->le_sym->u.gate;
					}
				    }
				    /* gate list terminator */
				    *fp++ = 0;
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
			    } else if (sp->ftype == OUTW) {
				if (sscanf(gp->gt_ids, "Q%1[BW]%d",
				    bw, &byte) == 2 && byte < IXD) {
				    gp->gt_list = (Gate**)&QX_[byte];
				    gp->gt_mark = bw[0] == 'B' ? 1 : 2;
				} else {
				    goto outErr;
				}
			    } else if (sp->ftype == OUTX) {
				if (sscanf(gp->gt_ids, "QX%d.%d",
				    &byte, &bit) == 2 &&
				    byte < IXD && bit < 16) {
				    /* OUTPUT bit pointer */
				    gp->gt_list = (Gate**)&QX_[byte];
				    gp->gt_mark = bitmask[bit];
				} else {
				outErr:
		    warning("OUTPUT configuration too small for:", gp->gt_ids);
				    gp->gt_list = (Gate**)&QX_[0];
				    gp->gt_mark = 0;	/* no change in bits */
				}
			    } else {
				register Symbol * tsp;	/* D_SH - D_FF */
				/* relies on action gates */
				/* having only one output */
				lp = sp->list;
				tsp = lp->le_sym;
				/* F_SW or F_CF action gate points to function */
				*fp++ = tsp ? tsp->u.gate : (Gate*)lp->le_val;
				/* room for clock or timer entry */
				*fp++ = 0;
				/* room for time value */
				*fp++ = 0;
				/* ZZZ modify later to do this only for */
				/* ZZZ action gates controlled by a TIMER */
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
			     * location of the function list (fl_gate). The
			     * clock reference is put in the second location
			     * which was previously cleared by a terminator.
			     * The 3rd location holds a time value for timers.
			     * All action gates were initialised first, because
			     * typ < MAX_LV were done first.
			     */

			    for (lp = sp->list; lp; lp = lp->le_next) {
				lp->le_sym->u.gate->gt_clk = gp;
				if (typ == TIM) {
				    /* Header is a Timer, store time value */
				    (unsigned int)lp->le_sym->u.gate->gt_time =
					lp->le_val;
				}
			    }
			    gp++;
			}
		    }
		}
	    }
	}
	if ((val = gp - igp) != block_total) {
	    fprintf(errFP,
		"\n%s: line %d: block error %d vs block_total %d\n",
		__FILE__, __LINE__, val, block_total);
	    rc = 2; goto end;
	}
	if ((val = fp - ifp) != link_count) {
	    fprintf(errFP,
		"\n%s: line %d: link error %d vs link_count %d\n",
		__FILE__, __LINE__, val, link_count);
	    rc = 3; goto end;
	}

	c_list = (lookup("iClock"))->u.gate;	/* initialise clock list */

	pplc(igp, gate_count);	/* execute the compiled logic */
    } else {
	register Symbol	*tsp;
	register List_e	*tlp;
	char *		modName;
	char *		nxs;
	char *		sam;
	int		li;
	int		c;
	FILE *		Fp;
	FILE *		Hp;
	FILE *		Lp;
	char *		s1;
	char *		source;
	char *		module;
	unsigned int	linecnt = 35;	/* MODIFY when changing format */

	/* open output file */

	if ((Fp = fopen(outfile, "w")) == 0) {
	    rc = 4; goto end;
	}

	if ((Hp = fopen(Hname, aflag ? "a" : "w")) == 0) {
	    rc = 4; goto endh;
	}

	if ((Lp = fopen(Lname, aflag ? "a" : "w")) == 0) {
	    rc = 4; goto endl;
	}
	if (! aflag) fprintf(Lp, "#define	I_LIST\\\n");

	/* rewind intermediate file */

	if (fseek(exoFP, 0L, SEEK_SET) != 0) {
	    rc = 6; goto endm;
	}

	if ((s1 = strrchr(szFile_g, '/')) != 0) {
	    s1++;			/* UNIX file name without path */
	} else {
	    s1 = szFile_g;
	}

	if ((source = strrchr(s1, '\\')) != 0) {
	    source++;			/* MS-DOS file name without path */
	} else {
	    source = s1;
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
#include	<pplc.h>\n\
#include	\"%s\"\n\
\n\
#define _(x) x.gt_old\n\
#define A(x,v) assign(&x, v)\n\
extern Gate *	l_[];\n\
", source, outfile, SC_ID, Hname);

/********************************************************************
 *
 *	reverse action links to input links for simple Gates
 *
 *******************************************************************/

	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if ((typ = sp->type) < MAX_LV) {
		    if ((dc = sp->ftype) == ARITH || dc == GATE) {
			for (lp = sp->list; lp; lp = sp->list) {
			    tsp = lp->le_sym;	/* reverse action links */
			    tlp = tsp->u.blist;
			    tsp->u.blist = lp;	/* to input links */
			    sp->list = lp->le_next;
			    lp->le_sym = sp;
			    lp->le_next = tlp;
			}
		    }
		    if (typ == UDF) {
			fprintf(Fp, "extern Gate	%s;\n", mN(sp));
			linecnt++;
		    } else if (dc == OUTW && (lp = sp->list) != 0) {
			fprintf(Hp, "#define %s	%s\n",
			    sp->name, lp->le_sym->name); /* back link to Hname */
			    /* important to use unmodified names here */
		    }
		} else if (typ == CLK || typ == TIM) {

/********************************************************************
 *
 *	this initialisation of clock references relies on gates which
 *	execute a function ftype != GATE having only one output which
 *	is in the first location of the function list (fl_gate).
 *	The clock reference is put in the second location which was
 *	previously cleared by a terminator. The 3rd location holds a
 *	time value for timers.
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
			fprintf(Hp, "#define %s	%s\n",
			    mN(sp), mN(lp->le_sym));	/* ALIAS to Hname */
		    } else if (sp->ftype != GATE) {
			fprintf(Fp,
"/* error in emitting code. Alias '%s' has wrong ftype %s */\n",
			    sp->name, full_ftype[sp->ftype]);
			linecnt++;
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

	mask = li = 0;
	nxs = "0";
	sam = "";
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if ((typ = sp->type) > UDF && typ < MAX_OP &&
		    strcmp(sp->name, "iClock") != 0) {
		    modName = mN(sp);	/* modified string, byte and bit */
		    if (typ >= AND && typ < MAX_GT) {
			if (typ == AND || typ == LATCH) {
			    dc = 0;
			    for (lp = sp->u.blist; lp; lp = lp->le_next) {
				dc++;	/* space in input list */
			    }
			    if (typ == LATCH) {
				dc++;
				dc >>= 1;
			    }
			} else {
			    dc = 1;	/* typ == OR */
			}
			fprintf(Fp, "Gate %-8s = { %d, %d,", modName, dc, dc);
		    } else {
			if (sp->ftype == OUTW || typ == NCONST) {
			    fprintf(Fp, "Gate _%-7s", modName); /* modify */
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
			/* space for ACTION or FUNCTION pointer + CLOCK */
			li += 2;
			if ((lp = sp->list->le_next) == 0) {
			    fprintf(Fp,
"/* error in emitting code. Action gate '%s' has no clock */\n",
				sp->name);	/* ZZZ wrong format */
			} else if (lp->le_sym->type == TIM) {
			    li++;	/* space for TIME or time function */
			}
		    } else if (dc == OUTW) {
			if (iqt[0] == 'Q' &&
			    bwx[0] != 'X' &&	/* can only be B or W */
			    cnt == 3 && byte < IXD) {
			    fprintf(Fp, " (Gate**)&QX_[%d],", byte);
			    mask = bwx[0] == 'B' ? 1 : 2;
			} else {
			    goto OutErr;
			}
		    } else if (dc == OUTX) {
			if (iqt[0] == 'Q' &&
			    bwx[0] == 'X' &&
			    cnt == 4 && byte < IXD && bit < 16) {
			    /* OUTPUT byte part of bit pointer */
			    fprintf(Fp, " (Gate**)&QX_[%d],", byte);
			    mask = bitmask[bit];
			} else {
			OutErr:
			    fprintf(Fp, " (Gate**)&QX_[0],\n"
"/* error in emitting code. OUT configuration error %s */\n", sp->name);
			    mask = 0;	/* no output because mask is 0x00 */
			}
		    } else {
			fprintf(Fp, " 0,");		/* no gt_list */
		    }
		    if (typ == ARN || typ >= AND && typ < MAX_GT) {
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
			fprintf(Fp, ", %d", mask);	/* bitmask for OUT */
		    }
		    fprintf(Fp, " };\n");
		    linecnt++;
		    nxs = modName;		/* previous Symbol name */
		    sam = (dc == OUTW || typ == NCONST) ? "&_" : "&";
		    mask = 0;
		}
	    }
	}
	/* the following assumes I/O names are objects, never aliases */
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if ((typ = sp->type) == ALIAS && sp->list != 0 &&
		    ((dc = sp->ftype) == GATE || Aflag && dc == ARITH)) {
		    if (dc == ARITH) {
			fprintf(Fp, "Gate _%-7s", sp->name);	/* modify */
		    } else {
			fprintf(Fp, "Gate %-8s", sp->name);
		    }
		    fprintf(Fp,
		    " = { 1, -%s, %s, 0, \"%s\", 0, (Gate**)&%s, %s%s, %d };\n",
		    full_type[typ], full_ftype[dc], sp->name,
		    mN(sp->list->le_sym), sam, nxs, sp->list->le_val);
		    linecnt++;
		    nxs = sp->name;		/* previous Symbol name */
		    sam = dc == ARITH ? "&_" : "&";
		}
	    }
	}

	module = strdup(source);	/* source file name without path */
	if ((s1 = strchr(module, '.')) != 0) {
	    *s1 = '\0';			/* module name without extensions */
	}

	fprintf(Fp, "\nGate *		%s_i_list = %s%s;\n", module, sam, nxs);
	fprintf(Hp, "extern Gate *	%s_i_list;\n", module);
	fprintf(Lp, "	&%s_i_list,\\\n", module);
	free(module);

	fprintf(Fp, "\n\
/********************************************************************\n\
 *\n\
 *	Embedded C fragment functions\n\
 *\n\
 *******************************************************************/\n\
\n");

	/* copy C intermediate file up to EOF to C output file */
	while ((c = getc(exoFP)) != EOF) {
	    putc(c, Fp);
	    if (c == '\n') linecnt++;
	}

/********************************************************************
 *
 *	produce initialised connection lists
 *	using modified symbol table
 *
 *******************************************************************/

	if (li == 0) goto endm;		/* zero length comnnection list */
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
		if ((typ = sp->type) != UDF && typ < MAX_GT) {
		    int		len = 16;
		    char *	fs = strlen(sp->name) > 1 ? "\t" : "\t\t";

		    fprintf(Fp, "/* %s */", sp->name);
		    if ((dc = sp->ftype) >= MIN_ACT && dc < MAX_ACT) {
			if ((lp = sp->list) == 0) {
			    fprintf(Fp,
    "/* error in emitting code. ACTION gate '%s' has no action list */\n",
				sp->name);
			} else {
			    if (lp->le_sym == 0) {	/* dc == F_SW or F_CF */
				/* Function Pointer for on construct */
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
				    fs, tsp->name);		/* clock */
				if (tsp->type == TIM) {
				    if ((signed)lp->le_val < 0) {
					/* Function Pointer for timer */
					len += 17;
					fprintf(Fp, "%s(Gate*)cexe_%d,",
					    fs, -lp->le_val);
				    } else {
					/* Numeric value for timer */
					len += 13;
					fprintf(Fp, "%s(Gate*)%d,",
					    fs, lp->le_val);
				    }
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
			    fprintf(Fp, "%s(Gate*)cexe_%d,", fs, lp->le_val);
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
	fclose(Lp);	/* close the list header file in case other actions */
    endl:
	fclose(Hp);	/* close the header file in case other actions */
    endh:
	fclose(Fp);	/* close the output file in case other actions */
    }
end:
#ifndef _WINDOWS
    if (outFP != stdout) fclose(outFP);
#endif
    if (errFP != stderr) fclose(errFP);
    return rc;		/* return code */
}

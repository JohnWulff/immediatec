static const char outp_c[] =
"@(#)$Id: outp.c,v 1.95 2014/08/18 08:24:43 jw Exp $";
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
#include	<sys/stat.h>
#ifndef	WIN32
#include	<unistd.h>
#endif	/* WIN32 */
#include	<string.h>
#include	<ctype.h>
#include	<assert.h>
#include	<errno.h>
#include	"icc.h"
#include	"comp.h"

#define SZSIZE	64

extern const char	iC_ID[];

static int		icerrFlag = 0;
typedef struct OverFlow {		/* allow alternate storage of Symbol or List_e pointers */
    Symbol *	target;			/* target Symbol * that overflowed */
    Symbol *	aux;			/* store linked Symbol * list of auxiliary gates */
    Symbol *	curr;			/* current linked Symbol * of auxiliary gates */
    int		inp;			/* real size of gate */
    int		div;			/* number of aux gates required */
    int		direct;			/* number of direct inputs after linking aux gates */
} OverFlow;
static OverFlow *	szList = NULL;		/* dynamic auxiliary input gate list for iC_listNet */
static int		szSize = 0;		/* allocated size of szList */
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

static int
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
iC_toIEC1131(char * name, char * buf, int bufLen,
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
} /* iC_toIEC1131 */

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
    if (sp->name == 0) return strncpy(np, "(null)", 7);	/* in case of more suprises */
    cnt = IEC1131(sp->name, np, BUFS, iqt, xbwl, &byte, &bit, tail);
    return np;
} /* mN */

unsigned short	iC_aflag;			/* -a on compile to append output */
unsigned short	iC_lflag;			/* -a build new aux files  */

static unsigned	block_total;			/* total gates   - shared by listNet and buildNet */
static unsigned	link_count;			/* forward links - shared by listNet and buildNet */
static unsigned	revl_count;			/* reverse links - shared by listNet and buildNet */
static unsigned	gate_count[MAX_LS];		/* gates by type - shared by listNet and buildNet */
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

#ifdef BOOT_COMPILE
#define SI	20				/* shift index in le_val */

static void
out_link(List_e * lp, int * picnt, int * pbcnt, int vflag)
{
    Symbol *		sp;
    char *		name;
    static int		acnt;
    static char *	headName = 0;
    static char		auxName[TSIZE];
    static char		auxLink[TSIZE];

    if (vflag == 2) {
	acnt = 1;
    }
    if ((sp = lp->le_sym) != 0) {
	name = sp->name;
	if  (name) {
	    if (strchr(name, '@') == 0) {
		headName = name;		/* only set once as first name in function definiton */
	    }
	    snprintf(auxName, TSIZE, "0");
	    if (vflag != 1) {
		sp->v_cnt |= *pbcnt & 0777;		/* set link1 */
	    } else {
		sp->v_cnt |= (*pbcnt & 0777) << 9;	/* set link2 */
	    }
	} else {
	    assert(headName);
	    snprintf(auxName, TSIZE, "&%sa_%d", headName, acnt);
	    if (vflag < 3) {
		acnt++;
		sp->name = iC_emalloc(strlen(auxName));	/* already 1 byte longer */
		strcpy(sp->name, auxName+1);	/* pass name without leading '&' */
	    }
	}
    } else {
	name = "no var list Symbols";
    }
    if (lp->le_next) {
	snprintf(auxLink, TSIZE, "&l[%d]", *pbcnt + 1);
    } else {
	snprintf(auxLink, TSIZE, "0");
    }
    printf("/* %3d */ { %-11s, %2d, %-7s, %-2s, %-2s, },	/* ==> %s%s */\n",
	*pbcnt,
	auxName,
	lp->le_val,
	auxLink,
	lp->le_first ? "\"\"" : "0",
	lp->le_last  ? "\"\"" : "0",
	vflag <= 1 && (lp->le_val == NOT) ? "~" : " ",
	name ? name : auxName+1);
    assert(lp->le_val < 1 << SI);		/* make sure shifted index fits */
    lp->le_val |= *pbcnt << SI;			/* save own index in l[] */
    if (picnt) (*picnt)++;			/* count Symbol in S.T. */
    (*pbcnt)++;					/* count link */
} /* out_link */

static void
out_builtin(List_e * lp, int * picnt)
{
    Symbol *		sp;
    char *		str_type;
    char *		str_ftype;
    char		str_list[10];
    char		str_blist[10];
    char		str_link1[10];
    char		str_link2[20];
    char		str_tail[20];
    char		spaces[] = "              ";

    sp = lp->le_sym;
    assert(sp && sp->name);
    str_type  = iC_full_type[sp->type];
    str_ftype = iC_full_ftype[sp->ftype];
    if (sp->list) snprintf(str_list, 10, "&l[%d]", sp->list->le_val >> SI); else strncpy(str_list, "0", 10);
    if (sp->blist) snprintf(str_blist, 10, "&l[%d]", sp->blist->le_val >> SI); else strncpy(str_blist, "0", 10);
    snprintf(str_link1, 10, "&l[%d]", sp->v_cnt & 0777);	/* link1 always used - first Symbol has &l[0] */
    if (sp->v_cnt & 0777000000) {
	snprintf(str_link2, 20, "(List_e*)&b[%d]", (sp->v_cnt & 0777000000) >> 18);
	snprintf(str_tail,  20, "alternate %s", sp->name);
	str_tail[strlen(str_tail)-1] = '\0';	/* chop trailing '2' of name */
    } else {
	if (sp->v_cnt & 0777000) {
	    snprintf(str_link2, 20, "&l[%d]", (sp->v_cnt & 0777000) >> 9);
	} else {
	    strncpy(str_link2, "0", 20);	/* no link2 */
	}
	strncpy(str_tail, "function block", 20);
    }
    if (sp->type == IFUNCT) {
	printf("  { \"%1$s\",%3$*2$.*2$s%4$s,%6$*5$.*5$s%7$s,%9$*8$.*8$s0x%10$02x, %11$-7s, %12$-7s, %13$-7s, %14$-7s, }, /* %15$d pre-installed %1$s %16$s */\n"
	    , sp->name			/* 1$ */
	    , 12 - strlen(sp->name)	/* 2$ */
	    , spaces			/* 3$ */
	    , str_type			/* 4$ */
	    , 8 - strlen(str_type)	/* 5$ */
	    , spaces			/* 6$ */
	    , str_ftype			/* 7$ */
	    , 6 - strlen(str_ftype)	/* 8$ */
	    , spaces			/* 9$ */
	    , sp->fm			/* 10$ */
	    , str_list			/* 11$ */
	    , str_blist			/* 12$ */
	    , str_link1			/* 13$ */
	    , str_link2			/* 14$ */
	    , *picnt			/* 15$ */
	    , str_tail			/* 16$ */
	);	
    } else {
	printf("  { \"%1$s\",%3$*2$.*2$s%4$s,%6$*5$.*5$s%7$s,%9$*8$.*8$s0x%10$02x, %11$-7s, %12$-7s, %13$-7s, %14$-7s, },\n"
	    , sp->name			/* 1$ */
	    , 14 - strlen(sp->name)	/* 2$ */
	    , spaces			/* 3$ */
	    , str_type			/* 4$ */
	    , 6 - strlen(str_type)	/* 5$ */
	    , spaces			/* 6$ */
	    , str_ftype			/* 7$ */
	    , 6 - strlen(str_ftype)	/* 8$ */
	    , spaces			/* 9$ */
	    , sp->fm			/* 10$ */
	    , str_list			/* 11$ */
	    , str_blist			/* 12$ */
	    , str_link1			/* 13$ */
	    , str_link2			/* 14$ */
	);	
    }
    (*picnt)++;
} /* out_builtin */

#endif	/* BOOT_COMPILE */
/********************************************************************
 *
 *	Output the forward network as NET TOPOLOGY in the listing file.
 *	Gather information on total memory required for generation in
 *	buildNet(). These are block_total, link_count and gate_count[]
 *	for each type of Gate. This must be done after the iC and
 *	C compile phase but before buildNet().
 *
 *	outNet() and later load generate their own counts.
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
iC_listNet(void)
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
    int		fflag;
    int		szFlag;
    int		szCount;

    link_count = revl_count = block_total = undefined = iClockAlias = 0;	/* init each time */
    for (typ = 0; typ < MAX_LS; typ++) {
	gate_count[typ] = 0;
    }
#if YYDEBUG	/* ############### SYMBOL TABLE ################## */
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
		    int fm = sp->fm & FU;
		    fprintf(iC_outFP, "\n\t%-20s %-6s %-2s %-2s %-6s",
			sp->name,
			iC_full_type[sp->type],
			(sp->em & EM)		? "EM" : "",
			fm == 0 ? "" : fm == 1	? "01" :
				       fm == 2	? "10" : "11",
			iC_full_ftype[sp->ftype]);
		    if (sp->type == ALIAS) {
			Symbol * sp1 = sp;
			List_e * lp1;
			Symbol * sp2;
			while (sp1 &&
			    sp1->type == ALIAS &&
			    (((sp1->fm & FM) == 0 && (lp1 = sp1->list) != 0) ||
			    ((sp1->fm & FM) != 0 && (lp1 = sp1->u_blist) != 0)) &&
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
#ifdef	BOOT_COMPILE
    if (lineno && (iC_debug & 020000)) {
	/********************************************************************
	 * Generate the pre-compiled functions for including permanently in
	 * init.c from the iC source file init_t.ic
	 *
	 * This scheme only works for simple functions with one level of
	 * master gates for each input and individual clocks for each input if
	 * the builtin function is clocked (all except FORCE() are clocked)
	 *
	 * execute the following after making a compiler with -D BOOT_COMPILE
	 *	immcc -d20000 init_t.ic | init_t.pl > init_t.out
	 *	vi init_t.out init.c	# insert the new code appropriately
	 *	# init_t.ic has the definitions of the buil-in functions with
	 *	# mangled names in an appropriate alphabetical order
	 *	# init_t.pl translates the mangled names to their proper names
	 *	# and moves the block of static Symbols ahead of the link
	 *	# definitions, so that addresses of static Symbols can be used
	 *	# in some links.
	 *	# use the modified init.c to make a new immcc compiler.
	 *	# (the original was produced manually and was used to check
	 *	# that this generated code produces the right results - now
	 *	# the automatic generator can be used to vary the built-ins)
	 *******************************************************************/
	static Symbol *	vlist;
	static Symbol *	vp;
	static BuiltIn*	flist;
	static int	bcnt = 0;
	static int	icnt = 0;

	Symbol *	functionHead;
	List_e *	slp;
	Symbol *	vsp;
	List_e *	vlp;
	char *		name;
	int		parCnt;
	int		indexChange;
	int		indexClock;
	int		indexTimer;
	int		indexTimer1;

	printf(
"/********************************************************************\n"
" * Pre-compiled built-in functions produced in functionDefinition()\n"
" *\n"
" * Bootstrapping the original built-in functions, requires an older\n"
" * version of the compiler which compiles built-ins directly.\n"
" *\n"
" * if BOOT_COMPILE is defined this direct compilation is left active\n"
" * and any pre-compiled functions are left out of init.c.\n"
" *******************************************************************/\n"
"\n"
"#ifndef BOOT_COMPILE\n"
"static List_e	l[];\n"
	);
	printf(
"static List_e	l[] = {\n"
"/*         le_sym    le_val  le_next  le_first le_last */\n"
	);
	/********************************************************************
	 * Scan Symbol Table and extract all function heads
	 * This code only works correctly with file init_t.ic, which contains
	 * function block definitions of the built-in functions to be compiled
	 * with a BOOT_COMPILE old style iC compiler to produce a template for
	 * each function which is printed by the following code
	 *******************************************************************/
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (functionHead = *hsp; functionHead; functionHead = functionHead->next) {
		name = functionHead->name + 1;		/* leave out first mangled character */
		if (functionHead->type == IFUNCT) {
		    printf( "\n");
		    if (strcasecmp(name, "hange") == 0) {
			functionHead->fm = 0x01;	/* 1 bit signature for CHANGE set manually */
			indexChange = icnt;		/* save index in b[] for CHANGE2 */
		    } else if (strcasecmp(name, "hange2") == 0) {
			functionHead->fm = 0x02;	/* 1 int signature for CHANGE2 set manually */
			functionHead->v_cnt |= indexChange << 18;	/* link from CHANGE2 to CHANGE */
		    } else {
			if (strcasecmp(name, "lock") == 0) {
			    indexClock = icnt;		/* save index in b[] for CLOCK2 */
			} else if (strcasecmp(name, "lock2") == 0) {
			    functionHead->v_cnt |= indexClock << 18;	/* link from CLOCK2 to CLOCK */
			} else if (strcasecmp(name, "imer") == 0) {
			    indexTimer = icnt;		/* save index in b[] for TIMER2 */
			} else if (strcasecmp(name, "imer2") == 0) {
			    functionHead->v_cnt |= indexTimer << 18;	/* link from TIMER2 to TIMER */
			} else if (strcasecmp(name, "imer1") == 0) {
			    indexTimer1 = icnt;		/* save index in b[] for TIMER12 */
			} else if (strcasecmp(name, "imer12") == 0) {
			    functionHead->v_cnt |= indexTimer1 << 18;	/* link from TIMER12 to TIMER1 */
			}
		    }
		    /********************************************************************
		     * Boot Pass 1: function head list
		     * do this list independently to make sure links are consecutive
		     *******************************************************************/
		    slp = functionHead->u_blist;	/* start of statement list */
		    assert(slp);
		    vlist = vp = 0;			/* var list Symbols */
		    flist = 0;				/* function list BuiltIns */
		    while (slp) {
			sp = slp->le_sym;		/* formal satement head Symbol */
			assert(sp);			/* do not handle functions with immC arrays */
			lp = sp->list;			/* possible link to function head */
			while (lp) {
			    out_link(lp, &icnt, &bcnt, 0);	/* output feedback link */
			    lp = lp->le_next;		/* has no follow ups - but just in case */
			}
			out_link(slp, &icnt, &bcnt, 2);	/* output statement list link */
			vlp = slp->le_next;		/* next varList link */
			assert(vlp);			/* statement list is in pairs */
			out_link(vlp, 0, &bcnt, 3);	/* output var list link */
			slp = vlp->le_next;		/* next statement link */
		    }
		    /********************************************************************
		     * Boot Pass 2: statement list
		     * no recursive scanning of expression links is needed, if first level
		     * expression links are output from statement list and then all var
		     * nodes with one level of expression links each
		     *******************************************************************/
		    assert(functionHead);
		    slp = functionHead->u_blist;	/* start of statement list */
		    while (slp) {
			sp = slp->le_sym;		/* formal satement head Symbol */
			lp = sp->u_blist;		/* cloned expression links */
			while (lp) {
			    out_link(lp, 0, &bcnt, 0);	/* output first level expression link */
			    lp = lp->le_next;		/* next expression link */
			}
			vlp = slp->le_next;		/* next varList link */
			vsp = vlp->le_sym;		/* varList of temp Symbols */
			while (vsp) {			/* varList may be empty */
			    lp = vsp->u_blist;		/* cloned expression links */
			    while (lp) {
				out_link(lp, 0, &bcnt, 0);	/* output var expression link */
				lp = lp->le_next;	/* next expression link */
			    }
			    vsp = vsp->next;		/* next varList Symbol */
			}
			slp = vlp->le_next;		/* next statement link */
		    }
		    /********************************************************************
		     * Boot Pass 3: parameter list
		     * finally output links to real parameters
		     *******************************************************************/
		    slp = functionHead->list;		/* parameter list */
		    assert(slp);
		    parCnt = 0;
		    while (slp) {
			out_link(slp, &icnt, &bcnt, 1);	/* output parameter link */
			if (slp->le_sym->ftype < MIN_ACT) {
			    parCnt += 2;		/* count bit and int parameters in steps of 2 */
			}
			slp = slp->le_next;
		    }
		    assert(parCnt <= 6);
		    if (functionHead->fm == 0) {
			functionHead->fm = (1 << parCnt) - 1;	/* 0x03 0x0f 0x3f bit or int signature */
		    }
		}
	    }
	}
	printf(
"}; /* l[] */\n"
	);
	/********************************************************************
	 * Boot Interlude 1: output var list Symbols
	 *******************************************************************/
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (functionHead = *hsp; functionHead; functionHead = functionHead->next) {
		if (functionHead->type == IFUNCT) {
		    slp = functionHead->u_blist;	/* start of statement list */
		    while (slp) {
			vlp = slp->le_next;		/* next varList link */
			/********************************************************************
			 * output individual var lists in reverse order so that .next links are defined
			 * use latest C syntax to initialise named elements of a struct
			 *******************************************************************/
			for (tsp = 0; vlp->le_sym != tsp; tsp = vsp) {
			    for (vsp = vlp->le_sym; vsp->next != tsp; vsp = vsp->next);
			    assert(vsp->u_blist);
			    if (vsp->next) {
				printf("static Symbol	%-10s = { .type=%-3s, .ftype=%-6s, .fm=0x%02x, .u_blist=&l[%d], .next=&%s };\n",
				    vsp->name,
				    iC_full_type[vsp->type],
				    iC_full_ftype[vsp->ftype],
				    vsp->fm,
				    vsp->u_blist->le_val >> SI,
				    vsp->next->name);	/* link new var list */
			    } else {
				printf("static Symbol	%-10s = { .type=%-3s, .ftype=%-6s, .fm=0x%02x, .u_blist=&l[%d] };\n",
				    vsp->name,
				    iC_full_type[vsp->type],
				    iC_full_ftype[vsp->ftype],
				    vsp->fm,
				    vsp->u_blist->le_val >> SI);
			    }
			}
			slp = vlp->le_next;		/* next statement link */
		    }
		    printf("\n");
		}
	    }
	}
	printf(
"#else	/* BOOT_COMPILE */\n"
"Symbol		iC_CHANGE_ar = { \"CHANGE\", KEYW, CH_AR, };	/* alternative arithmetic CHANGE */\n"
"#endif	/* BOOT_COMPILE */\n"
"\n"
"/********************************************************************\n"
" *\n"
" *	Initialise Symbol table with all reserved words for iC and C\n"
" *\n"
" *		built-in symbols:	\"name\"	for built-in\n"
" *					type	for built-in\n"
" *					ftype	for built-in\n"
" *					fm	usage for built-in Symbol\n"
" *					list	output list pointer\n"
" *					blist	forward blist pointer\n"
" *					link1	2 back links for new built-in\n"
" *					link2	set up in BOOT_COMPILE\n"
" *\n"
" *		reserved words:		\"name\"\n"
" *					type	KEYW etc\n"
" *					ftype	used in compilation\n"
" *					uVal	compiler_token\n"
" *\n"
" *******************************************************************/\n"
"\n"
"static BuiltIn b[] = {\n"
"#ifndef BOOT_COMPILE\n"
"  /* name            type   ftype  fm    list     blist    link1    link2/glist */\n"
	);
	/********************************************************************
	 * Boot Interlude 2: output pre-defined Symbols
	 *******************************************************************/
	icnt = 0;
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (functionHead = *hsp; functionHead; functionHead = functionHead->next) {
		if (functionHead->type == IFUNCT) {
		    /********************************************************************
		     * Boot Pass 1: function head list
		     * do this list independently to make sure links are consecutive
		     *******************************************************************/
		    slp = functionHead->u_blist;	/* start of statement list */
		    assert(slp);
		    vlist = vp = 0;			/* var list Symbols */
		    flist = 0;				/* function list BuiltIns */
		    while (slp) {
			sp = slp->le_sym;		/* formal satement head Symbol */
			assert(sp);			/* do not handle functions with immC arrays */
			lp = sp->list;			/* possible link to function head */
			while (lp) {
			    out_builtin(lp, &icnt);	/* formal function head Symbol */
			    lp = lp->le_next;		/* has no follow ups - but just in case */
			}
			out_builtin(slp, &icnt);	/* formal satement head Symbol */
			vlp = slp->le_next;		/* next varList link */
			slp = vlp->le_next;		/* next statement link */
		    }
		    /********************************************************************
		     * Boot Pass 3: parameter list
		     * finally output links to real parameters
		     *******************************************************************/
		    slp = functionHead->list;		/* parameter list */
		    assert(slp);
		    while (slp) {
			out_builtin(slp, &icnt);	/* output parameter Symbol */
			slp = slp->le_next;
		    }
		}
	    }
	}
	printf(
"										/* %d pre-installed BuiltIn function Symbols */\n"
"#else	/* BOOT_COMPILE */\n"
"  /* name	     type   ftype  uVal	*/\n"
	, icnt);
	printf(
"#endif	/* BOOT_COMPILE */\n"
"}; /* b[] */\n"
	);
    }
#endif	/* BOOT_COMPILE */
    /********************************************************************
     *	Clear all v_cnt fields to allow accumulation of gate inputs
     *******************************************************************/
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if (sp->type < MAX_LS) {		/* allows IFUNCT to use union v.cnt */
		sp->v_cnt = 0;			/* v.elist & v.glist no longer needed */
	    }
	}
    }
    szFlag = szCount = 0;
    /********************************************************************
     *	Detect large AND or OR gates > PPGATESIZE in this pass. (LATCH always small)
     *	Since gates are forward linked an indirect stategy must be used.
     *	Increment v_cnt in each target gate of type AND or OR
     *	If v_cnt > PPGATESIZE store gate details and set a general flag szFlag.
     *	Keep a list of these gates and generate auxiliary gates at
     *	the end of this Pass (can only only install them between scans)
     *******************************************************************/
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if (sp->type < MAX_LS) {		/* allows IFUNCT to use union v.cnt */
		if (sp->ftype == GATE) {
		    for (lp = sp->list; lp; lp = lp->le_next) {
			tsp = lp->le_sym;	/* logical target */
			if (tsp && (tsp->type == AND || tsp->type == OR)) {
			    if (tsp->v_cnt++ == PPGATESIZE) {	/* count logical inputs */
				szFlag++;	/* has just overflowed */
				while (szFlag >= szSize) {
				    szList = (OverFlow*)realloc(szList,	/* initially NULL */
					(szSize + SZSIZE) * sizeof(OverFlow));
				    assert(szList);
				    memset(&szList[szSize], '\0', SZSIZE * sizeof(OverFlow));
				    szSize += SZSIZE;		/* increase the size of the array */
#if YYDEBUG
				    if ((iC_debug & 0402) == 0402)
					fprintf(iC_outFP, "iC_listNet: szList[%d] increase\n", szSize);
#endif
				}
				szList[szCount].target = tsp;	/* remember gate which is too large */
				szCount = szFlag;
			    }
			}
		    }
		}
		if (sp->type == ALIAS) {
		    /* check if iClock is referenced in an ALIAS */
		    if ((sp->fm & FM) == 0) {
			if (sp->ftype == CLCKL) {
			    tsp = sp;
			    while (tsp->type == ALIAS && (tsp->fm & FM) == 0) {
				tsp = tsp->list->le_sym;	/* point to original */
				assert(tsp->ftype == CLCKL);
			    }
			    if (tsp == iclock) {
				iClockAlias = 1;	/* include iClock below if ALIAS */
			    }
			} else
			if (iC_Aflag && sp->ftype == ARITH) {
			    tsp = sp;
			    while (tsp->type == ALIAS && (tsp->fm & FM) == 0) {
				tsp = tsp->list->le_sym;	/* point to original */
				assert(tsp->ftype == ARITH);
			    }
			    if (tsp->type == NCONST) {
				tsp->u_val++;	/* alias uses constant target in C code */
			    }
			}
		    }
		} else
		if (sp->type == ARNF) {
		    sp->type = ARN;		/* convert ARNF back to ARN */
		} else
		if (sp->type == NCONST) {
		    for (lp = sp->list; lp; lp = lp->le_next) {
			tsp = lp->le_sym;	/* numeric target */
			if (tsp && (tsp->fm & FM) == 0) {
			    sp->u_val++;	/* use count of non function references */
			}
		    }
		}
	    }
	}
    }
    iClockHidden = 0;
    if (szCount) {
	if (iC_debug & 010) {
	    fprintf(iC_outFP, "\n******* Large AND or OR gates detected *********\n");
	}
	/********************************************************************
	 *	Large AND or OR gates have been detected and can now be split.
	 *	Only here can the full size of a large gate be determined.
	 *	The changes will appear in the Net Topology listing
	 *	From then on all logic gates will be <= PPGATESIZE
	 *******************************************************************/
	int	i, j, inp, div, rem, direct, atn;
	char *	cp;
	Symbol* spAux;
	Symbol* spPrev;
	List_e* lp1;
	char	temp[TSIZE];
	char	temp1[TSIZE];
	for (i = 0; i < szCount; i++) {
	    tsp = szList[i].target;		/* gate has too many inputs - must be split */
	    assert(tsp && tsp->v_cnt > PPGATESIZE);
	    inp = tsp->v_cnt;			/* now the full count has been reached */
	    tsp->v_cnt = 0;			/* needed again when relinking */
	    /********************************************************************
	     *  Install 'div' auxiliary gate extensions to allow more than 127 inputs per gate.
	     *  Connect these first as non-inverting inputs to the target inputs.
	     *  Then connect first 'direct' inputs to remaining inputs of target gate.
	     *  These may already be inverting. Terminate inversion state correctly.
	     *  Connect remaining inputs to auxiliary gates, taking care that each
	     *  auxiliary gate starts and finished with the correct inversion state.
	     *******************************************************************/
	    div = inp / PPGATESIZE;		/* all signed integer arithmetic */
	    rem = inp % PPGATESIZE;
	    if ((direct = PPGATESIZE - div) < rem) {
		div++; direct--;		/* requires an extra auxiliary gate */
	    }
	    szList[i].inp = inp;		/* save for later processing */
	    szList[i].div = div;
	    szList[i].direct = direct;
	    /********************************************************************
	     *	Create div auxiliary gates and links and link them to the aux list
	     *******************************************************************/
	    atn = 0;
	    strncpy(temp, tsp->name, TSIZE);
	    if ((cp = strrchr(temp, '_')) != 0) {	/* last occurence of _ */
		if (sscanf(cp, "_%d", &atn) == 1) {
		    *cp = '\0';			/* terminate string at extension */
		}
	    }
	    for (j = 0; j < div; j++) {
		spAux = (Symbol *) iC_emalloc(sizeof(Symbol));
		do {
		    snprintf(temp1, TSIZE, "%s_%d", temp, ++atn);
		} while (lookup(temp1) != 0);
		spAux->name = iC_emalloc(strlen(temp1)+1);
		strcpy(spAux->name, temp1);
		spAux->type = tsp->type;
		spAux->ftype = GATE;		/* remaining fields are 0 */
		spAux->list = lp1 = sy_push(spAux);
		lp1->le_sym = tsp;		/* aux gate drives original */
		if (j == 0) {
		    szList[i].curr = szList[i].aux = spAux;	/* first member of aux list */
		} else {
		    spPrev->next = spAux;	/* linked list on next instead of S.T. */
		}
		spPrev = spAux;
		if (iC_debug & 04) {
		    fprintf(iC_outFP, "\n\t%s\t  ---%c", spAux->name, iC_os[tsp->type]);
		    if (j == 0) {
			fprintf(iC_outFP, "\t%s\t// %d inputs - use %d direct inputs and share rest over %d aux gate%s",
			    tsp->name, inp, direct, div, div > 1 ? "s" : "");
		    }
		}
	    }
	    if (iC_debug & 04) {
		fprintf(iC_outFP, "\n\n");
	    }
	}
	/********************************************************************
	 *	Clear all v_cnt fields again - some have been partially counted up
	 *******************************************************************/
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if (sp->type < MAX_LS) {
		    sp->v_cnt = 0;
		}
	    }
	}
	/********************************************************************
	 *	Count auxiliary gates to original gates
	 *******************************************************************/
	for (i = 0; i < szCount; i++) {
	    tsp = szList[i].target;		/* overflow target */
	    spAux = szList[i].aux;
	    do {
		spPrev = spAux->next;
		tsp->v_cnt++;			/* count auxiliary gates in original target before direct */
	    } while ((spAux = spPrev)!= 0);
	}
	/********************************************************************
	 *	Relink overflow to auxiliary gates
	 *******************************************************************/
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if (sp->type < MAX_LS) {		/* allows IFUNCT to use union v.cnt */
		    if (sp->ftype == GATE) {
			for (lp = sp->list; lp; lp = lp->le_next) {
			    tsp = lp->le_sym;	/* logical target */
			    if (tsp->v_cnt++ >= PPGATESIZE) {	/* count logical inputs of original target */
				/********************************************************************
				 *	Pick correct auxiliary gate computed by tsp->v_cnt relative to total size
				 *	Must keep the orignal list so the Symbols can be link to S.T.
				 *******************************************************************/
				for (i = 0; i < szCount; i++) {
				    if (szList[i].target == tsp) {
					break;		/* correct target found */
				    }
				}
				assert(i < szCount);	/* means target has been found */
				spAux = szList[i].curr;
				if (spAux->v_cnt >= PPGATESIZE) {	/* count logical inputs in auxiliary */
				    spAux = szList[i].curr = spAux->next;	/* next auxiliary */
				    assert(spAux);
				}
				lp->le_sym = spAux;	/* some misc input now linked to this auxiliary */
				spAux->v_cnt++;
			    }
			}
		    }
		}
	    }
	}
	/********************************************************************
	 *	Install new nodes permanently
	 *******************************************************************/
	for (i = 0; i < szCount; i++) {
	    spAux = szList[i].aux;
	    do {
		spPrev = spAux->next;	/* install new nodes now */
		link_sym(spAux);	/* next is overwritten */
	    } while ((spAux = spPrev)!= 0);
	}
	/********************************************************************
	 *	Clear all v_cnt fields again - some have been partially counted up
	 *******************************************************************/
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if (sp->type < MAX_LS) {
		    sp->v_cnt = 0;
		}
	    }
	}
    }		/* end of splitting large gates */
    if (iC_debug & 020) {
	/* do not change spelling - used in 'pplstfix' */
	fprintf(iC_outFP, "\n******* NET TOPOLOGY    ************************\n\n");
    }
    /********************************************************************
     *	Continue normal processing
     *******************************************************************/
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if (sp->em & EM) {
		extFlag = 1;
	    }
	    if ((typ = sp->type) < MAX_LS && (sp->fm &= FM) == 0) {	/* clear use count in fm */
		if (typ != NCONST || sp->u_val != 0) {
		    if (sp == icerr && sp->list == 0) {
			icerrFlag = 1;			/* uninstall iCerr at end of loop */
			continue;
		    }
		    /* ignore iClock unless used, ERR (! CLK) or referenced in an ALIAS */
		    if (sp != iclock || sp->list != 0 || typ != CLK || iClockAlias) {
			gate_count[typ]++;		/* count ALIAS and ERR typ */
			if (typ < MAX_OP) {
			    block_total++;		/* don't count ALIAS and ERR typ */
			}
			/* ARNC or LOGC with a UDFA identifies an immC array variable */
			if ((typ == ARNC || typ == LOGC) && sp->ftype == UDFA) {
			    for (lp = sp->list; lp; lp = lp->le_next) {
				revl_count++;		/* count reverse link */
			    }
			    revl_count++;		/* for terminator */
			} else
			if (typ < MAX_LV &&		/* don't count outputs */
			    sp->ftype != OUTX && sp->ftype != OUTW) {
			    fflag = 0;
			    for (lp = sp->list; lp; lp = lp->le_next) {
				if (sp->ftype == F_CF && lp->le_sym == sp) {
				    fflag = 2;	/* detected _f0_1 */
				}
				if (lp->le_val != (unsigned)-1) {
				    if (fflag == 2) {
					fflag = 1;	/* miss only first link of _f0_1 */
				    } else {
					link_count++;
					if (sp->ftype < MAX_AR &&	/* arithmetic function */
					    lp->le_val != 0    &&
					    (tsp = lp->le_sym) != 0) {
					    assert(
						tsp->type == ARNC ||
						tsp->type == ARNF ||
						tsp->type == ARN ||
						tsp->type == SH ||
						tsp->type == NCONST ||
						tsp->type == INPW ||
						tsp->type == ERR);
					    /* also not IFUNCT, allows IFUNCT to use union v.cnt */
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
			/* actual output of a NET TOPOLOGY line */
			if (iC_debug & 020) {
			    if (typ == TIM && (sp->em & TM1)) {	/* Timer preset off */
				fprintf(iC_outFP, "%s\t%c1 %c", sp->name, iC_os[typ], iC_fos[sp->ftype]);
			    } else {
				fprintf(iC_outFP, "%s\t%c  %c", sp->name, iC_os[typ], iC_fos[sp->ftype]);
			    }
			    fcnt = 0;
			    for (lp = sp->list; lp; lp = lp->le_next) {
				tsp = lp->le_sym;
				if (tsp && tsp->fm != 0) continue;	/* no function internal variables */
				if (fcnt++ >= 8) {
				    fcnt = 1;
				    fprintf(iC_outFP, "\n\t");
				}
				if (sp->ftype == F_SW || sp->ftype == F_CF || sp->ftype == F_CE) {
				    /* unsigned case number of "if" or "switch" C fragment */
				    if (lp->le_val > 0 && lp->le_val <= VAR_MASK) {
					fprintf(iC_outFP, "\t%c (%d)",
					    iC_os[iC_types[sp->ftype]], lp->le_val >> FUN_OFFSET);
				    } else if (tsp == sp) {
					fcnt--;		/* dummy link of _f0_1 */
				    } else {
					goto normalOut;	/* reference link of _f0_1 */
				    }
				} else if (tsp->type == TIM && (tsp->em & TM1)) {	/* Timer preset off */
				    fprintf(iC_outFP, "\t %s%c1", tsp->name, iC_os[tsp->type]);
				    if (sp->type == ALIAS) {
					sp->em |= TM1;			/* Timer preset off for ALIAS */
				    }
				} else if (sp->ftype < MAX_AR && lp->le_val == (unsigned)-1) {
				    /* reference to a timer value - no link */
				    fprintf(iC_outFP, "\t<%s%c", tsp->name, iC_os[tsp->type]);
				} else {
				    normalOut:
				    if (tsp) {
					fprintf(iC_outFP, "\t%c%s%c",
					    (sp->ftype == GATE || sp->ftype == OUTX) &&
					    lp->le_val ? '~' : ' ',
					    tsp->name, iC_os[tsp->type]);
				    }
				}
			    }
			    fprintf(iC_outFP, "\n");
			}
			/* end of output of a NET TOPOLOGY line */
			if (typ == UDF) {
			    warning("undefined gate:", sp->name);
			} else if (typ == ERR) {
			    ierror("gate:", sp->name);
			}
		    } else {
			iClockHidden = 1;
		    }
		}
	    } else if (typ == UDF) {
		undefined++;	/* cannot execute if function internal gate not defined */
		ierror("undefined gate in function:", sp->name);
	    }
	}
    }
    if (icerrFlag) {
	uninstall(icerr);			/* iCerr was never used */
	icerr = 0;
	icerrFlag = 0;
    }
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if (sp->type < IFUNCT) {		/* allows IFUNCT to use union v.cnt */
		if (sp->v_cnt && (sp->type < AND || sp->type > LATCH)) {
		    assert(sp->type == ARN || sp->type == SH || sp->type == NCONST || sp->type == ARNC || sp->type == ERR);
		    link_count += sp->v_cnt + 1;	/* space for reverse links + function # */
		}
		sp->v_cnt = 0;			/* restore v_cnt for both uses */
	    }
	}
    }

    if (iC_debug & 010) {
	/* do not change spelling - used in 'pplstfix' */
	fprintf(iC_outFP, "\n******* NET STATISTICS  ************************\n\n");
	for (typ = 0; typ < MAX_LS; typ++) {
	    if (gate_count[typ]) {
		fprintf(iC_outFP, "%s\t%c %6d%s\n", iC_full_type[typ], iC_os[typ],
		    gate_count[typ], typ < MAX_OP ? " blocks" : "");
	    }
	}
	fprintf(iC_outFP, "\nTOTAL\t%8u blocks\n", block_total);
	fprintf(iC_outFP, "\t%8u links\n", link_count + revl_count);
    }
    if (iClockHidden) {
	block_total++;				/* iClock is generated anyway in buildNet() */
	gate_count[CLK]++;
    }
    if (iC_debug & 076) {
	/* do not change spelling "\ncompiled by:" - used in 'pplstfix' */
	fprintf(iC_outFP, "\ncompiled by:\n%s -%sO%o\n",
	    iC_ID, iC_gflag ? "g" : "", iC_optimise);
    }
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
} /* iC_listNet */
#if defined(RUN) || defined(TCP) && ! defined(LOAD)

/********************************************************************
 *
 *	Generate execution network for icr or ict direct execution
 *	igp ==> array of Gates generated with calloc(block_total)
 *
 *******************************************************************/

int
iC_buildNet(Gate *** asTable, Gate *** asTend)
{
    Symbol *	sp;
    Symbol *	tsp;
    List_e *	lp;
    Symbol **	hsp;
    int		typ;
    unsigned	val;
    unsigned	i;
    unsigned	rc = 0;				/* return code */
    Gate *	igp;
    Gate *	gp;
    Gate *	tgp;
    GppIpI	fp;				/* links - union of Gate ** and int */
    Gate **	ifp;
    Gate **	rp;				/* reverse links */
    Gate **	irp;
    Gate **	tfp;
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
    }						/* no need to mask ->type in buildNet() */

    /* initialise executable gates */

    igp = gp     = (Gate *)calloc(block_total, sizeof(Gate));	/* gates */
    ifp = fp.gpp = (Gate **)calloc(link_count, sizeof(Gate *));	/* links */
    /* use ifp to initialise rp if revl_count == 0 to avoid Segmentation faults on error */
    irp = rp = (revl_count > 0) ? (Gate **)calloc(revl_count, sizeof(Gate *)) : ifp; /* reverse links */

    for (typ = 0; typ < MAX_OP; typ++) {
	val = 0;
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if (sp->type == typ && (sp->fm & FM) == 0) {
		    if ((typ == ARNC || typ == LOGC) && sp->ftype == UDFA) {
			val++;			/* just count immC array */
		    } else
		    if (typ != NCONST || sp->u_val != 0) {
			gp->gt_ini = -typ;	/* overwritten for AND OR LATCH types */
			gp->gt_fni = sp->ftype;	/* basic gate first */
			gp->gt_ids = sp->name;	/* gate to symbol name */
			gp->gt_next = g_list;	/* link rest of g_list to new Gate */
			g_list = sp->u_gate = gp++;	/* symbol to gate - scrubs u_blist */
			val++;
		    }
		}
	    }
	}
	assert(val == gate_count[typ]);		/* check once only */
    }
    val = 0;
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    /* ARNC or LOGC with a UDFA identifies an immC array variable */
	    if (((typ = sp->type) == ARNC || typ == LOGC) && sp->ftype == UDFA && (sp->fm & FM) == 0) {
		gp->gt_ini = -typ;
		gp->gt_fni = sp->ftype;		/* basic gate first */
		gp->gt_ids = sp->name;		/* gate to symbol name */
		assert(sp->list->le_val);	   /* immC array has members */
		gp->gt_mark = sp->list->le_val; /* gt_mark = immC array size for now */
		gp->gt_rlist = rp;		/* start of immC array gate list */
		for (lp = sp->list; lp; lp = lp->le_next) {
		    tsp = lp->le_sym;
		    assert(tsp && tsp->u_gate);	/* non array ARNC and LOGC initialised above */
		    *rp++ = tsp->u_gate;	/* immC array members stored as gt_rlist */
		    val++;
		}
		*rp++ = 0;			/* immC array gate list terminator */
		val++;				/* gp handled here - not in next loop */
		gp->gt_next = g_list;		/* link rest of g_list to new Gate */
		g_list = sp->u_gate = gp++;	/* symbol to gate - scrubs u_blist */
	    }
	}
    }
    assert(val == revl_count);			/* check once only */
    if ((gp - igp) == block_total) {
	gp = igp;				/* repeat to initialise links */
	for (typ = 0; typ <= MAX_OP; typ++) {	/* include ALIAS */
	    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
		for (sp = *hsp; sp; sp = sp->next) {
		    if (sp->type == typ &&
			sp->u_gate == gp &&	/* exclude immC array - processed above */
			(sp->fm & FM) == 0 &&
			(typ != NCONST || sp->u_val != 0)) {
			if (typ < MAX_LV) {
			    gp->gt_list = fp.gpp;	/* start of gate list */
			    if (sp->ftype < MIN_ACT) {
				val = (sp->ftype < MAX_AR) ? NOT : 0;
				do {		/* go through list twice for LVs */
				    tfp = fp.gpp;	/* start of new gate list */
				    for (lp = sp->list; lp; lp = lp->le_next) {
					tsp = lp->le_sym;
					assert(tsp && tsp->u_gate);
					if (sp->ftype < MAX_AR) {
					    if (lp->le_val == (unsigned)-1) {
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
					*fp.gpp++ = tsp->u_gate;
				    }
				    /* sort gate list */
				    qsort(tfp, fp.gpp - tfp, sizeof(Gate*), (iC_fptr)iC_cmp_gt_ids);
				    *fp.gpp++ = 0;		/* gate list terminator */
				} while (val ^= NOT);
#ifndef	TCP
				if (typ == INPW) {		/* icr only */
				    if (sscanf(gp->gt_ids, "I%1[BWL]%d",
					bwl, &byte1) == 2 && byte1 < IXD) {
					if (bwl[0] == 'B') {
					    iC_IB_[byte1] = gp;
					} else if (bwl[0] == 'W') {
					    iC_IW_[byte1] = gp;
					} else if (bwl[0] == 'L') {
#if INT_MAX != 32767 || defined (LONG16)
					    iC_IL_[byte1] = gp;
#else	/* INT_MAX == 32767 && ! defined (LONG16) */
					    warning("32 bit INPUT not supported in 16 bit environment:", sp->name);
					    iC_IW_[byte1] = gp;	/* use 16 bit input instead */
#endif	/* INT_MAX != 32767 || defined (LONG16) */
					} else {
					    assert(0);	/* must be 'B', 'W' or 'L' */
					}
				    } else {
					goto inErr;
				    }
				} else if (typ == INPX) {	/* icr only */
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
				    *fp.gpp++ = tgp;
				    *fp.gpp++ = 0;		/* room for clock or timer entry */
				    *fp.gpp++ = 0;		/* room for time delay */
				    /* could do this only for action gates controlled by a TIMER */
				} else {
				    /* F_SW, F_CF or F_CE action gate points to function */
				    assert((sp->ftype == F_SW ||
					    sp->ftype == F_CF ||
					    sp->ftype == F_CE) &&
					   (lp->le_val & 0xff) == 0);
				    if (lp->le_val == 0) {
					tgp = sp->u_gate;
					assert(tgp && tsp == sp && strcmp(sp->name, "_f0_1") == 0);
					iC_pf0_1 = tgp;	/* pointer to _f0_1 used in cexe.c */
					tgp->gt_mcnt = 1;
					i = 0;
				    } else {
					fp.ip[0] = lp->le_val >> FUN_OFFSET;
					fp.gpp[1] = 0;	/* room for clock or timer entry */
					fp.gpp[2] = 0;	/* room for time delay or first parameter */
					i   = 3;	/* offset for above */
				    }
				    while ((lp = lp->le_next) != 0) {
					val = lp->le_val & VAR_MASK;
					tsp = lp->le_sym;
					while (tsp->type == ALIAS) {
					    tsp = tsp->list->le_sym;	/* point to original */
					}
					assert(tsp && tsp->u_gate);
					fp.gpp[val] = tsp->u_gate;
					i++;	/* count parameters */
				    }
				    fp.gpp += i;	/* space for above entries */
				}
#ifndef	TCP
			    } else if (sp->ftype == OUTW) {	/* icr only */
				if (sscanf(gp->gt_ids, "Q%1[BWL]%d", bwl, &byte1) == 2 &&
				    byte1 < IXD) {
				    gp->gt_slot = byte1;	/* slot index in union */
				    gp->gt_mark = bwl[0] == 'B' ? B_WIDTH :
						  bwl[0] == 'W' ? W_WIDTH :
#if INT_MAX != 32767 || defined (LONG16)
						  bwl[0] == 'L' ? L_WIDTH :
#else	/* INT_MAX == 32767 && ! defined (LONG16) */
						  bwl[0] == 'L' ? W_WIDTH : /* use as 16 bit output */
#endif	/* INT_MAX != 32767 || defined (LONG16) */
								  0;
				} else {
				    goto outErr;
				}
			    } else if (sp->ftype == OUTX) {	/* icr only */
				if (sscanf(gp->gt_ids, "QX%d.%d", &byte1, &bit1) == 2 &&
				    byte1 < IXD && bit1 < 8) {
				    gp->gt_slot = byte1;	/* slot index in union */
				    gp->gt_mark = iC_bitMask[bit1];
				} else {
				outErr:
				    ierror("OUTPUT byte or bit address exceeds limit:", gp->gt_ids);
				    gp->gt_slot = 0;
				    gp->gt_mark = 0;		/* no change in bits */
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
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if (((typ = sp->type) == ARNC || typ == LOGC) && sp->ftype == UDFA && (sp->fm & FM) == 0) {
		    assert(sp->u_gate == gp);	/* count immC array now */
		    gp++;
		}
	    }
	}
	if (iclock->type == ERR) {
	    gp++;				/* error - count iClock */
	    rc = 1;				/* generate error */
	}
	if ((gp - igp) == block_total) {
	    gp = igp;				/* repeat to initialise timer links */
	    for (typ = 0; typ < MAX_OP; typ++) {	/* keep gp in same order */
		for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
		    for (sp = *hsp; sp; sp = sp->next) {
			if (sp->type == typ &&
			    sp->u_gate == gp &&	/* exclude immC array - processed above */
			    (sp->fm & FM) == 0 &&
			    (typ != NCONST || sp->u_val != 0)) {
			    if (sp->ftype == ARITH) {	/* Arithmetic Gate */
				for (lp = sp->list; lp; lp = lp->le_next) {
				    tsp = lp->le_sym;
				    assert(tsp && tsp->u_gate);
				    if ((val = lp->le_val) != 0) {
					tgp = tsp->u_gate;
					if (val == (unsigned)-1) {
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
					    i = val & VAL_MASK;
					    val >>= FUN_OFFSET;
					    assert((tsp->type == ARN || tsp->type == ERR) &&
						   val && i && i <= tgp->gt_new);
					    if (tgp->gt_rlist == 0) {
						tgp->gt_rlist = fp.gpp;
						fp.gpp += tgp->gt_new + 1;
						tgp->gt_rfunctn = val;	/* reverse n in array */
					    } else {
						assert(val == tgp->gt_rfunctn);
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
		    if ((typ = sp->type) < MAX_OP &&
			(tgp = sp->u_gate) != 0 &&
			(sp->fm & FM) == 0 &&
			(typ != NCONST || sp->u_val != 0)) {
			tgp->gt_new = 0;	/* restore gt_new */
		    }
		    if (((typ = sp->type) == ARNC || typ == LOGC) && sp->ftype == UDFA && (sp->fm & FM) == 0) {
			assert(sp->u_gate == gp);	/* count immC array now */
			gp++;
		    }
		}
	    }
	    if (iclock->type == ERR) {
		gp++;				/* error - count iClock */
		rc = 1;				/* generate error */
	    }
	}
    }

    if ((val = gp - igp) != block_total) {
	fprintf(iC_errFP,			/* either first or second scan above */
	    "\n%s: line %d: block error %d vs block_total %d\n",
	    __FILE__, __LINE__, val, block_total);
	rc = 2;
    }
    if ((val = rp - irp) != revl_count) {
	fprintf(iC_errFP,
	    "\n%s: line %d: reverse link error %d vs revl_count %u\n",
	    __FILE__, __LINE__, val, revl_count);
	rc = 3;
    }
    if ((val = fp.gpp - ifp) != link_count) {
	fprintf(iC_errFP,
	    "\n%s: line %d: link error %d vs link_count %u\n",
	    __FILE__, __LINE__, val, link_count);
	rc = 4;
    }
    if (rc == 0){
#ifdef	TCP
	/********************************************************************
	 * Do ALIASes last to avoid forward references of Gates in gt_list
	 * Resolve multiple ALIASes here for the same reason.
	 * Generate all ALIASes for display, independent of -A flag
	 *******************************************************************/
	for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	    for (sp = *hsp; sp; sp = sp->next) {
		if ((typ = sp->type) == ALIAS && (sp->fm & FM) == 0 && sp->list != 0 &&
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
		    tgp->gt_ini = -typ;			/* ALIAS */
		    tgp->gt_fni = ftyp;			/* ftype */
		    tgp->gt_rptr = tsp->u_gate;		/* link to original */
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
			if (i != 4 || strcmp(tail1, "_0") != 0 || strcmp(iqt1, "Q") != 0) goto pass0Err;
			op->gt_mark = iC_bitMask[bit1];
			op->gt_ptr = tgp;		/* direct pointer (union with gt_list) */
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
	*asTable = *asTend = (Gate **)calloc(block_total, sizeof(Gate *));
	for (op = g_list; op; op = op->gt_next) {
	    *(*asTend)++ = op;				/* enter node into sTable */
#ifdef	TCP
	    /********************************************************************
	     *  arithmetic or logical input linkage to physical I/O (mainly display)
	     *******************************************************************/
	    if (op->gt_ini == -INPW) {
		if ((i = sscanf(op->gt_ids, "%1[IT]%1[XBWL]%5d%7s",
			iqt1, xbwl1, &byte1, tail1)) == 3) {
		    switch (iqt1[0]) {
		    case 'T':
			if (byte1 != 0) goto pass0Err;	/* TXD must be 1 */
			iC_TX0p = op;			/* forward input link */
			/* fall through */
		    case 'I':
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
			break;				/* op->gt_mark set to used bits 0x01 to 0x80 in Pass 0 */
		    case 'B':
			if (i != 3 || strcmp(tail1, "_0") != 0) goto pass0Err;
			op->gt_out = 0;			/* initial default value (gt_rlist no longer needed) */
			op->gt_mark = B_WIDTH;
			break;
		    case 'W':
			if (i != 3 || strcmp(tail1, "_0") != 0) goto pass0Err;
			op->gt_out = 0;			/* initial default value (gt_rlist no longer needed) */
			op->gt_mark = W_WIDTH;
			break;
#if INT_MAX != 32767 || defined (LONG16)
		    case 'L':
			if (i != 3 || strcmp(tail1, "_0") != 0) goto pass0Err;
			op->gt_out = 0;			/* initial default value (gt_rlist no longer needed) */
			op->gt_mark = L_WIDTH;
			break;
#endif	/* INT_MAX != 32767 || defined (LONG16) */
		    default:
			goto pass0Err;
		    }
		} else {
		    goto pass0Err;
		}
	    }
#endif	/* TCP */
	}
	if ((val = *asTend - *asTable) != block_total) {
	    fprintf(iC_errFP,			/* either first or second scan above */
		"\n%s: line %d: Symbol Table size %d vs block_total %d\n",
		__FILE__, __LINE__, val, block_total);
	    rc = 2;
	} else {
	    /********************************************************************
	     * Sort the symbol table in order of gt_ids.
	     *******************************************************************/
	    qsort(*asTable, val, sizeof(Gate*), (iC_fptr)iC_cmp_gt_ids);
	}
    }

    return rc;					/* return code */
} /* iC_buildNet */
#endif /* defined(RUN) || defined(TCP) && ! defined(LOAD) */

/********************************************************************
 *
 *	Generate network as C file
 *
 *******************************************************************/

int
iC_outNet(FILE * iFP, char * outfile)
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
    FILE *	H1p = 0;			/* declaration header .iC_list1.h */
    FILE *	H2p = 0;			/* list header .iC_list2.h */
    char *	cp;				/* auxiliary char pointer */
    char *	module;				/* module name built from path/name */
    unsigned	linecnt = 1;
    int		fflag;
    int		idx;
    int		ext;
    char	tail2[8];			/* compiler generated suffix _123456 max */

    /********************************************************************
     *
     *  Clear the c.expr cache and use the space in the union as c.use
     *
     *******************************************************************/

    for (idx = 1; idx < functionUseSize; idx++) {
	if ((cp = functionUse[idx].c.expr) != 0) {
	    free(cp);				/* free the cached expression space */
	    functionUse[idx].c.use = 0;		/* clear final use counter */
	}
    }

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

    if ((chmod(outfile, 0644) &&		/* make C output file writable */
	errno != ENOENT) ||			/* if it exists */
	(Fp = fopen(outfile, "w")) == NULL) {
	perror("chmod or fopen");
	rc = Oindex; goto endc;
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
static const char	iC_compiler[] =\n\
\"%s -%sO%o\";\n\
\n\
#include	<icg.h>\n\
",	inpNM, outfile, iC_ID, iC_gflag ? "g" : "", iC_optimise);
    linecnt += 11;

    /********************************************************************
     *	if iC_aflag generate auxiliary files .iC_list1.h and .iC_list2.h
     *  else write direct to C file
     *******************************************************************/

    if (iC_aflag) {
	/********************************************************************
	 *  include auxiliary files .iC_list1.h and .iC_list2.h
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
	     *	generate auxiliary files .iC_list1.h and .iC_list2.h
	     *******************************************************************/
	    if ((H1p = fopen(H1name, "w")) == 0) {	/* declaration header .iC_list1.h */
		rc = H1index; goto endh;
	    }
	    fprintf(H1p, "/*### Declaration header for '%s' ###*/\n", outfile);

	    if ((H2p = fopen(H2name, "w")) == 0) {	/* list header .iC_list2.h */
		rc = H2index; goto endd;
	    }
	    fprintf(H2p, "/*### List header for '%s' ###*/\n", outfile);
	    fprintf(H2p, "#define	iC_LIST\\\n");	/* list header .iC_list2.h */
	} else {
	    /********************************************************************
	     *	append to auxiliary files .iC_list1.h and .iC_list2.h
	     *******************************************************************/
	    if ((H1p = fopen(H1name, "a")) == 0) {	/* declaration header .iC_list1.h */
		rc = H1index; goto endh;
	    }
	    if ((H2p = fopen(H2name, "a")) == 0) {	/* list header .iC_list2.h */
		rc = H2index; goto endd;
	    }
	}
    }

    /********************************************************************
     *  Ignore function gates marked with fm != 0
     *  Output extern C statement for Gates which are UDF or marked with em
     *	Reverse action links to input links for simple Gates
     *	for Gates of ftype ARITH, keep the links in ascending order
     *******************************************************************/
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if (sp->fm == 0) {
		if ((typ = sp->type) == UDF || (sp->em & EM)) {
		    fprintf(Fp, "extern iC_Gt	%s;\n", mN(sp));
		    linecnt++;
		}
		if (typ < MAX_LV) {
		    ftyp = sp->ftype;
		    if (typ == INPX) {
			assert(ftyp == GATE || ftyp == ARITH);
			assert(sp->u_blist == 0);
		    }
		    if (ftyp == ARITH) {
			for (lpp = &sp->list; (lp = *lpp) != 0; ) {
			    List_e **	tlpp;
			    /* leave out timing controls */
			    if ((val = lp->le_val) != (unsigned)-1) {
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
			    if (lp->le_val != (unsigned)-1) {
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
		    }					/* temporarily in u (which is 0) */
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
    }

    /* do the timing controls last, to link them after their timer clock */
    for (hsp = symlist; hsp < &symlist[HASHSIZ]; hsp++) {
	for (sp = *hsp; sp; sp = sp->next) {
	    if (sp->type < MAX_LV && sp->fm == 0 && sp->ftype < MAX_AR) {
		for (lp = sp->list; lp; ) {
		    if (lp->le_val == (unsigned)-1) {
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
    if (functionUse[0].c_cnt & F_CALLED) {	/* has any function been called ? */
	if (functionUse[0].c_cnt & F_ARITHM) {
	    fprintf(Fp, "\
#define iC_MV(n)	iC_gf->gt_rlist[n]->gt_new\n\
");	    linecnt += 1;
	}
	if (functionUse[0].c_cnt & F_FFEXPR) {
	    fprintf(Fp, "\
#define iC_AV(n)	iC_gf->gt_list[n]->gt_new\n\
#define iC_LV(n)	(iC_gf->gt_list[n]->gt_val < 0 ? 1 : 0)\n\
#define iC_AA(n,p,v)	iC_assignA(iC_gf->gt_list[n], p, v)\n\
#define iC_LA(n,p,v)	iC_assignL(iC_gf->gt_list[n], p, v)\n\
");	    linecnt += 4;
	    if (functionUse[0].c_cnt & F_ARRAY) {
		fprintf(Fp, "\
#define iC_AVI(n,i)	iC_index(iC_gf->gt_list[n], i)->gt_new\n\
#define iC_LVI(n,i)	(iC_index(iC_gf->gt_list[n], i)->gt_val < 0 ? 1 : 0)\n\
#define iC_AAI(n,i,p,v)	iC_assignA(iC_index(iC_gf->gt_list[n], i), p, v)\n\
#define iC_LAI(n,i,p,v)	iC_assignL(iC_index(iC_gf->gt_list[n], i), p, v)\n\
");	    linecnt += 4;
	    }
	    if (functionUse[0].c_cnt & F_SIZE) {
		fprintf(Fp, "\
#define iC_SIZ(n)	iC_gf->gt_list[n]->gt_old\n\
");	    linecnt += 1;
	    }
	}
	if (functionUse[0].c_cnt & F_LITERAL) {
	    fprintf(Fp, "\
#define iC_AVL(n)	_f0_1.gt_list[n]->gt_new\n\
#define iC_LVL(n)	(_f0_1.gt_list[n]->gt_val < 0 ? 1 : 0)\n\
#define iC_AAL(n,p,v)	iC_assignA(_f0_1.gt_list[n], p, v)\n\
#define iC_LAL(n,p,v)	iC_assignL(_f0_1.gt_list[n], p, v)\n\
");	    linecnt += 4;
	    if (functionUse[0].c_cnt & F_ARRAY) {
		fprintf(Fp, "\
#define iC_AVIL(n,i)	iC_index(_f0_1.gt_list[n], i)->gt_new\n\
#define iC_LVIL(n,i)	(iC_index(_f0_1.gt_list[n], i)->gt_val < 0 ? 1 : 0)\n\
#define iC_AAIL(n,i,p,v) iC_assignA(iC_index(_f0_1.gt_list[n], i), p, v)\n\
#define iC_LAIL(n,i,p,v) iC_assignL(iC_index(_f0_1.gt_list[n], i), p, v)\n\
");	    linecnt += 4;
	    }
	    if (functionUse[0].c_cnt & F_SIZE) {
		fprintf(Fp, "\
#define iC_SIZL(n)	_f0_1.gt_list[n]->gt_old\n\
");	    linecnt += 1;
	    }
	}
    }
    fprintf(Fp, "\
static iC_Gt *	iC_l_[];\n\
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
		(sp->em & EM) == 0 && sp->fm == 0 && sp != iclock) {
		mask = 0;
		/********************************************************************
		 * mN() sets cnt, iqt, xbwl, byte, bit and tail via IEC1131() as side effect
		 *******************************************************************/
		modName = mN(sp);		/* modified string, bit is used in block */
		if (typ == NCONST) {
		    if (sp == iconst) {
			if (sp->u_val != 0) {
			    fprintf(Fp, "extern iC_Gt %s; /* %d */\n", modName, sp->u_val);
			    linecnt++;
			}
			continue;
		    } else
		    if (sp->u_val == 0) {
			continue;		/* do not include unused constants */
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
		ftyp = sp->ftype;
		if ((lp = sp->list) != 0 && lp->le_sym == sp) {
		    fflag = 1;			/* leave out _f0_1 */
		    /* generate gt_fni (ftype), gt_mcnt (1) and gt_ids */
		    fprintf(Fp, " %s, 1, \"%s\",",
			iC_ext_ftype[ftyp], sp->name);
		} else {
		    fflag = 0;
		    /* generate gt_fni (ftype), gt_mcnt (0) and gt_ids */
		    fprintf(Fp, " %s, 0, \"%s\",",
			iC_ext_ftype[ftyp], sp->name);
		}
		/* generate action gt_list */
		if (ftyp >= MIN_ACT && ftyp < MAX_ACT) {
		    /* gt_list */
		    fprintf(Fp, " {&iC_l_[%d]},", li);
		    if (fflag == 0) {		/* leave out _f0_1 */
			li += 2;		/* space for action or function pointer + clock */
			if ((lp = lp->le_next) != 0 &&
			    (tsp = lp->le_sym) != 0 &&
			    tsp->type == TIM && tsp->fm == 0) {
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
			fprintf(Fp, " {0},");
		    } else {
			goto OutErr;
		    }
		} else
		if (ftyp == OUTX) {
		    if (iqt[0] == 'Q' &&
			xbwl[0] == 'X' &&	/* QX0.0_0 is cnt == 5 */
			cnt == 5 && bit < 8) {
			fprintf(Fp, " {0},");
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
		    fprintf(Fp, " {0},");	/* no action gt_list */
		    if (ftyp == TIMRL) {
			mask = sp->u_val	/* preset off value for TIMER1 */;
			sp->u_val = 0;		/* restore temporary u to 0 */
		    }
		}
		/* generate gt_rlist */
		if ((typ == ARNC || typ == LOGC) && sp->ftype == UDFA) {
		    fprintf(Fp, " {&iC_l_[%d]},", li);	/* gt_rlist */
		    li += (mask = sp->list ? sp->list->le_val : 0) + 1;	/* immC array size + terminator */
		} else
		if (typ == ARN || (typ >= MIN_GT && typ < MAX_GT)) {
		    fprintf(Fp, " {&iC_l_[%d]},", li);	/* gt_rlist */
		    if (fflag == 0) {		/* leave dummy pointer for _f0_1 in gt_rlist */
			for (lp = sp->u_blist; lp; lp = lp->le_next) {
			    li++;		/* mark space in input list */
			}
			li += 2;		/* space for dual GATE list or ARITH with FUNCTION */
		    }
		} else
		if (typ == INPW) {
		    if (iqt[0] == 'I' &&	/* IB0 is cnt == 3 */
			xbwl[0] != 'X' &&	/* can only be 'B', 'W' or 'L' */
			cnt == 3) {
			fprintf(Fp, " {0},");
		    } else {
			goto InErr;
		    }
		} else
		if (typ == INPX) {
		    if (iqt[0] != 'Q' &&	/* can only be 'I' or 'T' */
			xbwl[0] == 'X' &&	/* IX0.0 is cnt == 4 */
			cnt == 4 && bit < 8) {
			fprintf(Fp, " {0},");
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
		    fprintf(Fp, " {0},");	/* no gt_rlist */
		}
		/* generate gt_next, which points to previous Gate */
		fprintf(Fp, " %s%s", sam, nxs);
		/********************************************************************
		 * optionally generate non-zero timer preset value in gt_mark
		 * or bitMask for OUT, or immC array size
		 * which is printed here to C file where it is picked up
		 * in load.c main() Pass 0 and transferred to gt_old
		 *******************************************************************/
		if (mask != 0) {
		    fprintf(Fp, ", %d", mask);	/* bitMask for OUT or immC array size in gt_mark */
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
	    if ((typ = sp->type) == ALIAS &&
		(sp->em & EM) == 0 &&
		sp->fm == 0 &&
		sp->list != 0 &&
		iC_Aflag &&
		((ftyp = sp->ftype) == GATE || ftyp == ARITH || ftyp == CLCKL || ftyp == TIMRL)
	    ) {
		modName = mN(sp);		/* modified string, byte and bit */
		fprintf(Fp, "iC_Gt %-8s", modName);
		val = sp->list->le_val;
		tsp = sp->list->le_sym;
		while (tsp->type == ALIAS && (tsp->em & EM) == 0 && tsp->fm == 0) {
		    val ^= tsp->list->le_val;	/* negate if necessary */
		    tsp = tsp->list->le_sym;	/* point to original */
		}
		fprintf(Fp,
		    " = { 1, -%s, %s, 0, \"%s\", {0}, {(iC_Gt**)&%s}, %s%s, %d };\n",
		    iC_ext_type[typ], iC_ext_ftype[ftyp], sp->name, mN(tsp), sam, nxs, val);
		linecnt++;
		nxs = modName;			/* previous Symbol name */
		sam = "&";
	    }
	}
    }
    /********************************************************************
     *	Link counting in outNet() counts all reverse links and is thus very
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
	fprintf(H1p, "extern iC_Gt *	iC_%s_list;\n",
	    module);					/* declaration header .iC_list1.h */
	fprintf(H2p, "	&iC_%s_list,\\\n", module);	/* list header .iC_list2.h */
    } else {
	/********************************************************************
	 *	iC_list already complete - no need for aux files
	 *******************************************************************/
	fprintf(Fp, "\
iC_Gt **	iC_list[] = { &iC_%s_list, 0, };\n\
",	module);
	linecnt += 1;
    }
    free(module);
    fprintf(Fp, "\
\n\
"); linecnt += 1;

    if (functionUse[0].c_cnt & F_CALLED) {	/* has any function been called ? */
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

	if ((rc = iC_copyXlate(iFP, Fp, outfile, &linecnt, 01)) != 0) { /* copy literal blocks */
	    goto endm;
	}
	if ((rc = iC_copyXlate(iFP, Fp, outfile, &linecnt, 02)) != 0) { /* copy called functions */
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
	    if (((typ = sp->type) == ARN ||		/* leave out UDF, ARNC, LOGC */
		((typ == ARNC || typ == LOGC) && sp->ftype == UDFA) ||	/* except immC array */
		(typ >= MIN_GT && typ < MAX_GT)) &&
		(sp->em & EM) == 0 && sp-> fm == 0) {	/* leave out EXT_TYPES */
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
			    val = lp->le_val >> FUN_OFFSET;
			    assert(val && val < functionUseSize);
			    fprintf(Fp, "%s(iC_Gt*)iC_%d,", fs, val);
			    functionUse[val].c.use++;	/* count the actual function ref */
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
				assert((tsp->type == CLK || tsp->type == TIM) && tsp->fm == 0);
				fprintf(Fp, "%s&%s,", fs, mN(tsp));	/* clock or timer */
				if (tsp->type == TIM && tsp->fm == 0) {
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
				unsigned int	useWord;
				int		useBits;
				int		i;
				int		i1;
				int		j;

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
					gpp[j++] = (Gate *)(unsigned long)useWord;
					i = useWord = 0;
				    }
				    while (tsp->type == ALIAS) {
					tsp = tsp->list->le_sym;	/* point to original */
				    }
				    len += (i1 = tsp->name ? strlen(tsp->name) : 0) + 3;
				    if (len > 73) {
					fs = "\n\t\t";
					linecnt++;
					len = 16 + i1 + 3;
				    }
				    fprintf(Fp, "%s&%s,", fs, mN(tsp));
				    fs = " ";
				    lc++;
				}
				if (i) {
				    gpp[j++] = (Gate *)(unsigned long)useWord;
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
				    fprintf(Fp, "%s(iC_Gt*)0x%lx,", fs, (long)gpp[j]);	/* write use bits */
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

		if ((typ == ARNC || typ == LOGC) && sp->ftype == UDFA) {
		    for (lp = sp->list; lp; lp = lp->le_next) {	/* set up immC array */
			len += 7;			/* assume len of %d is 2 */
			if (len > 73) {
			    fs = "\n\t\t";
			    linecnt++;
			    len = 16 + 7;
			}
			fprintf(Fp, "%s&%s,", fs, mN(lp->le_sym));	/* immC array members */
			fs = " ";
			lc++;
		    }
		    len += 3;
		    fprintf(Fp, "%s0,", fs);		/* 0 terminator - needed for Pass 6 check */
		    lc++;
		} else
		if (typ == ARN) {
		    if ((lp = sp->u_blist) == 0) {
			snprintf(errorBuf, sizeof errorBuf,
			    "Arithmetic gate '%s' has no function",
			    sp->name);
			errorEmit(Fp, errorBuf, &linecnt);
		    } else {
			/* Function Pointer at start of input list */
			len += 13;			/* assume len of %d is 2 */
			if (len > 73) {
			    fs = "\n\t\t";
			    linecnt++;
			    len = 16 + 13;
			}
			if ((val = lp->le_val >> FUN_OFFSET) != 0) {
			    assert(val < functionUseSize);
			    fprintf(Fp, "%s(iC_Gt*)iC_%d,", fs, val);
			    functionUse[val].c.use++;	/* count the actual function ref */
			} else {
			    fprintf(Fp, "%s(iC_Gt*)0,", fs);	/* OUTW */
			}
			fs = " ";
			lc++;
		    }
		    for (val = 0, lp = sp->u_blist; lp; lp = lp->le_next) {
			len += strlen(lp->le_sym->name) + 3;
			if (len > 73) {
			    fs = "\n\t\t";
			    linecnt++;
			    len = 16 + strlen(lp->le_sym->name) + 3;
			}
			/* check order of arithmetic input index from op_asgn() */
			/* needs fixing - arithmetic input > 256 - cannot count in byte */
#if YYDEBUG
			if (lp->le_val != 0 && ++val != (lp->le_val & VAL_MASK)) {
			    fprintf(iC_outFP, "*** output: differ  %s ==> %s val = 0x%04x lp->le_val = 0x%04x\n",
				sp->name, lp->le_sym->name, val, lp->le_val);
			}
#else
			assert(lp->le_val == 0 || ++val == (lp->le_val & VAL_MASK));
#endif
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
		    val = 0;
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
	    } else if (iC_aflag) {		/* appending more modules */
		assert(H1p);
		if ((typ == TIM || typ == ALIAS) && (sp->em & TM1) != 0) {
		    fprintf(H1p, "/* TIMER1:\t%s\t%d */\n", sp->name, TM1);	/* declaration header .iC_list1.h */
		} else if (typ == IFUNCT &&	/* function has occurred */
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
		    fprintf(H1p, "/* InstanceNum:\t%s\t%d */\n", sp->name, val);	/* declaration header .iC_list1.h */
		}
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

    /********************************************************************
     *
     *  Collect use statistics and display errors or divergencies
     *
     *******************************************************************/

    for (idx = 1; idx < functionUseSize; idx++) {
	if ((li = functionUse[idx].c_cnt) != (lc = functionUse[idx].c.use)) {
	    if (li == 0) {
		snprintf(errorBuf, sizeof errorBuf,
		    "ERROR: C function (%d) is used %d times but was never generated",
		    idx, lc);
		errorEmit(Fp, errorBuf, &linecnt);
	    } else if (lc == 0) {
		snprintf(errorBuf, sizeof errorBuf,
		    "WARNING: C function (%d) has generate count %d but never used",
		    idx, li);
		errorEmit(Fp, errorBuf, &linecnt);
	    } else {
		snprintf(errorBuf, sizeof errorBuf,
		    "WARNING: C function (%d) has generate count %d but used %d times",
		    idx, li, lc);
		errorEmit(Fp, errorBuf, &linecnt);
	    }
	}
	if (iC_gflag && lc > 1) {
	    snprintf(errorBuf, sizeof errorBuf,
		"WARNING: C function (%d) was used %d times despite -g option",
		idx, lc);
	    errorEmit(Fp, errorBuf, &linecnt);
	}
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
    rc |= chmod(outfile, 0444);		/* make C output file read-only */
endc:
    return rc;				/* return code */
} /* iC_outNet */

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
	    if ((c = iC_openT4T5(0)) != 0) return c;	/* re-open if necessary */
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
			if ((c = iC_openT4T5(0)) != 0) return c;	/* re-open if necessary */
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
			if ((c = iC_openT4T5(0)) != 0) return c;	/* re-open if necessary */
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
iC_c_compile(FILE * iFP, FILE * oFP, int flag, List_e * lp)
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
	fprintf(T2FP, "/*##*/long iC_exec(int iC_indx) { switch (iC_indx) {\n");
#else
	fprintf(T2FP, "/*##*/int iC_exec(int iC_indx) { switch (iC_indx) {\n");
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
    copyAdjust(NULL, NULL, 0);			/* initialize lineEntryArray */
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
} /* iC_c_compile */

/********************************************************************
 *
 *	Then this code is parsed by the C compiler and afterwards split on
 *	## in comments and output to oFP
 *
 *******************************************************************/

int
iC_copyXlate(FILE * iFP, FILE * oFP, char * outfile, unsigned * lcp, int mode)
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
		flag = functionUse[cFn].c_cnt;	/* has this function or case been called ? */
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
} /* iC_copyXlate */
